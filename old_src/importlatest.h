#ifndef IMPORTLATEST_H
#define IMPORTLATEST_H

#include <QDialog>

namespace Ui {
    class ImportLatest;
}

class ImportLatest : public QDialog
{
    Q_OBJECT

public:
    explicit ImportLatest(QWidget *parent = 0);
    ~ImportLatest();

private:
    Ui::ImportLatest *ui;
private slots:
    void importFinished(int);

};

#endif // IMPORTLATEST_H
