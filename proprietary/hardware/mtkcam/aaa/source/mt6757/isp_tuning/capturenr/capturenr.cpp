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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "capturenr"
//
/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/utils/std/ULog.h>
CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_SWNR);
#define MY_LOGV(fmt, arg...)    CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/def/common.h>
//
#include <utils/StrongPointer.h>
using namespace android;
//
//
#include <capturenr.h>
#include <libnr/MTKNR.h>
//
#include <utils/Mutex.h>

#include <sys/mman.h>
//
#include <camera_custom_capture_nr.h>
// custom tuning
#include <mtkcam/drv/IHalSensor.h>
#include <camera_custom_nvram.h>
#include <nvbuf_util.h>
//
#include <debug_exif/ver1/dbg_id_param.h>
#include <dbg_cam_param.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <aaa_types.h>
#include <mtkcam/utils/std/Log.h>
#include <isp_tuning/isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <isp_tuning/isp_tuning_custom_swnr.h>
#include <isp_interpolation.h>
//
#include <cutils/properties.h>
#include "paramctrl.h"
//
using namespace NSCam;
using namespace NSIspTuningv3;
/*******************************************************************************
*
********************************************************************************/
// these assertions are to make sure nvram is large enough
static_assert(
        sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) >= sizeof(SWNR_PARAM) + sizeof(FSWNR_PARAM),
        "should check NVRAM_CAMERA_FEATURE_SWNR_STRUCT"
        );

static_assert(
        sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) >=
        (sizeof(NRTuningInfo) + offsetof(NR2TuningInfo, reserved) +
        2 * (sizeof(CCRTuningInfo) + sizeof(HFGTuningInfo))),
        "[FIXME] should modify swnr nvram struct"
        );

static_assert(
        sizeof(NR_PARAM) == sizeof(NRTuningInfo),
        "[FIXME] struct size is not correct"
        );

static_assert(
        sizeof(NR2_PARAM) == sizeof(NR2TuningInfo) - member_size(NR2TuningInfo, reserved),
        "[FIXME] struct size is not correct"
        );

static_assert(
        sizeof(HFG_PARAM) == sizeof(HFGTuningInfo),
        "[FIXME] struct size is not correct"
        );

static_assert(
        sizeof(CCR_PARAM) == sizeof(CCRTuningInfo),
        "[FIXME] struct size is not correct"
        );

#if 0 //get the size of NVRAM_CAMERA_FEATURE_STRUCT
template<int s> struct test;
test<sizeof(NVRAM_CAMERA_FEATURE_STRUCT)> tt;
#endif

/*******************************************************************************
*
********************************************************************************/
class SwNRParam
{
    public:
        struct Params
        {
            MUINT32 iso;
            MBOOL   isMfll;
            MBOOL   isDualCam;
        };

    public:
        SwNRParam(MUINT32 openId);
        ~SwNRParam();

    public:
        MVOID      setParam(Params const& rParam) { mParams = rParam; }
        MINT32     getNRType() const { return mNRType; }
        MBOOL      getTuning(NRTuningInfo* pNR, HFGTuningInfo* pHFG, CCRTuningInfo* pCCR);
        MBOOL      getTuning(NR2TuningInfo* pNR, HFGTuningInfo* pHFG, CCRTuningInfo* pCCR);
        //
        MBOOL      update();
        //
        // dump params
        MBOOL      dumpParamsToFile(char * const filename);

    protected:

        MBOOL      getTuningFromNvram(
                        MUINT32 scenario,
                        MUINT32 idx,
                        MVOID** ppNvram
                   );
        MUINT32     getIso() const { return mParams.iso; }
        MUINT32 getScenario() const;
        const char* getScenarioString() const;

    private:
        MUINT32 const   mOpenId;
        Params          mParams;
        //
        MINT32          mNRType;
        //
        MVOID*          mpNvramData;
};

const char* SwNRParam::getScenarioString() const
{
    if (mParams.isMfll) return "MFNR";
    if (mParams.isDualCam) return "DUAL";
    return "NORM";
}

MUINT32 SwNRParam::getScenario() const
{
    return *(MUINT32*)(SwNRParam::getScenarioString());
}


/*******************************************************************************
*
********************************************************************************/
inline MINT32 generateSeed() { return (MINT32)::systemTime(); }

