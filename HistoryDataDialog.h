#ifndef HISTORYDATADIALOG_H
#define HISTORYDATADIALOG_H

#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QFile>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTabWidget>
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

struct SensorFieldInfo
{
    QStringList fieldNames;
    QStringList fieldUnits;
    int fieldCount;
};

struct HistoryTabPage
{
    QWidget *widget = nullptr;
    QComboBox *cbxPoint = nullptr;
    QDateEdit *deStart = nullptr;
    QDateEdit *deEnd = nullptr;
    QPushButton *btnQuery = nullptr;
    QLabel *labInfo = nullptr;
    QTableView *tableView = nullptr;
    DataTableModel *tableModel = nullptr;
    QChartView *chartView = nullptr;
    QSplitter *splitter = nullptr;
};

class HistoryDataDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDataDialog(QWidget *parent = nullptr);

    static SensorFieldInfo fieldInfoForType(const QString &dataType);

private:
    const QString MONITOR_FILE = "monitor_storage.txt";

    const QStringList SENSOR_TYPES = {
        "索力监测传感器", "挠度传感器", "振动监测传感器",
        "支座位移传感器", "伸缩缝监测传感器", "风速风向传感器", "温湿度监测传感器",
    };
    const QStringList SENSOR_LABELS = {
        "索力监测", "挠度", "振动", "支座位移", "伸缩缝", "风速风向", "温湿度",
    };

    QTabWidget *m_tabWidget;
    QVector<HistoryTabPage> m_tabs;

    void initUI();
    void createTabPage(int index);

    QVector<MonitoringPoint> loadAllMonitorPoints();
    QVector<DataPoint> loadDataForPoint(const QString &pointId,
                                        const QString &dataType,
                                        const QDateTime &start,
                                        const QDateTime &end);
    QString findCsvPath(const QString &pointId) const;
    void filterByDateRange(QVector<DataPoint> &data,
                           const QDateTime &start,
                           const QDateTime &end) const;
    static void updateChartForView(QChartView *chartView,
                                   const QVector<DataPoint> &data,
                                   const QStringList &fieldNames);

private slots:
    void slotQuery(int tabIndex);
    void slotMonitorChanged(int tabIndex);
};

#endif // HISTORYDATADIALOG_H
