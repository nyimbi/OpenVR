#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QComboBox>


namespace Ui {
    class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = 0);
    ~ConfigDialog();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ConfigDialog *ui;
    int stateId;
    int lgaId;
    int raId;



private slots:
    void on_btnCreatePu_clicked();
    void on_inpPuRa_currentIndexChanged(QString );
    void on_tabWidget_currentChanged(int index);
    void on_inpPuLga_currentIndexChanged(QString );
    void on_inpPuState_currentIndexChanged(QString );
    void on_inpLga_currentIndexChanged(QString );
    void on_inpState_currentIndexChanged(QString );
    void on_btnCreateRa_clicked();
    void loadStates();
    void loadLgas(QString,QComboBox*);
    void loadRas(QString,QComboBox*);
    void clearForms();
    QString createRaId(int,int);
};

#endif // CONFIGDIALOG_H
