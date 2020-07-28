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
#include <mtkcam/Log.h>
#define SWNR_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/common.h>
//
#include <utils/StrongPointer.h>
using namespace android;
//
#include <mtkcam/featureio/capturenr.h>
using namespace NSCam;
//
#include <utils/Mutex.h>
#include <utils/String8.h>
#include <list>
using namespace std;
//
#include <sys/mman.h>
//
// custom tuning
#include <mtkcam/hal/IHalSensor.h>
//
#include <cutils/properties.h>
//

#define SWNR_ASSERT(x)                                                      \
do {                                                                        \
    MBOOL __x = (x);                                                        \
    if (!__x) {                                                             \
        SWNR_LOGE("[%s:%d] ASSERT FAILED: %s", __FUNCTION__, __LINE__, #x); \
        goto lbExit;                                                        \
    }                                                                       \
} while(0);

/*******************************************************************************
*
********************************************************************************/
static Mutex                    g_SwParamlock;
static list<SwNRParam*>         g_SwParamlist;
/*******************************************************************************
*
********************************************************************************/
SwNRParam*
SwNRParam::
getInstance(MUINT32 openId)
{
    Mutex::Autolock _l(g_SwParamlock);

    SwNRParam* pParam = NULL;
    list<SwNRParam*>::const_iterator iter;
    for( iter = g_SwParamlist.begin() ; iter != g_SwParamlist.end(); iter++ )
    {
        if( (*iter)->mOpenId == openId )
            pParam = (*iter);
    }

    if( pParam == NULL )
    {
        //create new
        pParam = new SwNRParam(openId);
        g_SwParamlist.push_back(pParam);
    }

    return pParam;
}


SwNRParam::
SwNRParam(MUINT32 openId)
    : mOpenId(openId)
    , mIso(0)
    , mScale_x100(100)
    , miPerfLevel(-1)
{
}


SwNRParam::
~SwNRParam()
{
}


MVOID
SwNRParam::
setParam(MUINT32 const iso, MUINT32 const scale_x100, MINT32 const perfLevel)
{
    SWNR_LOGD("iso = %d, scale %d, perf lv %d", iso, scale_x100, perfLevel);
    mIso = iso;
    mScale_x100 = scale_x100;
    miPerfLevel = perfLevel;
}


MBOOL
SwNRParam::
getTuningNR(NRTuningInfo* pTuning)
{
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    MUINT32 idx, iso_low, iso_high;


    // set all params to zero
    memset(pTuning, 0, sizeof(NRTuningInfo));

    idx = map_ISO_value_to_index(mIso);
    iso_low = map_index_to_ISO_value(idx);
    iso_high = map_index_to_ISO_value( (idx+1)>=eNUM_OF_SWNR_IDX ? idx : idx+1);

    if (iso_high == (MUINT32)-1) {
        SWNR_LOGW("iso overflow, force to %d", iso_low);
        iso_high = iso_low;
    }
    
    SWNR_LOGD("%d: iso %d, idx low/high %d/%d",
            mOpenId, mIso, idx, ((idx+1)>=eNUM_OF_SWNR_IDX ? idx : idx+1)
    );

    if( is_to_invoke_swnr_interpolation(mIso) && iso_low < iso_high)
    {
        //
        String8 logParam;
        swnr_param_t param_low, param_high;
        if( !get_tuning_param_NR(sensorDev, idx, param_low) ||
                !get_tuning_param_NR(sensorDev, ((idx+1)>=eNUM_OF_SWNR_IDX ? idx : idx+1), param_high)) {
            SWNR_LOGE("cannot get tuning param from custom, %d:dev %d, idx %d/%d",
                mOpenId, sensorDev, idx, ((idx+1)>=eNUM_OF_SWNR_IDX ? idx : idx+1));
            return MFALSE;
        }
        logParam += String8::format("iso %d/%d, param %d, %d/%d, %d",
                iso_low, iso_high,
                param_low.ANR_Y_LUMA_SCALE_RANGE, param_low.ANR_C_CHROMA_SCALE,
                param_high.ANR_Y_LUMA_SCALE_RANGE, param_high.ANR_C_CHROMA_SCALE);
        //
        swnr_param_t param_smooth;
        if( !smooth_param(mIso, iso_high, iso_low, param_high, param_low,
                          param_smooth) )
        {
            SWNR_LOGE("smoth_param failed, %d:dev %d, iso %d, %d",
                mOpenId, sensorDev, iso_low, iso_high);
            return MFALSE;
        }
        logParam += String8::format(", smooth param %d, %d",
                param_smooth.ANR_Y_LUMA_SCALE_RANGE, param_smooth.ANR_C_CHROMA_SCALE);
        //
        if( !refineByScale(sensorDev, mScale_x100, param_smooth) ) {
            SWNR_LOGE("refineByScale failed, %d:dev %d, scale %d",
                    mOpenId, sensorDev, mScale_x100);
            return MFALSE;
        }
        logParam += String8::format(", refined param %d, %d",
                param_smooth.ANR_Y_LUMA_SCALE_RANGE, param_smooth.ANR_C_CHROMA_SCALE);
        //
        SWNR_LOGD("%s", logParam.string());
        //
        memcpy(pTuning, &param_smooth, sizeof(swnr_param_t));
    }
    else
    {
        String8 logParam;
        swnr_param_t param;
        if( !get_tuning_param_NR(sensorDev, idx, param) ) {
            SWNR_LOGE("cannot get tuning param from custom, %d:dev %d, idx %d",
                mOpenId, sensorDev, idx);
            return MFALSE;
        }
        logParam += String8::format("param %d, %d",
                param.ANR_Y_LUMA_SCALE_RANGE, param.ANR_C_CHROMA_SCALE);
        //
        if( !refineByScale(sensorDev, mScale_x100, param) ) {
            SWNR_LOGE("refineByScale failed, %d:dev %d, scale %d",
                    mOpenId, sensorDev, mScale_x100);
            return MFALSE;
        }
        logParam += String8::format(", refined param %d, %d",
                param.ANR_Y_LUMA_SCALE_RANGE, param.ANR_C_CHROMA_SCALE);
        //
        SWNR_LOGD("%s", logParam.string());
        //
        memcpy(pTuning, &param, sizeof(swnr_param_t));
    }

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug )
        {
#define DUMP_PARAM( param )                                  \
            do {                                             \
                SWNR_LOGD("%s:%d", #param, pTuning->param);  \
            } while(0)
            DUMP_PARAM(ANR_Y_LUMA_SCALE_RANGE);
            DUMP_PARAM(ANR_C_CHROMA_SCALE);
            DUMP_PARAM(ANR_Y_SCALE_CPY0);
            DUMP_PARAM(ANR_Y_SCALE_CPY1);
            DUMP_PARAM(ANR_Y_SCALE_CPY2);
            DUMP_PARAM(ANR_Y_SCALE_CPY3);
            DUMP_PARAM(ANR_Y_SCALE_CPY4);
            DUMP_PARAM(ANR_Y_CPX1);
            DUMP_PARAM(ANR_Y_CPX2);
            DUMP_PARAM(ANR_Y_CPX3);
            DUMP_PARAM(ANR_CEN_GAIN_LO_TH);
            DUMP_PARAM(ANR_CEN_GAIN_HI_TH);
            DUMP_PARAM(ANR_PTY_GAIN_TH);
            DUMP_PARAM(ANR_KSIZE_LO_TH);
            DUMP_PARAM(ANR_KSIZE_HI_TH);
            DUMP_PARAM(ANR_KSIZE_LO_TH_C);
            DUMP_PARAM(ANR_KSIZE_HI_TH_C);
            DUMP_PARAM(ITUNE_ANR_PTY_STD);
            DUMP_PARAM(ITUNE_ANR_PTU_STD);
            DUMP_PARAM(ITUNE_ANR_PTV_STD);
            DUMP_PARAM(ANR_ACT_TH_Y);
            DUMP_PARAM(ANR_ACT_BLD_BASE_Y);
            DUMP_PARAM(ANR_ACT_BLD_TH_Y);
            DUMP_PARAM(ANR_ACT_SLANT_Y);
            DUMP_PARAM(ANR_ACT_TH_C);
            DUMP_PARAM(ANR_ACT_BLD_BASE_C);
            DUMP_PARAM(ANR_ACT_BLD_TH_C);
            DUMP_PARAM(ANR_ACT_SLANT_C);
            DUMP_PARAM(RADIUS_H);
            DUMP_PARAM(RADIUS_V);
            DUMP_PARAM(RADIUS_H_C);
            DUMP_PARAM(RADIUS_V_C);
            DUMP_PARAM(ANR_PTC_HGAIN);
            DUMP_PARAM(ANR_PTY_HGAIN);
            DUMP_PARAM(ANR_LPF_HALFKERNEL);
            DUMP_PARAM(ANR_LPF_HALFKERNEL_C);
            DUMP_PARAM(ANR_ACT_MODE);
            DUMP_PARAM(ANR_LCE_SCALE_GAIN);
            DUMP_PARAM(ANR_LCE_C_GAIN);
            DUMP_PARAM(ANR_LCE_GAIN0);
            DUMP_PARAM(ANR_LCE_GAIN1);
            DUMP_PARAM(ANR_LCE_GAIN2);
            DUMP_PARAM(ANR_LCE_GAIN3);
            DUMP_PARAM(ANR_MEDIAN_LOCATION);
            DUMP_PARAM(ANR_CEN_X);
            DUMP_PARAM(ANR_CEN_Y);
            DUMP_PARAM(ANR_R1);
            DUMP_PARAM(ANR_R2);
            DUMP_PARAM(ANR_R3);
            DUMP_PARAM(LUMA_ON_OFF);
        }
    }
    return MTRUE;
}

