#include "./TableEditWidget.h"
#include "ui_TableEditWidget.h"
#include "../CSESTableProcessor/CSESTableProcessor.h"
#include <QDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSignalBlocker>
#include <exception>


QTime parseClassTime(const QString &text)
{
    static const QRegularExpression timePattern(
        QStringLiteral(R"(^([01]?\d|2[0-3]):([0-5]\d)$)"));
    const QRegularExpressionMatch match = timePattern.match(text.trimmed());
    if (!match.hasMatch())
    {
        return {};
    }
    return QTime(match.captured(1).toInt(), match.captured(2).toInt());
}

void sortClassesByStartTime(QJsonArray &classes)
{
    for (qsizetype i = 0; i + 1 < classes.size(); ++i)
    {
        qsizetype earliestIndex = i;
        QTime earliestTime = parseClassTime(classes[i].toObject()["start"].toString());
        for (qsizetype j = i + 1; j < classes.size(); ++j)
        {
            const QTime candidateTime = parseClassTime(classes[j].toObject()["start"].toString());
            if (candidateTime.isValid() && (!earliestTime.isValid() || candidateTime < earliestTime))
            {
                earliestIndex = j;
                earliestTime = candidateTime;
            }
        }
        if (earliestIndex != i)
        {
            const QJsonValue currentValue = classes[i];
            classes[i] = classes[earliestIndex];
            classes[earliestIndex] = currentValue;
        }
    }
}

namespace {
QString tableFilePath()
{
    return QDir::homePath() + QStringLiteral("/ClassTopLand_Data/tables.json");
}

bool saveFileAtomically(const QString &path, const QByteArray &content, QString *error)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        *error = file.errorString();
        return false;
    }
    if (file.write(content) != content.size())
    {
        *error = file.errorString();
        file.cancelWriting();
        return false;
    }
    if (!file.commit())
    {
        *error = file.errorString();
        return false;
    }
    return true;
}
}


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
    connect(ui->radioButton_7,&QRadioButton::toggled, this, &TableEditWidget::toggleded);
    connect(ui->radioButton_8,&QRadioButton::toggled, this, &TableEditWidget::toggleded);
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
    connect(ui->importCSES, &QPushButton::clicked, this, &TableEditWidget::importCSESTable);
    connect(ui->outputCSES, &QPushButton::clicked, this, &TableEditWidget::exportCSESTable);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->label_4->setText("Build " + QString(APP_VERSION));
    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &TableEditWidget::on_cellChanged);
    ui->tabWidget->setTabPosition(QTabWidget::West);
}

TableEditWidget::~TableEditWidget()
{

    delete ui;
}
void TableEditWidget::showEvent(QShowEvent*){
    readTableJson();
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
    readTableJson();
    ui->radioButton_6->setChecked(true);
    isEditAppendixTable = true;
    currentEditAppendixTableName = tableName;
    refechTableWidget(timeTableJson["appendixTables"].toObject()[tableName].toArray());
}

