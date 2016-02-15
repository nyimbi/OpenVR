#include "dashboard.h"
#include "mainwindow.h"
#include "reportsdialog.h"
#include "recordsdialog.h"
#include "queuetickets.h"
#include "ui_dashboard.h"
#include "controlpanel.h"

#include "reportsdialog.h"
#include "backup.h"
#include "import.h"

#include "QSqlQuery"
#include "QMessageBox"
#include "QInputDialog"

#include "QBuffer"
#include "QByteArray"
#include "QTimer"

#include "devicecheck.h"
//#include "finger_device_check.h"

#include <QtCore>


QTimer *backupTimer;
QTimer *importTimer;
QString deviceOkStyle = "border:1px solid grey; background-color:rgb(208, 255, 179);padding:10px;";
QString deviceErrStyle = "border:1px solid grey; background-color:rgb(255, 196, 196);padding:10px;";

bool checkHardware = true;

DashBoard::DashBoard(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::DashBoard)
{
    ui->setupUi(this);

    this->showFullScreen();

    connect(ui->btnReports, SIGNAL(clicked()), this, SLOT(printReports()));
    connect(ui->btnVotersCard, SIGNAL(clicked()), this, SLOT(printVotersCard()));
    connect(ui->btnQueueTickets, SIGNAL(clicked()), this, SLOT(queueTickets()));

    connect(ui->btnBackup, SIGNAL(clicked()), this, SLOT(backupAll()));
//    connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(importBackups()));

//    backupTimer = new QTimer();
//    connect(backupTimer, SIGNAL(timeout()), this, SLOT(backupAll()));
//    backupTimer->start(20000);

//    importTimer = new QTimer();
//    connect(importTimer, SIGNAL(timeout()), this, SLOT(importBackups()));
//    importTimer->start(300000);

    DeviceCheck *devChecker = new DeviceCheck();
    connect(devChecker, SIGNAL(backupStatusUpdate(int)), this, SLOT(backupStatusChanged(int)));
    connect(devChecker, SIGNAL(fpDeviceStatusUpdate(int)), this, SLOT(fpStatusChanged(int)));
    connect(devChecker, SIGNAL(cameraStatusUpdate(int)), this, SLOT(cameraStatusChanged(int)));
    connect(devChecker, SIGNAL(printerStatusUpdate(int)), this, SLOT(printerStatusChanged(int)));
    devFuture = QtConcurrent::run(devChecker, &DeviceCheck::checkTimer);
    checkRCSetting();
}

DashBoard::~DashBoard()
{
    delete ui;
}

void DashBoard::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_F2:
            ui->btnRegistration->click();
            break;
        case Qt::Key_F3:
            ui->btnVotersCard->click();
            break;
        case Qt::Key_F4:
            ui->btnQueueTickets->click();
            break;
        case Qt::Key_F5:
            ui->btnReports->click();
            break;
        case Qt::Key_F8:
            ui->btnControlPanel->click();
            break;
        case Qt::Key_F6:
            ui->btnBackup->click();
            break;
        case Qt::Key_F7:
//            ui->btnImport->click();
//            break;
        case Qt::Key_F11:
            ui->btnShutdown->click();
            break;
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default: QWidget::keyPressEvent(e);
            qDebug() << "Pressed Key: " << e->key();
            break;
    }
}

void DashBoard::on_btnRegistration_clicked()
{
    QString operatorCodeValue  = operatorLogin();
    bool isAuthorizedOperator = (operatorCodeValue=="")? false:true;


    if(!isAuthorizedOperator) {

        QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again", QMessageBox::Ok);

        return;

    }

    MainWindow *m = new MainWindow(this,operatorCodeValue);

    m->loggedOperatorCode = operatorCodeValue;
    connect(m,SIGNAL(closingMain()),this,SLOT(restoreDashboard()));
    checkHardware = false;
    m->setWindowFlags(Qt::Dialog);
    m->showMaximized();
    m->show();

}


void DashBoard::printReports()
{
    QString operatorCodeValue  = operatorLogin();
    bool isAuthorizedOperator = (operatorCodeValue=="")? false:true;

    if(!isAuthorizedOperator) {

        QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again", QMessageBox::Ok);

        return;

    }

    ReportsDialog *reportsDialog = new ReportsDialog(this);
    //    connect(reportsDialog,SIGNAL(closingMain()),this,SLOT(restoreDashboard()));
    checkHardware = false;
    reportsDialog->exec();
    checkHardware = true;
}



