#-------------------------------------------------
#
# Project created by QtCreator 2015-08-23T20:59:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenFish
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    OpenWarp.cpp

HEADERS  += mainwindow.h \
    openwarp.h \
    asmOpenCV.h

FORMS    += mainwindow.ui \
    getsize.ui

unix|win32: LIBS += -L$$PWD/../../../../OpenCVB/BuildMingw/install/x86_AVX/mingw/lib/ -llibopencv_world300

INCLUDEPATH += $$PWD/../../../../OpenCVB/BuildMingw/install/include
DEPENDPATH += $$PWD/../../../../OpenCVB/BuildMingw/install/include
