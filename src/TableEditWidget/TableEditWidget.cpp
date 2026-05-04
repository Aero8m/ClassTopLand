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
    connect(ui->btn_giteeLink, &QPushButton::clicked, this, [=] {
        QDesktopServices::openUrl(QUrl("https://gitee.com/Aero80wd/ClassTopLand"));
        });
    connect(ui->save_text_config,&QPushButton::clicked,this,&TableEditWidget::on_timerInfo_changed);
    connect(ui->weather_localtion_edit,&QLineEdit::returnPressed,this,&TableEditWidget::_searchWeatherLocaltions);
    connect(ui->save_weather_localtion,&QPushButton::clicked,this,&TableEditWidget::saveWeatherLocaltions);
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
    readPluginList();
    ui->pluginInfo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->pluginList->installEventFilter(this);
    ui->pluginList->setAcceptDrops(true);
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
void TableEditWidget::_searchWeatherLocaltions()
{
    if (ui->weather_localtion_edit->text().isEmpty()) {
        ui->weather_localtion_select->clear();
        ui->weather_localtion_select->setEnabled(false);
        ui->weather_localtion_select->addItem("请先在搜索框搜索");
        QMessageBox::critical(this, "错误", "请输入城市名称");
        return;
    }
    m_rWeatherSearchReq = NetworkRequests(GET,QUrl(QString("https://geoapi.qweather.com/v2/city/lookup?location=%1&key=b7f4a7d1bfca4f13b6f265ea8676c297").arg(ui->weather_localtion_edit->text())));
    m_rWeatherSearchReq.start();
    connect(&m_rWeatherSearchReq,&NetworkRequests::finished,this,[=](QJsonObject json,QString text,QString error_string)
    {
        showLog("Weather Request OK!",INFO);
        showLog("Weather Request Code:" + json["code"].toString(),INFO);
        showLog(text,INFO);
        if (json["code"].toString() == "200")
        {
            QJsonArray locations = json["location"].toArray();
            ui->weather_localtion_select->clear();
            for (auto x : locations)
            {
                ui->weather_localtion_select->addItem(x.toObject()["adm1"].toString() + QString(" ") + x.toObject()["adm2"].toString() + QString(" ") +x.toObject()["name"].toString(), x.toObject()["id"].toString());
            }
            ui->weather_localtion_select->setEnabled(true);
        }else
        {
            QMessageBox::critical(this,"错误","获取信息错误！");
        }
    });
}
void TableEditWidget::saveWeatherLocaltions()
{
    if (ui->weather_localtion_select->count() == 0)
    {
        QMessageBox::critical(this,"错误","您未选择城市！");
        return;
    }
    m_joConfig["weather_localtion_id"] = ui->weather_localtion_select->currentData().toString();
    m_joConfig["weather_localtion_name"] = ui->weather_localtion_select->currentText();
    QFile config_file(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    config_file.open(QFile::WriteOnly);
    QJsonDocument temp_doc;
    temp_doc.setObject(m_joConfig);
    config_file.write(temp_doc.toJson(QJsonDocument::Indented));
    config_file.close();
    QMessageBox::information(this,tr("提示"),tr("重启生效"));
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
    ui->chkHide->setChecked(m_joConfig.value("toolbox_status").toBool());
    ui->timer_hide->setChecked(m_joConfig.value("disable_timer").toBool());
    ui->timer_time->setDateTime(QDateTime::fromString(m_joConfig["end_time"].toString(),"yyyy-MM-dd hh:mm:ss"));
    ui->edit_name->setText(m_joConfig["label_tag"].toString());
    ui->edit_name_eng->setText(m_joConfig["english_tag"].toString());
    ui->weather_localtion_edit->setText(m_joConfig["weather_localtion_name"].toString());
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
void TableEditWidget::readPluginList(){
    QDir plugin_dir(QDir::homePath() + "/ClassTopLand_Data" + "/plugins");
    if (!plugin_dir.exists()){
        plugin_dir.mkdir(QDir::homePath() + "/ClassTopLand_Data" + "/plugins");
        return;
    }
    QFileInfoList fileList = plugin_dir.entryInfoList();
    ui->pluginList->clear();
    for (auto x : fileList){
        QString JsonPath = x.filePath() + "/pluginConfig.json";
        if (x.filePath()[x.filePath().size() -1] == '.'){
            continue;
        }
        QFile file(x.filePath() + "/pluginConfig.json");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QString value = file.readAll();
        file.close();
        QJsonParseError parseJsonErr;
        QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
        QJsonObject PluginObject = document.object();
        QListWidgetItem *item = new QListWidgetItem();
        item->setText(PluginObject["name"].toString());
        item->setData(Qt::UserRole,x.filePath() + "/pluginConfig.json");
        item->setData(Qt::UserRole+1,x.filePath());
        ui->pluginList->addItem(item);


    }
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



bool TableEditWidget::eventFilter(QObject *watched,QEvent*event)
{

    if (event->type() == QEvent::Drop){
        showLog("PluginFile is Drop",LogStatus::INFO);
        ui->status_bar->setText("");
        QDropEvent *drop_event = (QDropEvent*) event;
        const QMimeData *mimeData = drop_event->mimeData();

        if(!mimeData->hasUrls())
        {
            return true;
        }

        QList<QUrl> urlList = mimeData->urls();
        for (int x = 0;x<urlList.length();x++){
            QString fileName = urlList.at(x).toLocalFile();
            if(fileName.isEmpty())
            {
                return true;
            }

            if (fileName.split(".").last() == "zip"){
                QZipReader reader(fileName);
                reader.extractAll(QDir::homePath() + "/ClassTopLand_Data" + "/plugins");
                QFile file(QDir::homePath() + "/ClassTopLand_Data" + "/plugins");
                file.open(QIODevice::WriteOnly);
                file.write(reader.fileData(QDir::homePath() + "/ClassTopLand_Data" + "/plugins/" + QTime::currentTime().toString()));
                file.close();
                reader.close();
                readPluginList();
                QMessageBox::information(this,tr("提示"),tr("插件安装完成！"));
                emit refechToolBar_signal();
            }else if (fileName.split(".").last() == "lnk"){
                QFileInfo info(fileName);
                if (info.exists() && info.isSymLink()){
                    QJsonObject pluginObj;
                    pluginObj["name"] = info.baseName();
                    pluginObj["type"] = "link";
                    pluginObj["realPath"] = info.symLinkTarget();
                    pluginObj["pluginVersion"] = "0.0";
                    pluginObj["pluginAuthor"] = "";
                    pluginObj["pluginIntroduce"] = info.baseName();
                    pluginObj["icon"] = "icon.png";
                    QDir pluginDir(QDir::homePath() + "/ClassTopLand_Data" + "/plugins/" + info.baseName());
                    pluginDir.mkdir(QDir::homePath() + "/ClassTopLand_Data" + "/plugins/" + info.baseName());
                    QFile file(QDir::homePath() + "/ClassTopLand_Data" + "/plugins/" + info.baseName() + "/pluginConfig.json");
                    file.open(QIODevice::ReadWrite);
                    QJsonDocument pluginDoc;
                    pluginDoc.setObject(pluginObj);
                    file.write(pluginDoc.toJson());
                    file.close();
                    QFileInfo fileinfo(info.symLinkTarget());
                    QFileIconProvider fileicon;
                    QIcon icon=fileicon.icon(fileinfo);
                    QPixmap pixmap = icon.pixmap(256,256);
                    pixmap.save(QDir::homePath() + "/ClassTopLand_Data" + "/plugins/" + info.baseName() + "/icon.png");
                    readPluginList();
                    emit refechToolBar_signal();

                }
            }else{
                QMessageBox::critical(this,tr("错误"),tr("此文件不是插件或快捷方式！"));
                return true;
            }
        }
        QMessageBox::information(this,tr("提示"),tr("图标添加完成！"));
        return true;

    }
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent *drag_event = (QDragEnterEvent *) event;
        if (drag_event->mimeData()->hasUrls()) {
            QList<QUrl> urlList = drag_event->mimeData()->urls();
            bool isPlugin = false;
            for (int x = 0; x < urlList.length(); x++) {
                QString fileName = urlList.at(x).toLocalFile();
                if (fileName.split(".").last() == "zip" or fileName.split(".").last() == "lnk") {
                    isPlugin = true;
                } else {
                    isPlugin = false;
                    break;
                }

            }
            if (isPlugin) {
                ui->status_bar->setText("拖放以添加至工具栏");
                drag_event->acceptProposedAction();
            } else {
                ui->status_bar->setText("此文件不是插件或快捷方式！");
                drag_event->ignore();
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched,event);
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





void TableEditWidget::on_pluginList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    showLog("Reading Plugin List",LogStatus::INFO);
    if (current == nullptr){
        return;
    }
    QString config_path = current->data(Qt::UserRole).toString();
    QFile file(config_path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();
    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
    QJsonObject PluginObject = document.object();
    ui->pluginInfo->clearContents();
    ui->pluginInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->pluginInfo->setRowCount(4);
    ui->pluginInfo->setItem(0,0,new QTableWidgetItem("名称"));
    ui->pluginInfo->setItem(0,1,new QTableWidgetItem(PluginObject["name"].toString()));
    ui->pluginInfo->setItem(1,0,new QTableWidgetItem("版本"));
    ui->pluginInfo->setItem(1,1,new QTableWidgetItem(PluginObject["pluginVersion"].toString()));
    ui->pluginInfo->setItem(2,0,new QTableWidgetItem("作者"));
    ui->pluginInfo->setItem(2,1,new QTableWidgetItem(PluginObject["pluginAuthor"].toString()));
    ui->pluginInfo->setItem(3,0,new QTableWidgetItem("简介"));
    ui->pluginInfo->setItem(3,1,new QTableWidgetItem(PluginObject["pluginIntroduce"].toString()));

}


void TableEditWidget::on_pluginList_itemDoubleClicked(QListWidgetItem *item)
{
    QMessageBox MyBox(QMessageBox::Warning,"警告","将会删除此插件，是否继续？",QMessageBox::Yes | QMessageBox::No,this);
    int clickedBut = MyBox.exec();
    if (clickedBut == QMessageBox::Yes){
        QDir dir(item->data(Qt::UserRole+1).toString());
        dir.removeRecursively();

        readPluginList();
        QMessageBox::information(this,"提示","删除完成");
        emit refechToolBar_signal();
    }


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