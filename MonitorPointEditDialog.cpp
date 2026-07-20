#include "MonitorPointEditDialog.h"

MonitorPointEditDialog::MonitorPointEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("新增监测点");
    setFixedSize(420, 320);

    m_lePointId = new QLineEdit;
    m_lePointId->setPlaceholderText("如 MP-W-01");
    m_leSection = new QLineEdit;
    m_leSection->setPlaceholderText("如 北塔塔顶");
    m_deInstall = new QDateEdit(QDate::currentDate());
    m_deInstall->setDisplayFormat("yyyy-MM-dd");

    m_cmbDataType = new QComboBox();
    m_cmbDataType->addItems({
        "索力监测传感器",
        "挠度传感器",
        "振动监测传感器",
        "支座位移传感器",
        "伸缩缝监测传感器",
        "风速风向传感器",
        "温湿度监测传感器",
    });

    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);
    form->addRow("监测点编号：", m_lePointId);
    form->addRow("断面名称：", m_leSection);
    form->addRow("安装日期：", m_deInstall);
    form->addRow("数据类型：", m_cmbDataType);

    QPushButton *btnOk = new QPushButton("确认保存");
    QPushButton *btnCancel = new QPushButton("取消");
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addSpacing(15);
    main->addLayout(btnLayout);
    main->setContentsMargins(24, 24, 24, 24);

    connect(btnOk, &QPushButton::clicked, this, &MonitorPointEditDialog::slotConfirm);
    connect(btnCancel, &QPushButton::clicked, this, &MonitorPointEditDialog::slotCancel);
}

void MonitorPointEditDialog::setEditTarget(const MonitoringPoint &mp)
{
    m_isAddMode = false;
    m_oldPointId = mp.pointId;
    setWindowTitle("修改监测点信息");
    m_lePointId->setText(mp.pointId);
    m_leSection->setText(mp.sectionName);
    m_deInstall->setDate(mp.installDate);

    int idx = m_cmbDataType->findText(mp.dataType);
    if (idx >= 0)
        m_cmbDataType->setCurrentIndex(idx);
}

MonitoringPoint MonitorPointEditDialog::getPointData() const
{
    MonitoringPoint mp;
    mp.pointId = m_lePointId->text().trimmed();
    mp.sectionName = m_leSection->text().trimmed();
    mp.installDate = m_deInstall->date();
    mp.dataType = m_cmbDataType->currentText();
    mp.sensor = nullptr;
    return mp;
}

void MonitorPointEditDialog::slotConfirm()
{
    QString pid = m_lePointId->text().trimmed();
    QString sec = m_leSection->text().trimmed();
    if (pid.isEmpty() || sec.isEmpty()) {
        QMessageBox::warning(this, "输入校验", "监测点编号、断面名称不能为空！");
        return;
    }
    accept();
}

void MonitorPointEditDialog::slotCancel()
{
    reject();
}