#define DUMP_PARAM(fp, fmt, arg...)            \
    do {                                       \
        if( fp ) fprintf(fp, fmt "\n", ##arg); \
        else MY_LOGD(fmt, ##arg);              \
    } while(0)

MVOID
dumpParams(NRTuningInfo* pNR, FILE* fp = NULL)
{
    if( ! pNR ) {
        MY_LOGE("null");
        return;
    }
#define DUMP_NR(param)                               \
    do {                                             \
        DUMP_PARAM(fp, "%s:%d", #param, pNR->param); \
    } while(0)
    DUMP_NR(ANR_Y_LUMA_SCALE_RANGE);
    DUMP_NR(ANR_C_CHROMA_SCALE);
    DUMP_NR(ANR_Y_SCALE_CPY0);
    DUMP_NR(ANR_Y_SCALE_CPY1);
    DUMP_NR(ANR_Y_SCALE_CPY2);
    DUMP_NR(ANR_Y_SCALE_CPY3);
    DUMP_NR(ANR_Y_SCALE_CPY4);
    DUMP_NR(ANR_Y_CPX1);
    DUMP_NR(ANR_Y_CPX2);
    DUMP_NR(ANR_Y_CPX3);
    DUMP_NR(ANR_CEN_GAIN_LO_TH);
    DUMP_NR(ANR_CEN_GAIN_HI_TH);
    DUMP_NR(ANR_PTY_GAIN_TH);
    DUMP_NR(ANR_KSIZE_LO_TH);
    DUMP_NR(ANR_KSIZE_HI_TH);
    DUMP_NR(ANR_KSIZE_LO_TH_C);
    DUMP_NR(ANR_KSIZE_HI_TH_C);
    DUMP_NR(ITUNE_ANR_PTY_STD);
    DUMP_NR(ITUNE_ANR_PTU_STD);
    DUMP_NR(ITUNE_ANR_PTV_STD);
    DUMP_NR(ANR_ACT_TH_Y);
    DUMP_NR(ANR_ACT_BLD_BASE_Y);
    DUMP_NR(ANR_ACT_BLD_TH_Y);
    DUMP_NR(ANR_ACT_SLANT_Y);
    DUMP_NR(ANR_ACT_TH_C);
    DUMP_NR(ANR_ACT_BLD_BASE_C);
    DUMP_NR(ANR_ACT_BLD_TH_C);
    DUMP_NR(ANR_ACT_SLANT_C);
    DUMP_NR(RADIUS_H);
    DUMP_NR(RADIUS_V);
    DUMP_NR(RADIUS_H_C);
    DUMP_NR(RADIUS_V_C);
    DUMP_NR(ANR_PTC_HGAIN);
    DUMP_NR(ANR_PTY_HGAIN);
    DUMP_NR(ANR_LPF_HALFKERNEL);
    DUMP_NR(ANR_LPF_HALFKERNEL_C);
    DUMP_NR(ANR_ACT_MODE);
    DUMP_NR(ANR_LCE_SCALE_GAIN);
    DUMP_NR(ANR_LCE_C_GAIN);
    DUMP_NR(ANR_LCE_GAIN0);
    DUMP_NR(ANR_LCE_GAIN1);
    DUMP_NR(ANR_LCE_GAIN2);
    DUMP_NR(ANR_LCE_GAIN3);
    DUMP_NR(ANR_MEDIAN_LOCATION);
    DUMP_NR(ANR_CEN_X);
    DUMP_NR(ANR_CEN_Y);
    DUMP_NR(ANR_R1);
    DUMP_NR(ANR_R2);
    DUMP_NR(ANR_R3);
    DUMP_NR(LUMA_ON_OFF);
#undef DUMP_NR
}


MVOID
dumpParams(NR2TuningInfo* pNR, FILE* fp = NULL)
{
    if( ! pNR ) {
        MY_LOGE("null");
        return;
    }
#define DUMP_NR2(param)                              \
    do {                                             \
        DUMP_PARAM(fp, "%s:%d", #param, pNR->param); \
    } while(0)
    DUMP_NR2(NR_K);
    DUMP_NR2(NR_S);
    DUMP_NR2(NR_SD);
    DUMP_NR2(NR_BLD_W);
    DUMP_NR2(NR_BLD_TH);
    DUMP_NR2(NR_SMTH);
#undef DUMP_NR2
}


MVOID
dumpParams(HFGTuningInfo* pHFG, FILE* fp = NULL)
{
    if( ! pHFG ) {
        MY_LOGE("null");
        return;
    }
#define DUMP_HFG(param)                               \
    do {                                              \
        DUMP_PARAM(fp, "%s:%d", #param, pHFG->param); \
    } while(0)
    DUMP_HFG(HFG_ENABLE);
    DUMP_HFG(HFG_GSD);
    DUMP_HFG(HFG_SD0);
    DUMP_HFG(HFG_SD1);
    DUMP_HFG(HFG_SD2);
    DUMP_HFG(HFG_TX_S);
    DUMP_HFG(HFG_LCE_LINK_EN);
    DUMP_HFG(HFG_LUMA_CPX1);
    DUMP_HFG(HFG_LUMA_CPX2);
    DUMP_HFG(HFG_LUMA_CPX3);
    DUMP_HFG(HFG_LUMA_CPY0);
    DUMP_HFG(HFG_LUMA_CPY1);
    DUMP_HFG(HFG_LUMA_CPY2);
    DUMP_HFG(HFG_LUMA_CPY3);
    DUMP_HFG(HFG_LUMA_SP0);
    DUMP_HFG(HFG_LUMA_SP1);
    DUMP_HFG(HFG_LUMA_SP2);
    DUMP_HFG(HFG_LUMA_SP3);
#undef DUMP_HFG
}


MVOID
dumpParams(CCRTuningInfo* pCCR, FILE* fp = NULL)
{
    if( ! pCCR ) {
        MY_LOGE("null");
        return;
    }
#define DUMP_CCR(param)                               \
    do {                                              \
        DUMP_PARAM(fp, "%s:%d", #param, pCCR->param); \
    } while(0)
    DUMP_CCR(CCR_ENABLE);
    DUMP_CCR(CCR_CEN_U);
    DUMP_CCR(CCR_CEN_V);
    DUMP_CCR(CCR_Y_CPX1);
    DUMP_CCR(CCR_Y_CPX2);
    DUMP_CCR(CCR_Y_CPY1);
    DUMP_CCR(CCR_Y_SP1);
    DUMP_CCR(CCR_UV_X1);
    DUMP_CCR(CCR_UV_X2);
    DUMP_CCR(CCR_UV_X3);
    DUMP_CCR(CCR_UV_GAIN1);
    DUMP_CCR(CCR_UV_GAIN2);
    DUMP_CCR(CCR_UV_GAIN_SP1);
    DUMP_CCR(CCR_UV_GAIN_SP2);
    DUMP_CCR(CCR_Y_CPX3);
    DUMP_CCR(CCR_Y_CPY0);
    DUMP_CCR(CCR_Y_CPY2);
    DUMP_CCR(CCR_Y_SP0);
    DUMP_CCR(CCR_Y_SP2);
    DUMP_CCR(CCR_UV_GAIN_MODE);
    DUMP_CCR(CCR_MODE);
    DUMP_CCR(CCR_OR_MODE);
    DUMP_CCR(CCR_HUE_X1);
    DUMP_CCR(CCR_HUE_X2);
    DUMP_CCR(CCR_HUE_X3);
    DUMP_CCR(CCR_HUE_X4);
    DUMP_CCR(CCR_HUE_SP1);
    DUMP_CCR(CCR_HUE_SP2);
    DUMP_CCR(CCR_HUE_GAIN1);
    DUMP_CCR(CCR_HUE_GAIN2);
#undef DUMP_CCR
}
#undef DUMP_PARAM

SwNRParam::
SwNRParam(MUINT32 openId)
    : mOpenId(openId)
{
    mpNvramData = (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)malloc(sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT));
    mParams.iso = 0;
    mParams.isMfll = 0;
    mNRType = eSWNRType_SW;
}


SwNRParam::
~SwNRParam()
{
    if( mpNvramData )
        free(mpNvramData);
}


MBOOL
SwNRParam::
getTuningFromNvram(MUINT32 scenario, MUINT32 idx, MVOID** ppNvram)
{
    if( idx >= eNUM_OF_ISO_IDX )
    {
        MY_LOGE("wrong nvram idx %d", idx);
        return MFALSE;
    }

    // load some setting from nvram
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT* pNvram;
    MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(mOpenId);
    //
    err = NvBufUtil::getInstance().getBufAndRead(
            CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)pNvram);
    if( err != 0 )
    {
        MY_LOGE("getBufAndRead fail, err=%d", err);
        return MFALSE;
    }

    if (scenario==*(MUINT32*)"MFNR") {
        *ppNvram = &(pNvram->nr_mfll[idx]);
#if (MTKCAM_STEREO_DENOISE_SUPPORT == '1')
    } else if (scenario==*(MUINT32*)"DUAL") {
        *ppNvram = &(pNvram->nr_dual[idx]);
#endif
    } else {
        *ppNvram = &(pNvram->nr_normal[idx]);
    }

    return MTRUE;
}

static MUINT32 getNeighber(Paramctrl* pParamctrl, MUINT32 iso, MUINT32& iso_low, MUINT32& iso_high)
{
    MUINT32 idx = (MUINT32)pParamctrl->map_ISO_value2index(iso);
    // find lower and upper bound
    {
        MUINT32 iso2;
        iso2 = pParamctrl->map_ISO_index2value((EIndex_ISO_T)idx);
        if (iso2 > iso) {
            if (idx > 0) {
                idx -= 1;
                iso_low = pParamctrl->map_ISO_index2value((EIndex_ISO_T)idx);
                iso_high = iso2;
            } else {
                iso_low = iso_high = iso2;
            }
        } else {
            iso_low = iso2;
            iso_high = pParamctrl->map_ISO_index2value((EIndex_ISO_T)(idx + 1));
        }
    }
    return idx;
}

MBOOL
SwNRParam::
update()
{
    class ScopedHelper
    {
    public:
                ScopedHelper(MINT32 const openId) : pIspTuningCustom(NULL) {
                    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
                    sensorDev = pHalSensorList->querySensorDevIdx(openId);
                    MUINT32 u4SensorID;
                    {
                        SensorStaticInfo rSensorStaticInfo;
                        switch  ( sensorDev )
                        {
                            case SENSOR_DEV_MAIN:
                                pHalSensorList->querySensorStaticInfo(
                                        NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
                                break;
                            case SENSOR_DEV_SUB:
                                pHalSensorList->querySensorStaticInfo(
                                        NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
                                break;
                            case SENSOR_DEV_MAIN_2:
                                pHalSensorList->querySensorStaticInfo(
                                        NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
                                break;
                            default:    //  Shouldn't happen.
                                CAM_ULOGME("Invalid sensor device: %d", sensorDev);
                                rSensorStaticInfo.sensorDevID = 0;
                                break;
                        }
                        u4SensorID = rSensorStaticInfo.sensorDevID;
                        MY_LOGD("sensorDev(%d), u4SensorID(%d)", sensorDev, u4SensorID);
                    }
                    //
                    pIspTuningCustom = IspTuningCustom::createInstance(
                            static_cast<NSIspTuning::ESensorDev_T>(sensorDev), u4SensorID
                            );
                }
                ~ScopedHelper() {
                    if( pIspTuningCustom ) pIspTuningCustom->destroyInstance();
                }
    public:
        IspTuningCustom*    get() const { return pIspTuningCustom; };
        MUINT               getSensorDev() const { return sensorDev; }

    private:
        MUINT               sensorDev;
        IspTuningCustom*    pIspTuningCustom;

    } ispCustom(mOpenId);
    //
    if( ! ispCustom.get() ) {
        MY_LOGE("cannnot get ispTuningCustom");
        return MFALSE;
    }
    //
    // 1. update swnr type
    mNRType = get_swnr_type(ispCustom.getSensorDev());
    //
    { // debug
        MINT32 type = property_get_int32("vendor.debug.swnr.type", -1);
        if( type != -1 ) {
            MY_LOGD("force use swnr: %s", type == eSWNRType_SW ? "normal" : "fast");
            mNRType = type;
        }
    }
    //
    if( mNRType < 0 || mNRType >= eSWNRType_NUM ) {
        MY_LOGE("incorrect swnr type %d", mNRType);
        return MFALSE;
    }
    //

    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    MUINT sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    Paramctrl* pParamctrl = Paramctrl::getInstance((ESensorDev_T)sensorDev, mOpenId);
    MUINT32 iso, iso_low, iso_high;
    MUINT32 idx;

    iso = ispCustom.get()->remap_ISO_value(getIso());
    idx = getNeighber(pParamctrl, iso, iso_low, iso_high);

    MY_LOGD("%d: iso=%d (%d~%d), isMfll=%d", mOpenId, iso, iso_low, iso_high, getScenario());


    // 2. read from nvram & smooth
    if( is_to_invoke_swnr_interpolation(getScenario(), iso) && (iso_low != iso_high))
    {
        MY_LOGD("Interpolation of Nvram tuning data");
        //
        MVOID* pNvramLow = NULL;
        MVOID* pNvramHigh = NULL;
        if( !getTuningFromNvram(getScenario(), idx, &pNvramLow) ||
            !getTuningFromNvram(getScenario(), idx + 1, &pNvramHigh) )
        {
            MY_LOGE("update from nvram failed");
            return MFALSE;
        }
        //
        // interpolation
        auto interpolation_func = (mNRType == eSWNRType_SW) ? SmoothSWNR : SmoothFSWNR;
        interpolation_func(
                iso,
                iso_high, iso_low,
                *reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pNvramHigh),
                *reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pNvramLow),
                *reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(mpNvramData)
                );
    }
    else
    {
        // get nvram
        MVOID* pTuningNvram = NULL;
        if( !getTuningFromNvram(getScenario(), idx, &pTuningNvram) )
        {
            MY_LOGE("update from nvram failed");
            return MFALSE;
        }

        // update
        memcpy(mpNvramData, pTuningNvram, sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) );
    }

    pParamctrl->destroyInstance();
    return MTRUE;
}


MBOOL
SwNRParam::
getTuning(NRTuningInfo* pNR, HFGTuningInfo* pHFG, CCRTuningInfo* pCCR)
{
    if( ! pNR || ! pHFG || ! pCCR ) {
        MY_LOGE("pNR(%p), pHFG(%p), pCCR(%p)", pNR, pHFG, pCCR );
        return MFALSE;
    }
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram = (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)mpNvramData;
    SWNR_PARAM* pPARAM = &(pNvram->swnr);
    //
    memcpy(pNR , &(pPARAM->NR) , sizeof(NR_PARAM));
    memcpy(pHFG, &(pPARAM->HFG), sizeof(HFG_PARAM));
    memcpy(pCCR, &(pPARAM->CCR), sizeof(CCR_PARAM));
    //
    // update some parameters
    pHFG->HFG_GSD = generateSeed();
    //
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug ) {
            dumpParams(pNR);
            dumpParams(pHFG);
            dumpParams(pCCR);
        }
    }
    return MTRUE;
}


MBOOL
SwNRParam::
getTuning(NR2TuningInfo* pNR, HFGTuningInfo* pHFG, CCRTuningInfo* pCCR)
{
    if( ! pNR || ! pHFG || ! pCCR ) {
        MY_LOGE("pNR(%p), pHFG(%p), pCCR(%p)", pNR, pHFG, pCCR );
        return MFALSE;
    }
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram = (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)mpNvramData;
    FSWNR_PARAM* pPARAM = &(pNvram->fswnr);
    //
    memcpy(pNR , &(pPARAM->NR) , sizeof(NR2_PARAM));
    memcpy(pHFG, &(pPARAM->HFG), sizeof(HFG_PARAM));
    memcpy(pCCR, &(pPARAM->CCR), sizeof(CCR_PARAM));
    //
    // update some parameters
    pNR->NR_SD = generateSeed();
    pHFG->HFG_GSD = generateSeed();
    //
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug ) {
            dumpParams(pNR);
            dumpParams(pHFG);
            dumpParams(pCCR);
        }
    }
    return MTRUE;
}


