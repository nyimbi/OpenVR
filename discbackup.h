#ifndef DISCBACKUP_H
#define DISCBACKUP_H

#include <QDialog>

namespace Ui {
    class DiscBackup;
}

class DiscBackup : public QDialog
{
    Q_OBJECT

public:
    explicit DiscBackup(QWidget *parent = 0);
    ~DiscBackup();


protected:
    void changeEvent(QEvent *e);

private:
    Ui::DiscBackup *ui;
public slots:
    void createIsoFinished(int );
    void burnFinished(int);
    void syncToHddFinished(int);
    void getOutPut(QString);

private slots:
    void on_btnQuickBackup_clicked();
    void on_btnClose_clicked();
};

#endif // DISCBACKUP_H
