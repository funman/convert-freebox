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

/* transcode needed ES, mux all ES to mkv
 * blocking, progress is reported through a given callback */
static int convert_write(const char *in, const char *out,
                         unsigned i_es, libvlc_media_es_t *p_es,
                         void (*progress) (float, void*), void *param)
{
    unsigned i_videos = 0;
    unsigned i_subs = 0;
    unsigned i_audios = 0;

    char *p_channels, *p_ab, *p_acodec;

    for(unsigned i = 0; i < i_es; i++)
    {
        if(p_es[i].i_type == libvlc_es_video)
            i_videos++;
        else if(p_es[i].i_type == libvlc_es_text)
            i_subs++;
        else if(p_es[i].i_type == libvlc_es_audio)
            i_audios++;
    }

    if(i_videos > 1)
    {
        fprintf(stderr, "Files with multiple video tracks not supported\n");
        return -1;
    }

    if(i_subs > 0)
        fprintf(stderr, "%d subtitles won't be included\n", i_subs);

    bool video = false;
    bool audio[i_audios];
    unsigned i_audio = 0;

    for(unsigned i = 0; i < i_es; i++)
    {
        if(p_es[i].i_type == libvlc_es_video)
            video = transcode_video(p_es[i].i_codec, p_es[i].i_level);
        else if(p_es[i].i_type == libvlc_es_audio)
            audio[i_audio++] =
                transcode_audio(p_es[i].i_codec, p_es[i].i_profile);
    }

    char *sout;
    bool transcode_audio_stream = false;
    for(unsigned i = 0; i < i_audio; i++)
        if(audio[i])
            transcode_audio_stream = true;

    if(!video && !transcode_audio_stream)
    {
        if(asprintf(&sout, "sout=#std{access=file,dst=%s}", out) == -1)
            return -1;
    }
    else
    {
        if(asprintf(&sout, "%s", "sout=#transcode{") == -1)
            return -1;

        if(video)
        {
            char *new;
            if(asprintf(&new, "%svcodec=mp2v,vb=1000", sout) == -1) /* VBR ? */
            {
                free(sout);
                return -1;
            }
            free(sout);
            sout = new;
        }

        if(!transcode_audio_stream)
        {
            char acodec[sizeof("copy,") * i_audios];
            for(unsigned i = 0; i < i_audios; i++)
                memcpy(&acodec[5*i], "copy,", 5);
            acodec[(sizeof("copy,") * i_audios) - 1] = '\0';


            char *new;
            if(asprintf(&new, "%s%sacodec=%s}:std{access=file,dst=%s}",
                    sout, acodec, video ? "," : "", out) == -1)
            {
                free(sout);
                return -1;
            }
            free(sout);
            sout = new;
        }
        else
        {
            char *new;

            p_channels = strdup("channels={");
            p_ab = strdup("ab={");
            p_acodec = strdup("acodec={");
            if(!p_channels || !p_ab || !p_acodec)
                goto audio_transcode_error;

            for(unsigned i = 0; i < i_audios; i++)
            {
                char *acodec;
                unsigned channels, ab;
                if(audio[i])
                {
                    ab = 128000;
                    channels = 2;
                    acodec = "mp2";
                }
                else
                {
                    ab = 0;
                    channels = 0;
                    acodec = "copy";
                }

                if(asprintf(&new, "%s%d,", p_ab, ab ) == -1)
                    goto audio_transcode_error;
                free(p_ab); p_ab = new;

                if(asprintf(&new, "%s%d,", p_channels, channels ) == -1)
                    goto audio_transcode_error;
                free(p_channels); p_channels = new;

                if(asprintf(&new, "%s%s,", p_acodec, acodec ) == -1)
                    goto audio_transcode_error;
                free(p_acodec); p_acodec = new;
            }

            p_ab[strlen(p_ab) - 1] = '}';
            p_channels[strlen(p_channels) - 1] = '}';
            p_acodec[strlen(p_acodec) - 1] = '}';

            if(asprintf(&new, "%s%s%s,%s,%s", sout, video ? "," : "", p_acodec,
                    p_ab, p_channels) == -1)
                goto audio_transcode_error;

            free(sout); sout = new;
        }

        char *new;
        if(asprintf(&new, "%s}:std{access=file,dst=%s}", sout, out) == -1)
        {
            free(sout);
            return -1;
        }
        free(sout);
        sout = new;
    }

    libvlc_media_t *media = libvlc_media_new(libvlc, in);
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

audio_transcode_error:
    free(p_channels);
    free(p_ab);
    free(p_acodec);
    free(sout);
    return -1;
}


/* Extract all ES from input file (blocking) */
static int convert_read(const char *in, unsigned *i_es,
    libvlc_media_es_t **pp_es)
{
    libvlc_media_player_t *mp = NULL;

    libvlc_media_t *m = libvlc_media_new(libvlc, in);
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

    *i_es = libvlc_media_get_es(m, pp_es);

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
    libvlc_media_es_t *p_es = NULL;

    if(convert_read(in, &i_es, &p_es) != 0)
        return -1;

    if(i_es == 0)
        return -2;

    int ret = convert_write(in, out, i_es, p_es, progress, param);

    free(p_es);

    return ret;
}