SwNR::
SwNR(MUINT32 const openId)
    : muOpenId(openId)
    , mNRType(0)
    , mpNRTuningInfo(NULL)
    , mpNR2TuningInfo(NULL)
    , mpHFGTuningInfo(NULL)
    , mpCCRTuningInfo(NULL)
    , muWorkingBufSize(0)
    , mpWorkingBuf(NULL)
{
    mpNRInitInfo   = (NRInitInfo*)malloc(sizeof(NRInitInfo));
    mpNRImageInfo  = (NRImageInfo*)malloc(sizeof(NRImageInfo));
    mpNRResultInfo = (NRResultInfo*)malloc(sizeof(NRResultInfo));
    // initial
    mpNRInitInfo->pWorkingBuff    = 0;
    mpNRInitInfo->WorkingBuffSize = 0;
    mpNRInitInfo->CoreNumber      = NR_MAX_CORE_NO;
    mpNRInitInfo->NumOfExecution  = 1;
    mpNRInitInfo->pTuningInfo     = NULL;
    mpNRInitInfo->pCCRTuningInfo  = NULL;
    mpNRInitInfo->pHFGTuningInfo  = NULL;
    mpNRInitInfo->pPerfInfo       = NULL;
}


SwNR::
~SwNR()
{
#define MY_FREE( ptr ) \
    if( ptr )          \
    {                  \
        free(ptr);     \
        ptr = NULL;    \
    }
    MY_FREE(mpNRInitInfo->pPerfInfo);
    MY_FREE(mpNRInitInfo);
    MY_FREE(mpNRTuningInfo);
    MY_FREE(mpNR2TuningInfo);
    MY_FREE(mpHFGTuningInfo);
    MY_FREE(mpCCRTuningInfo);
    MY_FREE(mpNRImageInfo);
    MY_FREE(mpNRResultInfo);
#undef MY_FREE

    if( mpWorkingBuf ) {
        munmap(mpWorkingBuf, sizeof(MUINT8) * muWorkingBufSize);
        mpWorkingBuf = NULL;
    }
}

