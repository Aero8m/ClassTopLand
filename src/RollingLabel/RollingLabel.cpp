#include "./RollingLabel.h"

RollingLabel::RollingLabel(QWidget *parent):QLabel(parent)
{
    timerId = -1;// 定时器的ID
    text_wpixel = 0; //文本的像素长度

    speedt = 10;// 多久触发一次滚动
    spixel = 1;// 一次滚动多少像素

    //start_scroll = this->width();//保存了窗体最初的宽度，避免窗体变动就不滚动

    flag = 0;//默认不处理
}

RollingLabel::~RollingLabel()
{
    if(timerId >= 0)
        killTimer(timerId);
}

//在设置文本、缩放事件两次调用自适应的函数
void RollingLabel::setText(const QString &txt)
{
    if(Qt::mightBeRichText(txt))//判断是否为富文本
        flag = 0;  //0不处理，直接用原本的绘画事件显示，当属于富文本时使用这个 1左到右 2上到下
    else
        flag = 1;

    QLabel::setText(txt);
    upateLabelRollingState();
}

//设置图片，主要把lt设回0，使其恢复正常的图片显示
void RollingLabel::setPixmap(const QPixmap &pix)
{
    flag=0;
    QLabel::setPixmap(pix);
}

//窗口变化事件
void RollingLabel::resizeEvent(QResizeEvent *e)
{
    QLabel::resizeEvent(e);
    upateLabelRollingState();
}

// 根据给定的数值，修改滚动速度  sp是一次滚动多少像素,st是多少秒触发一次滚动
void RollingLabel::setspeed(int sp,int st)
{
    spixel = sp;
    speedt = st;

    upateLabelRollingState(); //　刷新一次滚动量
}

//用来判断label文本是否需要滚动起来，是这块功能的核心
void RollingLabel::upateLabelRollingState()
{
    //获取文本大小，小于文本框长度，则无需滚动
    QFont ft = font();// 获取当前字体的格式，里面有文本大小和文本像素大小
    QFontMetrics fm(ft); // 以当前的字体格式为基础

#if QT_VERSION > QT_VERSION_CHECK(5,11,0)//根据官方文档说明，5.11后使用新的函数
    text_wpixel = fm.horizontalAdvance(text() ); //以当前的字体格式为基础,计算字体的像素宽度
#else
    text_wpixel = fm.width(text() ); //以当前的字体格式为基础,计算字体的像素宽度
#endif

    if( (text_wpixel > 701 ) && flag == 1 )// **长度或高度超出本身label的像素大小，则开启滚动***关键判断
    {
        left = 0; // 标志当前的像素滚动量

#if QT_VERSION > QT_VERSION_CHECK(5,11,0)//根据官方文档说明，5.11后使用新的函数
        blank = " ";//空格
        blank_wp = fm.horizontalAdvance(blank );//空格的像素宽度,方便后面计算是否到达末尾
#else
        blank = " ";//空格
        blank_wp = fm.width(blank );//空格的像素宽度
#endif


        //开启定时器，定时器定时触发滚动效果
        timerId = startTimer(speedt);

    }
    else//关闭文本框滚动
    {
        flag = 0; //关闭
        if(timerId >= 0){
            killTimer(timerId);
            timerId = -1;
        }
    }

}

//定时改位移量，到末尾时改为开头  负责修改当前像素位移值
void RollingLabel::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == timerId && isVisible())
    {
        left += spixel;// (0,0)在左上角，每次增加对应像素

        //  判断是否已经完成一遍循环，完成则恢复起始位置，重新开始循环
        if (left > (text_wpixel + blank_wp))
            left = 0;

        //repaint();//立即触发一次刷新，不会产生冗余，但是耗性能
        update();//不会立马刷新，有可能产生事件冗余，但是节省性能
    }

    QLabel::timerEvent(e);
}

//重绘事件，根据位移量left显示文本
void RollingLabel::paintEvent(QPaintEvent *e)
{
    if(flag == 0){ //　不处理，直接调用标签的默认函数
        QLabel::paintEvent(e);
        return;
    }

    QPainter pen(this);
    //  获取当前label的矩形大小
    QRect rc = rect();
    rc.setHeight(rc.height() /*- 2*/);
    rc.setWidth(rc.width() /*- 2*/);

    QString strText = text();
    strText += blank + text();

    rc.setLeft(rc.left() - left); //修改矩形 x轴, 由于left在不断变大，setLeft就在不断变小,(0,0)在左上角,固左移
    pen.drawText(rc,Qt::AlignVCenter, strText);//根据给定的矩形坐标，绘制标签
}
