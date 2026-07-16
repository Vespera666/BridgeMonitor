#include "monitorpointdialog.h"
#include "ui_monitorpointdialog.h"

MonitorPointDialog::MonitorPointDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::MonitorPointDialog)
{
    ui->setupUi(this);
}

MonitorPointDialog::~MonitorPointDialog()
{
    delete ui;
}
