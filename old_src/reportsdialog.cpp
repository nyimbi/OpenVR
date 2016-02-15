#include <QMessageBox>
#include <QDebug>
#include <QSqlTableModel>
#include <QPainter>
#include <QPrinter>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QPrintPreviewDialog>
#include <QDateTime>
#include <QKeyEvent>

#include "reportsdialog.h"
#include "ui_reportsdialog.h"
#include "nightcharts.h"

QSqlQueryModel *reportsModel;
QSqlQueryModel *pulistModel;



QSqlQueryModel model;
QStandardItemModel reportSortFields;

QString mainTitle = "FEDERAL REPUBLIC OF NIGERIA";

ReportsDialog::ReportsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ReportsDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("OpenVR .:. Reports Generator");

    connect(ui->previewCaptureReport, SIGNAL(clicked()), this, SLOT( printPreviewCaptureReport() ) );
//    connect(ui->previewPrintAggregationReport, SIGNAL(clicked()), this, SLOT( printPreviewAggregationReport() ) );


    connect(ui->sortAddButton, SIGNAL(clicked()), this, SLOT( addSortField()) );
    connect(ui->sortRemoveButton, SIGNAL(clicked()), this, SLOT( removeSortField()) );
    connect(ui->sortMoveUpButton, SIGNAL(clicked()), this, SLOT( sortMoveUp()) );
    connect(ui->sortMoveDownButton, SIGNAL(clicked()), this, SLOT( sortMoveDown()) );
    connect(ui->sortTableView, SIGNAL(pressed(QModelIndex)), this, SLOT(sortRowSelected(QModelIndex)));


    connect(ui->searchStateList, SIGNAL(currentIndexChanged(int)), this, SLOT( showLGAinState(int) ) );
    connect(ui->searchLGAList, SIGNAL(currentIndexChanged(int)), this, SLOT( showRAinLGA(int)) );
    connect(ui->searchRAList, SIGNAL(currentIndexChanged(int)), this, SLOT( showPUinRA(int)) );
    showStates();

    ui->sortTableView->setModel(&reportSortFields);
    showReports();
}

ReportsDialog::~ReportsDialog()
{
    delete ui;
}

void ReportsDialog::showReports() {
    QString sql = "SELECT title, id FROM reports ORDER BY report_order ASC";

    reportsModel = new QSqlQueryModel;
    reportsModel->setQuery(sql);
    int numRows = reportsModel->rowCount(QModelIndex());
    qDebug() << numRows;

    for(int i=0; i<numRows; ++i) {
       QModelIndex reportIndex = reportsModel->index(i, 1);
       QString report = reportsModel->data(reportIndex, Qt::DisplayRole).toString();
        qDebug() << report;
    }

    ui->reportList->setModel(reportsModel);
}

void ReportsDialog::captureReport()
{
    qDebug() << "Previewing Capture Report:";



// Get ID of report to be printed

//    if (selectedIds.size() > 0) {
//        emit previewCaptureReport(selectedIds);
//    } else {
//        QMessageBox::critical(this, "Error :: No Record Selected", "Please select a record and try again.", QMessageBox::Ok);
//    }
}

void ReportsDialog::printPreviewAggregationReport()
{
#ifndef QT_NO_PRINTER

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setOrientation(QPrinter::Landscape);
    printer.setPageMargins(0, 0 ,0 , 0, QPrinter::Millimeter );
    printer.setOutputFileName("/tmp/aggregation_report.pdf");
//    printer.setOutputFileName("/home/sajayi/Desktop/report.pdf");

    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowFlags ( Qt::Dialog );
    preview.setWindowState(Qt::WindowMaximized);
//    preview.setWindowState(Qt::WindowFullScreen);

    connect(&preview, SIGNAL(paintRequested(QPrinter *)), SLOT(printAggregationReport(QPrinter *)));
    preview.exec();
#endif
}




/**
  * Aggregation Report (Statistics)
  *
  * Generates reports showing breaking by polling units and operators.
  *
  */
