#ifndef MONITORFORM_H
#define MONITORFORM_H

#include <QDialog>
#include <QTimer>
#include <QTcpSocket>

namespace Ui {
class MonitorForm;
}

class MonitorForm : public QDialog
{
    Q_OBJECT

public slots:
    void onTimer();
    void onReceive();

public:
    explicit MonitorForm(QWidget *parent = 0);
    ~MonitorForm();
    void init_variables();

private:
    Ui::MonitorForm *ui;
    QTcpSocket *socket;
    QTimer *timer;

    /* variables */
    long long int time;
    long long int *access_time_reg_mon;
    int *access_type_reg_mon;
    int CELL_PROGRAM_DELAY;
    int powCount;

    long int gcCount;
    int gcStarted;
    int randMergeCount, seqMergeCount;
    long int overwriteCount;

    long int writeCount, readCount;
    long int writeSectorCount, readSectorCount;
    int trimEffect, trimCount;
    long int writeAmpCount, writtenBlockCount;

    double readTime, writeTime;
};

#endif // MONITORFORM_H
