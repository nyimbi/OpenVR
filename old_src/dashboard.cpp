#include "dashboard.h"
#include "mainwindow.h"
#include "reportsdialog.h"
#include "recordsdialog.h"
#include "queuetickets.h"
#include "ui_dashboard.h"
#include "controlpanel.h"
#include "helpdialog.h"
#include "reportsdialog.h"
#include "operatorlogin.h"
#include "adminlogin.h"
#include "adminchangepassword.h"
#include "operatorreg.h"
#include "adminreg.h"
#include "changepassword.h"
#include "backup.h"
#include "ntfsfix.h"
#include "import.h"
#include "importlatest.h"
#include "aggregate_scan.h"
#include "configdialog.h"
#include "discbackup.h"


#include "QSqlQuery"
#include "QMessageBox"
#include "QInputDialog"
#include "QDesktopWidget"

#include "QBuffer"
#include "QByteArray"
#include "QTimer"
#include "QProcess"
#include "QSqlTableModel"

#include "devicecheck.h"
//#include "finger_device_check.h"

#include <QtCore>

QTimer *backupTimer;
QTimer *importTimer;
QString deviceOkStyle = "border:1px solid grey; background-color:rgb(208, 255, 179);padding:10px;";
QString deviceErrStyle = "border:1px solid grey; background-color:rgb(255, 196, 196);padding:10px;";
QString textRegExp = "^[a-zA-Z0-9\\s-\\.,]*"; // Whitelist filter for all text input. Prevents SQL Injection
bool allowAppShutDown = false;

QString lastAdminLogin;

bool checkHardware = true;
bool hasValidPU = false;
DashBoard::DashBoard(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::DashBoard)
{
    ui->setupUi(this);

    QDateTime currentSystemTime = QDateTime::currentDateTime();


    //if(currentSystemTime.toTime_t() > 1295071200){ //1295071200 Saturday January 15, 2010
    //if(currentSystemTime.toTime_t() > 1295244000) { //1295071200 Saturday January 15, 2010
    if(currentSystemTime.toTime_t() > 1295416800) { //1295071200 Saturday January 15, 2010
        ui->btnWipeOut->hide();
    }

    this->showFullScreen();


    connect(ui->btnReports, SIGNAL(clicked()), this, SLOT(printReports()));
    connect(ui->btnVotersCard, SIGNAL(clicked()), this, SLOT(printVotersCard()));
    connect(ui->btnQueueTickets, SIGNAL(clicked()), this, SLOT(queueTickets()));

    connect(ui->btnBackup, SIGNAL(clicked()), this, SLOT(backupAll()));
    connect(ui->actionImport_Backups, SIGNAL(triggered()), this, SLOT(importBackups()));
    connect(ui->actionPrinter_Setup, SIGNAL(triggered()), this, SLOT(printerSetup()));
    connect(ui->actionManage_Printers, SIGNAL(triggered()), this, SLOT(printerConfig()));

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
    connect(devChecker, SIGNAL(batteryStatusUpdate(QString)), this, SLOT(batteryStatusChanged(QString)));
    connect(devChecker, SIGNAL(batteryLevelUpdate(int)), this, SLOT(batteryLevelChanged(int)));
    devFuture = QtConcurrent::run(devChecker, &DeviceCheck::checkTimer);
    checkRCSetting();
    restoreAudioVolume();
    removeRedundantPrinter();//hack to remove that printer CN09K26MFR05D2
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
        case Qt::Key_F7:
            ui->btnControlPanel->click();
            break;
        case Qt::Key_F6:
            ui->btnBackup->click();
            break;
       // case Qt::Key_F7:
//            ui->btnImport->click();
//            break;
        case Qt::Key_BraceRight:
            hiddenDialog();
            break;
        case Qt::Key_F12:
            ui->btnShutdown->click();
            break;
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
        default: QWidget::keyPressEvent(e);
            //qDebug() << "Pressed Key: " << e->key();
            break;
    }
}

