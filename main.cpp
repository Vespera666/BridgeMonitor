#include "LoginDialog.h"
#include "UserManager.h"
#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载全局样式表
    QFile styleFile(":/style/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        a.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
        styleFile.close();
    }

    UserManager userManager("users.txt");

    if (!userManager.verify("admin", "123456"))
        userManager.registerUser("admin", "123456");
    if (userManager.roleOf("admin") != "admin")
        userManager.changeRole("admin", "admin");

    LoginDialog dlg(&userManager);
    if (dlg.exec() != QDialog::Accepted)
        return 0;

    MainWindow w(dlg.loggedUsername(), dlg.loggedRole(), &userManager);
    w.show();
    return a.exec();
}
