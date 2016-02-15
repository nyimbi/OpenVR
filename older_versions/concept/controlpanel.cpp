#include "controlpanel.h"
#include "ui_controlpanel.h"
#include "QSqlQuery"
#include "QSqlError"
#include "QDebug"
#include "QMessageBox"
#include "QSqlRelationalTableModel"
#include "QSqlField"
#include "QSqlRecord"
#include "QFile"
#include "QFileDialog"

int selectedStateId=0;
int selectedLgaId=0;
int selectedRaId=0;
int selectedPuId=0;
int ddcOperators[100];
int selectedOperator;
QString delimitationSplitter = "  - ";
QSqlRelationalTableModel *operatorModel;
QSqlRecord operatorRecord;


QSqlQueryModel *fp, *af;

QString macAddress;
QString macAddressFilename =  "/MacAddress.txt";

QString systemInfo;
QString systemInfoFilename = "/SystemInfo.txt";

ControlPanel::ControlPanel(QWidget *parent) : QDialog(parent), ui(new Ui::ControlPanel)
{
    ui->setupUi(this);

    loadStates();
    restoreSettings();
    loadFPSensitivity();
    getMacAddress();
    getSystemInfo();
    loadAddressFormats();
    loadOperators();

    //connect(ui->importSelectFile, SIGNAL(clicked()), this, SLOT(selectFileDialog()));
    connect(ui->updateSettingsButton, SIGNAL(clicked()), this, SLOT(saveSensitivity()));

}

void ControlPanel::loadOperators() {

    ui->inpOperator->clear();

    QSqlQuery dbQuery;

    QString selectQuery = "SELECT id, first_name,last_name,access_code from registered_operators ORDER by first_name,last_name";



    if(dbQuery.exec(selectQuery)){

        int a =0;
        while(dbQuery.next()){

            ddcOperators[a] = dbQuery.value(0).toInt();

            QString operatorName = dbQuery.value(1).toString() + " " + dbQuery.value(2).toString() ;

            ui->inpOperator->addItem(operatorName);

            a++;

        }
    }
}

ControlPanel::~ControlPanel()
{
    delete ui;
}

void ControlPanel::changeEvent(QEvent *e)
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



bool ControlPanel::saveSensitivity()
{
    QSqlQuery dbQuery;
    QString queryString;

    int fpSettingIndex = ui->fpSensitivity->currentIndex();
    // Get ID of selected

    QModelIndex selectedSensitivity = fp->index(fpSettingIndex, 1);
    int sensitivityId = fp->data(selectedSensitivity, Qt::DisplayRole).toInt();


    queryString.sprintf("UPDATE settings SET setting_value='%d' WHERE setting_name='fingerprint_sensitivity_id'",sensitivityId);
    if(dbQuery.exec(queryString)){
        QMessageBox::information(this, "Control Panel","Changes to the Fingerprint Sensitivity have been saved",QMessageBox::Ok);
        return true;
    }else{
        QMessageBox::critical(this, "Query Error","Unable to update: \n"+dbQuery.lastError().text()+"\n"+dbQuery.lastQuery(),QMessageBox::Cancel);
      return false;
    }

}

void ControlPanel::restoreSensitivitySetting()
{

}


bool ControlPanel::saveAddressFormat()
{
    QSqlQuery dbQuery;
    QString queryString;


    int afSettingIndex = ui->addressFormat->currentIndex();
    // Get ID of selected

    QModelIndex selectedFormat = af->index(afSettingIndex, 1);
    int formatId = fp->data(selectedFormat, Qt::DisplayRole).toInt();


    queryString.sprintf("UPDATE settings SET setting_value='%d' WHERE setting_name='address_format_id'",formatId);
    if(dbQuery.exec(queryString)){
        QMessageBox::information(this, "Control Panel","Changes to the Address Format have been saved",QMessageBox::Ok);
        return true;
    }else{
        QMessageBox::critical(this, "Query Error","Unable to update: \n"+dbQuery.lastError().text()+"\n"+dbQuery.lastQuery(),QMessageBox::Cancel);
      return false;
    }

}



