//
// Created by JH182 on 2024/11/24.
//

#ifndef SCHOOLTOOLS_APPENDIXTABLEMANAGER_H
#define SCHOOLTOOLS_APPENDIXTABLEMANAGER_H

#include <QDialog>
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QInputDialog>
#include <QLineEdit>
#include "../AppLog/AppLog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AppendixTableManager; }
QT_END_NAMESPACE

class AppendixTableManager : public QDialog {
Q_OBJECT

public:
    explicit AppendixTableManager(QWidget *parent = nullptr);

    QJsonObject appendixTables;
    ~AppendixTableManager() override;
private slots:
    void deleteAppendTables(QListWidgetItem *item);
    void addAppendTables();
    void editAppendTables();
signals:
    void editAppendixTable(QString TableName);
private:
    void readAppendixTables();
    void initSignal();
    void writeAppendixTables();
    Ui::AppendixTableManager *ui;
};


#endif //SCHOOLTOOLS_APPENDIXTABLEMANAGER_H
