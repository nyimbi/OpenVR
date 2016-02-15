#include "duplicatereg.h"
#include "ui_duplicatereg.h"
#include "mainwindow.h"
#include "QSqlQuery"
#include "QSqlRecord"
#include "QDebug"
#include "QImage"
#include "QMessageBox"

QStringList duplicateVinList;
int currentVinIndex=0;
bool allowClose= false;

DuplicateReg::DuplicateReg(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::DuplicateReg)
{
    ui->setupUi(this);

 //   setWindowFlags(Qt::CustomizeWindowHint);

    MainWindow *m_ui;
    m_ui = qobject_cast<MainWindow *>(parent);



    ui->lblNewLastName->setText(m_ui->prevLastName);
    ui->lblNewFirstName->setText(m_ui->prevFirstName);
    ui->lblNewMiddleName->setText(m_ui->prevMiddleName);
    QString dob;
    dob.sprintf("%s-%s-%s",m_ui->prevDobDay.toUtf8().data(),m_ui->prevDobMonth.toUtf8().data(),m_ui->prevDobYear.toUtf8().data());
    ui->lblNewDob->setText(dob);
    ui->lblNewGender->setText(m_ui->prevGender);
    ui->lblNewOccupation->setText(m_ui->prevOccupation);
    ui->lblNewAddress->setText(m_ui->prevAddress);
    ui->lblNewMobile->setText(m_ui->prevMobile);



    QString sqlQuery;

    sqlQuery.sprintf("SELECT gallery_file from tmp_search WHERE score >= %d AND fingers >= %d",m_ui->prevBzScoreMaximum,m_ui->prevBzGalleryMaximum);

    QSqlQuery dbQuery(sqlQuery);

    duplicateVinList.clear();

    while(dbQuery.next()){

        duplicateVinList.append(dbQuery.value(0).toString());

    }

    if(duplicateVinList.size()){
        ui->lblDupCount->setText(QString::number(duplicateVinList.size()).toStdString().c_str());
        loadVinDetails(currentVinIndex);
    }else{
        qDebug() << " This dialog should NOT be displayed! I wonder how and why you got here";
    }


}

DuplicateReg::~DuplicateReg()
{
    delete ui;
}

void DuplicateReg::changeEvent(QEvent *e)
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

void DuplicateReg::loadVinDetails(int currentIndex){


    QString selectedVin;

    try{
        selectedVin = duplicateVinList.at(currentIndex);
    }catch(...){
        qDebug() << "Assert Failure on currentIndex" << currentIndex;
        return;
    }

    if(selectedVin.isEmpty() || selectedVin.isNull()){
        qDebug() << "No duplicate at " << currentIndex;
        return;
    }

    currentVinIndex = currentIndex;

    QString sqlQuery;

    sqlQuery.sprintf("SELECT registrations.*,registered_operators.first_name op_first_name,registered_operators.last_name op_last_name from registrations LEFT JOIN registered_operators ON registered_operators.oid = registrations.operator_code WHERE vin='%s'",selectedVin.toUtf8().data());

    QSqlQuery dbQuery(sqlQuery);

    if(!dbQuery.size()){
        updateNavButtons();
        qDebug() << " Record not found with vin " << selectedVin;
        return;
    }

    dbQuery.next();

    ui->lblDupVin->setText(dbQuery.value(dbQuery.record().indexOf("vin")).toString());

    ui->lblDupCreated->setText(dbQuery.value(dbQuery.record().indexOf("int_created")).toString());

    QString regOperator = dbQuery.value(dbQuery.record().indexOf("op_last_name")).toString() + " " + dbQuery.value(dbQuery.record().indexOf("op_last_name")).toString();
    ui->lblDupRegistrar->setText(regOperator);

    ui->lblDupLastName->setText(dbQuery.value(dbQuery.record().indexOf("last_name")).toString());
    ui->lblDupFirstName->setText(dbQuery.value(dbQuery.record().indexOf("first_name")).toString());
    ui->lblDupMiddleName->setText(dbQuery.value(dbQuery.record().indexOf("other_names")).toString());

    QString dob = dbQuery.value(dbQuery.record().indexOf("dob_day")).toString() + "-" + dbQuery.value(dbQuery.record().indexOf("dob_month")).toString() +"-"+ dbQuery.value(dbQuery.record().indexOf("dob_year")).toString();

    ui->lblDupDob->setText(dob);


    ui->lblDupGender->setText(dbQuery.value(dbQuery.record().indexOf("gender")).toString());

    ui->lblDupAddress->setText(dbQuery.value(dbQuery.record().indexOf("home_address")).toString());

    ui->lblDupDelim->setText(dbQuery.value(dbQuery.record().indexOf("delimitation")).toString());

    ui->lblDupMobile->setText(dbQuery.value(dbQuery.record().indexOf("mobile_number")).toString());

    QVariant photoData = dbQuery.value(dbQuery.record().indexOf("photo_front"));

    QByteArray photoBytes = photoData.toByteArray();

    QImage photoFront;

    photoFront.loadFromData(photoBytes);

    ui->lblDupPhoto->setPixmap(QPixmap::fromImage(photoFront));


    updateNavButtons();

}


void DuplicateReg::on_btnNavPrev_clicked()
{

    if(currentVinIndex==0){
        return;
    }
    int currentIndex =currentVinIndex-1;

    loadVinDetails(currentIndex);

}

void DuplicateReg::on_btnNavNext_clicked()
{
    if(currentVinIndex>=duplicateVinList.size()-1){
        return;
    }
    int currentIndex =currentVinIndex+1;

    loadVinDetails(currentIndex);
}


void DuplicateReg::updateNavButtons() {

    int listSize = duplicateVinList.size();
    qDebug() << "List Size" <<listSize;

    if(listSize == 1 ){
        ui->btnNavPrev->hide();
        ui->btnNavNext->hide();
        return;
    }

    if(currentVinIndex==0){

        ui->btnNavNext->show();
        ui->btnNavPrev->hide();
        return;
    }

    if(currentVinIndex >= (listSize - 1)){

        ui->btnNavNext->hide();
        ui->btnNavPrev->show();
        return;
    }

    if(currentVinIndex > 0 && currentVinIndex < (listSize-1)){
        //ui->btnNavNext->show();
        ui->btnNavPrev->show();
    }

}

void DuplicateReg::closeEvent(QCloseEvent *ev) {

    if(allowClose == true){
        ev->accept();
        return;
    }

    ev->ignore();

}

void DuplicateReg::reject() {
    //to ignore ESC key
    return;

}


void DuplicateReg::on_btnRejectReg_clicked()
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setBackgroundRole(QPalette::Base);
    msgBox.setAutoFillBackground(true);
    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setText("Are you sure you want to <b>REJECT</b> this registration attempt?");
    msgBox.setWindowTitle("Reject Registration?");
    msgBox.setParent(this);

    switch (msgBox.exec()) {

    case QMessageBox::Yes:

        emit(rejectRegistration());
        allowClose = true;
        this->close();
        break;

    }







}


void DuplicateReg::on_btnAcceptReg_clicked()
{
    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setBackgroundRole(QPalette::Base);
    msgBox.setAutoFillBackground(true);
    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setText("Are you sure you want to <b>ACCEPT</b> this registration?<br /><b>NOTE:</b> This record will be flagged as a duplicate and requires administrative approval before it will be accepted into the Provisional Voters Register.");
    msgBox.setWindowTitle("Accept Registration?");
    msgBox.setParent(this);

    switch (msgBox.exec()) {

    case QMessageBox::Yes:

        emit(acceptRegistration());
        allowClose = true;
        this->close();
        break;

    }


}
