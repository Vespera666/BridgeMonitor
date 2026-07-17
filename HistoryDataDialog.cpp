#include "HistoryDataDialog.h"
#include <QCoreApplication>
#include <QDir>
#include <QHeaderView>
#include <QPainter>
#include <QSet>
#include <algorithm>
#include <QtCharts/QChart>

// ── dataType → 字段元信息映射 ──
SensorFieldInfo HistoryDataDialog::fieldInfoForType(const QString &dataType)
{
    if (dataType == "风速风向传感器")
        return {{"风速", "风向"}, {"m/s", "°"}, 2};
    if (dataType == "温湿度监测传感器")
        return {{"温度", "湿度"}, {"°C", "%"}, 2};
    if (dataType == "索力监测传感器")
        return {{"索力值"}, {"kN"}, 1};
    if (dataType == "挠度传感器")
        return {{"挠度"}, {"mm"}, 1};
    if (dataType == "振动监测传感器")
        return {{"振动频率"}, {"Hz"}, 1};
    if (dataType == "支座位移传感器")
        return {{"位移量"}, {"mm"}, 1};
    if (dataType == "伸缩缝监测传感器")
        return {{"伸缩量"}, {"mm"}, 1};
    return {{"数值"}, {""}, 1};
}

// ── 辅助：拼接表头 ──
static QStringList makeHeaders(const QStringList &names, const QStringList &units)
{
    QStringList h;
    h << "时间";
    for (int i = 0; i < names.size(); i++)
        h << names[i] + "(" + units[i] + ")";
    return h;
}

HistoryDataDialog::HistoryDataDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("历史数据展示");
    resize(1200, 850);
    initUI();
}

void HistoryDataDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    m_tabWidget = new QTabWidget();
    m_tabs.resize(SENSOR_TYPES.size());

    for (int i = 0; i < SENSOR_TYPES.size(); i++) {
        createTabPage(i);
        m_tabWidget->addTab(m_tabs[i].widget, SENSOR_LABELS[i]);
    }

    mainLayout->addWidget(m_tabWidget, 1);
}

void HistoryDataDialog::createTabPage(int index)
{
    HistoryTabPage &tab = m_tabs[index];
    const QString &sensorType = SENSOR_TYPES[index];

    // ── 容器 ──
    tab.widget = new QWidget();
    QVBoxLayout *pageLayout = new QVBoxLayout(tab.widget);
    pageLayout->setSpacing(8);
    pageLayout->setContentsMargins(8, 8, 8, 8);

    // ── 顶部筛选栏 ──
    QGroupBox *filterGroup = new QGroupBox("查询条件");
    QHBoxLayout *filterLayout = new QHBoxLayout(filterGroup);

    filterLayout->addWidget(new QLabel("监测点："));
    tab.cbxPoint = new QComboBox();
    tab.cbxPoint->setMinimumWidth(200);
    filterLayout->addWidget(tab.cbxPoint);

    filterLayout->addSpacing(15);
    filterLayout->addWidget(new QLabel("起始日期："));
    tab.deStart = new QDateEdit(QDate::currentDate().addDays(-7));
    tab.deStart->setDisplayFormat("yyyy-MM-dd");
    tab.deStart->setCalendarPopup(true);
    filterLayout->addWidget(tab.deStart);

    filterLayout->addSpacing(10);
    filterLayout->addWidget(new QLabel("截止日期："));
    tab.deEnd = new QDateEdit(QDate::currentDate());
    tab.deEnd->setDisplayFormat("yyyy-MM-dd");
    tab.deEnd->setCalendarPopup(true);
    filterLayout->addWidget(tab.deEnd);

    filterLayout->addSpacing(20);
    tab.btnQuery = new QPushButton("查询历史数据");
    filterLayout->addWidget(tab.btnQuery);
    filterLayout->addStretch();

    filterGroup->setMaximumHeight(80);

    // ── 传感器信息标签 ──
    tab.labInfo = new QLabel("请选择一个监测点");
    tab.labInfo->setStyleSheet("color: #555; font-weight: bold; padding: 4px;");

    // ── 表格 ──
    tab.tableModel = new DataTableModel(tab.widget);
    tab.tableView = new QTableView();
    tab.tableView->setModel(tab.tableModel);
    tab.tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tab.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tab.tableView->horizontalHeader()->setStretchLastSection(true);
    tab.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // ── 图表（空占位） ──
    tab.chartView = new QChartView();
    tab.chartView->setRenderHint(QPainter::Antialiasing);
    {
        auto *emptyChart = new QChart();
        emptyChart->setTitle("查询数据后此处显示折线图");
        tab.chartView->setChart(emptyChart);
    }

    // ── 上下分割器 ──
    tab.splitter = new QSplitter(Qt::Vertical);
    tab.splitter->addWidget(tab.tableView);
    tab.splitter->addWidget(tab.chartView);
    tab.splitter->setStretchFactor(0, 2);
    tab.splitter->setStretchFactor(1, 3);

    // ── 组装 ──
    pageLayout->addWidget(filterGroup);
    pageLayout->addWidget(tab.labInfo);
    pageLayout->addWidget(tab.splitter, 1);

    // ── 填充监测点下拉框：按 dataType 筛选 ──
    QVector<MonitoringPoint> allPoints = loadAllMonitorPoints();
    for (const MonitoringPoint &mp : allPoints) {
        if (mp.dataType == sensorType)
            tab.cbxPoint->addItem(
                QString("%1 - %2").arg(mp.pointId, mp.sectionName), mp.pointId);
    }

    // ── 信号连接 ──
    connect(tab.btnQuery, &QPushButton::clicked, this, [this, index]() {
        slotQuery(index);
    });
    connect(tab.cbxPoint,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this, index](int) { slotMonitorChanged(index); });

    slotMonitorChanged(index);
}

