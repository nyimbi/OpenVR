#-------------------------------------------------
#
# Project created by QtCreator 2010-11-17T23:17:47
#
#-------------------------------------------------

QT       += core gui

TARGET = cameraBot
TEMPLATE = app


SOURCES += main.cpp\
        capturephotodialog.cpp

HEADERS  += capturephotodialog.h


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
