#include "FluentTabWidget.h"
#include <QStyleOptionButton>
#include <QPainter>

FluentTabBar::FluentTabBar(QWidget *parent)
    : QTabBar(parent)
{
    setStyleSheet(R"(
        QTabBar::tab{
        background:#f3f3f3;
        width:100%;
        border-radius:0px;
        padding:5px;
        }
        QTabBar::tab:hover{
        background:rgb(217, 217, 217);
        }
        QTabBar::tab:selected{
        border-left:3px solid #1191d3;
        }
    )");
}

void FluentTabBar::enterEvent(QEnterEvent *event)
{
    hoverIndex = -1;
    QTabBar::enterEvent(event);
}

void FluentTabBar::leaveEvent(QEvent *event)
{
    hoverIndex = -1;
    update();
    QTabBar::leaveEvent(event);
}

void FluentTabBar::mouseMoveEvent(QMouseEvent *event)
{
    int index = tabAt(event->pos());
    if (index != hoverIndex) {
        hoverIndex = index;
        update();
    }
    QTabBar::mouseMoveEvent(event);
}

void FluentTabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 0; i < count(); ++i)
    {
        QRect rect = tabRect(i);
        QString text = tabText(i);
        bool selected = (i == currentIndex());
        bool hover = (hoverIndex == i);

        if (selected) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(17, 145, 211));
            painter.drawRoundedRect(rect.left()+2, rect.top()+7, 4, rect.height()-11,3,3);
        } else if (hover) {
            painter.setBrush(QColor(220, 220, 220));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(rect.adjusted(2, 2, -2, -2), 3, 3);
        }

        QRect textRect = rect;
        textRect.setLeft(rect.left() + 15);
        painter.setPen(Qt::black);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
}

QSize FluentTabBar::tabSizeHint(int index) const
{
    Q_UNUSED(index);
    return QSize(130, 35);
}

FluentTabWidget::FluentTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setTabBar(new FluentTabBar(this));
    setTabPosition(QTabWidget::West);
    setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #dcdcdc;
            background: white;
        }
    )");
}