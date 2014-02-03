!include($$PWD/../plugins.pri) {
	error("Couldn't find the plugins PRI file!")
}

TARGET = songs_view

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += \
	songs_view.h

SOURCES += \
	songs_view.cpp
