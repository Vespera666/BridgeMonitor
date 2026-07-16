#ifndef MONITORPOINTTABLEMODEL_H
#define MONITORPOINTTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "monitorpoint.h"

class MonitorPointTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit MonitorPointTableModel(QObject *parent = nullptr);
    // 载入全部监测点数据 + 绑定传感器文本
    void loadData(const QVector<MonitoringPoint>& data, const QStringList& bindInfo);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    QVector<MonitoringPoint> m_points;
    QStringList m_bindSensorText; // 每行对应绑定传感器型号/未绑定
    const QStringList m_header = {"监测点编号","断面名称","安装日期","绑定传感器"};
};

#endif // MONITORPOINTTABLEMODEL_H
