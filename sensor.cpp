#include "sensor.h"

QStringList Sensor::headerLabels() const { // 拼接完整表头
    QStringList headers;
    headers << "时间";
    QStringList names = fieldNames();
    QStringList units = fieldUnits();
    for (int i = 0; i < names.size(); i++)
        headers << names[i] + "(" + units[i] + ")";
    return headers;
}

int Sensor::columnCount() const {   // 表格列数：1(时间) + 字段数
    return 1 + fieldNames().size();
}