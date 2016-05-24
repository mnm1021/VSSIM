#include "monitorform.h"
#include "ui_monitorform.h"

int FLASH_NB, PLANES_PER_FLASH;

MonitorForm::MonitorForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonitorForm)
{
    /* initialize variables. */
    init_variables();

    ui->setupUi(this);

    /* initialize timer. */
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(1);

    /* initialize socket connected with VSSIM */
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(onReceive()));
    socket->connectToHost("127.0.0.1", 9995);
}

MonitorForm::~MonitorForm()
{
    delete ui;
}

/*
 * initialize variables.
 */
void MonitorForm::init_variables()
{
    FILE* pfData;
    char szCommand[1024];

    /* initialize count variables. */
    time = 0;
    powCount = 0;

    gcCount = 0;
    gcStarted = 0;
    randMergeCount = seqMergeCount = 0;
    //overwriteCount = 0;

    writeCount = readCount = 0;
    writeSectorCount = readSectorCount = 0;
    trimEffect = trimCount = 0;
    writeAmpCount = writtenBlockCount = 0;

    readTime = writeTime = 0;

    /* open ssd configuration file, set values. */
    pfData = fopen("./ssd.conf","r");

    if(pfData == NULL)
        printf(" Monitor file open failed\n");
    else
    {
        printf(" Monitor file open success\n");

        while(fscanf(pfData, "%s", szCommand) != EOF)
        {
            if(strcmp(szCommand, "FLASH_NB") == 0)
                fscanf(pfData, "%d", &FLASH_NB);

            else if(strcmp(szCommand, "PLANES_PER_FLASH") == 0)
                fscanf(pfData, "%d", &PLANES_PER_FLASH);

            else if(strcmp(szCommand, "CELL_PROGRAM_DELAY") == 0)
                fscanf(pfData, "%d", &CELL_PROGRAM_DELAY);

            memset(szCommand, 0x00, 1024);
        }

        fclose(pfData);

        access_time_reg_mon = (long long int*)malloc(sizeof(long long int) * FLASH_NB * PLANES_PER_FLASH);
        access_type_reg_mon = (int *)malloc(sizeof(int) * FLASH_NB * PLANES_PER_FLASH);

        printf("\n\t initialize\n");
        memset(access_time_reg_mon, 0x00, sizeof(long long int) * FLASH_NB * PLANES_PER_FLASH);
        memset(access_type_reg_mon, 0x00, sizeof(int) * FLASH_NB * PLANES_PER_FLASH);
    }

    fflush(stdout);
}

/*
 * callback method for timer.
 */
void MonitorForm::onTimer()
{
    char sz_timer[20];
    time++;
    sprintf(sz_timer, "%lld", time);
    ui->txtTime->setText(sz_timer);
}

/*
 * callback method for socket.
 */
void MonitorForm::onReceive()
{
    QStringList szCmdList;
    QString szCmd;
    char szTemp[128];

    while(socket->canReadLine())
    {

        szCmd = socket->readLine();
        szCmdList = szCmd.split(" ");

        if(szCmdList[0] == "POWER")
        {
            int regst;
            QTextStream stream1(&szCmdList[1]), stream2(&szCmdList[2]), stream3(&szCmdList[3]);
            stream1 >> regst;
            stream2 >> access_time_reg_mon[regst];
            stream3 >> access_type_reg_mon[regst];
//            sscanf(szCmdList[1], "%d", &regst);
//            sscanf(szCmdList[2], "%lld", &access_time_reg_mon[regst]);
//            sscanf(szCmdList[3], "%d", &access_type_reg_mon[regst]);
        }

        else if(szCmdList[0] == "WRITE")
        {
            QTextStream stream(&szCmdList[2]);

            if(szCmdList[1] == "PAGE")
            {
                unsigned int length;
                stream >> length;
//                sscanf(szCmdList[2], "%u", &length);

                // Write Sector Number Count
                writeSectorCount += length;

                sprintf(szTemp, "%ld", writeSectorCount);
                ui->txtWriteSectorCount->setText(szTemp);

                // Write SATA Command Count
                writeCount++;
                sprintf(szTemp, "%ld", writeCount);
                ui->txtWriteCount->setText(szTemp);
            }
            else if(szCmdList[1] == "BW")
            {
                double t;
                stream >> t;
//                sscanf(szCmdList[2], "%lf", &t);
                if(t != 0){
                    sprintf(szTemp, "%0.3lf", t);
                    ui->txtWriteSpeed->setText(szTemp);
                }
            }
        }

        else if(szCmdList[0] == "READ")
        {
            QTextStream stream(&szCmdList[2]);

            if(szCmdList[1] == "PAGE"){
                unsigned int length;

                /* Read Sector Number Count */
                stream >> length;
//                sscanf(szCmdList[2], "%u", &length);
                readSectorCount += length;

                sprintf(szTemp, "%ld", readSectorCount);
                ui->txtReadSectorCount->setText(szTemp);

                /* Read SATA Command Count */
                readCount++;
                sprintf(szTemp, "%ld", readCount);
                ui->txtReadCount->setText(szTemp);
            }
            else if(szCmdList[1] == "BW"){
                double t;
                stream >> t;
//                sscanf(szCmdList[2], "%lf", &t);
                if(t != 0)
                {
                    sprintf(szTemp, "%0.3lf", t);
                    ui->txtReadSpeed->setText(szTemp);
                }
            }
        }

        else if(szCmdList[0] == "READF")
        {
            int flash_nb;
            QTextStream stream(&szCmdList[1]);
            stream >> flash_nb;
//            sscanf(szCmdList[1], "%d", &flash_nb);
        }

        else if(szCmdList[0] == "GC")
        {
            gcCount++;
            sprintf(szTemp, "%ld", gcCount);
            ui->txtGCCount->setText(szTemp);
        }

        else if(szCmdList[0] == "WB")
        {
            long long int wb = 0;
            QTextStream stream(&szCmdList[2]);
            stream >> wb;
//            sscanf(szCmdList[2], "%lld", &wb);

            if(szCmdList[1] == "CORRECT")
            {
                    writtenBlockCount += wb;
                    sprintf(szTemp, "%ld", writtenBlockCount);
                    ui->txtWrittenBlockCount->setText(szTemp);
            }
            else
            {
                    writeAmpCount += wb;
                    sprintf(szTemp, "%ld", writeAmpCount);
                    ui->txtWriteAmp->setText(szTemp);
            }
        }

        else if(szCmdList[0] == "TRIM")
        {
            if(szCmdList[1] == "INSERT")
            {
                trimCount++;
                sprintf(szTemp, "%d", trimCount);
                ui->txtTrimCount->setText(szTemp);
            }
            else
            {
                int effect = 0;
                QTextStream stream(&szCmdList[2]);
                stream >> effect;
//                sscanf(szCmdList[2], "%d", &effect);
                trimEffect+= effect;
                sprintf(szTemp, "%d", trimEffect);
                ui->txtTrimEffect->setText(szTemp);
            }
        }

        else if(szCmdList[0] == "UTIL")
        {
            double util = 0;
            QTextStream stream(&szCmdList[1]);
            stream >> util;
//            sscanf(szCmdList[1], "%lf", &util);
            sprintf(szTemp, "%lf", util);
            ui->txtSSDUtil->setText(szTemp);
        }

        ui->txtDebug->setText(szCmd);
    }
}
