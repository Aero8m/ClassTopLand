//
// Created by JH182 on 2024/8/21.
//

#ifndef SCHOOLTOOLS_DAYTIMERWIDGET_H
#define SCHOOLTOOLS_DAYTIMERWIDGET_H

#include <QWidget>
#include<QTimer>
#include<QJsonObject>
#include<QJsonArray>
#include<QJsonDocument>
#include<QFile>
#include<QDir>
#include<QFont>
#include<qfontdatabase.h>
#include"../AppLog/AppLog.h"
QT_BEGIN_NAMESPACE
namespace Ui { class DayTimerWidget; }
QT_END_NAMESPACE

class DayTimerWidget : public QWidget {
Q_OBJECT

public:
    explicit DayTimerWidget(QWidget *parent = nullptr);
    QTimer* timer_timer;
    QFontMetrics *fm = nullptr;
    void readTimeJson();
    QJsonObject TimeJson;
    ~DayTimerWidget() override;
public slots:
    void reload_timer();
private:
    Ui::DayTimerWidget *ui;
    QFont font_label2;
    QFont font_label3;
    QFont font_label5;
    QFont font_label4;
    QFont font_label7;
    QFont font_label6;
    QFont font_label4_end;
    bool fontsInitialized = false;
    void initFonts();
};


#endif //SCHOOLTOOLS_DAYTIMERWIDGET_H
