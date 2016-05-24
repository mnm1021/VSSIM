#include "monitorform.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MonitorForm w;
    w.show();

    return a.exec();
}
