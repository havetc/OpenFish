#-------------------------------------------------
#
# Project created by QtCreator 2015-08-23T20:59:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenFish
TEMPLATE = app

win32:RC_ICONS += favicon.ico

SOURCES += main.cpp\
    mainwindow.cpp \
    OpenWarp.cpp

HEADERS += mainwindow.h \
    openwarp.h \
    asmOpenCV.h

FORMS    += mainwindow.ui \
    getsize.ui

#unix|win32: LIBS += -L$$PWD/../../../../OpenCVB/BuildMingw/install/x86/mingw/staticlib/ -llibopencv_core300 -llibopencv_imgproc300 -llibopencv_video300

#INCLUDEPATH += $$PWD/../../../../OpenCVB/BuildMingw/install/include
#DEPENDPATH += $$PWD/../../../../OpenCVB/BuildMingw/install/include

#unix|win32: LIBS += -lcomctl32
#unix|win32: LIBS += -lmsvfw32


unix|win32: LIBS += -L$$PWD/../../../../OpenCVB/BuildMingw/install/x86/mingw/lib -lopencv_world300

INCLUDEPATH += $$PWD/../../../../OpenCVB/BuildMingw/install/include
DEPENDPATH += $$PWD/../../../../OpenCVB/BuildMingw/install/include


