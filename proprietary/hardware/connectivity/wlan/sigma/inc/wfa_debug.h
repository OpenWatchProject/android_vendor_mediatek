/****************************************************************************
*
* Copyright (c) 2016 Wi-Fi Alliance
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
* USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/


#ifndef WFA_DEBUG_H
#define WFA_DEBUG_H

#define WFA_ERR         stderr  /* error: can be redefined to a log file */
#define WFA_OUT         stdout  /* info:  can be redefined to a log file */
#define WFA_WNG         stdout  /* warning: can be redefined to a log file */

#define WFA_DEBUG_DEFAULT          0x0001
#define WFA_DEBUG_ERR              0x0001
#define WFA_DEBUG_INFO             0x0002
#define WFA_DEBUG_WARNING          0x0004

#define WFA_DEBUG 1

#if CONFIG_MTK_COMMON

#include <android/log.h>
#include <time.h>

#ifdef CONFIG_MTK_AP
void dump_wmm_paraters();
#endif

#ifndef ANDROID_LOG_NAME
#define ANDROID_LOG_NAME	"wfa_debug"
#endif /* ANDROID_LOG_NAME */

static void build_timestamp(char *time_buf)
{
	struct timeval val;
	struct tm *tm;

	memset(time_buf, 0, 24);
	gettimeofday(&val, NULL);
	tm = localtime(&val.tv_sec);
	strftime(time_buf, 24, "%F %T", tm);
	snprintf(time_buf+19, 5, ".%03ld", val.tv_usec / 1000);
}

static void alog(int level, const char *format, ...) {
    va_list params;
    int len = strlen(format);
    char fmt[len + 1];
    if (len == 0)
        return;
    memset(fmt, 0, len + 1);
    if (format[0] == '\n')
        memcpy(fmt, format + 1, len--);
    else
        memcpy(fmt, format, len + 1);
    if (fmt[len - 1] == '\n')
        fmt[len - 1] = '\0';
    va_start(params, fmt);
    __android_log_vprint(level, ANDROID_LOG_NAME, fmt, params);
    va_end(params);
}

#define ALOG(level, format, arg...) \
do { \
    int len = strlen(format); \
    char buf[len + 1]; \
    memcpy(buf, format, len); \
    if (buf[len - 1] == '\n') \
        buf[len - 1] = '\0'; \
    else \
        buf[len] = '\0'; \
    __android_log_print(level, ANDROID_LOG_NAME, buf, ##arg); \
} while(0)

#define DPRINT_ERR(_file, _format, arg...) \
do {\
    char time_buf[30]; \
    build_timestamp(time_buf); \
    alog(ANDROID_LOG_ERROR, _format, ##arg); \
    fprintf(_file, "%s File %s, Line %ld: ",time_buf,  __FILE__, (long)__LINE__); \
    fprintf(_file, "%s "_format, time_buf, ##arg); \
    fflush(_file); \
} while (0)

#define DPRINT_INFO(_file, _format, arg...)     \
do {\
    if (wfa_defined_debug & WFA_DEBUG_INFO) {\
        char time_buf[30]; \
        build_timestamp(time_buf); \
        alog(ANDROID_LOG_INFO, _format, ##arg); \
	fprintf(_file, "%s "_format, time_buf, ##arg); \
	fflush(_file); \
    } \
} while (0)

#define DPRINT_WARNING(_file, _format, arg...) \
do {\
    if (wfa_defined_debug & WFA_DEBUG_WARNING) {\
        char time_buf[30]; \
        build_timestamp(time_buf); \
        alog(ANDROID_LOG_WARN, _format, ##arg); \
        fprintf(_file, "%s "_format, time_buf, ##arg); \
        fflush(_file); \
    } \
} while (0)

#else

#define DPRINT_ERR      fprintf(WFA_ERR, "File %s, Line %ld: ", \
                               __FILE__, (long)__LINE__); \
                        fprintf

#define DPRINT_INFO     if(wfa_defined_debug & WFA_DEBUG_INFO) \
                            fprintf

#define DPRINT_WARNING  if(wfa_defined_debug & WFA_DEBUG_WARNING) \
                            fprintf

#endif /* CONFIG_MTK_COMMON */

#endif
