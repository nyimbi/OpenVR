#ifndef IMPORT_H
#define IMPORT_H

#include <QDialog>

namespace Ui {
    class Import;
}

class Import : public QDialog
{
    Q_OBJECT

public:
    explicit Import(QWidget *parent = 0);
    ~Import();

private:
    Ui::Import *ui;

private slots:
    void importFinished(int);
};

#endif // IMPORT_H
