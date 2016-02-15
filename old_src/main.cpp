bool developmentMode = false;

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

//#include "mainwindow.h"
#include "dashboard.h"
#include "myeventfilter.h"
#include "mysql_database.h"// either use this or sqlite_database.h, though the wARDS,states and lga tables are missing

MyEventFilter filter;
bool showSplash = true;

void myMessageHandler(QtMsgType type, const char *msg)
{
    QString txt;
    QDateTime date;

    txt = msg;
    txt.prepend(" - ");
    txt.prepend(date.currentDateTime().toString());

    QFile outFile(".applog");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt <<endl<<flush;
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.addLibraryPath("/opt/qtsdk-2010.04/qt/plugins/");
    a.addLibraryPath("/usr/local/include/");
    a.addLibraryPath("/usr/local/lib/");



    filter.lastKnownActivity = 20;
    a.installEventFilter(&filter);

    //foreach (const QString &path, a.libraryPaths())
        //qDebug() << "Paths: " << path ;

    if (!createConnection()){
        qDebug("Unable to Connect to Database");
        return 1;
    }

    qInstallMsgHandler(myMessageHandler);

    QPixmap pixmap("./resources/splash.png");

    QSplashScreen splash(pixmap);

    if ( showSplash )
    {


        splash.show();

        splash.showMessage("Loading OpenVR. Please wait...",Qt::AlignBottom,Qt::white);

        a.processEvents();

        QProcess *ramDiskProcess = new QProcess();

        ramDiskProcess->startDetached("scripts/create_ramdisk.sh");

        ramDiskProcess->waitForStarted();


        splash.showMessage("Loading Settings",Qt::AlignBottom,Qt::white);
        a.processEvents();
        sleep(0.5);

        splash.showMessage("Initialising Database",Qt::AlignBottom,Qt::white);
        a.processEvents();
        sleep(0.75);

        splash.showMessage("Database Loaded",Qt::AlignBottom,Qt::white);
        a.processEvents();
        sleep(0.75);

        splash.showMessage("Loading Registration Form",Qt::AlignBottom,Qt::white);
        a.processEvents();
        sleep(0.75);

        splash.showMessage("Initialising Device Drivers",Qt::AlignBottom,Qt::white);
        a.processEvents();
        sleep(0.75);

        splash.showMessage("Application Loaded",Qt::AlignBottom,Qt::white);
        a.processEvents();
        sleep(1);

    }

    DashBoard w;

    splash.finish(&w);

/*
    try{
        throw "dsad";
    }catch(int e){
        qDebug()<<" Throwing Int " <<e;
    }catch(const char *f){
        qDebug()<<" Char " <<f;
    }catch(...){
qDebug()<<" Finally ";
    }
*/
        w.show();

    return a.exec();
}










/*/
  // QGraphicsScene scene;
//    QGraphicsView view(&scene);
//    QGraphicsPixmapItem item(QPixmap("./enrolled.pgm"));
//    scene.addItem(&item);
//    view.show();
//    QPixmap image = new QPixmap("sss.jpg");
//

//lblLeftPrint.setPixmap(image);
//
//w.create(lblLeftPrint);

//lblLeftPrint;
  */


