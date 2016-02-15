#include "fingerdialog.h"
#include "ui_fingerdialog.h"
#include "fingerprintthread.h"
#include "bozorth_check.h"
//#include "finger_device_check.h"
#include "QDebug"
#include <QtCore>
#include <QProcess>
#include "QMessageBox"
#include "QFile"
#include "QSqlQuery"
#include "QSqlRecord"
#include "QSound"

QProcess *fingerProcess = new QProcess(); // The threaded fingerprint capture process
int minimumMinutia = 50; //default minimum

int silentMinimum=24;//To accept this as a minimum when several tries have been made


int isShuttingDown  = 0; //to inform the rest of the processes
bool deviceError; //True or false if device is not connected
int currentPrintIndex=0;//The current subject of the capture
QString fingerName; //The current subject name of the capture
QString bzMinDtctExe = "mindtct"; //redundant: link to xyt generator
QString bzBozorth3Exe = "bozorth3";//redundant: ;link to bozorth search
int bzMatchThreshold = 20;//redundant
int bzMatchMinutiae=40;//redundant
int maximumFailedScans = 3; //Number of failed scans, after which a failed dialog is cycled
int failedStats[6][3];// Put maximumFailedScans+1 as the 1d part very important

// Stylesheets
QString newMessageStyle = "background-color:rgb(255, 230, 230);";
QString waitMessageStyle = "background-color:rgb(235, 255, 230);";
QString successMessageStyle = "background-color:rgb(255, 155, 207);";
QString defaultMessageStyle = "background-color:rgb(255, 255, 255);";
QString errorMessageStyle = "background-color:rgb(255, 50, 50);";

int SOUND_LOW_QUALITY=0;
int SOUND_DUPLICATE_SCAN=1;
int SOUND_SUCCESS=2;
int SOUND_FAILED=3;




FingerDialog::FingerDialog(QWidget *parent,int argFingersToCapture,int argTotalPassesLeft,int argSensitivityId,QStringList argCapturableFingers) :
        QDialog(parent),
        ui(new Ui::FingerDialog)
{

    //    qDebug() << " fingerDeviceExists " << fingerDeviceExists();



    cfgFingersToCapture = argFingersToCapture;

    cfgTotalPassesLeft = argTotalPassesLeft;

    cfgCapturableFingers = argCapturableFingers;

    qDebug() << "Capture Passes Required " <<cfgTotalPassesLeft;
    qDebug() << "Fingers To Capture " << cfgFingersToCapture ;
    qDebug() << "Capturable Fingers " << cfgCapturableFingers;

    ui->setupUi(this);

    if(argTotalPassesLeft==1){
        ui->lblPassesLeft->setVisible(false);
        ui->lblLblPasses->setVisible(false);
    }

    ui->lblPassesLeft->setText(QString::number(argTotalPassesLeft).toStdString().c_str());

    ui->captureLog->setText("");
    closeDevice();//To close any unclosed devices
    deviceError = false;
    failedScans = 0;//to store the number of failed scans per cycle

    ui->actionButtonBox->hide();

    //let's retrieve the current sensitivity settings
    QSqlQuery dbQuery;
    QString sqlQuery;
    sqlQuery.sprintf("SELECT * FROM fingerprint_sensitivity WHERE id=%d",argSensitivityId);

    //argSensitivityId
    dbQuery.exec(sqlQuery);

    QSqlRecord settingRec = dbQuery.record();

    if(dbQuery.size()){ //else stay with the default

        dbQuery.next();

        int miniMinuCol = settingRec.indexOf("minimum_minutiae");
        //int thresholdCol = settingRec.indexOf("match_threshold");
        //int matchMinuCol = settingRec.indexOf("match_minutiae");

        //  bzMatchThreshold=dbQuery.value(thresholdCol).toInt();
        //  bzMatchMinutiae=dbQuery.value(matchMinuCol).toInt();
        minimumMinutia=dbQuery.value(miniMinuCol).toInt();

        qDebug() << " fp " << minimumMinutia;

    }


    for(int y=0;y<=maximumFailedScans;y++){

        failedStats[y][0]=0;

        failedStats[y][1]=0;

    }

    //QStringList capturableFingers;
//capturableFingers << "3" << "4" << "5";




    //Scan the first finger. Afterwards, this automatically calls the next finger ++
    scanFinger(1);

//    qDebug() << "-------Calling the end--------";

}


