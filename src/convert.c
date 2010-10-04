/*
 * convert.c : libvlc-based video converter
 *
 * Copyright © 2009-2010 Rafaël Carré
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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>

#include <string.h>

#include <pthread.h>

#include <assert.h>

/* libvlc */
#include <vlc/libvlc.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_player.h>
#include <vlc/libvlc_events.h>

#include "convert.h"

#ifdef BIGENDIAN
#define FOURCC( a, b, c, d ) \
    ( ((uint32_t)d)         | ( ((uint32_t)c) << 8 ) | \
    ( ((uint32_t)b) << 16 ) | ( ((uint32_t)a) << 24 ) )
#else
#define FOURCC( a, b, c, d ) \
    ( ((uint32_t)a)         | ( ((uint32_t)b) << 8 ) | \
    ( ((uint32_t)c) << 16 ) | ( ((uint32_t)d) << 24 ) )
#endif

/* globals variables XXX : not reentrant! */
static libvlc_instance_t *libvlc;
static pthread_mutex_t callback_lock;

/* libvlc settings */
static const char* const libvlc_args[] = {
    "--intf=dummy",              /* no interface */
    "--vout=dummy",             /* we don't want video output */
    "--aout=dummy",             /* we don't want audio output */
    "--verbose=2",              /* show debug log */
    "--no-media-library",       /* don't want that */
    "--no-video-title-show",    /* nor the filename displayed */
    "--no-stats",               /* no stats */
    "--no-sub-autodetect",      /* don't want subtitles */
    "--no-disable-screensaver", /* no disabling screensaver */

#if !defined(WIN32) && !defined(__APPLE__)
    /* on windows & osx, libvlccore looks for plugins in the directory
     * relative to executable or libvlccore */
    "--plugin-path=/usr/lib/convert-freebox/modules",
#endif
};

static const int libvlc_nargs = sizeof(libvlc_args) / sizeof(libvlc_args[0]);

/* static functions */

#ifdef WIN32
/* asprintf() replacement (not present in mingw)
 * Copyright © 1998-2008 the VideoLAN project */
static int asprintf (char **strp, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start (ap, fmt);

    ssize_t len = vsnprintf (NULL, 0, fmt, ap) + 1;
    char *res = malloc (len);
    if (res == NULL)
        return -1;
    *strp = res;
    ret = vsnprintf (res, len, fmt, ap);

    va_end (ap);
    return ret;
}
#endif


/* notify event catching */
static void callback(const libvlc_event_t *ev, void *param)
{
    assert( ev->type == libvlc_MediaPlayerPlaying ||
            ev->type == libvlc_MediaPlayerEndReached );

    pthread_mutex_lock(&callback_lock);
    *(bool*)param = true;
    pthread_mutex_unlock(&callback_lock);
}


