

//#include <QtGui>
#include <QtSql>
#include <QMessageBox>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QByteArray>
#include <QProcess>
#include <QImage>
#include <QInputDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSound>
#include <QUuid>

//#include <QBuffer>
// For MD5 Hash
#include <QCryptographicHash>

// For Printing
#include <QPrintPreviewDialog>

#include <QPrintDialog>

#include <QPrinter>

#include <QPainter>

#include <QFileDialog>

// Barcode
#include "qzint.h"

// Search Records & Print

#include "recordsdialog.h"

#include "mainwindow.h"
#include "queuetickets.h"
#include "myeventfilter.h"

#include "ui_mainwindow.h"
#include "fingerdialog.h"
#include "bozorth_check.h"


#include "capturefingerdialog.h"
#include "capturephotodialog.h"
#include "helpdialog.h"

#include "controlpanel.h"
//#include "myfprint.h"

//add this header for the time fxn
#include <cstdlib>

#include "specialenrollment.h"

//Headers for WebCam
#include "capturephotodialog.h"
#include <QFileDialog>

#include "devicecheck.h"
#include <QtCore>

QString deviceOkStatusStyle = "border:1px solid grey; background-color:rgb(208, 255, 179);padding:10px;";
QString deviceErrStatusStyle = "border:1px solid grey; background-color:rgb(255, 196, 196);padding:10px;";


// For Asset Tag
//QString assetTag="434E4637-3530-375A-4E30-001B24F22553";// this should be automatically picked from the system
QString assetTag;
QString assetTagFilename = "/AssetTag.txt";
QString macAddr;
QString macAddrFilename =  "/MacAddress.txt";

QString authPassword = "password";//until we decide where to keep this
QString operatorCodeValue="";

QString delimState;
QString delimLga;
QString delimRa;
QString delimPu;
QString delimStateAbbr;
QString delimLgaAbbr;
QString delimRaAbbr;
QString delimPuAbbr;
QString delimitationText;
QString delimitationValue;

// Printing
//int selectedRecord;
//QStringList selectedRecords;  // Contains IDs of all Selected Records;

QProcess *cameraProcess;
QPushButton *btnSave;
QLineEdit *inpLastName;
QLineEdit *inpFirstName;
QLineEdit *inpMiddleName;

QComboBox *inpDobDay;
QComboBox *inpDobMonth;
QComboBox *inpDobYear;
QComboBox *inpGender;
QComboBox *inpOccupation;
QPlainTextEdit *inpAddress;
QComboBox *inpState;
QComboBox *inpLga;
QComboBox *inpWard;
QLineEdit *inpMobile;
QLabel *lblVin;
QLabel *lblPhotoFront;
QLCDNumber *lcdRegistrations;
QSqlRelationalTableModel *dbModel;
QSqlQuery dbQuery;


QFile fileReader;
QString fingerPrintFile = "enrolled.pgm";
QString vinValue;
QString numRows;
int minimumAge = 18;
int maximumAge = 120;


QByteArray leftPrintHolder;
QByteArray leftPrintHolderImg;

QByteArray rightPrintHolder;
QByteArray rightPrintHolderImg;

QByteArray nullPrintHolder;
QByteArray  frontPhotoHolder;
QByteArray  leftPhotoHolder;
QByteArray  rightPhotoHolder;
bool isAuthorizedOperator;

int allowedIdleTime = 10;//in secs

QTimer *lastActivityTimer;   //To handle "idle" time and launch a "screensaver"
int lastActivity;   //To handle "idle" time and launch a "screensaver"


int bozorthMatches[4000][2];
int bozorthRunningThreads=0;
int fingerprintAttributes[11][3];
int bzGalleryMaximum = 4;
int bzScoreMaximum = 168;
bool duplicateFingerPrintSet = false;
bool fingerPrintsCaptured = false;
QTimer *statusBarUpdater;
int fingersToCapture = 10;
int totalPassesRequired  = 1;
int fpPassesMade = 1;//leave alone please!!!
int fpScanResults[10][11][3];// 1=>Pass 2=>PrintIndex 3=> 0=>minutiaCount,1=>quality,2=>FailedCount
int fpFailedResults[10][11][30][2];// 1=>Pass 2=>PrintIndex 3=> 0=>minutiaCount,1=>quality,2=>FailedCount
int fpScanFailed[11];//1=>printIndex 2=>failedCount
QStringList capturableFingers;
bool isSpecialRegistration = false;
bool skipSpecialRegistration = false;
QString isSpecialNotes="";
QString isSpecialGroup="";

// Stylesheets
QString invalidInputStyle = "background-color:rgb(255, 207, 207);";
QString onFocusStyle = "background-color:rgb(255, 253, 185);";
QString onBlurStyle = "background-color:rgb(255, 255, 255);";

QString successMsgStyle = "background-color:rgb(255, 155, 207);";
QString defaultMsgStyle = "background-color:rgb(255, 255, 255);";
QString errorMsgStyle = "background-color:rgb(255, 0, 0);";




int globalProcessTimer[6][2];
int PROCESS_DATA=1;
int PROCESS_FINGERPRINT=2;
int PROCESS_PHOTO=3;
int PROCESS_PRINT=4;
int PROCESS_START = 0;
int PROCESS_END = 1;

int cfgFingerSensitivityId;
int cfgPollingUnitId;
int cfgLastMinutia;
QString cfgVinPrefix;
int cfgLastVin;





MainWindow::MainWindow(QWidget *parent,QString loggedOperator)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    operatorCodeValue = loggedOperator;

    //Set the focus of the cursor to the last name field on launch
    QTimer::singleShot(0,ui->inpLastName,SLOT(setFocus()));

    //Get the serial number/asset tag of this machine
    getAssetTag();
    getMACAddress();
    getCurrentSettings();

    //count the total number of records in database
    reCountRegistrations();

    setupStatusBar();

    recordsDialog = new RecordsDialog(this);
    reportsDialog = new ReportsDialog(this);

    connect(ui->actionPrintReports, SIGNAL(triggered()), reportsDialog, SLOT(exec()));
    connect(ui->actionPrintVotersCard, SIGNAL(triggered()), recordsDialog, SLOT(exec()));

    //    connect(ui->btnSave, SIGNAL(clicked()), this, SLOT( previewInfo() ) );
    //    connect(ui->previewVC, SIGNAL(clicked()), this, SLOT( previewInfo() ) );

    // For Realtime Validation
    connect(ui->inpLastName, SIGNAL(textChanged(QString)), this, SLOT(processLastName(QString)));
    connect(ui->inpFirstName, SIGNAL(textChanged(QString)), this, SLOT(processFirstName(QString)));
    connect(ui->inpMiddleName, SIGNAL(textChanged(QString)), this, SLOT(processMiddleName(QString)));
    // Address & Mobile Listener in clearForm(); to prevent it from highlighting fields on load.
    connect(ui->inpDobDay, SIGNAL(textChanged(QString)), this, SLOT(processDobDay(QString)));
    connect(ui->inpDobMonth, SIGNAL(textChanged(QString)), this, SLOT(processDobMonth(QString)));
    connect(ui->inpDobYear, SIGNAL(textChanged(QString)), this, SLOT(processDobYear(QString)));


    connect(ui->inpLastName, SIGNAL(editingFinished()), this, SLOT(lastNameOnBlur()));
    connect(ui->inpFirstName, SIGNAL(editingFinished()), this, SLOT(firstNameOnBlur()));
    connect(ui->inpMiddleName, SIGNAL(editingFinished()), this, SLOT(middleNameOnBlur()));
    connect(ui->inpAddress, SIGNAL(cursorPositionChanged()), this, SLOT(addressOnBlur()));
    connect(ui->inpDobDay, SIGNAL(editingFinished()), this, SLOT(dobDayOnBlur()));
    connect(ui->inpDobMonth, SIGNAL(editingFinished()), this, SLOT(dobMonthOnBlur()));
    connect(ui->inpDobYear, SIGNAL(editingFinished()), this, SLOT(dobYearOnBlur()));
    connect(ui->inpAge, SIGNAL(editingFinished()), this, SLOT(dobAgeOnBlur()));
    connect(ui->inpMobile, SIGNAL(editingFinished()), this, SLOT(mobileOnBlur()));


    DeviceCheck *devStatusChecker = new DeviceCheck();
    connect(devStatusChecker, SIGNAL(backupStatusUpdate(int)), this, SLOT(backupStatusChanged(int)));
    connect(devStatusChecker, SIGNAL(fpDeviceStatusUpdate(int)), this, SLOT(fpStatusChanged(int)));
    connect(devStatusChecker, SIGNAL(cameraStatusUpdate(int)), this, SLOT(cameraStatusChanged(int)));
    connect(devStatusChecker, SIGNAL(printerStatusUpdate(int)), this, SLOT(printerStatusChanged(int)));
    devFuture = QtConcurrent::run(devStatusChecker, &DeviceCheck::checkTimer);

    this->showFullScreen();
    //SET TIMER INTERVAL -- constantly capture key events
    lastActivityTimer = new QTimer();

    resetIdleTimeWatcher();

    clearForm();

}

