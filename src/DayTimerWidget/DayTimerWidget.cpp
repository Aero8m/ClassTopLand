//
// Created by JH182 on 2024/8/21.
//

// You may need to build the project (run Qt uic code generator) to get "ui_DayTimerWidget.h" resolved

#include "./DayTimerWidget.h"
#include "ui_DayTimerWidget.h"


DayTimerWidget::DayTimerWidget(QWidget *parent) :
        QWidget(parent), ui(new Ui::DayTimerWidget) {
    ui->setupUi(this);
    readTimeJson();
    setWindowFlags(Qt::WindowType::FramelessWindowHint | Qt::WindowType::Tool | Qt::WindowType::WindowStaysOnBottomHint);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground);
    refreshTimer = new QTimer();
    refreshTimer->setInterval(50);
    connect(refreshTimer,&QTimer::timeout,this,&DayTimerWidget::reloadTimer);
    initFonts();
    refreshTimer->start();
    ui->label_2->setText("距离"+timeJson["label_tag"].toString());

}

DayTimerWidget::~DayTimerWidget() {
    refreshTimer->stop();
    delete refreshTimer;
    delete fm;
    delete ui;
}

void DayTimerWidget::initFonts() {
    if (fontsInitialized) return;

    int fontId1 = QFontDatabase::addApplicationFont(":/res/DTF-1.ttf");
    QString fontName1 = QFontDatabase::applicationFontFamilies(fontId1).at(0);
    int fontId2 = QFontDatabase::addApplicationFont(":/res/DTF-2.ttf");
    QString fontName2 = QFontDatabase::applicationFontFamilies(fontId2).at(0);
    int fontId3 = QFontDatabase::addApplicationFont(":/res/DTF-3.ttf");
    QString fontName3 = QFontDatabase::applicationFontFamilies(fontId3).at(0);

    fontLabel2 = QFont(fontName1,36);
    fontLabel3 = QFont(fontName1,48);
    fontLabel5 = QFont(fontName1,48);
    fontLabel4 = QFont(fontName2,110);
    fontLabel7 = QFont(fontName2,20);
    fontLabel6 = QFont(fontName3,20);
    fontLabel4End = QFont(fontName1,47);

    ui->label_2->setFont(fontLabel2);
    ui->label_3->setFont(fontLabel3);
    ui->label_5->setFont(fontLabel5);
    ui->label_4->setFont(fontLabel4);
    ui->label_7->setFont(fontLabel7);
    ui->label_6->setFont(fontLabel6);

    fm = new QFontMetrics(fontLabel4);
    fontsInitialized = true;
}

void DayTimerWidget::reloadTimer() {
    timeJson["english_end"] = timeJson["english_end"].toString().toUpper();
    timeJson["english"] = timeJson["english"].toString().toUpper();
    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime endTime = QDateTime::fromString(timeJson["end_time"].toString(),"yyyy-MM-dd hh:mm:ss");
    qint64 remainingMsecs = currentTime.msecsTo(endTime);
    int days = remainingMsecs / (24 * 60 * 60 * 1000);
    remainingMsecs %= (24 * 60 * 60 * 1000);
    int hours = remainingMsecs / (60 * 60 * 1000);
    remainingMsecs %= (60 * 60 * 1000);
    int minutes = remainingMsecs / (60 * 1000);
    remainingMsecs %= (60 * 1000);
    int seconds = remainingMsecs / 1000;
    int milliseconds = remainingMsecs % 1000;
    QString dayUnit;
    QString dayUnitEnglish;
    int remainingTime;

    if (days >= 1){
        dayUnit = "天";
        dayUnitEnglish = "Days";
        ui->label_7->setText(QString(":%1:%2:%3.%4").arg(hours, 2, 10, QLatin1Char('0')).arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')).arg(milliseconds));
        remainingTime = days;
    }else if(hours > 0){
        dayUnit = "时";
        dayUnitEnglish = "Hours";
        ui->label_7->setText(QString(":%1:%2.%3").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds, 2, 10, QLatin1Char('0')).arg(milliseconds));
        remainingTime = hours;
    }else if(minutes > 0){
        dayUnit = "分";
        dayUnitEnglish = "Minutes";
        ui->label_7->setText(QString(":%1.%2").arg(seconds).arg(milliseconds));
        remainingTime = minutes;
    }else if(seconds > 0){
        dayUnit = "秒";
        dayUnitEnglish = "Seconds";
        ui->label_7->setText(QString(".%1").arg(milliseconds));
        remainingTime = seconds;
    }else if(milliseconds > 0){
        dayUnit = "毫秒";
        dayUnitEnglish = "Milliseconds";
        ui->label_7->setText("");
        remainingTime = milliseconds;
    }else{
        ui->label_3->setText("");
        ui->label_5->setText("");
        ui->label_4->setText("一天不剩");
        ui->label_7->setText("");
        ui->label_6->setText(timeJson["english_end"].toString().replace("$","DAY"));
        ui->label_4->setGeometry(QRect(230, 100, 200 * 4, 111));
        ui->label_4->setFont(fontLabel4End);
        ui->label_5->setGeometry(QRect(270, 90, 211, 111));
        showLog("Timer is stopped",LogStatus::INFO);
        refreshTimer->stop();
        return;
    }

    if (remainingTime < 0){
        ui->label_3->setText("");
        ui->label_5->setText("");
        ui->label_4->setText("一天不剩");
        ui->label_7->setText("");
        ui->label_6->setText(timeJson["english_end"].toString().replace("$","DAY"));
        ui->label_4->setGeometry(QRect(230, 100, 200 * 4, 111));
        ui->label_4->setFont(fontLabel4End);
        ui->label_5->setGeometry(QRect(270, 90, 211, 111));
        showLog("Timer is stopped",LogStatus::INFO);
        refreshTimer->stop();
        return;
    }

    int textWidth = fm->horizontalAdvance(QString::number(remainingTime));
    ui->label_7->setGeometry(QRect(350 + textWidth + 10, 100, 411, 21));
    dayUnitEnglish = dayUnitEnglish.toUpper();
    ui->label_5->setText(dayUnit);
    ui->label_4->setGeometry(QRect(350, 70, textWidth, 111));
    ui->label_5->setGeometry(QRect(350 + textWidth + 10, 120, 211, 61));
    ui->label_4->setNum(remainingTime);
    ui->label_6->setText(timeJson["english"].toString().replace("()", QString::number(remainingTime)).replace("$",dayUnitEnglish));
}
void DayTimerWidget::readTimeJson() {
    auto result = readJsonFile(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    if (result) {
        timeJson = *result;
    }
}