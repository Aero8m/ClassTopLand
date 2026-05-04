#include "./ToolBox.h"
#include "ui_ToolBox.h"
#include "../GlassHelper/GlassHelper.h"
ToolBox::ToolBox(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ToolBox)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowType::FramelessWindowHint | Qt::WindowType::Tool | Qt::WindowType::WindowStaysOnBottomHint);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground);
    LoadPlugins();

    this->setAcceptDrops(true);
    this->installEventFilter(this);
    GlassHelper::enableBlurBehind(this,0x11000000);
    QFile file(QDir::homePath() + "/ClassTopLand_Data" + "/config.json");
    file.open(QIODevice::ReadWrite | QIODevice::Text);

    QTextStream stream(&file);
    QString file_str = stream.readAll();
    file.close();
    QJsonParseError jsonError;
    QJsonDocument jsondoc = QJsonDocument::fromJson(file_str.toUtf8(),&jsonError);
    if (jsonError.error != QJsonParseError::NoError && !jsondoc.isNull()) {
        showLog("Config.json is Error!",LogStatus::ERR);
        return;
    }
    bool zuanyanisopen = false;
    if (jsondoc.object().contains("zuan_status"))
    {
        zuanyanisopen = jsondoc.object().value("zuan_status").toBool();
    }
    QJsonObject Config = jsondoc.object();
    yiyan = new yiyanDialog(this);
    yiyan->setConfig(Config);
    yiyan->setZuanYanOpen(false);

}

ToolBox::~ToolBox()
{
    delete ui;
}

void ToolBox::LoadPlugins(){
    for (auto x:findChildren<ClickLabel*>()){
        delete x;
    }
    QDir plugin_dir(QDir::homePath() + "/ClassTopLand_Data" + "/plugins");
    if (!plugin_dir.exists()){
        plugin_dir.mkdir(QDir::homePath() + "/ClassTopLand_Data" + "/plugins");
        return;
    }
    QFileInfoList fileList = plugin_dir.entryInfoList();
    ClickLabel *PluginBut = new ClickLabel(this);
    ui->verticalLayout->addWidget(PluginBut);
    connect(PluginBut,&ClickLabel::clicked,this,[=]
    {
#ifdef __linux__
        QScreen *screen = QApplication::primaryScreen();
        // 逻辑尺寸
        QSize logicalSize = screen->size();  // 或 screen->availableSize()
        qreal ratio = screen->devicePixelRatio();
        QSize physicalSize = logicalSize * ratio;
        int scr_w = physicalSize.width();
        int scr_h = physicalSize.height();
#else
        QScreen *scr = qApp->primaryScreen();
int scr_w = scr->size().width();
int scr_h = scr->size().height();
#endif
        yiyan->move((scr_w - yiyan->width()) / 2, (scr_h - yiyan->height()) / 3);
        yiyan->setModal(false);
        GlassHelper::enableBlurBehind(yiyan);
        yiyan->show();
    });
    PluginBut->setMinimumSize(QSize(50,50));
    PluginBut->setAniOpen(true);
    PluginBut->setCursor(Qt::PointingHandCursor);
    PluginBut->setStyleSheet("border-image: url(:/res/speak.png);");
    adjustSize();
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
        ClickLabel *PluginBut = new ClickLabel(this);
        ui->verticalLayout->addWidget(PluginBut);

        PluginBut->setMinimumSize(QSize(50,50));
        PluginBut->setAniOpen(true);
        PluginBut->setCursor(Qt::PointingHandCursor);
        adjustSize();

        PluginBut->setStyleSheet(QString("image: url(\"%1\");").arg(x.filePath() + "/" + PluginObject["icon"].toString()));
        if (PluginObject["type"].toString() == "win32"){

            connect(PluginBut,&ClickLabel::clicked,this,[=]{
                system(QString("%1/%2").arg(x.filePath()).arg(PluginObject["execPath"].toString()).toStdString().c_str());
            });
        }
        if (PluginObject["type"].toString() == "link"){

            connect(PluginBut,&ClickLabel::clicked,this,[=]{
                if (PluginObject.contains("realPath")){
                    QFileInfo info(PluginObject["realPath"].toString());
                    if (info.exists()){
                        system(PluginObject["realPath"].toString().toStdString().c_str());
                    }

                }else {
                    QFileInfo info(PluginObject["linkPath"].toString());
                    if (info.exists() && info.isSymLink()){
                        system(info.symLinkTarget().toStdString().c_str());
                    }
                }

            });
        }

    }
}
