#ifndef PASSWORDRESET_H
#define PASSWORDRESET_H

#include <QDialog>

namespace Ui {
    class PasswordReset;
}

class PasswordReset : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordReset(QWidget *parent = 0);
    ~PasswordReset();

    QString operatorId;

private:
    Ui::PasswordReset *ui;

private slots:
    void resetPassword();

public slots:
    void setRegExp(QString);

};

#endif // PASSWORDRESET_H
