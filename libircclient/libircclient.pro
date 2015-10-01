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
    irceventhandler.cpp \
    channel.cpp \
    mode.cpp \
    server.cpp \
    network.cpp \
    parser.cpp

HEADERS += user.h\
        libircclient_global.h \
    irceventhandler.h \
    channel.h \
    mode.h \
    server.h \
    network.h \
    parser.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
