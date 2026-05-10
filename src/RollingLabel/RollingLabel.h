#pragma once

#include <QDebug>
#include <QLabel>
#include <QTimerEvent>
#include <QPaintEvent>
#include <QTextDocument> //判断富文本用的
#include <QPainter>

// 继承于标签，之后在qt设计师内右键选择"提升" 使得这个继承类控制对应需要滚动的标签
class RollingLabel :public QLabel
{
    Q_OBJECT
public:
    explicit RollingLabel(QWidget *parent = nullptr);
    ~RollingLabel();

    //自适应函数，判断label文本是否需要滚动起来
    void upateLabelRollingState();

public slots:
    //定时改位移量，到末尾时改为开头  负责修改当前像素位移值left  startTimer开始，killTimer结束
    void timerEvent(QTimerEvent *e) Q_DECL_OVERRIDE;

    //重绘事件，根据位移量left显示文本
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;

    //在设置文本、缩放事件两次调用自适应的函数
    void setText(const QString &txt);

    //设置图片，主要把lt设回0，使其恢复正常的图片显示
    void setPixmap(const QPixmap &pix);

    //窗口变化事件
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

    // 根据给定的数值，修改滚动速度 sp是一次滚动多少像素,st是多少秒触发一次滚动
    void setspeed(int sp=10,int st=300);

private:
    int timerId; //定时器id
    int text_wpixel; //储存的当前label内字符串的像素水平长度

    int speedt;// 多久触发一次滚动
    int spixel;// 一次滚动多少像素

    int left;// 标明当前的像素滚动量

    QString blank;//空格
    int blank_wp;//空格的像素宽度

    int start_scroll;

    uint8_t flag; //判断是否应该开启滚动 0否 1真
};
