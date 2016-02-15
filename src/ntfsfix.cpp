#include <QProcess>
#include <QDebug>
#include "ntfsfix.h"
#include "ui_ntfsfix.h"

NtfsFix::NtfsFix(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NtfsFix)
{
    ui->setupUi(this);

    qDebug() << "NTFS Fix";
    QProcess *ntfsFixProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    ntfsFixProcess->start("/usr/local/bin/.ovr/scripts/ntfsfix.sh", args);
    connect(ntfsFixProcess, SIGNAL(finished(int)), this, SLOT(ntfsFixFinished(int)));
}

void NtfsFix::ntfsFixFinished(int exitStatus)
{
    qDebug() << "Backup Import Exit Status" << QString::number(exitStatus);
    ui->btnClose->setEnabled(true);
    ui->importProgress->setMinimum(0);
    ui->importProgress->setMaximum(100);
    ui->importProgress->setValue(100);
    ui->lblStatus->setText("NTFS Backup Drive Fix!");
}

NtfsFix::~NtfsFix()
{
    delete ui;
}
