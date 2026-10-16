#include "ConfigSettings.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>

namespace {
QString configPath()
{
    return QDir::homePath() + QStringLiteral("/ClassTopLand_Data/config.json");
}
}

bool ConfigSettings::read(QJsonObject *config, QString *error)
{
    *config = {};
    error->clear();
    QFile file(configPath());
    if (!file.exists()) {
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        *error = QCoreApplication::translate("ConfigSettings", "无法读取配置：%1").arg(file.errorString());
        return false;
    }
    const QByteArray contents = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        *error = QCoreApplication::translate("ConfigSettings", "读取配置失败：%1").arg(file.errorString());
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(contents, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        *error = QCoreApplication::translate("ConfigSettings", "配置格式无效，请检查 config.json。%1")
                     .arg(parseError.error == QJsonParseError::NoError ? QString{} : parseError.errorString());
        return false;
    }
    const QJsonObject object = document.object();
    if (object.contains(alwaysOnTopKey) && !object.value(alwaysOnTopKey).isBool()) {
        *error = QCoreApplication::translate("ConfigSettings", "config.json 中的课程条置顶设置必须为布尔值。");
        return false;
    }
    *config = object;
    return true;
}

bool ConfigSettings::mergeAndSave(const QJsonObject &changes, QJsonObject *savedConfig, QString *error)
{
    QJsonObject config;
    if (!read(&config, error)) {
        return false;
    }
    for (auto it = changes.constBegin(); it != changes.constEnd(); ++it) {
        config.insert(it.key(), it.value());
    }
    if (!QDir().mkpath(QDir::homePath() + QStringLiteral("/ClassTopLand_Data"))) {
        *error = QCoreApplication::translate("ConfigSettings", "无法创建配置目录。");
        return false;
    }
    QSaveFile file(configPath());
    if (!file.open(QIODevice::WriteOnly)) {
        *error = QCoreApplication::translate("ConfigSettings", "无法保存配置：%1").arg(file.errorString());
        return false;
    }
    const QByteArray contents = QJsonDocument(config).toJson(QJsonDocument::Indented);
    if (file.write(contents) != contents.size()) {
        *error = QCoreApplication::translate("ConfigSettings", "写入配置失败：%1").arg(file.errorString());
        file.cancelWriting();
        return false;
    }
    if (!file.commit()) {
        *error = QCoreApplication::translate("ConfigSettings", "提交配置失败：%1").arg(file.errorString());
        return false;
    }
    *savedConfig = config;
    return true;
}
