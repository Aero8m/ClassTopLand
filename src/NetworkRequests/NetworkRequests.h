//
// Created by JH182 on 2024/9/22.
//

#ifndef NETWORKREQUESTS_H
#define NETWORKREQUESTS_H
#include<QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include<QJsonArray>
#include<QJsonObject>
#include"../AppLog/AppLog.h"

enum RequestType {GET, POST};
class NetworkRequests : public QObject{
    Q_OBJECT
    public:
    NetworkRequests(RequestType type=GET,QUrl url=QUrl());
    QJsonObject json;
    QString reply_string;
    RequestType req_type;
    QUrl req_url;
    QString errorString;
    bool isFinished = false;
    bool hasError()
    {
        return !errorString.isEmpty();
    }
    void operator=(const NetworkRequests &other);
    void start(QJsonObject data = QJsonObject(),QJsonObject headers=QJsonObject(),QJsonObject cookie = QJsonObject(),QString ua = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36 Edg/127.0.0.0");
    private:
    void get(QUrl url);
    void post(QUrl url,QJsonObject data,QJsonObject headers=QJsonObject(),QJsonObject cookie = QJsonObject(),QString ua = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/127.0.0.0 Safari/537.36 Edg/127.0.0.0");
    void readJson(QByteArray data);
    signals:
    void finished(QJsonObject json,QString reply_string,QString error_string);
};



#endif //NETWORKREQUESTS_H
