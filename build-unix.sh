#!/bin/sh

set -e

qmake
make
lrelease convert-freebox.pro

rm -rf pkg; mkdir pkg
mkdir -p pkg/bin
mkdir -p pkg/share/applications
mkdir -p pkg/share/pixmaps
mkdir -p pkg/lib/convert-freebox/modules
mkdir -p pkg/lib/convert-freebox/translations

cp translations/*.qm pkg/lib/convert-freebox/translations/
find ./build-vlc/src -name \*.so\* -exec cp -d {} pkg/lib/convert-freebox/ \;
find ./build-vlc/modules -name \*.so -exec cp {} pkg/lib/convert-freebox/modules/ \;

for i in `cat extras/useless-modules`;do rm -f pkg/lib/convert-freebox/modules/lib${i}_plugin.so;done

cp convert-freebox pkg/lib/convert-freebox/
cp extras/convert-freebox.sh pkg/bin/convert-freebox
cp extras/convert-freebox.desktop pkg/share/applications
cp pics/free.png pkg/share/pixmaps
