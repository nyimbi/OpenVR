#include <QMessageBox>
#include <QSqlTableModel>
#include <QTableView>
#include <QDebug>
#include <string>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QBuffer>
#include <QDateTime>

#include "recordsdialog.h"
#include "ui_recordsdialog.h"

QSqlTableModel *searchModel;
QSqlQueryModel *lgaModel;
QSqlQueryModel *raModel;
QSqlQueryModel *puModel;
QSqlQueryModel *stateModel;
QStandardItemModel sortFields;

QString searchFilter;


// Printing
int selectedRecord;
QStringList selectedRecords;  // Contains IDs of all Selected Records;

/*
     @todo Print on multiple pages <DONE>
     @todo Pagination
     @todo Search Total <DONE>
     @todo Better search for name with multiple params
     @todo Save Printed and Number of Prints
 */

RecordsDialog::RecordsDialog(QWidget *parent) : QDialog(parent),
    ui(new Ui::RecordsDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("OpenVR .:|:. Print Temporary Voter's Card");

    searchModel = new QSqlTableModel;
    searchModel->setTable("registrations");
    searchModel->sort( searchModel->fieldIndex("int_created"), Qt::DescendingOrder );
    recGridPresentation();
    searchModel->select();
    ui->recordsTableView->setModel(searchModel);

    connect(ui->searchButton, SIGNAL(clicked()), this, SLOT( searchRecords() ) );
    connect(ui->printPreviewSelected, SIGNAL(clicked()), this, SLOT( previewSelected() ) );

    connect(ui->searchStateList, SIGNAL(currentIndexChanged(int)), this, SLOT( showLGAinState(int) ) );
    connect(ui->searchLGAList, SIGNAL(currentIndexChanged(int)), this, SLOT( showRAinLGA(int)) );
    connect(ui->searchRAList, SIGNAL(currentIndexChanged(int)), this, SLOT( showPUinRA(int)) );
    showStates();

    connect(ui->sortAddButton, SIGNAL(clicked()), this, SLOT( addSortField()) );
    connect(ui->sortRemoveButton, SIGNAL(clicked()), this, SLOT( removeSortField()) );
    connect(ui->sortMoveUpButton, SIGNAL(clicked()), this, SLOT( sortMoveUp()) );
    connect(ui->sortMoveDownButton, SIGNAL(clicked()), this, SLOT( sortMoveDown()) );
    connect(ui->sortTableView, SIGNAL(pressed(QModelIndex)), this, SLOT(sortRowSelected(QModelIndex)));

//    connect(this, SIGNAL(printTVCforSelected(int)), this, SLOT( filePrint(int) ) );
    connect(this, SIGNAL(printPreviewTVCforSelected(QStringList)), this, SLOT( filePrintPreview(QStringList) ) );
    connect(this, SIGNAL(previewCaptureReport(QStringList) ), this, SLOT( printPreviewCaptureReport(QStringList) ) );


    ui->sortTableView->setModel(&sortFields);
    ui->sortTableView->setColumnWidth(0, 500);
    ui->sortTableView->setColumnWidth(1, 1050);

}

RecordsDialog::~RecordsDialog()
{
    delete ui;
}

void RecordsDialog::recGridPresentation() {

    ui->searchResultsCountLabel->setText( "Found " + QString::number(searchModel->rowCount()) + " record(s)");

    searchModel->removeColumn(searchModel->fieldIndex("asset_tag"));
    searchModel->removeColumn(searchModel->fieldIndex("identifier"));
    searchModel->removeColumn(searchModel->fieldIndex("polling_unit_id"));
    searchModel->removeColumn(searchModel->fieldIndex("operator_code"));
    searchModel->removeColumn(searchModel->fieldIndex("polling_unit_id"));
    searchModel->removeColumn(searchModel->fieldIndex("modified"));
    searchModel->removeColumn(searchModel->fieldIndex("photo_front"));
    searchModel->removeColumn(searchModel->fieldIndex("mac_address"));

    int i=0;
    QStringList fields;
    fields << "ID";
    fields << "VIN";
    fields << "Delimitation";
    fields << "Last Name";
    fields << "First Name";
    fields << "Other Names";
    fields << "Day of Birth";
    fields << "Month of Birth";
    fields << "Year of Birth";
    fields << "Mobile Number";
    fields << "Gender";
    fields << "Home Address";
    fields << "Occupation";
    fields << "Time Registered";


    for(i=0; i<fields.size(); i++) {
        searchModel->setHeaderData(i, Qt::Horizontal, QObject::tr(fields[i].toAscii().constData() ) );
    }

    ui->recordsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->recordsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->recordsTableView->setSelectionMode(QAbstractItemView::MultiSelection);
    ui->recordsTableView->setSortingEnabled(false);
}

