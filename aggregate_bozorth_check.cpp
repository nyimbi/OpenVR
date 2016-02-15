#include "aggregate_bozorth_check.h"
#include "aggregate_scan.h"
#include "mainwindow.h"
#include "QProcess"
#include "QDebug"
#include "QDir"
#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

//QString aggregateStoredMinutiaDir = "/home/femi/qt4apps/OpenVRSystem/openvr/stored_minutiae";
QString aggregateStoredMinutiaDir = "stored_minutiae";

int aggregateMatchMinutiae = 30;
aggregate_scan *m_ui;

aggregate_bozorth_check::aggregate_bozorth_check(QObject *parent) :
        QObject(parent)
{
    //MainWindow *m_ui;
        m_ui = qobject_cast<aggregate_scan *>(parent);

}

void aggregate_bozorth_check::checkGallery(QString filePath,int printIndex){

    qDebug() << filePath << printIndex;

}

void aggregate_bozorth_check::startAfis() {

    QString sqlQuery;
    QString sqlQuery2;
    QString fpSqlQuery;
    QSqlQuery dbQuery;
    QSqlQuery innerDbQuery;
    QSqlQuery fpDbQuery;

    QString statusQuery;

    //statusQuery.sprintf("DELETE FROM auto_scans WHERE status=0");

    //dbQuery.exec(statusQuery);

    sqlQuery = "SELECT vin FROM `registrations` WHERE vin NOT IN (SELECT vin from auto_scans) AND vin IN (SELECT registration_id from fp_fingerprints) ORDER BY int_created";

    dbQuery.exec(sqlQuery);

    emit(setTotalSize(dbQuery.size()));

    QDir::setCurrent(aggregateStoredMinutiaDir) ;

    QDir *testDir = new QDir();


    QFile *testXyt = new QFile;


    //    bozorthProcess.setProcessChannelMode(QProcess::SeparateChannels);
    bool continueInnerLoop = true;

    while(dbQuery.next()) {

        updateProcessStage(1);

        cleanTables();


//qDebug() << "Looping Query1";
        QString subjectVin = dbQuery.value(0).toString();
        fpSqlQuery.sprintf("SELECT minutia_link FROM fp_fingerprints WHERE registration_id='%s'",subjectVin.toUtf8().data());

//        qDebug() << fpSqlQuery;

        QStringList minutiaList;

        fpDbQuery.exec(fpSqlQuery);
        QString subjectXytFile;
        while(fpDbQuery.next()) {

            subjectXytFile.sprintf("%s/%s.xyt",subjectVin.toUtf8().data(), fpDbQuery.value(0).toString().toUtf8().data());

            if(testXyt->exists(subjectXytFile)){

                minutiaList.append(fpDbQuery.value(0).toString());

            }

        }

        int minutiaSize = minutiaList.size();

        if(minutiaSize==0){
            //goto Skip_Vin;
            emit(switchingSubjectVin());
            continue;
        }

        emit(currentSubjectVin(subjectVin));

        sqlQuery2.sprintf("SELECT vin FROM `registrations` WHERE vin <> '%s' AND vin in (select registration_id from fp_fingerprints) ORDER BY int_created",subjectVin.toUtf8().data());

        innerDbQuery.exec(sqlQuery2);

        emit(currentTargetSize(innerDbQuery.size()+1));

        QStringList folderList;

        while(innerDbQuery.next() && continueInnerLoop==true) {

            QString targetVin = innerDbQuery.value(0).toString();

            QString vinXyts;
            vinXyts.sprintf("%s/*.xyt",targetVin.toUtf8().data());

            if(testDir->exists(targetVin)){
                folderList.append(vinXyts);
            }

            emit(currentTargetVin(targetVin));

            if(folderList.size()==0){
                qDebug() << "2";
                continue;
            }


            //continueInnerLoop = false;
        }

        QFile bzScript("afis_scan.sh");

        bzScript.open(QFile::WriteOnly);

        QTextStream stream( &bzScript );


        QString bozorthCommand;

        for(int a=0;a<minutiaSize;a++) {

            QString sVinMinutia = minutiaList.at(a);

            //bozorthCommand.sprintf("bozorth3 -o output.score -e output.error -A outfmt=sg -p %s/%s.xyt %s", subjectVin.toUtf8().data(), tVinMinutia.toUtf8().data(),folderList.join(" ").toUtf8().data());
            bozorthCommand.sprintf("bozorth3 -A outfmt=pgs -p %s/%s.xyt %s", subjectVin.toUtf8().data(), sVinMinutia.toUtf8().data(),folderList.join(" ").toUtf8().data());

            stream << bozorthCommand << ";\n";

            //qDebug() << bozorthCommand;



        }

        bzScript.close();

        QProcess bozorthProcess;
        bozorthProcess.setWorkingDirectory(aggregateStoredMinutiaDir);
        bozorthProcess.setProcessChannelMode(QProcess::SeparateChannels);

        bozorthProcess.start("sh afis_scan.sh");



        if (!bozorthProcess.waitForFinished(-1)){
            qDebug() << "Failed :" << bozorthProcess.errorString();
            continue;
        } else {
            updateProcessStage(2);
            QString bozorthResult =  bozorthProcess.readAllStandardOutput();
            updateProcessStage(3);

            //emit(scanResult(bozorthResult));
            processScanResult(bozorthResult);
            // qDebug() << "bozorthResult: " <<bozorthResult;

        }


        //qDebug() << folderList.join(" ");
        emit(switchingSubjectVin());

        //      continueInnerLoop = false;
    }

    //QDateTime xcurrentSystemTime = QDateTime::currentDateTime();

    //int processEnded = xcurrentSystemTime.toTime_t();


    //qDebug() << "Started : " <<processStarted << " Ended: "<<processEnded << " = " << processEnded - processStarted;
  //  qDebug() << "Done!";
    emit(jobCompleted());
}






