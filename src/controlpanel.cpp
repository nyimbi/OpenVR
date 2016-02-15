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
#include "QProcess"
#include "passwordreset.h"
#include "update.h"
#include "repairdatabase.h"

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

QSqlTableModel *opsModel;

ControlPanel::ControlPanel(QWidget *parent) : QDialog(parent), ui(new Ui::ControlPanel)
{
    ui->setupUi(this);

    loadStates();
    restoreSettings();
    loadFPSensitivity();
    getMacAddress();
    getSystemInfo();
    loadAddressFormats();
//    loadOperators();

    //connect(ui->importSelectFile, SIGNAL(clicked()), this, SLOT(selectFileDialog()));
    connect(ui->updateSettingsButton, SIGNAL(clicked()), this, SLOT(saveSensitivity()));

    opsModel = new QSqlTableModel;
    opsModel->setTable("registered_operators");
    opsModel->select();
    ui->operatorsView->setModel(opsModel);

    opsGridPresentation();
}


void ControlPanel::opsGridPresentation() {

    ui->lblOperatorsCount->setText( QString::number(opsModel->rowCount()) + " Operator(s) Registered");

    opsModel->removeColumn(opsModel->fieldIndex("id"));
    opsModel->removeColumn(opsModel->fieldIndex("access_code"));
    opsModel->removeColumn(opsModel->fieldIndex("passwd"));
    opsModel->removeColumn(opsModel->fieldIndex("deleted"));
//    opsModel->removeColumn(opsModel->fieldIndex("created"));
    opsModel->removeColumn(opsModel->fieldIndex("date_deleted"));


//    int i=0;
    QStringList fields;
    fields << "Operator ID";
    fields << "First Name";
    fields << "Last Name";
    fields << "Is Active";
    fields << "Created";


    for(int i=0; i<fields.size(); i++) {
        opsModel->setHeaderData(i, Qt::Horizontal, QObject::tr(fields[i].toAscii().constData() ) );
    }

    ui->operatorsView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->operatorsView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->operatorsView->setSortingEnabled(true);
}

void ControlPanel::loadOperators() {
    opsModel->select();
    ui->lblOperatorsCount->setText( QString::number(opsModel->rowCount()) + " Operator(s) Registered");
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
//    QSqlQuery dbQuery;
//    QString queryString;


//    int afSettingIndex = ui->addressFormat->currentIndex();
//    // Get ID of selected

//    QModelIndex selectedFormat = af->index(afSettingIndex, 1);
//    int formatId = fp->data(selectedFormat, Qt::DisplayRole).toInt();


//    queryString.sprintf("UPDATE settings SET setting_value='%d' WHERE setting_name='address_format_id'",formatId);
//    if(dbQuery.exec(queryString)){
//        QMessageBox::information(this, "Control Panel","Changes to the Address Format have been saved",QMessageBox::Ok);
//        return true;
//    }else{
//        QMessageBox::critical(this, "Query Error","Unable to update: \n"+dbQuery.lastError().text()+"\n"+dbQuery.lastQuery(),QMessageBox::Cancel);
//      return false;
//    }

    return false;
}



