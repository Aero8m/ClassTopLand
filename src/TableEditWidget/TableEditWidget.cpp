#include "./TableEditWidget.h"
#include "ui_TableEditWidget.h"


TableEditWidget::TableEditWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableEditWidget)
{
    ui->setupUi(this);
    connect(ui->radioButton,&QRadioButton::toggled, this, &TableEditWidget::toggleded);
    connect(ui->radioButton_2,&QRadioButton::toggled, this, &TableEditWidget::toggleded);
    connect(ui->radioButton_3,&QRadioButton::toggled, this, &TableEditWidget::toggleded);
    connect(ui->radioButton_4,&QRadioButton::toggled, this, &TableEditWidget::toggleded);
    connect(ui->radioButton_5,&QRadioButton::toggled, this, &TableEditWidget::toggleded);
    connect(ui->label,&ClickLabel::clicked,this,[=, this]{
        if (clickCount >=10){
            clickCount=0;
            QInputDialog dialog{this, Qt::WindowCloseButtonHint};
            dialog.setWindowTitle(tr("调试码"));
            dialog.setInputMode(QInputDialog::InputMode::TextInput);
            dialog.setTextEchoMode(QLineEdit::Normal);
            dialog.setLabelText(tr("请输入调试码"));
            dialog.setOkButtonText(QObject::tr("确定"));
            dialog.setCancelButtonText(QObject::tr("取消"));
            dialog.setFixedSize(350,250);
            dialog.setTextValue("");
            dialog.exec();
            if (!dialog.textValue().isEmpty()){
                QByteArray bytearray;
                bytearray.append(dialog.textValue().toUtf8());
                QCryptographicHash hash(QCryptographicHash::Sha256);
                hash.addData(bytearray);
                QByteArray hasharray = hash.result();
                QString hashValue = hasharray.toHex();
                qDebug() << hashValue;
                if (hashValue == "a8c97315e9aa9eed727ae5aa9515e2a27d4df30cc68c4a210fa7b2d3c4e3ea20") {
                    QDesktopServices::openUrl(QUrl("https://www.bilibili.com/video/BV1wv411Y7YN"));
                }else{
                   QMessageBox::critical(this,"错误","调试码错误！");
                }
            }

        }else{
            clickCount++;
        }
    });
    readTableJson();
    connect(ui->pushButton_3,&QPushButton::clicked,this,[=]{
        QDesktopServices::openUrl(QUrl("https://github.com/Aero80wd/ClassTopLand"));
    });
    connect(ui->btn_abLink, &QPushButton::clicked, this, [=] {
        QDesktopServices::openUrl(QUrl("https://autobuild.aero8m.cn/ClassTopLand"));
        });
    connect(ui->save_text_config,&QPushButton::clicked,this,&TableEditWidget::on_timerInfo_changed);
    connect(ui->start_table_manager,&QPushButton::clicked,this,&TableEditWidget::on_show_AppendixTableManager);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->installEventFilter(this);
    ui->label_4->setText("Build " + QString(APP_VERSION));
    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &TableEditWidget::on_cellChanged);
    ui->tabWidget->setTabPosition(QTabWidget::West);
    //ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);
}

TableEditWidget::~TableEditWidget()
{

    delete ui;
}
bool TableEditWidget::timesort(QJsonObject &obj1, QJsonObject &obj2){
    QTime time1 = QTime::fromString(obj1.value("start").toString());
    QTime time2 = QTime::fromString(obj2.value("start").toString());
    return time1 < time2;
}
void TableEditWidget::showEvent(QShowEvent* event){
    toggleded();
}
void TableEditWidget::closeEvent(QCloseEvent *event){
    QApplication::setQuitOnLastWindowClosed(false);
    emit refetchTableSignal();
    this->hide();
    event->ignore();
}
void TableEditWidget::on_show_AppendixTableManager() {
    AppendixTableManager *atm = new AppendixTableManager(this);
    atm->setAttribute(Qt::WA_DeleteOnClose);
    connect(atm,&AppendixTableManager::editAppendixTable,this,&TableEditWidget::on_editAppendixTable);
    atm->setModal(false);
    atm->show();
}