void RecordsDialog::searchRecords() {

    searchFilter = getSearchFilter();
    recGridPresentation();
    searchModel->setTable("registrations");
    recGridPresentation();

    if (!sortFields.rowCount())
        searchModel->sort( searchModel->fieldIndex("int_created"), Qt::DescendingOrder );


    for(int i=0; i<sortFields.rowCount();++i) {

        qDebug() << "Number of Sort Fields: " << sortFields.rowCount();
        QString field, fieldCol, fieldOrder;
        fieldCol = sortFields.data(sortFields.index(i, 0), Qt::DisplayRole).toString() ;
        fieldOrder = sortFields.data(sortFields.index(i, 1), Qt::DisplayRole).toString() ;
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
            searchModel->setSort( searchModel->fieldIndex(field), Qt::AscendingOrder);
            qDebug() << "Sorting " << fieldCol << " with fieldname " << field << " AscendingOrder " << fieldOrder;
        } else {
            searchModel->setSort( searchModel->fieldIndex(field), Qt::DescendingOrder);
            qDebug() << "Sorting " << fieldCol << " with fieldname " << field << " DescendingOrder " << fieldOrder;
        }

    }

    searchModel->setFilter(searchFilter);
    searchModel->select();

    while(searchModel->canFetchMore())
        searchModel->fetchMore();

    qDebug() << "Total Records Found: " << searchModel->rowCount();


    ui->recordsTableView->show();
}

QString RecordsDialog::getSearchFilter() {
    QString filter;
    bool prevFilter = false;

    QString search = ui->searchNameEdit->text();
    QRegExp sr("\\s");

    QStringList searchParams = search.split(sr);

    QStringList nameSearchFields;
    nameSearchFields <<  "first_name" << "last_name" << "other_names";

    // filter.split() QString::split()
    qDebug() << searchParams.size();
    if (searchParams.size() > 1) {
        filter = "(";

        int i, j;
        for(i=0; i < nameSearchFields.size(); i++) {
            filter += "(";
            for(j=0; j < searchParams.size(); j++) {
                filter += nameSearchFields[i] +" LIKE '%" + searchParams[j] + "%'";

                if (j != searchParams.size()-1) {
                    filter += " OR ";
                }
            }
            filter += ")";
            if (i != nameSearchFields.size()-1) {
                filter += " OR ";
            }
        }

        filter += ")";

        prevFilter = true;

    } else {
        if( search != "") {
            filter = "(first_name LIKE '%" + ui->searchNameEdit->text() + "%'";
            filter += "OR last_name LIKE '%" + ui->searchNameEdit->text() + "%'";
            filter += "OR other_names LIKE '%" + ui->searchNameEdit->text() + "%')";
            prevFilter = true;
        }
    }

    if ( ui->searchGender->currentText() != "" ) {
        if (prevFilter) filter += " AND ";
        filter += " gender='" + ui->searchGender->currentText() + "'";
        prevFilter = true;
    }

    if ( ui->searchAddressEdit->text() != "" ) {
        if (prevFilter) filter += " AND ";
        filter += " home_address LIKE '%" + ui->searchAddressEdit->text() + "%'";
        prevFilter = true;
    }


    if (ui->searchPUList->currentIndex() != -1) { // No PU Selected
        int puIndex = ui->searchPUList->currentIndex();

        qDebug() << "Selected Polling Unit Row: " << QString::number(puIndex);



        int puId = puModel->data(puModel->index(puIndex, 1), Qt::DisplayRole).toInt();
        if (prevFilter) filter += " AND ";
        filter += " polling_unit_id='"+QString::number(puId)+"'";
        prevFilter = true;

    }

    qDebug() << "Search Filter: " << filter;
    return filter;
}

