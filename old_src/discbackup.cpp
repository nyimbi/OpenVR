#include "discbackup.h"
#include "ui_discbackup.h"
#include "QProcess"
#include "QDebug"
#include "QMessageBox"

DiscBackup::DiscBackup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DiscBackup)
{
    ui->setupUi(this);

    ui->backupProgress->hide();

    /*
    ui->lblStatus->setText("Creating Backup Image. This may take a few minutes. Please wait");
    qDebug() << "Backing Up";
    QProcess *backupProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    //backupProcess->start("/usr/local/bin/.ovr/scripts/create_backup_iso.sh", args);
    backupProcess->start("./scripts/create_backup_iso.sh", args);

    connect(backupProcess, SIGNAL(finished(int)), this, SLOT(createIsoFinished(int)));
    */
}

void DiscBackup::createIsoFinished(int exitStatus) {

    qDebug() << "Backup Exit Status" << QString::number(exitStatus);

    ui->btnClose->setEnabled(true);
    ui->backupProgress->setMinimum(0);
    ui->backupProgress->setMaximum(100);
    ui->backupProgress->setValue(100);
    ui->lblStatus->setText("Image Created. Now Copying Image");

    QMessageBox msgBox;

    msgBox.setParent(this);

    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

    msgBox.setBackgroundRole(QPalette::Base);

    msgBox.setAutoFillBackground(true);

    msgBox.setWindowFlags(Qt::Dialog);

    msgBox.setText("Please place a BLANK CD/DVD in the drive and click YES to continue OR \nClick NO to cancel");

    msgBox.setWindowTitle("Backup to CD/DVD");

    QProcess *backupProcess = new QProcess();
    QStringList args;

    switch (msgBox.exec()) {

        case QMessageBox::Yes:
        ui->btnClose->setEnabled(false);
        ui->backupProgress->setMinimum(0);
        ui->backupProgress->setMaximum(100);


            ui->backupProgress->setValue(100);

            args << "JwgVN5aHth8J7";
            //backupProcess->start("/usr/local/bin/.ovr/scripts/create_backup_iso.sh", args);
            backupProcess->start("./scripts/backup_to_cd.sh", args);

            connect(backupProcess, SIGNAL(finished(int)), this, SLOT(burnFinished(int)));

            break;
        case QMessageBox::No:

            this->close();
            break;

    }

}

DiscBackup::~DiscBackup()
{
    delete ui;
}

void DiscBackup::changeEvent(QEvent *e)
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

void DiscBackup::on_btnClose_clicked()
{
    this->close();
}

void DiscBackup::burnFinished(int exitCode){

    qDebug() << "Burning Exit COde" << exitCode;
    ui->btnClose->setEnabled(true);
    ui->backupProgress->setMinimum(0);
    ui->backupProgress->setMaximum(100);
    QString errMsg;
    if(exitCode == 0) {

        ui->backupProgress->setValue(100);
        ui->lblStatus->setText("Finished!");
        QMessageBox::information(this, "Backup to CD/DVD Completed Successfully","A full backup has been stored on the CD/DVD", QMessageBox::Ok);
        this->close();

    } else {

       errMsg.sprintf("An error %d occurred. Unable to create Disc. Please ensure you have placed a Blank CD/DVD inside",exitCode);

       QMessageBox::critical(this, "Error",errMsg , QMessageBox::Ok);

       this->close();

    }

}

void DiscBackup::getOutPut(QString rOutput){


    qDebug()<< "Script Output" <<rOutput;

}

void DiscBackup::syncToHddFinished(int exitCode){


   QMessageBox::information(this, "Backup to External HDD Completed","A full backup has been copied to the connected external drives", QMessageBox::Ok);
   this->close();

}

void DiscBackup::on_btnQuickBackup_clicked()
{
    ui->backupProgress->show();

    ui->btnQuickBackup->setEnabled(false);

    int backupType = ui->boxBackupType->currentIndex();

    ui->lblStatus->setText("Creating Backup Image. This may take a few minutes. Please wait");

    qDebug() << "Backing Up";
    QProcess *backupProcess = new QProcess();
    QStringList args;
    QString passwd = "JwgVN5aHth8J7";
    //backupProcess->start("/usr/local/bin/.ovr/scripts/create_backup_iso.sh", args);
    QString processOutput ;
    if( backupType == 0 ){

        args << passwd << "1";

        ui->lblStatus->setText("Back Up to CD/DVD");

        ui->btnClose->setEnabled(false);

        //ui->backupProgress->setMinimum(0);

        //ui->backupProgress->setMaximum(100);

        //ui->backupProgress->setValue(100);
        ui->lblStatus->setText("Creating Backup Image for CD/DVD. Please wait...");
        backupProcess->start("./scripts/instant_backup.sh", args);

        connect(backupProcess, SIGNAL(finished(int)), this, SLOT(createIsoFinished(int)));

        //connect(backupProcess, SIGNAL(readyReadStandardOutput(QString)), this, SLOT(getOutPut(QString)));

    } else {

        args << passwd << "0";

        ui->lblStatus->setText("Back Up to External HDD");

        QMessageBox msgBox;

        msgBox.setParent(this);

        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        msgBox.setBackgroundRole(QPalette::Base);

        msgBox.setAutoFillBackground(true);

        msgBox.setWindowFlags(Qt::Dialog);

        msgBox.setText("Please connect the external drive you want to back up to and\nclick YES to continue OR Click NO to cancel");

        msgBox.setWindowTitle("Backup to External HDD");

     switch (msgBox.exec()) {

            case QMessageBox::Yes:
                ui->btnClose->setEnabled(false);

                backupProcess->start("./scripts/instant_backup.sh", args);

                connect(backupProcess, SIGNAL(finished(int)), this, SLOT(syncToHddFinished(int)));

            break;

            case QMessageBox::No:

                this->close();
                break;

        }

    }

}
