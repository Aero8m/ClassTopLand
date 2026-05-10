#ifndef GLASSHELPER_H
#define GLASSHELPER_H

#include <QWidget>
enum SystemType {
    Windows10p,
    Windows10d
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