bool ControlPanel::saveSettings()
{

    if(selectedPuId==0){

        //QMessageBox::warning(this, "Control Panel Warning","No Changes Saved",QMessageBox::Ok);
        return false;
    }

    QSqlQuery dbQuery;
    QString queryString;
    queryString.sprintf("SELECT * FROM settings WHERE setting_name='polling_unit_id'");

    dbQuery.exec(queryString);
    if(dbQuery.size()){//found a default polling unit id

        queryString.sprintf("UPDATE settings SET setting_value='%d' WHERE setting_name='polling_unit_id'",selectedPuId);

    } else {//no default polling_id

        queryString.sprintf("INSERT INTO settings(setting_name,setting_value) VALUES('polling_unit_id', '%d')",selectedPuId);

    }

    if(dbQuery.exec(queryString)){
        QMessageBox::information(this, "Control Panel","Changes to the Registration Centre have been saved",QMessageBox::Ok);
        return true;
    }else{
        QMessageBox::critical(this, "Query Error","Unable to update: \n"+dbQuery.lastError().text()+"\n"+dbQuery.lastQuery(),QMessageBox::Cancel);
        return false;
    }


    /*
        QSqlTableModel settingsModel = new QSqlTableModel;

        settingsModel.setTable("settings");
        settingsModel.setFilter("setting_name='polling_unit_id'");

        QSqlRecord dbRecord;
        QSqlField settingValue("setting_value", QVariant::String);
        settingValue.setValue(selectedPuId);
        dbRecord.append(settingValue);
    */




}

void ControlPanel::loadStates()
{
    QSqlQuery dbQuery;
    dbQuery.exec("SELECT id,name,abbreviation FROM states");

    QString stateId;
    QString stateName;

    ui->inpState->addItem("");
    if(!dbQuery.size()){
        return;
    }
    while (dbQuery.next()) {
        stateId = dbQuery.value(0).toString();
//        stateName = dbQuery.value(1).toString();
        stateName = dbQuery.value(2).toString() + delimitationSplitter.toUtf8() + dbQuery.value(1).toString();;
        ui->inpState->addItem(stateName,stateId);
    }

}

void ControlPanel::loadLgas(QString stateName)
{
    selectedPuId=0;
    qDebug() << stateName;
    QSqlQuery dbQuery;

    QStringList stateNameList = stateName.split(delimitationSplitter,QString::KeepEmptyParts);
    stateName = stateNameList.at(0);

    QString queryString;

    ui->inpLga->clear();
    ui->inpRa->clear();
    ui->inpPu->clear();

    ui->inpLga->addItem("");

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
    selectedStateId = dbQuery.value(0).toInt();



    queryString.sprintf("SELECT id,name,abbreviation FROM local_governments WHERE  state_id=%d",selectedStateId);

    if(!dbQuery.exec(queryString)){
        QMessageBox::critical(this, "Query Error","Unable to select: \n"+dbQuery.lastError().text()+"\n"+dbQuery.lastQuery(),QMessageBox::Cancel);
    }


    if(!dbQuery.size()){
        return;
    }
    QString lgaName;
    while (dbQuery.next()) {
        lgaName = dbQuery.value(2).toString() + delimitationSplitter.toUtf8()+ dbQuery.value(1).toString();
        ui->inpLga->addItem(lgaName,dbQuery.value(0).toString());
    }

}
/*

void ControlPanel::closeEvent(QCloseEvent *event)
{
    if (saveSettings()) {
        //writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}
*/


void ControlPanel::on_inpState_currentIndexChanged(QString stateName)
{
    qDebug()<<"Previous State ID: "<<selectedStateId;
   loadLgas(stateName);
   qDebug()<<"Current State ID: "<<selectedStateId;

}

