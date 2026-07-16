#include "HistoryDataDialog.h"
#include "CableForceSensor.h"
#include "VibrationSensor.h"
#include "deflection.h"
#include "displacement.h"
#include "expansionjointsensor.h"
#include "temperaturehumiditysensor.h"
#include "windsensor.h"

HistoryDataDialog::HistoryDataDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("历史数据展示");
    resize(1100, 650);
    initUI();
    refreshMonitorCombo();
}

void HistoryDataDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 顶部过滤区域
    QGroupBox *groupFilter = new QGroupBox("查询条件");
    QHBoxLayout *filterLayout = new QHBoxLayout(groupFilter);

    filterLayout->addWidget(new QLabel("监测点："));
    m_cbxMonitorPoint = new QComboBox();
    m_cbxMonitorPoint->setMinimumWidth(180);
    filterLayout->addWidget(m_cbxMonitorPoint);

    filterLayout->addSpacing(15);
    filterLayout->addWidget(new QLabel("起始日期："));
    m_deStartDate = new QDateEdit(QDate::currentDate().addDays(-7));
    m_deStartDate->setDisplayFormat("yyyy-MM-dd");
    m_deStartDate->setCalendarPopup(true);
    filterLayout->addWidget(m_deStartDate);

    filterLayout->addSpacing(10);
    filterLayout->addWidget(new QLabel("截止日期："));
    m_deEndDate = new QDateEdit(QDate::currentDate());
    m_deEndDate->setDisplayFormat("yyyy-MM-dd");
    m_deEndDate->setCalendarPopup(true);
    filterLayout->addWidget(m_deEndDate);

    filterLayout->addSpacing(20);
    m_btnQuery = new QPushButton("查询历史数据");
    filterLayout->addWidget(m_btnQuery);
    filterLayout->addStretch();

    groupFilter->setMaximumHeight(90);

    // 传感器信息标签
    m_labSensorInfo = new QLabel("请先选择一个已绑定传感器的监测点");
    m_labSensorInfo->setStyleSheet("color: #555; font-weight: bold; padding: 4px;");

    // 表格视图
    m_tableModel = new DataTableModel(this);
    m_tableView = new QTableView();
    m_tableView->setModel(m_tableModel);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 组装布局
    mainLayout->addWidget(groupFilter);
    mainLayout->addWidget(m_labSensorInfo);
    mainLayout->addWidget(m_tableView);

    // 信号槽
    connect(m_btnQuery, &QPushButton::clicked, this, &HistoryDataDialog::slotQuery);
    connect(m_cbxMonitorPoint,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &HistoryDataDialog::slotMonitorChanged);
}

void HistoryDataDialog::refreshMonitorCombo()
{
    m_cbxMonitorPoint->clear();
    QVector<MonitoringPoint> points = loadAllMonitorPoints();
    for (const MonitoringPoint &mp : points) {
        // 显示格式：监测点编号 - 断面名称
        m_cbxMonitorPoint->addItem(
            QString("%1 - %2").arg(mp.pointId, mp.sectionName),
            mp.pointId);
    }
    // 触发一次以更新传感器信息
    slotMonitorChanged(m_cbxMonitorPoint->currentIndex());
}

QVector<MonitoringPoint> HistoryDataDialog::loadAllMonitorPoints()
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

QString HistoryDataDialog::getBindSensorByPointId(const QString &pid)
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

Sensor *HistoryDataDialog::loadSensorByModel(const QString &model)
{
    QFile f(SENSOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return nullptr;
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
        if (cols[2] != model)
            continue;

        // 找到目标传感器
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
        f.close();
        return s;
    }
    f.close();
    return nullptr;
}

