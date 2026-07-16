#include "monitorpoint.h"

QString MonitoringPoint::displayName() const
{
    return sectionName + " - " + pointId;
}

QString MonitoringPoint::sensorType() const
{
    return sensor ? sensor->sensorType() : QString();
}

void MonitoringPoint::bindSensor(Sensor *s)
{
    sensor = s;
}

void MonitoringPoint::unbindSensor()
{
    sensor = nullptr;
}

bool MonitoringPoint::hasSensor() const
{
    return sensor != nullptr;
}
