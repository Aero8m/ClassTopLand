#pragma once

#include <QWidget>
#include "ui_ExportAPISettingsTab.h"
#include "../CloudAPI.h"
#include "../NetworkRequests/NetworkRequests.h"
#include "../AppLog/AppLog.h"
#include<QMessageBox>
#include<QDir>
QT_BEGIN_NAMESPACE
namespace Ui { class ExportAPISettingsTabClass; };
QT_END_NAMESPACE

class ExportAPISettingsTab : public QWidget
{
	Q_OBJECT

public:
	ExportAPISettingsTab(QWidget *parent = nullptr);
	~ExportAPISettingsTab();
	void initAPIList();
private:
	Ui::ExportAPISettingsTabClass *ui;

	NetworkRequests* apiListReq = nullptr;
	NetworkRequests* exApiTableReq = nullptr;
	NetworkRequests* exApiTokenReq = nullptr;
private slots:
	void SyncExAPITable();
};
