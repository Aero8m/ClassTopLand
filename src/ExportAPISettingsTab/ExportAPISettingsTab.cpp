#include "ExportAPISettingsTab.h"

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
	delete ui;
}

void ExportAPISettingsTab::initAPIList() {
	ui->APIList->clear();
	if (apilist_req) {
		delete apilist_req;
	}
	apilist_req = new NetworkRequests(GET,CloudAPIUrl::GET_EXAPI_LIST);
	apilist_req->start();
	connect(apilist_req, &NetworkRequests::finished, this, [=](QJsonObject json, QString reply_string, QString error_string) {
		if (!error_string.isEmpty())
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
		
	QString apiname = ui->APIList->currentText();
	QString apiid = ui->APIList->currentData().toString();
    if (exapitk_req) {
        delete exapitk_req;
    }
    exapitk_req = new NetworkRequests(POST,MAKE_EXAPI_URL(apiid,"get_token"));
    exapitk_req->start(QJsonObject{
        {"username",ui->APIUser->text()},
        {"password",ui->APIPwd->text()}
    });
    connect(exapitk_req, &NetworkRequests::finished, this, [=](QJsonObject json, QString reply_string, QString error_string) { 
        if (!error_string.isEmpty())
        {
            QMessageBox::critical(this, "错误", "请求失败！");
            return;
        }
		QString token = json["token"].toString();
		if (exapitb_req) {
		    delete exapitb_req;
		}
		exapitb_req = new NetworkRequests(POST,MAKE_EXAPI_URL(apiid,"get_classtable"));
        exapitb_req->start(QJsonObject{
            {"token",token}
        });
		connect(exapitb_req, &NetworkRequests::finished, this, [=](QJsonObject json, QString reply_string, QString error_string) {
			if (!error_string.isEmpty())
			{
				QMessageBox::critical(this, "错误", "请求失败！");
				return;
			}
			QFile file(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
			if (file.open(QIODevice::WriteOnly)) {
				file.write(reply_string.toUtf8());
				file.close();
				QMessageBox::information(this, "提示", "同步成功！重启后生效！");
			}
			else {
				QMessageBox::critical(this, "错误", "写入文件失败！");
			}
			});
    });
}