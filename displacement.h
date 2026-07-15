#ifndef DISPLACEMENT_H
#define DISPLACEMENT_H

#include "sensor.h"

class DisplacementSensor : public Sensor
{
public:
    DisplacementSensor();

    QString sensorType() const override;
    QStringList fieldNames() const override;
    QStringList fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;
};

#endif // DISPLACEMENT_H
