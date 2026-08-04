#include "./MainTableWidget.h"
#include "ui_MainTableWidget.h"
#include "../Utils/Utils.h"

void asyncSleep(unsigned int msec)
{
    QEventLoop loop;
    QTimer::singleShot(msec, &loop, &QEventLoop::quit);
    loop.exec();
}


MainTableWidget::MainTableWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainTableWidget)
{
    ui->setupUi(this);
    editWindow = new TableEditWidget();
    refetchThread = new RefetchTableThread();
    readConfig();
    readTimeTable();
    
    editWindow->setConfig(config);

    initUi();

    topTimer = new QTimer();

#ifdef WIN32
    connect(topTimer, &QTimer::timeout, this, [=, this]
    {
            SetWindowPos(HWND(this->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    });
    topTimer->start(3000);
#endif
    initSignal();

    refetchThread->start();
    initSysTrayIcon();

}

MainTableWidget::~MainTableWidget()
{
    if (refetchThread) {
        refetchThread->stopFlag = true;
        if (!refetchThread->wait(3000)) {
            refetchThread->terminate();
            refetchThread->wait();
        }
        delete refetchThread;
    }
    if (topTimer) {
        topTimer->stop();
        delete topTimer;
    }
    if (editWindow) {
        delete editWindow;
    }
    delete ui;
}
void MainTableWidget::showStatusAutoSelect(QList<QString> strList)
{
    if (strList.isEmpty())
        return;

    if (!statusMsgAnimation)
        return;

    if (windowHidden)
    {
        on_hideWindow();
        asyncSleep(700);
    }

    for (int i = 0; i < strList.size(); ++i)
    {
        QString text = strList.at(i);
        QFont ft("Microsoft YaHei UI", 18);
        QFontMetrics fm(ft);
        int textWidthPixel = fm.horizontalAdvance(text);
        if (textWidthPixel <= width() - 60)
        {
            showStatus(text);
            return;
        }
    }
    showStatus(strList.last());
}

void MainTableWidget::showStatus(QString str)
{
    if (!statusMsgAnimation)
        return;

    if (windowHidden)
    {
        on_hideWindow();
        asyncSleep(700);
    }

    ui->label_3->setText(str);
    statusMsgAnimation->setEasingCurve(QEasingCurve::OutExpo);
    statusMsgAnimation->setStartValue(QRect(width()/2, ANIM_OFFSET_Y, CLASS_BLOCK_SIZE, CLASS_BLOCK_SIZE));
    statusMsgAnimation->setEndValue(QRect(0,0,width(),49));
    statusMsgAnimation->setDirection(QAbstractAnimation::Forward);
    statusMsgAnimation->start();
    QTimer::singleShot(5000, this, [this]()
    {
        //statusMsgAnimation->setEasingCurve(QEasingCurve::InOutSine);
        statusMsgAnimation->setStartValue(QRect(width()/2, ANIM_OFFSET_Y, CLASS_BLOCK_SIZE, CLASS_BLOCK_SIZE));
        statusMsgAnimation->setEndValue(QRect(0,0,width(),49));
        statusMsgAnimation->setDirection(QAbstractAnimation::Backward);
        statusMsgAnimation->start();
    });
}
void MainTableWidget::initAnimation()
{
    QScreen *screen = qApp->primaryScreen();
    int screenW = screen->size().width();
    move((screenW - width()) / 2, 0);
    move((screenW - width()) / 2, 0);
    // 信息显示动画
    statusMsgAnimation = new QPropertyAnimation(ui->status_show,"geometry",this);
    statusMsgAnimation->setDuration(500);
    statusMsgAnimation->setEasingCurve(QEasingCurve::OutExpo);
    statusMsgAnimation->setStartValue(QRect(width()/2, ANIM_OFFSET_Y, CLASS_BLOCK_SIZE, CLASS_BLOCK_SIZE));
    statusMsgAnimation->setEndValue(QRect(0,0,width(),49));
    // 窗口隐藏/显示动画
    hideAnimation = new QPropertyAnimation(this,"pos",this);
    hideAnimation->setDuration(700);
    hideAnimation->setEasingCurve(QEasingCurve::InOutExpo);
    hideAnimation->setStartValue(pos());
    hideAnimation->setEndValue(QPoint(screenW - SIDEBAR_WIDTH, 0));

}

void MainTableWidget::initUi(){
    setWindowFlags(Qt::WindowType::FramelessWindowHint | Qt::WindowType::WindowStaysOnTopHint | Qt::WindowType::Tool | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground,true);
    setAttribute(Qt::WidgetAttribute::WA_ShowWithoutActivating,true);
    

    GlassHelper::enableBlurBehind(this);

    classShowWidgetLayout = new QHBoxLayout();
    classShowWidgetLayout->setContentsMargins(0,0,0,0);
    classShowWidgetLayout->setSpacing(0);
    ui->class_show_widget->setLayout(classShowWidgetLayout);
}
void MainTableWidget::on_hideWindow()
{
    if (isFinished || !hideAnimation)
    {
        return;
    }
    if (!windowHidden)
    {
        hideAnimation->setDirection(QAbstractAnimation::Forward);
        hideAnimation->start();
        QTimer::singleShot(hideAnimation->duration() / 2, this, [=, this] {
            ui->class_show_widget->hide();
            resize(SIDEBAR_WIDTH, height());
            ui->stackedWidget->resize(SIDEBAR_WIDTH, height());
            ui->status_show->resize(SIDEBAR_WIDTH, height());
            windowHidden = true;
        });
    }else
    {
        hideAnimation->setDirection(QAbstractAnimation::Backward);
        hideAnimation->start();
        QTimer::singleShot(hideAnimation->duration() / 2, this, [=, this] {
            ui->class_show_widget->show();
            resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE, height());
            ui->stackedWidget->resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE, height());
            ui->status_show->resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE, height());
            windowHidden = false;
        });
    }
}
void MainTableWidget::initSignal(){

    connect(editWindow, &TableEditWidget::refetchTableSignal, this, &MainTableWidget::refetchTableSlot);
    connect(refetchThread,&RefetchTableThread::tst,ui->class_time,&QLabel::setText,Qt::QueuedConnection);
    connect(refetchThread,&RefetchTableThread::showStatusMessage,this,&MainTableWidget::showStatus,Qt::QueuedConnection);
    connect(refetchThread,&RefetchTableThread::showStatusMessageAS,this,&MainTableWidget::showStatusAutoSelect,Qt::QueuedConnection);
    connect(refetchThread,&RefetchTableThread::changeStackedIndex,this,[=, this](int idx)
    {
        ui->stackedWidget->setCurrentIndex(idx);
    },Qt::QueuedConnection);
    connect(refetchThread,&RefetchTableThread::addClass,this,[=, this](QString text)
    {
        QLabel *classLabel = new QLabel(ui->class_show_widget);
        classShowWidgetLayout->addWidget(classLabel);
        classLabel->setText(text);
        classLabel->setFixedSize(CLASS_BLOCK_SIZE, CLASS_BLOCK_SIZE);
        QFont font("Microsoft YaHei UI",16);
        classLabel->setFont(font);
        classLabel->setStyleSheet("color: black;");
        classLabel->setAlignment(Qt::AlignCenter);

    },Qt::QueuedConnection);
    connect(refetchThread,&RefetchTableThread::setClassStyleSheet,this,[=, this](int idx,QString styleSheet)
    {
        QList<QLabel*> classList = ui->class_show_widget->findChildren<QLabel*>();
        classList[idx]->setStyleSheet(styleSheet);
    },Qt::QueuedConnection);
    connect(refetchThread,&RefetchTableThread::toDone,this,[=, this]
    {
        isFinished = true;
        resize(DEFAULT_WINDOW_WIDTH, height());
        ui->stackedWidget->resize(DEFAULT_WINDOW_WIDTH, height());
        ui->status_show->resize(DEFAULT_WINDOW_WIDTH, height());
        QScreen *screen = qApp->primaryScreen();
        int screenW = screen->size().width();
        move((screenW - width()) / 2, 0);

    },Qt::QueuedConnection);
    connect(refetchThread,&RefetchTableThread::initMainWindowAnimation,this,&MainTableWidget::initAnimation,Qt::QueuedConnection);
    connect(ui->hide_window,&QPushButton::clicked,this,&MainTableWidget::on_hideWindow);
}
void MainTableWidget::readTimeTable(){
    auto result = readJsonFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    if (!result) return;
    timeTable = *result;
    initTodayTable();
}
void MainTableWidget::readConfig(){
    auto result = readJsonFile(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    if (!result) return;
    config = *result;
}
void MainTableWidget::initTodayTable(){
    QDateTime currentDateTime = QDateTime::currentDateTime();
    todayTable = timeTable.value(currentDateTime.toString("ddd")).toArray();
    refetchThread->setTodayTable(todayTable);
    resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());
    ui->stackedWidget->resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());
    ui->status_show->resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());
}
void RefetchTableThread::run(){
    bool classStarted = false;
    if (todayTable.count() <= 0)
    {
        emit changeStackedIndex(0);
    }
    for (int i=0;i<todayTable.count();i++)
    {
        emit addClass(QString(todayTable[i].toObject()["name"].toString().at(0)));
    }
    emit changeStackedIndex(1);

    emit initMainWindowAnimation();
    QJsonObject nullClass = { {"start","00:00"},{"end","00:00"} };
    for(int idx = 0;idx < todayTable.count();)
    {
        if (stopFlag) {
            stopFlag = false;
            return;
        }

        QDate today = QDate::currentDate();
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QJsonObject currentClass = todayTable[idx].toObject();
        QJsonObject nextClass = idx < todayTable.count() - 1 ? todayTable[idx + 1].toObject() : nullClass;
        QDateTime currentClassStartTime = getTodayTime(currentClass.value("start").toString(), today);
        QDateTime currentClassEndTime = getTodayTime(currentClass.value("end").toString(), today);
        QDateTime nextClassStartTime = getTodayTime(nextClass.value("start").toString(), today);
        if (currentDateTime.secsTo(currentClassEndTime)> 0) { // 当前时间 - 当前课程下课时间 >= 0 (上课中)
            if (idx > 0) emit setClassStyleSheet(idx - 1, "color: black;"); //去除上一节课的边框
            emit setClassStyleSheet(idx, "border-width: 0px 0px 4px 0px; border-color:#1191d3; border-style: solid; color: black;");
            if (currentDateTime.secsTo(currentClassStartTime) == 0 and !classStarted) {  // 当前时间 - 当前课程开始时间 == 0 (刚开始上课)
                classStarted = true;
                emit showStatusMessageAS({QString("%1 已经上课，请回到座位").arg(currentClass["name"].toString()),
                        QString("%1 已上课").arg(currentClass["name"].toString()),
                        QString("上课时间到")
                });
            }
            else {
                int diffTime = currentDateTime.secsTo(currentClassEndTime);
                int hour = diffTime / 3600;
                diffTime = diffTime % 3600;
                int min = diffTime / 60;
                int sec = diffTime % 60;
                QString displayString = QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0')).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
                emit tst(displayString);
                msleep(50);
            }
        }
        else if (currentDateTime.secsTo(currentClassEndTime) <= 0) { // 当前时间 - 当前课程下课时间 <= 0 (下课 or 不是这节课)
            if (currentDateTime.secsTo(nextClassStartTime)> 0) { // 当前时间 - 下一节课开始时间 >= 0 (就是这节课，且正在下课时间)
                emit setClassStyleSheet(idx, "color: black;"); //去除上一节课的边框
                emit setClassStyleSheet(idx + 1, "border-width: 0px 0px 4px 0px; border-color:rgb(0,226,142); border-style: solid; color: black;");
                if (currentDateTime.secsTo(currentClassEndTime) == 0 and classStarted) {
                    classStarted = false;
                    emit showStatusMessageAS({QString("%1 已经下课，请做好下节课上课准备").arg(currentClass["name"].toString()),
                        QString("%1 已下课").arg(currentClass["name"].toString()),
                        QString("下课时间到")
                    });
                }
                int diffTime = currentDateTime.secsTo(nextClassStartTime); // 当前时间 - 下一节课开始时间 (课间还剩多久)
                int hour = diffTime / 3600;
                diffTime = diffTime % 3600;
                int min = diffTime / 60;
                int sec = diffTime % 60;
                QString displayString = QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0')).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
                emit tst(displayString);
                if (hour == 0 && min == 2 && sec == 0) {
                    emit showStatusMessageAS({QString("%1 即将上课，请做好上课准备").arg(nextClass["name"].toString()),
                        QString("%1 即将上课").arg(nextClass["name"].toString()),
                        QString("即将上课")
                    });
                }
                msleep(50);
                continue;
            }
            idx++; //下一节课
        }
    }
    emit changeStackedIndex(0);
    emit toDone();
    emit initMainWindowAnimation();

}
QDateTime RefetchTableThread::getTodayTime(QString str, QDate date){
    QStringList timeList = str.split(":");
    int hour = timeList[0].toInt();
    int minute = timeList[1].toInt();

    QDateTime dateTime;
    dateTime.setDate(date);
    dateTime.setTime(QTime(hour, minute));
    return dateTime;
}

