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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG     "NormalPipe"

#include <sys/prctl.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
using namespace android;

#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
using namespace NSCam;
using namespace NSCam::Utils;

// for thread priority
#include <mtkcam/def/PriorityDefs.h>

#include <utils/Thread.h>
#include <utils/Condition.h>
#include <cutils/properties.h>

#include <IPipe.h>
#include <ICamIOPipe.h>
#include <ispio_utility.h>
#include <ispio_pipe_ports.h>
#include <Cam_Notify_datatype.h>
#include <Cam/ICam_capibility.h>

#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>


#include "IOPipeProfile.h"
#include "NormalPipe.h"
#include "camio_log.h"
#include "FakeSensor.h"
//for N3D
#include <mtkcam/drv/IHwSyncDrv.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif
#define PTHREAD_CTRL_M      (1)

#define THIS_NAME           "NormalPipe"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;

DECLARE_DBG_LOG_VARIABLE(iopipe);
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (iopipe_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (iopipe_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (iopipe_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (iopipe_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (iopipe_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (iopipe_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

/******************************************************************************
 *
 ******************************************************************************/

//map portId to deque container index
#define _PortMap(PortIdx)   ({\
    MUINT32 _idx = 0;\
    if (PortIdx == PORT_RRZO.index)\
        _idx = 0;\
    else if (PortIdx == PORT_IMGO.index)\
        _idx = 1;\
    else if(PortIdx == PORT_EISO.index)\
        _idx = 2;\
    else if(PortIdx == PORT_LCSO.index)\
        _idx = 3;\
    else if(PortIdx == PORT_UFEO.index)\
        _idx = 4;\
    else if(PortIdx == PORT_RSSO.index)\
        _idx = 5;\
    else PIPE_ERR("error:portidx:0x%x\n",PortIdx);\
    _idx;\
})

static const NSImageio::NSIspio::EPortIndex m_Slot2PortIndex[] = {
    EPortIndex_RRZO, EPortIndex_IMGO, EPortIndex_EISO, EPortIndex_LCSO, EPortIndex_UFEO, EPortIndex_RSSO
};
#define _MapPort(index) ({\
    if (index >= MaxPortIdx) {\
        PIPE_ERR("Invalid _port %d", index);\
    }\
    m_Slot2PortIndex[index];\
})

// mapping element NormalPipe::mTgOut_size
#define _TGMapping(x)({\
    MUINT32 _tg=0;\
    if(x==CAM_TG_1)\
        _tg = 0;\
    else if(x==CAM_TG_2)\
        _tg = 1;\
    else if(x == CAM_SV_1)\
        _tg = 2;\
    else if(x == CAM_SV_2)\
        _tg = 3;\
    _tg;\
})

//switch to camiopipe index
#define _NORMALPIPE_GET_TGIFMT(sensorIdx,fmt) {\
    switch (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType) {\
        case SENSOR_TYPE_RAW:\
            switch(NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit) {\
                case RAW_SENSOR_8BIT:   fmt = NSCam::eImgFmt_BAYER8; break;\
                case RAW_SENSOR_10BIT:  fmt = NSCam::eImgFmt_BAYER10; break;\
                case RAW_SENSOR_12BIT:  fmt = NSCam::eImgFmt_BAYER12; break;\
                case RAW_SENSOR_14BIT:  fmt = NSCam::eImgFmt_BAYER14; break;\
                default: PIPE_ERR("Err sen raw fmt(%d) err\n", NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.rawSensorBit); break;\
            }\
            break;\
        case SENSOR_TYPE_YUV:\
            switch(NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder) {\
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
            PIPE_ERR("Err sen type(%d) err\n", NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType);\
            break;\
    }\
}

//switch to camiopipe idx
#define _NORMALPIPE_GET_TGI_PIX_ID(sensorIdx,pix_id) do{\
    switch(NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder){\
        case SENSOR_FORMAT_ORDER_RAW_B: pix_id = ERawPxlID_B; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gb: pix_id = ERawPxlID_Gb; break;\
        case SENSOR_FORMAT_ORDER_RAW_Gr: pix_id = ERawPxlID_Gr; break;\
        case SENSOR_FORMAT_ORDER_RAW_R: pix_id = ERawPxlID_R; break;\
        case SENSOR_FORMAT_ORDER_UYVY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_VYUY: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YUYV: pix_id = (ERawPxlID)0; break;\
        case SENSOR_FORMAT_ORDER_YVYU: pix_id = (ERawPxlID)0; break;\
        default:    PIPE_ERR("Error Pix_id: sensorIdx=%d, sensorFormatOrder=%d", sensorIdx, NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorFormatOrder); break;\
    }\
}while(0);

#define _NORMALPIPE_GET_TG_GRAB_START(sensorIdx, scenarioId, x, y) do{\
    switch(scenarioId) {\
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_PRV;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_PRV; break;\
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CAP;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CAP; break;\
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD; break;\
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD1;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD1; break;\
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_VD2;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_VD2; break;\
        case SENSOR_SCENARIO_ID_CUSTOM1:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST1;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST1; break;\
        case SENSOR_SCENARIO_ID_CUSTOM2:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST2;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST2; break;\
        case SENSOR_SCENARIO_ID_CUSTOM3:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST3;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST3; break;\
        case SENSOR_SCENARIO_ID_CUSTOM4:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST4;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST4; break;\
        case SENSOR_SCENARIO_ID_CUSTOM5:\
            x = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartX_CST5;\
            y = NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.SensorGrabStartY_CST5; break;\
        default: x = 0; y = 0; PIPE_ERR("Error: scnario:%d sensorIdx:%d", scenarioId, sensorIdx); break;\
    }\
}while(0);

#define _NORMALPIPE_GET_SENSORTYPE(sensorIdx) (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.sensorType)
//switch to camiopipe index
#define _NOMRALPIPE_MAP_TG_IDX(sensorIdx)({\
    MUINT32 tgidx = 0;\
    switch(NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo) {\
    case CAM_TG_1: tgidx = EPortIndex_TG1I; break;\
    case CAM_TG_2: tgidx = EPortIndex_TG2I; break;\
    case CAM_SV_1: tgidx = EPortIndex_CAMSV_TG1I; break;\
    case CAM_SV_2: tgidx = EPortIndex_CAMSV_TG2I; break;\
    default:    PIPE_ERR("Error tg idx mapping"); break;\
    }\
    tgidx;\
})

#define _NORMALPIPE_MAP_TG_PXLMODE(sensorIdx) ({\
    NSImageio::NSIspio::EPxlMode tgPxlMode = ePxlMode_One_;\
    switch (_NORMALPIPE_GET_PIX_MODE(sensorIdx)) {\
    case 0: tgPxlMode = NSImageio::NSIspio::ePxlMode_One_; break;\
    case 1: tgPxlMode = NSImageio::NSIspio::ePxlMode_Two_; break;\
    case 2: tgPxlMode = NSImageio::NSIspio::ePxlMode_Four_; break;\
    default: PIPE_ERR("Unknown tg pixel mode mapping"); break;\
    }\
    tgPxlMode;\
})

#define _NORMALPIPE_GET_PXLMODE_QUERY(sensorIdx, _tgPxlMode) ({\
    NSImageio::NSIspio::E_ISP_PIXMODE ePxlModeQuery;\
    switch (_tgPxlMode) {\
    case NSImageio::NSIspio::ePxlMode_One_: ePxlModeQuery = ISP_QUERY_1_PIX_MODE; break;\
    case NSImageio::NSIspio::ePxlMode_Two_: ePxlModeQuery = ISP_QUERY_2_PIX_MODE; break;\
    case NSImageio::NSIspio::ePxlMode_Four_:\
    default: ePxlModeQuery = ISP_QUERY_UNKNOWN_PIX_MODE;\
        PIPE_ERR("Unknown pixel mode %d", _tgPxlMode); break;\
    }\
    ePxlModeQuery;\
})

#define _NORMALPIPE_GET_SENSOR_WIDTH(sensorIdx) (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureWidth)
#define _NORMALPIPE_GET_SENSOR_HEIGHT(sensorIdx) (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mSInfo.captureHeight)

#define _NORMALPIPE_GET_SENSORCFG_CROP_W(sensorIdx) (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.w)
#define _NORMALPIPE_GET_SENSORCFG_CROP_H(sensorIdx) (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mConfig.crop.h)
#define _NORMALPIPE_GET_SENSOR_DEV_ID(sensorIdx)    (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDevId)

#define _NORMALPIPE_GET_TG_IDX(sensorIdx)     (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgInfo)
#define _NORMALPIPE_GET_PIX_MODE(sensorIdx)   (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.pixelMode)
#define _NORMALPIPE_GET_CLK_FREQ(sensorIdx)   (NormalPipe::pAllSensorInfo->mSenInfo[sensorIdx].mDInfo.TgCLKInfo)

#define _SensorTypeToPortFormat(sensorType) ({\
    NSImageio::NSIspio::EScenarioFmt _fmt = NSImageio::NSIspio::eScenarioFmt_RAW;\
    switch(sensorType){\
        case NSCam::SENSOR_TYPE_RAW:  _fmt = NSImageio::NSIspio::eScenarioFmt_RAW;\
            break;\
        case NSCam::SENSOR_TYPE_YUV:  _fmt = NSImageio::NSIspio::eScenarioFmt_YUV;\
            break;\
        case NSCam::SENSOR_TYPE_RGB:  _fmt = NSImageio::NSIspio::eScenarioFmt_RGB;\
            break;\
        case NSCam::SENSOR_TYPE_JPEG: _fmt = NSImageio::NSIspio::eScenarioFmt_JPG;\
            break;\
        default:\
            break;\
   }\
   _fmt;\
})

#define _NOMRALPIPE_MAP_SIGNAL(eSignal, status, statusType) ({\
    MBOOL ret = MTRUE;\
    switch (eSignal) {\
    case EPipeSignal_SOF: status = Irq_t::_SOF_; statusType = Irq_t::_SIGNAL_INT_; break;\
    case EPipeSignal_EOF: status = Irq_t::_EOF_; statusType = Irq_t::_SIGNAL_INT_; break;\
    case EPipeSignal_VSYNC: status = Irq_t::_VSYNC_; statusType = Irq_t::_SIGNAL_INT_; break;\
    case EPipeSignal_AFDONE: status = Irq_t::_AF_DONE_; statusType = Irq_t::_DMA_INT_; break;\
    case EPipeSignal_TG_INT: status = Irq_t::_TG_INT_; statusType = Irq_t::_SIGNAL_INT_; break;\
    default:\
        PIPE_ERR("Invalid Signal(%d,)", eSignal);\
        ret = MFALSE; break;\
    }\
    ret;\
})

#ifdef USE_IMAGEBUF_HEAP
    #define _NOMRALPIPE_PASS_BUFINFO(src, dst, plane_id) do {\
        dst.u4BufSize[0] = (MUINT32)src.mBuffer->getBufSizeInBytes(plane_id);\
        dst.u4BufVA[0] = (MUINTPTR)src.mBuffer->getBufVA(plane_id);\
        dst.u4BufPA[0] = (MUINTPTR)src.mBuffer->getBufPA(plane_id);\
        dst.memID[0] = (MUINTPTR)src.mBuffer->getFD(plane_id);\
    } while(0)
    #define _NOMRALPIPE_BUF_GETPA(buf, plane_id) (buf.mBuffer->getBufPA(plane_id))
    #define _NOMRALPIPE_BUF_GETVA(buf, plane_id) (buf.mBuffer->getBufVA(plane_id))
    #define _NOMRALPIPE_BUF_GETSZ(buf, plane_id) (buf.mBuffer->getBufSizeInBytes(plane_id))
#else
    #define _NOMRALPIPE_PASS_BUFINFO(src, dst, plane_id) do {\
        dst.u4BufSize[0] = (MUINT32)src.mSize;\
        dst.u4BufVA[0] = (MUINTPTR)src.mVa;\
        dst.u4BufPA[0] = (MUINTPTR)src.mPa;\
    } while(0)
    #define _NOMRALPIPE_BUF_GETPA(buf, plane_id) (buf.mPa)
    #define _NOMRALPIPE_BUF_GETVA(buf, plane_id) (buf.mVa)
    #define _NOMRALPIPE_BUF_GETSZ(buf, plane_id) (buf.mSize)
#endif
#define _NOMRALPIPE_SET_BUFEMPTY(_buf) do {\
    memset((void*)&_buf, 0, sizeof(_buf));\
    _buf.mPortID.index = (MUINT8)-1;\
} while(0)

#define NPIPE_MEM_NEW(dstPtr,type,size)\
do {\
    NPipeAllocMemSum += size; \
    dstPtr = new type;\
} while(0)

#define NPIPE_MEM_DEL(dstPtr,size)\
do{\
    NPipeFreedMemSum += size; \
    delete dstPtr;\
    dstPtr = NULL;\
}while(0)

#define NPIPE_DUMP_MEM_INFO(string) \
do {\
    PIPE_DBG("%s::NPipeAllocMemSum=0x%x, NPipeFreedMemSum=0x%x", string, NPipeAllocMemSum, NPipeFreedMemSum);\
}while(0)


static MUINT32      NPipeAllocMemSum = 0;
static MUINT32      NPipeFreedMemSum = 0;

Mutex               NormalPipe::NPipeGLock;
Mutex               NormalPipe::NPipeCfgGLock;
Mutex               NormalPipe::NPipeDbgLock;
Mutex               NormalPipe::NPipeUniSwitchLock;
NormalPipe*         NormalPipe::pNormalPipe[EPIPE_Sensor_RSVD] = {NULL, NULL, NULL, NULL};
platSensorsInfo*    NormalPipe::pAllSensorInfo = NULL;
MUINT32             NormalPipe::mEnablePath = 0;
MUINT32             NormalPipe::mFirstEnableTG = CAM_TG_NONE; /* Set at 1st config, clear when all cam stopped */
uniSwitchInfo*       NormalPipe::m_uniSwitchInfo = NULL;


#if (CONFIG_IOPIPE_PROFILING == 1)
MUINT32             IOPipeTrace::m_traceEnable = MFALSE;
MUINT32             IOPipeTrace::m_metPrfEnable = MFALSE;
MUINT32             IOPipeTrace::m_userCount = 0;
Mutex               IOPipeTrace::m_traceLock;
#endif

/******************************************************************************
 *
 ******************************************************************************/
extern "C"
INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName)
{
    NormalPipe* pNPipe = NULL;

    PIPE_DBG("+");

    NormalPipe::NPipeGLock.lock();

    if (NormalPipe::pAllSensorInfo == NULL) {
        IHalSensorList *mSList = NULL;
        _sensorInfo_t  *pSInfo = NULL;
        MUINT32 SCnt = 0, i = 0;

        MINT32 retTrace = IOPipeTrace::createTrace();
        if (retTrace < 0) {
            PIPE_ERR("enable trace fail: ret: %d", retTrace);
        }
        else if (retTrace > 0) {
            PIPE_INF("enable trace: ret: 0x%x", retTrace);
        }

        NPIPE_MEM_NEW(NormalPipe::pAllSensorInfo, platSensorsInfo, sizeof(platSensorsInfo));
        PIPE_INF("N:%d,%s,pAllSensorInfo=0x%8x", SensorIndex, szCallerName, NormalPipe::pAllSensorInfo);

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

        //#ifdef USING_MTK_LDVT
        //mSList->searchSensors();
        //#endif
        SCnt =  mSList->queryNumberOfSensors();

        NormalPipe::pAllSensorInfo->mSList          = mSList;
        NormalPipe::pAllSensorInfo->mExistedSCnt    = SCnt;
        NormalPipe::pAllSensorInfo->mUserCnt        = 0;
        if ((SCnt > IOPIPE_MAX_SENSOR_CNT) || (SCnt == 0)) {
            PIPE_ERR("Not support %d sensors", SCnt);
            NPIPE_MEM_DEL(NormalPipe::pAllSensorInfo, sizeof(platSensorsInfo));
            NormalPipe::pAllSensorInfo = NULL;
            NormalPipe::NPipeGLock.unlock();
            return MFALSE;
        }

        for (i = 0; i < SCnt; i++) {
            pSInfo = &NormalPipe::pAllSensorInfo->mSenInfo[i];

            pSInfo->mIdx       = i;
            pSInfo->mTypeforMw = mSList->queryType(i);
            pSInfo->mDevId     = mSList->querySensorDevIdx(i);
            mSList->querySensorStaticInfo(pSInfo->mDevId, &pSInfo->mSInfo);
            PIPE_INF("N:%d,SensorName=%s,Type=%d,DevId=%d", i, mSList->queryDriverName(i),\
                    pSInfo->mTypeforMw, pSInfo->mDevId);
        }

        PIPE_DBG("Attach seninf dump callback");
        SENINF_DBG::m_fp_Sen = NormalPipe::dumpSeninfDebugCB;
    }
    NormalPipe::pAllSensorInfo->mUserCnt++;

    if(NormalPipe::m_uniSwitchInfo == NULL) {
        NPIPE_MEM_NEW(NormalPipe::m_uniSwitchInfo, uniSwitchInfo, sizeof(uniSwitchInfo));
    }

    NormalPipe::NPipeGLock.unlock();

    if (SensorIndex >= EPIPE_Sensor_RSVD) {
        PIPE_ERR("InvalidSensorIdx = %d", SensorIndex);
        return MFALSE;
    }

    NormalPipe::NPipeGLock.lock();
    pNPipe = NormalPipe::pNormalPipe[SensorIndex];
    if (NULL == pNPipe) {
        NPIPE_MEM_NEW(NormalPipe::pNormalPipe[SensorIndex], NormalPipe(SensorIndex, THIS_NAME), sizeof(NormalPipe));
        pNPipe = NormalPipe::pNormalPipe[SensorIndex];

        PIPE_INF("%s NPipe[%d]=0x%8x create", szCallerName, SensorIndex, pNPipe);

        if (pNPipe->mpNPipeThread == NULL){
            pNPipe->mpNPipeThread = NormalPipe_Thread::createInstance(pNPipe);
            if (NULL == pNPipe->mpNPipeThread){
                PIPE_ERR("error: Thread:createinstance fail");
                return NULL;
            }
        }
    }

    pNPipe->addUser(szCallerName); //pNPipe->mUserCnt++;

    NormalPipe::NPipeGLock.unlock();

    PIPE_DBG("- NPipe[%d](0x%8x) mUserCnt(%d)", SensorIndex, pNPipe, pNPipe->mTotalUserCnt);

    return pNPipe;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID NormalPipe::dumpSeninfDebugCB(MUINT32 ispModule, MUINT32 tgStat)
{
#define NPIPE_DBG_USER      "NPipeDbg"
    MUINT32 idx = 0;
    MUINT32 camTg = CAM_TG_NONE;
    INormalPipe *pNPipeObj = NULL;

    NormalPipe::NPipeDbgLock.lock();

    switch (ispModule) {
    case CAM_A:
        camTg = CAM_TG_1;
        break;
    case CAM_B:
        camTg = CAM_TG_2;
        break;
    default:
        PIPE_ERR("ERROR: unknow isp hw module : %d, dump all", ispModule);
        NormalPipe::NPipeDbgLock.unlock();

        dumpSeninfDebugCB(CAM_A, tgStat);
        dumpSeninfDebugCB(CAM_B, tgStat);
        return;
    }

    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++) {
        if (_NORMALPIPE_GET_TG_IDX(idx) == camTg) {
            break;
        }
    }
    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("ERROR: unknow tg index : %d", camTg);
        goto _EXIT_DUMP;
    }

    if (NormalPipe::pNormalPipe[idx]) {
        MINT32 seninfStatus = 0;

        PIPE_WRN("Start dump seninf debug info: sensor_idx:%d sensor_devid:x%x",
                    idx, NormalPipe::pAllSensorInfo->mSenInfo[idx].mDevId);
        seninfStatus = NormalPipe::pNormalPipe[idx]->mpHalSensor->sendCommand(NormalPipe::pAllSensorInfo->mSenInfo[idx].mDevId,
                                                                SENSOR_CMD_DEBUG_P1_DQ_SENINF_STATUS, 0, 0, 0);
        if ((1 == tgStat) || (0 != seninfStatus)) {
            NormalPipe::pNormalPipe[idx]->mResetCount = MAX_P1_RST_COUNT;
        }
    }
    else {
        PIPE_ERR("ERROR: NormalPipe[%d] object not create yet", idx);
        goto _EXIT_DUMP;
    }

_EXIT_DUMP:
    NormalPipe::NPipeDbgLock.unlock();
}


/******************************************************************************
 *
 ******************************************************************************/

MVOID NormalPipe::destroyInstance(char const* szCallerName)
{
    MUINT32 idx = 0;

    PIPE_DBG("+ : %s", szCallerName);

    NPipeGLock.lock();
    for (idx = 0; idx < EPIPE_Sensor_RSVD; idx++){
        if (this == pNormalPipe[idx]) {
            break;
        }
    }

    if (EPIPE_Sensor_RSVD == idx) {
        PIPE_ERR("Can't find the entry");
        NPipeGLock.unlock();
        return;
    }

    if ((mTotalUserCnt <= 0)){
        PIPE_DBG("No instance now");
        NPipeGLock.unlock();
        return;
    }

    if (!delUser(szCallerName)) {
        PIPE_ERR("No such user : %s", szCallerName);
        NPipeGLock.unlock();
        return;
    }

    if ((mTotalUserCnt == 0))  {
        if (mpHalSensor) {
            mpHalSensor->destroyInstance(THIS_NAME);
            mpHalSensor = NULL;
        }

        if (mpNPipeThread) {
            mpNPipeThread->destroyInstance();
            mpNPipeThread = NULL;
        }

        if (mpCamIOPipe) {
            mpCamIOPipe->destroyInstance();
            mpCamIOPipe = NULL;
        }

        if (mpIMem) {
            if (mImgHeaderPool.virtAddr) {
                if (mpIMem->unmapPhyAddr(&mImgHeaderPool)) {
                    PIPE_ERR("imgheader unmap error %p", mImgHeaderPool.phyAddr);
                }
                if (mpIMem->freeVirtBuf(&mImgHeaderPool)) {
                    PIPE_ERR("imgheader free error %p", mImgHeaderPool.virtAddr);
                }
                mImgHeaderPool.virtAddr = 0;
                mImgHeaderPool.phyAddr = 0;
            }

            if (mvSttMem.size()) {
                MUINT32 i;

                PIPE_DBG("Post free mem");
                for (i = 0; i < mvSttMem.size(); i++) {
                    if (mpIMem->unmapPhyAddr(&(mvSttMem.at(i)))) {
                        PIPE_ERR("sttMem unmap error %p", mImgHeaderPool.phyAddr);
                    }
                    if (mpIMem->freeVirtBuf(&(mvSttMem.at(i)))) {
                        PIPE_ERR("sttMem free error %p", mImgHeaderPool.phyAddr);
                    }
                }
                mvSttMem.clear();
            }
            mpIMem->uninit();
            mpIMem->destroyInstance();
            mpIMem = NULL;
        }

        PIPE_INF("Destroy NPipe[%d]=0x%8x", idx, pNormalPipe[idx]);
        NPIPE_MEM_DEL(pNormalPipe[idx], sizeof(NormalPipe));
    }

    if (NormalPipe::pAllSensorInfo) {
       if (NormalPipe::pAllSensorInfo->mUserCnt > 0) {
           NormalPipe::pAllSensorInfo->mUserCnt--;
       }
       else {
          //return;
       }

       if (NormalPipe::pAllSensorInfo->mUserCnt == 0) {
           PIPE_INF("Destroy pAllSensorInfo. idx=%d", idx);
           NPIPE_MEM_DEL(NormalPipe::pAllSensorInfo, sizeof(platSensorsInfo));
           NPIPE_DUMP_MEM_INFO("Destroy");

            if(NormalPipe::m_uniSwitchInfo) {
                NPIPE_MEM_DEL(NormalPipe::m_uniSwitchInfo, sizeof(m_uniSwitchInfo));
            }

            PIPE_DBG("Detach seninf dump callback");
            SENINF_DBG::m_fp_Sen = NULL;

            if (IOPipeTrace::destroyTrace() < 0) {
                PIPE_ERR("destroyTrace failed...");
            }
       }

    }

    NPipeGLock.unlock();

    PIPE_DBG("-");
}


