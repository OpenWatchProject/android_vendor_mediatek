/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "CDN_HAL"

#include "cdn_hal_imp.h"
#include <mtkcam/utils/std/Log.h>

using namespace StereoHAL;

void
CDN_HAL_IMP::__logParams()
{
    if(!LOG_ENABLED) {
        return;
    }

    MY_LOGD("===== CDN Params =====");
    MY_LOGD("isRotate:[%d]", __runtimeParams.isRotate);
    MY_LOGD("mode:[%d]", __runtimeParams.mode);
    MY_LOGD("var:[%d]", __runtimeParams.var);
    for(int i = 0; i < WARPING_MATRIX_SIZE ; i++){
        MY_LOGD("Trans(%d):[%d]", i, __runtimeParams.Trans[i]);
    }
    MY_LOGD("width:[%d]", __runtimeParams.width);
    MY_LOGD("height:[%d]", __runtimeParams.height);
    MY_LOGD("======================");
}

void
CDN_HAL_IMP::__logImages()
{
    if(!LOG_ENABLED) {
        return;
    }

    MY_LOGD("===== CDN Images =====");
    MY_LOGD("width:[%d]", __runtimeImages.width);
    MY_LOGD("height:[%d]", __runtimeImages.height);
    MY_LOGD("MonoYUV:[%p]", __runtimeImages.MonoYUV);
    MY_LOGD("BayerYUV:[%p]", __runtimeImages.BayerYUV);
    MY_LOGD("output:[%p]", __runtimeImages.output);
    MY_LOGD("======================");
}