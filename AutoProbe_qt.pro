#-------------------------------------------------
#
# Project created by QtCreator 2011-02-04T20:45:39
#
#-------------------------------------------------

QT       += core gui

TARGET = AutoProbe_qt
TEMPLATE = app

#QMAKE_LFLAGS += -static-libgcc

#Release builds should have NDEBUG defined
CONFIG(release,debug|release) {
    DEFINES += NDEBUG
}

LIBS += -L"../AutoProbe_qt/lib/" -lclib_superprobe733

SOURCES += main.cpp\
        mainwindow.cpp \
    PointData.cpp \
    StageView.cpp \
    calibratezdlg.cpp \
    AutoProbe_Objects.cpp \
    optionsdlg.cpp \
    autoprobeobjview.cpp \
    CRADFileIO.cpp \
    AutoProbeSettings.cpp \
    ListenThread.cpp \
    WriteThread.cpp \
    brightnesscontrast.cpp \
    AutoProbeError.cpp \
    UtilThread.cpp

HEADERS  += mainwindow.h \
    PointData.h \
    ../clib_superprobe733/clib_superprobe733.h \
    StageView.h \
    calibratezdlg.h \
    3D_Math.h \
    AutoProbe_Objects.h \
    optionsdlg.h \
    autoprobeobjview.h \
    Constants.h \
    CRADFileIO.h \
    AutoProbeSettings.h \
    Utility.h \
    ListenThread.h \
    WriteThread.h \
    brightnesscontrast.h \
    AutoProbeError.h \
    UtilThread.h

FORMS    += mainwindow.ui \
    calibratezdlg.ui \
    optionsdlg.ui \
    brightnesscontrast.ui

win32:RC_FILE += AutoProbe_qt.rc
