#include "deflection.h"
#include "filemanager.h"

#include <QRandomGenerator>
#include <QtMath>

DeflectionSensor::DeflectionSensor()
{
    name = QStringLiteral("挠度计");
    size = QStringLiteral("标准");
    model = QStringLiteral("DF-100");
    manufacturer = QStringLiteral("通用");
    generDate = QDate(2024, 1, 1);
    frequency = 1;
}

QString DeflectionSensor::sensorType() const
{
    return QStringLiteral("挠度传感器");
}

QStringList DeflectionSensor::fieldNames() const
{
    return {QStringLiteral("挠度")};
}

QStringList DeflectionSensor::fieldUnits() const
{
    return {QStringLiteral("mm")};
}

QVector<DataPoint> DeflectionSensor::loadFile(const QString &filePath)
{
    return FileManager::readCsv(filePath, fieldNames());
}

QVector<DataPoint> DeflectionSensor::generateMockData(int count)
{
    QVector<DataPoint> result;
    QRandomGenerator *rng = QRandomGenerator::global();
    QDateTime base = QDateTime::currentDateTime();

    for (int i = 0; i < count; i++) {
        DataPoint dp;
        dp.timeStamp = base.addSecs(i * frequency);
        // 挠度范围 -50 ~ +50 mm，正弦波 + 噪声
        double value = 35.0 * qSin(i * 0.12) + 15.0 * qSin(i * 0.04)
                       + (rng->generateDouble() - 0.5) * 8.0;
        dp.value.append(value);
        result.append(dp);
    }
    return result;
}
