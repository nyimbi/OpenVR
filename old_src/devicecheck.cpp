#include "devicecheck.h"
#include "finger_device_check.h"
#include <QFile>
#include <QProcess>
#include <QTimer>

QTimer *deviceCheckTimer;
QTimer *printerTimer;
QProcess *printerProcess = new QProcess();
QProcess *printerTimerProcess = new QProcess();
QString  processOutput;
QString printerProcessOutput;

DeviceCheck::DeviceCheck(QObject *parent) : QObject(parent)
{
    deviceCheckTimer = new QTimer();
    connect(deviceCheckTimer, SIGNAL(timeout()), this, SLOT(checkDevices()));
    deviceCheckTimer->start(10000);




    printerTimer = new QTimer();
    connect(printerTimer, SIGNAL(timeout()), this, SLOT(checkPrinterDevice()));
    printerTimer->start(15000);
    connect(printerTimerProcess,SIGNAL(finished(int)),this,SLOT(updatePrinterStatus(int)));
    printerTimerProcess->start("scripts/check_printer.sh");


}

void DeviceCheck::checkPrinterDevice(){

    printerTimerProcess->start("scripts/check_printer.sh");

}

void DeviceCheck::updatePrinterStatus(int exitCode){
   // qDebug() << "Printer Output " << printerTimerProcess->readAll();
printerProcessOutput = printerTimerProcess->readAll();

}

void DeviceCheck::checkTimer() {
  //  qDebug() << "Device Checker Started";
    checkDevices();
    deviceCheckTimer = new QTimer();
    connect(deviceCheckTimer, SIGNAL(timeout()), this, SLOT(checkDevices()));
//    connect(printerProcess, SIGNAL(finished(int)), this, SLOT(printerStatus(int)));
    deviceCheckTimer->start(10000);
//    printerProcess->start("./scripts/check_printer.sh");
}

void DeviceCheck::checkDevices()
{
    //return;//added this to reduce noise when debugging
//    qDebug() << "Checking for Devices";

    QFile cameraChecker;
    if(cameraChecker.exists("/dev/video1") || cameraChecker.exists("/dev/video0")){
        cameraStatusUpdate(1);
    } else{
        cameraStatusUpdate(0);
    }

    processOutput="";
    QProcess *scriptProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    scriptProcess->start("./scripts/check_backup_storage.sh", args);

    if (!scriptProcess->waitForFinished()){
//        qDebug() << "Error :" << scriptProcess->errorString();
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





/*
    processOutput="";
    QProcess *printerProcess = new QProcess();
    printerProcess->start("./scripts/printer_check.sh");

    if (printerProcess->waitForFinished()){
        processOutput =  printerProcess->readAll();
    }

    if (processOutput != "") {
        emit printerStatusUpdate(1);
    } else {
        emit printerStatusUpdate(0);
    }
*/
    if (printerProcessOutput != "") {
        emit printerStatusUpdate(1);
    } else {
        emit printerStatusUpdate(0);
    }




    processOutput="";
    QProcess *batteryStatusProcess = new QProcess();
    batteryStatusProcess->start("./scripts/battery_status.sh");

    if (!batteryStatusProcess->waitForFinished()){
//        qDebug() << "Error :" << batteryStatusProcess->errorString();
    } else {
        processOutput =  batteryStatusProcess->readAll();
    }

    if (processOutput != "") {
        emit batteryStatusUpdate(processOutput);
    } else {
        emit batteryStatusUpdate(processOutput);
    }

    processOutput="";
    int batteryLevel;
    int cap = getBatteryCapacity();
    int fullCap = getBatteryFullCapacity();

    batteryLevel = cap*100/fullCap;

//    qDebug() << "Battery Level: " << QString::number(batteryLevel);
    emit batteryLevelUpdate(batteryLevel);
}

void DeviceCheck::printerStatus(int exitStatus)
{/*
    qDebug() << "Exit Status of Check Printer: " << exitStatus;
    qDebug() << "Error :" << printerProcess->errorString();
    processOutput =  printerProcess->readAll();

    if (processOutput != "") {
        emit printerStatusUpdate(1);
    } else {
        emit printerStatusUpdate(0);
    }*/

//    printerProcess->start("./scripts/check_printer.sh");
}


int DeviceCheck::getBatteryCapacity()
{
    processOutput="";
    QProcess *batteryStatusProcess = new QProcess();
    batteryStatusProcess->start("./scripts/battery_capacity.sh");
    if (!batteryStatusProcess->waitForFinished()){
//        qDebug() << "Error :" << batteryStatusProcess->errorString();
    } else {
        processOutput =  batteryStatusProcess->readAll();
    }
//    qDebug() << "Battery Current Capacity: " << QString::number(processOutput.toInt());
    if (processOutput.toInt() < 0) {
        return 0;
    }
    return processOutput.toInt();
}

int DeviceCheck::getBatteryFullCapacity()
{
    processOutput="";
    QProcess *batteryStatusProcess = new QProcess();
    batteryStatusProcess->start("./scripts/battery_full_capacity.sh");
    if (!batteryStatusProcess->waitForFinished()){
//        qDebug() << "Error :" << batteryStatusProcess->errorString();
    } else {
        processOutput =  batteryStatusProcess->readAll();
    }

//    qDebug() << "Battery Last Full Capacity: " << QString::number(processOutput.toInt());

    if (processOutput.toInt() == 0) {
        return 1;
    }
    return processOutput.toInt();
}
