//
// Created by JH182 on 2024/9/22.
//

// You may need to build the project (run Qt uic code generator) to get "ui_SyncingDialog.h" resolved

#include "./SyncingDialog.h"



#include "ui_SyncingDialog.h"


SyncingDialog::SyncingDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::SyncingDialog) {
    ui->setupUi(this);
    setWindowTitle("同步窗口");
    setFixedSize(width(),height());
    setWindowFlags(Qt::SplashScreen);

}

SyncingDialog::~SyncingDialog() {
    delete ui;
}
void SyncingDialog::syncRemoteJson()
{
    show();
    readConfig();
    QTime _Timer = QTime::currentTime().addMSecs(3000);
    while( QTime::currentTime() < _Timer )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QString req_url = QString("http://%1:%2/get_allday?class_name=%3").arg(config["sync_server"].toString()).arg(config["sync_server_port"].toInt()).arg(config["class_name"].toString());
    req = NetworkRequests(GET,QUrl(req_url));
    req.start();
    connect(&req,&NetworkRequests::finished,this,[=](QJsonObject json,QString text,QString error_string)
    {
        if (error_string != "")
        {
            QMessageBox::critical(this,"错误","服务器不可用，请重新配置！");
            close();
            return;
        }
        qDebug() << json;
        QJsonDocument doc;
        doc.setObject(json);
        QFile file(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            showLog("JsonOpen Error!",ERR);
            return;

        }

        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        // 写入文件
        stream << doc.toJson();
        file.close();
        close();
        return;
    });


}
void SyncingDialog::readConfig()
{
    QFile file(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    file.open(QIODevice::ReadWrite | QIODevice::Text);

    QTextStream stream(&file);
    QString file_str = stream.readAll();
    file.close();
    QJsonParseError jsonError;
    QJsonDocument jsondoc = QJsonDocument::fromJson(file_str.toUtf8(),&jsonError);
    if (jsonError.error != QJsonParseError::NoError && !jsondoc.isNull()) {
        showLog("Config.json is Error!",ERR);
    }
    config = jsondoc.object();
}
