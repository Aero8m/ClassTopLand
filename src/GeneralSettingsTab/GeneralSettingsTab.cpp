#include "GeneralSettingsTab.h"
#include "ui_GeneralSettingsTab.h"
#include "../Utils/ConfigSettings.h"

#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QStringList>
#include <string>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#include <shlobj.h>
#endif

namespace {
void setError(QLabel *label, const QString &message)
{
    label->setText(message);
    label->setVisible(!message.isEmpty());
    if (!message.isEmpty()) {
        qWarning().noquote() << message;
    }
}

#if defined(Q_OS_WIN)
constexpr auto runKey = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
constexpr auto appKey = "ClassTopLand";

// Older Inno Setup installers create a .lnk instead of a Run registry value.
// Resolve the Windows shell folders rather than assuming a ProgramData path.
bool startupShortcuts(QStringList *shortcuts, QString *error)
{
    shortcuts->clear();
    for (const int folder : {CSIDL_COMMON_STARTUP, CSIDL_STARTUP}) {
        wchar_t path[MAX_PATH] = {};
        const HRESULT result = SHGetFolderPathW(nullptr, folder, nullptr, SHGFP_TYPE_CURRENT, path);
        if (FAILED(result)) {
            *error = GeneralSettingsTab::tr("无法读取 Windows 启动文件夹（错误码：%1）。")
                         .arg(static_cast<quint32>(result));
            return false;
        }
        const QString shortcut = QDir(QString::fromWCharArray(path)).filePath(QStringLiteral("ClassTopLand.lnk"));
        const std::wstring nativePath = QDir::toNativeSeparators(shortcut).toStdWString();
        const DWORD attributes = GetFileAttributesW(nativePath.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            const DWORD code = GetLastError();
            if (code != ERROR_FILE_NOT_FOUND && code != ERROR_PATH_NOT_FOUND) {
                *error = GeneralSettingsTab::tr("无法检查启动快捷方式：%1（错误码：%2）。")
                             .arg(shortcut).arg(code);
                return false;
            }
        } else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
            *error = GeneralSettingsTab::tr("启动项路径被同名文件夹占用：%1").arg(shortcut);
            return false;
        } else {
            shortcuts->append(shortcut);
        }
    }
    return true;
}

bool readAutoStart(bool *enabled, QString *error)
{
    QStringList shortcuts;
    if (!startupShortcuts(&shortcuts, error)) {
        return false;
    }
    QSettings settings(QString::fromLatin1(runKey), QSettings::NativeFormat);
    settings.sync();
    *enabled = !shortcuts.isEmpty() || !settings.value(QString::fromLatin1(appKey)).toString().isEmpty();
    if (settings.status() != QSettings::NoError) {
        *error = GeneralSettingsTab::tr("无法读取 Windows 自启设置。");
        return false;
    }
    return true;
}

bool writeAutoStart(bool enabled, QString *error)
{
    QStringList shortcuts;
    if (!startupShortcuts(&shortcuts, error)) {
        return false;
    }
    if (enabled && !shortcuts.isEmpty()) {
        // The installer already registered startup. Do not add a second launch.
        return true;
    }
    if (!enabled) {
        for (const QString &shortcut : shortcuts) {
            const std::wstring nativePath = QDir::toNativeSeparators(shortcut).toStdWString();
            // Delete the .lnk itself, never its target executable.
            if (!DeleteFileW(nativePath.c_str())) {
                const DWORD code = GetLastError();
                if (code == ERROR_FILE_NOT_FOUND || code == ERROR_PATH_NOT_FOUND) {
                    continue;
                }
                *error = GeneralSettingsTab::tr(
                    "无法删除安装程序创建的启动快捷方式：%1（错误码：%2）。"
                    "所有用户的启动项通常需要管理员权限，请以管理员身份运行程序后重试。")
                             .arg(shortcut).arg(code);
                return false;
            }
        }
    }
    const QString command = QLatin1Char('"')
                            + QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
                            + QLatin1Char('"');
    QSettings settings(QString::fromLatin1(runKey), QSettings::NativeFormat);
    if (enabled) {
        settings.setValue(QString::fromLatin1(appKey), command);
    } else {
        settings.remove(QString::fromLatin1(appKey));
    }
    settings.sync();
    if (settings.status() != QSettings::NoError
        || (enabled ? settings.value(QString::fromLatin1(appKey)).toString() != command
                    : settings.contains(QString::fromLatin1(appKey)))) {
        *error = GeneralSettingsTab::tr("无法更新 Windows 自启设置，请检查系统权限后重试。");
        return false;
    }
    return true;
}
#endif
}

