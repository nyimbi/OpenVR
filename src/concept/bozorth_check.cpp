#include "bozorth_check.h"
#include "QProcess"
#include "QDebug"
#include "QDir"

QString minDtctExe = "mindtct";
QString bozorth3Exe = "bozorth3";
QString storedMinutiaDir = "stored_minutiae";
int matchThreshold = 20;
int matchMinutiae = 30;

Bozorth_Check::Bozorth_Check(QObject *parent) :
    QObject(parent)
{

}


void Bozorth_Check::checkGallery(QString filePath,int printIndex){

    qDebug()<< "Signal Sent About enrollFile: " << filePath;

    qDebug() << "Calling Bozorth3Check";
    QProcess bozorthProcess0;
    QProcess bozorthProcess;
    QString bozorthCheckResult;

    QString mindtctTarget;

    mindtctTarget.sprintf("templates/%d", printIndex);

    QString existingFiles;


    existingFiles.sprintf("rm -f templates/%d.xyt",printIndex);

    //qDebug() <<" exe code: " << existingFiles;

    QProcess::execute(existingFiles);

    QStringList mindtctArguments;

    QString fullMindtctExe;

    fullMindtctExe.sprintf("  templates/%d ", printIndex );

    fullMindtctExe=minDtctExe+" "+filePath+fullMindtctExe;

    mindtctArguments << filePath <<mindtctTarget;

    bozorthProcess0.setProcessChannelMode(QProcess::MergedChannels);
    qDebug() << fullMindtctExe;

    bozorthProcess0.start(fullMindtctExe);


    if (!bozorthProcess0.waitForFinished()){
        qDebug() << "Failed :" << bozorthProcess0.errorString();
        return;
    } else {
        QString mindtctCheckResult =  bozorthProcess0.readAll();
        qDebug() << "mindtctCheckResult: " <<mindtctCheckResult;

    }

    bozorthProcess.setProcessChannelMode(QProcess::SeparateChannels);

    QString fullBozorth3Exe;

    fullBozorth3Exe.sprintf("./scripts/search.sh %d %d  ../templates/%d.xyt",matchThreshold,matchMinutiae,printIndex);

    qDebug() << fullBozorth3Exe;

    //QString currentDir = QDir::currentPath();

    //QDir::setCurrent(storedMinutiaDir);

    bozorthProcess.start(fullBozorth3Exe);

    //connect(bozorthProcess,SIGNAL(finished()),this,SLOT(processBozorthResult()));

    //QDir::setCurrent(currentDir);

    if (!bozorthProcess.waitForFinished()){
        qDebug() << "Failed :" << bozorthProcess.errorString();
    } else {
        bozorthCheckResult =  bozorthProcess.readAllStandardOutput();
    }
    emit(scanResult(printIndex,bozorthCheckResult));
}
