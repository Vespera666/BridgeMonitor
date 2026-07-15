#include "deflection.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QtMath>
#include <QRandomGenerator>

// ============================================================
// 测点映射初始化
//
// 数据文件中有 14 列传感器数据：
//   L4_ND11_1,  L4_ND12_1  → DEFL001    L8_ND51_1,  L8_ND52_1  → DEFL005
//   L5_ND21_1,  L5_ND22_1  → DEFL002    L9_ND61_1,  L9_ND62_1  → DEFL006
//   L6_ND31_1,  L6_ND32_1  → DEFL003    L10_ND71_1, L10_ND72_1 → DEFL007
//   L7_ND41_1,  L7_ND42_1  → DEFL004
// ============================================================
void DeflectionSensor::buildPointMap()
{
    m_points.clear();
    m_colToPoint.clear();

    m_points["DEFL001"] = {"L4_ND11_1",  "L4_ND12_1"};
    m_points["DEFL002"] = {"L5_ND21_1",  "L5_ND22_1"};
    m_points["DEFL003"] = {"L6_ND31_1",  "L6_ND32_1"};
    m_points["DEFL004"] = {"L7_ND41_1",  "L7_ND42_1"};
    m_points["DEFL005"] = {"L8_ND51_1",  "L8_ND52_1"};
    m_points["DEFL006"] = {"L9_ND61_1",  "L9_ND62_1"};
    m_points["DEFL007"] = {"L10_ND71_1", "L10_ND72_1"};

    for (auto it = m_points.begin(); it != m_points.end(); ++it) {
        for (const auto &col : it.value())
            m_colToPoint[col] = it.key();
    }
}

// ============================================================
// Sensor 接口实现
// ============================================================
QString DeflectionSensor::sensorType() const
{
    return QStringLiteral("挠度");
}

QStringList DeflectionSensor::fieldNames() const
{
    return {QStringLiteral("挠度")};
}

QStringList DeflectionSensor::fieldUnits() const
{
    return {QStringLiteral("mm")};
}

// ============================================================
// loadFile - 从 TSV 文件加载挠度数据
// ============================================================
QVector<DataPoint> DeflectionSensor::loadFile(const QString &filePath)
{
    clearRecords();
    buildPointMap();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        return {};
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::System);

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
    qDebug() << "挠度数据加载完成:" << m_records.size() << "条记录";
    return m_records;
}

// ============================================================
// generateMockData - 生成模拟挠度数据
//
// 7 个测点各生成 count 条数据，正弦波 + 随机噪声，
// 挠度范围约 -50 ~ +50 mm。
// ============================================================
QVector<DataPoint> DeflectionSensor::generateMockData(int count)
{
    clearRecords();
    buildPointMap();

    QStringList pointIds = {"DEFL001", "DEFL002", "DEFL003", "DEFL004",
                            "DEFL005", "DEFL006", "DEFL007"};
    QDateTime base = QDateTime(QDate(2023, 4, 21), QTime(10, 10, 0));

    for (int i = 0; i < count; ++i) {
        QDateTime ts = base.addSecs(i * 600);

        for (const auto &pid : pointIds) {
            double phase = (pointIds.indexOf(pid) * 0.6);
            double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 8.0;
            double value = 35.0 * qSin(i * 0.12 + phase)
                         + 15.0 * qSin(i * 0.04)
                         + noise;

            DataPoint dp;
            dp.timestamp   = ts;
            dp.pointId     = pid;
            dp.sensorLabel = m_points.value(pid).first();
            dp.values      = {value};
            m_records.append(dp);
        }
    }

    qDebug() << "模拟挠度数据生成:" << m_records.size() << "条记录";
    return m_records;
}

// ============================================================
// 数据查询
// ============================================================
QVector<DataPoint> DeflectionSensor::allRecords() const
{
    return m_records;
}

QVector<DataPoint> DeflectionSensor::recordsByPoint(const QString &pointId) const
{
    QVector<DataPoint> result;
    for (const auto &r : m_records) {
        if (r.pointId == pointId)
            result.append(r);
    }
    return result;
}

QVector<DataPoint> DeflectionSensor::recentRecords(int count) const
{
    if (m_records.size() <= count)
        return m_records;
    return m_records.mid(m_records.size() - count);
}

void DeflectionSensor::clearRecords()
{
    m_records.clear();
}

QMap<QString, QStringList> DeflectionSensor::points() const
{
    return m_points;
}
