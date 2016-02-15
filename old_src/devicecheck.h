#ifndef DEVICECHECK_H
#define DEVICECHECK_H

#include <QObject>
#include <QDebug>

class DeviceCheck : public QObject
{
     Q_OBJECT
public:
    explicit DeviceCheck(QObject *parent = 0);

signals:
    void fpDeviceStatusUpdate(int);
    void printerStatusUpdate(int);
    void backupStatusUpdate(int);
    void cameraStatusUpdate(int);
    void batteryStatusUpdate(QString);
    void batteryLevelUpdate(int);


public slots:
     void checkDevices();
     void checkTimer();
     void printerStatus(int);
     int getBatteryCapacity();
     int getBatteryFullCapacity();
     void checkPrinterDevice();
     void updatePrinterStatus(int);
};

#endif // DEVICECHECK_H
