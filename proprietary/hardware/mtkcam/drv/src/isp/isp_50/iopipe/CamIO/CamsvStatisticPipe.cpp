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

#include <IPipe.h>
#include <ICamIOPipe.h>
#include <ispio_pipe_ports.h>
#include <Cam_Notify_datatype.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>


#include "NormalPipe.h"
#include "CamsvStatisticPipe.h"
#include "camio_log.h"
#include "FakeSensor.h"

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
platSensorsInfo*     CamsvStatisticPipe::pAllSensorInfo = NULL;

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
        _sensorInfo_t  *pSInfo = NULL;
        MUINT32 SCnt = 0, i = 0;

        SPIPE_MEM_NEW(CamsvStatisticPipe::pAllSensorInfo, platSensorsInfo, sizeof(platSensorsInfo));
        PIPE_INF("N:%d,%s,pAllSensorInfo=0x%p", SensorIndex, szCallerName,(void*)CamsvStatisticPipe::pAllSensorInfo);

        #if 1
        if (strcmp(szCallerName, "iopipeUseTM") == 0) {
            mSList = TS_FakeSensorList::getTestModel();
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
        if ((SCnt > IOPIPE_MAX_SENSOR_CNT) || (SCnt == 0)) {
            PIPE_ERR("Not support %d sensors", SCnt);
            SPIPE_MEM_DEL(CamsvStatisticPipe::pAllSensorInfo, sizeof(platSensorsInfo));
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
        SENINF_DBG::m_fp_Sen_Camsv= CamsvStatisticPipe::dumpSeninfDebugCB;
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


/******************************************************************************
 *
 ******************************************************************************/

MVOID CamsvStatisticPipe::dumpSeninfDebugCB(MUINT32 ispModule, MUINT32 tgStat)
{
    MUINT32 idx = 0;
    MUINT32 featureIdx = 0;
    MUINT32 camsvTg = CAM_TG_NONE;

    CamsvStatisticPipe::SPipeDbgLock.lock();

    switch (ispModule) {
    case CAMSV_0:
        camsvTg = CAM_SV_1;
        break;
    case CAMSV_1:
        camsvTg = CAM_SV_2;
        break;
    case CAMSV_2:
        camsvTg = CAM_SV_3;
        break;
    case CAMSV_3:
        camsvTg = CAM_SV_4;
        break;
    case CAMSV_4:
        camsvTg = CAM_SV_5;
        break;
    case CAMSV_5:
        camsvTg = CAM_SV_6;
        break;
    default:
        PIPE_ERR("ERROR: unknow isp hw module : %d, dump all", ispModule);
        CamsvStatisticPipe::SPipeDbgLock.unlock();

        dumpSeninfDebugCB(CAMSV_0, tgStat);
        dumpSeninfDebugCB(CAMSV_1, tgStat);
        return;
    }

    // get sensor index via comparing camsv tg
    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++) {
        if (_SPIPE_GET_PDAF_TG_INFO(idx) == camsvTg) {
            featureIdx = EPIPE_CAMSV_FEATURE_PDAF;
            goto DUMP_INFO;
        }
        if (_SPIPE_GET_HDR_TG_INFO(idx) == camsvTg) {
            featureIdx = EPIPE_CAMSV_FEATURE_MVHDR;
            goto DUMP_INFO;
        }
        if (_SPIPE_GET_MVHDR_EMB_TG_INFO(idx) == camsvTg) {
            featureIdx = EPIPE_CAMSV_FEATURE_4CELL_MVHDR_EMB;
            goto DUMP_INFO;
        }
        if (_SPIPE_GET_MVHDR_Y_TG_INFO(idx) == camsvTg) {
            featureIdx = EPIPE_CAMSV_FEATURE_4CELL_MVHDR_Y;
            goto DUMP_INFO;
        }
        if (_SPIPE_GET_MVHDR_AE_TG_INFO(idx) == camsvTg) {
            featureIdx = EPIPE_CAMSV_FEATURE_4CELL_MVHDR_AE;
            goto DUMP_INFO;
        }
        if (_SPIPE_GET_MVHDR_FLK_TG_INFO(idx) == camsvTg) {
            featureIdx = EPIPE_CAMSV_FEATURE_4CELL_MVHDR_FLK;
            goto DUMP_INFO;
        }
    }

DUMP_INFO:
    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("ERROR: unknow tg index : %d", camsvTg);
        goto _EXIT_DUMP;
    }

    if (CamsvStatisticPipe::pCamsvStatisticPipe[idx][featureIdx]) {
        MINT32 seninfStatus = 0;

        PIPE_INF("Start dump seninf debug info: sensor_idx:%d sensor_devid:x%x",
                    idx, CamsvStatisticPipe::pAllSensorInfo->mSenInfo[idx].mDevId);
        seninfStatus = CamsvStatisticPipe::pCamsvStatisticPipe[idx][featureIdx]->mpHalSensor->sendCommand(CamsvStatisticPipe::pAllSensorInfo->mSenInfo[idx].mDevId,
                                                                SENSOR_CMD_DEBUG_P1_DQ_SENINF_STATUS, 1, 0, 0);
    }
    else {
        PIPE_ERR("ERROR: NormalPipe[%d] object not create yet", idx);
        goto _EXIT_DUMP;
    }

_EXIT_DUMP:
    CamsvStatisticPipe::SPipeDbgLock.unlock();
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

        if (mpNormalPipe) {
            mpNormalPipe->destroyInstance(THIS_NAME);
            mpNormalPipe = NULL;
        }

        if (mpCamsvIOPipe) {
            mpCamsvIOPipe->destroyInstance();
            mpCamsvIOPipe = NULL;
        }

        for(MUINT32 i=0;i<(CAMSV_BUF_DEPTH * (m_subSample + 1));i++) {
            if( NULL != mpbuf_camsv_imgo ) {
                if(mpIMem->freeVirtBuf(&mpbuf_camsv_imgo[i])){
                    PIPE_ERR("IMem freeVirtBuf Failed");
                    SPipeGLock.unlock();
                    return;
                }
            }

            if( NULL != mpbuf_camsv_imgo_fh ) {
                mpbuf_camsv_imgo_fh[i].virtAddr = mp_org_va_imgo_fh[i];
                mpbuf_camsv_imgo_fh[i].phyAddr = mp_org_pa_imgo_fh[i];
                if(mpIMem->freeVirtBuf(&mpbuf_camsv_imgo_fh[i])){
                    PIPE_ERR("IMem freeVirtBuf Failed");
                    SPipeGLock.unlock();
                    return;
                }
            }
        }

        if( mpbuf_camsv_mnum ) {
            free(mpbuf_camsv_mnum);
            mpbuf_camsv_mnum = NULL;
        }

        if( mpbuf_camsv_imgo ) {
            free(mpbuf_camsv_imgo);
            mpbuf_camsv_imgo = NULL;
        }
        if( mpbuf_camsv_imgo_fh ) {
            free(mpbuf_camsv_imgo_fh);
            mpbuf_camsv_imgo_fh = NULL;
        }
        if( mp_org_va_imgo_fh ) {
            free(mp_org_va_imgo_fh);
            mp_org_va_imgo_fh = NULL;
        }
        if( mp_org_pa_imgo_fh ) {
            free(mp_org_pa_imgo_fh);
            mp_org_pa_imgo_fh = NULL;
        }

        if (mpIMem) {
            mpIMem->uninit();
            mpIMem->destroyInstance();
            mpIMem = NULL;
        }

        PIPE_INF("Destroy SPipe[%d][%d]=0x%p", idx, featIdx, (void*)pCamsvStatisticPipe[idx][featIdx]);
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
            SPIPE_MEM_DEL(CamsvStatisticPipe::pAllSensorInfo, sizeof(platSensorsInfo));

            PIPE_INF("Detach seninf dump callback");
            SENINF_DBG::m_fp_Sen_Camsv = NULL;
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
    , mpCamsvIOPipe(NULL)
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
    MBOOL   ret = MTRUE;
    MUINT32 _size = 0, _tmp;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_start, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+");

    if (mpCamsvIOPipe) {
        ret = mpCamsvIOPipe->start();
        if (MFALSE == ret) {
            PIPE_ERR("TG_Camsv_%d start fail", mTgInfoCamsv);
            return MFALSE;
        }
    }

    PIPE_INF("-");

    this->FSM_UPDATE(CamsvStatisticPipe::op_start);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::stop(MBOOL bNonblocking)
{
    MBOOL ret = MTRUE;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_stop, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_DBG("+ SenIdx[%d]", mpSensorIdx);

    if (mpCamsvIOPipe) {
        ret = mpCamsvIOPipe->stop(bNonblocking);
        if (MFALSE == ret) {
            PIPE_ERR("TG_Camsv_%d stop fail", mTgInfoCamsv);
            return MFALSE;
        }
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

    this->ring_cnt = 0;
    //also clear all buf record
    ret = mpCamsvIOPipe->suspend(NSImageio::NSIspio::ICamIOPipe::SW_SUSPEND);

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
    this->immediateEnque(rQBuf);

    ret = mpCamsvIOPipe->resume(NSImageio::NSIspio::ICamIOPipe::SW_SUSPEND);

    if(ret == MTRUE)
        this->FSM_UPDATE(CamsvStatisticPipe::op_start);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::init()
{
    MBOOL       ret = MTRUE, cmdSent = 0;
    MUINT32     i = 0;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_init, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+.SenIdx=%d", mpSensorIdx);

    do {
        Mutex::Autolock lock(mCfgLock);

        if (mpIMem) {
            PIPE_ERR("re-init");
            return MFALSE;
        }
        mpIMem = IMemDrv::createInstance();
        if (mpIMem) {
            ret = mpIMem->init();
        }
        if ((NULL == mpIMem) || (MFALSE == ret)) {
            PIPE_ERR("imem create/init fail %p, %d", mpIMem, ret);
        }
    } while(0);

    PIPE_DBG("-");

    this->FSM_UPDATE(CamsvStatisticPipe::op_init);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::uninit()
{
    MUINT32 i = 0;
    MBOOL   ret = MTRUE;


    if (this->FSM_CHECK(CamsvStatisticPipe::op_uninit, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+.SenIdx=%d", mpSensorIdx);

    if (mpCamsvIOPipe)  {
        ret = mpCamsvIOPipe->uninit();
    }

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
    NSImageio::NSIspio::PortID      portID;
    NSImageio::NSIspio::QBufInfo    rQBufInfo;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_enque, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    //Mutex::Autolock lock(mEnQLock); /* FIX ME */

    //save magic number
    if( rQBuf.mvOut.size() ) {
        mpbuf_camsv_mnum[this->ring_cnt] = rQBuf.mvOut.at(0).FrameBased.mMagicNum_tuning;
    }
    else {
        mpbuf_camsv_mnum[this->ring_cnt] = 0xbeefdead;
    }

    for(MUINT32 i=0;i<(this->m_subSample+1);i++){
        //enque
        portID.index = EPortIndex_IMGO;

        rQBufInfo.vBufInfo.resize((this->m_subSample+1));

        rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st]           = (MUINT32)this->mpbuf_camsv_imgo[this->ring_cnt*(this->m_subSample+1) + i].size;
        rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st]             = (MUINTPTR)this->mpbuf_camsv_imgo[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
        rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st]             = (MUINTPTR)this->mpbuf_camsv_imgo[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st] = (MUINT32)this->mpbuf_camsv_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].size;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st]   = (MUINTPTR)this->mpbuf_camsv_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].virtAddr;
        rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st]   = (MUINTPTR)this->mpbuf_camsv_imgo_fh[this->ring_cnt*(this->m_subSample+1) + i].phyAddr;
        rQBufInfo.vBufInfo[i].img_w = camsv_imgo_crop_w;
        rQBufInfo.vBufInfo[i].img_h = camsv_imgo_crop_h;
        rQBufInfo.vBufInfo[i].crop_win.p.x = camsv_imgo_crop_x;
        rQBufInfo.vBufInfo[i].crop_win.p.y = camsv_imgo_crop_y;
        rQBufInfo.vBufInfo[i].crop_win.s.w = camsv_imgo_crop_w;
        rQBufInfo.vBufInfo[i].crop_win.s.h = camsv_imgo_crop_h;
        PIPE_DBG("enque_imgo_%d: size:0x%08x_0x%08x, pa:0x%p_0x%p, magic:0x%x, crop:%d_%d_%d_%d, ring_cnt:%d",\
            i,\
            rQBufInfo.vBufInfo.at(i).u4BufSize[ePlane_1st],\
            rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize[ePlane_1st],\
            (void*)rQBufInfo.vBufInfo.at(i).u4BufPA[ePlane_1st],\
            (void*)rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA[ePlane_1st],\
            rQBufInfo.vBufInfo.at(i).m_num,\
            camsv_imgo_crop_x, camsv_imgo_crop_y,\
            camsv_imgo_crop_w, camsv_imgo_crop_h,\
            this->ring_cnt);
    }
    //pCamsvIo->sendCommand(EPIPECmd_SET_IMGO, (MINTPTR)&Input_L, (MINTPTR)&Input2_L,0);

    if (rQBuf.mShutterTimeNs) {
        mpCamsvIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(rQBuf.mShutterTimeNs/1000000), 1, 0);
    }
    else{
        //TO_DO: check camsv mShutterTimeNs if needed.
        PIPE_DBG("Warning:TG_Camsv_%d Shutter Time is 0!", mTgInfoCamsv);
    }
    if (mpCamsvIOPipe->enqueOutBuf(portID, rQBufInfo) == MFALSE) {
        ret =MFALSE;
        PIPE_ERR("error:TG_Camsv_%d port_%d enque fail", mTgInfoCamsv, portID.index);
    }

    this->ring_cnt = (this->ring_cnt + 1)%CAMSV_BUF_DEPTH;

    PIPE_DBG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
