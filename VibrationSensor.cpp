#include "VibrationSensor.h"
#include <QDateTime>
#include <QRandomGenerator>
#include "filemanager.h"

VibrationSensor::VibrationSensor()
{
    // 可在这里初始化设备基础信息
}

QString VibrationSensor::sensorType() const
{
    return "振动监测传感器";
}

QStringList VibrationSensor::fieldNames() const
{
    return {"振动频率"};
}

QStringList VibrationSensor::fieldUnits() const
{
    return {"Hz"};
}

QVector<DataPoint> VibrationSensor::loadFile(const QString &filePath)
{
    return FileManager::readCsv(filePath, this->fieldNames());
}

QVector<DataPoint> VibrationSensor::generateMockData(int count)
{
    QVector<DataPoint> dataList;
    QDateTime baseTime = QDateTime::currentDateTime();

    for (int i = 0; i < count; ++i) {
        DataPoint dp;
        // 时间逐秒递增
        dp.timeStamp = baseTime.addSecs(i);
        // 修复bounded歧义
        double freq = 0.5 + QRandomGenerator::global()->bounded(19.5);
        dp.value.append(freq);
        dataList.append(dp);
    }
    return dataList;
}