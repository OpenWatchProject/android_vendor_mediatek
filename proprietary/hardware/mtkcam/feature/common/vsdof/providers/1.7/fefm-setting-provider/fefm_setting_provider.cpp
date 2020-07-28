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
#define LOG_TAG "FEFMSettingProvider"

#include <algorithm>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <fefm_setting_provider.h>

#include <mtkcam/drv/IHalSensor.h>
#include <camera_custom_stereo.h>       // For CUST_STEREO_* definitions.
#include <mtkcam/utils/std/Log.h>

#include <cutils/properties.h>
#include <mtkcam/aaa/IHal3A.h>

#define CHECK_FREQ_FF 5 //Check trigger every 5 frames

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

#define FUNC_START          MY_LOGD_IF(LOG_ENABLED, "+")
#define FUNC_END            MY_LOGD_IF(LOG_ENABLED, "-")

using namespace NSCam;
using namespace android;
using namespace NSCam::v1::Stereo;
using namespace StereoHAL;
using namespace NS3Av3;

Mutex FEFMSettingProvider::__Lock;
FEFMSettingProvider *FEFMSettingProvider::__instance = NULL;
MTKFEFMTrigger *FEFMSettingProvider::__triggerInstance= NULL;

FEFMSettingProvider *
FEFMSettingProvider::getInstance()
{
    Mutex::Autolock lock(__Lock);

    if(NULL == __instance) {
        __instance = new FEFMSettingProvider();
        __triggerInstance = MTKFEFMTrigger::getInstance();
    }

    return __instance;
}

void
FEFMSettingProvider::destroyInstance()
{
    Mutex::Autolock lock(__Lock);

    if(__instance) {
        delete __instance;
        __instance = NULL;
    }

    if(__triggerInstance) {
        MTKFEFMTrigger::destroyInstance();
        __triggerInstance = NULL;
    }
}

void
FEFMSettingProvider::init(int main1Index, int main2Index)
{
    Mutex::Autolock lock(__Lock);

    int sensorIndex[2] = {main1Index, main2Index};

    for(int i = 0; i < 2; i++) {
        if(sensorIndex[i] != __sensorIndex[i]) {
            __sensorIndex[i] = sensorIndex[i];
            //Get new AF info and table
            __isAF[i] = false;

            IHal3A *pHal3A = MAKE_Hal3A(sensorIndex[i], LOG_TAG);
            if(NULL == pHal3A) {
                MY_LOGE("Cannot get 3A HAL of sensor %d", sensorIndex[i]);
                continue;
            } else {
                FeatureParam_T rFeatureParam;
                if(pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetSupportedInfo, (MUINTPTR)&rFeatureParam, 0)) {
                    __isAF[i] = (rFeatureParam.u4MaxFocusAreaNum > 0);
                    if(__isAF[i]) {
                        pHal3A->send3ACtrl(NS3Av3::E3ACtrl_GetAFDAFTable, (MUINTPTR)&__pAFTable[i], 0);
                        if(NULL == __pAFTable[i]) {
                            MY_LOGE("[AF] Cannot get AF table of sensor %d", sensorIndex[i]);
                        }
                    } else {
                        __pAFTable[i] = NULL;
                        if(0 == i) {
                            __triggerInstance->FEFMTrigger_set_reqular_trigger(::property_get_int32("vendor.STEREO.fefm_freq", CHECK_FREQ_FF));
                        }
                    }
                }

                pHal3A->destroyInstance(LOG_TAG);
            }
        }
    }

    MY_LOGD("Is AF: Main: %d Main2: %d", __isAF[0], __isAF[1]);
    __triggerInstance->FEFMTrigger_init(__isAF[0], __isAF[1]);
}

bool
FEFMSettingProvider::needToRunFEFM(int main1MagicNumber, int main2MagicNumber, bool isCapture)
{
    DAF_VEC_STRUCT *pMain1AfVec = (__pAFTable[0]) ? &__pAFTable[0]->daf_vec[main1MagicNumber%DAF_TBL_QLEN] : NULL;
    DAF_VEC_STRUCT *pMain2AfVec = (__pAFTable[1]) ? &__pAFTable[1]->daf_vec[main2MagicNumber%DAF_TBL_QLEN] : NULL;
    bool isValidMagicNumber = (pMain1AfVec) ? (pMain1AfVec->frm_mun == main1MagicNumber) : true;
    bool needTrigger = (isCapture ||
                        (isValidMagicNumber &&
                         __triggerInstance->FEFMTrigger_query(
                             main1MagicNumber, main2MagicNumber,
                             (pMain1AfVec) ? pMain1AfVec->is_af_stable : true,
                             (pMain2AfVec) ? pMain2AfVec->is_af_stable : true
                         )));

    MY_LOGD_IF(StereoSettingProvider::isLogEnabled(),
               "MagicNumber %d, %d, isCapture %d, isValidMagicNumber %d, AF frm_mun %d, magicNumber %d, AF stable %d, %d need trigger %d",
               main1MagicNumber, main2MagicNumber,  isCapture, isValidMagicNumber,
               (pMain1AfVec) ? pMain1AfVec->frm_mun : -1, main1MagicNumber,
               (pMain1AfVec) ? pMain1AfVec->is_af_stable : true,
               (pMain2AfVec) ? pMain2AfVec->is_af_stable : true,
               needTrigger);

    return needTrigger;
}

FEFMSettingProvider::FEFMSettingProvider()
{

}

FEFMSettingProvider::~FEFMSettingProvider()
{

}
