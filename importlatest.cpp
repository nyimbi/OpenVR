#include <QProcess>
#include <QDebug>
#include "importlatest.h"
#include "ui_importlatest.h"

ImportLatest::ImportLatest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportLatest)
{
    ui->setupUi(this);

    qDebug() << "Importing Backups";
    QProcess *importProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    importProcess->start("/usr/local/bin/.ovr/scripts/import_latest_backups.sh", args);
    connect(importProcess, SIGNAL(finished(int)), this, SLOT(importFinished(int)));
}

void ImportLatest::importFinished(int exitStatus)
{
    qDebug() << "Backup Import Exit Status" << QString::number(exitStatus);
    ui->btnClose->setEnabled(true);
    ui->importProgress->setMinimum(0);
    ui->importProgress->setMaximum(100);
    ui->importProgress->setValue(100);
    ui->lblStatus->setText("Backup Import Completed!");
}

ImportLatest::~ImportLatest()
{
    delete ui;
}
