#ifndef MAINTABLEWIDGET_H
#define MAINTABLEWIDGET_H
#include <QScreen>
#include <QWidget>
#include<QJsonObject>
#include<QJsonDocument>
#include<QJsonArray>
#include<QJsonValue>
#include <QTimer>
#include<QThread>
#include<QPropertyAnimation>
#include<QEasingCurve>
#include<QRect>
#include<QAbstractAnimation>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include<QHBoxLayout>
#include "../TableEditWidget/TableEditWidget.h"
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
    QDateTime getTodayTime(QString str, QDate date = QDate::currentDate());

    void run();
    void setTodayTable(const QJsonArray &today_table)
    {
        Q_ASSERT(!isRunning());
        todayTable = today_table;
    }
signals:
    void tst(QString text);
    void showStatusMessage(QString str);
    void showStatusMessageAS(QList<QString> strList);
    void changeStackedIndex(int idx);
    void addClass(QString text);
    void setClassStyleSheet(int idx,QString styleSheet);
    void toDone();
    void setDoneTabText(QString text);
    void initMainWindowAnimation();
private:
    QJsonArray todayTable;
};
class MainTableWidget : public QWidget
{
    Q_OBJECT

public:
    MainTableWidget(QWidget *parent = nullptr);
    ~MainTableWidget();


public slots:
    void refetchTableSlot();
    void huanKeSlot();
    void showStatus(QString str);
    void showStatusAutoSelect(QList<QString> strList);
    void initAnimation();
private slots:
    void on_showMainAction();
    void on_hideWindow();
    void restartApplication();
private:
    Ui::MainTableWidget *ui;
    void initSysTrayIcon();
    void initSignal();
    void initUi();
    void stopRefetchThread();
    int expandedWindowWidth() const;
    int collapsedWindowWidth() const;
    QHBoxLayout* classShowWidgetLayout;
    bool windowHidden = false;
    QTimer* topTimer;
    bool isFinished = false;
    QJsonObject timeTable;
    QJsonArray todayTable;
    QTimer* restartTimer;
    QDate runningDate;
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
    QAction* restartAppAction;
    QSystemTrayIcon *sysTrayIcon; //系统托盘
    RefetchTableThread* refetchThread;
    QPropertyAnimation* statusMsgAnimation = nullptr;
    QPropertyAnimation* hideAnimation = nullptr;
    QJsonObject config;

    static constexpr int SIDEBAR_WIDTH = 154;
    static constexpr int CLASS_BLOCK_SIZE = 49;
    static constexpr int BLOCK_SPACING = 20;
    static constexpr int DEFAULT_WINDOW_WIDTH = 531;
    static constexpr int ANIM_OFFSET_Y = -78;
};


#endif // MAINTABLEWIDGET_H
