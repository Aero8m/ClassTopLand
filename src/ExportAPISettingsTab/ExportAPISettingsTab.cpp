#include "ExportAPISettingsTab.h"
#include "ui_ExportAPISettingsTab.h"
#include "../CloudAPI.h"
#include <QMessageBox>
#include <QDir>
#include <QJsonArray>

ExportAPISettingsTab::ExportAPISettingsTab(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::ExportAPISettingsTabClass())
{
	ui->setupUi(this);
	initAPIList();
	connect(ui->SyncTable, &QPushButton::clicked, this, &ExportAPISettingsTab::SyncExAPITable);
}

ExportAPISettingsTab::~ExportAPISettingsTab()
{
	delete apiListReq;
	delete exApiTableReq;
	delete exApiTokenReq;
	delete ui;
}

void ExportAPISettingsTab::initAPIList() {
	ui->APIList->clear();
	if (apiListReq) {
		delete apiListReq;
	}
	apiListReq = new NetworkRequests(GET,CloudAPIUrl::GET_EXAPI_LIST);
	apiListReq->start();
	connect(apiListReq, &NetworkRequests::finished, this, [this](QJsonObject json, QString, QString errorString) {
		if (!errorString.isEmpty())
		{
			QMessageBox::critical(this, "错误", "请求失败！");
			return;
		}
		QJsonArray apilist = json["data"].toArray();
		for (auto i : apilist) {
			QJsonObject api = i.toObject();
			ui->APIList->addItem(api["name"].toString(),api["id"].toString());
		}
	});

}

void ExportAPISettingsTab::SyncExAPITable() {
	if (ui->APIUser->text().isEmpty() || ui->APIPwd->text().isEmpty()) {
        QMessageBox::critical(this, "错误", "请填写身份验证凭据！");
        return;
	}
	QMessageBox::StandardButton reply = QMessageBox::question(this, "提示", "是否同步课表？此操作将会清除本地课表！", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (reply == QMessageBox::No) {
		return;
	}
		
	QString apiName = ui->APIList->currentText();
	QString apiId = ui->APIList->currentData().toString();
    if (exApiTokenReq) {
        delete exApiTokenReq;
    }
    exApiTokenReq = new NetworkRequests(POST,MAKE_EXAPI_URL(apiId,"get_token"));
    exApiTokenReq->start(QJsonObject{
        {"username",ui->APIUser->text()},
        {"password",ui->APIPwd->text()}
    });
    connect(exApiTokenReq, &NetworkRequests::finished, this, [this,apiId](QJsonObject json, QString, QString errorString) {
        if (!errorString.isEmpty())
        {
            QMessageBox::critical(this, "错误", "请求失败！");
            return;
        }
		QString token = json["token"].toString();
		if (exApiTableReq) {
		    delete exApiTableReq;
		}
		exApiTableReq = new NetworkRequests(POST,MAKE_EXAPI_URL(apiId,"get_classtable"));
        exApiTableReq->start(QJsonObject{
            {"token",token}
        });
		connect(exApiTableReq, &NetworkRequests::finished, this, [this](QJsonObject, QString replyString, QString errorString) {
			if (!errorString.isEmpty())
			{
				QMessageBox::critical(this, "错误", "请求失败！");
				return;
			}
			QFile file(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
			if (file.open(QIODevice::WriteOnly)) {
				file.write(replyString.toUtf8());
				file.close();
				QMessageBox::information(this, "提示", "同步成功！重启后生效！");
			}
			else {
				QMessageBox::critical(this, "错误", "写入文件失败！");
			}
			});
    });
}
