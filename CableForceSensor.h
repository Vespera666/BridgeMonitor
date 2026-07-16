#ifndef CABLEFORCESENSOR_H
#define CABLEFORCESENSOR_H

#include "sensor.h"

class CableForceSensor : public Sensor
{
public:
    CableForceSensor();

    QString sensorType() const override;
    QStringList fieldNames() const override;
    QStringList fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;
};

#endif // CABLEFORCESENSOR_H