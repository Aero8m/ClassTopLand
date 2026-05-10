#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <QWidget>
#include <QtTest/QTest>
#include<QKeyEvent>
#include<QCoreApplication>
#include<QDir>
#include<QLibrary>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonDocument>
#include<QJsonParseError>
#include"../ClickLabel/ClickLabel.h"
#pragma comment(lib,"Dwmapi.lib")
#include<QColorDialog>
#include"../AppLog/AppLog.h"
#include "../YiyanDialog/YiyanDialog.h"
#include<cstdlib>
namespace Ui {
class ToolBox;
}

class ToolBox : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBox(QWidget *parent = nullptr);
    ~ToolBox();
public slots:
    void LoadPlugins();
signals:
    void reMove();
private:
    Ui::ToolBox *ui;
private:
    yiyanDialog *yiyan;
};

#endif // TOOLBOX_H