void MainWindow::reCountRegistrations()
{
    QSqlQuery query;
    
    //    rDeturn;
    query.exec("SELECT COUNT(*) total_records FROM registrations");
    //int fieldNo = query.record().indexOf("total_records");
    query.next();
    
    //        qDebug() << "Count: " <<fieldNo;
    numRows = query.value(0).toString();
    //Update the total registrations in the UI
    ui->lcdRegistrations->display(numRows);
    
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::capturePhoto(){


    globalProcessTimer[PROCESS_PHOTO][PROCESS_START] = getCurrentTime();

    bool cameraFound = false;
    QFile cameraChecker;
    if(cameraChecker.exists("/dev/video1")){

        cameraFound = true;

    }else if(cameraChecker.exists("/dev/video0")){

        cameraFound  = true;

    }

    if(cameraFound  == false){
        QMessageBox::critical(this,"No Camera Device","No Camera Detected.");
        return;
    }
    CapturePhotoDialog *captureDialog = new CapturePhotoDialog(this);
    captureDialog->show();

    if (captureDialog->exec() != 0){
        qDebug() << "for some reason, this failed";
    }

    globalProcessTimer[PROCESS_PHOTO][PROCESS_END] = getCurrentTime();

    //Use OpenCV to reload image as QImage Sucks!!!
    char filename[100]="photos/selected_photox.jpg";

    QFile fileReader(filename);

    if (fileReader.exists()) {
        IplImage* dummy=cvLoadImage(filename,1);
        QImage image(QSize(dummy->width,dummy->height),QImage::Format_RGB32);

        //Image by OpenCV has 3 channels, we want to make it compatible with QPixMap.
        int cvIndex = 0, cvLineStart = 0;
        for (int y = 0; y < dummy->height; y++) {
            unsigned char red,green,blue;
            cvIndex = cvLineStart;
            for (int x = 0; x < dummy->width; x++) {
                // DO it
                red = dummy->imageData[cvIndex+2];
                green = dummy->imageData[cvIndex+1];
                blue = dummy->imageData[cvIndex+0];

                //Modify the QImage
                image.setPixel(x,y,qRgb(red, green, blue));
                cvIndex += 3;
            }
            cvLineStart += dummy->widthStep;
        }

        //Update label and scale image to it.

        ui->lblPhotoFront->setPixmap(QPixmap::fromImage(image));

        ui->lblPhotoFront->setScaledContents(true);

        //        lblPhotoFront->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);

        fileReader.open(QFile::ReadOnly);
        frontPhotoHolder = fileReader.readAll();
        fileReader.close();
    }
}

void MainWindow::clearFingerPrints(){

    /* This will still run even if we were not displaying all 10 fingers. */

    //fpResetCapturableFingers();


    //clearBozorthTable();

    QString defaultText = "-None-";

    ui->lblFinger1->setText("Thumb");
    ui->lblFinger2->setText("Index");
    ui->lblFinger3->setText("Middle");
    ui->lblFinger4->setText("Ring");
    ui->lblFinger5->setText("Little");
    ui->lblFinger6->setText("Thumb");
    ui->lblFinger7->setText("Index");
    ui->lblFinger8->setText("Middle");
    ui->lblFinger9->setText("Ring");
    ui->lblFinger10->setText("Little");
    QProcess templateCleaner;

    QStringList rmCommandList,mkdirCommandList;

    rmCommandList << "-rf" << "templates" ;
    templateCleaner.execute("scripts/clearfingerprints.sh");




    duplicateFingerPrintSet  = false;

    //reset this
    for(int y=0;y < 4000; y++){
        bozorthMatches[y][0]=0;
        bozorthMatches[y][1]=0;
    }
    bozorthRunningThreads = 0;

    for(int y=1;y< 11; y++){
        fingerprintAttributes[y][0]=0;
        fingerprintAttributes[y][1]=0;
        fingerprintAttributes[y][2]=0;
    }
}


int MainWindow::getCurrentTime(){

    QDateTime currentTime = QDateTime::currentDateTime();

    return currentTime.toTime_t();

}

QByteArray MainWindow::capturePrint(){

    fpPassesMade = 1;//resetting this

    globalProcessTimer[PROCESS_FINGERPRINT][PROCESS_START]  = getCurrentTime();

    if(bozorthRunningThreads>0) {
        QMessageBox::warning(this, "Processing","The system is processing the previously captured fingerprints.", QMessageBox::Ok);
        return nullPrintHolder;

    }

    ui->lblBozorthProcess->setStyleSheet(invalidInputStyle);
    int finger;
    
    QString fileName;
    
    //Clear previous capture files
    //fpScanResults= defaultFpScanResults;

    if(totalPassesRequired == 1){

        clearFingerPrints();

        //let's get the setting for this pass

        QSqlQuery dbQuery;
        dbQuery.exec("SELECT setting_value FROM settings WHERE setting_name='fingerprint_sensitivity_id'");

        QSqlRecord settingRec = dbQuery.record();

        if(!dbQuery.size()){
            qDebug() << "Aborting Fingerprint Capture. Unable to get the current pass setting";
            return nullPrintHolder;
        }

        dbQuery.next();

        int currentPassSetting = settingRec.indexOf("setting_value");

        int currentSensitivityId=dbQuery.value(currentPassSetting).toInt();

        FingerDialog *fingerDialog = new FingerDialog(this,fingersToCapture,totalPassesRequired,currentSensitivityId,capturableFingers);

        ui->btnCaptureLeft->setEnabled(false);
        ui->btnSpecial->setEnabled(false);



        connect(fingerDialog,SIGNAL(initiateBozorthCheck(QString,int,int,int,int)),this,SLOT(initiateBozorthCheck(QString,int,int,int,int)));
        connect(fingerDialog,SIGNAL(isClosing(int)),this,SLOT(confirmFingersCaptured(int)));

        connect(fingerDialog,SIGNAL(rejected()),this,SLOT(closeFingerDevice()));

        fingerDialog->exec();

        bozorthRunningThreads--; //to get it to minus -1 when the last one is completed
        if(bozorthRunningThreads==-1){
            compileResults();
        }

    } else { //if totalPassesRequired > 1, then don't bozorthCheck. Instead move files around!!!


        QSqlQuery dbQuery;
        dbQuery.exec("SELECT * FROM fingerprint_sensitivity order by id");

        QSqlRecord settingRec = dbQuery.record();

        if(!dbQuery.size()){
            qDebug() << "Aborting Fingerprint Capture. Unable to get the any pass setting";
            return nullPrintHolder;
        }

        QString msg;
        msg.sprintf("You are required to capture %d fingers %d times\n Passes Left: %d",fingersToCapture,totalPassesRequired,totalPassesRequired);
        QMessageBox::information(this, "Multiple Passes Required",msg, QMessageBox::Ok);

        while (dbQuery.next() && fpPassesMade <= totalPassesRequired ) {

            if( fpPassesMade <= 1 ) {
               clearFingerPrints();
            }

            int totalPassesLeft = totalPassesRequired - fpPassesMade;

            int currentPassSetting = settingRec.indexOf("id");

            int currentSensitivityId=dbQuery.value(currentPassSetting).toInt();

            qDebug() << "Capturing at " << currentSensitivityId;

            FingerDialog *fingerDialog = new FingerDialog(this,fingersToCapture,totalPassesLeft,currentSensitivityId);

            connect(fingerDialog,SIGNAL(initiateBozorthCheck(QString,int,int,int,int)),this,SLOT(initiateBozorthCheck(QString,int,int,int,int)));

            connect(fingerDialog,SIGNAL(isClosing(int)),this,SLOT(confirmFingersCaptured(int)));

            connect(fingerDialog,SIGNAL(cancelCaptureRequested()),this,SLOT(fpCancelCaptureRequested()));

            connect(fingerDialog,SIGNAL(rejected()),this,SLOT(closeFingerDevice()));

            connect(fingerDialog,SIGNAL(failedToScanSignal(int,int,int)),this,SLOT(fpFailedToScanSignal(int,int,int)));

            connect(fingerDialog,SIGNAL(successfulFingerScan(int,int,int)),this,SLOT(fpAcceptScan(int,int,int)));

            fingerDialog->exec();

            fpPassesMade++;


            if(fpPassesMade <= totalPassesRequired){
                msg.sprintf("Passes Left: %d",totalPassesLeft);
                QMessageBox::information(this, "Multiple Passes Information",msg, QMessageBox::Ok);
            }



            for(int u=1;u<=11;u++) {
                continue;
                if(u > fpPassesMade) continue;
                qDebug() << "Pass " << u ;


                for(int v=1;v<=11;v++){
                    if(v > fingersToCapture) break;
                    fpScanResults[u][v][0] = fpScanResults[u][v][1] = fpScanResults[u][v][2] = 0;
                }
                qDebug() << " ";
            }

            for(int u=1;u<=10;u++){
                fpScanFailed[u] = 0;

            }


        }

qDebug() << "Passes Results ";
//fpScanResults[10][11][20][2]
/*
for(int u=1;u<=11;u++) {
    continue;
    if(u > fpPassesMade) continue;
    qDebug() << "Pass " << u ;


    for(int v=1;v<=11;v++){
        if(v > fingersToCapture) break;
        qDebug() << " Finger "<< v;
        for(int w=0;w<20;w++){

            if(fpScanResults[u][v][w][0]==0 && fpScanResults[u][v][w][1]==0) continue;
            qDebug() << " - Failed " << w;
            qDebug() << " - Minutia " << fpScanResults[u][v][w][0];
            qDebug() << " - Quality " << fpScanResults[u][v][w][1];
            qDebug() << " - - ";

        }

        qDebug() << " ";
    }
    qDebug() << " ";
}
*/
        bozorthRunningThreads--; //to get it to minus -1 when the last one is completed

        if(bozorthRunningThreads==-1) {
            compileResults();
        }

    }


    /**
  Finger Arrangement
  1 - Right Thumb
  2 - Right Index
  3 - Right Middle Finger
  4 - Right Ring Finger
  5 - Right Little Finger
  6 - Left Thumb
  7 - Left Index
  8 - Left Middle Finger
  9 - Left Ring Finger
  10 - Left Little Finger
  
*/
    for(finger=1;finger<=10;finger++){
        
        fileName.sprintf("templates/%d.pgm",finger);
        if(fileReader.exists(fileName)){
            
            QImage image(fileName);

            if(image.height()){
                switch(finger){
                    
                case 1:
                    ui->lblFinger1->setPixmap(QPixmap::fromImage(image));
                    break;
                case 2:
                    ui->lblFinger2->setPixmap(QPixmap::fromImage(image));
                    break;
                case 3:
                    ui->lblFinger3->setPixmap(QPixmap::fromImage(image));
                    break;
                case 4:
                    ui->lblFinger4->setPixmap(QPixmap::fromImage(image));
                    break;
                case 5:
                    ui->lblFinger5->setPixmap(QPixmap::fromImage(image));
                    break;
                case 6:
                    ui->lblFinger6->setPixmap(QPixmap::fromImage(image));
                    break;
                case 7:
                    ui->lblFinger7->setPixmap(QPixmap::fromImage(image));
                    break;
                case 8:
                    ui->lblFinger8->setPixmap(QPixmap::fromImage(image));
                    break;
                case 9:
                    ui->lblFinger9->setPixmap(QPixmap::fromImage(image));
                    break;
                case 10:
                    ui->lblFinger10->setPixmap(QPixmap::fromImage(image));
                    break;
                default:
                    qDebug() << " hmm... Unknown finger " << finger;
                }
                
            }else{
                qDebug() << " Not an image: " << fileName;
            }
            
            
            
        }else{
            //qDebug() << " No File: " << fileName;
        }
    }
    
    return nullPrintHolder;
    
}

/*
 
QByteArray MainWindow::XcapturePrint(){

    QProcess::execute("rm -rf ./"+fingerPrintFile);
    
 //QDialog dialog(this);
    
    
return nullPrintHolder;



//    QProgressDialog progress("Please place your finger on the device", "Cancel", 0, 1000, this);
//        for (int i = 0; i < 1000; i++) {
//            progress.setValue(i);
//
//            qApp->processEvents();
//
//            if (progress.wasCanceled())
//                break;
//
//        }
        //progress.setValue(1000);




//return nullPrintHolder;

    QProcess::execute("./templates/enroll");
    QFile fileReader(fingerPrintFile);
    if (!fileReader.exists()){
    
        //QMessageBox::critical(0, "Error Opening File ","Unable to locate the captured fingerprint "  + fingerPrintFile.toAscii(), QMessageBox::Ok);
    
        return nullPrintHolder;
    }
    
    fileReader.open( QFile::ReadOnly);
    return fileReader.readAll();
}

*/


/**
  Clears the previous save session from the form
*/

void MainWindow::clearForm(){

    fpResetCapturableFingers();

    isSpecialRegistration = false;

    clearBozorthTable();

    QValidator *textRx = new QRegExpValidator(QRegExp("^[a-zA-Z\\s-'\\.,]*"), this);
    //    QValidator *addressRx = new QRegExpValidator(QRegExp("^[a-zA-Z\\s-'/]*"), this);

    ui->inpLastName->clear();
    ui->inpLastName->setValidator(textRx);

    ui->inpFirstName->clear();
    ui->inpFirstName->setValidator(textRx);

    ui->inpMiddleName->clear();
    ui->inpMiddleName->setValidator(textRx);

//    ui->inpMobile->setInputMask("9999-999-9999");
    ui->inpMobile->clear();
    QValidator *mobileRx = new QRegExpValidator(QRegExp("^[[0][0-9]\\d{0,10}"), this);
    ui->inpMobile->setValidator(mobileRx);
    connect(ui->inpMobile, SIGNAL(textChanged(QString)), this, SLOT(processMobile(QString)));

    ui->inpAddress->clear();
    //    ui->inpAddress->setValidator(addressRx);
    connect(ui->inpAddress, SIGNAL(textChanged()), this, SLOT(processAddress()));


    ui->inpDobDay->clear();
    QValidator *dobDayRx = new QRegExpValidator(QRegExp("^[0-3][0-9]"), this);
    ui->inpDobDay->setValidator(dobDayRx);


    ui->inpDobMonth->clear();
    QValidator *dobMthRx = new QRegExpValidator(QRegExp("^[0-1][0-9]"), this);
    ui->inpDobMonth->setValidator(dobMthRx);

    ui->inpDobYear->clear();
    QValidator *dobYrRx = new QRegExpValidator(QRegExp("^1[8-9]\\d{0,2}"), this);
    ui->inpDobYear->setValidator(dobYrRx);
    ui->inpAge->setText("");


    ui->inpAge->clear();
    QValidator *ageRx = new QRegExpValidator(QRegExp("^[[1-9][0-9]\\d{0,1}"), this);
    ui->inpAge->setValidator(ageRx);

    ui->inpGender->setCurrentIndex(0);

    //    ui->lblVin->setText("");
    ui->cbxEstimatedAge->setChecked(false);
    ui->cbxNotMobile->setChecked(false);
    ui->lblPhotoFront->setBackgroundRole(QPalette::Base);
    ui->inpOccupation->setCurrentIndex(0);
    ui->lblBozorthProcess->setText("");
    ui->lblPhotoFront->setText("[ No Photo ]");
    
    ui->inpNationalID->clear();

    leftPrintHolder = nullPrintHolder;
    rightPrintHolder = nullPrintHolder;
    frontPhotoHolder = nullPrintHolder;
    leftPhotoHolder = nullPrintHolder;
    rightPhotoHolder = nullPrintHolder;
    fingerPrintsCaptured = false;
    
    // vinValue="";
    
    clearFingerPrints();

    ui->lblBozorthProcess->setStyleSheet(defaultMsgStyle);

    QProcess::execute("rm -f photos/selected_photox.jpg");

    //    QLineEdit *inpLastName;
    //    QLineEdit *inpFirstName;
    //    QLineEdit *inpMiddleName;

    //    QComboBox *inpDobDay;
    //    QComboBox *inpDobMonth;
    //    QComboBox *inpDobYear;

    ui->inpLastName->setStyleSheet(onBlurStyle);

    ui->inpFirstName->setStyleSheet(onBlurStyle);

    ui->inpMiddleName->setStyleSheet(onBlurStyle);

    ui->inpDobDay->setStyleSheet(onBlurStyle);

    ui->inpDobMonth->setStyleSheet(onBlurStyle);

    ui->inpDobYear->setStyleSheet(onBlurStyle);

    ui->inpMobile->setStyleSheet(onBlurStyle);
    


    for(int t=0;t<6;t++){

        globalProcessTimer[t][PROCESS_START] = 0;

        globalProcessTimer[t][PROCESS_END] = 0;

    }


    ui->btnCaptureLeft->setEnabled(true);
    ui->btnSpecial->setEnabled(true);

    cfgLastVin = 0;

    ui->inpLastName->setFocus();

}
void MainWindow::on_btnCaptureLeft_clicked()
{
    
    capturePrint();
        
}


void MainWindow::on_btnSave_clicked(){

    if(validateForm()) {

        previewInfo();
    }
    
}

bool MainWindow::validateForm() {
//    return true;
    QString errorMessages;
    if(ui->inpLastName->text()==""){
        
        errorMessages+="\nSurname";
    }
    
    if(ui->inpFirstName->text()==""){
        
        errorMessages+="\nFirst Name";
        
    }
    
    if(ui->inpMiddleName->text()=="") {
        
        //errorMessages+="\nOther Names";
        
    }
    
    if(ui->inpDobDay->text()==""){
        
        errorMessages+="\nDate of Birth - Day";
        
    }else if(ui->inpDobDay->text().toInt() > 31 || ui->inpDobDay->text().toInt() < 1){
        
        errorMessages+="\nDate of Birth - Invalid Day";
        
    }
    
    if(ui->inpDobMonth->text()==""){
        
        errorMessages+="\nDate of Birth - Month";
    }else if(ui->inpDobMonth->text().toInt() > 12 || ui->inpDobMonth->text().toInt() < 1){
        errorMessages+="\nDate of Birth - Invalid Month";
    }
    
    QDateTime currentTime = QDateTime::currentDateTime();
    
    int minimumDobYear = currentTime.toString("yyyy").toInt()-maximumAge;
    int maximumDobYear = currentTime.toString("yyyy").toInt()-minimumAge;
    if(ui->inpDobYear->text()=="") {
        
        errorMessages+="\nDate of Birth - Year";
        
    }else if(ui->inpDobYear->text().toInt() < minimumDobYear || ui->inpDobYear->text().toInt() > maximumDobYear){
        QString yearText;
        yearText.sprintf("Date of Birth - Minimum Age is %d and Maximum Age is %d",minimumAge,maximumAge);
        errorMessages+="\n" +  yearText;
        
    }else{
        //errorMessages+="\nDate of Birth - Invalid Year";
    }
    
    if(ui->inpAddress->toPlainText()=="") {
        
        errorMessages+="\nAddress";
        
    }

    if(ui->inpGender->currentIndex()==0) {

        errorMessages+="\nGender";

    }



    if(duplicateFingerPrintSet){
        //  qDebug() << "Duplicate Check is Currently disabled!";
        errorMessages+="\nDuplicate Fingerprint Set";
    }

    if(!fingerPrintsCaptured){
        errorMessages+="\nFingerprints have not been captured";
    }

    if(bozorthRunningThreads>=0){

        errorMessages+="\nStill Processing Fingerprints. Please wait.";

    }

    /*
     
    if(!leftPrintHolder.size()){
    
        errorMessages+="\nLeft FingerPrint";
    }
    qDebug("size %d",rightPrintHolder.size());
    if(!rightPrintHolder.size()){
    
        errorMessages+="\nRight FingerPrint";
    }
*/

    if(!frontPhotoHolder.size()){
        
        errorMessages+="\nFront Facing Photo";
    }


    if(!cfgPollingUnitId){
        errorMessages+="\nThis machine has not been assigned to a Registration Centre. Please go to the Control Panel to set this";
    }


    if(errorMessages.isNull()){
        
        qDebug("Form Validated");
        
    }else{
        //qDebug("Form Error " + errorMessages.toUtf8());
        qDebug() << "Form Errors: " << errorMessages;
        QMessageBox::critical(this, "Error Saving Registration","Please check that you have provided the following\n "+errorMessages, QMessageBox::Ok);
        return false;
    }
    return true;
}


void MainWindow::exitApp(){
    
}

void MainWindow::submitRegistration(){

    getCurrentSettings();

    globalProcessTimer[PROCESS_DATA][PROCESS_END] = getCurrentTime();


    QDateTime currentTime = QDateTime::currentDateTime();

    QSqlQuery query;
    QString nextGeneratedVinStr;
    int nextGeneratedVin = cfgLastVin+1;

    nextGeneratedVinStr.sprintf("%d",nextGeneratedVin);
    qDebug() << "vin prefix " <<cfgVinPrefix;
    //vinValue = cfgVinPrefix.toUtf8() + nextGeneratedVinStr.rightJustified(6,'0').toUtf8();
    QString newVin = generateNewVin();
    newVin.replace(QString("-"),QString(""));
    vinValue = newVin;

    QSqlTableModel *regModel = new QSqlTableModel;
    regModel->setTable("registrations");
    
    QSqlTableModel *printRegModel = new QSqlTableModel;
    printRegModel->setTable("fp_fingerprints");
    
    QSqlRecord dbRecord;
    
    QSqlRecord printDbRecord;
    
    QSqlField id("id", QVariant::String);
    //id.setValue(QUuid::createUuid().toString());
    id.setValue(vinValue);
    dbRecord.append(id);

    QSqlField firstName("first_name", QVariant::String);
    firstName.setValue(ui->inpFirstName->text());
    dbRecord.append(firstName);
    
    QSqlField lastName("last_name", QVariant::String);
    lastName.setValue(ui->inpLastName->text());
    dbRecord.append(lastName);
    
    QSqlField otherNames("other_names", QVariant::String);
    otherNames.setValue(ui->inpMiddleName->text());
    dbRecord.append(otherNames);
    
    QSqlField dobDay("dob_day", QVariant::String);
    dobDay.setValue(ui->inpDobDay->text());
    dbRecord.append(dobDay);
    
    QSqlField dobMonth("dob_month", QVariant::String);
    dobMonth.setValue(ui->inpDobMonth->text());
    dbRecord.append(dobMonth);
    
    QSqlField dobYear("dob_year", QVariant::String);
    dobYear.setValue(ui->inpDobYear->text());
    dbRecord.append(dobYear);

    QSqlField dobEstimated("dob_is_estimated", QVariant::Int);
    if(ui->cbxEstimatedAge->checkState()){
        dobEstimated.setValue(1);
    }else{
        dobEstimated.setValue(0);
    }
    dbRecord.append(dobEstimated);


    QSqlField dobNotMobile("landline_number", QVariant::Int);
    if(ui->cbxNotMobile->checkState()){
        dobNotMobile.setValue(1);
    }else{
        dobNotMobile.setValue(0);
    }
    dbRecord.append(dobNotMobile);
    
    QSqlField mobileNumber("mobile_number", QVariant::String);
    mobileNumber.setValue(ui->inpMobile->text());
    dbRecord.append(mobileNumber);
    
    QSqlField homeAddress("home_address", QVariant::String);
    homeAddress.setValue(ui->inpAddress->toPlainText());
    dbRecord.append(homeAddress);
    
    QSqlField gender("gender", QVariant::String);
    gender.setValue(ui->inpGender->currentText());
    dbRecord.append(gender);
    
    QSqlField occup("occupation", QVariant::String);
    occup.setValue(ui->inpOccupation->currentText());
    dbRecord.append(occup);

    QSqlField assetTagField("asset_tag", QVariant::String);
    assetTagField.setValue(assetTag);
    dbRecord.append(assetTagField);
    
    QString ident = ui->inpFirstName->text() + currentTime.toString("yyyy-MM-dd hh:mm:ss") + assetTag;
    QCryptographicHash *hash = new QCryptographicHash(QCryptographicHash::Md5);
    hash->addData(ident.toAscii());
    QByteArray hashed = hash->result();

    qDebug() << "Identifier Hash: " << hashed.toHex();
    
    QSqlField identifier("identifier", QVariant::String);
    identifier.setValue(hashed.toHex());
    dbRecord.append(identifier);
    
    QSqlField dateCreated("int_created", QVariant::String);
    dateCreated.setValue(currentTime.toString("yyyy-MM-dd hh:mm:ss"));
    dbRecord.append(dateCreated);

    QSqlField operatorCode("operator_code", QVariant::String);
    operatorCode.setValue(operatorCodeValue);
    dbRecord.append(operatorCode);

    QSqlField photoField("photo_front", QVariant::ByteArray);
    photoField.setValue(frontPhotoHolder);
    dbRecord.append(photoField);

    QSqlField pollingUnit("polling_unit_id", QVariant::Int);
    pollingUnit.setValue(cfgPollingUnitId);
    dbRecord.append(pollingUnit);
    
    QString delimitationValue = getUnitDelimitation();

    QSqlField delimitationCol("delimitation", QVariant::String);
    delimitationCol.setValue(delimitationValue);
    dbRecord.append(delimitationCol);



    QSqlField minimumMinutiaCol("minimum_minutia", QVariant::Int);
    minimumMinutiaCol.setValue(cfgFingerSensitivityId);
    dbRecord.append(minimumMinutiaCol);


    QSqlField nationalID("national_idnum", QVariant::String);
    nationalID.setValue(ui->inpNationalID->text());
    dbRecord.append(nationalID);
qDebug() << "is Special Reg";
    if(isSpecialRegistration == true){

        QSqlField isSpecial("is_special", QVariant::Int);
        isSpecial.setValue(1);
        dbRecord.append(isSpecial);

        QSqlField specialNote("special_notes", QVariant::String);
        specialNote.setValue(isSpecialNotes);
        dbRecord.append(specialNote);

        QSqlField specialGroup("special_group", QVariant::String);
        specialGroup.setValue(isSpecialGroup);
        dbRecord.append(specialGroup);


    }

    /*
I think I should run this to remove any blank lines in the gallery list.
I'm not sure why I have blank linesin the first place.
sed '/^$/d' stored_minutiae/gallery.lis > stored_minutiae/gallery.lis0
  mv  stored_minutiae/gallery.lis0 stored_minutiae/gallery.lis
  */

    if(regModel->insertRecord(-1, dbRecord)) {
        
        //query.exec("SELECT id from registrations ORDER BY vin DESC LIMIT 1");

        query.exec("SELECT id from registrations ORDER BY int_created DESC LIMIT 1");

        query.next();
        QString lastInsertId = query.value(0).toString();

        qDebug() << "Last Insert Id" << lastInsertId.toUtf8();
        
        //There are obviously better ways of getting this done. Find them out.
        
        
        int printIndex;
        
        QString printFieldName;
        
        QByteArray printFieldContent;
        
        QString  printFieldFile;
        
        QString printDataFile;
        
        QString minutiaDataFile;
        
        QSqlField printFieldBlob;
        
        QStringList minutiaeLines;


        //Create a folder for the xyt files
        QString newDirName ;
        newDirName = "stored_minutiae/"+lastInsertId;
        QDir *dirCreator = new QDir();
        dirCreator->mkdir(newDirName);
        if(totalPassesRequired > 1) {
            for(int cPass=1;cPass<=totalPassesRequired;cPass++) {

                for(printIndex=1; printIndex<=10; printIndex++) {

                    printFieldName.sprintf("finger_blob%d",printIndex);

                    printFieldFile.sprintf("templates/%d/%d.pgm",cPass,printIndex);

                    printDataFile.sprintf("templates/%d/%d.bin",cPass,printIndex);

                    minutiaDataFile.sprintf("templates/%d/%d.xyt",cPass,printIndex);

                    //PGM
                    QFile aFileReader(printFieldFile);
                    if(!aFileReader.exists()){
                        continue;
                    }

                    aFileReader.open(QFile::ReadOnly);

                    printFieldContent = aFileReader.readAll();
                    aFileReader.remove(printFieldFile);
                    qDebug() << "Name " <<printFieldName << " FileName " << printFieldFile << " Size " << printFieldContent.size();

                    printFieldBlob.setName("pass_index");
                    printFieldBlob.setType(QVariant::Int);
                    printFieldBlob.setValue(cPass);



                    printFieldBlob.setName(printFieldName);
                    printFieldBlob.setType(QVariant::ByteArray);
                    printFieldBlob.setValue(printFieldContent);

                    aFileReader.close();

                    dbRecord.append(printFieldBlob);

                    //LIBFPRINT BINARY
                    QFile bFileReader(printDataFile);
                    if(!bFileReader.exists()){
                        qDebug() << "Unable to find " << printDataFile;
                        continue;
                    }

                    bFileReader.open(QFile::ReadOnly);
                    printFieldContent = bFileReader.readAll();
                    bFileReader.remove(printDataFile);
                    bFileReader.close();

                    qDebug() << " Removing " << printDataFile;


                    cfgLastMinutia++;

                    QString mmId;

                    mmId = lastInsertId.toUtf8()+ "_"+ QString::number(cfgLastMinutia).toStdString().c_str();

                    qDebug() << "Last Minutia " << cfgLastMinutia << " mm " << mmId;

                    QSqlField printDataId("id", QVariant::String);
                    printDataId.setValue(mmId);
                    printDbRecord.append(printDataId);


                    QSqlField printDataBlob("fingerprint_data", QVariant::ByteArray);
                    printDataBlob.setValue(printFieldContent);
                    printDbRecord.append(printDataBlob);


                    printDataBlob.setName("fingerprint_finger");
                    printDataBlob.setType(QVariant::Int);
                    printDataBlob.setValue(printIndex);
                    printDbRecord.append(printDataBlob);


                    printDataBlob.setName("registration_id");
                    printDataBlob.setType(QVariant::Int);
                    printDataBlob.setValue(lastInsertId);
                    printDbRecord.append(printDataBlob);

                    QFile cFileReader(minutiaDataFile);
                    if(!cFileReader.exists()){
                        qDebug() << "Unable to find " << minutiaDataFile;

                        continue;
                    }

                    cFileReader.open(QFile::ReadOnly);
                    printFieldContent = cFileReader.readAll();
                    //

                    printDataBlob.setName("fingerprint_minutia");
                    printDataBlob.setType(QVariant::ByteArray);
                    printDataBlob.setValue(printFieldContent);
                    printDbRecord.append(printDataBlob);

                    printDataBlob.setName("minutia_count");
                    printDataBlob.setType(QVariant::Int);
                    printDataBlob.setValue(fpScanResults[cPass][printIndex][0]);
                    printDbRecord.append(printDataBlob);

                    printDataBlob.setName("print_quality");
                    printDataBlob.setType(QVariant::Int);
                    printDataBlob.setValue(fpScanResults[cPass][printIndex][1]);
                    printDbRecord.append(printDataBlob);


                    printDataBlob.setName("failed_scans");
                    printDataBlob.setType(QVariant::Int);
                    printDataBlob.setValue(fpScanResults[cPass][printIndex][2]);
                    printDbRecord.append(printDataBlob);



                    printDataBlob.setName("minutia_link");
                    printDataBlob.setType(QVariant::Int);
                    printDataBlob.setValue(cfgLastMinutia);
                    printDbRecord.append(printDataBlob);

                    printRegModel->insertRecord(-1, printDbRecord);
                    QString newMinutiaFile;


                    // query.exec("SELECT minutia_link from fp_fingerprints ORDER BY created DESC LIMIT 1");
                    //query.next();
                    int lastMinInsertId = cfgLastMinutia;

                    newMinutiaFile.sprintf("/%d.xyt",lastMinInsertId);
                    newMinutiaFile = newDirName.toUtf8()+newMinutiaFile;

                    qDebug() << " Moving " << minutiaDataFile << " to " <<newMinutiaFile;

                    cFileReader.rename(minutiaDataFile,newMinutiaFile);
                    cFileReader.close();

                    QString minutiaXyt;
                    minutiaXyt.sprintf("/%d.xyt",lastMinInsertId);
                    minutiaXyt = lastInsertId.toUtf8()+minutiaXyt;
                    QString minutiaLine;
                    minutiaLine.append(minutiaXyt);
                    //minutiaLine.append("\n");

                    minutiaeLines+=minutiaLine;

                }
        }
    }else{

        for(printIndex=1; printIndex<=10; printIndex++) {

            if(printIndex > fingersToCapture) continue;

            printFieldName.sprintf("finger_blob%d",printIndex);

            printFieldFile.sprintf("templates/%d.pgm",printIndex);

            printDataFile.sprintf("templates/%d.bin",printIndex);

            minutiaDataFile.sprintf("templates/%d.xyt",printIndex);

            //PGM
            QFile aFileReader(printFieldFile);
            if(!aFileReader.exists()){
                continue;
            }

            aFileReader.open(QFile::ReadOnly);

            printFieldContent = aFileReader.readAll();
            aFileReader.remove(printFieldFile);
            qDebug() << "Name " <<printFieldName << " FileName " << printFieldFile << " Size " << printFieldContent.size();
            printFieldBlob.setName(printFieldName);
            printFieldBlob.setType(QVariant::ByteArray);
            printFieldBlob.setValue(printFieldContent);

            aFileReader.close();

            dbRecord.append(printFieldBlob);

            //LIBFPRINT BINARY
            QFile bFileReader(printDataFile);
            if(!bFileReader.exists()){
                qDebug() << "Unable to find " << printDataFile;
                continue;
            }

            bFileReader.open(QFile::ReadOnly);
            printFieldContent = bFileReader.readAll();
            bFileReader.remove(printDataFile);
            bFileReader.close();

            qDebug() << " Removing " << printDataFile;

            cfgLastMinutia++;
            qDebug() << "Last nsertid " <<  lastInsertId;

            QString mmId;

            mmId = lastInsertId.toUtf8()+ "_"+ QString::number(cfgLastMinutia).toStdString().c_str();

            qDebug() << "Last Minutia " << cfgLastMinutia << " mm " << mmId;

            QSqlField printDataId("id", QVariant::String);
            printDataId.setValue(mmId);
            printDbRecord.append(printDataId);


//            QSqlField printDataId("id", QVariant::Int);
//            printDataId.setValue(QUuid::createUuid().toString());
//            printDbRecord.append(printDataId);

            QSqlField printDataBlob("fingerprint_data", QVariant::ByteArray);
            printDataBlob.setValue(printFieldContent);
            printDbRecord.append(printDataBlob);


            printDataBlob.setName("fingerprint_finger");
            printDataBlob.setType(QVariant::Int);
            printDataBlob.setValue(printIndex);
            printDbRecord.append(printDataBlob);


            printDataBlob.setName("registration_id");
            printDataBlob.setType(QVariant::Int);
            printDataBlob.setValue(lastInsertId);
            printDbRecord.append(printDataBlob);

            QFile cFileReader(minutiaDataFile);
            if(!cFileReader.exists()){
                qDebug() << "Unable to find " << minutiaDataFile;

                continue;
            }

            cFileReader.open(QFile::ReadOnly);
            printFieldContent = cFileReader.readAll();
            //

            printDataBlob.setName("fingerprint_minutia");
            printDataBlob.setType(QVariant::ByteArray);
            printDataBlob.setValue(printFieldContent);
            printDbRecord.append(printDataBlob);

            printDataBlob.setName("minutia_count");
            printDataBlob.setType(QVariant::Int);
            printDataBlob.setValue(fingerprintAttributes[printIndex][0]);
            printDbRecord.append(printDataBlob);

            printDataBlob.setName("print_quality");
            printDataBlob.setType(QVariant::Int);
            printDataBlob.setValue(fingerprintAttributes[printIndex][2]);
            printDbRecord.append(printDataBlob);

           // cfgLastMinutia++;

            printDataBlob.setName("minutia_link");
            printDataBlob.setType(QVariant::Int);
            printDataBlob.setValue(cfgLastMinutia);
            printDbRecord.append(printDataBlob);

            printRegModel->insertRecord(-1, printDbRecord);
            QString newMinutiaFile;




            // query.exec("SELECT minutia_link from fp_fingerprints ORDER BY created DESC LIMIT 1");
            //query.next();
            int lastMinInsertId = cfgLastMinutia;

            newMinutiaFile.sprintf("/%d.xyt",lastMinInsertId);
            newMinutiaFile = newDirName.toUtf8()+newMinutiaFile;

            qDebug() << " Moving " << minutiaDataFile << " to " <<newMinutiaFile;

            cFileReader.rename(minutiaDataFile,newMinutiaFile);
            cFileReader.close();

            QString minutiaXyt;
            minutiaXyt.sprintf("/%d.xyt",lastMinInsertId);
            minutiaXyt = lastInsertId.toUtf8()+minutiaXyt;
            QString minutiaLine;
            minutiaLine.append(minutiaXyt);
            //minutiaLine.append("\n");

            minutiaeLines+=minutiaLine;

        }


    }
        QString updateQuery;
        updateQuery.sprintf("UPDATE settings set setting_value='%d' WHERE setting_name='last_minutia'",cfgLastMinutia);
        query.exec(updateQuery);
        
        if(minutiaeLines.count()){

            QString galleryFileName = "stored_minutiae/"+ lastInsertId.toUtf8() +".lis";

            qDebug() << "Gallery file name: " << galleryFileName;
            QFile galleryFile(galleryFileName);
            if ( galleryFile.open( QIODevice::WriteOnly) )
            {
                QTextStream stream( &galleryFile );
                for ( QStringList::Iterator it = minutiaeLines.begin(); it != minutiaeLines.end(); ++it ){
                    //I need to do if empty... how ?
                    
                    stream << *it << "\n";
                }
                galleryFile.close();
            }
        }
        

        //vinValue=selectedVinPrefix;
        
        //        ui->lblVin->setText(vinValue);
        
        //Is this next line really necessary?
        query.exec("UPDATE registrations set vin='"+vinValue.toUtf8()+"' WHERE id='"+lastInsertId.toUtf8()+"'");
        QString updateSettingsQuery;
        updateSettingsQuery= "UPDATE settings set setting_value=setting_value+1 WHERE setting_name='last_vin'";
        query.exec(updateSettingsQuery);

        QMessageBox::information(this, "Success","The registration information has been saved. \nThe VIN is: "+vinValue, QMessageBox::Ok);

        saveExperimentInfo(vinValue);
        clearForm();
        reCountRegistrations();
        backup();



        
    } else {
        QMessageBox::critical(this, "Query Error","Unable to insert: \n" +regModel->lastError().text().toUtf8(),QMessageBox::Cancel);
    }
    return;
    //QSqlField f3("albumcount", QVariant::Int);
    /*
    f1.setValue(QVariant("Tunde"));
    f2.setValue(QVariant("Badmus"));
    f3.setValue(QVariant(leftPrintHolder));
    
    record.append(f2);
    //record.append(f3);
    record.append(f1);
    record.append(f3);
    
    
    if(regModel->insertRecord(-1, record)){
        qDebug("Record Inserted");
//clearForm();
    reCountRegistrations();
    
        //qDebug("Records %s",regModel->select());
    
    }else{
        qDebug("Record Error");
    }
    
return;

     query.prepare("INSERT INTO registrations ("
     
            "last_name,"
            "first_name, "
            "other_names, "
            "dob_day,"
            "dob_month,"
            "dob_year,"
            "mobile_number,"
            "home_address,"
            "state,"
            "lga,"
            "ward,"
            "int_created,"
            "left_finger_blob1,"
            "right_finger_blob1"
            
           ") VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)"
                   );
                   
     //query.addBindValue(1001);
     query.addBindValue(ui->inpLastName->text());
     
     query.addBindValue(ui->inpFirstName->text());
     query.addBindValue(ui->inpMiddleName->text());
     
     query.addBindValue(ui->inpDobDay->currentText());
     query.addBindValue(ui->inpDobMonth->currentText());
     query.addBindValue(ui->inpDobYear->currentText());
     
     query.addBindValue(ui->inpMobile->text());
     query.addBindValue(ui->inpAddress->toPlainText());
     query.addBindValue(ui->inpState->currentText());
     
     query.addBindValue(ui->inpLga->currentText());
     query.addBindValue(ui->inpWard->currentText());
     query.addBindValue(currentTime.toString("yyyy-MM-dd hh:mm:ss"));
     
     
    query.addBindValue(leftPrintHolder);
    query.addBindValue(rightPrintHolder);
    
    if(query.exec()){
    
     QMessageBox::information(0, "Success","The registration information has been saved", QMessageBox::Ok);
    clearForm();
    reCountRegistrations();
    
    
 }else{
     QMessageBox::critical(0, "Query Error","Unable to insert: \n"+query.lastError().text()+"\n"+query.lastQuery(),QMessageBox::Cancel);
 }
     //ui->inpAddress->insertPlainText(query.lastQuery());
 */
    /*
  "left_finger_index,"
            "left_finger_hash1,"
            "left_finger_blob1"
            "left_finger_hash2,"
            "left_finger_blob2"
            "left_finger_hash3,"
            "left_finger_blob3"
            "right_finger_index,"
            "right_finger_hash1,"
            "right_finger_blob1"
            "right_finger_hash2,"
            "right_finger_blob2"
            "right_finger_hash3,"
            "right_finger_blob3"
            "int_created,"
            
                //QLabel *label = new QLabel("Please place your left thumb on the device");
  //  label->show();
    //btnSave = new QPushButton;
    //btnSave = ui->btnSave;
    //ui->inpMobile->setText("some string");
            */
    
    
}


/* The following on_currentIndexChanged functions need to be rewritten
   to store the abbreviations globally and in an array
   Start Rewrite Portion
   */
/*
void MainWindow::on_inpState_currentIndexChanged(QString currentState)
{
    ui->inpLga->clear();
    ui->lblVin->setText("----------");
    if(currentState.isEmpty()){
        return;
    }
//This should be done with foreign key index rather than what is here now
    QSqlQuery dbQuery;
    dbQuery.exec("SELECT id,abbreviation FROM states WHERE  name='"+currentState.toUtf8()+"'");
    dbQuery.next();
    QString stateId = dbQuery.value(0).toString();
    QString stateAbbrev = dbQuery.value(1).toString();
    
    ui->lblVin->setText(stateAbbrev+"----------");
    
    QString queryString = "SELECT name FROM local_governments WHERE  state_id="+stateId;
    dbQuery.exec(queryString);
    
    while (dbQuery.next()) {
        ui->inpLga->addItem(dbQuery.value(0).toString());
    }
}

void MainWindow::on_inpLga_currentIndexChanged(QString currentLga)
{

    ui->inpWard->clear();
    ui->lblVin->setText("----------");
    
    if(currentLga.isEmpty()){
        return;
    }
    //This should be done with foreign key index rather than what is here now
    QSqlQuery dbQuery;
    dbQuery.exec("SELECT id,abbreviation FROM states WHERE  name='"+ui->inpState->currentText()+"'");
    dbQuery.next();
    QString stateId = dbQuery.value(0).toString();
     QString stateAbbrev = dbQuery.value(1).toString();
     
    QString queryString = "SELECT id,abbreviation FROM local_governments WHERE  name='"+currentLga+"'";
    dbQuery.exec(queryString);
    dbQuery.next();
    QString lgaId = dbQuery.value(0).toString();
    
    QString lgaAbbrev = dbQuery.value(1).toString();
    
    ui->lblVin->setText(stateAbbrev+lgaAbbrev+"-------");
    
    
    queryString = "SELECT name FROM wards WHERE  local_government_id='"+lgaId+"'";
    dbQuery.exec(queryString);
    
    while (dbQuery.next()) {
        ui->inpWard->addItem(dbQuery.value(0).toString());
    }
    
    
}

void MainWindow::on_inpWard_currentIndexChanged(QString currentWard )
{


    if(currentWard.isEmpty()){
        return;
    }
    //ui->lblVin->setText("----------");
    //This should be done with foreign key index rather than what is here now
    QSqlQuery dbQuery;
    dbQuery.exec("SELECT id,abbreviation FROM states WHERE  name='"+ui->inpState->currentText()+"'");
    dbQuery.next();
    QString stateId = dbQuery.value(0).toString();
     QString stateAbbrev = dbQuery.value(1).toString();
     
    QString queryString = "SELECT id,abbreviation FROM local_governments WHERE  name='"+ui->inpLga->currentText()+"'";
    dbQuery.exec(queryString);
    dbQuery.next();
    QString lgaId = dbQuery.value(0).toString();
    
    QString lgaAbbrev = dbQuery.value(1).toString();
    
    
    
    
    queryString = "SELECT abbreviation FROM wards WHERE local_government_id='"+lgaId+"' AND name='"+currentWard+"'";
    qDebug(queryString.toUtf8());
    dbQuery.exec(queryString);
    
    dbQuery.next();
    QString wardAbbrev = dbQuery.value(0).toString();
    vinValue = stateAbbrev+lgaAbbrev+wardAbbrev;
    ui->lblVin->setText(stateAbbrev+lgaAbbrev+wardAbbrev+"---");
    
    
    
}
*/
/* End Rewrite Portion */



void MainWindow::backupDatabase(){
    
    
    //    QString dbBackupExe="sh scripts/database_backup.sh";
    
    
    
    //    QProcess::execute(dbBackupExe);
    
    //    qDebug() << dbBackupExe;
    
    //Until I'm able to globalize this information
    //bool backupSuccess = runDatabaseBackup();
    /*
    QString dbBackupExe;
    QString targetFileName;
    QDateTime currentTime = QDateTime::currentDateTime();
    targetFileName.sprintf("database/create_backup_%d",currentTime.toTime_t());
    
    dbBackupExe.vsprintf("mysqldump -u %s --password=%s %s |gzip > %s",dbUsername,
                        dbPassword.toUtf8(),
                        dbName,
                        targetFileName);
                        
    qDebug() << dbBackupExe;
*/
    
}

void MainWindow::on_capturePhotoFront_clicked()
{
    //QString fileName = capturePhoto();
    capturePhoto();

    //qDebug() << fileName;
}


void MainWindow::keyPressEvent(QKeyEvent *e)
{


    //reset activity Timer
    resetIdleTimeWatcher();


    //    qDebug() <<"key Pressed :" <<e->key();
    switch (e->key())
    {
        //case Qt::Key_R : myResetMethod(); updateGL(); break;
        // and so on...
        
    case Qt::Key_F1:
        ui->inpLastName->setFocus();
        
        break;
    case Qt::Key_F2:
        ui->inpFirstName->setFocus();
        break;
    case Qt::Key_F3:
        ui->inpMiddleName->setFocus();
        break;
    case (Qt::ShiftModifier + Qt::Key_F4):
        if(ui->cbxEstimatedAge->checkState()){
            ui->cbxEstimatedAge->setChecked(false);
        }else{
            ui->cbxEstimatedAge->setChecked(true);
        }
        break;
    case Qt::Key_F4:
        ui->inpDobDay->setFocus();
        break;
    case Qt::Key_F5:
        ui->inpGender->setFocus();
        break;
    case Qt::Key_F6:
        ui->inpAddress->setFocus();
        break;
    case Qt::Key_F7:
        ui->inpMobile->setFocus();
        break;
    case Qt::Key_F8:
        ui->inpOccupation->setFocus();
        break;
    case Qt::Key_F9:
        capturePrint();
        break;
    case Qt::Key_F10:
        capturePhoto();
        break;
    case Qt::Key_F12:
        //submitRegistration();
        if(validateForm()){
            previewInfo();
        }
        break;

        // Default calls the original method to handle standard keys
    default: QWidget::keyPressEvent(e);
    }
}

bool MainWindow::controlLogin(){
    
    bool showDialog = false;
    bool ok;
    QString text = QInputDialog::getText(this, tr("Authorization Required"),
                                         tr("Password:"), QLineEdit::Password,
                                         "", &ok);
    if (ok && !text.isEmpty()){
        if(text!=authPassword){//save in database ?
            QMessageBox::critical(this, "Authorization Error","Invalid password provided. Please check and try again", QMessageBox::Ok);
            return false;
        }else{
            showDialog  = true;
        }
    }
    
    return showDialog;
}

void MainWindow::on_actionControl_Panel_triggered()
{
    
    if(controlLogin()){
        ControlPanel *controlPanelDialog = new ControlPanel(this);
        controlPanelDialog->exec();
        displayDelimitation();//Just in case cchanges have been made or thot to be made
    }
    
}


void MainWindow::getAssetTag()
{

    QFile file(assetTagFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        assetTag += line;
        line = in.readLine();
    }

    

    qDebug() << "Asset Tag" << assetTag;


}

void MainWindow::getMACAddress() {
    QFile file(macAddrFilename);
    file.open(QFile::ReadOnly);

    QByteArray assetContents = file.readAll();
    qDebug() << assetContents;


    QTextStream in(&file);
    in >> macAddr;
}

void MainWindow::on_actionRegister_Operator_triggered()
{

}


QString MainWindow::getUnitDelimitation(){
    refreshUnitDelimitation();
    QString delimText =  delimStateAbbr.toUtf8() + "/"+ delimLgaAbbr.toUtf8() + "/"+  delimRaAbbr.toUtf8() + "/"+  delimPuAbbr.toUtf8();
    return delimText;
}

void MainWindow::refreshUnitDelimitation(){

    QSqlQuery dbQuery;

    QString queryString = "SELECT polling_units.id pu_id,polling_units.name pu_name, polling_units.abbreviation pu_abbreviation, "
                          " registration_areas.id ra_id,registration_areas.name ra_name,registration_areas.abbreviation ra_abbreviation, local_governments.id lga_id, "
                          " local_governments.abbreviation lga_abbreviation,local_governments.name lga_name,states.id s_id,states.name s_name, states.abbreviation  s_abbreviation"
                          " FROM settings, states, local_governments, registration_areas, polling_units "
                          " WHERE setting_name =  'polling_unit_id' "
                          " AND polling_units.id = setting_value "
                          " AND registration_areas.id = polling_units.registration_area_id "
                          " AND registration_areas.local_government_id = local_governments.id "
                          " AND local_governments.state_id = states.id";

    //    qDebug() <<queryString;

    dbQuery.exec(queryString);

    QSqlRecord settingRec = dbQuery.record();
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    /*
    int pollingUnitCol = settingRec.indexOf("pu_id");
    int raCol = settingRec.indexOf("ra_id");
    int lgaCol = settingRec.indexOf("lga_id");
    int stateCol = settingRec.indexOf("s_id");

    selectedPuId = dbQuery.value(pollingUnitCol).toInt();
    selectedRaId = dbQuery.value(raCol).toInt();
    selectedLgaId = dbQuery.value(lgaCol).toInt();
    selectedStateId= dbQuery.value(stateCol).toInt();
    */

    int puAbbrCol = settingRec.indexOf("pu_abbreviation");
    int raAbbrCol = settingRec.indexOf("ra_abbreviation");
    int lgaAbbrCol  = settingRec.indexOf("lga_abbreviation");
    int stateAbbrCol  = settingRec.indexOf("s_abbreviation");

    int puNameCol = settingRec.indexOf("pu_name");
    int raNameCol = settingRec.indexOf("ra_name");
    int lgaNameCol  = settingRec.indexOf("lga_name");
    int stateNameCol  = settingRec.indexOf("s_name");

    delimState = dbQuery.value(stateNameCol).toString();
    delimStateAbbr = dbQuery.value(stateAbbrCol).toString();

    delimLga = dbQuery.value(lgaNameCol).toString();
    delimLgaAbbr = dbQuery.value(lgaAbbrCol).toString();

    delimRa = dbQuery.value(raNameCol).toString();
    delimRaAbbr = dbQuery.value(raAbbrCol).toString();

    delimPu = dbQuery.value(puNameCol).toString();
    delimPuAbbr = dbQuery.value(puAbbrCol).toString();


}

void MainWindow::displayDelimitation(){
    delimitationValue = getUnitDelimitation();
    QDateTime currentTime = QDateTime::currentDateTime();

    delimitationText = "Delimitation: " +delimitationValue.toUtf8();

    //statusBar()->showMessage(delimitationText);

    delimitationText = "State: \t\t["+ delimStateAbbr +"] "+delimState + "\n\n"
                       "LGA: \t\t["+ delimLgaAbbr +"] "+delimLga +"\n\n"
                       "Registration Area: \t["  + delimRaAbbr + "] "+ delimRa + "]\n\n"
                       "Registration Centre: \t["  + delimPuAbbr +  "] "+delimPu + "\n\n"
                       "Delimitation: \t"  + delimitationValue.toUtf8() + "\n\n"+
                       "Current Time: \t"  + currentTime.toString("h:mm:ss:AP, MMMM d, yyyy").toUtf8() + "\n\n"+
                       "\n\n"+
                       "If this computer is not for this Registration Centre, \nPlease contact your reporting officer to change it from the Control Panel "
                       ;

    QMessageBox::information(this, "Registration Centre Details",delimitationText, QMessageBox::Ok);
}

void MainWindow::on_inpDobYear_lostFocus()
{

    //    QDateTime currentTime = QDateTime::currentDateTime();
    //    QString errorMessages;
    //    int minimumDobYear = currentTime.toString("yyyy").toInt()-minimumAge;

    //    QString str = ui->inpDobYear->text();

    //    foreach (QChar c, str) {
    //        if (!c.isNumber()) {
    //             // one of the characters is a string
    //            QMessageBox::critical(this, "Invalid Date of Birth Year","Enter only number in the Year field", QMessageBox::Ok);
    //            QTimer::singleShot(200,ui->inpDobYear,SLOT(setFocus()));
    //            return;
    //        }
    //    }

    //    if(str=="" || str.toInt() > minimumDobYear){
    //        QString yearText;

    //        yearText.sprintf("Date of Birth - Minimum Age is %d",minimumAge);
    //        errorMessages+="\n" +  yearText;
    //        QMessageBox::critical(this, "Minimum Age Restriction",errorMessages, QMessageBox::Ok);
    //        QTimer::singleShot(200,ui->inpDobYear,SLOT(setFocus()));


    //    }

}

void MainWindow::lockInterface() {


    qDebug() << "Idle Time Exceeded. Locking Interface";

    lastActivityTimer->stop();

    QMessageBox msgBox;

    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setBackgroundRole(QPalette::Base);
    msgBox.setAutoFillBackground(true);
    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setWindowTitle("Security Check");
    msgBox.setText("This window has been locked because you have exceeded the idle time.\n Please click Yes to Continue, or No to Close the Registration Window.");

    switch (msgBox.exec()) {

        case QMessageBox::Yes:

            break;
        case QMessageBox::No:
        default:

            break;


    }

    //QMessageBox::critical(0, "Idle Time Reached","Locking Interface", QMessageBox::Ok);
    resetIdleTimeWatcher();
}

void MainWindow::resetIdleTimeWatcher() {

    qDebug() << "Resetting Timer";

    lastActivityTimer->stop();

    connect(lastActivityTimer,SIGNAL(timeout()),this,SLOT(lockInterface()));

    lastActivityTimer->start(allowedIdleTime * 1000);

}







//In mainwindow cpp file add. Other fxns can be called ere.
void MainWindow::closeEvent(QCloseEvent *ev) {
    //ev->ignore();
    /*Add other procedures for closing
        authenticate_user
        backup
        other_closing_checks
    */

    //this should be commented for now oh! I'm not responsible for any data loss :)
    //QProcess::execute("shutdown -h now"); //this is the ideal way forward
    emit(closingMain());
    //ev->accept();	//uncomment for now unless...you will see why yourself
}


void MainWindow::initiateBozorthCheck(QString filePath,int printIndex,int detectedMinutiaCount,int setMinimumMinutia,int printQuality){


    bozorthCheck = new Bozorth_Check(this);

    connect(bozorthCheck, SIGNAL(scanResult(int,QString)), this, SLOT(completeBozorthCheck(int,QString)));
    QFuture<void> scanFuture;
    scanFuture = QtConcurrent::run(bozorthCheck, &Bozorth_Check::checkGallery,filePath,printIndex  );

    bozorthRunningThreads++;
    QString processingMessage;
    processingMessage.sprintf("Processing %d fingerprints. Please wait...",bozorthRunningThreads);
    ui->lblBozorthProcess->setText(processingMessage);

    fingerprintAttributes[printIndex][0] = detectedMinutiaCount;
    fingerprintAttributes[printIndex][1] = setMinimumMinutia;
    fingerprintAttributes[printIndex][2] = printQuality;

    fingerPrintsCaptured = true;
}


void MainWindow::completeBozorthCheck(int printIndex, QString scanResult){
    qDebug()<< " Running Threads " << bozorthRunningThreads << " finger: "<<printIndex;

    QStringList resultList = scanResult.split("\n", QString::SkipEmptyParts);
    QString lineHolder;
    QStringList tHolder;
    //    qDebug() << "scanResuult:" <<  scanResult  << " Result List: " << resultList;
    for (int i = 0; i < resultList.size(); ++i) {

        lineHolder = resultList.at(i);

        tHolder = lineHolder.split(" ",QString::SkipEmptyParts);
        //        qDebug() << " lineHolder: " << lineHolder << " tHolder " << tHolder;
        QSqlQuery scoreQuery;
        QString insertQuery;
        for(int x=0;x<tHolder.size();x+=2){
            int y = x+1;
            QString galleryName = tHolder.at(x);
            QString galleryScoreStr = tHolder.at(y);
            //            qDebug() << "Gallery Name: " << galleryName << " GalleryScore: " << galleryScoreStr;
            //int galleryScore = galleryScoreStr.toUInt();

            galleryName.replace(QString(".lis"),QString(""));
            insertQuery = "INSERT INTO tmp_search SET gallery_file='"+galleryName.toUtf8()+"', fingers=1, score='"+ galleryScoreStr.toUtf8()+  "' ON DUPLICATE KEY UPDATE fingers=fingers+1, score=score+" + galleryScoreStr.toUtf8();
            //            qDebug() <<"tmp: " << insertQuery;
            scoreQuery.exec(insertQuery);

            //            short galleryIndex   = (short) galleryName.toInt();

            //            bozorthMatches[galleryIndex][0]++; //COUNT
            //            bozorthMatches[galleryIndex][1]+=galleryScore; //SCORE

        }

    }

    bozorthRunningThreads--;

    QString processingMessage;

    processingMessage.sprintf("Processing %d fingerprints. Please wait...",bozorthRunningThreads);

    ui->lblBozorthProcess->setText(processingMessage);

    QString logoFile = "./resources/logo.png";
    QImage logo;
    logo.load(logoFile);
    if(bozorthRunningThreads==-1){
        compileResults();
    }
    //qDebug() << printIndex << bozorthMatches ;
}

void MainWindow::compileResults(){

    globalProcessTimer[PROCESS_FINGERPRINT][PROCESS_END]  = getCurrentTime();
    qDebug() << "Fingerprint  Time Taken: " << globalProcessTimer[PROCESS_FINGERPRINT][PROCESS_END] - globalProcessTimer[3][PROCESS_END];

    qDebug() << "Results Compiled";
    QString processingMessage = "Processing Completed ";
    //qDebug () << bozorthMatches;
    ui->lblBozorthProcess->setText(processingMessage);

    QSqlQuery bzQuery;
    QString checkString;
    checkString.sprintf("SELECT * FROM tmp_search WHERE score >= %d",bzScoreMaximum);
    bzQuery.exec(checkString);


    if(bzQuery.size()) {

        qDebug() << " This set appears to be a duplicate by score ";
        duplicateFingerPrintSet  = true;

    }else{

        checkString.sprintf("SELECT * FROM tmp_search WHERE fingers >= %d",bzGalleryMaximum);
        bzQuery.exec(checkString);
        if(bzQuery.size()) {
            qDebug() << " This set appears to be a duplicate by fingers";
            duplicateFingerPrintSet  = true;
        }

    }


    if(fingerPrintsCaptured){

        processingMessage = "Fingerprints Accepted.";
        ui->lblBozorthProcess->setStyleSheet(successMsgStyle);
    }else{

        processingMessage = "Not Captured";
        ui->lblBozorthProcess->setStyleSheet(invalidInputStyle);
    }

    if(duplicateFingerPrintSet){

        processingMessage = "Duplicate Fingerprints Detected";
        clearBozorthTable();
        ui->lblBozorthProcess->setStyleSheet(errorMsgStyle);

        QMessageBox::critical(this, "Duplicate Registration Error","These fingerprints have been detected to be duplicates of another registration", QMessageBox::Ok);




    }

    ui->btnCaptureLeft->setEnabled(true);
    ui->btnSpecial->setEnabled(true);

    ui->lblBozorthProcess->setText(processingMessage);

}

void MainWindow::setupStatusBar(){

    QTimer::singleShot(500,this,SLOT(displayDelimitation()));
    QTimer *statusBarUpdater     = new QTimer();
    connect(statusBarUpdater,SIGNAL(timeout()),this,SLOT(updateAppTime()));
    statusBarUpdater->start(1000);

}

void MainWindow::updateAppTime(){

    QDateTime currentSystemTime = QDateTime::currentDateTime();
    QString  newStatus = currentSystemTime.toString("dddd dd MMM yyyy h:mm:ss:ap").toUtf8()+ " | " +delimitationValue;
    statusBar()->setToolTip(delimitationText);
    statusBar()->showMessage(newStatus);

}


void MainWindow::previewInfo(){


    //    QStringList vin;
    //    vin << QString::number(2);
    //    recordsDialog->filePrintPreview(vin);

    //

    //    if (validateForm()) {
    this->prevLastName = ui->inpLastName->text();
    this->prevFirstName = ui->inpFirstName->text();
    this->prevMiddleName = ui->inpMiddleName->text();

    this->prevDobDay = ui->inpDobDay->text();
    this->prevDobMonth = ui->inpDobMonth->text();
    this->prevDobYear = ui->inpDobYear->text();
    this->prevGender = ui->inpGender->currentText();
    this->prevOccupation = ui->inpOccupation->currentText();
    this->prevAddress = ui->inpAddress->toPlainText();
    //    this->prevState = ui->inpState->text();
    //    this->prevLga = ui->inpLga->text();
    //    this->prevWard = ui->inpWard->text();
    this->prevMobile = ui->inpMobile->text();

    previewDialog = new PreviewDialog(this);

    connect(previewDialog, SIGNAL(saveAndPrint()), this, SLOT(saveAndPrint()) );

    previewDialog->setWindowTitle("Preview Information");

    previewDialog->exec();
    //    }


}

void MainWindow::saveAndPrint() {
    // Save Info
    submitRegistration();

    // Get Vin Print Direct
    qDebug()<< "Vin Value for print "<<vinValue;
    recordsDialog->filePrint(vinValue.toUtf8());
}
void MainWindow::confirmFingersCaptured(int fingersCaptured){

    if(fingersCaptured<fingersToCapture){
        fingerPrintsCaptured = false;
        QMessageBox::critical(this, "FingerPrint Capture","It seems you cancelled the fingerprint capture. Please start again if you wish to complete this registration", QMessageBox::Ok);
    }

}



void MainWindow::processLastName(QString str) {
    preProcess();
    if(ui->inpLastName->text().length()==1){
        globalProcessTimer[PROCESS_DATA][PROCESS_START] = getCurrentTime();
    }
    ui->inpLastName->setText(ucfirst(str));
    ui->inpLastName->setStyleSheet(onFocusStyle);
}



void MainWindow::processFirstName(QString str) {
    preProcess();
    ui->inpFirstName->setText(ucfirst(str));
    ui->inpFirstName->setStyleSheet(onFocusStyle);
}

void MainWindow::processMiddleName(QString str) {
    preProcess();
    ui->inpMiddleName->setText(ucfirst(str));
    ui->inpMiddleName->setStyleSheet(onFocusStyle);
}


void MainWindow::processAddress() {
    //    ui->inpAddress->setPlainText(ucfirst(str));
    ui->inpAddress->setStyleSheet(onFocusStyle);
}


void MainWindow::processDobDay(QString str) {
    qDebug() << str;
    preProcess();
    ui->inpDobDay->setStyleSheet(onFocusStyle);
}


void MainWindow::processDobMonth(QString str) {
    preProcess();
    if (str == "00") {
        ui->inpDobMonth->setStyleSheet(invalidInputStyle);
    } else {
        ui->inpDobMonth->setStyleSheet(onFocusStyle);
    }

}

void MainWindow::processDobYear(QString str) {
    qDebug() << str;
    preProcess();
    ui->inpDobYear->setStyleSheet(onFocusStyle);
}

void MainWindow::processMobile(QString str) {
    qDebug() << str;
    preProcess();
    ui->inpMobile->setStyleSheet(onFocusStyle);
}

void MainWindow::lastNameOnBlur() {
    ui->inpLastName->setStyleSheet(onBlurStyle);
}

void MainWindow::firstNameOnBlur() {
    ui->inpFirstName->setStyleSheet(onBlurStyle);
}


void MainWindow::middleNameOnBlur() {
    ui->inpMiddleName->setStyleSheet(onBlurStyle);
}

void MainWindow::addressOnBlur() {
    ui->inpAddress->setStyleSheet(onBlurStyle);
}


void MainWindow::dobDayOnBlur() {

    if (ui->inpDobDay->text() == "00" || ui->inpDobDay->text().toInt() > 31 || ui->inpDobDay->text() == "0") {
        ui->inpDobDay->setStyleSheet(invalidInputStyle);
        ui->lblDobError->setText("Invalid Day");
    } else {
        ui->inpDobDay->setStyleSheet(onBlurStyle);
        ui->lblDobError->setText("");
    }
}

void MainWindow::dobMonthOnBlur() {
    dobDayOnBlur();

    if (ui->inpDobMonth->text() == "00") {
        ui->inpDobMonth->setStyleSheet(invalidInputStyle);
        ui->lblDobError->setText("Invalid Month");
    } else if (ui->inpDobMonth->text().toInt() > 12) {
        ui->inpDobMonth->setStyleSheet(invalidInputStyle);
        ui->lblDobError->setText("Invalid Month");
    } else if(ui->inpDobMonth->text() == "02" && ui->inpDobDay->text().toInt() > 28) {  // 28 Days in February // except it is a leap year
        if (ui->inpDobDay->text().toInt() == 29 && !ui->inpDobYear%4) { // Leap Year has 29 Days
            ui->inpDobMonth->setStyleSheet(onBlurStyle);
            ui->lblDobError->setText("");
        } else {
            ui->inpDobDay->setStyleSheet(invalidInputStyle);
            ui->lblDobError->setText("February has 28 Days");
        }

    } else if((ui->inpDobMonth->text().toInt() == 4 || ui->inpDobMonth->text().toInt() == 6 || ui->inpDobMonth->text().toInt() == 9 || ui->inpDobMonth->text().toInt() == 11) && ui->inpDobDay->text().toInt() > 30) {  // Months with 30 days only
        ui->inpDobDay->setStyleSheet(invalidInputStyle);
        ui->lblDobError->setText("Month Has Only 30 Days");
    } else {
        ui->inpDobMonth->setStyleSheet(onBlurStyle);
        ui->lblDobError->setText("");
    }


}

void MainWindow::preProcess() {
    ui->inpAddress->setStyleSheet(onBlurStyle);
}

void MainWindow::dobYearOnBlur() {
    ui->inpAge->setText(QString::number(2010 - ui->inpDobYear->text().toInt()) );

    if (2010 - ui->inpDobYear->text().toInt() > maximumAge) {
        ui->inpDobYear->setStyleSheet(invalidInputStyle);
        ui->inpAge->setStyleSheet(invalidInputStyle);
        ui->lblDobError->setText("Maximum Age Exceeded.");
    } else if (2010 - ui->inpDobYear->text().toInt() < minimumAge)  {
        ui->inpDobYear->setStyleSheet(invalidInputStyle);
        ui->inpAge->setStyleSheet(invalidInputStyle);
        ui->lblDobError->setText("Minimum Age is 18");
    } else {
        ui->inpDobYear->setStyleSheet(onBlurStyle);
        ui->inpAge->setStyleSheet(onBlurStyle);
        ui->lblDobError->setText("");
    }
}


void MainWindow::dobAgeOnBlur() {
    ui->inpDobYear->setText(QString::number(2010 - ui->inpAge->text().toInt()) );

    if (ui->inpDobDay->text() == "") {
        ui->inpDobDay->setText("01");
    }

    if (ui->inpDobMonth->text() == "") {
        ui->inpDobMonth->setText("01");
    }

    if (ui->inpAge->text().toInt() > maximumAge) {
        ui->inpDobYear->setStyleSheet(invalidInputStyle);
        ui->inpAge->setStyleSheet(invalidInputStyle);
        ui->lblDobError->setText("Maximum Age Exceeded.");
    } else if (2010 - ui->inpDobYear->text().toInt() < minimumAge)  {
        ui->inpDobYear->setStyleSheet(invalidInputStyle);
        ui->inpAge->setStyleSheet(invalidInputStyle);
        ui->lblDobError->setText("Minimum Age is 18");
    } else {
        ui->inpDobYear->setStyleSheet(onBlurStyle);
        ui->inpAge->setStyleSheet(onBlurStyle);
        ui->lblDobError->setText("");
    }
}

void MainWindow::mobileOnBlur() {
    ui->inpMobile->setStyleSheet(onBlurStyle);
}

QString MainWindow::ucfirst(QString str) {
    if (str.size() < 1) {
        return "";
    }

    str = str.toLower();

    // Remove trailing whitespace
    bool appendWhitespace = false;
    if (str.endsWith(" ")) {
        appendWhitespace = true;
    }

    QStringList tokens = str.split(" ", QString::SkipEmptyParts);
    QList<QString>::iterator tokItr = tokens.begin();

    for (tokItr = tokens.begin(); tokItr != tokens.end(); ++tokItr) {
        (*tokItr) = (*tokItr).at(0).toUpper() + (*tokItr).mid(1);
    }

    str = tokens.join(" ");
    if (appendWhitespace) str += " ";

    return str;
}

void MainWindow::on_btnCancelRegistration_clicked()
{

    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setBackgroundRole(QPalette::Base);
    msgBox.setAutoFillBackground(true);
    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setText("Are you sure you want to clear this form \nand start a new registration?");
    msgBox.setWindowTitle("Cancel Registration");

    switch (msgBox.exec()) {

        case QMessageBox::Yes:
            clearForm();
            break;
    }
}

void MainWindow::closeFingerDevice(){

    //fpPassesMade = totalPassesRequired+1; //Just to prevent it from coming back when totalPasssesRequired>0!

    QProcess fingerProcess;
    qDebug() << " Closing FingerDevice From MainWindow ";
    fingerProcess.startDetached("killall enroll");

}

void MainWindow::saveExperimentInfo(QString generatedVin){

    QSqlTableModel *experimentModel = new QSqlTableModel;
    experimentModel->setTable("experiment_results");

    QSqlRecord experimentRecord;

    QSqlField vin("vin", QVariant::String);
    vin.setValue(generatedVin);
    experimentRecord.append(vin);

    QSqlField opCode("operator_code", QVariant::String);
    opCode.setValue(operatorCodeValue);
    experimentRecord.append(opCode);

    QSqlField fingerSensitivity("fingerprint_sensitivity_id", QVariant::Int);
    fingerSensitivity.setValue(cfgFingerSensitivityId);
    experimentRecord.append(fingerSensitivity);

    QSqlField photoTime("photo_time", QVariant::Int);
    int photoTimeValue = globalProcessTimer[PROCESS_PHOTO][PROCESS_END] - globalProcessTimer[PROCESS_PHOTO][PROCESS_START];
    photoTime.setValue(photoTimeValue);
    experimentRecord.append(photoTime);

    QSqlField fingerTime("fingerprint_time", QVariant::Int);
    int fingerTimeValue = globalProcessTimer[PROCESS_FINGERPRINT][PROCESS_END] - globalProcessTimer[PROCESS_FINGERPRINT][PROCESS_START];
    fingerTime.setValue(fingerTimeValue);
    experimentRecord.append(fingerTime);


    QSqlField isDuplicateValue("is_duplicate", QVariant::Bool);
    isDuplicateValue.setValue(duplicateFingerPrintSet);
    experimentRecord.append(isDuplicateValue);



    experimentModel->insertRecord(-1, experimentRecord);

}


void MainWindow::getCurrentSettings(){

    QSqlQuery query;

    QSqlTableModel *settingsModel = new QSqlTableModel;
    settingsModel->setTable("settings");

    QSqlRecord settingRecord;


    QString queryString = "SELECT setting_value "
                          " FROM settings "
                          " WHERE setting_name =  'polling_unit_id' ";

    int currentTime  = getCurrentTime();

    query.exec(queryString);

    //QSqlRecord settingRec = query.record();

    if(!query.size()) {

        QMessageBox::critical(this, "Registration Centre Error","This machine has not yet been assigned to a registration centre. Please specify a Registration Centre in the Control Panel", QMessageBox::Ok);
        return;
    }

    query.next();

    cfgPollingUnitId = query.value(0).toInt();

    queryString = "SELECT setting_value "
                  " FROM settings "
                  " WHERE setting_name =  'vin_prefix' ";

    query.exec(queryString);

    //QSqlRecord settingRec = query.record();

    if(!query.size()){

        QSqlField setting_name("setting_name", QVariant::String);
        setting_name.setValue("vin_prefix");
        settingRecord.append(setting_name);

        QSqlField setting_value("setting_value", QVariant::String);
        setting_value.setValue(currentTime);
        settingRecord.append(setting_value);

        settingsModel->insertRecord(-1, settingRecord);
        cfgVinPrefix = currentTime;

    }else{

        query.next();

        cfgVinPrefix = query.value(0).toString();

        //qDebug() << " Loaded VIN Prefix "  << query.value(0).toString();
    }

    queryString = "SELECT setting_value "
                  " FROM settings "
                  " WHERE setting_name =  'last_vin' ";

    query.exec(queryString);

    //QSqlRecord settingRec = query.record();

    if(!query.size()){

        QSqlField setting_name("setting_name", QVariant::String);
        setting_name.setValue("last_vin");
        settingRecord.append(setting_name);

        QSqlField setting_value("setting_value", QVariant::Int);
        setting_value.setValue(0);
        settingRecord.append(setting_value);

        settingsModel->insertRecord(-1, settingRecord);
        cfgLastVin = 0;


    }else{

        query.next();

        cfgLastVin = query.value(0).toInt();

    }

    queryString = "SELECT setting_value "
                  " FROM settings "
                  " WHERE setting_name =  'last_minutia' ";

    query.exec(queryString);

    //QSqlRecord settingRec = query.record();

    if(!query.size()){

        QSqlField setting_name("setting_name", QVariant::String);
        setting_name.setValue("last_minutia");
        settingRecord.append(setting_name);

        QSqlField setting_value("setting_value", QVariant::Int);
        setting_value.setValue(1);
        settingRecord.append(setting_value);

        settingsModel->insertRecord(-1, settingRecord);
        cfgLastMinutia = 1;


    } else {

        query.next();

        cfgLastMinutia = query.value(0).toInt();

    }

    queryString = "SELECT setting_value "
                  " FROM settings "
                  " WHERE setting_name =  'fingerprint_sensitivity_id' ";

    query.exec(queryString);

    if(!query.size()) {

        QSqlField setting_name("setting_name", QVariant::String);
        setting_name.setValue("fingerprint_sensitivity_id");
        settingRecord.append(setting_name);

        QSqlField setting_value("setting_value", QVariant::Int);
        setting_value.setValue(3);//default of 3....
        settingRecord.append(setting_value);

        settingsModel->insertRecord(-1, settingRecord);
        cfgFingerSensitivityId = 3;


    }else{

        query.next();

        cfgFingerSensitivityId = query.value(0).toInt();

    }






}


QString MainWindow::generateNewVin() {

    bool ok;
    srand(time(0));
    //Used variables ///////////////////////////////////
    //First 7 digits
    uint init_epoch;
    QString elapsed_time;
    QString coded_time = "1234567";     //please dont modify except you want to increase digits
    //Middle 16 digits
    QString mac;
    //Last 3 digits
    int serial;
    QString coded_serial;
    QString digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    /////////////////////////////////////////////////
    //RETRIEVE INFO FROM FILES
    QFile saved(".nvi");
    if (!saved.exists())
    QProcess::execute("./scripts/vinhelper.sh");
    saved.open(QIODevice::ReadOnly);
    //Custom epoch
    QString dummy = saved.readLine();
    dummy.chop(1);
    init_epoch = dummy.toUInt(&ok);
    //Serial number
    dummy = saved.readLine();
    dummy.chop(1);     //remove trailing newline xter
    serial = dummy.toUInt(&ok);
    //MAC Addy
    mac = saved.readLine();
    mac.chop(1);
    //Convert mac to number -- digit decoding
    QString mac_buff;
    for (int i = 0; i < mac.size(); i++) {
    if ( i % 2 == 0 )
    mac_buff += "-";        //use hyphen to create number groups
    mac_buff += QString("%1").arg(dummy.setNum(digits.indexOf(mac[i]),10),2,QChar('0'));
    }
    //    qDebug() << init_epoch << " " << serial << " " << mac;
    saved.close();
    //FIRST SEVEN CHARACTERS
    //Randomize
    for (int i = 0; i < coded_time.size(); i++)
    coded_time[i] = digits[rand()%10];
    //add time information
    elapsed_time = elapsed_time.setNum(time(0) - init_epoch);
    coded_time.chop(elapsed_time.size());
    //UPD8 S/N COUNT
    serial++;
    coded_serial = coded_serial.setNum(serial,10);
    //CONCATENATE EVRI
    //qDebug() << coded_time+elapsed_time+"-"+"-"+QString("%1").arg(coded_serial,5,QChar('0'));
    //SAVE EPOCH, SERIAL AND MAC BACK TO FILE
    saved.open(QIODevice::WriteOnly | QIODevice::Truncate);
    QTextStream out(&saved);
    out << init_epoch << "\n" << serial << "\n" << mac << "\n";
    saved.close();
    //CONCATENATE EVRI AND RETURN
    //return coded_time+elapsed_time+mac_buff+"-"+QString("%1").arg(coded_serial,5,QChar('0'));
    QString newCo = coded_time+elapsed_time+mac_buff+""+QString("%1").arg(coded_serial,4,QChar('0'));
    qDebug() << "From Fs " << newCo;
    return newCo;

}

void MainWindow::on_cbxNotMobile_clicked()
{
    return;
    if(ui->cbxNotMobile->isChecked()) {
        ui->inpMobile->setInputMask("99999999999");
    } else {
        ui->inpMobile->setInputMask("9999-999-9999");
    }
}

void MainWindow::on_actionHelpOpenVR_triggered()
{
    HelpDialog *helpDialog = new HelpDialog(this);
    helpDialog->exec();
}


void MainWindow::backup() {
//    QString  processOutput;
    QProcess *scriptProcess = new QProcess();
    scriptProcess->start("./scripts/backup.sh");

//    if (!scriptProcess->waitForFinished()){
//        qDebug() << "Error :" << scriptProcess->errorString();
//    } else {
//        processOutput =  scriptProcess->readAll();
//        qDebug() << "====Backup Output====";
//        qDebug() << processOutput;
//    }


}



void MainWindow::fpCancelCaptureRequested(){

    fpPassesMade = totalPassesRequired+1; //Just to prevent it from coming back when totalPasssesRequired>0!
    if(totalPassesRequired>1){
        confirmFingersCaptured(0);
    }
}

void MainWindow::fpFailedToScanSignal(int fpPrintIndex,int fpMinutiaCount,int fpQuality){

    qDebug() << " fpPass " << fpPassesMade<<  " fpPrintIndex " << fpPrintIndex << " Minutia " << fpMinutiaCount << " fpQuality " << fpQuality;
    //int fpScanResults[10][10][20][2];// 1=>Pass 2=>PrintIndex 3=>Failed 4=>MinutiaCount|Quality

    try {

        fpScanFailed[fpPrintIndex]++;

        int curFailedCount = fpScanFailed[fpPrintIndex];

        fpFailedResults[fpPassesMade][fpPrintIndex][fpScanFailed[fpPrintIndex]][0] = fpMinutiaCount;
        fpFailedResults[fpPassesMade][fpPrintIndex][fpScanFailed[fpPrintIndex]][1] = fpQuality;




        //Create a folder for the xyt files
        QString newDirName ;

        newDirName.sprintf("templates/%d",fpPassesMade);

        QDir *dirCreator = new QDir();
        if(!dirCreator->exists(newDirName)){
            dirCreator->mkdir(newDirName);
        }

        QProcess xytGen;
        QString xytGenString;
        xytGenString.sprintf("mindtct templates/failed_scan_%d.pgm templates/%d/stored_failed_scan_%d_%d",curFailedCount,fpPassesMade,fpPrintIndex,curFailedCount);
        xytGen.startDetached(xytGenString);


        //pgm
        QString savedScanFile;
        savedScanFile.sprintf("templates/failed_scan_%d.pgm",curFailedCount);

        QString storeScanFile;
        storeScanFile.sprintf("templates/%d/stored_failed_scan_%d_%d.pgm",fpPassesMade,fpPrintIndex,curFailedCount);

        QFile saveScan;
        saveScan.copy(savedScanFile,storeScanFile);


        //bin
        savedScanFile.sprintf("templates/failed_scan_%d.bin",curFailedCount);

        storeScanFile.sprintf("templates/%d/stored_failed_scan_%d_%d.bin",fpPassesMade,fpPrintIndex,curFailedCount);
        saveScan.copy(savedScanFile,storeScanFile);


        fpScanResults[fpPassesMade][fpPrintIndex][2]++;

    } catch(...) {
        qDebug() << "Unable to catch signal ";
    }


}

void MainWindow::fpAcceptScan(int fpPrintIndex,int fpMinutiaCount,int fpQuality){

    qDebug() << " fpPass " << fpPassesMade<<  " fpPrintIndex " << fpPrintIndex << " Minutia " << fpMinutiaCount << " fpQuality " << fpQuality;

    try {

        //fpScanFailed[fpPrintIndex]++;
        fpScanResults[fpPassesMade][fpPrintIndex][0]  = fpMinutiaCount;
        fpScanResults[fpPassesMade][fpPrintIndex][1]  = fpQuality;

        //Maybe I should move the file at this point.

        QString fingerImage;
        fingerImage.sprintf("templates/%d.pgm",fpPrintIndex);

        QString newMinutiaFile;
        QFile cFileReader(fingerImage);



        //Create a folder for the xyt files
        QString newDirName ;

        newDirName.sprintf("templates/%d",fpPassesMade);

        QDir *dirCreator = new QDir();
        if(!dirCreator->exists(newDirName)){
            dirCreator->mkdir(newDirName);
        }

        if(!cFileReader.exists()){
            qDebug() << "Unable to find " << fingerImage;

            return;
        }

        QProcess xytGen;
        QString xytGenString;
        xytGenString.sprintf("mindtct templates/%d.pgm templates/%d/%d",fpPrintIndex,fpPassesMade,fpPrintIndex);
        xytGen.startDetached(xytGenString);


        xytGenString.sprintf("cp templates/%d.pgm templates/%d.bin templates/%d/",fpPrintIndex,fpPrintIndex,fpPassesMade);
        xytGen.startDetached(xytGenString);



//        newMinutiaFile.sprintf("/%d.xyt",fpPrintIndex);
//        newMinutiaFile = newDirName.toUtf8()+newMinutiaFile;

//        qDebug() << " Moving " << minutiaDataFile << " to " <<newMinutiaFile;

//        cFileReader.rename(minutiaDataFile,newMinutiaFile);

    }catch(...){
        qDebug() << "Something bad happened here! successfulFingerScan";
    }


}



void MainWindow::backupStatusChanged(int code)
{
    qDebug() << "Backup Status: " << code;
    if (code) {
        ui->devRemovable->setStyleSheet(deviceOkStatusStyle);
//        ui->leRemovable->setText("Backup Connected.");
    } else {
        ui->devRemovable->setStyleSheet(deviceErrStatusStyle);
//        ui->leRemovable->setText("Connect Backup.");
    }
}

void MainWindow::fpStatusChanged(int code)
{
    qDebug() << "FP Status: " << code;
    if (code) {
        ui->devFP->setStyleSheet(deviceOkStatusStyle);
//        ui->leFP->setText("Fingerprint Reader Connected.");
    } else {
        ui->devFP->setStyleSheet(deviceErrStatusStyle);
//        ui->leFP->setText("Connect Fingerprint Reader.");
    }

}

void MainWindow::cameraStatusChanged(int code)
{
    qDebug() << "Camera Status: " << code;
    if(code){
        ui->devWebcam->setStyleSheet(deviceOkStatusStyle);
//        ui->leWebcam->setText("Webcam Connected.");
    } else{
        ui->devWebcam->setStyleSheet(deviceErrStatusStyle);
//        ui->leWebcam->setText("Connect Webcam.");
    }
}



void MainWindow::printerStatusChanged(int code)
{
    qDebug() << "Printer Status: " << code;
    if(code){
        ui->devPrinter->setStyleSheet(deviceOkStatusStyle);
//        ui->lePrinter->setText("Printer Connected.");
    } else {
        ui->devPrinter->setStyleSheet(deviceErrStatusStyle);
//        ui->lePrinter->setText("Connect Printer.");
    }
}



void MainWindow::clearBozorthTable(){

    QSqlQuery clearQuery;

    clearQuery.exec("DROP TABLE IF EXISTS `tmp_search`; CREATE TABLE IF NOT EXISTS `tmp_search` (`gallery_file` varchar(50) NOT NULL DEFAULT '', `fingers` int(2) NOT NULL DEFAULT '0', `score` int(4) NOT NULL DEFAULT '0', `created` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, PRIMARY KEY (`gallery_file`)) ENGINE=MyISAM DEFAULT CHARSET=latin1");

}

void MainWindow::on_btnSpecial_clicked()
{

    QString ccFingers;

    fpResetCapturableFingers();

    ccFingers = capturableFingers.join(",");

    SpecialEnrollment *spE = new SpecialEnrollment(this,ccFingers);

    connect(spE,SIGNAL(setFingersToCapture(QStringList,QString,QString)),this,SLOT(fpSetCapturableFingers(QStringList,QString,QString)));
    connect(spE,SIGNAL(cancelSpecialEnrollment()),this,SLOT(fpResetCapturableFingers()));
    spE->exec();


    qDebug() << "Capturable Fingers " << capturableFingers;


    if(!skipSpecialRegistration) {

        if(!capturableFingers.contains("0")){
            if(capturableFingers.length()==0 || capturableFingers.length()==10){
                qDebug() << "is special is false here";
                isSpecialRegistration  = false;
            }else{
                qDebug() << "is special is true";
                isSpecialRegistration  = true;
            }

           capturePrint();

       } else {

            //it means we are NOT capturing any fingers for this registration
            fingerPrintsCaptured  = true;
            isSpecialRegistration  = true;
            bozorthRunningThreads=-1;// is this the best way?
            ui->lblBozorthProcess->setText("No Fingerprint Scan");
       }
    }
}



void MainWindow::fpSetCapturableFingers(QStringList selectedFingers,QString specialNotes,QString specialGroup){

    qDebug() << "Finger Signal Recieved: " << selectedFingers;

    capturableFingers = selectedFingers;

    isSpecialGroup = specialGroup;

    isSpecialNotes = specialNotes;
    skipSpecialRegistration  = false;

}

void MainWindow::fpResetCapturableFingers(){

    isSpecialRegistration = false;
    skipSpecialRegistration  = true;
    capturableFingers.clear();
    capturableFingers << "1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7"<<"8"<<"9"<<"10";
}
