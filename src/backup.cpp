#include <QProcess>
#include <QDebug>

#include "backup.h"
#include "ui_backup.h"

Backup::Backup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Backup)
{
    ui->setupUi(this);

    qDebug() << "Backing Up";
    QProcess *backupProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    backupProcess->start("/usr/local/bin/.ovr/scripts/backup.sh", args);

    connect(backupProcess, SIGNAL(finished(int)), this, SLOT(backupFinished(int)));
}

void Backup::backupFinished(int exitStatus) {
    qDebug() << "Backup Exit Status" << QString::number(exitStatus);
    ui->btnClose->setEnabled(true);
    ui->backupProgress->setMinimum(0);
    ui->backupProgress->setMaximum(100);
    ui->backupProgress->setValue(100);
    ui->lblStatus->setText("Backup Completed!");
}

Backup::~Backup()
{
    delete ui;
}