void DashBoard::printVotersCard()
{
    QString operatorCodeValue  = operatorLogin();
    bool isAuthorizedOperator = (operatorCodeValue=="")? false:true;

    if(!isAuthorizedOperator) {

        QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again", QMessageBox::Ok);

        return;

    }

    RecordsDialog *recordsDialog = new RecordsDialog(this);
    connect(recordsDialog,SIGNAL(finished(int)),this,SLOT(restoreDashboard()));
    checkHardware = false;
    recordsDialog->exec();
    checkHardware = true;

    //this->show();
}


void DashBoard::restoreDashboard(){

    checkHardware = true;
    this->show();

}

//void DashBoard::on_btnReports_clicked()
//{
//    if(controlLogin()){
//        ReportsDialog *reportsDialog = new ReportsDialog(this);
//        reportsDialog->exec();
//    }
//}

bool DashBoard::controlLogin()
{
    bool showDialog = false;
    bool ok;
    QString text = QInputDialog::getText(this, tr("Authorization Required"),
                                         tr("Password:"), QLineEdit::Password,
                                         "", &ok);
    if (ok && !text.isEmpty()){
        if(text!="password"){//save in database ?
            QMessageBox::critical(this, "Authorization Error","Invalid password provided. Please check and try again", QMessageBox::Ok);
            return false;
        }else{
            showDialog  = true;
            checkHardware = false;
        }
    }

    return showDialog;
}

void DashBoard::on_btnControlPanel_clicked()
{
    if(controlLogin()) {
        checkHardware = false;
        ControlPanel *controlPanelDialog = new ControlPanel(this);
        controlPanelDialog->exec();
        checkHardware = true;
        //displayDelimitation();//Just in case cchanges have been made or thot to be made
    }
}



void DashBoard::queueTickets() {

    QString operatorCodeValue  = operatorLogin();
    bool isAuthorizedOperator = (operatorCodeValue=="")? false:true;

    if(!isAuthorizedOperator) {

        QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again", QMessageBox::Ok);

        return;

    }

    QueueTickets *queueTickets = new QueueTickets(this);
    //    connect(queueTickets,SIGNAL(closingMain()),this,SLOT(restoreDashboard()));
    checkHardware = false;
    queueTickets->exec();
    checkHardware = true;
}

//void DashBoard::checkDevices() {
//    if (checkHardware) {
//        QFile cameraChecker;
//        if(cameraChecker.exists("/dev/video1") || cameraChecker.exists("/dev/video0")){
//            ui->devWebcam->setStyleSheet(deviceOkStyle);
//            ui->leWebcam->setText("Webcam Connected.");
//        } else{
//            ui->devWebcam->setStyleSheet(deviceErrStyle);
//            ui->leWebcam->setText("Connect Webcam.");
//        }


//        QString  processOutput;
//        QProcess *scriptProcess = new QProcess();
//    //    QDir::current();
//        scriptProcess->start("./scripts/check_removable.sh");

//        if (!scriptProcess->waitForFinished()){
//            qDebug() << "Error :" << scriptProcess->errorString();
//        } else {
//            processOutput =  scriptProcess->readAll();
//        }

    

//        if (processOutput != "") {
//            ui->devRemovable->setStyleSheet(deviceOkStyle);
//            ui->leRemovable->setText("Backup Connected.");
//        } else {
//            ui->devRemovable->setStyleSheet(deviceErrStyle);
//            ui->leRemovable->setText("Connect Backup.");
//        }


//        if (fingerDeviceExists()) {
//            ui->devFP->setStyleSheet(deviceOkStyle);
//            ui->leFP->setText("Fingerprint Reader Connected.");
//        } else {
//            ui->devFP->setStyleSheet(deviceErrStyle);
//            ui->leFP->setText("Connect Fingerprint Reader.");
//        }


//        processOutput="";

//        QProcess *printerProcess = new QProcess();
//        printerProcess->start("./scripts/check_printer.sh");

//        if (!printerProcess->waitForFinished()){
//            qDebug() << "Error :" << printerProcess->errorString();
//        } else {
//            processOutput =  printerProcess->readAll();
//        }

//        if (processOutput != "") {
//            ui->devPrinter->setStyleSheet(deviceOkStyle);
//            ui->lePrinter->setText("Printer Connected.");
//        } else {
//            ui->devPrinter->setStyleSheet(deviceErrStyle);
//            ui->lePrinter->setText("Connect Printer.");
//        }
//    }
//}

