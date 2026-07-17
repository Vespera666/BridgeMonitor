#include "MonitorPointManageDialog.h"

MonitorPointManageDialog::MonitorPointManageDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("监测点管理");
    resize(960, 640);
    initUI();
    initFile();
    refreshTable();
    refreshBindCombobox();
}

void MonitorPointManageDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // 顶部按钮栏
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("新增监测点");
    QPushButton *btnEdit = new QPushButton("修改选中");
    QPushButton *btnDel = new QPushButton("删除(输入编号)");
    QPushButton *btnRefresh = new QPushButton("刷新表格");
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnEdit);
    btnLayout->addWidget(btnDel);
    btnLayout->addWidget(btnRefresh);
    btnLayout->addStretch();

    // 表格视图
    m_tableModel = new MonitorPointTableModel(this);
    m_tableView = new QTableView();
    m_tableView->setModel(m_tableModel);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 底部【监测点-传感器绑定区域】
    QGroupBox *groupBind = new QGroupBox("监测点 ↔ 传感器绑定（一个监测点仅允许绑定1个传感器）");
    QHBoxLayout *bindLayout = new QHBoxLayout(groupBind);

    QLabel *labPoint = new QLabel("监测点：");
    m_cbxPointSelect = new QComboBox();
    QLabel *labSensor = new QLabel("传感器：");
    m_cbxSensorSelect = new QComboBox();
    m_btnBind = new QPushButton("建立绑定");
    m_btnUnBind = new QPushButton("解除绑定");

    bindLayout->addWidget(labPoint);
    bindLayout->addWidget(m_cbxPointSelect);
    bindLayout->addSpacing(10);
    bindLayout->addWidget(labSensor);
    bindLayout->addWidget(m_cbxSensorSelect);
    bindLayout->addSpacing(20);
    bindLayout->addWidget(m_btnBind);
    bindLayout->addWidget(m_btnUnBind);
    bindLayout->addStretch();
    groupBind->setMaximumHeight(90);

    // 组装布局
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(m_tableView);
    mainLayout->addWidget(groupBind);

    // 信号槽
    connect(btnAdd, &QPushButton::clicked, this, &MonitorPointManageDialog::slotAddPoint);
    connect(btnEdit, &QPushButton::clicked, this, &MonitorPointManageDialog::slotEditPoint);
    connect(btnDel, &QPushButton::clicked, this, &MonitorPointManageDialog::slotDelPoint);
    connect(btnRefresh, &QPushButton::clicked, this, &MonitorPointManageDialog::slotRefresh);
    connect(m_btnBind, &QPushButton::clicked, this, &MonitorPointManageDialog::slotDoBind);
    connect(m_btnUnBind, &QPushButton::clicked, this, &MonitorPointManageDialog::slotDoUnBind);
    connect(m_cbxPointSelect,
            &QComboBox::currentTextChanged,
            this,
            &MonitorPointManageDialog::slotPointComboboxChanged);
}

void MonitorPointManageDialog::initFile()
{
    QFile monF(MONITOR_FILE);
    if (!monF.exists()) {
        if (!monF.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "创建监测点文件失败！";
            return;
        }
        QTextStream st(&monF);
        st << "监测点编号,断面名称,安装日期\n";
        monF.close();
    }
}

void MonitorPointManageDialog::clearTable()
{
    QVector<MonitoringPoint> empty;
    QStringList emptyBind;
    m_tableModel->loadData(empty, emptyBind);
}

void MonitorPointManageDialog::refreshTable()
{
    clearTable();
    QVector<MonitoringPoint> allPoints = loadAllMonitorPoints();
    QStringList bindList;
    for (auto &mp : allPoints) {
        QString senModel = getBindSensorByPointId(mp.pointId);
        bindList.append(senModel.isEmpty() ? "未绑定" : senModel);
    }
    m_tableModel->loadData(allPoints, bindList);
}

void MonitorPointManageDialog::refreshBindCombobox()
{
    m_cbxPointSelect->clear();
    m_cbxSensorSelect->clear();

    QVector<MonitoringPoint> points = loadAllMonitorPoints();
    for (auto &p : points)
        m_cbxPointSelect->addItem(p.pointId);

    QStringList sensors = loadAllSensorModel();
    for (auto &s : sensors)
        m_cbxSensorSelect->addItem(s);

    slotPointComboboxChanged(m_cbxPointSelect->currentText());
}

QVector<MonitoringPoint> MonitorPointManageDialog::loadAllMonitorPoints()
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