MBOOL
SwNRParam::
getTuningNR2(NR2TuningInfo* pTuningNR2)
{
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    MUINT32 idx, iso_low, iso_high;

    // set all params to zero
    memset(pTuningNR2, 0, sizeof(NR2TuningInfo));

    idx = map_ISO_value_to_index(mIso);
    iso_low = map_index_to_ISO_value(idx);
    iso_high = map_index_to_ISO_value( (idx+1)>=eNUM_OF_SWNR_IDX ? idx : idx+1);
    if (iso_high == (MUINT32)-1) {
        SWNR_LOGW("iso overflow, force to %d", iso_low);
        iso_high = iso_low;
    }
    SWNR_LOGD("%d: iso %d, idx low/high %d/%d",
            mOpenId, mIso, idx, ((idx+1)>=eNUM_OF_SWNR_IDX ? idx : idx+1)
    );

    if( is_to_invoke_swnr_interpolation(mIso) && iso_low < iso_high)
    {
        String8 logParam;
        swnr2_param_t param_low, param_high;
        if( !get_tuning_param_NR2(sensorDev, idx, param_low) ||
                !get_tuning_param_NR2(sensorDev, ((idx+1)>=eNUM_OF_SWNR_IDX ? idx : idx+1), param_high)) {
            SWNR_LOGE("cannot get tuning param from custom, %d:dev %d, idx %d/%d",
                mOpenId, sensorDev, idx, ((idx+1)>=eNUM_OF_SWNR_IDX ? idx : idx+1));
            return MFALSE;
        }
        logParam += String8::format("iso %d/%d, param %d, %d, %d, %d, %d, %d/%d, %d, %d, %d, %d, %d",
                iso_low, iso_high,
                param_low.NR_K, param_low.NR_S, param_low.NR_SD, param_low.NR_BLD_W, param_low.NR_BLD_TH, param_low.NR_SMTH,
                param_high.NR_K, param_high.NR_S, param_high.NR_SD, param_high.NR_BLD_W, param_high.NR_BLD_TH, param_high.NR_SMTH);
        //
        swnr2_param_t param_smooth;
        if( !smooth_param_NR2(mIso, iso_high, iso_low, param_high, param_low, param_smooth) )
        {
            SWNR_LOGE("smoth_param failed, %d:dev %d, iso %d, %d", mOpenId, sensorDev, iso_low, iso_high);
            return MFALSE;
        }
        logParam += String8::format(", smooth param %d, %d, %d, %d, %d, %d",
                param_smooth.NR_K, param_smooth.NR_S, param_smooth.NR_SD, param_smooth.NR_BLD_W, param_smooth.NR_BLD_TH, param_smooth.NR_SMTH);
        //
        if( !refineByScaleNR2(sensorDev, mScale_x100, param_smooth) ) {
            SWNR_LOGE("refineByScale failed, %d:dev %d, scale %d", mOpenId, sensorDev, mScale_x100);
            return MFALSE;
        }
        logParam += String8::format(", refined param %d, %d",
                param_smooth.NR_K, param_smooth.NR_S);
        //
        SWNR_LOGD("%s", logParam.string());
        //
        memcpy(pTuningNR2, &param_smooth, sizeof(swnr2_param_t));        
    }
    else
    {
        String8 logParam;
        swnr2_param_t param;
		hfg_param_t param_hfg;
		ccr_param_t param_ccr;
        if( !get_tuning_param_NR2(sensorDev, idx, param) ) {
            SWNR_LOGE("cannot get tuning param from custom, %d:dev %d, idx %d",
                mOpenId, sensorDev, idx);
            return MFALSE;
        }
        logParam += String8::format("param %d, %d, %d, %d, %d, %d",
                param.NR_K, param.NR_S, param.NR_SD, param.NR_BLD_W, param.NR_BLD_TH, param.NR_SMTH);
        //
        if( !refineByScaleNR2(sensorDev, mScale_x100, param) ) {
            SWNR_LOGE("refineByScaleNR2 failed, %d:dev %d, scale %d",
                    mOpenId, sensorDev, mScale_x100);
            return MFALSE;
        }
        logParam += String8::format(", refined param %d, %d, %d, %d, %d, %d",
                param.NR_K, param.NR_S, param.NR_SD, param.NR_BLD_W, param.NR_BLD_TH, param.NR_SMTH);
        //
        SWNR_LOGD("%s", logParam.string());
        //
        memcpy(pTuningNR2, &param, sizeof(swnr2_param_t));
    }

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug )
        {
#define DUMP_PARAM( param )                                     \
            do {                                                \
                SWNR_LOGD("%s:%d", #param, pTuningNR2->param);  \
            } while(0)
            DUMP_PARAM(NR_K);
            DUMP_PARAM(NR_S);
            DUMP_PARAM(NR_SD);
            DUMP_PARAM(NR_BLD_W);
            DUMP_PARAM(NR_BLD_TH);
            DUMP_PARAM(NR_SMTH);
        }
    }
    return MTRUE;
}

