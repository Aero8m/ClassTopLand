#include "./YiyanDialog.h"
#include "ui_YiyanDialog.h"

yiyanDialog::yiyanDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::yiyanDialog)
{
    ui->setupUi(this);


    connect(ui->label_2,&ClickLabel::clicked,this,&yiyanDialog::reject);
    connect(ui->label_5,&ClickLabel::clicked,this,&yiyanDialog::refechYiYan);
    this->setWindowFlags(Qt::FramelessWindowHint | windowFlags() | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    ui->widget->installEventFilter(this);
    ani_opty = new QPropertyAnimation(this,"windowOpacity");
    ani_opty->setDuration(500);
    ani_opty->setStartValue(0);
    ani_opty->setEndValue(1);
    back_show = nullptr;
    readConfig();
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

}

yiyanDialog::~yiyanDialog()
{
    if (back_show) {
        delete back_show;
    }
    delete ui;
}
void yiyanDialog::paintEvent(QPaintEvent *event)
{
    // QPainter painter(this);
    // painter.fillRect(this->rect().adjusted(10, 10, -10, -10), QColor(223, 223, 223));
}
bool yiyanDialog::eventFilter(QObject* watched,QEvent* event){
    if (event->type() == QEvent::MouseButtonPress and watched == ui->widget){
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->buttons() == Qt::LeftButton){
            startPos = mouse_event->pos();
            tracking = true;
            return true;
        }
    }
    if (event->type() == QEvent::MouseMove){
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);

            endPos = mouse_event->pos() - startPos;
            move(pos() + endPos - QPoint(width()/2,0));

            if (!back_show) {
                back_show = new QPropertyAnimation(this,"pos", this);
                back_show->setEasingCurve(QEasingCurve::InOutSine);
                back_show->setDuration(500);
            }
            back_show->setStartValue(pos() - QPoint(0,100));
            back_show->setEndValue(pos());
            return true;

    }
    if (event->type() == QEvent::MouseButtonRelease){
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        if (mouse_event->buttons() == Qt::LeftButton){
            tracking = false;
        }

    }
    if (event->type() == QEvent::Show){
        refechYiYan();

        if (!back_show) {
            back_show = new QPropertyAnimation(this,"pos", this);
            back_show->setEasingCurve(QEasingCurve::InOutSine);
            back_show->setDuration(500);
        }
        back_show->setStartValue(pos() - QPoint(0,100));
        back_show->setEndValue(pos());

        back_show->setDirection(QAbstractAnimation::Forward);
        ani_opty->setDirection(QAbstractAnimation::Forward);
        ani_opty->start();
        back_show->start();
        return true;
    }

    return QDialog::eventFilter(watched,event);
}
void yiyanDialog::reject(){

    back_show->setDirection(QAbstractAnimation::Backward);
    connect(back_show,&QPropertyAnimation::finished,this,[=]{
        close();
    });
    ani_opty->setDirection(QAbstractAnimation::Backward);
    ani_opty->start();
    back_show->start();
}
void yiyanDialog::closeEvent(QCloseEvent* event){

    return;
}
void yiyanDialog::readConfig(){

    if (ZuanYanisOpen){
        ZuanYanisOpen = true;
    }

}
void yiyanDialog::refechYiYan(){
        QNetworkAccessManager *smsManager = new QNetworkAccessManager(this);
        QString url = "https://v2.jinrishici.com/sentence";
        QNetworkRequest *Request = new QNetworkRequest(QUrl(url));
        if (!Config.contains("yiyan_token")){
            Config.insert("yiyan_token",getToken());
            QFile config_file(CONFIG_JSON);
            config_file.open(QFile::WriteOnly);
            QJsonDocument temp_doc;
            temp_doc.setObject(Config);
            config_file.write(temp_doc.toJson(QJsonDocument::Indented));
            config_file.close();
        }
        Request->setRawHeader("X-User-Token",Config.value("yiyan_token").toString().toUtf8());
        QByteArray responseData;
        QEventLoop eventLoop;
        connect(smsManager , SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
        QNetworkReply* initReply = smsManager ->get(*Request );
        eventLoop.exec();       //block until finish
        responseData = initReply->readAll();

        //解析json
        QString imageData;//接收到服务器的base64数据是string类型
        QString txt;
        QJsonParseError json_error;
        QJsonDocument doucment = QJsonDocument::fromJson(responseData, &json_error);
        if (json_error.error == QJsonParseError::NoError) {
            if (doucment.isObject()) {
                const QJsonObject obj = doucment.object();
                if (obj.contains("data")) {
                    QJsonObject object_data = obj.value("data").toObject();
                    ui->label_3->setText(object_data.value("content").toString());
                    QString foot_str = QString("——[%1]%2《%3》").arg(object_data.value("origin").toObject().value("dynasty").toString()).arg(object_data.value("origin").toObject().value("author").toString()).arg(object_data.value("origin").toObject().value("title").toString());
                    QJsonArray tags = object_data.value("matchTags").toArray();
                    QString tags_text = "Tags：";
                    for(int x = 0;x<tags.count();x++){
                        if (x != tags.count() - 1){
                            tags_text += tags[x].toString() + "、";

                        }else{
                            tags_text += tags[x].toString();
                        }
                    }
                    ui->show_tags->setText(tags_text);
                    ui->label_4->setText(foot_str);
                }
            }
        }
}
QString yiyanDialog::getToken(){
    QNetworkAccessManager *smsManager = new QNetworkAccessManager(this);
    QString url = "https://v2.jinrishici.com/token";
    QNetworkRequest *Request = new QNetworkRequest(QUrl(url));
    QByteArray responseData;
    QEventLoop eventLoop;
    connect(smsManager , SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QNetworkReply* initReply = smsManager ->get(*Request );
    eventLoop.exec();       //block until finish
    responseData = initReply->readAll();

    //解析json
    QString imageData;//接收到服务器的base64数据是string类型
    QString txt;
    QJsonParseError json_error;
    QJsonDocument doucment = QJsonDocument::fromJson(responseData, &json_error);
    if (json_error.error == QJsonParseError::NoError) {
        if (doucment.isObject()) {
            const QJsonObject obj = doucment.object();
            return obj.value("data").toString();
        }
    }
}

