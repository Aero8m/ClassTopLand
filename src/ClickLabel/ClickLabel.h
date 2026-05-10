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
    QPropertyAnimation* resize_animation;
    bool anisettinged = false;
    bool aniopen = false;
    void setAniOpen(bool status){
        aniopen = status;
    }
signals:
    void clicked();
    void DoubleClicked();
};

#endif // CLICKLABEL_H
