#ifndef WINDSENSOR_H
#define WINDSENSOR_H

#include "sensor.h"

class WindSensor :public Sensor
{
public:
    WindSensor();
    QString sensorType();
};

#endif // WINDSENSOR_H
