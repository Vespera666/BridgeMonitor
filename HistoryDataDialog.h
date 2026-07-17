#ifndef HISTORYDATADIALOG_H
#define HISTORYDATADIALOG_H

#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QFile>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTableView>
#include <QTextStream>
#include <QVBoxLayout>
#include <QVector>
#include <QtCharts/QChartView>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "DataPoint.h"
#include "DataTableModel.h"
#include "monitorpoint.h"
#include "sensor.h"

// 历史数据展示对话框 —— 选择监测点后，展示其传感器采集的历史数据
class HistoryDataDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDataDialog(QWidget *parent = nullptr);

private:
    // 文件常量
    const QString MONITOR_FILE = "monitor_storage.txt";
    const QString BIND_FILE = "bind_link.txt";
    const QString SENSOR_FILE = "sensor_storage.txt";

    // UI 控件
    QTableView *m_tableView;
    DataTableModel *m_tableModel;
    QChartView *m_chartView;
    QSplitter *m_splitter;

    QComboBox *m_cbxMonitorPoint;
    QDateEdit *m_deStartDate;
    QDateEdit *m_deEndDate;
    QPushButton *m_btnQuery;

    QLabel *m_labSensorInfo; // 显示当前传感器信息

    void initUI();
    void refreshMonitorCombo();

    // 文件读写
    QVector<MonitoringPoint> loadAllMonitorPoints();
    QString getBindSensorByPointId(const QString &pid);
    Sensor *loadSensorByModel(const QString &model);
    // 尝试从 CSV 加载真实数据，失败则回退到模拟数据
    QVector<DataPoint> loadRealOrMockData(Sensor *sensor,
                                          const QString &pointId,
                                          const QDateTime &start,
                                          const QDateTime &end);
    // 根据监测点编号查找拆分后的 CSV 文件路径
    QString findCsvPath(const QString &pointId) const;
    // 按日期范围过滤 DataPoint 列表
    void filterByDateRange(QVector<DataPoint> &data,
                           const QDateTime &start,
                           const QDateTime &end) const;
    // 绘制折线图
    void updateChart(const QVector<DataPoint> &data,
                     const QStringList &fieldNames);

private slots:
    void slotQuery();
    void slotMonitorChanged(int index);
};

#endif // HISTORYDATADIALOG_H
