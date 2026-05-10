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
    m_hoverIndex = -1;
    QTabBar::enterEvent(event);
}

void FluentTabBar::leaveEvent(QEvent *event)
{
    m_hoverIndex = -1;
    update();
    QTabBar::leaveEvent(event);
}

void FluentTabBar::mouseMoveEvent(QMouseEvent *event)
{
    int index = tabAt(event->pos());
    if (index != m_hoverIndex) {
        m_hoverIndex = index;
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
        bool hover = (m_hoverIndex == i);

        // 绘制背景
        if (selected) {
            //painter.save();
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(17, 145, 211)); // 蓝色边框颜色
            painter.drawRoundedRect(rect.left()+2, rect.top()+7, 4, rect.height()-11,3,3); // 左侧边框宽度为 4px
            //painter.restore();
            // painter.setBrush(QColor(17, 145, 211));
            // painter.setPen(Qt::NoPen);
            // painter.drawRoundedRect(rect.adjusted(2, 2, -2, -2), 1, 1);
        } else if (hover) {
            painter.setBrush(QColor(220, 220, 220));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(rect.adjusted(2, 2, -2, -2), 3, 3);
        }

        QRect textRect = rect;
        textRect.setLeft(rect.left() + 15); // 左边留出一点边距

        // 绘制左对齐文本
        painter.setPen(Qt::black);
        painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    }
}

QSize FluentTabBar::tabSizeHint(int index) const
{
    Q_UNUSED(index);
    return QSize(130, 35); // 固定每个 Tab 的大小
}

FluentTabWidget::FluentTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    // 设置自定义 TabBar
    setTabBar(new FluentTabBar(this));

    // 设置 TabBar 在左侧
    setTabPosition(QTabWidget::West);

    // 设置样式（Fluent 风格）
    setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #dcdcdc;
            background: white;
        }
    )");
}