void ReportsDialog::printAggregationReport(QPrinter *printer)
{
#ifndef QT_NO_PRINTER


//    qDebug() << "Printing Report " << ui->reportList->currentText();

//    int rowIndex = ui->reportList->currentIndex();
//    QModelIndex selectedReport = reportsModel->index(rowIndex, 1);
//    int reportId = reportsModel->data(selectedReport, Qt::DisplayRole).toInt();


    QSqlTableModel adminModel;
    adminModel.setTable("administrators");
    adminModel.setFilter(QString("rid=%1").arg(this->administratorId));
    adminModel.select();

    QString adminName = adminModel.record(0).value("first_name").toString() + " " + adminModel.record(0).value("last_name").toString();


    QSqlTableModel reportInfo;
    reportInfo.setTable("aggregation_reports");

    reportInfo.setFilter(QString("id=%1").arg("1"));
    reportInfo.select();

//    QString reportSQL = reportInfo.record(0).value("filter_statement").toString();
//    QString puSQL = reportInfo.record(0).value("polling_units_statement").toString();
//    QString reportTitle = reportInfo.record(0).value("title").toString();
    QString reportTitle = reportInfo.record(0).value("title").toString();
    QString puSummarySql = reportInfo.record(0).value("pu_summary").toString();
    QString opSummarySql = reportInfo.record(0).value("operator_summary").toString();
    QString opPUSummarySql = reportInfo.record(0).value("operator_pu_summary").toString();
    bool showSummary = reportInfo.record(0).value("show_summary").toBool();

    qDebug() << "Report Title: " << reportTitle;


    QFont footerFont;
    footerFont.setFamily("Arial");
    footerFont.setItalic(true);
    footerFont.setPointSize(6);


    QFont summaryTitleFont;
    summaryTitleFont.setFamily("Arial");
    summaryTitleFont.setPointSize(10);

    QFont summaryHeadFont;
    summaryHeadFont.setBold(true);
    summaryHeadFont.setFamily("Arial");
    summaryHeadFont.setPointSize(6);

    QFont summaryFont;
    summaryFont.setFamily("Arial");
    summaryFont.setPointSize(5);

    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setFamily("Arial");
    titleFont.setPointSize(14);

    QFont subtitleFont;
    subtitleFont.setBold(true);
    subtitleFont.setFamily("Arial");
    subtitleFont.setPointSize(10);


    QString logoFile = "./resources/logo.png";
    QImage logo;
    logo.load(logoFile);

    QString coatOfArmsFile = "./resources/coat_of_arms.jpg";
    QImage coatOfArms;
    coatOfArms.load(coatOfArmsFile);

    QPainter painter;

    // Page 1
    if (! painter.begin(printer)) { // failed to open file
        qWarning("Failed to open file, is it writable?");
    }
//    pulistModel = new QSqlQueryModel;
//    pulistModel->setQuery(puSQL);

//    bool printPUSummary;
    int headerLabelX = 1500;
    int headerLabelY = 900;
    int headerValueX = 3000;
    int headerValueY = 1500;
    int headerLineSpacing = 200;
//    int headerColumnMargin = 3750;
//    int marginTop = 2000;
//    int page = 1;

    // Summary
    bool printSummary = true;
    int pageNo = 1;

    if (printSummary && showSummary ) { // Print Summary

        painter.setFont(titleFont);

        QRectF titleArea0(1500, 75, 11000, 500);
        QRectF titleBox0  = painter.boundingRect(titleArea0, Qt::TextWordWrap, mainTitle );
        painter.drawText(titleArea0, Qt::AlignCenter | Qt::AlignTop, mainTitle, &titleBox0);

        painter.setFont(subtitleFont);
        QRectF subtitleArea0(1500, titleBox0.height() + 50, 11000, 500);
        QRectF subtitleBox0  = painter.boundingRect(subtitleArea0, Qt::TextWordWrap, reportTitle );
        painter.drawText(subtitleArea0, Qt::AlignCenter | Qt::AlignTop, reportTitle, &subtitleBox0);

        painter.drawImage(400, 300, coatOfArms.scaledToWidth(700));
        painter.drawImage(13100, 300, logo.scaledToWidth(400));
        painter.setFont(footerFont);
        painter.drawText(500, 9500, "Printed by " + adminName + " (" + this->administratorId + ") on " + QDateTime::currentDateTime().toString() );
        painter.drawText(13200, 9500, "Page " + QString::number(pageNo));
        pageNo++;


        // Operator Reg Report.

        painter.setFont(summaryTitleFont);
        painter.drawText(500, 1400, "Registrations Per Operator");

        QSqlQueryModel *opSummaryModel = new QSqlQueryModel;
        opSummaryModel->setQuery(opSummarySql);
        qDebug() << "Summary PU in results: " << opSummaryModel->rowCount();

        headerLabelX = 3000;
        headerLabelY = 2500;
        headerValueX = 500;
        headerValueY = 1500;
        int cellWidth = 1500;
        int cellHeight = 2500;
        int cellSpacing = 50;

        painter.setFont(summaryHeadFont);

        cellWidth = 750;
        QRectF sumArea(headerValueX, headerValueY, cellWidth, cellHeight);
        QRectF sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Operator ID" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Operator ID", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "First Name" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "First Name", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Lastname" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Lastname", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Delimitation" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Delimitation", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Male" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Male", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Female" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Female", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Normal" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Normal", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Special" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Special", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 500;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Duplicates" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Duplicates", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Total" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Total", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        headerValueY += headerLineSpacing;

        for(int k=0; k < opSummaryModel->rowCount(); k++)
        {
            headerValueX = 500;
            painter.setFont(summaryFont);

            QString oid = opSummaryModel->record(k).value("OID").toString();
            QString fname = opSummaryModel->record(k).value("Firstname").toString();
            QString lname = opSummaryModel->record(k).value("Lastname").toString();
            QString delim = opSummaryModel->record(k).value("Delimitation").toString();
            QString male = opSummaryModel->record(k).value("Males").toString();
            QString female = opSummaryModel->record(k).value("Females").toString();
            QString normal = opSummaryModel->record(k).value("Normal").toString();
            QString special = opSummaryModel->record(k).value("Special").toString();
            QString duplicates = opSummaryModel->record(k).value("Duplicates").toString();
            QString total = opSummaryModel->record(k).value("Total Registrations").toString();

            cellWidth = 750;
            QRectF sumArea(headerValueX, headerValueY, cellWidth, cellHeight);
            QRectF sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, oid);
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, oid, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 750;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, fname );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, fname, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 750;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, lname );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, lname, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 750;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, delim );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, delim, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, male );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, male, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, female );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, female , &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, normal );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, normal, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, special );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, special, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 500;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, duplicates );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, duplicates, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, total );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, total, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            headerValueY += (headerLineSpacing);

            if (headerValueY > 9200) { // Change Page

                // If possible print table headers again.

                headerValueY = 1500;
                printer->newPage();


                painter.drawImage(400, 300, coatOfArms.scaledToWidth(700));
                painter.drawImage(13100, 300, logo.scaledToWidth(400));
                painter.setFont(footerFont);

                painter.drawText(500, 9500, "Printed by " + adminName + " (" + this->operatorId + ") on " + QDateTime::currentDateTime().toString() );
                painter.drawText(13200, 9500, "Page " + QString::number(pageNo));
                pageNo++;

                painter.setFont(titleFont);

                QRectF titleArea0(1500, 75, 11000, 500);
                QRectF titleBox0  = painter.boundingRect(titleArea0, Qt::TextWordWrap, mainTitle );
                painter.drawText(titleArea0, Qt::AlignCenter | Qt::AlignTop, mainTitle, &titleBox0);

                painter.setFont(subtitleFont);
                QRectF subtitleArea0(1500, titleBox0.height() + 50, 11000, 500);
                QRectF subtitleBox0  = painter.boundingRect(subtitleArea0, Qt::TextWordWrap, reportTitle );
                painter.drawText(subtitleArea0, Qt::AlignCenter | Qt::AlignTop, reportTitle, &subtitleBox0);
            }
        }
        headerValueY += (headerLineSpacing);
        headerValueY += (headerLineSpacing);


        // REGISTRATIONS PER OPERATOR PER PU
        headerValueX = 500;

        painter.setFont(summaryTitleFont);
        painter.drawText(500, headerValueY, "Registrations Per Operator Per Polling Unit");
        headerValueY += headerLineSpacing;

        QSqlQueryModel *opPUSummaryModel = new QSqlQueryModel;
        opPUSummaryModel->setQuery(opPUSummarySql);
        qDebug() << "Summary PU in results: " << opPUSummaryModel->rowCount();

        painter.setFont(summaryHeadFont);

        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Operator ID" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Operator ID", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "First Name" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "First Name", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Lastname" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Lastname", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Delimitation" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Delimitation", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Male" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Male", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Female" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Female", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Normal" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Normal", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Special" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Special", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 500;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Duplicates" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Duplicates", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Total" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Total", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        headerValueY += headerLineSpacing;


        for(int k=0; k < opPUSummaryModel->rowCount(); k++)
        {
            headerValueX = 500;
            painter.setFont(summaryFont);

            QString oid = opPUSummaryModel->record(k).value("OID").toString();
            QString fname = opPUSummaryModel->record(k).value("Firstname").toString();
            QString lname = opPUSummaryModel->record(k).value("Lastname").toString();
            QString delim = opPUSummaryModel->record(k).value("Delimitation").toString();
            QString male = opPUSummaryModel->record(k).value("Males").toString();
            QString female = opPUSummaryModel->record(k).value("Females").toString();
            QString normal = opPUSummaryModel->record(k).value("Normal").toString();
            QString special = opPUSummaryModel->record(k).value("Special").toString();
            QString duplicates = opPUSummaryModel->record(k).value("Duplicates").toString();
            QString total = opPUSummaryModel->record(k).value("Total Registrations").toString();

            cellWidth = 750;
            QRectF sumArea(headerValueX, headerValueY, cellWidth, cellHeight);
            QRectF sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, oid);
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, oid, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 750;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, fname );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, fname, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 750;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, lname );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, lname, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 750;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, delim );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, delim, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, male );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, male, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, female );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, female , &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, normal );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, normal, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, special );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, special, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 500;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, duplicates );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, duplicates, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, total );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, total, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            headerValueY += (headerLineSpacing);

            if (headerValueY > 9200) { // Change Page

                // If possible print table headers again.

                headerValueY = 1500;
                printer->newPage();


                painter.drawImage(400, 300, coatOfArms.scaledToWidth(700));
                painter.drawImage(13100, 300, logo.scaledToWidth(400));
                painter.setFont(footerFont);

                painter.drawText(500, 9500, "Printed by " + adminName + " (" + this->operatorId + ") on " + QDateTime::currentDateTime().toString() );
                painter.drawText(13200, 9500, "Page " + QString::number(pageNo));
                pageNo++;

                painter.setFont(titleFont);

                QRectF titleArea0(1500, 75, 11000, 500);
                QRectF titleBox0  = painter.boundingRect(titleArea0, Qt::TextWordWrap, mainTitle );
                painter.drawText(titleArea0, Qt::AlignCenter | Qt::AlignTop, mainTitle, &titleBox0);

                painter.setFont(subtitleFont);
                QRectF subtitleArea0(1500, titleBox0.height() + 50, 11000, 500);
                QRectF subtitleBox0  = painter.boundingRect(subtitleArea0, Qt::TextWordWrap, reportTitle );
                painter.drawText(subtitleArea0, Qt::AlignCenter | Qt::AlignTop, reportTitle, &subtitleBox0);
            }
        }
        headerValueY += (headerLineSpacing);
        headerValueY += (headerLineSpacing);




        // States Reg Report Summary
        painter.setFont(summaryTitleFont);
        painter.drawText(500, headerValueY, "Registered Voters Per Polling Unit");
        headerValueY += (headerLineSpacing);

        QSqlQueryModel *puSummaryModel = new QSqlQueryModel;
        puSummaryModel->setQuery(puSummarySql);
        qDebug() << "Summary PU in results: " << puSummaryModel->rowCount();/*

        painter.drawImage(400, 300, coatOfArms.scaledToWidth(1000));
        painter.drawImage(9100, 300, logo.scaledToWidth(600));

        painter.setFont(titleFont);

         titleArea0 = QRectF(1500, 75, 7000, 500);
        titleBox0  = painter.boundingRect(titleArea0, Qt::TextWordWrap, mainTitle );
        painter.drawText(titleArea0, Qt::AlignCenter | Qt::AlignTop, mainTitle, &titleBox0);

        painter.setFont(subtitleFont);
        subtitleArea0(1500, titleBox0.height() + 50, 7000, 500);
        QRectF subtitleBox0  = painter.boundingRect(subtitleArea0, Qt::TextWordWrap, reportTitle );
        painter.drawText(subtitleArea0, Qt::AlignCenter | Qt::AlignTop, reportTitle, &subtitleBox0);*/

        headerLabelX = 3000;
        headerLabelY = 2500;
        headerValueX = 500;
