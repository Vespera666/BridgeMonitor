#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.close();
    w.close();
    w.show();
    return QCoreApplication::exec();
}
