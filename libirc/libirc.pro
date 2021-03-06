#-------------------------------------------------
#
# Project created by QtCreator 2015-09-23T17:47:15
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = libirc
TEMPLATE = lib

DEFINES += LIBIRC_LIBRARY

SOURCES += network.cpp \
    user.cpp \
    server.cpp \
    mode.cpp \
    channel.cpp \
    serializableitem.cpp \
    serveraddress.cpp

HEADERS += network.h\
        libirc_global.h \
    user.h \
    server.h \
    mode.h \
    channel.h \
    error_code.h \
    serializableitem.h \
    serveraddress.h \
    irc_standards.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
