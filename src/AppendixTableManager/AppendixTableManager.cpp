//
// Created by JH182 on 2024/11/24.
//

// You may need to build the project (run Qt uic code generator) to get "ui_AppendixTableManager.h" resolved

#include "./AppendixTableManager.h"
#include "ui_AppendixTableManager.h"


AppendixTableManager::AppendixTableManager(QWidget *parent) :
        QDialog(parent), ui(new Ui::AppendixTableManager) {
    ui->setupUi(this);
    readAppendixTables();
    initSignal();
}

AppendixTableManager::~AppendixTableManager() {
    delete ui;
}

void AppendixTableManager::initSignal() {
    connect(ui->list_appendTable,&QListWidget::itemDoubleClicked,this,&AppendixTableManager::deleteAppendTables);
    connect(ui->add_table,&QPushButton::clicked,this,&AppendixTableManager::addAppendTables);
    connect(ui->edit_table,&QPushButton::clicked,this,&AppendixTableManager::editAppendTables);
}
void AppendixTableManager::readAppendixTables() {
    ui->list_appendTable->clear();
    auto result = readJsonFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    if (!result) return;
    appendixTables = (*result)["appendixTables"].toObject();
    for (QJsonObject::iterator iter = appendixTables.begin(); iter != appendixTables.end(); iter++){
        ui->list_appendTable->addItem(iter.key());
    }
}

void AppendixTableManager::addAppendTables() {
    QInputDialog dialog{this, Qt::WindowCloseButtonHint};
    dialog.setWindowTitle(tr("新建附加课表"));
    dialog.setInputMode(QInputDialog::InputMode::TextInput);
    dialog.setTextEchoMode(QLineEdit::Normal);
    dialog.setLabelText(tr("请输入课表名称"));
    dialog.setOkButtonText(QObject::tr("确定"));
    dialog.setCancelButtonText(QObject::tr("取消"));
    dialog.setFixedSize(350,250);
    dialog.setTextValue("");
    // 设置了 QPushButtuon 的三态， QLineEdit 的样式， QLable 和 整体的背景色
    QString style = "QPushButton{background:#1191d3; color:rgb(255,255,255); border-radius:3px; min-height:30px; min-width:60px; font:13px \"Microsoft YaHei\";}"
                    "QPushButton:hover{background:#1191d0;}"
                    "QPushButton:pressed{background:rgb(32,75,148);}"
                    "QLineEdit{border:2px solid rgb(229,230,231);background:#ffffff;padding:4px;padding-left:10px;border-radius:3px;color:rgb(105,105,105);font:13px \"Microsoft YaHei\";}"
                    "QLineEdit:focus{border:2px solid #1191d3;}"
                    "QLineEdit:disabled{background-color:rgb(238,238,238);}"
                    "QLabel{color:rgb(85,85,85); background:#ffffff;font:12px \"Microsoft YaHei\"; font-weight:bold;}"
                    "QInputDialog{background-color:rgb(255,255,255); }";
    dialog.setStyleSheet(style);
    dialog.exec();
    if (!dialog.textValue().isEmpty()){
        bool hasInput = false;
        for (int i = 0; i < ui->list_appendTable->count(); i++){
            if (dialog.textValue() == ui->list_appendTable->item(i)->text()){
                hasInput = true;
                break;
            }
        }
        if (!hasInput){
            QJsonArray nullJsonArray;
            appendixTables[dialog.textValue()] = nullJsonArray;
            writeAppendixTables();
            readAppendixTables();
        }

    }
}
void AppendixTableManager::deleteAppendTables(QListWidgetItem *item) {
    appendixTables.remove(item->text());
    writeAppendixTables();
    ui->list_appendTable->removeItemWidget(item);
    delete item;
    QMessageBox::information(this,"提示","删除成功！");
}

void AppendixTableManager::writeAppendixTables() {
    auto result = readJsonFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    if (!result) return;
    QJsonObject allTables = *result;
    allTables["appendixTables"] = appendixTables;
    QFile outFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    outFile.open(QIODevice::Truncate | QIODevice::WriteOnly);
    QJsonDocument writeDoc;
    writeDoc.setObject(allTables);
    outFile.write(writeDoc.toJson());
    outFile.close();
}

void AppendixTableManager::editAppendTables() {
    QString editTableName = ui->list_appendTable->currentItem()->text();
    emit editAppendixTable(editTableName);
    close();
}