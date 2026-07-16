#ifndef SENSOREDITDIALOG_H
#define SENSOREDITDIALOG_H
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "sensor.h"

class SensorEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SensorEditDialog(QWidget *parent = nullptr);
    ~SensorEditDialog() override = default;
    void setEditTarget(Sensor *srcSensor);
    Sensor *getNewSensor();
    QString getInputModel() const;

private slots:
    void slotConfirm();
    void slotCancel();

private:
    QLineEdit *m_leName;
    QLineEdit *m_leSize;
    QLineEdit *m_leModel;
    QLineEdit *m_leManufacturer;
    QDateEdit *m_deProduce;
    QLineEdit *m_leFreq;
    QComboBox *m_cmbType;
    bool m_isAddMode = true;
    Sensor *m_oldSensor = nullptr;
};
#endif // SENSOREDITDIALOG_H