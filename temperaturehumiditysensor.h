#ifndef TEMPERATUREHUMIDITYSENSOR_H
#define TEMPERATUREHUMIDITYSENSOR_H

#include "sensor.h"

class TemperatureHumiditySensor : public Sensor
{
public:
    TemperatureHumiditySensor();

    QString sensorType() const override;
    QStringList fieldNames() const override;
    QStringList fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;
};

#endif // TEMPERATUREHUMIDITYSENSOR_H