#include "./ClickLabel.h"

ClickLabel::ClickLabel(QWidget* parent) {

}
void ClickLabel::mouseClickEvent(QMouseEvent* e){
    setWindowFlags(Qt::Widget);
    emit clicked();
}
void ClickLabel::mouseDoubleClickEvent(QMouseEvent* e){
    setWindowFlags(Qt::Widget);
    emit DoubleClicked();
}
bool ClickLabel::event(QEvent *e)
{
    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
        if(mouseEvent->button() == Qt::LeftButton){
            emit clicked();
            return true;
        }
    }
    if (e->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(e);
        if(mouseEvent->button() == Qt::LeftButton)
        {
            emit DoubleClicked();
            return true;
        }
    }
    if (e->type() == QEvent::Enter and animationEnabled){
        resizeAnimation->setDirection(QAbstractAnimation::Forward);
        resizeAnimation->start();
        return true;
    }
    if (e->type() == QEvent::Leave and animationEnabled){
        resizeAnimation->setDirection(QAbstractAnimation::Backward);
        resizeAnimation->start();
        return true;
    }
    if (e->type() == QEvent::Resize and !animationConfigured){
        resizeAnimation = new QPropertyAnimation(this,"geometry");
        resizeAnimation->setStartValue(QRect(pos().x(),pos().y(),width(),height()));
        resizeAnimation->setEndValue(QRect(pos().x()-((width()*1.2-width())/2),pos().y()-((height()*1.2-height())/2),width()*1.2,height()*1.2));
        resizeAnimation->setDuration(200);
        resizeAnimation->setEasingCurve(QEasingCurve::OutInExpo);
        animationConfigured = true;
        return true;
    }
    return QLabel::event(e);
}
