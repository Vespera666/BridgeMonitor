#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class UserManager;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 构造函数接收登录者信息 + UserManager 指针（管理员需要调账号管理）
    explicit MainWindow(const QString &username, const QString &role,
                        UserManager *userManager, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_monitorBtn_clicked();
    void on_sensorBtn_clicked();
    void on_historydataBtn_clicked();
    void on_currentdataBtn_clicked();
    void on_accountBtn_clicked();  // 管理员专属

private:
    void applyPermissions();  // 根据角色隐藏/显示按钮

    Ui::MainWindow *ui;
    QString m_username;
    QString m_role;
    UserManager *m_userManager;
};
#endif // MAINWINDOW_H
