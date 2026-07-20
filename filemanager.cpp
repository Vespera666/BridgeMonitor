#include "filemanager.h"

#include <QFile>
#include <QTextStream>

FileManager::FileManager() {}


QVector<DataPoint> FileManager::readCsv(const QString &filePath, const QStringList &fieldNames)
{
    QVector<DataPoint> result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    QTextStream in(&file);
    int lineNo = 0;
    const int expectedCols = 1 + fieldNames.size();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNo++;

        if (line.isEmpty() || line.startsWith('#'))
            continue;
        if (lineNo == 1)
            continue;

        QStringList parts = parseLine(line);
        if (parts.size() < expectedCols) {
            qWarning("行 %d 列数不足，跳过: %s", lineNo, qPrintable(line));
            continue;
        }

        DataPoint dp;
        dp.timeStamp = QDateTime::fromString(parts[0], "yyyy-MM-dd HH:mm:ss");
        if (!dp.timeStamp.isValid()) {

            dp.timeStamp = QDateTime::fromString(parts[0], "yyyy/MM/dd HH:mm:ss");
        }
        for (int i = 1; i <= fieldNames.size(); i++)
            dp.value.append(parts[i].toDouble());

        result.append(dp);
    }

    file.close();
    return result;
}


bool FileManager::writeCsv(const QString &filePath,
                           const QStringList &fieldNames,
                           const QStringList &fieldUnits,
                           const QVector<DataPoint> &data)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);

    QStringList headers;
    headers << QStringLiteral("时间");
    for (int i = 0; i < fieldNames.size(); i++) {
        headers << fieldNames[i] + "(" + fieldUnits[i] + ")";
    }
    out << headers.join(',') << "\n";


    for (const DataPoint &dp : data) {
        out << dp.timeStamp.toString("yyyy-MM-dd HH:mm:ss");
        for (int i = 0; i < dp.value.size(); i++)
            out << ',' << dp.value[i];
        out << '\n';
    }

    file.close();
    return true;
}

QStringList FileManager::parseLine(const QString &line)
{
    QStringList fields;
    QString field;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); i++) {
        QChar c = line[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.append(field.trimmed());
            field.clear();
        } else {
            field.append(c);
        }
    }
    fields.append(field.trimmed());
    return fields;
}
