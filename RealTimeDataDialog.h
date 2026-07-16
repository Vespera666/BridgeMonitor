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
#include <QTableView>
#include <QTextStream>
#include <QVBoxLayout>
#include <QVector>
#include "DataPoint.h"
#include "RealTimeDataModel.h"
#include "monitorpoint.h"
#include "sensor.h"

// 实时数据展示对话框 —— 表格展示所有监测点最新传感器读数
class RealTimeDataDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RealTimeDataDialog(QWidget *parent = nullptr);

private:
    // 文件常量
    const QString MONITOR_FILE = "monitor_storage.txt";
    const QString BIND_FILE = "bind_link.txt";
    const QString SENSOR_FILE = "sensor_storage.txt";

    // UI 控件
    QTableView *m_tableView;
    RealTimeDataModel *m_tableModel;

    void initUI();
    void refreshTable();
    void clearTable();

    // 文件读写（与 MonitorPointManageDialog 保持一致）
    QVector<MonitoringPoint> loadAllMonitorPoints();
    QVector<Sensor *> loadAllSensors();
    QString getBindSensorByPointId(const QString &pid);
    Sensor *findSensorByModel(const QString &model, const QVector<Sensor *> &sensors);
    // 将 Sensor* 提取为值类型的 SensorMeta（避免指针生存期问题）
    SensorMeta extractSensorMeta(Sensor *s) const;

private slots:
    void slotRefresh();
};

#endif // REALTIMEDATADIALOG_H
