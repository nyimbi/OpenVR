#ifndef ADMINCHANGEPASSWORD_H
#define ADMINCHANGEPASSWORD_H

#include <QDialog>

namespace Ui {
    class AdminChangePassword;
}

class AdminChangePassword : public QDialog
{
    Q_OBJECT

public:
    explicit AdminChangePassword(QWidget *parent = 0);
    ~AdminChangePassword();

private:
    Ui::AdminChangePassword *ui;

    QString operatorId;
    QString oldPasswd;
    QString newPasswd;
    QString confirmNewPasswd;

private slots:
    void updatePassword();

public slots:
    void setRegExp(QString);
};

#endif // ADMINCHANGEPASSWORD_H
