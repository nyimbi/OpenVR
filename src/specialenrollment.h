#ifndef SPECIALENROLLMENT_H
#define SPECIALENROLLMENT_H

#include <QDialog>

namespace Ui {
    class SpecialEnrollment;
}

class SpecialEnrollment : public QDialog
{
    Q_OBJECT

public:
    explicit SpecialEnrollment(QWidget *parent = 0,QString capturableFingers=QString(""));

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SpecialEnrollment *ui;

private slots:
    void on_btnCapturePhoto_clicked();
    void on_btnScanAll_clicked();
    void on_btnSkipAll_clicked();
    void on_btnReset_clicked();
    void on_btnContinue_clicked();
    void on_btnCancel_clicked();
    void loadFingerOptions();
    void finishPhotoCapture(int);
signals:
    void cancelSpecialEnrollment();
    void setFingersToCapture(QStringList,QString,QString);

};

#endif // SPECIALENROLLMENT_H
