#pragma once

#include <QWidget>
#include "../NetworkRequests/NetworkRequests.h"
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
