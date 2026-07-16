#include "RealTimeDataModel.h"

RealTimeDataModel::RealTimeDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

void RealTimeDataModel::loadData(const QVector<MonitoringPoint> &points,
                                 const QVector<SensorMeta> &sensorMetas,
                                 const QVector<DataPoint> &latestData)
{
    beginResetModel();
    m_points = points;
    m_sensorMetas = sensorMetas;
    m_latestData = latestData;

    // 收集所有传感器字段名的并集作为动态列头
    QStringList unionFields;
    for (const SensorMeta &meta : sensorMetas) {
        for (const QString &fn : meta.fieldNames) {
            if (!unionFields.contains(fn))
                unionFields.append(fn);
        }
    }
    m_valueHeaders = unionFields;

    endResetModel();
}

int RealTimeDataModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_points.size();
}

int RealTimeDataModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    // 固定列: 监测点编号, 断面名称, 传感器类型, 传感器型号, 采集时间
    // + 动态数值列
    return 5 + m_valueHeaders.size();
}

QVariant RealTimeDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    int r = index.row();
    int c = index.column();
    if (r >= m_points.size() || c >= columnCount())
        return {};

    const MonitoringPoint &mp = m_points[r];

    if (role == Qt::DisplayRole) {
        // ---- 固定列 ----
        switch (c) {
        case 0:
            return mp.pointId;
        case 1:
            return mp.sectionName;
        case 2:
            return (r < m_sensorMetas.size())
                       ? m_sensorMetas[r].sensorType
                       : "未绑定";
        case 3:
            return (r < m_sensorMetas.size())
                       ? m_sensorMetas[r].model
                       : QString();
        case 4:
            return (r < m_latestData.size())
                       ? m_latestData[r].timeStamp.toString("yyyy-MM-dd HH:mm:ss")
                       : QString();
        default: {
            // ---- 动态数值列 ----
            int fieldIdx = c - 5;
            if (r >= m_sensorMetas.size())
                return "—";
            const QStringList &sensorFields = m_sensorMetas[r].fieldNames;
            int posInSensor = sensorFields.indexOf(m_valueHeaders[fieldIdx]);
            if (posInSensor < 0)
                return "—"; // 该传感器无此字段
            if (r >= m_latestData.size())
                return "—";
            const DataPoint &dp = m_latestData[r];
            if (posInSensor >= dp.value.size())
                return "—";
            return QString::number(dp.value[posInSensor], 'f', 2);
        }
        }
    }

    if (role == Qt::TextAlignmentRole)
        return int(Qt::AlignCenter | Qt::AlignVCenter);

    return {};
}

QVariant RealTimeDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        const QStringList fixedHeaders = {"监测点编号",
                                          "断面名称",
                                          "传感器类型",
                                          "传感器型号",
                                          "采集时间"};
        if (section < fixedHeaders.size())
            return fixedHeaders[section];
        int fieldIdx = section - fixedHeaders.size();
        if (fieldIdx < m_valueHeaders.size())
            return m_valueHeaders[fieldIdx];
    }
    return {};
}
