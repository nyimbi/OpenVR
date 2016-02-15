#ifndef NTFSFIX_H
#define NTFSFIX_H

#include <QDialog>

namespace Ui {
    class NtfsFix;
}

class NtfsFix : public QDialog
{
    Q_OBJECT

public:
    explicit NtfsFix(QWidget *parent = 0);
    ~NtfsFix();

private:
    Ui::NtfsFix *ui;

private slots:
    void ntfsFixFinished(int);

};

#endif // NTFSFIX_H