void MainTableWidget::huanKeSlot(){
    static const QStringList baseItems = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};
    static const QMap<QString,QString> baseEnCnDay = {
        {"星期一", "Mon"}, {"星期二", "Tue"}, {"星期三", "Wed"}, {"星期四", "Thu"},
        {"星期五", "Fri"}, {"星期六", "Sat"}, {"星期日", "Sun"}
    };
    QStringList items = baseItems;
    QMap<QString,QString> enCnDay = baseEnCnDay;
    auto result = readJsonFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    if (!result) return;
    QJsonObject appendixTables = (*result)["appendixTables"].toObject();
    for (auto iter = appendixTables.begin();iter != appendixTables.end();iter++){
        items << "附加课表：" + iter.key();
        enCnDay["附加课表：" + iter.key()] = "APPEND_" + iter.key();
    }
    bool ok;
    QString cnDay = QInputDialog::getItem(nullptr,"换课","选择使用的星期或附加课表",items,0,false,&ok);
    if (!ok) return;
    QString day = enCnDay[cnDay];
    refetchThread->stopFlag = true;
    if (!refetchThread->wait(3000)) {
        refetchThread->terminate();
        refetchThread->wait();
    }
    refetchThread->stopFlag = false;
    isFinished = false;
    if (day.contains("APPEND_",Qt::CaseSensitive)){
        todayTable = timeTable["appendixTables"].toObject()[day.split("_").last()].toArray();
    }else{
        todayTable = timeTable.value(day).toArray();
    }
    for (QWidget*widget : ui->class_show_widget->findChildren<QWidget*>())
    {
        delete widget;
    }
    refetchThread->setTodayTable(todayTable);
    refetchThread->start();
    resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());
    ui->stackedWidget->resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());
    ui->status_show->resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());

    if (windowHidden) on_hideWindow();
}



