
#ifndef RECORDSDIALOG_H
#define RECORDSDIALOG_H


// For Printing
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinter>
#include <QPainter>
#include <QFileDialog>

#include <QDialog>
#include <QKeyEvent>
#include <QModelIndex>
#include "barcodeitem.h"

namespace Ui {
    class RecordsDialog;
}

class RecordsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecordsDialog(QWidget *parent = 0);
    ~RecordsDialog();

private:
    Ui::RecordsDialog *ui;
    QColor m_fgcolor,m_bgcolor;
    BarcodeItem m_bc, vin_bc;

private slots:
    void previewSelected();

    void on_printSelectedButton_clicked();
    QString getSearchFilter();
    void searchRecords();
    void keyPressEvent(QKeyEvent * e);
    void recGridPresentation();


public slots:
    void showStates();
    void showLGAinState(int);
    void showRAinLGA(int);
    void showPUinRA(int);
    void addSortField();
    void removeSortField();
    void sortMoveUp();
    void sortMoveDown();
    void sortRowSelected(QModelIndex);

    void printTVC(QPrinter * printer);
    void filePrint(QString);
    void filePrintPreview(int);

    void filePrintPreview(QStringList);

signals:
    void printTVCforSelected(int);
    void printPreviewTVCforSelected(QStringList);
    void previewCaptureReport(QStringList);

};

#endif // RECORDSDIALOG_H
