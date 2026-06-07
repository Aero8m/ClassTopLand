//
// Created by jiahang on 25-5-24.
//

#include "Utils.h"
#include "../AppLog/AppLog.h"

std::optional<QJsonObject> readJsonFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        showLog(QString("Failed to open: %1").arg(filePath), LogStatus::ERR);
        return std::nullopt;
    }
    QTextStream stream(&file);
    QString fileStr = stream.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(fileStr.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError || jsonDoc.isNull()) {
        showLog(QString("JSON parse error in %1: %2").arg(filePath, jsonError.errorString()), LogStatus::ERR);
        return std::nullopt;
    }
    return jsonDoc.object();
}

QString getStyleSheet(QString fileName) {
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QTextStream fileText(&file);
    QString styleSheet = fileText.readAll();
    return styleSheet;

}
void loadStyleSheet(QWidget* widget, QString fileName) {
    QFile file(fileName);
    file.open(QFile::ReadOnly);
    QTextStream fileText(&file);
    QString styleSheet = fileText.readAll();
    widget->setStyleSheet(styleSheet);
    file.close();
}

