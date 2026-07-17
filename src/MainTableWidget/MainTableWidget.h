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
#include"../NetworkRequests/NetworkRequests.h"
#include<QTranslator>
#include"../AppLog/AppLog.h"
#include<QTimerEvent>
#include <QList>
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
class RefetchTableThread : public QThread{
    Q_OBJECT
public:
    QDateTime getTodayTime(QString str);

    void run();
    bool canShow(QString text) {
        QFont ft("Microsoft YaHei UI",18);// 获取当前字体的格式，里面有文本大小和文本像素大小
        QFontMetrics fm(ft); // 以当前的字体格式为基础
        int textWidthPixel = fm.horizontalAdvance(text); //以当前的字体格式为基础,计算字体的像素宽度
        qDebug() << "textWidthPixel:" << textWidthPixel << "getWidth():" << getWidth() << "getWidth() - 60:" << getWidth() - 60;
        if (textWidthPixel > emit getWidth() - 60) {
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
    void showStatusMessageAS(QList<QString> strList);
    void changeStackedIndex(int idx);
    void addClass(QString text);
    void setClassStyleSheet(int idx,QString styleSheet);
    void toDone();
    void initMainWindowAnimation();
    void windowTop();
    int getWidth();
private:
    bool isProcessRunning(QString& processName);
};
class MainTableWidget : public QWidget
{
    Q_OBJECT

public:
    bool todoIsOpen = false;
    MainTableWidget(QWidget *parent = nullptr);
    ~MainTableWidget();
    void readTimeTable();
    void readConfig();
    void initTodayTable();
    void createActions();
    void createMenu();
    void setStyleSheetFromFile(QWidget* widget,QString file);
    TableEditWidget *editWindow;
    QMenu *trayMenu;
    QAction *showEditAction;
    QAction *exitAppAction;
    QAction *huanKeAction;
    QSystemTrayIcon *sysTrayIcon; //系统托盘
    bool zuanYanIsOpen = false;
    bool todoIsOpenInBack = false;
    RefetchTableThread* refetchThread;
    QString getToken();
    QPropertyAnimation* statusMsgAnimation;
    QPropertyAnimation* hideAnimation;
    QAction* showMainAction;
    QJsonObject config;
    bool isHidden = false;
    int flag = 0;
    NetworkRequests networkReq;

public slots:
    void refetchTableSlot();
    void huanKeSlot();
    void showStatus(QString str);
    void showStatusAutoSelect(QList<QString> strList);
    void initAnimation();
    int onGetWidth() {
        return width();
    };
private slots:
    void on_showMainAction();
    void on_exitAppAction();
    void on_hideWindow();
    void on_getTimer(int &m,int &s);

signals:
    void reText();
private:
    Ui::MainTableWidget *ui;
    void initSysTrayIcon();
    void initSignal();
    void initUi();
    bool timerStart = false;
    QHBoxLayout* classShowWidgetLayout;
    bool windowHidden = false;
    int timerId;
    qint64 minTime = 0;
    qint64 secTime = 0;
    QTimer* topTimer;
    bool isFinished = false;

    static constexpr int SIDEBAR_WIDTH = 154;
    static constexpr int CLASS_BLOCK_SIZE = 49;
    static constexpr int BLOCK_SPACING = 20;
    static constexpr int DEFAULT_WINDOW_WIDTH = 531;
    static constexpr int ANIM_OFFSET_Y = -78;
};


#endif // MAINTABLEWIDGET_H
