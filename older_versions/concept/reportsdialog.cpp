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

ReportsDialog::ReportsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ReportsDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("OpenVR .:. Reports Generator");

    connect(ui->previewCaptureReport, SIGNAL(clicked()), this, SLOT( printPreviewCaptureReport() ) );

    connect(ui->sortAddButton, SIGNAL(clicked()), this, SLOT( addSortField()) );
    connect(ui->sortRemoveButton, SIGNAL(clicked()), this, SLOT( removeSortField()) );
    connect(ui->sortMoveUpButton, SIGNAL(clicked()), this, SLOT( sortMoveUp()) );
    connect(ui->sortMoveDownButton, SIGNAL(clicked()), this, SLOT( sortMoveDown()) );
    connect(ui->sortTableView, SIGNAL(pressed(QModelIndex)), this, SLOT(sortRowSelected(QModelIndex)));

    ui->sortTableView->setModel(&reportSortFields);
    showReports();
}

ReportsDialog::~ReportsDialog()
{
    delete ui;
}

void ReportsDialog::showReports() {
    QString sql = "SELECT title, id FROM reports";

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


void ReportsDialog::printPreviewCaptureReport()
{
#ifndef QT_NO_PRINTER

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setPageMargins(0, 0 ,0 , 0, QPrinter::Millimeter );

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

    QString mainTitle = "FEDERAL REPUBLIC OF NIGERIA";

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


    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setFamily("Lucida Sans");
    titleFont.setPointSize(14);

    QFont subtitleFont;
    subtitleFont.setBold(true);
    subtitleFont.setFamily("Lucida Sans");
    subtitleFont.setPointSize(12);

    QFont headerValueFont;
    headerValueFont.setBold(true);
    headerValueFont.setPointSize(8);

    QFont headerLabelFont;
    headerLabelFont.setPointSize(8);

    QFont labelFont;
    labelFont.setFamily("Arial");
    labelFont.setPointSize(8);

    QFont infoFont;
    infoFont.setBold(true);
    infoFont.setFamily("Arial");
    infoFont.setPointSize(8);

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
    int headerValueY = 900;
    int headerLineSpacing = 200;
    int headerColumnMargin = 3750;
    int marginTop = 2000;
    int page = 1;

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


        // Sorting Records
        QSqlTableModel model;
        model.setTable("registrations");

        if ( !reportSortFields.rowCount() ) {
            model.setSort( model.fieldIndex("last_name"), Qt::AscendingOrder);
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

        model.setFilter(reportSQL + " AND polling_unit_id='" + curPUId + "' ");
        model.select();

        qDebug() << "Print Filter: " <<  reportSQL + " AND polling_unit_id='" + curPUId + "' ";
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
        labelSpacing = 150;

        int valueX, valueY, valueSpacing;
        valueSpacing = 150;
        int currentRow;

        bool printHeader = true; // Print Header on First Page
        qDebug() << "Number of Records in PU" << model.rowCount();

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

                QRectF titleArea0(1500, 100, 7000, 500);
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

                painter.setFont(headerValueFont);
                if ( !reportSortFields.rowCount() ) {
                    model.setSort( model.fieldIndex("last_name"), Qt::AscendingOrder);

                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "Last Name Ascending");
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

                printer->newPage();
                page++;

                printPUSummary = false;
            }

            // PAGE HEADER
            if ( printHeader ) {

                // HEADER
                headerLabelX = 1500;
                headerLabelY = 900;
                headerValueX = 3000;
                headerValueY = 900;
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

                headerLabelY = 900;
                headerValueY = 900;
                painter.setFont(headerLabelFont);

                painter.drawText(headerLabelX+headerColumnMargin, headerLabelY, "Sorting:");
                headerLabelY += headerLineSpacing;

                painter.setFont(headerValueFont);
                if ( !reportSortFields.rowCount() )
                {
                    painter.drawText(headerValueX+headerColumnMargin, headerValueY, "Last Name Ascending");
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

                painter.drawLine ( 0 , headerValueY, 10000, headerValueY );
                marginTop = headerValueY;
                printHeader = false; // Don't Print Header until you get to the next page
            }

            if (pos % rowsPerPage == 0 && pos != 0) {
                currentCol++;
            }

            labelX = 1650+(recWidth*currentCol);
            valueX = 2500+(recWidth*currentCol);

            labelY = marginTop+350+(recHeight*currentRow);
            valueY = marginTop+350+(recHeight*currentRow);

            // Printing Labels
            painter.setFont(labelFont);
            painter.drawText(300+(recWidth*currentCol), marginTop+500+(recHeight*currentRow), QString::number(serialNo));

            painter.drawText(labelX, labelY, "VIN:");
            labelY += labelSpacing;

            painter.drawText(labelX, labelY, "Name:");
            labelY += labelSpacing;

            painter.drawText(labelX, labelY, "Occupation:");
            labelY += labelSpacing;

            painter.drawText(labelX, labelY, "Gender:");
            labelY += labelSpacing;

            painter.drawText(labelX, labelY, "Age:");
            labelY += labelSpacing;

            // Printing Values
            painter.setFont(infoFont);
            painter.drawText(valueX, valueY, model.record(i).value("vin").toString());
            valueY += valueSpacing;

            painter.drawText(valueX, valueY, model.record(i).value("last_name").toString() + " " + model.record(i).value("first_name").toString()
                             + ", " + model.record(i).value("other_names").toString() );
            valueY += valueSpacing;

            painter.drawText(valueX, valueY, model.record(i).value("occupation").toString());
            valueY += valueSpacing;

            painter.drawText(valueX, valueY, model.record(i).value("gender").toString());
            valueY += valueSpacing;

            painter.drawText(valueX, valueY, QString::number( 2010-model.record(i).value("dob_year").toInt() ) );
            valueY += valueSpacing;

            painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
            painter.drawRect(750+(recWidth*currentCol), marginTop+150+(recHeight*currentRow), 800, 900);

            // Display Passport From Database Blog
            QVariant photoData = model.record(i).value("photo_front");
            QByteArray photoBytes = photoData.toByteArray();
            QImage photoFront;
            photoFront.loadFromData(photoBytes);
            painter.drawImage( 800+(recWidth*currentCol), marginTop+200+(recHeight*currentRow), photoFront.scaled(700, 800));

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