MBOOL
SwNR::
prepareTuningInfo()
{
    MBOOL ret = MFALSE;
    //
    SwNRParam param = SwNRParam(muOpenId);
    //
    SwNRParam::Params p;
    p.iso    = mParam.iso;
    p.isMfll = mParam.isMfll;
    p.isDualCam = mParam.isDualCam;
    param.setParam(p);
    //
    if( ! param.update() ) {
        CAM_ULOGMW("cannot update swnr param properly");
        goto lbExit;
    }
    //
    switch( param.getNRType() )
    {
        case eSWNRType_SW:
            mNRType = DRV_NR_OBJ_SW;
            //
            if( ! mpNRTuningInfo ) mpNRTuningInfo = (NRTuningInfo*)malloc(sizeof(NRTuningInfo));
            if( mpNR2TuningInfo ) { free(mpNR2TuningInfo); mpNR2TuningInfo = NULL; }
            if( ! mpHFGTuningInfo ) mpHFGTuningInfo = (HFGTuningInfo*)malloc(sizeof(HFGTuningInfo));
            if( ! mpCCRTuningInfo ) mpCCRTuningInfo = (CCRTuningInfo*)malloc(sizeof(CCRTuningInfo));
            //
            if( ! param.getTuning(mpNRTuningInfo, mpHFGTuningInfo, mpCCRTuningInfo) ) {
                MY_LOGE("getTuning failed");
                goto lbExit;
            }
            //
            mpNRInitInfo->pTuningInfo    = mpNRTuningInfo;
            mpNRInitInfo->pCCRTuningInfo = mpCCRTuningInfo;
            mpNRInitInfo->pHFGTuningInfo = mpHFGTuningInfo;
            break;
        case eSWNRType_SW2:
        default:
            mNRType = DRV_NR_OBJ_SW2;
            //
            if( mpNRTuningInfo ) { free(mpNRTuningInfo); mpNRTuningInfo = NULL; }
            if( ! mpNR2TuningInfo ) mpNR2TuningInfo = (NR2TuningInfo*)malloc(sizeof(NR2TuningInfo));
            if( ! mpHFGTuningInfo ) mpHFGTuningInfo = (HFGTuningInfo*)malloc(sizeof(HFGTuningInfo));
            if( ! mpCCRTuningInfo ) mpCCRTuningInfo = (CCRTuningInfo*)malloc(sizeof(CCRTuningInfo));
            //
            if( ! param.getTuning(mpNR2TuningInfo, mpHFGTuningInfo, mpCCRTuningInfo) ) {
                MY_LOGE("getTuning failed");
                goto lbExit;
            }
            mpNRInitInfo->pNR2TuningInfo = mpNR2TuningInfo;
            mpNRInitInfo->pCCRTuningInfo = mpCCRTuningInfo;
            mpNRInitInfo->pHFGTuningInfo = mpHFGTuningInfo;
            break;
    }
    //
    // set performance level
    {
        if( mpNRInitInfo->pPerfInfo == NULL ) {
            mpNRInitInfo->pPerfInfo = new NRPerfGear;
        }
        //
        switch(mParam.perfLevel)
        {
#define PerfCase(val, level)                             \
            case val:                                    \
                mpNRInitInfo->pPerfInfo->enable = 1;     \
                mpNRInitInfo->pPerfInfo->option = level; \
                break;
            PerfCase(eSWNRPerf_Performance_First, NR_PERF_GEAR_PERF_FIRST)
            PerfCase(eSWNRPerf_Custom0          , NR_PERF_GEAR_CUSTOM_0)
            PerfCase(eSWNRPerf_Power_First      , NR_PERF_GEAR_POWER_FIRST)
            default:
                CAM_ULOGMW("not valid level %d", mParam.perfLevel);
                delete mpNRInitInfo->pPerfInfo;
                mpNRInitInfo->pPerfInfo = NULL;
#undef PerfCase
        }
    }
    //
    ret = MTRUE;
lbExit:
    return ret;
}

