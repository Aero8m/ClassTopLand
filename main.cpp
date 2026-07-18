#include "./src/MainTableWidget/MainTableWidget.h"
#include"./src/DayTimerWidget/DayTimerWidget.h"
#include <QApplication>
#include <QStyleHints>
#include <QLocale>
#include <QTranslator>
#include<QDir>
#include<QDateTime>
#include<QProcess>
#include<QFontDatabase>
#include<QStyleFactory>
#include <iostream>


#include"./src/AppLog/AppLog.h"
#include"./src/NetworkRequests/NetworkRequests.h"
#include "./src/Utils/Utils.h"
bool timerIsOpen(){
    auto result = readJsonFile(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    if (!result) return true;

    QJsonObject config = *result;
    if (config.contains("disable_timer")){
        if (config["disable_timer"].toBool()){
            showLog("Timer is Not Show",LogStatus::INFO);
            return false;
        }else{
            showLog("Timer is Show",LogStatus::INFO);
            return true;
        }
    }
    showLog("Timer is Show",LogStatus::INFO);
    return true;
}
void printLogo() {
    QFile file(":/res/logo.txt");
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream stream(&file);
    QString fileStr = stream.readAll();
    file.close();
    std::cout << fileStr.toStdString() << std::endl;
    std::cout << "---------------------------------------------------------------------------" << std::endl;
}
void createFolder(const QString &folderPath) {
    QDir dir(folderPath);
    if (!dir.exists()) {
        dir.mkdir(folderPath);
    }
}
int main(int argc, char *argv[])
{
#ifdef __linux__

    // 插入 -platform xcb 参数
    char *newArgv[argc + 2];
    newArgv[0] = argv[0];
    newArgv[1] = const_cast<char*>("-platform");
    newArgv[2] = const_cast<char*>("xcb");

    for (int i = 1; i < argc; ++i) {
        newArgv[i + 2] = argv[i];
    }

    int newArgc = argc + 2;
    QApplication a(newArgc, newArgv);
#else
    QApplication a(argc, argv);
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif
    //QApplication::setAttribute(Qt::AA_SetPlatformPlugin, QVariant("xcb"));
    printLogo();

    createFolder(QDir::homePath() + "/ClassTopLand_Data");
    a.setApplicationDisplayName("ClassTopLand");
    a.setStyleSheet(getStyleSheet(":/qss/global.qss"));
    QApplication::setQuitOnLastWindowClosed(false);
    showLog("MainWindow is Show",LogStatus::INFO);
    MainTableWidget *mainWidget = new MainTableWidget();


#ifdef __linux__
    QScreen *screen = QApplication::primaryScreen();
    // 逻辑尺寸
    QSize logicalSize = screen->size();  // 或 screen->availableSize()

    // 物理尺寸（实际像素）
    qreal ratio = screen->devicePixelRatio();
    QSize physicalSize = logicalSize * ratio;
    int screenWidth = physicalSize.width();
    int screenHeight = physicalSize.height();
    DayTimerWidget *dayTimerWidget = new DayTimerWidget();
    dayTimerWidget->move((screenWidth - dayTimerWidget->width()),(screenHeight - dayTimerWidget->height()) * 0.95);
    if (timerIsOpen()) {
        dayTimerWidget->show();
    }
    mainWidget->show();
#else
    QScreen *screen = a.primaryScreen();
    int screenWidth = screen->size().width();
    int screenHeight = screen->size().height();
    // mainWidget->move((screenWidth - mainWidget->width()) / 2, 0);
    mainWidget->show();
    DayTimerWidget *dayTimerWidget = new DayTimerWidget();
    dayTimerWidget->move((screenWidth - dayTimerWidget->width()),(screenHeight - dayTimerWidget->height()) * 0.95);
    if (timerIsOpen()) {
        dayTimerWidget->show();
    }
#endif
    return a.exec();
}
