#include "WindowInterceptionTab.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHash>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>

#if defined(Q_OS_WIN)
#include <qt_windows.h>
#include <optional>
#include <string>
#endif

struct WindowInterceptionTab::Impl
{
#if defined(Q_OS_WIN)
    struct Rule
    {
        QString exePath;
        QString className;
        QString titleContains;
        bool enabled = true;
    };

    struct WindowInfo
    {
        HWND handle = nullptr;
        DWORD processId = 0;
        QString title;
        QString exePath;
        QString className;
        bool visible = false;
    };

    QVector<Rule> rules;
    QHash<quintptr, DWORD> hiddenWindows;
    QHash<quintptr, DWORD> recoveryWindows;
    HWINEVENTHOOK showHook = nullptr;
    QTableWidget *currentTable = nullptr;
    QTableWidget *rulesTable = nullptr;
    QLineEdit *titleFilter = nullptr;
    QLabel *errorLabel = nullptr;
    bool stopped = false;
#endif
};

#if defined(Q_OS_WIN)
namespace {
WindowInterceptionTab *activeTab = nullptr;

QString rulesPath()
{
    return QDir::homePath() + QStringLiteral("/ClassTopLand_Data/window_rules.json");
}

QString normalizedPath(const QString &path)
{
    return QDir::cleanPath(path).replace('\\', '/');
}

QString processPath(DWORD processId)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (!process)
        return {};

    std::wstring buffer(32768, L'\0');
    DWORD size = static_cast<DWORD>(buffer.size());
    const bool found = QueryFullProcessImageNameW(process, 0, buffer.data(), &size);
    CloseHandle(process);
    return found ? normalizedPath(QString::fromWCharArray(buffer.data(), static_cast<qsizetype>(size))) : QString{};
}

std::optional<WindowInterceptionTab::Impl::WindowInfo> inspectWindow(HWND handle)
{
    if (!IsWindow(handle) || GetAncestor(handle, GA_ROOT) != handle)
        return std::nullopt;

    DWORD processId = 0;
    if (!GetWindowThreadProcessId(handle, &processId) || processId == GetCurrentProcessId())
        return std::nullopt;

    const QString exePath = processPath(processId);
    if (exePath.isEmpty())
        return std::nullopt;

    wchar_t classBuffer[256] = {};
    if (!GetClassNameW(handle, classBuffer, 256))
        return std::nullopt;

    wchar_t titleBuffer[512] = {};
    GetWindowTextW(handle, titleBuffer, 512);

    return WindowInterceptionTab::Impl::WindowInfo{
        handle, processId, QString::fromWCharArray(titleBuffer), exePath,
        QString::fromWCharArray(classBuffer), IsWindowVisible(handle) != FALSE
    };
}

QVector<WindowInterceptionTab::Impl::WindowInfo> enumerateWindows()
{
    QVector<WindowInterceptionTab::Impl::WindowInfo> windows;
    EnumWindows([](HWND handle, LPARAM context) -> BOOL {
        auto *result = reinterpret_cast<QVector<WindowInterceptionTab::Impl::WindowInfo> *>(context);
        if (auto info = inspectWindow(handle))
            result->append(*info);
        return TRUE;
    }, reinterpret_cast<LPARAM>(&windows));
    return windows;
}

bool matches(const WindowInterceptionTab::Impl::Rule &rule,
             const WindowInterceptionTab::Impl::WindowInfo &window)
{
    return rule.exePath.compare(window.exePath, Qt::CaseInsensitive) == 0
        && rule.className == window.className
        && (rule.titleContains.isEmpty()
            || window.title.contains(rule.titleContains, Qt::CaseInsensitive));
}

void CALLBACK onWindowEvent(HWINEVENTHOOK, DWORD, HWND handle, LONG objectId,
                            LONG childId, DWORD, DWORD)
{
    if (!activeTab || objectId != OBJID_WINDOW || childId != CHILDID_SELF)
        return;

    auto *tab = activeTab;
    QMetaObject::invokeMethod(tab, [tab, handle] {
        tab->handleWindowShown(reinterpret_cast<quintptr>(handle));
    }, Qt::QueuedConnection);
}

