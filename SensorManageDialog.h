#ifndef SENSORMANAGEDIALOG_H
#define SENSORMANAGEDIALOG_H
#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QInputDialog>
#include <QVector>
#include <QDate>
#include "sensor.h"
#include "monitorpoint.h"
#include "SensorEditDialog.h"
#include <QHeaderView>
class SensorManageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SensorManageDialog(QWidget *parent = nullptr);
    ~SensorManageDialog() override;

private:
    // 持久化文件路径
    const QString SENSOR_FILE = "sensor_storage.txt";
    const QString MONITOR_FILE = "monitor_storage.txt";
    const QString BIND_FILE = "bind_link.txt";
    const QStringList m_sensorHeader = {
        "设备名称","规格","型号","厂家","生产日期","采集频率(秒)","传感器类型","绑定监测点"
    };

    // UI控件
    QStandardItemModel* m_tableModel;
    QTableView* m_tableView;
    QComboBox* m_cmbSensorSel;
    QComboBox* m_cmbMonitorSel;

    // 初始化
    void initUI();
    void initFile();
    void refreshTable();
    void clearTable();
    void refreshCombo();

    // 文件操作
    QVector<Sensor*> loadAllSensor();
    bool saveSensor(Sensor* s, bool isUpdate);
    bool delSensorByModel(const QString& model);
    QString getBindPoint(const QString& sensorModel);
    QVector<MonitoringPoint> loadAllMonitor();

    // 绑定操作
    void bindPoint();
    void unBindPoint();

private slots:
    void slotAdd();
    void slotEdit();
    void slotDel();
    void slotRefresh();
};
#endif // SENSORMANAGEDIALOG_H