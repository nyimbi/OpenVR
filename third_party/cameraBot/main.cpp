#include "capturephotodialog.h"
#include <QtGui/QApplication>
#include <QProcess>
#include <QPushButton>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSplashScreen>
#include <QDebug>
#include <QLabel>
#include <QFile>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CapturePhotoDialog w;
    w.show();

    return a.exec();
}
