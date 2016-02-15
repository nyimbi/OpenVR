#ifndef OPERATORREGISTER_H
#define OPERATORREGISTER_H

#include <QDialog>

namespace Ui {
    class OperatorRegister;
}

class OperatorRegister : public QDialog
{
    Q_OBJECT

public:
    explicit OperatorRegister(QWidget *parent = 0);
    ~OperatorRegister();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::OperatorRegister *ui;

private slots:


private slots:
    void on_btnUpdateDetails_clicked();
    void on_inpOperator_currentIndexChanged(int index);
    void on_selectOperator_clicked();
    void on_btnRegister_clicked();
};

#endif // OPERATORREGISTER_H
