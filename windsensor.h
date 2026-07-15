#ifndef WINDSENSOR_H
#define WINDSENSOR_H

#include "sensor.h"

class WindSensor : public Sensor
{
public:
    WindSensor();

    QString sensorType() const override;
    QStringList fieldNames() const override;
    QStringList fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;
};

#endif // WINDSENSOR_H
