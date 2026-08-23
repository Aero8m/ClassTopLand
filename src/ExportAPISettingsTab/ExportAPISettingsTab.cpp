#include "ExportAPISettingsTab.h"
#include "ui_ExportAPISettingsTab.h"
#include "../CloudAPI.h"
#include <QMessageBox>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStringList>
#include <QTime>

namespace {
QTime parseClassTime(const QString &text)
{
    static const QRegularExpression timePattern(
        QStringLiteral(R"(^([01]?\d|2[0-3]):([0-5]\d)$)"));
    const QRegularExpressionMatch match = timePattern.match(text.trimmed());
    if (!match.hasMatch())
    {
        return {};
    }
    return QTime(match.captured(1).toInt(), match.captured(2).toInt());
}

bool validateClassArray(const QJsonArray &classes, const QString &tableName, QString *reason)
{
    for (qsizetype i = 0; i < classes.size(); ++i)
    {
        if (!classes[i].isObject())
        {
            *reason = QString("%1 的第 %2 项不是课程对象").arg(tableName).arg(i + 1);
            return false;
        }

        const QJsonObject course = classes[i].toObject();
        const QString name = course["name"].toString().trimmed();
        const QTime startTime = parseClassTime(course["start"].toString());
        const QTime endTime = parseClassTime(course["end"].toString());
        if (name.isEmpty())
        {
            *reason = QString("%1 的第 %2 节课名称为空").arg(tableName).arg(i + 1);
            return false;
        }
        if (!startTime.isValid() || !endTime.isValid() || startTime >= endTime)
        {
            *reason = QString("%1 的第 %2 节课时间无效").arg(tableName).arg(i + 1);
            return false;
        }
    }
    return true;
}

bool validateClassTable(const QJsonObject &table, QString *reason)
{
    const QStringList dayKeys = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    bool containsDayTable = false;
    for (const QString &day : dayKeys)
    {
        if (!table.contains(day))
        {
            continue;
        }
        containsDayTable = true;
        if (!table[day].isArray())
        {
            *reason = day + " 不是课程数组";
            return false;
        }
        if (!validateClassArray(table[day].toArray(), day, reason))
        {
            return false;
        }
    }

    if (!containsDayTable)
    {
        *reason = "响应中没有任何星期课表";
        return false;
    }

    if (table.contains("appendixTables"))
    {
        if (!table["appendixTables"].isObject())
        {
            *reason = "appendixTables 不是对象";
            return false;
        }
        const QJsonObject appendixTables = table["appendixTables"].toObject();
        for (auto it = appendixTables.begin(); it != appendixTables.end(); ++it)
        {
            if (!it.value().isArray())
            {
                *reason = QString("附加课表 %1 不是课程数组").arg(it.key());
                return false;
            }
            if (!validateClassArray(it.value().toArray(), "附加课表 " + it.key(), reason))
            {
                return false;
            }
        }
    }
    return true;
}
}

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
	apiListReq->start();

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
		
	QString apiId = ui->APIList->currentData().toString();
	if (apiId.isEmpty()) {
		QMessageBox::critical(this, "错误", "请先选择一个可用的导入 API！");
		return;
	}
    if (exApiTokenReq) {
        delete exApiTokenReq;
    }
    exApiTokenReq = new NetworkRequests(POST,MAKE_EXAPI_URL(apiId,"get_token"));
    connect(exApiTokenReq, &NetworkRequests::finished, this, [this,apiId](QJsonObject json, QString, QString errorString) {
        if (!errorString.isEmpty())
        {
            QMessageBox::critical(this, "错误", "请求失败！");
            return;
        }
		QString token = json["token"].toString();
		if (token.isEmpty()) {
			QMessageBox::critical(this, "错误", "服务器未返回有效令牌！");
			return;
		}
		if (exApiTableReq) {
		    delete exApiTableReq;
		}
		exApiTableReq = new NetworkRequests(POST,MAKE_EXAPI_URL(apiId,"get_classtable"));
		connect(exApiTableReq, &NetworkRequests::finished, this, [this](QJsonObject table, QString, QString errorString) {
			if (!errorString.isEmpty())
			{
				QMessageBox::critical(this, "错误", "请求失败！");
				return;
			}
			QString validationError;
			if (!validateClassTable(table, &validationError)) {
				QMessageBox::critical(this, "错误", "服务器返回的课表无效：" + validationError);
				return;
			}

			QSaveFile file(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
			if (file.open(QIODevice::WriteOnly)) {
				const QByteArray tableData = QJsonDocument(table).toJson(QJsonDocument::Indented);
				if (file.write(tableData) == tableData.size() && file.commit()) {
					QMessageBox::information(this, "提示", "同步成功！重启后生效！");
				} else {
					file.cancelWriting();
					QMessageBox::critical(this, "错误", "写入文件失败！");
				}
			}
			else {
				QMessageBox::critical(this, "错误", "写入文件失败！");
			}
			});
        exApiTableReq->start(QJsonObject{
            {"token",token}
        });
    });
    exApiTokenReq->start(QJsonObject{
        {"username",ui->APIUser->text()},
        {"password",ui->APIPwd->text()}
    });
}
