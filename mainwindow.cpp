#include "mainwindow.h"
#include <QHeaderView>
#include "./ui_mainwindow.h"
#include "AccountManageDialog.h"
#include "HistoryDataDialog.h"
#include "MonitorPointManageDialog.h"
#include "RealTimeDataDialog.h"
#include "SensorManageDialog.h"
#include "UserManager.h"

MainWindow::MainWindow(const QString &username,
                       const QString &role,
                       UserManager *userManager,
                       QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_username(username)
    , m_role(role)
    , m_userManager(userManager)
{
    ui->setupUi(this);

    // 窗口标题显示当前用户
    setWindowTitle(
        QStringLiteral("万州三桥桥梁监测系统  —  当前用户：%1 (%2)").arg(m_username, m_role));

    applyPermissions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_monitorBtn_clicked()
{
    MonitorPointManageDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_sensorBtn_clicked()
{
    SensorManageDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_historydataBtn_clicked()
{
    HistoryDataDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_currentdataBtn_clicked()
{
    RealTimeDataDialog dlg(this);
    dlg.exec();
}

void MainWindow::applyPermissions()
{
    if (m_role == "analyst") {
        // 分析师：只看数据
        ui->monitorBtn->hide();
        ui->sensorBtn->hide();
        ui->accountBtn->hide();
        // historydataBtn 和 currentdataBtn 保持可见
    } else if (m_role == "engineer") {
        // 工程师：除账号管理外都可见
        ui->accountBtn->hide();
    }
    // admin：全部可见（不隐藏任何按钮）
}

void MainWindow::on_accountBtn_clicked()
{
    AccountManageDialog dlg(m_userManager, this);
    dlg.exec();
}
