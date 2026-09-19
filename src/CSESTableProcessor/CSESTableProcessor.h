#ifndef CLASSTOPLAND_CSESTABLEPROCESSOR_H
#define CLASSTOPLAND_CSESTABLEPROCESSOR_H

#include <QByteArray>
#include <QJsonObject>
#include <yaml-cpp/yaml.h>

class CSESTableProcessor
{
public:
    // Only weeks: all and whole-minute times fit the local weekly table.
    // Subject metadata is discarded; exporting nonempty appendix tables fails.
    // Conversion errors throw std::invalid_argument; malformed YAML may also
    // throw YAML::Exception. Neither conversion reads or writes user files.
    static QJsonObject CSESToCTPJsonTable(const YAML::Node &csesTable);
    static YAML::Node CTPJsonTableToCSES(const QJsonObject &ctpJsonTable);

    static QJsonObject parseCSESYaml(const QByteArray &yamlText);
    static QByteArray serializeCSESYaml(const QJsonObject &ctpJsonTable);
};

#endif // CLASSTOPLAND_CSESTABLEPROCESSOR_H
