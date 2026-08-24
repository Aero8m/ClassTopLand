#include "./src/MainTableWidget/MainTableWidget.h"
#include"./src/DayTimerWidget/DayTimerWidget.h"
#include"./src/AppLog/AppLog.h"
#include"./src/NetworkRequests/NetworkRequests.h"
#include "./src/Utils/Utils.h"
#include <QApplication>
#include <QStyleHints>
#include<QDir>
#include <iostream>

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
    QApplication a(argc, argv);
#else
    QApplication a(argc, argv);
    QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif
    printLogo();

    createFolder(QDir::homePath() + "/ClassTopLand_Data");
    a.setApplicationDisplayName("ClassTopLand");
    a.setStyleSheet(getStyleSheet(":/qss/global.qss"));
    QApplication::setQuitOnLastWindowClosed(false);
    showLog("MainWindow is Show",LogStatus::INFO);
    MainTableWidget *mainWidget = new MainTableWidget();


#ifdef __linux__
    QScreen *screen = QApplication::primaryScreen();
    QSize logicalSize = screen->size();
    int screenWidth = logicalSize.width();
    int screenHeight = logicalSize.height();
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
    mainWidget->show();
    if (timerIsOpen()) {
        DayTimerWidget *dayTimerWidget = new DayTimerWidget();
        dayTimerWidget->move((screenWidth - dayTimerWidget->width()),(screenHeight - dayTimerWidget->height()) * 0.95);
        dayTimerWidget->show();
    }
#endif
    return a.exec();
}
