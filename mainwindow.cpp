#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "AccountManageDialog.h"
#include "HistoryDataDialog.h"
#include "MonitorPointManageDialog.h"
#include "RealTimeDataDialog.h"
#include "SensorManageDialog.h"
#include "UserManager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSizePolicy>
#include <QVBoxLayout>

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

    setWindowTitle(
        QStringLiteral("万州三桥桥梁监测系统  —  当前用户：%1 (%2)").arg(m_username, m_role));

    // ── 用代码重建 centralWidget 布局，替代 .ui 中的绝对定位 ──
    rebuildCentralWidget();

    applyPermissions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::rebuildCentralWidget()
{
    // 获取 .ui 中已创建的按钮（保留 objectName 以兼容自动槽连接）
    QPushButton *btnMonitor = ui->monitorBtn;
    QPushButton *btnSensor = ui->sensorBtn;
    QPushButton *btnHistory = ui->historydataBtn;
    QPushButton *btnCurrent = ui->currentdataBtn;
    QPushButton *btnAccount = ui->accountBtn;

    // 设置导航按钮 CSS class
    btnMonitor->setProperty("cssClass", "nav-btn");
    btnSensor->setProperty("cssClass", "nav-btn");
    btnHistory->setProperty("cssClass", "nav-btn");
    btnCurrent->setProperty("cssClass", "nav-btn");
    btnAccount->setProperty("cssClass", "nav-btn");

    // ── 左侧导航面板 ──
    QWidget *navPanel = new QWidget();
    navPanel->setFixedWidth(140);
    QVBoxLayout *navLayout = new QVBoxLayout(navPanel);
    navLayout->setSpacing(10);
    navLayout->setContentsMargins(0, 0, 0, 0);

    navLayout->addStretch(1);
    navLayout->addWidget(btnMonitor);
    navLayout->addWidget(btnSensor);
    navLayout->addWidget(btnHistory);
    navLayout->addWidget(btnCurrent);
    navLayout->addWidget(btnAccount);
    navLayout->addStretch(1);

    // ── 右侧背景图片 ──
    QLabel *bgLabel = new QLabel();
    bgLabel->setPixmap(QPixmap(":/images/screenshot.png"));
    bgLabel->setScaledContents(true);
    bgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ── 主布局 ──
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);
    mainLayout->addWidget(navPanel);
    mainLayout->addWidget(bgLabel, 1);

    setCentralWidget(centralWidget);
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
        ui->monitorBtn->hide();
        ui->sensorBtn->hide();
        ui->accountBtn->hide();
    } else if (m_role == "engineer") {
        ui->accountBtn->hide();
    }
}

void MainWindow::on_accountBtn_clicked()
{
    AccountManageDialog dlg(m_userManager, this);
    dlg.exec();
}
