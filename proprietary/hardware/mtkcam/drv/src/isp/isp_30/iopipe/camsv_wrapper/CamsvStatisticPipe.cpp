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

/*++++++++++++++++from _MyUtils.h++++++++++++++++ */
#ifdef LOG_TAG
    #undef LOG_TAG
#endif
#define LOG_TAG     "CamsvStatisticPipe"

#include <utils/Vector.h>
#include <utils/KeyedVector.h>
using namespace android;

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
using namespace NSCam;

#include <kd_imgsensor_define.h>

/*------------------from _MyUtils.h------------------ */

//#include "VDThread.h"
/*++++++++++++++++from VDThread.h++++++++++++++++ */
#include <utils/Thread.h>
#include <utils/Condition.h>
/*------------------from VDThread.h------------------ */
#include <cutils/properties.h>

#include <imageio/IPipe.h>
#include <imageio/ICamIOPipe.h>
#include <imageio/ispio_pipe_ports.h>
#include <imageio/ispio_pipe_buffer.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>


#include <iopipe/CamIO/NormalPipe_FrmB.h>
#include "iopipe/CamIO/IHalCamIO.h"
#include "CamsvStatisticPipe.h"
#include "iopipe/CamIO/FakeSensor.h"
#include "camsv_buf_mgr.h"
//#include <isp_tuning.h>
//#include "mtkcam/drv/isp_drv.h"

//#include "isp_datatypes.h"
//#include "isp_function.h"
/*TODO:currently sensor provides no data of BAYER type for CAMSV*/
//#define SENSOR_BAYER_TYPE_QUERY_API
#define CAMSV_FORCE_SENSOR_PIXEL_ID


#undef   DBG_LOG_TAG     // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
    #define  DBG_LOG_TAG        ""
#else
    #define  DBG_LOG_TAG        LOG_TAG
#endif

#define THIS_NAME   "CamsvStatisticPipe"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

DECLARE_DBG_LOG_VARIABLE(iocamsvspipe);
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#if 1
#define PIPE_VRB(fmt, arg...)        do { if (iocamsvspipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (iocamsvspipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (iocamsvspipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (iocamsvspipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (iocamsvspipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#else
#define PIPE_VRB(fmt, arg...)        do { { printf(fmt "\n", ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { { printf(fmt "\n", ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { { printf(fmt "\n", ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { { printf(fmt "\n", ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { { printf(fmt "\n", ##arg); } } while(0)
#endif
#define PIPE_AST(cond, fmt, arg...)  do { if (iocamsvspipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


/******************************************************************************
 *  Global Function Declaration.
 ******************************************************************************/
extern "C" INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);


/******************************************************************************
 *
 ******************************************************************************/
#define SPIPE_MEM_NEW(dstPtr,type,size)\
do {\
    SPipeAllocMemSum += size; \
    dstPtr = new type;\
} while(0)

#define SPIPE_MEM_DEL(dstPtr,size)\
do{\
    SPipeFreedMemSum += size; \
    delete dstPtr;\
    dstPtr = NULL;\
}while(0)

#define SPIPE_DUMP_MEM_INFO(string) \
do {\
    PIPE_DBG("%s::SPipeAllocMemSum=0x%x, SPipeFreedMemSum=0x%x", string, SPipeAllocMemSum, SPipeFreedMemSum);\
}while(0)

#define _SPIPE_GET_TGIFMT(sensorIdx,fmt) {\
    switch (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType) {\
        case SENSOR_TYPE_RAW:\
            switch(CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit) {\
                case RAW_SENSOR_8BIT:   fmt = NSCam::eImgFmt_BAYER8; break;\
                case RAW_SENSOR_10BIT:  fmt = NSCam::eImgFmt_BAYER10; break;\
                case RAW_SENSOR_12BIT:  fmt = NSCam::eImgFmt_BAYER12; break;\
                case RAW_SENSOR_14BIT:  fmt = NSCam::eImgFmt_BAYER14; break;\
                default: PIPE_ERR("Err sen raw fmt(%d) err\n", CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit); break;\
            }\
            break;\
        case SENSOR_TYPE_YUV:\
            switch(CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder) {\
                case SENSOR_FORMAT_ORDER_UYVY: fmt = NSCam::eImgFmt_UYVY; break;\
                case SENSOR_FORMAT_ORDER_VYUY: fmt = NSCam::eImgFmt_VYUY; break;\
                case SENSOR_FORMAT_ORDER_YVYU: fmt = NSCam::eImgFmt_YVYU; break;\
                case SENSOR_FORMAT_ORDER_YUYV: fmt = NSCam::eImgFmt_YUY2; break;\
                default:    PIPE_ERR("Err sen yuv fmt err\n"); break;\
            }\
            break;\
        case SENSOR_TYPE_JPEG:\
            fmt = NSCam::eImgFmt_JPEG; break;\
        default:\
            PIPE_ERR("Err sen type(%d) err\n", CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType);\
            break;\
    }\
}

#define _SPIPE_GET_TGI_PIX_ID(sensorIdx,pix_id) do{\
    switch(CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder){\
        case SENSOR_FORMAT_ORDER_RAW_B: pix_id = ERawPxlID_B; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gb: pix_id = ERawPxlID_Gb; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gr: pix_id = ERawPxlID_Gr; break;\
        case SENSOR_FORMAT_ORDER_RAW_R: pix_id = ERawPxlID_R; break;\
        case SENSOR_FORMAT_ORDER_UYVY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_VYUY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YUYV: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YVYU: pix_id = (ERawPxlID)0; break;\
        default:    PIPE_ERR("Error Pix_id: sensorIdx=%d, sensorFormatOrder=%d", sensorIdx, CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder); break;\
    }\
}while(0);

#define _SPIPE_GET_SENSORTYPE(sensorIdx)        (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType)

#define _SPIPE_GET_SENSOR_WIDTH(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureWidth)
#define _SPIPE_GET_SENSOR_HEIGHT(sensorIdx)      (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureHeight)

#define _SPIPE_GET_SENSORCFG_CROP_W(sensorIdx)   (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.w)
#define _SPIPE_GET_SENSORCFG_CROP_H(sensorIdx)   (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.h)
#define _SPIPE_GET_SENSOR_DEV_ID(sensorIdx)      (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDevId)

