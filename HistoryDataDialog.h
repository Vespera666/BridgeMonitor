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
#include <QTableView>
#include <QTextStream>
#include <QVBoxLayout>
#include <QVector>
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
    QVector<DataPoint> loadHistoryData(Sensor *sensor,
                                       const QDateTime &start,
                                       const QDateTime &end);

private slots:
    void slotQuery();
    void slotMonitorChanged(int index);
};

#endif // HISTORYDATADIALOG_H
