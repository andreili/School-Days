#-------------------------------------------------
#
# Project created by QtCreator 2015-04-22T20:02:43
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = unpacker
CONFIG   += console debug
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    ../classes/qfilesystem.cpp \
    ../classes/qgpk.cpp \
    ../classes/qgpkfile.cpp

HEADERS += \
    ../classes/qfilesystem.h \
    ../classes/qgpk.h \
    ../classes/qgpkfile.h