#define _SPIPE_GET_TG_IDX(sensorIdx)             (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo)
#define _SPIPE_GET_PIX_MODE(sensorIdx)           (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.pixelMode)
#define _SPIPE_GET_HDR_PIX_MODE(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.HDRPixelMode)
#define _SPIPE_GET_PDAF_PIX_MODE(sensorIdx)      (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.PDAFPixelMode)
#define _SPIPE_GET_MVHDR_EMB_PIX_MODE(sensorIdx) (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.PixelMode[EmbInfo])
#define _SPIPE_GET_MVHDR_Y_PIX_MODE(sensorIdx)   (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.PixelMode[YInfo])
#define _SPIPE_GET_MVHDR_AE_PIX_MODE(sensorIdx)  (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.PixelMode[AEInfo])
#define _SPIPE_GET_MVHDR_FLK_PIX_MODE(sensorIdx) (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.PixelMode[FlickerInfo])
#define _SPIPE_GET_HDR_TG_INFO(sensorIdx)        (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.HDRInfo)//TgVR1Info
#define _SPIPE_GET_PDAF_TG_INFO(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.PDAFInfo)//TgVR2Info
#define _SPIPE_GET_MVHDR_EMB_TG_INFO(sensorIdx)  (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.CamInfo[EmbInfo])//TgVR1Info
#define _SPIPE_GET_MVHDR_Y_TG_INFO(sensorIdx)    (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.CamInfo[YInfo])//TgVR2Info
#define _SPIPE_GET_MVHDR_AE_TG_INFO(sensorIdx)   (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.CamInfo[AEInfo])//TgVR3Info
#define _SPIPE_GET_MVHDR_FLK_TG_INFO(sensorIdx)  (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.CamInfo[FlickerInfo])//TgVR4Info
#define _SPIPE_GET_CLK_FREQ(sensorIdx)           (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgCLKInfo)

inline E_CamPixelMode SPIPE_MAP_PXLMODE(MUINT32 sensorIdx, MUINT32 featureIdx)
{
    E_CamPixelMode tgPxlMode = ePixMode_NONE;
    MUINT32 tgPxlModeSeninf = 0;

    switch ((EPIPE_CAMSV_FEATURE_E)featureIdx) {
        case EPIPE_CAMSV_FEATURE_PDAF:
            tgPxlModeSeninf = _SPIPE_GET_PDAF_PIX_MODE(sensorIdx);
            break;
        case EPIPE_CAMSV_FEATURE_MVHDR:
            tgPxlModeSeninf = _SPIPE_GET_HDR_PIX_MODE(sensorIdx);
            break;
        case EPIPE_CAMSV_FEATURE_4CELL_MVHDR_EMB:
            tgPxlModeSeninf = _SPIPE_GET_MVHDR_EMB_PIX_MODE(sensorIdx);
            break;
        case EPIPE_CAMSV_FEATURE_4CELL_MVHDR_Y:
            tgPxlModeSeninf = _SPIPE_GET_MVHDR_Y_PIX_MODE(sensorIdx);
            break;
        case EPIPE_CAMSV_FEATURE_4CELL_MVHDR_AE:
            tgPxlModeSeninf = _SPIPE_GET_MVHDR_AE_PIX_MODE(sensorIdx);
            break;
        case EPIPE_CAMSV_FEATURE_4CELL_MVHDR_FLK:
            tgPxlModeSeninf = _SPIPE_GET_MVHDR_FLK_PIX_MODE(sensorIdx);
            break;
        default:
            PIPE_ERR("Unknown featureIdx during tg pixel mode mapping");
            break;
    }

    switch (tgPxlModeSeninf) {
        case ONE_PIXEL_MODE:
            tgPxlMode = ePixMode_1;
            break;
        case TWO_PIXEL_MODE:
            tgPxlMode = ePixMode_2;
            break;
        case FOUR_PIXEL_MODE:
            tgPxlMode = ePixMode_4;
            break;
        default:
            PIPE_ERR("Unknown tg pixel mode mapping (%d)", (MUINT32)tgPxlModeSeninf);
            break;
    }

    return tgPxlMode;
}

//#define _SPIPE_GET_TG_VC1_INFO(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgVR1Info)
//#define _SPIPE_GET_TG_VC2_INFO(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgVR2Info)
//#define _SPIPE_GET_TG_VC3_INFO(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgVR3Info)
//#define _SPIPE_GET_TG_VC4_INFO(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgVR4Info)
//#define _SPIPE_GET_TG_VC5_INFO(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgVR5Info)
//#define _SPIPE_GET_TG_VC6_INFO(sensorIdx)       (CamsvStatisticPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgVR6Info)

static               MUINT32 SPipeAllocMemSum = 0;
static               MUINT32 SPipeFreedMemSum = 0;

Mutex                CamsvStatisticPipe::SPipeGLock;
Mutex                CamsvStatisticPipe::SPipeDbgLock;
CamsvStatisticPipe*  CamsvStatisticPipe::pCamsvStatisticPipe[EPIPE_Sensor_RSVD][EPIPE_CAMSV_FEATURE_NUM] = \
    {{NULL, NULL}, {NULL, NULL}, {NULL, NULL}};
NSCam::NSIoPipeIsp3::NSCamIOPipe::platSensorsInfo*     CamsvStatisticPipe::pAllSensorInfo = NULL;


/******************************************************************************
 *
 ******************************************************************************/
