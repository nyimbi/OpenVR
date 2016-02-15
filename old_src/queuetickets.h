#ifndef QUEUETICKETS_H
#define QUEUETICKETS_H

#include <QDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPainter>
#include <QDebug>
#include <QSqlQuery>
#include <QDateTime>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QTime>

namespace Ui {
    class QueueTickets;
}

class QueueTickets : public QDialog
{
    Q_OBJECT

public:
    explicit QueueTickets(QWidget *parent = 0);
    ~QueueTickets();
    void keyPressEvent(QKeyEvent *);

private:
    Ui::QueueTickets *ui;

private slots:
    void printTickets(QPrinter *printer);
    void ticketPrintPreview();
    bool saveLastTicketNo(int);
    void updateTotalTickets();
    void updateLastEstTime(bool);
};

#endif // QUEUETICKETS_H
