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
#include <mtkcam/utils/std/Log.h>
#define SWNR_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/def/common.h>
//
#include <utils/StrongPointer.h>

//
//
#include "capturenr.h"
#include <sys/mman.h>
#include <camera_custom_capture_nr.h>

// custom tuning
#include <mtkcam/drv/IHalSensor.h>
#include <camera_custom_nvram.h>
#include <nvbuf_util.h>
//
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/cam/dbg_cam_param.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//
#include <aaa_types.h>
#include <isp_tuning.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_custom.h>
#include <isp_tuning/isp_tuning_custom_swnr.h>
#include <ispfeature.h>
#include <isp_interpolation/isp_interpolation.h>
#include "property_utils.h"
#include <vendor/mediatek/hardware/power/2.0/IPower.h>
using namespace vendor::mediatek::hardware::power::V2_0;

#include "paramctrl.h"
#include "ui/GraphicBuffer.h"
#include "mtkcam/utils/imgbuf/IGrallocImageBufferHeap.h"
#include "utils/std/Trace.h"
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#define GPUVERSION (0)
#define SWNR_ASSERT(x)                                                      \
do {                                                                        \
    MBOOL __x = (x);                                                        \
    if (!__x) {                                                             \
        SWNR_LOGE("[%s:%d] ASSERT FAILED: %s", __FUNCTION__, __LINE__, #x); \
        goto lbExit;                                                        \
    }                                                                       \
} while(0);

using namespace android;
using namespace NSCam;
using namespace NSIspTuning;
/*******************************************************************************
*
********************************************************************************/
// these assertions are to make sure nvram is large enough
static_assert(
        sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) >= (sizeof(NR_PARAM) + sizeof(HFG_PARAM) + sizeof(CCR_PARAM)),
        "should check NVRAM_CAMERA_FEATURE_SWNR_STRUCT"
        );

static_assert(
        sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) >=
        (sizeof(NR2TuningInfo) +
         sizeof(CCRTuningInfo) + sizeof(HFGTuningInfo)),
        "[FIXME] should modify swnr nvram struct"
        );

