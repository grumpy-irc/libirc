#-------------------------------------------------
#
# Project created by QtCreator 2015-09-24T15:15:16
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = libircclient
TEMPLATE = lib

DEFINES += LIBIRCCLIENT_LIBRARY

SOURCES += user.cpp \
    irceventhandler.cpp

HEADERS += user.h\
        libircclient_global.h \
    irceventhandler.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
