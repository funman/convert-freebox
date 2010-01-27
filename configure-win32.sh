#!/bin/sh

root=../vlc

if [ ! -f $root/configure ]; then
    echo vlc source directory not found in $root !
    exit 1
fi

contrib=$PWD/$root/extras/contrib/build

PATH="$contrib/bin:$PATH" \
PKG_CONFIG_LIBDIR=$contrib/lib/pkgconfig \
CPPFLAGS="-I$contrib/include -I$contrib/include/ebml" \
LDFLAGS="-L$contrib/lib" \
CC=i586-mingw32msvc-gcc CXX=i586-mingw32msvc-g++ \
$root/configure --host=i586-mingw32msvc \
      --enable-mkv --enable-debug \
      --enable-faad \
      --enable-flac \
      --enable-theora \
      --enable-twolame \
      --enable-avcodec  --enable-merge-ffmpeg \
      --enable-dca \
      --enable-mpc \
      --enable-libass \
      --disable-qt4 --disable-skins2 \
      --enable-sse --enable-mmx \
      --enable-libcddb \
      --enable-zvbi --disable-telx \
      --disable-cddax --disable-vcdx --disable-libcdio --disable-vcdinfo \
      --disable-dvb \
      --enable-peflags
