/*
 * ConvertThread.cpp : GUI to libvlc-based video converter
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

#include <stdlib.h>
#include "gui/ConvertThread.hpp"
#include "src/convert.h"

/* ConvertThread: run blocking transcode in a thread */

/* helper to emit progress_changed signal */
static void progress_func(float f, void *param)
{
    ConvertThread *C = (ConvertThread*)param;
    C->emit_progress_changed(f);
}

void ConvertThread::emit_progress_changed(float f)
{
    emit progress_changed(f);
}


ConvertThread::~ConvertThread(void)
{
    free(in);
    free(out);
}


ConvertThread::ConvertThread(const char *in, const char *out)
{
    this->in = strdup(in);
    this->out = strdup(out);
}


void ConvertThread::run(void)
{
    convert(in, out, progress_func, (void*)this);
}
