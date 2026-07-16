#include "DataTableModel.h"

DataTableModel::DataTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{}

// ---------------------------------------------------------------------------
// rowCount — "你有几行？"
//
// 不加 const 修饰返回 m_data.size() 会编译报错，因为 QTableView 要求这个方法
// 不能修改对象状态。空数据时返回 0，QTableView 自动显示空白。
// ---------------------------------------------------------------------------
int DataTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return m_data.size();
}

// ---------------------------------------------------------------------------
// columnCount — "你有几列？"
//
// 列数 = 表头字符串数量。loadData 时会把 Sensor::headerLabels() 的结果
// （如 ["时间","风速(m/s)","风向(°)"]）存进 m_headers，这里直接取它的长度。
// ---------------------------------------------------------------------------
int DataTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return m_headers.size();
}

// ---------------------------------------------------------------------------
// data — "(row, col) 这个格子里显示什么？"
//
// 这是最核心的方法。QTableView 绘制每一个可见单元格时都会调它。
// 它只渲染可见区域，所以即使 m_data 有 10000 条，这个方法也只会被调
// ~几十行 × 几列 次，不会卡。
//
// role 参数：Qt 用不同的 role 问不同的东西：
//   Qt::DisplayRole   → "要显示的文本是啥？"
//   Qt::TextAlignmentRole → "文字靠左还是靠右？"
//   Qt::ForegroundRole → "字体颜色？"
//   等等……我们只处理 DisplayRole 和 TextAlignmentRole
// ---------------------------------------------------------------------------
QVariant DataTableModel::data(const QModelIndex &index, int role) const
{
    // ----- 安全检查：索引越界，返回空 -----
    if (!index.isValid())
        return {};
    if (index.row() < 0 || index.row() >= m_data.size())
        return {};
    if (index.column() < 0 || index.column() >= m_headers.size())
        return {};

    const DataPoint &dp = m_data.at(index.row());

    // ----- 显示文字 -----
    if (role == Qt::DisplayRole) {
        // 第 0 列：时间 → 转成字符串
        if (index.column() == 0)
            return dp.timeStamp.toString("yyyy-MM-dd HH:mm:ss");

        // 第 1 列及以后：数值 → 保留合适小数位
        int valueIdx = index.column() - 1;
        if (valueIdx < dp.value.size())
            return QString::number(dp.value.at(valueIdx), 'f', 2); // 2 位小数

        return {};
    }

    // ----- 对齐方式 -----
    // 第 0 列（时间）居中，数值列靠右（方便看大小）
    if (role == Qt::TextAlignmentRole) {
        if (index.column() == 0)
            return int(Qt::AlignCenter | Qt::AlignVCenter);
        else
            return int(Qt::AlignRight | Qt::AlignVCenter);
    }

    return {}; // 其他 role 不处理
}

// ---------------------------------------------------------------------------
// headerData — "第 N 列 / 第 N 行的表头叫什么？"
//
// orientation: Horizontal = 列标题, Vertical = 行号
// ---------------------------------------------------------------------------
QVariant DataTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // 只要列标题（横向表头），行号用默认（1, 2, 3...）
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_headers.value(section);

    return {};
}

// ---------------------------------------------------------------------------
// loadData — 把新的 QVector<DataPoint> 灌进 Model
//
// beginResetModel() / endResetModel() 是给 QTableView 发信号：
// "数据要全换了，你准备好重建整个视图" / "换完了，你刷新吧"
// 不写这两个的话，QTableView 不知道数据变了，界面不会更新。
// ---------------------------------------------------------------------------
void DataTableModel::loadData(const QVector<DataPoint> &records, const QStringList &headerLabels)
{
    beginResetModel();
    m_data = records;
    m_headers = headerLabels;
    endResetModel();
}