bool MonitorPointManageDialog::saveMonitorPoint(const MonitoringPoint &mp,
                                                bool isUpdate,
                                                const QString &oldPointId)
{
    QFile f(MONITOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "读取监测点文件失败";
        return false;
    }
    QString allText = f.readAll();
    f.close();
    QStringList lines = allText.split("\n", Qt::KeepEmptyParts);

    if (isUpdate && !oldPointId.isEmpty()) {
        QStringList newLines;
        for (QString l : lines) {
            QStringList col = l.split(",", Qt::KeepEmptyParts);
            if (col.size() >= 1 && col[0] == oldPointId)
                continue;
            newLines.append(l);
        }
        lines = newLines;
    }
    QString newLine = QString("%1,%2,%3,%4")
                          .arg(mp.pointId)
                          .arg(mp.sectionName)
                          .arg(mp.installDate.toString("yyyy-MM-dd"))
                          .arg(mp.dataType);
    for (int i = 0; i < lines.size(); i++)
        if (lines[i].trimmed().isEmpty())
            lines.removeAt(i--);
    lines.append(newLine);

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "写入监测点文件失败";
        return false;
    }
    QTextStream st(&f);
    st.setEncoding(QStringConverter::Utf8);
    st << lines.join("\n");
    f.close();
    return true;
}

bool MonitorPointManageDialog::deletePointById(const QString &pointId)
{
    QFile f(MONITOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString all = f.readAll();
    f.close();
    QStringList lines = all.split("\n");
    bool find = false;
    QStringList newLines;
    for (QString l : lines) {
        QStringList col = l.split(",", Qt::KeepEmptyParts);
        if (col.size() >= 1 && col[0] == pointId) {
            find = true;
            continue;
        }
        newLines.append(l);
    }
    if (!find)
        return false;

    // 删除监测点，同步解绑
    unbindPoint(pointId);

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream st(&f);
    st << newLines.join("\n");
    f.close();
    return true;
}

QString MonitorPointManageDialog::getBindSensorByPointId(const QString &pid)
{
    // 从 sensor_storage.txt 最后一列查找
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
        if (cols.size() >= 8 && cols[7] == pid) {
            f.close();
            return cols[2]; // 返回型号
        }
    }
    f.close();
    return "";
}

QStringList MonitorPointManageDialog::loadAllSensorModel()
{
    QStringList res;
    QFile f(SENSOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return res;
    QTextStream st(&f);
    bool skipHead = true;
    while (!st.atEnd()) {
        QString line = st.readLine().trimmed();
        if (line.isEmpty())
            continue;
        if (skipHead) {
            skipHead = false;
            continue;
        }
        QStringList col = line.split(",");
        if (col.size() >= 1)
            res.append(col[0]);
    }
    f.close();
    return res;
}

// ====================== 一对一绑定函数实现 ======================
bool MonitorPointManageDialog::isPointAlreadyBound(const QString &pointId)
{
    QFile f(SENSOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
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
            return true;
        }
    }
    f.close();
    return false;
}

bool MonitorPointManageDialog::unbindPoint(const QString &pointId)
{
    // 将 sensor_storage.txt 中该监测点对应记录的最后一列改回"未绑定"
    QFile f(SENSOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString allText = f.readAll();
    f.close();

    QStringList lines = allText.split("\n", Qt::KeepEmptyParts);
    QStringList newLines;
    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i];
        QStringList cols = line.split(",", Qt::KeepEmptyParts);
        if (i == 0 || cols.size() < 8) {
            newLines << line;
            continue;
        }
        if (cols[7] == pointId) {
            cols[7] = "未绑定";
            newLines << cols.join(",");
        } else {
            newLines << line;
        }
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream st(&f);
    st.setEncoding(QStringConverter::Utf8);
    st << newLines.join("\n");
    f.close();
    return true;
}

bool MonitorPointManageDialog::bindSensorToPoint(const QString &sensorId, const QString &pointId)
{
    // 先解绑
    unbindPoint(pointId);

    // 读写 sensor_storage.txt：找到该传感器型号未绑定的记录，修改最后一列
    QFile f(SENSOR_FILE);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString allText = f.readAll();
    f.close();

    QStringList lines = allText.split("\n", Qt::KeepEmptyParts);
    QStringList newLines;
    bool found = false;

    for (int i = 0; i < lines.size(); i++) {
        QString line = lines[i];
        QStringList cols = line.split(",", Qt::KeepEmptyParts);
        if (i == 0 || cols.size() < 8) {
            newLines << line;
            continue;
        }
        if (cols[2] == sensorId && !found && (cols[7].isEmpty() || cols[7] == "未绑定")) {
            cols[7] = pointId;
            newLines << cols.join(",");
            found = true;
        } else {
            newLines << line;
        }
    }

    if (!found) {
        // 所有该型号传感器都已绑定 → 追加新记录
        for (const QString &line : lines) {
            QStringList cols = line.split(",", Qt::KeepEmptyParts);
            if (cols.size() >= 8 && cols[2] == sensorId) {
                cols[7] = pointId;
                newLines << cols.join(",");
                break;
            }
        }
    }

    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream st(&f);
    st.setEncoding(QStringConverter::Utf8);
    st << newLines.join("\n");
    f.close();
    return true;
}

