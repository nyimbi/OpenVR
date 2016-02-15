#ifndef AGGREGATE_BOZORTH_CHECK_H
#define AGGREGATE_BOZORTH_CHECK_H

#include <QObject>

class aggregate_bozorth_check : public QObject
{
    Q_OBJECT
public:
    explicit aggregate_bozorth_check(QObject *parent = 0);

signals:
    void scanResult(int,QString);
    void currentSubjectVin(QString);
    void currentTargetVin(QString);
    void currentTargetSize(int);
    void setTotalSize(int);
    void switchingSubjectVin();
    void jobCompleted();
    void scanResult(QString);
    void setProcessStage(int);
    void jobCancelled();

public slots:

    void startAfis();
private slots:
    void cleanTables();
    void processScanResult(QString);
    void extractRelevantData();
    void updateProcessStage(int);
    void checkGallery(QString,int);

};

#endif // AGGREGATE_BOZORTH_CHECK_H
