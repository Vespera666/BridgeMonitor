#ifndef MONITORPOINTDIALOG_H
#define MONITORPOINTDIALOG_H

#include <QDialog>

namespace Ui {
class MonitorPointDialog;
}

class MonitorPointDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MonitorPointDialog(QWidget *parent = nullptr);
    ~MonitorPointDialog();

private:
    Ui::MonitorPointDialog *ui;
};

#endif // MONITORPOINTDIALOG_H
