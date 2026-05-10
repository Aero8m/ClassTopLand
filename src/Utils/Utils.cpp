//
// Created by jiahang on 25-5-24.
//

#include "Utils.h"
QString getStyleSheet(QString filen) {
    QFile file(filen);
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    return stylesheet;

}
void loadStyleSheet(QWidget* widget,QString filen) {
    QFile file(filen);
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    widget->setStyleSheet(stylesheet);
    file.close();
}

