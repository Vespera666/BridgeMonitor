#include "CableForceSensor.h"
#include <QDateTime>
#include <QRandomGenerator>
#include "filemanager.h"

CableForceSensor::CableForceSensor() {}

QString CableForceSensor::sensorType() const
{
    return "索力监测传感器";
}

QStringList CableForceSensor::fieldNames() const
{
    return {"索力值"};
}

QStringList CableForceSensor::fieldUnits() const
{
    return {"kN"};
}

QVector<DataPoint> CableForceSensor::loadFile(const QString &filePath)
{
    return FileManager::readCsv(filePath, this->fieldNames());
}

QVector<DataPoint> CableForceSensor::generateMockData(int count)
{
    QVector<DataPoint> dataList;
    QDateTime baseTime = QDateTime::currentDateTime();

    for (int i = 0; i < count; ++i) {
        DataPoint dp;
        dp.timeStamp = baseTime.addSecs(i);
        // 修复bounded歧义问题
        double force = 1000.0 + QRandomGenerator::global()->bounded(7000.0);
        dp.value.append(force);
        dataList.append(dp);
    }
    return dataList;
}