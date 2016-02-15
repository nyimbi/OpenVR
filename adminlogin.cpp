#include "adminlogin.h"
#include "ui_adminlogin.h"
#include "QDebug"

AdminLogin::AdminLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdminLogin)
{
    ui->setupUi(this);

    ui->inpAdminID->setFocus();
    connect(ui->btnLogin, SIGNAL(clicked()), this, SLOT(adminLogin()));
}

AdminLogin::~AdminLogin()
{
    delete ui;
}

void AdminLogin::adminLogin()
{
    adminId = ui->inpAdminID->text();
    adminPasswd = ui->inpPasswd->text();

    qDebug() << "Admin ID: " << ui->inpAdminID->text().toAscii();
    qDebug() << "Password: " << ui->inpPasswd->text();
    this->close();
}


void AdminLogin::setRegExp(QString regExp) {
    QValidator *textVal = new QRegExpValidator(QRegExp(regExp), this);

    ui->inpAdminID->setValidator(textVal);
    ui->inpPasswd->setValidator(textVal);
}


