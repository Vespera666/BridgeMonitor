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

    // ── 三个必须实现的虚函数（QTableView 每次刷新都会调它们）──

    // 返回总行数 = 数据条数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // 返回总列数 = 时间列(1) + 字段数
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // 返回 (row, col) 单元格的内容。role 用来区分"显示文字"还是"对齐方式"等
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // ── 表头（列名）──
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // ── 加载/替换全部数据 ──
    void loadData(const QVector<DataPoint> &records, const QStringList &headerLabels);

private:
    QVector<DataPoint> m_data;
    QStringList m_headers;
};

#endif // DATATABLEMODEL_H