MBOOL
SwNRParam::
getTuningHFG(HFGTuningInfo* pTuningHFG)
{
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    MUINT32 idx;

    // set all params to zero
    memset(pTuningHFG, 0, sizeof(HFGTuningInfo));

    idx = map_ISO_value_to_index(mIso);
    SWNR_LOGD("%d: iso %d, idx %d",
            mOpenId,
            mIso,
            idx
           );
    //
	hfg_param_t param;
    if( !get_tuning_param_HFG(sensorDev, idx, param) ) {
        SWNR_LOGE("cannot get HFG tuning param from custom, %d:dev %d, idx %d",
            mOpenId, sensorDev, idx);
        return MFALSE;
    }
    //
    memcpy(pTuningHFG, &param, sizeof(hfg_param_t));
    
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug )
        {
#define DUMP_PARAM( param )                                     \
            do {                                                \
                SWNR_LOGD("%s:%d", #param, pTuningHFG->param);  \
            } while(0)
            DUMP_PARAM(HFG_ENABLE);
            DUMP_PARAM(HFG_GSD);
            DUMP_PARAM(HFG_SD0);
            DUMP_PARAM(HFG_SD1);
            DUMP_PARAM(HFG_SD2);
            DUMP_PARAM(HFG_TX_S);
            DUMP_PARAM(HFG_LCE_LINK_EN);
            DUMP_PARAM(HFG_LUMA_CPX1);
            DUMP_PARAM(HFG_LUMA_CPX2);
            DUMP_PARAM(HFG_LUMA_CPX3);
            DUMP_PARAM(HFG_LUMA_CPY0);
            DUMP_PARAM(HFG_LUMA_CPY1);
            DUMP_PARAM(HFG_LUMA_CPY2);
            DUMP_PARAM(HFG_LUMA_CPY3);
            DUMP_PARAM(HFG_LUMA_SP0);
            DUMP_PARAM(HFG_LUMA_SP1);
            DUMP_PARAM(HFG_LUMA_SP2);
            DUMP_PARAM(HFG_LUMA_SP3);			
        }
    }
    return MTRUE;
}