bool ControlPanel::saveSettings()
{

    if(selectedPuId==0) {

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

    if(!dbQuery.size()) {
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


    //Make Current Fingerprint Setting The Default Option


    QSqlQuery dbQuery;

    QString queryString = "SELECT setting_value,name FROM settings,fingerprint_sensitivity WHERE setting_name='fingerprint_sensitivity_id' AND settings.setting_value=fingerprint_sensitivity.id";

//    qDebug() << "Getting FP Sensitivity Setting: " << queryString;
    dbQuery.exec(queryString);

    QSqlRecord settingRec = dbQuery.record();
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();

    int curSensitivityIndex = settingRec.indexOf("name");

    QString curSensitivityName = dbQuery.value(curSensitivityIndex).toString();

    qDebug()<< "Sensitivity: " << curSensitivityName;
    int matchingIndex = ui->fpSensitivity->findText(curSensitivityName);

    qDebug()<< "Matching Index: " << matchingIndex;

    ui->fpSensitivity->setCurrentIndex(matchingIndex);

    //ui->fpSensitivity->



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


// Latest
//    QFile file(systemInfoFilename);
//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//        return;

//    QTextStream in(&file);
//    QString line = in.readLine();
//    while (!line.isNull()) {
//        systemInfo += line;
//        line = in.readLine();
//    }

//    ui->systemInfo->setText(systemInfo);
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

//    qDebug() << "Loading Address Formats";
//    QString sql = "SELECT name, id FROM address_formats";

//    af = new QSqlQueryModel;
//    af->setQuery(sql);

//    ui->addressFormat->setModel(af);


//    QSqlQuery dbQuery;

//    QString queryString = "SELECT setting_value FROM settings WHERE setting_name='address_format_id'";

//    qDebug() << "Getting Address Format Setting: " << queryString;
//    dbQuery.exec(queryString);

//    QSqlRecord settingRec = dbQuery.record();
//    if(!dbQuery.size()){
//        return;
//    }
//    dbQuery.next();

//    int curSensitivity = settingRec.indexOf("setting_value");

    // @todo Make Current Address Format The Selected Option In Control Panel
}


void ControlPanel::clearOpForm() {

}

//void ControlPanel::on_selectOperator_clicked()
//{
//    int selectedOperatorIndex = ui->inpOperator->currentIndex();
//    selectedOperator = ddcOperators[selectedOperatorIndex];
//    qDebug() << "Selected Operator ID "<< selectedOperator;

//    QSqlQuery dbQuery;

//    QString selectQuery;
//    selectQuery.sprintf( "SELECT id, first_name,last_name,access_code,active from registered_operators WHERE id =%d",selectedOperator);
//    qDebug() << selectQuery;
//    if(dbQuery.exec(selectQuery)){
//        if(dbQuery.next()){
//            ui->inpUFirstName->setText(dbQuery.value(1).toString());
//            ui->inpULastName->setText(dbQuery.value(2).toString());
//            ui->inpCurAccessCode->setText(dbQuery.value(3).toString());
//            ui->inpAccountStatus->setCurrentIndex(dbQuery.value(4).toInt());
//            ui->btnUpdateDetails->setEnabled(true);
//        }
//    }

//}

//void ControlPanel::on_inpOperator_currentIndexChanged(int index)
//{
//    ui->btnUpdateDetails->setEnabled(false);
//    qDebug() << index;

//}

//void ControlPanel::on_btnUpdateDetails_clicked()
//{
//    int selectedOperatorIndex = ui->inpOperator->currentIndex();
//    selectedOperator = ddcOperators[selectedOperatorIndex];

//    QString errorMessages;



//    if(ui->inpULastName->text() == ""){

//        errorMessages+="\nLast Name";
//    }

//    if(ui->inpUFirstName->text() == "" ){

//        errorMessages+="\nFirst Name";

//    }


//    if(errorMessages.isNull()) {

//        qDebug() << "Form Validated";

//        QSqlQuery dbQuery;

//        QString updateQuery;

//        //The next query is not nice
//        updateQuery= "UPDATE registered_operators SET "
//                     "first_name='"+ui->inpUFirstName->text()+"', "
//                     "last_name='"+ui->inpULastName->text()+"', "
//                     "active="+QString::number(ui->inpAccountStatus->currentIndex()).toStdString().c_str() +
//                     " WHERE id="+QString::number(selectedOperator).toStdString().c_str();

//        qDebug() << "SQL QUERY " << updateQuery;

//        if(dbQuery.exec(updateQuery)) {

//            QMessageBox::information(0, "Success","The registration information has been updated", QMessageBox::Ok);

//        } else {

//            QMessageBox::critical(0, "Query Error","Unable to update: \n" +dbQuery.lastError().text(),QMessageBox::Cancel);

//        }

//    } else {
//        qDebug() << "Form Errors: " << errorMessages;
//        QMessageBox::critical(0, "Error Saving Information","Please check that you have provided the following\n "+errorMessages, QMessageBox::Ok);
//    }
//}

void ControlPanel::updateOperators(int status) {
    qDebug() << "Updating Operator Status:";
    QModelIndexList selectedRecords = ui->operatorsView->selectionModel()->selectedRows();

    QStringList selectedIds;
    QString updateFilter = "";

    for(int i=0; i<selectedRecords.size(); i++) {
        updateFilter += "`oid`='" + opsModel->data(opsModel->index(selectedRecords[i].row(), opsModel->fieldIndex("oid")),0).toString() + "'";
        if (i != selectedRecords.size()-1) {
            updateFilter += " OR ";
        }
        selectedIds << opsModel->data(opsModel->index(selectedRecords[i].row(), opsModel->fieldIndex("oid")),0).toString();
        qDebug() << "Selected ID:" <<  opsModel->data(opsModel->index(selectedRecords[i].row(), opsModel->fieldIndex("oid")),0).toString();
    }



    if (selectedIds.size() > 0) {
        qDebug() << "Update Filter: " << updateFilter;


        QSqlTableModel *updateModel = new QSqlTableModel();
        updateModel->setTable("registered_operators");
        updateModel->setFilter(updateFilter);
        updateModel->select();

        for (int i = 0; i < updateModel->rowCount(); ++i) {
            QSqlRecord opRec = updateModel->record(i);
            opRec.setValue("active", QString::number(status) );
            updateModel->setRecord(i, opRec);
        }

        updateModel->submitAll();
        loadOperators();
    } else {
        QMessageBox::critical(this, "Error :: No Record Selected", "Please select a record and try again.", QMessageBox::Ok);
    }
}

void ControlPanel::on_btnEnableOperator_clicked()
{
    updateOperators(1);
}

void ControlPanel::on_btnDisableOperator_clicked()
{
    updateOperators(0);
}

void ControlPanel::on_btnResetPassword_clicked()
{
    qDebug() << "Updating Operator Status:";
    QModelIndexList selectedRecords = ui->operatorsView->selectionModel()->selectedRows();

    QStringList selectedIds;
    QString updateFilter = "";

    for(int i=0; i<selectedRecords.size(); i++) {
        updateFilter += "`oid`='" + opsModel->data(opsModel->index(selectedRecords[i].row(), opsModel->fieldIndex("oid")),0).toString() + "'";
        if (i != selectedRecords.size()-1) {
            updateFilter += " OR ";
        }
        selectedIds << opsModel->data(opsModel->index(selectedRecords[i].row(), opsModel->fieldIndex("oid")),0).toString();
        qDebug() << "Selected ID:" <<  opsModel->data(opsModel->index(selectedRecords[i].row(), opsModel->fieldIndex("oid")),0).toString();
    }

    if (selectedRecords.size() == 1) {
        PasswordReset *pr = new PasswordReset(this);
        pr->operatorId = selectedIds[0];
        pr->exec();
    }
}

void ControlPanel::on_pushButton_clicked()
{
    QString processOutput="";
    QProcess *scriptProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    scriptProcess->start("./scripts/check_removable.sh", args);

    if (!scriptProcess->waitForFinished()){
//        qDebug() << "Error :" << scriptProcess->errorString();
    } else {
        processOutput =  scriptProcess->readAll();
    }

    if (processOutput == "") {
        QMessageBox::critical(this, "Device Error","Backup Storage Not Detected. Please check and try again", QMessageBox::Ok);
        return;
    }

    Update *update = new Update(this);
    update->exec();
}

void ControlPanel::on_btnRepairDatabase_clicked()
{

    RepairDatabase *repair = new RepairDatabase(this);
    repair->exec();

//    // Check if backup is connected
//    QString processOutput="";
//    QProcess *scriptProcess = new QProcess();
//    QStringList args;
//    args << "JwgVN5aHth8J7";
//    scriptProcess->start("./scripts/check_removable.sh", args);

//    if (!scriptProcess->waitForFinished()){
////        qDebug() << "Error :" << scriptProcess->errorString();
//    } else {
//        processOutput =  scriptProcess->readAll();
//    }

//    if (processOutput == "") {
//        QMessageBox::critical(this, "Device Error","Backup Storage Not Detected. Please check and try again", QMessageBox::Ok);
//        return;
//    }

//    // Backup database


//    // Repair Database
//    processOutput="";
//    QProcess *repairProcess = new QProcess();
//    repairProcess->start("./scripts/repair_database.sh", args);

//    if (!repairProcess->waitForFinished()){
//        processOutput =  repairProcess->readAll();
//    }

//    if (processOutput == "") {
//        QMessageBox::critical(this, "Error","Nothing returned", QMessageBox::Ok);
//        return;
//    }


}
