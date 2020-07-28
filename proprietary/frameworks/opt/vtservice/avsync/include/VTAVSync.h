/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __VTAVSYNC_H__
#define __VTAVSYNC_H__

#include <utils/RefBase.h>
#include <utils/threads.h>
#include <media/stagefright/foundation/ADebug.h>
#include <assert.h>

using namespace android;

namespace VTService {

#define VOLTE_AV_DEV_NAME "/dev/ccci_imsdc"

#define NS_TO_NTP (4.3)
#define US_TO_NTP (4294)

// Hide Debug msg in user/userdebug load
#ifdef __PRODUCTION_RELEASE__
    #define VTAVSYNC_LOGD(...)
#else
    #define VTAVSYNC_LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG  , "AVSync ", ##__VA_ARGS__);
#endif

#define     VTAVSYNC_LOGI(...)  __android_log_print(ANDROID_LOG_INFO   , "AVSync ", ##__VA_ARGS__);
#define     VTAVSYNC_LOGW(...)  __android_log_print(ANDROID_LOG_WARN   , "AVSync ", ##__VA_ARGS__);
#define     VTAVSYNC_LOGE(...)  __android_log_print(ANDROID_LOG_ERROR  , "AVSync ", ##__VA_ARGS__);

#define VTAVSYNC_ASSERT(expr, M, ...) while(!(expr)) { VTAVSYNC_LOGE("@@@ ASSERT @@@: (%s:%d)" M, __FILE__, __LINE__, ##__VA_ARGS__); fflush(stdout); LOG_ALWAYS_FATAL("ASSERT!!!!"); assert(0); exit(-11);}

    typedef enum av_msg_type{
        AV_ZERO = 0,
        AV_UL_NTP,
        AV_DL_AUDIO_NTP,
        AV_DL_AUDIO_RESET,
        AV_MAX = 0xFFFFFFFF
    }av_msg_e;

    typedef struct av_msg{
        av_msg_e type; /* rtp or rtcp */
        unsigned integer;
        unsigned fraction;
    } av_msg_t;

    typedef struct av_param {
        /*System Time*/
        int64_t  st;
        /*NTP in nano second*/
        uint64_t nsNTP;
    }av_param_t;

    class VTAVSync : public RefBase {
        public:
            VTAVSync();
            virtual ~VTAVSync();
            static void getDLAudioTimeStamp(int64_t *pAudioTime);
        private:
            void resetParam(void);
            int openFD(void);
            int closeFD(void);
            int writeFD(int fd, void *buffer, int size);
            void checkFD(void);
            void HandleDLAVSync(unsigned char* buffer);
            void avInit(void);
            static void *downlinkThread(void *arg);

            static av_param_t mAV;
    };

}
#endif
