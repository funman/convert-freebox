/*
 * convert-cli.c : CLI to libvlc-based video converter
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

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include "convert.h"

static void progress(float f, void *param)
{
    (void)param;
    printf("%.2f%%\n", f * 100);
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, getenv("LANG"));

    if(argc != 3)
    {
        fprintf(stderr, "Usage: %s <input> <output>\n", argv[0]);
        return -1;
    }

    if(convert_init() != 0)
    {
        fprintf(stderr, "convert_init() failed\n");
        return 1;
    }

    if(convert(argv[1], argv[2], progress, NULL) != 0)
    {
        fprintf(stderr, "conversion failed\n");
        return 2;
    }

    convert_exit();

    return 0;
}