void aggregate_bozorth_check::processScanResult(QString  scanResult){
qDebug() << "Processing Scan Result";
    //Step 1: Explode By newline \n
    //Step 2: Explode by space
    //Step 3: Explode / symbol
    //Step 4: Remove .xyt suffix
    //Step 5: Do a getMaxScore check on the score

    //Step 1
    QStringList scanResultX = scanResult.split("\n");

    //Step 2
    int totalLines = scanResultX.size();
    //aqdebug() << "Total Lines " << totalLines;
    // return;

    QString sqlQuery;

    //    //aqdebug() << scanResultX.join(",");

    //    ui->pbCurrent->setMaximum(totalLines);
    //aqdebug() << "maximum set";

    try{
        for(int curLine=0;curLine < totalLines;curLine++) {

//            qDebug()<< "Starting at curLine " <<curLine;

            QString lineContentsStr = scanResultX.at(curLine);

//            qDebug()<< "got curLine " <<curLine;

            QStringList lineParts = lineContentsStr.split(" ");

            if( lineParts.size() == 3 ) {

                QString subjectVinPartsStr = lineParts.at(0);

                QStringList subjectVinParts = subjectVinPartsStr.split("/");

                QString subjectVin = subjectVinParts.at(0);

                QString subjectMin = subjectVinParts.at(1);
                subjectMin.replace(QString(".xyt"),QString(""));

                QString targetVinPartsStr = lineParts.at(1);

                QStringList targetVinParts = targetVinPartsStr.split("/");

                QString targetVin = targetVinParts.at(0);

                QString targetMin = targetVinParts.at(1);

                targetMin.replace(QString(".xyt"),QString(""));

                ////aqdebug() << " Line: " << lineContentsStr;

                ////aqdebug() << " subjectVin: " << subjectVin << " finger " << subjectMin << " targetVin " << targetVin << " targetMIn" << targetMin;
                QString simScore = lineParts.at(2);

                QSqlQuery dbQuery;

                sqlQuery.sprintf("INSERT INTO tmp_afis_scans(vin,to_vin,finger,to_finger,score) VALUES('%s','%s','%s','%s','%s')",subjectVin.toUtf8().data(),targetVin.toUtf8().data(),subjectMin.toUtf8().data(),targetMin.toUtf8().data(),simScore.toUtf8().data());
                ////aqdebug() << sqlQuery;
                dbQuery.exec(sqlQuery);
                //            ui->pbCurrent->setValue(curLine);

            }

        }
    }catch(...){
        qDebug() << "Bad error!!!"   ;
    }
    ///It's been quite difficult to sort it within c++, so I decided to take it outside
    qDebug() << "Extracting Result";
    updateProcessStage(4);

    extractRelevantData();

    updateProcessStage(5);

    //aqdebug() << "FINISHED PROCESS";
qDebug() << "Finished Scan Result";
}


