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
	file_dialog.h \
	dulcet_utilities.h \
	media_discoverer.h \
	media_organizer.h \
    sort_multi_filter_proxy_model.h

SOURCES += \
	main.cpp \
	main_window.cpp \
	music_file_type_resolver.cpp \
	droppable_widget.cpp \
	media_manager.cpp \
	preferences_dialog.cpp \
	file_dialog.cpp \
	dulcet_utilities.cpp \
	media_discoverer.cpp \
	media_organizer.cpp \
    sort_multi_filter_proxy_model.cpp

FORMS += main_window.ui \
	preferences_dialog.ui

LIBS += -ltag

RESOURCES += images.qrc
