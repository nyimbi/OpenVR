#include "queuetickets.h"
#include "ui_queuetickets.h"

bool settingExists;

QueueTickets::QueueTickets(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueueTickets)
{
    ui->setupUi(this);

    connect(ui->btnPrint, SIGNAL(clicked()), this, SLOT(ticketPrintPreview()));
    connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->spinColumns, SIGNAL(valueChanged(int)), this, SLOT(updateTotalTickets()));
    connect(ui->spinRows, SIGNAL(valueChanged(int)), this, SLOT(updateTotalTickets()));
    connect(ui->spinNumPages, SIGNAL(valueChanged(int)), this, SLOT(updateTotalTickets()));
    connect(ui->cbEstTime, SIGNAL(clicked(bool)), this, SLOT(updateTotalTickets()));
    connect(ui->startTime, SIGNAL(timeChanged(QTime)), this, SLOT(updateTotalTickets()));

    ui->startTime->setTime(QTime::currentTime());
}

QueueTickets::~QueueTickets()
{
    delete ui;
}

void QueueTickets::printTickets(QPrinter *printer) {
#ifndef QT_NO_PRINTER

    qDebug() << "Printing Queue Tickets";
    QPainter painter;

    // Page 1
    if (! painter.begin(printer)) { // failed to open file
        qWarning("Failed to open file, is it writable?");
    }

    QFont titleFont;
    titleFont.setFamily("Lucida Sans");
    titleFont.setBold(true);
    titleFont.setPointSize(12);

    QFont numFont;
    numFont.setFamily("Lucida Sans");
    numFont.setPointSize(36);

    QFont timeFont;
    timeFont.setFamily("Lucida Sans");
    timeFont.setPointSize(14);

    QFont noFont;
    noFont.setFamily("Lucida Sans");
    noFont.setPointSize(14);

    QFont labelFont;
    labelFont.setFamily("Lucida Sans");
    labelFont.setPointSize(6);

    QSqlQuery dbSettingQuery;
    QString queryString = "SELECT setting_value FROM settings WHERE setting_name='last_ticket_no'";

    qDebug() << "Getting Last Ticket No : " << queryString;
    dbSettingQuery.exec(queryString);

    int ticketNo;


    if(!dbSettingQuery.size()){
        ticketNo = 1;
        settingExists = false;
    } else {
        dbSettingQuery.next();
        ticketNo = dbSettingQuery.value(0).toInt();
        settingExists = true;
    }

    QSqlQuery dbQuery;
    QString queryPUString = "SELECT setting_value FROM settings WHERE setting_name='polling_unit_id'";
    qDebug() << "Getting Current Polling Unit : " << queryPUString;
    dbQuery.exec(queryPUString);
    if(!dbQuery.size()){
        return;
    }
    dbQuery.next();
    int curPollingUnitId = dbQuery.value(0).toInt();

    QString puSQL = "SELECT states.`name` AS state, polling_units.`name` AS polling_unit, polling_units.id AS polling_unit_id, registration_areas.`name` AS registration_area, local_governments.`name` AS local_government, local_governments.abbreviation AS lga_code, states.abbreviation AS state_code, registration_areas.abbreviation AS ra_code, polling_units.abbreviation AS pu_code FROM    polling_units INNER JOIN registration_areas ON polling_units.registration_area_id = registration_areas.id INNER JOIN local_governments ON registration_areas.local_government_id = local_governments.id INNER JOIN states ON local_governments.state_id = states.id WHERE    polling_units.id ='" +  QString::number(curPollingUnitId) + "' GROUP BY polling_units.id ";
    qDebug() << "State Info Query" << puSQL;

    QSqlQueryModel *pulistModel;
    pulistModel = new QSqlQueryModel;
    pulistModel->setQuery(puSQL);

    int j = 0;
    QString curState = pulistModel->record(j).value("state").toString();
    QString curPollingUnit = pulistModel->record(j).value("polling_unit").toString();
    QString curRA = pulistModel->record(j).value("registration_area").toString();
    QString curLGA = pulistModel->record(j).value("local_government").toString();
    QString stateCode = pulistModel->record(j).value("state_code").toString();
    QString lgaCode = pulistModel->record(j).value("lga_code").toString();
    QString raCode = pulistModel->record(j).value("ra_code").toString();
    QString puCode = pulistModel->record(j).value("pu_code").toString();

    qDebug() << "Current State: " << curState;

    int pageWidth = 10000;
    int pageHeight = 13500;
    int columns = ui->spinColumns->text().toInt();
    int rows = ui->spinRows->text().toInt();
    int ticketWidth = pageWidth/columns;
    int ticketHeight = pageHeight/rows;

    QTime estTime = ui->startTime->time();
    int intervalMin;
    intervalMin = ui->spinTimeInterval->text().toInt();


    int ticketLabelX, ticketLabelY;
    int ticketValueX, ticketValueY;
    int ticketSpacing = 125;
    ticketLabelX = 300; // Left Padding of Label
    ticketLabelY = ticketValueY = 250;

    ticketValueX = 1050; // Left Padding of Value
    QString currentTime = QDateTime::currentDateTime().toString();

    int numPages = ui->spinNumPages->text().toInt();
    int count = 0;

    for (int p = 0; p < numPages; p++) {

        // Draw lines
        for(int i = 0; i < rows; i++) {
            painter.drawLine(0, ticketHeight+(ticketHeight*i), pageWidth, ticketHeight+(ticketHeight*i));
        }

        for(int j = 0; j < columns; j++) {
            painter.drawLine(ticketWidth+(ticketWidth*j), 0 , ticketWidth+(ticketWidth*j), pageHeight);
        }

        for(int i = 0; i < columns; i++) {
            for(int j = 0; j < rows; j++) {

                count++;
                if (ui->cbEstTime->isChecked())
                {
                    painter.setFont(timeFont);
                    painter.drawText(ticketWidth-1500+(ticketWidth*i), (ticketHeight-600)+(ticketHeight*j), estTime.toString("hh:mmAP"));
                    painter.setFont(labelFont);
                    painter.drawText(ticketWidth-1750+(ticketWidth*i), ticketHeight-850+(ticketHeight*j), "Estimated Time of Service");

                    estTime = estTime.addSecs(intervalMin * 60);
                }

                painter.setFont(titleFont);
                painter.drawText(300+(ticketWidth*i), 300+(ticketHeight*j), "INEC");


                painter.drawText(ticketWidth-2000+(ticketWidth*i), 300+(ticketHeight*j), "QUEUE TICKET");



                painter.setFont(noFont);
                painter.drawText(850+(ticketWidth*i), 750+(ticketHeight*j), "NO");

                painter.setFont(numFont);
                painter.drawText(1400+(ticketWidth*i), 750+(ticketHeight*j), QString::number(ticketNo));

                ticketLabelY += 675;

                painter.setFont(labelFont);
                painter.drawText(ticketLabelX+(ticketWidth*i), ticketLabelY+(ticketHeight*j), "State: "+ stateCode + " - " +curState+"  LGA: "+ lgaCode + " - " + curLGA+"   Reg. Area: "+raCode+" - "+curRA);
                ticketLabelY += ticketSpacing;

                painter.drawText(ticketLabelX+(ticketWidth*i), ticketLabelY+(ticketHeight*j), "Polling Unit: " + puCode + " - " + curPollingUnit);
                ticketLabelY += ticketSpacing;

                painter.drawText(ticketLabelX+(ticketWidth*i), ticketLabelY+(ticketHeight*j), "Time Printed: " + currentTime);
                ticketLabelY += ticketSpacing;

                ticketNo++;

                ticketLabelY = 250;
                ticketValueY = 250;
            }
        }

        if (p != numPages-1) {
            printer->newPage();
        }
    }

    painter.end();
    saveLastTicketNo(ticketNo);


#endif
}