ICamsvStatisticPipe* ICamsvStatisticPipe::createInstance(MUINT32 SensorIndex, char const* szCallerName, MUINT32 FeatureIdx)
{
    PIPE_INF("+ SenIdx=%d, %s, FeatIdx=%d", SensorIndex, szCallerName, FeatureIdx);

    CamsvStatisticPipe* pPipe = 0;

    if (SensorIndex >= EPIPE_Sensor_RSVD) { /* FIX ME */
        PIPE_ERR("InvalidSensorIdx = %d", SensorIndex);
        return MFALSE;
    }

    CamsvStatisticPipe::SPipeGLock.lock();

    if (CamsvStatisticPipe::pAllSensorInfo == NULL) {
        IHalSensorList *mSList = NULL;
        NSCam::NSIoPipeIsp3::NSCamIOPipe::_sensorInfo_t  *pSInfo = NULL;
        MUINT32 SCnt = 0, i = 0;

        SPIPE_MEM_NEW(CamsvStatisticPipe::pAllSensorInfo, NSCam::NSIoPipeIsp3::NSCamIOPipe::platSensorsInfo, sizeof(NSCam::NSIoPipeIsp3::NSCamIOPipe::platSensorsInfo));
        PIPE_INF("N:%d,%s,pAllSensorInfo=0x%p", SensorIndex, szCallerName,(void*)CamsvStatisticPipe::pAllSensorInfo);

    #if 1
        if (strcmp(szCallerName, "iopipeUseTM") == 0) {
            mSList = FakeSensorList::getTestModel();
        }
        else {
            mSList = IHalSensorList::get();
        }
    #else
        mSList = IHalSensorList::get();
    #endif
        SCnt =  mSList->queryNumberOfSensors();

        CamsvStatisticPipe::pAllSensorInfo->mSList          = mSList;
        CamsvStatisticPipe::pAllSensorInfo->mExistedSCnt    = SCnt;
        CamsvStatisticPipe::pAllSensorInfo->mUserCnt        = 0;
        if ((SCnt > NPIPE_MAX_SENSOR_CNT) || (SCnt == 0)) {
            PIPE_ERR("Not support %d sensors", SCnt);
            SPIPE_MEM_DEL(CamsvStatisticPipe::pAllSensorInfo, sizeof(NSCam::NSIoPipeIsp3::NSCamIOPipe::platSensorsInfo));
            CamsvStatisticPipe::pAllSensorInfo = NULL;
            CamsvStatisticPipe::SPipeGLock.unlock();
            return MFALSE;
        }

        for (i = 0; i < SCnt; i++) {
            pSInfo = &CamsvStatisticPipe::pAllSensorInfo->mSenInfo[i];
            pSInfo->mIdx       = i;
            pSInfo->mTypeforMw = mSList->queryType(i);
            pSInfo->mDevId     = mSList->querySensorDevIdx(i);
            mSList->querySensorStaticInfo(pSInfo->mDevId, &pSInfo->mSInfo);
            PIPE_INF("N:%d,SensorName=%s,Type=%d,DevId=%d", i, mSList->queryDriverName(i), pSInfo->mTypeforMw, pSInfo->mDevId);
        }

        PIPE_INF("Attach seninf dump callback");
    }
    CamsvStatisticPipe::pAllSensorInfo->mUserCnt++;

    pPipe = CamsvStatisticPipe::pCamsvStatisticPipe[SensorIndex][FeatureIdx];
    if (NULL == pPipe) {
        SPIPE_MEM_NEW(CamsvStatisticPipe::pCamsvStatisticPipe[SensorIndex][FeatureIdx], CamsvStatisticPipe(SensorIndex, THIS_NAME, FeatureIdx), sizeof(CamsvStatisticPipe));
        pPipe = CamsvStatisticPipe::pCamsvStatisticPipe[SensorIndex][FeatureIdx];
    }

    pPipe->addUser(szCallerName);

    CamsvStatisticPipe::SPipeGLock.unlock();

    PIPE_DBG("-, pPipe:0x%p, pCamsvStatisticPipe[0][0]:0x%p", pPipe, CamsvStatisticPipe::pCamsvStatisticPipe[0][0]);

    return pPipe;
}


void ICamsvStatisticPipe::destroyInstance(char const* szCallerName)
{
    (void)szCallerName;
    return;
}

MVOID CamsvStatisticPipe::destroyInstance(char const* szCallerName)
{
    MINT32 idx = 0;
    MINT32 featIdx = 0;

    SPipeGLock.lock();

    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++) {
        for (featIdx = 0; featIdx < EPIPE_CAMSV_FEATURE_NUM; featIdx++) {
            if (this == pCamsvStatisticPipe[idx][featIdx]) {
                goto EXIT;
            }
        }
    }

EXIT:
    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("Can't find the entry");
        SPipeGLock.unlock();
        return;
    }

    if (mTotalUserCnt <= 0) {
        PIPE_DBG("No instance now");
        SPipeGLock.unlock();
        return;
    }

    if (!delUser(szCallerName)) {
        PIPE_ERR("No such user");
        SPipeGLock.unlock();
        return;
    }

    if (mTotalUserCnt == 0)  {
        PIPE_INF("Destroy SPipe[%d][%d]=0x%p", idx, featIdx, (void*)pCamsvStatisticPipe[idx][featIdx]);

        if (mpNormalPipe) {
            mpNormalPipe->destroyInstance(THIS_NAME);
            mpNormalPipe = NULL;
        }

        SPIPE_MEM_DEL(pCamsvStatisticPipe[idx][featIdx], sizeof(CamsvStatisticPipe));
    }

    if (CamsvStatisticPipe::pAllSensorInfo) {
       if (CamsvStatisticPipe::pAllSensorInfo->mUserCnt > 0) {
           CamsvStatisticPipe::pAllSensorInfo->mUserCnt--;
       }
       else {
          //return;
       }

       if (CamsvStatisticPipe::pAllSensorInfo->mUserCnt == 0) {
            PIPE_INF("Destroy pAllSensorInfo. idx=%d", idx);
            SPIPE_MEM_DEL(CamsvStatisticPipe::pAllSensorInfo, sizeof(NSCam::NSIoPipeIsp3::NSCamIOPipe::platSensorsInfo));

            PIPE_INF("Detach seninf dump callback");
       }

    }

    SPipeGLock.unlock();

    PIPE_DBG("-");
}


/******************************************************************************
 *
 ******************************************************************************/
CamsvStatisticPipe::CamsvStatisticPipe(MUINT32 SensorIdx, char const* szCallerName, MUINT32 FeatureIdx)
    : mpNormalPipe(NULL)
    , mpHalSensor(NULL)
    , mpName(szCallerName)
    , mBurstQNum(1)
    , mCamsvImgoBufSize(0)
    , mTgInfo(0)
    , mTgInfoCamsv(0)
    , mTotalUserCnt(0)
    , mpSensorIdx(SensorIdx)
    , mFeatureIdx(FeatureIdx)
    , mOpenedPort(_DMAO_NONE)
    , m_FSM(CamsvStatisticPipe::op_unknown)
    , mpIMem(NULL)
    , mpbuf_camsv_mnum(NULL)
    , mpbuf_camsv_imgo(NULL)
    , mpbuf_camsv_imgo_fh(NULL)
    , mp_org_va_imgo_fh(NULL)
    , mp_org_pa_imgo_fh(NULL)
    , mscenarioId(0xdeadbeef)
    , m_subSample(0)
    , ring_cnt(0)
    , camsv_imgo_crop_x(0)
    , camsv_imgo_crop_y(0)
    , camsv_imgo_crop_floatX(0)
    , camsv_imgo_crop_floatY(0)
    , camsv_imgo_crop_w(0)
    , camsv_imgo_crop_h(0)
    , mErrorCode(ECAMSVSPipeErrorCode_UNKNOWN)
{
    DBG_LOG_CONFIG(imageio, iocamsvspipe);

    memset((void*)mUserCnt, 0, sizeof(mUserCnt));
    memset((void*)mUserName, 0, sizeof(mUserName));
    if (NULL == mpNormalPipe) {
        mpNormalPipe = createInstance_INormalPipe(SensorIdx, THIS_NAME);
    }
}

