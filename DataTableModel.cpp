#include "DataTableModel.h"

DataTableModel::DataTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{}


int DataTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_data.size();
}


int DataTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return m_headers.size();
}


QVariant DataTableModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid())
        return {};
    if (index.row() < 0 || index.row() >= m_data.size())
        return {};
    if (index.column() < 0 || index.column() >= m_headers.size())
        return {};

    const DataPoint &dp = m_data.at(index.row());


    if (role == Qt::DisplayRole) {

        if (index.column() == 0)
            return dp.timeStamp.toString("yyyy-MM-dd HH:mm:ss");


        int valueIdx = index.column() - 1;
        if (valueIdx < dp.value.size())
            return QString::number(dp.value.at(valueIdx), 'f', 2); // 2 位小数

        return {};
    }


    if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0)
            return int(Qt::AlignCenter | Qt::AlignVCenter);
        else
            return int(Qt::AlignRight | Qt::AlignVCenter);
    }

    return {};
}


QVariant DataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_headers.value(section);

    return {};
}

void DataTableModel::loadData(const QVector<DataPoint> &records, const QStringList &headerLabels)
{
    beginResetModel();
    m_data = records;
    m_headers = headerLabels;
    endResetModel();
}
