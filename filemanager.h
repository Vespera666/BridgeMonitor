#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include "DataPoint.h"

class FileManager
{
public:
    FileManager();

    // 读写 CSV
    static QVector<DataPoint> readCsv(const QString &filePath,
                                      const QStringList &fieldNames);

    static bool writeCsv(const QString &filePath,
                         const QStringList &fieldNames,
                         const QStringList &fieldUnits,
                         const QVector<DataPoint> &data);

private:
    static QStringList parseLine(const QString &line);
};

#endif // FILEMANAGER_H
