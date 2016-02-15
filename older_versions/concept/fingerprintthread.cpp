#include "fingerprintthread.h"
#include "myfprint.h"
#include "QProcess"
#include "QDir"
int lastEnrollPid =0;
int restartsCount =0;
FingerPrintThread::FingerPrintThread(QObject *parent) :
        QObject(parent)
{
    dontRestartDevice = false;
}

void FingerPrintThread::captureFinger(int printIndex){

    QString  processOutput;

    QProcess *fingerProcess = new QProcess();

    fingerProcess->setProcessChannelMode(QProcess::MergedChannels);
    //fingerProcess->kill();

    QDir::current();
    fingerProcess->start("./enroll");
    //qDebug() << "Process PID: " << fingerProcess->pid();
    bool processTimedOut = false;
    if (!fingerProcess->waitForFinished()){

        qDebug() << "Enroll Exe Failed :" << fingerProcess->errorString();
        processTimedOut = true;


    } else {
        processOutput =  fingerProcess->readAll();
    }

    qDebug() << "Final Output " << processOutput;




    if(processOutput.toUtf8()==""||processOutput.toUtf8()=="-22"){
        qDebug() << "Device crashed/timedout";
        QProcess *killFingerProcess = new QProcess();

        killFingerProcess->startDetached("scripts/close_fp.sh");
        QString killString;
        killString.sprintf("kill -9 %d",fingerProcess->pid());
        killFingerProcess->startDetached(killString);



        sleep(0.5);
        qDebug() << "dontRestartDevice :" << dontRestartDevice;
        if(processTimedOut){
            restartsCount++;

            if(restartsCount > 4){
                restartsCount=0;
                emit(deviceCrashed(printIndex));
                return;
            }
            qDebug() << "Internally restarting device scan";
            dontRestartDevice = true;
            captureFinger(printIndex);
            return;
        }else{
            if(dontRestartDevice){

                qDebug() << "Not Emitting Signal";

            } else {

                qDebug() << "Emitting Signal";
                dontRestartDevice = true;
                emit(deviceCrashed(printIndex));

            }
        }

        return;

    }

    emit(processCompleted(printIndex,processOutput));

}

void FingerPrintThread::toggleRecovery(bool enableRestart){

    if(enableRestart){
        dontRestartDevice = false;
    }else{
        dontRestartDevice = true;
    }



}
