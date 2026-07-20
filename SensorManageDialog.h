#ifndef SENSORMANAGEDIALOG_H
#define SENSORMANAGEDIALOG_H
#include <QComboBox>
#include <QDate>
#include <QDialog>
#include <QFile>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QTextStream>
#include <QVBoxLayout>
#include <QVector>
#include "SensorEditDialog.h"
#include "monitorpoint.h"
#include "sensor.h"
class SensorManageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SensorManageDialog(QWidget *parent = nullptr);
    ~SensorManageDialog() override;

private:
    const QString SENSOR_FILE = "sensor_storage.txt";
    const QString MONITOR_FILE = "monitor_storage.txt";
    const QStringList m_sensorHeader = {"设备名称",
                                        "规格",
                                        "型号",
                                        "厂家",
                                        "生产日期",
                                        "采集频率(秒)",
                                        "传感器类型",
                                        "绑定监测点"};

    QStandardItemModel *m_tableModel;
    QTableView *m_tableView;
    QComboBox *m_cmbSensorSel;
    QComboBox *m_cmbMonitorSel;

    void initUI();
    void initFile();
    void refreshTable();
    void clearTable();
    void refreshCombo();

    QVector<Sensor *> loadAllSensor();
    bool saveSensor(Sensor *s, bool isUpdate);
    bool delSensorByModel(const QString &model);
    QString getBindPoint(const QString &sensorModel);
    QVector<MonitoringPoint> loadAllMonitor();

    bool isPointHasBind(const QString &monId);

    void bindPoint();
    void unBindPoint();
private slots:
    void slotAdd();
    void slotEdit();
    void slotDel();
    void slotRefresh();
};
#endif // SENSORMANAGEDIALOG_H
