#include "sensor.h"

QStringList Sensor::headerLabels() const
{
    QStringList headers;
    headers << "时间";
    QStringList names = fieldNames();
    QStringList units = fieldUnits();
    for (int i = 0; i < names.size(); i++)
        headers << names[i] + "(" + units[i] + ")";
    return headers;
}

int Sensor::columnCount() const
{
    return 1 + fieldNames().size();
}
