#ifndef CAPTUREFINGERDIALOG_H
#define CAPTUREFINGERDIALOG_H

#include <QtGui/QDialog>
#include <QFuture>
#include <QProcess>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include "finger_thread.h"
#include "bozorth_check.h"


namespace Ui {
    class CaptureFingerDialog;
}

class CaptureFingerDialog : public QDialog {
    Q_OBJECT
public:
    CaptureFingerDialog(QWidget *parent = 0);
    ~CaptureFingerDialog();
    void addCaptureLog(QString newLog);

    Finger_Thread *finger_thread;
    Bozorth_Check *bozorthCheck;


    QFuture<void> future;


protected:
    void changeEvent(QEvent *e);

private:
    Ui::CaptureFingerDialog *m_ui;



public slots :
    void XCaptureFingerDialog();
    void threadDone(int);
    void captureFingerLog(QString);
    void switchFinger(int);
    void previewFingerprint(int);


private slots:

    void updateMatchMinutiae(int);
    void updateMatchThreshold(int);
    void updateMinimumMinutiaeCount(int);
    void initSliderValues();





signals:
    void initiateBozorthCheck(const char *,int);


private slots:
    void on_btnCancelCapture_clicked();
    void fingerScanned(const char *,int);
    void fingerChecked(int,QString);
    //void reject();
};

#endif // CAPTUREFINGERDIALOG_H