/* video ES must be transcoded ? */
static bool transcode_video(uint32_t i_codec, unsigned int i_level)
{
    switch(i_codec)
    {
    /* MPEG-1 */
    case FOURCC('m','p','1','v'):
    case FOURCC('m','p','e','g'):
    case FOURCC('m','p','g','1'):
    case FOURCC('P','I','M','1'):
    /* MPEG-2 */
    case FOURCC('m','p','g','v'):
    case FOURCC('m','p','2','v'):
    case FOURCC('M','P','E','G'):
    case FOURCC('m','p','g','2'):
    case FOURCC('h','d','v','1'):
    case FOURCC('h','d','v','2'):
    case FOURCC('h','d','v','3'):
    case FOURCC('h','d','v','5'):
    case FOURCC('m','x','5','n'):
    case FOURCC('m','x','5','p'):
    case FOURCC('m','x','4','n'):
    case FOURCC('m','x','4','p'):
    case FOURCC('m','x','3','n'):
    case FOURCC('m','x','3','p'):
    case FOURCC('x','d','v','2'):
    case FOURCC('A','V','m','p'):
    case FOURCC('V','C','R','2'):
    case FOURCC('M','M','E','S'):
    case FOURCC('m','m','e','s'):
    /* MPEG-4 */
    case FOURCC('D','I','V','X'):
    case FOURCC('d','i','v','x'):
    case FOURCC('M','P','4','S'):
    case FOURCC('M','P','4','s'):
    case FOURCC('M','4','S','2'):
    case FOURCC('m','4','s','2'):
    case FOURCC('x','v','i','d'):
    case FOURCC('X','V','I','D'):
    case FOURCC('X','v','i','D'):
    case FOURCC('X','V','I','X'):
    case FOURCC('x','v','i','x'):
    case FOURCC('D','X','5','0'):
    case FOURCC('d','x','5','0'):
    case FOURCC('B','L','Z','0'):
    case FOURCC('B','X','G','M'):
    case FOURCC('m','p','4','v'):
    case FOURCC('M','P','4','V'):
    case FOURCC( 4 , 0 , 0 , 0 ):
    case FOURCC('m','4','c','c'):
    case FOURCC('M','4','C','C'):
    case FOURCC('F','M','P','4'):
    case FOURCC('f','m','p','4'):
    case FOURCC('3','I','V','2'):
    case FOURCC('3','i','v','2'):
    case FOURCC('U','M','P','4'):
    case FOURCC('W','V','1','F'):
    case FOURCC('S','E','D','G'):
    case FOURCC('R','M','P','4'):
    case FOURCC('H','D','X','4'):
    case FOURCC('h','d','x','4'):
    case FOURCC('S','M','P','4'):
    case FOURCC('f','v','f','w'):
    case FOURCC('F','V','F','W'):
    /* MSMPEG4 v1 (Not sure that we should include these) */
    case FOURCC('D','I','V','1'):
    case FOURCC('d','i','v','1'):
    case FOURCC('M','P','G','4'):
    case FOURCC('m','p','g','4'):
    /* MSMPEG4 v2 (Not sure that we should include these) */
    case FOURCC('D','I','V','2'):
    case FOURCC('d','i','v','2'):
    case FOURCC('M','P','4','2'):
    case FOURCC('m','p','4','2'):
    /* MSMPEG4 v3 (Not sure that we should include these) */
    case FOURCC('M','P','G','3'):
    case FOURCC('m','p','g','3'):
    case FOURCC('d','i','v','3'):
    case FOURCC('M','P','4','3'):
    case FOURCC('m','p','4','3'):
    case FOURCC('D','I','V','3'):
    case FOURCC('D','I','V','4'):
    case FOURCC('d','i','v','4'):
    case FOURCC('D','I','V','5'):
    case FOURCC('d','i','v','5'):
    case FOURCC('D','I','V','6'):
    case FOURCC('d','i','v','6'):
    case FOURCC('C','O','L','1'):
    case FOURCC('c','o','l','1'):
    case FOURCC('C','O','L','0'):
    case FOURCC('c','o','l','0'):
    case FOURCC('A','P','4','1'):
    case FOURCC('3','I','V','D'):
    case FOURCC('3','i','v','d'):
    case FOURCC('3','V','I','D'):
    case FOURCC('3','v','i','d'):
        return false;

    case FOURCC('a','v','c','1'):
    case FOURCC('A','V','C','1'):
    case FOURCC('h','2','6','4'):
    case FOURCC('H','2','6','4'):
    case FOURCC('x','2','6','4'):
    case FOURCC('X','2','6','4'):
    case FOURCC('V','S','S','H'):
    case FOURCC('V','S','S','W'):
    case FOURCC('v','s','s','h'):
    case FOURCC('D','A','V','C'):
    case FOURCC('d','a','v','c'):
        if(i_level <= 40) /* level 4.0 max */
            return false;

    default:    /* transcoding */
        return true;
    }
}


/* audio ES must be transcoded ? */
static bool transcode_audio(uint32_t i_codec, unsigned int i_layer)
{
    switch(i_codec)
    {
    case FOURCC('m','p','4','a'):
        return false;
    case FOURCC('m','p','g','a'):
        switch(i_layer)
        {
            case 1:
            case 2:
                return false;
        }
    default:
        return true;
    }
}

static char *get_transcode_chain(libvlc_media_track_info_t *p_es)
{
    char *chain;
    int ret = -1;

    if(p_es->i_type == libvlc_track_video)
    {
        ret = asprintf(&chain, "dst=transcode%s,select=es=%d",
            transcode_video(p_es->i_codec, p_es->i_level) ?
                "{vcodec=mp2v,vb=1000}" : "",
            p_es->i_id);
    }
    else if(p_es->i_type == libvlc_track_audio)
    {
        ret = asprintf(&chain, "dst=transcode%s,select=es=%d",
            transcode_audio(p_es->i_codec, p_es->i_profile) ?
                "{acodec=mp2,ab=128000,channels=2}" : "",
            p_es->i_id);
    }
    /* else, subs or unknown */

    return (ret == -1) ? NULL : chain;
}

/* transcode needed ES, mux all ES to mkv
 * blocking, progress is reported through a given callback */
