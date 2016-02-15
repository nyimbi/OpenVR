#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H
#include <QDialog>

namespace Ui {
    class PreviewDialog;
}

class PreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewDialog(QWidget *parent = 0);
    ~PreviewDialog();

    void keyPressEvent(QKeyEvent *);

private:
    Ui::PreviewDialog *ui;


public slots:
    void acceptRegistration();

signals:
    void saveAndPrint();


};

#endif // PREVIEWDIALOG_H
