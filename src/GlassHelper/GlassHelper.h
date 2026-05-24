#ifndef GLASSHELPER_H
#define GLASSHELPER_H

#include <QWidget>
#include<QPalette>
#include<QEvent>
#include<QPainter>
#include<QPaintEvent>
#include<QVBoxLayout>
enum SystemType {
    Windows10p,
    Windows10d
};

class BackgroundEventFilter : public QObject {
    Q_OBJECT
    public:
        BackgroundEventFilter(QObject *parent = nullptr);
        bool eventFilter(QObject *obj, QEvent *event) override {
            if (event->type() == QEvent::Paint) {
                QWidget *w = qobject_cast<QWidget*>(obj);
                QPaintEvent *pe = static_cast<QPaintEvent*>(event);
                if (w && !pe->rect().isEmpty()) {
                    QPainter painter(w);
                    painter.setClipRect(pe->rect()); // 仅重绘脏区域，提升性能
                    // 0.3 * 255 ≈ 76
                    painter.fillRect(w->rect(), QColor(255, 255, 255, 76));
                }
            }
            return false; // 继续传递事件，保证子控件和原生绘制正常执行
        }
};

class GlassHelper {
public:
    static void enableBlurBehind(QWidget *widget,int alpha=0x66000000);
    #ifdef __linux__
    static void enableBlurBehindX11(QWidget *widget);
    #endif
    #ifdef _WIN32
    static void enableBlurBehindWin32(QWidget* widget,int alpha);
    #endif

};

#endif // GLASSHELPER_H