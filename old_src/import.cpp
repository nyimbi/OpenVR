#include <QProcess>
#include <QDebug>
#include "import.h"
#include "ui_import.h"

Import::Import(QWidget *parent) :
    QDialog(parent), ui(new Ui::Import)
{
    ui->setupUi(this);

    qDebug() << "Importing Backups";
    QProcess *importProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    importProcess->start("/usr/local/bin/.ovr/scripts/import_backups.sh", args);
    connect(importProcess, SIGNAL(finished(int)), this, SLOT(importFinished(int)));
}

void Import::importFinished(int exitStatus)
{
    qDebug() << "Backup Import Exit Status" << QString::number(exitStatus);
    ui->btnClose->setEnabled(true);
    ui->importProgress->setMinimum(0);
    ui->importProgress->setMaximum(100);
    ui->importProgress->setValue(100);
    ui->lblStatus->setText("Backup Import Completed!");
}

Import::~Import()
{
    delete ui;
}