void TableEditWidget::setConfig(QJsonObject obj){
    configJson=obj;
    ui->timer_hide->setChecked(configJson.value("disable_timer").toBool());
    ui->timer_time->setDateTime(QDateTime::fromString(configJson["end_time"].toString(),"yyyy-MM-dd hh:mm:ss"));
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

void TableEditWidget::importCSESTable()
{
    const QString sourcePath = QFileDialog::getOpenFileName(
        this, tr("导入 CSES 课表"), QDir::homePath(),
        tr("CSES YAML 文件 (*.yaml *.yml);;所有文件 (*)"));
    if (sourcePath.isEmpty())
    {
        return;
    }

    QFile source(sourcePath);
    if (!source.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("导入失败"),
                              tr("无法读取文件：%1").arg(source.errorString()));
        return;
    }
    const QByteArray yamlText = source.readAll();
    if (source.error() != QFileDevice::NoError)
    {
        QMessageBox::critical(this, tr("导入失败"),
                              tr("读取文件失败：%1").arg(source.errorString()));
        return;
    }

    QJsonObject importedTable;
    try
    {
        importedTable = CSESTableProcessor::parseCSESYaml(yamlText);
    }
    catch (const std::exception &error)
    {
        QMessageBox::critical(this, tr("导入失败"),
                              tr("CSES 文件无效：%1").arg(QString::fromUtf8(error.what())));
        return;
    }

    if (QMessageBox::question(this, tr("确认导入"),
                              tr("导入将覆盖现有的全部星期课表和附加课表，确定继续吗？"),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
    {
        return;
    }

    QString writeError;
    if (!saveFileAtomically(tableFilePath(), QJsonDocument(importedTable).toJson(QJsonDocument::Indented),
                            &writeError))
    {
        QMessageBox::critical(this, tr("导入失败"),
                              tr("保存课表失败：%1").arg(writeError));
        return;
    }

    timeTableJson = importedTable;
    isEditAppendixTable = false;
    currentEditAppendixTableName.clear();
    if (ui->radioButton_6->isChecked())
    {
        ui->radioButton->setChecked(true);
    }
    toggleded();
    emit refetchTableSignal();
    QMessageBox::information(this, tr("导入成功"), tr("CSES 课表已导入并应用。"));
}

void TableEditWidget::exportCSESTable()
{
    QFileDialog dialog(this, tr("导出 CSES 课表"), QDir::homePath());
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter(tr("CSES YAML 文件 (*.yaml *.yml)"));
    dialog.setDefaultSuffix(QStringLiteral("yaml"));
    dialog.selectFile(QStringLiteral("ClassTopLand_CSES.yaml"));
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }
    const QStringList selectedFiles = dialog.selectedFiles();
    if (selectedFiles.isEmpty() || selectedFiles.first().isEmpty())
    {
        return;
    }
    const QString outputPath = selectedFiles.first();
    const QString localPath = tableFilePath();
    const QFileInfo outputInfo(outputPath);
    if (outputInfo.exists() && outputInfo.canonicalFilePath() == QFileInfo(localPath).canonicalFilePath())
    {
        QMessageBox::critical(this, tr("导出失败"), tr("不能将 YAML 导出到当前的 tables.json。"));
        return;
    }

    QFile localFile(localPath);
    if (!localFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("导出失败"),
                              tr("无法读取本地课表：%1").arg(localFile.errorString()));
        return;
    }
    const QByteArray jsonText = localFile.readAll();
    if (localFile.error() != QFileDevice::NoError)
    {
        QMessageBox::critical(this, tr("导出失败"),
                              tr("读取本地课表失败：%1").arg(localFile.errorString()));
        return;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(jsonText, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        QMessageBox::critical(this, tr("导出失败"),
                              tr("本地 tables.json 解析失败：%1").arg(parseError.errorString()));
        return;
    }
    if (!document.isObject())
    {
        QMessageBox::critical(this, tr("导出失败"), tr("本地 tables.json 不是有效的课表对象。"));
        return;
    }

    QByteArray yamlText;
    try
    {
        yamlText = CSESTableProcessor::serializeCSESYaml(document.object());
    }
    catch (const std::exception &error)
    {
        QMessageBox::critical(this, tr("导出失败"),
                              tr("课表无法转换为 CSES：%1").arg(QString::fromUtf8(error.what())));
        return;
    }

    QString writeError;
    if (!saveFileAtomically(outputPath, yamlText, &writeError))
    {
        QMessageBox::critical(this, tr("导出失败"),
                              tr("保存文件失败：%1").arg(writeError));
        return;
    }
    QMessageBox::information(this, tr("导出成功"), tr("CSES 课表已保存。"));
}

void TableEditWidget::refechTableWidget(QJsonArray todayTable){
    const QSignalBlocker blocker(ui->tableWidget);
    ui->tableWidget->clear();
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "课程" << "上课时间" << "下课时间");
    ui->tableWidget->setRowCount(todayTable.count());
    for (int x = 0; x<todayTable.count();x++){
        QJsonObject valueObject = todayTable.at(x).toObject();
        QString name = valueObject.value("name").toString();
        QString startTime = valueObject.value("start").toString();
        QString endTime = valueObject.value("end").toString();
        ui->tableWidget->setItem(x,0,new QTableWidgetItem(name));
        ui->tableWidget->setItem(x,1,new QTableWidgetItem(startTime));
        ui->tableWidget->setItem(x,2,new QTableWidgetItem(endTime));
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
    }else
    if (ui->radioButton_7->isChecked()){
        isEditAppendixTable = false;
        refechTableWidget(timeTableJson.value("Sat").toArray());
    }else
    if (ui->radioButton_8->isChecked()){
        isEditAppendixTable = false;
        refechTableWidget(timeTableJson.value("Sun").toArray());
    }else
    if (ui->radioButton_6->isChecked() && isEditAppendixTable){
        refechTableWidget(timeTableJson["appendixTables"].toObject()[currentEditAppendixTableName].toArray());
    }
}

CourseValidationResult TableEditWidget::validateCourse(const QJsonObject &course)
{
    if (!course.value("name").isString() ||
        !course.value("start").isString() ||
        !course.value("end").isString())
    {
        return {
            false,
            {},
            tr("课程必须包含字符串类型的 name、start 和 end")
        };
    }

    const QString name = course.value("name").toString().trimmed();
    const QString startText = course.value("start").toString().trimmed();
    const QString endText = course.value("end").toString().trimmed();

    if (name.isEmpty())
    {
        return {false, {}, tr("课程名称不能为空")};
    }

    static const QRegularExpression timePattern(
        QStringLiteral(R"(^(?:[01]\d|2[0-3]):[0-5]\d$)"));
    if (!timePattern.match(startText).hasMatch() ||
        !timePattern.match(endText).hasMatch())
    {
        return {false, {}, tr("课程时间必须使用 HH:mm 格式")};
    }

    const QTime startTime = QTime::fromString(startText, QStringLiteral("HH:mm"));
    const QTime endTime = QTime::fromString(endText, QStringLiteral("HH:mm"));
    if (!startTime.isValid() || !endTime.isValid())
    {
        return {false, {}, tr("课程时间无效")};
    }
    if (startTime >= endTime)
    {
        return {false, {}, tr("上课时间必须早于下课时间")};
    }

    return {
        true,
        QJsonObject{
                {"name", name},
                {"start", startTime.toString(QStringLiteral("HH:mm"))},
                {"end", endTime.toString(QStringLiteral("HH:mm"))}
        },
        {}
    };
}

