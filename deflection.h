#ifndef DEFLECTION_H
#define DEFLECTION_H

#include "sensor.h"
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVector>
#include <QMap>

// ============================================================
// DeflectionSensor - 挠度传感器
//
// 测量字段：挠度 (mm)
// 测点：DEFL001 ~ DEFL007（每个测点 2 个传感器）
// ============================================================
class DeflectionSensor : public Sensor
{
public:
    DeflectionSensor() = default;

    // --- 实现 Sensor 纯虚接口 ---
    QString            sensorType() const override;
    QStringList        fieldNames() const override;
    QStringList        fieldUnits() const override;
    QVector<DataPoint> loadFile(const QString &filePath) override;
    QVector<DataPoint> generateMockData(int count) override;

};

#endif // DEFLECTION_H