//        headerValueY = 1500;
//        int cellWidth = 1500;
//        int cellHeight = 2500;
//        int cellSpacing = 50;

        painter.setFont(summaryHeadFont);
        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "State" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "State", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 1250;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Local Govt." );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Local Govt.", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 2250;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Registration Area" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Registration Area", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 3750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Polling Unit" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Polling Unit", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 750;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Delimitation" );
        painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, "Delimitation", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Male" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Male", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Female" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Female", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Normal" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Normal", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Special" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Special", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 500;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Duplicates" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Duplicates", &sumBox);
        headerValueX += cellWidth + cellSpacing;

        cellWidth = 450;
        sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
        sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, "Total" );
        painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, "Total", &sumBox);
        headerValueX += cellWidth + cellSpacing;


        headerValueY += headerLineSpacing;


        for(int k=0; k < puSummaryModel->rowCount(); k++)
        {
            headerValueX = 500;
            painter.setFont(summaryFont);

            QString state = puSummaryModel->record(k).value("State").toString();
            QString lga = puSummaryModel->record(k).value("Local Govt.").toString();
            QString ra = puSummaryModel->record(k).value("Registration Area").toString();
            QString pu = puSummaryModel->record(k).value("Polling Unit").toString();
            QString delim = puSummaryModel->record(k).value("Delimitation").toString();
            QString male = puSummaryModel->record(k).value("Males").toString();
            QString female = puSummaryModel->record(k).value("Females").toString();
            QString normal = puSummaryModel->record(k).value("Normal").toString();
            QString special = puSummaryModel->record(k).value("Special").toString();
            QString duplicates = puSummaryModel->record(k).value("Duplicates").toString();
            QString total = puSummaryModel->record(k).value("Total Registrations").toString();

            cellWidth = 750;
            QRectF sumArea(headerValueX, headerValueY, cellWidth, cellHeight);
            QRectF sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, state );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, state, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 1250;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, lga );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, lga, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 2250;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, ra );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, ra, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 3750;
            QRectF sumAreaPu(headerValueX, headerValueY, cellWidth, cellHeight);
            QRectF sumBoxPu  = painter.boundingRect(sumAreaPu, Qt::TextWordWrap, pu );
            painter.drawText(sumAreaPu, Qt::AlignLeft | Qt::AlignTop, pu, &sumBoxPu);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 750;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, delim );
            painter.drawText(sumArea, Qt::AlignLeft | Qt::AlignTop, delim, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, male );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, male, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, female );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, female , &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, normal );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, normal, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, special );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, special, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 500;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, duplicates );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, duplicates, &sumBox);
            headerValueX += cellWidth + cellSpacing;


            cellWidth = 450;
            sumArea = QRectF(headerValueX, headerValueY, cellWidth, cellHeight);
            sumBox  = painter.boundingRect(sumArea, Qt::TextWordWrap, total );
            painter.drawText(sumArea, Qt::AlignRight | Qt::AlignTop, total, &sumBox);
            headerValueX += cellWidth + cellSpacing;

            headerValueY += (headerLineSpacing);


            if (headerValueY > 9200) { // Change Page

                // If possible print table headers again.

                headerValueY = 1500;
                printer->newPage();


                painter.drawImage(400, 300, coatOfArms.scaledToWidth(700));
                painter.drawImage(13100, 300, logo.scaledToWidth(400));
                painter.setFont(footerFont);
                painter.drawText(500, 9500, "Printed by " + adminName + " (" + this->administratorId + ") on " + QDateTime::currentDateTime().toString() );
                painter.drawText(13200, 9500, "Page " + QString::number(pageNo));
                pageNo++;

                painter.setFont(titleFont);

                QRectF titleArea0(1500, 75, 11000, 500);
                QRectF titleBox0  = painter.boundingRect(titleArea0, Qt::TextWordWrap, mainTitle );
                painter.drawText(titleArea0, Qt::AlignCenter | Qt::AlignTop, mainTitle, &titleBox0);

                painter.setFont(subtitleFont);
                QRectF subtitleArea0(1500, titleBox0.height() + 50, 11000, 500);
                QRectF subtitleBox0  = painter.boundingRect(subtitleArea0, Qt::TextWordWrap, reportTitle );
                painter.drawText(subtitleArea0, Qt::AlignCenter | Qt::AlignTop, reportTitle, &subtitleBox0);

            }

        }

        printer->newPage();
    } // End of Summary

    painter.end();

#endif
}


void ReportsDialog::printPreviewCaptureReport()
{
#ifndef QT_NO_PRINTER

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setPageMargins(0, 0 ,0 , 0, QPrinter::Millimeter );
    printer.setOutputFileName("/home/sajayi/Desktop/report.pdf");

    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowFlags ( Qt::Dialog );
    preview.setWindowState(Qt::WindowMaximized);
//    preview.setWindowState(Qt::WindowFullScreen);

    connect(&preview, SIGNAL(paintRequested(QPrinter *)), SLOT(printCaptureReport(QPrinter *)));
    preview.exec();
#endif
}

