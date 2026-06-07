#ifndef API_H

#define API_H
#include<QString>
#include<QStandardPaths>
#include<QDir>
#include<QJsonObject>
#include<QJsonArray>
#include<QCoreApplication>

QString TABLE_JSON = QDir::homePath() + "/ClassTopLand_Data" + "/tables.json";
QString CONFIG_JSON = QDir::homePath() + "/ClassTopLand_Data" + "/config.json";

QString zuanDb = QDir::homePath() + "/ClassTopLand_Data" + "/zuan.db";
QDir zuanDbDir(TABLE_JSON);
QJsonObject timeTable;
QJsonArray todayTable;
#endif // API_H
