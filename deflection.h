#ifndef DEFLECTION_H
#define DEFLECTION_H

#include "sensor.h"

class DeflectionSensor : public Sensor
{
public:
    DeflectionSensor();

    QString sensorType() const override;
    QStringList fieldNames() const override;
    QStringList fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;
};

#endif // DEFLECTION_H
