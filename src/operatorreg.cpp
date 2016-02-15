#include "operatorreg.h"
#include "ui_operatorreg.h"
#include "QSqlTableModel"
#include "QMessageBox"
#include "QDebug"
#include "QSqlField"
#include "QSqlRecord"
#include "QSqlError"
#include "QSqlRelationalTableModel"
#include "QProcess"
#include "QDateTime"

OperatorReg::OperatorReg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OperatorReg)
{
    ui->setupUi(this);
}


OperatorReg::~OperatorReg()
{
    delete ui;
}


void OperatorReg::on_btnRegister_clicked()
{
    QString errorMessages;

    if(ui->inpAccessCode->text()==""){

        errorMessages+="\nMissing Access Code";

    }else if(ui->inpAccessCode->text()!=ui->inpAccessCodeConfirm->text()){
        errorMessages+="\nThe same information in the Access Code/Confirm Code fields ";
    }


    if(ui->inpOperatorID->text() == ""){

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

        // Check if PAC and OID match
        QSqlTableModel *opCodeModel = new QSqlTableModel;
        opCodeModel->setTable("operator_codes");
//        QString opCodeFilter = "pac='" + ui->inpAccessCode->text().toUtf8() + "' AND id='" + ui->inpOperatorID->text().toUtf8() + "' AND is_used='0'";
        QString opCodeFilter = "pac='" + ui->inpAccessCode->text().toUtf8() + "' AND id='" + ui->inpOperatorID->text().toUtf8() + "'";
        qDebug() << "Op Code Filter: " << opCodeFilter;
        opCodeModel->setFilter(opCodeFilter);
        opCodeModel->select();
        int opCodeRows = opCodeModel->rowCount();
        qDebug() << "Op Code Rows: " << QString::number(opCodeRows);

        if (opCodeRows) {

            if (opCodeModel->record(0).value("is_used").toBool())
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
                    //Seems there are haier machines that the getMac function does not work for
                    QMessageBox::critical(this, "System Error.","Could Not Access System Information. Please check and try again", QMessageBox::Ok);
                //    this->close();
                //    return;
                    QDateTime currentTime = QDateTime::currentDateTime();
                    macAddr.sprintf("MC_%d",currentTime.toTime_t());
                }

                QString opPasswd = opCodeModel->record(0).value("passwd").toString();

                QSqlTableModel *operatorModel = new QSqlTableModel;
                operatorModel->setTable("registered_operators");

                QSqlField opid("id", QVariant::String);
                opid.setValue(macAddr + "_" + ui->inpOperatorID->text());
                operatorRecord.append(opid);

                QSqlField oid("oid", QVariant::String);
                oid.setValue(ui->inpOperatorID->text());
                operatorRecord.append(oid);

                QSqlField passwd("passwd", QVariant::String);
                passwd.setValue(opPasswd);
                operatorRecord.append(passwd);

                QSqlField firstName("first_name", QVariant::String);
                firstName.setValue(ui->inpFirstName->text());
                operatorRecord.append(firstName);

                QSqlField lastName("last_name", QVariant::String);
                lastName.setValue(ui->inpLastName->text());
                operatorRecord.append(lastName);

                QSqlField regCode("access_code", QVariant::String);
                regCode.setValue(ui->inpAccessCode->text());
                operatorRecord.append(regCode);

                QSqlField active("active", QVariant::Int);
                active.setValue(1);
                operatorRecord.append(active);

                if(operatorModel->insertRecord(-1, operatorRecord)) {

                    // Set Operator Code to Used
                    QSqlRecord opCode = opCodeModel->record(0);
                    opCode.setValue("is_used", "1");
                    opCodeModel->setRecord(0, opCode);
                    opCodeModel->submitAll();

                    QMessageBox::information(0, "Success","The Operator Has Been Successfully Registered. \n\n OPERATOR PASSWORD: " + opPasswd + "\n\n IMPORTANT NOTICE: \n Memorize password immediately or store in a safe place. You MUST change your password to something easier to remember from the Dashboard Menu.", QMessageBox::Ok);
                    QMessageBox::information(0, "Success","The Operator Has Been Successfully Registered. \n\n OPERATOR PASSWORD: " + opPasswd + "\n\n IMPORTANT NOTICE: \n Memorize password immediately or store in a safe place. You MUST change your password to something easier to remember from the Dashboard Menu.", QMessageBox::Ok);

                    this->close();

                }else{
                    QMessageBox::critical(0, "Query Error","Unable to insert: \n" +operatorModel->lastError().text().toUtf8(),QMessageBox::Cancel);
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



void OperatorReg::setRegExp(QString regExp) {
    QValidator *textVal = new QRegExpValidator(QRegExp(regExp), this);

    ui->inpAccessCode->setValidator(textVal);
    ui->inpAccessCodeConfirm->setValidator(textVal);
    ui->inpFirstName->setValidator(textVal);
    ui->inpLastName->setValidator(textVal);
    ui->inpOperatorID->setValidator(textVal);

}
