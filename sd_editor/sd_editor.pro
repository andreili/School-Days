#-------------------------------------------------
#
# Project created by QtCreator 2015-04-14T08:28:48
#
#-------------------------------------------------

QT       += core gui
#multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = sd_editor
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ../classes/qfilesystem.cpp \
    ../classes/qgpk.cpp \
    ../classes/qgpkfile.cpp \
    ../classes/qscript.cpp \
    ../classes/enginewidget.cpp

HEADERS  += mainwindow.h \
    ../classes/qfilesystem.h \
    ../classes/qgpk.h \
    ../classes/qgpkfile.h \
    ../classes/qscript.h \
    ../classes/enginewidget.h \
    ../classes/qactiontimer.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    ../game/game.json

debug:QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
debug:QMAKE_LDFLAGS += -fprofile-arcs -ftest-coverage
debug:LIBS += -lgcov
