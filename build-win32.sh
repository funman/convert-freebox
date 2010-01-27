#!/bin/sh

set -e

qmake -spec win32-x-g++
sed -i "s/^LIBS.*/& -lgdi32 -lcomdlg32 -loleaut32 -limm32 -lwinmm -lwinspool -lws2_32 -lole32 -luuid -luser32 -lpthreadGC2 -lwsock32/" Makefile.Release
make
lrelease convert-freebox.pro

rm -rf convert-freebox
mkdir convert-freebox; mkdir convert-freebox/modules

cp release/convert-freebox.exe convert-freebox/
find ./build-vlc/modules -name \*.dll -exec cp {} convert-freebox/modules/ \;
for i in `cat extras/useless-modules`;do rm -f convert-freebox/modules/lib${i}_plugin.dll;done
find ./build-vlc/src -name \*.dll -exec cp {} convert-freebox \;
mkdir convert-freebox/translations
cp translations/*.qm convert-freebox/translations/

zip -r convert-freebox.zip convert-freebox