/******************************************************************************
 *
 ******************************************************************************/

MBOOL CamsvStatisticPipe::FSM_CHECK(E_FSM op, const char *callee)
{
    MBOOL ret = MTRUE;

    this->m_FSMLock.lock();

    switch(op){
    case CamsvStatisticPipe::op_unknown:
        if(this->m_FSM != CamsvStatisticPipe::op_uninit)
            ret = MFALSE;
        break;
    case CamsvStatisticPipe::op_init:
        if(this->m_FSM != CamsvStatisticPipe::op_unknown)
            ret = MFALSE;
        break;
    case CamsvStatisticPipe::op_cfg:
        if(this->m_FSM != CamsvStatisticPipe::op_init)
            ret = MFALSE;
        break;
    case CamsvStatisticPipe::op_start:
        switch (this->m_FSM) {
        case CamsvStatisticPipe::op_cfg:
        case CamsvStatisticPipe::op_suspend:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case CamsvStatisticPipe::op_stop:
        switch (this->m_FSM) {
        case CamsvStatisticPipe::op_start:
        case CamsvStatisticPipe::op_suspend:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case CamsvStatisticPipe::op_suspend:
        if (this->m_FSM != CamsvStatisticPipe::op_start) {
            ret = MFALSE;
        }
        break;
    case CamsvStatisticPipe::op_uninit:
        switch(this->m_FSM){
        case CamsvStatisticPipe::op_init:
        case CamsvStatisticPipe::op_cfg:
        case CamsvStatisticPipe::op_stop:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case CamsvStatisticPipe::op_cmd:
        switch(this->m_FSM){
        case CamsvStatisticPipe::op_cfg:
        case CamsvStatisticPipe::op_start:
        case CamsvStatisticPipe::op_stop:
        case CamsvStatisticPipe::op_suspend:
            break;
        default:
            ret= MFALSE;
            break;
        }
        break;
    case CamsvStatisticPipe::op_enque:
        switch(this->m_FSM){
        case CamsvStatisticPipe::op_cfg:
        case CamsvStatisticPipe::op_start:
        case CamsvStatisticPipe::op_suspend://this case is passed due to enque before resume. at deque , alwlasy return MTRUE with bufsize = 0
            break;
        default:
            ret= MFALSE;
            break;
        }
        break;
    case CamsvStatisticPipe::op_deque:
        switch(this->m_FSM){
        case CamsvStatisticPipe::op_cfg://for user to deque after cfg to simplify user's flow control.
        case CamsvStatisticPipe::op_start:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    default:
        ret = MFALSE;
        break;
    }
    if (ret == MFALSE) {
        PIPE_ERR("[%s]op error:cur:0x%x,tar:0x%x(%d)\n", callee, this->m_FSM,op,this->mFeatureIdx);
    }

    this->m_FSMLock.unlock();

    return ret;
}

MBOOL CamsvStatisticPipe::FSM_UPDATE(E_FSM op)
{
    switch(op){
    case CamsvStatisticPipe::op_unknown:
    case CamsvStatisticPipe::op_init:
    case CamsvStatisticPipe::op_cfg:
    case CamsvStatisticPipe::op_start:
    case CamsvStatisticPipe::op_stop:
    case CamsvStatisticPipe::op_suspend:
    case CamsvStatisticPipe::op_uninit:
        break;
    default:
        PIPE_ERR("op update error: cur:0x%x,tar:0x%x\n",this->m_FSM,op);
        return MFALSE;
    }

    this->m_FSMLock.lock();
    this->m_FSM = op;
    this->m_FSMLock.unlock();

    return MTRUE;
}


MBOOL CamsvStatisticPipe::start()
{
    if (this->FSM_CHECK(CamsvStatisticPipe::op_start, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+");

    NS3Av3::ICamSVBufMgr::getInstance().start(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId);

    switch (mTgInfoCamsv) {
        case CAM_SV_1: // 0x10
            if (MFALSE == mpNormalPipe->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageioIsp3::NSIspio::EModule_CAMSV_IMGO, MTRUE, MNULL)) {
                PIPE_ERR("EPIPECmd_SET_MODULE_En fail: EModule_CAMSV_IMGO");
                return MFALSE;
            }
            break;
        case CAM_SV_2: // 0x11
            if (MFALSE == mpNormalPipe->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageioIsp3::NSIspio::EModule_CAMSV2_IMGO, MTRUE, MNULL)) {
                PIPE_ERR("EPIPECmd_SET_MODULE_En fail: EModule_CAMSV2_IMGO");
                return MFALSE;
            }
            break;
    }
    PIPE_INF("-");

    this->FSM_UPDATE(CamsvStatisticPipe::op_start);
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::stop(MBOOL bNonblocking)
{
    MBOOL ret = MTRUE;
    (void)bNonblocking;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_stop, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }
    // TODO: flush waitirq
    PIPE_DBG("+ SenIdx[%d]", mpSensorIdx);

    NS3Av3::ICamSVBufMgr::getInstance().stop(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId);

    switch (mTgInfoCamsv) {
        case CAM_SV_1: // 0x10
            if (MFALSE == mpNormalPipe->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageioIsp3::NSIspio::EModule_CAMSV_IMGO, MFALSE, MNULL)) {
                PIPE_ERR("EPIPECmd_SET_MODULE_En fail: EModule_CAMSV_IMGO");
                return MFALSE;
            }
            break;
        case CAM_SV_2: // 0x11
            if (MFALSE == mpNormalPipe->sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageioIsp3::NSIspio::EModule_CAMSV2_IMGO, MFALSE, MNULL)) {
                PIPE_ERR("EPIPECmd_SET_MODULE_En fail: EModule_CAMSV2_IMGO");
                return MFALSE;
            }
            break;
    }

    PIPE_DBG("-");

    this->FSM_UPDATE(CamsvStatisticPipe::op_stop);

    return ret;
}

MBOOL CamsvStatisticPipe::suspend(void)
{
    MBOOL ret = MTRUE;
    if (this->FSM_CHECK(CamsvStatisticPipe::op_suspend, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }

    if(ret == MTRUE)
        this->FSM_UPDATE(CamsvStatisticPipe::op_suspend);

    return ret;
}

MBOOL CamsvStatisticPipe::resume(void)
{
    MBOOL ret = MTRUE;
    QBufInfo rQBuf;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_start, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }

    //need to enque before resume.
    //this->immediateEnque(rQBuf);

    if(ret == MTRUE)
        this->FSM_UPDATE(CamsvStatisticPipe::op_start);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::init()
{
    MBOOL       ret = MTRUE;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_init, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+.SenIdx=%d", mpSensorIdx);

    NS3Av3::ICamSVBufMgr::getInstance().init(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, mpSensorIdx);

    PIPE_DBG("-");

    this->FSM_UPDATE(CamsvStatisticPipe::op_init);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::uninit()
{
    MBOOL   ret = MTRUE;


    if (this->FSM_CHECK(CamsvStatisticPipe::op_uninit, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+.SenIdx=%d", mpSensorIdx);

    // will also do delete buffer
    NS3Av3::ICamSVBufMgr::getInstance().uninit(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId);

    PIPE_DBG("-");

    this->FSM_UPDATE(CamsvStatisticPipe::op_uninit);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::enque(QBufInfo const& rQBuf)
{
    int ret=MTRUE;
    BufInfo_T rBufInfo;
    (void)rQBuf;

    PIPE_DBG("+");

    if (this->FSM_CHECK(CamsvStatisticPipe::op_enque, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mFeatureIdx == EPIPE_CAMSV_FEATURE_PDAF) {
        if (NS3Av3::ICamSVBufMgr::getInstance().enqueueHwBuf(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, rBufInfo, 1) == MFALSE) {
            PIPE_WRN("PDAF enqueueHwBuf fail.");
        } else {
            /*
            if (NS3Av3::ICamSVBufMgr::getInstance().updateDMABaseAddr(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, 1) == MFALSE) {
                PIPE_WRN("PDAF updateDMABaseAddr fail.");
            }
            */
        }
    } else {
        if (NS3Av3::ICamSVBufMgr::getInstance().enqueueHwBuf(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, rBufInfo, 0) == MFALSE) {
            PIPE_WRN("Other features enqueueHwBuf fail.");
        } else {
            /*
            if (NS3Av3::ICamSVBufMgr::getInstance().updateDMABaseAddr(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, 0) == MFALSE) {
                PIPE_WRN("PDAF updateDMABaseAddr fail.");
            }
            */
        }
    }

    PIPE_DBG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
// TODO: really need this ?
MBOOL CamsvStatisticPipe::immediateEnque(QBufInfo const& rQBuf)
{
    (void)rQBuf;

    QBufInfo _eq_rQBufInfo;
    //enque twice, currently , CAMSV have only 2 ping-pong buffer

    _eq_rQBufInfo.mvOut.resize(1);
    _eq_rQBufInfo.mvOut.at(0).FrameBased.mMagicNum_tuning = 0x12345678; // CAMSV : pass magic number
    if( MFALSE == this->enque(_eq_rQBufInfo)){
        PIPE_ERR("error: imme. enque fail\n");
        return MFALSE;
    }

    _eq_rQBufInfo.mvOut.resize(1);
    _eq_rQBufInfo.mvOut.at(0).FrameBased.mMagicNum_tuning = 0x12345678; // CAMSV : pass magic number
    if( MFALSE == this->enque(_eq_rQBufInfo)){
        PIPE_ERR("error: imme. enque fail\n");
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::deque(PortID port, QBufInfo& rQBuf, ESTT_CACHE_SYNC_POLICY cacheSyncPolicy, MUINT32 u4TimeoutMs)
{
    if (this->FSM_CHECK(CamsvStatisticPipe::op_deque, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    {
        BufInfo_T rBufInfo;
        (void)rQBuf;
        (void)port;
        //(void)cacheSyncPolicy;
        (void)u4TimeoutMs;

        // TODO: implement cachesync

        // PDAF
        if (mFeatureIdx == EPIPE_CAMSV_FEATURE_PDAF) {
            if (NS3Av3::ICamSVBufMgr::getInstance().dequeueHwBuf(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, rBufInfo, 1, 0, cacheSyncPolicy) == MFALSE) {
                PIPE_WRN("PDAF dequeHwBuf fail.");
                return MFALSE;
            }
        } else {
            if (NS3Av3::ICamSVBufMgr::getInstance().dequeueHwBuf(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, rBufInfo, 0, 0, cacheSyncPolicy) == MFALSE) {
                PIPE_WRN("Other features dequeHwBuf fail, mFeatureIdx=%d", mFeatureIdx);
                return MFALSE;
            }
        }

        rQBuf.mvOut.resize(this->m_subSample+1);

        BufInfo* pbuf = &(rQBuf.mvOut.at(0));
        /*
        pbuf->u4BufSize[0]           = (MUINT32)rBufInfo.size;
        pbuf->u4BufVA[0]             = (MUINTPTR)rBufInfo.virtAddr;
        pbuf->u4BufPA[0]             = (MUINTPTR)rBufInfo.phyAddr;
        pbuf->memID[0]             = (MINT32)rBufInfo.memID;
        pbuf->bufSecu[0]             = (MINT32)rBufInfo.bufSecu;
        pbuf->bufCohe[0]             = (MINT32)rBufInfo.bufCohe;
        */

        /* PDOBufMgr usage
        // get the last HW buffer to SW Buffer.
        int index = m_rBufIndex;
        BufInfo rLastBuf = rDQBuf.mvOut.at( size-1 );

        // copy the last HW buffer to SW Buffer.
        StatisticBufInfo rHwBuf;
        rHwBuf.mMagicNumber = rLastBuf.mMetaData.mMagicNum_hal;
        rHwBuf.mSize = rLastBuf.mSize;
        rHwBuf.mVa = rLastBuf.mVa;
        rHwBuf.mPa_offset = rLastBuf.mPa_offset;
        rHwBuf.mPrivateData = rLastBuf.mMetaData.mPrivateData;
        rHwBuf.mPrivateDataSize = rLastBuf.mMetaData.mPrivateDataSize;
        rHwBuf.mStride = rLastBuf.mStride;
        */
        pbuf->mSize = (MUINT32)rBufInfo.size;
        pbuf->mVa = (MUINTPTR)rBufInfo.virtAddr;
        pbuf->mPa = (MUINTPTR)rBufInfo.phyAddr;
        pbuf->mStride = mCamsvImgoBufSize/camsv_imgo_crop_h;

        /* lack info
        rHwBuf.mMagicNumber = rLastBuf.mMetaData.mMagicNum_hal; //useless
        rHwBuf.mPa_offset = rLastBuf.mPa_offset;
        rHwBuf.mPrivateData = rLastBuf.mMetaData.mPrivateData;
        rHwBuf.mPrivateDataSize = rLastBuf.mMetaData.mPrivateDataSize;
        */
    }

    PIPE_DBG("-");

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::addUser(char const* szUserName)
{
    /* search user names to find if exist
     * if not found, check max user num
     *      ok => set idx to the 1st empty loc
     *      ng => return err
     * else set idx to found loc
     * add user cnt @idx
     **/
    MINT32 i = 0, found_idx = -1, len = 0;

    len = strlen(szUserName);
    if ((len == 0) || (len >= IOPIPE_MAX_USER_NAME_LEN)) {
        PIPE_ERR("Ivalide user name length: %d", len);
        return MFALSE;
    }

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            if (strcmp(mUserName[i], szUserName) == 0) {
                found_idx = i;
                break;
            }
        }
    }
    if (found_idx == -1) {
        for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
            if (mUserCnt[i] == 0) {
                break;
            }
        }
        if (i < IOPIPE_MAX_NUM_USERS) {
            found_idx = i;
            strncpy(mUserName[i], szUserName,len);
            mUserName[i][len] = '\0';
        }
        else {
            PIPE_ERR("User count(%d) reaches maximum!", i);
            return MFALSE;
        }
    }

    mUserCnt[found_idx]++;
    mTotalUserCnt++;

    PIPE_INF("%s ++, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::delUser(char const* szUserName)
{
    /* search user names to find if exist
     * if not found, return err
     * else set idx to found loc
     * dec user cnt @idx
     * if user cnt is 0, set name[0]='\0'
     **/
    MINT32 i = 0, found_idx = -1, len = 0;

    len = strlen(szUserName);
    if ((len == 0) || (len >= IOPIPE_MAX_USER_NAME_LEN)) {
        PIPE_ERR("Invalid user name length: %d", len);
        return MFALSE;
    }

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            if (strcmp(mUserName[i], szUserName) == 0) {
                found_idx = i;
            }
        }
    }

    if (found_idx == -1) {
        PIPE_ERR("Invalid user name:%s", szUserName);
        return MFALSE;
    }

    mUserCnt[found_idx]--;
    mTotalUserCnt--;

    if (mUserCnt[found_idx] == 0) {
        mUserName[found_idx][0] = '\0';
    }

    PIPE_INF("%s --, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32 CamsvStatisticPipe::getTotalUserNum(void)
{
    MINT32 i = 0, sum = 0;

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            sum++;
        }
    }
    return sum;
}

static INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    if ( ! pModule )
        PIPE_ERR("INormalPipeModule::get() fail");

    return pModule;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::configPipe(QInitStatParam const& vInPorts, MINT32 burstQnum)
{
    MBOOL       ret = MTRUE, bRaw10 = MFALSE;
    MUINT32     _pIMG_W = 0, bIMG_W = 0, imgH = 0;
    MUINT32     SIdx;
    MUINT32     data1 = 0;
    SENSOR_VC_INFO_STRUCT           VcInfo;
    EImageFormat                    tgi_eImgFmt[4];
    MUINT32 stride,xsize_byte;
    (void)burstQnum;
    (void)vInPorts;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_cfg, __FUNCTION__) == MFALSE) {
        PIPE_ERR("configPipe multiple times, SenIdx=%d", mpSensorIdx);
        return MFALSE;
    }

    PIPE_INF("+.SenIdx=%d, FeatureIdx=%d, burstQnum=%d", mpSensorIdx, mFeatureIdx, burstQnum);

    SIdx = mpSensorIdx;
    m_subSample = burstQnum - 1; // m_subSample: # of subsample SOF/ P1_DON, which is equal to (burstQnum - 1)


#ifdef SENSOR_BAYER_TYPE_QUERY_API
    _SPIPE_GET_TGIFMT(SIdx, tgi_eImgFmt[SIdx]);
#endif

    if (!mpHalSensor) {
        mpHalSensor = pAllSensorInfo->mSList->createSensor(THIS_NAME, 1, &mpSensorIdx);
        if (!mpHalSensor) {
            PIPE_ERR("mpHalSensor Fail");
            mErrorCode = ECAMSVSPipeErrorCode_CREATE_HALSENSOR_FAIL;
            return MFALSE;
        }
    }
    else {
        PIPE_ERR("ERROR: ConfigPipe multiple times...");
        mErrorCode = ECAMSVSPipeErrorCode_MULTILE_CONFIGPIPE;
        return MTRUE;
    }

    if (this->mscenarioId == 0xdeadbeef) {
        PIPE_ERR("invalid Seneario ID(0x%08x)", this->mscenarioId);
        mErrorCode = ECAMSVSPipeErrorCode_INVALID_SCENARIO_ID;
        return MFALSE;
    }

    mpHalSensor->querySensorDynamicInfo(pAllSensorInfo->mSenInfo[SIdx].mDevId, &pAllSensorInfo->mSenInfo[SIdx].mDInfo);

    do {
        Mutex::Autolock lock(mCfgLock);

        data1 = this->mscenarioId;
        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MINTPTR)&data1, 0);


        switch(mFeatureIdx)
        {
            case EPIPE_CAMSV_FEATURE_MVHDR:
                if( _SPIPE_GET_HDR_TG_INFO(mpSensorIdx) == CAM_TG_NONE ) {
                    PIPE_INF("Not support HDR, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mErrorCode = ECAMSVSPipeErrorCode_SENSOR_NOT_SUPPORT_HDR;
                    return MFALSE;
                }
                else {
                    PIPE_INF("HDR, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mTgInfoCamsv = _SPIPE_GET_HDR_TG_INFO(mpSensorIdx);
                    // HDR use VC1
                    bIMG_W = VcInfo.VC1_SIZEH;
                    imgH = VcInfo.VC1_SIZEV;
                    bRaw10 = (VcInfo.VC1_DataType == 0x2b)?MTRUE:MFALSE;
                }
                break;
            case EPIPE_CAMSV_FEATURE_PDAF:
                if( _SPIPE_GET_PDAF_TG_INFO(mpSensorIdx) == CAM_TG_NONE ) {
                    PIPE_INF("Not support PDAF, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mErrorCode = ECAMSVSPipeErrorCode_SENSOR_NOT_SUPPORT_PDAF;
                    return MFALSE;
                }
                else {
                    PIPE_INF("PDAF, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mTgInfoCamsv = _SPIPE_GET_PDAF_TG_INFO(mpSensorIdx);
                    // PDAF use VC2
                    bIMG_W = VcInfo.VC2_SIZEH;
                    imgH = VcInfo.VC2_SIZEV;
                    bRaw10 = (VcInfo.VC2_DataType == 0x2b)?MTRUE:MFALSE;
                }
                break;
            case EPIPE_CAMSV_FEATURE_4CELL_MVHDR_EMB:
                if( _SPIPE_GET_MVHDR_EMB_TG_INFO(mpSensorIdx) == CAM_TG_NONE ) {
                    PIPE_INF("Not support 4CELL_MVHDR_EMB, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mErrorCode = ECAMSVSPipeErrorCode_SENSOR_NOT_SUPPORT_4CELL_MVHDR_EMB;
                    return MFALSE;
                }
                else {
                    PIPE_INF("4CELL_MVHDR_EMB, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mTgInfoCamsv = _SPIPE_GET_MVHDR_EMB_TG_INFO(mpSensorIdx);
                    // 4CELL_MVHDR_HDR use VC1
                    bIMG_W = VcInfo.VC1_SIZEH;
                    imgH = VcInfo.VC1_SIZEV;
                    bRaw10 = (VcInfo.VC1_DataType == 0x2b)?MTRUE:MFALSE;
                }
                break;
            case EPIPE_CAMSV_FEATURE_4CELL_MVHDR_Y:
                if( _SPIPE_GET_MVHDR_Y_TG_INFO(mpSensorIdx) == CAM_TG_NONE ) {
                    PIPE_INF("Not support 4CELL_MVHDR_Y, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mErrorCode = ECAMSVSPipeErrorCode_SENSOR_NOT_SUPPORT_4CELL_MVHDR_Y;
                    return MFALSE;
                }
                else {
                    PIPE_INF("4CELL_MVHDR_Y, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mTgInfoCamsv = _SPIPE_GET_MVHDR_Y_TG_INFO(mpSensorIdx);
                    // 4CELL_MVHDR_Y use VC2
                    bIMG_W = VcInfo.VC2_SIZEH;
                    imgH = VcInfo.VC2_SIZEV;
                    bRaw10 = (VcInfo.VC2_DataType == 0x2b)?MTRUE:MFALSE;
                }
                break;
            case EPIPE_CAMSV_FEATURE_4CELL_MVHDR_AE:
                if( _SPIPE_GET_MVHDR_AE_TG_INFO(mpSensorIdx) == CAM_TG_NONE ) {
                    PIPE_INF("Not support 4CELL_MVHDR_AE, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mErrorCode = ECAMSVSPipeErrorCode_SENSOR_NOT_SUPPORT_4CELL_MVHDR_AE;
                    return MFALSE;
                }
                else {
                    PIPE_INF("4CELL_MVHDR_AE, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mTgInfoCamsv = _SPIPE_GET_MVHDR_AE_TG_INFO(mpSensorIdx);
                    // 4CELL_MVHDR_AE use VC3
                    bIMG_W = VcInfo.VC3_SIZEH;
                    imgH = VcInfo.VC3_SIZEV;
                    bRaw10 = (VcInfo.VC3_DataType == 0x2b)?MTRUE:MFALSE;
                }
                break;
            case EPIPE_CAMSV_FEATURE_4CELL_MVHDR_FLK:
                if( _SPIPE_GET_MVHDR_FLK_TG_INFO(mpSensorIdx) == CAM_TG_NONE ) {
                    PIPE_INF("Not support 4CELL_MVHDR_FLK, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mErrorCode = ECAMSVSPipeErrorCode_SENSOR_NOT_SUPPORT_4CELL_MVHDR_FLK;
                    return MFALSE;
                }
                else {
                    PIPE_INF("4CELL_MVHDR_FLK, sensorid(%d), scenario(%d)", mpSensorIdx, mscenarioId);
                    mTgInfoCamsv = _SPIPE_GET_MVHDR_FLK_TG_INFO(mpSensorIdx);
                    // 4CELL_MVHDR_FLK use VC4
                    bIMG_W = VcInfo.VC4_SIZEH;
                    imgH = VcInfo.VC4_SIZEV;
                    bRaw10 = (VcInfo.VC4_DataType == 0x2b)?MTRUE:MFALSE;
                }
                break;
            default:
                break;
        }
        // datatype 0x2b is RAW10, other is RAW8
        if(bRaw10) {
                tgi_eImgFmt[SIdx] = NSCam::eImgFmt_BAYER10;
                _pIMG_W = bIMG_W * 8 / 10;
        } else {
                tgi_eImgFmt[SIdx] = NSCam::eImgFmt_BAYER8;
                _pIMG_W = bIMG_W;
        }

        //prepare for enque if needed
        camsv_imgo_crop_w = _pIMG_W;
        camsv_imgo_crop_h = imgH;

        //setTGInfo
        switch (mTgInfoCamsv) {
            case CAM_SV_1: // 0x10
                NS3Av3::ICamSVBufMgr::getInstance().setTGInfo(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, CAM_SV_1);
                break;
            case CAM_SV_2: // 0x11
                NS3Av3::ICamSVBufMgr::getInstance().setTGInfo(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, CAM_SV_2);
                break;
            case CAM_SV_3: // 0x12
            case CAM_SV_4: // 0x13
            case CAM_SV_5: // 0x14
            case CAM_SV_6: // 0x15
            default:
                PIPE_ERR("TG_Camsv_%d %s fail", mTgInfoCamsv, "unknown idx");
                mErrorCode = ECAMSVSPipeErrorCode_UNKNOWN_TG_CAMSV_ID;
                return MFALSE;
        }

        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryIn qryInput;
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo queryRst;
        qryInput.width      = bIMG_W;
        qryInput.pixMode = SPIPE_MAP_PXLMODE(mpSensorIdx, mFeatureIdx);

        if (getNormalPipeModule()->query(NSImageio::NSIspio::EPortIndex_CAMSV_IMGO,
                                NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
                                tgi_eImgFmt[mpSensorIdx],
                                qryInput,
                                queryRst) == MFALSE)
        {
            PIPE_ERR("query imgo xsize err\n");
            ret = MFALSE;
            break;
        }
        xsize_byte = queryRst.xsize_byte;
        stride = queryRst.stride_byte;
        PIPE_DBG("xsize_byte(%d_%x),stride(%d_%x)", xsize_byte, xsize_byte, stride, stride);

        mCamsvImgoBufSize = stride*imgH;
    } while(0);

    //AEBufMgr configPipe
    NS3Av3::ICamSVBufMgr::getInstance().configPipe(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId);

    PIPE_INF("-, ret(%d)", ret);

    this->FSM_UPDATE(CamsvStatisticPipe::op_cfg);

    {//need to enque before start.
        //QBufInfo rQBuf;
        //this->immediateEnque(rQBuf);
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;

    //this is a special case
    if(cmd == ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID){
        if (arg1) {
            //set sensor mode
            this->mscenarioId = *(MINT32*)arg1;
            NS3Av3::ICamSVBufMgr::getInstance().setSensorMode(pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId, this->mscenarioId);
            PIPE_INF("ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID, Scenario:%d\n", this->mscenarioId);
            goto _EXIT_SEND_CMD;
        }
        else {
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
    }
    if (this->FSM_CHECK(CamsvStatisticPipe::op_cmd, __FUNCTION__) == MFALSE) {
        ret = MFALSE;
        goto _EXIT_SEND_CMD;
    }

    switch (cmd) {
        case ECAMSVSPipeCmd_GET_TG_INDEX:
            //arg1: [Out] (MUINT32*)  CAM_TG_1/CAM_TG_2/...
            ret = mpNormalPipe->sendCommand(cmd, arg1, arg2, arg3);
            if (MFALSE == ret) {
                goto _EXIT_SEND_CMD;
            }
            break;
        case ECAMSVSPipeCmd_GET_BURSTQNUM:
            if (arg1) {
                *(MINT32*)arg1 = mBurstQNum;
            }
            else {
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }
            break;
        case ECAMSVSPipeCmd_GET_TG_OUT_SIZE:
            //arg1 : sensor-idx
            //arg2 : width
            //arg3 : height
            ret = mpNormalPipe->sendCommand(cmd, arg1, arg2, arg3);
            if (MFALSE == ret) {
                goto _EXIT_SEND_CMD;
            }
            break;
        case ECAMSVSPipeCmd_GET_CAMSV_IMGO_SIZE:
            if (arg1) {
                *(MINT32*)arg1 = mCamsvImgoBufSize;
            }
            else {
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }
            break;
        case ECAMSVSPipeCmd_GET_CAMSV_ENQUE_BUFFER_INFO:
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
            break;
        case ECAMSVSPipeCmd_GET_CAMSV_DEQUE_BUFFER_INFO:
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
            break;
        case ECAMSVSPipeCmd_GET_CAMSV_GET_ERROR_CODE:
            if (arg1) {
                *(MINT32*)arg1 = mErrorCode;
            }
            else {
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }
            break;
        default:
            ret = MFALSE;
    }

_EXIT_SEND_CMD:
    if (ret != MTRUE) {
        PIPE_ERR("error: sendCommand fail: (cmd,arg1,arg2,arg3)=(0x%08x,0x%p,0x%p,0x%p)", cmd, (void*)arg1,(void*)arg2,(void*)arg3);
    }

    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
//arg1 : only valid in cmd:_GetPort_Index
MUINT32 CamsvStatisticPipe::GetOpenedPortInfo(CamsvStatisticPipe::ePortInfoCmd cmd, MUINT32 arg1)
{
    MUINT32 _num = 0;
    (void) arg1;
    switch (cmd) {
        case CamsvStatisticPipe::_GetPort_Opened:
            return (MUINT32)mOpenedPort;
        case CamsvStatisticPipe::_GetPort_OpendNum:
            if (mOpenedPort& CamsvStatisticPipe::_CAMSV_IMGO_ENABLE)
                _num++;
            return _num;
        case CamsvStatisticPipe::_GetPort_Index:
        {
            PIPE_ERR("un-supported cmd:0x%x\n",cmd);
            return -1;
        }
        default:
            PIPE_ERR("un-supported cmd:0x%x\n",cmd);
            return 0;
    }
}

MINT32  CamsvStatisticPipe::attach(const char* UserName)
{
    (void)UserName;
    return -1;
}

MBOOL   CamsvStatisticPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    (void)eSignal;
    (void)eClear;
    (void)mUserKey;
    (void)TimeoutMs;
    (void)pTime;
    return MFALSE;
}

MBOOL   CamsvStatisticPipe::signal(EPipeSignal eSignal, const MINT32 mUserKey)
{
    (void)eSignal;
    (void)mUserKey;
    return MFALSE;
}

MBOOL   CamsvStatisticPipe::abortDma(PortID port, char const* szCallerName)
{
    (void)port;
    (void)szCallerName;
    return MFALSE;
}

MUINT32 CamsvStatisticPipe::getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical)
{
    (void)RegAddr;
    (void)RegCount;
    (void)RegData;
    (void)bPhysical;
    return MFALSE;
}

MUINT32 CamsvStatisticPipe::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    (void)pRegs;
    (void)RegCount;
    (void)bPhysical;
    return MFALSE;
}

MUINT32 CamsvStatisticPipe::getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    (void)pRegs;
    (void)RegCount;
    (void)bPhysical;
    return MFALSE;
}