void aggregate_bozorth_check::extractRelevantData(){

    //aqdebug() << "Started Extracting Relevant Data";

    QSqlQuery sortQuery;

    QSqlQuery dbQuery;

    QString insertSql;

    sortQuery.exec("SELECT * FROM `tmp_afis_scans` order by vin, to_vin, score,finger,to_finger DESC");

    QString fromVin,toVin,fromFinger,toFinger,similarityScore;

    QStringList fromVinsLst,toVinsLst  , fromFingLst,toFingLst;

    int scoreCol= sortQuery.record().indexOf("score");
    int vinCol= sortQuery.record().indexOf("vin");
    int toVinCol= sortQuery.record().indexOf("to_vin");
    int fingerCol= sortQuery.record().indexOf("finger");
    int toFingerCol= sortQuery.record().indexOf("to_finger");

    QString lastFromVin="0";

    while(sortQuery.next()) {

        similarityScore = sortQuery.value(scoreCol).toString();

        fromVin = sortQuery.value(vinCol).toString();

        toVin = sortQuery.value(toVinCol).toString();

        fromFinger = sortQuery.value(fingerCol).toString();

        toFinger = sortQuery.value(toFingerCol).toString();

        if( !fromVinsLst.contains(fromVin)) {

            //aqdebug() << "Not in fromVinsLst";
            toVinsLst.clear();
            fromFingLst.clear();
            toFingLst.clear();
            fromVinsLst.append(fromVin);
            QString statusQuery;
            statusQuery.sprintf("UPDATE auto_scans SET status=1 WHERE vin='%s'",lastFromVin.toUtf8().data());
            dbQuery.exec(statusQuery);


        }

        lastFromVin  = fromVin;

        if(!toVinsLst.contains(toVin)){
            //aqdebug() << "not in toVinsLst";
            fromFingLst.clear();
            toFingLst.clear();
            toVinsLst.append(toVin);

        }
        //aqdebug() << "checking for valid mixx";

        if(!fromFingLst.contains(fromFinger) && !toFingLst.contains(toFinger)){
            //aqdebug() << "found valid mixx";
            //means we have found a valid mix

            insertSql.sprintf("INSERT INTO auto_scans SET vin='%s', to_vin='%s', fingers=1, score='%s' ON DUPLICATE KEY UPDATE fingers=fingers+1, score=score+%s",
                              fromVin.toUtf8().data(), toVin.toUtf8().data(), similarityScore.toUtf8().data(), similarityScore.toUtf8().data()
                              );
            //aqdebug() << "running insert"            ;
            //            //aqdebug() << insertSql;
            dbQuery.exec(insertSql);
            //aqdebug() << "insert executed"            ;
            //aqdebug() << "appending to fromFingLst"            ;
            fromFingLst.append(fromFinger);
            //aqdebug() << "appending to toFingLst"            ;
            toFingLst.append(toFinger);
            //aqdebug() << "done appending as valid mixx";
        }
        //aqdebug() << "got HERE!!!"            ;
    }
    cleanTables();
    //aqdebug() << "Finished Extracting Relevant Data";

}


void aggregate_bozorth_check::cleanTables(){
    QSqlQuery dbQuery;
    dbQuery.exec("TRUNCATE TABLE tmp_afis_scans");

}


void aggregate_bozorth_check::updateProcessStage(int newStage){




    if(m_ui->continueAfis==false){
        emit(jobCancelled());
        exit(0);
    }

    emit(setProcessStage(newStage));

}
