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
    if (sensor)
        sensor->boundPoint = nullptr;

    sensor = s;

    if (s)
        s->boundPoint = this;
}

void MonitoringPoint::unbindSensor()
{
    if (sensor)
        sensor->boundPoint = nullptr;
    sensor = nullptr;
}

bool MonitoringPoint::hasSensor() const
{
    return sensor != nullptr;
}
