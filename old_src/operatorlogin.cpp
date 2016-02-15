#include "operatorlogin.h"
#include "ui_operatorlogin.h"
#include "QDebug"
#include "QValidator"

OperatorLogin::OperatorLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OperatorLogin)
{
    ui->setupUi(this);
    ui->inpOperatorID->setFocus();
    connect(ui->btnLogin, SIGNAL(clicked()), this, SLOT(operatorLogin()));



}

OperatorLogin::~OperatorLogin()
{
    delete ui;
}

void OperatorLogin::operatorLogin()
{
    operatorId = ui->inpOperatorID->text();
    passwd = ui->inpPasswd->text();


    qDebug() << "Operator ID: " << ui->inpOperatorID->text().toAscii();
    qDebug() << "Password: " << ui->inpPasswd->text();

    this->close();
}

void OperatorLogin::setRegExp(QString regExp) {
    QValidator *textVal = new QRegExpValidator(QRegExp(regExp), this);

    ui->inpOperatorID->setValidator(textVal);
    ui->inpPasswd->setValidator(textVal);
}
