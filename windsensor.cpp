#include "windsensor.h"
#include "filemanager.h"

#include <QRandomGenerator>
#include <QtMath>

WindSensor::WindSensor() {}

QString WindSensor::sensorType() const
{
    return QStringLiteral("风速风向传感器");
}

QStringList WindSensor::fieldNames() const
{
    return {QStringLiteral("风速"), QStringLiteral("风向")};
}

QStringList WindSensor::fieldUnits() const
{
    return {QStringLiteral("m/s"), QStringLiteral("°")};
}

QVector<DataPoint> WindSensor::loadFile(const QString &filePath)
{
    return FileManager::readCsv(filePath, fieldNames());
}

QVector<DataPoint> WindSensor::generateMockData(int count)
{
    QVector<DataPoint> result;
    QRandomGenerator *rng = QRandomGenerator::global();
    QDateTime base = QDateTime::currentDateTime();

    for (int i = 0; i < count; i++) {
        DataPoint dp;
        dp.timeStamp = base.addSecs(i * frequency);

        double speed = rng->bounded(300) / 10.0;

        double direction = rng->bounded(3600) / 10.0;
        dp.value.append(speed);
        dp.value.append(direction);
        result.append(dp);
    }
    return result;
}