/******************************************************************************
 *
 ******************************************************************************/
NormalPipe::NormalPipe(MUINT32 SensorIdx, char const* szCallerName)
    : mpHalSensor(NULL)
    , mBurstQNum(1)
    , mpCamIOPipe(NULL)
    #ifdef USE_IMAGEBUF_HEAP
    , mpFrameMgr(NULL)
    #endif
    , mpName(szCallerName)
    , mTotalUserCnt(0)
    , mpSensorIdx(SensorIdx)
    , m_FSM(op_unknown)
    , mInitSettingApplied(MFALSE)
    , mDynSwtRawType(MTRUE)
    , mPureRaw(MFALSE)
    , mResetCount(0)
    , mOpenedPort(_DMAO_NONE)
    , mPrvEnqSOFIdx(0xFFFFFFFF)
    , mReqDepthPreStart(0)
    , m_b1stEnqLoopDone(MFALSE)
    , mpNPipeThread(NULL)
    , m_DropCB(NULL)
    , m_returnCookie(NULL)
    , m_SofCB(NULL)
    , m_SofCBCookie(NULL)
    , mpIMem(NULL)
{
    DBG_LOG_CONFIG(imageio, iopipe);

    mRrzoOut_size.w = 0;
    mRrzoOut_size.h = 0;
    mImgoOut_size.w = 0;
    mImgoOut_size.h = 0;

    for (MUINT32 i = 0; i < MaxPortIdx; i++) {
        PortImgFormat[i] = eImgFmt_UNKNOWN;
    }
    memset((void*)mpEnqueRequest, 0, sizeof(mpEnqueRequest));
    memset((void*)mpEnqueReserved, 0, sizeof(mpEnqueReserved));
    memset((void*)mpDeQueue, 0, sizeof(mpDeQueue));
    memset((void*)PortIndex, 0, sizeof(PortIndex));
    memset((void*)mUserCnt, 0, sizeof(mUserCnt));
    memset((void*)mUserName, 0, sizeof(mUserName));
    mvSttMem.clear();
    m_bN3DEn = MFALSE;
}


