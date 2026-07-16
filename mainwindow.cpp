#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "SensorManageDialog.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_monitorBtn_clicked() {}

void MainWindow::on_sensorBtn_clicked() {
    SensorManageDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_historydataBtn_clicked() {}

void MainWindow::on_currentdataBtn_clicked() {}
