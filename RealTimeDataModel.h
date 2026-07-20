#ifndef REALTIMEDATAMODEL_H
#define REALTIMEDATAMODEL_H

#include <QAbstractTableModel>
#include <QStringList>
#include <QVector>
#include "DataPoint.h"
#include "monitorpoint.h"

struct SensorMeta
{
    QString sensorType;
    QString model;
    QStringList fieldNames;
};

class RealTimeDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RealTimeDataModel(QObject *parent = nullptr);

    void loadData(const QVector<MonitoringPoint> &points,
                  const QVector<SensorMeta> &sensorMetas,
                  const QVector<DataPoint> &latestData);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QString pointIdAt(int row) const;
    QString sensorModelAt(int row) const;

private:
    QVector<MonitoringPoint> m_points;
    QVector<SensorMeta> m_sensorMetas;
    QVector<DataPoint> m_latestData;
    QStringList m_valueHeaders;
};

#endif // REALTIMEDATAMODEL_H
