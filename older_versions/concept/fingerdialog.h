#ifndef FINGERDIALOG_H
#define FINGERDIALOG_H

#include <QDialog>
#include <QFuture>
#include <QProcess>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include "fingerprintthread.h"


namespace Ui {
    class FingerDialog;
}

class FingerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FingerDialog(QWidget *parent = 0,int fingersToCapture=10, int totalPassesRequired=1,int sensitivityId=1,QStringList capturableFingers=QStringList(QString( )));
    ~FingerDialog();
    FingerPrintThread *fingerPrintThread;
    bool continueCaptureProcess;
    QFuture<void> future;
    int cfgFingersToCapture;
    int cfgTotalPassesLeft;
    QStringList cfgCapturableFingers;
    int fingersCaptured;
    int failedScans;


public slots:
    void addCaptureLog(QString);
    void closeDevice();
    void startCapture();
    void scanFinger(int);
    void fingerCaptured(int,QString);
    void displayMessage(QString message,int);
    void clearMessages();
    void restartScan(int);
    int isDoubleScanned(int,QString);



protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *evt);

private:
    Ui::FingerDialog *ui;

private slots:
    void on_btnRestartDevice_clicked();
     void on_actionButtonBox_clicked();
    //void on_FingerDialog_finished(int result);
    void on_btnCancelCapture_clicked();
    void hideEvent(QHideEvent*);
    void unflashVisualLog();
    void audibleSound(int);
    int printQuality(QString);
    void reject();

signals:
    void initiateBozorthCheck(QString,int,int,int,int);
    void isClosing(int);
    void rejected(int);
    void cancelCaptureRequested();
    void failedToScanSignal(int,int,int);
    void successfulFingerScan(int,int,int);
};

#endif // FINGERDIALOG_H
