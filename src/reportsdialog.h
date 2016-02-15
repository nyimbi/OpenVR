#ifndef REPORTSDIALOG_H
#define REPORTSDIALOG_H

#include <QDialog>
#include <QModelIndex>
#include <QSqlQueryModel>

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
    QString operatorId;
    QString administratorId;


private:
    Ui::ReportsDialog *ui;
    QSqlQueryModel *lgaModel;
    QSqlQueryModel *raModel;
    QSqlQueryModel *puModel;
    QSqlQueryModel *stateModel;

private slots:
    void captureReport();
    void showReports();
    void addSortField();
    void removeSortField();
    void sortMoveUp();
    void sortMoveDown();
    void sortRowSelected(QModelIndex);

//    void showGuidelines(QPainter * painter);
    void keyPressEvent(QKeyEvent * e);
    void distributionChart(QString, QRect, QString, QPainter *);
//    void on_btnClose_clicked();
    void printAggregationReport(QPrinter *printer);
    void printCaptureReport(QPrinter *printer);

public slots:
    void printPreviewAggregationReport();
    void printPreviewCaptureReport();
    void showStates();
    void showLGAinState(int);
    void showRAinLGA(int);
    void showPUinRA(int);
};


#endif // REPORTSDIALOG_H