// ====================== 槽函数 ======================
void MonitorPointManageDialog::slotAddPoint()
{
    MonitorPointEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        MonitoringPoint mp = dlg.getPointData();
        saveMonitorPoint(mp, false, "");
        QMessageBox::information(this, "成功", "新增监测点完成");
        refreshTable();
        refreshBindCombobox();
    }
}

void MonitorPointManageDialog::slotEditPoint()
{
    QModelIndexList sel = m_tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选中一行监测点");
        return;
    }
    int row = sel.first().row();
    QString pid = m_tableModel->index(row, 0).data().toString();
    QString sec = m_tableModel->index(row, 1).data().toString();
    QString dateStr = m_tableModel->index(row, 2).data().toString();
    QDate installDate = QDate::fromString(dateStr, "yyyy-MM-dd");

    MonitoringPoint temp;
    temp.pointId = pid;
    temp.sectionName = sec;
    temp.installDate = installDate;

    MonitorPointEditDialog dlg(this);
    dlg.setEditTarget(temp);
    if (dlg.exec() == QDialog::Accepted) {
        MonitoringPoint newMp = dlg.getPointData();
        saveMonitorPoint(newMp, true, pid);
        refreshTable();
        refreshBindCombobox();
    }
}

void MonitorPointManageDialog::slotDelPoint()
{
    QString targetPid = QInputDialog::getText(this, "删除监测点", "输入要删除的监测点编号：");
    QString pid = targetPid.trimmed();
    if (pid.isEmpty())
        return;

    bool exist = deletePointById(pid);
    if (!exist) {
        QMessageBox::information(this, "提示", "未找到该监测点编号");
        return;
    }
    QMessageBox::StandardButton ret
        = QMessageBox::question(this,
                                "确认删除",
                                QString("确定删除监测点 %1，同时清除传感器绑定关系？").arg(pid),
                                QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        deletePointById(pid);
        refreshTable();
        refreshBindCombobox();
        QMessageBox::information(this, "完成", "删除成功");
    }
}

void MonitorPointManageDialog::slotRefresh()
{
    refreshTable();
    refreshBindCombobox();
    QMessageBox::information(this, "刷新", "监测点数据已同步");
}

void MonitorPointManageDialog::slotPointComboboxChanged(const QString &pointId)
{
    QString bindSensor = getBindSensorByPointId(pointId);
    if (!bindSensor.isEmpty())
        m_cbxSensorSelect->setCurrentText(bindSensor);
}

void MonitorPointManageDialog::slotDoBind()
{
    QString pointId = m_cbxPointSelect->currentText().trimmed();
    QString sensorId = m_cbxSensorSelect->currentText().trimmed();
    if (pointId.isEmpty() || sensorId.isEmpty()) {
        QMessageBox::warning(this, "提示", "监测点与传感器不能为空！");
        return;
    }

    if (isPointAlreadyBound(pointId)) {
        auto res = QMessageBox::question(this,
                                         "警告",
                                         QString("监测点【%1】已绑定传感器，是否替换原有绑定？")
                                             .arg(pointId),
                                         QMessageBox::Yes | QMessageBox::No);
        if (res != QMessageBox::Yes)
            return;
    }

    bool ok = bindSensorToPoint(sensorId, pointId);
    if (ok) {
        QMessageBox::information(this, "成功", "绑定完成");
        refreshTable();
    } else
        QMessageBox::critical(this, "失败", "绑定写入文件失败");
}

void MonitorPointManageDialog::slotDoUnBind()
{
    QString pointId = m_cbxPointSelect->currentText().trimmed();
    if (pointId.isEmpty())
        return;

    if (!isPointAlreadyBound(pointId)) {
        QMessageBox::information(this, "提示", "该监测点暂无绑定传感器");
        return;
    }
    auto res = QMessageBox::question(this,
                                     "确认解绑",
                                     QString("确认解除监测点【%1】的传感器绑定？").arg(pointId));
    if (res == QMessageBox::Yes) {
        unbindPoint(pointId);
        refreshTable();
        slotPointComboboxChanged(pointId);
        QMessageBox::information(this, "完成", "解绑成功");
    }
}