static_assert(
        sizeof(NR_PARAM) == sizeof(NR2TuningInfo),
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


typedef enum {
    SWNR_SCENARIO_NORMAL = 0,
    SWNR_SCENARIO_MFNR,
    SWNR_SCENARIO_DUAL,
} SWNR_SCENARIO;


static const char* SWNR_SCENARIO_toString(SWNR_SCENARIO scenario)
{
    switch (scenario)
    {
    case SWNR_SCENARIO_MFNR:
        return "MFNR";
    case SWNR_SCENARIO_DUAL:
        return "DUAL";
    case SWNR_SCENARIO_NORMAL:
        return "NORM";
    default:
        return "UNKNOWN";
    }
}


/*******************************************************************************
*
********************************************************************************/
inline MINT32 generateSeed() { return (MINT32)::systemTime(); }

#define DUMP_PARAM(fp, fmt, arg...)            \
    do {                                       \
        if (fp ) fprintf(fp, fmt "\n", ##arg); \
        else SWNR_LOGD(fmt, ##arg);              \
    } while(0)

/*******************************************************************************
*
********************************************************************************/
static const char* NRType_String(DrvNRObject_e NRType)
{
    switch (NRType)
    {
    case DRV_NR_OBJ_SW2:
        return "Fast SwNR";
    case DRV_NR_OBJ_SW2_VPU:
        return "Fast SwNR VPU";
    /*case DRV_NR_OBJ_SW2_HIGH_PERF:
        return "Fast SwNR (high performance)";
    case DRV_NR_OBJ_SW2_LOW_POWER:
        return "Fast SwNR (low power)";
    case DRV_NR_OBJ_NONE:
    case DRV_NR_OBJ_UNKNOWN:*/
    default:
        return "Unsupported SwNR Type";
    }
}


static DrvNRObject_e ESWNRType_2_DrvNRObject(ESWNRType CustomerNRType)
{
    switch (CustomerNRType)
    {
    case eSWNRType_SW2:
        return DRV_NR_OBJ_SW2;
    case eSWNRType_SW2_VPU:
        return DRV_NR_OBJ_SW2_VPU;
    /*case eSWNRType_SW2_HIGH_PERF:
        return DRV_NR_OBJ_SW2_HIGH_PERF;
    case eSWNRType_SW2_LOW_POWER:
        return DRV_NR_OBJ_SW2_LOW_POWER;*/
    default:
        return DRV_NR_OBJ_UNKNOWN;
    }
}





static MVOID dumpParams(NR2TuningInfo* pNR, FILE* fp = NULL)
{
    if (! pNR) {
        SWNR_LOGE("null");
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
    DUMP_NR2(NR_NTRL_TH_1_Y);
    DUMP_NR2(NR_NTRL_TH_2_Y);
    DUMP_NR2(NR_NTRL_TH_1_UV);
    DUMP_NR2(NR_NTRL_TH_2_UV);
#undef DUMP_NR2
}

static MVOID dumpParams(HFGTuningInfo* pHFG, FILE* fp = NULL)
{
    if (!pHFG) {
        SWNR_LOGE("null");
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


static MVOID dumpParams(CCRTuningInfo* pCCR, FILE* fp = NULL)
{
    if (! pCCR) {
        SWNR_LOGE("null");
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


static const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *getTuningFromNvram(MUINT32 openId, SWNR_SCENARIO scenario, MUINT32 idx, MINT32 magicNo)
{
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *pNRNvram;

    if (idx >= EISO_NUM) {
        SWNR_LOGE("wrong nvram idx %d", idx);
        return NULL;
    }

    // load some setting from nvram
	MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    CAM_IDX_QRY_COMB rMapping_Info;
    pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);
    idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_SWNR, rMapping_Info, LOG_TAG);
    SWNR_LOGD("query nvram(%d) index: %d ispprofile: %d sensor: %d ismfll(%d)", magicNo, idx, rMapping_Info.eIspProfile, rMapping_Info.eSensorMode, scenario == SWNR_SCENARIO_MFNR );

    err = NvBufUtil::getInstance().getBufAndRead(
            CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)pNvram);
    if (err != 0)
    {
        SWNR_LOGE("getBufAndRead fail, err=%d", err);
        return NULL;
    }
    pNRNvram = &(pNvram->SWNR[idx]);

    return pNRNvram;
}


static MBOOL getSensorDevice(MUINT* pSensorDev, MUINT *pSensorDevId, MUINT32 openId)
{
    MBOOL ret = MFALSE;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    *pSensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);
    SensorStaticInfo rSensorStaticInfo;
    if (pHalSensorList == NULL)
    {
        MY_ERR("pHalSensorList is NULL!!");
        SWNR_ASSERT(0);
    }
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
        MY_ERR("Invalid sensor device: %d", *pSensorDev);
        SWNR_ASSERT(0);
    }

    *pSensorDevId = rSensorStaticInfo.sensorDevID;
    SWNR_LOGD("sensorDev(%d), u4SensorID(%d)", *pSensorDev, *pSensorDevId);

    ret = MTRUE;
lbExit:
    return ret;
}


MBOOL allocWorkingBuf(sp<NSCam::IImageBuffer>& pImgBuf,  NRWorkingBufferInfo &MyWorkingBufInfo)
{
    // 4. set buffer address
    MBOOL ret = MFALSE;
    MUINT32 i;
    MUINT8 *pBuffer;
    MINT32 GBWorkBufNum =0;
    for (i=0; i<MyWorkingBufInfo.WorkingBufferNum; i++)
    {
        NRBuffer *pWorkingBuffer = &(MyWorkingBufInfo.WorkingBuffer[i]);
        if(pWorkingBuffer->MemType == NR_MEMORY_ION) {
            if(pWorkingBuffer->ImgFmt != NR_IMAGE_Y8 ||
               pWorkingBuffer->Size == 0 ||
               pWorkingBuffer->BufferNum != 1) {
                SWNR_LOGE("Get NR_FEATURE_GET_WORKBUF_SIZE info error (MemType: %d, ImgFmt: %d, Size: %d, BufferNum: %d,)",
                    pWorkingBuffer->MemType,
                    pWorkingBuffer->ImgFmt,
                    pWorkingBuffer->Size,
                    pWorkingBuffer->BufferNum);
                SWNR_ASSERT(0);
            }
            size_t bufBoundaryInBytes[3] = {0};
            if(!pImgBuf.get()){
                IImageBufferAllocator::ImgParam imgParam =
                            IImageBufferAllocator::ImgParam(
                                pWorkingBuffer->Size,
                                (*bufBoundaryInBytes)
                            );

                IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
                pImgBuf = allocator->alloc_ion(
                                LOG_TAG,
                                imgParam
                                );
                pImgBuf->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK);
            }
            if(pWorkingBuffer->Size > pImgBuf->getBufSizeInBytes(0))
                SWNR_LOGE("SWNRAlgo want %d bytes wxh(%dx%d), but real size is %d", pWorkingBuffer->Size, pWorkingBuffer->Width, pWorkingBuffer->Height, pImgBuf->getBufSizeInBytes(0));
            pBuffer = (MUINT8*)pImgBuf->getBufVA(0);
            pWorkingBuffer->pBuff[0] = (void*)pBuffer;
            pWorkingBuffer->BuffFD[0] = pImgBuf->getFD(0);
            SWNR_LOGD("Working Buffer FD %d", pImgBuf->getFD(0));
        } else if(pWorkingBuffer->MemType == NR_MEMORY_GRALLOC_Y8) {
            SWNR_LOGD("working buffer: GrphicBuffer");
            const int gbTexUsage =  GraphicBuffer::USAGE_SW_WRITE_RARELY;   // GraphicBuffer::USAGE_HW_TEXTURE |
            const int gbTexFormat = HAL_PIXEL_FORMAT_Y8; //PIXEL_FORMAT_RGBA_8888/ PIXEL_FORMAT_RGB_888

            sp<GraphicBuffer> *gb_work_buf = new sp<GraphicBuffer>;
            *gb_work_buf = new GraphicBuffer(pWorkingBuffer->Width, pWorkingBuffer->Height, gbTexFormat, gbTexUsage);
            gb_work_buf->get()->incStrong(gb_work_buf->get());
            pWorkingBuffer->pBuff[0] = (void*)gb_work_buf;
            ++GBWorkBufNum;
        } else {
            SWNR_LOGE("Shuld Not Be Here!!");
            SWNR_ASSERT(0);
        }
    }

    ret = MTRUE;
lbExit:
    return ret;
}


static void freeWorkingBuf(sp<NSCam::IImageBuffer>& pImgBuf, NRWorkingBufferInfo &MyWorkingBufInfo)
{
    MUINT32 i;
    for (i=0; i<MyWorkingBufInfo.WorkingBufferNum; i++)
    {
        NRBuffer *pWorkingBuffer = &(MyWorkingBufInfo.WorkingBuffer[i]);
        void *pBuffer = pWorkingBuffer->pBuff[0];
        if (pBuffer)
        {
            if (pWorkingBuffer->MemType == NR_MEMORY_ION) {
                 if(pImgBuf.get() != NULL)
                    pImgBuf->unlockBuf(LOG_TAG);
                 IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
                 allocator->free(pImgBuf.get());
                 pImgBuf = NULL;
            } else if(pWorkingBuffer->MemType == NR_MEMORY_GRALLOC_Y8) {
                sp<GraphicBuffer> *gb_work_buf = (sp<GraphicBuffer>*)pBuffer;
                gb_work_buf->get()->decStrong(gb_work_buf->get());
                delete gb_work_buf;
            }
            else
                SWNR_LOGW("CAN't free buffer MemType 0x%x", pWorkingBuffer->MemType);
        }
        pWorkingBuffer->pBuff[0] = NULL;
    }
}
static MUINT32 getNeighber(Paramctrl* pParamctrl, MUINT32 iso, MUINT32& iso_low, MUINT32& iso_high)
{


    EISO_GROUP_T temp_ISO_Group = EISO_GROUP_00;

    // need query function to get ISO Group

    MUINT32 idx = (MUINT32)pParamctrl->map_ISO_value2index(iso, temp_ISO_Group);
    // find lower and upper bound
    {
        MUINT32 iso2;
        iso2 = pParamctrl->map_ISO_index2value((EISO_T)idx, temp_ISO_Group);
        if (iso2 > iso) {
            if (idx > 0) {
                idx -= 1;
                iso_low = pParamctrl->map_ISO_index2value((EISO_T)idx, temp_ISO_Group);
                iso_high = iso2;
            } else {
                iso_low = iso_high = iso2;
            }
        } else {
            iso_low = iso2;
            iso_high = pParamctrl->map_ISO_index2value((EISO_T)(idx + 1), temp_ISO_Group);
        }
    }
    return idx;
}

static MBOOL getTuningInfo(
    SwNR::TUNINGINFO *pTuningInfo,  // output
    DrvNRObject_e *pNRType,         // output
    MUINT32 iso,
    SWNR_SCENARIO scenario,
    MUINT32 openId,
    MINT32 magicNo)
{
    MBOOL ret = MFALSE;
    IspTuningCustom *pIspTuningCustom = NULL;
    DrvNRObject_e NRType = DRV_NR_OBJ_NONE;

    MUINT sensorDevID;
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT NvramData;
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);
    Paramctrl* pParamctrl = Paramctrl::getInstance((ESensorDev_T)sensorDev, openId);
    MUINT32 iso_low, iso_high;
    MUINT32 idx;

    SWNR_ASSERT(getSensorDevice(&sensorDev, &sensorDevID, openId) == MTRUE);

    pIspTuningCustom = IspTuningCustom::createInstance(
            static_cast<NSIspTuning::ESensorDev_T>(sensorDev),
            sensorDevID);
    SWNR_ASSERT(pIspTuningCustom != NULL);

    // 1. update swnr type
    NRType = ESWNRType_2_DrvNRObject((ESWNRType)get_swnr_type(sensorDev));// eSWNRType_SW2 eSWNRType_SW2_VPU
    {
        // force use swnr (for debug)
        MINT32 type = 0;
        getPropInt("vendor.debug.swnr.type", &type, -1);
        if (type != -1) {
            NRType = ESWNRType_2_DrvNRObject((ESWNRType)type);
            SWNR_LOGD("force use SwNR type: %s", NRType_String(NRType));
        }
    }

    SWNR_ASSERT(NRType != DRV_NR_OBJ_UNKNOWN);

    idx = getNeighber(pParamctrl, iso, iso_low, iso_high);

    SWNR_LOGD("%d: iso=%d (%d~%d), isMfll=%d NRType:%d", openId, iso, iso_low, iso_high, scenario, NRType);


    // 2. read from nvram & smooth
#warning FIXME: temp mark(0) to close interpolation flow. Need open after SmoothFSWNR fix!
    if (is_to_invoke_swnr_interpolation(scenario, iso) && (iso_low != iso_high) && (0))
    {
        const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *pNvramLow   = getTuningFromNvram(openId, scenario, idx, magicNo);
        const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *pNvramHigh  = getTuningFromNvram(openId, scenario, idx + 1, magicNo);
        if (!pNvramLow || !pNvramHigh)
        {
            SWNR_LOGE("update from nvram failed");
            return MFALSE;
        }

        #if 0
        // Need to fix
        // interpolation
        auto interpolation_func = SmoothFSWNR;
        interpolation_func(
                iso,
                iso_high, iso_low,
                *pNvramHigh,
                *pNvramLow,
                NvramData);
        #endif
    } else {
        // get nvram
        const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *pTuningNvram = getTuningFromNvram(openId, scenario, idx, magicNo);
        if (!pTuningNvram) {
            SWNR_LOGE("update from nvram failed");
            return MFALSE;
        }

        // update
        memcpy(&NvramData, pTuningNvram, sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT));
    }
    // get tuning info from NVRam
    {
        // DRV_NR_OBJ_SW2, DRV_NR_OBJ_SW2_HIGH_PERF, DRV_NR_OBJ_SW2_LOW_POWER
        memcpy(&pTuningInfo->NR2,  &(NvramData.NR) , sizeof(NR_PARAM));
        memcpy(&pTuningInfo->HFG, &(NvramData.HFG), sizeof(HFG_PARAM));
        memcpy(&pTuningInfo->CCR, &(NvramData.CCR), sizeof(CCR_PARAM));
        // update some parameters
        pTuningInfo->NR2.NR_SD      = generateSeed();
        pTuningInfo->HFG.HFG_GSD    = generateSeed();
    }

    {
        MBOOL bDebug = MFALSE;
        getPropInt("vendor.debug.swnr.enable", &bDebug, 0);

        if (bDebug) {
            dumpParams(&pTuningInfo->NR2);
            dumpParams(&pTuningInfo->HFG);
            dumpParams(&pTuningInfo->CCR);
        }
    }

    ret = MTRUE;
lbExit:
    *pNRType = NRType;
    if (pIspTuningCustom) pIspTuningCustom->destroyInstance();
    if (pParamctrl) pParamctrl->destroyInstance();
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

ISwNR*
ISwNR::
createInstance(MUINT32 const openId)
{
    return new SwNR(openId);
}

static MBOOL setImageInfo(NRImageInfo &MyNRImageInfo, const IImageBuffer* pBuf, DrvNRObject_e NRType)
{
    MBOOL ret = MFALSE;
    MTK_NR_IMAGE_FMT_ENUM NRFmt = NR_IMAGE_Y8;
    IImageBufferHeap *heap = NULL;

    switch( pBuf->getImgFormat() )
    {
        case eImgFmt_Y8:
            NRFmt = NR_IMAGE_Y8;
            break;
        case eImgFmt_I420:
            NRFmt = NR_IMAGE_YUV420;
            break;
        case eImgFmt_YV12:
            NRFmt = NR_IMAGE_YV12;
            break;
        default:
            SWNR_LOGE("not supported format: 0x%x", pBuf->getImgFormat() );
            SWNR_ASSERT(0);
    }

    // set image info
    MyNRImageInfo.Width = pBuf->getImgSize().w;
    MyNRImageInfo.Height = pBuf->getImgSize().h;

    if (NRType==DRV_NR_OBJ_SW2 || NRType==DRV_NR_OBJ_SW2_VPU) {
        MyNRImageInfo.pImg[0] = (void*)pBuf->getBufVA(0);
        MyNRImageInfo.pImg[1] = pBuf->getPlaneCount() > 1 ? (void*)pBuf->getBufVA(1) : (void*)0;
        MyNRImageInfo.pImg[2] = pBuf->getPlaneCount() > 2 ? (void*)pBuf->getBufVA(2) : (void*)0;
    }
#if GPUVERSION
    else {
        SWNR_LOGD("[%s:%d] plane=%d", __FUNCTION__, __LINE__, pBuf->getPlaneCount());
        heap = pBuf->getImageBufferHeap();
        MyNRImageInfo.pImg[0] = (void*)heap->getGraphicBuffer(0);
        MyNRImageInfo.pImg[1] = pBuf->getPlaneCount() > 1 ? (void*)heap->getGraphicBuffer(1) : (void*)0;
        MyNRImageInfo.pImg[2] = pBuf->getPlaneCount() > 2 ? (void*)heap->getGraphicBuffer(2) : (void*)0;
    }
#endif
    MyNRImageInfo.ImgFD[0] = pBuf->getFD(0);
    MyNRImageInfo.ImgFD[1] = pBuf->getPlaneCount() > 1 ? pBuf->getFD(1) : -1;
    MyNRImageInfo.ImgFD[2] = pBuf->getPlaneCount() > 2 ? pBuf->getFD(2) : -1;
    MyNRImageInfo.ImgFmt = NRFmt;
    SWNR_LOGD("Buffer FD %d/%d/%d", MyNRImageInfo.ImgFD[0], MyNRImageInfo.ImgFD[1], MyNRImageInfo.ImgFD[2]);
    ret = MTRUE;
lbExit:
    return ret;
}

SwNR::SwNR(MUINT32 const openId)
    : muOpenId(openId)
    , mNRType(DRV_NR_OBJ_NONE)
    , mpWb(nullptr)
{
    getPropInt("vendor.debug.camera.capturenr.wb", &mDbgReuseWb, 1);
    if(mDbgReuseWb) {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(openId);
        if(!pMetadataProvider.get())
            SWNR_LOGE(" !pMetadataProvider.get() ");
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        MRect activeArray;
        if (tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray)) {
            CAM_TRACE_BEGIN("swnr:allocWb");
            size_t bufBoundaryInBytes[3] = {0};
            IImageBufferAllocator::ImgParam imgParam =
                        IImageBufferAllocator::ImgParam(
                            NR2_BUFFER_SIZE(activeArray.s.w, activeArray.s.h),
                            (*bufBoundaryInBytes)
                        );
            IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
            mpWb = allocator->alloc_ion(
                            LOG_TAG,
                            imgParam
                            );
            mpWb->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_MASK);
            CAM_TRACE_END();
            SWNR_LOGD("get active array(%d, %d, %dx%d) to alloc Wb(%d)",
                      activeArray.p.x, activeArray.p.y,
                      activeArray.s.w, activeArray.s.h, (int)NR2_BUFFER_SIZE(activeArray.s.w, activeArray.s.h));
        } else {
            SWNR_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
        }
    }
    memset(&mTuningInfo, 0, sizeof(mTuningInfo));
}

