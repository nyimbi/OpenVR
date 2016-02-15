#include "finger_thread.h"
#include "myfprint.h"

#include "QInputDialog"
#include <QtDebug>
#include <QSqlQuery>
#include <QtSql>
#include <QDir>
#include <QFile>
#include "QMessageBox"
#include "QProcess"

#include "QTimer"
QString currentCaptureLog;
/*
QString minDtctExe = "/opt/nbis/bin/mindtct";
QString bozorth3Exe = "/opt/nbis/bin/bozorth3";
QString storedMinutiaDir = "stored_minutiae";

int matchThreshold =50;
int matchMinutiae=25;
*/
struct fp_dev *dev;
int r = 1;

struct fp_dscv_dev *discover_device(struct fp_dscv_dev **discovered_devs)
{
    struct fp_dscv_dev *ddev = discovered_devs[0];
    struct fp_driver *drv;
    if (!ddev)
        return NULL;

    drv = fp_dscv_dev_get_driver(ddev);
    qDebug("Found device claimed by %s driver\n", fp_driver_get_full_name(drv));

    return ddev;
}

Finger_Thread::Finger_Thread(QObject *parent) : QObject(parent)
{
    continueCapture = true;
}

Finger_Thread::~Finger_Thread() {

    qDebug() << "Destroying Finger_Thread ";
    qDebug() << "Dev value: " << dev;

    if(!dev) {
        qDebug() << " Dev is true here ";
      //  fp_dev_close(dev);
//        fp_exit();
    }

    //fp_exit();

}

