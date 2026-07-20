#include "RealTimeDataDialog.h"
#include "HistoryDataDialog.h"
#include "StyleConstants.h"
#include <QCoreApplication>
#include <QDir>
#include <QPainter>
#include <QSet>
#include <algorithm>
#include <QtCharts/QChart>
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
    resize(1200, 650);
    initUI();
    refreshTable();
}

void RealTimeDataDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    m_splitter = new QSplitter(Qt::Horizontal);

    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(6);

    m_labTableHint = new QLabel("表格展示所有已绑定传感器的监测点最新数据");
    StyleConstants::applyCssClass(m_labTableHint, StyleConstants::kCssInfo);

    m_tableModel = new RealTimeDataModel(this);
    m_tableView = new QTableView();
    m_tableView->setModel(m_tableModel);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    m_btnRefresh = new QPushButton("刷新数据");
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->addStretch();
    btnRow->addWidget(m_btnRefresh);

    leftLayout->addWidget(m_labTableHint);
    leftLayout->addWidget(m_tableView, 1);
    leftLayout->addLayout(btnRow);

    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(6);

    QHBoxLayout *chartCtrlRow = new QHBoxLayout();
    m_labChartHint = new QLabel("请在左侧表格中选中一个监测点以查看波形图");
    StyleConstants::applyCssClass(m_labChartHint, StyleConstants::kCssInfo);
    chartCtrlRow->addWidget(m_labChartHint);
    chartCtrlRow->addStretch();
    chartCtrlRow->addWidget(new QLabel("展示条数："));
    m_spinCount = new QSpinBox();
    m_spinCount->setRange(10, 10000);
    m_spinCount->setValue(100);
    m_spinCount->setSingleStep(50);
    m_spinCount->setSuffix(" 条");
    chartCtrlRow->addWidget(m_spinCount);

    m_chartView = new QChartView();
    m_chartView->setRenderHint(QPainter::Antialiasing);
    {
        auto *emptyChart = new QChart();
        emptyChart->setTitle("选中监测点后此处显示实时波形图");
        emptyChart->setBackgroundBrush(QBrush(QColor("#FFFFFF")));
        m_chartView->setChart(emptyChart);
        m_chartView->setBackgroundBrush(QBrush(QColor("#F5F6FA")));
    }

    rightLayout->addLayout(chartCtrlRow);
    rightLayout->addWidget(m_chartView, 1);

    m_splitter->addWidget(leftPanel);
    m_splitter->addWidget(rightPanel);
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 2);

    mainLayout->addWidget(m_splitter, 1);

    connect(m_btnRefresh, &QPushButton::clicked, this, &RealTimeDataDialog::slotRefresh);
    connect(m_spinCount,
            QOverload<int>::of(&QSpinBox::valueChanged),
            this,
            &RealTimeDataDialog::slotCountChanged);
}

QString RealTimeDataDialog::getBoundSensorModelByPoint(const QString &pointId)
{
    QFile f(SENSOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return "";
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
        if (cols.size() >= 8 && cols[7] == pointId) {
            f.close();
            return cols[2]; // 型号
        }
    }
    f.close();
    return "";
}

QStringList RealTimeDataDialog::getAllBoundPointIds()
{
    QStringList ids;
    QFile f(SENSOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return ids;
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
        if (cols.size() >= 8 && cols[7] != "未绑定" && !cols[7].isEmpty())
            ids.append(cols[7]);
    }
    f.close();
    return ids;
}