MBOOL NormalPipe::FSM_CHECK(E_FSM op, const char *callee)
{
    MBOOL ret = MTRUE;

    this->m_FSMLock.lock();

    switch(op){
    case op_unknown:
        if(this->m_FSM != op_uninit)
            ret = MFALSE;
        break;
    case op_init:
        if(this->m_FSM != op_unknown)
            ret = MFALSE;
        break;
    case op_cfg:
        if(this->m_FSM != op_init)
            ret = MFALSE;
        break;
    case op_start:
        switch (this->m_FSM) {
        case op_cfg:
        case op_suspend:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case op_stop:
        switch (this->m_FSM) {
        case op_start:
        case op_suspend:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case op_suspend:
        if (this->m_FSM != op_start) {
            ret = MFALSE;
        }
        break;
    case op_uninit:
        switch(this->m_FSM){
        case op_init:
        case op_cfg:
        case op_stop:
            break;
        default:
            ret = MFALSE;
            break;
        }
        break;
    case op_cmd:
        switch(this->m_FSM){
        case op_cfg:
        case op_start:
        case op_stop:
        case op_suspend:
            break;
        default:
            ret= MFALSE;
            break;
        }
        break;
    case op_streaming:
        switch(this->m_FSM){
        case op_cfg:
        case op_start:
            break;
        default:
            ret= MFALSE;
            break;
        }
        break;
    default:
        ret = MFALSE;
        break;
    }
    if (ret == MFALSE) {
        PIPE_ERR("[%s]op error:cur:0x%x,tar:0x%x\n", callee, this->m_FSM,op);
    }

    this->m_FSMLock.unlock();

    return ret;
}

MBOOL NormalPipe::FSM_UPDATE(E_FSM op)
{
    switch(op){
    case op_unknown:
    case op_init:
    case op_cfg:
    case op_start:
    case op_stop:
    case op_suspend:
    case op_uninit:
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


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::start()
{
    IOPipeTrace _traceObj("NormalPipe::start", IOPIPE_TRACE_EN_NOMALPIPE);
    Mutex::Autolock lock(mCfgLock);

    MUINT32 _size = 0;

    PIPE_DBG("E:");

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    /*
     * Consume 1st element, which alread applied in immediate request
     */
    if (MTRUE == mInitSettingApplied) {
        MUINT32     i, _port;

        mEnqContainerLock.lock();

        for (i = 0; i < GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum); i++) {
            _port = _PortMap(GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

            mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_pop, 0, 0);
        }

        mEnqContainerLock.unlock();

        if (mpNPipeThread) {
            if (mpNPipeThread->m_ConsumedReqCnt >= mReqDepthPreStart) {
                PIPE_DBG("N:%d static loop done 0(%d)", mpSensorIdx, mpNPipeThread->m_ConsumedReqCnt);
                m_b1stEnqLoopDone = MTRUE;
            }
        }
    }
    else {
        PIPE_ERR("no enque/p1_update before start");
        return MFALSE;
    }

    /* start: CamIo -> NPipeThread to prevent timing issue below */
    /* start: NPipeThread -> CamIo may happen enqueRequest run during CamIo start when performance low */
    if (mpCamIOPipe) {
        if (MFALSE == mpCamIOPipe->start()) {
            PIPE_ERR("N:%d start fail", mpSensorIdx);
            return MFALSE;
        }
    }

    if (mpNPipeThread) {
        if (mpNPipeThread->start() == MFALSE) {
            PIPE_ERR("error: NPipeThread start fail\n");
            return MFALSE;
        }
    }

    PIPE_DBG("X:");

    this->FSM_UPDATE(op_start);

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::stop(MBOOL bNonblocking)
{
    IOPipeTrace _traceObj("NormalPipe::Stop", IOPIPE_TRACE_EN_NOMALPIPE);
    Mutex::Autolock lock(mCfgLock);

    bool ret = MTRUE;

    if (this->FSM_CHECK(op_stop, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if(this->m_bN3DEn == MTRUE){
        HWSyncDrv* pHwSync = NULL;
        pHwSync = HWSyncDrv::createInstance();
        if(pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, \
                HW_SYNC_BLOCKING_EXE, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,0) != 0){
                PIPE_ERR("N3D stop fail\n");
        }
        pHwSync->destroyInstance();
        this->m_bN3DEn = MFALSE;
    }

    PIPE_INF("N:%d stop+ %s", mpSensorIdx, (bNonblocking? "Non-blocking": "Blocking"));

    if (bNonblocking) {
        NormalPipe_Thread::E_ThreadGrop thdGrp;

        switch(this->m_FSM) {
        case op_start:
            thdGrp = (NormalPipe_Thread::E_ThreadGrop)(NormalPipe_Thread::eThdGrp_Internal|NormalPipe_Thread::eThdGrp_External);
            break;
        case op_suspend:
            thdGrp = NormalPipe_Thread::eThdGrp_External;
            break;
        default:
            PIPE_ERR("Unknown state: %d\n", this->m_FSM);
            return MFALSE;;
        }

        /* stop isp hw first */
        if (mpCamIOPipe) {
            /* hold idle lock */
            if (mpNPipeThread) {
                mpNPipeThread->idleStateHold(1, thdGrp);
            }

            PIPE_TRACE_BEGIN("TAG_CAM_STOP");
            ret = mpCamIOPipe->stop(MFALSE, MFALSE);
            PIPE_TRACE_END("TAG_CAM_STOP");

            if (mpNPipeThread) {
                this->mpNPipeThread->prestop();
            }

            /* release idle lock */
            if (mpNPipeThread) {
                mpNPipeThread->idleStateHold(0,
                    (NormalPipe_Thread::E_ThreadGrop)(NormalPipe_Thread::eThdGrp_Internal|NormalPipe_Thread::eThdGrp_External));
            }
        }

        if (mpNPipeThread) {
            if (mpNPipeThread->stop(MTRUE) == MFALSE) {
                PIPE_ERR("error: NPipeThread stop fail");
                return MFALSE;
            }
        }
    } else {
        if (this->m_FSM == op_suspend) {
            mCfgLock.unlock();

            PIPE_INF("Blocking stop at suspending, resume first");

            this->resume(0);

            mCfgLock.lock();
        }

        if (mpNPipeThread) {
            if (mpNPipeThread->stop() == MFALSE) {
                PIPE_ERR("error: NPipeThread stop fail");
                return MFALSE;
            }
        }

        if (mpCamIOPipe) {
            PIPE_TRACE_BEGIN("TAG_CAM_STOP");
            ret = mpCamIOPipe->stop(MFALSE, MTRUE);
            PIPE_TRACE_END("TAG_CAM_STOP");

            mpCamIOPipe->sendCommand(EPIPECmd_DEALLOC_UNI, 0, 0, 0);

            if (ret) {
                Mutex::Autolock lock(NPipeCfgGLock);

                switch (_NORMALPIPE_GET_TG_IDX(mpSensorIdx)) {
                case CAM_TG_1:
                    mEnablePath &= ~NPIPE_CAM_A;
                    break;
                case CAM_TG_2:
                    mEnablePath &= ~NPIPE_CAM_B;
                    break;
                case CAM_SV_1:
                    mEnablePath &= ~NPIPE_CAMSV_1;
                    break;
                case CAM_SV_2:
                    mEnablePath &= ~NPIPE_CAMSV_2;
                    break;
                default:
                    PIPE_ERR("unknown TG");
                    break;
                }
                if (0 == (mEnablePath & (NPIPE_CAM_A | NPIPE_CAM_B))) {
                    /* all cam stopped, uni can then change link */
                    mFirstEnableTG = CAM_TG_NONE;
                }
            }
        }
    }

    PIPE_DBG("-");

    this->FSM_UPDATE(op_stop);

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::abort()
{
    IOPipeTrace _traceObj("Normalpipe::abort", IOPIPE_TRACE_EN_NOMALPIPE);
    Mutex::Autolock lock(mCfgLock);

    bool ret = MTRUE;

    if (this->FSM_CHECK(op_stop, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_INF("+ N:%d", mpSensorIdx);

    NormalPipe_Thread::E_ThreadGrop thdGrp;

    switch(this->m_FSM) {
    case op_start:
        thdGrp = (NormalPipe_Thread::E_ThreadGrop)(NormalPipe_Thread::eThdGrp_Internal|NormalPipe_Thread::eThdGrp_External);
        break;
    case op_suspend:
        thdGrp = NormalPipe_Thread::eThdGrp_External;
        break;
    default:
        PIPE_ERR("Unknown state: %d\n", this->m_FSM);
        return MFALSE;;
    }

    /* stop isp hw first */
    if (mpCamIOPipe) {
        if (mpNPipeThread == 0) {
            PIPE_ERR("mpNPipeThread is null");
        } else {
            /* hold idle lock */
            mpNPipeThread->idleStateHold(1, thdGrp);
        }

        PIPE_TRACE_BEGIN("TAG_CAM_STOP");
        ret = mpCamIOPipe->stop(MTRUE, MFALSE);/*force stop*/
        PIPE_TRACE_END("TAG_CAM_STOP");

        if (mpNPipeThread) {
            this->mpNPipeThread->prestop();

            /* release idle lock */
            mpNPipeThread->idleStateHold(0,
                (NormalPipe_Thread::E_ThreadGrop)(NormalPipe_Thread::eThdGrp_Internal|NormalPipe_Thread::eThdGrp_External));
        }
    }

    if (mpNPipeThread) {
        if (mpNPipeThread->stop(MTRUE) == MFALSE) {
            PIPE_ERR("error: NPipeThread stop fail");
            return MFALSE;
        }
    }

    if(this->m_bN3DEn == MTRUE){
        HWSyncDrv* pHwSync = NULL;
        pHwSync = HWSyncDrv::createInstance();
        if(pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, \
                HW_SYNC_BLOCKING_EXE, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,0) != 0){
                PIPE_ERR("N3D stop fail\n");
        }
        pHwSync->destroyInstance();
        this->m_bN3DEn = MFALSE;
    }

    PIPE_INF("- N:%d", mpSensorIdx);

    this->FSM_UPDATE(op_stop);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::init(MBOOL EnableSec)
{
    IOPipeTrace _traceObj("NormalPipe::init", IOPIPE_TRACE_EN_NOMALPIPE);
    MUINT32     i, j;
    MBOOL       ret = MTRUE;

    if (this->FSM_CHECK(op_init, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_DBG("+ SenIdx=%d", mpSensorIdx);

    mpIMem = IMemDrv::createInstance();
    if (mpIMem) {
        ret = mpIMem->init();
    }
    if ((NULL == mpIMem) || (MFALSE == ret)) {
        PIPE_ERR("imem create/init fail %p, %d", mpIMem, ret);
    }

    /*
     * NormalPipe thread init
     */
    if (mpNPipeThread) {
        if (mpNPipeThread->init() == MFALSE) {
            PIPE_ERR("error: NPipeThread init fail\n");
            return MFALSE;
        }
    }

    #ifdef USE_IMAGEBUF_HEAP
    if (!mpFrameMgr) {
        NPIPE_MEM_NEW(mpFrameMgr,FrameMgr(),sizeof(FrameMgr));
    }
    mpFrameMgr->init();
    #endif

    for (i = 0; i < MaxPortIdx; i++) {
        if (!mpEnqueRequest[i]) {
            NPIPE_MEM_NEW(mpEnqueRequest[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>));
            mpEnqueRequest[i]->init();
        }
        if (!mpEnqueReserved[i]) {
            NPIPE_MEM_NEW(mpEnqueReserved[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>));
            mpEnqueReserved[i]->init();
        }
        if (!mpDeQueue[i]) {
            NPIPE_MEM_NEW(mpDeQueue[i], QueueMgr<QBufInfo>(), sizeof(QueueMgr<QBufInfo>));
            mpDeQueue[i]->init();
        }
    }

    PIPE_DBG("-");

    this->FSM_UPDATE(op_init);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::uninit()
{
    IOPipeTrace _traceObj("NormalPipe::uninit", IOPIPE_TRACE_EN_NOMALPIPE);
    MUINT32 i;
    MBOOL   ret = MTRUE;

    if (this->FSM_CHECK(op_uninit, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    PIPE_DBG("+ SenIdx=%d", mpSensorIdx);

    //Mutex::Autolock lock(mCfgLock);
    if (mpNPipeThread) {
        if(mpNPipeThread->uninit() == MFALSE){
            PIPE_ERR("error NPipeThread uninit fail\n");
            return MFALSE;
        }
    }

    if (mpCamIOPipe)  {
        ret = mpCamIOPipe->uninit();
    }

    if (mpCamIOPipe)  {
        Mutex::Autolock lock(NPipeCfgGLock);

        switch (_NORMALPIPE_GET_TG_IDX(mpSensorIdx)) {
        case CAM_TG_1:
            mEnablePath &= ~NPIPE_CAM_A;
            break;
        case CAM_TG_2:
            mEnablePath &= ~NPIPE_CAM_B;
            break;
        case CAM_SV_1:
            mEnablePath &= ~NPIPE_CAMSV_1;
            break;
        case CAM_SV_2:
            mEnablePath &= ~NPIPE_CAMSV_2;
            break;
        default:
            PIPE_ERR("unknown TG");
            break;
        }

        if (0 == (mEnablePath & (NPIPE_CAM_A | NPIPE_CAM_B))) {
            /* all cam stopped, uni can then change link */
            mFirstEnableTG = CAM_TG_NONE;
        }
    }

    //20140305: Middleware(Allan) will control the powerOn/Off life cycle.
    //if (mpHalSensor)  {
    //    mpHalSensor->powerOff("NormalPipe", mpEnabledSensorCnt, mpSensorIdx);
    //}

    #if 0
    for (int i = 0; i < mpEnabledSensorCnt; i++)  {

        SIdx = mpSensorIdx;

        NPipeGLock();
        if (pAllSensorInfo) {
            memset (&pAllSensorInfo->mSenInfo[SIdx].mConfig, 0, sizeof(IHalSensor::ConfigParam));
            pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner = (MUINT32)NULL;
            PIPE_DBG("Reset mSenInfo[%d], sz=%d ", SIdx, sizeof(IHalSensor::ConfigParam));
        }
        else {
            PIPE_ERR("NormalPipe::uninit: pAllSensorInfo NULL");
        }
        NPipeGUnLock();
    }
    #else
    NPipeGLock.lock();
    if (NormalPipe::pAllSensorInfo) {
        NormalPipe::pAllSensorInfo->mSenInfo[mpSensorIdx].mOccupiedOwner = (MUINT32)NULL;
    }
    else {
        PIPE_ERR("error:NormalPipe::uninit: pAllSensorInfo NULL");
    }
    NPipeGLock.unlock();
    #endif

    #ifdef USE_IMAGEBUF_HEAP
    if (mpFrameMgr)  {
        mpFrameMgr->uninit();
        NPIPE_MEM_DEL(mpFrameMgr,sizeof(FrameMgr));
    }
    #endif

    for (i = 0; i < MaxPortIdx; i++) {
        if (mpEnqueRequest[i])  {
            mpEnqueRequest[i]->uninit();
            NPIPE_MEM_DEL(mpEnqueRequest[i],sizeof(QueueMgr<QBufInfo>));
        }
        if (mpEnqueReserved[i])  {
            mpEnqueReserved[i]->uninit();
            NPIPE_MEM_DEL(mpEnqueReserved[i],sizeof(QueueMgr<QBufInfo>));
        }
        if (mpDeQueue[i])  {
            mpDeQueue[i]->uninit();
            NPIPE_MEM_DEL(mpDeQueue[i],sizeof(QueueMgr<QBufInfo>));
        }
        mpImgHeaderMgr[i].uninit();
    }

    PIPE_DBG("-");

    this->FSM_UPDATE(op_uninit);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::enque(QBufInfo const& rQBuf)
{
    IOPipeTrace _traceObj("NormalPipe::enque", IOPIPE_TRACE_EN_NOMALPIPE);
    MBOOL       ret = MTRUE;
    MUINT32     i = 0, _port = 0, PLANE_ID = 0;
    QBufInfo    vPortQBufSlot[MaxPortIdx];
    BufInfo*    pBuf;
    NSImageio::NSIspio::PortID      portID;
    NSImageio::NSIspio::QBufInfo    rQBufInfo;
    Mutex::Autolock lock(mEnQLock);
    char str[256] = {'\0'};
    MUINT32 strLeng = sizeof(str) - 1, totalStrLeng = 0;

    PIPE_DBG("+");

    mEnqContainerLock.lock();

    if (checkEnque(rQBuf) == MFALSE) {
        PIPE_ERR("enque fail\n");
        ret = MFALSE;
        goto _enque_exit;
    }

    for (i = 0; i < rQBuf.mvOut.size(); i++) {
        QBufInfo *pSlot = NULL;

        _port = _PortMap(rQBuf.mvOut.at(i).mPortID.index);

        pSlot = &(vPortQBufSlot[_port]);
        if (pSlot->mvOut.size() == 0) {
            pSlot->mvOut.reserve(mBurstQNum);
        }
        pSlot->mvOut.push_back(rQBuf.mvOut.at(i));
#if 0 //ufeo temp
        if((PortImgFormat[_PortMap(rQBuf.mvOut.at(i).mPortID.index)] == eImgFmt_UFEO_BAYER10)||(PortImgFormat[_PortMap(rQBuf.mvOut.at(i).mPortID.index)] == eImgFmt_UFEO_BAYER12)){
            pSlot = &(vPortQBufSlot[_PortMap(PORT_UFEO.index)]);
            pSlot->mvOut.push_back(rQBuf.mvOut.at(i));
            pSlot->mvOut.at(i).mPortID.index = PORT_UFEO.index;

#endif    }
    }
    for (_port = 0; _port < MaxPortIdx; _port++) { //check enque buf num for each port
        if (vPortQBufSlot[_port].mvOut.size() == 0) {
            continue;
        }
        else if (vPortQBufSlot[_port].mvOut.size() != mBurstQNum) {
            PIPE_ERR("enque buf num not sync: %d/%d", vPortQBufSlot[_port].mvOut.size(), mBurstQNum);
            ret = MFALSE;
            goto _enque_exit;
        }
        vPortQBufSlot[_port].mShutterTimeNs = rQBuf.mShutterTimeNs;
    }

    if (checkDropEnque(vPortQBufSlot) == MFALSE) {
        goto _enque_exit;
    }

    mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(rQBuf.mShutterTimeNs/1000000), 1, 0);
    mpNPipeThread->updateFrameTime((MUINT32)(rQBuf.mShutterTimeNs/1000000), 1);

    for (_port = 0; _port < MaxPortIdx; _port++) {
        if (vPortQBufSlot[_port].mvOut.size() == 0) {
            continue;
        }

        mpEnqueRequest[_port]->sendCmd(eCmd_push, (MUINTPTR)&vPortQBufSlot[_port], 0);
        for (i = 0; i < vPortQBufSlot[_port].mvOut.size(); i++) {
            pBuf = &(vPortQBufSlot[_port].mvOut.at(i));
#if 0//ufeo temp
            if(_port == _PortMap(PORT_UFEO.index)){
				PLANE_ID = 1;
			}
            else
                PLANE_ID = 0;
#endif
            char _tmp[128];
            MUINT32 curLeng = sprintf(_tmp,"dma:x%x pa(0x%x)io(%d_%d_%d_%d_%d_%d)pure(%d)M:x%x, ",
                    pBuf->mPortID.index, pBuf->mBuffer->getBufPA(0),
                    pBuf->FrameBased.mCropRect.p.x,pBuf->FrameBased.mCropRect.p.y,
                    pBuf->FrameBased.mCropRect.s.w,pBuf->FrameBased.mCropRect.s.h,
                    pBuf->FrameBased.mDstSize.w,pBuf->FrameBased.mDstSize.h,
                    pBuf->mRawOutFmt,pBuf->FrameBased.mMagicNum_tuning);
            if(strLeng - totalStrLeng < curLeng){
                PIPE_INF("MyEnq:%d: %s", mpSensorIdx, str);
                str[0] = '\0';
                totalStrLeng = 0;
            }
            strncat(str, _tmp, (sizeof(str) - totalStrLeng));
            totalStrLeng += curLeng;
        }
    }
    /* for log reduction */
    PIPE_INF("MyEnq:%d: %s", mpSensorIdx, str);

    //flush to cmdQ at very first enque before start
    if (this->m_FSM == op_cfg) {
        MUINT32 minDepth = 0xFFFFFFFF, maxDepth = 0, _size = 0, _OpenedPort = 0;

        _OpenedPort = GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);

        for (i = 0; i < _OpenedPort; i++) {
            _port = _PortMap(GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

            mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_getsize, (MUINTPTR)&_size, 0);

            if (minDepth > _size) {
                minDepth = _size;
            }
            if (maxDepth < _size) {
                maxDepth = _size;
            }
            PIPE_DBG("port depth %d, min/max %d/%d", _size, minDepth, maxDepth);
        }
        mReqDepthPreStart = minDepth;

        if ((1 == maxDepth) && (1 == minDepth)) {
            mpNPipeThread->immediateRequest(MFALSE/*MTRUE*/);
        }
    }

_enque_exit:
    mEnqContainerLock.unlock();

    PIPE_DBG("-");

    return ret;
}

/*****************************************************************************
*
******************************************************************************/
#define __FMT(portidx,fmt)({\
    MUINT32 _fmt = fmt;\
    if (portidx == EPortIndex_RRZO) {\
        switch(fmt){\
            case eImgFmt_BAYER8: _fmt = eImgFmt_FG_BAYER8; break;   \
            case eImgFmt_BAYER10: _fmt = eImgFmt_FG_BAYER10; break; \
            case eImgFmt_BAYER12: _fmt = eImgFmt_FG_BAYER12; break; \
            default: _fmt = eImgFmt_FG_BAYER10; break;              \
        }\
    }\
    _fmt;})
MBOOL NormalPipe::checkEnque(QBufInfo const& rQBuf)
{
    MBOOL           ret = MTRUE;
    MUINT32         i = 0, _size;
    ISP_QUERY_RST   queryRst;
    EImageFormat    _ImgFmt;
    E_ISP_PIXMODE   _PixMode;
    E_ISP_QUERY     _op = ISP_QUERY_X_PIX;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_streaming, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (MTRUE == rQBuf.mvOut.empty())  {
        PIPE_ERR("error:queue empty");
        return MFALSE;
    }

    _size = rQBuf.mvOut.size();

    #if 0
    // Must enque all opened ports at the same time
    if (rQBuf.mvOut.size() != (GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum) * mBurstQNum)) {
        PIPE_ERR("error: enabled port number:0x%x, enqued port:0x%x, mismatch",\
                GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum), rQBuf.mvOut.size());
        return MFALSE;
    }
    #else
    if (mBurstQNum && (_size % mBurstQNum)) {
        PIPE_ERR("enque buf cnt is not multiple of mBurstQNum %d/%d", _size, mBurstQNum);
        return MFALSE;
    }
    #endif

    for (i = 0; i < _size; i++) {
        if (0xFFFF != rQBuf.mvOut.at(i).mBufIdx) {
            PIPE_ERR("replace buffer with the same address r not supported, plz make sure mBufIdx=0xffff");
            return MFALSE;
        }
        if (0xFFFF == rQBuf.mvOut.at(i).FrameBased.mSOFidx) {
            PIPE_ERR("buf has invalid sof idx(%x)", rQBuf.mvOut.at(i).FrameBased.mSOFidx);
            return MFALSE;
        }
    }
    //IO checking
    for (i = 0; i < _size; i++) {
        MSize   _tgsize = mTgOut_size[_TGMapping(_NORMALPIPE_GET_TG_IDX(mpSensorIdx))];
        MUINT32 _portIdx = rQBuf.mvOut.at(i).mPortID.index;

        switch (_portIdx) {
        case EPortIndex_IMGO:
        case EPortIndex_RRZO:
        case EPortIndex_CAMSV_IMGO: //TBD
        case EPortIndex_CAMSV2_IMGO: //TBD
            if ((_portIdx == EPortIndex_IMGO) ||
                (_portIdx == EPortIndex_CAMSV_IMGO) || (_portIdx == EPortIndex_CAMSV2_IMGO)) {
                _op = (_tgsize.w != rQBuf.mvOut.at(i).FrameBased.mDstSize.w)? (ISP_QUERY_CROP_X_PIX): (ISP_QUERY_X_PIX);

                if ((_tgsize.w < rQBuf.mvOut.at(i).FrameBased.mDstSize.w) || (_tgsize.h < rQBuf.mvOut.at(i).FrameBased.mDstSize.h)) {
                    PIPE_ERR("error: imgo: out size(0x%x_0x%x) > tg size(0x%x_0x%x)",\
                            rQBuf.mvOut.at(i).FrameBased.mDstSize.w, rQBuf.mvOut.at(i).FrameBased.mDstSize.h, _tgsize.w, _tgsize.h);
                    return MFALSE;
                }
                if ((rQBuf.mvOut.at(i).FrameBased.mDstSize.w > rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w) ||\
                        (rQBuf.mvOut.at(i).FrameBased.mDstSize.h > rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h)) {
                    PIPE_ERR("error: imgo: out size(0x%x_0x%x) > crop size(0x%x_0x%x)",\
                            rQBuf.mvOut.at(i).FrameBased.mDstSize.w, rQBuf.mvOut.at(i).FrameBased.mDstSize.h,\
                            rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w, rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h);
                    return MFALSE;
                }
            }
            else if (_portIdx == EPortIndex_RRZO) {
                _op = ISP_QUERY_X_PIX;

                if ((rQBuf.mvOut.at(i).FrameBased.mDstSize.w == 0) ||
                        (rQBuf.mvOut.at(i).FrameBased.mDstSize.h == 0)) {
                    PIPE_ERR("error: rrz: out size is 0(0x%x_0x%x)",\
                            rQBuf.mvOut.at(i).FrameBased.mDstSize.w, rQBuf.mvOut.at(i).FrameBased.mDstSize.h);
                    return MFALSE;
                }
                if ((_tgsize.w < rQBuf.mvOut.at(i).FrameBased.mDstSize.w) ||
                        (_tgsize.h < rQBuf.mvOut.at(i).FrameBased.mDstSize.h)) {
                    PIPE_ERR("error: rrz: out size(0x%x_0x%x) > tg size(0x%x_0x%x)",\
                            rQBuf.mvOut.at(i).FrameBased.mDstSize.w, rQBuf.mvOut.at(i).FrameBased.mDstSize.h,
                            _tgsize.w, _tgsize.h);
                    return MFALSE;
                }
            }
            if ((rQBuf.mvOut.at(i).FrameBased.mCropRect.p.x + rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w) >
                    _tgsize.w) {
                PIPE_ERR("error: witdh: crop region out of grab window(0x%x+0x%x>0x%x) \n",\
                    rQBuf.mvOut.at(i).FrameBased.mCropRect.p.x, rQBuf.mvOut.at(i).FrameBased.mCropRect.s.w,
                    _tgsize.w);
                return MFALSE;
            }
            if ((rQBuf.mvOut.at(i).FrameBased.mCropRect.p.y + rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h) > _tgsize.h) {
                PIPE_ERR("error: height: crop region out of grab window(0x%x+0x%x>0x%x) \n",\
                    rQBuf.mvOut.at(i).FrameBased.mCropRect.p.y, rQBuf.mvOut.at(i).FrameBased.mCropRect.s.h,
                    _tgsize.h);
                return MFALSE;
            }
            _PixMode = (NSCam::SENSOR_TYPE_YUV == _NORMALPIPE_GET_SENSORTYPE(mpSensorIdx))? ISP_QUERY_2_PIX_MODE:
                        ISP_QUERY_1_PIX_MODE;
            //_ImgFmt = rQBuf.mvOut.at(i).mBuffer->getImgFormat();
            _ImgFmt = (EImageFormat)GetOpenedPortInfo(NormalPipe::_GetPort_ImgFmt, _PortMap(_portIdx));
            _ImgFmt = (EImageFormat)__FMT(_portIdx, _ImgFmt);

            if (ISP_QuerySize(_portIdx, _op, _ImgFmt, rQBuf.mvOut.at(i).FrameBased.mDstSize.w,
                    queryRst, _PixMode) == 0) {
                PIPE_ERR(" fmt(0x%x) | dma(0x%x) err", _ImgFmt, _portIdx);
                return MFALSE;
            }
            else {
                if (rQBuf.mvOut.at(i).FrameBased.mDstSize.w != (MINT32)queryRst.x_pix) {
                    PIPE_ERR("port_%d width r invalid under op:0x%x. cur:0x%x, valid:0x%x",\
                            _portIdx, _op, rQBuf.mvOut.at(i).FrameBased.mDstSize.w, queryRst.x_pix);
                    return MFALSE;
                }
            }
            break;
        case EPortIndex_UFEO:
        case EPortIndex_EISO:
        case EPortIndex_LCSO:
        case EPortIndex_RSSO:
            //TBD
            break;
        default:
            PIPE_ERR("Unsupported port_%d", _portIdx);
            break;
        }
    }
#undef __FMT

    PIPE_DBG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::checkDropEnque(QBufInfo *pQBuf)
{
    MUINT32     _port = 0, i = 0, j = 0, _sof_idx = 0, _size, PLANE_ID = 0;
    QBufInfo    _popQBuf;

    PIPE_DBG("+");

    if (this->m_FSM == op_cfg) {
        return MTRUE;;
    }

    if (MFALSE == m_b1stEnqLoopDone) {
        PIPE_ERR("N:%d static enque loop not done", mpSensorIdx);
        return MFALSE;
    }

    //this->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINTPTR)&_drop_status,0,0);
    sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&_sof_idx, 0, 0);
    #if 0//check _drop_frame_status at EnqueThread
    if(m_PerFrameCtl && ((NormalPipe_FRM_STATUS)_drop_status) == _drop_frame_status){
        PIPE_INF("WARNING:cur frame r drop frame, drop this enque request(pa:0x%x,mag:0x%x)",\
                (MUINT32)rQBuf.mvOut.at(0).mBuffer->getBufPA(0),rQBuf.mvOut.at(0).FrameBased.mMagicNum_tuning);
        if (m_DropCB)
            m_DropCB(rQBuf.mvOut.at(0).FrameBased.mMagicNum_tuning, m_returnCookie);

        mPrvEnqSOFIdx = _sof_idx;
        return MFALSE;
    }
    #endif

    //to chk if enque twice within the the frame
    //drop previous enque request and push this request into enque container
    if (_sof_idx == mPrvEnqSOFIdx) {
        vector<MUINT32> vMnum;
        MBOOL bFirst = 1;
        for (_port = 0; _port < MaxPortIdx; _port++) {
            if (pQBuf[_port].mvOut.size() == 0) {
                continue;
            }
            for (i = 0; i < mBurstQNum; i++) {
                mpEnqueRequest[_port]->sendCmd(eCmd_getsize, (MUINTPTR)&_size, 0);
                if (_size) {
                    mpEnqueRequest[_port]->sendCmd(eCmd_at, _size-1, (MUINTPTR)&_popQBuf);
                    mpEnqueRequest[_port]->sendCmd(eCmd_pop_back, 0, 0);
                    if (bFirst) {
                        MUINT32 j;
                        for (j = 0; j < _popQBuf.mvOut.size(); j++) {
                            vMnum.push_back(_popQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                        }
                    }
                }
                else {
                    PIPE_ERR("error: buf burstNum %d not enought to drop", _size);
                }
            }
            bFirst = 0;
        }
        for (i = 0; i < vMnum.size(); i++) {
            PIPE_INF("WARNING: enque drv > 1 in 1 frame, drop request(mag:0x%x)sof(%d/%d)",\
                vMnum.at(i), mPrvEnqSOFIdx, _sof_idx);
            if (m_DropCB) {
                m_DropCB(vMnum.at(i), m_returnCookie);
            }
        }
    }
    mPrvEnqSOFIdx = _sof_idx;

    // check if input buf alread enqued before
    for (_port = 0; _port < MaxPortIdx; _port++) {
        if (pQBuf[_port].mvOut.size() == 0) {
            continue;
        }

        #if 1//TBD check if buf alread enqued
        mpEnqueRequest[_port]->sendCmd(eCmd_getsize, (MUINTPTR)&_size, 0);
        for (i = 0; i < _size; i++) {
            MUINT32 k;
            mpEnqueRequest[_port]->sendCmd(eCmd_at, i, (MUINTPTR)&_popQBuf);
            for (j = 0; j < _popQBuf.mvOut.size(); j++) {
                for (k = 0; k < pQBuf[_port].mvOut.size(); k++) {
                    if (_NOMRALPIPE_BUF_GETPA(pQBuf[_port].mvOut.at(k),PLANE_ID) ==
                            _NOMRALPIPE_BUF_GETPA(_popQBuf.mvOut.at(j),PLANE_ID)) {
                        PIPE_ERR("error: buf already enqued before: x%x@%d/%d",\
                                _NOMRALPIPE_BUF_GETPA(_popQBuf.mvOut.at(j),PLANE_ID), _port, k);
                        return MFALSE;
                    }
                }
            }
        }
        #endif
    }

    PIPE_DBG("-");

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
#define _NS_PER_SEC     (1000000000)
MBOOL NormalPipe::deque(QPortID& rQPort, QBufInfo& rQBuf, MUINT32 u4TimeoutMs)
{
    IOPipeTrace _traceObj("NormalPipe::deque", IOPIPE_TRACE_EN_NOMALPIPE);
    MBOOL   ret = MTRUE, dequePortsBreak = MFALSE;
    MUINT32 ii = 0, j = 0;
    MINT32  waitStat, queueDepth = 0;
    vector<NSImageio::NSIspio::PortID>      QportID; //used for CamIOPipe
    NSImageio::NSIspio::QTimeStampBufInfo   rQTSBufInfo;
    NSImageio::NSIspio::PortID              portID;    //used for CamIOPipe
    IImageBuffer    *pframe= NULL;
    BufInfo         _buf;
    QBufInfo        _qBuf;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_streaming, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (0 == rQPort.mvPortId.size()) {
        PIPE_ERR("deque port is empty");
        return MFALSE;
    }

    //

    rQBuf.mvOut.clear();
    for (ii = 0 ; ii < rQPort.mvPortId.size() ; ii++) {
        portID.index = rQPort.mvPortId.at(ii).index;
        PIPE_DBG("dma(x%x)", portID.index);

        if (u4TimeoutMs == 0xFFFFFFFF) {
            do {
                waitStat = ::sem_wait(&mpNPipeThread->m_semDeqDonePort[_PortMap(portID.index)]);
                if ((waitStat == -1) && (errno == EINTR)) {
                    PIPE_INF("WARNING: wait again 1!");
                }
            } while ((waitStat == -1) && (errno == EINTR));
        }
        else {
            struct timespec ts;

            if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                PIPE_ERR("wait clock_gettime error. wait with no timeout");
                do {
                    waitStat = ::sem_wait(&mpNPipeThread->m_semDeqDonePort[_PortMap(portID.index)]);
                    if ((waitStat == -1) && (errno == EINTR)) {
                        PIPE_INF("WARNING: wait again 2!");
                    }
                } while ((waitStat == -1) && (errno == EINTR));
            }
            else {
                MUINT32 delta_s = u4TimeoutMs / 1000;
                MUINT32 delta_ns = (u4TimeoutMs - delta_s*1000)*1000000;

                if ((ts.tv_nsec + delta_ns) >= _NS_PER_SEC) { // 2*10^9=0x77359400 < 32bit
                    MUINT32 dds = (delta_ns + ts.tv_nsec) / _NS_PER_SEC;

                    ts.tv_sec = (ts.tv_sec + delta_s + dds);
                    ts.tv_nsec = (ts.tv_nsec + delta_ns) - dds*_NS_PER_SEC;
                }
                else {
                    ts.tv_sec = ts.tv_sec + delta_s;
                    ts.tv_nsec = ts.tv_nsec + delta_ns;
                }

                do {
                    waitStat = sem_timedwait(&mpNPipeThread->m_semDeqDonePort[_PortMap(portID.index)], &ts);
                    if ((waitStat == -1) && (errno == EINTR)) {
                        PIPE_INF("WARNING: wait again 3!");
                    }
                } while ((waitStat == -1) && (errno == EINTR));
            }
        }
        if (0 != waitStat) {
            PIPE_ERR("N:%d dma:x%x pass1 deque fail %d", mpSensorIdx, portID.index, waitStat);
            return MFALSE;
        }

        if (NormalPipe_Thread::_ThreadErrExit == mpNPipeThread->m_DeqThrdState) {
            PIPE_ERR("N:%d dma:x%x deque fail", mpSensorIdx, portID.index);
            return MFALSE;
        }
        else if (NormalPipe_Thread::_ThreadStop == mpNPipeThread->m_DeqThrdState) {
            rQBuf.mvOut.resize(0);
            PIPE_WRN("N:%d dma:x%x deque, already stopped", mpSensorIdx, portID.index);
            return MTRUE;
        }

        mDeqContainerLock.lock();
        mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::eCmd_getsize, (MUINTPTR)&queueDepth, 0);
        if (queueDepth) {
            mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::eCmd_front, (MUINTPTR)&_qBuf, 0);
        }
        else {
            dequePortsBreak = MTRUE;
            _qBuf.mvOut.resize(0);
            rQBuf.mvOut.resize(0);
            PIPE_INF("N:%d dma:x%x break depth %d", mpSensorIdx, portID.index, rQBuf.mvOut.size());
        }
        mDeqContainerLock.unlock();

        for (j = 0; j < _qBuf.mvOut.size(); j++) {
            rQBuf.mvOut.push_back(_qBuf.mvOut.at(j));
        }

        if (_qBuf.mvOut.size()) {
            PIPE_DBG("MyDeq:%d: dma:x%x M:x%x", mpSensorIdx, portID.index, _qBuf.mvOut.at(0).mMetaData.mMagicNum_tuning);
        }
    }

    if (dequePortsBreak == MTRUE) {
        PIPE_INF("MYDEQ:%d not ready for all port,, clear out queue", mpSensorIdx);
        rQBuf.mvOut.resize(0);
    }
    else {
        for (ii = 0 ; ii < rQPort.mvPortId.size() ; ii++) {
            portID.index = rQPort.mvPortId.at(ii).index;

            mDeqContainerLock.lock();
            /* pop from deque container */
            mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::eCmd_getsize, (MUINTPTR)&queueDepth, 0);
            if (queueDepth) {
                mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::eCmd_pop, 0, 0);
            }
            mDeqContainerLock.unlock();
        }
    }

    PIPE_DBG("-");

    return ret;
}

MBOOL NormalPipe::addUser(char const* szUserName)
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
            strlcpy(mUserName[i], szUserName, strlen(szUserName) + 1);
        }
        else {
            PIPE_ERR("User count(%d) reaches maximum!", i);
            return MFALSE;
        }
    }

    mUserCnt[found_idx]++;
    mTotalUserCnt++;

    PIPE_DBG("%s ++, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

MBOOL NormalPipe::delUser(char const* szUserName)
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
        PIPE_ERR("Invalid user name: %s", szUserName);
        return MFALSE;
    }

    mUserCnt[found_idx]--;
    mTotalUserCnt--;

    if (mUserCnt[found_idx] == 0) {
        mUserName[found_idx][0] = '\0';
    }

    PIPE_DBG("%s --, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

MINT32 NormalPipe::getTotalUserNum(void)
{
    MINT32 i = 0, sum = 0;

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            sum++;
        }
    }
    return sum;
}

#define DUMP_SENSOR_INFO()                                 \
do    {                                                    \
        MUINT32 SCnt = 2;                                  \
        _sensorInfo_t  *pSInfo = NULL;                     \
        for (int i = 0; i < 1; i++) {                      \
            pSInfo = &pAllSensorInfo->mSenInfo[i];         \
            PIPE_DBG("[%d]Type=%d, DevId=%d, tg=%d",       \
                     i,                                    \
                     pSInfo->mTypeforMw, pSInfo->mDevId,_NORMALPIPE_GET_TG_IDX( mpSensorIdx ));\
        }\
    } while(0);

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::reset(void)
{
    char const* szCallerName = THIS_NAME;
    MINT32 irqUserKey = -1;

    PIPE_WRN("ESD flow start +");

    if (NULL == mpHalSensor) {
         mpHalSensor = pAllSensorInfo->mSList->createSensor(szCallerName, 1, &mpSensorIdx);
         PIPE_ERR("error: ESD flow error, HalSensor NULL obj");
    }

    PIPE_WRN("Reset+, SenIdx=%d", mpSensorIdx);

    //we need to power off sensor and power on the sensor.
    //Power off
    //avoid reset when HWSYNC is enabled
    if(this->m_bN3DEn){
        HWSyncDrv* pHwSync = NULL;
        pHwSync = HWSyncDrv::createInstance();
        if(pHwSync->sendCommand(HW_SYNC_CMD_DISABLE_SYNC, \
                HW_SYNC_BLOCKING_EXE, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,0) != 0){
                PIPE_ERR("N3D stop fail\n");
        }
        pHwSync->destroyInstance();
    }
    this->mpHalSensor->powerOff(szCallerName, 1, &mpSensorIdx);

    if (1 == this->mBurstQNum) {
        this->mpCamIOPipe->suspend(ICamIOPipe::HW_RST_SUSPEND);
    }

    //Power On
    this->mpHalSensor->powerOn(szCallerName, 1, &mpSensorIdx);
    //Configure the sensor again.
    this->mpHalSensor->configure(1, &m_sensorConfigParam);

    if (1 == this->mBurstQNum) {
        this->mpCamIOPipe->resume(ICamIOPipe::HW_RST_SUSPEND);
    }

    irqUserKey = attach(THIS_NAME);
    if (irqUserKey < 0) {
        PIPE_ERR("attach IRQ failed %d !", irqUserKey);
    }
    else {
        PIPE_WRN("Wait one VSYNC for sensor alive");
        if (wait(EPipeSignal_VSYNC, EPipeSignal_ClearWait, irqUserKey, 5000) == MFALSE) {
            PIPE_ERR("wait VSYNC fail, reset failed!");
        }
        else{
            if(this->m_bN3DEn){
                HWSyncDrv* pHwSync = NULL;
                pHwSync = HWSyncDrv::createInstance();
                if(pHwSync->sendCommand(HW_SYNC_CMD_ENABLE_SYNC, \
                    0,pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,HW_SYNC_BLOCKING_EXE) != 0){
                    pHwSync->destroyInstance();
                    PIPE_ERR("N3D start fail\n");
                    return MFALSE;
                }
                this->m_bN3DEn = MTRUE;
                pHwSync->destroyInstance();
            }
        }
    }

    PIPE_WRN("-");

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::configPipe(QInitParam const& vInPorts, MINT32 burstQnum)
{
    IOPipeTrace     _traceObj("NormalPipe::configpipe", IOPIPE_TRACE_EN_NOMALPIPE);
    MBOOL           dump = 0, ret = MTRUE;
    MUINT32         temp, SIdx, sensorDevIdx, i = 0, j = 0;
    EImageFormat    tgi_eImgFmt[4]; //Temp Used
    ISP_QUERY_RST   QueryRst;
    CAMIO_Func      _func;
    _func.Raw = 0;

    PIPE_DBG("+, SenIdx=%d", mpSensorIdx);

    Mutex::Autolock lock(mCfgLock);

    if (this->FSM_CHECK(op_cfg, __FUNCTION__) == MFALSE) {
        PIPE_ERR("configPipe multiple times, SenIdx=%d", mpSensorIdx);
        return MFALSE;
    }

    if ((burstQnum < 1) || (burstQnum == 2)) {
        PIPE_ERR("BurstQNum %d, should be 1 or > 2, SenIdx=%d", burstQnum, mpSensorIdx);
        return MFALSE;
    }

    ///1 Parameter Check
    if (1 != vInPorts.mSensorCfg.size()) {
        PIPE_ERR("SensorCfgCnt is not 1. (%d)", vInPorts.mSensorCfg.size());
        return MFALSE;
    }

    if (mpSensorIdx != vInPorts.mSensorCfg.at(0).index) {
        PIPE_ERR("SensorIdx is not match. (%d,%d)", mpSensorIdx, vInPorts.mSensorCfg.at(0).index);
        return MFALSE;
    }

    ///2 Get Info
    memset(&m_sensorConfigParam, 0, sizeof(IHalSensor::ConfigParam));

    SIdx = mpSensorIdx;

    if (pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner) {
        PIPE_INF("WARNING: Overwrite SenIdx=%d-SensorCfg...", SIdx);
    }
    pAllSensorInfo->mSenInfo[SIdx].mConfig = vInPorts.mSensorCfg.at(0);
    pAllSensorInfo->mSenInfo[SIdx].mOccupiedOwner = (MUINTPTR)this;

    m_sensorConfigParam = vInPorts.mSensorCfg.at(0); //used for sensorCfg

    _NORMALPIPE_GET_TGIFMT(SIdx, tgi_eImgFmt[SIdx]);

    #if 0 //unused
    //enforce seninf/tg in 2-pixmode if rrzo over RRZ_LIMITATION is required
    for (j = 0; j < vInPorts.mPortInfo.size(); j++) {
        if (PORT_RRZO.index == vInPorts.mPortInfo.at(j).mPortID.index) {
            #if 1
            if (vInPorts.mPortInfo.at(j).mDstSize.w > CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL) {
                this->m_sensorConfigParam.twopixelOn = MTRUE;
                break;
            }
            #endif
        }
    }
    #endif

    if (!mpHalSensor) {
         mpHalSensor = pAllSensorInfo->mSList->createSensor(THIS_NAME, 1, &mpSensorIdx);
    } else {
        PIPE_ERR("ERROR: ConfigPipe multiple times...");
    }
    if (!mpHalSensor) {
        PIPE_ERR("mpHalSensor Fail");
        return MFALSE;
    }

    //20140305: Middleware(Allan) will control the powerOn/Off life cycle.
    //mpHalSensor->powerOn(THIS_NAME, mpEnabledSensorCnt, mpSensorIdx);
    PIPE_TRACE_BEGIN("TAG_SENSOR_CONFIG");
    mpHalSensor->configure(1, &m_sensorConfigParam);
    PIPE_TRACE_END("TAG_SENSOR_CONFIG");

    if (vInPorts.mRawType) {
        PIPE_INF("Enable sensor test pattern SenIdx=%d DevId_x%x", SIdx, pAllSensorInfo->mSenInfo[SIdx].mDevId);
        mpHalSensor->sendCommand(pAllSensorInfo->mSenInfo[SIdx].mDevId, SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                    (MUINTPTR)&vInPorts.mRawType, 0, 0);
    }

    mpHalSensor->querySensorDynamicInfo(pAllSensorInfo->mSenInfo[SIdx].mDevId, &pAllSensorInfo->mSenInfo[SIdx].mDInfo);
    if (_NORMALPIPE_GET_TG_IDX(SIdx) <= 0){
        PIPE_ERR("ERROR: mpDynamicInfo.TgInfo <= 0,SIdx=%d,mDevId(0x%x),tg_idx(%d)", \
                  SIdx, pAllSensorInfo->mSenInfo[SIdx].mDevId, _NORMALPIPE_GET_TG_IDX(SIdx));
        return MFALSE;
    }

    if (!mpCamIOPipe) {
        vector<PortInfo const*> vCamIOInPorts;
        vector<PortInfo const*> vCamIOOutPorts;
        NSImageio::NSIspio::E_ISP_PIXMODE eIspPxlModeQuery;
        PortInfo    tgi, imgo, rrzo, ufeo, eiso, lcso, camsv_imgo, camsv2_imgo, rsso;
        PortInfo    *pOut = NULL;
        PortInfo    *pOut_table = NULL;
        MBOOL       enable2PixelTwin = MFALSE, _firstLink = MFALSE;
        MUINT32     tg = 0, _path = 0, imgHdrBufNum = 0, inputFps = 0;
        MUINTPTR    imgHdrAllocOfst = 0;

        Mutex::Autolock lock(NPipeCfgGLock); // for protect twin mode check

        mpCamIOPipe = ICamIOPipe::createInstance((MINT8*)mpName,
                (_NORMALPIPE_GET_TG_IDX(mpSensorIdx) == CAM_TG_1)? NSImageio::NSIspio::ICamIOPipe::TG_A:
                        NSImageio::NSIspio::ICamIOPipe::TG_B, NSImageio::NSIspio::ICamIOPipe::CAMIO);
        if (!mpCamIOPipe) {
            PIPE_ERR("CamIOPipe createInstance fail");
            return MFALSE;
        }

        if (MFALSE == mpCamIOPipe->init()) {
             PIPE_ERR("CamIOPipe init fail");
             return MFALSE;
        }

        //get IMGO RawType, check it is pure raw of not
        //mxxxxOut_size is for _setTgPixelAndDualMode(), check IMGO only
        for (j = 0; j < vInPorts.mPortInfo.size(); j++) {
            if (PORT_RRZO == vInPorts.mPortInfo.at(j).mPortID) {
                mRrzoOut_size = vInPorts.mPortInfo.at(j).mDstSize;
            }
            if (PORT_IMGO == vInPorts.mPortInfo.at(j).mPortID) {
                mPureRaw = vInPorts.mPortInfo.at(j).mPureRaw;
                mImgoOut_size = vInPorts.mPortInfo.at(j).mDstSize;
            }
        }
        mDynSwtRawType = vInPorts.m_DynamicRawType;

        tg = _NORMALPIPE_GET_TG_IDX(mpSensorIdx);
        tgi.index        = _NOMRALPIPE_MAP_TG_IDX(mpSensorIdx);
        tgi.ePxlMode     = _NORMALPIPE_MAP_TG_PXLMODE(mpSensorIdx);//TBD check idx mapping in sensor drv
        tgi.eImgFmt      = tgi_eImgFmt[mpSensorIdx];
        _NORMALPIPE_GET_TGI_PIX_ID(mpSensorIdx, tgi.eRawPxlID);
        tgi.u4ImgWidth   = _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx);
        tgi.u4ImgHeight  = _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
        _NORMALPIPE_GET_TG_GRAB_START(mpSensorIdx, m_sensorConfigParam.scenarioId, tgi.crop1.x, tgi.crop1.y);
        tgi.crop1.floatX = 0;
        tgi.crop1.floatY = 0;
        tgi.crop1.w      = tgi.u4ImgWidth;
        tgi.crop1.h      = tgi.u4ImgHeight;
        tgi.type         = EPortType_Sensor;
        tgi.inout        = EPortDirection_In;
        tgi.tgFps        = vInPorts.mSensorCfg.at(0).framerate;
        inputFps = tgi.tgFps;
        #if 0 //use per-frame timeout
        if ((tgi.tgFps > 10) && (tgi.tgFps < 60)) {
            tgi.tgFps = 10;
        }
        #endif

        tgi.tTimeClk     = _NORMALPIPE_GET_CLK_FREQ(SIdx)/100; //0.1MHz <- KHz
        vCamIOInPorts.push_back(&tgi);
        mTgOut_size[_TGMapping(tg)].w = tgi.u4ImgWidth;
        mTgOut_size[_TGMapping(tg)].h = tgi.u4ImgHeight;

        PIPE_INF("N:%d TG=%d Scen=%d tgFmt=x%x devID=%d eRawPxlID=%d PixelMode=%d W/H=[%d,%d] fps=%d/%d Clk=%d khz burst=%d",
             mpSensorIdx, tg, m_sensorConfigParam.scenarioId, tgi.eImgFmt, pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,
             tgi.eRawPxlID, tgi.ePxlMode, tgi.u4ImgWidth, tgi.u4ImgHeight, tgi.tgFps, inputFps, tgi.tTimeClk*100, burstQnum);

        for (i = 0; i < vInPorts.mPortInfo.size(); i++) {
            MBOOL bPushCamIO = MFALSE;

            switch (vInPorts.mPortInfo.at(i).mPortID.index) {
            case EPortIndex_IMGO:
                pOut = &imgo;
                mImgoOut_size   = vInPorts.mPortInfo.at(i).mDstSize;
                mPureRaw        = vInPorts.mPortInfo.at(i).mPureRaw;

                mOpenedPort |= NormalPipe::_IMGO_ENABLE;

                bPushCamIO = MTRUE;
                break;
            case EPortIndex_RRZO:
                pOut = &rrzo;
                mRrzoOut_size = vInPorts.mPortInfo.at(i).mDstSize;

                mOpenedPort |= NormalPipe::_RRZO_ENABLE;
                if((vInPorts.mPortInfo.at(i).mFmt == eImgFmt_UFEO_BAYER10)||(vInPorts.mPortInfo.at(i).mFmt == eImgFmt_UFEO_BAYER12)){
                    pOut_table = &ufeo;
                    mOpenedPort |= NormalPipe::_UFEO_ENABLE;
                }

                bPushCamIO = MTRUE;
                break;
            case EPortIndex_EISO:
                pOut = &eiso;

                mOpenedPort |= NormalPipe::_EISO_ENABLE;
                break;
            case EPortIndex_LCSO:
                pOut = &lcso;

                mOpenedPort |= NormalPipe::_LCSO_ENABLE;
                break;
            case EPortIndex_RSSO:
                pOut = &rsso;

                mOpenedPort |= NormalPipe::_RSSO_ENABLE;
                break;
            default:
                PIPE_ERR("unknown port id %d", vInPorts.mPortInfo.at(i).mPortID.index);
                break;
            }

            if ((GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum) > MaxPortIdx)
                    || (GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum) == 0)) {
                PIPE_ERR("dmao config error,opend port num:0x%x(max:0x%x)\n",\
                        GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum),MaxPortIdx);
                return MFALSE;
            }

            pOut->index         = vInPorts.mPortInfo.at(i).mPortID.index;
            pOut->eImgFmt       = vInPorts.mPortInfo.at(i).mFmt;
            pOut->u4PureRaw     = vInPorts.mPortInfo.at(i).mPureRaw;
            pOut->u4PureRawPak  = vInPorts.mPortInfo.at(i).mPureRawPak;
            pOut->u4ImgWidth    = vInPorts.mPortInfo.at(i).mDstSize.w;
            pOut->u4ImgHeight   = vInPorts.mPortInfo.at(i).mDstSize.h;
            #if 1
            pOut->crop1.x       = 0;
            pOut->crop1.y       = 0;
            pOut->crop1.floatX  = 0;
            pOut->crop1.floatY  = 0;
            pOut->crop1.w       = tgi.u4ImgWidth;
            pOut->crop1.h       = tgi.u4ImgHeight;
            #else
            pOut->crop1.x       = vInPorts.mPortInfo.at(i).mCropRect.p.x;
            pOut->crop1.y       = vInPorts.mPortInfo.at(i).mCropRect.p.y;
            pOut->crop1.floatX  = 0;
            pOut->crop1.floatY  = 0;
            pOut->crop1.w       = vInPorts.mPortInfo.at(i).mCropRect.s.w;
            pOut->crop1.h       = vInPorts.mPortInfo.at(i).mCropRect.s.h;
            #endif

            pOut->ePxlMode      = _NORMALPIPE_MAP_TG_PXLMODE(mpSensorIdx);

            //UFEO
            if((vInPorts.mPortInfo.at(i).mFmt == eImgFmt_UFEO_BAYER10)||(vInPorts.mPortInfo.at(i).mFmt == eImgFmt_UFEO_BAYER12)){
                if(pOut_table){
                    pOut_table->index         = EPortIndex_UFEO;
                    pOut_table->eImgFmt       = vInPorts.mPortInfo.at(i).mFmt;
                    pOut_table->u4PureRaw     = vInPorts.mPortInfo.at(i).mPureRaw;
                    pOut_table->u4PureRawPak  = vInPorts.mPortInfo.at(i).mPureRawPak;
                    pOut_table->u4ImgWidth    = vInPorts.mPortInfo.at(i).mDstSize.w;
                    pOut_table->u4ImgHeight   = vInPorts.mPortInfo.at(i).mDstSize.h;
#if 1
                    pOut_table->crop1.x       = 0;
                    pOut_table->crop1.y       = 0;
                    pOut_table->crop1.floatX  = 0;
                    pOut_table->crop1.floatY  = 0;
                    pOut_table->crop1.w       = tgi.u4ImgWidth;
                    pOut_table->crop1.h       = tgi.u4ImgHeight;
                    pOut_table->ePxlMode      = _NORMALPIPE_MAP_TG_PXLMODE(mpSensorIdx);
#endif
                }
            }

            if (pOut->index == EPortIndex_RRZO) {
                NSImageio::NSIspio::EPxlMode pxlMode = _NORMALPIPE_MAP_TG_PXLMODE(mpSensorIdx);
                if (ISP_QuerySize(pOut->index, ISP_QUERY_X_PIX, pOut->eImgFmt, pOut->u4ImgWidth,\
                        QueryRst, _NORMALPIPE_GET_PXLMODE_QUERY(mpSensorIdx, pxlMode)) == 0) {
                    PIPE_ERR(" fmt(0x%x) | dma(0x%x) err", pOut->eImgFmt, pOut->index);
                    return MFALSE;
                }
                else {
                    if (QueryRst.x_pix != pOut->u4ImgWidth) {
                        PIPE_ERR("port:0x%x width r invalid (cur:%d valid:%d)", pOut->index,\
                                pOut->u4ImgWidth, QueryRst.x_pix);
                        return MFALSE;
                    }
                }

                if((pOut->eImgFmt == eImgFmt_UFEO_BAYER10)||(pOut->eImgFmt == eImgFmt_UFEO_BAYER12)){
                    if(pOut_table){
                        if (ISP_QuerySize(pOut_table->index, ISP_QUERY_X_PIX, pOut_table->eImgFmt, pOut_table->u4ImgWidth,\
                                QueryRst, _NORMALPIPE_GET_PXLMODE_QUERY(mpSensorIdx, pxlMode)) == 0) {
                            PIPE_ERR(" fmt(0x%x) | dma(0x%x) err", pOut_table->eImgFmt, pOut_table->index);
                            return MFALSE;
                        }
                        else {
                            if (QueryRst.x_pix != pOut_table->u4ImgWidth) {
                                PIPE_ERR("port:0x%x width r invalid (cur:%d valid:%d)", pOut_table->index,\
                                        pOut_table->u4ImgWidth, QueryRst.x_pix);
                                return MFALSE;
                            }
                        }
                    }
                }
            }
            if (pOut->index == EPortIndex_IMGO) {
                E_ISP_QUERY _op;

                NSImageio::NSIspio::EPxlMode pxlMode = _NORMALPIPE_MAP_TG_PXLMODE(mpSensorIdx);
                if (pOut->u4ImgWidth != ((MUINT32)mTgOut_size[_TGMapping(tg)].w))
                    _op = ISP_QUERY_CROP_X_PIX;
                else
                    _op = ISP_QUERY_X_PIX;

                eIspPxlModeQuery = (NSCam::SENSOR_TYPE_YUV == _NORMALPIPE_GET_SENSORTYPE(mpSensorIdx))?
                                        ISP_QUERY_2_PIX_MODE: _NORMALPIPE_GET_PXLMODE_QUERY(mpSensorIdx, pxlMode);
                if (ISP_QuerySize(pOut->index, _op, pOut->eImgFmt, pOut->u4ImgWidth, QueryRst, eIspPxlModeQuery) == 0) {
                    PIPE_ERR(" fmt(0x%x) | dma(0x%x) err\n",pOut->eImgFmt,pOut->index);
                    return MFALSE;
                }
                else {
                    if (QueryRst.x_pix != pOut->u4ImgWidth) {
                        PIPE_ERR("port:0x%x width r invalid (cur:%d valid:%d)\n",pOut->index,pOut->u4ImgWidth,QueryRst.x_pix);
                        return MFALSE;
                    }
                }
            }
            #if 0 //TBD statistic port : eiso/lcso
            #endif

            pOut->u4Stride[ESTRIDE_1ST_PLANE] = vInPorts.mPortInfo.at(i).mStride[0];

            //UFEO
            if((pOut->eImgFmt == eImgFmt_UFEO_BAYER10)||(pOut->eImgFmt == eImgFmt_UFEO_BAYER12)){
                if(pOut_table){
                    pOut_table->u4Stride[ESTRIDE_1ST_PLANE] = vInPorts.mPortInfo.at(i).mStride[0];
                }
            }
            if ((pOut->index == EPortIndex_RRZO) || (pOut->index == EPortIndex_IMGO)) {
                if (ISP_QuerySize(pOut->index, ISP_QUERY_STRIDE_BYTE, pOut->eImgFmt, pOut->u4ImgWidth, QueryRst) == 0) {
                    PIPE_ERR(" fmt(0x%x) | dma(0x%x) err\n",pOut->eImgFmt,pOut->index);
                    return MFALSE;
                }
                else {
                    if (QueryRst.stride_byte != pOut->u4Stride[ESTRIDE_1ST_PLANE]) {
                        PIPE_DBG("cur dma(%d) stride:0x%x, recommended:0x%x",pOut->index,pOut->u4Stride[ESTRIDE_1ST_PLANE],QueryRst.stride_byte);
                    }
                }
            }

            if((pOut->eImgFmt == eImgFmt_UFEO_BAYER10)||(pOut->eImgFmt == eImgFmt_UFEO_BAYER12)){
                if(pOut_table){
                    if (ISP_QuerySize(pOut_table->index, ISP_QUERY_STRIDE_BYTE, pOut_table->eImgFmt, pOut_table->u4ImgWidth, QueryRst) == 0) {
                        PIPE_ERR(" fmt(0x%x) | dma(0x%x) err\n",pOut_table->eImgFmt,pOut_table->index);
                        return MFALSE;
                    }
                    else {
                        if (QueryRst.stride_byte != pOut_table->u4Stride[ESTRIDE_1ST_PLANE]) {
                            PIPE_INF("cur dma(%d) stride:0x%x, recommended:0x%x",pOut_table->index,pOut_table->u4Stride[ESTRIDE_1ST_PLANE],QueryRst.stride_byte);
                        }
                    }
                }
            }

            pOut->u4Stride[ESTRIDE_2ND_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
            pOut->u4Stride[ESTRIDE_3RD_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
            pOut->type   = EPortType_Memory;
            pOut->inout  = EPortDirection_Out;

            PortImgFormat[_PortMap(pOut->index)] = pOut->eImgFmt; //for GetOpenedPortInfo()

            PIPE_DBG("OutPorts[%d] Crop=[%d,%d,%d,%d], sz=(%d,%d), Stride=%d, p(%d)",\
                    pOut->index, pOut->crop1.x, pOut->crop1.y, pOut->crop1.w, pOut->crop1.h,\
                    pOut->u4ImgWidth, pOut->u4ImgHeight,pOut->u4Stride[0], bPushCamIO);
            if (bPushCamIO) {
                vCamIOOutPorts.push_back(pOut);
            }
            //UFEO
            if((pOut->eImgFmt == eImgFmt_UFEO_BAYER10)||(pOut->eImgFmt == eImgFmt_UFEO_BAYER12)){
                if(pOut_table){
                    pOut_table->u4Stride[ESTRIDE_2ND_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[1];
                    pOut_table->u4Stride[ESTRIDE_3RD_PLANE] = 0;//vInPorts.mPortInfo.at(i).mStride[2];
                    pOut_table->type   = EPortType_Memory;
                    pOut_table->inout  = EPortDirection_Out;
                    PortImgFormat[_PortMap(pOut_table->index)] = pOut_table->eImgFmt; //for GetOpenedPortInfo()
                    PIPE_INF("OutPorts[%d] Crop=[%d,%d,%d,%d], sz=(%d,%d), Stride=%d, p(%d)",\
                        pOut_table->index, pOut_table->crop1.x, pOut_table->crop1.y, pOut_table->crop1.w, pOut_table->crop1.h,\
                        pOut_table->u4ImgWidth, pOut_table->u4ImgHeight,pOut_table->u4Stride[0], bPushCamIO);

                    if (bPushCamIO) {
                        vCamIOOutPorts.push_back(pOut_table);
                    }
                }
            }
        }

        //DUMP_SENSOR_INFO();
        switch (_NORMALPIPE_GET_TG_IDX((mpSensorIdx))) {
        case CAM_TG_1:
        case CAM_TG_2:
            if (vInPorts.m_UniLinkSel == E_UNI_LINK_AUTO) {
                if (!(mEnablePath & (NPIPE_CAM_A|NPIPE_CAM_B))) {
                    _firstLink = MTRUE;
                }
                if (_NORMALPIPE_GET_TG_IDX(mpSensorIdx) == mFirstEnableTG) {
                    _firstLink = MTRUE;
                }
            }
            else {
                if (vInPorts.m_UniLinkSel == E_UNI_LINK_ON) {
                    _firstLink = MTRUE;
                }
                else if (vInPorts.m_UniLinkSel == E_UNI_LINK_OFF) {
                    _firstLink = MFALSE;
                }
            }

            if (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx)) {
                _path = NPIPE_CAM_A;
            }
            else {
                _path = NPIPE_CAM_B;
            }
            break;
        default:
            PIPE_ERR("unkown TG %d", _NORMALPIPE_GET_TG_IDX((mpSensorIdx)));
            return MFALSE;
        }

        PIPE_TRACE_BEGIN("TAG_CAM_CONFIG");
        //tempory code, wait for me15 remove m_bOffBin
        _func.Bits.bin_off = vInPorts.m_bOffBin;
        if(vInPorts.m_Func.Bits.OFF_BIN)
            _func.Bits.bin_off = vInPorts.m_Func.Bits.OFF_BIN;

        _func.Bits.SUBSAMPLE = burstQnum-1;
        switch(vInPorts.m_Func.Bits.DATA_PATTERN){
            case EPipe_Dual_pix:
                _func.Bits.DATA_PATTERN = ECamio_Dual_pix;
                break;
            case EPipe_Normal:
            default:
                _func.Bits.DATA_PATTERN = ECamio_Normal;
                break;
        }
        if (MFALSE == mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts, _func)) {
            PIPE_ERR("CamIOPipe configPipe Fail");
            return MFALSE;
        }
        PIPE_TRACE_END("TAG_CAM_CONFIG");
        if(vInPorts.m_bN3D == MTRUE){
            HWSyncDrv* pHwSync = NULL;
            pHwSync = HWSyncDrv::createInstance();
            if(pHwSync->sendCommand(HW_SYNC_CMD_ENABLE_SYNC, \
                0,pAllSensorInfo->mSenInfo[mpSensorIdx].mDevId,HW_SYNC_BLOCKING_EXE) != 0){
                pHwSync->destroyInstance();
                PIPE_ERR("N3D start fail\n");
                return MFALSE;
            }
            this->m_bN3DEn = MTRUE;
            pHwSync->destroyInstance();
        }
        else
            this->m_bN3DEn = MFALSE;
        mEnablePath     |= _path;
        mBurstQNum      = burstQnum;
        m_DropCB        = vInPorts.m_DropCB;
        m_returnCookie  = vInPorts.m_returnCookie;

        if (_firstLink) {
            PIPE_INF("EnablePath: x%x %s-link UNI [%s]", mEnablePath,
                ((CAM_TG_NONE == mFirstEnableTG)? "1st": "re"),
                ((vInPorts.m_UniLinkSel == E_UNI_LINK_AUTO)? "auto": "manual"));
        }
        else {
            PIPE_INF("EnablePath: x%x no UNI [%s]", mEnablePath,
                ((vInPorts.m_UniLinkSel == E_UNI_LINK_AUTO)? "auto": "manual"));
        }

        if (_firstLink) {
            mFirstEnableTG = _NORMALPIPE_GET_TG_IDX((mpSensorIdx));

            mpCamIOPipe->sendCommand(EPIPECmd_ALLOC_UNI, 0, 0, 0);
        }

        /*
         * Image Header buffer allocation
         */
        imgHdrBufNum = IMG_HEADER_BUF_NUM * mBurstQNum;
        mImgHeaderPool.size = (IMG_HEADER_SIZE * imgHdrBufNum * MaxPortIdx) + 8; //+8 for 64bit alignment
        mImgHeaderPool.useNoncache = 1;
        if (mpIMem->allocVirtBuf(&mImgHeaderPool) < 0) {
            PIPE_ERR("imgHeader alloc %d fail", mImgHeaderPool.size);
            return MFALSE;
        }
        if (mpIMem->mapPhyAddr(&mImgHeaderPool) < 0) {
            PIPE_ERR("imgHeader map %d fail", mImgHeaderPool.size);
            return MFALSE;
        }

        /*
         * image header buffer to each port
         */
        imgHdrAllocOfst = ((8 - (mImgHeaderPool.virtAddr & 7)) & 7); //align 8
        for (i = 0; i < MaxPortIdx; i++) {
            mpImgHeaderMgr[i].init();

            for (j = 0; j < imgHdrBufNum; j++) {
                IMEM_BUF_INFO iHeader(IMG_HEADER_SIZE, mImgHeaderPool.memID, (mImgHeaderPool.virtAddr + imgHdrAllocOfst),
                        (mImgHeaderPool.phyAddr + imgHdrAllocOfst), mImgHeaderPool.bufSecu, mImgHeaderPool.bufCohe,
                        mImgHeaderPool.useNoncache);
                mpImgHeaderMgr[i].sendCmd(eCmd_push, (MUINTPTR)&iHeader, 0);
                imgHdrAllocOfst += IMG_HEADER_SIZE;
            }
        }

        if (mOpenedPort == 0) {
            PIPE_ERR("error:no dmao enabled\n");
            return MFALSE;
        }
        PIPE_DBG("DropCB(0x%x)/Cookie(0x%x) DynamicRawType(%d) PureRaw(%d)", m_DropCB, m_returnCookie,
                mDynSwtRawType, mPureRaw);

        mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)0, 0, 0);
        mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)0, 1, 0);
        mpNPipeThread->updateFrameTime(0, 0);
        mpNPipeThread->updateFrameTime(0, 1);

        // NPipeCfgGLock auto unlocked
    }
    else {
        PIPE_ERR("CamioPipe is not NULL for 1st cfg. Remained user(%d)", mTotalUserCnt);
        for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
            if (mUserCnt[i] > 0)
                PIPE_ERR("[%d]%s ",i, mUserName[i]);
        }
    }

    #if 0 //TBD
    {
        CamManager* pCamMgr = CamManager::getInstance();
        pCamMgr->setAvailableHint((0 == this->mbTwinEnable)? MTRUE : MFALSE);
    }
    #endif

