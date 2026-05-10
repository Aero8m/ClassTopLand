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

QString ZUAN_DB = QDir::homePath() + "/ClassTopLand_Data" + "/zuan.db";
QDir ZUAN_DB_QDOBJECT(TABLE_JSON);
QJsonObject time_table;
QJsonArray today_table;
#endif // API_H