void TableEditWidget::on_editAppendixTable(QString tableName) {
    ui->radioButton_6->setChecked(true);
    isEditAppendixTable = true;
    currentEditAppendixTableName = tableName;
    refechTableWidget(timeTableJson["appendixTables"].toObject()[tableName].toArray());
}

void TableEditWidget::setConfig(QJsonObject obj){
    configJson=obj;
    ui->timer_hide->setChecked(configJson.value("disable_timer").toBool());
    ui->timer_time->setDateTime(QDateTime::fromString(configJson["endTime"].toString(),"yyyy-MM-dd hh:mm:ss"));
    ui->edit_name->setText(configJson["label_tag"].toString());
    ui->edit_name_eng->setText(configJson["english_tag"].toString());
}
void TableEditWidget::on_timerInfo_changed(){
    configJson["end_time"] = ui->timer_time->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    configJson["label_tag"] = ui->edit_name->text();
    configJson["english"] = QString("There are () $\nleft until %1").arg(ui->edit_name_eng->text());
    configJson["english_end"] = QString("There is not a $\nleft until %1").arg(ui->edit_name_eng->text());
    configJson["english_tag"] = ui->edit_name_eng->text();
    configJson["disable_timer"] = ui->timer_hide->isChecked();
    QFile configFile(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    configFile.open(QFile::WriteOnly);
    QJsonDocument tempDoc;
    tempDoc.setObject(configJson);
    configFile.write(tempDoc.toJson(QJsonDocument::Indented));
    configFile.close();
    QMessageBox::information(this,tr("提示"),tr("重启生效"));
}

void TableEditWidget::readTableJson(){
    auto result = readJsonFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    if (!result) return;
    timeTableJson = *result;
}
void TableEditWidget::refechTableWidget(QJsonArray todayTable){

    ui->tableWidget->clear();
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "课程" << "上课时间" << "下课时间" << "课间时间");
    ui->tableWidget->setRowCount(todayTable.count());
    for (int x = 0; x<todayTable.count();x++){
        QJsonObject valueObject = todayTable.at(x).toObject();
        QString name = valueObject.value("name").toString();
        QString startTime = valueObject.value("start").toString();
        QString endTime = valueObject.value("end").toString();
        int splitTime = valueObject.value("split").toInt();
        ui->tableWidget->setItem(x,0,new QTableWidgetItem(name));
        ui->tableWidget->setItem(x,1,new QTableWidgetItem(startTime));
        ui->tableWidget->setItem(x,2,new QTableWidgetItem(endTime));
        ui->tableWidget->setItem(x,3,new QTableWidgetItem(QString::number(splitTime)));
    }
}
void TableEditWidget::toggleded(){

    if (ui->radioButton->isChecked()){
        isEditAppendixTable = false;
        refechTableWidget(timeTableJson.value("Mon").toArray());
    }else
    if (ui->radioButton_2->isChecked()){
        isEditAppendixTable = false;
        refechTableWidget(timeTableJson.value("Tue").toArray());
    }else
    if (ui->radioButton_3->isChecked()){
        isEditAppendixTable = false;
        refechTableWidget(timeTableJson.value("Wed").toArray());
    }else
    if (ui->radioButton_4->isChecked()){
        isEditAppendixTable = false;
        refechTableWidget(timeTableJson.value("Thu").toArray());
    }else
    if (ui->radioButton_5->isChecked()){
        isEditAppendixTable = false;
        refechTableWidget(timeTableJson.value("Fri").toArray());
    }
}
void TableEditWidget::addItem(QString key){
    QJsonObject insertJson;
    insertJson.insert("name",ui->lineEdit->text());
    insertJson.insert("start",ui->timeEdit->text());
    insertJson.insert("end",ui->timeEdit_2->text());
    QJsonArray editArray;
    if (isEditAppendixTable){
        editArray = timeTableJson["appendixTables"][key].toArray();
    }else{
        editArray = timeTableJson[key].toArray();
    }
    editArray.append(insertJson);

    // for (int x = 0;x<editArray.count()-1;x++){
    //     for (int y = x+1;y<editArray.count();y++){
    //         if (QTime::fromString(editArray[y].toObject().value("start").toString()) < QTime::fromString(editArray[x].toObject().value("start").toString())){
    //             min_index = y;
    //         }
    //     }
    //     QJsonObject max_Object = editArray[x].toObject();
    //     editArray[x] = editArray[min_index];
    //     editArray[min_index] = max_Object;
    // }

    for (int i = 0; i < editArray.count() - 1; i++) {	// 操作i至len-1个数据（剩下最后一个不需要操作）
        int index = i;	// 赋初值给索引
        for (int j = i + 1; j < editArray.count(); j++) {	// 比较剩余未排序的数据
            if (getTodayTime(editArray[j].toObject().value("start").toString()) < getTodayTime(editArray[index].toObject().value("start").toString())) {	// 当剩余的数据有比索引对应的数小时，更新索引
                index = j;
            }
        }
        // 当索引不等于初值时
        if (index != i) {
            // 交换数据
            QJsonObject temp = editArray[index].toObject();
            editArray[index] = editArray[i];
            editArray[i] = temp;
        }
    }
    if (isEditAppendixTable){
        QJsonObject appendixTablesObj = timeTableJson["appendixTables"].toObject();
        appendixTablesObj[key] = editArray;
        timeTableJson["appendixTables"] = appendixTablesObj;
    }else{
        timeTableJson[key] = editArray;
    }
    QFile configFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    configFile.open(QFile::WriteOnly);
    QJsonDocument tempDoc;
    tempDoc.setObject(timeTableJson);
    configFile.write(tempDoc.toJson(QJsonDocument::Indented));
    configFile.close();
    if (isEditAppendixTable){
        refechTableWidget(timeTableJson["appendixTables"].toObject()[currentEditAppendixTableName].toArray());
    }else{
        toggleded();
    }


}