static int convert_write(const char *in, const char *out,
                         unsigned i_es, libvlc_media_track_info_t *p_es,
                         void (*progress) (float, void*), void *param)
{
    unsigned i_videos = 0;
    unsigned i_subs = 0;

    /* check ES */
    for(unsigned i = 0; i < i_es; i++)
    {
        if(p_es[i].i_type == libvlc_track_video)
            i_videos++;
        else if(p_es[i].i_type == libvlc_track_text)
            i_subs++;
    }

    if(i_videos > 1)
    {
        fprintf(stderr, "Files with multiple video tracks not supported\n");
        return -1;
    }

    if(i_subs > 0)
        fprintf(stderr, "%d subtitles won't be included\n", i_subs);

    char *chains[i_es];
    unsigned used_es = 0;
    for(unsigned i = 0; i < i_es; i++)
    {
        char *chain = get_transcode_chain(&p_es[i]);
        if(chain)
            chains[used_es++] = chain;
    }

    if(used_es == 0)
    {
        fprintf(stderr, "No Elementary Streams found!\n");
        return -1;
    }

    char *std = NULL, *sout = strdup("sout=#duplicate{");
    if(!sout)
        goto transcode_error;

    if(asprintf(&std, "}:std{access=file,dst=%s}", out) == -1)
        goto transcode_error;

    for(unsigned i = 0; i < used_es; i++)
    {
        char *new;
        if(-1 == asprintf(&new, "%s%s%s", sout, chains[i],
                (i == used_es - 1) ? std : ","))
        {
            free(std);
            goto transcode_error;
        }
        free(sout);
        sout = new;
    }

    for(unsigned i = 0; i < used_es; i++)
        free(chains[i]);

    libvlc_media_t *media = libvlc_media_new_path(libvlc, in);
    if(!media)
        goto error;

    libvlc_media_add_option(media, "sout-all");
    libvlc_media_add_option(media, "no-sout-spu");
    libvlc_media_add_option(media, sout);
    free(sout);

    libvlc_media_player_t *mp;
    libvlc_event_manager_t *em;

    mp = libvlc_media_player_new_from_media(media);
    if(!mp)
        goto error;
    em = libvlc_media_player_event_manager(mp);
    if(!em)
        goto error;
    bool end = false;
    if(libvlc_event_attach(em, libvlc_MediaPlayerEndReached, callback, &end))
        goto error;

    if(libvlc_media_player_play(mp))
        goto error;

    int ds = 0;
    bool finished;
    do
    {
        usleep(100000);
        if(ds++ == 10)
        {
            ds = 0;
            float pos = libvlc_media_player_get_position(mp);
            progress(pos, param);
        }
        pthread_mutex_lock(&callback_lock);
        finished = end;
        pthread_mutex_unlock(&callback_lock);
    } while(!finished);

    libvlc_event_detach(em, libvlc_MediaPlayerEndReached, callback, &end);

    libvlc_media_player_stop(mp);

    libvlc_media_player_release(mp);

    libvlc_media_release(media);
    return 0;

error:
    libvlc_media_release(media);
    return -1;

transcode_error:
    free(sout);
    free(std);
    for(unsigned i = 0; i < used_es; i++)
        free(chains[i]);

    return -1;
}


/* Extract all ES from input file (blocking) */
static int convert_read(const char *in, unsigned *i_es,
    libvlc_media_track_info_t **pp_es)
{
    libvlc_media_player_t *mp = NULL;

    libvlc_media_t *m = libvlc_media_new_path(libvlc, in);
    if(!m)
        goto error;

    libvlc_media_add_option(m, "sout-all");
    libvlc_media_add_option(m, "no-sout-spu");
    libvlc_media_add_option(m, "sout=#description");

    mp = libvlc_media_player_new_from_media(m);
    if(!mp)
        goto error;

    libvlc_event_manager_t *em = libvlc_media_player_event_manager(mp);
    if(!em)
        goto error;

    bool playing = false;
    if(libvlc_event_attach(em, libvlc_MediaPlayerPlaying, callback, &playing))
        goto error;

    if(libvlc_media_player_play(mp))
        goto error;

    for(;;)
    {
        bool b;
        pthread_mutex_lock(&callback_lock);
        b = playing;
        pthread_mutex_unlock(&callback_lock);
        if(b) break;
        usleep(100000); /* 0.1 second */
    }

    libvlc_event_detach(em, libvlc_MediaPlayerPlaying, callback, &playing);

    *i_es = libvlc_media_get_tracks_info(m, pp_es);

    libvlc_media_player_stop(mp);

    libvlc_media_release(m);
    libvlc_media_player_release(mp);

    return 0;

error:
    if(m) libvlc_media_release(m);
    if(mp) libvlc_media_player_release(mp);
    return -1;
}


/* exported functions */


/* library init */
int convert_init(void)
{
    if(pthread_mutex_init(&callback_lock, NULL) != 0)
        return -1;

    libvlc = libvlc_new(libvlc_nargs, libvlc_args);
    if(!libvlc)
        return -1;

    return 0;
}


/* library cleanup */
void convert_exit(void)
{
    pthread_mutex_destroy(&callback_lock);
    libvlc_release(libvlc);
}


/* */
int convert(const char *in, const char *out, void (*progress) (float, void*), void *param)
{
    unsigned i_es;
    libvlc_media_track_info_t *p_es = NULL;

    if(convert_read(in, &i_es, &p_es) != 0)
        return -1;

    if(i_es == 0)
        return -2;

    int ret = convert_write(in, out, i_es, p_es, progress, param);

    free(p_es);

    return ret;
}
