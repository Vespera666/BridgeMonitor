#include "HistoryDataDialog.h"
#include "CableForceSensor.h"
#include "VibrationSensor.h"
#include "deflection.h"
#include "displacement.h"
#include "expansionjointsensor.h"
#include "temperaturehumiditysensor.h"
#include "windsensor.h"
#include <QCoreApplication>
#include <QDir>
#include <QPainter>
#include <QtCharts/QChart>

HistoryDataDialog::HistoryDataDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("历史数据展示");
    resize(1200, 850);
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

    // 图表视图
    m_chartView = new QChartView();
    m_chartView->setRenderHint(QPainter::Antialiasing);
    // 空状态占位
    {
        auto *emptyChart = new QChart();
        emptyChart->setTitle("查询数据后此处显示折线图");
        m_chartView->setChart(emptyChart);
    }

    // 上下分割：表格 + 图表
    m_splitter = new QSplitter(Qt::Vertical);
    m_splitter->addWidget(m_tableView);
    m_splitter->addWidget(m_chartView);
    m_splitter->setStretchFactor(0, 2);  // 表格占 2
    m_splitter->setStretchFactor(1, 3);  // 图表占 3

    // 组装布局
    mainLayout->addWidget(groupFilter);
    mainLayout->addWidget(m_labSensorInfo);
    mainLayout->addWidget(m_splitter, 1);  // splitter 拉伸填充剩余空间

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

// ── 按监测点编号查找 CSV 文件路径 ──
// 依次搜索以下目录（开发/部署兼容）：
//   1. exe目录/data/         — 部署时
//   2. exe目录/../data/      — Qt Creator 运行目录
//   3. exe目录/../../data/   — Qt Creator 嵌套构建目录
QString HistoryDataDialog::findCsvPath(const QString &pointId) const
{
    const QString fileName = pointId + ".csv";
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList searchDirs = {
        appDir + "/data/",
        appDir + "/../data/",
        appDir + "/../../data/",
    };
    for (const QString &dir : searchDirs) {
        QString fullPath = QDir(dir).filePath(fileName);
        if (QFile::exists(fullPath))
            return fullPath;
    }
    return {}; // 没找到
}

// ── 按日期范围过滤 ──
void HistoryDataDialog::filterByDateRange(QVector<DataPoint> &data,
                                          const QDateTime &start,
                                          const QDateTime &end) const
{
    QVector<DataPoint> filtered;
    filtered.reserve(data.size());
    for (const DataPoint &dp : data) {
        if (dp.timeStamp >= start && dp.timeStamp <= end)
            filtered.append(dp);
    }
    data = filtered;
}

// ── 加载真实 CSV 或回退到模拟数据 ──
QVector<DataPoint> HistoryDataDialog::loadRealOrMockData(Sensor *sensor,
                                                         const QString &pointId,
                                                         const QDateTime &start,
                                                         const QDateTime &end)
{
    if (!sensor)
        return {};

    // 尝试从预处理后的 CSV 加载真实数据
    QString csvPath = findCsvPath(pointId);
    if (!csvPath.isEmpty()) {
        QVector<DataPoint> data;

        // 判断通道名结尾：-A 或 -B 表示该通道只含传感器部分字段
        int fieldCount = sensor->fieldNames().size();
        int valueIndex = 0;         // 通道值对应传感器 fieldNames 的索引
        if (pointId.endsWith("-A"))
            valueIndex = 0;
        else if (pointId.endsWith("-B"))
            valueIndex = 1;

        // 单字段传感器 → 直接用 FileManager::readCsv
        if (fieldCount == 1 && valueIndex == 0) {
            data = sensor->loadFile(csvPath);
            filterByDateRange(data, start, end);
            return data;
        }

        // 多字段传感器的 A/B 拆分 CSV → 自行解析（避免列数校验失败）
        QFile file(csvPath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            int lineNo = 0;
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                lineNo++;
                if (line.isEmpty() || line.startsWith('#') || lineNo == 1)
                    continue;
                QStringList parts = line.split(",");
                if (parts.size() < 2)
                    continue;
                QDateTime ts = QDateTime::fromString(parts[0], "yyyy-MM-dd HH:mm:ss");
                if (!ts.isValid())
                    ts = QDateTime::fromString(parts[0], "yyyy/MM/dd HH:mm:ss");
                DataPoint dp;
                dp.timeStamp = ts;
                // 构造完整的 value 向量，未提供字段填 0
                for (int i = 0; i < fieldCount; i++)
                    dp.value.append(i == valueIndex ? parts[1].toDouble() : 0.0);
                data.append(dp);
            }
            file.close();
        }
        filterByDateRange(data, start, end);
        return data;
    }

    // 回退：生成模拟数据
    qint64 totalSecs = start.secsTo(end);
    if (totalSecs <= 0)
        return {};

    int interval = sensor->frequency > 0 ? sensor->frequency : 1;
    int count = static_cast<int>(totalSecs / interval);
    if (count > 2000)
        count = 2000;

    QVector<DataPoint> mockData = sensor->generateMockData(count);
    for (int i = 0; i < mockData.size(); i++)
        mockData[i].timeStamp = start.addSecs(i * interval);

    filterByDateRange(mockData, start, end);
    return mockData;
}

