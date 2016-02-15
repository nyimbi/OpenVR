#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <QMainWindow>
#include <QFuture>
#include <QValidator>

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
    QValidator *textRx;

protected:

private:
    Ui::DashBoard *ui;

private slots:
    void on_actionRepair_NTFS_Backup_Drive_triggered();
    void on_actionImport_Latest_Backups_triggered();
    void on_actionBackup_to_CD_DVD_triggered();
    void on_btnWipeOut_clicked();
    void on_actionPrint_Aggregation_Report_triggered();
    void on_actionManual_triggered();
    void on_actionLaunch_Aggregate_AFIS_triggered();
    void on_actionChange_Admin_Password_triggered();
    void on_actionAdministrator_Registration_triggered();
    void on_actionOperator_Registration_triggered();
    void on_actionChange_Password_triggered();
    void on_actionAbout_OpenVR_triggered();
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
    void batteryStatusChanged(QString);
    void batteryLevelChanged(int);

    void checkRCSetting();
    void closeEvent(QCloseEvent*);
    void backupAll();
    void importBackups();
    void printerSetup();
    void printerConfig();
    void showDash(int);
    void restoreAudioVolume();
    void hiddenDialog();
    void removeRedundantPrinter();

public slots:
    bool controlLogin();

};

#endif // DASHBOARD_H
