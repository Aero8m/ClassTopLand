//
// Created by jiahang on 25-5-24.
//

#ifndef UTILS_H
#define UTILS_H

#include<QString>
#include<QFile>
#include<QTextStream>
#include<QWidget>
#include<QJsonObject>
#include<QJsonDocument>
#include<QJsonParseError>
#include <optional>

QString getStyleSheet(QString fileName);
void loadStyleSheet(QWidget* widget, QString fileName);
std::optional<QJsonObject> readJsonFile(const QString &filePath);

#endif //UTILS_H
