#-------------------------------------------------
#
# Project created by QtCreator 2015-11-17T18:21:52
#
#-------------------------------------------------

QT       += core gui network webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets webkitwidgets

TARGET = sniff_browser
TEMPLATE = app


SOURCES += main.cpp\
        browserwindow.cpp \
    interceptormanager.cpp \
    networkreplyproxy.cpp

HEADERS  += browserwindow.h \
    interceptormanager.h \
    networkreplyproxy.h

FORMS    += browserwindow.ui

CONFIG += c++11
