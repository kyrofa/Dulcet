TEMPLATE = lib
CONFIG += plugin

INCLUDEPATH += $$PWD/

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../plugin_interfaces/release/ -lplugin_interfaces
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../plugin_interfaces/debug/ -lplugin_interfaces
else:unix: LIBS += -L$$OUT_PWD/../plugin_interfaces/ -lplugin_interfaces

DEPENDPATH += $$PWD/plugin_interfaces

QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/../plugin_interfaces\',-z,origin'