/**
  * Report Generator
  *
  * Generates reports. Filters are defined in the database.
  * Sorting is done customizable but defaults to ordering by last name.
  */
void ReportsDialog::printCaptureReport(QPrinter *printer)
{
#ifndef QT_NO_PRINTER

    qDebug() << "Printing Report " << ui->reportList->currentText();

    int rowIndex = ui->reportList->currentIndex();
    QModelIndex selectedReport = reportsModel->index(rowIndex, 1);
    int reportId = reportsModel->data(selectedReport, Qt::DisplayRole).toInt();

    QSqlTableModel reportInfo;
    reportInfo.setTable("reports");

    reportInfo.setFilter(QString("id=%1").arg(reportId));
    reportInfo.select();

    QString reportSQL = reportInfo.record(0).value("filter_statement").toString();
    QString puSQL = reportInfo.record(0).value("polling_units_statement").toString();
    QString reportTitle = reportInfo.record(0).value("title").toString();
//    QString puSummarySql = reportInfo.record(0).value("pu_summary").toString();
//    QString opSummarySql = reportInfo.record(0).value("operator_summary").toString();
//    bool showSummary = reportInfo.record(0).value("show_summary").toBool();


    QSqlTableModel operatorModel;
    operatorModel.setTable("registered_operators");
    operatorModel.setFilter(QString("oid=%1").arg(this->operatorId));
    operatorModel.select();

    QString operatorName = operatorModel.record(0).value("first_name").toString() + " " + operatorModel.record(0).value("last_name").toString();


    QFont summaryTitleFont;
    summaryTitleFont.setFamily("Arial");
    summaryTitleFont.setPointSize(10);

    QFont summaryHeadFont;
    summaryHeadFont.setBold(true);
    summaryHeadFont.setFamily("Arial");
    summaryHeadFont.setPointSize(6);

    QFont summaryFont;
    summaryFont.setFamily("Arial");
    summaryFont.setPointSize(5);

    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setFamily("Arial");
    titleFont.setPointSize(14);

    QFont subtitleFont;
    subtitleFont.setBold(true);
    subtitleFont.setFamily("Arial");
    subtitleFont.setPointSize(10);

    QFont headerValueFont;
    headerValueFont.setBold(true);
    headerValueFont.setPointSize(6);

    QFont headerLabelFont;
    headerLabelFont.setPointSize(7);

    QFont labelFont;
    labelFont.setFamily("Arial");
    labelFont.setPointSize(5);

    QFont infoFont;
    infoFont.setBold(true);
    infoFont.setFamily("Arial");
    infoFont.setPointSize(7);

    QFont infoLargeFont;
    infoLargeFont.setBold(true);
    infoLargeFont.setFamily("Arial");
    infoLargeFont.setPointSize(9);

    QFont footerFont;
    footerFont.setFamily("Arial");
    footerFont.setItalic(true);
    footerFont.setPointSize(5);

    QString logoFile = "./resources/logo.png";
    QImage logo;
    logo.load(logoFile);

    QString coatOfArmsFile = "./resources/coat_of_arms.jpg";
    QImage coatOfArms;
    coatOfArms.load(coatOfArmsFile);

    QPainter painter;

    // Page 1
    if (! painter.begin(printer)) { // failed to open file
        qWarning("Failed to open file, is it writable?");
    }

    pulistModel = new QSqlQueryModel;
    pulistModel->setQuery(puSQL);

    bool printPUSummary;
    int headerLabelX = 1500;
    int headerLabelY = 900;
    int headerValueX = 3000;
    int headerValueY = 1500;
    int headerLineSpacing = 200;
    int headerColumnMargin = 3750;
    int marginTop = 2000;
    int page = 1;

    QString selectedPuId = "";

    if (ui->useSelectedPU->isChecked()) {
        if (ui->searchPUList->currentIndex() != -1) { // No PU Selected
            int puIndex = ui->searchPUList->currentIndex();

            qDebug() << "Selected Polling Unit Row: " << QString::number(puIndex);

            selectedPuId = puModel->data(puModel->index(puIndex, 1), Qt::DisplayRole).toString();
            qDebug() << "Selected Polling Unit: " << selectedPuId;
        }
    }


    // Loop thru all polling units with registered voters
    for(int j=0; j< pulistModel->rowCount(QModelIndex()); ++j) {
        printPUSummary = true;

        QString curState = pulistModel->record(j).value("state").toString();
        QString curPollingUnit = pulistModel->record(j).value("polling_unit").toString();
        QString curPUId = pulistModel->record(j).value("polling_unit_id").toString();
        QString curRA = pulistModel->record(j).value("registration_area").toString();
        QString curLGA = pulistModel->record(j).value("local_government").toString();
        QString stateCode = pulistModel->record(j).value("state_code").toString();
        QString lgaCode = pulistModel->record(j).value("lga_code").toString();
        QString raCode = pulistModel->record(j).value("ra_code").toString();
        QString puCode = pulistModel->record(j).value("pu_code").toString();

        if ( curPUId != selectedPuId && ui->useSelectedPU->isChecked() ) {
            continue;
        }

        // Sorting Records
        QSqlTableModel model;
        model.setTable("registrations");
        qDebug()  << "Sort Fields: " << reportSortFields.rowCount();

        if ( !reportSortFields.rowCount() ) {
            model.setSort( model.fieldIndex("last_name"), Qt::AscendingOrder);
            model.setSort( model.fieldIndex("first_name"), Qt::AscendingOrder);
        }

        for(int i=0; i<reportSortFields.rowCount(); ++i)
        {
            qDebug() << "Number of Sort Fields: " << reportSortFields.rowCount();
            QString field, fieldCol, fieldOrder;
            fieldCol = reportSortFields.data(reportSortFields.index(i, 0), Qt::DisplayRole).toString() ;
            fieldOrder = reportSortFields.data(reportSortFields.index(i, 1), Qt::DisplayRole).toString() ;

            if (fieldCol == "Last Name") {
                field = "last_name";
            }
            if (fieldCol ==  "First Name") {
                field = "first_name";
            }
            if (fieldCol ==  "Time Registered") {
                field = "int_created";
            }
            if (fieldCol ==  "VIN") {
                field = "vin";
            }
            if (fieldCol ==  "Age") {
                field = "dob_year";
            }
            if (fieldCol ==  "Gender") {
                field = "gender";
            }

            if (fieldOrder == "Ascending") {
                model.setSort( model.fieldIndex(field), Qt::AscendingOrder);
                qDebug() << "Sorting " << fieldCol << " with fieldname " << field << " AscendingOrder " << fieldOrder;
            } else {
                model.setSort( model.fieldIndex(field), Qt::DescendingOrder);
                qDebug() << "Sorting " << fieldCol << " with fieldname " << field << " DescendingOrder " << fieldOrder;
            }

        }


        QString votersFilter = reportSQL + " AND polling_unit_id='" + curPUId + "' ";

        // Filter by date
        if (ui->useCustomPeriod->isChecked()) {
            votersFilter += " AND int_created BETWEEN '" + ui->calStartDate->selectedDate().toString("yyyy-MM-dd 00:00:00") + "' AND '" +
            ui->calEndDate->selectedDate().toString("yyyy-MM-dd 23:59:59") + "' ";

            qDebug() << "Start: " << ui->calStartDate->selectedDate().toString("yyyy-MM-dd 00:00:00");
            qDebug() << "End: " << ui->calEndDate->selectedDate().toString("yyyy-MM-dd 23:59:59");
        }



        model.setFilter(votersFilter);
        model.select();

        //qDebug() << "SELECT STATEMENT: " << model.selectStatement();

        qDebug() << "Print Filter: " <<  votersFilter;
        qDebug() << "Number of Records to be printed" << model.rowCount();
        qDebug() << "Printing Records in the Report";

        int recHeight = 1150;
        int recWidth = 5000;
        int pos = 0;
        int serialNo = 0;
        int rowsPerPage = 10;
        int colsPerPage = 2;
        int recsPerPage = rowsPerPage*colsPerPage;
        int currentCol = 0;

        int labelX, labelY, labelSpacing;
        labelSpacing = 125;

        int valueX, valueY, valueSpacing;
        valueSpacing = 125;
        int currentRow;

        bool printHeader = true; // Print Header on First Page
        bool printFooter = true;
        qDebug() << "Number of Records in PU" << model.rowCount();
        if (model.rowCount() == 0) continue;

        for(int i=0; i<model.rowCount(); i++)
        {


            pos = i % recsPerPage; // Record on page
            currentRow = i % rowsPerPage;
            serialNo++;

            if (i % recsPerPage == 0 && i != 0) {
                printer->newPage();

                currentCol = 0;
                page++;

                printHeader = true; // New Page needs its header
            }

            if ( printPUSummary ) { // PU REPORT SUMMARY
                headerLabelX = 3000;
                headerLabelY = 2500;
                headerValueX = 4500;
                headerValueY = 2500;
                headerLineSpacing = 300;
                headerColumnMargin = 0;

                painter.drawImage(400, 300, coatOfArms.scaledToWidth(1000));
                painter.drawImage(9100, 300, logo.scaledToWidth(600));

                painter.setFont(titleFont);
                QRectF titleArea0(1500, 75, 7000, 500);
                QRectF titleBox0  = painter.boundingRect(titleArea0, Qt::TextWordWrap, mainTitle );
                painter.drawText(titleArea0, Qt::AlignCenter | Qt::AlignTop, mainTitle, &titleBox0);

                painter.setFont(subtitleFont);
                QRectF subtitleArea0(1500, titleBox0.height() + 50, 7000, 500);
                QRectF subtitleBox0  = painter.boundingRect(subtitleArea0, Qt::TextWordWrap, reportTitle );
                painter.drawText(subtitleArea0, Qt::AlignCenter | Qt::AlignTop, reportTitle, &subtitleBox0);

                painter.setFont(headerLabelFont);

                painter.drawText(headerLabelX, headerLabelY, "State:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX, headerLabelY, "Local Govt. Area:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX, headerLabelY, "Registration Area:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX, headerLabelY, "Polling Unit:");
                headerLabelY += headerLineSpacing;

                painter.setFont(headerValueFont);
                painter.drawText(headerValueX, headerValueY, stateCode + " - " + curState);
                headerValueY += headerLineSpacing;

                painter.drawText(headerValueX, headerValueY, lgaCode + " - " + curLGA);
                headerValueY += headerLineSpacing;

                painter.drawText(headerValueX, headerValueY, raCode + " - " + curRA);
                headerValueY += headerLineSpacing;

                QRectF textArea(headerValueX, headerValueY-150, 2250, 500);

                QRectF textArea3  = painter.boundingRect(textArea, Qt::TextWordWrap, puCode + " - " + curPollingUnit );
                painter.drawText(textArea, Qt::TextWordWrap, puCode + " - " + curPollingUnit,&textArea3);

                headerValueY += textArea3.height() + headerLineSpacing;
                headerLabelY += textArea3.height();

                painter.setFont(headerLabelFont);
                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Sorting:");
                headerLabelY += headerLineSpacing;
                headerLabelY += headerLineSpacing;

                painter.setFont(headerValueFont);
                if ( !reportSortFields.rowCount() ) {
                    model.setSort( model.fieldIndex("last_name"), Qt::AscendingOrder);
                    model.setSort( model.fieldIndex("first_name"), Qt::AscendingOrder);

                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "Last Name Ascending");
                    headerValueY += headerLineSpacing;
                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "First Name Ascending");
                    headerValueY += headerLineSpacing;
                }

                for(int x=0; x < reportSortFields.rowCount(); x++) {
                    qDebug() << "Number of Sort Fields: " << reportSortFields.rowCount();
                    QString fieldCol, fieldOrder;
                    fieldCol = reportSortFields.data(reportSortFields.index(x, 0), Qt::DisplayRole).toString() ;
                    fieldOrder = reportSortFields.data(reportSortFields.index(x, 1), Qt::DisplayRole).toString() ;

                    // Printing Sorting Info on Header
                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, fieldCol + " " + fieldOrder);
                    headerValueY += headerLineSpacing;

                    if (x > 0) headerLabelY += headerLineSpacing;
                }

                painter.setFont(headerLabelFont);
                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Time Printed:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Total Records:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Page:");
                headerLabelY += headerLineSpacing;

                painter.setFont(headerValueFont);
                painter.drawText(headerValueX+headerColumnMargin, headerValueY, QDateTime::currentDateTime().toString() );
                headerValueY += headerLineSpacing;

                painter.drawText(headerValueX+headerColumnMargin, headerValueY, QString::number( model.rowCount() ) );
                headerValueY += headerLineSpacing;

                painter.drawText(headerValueX+headerColumnMargin, headerValueY, QString::number( page ) );
                headerValueY += headerLineSpacing;

                if (ui->useCustomPeriod->isChecked()) {
                    painter.setFont(headerLabelFont);
                    painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Custom Date Filter:");
                    headerLabelY += headerLineSpacing;
                    headerLabelY += headerLineSpacing;

                    painter.setFont(headerValueFont);
                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "Start Date: " + ui->calStartDate->selectedDate().toString("dd.MM.yyyy") );
                    headerValueY += headerLineSpacing;

                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "End Date: " + ui->calEndDate->selectedDate().toString("dd.MM.yyyy") );
                    headerValueY += headerLineSpacing;
                }

                // Distribution Charts
                if ( ui->showCharts->isChecked() )
                {
                   // Gender Chart
                   QString genderSQL = "SELECT registrations.gender AS title, COUNT(*) AS total FROM registrations WHERE polling_unit_id='"+ curPUId + "' GROUP BY registrations.gender";
                   distributionChart("Gender Distribution", QRect(1000, 7000, 2000, 2000), genderSQL, &painter);

                   // Occupation Distribution Chart
                   QString occupationSQL = "SELECT registrations.occupation AS title, COUNT(*) AS total FROM registrations WHERE polling_unit_id='"+ curPUId + "' GROUP BY registrations.occupation";
                   distributionChart( "Occupation Distribution", QRect(5000, 7000, 2000, 2000), occupationSQL, &painter);

                   // Age Distribution Chart
                   QString ageSQL = "SELECT \"18 - 35\" AS title,  COUNT(*) AS total FROM registrations WHERE (2010-dob_year) >= 18 AND (2010 - dob_year) <= 35 AND polling_unit_id='"+ curPUId + "'  GROUP BY title  UNION                                     SELECT \"36 - 50\" AS title,  COUNT(*) AS total FROM registrations WHERE (2010-dob_year) >= 36 AND (2010 - dob_year) <= 50  AND polling_unit_id='"+ curPUId + "' GROUP BY title  UNION                                     SELECT \"51 - 65\" AS title,  COUNT(*) AS total FROM registrations WHERE (2010-dob_year) >= 51 AND (2010 - dob_year) <= 65  AND polling_unit_id='"+ curPUId + "' GROUP BY title  UNION                                     SELECT \"66 - 80\" AS title,  COUNT(*) AS total FROM registrations WHERE (2010-dob_year) >= 66 AND (2010 - dob_year) <= 80  AND polling_unit_id='"+ curPUId + "'  GROUP BY title  UNION                                     SELECT \"81 - 120\" AS title,  COUNT(*) AS total FROM registrations WHERE (2010-dob_year) >= 81 AND (2010 - dob_year) <= 120  AND polling_unit_id='"+ curPUId + "' GROUP BY title  ";
                   distributionChart("Age Distribution", QRect(1000, 10000, 2000, 2000), ageSQL, &painter);

                }

                // PAGE FOOTER
                if ( printFooter ) {
                    painter.setFont(footerFont);
                    painter.drawText(500, 13700, "Printed by " + operatorName + " (" + this->operatorId + ") on " + QDateTime::currentDateTime().toString() );
                    painter.drawText(9000, 13700, "Page " + QString::number(page));
                }

                printer->newPage();
                page++;

                printPUSummary = false;
            }


            // PAGE HEADER
            if ( printHeader ) {

                // HEADER
                headerLabelX = 1500;
                headerLabelY = 750;
                headerValueX = 2650;
                headerValueY = 750;
                headerLineSpacing = 200;
                headerColumnMargin = 3750;

                painter.drawImage(400, 300, coatOfArms.scaledToWidth(1000));
                painter.drawImage(9100, 300, logo.scaledToWidth(600));
                painter.setFont(titleFont);

                QRectF titleArea(1500, 100, 7000, 500);
                QRectF titleBox  = painter.boundingRect(titleArea, Qt::TextWordWrap, mainTitle );
                painter.drawText(titleArea, Qt::AlignCenter | Qt::AlignTop, mainTitle, &titleBox);

                painter.setFont(subtitleFont);
                QRectF subtitleArea(1500, titleBox.height() + 50, 7000, 500);
                QRectF subtitleBox  = painter.boundingRect(subtitleArea, Qt::TextWordWrap, reportTitle );
                painter.drawText(subtitleArea, Qt::AlignCenter | Qt::AlignTop, reportTitle, &subtitleBox);

                painter.setFont(headerLabelFont);

                painter.drawText(headerLabelX, headerLabelY, "State:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX, headerLabelY, "Local Govt. Area:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX, headerLabelY, "Registration Area:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX, headerLabelY, "Polling Unit:");
                headerLabelY += headerLineSpacing;

                painter.setFont(headerValueFont);
                painter.drawText(headerValueX, headerValueY, stateCode + " - " + curState);
                headerValueY += headerLineSpacing;

                painter.drawText(headerValueX, headerValueY, lgaCode + " - " + curLGA);
                headerValueY += headerLineSpacing;

                painter.drawText(headerValueX, headerValueY, raCode + " - " + curRA);
                headerValueY += headerLineSpacing;

                QRectF textArea(headerValueX, headerValueY-150, 2250, 500);

                QRectF textArea3  = painter.boundingRect(textArea, Qt::TextWordWrap, puCode + " - " + curPollingUnit );
                painter.drawText(textArea, Qt::TextWordWrap, puCode + " - " + curPollingUnit,&textArea3);

                headerValueY += headerLineSpacing;

                headerLabelY = 750;
                headerValueY = 750;
                painter.setFont(headerLabelFont);

                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Sorting:");
                headerLabelY += headerLineSpacing;
                headerLabelY += headerLineSpacing;

                painter.setFont(headerValueFont);
                if ( !reportSortFields.rowCount() )
                {
                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "Last Name Ascending");
                    headerValueY += headerLineSpacing;
                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "First Name Ascending");
                    headerValueY += headerLineSpacing;
                }

                for(int x=0; x<reportSortFields.rowCount(); x++)
                {
                    qDebug() << "Number of Sort Fields: " << reportSortFields.rowCount();
                    QString fieldCol, fieldOrder;
                    fieldCol = reportSortFields.data(reportSortFields.index(x, 0), Qt::DisplayRole).toString() ;
                    fieldOrder = reportSortFields.data(reportSortFields.index(x, 1), Qt::DisplayRole).toString() ;

                    // Printing Sorting Info on Header
                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, fieldCol + " " + fieldOrder);
                    headerValueY += headerLineSpacing;

                    if (x > 0) headerLabelY += headerLineSpacing;
                }

                painter.setFont(headerLabelFont);
                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Time Printed:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Total Records:");
                headerLabelY += headerLineSpacing;

                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Page:");
                headerLabelY += headerLineSpacing;

                painter.setFont(headerValueFont);
                painter.drawText(headerValueX+headerColumnMargin, headerValueY, QDateTime::currentDateTime().toString() );
                headerValueY += headerLineSpacing;

                painter.drawText(headerValueX+headerColumnMargin, headerValueY, QString::number( model.rowCount() ) );
                headerValueY += headerLineSpacing;

                painter.drawText(headerValueX+headerColumnMargin, headerValueY, QString::number( page ) );
                headerValueY += headerLineSpacing;

                if (ui->useCustomPeriod->isChecked()) {
                    painter.setFont(headerLabelFont);
                    painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Custom Date Filter:");
                    headerLabelY += headerLineSpacing;
                    headerLabelY += headerLineSpacing;

                    painter.setFont(headerValueFont);
                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "Start Date: " + ui->calStartDate->selectedDate().toString("dd.MM.yyyy") );
                    headerValueY += headerLineSpacing;

                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "End Date: " + ui->calEndDate->selectedDate().toString("dd.MM.yyyy") );
                    headerValueY += headerLineSpacing;
                }

                painter.drawLine ( 0 , headerValueY, 10000, headerValueY );
                marginTop = headerValueY;

                // PAGE FOOTER
                if ( printFooter ) {
                    painter.setFont(footerFont);
                    painter.drawText(500, 13700, "Printed by " + operatorName + " (" + this->operatorId + ") on " + QDateTime::currentDateTime().toString() );
                    painter.drawText(9000, 13700, "Page " + QString::number(page));
                }

                printHeader = false; // Don't Print Header until you get to the next page
            }




            if (pos % rowsPerPage == 0 && pos != 0) {
                currentCol++;
            }

            labelX = 1850+(recWidth*currentCol);
            valueX = 2500+(recWidth*currentCol);
            int sideSpacing = 1200;

            labelY = marginTop+200+(recHeight*currentRow);
            valueY = marginTop+200+(recHeight*currentRow);

            painter.setFont(labelFont);
            painter.drawText(250+(recWidth*currentCol), marginTop+200+(recHeight*currentRow), QString::number(serialNo));

            // Printing Labels
            if (ui->showTickBoxes->isChecked())
            {
                painter.setFont(labelFont);
                painter.drawText(250+(recWidth*currentCol), marginTop+200+(recHeight*currentRow), QString::number(serialNo));

                painter.drawText(labelX, labelY, "VIN");
                labelY += labelSpacing;

                painter.drawText(labelX, labelY, "NAME");
                labelY += labelSpacing;
    //            labelY += labelSpacing;



                painter.setFont(infoFont);
                painter.drawText(valueX, valueY, model.record(i).value("id").toString());
                valueY += valueSpacing;

                painter.drawText(valueX, valueY, model.record(i).value("last_name").toString() + ", " + model.record(i).value("first_name").toString()
                                 + " " + model.record(i).value("other_names").toString() );
                valueY += valueSpacing;
                valueY += valueSpacing;


            } else {

//                labelY += labelSpacing;
                painter.setFont(labelFont);
                painter.drawText(labelX, labelY, "VIN");
                labelY += labelSpacing;

                painter.setFont(infoLargeFont);
                painter.drawText(labelX, labelY, model.record(i).value("id").toString());
                labelY += labelSpacing;

                painter.setFont(labelFont);
                painter.drawText(labelX, labelY, "NAME");
                labelY += labelSpacing;

                painter.setFont(infoLargeFont);
                painter.drawText(labelX, labelY, model.record(i).value("last_name").toString() + ", " + model.record(i).value("first_name").toString()
                                 + " " + model.record(i).value("other_names").toString() );
                labelY += labelSpacing;
            }

            painter.setFont(labelFont);
            painter.drawText(labelX, labelY, "OCCUPATION");
