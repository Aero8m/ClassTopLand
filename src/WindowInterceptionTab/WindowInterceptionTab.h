#ifndef WINDOWINTERCEPTIONTAB_H
#define WINDOWINTERCEPTIONTAB_H

#include <QWidget>
#include <memory>

class WindowInterceptionTab : public QWidget
{
public:
    struct Impl;
    explicit WindowInterceptionTab(QWidget *parent = nullptr);
    ~WindowInterceptionTab() override;

#if defined(Q_OS_WIN)
    void handleWindowShown(quintptr handle);
#endif

private:
    std::unique_ptr<Impl> impl;
};

#endif
