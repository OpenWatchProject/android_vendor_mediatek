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
#define LOG_TAG "Iop/P2PPWp"

#include "HalPipeWrapper.h"
#include <imageio/IPipe.h>
#include <imageio/IPostProcPipe.h>
#include <mtkcam/drv/def/ispio_sw_scenario.h>
#include <imageio/ispio_stddef.h>
#include <mtkcam/drv/IHalSensor.h>

#include <vector>
/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#define IP_BASE_ISP_3    (1)
#include <mtkcam/utils/std/ULog.h>
#include <imageio/inc/imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(P2PipeWrapper);
DECLARE_DBG_LOG_VARIABLE(P2PipeWrapper_Thread);
CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_DIP);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST

#define LOG_VRB(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (P2PipeWrapper_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace NSCam;
using namespace NSCam::NSIoPipeIsp3::NSPostProc;
//using namespace NSPostProc;
using namespace NSIoPipeIsp3;

pthread_mutex_t CQUserNumMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for cq user number when enqueue/dequeue
pthread_mutex_t BufBatchEnQMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for cq user number when enqueue/dequeue
pthread_mutex_t BufNodeEnQMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for biffer node number when enqueue/dequeue
pthread_mutex_t BufBatchDeQMutex = PTHREAD_MUTEX_INITIALIZER;  //mutex for dequeued buffer list read/write
pthread_mutex_t VencCnt_Mutex = PTHREAD_MUTEX_INITIALIZER;          //mutex for count venc port number
pthread_mutex_t VssOccupied_Mutex = PTHREAD_MUTEX_INITIALIZER;  	//mutex for vss occupied check	// For Vss Concurrency Check
pthread_mutex_t MultiframeEnQMutex = PTHREAD_MUTEX_INITIALIZER;    // mutex for multiframe enque

nsecs_t nsTimeoutToWait = 3LL*1000LL*1000LL;//wait 3 msecs.


PortTypeMapping mPortTypeMapping[EPortType_Memory+1] =
{
    {NSCam::NSIoPipe::EPortType_Sensor,NSImageioIsp3::NSIspio::EPortType_Sensor},
    {NSCam::NSIoPipe::EPortType_Memory,NSImageioIsp3::NSIspio::EPortType_Memory}
};