//            labelY += labelSpacing;

            painter.drawText(labelX+sideSpacing, labelY, "GENDER");
//            labelY += labelSpacing;

            painter.drawText(labelX+(2*sideSpacing), labelY, "AGE");
//            labelY += labelSpacing;

            // Printing Values
//            valueY += valueSpacing;


            labelY += labelSpacing;
            painter.setFont(infoFont);
            painter.drawText(labelX, labelY, model.record(i).value("occupation").toString());
//            valueY += valueSpacing;

            painter.drawText(labelX+sideSpacing, labelY, model.record(i).value("gender").toString());
//            valueY += valueSpacing;

            painter.drawText(labelX+(2*sideSpacing), labelY, QString::number( 2010-model.record(i).value("dob_year").toInt() ) );


            if (!ui->showTickBoxes->isChecked())
            {
                labelY += labelSpacing;
                painter.setFont(labelFont);
                painter.drawText(labelX, labelY, "TIME REGISTERED");

                labelY += labelSpacing;
                painter.setFont(infoFont);
                painter.drawText(labelX, labelY, model.record(i).value("int_created").toDateTime().toString("MMM dd, yyyy h:mm AP"));
            }


            labelY += labelSpacing;
            labelY += 50;

            if ( ui->showTickBoxes->isChecked() )
            {
                painter.setFont(labelFont);
                QStringList elections;
                elections << "PRE";
                elections << "GOV";
                elections << "SEN";
                elections << "HR";
                elections << "HA";
                elections << "BYE";

                for (int i = 0; i < elections.size(); ++i)
                {
                    painter.drawText(labelX, labelY-50, elections[i]);
                    painter.setBackground(QBrush(Qt::white, Qt::SolidPattern));
                    painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
                    painter.drawRect(labelX, labelY, 250, 250);
                    labelX += 475;
                }
            }






            painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
            painter.drawRect(750+(recWidth*currentCol), marginTop+150+(recHeight*currentRow), 800, 900);

            // Display Passport From Database Blog
            QVariant photoData = model.record(i).value("photo_front");
            QByteArray photoBytes = photoData.toByteArray();
            QImage photoFront;
            photoFront.loadFromData(photoBytes);
            painter.drawImage( 800+(recWidth*currentCol), marginTop+200+(recHeight*currentRow), photoFront.scaled(700, 800));

            QString fingersQuery = "SELECT fingerprint_finger FROM fp_fingerprints WHERE registration_id='" + model.record(i).value("vin").toString() + "' ORDER BY fingerprint_finger ASC";
            QStringList capturedFingers;

            QSqlQueryModel *fingersModel = new QSqlQueryModel;
            fingersModel->setQuery(fingersQuery);

            for(int j=0; j< fingersModel->rowCount(QModelIndex()); ++j) {
                capturedFingers << fingersModel->record(j).value("fingerprint_finger").toString();
            }

            painter.setFont(labelFont);
            painter.drawText(650+(recWidth*currentCol),  marginTop+350+(recHeight*currentRow), "L");
            painter.drawText(1600+(recWidth*currentCol),  marginTop+350+(recHeight*currentRow), "R");

            int fpX = 1600+(recWidth*currentCol);
            int fpY = marginTop+450+(recHeight*currentRow);
            int fpW = 50;
            int fpH = 50;
            int side = 0;
            int fpSideSpacing = 950;
            int resetSide = false;

            for (int f=1; f<=10; f++) {
                bool fingerCaptured = false;
                for(int i=0; i<capturedFingers.size(); i++) {
                    if (f == capturedFingers[i].toInt()) {
                        fingerCaptured=true;
                    }
                }

                if (f > 5) {
                    side = 1;
                }

                if ( side && !resetSide) { // reset the Y when switching to the other side
                    fpY = marginTop+450+(recHeight*currentRow);
                    resetSide = true;
                }

                // Print Box
                if (fingerCaptured) {
                    painter.fillRect(fpX-(fpSideSpacing*side), fpY, fpW, fpH, QColor(0 , 0, 0));
                } else {
                    painter.drawRect(fpX-(fpSideSpacing*side), fpY, fpW, fpH);
                }

                fpY += 100;
            }

            // Horizontal Line
            painter.drawLine ( 0 +(recWidth*currentCol), marginTop+recHeight+(recHeight*currentRow), 5000+(recWidth*currentCol), marginTop+recHeight+(recHeight*currentRow) );

            // Vertical Line
            painter.drawLine ( 5000 , marginTop+0, 5000, marginTop+recHeight+(recHeight*currentRow) );
        }

        if (j != pulistModel->rowCount(QModelIndex()) - 1) {
            printer->newPage();
            page++;
        }
    }

    painter.end();

