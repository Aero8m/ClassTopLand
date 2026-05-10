//
// Created by JH182 on 2024/9/22.
//

#ifndef SYNCINGDIALOG_H
#define SYNCINGDIALOG_H

#include <QDialog>
#include<QJsonArray>
#include<QJsonObject>
#include<QFile>
#include<QJsonDocument>
#include<QMessageBox>
#include<QDir>
#include"../AppLog/AppLog.h"
#include"../NetworkRequests/NetworkRequests.h"
#include <QMouseEvent>
QT_BEGIN_NAMESPACE
namespace Ui { class SyncingDialog; }
QT_END_NAMESPACE

class SyncingDialog : public QDialog {
Q_OBJECT

public:
    explicit SyncingDialog(QWidget *parent = nullptr);
    ~SyncingDialog() override;
    void syncRemoteJson();
private:
    NetworkRequests req;
    void readConfig();
    QJsonObject config;
    Ui::SyncingDialog *ui;
};


#endif //SYNCINGDIALOG_H