ISwNR*
ISwNR::
createInstance(MUINT32 const openId)
{
    return new SwNR(openId);
}

MBOOL
SwNR::
setImageInfo(IImageBuffer* pBuf)
{
    MBOOL ret = MFALSE;
    MTK_NR_IMAGE_FMT_ENUM NRFmt = NR_IMAGE_PACKET_YUY2;
    switch( pBuf->getImgFormat() )
    {
        case eImgFmt_YUY2:
            NRFmt = NR_IMAGE_PACKET_YUY2;
            break;
        case eImgFmt_I420:
            NRFmt = NR_IMAGE_YUV420;
            break;
        case eImgFmt_YV12:
            NRFmt = NR_IMAGE_YV12;
            break;
        default:
            MY_LOGE("not supported format: 0x%x", pBuf->getImgFormat() );
            goto lbExit;
    }

    // set image info
    mpNRImageInfo->Width = pBuf->getImgSize().w;
    mpNRImageInfo->Height = pBuf->getImgSize().h;
    mpNRImageInfo->pImg[0] = (void*)pBuf->getBufVA(0);
    mpNRImageInfo->pImg[1] = pBuf->getPlaneCount() > 1 ? (void*)pBuf->getBufVA(1) : (void*)0,
    mpNRImageInfo->pImg[2] = pBuf->getPlaneCount() > 2 ? (void*)pBuf->getBufVA(2) : (void*)0,

#if 0
    CAM_ULOGMD("buf %dx%d va(0x%x/0x%x/0x%x) size(%d/%d/%d)",
            mpNRImageInfo->Width, mpNRImageInfo->Height,
            mpNRImageInfo->pImg[0], mpNRImageInfo->pImg[1], mpNRImageInfo->pImg[2],
            pBuf->getBufSizeInBytes(0),
            pBuf->getPlaneCount() > 1 ? pBuf->getBufSizeInBytes(1) : 0,
            pBuf->getPlaneCount() > 2 ? pBuf->getBufSizeInBytes(2) : 0
          );

#endif
    mpNRImageInfo->ImgNum = 1;
    mpNRImageInfo->ImgFmt = NRFmt;

    ret = MTRUE;
lbExit:
    return ret;
}


