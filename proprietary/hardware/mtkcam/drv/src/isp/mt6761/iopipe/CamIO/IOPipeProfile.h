/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_PROFILE_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_PROFILE_H_

#include <utils/threads.h>

#include "camio_log.h"

/******************************************************************************
 *
 ******************************************************************************/

#if (IOPIPE_ENABLE_SYSTRACE == 1) || (IOPIPE_ENABLE_MET_PROFILE == 1)
    #define CONFIG_IOPIPE_PROFILING         (1)
#else
    #define CONFIG_IOPIPE_PROFILING         (0)
#endif

/* For enable MET profiling, user need to set IOPIPE_ENABLE_MET_PROFILE=yes
 * in the following both mk files:
 *      drv/src/isp/<platform>/iopipe/Android.mk
 *      drv/src/isp/<platform>/iopipe/CamIO/Android.mk
 */
#if (IOPIPE_ENABLE_MET_PROFILE == 1)
#include "met_tag.h"
#endif


typedef enum _IOPIPE_TRACE_EN_MODE {
    IOPIPE_TRACE_EN_NOMALPIPE = 0,
    IOPIPE_TRACE_EN_STTPIPE = 1,
    IOPIPE_TRACE_EN_CAMSVPIPE = 2,
} IOPIPE_TRACE_EN_MODE;


/******************************************************************************
 *
 ******************************************************************************/

