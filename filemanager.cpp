#include "filemanager.h"

#include <QFile>
#include <QTextStream>

FileManager::FileManager() {}

/* 读 CSV 文件，返回 DataPoint 列表。
   fieldNames 用于校验列数（不含时间列），列数不匹配的行跳过并打印警告。 */
QVector<DataPoint> FileManager::readCsv(const QString &filePath,
                                        const QStringList &fieldNames)
{
    QVector<DataPoint> result;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    QTextStream in(&file);
    int lineNo = 0;
    const int expectedCols = 1 + fieldNames.size(); // 时间 + 各字段

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNo++;

        if (line.isEmpty() || line.startsWith('#'))
            continue;
        if (lineNo == 1)
            continue; // 跳过表头行

        QStringList parts = parseLine(line);
        if (parts.size() < expectedCols) {
            qWarning("行 %d 列数不足，跳过: %s", lineNo, qPrintable(line));
            continue;
        }

        DataPoint dp;
        dp.timeStamp = QDateTime::fromString(parts[0], "yyyy-MM-dd HH:mm:ss");
        if (!dp.timeStamp.isValid()) {
            // 尝试另一种常见格式
            dp.timeStamp = QDateTime::fromString(parts[0], "yyyy/MM/dd HH:mm:ss");
        }
        for (int i = 1; i <= fieldNames.size(); i++)
            dp.value.append(parts[i].toDouble());

        result.append(dp);
    }

    file.close();
    return result;
}

/* 将 DataPoint 列表写入 CSV 文件，第一行为表头。
   表头格式与 Sensor::headerLabels() 一致："时间", "风速(m/s)", ... */
bool FileManager::writeCsv(const QString &filePath,
                           const QStringList &fieldNames,
                           const QStringList &fieldUnits,
                           const QVector<DataPoint> &data)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);

    // 写表头
    QStringList headers;
    headers << QStringLiteral("时间");
    for (int i = 0; i < fieldNames.size(); i++) {
        headers << fieldNames[i] + "(" + fieldUnits[i] + ")";
    }
    out << headers.join(',') << "\n";

    // 写数据行
    for (const DataPoint &dp : data) {
        out << dp.timeStamp.toString("yyyy-MM-dd HH:mm:ss");
        for (int i = 0; i < dp.value.size(); i++)
            out << ',' << dp.value[i];
        out << '\n';
    }

    file.close();
    return true;
}

/* 解析 CSV 的一行：处理逗号分隔和引号包裹 */
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