void CamsvStatisticPipe::enqueRequestCB(void *para, void *cookie)
{
    (void)para;
    (void)cookie;
    PIPE_ERR("Unsupported");
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::immediateEnque(QBufInfo const& rQBuf)
{
    rQBuf;

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
    NSImageio::NSIspio::QTimeStampBufInfo   rQTBufInfo;
    NSImageio::NSIspio::PortID              portID;
    NSImageio::NSIspio::E_BUF_STATUS        state;
    (void) u4TimeoutMs;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_deque, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    {

        portID.index = port.index;

        state = mpCamsvIOPipe->dequeOutBuf( portID, rQTBufInfo);
        switch(state){
            case NSImageio::NSIspio::eBuf_Fail:
                PIPE_ERR("error:TG_Camsv_%d port_%d deque fail", mTgInfoCamsv, portID.index);
                return MFALSE;
                break;
            case NSImageio::NSIspio::eBuf_suspending:
                //when user find return MTRUE with vector size= 0 , means hw is under suspending!
                rQBuf.mvOut.clear();
                return MTRUE;
                break;
            case NSImageio::NSIspio::eBuf_Pass:
                break;
            default:
                PIPE_ERR("unsupported state:%d\n",state);
                return MFALSE;
                break;
        }

        if (rQTBufInfo.vBufInfo.size() != mBurstQNum) {
            PIPE_INF("WARNNING: TG_Camsv_%d port_%d deque buf num unsync, %zu/%d", mTgInfoCamsv,\
                      portID.index, rQTBufInfo.vBufInfo.size(), mBurstQNum);
        }


        rQBuf.mvOut.resize(rQTBufInfo.vBufInfo.size());

        for(MUINT32 idx=0; idx<rQTBufInfo.vBufInfo.size(); idx++){

            BufInfo* pbuf = &(rQBuf.mvOut.at(idx));
            ResultMetadata result;

            result.mDstSize         = rQTBufInfo.vBufInfo.at(idx).DstSize;
            result.mMagicNum_hal    = rQTBufInfo.vBufInfo.at(idx).m_num;
            result.mMagicNum_tuning = rQTBufInfo.vBufInfo.at(idx).m_num;
            result.mTimeStamp       = rQTBufInfo.vBufInfo.at(idx).getTimeStamp_ns();
#ifdef TS_PERF
            result.mTimeStamp_B     = rQTBufInfo.vBufInfo.at(idx).getTimeStamp_B_ns();
            PIPE_DBG("deque_imgo_%d: size:0x%08x_0x%08x, pa:0x%x_0x%x, magic:0x%x, time:%" PRIu64 ", Tg_Camsv size:%dx%d,crop:%d_%d_%d_%d",\
                idx,\
                rQTBufInfo.vBufInfo.at(idx).u4BufSize[ePlane_1st],\
                rQTBufInfo.vBufInfo.at(idx).Frame_Header.u4BufSize,\
                rQTBufInfo.vBufInfo.at(idx).u4BufPA[ePlane_1st],\
                rQTBufInfo.vBufInfo.at(idx).Frame_Header.u4BufPA,\
                rQTBufInfo.vBufInfo.at(idx).m_num,\
                rQTBufInfo.vBufInfo.at(idx).i4TimeStamp_us,\
                rQTBufInfo.vBufInfo.at(idx).img_w, rQTBufInfo.vBufInfo.at(idx).img_h,\
                rQTBufInfo.vBufInfo.at(idx).crop_win.p.x, rQTBufInfo.vBufInfo.at(idx).crop_win.p.y,\
                rQTBufInfo.vBufInfo.at(idx).crop_win.s.w, rQTBufInfo.vBufInfo.at(idx).crop_win.s.h);

#else
            PIPE_DBG("deque_imgo_%d: size:0x%08x_0x%08x, pa:0x%x_0x%x, magic:0x%x, time:%" PRIu64 "_%d, Tg_Camsv size:%dx%d,crop:%d_%d_%d_%d",\
                idx,\
                rQTBufInfo.vBufInfo.at(idx).u4BufSize[ePlane_1st],\
                rQTBufInfo.vBufInfo.at(idx).Frame_Header.u4BufSize,\
                rQTBufInfo.vBufInfo.at(idx).u4BufPA[ePlane_1st],\
                rQTBufInfo.vBufInfo.at(idx).Frame_Header.u4BufPA,\
                rQTBufInfo.vBufInfo.at(idx).m_num,\
                rQTBufInfo.vBufInfo.at(idx).i4TimeStamp_sec,\
                rQTBufInfo.vBufInfo.at(idx).i4TimeStamp_us,\
                rQTBufInfo.vBufInfo.at(idx).img_w, rQTBufInfo.vBufInfo.at(idx).img_h,\
                rQTBufInfo.vBufInfo.at(idx).crop_win.p.x, rQTBufInfo.vBufInfo.at(idx).crop_win.p.y,\
                rQTBufInfo.vBufInfo.at(idx).crop_win.s.w, rQTBufInfo.vBufInfo.at(idx).crop_win.s.h);
#endif

            pbuf->mPortID   = portID.index;
            pbuf->mBuffer   = NULL;
            pbuf->mMetaData = result;
            pbuf->mSize     = rQTBufInfo.vBufInfo.at(idx).u4BufSize[ePlane_1st];
            pbuf->mVa       = rQTBufInfo.vBufInfo.at(idx).u4BufVA[ePlane_1st];
            pbuf->mPa       = rQTBufInfo.vBufInfo.at(idx).u4BufPA[ePlane_1st];
            pbuf->mStride   = mCamsvImgoBufSize/camsv_imgo_crop_h;
            //get magic number which saved in previous enque
            pbuf->FrameBased.mMagicNum_tuning = mpbuf_camsv_mnum[ (this->ring_cnt==0) ? (CAMSV_BUF_DEPTH-1) : (this->ring_cnt-1)];

            /* FIX ME : cache invalidate */
            MUINT32 b;
            for(b = 0; b < CAMSV_BUF_DEPTH*(this->m_subSample+1); b++) {
                if(pbuf->mPa == mpbuf_camsv_imgo[b].phyAddr) {
                    PIPE_DBG("cache sync PA(x%p) policy(%d)", (void*)pbuf->mPa, cacheSyncPolicy);
                    switch(cacheSyncPolicy){
                        case ESTT_CacheBypass:
                            PIPE_WRN("Camsv:%d: DMA(x%x) PA:0x%p bypass cache sync.", mpSensorIdx , portID.index, (void*)pbuf->mPa);
                            break;
                        case ESTT_CacheInvalidAll:
                            mpIMem->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID_ALL, &(mpbuf_camsv_imgo[b]));
                            break;
                        case ESTT_CacheInvalidByRange:
                        default:
                            mpIMem->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID    , &(mpbuf_camsv_imgo[b]));
                            break;
                    }
                    break;
                }
            }
            if (b == CAMSV_BUF_DEPTH*(this->m_subSample+1) )
                PIPE_ERR("PA(0x%p) not found to sync cache", (void*)pbuf);

        }
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
    MBOOL       ret = MTRUE, cmdSent = 0, bRaw10 = MFALSE;
    MUINT32     MainImgBQNum = 0, _portNum = 0;
    MUINT32     i = 0, j = 0, _portIdx = 0, _pIMG_W = 0, imgW = 0, imgH = 0, dma_size = 0;
    MUINT32     SIdx, sensorDevIdx;
    MUINT32     data1 = 0;
    MUINTPTR    imgHdrAllocOfst = 0;
    E_INPUT eTgInput;
    E_INPUT eTgInputCamsv;
    vector<PortInfo const*>         vCamsvIOOutPorts, vCamsvIOInPorts;
    NSImageio::NSIspio::PortInfo    *_in_port, *_out_port;
    NSImageio::NSIspio::PortInfo    camsv_tgi, camsv_imgo;
    SENSOR_VC_INFO_STRUCT           VcInfo;
    EImageFormat                    tgi_eImgFmt[4];
    CAMIO_Func func;
    func.Raw = 0;
    MUINT32 stride,xsize_byte;
    MUINT32 pad = 0;
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
                    imgW = VcInfo.VC1_SIZEH;
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
                    imgW = VcInfo.VC2_SIZEH;
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
                    imgW = VcInfo.VC1_SIZEH;
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
                    imgW = VcInfo.VC2_SIZEH;
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
                    imgW = VcInfo.VC3_SIZEH;
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
                    imgW = VcInfo.VC4_SIZEH;
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
                _pIMG_W = imgW * 8 / 10;
        } else {
                tgi_eImgFmt[SIdx] = NSCam::eImgFmt_BAYER8;
                _pIMG_W = imgW;
        }

        switch (mTgInfoCamsv) {
            case CAM_SV_1: // 0x10
                eTgInputCamsv = TG_CAMSV_0; // 10
                break;
            case CAM_SV_2: // 0x11
                eTgInputCamsv = TG_CAMSV_1; // 11
                break;
            case CAM_SV_3: // 0x12
                eTgInputCamsv = TG_CAMSV_2; // 12
                break;
            case CAM_SV_4: // 0x13
                eTgInputCamsv = TG_CAMSV_3; // 13
                break;
            case CAM_SV_5: // 0x14
                eTgInputCamsv = TG_CAMSV_4; // 14
                break;
            case CAM_SV_6: // 0x15
                eTgInputCamsv = TG_CAMSV_5; // 15
                break;
            default:
                PIPE_ERR("TG_Camsv_%d %s fail", mTgInfoCamsv, "unknown idx");
                mErrorCode = ECAMSVSPipeErrorCode_UNKNOWN_TG_CAMSV_ID;
                return MFALSE;
        }

        if( !mpCamsvIOPipe ) {
            mpCamsvIOPipe = ICamIOPipe::createInstance((MINT8 const*)mpName, eTgInputCamsv, NSImageio::NSIspio::ICamIOPipe::CAMSVIO);
            if (NULL == mpCamsvIOPipe) {
                PIPE_ERR("TG_Camsv_%d %s fail", eTgInputCamsv, "create");
                mErrorCode = ECAMSVSPipeErrorCode_CREATE_CAMSAV_IOPIPE_FAILE;
                return MFALSE;
            }
        }
        if (MFALSE == mpCamsvIOPipe->init()) {
            PIPE_ERR("TG_Camsv_%d %s fail", eTgInputCamsv, "init");
            mErrorCode = ECAMSVSPipeErrorCode_INIT_CAMSAV_IOPIPE_FAILE;
            return MFALSE;
        }

        // alocate memory
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryIn qryInput;
        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo queryRst;
        qryInput.width      = _pIMG_W;
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

        for(MUINT32 i=0;i<(CAMSV_BUF_DEPTH*(m_subSample+1));i++) {
            if(mpbuf_camsv_imgo == NULL){
                MUINT32 buf_imgo_size = sizeof(IMEM_BUF_INFO)*(CAMSV_BUF_DEPTH* (this->m_subSample+1));
                MUINT32 buf_imgo_fh_size = sizeof(IMEM_BUF_INFO)*(CAMSV_BUF_DEPTH* (this->m_subSample+1));
                MUINT32 orig_va_buf_imgo_fh_size = sizeof(MUINTPTR)*(CAMSV_BUF_DEPTH* (this->m_subSample+1));
                MUINT32 orig_pa_buf_imgo_fh_size = sizeof(MUINTPTR)*(CAMSV_BUF_DEPTH* (this->m_subSample+1));
                mpbuf_camsv_mnum = (MUINT32*)malloc(sizeof(MUINT32)*(CAMSV_BUF_DEPTH));
                mpbuf_camsv_imgo = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(CAMSV_BUF_DEPTH*(m_subSample+1)));
                mpbuf_camsv_imgo_fh = (IMEM_BUF_INFO*)malloc(sizeof(IMEM_BUF_INFO)*(CAMSV_BUF_DEPTH*(m_subSample+1)));
                mp_org_va_imgo_fh = (MUINTPTR*)malloc(sizeof(MUINTPTR)*(CAMSV_BUF_DEPTH*(m_subSample+1)));
                mp_org_pa_imgo_fh = (MUINTPTR*)malloc(sizeof(MUINTPTR)*(CAMSV_BUF_DEPTH*(m_subSample+1)));

                if(mpbuf_camsv_mnum == NULL) {
                   PIPE_ERR("buf malloc err\n");
                   return MFALSE;
                }

                for(MUINT32 j=0;j<CAMSV_BUF_DEPTH;j++) {
                    mpbuf_camsv_mnum[j] = 0xdeadbeef;
                }
            }

            mpbuf_camsv_imgo[i].useNoncache = MFALSE;
            mpbuf_camsv_imgo_fh[i].useNoncache = MFALSE;

            mpbuf_camsv_imgo[i].size = ((mCamsvImgoBufSize + 15) >> 4) << 4; // for 16 bytes align

            if(mpIMem->allocVirtBuf(&mpbuf_camsv_imgo[i]) < 0){
                PIPE_ERR("allocVirtBuf() error imgo(%d)", i);
                return MFALSE;
            }
            if(mpIMem->mapPhyAddr(&mpbuf_camsv_imgo[i]) < 0 ){
                PIPE_ERR("mapPhyAddr() error imgo(%d)", i);
                return MFALSE;
            }

            mpbuf_camsv_imgo_fh[i].size = sizeof(MUINT32)*(16 + 2);
            mpbuf_camsv_imgo_fh[i].size = ((mpbuf_camsv_imgo_fh[i].size + 15) >> 4) << 4; // for 16 bytes align

            if(mpIMem->allocVirtBuf(&mpbuf_camsv_imgo_fh[i]) < 0){
                PIPE_ERR("allocVirtBuf() error fh(%d)", i);
                return MFALSE;
            }

            if(mpIMem->mapPhyAddr(&mpbuf_camsv_imgo_fh[i]) < 0 ){
                PIPE_ERR("mapPhyAddr() error fh(%d)", i);
                return MFALSE;
            }

            //save address head then free valid memory address when struct dectroy
            mp_org_va_imgo_fh[i] = mpbuf_camsv_imgo_fh[i].virtAddr;
            mp_org_pa_imgo_fh[i] = mpbuf_camsv_imgo_fh[i].phyAddr;
            //force to 64bit alignment wheather or not.
            mpbuf_camsv_imgo_fh[i].virtAddr = ((mpbuf_camsv_imgo_fh[i].virtAddr + 15) >> 4) << 4;
            mpbuf_camsv_imgo_fh[i].phyAddr = ((mpbuf_camsv_imgo_fh[i].phyAddr + 15) >> 4) << 4;
        }

        _in_port = &camsv_tgi;
        _out_port = &camsv_imgo;

        switch(eTgInputCamsv) {
            case TG_CAMSV_0: // 10 ( 0x0A )
                _in_port->index = EPortIndex_CAMSV_0_TGI;
                break;
            case TG_CAMSV_1: // 11 ( 0x0B )
                _in_port->index = EPortIndex_CAMSV_1_TGI;
                break;
            case TG_CAMSV_2: // 12 ( 0x0C )
                _in_port->index = EPortIndex_CAMSV_2_TGI;
                break;
            case TG_CAMSV_3: // 13 ( 0x0D )
                _in_port->index = EPortIndex_CAMSV_3_TGI;
                break;
            case TG_CAMSV_4: // 14 ( 0x0D )
                _in_port->index = EPortIndex_CAMSV_4_TGI;
                break;
            case TG_CAMSV_5: // 15 ( 0x0F )
                _in_port->index = EPortIndex_CAMSV_5_TGI;
                break;
            default:
                PIPE_ERR("Unsupported TG Input(%d)", eTgInputCamsv);
                return MFALSE;
                break;
        }


        _in_port->ePxlMode     = SPIPE_MAP_PXLMODE(mpSensorIdx, mFeatureIdx);
        _in_port->eImgFmt      = tgi_eImgFmt[mpSensorIdx];
