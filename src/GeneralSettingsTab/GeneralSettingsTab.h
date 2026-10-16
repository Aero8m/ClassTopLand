#ifndef GENERALSETTINGSTAB_H
#define GENERALSETTINGSTAB_H

#include <QJsonObject>
#include <QWidget>

class QCheckBox;
class QLabel;
class QPushButton;

namespace Ui {
class GeneralSettingsTab;
}

class GeneralSettingsTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralSettingsTab(QWidget *parent = nullptr);
    ~GeneralSettingsTab() override;
    void reloadSettings();
    void discardChanges();

signals:
    void configSaved(QJsonObject config);
    void alwaysOnTopChanged(bool enabled);

private:
    Ui::GeneralSettingsTab *ui;
    void applySettings();
    void updateDirtyState();
    bool hasChanges() const;

    QCheckBox *autoStartCheckBox;
    QCheckBox *alwaysOnTopCheckBox;
    QLabel *autoStartError;
    QLabel *alwaysOnTopError;
    QLabel *statusLabel;
    QPushButton *restoreButton;
    QPushButton *applyButton;
    bool savedAutoStart = false;
    bool savedAlwaysOnTop = true;
};

#endif // GENERALSETTINGSTAB_H
