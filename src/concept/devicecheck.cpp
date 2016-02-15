#include "devicecheck.h"
#include "finger_device_check.h"
#include <QFile>
#include <QProcess>
#include <QTimer>

QTimer *deviceCheckTimer;
QProcess *printerProcess = new QProcess();
QString  processOutput;

DeviceCheck::DeviceCheck(QObject *parent) :
        QObject(parent)
{
    deviceCheckTimer = new QTimer();
    connect(deviceCheckTimer, SIGNAL(timeout()), this, SLOT(checkDevices()));
    deviceCheckTimer->start(10000);
    connect(printerProcess, SIGNAL(finished(int)), this, SLOT(printerStatus(int)));
}

void DeviceCheck::checkTimer() {
    qDebug() << "Device Checker Started";
    deviceCheckTimer = new QTimer();
    connect(deviceCheckTimer, SIGNAL(timeout()), this, SLOT(checkDevices()));
    deviceCheckTimer->start(10000);
}

void DeviceCheck::checkDevices()
{
    qDebug() << "Checking for Devices";

        QFile cameraChecker;
        if(cameraChecker.exists("/dev/video1") || cameraChecker.exists("/dev/video0")){
            cameraStatusUpdate(1);
        } else{
            cameraStatusUpdate(0);
        }


        processOutput="";
        QProcess *scriptProcess = new QProcess();
        scriptProcess->start("./scripts/check_removable.sh");

        if (!scriptProcess->waitForFinished()){
            qDebug() << "Error :" << scriptProcess->errorString();
        } else {
            processOutput =  scriptProcess->readAll();
        }

        if (processOutput != "") {
            emit backupStatusUpdate(1);
        } else {
            emit backupStatusUpdate(0);
        }


        if (fingerDeviceExists()) {
            emit fpDeviceStatusUpdate(1);
        } else {
            emit fpDeviceStatusUpdate(0);
        }


        processOutput="";
        printerProcess->start("./scripts/check_printer.sh");

}

void DeviceCheck::printerStatus(int exitStatus)
{

    qDebug() << "Exit Status of Check Printer: " << exitStatus;
    qDebug() << "Error :" << printerProcess->errorString();
    processOutput =  printerProcess->readAll();

    if (processOutput != "") {
        emit printerStatusUpdate(1);
    } else {
        emit printerStatusUpdate(0);
    }
}