void DashBoard::on_btnRegistration_clicked()
{

    checkRCSetting();
    if(hasValidPU == false){
        QMessageBox::critical(this, "Voter Registration Centre Error","You cannot register voters unless the machine has been assigned to a Registration Centre by an Administrator", QMessageBox::Ok);
        return;
    }





    QString operatorCodeValue  = operatorLogin();
    bool isAuthorizedOperator = (operatorCodeValue=="")? false:true;

    if(!isAuthorizedOperator) {
        QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again.\n\nIf you have NOT registered this account, please do so from the Operator Registration menu", QMessageBox::Ok);
        return;
    }

    MainWindow *m = new MainWindow(this,operatorCodeValue);
    m->setRegExp(textRegExp);
    m->loggedOperatorCode = operatorCodeValue;
    connect(m,SIGNAL(closingMain()),this,SLOT(restoreDashboard()));
    checkHardware = false;
    m->setWindowFlags(Qt::Dialog);

    QDesktopWidget *xa = QApplication::desktop();
    m->setGeometry(0,0,xa->screenGeometry().width(),xa->screenGeometry().height());

    //m->showMaximized();
    //m->showFullScreen();
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
    reportsDialog->operatorId = operatorCodeValue;
    reportsDialog->exec();
    checkHardware = true;
}



void DashBoard::printVotersCard()
{
    QString operatorCodeValue  = operatorLogin();
    bool isAuthorizedOperator = (operatorCodeValue=="")? false:true;

    if (!isAuthorizedOperator) {
        QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again", QMessageBox::Ok);
        return;
    }

    RecordsDialog *recordsDialog = new RecordsDialog(this);
    recordsDialog->setRegExp(textRegExp);

    connect(recordsDialog,SIGNAL(finished(int)),this,SLOT(restoreDashboard()));
    checkHardware = false;
    recordsDialog->exec();
    checkHardware = true;

    //this->show();
}


void DashBoard::restoreDashboard()
{
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
            allowAppShutDown = true;
            QProcess *shutDownCall = new QProcess;

            QString gnomeCall = "sudo poweroff -f -p";

            shutDownCall->start(gnomeCall);

            this->close();
        break;
    }
}

void DashBoard::backupStatusChanged(int code)
{
//    qDebug() << "Backup Status: " << code;
    if (code) {
        ui->devRemovable->setStyleSheet(deviceOkStyle);
        ui->leRemovable->setText("Backup Online.");
    } else {
        ui->devRemovable->setStyleSheet(deviceErrStyle);
        ui->leRemovable->setText("Connect Backup.");
    }
}

void DashBoard::fpStatusChanged(int code)
{
//    qDebug() << "FP Status: " << code;
    if (code) {
        ui->devFP->setStyleSheet(deviceOkStyle);
        ui->leFP->setText("Fingerprint Reader Online.");
    } else {
        ui->devFP->setStyleSheet(deviceErrStyle);
        ui->leFP->setText("Connect Fingerprint Reader.");
    }

}

void DashBoard::cameraStatusChanged(int code)
{
//    qDebug() << "Camera Status: " << code;
    if(code){
        ui->devWebcam->setStyleSheet(deviceOkStyle);
        ui->leWebcam->setText("Webcam Online.");
    } else{
        ui->devWebcam->setStyleSheet(deviceErrStyle);
        ui->leWebcam->setText("Connect Webcam.");
    }
}

void DashBoard::printerStatusChanged(int code)
{
//    qDebug() << "Printer Status: " << code;
    if(code) {
        ui->devPrinter->setStyleSheet(deviceOkStyle);
        ui->lePrinter->setText("Printer Online.");
    } else {
        ui->devPrinter->setStyleSheet(deviceErrStyle);
        ui->lePrinter->setText("Connect Printer.");
    }
}


void DashBoard::batteryStatusChanged(QString status)
{
    ui->lbBatteryStatus->setText(status);
}


void DashBoard::batteryLevelChanged(int level)
{
    ui->batteryBar->setValue(level);
}