#if (CONFIG_IOPIPE_PROFILING == 1)
    #define ATRACE_TAG                  ATRACE_TAG_ALWAYS//must before include Trace.h
    #include <utils/Trace.h>
    /* #define PIPE_TRACE_CALL()           ATRACE_CALL()*/
    /* #define PIPE_TRACE_NAME(name)       ATRACE_NAME(name) */

    #if (IOPIPE_ENABLE_SYSTRACE == 1)
        #define _TRACE_BEGIN(name, bit)     do { \
                if (IOPipeTrace::m_traceEnable & (1 << bit)) { ATRACE_BEGIN(name); } \
            } while(0)
        #define _TRACE_END(name, bit)       do { \
                if (IOPipeTrace::m_traceEnable & (1 << bit)) { ATRACE_END(); } \
            } while(0)
    #else
        #define _TRACE_BEGIN(name, bit)
        #define _TRACE_END(name, bit)
    #endif

    #if (IOPIPE_ENABLE_MET_PROFILE == 1)
        #define _METTAG_BEGIN(name, bit)     do { \
                if (IOPipeTrace::m_metPrfEnable & (1 << bit)) { met_tag_start(0, name); } \
            } while(0)
        #define _METTAG_END(name, bit)       do { \
                if (IOPipeTrace::m_metPrfEnable & (1 << bit)) { met_tag_end(0, name); } \
            } while(0)
    #else
        #define _METTAG_BEGIN
        #define _METTAG_END(name, bit)
    #endif

    #define PIPE_TRACE_BEGIN(name)      do { _TRACE_BEGIN(name, IOPIPE_TRACE_EN_NOMALPIPE); _METTAG_BEGIN(name, IOPIPE_TRACE_EN_NOMALPIPE); } while(0)
    #define PIPE_TRACE_END(name)        do { _TRACE_END(name, IOPIPE_TRACE_EN_NOMALPIPE); _METTAG_END(name, IOPIPE_TRACE_EN_NOMALPIPE); } while(0)

    #define STTPIPE_TRACE_BEGIN(name)   do { _TRACE_BEGIN(name, IOPIPE_TRACE_EN_STTPIPE); _METTAG_BEGIN(name, IOPIPE_TRACE_EN_STTPIPE); } while(0)
    #define STTPIPE_TRACE_END(name)     do { _TRACE_END(name, IOPIPE_TRACE_EN_STTPIPE); _METTAG_END(name, IOPIPE_TRACE_EN_STTPIPE); } while(0)

    #define CAMSV_TRACE_BEGIN(name)     do { _TRACE_BEGIN(name, IOPIPE_TRACE_EN_CAMSVPIPE); _METTAG_BEGIN(name, IOPIPE_TRACE_EN_CAMSVPIPE); } while(0)
    #define CAMSV_TRACE_END(name)       do { _TRACE_END(name, IOPIPE_TRACE_EN_CAMSVPIPE); _METTAG_END(name, IOPIPE_TRACE_EN_CAMSVPIPE); } while(0)


    class IOPipeTrace {
    public:
        IOPipeTrace (const char *pTagName, IOPIPE_TRACE_EN_MODE bit) {
            m_pTagName = pTagName;
            m_traceBit = bit;

            #if (IOPIPE_ENABLE_MET_PROFILE == 1)
            if (m_metPrfEnable & (1 << m_traceBit)) {
                met_tag_start(0, m_pTagName);
            }
            #endif
            #if (IOPIPE_ENABLE_SYSTRACE == 1)
            if (m_traceEnable & (1 << m_traceBit)) {
                ATRACE_BEGIN(m_pTagName);
            }
            #endif
        }
        ~IOPipeTrace () {
            #if (IOPIPE_ENABLE_SYSTRACE == 1)
            if (m_traceEnable & (1 << m_traceBit)) {
                ATRACE_END();
            }
            #endif
            #if (IOPIPE_ENABLE_MET_PROFILE == 1)
            if (m_metPrfEnable & (1 << m_traceBit)) {
                met_tag_end(0, m_pTagName);
            }
            #endif
        }

        static MINT32 createTrace(void) {
            Mutex::Autolock _lock(m_traceLock);
            char *value;
            MINT32 ret = 0;

            if (m_userCount > 0) {
                m_userCount++;
                return 0;
            }

            value = (char *)malloc(sizeof(char) * PROPERTY_VALUE_MAX);

            if (value == NULL) {
                return -1;
            }
            else {
                property_get("vendor.debuglog.iopipe.systrace", value, "0");
                m_traceEnable = atoi(value);

                property_get("vendor.debuglog.iopipe.met_prof", value, "0");
                m_metPrfEnable = atoi(value);

                free(value);
                value = NULL;
            }

            #if (IOPIPE_ENABLE_MET_PROFILE == 1)
            if (m_metPrfEnable && (m_userCount == 0)) {
                /* 1st user, potential user: NormalPipe, Camsv */
                if (met_tag_init() < 0) {
                    return -2;
                }
            }
            #endif

            if (m_traceEnable > 0) {
                ret |= 0x1;
            }
            if (m_metPrfEnable > 0) {
                ret |= 0x2;
            }

            m_userCount++;

            return ret;
        }

        static MINT32 destroyTrace(void) {
            Mutex::Autolock _lock(m_traceLock);

            if (m_userCount == 0) {
                return -1;
            }

            #if (IOPIPE_ENABLE_MET_PROFILE == 1)
            if (m_metPrfEnable && (m_userCount == 1)) {
                /* last user, potential user: NormalPipe, Camsv */
                met_tag_uninit();
            }
            #endif

            m_userCount--;

            return 0;
        }

        const char      *m_pTagName;
        IOPIPE_TRACE_EN_MODE m_traceBit;

        static MUINT32  m_traceEnable;
        static MUINT32  m_metPrfEnable;
        static MUINT32  m_userCount;
        static Mutex    m_traceLock; /* Protects m_userCount */
    };

#else
    #define PIPE_TRACE_BEGIN(name)
    #define PIPE_TRACE_END(name)

    #define PIPE_TRACE_BEGIN(name)
    #define PIPE_TRACE_END(name)

    #define STTPIPE_TRACE_BEGIN(name)
    #define STTPIPE_TRACE_END(name)

    #define CAMSV_TRACE_BEGIN(name)
    #define CAMSV_TRACE_END(name)

    class IOPipeTrace {
    public:
        IOPipeTrace (const char *, IOPIPE_TRACE_EN_MODE) {}
        ~IOPipeTrace () {}
        static MINT32 createTrace(void) { return 0; }
        static MINT32 destroyTrace(void) { return 0; }
    };
#endif // (CONFIG_IOPIPE_PROFILING == 1)



#endif //_MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_PROFILE_H_


