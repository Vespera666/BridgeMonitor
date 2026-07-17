#include "SensorManageDialog.h"
#include "CableForceSensor.h"
#include "VibrationSensor.h"
#include "deflection.h"
#include "displacement.h"
#include "expansionjointsensor.h"
#include "temperaturehumiditysensor.h"
#include "windsensor.h"
SensorManageDialog::SensorManageDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("传感器管理");
    resize(1000, 680);
    initUI();
    initFile();
    refreshTable();
    refreshCombo();
}
SensorManageDialog::~SensorManageDialog() {}
void SensorManageDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    // 顶部按钮栏
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnAdd = new QPushButton("新增传感器");
    QPushButton *btnEdit = new QPushButton("修改选中");
    QPushButton *btnDel = new QPushButton("删除(输入型号)");
    QPushButton *btnRefresh = new QPushButton("刷新表格");
    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnEdit);
    btnLayout->addWidget(btnDel);
    btnLayout->addWidget(btnRefresh);
    btnLayout->addStretch();
    // 表格
    m_tableModel = new QStandardItemModel(this);
    m_tableModel->setHorizontalHeaderLabels(m_sensorHeader);
    m_tableView = new QTableView();
    m_tableView->setModel(m_tableModel);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    // 底部绑定模块
    QGroupBox *groupBind = new QGroupBox("传感器-监测点绑定");
    QHBoxLayout *bindLayout = new QHBoxLayout(groupBind);
    bindLayout->addWidget(new QLabel("传感器："));
    m_cmbSensorSel = new QComboBox();
    bindLayout->addWidget(m_cmbSensorSel);
    bindLayout->addWidget(new QLabel("监测点："));
    m_cmbMonitorSel = new QComboBox();
    bindLayout->addWidget(m_cmbMonitorSel);
    QPushButton *btnBind = new QPushButton("绑定");
    QPushButton *btnUnbind = new QPushButton("解绑");
    bindLayout->addWidget(btnBind);
    bindLayout->addWidget(btnUnbind);
    groupBind->setMaximumHeight(90);
    // 组装布局
    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(m_tableView);
    mainLayout->addWidget(groupBind);
    // 信号绑定
    connect(btnAdd, &QPushButton::clicked, this, &SensorManageDialog::slotAdd);
    connect(btnEdit, &QPushButton::clicked, this, &SensorManageDialog::slotEdit);
    connect(btnDel, &QPushButton::clicked, this, &SensorManageDialog::slotDel);
    connect(btnRefresh, &QPushButton::clicked, this, &SensorManageDialog::slotRefresh);
    connect(btnBind, &QPushButton::clicked, this, &SensorManageDialog::bindPoint);
    connect(btnUnbind, &QPushButton::clicked, this, &SensorManageDialog::unBindPoint);
}
void SensorManageDialog::initFile()
{
    // 传感器文件
    QFile senF(SENSOR_FILE);
    if (!senF.exists()) {
        senF.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream st(&senF);
        st << m_sensorHeader.join(",") << "\n";
        senF.close();
    }
    // 监测点文件
    QFile monF(MONITOR_FILE);
    if (!monF.exists()) {
        monF.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream st(&monF);
        st << "监测点编号,断面名称,安装日期\n";
        monF.close();
    }
}
void SensorManageDialog::clearTable()
{
    m_tableModel->removeRows(0, m_tableModel->rowCount());
}
void SensorManageDialog::refreshTable()
{
    clearTable();
    QVector<Sensor *> allSen = loadAllSensor();
    for (Sensor *s : allSen) {
        QString bindId = getBindPoint(s->model);
        QList<QStandardItem *> row;
        row.append(new QStandardItem(s->name));
        row.append(new QStandardItem(s->size));
        row.append(new QStandardItem(s->model));
        row.append(new QStandardItem(s->manufacturer));
        row.append(new QStandardItem(s->generDate.toString("yyyy-MM-dd")));
        row.append(new QStandardItem(QString::number(s->frequency)));
        row.append(new QStandardItem(s->sensorType()));
        row.append(new QStandardItem(bindId.isEmpty() ? "未绑定" : bindId));
        m_tableModel->appendRow(row);
        delete s;
    }
    refreshCombo();
}
void SensorManageDialog::refreshCombo()
{
    m_cmbSensorSel->clear();
    m_cmbMonitorSel->clear();
    QVector<Sensor *> senList = loadAllSensor();
    for (Sensor *s : senList) {
        m_cmbSensorSel->addItem(QString("%1 | %2").arg(s->model).arg(s->name));
        delete s;
    }
    QVector<MonitoringPoint> mpList = loadAllMonitor();
    for (auto mp : mpList)
        m_cmbMonitorSel->addItem(mp.displayName());
}
QVector<Sensor *> SensorManageDialog::loadAllSensor()
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
bool SensorManageDialog::saveSensor(Sensor *s, bool isUpdate)
{
    QFile f(SENSOR_FILE);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QString allText = f.readAll();
    f.close();
    QStringList lines = allText.split("\n", Qt::KeepEmptyParts);
    QString targetModel = s->model.trimmed();
    if (isUpdate) {
        QStringList newLines;
        for (QString l : lines) {
            QStringList col = l.split(",");
            if (col.size() >= 3 && col[2] == targetModel)
                continue;
            newLines.append(l);
        }
        lines = newLines;
    }
    QString bindId = getBindPoint(targetModel);
    QString newLine = QString("%1,%2,%3,%4,%5,%6,%7,%8")
                          .arg(s->name)
                          .arg(s->size)
                          .arg(s->model)
                          .arg(s->manufacturer)
                          .arg(s->generDate.toString("yyyy-MM-dd"))
                          .arg(s->frequency)
                          .arg(s->sensorType())
                          .arg(bindId.isEmpty() ? "未绑定" : bindId);
    for (int i = 0; i < lines.size(); i++)
        if (lines[i].trimmed().isEmpty())
            lines.removeAt(i--);
    lines.append(newLine);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream st(&f);
    st.setEncoding(QStringConverter::Utf8);
    st << lines.join("\n");
    f.close();
    return true;
}
bool SensorManageDialog::delSensorByModel(const QString &model)
{
    QFile f(SENSOR_FILE);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QString all = f.readAll();
    f.close();
    QStringList lines = all.split("\n");
    bool find = false;
    QStringList newLines;
    for (QString l : lines) {
        QStringList col = l.split(",");
        if (col.size() >= 3 && col[2] == model) {
            find = true;
            continue;
        }
        newLines.append(l);
    }
    if (!find)
        return false;
    // 写回传感器文件（删除传感器即自动解绑）
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream st(&f);
    st << newLines.join("\n");
    f.close();
    return true;
}
QString SensorManageDialog::getBindPoint(const QString &sensorModel)
{
    // 从 sensor_storage.txt 最后一列读取绑定关系
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
        if (cols.size() >= 8 && cols[2] == sensorModel && cols[7] != "未绑定") {
            f.close();
            return cols[7];
        }
    }
    f.close();
    return "";
}
QVector<MonitoringPoint> SensorManageDialog::loadAllMonitor()
{
    QVector<MonitoringPoint> res;
    QFile f(MONITOR_FILE);
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
        QStringList col = line.split(",");
        if (col.size() < 3)
            continue;
        MonitoringPoint mp;
        mp.pointId = col[0];
        mp.sectionName = col[1];
        mp.installDate = QDate::fromString(col[2], "yyyy-MM-dd");
        mp.dataType = (col.size() >= 4) ? col[3] : QString();
        mp.sensor = nullptr;
        res.append(mp);
    }
    f.close();
    return res;
}
// 新增：判断监测点是否已绑定任意传感器
bool SensorManageDialog::isPointHasBind(const QString &monId)
{
    // 扫描 sensor_storage.txt 最后一列
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
        if (cols.size() >= 8 && cols[7] == monId) {
            f.close();
            return true;
        }
    }
    f.close();
    return false;
}
// 修改后的绑定函数：增加监测点绑定校验
void SensorManageDialog::bindPoint()
{
    int senIdx = m_cmbSensorSel->currentIndex();
    int monIdx = m_cmbMonitorSel->currentIndex();
    if (senIdx < 0 || monIdx < 0) {
        QMessageBox::warning(this, "提示", "请选择传感器和监测点");
        return;
    }
    QString senModel = m_cmbSensorSel->currentText().split(" | ").first();
    QString monId = m_cmbMonitorSel->currentText().split(" - ").last();

    // 核心校验：监测点已绑定则警告
    if (isPointHasBind(monId)) {
        QMessageBox::warning(
            this,
            "绑定失败",
            QString("监测点【%1】已绑定传感器，一个监测点仅允许绑定一个传感器，请先解绑后再操作！")
                .arg(monId));
        return;
    }

    // 读写 sensor_storage.txt：修改/追加传感器记录最后一列
    QFile f(SENSOR_FILE);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QString all = f.readAll();
    f.close();
    QStringList lines = all.split("\n", Qt::KeepEmptyParts);
    QStringList newLines;
    bool found = false; // 是否找到该型号且未绑定的记录

    for (int i = 0; i < lines.size(); i++) {
        QString l = lines[i];
        QStringList col = l.split(",", Qt::KeepEmptyParts);
        if (i == 0 || col.size() < 8) {
            // 表头或无效行
            newLines.append(l);
            continue;
        }
        if (col[2] == senModel && !found && (col.size() < 8 || col[7].isEmpty() || col[7] == "未绑定")) {
            // 找到该型号且未绑定 → 直接修改最后一列
            col[7] = monId;
            newLines.append(col.join(","));
            found = true;
        } else {
            newLines.append(l);
        }
    }

    if (!found) {
        // 所有该型号传感器都已绑定 → 追加新记录
        // 从现有记录复制传感器信息
        for (const QString &l : lines) {
            QStringList col = l.split(",", Qt::KeepEmptyParts);
            if (col.size() >= 8 && col[2] == senModel) {
                col[7] = monId;
                newLines.append(col.join(","));
                break;
            }
        }
    }

    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream st(&f);
    st.setEncoding(QStringConverter::Utf8);
    st << newLines.join("\n");
    f.close();
    refreshTable();
    QMessageBox::information(this, "绑定成功", "已完成绑定");
}
void SensorManageDialog::unBindPoint()
{
    int senIdx = m_cmbSensorSel->currentIndex();
    if (senIdx < 0)
        return;
    QString senModel = m_cmbSensorSel->currentText().split(" | ").first();
    // 在 sensor_storage.txt 中将该型号对应记录的最后一列改回"未绑定"
    QFile f(SENSOR_FILE);
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    QString all = f.readAll();
    f.close();
    QStringList lines = all.split("\n", Qt::KeepEmptyParts);
    QStringList newLines;
    for (int i = 0; i < lines.size(); i++) {
        QString l = lines[i];
        QStringList col = l.split(",", Qt::KeepEmptyParts);
        if (i == 0 || col.size() < 8) {
            newLines.append(l);
            continue;
        }
        if (col[2] == senModel && col[7] != "未绑定") {
            col[7] = "未绑定";
            newLines.append(col.join(","));
        } else {
            newLines.append(l);
        }
    }
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream st(&f);
    st.setEncoding(QStringConverter::Utf8);
    st << newLines.join("\n");
    f.close();
    refreshTable();
    QMessageBox::information(this, "解绑完成", "已解除关联");
}
void SensorManageDialog::slotAdd()
{
    SensorEditDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        Sensor *s = dlg.getNewSensor();
        bool ok = saveSensor(s, false);
        if (ok)
            QMessageBox::information(this, "成功", "新增完成");
        delete s;
        refreshTable();
    }
}
void SensorManageDialog::slotEdit()
{
    QModelIndexList sel = m_tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选中一行传感器");
        return;
    }
    int row = sel.first().row();
    QString model = m_tableModel->item(row, 2)->text();
    QString name = m_tableModel->item(row, 0)->text();
    QString size = m_tableModel->item(row, 1)->text();
    QString manu = m_tableModel->item(row, 3)->text();
    QDate prod = QDate::fromString(m_tableModel->item(row, 4)->text(), "yyyy-MM-dd");
    int freq = m_tableModel->item(row, 5)->text().toInt();
    QString type = m_tableModel->item(row, 6)->text();
    Sensor *temp;
    if (type == "风速风向传感器")
        temp = new WindSensor();
    else if (type == "振动监测传感器")
        temp = new VibrationSensor();
    else if (type == "索力监测传感器")
        temp = new CableForceSensor();
    else if (type == "挠度传感器")
        temp = new DeflectionSensor();
    else if (type == "支座位移传感器")
        temp = new DisplacementSensor();
    else if (type == "伸缩缝监测传感器")
        temp = new ExpansionJointSensor();
    else
        temp = new TemperatureHumiditySensor();
    temp->name = name;
    temp->size = size;
    temp->model = model;
    temp->manufacturer = manu;
    temp->generDate = prod;
    temp->frequency = freq;
    SensorEditDialog dlg(this);
    dlg.setEditTarget(temp);
    if (dlg.exec() == QDialog::Accepted) {
        Sensor *res = dlg.getNewSensor();
        saveSensor(res, true);
        delete res;
    }
    delete temp;
    refreshTable();
}
void SensorManageDialog::slotDel()
{
    QString target = QInputDialog::getText(this, "删除传感器", "输入传感器型号：");
    if (target.trimmed().isEmpty())
        return;
    bool exist = delSensorByModel(target);
    if (!exist) {
        QMessageBox::information(this, "提示", "未找到该型号传感器");
        return;
    }
    QMessageBox::StandardButton btn
        = QMessageBox::question(this,
                                "确认删除",
                                QString("确定删除型号%1，绑定关系同步清除？").arg(target),
                                QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::Yes) {
        delSensorByModel(target);
        refreshTable();
        QMessageBox::information(this, "完成", "删除成功");
    }
}
void SensorManageDialog::slotRefresh()
{
    refreshTable();
    QMessageBox::information(this, "刷新", "数据已同步");
}