#ifdef CAMSV_FORCE_SENSOR_PIXEL_ID
        _in_port->eRawPxlID    = ERawPxlID_Gb; // don't care for virtual channel
#else
        _SPIPE_GET_TGI_PIX_ID(mpSensorIdx, _in_port->eRawPxlID);
#endif

        _in_port->u4ImgWidth   = _pIMG_W;
        _in_port->u4ImgHeight  = imgH;
        _in_port->crop1.x      = 0;
        _in_port->crop1.y      = 0;
        _in_port->crop1.floatX = 0;
        _in_port->crop1.floatY = 0;
        _in_port->crop1.w      = _in_port->u4ImgWidth;
        _in_port->crop1.h      = _in_port->u4ImgHeight;
        _in_port->type         = EPortType_Sensor;
        _in_port->inout        = EPortDirection_In;
        //_port->tgFps         = vInPorts.mSensorCfg.at(0).framerate; // _CAM_SV_ : UT : FIX ME
        _in_port->tTimeClk     = _SPIPE_GET_CLK_FREQ(mpSensorIdx)/100; //0.1MHz <- KHz
        vCamsvIOInPorts.push_back(_in_port);

        /*if(this->bypass_imgo == MFALSE)*/ /* _CAM_SV_ : UT : FIX ME */
        {
            _out_port->index          = EPortIndex_CAMSV_IMGO;
            _out_port->ePxlMode       = _in_port->ePxlMode;
            _out_port->eImgFmt        = _in_port->eImgFmt;
            _out_port->u4PureRaw      = MFALSE;
            _out_port->u4PureRawPak   = MTRUE;
            _out_port->u4ImgWidth     = xsize_byte;//imgW;
            _out_port->u4ImgHeight    = _in_port->u4ImgHeight;
            _out_port->crop1.x        = 0;
            _out_port->crop1.y        = 0;
            _out_port->crop1.floatX   = 0;
            _out_port->crop1.floatY   = 0;
            _out_port->crop1.w        = _pIMG_W;
            _out_port->crop1.h        = _out_port->u4ImgHeight;
            _out_port->u4Stride[ePlane_1st] = mCamsvImgoBufSize / _out_port->crop1.h;

            //prepare for enque if needed
            camsv_imgo_crop_w = _pIMG_W;
            camsv_imgo_crop_h = imgH;

            _out_port->u4Stride[ePlane_2nd] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
            _out_port->u4Stride[ePlane_3rd] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
            _out_port->type   = EPortType_Memory;
            _out_port->inout  = EPortDirection_Out;
            vCamsvIOOutPorts.push_back(_out_port);
        }

        // config camsv iopipe
        func.Bits.SUBSAMPLE = this->m_subSample;
        if(mpCamsvIOPipe->configPipe(vCamsvIOInPorts, vCamsvIOOutPorts, &func) == MFALSE){
            mpCamsvIOPipe->uninit();
            mpCamsvIOPipe->destroyInstance();
            mpCamsvIOPipe = NULL;
            PIPE_ERR("TG_Camsv_%d %s configPipe fail", eTgInputCamsv, "config");
            mErrorCode = ECAMSVSPipeErrorCode_CAMSAV_IOPIPE_CONFIGPIPE_FAIL;
            mOpenedPort &= ~CamsvStatisticPipe::_CAMSV_IMGO_ENABLE;
            return MFALSE;
        }

        // finish config part
        mOpenedPort |= CamsvStatisticPipe::_CAMSV_IMGO_ENABLE;
    } while(0);

    PIPE_INF("-, ret(%d)", ret);

    this->FSM_UPDATE(CamsvStatisticPipe::op_cfg);

    {//need to enque before start.
        QBufInfo rQBuf;
        this->immediateEnque(rQBuf);
    }
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 CamsvStatisticPipe::attach(const char* UserName)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(CamsvStatisticPipe::op_cmd, __FUNCTION__) == MFALSE) {
        return -1;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->attach(UserName);
    }
    else {
        PIPE_ERR("instance not create");
        return -1;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(CamsvStatisticPipe::op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->wait(eSignal, eClear, mUserKey, TimeoutMs, pTime);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

MBOOL CamsvStatisticPipe::signal(EPipeSignal eSignal, const MINT32 mUserKey)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(CamsvStatisticPipe::op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->signal(eSignal, mUserKey);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::abortDma(PortID port, char const* szCallerName)
{
    NSImageio::NSIspio::PortID portID;

    if (this->FSM_CHECK(CamsvStatisticPipe::op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_DBG("+, usr[%s]dma[0x%x]SenIdx[%d]", szCallerName, port.index, mpSensorIdx);

    if (mpCamsvIOPipe) {
        switch (port.index){
        case EPortIndex_CAMSV_IMGO:
            break;
        default:
            PIPE_ERR("not support dma(0x%x)user(%s)\n", port.index, szCallerName);
            return MFALSE;
        }
        portID.index = port.index;
        return mpCamsvIOPipe->abortDma(portID);
    }
    else {
        PIPE_ERR("instance not create, user(%s)", szCallerName);
        return MFALSE;
    }
}

MUINT32 CamsvStatisticPipe::getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(CamsvStatisticPipe::op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->getIspReg(RegAddr, RegCount, RegData, bPhysical);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 CamsvStatisticPipe::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(CamsvStatisticPipe::op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpNormalPipe) {
        return mpNormalPipe->getIspReg(pRegs, RegCount, bPhysical);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 CamsvStatisticPipe::getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (mpNormalPipe) {
        return mpNormalPipe->getIspUniReg(pRegs, RegCount, bPhysical);
    }
    else {
        PIPE_ERR("instance not create");
        return MFALSE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;
    MUINT32 tg;
    //this is a special case
    if(cmd == ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID){
        if (arg1) {
            this->mscenarioId = *(MINT32*)arg1;
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
            ret = mpNormalPipe->sendCommand(ENPipeCmd_GET_TG_INDEX, arg1, arg2, arg3);
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
            ret = mpNormalPipe->sendCommand(ENPipeCmd_GET_TG_OUT_SIZE, arg1, arg2, arg3);
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



