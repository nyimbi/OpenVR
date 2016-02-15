#ifndef REPAIRDATABASE_H
#define REPAIRDATABASE_H

#include <QDialog>

namespace Ui {
    class RepairDatabase;
}

class RepairDatabase : public QDialog
{
    Q_OBJECT

public:
    explicit RepairDatabase(QWidget *parent = 0);
    ~RepairDatabase();

private:
    Ui::RepairDatabase *ui;
};

#endif // REPAIRDATABASE_H