void Finger_Thread::captureFinger( ){

    this->isRunning = true;

    int a;

    struct fp_dscv_dev *ddev;
    struct fp_dscv_dev **discovered_devs;
    //struct fp_dev *dev;
    QString fingerName;

    const char *enrollFile;

    struct  fp_print_data *enrolled_print;

    addCaptureLog("Initializing the capture sequence");
    //sleep(1);


    r = fp_init();
    addCaptureLog("Initialized");

    if (r < 0) {
        addCaptureLog("Failed to initialize libfprint\n");
        exit(1);
    }

    discovered_devs = fp_discover_devs();

    if (!discovered_devs) {
        addCaptureLog("Could not discover devices\n");
        goto out;
    }

    ddev = discover_device(discovered_devs);

    if (!ddev) {
        addCaptureLog("No devices detected.\n Please click on Close, reconnect the fingerprint device, and try again");
        goto out;
    }
/* added this based on the fact that it throws an exception if you relaunch
   the tthread over and over again. This is still an issue. - 3pm, Sept 4
   */
  //  if(dev) fp_dev_close(dev);

    dev = fp_dev_open(ddev);
    qDebug() << "Dev: " <<dev;
    devLink = &*dev;

    fp_dscv_devs_free(discovered_devs);

    if (!dev) {
        //sleep(5);
        //goto beginning;
        addCaptureLog("Could not open device.\nPlease click on Close, reconnect the fingerprint device, and try again");
        goto out;

    }
    fp_dev_close(dev);

    if(fp_dev_supports_identification(dev)){
        //addCaptureLog("supports identification");
    }else{
        //addCaptureLog("does not support identification");
    }
    addCaptureLog("Opened device. It's now time to enroll your finger.");

    //timeOutWatch = new QTimer();
    //connect(timeOutWatch,SIGNAL(timeout()),this,SLOT(disconnectDevice()));
    //timeOutWatch->start(5000);
    //QTimer::singleShot(5000,this,SLOT(disconnectDevice()));

    for(a=1;a<=10;a++){
        //you can add the code for skipping fingers here.
        qDebug() << "continueCapture " << continueCapture;
        if(a < 1){//In the event that this happens
            a=1;
        }
        emit(threadSwitchFinger(a));
        if(!continueCapture){
            qDebug() << " ABORTING CAPTURE " ;
            emit(threadFinished(2));
            break;
        }
        switch(a){
        case 1:
            fingerName= "Right Hand Thumb";
            enrollFile = "templates/1.pgm";
            break;
        case 2:
            fingerName= "Right Hand Index Finger";

            enrollFile ="templates/2.pgm";

            break;

        case 3:
            fingerName= "Right Hand Middle Finger";
            enrollFile = "templates/3.pgm";
            break;

        case 4:
            fingerName= "Right Hand Ring Finger";
            enrollFile = "templates/4.pgm";
            break;

        case 5:
            fingerName= "Right Hand Little Finger (Pinkie)";
            enrollFile = "templates/5.pgm";
            break;
        case 6:
            fingerName= "Left Hand Thumb";
            enrollFile = "templates/6.pgm";
            break;
        case 7:
            fingerName= "Left Hand Index Finger";
            enrollFile = "templates/7.pgm";

            break;

        case 8:
            fingerName= "Left Hand Middle Finger";
            enrollFile = "templates/8.pgm";
            break;

        case 9:
            fingerName= "Left Hand Ring Finger";
            enrollFile = "templates/9.pgm";
            break;

        case 10:
            fingerName= "Left Hand Little Finger (Pinkie)";
            enrollFile = "templates/10.pgm";
            break;
        default:
            fingerName= "Unidentified Finger";
            enrollFile = "templates/0.pgm";

        }


        bool fingerEnrolled = false;

        do {
            struct fp_img *img = NULL;

            addCaptureLog("Scan your "+ fingerName + " now.");
            discovered_devs = fp_discover_devs();

            ddev = discover_device(discovered_devs);

            dev = fp_dev_open(ddev);

            fp_dscv_devs_free(discovered_devs);

            sleep(1);
            r =  fp_enroll_finger_img(dev, &enrolled_print, &img);
            sleep(1);
            if(!continueCapture){ goto out_close; }

            if(dev) fp_dev_close(dev);

            if (img) {

                fp_img_standardize(img);
                int minutiaCount=0;

                struct fp_minutia** minutiaList = fp_img_get_minutiae(img,&minutiaCount);
                qDebug()<< "Minutia Count: " <<minutiaCount << "List " <<minutiaList;
//                lastMunitiaCount = minutiaCount;

                QString minutiaLog ;
                minutiaLog.sprintf("Minutia Count: %d",minutiaCount);
                addCaptureLog(minutiaLog);

                struct fp_img *binaryImage = fp_img_binarize(img);

                fp_img_save_to_file(binaryImage, enrollFile);

                emit(fingerImageSaved(a));

                if(minutiaCount < 40){ //need to confirm what the actual minimum should be
                    a--;
                    //QProcess::startDetached("mplayer resources/sounds/failed.wav");
                    sleep(1);

                    addCaptureLog(" - - - PLEASE SCAN FINGER AGAIN. PLACE FINGER GENTLY ON DEVICE AND REMOVE AFTER 2 SECONDS - - -");
                    fp_img_free(img);

                    continue;
                }
                fingerEnrolled = true;

                emit(fingerScanned(enrollFile,a));
                //duplicateStatus = bozorth3Check(enrollFile,a);

                //qDebug() << "Duplicate Status: " <<duplicateStatus;
                /*
                if(duplicateStatus==true){

                    a--;

                    addCaptureLog(" - - - DUPLICATE FINGERPRINT ERROR.\n This finger has already been captured - - -");

                    //QMessageBox::critical(0, qApp->tr("DUPLICATE FINGERPRINT ERROR"),
                    //QProcess::startDetached("mplayer resources/sounds/failed.wav");

                    //qApp->tr("This finger has already been captured. "), QMessageBox::Cancel);
                    fp_img_free(img);
                    fingerEnrolled = false;
                    sleep(1);

                    continue;
                }
                */

                unsigned char *binaryData;

                size_t binaryDataSize;
                binaryDataSize = fp_print_data_get_data( enrolled_print,&binaryData);

                char binaryDataFile[50];
                sprintf(binaryDataFile,"templates/%d.bin",a);

                unsigned int binI;
                FILE *datei_ptr;
                //char dateiname[25];
                int ch;
                datei_ptr = fopen(binaryDataFile,"wb");
                if (datei_ptr == NULL) {
                    printf("Unable to Open File\n");
                } else {
                    //printf("Opening File\n");

                    for (binI=0;binI<binaryDataSize;binI++) {
                        ch = binaryData[binI];
                        fputc(ch, datei_ptr);
                    }

                    fclose(datei_ptr);
                }


                //addCaptureLog("Wrote scanned image");
                fp_img_free(img);
            }

            if (r < 0) {
                addCaptureLog("Enroll failed with error " + r);
            }

            switch (r) {
            case FP_ENROLL_COMPLETE:
                addCaptureLog("<" + fingerName + " check ");
                break;
            case FP_ENROLL_FAIL:
                addCaptureLog("Enroll failed, something went wrong");
                break;
            case FP_ENROLL_PASS:
                addCaptureLog("Enroll stage passed.");
                break;
            case FP_ENROLL_RETRY:
                addCaptureLog("Didn't quite catch that. Please try again.");
                break;
            case FP_ENROLL_RETRY_TOO_SHORT:
                addCaptureLog("Your swipe was too short, please try again.");
                break;
            case FP_ENROLL_RETRY_CENTER_FINGER:
                addCaptureLog("Didn't catch that, please center your finger on the "
                              "sensor and try again.");
                break;
            case FP_ENROLL_RETRY_REMOVE_FINGER:
                addCaptureLog("Scan failed, please remove your finger and then try "
                              "again.");
                break;
            }
        } while (r != FP_ENROLL_COMPLETE);

        if (!enrolled_print) {
            addCaptureLog("Enroll complete but no print?");
            goto out_close;
        }

        if(fingerEnrolled){
            addCaptureLog(fingerName + " enrollment  completed!");
            //QProcess::startDetached("mplayer resources/sounds/captured.wav");
        }
        addCaptureLog(" ->");
    }

    emit (threadFinished(0));
    return;

    out_close:
    qDebug("Calling fp_close");
    emit(threadFinished(-1));
    //if(!dev) fp_dev_close(dev);
    return;

    out:
    qDebug("Calling fp_exit");
    emit(threadFinished(-2));
    //fp_exit();


}