void RecordsDialog::on_printSelectedButton_clicked()
{
    QModelIndex selected = ui->recordsTableView->selectionModel()->currentIndex();
    QString selectedId = searchModel->data(searchModel->index(selected.row(), searchModel->fieldIndex("id")),0).toString();
    qDebug() << "Selected ID:" <<  searchModel->data(searchModel->index(selected.row(), searchModel->fieldIndex("id")),0).toString() <<  " Belonging to "  <<  searchModel->data(searchModel->index(selected.row(), 2),0).toString();

    if (selectedId != "") {
        emit printTVCforSelected(selectedId.toInt());
    } else {
        QMessageBox::critical(this, "Error :: No Record Selected", "Please select a record and try again.", QMessageBox::Ok);
    }
}



void RecordsDialog::previewSelected()
{
    qDebug() << "Previewing TVC:";
    QModelIndexList selectedRecords = ui->recordsTableView->selectionModel()->selectedRows();

    QStringList selectedIds;

    for(int i=0; i<selectedRecords.size(); i++) {
        selectedIds << searchModel->data(searchModel->index(selectedRecords[i].row(), searchModel->fieldIndex("id")),0).toString();
        qDebug() << "Selected ID:" <<  searchModel->data(searchModel->index(selectedRecords[i].row(), searchModel->fieldIndex("id")),0).toString() <<
                " Belonging to "  <<  searchModel->data(searchModel->index(selectedRecords[i].row(), searchModel->fieldIndex("first_name")),0).toString();
    }

    if (selectedIds.size() > 0) {
        emit printPreviewTVCforSelected(selectedIds);
    } else {
        QMessageBox::critical(this, "Error :: No Record Selected", "Please select a record and try again.", QMessageBox::Ok);
    }
}



void RecordsDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_F1:
        ui->searchNameEdit->setFocus();
        break;
    case Qt::Key_F2:
        ui->searchAddressEdit->setFocus();
        break;
    case Qt::Key_F3:
        ui->searchGender->setFocus();
        break;
    case Qt::Key_F8:
        ui->searchButton->click();
        ui->recordsTableView->setFocus();
        break;
    case Qt::Key_F9:
        ui->recordsTableView->setFocus();
        break;
    case Qt::Key_F10:
        this->reject();
        break;
    case Qt::Key_F11:
        ui->printPreviewSelected->click();
        break;

        // Default calls the original method to handle standard keys
    default: QWidget::keyPressEvent(e);
    }
}

