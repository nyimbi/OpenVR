#include "configdialog.h"
#include "ui_configdialog.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlField>


QString sDelimitationSplitter = "  - ";
QSqlQuery dbSqlQuery;

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);

    stateId = 0;
    lgaId = 0;
    loadStates();

    QValidator *textRx = new QRegExpValidator(QRegExp("^[a-zA-Z0-9\\s-\\.,_/]*"), this);
    QValidator *numRx = new QRegExpValidator(QRegExp("^[0-9]*"), this);

    ui->inpRaName->setValidator(textRx);
    ui->inpPuName->setValidator(textRx);

    ui->inpRaCode->setValidator(numRx);
    ui->inpPuCode->setValidator(numRx);


}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::changeEvent(QEvent *e)
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

void ConfigDialog::on_btnCreateRa_clicked()
{

    QString errorMessages;


    if(ui->inpState->currentIndex()==0) {

        errorMessages+="\nState";

    }

    if(ui->inpLga->currentIndex() == 0) {

        errorMessages+="\nLocal Government";

    }

    if(ui->inpRaName->text() == ""){

        errorMessages+="\nMissing Registration Area Name ";

    }

    if(ui->inpRaCode->text() == "" ) {

        errorMessages+="\nMissing Registration Area Code";

    }

    if( ui->inpRaCode->text().toInt() > 999 || ui->inpRaCode->text().toInt() < 1 ) {

        errorMessages+="\nInvalid Registration Area Code. Must only contain a number and must be less than 1000";

    }

    if(errorMessages.isNull()){

        qDebug("Form Validated");

        //check if RA Code exists in this LGA, if yes, then disallow

        int raCode = ui->inpRaCode->text().toInt();

        QSqlQuery dbQuery;

        QString queryString= "";

        queryString.sprintf("SELECT id,name FROM registration_areas WHERE  local_government_id='%d' AND abbreviation=%d",lgaId,raCode);

        dbQuery.exec(queryString);

        if(dbQuery.size()) {

            dbQuery.next();

            QString existingRaName = dbQuery.value(1).toString();

            errorMessages.sprintf("Another Registration Area with the same RA Code of %d exists - %s",raCode,existingRaName.toUtf8().data());

            QMessageBox::critical(this, "Error Saving Registration Area",errorMessages, QMessageBox::Ok);

            return;

        }

        int proposedRaId = 1000 + lgaId;

        qDebug() << " Proposed RA ID: " << proposedRaId;
        QString strRaCode="";
        if(raCode < 10 ){

            strRaCode.sprintf("00%d",raCode);

        }else if( raCode >=10 && raCode < 100 ){

            strRaCode.sprintf("0%d",raCode);

        }else if(raCode >= 100){
            strRaCode.sprintf("%d",raCode);
        }

        qDebug()<< "RA CODE: "<< strRaCode;
        QString newRaId;
        newRaId.sprintf("%d%s",proposedRaId,strRaCode.toUtf8().data());


        //at this point, it should allow it to be saved
        QSqlTableModel *regModel = new QSqlTableModel;
        regModel->setTable("registration_areas");

        QSqlRecord dbRecord;

        QSqlField id("id", QVariant::Int);
        id.setValue(newRaId.toInt());
        dbRecord.append(id);

        QSqlField raLgaId("local_government_id", QVariant::String);
        raLgaId.setValue(lgaId);
        dbRecord.append(raLgaId);


        QSqlField raN("name", QVariant::String);
        raN.setValue(ui->inpRaName->text());
        dbRecord.append(raN);

        QSqlField raC("abbreviation", QVariant::String);
        raC.setValue(ui->inpRaCode->text());
        dbRecord.append(raC);

        if(regModel->insertRecord(-1, dbRecord)){

            qDebug() << "New RA Created! ";

            QString successMessage;
            successMessage.sprintf("Registration Area  has been created.\nTo ensure that this RA is recognized at the State,\nplease inform support of this ID: %d. \n\nProceed to the Create PU tab to create the Polling Units in this Registration Area",newRaId.toInt());
            QMessageBox::information(this, "Registration Area Created",successMessage, QMessageBox::Ok);

            clearForms();
            return;

        }else{
            QMessageBox::critical(this, "Error Saving Registration Area","An unexpected error occurred. Please check your input and try again", QMessageBox::Ok);
            return;
        }

    } else {
        //qDebug("Form Error " + errorMessages.toUtf8());
        qDebug() << "Form Errors: " << errorMessages;
        QMessageBox::critical(this, "Error Saving Registration Area","Please check that you have provided the following\n "+errorMessages, QMessageBox::Ok);
        return;
    }


}