EXIT:
    this->FSM_UPDATE(op_cfg);

    PIPE_DBG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32 NormalPipe::attach(const char* UserName)
{
    MINT32 user_key = 0;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return -1;
    }

    if (mpCamIOPipe) {
        user_key = mpCamIOPipe->registerIrq((MINT8*)UserName);
        if (user_key >= 0) {
            return user_key;
        }
        PIPE_ERR("registerIrq(%s, %d) fail", UserName, user_key);
    }
    else {
        PIPE_ERR("CamIOPipe not create");
    }

    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                        MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    Irq_t irq;
    MBOOL ret = MTRUE;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    irq.Type = (eClear == EPipeSignal_ClearNone)? Irq_t::_CLEAR_NONE: Irq_t::_CLEAR_WAIT;
    ret = _NOMRALPIPE_MAP_SIGNAL(eSignal, irq.Status, irq.StatusType);
    if (!ret) {
        PIPE_ERR("Signal x%x not support", eSignal);
        return ret;
    }
    irq.UserKey = mUserKey;
    irq.Timeout = TimeoutMs; //ms

    if (mpCamIOPipe) {
        ret = mpCamIOPipe->irq(&irq);
        if (!ret) {
            PIPE_ERR("Wait Signal Fail(%d,%d,x%x,%d)", irq.UserKey, irq.Type, irq.Status, irq.Timeout);
            return ret;
        }
        else {
            if (pTime) {
                // fill time info in irq
                pTime->tLastSig_sec = irq.TimeInfo.tLastSig_sec;
                pTime->tLastSig_usec = irq.TimeInfo.tLastSig_usec;
                pTime->tMark2WaitSig_sec = irq.TimeInfo.tMark2WaitSig_sec;
                pTime->tMark2WaitSig_usec = irq.TimeInfo.tMark2WaitSig_usec;
                pTime->tLastSig2GetSig_sec = irq.TimeInfo.tLastSig2GetSig_sec;
                pTime->tLastSig2GetSig_usec = irq.TimeInfo.tLastSig2GetSig_usec;
                pTime->passedbySigcnt = irq.TimeInfo.passedbySigcnt;
            }
        }
    }
    else {
        PIPE_ERR("CamIOPipe not create");
    }

    PIPE_DBG("-");

    return ret;
}