void MainTableWidget::initSysTrayIcon()
{

    sysTrayIcon = new QSystemTrayIcon(this);
    QIcon icon = QIcon(":/res/icon.png");
    sysTrayIcon->setIcon(icon);
    sysTrayIcon->setToolTip("ClassTopLand");
    connect(sysTrayIcon, &QSystemTrayIcon::activated,
            [=, this](QSystemTrayIcon::ActivationReason reason)
            {
                switch(reason)
                {
                case QSystemTrayIcon::Trigger:
                    showEditAction->activate(QAction::Trigger);
                    break;
                default:
                    break;
                }
            });
    createActions();
    createMenu();
    sysTrayIcon->show();
}
void MainTableWidget::createActions(){
    showEditAction = new QAction(tr("设置"),this);
    connect(showEditAction, &QAction::triggered, this, &MainTableWidget::on_showMainAction);
    // showMainAction = new QAction(tr("打开主界面"),this);
    // connect(showMainAction,&QAction::triggered,this,&MainTableWidget::startMainWindow);
    exitAppAction = new QAction(tr("退出"),this);
    connect(exitAppAction, &QAction::triggered, this, &MainTableWidget::on_exitAppAction);
    huanKeAction = new QAction(tr("临时换课"),this);
    connect(huanKeAction,&QAction::triggered,this,&MainTableWidget::huanKeSlot);

}
void MainTableWidget::createMenu(){

    trayMenu = new QMenu(this);
    trayMenu->addAction(showEditAction);
    trayMenu->addAction(huanKeAction);
    trayMenu->addSeparator();
    trayMenu->addAction(exitAppAction);
    sysTrayIcon->setContextMenu(trayMenu);
}
void MainTableWidget::on_showMainAction(){
    editWindow->show();
}

