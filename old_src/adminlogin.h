#ifndef ADMINLOGIN_H
#define ADMINLOGIN_H

#include <QDialog>

namespace Ui {
    class AdminLogin;
}

class AdminLogin : public QDialog
{
    Q_OBJECT

public:
    explicit AdminLogin(QWidget *parent = 0);
    ~AdminLogin();

    QString adminId;
    QString adminPasswd;

private:
    Ui::AdminLogin *ui;

private slots:
    void adminLogin();

public slots:
    void setRegExp(QString);

};

#endif // ADMINLOGIN_H
