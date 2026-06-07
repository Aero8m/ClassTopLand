//
// Created by JH182 on 2024/9/22.
//

#include "./NetworkRequests.h"
NetworkRequests::NetworkRequests(RequestType type,QUrl url)
{
    reqType = type;
    reqUrl = url;
}
void NetworkRequests::get(QUrl url)
{
    QNetworkAccessManager *netManager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(url);
    connect(netManager,&QNetworkAccessManager::finished,this,[=, this](QNetworkReply *reply)
    {
        showLog(QString("Status Code:%1").arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()),INFO);
        if (reply->error())
        {
            showLog("Request Error" + reply->errorString(),ERR);
            errorString = reply->errorString();
        }else
        {
            showLog(QString("Request ok, Reading..."),INFO);
            QByteArray data = reply->readAll();
            replyString = QString::fromUtf8(data);
            readJson(data);
        }
    });
    netManager->get(request);

}
void NetworkRequests::post(QUrl url,QJsonObject data,QJsonObject headers,QJsonObject cookie,QString ua)
{
    QNetworkRequest request;
    QNetworkAccessManager* naManager = new QNetworkAccessManager(this);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    if (!headers.isEmpty())
    {
        for(auto x = headers.begin(); x != headers.end(); x++)
        {
            request.setRawHeader(x.key().toUtf8(),x.value().toString().toUtf8());
        }
    }
    if (!cookie.isEmpty())
    {
        request.setHeader(QNetworkRequest::CookieHeader,cookie);
    }
    request.setRawHeader("User-Agent",ua.toUtf8());
    request.setUrl(url);
    connect(naManager,&QNetworkAccessManager::finished,this,[=, this](QNetworkReply *reply)
    {
        if (reply->error())
        {
            showLog(QString("Request Error %1：").arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) + reply->errorString(),ERR);
            errorString = reply->errorString();
            return;
        }else
        {
            showLog(QString("Request %1 ok, Reading...").arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()));
            QByteArray data = reply->readAll();
            replyString = QString::fromUtf8(data);
            readJson(data);
        }
    });
    naManager->post(request, QJsonDocument(data).toJson());
}
void NetworkRequests::readJson(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data,&error);
    if(document.isNull())
    {
        showLog(QString("JSON Parse Error" + error.errorString()));
    }else
    {
        json = document.object();
    }
    isFinished = true;
    emit finished(json,replyString,errorString);
}
void NetworkRequests::start(QJsonObject data, QJsonObject headers, QJsonObject cookie, QString ua)
{
    if (reqType == RequestType::GET)
    {
        get(reqUrl);
    }
    else if (reqType == RequestType::POST)
    {
        post(reqUrl,data,headers,cookie,ua);
    }
}
void NetworkRequests::operator=(const NetworkRequests &other)
{
    json = other.json;
    replyString = other.replyString;
    reqType = other.reqType;
    reqUrl = other.reqUrl;
    errorString = other.errorString;
    isFinished = other.isFinished;
}