// ── 绘制折线图 ──
void HistoryDataDialog::updateChart(const QVector<DataPoint> &data,
                                    const QStringList &fieldNames)
{
    // 清除旧图表
    QChart *oldChart = m_chartView->chart();
    if (oldChart)
        oldChart->deleteLater();

    auto *chart = new QChart();
    chart->setTitle("历史数据趋势");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    if (data.isEmpty()) {
        m_chartView->setChart(chart);
        return;
    }

    // X 轴：时间
    auto *axisX = new QDateTimeAxis();
    axisX->setFormat("MM-dd\nHH:mm");
    axisX->setRange(data.first().timeStamp, data.last().timeStamp);
    chart->addAxis(axisX, Qt::AlignBottom);

    // Y 轴：自动范围
    auto *axisY = new QValueAxis();
    axisY->setLabelFormat("%.1f");
    chart->addAxis(axisY, Qt::AlignLeft);

    // 每个字段一条折线
    const QList<QColor> colors = {
        QColor("#e74c3c"), QColor("#2980b9"), QColor("#27ae60"),
        QColor("#8e44ad"), QColor("#e67e22"), QColor("#1abc9c"),
    };

    for (int fi = 0; fi < fieldNames.size(); fi++) {
        auto *series = new QLineSeries();
        series->setName(fieldNames[fi]);
        QPen pen(colors[fi % colors.size()]);
        pen.setWidth(2);
        series->setPen(pen);

        for (const DataPoint &dp : data) {
            if (fi < dp.value.size())
                series->append(dp.timeStamp.toMSecsSinceEpoch(), dp.value[fi]);
        }

        chart->addSeries(series);
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    m_chartView->setChart(chart);
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

    // 加载数据（优先真实 CSV，回退模拟）
    QString csvPath = findCsvPath(pointId);
    QVector<DataPoint> data = loadRealOrMockData(sensor, pointId, start, end);
    QString sourceTag = csvPath.isEmpty() ? "模拟数据" : "真实数据";
    if (data.isEmpty()) {
        QMessageBox::information(this, "提示",
            QString("所选时间范围内无数据\n数据来源: %1").arg(sourceTag));
    }

    // 更新表格模型
    QStringList headers = sensor->headerLabels();
    m_tableModel->loadData(data, headers);

    // 取出传感器信息（delete 前）
    QStringList fieldNames = sensor->fieldNames();
    QString sName = sensor->name;
    QString sModel = sensor->model;
    QString sType = sensor->sensorType();
    int sFreq = sensor->frequency;
    delete sensor;

    // 更新图表
    updateChart(data, fieldNames);

    // 更新传感器信息标签
    m_labSensorInfo->setText(
        QString("当前传感器：%1 | 型号：%2 | 类型：%3 | 采集频率：每 %4 秒 | 数据条数：%5 | 来源：%6")
            .arg(sName)
            .arg(sModel)
            .arg(sType)
            .arg(sFreq)
            .arg(data.size())
            .arg(sourceTag));
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
            bool hasCsv = !findCsvPath(pointId).isEmpty();
            m_labSensorInfo->setText(
                QString("监测点【%1】已绑定传感器：%2 | 型号：%3 | 类型：%4 | 采集频率：每 %5 秒 | %6")
                    .arg(pointId)
                    .arg(sensor->name)
                    .arg(sensor->model)
                    .arg(sensor->sensorType())
                    .arg(sensor->frequency)
                    .arg(hasCsv ? "已有真实数据 ✓" : "无数据文件，将使用模拟数据"));
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
