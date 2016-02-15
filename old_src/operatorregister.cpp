#include "operatorregister.h"
#include "ui_operatorregister.h"
#include "QMessageBox"
#include "QDebug"
#include <QtSql>
#include <QSqlField>
#include <QSqlRecord>

QSqlRelationalTableModel *operatorModel;
QSqlRecord operatorRecord;
int ddcOperators[100];
int selectedOperator;

OperatorRegister::OperatorRegister(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::OperatorRegister)
{
    ui->setupUi(this);

    QSqlQuery dbQuery;

    QString selectQuery = "SELECT id, first_name,last_name,access_code from registered_operators ORDER by first_name,last_name";

    if(dbQuery.exec(selectQuery)){

        int a =0;
        while(dbQuery.next()){

            ddcOperators[a] = dbQuery.value(0).toInt();

            QString operatorName = dbQuery.value(1).toString() + " " + dbQuery.value(2).toString() + " ["+ dbQuery.value(3).toString() +"]";

            ui->inpOperator->addItem(operatorName);

            a++;

        }



    }


}

OperatorRegister::~OperatorRegister()
{
    delete ui;
}

void OperatorRegister::changeEvent(QEvent *e)
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





void OperatorRegister::on_btnRegister_clicked()
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

            this->close();

        }else{
            QMessageBox::critical(0, "Query Error","Unable to insert: \n" +operatorModel->lastError().text().toUtf8(),QMessageBox::Cancel);
        }


        qDebug() <<  "Here";


    }else{
        qDebug() << "Form Errors: " << errorMessages;
        QMessageBox::critical(0, "Error Saving Information","Please check that you have provided the following\n "+errorMessages, QMessageBox::Ok);
    }




}

void OperatorRegister::on_selectOperator_clicked()
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

void OperatorRegister::on_inpOperator_currentIndexChanged(int index)
{
    ui->btnUpdateDetails->setEnabled(false);
    qDebug() << index;

}

void OperatorRegister::on_btnUpdateDetails_clicked()
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
