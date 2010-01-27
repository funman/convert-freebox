/*
 * ConvertThread.hpp : GUI to libvlc-based video converter
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

#ifndef CONVERTTHREAD_HPP
#define CONVERTTHREAD_HPP

#include <QObject>
#include <QThread>

class ConvertThread: public QThread
{
    Q_OBJECT;

public:
    ConvertThread(const char *in, const char *out);
    void emit_progress_changed(float f);

protected:
    void run();

private:
    ~ConvertThread(void);
    char *in;
    char *out;

signals:
    void progress_changed(float f);
};

#endif /* CONVERTTHREAD_HPP */