// ── 读取 monitor_storage.txt（4列：编号,断面,日期,传感器类型） ──
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
        mp.dataType = (cols.size() >= 4) ? cols[3] : QString();
        mp.sensor = nullptr;
        res.append(mp);
    }
    f.close();
    return res;
}

// ── 查找 CSV 文件路径 ──
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
    return {};
}

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

// ── 加载监测点数据（不依赖传感器绑定） ──
QVector<DataPoint> HistoryDataDialog::loadDataForPoint(const QString &pointId,
                                                       const QString &dataType,
                                                       const QDateTime &start,
                                                       const QDateTime &end)
{
    SensorFieldInfo info = fieldInfoForType(dataType);

    // ── 单字段：直接读 {pointId}.csv ──
    if (info.fieldCount == 1) {
        QString csvPath = findCsvPath(pointId);
        if (!csvPath.isEmpty()) {
            QVector<DataPoint> data;
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
                    dp.value.append(parts[1].toDouble());
                    data.append(dp);
                }
                file.close();
            }
            filterByDateRange(data, start, end);
            return data;
        }
    }

    // ── 双字段：合并 {pointId}-A.csv + {pointId}-B.csv ──
    if (info.fieldCount == 2) {
        QString pathA = findCsvPath(pointId + "-A");
        QString pathB = findCsvPath(pointId + "-B");

        if (!pathA.isEmpty() || !pathB.isEmpty()) {
            auto readToMap = [](const QString &path) -> QMap<qint64, double> {
                QMap<qint64, double> map;
                QFile f(path);
                if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
                    return map;
                QTextStream in(&f);
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
                    if (ts.isValid())
                        map[ts.toMSecsSinceEpoch()] = parts[1].toDouble();
                }
                f.close();
                return map;
            };

            QMap<qint64, double> mapA = pathA.isEmpty() ? QMap<qint64, double>() : readToMap(pathA);
            QMap<qint64, double> mapB = pathB.isEmpty() ? QMap<qint64, double>() : readToMap(pathB);

            QSet<qint64> allKeys;
            for (qint64 k : mapA.keys()) allKeys.insert(k);
            for (qint64 k : mapB.keys()) allKeys.insert(k);
            QList<qint64> sortedKeys = allKeys.values();
            std::sort(sortedKeys.begin(), sortedKeys.end());

            QVector<DataPoint> data;
            data.reserve(sortedKeys.size());
            for (qint64 key : sortedKeys) {
                DataPoint dp;
                dp.timeStamp = QDateTime::fromMSecsSinceEpoch(key);
                dp.value.append(mapA.value(key, 0.0));
                dp.value.append(mapB.value(key, 0.0));
                data.append(dp);
            }
            filterByDateRange(data, start, end);
            if (!data.isEmpty())
                return data;
        }
    }

    // ── 回退：生成模拟数据 ──
    qint64 totalSecs = start.secsTo(end);
    if (totalSecs <= 0)
        return {};

    int interval = 60;
    int count = static_cast<int>(totalSecs / interval);
    if (count > 2000)
        count = 2000;

    QVector<DataPoint> mockData;
    for (int i = 0; i < count; i++) {
        DataPoint dp;
        dp.timeStamp = start.addSecs(i * interval);
        for (int j = 0; j < info.fieldCount; j++)
            dp.value.append(0.0);
        mockData.append(dp);
    }
    return mockData;
}