MBOOL NormalPipe::signal(EPipeSignal eSignal, const MINT32 mUserKey)
{
    Irq_t irq;
    MBOOL ret = MTRUE;

    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    ret = _NOMRALPIPE_MAP_SIGNAL(eSignal, irq.Status, irq.StatusType);
    if (!ret) {
        PIPE_ERR("Signal x%x not support", eSignal);
        return ret;
    }

    if(mUserKey < 1){
        PIPE_ERR("invalid userkey(%d)", mUserKey);
        return ret;
    }

    irq.UserKey = mUserKey;

    if (mpCamIOPipe) {
        ret = mpCamIOPipe->signalIrq(irq);
        if (!ret) {
            PIPE_ERR("Signal x%x/x%x fail", eSignal, irq.Status);
            return ret;
        }
    }
    else {
        PIPE_ERR("CamIOPipe not create");
    }

    PIPE_DBG("-");

    return ret;
}

MBOOL NormalPipe::suspend(void)
{
    Mutex::Autolock lock(mCfgLock);
    MBOOL ret = MTRUE;
    MUINT32 _OpenedPort = this->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum), i;

    if (this->FSM_CHECK(op_suspend, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }
    PIPE_INF("N:%d Holding idle locks before suspend", this->mpSensorIdx);

    this->mpNPipeThread->idleStateHold(1, NormalPipe_Thread::eThdGrp_Internal);
    /* Not holding stt locks for 3a threads to error return when suspend
     * NormalPipe enq/deq threads will blocking at mutex lock
     */

    ret = this->mpCamIOPipe->suspend(ICamIOPipe::SW_SUSPEND);

    PIPE_INF("N:%d DevId:0x%x p1 suspend done, flushing sof signal",
        this->mpSensorIdx, _NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx));

    this->signal(EPipeSignal_SOF, mpNPipeThread->m_irqUserKey); //flush SOF signal, since irqUserKey is here

    for (i = 0; i < _OpenedPort; i++) {
        ::sem_post(&this->mpNPipeThread->m_semDeqDonePort[_PortMap(this->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i))]);
    }

    this->mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                    SENSOR_CMD_SET_STREAMING_SUSPEND, 0,0,0);

    this->FSM_UPDATE(op_suspend);

    return ret;
}


