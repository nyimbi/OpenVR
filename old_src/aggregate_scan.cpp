#include <QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QMessageBox>

#include "aggregate_scan.h"
#include "ui_aggregate_scan.h"
#include "aggregate_bozorth_check.h"

int subProcessUnverified;
int subProcessVerified=0;

int mainProcessTotal;
int mainProcessVerified = 0;

int totalProgress;
int currentProgress;

int totalUnverified;

int processStarted;



aggregate_scan::aggregate_scan(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aggregate_scan)
{
    ui->setupUi(this);
//    runScan();
}

aggregate_scan::~aggregate_scan()
{
    delete ui;
}

void aggregate_scan::changeEvent(QEvent *e)
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



void aggregate_scan::runScan(){

    QDateTime currentSystemTime = QDateTime::currentDateTime();

    processStarted = currentSystemTime.toTime_t();


    //ui->statusBar->showMessage();


    this->continueAfis  = true;

    ui->btnRunAfis->setEnabled(false);
    ui->btnCancel->setEnabled(true);


    QString sqlQuery = "SELECT count(*) as total_registrations FROM `registrations`";

    QSqlQuery dbQuery;

    dbQuery.exec(sqlQuery);

    dbQuery.next();

    QString totalRegistrations = dbQuery.value(0).toString();

    //aqdebug() << "Total Registrations " << totalRegistrations;

    ui->lblTotalSet->setText(totalRegistrations);

    aggregate_bozorth_check *bzCheck = new aggregate_bozorth_check(this);

    connect(bzCheck,SIGNAL(currentSubjectVin(QString)),this,SLOT(setCurrentVin(QString)));
    connect(bzCheck,SIGNAL(currentTargetSize(int)),this,SLOT(setCurrentTargetSize(int)));
    connect(bzCheck,SIGNAL(switchingSubjectVin()),this,SLOT(switchSubjectVin()));
    //Next  line commented out because it was too  cpu intensive
    //    connect(bzCheck,SIGNAL(currentTargetVin(QString)),this,SLOT(setCurrentTarget(QString)));
    connect(bzCheck,SIGNAL(jobCompleted()),this,SLOT(jobCompleted()));
    //connect(bzCheck,SIGNAL(scanResult(QString )),this,SLOT(processScanResult(QString)));
    connect(bzCheck,SIGNAL(setProcessStage(int)),this,SLOT(setCurrentProgressBar(int)));
    connect(bzCheck,SIGNAL(setTotalSize(int)),this,SLOT(setUnverifiedSize(int)));
    connect(bzCheck,SIGNAL(jobCancelled()),this,SLOT(jobCancelled()));

    future = QtConcurrent::run(bzCheck, &aggregate_bozorth_check::startAfis);

}



void aggregate_scan::setCurrentTargetSize(int tSize){


    ui->lblTargetSize->setText(QString::number(tSize).toStdString().c_str());
    ui->pbCurrent->setValue(0);
    ui->pbCurrent->setMaximum(tSize);
    subProcessVerified=0;



}

void aggregate_scan::setCurrentTarget(QString cTarget){

    subProcessVerified++;
    ui->pbCurrent->setValue(subProcessVerified);


}

void aggregate_scan::setCurrentVin(QString sVin){


    ui->lblCurrentVin->setText(sVin);


}

void aggregate_scan::switchSubjectVin(){

    mainProcessVerified++;

    int percentComplete = (mainProcessVerified*100)/mainProcessTotal;

    ui->pbTotal->setValue(percentComplete);

}


void aggregate_scan::updateCurrentProgress(int nowVerified){


}

void aggregate_scan::updateTotalProgress(int nowVerified){

}

void aggregate_scan::on_btnRunAfis_clicked()
{
    runScan();
}

void aggregate_scan::jobCompleted(){

    //aqdebug()<< "exiting";
    QSqlQuery dbQuery;
    dbQuery.exec("TRUNCATE TABLE tmp_afis_scans");
    ui->btnRunAfis->setEnabled(true);
    ui->btnCancel->setEnabled(false);
    //    exit(0);
    QDateTime currentSystemTime = QDateTime::currentDateTime();

    int processEnded = currentSystemTime.toTime_t();

    QString msg;
    msg.sprintf("The scan has been completed!\nTook %d seconds",processEnded-processStarted);

    QMessageBox::information(this,"Scan Completed",msg, QMessageBox::Ok);


}

void aggregate_scan::setUnverifiedSize(int totalUnverified) {

    //    QString totalVerified = QString::number((totalRegistrations.toInt() - totalUnverified.toInt())).toStdString().c_str();
    //    ui->lblVerified->setText(totalVerified);
    mainProcessTotal = totalUnverified;
    ui->lblUnverified->setText(QString::number(totalUnverified).toStdString().c_str());

}

void aggregate_scan::setCurrentProgressBar(int stage){

    ui->pbCurrent->setMaximum(5);

    ui->pbCurrent->setValue(stage);

}

void aggregate_scan::on_btnCancel_clicked()
{

    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setBackgroundRole(QPalette::Base);
    msgBox.setAutoFillBackground(true);
    msgBox.setWindowFlags(Qt::Dialog);
    msgBox.setText("Are you sure you want to cancel the scan?");
    msgBox.setWindowTitle("Cancel AFIS Scan");

    switch (msgBox.exec()) {

    case QMessageBox::Yes:
        this->continueAfis  = false;

        ui->btnCancel->setText("Cancelling...");

        ui->btnCancel->setEnabled(false);

        break;
    case QMessageBox::No:
    default:

        break;


    }



}

void aggregate_scan::jobCancelled(){

    ui->btnCancel->setText("Cancel");

    ui->btnRunAfis->setEnabled(true);

    QDateTime currentSystemTime = QDateTime::currentDateTime();

    int processEnded = currentSystemTime.toTime_t();


    QString msg;

    msg.sprintf("The scan has been Cancelled!\n Started %d  Cancelled %d Took %d seconds",processStarted,processEnded,processEnded-processStarted);

    QMessageBox::information(this,"Scan Cancelled",msg, QMessageBox::Ok);
}
