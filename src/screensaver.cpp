#include "screensaver.h"
#include "ui_screensaver.h"
#include "QCloseEvent"
#include "QSqlQuery"
#include "QMessageBox"
#include "QDesktopWidget"
#include "QSqlTableModel"
#include "QDebug"
#include "QSqlRecord"


bool allowToClose = false;

ScreenSaver::ScreenSaver(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScreenSaver)
{
    ui->setupUi(this);

        //this->showFullScreen();
    QDesktopWidget *xa = QApplication::desktop();
    int screenWidth = xa->screenGeometry().width();
    int screenHeight = xa->screenGeometry().height();
    //this->setGeometry(0,0,xa->screenGeometry().width(),xa->screenGeometry().height());
    this->setMinimumSize(screenWidth,screenHeight);
    this->setMaximumSize(screenWidth,screenHeight);
        //setWindowFlags(Qt::FramelessWindowHint);

    //    setWindowFlags(Qt::WindowTitleHint);

}

ScreenSaver::~ScreenSaver()
{
    delete ui;
}

void ScreenSaver::changeEvent(QEvent *e)
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


void ScreenSaver::closeEvent(QCloseEvent *ev) {

    if(allowToClose == true){
        ev->accept();
        return;
    }

    ev->ignore();

}

void ScreenSaver::reject() {
    //to ignore ESC key
    return;

}


void ScreenSaver::on_btnLogin_clicked()
{

    //    bool ok = false;
        QString operatorCodeValue;
        QString suppliedPassword, suppliedOperatorId;
    //    QString suppliedPassword = QInputDialog::getText(this, tr("Please Authorization Required"),
    //                                                       tr("Please Enter Your \nOperator Access Code:"), QLineEdit::Password,
    //                                                       "", &ok);

        suppliedPassword = ui->txtAccessCode->text();
        suppliedOperatorId = ui->txtOperatorID->text();

        if (!suppliedPassword.isEmpty()) {
            QSqlTableModel *loginModel = new QSqlTableModel;
            loginModel->setTable("registered_operators");
            QString loginFilter = "passwd='" + suppliedPassword + "' AND oid='" + suppliedOperatorId + "'";
            qDebug() << "Login Filter: " << loginFilter;
            loginModel->setFilter(loginFilter);
            loginModel->select();

            int loginRows = loginModel->rowCount();
            qDebug() << "Login Rows: " << QString::number(loginRows);

            if (loginRows)
            {
                if (loginModel->record(0).value("active").toInt() == 1)
                {
                    QMessageBox::information(this, "Access Granted", "Welcome "+ loginModel->record(0).value("first_name").toString() + " " + loginModel->record(0).value("last_name").toString() + ".\nPlease proceed.", QMessageBox::Ok);
                    allowToClose = true;
                    operatorCodeValue = suppliedOperatorId;
                    emit(accessCodeSuccess(operatorCodeValue));
                    this->close();

                } else {
                    QMessageBox::critical(this, "Authorization Error","Your access code has been deactivated.", QMessageBox::Ok);
                }
            } else {
                QMessageBox::critical(this,"Authorization Error","Please enter your access code below to log-in", QMessageBox::Ok);
            }
            ui->txtAccessCode->clear();
            ui->txtOperatorID->clear();
       }

//    QString suppliedAccessCode = ui->txtAccessCode->text();

//    //There must be a better way
//    QString operatorCodeValue;

//    if (!suppliedAccessCode.isEmpty()){
//        QSqlQuery dbQuery;
//        QString sqlQuery= "SELECT * FROM registered_operators WHERE access_code='"+suppliedAccessCode.toUtf8()+"'";
//        dbQuery.exec(sqlQuery);

//        if(dbQuery.next()){
//            if(dbQuery.value(4).toInt()==1){
//                QMessageBox::information(this, "Access Granted","Welcome "+dbQuery.value(2).toString()+" "+dbQuery.value(3).toString()+".\nPlease proceed.", QMessageBox::Ok);

//                operatorCodeValue = dbQuery.value(1).toString();
//                allowToClose = true;

//                emit(accessCodeSuccess(operatorCodeValue));

//                this->close();
//                //qDebug() << " Operator Code: "<< operatorCodeValue;
//                //return operatorCodeValue;
//            } else {
//                QMessageBox::critical(this, "Authorization Error","Your access code has been deactivated.", QMessageBox::Ok);
//            }
//        } else {

//            QMessageBox::critical(this, "Authorization Error","Invalid access code provided. Please check and try again", QMessageBox::Ok);

//        }
//    }else{

//        QMessageBox::critical(this,"Authorization Error","Please enter your access code below to log-in", QMessageBox::Ok);
//    }
//    ui->txtAccessCode->clear();

}

void ScreenSaver::on_btnExit_clicked()
{

    QMessageBox msgBox;
    msgBox.setParent(this);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setBackgroundRole(QPalette::Base);
    msgBox.setAutoFillBackground(true);
    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setText("Any data currently on the registration form will be lost! Are you sure you want to exit the registration window and return to the dashboard?");
    msgBox.setWindowTitle("Exit Registration?");
    msgBox.setParent(this);

    switch (msgBox.exec()) {

    case QMessageBox::Yes:

        allowToClose = true;
        emit(exitRequested());
        this->close();
        break;

    }

}

void ScreenSaver::setRegExp(QString regExp) {
    QValidator *textVal = new QRegExpValidator(QRegExp(regExp), this);

    ui->txtAccessCode->setValidator(textVal);
    ui->txtOperatorID->setValidator(textVal);
}