void QueueTickets::ticketPrintPreview()
{
#ifndef QT_NO_PRINTER

    QPrinter printer(QPrinter::HighResolution);
    printer.setPaperSize(QPrinter::A4);
    printer.setPageMargins(0, 0 ,0 , 0, QPrinter::Millimeter );

    QPrintPreviewDialog preview(&printer, this);
    preview.setWindowFlags ( Qt::Dialog );
    preview.setWindowState(Qt::WindowMaximized);

    connect(&preview, SIGNAL(paintRequested(QPrinter *)), SLOT(printTickets(QPrinter *)));
    preview.exec();
#endif
}


bool QueueTickets::saveLastTicketNo(int lastTicketNo)
{
    QSqlQuery dbQuery;
    QString queryString;
    if (settingExists) {
        queryString.sprintf("UPDATE settings SET setting_value='%d' WHERE setting_name='last_ticket_no'", lastTicketNo);
    } else {
        queryString.sprintf("INSERT INTO settings (setting_name, setting_value) VALUES ('last_ticket_no', '%d')", lastTicketNo);
    }
    return dbQuery.exec(queryString);
}


void QueueTickets::updateTotalTickets()
{
    int totalTickets;
    totalTickets = ui->spinRows->text().toInt() * ui->spinColumns->text().toInt() * ui->spinNumPages->text().toInt();
    ui->lblTotalTickets->setText( QString::number( totalTickets ) );

    // Calculate Last Estimated Time
    if (ui->cbEstTime->isChecked()) {
        updateLastEstTime(true);
    }

}

void QueueTickets::updateLastEstTime(bool useEstTime = true) {

    if (useEstTime) {
        if (ui->cbEstTime->isChecked()) {
            int totalTickets;
            totalTickets = ui->spinRows->text().toInt() * ui->spinColumns->text().toInt() * ui->spinNumPages->text().toInt();

            QTime lastTicketTime = ui->startTime->time();
            qDebug() << "Start Ticket Time: " << lastTicketTime.toString("hh:mmAP");
            lastTicketTime = lastTicketTime.addSecs( (totalTickets-1) * ui->spinTimeInterval->text().toInt() * 60 );
            ui->lblLastEstTime->setText( lastTicketTime.toString("hh:mmAP") );

//            qDebug() << "Total Ticket : " << QString::number(totalTickets);
//            qDebug() << "Last Ticket Time: " << lastTicketTime.toString("h:mAP");
        }
    } else {
        ui->lblLastEstTime->setText("-- -- --");
    }
}