void ConfigDialog::loadStates()
{
    QSqlQuery dbQuery;
    dbQuery.exec("SELECT id,name,abbreviation FROM states");

    QString stateId;
    QString stateName;

    ui->inpState->addItem("");
    ui->inpPuState->addItem("");

    if(!dbQuery.size()){
        return;
    }
    while (dbQuery.next()) {
        stateId = dbQuery.value(0).toString();
//        stateName = dbQuery.value(1).toString();
        stateName = dbQuery.value(2).toString() + sDelimitationSplitter .toUtf8() + dbQuery.value(1).toString();;
        ui->inpState->addItem(stateName,stateId);
        ui->inpPuState->addItem(stateName,stateId);
    }

}

void ConfigDialog::loadLgas(QString stateName,QComboBox *targetCombo )
{



    qDebug() << stateName;

    QSqlQuery dbQuery;

    QStringList stateNameList = stateName.split(sDelimitationSplitter ,QString::KeepEmptyParts);

    stateName = stateNameList.at(0);

    QString queryString;



    targetCombo->clear();

    targetCombo->addItem("");

    if(stateName.isEmpty()){
        return;
    }



    if(!dbQuery.exec("SELECT id FROM states WHERE  abbreviation='"+stateName+"'")){
        QMessageBox::critical(this, "Query Error","Unable to select: \n"+dbQuery.lastError().text()+"\n"+dbQuery.lastQuery(),QMessageBox::Cancel);
    }
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();
    stateId = dbQuery.value(0).toInt();

    queryString.sprintf("SELECT id,name,abbreviation FROM local_governments WHERE  state_id=%d",stateId);

    if(!dbQuery.exec(queryString)){
        QMessageBox::critical(this, "Query Error","Unable to select: \n"+dbQuery.lastError().text()+"\n"+dbQuery.lastQuery(),QMessageBox::Cancel);
    }


    if(!dbQuery.size()){
        return;
    }
    QString lgaName;
    while (dbQuery.next()) {
        lgaName = dbQuery.value(2).toString() + sDelimitationSplitter.toUtf8()+ dbQuery.value(1).toString();
        targetCombo->addItem(lgaName,dbQuery.value(0).toString());
    }

}


void ConfigDialog::on_inpState_currentIndexChanged(QString stateName)
{
    loadLgas(stateName,ui->inpLga);

}

void ConfigDialog::on_inpLga_currentIndexChanged(QString currentLga)
{
    if(currentLga.isEmpty()){
        return;
    }

    QStringList lgaNameList = currentLga.split(sDelimitationSplitter,QString::KeepEmptyParts);
    currentLga = lgaNameList.at(0);


    QSqlQuery dbQuery;

    QString queryString = "SELECT id FROM local_governments WHERE  abbreviation='"+currentLga+"' AND state_id="+QString::number(stateId).toStdString().c_str();
    qDebug() <<queryString;

    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    lgaId = dbQuery.value(0).toInt();
    qDebug()<< "Selected LGA :" << lgaId;
}


void ConfigDialog::clearForms(){
    ui->inpState->setCurrentIndex(0);
    ui->inpRaName->clear();
    ui->inpRaCode->clear();
    ui->inpPuName->clear();
    ui->inpPuCode->clear();

    ui->inpPuState->setCurrentIndex(0);



}

QString ConfigDialog::createRaId(int lgaId,int raCode){


    int proposedRaId = 1000 + lgaId;

    qDebug() << " Proposed RA ID: " << proposedRaId;
    QString strRaCode="";
    if(raCode < 10 ){

        strRaCode.sprintf("00%d",raCode);

    }else if( raCode >=10 && raCode < 100 ){

        strRaCode.sprintf("0%d",raCode);

    }else if(raCode >= 100){
        strRaCode.sprintf("%d",raCode);
    }

    qDebug()<< "RA CODE: "<< strRaCode;
    QString newRaId;
    newRaId.sprintf("%d%s",proposedRaId,strRaCode.toUtf8().data());
    return newRaId;

}

