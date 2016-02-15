#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QMainWindow>
#include <QFuture>

namespace Ui {
    class DashBoard;
}

class DashBoard : public QMainWindow
{
    Q_OBJECT

public:
    explicit DashBoard(QWidget *parent = 0);
    ~DashBoard();

    void keyPressEvent(QKeyEvent * e);
    QFuture<void> devFuture;

protected:

private:
    Ui::DashBoard *ui;

private slots:
    void on_btnShutdown_clicked();
//    void on_btnControlPanel_2_clicked();
    void on_btnControlPanel_clicked();
    void printReports();
    void printVotersCard();
    void on_btnRegistration_clicked();
    void restoreDashboard();
    void queueTickets();
//    void capturePhoto();
    QString operatorLogin();
    void backupStatusChanged(int);
    void fpStatusChanged(int);
    void cameraStatusChanged(int);
    void printerStatusChanged(int);
    void checkRCSetting();
    void closeEvent(QCloseEvent*);
    void backupAll();
    void importBackups();



public slots:
    bool controlLogin();

};

#endif // DASHBOARD_H
