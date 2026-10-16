#ifndef CONFIGSETTINGS_H
#define CONFIGSETTINGS_H

#include <QJsonObject>
#include <QString>

namespace ConfigSettings {
inline constexpr auto alwaysOnTopKey = "course_bar_always_on_top";

// A missing file is a new configuration; an existing invalid file is never replaced.
bool read(QJsonObject *config, QString *error);
bool mergeAndSave(const QJsonObject &changes, QJsonObject *savedConfig, QString *error);
}

#endif // CONFIGSETTINGS_H
