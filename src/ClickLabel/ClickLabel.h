#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include<QMouseEvent>
#include <QToolTip>
#include<QPropertyAnimation>
class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    ClickLabel(QWidget* parent=nullptr);
    virtual void mouseClickEvent(QMouseEvent* e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual bool event(QEvent *e);
    QPropertyAnimation* resizeAnimation;
    bool animationConfigured = false;
    bool animationEnabled = false;
    void setAnimationEnabled(bool status){
        animationEnabled = status;
    }
signals:
    void clicked();
    void DoubleClicked();
};

#endif // CLICKLABEL_H
