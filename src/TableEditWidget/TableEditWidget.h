#ifndef TABLEEDITWIDGET_H
#define TABLEEDITWIDGET_H
#include <QWidget>
#include<QFile>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>
#include<QDir>
#include<QCloseEvent>
#include<QTableWidgetItem>
#include<QtGlobal>
#include<QMessageBox>
#include <qdesktopservices.h>
#include"../AppLog/AppLog.h"
#include <QUrl>
#include<QInputDialog>
#include<QCryptographicHash>
#include "../NetworkRequests/NetworkRequests.h"
#include "../AppendixTableManager/AppendixTableManager.h"
#include "../Utils/Utils.h"
#include "../VERSION.h"
namespace Ui {
class TableEditWidget;
}
class TableEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableEditWidget(QWidget *parent = nullptr);
    void setConfig(QJsonObject obj);
    
    ~TableEditWidget();
    
private:
    void showEvent(QShowEvent* event) override;
    void readTableJson();
    void refechTableWidget(QJsonArray todayTable);
    
    void closeEvent(QCloseEvent* event) override;
    void addItem(QString key);
    QTime getTodayTime(QString str) {
        QString timeString = str;
        QStringList timeList = timeString.split(":");
        int hour = timeList[0].toInt();
        int minute = timeList[1].toInt();
        return QTime(hour, minute);
    }
    
public slots:
    void toggleded();
private slots:
    void on_pushButton_clicked();
    void on_show_AppendixTableManager();
    void on_timerInfo_changed();
    void on_editAppendixTable(QString tableName);
    void on_cellChanged(int row,int column);
    void on_deleteButton_clicked();
signals:
    void refetchTableSignal();

private:
    bool isEditAppendixTable = false;
    QString currentEditAppendixTableName;
    QJsonObject configJson;
    QJsonObject timeTableJson;
    int clickCount = 0;
    NetworkRequests weatherSearchReq;

private:
    Ui::TableEditWidget* ui;
};

#endif // TABLEEDITWIDGET_H