void setTableItem(QTableWidget *table, int row, int column, const QString &text)
{
    auto *item = new QTableWidgetItem(text);
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    item->setToolTip(text);
    table->setItem(row, column, item);
}

bool saveRules(const QVector<WindowInterceptionTab::Impl::Rule> &rules, QString *error)
{
    QJsonArray array;
    for (const auto &rule : rules) {
        array.append(QJsonObject{
            {QStringLiteral("exePath"), rule.exePath},
            {QStringLiteral("className"), rule.className},
            {QStringLiteral("titleContains"), rule.titleContains},
            {QStringLiteral("enabled"), rule.enabled}
        });
    }

    QSaveFile file(rulesPath());
    if (!file.open(QIODevice::WriteOnly)) {
        *error = file.errorString();
        return false;
    }
    const QByteArray content = QJsonDocument(QJsonObject{{QStringLiteral("rules"), array}})
                                   .toJson(QJsonDocument::Indented);
    if (file.write(content) != content.size()) {
        *error = file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        *error = file.errorString();
        return false;
    }
    return true;
}

bool loadRules(QVector<WindowInterceptionTab::Impl::Rule> *rules, QString *error)
{
    QFile file(rulesPath());
    if (!file.exists())
        return true;
    if (!file.open(QIODevice::ReadOnly)) {
        *error = file.errorString();
        return false;
    }
    const QByteArray content = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        *error = file.errorString();
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(content, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()
        || !document.object().value(QStringLiteral("rules")).isArray()) {
        *error = parseError.error == QJsonParseError::NoError
                     ? QStringLiteral("规则文件格式无效")
                     : parseError.errorString();
        return false;
    }
    QVector<WindowInterceptionTab::Impl::Rule> parsed;
    for (const QJsonValue &value : document.object().value(QStringLiteral("rules")).toArray()) {
        if (!value.isObject()) {
            *error = QStringLiteral("规则文件包含非对象条目");
            return false;
        }
        const QJsonObject item = value.toObject();
        if (!item.value(QStringLiteral("exePath")).isString()
            || !item.value(QStringLiteral("className")).isString()
            || !item.value(QStringLiteral("titleContains")).isString()
            || !item.value(QStringLiteral("enabled")).isBool()
            || item.value(QStringLiteral("exePath")).toString().isEmpty()
            || item.value(QStringLiteral("className")).toString().isEmpty()) {
            *error = QStringLiteral("规则文件包含无效字段");
            return false;
        }
        parsed.append({normalizedPath(item.value(QStringLiteral("exePath")).toString()),
                       item.value(QStringLiteral("className")).toString(),
                       item.value(QStringLiteral("titleContains")).toString(),
                       item.value(QStringLiteral("enabled")).toBool()});
    }
    *rules = parsed;
    return true;
}
}
#endif

