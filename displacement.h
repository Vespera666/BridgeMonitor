#ifndef DISPLACEMENT_H
#define DISPLACEMENT_H

#include "sensor.h"
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVector>
#include <QMap>

// ============================================================
// DisplacementSensor - 支座位移传感器
//
// 测量字段：位移量 (mm)
// 测点：DISP001 ~ DISP004（每个测点 2 个传感器）
// ============================================================
class DisplacementSensor : public Sensor
{
public:
    DisplacementSensor() = default;

    // --- 实现 Sensor 纯虚接口 ---
    QString            sensorType() const override;
    QStringList        fieldNames() const override;
    QStringList        fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;


};

#endif // DISPLACEMENT_H
