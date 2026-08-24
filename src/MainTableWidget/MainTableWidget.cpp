#include "./MainTableWidget.h"
#include "ui_MainTableWidget.h"
#include "../Utils/Utils.h"
#include <QSet>



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
    connect(qApp, &QCoreApplication::aboutToQuit,
            this, &MainTableWidget::stopRefetchThread);
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

    runningDate = QDate::currentDate();
    restartTimer = new QTimer(this);
    connect(restartTimer, &QTimer::timeout, this, [this] {
        if (QDate::currentDate() != runningDate) {
            restartApplication();
        }
    });

    restartTimer->start(1000);
}

MainTableWidget::~MainTableWidget()
{
    if (refetchThread) {
        stopRefetchThread();
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

void MainTableWidget::stopRefetchThread()
{
    if (!refetchThread || !refetchThread->isRunning())
    {
        return;
    }

    refetchThread->requestInterruption();
    refetchThread->wait();
}

void MainTableWidget::restartApplication()
{
    const QString program = QCoreApplication::applicationFilePath();
    QStringList arguments = QCoreApplication::arguments();
    if (!arguments.isEmpty()) {
        arguments.removeFirst();
    }

    const bool started = QProcess::startDetached(
        program,
        arguments,
        QCoreApplication::applicationDirPath()
    );
    if (started) {
        qApp->quit();
    }
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
    const QPoint expandedPosition((screenW - expandedWindowWidth()) / 2, 0);
    const QPoint hiddenPosition(screenW - collapsedWindowWidth(), 0);
    move(windowHidden ? hiddenPosition : expandedPosition);
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
    hideAnimation->setStartValue(expandedPosition);
    hideAnimation->setEndValue(hiddenPosition);

}

int MainTableWidget::expandedWindowWidth() const
{
    if (isFinished)
    {
        return DEFAULT_WINDOW_WIDTH;
    }

    return SIDEBAR_WIDTH + BLOCK_SPACING
        + static_cast<int>(todayTable.count()) * CLASS_BLOCK_SIZE;
}

int MainTableWidget::collapsedWindowWidth() const
{
    return isFinished ? ui->hide_window->width() : SIDEBAR_WIDTH;
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
    if (!hideAnimation || hideAnimation->state() == QAbstractAnimation::Running)
    {
        return;
    }

    if (isFinished)
    {
        const int expandedWidth = expandedWindowWidth();
        resize(expandedWidth, height());
        ui->stackedWidget->resize(expandedWidth, height());
        ui->status_show->resize(expandedWidth, height());
        ui->stackedWidget->show();
        ui->status_show->show();
        ui->hide_window->show();
        ui->hide_window->raise();

        if (!windowHidden)
        {
            windowHidden = true;
            hideAnimation->setDirection(QAbstractAnimation::Forward);
        }
        else
        {
            windowHidden = false;
            hideAnimation->setDirection(QAbstractAnimation::Backward);
        }
        hideAnimation->start();
        return;
    }

    if (!windowHidden)
    {
        hideAnimation->setDirection(QAbstractAnimation::Forward);
        hideAnimation->start();
        QTimer::singleShot(hideAnimation->duration() / 2, this, [=, this] {
            ui->class_show_widget->hide();
            const int collapsedWidth = collapsedWindowWidth();
            resize(collapsedWidth, height());
            ui->stackedWidget->resize(collapsedWidth, height());
            ui->status_show->resize(collapsedWidth, height());
            ui->hide_window->show();
            ui->hide_window->raise();
            windowHidden = true;
        });
    }else
    {
        hideAnimation->setDirection(QAbstractAnimation::Backward);
        hideAnimation->start();
        QTimer::singleShot(hideAnimation->duration() / 2, this, [=, this] {
            ui->class_show_widget->show();
            ui->stackedWidget->show();
            ui->status_show->show();
            const int expandedWidth = expandedWindowWidth();
            resize(expandedWidth, height());
            ui->stackedWidget->resize(expandedWidth, height());
            ui->status_show->resize(expandedWidth, height());
            ui->hide_window->raise();
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
        const int targetWidth = expandedWindowWidth();
        ui->stackedWidget->show();
        ui->status_show->show();
        ui->hide_window->show();
        ui->hide_window->raise();
        resize(targetWidth, height());
        ui->stackedWidget->resize(targetWidth, height());
        ui->status_show->resize(targetWidth, height());
        QScreen *screen = qApp->primaryScreen();
        int screenW = screen->size().width();
        const QPoint expandedPosition((screenW - expandedWindowWidth()) / 2, 0);
        const QPoint hiddenPosition(screenW - collapsedWindowWidth(), 0);
        if (hideAnimation)
        {
            hideAnimation->stop();
            hideAnimation->setStartValue(expandedPosition);
            hideAnimation->setEndValue(hiddenPosition);
        }
        if (windowHidden)
        {
            move(hiddenPosition);
        }
        else
        {
            move(expandedPosition);
        }

    },Qt::QueuedConnection);
    connect(refetchThread,&RefetchTableThread::initMainWindowAnimation,this,&MainTableWidget::initAnimation,Qt::QueuedConnection);
    connect(ui->hide_window,&QPushButton::clicked,this,&MainTableWidget::on_hideWindow);
    connect(refetchThread,&RefetchTableThread::setDoneTabText,this,[this](const QString &text)
    {
        ui->done_tab_text->setText(text);
    },Qt::QueuedConnection);
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
    if (isInterruptionRequested())
    {
        return;
    }
    if (todayTable.count() <= 0)
    {
        emit setDoneTabText("今天没有课程");
        emit changeStackedIndex(0);
        emit initMainWindowAnimation();
        emit toDone();
        return;
    }
    for (int i=0;i<todayTable.count();i++)
    {
        if (isInterruptionRequested())
        {
            return;
        }
        emit addClass(todayTable[i].toObject()["name"].toString().trimmed().left(1));
    }

    emit initMainWindowAnimation();
    QJsonObject nullClass = { {"start","00:00"},{"end","00:00"} };
    int requestedPage = -1;
    QString lastDoneText;
    QSet<int> upcomingReminderShown;
    auto switchPage = [this, &requestedPage](int page)
    {
        if (requestedPage != page)
        {
            emit changeStackedIndex(page);
            requestedPage = page;
        }
    };

    for(int idx = 0;idx < todayTable.count();)
    {
        if (isInterruptionRequested()) {
            return;
        }

        QDate today = QDate::currentDate();
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QJsonObject currentClass = todayTable[idx].toObject();
        QJsonObject nextClass = idx < todayTable.count() - 1 ? todayTable[idx + 1].toObject() : nullClass;
        QDateTime currentClassStartTime = getTodayTime(currentClass.value("start").toString(), today);
        QDateTime currentClassEndTime = getTodayTime(currentClass.value("end").toString(), today);
        QDateTime nextClassStartTime = getTodayTime(nextClass.value("start").toString(), today);

        switchPage(1);

        if (idx == 0 && currentDateTime < currentClassStartTime) // 当前还没有上课
        {
            emit setClassStyleSheet(idx, "border-width: 0px 0px 4px 0px; border-color:rgb(0,226,142); border-style: solid; color: black;");
            int diffTime = currentDateTime.secsTo(currentClassStartTime);
            int hour = diffTime / 3600;
            diffTime = diffTime % 3600;
            int min = diffTime / 60;
            int sec = diffTime % 60;
            QString displayString = QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0')).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
            emit tst(displayString);
            msleep(50);
        } else if (currentDateTime < currentClassEndTime) { // 已到上课时间，且当前课程尚未结束
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
        } else { // 当前课程已结束
            const qint64 remainingSecs = currentDateTime.secsTo(nextClassStartTime);
            if (remainingSecs > 0) { // 下一节课尚未开始，当前处于课间
                emit setClassStyleSheet(idx, "color: black;"); //去除上一节课的边框
                emit setClassStyleSheet(idx+1, "border-width: 0px 0px 4px 0px; border-color:rgb(0,226,142); border-style: solid; color: black;");
                if (currentDateTime.secsTo(currentClassEndTime) == 0 and classStarted) {
                    classStarted = false;
                    emit showStatusMessageAS({QString("%1 已经下课，请做好下节课上课准备").arg(currentClass["name"].toString()),
                        QString("%1 已下课").arg(currentClass["name"].toString()),
                        QString("下课时间到")
                    });
                }
                int diffTime = static_cast<int>(remainingSecs); // 当前时间 - 下一节课开始时间 (课间还剩多久)
                int hour = diffTime / 3600;
                diffTime = diffTime % 3600;
                int min = diffTime / 60;
                int sec = diffTime % 60;
                QString displayString = QString("%1:%2:%3").arg(hour, 2, 10, QLatin1Char('0')).arg(min, 2, 10, QLatin1Char('0')).arg(sec, 2, 10, QLatin1Char('0'));
                emit tst(displayString);
                const int nextClassIndex = idx + 1;
                if (remainingSecs <= 120 && !upcomingReminderShown.contains(nextClassIndex)) {
                    upcomingReminderShown.insert(nextClassIndex);
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
    emit setDoneTabText("今日的课程已上完");
    switchPage(0);
    emit toDone();
    emit initMainWindowAnimation();

}
QDateTime RefetchTableThread::getTodayTime(QString str, QDate date){
    QStringList timeList = str.split(":");
    if (timeList.size() != 2)
    {
        return {};
    }

    bool hourOk = false;
    bool minuteOk = false;
    int hour = timeList[0].toInt(&hourOk);
    int minute = timeList[1].toInt(&minuteOk);
    QTime time(hour, minute);
    if (!hourOk || !minuteOk || !time.isValid())
    {
        return {};
    }

    QDateTime dateTime;
    dateTime.setDate(date);
    dateTime.setTime(time);
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
    stopRefetchThread();
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
    exitAppAction = new QAction(tr("退出"),this);
    connect(exitAppAction, &QAction::triggered, this, &QApplication::exit);
    huanKeAction = new QAction(tr("临时换课"),this);
    connect(huanKeAction,&QAction::triggered,this,&MainTableWidget::huanKeSlot);
    restartAppAction = new QAction(tr("重启"),this);
    connect(restartAppAction, &QAction::triggered, this, &MainTableWidget::restartApplication);

}
void MainTableWidget::createMenu(){

    trayMenu = new QMenu(this);
    trayMenu->addAction(showEditAction);
    trayMenu->addAction(huanKeAction);
    trayMenu->addSeparator();
    trayMenu->addAction(restartAppAction);
    trayMenu->addAction(exitAppAction);
    sysTrayIcon->setContextMenu(trayMenu);
}
void MainTableWidget::on_showMainAction(){
    editWindow->show();
}

void MainTableWidget::refetchTableSlot(){
    stopRefetchThread();
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