MBOOL NormalPipe::resume(MUINT64 shutterTimeUs)
{
    Mutex::Autolock lock(mCfgLock);
    MBOOL ret = MTRUE;
    MUINT32 _shuttertime_us = (MUINT32)shutterTimeUs;

    if (this->FSM_CHECK(op_start, __FUNCTION__) == MFALSE) {
        PIPE_ERR("N:%d suspend can only be trigger under streaming", this->mpSensorIdx);
        return MFALSE;
    }

    PIPE_INF("N:%d DevId:0x%x Release idle locks and resume shutter_us:%d",
        this->mpSensorIdx, _NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx), _shuttertime_us);

    this->mpHalSensor->sendCommand(_NORMALPIPE_GET_SENSOR_DEV_ID(this->mpSensorIdx),
                                    SENSOR_CMD_SET_STREAMING_RESUME, (MUINTPTR)&_shuttertime_us, 0, 0);

    ret = this->mpCamIOPipe->resume(ICamIOPipe::SW_SUSPEND);

    this->FSM_UPDATE(op_start);

    this->mpNPipeThread->idleStateHold(0, NormalPipe_Thread::eThdGrp_Internal);

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::abortDma(PortID port, char const* szCallerName)
{
    MBOOL ret = MTRUE;
    NSImageio::NSIspio::PortID portID;

    PIPE_DBG("+, usr[%s]dma[0x%x]SenIdx[%d]", szCallerName, port.index, mpSensorIdx);

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (mpCamIOPipe) {
        switch (port.index){
        case EPortIndex_RRZO:
        case EPortIndex_IMGO:
        case EPortIndex_LCSO:
        case EPortIndex_EISO:
        case EPortIndex_RSSO:
            break;
        default:
            PIPE_ERR("not support dma(0x%x)user(%s)\n", port.index, szCallerName);
            return MFALSE;
        }
        portID.index = port.index;
        ret = mpCamIOPipe->abortDma(portID);
        if (!ret) {
            PIPE_ERR("abort dma(0x%x) fail", port.index);
            return ret;
        }
    }
    else {
        PIPE_ERR("CamIOPipe not create, user(%s)", szCallerName);
    }

    PIPE_DBG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (NULL == RegData) {
        PIPE_ERR("Wrong out buf");
        return MFALSE;
    }

    if (mpCamIOPipe) {
        MUINT32 i = 0;

        for (i = 0; i < RegCount; i++) {
            RegData[i] = mpCamIOPipe->ReadReg(RegAddr + (i << 2), bPhysical);
        }
    }
    else {
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (NULL == pRegs) {
        PIPE_ERR("NULL param");
        return MFALSE;
    }

    if (mpCamIOPipe) {
        MUINT32 i = 0;

        for (i = 0; i < RegCount; i++) {
            pRegs[i].Data = mpCamIOPipe->ReadReg(pRegs[i].Addr, bPhysical);
        }
    }
    else {
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::getIspUniReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_DBG("+");

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (NULL == pRegs) {
        PIPE_ERR("NULL param");
        return MFALSE;
    }

    if (mpCamIOPipe) {
        MUINT32 i = 0;

        for (i = 0; i < RegCount; i++) {
            pRegs[i].Data = mpCamIOPipe->UNI_ReadReg(pRegs[i].Addr, bPhysical);
        }
    }
    else {
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/

MBOOL NormalPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;
    MUINT32 tg;
    MINT32  sensorDev = 1;
    NormalPipe_HBIN_Info    *hbin = NULL;
    NormalPipe_EIS_Info     *pEis = NULL;
    NormalPipe_RSS_Info     *pRss = NULL;

    if (this->FSM_CHECK(op_cmd, __FUNCTION__) == MFALSE) {
        return MFALSE;
    }

    if (!mpCamIOPipe) {
        PIPE_ERR("CamIOPipe NULL");
        return MFALSE;
    }

    switch (cmd) {
    case ENPipeCmd_GET_TG_INDEX:
        //arg1: [Out] (MUINT32*)  CAM_TG_1/CAM_TG_2/...
        if (arg1) {
            *(MINT32*)arg1 = _NORMALPIPE_GET_TG_IDX(mpSensorIdx);
        }
        else {
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_GET_BURSTQNUM:
        if (arg1) {
            *(MINT32*)arg1 = mBurstQNum;
        }
        else {
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_SET_STT_SOF_CB:
        mEnqSofCBLock.lock();
        if (m_SofCB) {
            PIPE_ERR("SOF callback already registered!");
            ret = MFALSE;
            mEnqSofCBLock.unlock();
            goto _EXIT_SEND_CMD;
        }
        m_SofCB = (fp_EventCB)arg1;
        m_SofCBCookie = (void*)arg2;
        mEnqSofCBLock.unlock();
        break;
    case ENPipeCmd_CLR_STT_SOF_CB:
        mEnqSofCBLock.lock();
        if (m_SofCB != (fp_EventCB)arg1) {
            PIPE_ERR("SOF CBFP un-reg wrong!");
            ret = MFALSE;
            mEnqSofCBLock.unlock();
            goto _EXIT_SEND_CMD;
        }
        m_SofCB = NULL;
        mEnqSofCBLock.unlock();
        break;
    case ENPipeCmd_GET_LAST_ENQ_SOF:
        //arg1: [Out] last enque sof_idx
        if (!arg1) {
            PIPE_ERR("NULL param");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        *((MUINT32*)arg1) = mpNPipeThread->m_LastEnqSof;
        break;
    case ENPipeCmd_SET_RRZ_CBFP:
        if (!mpCamIOPipe) {
            PIPE_WRN("RRZ CBFP is not supported on this platform.");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        break;
    case ENPipeCmd_SET_MEM_CTRL:
        //arg1: [In]  0: allocate
        //arg2: [In] IMEM_BUF_INFO*
        //arg3: -
        if (arg1 == 0) {
            if (arg2) {
                IMEM_BUF_INFO *pBuf = (IMEM_BUF_INFO*)arg2;

                if (mpIMem->allocVirtBuf(pBuf) < 0) {
                    PIPE_ERR("TG_%d alloc %d", mpSensorIdx, pBuf->size);
                    ret = MFALSE;
                    break;
                }
                if (mpIMem->mapPhyAddr(pBuf) < 0) {
                    PIPE_ERR("TG_%d map %d", mpSensorIdx, pBuf->size);
                    ret = MFALSE;
                    break;
                }
                mvSttMem.push_back(*pBuf);
                PIPE_DBG("SttBuf depth: %d, cur size: %d", mvSttMem.size(), pBuf->size);
            }
        }
        else {
            ret = MFALSE;
        }
        break;
    case ENPipeCmd_SET_IDLE_HOLD_CB:
        if (0 == arg1) {
            ret = MFALSE;
            break;
        }

        mpNPipeThread->idleHoldNotifyCB = *(CAM_STATE_NOTIFY *)arg1;
        PIPE_INF("set idle hold cb: %p", mpNPipeThread->idleHoldNotifyCB.fpNotifyState);

        break;
    case ENPipeCmd_SET_EIS_CBFP:
        //arg1: [In]   enum EModule
        //arg2: [Out] (MUINT32)Module_Handle
        //arg3: user name string
        ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_EIS_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_LCS_CBFP:
        //arg1: [In]   enum EModule
        //arg2: [Out] (MUINT32)Module_Handle
        //arg3: user name string
        ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_LCS_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_RSS_CBFP:
        //arg1: [In]   enum EModule
        //arg2: [Out] (MUINT32)Module_Handle
        //arg3: user name string
        ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_RSS_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_SET_REGDUMP_CBFP:
        if (!mpCamIOPipe) {
            PIPE_ERR("[Error]mpCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_REGDUMP_CBFP, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_TG_OUT_SIZE:
        //arg1 : width
        //arg2 : height
        #if 1
        if (!arg1 || !arg2) {
            PIPE_ERR("NULL param");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        *((MINT32*)arg1) = _NORMALPIPE_GET_SENSORCFG_CROP_W(mpSensorIdx);
        *((MINT32*)arg2) = _NORMALPIPE_GET_SENSORCFG_CROP_H(mpSensorIdx);
        #else

        if (2 < arg1) {
            PIPE_ERR("Err: Invalid SensorIdx(%d)!",arg1);
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        tg = _NORMALPIPE_GET_TG_IDX(arg1);
        switch (CAM_TG_1) {
            case CAM_TG_1:
                *((MINT32*)arg2) = mImgoOut_size.w;
                *((MINT32*)arg3) = mImgoOut_size.h;
                break;
            case CAM_TG_2:
                *((MINT32*)arg2) = mImgodOut_size.w;
                *((MINT32*)arg3) = mImgodOut_size.h;
                break;
            case CAM_SV_1:
                *((MINT32*)arg2) = mCamSvImgoOut_size.w;
                *((MINT32*)arg3) = mCamSvImgoOut_size.h;
                break;
            case CAM_SV_2:
                *((MINT32*)arg2) = mCamSv2ImgoOut_size.w;
                *((MINT32*)arg3) = mCamSv2ImgoOut_size.h;
                break;
        }
        #endif
        //PIPE_DBG("ENPipeCmd_GET_TG_OUT_SIZE SIdx(%d), w=0x%x, h=0x%x!",arg1, *((MINT32*)arg2), *((MINT32*)arg3));
        break;
    case ENPipeCmd_GET_HBIN_INFO:
        ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_HBIN_INFO, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_BIN_INFO:
        ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_BIN_INFO, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_PMX_INFO:
        ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_PMX_INFO, arg1, arg2, arg3);
        break;
    #if 0
    case ENPipeCmd_GET_RMX_OUT_SIZE:
        //arg1 : sensor-idx
        //arg2 : width
        //arg3 : height
        if (2 < arg1) {
            PIPE_ERR("Err: Invalid SensorIdx(%d)!",arg1);
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        tg = _NORMALPIPE_GET_TG_IDX(arg1);
        switch (tg) {
            case CAM_TG_1:
                *((MINT32*)arg2) = mRrzoOut_size.w;
                *((MINT32*)arg3) = mRrzoOut_size.h;
                break;
            case CAM_TG_2:
                *((MINT32*)arg2) = mRrzodOut_size.w;
                *((MINT32*)arg3) = mRrzodOut_size.h;
                break;
            default:
                PIPE_ERR("Fail! SIdx=%d, TG=%d",arg1,tg);
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
        }
        //PIPE_DBG("ENPipeCmd_GET_RMX_OUT_SIZE SIdx(%d),tg(%d), w=0x%x, h=0x%x!",arg1,tg, *((MINT32*)arg2), *((MINT32*)arg3));
        break;
    #endif
    case ENPipeCmd_GET_EIS_INFO:
        //[X]arg1 : sensor-idx
        //arg2 : NormalPipe_EIS_Info*
        if (0 == arg2) {
            PIPE_ERR("ENPipeCmd_GET_EIS_INFO Err: Invalid Para arg1(%d)",arg1);
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        else {
            MUINT32 _uni = 0;

            pEis = (NormalPipe_EIS_Info*) arg2;

            ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_UNI_INFO, (MINTPTR)&_uni, 0, 0);
            if (MFALSE == ret) {
                PIPE_ERR("N:%d Get UNI INFO failed", mpSensorIdx);
                goto _EXIT_SEND_CMD;
            }

            pEis->mSupported = !!_uni;
        }
        break;
    case ENPipeCmd_GET_UNI_INFO:
        {
            MUINT32 _uni = 0;

            ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_FLK_INFO, (MINTPTR)&_uni, 0, 0);
            if (MFALSE == ret) {
                PIPE_ERR("N:%d Get FLK INFO failed", mpSensorIdx);
                goto _EXIT_SEND_CMD;
            }

            *(MUINT32*)arg1 = !!_uni;
        }
        break;
    case ENPipeCmd_GET_CUR_FRM_STATUS:
        ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS, arg1, arg2, arg3);
        //if 1st enq loop does NOT apply patch, MW cannot enque at this timing
        if (MFALSE == m_b1stEnqLoopDone) {
            *(MUINT32*)arg1 = _1st_enqloop_status;
        }
        break;
    case ENPipeCmd_SET_TG_INT_LINE:
        if (!mpCamIOPipe) {
            PIPE_ERR("[Error]mpCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_TG_INT_LINE, arg1, (MINTPTR)(_NORMALPIPE_MAP_TG_PXLMODE(mpSensorIdx)), arg3);
        break;
    case ENPipeCmd_UNI_SWITCH:
        {
            mpCamIOPipe->sendCommand(EPIPECmd_GET_UNI_ATTACH_MODULE, (MINTPTR)&m_uniSwitchInfo->m_uniAttachedCam, 0, 0);
            ISP_HW_MODULE cam = CAM_MAX;

            /* Feature need to know uni is attached to which cam currently to maintain their state machine */
            if((ISP_HW_MODULE)m_uniSwitchInfo->m_uniAttachedCam != (cam = (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX(mpSensorIdx)) ? CAM_A : CAM_B)) {
                PIPE_ERR("UNI(%d) is not attached to CAM(%d)", m_uniSwitchInfo->m_uniAttachedCam, cam);
                m_uniSwitchInfo->m_uniAttachedCam = CAM_MAX;
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }

            if(!mpNPipeThread->uniFSM_CHK(UNI_SWITCH_NULL)) {
                PIPE_ERR("UNI_SWITCH(%d) is on progress", m_uniSwitchInfo->m_uniSwitchState);
                ret = MFALSE;
                goto _EXIT_SEND_CMD;
            }

            /* Enable uni-switch procedure */
            m_uniSwitchInfo->m_uniSwitchCtrl = MTRUE;
            PIPE_INF("uniAttachedCam(%d), switchstate(%d), m_uniSwitchCtrl(%d)", \
                m_uniSwitchInfo->m_uniAttachedCam, m_uniSwitchInfo->m_uniSwitchState, m_uniSwitchInfo->m_uniSwitchCtrl);
        }
        break;
    case ENPipeCmd_GET_UNI_SWITCH_STATE:
        NormalPipe::NPipeUniSwitchLock.lock();
        *(MUINT32*)arg1 = (MUINT32)m_uniSwitchInfo->m_uniSwitchState;
        NormalPipe::NPipeUniSwitchLock.unlock();
        break;
    case ENPipeCmd_GET_FLK_INFO:
        ret = mpCamIOPipe->sendCommand(EPIPECmd_GET_FLK_INFO, arg1, arg2, arg3);
        break;
    case ENPipeCmd_GET_CUR_SOF_IDX:
    case ENPipeCmd_HIGHSPEED_AE:
    default:
        if (!mpCamIOPipe) {
            PIPE_ERR("[Error]mpCamIOPipe=NULL");
            ret = MFALSE;
            goto _EXIT_SEND_CMD;
        }
        ret = mpCamIOPipe->sendCommand(cmd, arg1, arg2, arg3);
        break;
    }

_EXIT_SEND_CMD:
    if (ret != MTRUE) {
        PIPE_ERR("error: sendCommand fail: (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", cmd, arg1, arg2, arg3);
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
//arg1 : only valid in cmd:_GetPort_Index
MUINT32 NormalPipe::GetOpenedPortInfo(NormalPipe::ePortInfoCmd cmd, MUINT32 arg1)
{
    MUINT32 _num = 0;

    switch (cmd) {
        case NormalPipe::_GetPort_Opened:
            return (MUINT32)mOpenedPort;
        case NormalPipe::_GetPort_OpendNum:
            if (mOpenedPort& NormalPipe::_RRZO_ENABLE)
                _num++;
            if (mOpenedPort& NormalPipe::_UFEO_ENABLE)
                _num++;
            if (mOpenedPort& NormalPipe::_IMGO_ENABLE)
                _num++;
            if (mOpenedPort& NormalPipe::_EISO_ENABLE)
                _num++;
            if (mOpenedPort& NormalPipe::_LCSO_ENABLE)
                _num++;
            if (mOpenedPort& NormalPipe::_RSSO_ENABLE)
                _num++;
            return _num;
        case NormalPipe::_GetPort_Index:
        {
            MUINT32 idx = 0;
            if (mOpenedPort & NormalPipe::_RRZO_ENABLE) {
                PortIndex[idx] = PORT_RRZO.index;
                idx++;
            }
            if (mOpenedPort & NormalPipe::_UFEO_ENABLE) {
                PortIndex[idx] = PORT_UFEO.index;
                idx++;
            }
            if (mOpenedPort & NormalPipe::_IMGO_ENABLE) {
                PortIndex[idx] = PORT_IMGO.index;
                idx++;
            }
            if (mOpenedPort & NormalPipe::_EISO_ENABLE) {
                PortIndex[idx] = PORT_EISO.index;
                idx++;
            }
            if (mOpenedPort & NormalPipe::_LCSO_ENABLE) {
                PortIndex[idx] = PORT_LCSO.index;
                idx++;
            }
            if (mOpenedPort & NormalPipe::_RSSO_ENABLE) {
                PortIndex[idx] = PORT_RSSO.index;
                idx++;
            }
            if ((arg1 >= idx) || (arg1 >= MaxPortIdx)) {
                PIPE_ERR("Opened port num not enough %d/%d/%d", arg1, idx, MaxPortIdx);
                return -1;
            }
            else {
                return PortIndex[arg1];
            }
        }
        case NormalPipe::_GetPort_ImgFmt:
            if (arg1 >= MaxPortIdx) {
                PIPE_ERR("Port num not enough %d/%d", arg1,  MaxPortIdx);
                return -1;
            }
            return PortImgFormat[arg1];
        default:
            PIPE_ERR("un-supported cmd:0x%x\n",cmd);
            return 0;
    }
}

#ifdef USE_IMAGEBUF_HEAP
/******************************************************************************
 *
 ******************************************************************************/
void NormalPipe::FrameMgr::init()
{
    mvFrameQ.clear();
}

/******************************************************************************
 *
 ******************************************************************************/
void NormalPipe::FrameMgr::uninit()
{
    mvFrameQ.clear();
}

/******************************************************************************
 *
 ******************************************************************************/
void NormalPipe::FrameMgr::enque(IImageBuffer *pframe)
{
    Mutex::Autolock lock(mFrameMgrLock);
    PIPE_DBG("FrameMgr::enque+, 0x%x", pframe->getBufPA(0));
    mvFrameQ.add(pframe->getBufPA(0), pframe);//change from VA 2 PA is because of in camera3, drv may have no VA
}

/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer* NormalPipe::FrameMgr::deque(MINTPTR key) //key == va
{
    Mutex::Autolock lock(mFrameMgrLock);
    PIPE_DBG("FrameMgr::deque+ key=0x%x",key);
    IImageBuffer *pframe = mvFrameQ.valueFor(key);
    if (pframe != NULL) {
        mvFrameQ.removeItem(key);
        return pframe;
    }
    return NULL;
}

#else

void NormalPipe::FrameMgr::enque(const BufInfo& pframe)
{
}

NSCam::NSIoPipe::NSCamIOPipe::BufInfo*
NormalPipe::FrameMgr::deque(MINTPTR key) //key == va
{
    return NULL;
}
#endif

/****************************************************
*
*****************************************************/
template <class _T>
void NormalPipe::QueueMgr<_T>::init(void)
{
    this->m_v_pQueue.clear();
}

template <class _T>
void NormalPipe::QueueMgr<_T>::uninit(void)
{
    this->m_v_pQueue.clear();
}

template <class _T>
MBOOL NormalPipe::QueueMgr<_T>::sendCmd(QueueMgr_cmd cmd, MUINTPTR arg1, MUINTPTR arg2)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(mQueueLock);

    switch (cmd) {
    case eCmd_getsize:
        *(MUINT32*)arg1 = m_v_pQueue.size();
        break;
    case eCmd_push:
        this->m_v_pQueue.push_back(*(_T*)(arg1));
        break;
    case eCmd_pop:
        this->m_v_pQueue.pop_front();
        break;
    case eCmd_front:
        *(_T*)arg1 = m_v_pQueue.front();
        //WTF?? {
        //    _T buf;
        //    buf = m_v_pQueue.front();
        //}
        break;
    case eCmd_at:
        if (this->m_v_pQueue.size() > arg1) {
            *(_T*)arg2 = m_v_pQueue.at(arg1);
        }
        else {
            PIPE_INF("size have been reduce to 0x%x(0x%x) by enque", m_v_pQueue.size(),arg1);
            ret = MFALSE;
        }
        break;
    case eCmd_pop_back:
        m_v_pQueue.pop_back();
        break;
    case eCmd_end:
    default:
        PIPE_ERR("error: unsopported cmd:0x%x\n",cmd);
        ret = MFALSE;
        break;
    }

    return ret;
}

/*****************************************************
*
*****************************************************/
NormalPipe_Thread::NormalPipe_Thread(NormalPipe* obj)
{
    m_bStart        = MFALSE;
    mCnt            = 0;
    m_pNormalpipe   = (NormalPipe*)obj;
    m_ConsumedReqCnt = 0;
    m_LastEnqSof    = 0;
    m_EnqThrdState  = _ThreadIdle;
    m_DeqThrdState  = _ThreadIdle;
    m_irqUserKey    = -1;
    m_ShutterTimeMs[0] = 0;
    m_ShutterTimeMs[1] = 0;

    idleHoldNotifyCB.fpNotifyState = NULL;
    idleHoldNotifyCB.Obj = NULL;
    ::sem_init(&m_semEnqueThread, 0, 0);
    ::sem_init(&m_semDequeThread, 0, 0);
    ::sem_init(&m_semDeqLoopToken, 0, 0);
    for(MUINT32 i = 0 ; i < MaxPortIdx ; i++){
        ::sem_init(m_semDeqDonePort + i, 0, 0);
    }
    m_EnqueThread = NULL;
    m_DequeThread = NULL;
}

NormalPipe_Thread* NormalPipe_Thread::createInstance(NormalPipe* obj)
{
    if (obj == 0) {
        PIPE_ERR("create instance fail");
        return NULL;
    }

    return new NormalPipe_Thread(obj);
}

MBOOL NormalPipe_Thread::destroyInstance(void)
{
    delete this;
    return MTRUE;
}

MBOOL NormalPipe_Thread::init(void)
{
    MBOOL ret = MTRUE;
    MUINT32 i, r;
    #if (PTHREAD_CTRL_M == 1)
    pthread_attr_t  attr_e, attr_d;
    #endif

    PIPE_INF("SenIdx=%d, mInitCount(%d)", m_pNormalpipe->mpSensorIdx, this->mCnt);

    if (mCnt > 0) {
        android_atomic_inc((volatile MINT32*)&mCnt);
        return ret;
    }

    ::sem_init(&m_semEnqueThread, 0, 0);
    ::sem_init(&m_semDequeThread, 0, 0);
    ::sem_init(&m_semDeqLoopToken, 0, 0);
    for (i = 0; i < MaxPortIdx; i++) {
        ::sem_init(&m_semDeqDonePort[i], 0, 0);
    }

    m_EnqThrdState = _ThreadRunning;
    m_DeqThrdState = _ThreadRunning;

    #if (PTHREAD_CTRL_M == 0)
    pthread_create(&m_EnqueThread, NULL, ispEnqueThread, this);
    pthread_create(&m_DequeThread, NULL, ispDequeThread, this);
    #else
    /* Create main thread for preview and capture */
    if ((r = pthread_attr_init(&attr_e)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "set enqThd attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_init(&attr_d)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "set deqThd attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_setdetachstate(&attr_e, PTHREAD_CREATE_DETACHED)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "enqThd detach", r);
        return MFALSE;
    }
    if ((r = pthread_attr_setdetachstate(&attr_d, PTHREAD_CREATE_DETACHED)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "deqThd detach", r);
        return MFALSE;
    }
    if ((r = pthread_create(&m_EnqueThread, &attr_e, ispEnqueThread, this)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "enqThd create", r);
        return MFALSE;
    }
    if ((r = pthread_create(&m_DequeThread, &attr_d, ispDequeThread, this)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "deqThd create", r);
    }
    if ((r = pthread_attr_destroy(&attr_e)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "destroy enqThd attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_destroy(&attr_d)) != 0) {
        PIPE_ERR("thread ctrl [%s] fail: %d", "destroy deqThd attr", r);
        return MFALSE;
    }
    #endif

    android_atomic_inc((volatile MINT32*)&mCnt);

    return ret;
}

MBOOL NormalPipe_Thread::uninit(void)
{
    PIPE_INF("SenIdx=%d,mInitCount(%d)", m_pNormalpipe->mpSensorIdx, mCnt);

    if (mCnt <= 0) {
        return MTRUE;
    }

    android_atomic_dec((volatile MINT32*)&mCnt);

    if (mCnt > 0) {
        return MTRUE;
    }

    if (_ThreadRunning == m_EnqThrdState) {
        ::sem_post(&m_semEnqueThread);

        PIPE_INF("Post for enqThread exit");
        while(_ThreadRunning == m_EnqThrdState) {
            usleep(500);
        }

        ::sem_wait(&m_semEnqueThread); //wait thread exit
    }
    if (_ThreadRunning == m_DeqThrdState) {
        ::sem_post(&m_semDequeThread);

        PIPE_INF("Post for deqThread exit");
        while(_ThreadRunning == m_DeqThrdState) {
            usleep(500);
        }

        ::sem_wait(&m_semDequeThread); //wait thread exit
    }

    return MTRUE;
}


MBOOL NormalPipe_Thread::start(void)
{
    PIPE_DBG("Thd: E");

    mLock.lock();
    m_bStart = MTRUE;
    mLock.unlock();

    ::sem_post(&m_semEnqueThread);
    ::sem_post(&m_semDequeThread);

    PIPE_DBG("Thd: X");

    return MTRUE;
}

MBOOL NormalPipe_Thread::stop(MBOOL bForce)
{
    IOPipeTrace _traceObj("NormalPipeThread::stop", IOPIPE_TRACE_EN_NOMALPIPE);

    MUINT32 i = 0, _OpenedPort = 0;

    PIPE_DBG("Thd: E");

    if (this->m_bStart == MTRUE) {
        this->prestop();
    }

    ::sem_post(&m_semDeqLoopToken);

    if(bForce == MTRUE) {
        /*abort enqThr*/
        PIPE_INF("N:%d signal enqThr", m_pNormalpipe->mpSensorIdx);

        this->m_pNormalpipe->signal(EPipeSignal_SOF, this->m_irqUserKey);

        /*abort deqThr*/
        _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);
        for (i = 0; i < _OpenedPort; i++) {
            m_pNormalpipe->abortDma(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i), THIS_NAME);
        }
    }

    //PIPE_INF("Thd: join...");
    //pthread_join(m_EnqueThread, NULL);// <-- while still in SOF_CB, this may causes resource to be freed?? TBD fixme
    //pthread_join(m_DequeThread, NULL);

    PIPE_INF("N:%d:Thd: wait enq", m_pNormalpipe->mpSensorIdx);
    ::sem_wait(&m_semEnqueThread); // wait until thread prepare to leave
    PIPE_INF("N:%d:Thd: wait deq", m_pNormalpipe->mpSensorIdx);
    ::sem_wait(&m_semDequeThread); // wait until thread prepare to leave

    /* Prevent NormalPipe::deque blocking */
    PIPE_INF("N:%d:Thd: post for deque", m_pNormalpipe->mpSensorIdx);
    _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);
    for (i = 0; i < _OpenedPort; i++) {
        ::sem_post(&m_semDeqDonePort[_PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i))]);
    }

    PIPE_DBG("Thd: X");

    return MTRUE;
}

MBOOL NormalPipe_Thread::prestop(void)
{
    this->mLock.lock();
    this->m_bStart = MFALSE;
    this->mLock.unlock();

    return MTRUE;
}


/**
    this enque run in immediate mode,enque 1 port with 1 buffer at 1 time.

* return value:
* 1: isp is already stopped
* 0: sucessed
*-1: fail
*/
MINT32 NormalPipe_Thread::enque(QBufInfo* pQBuf)
{
    NSImageio::NSIspio::PortID      portID;
    NSImageio::NSIspio::QBufInfo    rQBufInfo;
    NSImageio::NSIspio::QBufInfo    rQBufInfo_table;
    MUINT32  PLANE_ID = 0;
    MINT32   ret = 0;
    MUINT32 i = 0, _size = pQBuf->mvOut.size();

    PIPE_DBG("Thd: E");

    rQBufInfo.vBufInfo.resize(_size);

    portID.index = pQBuf->mvOut.at(0).mPortID.index;

    for (i = 0; i < _size; i++) {
        IMEM_BUF_INFO imBuf;

        if (!m_pNormalpipe->mpImgHeaderMgr[_PortMap(portID.index)].sendCmd(
                NormalPipe::eCmd_at, i, (MUINTPTR)&imBuf)) {
            PIPE_ERR("port_%d out of imgheader buf", portID.index);
            return -1;
        }
        rQBufInfo.vBufInfo.at(i).Frame_Header.bReplace = MFALSE;
        rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufSize = IMG_HEADER_SIZE;
        rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufVA  = imBuf.virtAddr;
        rQBufInfo.vBufInfo.at(i).Frame_Header.u4BufPA  = imBuf.phyAddr;
        rQBufInfo.vBufInfo.at(i).Frame_Header.memID    = imBuf.memID;
        rQBufInfo.vBufInfo.at(i).Frame_Header.bufSecu  = imBuf.bufSecu;
        rQBufInfo.vBufInfo.at(i).Frame_Header.bufCohe  = imBuf.bufCohe;

        //UFEO
        if(PORT_UFEO.index == portID.index){
            #ifdef USE_IMAGEBUF_HEAP
            PLANE_ID = 0;
            _NOMRALPIPE_PASS_BUFINFO(pQBuf->mvOut.at(i), rQBufInfo.vBufInfo.at(i),PLANE_ID);
            #else
            PIPE_ERR("UFEO must use HEAP!");
            #endif
            PLANE_ID = 0;
        }
        else{
            _NOMRALPIPE_PASS_BUFINFO(pQBuf->mvOut.at(i), rQBufInfo.vBufInfo.at(i),PLANE_ID);
        }
    }

    if (m_pNormalpipe->mpCamIOPipe)  {
        ret = m_pNormalpipe->mpCamIOPipe->enqueOutBuf(portID, rQBufInfo, 1);
        if (ret < 0) {
            PIPE_ERR("error:enqueOutBuf");
            return -1;
        }
        else if(ret > 0) {
            PIPE_WRN("enq:already stopped, ma:x%x",portID.index);
            return 1;
        }
        else {
            for (i = 0; i < _size; i++) {
                m_pNormalpipe->mpImgHeaderMgr[_PortMap(portID.index)].sendCmd(NormalPipe::eCmd_pop, 0, 0);

                #ifdef USE_IMAGEBUF_HEAP
                m_pNormalpipe->mpFrameMgr->enque(pQBuf->mvOut.at(i).mBuffer);
                #endif
            }
        }
    }

    PIPE_DBG("Thd: X");

    return ret;
}


MBOOL NormalPipe_Thread::dmaCfg(QBufInfo* pQBuf)
{
    NSImageio::NSIspio::STImgCrop   imgCrop;
    NSImageio::NSIspio::STImgResize imgRsz;
    list<NSImageio::NSIspio::STImgCrop>     listImgCrop;
    list<NSImageio::NSIspio::STImgResize>   listImgRsz;
    list<MUINT32>                           listImgRawType;
    NSCam::NSIoPipe::PortID port;
    BufInfo *pBuf;
    MBOOL   ret = MTRUE;
    MUINT32 _cmd, i = 0;
    MINTPTR _arg1 = 0, _arg2 = 0;

    PIPE_DBG("+");

    if (0 == pQBuf->mvOut.size()) {
        PIPE_ERR("qBuf no buf");
        return MFALSE;
    }

    port = pQBuf->mvOut.at(0).mPortID;

    switch (port.index) {
    case EPortIndex_RRZO:
        _arg1 = (MINTPTR)&listImgCrop;
        _arg2 = (MINTPTR)&listImgRsz;
        _cmd = EPIPECmd_SET_RRZ;
        break;
    case EPortIndex_IMGO:
        _arg1 = (MINTPTR)&listImgCrop;
        _arg2 = (MINTPTR)&listImgRawType;
        _cmd = EPIPECmd_SET_IMGO;
        break;
    case EPortIndex_UFEO:
        _arg1 = (MINTPTR)&listImgRsz;
        _cmd = EPIPECmd_SET_UFEO;
        break;
    case EPortIndex_EISO:
    case EPortIndex_LCSO:
    case EPortIndex_RSSO:
        goto _BYPASS_DMA_CFG;
    default:
        PIPE_ERR("N:%d dma:x%x r not supported in dmaCfg()", m_pNormalpipe->mpSensorIdx, port.index);
        return MFALSE;
    }

    for (i = 0; i < pQBuf->mvOut.size(); i++) {
        pBuf = &(pQBuf->mvOut.at(i));

        if ((EPortIndex_IMGO == port.index) || (EPortIndex_RRZO == port.index)) {
            imgCrop.x       = pBuf->FrameBased.mCropRect.p.x;
            imgCrop.y       = pBuf->FrameBased.mCropRect.p.y;
            imgCrop.floatX  = 0;
            imgCrop.floatY  = 0;
            imgCrop.w       = pBuf->FrameBased.mCropRect.s.w;
            imgCrop.h       = pBuf->FrameBased.mCropRect.s.h;
            listImgCrop.push_back(imgCrop);
        }
        if (EPortIndex_IMGO == port.index) {
            listImgRawType.push_back(pBuf->mRawOutFmt);
        }
        if ((EPortIndex_RRZO == port.index) || (EPortIndex_UFEO == port.index)) {
            imgRsz.src_x    = pBuf->FrameBased.mCropRect.p.x;
            imgRsz.src_y    = pBuf->FrameBased.mCropRect.p.y;
            imgRsz.src_w    = pBuf->FrameBased.mCropRect.s.w;
            imgRsz.src_h    = pBuf->FrameBased.mCropRect.s.h;
            imgRsz.tar_x    = 0;
            imgRsz.tar_y    = 0;
            imgRsz.tar_w    = pBuf->FrameBased.mDstSize.w;
            imgRsz.tar_h    = pBuf->FrameBased.mDstSize.h;
            listImgRsz.push_back(imgRsz);
        }

        PIPE_DBG("N:%d dma:x%x cmd:0x%x, crop:(%d,%d,%d,%d), outsize:(%d,%d)",\
                m_pNormalpipe->mpSensorIdx, port.index, _cmd, imgCrop.x, imgCrop.y,\
                imgCrop.w, imgCrop.h, pBuf->FrameBased.mDstSize.w, pBuf->FrameBased.mDstSize.h);
    }

    if (m_pNormalpipe->mpCamIOPipe) {
        //Arg1: list<STImgCrop>*    [RRZO][IMGO]
        //Arg2: list<STImgResize>*  [RRZO] //only tar_w & tar_h r useful here.

        //for de-tuningQueue input

        //lock rrz in order to update rrzo size for EIS to get rrz size
        NormalPipe::NPipeGLock.lock();

        ret = m_pNormalpipe->mpCamIOPipe->sendCommand(_cmd, _arg1, _arg2, 0);
        if (MFALSE == ret) {
            PIPE_ERR("Error: EPIPECmd_SET_RRZ/IMGO/UFEO(0x%x) Fail\n",_cmd);
             NormalPipe::NPipeGLock.unlock();
            return MFALSE;
        }

        if (EPortIndex_RRZO == port.index) {
            m_pNormalpipe->mRrzoOut_size.w = imgRsz.tar_w;
            m_pNormalpipe->mRrzoOut_size.h = imgRsz.tar_h;
        }
        NormalPipe::NPipeGLock.unlock();

        #if 1 //TODO
        // change to sendCommand to config pure raw or processed raw
        if (m_pNormalpipe->mDynSwtRawType){
        }
        #endif
    }

_BYPASS_DMA_CFG:
    PIPE_DBG("-");

    return ret;
}


MBOOL NormalPipe_Thread::tuningUpdate(list<MUINT32>* pLMagicNum)
{
    PIPE_DBG("N:%d M:x%x~x%x", m_pNormalpipe->mpSensorIdx, pLMagicNum->front(), pLMagicNum->back());

    m_pNormalpipe->mLMagicNum = *pLMagicNum;
    //Arg2: magic number, list<MUINT32>
    //Arg3: senIdx, MUINT32, not a list
    return m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE, 0, (MINTPTR)pLMagicNum,
            (MINTPTR)(m_pNormalpipe->mpSensorIdx));
}

MBOOL NormalPipe_Thread::updateFrameTime(MUINT32 timeMs, MUINT32 offset)
{
    if (offset >= 2) {
        PIPE_ERR("Wrong time index");
        return MFALSE;
    }

    m_ShutterTimeMs[offset] = timeMs;

    return MTRUE;
}

MUINT32 NormalPipe_Thread::estimateFrameTime(void)
{
#define SOF_TIMEOUT_TH          (1500)
    MUINT32 _wait_time = 0, _t0 = m_ShutterTimeMs[0], _t1 = m_ShutterTimeMs[1];

    if (_t0 > _t1) {
        _wait_time = (_t0 > SOF_TIMEOUT_TH)? _t0: SOF_TIMEOUT_TH;
    }
    else {
        _wait_time = (_t1 > SOF_TIMEOUT_TH)? _t1: SOF_TIMEOUT_TH;
    }

    _wait_time *= 2;

    return _wait_time;
}

MVOID NormalPipe_Thread::frameToEnque(MUINT32 _OpenedPort, MUINT32 sofIdx)
{
    QBufInfo        _qBuf;
    list<MUINT32>   listMagic;
    MUINT32         i, _port;
    MBOOL           ret = MTRUE;
    MINT32          EqRet;

    PIPE_DBG("+");

    #if 0
    if (CAM_FST_DROP_FRAME == frameStatus) {
        if (sofIdx <= (m_LastEnqSof+1)) {
            MUINT32         j;
            vector<MUINT32> dropQ;

            PIPE_INF("WARNING: N:%d drop 4 old-request @sof_%d", m_pNormalpipe->mpSensorIdx, m_LastEnqSof);

            dropQ.resize(0);
            for(i = 0; i < _OpenedPort; i++) {
                _port = _PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_front, (MUINTPTR)&_qBuf, 0);

                for (j = 0; j < _qBuf.mvOut.size(); j++) {
                    PIPE_DBG("N:%d drop dma:x%x PA:0x%x M:%d", m_pNormalpipe->mpSensorIdx,\
                        _qBuf.mvOut.at(j).mPortID.index, _qBuf.mvOut.at(j).mBuffer->getBufPA(0),\
                        _qBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    if (i == 0) { // all ports should have the same magic_num
                        dropQ.push_back(_qBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    }
                }
            }
            for (i = 0; i < dropQ.size(); i++) {
                if (m_pNormalpipe->m_DropCB) {
                    m_pNormalpipe->m_DropCB(dropQ.at(i), m_pNormalpipe->m_returnCookie);
                }
            }
            PIPE_INF("N:%d drop request# %d [%d~%d]", m_pNormalpipe->mpSensorIdx, dropQ.size(),\
                dropQ.at(0), dropQ.at(dropQ.size()-1));
            return;
        }
    }
    #endif

    #if 1 // check frame header buffer first, prevent enque breaks for some port is out of buffer
    for(i = 0; i < _OpenedPort; i++) {
    	MUINT32 imgHeaderDepth = 0;

        _port = _PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

        m_pNormalpipe->mpImgHeaderMgr[_port].sendCmd(NormalPipe::eCmd_getsize, (MUINTPTR)&imgHeaderDepth, 0);

        if (0 == imgHeaderDepth) {
            PIPE_ERR("N:%d dma:0x%x imgheader buf empty", m_pNormalpipe->mpSensorIdx,
                m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));
            return;
        }
    }
    #endif

    /* If uni-SwitchCtrl is on, do the uni-switch procedure. */
    if(NormalPipe::m_uniSwitchInfo->m_uniSwitchCtrl) {
        if(this->uniFSM_CHK(UNI_SWITCH_OUT)) {
            if(!uniSwitch())
                PIPE_ERR("UniSwitch error!");
        }
    }

    for(i = 0; i < _OpenedPort; i++) {
        _port = _PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

        m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_front, (MUINTPTR)&_qBuf, 0);

        EqRet = enque(&_qBuf);
        if (EqRet != 0) {
            if(EqRet > 0)
                PIPE_WRN("N:%d M:%d enque,already stopped", m_pNormalpipe->mpSensorIdx,
                            _qBuf.mvOut.at(0).FrameBased.mMagicNum_tuning);
            else
                PIPE_ERR("N:%d M:%d enque failed", m_pNormalpipe->mpSensorIdx,
                            _qBuf.mvOut.at(0).FrameBased.mMagicNum_tuning);
            return;
        }

        if (MFALSE == this->m_bStart) {
            PIPE_WRN("enqThread stop: return");
            return;
        }

        ret = dmaCfg(&_qBuf);
        if (MFALSE == ret) {
            PIPE_ERR("N:%d M:%d dmaCfg failed", m_pNormalpipe->mpSensorIdx,
                        _qBuf.mvOut.at(0).FrameBased.mMagicNum_tuning);
            return;
        }

        m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_pop, 0, 0);

        if (0 == listMagic.size()) {
            MUINT32 m;
            for (m = 0; m < _qBuf.mvOut.size(); m++) {
                listMagic.push_back(_qBuf.mvOut.at(m).FrameBased.mMagicNum_tuning);
            }
        }
    }

    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(_qBuf.mShutterTimeNs/1000000), 0, 0);
    updateFrameTime((MUINT32)(_qBuf.mShutterTimeNs/1000000), 0);

    if (MFALSE == tuningUpdate(&listMagic)) {
        PIPE_ERR("N:%d p1_update failed", m_pNormalpipe->mpSensorIdx);
        return;
    }

    m_pNormalpipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&sofIdx, 0, 0);

    m_LastEnqSof = sofIdx;
    m_ConsumedReqCnt++;

    ::sem_post(&m_semDeqLoopToken); //let dequeThread get 1 loop, after all opened ports had enqued
}

MVOID NormalPipe_Thread::enqueRequest(MUINT32 sof_idx)
{
    IOPipeTrace _traceObj("NormalPipeThread::enqueRequest", IOPIPE_TRACE_EN_NOMALPIPE);
    QBufInfo    enqQBuf;
    MUINT32     _ReserveCurReq = 0, i = 0, j = 0;
    MUINT32     _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);
    MUINT32     _size = 0, reqQueNum[MaxPortIdx], frm_status = 0, phyCqCnt = 0, virCqCnt = 0;

    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINTPTR)&frm_status, 0, 0);
    /* check previous enque consumed or not */
    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_GET_CQUPDATECNT,(MINTPTR)MTRUE, (MINTPTR)&phyCqCnt, 0);
    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_GET_CQUPDATECNT,(MINTPTR)MFALSE, (MINTPTR)&virCqCnt, 0);

    PIPE_DBG("+N:%d CurSof:%d enqCnt:%d cqCnt(p/v):%d/%d FrameStatus(%d)",
        m_pNormalpipe->mpSensorIdx, sof_idx, m_ConsumedReqCnt, phyCqCnt, virCqCnt, frm_status);

    if (phyCqCnt != virCqCnt) {
        PIPE_WRN("N:%d cqCnt phy:%d vir:%d reserve request, previous not consumed.", m_pNormalpipe->mpSensorIdx, phyCqCnt, virCqCnt);
        return;
    }

    if (m_ConsumedReqCnt < m_pNormalpipe->mReqDepthPreStart) {
        /* '-1', because of the first tuning is consumed immediate_request at enque() */
        MUINT32 _availReq = 1, _expectConsumedReq;

        PIPE_DBG("N:%d static loop(%d_%d)", m_pNormalpipe->mpSensorIdx, m_pNormalpipe->mReqDepthPreStart,\
            m_ConsumedReqCnt);

        _expectConsumedReq = min((sof_idx+1), m_pNormalpipe->mReqDepthPreStart);
        if (_expectConsumedReq > (m_ConsumedReqCnt+1)) { /* +1 for this loop */
            MUINT32 _dropCnt = (_expectConsumedReq - m_ConsumedReqCnt - 1);
            _availReq += _dropCnt;
            m_ConsumedReqCnt += _dropCnt;

            PIPE_DBG("N:%d drop(%d) static loop", m_pNormalpipe->mpSensorIdx, _dropCnt);
        }

        /* drop expired request */
        for (; _availReq > 1; _availReq--) {
            m_DropEnQ.resize(0);
            for (i = 0; i < _OpenedPort; i++) {
                MUINT32 _port = _PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_front, (MUINTPTR)&enqQBuf, 0);
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_pop, 0, 0);

                for (j = 0; j < enqQBuf.mvOut.size(); j++) {
                    PIPE_DBG("N:%d drop dma:x%x PA:0x%x M:%d", m_pNormalpipe->mpSensorIdx,\
                        enqQBuf.mvOut.at(j).mPortID.index, enqQBuf.mvOut.at(j).mBuffer->getBufPA(0),\
                        enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    if (i == 0) { // all ports should have the same magic_num
                        m_DropEnQ.push_back(enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    }
                }
            }
            for (i = 0; i < m_DropEnQ.size(); i++) {
                if (m_pNormalpipe->m_DropCB) {
                    m_pNormalpipe->m_DropCB(m_DropEnQ.at(i), m_pNormalpipe->m_returnCookie);
                }
            }
            PIPE_INF("N:%d drop request# %d [%d~%d]", m_pNormalpipe->mpSensorIdx, m_DropEnQ.size(),\
                m_DropEnQ.at(0), m_DropEnQ.at(m_DropEnQ.size()-1));
        }

        if (CAM_FST_DROP_FRAME == frm_status) {
            MUINT32 _cur_sof = sof_idx, _last_enq_sof = m_LastEnqSof;

            if ((_cur_sof+SOF_IDX_MSB) < _last_enq_sof) {
                _cur_sof += ((_cur_sof&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
                _last_enq_sof += ((_last_enq_sof&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
                PIPE_INF("N:%d wrapping sof: %d_%d", m_pNormalpipe->mpSensorIdx, _last_enq_sof, _cur_sof);
            }

            if (_cur_sof <= (_last_enq_sof + 1)) {
                _availReq = 0;
            }
        }

        if (_availReq) {
            frameToEnque(_OpenedPort, sof_idx);
        }
        else {
            PIPE_INF("N:%d reserve static request @%d", m_pNormalpipe->mpSensorIdx, m_ConsumedReqCnt);
        }

        //if 1st enq loop does NOT apply patch, MW cannot enque at this timing
        if (m_ConsumedReqCnt >= m_pNormalpipe->mReqDepthPreStart) {
            PIPE_DBG("N:%d static loop done(%d)", m_pNormalpipe->mpSensorIdx, m_ConsumedReqCnt);
            m_pNormalpipe->m_b1stEnqLoopDone = MTRUE;
        }
        PIPE_DBG("- N:%d static loop", m_pNormalpipe->mpSensorIdx);

        return;
    }

    for (i = 0; i < _OpenedPort; i++) {
        MUINT32 _port = _PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

        m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_getsize, (MUINTPTR)&(reqQueNum[_port]), 0);
        if (0 == reqQueNum[_port]) {
            PIPE_INF("N:%d dma:x%x no request sof_idx:%d, FrameStatus(%d)", m_pNormalpipe->mpSensorIdx,
                m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i), sof_idx, frm_status);
            return;
        }
    }

    //available request from MW
    //full mode means 1 request per frame, if requests r over 2, drop request
    m_DropEnQ.resize(0);
    for (i = 0; i < _OpenedPort; i++) {
        MUINT32 _dmaIdx = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index,i);
        MUINT32 _port = _PortMap(_dmaIdx);

        /*
         * push oldest request into drv, and push other requests into reserve que for next frame.
         */
        /*while */if (reqQueNum[_port] > 1) {
            MUINT32 _decentReqCnt = 2/*, _resvNum = 0*/;

            PIPE_INF("WARNING: N:%d dma:x%x over 1 request: %d", m_pNormalpipe->mpSensorIdx, _dmaIdx, reqQueNum[_port]);

            #if 0
            //request over 2, must drop
            m_pNormalpipe->mpEnqueReserved[_port]->sendCmd(NormalPipe::eCmd_getsize, (MUINTPTR)&_resvNum, 0);
            if (_resvNum != 0) {
                _decentReqCnt = 1;
            }
            #endif
            //if reserved container r empty, drop requestes which r over 2, otherwise, leave only 1 request.
            while (reqQueNum[_port] > _decentReqCnt) {
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_front, (MUINTPTR)&enqQBuf, 0);
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_pop, 0, 0);
                reqQueNum[_port]--;

                for (j = 0; j < enqQBuf.mvOut.size(); j++) {
                    if (i == 0) { // all ports should have the same magic_num
                        PIPE_INF("WARNING: N:%d dma:x%x drop req M:x%x", m_pNormalpipe->mpSensorIdx, _dmaIdx,
                                    enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                        m_DropEnQ.push_back(enqQBuf.mvOut.at(j).FrameBased.mMagicNum_tuning);
                    }
                }
            }

            #if 0
            if (_resvNum == 0) {
                //maximum 2 req are remained, and pop latest one of the two into reserved container
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_at, 1, (MUINTPTR)&enqQBuf);
                m_pNormalpipe->mpEnqueReserved[_port]->sendCmd(NormalPipe::eCmd_push, (MUINTPTR)&enqQBuf, 0);
                m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_pop_back, 0, 0);
                reqQueNum[_port]--;

                PIPE_INF("WARNING: N:%d dma:x%x resv req: %d M(0x%x)",
                            m_pNormalpipe->mpSensorIdx, _dmaIdx, enqQBuf.mvOut.size(),
                            ((enqQBuf.mvOut.size())? enqQBuf.mvOut.at(0).FrameBased.mMagicNum_tuning: 0xFFFFFFFF));
            }
            #endif
        }
    }

    //CB for notify MW which enque be dropped.
    for (i = 0; i < m_DropEnQ.size(); i++) {
        PIPE_INF("N:%d drop req CB: M:x%x(%d)", m_pNormalpipe->mpSensorIdx, m_DropEnQ.at(i), m_DropEnQ.size());
        if (m_pNormalpipe->m_DropCB != NULL) {
            m_pNormalpipe->m_DropCB(m_DropEnQ.at(i), m_pNormalpipe->m_returnCookie);
        }
    }
    /*
     * enqueRequest of each port should be 1
     */

    //for HDRViewFinder: if current SOF = enqueRequest SOF, reserve this request
    /* if (0 == m_DropEnQ.size()) */
    {
        MUINT32 _cur_sof_idx, _buf_sof_idx, _last_enq_sof_idx; /* for sof 255 wrapping */

        m_pNormalpipe->mpEnqueRequest[_PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index,0))]
                                      ->sendCmd(NormalPipe::eCmd_front, (MUINTPTR)&enqQBuf, 0);

        _cur_sof_idx        = sof_idx;
        _buf_sof_idx        = enqQBuf.mvOut.at(0).FrameBased.mSOFidx;
        _last_enq_sof_idx   = m_LastEnqSof;

        /* sof = 0~255, _cur_sof_idx-_buf_sof must == 1 */
        if (((_cur_sof_idx+SOF_IDX_MSB) < _buf_sof_idx) || ((_cur_sof_idx+SOF_IDX_MSB) < _last_enq_sof_idx) ||
            ((_buf_sof_idx+SOF_IDX_MSB) < _cur_sof_idx)) {
            /* need wraping, some var is at the other half */
            _cur_sof_idx += ((_cur_sof_idx&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
            _buf_sof_idx += ((_buf_sof_idx&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
            _last_enq_sof_idx += ((_last_enq_sof_idx&SOF_IDX_MSB)? 0: (SOF_IDX_MSB<<1));
            PIPE_INF("N:%d wrapping sof: %d_%d_%d", m_pNormalpipe->mpSensorIdx, _last_enq_sof_idx,
                        _buf_sof_idx, _cur_sof_idx);
        }

        #if 0 /* replaced through phy/vir cqCnt mechanism */
        if (CAM_FST_DROP_FRAME == frm_status) {
            if (_cur_sof_idx <= (_last_enq_sof_idx + m_pNormalpipe->mBurstQNum)) {
                PIPE_INF("WARNING: N:%d prev req over p1done", m_pNormalpipe->mpSensorIdx);
                _ReserveCurReq = 1;
            }
        }
        #endif

        if (0 == m_DropEnQ.size()) {
            if (_cur_sof_idx <= _buf_sof_idx) {
                PIPE_INF("WARNING: N:%d SOF cur_sof(%d)<=buf_sof(%d), postpone req", m_pNormalpipe->mpSensorIdx,
                            _cur_sof_idx, _buf_sof_idx);
                _ReserveCurReq = 1;
            }
        }

        #if 0
        if (_cur_sof_idx - _buf_sof_idx > 1) {
            PIPE_INF("WARNING: N:%d cur_sof(%d)-buf_sof(%d)>1", m_pNormalpipe->mpSensorIdx,
                        sof_idx, _buf_sof_idx);
        }
        #endif
    }

    if (0 == _ReserveCurReq) {
        frameToEnque(_OpenedPort, sof_idx);
    }
    else {
        PIPE_INF("WARNING: N:%d reserve request", m_pNormalpipe->mpSensorIdx);
    }

    #if 0
    /*
     * if reserved container are not empty, push to enqueRequest
     */
    m_pNormalpipe->mpEnqueReserved[_PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, 0))]
                                   ->sendCmd(NormalPipe::eCmd_getsize, (MUINTPTR)&_size, 0);
    if (_size != 0) {
        for (i = 0; i < _OpenedPort; i++) {
            MUINT32 _port = _PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

            m_pNormalpipe->mpEnqueReserved[_port]->sendCmd(NormalPipe::eCmd_front, (MUINTPTR)&enqQBuf, 0);
            m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_push, (MUINTPTR)&enqQBuf, 0);
            m_pNormalpipe->mpEnqueReserved[_port]->sendCmd(NormalPipe::eCmd_pop, 0, 0);
        }
    }
    #endif

    PIPE_DBG("-");
}

