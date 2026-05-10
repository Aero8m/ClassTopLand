#ifndef MAINTABLEWIDGET_H
#define MAINTABLEWIDGET_H
#include <QScreen>
#include <QWidget>
#include<QJsonObject>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonValue>
#include<QJsonParseError>
#include<QMessageBox>
#include <QTimer>
#include<QThread>
#include<QPropertyAnimation>
#include<QEasingCurve>
#include<QAbstractAnimation>
#include<QRect>
#include<QAbstractAnimation>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include<QHBoxLayout>
#include<QProcess>
#include <QCloseEvent>
#include "../TableEditWidget/TableEditWidget.h"
#include "../YiyanDialog/YiyanDialog.h"
#include "../MainWindow/MainWindow.h"
#include"../NetworkRequests/NetworkRequests.h"
#include<QTranslator>
#include"../AppLog/AppLog.h"
#include<QTimerEvent>

#include"../GlassHelper/GlassHelper.h"
#ifdef WIN32
#include<qt_windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif
QT_BEGIN_NAMESPACE

namespace Ui {
class MainTableWidget;

}
QT_END_NAMESPACE
class refechTableThread:public QThread{
    Q_OBJECT
public:
    QDateTime getTodayTime(QString str);

    void run();
    bool canShow(QString text) {
        QFont ft("Microsoft YaHei UI",18);// 获取当前字体的格式，里面有文本大小和文本像素大小
        QFontMetrics fm(ft); // 以当前的字体格式为基础
        int text_wpixel = fm.horizontalAdvance(text); //以当前的字体格式为基础,计算字体的像素宽度
        qDebug() << "text_wpixel:" << text_wpixel << "getWidth():" << getWidth() << "getWidth() - 60:" << getWidth() - 60;
        if (text_wpixel > emit getWidth() - 60) {
            return false;
        }
        else {
            return true;
        }
    }
    bool stopFlag = false;
signals:
    void setTable(QString str);
    void repaint();
    void tst(QString text);
    void showStatusMessage(QString str);
    void changeStackedIndex(int idx);
    void addClass(QString text);
    void setClassStyleSheet(int idx,QString styleSheet);
    void toDone();
    void initMainWindowAnimation();
    void windowTop();
    int getWidth();
private:
    bool WhetherProcessRunning(QString& processName);
};
class MainTableWidget : public QWidget
{
    Q_OBJECT

public:
    bool TodoisOpen=false;
    MainTableWidget(QWidget *parent = nullptr);
    ~MainTableWidget();
    void readTimeTable();
    void readConfig();
    void initTodayTable();
    void createActions();
    void createMenu();
    void setStyleSheetFromFile(QWidget* widget,QString file);
    void showWindow(int scr_w, int scr_h);
    TableEditWidget *EditWindow;
    QMenu *tray_menu;
    QAction *m_showedit;
    QAction *m_gongde;
    QAction *m_exitApp;
    QAction *m_hidewindow;
    QSystemTrayIcon *m_sysTrayIcon; //系统托盘
    bool ZuanYanisOpen = false;
    bool TodoisOpeninBack = false;
    refechTableThread* rtt;
    QString getToken();
    QPropertyAnimation* status_msg_animation;
    QPropertyAnimation* hide_animation;
    QPropertyAnimation* timer_animation;
    QPropertyAnimation* window_show_animation;
    QAction* m_showmain;
    QJsonObject Config;
    bool ishide = false;
    int flag = 0;
    NetworkRequests req;

public slots:
    void refechTable_slot();
    void do_repaint();
    void startMainWindow();
    void on_showConfig_modal();
    void hk_slot(QString day);
    void showStatus(QString str);
    void initAnimation();
    // void do_pss();
    // void do_pst();
    // void do_tss();
    // void do_tst();
    void on_startTimer(QString timer_str);
    void on_stopTimer();
    void on_timerisStart(bool &st);
    int on_getWidth() {
        return width();
    };
private slots:
    void on_showMainAction();
    void on_exitAppAction();
    void on_hideWindow();
    void on_showTimer();
    void on_getTimer(int &m,int &s);

signals:
    void showTimer();
    void reText();
private:
    Ui::MainTableWidget *ui;
    void initSysTrayIcon();
    void initSignal();
    void initUi();
    bool timerStart = false;
    QHBoxLayout* class_show_widget_layout;
    bool WindowHide = false;
    int timer_id;
    qint64 min_time=0;
    qint64 sec_time=0;
    QTimer* topTimer;
    void timerEvent(QTimerEvent *event);
    int scr_w;
    int scr_h;
    bool isFinish = false;
    
};


#endif // MAINTABLEWIDGET_H
