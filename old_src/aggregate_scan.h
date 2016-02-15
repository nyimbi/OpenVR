#ifndef AGGREGATE_SCAN_H
#define AGGREGATE_SCAN_H

#include <QMainWindow>
#include <QFuture>
#include <QProcess>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QDialog>

namespace Ui {
    class aggregate_scan;
}

class aggregate_scan : public QDialog
{
    Q_OBJECT

public:
    explicit aggregate_scan(QWidget *parent = 0);
    ~aggregate_scan();
    aggregate_scan *bzCheck;
    QFuture<void> future;
    bool continueAfis;

protected:
    void changeEvent(QEvent *e);

private:
    Ui::aggregate_scan *ui;
public slots:
    void runScan();


private slots:
    void on_btnCancel_clicked();
    void on_btnRunAfis_clicked();
    void jobCompleted();
    void setCurrentProgressBar(int);
    void setUnverifiedSize(int);
    void jobCancelled();
    void setCurrentVin(QString);
    void setCurrentTarget(QString);
    void setCurrentTargetSize(int);
    void updateCurrentProgress(int);
   void updateTotalProgress(int);
   void switchSubjectVin();
};

#endif // AGGREGATE_SCAN_H
