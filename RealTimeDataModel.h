#ifndef REALTIMEDATAMODEL_H
#define REALTIMEDATAMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>
#include "DataPoint.h"
#include "monitorpoint.h"

// 传感器元数据（值类型，避免裸指针的生存期问题）
struct SensorMeta
{
    QString sensorType;
    QString model;
    QStringList fieldNames;
};

// 实时数据表格模型 —— 每一行对应一个已绑定传感器的监测点，
// 展示该监测点最新的传感器采集数据。
class RealTimeDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RealTimeDataModel(QObject *parent = nullptr);

    // 载入监测点 + 传感器元数据 + 最新数据
    void loadData(const QVector<MonitoringPoint> &points,
                  const QVector<SensorMeta> &sensorMetas,
                  const QVector<DataPoint> &latestData);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // 获取第 row 行对应的监测点 ID（用于图表联动）
    QString pointIdAt(int row) const;
    QString sensorModelAt(int row) const;

private:
    QVector<MonitoringPoint> m_points;
    QVector<SensorMeta> m_sensorMetas;
    QVector<DataPoint> m_latestData;
    // 所有传感器字段名的并集，作为动态列头
    QStringList m_valueHeaders;
};

#endif // REALTIMEDATAMODEL_H
