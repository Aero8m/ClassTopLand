#ifndef TABLEEDITWIDGET_H
#define TABLEEDITWIDGET_H
#include<QAbstractItemView>
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
#include<QTranslator>
#include <qdesktopservices.h>
#include<QListWidgetItem>
//拖拽事件
#include <QDragEnterEvent>
//放下事件
#include <QDropEvent>
#include<QMimeData>
#include <private/qzipwriter_p.h>
#include <private/qzipreader_p.h>
#include<QFileIconProvider>
#include<QTimer>
#include"../AppLog/AppLog.h"
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include<QInputDialog>
#include<QCryptographicHash>
#include<QProcess>
#include "../NetworkRequests/NetworkRequests.h"
#include "../AppendixTableManager/AppendixTableManager.h"
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
    void refechTableWidget(QJsonArray today_table);
    
    void closeEvent(QCloseEvent* event) override;
    void addItem(QString key);
    static bool timesort(QJsonObject& obj1, QJsonObject& obj2);
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

    void on_checkBox_2_clicked(bool checked);

    void on_checkBox_clicked(bool checked);

    void on_chkHide_clicked(bool checked);

    void on_show_AppendixTableManager();
    void on_timerInfo_changed();
    void on_editAppendixTable(QString table_name);
    void _startUpdateTool();
    void on_cellChanged(int row,int column);
signals:
    void refechTable_signal();
    void refechToolBar_signal();

private:
    bool m_bIsEditAppendixTable = false;
    QString m_sCurrentEditAppendixTableName;
    QJsonObject m_joConfig;
    QJsonObject m_joTimeTable;
    int m_iClickcnt = 0;
    NetworkRequests m_rWeatherSearchReq;

private:
    Ui::TableEditWidget* ui;
};

#endif // TABLEEDITWIDGET_H
