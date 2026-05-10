//
// Created by JH182 on 2024/9/22.
//

#include "./NetworkRequests.h"
NetworkRequests::NetworkRequests(RequestType type,QUrl url)
{
    req_type = type;
    req_url = url;
}
void NetworkRequests::get(QUrl url)
{
    QNetworkAccessManager *netManager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(url);
    connect(netManager,&QNetworkAccessManager::finished,this,[=](QNetworkReply *reply)
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
            reply_string = QString::fromUtf8(data);
            readJson(data);
        }
    });
    QNetworkReply* reply = netManager->get(request);

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
    connect(naManager,&QNetworkAccessManager::finished,this,[=](QNetworkReply *reply)
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
            reply_string = QString::fromUtf8(data);
            readJson(data);
        }
    });
    QNetworkReply* reply = naManager->post(request,QJsonDocument(data).toJson());
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
    emit finished(json,reply_string,errorString);
}
void NetworkRequests::start(QJsonObject data, QJsonObject headers, QJsonObject cookie, QString ua)
{
    if (req_type == RequestType::GET)
    {
        get(req_url);
    }
    else if (req_type == RequestType::POST)
    {
        post(req_url,data,headers,cookie,ua);
    }
}
void NetworkRequests::operator=(const NetworkRequests &other)
{
    req_type = other.req_type;
    req_url = other.req_url;
}