SwHwScenarioPathMapping mSwHwPathMapping[eSoftwareScenario_total_num]=
{
    {eSoftwareScenario_Main_Normal_Stream,   "Main_NormalStream",     NSImageioIsp3::NSIspio::eDrvScenario_CC,                 NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    {eSoftwareScenario_Main_Normal_Capture,  "Main_NormalCapture",    NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    {eSoftwareScenario_Main_VSS_Capture,     "Main_VSSCapture",       NSImageioIsp3::NSIspio::eDrvScenario_VSS,                NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Main_ZSD_Capture,     "Main_ZSDCapture",       NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Main_Mfb_Capture,     "Main_MFBCapture",       NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Main_Mfb_Blending,    "Main_MFBBlending",      NSImageioIsp3::NSIspio::eDrvScenario_CC_MFB_Blending,    NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Main_Mfb_Mixing,      "Main_MFBMixing",        NSImageioIsp3::NSIspio::eDrvScenario_CC_MFB_Mixing,      NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Main_VSS_Mfb_Capture,     "Main_MFBCapture_Vss",       NSImageioIsp3::NSIspio::eDrvScenario_VSS,           NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Main_VSS_Mfb_Blending,    "Main_MFBBlending_Vss",      NSImageioIsp3::NSIspio::eDrvScenario_VSS_MFB_Blending,    NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Main_VSS_Mfb_Mixing,      "Main_MFBMixing_Vss",        NSImageioIsp3::NSIspio::eDrvScenario_VSS_MFB_Mixing,      NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Main_Vfb_Stream_1,    "Main_vFBStream1",       NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_Normal,      NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    {eSoftwareScenario_Main_Vfb_Stream_2,    "Main_vFBStream2",       NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_FB,          NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Main_Pure_Raw_Stream, "Main_PureRawStream",    NSImageioIsp3::NSIspio::eDrvScenario_IP,                 NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    {eSoftwareScenario_Main_CC_Raw_Stream,   "Main_CCRawStream",      NSImageioIsp3::NSIspio::eDrvScenario_CC_RAW,             NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    {eSoftwareScenario_Main_IP_Raw_TPipe_Stream, "Main_IPRawTPipeStream",    NSImageioIsp3::NSIspio::eDrvScenario_IP_TPipe,    NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    //
    {eSoftwareScenario_Main2_N3D_Stream,     "Main2_N3DStream",       NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_Normal,          NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub_Normal_Stream,    "Sub_NormalStream",      NSImageioIsp3::NSIspio::eDrvScenario_CC,                 NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Sub_Normal_Capture,   "Sub_NormalCapture",     NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Sub_VSS_Capture,      "Sub_VSSCapture",        NSImageioIsp3::NSIspio::eDrvScenario_VSS,                NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub_ZSD_Capture,      "Sub_ZSDCapture",        NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2B,    EPathCQ_CQ2},
    {eSoftwareScenario_Sub_Mfb_Capture,      "Sub_MFBCapture",        NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Sub_Mfb_Blending,     "Sub_MFBBlending",       NSImageioIsp3::NSIspio::eDrvScenario_CC_MFB_Blending,    NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Sub_Mfb_Mixing,       "Sub_MFBMixing",         NSImageioIsp3::NSIspio::eDrvScenario_CC_MFB_Mixing,      NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Sub_VSS_Mfb_Capture,     "Sub_MFBCapture_Vss",       NSImageioIsp3::NSIspio::eDrvScenario_VSS,           NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub_VSS_Mfb_Blending,    "Sub_MFBBlending_Vss",      NSImageioIsp3::NSIspio::eDrvScenario_VSS_MFB_Blending,    NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub_VSS_Mfb_Mixing,      "Sub_MFBMixing_Vss",        NSImageioIsp3::NSIspio::eDrvScenario_VSS_MFB_Mixing,      NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub_Vfb_Stream_1,     "Sub_vFBStream1",        NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_Normal,      NSImageioIsp3::NSIspio::EPipePass_PASS2B,    EPathCQ_CQ2},
    {eSoftwareScenario_Sub_Vfb_Stream_2,     "Sub_vFBStream2",        NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_FB,          NSImageioIsp3::NSIspio::EPipePass_PASS2,     EPathCQ_CQ1},
    {eSoftwareScenario_Sub_Pure_Raw_Stream,  "Sub_PureRawStream",     NSImageioIsp3::NSIspio::eDrvScenario_IP,                 NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Sub_CC_Raw_Stream,    "Sub_CCRawStream",       NSImageioIsp3::NSIspio::eDrvScenario_CC_RAW,             NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Sub_IP_Raw_TPipe_Stream,  "Sub_IPRawTPipeStream",     NSImageioIsp3::NSIspio::eDrvScenario_IP_TPipe,    NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    //
    {eSoftwareScenario_Main2_Normal_Stream,   "Main2_NormalStream",     NSImageioIsp3::NSIspio::eDrvScenario_CC,                 NSImageioIsp3::NSIspio::EPipePass_PASS2C,    EPathCQ_CQ3},
    {eSoftwareScenario_Main2_Normal_Capture,  "Main2_NormalCapture",    NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2C,    EPathCQ_CQ3},
    {eSoftwareScenario_Main2_VSS_Capture,     "Main2_VSSCapture",       NSImageioIsp3::NSIspio::eDrvScenario_VSS,                NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Main2_ZSD_Capture,     "Main2_ZSDCapture",       NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Main2_Mfb_Capture,     "Main2_MFBCapture",       NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Main2_Mfb_Blending,    "Main2_MFBBlending",      NSImageioIsp3::NSIspio::eDrvScenario_CC_MFB_Blending,    NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Main2_Mfb_Mixing,      "Main2_MFBMixing",        NSImageioIsp3::NSIspio::eDrvScenario_CC_MFB_Mixing,      NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Main2_VSS_Mfb_Capture,     "Main2_MFBCapture_Vss",       NSImageioIsp3::NSIspio::eDrvScenario_VSS,           NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Main2_VSS_Mfb_Blending,    "Main2_MFBBlending_Vss",      NSImageioIsp3::NSIspio::eDrvScenario_VSS_MFB_Blending,    NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Main2_VSS_Mfb_Mixing,      "Main2_MFBMixing_Vss",        NSImageioIsp3::NSIspio::eDrvScenario_VSS_MFB_Mixing,      NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Main2_Vfb_Stream_1,    "Main2_vFBStream1",       NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_Normal,      NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    {eSoftwareScenario_Main2_Vfb_Stream_2,    "Main2_vFBStream2",       NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_FB,          NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Main2_Pure_Raw_Stream, "Main2_PureRawStream",    NSImageioIsp3::NSIspio::eDrvScenario_IP,                 NSImageioIsp3::NSIspio::EPipePass_PASS2C,    EPathCQ_CQ3},
    {eSoftwareScenario_Main2_CC_Raw_Stream,   "Main2_CCRawStream",      NSImageioIsp3::NSIspio::eDrvScenario_CC_RAW,             NSImageioIsp3::NSIspio::EPipePass_PASS2C,    EPathCQ_CQ3},
    {eSoftwareScenario_Main2_IP_Raw_TPipe_Stream, "Main2_IPRawTPipeStream",    NSImageioIsp3::NSIspio::eDrvScenario_IP_TPipe,    NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    //
    {eSoftwareScenario_Sub2_Normal_Stream,   "Sub2_NormalStream",     NSImageioIsp3::NSIspio::eDrvScenario_CC,                 NSImageioIsp3::NSIspio::EPipePass_PASS2C,    EPathCQ_CQ3},
    {eSoftwareScenario_Sub2_Normal_Capture,  "Sub2_NormalCapture",    NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2C,    EPathCQ_CQ3},
    {eSoftwareScenario_Sub2_VSS_Capture,     "Sub2_VSSCapture",       NSImageioIsp3::NSIspio::eDrvScenario_VSS,                NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Sub2_ZSD_Capture,     "Sub2_ZSDCapture",       NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub2_Mfb_Capture,     "Sub2_MFBCapture",       NSImageioIsp3::NSIspio::eDrvScenario_CC_SShot,           NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub2_Mfb_Blending,    "Sub2_MFBBlending",      NSImageioIsp3::NSIspio::eDrvScenario_CC_MFB_Blending,    NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub2_Mfb_Mixing,      "Sub2_MFBMixing",        NSImageioIsp3::NSIspio::eDrvScenario_CC_MFB_Mixing,      NSImageioIsp3::NSIspio::EPipePass_PASS2C,   EPathCQ_CQ3},
    {eSoftwareScenario_Sub2_VSS_Mfb_Capture,     "Sub2_MFBCapture_Vss",       NSImageioIsp3::NSIspio::eDrvScenario_VSS,           NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Sub2_VSS_Mfb_Blending,    "Sub2_MFBBlending_Vss",      NSImageioIsp3::NSIspio::eDrvScenario_VSS_MFB_Blending,    NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Sub2_VSS_Mfb_Mixing,      "Sub2_MFBMixing_Vss",        NSImageioIsp3::NSIspio::eDrvScenario_VSS_MFB_Mixing,      NSImageioIsp3::NSIspio::EPipePass_PASS2,   EPathCQ_CQ1},
    {eSoftwareScenario_Sub2_Vfb_Stream_1,    "Sub2_vFBStream1",       NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_Normal,      NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1},
    {eSoftwareScenario_Sub2_Vfb_Stream_2,    "Sub2_vFBStream2",       NSImageioIsp3::NSIspio::eDrvScenario_CC_vFB_FB,          NSImageioIsp3::NSIspio::EPipePass_PASS2B,   EPathCQ_CQ2},
    {eSoftwareScenario_Sub2_Pure_Raw_Stream, "Sub2_PureRawStream",    NSImageioIsp3::NSIspio::eDrvScenario_IP,                 NSImageioIsp3::NSIspio::EPipePass_PASS2C,    EPathCQ_CQ3},
    {eSoftwareScenario_Sub2_CC_Raw_Stream,   "Sub2_CCRawStream",      NSImageioIsp3::NSIspio::eDrvScenario_CC_RAW,             NSImageioIsp3::NSIspio::EPipePass_PASS2C,    EPathCQ_CQ3},
    {eSoftwareScenario_Sub2_IP_Raw_TPipe_Stream, "Sub2_IPRawTPipeStream",    NSImageioIsp3::NSIspio::eDrvScenario_IP_TPipe,    NSImageioIsp3::NSIspio::EPipePass_PASS2,    EPathCQ_CQ1}
};


PortIdxMappingAlltoP2 mPortIdxMappingAlltoP2[NSImageio::NSIspio::EPortIndex_VENC_STREAMO+1]=
{
    {NSImageio::NSIspio::EPortIndex_TG1I,   EPostProcPortIdx_NOSUP},            //0
    {NSImageio::NSIspio::EPortIndex_TG2I,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_TG1I,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_TG2I,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_LSCI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CQI,    EPostProcPortIdx_NOSUP},            //5
    {NSImageio::NSIspio::EPortIndex_IMGO,   EPostProcPortIdx_IMGO},
    {NSImageio::NSIspio::EPortIndex_UFEO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_RRZO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV_IMGO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_CAMSV2_IMGO, EPostProcPortIdx_NOSUP},       //10
    {NSImageio::NSIspio::EPortIndex_LCSO,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_AAO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_AFO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_PDO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_EISO,    EPostProcPortIdx_NOSUP},           //15
    {NSImageio::NSIspio::EPortIndex_FLKO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_RSSO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_PSO,    EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_IMGI,   EPostProcPortIdx_IMGI},
    {NSImageio::NSIspio::EPortIndex_IMGBI,   EPostProcPortIdx_NOSUP},           //20
    {NSImageio::NSIspio::EPortIndex_IMGCI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_VIPI,   EPostProcPortIdx_VIPI},
    {NSImageio::NSIspio::EPortIndex_VIP2I,  EPostProcPortIdx_VIP2I},
    {NSImageio::NSIspio::EPortIndex_VIP3I,  EPostProcPortIdx_VIP3I},
    {NSImageio::NSIspio::EPortIndex_UFDI,   EPostProcPortIdx_UFDI},             //25
    {NSImageio::NSIspio::EPortIndex_LCEI,   EPostProcPortIdx_LCEI},
    {NSImageio::NSIspio::EPortIndex_DMGI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_DEPI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_TDRI,   EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_IMG2O,  EPostProcPortIdx_IMG2O},            //30
    {NSImageio::NSIspio::EPortIndex_IMG2BO, EPostProcPortIdx_NOSUP},
    {NSImageio::NSIspio::EPortIndex_IMG3O,  EPostProcPortIdx_IMG3O},
    {NSImageio::NSIspio::EPortIndex_IMG3BO, EPostProcPortIdx_IMG3BO},
    {NSImageio::NSIspio::EPortIndex_IMG3CO, EPostProcPortIdx_IMG3CO},
    {NSImageio::NSIspio::EPortIndex_MFBO,   EPostProcPortIdx_MFBO},             //35
    {NSImageio::NSIspio::EPortIndex_FEO,    EPostProcPortIdx_FEO},
    {NSImageio::NSIspio::EPortIndex_WROTO,  EPostProcPortIdx_WROTO},
    {NSImageio::NSIspio::EPortIndex_WDMAO,  EPostProcPortIdx_WDMAO},
    {NSImageio::NSIspio::EPortIndex_JPEGO,  EPostProcPortIdx_JPEGO},
    {NSImageio::NSIspio::EPortIndex_VENC_STREAMO,  EPostProcPortIdx_VENC_STREAMO} //40
};

PortIdxMappingP2toAll mPortIdxMappingP2toAll[EPostProcPortIdx_NUM]=
{
    {EPostProcPortIdx_IMGI,     NSImageio::NSIspio::EPortIndex_IMGI},
    {EPostProcPortIdx_UFDI,     NSImageio::NSIspio::EPortIndex_UFDI},
    {EPostProcPortIdx_LCEI,     NSImageio::NSIspio::EPortIndex_LCEI},
    {EPostProcPortIdx_VIPI,     NSImageio::NSIspio::EPortIndex_VIPI},
    {EPostProcPortIdx_VIP2I,    NSImageio::NSIspio::EPortIndex_VIP2I},
    {EPostProcPortIdx_VIP3I,    NSImageio::NSIspio::EPortIndex_VIP3I},
    {EPostProcPortIdx_IMGO,     NSImageio::NSIspio::EPortIndex_IMGO},
    {EPostProcPortIdx_MFBO,     NSImageio::NSIspio::EPortIndex_MFBO},
    {EPostProcPortIdx_FEO,      NSImageio::NSIspio::EPortIndex_FEO},
    {EPostProcPortIdx_IMG3CO,   NSImageio::NSIspio::EPortIndex_IMG3CO},
    {EPostProcPortIdx_IMG3BO,   NSImageio::NSIspio::EPortIndex_IMG3BO},
    {EPostProcPortIdx_IMG3O,    NSImageio::NSIspio::EPortIndex_IMG3O},
    {EPostProcPortIdx_IMG2O,    NSImageio::NSIspio::EPortIndex_IMG2O},
    {EPostProcPortIdx_JPEGO,    NSImageio::NSIspio::EPortIndex_JPEGO},
    {EPostProcPortIdx_WROTO,    NSImageio::NSIspio::EPortIndex_WROTO},
    {EPostProcPortIdx_WDMAO,    NSImageio::NSIspio::EPortIndex_WDMAO},
    {EPostProcPortIdx_VENC_STREAMO,    NSImageio::NSIspio::EPortIndex_VENC_STREAMO}
};

#define TRANSFORM_MAX   8  //eTransform in ImageFormat.h
TransformMapping mTransformMapping[TRANSFORM_MAX]=
{
    {0,                 NSImageioIsp3::NSIspio::eImgRot_0,  NSImageioIsp3::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H, NSImageioIsp3::NSIspio::eImgRot_0,  NSImageioIsp3::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V, NSImageioIsp3::NSIspio::eImgRot_180,  NSImageioIsp3::NSIspio::eImgFlip_ON},
    {eTransform_ROT_180,NSImageioIsp3::NSIspio::eImgRot_180,NSImageioIsp3::NSIspio::eImgFlip_OFF},
    {eTransform_ROT_90, NSImageioIsp3::NSIspio::eImgRot_90, NSImageioIsp3::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H|eTransform_ROT_90, NSImageioIsp3::NSIspio::eImgRot_270,  NSImageioIsp3::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V|eTransform_ROT_90, NSImageioIsp3::NSIspio::eImgRot_90,  NSImageioIsp3::NSIspio::eImgFlip_ON},
    {eTransform_ROT_270,NSImageioIsp3::NSIspio::eImgRot_270,NSImageioIsp3::NSIspio::eImgFlip_OFF}
};

#define NAME "HalPipeWrapper"

MINT32 getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}

/******************************************************************************
 *
 ******************************************************************************/
HalPipeWrapper*
HalPipeWrapper::createInstance()
{
    return HalPipeWrapper::getInstance();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper::destroyInstance()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
}

/******************************************************************************
 *
 ******************************************************************************/
static HalPipeWrapper Singleton;
HalPipeWrapper*
HalPipeWrapper::
getInstance()
{
    LOG_DBG("&Singleton(0x%lx)\n",(unsigned long)(&Singleton));
    //
    return &Singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
HalPipeWrapper::
HalPipeWrapper()
    : mpPostProcPipe(NULL)
    , mVencPortCnt(0)
    , mCmdListCond()
    , mVssCond()
{
    mp2CQDupIdx[0]=0;
    mp2CQDupIdx[1]=0;
    mp2CQDupIdx[2]=0;
    mp2CQUserNum[0]=0;
    mp2CQUserNum[1]=0;
    mp2CQUserNum[2]=0;
    misV3=false;
    mIsVssOccupied=false;	// For Vss Concurrency Check
    LOG_INF("dupidx CQ1/CQ2/CQ3 (%d/%d/%d)",mp2CQDupIdx[0],mp2CQDupIdx[1],mp2CQDupIdx[2]);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
init(
    ESoftwareScenario swScenario,
    unsigned long & callerPostProc,
    MBOOL isV3,
    MUINT32 pixIdP2 //QQ

)
{
    DBG_LOG_CONFIG(iopipe, P2PipeWrapper);
    DBG_LOG_CONFIG(iopipe, P2PipeWrapper_Thread);
    FUNCTION_LOG_START;
    bool ret = true;
    MUINT32 mP2TuningQueBufferSize = 0;
    // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.
    Mutex::Autolock lock(mLock);

    //[1] increase user count and record user
    android_atomic_inc(&mInitCount);
    LOG_INF("mUCnt(%d),mUser(%s)",mInitCount,mSwHwPathMapping[swScenario].iHalPipeUser);
    if(strlen(mSwHwPathMapping[swScenario].iHalPipeUser)<1)
    {
        LOG_ERR("Plz add userName\n");
        return MFALSE;
    }
    mUserNameList.push_back(mSwHwPathMapping[swScenario].iHalPipeUser);

    //[2] map sw scenario to hw scenario
    //[3] create and init dedicated postproc pipe for halpipe user
    callerPostProc=(unsigned long)NSImageioIsp3::NSIspio::IPostProcPipe::createInstance(mSwHwPathMapping[swScenario].hwPath);
    if (NULL == callerPostProc || ! ((NSImageioIsp3::NSIspio::IPostProcPipe *)callerPostProc)->init())
    {
        LOG_ERR("caller PostProc init fail");
        return false;
    }

    //first user
    if(mInitCount==1)
    {
        //temp workaround, flush CQ descriptor
        ((NSImageioIsp3::NSIspio::IPostProcPipe *)callerPostProc)->sendCommand(0x1103,\
            mSwHwPathMapping[swScenario].ePathCQ + EDrvCQ_CQ01,0,0);
        ((NSImageioIsp3::NSIspio::IPostProcPipe *)callerPostProc)->sendCommand(0x1103,\
            mSwHwPathMapping[swScenario].ePathCQ + EDrvCQ_CQ01,0,1);
        //
        misV3=isV3;

        //[4]initialize variables
        for(int i=0;i<EPathCQ_NUM;i++)
        {
            mp2CQDupIdx[i]=0;
            mp2CQUserNum[i]=0;
        }
        sem_init(&mSemCQ1, 0, 0);
        sem_init(&mSemCQ2, 0, 0);
        sem_init(&mSemCQ3, 0, 0);
        sem_init(&mSemDequeThread, 0, 0);
        sem_init(&mSemVencCnt, 0, 0);
        sem_init(&mSemVssOccupied, 0, 0);	// For Vss Concurrency Check
        mIsVssOccupied=false;	// For Vss Concurrency Check
        for (list<BufParamPackage>::iterator it = mvDequeuedBufBatch.begin(); it != mvDequeuedBufBatch.end();)
        {
            //LOG_INF("cmd clear(%d)", *it);
            it = mvDequeuedBufBatch.erase(it);
            it++;
        }

        #if !defined(__PMEM_ONLY__)  //imem
    // create imem driver
        m_pIMemDrv = IMemDrv::createInstance();
        LOG_DBG("[m_pIMemDrv]:0x%08lx ",(unsigned long)(m_pIMemDrv));
        if ( NULL == m_pIMemDrv ) {
            LOG_ERR("IMemDrv::createInstance fail.");
            return -1;
        }
        m_pIMemDrv->init(NAME);
        #endif //!defined(__PMEM_ONLY__)

        //
        // initialize p2 tuningQue buffer
        mP2TuningQueBufferSize = sizeof(isp_reg_t);
        if(!gTuningQueNode.empty() || !gTuningQueData.empty()) {
            LOG_ERR("[Error]gTuningQueNode|gTuningQueData buffer is not empty!");
            return -1;
        } else {
            tuninQueData tuningQueDataNew[DEFAULT_P2_TUNING_QUE_BUFFER_NUM];
            //
            mTuningQueNum = DEFAULT_P2_TUNING_QUE_BUFFER_NUM;
            for(int i=0;i<DEFAULT_P2_TUNING_QUE_BUFFER_NUM;i++) {
                #if defined(__PMEM_ONLY__)
                    tuningQueDataNew[i].bufferInfo.virtAddr = (MUINT32*)pmem_alloc_sync(size, &gTuningQueData[i].bufferInfo.memID);
                    memset((MUINT8*)tuningQueDataNew[i].bufferInfo.virtAddr,0,size);
                #else
                    tuningQueDataNew[i].bufferInfo.size = mP2TuningQueBufferSize;
                    tuningQueDataNew[i].bufferInfo.useNoncache = 0; //alloc cacheable mem.
                    if ( m_pIMemDrv->allocVirtBuf(NAME, &tuningQueDataNew[i].bufferInfo) ) {
                        LOG_ERR("[ERROR]:m_pIMemDrv->allocVirtBuf");
                    }
                    memset((MUINT8*)tuningQueDataNew[i].bufferInfo.virtAddr,0,mP2TuningQueBufferSize);
                #endif
                LOG_INF("[TuningQue]i(%d),VA(0x%x),size(0x%x)\n",i,tuningQueDataNew[i].bufferInfo.virtAddr,mP2TuningQueBufferSize);
                gTuningQueData.push_back(tuningQueDataNew[i]);
                gTuningQueNode.push_back((void*)tuningQueDataNew[i].bufferInfo.virtAddr);
            }
        }

        //[5] create postproc pipe dedicated for dequeue buf
        mpPostProcPipe=NSImageioIsp3::NSIspio::IPostProcPipe::createInstance(NSImageioIsp3::NSIspio::eDrvScenario_CC);//no matter
        if (NULL == mpPostProcPipe || !mpPostProcPipe->init())
        {
            LOG_ERR("dequeue PostProc init fail");
            return false;
        }
        //[6] create dequeue thread
        createThread();
    }
    else
    {   //check version if more than 1 users
        if(misV3 != isV3)
        {
            LOG_ERR("wrong version mV3/isV3 (%d/%d)",misV3,isV3);
        }
    }

    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
uninit(
    ESoftwareScenario swScenario,
    NSImageioIsp3::NSIspio::IPostProcPipe * callerPostProc)
{
    FUNCTION_LOG_START;
    bool ret = true;
    Mutex::Autolock lock(mLock);
    //[1] decrease user count and record user
    android_atomic_dec(&mInitCount);
    LOG_INF("mUCnt(%d),mUser(%s)",mInitCount,mSwHwPathMapping[swScenario].iHalPipeUser);
    if(strlen(mSwHwPathMapping[swScenario].iHalPipeUser)==0)
    {
    }
    else
    {
        #if 0   //temp remove, need check
        mUserNameList.remove(mSwHwPathMapping[swScenario].iHalPipeUser);
        list<const char*>::iterator iter;
        char* userlist=(char*)malloc(128*sizeof(char));
        for (iter = mUserNameList.begin(); iter != mUserNameList.end(); ++iter)
        {
           strcat(userlist,*iter);
           strcat(userlist,",");
        }
        LOG_INF("HalPipeWrapper RestUsers:%s\n",userlist);
        free(userlist);
        #endif
    }

    //deq thread related variables are init only once
    if(mInitCount==0)
    {

        //[2] stop dequeue thread
        destroyThread();
        ::sem_wait(&mSemDequeThread);

        //[3] create postproc pipe dedicated for dequeue buf
        if (true != mpPostProcPipe->uninit())
        {
            LOG_ERR("PostProc uninit fail");
            ret = false;
        }

        mpPostProcPipe->destroyInstance();
        mpPostProcPipe = NULL;

        //[4]free vector variable
        vector<BufParamPackage>().swap(mvEnqueuedBufBatch); //force vector to relinquish its memory (look up "stl swap trick")
        // free p2 tuning Queue
        LOG_DBG("Free Tuning Queue");

        // check tuning que size
        if(mTuningQueNum != gTuningQueData.size()){
            LOG_ERR("[Error]gTuningQueData number(%d) is different from that be alloced(%d)",
                mTuningQueNum,gTuningQueData.size());
            return -1;
        }
        if(mTuningQueNum != gTuningQueNode.size()){
            LOG_ERR("[Error]gTuningQueNode number(%d) is different from that be alloced(%d)",
                mTuningQueNum,gTuningQueNode.size());
            return -1;
        }

        for(int i=0;i<mTuningQueNum;i++) {
            if((gTuningQueData[i].bufferInfo.memID>=0) && (gTuningQueData[i].bufferInfo.virtAddr!=0)){
                #if defined(__PMEM_ONLY__)
                    pmem_free((MUINT8*)gTuningQueData[i].bufferInfo.virtAddr,gTuningQueData[i].bufferInfo.size,gTuningQueData[i].bufferInfo.memID);
                    gTuningQueData[i].bufferInfo.virtAddr = NULL;
                    gTuningQueData[i].bufferInfo.memID = -1;
                    gTuningQueNode[i] = NULL;
                #else
                    if ( m_pIMemDrv->freeVirtBuf(NAME, &gTuningQueData[i].bufferInfo) ) {
                        LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
                    }
                    gTuningQueData[i].bufferInfo.virtAddr = NULL;
                    gTuningQueData[i].bufferInfo.memID = -1;
                    gTuningQueNode[i] = NULL;
                #endif
            } else {
                LOG_ERR("[Error]free TuningQue error i(%d),fd(%d),va(0x%08x)",i,gTuningQueData[i].bufferInfo.memID,gTuningQueData[i].bufferInfo.virtAddr);
            }
        }
        gTuningQueData.erase(gTuningQueData.begin(),gTuningQueData.end());
        gTuningQueNode.erase(gTuningQueNode.begin(),gTuningQueNode.end());
        LOG_DBG("[TuningQueSize]%d-%d",gTuningQueData.size(),gTuningQueNode.size());

        //destroy imem
        #if !defined(__PMEM_ONLY__)  //imem
        m_pIMemDrv->uninit(NAME);
        LOG_DBG("m_pIMemDrv->uninit()");
        m_pIMemDrv->destroyInstance();
        LOG_DBG("m_pIMemDrv->destroyInstance()");
        m_pIMemDrv = NULL;
        #endif  //!defined(__PMEM_ONLY__)

        //
        misV3=false;

        sem_destroy(&mSemVssOccupied);	// For Vss Concurrency Check
    }

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MVOID
HalPipeWrapper::
getLock(
    ELockEnum lockType)
{
	LOG_DBG("getLock");

    switch(lockType)
    {
        case ELockEnum_CQUser:
            pthread_mutex_lock(&CQUserNumMutex);
            break;
        case ELockEnum_BufBatchEnQ:
            pthread_mutex_lock(&BufBatchEnQMutex);
            break;
        case ELockEnum_BufBatchDeQ:
            pthread_mutex_lock(&BufBatchDeQMutex);
            break;
        case ELockEnum_BufNodeEnQ:
            pthread_mutex_lock(&BufNodeEnQMutex);
            break;
        case ELockEnum_VencCnt:
            pthread_mutex_lock(&VencCnt_Mutex);
            break;
    	case ELockEnum_VssOccupied:	// For Vss Concurrency Check
    		pthread_mutex_lock(&VssOccupied_Mutex);
    		break;
        case ELockEnum_MultiFrameEnQ:
            pthread_mutex_lock(&MultiframeEnQMutex);
            break;
        default:
            break;
    }
}

/******************************************************************************
*
******************************************************************************/
MVOID
HalPipeWrapper::
releaseLock(
    ELockEnum lockType)
{
    switch(lockType)
    {
        case ELockEnum_CQUser:
            pthread_mutex_unlock(&CQUserNumMutex);
            break;
        case ELockEnum_BufBatchEnQ:
            pthread_mutex_unlock(&BufBatchEnQMutex);
            break;
        case ELockEnum_BufBatchDeQ:
            pthread_mutex_unlock(&BufBatchDeQMutex);
            break;
        case ELockEnum_BufNodeEnQ:
            pthread_mutex_unlock(&BufNodeEnQMutex);
            break;
        case ELockEnum_VencCnt:
            pthread_mutex_unlock(&VencCnt_Mutex);
            break;
    	case ELockEnum_VssOccupied:	// For Vss Concurrency Check
    		pthread_mutex_unlock(&VssOccupied_Mutex);
    		break;
        case ELockEnum_MultiFrameEnQ:
            pthread_mutex_unlock(&MultiframeEnQMutex);
            break;
        default:
            break;
    }
	LOG_DBG("releaseLock");
}


/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
checkCQEnqueuedUserNum(
    MINT32 p2cq)
{
    getLock(ELockEnum_CQUser);
    if(mp2CQUserNum[p2cq]==MAX_ENQUEUE_USER_PERCQ)
    {
        releaseLock(ELockEnum_CQUser);
        return true;
    }
    else
    {
        releaseLock(ELockEnum_CQUser);
        return false;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
enque(
    ESoftwareScenario swScenario,
    MINT32 pixID,
    NSCam::NSIoPipe::QParams const& rParams,
    NSImageioIsp3::NSIspio::IPostProcPipe * callerPostProc,
    vector<NSCam::NSIoPipe::QParams>& callerDeQList,
    EBufferTag bufferTag,
    QFeatureData& rFeatureData)
{

    //FUNCTION_LOG_START;
    Mutex::Autolock lock(mEnqueLock);
    bool ret = false;
    BufParamPackage bufPackage;
    int pathCQ=mSwHwPathMapping[swScenario].ePathCQ;

    //[1] block if meet the max enqueue user for the CQ)
    while(checkCQEnqueuedUserNum(pathCQ))
    {
        switch (pathCQ)
        {
            case EPathCQ_CQ1:
                LOG_INF("Block CQ1 Enqueue Here");
                ::sem_wait(&mSemCQ1);
                break;
            case EPathCQ_CQ2:
                LOG_INF("Block CQ2 Enqueue Here");
                ::sem_wait(&mSemCQ2);
                break;
            case EPathCQ_CQ3:
                LOG_INF("Block CQ3 Enqueue Here");
                ::sem_wait(&mSemCQ3);
                break;
            default:
                break;
        }
    }

	LOG_DBG("sem_wait done");

    //[2] using cq and duplicate index
    switch(pathCQ)
    {
        case EPathCQ_CQ1:
            bufPackage.p2cqIdx=EDrvCQ_CQ01;
            break;
        case EPathCQ_CQ2:
            bufPackage.p2cqIdx=EDrvCQ_CQ02;
            break;
        case EPathCQ_CQ3:
            bufPackage.p2cqIdx=EDrvCQ_CQ03;
            break;
        default:
            LOG_ERR("no support this p2 path(%d)",pathCQ);
            break;
    }
    getLock(ELockEnum_CQUser);
    bufPackage.p2cqDupIdx=mp2CQDupIdx[pathCQ];
    releaseLock(ELockEnum_CQUser);


    //judge buffer package number in a enque request
    int bufPackageNum=0;
    if(misV3)
    {
        //bufPackageNum=rParams.mvTuningData.size();
        bufPackageNum=rParams.mvFrameParams.size();
    }
	#if (IP_BASE_ISP_3 == 0)	// IP base no V1
    else
    {
        bufPackageNum=rParams.mvMagicNo.size();
    }
    #endif
	bufPackage.frameNum = bufPackageNum;
    //LOG_INF("bufPackageNum(%d),misV3(%d),magic(%d),tuningData(%d)",bufPackageNum,misV3,rParams.mvMagicNo.size(),rParams.mvTuningData.size());
    LOG_INF("bufPackageNum(%d),misV3(%d),tuningData(%d)",bufPackageNum,misV3,rParams.mvFrameParams.size());
    //

    /////////////////////////////////////
    //only v1 support non-ventor enque
    /////////////////////////////////////
    int pixelInByte=1;
	// IP base no V1
    {   //enque by vector, v3 and v1 [v3 only support vector enque]
        //[3]judge that the number of enque buffer batches exceeds supported number or not

        //[4]parse the seperately number of in/out ports in each buffer
        MINT32 planeNum=0;
        EPostProcPortIdx p2PortIdx=EPostProcPortIdx_NOSUP;
        int inPorts=0,outPorts=0;
        MINT32 curgroup=0;
        int tmpC=0;
        int inCnt=0,outCnt=0;
		//error handle
		if(rParams.mvFrameParams.size()<1)
		{
			LOG_ERR("unexpected frameParas size(%lu)", (unsigned long)rParams.mvFrameParams.size());
			return MFALSE;
		}

	    for(int q=0;q<bufPackageNum;q++)
        {
        	inPorts += rParams.mvFrameParams[q].mvIn.size();
        	outPorts += rParams.mvFrameParams[q].mvOut.size();
        }

        vector<NSImageioIsp3::NSIspio::PortInfo const*> vPostProcInPorts;
        vector<NSImageioIsp3::NSIspio::PortInfo const*> vPostProcOutPorts;
        NSImageioIsp3::NSIspio::PortInfo ports[100];
        //LOG_INF("batch(%d),in/out(%d/%d)",bufPackageNum,inPorts,outPorts);

        //[5]sequencely parse buffer data
        int cnt=0;
        MUINT32 dmaEnPort=0x0;
        int crop1StartIdx=0;
        MBOOL getcrop1=MFALSE;
        std::vector<NSImageioIsp3::NSIspio::ModuleParaInfo> mvModule;
        NSImageioIsp3::NSIspio::PipePackageInfo pipePackageInfo;
        MVOID* pTuningQue=NULL;

        if (bufPackageNum >= 2) {
            getLock(ELockEnum_MultiFrameEnQ);
        }

        for(int q=0;q<bufPackageNum;q++)
        {
            vPostProcInPorts.resize(rParams.mvFrameParams[q].mvIn.size());	  //in/out dma for each frame unit
            vPostProcOutPorts.resize(rParams.mvFrameParams[q].mvOut.size());
            inCnt=0;
            outCnt=0;
            dmaEnPort=0x0;
            getcrop1=MFALSE;
            //mvin
            for(MUINT32 i=0;i< rParams.mvFrameParams[q].mvIn.size() ;i++)
	        {
	            ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]=0;
	            planeNum = rParams.mvFrameParams[q].mvIn[i].mBuffer->getPlaneCount();
	            p2PortIdx=(mPortIdxMappingAlltoP2[rParams.mvFrameParams[q].mvIn[i].mPortID.index].eP2PortIdx);
	            //LOG_INF("mPortID [%d].index: %d, p2PortIdx: %d", i, rParams.mvFrameParams[q].mvIn[i].mPortID.index, p2PortIdx);
	            if(p2PortIdx == EPostProcPortIdx_NOSUP)
	            {
	                LOG_ERR("not support this port: %d",rParams.mvFrameParams[q].mvIn[i].mPortID.index); //using maping string for debug?
	                return -1;
	            }
	            else
	            {
	                //if user pass-in vipi~vip3i, we would record vipi only
	                dmaEnPort |= (1 << p2PortIdx);
	            }
	            //actually, only imgi need to set crop information
	            ports[cnt].u4ImgWidth  = rParams.mvFrameParams[q].mvIn[i].mBuffer->getImgSize().w;
	            ports[cnt].u4ImgHeight = rParams.mvFrameParams[q].mvIn[i].mBuffer->getImgSize().h;

                switch(rParams.mvFrameParams[q].mvIn[i].mPortID.index)
                {
                    case NSImageio::NSIspio::EPortIndex_IMGI:
                        // MW might assign different sensorID/pxlID per frameParams in one QParams.
                        pixID = pixelIDMapping(rParams.mvFrameParams[q].mvIn[i].mBuffer->getColorArrangement());

                        if(rParams.mvFrameParams[q].mvCropRsInfo.size() >0)
                        {
                            ports[cnt].crop1.x      = 0;
                            ports[cnt].crop1.floatX = 0;
                            ports[cnt].crop1.y      = 0;
                            ports[cnt].crop1.floatY = 0;
                            ports[cnt].crop1.w      = 0;
                            ports[cnt].crop1.h      = 0;
                            ports[cnt].resize1.tar_w = 0;
                            ports[cnt].resize1.tar_h = 0;
                            ports[cnt].crop2.x = 0;
                            ports[cnt].crop2.floatX = 0;
                            ports[cnt].crop2.y = 0;
                            ports[cnt].crop2.floatY = 0;
                            ports[cnt].crop2.w      = 0;
                            ports[cnt].crop2.h      = 0;
                            ports[cnt].resize2.tar_w = 0;
                            ports[cnt].resize2.tar_h = 0;
                            ports[cnt].crop3.x = 0;
                            ports[cnt].crop3.floatX = 0;
                            ports[cnt].crop3.y = 0;
                            ports[cnt].crop3.floatY = 0;
                            ports[cnt].crop3.w      = 0;
                            ports[cnt].crop3.h      = 0;
                            ports[cnt].resize3.tar_w = 0;
                            ports[cnt].resize3.tar_h = 0;
			                // add for crop4 +++++
			                ports[cnt].crop4.x = 0;
			                ports[cnt].crop4.floatX = 0;
			                ports[cnt].crop4.y = 0;
			                ports[cnt].crop4.floatY = 0;
			                ports[cnt].crop4.w      = 0;
			                ports[cnt].crop4.h      = 0;
			                ports[cnt].resize4.tar_w = 0;
			                ports[cnt].resize4.tar_h = 0;
			                // add for crop4 -----
                            for(int k=crop1StartIdx;k<rParams.mvFrameParams[q].mvCropRsInfo.size();k++)
                            {
                                //crop1/crop2 may be different in each buffer node
                                //asume user push crop information by the order crop1/crop2
                                switch(rParams.mvFrameParams[q].mvCropRsInfo[k].mGroupID)
                                {
                                    case 1:
                                        if(getcrop1)
                                        {
                                            //we get crop1 of next buffer node, should jump out
                                            crop1StartIdx=k;
                                            k=rParams.mvFrameParams[q].mvCropRsInfo.size();
                                            break;
                                        }
                                        ports[cnt].crop1.x      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.x;
                                        ports[cnt].crop1.floatX = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.x;
                                        ports[cnt].crop1.y      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.y;
                                        ports[cnt].crop1.floatY = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.y;
                                        ports[cnt].crop1.w      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w;
                                        ports[cnt].crop1.h      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h;
                                        ports[cnt].resize1.tar_w = (rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w == 0)?(rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w):(rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w);
                                        ports[cnt].resize1.tar_h = (rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h == 0)?(rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h):(rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h);
                                        getcrop1=MTRUE;
                                        ports[cnt].crop2.x      = 0;
                                        ports[cnt].crop2.floatX = 0;
                                        ports[cnt].crop2.y      = 0;
                                        ports[cnt].crop2.floatY = 0;
                                        ports[cnt].crop2.w      = ports[cnt].crop1.w;
                                        ports[cnt].crop2.h      = ports[cnt].crop1.h;
                                        ports[cnt].resize2.tar_w = ports[cnt].crop1.w;
                                        ports[cnt].resize2.tar_h = ports[cnt].crop1.h;
                                        ports[cnt].crop3.x      = 0;
                                        ports[cnt].crop3.floatX = 0;
                                        ports[cnt].crop3.y      = 0;
                                        ports[cnt].crop3.floatY = 0;
                                        ports[cnt].crop3.w      = ports[cnt].crop1.w;
                                        ports[cnt].crop3.h      = ports[cnt].crop1.h;
                                        ports[cnt].resize3.tar_w = ports[cnt].crop1.w;
                                        ports[cnt].resize3.tar_h = ports[cnt].crop1.h;
                                        break;
                                    case 2:
                                        ports[cnt].crop2.x = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.x;
                                        ports[cnt].crop2.floatX = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.x;
                                        ports[cnt].crop2.y = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.y;
                                        ports[cnt].crop2.floatY = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.y;
                                        ports[cnt].crop2.w      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w;
                                        ports[cnt].crop2.h      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h;
                                        ports[cnt].resize2.tar_w = (rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w == 0)?(rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w):(rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w);
                                        ports[cnt].resize2.tar_h = (rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h == 0)?(rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h):(rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h);
                                        break;
                                   case 3:
                                        ports[cnt].crop3.x = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.x;
                                        ports[cnt].crop3.floatX = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.x;
                                        ports[cnt].crop3.y = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.y;
                                        ports[cnt].crop3.floatY = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.y;
                                        ports[cnt].crop3.w      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w;
                                        ports[cnt].crop3.h      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h;
                                        ports[cnt].resize3.tar_w = (rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w == 0)?(rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w):(rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w);
                                        ports[cnt].resize3.tar_h = (rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h == 0)?(rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h):(rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h);
                                        break;
			                            // add for crop4 +++++
			                       case 4:
			                            ports[cnt].crop4.x = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.x;
			                            ports[cnt].crop4.floatX = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.x;
			                            ports[cnt].crop4.y = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_integral.y;
			                            ports[cnt].crop4.floatY = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.p_fractional.y;
			                            ports[cnt].crop4.w      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w;
			                            ports[cnt].crop4.h      = rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h;
			                            ports[cnt].resize4.tar_w = (rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w == 0)?(rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.w):(rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.w);
			                            ports[cnt].resize4.tar_h = (rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h == 0)?(rParams.mvFrameParams[q].mvCropRsInfo[k].mCropRect.s.h):(rParams.mvFrameParams[q].mvCropRsInfo[k].mResizeDst.h);
			                            break;
			                            // add for crop4 -----
                                    default:
                                        LOG_ERR("DO not support crop group (%d)",rParams.mvFrameParams[q].mvCropRsInfo[k].mGroupID);
                                        break;
                                }
                            }
                        }
                        else
                        {
                            LOG_ERR("MvCropRsInfo is Empty!!");
                        }
                        LOG_DBG("[Crop] g1 x/fx/y/fy/w/h/tw/th(%d/%d/%d/%d/%d/%d/%d/%d),g2 x/fx/y/fy/w/h/tw/th(%d/%d/%d/%d/%d/%d/%d/%d),g2 x/fx/y/fy/w/h/tw/th(%d/%d/%d/%d/%d/%d/%d/%d)",\
                            ports[cnt].crop1.x,ports[cnt].crop1.floatX,ports[cnt].crop1.y,ports[cnt].crop1.floatY,ports[cnt].crop1.w,ports[cnt].crop1.h,ports[cnt].resize1.tar_w,\
                            ports[cnt].resize1.tar_h,ports[cnt].crop2.x,ports[cnt].crop2.floatX,ports[cnt].crop2.y,ports[cnt].crop2.floatY,ports[cnt].crop2.w,ports[cnt].crop2.h,\
                                    ports[cnt].resize2.tar_w,ports[cnt].resize2.tar_h,ports[cnt].crop3.x,ports[cnt].crop3.floatX,ports[cnt].crop3.y,ports[cnt].crop3.floatY,ports[cnt].crop3.w,ports[cnt].crop3.h,\
                                    ports[cnt].resize3.tar_w,ports[cnt].resize3.tar_h);
					    LOG_DBG("[Crop] g4 x/fx/y/fy/w/h/tw/th(%d/%d/%d/%d/%d/%d/%d/%d)",ports[cnt].crop4.x,ports[cnt].crop4.floatX,ports[cnt].crop4.y,ports[cnt].crop4.floatY,ports[cnt].crop4.w,ports[cnt].crop4.h,ports[cnt].resize4.tar_w,ports[cnt].resize4.tar_h);	// add for crop4
		        		break;
                    default:
                        break;
                }
                //
                ports[cnt].eImgFmt     = (EImageFormat)(rParams.mvFrameParams[q].mvIn[i].mBuffer->getImgFormat());
                ports[cnt].type        = mPortTypeMapping[rParams.mvFrameParams[q].mvIn[i].mPortID.type].eImgIOPortType;
                ports[cnt].index       = rParams.mvFrameParams[q].mvIn[i].mPortID.index; //need map to index defined in imageio
                ports[cnt].inout       = rParams.mvFrameParams[q].mvIn[i].mPortID.inout;
                ports[cnt].capbility   = rParams.mvFrameParams[q].mvIn[i].mPortID.capbility;
                ports[cnt].eImgRot     = mTransformMapping[rParams.mvFrameParams[q].mvIn[i].mTransform].eImgRot;
                ports[cnt].eImgFlip    = mTransformMapping[rParams.mvFrameParams[q].mvIn[i].mTransform].eImgFlip;
                ports[cnt].pipePass    = mSwHwPathMapping[swScenario].ePipePass;
                for(int k=0;k<planeNum;k++)
                {
                    ports[cnt].u4Stride[k] = rParams.mvFrameParams[q].mvIn[i].mBuffer->getBufStridesInBytes(k);
                    ports[cnt].u4BufSize[k]= rParams.mvFrameParams[q].mvIn[i].mBuffer->getBufSizeInBytes(k);
                    ports[cnt].u4BufVA[k]  = rParams.mvFrameParams[q].mvIn[i].mBuffer->getBufVA(k);
                    ports[cnt].u4BufPA[k]  = rParams.mvFrameParams[q].mvIn[i].mBuffer->getBufPA(k);
                    //ports[cnt].memID[k]    = rParams.mvFrameParams[q].mvIn[i].mBuffer->getFD(k);
                }
                //Note, Only support offset of vipi/img3o for nr3d (tpipe calculation, and nr3d + eis needed)
                //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
                switch(ports[cnt].eImgFmt)
                {
                    case eImgFmt_YUY2:
                        pixelInByte=2;
                        break;
                    case eImgFmt_YV12:
                    default:
                        pixelInByte=1;
                        break;
                }
                ports[cnt].xoffset = (rParams.mvFrameParams[q].mvIn[i].mBuffer->getExtOffsetInBytes(0) % ports[cnt].u4Stride[0])/pixelInByte;//dma x-offset for tile calculation
                ports[cnt].yoffset = rParams.mvFrameParams[q].mvIn[i].mBuffer->getExtOffsetInBytes(0) / ports[cnt].u4Stride[0];//dma y-offset for tile calculation
                if(misV3)
                {
                    ports[cnt].u4MagicNum=0;
                }
#if (IP_BASE_ISP_3 == 0)
                else
                {
                    ports[cnt].u4MagicNum  = rParams.mvMagicNo[q];
                }
#endif
                LOG_DBG("[mvIn (%d)] fmt(%d),w/h(%d/%d),oft(%d/%d/%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                    ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,rParams.mvFrameParams[q].mvIn[i].mBuffer->getExtOffsetInBytes(0),\
                    ports[cnt].xoffset,ports[cnt].yoffset,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                    ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                    ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);
                vPostProcInPorts.at(inCnt)=&(ports[cnt]);
                cnt++;
                inCnt++;
            }
            //mvout
            for(MUINT32 i=0;i< rParams.mvFrameParams[q].mvOut.size() ;i++)
            {
                ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]=0;
                planeNum = rParams.mvFrameParams[q].mvOut[i].mBuffer->getPlaneCount();
                //
                p2PortIdx=(mPortIdxMappingAlltoP2[rParams.mvFrameParams[q].mvOut[i].mPortID.index].eP2PortIdx);
                //LOG_INF("mvOut mPortID.index: %d, p2PortIdx: %d",rParams.mvFrameParams[q].mvOut[i].mPortID.index, p2PortIdx);
                if(p2PortIdx == EPostProcPortIdx_NOSUP)
                {
                    LOG_ERR("not support this port: %d",rParams.mvFrameParams[q].mvOut[i].mPortID.index);
                    return -1;
                }
                else
                {
                    //if user pass-in img3o~img3co, we would record img3o only
                    dmaEnPort |= (1 << p2PortIdx);
                }
                if(p2PortIdx==EPostProcPortIdx_VENC_STREAMO)
                {
                    getLock(ELockEnum_VencCnt);
                    mVencPortCnt++;
                    LOG_DBG("e mVencPortCnt(%d)",mVencPortCnt);
                    releaseLock(ELockEnum_VencCnt);
                }
                //
                ports[cnt].eImgFmt     = (EImageFormat)(rParams.mvFrameParams[q].mvOut[i].mBuffer->getImgFormat());
                if((rParams.mvFrameParams[q].mvOut[i].mPortID.index) == NSImageio::NSIspio::EPortIndex_JPEGO)
                {   //always send jpg 422 to mdp if direct link to jpeg
                    ports[cnt].eImgFmt=eImgFmt_JPG_I422;
                }
                ports[cnt].type        = mPortTypeMapping[rParams.mvFrameParams[q].mvOut[i].mPortID.type].eImgIOPortType;//NSImageio::NSIspio::EPortType_Memory;
                ports[cnt].index       = rParams.mvFrameParams[q].mvOut[i].mPortID.index; //need map to index defined in imageio
                ports[cnt].inout       = rParams.mvFrameParams[q].mvOut[i].mPortID.inout;
                ports[cnt].capbility   = rParams.mvFrameParams[q].mvOut[i].mPortID.capbility;
                ports[cnt].eImgRot     = mTransformMapping[rParams.mvFrameParams[q].mvOut[i].mTransform].eImgRot;
                ports[cnt].eImgFlip    = mTransformMapping[rParams.mvFrameParams[q].mvOut[i].mTransform].eImgFlip;
                ports[cnt].pipePass    = mSwHwPathMapping[swScenario].ePipePass;
                ports[cnt].u4ImgWidth  = rParams.mvFrameParams[q].mvOut[i].mBuffer->getImgSize().w;
                ports[cnt].u4ImgHeight = rParams.mvFrameParams[q].mvOut[i].mBuffer->getImgSize().h;
				//ports[cnt].bIsRwbEn	   = rParams.mvFrameParams[q].mvOut[i].bIsRwbEn;	// add for RWB sensor
				//LOG_DBG("[3]ports[%d].bIsRwbEn = %d, rParams.mvFrameParams[q].mvOut[%d].bIsRwbEn = %d", cnt, ports[cnt].bIsRwbEn, i, rParams.mvFrameParams[q].mvOut[i].bIsRwbEn);	// add for RWB sensor
                for(int k=0;k<planeNum;k++)
                {
                    ports[cnt].u4Stride[k] = rParams.mvFrameParams[q].mvOut[i].mBuffer->getBufStridesInBytes(k);
                    ports[cnt].u4BufSize[k]= rParams.mvFrameParams[q].mvOut[i].mBuffer->getBufSizeInBytes(k);
                    ports[cnt].u4BufVA[k]  = rParams.mvFrameParams[q].mvOut[i].mBuffer->getBufVA(k);
                    ports[cnt].u4BufPA[k]  = rParams.mvFrameParams[q].mvOut[i].mBuffer->getBufPA(k);
                    //ports[cnt].memID[k]    = rParams.mvFrameParams[q].mvOut[i].mBuffer->getFD(k);
                }
                //Note, Only support offset of vipi/img3o for nr3d (tpipe calculation, and nr3d + eis needed)
                //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
                switch(ports[cnt].eImgFmt)
                {
                    case eImgFmt_YUY2:
                        pixelInByte=2;
                        break;
                    case eImgFmt_YV12:
                    default:
                        pixelInByte=1;
                        break;
                }

                ports[cnt].xoffset = (rParams.mvFrameParams[q].mvOut[i].mBuffer->getExtOffsetInBytes(0) % ports[cnt].u4Stride[0])/pixelInByte;//dma x-offset for tile calculation
                ports[cnt].yoffset = rParams.mvFrameParams[q].mvOut[i].mBuffer->getExtOffsetInBytes(0) / ports[cnt].u4Stride[0];//dma y-offset for tile calculation
                //temp test(work around)
                switch(planeNum)
                {
                    case 2:
                        if(ports[cnt].u4BufPA[1]==ports[cnt].u4BufPA[0])
                        {
                            ports[cnt].u4BufPA[1]=ports[cnt].u4BufPA[0]+(ports[cnt].u4BufVA[1]-ports[cnt].u4BufVA[0]);
                        }
                        break;
                    case 3:
                        if((ports[cnt].u4BufPA[2]==ports[cnt].u4BufPA[1])&&(ports[cnt].u4BufPA[1]==ports[cnt].u4BufPA[0]))
                        {
                            ports[cnt].u4BufPA[1]=ports[cnt].u4BufPA[0]+(ports[cnt].u4BufVA[1]-ports[cnt].u4BufVA[0]);
                            ports[cnt].u4BufPA[2]=ports[cnt].u4BufPA[1]+(ports[cnt].u4BufVA[2]-ports[cnt].u4BufVA[1]);
                        }
                        break;
                    default:
                        break;
                }
                //
                if(misV3)
                {
                    ports[cnt].u4MagicNum = 0;
                }
#if (IP_BASE_ISP_3 == 0)
                else
                {
                ports[cnt].u4MagicNum  = rParams.mvMagicNo[q];
                }
#endif
                LOG_DBG("[mvOut (%d)] fmt(%d),w/h(%d/%d),oft(%d/%d/%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                    ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,rParams.mvFrameParams[q].mvOut[i].mBuffer->getExtOffsetInBytes(0),\
                    ports[cnt].xoffset,ports[cnt].yoffset,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                    ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                    ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);

                vPostProcOutPorts.at(outCnt)=&(ports[cnt]);
                cnt++;
                outCnt++;
            }

            ////////////////////////////////////////////////////////////////
            //all the additional feature data
            MBOOL isModuleSetting=MFALSE;
            MUINT32 featureEntag=0x0;
			MINT32 moduletag=0;
            for(int i=0;i<rParams.mvFrameParams[q].mvModuleData.size();i++)
            {
                moduletag=rParams.mvFrameParams[q].mvModuleData[i].moduleTag;
                featureEntag |= moduletag;
                switch(moduletag)
                {
                    case NSImageioIsp3::NSIspio::EP2Module_STA_FEO:
                    case NSImageioIsp3::NSIspio::EP2Module_STA_LCEI:
                        isModuleSetting=MFALSE;
                        break;
                    case NSImageioIsp3::NSIspio::EP2Module_NR3D:
                    case NSImageioIsp3::NSIspio::EP2Module_SRZ1:
                        isModuleSetting=MTRUE;
                        break;
                    default:
                        break;
                }
                //
                if(isModuleSetting)
                {
                    LOG_DBG("[mvModule] moduleEnum(%d)",moduletag);
                    NSImageioIsp3::NSIspio::ModuleParaInfo module;
                    module.eP2module=(NSImageioIsp3::NSIspio::EP2Module)(rParams.mvFrameParams[q].mvModuleData[i].moduleTag);
                    module.moduleStruct=rParams.mvFrameParams[q].mvModuleData[i].moduleStruct;
                    mvModule.push_back(module);
                }
                else
                {
                    ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]=0;
                    p2PortIdx=(mPortIdxMappingAlltoP2[reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->port_idx].eP2PortIdx);
                    if(p2PortIdx == EPostProcPortIdx_NOSUP)
                    {
                        LOG_ERR("not support this port: %d",reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->port_idx); //using maping string for debug?
                        return -1;
                    }
                    else
                    {
                        bufPackage.portEn |= (1 << p2PortIdx);
                    }
                    ports[cnt].eImgFmt     = eImgFmt_BAYER8;
                    ports[cnt].u4ImgWidth  = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->w;//rFeatureData.feoData.w;
                    ports[cnt].u4ImgHeight = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->h;//rFeatureData.feoData.h;
                    ports[cnt].u4Stride[0] = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->stride;//rFeatureData.feoData.stride;//stride in bytes
                    ports[cnt].u4Stride[1] = 0;
                    ports[cnt].u4Stride[2] = 0;
                    ports[cnt].type        = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->port_type;//rFeatureData.feoData.portID.type;
                    ports[cnt].index       = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->port_idx;//rFeatureData.feoData.portID.index;
                    ports[cnt].inout       = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->port_inout;//rFeatureData.feoData.portID.inout;
                    ports[cnt].eImgRot     = NSImageioIsp3::NSIspio::eImgRot_0; //no support rotate
                    ports[cnt].eImgFlip    = NSImageioIsp3::NSIspio::eImgFlip_OFF; //no support fliip
                    ports[cnt].pipePass    = mSwHwPathMapping[swScenario].ePipePass;
                    ports[cnt].u4BufVA[0]  = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->bufInfo.virtAddr;//rFeatureData.feoData.bufInfo.virtAddr;
                    ports[cnt].u4BufPA[0]  = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->bufInfo.phyAddr;//rFeatureData.feoData.bufInfo.phyAddr;
                    ports[cnt].memID[0]    = reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->bufInfo.memID;//rFeatureData.feoData.bufInfo.memID;
                    ports[cnt].u4BufSize[0]= reinterpret_cast<StaDataCfg*>(rParams.mvFrameParams[q].mvModuleData[i].moduleStruct)->bufInfo.size;//rFeatureData.feoData.bufInfo.size;
                    ports[cnt].u4MagicNum  = 0;//rParams.mFrameNo;

                    //
                    switch(moduletag)
                    {
                        case NSImageioIsp3::NSIspio::EP2Module_STA_FEO:
                            LOG_DBG("[mvOut (%d)] fmt(%d),w(%d),h(%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                                ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                                ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                                ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);
                            vPostProcOutPorts.at(outCnt)=&(ports[cnt]);
                            cnt++;
                            outCnt++;
                            break;
                        case NSImageioIsp3::NSIspio::EP2Module_STA_LCEI:
                            LOG_DBG("[mvIn (%d)] fmt(%d),w(%d),h(%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                                ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                                ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                                ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);
                            vPostProcInPorts.at(inCnt)=&(ports[cnt]);
                            cnt++;
                            inCnt++;
                            break;
                        default:
                            break;
                    }
                }
            }

            LOG_DBG("dmaEnPort(0x%08x),featureEntag (0x%x),q(%d)",dmaEnPort,featureEntag,q);

            // process p2 tuning setting only for camera3.0
            if(misV3){
                if(rParams.mvFrameParams[q].mTuningData==0)
				{
                	LOG_WRN("tuning buffer empty");
                }
				else
				{
                 	pTuningQue = rParams.mvFrameParams[q].mTuningData;
                 	LOG_DBG("q(%d),pTuningQue(0x%lx)",q,(unsigned long)(pTuningQue));
                }
#if (IP_BASE_ISP_3 == 0)
                //pass1 data
                if(rParams.mvP1SrcCrop.size()==0 || rParams.mvP1Dst.size()==0 || rParams.mvP1DstCrop.size()==0)
				{
                	LOG_ERR("[Error]PLZ add mvP1SrcCrop/mvP1Dst/mvP1DstCrop inv3");
                	return MFALSE;
            	}
				else
#endif
				{
					// In ISP4.0, these parameters are controlled in Tuning Buffer by Tuning Team
                    pipePackageInfo.mP1PrivateData.ResizeSrcW = 0;//rParams.mvP1SrcCrop[q].s.w;
                    pipePackageInfo.mP1PrivateData.ResizeSrcH = 0;//rParams.mvP1SrcCrop[q].s.h;
                    pipePackageInfo.mP1PrivateData.ResizeDstW = 0;//rParams.mvP1Dst[q].w;
                    pipePackageInfo.mP1PrivateData.ResizeDstH = 0;//rParams.mvP1Dst[q].h;
                    pipePackageInfo.mP1PrivateData.ResizeCropX = 0;//rParams.mvP1DstCrop[q].p.x;
                    pipePackageInfo.mP1PrivateData.ResizeCropY = 0;//rParams.mvP1DstCrop[q].p.y;

                    LOG_DBG("mP1PrivateData, crop(%d,%d),src(%d,%d),dst(%d,%d)", \
                    pipePackageInfo.mP1PrivateData.ResizeCropX, pipePackageInfo.mP1PrivateData.ResizeCropY, \
                    pipePackageInfo.mP1PrivateData.ResizeSrcW,pipePackageInfo.mP1PrivateData.ResizeSrcH, \
                    pipePackageInfo.mP1PrivateData.ResizeDstW,pipePackageInfo.mP1PrivateData.ResizeDstH);
                }
            }
            pipePackageInfo.burstQIdx=q;
            pipePackageInfo.dupCqIdx=bufPackage.p2cqDupIdx;
            pipePackageInfo.vModuleParams=mvModule;
            pipePackageInfo.pPrivateData= NULL;//rParams.mvPrivaData[q];
            pipePackageInfo.pTuningQue = pTuningQue;
            pipePackageInfo.isV3=misV3;
			pipePackageInfo.pixIdP2 = pixID;
            pipePackageInfo.drvScen = mSwHwPathMapping[swScenario].hwPath;
            pipePackageInfo.pExtraParam=&(rParams.mvFrameParams[q].mvExtraParam);
            // No iso value in QParam
			//pipePackageInfo.isoValue = rParams.isoValue;	// set ISO
			//LOG_DBG("set iso(HalPipeWrapper): Qparam(%d), PipePackageInfo(%d)", rParams.isoValue, pipePackageInfo.isoValue);

            //[5]config pipe
            ret=callerPostProc->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);
            if(!ret)
            {
                LOG_ERR("postprocPipe config fail");
                return ret;
            }

            //[6] trigger mdp start
            ret=callerPostProc->start();
            if(!ret)
            {
                LOG_ERR("P2 Start Fail!");
                return ret;
            }

            //[7] add enqueued buf info to kernel after cofig pipe, to make sure the buffer order in kernel list is the same with we send to mdp
            bufPackage.callerID=(MUINTPTR)(&callerDeQList);
            bufPackage.bufTag=bufferTag;
	    	LOG_INF("swScenario(0x%x)/ecID(0x%x),tag(%d),pixID(%d)",swScenario,bufPackage.callerID,bufPackage.bufTag,pixID);


            if(q==0)
            {
                //top-level record number of buffer-batches
                bufPackage.rParams=(rParams);
                bufPackage.drvSce=mSwHwPathMapping[swScenario].hwPath;
                /******************************* BufBatch Lock Region Start ********************************/
                getLock(ELockEnum_BufBatchEnQ);
                //top-level buffer-batches list
                bufPackage.idxINbatchLforBnode=mvEnqueuedBufBatch.size();    //get real current index in batch list (HalpipeWrapper.Thread would delete element from batch list if deque success)
                mvEnqueuedBufBatch.push_back(bufPackage);

                //second-level record number of buffer nodes(one buffer batch may include lots of buffer nodes)
                getLock(ELockEnum_BufNodeEnQ);
                BufParamNode bufNode;
                bufNode.portEn=dmaEnPort;
                bufNode.p2cqIdx=bufPackage.p2cqIdx;
                bufNode.p2cqDupIdx=bufPackage.p2cqDupIdx;
                bufNode.p2BurstQIdx=q;
                bufNode.bufTag=bufPackage.bufTag;
                bufNode.drvSce=bufPackage.drvSce;
                bufNode.callerID=bufPackage.callerID;
                bufNode.idxInBufBatchList=bufPackage.idxINbatchLforBnode;        //avoid timing issue, addBatch -> addNode -> removeBatch
                bufNode.jpgo_idx=bufPackage.jpgo_idx;
                mvEnqueueBufNode.push_back(bufNode);
                //for user, they only know callback or deque done
                ret=callerPostProc->endequeFrameCtrl(NSImageioIsp3::NSIspio::EPIPE_BUFQUECmd_ENQUE_FRAME,bufNode.callerID,bufNode.p2BurstQIdx,bufPackage.p2cqDupIdx,bufPackage.frameNum,bufPackage.drvSce);
                releaseLock(ELockEnum_BufNodeEnQ);
                releaseLock(ELockEnum_BufBatchEnQ);
                /******************************* BufBatch Lock Region End ********************************/

                getLock(ELockEnum_CQUser);
                mp2CQDupIdx[pathCQ]=1-mp2CQDupIdx[pathCQ];  //pinpon for next user
                mp2CQUserNum[pathCQ]++;  //update current user number
                releaseLock(ELockEnum_CQUser);
                LOG_DBG("Enque bufNode[0]: %d", bufNode.idxInBufBatchList);
            }
            else
            {
                BufParamNode bufNode;
                getLock(ELockEnum_BufBatchEnQ);
                //get real current index in batch list (HalpipeWrapper.Thread would delete element from batch list if deque success)
                //FIXME, if there is the scenario that buffer nodes of two different buffer batches are cross enque ?
                //eleMentidx=mvEnqueuedBufBatch.size()-1;

                //second-level record number of buffer nodes(one buffer batch may include lots of buffer nodes)
                getLock(ELockEnum_BufNodeEnQ);

                bufNode.portEn=dmaEnPort;
                bufNode.p2cqIdx=bufPackage.p2cqIdx;
                bufNode.p2cqDupIdx=bufPackage.p2cqDupIdx;
                bufNode.p2BurstQIdx=q;
                bufNode.bufTag=bufPackage.bufTag;
                bufNode.drvSce=bufPackage.drvSce;
                bufNode.callerID=bufPackage.callerID;
                if(bufPackage.idxINbatchLforBnode == mvEnqueuedBufBatch.size())    //update real index for waited added buffer node in batchA
                { bufNode.idxInBufBatchList = mvEnqueuedBufBatch[mvEnqueuedBufBatch.size()-1].idxINbatchLforBnode; }
                else
                { bufNode.idxInBufBatchList = bufPackage.idxINbatchLforBnode; }
                bufNode.jpgo_idx=bufPackage.jpgo_idx;
                mvEnqueueBufNode.push_back(bufNode);
                //for user, they only know callback or deque done
                ret=callerPostProc->endequeFrameCtrl(NSImageioIsp3::NSIspio::EPIPE_BUFQUECmd_ENQUE_FRAME,bufNode.callerID,bufNode.p2BurstQIdx,bufPackage.p2cqDupIdx,bufPackage.frameNum,bufPackage.drvSce);
                releaseLock(ELockEnum_BufNodeEnQ);
                releaseLock(ELockEnum_BufBatchEnQ);
                LOG_DBG("Enque bufNode[%d]: %d", q, bufNode.idxInBufBatchList);
            }
            if(!ret)
            {
                LOG_ERR("enque ioctrl fail, return");
                return ret;
            }

            //[8] send signal to dequeue thread that another user enqueue a buf
            #if 1
            ECmd enqeueCmd=ECmd_ENQUEUE;
            sendCommand(NSImageioIsp3::NSIspio::EPIPECmd_SET_P2_QUEUE_CONTROL_STATE, (MUINT32)pathCQ, (MUINT32)enqeueCmd);
            #else
            ECmd enqeueCmd=ECmd_ENQUEUE;
            sendCommand((int)pathCQ,enqeueCmd);
            #endif

            //[9] pop out local variables forr next turn
            for(int i=(int)vPostProcInPorts.size()-1;i>=0;i--)
                vPostProcInPorts.pop_back();
            for(int i=(int)vPostProcOutPorts.size()-1;i>=0;i--)
                vPostProcOutPorts.pop_back();
            for(int i=(int)mvModule.size()-1;i>=0;i--)
                mvModule.pop_back();
        }

        if (bufPackageNum >= 2) {
            releaseLock(ELockEnum_MultiFrameEnQ);
        }
    }

    LOG_DBG("-");
    return ret;

}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
deque(
    ESoftwareScenario swScenario,
    NSCam::NSIoPipe::QParams& rParams,
    NSImageioIsp3::NSIspio::IPostProcPipe *callerPostProc,
    MUINTPTR callerID,
    MINT64 i8TimeoutNs,
    QFeatureData& rFeatureData)
{
    // suppose HalPipeWrapper::deque is not supported
    bool ret = MFALSE;

    MINT32 timeout=i8TimeoutNs/1000;
    //LOG_DBG("timeout(%d),i8TimeoutNs(%lld),callerID(0x%x),de_frameNo(%d)",timeout,i8TimeoutNs,callerID,rParams.mFrameNo);
    LOG_DBG("timeout(%d),i8TimeoutNs(%lld),callerID(0x%x)",timeout,i8TimeoutNs,callerID);

    //[1] goto check if buffer ready
    MUINT32 a,b;
    a=getUs();
    //assume that first enqued p2 dupCQ first out, always pass 0 as p2dupCQidx when deque
    ret=callerPostProc->endequeFrameCtrl(NSImageioIsp3::NSIspio::EPIPE_BUFQUECmd_WAIT_FRAME,callerID,0, 0 ,0, mSwHwPathMapping[swScenario].hwPath, timeout);
    b=getUs();
    LOG_INF("===== ret/period(%d/%d us) =====",ret,b-a);
    //[2] get the first matched element in dequeue buffer list
    if(ret)
    {
        queryFrame(rParams,rFeatureData,callerID);
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}
/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
queryFrame(
    NSCam::NSIoPipe::QParams& params,
    QFeatureData& featureParams,
    MINT32 callerID)
{
    bool ret=false;
    //MUINT32 cnt; // remove for no use
    FUNCTION_LOG_START;

    getLock(ELockEnum_BufBatchDeQ);

//    LOG_INF("callerID(%d) cID(%d) dBufSize(%d)",callerID,params.mFrameNo,(int)(mvDequeuedBufBatch.size()));//QQ

    for (list<BufParamPackage>::iterator it = mvDequeuedBufBatch.begin(); it != mvDequeuedBufBatch.end();)
    {
        if((*it).callerID == callerID)
        {   //get matched buffer
            params=(*it).rParams;
            featureParams=(*it).rFeatureData;
            mvDequeuedBufBatch.erase(it);
            LOG_INF("dequeuedBufListSize(%d)",(int)(mvDequeuedBufBatch.size()));
            ret=true;
            break;
        }
        it++;
        //cnt++; // remove for no use
//        LOG_INF("QQ count(%d)",cnt);
    }

    releaseLock(ELockEnum_BufBatchDeQ);
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
startVideoRecord(MINT32 wd,MINT32 ht,MINT32 fps, ESoftwareScenario swScen)
{
    bool ret=false;
    FUNCTION_LOG_START;
    getLock(ELockEnum_VencCnt);
    mVencPortCnt=0;
    LOG_DBG("mVencPortCnt(%d)",mVencPortCnt);
    releaseLock(ELockEnum_VencCnt);
    if(mpPostProcPipe)
    {
        NSImageioIsp3::NSIspio::EDrvScenario eDrv=mSwHwPathMapping[swScen].hwPath;
        MUINT32 cqIdx=0x0;
        switch(mSwHwPathMapping[swScen].ePathCQ)
        {
            case EPathCQ_CQ1:
                cqIdx=EDrvCQ_CQ01;
                break;
            case EPathCQ_CQ2:
                cqIdx=EDrvCQ_CQ02;
                break;
            case EPathCQ_CQ3:
                cqIdx=EDrvCQ_CQ03;
                break;
            default:
                LOG_ERR("no support this p2 path(%d)",mSwHwPathMapping[swScen].ePathCQ);
                break;
        }
        ret=mpPostProcPipe->startVideoRecord(wd,ht,fps,eDrv,cqIdx);
        if(!ret)
        {
            LOG_ERR("startVideo Record Fail");
        }
    }
    else
    {
        LOG_ERR("plz do init first");
    }
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
stopVideoRecord(ESoftwareScenario swScen)
{
    bool ret=false;
    FUNCTION_LOG_START;
    //[1] wait all the buffer-batches containing venc port are deque done
    getLock(ELockEnum_VencCnt);
    LOG_INF("+ mVencPortCnt(%d)",mVencPortCnt);
    if(mVencPortCnt>0)
    {
        releaseLock(ELockEnum_VencCnt);
        while(1)
        {
            ::sem_wait(&mSemVencCnt);
            getLock(ELockEnum_VencCnt);
            LOG_DBG("* mVencPortCnt(%d)",mVencPortCnt);
            if(mVencPortCnt==0)
            {
                releaseLock(ELockEnum_VencCnt);
                break;
            }
            else
            {
                releaseLock(ELockEnum_VencCnt);
            }
        }

    }
    else
    {
        releaseLock(ELockEnum_VencCnt);
    }

    //[2] do stop video record
    LOG_INF("- mVencPortCnt(%d)",mVencPortCnt);
    if(mpPostProcPipe)
    {
        NSImageioIsp3::NSIspio::EDrvScenario eDrv=mSwHwPathMapping[swScen].hwPath;
        MUINT32 cqIdx=0x0;
        switch(mSwHwPathMapping[swScen].ePathCQ)
        {
            case EPathCQ_CQ1:
                cqIdx=EDrvCQ_CQ01;
                break;
            case EPathCQ_CQ2:
                cqIdx=EDrvCQ_CQ02;
                break;
            case EPathCQ_CQ3:
                cqIdx=EDrvCQ_CQ03;
                break;
            default:
                LOG_ERR("no support this p2 path(%d)",mSwHwPathMapping[swScen].ePathCQ);
                break;
        }
        ret=mpPostProcPipe->stopVideoRecord(eDrv,cqIdx);
        if(!ret)
        {
            LOG_ERR("stopVideo Record Fail");
        }
    }
    else
    {
        LOG_ERR("plz do init first");
    }
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
sendCommand(
    MINT32 cmd,
    MINTPTR arg1,
    MINTPTR arg2,
    MINTPTR arg3)
{

    LOG_DBG("+,cmd(0x%x),arg1(0x%x),arg2(0x%x),arg3(0x%x)",cmd,arg1,arg2,arg3);
    switch (cmd){
        case NSImageioIsp3::NSIspio::EPIPECmd_SET_P2_QUEUE_CONTROL_STATE:
            if((ECmd)arg2 == ECmd_ENQUEUE){
                // signal to vss buffer if there is any incoming buffer (actually user should not enque any vss buffer if there still exist vss buffer in list)
                {   //the life cycle of mVssCondMtx is in the following region
                    Mutex::Autolock autoLock(mVssCondMtx);
                    mVssCond.signal();
                }
                // add enque cmd
                addCommand(arg1,ECmd_ENQUEUE);  // arg1 = dequeCq
            }
            break;
        case NSImageioIsp3::NSIspio::EPIPECmd_GET_NR3D_GAIN:
            {
                Mutex::Autolock lock(mEnqueLock);
                mpPostProcPipe->sendCommand(cmd, arg1, arg2, arg3);
            }
            break;
        default:
            mpPostProcPipe->sendCommand(cmd, arg1, arg2, arg3);
            break;

    }
    return true;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
deTuningQue(
    unsigned int& size,
    void* &pTuningQueBuf)
{
    Mutex::Autolock lock(mTuningQueLock);
    bool ret=true;
    //
    if(misV3)
    {
        size = sizeof(isp_reg_t);
        if(!gTuningQueNode.empty()) {
            pTuningQueBuf = gTuningQueNode[0];
            gTuningQueNode.erase(gTuningQueNode.begin());
            //
            LOG_DBG("queNum(%d),detuningQue(0x%08lx)\n",gTuningQueNode.size(),(unsigned long)(pTuningQueBuf));
        } else {
            tuninQueData tuningQueDataNew;
            #if defined(__PMEM_ONLY__)
                tuningQueDataNew.bufferInfo.virtAddr = (MUINT32*)pmem_alloc_sync(size, &gTuningQueData[i].bufferInfo.memID);
                memset((MUINT8*)tuningQueDataNew.bufferInfo.virtAddr,0,size);
            #else     // Not PMEM.
                tuningQueDataNew.bufferInfo.size = size;
                tuningQueDataNew.bufferInfo.useNoncache = 0; //alloc cacheable mem.
                if ( m_pIMemDrv->allocVirtBuf(NAME, &tuningQueDataNew.bufferInfo) ) {
                    LOG_ERR("[ERROR]:m_pIMemDrv->allocVirtBuf");
                }
                memset((MUINT8*)tuningQueDataNew.bufferInfo.virtAddr,0,size);
            #endif
            gTuningQueData.push_back(tuningQueDataNew);
            gTuningQueNode.push_back((void*)tuningQueDataNew.bufferInfo.virtAddr);
            //
            pTuningQueBuf = gTuningQueNode[0];
            gTuningQueNode.erase(gTuningQueNode.begin());
            //
            mTuningQueNum++;
            LOG_INF("[Warning]out of tuningque, add one more(0x%x)\n",gTuningQueData[gTuningQueData.size()-1].bufferInfo.virtAddr);
        }

        LOG_INF("[size]gTuningQueData(%d),gTuningQueNode(%d)\n",gTuningQueData.size(),gTuningQueNode.size());
    }
    else
    {
        LOG_ERR("v1 do not support this function, misV3(%d)", misV3);
    }
    //
    return ret;
}
/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
enTuningQue(
    void* pTuningQueBuf)
{
        Mutex::Autolock lock(mTuningQueLock);
    bool ret=true;

    if(misV3)
    {
        bool isCheckPass = false;
        //
        for(MUINT32 i=0;i<gTuningQueData.size();i++){
            if((void*)gTuningQueData[i].bufferInfo.virtAddr == pTuningQueBuf) {
                isCheckPass = true;
                break;
            }
        }
        // check tuningquebuufer address that get from M/W
        if(isCheckPass == true){
            gTuningQueNode.push_back(pTuningQueBuf);
        }else{
            LOG_ERR("[Error]tuningque buffer(0x%lx) is invalid",(unsigned long)(pTuningQueBuf));
        }
        //
        for(MUINT32 i=0;i<gTuningQueNode.size();i++){
            LOG_INF("i(%d),pmem(0x%08lx)\n",i,(unsigned long)(gTuningQueNode[i]));
        }
    }
    else
    {
        LOG_ERR("v1 do not support this function, misV3(%d)", misV3);
    }
    //
    return ret;
}
/******************************************************************************
* For Vss Concurrency Check
******************************************************************************/
MVOID
HalPipeWrapper::
checkVssStatus()
{
    int ret = -1;
    getLock(ELockEnum_VssOccupied);
    //LOG_INF("mIsVssOccupied(%d)",mIsVssOccupied);
    if (mIsVssOccupied == true)
    {
        releaseLock(ELockEnum_VssOccupied);
        LOG_INF("sem_wait");
        ret = ::sem_wait(&mSemVssOccupied);
        getLock(ELockEnum_VssOccupied);
        mIsVssOccupied = true;
        releaseLock(ELockEnum_VssOccupied);
    }
    else
    {
        mIsVssOccupied = true;
        releaseLock(ELockEnum_VssOccupied);
        //LOG_INF("sem_trywait");
        ret = ::sem_trywait(&mSemVssOccupied);
    }
}

int HalPipeWrapper::pixelIDMapping(MUINT32 sensorFormatOrder)
{
    switch (sensorFormatOrder)
    {
        case SENSOR_FORMAT_ORDER_RAW_B:
            return CAM_PIX_ID_B;
        case SENSOR_FORMAT_ORDER_RAW_Gb:
            return CAM_PIX_ID_Gb;
        case SENSOR_FORMAT_ORDER_RAW_Gr:
            return CAM_PIX_ID_Gr;
        case SENSOR_FORMAT_ORDER_RAW_R:
            return CAM_PIX_ID_R;
        default:
            return CAM_PIX_ID_B;
    }
}
