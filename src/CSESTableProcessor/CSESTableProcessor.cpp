#include "CSESTableProcessor.h"

#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QTime>

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {
const QStringList dayKeys = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const QStringList dayNames = {"星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"};

[[noreturn]] void fail(const QString &message)
{
    throw std::invalid_argument(message.toUtf8().toStdString());
}

QString requiredScalar(const YAML::Node &parent, const char *key, const QString &place)
{
    const YAML::Node value = parent[key];
    if (!value || !value.IsScalar())
    {
        fail(place + QStringLiteral(" 缺少字符串字段 ") + QString::fromLatin1(key));
    }
    return QString::fromUtf8(value.as<std::string>());
}

QTime parseTime(const QString &value, const QString &place, bool withSeconds)
{
    static const QRegularExpression yamlTime(QStringLiteral(R"(^([01]\d|2[0-3]):[0-5]\d:[0-5]\d$)"));
    static const QRegularExpression jsonTime(QStringLiteral(R"(^([01]\d|2[0-3]):[0-5]\d$)"));
    if (!(withSeconds ? yamlTime : jsonTime).match(value).hasMatch())
    {
        fail(place + QStringLiteral(" 时间格式无效：") + value);
    }
    const QTime time = QTime::fromString(value, withSeconds ? QStringLiteral("HH:mm:ss")
                                                         : QStringLiteral("HH:mm"));
    if (!time.isValid() || (withSeconds && time.second() != 0))
    {
        fail(place + QStringLiteral(" 时间必须精确到整分钟：") + value);
    }
    return time;
}

QJsonArray sortedCourses(std::vector<QJsonObject> courses)
{
    std::stable_sort(courses.begin(), courses.end(), [](const QJsonObject &left, const QJsonObject &right) {
        return left.value("start").toString() < right.value("start").toString();
    });
    QJsonArray result;
    for (const QJsonObject &course : courses)
    {
        result.append(course);
    }
    return result;
}
}

QJsonObject CSESTableProcessor::CSESToCTPJsonTable(const YAML::Node &csesTable)
{
    if (!csesTable || !csesTable.IsMap())
    {
        fail(QStringLiteral("CSES 根节点必须是对象"));
    }
    const YAML::Node version = csesTable["version"];
    if (!version || !version.IsScalar() || version.Scalar() != "1")
    {
        fail(QStringLiteral("CSES 只支持 version: 1"));
    }

    const YAML::Node subjects = csesTable["subjects"];
    const YAML::Node schedules = csesTable["schedules"];
    if (!subjects || !subjects.IsSequence() || !schedules || !schedules.IsSequence())
    {
        fail(QStringLiteral("CSES subjects 和 schedules 必须是数组"));
    }

    QSet<QString> subjectNames;
    for (std::size_t i = 0; i < subjects.size(); ++i)
    {
        const YAML::Node subject = subjects[i];
        const QString place = QStringLiteral("subjects[%1]").arg(static_cast<qulonglong>(i));
        if (!subject.IsMap())
        {
            fail(place + QStringLiteral(" 必须是对象"));
        }
        const QString name = requiredScalar(subject, "name", place);
        if (name.trimmed().isEmpty() || subjectNames.contains(name))
        {
            fail(place + QStringLiteral(" 的科目名为空或重复"));
        }
        subjectNames.insert(name);
    }

    QJsonObject result;
    for (const QString &day : dayKeys)
    {
        result.insert(day, QJsonArray{});
    }
    result.insert("appendixTables", QJsonObject{});

    QSet<int> seenDays;
    for (std::size_t i = 0; i < schedules.size(); ++i)
    {
        const YAML::Node schedule = schedules[i];
        const QString place = QStringLiteral("schedules[%1]").arg(static_cast<qulonglong>(i));
        if (!schedule.IsMap())
        {
            fail(place + QStringLiteral(" 必须是对象"));
        }
        if (requiredScalar(schedule, "name", place).trimmed().isEmpty())
        {
            fail(place + QStringLiteral(" 的名称为空"));
        }
        const QString dayText = requiredScalar(schedule, "enable_day", place);
        bool validDay = false;
        const int day = dayText.toInt(&validDay);
        if (!validDay || day < 1 || day > 7 || seenDays.contains(day))
        {
            fail(place + QStringLiteral(" 的 enable_day 无效或重复"));
        }
        seenDays.insert(day);
        if (requiredScalar(schedule, "weeks", place) != QStringLiteral("all"))
        {
            fail(place + QStringLiteral(" 使用单双周课表；本地格式无法表示"));
        }
        const YAML::Node classes = schedule["classes"];
        if (!classes || !classes.IsSequence())
        {
            fail(place + QStringLiteral(" 的 classes 必须是数组"));
        }

        std::vector<QJsonObject> courses;
        courses.reserve(classes.size());
        for (std::size_t j = 0; j < classes.size(); ++j)
        {
            const YAML::Node item = classes[j];
            const QString classPlace = place + QStringLiteral(".classes[%1]").arg(static_cast<qulonglong>(j));
            if (!item.IsMap())
            {
                fail(classPlace + QStringLiteral(" 必须是对象"));
            }
            const QString name = requiredScalar(item, "subject", classPlace);
            if (!subjectNames.contains(name))
            {
                fail(classPlace + QStringLiteral(" 引用了未定义的科目：") + name);
            }
            const QString start = requiredScalar(item, "start_time", classPlace);
            const QString end = requiredScalar(item, "end_time", classPlace);
            const QTime startTime = parseTime(start, classPlace, true);
            const QTime endTime = parseTime(end, classPlace, true);
            if (startTime >= endTime)
            {
                fail(classPlace + QStringLiteral(" 开始时间必须早于结束时间"));
            }
            courses.push_back({{"name", name}, {"start", start.left(5)}, {"end", end.left(5)}});
        }
        result.insert(dayKeys.at(day - 1), sortedCourses(std::move(courses)));
    }
    return result;
}