void TableEditWidget::on_pushButton_clicked()
{
    if (ui->radioButton->isChecked()){
        addItem("Mon");
    }else
    if (ui->radioButton_2->isChecked()){
        addItem("Tue");
    }else
    if (ui->radioButton_3->isChecked()){
        addItem("Wed");
    }else
    if (ui->radioButton_4->isChecked()){
        addItem("Thu");
    }else
    if (ui->radioButton_5->isChecked()){
        addItem("Fri");
    }else if(ui->radioButton_6->isChecked()){
        addItem(currentEditAppendixTableName);
    }
}

void TableEditWidget::on_deleteButton_clicked()
{
    auto result = QMessageBox::question(this,"提示","确定删除吗？");
    if (result == QMessageBox::No) return;
    QString key;
    if (ui->radioButton->isChecked()){
        key = "Mon";
    }else
    if (ui->radioButton_2->isChecked()){
        key = "Tue";
    }else
    if (ui->radioButton_3->isChecked()){
        key = "Wed";
    }else
    if (ui->radioButton_4->isChecked()){
        key = "Thu";
    }else
    if (ui->radioButton_5->isChecked()){
        key = "Tri";
    }else if(ui->radioButton_6->isChecked()){
        key = currentEditAppendixTableName;
    }
    QJsonArray editArray;
    if (ui->tableWidget->currentRow() == -1)
    {
        QMessageBox::critical(this,"错误","未选择项！请选择一行删除");
    }
    if (isEditAppendixTable){
        editArray = timeTableJson["appendixTables"][key].toArray();
    }else{
        editArray = timeTableJson[key].toArray();
    }
    editArray.removeAt(ui->tableWidget->currentRow());
    if (isEditAppendixTable){
        QJsonObject appendixTablesObj = timeTableJson["appendixTables"].toObject();
        appendixTablesObj[key] = editArray;
        timeTableJson["appendixTables"] = appendixTablesObj;
    }else{
        timeTableJson[key] = editArray;
    }
    QFile configFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    configFile.open(QFile::WriteOnly);
    QJsonDocument tempDoc;
    tempDoc.setObject(timeTableJson);
    configFile.write(tempDoc.toJson(QJsonDocument::Indented));
    configFile.close();
    if (isEditAppendixTable){
        refechTableWidget(timeTableJson["appendixTables"].toObject()[currentEditAppendixTableName].toArray());
    }else{
        toggleded();
    }
}

