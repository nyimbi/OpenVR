#ifndef REPORTSDIALOG_H
#define REPORTSDIALOG_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {
    class ReportsDialog;
}

class ReportsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReportsDialog(QWidget *parent = 0);
    ~ReportsDialog();
    QStringList selectedRecords;


private:
    Ui::ReportsDialog *ui;

private slots:
    void captureReport();
    void showReports();
    void addSortField();
    void removeSortField();
    void sortMoveUp();
    void sortMoveDown();
    void sortRowSelected(QModelIndex);
    void printPreviewCaptureReport();
//    void showGuidelines(QPainter * painter);
    void keyPressEvent(QKeyEvent * e);
    void distributionChart(QString, QRect, QString, QPainter *);

public slots:
    void printCaptureReport(QPrinter *printer);
};


#endif // REPORTSDIALOG_H
