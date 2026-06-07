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
#include"../Utils/Utils.h"
QT_BEGIN_NAMESPACE
namespace Ui { class DayTimerWidget; }
QT_END_NAMESPACE

class DayTimerWidget : public QWidget {
Q_OBJECT

public:
    explicit DayTimerWidget(QWidget *parent = nullptr);
    QTimer* refreshTimer;
    QFontMetrics *fm = nullptr;
    void readTimeJson();
    QJsonObject timeJson;
    ~DayTimerWidget() override;
public slots:
    void reloadTimer();
private:
    Ui::DayTimerWidget *ui;
    QFont fontLabel2;
    QFont fontLabel3;
    QFont fontLabel5;
    QFont fontLabel4;
    QFont fontLabel7;
    QFont fontLabel6;
    QFont fontLabel4End;
    bool fontsInitialized = false;
    void initFonts();
};


#endif //SCHOOLTOOLS_DAYTIMERWIDGET_H
