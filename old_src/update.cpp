#include "update.h"
#include "ui_update.h"

#include <QProcess>
#include <QDebug>

Update::Update(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Update)
{
    ui->setupUi(this);

    qDebug() << "Patching ...";
    QProcess *patchProcess = new QProcess();
    QStringList args;
    args << "JwgVN5aHth8J7";
    patchProcess->start("/usr/local/bin/.ovr/scripts/import_patches.sh", args);
    connect(patchProcess, SIGNAL(finished(int)), this, SLOT(patchingFinished(int)));
}

Update::~Update()
{
    delete ui;
}


void Update::patchingFinished(int exitStatus) {
    qDebug() << "Patching Exit Status: " << QString::number(exitStatus);
    ui->btnClose->setEnabled(true);
    ui->updateProgress->setMinimum(0);
    ui->updateProgress->setMaximum(100);
    ui->updateProgress->setValue(100);
    ui->lblStatus->setText("Update Completed! Please Restart DDC Laptop.");
}
