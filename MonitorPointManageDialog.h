#ifndef MONITORPOINTMANAGEDIALOG_H
#define MONITORPOINTMANAGEDIALOG_H

#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QTextStream>
#include <QVBoxLayout>
#include <QVector>
#include "MonitorPointEditDialog.h"
#include "MonitorPointTableModel.h"
#include "monitorpoint.h"

class MonitorPointManageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MonitorPointManageDialog(QWidget *parent = nullptr);

private:
    // 文件常量
    const QString MONITOR_FILE = "monitor_storage.txt";
    const QString SENSOR_FILE = "sensor_storage.txt";

    // UI控件
    QTableView *m_tableView;
    MonitorPointTableModel *m_tableModel;

    QComboBox *m_cbxPointSelect;
    QComboBox *m_cbxSensorSelect;
    QPushButton *m_btnBind;
    QPushButton *m_btnUnBind;

    void initUI();
    void initFile();
    void refreshTable();
    void clearTable();
    void refreshBindCombobox();

    // 文件读写
    QVector<MonitoringPoint> loadAllMonitorPoints();
    bool saveMonitorPoint(const MonitoringPoint &mp, bool isUpdate, const QString &oldPointId);
    bool deletePointById(const QString &pointId);
    QString getBindSensorByPointId(const QString &pid);
    QStringList loadAllSensorModel();

    // ===== 新增：一对一绑定相关函数 =====
    bool isPointAlreadyBound(const QString &pointId);
    bool bindSensorToPoint(const QString &sensorId, const QString &pointId);
    bool unbindPoint(const QString &pointId);

private slots:
    void slotAddPoint();
    void slotEditPoint();
    void slotDelPoint();
    void slotRefresh();
    void slotDoBind();
    void slotDoUnBind();
    void slotPointComboboxChanged(const QString &pointId);
};

#endif // MONITORPOINTMANAGEDIALOG_H