void DashBoard::hiddenDialog(){

    QString sCodeValue;
    bool ok = false;

    QString suppliedPassword = QInputDialog::getText(this, tr("Authorization Required"),
                                                       tr("Please Enter The Authorization Code:"), QLineEdit::Password,
                                                       "", &ok);

    if (!suppliedPassword.isEmpty()) {

        if (suppliedPassword.toUtf8()=="e101ACCeSS")
        {
            QMessageBox::information(this, "Access Granted", "Please proceed.", QMessageBox::Ok);

            ConfigDialog *cConfig   = new ConfigDialog(this);

            cConfig->exec();

        } else {
            QMessageBox::critical(this, "Authorization Error","Invalid Authorization Code.", QMessageBox::Ok);
            return ;
        }


    } else {

    }

    return;


}


void DashBoard::backupAll()
{
    QString processOutput="";
    QProcess *scriptProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    scriptProcess->start("./scripts/check_backup_storage.sh", args);

    if (!scriptProcess->waitForFinished()){
//        qDebug() << "Error :" << scriptProcess->errorString();
    } else {
        processOutput =  scriptProcess->readAll();
    }

    if (processOutput == "") {
        QMessageBox::critical(this, "Device Error","Backup Storage Not Detected. Please check and try again", QMessageBox::Ok);
        return;
    }


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
    QString processOutput="";
    QProcess *scriptProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    scriptProcess->start("./scripts/check_backup_storage.sh", args);

    if (!scriptProcess->waitForFinished()){
//        qDebug() << "Error :" << scriptProcess->errorString();
    } else {
        processOutput =  scriptProcess->readAll();
    }

    if (processOutput == "") {
        QMessageBox::critical(this, "Device Error","Backup Storage Not Detected. Please check and try again", QMessageBox::Ok);
        return;
    }

    if(controlLogin()) {
        Import *import = new Import(this);
        import->exec();
    }
}


QString DashBoard::operatorLogin()
{
//    bool ok = false;
    QString operatorCodeValue;
    QString suppliedPassword, suppliedOperatorId;
//    QString suppliedPassword = QInputDialog::getText(this, tr("Please Authorization Required"),
//                                                       tr("Please Enter Your \nOperator Access Code:"), QLineEdit::Password,
//                                                       "", &ok);
    OperatorLogin *ologin = new OperatorLogin(this);
    ologin->setRegExp(textRegExp);
    ologin->exec();

    suppliedOperatorId = ologin->operatorId;
    suppliedPassword = ologin->passwd;
    qDebug() << "Operator ID: " << ologin->operatorId;
    qDebug() << "Password: " << ologin->passwd;

    if (!suppliedPassword.isEmpty()) {
        QSqlTableModel *loginModel = new QSqlTableModel;
        loginModel->setTable("registered_operators");

        QString loginFilter = "passwd='" + suppliedPassword + "' AND oid='" + suppliedOperatorId + "'";

//        QString loginFilter;
//        loginFilter.sprintf("passwd='%s' AND oid='%s'",suppliedPassword.toUtf8().data(),suppliedOperatorId.toUtf8().data());

        qDebug() << "Login Filter: " << loginFilter;
        loginModel->setFilter(loginFilter);
        loginModel->select();

        int loginRows = loginModel->rowCount();
        qDebug() << "Login Rows: " << QString::number(loginRows);

        if (loginRows)
        {
            if (loginModel->record(0).value("active").toInt() == 1)
            {
                QMessageBox::information(this, "Access Granted", "Welcome "+ loginModel->record(0).value("first_name").toString() + " " + loginModel->record(0).value("last_name").toString() + ".\nPlease proceed.", QMessageBox::Ok);
                operatorCodeValue = suppliedOperatorId;
                return operatorCodeValue;
            } else {
                QMessageBox::critical(this, "Authorization Error","Your access code has been deactivated.", QMessageBox::Ok);
                return "";
            }
        } else {
            return "";
        }

//        QSqlQuery dbQuery;
//        QString sqlQuery= "SELECT * FROM registered_operators WHERE passwd='"+suppliedPassword.toUtf8()+"'";
//        dbQuery.exec(sqlQuery);

//        if(dbQuery.next()){
//            if(dbQuery.value(4).toInt()==1){
//                QMessageBox::information(this, "Access Granted","Welcome "+dbQuery.value(2).toString()+" "+dbQuery.value(3).toString()+".\nPlease proceed.", QMessageBox::Ok);

//                operatorCodeValue = dbQuery.value(1).toString();
//                //qDebug() << " Operator Code: "<< operatorCodeValue;
//                return operatorCodeValue;
//            } else {
//                QMessageBox::critical(this, "Authorization Error","Your access code has been deactivated.", QMessageBox::Ok);
//            }
//        }
    }

    return "";
}