void RecordsDialog::showStates()
{
     qDebug() << "Searching for States with Records:";

     QString sql = "SELECT name, id FROM states WHERE id IN (SELECT states.id FROM registrations INNER JOIN polling_units ON registrations.polling_unit_id = polling_units.id INNER JOIN registration_areas ON polling_units.registration_area_id = registration_areas.id INNER JOIN local_governments ON registration_areas.local_government_id = local_governments.id INNER JOIN states ON local_governments.state_id = states.id GROUP BY 	states.`name`)";

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


void RecordsDialog::showLGAinState(int rowIndex)
{
   ui->searchLGAList->clear();
   ui->searchRAList->clear();
   ui->searchPUList->clear();

    qDebug() << "Selected State Index ID: " << QString::number(rowIndex);
    QModelIndex selectedState = stateModel->index(rowIndex, 1);
    int stateId = stateModel->data(selectedState, Qt::DisplayRole).toInt();

    QString sql = QString("SELECT name, id FROM local_governments WHERE state_id='%1' AND id IN (SELECT local_governments.id FROM registrations INNER JOIN polling_units ON registrations.polling_unit_id = polling_units.id INNER JOIN registration_areas ON polling_units.registration_area_id = registration_areas.id INNER JOIN local_governments ON registration_areas.local_government_id = local_governments.id GROUP BY local_governments.id)").arg(stateId);

    lgaModel = new QSqlQueryModel;
    lgaModel->setQuery(sql);

    ui->searchLGAList->setModel(lgaModel);

}

void RecordsDialog::showRAinLGA(int rowIndex)
{

    QModelIndex selectedLGA = lgaModel->index(rowIndex, 1);
    int lgaId = lgaModel->data(selectedLGA, Qt::DisplayRole).toInt();

    QString sql = QString("SELECT name, id FROM registration_areas WHERE local_government_id='%1' AND id IN (SELECT registration_areas.id FROM registrations INNER JOIN polling_units ON registrations.polling_unit_id = polling_units.id INNER JOIN registration_areas ON polling_units.registration_area_id = registration_areas.id GROUP BY registration_areas.id)").arg(lgaId);

    raModel = new QSqlQueryModel;
    raModel->setQuery(sql);

    ui->searchRAList->setModel(raModel);
}

void RecordsDialog::showPUinRA(int rowIndex)
{
    ui->searchPUList->clear();

    QModelIndex selectedRA = raModel->index(rowIndex, 1);
    int raId = raModel->data(selectedRA, Qt::DisplayRole).toInt();

    QString sql = QString("SELECT name, id FROM polling_units WHERE registration_area_id='%1' AND id IN (SELECT  polling_units.id FROM registrations INNER JOIN polling_units ON registrations.polling_unit_id = polling_units.id GROUP BY polling_units.id)").arg(raId);

    puModel = new QSqlQueryModel;
    puModel->setQuery(sql);

    ui->searchPUList->setModel(puModel);

}

void RecordsDialog::addSortField() {
    QStandardItem *parentItem = sortFields.invisibleRootItem();

    QList<QStandardItem*> * row = new QList<QStandardItem*>;

    QStandardItem *item1 = new QStandardItem( ui->sortFieldSelect->currentText() );
    QStandardItem *item2 = new QStandardItem( ui->sortOrderSelect->currentText() );
    row->append(item1);
    row->append(item2);

    parentItem->appendRow(*row);
}


void RecordsDialog::removeSortField()
{
    QModelIndex sortField = ui->sortTableView->selectionModel()->currentIndex();
    sortFields.removeRow(sortField.row());
}

void RecordsDialog::sortMoveUp() {
    QModelIndex sortField = ui->sortTableView->selectionModel()->currentIndex();
    if (sortField.row() != 0) {
        QList<QStandardItem*> row = sortFields.takeRow(sortField.row());
        sortFields.insertRow(sortField.row() - 1, row);
    }
}

void RecordsDialog::sortMoveDown() {
    QModelIndex sortField = ui->sortTableView->selectionModel()->currentIndex();
    if (sortField.row() != sortFields.rowCount() - 1 ) {
        QList<QStandardItem*> row = sortFields.takeRow(sortField.row());
        sortFields.insertRow(sortField.row() + 1, row);
    }
}

void RecordsDialog::sortRowSelected(QModelIndex index) {
    qDebug() << "Updating Move Buttons";

    if (index.row() == 0) {
        ui->sortMoveUpButton->setDisabled(true);
    } else {
        ui->sortMoveUpButton->setEnabled(true);
    }

    if (index.row() == sortFields.rowCount() - 1) {
        ui->sortMoveDownButton->setDisabled(true);
    } else {
        ui->sortMoveDownButton->setEnabled(true);
    }
}



void RecordsDialog::filePrint(QString id)
{
#ifndef QT_NO_PRINTER
    QStringList vins;
    vins << id;
    selectedRecords = vins;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setPageMargins(0, 0 ,0 , 0, QPrinter::Millimeter );

    QPrintDialog *dlg = new QPrintDialog(&printer, this);

    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted) {
//        textEdit->print(&printer);
        printTVC(&printer);
    }
    delete dlg;
#endif
}

void RecordsDialog::filePrintPreview(int id)
{
#ifndef QT_NO_PRINTER
    selectedRecord = id;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setPageMargins(0, 0 ,0 , 0, QPrinter::Millimeter );

    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowFlags ( Qt::Dialog );
    preview.setWindowState(Qt::WindowMaximized);

    connect(&preview, SIGNAL(paintRequested(QPrinter *)), SLOT(printTVC(QPrinter *)));
    preview.exec();
#endif
}


void RecordsDialog::filePrintPreview(QStringList ids)
{
#ifndef QT_NO_PRINTER
    selectedRecords = ids;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setPageMargins(0, 0 ,0 , 0, QPrinter::Millimeter );

    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowFlags ( Qt::Dialog );
    preview.setWindowState(Qt::WindowMaximized);

    connect(&preview, SIGNAL(paintRequested(QPrinter *)), SLOT(printTVC(QPrinter *)));
    preview.exec();
#endif
}





