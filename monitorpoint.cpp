#include "monitorpoint.h"

QString MonitoringPoint::displayName() const
{
    return sectionName + " - " + pointId;
}

bool MonitoringPoint::hasSensorType(const QString &type) const
{
    for (const Sensor *s : sensors) {
        if (s->sensorType() == type)
            return true;
    }
    return false;
}

void MonitoringPoint::addSensor(Sensor *s)
{
    sensors.append(s);
}

void MonitoringPoint::removeSensor(int index)
{
    if (index >= 0 && index < sensors.size())
        sensors.removeAt(index);
}

int MonitoringPoint::sensorCount() const
{
    return sensors.size();
}