MBOOL
SwNR::
allocWorkingBuf(MUINT32 size)
{
    MBOOL ret = MFALSE;
    // allocate working buffer
    if( size != muWorkingBufSize )
    {
        if( mpWorkingBuf )
            free(mpWorkingBuf);
        mpWorkingBuf = (MUINT8*)mmap(NULL, sizeof(MUINT8) * size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if( mpWorkingBuf == MAP_FAILED )
        {
            MY_LOGE("allocate working buffer failed");
            goto lbExit;
        }
        MY_LOGD("allocate working buffer %p, size %d", mpWorkingBuf, size);
        muWorkingBufSize = size;
    }

    mpNRInitInfo->pWorkingBuff = mpWorkingBuf;
    mpNRInitInfo->WorkingBuffSize = muWorkingBufSize;

    ret = MTRUE;
lbExit:
    return ret;
}


MBOOL
SwNR::
doSwNR(SWNRParam const& param, NSCam::IImageBuffer* pBuf)
{
#define CHECK_NR_RET( NR_ret )                     \
    do{                                            \
        MRESULT nr_ret = (NR_ret);                 \
        if( nr_ret != S_NR_OK ) {                  \
            MY_LOGE("NR with error 0x%x", nr_ret); \
            goto lbExit;                           \
        }                                          \
    }                                              \
    while(0);
    mParam = param;
    MY_LOGD("idx %d: iso %d perf %d, buf %p",
            muOpenId,
            mParam.iso, mParam.perfLevel,
            pBuf);
    MBOOL ret = MFALSE;
    //
    MUINT32 workingbufsize = 0;
    MTKNR* pMTKNR = NULL;

    // get tuning info
    if( !prepareTuningInfo() )
        goto lbExit;
    //
    MY_LOGD("swnr type %s", mNRType == DRV_NR_OBJ_SW ? "normal" : "fast");
    pMTKNR = MTKNR::createInstance(static_cast<DrvNRObject_e>(mNRType));
    //
    CHECK_NR_RET( pMTKNR->NRInit((MUINT32*) mpNRInitInfo, 0) );

    if( !setImageInfo(pBuf) )
        goto lbExit;

    CHECK_NR_RET(
            pMTKNR->NRFeatureCtrl(
                NR_FEATURE_GET_WORKBUF_SIZE,
                (void *)mpNRImageInfo,
                (void *)&workingbufsize)
            );

    if( !allocWorkingBuf(workingbufsize) )
        goto lbExit;

    CHECK_NR_RET(
            pMTKNR->NRFeatureCtrl(
                NR_FEATURE_SET_WORKBUF_ADDR,
                mpNRInitInfo->pWorkingBuff,
                NULL)
            );

    // add img
    CHECK_NR_RET(
            pMTKNR->NRFeatureCtrl(
                NR_FEATURE_ADD_IMG,
                (void *)mpNRImageInfo,
                NULL)
            );

    MY_LOGD("start+");
    // main
    CHECK_NR_RET( pMTKNR->NRMain() );
    MY_LOGD("start-");

    CHECK_NR_RET(
            pMTKNR->NRFeatureCtrl(
                NR_FEATURE_GET_RESULT,
                NULL,
                (void *)mpNRResultInfo)
            );

    pBuf->syncCache(eCACHECTRL_FLUSH);

    pMTKNR->NRReset();

#undef CHECK_NR_RET
    ret = MTRUE;
lbExit:
    if( pMTKNR ) {
        pMTKNR->destroyInstance(pMTKNR);
        pMTKNR = NULL;
    }

    return ret;
}


MBOOL
SwNR::
getDebugInfo(IMetadata& halMetadata) const
{
    bool haveExif = false;
    {
        IMetadata::IEntry entry = halMetadata.entryFor(MTK_HAL_REQUEST_REQUIRE_EXIF);
        if( ! entry.isEmpty()  && entry.itemAt(0, Type2Type<MUINT8>()) )
                haveExif = true;
    }
    //
    if( haveExif )
    {
        IMetadata::Memory memory_dbgInfo;
        memory_dbgInfo.resize(sizeof(DEBUG_RESERVEA_INFO_T));
        DEBUG_RESERVEA_INFO_T& dbgInfo =
            *reinterpret_cast<DEBUG_RESERVEA_INFO_T*>(memory_dbgInfo.editArray());
        ssize_t idx = 0;
#define addPair(debug_info, index, id, value)           \
        do{                                             \
            debug_info.Tag[index].u4FieldID = id;       \
            debug_info.Tag[index].u4FieldValue = value; \
            index++;                                    \
        } while(0)
        //
        addPair(dbgInfo, idx, RESERVEA_TAG_VERSION, RESERVEA_DEBUG_TAG_VERSION);
        //
        if( mNRType == DRV_NR_OBJ_SW && mpNRTuningInfo ) // normal swnr
        {
            addPair(dbgInfo , idx , ANR_Y_LUMA_SCALE_RANGE , mpNRTuningInfo->ANR_Y_LUMA_SCALE_RANGE );
            addPair(dbgInfo , idx , ANR_C_CHROMA_SCALE     , mpNRTuningInfo->ANR_C_CHROMA_SCALE     );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY0       , mpNRTuningInfo->ANR_Y_SCALE_CPY0       );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY1       , mpNRTuningInfo->ANR_Y_SCALE_CPY1       );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY2       , mpNRTuningInfo->ANR_Y_SCALE_CPY2       );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY3       , mpNRTuningInfo->ANR_Y_SCALE_CPY3       );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY4       , mpNRTuningInfo->ANR_Y_SCALE_CPY4       );
            addPair(dbgInfo , idx , ANR_Y_CPX1             , mpNRTuningInfo->ANR_Y_CPX1             );
            addPair(dbgInfo , idx , ANR_Y_CPX2             , mpNRTuningInfo->ANR_Y_CPX2             );
            addPair(dbgInfo , idx , ANR_Y_CPX3             , mpNRTuningInfo->ANR_Y_CPX3             );
            addPair(dbgInfo , idx , ANR_CEN_GAIN_LO_TH     , mpNRTuningInfo->ANR_CEN_GAIN_LO_TH     );
            addPair(dbgInfo , idx , ANR_CEN_GAIN_HI_TH     , mpNRTuningInfo->ANR_CEN_GAIN_HI_TH     );
            addPair(dbgInfo , idx , ANR_PTY_GAIN_TH        , mpNRTuningInfo->ANR_PTY_GAIN_TH        );
            addPair(dbgInfo , idx , ANR_KSIZE_LO_TH        , mpNRTuningInfo->ANR_KSIZE_LO_TH        );
            addPair(dbgInfo , idx , ANR_KSIZE_HI_TH        , mpNRTuningInfo->ANR_KSIZE_HI_TH        );
            addPair(dbgInfo , idx , ANR_KSIZE_LO_TH_C      , mpNRTuningInfo->ANR_KSIZE_LO_TH_C      );
            addPair(dbgInfo , idx , ANR_KSIZE_HI_TH_C      , mpNRTuningInfo->ANR_KSIZE_HI_TH_C      );
            addPair(dbgInfo , idx , ITUNE_ANR_PTY_STD      , mpNRTuningInfo->ITUNE_ANR_PTY_STD      );
            addPair(dbgInfo , idx , ITUNE_ANR_PTU_STD      , mpNRTuningInfo->ITUNE_ANR_PTU_STD      );
            addPair(dbgInfo , idx , ITUNE_ANR_PTV_STD      , mpNRTuningInfo->ITUNE_ANR_PTV_STD      );
            addPair(dbgInfo , idx , ANR_ACT_TH_Y           , mpNRTuningInfo->ANR_ACT_TH_Y           );
            addPair(dbgInfo , idx , ANR_ACT_BLD_BASE_Y     , mpNRTuningInfo->ANR_ACT_BLD_BASE_Y     );
            addPair(dbgInfo , idx , ANR_ACT_BLD_TH_Y       , mpNRTuningInfo->ANR_ACT_BLD_TH_Y       );
            addPair(dbgInfo , idx , ANR_ACT_SLANT_Y        , mpNRTuningInfo->ANR_ACT_SLANT_Y        );
            addPair(dbgInfo , idx , ANR_ACT_TH_C           , mpNRTuningInfo->ANR_ACT_TH_C           );
            addPair(dbgInfo , idx , ANR_ACT_BLD_BASE_C     , mpNRTuningInfo->ANR_ACT_BLD_BASE_C     );
            addPair(dbgInfo , idx , ANR_ACT_BLD_TH_C       , mpNRTuningInfo->ANR_ACT_BLD_TH_C       );
            addPair(dbgInfo , idx , ANR_ACT_SLANT_C        , mpNRTuningInfo->ANR_ACT_SLANT_C        );
            addPair(dbgInfo , idx , RADIUS_H               , mpNRTuningInfo->RADIUS_H               );
            addPair(dbgInfo , idx , RADIUS_V               , mpNRTuningInfo->RADIUS_V               );
            addPair(dbgInfo , idx , RADIUS_H_C             , mpNRTuningInfo->RADIUS_H_C             );
            addPair(dbgInfo , idx , RADIUS_V_C             , mpNRTuningInfo->RADIUS_V_C             );
            addPair(dbgInfo , idx , ANR_PTC_HGAIN          , mpNRTuningInfo->ANR_PTC_HGAIN          );
            addPair(dbgInfo , idx , ANR_PTY_HGAIN          , mpNRTuningInfo->ANR_PTY_HGAIN          );
            addPair(dbgInfo , idx , ANR_LPF_HALFKERNEL     , mpNRTuningInfo->ANR_LPF_HALFKERNEL     );
            addPair(dbgInfo , idx , ANR_LPF_HALFKERNEL_C   , mpNRTuningInfo->ANR_LPF_HALFKERNEL_C   );
            addPair(dbgInfo , idx , ANR_ACT_MODE           , mpNRTuningInfo->ANR_ACT_MODE           );
            addPair(dbgInfo , idx , ANR_LCE_SCALE_GAIN     , mpNRTuningInfo->ANR_LCE_SCALE_GAIN     );
            addPair(dbgInfo , idx , ANR_LCE_C_GAIN         , mpNRTuningInfo->ANR_LCE_C_GAIN         );
            addPair(dbgInfo , idx , ANR_LCE_GAIN0          , mpNRTuningInfo->ANR_LCE_GAIN0          );
            addPair(dbgInfo , idx , ANR_LCE_GAIN1          , mpNRTuningInfo->ANR_LCE_GAIN1          );
            addPair(dbgInfo , idx , ANR_LCE_GAIN2          , mpNRTuningInfo->ANR_LCE_GAIN2          );
            addPair(dbgInfo , idx , ANR_LCE_GAIN3          , mpNRTuningInfo->ANR_LCE_GAIN3          );
            addPair(dbgInfo , idx , ANR_MEDIAN_LOCATION    , mpNRTuningInfo->ANR_MEDIAN_LOCATION    );
            addPair(dbgInfo , idx , ANR_CEN_X              , mpNRTuningInfo->ANR_CEN_X              );
            addPair(dbgInfo , idx , ANR_CEN_Y              , mpNRTuningInfo->ANR_CEN_Y              );
            addPair(dbgInfo , idx , ANR_R1                 , mpNRTuningInfo->ANR_R1                 );
            addPair(dbgInfo , idx , ANR_R2                 , mpNRTuningInfo->ANR_R2                 );
            addPair(dbgInfo , idx , ANR_R3                 , mpNRTuningInfo->ANR_R3                 );
            addPair(dbgInfo , idx , LUMA_ON_OFF            , mpNRTuningInfo->LUMA_ON_OFF            );
        }
        else if( mNRType == DRV_NR_OBJ_SW2 && mpNR2TuningInfo ) // fast swnr
        {
            addPair(dbgInfo , idx , NR_K      , mpNR2TuningInfo->NR_K);
            addPair(dbgInfo , idx , NR_S      , mpNR2TuningInfo->NR_S);
            addPair(dbgInfo , idx , NR_SD     , mpNR2TuningInfo->NR_SD);
            addPair(dbgInfo , idx , NR_BLD_W  , mpNR2TuningInfo->NR_BLD_W);
            addPair(dbgInfo , idx , NR_BLD_TH , mpNR2TuningInfo->NR_BLD_TH);
            addPair(dbgInfo , idx , NR_SMTH   , mpNR2TuningInfo->NR_SMTH);
        }
        else {
            MY_LOGE("not supported type %d", mNRType);
            return MFALSE;
        }
        // HFG
        if( mpHFGTuningInfo )
        {
            addPair(dbgInfo , idx , HFG_ENABLE      , mpHFGTuningInfo->HFG_ENABLE);
            addPair(dbgInfo , idx , HFG_GSD         , mpHFGTuningInfo->HFG_GSD);
            addPair(dbgInfo , idx , HFG_SD0         , mpHFGTuningInfo->HFG_SD0);
            addPair(dbgInfo , idx , HFG_SD1         , mpHFGTuningInfo->HFG_SD1);
            addPair(dbgInfo , idx , HFG_SD2         , mpHFGTuningInfo->HFG_SD2);
            addPair(dbgInfo , idx , HFG_TX_S        , mpHFGTuningInfo->HFG_TX_S);
            addPair(dbgInfo , idx , HFG_LCE_LINK_EN , mpHFGTuningInfo->HFG_LCE_LINK_EN);
            addPair(dbgInfo , idx , HFG_LUMA_CPX1   , mpHFGTuningInfo->HFG_LUMA_CPX1);
            addPair(dbgInfo , idx , HFG_LUMA_CPX2   , mpHFGTuningInfo->HFG_LUMA_CPX2);
            addPair(dbgInfo , idx , HFG_LUMA_CPX3   , mpHFGTuningInfo->HFG_LUMA_CPX3);
            addPair(dbgInfo , idx , HFG_LUMA_CPY0   , mpHFGTuningInfo->HFG_LUMA_CPY0);
            addPair(dbgInfo , idx , HFG_LUMA_CPY1   , mpHFGTuningInfo->HFG_LUMA_CPY1);
            addPair(dbgInfo , idx , HFG_LUMA_CPY2   , mpHFGTuningInfo->HFG_LUMA_CPY2);
            addPair(dbgInfo , idx , HFG_LUMA_CPY3   , mpHFGTuningInfo->HFG_LUMA_CPY3);
            addPair(dbgInfo , idx , HFG_LUMA_SP0    , mpHFGTuningInfo->HFG_LUMA_SP0);
            addPair(dbgInfo , idx , HFG_LUMA_SP1    , mpHFGTuningInfo->HFG_LUMA_SP1);
            addPair(dbgInfo , idx , HFG_LUMA_SP2    , mpHFGTuningInfo->HFG_LUMA_SP2);
            addPair(dbgInfo , idx , HFG_LUMA_SP3    , mpHFGTuningInfo->HFG_LUMA_SP3);
        }
        else {
            MY_LOGE("null HFG info");
            return MFALSE;
        }
        // CCR
        if( mpCCRTuningInfo )
        {
            addPair(dbgInfo , idx , CCR_ENABLE       , mpCCRTuningInfo->CCR_ENABLE);
            addPair(dbgInfo , idx , CCR_CEN_U        , mpCCRTuningInfo->CCR_CEN_U);
            addPair(dbgInfo , idx , CCR_CEN_V        , mpCCRTuningInfo->CCR_CEN_V);
            addPair(dbgInfo , idx , CCR_Y_CPX1       , mpCCRTuningInfo->CCR_Y_CPX1);
            addPair(dbgInfo , idx , CCR_Y_CPX2       , mpCCRTuningInfo->CCR_Y_CPX2);
            addPair(dbgInfo , idx , CCR_Y_CPY1       , mpCCRTuningInfo->CCR_Y_CPY1);
            addPair(dbgInfo , idx , CCR_Y_SP1        , mpCCRTuningInfo->CCR_Y_SP1);
            addPair(dbgInfo , idx , CCR_UV_X1        , mpCCRTuningInfo->CCR_UV_X1);
            addPair(dbgInfo , idx , CCR_UV_X2        , mpCCRTuningInfo->CCR_UV_X2);
            addPair(dbgInfo , idx , CCR_UV_X3        , mpCCRTuningInfo->CCR_UV_X3);
            addPair(dbgInfo , idx , CCR_UV_GAIN1     , mpCCRTuningInfo->CCR_UV_GAIN1);
            addPair(dbgInfo , idx , CCR_UV_GAIN2     , mpCCRTuningInfo->CCR_UV_GAIN2);
            addPair(dbgInfo , idx , CCR_UV_GAIN_SP1  , mpCCRTuningInfo->CCR_UV_GAIN_SP1);
            addPair(dbgInfo , idx , CCR_UV_GAIN_SP2  , mpCCRTuningInfo->CCR_UV_GAIN_SP2);
            addPair(dbgInfo , idx , CCR_Y_CPX3       , mpCCRTuningInfo->CCR_Y_CPX3);
            addPair(dbgInfo , idx , CCR_Y_CPY0       , mpCCRTuningInfo->CCR_Y_CPY0);
            addPair(dbgInfo , idx , CCR_Y_CPY2       , mpCCRTuningInfo->CCR_Y_CPY2);
            addPair(dbgInfo , idx , CCR_Y_SP0        , mpCCRTuningInfo->CCR_Y_SP0);
            addPair(dbgInfo , idx , CCR_Y_SP2        , mpCCRTuningInfo->CCR_Y_SP2);
            addPair(dbgInfo , idx , CCR_UV_GAIN_MODE , mpCCRTuningInfo->CCR_UV_GAIN_MODE);
            addPair(dbgInfo , idx , CCR_MODE         , mpCCRTuningInfo->CCR_MODE);
            addPair(dbgInfo , idx , CCR_OR_MODE      , mpCCRTuningInfo->CCR_OR_MODE);
            addPair(dbgInfo , idx , CCR_HUE_X1       , mpCCRTuningInfo->CCR_HUE_X1);
            addPair(dbgInfo , idx , CCR_HUE_X2       , mpCCRTuningInfo->CCR_HUE_X2);
            addPair(dbgInfo , idx , CCR_HUE_X3       , mpCCRTuningInfo->CCR_HUE_X3);
            addPair(dbgInfo , idx , CCR_HUE_X4       , mpCCRTuningInfo->CCR_HUE_X4);
            addPair(dbgInfo , idx , CCR_HUE_SP1      , mpCCRTuningInfo->CCR_HUE_SP1);
            addPair(dbgInfo , idx , CCR_HUE_SP2      , mpCCRTuningInfo->CCR_HUE_SP2);
            addPair(dbgInfo , idx , CCR_HUE_GAIN1    , mpCCRTuningInfo->CCR_HUE_GAIN1);
            addPair(dbgInfo , idx , CCR_HUE_GAIN2    , mpCCRTuningInfo->CCR_HUE_GAIN2);
        }
        else {
            MY_LOGE("null CCR info");
            return MFALSE;
        }
        // set zeros to remaining
        while( idx < RESERVEA_DEBUG_TAG_SIZE ) {
            addPair(dbgInfo , idx , 0, 0);
        }
        //
