#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include<QMouseEvent>
#include<QPropertyAnimation>
class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    ClickLabel(QWidget* parent=nullptr);
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
