#include "adminreg.h"
#include "ui_adminreg.h"
#include "QSqlTableModel"
#include "QMessageBox"
#include "QDebug"
#include "QSqlField"
#include "QSqlRecord"
#include "QSqlError"
#include "QSqlRelationalTableModel"
#include "QProcess"
#include "QDateTime"

AdminReg::AdminReg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminReg)
{
    ui->setupUi(this);
}

AdminReg::~AdminReg()
{
    delete ui;
}


void AdminReg::on_btnRegister_clicked()
{
    QString errorMessages;

    if(ui->inpAccessCode->text()==""){

        errorMessages+="\nMissing Access Code";

    }else if(ui->inpAccessCode->text()!=ui->inpAccessCodeConfirm->text()){
        errorMessages+="\nThe same information in the Access Code/Confirm Code fields ";
    }


    if(ui->inpAdminID->text() == ""){

        errorMessages+="\nOperator ID";
    }

    if(ui->inpLastName->text() == ""){

        errorMessages+="\nLast Name";
    }

    if(ui->inpFirstName->text() == "" ){

        errorMessages+="\nFirst Name";

    }



    if(errorMessages.isNull()) {
        qDebug() << "Form Validated";

        // Check if PAC and RID match
        QSqlTableModel *adCodeModel = new QSqlTableModel;
        adCodeModel->setTable("administrator_codes");
//        QString opCodeFilter = "pac='" + ui->inpAccessCode->text().toUtf8() + "' AND id='" + ui->inpAdminID->text().toUtf8() + "' AND is_used='0'";
        QString opCodeFilter = "pac='" + ui->inpAccessCode->text().toUtf8() + "' AND id='" + ui->inpAdminID->text().toUtf8() + "'";
        qDebug() << "Op Code Filter: " << opCodeFilter;
        adCodeModel->setFilter(opCodeFilter);
        adCodeModel->select();
        int adCodeRows = adCodeModel->rowCount();
        qDebug() << "Op Code Rows: " << QString::number(adCodeRows);
        qDebug() << "Code Is Used: " << adCodeModel->record(0).value("is_used").toString();

        if (adCodeRows) {

            if (adCodeModel->record(0).value("is_used").toBool())
            {
                QMessageBox::critical(0, "Invalid Access Code","Access Code Has Already Been Used. Please Contact The Administrator." ,QMessageBox::Cancel);
            } else {

                // Get Mac
                QString macAddr="";
                QProcess *scriptProcess = new QProcess();
                QStringList args;
                args << "JwgVN5aHth8J7";
                scriptProcess->start("./scripts/getMac.sh", args);

                if (!scriptProcess->waitForFinished()){
                    qDebug() << "Error :" << scriptProcess->errorString();
                } else {
                    macAddr =  scriptProcess->readAll();
                }

                qDebug() << "System Mac: " << macAddr;

                if (macAddr == "") {
                    QMessageBox::critical(this, "System Error.","Could Not Access System Information. Please check and try again", QMessageBox::Ok);
                    //Seems there are haier machines that the getMac function does not work for
                    //this->close();
                    //return;
                    QDateTime currentTime = QDateTime::currentDateTime();
                    macAddr.sprintf("MC_%d",currentTime.toTime_t());


                }

                QString opPasswd = adCodeModel->record(0).value("passwd").toString();

                QSqlTableModel *adminModel = new QSqlTableModel;
                adminModel->setTable("administrators");

                QSqlField adminid("id", QVariant::String);
                adminid.setValue(macAddr + "_" + ui->inpAdminID->text());
                adminRecord.append(adminid);

                QSqlField rid("rid", QVariant::String);
                rid.setValue(ui->inpAdminID->text());
                adminRecord.append(rid);

                QSqlField passwd("passwd", QVariant::String);
                passwd.setValue(opPasswd);
                adminRecord.append(passwd);

                QSqlField active("active", QVariant::Int);
                active.setValue(1);
                adminRecord.append(active);

                QSqlField firstName("first_name", QVariant::String);
                firstName.setValue(ui->inpFirstName->text());
                adminRecord.append(firstName);

                QSqlField lastName("last_name", QVariant::String);
                lastName.setValue(ui->inpLastName->text());
                adminRecord.append(lastName);

                QSqlField regCode("access_code", QVariant::String);
                regCode.setValue(ui->inpAccessCode->text());
                adminRecord.append(regCode);

                if(adminModel->insertRecord(-1, adminRecord)) {

                    // Set Operator Code to Used
                    QSqlRecord opCode = adCodeModel->record(0);
                    opCode.setValue("is_used", "1");
                    adCodeModel->setRecord(0, opCode);
                    adCodeModel->submitAll();

                    QMessageBox::information(0, "Success","Your Administator Account Has Been Succcessfully Created. \n\n ADMINISTRATOR PASSWORD: " + opPasswd + "\n\n IMPORTANT NOTICE: \n Memorize password immediately or store in a safe place. You can also change your password from the Dashboard Menu.", QMessageBox::Ok);

                    this->close();

                } else {
                    QMessageBox::critical(0, "Query Error","Unable to insert: \n" +adminModel->lastError().text().toUtf8(),QMessageBox::Cancel);
                    this->close();
                }
            }
        } else {
            QMessageBox::critical(0, "Invalid Access Code","Invalid Access Code. Please Enter Operator Login Correctly And Try Again." ,QMessageBox::Cancel);
            this->close();
        }


//        qDebug() <<  "Here";


    }else{
        qDebug() << "Form Errors: " << errorMessages;
        QMessageBox::critical(0, "Error Saving Information","Please check that you have provided the following\n "+errorMessages, QMessageBox::Ok);
    }

//    clearOpForm();


}


void AdminReg::setRegExp(QString regExp) {
    QValidator *textVal = new QRegExpValidator(QRegExp(regExp), this);

    ui->inpAccessCode->setValidator(textVal);
    ui->inpAccessCodeConfirm->setValidator(textVal);
    ui->inpAdminID->setValidator(textVal);
    ui->inpFirstName->setValidator(textVal);
    ui->inpLastName->setValidator(textVal);
}