void ControlPanel::loadRas(QString currentLga)
{
    ui->inpRa->clear();
    ui->inpPu->clear();

    selectedPuId=0;

    if(currentLga.isEmpty()){
        return;
    }

    QStringList lgaNameList = currentLga.split(delimitationSplitter,QString::KeepEmptyParts);
    currentLga = lgaNameList.at(0);


    QSqlQuery dbQuery;

    QString queryString = "SELECT id FROM local_governments WHERE  abbreviation='"+currentLga+"' AND state_id="+QString::number(selectedStateId).toStdString().c_str();
    qDebug() <<queryString;

    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    selectedLgaId = dbQuery.value(0).toInt();


    queryString.sprintf("SELECT name,abbreviation FROM registration_areas WHERE  local_government_id='%d'",selectedLgaId);
    qDebug() <<queryString;
    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }

    ui->inpRa->addItem("");
    QString raName;
    while (dbQuery.next()) {

       raName =  dbQuery.value(1).toString() + delimitationSplitter.toUtf8()+ dbQuery.value(0).toString();
        ui->inpRa->addItem(raName);
    }


}

void ControlPanel::on_inpLga_currentIndexChanged(QString currentLga)
{
   loadRas(currentLga);
}


void ControlPanel::loadPollingUnits(QString repArea)
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

    selectedRaId =  dbQuery.value(0).toInt();

    //selectedRaId

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

}

void ControlPanel::on_inpRa_currentIndexChanged(QString repArea )
{

    loadPollingUnits(repArea);


}

void ControlPanel::on_buttonBox_accepted()
{
    qDebug() <<"Requesting Save";


    saveSettings();




}

void ControlPanel::on_buttonBox_rejected()
{
    qDebug() <<"Requesting Cancel";
}

void ControlPanel::on_inpPu_currentIndexChanged(QString currentRa )
{
    setSelectedRa(currentRa);
}


void ControlPanel::setSelectedRa(QString currentRa)
{
    QSqlQuery dbQuery;


    QStringList raNameList = currentRa.split(delimitationSplitter,QString::KeepEmptyParts);
    currentRa = raNameList.at(0);

    QString queryString = "SELECT id FROM polling_units WHERE  abbreviation='"+currentRa+"' AND registration_area_id="+QString::number(selectedRaId).toStdString().c_str();

    qDebug() <<queryString;

    dbQuery.exec(queryString);
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    selectedPuId =  dbQuery.value(0).toInt();

}

void ControlPanel::restoreSettings(){

    QSqlQuery dbQuery;
    QString queryString = "SELECT polling_units.id pu_id,polling_units.name pu_name, polling_units.abbreviation pu_abbrev, registration_areas.id ra_id,registration_areas.name ra_name,registration_areas.abbreviation ra_abbrev, local_governments.id lga_id, local_governments.name lga_name,local_governments.abbreviation lga_abbrev, states.id s_id,states.name s_name, states.abbreviation state_abbrev "
                          " FROM settings, states, local_governments, registration_areas, polling_units "
                          " WHERE setting_name =  'polling_unit_id' "
                          " AND polling_units.id = setting_value "
                          " AND registration_areas.id = polling_units.registration_area_id "
                          " AND registration_areas.local_government_id = local_governments.id "
                          " AND local_governments.state_id = states.id";

    qDebug() <<queryString;
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
    int pollingUnitNameCol = settingRec.indexOf("pu_name");
    int raNameCol = settingRec.indexOf("ra_name");
    int lgaNameCol  = settingRec.indexOf("lga_name");
    int stateNameCol  = settingRec.indexOf("s_name");


    int pollingUnitAbbrevCol = settingRec.indexOf("pu_abbrev");
    int raAbbrevCol = settingRec.indexOf("ra_abbrev");
    int lgaAbbrevCol  = settingRec.indexOf("lga_abbrev");
    int stateAbbrevCol  = settingRec.indexOf("state_abbrev");

    ui->lblCurrentState->setText(dbQuery.value(stateAbbrevCol).toString()+ delimitationSplitter.toUtf8()+dbQuery.value(stateNameCol).toString());
    ui->lblCurrentLga->setText(dbQuery.value(lgaAbbrevCol).toString()+ delimitationSplitter.toUtf8()+dbQuery.value(lgaNameCol).toString());
    ui->lblCurrentRa->setText(dbQuery.value(raAbbrevCol).toString()+ delimitationSplitter.toUtf8()+dbQuery.value(raNameCol).toString());
    ui->lblCurrentPu->setText(dbQuery.value(pollingUnitAbbrevCol).toString()+ delimitationSplitter.toUtf8()+dbQuery.value(pollingUnitNameCol).toString());
}


