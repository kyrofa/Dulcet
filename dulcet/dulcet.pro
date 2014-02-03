TEMPLATE = app
TARGET = Dulcet

QT += core gui multimedia sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

HEADERS += \
	main_window.h \
	music_file_type_resolver.h \
	droppable_widget.h \
    media_manager.h \
    preferences_dialog.h \
    file_dialog.h

SOURCES += \
	main.cpp \
	main_window.cpp \
	music_file_type_resolver.cpp \
	droppable_widget.cpp \
    media_manager.cpp \
    preferences_dialog.cpp \
    file_dialog.cpp

FORMS += main_window.ui \
    preferences_dialog.ui

LIBS += -ltag

RESOURCES += images.qrc