SwNR::~SwNR()
{
    if(mDbgReuseWb) {
        if(mpWb.get() != NULL)
            mpWb->unlockBuf(LOG_TAG);
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        allocator->free(mpWb.get());
        mpWb = NULL;
    }
}

MBOOL SwNR::doSwNR(SWNRParam const& param, NSCam::IImageBuffer* pBuf)
{
    return doSwNR(param, pBuf, -1);
}

MBOOL SwNR::doSwNR(SWNRParam const& param, NSCam::IImageBuffer* pBuf, MINT32 magicNo)
{
    MBOOL ret = MFALSE;
    SWNR_SCENARIO scenario;

    MTKNR2* MyNR = NULL;
    MTKNRGPU* MyGPU = NULL;

    NRInitInfo          MyNRInitInfo;
    NRWorkingBufferInfo MyWorkingBufInfo;
    NRImageInfo         MyNRImageInfo;
    sp<IPower> pPerf = IPower::getService();
    MINT32 handle_perf_serv_iir = -1;
    MUINT32 coreNum = 0;
    MUINT32 numOfCluster = 0;
    MUINT32 numOfCPU = 0;
    sp<IImageBuffer> pWorkingImgBuf = mDbgReuseWb ? mpWb : NULL;

    SWNR_LOGD("idx %d: iso %d perf %d, buf %p", muOpenId, param.iso, param.perfLevel, pBuf);

    memset(&MyWorkingBufInfo, 0, sizeof(MyWorkingBufInfo));

    // get tuning info
    if (param.isMfll) scenario = SWNR_SCENARIO_MFNR;
    else if (param.isDualCam) scenario = SWNR_SCENARIO_DUAL;
    else scenario = SWNR_SCENARIO_NORMAL;
    char filename[PROPERTY_VALUE_MAX] = {'\0'};
    SWNR_ASSERT(getTuningInfo(&mTuningInfo, &mNRType, param.iso, scenario, muOpenId, magicNo) != 0);
    // debug: read from file
    if( property_get("vendor.debug.swnr.readfile", filename, "") > 0 ) {
        memset(&mTuningInfo, 0, sizeof(mTuningInfo));
        getTuningFromFile(filename, mTuningInfo);
    }
    SWNR_LOGD("SwNR type: %s", NRType_String(mNRType));

    numOfCluster = pPerf->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_NUM, 0);

    // set MyNRInitInfo
    for(int i=0;i<numOfCluster;i++)
    {
        numOfCPU = pPerf->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_CPU_NUM, i);
        coreNum += numOfCPU;
    }
    MyNRInitInfo.CoreNumber     = coreNum;
    MyNRInitInfo.NumOfExecution = 1;
    MyNRInitInfo.pNR2TuningInfo    = &mTuningInfo.NR2;
    MyNRInitInfo.pCCRTuningInfo = &mTuningInfo.CCR;
    MyNRInitInfo.pHFGTuningInfo = &mTuningInfo.HFG;

    // 1. create instance
    MyNR = MTKNR2::createInstance(mNRType); // DRV_NR_OBJ_SW2_LOW_POWER, DRV_NR_OBJ_SW2_LOW_POWER, DRV_NR_OBJ_SW2_HIGH_PERF
    SWNR_ASSERT(MyNR != NULL);