#endif
}


void ReportsDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_F2:
            ui->reportList->setFocus();
            break;
        case Qt::Key_F3:
            ui->sortFieldSelect->setFocus();
            break;
        case Qt::Key_F4:
            ui->sortOrderSelect->setFocus();
            break;
        case Qt::Key_F5:
            ui->sortAddButton->click();
            break;
        case Qt::Key_F6:
            ui->sortMoveUpButton->click();
            break;
        case Qt::Key_F7:
            ui->sortMoveDownButton->click();
            break;
        case Qt::Key_F8:
            ui->sortRemoveButton->click();
            break;
        case Qt::Key_F12:
            ui->previewCaptureReport->click();
            break;

            // Default calls the original method to handle standard keys
        default: QWidget::keyPressEvent(e);
    }
}


void ReportsDialog::addSortField() {
    qDebug() << "Adding Sort Field";
    QStandardItem *parentItem = reportSortFields.invisibleRootItem();

    QList<QStandardItem*> * row = new QList<QStandardItem*>;

    QStandardItem *item1 = new QStandardItem( ui->sortFieldSelect->currentText() );
    QStandardItem *item2 = new QStandardItem( ui->sortOrderSelect->currentText() );
    row->append(item1);
    row->append(item2);

    parentItem->appendRow(*row);
}


