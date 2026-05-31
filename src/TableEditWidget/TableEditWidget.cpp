#include "./TableEditWidget.h"
#include "ui_TableEditWidget.h"


TableEditWidget::TableEditWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableEditWidget)
{
    ui->setupUi(this);
    connect(ui->radioButton,SIGNAL(toggled(bool)),this,SLOT(toggleded()));
    connect(ui->radioButton_2,SIGNAL(toggled(bool)),this,SLOT(toggleded()));
    connect(ui->radioButton_3,SIGNAL(toggled(bool)),this,SLOT(toggleded()));
    connect(ui->radioButton_4,SIGNAL(toggled(bool)),this,SLOT(toggleded()));
    connect(ui->radioButton_5,SIGNAL(toggled(bool)),this,SLOT(toggleded()));
    connect(ui->label,&ClickLabel::clicked,this,[=]{
        if (m_iClickcnt >=10){
            m_iClickcnt=0;
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
                QString hash_value = hasharray.toHex();
                qDebug() << hash_value;
                if (hash_value == "a8c97315e9aa9eed727ae5aa9515e2a27d4df30cc68c4a210fa7b2d3c4e3ea20") {
                    QDesktopServices::openUrl(QUrl("https://www.bilibili.com/video/BV1wv411Y7YN"));
                }else{
                   QMessageBox::critical(this,"错误","调试码错误！");
                }
            }

        }else{
            m_iClickcnt++;
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
    QTranslator translator;
    QLocale::Language lab = QLocale::system().language();
    if(QLocale::Chinese == lab)
    {
        translator.load(":/lang/lang_cn.qm");
        qApp->installTranslator(&translator);
        ui->retranslateUi(this);
    }else if(QLocale::English== lab){
        translator.load(":/language/lang_en.qm");
        qApp->installTranslator(&translator);
        ui->retranslateUi(this);
    }
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
    emit refechTable_signal();
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

void TableEditWidget::on_editAppendixTable(QString table_name) {
    ui->radioButton_6->setChecked(true);
    m_bIsEditAppendixTable = true;
    m_sCurrentEditAppendixTableName = table_name;
    refechTableWidget(m_joTimeTable["appendixTables"].toObject()[table_name].toArray());
}

void TableEditWidget::setConfig(QJsonObject obj){
    m_joConfig=obj;
    ui->timer_hide->setChecked(m_joConfig.value("disable_timer").toBool());
    ui->timer_time->setDateTime(QDateTime::fromString(m_joConfig["end_time"].toString(),"yyyy-MM-dd hh:mm:ss"));
    ui->edit_name->setText(m_joConfig["label_tag"].toString());
    ui->edit_name_eng->setText(m_joConfig["english_tag"].toString());
}
void TableEditWidget::on_timerInfo_changed(){
    m_joConfig["end_time"] = ui->timer_time->dateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_joConfig["label_tag"] = ui->edit_name->text();
    m_joConfig["english"] = QString("There are () $\nleft until %1").arg(ui->edit_name_eng->text());
    m_joConfig["english_end"] = QString("There is not a $\nleft until %1").arg(ui->edit_name_eng->text());
    m_joConfig["english_tag"] = ui->edit_name_eng->text();
    m_joConfig["disable_timer"] = ui->timer_hide->isChecked();
    QFile config_file(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    config_file.open(QFile::WriteOnly);
    QJsonDocument temp_doc;
    temp_doc.setObject(m_joConfig);
    config_file.write(temp_doc.toJson(QJsonDocument::Indented));
    config_file.close();
    QMessageBox::information(this,tr("提示"),tr("重启生效"));
}

void TableEditWidget::readTableJson(){
    QFile file(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    file.open(QIODevice::ReadWrite | QIODevice::Text);


    QTextStream stream(&file);
    QString file_str = stream.readAll();
    file.close();
    QJsonParseError jsonError;
    QJsonDocument jsondoc = QJsonDocument::fromJson(file_str.toUtf8(),&jsonError);
    if (jsonError.error != QJsonParseError::NoError && !jsondoc.isNull()) {
        showLog("table.json is Error!",LogStatus::ERR);
        return;
    }
    m_joTimeTable = jsondoc.object();


}
void TableEditWidget::refechTableWidget(QJsonArray today_table){

    ui->tableWidget->clear();
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "课程" << "上课时间" << "下课时间" << "课间时间");
    ui->tableWidget->setRowCount(today_table.count());
    for (int x = 0; x<today_table.count();x++){
        QJsonObject value_object = today_table.at(x).toObject();
        QString name = value_object.value("name").toString();
        QString start_time = value_object.value("start").toString();
        QString end_time = value_object.value("end").toString();
        int split_time = value_object.value("split").toInt();
        ui->tableWidget->setItem(x,0,new QTableWidgetItem(name));
        ui->tableWidget->setItem(x,1,new QTableWidgetItem(start_time));
        ui->tableWidget->setItem(x,2,new QTableWidgetItem(end_time));
        ui->tableWidget->setItem(x,3,new QTableWidgetItem(QString::number(split_time)));
    }
}
void TableEditWidget::toggleded(){

    if (ui->radioButton->isChecked()){
        m_bIsEditAppendixTable = false;
        refechTableWidget(m_joTimeTable.value("Mon").toArray());
    }else
    if (ui->radioButton_2->isChecked()){
        m_bIsEditAppendixTable = false;
        refechTableWidget(m_joTimeTable.value("Tue").toArray());
    }else
    if (ui->radioButton_3->isChecked()){
        m_bIsEditAppendixTable = false;
        refechTableWidget(m_joTimeTable.value("Wed").toArray());
    }else
    if (ui->radioButton_4->isChecked()){
        m_bIsEditAppendixTable = false;
        refechTableWidget(m_joTimeTable.value("Thu").toArray());
    }else
    if (ui->radioButton_5->isChecked()){
        m_bIsEditAppendixTable = false;
        refechTableWidget(m_joTimeTable.value("Fri").toArray());
    }
}
void TableEditWidget::addItem(QString key){
    QJsonObject insert_json;
    insert_json.insert("name",ui->lineEdit->text());
    insert_json.insert("start",ui->timeEdit->text());
    insert_json.insert("end",ui->timeEdit_2->text());
    QJsonArray editarray;
    if (m_bIsEditAppendixTable){
        editarray = m_joTimeTable["appendixTables"][key].toArray();
    }else{
        editarray = m_joTimeTable[key].toArray();
    }
    editarray.append(insert_json);

    // for (int x = 0;x<editarray.count()-1;x++){
    //     for (int y = x+1;y<editarray.count();y++){
    //         if (QTime::fromString(editarray[y].toObject().value("start").toString()) < QTime::fromString(editarray[x].toObject().value("start").toString())){
    //             min_index = y;
    //         }
    //     }
    //     QJsonObject max_Object = editarray[x].toObject();
    //     editarray[x] = editarray[min_index];
    //     editarray[min_index] = max_Object;
    // }

    for (int i = 0; i < editarray.count() - 1; i++) {	// 操作i至len-1个数据（剩下最后一个不需要操作）
        int index = i;	// 赋初值给索引
        for (int j = i + 1; j < editarray.count(); j++) {	// 比较剩余未排序的数据
            if (getTodayTime(editarray[j].toObject().value("start").toString()) < getTodayTime(editarray[index].toObject().value("start").toString())) {	// 当剩余的数据有比索引对应的数小时，更新索引
                index = j;
            }
        }
        // 当索引不等于初值时
        if (index != i) {
            // 交换数据
            QJsonObject temp = editarray[index].toObject();
            editarray[index] = editarray[i];
            editarray[i] = temp;
        }
    }
    if (m_bIsEditAppendixTable){
        QJsonObject bfatable = m_joTimeTable["appendixTables"].toObject();
        bfatable[key] = editarray;
        m_joTimeTable["appendixTables"] = bfatable;
    }else{
        m_joTimeTable[key] = editarray;
    }
    QFile config_file(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    config_file.open(QFile::WriteOnly);
    QJsonDocument temp_doc;
    temp_doc.setObject(m_joTimeTable);
    config_file.write(temp_doc.toJson(QJsonDocument::Indented));
    config_file.close();
    if (m_bIsEditAppendixTable){
        refechTableWidget(m_joTimeTable["appendixTables"].toObject()[m_sCurrentEditAppendixTableName].toArray());
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
        addItem(m_sCurrentEditAppendixTableName);
    }
}






void TableEditWidget::on_checkBox_2_clicked(bool checked)
{
    m_joConfig["zuan_status"] = checked;
    QFile config_file(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    config_file.open(QFile::WriteOnly);
    QJsonDocument temp_doc;
    temp_doc.setObject(m_joConfig);
    config_file.write(temp_doc.toJson(QJsonDocument::Indented));
    config_file.close();
    QMessageBox::information(this,tr("提示"),tr("重启生效"));

}


void TableEditWidget::on_checkBox_clicked(bool checked)
{
    m_joConfig["muyu_status"] = checked;
    QFile config_file(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    config_file.open(QFile::WriteOnly);
    QJsonDocument temp_doc;
    temp_doc.setObject(m_joConfig);
    config_file.write(temp_doc.toJson(QJsonDocument::Indented));
    config_file.close();
    QMessageBox::information(this,tr("提示"),tr("重启生效"));
}


void TableEditWidget::on_chkHide_clicked(bool checked)
{
    m_joConfig["toolbox_status"] = checked;
    QFile config_file(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    config_file.open(QFile::WriteOnly);
    QJsonDocument temp_doc;
    temp_doc.setObject(m_joConfig);
    config_file.write(temp_doc.toJson(QJsonDocument::Indented));
    config_file.close();
    QMessageBox::information(this,tr("提示"),tr("重启生效"));
}

void TableEditWidget::_startUpdateTool() {
    //system(QString("start %1\\updateTool \"v%2\"").arg(QDir::homePath() + "/ClassTopLand_Data").arg(APP_VERSION).toStdWString().c_str());
    return;

}
void TableEditWidget::on_cellChanged(int row, int column) {
    ui->tableWidget->blockSignals(true);
    QJsonArray current_table;
    QString current_table_name;
    if (ui->radioButton->isChecked()) {
        current_table = m_joTimeTable["Mon"].toArray();
        current_table_name = "Mon";
    }
    else if (ui->radioButton_2->isChecked()) {
        current_table = m_joTimeTable["Tue"].toArray();
        current_table_name = "Tue";
    }
    else if (ui->radioButton_3->isChecked()) {
        current_table = m_joTimeTable["Wed"].toArray();
        current_table_name = "Wed";
    }
    else if (ui->radioButton_4->isChecked()) {
        current_table = m_joTimeTable["Thu"].toArray();
        current_table_name = "Thu";
    }
    else if (ui->radioButton_5->isChecked()) {
        current_table = m_joTimeTable["Fri"].toArray();
        current_table_name = "Fri";
    }
    else if (ui->radioButton_6->isChecked()) {
        current_table = m_joTimeTable["appendixTables"].toObject()[m_sCurrentEditAppendixTableName].toArray();
        current_table_name =  m_sCurrentEditAppendixTableName;
    }
    switch (column) {
        case 0: {
            QJsonObject current_class = current_table[row].toObject();
            current_class["name"] = ui->tableWidget->item(row, column)->text();
            current_table[row] = current_class;
            break;
        }
        case 1: {
            QJsonObject current_class = current_table[row].toObject();
            current_class["start"] = ui->tableWidget->item(row, column)->text();
            current_table[row] = current_class;
            break;
        }
        case 2: {
            QJsonObject current_class = current_table[row].toObject();
            current_class["end"] = ui->tableWidget->item(row, column)->text();
            current_table[row] = current_class;
            break;
        }
        case 3: {
            QJsonObject current_class = current_table[row].toObject();
            current_class["split"] = ui->tableWidget->item(row, column)->text();
            current_table[row] = current_class;
            break;
        }
    }
    if (!m_bIsEditAppendixTable) {
        m_joTimeTable[current_table_name] = current_table;
    }
    else {
        QJsonObject appendix_tables = m_joTimeTable["appendixTables"].toObject();
        appendix_tables[current_table_name] = current_table;
        m_joTimeTable["appendixTables"] = appendix_tables;
    }
    QFile config_file(QDir::homePath() + "/ClassTopLand_Data" + "/tables.json");
    config_file.open(QFile::WriteOnly);
    QJsonDocument temp_doc;
    temp_doc.setObject(m_joTimeTable);
    config_file.write(temp_doc.toJson(QJsonDocument::Indented));
    config_file.close();
    if (m_bIsEditAppendixTable) {
        refechTableWidget(m_joTimeTable["appendixTables"].toObject()[m_sCurrentEditAppendixTableName].toArray());
    }
    else {
        toggleded();
    }
    ui->tableWidget->blockSignals(false);
}