void ConfigDialog::on_inpPuState_currentIndexChanged(QString stateName)
{
    loadLgas(stateName,ui->inpPuLga);
}

void ConfigDialog::on_inpPuLga_currentIndexChanged(QString currentLga)
{
     loadRas(currentLga,ui->inpPuRa);

}

void ConfigDialog::loadRas(QString currentLga,QComboBox *targetRaCombo)
{
    targetRaCombo->clear();


    //selectedPuId=0;

    if(currentLga.isEmpty()){
        return;
    }

    QStringList lgaNameList = currentLga.split(sDelimitationSplitter,QString::KeepEmptyParts);
    currentLga = lgaNameList.at(0);


    QSqlQuery dbQuery;

    QString queryString = "SELECT id FROM local_governments WHERE  abbreviation='"+currentLga+"' AND state_id="+QString::number(stateId).toStdString().c_str();
    qDebug() <<queryString;

    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    lgaId = dbQuery.value(0).toInt();


    queryString.sprintf("SELECT name,abbreviation FROM registration_areas WHERE  local_government_id='%d'",lgaId);
    qDebug() <<queryString;
    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }

    targetRaCombo->addItem("");
    QString raName;
    while (dbQuery.next()) {

       raName =  dbQuery.value(1).toString() + sDelimitationSplitter.toUtf8()+ dbQuery.value(0).toString();
       targetRaCombo->addItem(raName);
    }


}

void ConfigDialog::on_tabWidget_currentChanged(int index)
{
    clearForms();
    lgaId=0;
    stateId=0;

}

void ConfigDialog::on_inpPuRa_currentIndexChanged(QString repArea)
{
    if(repArea.isEmpty()){
        return;
    }

    QStringList repNameList = repArea.split(sDelimitationSplitter,QString::KeepEmptyParts);
    repArea = repNameList.at(0);


    //This should be done with foreign key index rather than what is here now
    QSqlQuery dbQuery;

    QString queryString = "SELECT id FROM registration_areas WHERE  abbreviation='"+repArea+"' AND local_government_id="+QString::number(lgaId).toStdString().c_str();

    qDebug() <<queryString;

    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    raId =  dbQuery.value(0).toInt();

    qDebug() << " Selected RA: " << raId;
}
/*
void ConfigDialog::loadPollingUnits(QString repArea)
{

    ui->inpPu->clear();

    selectedPuId=0;

    if(repArea.isEmpty()){
        return;
    }

    QStringList repNameList = repArea.split(delimitationSplitter,QString::KeepEmptyParts);
    repArea = repNameList.at(0);


    //This should be done with foreign key index rather than what is here now
    QSqlQuery dbQuery;

    QString queryString = "SELECT id FROM registration_areas WHERE  abbreviation='"+repArea+"' AND local_government_id="+QString::number(selectedLgaId).toStdString().c_str();

    qDebug() <<queryString;

    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    raId =  dbQuery.value(0).toInt();

    //selectedRaId
    */
/*
    queryString.sprintf("SELECT name,abbreviation FROM polling_units WHERE  registration_area_id='%d'",selectedRaId);
    qDebug() <<queryString;
    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }
    QString puName;
    ui->inpPu->addItem("");
    while (dbQuery.next()) {
        puName = dbQuery.value(1).toString() + delimitationSplitter.toUtf8() +dbQuery.value(0).toString();
        ui->inpPu->addItem(puName);
    }
*/
//}