bool DashBoard::controlLogin()
{
    AdminLogin *adminLogin = new AdminLogin(this);
    adminLogin->setRegExp(textRegExp);
    adminLogin->exec();

    QString suppliedAdminId, suppliedAdminPasswd;
    suppliedAdminId = adminLogin->adminId;
    suppliedAdminPasswd = adminLogin->adminPasswd;
    qDebug() << "Admin ID: " << suppliedAdminId;
    qDebug() << "Admin Password: " << suppliedAdminPasswd;

    if (!suppliedAdminPasswd.isEmpty()) {

        QSqlTableModel *adminModel = new QSqlTableModel;
        adminModel->setTable("administrators");

        QString loginFilter = "passwd='" + suppliedAdminPasswd + "' AND rid='" + suppliedAdminId + "'";

        qDebug() << "Login Filter: " << loginFilter;
        adminModel->setFilter(loginFilter);
        adminModel->select();

        int loginRows = adminModel->rowCount();
        qDebug() << "Login Rows: " << QString::number(loginRows);

        if (loginRows)
        {
            QMessageBox::information(this, "Access Granted", "Welcome "+ adminModel->record(0).value("first_name").toString() + " " + adminModel->record(0).value("last_name").toString() + ".\nPlease proceed.", QMessageBox::Ok);
            lastAdminLogin = suppliedAdminId;
            return true;

        } else {
            QMessageBox::critical(this, "Authentication Error", "Invalid Authentication Information. \n\nIf you have NOT registered this account, please do so from the Administrator Registration menu", QMessageBox::Ok);
            lastAdminLogin = "";
            return false;
        }

    }

    return false;

//    bool showDialog = false;
//    bool ok;
//    QString text = QInputDialog::getText(this, tr("Authorization Required"),
//                                         tr("Password:"), QLineEdit::Password,
//                                         "", &ok);
//    if (ok && !text.isEmpty()){
//        if(text!="password"){//save in database ?
//            QMessageBox::critical(this, "Authorization Error","Invalid password provided. Please check and try again", QMessageBox::Ok);
//            return false;
//        }else{
//            showDialog  = true;
//            checkHardware = false;
//        }
//    }

//    return showDialog;
}


void DashBoard::checkRCSetting()
{
    QSqlQuery query;
    QString queryString = "SELECT setting_value "
                          " FROM settings "
                          " WHERE setting_name =  'polling_unit_id' ";
    query.exec(queryString);

    if(!query.size()) {
        QMessageBox::critical(this, "Registration Centre Error","This machine has not yet been assigned to a registration centre. Please specify a Registration Centre in the Control Panel", QMessageBox::Ok);
        return;
    }
    hasValidPU = true;
}


void DashBoard::closeEvent(QCloseEvent *ev)
{

//    ev->accept();
//    return;
    if(allowAppShutDown){
        ev->accept();
        return;
    }
    ev->ignore();
    return; // THIS LINE SHOULD BE COMMENTED OUT IN THE PRODUCTION SYSTEM
    QProcess::execute("sudo poweroff -f -p");
}