void RealTimeDataDialog::refreshTable()
{
    QVector<MonitoringPoint> emptyPoints;
    QVector<SensorMeta> emptyMetas;
    QVector<DataPoint> emptyData;
    m_tableModel->loadData(emptyPoints, emptyMetas, emptyData);

    QVector<MonitoringPoint> allPoints = loadAllMonitorPoints();
    QVector<Sensor *> allSensors = loadAllSensors();

    QStringList boundIds = getAllBoundPointIds();

    QVector<MonitoringPoint> boundPoints;
    QVector<SensorMeta> boundSensorMetas;
    QVector<DataPoint> latestDataList;

    for (const MonitoringPoint &mp : allPoints) {
        if (!boundIds.contains(mp.pointId))
            continue;

        QString sensorModel = getBoundSensorModelByPoint(mp.pointId);
        Sensor *s = findSensorByModel(sensorModel, allSensors);
        if (!s)
            continue;

        boundPoints.append(mp);
        boundSensorMetas.append(extractSensorMeta(s));

        auto readLatestFromCsv = [](const QString &path) -> DataPoint {
            DataPoint dp;
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return dp;
            QTextStream in(&file);
            in.setEncoding(QStringConverter::Utf8);
            QString lastLine;
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty() && !line.startsWith('#') && !line.startsWith("时间"))
                    lastLine = line;
            }
            file.close();
            if (!lastLine.isEmpty()) {
                QStringList parts = lastLine.split(",");
                if (parts.size() >= 2) {
                    dp.timeStamp = QDateTime::fromString(parts[0], "yyyy-MM-dd HH:mm:ss");
                    if (!dp.timeStamp.isValid())
                        dp.timeStamp = QDateTime::fromString(parts[0], "yyyy/MM/dd HH:mm:ss");
                    for (int i = 1; i < parts.size(); i++)
                        dp.value.append(parts[i].toDouble());
                }
            }
            return dp;
        };

        QString csvPath = findCsvPath(mp.pointId);
        QString pathA = findCsvPath(mp.pointId + "-A");
        QString pathB = findCsvPath(mp.pointId + "-B");

        if (!csvPath.isEmpty()) {
            DataPoint dp = readLatestFromCsv(csvPath);
            latestDataList.append(dp);
        } else if (!pathA.isEmpty() || !pathB.isEmpty()) {
            DataPoint dpA = pathA.isEmpty() ? DataPoint() : readLatestFromCsv(pathA);
            DataPoint dpB = pathB.isEmpty() ? DataPoint() : readLatestFromCsv(pathB);
            DataPoint merged;
            merged.timeStamp = dpA.timeStamp.isValid() ? dpA.timeStamp : dpB.timeStamp;
            merged.value.append(dpA.value.isEmpty() ? 0.0 : dpA.value[0]);
            merged.value.append(dpB.value.isEmpty() ? 0.0 : dpB.value[0]);
            latestDataList.append(merged);
        } else {
            QVector<DataPoint> mock = s->generateMockData(1);
            latestDataList.append(mock.isEmpty() ? DataPoint() : mock.first());
        }
    }

    m_tableModel->loadData(boundPoints, boundSensorMetas, latestDataList);

    m_labTableHint->setText(
        QString("已绑定监测点：%1 个 | 点击某行可在右侧查看波形")
            .arg(boundPoints.size()));

    if (m_tableView->selectionModel()) {
        connect(m_tableView->selectionModel(),
                &QItemSelectionModel::currentRowChanged,
                this,
                &RealTimeDataDialog::slotRowSelected,
                Qt::UniqueConnection);
    }

    qDeleteAll(allSensors);
    allSensors.clear();
}

void RealTimeDataDialog::slotRowSelected(const QModelIndex &current,
                                         const QModelIndex & /*previous*/)
{
    if (!current.isValid())
        return;

    int row = current.row();
    QString pointId = m_tableModel->pointIdAt(row);
    if (pointId.isEmpty())
        return;

    m_labChartHint->setText(QString("当前监测点：%1").arg(pointId));
    updateRealtimeChart(pointId, m_spinCount->value());
}

void RealTimeDataDialog::slotCountChanged(int count)
{
    QModelIndex idx = m_tableView->selectionModel()->currentIndex();
    if (!idx.isValid())
        return;
    QString pointId = m_tableModel->pointIdAt(idx.row());
    if (!pointId.isEmpty())
        updateRealtimeChart(pointId, count);
}