WindowInterceptionTab::WindowInterceptionTab(QWidget *parent)
    : QWidget(parent), impl(std::make_unique<Impl>())
{
    QFont pageFont = font();
    pageFont.setFamily(QStringLiteral("Microsoft YaHei"));
    pageFont.setPointSize(10);
    setFont(pageFont);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(10);
    setObjectName(QStringLiteral("windowInterceptionTab"));
    setStyleSheet(QStringLiteral("QWidget#windowInterceptionTab { background: #ffffff; }"));

#if !defined(Q_OS_WIN)
    auto *message = new QLabel(tr("窗口拦截仅支持 Windows，当前系统暂不可用。"), this);
    message->setAlignment(Qt::AlignCenter);
    message->setWordWrap(true);
    layout->addWidget(message);
#else
    impl->errorLabel = new QLabel(this);
    impl->errorLabel->setStyleSheet(QStringLiteral("color:#b42318;"));
    impl->errorLabel->setWordWrap(true);
    impl->errorLabel->hide();
    layout->addWidget(impl->errorLabel);

    auto *cardsLayout = new QHBoxLayout();
    cardsLayout->setSpacing(10);
    auto *currentCard = new QFrame(this);
    currentCard->setObjectName(QStringLiteral("fluentCard"));
    auto *currentLayout = new QVBoxLayout(currentCard);
    currentLayout->setContentsMargins(16, 12, 16, 14);
    currentLayout->setSpacing(8);
    auto *currentTitle = new QLabel(tr("当前窗口"), currentCard);
    QFont cardTitleFont = pageFont;
    cardTitleFont.setPointSize(11);
    cardTitleFont.setBold(true);
    currentTitle->setFont(cardTitleFont);
    auto *refresh = new QPushButton(tr("刷新列表"), currentCard);
    auto *currentHeader = new QHBoxLayout();
    currentHeader->addWidget(currentTitle);
    currentHeader->addStretch();
    currentHeader->addWidget(refresh);
    currentLayout->addLayout(currentHeader);

    impl->currentTable = new QTableWidget(0, 3, currentCard);
    const QString tableStyle = QStringLiteral(R"(
        QTableWidget {
            background: #ffffff;
            border-radius: 10px;
        }
        QTableWidget::item {
            border: none;
            border-radius: 3px;
        }
        QTableWidget::item:selected {
            background: #1191d3;
        }
        QTableWidget::item:hover {
            color: #000000;
            background: rgb(217, 217, 217);
        }
        QHeaderView::section {
            border-radius: 3px;
            text-align: center;
            background: none;
            padding: 3px;
            color: #000000;
        }
        QScrollBar::handle:vertical {
            background: #1191d3;
        }
    )");
    impl->currentTable->setStyleSheet(tableStyle);
    impl->currentTable->setHorizontalHeaderLabels({tr("窗口标题"), tr("所属程序"), tr("状态")});
    impl->currentTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    impl->currentTable->setSelectionMode(QAbstractItemView::SingleSelection);
    impl->currentTable->verticalHeader()->hide();
    impl->currentTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    impl->currentTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    impl->currentTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    currentLayout->addWidget(impl->currentTable, 1);

    impl->titleFilter = new QLineEdit(currentCard);
    impl->titleFilter->setMinimumHeight(30);
    impl->titleFilter->setPlaceholderText(tr("标题包含（可选）"));
    impl->titleFilter->setToolTip(tr("留空时，同一程序中具有相同窗口类名的窗口都会匹配。"));
    auto *hideAndSave = new QPushButton(tr("隐藏并保存"), currentCard);
    auto *hideOnce = new QPushButton(tr("仅隐藏一次"), currentCard);
    auto *restoreSelected = new QPushButton(tr("恢复所选窗口"), currentCard);
    currentLayout->addWidget(impl->titleFilter);
    auto *hideActions = new QHBoxLayout();
    hideActions->setSpacing(6);
    hideActions->addWidget(hideAndSave);
    hideActions->addWidget(hideOnce);
    hideActions->addWidget(restoreSelected);
    currentLayout->addLayout(hideActions);
    cardsLayout->addWidget(currentCard, 1);

    auto *rulesCard = new QFrame(this);
    rulesCard->setObjectName(QStringLiteral("fluentCard"));
    auto *rulesLayout = new QVBoxLayout(rulesCard);
    rulesLayout->setContentsMargins(16, 12, 16, 14);
    rulesLayout->setSpacing(8);
    auto *rulesTitle = new QLabel(tr("已保存的规则"), rulesCard);
    rulesTitle->setFont(cardTitleFont);
    rulesLayout->addWidget(rulesTitle);
    impl->rulesTable = new QTableWidget(0, 4, rulesCard);
    impl->rulesTable->setStyleSheet(tableStyle);
    impl->rulesTable->setHorizontalHeaderLabels({tr("所属程序"), tr("窗口类名"), tr("标题包含"), tr("状态")});
    impl->rulesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    impl->rulesTable->setSelectionMode(QAbstractItemView::SingleSelection);
    impl->rulesTable->verticalHeader()->hide();
    impl->rulesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    impl->rulesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    impl->rulesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    impl->rulesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    rulesLayout->addWidget(impl->rulesTable, 1);
    auto *ruleActions = new QHBoxLayout();
    auto *restoreRule = new QPushButton(tr("恢复并停用"), rulesCard);
    auto *enableRule = new QPushButton(tr("启用规则"), rulesCard);
    auto *deleteRule = new QPushButton(tr("删除规则"), rulesCard);
    for (QPushButton *button : {refresh, hideAndSave, hideOnce, restoreSelected,
                                restoreRule, enableRule, deleteRule}) {
        button->setMinimumHeight(30);
        button->setMinimumWidth(100);
    }
    refresh->setMinimumWidth(120);
    ruleActions->setSpacing(6);
    ruleActions->addWidget(restoreRule);
    ruleActions->addWidget(enableRule);
    ruleActions->addWidget(deleteRule);
    rulesLayout->addLayout(ruleActions);
    cardsLayout->addWidget(rulesCard, 1);
    layout->addLayout(cardsLayout, 1);

    auto showError = [this](const QString &message) {
        impl->errorLabel->setText(message);
        impl->errorLabel->setVisible(!message.isEmpty());
    };
    QString error;
    if (!loadRules(&impl->rules, &error)) {
        showError(tr("无法加载窗口规则：%1").arg(error));
        hideAndSave->setEnabled(false);
        restoreRule->setEnabled(false);
        enableRule->setEnabled(false);
        deleteRule->setEnabled(false);
    }

    auto refreshRules = [this] {
        impl->rulesTable->setRowCount(impl->rules.size());
        for (int row = 0; row < impl->rules.size(); ++row) {
            const auto &rule = impl->rules[row];
            setTableItem(impl->rulesTable, row, 0, QFileInfo(rule.exePath).fileName());
            setTableItem(impl->rulesTable, row, 1, rule.className);
            setTableItem(impl->rulesTable, row, 2, rule.titleContains.isEmpty() ? tr("不限") : rule.titleContains);
            setTableItem(impl->rulesTable, row, 3, rule.enabled ? tr("已启用") : tr("已停用"));
        }
    };
    auto refreshCurrent = [this] {
        const auto windows = enumerateWindows();
        impl->currentTable->setRowCount(0);
        for (const auto &window : windows) {
            const quintptr handle = reinterpret_cast<quintptr>(window.handle);
            const bool tracked = impl->hiddenWindows.value(handle, 0) == window.processId;
            bool knownRule = false;
            for (const auto &rule : impl->rules) {
                if (matches(rule, window)) {
                    knownRule = true;
                    break;
                }
            }
            const bool recoverable = impl->recoveryWindows.value(handle, 0) == window.processId;
            if (!window.visible && !tracked && !knownRule && !recoverable)
                continue;
            const int row = impl->currentTable->rowCount();
            impl->currentTable->insertRow(row);
            setTableItem(impl->currentTable, row, 0,
                         window.title.isEmpty() ? tr("无标题窗口") : window.title);
            setTableItem(impl->currentTable, row, 1, QFileInfo(window.exePath).fileName());
            setTableItem(impl->currentTable, row, 2, window.visible ? tr("显示中")
                         : tracked ? tr("已隐藏") : tr("已隐藏（来源未知）"));
            impl->currentTable->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue<qulonglong>(handle));
            impl->currentTable->item(row, 0)->setData(
                Qt::UserRole + 1, QVariant::fromValue<quint32>(static_cast<quint32>(window.processId)));
            impl->currentTable->item(row, 0)->setData(Qt::UserRole + 2, window.exePath);
            impl->currentTable->item(row, 0)->setData(Qt::UserRole + 3, window.className);
            impl->currentTable->item(row, 0)->setToolTip(tr("窗口类名：%1").arg(window.className));
        }
    };
    auto selectedWindow = [this]() -> std::optional<Impl::WindowInfo> {
        const int row = impl->currentTable->currentRow();
        if (row < 0 || !impl->currentTable->item(row, 0))
            return std::nullopt;
        const quintptr handle = static_cast<quintptr>(impl->currentTable->item(row, 0)
                                                        ->data(Qt::UserRole).toULongLong());
        const auto window = inspectWindow(reinterpret_cast<HWND>(handle));
        const auto *item = impl->currentTable->item(row, 0);
        if (!window || window->processId != item->data(Qt::UserRole + 1).toUInt()
            || window->exePath.compare(item->data(Qt::UserRole + 2).toString(), Qt::CaseInsensitive) != 0
            || window->className != item->data(Qt::UserRole + 3).toString())
            return std::nullopt;
        return window;
    };
    auto hideWindow = [this](const Impl::WindowInfo &window) {
        if (!window.visible)
            return true;
        ShowWindow(window.handle, SW_HIDE);
        if (!IsWindowVisible(window.handle)) {
            impl->hiddenWindows.insert(reinterpret_cast<quintptr>(window.handle), window.processId);
            return true;
        }
        return false;
    };
    auto restoreMatches = [this](const Impl::Rule &rule) {
        for (const auto &window : enumerateWindows()) {
            const quintptr handle = reinterpret_cast<quintptr>(window.handle);
            if (!window.visible && matches(rule, window)
                && impl->hiddenWindows.value(handle, 0) == window.processId) {
                bool stillBlocked = false;
                for (const auto &remaining : impl->rules) {
                    if (remaining.enabled && matches(remaining, window)) {
                        stillBlocked = true;
                        break;
                    }
                }
                if (stillBlocked)
                    continue;
                ShowWindow(window.handle, SW_SHOWNA);
                if (IsWindowVisible(window.handle))
                    impl->hiddenWindows.remove(handle);
            }
        }
    };
    auto persist = [this, showError](const QVector<Impl::Rule> &updated) {
        QString saveError;
        if (!saveRules(updated, &saveError)) {
            showError(tr("无法保存窗口规则：%1").arg(saveError));
            return false;
        }
        impl->rules = updated;
        showError({});
        return true;
    };

    connect(refresh, &QPushButton::clicked, this, refreshCurrent);
    connect(hideOnce, &QPushButton::clicked, this, [selectedWindow, hideWindow, refreshCurrent, showError] {
        if (const auto window = selectedWindow()) {
            if (!hideWindow(*window))
                showError(tr("无法隐藏所选窗口。"));
            refreshCurrent();
        }
    });
    connect(hideAndSave, &QPushButton::clicked, this,
            [this, selectedWindow, hideWindow, refreshCurrent, refreshRules, persist, showError] {
        const auto window = selectedWindow();
        if (!window)
            return;
        Impl::Rule rule{window->exePath, window->className, impl->titleFilter->text().trimmed(), true};
        if (!rule.titleContains.isEmpty() && !matches(rule, *window)) {
            showError(tr("所填标题条件与当前窗口不匹配。"));
            return;
        }
        QVector<Impl::Rule> updated = impl->rules;
        bool found = false;
        for (auto &existing : updated) {
            if (existing.exePath.compare(rule.exePath, Qt::CaseInsensitive) == 0
                && existing.className == rule.className
                && existing.titleContains == rule.titleContains) {
                existing.enabled = true;
                found = true;
                break;
            }
        }
        if (!found)
            updated.append(rule);
        if (!persist(updated))
            return;
        if (!hideWindow(*window))
            showError(tr("规则已保存，但无法隐藏所选窗口。"));
        refreshRules();
        refreshCurrent();
    });
    connect(restoreSelected, &QPushButton::clicked, this, [this, selectedWindow, refreshCurrent, showError] {
        if (const auto window = selectedWindow()) {
            const quintptr handle = reinterpret_cast<quintptr>(window->handle);
            if (!window->visible) {
                for (const auto &rule : impl->rules) {
                    if (rule.enabled && matches(rule, *window)) {
                        showError(tr("该窗口有启用中的规则，请先在下方选择“恢复并停用”。"));
                        return;
                    }
                }
                ShowWindow(window->handle, SW_SHOWNA);
                if (IsWindowVisible(window->handle)) {
                    impl->hiddenWindows.remove(handle);
                    impl->recoveryWindows.remove(handle);
                } else {
                    showError(tr("无法恢复所选窗口。"));
                }
                refreshCurrent();
            }
        }
    });
    connect(restoreRule, &QPushButton::clicked, this,
            [this, persist, refreshRules, refreshCurrent, restoreMatches] {
        const int row = impl->rulesTable->currentRow();
        if (row < 0 || row >= impl->rules.size())
            return;
        auto updated = impl->rules;
        updated[row].enabled = false;
        if (!persist(updated))
            return;
        restoreMatches(updated[row]);
        refreshRules();
        refreshCurrent();
    });
    connect(enableRule, &QPushButton::clicked, this,
            [this, persist, refreshRules, refreshCurrent, hideWindow] {
        const int row = impl->rulesTable->currentRow();
        if (row < 0 || row >= impl->rules.size())
            return;
        auto updated = impl->rules;
        updated[row].enabled = true;
        if (!persist(updated))
            return;
        for (const auto &window : enumerateWindows()) {
            if (matches(updated[row], window))
                hideWindow(window);
        }
        refreshRules();
        refreshCurrent();
    });
    connect(deleteRule, &QPushButton::clicked, this,
            [this, persist, refreshRules, refreshCurrent, restoreMatches] {
        const int row = impl->rulesTable->currentRow();
        if (row < 0 || row >= impl->rules.size())
            return;
        auto updated = impl->rules;
        const auto removed = updated.takeAt(row);
        if (!persist(updated))
            return;
        for (const auto &window : enumerateWindows()) {
            const quintptr handle = reinterpret_cast<quintptr>(window.handle);
            if (!window.visible && matches(removed, window)
                && impl->hiddenWindows.value(handle, 0) != window.processId)
                impl->recoveryWindows.insert(handle, window.processId);
        }
        restoreMatches(removed);
        refreshRules();
        refreshCurrent();
    });

    refreshRules();
    refreshCurrent();
    activeTab = this;
    impl->showHook = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, nullptr,
                                     onWindowEvent, 0, 0,
                                     WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
    if (!impl->showHook)
        showError(tr("无法监听新出现的窗口（错误 %1）。").arg(GetLastError()));

    QTimer::singleShot(0, this, [this, hideWindow, refreshCurrent] {
        for (const auto &window : enumerateWindows()) {
            for (const auto &rule : impl->rules) {
                if (rule.enabled && matches(rule, window)) {
                    hideWindow(window);
                    break;
                }
            }
        }
        refreshCurrent();
    });

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this] {
        if (impl->stopped)
            return;
        impl->stopped = true;
        if (impl->showHook) {
            UnhookWinEvent(impl->showHook);
            impl->showHook = nullptr;
        }
        if (activeTab == this)
            activeTab = nullptr;
        for (auto it = impl->hiddenWindows.cbegin(); it != impl->hiddenWindows.cend(); ++it) {
            HWND handle = reinterpret_cast<HWND>(it.key());
            DWORD processId = 0;
            if (IsWindow(handle) && GetWindowThreadProcessId(handle, &processId)
                && processId == it.value())
                ShowWindow(handle, SW_SHOWNA);
        }
        impl->hiddenWindows.clear();
    });
#endif
}

WindowInterceptionTab::~WindowInterceptionTab()
{
#if defined(Q_OS_WIN)
    if (!impl->stopped) {
        if (impl->showHook)
            UnhookWinEvent(impl->showHook);
        if (activeTab == this)
            activeTab = nullptr;
        for (auto it = impl->hiddenWindows.cbegin(); it != impl->hiddenWindows.cend(); ++it) {
            HWND handle = reinterpret_cast<HWND>(it.key());
            DWORD processId = 0;
            if (IsWindow(handle) && GetWindowThreadProcessId(handle, &processId)
                && processId == it.value())
                ShowWindow(handle, SW_SHOWNA);
        }
    }
#endif
}

#if defined(Q_OS_WIN)
void WindowInterceptionTab::handleWindowShown(quintptr handle)
{
    if (impl->stopped)
        return;
    const auto window = inspectWindow(reinterpret_cast<HWND>(handle));
    if (!window || !window->visible)
        return;
    for (const auto &rule : impl->rules) {
        if (rule.enabled && matches(rule, *window)) {
            ShowWindow(window->handle, SW_HIDE);
            if (!IsWindowVisible(window->handle))
                impl->hiddenWindows.insert(handle, window->processId);
            break;
        }
    }
}
#endif
