#ifndef DATAPOINT_H
#define DATAPOINT_H

#endif // DATAPOINT_H

#include <QDateTime>
#include <QVector>

class DataPoint
{
public:
    QDateTime timeStamp;
    QVector<double> value;
};
