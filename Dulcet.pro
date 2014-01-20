#-------------------------------------------------
#
# Project created by QtCreator 2014-01-18T09:40:55
#
#-------------------------------------------------

QT       += core gui multimedia sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Dulcet
TEMPLATE = app


SOURCES += main.cpp\
        main_window.cpp \
    droppable_table_view.cpp \
    music_table_model.cpp \
    music_table_item.cpp \
    music_table_item_delegate.cpp \
    music_file_type_resolver.cpp

HEADERS  += main_window.h \
    droppable_table_view.h \
    music_table_model.h \
    music_table_item.h \
    music_table_item_delegate.h \
    music_file_type_resolver.h

FORMS    += main_window.ui

LIBS += -ltag