YAML::Node CSESTableProcessor::CTPJsonTableToCSES(const QJsonObject &ctpJsonTable)
{
    const QJsonValue appendixValue = ctpJsonTable.value("appendixTables");
    if (!appendixValue.isUndefined())
    {
        if (!appendixValue.isObject())
        {
            fail(QStringLiteral("appendixTables 必须是对象"));
        }
        const QJsonObject appendixTables = appendixValue.toObject();
        for (auto it = appendixTables.begin(); it != appendixTables.end(); ++it)
        {
            if (!it.value().isArray() || !it.value().toArray().isEmpty())
            {
                fail(QStringLiteral("非空附加课表无法导出为 CSES：") + it.key());
            }
        }
    }

    YAML::Node root(YAML::NodeType::Map);
    root["version"] = 1;
    YAML::Node subjects(YAML::NodeType::Sequence);
    YAML::Node schedules(YAML::NodeType::Sequence);
    QSet<QString> seenSubjects;
    for (int day = 0; day < dayKeys.size(); ++day)
    {
        const QString dayKey = dayKeys.at(day);
        const QJsonValue dayValue = ctpJsonTable.value(dayKey);
        if (!dayValue.isUndefined() && !dayValue.isArray())
        {
            fail(dayKey + QStringLiteral(" 必须是课程数组"));
        }
        std::vector<QJsonObject> courses;
        const QJsonArray dayCourses = dayValue.toArray();
        courses.reserve(static_cast<std::size_t>(dayCourses.size()));
        for (qsizetype i = 0; i < dayCourses.size(); ++i)
        {
            const QString place = dayKey + QStringLiteral("[%1]").arg(i);
            if (!dayCourses.at(i).isObject())
            {
                fail(place + QStringLiteral(" 必须是课程对象"));
            }
            const QJsonObject course = dayCourses.at(i).toObject();
            if (!course.value("name").isString() || !course.value("start").isString()
                || !course.value("end").isString())
            {
                fail(place + QStringLiteral(" 缺少课程名称或时间"));
            }
            const QString name = course.value("name").toString();
            const QString start = course.value("start").toString();
            const QString end = course.value("end").toString();
            if (name.trimmed().isEmpty()
                || parseTime(start, place, false) >= parseTime(end, place, false))
            {
                fail(place + QStringLiteral(" 的课程名称或时间无效"));
            }
            courses.push_back(course);
        }
        const QJsonArray sorted = sortedCourses(std::move(courses));
        YAML::Node classes(YAML::NodeType::Sequence);
        for (const QJsonValue &value : sorted)
        {
            const QJsonObject course = value.toObject();
            const QString name = course.value("name").toString();
            if (!seenSubjects.contains(name))
            {
                YAML::Node subject(YAML::NodeType::Map);
                subject["name"] = name.toUtf8().toStdString();
                subjects.push_back(subject);
                seenSubjects.insert(name);
            }
            YAML::Node item(YAML::NodeType::Map);
            item["subject"] = name.toUtf8().toStdString();
            item["start_time"] = course.value("start").toString().toStdString() + ":00";
            item["end_time"] = course.value("end").toString().toStdString() + ":00";
            classes.push_back(item);
        }
        YAML::Node schedule(YAML::NodeType::Map);
        schedule["name"] = dayNames.at(day).toUtf8().toStdString();
        schedule["enable_day"] = day + 1;
        schedule["weeks"] = "all";
        schedule["classes"] = classes;
        schedules.push_back(schedule);
    }
    root["subjects"] = subjects;
    root["schedules"] = schedules;
    return root;
}

QJsonObject CSESTableProcessor::parseCSESYaml(const QByteArray &yamlText)
{
    return CSESToCTPJsonTable(YAML::Load(yamlText.toStdString()));
}

QByteArray CSESTableProcessor::serializeCSESYaml(const QJsonObject &ctpJsonTable)
{
    YAML::Emitter emitter;
    emitter << CTPJsonTableToCSES(ctpJsonTable);
    if (!emitter.good())
    {
        fail(QStringLiteral("生成 CSES YAML 失败"));
    }
    return QByteArray(emitter.c_str(), static_cast<qsizetype>(emitter.size()));
}