MBOOL
SwNRParam::
getTuningCCR(CCRTuningInfo* pTuningCCR)
{
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    MUINT32 idx;

    // set all params to zero
    memset(pTuningCCR, 0, sizeof(CCRTuningInfo));

    idx = map_ISO_value_to_index(mIso);
    SWNR_LOGD("%d: iso %d, idx %d",
            mOpenId,
            mIso,
            idx
           );
    //
	ccr_param_t param;
    if( !get_tuning_param_CCR(sensorDev, idx, param) ) {
        SWNR_LOGE("cannot get CCR tuning param from custom, %d:dev %d, idx %d",
            mOpenId, sensorDev, idx);
        return MFALSE;
    }
    //
    memcpy(pTuningCCR, &param, sizeof(ccr_param_t));

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug )
        {
#define DUMP_PARAM( param )                                     \
            do {                                                \
                SWNR_LOGD("%s:%d", #param, pTuningCCR->param);  \
            } while(0)
            DUMP_PARAM(CCR_ENABLE);
            DUMP_PARAM(CCR_CEN_U);
            DUMP_PARAM(CCR_CEN_V);
            DUMP_PARAM(CCR_Y_CPX1);
            DUMP_PARAM(CCR_Y_CPX2);
            DUMP_PARAM(CCR_Y_CPY1);
            DUMP_PARAM(CCR_Y_SP1);
            DUMP_PARAM(CCR_UV_X1);
            DUMP_PARAM(CCR_UV_X2);
            DUMP_PARAM(CCR_UV_X3);
            DUMP_PARAM(CCR_UV_GAIN1);
            DUMP_PARAM(CCR_UV_GAIN2);
            DUMP_PARAM(CCR_UV_GAIN_SP1);
            DUMP_PARAM(CCR_UV_GAIN_SP2);
            DUMP_PARAM(CCR_Y_CPX3);
            DUMP_PARAM(CCR_Y_CPY0);
            DUMP_PARAM(CCR_Y_CPY2);
            DUMP_PARAM(CCR_Y_SP0);
            DUMP_PARAM(CCR_Y_SP2);
            DUMP_PARAM(CCR_UV_GAIN_MODE);
            DUMP_PARAM(CCR_MODE);
            DUMP_PARAM(CCR_OR_MODE);
            DUMP_PARAM(CCR_HUE_X1);
            DUMP_PARAM(CCR_HUE_X2);
            DUMP_PARAM(CCR_HUE_X3);
            DUMP_PARAM(CCR_HUE_X4);
            DUMP_PARAM(CCR_HUE_SP1);
            DUMP_PARAM(CCR_HUE_SP2);
            DUMP_PARAM(CCR_HUE_GAIN1);
            DUMP_PARAM(CCR_HUE_GAIN2);
        }
    }
    return MTRUE;
}