void DashBoard::printerSetup()
{

    QProcess *printerProcess = new QProcess();
    printerProcess->start("hp-setup");
    this->hide();


    connect(printerProcess, SIGNAL(finished(int)), this, SLOT(showDash(int)));
}


void DashBoard::printerConfig()
{

    QProcess *printerProcess = new QProcess();
    printerProcess->start("system-config-printer");
    this->hide();


    connect(printerProcess, SIGNAL(finished(int)), this, SLOT(showDash(int)));
}


void DashBoard::showDash(int exitCode)
{
    this->show();
    this->showFullScreen();
}

void DashBoard::on_actionAbout_OpenVR_triggered()
{
    HelpDialog *helpDialog = new HelpDialog(this);
    helpDialog->exec();
}


void DashBoard::restoreAudioVolume(){

    //Sets the system volume to high
    QString volCommand= "/usr/bin/amixer -c 0 sset Master,0 95% >/dev/null";
    QProcess *adjustVolume;
    adjustVolume->startDetached(volCommand);

}
void DashBoard::on_actionChange_Password_triggered()
{
    ChangePassword *chgPwd = new ChangePassword(this);
    chgPwd->setRegExp(textRegExp);
    chgPwd->exec();
}

void DashBoard::on_actionOperator_Registration_triggered()
{
    OperatorReg *opReg = new OperatorReg(this);
    opReg->setRegExp(textRegExp);
    opReg->exec();
}

void DashBoard::on_actionAdministrator_Registration_triggered()
{
    AdminReg *adminReg = new AdminReg(this);
    adminReg->setRegExp(textRegExp);
    adminReg->exec();

}

void DashBoard::on_actionChange_Admin_Password_triggered()
{
    AdminChangePassword *acp = new AdminChangePassword(this);
    acp->setRegExp(textRegExp);
    acp->exec();
}

void DashBoard::on_actionLaunch_Aggregate_AFIS_triggered()
{

    aggregate_scan *aggScan = new aggregate_scan;
    aggScan->exec();


}

void DashBoard::on_actionManual_triggered()
{

    QProcess *printerProcess = new QProcess();
    printerProcess->start("firefox");
    this->hide();


    connect(printerProcess, SIGNAL(finished(int)), this, SLOT(showDash(int)));
}

void DashBoard::on_actionPrint_Aggregation_Report_triggered()
{

    if(controlLogin()) {
        ReportsDialog *reportsDialog = new ReportsDialog(this);
        checkHardware = false;
        reportsDialog->administratorId = lastAdminLogin;
        reportsDialog->printPreviewAggregationReport();

        checkHardware = true;
        //displayDelimitation();//Just in case cchanges have been made or thot to be made
    }


}

void DashBoard::removeRedundantPrinter(){

    QString strCommand ="lpadmin -x `lpstat -t|grep 'CN09K26MFR05D2'|awk '{print $3}'|tr ':' ' '` ";

    QFile prtScript("scripts/remove_printer.sh");

    prtScript.open(QFile::WriteOnly);

    QTextStream stream( &prtScript );

    stream << strCommand;
    prtScript.close();
    QProcess *removePrinterCmd = new QProcess;
    QString processOutput;
    removePrinterCmd->execute("sh ./scripts/remove_printer.sh") ;

    if (!removePrinterCmd->waitForFinished(10000)){
        //qDebug() << "Error :" << removePrinterCmd->errorString();
    } else {
        //processOutput =  removePrinterCmd->readAll();
    }
    removePrinterCmd->execute("rm ./scripts/remove_printer.sh") ;


}

