TEMPLATE = lib # This isn't actually a plugin

TARGET = plugin_interfaces
#DESTDIR = $$OUT_PWD/../../ # Put in with Dulcet

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += \
	dulcet_plugin_interface.h \
	music_view_interface.h