MBOOL
SwNRParam::
dumpParamsToFile(char * const filename)
{
    // write to file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        SWNR_LOGE("fopen fail: %s", filename);
        return MFALSE;
    }
    else {
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        MUINT const sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
        for( MUINT32 i = 0; i < eNUM_OF_SWNR_IDX; i++ )
        {
            swnr_param_t nrparam;
            if( !get_tuning_param_NR(sensorDev, (EIdxSwNR)i, nrparam) ) {
                SWNR_LOGE("cannot get tuning param from custom, %d:dev %d, idx %d",
                        mOpenId, sensorDev, i);
                break;
            }
#define FWRITE_PARAM( param )                                    \
            do {                                                 \
                fprintf(fp, "%s:%d\n", #param, nrparam.param);  \
            } while(0)
            FWRITE_PARAM(ANR_Y_LUMA_SCALE_RANGE);
            FWRITE_PARAM(ANR_C_CHROMA_SCALE);
        }
        //
        fclose(fp);
    }

    return MTRUE;
}

inline MINT32 generateSeed() { return (MINT32)::systemTime(); }
static MBOOL getSensorDevice(MUINT* pSensorDev, MUINT *pSensorDevId, MUINT32 openId)
{
    MBOOL ret = MFALSE;
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    *pSensorDev = pHalSensorList->querySensorDevIdx(openId);
    SensorStaticInfo rSensorStaticInfo;
    switch (*pSensorDev)
    {
    case SENSOR_DEV_MAIN:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        break;

    case SENSOR_DEV_SUB:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        break;

    case SENSOR_DEV_MAIN_2:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        break;

    default:    //  Shouldn't happen.
        SWNR_LOGE("Invalid sensor device: %d", *pSensorDev);
        SWNR_ASSERT(0);
    }

    *pSensorDevId = rSensorStaticInfo.sensorDevID;
    SWNR_LOGD("sensorDev(%d), u4SensorID(%d)", *pSensorDev, *pSensorDevId);

    ret = MTRUE;
lbExit:
    return ret;
}

