#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include<QMouseEvent>
#include<QDialog>
#include<QGraphicsDropShadowEffect>
#include<QPropertyAnimation>
#include<QInputDialog>
#include<QList>
#include<QListWidgetItem>
#include<QCheckBox>
#include<QRandomGenerator>
#include<QTimer>
#include<QJsonObject>
#include<QJsonDocument>
#include<QJsonArray>
#include<QFileInfo>
#include<QDir>
#include<QNetworkReply>
#include<QNetworkRequest>
#include<QNetworkAccessManager>
#include<QNetworkProxy>
#include<QTranslator>
#include"../AppLog/AppLog.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void showEvent(QShowEvent *event) override;
    void readTodos();
    QStringList yilist;
    QStringList jilist;
    QJsonArray todos;
    QJsonObject Config;
    QNetworkAccessManager *smsManager = nullptr;
    void newTodo(QString name,bool n=false);
    void timerEvent(QTimerEvent *event) override;
private slots:
    void on_butAdd_clicked();
    void on_btnhk_clicked();
    void on_time_changed();
    void on_startTimer_Click();

signals:
    void hk(QString day);
    void timerisStart(bool &st);
    void startTm(QString timer_st);
    void stopTm();
    void getTimer(int &m,int &s);
public slots:
    void on_reText();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
