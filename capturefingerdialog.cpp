#include "capturefingerdialog.h"
#include "ui_capturefingerdialog.h"
#include "myfprint.h"
#include "finger_thread.h"
#include "bozorth_check.h"

#include "QInputDialog"
#include <QtDebug>
#include <QSqlQuery>
#include <QtSql>
#include <QDir>
#include <QFile>
#include "QMessageBox"
//#include "QStreamer.hpp"

//QLabel *captureLog;
//int bozorthMatches[];


CaptureFingerDialog::CaptureFingerDialog(QWidget *parent) : QDialog(parent),
    m_ui(new Ui::CaptureFingerDialog)
{
    m_ui->setupUi(this);

//    setWindowFlags(Qt::FramelessWindowHint);

//    setWindowFlags(Qt::WindowTitleHint);




    finger_thread = new Finger_Thread(this);

    connect(finger_thread, SIGNAL(threadFinished(int)), this, SLOT(threadDone(int)));

    connect(finger_thread, SIGNAL(threadLog(QString)), this, SLOT(captureFingerLog(QString)));

    connect(finger_thread, SIGNAL(threadSwitchFinger(int)), this, SLOT(switchFinger(int)));

    connect(finger_thread, SIGNAL(fingerScanned(const char*,int)), this, SLOT(fingerScanned(const char*,int)));
    connect(finger_thread, SIGNAL(fingerImageSaved(int)), this, SLOT(previewFingerprint(int)));

    future = QtConcurrent::run(finger_thread, &Finger_Thread::captureFinger  );
    //future.cancel();
    //finger_thread->isRunning = true;

    m_ui->actionButtonBox->hide();

    m_ui->btnCancelCapture->hide();


// For Fingerprint Matching Params (slider)
    connect(m_ui->matchMinutiaeSlider, SIGNAL(valueChanged(int)), this, SLOT(updateMatchMinutiae(int) ) );
    connect(m_ui->matchThresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(updateMatchThreshold(int))  );
    connect(m_ui->minimumMinutiaeCountSlider, SIGNAL(valueChanged(int)), this, SLOT(updateMinimumMinutiaeCount(int))  );
    initSliderValues();

}

void CaptureFingerDialog::threadDone(int x){

    qDebug() << "finger threadDone: " << x;
    m_ui->actionButtonBox->show();
    m_ui->actionButtonBox->setFocus();

    m_ui->btnCancelCapture->hide();

    qDebug("Done Capturing");

//    finger_thread->continueCapture = false;

////    try{
////        if(!finger_thread->devLink){
////            fp_dev_close(finger_thread->devLink);

////         //   fp_init();
////        }
////    }catch(int errM){

////        qDebug()<< errM;
////    }

    if(finger_thread->isRunning){
        qDebug()<<"Thread is still running";

        finger_thread->~Finger_Thread();
    }

    //this->close();


}

/*
void CaptureFingerDialog::reject(){

    //threadDone(0);
    //this->done(1);
  //  this->close();
    //return;
}
*/

void CaptureFingerDialog::switchFinger(int x) {

    qDebug() << "Switch to finger "<< x;

    QString fingerPrintFile;

    fingerPrintFile.sprintf("resources/finger_%d.png",x);

    QFile fileReader(fingerPrintFile);
    if (fileReader.exists()){
        QImage image(fingerPrintFile);
        if(image.height()){
            m_ui->lblHandPreview->setPixmap(QPixmap::fromImage(image));
         }
    }else{
        qDebug() << fingerPrintFile << " does not exist";
    }

}


void CaptureFingerDialog::previewFingerprint(int y){
    QString fingerPrintFile;
    fingerPrintFile.sprintf("templates/%d.pgm",y);

    QFile fileReader(fingerPrintFile);
    if (fileReader.exists()){
        QImage image(fingerPrintFile);
        if(image.height()){
            //m_ui->lblFingerPreview->setPixmap(QPixmap::fromImage(image));
         }
    }else{
        qDebug() << fingerPrintFile << " does not exist";
    }

}

void CaptureFingerDialog::captureFingerLog(QString newLog){

//    qDebug() << "Current Log Val" << newLog;

   m_ui->captureLog->setText(m_ui->captureLog->toPlainText()+newLog.toUtf8());
   QTextCursor c  = m_ui->captureLog->textCursor();
   c.movePosition(QTextCursor::End);
   m_ui->captureLog->setTextCursor(c);
//    m_ui->captureLog->scroll(0,1000);
   // m_ui->captureLog->setText(newLog.toUtf8());


}


void CaptureFingerDialog::XCaptureFingerDialog()

{
    //m_ui->setupUi(this);

}


void CaptureFingerDialog::addCaptureLog(QString newLog){

    //currentCaptureLog+=newLog+"\n";

    qDebug() << " Log: " << newLog;
    QMessageBox::critical(this, "Continue or Skip",newLog, QMessageBox::Ok);
 //   m_ui->captureLog->setText(currentCaptureLog);


}



CaptureFingerDialog::~CaptureFingerDialog()
{
    delete m_ui;
}

void CaptureFingerDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}



void CaptureFingerDialog::on_btnCancelCapture_clicked()
{

    //QtConcurrent::
    threadDone(0);

}

void CaptureFingerDialog::fingerChecked(int printIndex,QString scanResult){

    qDebug() << "Print Index: " << printIndex << " Scan Result " << scanResult;



}

void CaptureFingerDialog::fingerScanned(const char *filePath,int printIndex) {

    qDebug()<< "Signal Sent About enrollFile: " << filePath;

    qDebug() << "Calling Bozorth3Check";

    /*

    */
    previewFingerprint(printIndex);
    emit(initiateBozorthCheck(filePath,printIndex));

}

void CaptureFingerDialog::updateMatchMinutiae(int value) {
    m_ui->matchMinutiaeValue->setText( QString::number(value) );
    finger_thread->matchMinutiae = value;
}


void CaptureFingerDialog::updateMatchThreshold(int value) {
    m_ui->matchThresholdValue->setText( QString::number(value) );
    finger_thread->matchThreshold = value;
}


void CaptureFingerDialog::updateMinimumMinutiaeCount(int value) {
    m_ui->minimumMinutiaeValue->setText( QString::number(value) );
    finger_thread->minimumMunitiaeCount = value;
}


void CaptureFingerDialog::initSliderValues() {
    finger_thread->matchMinutiae = m_ui->matchMinutiaeSlider->value();
    finger_thread->matchThreshold = m_ui->matchThresholdSlider->value();
    finger_thread->minimumMunitiaeCount = m_ui->minimumMinutiaeCountSlider->value();
}