#if GPUVERSION
    if (mNRType==DRV_NR_OBJ_SW2_HIGH_PERF || mNRType==DRV_NR_OBJ_SW2_LOW_POWER)
    {
        MyGPU = MTKNRGPU::createInstance(mNRType);
        SWNR_ASSERT(MyGPU != NULL);
        SWNR_ASSERT(MyGPU->NRGPUInit(NULL)==S_NR_OK);
    }
#endif
    // 2. init
    SWNR_ASSERT(MyNR->NRInit(&MyNRInitInfo, 0) == S_NR_OK);

    // 3. get buffer size
    MyWorkingBufInfo.Width = pBuf->getImgSize().w;
    MyWorkingBufInfo.Height = pBuf->getImgSize().h;
    SWNR_ASSERT(MyNR->NRFeatureCtrl(NR_FEATURE_GET_WORKBUF_SIZE, (void*)&MyWorkingBufInfo, NULL) == S_NR_OK);

    SWNR_ASSERT(allocWorkingBuf(pWorkingImgBuf, MyWorkingBufInfo) == MTRUE);
    // 4. set buffer address
    SWNR_ASSERT(MyNR->NRFeatureCtrl(NR_FEATURE_SET_WORKBUF_ADDR, (void*)&MyWorkingBufInfo, NULL) == S_NR_OK);

    // 5. get src image requirement
    SWNR_ASSERT(MyNR->NRFeatureCtrl(NR_FEATURE_GET_INPUT_FMT, (void *)&MyNRImageInfo, NULL) == S_NR_OK);
    SWNR_ASSERT(setImageInfo(MyNRImageInfo, pBuf, mNRType) == MTRUE);

    CAM_TRACE_BEGIN("enablePerformanceService");
    enablePerformanceService(handle_perf_serv_iir, param.perfLevel);
    CAM_TRACE_END();
    // 6. Main
    MyNRImageInfo.Instance = MyGPU;
    NRVerInfo Version;
    SWNR_ASSERT(MyNR->NRFeatureCtrl(NR_FEATURE_GET_VERSION, NULL, &Version) == S_NR_OK);
    CAM_TRACE_BEGIN("ALGO:swnr main");
    SWNR_LOGD("start+");
    SWNR_ASSERT(MyNR->NRMain(&MyNRImageInfo, NULL) == S_NR_OK);
    SWNR_LOGD("start-");
    CAM_TRACE_END();
    if(mNRType!=DRV_NR_OBJ_SW2_VPU||mTuningInfo.HFG.HFG_ENABLE||mTuningInfo.CCR.CCR_ENABLE)
    {
        CAM_TRACE_BEGIN("sysncCache");
        SWNR_LOGD("sync cache+");
        pBuf->syncCache(eCACHECTRL_FLUSH);
        SWNR_LOGD("sync cache-");
        CAM_TRACE_END();
    }
    CAM_TRACE_BEGIN("disablePerformanceService");
    disablePerformanceService(handle_perf_serv_iir);
    CAM_TRACE_END();
    // 7. reset
    MyNR->NRReset();

    ret = MTRUE;