void FingerDialog::scanFinger(int printIndex) {

    if(!cfgCapturableFingers.isEmpty()) {

        QString curFinger = QString::number(printIndex).toStdString().c_str();

        qDebug() << "Capturable Fingers " << cfgCapturableFingers;

        if(!cfgCapturableFingers.contains(curFinger)) {

            qDebug() << "Skipping: " << curFinger;

            fingersCaptured = printIndex;

            printIndex++;

            if(printIndex <= cfgFingersToCapture){

                scanFinger(printIndex);
                fingersCaptured = printIndex;

            } else {

                this->close();

            }
            return;

        }
    }
    //Assign the subject to a global variable
    if(currentPrintIndex!=printIndex){
        clearMessages();
    }
    currentPrintIndex = printIndex;
    //Inform other processes that this capture process is still valid
    isShuttingDown = 0;

    //Start up a thread (pointed one)
    FingerPrintThread *fingerPrintThread = new FingerPrintThread();
    //Accept the end result from the thread
    connect(fingerPrintThread,SIGNAL(processCompleted(int,QString)),this,SLOT(fingerCaptured(int,QString)));
    //receive the error - device crashing from disconnection OR process timeout
    connect(fingerPrintThread,SIGNAL(deviceCrashed(int)),this,SLOT(restartScan(int)));

    //This displays the image guide for the hands
    QString fingerPrintFile;
    fingerPrintFile.sprintf("resources/finger_%d.jpg",printIndex);

    QFile fileReader(fingerPrintFile);
    if (fileReader.exists()){
        QImage image(fingerPrintFile);
        if(image.height()){
            if(printIndex<=5){//Right Hand Side Images

                ui->lblHandPreview->setPixmap(QPixmap::fromImage(image));

            }else{//Left Hand Side Images
                if(printIndex==6){//Leave/Display the RHS image  as all green :D
                    QImage greenImage("resources/finger_5_r.jpg");
                    ui->lblHandPreview->setPixmap(QPixmap::fromImage(greenImage));
                }
                ui->lblHandPreviewLeft->setPixmap(QPixmap::fromImage(image));

            }
        }
    }else{
        qDebug() << fingerPrintFile << " does not exist";
    }
    //Start an instance of the finger print thread
    //We are using a thread because otherwise,t he gui will hang while waiting for a response
    future = QtConcurrent::run(fingerPrintThread, &FingerPrintThread::captureFinger,printIndex);

    switch(printIndex){//Could not find a better way than this :)
    case 1:
        fingerName= "Right Hand Thumb";
        break;
    case 2:
        fingerName= "Right Hand Index Finger";
        break;

    case 3:
        fingerName= "Right Hand Middle Finger";
        break;

    case 4:
        fingerName= "Right Hand Ring Finger";
        break;

    case 5:
        fingerName= "Right Hand Little Finger";
        break;
    case 6:
        fingerName= "Left Hand Thumb";
        break;
    case 7:
        fingerName= "Left Hand Index Finger";
        break;

    case 8:
        fingerName= "Left Hand Middle Finger";
        break;

    case 9:
        fingerName= "Left Hand Ring Finger";
        break;

    case 10:
        fingerName= "Left Hand Little Finger";
        break;
    default://Just in case, but this should really never happen
        fingerName= "Unidentified Finger";
    }
    //Send a text based message to the screen
    QString displayLog = "Please place your " +fingerName.toUtf8()+ " on the device now";
    displayMessage(displayLog,1);

}

FingerDialog::~FingerDialog()
{
    delete ui;
}

void FingerDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void FingerDialog::addCaptureLog(QString newLog){

    qDebug()<< newLog;

}

void FingerDialog::closeEvent(QCloseEvent *evt){
    //If the window is being closed, then close the device, and inform other processes of the new status
    //qDebug() << "Close Event: " << evt << " fingers captured " << fingersCaptured;

    closeDevice();

    isShuttingDown = 1;

    emit(isClosing(fingersCaptured));
    closeDevice();
    //just to be double sure
    QProcess *killFingerProcess = new QProcess();

    killFingerProcess->start("scripts/close_fp.sh");
    killFingerProcess->waitForFinished();

}

void FingerDialog::restartScan(int printIndex){
    //Restart the capture only if the window is NOT being closed, else we end up leaving the scanner waiting to enroll
    if(isShuttingDown==1) return;

    closeDevice();

    scanFinger(printIndex);

}
void FingerDialog::reject(){
    emit(isClosing(fingersCaptured));
    closeDevice();
    //this->close();
}

void FingerDialog::closeDevice() {
    //Force a close on the device
    try {

        FingerPrintThread *fingerPrintThread = new FingerPrintThread();
        fingerPrintThread->toggleRecovery(false);

        fingerProcess->startDetached("scripts/close_fp.sh");


    } catch(...) {

        qDebug() << "Unable to close properly";
    }

}

