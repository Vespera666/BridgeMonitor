#ifndef MONITORPOINT_H
#define MONITORPOINT_H

#include <QDate>
#include <QString>
#include "sensor.h"

class MonitoringPoint
{
public:
    QString pointId;                               // 监测点编号，如 "MP-W-01"
    QString sectionName;                           // 断面名称，如 "北塔塔顶"
    QDate installDate;                             // 安装日期
    QVector<Sensor *> sensors;                     // 该监测点安装的所有传感器
    QString displayName() const;                   // 显示名
    bool hasSensorType(const QString &type) const; // 查询
    void addSensor(Sensor *s);
    void removeSensor(int index);
    int sensorCount() const;
    w
    // 增删传感器
};

#endif // MONITORPOINT_H
