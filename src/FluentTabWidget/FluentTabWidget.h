#ifndef FLUENTTABWIDGET_H
#define FLUENTTABWIDGET_H

#include <QTabWidget>
#include <QTabBar>
#include <QPainter>
#include <QStyleOptionButton>
#include<QObject>
#include<QEnterEvent>
#include<QMouseEvent>
class FluentTabBar : public QTabBar
{
    Q_OBJECT

public:
    explicit FluentTabBar(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    QSize tabSizeHint(int index) const override;
private:
    int m_hoverIndex = -1;
};
class FluentTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit FluentTabWidget(QWidget *parent = nullptr);
};

#endif // FLUENTTABWIDGET_H