//
// Created by JH182 on 2024/9/22.
//

#include "./NetworkRequests.h"
NetworkRequests::NetworkRequests(RequestType type,QUrl url)
{
    reqType = type;
    reqUrl = url;
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished,
            this, &NetworkRequests::handleReply);
}
void NetworkRequests::get(QUrl url)
{
    QNetworkRequest request;
    request.setUrl(url);
    manager->get(request);

}
void NetworkRequests::post(QUrl url,QJsonObject data,QJsonObject headers,QJsonObject cookie,QString ua)
{
    QNetworkRequest request;
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
    manager->post(request, QJsonDocument(data).toJson());
}

void NetworkRequests::handleReply(QNetworkReply *reply)
{
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QByteArray data = reply->readAll();
    replyString = QString::fromUtf8(data);

    if (reply->error() != QNetworkReply::NoError)
    {
        errorString = QString("HTTP %1: %2").arg(statusCode).arg(reply->errorString());
        showLog("Request Error: " + errorString, ERR);
        emit finished({}, replyString, errorString);
        reply->deleteLater();
        return;
    }

    showLog(QString("Request %1 ok, Reading...").arg(statusCode), INFO);
    readJson(data);
    reply->deleteLater();
}

void NetworkRequests::readJson(QByteArray data)
{
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(data,&error);
    if (error.error != QJsonParseError::NoError || document.isNull())
    {
        errorString = "JSON Parse Error: " + error.errorString();
        json = {};
        showLog(errorString, ERR);
    }
    else if (!document.isObject())
    {
        errorString = "JSON Parse Error: response root is not an object";
        json = {};
        showLog(errorString, ERR);
    }
    else
    {
        errorString.clear();
        json = document.object();
    }
    emit finished(json,replyString,errorString);
}
void NetworkRequests::start(QJsonObject data, QJsonObject headers, QJsonObject cookie, QString ua)
{
    json = {};
    replyString.clear();
    errorString.clear();
    if (reqType == RequestType::GET)
    {
        get(reqUrl);
    }
    else if (reqType == RequestType::POST)
    {
        post(reqUrl,data,headers,cookie,ua);
    }
}