void ReportsDialog::removeSortField()
{
    QModelIndex sortField = ui->sortTableView->selectionModel()->currentIndex();
    reportSortFields.removeRow(sortField.row());
}

void ReportsDialog::sortMoveUp() {
    QModelIndex sortField = ui->sortTableView->selectionModel()->currentIndex();
    if (sortField.row() != 0) {
        QList<QStandardItem*> row = reportSortFields.takeRow(sortField.row());
        reportSortFields.insertRow(sortField.row() - 1, row);
    }
}

void ReportsDialog::sortMoveDown() {
    QModelIndex sortField = ui->sortTableView->selectionModel()->currentIndex();
    if (sortField.row() != reportSortFields.rowCount() - 1 ) {
        QList<QStandardItem*> row = reportSortFields.takeRow(sortField.row());
        reportSortFields.insertRow(sortField.row() + 1, row);
    }
}

void ReportsDialog::sortRowSelected(QModelIndex index) {
    qDebug() << "Updating Move Buttons";

    if (index.row() == 0) {
        ui->sortMoveUpButton->setDisabled(true);
    } else {
        ui->sortMoveUpButton->setEnabled(true);
    }

    if (index.row() == reportSortFields.rowCount() - 1) {
        ui->sortMoveDownButton->setDisabled(true);
    } else {
        ui->sortMoveDownButton->setEnabled(true);
    }
}