void DashBoard::on_btnWipeOut_clicked()
{

    if(!controlLogin()){
        return;
    }
    QString sqlQuery;
    QSqlQuery dbQuery;

    QMessageBox msgBox;

    msgBox.setParent(this);

    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    msgBox.setBackgroundRole(QPalette::Base);

    msgBox.setAutoFillBackground(true);

    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setText("Are you sure you want to REMOVE ALL THE DATA on the DDC?\n This process may take a few minutes.");

    msgBox.setWindowTitle("WIPE-OUT DDC DATA");

    switch (msgBox.exec()) {

        case QMessageBox::Yes:

            //QMessageBox::critical(this, "Task Pre-Check", "Ensure that you connect any external hard disk/backup you have used as well.", QMessageBox::Ok);

            dbQuery.exec("DELETE FROM `fp_fingerprints` WHERE registration_id IN (SELECT vin FROM `registrations` WHERE int_created BETWEEN '2010-09-01' AND '2011-01-14')");

            dbQuery.exec("DELETE FROM `registrations` WHERE int_created BETWEEN '2010-09-01' AND '2011-01-14'");

//            dbQuery.exec("TRUNCATE TABLE experiment_results");
//            dbQuery.exec("TRUNCATE TABLE duplicate_maps");
//            dbQuery.exec("TRUNCATE TABLE tmp_afis_scans");
//            dbQuery.exec("TRUNCATE TABLE tmp_search");
//            dbQuery.exec("TRUNCATE TABLE auto_scans");
//            dbQuery.exec("TRUNCATE TABLE registered_operators");
//            dbQuery.exec("TRUNCATE TABLE administrators");
//            dbQuery.exec("UPDATE operator_codes SET is_used=0");
//            dbQuery.exec("UPDATE administrator_codes SET is_used=0");

//            QString deleteScripts = "rm -rf stored_minutiae/* ;\nfind /media/ /backups -name 'OpenVR-BK' -exec rm {} -rf \\;\nfind /media/ /backups -name '\.OpenVR-RAW' -exec rm {} -rf \\;\n";

//            QFile prtScript("scripts/clear_ddc.sh");

//            prtScript.open(QFile::WriteOnly);

//            QTextStream stream( &prtScript );

//            stream << deleteScripts;

//            prtScript.close();

//            QProcess *clearDdcCmd = new QProcess;

//            clearDdcCmd->execute("sh ./scripts/clear_ddc.sh");

//            clearDdcCmd->waitForFinished(10000);

//            clearDdcCmd->execute("rm ./scripts/clear_ddc.sh");

            QMessageBox::critical(this, "DDC Cleared", "All previous training data cleared.", QMessageBox::Ok);

        break;

    }

}

void DashBoard::on_actionBackup_to_CD_DVD_triggered()
{

    //if( operatorLogin() == "" ) {

    //}else{

        DiscBackup *discBackup = new DiscBackup(this);
        discBackup->exec();

    //}



}

void DashBoard::on_actionImport_Latest_Backups_triggered()
{
    QString processOutput="";
    QProcess *scriptProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    scriptProcess->start("./scripts/check_backup_storage.sh", args);

    if (!scriptProcess->waitForFinished()){
//        qDebug() << "Error :" << scriptProcess->errorString();
    } else {
        processOutput =  scriptProcess->readAll();
    }

    if (processOutput == "") {
        QMessageBox::critical(this, "Device Error","Backup Storage Not Detected. Please check and try again", QMessageBox::Ok);
        return;
    }

    if(controlLogin()) {
        ImportLatest *importLatest = new ImportLatest(this);
        importLatest->exec();
    }
}

void DashBoard::on_actionRepair_NTFS_Backup_Drive_triggered()
{
    QString processOutput="";
    QProcess *scriptProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    scriptProcess->start("./scripts/check_ntfs_backup_storage.sh", args);

    if (!scriptProcess->waitForFinished()){
//        qDebug() << "Error :" << scriptProcess->errorString();
    } else {
        processOutput =  scriptProcess->readAll();
    }

    if (processOutput == "") {
        QMessageBox::critical(this, "Device Error","Backup Storage Not Detected. Please check that an NTFS backup is connected and try again", QMessageBox::Ok);
        return;
    }

    if(controlLogin()) {
        NtfsFix *ntfsFix = new NtfsFix(this);
        ntfsFix->exec();
    }
}