void DashBoard::on_btnShutdown_clicked()
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setBackgroundRole(QPalette::Base);
    msgBox.setAutoFillBackground(true);
    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setText("Are you sure you want to exit the software and shutdown?");
    msgBox.setWindowTitle("Shutdown Computer");

    switch (msgBox.exec()) {

        case QMessageBox::Yes:
            QProcess *shutDownCall = new QProcess;

            QString gnomeCall = "sudo poweroff -f -p";

            shutDownCall->start(gnomeCall);

            this->close();
        break;
    }
}

void DashBoard::backupStatusChanged(int code)
{
    qDebug() << "Backup Status: " << code;
    if (code) {
        ui->devRemovable->setStyleSheet(deviceOkStyle);
        ui->leRemovable->setText("Backup Connected.");
    } else {
        ui->devRemovable->setStyleSheet(deviceErrStyle);
        ui->leRemovable->setText("Connect Backup.");
    }
}

void DashBoard::fpStatusChanged(int code)
{
    qDebug() << "FP Status: " << code;
    if (code) {
        ui->devFP->setStyleSheet(deviceOkStyle);
        ui->leFP->setText("Fingerprint Reader Connected.");
    } else {
        ui->devFP->setStyleSheet(deviceErrStyle);
        ui->leFP->setText("Connect Fingerprint Reader.");
    }

}

void DashBoard::cameraStatusChanged(int code)
{
    qDebug() << "Camera Status: " << code;
    if(code){
        ui->devWebcam->setStyleSheet(deviceOkStyle);
        ui->leWebcam->setText("Webcam Connected.");
    } else{
        ui->devWebcam->setStyleSheet(deviceErrStyle);
        ui->leWebcam->setText("Connect Webcam.");
    }
}

void DashBoard::printerStatusChanged(int code)
{
    qDebug() << "Printer Status: " << code;
    if(code){
        ui->devPrinter->setStyleSheet(deviceOkStyle);
        ui->lePrinter->setText("Printer Connected.");
    } else {
        ui->devPrinter->setStyleSheet(deviceErrStyle);
        ui->lePrinter->setText("Connect Printer.");
    }
}

void DashBoard::backupAll() {
    QString operatorCodeValue  = operatorLogin();
    bool isAuthorizedOperator = (operatorCodeValue=="")? false:true;

    if(!isAuthorizedOperator) {
        QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again", QMessageBox::Ok);
        return;
    }

    Backup *backup = new Backup(this);
    backup->exec();
}

void DashBoard::importBackups() {
    QString operatorCodeValue  = operatorLogin();
    bool isAuthorizedOperator = (operatorCodeValue=="")? false:true;

    if(!isAuthorizedOperator) {
        QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again", QMessageBox::Ok);
        return;
    }

    Import *import = new Import(this);
    import->exec();
}


QString DashBoard::operatorLogin(){

    QString operatorCodeValue;
    bool ok = false;
    QString suppliedAccessCode = QInputDialog::getText(this, tr("Please Authorization Required"),
                                                       tr("Please Enter Your \nOperator Access Code:"), QLineEdit::Password,
                                                       "", &ok);

    if (ok && !suppliedAccessCode.isEmpty()){
        QSqlQuery dbQuery;
        QString sqlQuery= "SELECT * FROM registered_operators WHERE access_code='"+suppliedAccessCode.toUtf8()+"'";
        dbQuery.exec(sqlQuery);

        if(dbQuery.next()){
            if(dbQuery.value(4).toInt()==1){
                QMessageBox::information(this, "Access Granted","Welcome "+dbQuery.value(2).toString()+" "+dbQuery.value(3).toString()+".\nPlease proceed.", QMessageBox::Ok);

                operatorCodeValue = dbQuery.value(1).toString();
                qDebug() << " Operator Code: "<< operatorCodeValue;
                return operatorCodeValue;
            } else {
                QMessageBox::critical(this, "Authorization Error","Your access code has been deactivated.", QMessageBox::Ok);
            }
        }
    }

    return "";
}


void DashBoard::checkRCSetting(){

    QSqlQuery query;

    QString queryString = "SELECT setting_value "
                          " FROM settings "
                          " WHERE setting_name =  'polling_unit_id' ";

    query.exec(queryString);

    if(!query.size()) {

        QMessageBox::critical(this, "Registration Centre Error","This machine has not yet been assigned to a registration centre. Please specify a Registration Centre in the Control Panel", QMessageBox::Ok);
        return;
    }

}


void DashBoard::closeEvent(QCloseEvent *ev) {

//    ev->accept();
//    return;
    //ev->ignore();
    return; // THIS LINE SHOULD BE COMMENTED OUT IN THE PRODUCTION SYSTEM
    QProcess::execute("sudo poweroff -f -p");

}