QVector<DataPoint> HistoryDataDialog::loadHistoryData(Sensor *sensor,
                                                      const QDateTime &start,
                                                      const QDateTime &end)
{
    if (!sensor)
        return {};

    // 根据时间范围计算需要生成的数据条数
    // 以传感器的采集频率为间隔生成模拟数据
    qint64 totalSecs = start.secsTo(end);
    if (totalSecs <= 0)
        return {};

    int interval = sensor->frequency > 0 ? sensor->frequency : 1;
    int count = static_cast<int>(totalSecs / interval);
    // 限制最多显示 2000 条，避免界面卡顿
    if (count > 2000)
        count = 2000;

    QVector<DataPoint> allData = sensor->generateMockData(count);

    // 调整时间戳使其落在指定范围内
    QVector<DataPoint> filtered;
    for (int i = 0; i < allData.size(); i++) {
        DataPoint &dp = allData[i];
        // 重新设置时间戳为范围内的均匀分布
        dp.timeStamp = start.addSecs(i * interval);

        // 按日期范围过滤
        if (dp.timeStamp >= start && dp.timeStamp <= end)
            filtered.append(dp);
    }

    return filtered;
}

void HistoryDataDialog::slotQuery()
{
    QString pointId = m_cbxMonitorPoint->currentData().toString();
    if (pointId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择监测点");
        return;
    }

    QDateTime start(m_deStartDate->date(), QTime(0, 0, 0));
    QDateTime end(m_deEndDate->date(), QTime(23, 59, 59));

    if (start > end) {
        QMessageBox::warning(this, "提示", "起始日期不能晚于截止日期");
        return;
    }

    // 查找绑定的传感器型号
    QString sensorModel = getBindSensorByPointId(pointId);
    if (sensorModel.isEmpty()) {
        QMessageBox::information(this, "提示", "该监测点未绑定传感器，无法查看历史数据");
        return;
    }

    // 加载传感器对象
    Sensor *sensor = loadSensorByModel(sensorModel);
    if (!sensor) {
        QMessageBox::warning(this, "错误", "未找到绑定的传感器信息");
        return;
    }

    // 加载并过滤数据
    QVector<DataPoint> data = loadHistoryData(sensor, start, end);
    if (data.isEmpty()) {
        QMessageBox::information(this, "提示", "所选时间范围内无数据");
    }

    // 更新表格模型
    QStringList headers = sensor->headerLabels();
    m_tableModel->loadData(data, headers);

    // 更新传感器信息标签
    m_labSensorInfo->setText(
        QString("当前传感器：%1 | 型号：%2 | 类型：%3 | 采集频率：每 %4 秒 | 数据条数：%5")
            .arg(sensor->name)
            .arg(sensor->model)
            .arg(sensor->sensorType())
            .arg(sensor->frequency)
            .arg(data.size()));

    delete sensor;
}

void HistoryDataDialog::slotMonitorChanged(int index)
{
    if (index < 0)
        return;

    QString pointId = m_cbxMonitorPoint->itemData(index).toString();
    QString sensorModel = getBindSensorByPointId(pointId);

    if (sensorModel.isEmpty()) {
        m_labSensorInfo->setText(
            QString("监测点【%1】未绑定传感器，无法查询历史数据").arg(pointId));
        m_labSensorInfo->setStyleSheet("color: #c0392b; font-weight: bold; padding: 4px;");
        m_btnQuery->setEnabled(false);
    } else {
        // 只加载轻量信息显示
        Sensor *sensor = loadSensorByModel(sensorModel);
        if (sensor) {
            m_labSensorInfo->setText(
                QString("监测点【%1】已绑定传感器：%2 | 型号：%3 | 类型：%4 | 采集频率：每 %5 秒")
                    .arg(pointId)
                    .arg(sensor->name)
                    .arg(sensor->model)
                    .arg(sensor->sensorType())
                    .arg(sensor->frequency));
            m_labSensorInfo->setStyleSheet(
                "color: #27ae60; font-weight: bold; padding: 4px;");
            m_btnQuery->setEnabled(true);
            delete sensor;
        } else {
            m_labSensorInfo->setText(
                QString("监测点【%1】绑定传感器型号【%2】，但传感器信息未找到")
                    .arg(pointId, sensorModel));
            m_labSensorInfo->setStyleSheet(
                "color: #e67e22; font-weight: bold; padding: 4px;");
            m_btnQuery->setEnabled(false);
        }
    }
}
