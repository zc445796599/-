#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    a.setOrganizationName("zc");
    a.setApplicationName("　文本阅读器　");
    w.show();

    return a.exec();
}
