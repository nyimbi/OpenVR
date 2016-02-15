#ifndef BACKUP_H
#define BACKUP_H

#include <QDialog>

namespace Ui {
    class Backup;
}

class Backup : public QDialog
{
    Q_OBJECT

public:
    explicit Backup(QWidget *parent = 0);
    ~Backup();

private:
    Ui::Backup *ui;

private slots:
    void backupFinished(int);
};

#endif // BACKUP_H
