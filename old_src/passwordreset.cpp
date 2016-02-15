#include "passwordreset.h"
#include "ui_passwordreset.h"
#include "QSqlTableModel"
#include "QSqlRecord"
#include "QMessageBox"
#include "QDebug"

PasswordReset::PasswordReset(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordReset)
{
    ui->setupUi(this);
    connect( ui->btnUpdate, SIGNAL(clicked()), this, SLOT(resetPassword()) );
}

PasswordReset::~PasswordReset()
{
    delete ui;
}


void PasswordReset::resetPassword()
{
    qDebug() << "Operator: " << operatorId;

    // Check if new passwords match
    if (ui->inpNewPasswd->text() != ui->inpConfirmNewPasswd->text() ) {
        QMessageBox::critical(0, "Password Error","New Passwords Do Not Match. Please Try Again.",QMessageBox::Cancel);
        return;
    }

    QString newPasswd = ui->inpNewPasswd->text();

    // Check if operator id and password exist
    QSqlTableModel *passwdModel = new QSqlTableModel;
    passwdModel->setTable("registered_operators");
    QString passwdFilter = "`oid`='" + operatorId + "'";
    qDebug() << "Login Filter: " << passwdFilter;
    passwdModel->setFilter(passwdFilter);
    passwdModel->select();

    int pwdRows = passwdModel->rowCount();
    qDebug() << "Rows Found: " << QString::number(pwdRows);

    // Update Password
    if (pwdRows) {
            // Set Operator Code to Used
            QSqlRecord opRec = passwdModel->record(0);
            opRec.setValue("passwd", newPasswd);
            passwdModel->setRecord(0, opRec);
            passwdModel->submitAll();

            QMessageBox::information(this, "Password Successfully Updated","Your Password Has Been Updated Successfully! ", QMessageBox::Ok);
            this->close();
    } else {
        QMessageBox::critical(0, "Error","Invalid Operator.",QMessageBox::Cancel);
    }

}


void PasswordReset::setRegExp(QString regExp) {
    QValidator *textVal = new QRegExpValidator(QRegExp(regExp), this);

    ui->inpConfirmNewPasswd->setValidator(textVal);
    ui->inpNewPasswd->setValidator(textVal);
}