GeneralSettingsTab::GeneralSettingsTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GeneralSettingsTab)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    autoStartCheckBox = ui->autoStartCheckBox;
    alwaysOnTopCheckBox = ui->alwaysOnTopCheckBox;
    autoStartError = ui->autoStartCheckBoxError;
    alwaysOnTopError = ui->alwaysOnTopCheckBoxError;
    statusLabel = ui->generalSettingsStatus;
    restoreButton = ui->restoreGeneralSettingsButton;
    applyButton = ui->applyGeneralSettingsButton;
    autoStartError->hide();
    alwaysOnTopError->hide();
    autoStartCheckBox->setToolTip(tr(
        "兼容安装程序创建的启动快捷方式。关闭所有用户的启动项可能需要管理员权限。"));
#if !defined(Q_OS_WIN)
    autoStartCheckBox->setEnabled(false);
    ui->startupDescription->setText(tr("当前平台暂不支持开机自启"));
#endif

    connect(autoStartCheckBox, &QCheckBox::toggled, this, &GeneralSettingsTab::updateDirtyState);
    connect(alwaysOnTopCheckBox, &QCheckBox::toggled, this, &GeneralSettingsTab::updateDirtyState);
    connect(restoreButton, &QPushButton::clicked, this, &GeneralSettingsTab::discardChanges);
    connect(applyButton, &QPushButton::clicked, this, &GeneralSettingsTab::applySettings);
    reloadSettings();
}

GeneralSettingsTab::~GeneralSettingsTab()
{
    delete ui;
}

void GeneralSettingsTab::reloadSettings()
{
    QString error;
#if defined(Q_OS_WIN)
    bool enabled = savedAutoStart;
    if (readAutoStart(&enabled, &error)) {
        savedAutoStart = enabled;
    }
    setError(autoStartError, error);
#endif
    error.clear();
    QJsonObject config;
    if (ConfigSettings::read(&config, &error)) {
        savedAlwaysOnTop = config.value(ConfigSettings::alwaysOnTopKey).toBool(true);
    }
    setError(alwaysOnTopError, error);
    discardChanges();
}

void GeneralSettingsTab::discardChanges()
{
    const QSignalBlocker autoStartBlocker(autoStartCheckBox);
    const QSignalBlocker alwaysOnTopBlocker(alwaysOnTopCheckBox);
    autoStartCheckBox->setChecked(savedAutoStart);
    alwaysOnTopCheckBox->setChecked(savedAlwaysOnTop);
    updateDirtyState();
}

bool GeneralSettingsTab::hasChanges() const
{
    return (autoStartCheckBox->isEnabled() && autoStartCheckBox->isChecked() != savedAutoStart)
           || alwaysOnTopCheckBox->isChecked() != savedAlwaysOnTop;
}

void GeneralSettingsTab::updateDirtyState()
{
    const bool dirty = hasChanges();
    applyButton->setEnabled(dirty);
    restoreButton->setEnabled(dirty);
    statusLabel->setText(dirty ? tr("有未应用的更改") : QString{});
}

void GeneralSettingsTab::applySettings()
{
    bool failed = false;
    bool applied = false;
    QString error;
#if defined(Q_OS_WIN)
    if (autoStartCheckBox->isChecked() != savedAutoStart) {
        if (writeAutoStart(autoStartCheckBox->isChecked(), &error)) {
            savedAutoStart = autoStartCheckBox->isChecked();
            applied = true;
            setError(autoStartError, {});
        } else {
            failed = true;
            setError(autoStartError, error);
        }
    }
#endif
    if (alwaysOnTopCheckBox->isChecked() != savedAlwaysOnTop) {
        QJsonObject savedConfig;
        if (ConfigSettings::mergeAndSave(
                {{ConfigSettings::alwaysOnTopKey, alwaysOnTopCheckBox->isChecked()}}, &savedConfig, &error)) {
            savedAlwaysOnTop = alwaysOnTopCheckBox->isChecked();
            applied = true;
            setError(alwaysOnTopError, {});
            emit configSaved(savedConfig);
            emit alwaysOnTopChanged(savedAlwaysOnTop);
        } else {
            failed = true;
            setError(alwaysOnTopError, error);
        }
    }
    updateDirtyState();
    statusLabel->setText(failed ? (applied ? tr("部分设置已应用，其余更改未应用，请重试。")
                                          : tr("应用失败，更改未应用，请重试。"))
                                : tr("设置已应用"));
}