void ConfigDialog::on_btnCreatePu_clicked()
{
    QString errorMessages;


    if(ui->inpPuState->currentIndex()==0) {

        errorMessages+="\nState";

    }

    if(ui->inpPuLga->currentIndex() == 0) {

        errorMessages+="\nLocal Government";

    }


    if(ui->inpPuRa->currentIndex() == 0) {

        errorMessages+="\nRegistration Area";

    }


    if(ui->inpPuName->text() == ""){

        errorMessages+="\nMissing Registration Center (PU) Name ";

    }

    if(ui->inpPuCode->text() == "" ) {

        errorMessages+="\nMissing Registration Center (PU) Code";

    }

    if( ui->inpPuCode->text().toInt() > 999 || ui->inpPuCode->text().toInt() < 1 ) {

        errorMessages+="\nInvalid Registration Center Code. Must only contain a number and must be less than 1000";

    }

    if(errorMessages.isNull()){

        qDebug("Form Validated");

        //check if Pu Code exists in this RA, if yes, then disallow

        int puCode = ui->inpPuCode->text().toInt();

        QSqlQuery dbQuery;

        QString queryString= "";

        queryString.sprintf("SELECT id,name FROM polling_units WHERE registration_area_id='%d' AND abbreviation=%d",raId,puCode);

        dbQuery.exec(queryString);

        if(dbQuery.size()) {

            dbQuery.next();

            QString existingPuName = dbQuery.value(1).toString();

            errorMessages.sprintf("Another Registration Center/PU with the same PU Code of %d exists - %s",puCode,existingPuName.toUtf8().data());

            QMessageBox::critical(this, "Error Saving Registration Center/PU",errorMessages, QMessageBox::Ok);

            return;

        }

        queryString.sprintf("SELECT abbreviation FROM registration_areas WHERE id='%d'",raId);

        dbQuery.exec(queryString);

        if(!dbQuery.size()) {
            QMessageBox::critical(this, "Error Saving Registration Center/PU","Registration Area Not Found. Please check and try again. Or restart DDC", QMessageBox::Ok);
            return;
        }

        dbQuery.next();
        QString raCode= dbQuery.value(0).toString();


        QString strProposedPuId;

        if(puCode<10){

            strProposedPuId.sprintf("00%d",puCode);

        }else if(puCode<100){ //and obviously greater than or equal to 10

            strProposedPuId.sprintf("0%d",puCode);

        }else{
            strProposedPuId.sprintf("%d",puCode);
        }


        qDebug() << " Proposed U ID: " << strProposedPuId;


        qDebug()<< "PU CODE Suffix: "<< strProposedPuId;

        QString assignedRaId = createRaId(lgaId,raCode.toInt());
        QString newPuId;

        newPuId.sprintf("%s%s",assignedRaId.toUtf8().data(),strProposedPuId.toUtf8().data());

        qDebug()<< "Final PU CODE: "<< newPuId;

        //at this point, it should allow it to be saved
        QSqlTableModel *regModel = new QSqlTableModel;
        regModel->setTable("polling_units");

        QSqlRecord dbRecord;

        QSqlField id("id", QVariant::Int);
        id.setValue(newPuId.toInt());
        dbRecord.append(id);

        QSqlField shtRaId("registration_area_id", QVariant::String);
        shtRaId.setValue(raId);
        dbRecord.append(shtRaId);


        QSqlField raN("name", QVariant::String);
        raN.setValue(ui->inpPuName->text());
        dbRecord.append(raN);

        QSqlField raC("abbreviation", QVariant::String);
        raC.setValue(ui->inpPuCode->text());
        dbRecord.append(raC);

        if(regModel->insertRecord(-1, dbRecord)){

            qDebug() << "New PU Created! ";

            QString successMessage;
            successMessage.sprintf("Registration Center  has been created.\nTo ensure that this Registration Center is recognized at the State,\nplease inform support of this ID: %d. \n\nProceed to the Control Panel to assign this machine to this Registration Center",newPuId.toInt());
            QMessageBox::information(this, "Registration Center Created",successMessage, QMessageBox::Ok);

            clearForms();
            return;

        }else{
            QMessageBox::critical(this, "Error Saving Registration Center","An unexpected error occurred. Please check your input and try again", QMessageBox::Ok);
            return;
        }

    } else {
        //qDebug("Form Error " + errorMessages.toUtf8());
        qDebug() << "Form Errors: " << errorMessages;
        QMessageBox::critical(this, "Error Saving Registration Area","Please check that you have provided the following\n "+errorMessages, QMessageBox::Ok);
        return;
    }

}
