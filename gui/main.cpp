/*
 * main.cpp : GUI to libvlc-based video converter
 *
 * Copyright © 2009 Rafaël Carré
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <QApplication>
#include <QTranslator>
#include <QLocale>

#include "gui/Convert.hpp"

#include <libgen.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator translator;

    /* translations folder is next to the executable */
    translator.load(QString(dirname(argv[0])) + "/translations/convert_" +
            QLocale::system().name());
    app.installTranslator(&translator);

    Convert *convert = new Convert;

    convert->setWindowIcon(QIcon(":/free.png"));

    int ret = app.exec();

    delete convert;

    return ret;
}
