#include "expansionjointsensor.h"
#include "filemanager.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QtMath>

ExpansionJointSensor::ExpansionJointSensor()
{

}

QString ExpansionJointSensor::sensorType() const
{
    return "伸缩缝监测传感器";
}

QStringList ExpansionJointSensor::fieldNames() const
{
    return {"伸缩量"};
}

QStringList ExpansionJointSensor::fieldUnits() const
{
    return {"mm"};
}

QVector<DataPoint> ExpansionJointSensor::loadFile(const QString &filePath)
{
    return FileManager::readCsv(filePath, this->fieldNames());
}

QVector<DataPoint> ExpansionJointSensor::generateMockData(int count)
{
    QVector<DataPoint> dataList;
    QDateTime baseTime = QDateTime::currentDateTime();
    QRandomGenerator* rng = QRandomGenerator::global();

    for (int i = 0; i < count; ++i)
    {
        DataPoint dp;
        dp.timeStamp = baseTime.addSecs(i * this->frequency);

        double expansion = 5.0 + 3.0 * qSin(i * 0.005) + (rng->generateDouble() - 0.5) * 0.5;
        if (expansion < 0) expansion = 0;
        if (expansion > 15) expansion = 15;

        dp.value.append(expansion);
        dataList.append(dp);
    }
    return dataList;
}