void FingerDialog::on_btnCancelCapture_clicked()
{
    //Cancel the capture process
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setBackgroundRole(QPalette::Base);
    msgBox.setAutoFillBackground(true);
    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setText("Are you sure you want to cancel this capture?");
    msgBox.setWindowTitle("Cancel Capture");
    msgBox.setParent(this);

    switch (msgBox.exec()) {

    case QMessageBox::Yes:

        emit(cancelCaptureRequested());
        this->close();
        break;

    }

}

void FingerDialog::hideEvent(QHideEvent * evt){
    //qDebug() << evt;
    // closeDevice();

}

void FingerDialog::startCapture(){

}

void FingerDialog::fingerCaptured(int printIndex,QString processResult){

//    displayMessage("Checking. Please wait",3);
    //bool stillChecking = true;
    QStringList processOutput = processResult.split(" ");
    int trueBestMinutia=0;
    int trueBestScan=0;
    /**
    Ideally, the output from the process via signal will be two words (two numbers)
    e.g 909 56
        909 is the device/enrollment response, in this case 909 is success
        56 is the minutia count of the scan
    */

    QString enrollResponse = processOutput.at(0);
    QString log;
    int intResponse = (int) enrollResponse.toInt();
    qDebug() << " enrollResponse " << intResponse;

    closeDevice();
    //QString displayLog = "Checking "+fingerName.toUtf8()+"  image. Please wait";
    //displayMessage(displayLog);

    if(intResponse==909) { //Enrollment Complete

        QString minutiaResponse = processOutput.at(1);//always comes with a minutia count

        int minutiaCount = (int) minutiaResponse.toInt();//type casting

        QString normalFingerPrintFile = "enrolled_normal.pgm"; //by default

        QString fingerPrintFile = "enrolled_binarized.pgm"; //by default

        //Display a preview of the scanned image, binarized
        QFile fileReader(fingerPrintFile);
        if (fileReader.exists()){
            QImage image(fingerPrintFile);
            if(image.height()){
                ui->lblFingerPreview->setPixmap(QPixmap::fromImage(image));
            }
        }else{
            qDebug() << fingerPrintFile << " does not exist";
        }

        //Remove the destination file (old), the copy the new one there
        QString newEnrollFile;

        newEnrollFile.sprintf("templates/%d.pgm",printIndex);//For the normal version

        QString newBinEnrollFile;

        newBinEnrollFile.sprintf("templates/%d.bin",printIndex);//For the binarized version

        qDebug() << " Moving " << normalFingerPrintFile << " to " << newEnrollFile;

        QFile cFileReader(normalFingerPrintFile);

        if(!cFileReader.exists()){

            qDebug() << "Unable to find " << normalFingerPrintFile;

        } else {
            qDebug() << "Found, ready to move";
            cFileReader.remove(newEnrollFile);
            cFileReader.copy(normalFingerPrintFile,newEnrollFile);
            cFileReader.remove(newBinEnrollFile);
            cFileReader.copy(fingerPrintFile,newBinEnrollFile);
        }

        //Check the quality & minutiaCount


        int fingerPrintQuality = printQuality(newEnrollFile);

        if( minutiaCount < minimumMinutia || fingerPrintQuality > 2) {



            failedScans++;//the number of failed scans on this finger
            qDebug() << "Failed Scans: " << failedScans;
            failedStats[failedScans][0] = minutiaCount;
            failedStats[failedScans][2] = fingerPrintQuality;


            QFile saveScan;
            QString savedScanFile;
            savedScanFile.sprintf("templates/failed_scan_%d.pgm",failedScans);
            saveScan.copy(newEnrollFile,savedScanFile);

            savedScanFile.sprintf("templates/failed_scan_%d.bin",failedScans);
            saveScan.copy(newBinEnrollFile,savedScanFile);

            emit(failedToScanSignal(printIndex,minutiaCount,fingerPrintQuality));

            //qDebug() << "Here now";

            audibleSound(SOUND_FAILED);

            if(failedScans >= maximumFailedScans) {

                failedScans=0;

                qDebug()<< "Trying to automatically accept Best Scan ";

                trueBestMinutia=0;
                trueBestScan=0;

                for(int fFingerScan=1;fFingerScan<=maximumFailedScans;fFingerScan++){

                    qDebug()<< "Scan " << fFingerScan << " Minutia: " << failedStats[fFingerScan][0] << " Quality: "<< failedStats[fFingerScan][2];

                    if(failedStats[fFingerScan][0] < silentMinimum){
                        continue;
                    }

                    if(failedStats[fFingerScan][0] > trueBestMinutia && failedStats[fFingerScan][2]<=2){
                        trueBestMinutia = failedStats[fFingerScan][0];
                        trueBestScan = fFingerScan;
                        fingerPrintQuality = failedStats[fFingerScan][2];
                    }

                }

                if(trueBestMinutia==0){

                    qDebug() << "No True Best Scan Found";

                    for(int y=1;y<=maximumFailedScans;y++){
                        failedStats[y][0]=0;
                        failedStats[y][1]=0;
                        failedStats[y][2]=5;
                    }

                    log = "Please scan your "+ fingerName.toUtf8() +" again";
                    displayMessage(log,0);
                    scanFinger(printIndex);
                    return;

                }

                qDebug()<< "Best Scan " << trueBestScan << " with " << trueBestMinutia << " minutia count";

                //So I need to copy it backwards

                savedScanFile.sprintf("templates/failed_scan_%d.pgm",trueBestScan);
                saveScan.copy(savedScanFile,newEnrollFile);

                savedScanFile.sprintf("templates/failed_scan_%d.bin",trueBestScan);
                saveScan.copy(savedScanFile,newBinEnrollFile);

                for(int y=1;y<=maximumFailedScans;y++){
                    failedStats[y][0]=0;
                    failedStats[y][1]=0;
                }


            } else {

                displayMessage("Scan Failed",0);
                log = "Please scan your "+ fingerName.toUtf8() +" again";
                displayMessage(log,0);
                scanFinger(printIndex);
                return;
            }

        }

        //and if this finger has been scanned before
        //0 means no problems at all
        //1 means duplicate scan

        int doubleScanResult = isDoubleScanned(printIndex,newEnrollFile);

        //failedStats[failedScans][2] = doubleScanResult; //not sure if this is still relevant at this point

        qDebug() << " Double Scan Result " << doubleScanResult;

        if(doubleScanResult == 1 ) {//Duplicate Scan!

            failedScans = 0;

            audibleSound(SOUND_DUPLICATE_SCAN);
            QString displayLog= "Duplicate Scan Detected. You cannot capture the same finger more than once";
            displayMessage(displayLog,0);
            scanFinger(printIndex);
            return;

        }


        QString displayLog= fingerName.toUtf8() + " scanned successfully.";

        for(int y=1;y<=maximumFailedScans;y++) {
            failedStats[y][0]=0;
            failedStats[y][1]=0;
        }

        displayMessage(displayLog,2);

        qDebug()<< "Signal Sent About enrollFile: " << newEnrollFile;

        //qDebug() << "Calling Bozorth3Check";

        fingerPrintQuality = printQuality(newEnrollFile);

        emit(successfulFingerScan(printIndex,minutiaCount,fingerPrintQuality));

        emit(initiateBozorthCheck(newEnrollFile, printIndex,minutiaCount,minimumMinutia,fingerPrintQuality));

        fingersCaptured = printIndex;

        audibleSound(SOUND_SUCCESS);

        printIndex++;

        if(printIndex > cfgFingersToCapture) { //time to close since I have captured all I'm required to do
            //show the close button
            ui->actionButtonBox->show();

            //set the focus on the close button
            ui->actionButtonBox->setFocus();
            //hide the cancel button
            ui->btnCancelCapture->hide();


            QString fingerPrintFile= "resources/finger_10_l.jpg";

            QFile fileReader(fingerPrintFile);
            if (fileReader.exists()) {
                QImage image(fingerPrintFile);
                if(image.height()) {
                    ui->lblHandPreviewLeft->setPixmap(QPixmap::fromImage(image));
                }
            } else {
                qDebug() << fingerPrintFile << " does not exist";
            }
            this->close();
            return;
        }
        failedScans=0;// reset the number of failed scans to 0

        scanFinger(printIndex);


    } else {
        switch(intResponse) {//since it failed

        case -100:
        case -101:
        case -102:
            log = "A device error occurred while scanning. Please reconnect the fingerprint device and click on restart scan";
            displayMessage(log,0);
            deviceError  = true;
            closeDevice();
            break;
        case -103:
            log = "You moved your finger too fast on the device. Please try again";
            displayMessage(log,0);
            scanFinger(printIndex);
            break;
        case -105:
            log = "Invalid scan. Please try again";
            displayMessage(log,0);
            scanFinger(printIndex);
            break;
        case 0:
            log = "- - -";
            displayMessage(log,0);
            deviceError  = true;
            break;
        case -22:
            closeDevice();
            log = "Please prepare to rescan your finger";
            displayMessage(log,0);
            scanFinger(printIndex);

            break;
        default:
            closeDevice();
            restartScan(printIndex);
            log = "Please scan your " + fingerName.toUtf8() + " finger again.";
            displayMessage(log,1);

        }

        //        log = "Please scan your finger again.";
        //        displayMessage(log);
    }

}

