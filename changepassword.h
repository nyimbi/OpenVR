#ifndef CHANGEPASSWORD_H
#define CHANGEPASSWORD_H

#include <QDialog>

namespace Ui {
    class ChangePassword;
}

class ChangePassword : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePassword(QWidget *parent = 0);
    ~ChangePassword();

private:
    Ui::ChangePassword *ui;

    QString operatorId;
    QString oldPasswd;
    QString newPasswd;
    QString confirmNewPasswd;


private slots:
    void updatePassword();

public slots:
    void setRegExp(QString);
};

#endif // CHANGEPASSWORD_H
