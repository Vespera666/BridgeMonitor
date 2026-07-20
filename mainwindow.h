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
    explicit MainWindow(const QString &username,
                        const QString &role,
                        UserManager *userManager,
                        QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_monitorBtn_clicked();
    void on_sensorBtn_clicked();
    void on_historydataBtn_clicked();
    void on_currentdataBtn_clicked();
    void on_accountBtn_clicked();

private:
    void rebuildCentralWidget();
    void applyPermissions();
    Ui::MainWindow *ui;
    QString m_username;
    QString m_role;
    UserManager *m_userManager;
};
#endif // MAINWINDOW_H