lbExit:
    // freeWorkingBuf
    if(!mDbgReuseWb) {
        freeWorkingBuf(pWorkingImgBuf, MyWorkingBufInfo);
        pWorkingImgBuf = NULL;
    }
#if GPUVERSION
    // 8. destroy (MyGPU)
    if (MyGPU) {
        MyGPU->NRGPUReset();
        MyGPU->destroyInstance(MyGPU);
        MyGPU = NULL;
    }
#endif
    // 8. destroy (MyNR)
    if (MyNR) {
        MyNR->destroyInstance(MyNR);
        MyNR = NULL;
    }

    return ret;
}

MBOOL SwNR::dumpDbgToFile(char * const filename, const void * p) const
{
    SWNR_LOGD("dump dbg to file +");
    FILE * fp = fopen(filename, "w+b");
    if(!fp) {
        MY_LOGE("fopen fail: %s", filename);
        return MFALSE;
    }
    //
    DEBUG_RESERVEA_INFO_T *pbuf = (DEBUG_RESERVEA_INFO_T *)p;
    fwrite(pbuf, sizeof(MINT32), sizeof(DEBUG_RESERVEA_INFO_T)/sizeof(MINT32), fp);
    fclose(fp);
    SWNR_LOGD("dump dbg to file -");
    return MTRUE;
}