MVOID NormalPipe_Thread::immediateRequest(MBOOL bByPassHwEnq)
{
    QBufInfo        _qBuf;
    list<MUINT32>   listMagic;
    MUINT32         i, _port;
    MUINT32         PLANE_ID = 0;

    PIPE_DBG("+");

    for (i = 0; i < m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum); i++) {
        _port = _PortMap(m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i));

        m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_front, (MINTPTR)&_qBuf, 0);

        if (bByPassHwEnq == MFALSE) {
            enque(&_qBuf);
        }
        else {
            PIPE_DBG("bypass hw enque");
        }

        dmaCfg(&_qBuf);
        #if 1 //dbg only
        for (MUINT32 g = 0; g < _qBuf.mvOut.size(); g++) {
#if 0 //ufeo temp
            if(_port == _PortMap(PORT_UFEO.index)){
				PLANE_ID = 1;//UFEO
				}
            else
                PLANE_ID = 0;
#endif
            PIPE_INF("P(%p), SOF(%d), size(%d,%d), crop(%d_%d_%d_%d), M(%d)", \
                    _qBuf.mvOut.at(g).mBuffer->getBufPA(0), _qBuf.mvOut.at(g).FrameBased.mSOFidx,\
                    _qBuf.mvOut.at(g).FrameBased.mDstSize.w, _qBuf.mvOut.at(g).FrameBased.mDstSize.h,\
                    _qBuf.mvOut.at(g).FrameBased.mCropRect.p.x, _qBuf.mvOut.at(g).FrameBased.mCropRect.p.y,\
                    _qBuf.mvOut.at(g).FrameBased.mCropRect.s.w, _qBuf.mvOut.at(g).FrameBased.mCropRect.s.h,\
                    _qBuf.mvOut.at(g).FrameBased.mMagicNum_tuning);
        }
        #endif

        /* m_pNormalpipe->mpEnqueRequest[_port]->sendCmd(NormalPipe::eCmd_pop, 0, 0); */ /* Move to start() */

        if (0 == listMagic.size()) {
            MUINT32 m;
            for (m = 0; m < _qBuf.mvOut.size(); m++) {
                listMagic.push_back(_qBuf.mvOut.at(m).FrameBased.mMagicNum_tuning);
            }
        }
    }

    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_SET_FRM_TIME, (MINTPTR)(_qBuf.mShutterTimeNs/1000000), 0, 0);
    updateFrameTime((MUINT32)(_qBuf.mShutterTimeNs/1000000), 0);

    if (MFALSE == tuningUpdate(&listMagic)) {
        PIPE_ERR("p1_update imm");
        return;
    }

    m_pNormalpipe->mInitSettingApplied = MTRUE;

    m_LastEnqSof = 0;
    m_ConsumedReqCnt++;

    ::sem_post(&m_semDeqLoopToken); //let dequeThread get 1 loop, after all opened ports had enqued

    PIPE_DBG("-");
}

/******************************************************************************
 * NormalPipe::
 * 1: already stopped
 * 0: sucessed
 *-1: fail
 ******************************************************************************/
