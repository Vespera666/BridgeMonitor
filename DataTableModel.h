#ifndef DATATABLEMODEL_H
#define DATATABLEMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QStringList>
#include <QVector>

#include "DataPoint.h"

class DataTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit DataTableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void loadData(const QVector<DataPoint> &records, const QStringList &headerLabels);

private:
    QVector<DataPoint> m_data;
    QStringList m_headers;
};

#endif // DATATABLEMODEL_H
