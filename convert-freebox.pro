DEFINES = PTW32_STATIC_LIB
INCLUDEPATH = ./vlc/include
LIBS = -lvlc -L./lib

QMAKE_CFLAGS += -std=c99

HEADERS = src/convert.h gui/ConvertThread.hpp gui/Convert.hpp
SOURCES = src/convert.c gui/main.cpp gui/ConvertThread.cpp gui/Convert.cpp
FORMS   = ui/convert.ui

TRANSLATIONS = translations/convert_fr.ts

RESOURCES = pics/free.qrc
RC_FILE = pics/free.rc
ICON = pics/free.icns

target.path = convert-freebox
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro
sources.path = .
INSTALLS += target sources
