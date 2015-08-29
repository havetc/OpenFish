#include "mainwindow.h"
#include "OpenWarp.cpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    start(argc, argv);

    return a.exec();
}

