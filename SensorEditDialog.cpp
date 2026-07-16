#include "SensorEditDialog.h"
#include "CableForceSensor.h"
#include "VibrationSensor.h"
#include "deflection.h"
#include "displacement.h"
#include "expansionjointsensor.h"
#include "temperaturehumiditysensor.h"
#include "windsensor.h"
SensorEditDialog::SensorEditDialog(QWidget *parent)
    : QDialog(parent)
{
    this->setWindowTitle("传感器信息编辑");
    this->setFixedSize(540, 380);

    QFormLayout *form = new QFormLayout();
    form->setSpacing(12);

    m_leName = new QLineEdit;
    m_leSize = new QLineEdit;
    m_leModel = new QLineEdit;
    m_leManufacturer = new QLineEdit;
    m_deProduce = new QDateEdit(QDate::currentDate());
    m_deProduce->setDisplayFormat("yyyy-MM-dd");
    m_leFreq = new QLineEdit;
    m_cmbType = new QComboBox;
    m_cmbType->addItems({"风速风向传感器",
                         "振动监测传感器",
                         "索力监测传感器",
                         "挠度传感器",
                         "支座位移传感器",
                         "伸缩缝监测传感器",
                         "温湿度监测传感器"});

    form->addRow("设备名称：", m_leName);
    form->addRow("规格：", m_leSize);
    form->addRow("型号：", m_leModel);
    form->addRow("生产厂家：", m_leManufacturer);
    form->addRow("生产日期：", m_deProduce);
    form->addRow("采集频率(秒)：", m_leFreq);
    form->addRow("传感器类型：", m_cmbType);

    QPushButton *btnOk = new QPushButton("确认保存");
    QPushButton *btnCancel = new QPushButton("取消");
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    QVBoxLayout *main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addSpacing(20);
    main->addLayout(btnLayout);
    main->setContentsMargins(30, 30, 30, 30);

    connect(btnOk, &QPushButton::clicked, this, &SensorEditDialog::slotConfirm);
    connect(btnCancel, &QPushButton::clicked, this, &SensorEditDialog::slotCancel);
}

void SensorEditDialog::setEditTarget(Sensor *srcSensor)
{
    m_isAddMode = false;
    m_oldSensor = srcSensor;
    setWindowTitle("修改传感器信息");
    m_leName->setText(srcSensor->name);
    m_leSize->setText(srcSensor->size);
    m_leModel->setText(srcSensor->model);
    m_leManufacturer->setText(srcSensor->manufacturer);
    m_deProduce->setDate(srcSensor->generDate);
    m_leFreq->setText(QString::number(srcSensor->frequency));
    m_cmbType->setCurrentText(srcSensor->sensorType());
}

Sensor *SensorEditDialog::getNewSensor()
{
    QString type = m_cmbType->currentText();
    Sensor *s = nullptr;
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

    s->name = m_leName->text().trimmed();
    s->size = m_leSize->text().trimmed();
    s->model = m_leModel->text().trimmed();
    s->manufacturer = m_leManufacturer->text().trimmed();
    s->generDate = m_deProduce->date();
    s->frequency = m_leFreq->text().toInt();
    s->boundPoint = nullptr;
    s->isWorking = true;
    return s;
}

QString SensorEditDialog::getInputModel() const
{
    return m_leModel->text().trimmed();
}

void SensorEditDialog::slotConfirm()
{
    QString name = m_leName->text().trimmed();
    QString model = m_leModel->text().trimmed();
    QString freqStr = m_leFreq->text().trimmed();
    if (name.isEmpty() || model.isEmpty() || freqStr.isEmpty()) {
        QMessageBox::warning(this, "输入校验", "设备名称、型号、采集频率不能为空！");
        return;
    }
    if (freqStr.toInt() <= 0) {
        QMessageBox::warning(this, "输入校验", "采集频率必须为正整数！");
        return;
    }
    accept();
}

void SensorEditDialog::slotCancel()
{
    reject();
}