void TableEditWidget::saveBoolConfig(const QString &key, bool value)
{
    configJson[key] = value;
    QFile configFile(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    configFile.open(QFile::WriteOnly);
    QJsonDocument tempDoc;
    tempDoc.setObject(configJson);
    configFile.write(tempDoc.toJson(QJsonDocument::Indented));
    configFile.close();
    QMessageBox::information(this, tr("提示"), tr("重启生效"));
}

void TableEditWidget::on_checkBox_2_clicked(bool checked)
{
    saveBoolConfig("zuan_status", checked);
}


void TableEditWidget::on_checkBox_clicked(bool checked)
{
    saveBoolConfig("muyu_status", checked);
}


void TableEditWidget::on_chkHide_clicked(bool checked)
{
    saveBoolConfig("toolbox_status", checked);
}

void TableEditWidget::_startUpdateTool() {
    //system(QString("start %1\\updateTool \"v%2\"").arg(QDir::homePath() + "/ClassTopLand_Data").arg(APP_VERSION).toStdWString().c_str());
    return;

}
void TableEditWidget::on_cellChanged(int row, int column) {
    ui->tableWidget->blockSignals(true);
    QJsonArray currentTable;
    QString currentTableName;
    if (ui->radioButton->isChecked()) {
        currentTable = timeTableJson["Mon"].toArray();
        currentTableName = "Mon";
    }
    else if (ui->radioButton_2->isChecked()) {
        currentTable = timeTableJson["Tue"].toArray();
        currentTableName = "Tue";
    }
    else if (ui->radioButton_3->isChecked()) {
        currentTable = timeTableJson["Wed"].toArray();
        currentTableName = "Wed";
    }
    else if (ui->radioButton_4->isChecked()) {
        currentTable = timeTableJson["Thu"].toArray();
        currentTableName = "Thu";
    }
    else if (ui->radioButton_5->isChecked()) {
        currentTable = timeTableJson["Fri"].toArray();
        currentTableName = "Fri";
    }
    else if (ui->radioButton_6->isChecked()) {
        currentTable = timeTableJson["appendixTables"].toObject()[currentEditAppendixTableName].toArray();
        currentTableName =  currentEditAppendixTableName;
    }
    switch (column) {
        case 0: {
            QJsonObject currentClass = currentTable[row].toObject();
            currentClass["name"] = ui->tableWidget->item(row, column)->text();
            currentTable[row] = currentClass;
            break;
        }
        case 1: {
            QJsonObject currentClass = currentTable[row].toObject();
            currentClass["start"] = ui->tableWidget->item(row, column)->text();
            currentTable[row] = currentClass;
            break;
        }
        case 2: {
            QJsonObject currentClass = currentTable[row].toObject();
            currentClass["end"] = ui->tableWidget->item(row, column)->text();
            currentTable[row] = currentClass;
            break;
        }
        case 3: {
            QJsonObject currentClass = currentTable[row].toObject();
            currentClass["split"] = ui->tableWidget->item(row, column)->text();
            currentTable[row] = currentClass;
            break;
        }
    }
    if (!isEditAppendixTable) {
        timeTableJson[currentTableName] = currentTable;
    }
    else {
        QJsonObject appendixTables = timeTableJson["appendixTables"].toObject();
        appendixTables[currentTableName] = currentTable;
        timeTableJson["appendixTables"] = appendixTables;
    }
    QFile configFile(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    configFile.open(QFile::WriteOnly);
    QJsonDocument tempDoc;
    tempDoc.setObject(timeTableJson);
    configFile.write(tempDoc.toJson(QJsonDocument::Indented));
    configFile.close();
    if (isEditAppendixTable) {
        refechTableWidget(timeTableJson["appendixTables"].toObject()[currentEditAppendixTableName].toArray());
    }
    else {
        toggleded();
    }
    ui->tableWidget->blockSignals(false);
}