void ControlPanel::loadFPSensitivity()
{
    qDebug() << "Loading FP Sensitivity";
    QString sql = "SELECT name, id FROM fingerprint_sensitivity";

    fp = new QSqlQueryModel;
    fp->setQuery(sql);

    ui->fpSensitivity->setModel(fp);




//    QSqlQuery dbQuery;

//    QString queryString = "SELECT setting_value FROM settings WHERE setting_name='fingerprint_sensitivity_id'";

//    qDebug() << "Getting FP Sensitivity Setting: " << queryString;
//    dbQuery.exec(queryString);

//    QSqlRecord settingRec = dbQuery.record();
//    if(!dbQuery.size()){
//        return;
//    }
//    dbQuery.next();

//    int curSensitivity = settingRec.indexOf("setting_value");
//     @todo Make Current Fingerprint Setting The Default Option


}



void ControlPanel::getMacAddress()
{

//    QFile file(macAddressFilename);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//        return;

//    QTextStream in(&file);
//    QString line = in.readLine();
//    while (!line.isNull()) {
//        macAddress += line;
//        line = in.readLine();
//    }

//    ui->macAddress->setText(macAddress);
}


void ControlPanel::getSystemInfo()
{
//    qDebug() << "Retrieving System Info";
//    QFile file(systemInfoFilename);
//    file.open(QFile::ReadOnly);

//    QByteArray assetContents = file.readAll();
//    QTextStream in(&file);
//    in >> systemInfo;

//    qDebug() << systemInfo;



    QFile file(systemInfoFilename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    QString line = in.readLine();
    while (!line.isNull()) {
        systemInfo += line;
        line = in.readLine();
    }

    ui->systemInfo->setText(systemInfo);
}

void ControlPanel::selectFileDialog()
{
//    QFileDialog dialog(this);
//     dialog.setFileMode(QFileDialog::AnyFile);
//    dialog.setNameFilter(tr("INEC Backups (*.inec *.tar.gz *.gz)"));
//    dialog.setViewMode(QFileDialog::Detail);
//    QStringList fileNames;
//     if (dialog.exec())
//         fileNames = dialog.selectedFiles();
//    dialog.exec();
}


void ControlPanel::loadAddressFormats() {

    qDebug() << "Loading Address Formats";
    QString sql = "SELECT name, id FROM address_formats";

    af = new QSqlQueryModel;
    af->setQuery(sql);

    ui->addressFormat->setModel(af);


    QSqlQuery dbQuery;

    QString queryString = "SELECT setting_value FROM settings WHERE setting_name='address_format_id'";

    qDebug() << "Getting Address Format Setting: " << queryString;
    dbQuery.exec(queryString);

    QSqlRecord settingRec = dbQuery.record();
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    int curSensitivity = settingRec.indexOf("setting_value");

    // @todo Make Current Address Format The Selected Option In Control Panel
}

void ControlPanel::on_btnRegister_clicked()
{
    QString errorMessages;

    if(ui->inpAccessCode->text()==""){

        errorMessages+="\nMissing Access Code";

    }else if(ui->inpAccessCode->text()!=ui->inpAccessCodeConfirm->text()){
        errorMessages+="\nThe same information in the Access Code/Confirm Code fields ";
    }


    if(ui->inpLastName->text() == ""){

        errorMessages+="\nLast Name";
    }

    if(ui->inpFirstName->text() == "" ){

        errorMessages+="\nFirst Name";

    }


    if(errorMessages.isNull()) {

        qDebug() << "Form Validated";


        QSqlTableModel *operatorModel = new QSqlTableModel;
        operatorModel->setTable("registered_operators");

        QSqlField firstName("first_name", QVariant::String);
        firstName.setValue(ui->inpFirstName->text());
        operatorRecord.append(firstName);

        QSqlField lastName("last_name", QVariant::String);
        lastName.setValue(ui->inpLastName->text());
        operatorRecord.append(lastName);

        QSqlField regCode("access_code", QVariant::String);
        regCode.setValue(ui->inpAccessCode->text());
        operatorRecord.append(regCode);

        if(operatorModel->insertRecord(-1, operatorRecord)) {

            QMessageBox::information(0, "Success","The registration information has been saved", QMessageBox::Ok);
            loadOperators();
            //this->close();

        }else{
            QMessageBox::critical(0, "Query Error","Unable to insert: \n" +operatorModel->lastError().text().toUtf8(),QMessageBox::Cancel);
        }


        qDebug() <<  "Here";


    }else{
        qDebug() << "Form Errors: " << errorMessages;
        QMessageBox::critical(0, "Error Saving Information","Please check that you have provided the following\n "+errorMessages, QMessageBox::Ok);
    }




}

void ControlPanel::on_selectOperator_clicked()
{
    int selectedOperatorIndex = ui->inpOperator->currentIndex();
    selectedOperator = ddcOperators[selectedOperatorIndex];
    qDebug() << "Selected Operator ID "<< selectedOperator;

    QSqlQuery dbQuery;

    QString selectQuery;
    selectQuery.sprintf( "SELECT id, first_name,last_name,access_code,active from registered_operators WHERE id =%d",selectedOperator);
    qDebug() << selectQuery;
    if(dbQuery.exec(selectQuery)){
        if(dbQuery.next()){
            ui->inpUFirstName->setText(dbQuery.value(1).toString());
            ui->inpULastName->setText(dbQuery.value(2).toString());
            ui->inpCurAccessCode->setText(dbQuery.value(3).toString());
            ui->inpAccountStatus->setCurrentIndex(dbQuery.value(4).toInt());
            ui->btnUpdateDetails->setEnabled(true);
        }
    }

}

void ControlPanel::on_inpOperator_currentIndexChanged(int index)
{
    ui->btnUpdateDetails->setEnabled(false);
    qDebug() << index;

}

void ControlPanel::on_btnUpdateDetails_clicked()
{
    int selectedOperatorIndex = ui->inpOperator->currentIndex();
    selectedOperator = ddcOperators[selectedOperatorIndex];

    QString errorMessages;



    if(ui->inpULastName->text() == ""){

        errorMessages+="\nLast Name";
    }

    if(ui->inpUFirstName->text() == "" ){

        errorMessages+="\nFirst Name";

    }


    if(errorMessages.isNull()) {

        qDebug() << "Form Validated";

        QSqlQuery dbQuery;

        QString updateQuery;

        //The next query is not nice
        updateQuery= "UPDATE registered_operators SET "
                     "first_name='"+ui->inpUFirstName->text()+"', "
                     "last_name='"+ui->inpULastName->text()+"', "
                     "active="+QString::number(ui->inpAccountStatus->currentIndex()).toStdString().c_str() +
                     " WHERE id="+QString::number(selectedOperator).toStdString().c_str();

        qDebug() << "SQL QUERY " << updateQuery;

        if(dbQuery.exec(updateQuery)) {

            QMessageBox::information(0, "Success","The registration information has been updated", QMessageBox::Ok);

        } else {

            QMessageBox::critical(0, "Query Error","Unable to update: \n" +dbQuery.lastError().text(),QMessageBox::Cancel);

        }

    }else{
        qDebug() << "Form Errors: " << errorMessages;
        QMessageBox::critical(0, "Error Saving Information","Please check that you have provided the following\n "+errorMessages, QMessageBox::Ok);
    }



}
