#ifndef OPERATORREG_H
#define OPERATORREG_H

#include <QDialog>
#include "QSqlRecord"
#include "QSqlRelationalTableModel"

namespace Ui {
    class OperatorReg;
}

class OperatorReg : public QDialog
{
    Q_OBJECT

public:
    explicit OperatorReg(QWidget *parent = 0);
    ~OperatorReg();

private:
    Ui::OperatorReg *ui;
    QSqlRelationalTableModel *operatorModel;
    QSqlRecord operatorRecord;

private slots:
    void on_btnRegister_clicked();

public slots:
    void setRegExp(QString);
};

#endif // OPERATORREG_H
