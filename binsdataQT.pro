#-------------------------------------------------
#
# Project created by QtCreator 2018-08-13T14:45:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4):

QT += widgets serialport

TARGET = binsdataQT
TEMPLATE = app


SOURCES += \
        main.cpp \
    window.cpp \
    glwidget.cpp \
    datawork.cpp \
    masterthread.cpp

HEADERS += \
    window.h \
    glwidget.h \
    datawork.h \
    masterthread.h

INCLUDEPATH += ....\Libs\Eigen@




