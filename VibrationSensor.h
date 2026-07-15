#ifndef VIBRATIONSENSOR_H
#define VIBRATIONSENSOR_H

#include "sensor.h"

class VibrationSensor : public Sensor
{
public:
    VibrationSensor();

    // 重写基类纯虚函数
    QString sensorType() const override;
    QStringList fieldNames() const override;
    QStringList fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;
};

#endif // VIBRATIONSENSOR_H