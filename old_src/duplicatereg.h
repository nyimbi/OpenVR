#ifndef DUPLICATEREG_H
#define DUPLICATEREG_H

#include <QDialog>

namespace Ui {
    class DuplicateReg;
}

class DuplicateReg : public QDialog
{
    Q_OBJECT

public:
    explicit DuplicateReg(QWidget *parent = 0);
    ~DuplicateReg();

protected:
    void changeEvent(QEvent *e);
     void closeEvent(QCloseEvent *ev);

private:
    Ui::DuplicateReg *ui;

private slots:
     void reject();

public slots:
    void loadVinDetails(int);

signals:
    void rejectRegistration();
    void acceptRegistration();


private slots:

private slots:
    void on_btnAcceptReg_clicked();
    void on_btnRejectReg_clicked();
    void on_btnNavNext_clicked();
    void on_btnNavPrev_clicked();
    void updateNavButtons();
};

#endif // DUPLICATEREG_H
