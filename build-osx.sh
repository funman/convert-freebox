#!/bin/sh

set -e

APP=convert-freebox.app/Contents/MacOS

qmake
xcodebuild
for i in lib modules; do cp -r ./vlc/VLC-release.app/Contents/MacOS/$i $APP;done
for i in `cat extras/useless-libs`;do rm -f $APP/lib/lib${i}*.dylib;done
for i in `cat extras/useless-modules`;do rm -f $APP/modules/lib${i}_plugin.dylib;done

mkdir $APP/translations
lrelease convert-freebox.pro
cp translations/*.qm $APP/translations/

install_name_tool -change QtGui.framework/Versions/4/QtGui @executable_path/QtGui.framework/Versions/4/QtGui $APP/convert-freebox
install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore.framework/Versions/4/QtCore $APP/convert-freebox

mkdir -p $APP/QtGui.framework/Versions/4
mkdir -p $APP/QtCore.framework/Versions/4
cp /Library/Frameworks/QtGui.framework/Versions/Current/QtGui $APP/QtGui.framework/Versions/4
cp /Library/Frameworks/QtCore.framework/Versions/Current/QtCore $APP/QtCore.framework/Versions/4

install_name_tool -change QtCore.framework/Versions/4/QtCore @executable_path/QtCore.framework/Versions/4/QtCore $APP/QtGui.framework/Versions/4/QtGui

mkdir -p $APP/../Resources/fr.lproj

rm -f convert-freebox.zip
zip -r convert-freebox.zip convert-freebox.app
