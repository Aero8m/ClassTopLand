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
#include <QPainter>
#include <QProxyStyle>

// class CustomTabStyle : public QProxyStyle
// {
// public:
//     QSize sizeFromContents(ContentsType type, const QStyleOption *option,
//         const QSize &size, const QWidget *widget) const
//     {
//         QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
//         if (type == QStyle::CT_TabBarTab) {
//             s.transpose();
//             s.rwidth() = 90; // 设置每个tabBar中item的大小
//             s.rheight() = 44;
//         }
//         return s;
//     }
//
//     void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
//     {
//         if (element == CE_TabBarTabLabel) {
//             if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
//                 QRect allRect = tab->rect;
//
//                 if (tab->state & QStyle::State_Selected) {
//                     painter->save();
//                     painter->setPen(0x89cfff);
//                     painter->setBrush(QBrush(0x89cfff));
//                     painter->drawRect(allRect.adjusted(6, 6, -6, -6));
//                     painter->restore();
//                 }
//                 QTextOption option;
//                 option.setAlignment(Qt::AlignCenter);
//                 if (tab->state & QStyle::State_Selected) {
//                     painter->setPen(0xf8fcff);
//                 }
//                 else {
//                     painter->setPen(0x5d5d5d);
//                 }
//
//                 painter->drawText(allRect, tab->text, option);
//                 return;
//             }
//         }
//
//         if (element == CE_TabBarTab) {
//             QProxyStyle::drawControl(element, option, painter, widget);
//         }
//     }
// };
class TableEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TableEditWidget(QWidget *parent = nullptr);
    void setConfig(QJsonObject obj);
    
    ~TableEditWidget();
    bool eventFilter(QObject *watched,QEvent* e) override;
    
private:
    void showEvent(QShowEvent* event) override;
    void readTableJson();
    void refechTableWidget(QJsonArray today_table);
    
    void closeEvent(QCloseEvent* event) override;
    void addItem(QString key);
    void readPluginList();
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

    void _searchWeatherLocaltions();
    void saveWeatherLocaltions();

    void on_pluginList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_pluginList_itemDoubleClicked(QListWidgetItem *item);

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