void RealTimeDataDialog::slotRefresh()
{
    refreshTable();
    QModelIndex idx = m_tableView->selectionModel()->currentIndex();
    if (idx.isValid()) {
        QString pointId = m_tableModel->pointIdAt(idx.row());
        if (!pointId.isEmpty())
            updateRealtimeChart(pointId, m_spinCount->value());
    }
}

void RealTimeDataDialog::updateRealtimeChart(const QString &pointId, int count)
{
    QChart *oldChart = m_chartView->chart();
    if (oldChart)
        oldChart->deleteLater();

    auto *chart = new QChart();
    chart->setTitle(QString("监测点 %1 — 最近 %2 条数据").arg(pointId).arg(count));
    chart->setBackgroundBrush(QBrush(QColor("#FFFFFF")));
    m_chartView->setBackgroundBrush(QBrush(QColor("#F5F6FA")));

    QString sensorModel = getBoundSensorModelByPoint(pointId);
    QStringList fieldNames;

    if (!sensorModel.isEmpty()) {
        QVector<Sensor *> allSensors = loadAllSensors();
        Sensor *s = findSensorByModel(sensorModel, allSensors);
        if (s)
            fieldNames = s->fieldNames();
        qDeleteAll(allSensors);
    }

    if (fieldNames.isEmpty())
        fieldNames = {"数值"};

    QVector<DataPoint> allData;

    QString csvPath = findCsvPath(pointId);
    QString pathA = findCsvPath(pointId + "-A");
    QString pathB = findCsvPath(pointId + "-B");

    bool hasSplit = (!pathA.isEmpty() || !pathB.isEmpty());

    if (!csvPath.isEmpty() && !hasSplit) {
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
                for (int i = 1; i < parts.size(); i++)
                    dp.value.append(parts[i].toDouble());
                allData.append(dp);
            }
            file.close();
        }
    } else if (hasSplit) {
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

        allData.reserve(sortedKeys.size());
        for (qint64 key : sortedKeys) {
            DataPoint dp;
            dp.timeStamp = QDateTime::fromMSecsSinceEpoch(key);
            dp.value.append(mapA.value(key, 0.0));
            dp.value.append(mapB.value(key, 0.0));
            allData.append(dp);
        }
    } else {
        chart->setTitle(QString("监测点 %1 — 无历史数据文件").arg(pointId));
        m_chartView->setChart(chart);
        return;
    }

    if (allData.isEmpty()) {
        chart->setTitle(QString("监测点 %1 — CSV 文件无有效数据").arg(pointId));
        m_chartView->setChart(chart);
        return;
    }

    int takeCount = qMin(count, allData.size());
    QVector<DataPoint> data;
    data.reserve(takeCount);
    for (int i = allData.size() - takeCount; i < allData.size(); i++)
        data.append(allData[i]);

    // 绘图
    auto *axisX = new QDateTimeAxis();
    axisX->setFormat("MM-dd\nHH:mm");
    axisX->setRange(data.first().timeStamp, data.last().timeStamp);
    chart->addAxis(axisX, Qt::AlignBottom);

    auto *axisY = new QValueAxis();
    axisY->setLabelFormat("%.1f");
    axisY->setTitleText(fieldNames.size() >= 1 ? fieldNames[0] : "");
    chart->addAxis(axisY, Qt::AlignLeft);

    QValueAxis *axisY2 = nullptr;
    if (fieldNames.size() >= 2) {
        axisY2 = new QValueAxis();
        axisY2->setLabelFormat("%.1f");
        axisY2->setTitleText(fieldNames[1]);
        chart->addAxis(axisY2, Qt::AlignRight);
    }

    const auto &colors = StyleConstants::chartColors();

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
        series->attachAxis((fi == 0 || !axisY2) ? axisY : axisY2);
    }

    m_chartView->setChart(chart);
}

QString RealTimeDataDialog::findCsvPath(const QString &pointId) const
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
        mp.dataType = (cols.size() >= 4) ? cols[3] : QString();
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
