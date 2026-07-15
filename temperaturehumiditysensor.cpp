#include "temperaturehumiditysensor.h"
#include "filemanager.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QtMath>

TemperatureHumiditySensor::TemperatureHumiditySensor()
{

}
QString TemperatureHumiditySensor::sensorType() const
{
    return "温湿度监测传感器";
}

QStringList TemperatureHumiditySensor::fieldNames() const
{
    return {"温度", "湿度"};
}

QStringList TemperatureHumiditySensor::fieldUnits() const
{
    return {"°C", "%"};
}

QVector<DataPoint> TemperatureHumiditySensor::loadFile(const QString &filePath)
{
    return FileManager::readCsv(filePath, this->fieldNames());
}

QVector<DataPoint> TemperatureHumiditySensor::generateMockData(int count)
{
    QVector<DataPoint> dataList;
    QDateTime baseTime = QDateTime::currentDateTime();
    QRandomGenerator* rng = QRandomGenerator::global();

    for (int i = 0; i < count; ++i)
    {
        DataPoint dp;
        dp.timeStamp = baseTime.addSecs(i * this->frequency);

        double temp = 20.0 + 15.0 * qSin(i * 0.01) + (rng->generateDouble() - 0.5) * 2.0;
        double humidity = 60.0 + 20.0 * qSin(i * 0.008 + 1.5) + (rng->generateDouble() - 0.5) * 5.0;

        dp.value.append(temp);
        dp.value.append(humidity);
        dataList.append(dp);
    }
    return dataList;
}