void RecordsDialog::printTVC(QPrinter *printer)
{
#ifndef QT_NO_PRINTER
    qDebug() << "Calling printTVC(QPrinter)";
    QPainter painter;

    // Page 1
    if (! painter.begin(printer)) { // failed to open file
        qWarning("Failed to open file, is it writable?");
    }

    QString bcData; // Barcode Data
    int regNo;

    QFont sampleFont;
    sampleFont.setFamily("Lucida Sans");
    sampleFont.setBold(true);
    sampleFont.setPointSize(36);

    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setFamily("Lucida Sans");
    titleFont.setPointSize(9);

    QFont labelFont;
    labelFont.setFamily("Lucida Sans");
    labelFont.setPointSize(7);

    QFont actFont;
    actFont.setFamily("Lucida Sans");
    actFont.setPointSize(6);

    QFont infoFont;
    infoFont.setFamily("Lucida Sans");
    infoFont.setBold(true);
    infoFont.setPointSize(7);

    QFont vinFont;
    vinFont.setFamily("Lucida Sans");
    vinFont.setBold(true);
    vinFont.setPointSize(9);

    QString bgFile = "./resources/bg.jpg";
    QImage bg;
    bg.load(bgFile);

    QString logoFile = "./resources/logo.png";
    QImage logo;
    logo.load(logoFile);

//    QString titleFile = "./resources/title.png";
//    QImage titleImg;
//    titleImg.load(titleFile);

//    int recIndex=regNo-1;
    int tvcLabelX, tvcLabelY, tvcLabelSpacing;
    tvcLabelSpacing = 175;
    tvcLabelX = 2200; // Left Padding of Label

    int tvcValueX, tvcValueY, tvcValueSpacing;
    tvcValueSpacing = 175;
    tvcValueX = 3300; // Left Padding of Value

    QString rowSeparator = "\n";

//    tvcValueSpacing = ui->printWidth->text().toInt();
//    tvcValueX = ui->printX->text().toInt(); // Left Padding of Value
//    tvcValueY = ui->printY->text().toInt(); // Initial Value Y

//    tvcLabelSpacing = ui->printWidth->text().toInt();
//    tvcLabelX = 1700; // Left Padding of Label
//    tvcLabelY = ui->printY->text().toInt(); // Initial Label Y

    int tvcHeight = 2800;
//    int tvcWidth = 10000;


        painter.setFont(infoFont);
//        showGuidelines(&painter);

    QString glue, filter;
    filter = "";
    glue = " OR ";
    for(int i=0; i < selectedRecords.size(); i++) {
        filter += "id='"+ selectedRecords[i];
        filter += "'";
        if (i != selectedRecords.size()-1 ) {
             filter+=glue;
        }
    }

    QSqlTableModel model;
    model.setTable("registrations");
    model.setFilter(filter);
    model.select();

    qDebug() << "Print Filter: " << filter;

    int recIndex;

    int pos;


    int maxBarcodeDataLen = 1192;
    int remainingDataLen;

    for(int i=0; i<selectedRecords.size(); i++) {

        painter.setPen(QColor(0, 0, 0));

        remainingDataLen = maxBarcodeDataLen;
        pos = i % 5;

        if (i % 5 == 0 && i != 0) {
            printer->newPage();
            painter.drawLine ( 550 , 0, 550, 14000 );
            painter.drawLine ( 9450 , 0, 9450, 14000 );
        }

        tvcLabelY = 850+(tvcHeight*pos);
        tvcValueY = 850+(tvcHeight*pos);

        regNo=selectedRecords[i].toInt();
        recIndex = i;


        qDebug() << "Printing for ID: " << regNo;

        bcData = "";

        painter.setFont(titleFont);
        painter.drawText(750, 400+(tvcHeight*pos), "INDEPENDENT NATIONAL ELECTORAL COMMISSION");
        painter.drawText(2250, 600+(tvcHeight*pos), "VOTER'S CARD");
        painter.drawImage(8250, 300+(tvcHeight*pos), logo.scaledToWidth(1000));

//        painter.drawImage(350, 100+(tvcHeight*pos), titleImg.scaledToWidth(4500));

    // TVC Labels
        painter.setFont(labelFont);

        painter.drawText(tvcLabelX, tvcLabelY, "Surname:");
        tvcLabelY += tvcLabelSpacing;

        painter.drawText(tvcLabelX, tvcLabelY, "First Name:");
        tvcLabelY += tvcLabelSpacing;

        painter.drawText(tvcLabelX, tvcLabelY, "Other Names:");
        tvcLabelY += tvcLabelSpacing;

        painter.drawText(tvcLabelX, tvcLabelY, "Date of Birth:");
        tvcLabelY += tvcLabelSpacing;

        painter.drawText(tvcLabelX, tvcLabelY, "Occupation:");
        tvcLabelY += tvcLabelSpacing;

        painter.drawText(tvcLabelX, tvcLabelY, "Gender: " +
             model.record(recIndex).value("gender").toString() +
             "      Age: " + QString::number( 2010-model.record(recIndex).value("dob_year").toInt() )
        );
        tvcLabelY += tvcLabelSpacing;

//        painter.drawText(tvcLabelX, tvcLabelY, "Age:");
//        tvcLabelY += tvcLabelSpacing;

        painter.drawText(tvcLabelX, tvcLabelY, "Address:");
        tvcLabelY += tvcLabelSpacing;

//        painter.drawText(tvcLabelX, tvcLabelY, "VIN:");
//        tvcLabelY += tvcLabelSpacing;

        painter.drawText(tvcLabelX, tvcLabelY, "Delimitation:");
        tvcLabelY += tvcLabelSpacing;

        painter.drawText(tvcLabelX, tvcLabelY, "Date of Issue:");
        tvcLabelY += tvcLabelSpacing;


        painter.setFont(infoFont);

        painter.drawText(tvcValueX, tvcValueY, model.record(recIndex).value("last_name").toString());
        tvcValueY += tvcValueSpacing;


        painter.drawText(tvcValueX, tvcValueY, model.record(recIndex).value("first_name").toString());
        tvcValueY += tvcValueSpacing;

        painter.drawText(tvcValueX, tvcValueY,  model.record(recIndex).value("other_names").toString() );
        tvcValueY += tvcValueSpacing;

        painter.drawText(tvcValueX, tvcValueY, model.record(recIndex).value("dob_day").toString()
                                                 + " - " + model.record(recIndex).value("dob_month").toString()
                                                 + " - " + model.record(recIndex).value("dob_year").toString() );
        tvcValueY += tvcValueSpacing;

        painter.drawText(tvcValueX, tvcValueY, model.record(recIndex).value("occupation").toString());
        tvcValueY += tvcValueSpacing;

//        painter.drawText(tvcValueX, tvcValueY, model.record(recIndex).value("gender").toString());
//        tvcValueY += tvcValueSpacing;

//        painter.drawText(tvcValueX, tvcValueY, QString::number( 2010-model.record(recIndex).value("dob_year").toInt() ) );
//        tvcValueY += tvcValueSpacing;

        tvcValueY += tvcValueSpacing;  // Move one more step for Gender's sake

        painter.drawText(tvcValueX, tvcValueY, model.record(recIndex).value("home_address").toString());
        tvcValueY += tvcValueSpacing;


        painter.drawText(tvcValueX, tvcValueY, model.record(recIndex).value("delimitation").toString());
        tvcValueY += tvcValueSpacing;

        QDateTime timeCreated =  model.record(recIndex).value("int_created").toDateTime();
        painter.drawText(tvcValueX, tvcValueY, timeCreated.toString("MMM dd, yyyy h:mm AP"));
        tvcValueY += tvcValueSpacing;



        painter.setFont(vinFont);
        painter.drawText(800,  2500+(tvcHeight*pos), model.record(recIndex).value("vin").toString());


        painter.setFont(infoFont);
        painter.drawText( 5250, 2000+(tvcHeight*pos) , model.record(recIndex).value("last_name").toString().toUpper() + ":" + model.record(recIndex).value("vin").toString());

        painter.setFont(actFont);
        painter.drawText( 5250, 2200+(tvcHeight*pos) , "ISSUED BY INEC PURSUANT TO SECTION 16 OF THE ELECTORAL ACT 2010");

        bcData += "Surname,";
        bcData += model.record(recIndex).value("last_name").toString();
        bcData += rowSeparator;

        bcData += "First Name,";
        bcData += model.record(recIndex).value("first_name").toString();
        bcData += rowSeparator;

        bcData += "Other Names,";
        bcData += model.record(recIndex).value("other_names").toString();
        bcData += rowSeparator;

        bcData += "Date of Birth,";
        bcData += model.record(recIndex).value("dob_day").toString()
                                                    + " - " + model.record(recIndex).value("dob_month").toString()
                                                    + " - " + model.record(recIndex).value("dob_year").toString();
        bcData += rowSeparator;

        bcData += "Occupation,";
        bcData += model.record(recIndex).value("occupation").toString();
        bcData += rowSeparator;

        bcData += "Gender,";
        bcData += model.record(recIndex).value("gender").toString();
        bcData += rowSeparator;

        bcData += "Age,";
        bcData += 2010 - model.record(recIndex).value("dob_year").toInt();
        bcData += rowSeparator;

        bcData += "Date of Issue,";
        bcData += model.record(recIndex).value("int_created").toString();
        bcData += rowSeparator;

        bcData += "VIN,";
        bcData += model.record(recIndex).value("vin").toString();
        bcData += rowSeparator;

        bcData += "Delimitation,";
        bcData += model.record(recIndex).value("delimitation").toString();

//        remainingDataLen -= bcData.length();


       // Display Passport From Database Blog
        QVariant photoData = model.record(recIndex).value("photo_front");
        QByteArray photoBytes = photoData.toByteArray();
        QImage photoFront;
        photoFront.loadFromData(photoBytes);

        painter.drawImage( 800, 800+(tvcHeight*pos), photoFront.scaled(1200, 1300));
        painter.drawRect(750, 750+(tvcHeight*pos), 1300, 1400 );

        painter.drawLine ( 0 , 2800+(tvcHeight*pos), 10000, 2800+(tvcHeight*pos) );
        painter.drawLine ( 5000 , 0, 5000, 2800+(tvcHeight*pos) );


        int size = 25;
        QPixmap pf = QPixmap::fromImage(photoFront);
        pf = pf.scaled(QSize(size, size), Qt::KeepAspectRatio, Qt::FastTransformation);
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        pf.save(&buffer, "JPG");

        QByteArray securityImg = bytes;
        securityImg = qCompress(securityImg);

        qDebug() << "=============================================================";
        qDebug() << "Barcode Space Left: " << QString::number( remainingDataLen - bcData.length() );

        // Auto Adjust Image Quality to Suit Barcode
        while(securityImg.toBase64().length() > remainingDataLen) {

            size -= 1;
            qDebug() << "Auto-Resizing Image to Fit in Barcode";
            qDebug() << "Current Size: " << size;
            pf = pf.scaled(QSize(size, size), Qt::KeepAspectRatio, Qt::FastTransformation);
            QByteArray bytes2;
            QBuffer buffer2(&bytes2);
            buffer2.open(QIODevice::WriteOnly);
            pf.save(&buffer2, "JPG");

            securityImg = bytes2;
            securityImg = qCompress(securityImg);
            qDebug() << "Image Length: " << securityImg.toBase64().length();

            if (size <= 2) {
                break;
            }
        }

        bcData += "Front Photo," + QString(securityImg.toBase64());

        remainingDataLen -= bcData.length();
        qDebug() << "Barcode Space Left: " << QString::number(remainingDataLen);

        m_fgcolor=qRgb(0,0,0);
        m_bgcolor=qRgb(0xff,0xff,0xff);

        // Compress Barcode Data
//        QByteArray bcComp = qCompress(bcData.toUtf8());
//        bcData = QString(bcComp);

        m_bc.bc.setText(bcData);
        qDebug() << "Barcode Data Length: " << bcData.length();

        m_bc.ar=(Zint::QZint::AspectRatioMode)0;
        m_bc.bc.setSecurityLevel(0);
        m_bc.bc.setWidth(3200);
        m_bc.bc.setInputMode(UNICODE_MODE);
        m_bc.bc.setHideText(TRUE);
        m_bc.bc.setSymbol(BARCODE_PDF417);
        m_bc.bc.setPdf417CodeWords(928);

        m_bc.bc.setHeight(1000);
        m_bc.bc.setWhitespace(0);
        m_bc.bc.setFgColor(m_fgcolor);
        m_bc.bc.setBgColor(m_bgcolor);

        m_bc.bc.render(painter, QRectF(5250, 300+(tvcHeight*pos), 2800, 1500), m_bc.ar);


        // Sample
        painter.setFont(sampleFont);
        painter.setPen(QColor(160, 160, 160));
        painter.drawText(1500, 1500+(tvcHeight*pos), "SAMPLE");
        painter.drawText(6500, 1500+(tvcHeight*pos), "SAMPLE");


        // Guidelines for cutting
        painter.drawLine ( 550 , 0, 550, tvcHeight+ (tvcHeight*pos));
        painter.drawLine ( 9450 , 0, 9450, tvcHeight+ (tvcHeight*pos) );

    }


    painter.end();



#endif
}

