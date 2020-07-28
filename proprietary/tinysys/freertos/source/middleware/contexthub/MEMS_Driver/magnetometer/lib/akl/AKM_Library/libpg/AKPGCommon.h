/******************************************************************************
 *
 *  $Id: AKCommon.h 1032 2013-06-12 09:23:43Z yamada.rj $
 *
 * -- Copyright Notice --
 *
 * Copyright (c) 2004 Asahi Kasei Microdevices Corporation, Japan
 * All Rights Reserved.
 *
 * This software program is the proprietary program of Asahi Kasei Microdevices
 * Corporation("AKM") licensed to authorized Licensee under the respective
 * agreement between the Licensee and AKM only for use with AKM's electronic
 * compass IC.
 *
 * THIS SOFTWARE IS PROVIDED TO YOU "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABLITY, FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT OF
 * THIRD PARTY RIGHTS, AND WE SHALL NOT BE LIABLE FOR ANY LOSSES AND DAMAGES
 * WHICH MAY OCCUR THROUGH USE OF THIS SOFTWARE.
 *
 * -- End Asahi Kasei Microdevices Copyright Notice --
 *
 ******************************************************************************/
#ifndef AKMD_INC_AKCOMMON_H
#define AKMD_INC_AKCOMMON_H

#ifdef WIN32
#include <windows.h>
#include <stdio.h>    //fopen, snprint
#include <string.h>    //strncmp
#define snprintf    _snprintf
#define strlcpy        strncpy

#else
#include <stdio.h>     //frpintf
#include <stdlib.h>    //atoi
#include <string.h>    //memset
#include <unistd.h>
#include <stdarg.h>    //va_list
//#include <utils/Log.h> //LOGV
#include <errno.h>     //errno
#include <sys/stat.h>  //chmod (in FileIO)

#endif


/*** Constant definition ******************************************************/
#if defined(AKMD_AK09911)
//#define AKMD_WIA            0x548
//#define AKMD_ASA            0x80
#endif

/*** Constant definition ******************************************************/
#define AKRET_PROC_SUCCEED      0x00    /*!< The process has been successfully done. */
#define AKRET_OFFSET_OVERFLOW   0x08    /*!< Offset values overflow. */
#define AKRET_VNORM_ERROR       0x40    /*!< AKSC_VNorm error. */
#define AKRET_PROC_FAIL         0x80    /*!< The process failes. */

/* deg x (pi/180.0) */
#define DEG2RAD                 (AKSC_PI / 180.f)
#define RAD2DEG                 (1/DEG2RAD)
#define AKSC2SI(x)              ((AKSC_FLOAT)(((x) * 9.80665f) / 720.0))

/*** Constant definition ******************************************************/
#ifndef ALOGV
#ifdef LOGV
#define ALOGV    LOGV
#else
#define ALOGV    printf
#endif
#endif

#ifndef ALOGD
#ifdef LOGD
#define ALOGD    LOGD
#else
#define ALOGD    printf
#endif
#endif

#ifndef ALOGI
#ifdef LOGI
#define ALOGI    LOGI
#else
#define ALOGI    printf
#endif
#endif

#ifndef ALOGW
#ifdef LOGW
#define ALOGW    LOGW
#else
#define ALOGW    printf
#endif
#endif

#ifndef ALOGE
#ifdef LOGE
#define ALOGE    LOGE
#else
#define ALOGE    printf
#endif
#endif

/*** Constant definition ******************************************************/
#undef LOG_TAG
#define LOG_TAG "AKMPG"
//#define LOG_TAG "AKMD2"

/***** Debug output ******************************************/
#define AKMDEBUG(flag, format, ...)

/***** Error output *******************************************/
#define AKMERROR \
      (ALOGE("%s:%d Error.", __FUNCTION__, __LINE__))

#define AKMERROR_STR(api) \
      (ALOGE("%s:%d %s Error (%s).", \
                __FUNCTION__, __LINE__, (api), strerror(errno)))

#endif //AKMD_INC_AKCOMMON_H

