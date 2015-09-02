#-------------------------------------------------
#
# Project created by QtCreator 2015-09-01T16:48:38
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GoBangGame
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    gamelogic.cpp

HEADERS  += dialog.h \
    map.h \
    gamelogic.h

FORMS    += dialog.ui
