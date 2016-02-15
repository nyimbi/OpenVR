#ifndef ADMINREG_H
#define ADMINREG_H

#include <QDialog>
#include "QSqlRecord"
#include "QSqlRelationalTableModel"

namespace Ui {
    class AdminReg;
}

class AdminReg : public QDialog
{
    Q_OBJECT

public:
    explicit AdminReg(QWidget *parent = 0);
    ~AdminReg();

private:
    Ui::AdminReg *ui;
    QSqlRelationalTableModel *adminModel;
    QSqlRecord adminRecord;

private slots:
    void on_btnRegister_clicked();

public slots:
    void setRegExp(QString);
};

#endif // ADMINREG_H
