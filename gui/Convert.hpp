/*
 * Convert.hpp : GUI to libvlc-based video converter
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


#ifndef CONVERT_HPP
#define CONVERT_HPP

#include <QObject>
#include <QMainWindow>
#include "gui/ConvertThread.hpp"
#include "ui_convert.h"

class Convert: public QMainWindow
{
    Q_OBJECT;

public:
    Convert(void);
    ~Convert(void);

private:
    Ui::convertWindow ui;
    bool libvlc_loaded;
    void status(QString);
    ConvertThread *thread;

private slots:
    void input_choose(void);
    void output_choose(void);
    void convert_start(void);
    void  input_check(const QString &);
    void output_check(const QString &);
    void finished(void);
    void update_progress(float);
};

#endif /* CONVERT_HPP */
