#ifndef MONITORPOINT_H
#define MONITORPOINT_H

#include <QDate>
#include <QString>
#include "sensor.h"

class MonitoringPoint
{
public:
    QString pointId;          // 监测点编号，如 "MP-W-01"
    QString sectionName;      // 断面名称，如 "北塔塔顶"
    QDate installDate;        // 安装日期
    QString dataType;         // 传感器数据类型，如 "挠度传感器"、"索力监测传感器"
    Sensor *sensor = nullptr; // 该监测点绑定的传感器（1:1）

    QString displayName() const; // 显示名
    QString sensorType() const;  // 查询绑定传感器的类型
    void bindSensor(Sensor *s);  // 绑定传感器
    void unbindSensor();         // 解绑传感器
    bool hasSensor() const;      // 是否已绑定传感器
};

#endif // MONITORPOINT_H