#undef addPair
        //
        IMetadata exifMeta;
        // query from hal metadata first
        {
            IMetadata::IEntry entry= halMetadata.entryFor(MTK_3A_EXIF_METADATA);
            if( ! entry.isEmpty() )
                exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
        }
        // update
        IMetadata::IEntry entry_key(MTK_POSTNR_EXIF_DBGINFO_NR_KEY);
        entry_key.push_back(DEBUG_EXIF_MID_CAM_RESERVE1, Type2Type<MINT32>());
        exifMeta.update(entry_key.tag(), entry_key);
        //
        IMetadata::IEntry entry_data(MTK_POSTNR_EXIF_DBGINFO_NR_DATA);
        entry_data.push_back(memory_dbgInfo, Type2Type<IMetadata::Memory>());
        exifMeta.update(entry_data.tag(), entry_data);
        //
        IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
        entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
        halMetadata.update(entry_exif.tag(), entry_exif);
    }
    else
    {
        MY_LOGD("no need to dump exif");
    }
    //
    return MTRUE;
}


MVOID
SwNR::
dumpParam(char * const filename)
{
    // write to file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        MY_LOGE("fopen fail: %s", filename);
        return;
    }
    //
    fprintf(fp, "swnr:%s\n\n", (mNRType == eSWNRType_SW) ? "normal" : "fast");
    //
    if( mNRType == eSWNRType_SW )
    {
        dumpParams(mpNRTuningInfo, fp);
        dumpParams(mpHFGTuningInfo, fp);
        dumpParams(mpCCRTuningInfo, fp);
    }
    else
    {
        dumpParams(mpNR2TuningInfo, fp);
        dumpParams(mpHFGTuningInfo, fp);
        dumpParams(mpCCRTuningInfo, fp);
    }
    //
    fclose(fp);
}
