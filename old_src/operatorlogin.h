#ifndef OPERATORLOGIN_H
#define OPERATORLOGIN_H

#include <QDialog>
#include <QValidator>

namespace Ui {
    class OperatorLogin;
}

class OperatorLogin : public QDialog
{
    Q_OBJECT

public:
    explicit OperatorLogin(QWidget *parent = 0);
    ~OperatorLogin();

    QString operatorId;
    QString passwd;


private:
    Ui::OperatorLogin *ui;

public slots:
    void operatorLogin();
    void setRegExp(QString);

};


#endif // OPERATORLOGIN_H
