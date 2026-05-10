#include "GlassHelper.h"
#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QDebug>
#include <cmath>
#include <qguiapplication.h>
#include <QWindow>
#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#endif
#ifdef _WIN32
#include<windows.h>
enum AccentState {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_INVALID_STATE = 5
};

struct AccentPolicy {
    AccentState AccentState;
    int AccentFlags;
    int GradientColor;
    int AnimationId;
};
struct WindowCompositionAttributeData {
    int Attribute;
    PVOID Data;
    ULONG DataSize;
};
#endif
#ifdef __APPLE__
#endif

void GlassHelper::enableBlurBehind(QWidget *widget,int alpha) {
#ifdef __linux__
    enableBlurBehindX11(widget);
#elifdef _WIN32
    enableBlurBehindWin32(widget,alpha);
#elifdef __APPLE__
    qDebug() << QString("%1 {background: rgba(255,255,255,0.7);}").arg(widget->metaObject()->className());
    widget->setStyleSheet(QString("%1 {background-color: rgba(255,255,255,0.7);}").arg(widget->metaObject()->className()));
#endif

}

#ifdef __linux__
void GlassHelper::enableBlurBehindX11(QWidget *widget) {
    QString platform = QGuiApplication::platformName();
    if (platform == "xcb" || platform == "x11") {
        if (!widget->winId()) {
            qWarning() << "Widget has no winId!";
            return;
        }

        Display *display = XOpenDisplay(nullptr);
        if (!display) {
            qWarning() << "Failed to open X display.";
            return;
        }

        Window window = static_cast<Window>(widget->winId());

        Atom blurAtom = XInternAtom(display, "_KDE_NET_WM_BLUR_BEHIND_REGION", false);
        if (blurAtom == None) {
            qWarning() << "_KDE_NET_WM_BLUR_BEHIND_REGION atom not found.";
            XCloseDisplay(display);
            return;
        }

        unsigned long blurValue = 1;
        XChangeProperty(display,
                        window,
                        blurAtom,
                        XA_CARDINAL,
                        32,
                        PropModeReplace,
                        reinterpret_cast<unsigned char*>(&blurValue),
                        1);

        XFlush(display);
        XCloseDisplay(display);
    }else {
        qWarning() << "Unsupported platform for blur behind.";
    }
}
#endif
SystemType getWindowsVersion() {
    double major = QSysInfo::productVersion().toDouble();
    if (major >= 10.0) {
        return Windows10p;
    }
    else {
        return Windows10d;
    }

}
#ifdef _WIN32
typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WindowCompositionAttributeData*);
pSetWindowCompositionAttribute SetWindowCompositionAttribute = nullptr;
void GlassHelper::enableBlurBehindWin32(QWidget* widget,int alpha) {
    if (getWindowsVersion() == Windows10p) {
        if (!SetWindowCompositionAttribute) {
            HMODULE hModule = LoadLibrary(TEXT("user32.dll"));
            if (hModule) {
                SetWindowCompositionAttribute =
                    (pSetWindowCompositionAttribute)GetProcAddress(hModule, "SetWindowCompositionAttribute");
            }
        }
        HWND hWnd = HWND(widget->winId());
        AccentPolicy policy = { ACCENT_ENABLE_ACRYLICBLURBEHIND, 0, int(0x00FFFFFF + alpha), 0 };
        WindowCompositionAttributeData data = { 19, &policy, sizeof(AccentPolicy) };
        SetWindowCompositionAttribute(hWnd, &data);
    }
    else {
        widget->setStyleSheet("background: rgba(255,255,0.7)");
    }

}
#endif

