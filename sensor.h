#ifndef SENSOR_H
#define SENSOR_H

#include <QDateTime>
#include <QString>
#include "DataPoint.h"

class Sensor
{
public:
    QString name;                               // 设备名，如 "风速风向仪"
    QString size;                               // 规格
    QString model;                              // 型号
    QString manufacturer;                       // 厂家
    QDate generDate;                            // 生产日期
    int frequency;                              //采集频率
    virtual QString sensorType() const = 0;     // 返回传感器类型名
    virtual QStringList fieldNames() const = 0; // 返回字段名列表
    virtual QStringList fieldUnits() const = 0; // 返回字段单位列表
    virtual QVector<DataPoint> loadFile(const QString &filePath) = 0;
    // 从 CSV 文件加载数据
    virtual QVector<DataPoint> generateMockData(int count) = 0;
    // 生成模拟数据
    QStringList headerLabels() const; // 拼接完整表头
    int columnCount() const;          // 表格列数：1(时间) + 字段数
    virtual ~Sensor() {}
};

#endif // SENSOR_H
