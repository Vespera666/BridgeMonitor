#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:

    void on_monitorBtn_clicked();

    void on_sensorBtn_clicked();

    void on_historydataBtn_clicked();

    void on_currentdataBtn_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