void FingerDialog:: displayMessage(QString message,int messageType){

    //It simply displays a message to the user
    QString newLog = ui->captureLog->toHtml()+"<b>"+message.toUtf8()+"</b><br />";
    //QString newLog = "\r\n"+message.toUtf8();
    ui->captureLog->setHtml(newLog);
    QTextCursor c  = ui->captureLog->textCursor();
    c.movePosition(QTextCursor::End);
    ui->captureLog->setTextCursor(c);
    //ui->inpDobMonth->setStyleSheet(invalidInputStyle);
    switch(messageType){
    case 0:
        ui->captureLog->setStyleSheet(errorMessageStyle);
        break;
    case 1:
        ui->captureLog->setStyleSheet(newMessageStyle);
        break;
    case 2:
        ui->captureLog->setStyleSheet(successMessageStyle);
        break;
    case 3:
        ui->captureLog->setStyleSheet(waitMessageStyle);
        break;
    }

    //QTimer::singleShot(1250,this,SLOT(unflashVisualLog()));

    qDebug() << " Log: " <<message;

}
void FingerDialog::clearMessages(){

    ui->captureLog->setText("");
}


void FingerDialog::on_actionButtonBox_clicked()
{
    //Close the window
    this->close();

}



int FingerDialog::isDoubleScanned(int printIndex,QString newEnrollFile){

    /**

  Checks to see if this finger has already been scanned during this capture process
  and also verifies the image quality
  */
    qDebug() << "Checking " << newEnrollFile;
    int isDuplicated = 0;
    if(printIndex > 1) {

        QProcess *bozorthProcess = new QProcess;
        QString fullBozorth3Exe;
        fullBozorth3Exe.sprintf("scripts/duplicate_scan.sh enrolled_binarized.pgm %d %d %d",printIndex,bzMatchThreshold,bzMatchMinutiae);
        qDebug()<< "Bozorth EXE" << fullBozorth3Exe;
        bozorthProcess->start(fullBozorth3Exe);

        if (!bozorthProcess->waitForFinished()){
            qDebug() << "Failed :" << bozorthProcess->errorString();

        } else {
            QString bozorthCheckResult =  bozorthProcess->readAllStandardOutput();

            qDebug() << "bozorthCheckResult: " <<bozorthCheckResult;
            qDebug() << "bozorthCheckResult (int): " << bozorthCheckResult.toInt();

            if(bozorthCheckResult.toInt() > 0){
                isDuplicated = 1;
            }else if(bozorthCheckResult.toInt() == -1){
                isDuplicated = -1;
            }

        }

    }

    return isDuplicated;
}



