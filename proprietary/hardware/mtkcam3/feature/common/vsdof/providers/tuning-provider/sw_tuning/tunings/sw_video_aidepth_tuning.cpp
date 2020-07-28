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
#define LOG_TAG "StereoTuningProvider_SW_VideoAIDEPTH"

#include <mtkcam/utils/std/ULog.h>
CAM_ULOG_DECLARE_MODULE_ID(MOD_MULTICAM_PROVIDER);
#include "sw_video_aidepth_tuning.h"

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

SW_VIDEO_AIDEPTHTuning::SW_VIDEO_AIDEPTHTuning(json &tuningJson)
{
    _init(tuningJson);
}

SW_VIDEO_AIDEPTHTuning::~SW_VIDEO_AIDEPTHTuning()
{
}

bool
SW_VIDEO_AIDEPTHTuning::retrieveTuningParams(TuningQuery_T &query)
{
    int *coreNumber = (int *)query.results[QUERY_KEY_CORE_NUMBER];
    TUNING_PAIR_LIST_T *params = (TUNING_PAIR_LIST_T *)query.results[QUERY_KEY_TUNING_PARAMS];

    if(NULL == params) {
        MY_LOGE("Cannot get %s", QUERY_KEY_TUNING_PARAMS);
        return false;
    }

    *coreNumber = __tuning.coreNumber;
    *params     = __tuning.params;

    return true;
}

void
SW_VIDEO_AIDEPTHTuning::_initDefaultValues()
{
}

void
SW_VIDEO_AIDEPTHTuning::log(FastLogger &logger, bool inJSON)
{
    if(inJSON) {
        return StereoTuningBase::log(logger, inJSON);
    }

    logger
    .FastLogD("======== Video AIDepth Parameters ========")
    .FastLogD("CoreNumber: %d", __tuning.coreNumber);

    for(auto &param : __tuning.params) {
        logger.FastLogD("\"%s\": %d", param.first.c_str(), param.second);
    }

    logger.print();
}

void
SW_VIDEO_AIDEPTHTuning::_loadValuesFromDocument(const json& aidepthValues)
{
    const char *TUNING_PREFIX = "videoaidepth.";
    const size_t TUNING_PREFIX_SIZE = strlen(TUNING_PREFIX);

    for(auto &aidepthValue : aidepthValues) {
        const json &tuningValues = aidepthValue[VALUE_KEY_VALUES];
        __tuning.coreNumber = tuningValues[QUERY_KEY_CORE_NUMBER].get<int>();
        if(__tuning.coreNumber < 1) {
            __tuning.coreNumber = 1;
        }

        //Extract tuning params
        __tuning.params.clear();
        for(json::const_iterator it = tuningValues.begin(); it != tuningValues.end(); ++it) {
            if(it.key().length() > TUNING_PREFIX_SIZE &&
               !strncmp(it.key().c_str(), TUNING_PREFIX, TUNING_PREFIX_SIZE))
            {
                __tuning.params.push_back({it.key(), it.value().get<int>()});
            }
        }
    }
}
