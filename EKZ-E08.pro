#-------------------------------------------------
#
# Project created by QtCreator 2013-07-28T09:52:24
#
#-------------------------------------------------
cache()

QT       += core gui serialport xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EKZ-E08
TEMPLATE = app

CONFIG(release, debug|release):DEFINES += DEBUG=0 QT_NO_DEBUG_OUTPUT=1 QT_NO_WARNING_OUTPUT=1
else:CONFIG(debug, debug|release):DEFINES += DEBUG=1


SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/model_databuffer.cpp \
    src/dialogabout.cpp \
    src/dialogconfigcomm.cpp \
    src/dialogselectdevice.cpp \
    src/flashtable.cpp \
    src/programmer.cpp \
    src/worker.cpp \
    src/err_chk.cpp \
    src/packer.cpp \
    src/dialogdoaction.cpp \
    src/dialogproginfo.cpp

HEADERS  += src/mainwindow.h \
    src/model_databuffer.h \
    src/dialogabout.h \
    src/dialogconfigcomm.h \
    src/dialogselectdevice.h \
    src/flashtable.h \
    src/programmer.h \
    src/worker.h \
    src/err_chk.h \
    src/packer.h \
    src/dialogdoaction.h \
    src/dialogproginfo.h

FORMS    += src/mainwindow.ui \
    src/dialogabout.ui \
    src/dialogconfigcomm.ui \
    src/dialogselectdevice.ui \
    src/dialogdoaction.ui \
    src/dialogproginfo.ui

RESOURCES += \
    asset/asset.qrc