int FingerDialog::printQuality(QString newEnrollFile){
    /**

    Returns the image quality
  */

    QProcess *qualityProcess = new QProcess;
    QString nfiqExe= "nfiq "+ newEnrollFile.toUtf8();

    qualityProcess->start(nfiqExe);

    if (!qualityProcess->waitForFinished()){
        qDebug() << "Failed :" << qualityProcess->errorString();

    } else {
        QString qualityCheckResult =  qualityProcess->readAllStandardOutput();

        qDebug() << "qualityCheckResult: " <<qualityCheckResult;
        qDebug() << "qualityCheckResult (int): " << qualityCheckResult.toInt();

        return qualityCheckResult.toInt();

    }


    return 0;
}



void FingerDialog::on_btnRestartDevice_clicked()
{
    //Just llike it says - it restarts the scanning of the current finger
    closeDevice();

    restartScan(currentPrintIndex);

}

void FingerDialog::unflashVisualLog(){
    ui->captureLog->setStyleSheet(defaultMessageStyle);
}

void FingerDialog::audibleSound(int messageType){

    QString soundFile = "no_sound.wav";
    switch(messageType) {
        case 0:
            soundFile="low_quality.wav";
            break;
        case 1:
            soundFile="duplicate_scan.wav";
            break;
        case 2:
            soundFile="success.wav";
            break;
        case 3:
            soundFile="failed.wav";
            break;
    }

    QString playExe = "mplayer -really-quiet resources/sounds/"+soundFile.toUtf8()+ " 2>&-";
    QProcess::startDetached(playExe);

}