void ReportsDialog::distributionChart(QString title, QRect rect, QString sql, QPainter * painter)
{
    int colors[10][3] = {{0,0,255}, {0,255,0}, {255,0,0}, {255,255,0}, {255,0,255}, {0,255,255}, {0,255,128}, {128,0,255}, {0,128,0}, {0,0,128}};

    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setFamily("Verdana");
    titleFont.setPointSize(10);

    painter->setFont(titleFont);
    painter->drawText(rect.x(), rect.y() - 100, title);

    QFont legendFont;
    legendFont.setBold(false);
    legendFont.setFamily("Verdana");
    legendFont.setPointSize(8);

    painter->setFont(legendFont);

    Nightcharts infoChart;
    infoChart.setType(Nightcharts::Dpie); // {Histogramm,Pie,Dpie};
    infoChart.setLegendType(Nightcharts::Vertical); // {Round,Vertical}
    infoChart.setCords( rect.x(), rect.y(), rect.width(), rect.height() ); // x, y, w, h
//    infoChart.setCords(1000, 10000, 2000, 2000);

    QSqlQueryModel *infoModel = new QSqlQueryModel;
    infoModel->setQuery(sql);

    qDebug() << "Getting info Distribution";
    qDebug() << "Query: " << sql;
    qDebug() << "Rows in results: " << infoModel->rowCount();

    float infoTotal = 0;
    for(int k=0; k < infoModel->rowCount(); k++)
    {
         infoTotal += infoModel->record(k).value("total").toFloat();
    }

    for(int k=0; k < infoModel->rowCount(); k++)
    {
        QString info = infoModel->record(k).value("title").toString();
        qDebug() << "info: " << info;

        float infoCount = infoModel->record(k).value("total").toFloat();
        qDebug() << "Value: " << QString::number(infoCount);
        infoChart.addPiece(info, QColor(colors[k][0], colors[k][1], colors[k][2]), infoCount/infoTotal*100);
    }

    infoChart.draw(painter);
    infoChart.drawLegend(painter);
}



void ReportsDialog::showStates()
{
     qDebug() << "Searching for States with Records:";

     QString sql = "SELECT CONCAT(abbreviation, ' - ', name) AS name, id FROM states WHERE id IN (SELECT states.id FROM registrations INNER JOIN polling_units ON registrations.polling_unit_id = polling_units.id INNER JOIN registration_areas ON polling_units.registration_area_id = registration_areas.id INNER JOIN local_governments ON registration_areas.local_government_id = local_governments.id INNER JOIN states ON local_governments.state_id = states.id GROUP BY 	states.`name`)";

     stateModel = new QSqlQueryModel;
     stateModel->setQuery(sql);
     int numRows = stateModel->rowCount(QModelIndex());
     qDebug() << numRows;

     for(int i=0; i<numRows; ++i) {
        QModelIndex stateIndex = stateModel->index(i, 1);
        QString state = stateModel->data(stateIndex, Qt::DisplayRole).toString();
//        qDebug() << state;
     }

     ui->searchStateList->setModel(stateModel);
}


void ReportsDialog::showLGAinState(int rowIndex)
{
   ui->searchLGAList->clear();
   ui->searchRAList->clear();
   ui->searchPUList->clear();

    qDebug() << "Selected State Index ID: " << QString::number(rowIndex);
    QModelIndex selectedState = stateModel->index(rowIndex, 1);
    int stateId = stateModel->data(selectedState, Qt::DisplayRole).toInt();

    QString sql = QString("SELECT CONCAT(abbreviation, ' - ', name) AS name, id FROM local_governments WHERE state_id='%1' AND id IN (SELECT local_governments.id FROM registrations INNER JOIN polling_units ON registrations.polling_unit_id = polling_units.id INNER JOIN registration_areas ON polling_units.registration_area_id = registration_areas.id INNER JOIN local_governments ON registration_areas.local_government_id = local_governments.id GROUP BY local_governments.id)").arg(stateId);

    lgaModel = new QSqlQueryModel;
    lgaModel->setQuery(sql);

    ui->searchLGAList->setModel(lgaModel);

}

void ReportsDialog::showRAinLGA(int rowIndex)
{

    QModelIndex selectedLGA = lgaModel->index(rowIndex, 1);
    int lgaId = lgaModel->data(selectedLGA, Qt::DisplayRole).toInt();

    QString sql = QString("SELECT CONCAT(abbreviation, ' - ', name) AS name, id FROM registration_areas WHERE local_government_id='%1' AND id IN (SELECT registration_areas.id FROM registrations INNER JOIN polling_units ON registrations.polling_unit_id = polling_units.id INNER JOIN registration_areas ON polling_units.registration_area_id = registration_areas.id GROUP BY registration_areas.id)").arg(lgaId);

    raModel = new QSqlQueryModel;
    raModel->setQuery(sql);

    ui->searchRAList->setModel(raModel);
}

void ReportsDialog::showPUinRA(int rowIndex)
{
    ui->searchPUList->clear();

    QModelIndex selectedRA = raModel->index(rowIndex, 1);
    int raId = raModel->data(selectedRA, Qt::DisplayRole).toInt();

    QString sql = QString("SELECT CONCAT(abbreviation, ' - ', name) AS name, id FROM polling_units WHERE registration_area_id='%1' AND id IN (SELECT  polling_units.id FROM registrations INNER JOIN polling_units ON registrations.polling_unit_id = polling_units.id GROUP BY polling_units.id)").arg(raId);

    puModel = new QSqlQueryModel;
    puModel->setQuery(sql);

    ui->searchPUList->setModel(puModel);

}
