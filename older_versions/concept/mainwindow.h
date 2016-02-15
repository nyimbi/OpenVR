#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include "recordsdialog.h"
#include "reportsdialog.h"
#include "previewdialog.h"
#include "myeventfilter.h"
#include "bozorth_check.h"
#include "fingerdialog.h"
#include "helpdialog.h"

#include <QFuture>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0,QString operatorCodeValue="");
    ~MainWindow();
    RecordsDialog *recordsDialog;
    ReportsDialog *reportsDialog;
    PreviewDialog *previewDialog;
    MyEventFilter *eventFilterHandler;



    QFuture<void> devFuture;

    bool *fingerPrintPassed;
    QString loggedOperatorCode;

    QString prevLastName;
    QString prevFirstName;
    QString prevMiddleName;


    QString prevDobDay;
    QString prevDobMonth;
    QString prevDobYear;
    QString prevGender;
    QString prevOccupation;
    QString prevAddress;
    QString prevState;
    QString prevLga;
    QString prevWard;
    QString prevMobile;






private:
    Ui::MainWindow *ui;
//    QColor m_fgcolor,m_bgcolor;
//    BarcodeItem m_bc, vin_bc;
    Bozorth_Check *bozorthCheck;
    FingerDialog *fingerDialog ;
    HelpDialog *helpDialog;


private slots:

   //void on_inpLastName_lostFocus();
   void on_btnSpecial_clicked();
   void on_btnCaptureSpecial_clicked();
   void on_actionHelpOpenVR_triggered();
   void on_cbxNotMobile_clicked();
   void on_btnCancelRegistration_clicked();
   void on_inpDobYear_lostFocus();

    void on_actionRegister_Operator_triggered();
    void on_actionControl_Panel_triggered();
    QString  getUnitDelimitation();
    void refreshUnitDelimitation();
    void displayDelimitation();
    void previewInfo();

    void lastNameOnBlur();
    void firstNameOnBlur();
    void middleNameOnBlur();
    void addressOnBlur();
    void dobDayOnBlur();
    void dobMonthOnBlur();
    void dobYearOnBlur();
    void dobAgeOnBlur();
    void mobileOnBlur();


    void processFirstName(QString);
    void processMiddleName(QString);
    void processLastName(QString);
    void processAddress();
    void processDobDay(QString);
    void processDobMonth(QString);
    void processDobYear(QString);
    void processMobile(QString);

    void preProcess();
    void closeFingerDevice();

    QString ucfirst(const QString);


    void on_capturePhotoFront_clicked();

     void on_btnCaptureLeft_clicked();
    void on_btnSave_clicked();
    void clearForm();
    bool validateForm();
    void keyPressEvent(QKeyEvent * e);

    void backupDatabase();
    bool controlLogin();

    void getAssetTag();
    void lockInterface();
    void resetIdleTimeWatcher();
    //BUT WE DONT WANT THE APP TO CLOSE AT ALL either using Alt+F4 or File->Close?
    //In mainwindow header file add under private slot
    void closeEvent(QCloseEvent *ev );
    void setupStatusBar();
    void updateAppTime();

    void confirmFingersCaptured(int );

    void getMACAddress();
    int getCurrentTime();
    void saveExperimentInfo(QString);
    void getCurrentSettings();
    QString generateNewVin();
    void fpCancelCaptureRequested();

    void fpFailedToScanSignal(int,int,int);
    void fpAcceptScan(int,int,int);

    void backupStatusChanged(int);
    void fpStatusChanged(int);
    void cameraStatusChanged(int);
    void printerStatusChanged(int);
    void clearBozorthTable();
    void fpSetCapturableFingers(QStringList,QString,QString);
    void fpResetCapturableFingers();

public slots:

        void submitRegistration();
        void exitApp();
        QByteArray capturePrint();
        void reCountRegistrations();
        void clearFingerPrints();

        void capturePhoto();

        void initiateBozorthCheck(QString,int,int,int,int);
        void completeBozorthCheck(int printIndex, QString scanResult);
        void compileResults();
        void saveAndPrint();
        void backup();

signals:
        void closingMain();        


};


#endif // MAINWINDOW_H
