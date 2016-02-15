#ifndef BOZORTH_CHECK_H
#define BOZORTH_CHECK_H

#include <QObject>

class Bozorth_Check : public QObject
{
    Q_OBJECT
public:
    explicit Bozorth_Check(QObject *parent = 0);

signals:
    void scanResult(int,QString);
public slots:
    void checkGallery(QString,int);
};

#endif // BOZORTH_CHECK_H