MBOOL SwNR::getTuningFromFile(char * const filename, SwNR::TUNINGINFO& pTuningInfo)
{
    SWNR_LOGD("read from file +");
    FILE * fp = fopen(filename, "rb");
    if(!fp) {
        MY_LOGE("fopen fail: %s", filename);
        return MFALSE;
    }
    DEBUG_RESERVEA_INFO_T p;
    fread(&p, sizeof(DEBUG_RESERVEA_INFO_T), 1, fp);
    fclose(fp);

#if 0
    FILE * fp1 = fopen("sdcard/camera_dump/swnr_debug.dump", "wb");
    if(!fp1) {
        MY_LOGE("fopen fail --: %s", "sdcard/camera_dump/swnr_debug.dump");
        return MFALSE;
    }
    fwrite(&p, sizeof(DEBUG_RESERVEA_INFO_T), 1, fp1);
    fclose(fp1);
#endif

    ssize_t index = 0;

#define read(dst)           \
        do{                                  \
            if(index >= sizeof(DEBUG_RESERVEA_INFO_T)/sizeof(debug_exif_field)) \
                SWNR_LOGE("out of bound");   \
            else                             \
                dst = p.Tag[index].u4FieldValue; \
            SWNR_LOGD("%s:%d", #dst, dst); \
            index++;                         \
        } while(0)

    MINT32 version;
    read(version);
    read(pTuningInfo.NR2.NR_K);
    read(pTuningInfo.NR2.NR_S);
    read(pTuningInfo.NR2.NR_SD);
    read(pTuningInfo.NR2.NR_BLD_W);
    read(pTuningInfo.NR2.NR_BLD_TH);
    read(pTuningInfo.NR2.NR_SMTH);
    read(pTuningInfo.NR2.NR_NTRL_TH_1_Y);
    read(pTuningInfo.NR2.NR_NTRL_TH_2_Y);
    read(pTuningInfo.NR2.NR_NTRL_TH_1_UV);
    read(pTuningInfo.NR2.NR_NTRL_TH_2_UV);
    // HFG
    read(pTuningInfo.HFG.HFG_ENABLE);
    read(pTuningInfo.HFG.HFG_GSD);
    read(pTuningInfo.HFG.HFG_SD0);
    read(pTuningInfo.HFG.HFG_SD1);
    read(pTuningInfo.HFG.HFG_SD2);
    read(pTuningInfo.HFG.HFG_TX_S);
    read(pTuningInfo.HFG.HFG_LCE_LINK_EN);
    read(pTuningInfo.HFG.HFG_LUMA_CPX1);
    read(pTuningInfo.HFG.HFG_LUMA_CPX2);
    read(pTuningInfo.HFG.HFG_LUMA_CPX3);
    read(pTuningInfo.HFG.HFG_LUMA_CPY0);
    read(pTuningInfo.HFG.HFG_LUMA_CPY1);
    read(pTuningInfo.HFG.HFG_LUMA_CPY2);
    read(pTuningInfo.HFG.HFG_LUMA_CPY3);
    read(pTuningInfo.HFG.HFG_LUMA_SP0);
    read(pTuningInfo.HFG.HFG_LUMA_SP1);
    read(pTuningInfo.HFG.HFG_LUMA_SP2);
    read(pTuningInfo.HFG.HFG_LUMA_SP3);
    // CCR
    read(pTuningInfo.CCR.CCR_ENABLE);
    read(pTuningInfo.CCR.CCR_CEN_U);
    read(pTuningInfo.CCR.CCR_CEN_V);
    read(pTuningInfo.CCR.CCR_Y_CPX1);
    read(pTuningInfo.CCR.CCR_Y_CPX2);
    read(pTuningInfo.CCR.CCR_Y_CPY1);
    read(pTuningInfo.CCR.CCR_Y_SP1);
    read(pTuningInfo.CCR.CCR_UV_X1);
    read(pTuningInfo.CCR.CCR_UV_X2);
    read(pTuningInfo.CCR.CCR_UV_X3);
    read(pTuningInfo.CCR.CCR_UV_GAIN1);
    read(pTuningInfo.CCR.CCR_UV_GAIN2);
    read(pTuningInfo.CCR.CCR_UV_GAIN_SP1);
    read(pTuningInfo.CCR.CCR_UV_GAIN_SP2);
    read(pTuningInfo.CCR.CCR_Y_CPX3);
    read(pTuningInfo.CCR.CCR_Y_CPY0);
    read(pTuningInfo.CCR.CCR_Y_CPY2);
    read(pTuningInfo.CCR.CCR_Y_SP0);
    read(pTuningInfo.CCR.CCR_Y_SP2);
    read(pTuningInfo.CCR.CCR_UV_GAIN_MODE);
    read(pTuningInfo.CCR.CCR_MODE);
    read(pTuningInfo.CCR.CCR_OR_MODE);
    read(pTuningInfo.CCR.CCR_HUE_X1);
    read(pTuningInfo.CCR.CCR_HUE_X2);
    read(pTuningInfo.CCR.CCR_HUE_X3);
    read(pTuningInfo.CCR.CCR_HUE_X4);
    read(pTuningInfo.CCR.CCR_HUE_SP1);
    read(pTuningInfo.CCR.CCR_HUE_SP2);
    read(pTuningInfo.CCR.CCR_HUE_GAIN1);
    read(pTuningInfo.CCR.CCR_HUE_GAIN2);
    SWNR_LOGD("read from file -");
    return MTRUE;
}

MBOOL SwNR::getDebugInfo(IMetadata& halMetadata) const
{
    bool haveExif = false;
    {
        IMetadata::IEntry entry = halMetadata.entryFor(MTK_HAL_REQUEST_REQUIRE_EXIF);
        if (! entry.isEmpty()  && entry.itemAt(0, Type2Type<MUINT8>()) )
                haveExif = true;
    }
    //
    if (haveExif )
    {
        IMetadata::Memory memory_dbgInfo;
        memory_dbgInfo.resize(sizeof(DEBUG_RESERVEA_INFO_T));
        DEBUG_RESERVEA_INFO_T& dbgInfo =
            *reinterpret_cast<DEBUG_RESERVEA_INFO_T*>(memory_dbgInfo.editArray());
        ssize_t idx = 0;
#define addPair(debug_info, index, id, value)           \
        do{                                             \
            debug_info.Tag[index].u4FieldID = (0x01000000 | id); \
            debug_info.Tag[index].u4FieldValue = value; \
            index++;                                    \
        } while(0)
        //
        addPair(dbgInfo, idx, RESERVEA_TAG_VERSION, RESERVEA_DEBUG_TAG_VERSION);
        //

        if (mNRType == DRV_NR_OBJ_SW2 || mNRType == DRV_NR_OBJ_SW2_VPU) // fast swnr
        {
            addPair(dbgInfo , idx , NR_K      , mTuningInfo.NR2.NR_K);
            addPair(dbgInfo , idx , NR_S      , mTuningInfo.NR2.NR_S);
            addPair(dbgInfo , idx , NR_SD     , mTuningInfo.NR2.NR_SD);
            addPair(dbgInfo , idx , NR_BLD_W  , mTuningInfo.NR2.NR_BLD_W);
            addPair(dbgInfo , idx , NR_BLD_TH , mTuningInfo.NR2.NR_BLD_TH);
            addPair(dbgInfo , idx , NR_SMTH   , mTuningInfo.NR2.NR_SMTH);
            addPair(dbgInfo , idx , NR_NTRL_TH_1_Y   , mTuningInfo.NR2.NR_NTRL_TH_1_Y);
            addPair(dbgInfo , idx , NR_NTRL_TH_2_Y   , mTuningInfo.NR2.NR_NTRL_TH_2_Y);
            addPair(dbgInfo , idx , NR_NTRL_TH_1_UV   , mTuningInfo.NR2.NR_NTRL_TH_1_UV);
            addPair(dbgInfo , idx , NR_NTRL_TH_2_UV   , mTuningInfo.NR2.NR_NTRL_TH_2_UV);
        }
        else {
            SWNR_LOGE("not supported type %d", mNRType);
            return MFALSE;
        }

        // HFG
        addPair(dbgInfo , idx , HFG_ENABLE      , mTuningInfo.HFG.HFG_ENABLE);
        addPair(dbgInfo , idx , HFG_GSD         , mTuningInfo.HFG.HFG_GSD);
        addPair(dbgInfo , idx , HFG_SD0         , mTuningInfo.HFG.HFG_SD0);
        addPair(dbgInfo , idx , HFG_SD1         , mTuningInfo.HFG.HFG_SD1);
        addPair(dbgInfo , idx , HFG_SD2         , mTuningInfo.HFG.HFG_SD2);
        addPair(dbgInfo , idx , HFG_TX_S        , mTuningInfo.HFG.HFG_TX_S);
        addPair(dbgInfo , idx , HFG_LCE_LINK_EN , mTuningInfo.HFG.HFG_LCE_LINK_EN);
        addPair(dbgInfo , idx , HFG_LUMA_CPX1   , mTuningInfo.HFG.HFG_LUMA_CPX1);
        addPair(dbgInfo , idx , HFG_LUMA_CPX2   , mTuningInfo.HFG.HFG_LUMA_CPX2);
        addPair(dbgInfo , idx , HFG_LUMA_CPX3   , mTuningInfo.HFG.HFG_LUMA_CPX3);
        addPair(dbgInfo , idx , HFG_LUMA_CPY0   , mTuningInfo.HFG.HFG_LUMA_CPY0);
        addPair(dbgInfo , idx , HFG_LUMA_CPY1   , mTuningInfo.HFG.HFG_LUMA_CPY1);
        addPair(dbgInfo , idx , HFG_LUMA_CPY2   , mTuningInfo.HFG.HFG_LUMA_CPY2);
        addPair(dbgInfo , idx , HFG_LUMA_CPY3   , mTuningInfo.HFG.HFG_LUMA_CPY3);
        addPair(dbgInfo , idx , HFG_LUMA_SP0    , mTuningInfo.HFG.HFG_LUMA_SP0);
        addPair(dbgInfo , idx , HFG_LUMA_SP1    , mTuningInfo.HFG.HFG_LUMA_SP1);
        addPair(dbgInfo , idx , HFG_LUMA_SP2    , mTuningInfo.HFG.HFG_LUMA_SP2);
        addPair(dbgInfo , idx , HFG_LUMA_SP3    , mTuningInfo.HFG.HFG_LUMA_SP3);

        // CCR
        addPair(dbgInfo , idx , CCR_ENABLE       , mTuningInfo.CCR.CCR_ENABLE);
        addPair(dbgInfo , idx , CCR_CEN_U        , mTuningInfo.CCR.CCR_CEN_U);
        addPair(dbgInfo , idx , CCR_CEN_V        , mTuningInfo.CCR.CCR_CEN_V);
        addPair(dbgInfo , idx , CCR_Y_CPX1       , mTuningInfo.CCR.CCR_Y_CPX1);
        addPair(dbgInfo , idx , CCR_Y_CPX2       , mTuningInfo.CCR.CCR_Y_CPX2);
        addPair(dbgInfo , idx , CCR_Y_CPY1       , mTuningInfo.CCR.CCR_Y_CPY1);
        addPair(dbgInfo , idx , CCR_Y_SP1        , mTuningInfo.CCR.CCR_Y_SP1);
        addPair(dbgInfo , idx , CCR_UV_X1        , mTuningInfo.CCR.CCR_UV_X1);
        addPair(dbgInfo , idx , CCR_UV_X2        , mTuningInfo.CCR.CCR_UV_X2);
        addPair(dbgInfo , idx , CCR_UV_X3        , mTuningInfo.CCR.CCR_UV_X3);
        addPair(dbgInfo , idx , CCR_UV_GAIN1     , mTuningInfo.CCR.CCR_UV_GAIN1);
        addPair(dbgInfo , idx , CCR_UV_GAIN2     , mTuningInfo.CCR.CCR_UV_GAIN2);
        addPair(dbgInfo , idx , CCR_UV_GAIN_SP1  , mTuningInfo.CCR.CCR_UV_GAIN_SP1);
        addPair(dbgInfo , idx , CCR_UV_GAIN_SP2  , mTuningInfo.CCR.CCR_UV_GAIN_SP2);
        addPair(dbgInfo , idx , CCR_Y_CPX3       , mTuningInfo.CCR.CCR_Y_CPX3);
        addPair(dbgInfo , idx , CCR_Y_CPY0       , mTuningInfo.CCR.CCR_Y_CPY0);
        addPair(dbgInfo , idx , CCR_Y_CPY2       , mTuningInfo.CCR.CCR_Y_CPY2);
        addPair(dbgInfo , idx , CCR_Y_SP0        , mTuningInfo.CCR.CCR_Y_SP0);
        addPair(dbgInfo , idx , CCR_Y_SP2        , mTuningInfo.CCR.CCR_Y_SP2);
        addPair(dbgInfo , idx , CCR_UV_GAIN_MODE , mTuningInfo.CCR.CCR_UV_GAIN_MODE);
        addPair(dbgInfo , idx , CCR_MODE         , mTuningInfo.CCR.CCR_MODE);
        addPair(dbgInfo , idx , CCR_OR_MODE      , mTuningInfo.CCR.CCR_OR_MODE);
        addPair(dbgInfo , idx , CCR_HUE_X1       , mTuningInfo.CCR.CCR_HUE_X1);
        addPair(dbgInfo , idx , CCR_HUE_X2       , mTuningInfo.CCR.CCR_HUE_X2);
        addPair(dbgInfo , idx , CCR_HUE_X3       , mTuningInfo.CCR.CCR_HUE_X3);
        addPair(dbgInfo , idx , CCR_HUE_X4       , mTuningInfo.CCR.CCR_HUE_X4);
        addPair(dbgInfo , idx , CCR_HUE_SP1      , mTuningInfo.CCR.CCR_HUE_SP1);
        addPair(dbgInfo , idx , CCR_HUE_SP2      , mTuningInfo.CCR.CCR_HUE_SP2);
        addPair(dbgInfo , idx , CCR_HUE_GAIN1    , mTuningInfo.CCR.CCR_HUE_GAIN1);
        addPair(dbgInfo , idx , CCR_HUE_GAIN2    , mTuningInfo.CCR.CCR_HUE_GAIN2);
        // dump debug info to file
        char filename[PROPERTY_VALUE_MAX] = {'\0'};
        if( property_get("vendor.debug.swnr.writefile", filename, "") > 0 )
            dumpDbgToFile(filename, (void*)&dbgInfo);

        // set zeros to remaining
        while( idx < RESERVEA_DEBUG_TAG_SIZE) {
            addPair(dbgInfo , idx , 0, 0);
        }
        //
#undef addPair
        //
        IMetadata exifMeta;
        // query from hal metadata first
        {
            IMetadata::IEntry entry= halMetadata.entryFor(MTK_3A_EXIF_METADATA);
            if (! entry.isEmpty() )
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
        SWNR_LOGD("no need to dump exif");
    }
    //
    return MTRUE;
}


MVOID SwNR::dumpParam(char * const filename)
{
    // write to file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        SWNR_LOGE("fopen fail: %s", filename);
        return;
    }

    fprintf(fp, "SwNR type: %s\n\n", NRType_String(mNRType));

    {
        dumpParams(&mTuningInfo.NR2, fp);
        dumpParams(&mTuningInfo.HFG, fp);
        dumpParams(&mTuningInfo.CCR, fp);
    }

    fclose(fp);
}
MVOID SwNR::enablePerformanceService(
    MINT32& handle_perf_serv_iir,
    MINT32 const perfLevel )
{
    MINT32 policy = 0;
    getPropInt("vendor.debug.swnr.changecpupolicy", &policy, 1);
    sp<IPower> pPowerHal = IPower::getService();
    if(pPowerHal.get() == nullptr) return;
    handle_perf_serv_iir = pPowerHal->scnReg();
    if (handle_perf_serv_iir == -1)
    {
        SWNR_LOGD("[%s] PerfServiceNative_userRegScn fail!\n", LOG_TAG);
    }
    else
    {
        SWNR_LOGD("[%s] PerfServiceNative_userRegScn succeed! handle = %d policy(%d)\n", LOG_TAG, handle_perf_serv_iir, policy);
        if(policy==1)
        {
            pPowerHal-> scnConfig(handle_perf_serv_iir, MtkPowerCmd::CMD_SET_FG_BOOST_VALUE, 100, 0, 0, 0);
            pPowerHal-> scnConfig(handle_perf_serv_iir, MtkPowerCmd::CMD_SET_MTK_PREFER_IDLE, 1, 0, 0, 0);
            pPowerHal-> scnEnable(handle_perf_serv_iir, 0);
        }
        else if(policy==2)
        {
            pPowerHal-> scnConfig(handle_perf_serv_iir, MtkPowerCmd::CMD_SET_CPU_PERF_MODE, 1, 0, 0, 0);
            pPowerHal-> scnConfig(handle_perf_serv_iir, MtkPowerCmd::CMD_SET_SCHED_MODE, 0, 0, 0, 0);
            pPowerHal-> scnEnable(handle_perf_serv_iir, 0);
        }
        else {
            pPowerHal-> scnConfig(handle_perf_serv_iir, MtkPowerCmd::CMD_SET_SCHED_MODE, 0, 0, 0, 0);
            for (int i=0; i<NR_MAX_CLUSTER_NUM; i++)
            {
                pPowerHal-> scnConfig(handle_perf_serv_iir, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, i, NRPerfGearOption[perfLevel][i*2+1], 0, 0);
            }
            pPowerHal-> scnEnable(handle_perf_serv_iir, 100/*timeout value(ms)*/);
        }
    }

}

MVOID SwNR::disablePerformanceService(MINT32 const handle_perf_serv_iir)
{
    sp<IPower> pPowerHal = IPower::getService();
    if(pPowerHal.get() == nullptr) return;
    if (handle_perf_serv_iir != -1) // registered
    {
        pPowerHal-> scnDisable (handle_perf_serv_iir);
        pPowerHal-> scnUnreg (handle_perf_serv_iir);
    }
}

