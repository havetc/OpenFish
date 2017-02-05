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
    OpenWarp.cpp \
    fifoomp.cpp

HEADERS += mainwindow.h \
    openwarp.h \
    asmOpenCV.h \
    renderThread.hpp \
    fifoomp.h

FORMS    += mainwindow.ui \
    getsize.ui

#unix|win32: LIBS += -L$$PWD/../../../../OpenCVB/BuildMingw/install/x86/mingw/staticlib/ -llibopencv_core300 -llibopencv_imgproc300 -llibopencv_video300

#INCLUDEPATH += $$PWD/../../../../OpenCVB/BuildMingw/install/include
#DEPENDPATH += $$PWD/../../../../OpenCVB/BuildMingw/install/include

#unix|win32: LIBS += -lcomctl32
#unix|win32: LIBS += -lmsvfw32


win32: LIBS += -L$(OPENCV_DIR)\BuildMingw\install\x86\mingw\lib -lgomp -lopencv_world300

win32: INCLUDEPATH += $(OPENCV_DIR)\BuildMingw\install\include
win32: DEPENDPATH += $(OPENCV_DIR)\BuildMingw\install\include

#file icon: http://www.iconarchive.com/show/must-have-icons-by-visualpharm/Open-icon.html
win32:RC_ICONS += favicon.ico

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv
unix: LIBS += -lgomp
unix: packagesExist(libav-tools){message(ok) }

QMAKE_CXXFLAGS += -std=c++11 -fopenmp

DISTFILES += \
    LICENSE
