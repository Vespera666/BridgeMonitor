#include "displacement.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>

// ============================================================
// 测点映射初始化
//
// 数据文件中有 8 列传感器数据：
//   L1_ZY11_1, L1_ZY12_1  → DISP001（桥墩 L1）
//   L3_ZY21_1, L3_ZY22_1  → DISP002（桥墩 L3）
//   L11_ZY31_1, L11_ZY32_1 → DISP003（桥墩 L11）
//   L13_ZY41_1, L13_ZY42_1 → DISP004（桥墩 L13）
// ============================================================
void DisplacementSensor::buildPointMap()
{
    m_points.clear();
    m_colToPoint.clear();

    m_points["DISP001"] = {"L1_ZY11_1", "L1_ZY12_1"};
    m_points["DISP002"] = {"L3_ZY21_1", "L3_ZY22_1"};
    m_points["DISP003"] = {"L11_ZY31_1", "L11_ZY32_1"};
    m_points["DISP004"] = {"L13_ZY41_1", "L13_ZY42_1"};

    for (auto it = m_points.begin(); it != m_points.end(); ++it) {
        for (const auto &col : it.value())
            m_colToPoint[col] = it.key();
    }
}

// ============================================================
// Sensor 接口实现
// ============================================================
QString DisplacementSensor::sensorType() const
{
    return QStringLiteral("支座位移");
}

QStringList DisplacementSensor::fieldNames() const
{
    return {QStringLiteral("位移量")};
}

QStringList DisplacementSensor::fieldUnits() const
{
    return {QStringLiteral("mm")};
}

// ============================================================
// loadFile - 从 TSV 文件加载支座位移数据
//
// 文件虽然后缀是 .xls，实际是制表符分隔的文本文件（GBK 编码）。
// 第一行是列名，后续每行是一条时间点上的多列采集数据。
// ============================================================
QVector<DataPoint> DisplacementSensor::loadFile(const QString &filePath)
{
    clearRecords();
    buildPointMap();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        return {};
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);  // GBK 编码

    // 第一行：列名
    QString headerLine = in.readLine();
    if (headerLine.isEmpty()) {
        qWarning() << "文件首行为空:" << filePath;
        return {};
    }

    QStringList headers = headerLine.split('\t');

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList cols = line.split('\t');
        if (cols.size() < headers.size()) continue;

        QDateTime timestamp = Sensor::parseTime(cols[0]);
        if (!timestamp.isValid()) continue;

        for (int i = 1; i < headers.size(); i++) {
            QString colName = headers[i].trimmed();
            bool ok = false;
            double value = cols[i].toDouble(&ok);
            if (!ok) continue;

            QString pointId = m_colToPoint.value(colName, "");
            if (pointId.isEmpty()) continue;

            DataPoint dp;
            dp.timestamp   = timestamp;
            dp.pointId     = pointId;
            dp.sensorLabel = colName;
            dp.values      = {value};
            m_records.append(dp);
        }
    }

    file.close();
    qDebug() << "支座位移数据加载完成:" << m_records.size() << "条记录";
    return m_records;
}

// ============================================================
// generateMockData - 生成模拟支座位移数据
//
// 4 个测点各生成 count 条数据，正弦波 + 随机噪声，
// 位移范围约 -5 ~ +5 mm。
// ============================================================
QVector<DataPoint> DisplacementSensor::generateMockData(int count)
{
    clearRecords();
    buildPointMap();

    QStringList pointIds = {"DISP001", "DISP002", "DISP003", "DISP004"};
    QDateTime base = QDateTime(QDate(2023, 4, 21), QTime(10, 10, 0));

    for (int i = 0; i < count; ++i) {
        QDateTime ts = base.addSecs(i * 600);  // 每 10 分钟一条

        for (const auto &pid : pointIds) {
            // 不同测点不同相位，模拟真实桥梁不同位置的位移差异
            double phase = (pointIds.indexOf(pid) * 0.8);
            double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 1.0;
            double value = 3.0 * qSin(i * 0.15 + phase)
                         + 1.5 * qSin(i * 0.05)
                         + noise;

            DataPoint dp;
            dp.timestamp   = ts;
            dp.pointId     = pid;
            dp.sensorLabel = m_points.value(pid).first();  // 取第一个列名
            dp.values      = {value};
            m_records.append(dp);
        }
    }

    qDebug() << "模拟支座位移数据生成:" << m_records.size() << "条记录";
    return m_records;
}

// ============================================================
// 数据查询
// ============================================================
QVector<DataPoint> DisplacementSensor::allRecords() const
{
    return m_records;
}

QVector<DataPoint> DisplacementSensor::recordsByPoint(const QString &pointId) const
{
    QVector<DataPoint> result;
    for (const auto &r : m_records) {
        if (r.pointId == pointId)
            result.append(r);
    }
    return result;
}

QVector<DataPoint> DisplacementSensor::recentRecords(int count) const
{
    if (m_records.size() <= count)
        return m_records;
    return m_records.mid(m_records.size() - count);
}

void DisplacementSensor::clearRecords()
{
    m_records.clear();
}

QMap<QString, QStringList> DisplacementSensor::points() const
{
    return m_points;
}
