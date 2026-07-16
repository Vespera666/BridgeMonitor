#include "LoginDialog.h"
#include "UserManager.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 数据层
    UserManager userManager("users.txt");

    // 首次运行创建默认管理员
    if (!userManager.verify("admin", "123456"))
        userManager.registerUser("admin", "123456");
    // 修正管理员角色（兼容旧文件格式）
    if (userManager.roleOf("admin") != "admin")
        userManager.changeRole("admin", "admin");

    // UI 层：登录
    LoginDialog dlg(&userManager);
    if (dlg.exec() != QDialog::Accepted)
        return 0; // 退出

    // 登录成功，传身份进主窗口
    MainWindow w(dlg.loggedUsername(), dlg.loggedRole(), &userManager);
    w.show();
    return a.exec();
}
