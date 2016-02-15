#ifndef SCREENSAVER_H
#define SCREENSAVER_H

#include <QDialog>

namespace Ui {
    class ScreenSaver;
}

class ScreenSaver : public QDialog
{
    Q_OBJECT

public:
    explicit ScreenSaver(QWidget *parent = 0);
    ~ScreenSaver();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *ev);

private:
    Ui::ScreenSaver *ui;

private slots:
    void on_btnExit_clicked();
    void on_btnLogin_clicked();
    void reject();
signals:
    void accessCodeSuccess(QString);
    void exitRequested();
public slots:
    void setRegExp(QString);
};

#endif // SCREENSAVER_H