static void freeWorkingBuf(void *pWorkBuf, size_t size)
{
    if (pWorkBuf) {
        munmap(pWorkBuf, size);
    }
}

SwNR::
SwNR(MUINT32 const openId)
    : muOpenId(openId)
    , mNRType(DRV_NR_OBJ_NONE)
    , mpMTKNR(NULL)
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
    mpNRInitInfo->pWorkingBuff = NULL;
    mpNRInitInfo->WorkingBuffSize = 0;
    mpNRInitInfo->CoreNumber = NR_MAX_CORE_NO;
    mpNRInitInfo->NumOfExecution = 1;
    mpNRInitInfo->ForcedFreq = 0;

    mpNRInitInfo->pPerfInfo = NULL;
    mpNRInitInfo->pTuningInfo = NULL;
	mpNRInitInfo->pNR2TuningInfo = NULL;
	mpNRInitInfo->pCCRTuningInfo = NULL;
	mpNRInitInfo->pHFGTuningInfo = NULL;
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
    if( mpWorkingBuf )
    {
        munmap(mpWorkingBuf, sizeof(MUINT8) * muWorkingBufSize);
        mpWorkingBuf = NULL;
    }
}

MBOOL
SwNR::
getTuningInfo()
{
    MBOOL ret = MFALSE;
    SwNRParam* pParam = SwNRParam::getInstance(muOpenId);
    MINT32 perfLevel = -1;
    MINT32 CustomerNRType;
    DrvNRObject_e NRType = DRV_NR_OBJ_NONE;
    MUINT sensorDev;    
    MUINT sensorDevID;
    //
    SWNR_ASSERT(pParam != NULL);
    //
    perfLevel = pParam->getPerfLevel();
    SWNR_ASSERT(getSensorDevice(&sensorDev, &sensorDevID, muOpenId) == MTRUE);

{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.swnr.type", value, "0");
    
    if( atoi(value) == 1)
        CustomerNRType = eSWNRType_SW;
    else if( atoi(value) == 2)
        CustomerNRType = eSWNRType_SW2;
    else
        CustomerNRType = get_swnr_type(sensorDev);
}
    SWNR_ASSERT(CustomerNRType >= 0);
    SWNR_ASSERT(CustomerNRType < eSWNRType_NUM);

    if(CustomerNRType == eSWNRType_SW)
    {	
        NRType = DRV_NR_OBJ_SW;

        if( !mpNRTuningInfo )
        {
            mpNRTuningInfo = (NRTuningInfo*)malloc(sizeof(NRTuningInfo));
            SWNR_ASSERT(mpNRTuningInfo != NULL);
        }
        SWNR_ASSERT(pParam->getTuningNR(mpNRTuningInfo));            
        mpNRInitInfo->pTuningInfo = mpNRTuningInfo;
    }    
    else
    {
        NRType = DRV_NR_OBJ_SW2;
        if( !mpNR2TuningInfo )
        {
            mpNR2TuningInfo = (NR2TuningInfo*)malloc(sizeof(NR2TuningInfo));
            SWNR_ASSERT(mpNR2TuningInfo != NULL);
        }
        SWNR_ASSERT(pParam->getTuningNR2(mpNR2TuningInfo));

        // update some parameters
        mpNR2TuningInfo->NR_SD = generateSeed();
        mpNRInitInfo->pNR2TuningInfo = mpNR2TuningInfo;
    }
	if( !mpHFGTuningInfo )
    {
        mpHFGTuningInfo = (HFGTuningInfo*)malloc(sizeof(HFGTuningInfo));
        SWNR_ASSERT(mpHFGTuningInfo != NULL);
    }
	if( !mpCCRTuningInfo )
    {
        mpCCRTuningInfo = (CCRTuningInfo*)malloc(sizeof(CCRTuningInfo));
        SWNR_ASSERT(mpCCRTuningInfo != NULL);

    }
    SWNR_ASSERT(pParam->getTuningHFG(mpHFGTuningInfo));
    SWNR_ASSERT(pParam->getTuningCCR(mpCCRTuningInfo));
    
    // update some parameters
    mpHFGTuningInfo->HFG_GSD = generateSeed();
    mpNRInitInfo->pHFGTuningInfo = mpHFGTuningInfo;
    mpNRInitInfo->pCCRTuningInfo = mpCCRTuningInfo;
    //
    ret = MTRUE;
lbExit:
    //
    mNRType = NRType;

    // set performance level
    if( perfLevel != -1 ) {
        if( mpNRInitInfo->pPerfInfo == NULL ) {
            mpNRInitInfo->pPerfInfo = new NRPerfGear;
        }
        //
        switch(perfLevel)
        {
#define PerfCase(val, level)                             \
            case val:                                    \
                mpNRInitInfo->pPerfInfo->enable = 1;     \
                mpNRInitInfo->pPerfInfo->option = level; \
                break;
            PerfCase(2, NR_PERF_GEAR_PERF_FIRST)
            PerfCase(1, NR_PERF_GEAR_CUSTOM_0)
            PerfCase(0, NR_PERF_GEAR_POWER_FIRST)
            default:
                SWNR_LOGW("not valid level %d", perfLevel);
                delete mpNRInitInfo->pPerfInfo;
                mpNRInitInfo->pPerfInfo = NULL;
#undef PerfCase
        }
    }
    //    
    return ret;
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
            SWNR_LOGE("not supported format: 0x%x", pBuf->getImgFormat() );
            goto lbExit;
    }

    // set image info
    mpNRImageInfo->Width = pBuf->getImgSize().w;
    mpNRImageInfo->Height = pBuf->getImgSize().h;
    mpNRImageInfo->pImg[0] = (void*)pBuf->getBufVA(0);
    if( NRFmt == NR_IMAGE_YUV420 )
    {
        mpNRImageInfo->pImg[1] = (void*)pBuf->getBufVA(1);
        mpNRImageInfo->pImg[2] = (void*)pBuf->getBufVA(2);
    }
    else
    {
        mpNRImageInfo->pImg[1] = (void*)0;
        mpNRImageInfo->pImg[2] = (void*)0;
    }
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
            munmap(mpWorkingBuf, muWorkingBufSize);
        mpWorkingBuf = (MUINT8*)mmap(NULL, sizeof(MUINT8) * size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        /*
        if( mpWorkingBuf == MAP_FAILED )
        {
            SWNR_LOGE("allocate working buffer failed");
            goto lbExit;
        }
        */
        SWNR_ASSERT(mpWorkingBuf != MAP_FAILED);

        SWNR_LOGD("allocate working buffer %p, size %d", mpWorkingBuf, size);
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
doSwNR(IImageBuffer* pBuf)
{
    SWNR_LOGD("idx %d, buf %p", muOpenId, pBuf);
    MBOOL ret = MFALSE;
    MUINT32 workingbufsize = 0;

    // get tuning info
    SWNR_ASSERT(getTuningInfo() == MTRUE);

    // create MTKNR
    mpMTKNR = MTKNR::createInstance(mNRType);    
    SWNR_ASSERT(mpMTKNR != NULL);

    // init
    SWNR_ASSERT(mpMTKNR->NRInit((MUINT32*) mpNRInitInfo, 0) == S_NR_OK);

    // setImageInfo
    SWNR_ASSERT(setImageInfo(pBuf) != 0);

    // GetWorkbufSize
    SWNR_ASSERT(mpMTKNR->NRFeatureCtrl(NR_FEATURE_GET_WORKBUF_SIZE, (void*)mpNRImageInfo, (void*)&workingbufsize)==S_NR_OK);

    // allocWorkingBuf
    SWNR_ASSERT(allocWorkingBuf(workingbufsize));

    // SetWorkBufAddr
    SWNR_ASSERT(mpMTKNR->NRFeatureCtrl(NR_FEATURE_SET_WORKBUF_ADDR, mpNRInitInfo->pWorkingBuff, NULL) == S_NR_OK);

    // Add img
    SWNR_ASSERT(mpMTKNR->NRFeatureCtrl(NR_FEATURE_ADD_IMG, (void*)mpNRImageInfo, NULL) == S_NR_OK);

    // NR Main
    SWNR_LOGD("start+");
    SWNR_ASSERT(mpMTKNR->NRMain() == S_NR_OK);
    SWNR_LOGD("start-");

    // GetResult
    SWNR_ASSERT(mpMTKNR->NRFeatureCtrl(NR_FEATURE_GET_RESULT, NULL, (void*)mpNRResultInfo) == S_NR_OK);

    pBuf->syncCache(eCACHECTRL_FLUSH);

    mpMTKNR->NRReset();

    ret = MTRUE;
lbExit:
    // freeWorkingBuf
    if (mpNRInitInfo->pWorkingBuff) {
        freeWorkingBuf(mpNRInitInfo->pWorkingBuff, workingbufsize);
        mpWorkingBuf = NULL;
        mpNRInitInfo->pWorkingBuff = NULL;
        workingbufsize = 0;
    }
    if( mpMTKNR ) {
        mpMTKNR->destroyInstance(mpMTKNR);
        mpMTKNR = NULL;
    }

    return ret;
}
