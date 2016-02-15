#ifndef FINGER_THREAD_H
#define FINGER_THREAD_H

#include <QObject>
#include <QTimer>
#include "myfprint.h"
#include <QFutureWatcher>

class Finger_Thread : public QObject
{
    Q_OBJECT

public :
    Finger_Thread(QObject *parent = 0);
    ~Finger_Thread();
    void captureFinger( );
    void addCaptureLog(QString newLog);
    bool bozorth3Check(const char *filePath,int printIndex);
    bool continueCapture;
    bool isRunning;
    struct fp_dev *devLink;
    QTimer *timeOutWatch;

    int matchThreshold;
    int matchMinutiae;
    int minimumMunitiaeCount;

public slots :
    void disconnectDevice();

signals :
    void threadFinished(int);
    void threadSwitchFinger(int);
    void threadLog(QString);
    void fingerScanned(const char*,int);
    void fingerImageSaved(int);

};

#endif // FINGER_THREAD_H
