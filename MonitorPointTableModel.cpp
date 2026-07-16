#include "MonitorPointTableModel.h"

MonitorPointTableModel::MonitorPointTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

void MonitorPointTableModel::loadData(const QVector<MonitoringPoint> &data,
                                      const QStringList &bindInfo)
{
    beginResetModel();
    m_points = data;
    m_bindSensorText = bindInfo;
    endResetModel();
}

int MonitorPointTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_points.size();
}

int MonitorPointTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_header.size();
}

QVariant MonitorPointTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};
    int r = index.row();
    int c = index.column();
    if (r >= m_points.size() || c >= m_header.size())
        return {};

    const MonitoringPoint &mp = m_points[r];
    if (role == Qt::DisplayRole) {
        switch (c) {
        case 0:
            return mp.pointId;
        case 1:
            return mp.sectionName;
        case 2:
            return mp.installDate.toString("yyyy-MM-dd");
        case 3:
            return m_bindSensorText[r];
        default:
            return {};
        }
    }
    if (role == Qt::TextAlignmentRole)
        return int(Qt::AlignCenter | Qt::AlignVCenter);
    return {};
}

QVariant MonitorPointTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section < m_header.size())
        return m_header[section];
    return {};
}