void Finger_Thread::addCaptureLog(QString newLog){

    currentCaptureLog+=newLog+"\n";

    qDebug() << " Log: " << newLog;
    emit(threadLog(newLog+ "\n"));

}

bool Finger_Thread::bozorth3Check(const char *filePath,int printIndex){
    return false;
    /*
    qDebug() << "Calling Bozorth3Check";
    QProcess bozorthProcess0;
    QProcess bozorthProcess;

    QString mindtctTarget;

    mindtctTarget.sprintf("templates/%d", printIndex);

    QString existingFiles;


    existingFiles.sprintf("rm -f templates/%d.xyt",printIndex);

    //qDebug() <<" exe code: " << existingFiles;

    QProcess::execute(existingFiles);

    QStringList mindtctArguments;

    QDateTime currentTime = QDateTime::currentDateTime();

    QString fullMindtctExe;

    fullMindtctExe.sprintf(" %s templates/%d ", filePath, printIndex );

    fullMindtctExe=minDtctExe+fullMindtctExe;

    mindtctArguments << filePath <<mindtctTarget;

    bozorthProcess0.setProcessChannelMode(QProcess::MergedChannels);
    qDebug() << fullMindtctExe;

    bozorthProcess0.start(fullMindtctExe);


    if (!bozorthProcess0.waitForFinished()){
        qDebug() << "Failed :" << bozorthProcess0.errorString();
        return false;
    } else {
        //QString mindtctCheckResult =  bozorthProcess0.readAll();
        //qDebug() << "mindtctCheckResult: " <<mindtctCheckResult;

    }

    bozorthProcess.setProcessChannelMode(QProcess::MergedChannels);

    QString fullBozorth3Exe;

    fullBozorth3Exe.sprintf(" -q -T %d -A minminutiae=%d -A outfmt=s -p ../templates/%d.xyt -G gallery.lis ",matchThreshold,matchMinutiae,printIndex);//need to figure out a cleaner way...

    fullBozorth3Exe = bozorth3Exe + fullBozorth3Exe;

    qDebug() << fullBozorth3Exe;

    QString currentDir = QDir::currentPath();

    QDir::setCurrent(storedMinutiaDir);

    bozorthProcess.start(fullBozorth3Exe);

    QDir::setCurrent(currentDir);

    if (!bozorthProcess.waitForFinished()){
        qDebug() << "Failed :" << bozorthProcess.errorString();

    } else {
        QString bozorthCheckResult =  bozorthProcess.readAll();

        qDebug() << "Match Munitiae: " << matchMinutiae;
        qDebug() << "Match Threshold: " << matchThreshold;
        qDebug() << "Minimum Munitiae Count: " << minimumMunitiaeCount;

        qDebug() << "Munitiae Count: " << lastMunitiaCount;
        qDebug() << "bozorthCheckResult: " <<bozorthCheckResult;
        qDebug() << "bozorthCheckResult (int): " <<bozorthCheckResult.toInt();

        if(bozorthCheckResult.size() > 0){
            //if(!bozorthCheckResult.is){

            return true;
        }

    }


    //Check for a smart one where the user enrolls the same finger twice
    //
    if(printIndex>1){
    QDir::setCurrent("templates");
    int previousIndex;
    bool isDuplicated = false;
    for(previousIndex=1;previousIndex<printIndex-1;previousIndex++) {

        emit threadLog("Match Threshold: " + matchThreshold);
        fullBozorth3Exe.sprintf(" -q -T %d -A minminutiae=%d -A outfmt=s %d.xyt %d.xyt ",matchThreshold,matchMinutiae,printIndex,previousIndex);//need to figure out a cleaner way...

        fullBozorth3Exe = bozorth3Exe + fullBozorth3Exe;

        qDebug() << "bozorth Full Exe: " << fullBozorth3Exe;
        emit threadLog("bozorth Full Exe: "+fullBozorth3Exe);


        bozorthProcess.start(fullBozorth3Exe);

        if (!bozorthProcess.waitForFinished()){
            qDebug() << "Failed :" << bozorthProcess.errorString();

        } else {
            QString bozorthCheckResult =  bozorthProcess.readAll();

            qDebug() << "bozorthCheckResult: " <<bozorthCheckResult;
            qDebug() << "bozorthCheckResult (int): " <<bozorthCheckResult.toInt();

            if(bozorthCheckResult.size() > 0){
                isDuplicated = true;
            }

        }


    }

    QDir::setCurrent(currentDir);
    return isDuplicated;


}
*/


    return false;

}

void Finger_Thread::disconnectDevice(){
    qDebug() <<"Trying to disconnect Device";
//    fp_dev_close(dev);

}
