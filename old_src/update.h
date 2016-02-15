#ifndef UPDATE_H
#define UPDATE_H

#include <QDialog>

namespace Ui {
    class Update;
}

class Update : public QDialog
{
    Q_OBJECT

public:
    explicit Update(QWidget *parent = 0);
    ~Update();

private:
    Ui::Update *ui;

private slots:
    void patchingFinished(int);

};

#endif // UPDATE_H
