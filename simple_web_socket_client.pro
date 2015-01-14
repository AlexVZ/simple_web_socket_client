#-------------------------------------------------
#
# Project created by QtCreator 2015-01-12T16:04:31
#
#-------------------------------------------------

QT       += core gui websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = simple_web_socket_client
TEMPLATE = app

LIBS += -lcrypto
# PKGCONFIG += openssl

SOURCES += main.cpp\
        mainwindow.cpp \
    aes256helper.cpp \
    diffiehellmanhelper.cpp

HEADERS  += mainwindow.h \
    aes256helper.h \
    diffiehellmanhelper.h

FORMS    += mainwindow.ui

DISTFILES += \
    LICENSE
