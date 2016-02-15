# -------------------------------------------------
# Project created by QtCreator 2010-08-20T06:12:03
# -------------------------------------------------
QT += sql
TARGET = concept
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    registration.cpp \
    capturefingerdialog.cpp \
    capturephotodialog.cpp \
    controlpanel.cpp \
    finger_thread.cpp \
    recordsdialog.cpp \
    barcodeitem.cpp \
    reportsdialog.cpp \
    myeventfilter.cpp \
    bozorth_check.cpp \
    fingerdialog.cpp \
    fingerprintthread.cpp \
    dashboard.cpp \
    previewdialog.cpp \
    queuetickets.cpp \
    lineedit.cpp \
    helpdialog.cpp \
    nightcharts.cpp \
    devicecheck.cpp \
    specialenrollment.cpp\
    backup.cpp \
    import.cpp



HEADERS += mainwindow.h \
    registration.h \
    database.h \
    myfprint.h \
    sqlite_database.h \
    mysql_database.h \
    capturefingerdialog.h \
    capturephotodialog.h \
    controlpanel.h \
    finger_thread.h \
    recordsdialog.h \
    barcodeitem.h \
    reportsdialog.h \
    myeventfilter.h \
    bozorth_check.h \
    fingerdialog.h \
    fingerprintthread.h \
    dashboard.h \
    previewdialog.h \
    queuetickets.h \
    lineedit.h \
    finger_device_check.h \
    pgsql_database.h \
    helpdialog.h \
    nightcharts.h \
    devicecheck.h \
    specialenrollment.h\
    backup.h \
    import.h



FORMS += mainwindow.ui \
    registration_dialog.ui \
    capturefingerdialog.ui \
    controlpanel.ui \
    fingerprintwizard.ui \
    recordsdialog.ui \
    fingerdialog.ui \
    reportsdialog.ui \
    dashboard.ui \
    reportsdialog.ui \
    previewdialog.ui \
    queuetickets.ui \
    helpdialog.ui \
    capturephotodialog.ui \
    specialenrollment.ui \
    backup.ui \
    import.ui


   
LIBS += -L/usr/includes/libfprint/ -lfprint -L/opt/lib/ -lcv -lhighgui\
    -L/usr/local/lib/ -lzint -lQZint \
    -L/usr/lib/ -lzint -lQZint

LIBS += -L/usr/includes/libfprint/ -lfprint \
    -L/usr/local/includes/ -lzint -lQZint \
    -L/usr/local/include/opencv/ -lcv -lhighgui \
    -L/usr/local/include/ -lcv -lhighgui \
    -L/usr/lib/ -lcv -lhighgui \
    -L/usr/local/lib/ -lzint -lQZint



INCLUDEPATH += /usr/include/mysql/ /usr/lib/qt4/plugins/sqldrivers/ /opt/include/ /opt/qjson/
