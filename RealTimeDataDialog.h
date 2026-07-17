#ifndef REALTIMEDATADIALOG_H
#define REALTIMEDATADIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QFile>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
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
#include "RealTimeDataModel.h"
#include "monitorpoint.h"
#include "sensor.h"

// 实时数据展示 —— 左表格+右图表，只展示已绑定传感器的监测点
// 绑定关系来自 sensor_storage.txt 最后一列
class RealTimeDataDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RealTimeDataDialog(QWidget *parent = nullptr);

private:
    const QString MONITOR_FILE = "monitor_storage.txt";
    const QString SENSOR_FILE = "sensor_storage.txt";

    QSplitter *m_splitter;
    QTableView *m_tableView;
    RealTimeDataModel *m_tableModel;
    QLabel *m_labTableHint;
    QChartView *m_chartView;
    QSpinBox *m_spinCount;
    QLabel *m_labChartHint;
    QPushButton *m_btnRefresh;

    void initUI();
    void refreshTable();

    QVector<MonitoringPoint> loadAllMonitorPoints();
    QVector<Sensor *> loadAllSensors();
    // 从 sensor_storage.txt 最后一列查绑定关系
    QString getBoundSensorModelByPoint(const QString &pointId);
    // 返回所有已绑定的监测点 ID 列表
    QStringList getAllBoundPointIds();
    Sensor *findSensorByModel(const QString &model, const QVector<Sensor *> &sensors);
    SensorMeta extractSensorMeta(Sensor *s) const;
    QString findCsvPath(const QString &pointId) const;
    void updateRealtimeChart(const QString &pointId, int count);

private slots:
    void slotRefresh();
    void slotRowSelected(const QModelIndex &current, const QModelIndex &previous);
    void slotCountChanged(int count);
};

#endif // REALTIMEDATADIALOG_H