void TableEditWidget::addItem(QString key){
    const QJsonObject candidateCourse{
        {"name", ui->lineEdit->text()},
        {"start", ui->timeEdit->time().toString(QStringLiteral("HH:mm"))},
        {"end", ui->timeEdit_2->time().toString(QStringLiteral("HH:mm"))}
    };
    const CourseValidationResult validationResult = validateCourse(candidateCourse);
    if (!validationResult.valid)
    {
        QMessageBox::critical(this, tr("错误"), validationResult.error);
        return;
    }

    readTableJson();
    QJsonArray editArray;
    if (isEditAppendixTable){
        editArray = timeTableJson["appendixTables"][key].toArray();
    }else{
        editArray = timeTableJson[key].toArray();
    }
    editArray.append(validationResult.normalizedCourse);
    sortClassesByStartTime(editArray);
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
    }else
    if (ui->radioButton_7->isChecked()){
        addItem("Sat");
    }else
    if (ui->radioButton_8->isChecked()){
        addItem("Sun");
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
        key = "Fri";
    }else
    if (ui->radioButton_7->isChecked()){
        key = "Sat";
    }else
    if (ui->radioButton_8->isChecked()){
        key = "Sun";
    }else if(ui->radioButton_6->isChecked()){
        key = currentEditAppendixTableName;
    }
    QJsonArray editArray;
    if (ui->tableWidget->currentRow() == -1)
    {
        QMessageBox::critical(this,"错误","未选择项！请选择一行删除");
        return;
    }
    readTableJson();
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

void TableEditWidget::on_cellChanged(int row, int column) {
    const QSignalBlocker blocker(ui->tableWidget);
    readTableJson();
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
    else if (ui->radioButton_7->isChecked()) {
        currentTable = timeTableJson["Sat"].toArray();
        currentTableName = "Sat";
    }
    else if (ui->radioButton_8->isChecked()) {
        currentTable = timeTableJson["Sun"].toArray();
        currentTableName = "Sun";
    }
    else if (ui->radioButton_6->isChecked()) {
        currentTable = timeTableJson["appendixTables"].toObject()[currentEditAppendixTableName].toArray();
        currentTableName =  currentEditAppendixTableName;
    }

    QTableWidgetItem *editedItem = ui->tableWidget->item(row, column);
    if (currentTableName.isEmpty() || row < 0 || row >= currentTable.size() || !editedItem)
    {
        return;
    }

    QJsonObject currentClass = currentTable[row].toObject();
    const QString editedText = editedItem->text().trimmed();
    auto rejectEdit = [this, editedItem](const QString &message, const QString &oldValue)
    {
        editedItem->setText(oldValue);
        QMessageBox::critical(this, "错误", message);
    };

    switch (column) {
        case 0: {
            if (editedText.isEmpty())
            {
                rejectEdit("课程名称不能为空！", currentClass["name"].toString());
                return;
            }
            currentClass["name"] = editedText;
            currentTable[row] = currentClass;
            break;
        }
        case 1: {
            const QTime startTime = parseClassTime(editedText);
            const QTime endTime = parseClassTime(currentClass["end"].toString());
            if (!startTime.isValid() || (endTime.isValid() && startTime >= endTime))
            {
                rejectEdit("上课时间格式应为 HH:mm，且必须早于下课时间！",
                           currentClass["start"].toString());
                return;
            }
            const QString normalizedTime = startTime.toString("HH:mm");
            editedItem->setText(normalizedTime);
            currentClass["start"] = normalizedTime;
            currentTable[row] = currentClass;
            break;
        }
        case 2: {
            const QTime startTime = parseClassTime(currentClass["start"].toString());
            const QTime endTime = parseClassTime(editedText);
            if (!endTime.isValid() || (startTime.isValid() && endTime <= startTime))
            {
                rejectEdit("下课时间格式应为 HH:mm，且必须晚于上课时间！",
                           currentClass["end"].toString());
                return;
            }
            const QString normalizedTime = endTime.toString("HH:mm");
            editedItem->setText(normalizedTime);
            currentClass["end"] = normalizedTime;
            currentTable[row] = currentClass;
            break;
        }
        default:
            return;
    }
    if (column == 1)
    {
        sortClassesByStartTime(currentTable);
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
}