// ── 静态图表更新 ──
void HistoryDataDialog::updateChartForView(QChartView *chartView,
                                           const QVector<DataPoint> &data,
                                           const QStringList &fieldNames)
{
    QChart *oldChart = chartView->chart();
    if (oldChart)
        oldChart->deleteLater();

    auto *chart = new QChart();
    chart->setTitle("历史数据趋势");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    if (data.isEmpty()) {
        chartView->setChart(chart);
        return;
    }

    auto *axisX = new QDateTimeAxis();
    axisX->setFormat("MM-dd\nHH:mm");
    axisX->setRange(data.first().timeStamp, data.last().timeStamp);
    chart->addAxis(axisX, Qt::AlignBottom);

    auto *axisY = new QValueAxis();
    axisY->setLabelFormat("%.1f");
    chart->addAxis(axisY, Qt::AlignLeft);

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

    chartView->setChart(chart);
}

// ── 查询 ──
void HistoryDataDialog::slotQuery(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= m_tabs.size())
        return;

    HistoryTabPage &tab = m_tabs[tabIndex];
    QString pointId = tab.cbxPoint->currentData().toString();
    if (pointId.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择监测点");
        return;
    }

    QDateTime start(tab.deStart->date(), QTime(0, 0, 0));
    QDateTime end(tab.deEnd->date(), QTime(23, 59, 59));

    if (start > end) {
        QMessageBox::warning(this, "提示", "起始日期不能晚于截止日期");
        return;
    }

    // 从当前 Tab 的传感器类型确定字段信息
    QString dataType = SENSOR_TYPES[tabIndex];
    SensorFieldInfo info = fieldInfoForType(dataType);

    // 加载数据（不依赖传感器绑定）
    QString csvPath = findCsvPath(pointId);
    QVector<DataPoint> data = loadDataForPoint(pointId, dataType, start, end);
    QString sourceTag = csvPath.isEmpty() ? "模拟数据" : "真实数据";

    // 更新表格
    QStringList headers = makeHeaders(info.fieldNames, info.fieldUnits);
    tab.tableModel->loadData(data, headers);

    // 更新图表
    updateChartForView(tab.chartView, data, info.fieldNames);

    // 更新信息标签
    tab.labInfo->setText(
        QString("监测点【%1】| 数据类型：%2 | 数据条数：%3 | 来源：%4")
            .arg(pointId)
            .arg(dataType)
            .arg(data.size())
            .arg(sourceTag));
}

// ── 监测点切换 ──
void HistoryDataDialog::slotMonitorChanged(int tabIndex)
{
    if (tabIndex < 0 || tabIndex >= m_tabs.size())
        return;

    HistoryTabPage &tab = m_tabs[tabIndex];
    if (tab.cbxPoint->currentIndex() < 0)
        return;

    QString pointId = tab.cbxPoint->currentData().toString();
    bool hasCsv = !findCsvPath(pointId).isEmpty();

    // 也检查双字段传感器（FS/WSD）的 -A 文件
    if (!hasCsv)
        hasCsv = !findCsvPath(pointId + "-A").isEmpty();

    if (hasCsv) {
        tab.labInfo->setText(
            QString("监测点【%1】已有真实数据 ✓ | 点击查询按钮查看历史趋势").arg(pointId));
        tab.labInfo->setStyleSheet("color: #27ae60; font-weight: bold; padding: 4px;");
        tab.btnQuery->setEnabled(true);
    } else {
        tab.labInfo->setText(
            QString("监测点【%1】无数据文件，查询时将使用模拟数据").arg(pointId));
        tab.labInfo->setStyleSheet("color: #e67e22; font-weight: bold; padding: 4px;");
        tab.btnQuery->setEnabled(true);
    }
}
