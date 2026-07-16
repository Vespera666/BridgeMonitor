#ifndef MONITORPOINTEDITDIALOG_H
#define MONITORPOINTEDITDIALOG_H

#include <QDateEdit>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "monitorpoint.h"

class MonitorPointEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MonitorPointEditDialog(QWidget *parent = nullptr);
    void setEditTarget(const MonitoringPoint &mp);
    MonitoringPoint getPointData() const;
private slots:
    void slotConfirm();
    void slotCancel();

private:
    QLineEdit *m_lePointId;
    QLineEdit *m_leSection;
    QDateEdit *m_deInstall;
    bool m_isAddMode = true;
    QString m_oldPointId;
};

#endif // MONITORPOINTEDITDIALOG_H
