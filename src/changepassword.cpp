#include "changepassword.h"
#include "ui_changepassword.h"
#include "QSqlTableModel"
#include "QSqlRecord"
#include "QMessageBox"
#include "QDebug"


ChangePassword::ChangePassword(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangePassword)
{
    ui->setupUi(this);

    connect(ui->btnUpdate, SIGNAL(clicked()), this, SLOT(updatePassword()));
}

ChangePassword::~ChangePassword()
{
    delete ui;
}


void ChangePassword::updatePassword()
{
    // Check if new passwords match
    if (ui->inpNewPasswd->text() != ui->inpConfirmNewPasswd->text() ) {
        QMessageBox::critical(0, "Password Error","New Passwords Do Not Match. Please Try Again.",QMessageBox::Cancel);
        return;
    }

    QString suppliedPassword = ui->inpOldPasswd->text();
    QString suppliedOperatorId = ui->inpOperatorID->text();
    QString newPasswd = ui->inpNewPasswd->text();

    // Check if operator id and password exist
    QSqlTableModel *passwdModel = new QSqlTableModel;
    passwdModel->setTable("registered_operators");
    QString passwdFilter = "`passwd`='" + suppliedPassword.toUtf8() + "' AND `oid`='" + suppliedOperatorId.toUtf8() + "' AND `active`='1'";
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


void ChangePassword::setRegExp(QString regExp) {
    QValidator *textVal = new QRegExpValidator(QRegExp(regExp), this);

    ui->inpNewPasswd->setValidator(textVal);
    ui->inpConfirmNewPasswd->setValidator(textVal);
    ui->inpOldPasswd->setValidator(textVal);
    ui->inpOperatorID->setValidator(textVal);
}