void MainTableWidget::on_exitAppAction(){
    qApp->quit();
}
void MainTableWidget::refetchTableSlot(){
    refetchThread->stopFlag = true;
    if (!refetchThread->wait(3000)) {
        refetchThread->terminate();
        refetchThread->wait();
    }
    refetchThread->stopFlag = false;
    readTimeTable();
    isFinished = false;
    for (QWidget*widget : ui->class_show_widget->findChildren<QWidget*>())
    {
        delete widget;
    }
    refetchThread->start();
    resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());
    ui->stackedWidget->resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());
    ui->status_show->resize(SIDEBAR_WIDTH + BLOCK_SPACING + todayTable.count() * CLASS_BLOCK_SIZE,height());
    if (windowHidden) on_hideWindow();
    sysTrayIcon->showMessage(tr("提示"),tr("配置已成功应用！"),QSystemTrayIcon::MessageIcon::Information,500);
}


void MainTableWidget::setStyleSheetFromFile(QWidget* widget,QString file){
    QFile styleFile(file);
    if(styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showLog("StyleSheet is Loaded",LogStatus::INFO);
        QString styleSheet(styleFile.readAll());
        widget->setStyleSheet(styleSheet);
        styleFile.close();
    }
    else
    {
        showLog("StyleSheet is Load failed",LogStatus::ERR);
    }
}
