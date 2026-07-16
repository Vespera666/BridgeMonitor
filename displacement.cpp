#include "displacement.h"
#include "filemanager.h"

#include <QRandomGenerator>
#include <QtMath>

DisplacementSensor::DisplacementSensor()
{
    name = QStringLiteral("位移计");
    size = QStringLiteral("标准");
    model = QStringLiteral("DISP-100");
    manufacturer = QStringLiteral("通用");
    generDate = QDate(2024, 1, 1);
    frequency = 1;
}

QString DisplacementSensor::sensorType() const
{
    return QStringLiteral("支座位移传感器");
}

QStringList DisplacementSensor::fieldNames() const
{
    return {QStringLiteral("位移量")};
}

QStringList DisplacementSensor::fieldUnits() const
{
    return {QStringLiteral("mm")};
}

QVector<DataPoint> DisplacementSensor::loadFile(const QString &filePath)
{
    return FileManager::readCsv(filePath, fieldNames());
}

QVector<DataPoint> DisplacementSensor::generateMockData(int count)
{
    QVector<DataPoint> result;
    QRandomGenerator *rng = QRandomGenerator::global();
    QDateTime base = QDateTime::currentDateTime();

    for (int i = 0; i < count; i++) {
        DataPoint dp;
        dp.timeStamp = base.addSecs(i * frequency);
        // 位移范围 -5 ~ +5 mm，正弦波 + 噪声
        double value = 3.0 * qSin(i * 0.15) + 1.5 * qSin(i * 0.05)
                       + (rng->generateDouble() - 0.5) * 1.0;
        dp.value.append(value);
        result.append(dp);
    }
    return result;
}
