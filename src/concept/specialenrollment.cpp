#include "specialenrollment.h"
#include "ui_specialenrollment.h"
#include "QDebug"
#include "QMessageBox"

QStringList comboOptions;


SpecialEnrollment::SpecialEnrollment(QWidget *parent,QString capturableFingers) :
        QDialog(parent),
        ui(new Ui::SpecialEnrollment)
{
    ui->setupUi(this);

    comboOptions.clear();
    comboOptions << "Scan" << "Skip";

    loadFingerOptions();

    ui->btnCapturePhoto->setEnabled(false);

}

//SpecialEnrollment::~SpecialEnrollment()
//{
//    delete ui;
//}

void SpecialEnrollment::changeEvent(QEvent *e)
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


void SpecialEnrollment::loadFingerOptions(){



    ui->cmbLeftThumb->addItems(comboOptions);
    ui->cmbLeftIndex->addItems(comboOptions);
    ui->cmbLeftMiddle->addItems(comboOptions);
    ui->cmbLeftRing->addItems(comboOptions);
    ui->cmbLeftLittle->addItems(comboOptions);

    ui->cmbRightThumb->addItems(comboOptions);
    ui->cmbRightIndex->addItems(comboOptions);
    ui->cmbRightMiddle->addItems(comboOptions);
    ui->cmbRightRing->addItems(comboOptions);
    ui->cmbRightLittle->addItems(comboOptions);



}



void SpecialEnrollment::on_btnCancel_clicked()
{
    emit(cancelSpecialEnrollment());



    this->close();

}

void SpecialEnrollment::on_btnContinue_clicked()
{

    //let's validate

    QString errorMessages="";

    if(ui->txtNotes->toPlainText().isEmpty()) {

        errorMessages+="\nPlease enter NOTES to describe why this is a special enrollment";

    }

    if(ui->cmbGroup->currentText().isEmpty()) {

        errorMessages+="\nPlease select a GROUP for this special case";

    }

    if(!errorMessages.isEmpty()) {
        QMessageBox::critical(this,"Incomplete Information",errorMessages);
        return;
    }

    QStringList selectedFingers;

    if(ui->cmbLeftThumb->currentIndex()==0) selectedFingers.append("6");
    if(ui->cmbLeftIndex->currentIndex()==0) selectedFingers.append("7");
    if(ui->cmbLeftMiddle->currentIndex()==0) selectedFingers.append("8");;
    if(ui->cmbLeftRing->currentIndex()==0) selectedFingers.append("9");;
    if(ui->cmbLeftLittle->currentIndex()==0) selectedFingers.append("10");;
    if(ui->cmbRightThumb->currentIndex()==0) selectedFingers.append("1");;
    if(ui->cmbRightIndex->currentIndex()==0) selectedFingers.append("2");;
    if(ui->cmbRightMiddle->currentIndex()==0) selectedFingers.append("3");;
    if(ui->cmbRightRing->currentIndex()==0) selectedFingers.append("4");;
    if(ui->cmbRightLittle->currentIndex()==0) selectedFingers.append("5");;


    if(selectedFingers.length()==0){
        selectedFingers.append("0");
    }

    qDebug() << selectedFingers;

    emit(setFingersToCapture(selectedFingers,ui->txtNotes->toPlainText(),ui->cmbGroup->currentText()));
    this->close();
}

void SpecialEnrollment::on_btnReset_clicked()
{

    ui->cmbLeftThumb->clear();
    ui->cmbLeftIndex->clear();
    ui->cmbLeftMiddle->clear();
    ui->cmbLeftRing->clear();
    ui->cmbLeftLittle->clear();
    ui->cmbRightThumb->clear();
    ui->cmbRightIndex->clear();
    ui->cmbRightMiddle->clear();
    ui->cmbRightRing->clear();
    ui->cmbRightLittle->clear();

    ui->cmbLeftThumb->addItems(comboOptions);
    ui->cmbLeftIndex->addItems(comboOptions);
    ui->cmbLeftMiddle->addItems(comboOptions);
    ui->cmbLeftRing->addItems(comboOptions);
    ui->cmbLeftLittle->addItems(comboOptions);
    ui->cmbRightThumb->addItems(comboOptions);
    ui->cmbRightIndex->addItems(comboOptions);
    ui->cmbRightMiddle->addItems(comboOptions);
    ui->cmbRightRing->addItems(comboOptions);
    ui->cmbRightLittle->addItems(comboOptions);

    ui->txtNotes->clear();
}

void SpecialEnrollment::on_btnSkipAll_clicked()
{
    ui->cmbLeftThumb->setCurrentIndex(1);
    ui->cmbLeftIndex->setCurrentIndex(1);
    ui->cmbLeftMiddle->setCurrentIndex(1);
    ui->cmbLeftRing->setCurrentIndex(1);
    ui->cmbLeftLittle->setCurrentIndex(1);

    ui->cmbRightThumb->setCurrentIndex(1);
    ui->cmbRightIndex->setCurrentIndex(1);
    ui->cmbRightMiddle->setCurrentIndex(1);
    ui->cmbRightRing->setCurrentIndex(1);
    ui->cmbRightLittle->setCurrentIndex(1);

}

void SpecialEnrollment::on_btnScanAll_clicked()
{
    ui->cmbLeftThumb->setCurrentIndex(0);
    ui->cmbLeftIndex->setCurrentIndex(0);
    ui->cmbLeftMiddle->setCurrentIndex(0);
    ui->cmbLeftRing->setCurrentIndex(0);
    ui->cmbLeftLittle->setCurrentIndex(0);

    ui->cmbRightThumb->setCurrentIndex(0);
    ui->cmbRightIndex->setCurrentIndex(0);
    ui->cmbRightMiddle->setCurrentIndex(0);
    ui->cmbRightRing->setCurrentIndex(0);
    ui->cmbRightLittle->setCurrentIndex(0);

}
