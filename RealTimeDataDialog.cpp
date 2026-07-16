#include "RealTimeDataDialog.h"
#include "CableForceSensor.h"
#include "VibrationSensor.h"
#include "deflection.h"
#include "displacement.h"
#include "expansionjointsensor.h"
#include "temperaturehumiditysensor.h"
#include "windsensor.h"

RealTimeDataDialog::RealTimeDataDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("实时数据展示");
    resize(1100, 650);
    initUI();
    refreshTable();
}

void RealTimeDataDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 顶部按钮栏
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnRefresh = new QPushButton("刷新数据");
    btnLayout->addWidget(btnRefresh);
    btnLayout->addStretch();

    QLabel *labHint = new QLabel("表格展示所有已绑定传感器的监测点最新采集数据");
    labHint->setStyleSheet("color: gray;");

    // 表格视图
    m_tableModel = new RealTimeDataModel(this);
    m_tableView = new QTableView();
    m_tableView->setModel(m_tableModel);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 组装布局
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(labHint);
    mainLayout->addWidget(m_tableView);

    // 信号槽
    connect(btnRefresh, &QPushButton::clicked, this, &RealTimeDataDialog::slotRefresh);
}

void RealTimeDataDialog::clearTable()
{
    QVector<MonitoringPoint> emptyPoints;
    QVector<SensorMeta> emptyMetas;
    QVector<DataPoint> emptyData;
    m_tableModel->loadData(emptyPoints, emptyMetas, emptyData);
}

void RealTimeDataDialog::refreshTable()
{
    clearTable();

    // 1. 加载所有监测点
    QVector<MonitoringPoint> allPoints = loadAllMonitorPoints();
    // 2. 加载所有传感器
    QVector<Sensor *> allSensors = loadAllSensors();

    // 3. 筛选出已绑定传感器的监测点，提取传感器元数据
    QVector<MonitoringPoint> boundPoints;
    QVector<SensorMeta> boundSensorMetas;
    QVector<DataPoint> latestDataList;

    for (const MonitoringPoint &mp : allPoints) {
        QString sensorModel = getBindSensorByPointId(mp.pointId);
        if (sensorModel.isEmpty())
            continue; // 未绑定则跳过

        Sensor *s = findSensorByModel(sensorModel, allSensors);
        if (!s)
            continue;

        boundPoints.append(mp);
        boundSensorMetas.append(extractSensorMeta(s));

        // 为每个传感器生成一条最新模拟数据
        QVector<DataPoint> mock = s->generateMockData(1);
        if (!mock.isEmpty())
            latestDataList.append(mock.first());
        else
            latestDataList.append(DataPoint());
    }

    m_tableModel->loadData(boundPoints, boundSensorMetas, latestDataList);

    // 清理传感器对象
    qDeleteAll(allSensors);
    allSensors.clear();
}

QVector<MonitoringPoint> RealTimeDataDialog::loadAllMonitorPoints()
{
    QVector<MonitoringPoint> res;
    QFile f(MONITOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "打开监测点文件失败";
        return res;
    }
    QTextStream st(&f);
    st.setEncoding(QStringConverter::Utf8);
    bool skipHead = true;
    while (!st.atEnd()) {
        QString line = st.readLine().trimmed();
        if (line.isEmpty())
            continue;
        if (skipHead) {
            skipHead = false;
            continue;
        }
        QStringList cols = line.split(",", Qt::KeepEmptyParts);
        if (cols.size() < 3)
            continue;
        MonitoringPoint mp;
        mp.pointId = cols[0];
        mp.sectionName = cols[1];
        mp.installDate = QDate::fromString(cols[2], "yyyy-MM-dd");
        mp.sensor = nullptr;
        res.append(mp);
    }
    f.close();
    return res;
}

QVector<Sensor *> RealTimeDataDialog::loadAllSensors()
{
    QVector<Sensor *> res;
    QFile f(SENSOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return res;
    QTextStream st(&f);
    st.setEncoding(QStringConverter::Utf8);
    bool skipHead = true;
    while (!st.atEnd()) {
        QString line = st.readLine().trimmed();
        if (line.isEmpty())
            continue;
        if (skipHead) {
            skipHead = false;
            continue;
        }
        QStringList cols = line.split(",", Qt::KeepEmptyParts);
        if (cols.size() < 7)
            continue;

        Sensor *s = nullptr;
        QString type = cols[6];
        if (type == "风速风向传感器")
            s = new WindSensor();
        else if (type == "振动监测传感器")
            s = new VibrationSensor();
        else if (type == "索力监测传感器")
            s = new CableForceSensor();
        else if (type == "挠度传感器")
            s = new DeflectionSensor();
        else if (type == "支座位移传感器")
            s = new DisplacementSensor();
        else if (type == "伸缩缝监测传感器")
            s = new ExpansionJointSensor();
        else if (type == "温湿度监测传感器")
            s = new TemperatureHumiditySensor();
        else
            s = new WindSensor();

        s->name = cols[0];
        s->size = cols[1];
        s->model = cols[2];
        s->manufacturer = cols[3];
        s->generDate = QDate::fromString(cols[4], "yyyy-MM-dd");
        s->frequency = cols[5].toInt();
        s->boundPoint = nullptr;
        res.append(s);
    }
    f.close();
    return res;
}

QString RealTimeDataDialog::getBindSensorByPointId(const QString &pid)
{
    QFile f(BIND_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";
    QTextStream st(&f);
    while (!st.atEnd()) {
        QString line = st.readLine().trimmed();
        QStringList cols = line.split(",");
        if (cols.size() >= 2 && cols[1] == pid) {
            f.close();
            return cols[0];
        }
    }
    f.close();
    return "";
}

Sensor *RealTimeDataDialog::findSensorByModel(const QString &model,
                                              const QVector<Sensor *> &sensors)
{
    for (Sensor *s : sensors) {
        if (s && s->model == model)
            return s;
    }
    return nullptr;
}

SensorMeta RealTimeDataDialog::extractSensorMeta(Sensor *s) const
{
    SensorMeta meta;
    if (s) {
        meta.sensorType = s->sensorType();
        meta.model = s->model;
        meta.fieldNames = s->fieldNames();
    }
    return meta;
}

void RealTimeDataDialog::slotRefresh()
{
    refreshTable();
    QMessageBox::information(this, "刷新", "实时数据已更新");
}
