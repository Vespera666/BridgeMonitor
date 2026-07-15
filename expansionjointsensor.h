#ifndef EXPANSIONJOINTSENSOR_H
#define EXPANSIONJOINTSENSOR_H

#include "sensor.h"

class ExpansionJointSensor : public Sensor
{
public:
    ExpansionJointSensor();

    QString sensorType() const override;
    QStringList fieldNames() const override;
    QStringList fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;
};

#endif // EXPANSIONJOINTSENSOR_H