MINT32 NormalPipe_Thread::dequeResponse(void)
{
#define NPIPE_DEQUE_TIMEOUT     (5566)
#define NPIPE_MAX_DEQ_COUNT     (3)
    PIPE_DBG("+");

    MUINT32 _size = 0, _OpenedPort = 0, idx = 0, i;
    MINT32 deqRet = 0;
    vector<NSImageio::NSIspio::PortID>      QportID; //used for CamIOPipe
    NSImageio::NSIspio::QTimeStampBufInfo   rQTSBufInfo;
    NSImageio::NSIspio::PortID              portID;    //used for CamIOPipe
    IImageBuffer                            *pframe = NULL;
    IMEM_BUF_INFO                           imBuf;
    CAM_STATE_NOTIFY                        notifyObj = {.fpNotifyState = camStateNotify, .Obj = this};

    _OpenedPort = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);

    for (i = 0; i < _OpenedPort; i++) {
        QBufInfo    deqQBuf;
        MUINT32     deqTryCount;

        portID.index = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i);

        m_pNormalpipe->mResetCount = 0;
        for (deqTryCount = NPIPE_MAX_DEQ_COUNT; deqTryCount > 0; deqTryCount--) {
            if ( (deqRet = m_pNormalpipe->mpCamIOPipe->dequeOutBuf(portID, rQTSBufInfo, NPIPE_DEQUE_TIMEOUT, &notifyObj)) >= 0) {
                break;
            }

            if (m_pNormalpipe->mResetCount > 0) {
                MUINT32 retry = m_pNormalpipe->mResetCount; // only query mResetCount once

                for (; retry > 0; retry--) {
                    PIPE_INF("N:%d dma:x%x reset retry flow %d times",
                        m_pNormalpipe->mpSensorIdx, portID.index, retry);

                    m_pNormalpipe->reset();

                    if ((deqRet = m_pNormalpipe->mpCamIOPipe->dequeOutBuf(portID, rQTSBufInfo, NPIPE_DEQUE_TIMEOUT, &notifyObj)) >= 0) {
                        m_pNormalpipe->mResetCount = 0;
                        break;
                    }
                }
                if (0 == retry) {
                    PIPE_ERR("N:%d dma:x%x deque(%d) fail after reset sensor",
                        m_pNormalpipe->mpSensorIdx, portID.index, NPIPE_MAX_DEQ_COUNT);
                    return -1;
                }

                break;
            }
        }
        if(deqRet > 0){
            PIPE_DBG("N:%d already stopped, dma:x%x", m_pNormalpipe->mpSensorIdx, portID.index);
            return 1;
        }

        if (0 == deqTryCount) {
            PIPE_ERR("N:%d dma:x%x deque(%d) fail", m_pNormalpipe->mpSensorIdx, portID.index, NPIPE_MAX_DEQ_COUNT);
            return -1;
        }

        if (rQTSBufInfo.vBufInfo.size() != m_pNormalpipe->mBurstQNum) {
            PIPE_ERR("N:%d dma:x%x deque buf num error, %d/%d", m_pNormalpipe->mpSensorIdx,\
                    portID.index, rQTSBufInfo.vBufInfo.size(), m_pNormalpipe->mBurstQNum);
            return -1;
        }

        /* if there is at least one buf dequed
         *  => return done buf with empty buf to caller
         * if dequed buf num is not mBurstQNum,
         *  => return error
         */
        for (idx = 0; idx < rQTSBufInfo.vBufInfo.size(); idx++) {
            BufInfo         buff;
            ResultMetadata  result;

            #ifdef USE_IMAGEBUF_HEAP
            //chage from va 2 pa is because of in camera3, drv may have no va infor.
            pframe = m_pNormalpipe->mpFrameMgr->deque((MINTPTR)rQTSBufInfo.vBufInfo.at(idx).u4BufPA[0]);
            if (NULL == pframe) {
                PIPE_ERR("N:%d dma:x%x frameMgr null idx_%d,PA_0x%x", m_pNormalpipe->mpSensorIdx,\
                        portID.index, idx, rQTSBufInfo.vBufInfo.at(idx).u4BufPA[0]);
                return -1;
            }
            else {
                pframe->setTimestamp(rQTSBufInfo.vBufInfo.at(idx).getTimeStamp_ns());
            }
            #endif

            result.mDstSize     = rQTSBufInfo.vBufInfo.at(idx).DstSize;
            result.mTransform   = 0;
            result.mMagicNum_hal = rQTSBufInfo.vBufInfo.at(idx).m_num;
            result.mMagicNum_tuning = rQTSBufInfo.vBufInfo.at(idx).m_num;
            result.mRawType     = rQTSBufInfo.vBufInfo.at(idx).raw_type;
            result.mTimeStamp   = m_pNormalpipe->getTimeStamp_ns(rQTSBufInfo.vBufInfo.at(idx).i4TimeStamp_sec, rQTSBufInfo.vBufInfo.at(idx).i4TimeStamp_us);
            switch (portID.index) {
            case EPortIndex_IMGO:
                result.mCrop_s = MRect(MPoint(0,0), MSize(rQTSBufInfo.vBufInfo.at(idx).img_w, rQTSBufInfo.vBufInfo.at(idx).img_h));
                result.mCrop_d = rQTSBufInfo.vBufInfo.at(idx).crop_win;
                break;
            case EPortIndex_RRZO:
                result.mCrop_s = rQTSBufInfo.vBufInfo.at(idx).crop_win;
            case EPortIndex_UFEO:
                result.mCrop_s = rQTSBufInfo.vBufInfo.at(idx).crop_win;
                result.mCrop_d = MRect(MPoint(0, 0), MSize(result.mDstSize.w, result.mDstSize.h));
                break;
            case EPortIndex_EISO:
            case EPortIndex_LCSO:
                result.mCrop_s = result.mCrop_d = 0;
                break;
            case EPortIndex_RSSO:
                result.mCrop_s = result.mCrop_d = 0;
                break;
            default:
                PIPE_ERR("unsupported dmao:0x%x", portID.index);
                portID.index = (MUINT8)-1;
                return -1;
            }

            buff.mPortID    = portID.index;
            buff.mBuffer    = pframe;
            buff.mMetaData  = result;
            buff.mSize      = rQTSBufInfo.vBufInfo.at(idx).u4BufSize[0];
            buff.mVa        = rQTSBufInfo.vBufInfo.at(idx).u4BufVA[0];
            buff.mPa        = rQTSBufInfo.vBufInfo.at(idx).u4BufPA[0];

            //m_pNormalpipe->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::eCmd_push, (MUINTPTR)&buff, 0);
            deqQBuf.mvOut.push_back(buff);

            PIPE_DBG("DEQ:%d: dma:x%x pa(0x%x) io(%d_%d_%d_%d_%d_%d_%d_%d_%d_%d) mag(0x%x)",\
                    m_pNormalpipe->mpSensorIdx, portID.index, buff.mPa,\
                    result.mCrop_s.p.x,result.mCrop_s.p.y,result.mCrop_s.s.w,result.mCrop_s.s.h, \
                    result.mCrop_d.p.x,result.mCrop_d.p.y,result.mCrop_d.s.w,result.mCrop_d.s.h, \
                    result.mDstSize.w,result.mDstSize.h,\
                    result.mMagicNum_tuning);

            /* Get back image header buffer */
            imBuf           = m_pNormalpipe->mImgHeaderPool;
            imBuf.virtAddr  = rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufVA;
            imBuf.phyAddr   = rQTSBufInfo.vBufInfo.at(idx).Frame_Header.u4BufPA;
            imBuf.size      = IMG_HEADER_SIZE;
            m_pNormalpipe->mpImgHeaderMgr[_PortMap(portID.index)].sendCmd(NormalPipe::eCmd_push, (MUINTPTR)&imBuf, 0);
        }

        #if 0 // not needed when container entry is QBufInfo
        /* fill container with empty buf */
        if (idx < m_pNormalpipe->mBurstQNum) {
            /* some not dequed, pad empty buf to deque container
             * empty buf + deque buf num must equal to 'mBurstQNum'
             */
            BufInfo buff;
            _NOMRALPIPE_SET_BUFEMPTY(buff);
            for (; idx < m_pNormalpipe->mBurstQNum; idx++) {
                //m_pNormalpipe->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::eCmd_push, (MUINTPTR)&buff, 0);
                deqQBuf.mvOut.push_back(buff);
            }
            PIPE_INF("WARNNING: N:%d dma_%d push empty buf", m_pNormalpipe->mpSensorIdx, portID.index);
        }
        #endif

        m_pNormalpipe->mDeqContainerLock.lock();
        m_pNormalpipe->mpDeQueue[_PortMap(portID.index)]->sendCmd(NormalPipe::eCmd_push, (MUINTPTR)&deqQBuf, 0);
        m_pNormalpipe->mDeqContainerLock.unlock();
    } //for (i = 0; i < _OpenedPort; i++)

    /* All ports dequed, and pushed into deque container */
    for (i = 0; i < _OpenedPort; i++) {
        portID.index = m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i);
        if(portID.index != EPortIndex_UFEO)
            ::sem_post(&m_semDeqDonePort[_PortMap(portID.index)]);
    }

    PIPE_DBG("-");

    return 0;
}

MINT32 NormalPipe_Thread::idleStateHold(MUINT32 hold, E_ThreadGrop thdGrpBm)
{
    if (hold) {
        PIPE_INF("Holding idle locks grp: 0x%x", thdGrpBm);

        if ((thdGrpBm & eThdGrp_Enq) == eThdGrp_Enq) {
            mThreadIdleLock_Enq.lock();
        }
        if ((thdGrpBm & eThdGrp_Deq) == eThdGrp_Deq) {
            mThreadIdleLock_Deq.lock();
        }

        if ((thdGrpBm & eThdGrp_External) == eThdGrp_External) {
            if (idleHoldNotifyCB.fpNotifyState) {
                idleHoldNotifyCB.fpNotifyState(CAM_HOLD_IDLE, idleHoldNotifyCB.Obj);
            }
        }
    } else {
        if ((thdGrpBm & eThdGrp_External) == eThdGrp_External) {
            if (idleHoldNotifyCB.fpNotifyState) {
                idleHoldNotifyCB.fpNotifyState(CAM_RELEASE_IDLE, idleHoldNotifyCB.Obj);
            }
        }

        if ((thdGrpBm & eThdGrp_Enq) == eThdGrp_Enq) {
            mThreadIdleLock_Enq.unlock();
        }
        if ((thdGrpBm & eThdGrp_Deq) == eThdGrp_Deq) {
            mThreadIdleLock_Deq.unlock();
        }

        PIPE_INF("Released idle locks grp: 0x%x", thdGrpBm);
    }

    return 0;
}

MUINT32 NormalPipe_Thread::camStateNotify(CAM_STATE_OP state, MVOID *Obj)
{
    NormalPipe_Thread *pNpipeThd = (NormalPipe_Thread *)Obj;

    switch (state) {
    case CAM_ENTER_WAIT:
        pNpipeThd->mThreadIdleLock_Deq.unlock();
        break;
    case CAM_EXIT_WAIT:
        pNpipeThd->mThreadIdleLock_Deq.lock();
        break;
    default:
        PIPE_ERR("Unknown state: %d", state);
        break;
    }

    return 0;
}

MINT32 NormalPipe_Thread::uniFSM_CHK(MUINT32 op)
{
    ISP_HW_MODULE hwModule = CAM_MAX;
    MUINT32 ret =MTRUE, uni_cam = CAM_MAX;

    m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_GET_UNI_ATTACH_MODULE, (MINTPTR)&uni_cam, 0, 0);
    hwModule = (CAM_TG_1 == _NORMALPIPE_GET_TG_IDX(m_pNormalpipe->mpSensorIdx)) ? CAM_A : CAM_B;

    NormalPipe::NPipeUniSwitchLock.lock();

    switch(op)
    {
        case UNI_SWITCH_NULL:
            if(NormalPipe::m_uniSwitchInfo->m_uniSwitchState == UNI_SWITCH_NULL) {
                NormalPipe::m_uniSwitchInfo->m_uniSwitchState = UNI_SWITCH_OUT;
            } else {
                ret = MFALSE;
            }
            break;
        case UNI_SWITCH_OUT:
            switch(NormalPipe::m_uniSwitchInfo->m_uniSwitchState)
            {
                case UNI_SWITCH_OUT:
                case UNI_SWITCH_OUT_TRIG:
                    if(NormalPipe::m_uniSwitchInfo->m_uniAttachedCam != hwModule) {
                        ret = MFALSE;
                    }
                    break;
                case UNI_SWITCH_IN:
                case UNI_SWITCH_IN_TRIG:
                    if(NormalPipe::m_uniSwitchInfo->m_uniAttachedCam == hwModule) {
                        ret = MFALSE;
                    }
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        default:
            break;
    }

    PIPE_DBG("Sensor(%d), FSM(%d), op(%d)", m_pNormalpipe->mpSensorIdx, NormalPipe::m_uniSwitchInfo->m_uniSwitchState, op);

    NormalPipe::NPipeUniSwitchLock.unlock();
    return ret;
}

MINT32 NormalPipe_Thread::uniSwitch()
{
    MUINT32 isOutrigDone=0;
    MINT32 ret = MTRUE;

    PIPE_INF("uniSwitchState(%d), uniAttachedCam(%d)", \
        NormalPipe::m_uniSwitchInfo->m_uniSwitchState, NormalPipe::m_uniSwitchInfo->m_uniAttachedCam);

    NormalPipe::NPipeUniSwitchLock.lock();

    switch(NormalPipe::m_uniSwitchInfo->m_uniSwitchState)
    {
        case UNI_SWITCH_OUT:
                ret = m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_UNI_SWITCHOUT, 0, 0, 0);
                if(ret == MFALSE) {
                    PIPE_ERR("Uni_switchout error state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
                } else {
                    NormalPipe::m_uniSwitchInfo->m_uniSwitchState = UNI_SWITCH_OUT_TRIG;
                    PIPE_INF("Switch_out done state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
                }
            break;
        case UNI_SWITCH_OUT_TRIG:
                ret = m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_UNI_SWITCHOUTRIG, 0, 0, 0);
                if(ret == MFALSE) {
                    PIPE_ERR("Uni_switchout_trig error state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
                } else {
                    NormalPipe::m_uniSwitchInfo->m_uniSwitchState = UNI_SWITCH_IN;
                    PIPE_INF("Switch_outrig done state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
                }
            break;
        case UNI_SWITCH_IN:
                m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_GET_UNI_SWITCHOUTRIG_DONE, (MINTPTR)&isOutrigDone, 0, 0);
                if(isOutrigDone) {
                    ret = m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_UNI_SWITCHIN, 0, 0, 0);
                    if(ret == MFALSE) {
                        PIPE_ERR("Uni_switchin error state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
                    } else {
                        NormalPipe::m_uniSwitchInfo->m_uniSwitchState = UNI_SWITCH_IN_TRIG;
                        PIPE_INF("Switch_in done state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
                    }
                } else {
                    PIPE_INF("EIS/RSS hw is not disabled!");
                }
            break;
        case UNI_SWITCH_IN_TRIG:
                ret = m_pNormalpipe->mpCamIOPipe->sendCommand(EPIPECmd_UNI_SWITCHINTRIG, 0, 0, 0);
                if(ret == MFALSE) {
                    PIPE_ERR("Uni_switchin_trig error state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
                } else {
                    NormalPipe::m_uniSwitchInfo->m_uniSwitchState = UNI_SWITCH_NULL;
                    NormalPipe::m_uniSwitchInfo->m_uniAttachedCam = CAM_MAX;
                    NormalPipe::m_uniSwitchInfo->m_uniSwitchCtrl = MFALSE;
                    PIPE_INF("Switch_intrig done state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
                }
            break;
        default:
            PIPE_ERR("Wrong switch state(%d)", NormalPipe::m_uniSwitchInfo->m_uniSwitchState);
            break;
    }

    NormalPipe::NPipeUniSwitchLock.unlock();

    return ret;
}

MVOID* NormalPipe_Thread::ispEnqueThread(void *arg)
{
    NormalPipe_Thread *_this = reinterpret_cast<NormalPipe_Thread*>(arg);
    MUINT32 _sof_idx, _sof_idx2, ret;

    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_P1_ENQUE;
    struct sched_param sched_p;

    ::sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        setThreadPriority(policy, priority);
        #if 0
        int _policy    = SCHED_OTHER;
        int _priority  = NICE_CAMERA_LOMO;
        getThreadPriority(_policy, _priority);

        PIPE_DBG(
            "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , policy, _policy, priority, _priority
            );
        #endif
    }
    else {   //  Note: "priority" is real-time priority.
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }
    //  detach thread => cannot be join, it means that thread would release resource after exit
    #if (PTHREAD_CTRL_M == 0)
    ::pthread_detach(::pthread_self());
    #endif

    ::sem_wait(&_this->m_semEnqueThread);

    _this->mThreadIdleLock_Enq.lock();

    if (MFALSE == _this->m_bStart) {
        PIPE_INF("enqThread early exit");
        goto _exit;
    }

    PIPE_INF("WARNING: use self priority NICE_CAMERA_P1_ENQUE:%d", NICE_CAMERA_P1_ENQUE);
    switch (_NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe->mpSensorIdx)) {
    case CAM_TG_1:  ::prctl(PR_SET_NAME,"IspEnqThd_TG1",0,0,0);
        break;
    case CAM_TG_2:  ::prctl(PR_SET_NAME,"IspEnqThd_TG2",0,0,0);
        break;
    case CAM_SV_1:  ::prctl(PR_SET_NAME,"IspEnqThd_SV1",0,0,0);
        break;
    case CAM_SV_2:  ::prctl(PR_SET_NAME,"IspEnqThd_SV2",0,0,0);
        break;
    default:
        PIPE_ERR("ERROR: TG:0x%x(0x%x)", _NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe->mpSensorIdx),\
                _this->m_pNormalpipe->mpSensorIdx);
        goto _exit;
    }

    _this->m_irqUserKey = _this->m_pNormalpipe->attach(THIS_NAME);
    if (_this->m_irqUserKey < 0) {
        PIPE_ERR("ERROR: attach IRQ failed %d", _this->m_irqUserKey);
        goto _exit;
    }
    PIPE_DBG("Got irqUserKey(%d)", _this->m_irqUserKey);

    while (1) {
        if (MFALSE == _this->m_bStart) {
            PIPE_DBG("enqThread leave");
            break;
        }

        _this->mThreadIdleLock_Enq.unlock();
        ret = _this->m_pNormalpipe->wait(EPipeSignal_SOF, EPipeSignal_ClearWait, _this->m_irqUserKey, _this->estimateFrameTime(), NULL);
        _this->mThreadIdleLock_Enq.lock();
        if (MFALSE == ret) {
            PIPE_ERR("ERROR: wait SOF fail");
            continue;
        }

        if (_this->m_bStart == MFALSE) {
            PIPE_DBG("enqThread leave after wait SOF\n");
            break;
        }

        _this->m_pNormalpipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX,(MINTPTR)&_sof_idx,0,0);
        PIPE_DBG("N:%d: cur SOF: %d", _this->m_pNormalpipe->mpSensorIdx, _sof_idx);

        _this->m_pNormalpipe->mEnqContainerLock.lock();
        _this->enqueRequest(_sof_idx);
        _this->m_pNormalpipe->mEnqContainerLock.unlock();

        _this->m_pNormalpipe->mEnqSofCBLock.lock();
        if (_this->m_pNormalpipe->m_SofCB) {
            _this->m_pNormalpipe->m_SofCB(&(_this->m_pNormalpipe->mLMagicNum), _this->m_pNormalpipe->m_SofCBCookie);
        }
        _this->m_pNormalpipe->mEnqSofCBLock.unlock();

        _this->m_pNormalpipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX,(MINTPTR)&_sof_idx2,0,0);
        if(_sof_idx != _sof_idx2){
            PIPE_INF("WARNING: N:%d: enqueRequest over vsync(%d_%d)", _this->m_pNormalpipe->mpSensorIdx,\
                    _sof_idx, _sof_idx2);
        }
    }
_exit:
    _this->m_EnqThrdState = _ThreadFinish;

    _this->mThreadIdleLock_Enq.unlock();

    ::sem_post(&_this->m_semEnqueThread);

    return NULL;
}


MVOID* NormalPipe_Thread::ispDequeThread(void *arg)
{
    NormalPipe_Thread *_this = reinterpret_cast<NormalPipe_Thread*>(arg);
    MUINT32 _sof_idx;
    MINT32 ret = 0;//success

    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_P1_DEQUE;
    struct sched_param sched_p;

    ::sched_getparam(0, &sched_p);
    if (policy == SCHED_OTHER) {
        setThreadPriority(policy, priority);
    }
    else {  //Note: "priority" is real-time priority.
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }

    #if (PTHREAD_CTRL_M == 0)
    ::pthread_detach(::pthread_self());
    #endif

    ::sem_wait(&_this->m_semDequeThread);

    _this->mThreadIdleLock_Deq.lock();

    if (MFALSE == _this->m_bStart) {
        PIPE_INF("deqThread early exit");
        goto _exit_deq;
    }

    PIPE_INF("WARNING: use self priority, NICE_CAMERA_P1_DEQUE:%d",NICE_CAMERA_P1_DEQUE);
    switch (_NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe->mpSensorIdx)) {
    case CAM_TG_1:  ::prctl(PR_SET_NAME,"IspDeqThd_TG1",0,0,0);
        break;
    case CAM_TG_2:  ::prctl(PR_SET_NAME,"IspDeqThd_TG2",0,0,0);
        break;
    case CAM_SV_1:  ::prctl(PR_SET_NAME,"IspDeqThd_SV1",0,0,0);
        break;
    case CAM_SV_2:  ::prctl(PR_SET_NAME,"IspDeqThd_TG2",0,0,0);
        break;
    default:
        PIPE_ERR("ERROR: TG:0x%x(0x%x)", _NORMALPIPE_GET_TG_IDX(_this->m_pNormalpipe->mpSensorIdx),\
                _this->m_pNormalpipe->mpSensorIdx);
        goto _exit_deq;
    }

    while (1) {
        if (_this->m_bStart == MFALSE) {
            PIPE_DBG("deqThread leave");
            break;
        }

        _this->mThreadIdleLock_Deq.unlock();
        ::sem_wait(&_this->m_semDeqLoopToken);
        _this->mThreadIdleLock_Deq.lock();

        if (_this->m_bStart == MFALSE) {
            PIPE_DBG("deqThread leave");
            break;
        }

        _this->m_pNormalpipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&_sof_idx, 0, 0);
        PIPE_DBG("N:%d_deq@SOF_%d", _this->m_pNormalpipe->mpSensorIdx,  _sof_idx);

        ret = _this->dequeResponse();

        if (ret < 0) {
            PIPE_ERR("dequeResponse false break");
            break;
        } else if (ret > 0){
            PIPE_WRN("deqThread leave:dequeResponse already stopped");
            break;
        }
    }

_exit_deq:
    if (ret == 0) {
        _this->m_DeqThrdState = _ThreadFinish;
    }
    else {
        MUINT32 i = 0;
        MUINT32 _OpenedPort = _this->m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_OpendNum);

        if (ret > 0)
            _this->m_DeqThrdState = _ThreadStop;
        else
            _this->m_DeqThrdState = _ThreadErrExit;

        for (i = 0; i < _OpenedPort; i++) {
            ::sem_post(&_this->m_semDeqDonePort[_PortMap(_this->m_pNormalpipe->GetOpenedPortInfo(NormalPipe::_GetPort_Index, i))]);
        }
    }

    _this->mThreadIdleLock_Deq.unlock();

    ::sem_post(&_this->m_semDequeThread);

    return NULL;
}

