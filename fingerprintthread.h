#ifndef FINGERPRINTTHREAD_H
#define FINGERPRINTTHREAD_H

#include <QObject>
#include "myfprint.h"
#include "QDebug"

class FingerPrintThread : public QObject
{
    Q_OBJECT

public:

    explicit FingerPrintThread(QObject *parent = 0);

    bool dontRestartDevice;

signals:

    void processCompleted(int,QString);
    void deviceCrashed(int);

public slots:

    void captureFinger(int printIndex);
    void toggleRecovery(bool);

};

#endif // FINGERPRINTTHREAD_H
