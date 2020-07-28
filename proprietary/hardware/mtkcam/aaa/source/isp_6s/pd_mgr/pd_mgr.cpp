/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "pd_mgr"

#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG       (1)
#endif

#include <sys/stat.h>
#include <stdio.h>

// android related
#include <cutils/properties.h>
#include <utils/threads.h>

// driver related
#include <kd_camera_feature.h>
#include "kd_imgsensor.h"
#include <private/IopipeUtils.h>

// systrace
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>

// pd hal
#include "pd_mgr.h"
#include <pd_buf_mgr_open.h>
#include <pd_buf_mgr.h>
#include <Local.h>

// nvram
#include <nvbuf_util.h>

// get AE status from resultpool
#include "IResultPool.h"
#include <ResultPool4Module.h>

// bpci talbe generator
#include "private/PDTblGen.h"

// algorithm
#include <pd_algo_if.h>

// thread
#include <mtkcam/def/PriorityDefs.h>
#include <sys/prctl.h>
#include <mtkcam/utils/std/common.h>

//
#include "mtkcam/utils/std/LogTool.h"

// for swpde
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>


using namespace NSCam::Utils;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

#define DBG_MSG_BUF_SZ 1024 //byte

namespace NS3Av3
{
#define DBG_MOVE_LENS_STABLE_FRAME_NUM (5)
#define DBG_PROPERTY_VALUE_MAX 10
#define PBN_SEPARATE_MODE 0

/************************************************************************/
/* Systrace                                                             */
/************************************************************************/

#define TRACE_LEVEL_DBG 1
#define TRACE_LEVEL_DEF 0

#ifdef PD_TRACE_FMT_BEGIN
#undef PD_TRACE_FMT_BEGIN
#endif
#define PD_TRACE_FMT_BEGIN(lv, fmt, arg...)      \
    do {                                         \
        if (m_i4DbgSystraceLevel >= lv) {        \
            CAM_TRACE_FMT_BEGIN(fmt, ##arg);     \
        }                                        \
    } while(0)

#ifdef PD_TRACE_FMT_END
#undef PD_TRACE_FMT_END
#endif
#define PD_TRACE_FMT_END(lv)                     \
    do {                                         \
        if (m_i4DbgSystraceLevel >= lv) {        \
            CAM_TRACE_FMT_END();                 \
        }                                        \
    } while(0)


/************************************************************************/
/* Calculation                                                          */
/************************************************************************/
//
#define _GET_TIME_STAMP_US_() ({\
        MINT64 _ts = 0;\
        struct timespec t;\
        t.tv_sec = t.tv_nsec = 0;\
        clock_gettime(CLOCK_MONOTONIC, &t);\
        _ts = ((t.tv_sec) * 1000000000LL + t.tv_nsec)/1000;\
        _ts;\
    })

//
MINT32
PDMgr::Boundary( MINT32 a_i4Min,
                 MINT32 a_i4Vlu,
                 MINT32 a_i4Max)
{
    if (a_i4Max < a_i4Min)
    {
        a_i4Max = a_i4Min;
    }
    if (a_i4Vlu < a_i4Min)
    {
        a_i4Vlu = a_i4Min;
    }
    if (a_i4Vlu > a_i4Max)
    {
        a_i4Vlu = a_i4Max;
    }
    return a_i4Vlu;
}

MUINT32
PDMgr::GetLensAdpCompPos( MINT32  pixel_clk,
                          MINT32  line_length,
                          MUINT64 expTime,
                          MINT64  time_stamp_sof, /* ns */
                          mcuMotorInfo &lensInfo,
                          AREA_T &roi)
{
    MUINT32 pos = 0;

    //
    if( /* checking lens adaptive compensation is working or not */
        (0 < expTime                         ) &&
        (expTime < (MUINT64)(time_stamp_sof) ) &&
        (0 < time_stamp_sof                  ) &&
        (0 < lensInfo.u4CurrentPosition      ) &&
        (lensInfo.u4CurrentPosition <= 1023  ) &&
        (0 < lensInfo.u8CurrentTimestamp     ) &&
        (0 < lensInfo.u4PreviousPosition     ) &&
        (lensInfo.u4PreviousPosition <= 1023 ) &&
        (0 < pixel_clk                       ) &&
        (0 < line_length))
    {
        MUINT64 ts_exp_start     = time_stamp_sof - expTime;           //ns
        MUINT64 line_length_time = 1000000000LL*line_length/pixel_clk; //ns

        //
        MUINT64 ts_roi_start     = ts_exp_start + roi.i4Y*line_length_time;
        MUINT64 ts_roi_end       = ts_roi_start + roi.i4H*line_length_time + expTime;
        MUINT64 time_length_roi  = ts_roi_end - ts_roi_start;

        //
        MUINT64 time_length_pre_pos = (ts_roi_start < (lensInfo.u8CurrentTimestamp*1000LL) ) ?
                                      ( (lensInfo.u8CurrentTimestamp*1000LL) - ts_roi_start) :
                                      0;
        time_length_pre_pos = (time_length_pre_pos < time_length_roi) ?
                              time_length_pre_pos :
                              time_length_roi;
        //
        MUINT64 time_length_cur_pos = ((lensInfo.u8CurrentTimestamp*1000LL + 12000000LL) < ts_roi_end) ?
                                      (ts_roi_end - (lensInfo.u8CurrentTimestamp*1000LL + 12000000LL)) :
                                      0;
        time_length_cur_pos = (time_length_cur_pos < time_length_roi) ?
                              time_length_cur_pos :
                              time_length_roi;

        //
        if( /* */
            (time_length_roi) &&
            ((time_length_cur_pos + time_length_pre_pos) <= time_length_roi))
        {
            MUINT64 time_length_mving = time_length_roi - time_length_cur_pos - time_length_pre_pos;

            //
            pos  = (
                       (lensInfo.u4CurrentPosition  * time_length_cur_pos) +
                       (lensInfo.u4PreviousPosition * time_length_pre_pos) +
                       ((lensInfo.u4CurrentPosition + lensInfo.u4PreviousPosition)/2)*time_length_mving
                   )/time_length_roi;

            CAM_LOGD_IF(m_bDebugEnable,
                        "#(%d) [%s] %d->%d, (%" PRIu64 ":%" PRIu64 "/%" PRIu64 "/%" PRIu64 "). exTime:%" PRIu64 " ts_sof:%" PRId64 " ns cur_lens_info(%d, %" PRId64 " us) pre_lens_info(%d, %" PRId64 " us) pixel_clk:%d line_length:%d ts_exp_s:%" PRIu64 " line_length_time:%" PRIu64 " ts_roi_s:%" PRIu64 " ts_roi_e:%" PRIu64 " roi:%d/%d/%d/%d",
                        m_MagicNumber,
                        __FUNCTION__,
                        lensInfo.u4CurrentPosition,
                        pos,
                        time_length_roi,
                        time_length_pre_pos,
                        time_length_mving,
                        time_length_cur_pos,
                        expTime,
                        time_stamp_sof,
                        lensInfo.u4CurrentPosition,
                        lensInfo.u8CurrentTimestamp,
                        lensInfo.u4PreviousPosition,
                        lensInfo.u8PreviousTimestamp,
                        pixel_clk,
                        line_length,
                        ts_exp_start,
                        line_length_time,
                        ts_roi_start,
                        ts_roi_end,
                        roi.i4X,
                        roi.i4Y,
                        roi.i4W,
                        roi.i4H);

        }
        else
        {
            /*should not be happened*/

            pos = lensInfo.u4CurrentPosition;

            CAM_LOGW( "#(%d) [%s] abnormal(%" PRIu64 "+%" PRIu64 ">%" PRIu64 "). exTime:%" PRIu64 " ts_sof:%" PRId64 " ns cur_lens_info(%d, %d us) pre_lens_info(%d, %d us) pixel_clk:%d line_length:%d ts_exp_s:%" PRIu64 " line_length_time:%" PRIu64 " ts_roi_s:%" PRIu64 " ts_roi_e:%" PRIu64 " roi:%d/%d/%d/%d",
                      m_MagicNumber,
                      __FUNCTION__,
                      time_length_pre_pos,
                      time_length_cur_pos,
                      time_length_roi,
                      expTime,
                      time_stamp_sof,
                      lensInfo.u4CurrentPosition,
                      lensInfo.u8CurrentTimestamp,
                      lensInfo.u4PreviousPosition,
                      lensInfo.u8PreviousTimestamp,
                      pixel_clk,
                      line_length,
                      ts_exp_start,
                      line_length_time,
                      ts_roi_start,
                      ts_roi_end,
                      roi.i4X,
                      roi.i4Y,
                      roi.i4W,
                      roi.i4H);
        }



    }
    else
    {
        pos = lensInfo.u4CurrentPosition;

        CAM_LOGD_IF( m_bDebugEnable,
                     "#(%d) [%s] not working : expTime:%" PRIu64 " ts_sof:%" PRId64 " ns cur_lens_info(%d, %d us) pre_lens_info(%d, %d us) pixel_clock:%d line_length:%d",
                     m_MagicNumber,
                     __FUNCTION__,
                     expTime,
                     time_stamp_sof,
                     lensInfo.u4CurrentPosition,
                     lensInfo.u8CurrentTimestamp,
                     lensInfo.u4PreviousPosition,
                     lensInfo.u8PreviousTimestamp,
                     pixel_clk,
                     line_length);
    }

    pos = Boundary( 0, pos, 1023);

    return pos;

}


/************************************************************************/
/* Multi-instance                                                       */
/************************************************************************/
//
PDMgr* PDMgr::s_pPDMgr = MNULL;

//
template <ESensorDev_T const eSensorDev>
class PDMgrDev : public PDMgr
{
public:
    static PDMgr& getInstance()
    {
        static PDMgrDev<eSensorDev> singleton;
        PDMgr::s_pPDMgr = &singleton;
        return singleton;
    }

    PDMgrDev() : PDMgr(eSensorDev) {}
    virtual ~PDMgrDev() {}
};

//
PDMgr&
PDMgr::getInstance(MINT32 const i4SensorDev)
{
    switch ( i4SensorDev)
    {
        case ESensorDev_Main:
            return  PDMgrDev<ESensorDev_Main>::getInstance();
        case ESensorDev_MainSecond:
            return  PDMgrDev<ESensorDev_MainSecond>::getInstance();
        case ESensorDev_MainThird:
            return  PDMgrDev<ESensorDev_MainThird>::getInstance();
        case ESensorDev_Sub:
            return  PDMgrDev<ESensorDev_Sub>::getInstance();
        case ESensorDev_SubSecond:
            return  PDMgrDev<ESensorDev_SubSecond>::getInstance();
        default:
            CAM_LOGE("unknow seneor dev(%d)", i4SensorDev);
            if ( PDMgr::s_pPDMgr)
            {
                return  *PDMgr::s_pPDMgr;
            }
            else
            {
                return  PDMgrDev<ESensorDev_Main>::getInstance();
            }
    }
}

/************************************************************************/
/* Initialization and uninitialization                                  */
/************************************************************************/
PDMgr::PDMgr(ESensorDev_T eSensorDev) :
    m_i4CurrSensorDev((MINT32)eSensorDev),
    m_i4SensorIdx(0),
    m_pIHalSensorList(nullptr),
    m_pIHalSensor(nullptr),
    m_Sensor_pixel_clk(0),
    m_Sensor_line_length(0),
    m_Sensor_frame_length(0),
    m_pNVRam(nullptr),
    m_pLensNVRam(nullptr),
    m_u1CaliDaSrc(0),
    m_i4CaliAFPos_Inf(0),
    m_i4CaliAFPos_Macro(0),
    m_i4CaliAFPos_50cm(0),
    m_i4CaliAFPos_Middle(0),
    m_i4NVRamIdx(AF_CAM_SCENARIO_NUM_2),
    m_i4DbgSystraceLevel(TRACE_LEVEL_DEF),
    m_bDebugEnable(0),
    m_i4DbgPdDump(0),
    m_i4DbgPdDumpTS(""),
    m_i4DbgDisPdHandle(0),
    m_i4DbgPDVerifyEn(0),
    m_i4DbgPDVerifyRun(0),
    m_i4DbgPDVerifyStepSize(0),
    m_i4DbgPDVerifyCalculateNum(0),
    m_i4DbgPDVerifyCalculateCnt(0),
    m_i4DbgPDVerifyRangeL(0),
    m_i4DbgPDVerifyRangeH(1023),
    m_MagicNumber(0),
    m_requestNum(0),
    m_bEnPDBufThd(MFALSE),
    m_Status(EPD_Not_Enabled),
    m_pIPdAlgo(nullptr),
    m_pPDBufMgrCore(nullptr),
    m_pPDBufMgrOpen(nullptr),
    m_pAnalysisData_Update(nullptr),
    m_pAnalysisData_Calculate(nullptr),
    m_databuf_size(0),
    m_databuf_stride(0),
    m_databuf(nullptr),
    m_minLensPos(0),
    m_maxLensPos(0),
    m_exposureTime(0),
    m_time_stamp_sof(0),
    m_bEnablePBIN(MFALSE),
    m_eAETargetMode(AE_MODE_NORMAL),
    m_bConfigCompleted(MFALSE),
    m_bNeedPDResult(MFALSE),
    m_bForceCalculation(MFALSE)
{
    memset( &m_lensInfo,          0, sizeof(mcuMotorInfo));
    memset( &m_SensorStaticInfo,  0, sizeof(SensorStaticInfo));
    memset( &m_SensorDynamicInfo, 0, sizeof(SensorDynamicInfo));
    memset( &m_sCaliData,         0, sizeof(PD_CALIBRATION_DATA_T));
    memset( &m_profile,           0, sizeof(SPDProfile_t));
    m_sPDOHWCfg = ISP_PDO_CFG_T();
    m_vRawBufCtrl.clear();
}
//
PDMgr::~PDMgr()
{}

//
MBOOL
PDMgr::CamPwrOnState()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

//
MBOOL
PDMgr::CamPwrOffState()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

//
MRESULT
PDMgr::getPDInfoForSttCtrl(MINT32 sensorIdx,
                           MINT32 sensorMode,
                           MUINT32 &oPDOSizeW,
                           MUINT32 &oPDOSizeH,
                           FEATURE_PDAF_STATUS &oPDAFStatus)
{
    //
    ::IPDTblGen::getInstance()->start(m_i4CurrSensorDev, m_i4SensorIdx);

    //
    CAM_LOGD( "[%s] + sensor_idx(%d), sensor_mode(%d)", __FUNCTION__, sensorIdx, sensorMode);

    MRESULT ret = E_3A_ERR;

    /*-------------------------------------------------------------------------------------------------------------------------
     * Calling timing :
     *    PDTblGen module should be executed before getPDInfoForSttCtrl is executed.
     *    Before configuring PD manager, getPDInfoForSttCtrl should be executed by 3a framework.
     *    So initial PD manager flow is (start PDTblGen) -> (PDMgr::getPDInfoForSttCtrl) -> (PDMgr::config)
     *    Outputs oPDOSizeW, oPDOSizeH, oPDAFStatus for each type PD HW path for configuring SttPipe in 3A framework.
     *-------------------------------------------------------------------------------------------------------------------------*/

    // reset output
    oPDOSizeW   = 0;
    oPDOSizeH   = 0;
    oPDAFStatus = FEATURE_PDAF_UNSUPPORT;

    // initial parameters
    memset( &m_profile,       0xFF, sizeof(SPDProfile_t       ));
    memset( &m_PDBlockInfo,      0, sizeof(SET_PD_BLOCK_INFO_T));
    memset( &m_PDVCInfo,         0, sizeof(SINGLE_VC_INFO2    ));
    memset( &m_SensorCropInfo,   0, sizeof(SensorCropWinInfo  ));

    //
    m_pIHalSensorList = MAKE_HalSensorList();
    INormalPipe* pNormalPipe = (INormalPipe*)INormalPipeUtils::get()->createDefaultNormalPipe( sensorIdx, LOG_TAG);

    // query pd capacity from sensor driver and TG information.
    if(m_pIHalSensorList && pNormalPipe)
    {
        // Get query sensor dev from sensor index
        MUINT sensor_dev = m_pIHalSensorList->querySensorDevIdx(sensorIdx);

        // Get p1 information :
        MSize sz_tg;
        MSize sz_bin;
        {
            MINT32 err1 = pNormalPipe->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_TG_OUT_SIZE, (MINTPTR)(&sz_tg.w),  (MINTPTR)(&sz_tg.h),  0);
            MINT32 err2 = pNormalPipe->sendCommand( NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_BIN_INFO,    (MINTPTR)(&sz_bin.w), (MINTPTR)(&sz_bin.h), 0);
            pNormalPipe->destroyInstance( LOG_TAG);
        }

        //
        if(sensor_dev==(MUINT)(m_i4CurrSensorDev))
        {
            /*
             * Query sensor static informataion PDAF_Support :
             * 0 : NO PDAF
             * 1 : PDAF Raw Data mode
             * 2 : PDAF VC mode(Full)
             * 3 : PDAF VC mode(Binning)
             * 4 : PDAF DualPD Raw Data mode
             * 5 : PDAF DualPD VC mode
             */
            m_pIHalSensorList->querySensorStaticInfo(sensor_dev, &m_SensorStaticInfo);

            //
            m_pIHalSensor = m_pIHalSensorList->createSensor( LOG_TAG, m_i4CurrSensorDev);
            if(m_pIHalSensor)
            {
                /*
                 * Store PDAF related information for flow control.
                 */
                m_profile.i4CurrSensorId       = sensorIdx;
                m_profile.u4IsZSD              = ((MUINT)(sz_tg.w)==m_SensorStaticInfo.captureWidth) && ( (MUINT)(sz_tg.h)==m_SensorStaticInfo.captureHeight) ? MTRUE : MFALSE;
                m_profile.u4PDAF_support_type  = (IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM)m_SensorStaticInfo.PDAF_Support;
                m_profile.uImgXsz              = sz_tg.w;
                m_profile.uImgYsz              = sz_tg.h;
                m_profile.uFullXsz             = (MINT32)(m_SensorStaticInfo.captureWidth);
                m_profile.uFullYsz             = (MINT32)(m_SensorStaticInfo.captureHeight);
                m_profile.u4IsFrontalBinning   = ((sz_tg.w==sz_bin.w) && (sz_tg.h==sz_bin.h)) ? 0 : 1;
                m_profile.i4SensorMode         = sensorMode;
                m_profile.i4CurrSensorId       = m_SensorStaticInfo.sensorDevID;
                m_profile.BufType              = EPDBUF_NOTDEF;
                m_profile.uPdSeparateMode      = PBN_SEPARATE_MODE;
                m_profile.bEnablePBIN          = m_bEnablePBIN; /* setPBNen should be executed before getPDInfoForSttCtrl !! */

                // query information from sensor driver
                MINT32 err0 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY, (MINTPTR )&(m_profile.i4SensorMode)/*input*/, (MINTPTR)&(m_profile.bSensorModeSupportPD)/*Output*/,  0);
                MINT32 err1 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_INFO,     (MINTPTR )&(m_profile.i4SensorMode)/*input*/, (MINTPTR )&(m_PDBlockInfo)/*Output*/,                  0);
                //
                MBOOL pd_vc_info_valid = MFALSE;
                SensorVCInfo2 sensor_vc_info_v2;
                memset( &sensor_vc_info_v2, 0, sizeof(SensorVCInfo2));
                MINT32 err2 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_VC_INFO2,         (MUINTPTR)&(sensor_vc_info_v2)/*Output*/,        (MUINTPTR)&(m_profile.i4SensorMode)/*input*/,          0);
                if(err2==0)
                {
                    for(MUINT32 i=0; i<sensor_vc_info_v2.vcInfo2s.size(); i++)
                    {
                        if(sensor_vc_info_v2.vcInfo2s[i].VC_FEATURE == VC_PDAF_STATS)
                        {
                            memcpy( reinterpret_cast<void *>(&m_PDVCInfo), reinterpret_cast<void *>(&(sensor_vc_info_v2.vcInfo2s[i])), sizeof(SINGLE_VC_INFO2));
                            pd_vc_info_valid = MTRUE;
                        }
                    }
                }

                //
                MINT32 err3 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO, (MUINTPTR)&(m_profile.i4SensorMode)/*input*/, (MUINTPTR)&(m_SensorCropInfo)/*Output*/,               0);
                MINT32 err4 = m_pIHalSensor->querySensorDynamicInfo( m_i4CurrSensorDev, &m_SensorDynamicInfo);
                //
                m_Sensor_pixel_clk = 0;
                MINT32 err5 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_PIXEL_CLOCK_FREQ, (MINTPTR)&m_Sensor_pixel_clk, 0,0);
                //
                MINT32 sensor_period = 0;
                MINT32 err6 = m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM, (MINTPTR)&sensor_period, 0,0);
                m_Sensor_line_length  = (0x0000FFFF & sensor_period);
                m_Sensor_frame_length = (0xFFFF0000 & sensor_period) >> 16;

                //


                //
                CAM_LOGD( "[%s] SensorMode(%d), SensorInfo:pixel clock(%d)/line length(%d)/frame length(%d), PD block info:offset(%d,%d)/pitch(%d,%d)/pairNum(%d)/subBlkSz(%d,%d)/blockNum(%d,%d)/longExpFirst(%d)/mirrorFlip(%d), pd Info(valid=%d):DataType(0x%x)/ID(0x%x)/SIZEH(0x%x PIXEL=0x%x BYTE)/SIZEV(0x%x), win Info:FullImgSz(%d,%d)/Crop(%d, %d, %d, %d)",
                          __FUNCTION__,
                          m_profile.i4SensorMode,
                          m_Sensor_pixel_clk,
                          m_Sensor_line_length,
                          m_Sensor_frame_length,
                          m_PDBlockInfo.i4OffsetX,
                          m_PDBlockInfo.i4OffsetY,
                          m_PDBlockInfo.i4PitchX,
                          m_PDBlockInfo.i4PitchY,
                          m_PDBlockInfo.i4PairNum,
                          m_PDBlockInfo.i4SubBlkW,
                          m_PDBlockInfo.i4SubBlkH,
                          m_PDBlockInfo.i4BlockNumX,
                          m_PDBlockInfo.i4BlockNumY,
                          m_PDBlockInfo.i4LeFirst,
                          m_PDBlockInfo.iMirrorFlip,
                          pd_vc_info_valid,
                          m_PDVCInfo.VC_DataType,
                          m_PDVCInfo.VC_ID,
                          m_PDVCInfo.VC_SIZEH_PIXEL,
                          m_PDVCInfo.VC_SIZEH_BYTE,
                          m_PDVCInfo.VC_SIZEV,
                          m_SensorCropInfo.full_w,
                          m_SensorCropInfo.full_h,
                          m_SensorCropInfo.x0_offset,
                          m_SensorCropInfo.y0_offset,
                          m_SensorCropInfo.w0_size,
                          m_SensorCropInfo.h0_size);

                //
                MINT32 err  = (err0!=0 /* 0=SENSOR_NO_ERROR */) ||
                              (err1!=0 /* 0=SENSOR_NO_ERROR */) ||
                              (err2!=0 /* 0=SENSOR_NO_ERROR */) ||
                              (err3!=0 /* 0=SENSOR_NO_ERROR */) ||
                              (err4!=1);
                if(!err)
                {
                    /*
                     * Outputs u4PDOSizeW, u4PDOSizeH, PDAFStatus by PD type.
                     */
                    if( m_profile.bSensorModeSupportPD==0)
                    {
                        // Currently, sensor mode is not support PD.
                        oPDOSizeW   = 0;
                        oPDOSizeH   = 0;
                        oPDAFStatus = FEATURE_PDAF_UNSUPPORT;

                    }
                    else if( m_profile.u4PDAF_support_type==PDAF_SUPPORT_RAW)
                    {
                        // Get BPCI table information
                        Tbl bpci;
                        MBOOL getTblDn = ::IPDTblGen::getInstance()->getTbl( m_i4CurrSensorDev, m_profile.i4SensorMode, bpci);

                        // checking constraint
                        MBOOL   isTGInfoValid  = (0<sz_tg.w) && ((MUINT)(sz_tg.w)<=m_SensorStaticInfo.captureWidth) && (0<sz_tg.h) && ((MUINT)(sz_tg.h)<=m_SensorStaticInfo.captureHeight);
                        MUINT32 pdo_pixels_x   = (bpci.tbl.pdo_xsz+1)/2;
                        MUINT32 pdo_pixels_y   = (bpci.tbl.pdo_ysz+1);
                        MBOOL   isBPCItblValid = (getTblDn) &&
                                                 (0<pdo_pixels_x                    ) &&
                                                 (  pdo_pixels_x<=(MUINT32)(sz_tg.w)) &&
                                                 (0<pdo_pixels_y                    ) &&
                                                 (  pdo_pixels_y<=(MUINT32)(sz_tg.h)) &&
                                                 (0<bpci.tbl.tbl_xsz                ) &&
                                                 (bpci.tbl.tbl_ysz==0);

                        //
                        CAM_LOGD( "[%s] Dev 0x%04x, sensormode(%d) TGSZ(%d,%d), BINSZ(%d,%d), BPCITbl(Valid=%d): xSz(%d) ySz(%d) pa(%p) va(%p) memID(%d), PDO: xSz(0x%x) ySz(0x%x), isTGInfoValid(%d)/isFrontalBINDisable(%d)/isFullMode(%d)",
                                  __FUNCTION__,
                                  m_i4CurrSensorDev,
                                  m_profile.i4SensorMode,
                                  sz_tg.w,
                                  sz_tg.h,
                                  sz_bin.w,
                                  sz_bin.h,
                                  isBPCItblValid,
                                  bpci.tbl.tbl_xsz,
                                  bpci.tbl.tbl_ysz,
                                  bpci.tbl.tbl_pa,
                                  bpci.tbl.tbl_va,
                                  bpci.tbl.memID,
                                  bpci.tbl.pdo_xsz,
                                  bpci.tbl.pdo_ysz,
                                  isTGInfoValid,
                                  m_profile.u4IsFrontalBinning,
                                  m_profile.u4IsZSD);

                        if( (isTGInfoValid) && (isBPCItblValid))
                        {
                            /*
                             * PDAF Raw Data mode is supported once frontal binning is not enabled.
                             *
                             * Unit of bpci.tbl.pdo_xsz is BYTE and start from 0, so total byte of PDO x size is (bpci.tbl.pdo_xsz+1).
                             *
                             * Unit of u4PDOSizeW is PIXEL, and there is one more pixel data which means EOL.
                             * So u4PDOSizeW is (total PD pixels + one EOL data).
                             */
                            oPDOSizeW   = (_PDO_STRIDE_ALIGN_((bpci.tbl.pdo_xsz+1)/2 + 1));
                            oPDOSizeH   = bpci.tbl.pdo_ysz+1;
                            oPDAFStatus = FEATURE_PDAF_SUPPORT_BNR_PDO;
                        }
                        else
                        {
                            oPDOSizeW   = 0;
                            oPDOSizeH   = 0;
                            oPDAFStatus = FEATURE_PDAF_UNSUPPORT;
                        }


                    }
                    else if( m_profile.u4PDAF_support_type==PDAF_SUPPORT_RAW_DUALPD)
                    {
                        oPDOSizeW   = sz_tg.w; // no use for dual pd raw type case, set TG size just for avoid P1 pipe check fail.
                        oPDOSizeH   = sz_tg.h;
                        oPDAFStatus = FEATURE_PDAF_SUPPORT_PBN_PDO;
                    }
                    else if( m_profile.u4PDAF_support_type==PDAF_SUPPORT_CAMSV || m_profile.u4PDAF_support_type==PDAF_SUPPORT_CAMSV_LEGACY || m_profile.u4PDAF_support_type==PDAF_SUPPORT_CAMSV_DUALPD)
                    {
                        oPDOSizeW   = 0; // no use for camsv case.
                        oPDOSizeH   = 0;
                        oPDAFStatus = FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL;
                    }
                    else if( m_profile.u4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY)
                    {
                        oPDOSizeW   = 0; // no use for legacy raw case.
                        oPDOSizeH   = 0;
                        oPDAFStatus = FEATURE_PDAF_SUPPORT_LEGACY;
                    }
                    else
                    {
                        oPDOSizeW   = 0;
                        oPDOSizeH   = 0;
                        oPDAFStatus = FEATURE_PDAF_UNSUPPORT;
                    }

                    //
                    m_profile.u4PDPipeCtrl = oPDAFStatus;

                    CAM_LOGD("[%s] SensorMode(%d), PDAF_Support(%d), bSensorModeSupportPDAF(%d), Info for 3A framework to Cfg sttPipe :PDAFStatus(%d), PDOSzW(%d PIXELS), PDOSzH(%d)",
                             __FUNCTION__,
                             m_profile.i4SensorMode,
                             m_profile.u4PDAF_support_type,
                             m_profile.bSensorModeSupportPD,
                             oPDAFStatus,
                             oPDOSizeW,
                             oPDOSizeH);

                    //
                    ret = S_AF_OK;
                }
                else
                {
                    CAM_LOGE( "[%s] sensor command fail (%d/%d/%d/%d/%d)",
                              __FUNCTION__,
                              err0,
                              err1,
                              err2,
                              err3,
                              err4);
                }
            }
            else
            {
                CAM_LOGE( "[%s] create sensor fail (%p). Please check 3A framework flow.", __FUNCTION__, m_pIHalSensor);
            }
        }
        else
        {
            CAM_LOGE( "[%s] sensor info fail dev(%d/%d), idx(%d). Please check dev info mapping in 3a framework.",
                      __FUNCTION__,
                      m_i4CurrSensorDev,
                      sensor_dev,
                      sensorIdx);
        }

    }
    else
    {
        CAM_LOGE("[%s] Can not query information. Please check 3A framework flow. (%p/%p)", __FUNCTION__, m_pIHalSensorList, pNormalPipe);
    }

    CAM_LOGD( "[%s] - %d : PDO size(%d, %d), PDAFStatus(%d)", __FUNCTION__, ret, oPDOSizeW, oPDOSizeH, oPDAFStatus);

    return ret;

}

//
MRESULT
PDMgr::setNVRAMIndex(MUINT32 afNVRamIdx)
{
    if(afNVRamIdx<AF_CAM_SCENARIO_NUM_2)
    {
        Mutex::Autolock lock(m_Lock_NvRamIdx);

        //
        if( afNVRamIdx!=m_i4NVRamIdx)
        {
            CAM_LOGW( "#(%d) [%s] update nvram index (%d->%d)", m_MagicNumber, __FUNCTION__, m_i4NVRamIdx, afNVRamIdx);
        }

        //
        m_i4NVRamIdx = afNVRamIdx;
    }
    else
    {
        CAM_LOGW( "#(%d) [%s] No need to update tuning data to algorithm because of invalid index(%d)!! Please check flow in IdxMgr.", m_MagicNumber, __FUNCTION__, afNVRamIdx);
    }

    return S_AF_OK;
}


//
MRESULT
PDMgr::config(MINT32 sensorOpenIndex,
              MUINT32 sensorMode,
              MUINT32 afNVRamIdx)
{
    Mutex::Autolock lock(m_Lock);

    CAM_LOGD_IF( m_bDebugEnable,"[%s] +", __FUNCTION__);

    /*--------------------------------------------------------------------------------------------------------
     * Configure PD manager.
     *  m_profile is fully setted here.
     *  Some information in m_profile is setted in PDMgr::getPDInfoForSttCtrl.
     *  So 3A framework shoul be executed PDMgr::getPDInfoForSttCtrl before PDMgr::config() is executed !!!
     *--------------------------------------------------------------------------------------------------------*/

    //initial member.
    m_requestNum        = 0;
    m_Status            = EPD_Not_Enabled;
    m_databuf_size      = 0;
    m_databuf_stride    = 0;
    m_databuf           = nullptr;
    m_pPDBufMgrCore     = nullptr;
    m_pPDBufMgrOpen     = nullptr;
    m_pIPdAlgo          = nullptr;
    m_pNVRam            = nullptr;
    m_pLensNVRam        = nullptr;
    m_i4SensorIdx       = sensorOpenIndex;
    m_minLensPos        = 0;
    m_maxLensPos        = 0;
    m_bNeedPDResult     = MFALSE;
    m_bConfigCompleted  = MFALSE;
    m_bForceCalculation = MFALSE;
    m_pAnalysisData_Update    = &m_sAnalysisData[1];
    m_pAnalysisData_Calculate = &m_sAnalysisData[0];

    memset( &m_sAnalysisData,     0, sizeof(S_ANALYSIS_DATA_T)*2);
    memset( &m_sAnalysisDataOpen, 0, sizeof(S_ANALYSIS_DATA_OPEN_T));
    memset( &m_sPDOHWInfo,        0, sizeof(SPDOHWINFO_T));

    m_vPDOutput.clear();

    //
    m_vRawBufCtrl.clear();
    sem_init( &m_semSWPDE, 0, 0);


    //
    if(m_pIHalSensorList && m_pIHalSensor && (MUINT32)(m_profile.i4SensorMode)==sensorMode)
    {
        // Get query sensor dev from sensor index
        MUINT sensor_dev = m_pIHalSensorList->querySensorDevIdx(sensorOpenIndex);

        // Get sensor static information
        //pIHalSensorList->querySensorStaticInfo( sensor_dev, &m_SensorStaticInfo);

        // PD manager profile
        m_profile.AETargetMode = m_eAETargetMode;

        //
        if((MUINT)(m_i4CurrSensorDev)==sensor_dev)
        {
            //
            m_i4DbgPDVerifyEn    = property_get_int32("vendor.debug.pd_verify_flow.enable", 0);
            m_i4DbgPdDump        = property_get_int32("vendor.pd.dump.enable", 0);
            m_bDebugEnable       = (MINT32)( (m_i4DbgPdDump) || (m_i4DbgPDVerifyEn) || (property_get_int32("vendor.debug.af_mgr.enable", 0)!=0));
            m_i4DbgDisPdHandle   = property_get_int32("vendor.debug.handlepd.disable", 0);
            m_i4DbgSystraceLevel = property_get_int32("vendor.debug.pdsystrace", TRACE_LEVEL_DEF);


            //create folder for saving debug information.
            if( m_i4DbgPdDump || m_bDebugEnable || m_i4DbgPDVerifyEn)
            {
                FILE *fp = fopen("/sdcard/pdo/pd_mgr_info", "w");
                if( NULL == fp)
                {
                    MINT32 err = mkdir( "/sdcard/pdo", S_IRWXU | S_IRWXG | S_IRWXO);

                    if( 0!=err && EEXIST!=errno )
                    {
                        CAM_LOGW("fail to create folder /sdcard/pdo : %d[%s]", errno, ::strerror(errno));
                    }
                    else
                    {
                        CAM_LOGD( "create folder /sdcard/pdo (%d)", err);
                    }
                }
                else
                {
                    fprintf( fp, "folder /sdcard/pdo is exist");
                    fclose( fp);
                }
            }

            //
            m_sPDOHWCfg = ISP_PDO_CFG_T(m_i4SensorIdx);


            // inital pd buffer manager and pd algo once PD is supported in current sensor mode.
            if( m_profile.bSensorModeSupportPD)
            {
                /*--------------------------------------------------------------------------------------------------------
                 *                                                  (1)
                 *               Maching PD buffer manager setting and kernel image sensor driver setting
                 *--------------------------------------------------------------------------------------------------------*/
                //get pd buffer type from custom setting.
                MUINT32 pd_buf_type = GetPDBuf_Type( m_i4CurrSensorDev, m_profile.i4CurrSensorId);
                //
                m_profile.BufType = EPDBUF_NOTDEF;
                switch( (IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM)(m_profile.u4PDAF_support_type))
                {
                    case PDAF_SUPPORT_RAW_LEGACY:
                        if( (pd_buf_type&(MSK_CATEGORY | MSK_PDBUF_TYPE_RAW_LEGACY))==EPDBUF_RAW_LEGACY)
                        {
                            m_profile.BufType = EPDBUF_RAW_LEGACY;
                        }
                        break;

                    case PDAF_SUPPORT_RAW:
                        if( (pd_buf_type&(MSK_CATEGORY | MSK_PDBUF_TYPE_PDO))==EPDBUF_PDO)
                        {
                            // checking dependency module constraint
                            if( m_profile.u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_BNR_PDO)
                            {
                                m_profile.BufType = (EPDBUF_TYPE_t)EPDBUF_PDO;
                            }
                            else
                            {
                                CAM_LOGE( "[%s] config PD buffer manager as PDO, but sttPipe isn't initialized as FEATURE_PDAF_SUPPORT_BNR_PDO (cur=%d).", __FUNCTION__, m_profile.u4PDPipeCtrl);
                            }
                        }
                        break;

                    case PDAF_SUPPORT_CAMSV:
                        if( (pd_buf_type&(MSK_CATEGORY | MSK_PDBUF_TYPE_VC)) == EPDBUF_VC)
                        {
                            // checking dependency module constraint
                            if( m_profile.u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
                            {
                                m_profile.BufType = (EPDBUF_TYPE_t)EPDBUF_VC;
                            }
                            else
                            {
                                CAM_LOGE( "[%s] config PD buffer manager as CamSv, but sttPipe isn't initialized as FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL (cur=%d).", __FUNCTION__, m_profile.u4PDPipeCtrl);
                            }
                        }
                        else if( (pd_buf_type&(MSK_CATEGORY | MSK_PDBUF_TYPE_VC)) == EPDBUF_VC_OPEN)
                        {
                            //
                            if( m_databuf)
                            {
                                CAM_LOGE( "[%s] should not be happened!! someting wrong during last time stop pd_mgr flow", __FUNCTION__);
                                delete m_databuf;
                            }

                            //
                            m_databuf        = nullptr;
                            m_databuf_size   = 0;
                            m_databuf_stride = 0;

                            //
                            m_profile.BufType = (EPDBUF_TYPE_t)EPDBUF_VC_OPEN;
                        }
                        break;

                    case PDAF_SUPPORT_RAW_DUALPD:
                        if( (pd_buf_type&(MSK_CATEGORY | EPDBUF_PDO))== EPDBUF_DUALPD_RAW)
                        {
                            if( /* checking dependency module constraint*/
                                (m_profile.u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_PBN_PDO) &&
                                (m_profile.bEnablePBIN))
                            {
                                m_profile.BufType = EPDBUF_DUALPD_RAW;
                            }
                            else
                            {
                                CAM_LOGE( "[%s] config PD buffer manager as dualPD raw type, but sttPipe or P1Node isn't initialized as enabling PBN (%d/%d).", __FUNCTION__, m_profile.u4PDPipeCtrl, m_profile.bEnablePBIN);
                            }
                        }
                        break;

                    case PDAF_SUPPORT_CAMSV_DUALPD:
                        if( (pd_buf_type&(MSK_CATEGORY | MSK_PDBUF_TYPE_VC))== EPDBUF_DUALPD_VC)
                        {
                            // checking dependency module constraint
                            if( m_profile.u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
                            {
                                m_profile.BufType = EPDBUF_DUALPD_VC;
                            }
                            else
                            {
                                CAM_LOGE( "[%s] config PD buffer manager as dualPD CamSv type, but sttPipe isn't initialized as FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL  (cur=%d).", __FUNCTION__, m_profile.u4PDPipeCtrl);
                            }
                        }
                        break;

                    default:
                        CAM_LOGE( "[%s] Unknown PD sensor type (%d)", __FUNCTION__, m_profile.u4PDAF_support_type);
                        break;
                }

                CAM_LOGD( "[%s]: dev(0x%04x) PDBufMgr type(0x%x/0x%x) sensorID(0x%x) isZSD(%d) tg_sz(%d, %d) cap_sz(%d, %d) pdaf_support_type(%d) FrontalBin(%d) PBNen(%d) separateMode(%d) sensorMode(%d), aeTargetMode(%d), PDpipeCtrl(%d), sensor support PDAF(%d)",
                          __FUNCTION__,
                          m_i4CurrSensorDev,
                          pd_buf_type,
                          m_profile.BufType,
                          m_profile.i4CurrSensorId,
                          m_profile.u4IsZSD,
                          m_profile.uImgXsz,
                          m_profile.uImgYsz,
                          m_profile.uFullXsz,
                          m_profile.uFullYsz,
                          m_profile.u4PDAF_support_type,
                          m_profile.u4IsFrontalBinning,
                          m_profile.bEnablePBIN,
                          m_profile.uPdSeparateMode,
                          m_profile.i4SensorMode,
                          m_profile.AETargetMode,
                          m_profile.u4PDPipeCtrl,
                          m_profile.bSensorModeSupportPD);



                //
                MINT32 _err = NvBufUtil::getInstance().getBufAndRead( CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)m_pLensNVRam);
                if(_err ==0 && m_pLensNVRam)
                {
                    //
                    if(m_bDebugEnable)
                    {
                        for(int i=0; i<AF_CAM_SCENARIO_NUM_2; i++)
                        {
                            CAM_LOGD_IF(m_bDebugEnable, "DBG : Lens NVRAM #%d, File Path: %s", i, m_pLensNVRam->AF[i].rFilePath);
                        }
                    }

                    //
                    if( afNVRamIdx<AF_CAM_SCENARIO_NUM_2)
                    {
                        //
                        m_i4NVRamIdx = afNVRamIdx;
                        m_pNVRam     = &(m_pLensNVRam->AF[m_i4NVRamIdx]);

                        //
                        CAM_LOGD( "[%s]: Dev(0x%04x) nvram_info : idx(%d) ver(%d) Sz(%d, %zu, %zu, %zu, %zu, %zu) path(%s)",
                                  __FUNCTION__,
                                  m_i4CurrSensorDev,
                                  m_i4NVRamIdx,
                                  m_pLensNVRam->Version,
                                  MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE,
                                  sizeof(UINT32),
                                  sizeof(FOCUS_RANGE_T),
                                  sizeof(AF_NVRAM_T),
                                  sizeof(PD_NVRAM_T),
                                  MAXIMUM_NVRAM_CAMERA_LENS_FILE_SIZE-sizeof(UINT32)-sizeof(FOCUS_RANGE_T)-sizeof(AF_NVRAM_T)-sizeof(PD_NVRAM_T),
                                  m_pNVRam->rFilePath);


                        // Initial PD related HW module.
                        ConfigurePDHWSetting( &m_profile, &(m_pNVRam->rPDNVRAM.rTuningData));

                        //
                        if( property_get_int32("vendor.debug.pdflow.disable", 0)==1)
                        {
                            CAM_LOGW("disabled PD calculation only (pd related hw is still running) : sensor mode supported PD (%d->%d)", m_profile.bSensorModeSupportPD, FEATURE_PDAF_UNSUPPORT);

                            //
                            m_profile.BufType = EPDBUF_NOTDEF;
                        }

                        /*--------------------------------------------------------------------------------------------------------
                         *                                                  (2)
                         *                                        Create related instance
                         *--------------------------------------------------------------------------------------------------------*/
                        if( m_profile.BufType == EPDBUF_NOTDEF)
                        {
                            // related instance is initialized, do nothing.
                        }
                        else if( m_profile.BufType & MSK_CATEGORY_OPEN)
                        {
                            m_pPDBufMgrOpen = ::PDBufMgrOpen::createInstance( m_profile);
                        }
                        else if( m_profile.BufType & MSK_CATEGORY_DUALPD)
                        {
                            m_pPDBufMgrCore = ::PDBufMgr::createInstance( m_profile);
                            m_pIPdAlgo      = NS3A::IPdAlgo::createInstance( m_i4CurrSensorDev, PDType_DualPD);
                        }
                        else
                        {
                            m_pPDBufMgrCore = ::PDBufMgr::createInstance( m_profile);
                            m_pIPdAlgo      = NS3A::IPdAlgo::createInstance( m_i4CurrSensorDev, PDType_Legacy);
                        }

                        //
                        MBOOL instanceRdy = (m_pPDBufMgrOpen) || (m_pPDBufMgrCore && m_pIPdAlgo) ? MTRUE : MFALSE;

                        /*--------------------------------------------------------------------------------------------------------
                         *                                                  (3)
                         *                     Check PD mgr initial status to deside thread should be created or not.
                         *                     After related instance and PD thread is created, load calibration data.
                         *--------------------------------------------------------------------------------------------------------*/
                        if( instanceRdy)
                        {
                            //start calculating thread.
                            createThread();

                            if( m_bEnPDBufThd)
                            {
                                // Set PD calibration data and tuning data.
                                if( setPDCaliData( &(m_pNVRam->rPDNVRAM))==S_3A_OK)
                                {
                                    // Get parameters for verification flow :
                                    // switch for executing flow.
                                    m_i4DbgPDVerifyRun          = property_get_int32("vendor.debug.pd_verify_flow.run", 0);

                                    // calculate one step size.
                                    MINT32 rangeL  = property_get_int32("vendor.debug.pd_verify_flow.RangeL",  -1);
                                    MINT32 rangeH  = property_get_int32("vendor.debug.pd_verify_flow.RangeH",  -1);
                                    MINT32 stepNum = property_get_int32("vendor.debug.pd_verify_flow.StepNum", 10);
                                    m_i4DbgPDVerifyRangeL   = (rangeL==-1) ?   Boundary( 0,   m_i4CaliAFPos_Inf, 1023) : Boundary(   0, rangeL,  200);
                                    m_i4DbgPDVerifyRangeH   = (rangeH==-1) ?   Boundary( 0, m_i4CaliAFPos_Macro, 1023) : Boundary( 900, rangeH, 1023);
                                    m_i4DbgPDVerifyStepSize = (m_i4DbgPDVerifyRangeH-m_i4DbgPDVerifyRangeL)/( Boundary( 10, stepNum, 100));

                                    // calculation times for each step
                                    m_i4DbgPDVerifyCalculateNum = (DBG_MOVE_LENS_STABLE_FRAME_NUM)+( Boundary( 10, property_get_int32("vendor.debug.pd_verify_flow.CalNum", 10), 20));

                                    // reset parameter
                                    m_i4DbgPDVerifyCalculateCnt = 0;

                                    if( /* checking pd verification can be enabled or not*/
                                        (m_i4DbgPDVerifyEn                     )&&
                                        (0<m_i4DbgPDVerifyCalculateNum         )&&
                                        (m_i4DbgPDVerifyRangeL!=m_i4DbgPDVerifyRangeH)&&
                                        (m_i4DbgPDVerifyStepSize!=0            )&&
                                        (0<m_i4DbgPDVerifyCalculateNum         )&&
                                        (m_i4DbgPDVerifyCalculateNum<1023      ))
                                    {
                                        property_set("vendor.debug.pdmgr.lockae", "0");
                                        property_set("vendor.debug.af_motor.disable", "1");

                                        /* Set m_i4DbgPDVerifyRangeL as initial position*/
                                        char value[DBG_PROPERTY_VALUE_MAX] = {'\0'};
                                        snprintf( value,
                                                  DBG_PROPERTY_VALUE_MAX,
                                                  "%d",
                                                  m_i4DbgPDVerifyRangeL + (m_i4DbgPDVerifyCalculateCnt/m_i4DbgPDVerifyCalculateNum)*m_i4DbgPDVerifyStepSize);
                                        property_set("vendor.debug.af_motor.position", value);

                                        CAM_LOGD( "[%s] DBG : check initial value lockae(%d) Motordisable(%d) motorPos(%d)",
                                                  __FUNCTION__,
                                                  property_get_int32("vendor.debug.pdmgr.lockae", 0),
                                                  property_get_int32("vendor.debug.af_motor.disable", 0),
                                                  property_get_int32("vendor.debug.af_motor.position", 0));
                                    }
                                    else
                                    {
                                        m_i4DbgPDVerifyEn = 0;
                                    }
                                    CAM_LOGD( "[%s] DBG : verification flow enable(%d), step number(%d) step size(%d), calculation interval(%d frames), range(%d, %d)",
                                              __FUNCTION__,
                                              m_i4DbgPDVerifyEn,
                                              stepNum,
                                              m_i4DbgPDVerifyStepSize,
                                              m_i4DbgPDVerifyCalculateNum,
                                              m_i4DbgPDVerifyRangeL,
                                              m_i4DbgPDVerifyRangeH);

                                    //
                                    SPDLibVersion_t PdLibVersion;
                                    GetVersionOfPdafLibrary( PdLibVersion);
                                    CAM_LOGD( "[%s]: dev(0x%04x), PDAF flow is enabled, PD version %d.%d", __FUNCTION__, m_i4CurrSensorDev, (MINT32)PdLibVersion.MajorVersion, (MINT32)PdLibVersion.MinorVersion);

                                }
                                else
                                {
                                    CAM_LOGE( "[%s] set PD calibration and tuning data fail, close PD thread !! Please check previous error log !!", __FUNCTION__);
                                    closeThread();
                                    m_profile.BufType = EPDBUF_NOTDEF;
                                }
                            }
                        }
                        else
                        {
                            m_profile.BufType = EPDBUF_NOTDEF;
                            CAM_LOGD( "[%s] Instance is not ready : PDBufMgrCore(%p) PDAlgo(%p) PDBufMgrOpen(%p)", __FUNCTION__, (void*)m_pPDBufMgrCore, (void*)m_pIPdAlgo, (void*)m_pPDBufMgrOpen);
                        }
                    }
                    else
                    {
                        m_profile.BufType = EPDBUF_NOTDEF;
                        CAM_LOGE( "[%s] NvRam index (%d) from idxMgr is not vaild!!", __FUNCTION__, afNVRamIdx);
                    }
                }
                else
                {
                    m_profile.BufType = EPDBUF_NOTDEF;
                    CAM_LOGE( "[%s] get NvRam fail !! (%d/%p)", __FUNCTION__, _err, m_pLensNVRam);
                    m_pLensNVRam = nullptr;
                    m_pNVRam     = nullptr;
                }
            }
            else
            {
                m_profile.BufType = EPDBUF_NOTDEF;
                CAM_LOGD("[%s] Current sensor mode is not support PD", __FUNCTION__);
            }

            CAM_LOGD( "[%s] SensorID(0x%x), PDAF_Support(%d), SensorModeSupportPD(%d), BufType(0x%02x) Note:(NOTDEF[0x00] VC[0x01] VC_OPEN[0x11] RAW_LEGACY[0x02] RAW_LEGACY_OPEN[0x12] PDO[0x04] PDO_OPEN[0x14] DUALPD_VC[0x21] DUALPD_RAW[0x24]",
                      __FUNCTION__,
                      m_profile.i4CurrSensorId,
                      m_profile.u4PDAF_support_type,
                      m_profile.bSensorModeSupportPD,
                      m_profile.BufType);
        }
        else
        {
            CAM_LOGE( "[%s] sensor info fail dev(%d/%d), idx(%d). Please check dev info mapping in 3a framework.",
                      __FUNCTION__,
                      m_i4CurrSensorDev,
                      sensor_dev,
                      sensorOpenIndex);

        }
    }
    else
    {
        CAM_LOGE("[%s]: Dev(0x%04x), Initial flow in 3A framework is not correct (PDMgr::getPDInfoForSttCtrl must be executed before config PD manager) !! (%p/%p),(%d!=%d)",
                 __FUNCTION__,
                 m_i4CurrSensorDev,
                 m_pIHalSensorList,
                 m_pIHalSensor,
                 m_profile.i4SensorMode,
                 sensorMode);
    }


    CAM_LOGD_IF( m_bDebugEnable,"[%s] - return pd buffer manager type[0x%02x]", __FUNCTION__, m_profile.BufType);

    return S_3A_OK;
}


//
MRESULT
PDMgr::stop()
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    if( m_pIHalSensor)
    {
        m_pIHalSensor->destroyInstance(LOG_TAG);
        m_pIHalSensor     = nullptr;
        m_pIHalSensorList = nullptr;
    }

    if( m_bEnPDBufThd)
    {
        //close thread
        closeThread();
    }

    //
    while(1)
    {
        Mutex::Autolock lock( m_Lock_AnalysisData);

        S_ANALYSIS_DATA_OPEN_T *del_element = m_sAnalysisDataOpen.head;

        if( del_element==NULL)
        {
            m_sAnalysisDataOpen.tail = NULL;
            CAM_LOGD( "[%s] all resource in data list are destroyed : sz(%d) prv(%p) nxt(%p) head(%p) tail(%p)",
                      __FUNCTION__,
                      m_sAnalysisDataOpen.totalsz,
                      m_sAnalysisDataOpen.prv,
                      m_sAnalysisDataOpen.nxt,
                      m_sAnalysisDataOpen.head,
                      m_sAnalysisDataOpen.tail);
            break;
        }

        m_sAnalysisDataOpen.head = del_element->nxt;
        m_sAnalysisDataOpen.nxt  = del_element->nxt;
        m_sAnalysisDataOpen.totalsz--;

        delete del_element;
    }

    //
    m_databuf_size   = 0;
    m_databuf_stride = 0;

    if( m_databuf)
        delete m_databuf;
    m_databuf = nullptr;

    //
    m_pPDBufMgrCore = NULL;
    m_pPDBufMgrOpen = NULL;

    //
    m_pIPdAlgo   = nullptr;
    m_pNVRam     = nullptr;
    m_pLensNVRam = nullptr;

    //
    {
        Mutex::Autolock lock(m_Lock_Result);
        m_vPDOutput.clear();
    }

    //
    m_bEnablePBIN = MFALSE;

    if( m_i4DbgPDVerifyEn)
    {
        property_set("vendor.debug.pd_verify_flow.run", "0");
        property_set("vendor.debug.pd_verify_flow.enable", "0");
        property_set("vendor.debug.pdmgr.lockae", "0");
        property_set("vendor.debug.af_motor.disable", "0");
    }


    //
    m_bConfigCompleted = MFALSE;

    CAM_LOGD("[%s] -", __FUNCTION__);

    return S_3A_OK;
}


//----------------------------------------------------------------------------------------------------
MRESULT
PDMgr::ConfigurePDHWSetting(SPDProfile_t *iPDProfile,
                            PD_ALGO_TUNING_T *ptrInTuningData)
{
    MRESULT ret=E_3A_ERR;

    if( iPDProfile->u4PDPipeCtrl==FEATURE_PDAF_SUPPORT_VIRTUAL_CHANNEL)
    {
        if( iPDProfile->BufType==EPDBUF_VC_OPEN)
        {
            S_ANALYSIS_DATA_OPEN_T *last_cfg = m_sAnalysisDataOpen.tail;
            if( last_cfg)
            {
                // query sensor driver setting for 3rd party PD library.
                MUINT32  arg2 = 0;
                MUINT16 *arg3 = NULL;
                MBOOL    res  = MFALSE;

                if( m_pPDBufMgrOpen)
                {
                    PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "GetRegSetting");

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "[%s] send command PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING to pd buffer manager (open) +",
                                 __FUNCTION__);

                    res  = m_pPDBufMgrOpen->sendCommand( PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING,
                                                         (MVOID *)(last_cfg),
                                                         (MVOID *)(&arg2),
                                                         (MVOID *)(&arg3));

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "[%s] send command PDBUFMGR_OPEN_CMD_GET_PD_WIN_REG_SETTING to pd buffer manager (open) - result(%d) size(%d) addr(%p)",
                                 __FUNCTION__,
                                 res,
                                 arg2,
                                 arg3);

                    PD_TRACE_FMT_END(TRACE_LEVEL_DEF);
                }

                if( res && m_pIHalSensor && arg2 && arg3)
                {
                    PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "SendCmd2Sensor");

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "[%s] send command SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING to IHalSensor +",
                                 __FUNCTION__);

                    MINT64 timestamp0 = _GET_TIME_STAMP_US_();
                    m_pIHalSensor->sendCommand( m_i4CurrSensorDev,
                                                SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING,
                                                (MUINTPTR)&arg2,
                                                (MUINTPTR)&arg3,
                                                0);
                    MINT64 timestamp1 = _GET_TIME_STAMP_US_();

                    if( timestamp1-timestamp0 > 10*1000)
                    {
                        CAM_LOGW( "[%s] performance issue !! Time duration of command SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING is over 10ms, Please check that burst writing I2C mode is used in command SENSOR_FEATURE_SET_PDAF_REG_SETTING int image sensor kernel driver !!", __FUNCTION__);
                    }

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "[%s] send command SENSOR_CMD_SET_SENSOR_PDAF_REG_SETTING to IHalSensor -",
                                 __FUNCTION__);

                    PD_TRACE_FMT_END(TRACE_LEVEL_DEF);
                }
                else
                {
                    CAM_LOGD_IF( m_bDebugEnable,
                                 "[%s] Can not get PD window HW setting. Use setting in kernel driver. Send command result(%d) : size(%d), addr(%p)",
                                 __FUNCTION__,
                                 res,
                                 arg2,
                                 arg3);
                }
            }
            else
            {
                CAM_LOGE( "[%s] pd configuration to sensor is not vaild. Please check hybrid AF timing of updating PD parameters.", __FUNCTION__);
            }
        }
        else
        {
            // Do nothing.
            ret = E_3A_ERR;
        }
    }
    else if( (iPDProfile->u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_PBN_PDO) || (iPDProfile->u4PDPipeCtrl == FEATURE_PDAF_SUPPORT_BNR_PDO))
    {
        //
        IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM PDAF_support_type = (IMGSENSOR_PDAF_SUPPORT_TYPE_ENUM)iPDProfile->u4PDAF_support_type;

        if( PDAF_support_type == PDAF_SUPPORT_RAW)
        {
            // Get BPCI table information for PDO
            Tbl bpci;
            if( ::IPDTblGen::getInstance()->getTbl( m_i4CurrSensorDev, iPDProfile->i4SensorMode, bpci) == MFALSE)
            {
                CAM_LOGE("Failed acquiring BPCI table!");
                ret = E_3A_ERR;
            }
            else
            {
                m_sPDOHWInfo.u1IsDualPD      = 0;
                m_sPDOHWInfo.u4BitDepth      = 10; /* Expect bit depth of PDO port*/
                m_sPDOHWInfo.u4Bpci_xsz      = bpci.tbl.tbl_xsz;
                m_sPDOHWInfo.u4Bpci_ysz      = bpci.tbl.tbl_ysz;
                m_sPDOHWInfo.u4Pdo_xsz       = bpci.tbl.pdo_xsz;
                m_sPDOHWInfo.u4Pdo_ysz       = bpci.tbl.pdo_ysz;
                m_sPDOHWInfo.phyAddrBpci_tbl = (MUINTPTR)bpci.tbl.tbl_pa;
                m_sPDOHWInfo.virAddrBpci_tbl = (MUINTPTR)bpci.tbl.tbl_va;
                m_sPDOHWInfo.i4memID         = bpci.tbl.memID;

                CAM_LOGD( "[%s] BPCI table first 4 dword: %x %x %x %x",
                          __FUNCTION__,
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[0],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[1],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[2],
                          ((MUINT32*)m_sPDOHWInfo.virAddrBpci_tbl)[3]);

                ret = S_3A_OK;
            }

#if 0
            // TODO : remove it
            // Read BPCI table related information from BNR2 ISP module.
            MUINT32  u4Bpci_xsz=0, u4Bpci_ysz=0, u4Pdo_xsz=0, u4Pdo_ysz=0;
            MUINTPTR phyAddrBpci_tbl, virAddrBpci_tbl;
            MINT32   memID=0;
            MBOOL isBPCItbl = ISP_MGR_BNR2::getInstance(static_cast<ESensorDev_T>(m_i4CurrSensorDev)).getBPCItable(
                                  u4Bpci_xsz,
                                  u4Bpci_ysz,
                                  u4Pdo_xsz,
                                  u4Pdo_ysz,
                                  phyAddrBpci_tbl,
                                  virAddrBpci_tbl,
                                  memID,
                                  eIDX_PDE);


            if( bpci.tbl.tbl_xsz != u4Bpci_xsz)
            {
                CAM_LOGD("error: tbl_xsz is not the same %d %d\n", bpci.tbl.tbl_xsz, u4Bpci_xsz);
            }
            else if( bpci.tbl.tbl_ysz != u4Bpci_ysz)
            {
                CAM_LOGD("error: tbl_ysz is not the same %d %d\n", bpci.tbl.tbl_ysz, u4Bpci_ysz);
            }
            else if( bpci.tbl.pdo_xsz != u4Pdo_xsz)
            {
                CAM_LOGD("error: pdo_xsz is not the same %d %d\n", bpci.tbl.pdo_xsz, u4Pdo_xsz);
            }
            else if( bpci.tbl.pdo_ysz != u4Pdo_ysz)
            {
                CAM_LOGD("error: pdo_ysz is not the same %d %d\n", bpci.tbl.pdo_ysz, u4Pdo_ysz);
            }
            else
            {
                bool isMatch = true;
                char *ptr0 = (char*)(bpci.tbl.tbl_va);
                char *ptr1 = (char*)(virAddrBpci_tbl);
                for( int i=0; i<(bpci.tbl.tbl_xsz)*(bpci.tbl.tbl_ysz); i++)
                {
                    if( ptr1[i]!=ptr0[i])
                    {
                        isMatch = false;
                        CAM_LOGD("error: element %d is not the same %d %d\n", i, ptr0[i], ptr1[i]);
                    }
                }

                if( isMatch)
                {
                    CAM_LOGD("!! Match !!\n");
                }
            }
#endif
        }
        else if( PDAF_support_type == PDAF_SUPPORT_RAW_DUALPD)
        {
            MINT32 binningX     = 0;
            MINT32 binningY     = 0;
            MINT32 needShortExp = 0;

            if( ptrInTuningData)
            {
                binningX     = ptrInTuningData->i4Reserved[0];
                binningY     = ptrInTuningData->i4Reserved[1];
                needShortExp = ptrInTuningData->i4Reserved[2];
            }
            if( /* error check */
                (binningX > 8) ||
                (binningY > 8) ||
                (needShortExp > 2) ||
                (needShortExp < 0) ||
                (binningX < 1) ||
                (binningY < 1))
            {
                CAM_LOGW( "[%s] dualpd tuning data is wrong !! binningX(%d) binningY(%d) needShortExp(%d), Please check PD tuning parameters at index 0,1,2 in reserved array",
                          __FUNCTION__,
                          binningX,
                          binningY,
                          needShortExp);
                binningX     = 8;
                binningY     = 8;
                needShortExp = 0;
            }

            m_sPDOHWInfo.u1IsDualPD  = 1;
            m_sPDOHWInfo.u4BitDepth  = 10; /* Expect bit depth of PDO port*/
            m_sPDOHWInfo.u1PBinType  = (iPDProfile->AETargetMode == 1  /*AE_MODE_IVHDR_TARGET*/) ? 0 : 1; /*0:1x4, 1:4x4*/
            m_sPDOHWInfo.u4BinRatioX = binningX;
            m_sPDOHWInfo.u4BinRatioY = binningY;

            if( iPDProfile->uPdSeparateMode==1)
            {
                m_sPDOHWInfo.u1PdSeparateMode = 1;
                m_sPDOHWInfo.u4Pdo_xsz        = ((iPDProfile->uImgXsz / m_sPDOHWInfo.u4BinRatioX)*sizeof(MUINT16))-1;
                m_sPDOHWInfo.u4Pdo_ysz        = ((iPDProfile->uImgYsz / m_sPDOHWInfo.u4BinRatioY)*2)- 1; // 2 : L and R
            }
            else if( iPDProfile->uPdSeparateMode==0)
            {
                m_sPDOHWInfo.u1PdSeparateMode = 0;
                m_sPDOHWInfo.u4Pdo_xsz        = ((iPDProfile->uImgXsz / m_sPDOHWInfo.u4BinRatioX)*sizeof(MUINT16)*2)-1; // 2 : L and R
                m_sPDOHWInfo.u4Pdo_ysz        = ((iPDProfile->uImgYsz / m_sPDOHWInfo.u4BinRatioY)- 1);
            }
            else
            {
                CAM_LOGE("[%s]- separate mode setting ERROR!", __FUNCTION__);
            }

            m_sPDOHWInfo.u1PBinStartLine = (needShortExp                   ) ?
                                           (m_PDBlockInfo.i4LeFirst ? 1 : 0) :
                                           (m_PDBlockInfo.i4LeFirst ? 0 : 1);
            m_sPDOHWInfo.u1PBinStartLine +=  (m_sPDOHWInfo.u1PBinType == 1) ? 0 : 2; // in 1x4, start line + 2

        }


        //
        m_sPDOHWCfg.pdo_ctl.Bits.pdo_en = 1;
        if( m_sPDOHWInfo.u1IsDualPD)
        {
            m_sPDOHWCfg.pdo_ctl.Bits.pde_en = 0;
            m_sPDOHWCfg.pdo_ctl.Bits.pbn_en = 1;
        }
        else
        {
            m_sPDOHWCfg.pdo_ctl.Bits.pde_en = 1;
            m_sPDOHWCfg.pdo_ctl.Bits.pbn_en = 0;
        }
        m_sPDOHWCfg.pdo_xsz           = m_sPDOHWInfo.u4Pdo_xsz;
        m_sPDOHWCfg.pdo_ysz           = m_sPDOHWInfo.u4Pdo_ysz;
        m_sPDOHWCfg.bit_depth         = m_sPDOHWInfo.u4BitDepth;
        m_sPDOHWCfg.pdi_tbl_xsz       = m_sPDOHWInfo.u4Bpci_xsz;
        m_sPDOHWCfg.pdi_tbl_ysz       = m_sPDOHWInfo.u4Bpci_ysz;
        m_sPDOHWCfg.pdi_tbl_pa        = m_sPDOHWInfo.phyAddrBpci_tbl;
        m_sPDOHWCfg.pdi_tbl_va        = m_sPDOHWInfo.virAddrBpci_tbl;
        m_sPDOHWCfg.pdi_tbl_memID     = m_sPDOHWInfo.i4memID;
        m_sPDOHWCfg.pbn_type          = m_sPDOHWInfo.u1PBinType;
        m_sPDOHWCfg.pbn_start_line    = m_sPDOHWInfo.u1PBinStartLine;
        m_sPDOHWCfg.pbn_separate_mode = m_sPDOHWInfo.u1PdSeparateMode;


        /**********************************************************************************************************************************************
         *
         * Notice :
         *   No matter if m_pPDBufMgrCore or m_pIPdAlgo is NULL, need to config PDO HW once
         *   when u4PDPipeCtrl is FEATURE_PDAF_SUPPORT_PBN_PDO or FEATURE_PDAF_SUPPORT_BNR_PDO, otherwise system crash.
         *
         ***********************************************************************************************************************************************/
        CAM_LOGD( "[%s] pdaf_support_type(%d), PDOHWInfo :  pd_ctrl[%d/%d/%d] pdi_tbl[sz(0x%x,0x%x) pa(%p) va(%p) memID(%d)] pdo_sz[0x%x,0x%x] bitDepth[%d] PBinType[%d - 0:1x4, 1:4x4] PBinStartLine[%d] PdSeparateMode[%d]",
                  __FUNCTION__,
                  PDAF_support_type,
                  m_sPDOHWCfg.pdo_ctl.Bits.pdo_en,
                  m_sPDOHWCfg.pdo_ctl.Bits.pde_en,
                  m_sPDOHWCfg.pdo_ctl.Bits.pbn_en,
                  m_sPDOHWCfg.pdi_tbl_xsz,
                  m_sPDOHWCfg.pdi_tbl_ysz,
                  (void*)m_sPDOHWCfg.pdi_tbl_pa,
                  (void*)m_sPDOHWCfg.pdi_tbl_va,
                  m_sPDOHWCfg.pdi_tbl_memID,
                  m_sPDOHWCfg.pdo_xsz,
                  m_sPDOHWCfg.pdo_ysz,
                  m_sPDOHWCfg.bit_depth,
                  m_sPDOHWCfg.pbn_type,
                  m_sPDOHWCfg.pbn_start_line,
                  m_sPDOHWCfg.pbn_separate_mode);
    }
    else
    {
        // Do nothing.
        ret = E_3A_ERR;
    }

    return ret;
}

/************************************************************************/
/* PD calculation thread                                                */
/************************************************************************/
//
MVOID
PDMgr::createThread()
{
    if( m_bEnPDBufThd==MFALSE)
    {
        CAM_LOGD( "create PD buffer calculation thread");
        //create thread
        m_bEnPDBufThd = MTRUE;
        sem_init( &m_semPDBuf, 0, 0);
        sem_init( &m_semPDBufThdEnd, 0, 1);
        pthread_create( &m_PDBufThread, NULL, PDBufThreadLoop, this);
    }
    else
    {
        CAM_LOGE( "PD buffer calculation thread is not closed last time!!");
    }
}

//
MVOID
PDMgr::closeThread()
{
    CAM_LOGD("close PD buffer calculation thread");
    //close thread
    m_bEnPDBufThd = MFALSE;
    ::sem_post(&m_semPDBuf);
    pthread_join( m_PDBufThread, NULL);
}

//
MVOID
PDMgr::changePDBufThreadSetting()
{
    // (1) set name
    ::prctl( PR_SET_NAME, "PDBufThd", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_STT;
        int policy = 0, priority = 0;
        setThreadPriority( expect_policy, expect_priority);
        getThreadPriority( policy, priority);
        //

        CAM_LOGD_IF( m_bDebugEnable,
                     "[PDMgr::PDBufThreadLoop] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)",
                     expect_policy,
                     policy,
                     expect_priority,
                     priority);
    }

}

//
MVOID*
PDMgr::PDBufThreadLoop(MVOID *arg)
{
    MRESULT ret=E_3A_ERR;

    PDMgr *_this = reinterpret_cast<PDMgr*>(arg);

    CAM_LOGD( "[%s] thread start", __FUNCTION__);

    // (1) change thread setting
    _this->changePDBufThreadSetting();
    _this->m_Status = EPD_Init;


    // (2) thread-in-loop
    while(1)
    {
        ::sem_wait( &_this->m_semPDBuf);
        if ( ! _this->m_bEnPDBufThd)
        {
            break;
        }

        _this->m_Status = EPD_BZ;

        CAM_LOGD_IF( _this->m_bDebugEnable, "#(%d) [%s] + run calculation task, dev:%d", _this->m_MagicNumber, __FUNCTION__, _this->m_i4CurrSensorDev);

        //
        if(_this->m_i4DbgSystraceLevel >= TRACE_LEVEL_DBG)
        {
            CAM_TRACE_FMT_BEGIN("PDCalculation #(%d)", _this->m_MagicNumber);
        }

        //run core pd flow to get pd algorithm result.
        switch( _this->m_profile.BufType)
        {
            case EPDBUF_VC :
            case EPDBUF_RAW_LEGACY :
            case EPDBUF_PDO :
            case EPDBUF_DUALPD_VC:
            case EPDBUF_DUALPD_RAW:
                ret = _this->PDCoreFlow();
                break;

            case EPDBUF_VC_OPEN :
            case EPDBUF_RAW_LEGACY_OPEN :
            case EPDBUF_PDO_OPEN :
                ret = _this->PDOpenCoreFlow();
                break;

            default :
                break;

        }
        CAM_LOGD_IF( _this->m_bDebugEnable, "#(%d) [%s] - run calculation task, dev:%d", _this->m_MagicNumber, __FUNCTION__, _this->m_i4CurrSensorDev);

        //thread control
        {
            int Val;
            ::sem_getvalue( &_this->m_semPDBufThdEnd, &Val);

            if( Val==0) //should be 0. 1 means PD task done(abnormal case).
            {
                CAM_LOGD_IF( _this->m_bDebugEnable,
                             "#(%d) [%s] semPDThdEnd before post is [0] : normal case",
                             _this->m_MagicNumber,
                             __FUNCTION__);

                //
                ::sem_post(&_this->m_semPDBufThdEnd);
            }
            else
            {
                CAM_LOGE( "#(%d) [%s] semPDThdEnd before post is [%d] : Abnormal case. Something wrong is happened during about multi thread communication !!",
                          _this->m_MagicNumber,
                          __FUNCTION__,
                          Val);
            }
        }

        //ready output data
        _this->m_Status = EPD_Data_Ready;

        //check pd flow status.
        if( ret==E_3A_ERR)
        {
            CAM_LOGE( "#(%d) [%s] PD calculation thread is colsed because of some is happened during calculation !!!", _this->m_MagicNumber, __FUNCTION__);
            break;
        }

        //
        if(_this->m_i4DbgSystraceLevel >= TRACE_LEVEL_DBG)
        {
            CAM_TRACE_FMT_END();
        }
    }

    _this->m_bEnPDBufThd = MFALSE;
    _this->m_Status      = EPD_Not_Enabled;


    CAM_LOGD( "[%s] thread end", __FUNCTION__);
    return NULL;
}

/************************************************************************/
/* Data path 1 : using protect PD algorithm                             */
/************************************************************************/
MRESULT
PDMgr::SetCaliData2PDCore(PD_ALGO_TUNING_T* ptrInTuningData,
                          PD_CALIBRATION_DATA_T* ptrInCaliData)
{
    CAM_LOGD( "[%s] +", __FUNCTION__);

    //
    MRESULT ret    = E_3A_ERR;
    MRESULT retAll = S_3A_OK;

    //
    PD_INIT_T initPdData;

    //get calibration data
    initPdData.rPDNVRAM.rCaliData.i4Size = ptrInCaliData->i4Size;
    memcpy( initPdData.rPDNVRAM.rCaliData.uData, ptrInCaliData->uData, ptrInCaliData->i4Size);

    //get tuning data from host
    memcpy( &initPdData.rPDNVRAM.rTuningData, ptrInTuningData, sizeof(PD_ALGO_TUNING_T));

    //
    if( m_bDebugEnable)
    {
        FILE *fp = NULL;

        //
        fp = fopen("/sdcard/pdo/__pd_cali_data.bin", "w");
        if( fp!=NULL)
        {
            fwrite( reinterpret_cast<void *>(initPdData.rPDNVRAM.rCaliData.uData), 1, PD_CALI_DATA_SIZE, fp);
            fclose( fp);

            //
            CAM_LOGD( "[%s] calibration data is dumpped to /sdcard/pdo/__pd_cali_data.bin (size=%d)",
                      __FUNCTION__,
                      PD_CALI_DATA_SIZE);
        }
        else
        {
            CAM_LOGW( "[%s] can not dump calibration data to /sdcard/pdo/__pd_cali_data.bin !!", __FUNCTION__);
        }

        //
        fp = fopen("/sdcard/pdo/__pd_tuning_data.bin", "w");
        if( fp!=NULL)
        {
            fwrite( reinterpret_cast<void *>(&initPdData.rPDNVRAM.rTuningData), 1, sizeof(PD_ALGO_TUNING_T), fp);
            fclose( fp);

            //
            CAM_LOGD( "[%s] tuning data is dumpped to /sdcard/pdo/__pd_tuning_data.bin (size=%lu)",
                      __FUNCTION__,
                      sizeof(PD_ALGO_TUNING_T));
        }
        else
        {
            CAM_LOGW( "[%s] can not dump tuning data to /sdcard/pdo/__pd_tuning_data.bin !!", __FUNCTION__);
        }
    }


    CAM_LOGD( "[%s] tuning data(size=%zu) : ConfThr=%d, SaturateLevel=%d, i4SaturateThr=%d, FocusPDSizeX=%d, FocusPDSizeY=%d",
              __FUNCTION__,
              sizeof(NVRAM_LENS_PARA_STRUCT),
              initPdData.rPDNVRAM.rTuningData.i4ConfThr,
              initPdData.rPDNVRAM.rTuningData.i4SaturateLevel,
              initPdData.rPDNVRAM.rTuningData.i4SaturateThr,
              initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeX,
              initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeY);

    char  strMsgBuf[256];
    char *strTarget = strMsgBuf;

    strTarget += sprintf(strTarget, "[%s] ConfIdx1( ", __FUNCTION__);
    for (MINT32 i=0; i<PD_CONF_IDX_SIZE; i++)
        strTarget += sprintf(strTarget, "%d ", initPdData.rPDNVRAM.rTuningData.i4ConfIdx1[i]);
    strTarget += sprintf(strTarget, ") ");

    strTarget += sprintf(strTarget, "[%s] ConfIdx2( ", __FUNCTION__);
    for (MINT32 i=0; i<PD_CONF_IDX_SIZE; i++)
        strTarget += sprintf(strTarget, "%d ", initPdData.rPDNVRAM.rTuningData.i4ConfIdx2[i]);
    strTarget += sprintf(strTarget, ") ");
    CAM_LOGD( "%s", strMsgBuf);

    for( MINT32 i=0; i<(PD_CONF_IDX_SIZE+1); i++)
    {
        strTarget = strMsgBuf;
        strTarget += sprintf(strTarget, "[%s] ConfTbl %02d( ",__FUNCTION__, i);
        for (MINT32 j=0; j<(PD_CONF_IDX_SIZE+1); j++)
            strTarget += sprintf(strTarget, "%3d ", initPdData.rPDNVRAM.rTuningData.i4ConfTbl[i][j]);
        strTarget += sprintf(strTarget, ")");
        CAM_LOGD( "%s", strMsgBuf);
    }

    PD_CONFIG_T pd_block_info_cali;

    //(1) set calibration data to PD core.
    ret = m_pIPdAlgo->initPD(initPdData, &pd_block_info_cali);
    if(ret==S_3A_OK)
    {
        CAM_LOGD( "[%s] InitPD pass. Sensor type:%d. Calibration data info: cali raw size(%d, %d). PD block info: offset(%d, %d), pitch size(%d, %d), sub block size(%d, %d), block number(%d, %d), pair number(%d)",
                  __FUNCTION__,
                  pd_block_info_cali.i4SensorType,
                  pd_block_info_cali.i4RawWidth,
                  pd_block_info_cali.i4RawHeight,
                  pd_block_info_cali.sPdBlockInfo.i4OffsetX,
                  pd_block_info_cali.sPdBlockInfo.i4OffsetY,
                  pd_block_info_cali.sPdBlockInfo.i4PitchX,
                  pd_block_info_cali.sPdBlockInfo.i4PitchY,
                  pd_block_info_cali.sPdBlockInfo.i4SubBlkW,
                  pd_block_info_cali.sPdBlockInfo.i4SubBlkH,
                  pd_block_info_cali.sPdBlockInfo.i4BlockNumX,
                  pd_block_info_cali.sPdBlockInfo.i4BlockNumY,
                  pd_block_info_cali.sPdBlockInfo.i4PairNum);
    }
    else
    {
        CAM_LOGE( "[%s] initPD fail. PD block info which is described in alibration data and kernel driver is mismatch. Please check image sensor kernel driver setting or cam_cal setting!!", __FUNCTION__);

        for( MINT32 kidx=0; kidx<PD_CALI_DATA_SIZE; kidx=kidx+32)
        {
            strTarget = strMsgBuf;
            for (MINT i=0; i<32; i++)
                strTarget += sprintf(strTarget, "0x%02x ", initPdData.rPDNVRAM.rCaliData.uData[kidx+i]);
            CAM_LOGE( "%s", strMsgBuf);
        }
        retAll = E_3A_ERR;
    }

    //(2) configure pd orientation information.
    ePDWIN_ORIENTATION_T PDOrientation;
    switch( m_PDBlockInfo.iMirrorFlip)
    {
        /* 0:IMAGE_NORMAL,1:IMAGE_H_MIRROR,2:IMAGE_V_MIRROR,3:IMAGE_HV_MIRROR*/
        case IMAGE_HV_MIRROR:
            PDOrientation = ePDWIN_M1F1;
            break;
        case IMAGE_V_MIRROR:
            PDOrientation = ePDWIN_M0F1;
            break;
        case IMAGE_H_MIRROR:
            PDOrientation = ePDWIN_M1F0;
            break;
        case IMAGE_NORMAL:
        default :
            PDOrientation = ePDWIN_M0F0;
            break;
    }
    CAM_LOGD( "[%s] mirror flip info : kernel(%d):{[0]IMAGE_NORMAL [1]IMAGE_H_MIRROR [2]IMAGE_V_MIRROR [3]IMAGE_HV_MIRROR} set to algo(%d):{[0]ePDWIN_M0F0 [1]ePDWIN_M0F1 [2]ePDWIN_M1F0 [3]ePDWIN_M1F1}",
              __FUNCTION__,
              m_PDBlockInfo.iMirrorFlip,
              PDOrientation);

    if ((ret = m_pIPdAlgo->setPDOrientation(PDOrientation)) != S_3A_OK)
        retAll = E_3A_ERR;


    //(3) configure pd algorithm.
    PD_CONFIG_T algCfg;
    memset( &algCfg, 0, sizeof(PD_CONFIG_T));

    algCfg.i4RawWidth          = m_profile.uImgXsz;
    algCfg.i4RawHeight         = m_profile.uImgYsz;
    switch(pd_block_info_cali.i4SensorType)
    {
        case ePDSENS_4CELL_BIN:
            algCfg.i4FullRawWidth      = pd_block_info_cali.i4RawWidth;
            algCfg.i4FullRawHeight     = pd_block_info_cali.i4RawHeight;
            // the FullRawOffset is described as the coordinator of first (0,0) pixle of current sensor mode relative to the sensor mode druing calibration.
            algCfg.i4FullRawXOffset    = (MINT32)(m_PDBlockInfo.i4Crop[m_profile.i4SensorMode][0]);
            algCfg.i4FullRawYOffset    = (MINT32)(m_PDBlockInfo.i4Crop[m_profile.i4SensorMode][1]);
            break;

        default:
            algCfg.i4FullRawWidth      = m_SensorCropInfo.full_w;
            algCfg.i4FullRawHeight     = m_SensorCropInfo.full_h;
            // the FullRawOffset is described as the coordinator of first (0,0) pixle of current sensor mode in the sensor's active area.
            algCfg.i4FullRawXOffset    = ((m_SensorCropInfo.x2_tg_offset+m_SensorCropInfo.x1_offset)* m_SensorCropInfo.w0_size / m_SensorCropInfo.scale_w) + m_SensorCropInfo.x0_offset;
            algCfg.i4FullRawYOffset    = ((m_SensorCropInfo.y2_tg_offset+m_SensorCropInfo.y1_offset)* m_SensorCropInfo.h0_size / m_SensorCropInfo.scale_h) + m_SensorCropInfo.y0_offset;
            break;
    }
    if( /* */
        (algCfg.i4FullRawXOffset!=(MINT32)(m_PDBlockInfo.i4Crop[m_profile.i4SensorMode][0])) ||
        (algCfg.i4FullRawYOffset!=(MINT32)(m_PDBlockInfo.i4Crop[m_profile.i4SensorMode][1])))
    {
        CAM_LOGW("Calculing FullRawOffset is fail:(%d,%d)!=(%d,%d). crop information(%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d)",
                 algCfg.i4FullRawXOffset,
                 algCfg.i4FullRawYOffset,
                 m_PDBlockInfo.i4Crop[m_profile.i4SensorMode][0],
                 m_PDBlockInfo.i4Crop[m_profile.i4SensorMode][1],
                 m_SensorCropInfo.full_w,
                 m_SensorCropInfo.full_h,
                 m_SensorCropInfo.x0_offset,
                 m_SensorCropInfo.y0_offset,
                 m_SensorCropInfo.w0_size,
                 m_SensorCropInfo.h0_size,
                 m_SensorCropInfo.scale_w,
                 m_SensorCropInfo.scale_h,
                 m_SensorCropInfo.x1_offset,
                 m_SensorCropInfo.y1_offset,
                 m_SensorCropInfo.w1_size,
                 m_SensorCropInfo.h1_size,
                 m_SensorCropInfo.x2_tg_offset,
                 m_SensorCropInfo.y2_tg_offset,
                 m_SensorCropInfo.w2_tg_size,
                 m_SensorCropInfo.h2_tg_size);
    }



#define USE_DRV_PD_COORDINATOR 1  //[TODO] set as 0
#if USE_DRV_PD_COORDINATOR
    algCfg.i4DRVBlockInfoUseEn = 1;
#else
    algCfg.i4DRVBlockInfoUseEn = 0;
#endif

    switch( m_profile.u4PDAF_support_type)
    {
        case PDAF_SUPPORT_RAW_DUALPD:
        {
            //
            algCfg.sDPdFormat.i4Bits      = m_sPDOHWInfo.u4BitDepth;
            algCfg.sDPdFormat.i4BinningX  = m_sPDOHWInfo.u4BinRatioX;
            algCfg.sDPdFormat.i4BinningY  = m_sPDOHWInfo.u4BinRatioY;
            algCfg.sDPdFormat.i4BufFmt    = (m_sPDOHWInfo.u1PdSeparateMode == 1) ? 1 : 0;
            //
            algCfg.sDPdFormat.i4BufStride = _PDO_STRIDE_ALIGN_(m_sPDOHWInfo.u4Pdo_xsz+1);
            algCfg.sDPdFormat.i4BufHeight = m_sPDOHWInfo.u4Pdo_ysz + 1;
        }
        break;

        case PDAF_SUPPORT_CAMSV_DUALPD:
        {
            //
            m_pPDBufMgrCore->GetDualPDVCInfo( 0, m_sDualPDVCInfo, m_profile.AETargetMode);

            algCfg.sDPdFormat.i4Bits     = 10;
            //
            algCfg.sDPdFormat.i4BinningX = m_sDualPDVCInfo.u4VCBinningX;
            algCfg.sDPdFormat.i4BinningY = m_sDualPDVCInfo.u4VCBinningY;
            algCfg.sDPdFormat.i4BufFmt   = m_sDualPDVCInfo.u4VCBufFmt;
            //
            algCfg.sDPdFormat.i4BufStride = m_PDVCInfo.VC_SIZEH_BYTE;
            algCfg.sDPdFormat.i4BufHeight = m_PDVCInfo.VC_SIZEV;
        }
        break;

        case PDAF_SUPPORT_RAW:
        case PDAF_SUPPORT_CAMSV:
        case PDAF_SUPPORT_CAMSV_LEGACY:
        case PDAF_SUPPORT_RAW_LEGACY:
        default:
        {
            /* Condition checking*/
            if( m_PDBlockInfo.i4BlockNumX == 0 && m_PDBlockInfo.i4BlockNumY == 0)
            {
                CAM_LOGE( "Please check block number setting in kernel driver: i4BlockNumX(%d) i4BlockNumY(%d)", m_PDBlockInfo.i4BlockNumX, m_PDBlockInfo.i4BlockNumY);
                retAll = E_3A_ERR;
            }

            if(m_PDBlockInfo.i4PairNum > MAX_PD_PAIR_NUM)
            {
                CAM_LOGE( "Please check pair number setting in kernel driver: i4PairNum(%d)", m_PDBlockInfo.i4PairNum);
                m_PDBlockInfo.i4PairNum = MAX_PD_PAIR_NUM;
            }

            algCfg.i4IsPacked  = 1;
            algCfg.i4Bits      = 10; /* pSttData->mImgoBitsPerPixel for PDAF_SUPPORT_RAW_LEGACY*/
            algCfg.i4RawStride = m_profile.uImgXsz*algCfg.i4Bits/8;
            algCfg.sPdBlockInfo.i4BlockNumX = m_PDBlockInfo.i4BlockNumX;
            algCfg.sPdBlockInfo.i4BlockNumY = m_PDBlockInfo.i4BlockNumY;
            algCfg.sPdBlockInfo.i4PairNum   = m_PDBlockInfo.i4PairNum;
            algCfg.sPdBlockInfo.i4PitchX    = m_PDBlockInfo.i4PitchX;
            algCfg.sPdBlockInfo.i4PitchY    = m_PDBlockInfo.i4PitchY;
            algCfg.sPdBlockInfo.i4SubBlkH   = m_PDBlockInfo.i4SubBlkH;
            algCfg.sPdBlockInfo.i4SubBlkW   = m_PDBlockInfo.i4SubBlkW;

#if USE_DRV_PD_COORDINATOR
            /*******************************************************************************
             * Reference mirrorflip information to modify PD block information.
             * The i4OffsetX, i4OffsetY, i4PosL and i4PosR are calculated in full raw domain without considering crop information.
             *******************************************************************************/
            unsigned int pitch_x    = algCfg.sPdBlockInfo.i4PitchX;
            unsigned int pitch_y    = algCfg.sPdBlockInfo.i4PitchY;
            unsigned int nblk_x     = algCfg.sPdBlockInfo.i4BlockNumX;
            unsigned int nblk_y     = algCfg.sPdBlockInfo.i4BlockNumY;
            unsigned int full_lOffx = m_PDBlockInfo.i4OffsetX;
            unsigned int full_lOffy = m_PDBlockInfo.i4OffsetY;

            // Current image related full size coordinate
            unsigned int crop_x = algCfg.i4FullRawXOffset;
            unsigned int crop_y = algCfg.i4FullRawYOffset;

            // Current pd block offset related to full size coordinate
#if 0
            int shift_x = crop_x-full_lOffx;
            if( shift_x<=0)
            {
                shift_x = -crop_x;
            }
            else if( shift_x%pitch_x)
            {
                shift_x = ( (shift_x+(pitch_x-1)) / pitch_x) * pitch_x - crop_x;
            }
            else
            {
                shift_x = ( shift_x / pitch_x) * pitch_x - crop_x;
            }


            int shift_y = crop_y-full_lOffy;
            if( shift_y<=0)
            {
                shift_y = -crop_y;
            }
            else if( shift_y%pitch_y)
            {
                shift_y = ( (shift_y+(pitch_y-1)) / pitch_y) * pitch_y - crop_y;
            }
            else
            {
                shift_y = ( shift_y / pitch_y) * pitch_y - crop_y;
            }
            CAM_LOGD( "coordinate shift in current sensor mode (%d, %d)", shift_x, shift_y);
#else
            // shift value is not considered for algorithm's input offset and coordinator
            int shift_x = 0;
            int shift_y = 0;
#endif

            /*******************************************************************************
             * calculate pd pixels' position by orientation and crop information for general separate function
             *******************************************************************************/
            unsigned int cur_lOffx = full_lOffx + shift_x;
            unsigned int cur_lOffy = full_lOffy + shift_y;
            unsigned int cur_rOffx = algCfg.i4FullRawWidth  - cur_lOffx - pitch_x * nblk_x;
            unsigned int cur_rOffy = algCfg.i4FullRawHeight - cur_lOffy - pitch_y * nblk_y;
            unsigned int offx  = (m_PDBlockInfo.iMirrorFlip & 0x1) ? cur_rOffx : cur_lOffx;
            unsigned int offy  = (m_PDBlockInfo.iMirrorFlip & 0x2) ? cur_rOffy : cur_lOffy;
            //
            algCfg.sPdBlockInfo.i4OffsetX   = offx;
            algCfg.sPdBlockInfo.i4OffsetY   = offy;

            CAM_LOGD("mirror_flip(%x), block offset : left side(%d, %d) right sied(%d, %d), offset set to algo(%d, %d)",
                     m_PDBlockInfo.iMirrorFlip,
                     cur_lOffx,
                     cur_lOffy,
                     cur_rOffx,
                     cur_rOffy,
                     algCfg.sPdBlockInfo.i4OffsetX,
                     algCfg.sPdBlockInfo.i4OffsetY);

            //
            MUINT32 pairNum = algCfg.sPdBlockInfo.i4PairNum;
            MUINT32 pdPixels[MAX_PAIR_NUM * 2][2];
            for( MUINT32 Pidx=0; Pidx<pairNum; Pidx++)
            {
                MUINT32 PosL_X = m_PDBlockInfo.i4PosL[Pidx][0] + shift_x;
                MUINT32 PosL_Y = m_PDBlockInfo.i4PosL[Pidx][1] + shift_y;
                MUINT32 PosR_X = m_PDBlockInfo.i4PosR[Pidx][0] + shift_x;
                MUINT32 PosR_Y = m_PDBlockInfo.i4PosR[Pidx][1] + shift_y;

                if( m_PDBlockInfo.iMirrorFlip & 0x1) //mirror
                {
                    PosL_X = pitch_x - (PosL_X - cur_lOffx) - 1 + cur_rOffx;
                    PosR_X = pitch_x - (PosR_X - cur_lOffx) - 1 + cur_rOffx;
                }
                if( m_PDBlockInfo.iMirrorFlip & 0x2) //flip
                {
                    PosL_Y = pitch_y - (PosL_Y - cur_lOffy) - 1 + cur_rOffy;
                    PosR_Y = pitch_y - (PosR_Y - cur_lOffy) - 1 + cur_rOffy;
                }
                pdPixels[Pidx][0] = PosR_X;
                pdPixels[Pidx][1] = PosR_Y;
                pdPixels[Pidx + pairNum][0] = PosL_X;
                pdPixels[Pidx + pairNum][1] = PosL_Y;

                CAM_LOGD( "[%s] pd coordinator [L](%3d, %3d)->(%3d, %3d), [R](%3d, %3d)->(%3d, %3d)",
                          __FUNCTION__,
                          m_PDBlockInfo.i4PosL[Pidx][0],
                          m_PDBlockInfo.i4PosL[Pidx][1],
                          PosL_X,
                          PosL_Y,
                          m_PDBlockInfo.i4PosR[Pidx][0],
                          m_PDBlockInfo.i4PosR[Pidx][1],
                          PosR_X,
                          PosR_Y);

                MUINT32 idx = (m_PDBlockInfo.iMirrorFlip & 0x2) ? ((pairNum - 1) - Pidx) : Pidx;
                algCfg.sPdBlockInfo.i4PosR[idx][0] = pdPixels[Pidx][0];
                algCfg.sPdBlockInfo.i4PosR[idx][1] = pdPixels[Pidx][1];
                algCfg.sPdBlockInfo.i4PosL[idx][0] = pdPixels[Pidx + pairNum][0];
                algCfg.sPdBlockInfo.i4PosL[idx][1] = pdPixels[Pidx + pairNum][1];
            }

#else
            for( MUINT32 Pidx=0; Pidx<algCfg.sPdBlockInfo.i4PairNum; Pidx++)
            {
                algCfg.sPdBlockInfo.i4PosR[Pidx][0] = m_PDBlockInfo.i4PosR[Pidx][0];
                algCfg.sPdBlockInfo.i4PosR[Pidx][1] = m_PDBlockInfo.i4PosR[Pidx][1];
                algCfg.sPdBlockInfo.i4PosL[Pidx][0] = m_PDBlockInfo.i4PosL[Pidx][0];
                algCfg.sPdBlockInfo.i4PosL[Pidx][1] = m_PDBlockInfo.i4PosL[Pidx][1];

                CAM_LOGD( "[%s] pd coordinator [L](%3d, %3d), [R](%3d, %3d)",
                          __FUNCTION__,
                          m_PDBlockInfo.i4PosL[Pidx][0],
                          m_PDBlockInfo.i4PosL[Pidx][1],
                          m_PDBlockInfo.i4PosR[Pidx][0],
                          m_PDBlockInfo.i4PosR[Pidx][1]);
            }
#endif
        }
        break;
    }

    CAM_LOGD( "[%s] configuration for algorithm : rawInfo(w[%d],h[%d],stride[%d],bits[%d],isPack[%d]) BlockInfo(blknum[%d,%d],offset[%d,%d],pairNum[%d],pitchSz[%d,%d],subblkSz[%d,%d]) fullRawOffset(%d,%d) fullRawSize(%d,%d) DualPdFormat(bufStride[0x%x],bufHeight(0x%x),bufFmt[%d],bits[%d],binning[%d,%d])",
              __FUNCTION__,
              algCfg.i4RawWidth,
              algCfg.i4RawHeight,
              algCfg.i4RawStride,
              algCfg.i4Bits,
              algCfg.i4IsPacked,
              algCfg.sPdBlockInfo.i4BlockNumX,
              algCfg.sPdBlockInfo.i4BlockNumY,
              algCfg.sPdBlockInfo.i4OffsetX,
              algCfg.sPdBlockInfo.i4OffsetY,
              algCfg.sPdBlockInfo.i4PairNum,
              algCfg.sPdBlockInfo.i4PitchX,
              algCfg.sPdBlockInfo.i4PitchY,
              algCfg.sPdBlockInfo.i4SubBlkH,
              algCfg.sPdBlockInfo.i4SubBlkW,
              algCfg.i4FullRawXOffset,
              algCfg.i4FullRawYOffset,
              algCfg.i4FullRawWidth,
              algCfg.i4FullRawHeight,
              algCfg.sDPdFormat.i4BufStride,
              algCfg.sDPdFormat.i4BufHeight,
              algCfg.sDPdFormat.i4BufFmt,
              algCfg.sDPdFormat.i4Bits,
              algCfg.sDPdFormat.i4BinningX,
              algCfg.sDPdFormat.i4BinningY);


    // General separate function needs block information.
    if( m_pPDBufMgrCore != NULL)
    {
        m_pPDBufMgrCore->SetPDInfo( m_PDBlockInfo, m_profile, m_sPDOHWInfo);
    }

    // (4) check configure core PD algorithm is correct or not.
    if( (ret = m_pIPdAlgo->setPDBlockInfo(algCfg)) != S_3A_OK)
    {
        CAM_LOGD("[Core] PD init data error");
        retAll = E_3A_ERR;
    }

    if( retAll == S_3A_OK)
        CAM_LOGD( "[%s] - configure PD algo done", __FUNCTION__);
    else
        CAM_LOGE( "[%s] - configure PD algo failed !! close PD HAL flow !!", __FUNCTION__);

    return retAll;
}

//
MRESULT
PDMgr::PDCoreFlow()
{
    Mutex::Autolock lock(m_Lock);

    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] +", m_MagicNumber, __FUNCTION__);

    MRESULT ret=S_3A_OK;

    if( m_pIPdAlgo && m_pAnalysisData_Calculate)
    {
        SPDResult_T PDRes;
        //reset output result.
        memset( &PDRes, 0, sizeof(SPDResult_T));

        if( /* output all 0  result directly once current lens position is 0*/
            ( 0 < m_lensInfo.u4CurrentPosition                        ) &&
            (     m_lensInfo.u4CurrentPosition <= 1023                ) &&
            ( 0 < m_pAnalysisData_Calculate->numDa                    ) &&
            (     m_pAnalysisData_Calculate->numDa<=MAX_SIZE_OF_PD_ROI) )
        {

            if( !m_i4DbgDisPdHandle)
            {
                PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "PDCoreFlow");

                // set output data puffer.
                PDRes.magicNumber = m_MagicNumber;
                PDRes.numROIs     = m_pAnalysisData_Calculate->numDa;

                PD_INPUT_T *aHandlePDInput[AF_PSUBWIN_NUM];
                PD_OUTPUT_T *aHandlePDOutput[AF_PSUBWIN_NUM],sHandlePDOutputList[AF_PSUBWIN_NUM];
                memset(sHandlePDOutputList,0,sizeof(PD_OUTPUT_T)*m_pAnalysisData_Calculate->numDa);
                for (MUINT32 i=0; i<m_pAnalysisData_Calculate->numDa; i++)
                {
                    aHandlePDInput[i] = &m_pAnalysisData_Calculate->Da[i].sHandlePDInput;
                    aHandlePDOutput[i] = &sHandlePDOutputList[i];
                }
                PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "handleAllPD");
                ret = m_pIPdAlgo->handleAllPD(m_pAnalysisData_Calculate->numDa, aHandlePDInput, aHandlePDOutput);
                PD_TRACE_FMT_END(TRACE_LEVEL_DEF);

                // calculating multi PD windowns
                for( MUINT32 i=0; i<m_pAnalysisData_Calculate->numDa; i++)
                {
                    /*
                    if( (MUINT32)(m_pAnalysisData_Calculate->Da[i].sHandlePDInput.i4CurLensPos) != m_lensInfo.u4CurrentPosition)
                    {
                        CAM_LOGE( "#(%d) [%s] lens position is mismatch (%3d != %3d), Something wrong of multi thread mechanism!!, Please reproduce issue with setting property vendor.debug.af_mgr.enable as 1 and provied log file to RD.",
                                  m_MagicNumber,
                                  __FUNCTION__,
                                  m_pAnalysisData_Calculate->Da[i].sHandlePDInput.i4CurLensPos,
                                  m_lensInfo.u4CurrentPosition);

                    }
                    */

                    PD_OUTPUT_T &sPDhandleOutput = sHandlePDOutputList[i];

                    CAM_LOGD_IF( m_bDebugEnable,
                                 "#(%d) [%s] handlePD : win#%d Confidence(%3d) LensPos[%4d(%4d)->%4d] PD value(%7d), SatPercent(%3d), SensorAGain(%d), ROI(x[%4d->%4d],y[%4d->%4d],w[%4d->%4d],h[%4d->%4d]), BufferAddress(%p, %p, %p, %p)",
                                 m_MagicNumber,
                                 __FUNCTION__,
                                 i,
                                 sPDhandleOutput.i4ConfidenceLevel,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.i4CurLensPos,
                                 m_lensInfo.u4CurrentPosition,
                                 sPDhandleOutput.i4FocusLensPos,
                                 (MINT32)(sPDhandleOutput.fPdValue*1000),
                                 sPDhandleOutput.i4SatPercent,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.i4SensorAGain,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4X,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4X,//refined focus PD window (refer to PD image coordinate));
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4Y,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4Y,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4W,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4W,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.sPdWin.i4H,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.sPdBlk.i4H,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.pPDLData,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.pPDRData,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.pPDLPos,
                                 m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData.pPDRPos);

                    if( ret==E_3A_ERR)
                    {
                        CAM_LOGE("#(%d) [%s] handlePD error at window#%d !! Please contact pdalgo owner !!", m_MagicNumber, __FUNCTION__, i);
                    }

                    //set output data puffer.
                    PDRes.ROIRes[i].CurLensPos        = m_lensInfo.u4CurrentPosition;
                    PDRes.ROIRes[i].DesLensPos        = Boundary(1, sPDhandleOutput.i4FocusLensPos, 1023);
                    PDRes.ROIRes[i].Confidence        = sPDhandleOutput.i4ConfidenceLevel;
                    PDRes.ROIRes[i].ConfidenceLevel   = sPDhandleOutput.i4ConfidenceLevel;
                    PDRes.ROIRes[i].PhaseDifference   = (MINT32)(sPDhandleOutput.fPdValue*1000);
                    PDRes.ROIRes[i].sROIInfo          = m_pAnalysisData_Calculate->Da[i].sRoiInfo;
                    PDRes.ROIRes[i].SaturationPercent = sPDhandleOutput.i4SatPercent;
                }
                PD_TRACE_FMT_END(TRACE_LEVEL_DBG);
            }
            else
            {
                CAM_LOGW( "#(%d) [%s] handlePD is disabled by debug command vendor.debug.handlepd.disable.", m_MagicNumber, __FUNCTION__);
                ret = S_3A_OK;
            }

        }
        else
        {
            CAM_LOGW( "#(%d) [%s] Calculation task is not executed, lens pos:%d(should be:0<value<1024), number of ROI:%d(should be:0<value<=MAX_SIZE_OF_PD_ROI(%d))",
                      m_MagicNumber,
                      __FUNCTION__,
                      m_lensInfo.u4CurrentPosition,
                      m_pAnalysisData_Calculate->numDa,
                      MAX_SIZE_OF_PD_ROI);
        }


        if( ret==S_3A_OK)
        {
            Mutex::Autolock lock(m_Lock_Result);

            //
            m_vPDOutput.push_back( PDRes);

            //
            if( 1<m_vPDOutput.size())
            {
                //pop_front and keep only last 2 result
                m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
                CAM_LOGD("pop_front, sz=%zu", m_vPDOutput.size());
            }

            //
            if(m_vPDOutput.size())
            {
                vector<SPDResult_T>::iterator itr = m_vPDOutput.begin();

                itr += m_vPDOutput.size()-1;

                char  dbgMsgBuf[DBG_MSG_BUF_SZ];
                char* ptrMsgBuf = dbgMsgBuf;

                //
                ptrMsgBuf += sprintf( ptrMsgBuf,
                                      "#(%d) [%s] numRes(%d) : ",
                                      itr->magicNumber,
                                      __FUNCTION__,
                                      itr->numROIs);
                for( MUINT32 i=0; i<itr->numROIs; i++)
                {
                    ptrMsgBuf += sprintf( ptrMsgBuf,
                                          "(%d/%d/%d/%d/%d/%d/%d/%d/%ld/%d) ",
                                          itr->ROIRes[i].sROIInfo.sType,
                                          itr->ROIRes[i].sROIInfo.sPDROI.i4X,
                                          itr->ROIRes[i].sROIInfo.sPDROI.i4Y,
                                          itr->ROIRes[i].sROIInfo.sPDROI.i4W,
                                          itr->ROIRes[i].sROIInfo.sPDROI.i4H,
                                          itr->ROIRes[i].CurLensPos,
                                          itr->ROIRes[i].DesLensPos,
                                          itr->ROIRes[i].Confidence,
                                          itr->ROIRes[i].PhaseDifference,
                                          itr->ROIRes[i].SaturationPercent);
                }
                CAM_LOGD( "%s", dbgMsgBuf);
            }

        }
    }


    // verification flow
    if( m_i4DbgPDVerifyEn)
    {
        PDVerificationFlow();
    }

    //
    {
        Mutex::Autolock lock( m_Lock_AnalysisData);

        //
        CAM_LOGD_IF( m_bDebugEnable,
                     "#(%d) [%s] data buffer (%p) is processed. ping-pong update buffer and calculation buffer : update buffer as (%p), calculation buffer as(%p)",
                     m_MagicNumber,
                     __FUNCTION__,
                     m_pAnalysisData_Calculate,
                     m_pAnalysisData_Calculate,
                     m_pAnalysisData_Update);

        //
        S_ANALYSIS_DATA_T *pTmp = m_pAnalysisData_Calculate;
        m_pAnalysisData_Calculate = m_pAnalysisData_Update;
        m_pAnalysisData_Update = pTmp;
    }

    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] -", m_MagicNumber, __FUNCTION__);
    return ret;

}

/************************************************************************/
/* Data path 2 : using 3rd party PD algorithm                           */
/************************************************************************/
MRESULT
PDMgr::SetCaliData2PDOpenCore(PD_ALGO_TUNING_T* /*ptrInTuningData*/,
                              PD_CALIBRATION_DATA_T* ptrInCaliData)
{
    CAM_LOGD("[%s] +", __FUNCTION__);

    MRESULT ret=E_3A_ERR;

    if( m_pPDBufMgrOpen)
    {

        PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "SetCaliData2PDOpenCore");

        // (1) query sensor driver setting for 3rd party PD library.
        MUINT32  arg1=0;
        MUINT16 *arg2=NULL;
        if( m_pPDBufMgrOpen->sendCommand( PDBUFMGR_OPEN_CMD_GET_REG_SETTING_LIST, (MVOID *)(&arg1), (MVOID *)(&arg2), NULL))
        {
            if( m_pIHalSensor)
            {
                m_pIHalSensor->sendCommand( m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_REG_SETTING, (MUINTPTR)&arg1, (MUINTPTR)&arg2, 0);
            }
        }

        // (2) set calibration to PD open core.
        m_pPDBufMgrOpen->SetCalibrationData( ptrInCaliData->i4Size, ptrInCaliData->uData);

        if( m_bDebugEnable)
        {
            FILE *fp = fopen("/sdcard/pdo/__pd_cali_data_open.bin", "w");
            if( fp!=NULL)
            {
                fwrite( reinterpret_cast<void *>(ptrInCaliData->uData), 1, ptrInCaliData->i4Size, fp);
                fclose( fp);
            }
            else
            {
                CAM_LOGW( "[%s] can not dump calibration data to /sdcard/pdo/__pd_cali_data_open.bin !!", __FUNCTION__);
            }
        }

        //
        ret = S_3A_OK;

        PD_TRACE_FMT_END(TRACE_LEVEL_DEF);
    }

    CAM_LOGD("[%s] -", __FUNCTION__);
    return ret;
}

//
MRESULT
PDMgr::PDOpenCoreFlow()
{
    Mutex::Autolock lock(m_Lock);

    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] +", m_MagicNumber, __FUNCTION__);

    SPDROIInput_T  iPDInputData;
    SPDROIResult_T oPdOutputData;

    PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "DaBuf2PDBufMgr");
    m_pPDBufMgrOpen->SetDataBuf( m_databuf_size, m_databuf, m_MagicNumber);
    PD_TRACE_FMT_END(TRACE_LEVEL_DEF);

    SPDResult_T PDRes;

    //reset output result.
    memset( &PDRes, 0, sizeof(SPDResult_T));


    if( /* Output all 0  result directly once current lens position is not vaild */
        ( 0<=m_lensInfo.u4CurrentPosition      ) &&
        (    m_lensInfo.u4CurrentPosition<=1023))
    {
        // query sensor driver setting for 3rd party PD library.
        MINT32  mode = 0;
        MUINT32 cfgNum = 0;

        PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "SendCmd(%d)ToPDBufMgr", PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO);
        MBOOL res  = m_pPDBufMgrOpen->sendCommand( PDBUFMGR_OPEN_CMD_GET_CUR_BUF_SETTING_INFO, (MVOID *)(&mode), (MVOID *)(&cfgNum));
        PD_TRACE_FMT_END(TRACE_LEVEL_DEF);

        PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "PDOpenCoreFlow");
        {
            Mutex::Autolock lock(m_Lock_AnalysisData);

            // Debug only
            if( m_bDebugEnable)
            {
                CAM_LOGD( "DBG : dump configuration +, total size(%d)", m_sAnalysisDataOpen.totalsz);
                for( S_ANALYSIS_DATA_OPEN_T *ptr = m_sAnalysisDataOpen.tail; ptr!=NULL; ptr=ptr->prv)
                {
                    CAM_LOGD( "DBG : configuration(frmNum[%3d], cfgNum[0x%04x], number of ROIs[%d]",
                              ptr->frmNum,
                              ptr->cfgNum,
                              ptr->numROIs);
                }
                CAM_LOGD( "DBG : dump configuration -");
            }

            CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] res(%d) mode(%d)", m_MagicNumber, __FUNCTION__, res, mode);

            //
            if( m_sAnalysisDataOpen.totalsz)
            {
                // Find matched setting by configuration number.
                S_ANALYSIS_DATA_OPEN_T *cfg = NULL;
                if( /* sensor is configured as fiexible window mode*/
                    (res) &&
                    (mode==2))
                {
                    for( cfg = m_sAnalysisDataOpen.tail; cfg!=NULL; cfg=cfg->prv)
                    {

                        if( cfg->cfgNum==cfgNum)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    cfg = m_sAnalysisDataOpen.tail;
                    mode = 0;
                }

                CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] cfg(%p), current buffer setting : mode(%d), cfgNum(%d)", m_MagicNumber, __FUNCTION__, cfg, mode, cfgNum);

                if( cfg) /* match */
                {
                    CAM_LOGD_IF( m_bDebugEnable,
                                 "#(%d) [%s] mode(0x%x), buffer's cfgNum(0x%x) is found in configuration list(0x%04x), number of ROIs (%d) is configured",
                                 m_MagicNumber,
                                 __FUNCTION__,
                                 mode,
                                 cfgNum,
                                 cfg->cfgNum,
                                 cfg->numROIs);

                    PDRes.magicNumber = m_MagicNumber;
                    PDRes.numROIs     = cfg->numROIs;

                    for( MUINT32 i=0; i<cfg->numROIs; i++)
                    {
                        iPDInputData.curLensPos     = GetLensAdpCompPos( m_Sensor_pixel_clk, m_Sensor_line_length, m_exposureTime, m_time_stamp_sof, m_lensInfo, cfg->ROI[i].sRoiInfo.sPDROI);
                        iPDInputData.XSizeOfImage   = m_profile.uImgXsz;
                        iPDInputData.YSizeOfImage   = m_profile.uImgYsz;
                        iPDInputData.ROI.sPDROI.i4X = cfg->ROI[i].sRoiInfo.sPDROI.i4X;
                        iPDInputData.ROI.sPDROI.i4Y = cfg->ROI[i].sRoiInfo.sPDROI.i4Y;
                        iPDInputData.ROI.sPDROI.i4W = cfg->ROI[i].sRoiInfo.sPDROI.i4W;
                        iPDInputData.ROI.sPDROI.i4H = cfg->ROI[i].sRoiInfo.sPDROI.i4H;
                        iPDInputData.ROI.sType      = cfg->ROI[i].sRoiInfo.sType;
                        iPDInputData.cfgInfo        = mode ? cfg->ROI[i].cfgInfo : 0xFF;
                        iPDInputData.afegain        = m_afeGain;
                        iPDInputData.mode           = mode;

                        memset( &oPdOutputData, 0, sizeof(SPDROIResult_T));
                        PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "GetDefocus");
                        m_pPDBufMgrOpen->GetDefocus( iPDInputData, oPdOutputData);
                        PD_TRACE_FMT_END(TRACE_LEVEL_DEF);

                        PDRes.ROIRes[i].CurLensPos         = iPDInputData.curLensPos;
                        PDRes.ROIRes[i].DesLensPos         = Boundary(1, oPdOutputData.DesLensPos, 1023);
                        PDRes.ROIRes[i].Confidence         = oPdOutputData.Confidence;
                        PDRes.ROIRes[i].ConfidenceLevel    = oPdOutputData.ConfidenceLevel;
                        PDRes.ROIRes[i].PhaseDifference    = oPdOutputData.PhaseDifference;
                        PDRes.ROIRes[i].sROIInfo           = cfg->ROI[i].sRoiInfo;
                        PDRes.ROIRes[i].SaturationPercent  = 0; // TODO: oPdOutputData.SaturationPercent;

                        CAM_LOGD_IF( m_bDebugEnable,
                                     "#(%d) [%s] win#%d(%d/%d/%d/%d/%d) Confidence(%3d) LensPos(%4d/%4d->%4d) PD value(%7ld) SaturationPercent(%d)",
                                     m_MagicNumber,
                                     __FUNCTION__,
                                     i,
                                     PDRes.ROIRes[i].sROIInfo.sType,
                                     PDRes.ROIRes[i].sROIInfo.sPDROI.i4X,
                                     PDRes.ROIRes[i].sROIInfo.sPDROI.i4Y,
                                     PDRes.ROIRes[i].sROIInfo.sPDROI.i4W,
                                     PDRes.ROIRes[i].sROIInfo.sPDROI.i4H,
                                     PDRes.ROIRes[i].Confidence,
                                     PDRes.ROIRes[i].CurLensPos,
                                     m_lensInfo.u4CurrentPosition,
                                     PDRes.ROIRes[i].DesLensPos,
                                     PDRes.ROIRes[i].PhaseDifference,
                                     PDRes.ROIRes[i].SaturationPercent);
                    }

                    if( cfg==m_sAnalysisDataOpen.tail)
                    {
                        /* last element is found, clean all resource inside data list */
                        m_sAnalysisDataOpen.nxt  = NULL;
                        m_sAnalysisDataOpen.head = NULL;
                        m_sAnalysisDataOpen.tail = NULL;
                    }
                    else
                    {
                        /* clean resource from head to the matched element */
                        m_sAnalysisDataOpen.nxt  = cfg->nxt;
                        m_sAnalysisDataOpen.head = cfg->nxt;
                        m_sAnalysisDataOpen.head->prv = NULL;
                    }

                    /* clean resource */
                    while(1)
                    {
                        if( cfg==NULL)
                        {
                            CAM_LOGD_IF( m_bDebugEnable,
                                         "#(%d) [%s] delete resource, current list size(%d)",
                                         m_MagicNumber,
                                         __FUNCTION__,
                                         m_sAnalysisDataOpen.totalsz);
                            break;
                        }

                        S_ANALYSIS_DATA_OPEN_T *nxt_del_element = cfg->prv;
                        delete cfg;
                        m_sAnalysisDataOpen.totalsz--;
                        cfg = nxt_del_element;
                    }

                    {
                        Mutex::Autolock lock( m_Lock_Result);
                        //
                        m_vPDOutput.push_back( PDRes);

                        //
                        if( 1<m_vPDOutput.size())
                        {
                            //pop_front and keep only last 2 result
                            m_vPDOutput.erase( m_vPDOutput.begin(), m_vPDOutput.end()-1);
                            CAM_LOGD("#(%d) pop_front, sz=%zu", m_MagicNumber, m_vPDOutput.size());
                        }

                        //
                        if(m_vPDOutput.size())
                        {
                            vector<SPDResult_T>::iterator itr = m_vPDOutput.begin();

                            itr += m_vPDOutput.size()-1;

                            char  dbgMsgBuf[DBG_MSG_BUF_SZ];
                            char* ptrMsgBuf = dbgMsgBuf;

                            //
                            ptrMsgBuf += sprintf( ptrMsgBuf,
                                                  "#(%d) [%s] numRes(%d):",
                                                  itr->magicNumber,
                                                  __FUNCTION__,
                                                  itr->numROIs);
                            for( MUINT32 i=0; i<itr->numROIs; i++)
                            {
                                ptrMsgBuf += sprintf( ptrMsgBuf,
                                                      "(%d/%d/%d/%d/%d/%d/%d/%d/%ld/%d) ",
                                                      itr->ROIRes[i].sROIInfo.sType,
                                                      itr->ROIRes[i].sROIInfo.sPDROI.i4X,
                                                      itr->ROIRes[i].sROIInfo.sPDROI.i4Y,
                                                      itr->ROIRes[i].sROIInfo.sPDROI.i4W,
                                                      itr->ROIRes[i].sROIInfo.sPDROI.i4H,
                                                      itr->ROIRes[i].CurLensPos,
                                                      itr->ROIRes[i].DesLensPos,
                                                      itr->ROIRes[i].Confidence,
                                                      itr->ROIRes[i].PhaseDifference,
                                                      itr->ROIRes[i].SaturationPercent);

                            }
                            CAM_LOGD( "%s", dbgMsgBuf);
                        }


                    }
                }
            }
            else
            {
                CAM_LOGD("[W] Timing of updateing setting(AF_done) and receiving PD buffer(CAMSV_done) is not the same");
            }

        }
        PD_TRACE_FMT_END(TRACE_LEVEL_DBG);
    }


    // verification flow
    if( m_i4DbgPDVerifyEn)
    {
        PDVerificationFlow();
    }


    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] -", m_MagicNumber, __FUNCTION__);
    return S_3A_OK;

}

/************************************************************************/
/* Data path  : verification flow                                       */
/************************************************************************/
MVOID
PDMgr::PDVerificationFlow()
{
    char value[DBG_PROPERTY_VALUE_MAX] = {'\0'};

    MUINT32 pos = m_i4DbgPDVerifyRangeL;

    if( m_i4DbgPDVerifyRun && m_bAEStable)
    {
        // lock AE during verification
        property_set("vendor.debug.pdmgr.lockae", "1");

        // verification flow
        m_i4DbgPDVerifyCalculateCnt++;

        pos = m_i4DbgPDVerifyRangeL + (m_i4DbgPDVerifyCalculateCnt/m_i4DbgPDVerifyCalculateNum)*(m_i4DbgPDVerifyStepSize);

        if( pos<= (MUINT32)(m_i4DbgPDVerifyRangeH))
        {
#define FILE_NAME_BUFFER_SIZE 256
            char fileName[FILE_NAME_BUFFER_SIZE];
            FILE *fp = nullptr;

            /* Dump pd calculation result once VCM is stable */
            if( DBG_MOVE_LENS_STABLE_FRAME_NUM<(m_i4DbgPDVerifyCalculateCnt%m_i4DbgPDVerifyCalculateNum))
            {
                //
                snprintf( fileName, FILE_NAME_BUFFER_SIZE, "/sdcard/pdo/%s/__pd_verification_dump.0x%04x.log", m_i4DbgPdDumpTS.c_str(), m_profile.i4CurrSensorId);

                //
                fp = fopen( fileName, "a+");
                if( fp==NULL)
                {
                    char folderName[FILE_NAME_BUFFER_SIZE];
                    snprintf( folderName, FILE_NAME_BUFFER_SIZE, "/sdcard/pdo/%s", m_i4DbgPdDumpTS.c_str());

                    //
                    MINT32 err = mkdir( folderName, S_IRWXU | S_IRWXG | S_IRWXO);
                    if( 0!=err && EEXIST!=errno )
                    {
                        CAM_LOGW( "fail to create folder /sdcard/pdo/%s : %d[%s]", m_i4DbgPdDumpTS.c_str(), errno, ::strerror(errno));
                    }
                    else
                    {
                        CAM_LOGD( "create folder /sdcard/pdo/%s (%d)", m_i4DbgPdDumpTS.c_str(), err);
                    }

                    // openc file again
                    fp = fopen(fileName, "a+");
                }

                if( fp)
                {
                    if( m_vPDOutput.size())
                    {
#define MAX_DUMP_DATA_BUFFER_SIZE 1024
                        char  dumpDataBuf[MAX_DUMP_DATA_BUFFER_SIZE] = {'\0'};
                        char *ptrDumpDataBuf = dumpDataBuf;
                        unsigned int validBufSz = MAX_DUMP_DATA_BUFFER_SIZE;
                        unsigned int storedSz   = 0;

#define PD_VERIFICATION_WINDOW (9+1)
                        vector<SPDResult_T>::iterator itr = m_vPDOutput.begin();

                        if( itr->numROIs<=PD_VERIFICATION_WINDOW)
                        {
                            storedSz = snprintf( ptrDumpDataBuf,
                                                 validBufSz,
                                                 "FrmID_%d__ISO_%d__ExpTime_%" PRIu64 "__Gain_%d__NumData_%d_",
                                                 itr->magicNumber,
                                                 m_iso,
                                                 m_exposureTime,
                                                 m_afeGain,
                                                 itr->numROIs);
                            validBufSz     -= storedSz;
                            ptrDumpDataBuf += storedSz;

                            for( MUINT32 i=0; i<itr->numROIs; i++)
                            {
                                storedSz = snprintf( ptrDumpDataBuf,
                                                     validBufSz,
                                                     "WIN_%d_CurLens_%d_PD_%d_CL_%d_TARGETPOS_%d_",
                                                     i,
                                                     (MINT32)(itr->ROIRes[i].CurLensPos),
                                                     (MINT32)(itr->ROIRes[i].PhaseDifference),
                                                     itr->ROIRes[i].Confidence,
                                                     (MINT32)(itr->ROIRes[i].DesLensPos));
                                validBufSz     -= storedSz;
                                ptrDumpDataBuf += storedSz;
                            }
                            fprintf (fp, "%s\n", dumpDataBuf);
                            CAM_LOGD( "#(%d) [%s] dump info (valid buffer size %d) %s", m_MagicNumber, __FUNCTION__, validBufSz, dumpDataBuf);
                        }
                        else
                        {
                            CAM_LOGE( "#(%d) [%s] dump calculation result fail. something is wrong", m_MagicNumber, __FUNCTION__);
                        }
                    }

                    fclose( fp);
                    fp = nullptr;
                }
                else
                {
                    CAM_LOGW( "[%s] can not dump calculation result to /sdcard/pdo/%s/__pd_verification_dump.pd !!", __FUNCTION__, m_i4DbgPdDumpTS.c_str());
                }
            }


            /* Dump one pd buffer once VCM is stable */
            if(DBG_MOVE_LENS_STABLE_FRAME_NUM+2==(m_i4DbgPDVerifyCalculateCnt%m_i4DbgPDVerifyCalculateNum))
            {
                if( m_profile.BufType&MSK_CATEGORY_OPEN)
                {
                    sprintf(fileName,
                            "/sdcard/pdo/%s/0x%04x_size_0x%x_ISO_%d_exposureTime_%" PRIu64 "_curLens_%d_%d_preLens_%d_%d_R.raw",
                            m_i4DbgPdDumpTS.c_str(),
                            m_profile.i4CurrSensorId,
                            m_databuf_size,
                            m_iso,
                            m_exposureTime,
                            m_lensInfo.u4CurrentPosition,
                            m_lensInfo.u8CurrentTimestamp,
                            m_lensInfo.u4PreviousPosition,
                            m_lensInfo.u8PreviousTimestamp);

                    fp = fopen(fileName, "w");
                    if( fp)
                    {
                        CAM_LOGD( "[%s] dump data buffer : %s", __FUNCTION__, fileName);
                        fwrite(reinterpret_cast<void *>(m_databuf), 1, m_databuf_size, fp);
                        fclose(fp);
                        fp = nullptr;
                    }
                    else
                    {
                        CAM_LOGW( "[%s] dump data buffer %s !!", __FUNCTION__, fileName);
                    }


                }
                else
                {
                    if( m_pAnalysisData_Calculate)
                    {
                        //
                        MUINT32  _cvtBuf_Xsz   = 0;
                        MUINT32  _cvtBuf_Ysz   = 0;
                        MUINT32  _cvtBuf_Sz    = 0;
                        MUINT16 *_cvtBuf_Ptr   = nullptr;
                        MUINT16 *_tmp_buf      = nullptr;

                        if( (m_profile.BufType==EPDBUF_VC || m_profile.BufType==EPDBUF_PDO) && m_pPDBufMgrCore)
                        {
                            if( m_pAnalysisData_Calculate->pBufForExtractPD && m_PDBlockInfo.i4SubBlkW && m_PDBlockInfo.i4SubBlkH)
                            {
                                _cvtBuf_Xsz   = m_PDBlockInfo.i4BlockNumX * (m_PDBlockInfo.i4PitchX/m_PDBlockInfo.i4SubBlkW);
                                _cvtBuf_Ysz   = m_PDBlockInfo.i4BlockNumY * (m_PDBlockInfo.i4PitchY/m_PDBlockInfo.i4SubBlkH) * 2; //2: L and R pixels
                                _cvtBuf_Sz = _cvtBuf_Xsz*_cvtBuf_Ysz; //Pixels

                                // get porting result
                                MUINT32 PDXsz, PDYsz, PDBufSz;
                                m_pPDBufMgrCore->GetLRBufferInfo(PDXsz, PDYsz, PDBufSz);

                                if( (PDXsz==_cvtBuf_Xsz) && (PDYsz==_cvtBuf_Ysz) && (PDBufSz==_cvtBuf_Sz))
                                {
                                    _cvtBuf_Ptr = reinterpret_cast<MUINT16 *>(m_pAnalysisData_Calculate->pBufForExtractPD);
                                }
                                else
                                {
                                    CAM_LOGE( "#(%d) porting task is not correct !! size mismatch : PDXsz(%d!=%d) PDYsz(%d!=%d) PDBufSz(%d!=%d)",
                                              m_MagicNumber,
                                              PDXsz,
                                              _cvtBuf_Xsz,
                                              PDYsz,
                                              _cvtBuf_Ysz,
                                              PDBufSz,
                                              _cvtBuf_Sz);
                                }
                            }
                        }
                        else if( m_profile.BufType==EPDBUF_DUALPD_VC)
                        {
                            const unsigned int bufStride_src = m_PDVCInfo.VC_SIZEH_BYTE;
                            char *buf_src = reinterpret_cast<char *>(m_pAnalysisData_Calculate->pBufForExtractPD);

                            //convert format from DMA buffer format(Raw10) to pixel format
                            MUINT32 _h_sz   = (m_PDVCInfo.VC_DataType==0x2b ) ?
                                              (m_PDVCInfo.VC_SIZEH_PIXEL    ) :
                                              (m_PDVCInfo.VC_SIZEH_BYTE*8/10); //pixel number
                            MUINT32 _v_sz   = m_PDVCInfo.VC_SIZEV;
                            MUINT32 _bufsz = _h_sz*_v_sz; //Pixels

                            //
                            MUINT16 *_unpack_src_buf = new unsigned short [_bufsz];

                            //
                            memset( _unpack_src_buf, 0, sizeof(short)*_bufsz);

                            //
                            unsigned int i,j,k;
                            for( j=0, k=0; j<_v_sz; j++)
                            {
                                for( i=0; i<bufStride_src; i+=5)
                                {
                                    char val0 = buf_src[ j*bufStride_src + (i+0)];
                                    char val1 = buf_src[ j*bufStride_src + (i+1)];
                                    char val2 = buf_src[ j*bufStride_src + (i+2)];
                                    char val3 = buf_src[ j*bufStride_src + (i+3)];
                                    char val4 = buf_src[ j*bufStride_src + (i+4)];

                                    if(m_PDVCInfo.VC_DataType==0x2b)
                                    {
                                        _unpack_src_buf[k  ] = ((val0>>0)&0xFF) | ((val1<<8)&0x300);
                                        _unpack_src_buf[k+1] = ((val1>>2)&0x3F) | ((val2<<6)&0x3C0);
                                        _unpack_src_buf[k+2] = ((val2>>4)&0x0F) | ((val3<<4)&0x3F0);
                                        _unpack_src_buf[k+3] = ((val3>>6)&0x03) | ((val4<<2)&0x3FC);
                                    }
                                    else
                                    {
                                        _unpack_src_buf[k  ] = ((val0<<2)&0x3FC) | (((val4&0x03)>>0)&0x3);
                                        _unpack_src_buf[k+1] = ((val1<<2)&0x3FC) | (((val4&0x0C)>>2)&0x3);
                                        _unpack_src_buf[k+2] = ((val2<<2)&0x3FC) | (((val4&0x30)>>4)&0x3);
                                        _unpack_src_buf[k+3] = ((val3<<2)&0x3FC) | (((val4&0xC0)>>6)&0x3);
                                    }
                                    k+=4;
                                }
                            }




                            // Seprate source data buffer to L plane and R plane
                            _cvtBuf_Xsz = _h_sz/2;
                            _cvtBuf_Ysz = _v_sz*2;
                            _cvtBuf_Sz  = _cvtBuf_Xsz*_cvtBuf_Ysz; //Pixels

                            //
                            _tmp_buf    = new MUINT16 [_cvtBuf_Sz];

                            //
                            MUINT16 *ptrL = _tmp_buf;
                            MUINT16 *ptrR = _tmp_buf + (_cvtBuf_Sz/2);
                            MUINT16 **ptr;
                            for( j=0, k=0; j<_v_sz; j++)
                            {
                                for( i=0; i<_h_sz; i++, k++)
                                {
                                    ptr = (i%2==0) ? &ptrR : &ptrL; //interleve format : RLRLRL...
                                    (*ptr)[i/2] = _unpack_src_buf[k];
                                }
                                ptrL += _cvtBuf_Xsz;
                                ptrR += _cvtBuf_Xsz;
                            }

                            // release temp resource
                            delete []_unpack_src_buf;
                            _unpack_src_buf = nullptr;

                            //
                            _cvtBuf_Ptr = reinterpret_cast<MUINT16 *>(_tmp_buf);
                        }

                        if(_cvtBuf_Xsz && _cvtBuf_Ysz && _cvtBuf_Sz && _cvtBuf_Ptr)
                        {
                            MUINT32  _dmpBuf_Xsz   = _cvtBuf_Xsz;
                            MUINT32  _dmpBuf_Ysz   = _cvtBuf_Ysz/2;
                            MUINT32  _dmpBuf_Sz    = _dmpBuf_Xsz*_dmpBuf_Ysz;
                            void    *_dmpBuf_Ptr   = nullptr;

                            // dump L buffer
                            {
                                snprintf(fileName,
                                         FILE_NAME_BUFFER_SIZE,
                                         "/sdcard/pdo/%s/0x%04x_size_0x%lx_stride_0x%lx_ISO_%d_exposureTime_%" PRIu64 "_pixelNumX_%d_pixelNumY_%d_curLens_%d_%d_preLens_%d_%d_L.raw",
                                         m_i4DbgPdDumpTS.c_str(),
                                         m_profile.i4CurrSensorId,
                                         _dmpBuf_Sz*sizeof(MUINT16),
                                         _dmpBuf_Xsz*sizeof(MUINT16),
                                         m_iso,
                                         m_exposureTime,
                                         _dmpBuf_Xsz,
                                         _dmpBuf_Ysz,
                                         m_lensInfo.u4CurrentPosition,
                                         m_lensInfo.u8CurrentTimestamp,
                                         m_lensInfo.u4PreviousPosition,
                                         m_lensInfo.u8PreviousTimestamp);

                                fp = fopen(fileName, "w");
                                if( fp)
                                {
                                    CAM_LOGD( "[%s] dump L file : %s", __FUNCTION__, fileName);
                                    _dmpBuf_Ptr = reinterpret_cast<void *>(_cvtBuf_Ptr);
                                    fwrite(_dmpBuf_Ptr, 1, _dmpBuf_Sz*sizeof(MUINT16), fp);
                                    fclose(fp);
                                    fp = nullptr;
                                }
                                else
                                {
                                    CAM_LOGW( "[%s] can not dump L buffer to %s !!", __FUNCTION__, fileName);
                                }
                            }

                            // dump R buffer
                            {
                                sprintf(fileName,
                                        "/sdcard/pdo/%s/0x%04x_size_0x%lx_stride_0x%lx_ISO_%d_exposureTime_%" PRIu64 "_pixelNumX_%d_pixelNumY_%d_curLens_%d_%d_preLens_%d_%d_R.raw",
                                        m_i4DbgPdDumpTS.c_str(),
                                        m_profile.i4CurrSensorId,
                                        _dmpBuf_Sz*sizeof(MUINT16),
                                        _dmpBuf_Xsz*sizeof(MUINT16),
                                        m_iso,
                                        m_exposureTime,
                                        _dmpBuf_Xsz,
                                        _dmpBuf_Ysz,
                                        m_lensInfo.u4CurrentPosition,
                                        m_lensInfo.u8CurrentTimestamp,
                                        m_lensInfo.u4PreviousPosition,
                                        m_lensInfo.u8PreviousTimestamp);

                                fp = fopen(fileName, "w");
                                if( fp)
                                {
                                    CAM_LOGD( "[%s] dump R file : %s", __FUNCTION__, fileName);
                                    _dmpBuf_Ptr = reinterpret_cast<void *>(_cvtBuf_Ptr + _dmpBuf_Sz);
                                    fwrite(_dmpBuf_Ptr, 1, _dmpBuf_Sz*sizeof(MUINT16), fp);
                                    fclose(fp);
                                    fp = nullptr;
                                }
                                else
                                {
                                    CAM_LOGW( "[%s] can not dump R buffer to %s !!", __FUNCTION__, fileName);
                                }
                            }
                        }
                        else
                        {
                            CAM_LOGW( "[%s] can not dump cvtBuf : pd buffer manager type(0x%x) cvtBuf_xsz(%d) cvtBuf_ysz(%d) cvtBuf_sz(%d) cvtBuf_addr(%p)!!",
                                      __FUNCTION__,
                                      m_profile.BufType,
                                      _cvtBuf_Xsz,
                                      _cvtBuf_Ysz,
                                      _cvtBuf_Sz,
                                      _cvtBuf_Ptr);
                        }

                        //
                        if(_tmp_buf)
                        {
                            delete []_tmp_buf;
                            _tmp_buf = nullptr;
                        }
                    }
                }


            }

        }
        else
        {
            /******************************************************
             * A verification flow is finished.
             * Clear all status and waiting host trigger command.
             ******************************************************/
            property_set("vendor.debug.pdmgr.lockae", "0");
            property_set("vendor.debug.pd_verify_flow.run", "0");
            m_i4DbgPDVerifyRun = 0;
            m_i4DbgPDVerifyCalculateCnt = 0;
            pos = m_i4DbgPDVerifyRangeL;
        }
    }
    else
    {
        property_set("vendor.debug.pdmgr.lockae", "0");

        m_i4DbgPDVerifyCalculateCnt = 0;
        pos = m_i4DbgPDVerifyRangeL;

        /* wait host trigger command*/
        m_i4DbgPDVerifyRun = property_get_int32("vendor.debug.pd_verify_flow.run", 0);

        //
        m_i4DbgPdDumpTS = NSCam::Utils::LogTool::get()->getFormattedLogTime();

        //
        const std::string mark[3] = {" ", ":", "."};
        for( MUINT32 i=0; i<3; i++)
        {
            size_t nSize = mark[i].size();
            while(1)
            {
                size_t _p = m_i4DbgPdDumpTS.find(mark[i]);
                if(_p == string::npos)
                {
                    break;
                }
                m_i4DbgPdDumpTS.replace(_p, nSize, "-");
            }
        }
    }

    snprintf(value, DBG_PROPERTY_VALUE_MAX, "%d", pos);
    property_set("vendor.debug.af_motor.position", value);

    CAM_LOGD( "#(%d) [%s] DBG (verification flow) : dev(%d) En(%d) RunFlow(%d) bAEStable(%d) cnt(%d) calNum(%d) step size(%d) move lens to(%s/%d), lock ae request(%d)",
              m_MagicNumber,
              __FUNCTION__,
              m_i4CurrSensorDev,
              m_i4DbgPDVerifyEn,
              m_i4DbgPDVerifyRun,
              m_bAEStable,
              m_i4DbgPDVerifyCalculateCnt,
              m_i4DbgPDVerifyCalculateNum,
              m_i4DbgPDVerifyStepSize,
              value,
              property_get_int32("vendor.debug.af_motor.position", 0),
              property_get_int32("vendor.debug.pdmgr.lockae", 0));
}



/************************************************************************/
/* Interface to host                                                    */
/************************************************************************/
//
MRESULT
PDMgr::GetVersionOfPdafLibrary(SPDLibVersion_t &tOutSWVer)
{
    MRESULT ret=E_3A_ERR;

    tOutSWVer.MajorVersion = 0;
    tOutSWVer.MinorVersion = 0;

    if( m_bEnPDBufThd)
    {
        if( m_profile.BufType & MSK_CATEGORY_OPEN)
        {
            if( m_pPDBufMgrOpen)
            {
                ret = m_pPDBufMgrOpen->GetVersionOfPdafLibrary( tOutSWVer);
            }
        }
        else
        {
            if( m_pIPdAlgo)
            {
                PD_LIB_VERSION_T tSWVer;
                ret = m_pIPdAlgo->getVersion( tSWVer);
                tOutSWVer.MajorVersion = tSWVer.u4MajorVersion;
                tOutSWVer.MinorVersion = tSWVer.u4MinorVersion;
            }
        }

        CAM_LOGD_IF( m_bDebugEnable, "[%s] %d.%d", __FUNCTION__, (int)tOutSWVer.MajorVersion, (int)tOutSWVer.MinorVersion);
    }
    return ret;
}

//
MRESULT
PDMgr::setPDCaliData( PD_NVRAM_T *ptrInPDNvRam)
{
    //This function is used to set PD calibration data and output PD related data for Hybrid AF.
    MRESULT ret=E_3A_ERR;
    MUINT32 result;
    MINT32 i4SensorDevID;
    CAM_CAL_DATA_STRUCT CamCalData;

    switch( m_i4CurrSensorDev)
    {
        case ESensorDev_Main:
            i4SensorDevID = SENSOR_DEV_MAIN;
            break;
        case ESensorDev_Sub:
            i4SensorDevID = SENSOR_DEV_SUB;
            break;
        case ESensorDev_MainSecond:
            i4SensorDevID = SENSOR_DEV_MAIN_2;
            break;
        case ESensorDev_SubSecond:
            i4SensorDevID = SENSOR_DEV_SUB_2;
            break;
        case ESensorDev_MainThird:
            i4SensorDevID = SENSOR_DEV_MAIN_3;
            break;
        default:
            i4SensorDevID = SENSOR_DEV_NONE;
            break;
    }

#if 0
    // Read PDAF calibration data from NVRAM.
    NVRAM_AF_PARA_STRUCT *pNvramDataAF;   /* nvram for managers*/
    NvBufUtil::getInstance().getBufAndReadNoDefault( CAMERA_NVRAM_DATA_AF, m_i4CurrSensorDev, (void*&)pNvramDataAF);
    // Storing PDAF calibration data into NVRAM.
    NvBufUtil::getInstance().write( CAMERA_NVRAM_DATA_AF, m_i4CurrSensorDev);
#endif


    /**********************************************************
     * Read AF calibration data.
     * AF calibration data is used for verification flow only.
     **********************************************************/
    result = CamCalDrvBase::createInstance()->GetCamCalCalData(i4SensorDevID, CAMERA_CAM_CAL_DATA_3A_GAIN, (void *)&CamCalData);
    if( /* Verify AF calibration data */
        (result==CAM_CAL_ERR_NO_ERR) &&
        (CamCalData.Single2A.S2aAf[0]<CamCalData.Single2A.S2aAf[1]) &&
        (  CamCalData.Single2A.S2aAf[0]<=1023) &&
        (0<CamCalData.Single2A.S2aAf[0]      ) &&
        (  CamCalData.Single2A.S2aAf[1]<=1023) &&
        (0<CamCalData.Single2A.S2aAf[1]      ))
    {
        m_i4CaliAFPos_Inf    = CamCalData.Single2A.S2aAf[0];
        m_i4CaliAFPos_Macro  = CamCalData.Single2A.S2aAf[1];
        m_i4CaliAFPos_50cm   = CamCalData.Single2A.S2aAf[3];
        m_i4CaliAFPos_Middle = CamCalData.Single2A.S2aAF_t.AF_Middle_calibration;
    }
    else
    {
        m_i4CaliAFPos_Inf   = 0;
        m_i4CaliAFPos_Macro = 1023;
        CAM_LOGE("[%s] Getting CAMERA_CAM_CAL_DATA_3A_GAIN from cam_cal driver fail !!! Please contact cam_cal driver porting owner!! result(0x%x) value(%d %d %d %d)",
                 __FUNCTION__,
                 result,
                 CamCalData.Single2A.S2aAf[0],
                 CamCalData.Single2A.S2aAf[1],
                 CamCalData.Single2A.S2aAf[3],
                 CamCalData.Single2A.S2aAF_t.AF_Middle_calibration);
    }



    /************************************************************************************************
     * Read PD calibration data
     * If calibration data hasn't read out from EEPROM or something wrong of calibration data buffer.
     * Reading calibration data from EEPROM by using cam_cal driver.
     * Please notice that m_sCaliData is only initialized druing construct pd_mgr.
     ************************************************************************************************/
#define _PDCALIDA_READ_ERROR_ 0
#define _PDCALIDA_READ_FROM_DEFAULT_ 1
#define _PDCALIDA_READ_FROM_EEPROM_  2
#define _DEF_CALI_DATA_PATH_ "/vendor/etc/default_pd_calibration.bin"

    // data valid of default calibration data which is stored in file system (/sdcard/pdo/pd_cali_data.bin)
    MINT32 cali_da_vd_F = property_get_int32("vendor.debug.pdcalidata.enable", 0)!=0;
    // data valid of default calibration data which is stored in /vendor/etc/default_pd_calibration.bin
    MBOOL  cali_da_vd_D = MFALSE;
    {
        FILE *fp = NULL;
        fp = fopen(_DEF_CALI_DATA_PATH_, "rb");
        if(fp)
        {
            //
            fseek( fp, 0, SEEK_END );
            int file_size = ftell( fp );
            rewind( fp );

            //
            MBOOL header_valid = MTRUE;

            //
#define _HEADER_SIZE_IN_BYTE_ 16
            if( file_size>_HEADER_SIZE_IN_BYTE_)
            {
                char header[_HEADER_SIZE_IN_BYTE_] = { 'D', 'E', 'F', '_', 'C', 'A', 'L', 'I', '_', '_', 'D', 'A', '_', 'V', '0', '1' };
                for(int i=0; i<_HEADER_SIZE_IN_BYTE_; i++)
                {
                    char val=0;
                    fread( reinterpret_cast<void *>(&val), sizeof(char), 1, fp);
                    if( val!=header[i])
                    {
                        header_valid = MFALSE;
                        break;
                    }
                }
            }
            else
            {
                header_valid = MFALSE;
            }

            if(header_valid)
            {
                unsigned int numData = 0;
                fread( reinterpret_cast<void *>(&numData), sizeof(int), 1, fp);

                if( (0<numData) && (numData<32))
                {
#define _METADATA_SIZE_ 6
                    unsigned int *data_info = new unsigned int [numData*_METADATA_SIZE_];
                    fread( reinterpret_cast<void *>(data_info), sizeof(unsigned int), numData*_METADATA_SIZE_, fp);

                    // match meta
                    for(int i=0; i<numData; i++)
                    {
                        unsigned int dev            = data_info[i*_METADATA_SIZE_ + 0];
                        unsigned int sensor_id      = data_info[i*_METADATA_SIZE_ + 1];
                        unsigned int lens_id        = data_info[i*_METADATA_SIZE_ + 2];
                        unsigned int module_id      = data_info[i*_METADATA_SIZE_ + 3];
                        unsigned int cali_da_sz     = data_info[i*_METADATA_SIZE_ + 4];
                        unsigned int cali_da_offset = data_info[i*_METADATA_SIZE_ + 5];

                        CAM_LOGD( "[%s] Meta data of default calibration data : dev(%d) sensor_id(%x) lens_id(%x) module_id(%x) cali_data_sz(%x) cali_data_offset(%x)",
                                  __FUNCTION__,
                                  dev,
                                  sensor_id,
                                  lens_id,
                                  module_id,
                                  cali_da_sz,
                                  cali_da_offset);

                        if( /* */
                            (dev      ==(unsigned int)(m_i4CurrSensorDev       )) &&
                            (sensor_id==(unsigned int)(m_profile.i4CurrSensorId)) &&
                            ((cali_da_offset+1+cali_da_sz)<file_size))
                        {
                            fseek( fp, cali_da_offset, SEEK_SET);
                            m_sCaliData.i4Size = cali_da_sz;
                            fread( reinterpret_cast<void *>(m_sCaliData.uData), sizeof(char), m_sCaliData.i4Size, fp);
                            cali_da_vd_D = MTRUE;
                            break;
                        }

                    }

                    //
                    if(data_info)
                    {
                        delete []data_info;
                    }
                }
                else
                {
                    CAM_LOGD( "[%s] number of calibration data in %s is wrong : %d", __FUNCTION__, _DEF_CALI_DATA_PATH_, numData);
                }
            }
            else
            {
                CAM_LOGD( "[%s] Header of default PD calibration data (%s) is not valid", __FUNCTION__, _DEF_CALI_DATA_PATH_);
            }

            //
            fclose( fp);
        }
        else
        {
            CAM_LOGD( "[%s] Default PD calibration data (%s) is not exist ", __FUNCTION__, _DEF_CALI_DATA_PATH_);
        }
    }


    // checking using default calibration data.
    MBOOL enable_cali_data_from_default = cali_da_vd_F || cali_da_vd_D;

    CAM_LOGD( "[%s] Load PD default calibration data(%d) : from file system(%d), from tuning file(%d)",
              __FUNCTION__,
              enable_cali_data_from_default,
              cali_da_vd_F,
              cali_da_vd_D);

    if( enable_cali_data_from_default==0)
    {
        if( (m_sCaliData.i4Size<=0) || (PD_CALI_DATA_SIZE<m_sCaliData.i4Size) || (m_u1CaliDaSrc!=_PDCALIDA_READ_FROM_EEPROM_))
        {
            result = CamCalDrvBase::createInstance()->GetCamCalCalData(i4SensorDevID, CAMERA_CAM_CAL_DATA_PDAF, (void *)&CamCalData);
            if( result==CAM_CAL_ERR_NO_ERR)
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_FROM_EEPROM_;
                m_sCaliData.i4Size = CamCalData.PDAF.Size_of_PDAF;
                memcpy( m_sCaliData.uData, CamCalData.PDAF.Data, sizeof(MUINT8)*PD_CALI_DATA_SIZE);
                CAM_LOGD( "[%s] Dev(%d):PDAF calibration data hasn't read out. Reading calibration data from cam_cal. (%d BYTE)",
                          __FUNCTION__,
                          i4SensorDevID,
                          m_sCaliData.i4Size);
            }
            else
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_ERROR_;
                m_sCaliData.i4Size = 0;
                CAM_LOGE("[%s] Dev(%d):Getting CAMERA_CAM_CAL_DATA_PDAF from cam_cal driver fail (result:%8x) !!! Please contact cam_cal driver porting owner!!",
                         __FUNCTION__,
                         i4SensorDevID,
                         result);
            }
        }
        else
        {
            CAM_LOGD( "[%s] PD calibration data is already read out normally from EEPROM (%d). Using it directly.", __FUNCTION__, m_u1CaliDaSrc);
        }
    }
    else
    {
        if( (m_sCaliData.i4Size<=0) || (PD_CALI_DATA_SIZE<m_sCaliData.i4Size) || (m_u1CaliDaSrc!=_PDCALIDA_READ_FROM_DEFAULT_))
        {
            FILE *fp = NULL;
            fp = fopen("/sdcard/pdo/pd_cali_data.bin", "r");

            if( fp)
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_FROM_DEFAULT_;
                m_sCaliData.i4Size = PD_CALI_DATA_SIZE;
                fread( reinterpret_cast<void *>(m_sCaliData.uData), 1, PD_CALI_DATA_SIZE, fp);
                fclose( fp);
                CAM_LOGW( "[%s] Read default PD calibration data from /sdcard/pdo/pd_cali_data.bin!! This is a debug flow!!", __FUNCTION__);
            }
            else if( m_sCaliData.i4Size)
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_FROM_DEFAULT_;
                //m_sCaliData.i4Size = PD_CALI_DATA_SIZE;
                //memcpy( m_sCaliData.uData, ptrInPDNvRam->rCaliData.uData, sizeof(MUINT8)*PD_CALI_DATA_SIZE);
                CAM_LOGW( "[%s] Read default PD calibration data from %s!! Please double check this setting is correct!! (%x,%x,%x,%x)", __FUNCTION__, _DEF_CALI_DATA_PATH_, m_sCaliData.uData[0], m_sCaliData.uData[1], m_sCaliData.uData[2], m_sCaliData.uData[3]);
            }
            else
            {
                m_u1CaliDaSrc      = _PDCALIDA_READ_ERROR_;
                m_sCaliData.i4Size = 0;
                CAM_LOGE("[%s] Read default PD calibration data FAIL. Please check setting of the tuning file or file /sdcard/pdo/pd_cali_data.bin", __FUNCTION__);
            }
        }
        else
        {
            CAM_LOGD( "[%s] Default PD calibration data is already read out normally (%d). Using it directly.", __FUNCTION__, m_u1CaliDaSrc);
        }
    }


    CAM_LOGD( "[%s] AF calibration information : Inf(%d) Macro(%d) 50cm(%d) Middle(%d) ,PDAF calibration information (%d): size(%d BYTE), first 4 dword(0x%08x 0x%08x 0x%08x 0x%08x)",
              __FUNCTION__,
              m_i4CaliAFPos_Inf,
              m_i4CaliAFPos_Macro,
              m_i4CaliAFPos_50cm,
              m_i4CaliAFPos_Middle,
              m_u1CaliDaSrc,
              m_sCaliData.i4Size,
              *(((MUINT32 *)m_sCaliData.uData)+0),
              *(((MUINT32 *)m_sCaliData.uData)+1),
              *(((MUINT32 *)m_sCaliData.uData)+2),
              *(((MUINT32 *)m_sCaliData.uData)+3));
    //
    if( m_bDebugEnable)
    {
        CAM_LOGD("dump calibration data, size(%d):", m_sCaliData.i4Size);
        for( int index = 0; index<m_sCaliData.i4Size; index+=32)
        {
            CAM_LOGD( "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                      m_sCaliData.uData[index+ 0], m_sCaliData.uData[index+ 1],
                      m_sCaliData.uData[index+ 2], m_sCaliData.uData[index+ 3],
                      m_sCaliData.uData[index+ 4], m_sCaliData.uData[index+ 5],
                      m_sCaliData.uData[index+ 6], m_sCaliData.uData[index+ 7],
                      m_sCaliData.uData[index+ 8], m_sCaliData.uData[index+ 9],
                      m_sCaliData.uData[index+10], m_sCaliData.uData[index+11],
                      m_sCaliData.uData[index+12], m_sCaliData.uData[index+13],
                      m_sCaliData.uData[index+14], m_sCaliData.uData[index+15],
                      m_sCaliData.uData[index+16], m_sCaliData.uData[index+17],
                      m_sCaliData.uData[index+18], m_sCaliData.uData[index+19],
                      m_sCaliData.uData[index+20], m_sCaliData.uData[index+21],
                      m_sCaliData.uData[index+22], m_sCaliData.uData[index+23],
                      m_sCaliData.uData[index+24], m_sCaliData.uData[index+25],
                      m_sCaliData.uData[index+26], m_sCaliData.uData[index+27],
                      m_sCaliData.uData[index+28], m_sCaliData.uData[index+29],
                      m_sCaliData.uData[index+30], m_sCaliData.uData[index+31]);
        }
    }

    //==========
    // (3) Set PDAF calibration data.
    //==========
    if( /* After related instance and PD thread is created, load calibration data*/
        ((m_bEnPDBufThd                                        )) &&
        ((m_sCaliData.i4Size) != (0                            )) &&
        ((m_profile.BufType ) != (EPDBUF_NOTDEF                )) &&
        ((m_pPDBufMgrOpen) || (m_pPDBufMgrCore && m_pIPdAlgo)))
    {

        if( m_profile.BufType & MSK_CATEGORY_OPEN)
            ret = SetCaliData2PDOpenCore( &(ptrInPDNvRam->rTuningData), &m_sCaliData);
        else
            ret = SetCaliData2PDCore( &(ptrInPDNvRam->rTuningData), &m_sCaliData);
    }
    else
    {
        ret = E_3A_ERR;
        CAM_LOGD( "[%s] No setting PD calibration and tuning data: type(%d), enThd(%d), instance(%p %p %p)",
                  __FUNCTION__,
                  m_profile.BufType,
                  m_bEnPDBufThd,
                  (void*)m_pPDBufMgrCore,
                  (void*)m_pIPdAlgo,
                  (void*)m_pPDBufMgrOpen);
    }

    return ret;
}

//
MRESULT
PDMgr::UpdatePDParam( MUINT32 u4FrmNum,
                      MINT32 i4InputPDAreaNum,
                      AFPD_BLOCK_ROI_T *tInputPDArea,
                      MINT32 i4MinLensPos,
                      MINT32 i4MaxLensPos,
                      MBOOL  bForceCalculation)
{
    MRESULT ret = E_3A_ERR;

    if( m_profile.BufType!=EPDBUF_NOTDEF)
    {
        if( m_profile.BufType & MSK_CATEGORY_OPEN)
        {
            if( 0<i4InputPDAreaNum && i4InputPDAreaNum<=MAX_QUEUE_SIZE_OF_CONFIGURATION_FOR_3RD_PARTY_LIB && tInputPDArea!=NULL)
            {
                S_ANALYSIS_DATA_OPEN_T *cfg  = new S_ANALYSIS_DATA_OPEN_T;

                cfg->frmNum  = u4FrmNum; /* current magic number of statistic data */
                cfg->numROIs = i4InputPDAreaNum;
                for( MUINT32 i=0; i<cfg->numROIs; i++)
                {
                    cfg->ROI[i].sRoiInfo = tInputPDArea[i];
                }
                cfg->nxt  = NULL;
                cfg->head = NULL;
                cfg->tail = NULL;

                {
                    Mutex::Autolock lock( m_Lock_AnalysisData);
                    S_ANALYSIS_DATA_OPEN_T *tail = m_sAnalysisDataOpen.tail;

                    if( tail)
                    {
                        // add element at the end of list
                        cfg->prv  = tail;
                        tail->nxt = cfg;
                    }
                    else
                    {
                        // add first element to list
                        cfg->prv  = NULL;
                        m_sAnalysisDataOpen.head = cfg;
                        m_sAnalysisDataOpen.nxt  = cfg;
                    }
                    m_sAnalysisDataOpen.tail = cfg;
                    m_sAnalysisDataOpen.totalsz++;

                    // Limit list size.
                    if( MAX_QUEUE_SIZE_OF_CONFIGURATION_FOR_3RD_PARTY_LIB<m_sAnalysisDataOpen.totalsz)
                    {
                        S_ANALYSIS_DATA_OPEN_T *del_element = m_sAnalysisDataOpen.head;

                        m_sAnalysisDataOpen.head = del_element->nxt;
                        m_sAnalysisDataOpen.nxt  = del_element->nxt;
                        m_sAnalysisDataOpen.head->prv = NULL;
                        m_sAnalysisDataOpen.totalsz--;

                        delete del_element;
                    }


                    //
                    ConfigurePDHWSetting( &m_profile, NULL);
                }
            }
        }
        else
        {
            Mutex::Autolock lock( m_Lock_AnalysisData);

            if(m_pAnalysisData_Calculate && m_pAnalysisData_Update)
            {

                //
                MUINT32 max_num_roi = MAX_SIZE_OF_PD_ROI<AF_PSUBWIN_NUM ? MAX_SIZE_OF_PD_ROI : AF_PSUBWIN_NUM;

                //
                if( (MUINT32)(i4InputPDAreaNum)<max_num_roi)
                {
                    m_pAnalysisData_Update->numDa = i4InputPDAreaNum;
                }
                else
                {
                    m_pAnalysisData_Update->numDa = max_num_roi;
                    CAM_LOGE("[%s] Too many of ROIs(%d). Only %d ROIs can be calculated", __FUNCTION__, i4InputPDAreaNum, max_num_roi);
                }

                //
                for( MUINT32 i=0; i<m_pAnalysisData_Update->numDa; i++)
                {
                    m_pAnalysisData_Update->Da[i].sRoiInfo = tInputPDArea[i];
                    m_pAnalysisData_Update->Da[i].sHandlePDInput.i4MinLensPos = i4MinLensPos;
                    m_pAnalysisData_Update->Da[i].sHandlePDInput.i4MaxLensPos = i4MaxLensPos;
                }

                //
                if(m_bConfigCompleted==MFALSE)
                {
                    /* The first time configure PD manager. Update calculaltion buffer, too. */
                    memcpy( m_pAnalysisData_Calculate, m_pAnalysisData_Update, sizeof(S_ANALYSIS_DATA_T));
                }


                if( m_bDebugEnable)
                {
                    //
                    char  dbgMsgBuf_roi[DBG_MSG_BUF_SZ];
                    char* ptrMsgBuf_roi = dbgMsgBuf_roi;
                    ptrMsgBuf_roi += sprintf( ptrMsgBuf_roi,
                                              "[%s]: update analyssis data buffer %p, MinLensPos(%d), MaxLensPos(%d), %d ROIs",
                                              __FUNCTION__,
                                              m_pAnalysisData_Update,
                                              i4MinLensPos,
                                              i4MaxLensPos,
                                              m_pAnalysisData_Update->numDa);
                    //
                    for( MUINT32 i=0; i<m_pAnalysisData_Update->numDa; i++)
                    {
                        ptrMsgBuf_roi += sprintf( ptrMsgBuf_roi,
                                                  "(%d/%d/%d/%d/%d/%d),",
                                                  m_pAnalysisData_Update->Da[i].sRoiInfo.sType,
                                                  m_pAnalysisData_Update->Da[i].sRoiInfo.sPDROI.i4X,
                                                  m_pAnalysisData_Update->Da[i].sRoiInfo.sPDROI.i4Y,
                                                  m_pAnalysisData_Update->Da[i].sRoiInfo.sPDROI.i4W,
                                                  m_pAnalysisData_Update->Da[i].sRoiInfo.sPDROI.i4H,
                                                  m_pAnalysisData_Update->Da[i].sRoiInfo.sPDROI.i4Info);
                    }
                    CAM_LOGD_IF( m_bDebugEnable, "%s", dbgMsgBuf_roi);

                }
            }
            else
            {
                CAM_LOGE("[%s] Should not be happened !! analysis data buffer is not vaild. Please check config flow", __FUNCTION__);
            }

        }
    }


    // update common parameters which is depended on hybrid af output
    m_minLensPos = i4MinLensPos;
    m_maxLensPos = i4MaxLensPos;

    // for flow control.
    m_bConfigCompleted  = MTRUE;
    m_bForceCalculation = bForceCalculation;
    return ret;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
PDMgr::doSWPDE(MVOID *iHalMetaData,
               MVOID *iImgbuf)
{
    CAM_LOGD_IF( m_bDebugEnable, "%s +", __FUNCTION__);

    PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "doSWPDE");
    if( /* SWPDE path*/
        (iHalMetaData) &&
        (iImgbuf) &&
        (m_profile.u4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY))
    {

        IMetadata *metaDa = reinterpret_cast<IMetadata *>(iHalMetaData);

        //
        MUINT32 bufFmt = -1;
        {
            // Reference : IHalCamIO.h, Pipe_PROCESSED_RAW=0x0000, EPipe_PURE_RAW=0x0001
            IMetadata::IEntry const& entry = metaDa->entryFor(MTK_P1NODE_RAW_TYPE);
            if(!entry.isEmpty())
            {
                bufFmt = entry.itemAt(0, Type2Type<MINT32>());
            }
            else
            {
                bufFmt = -1;
            }
        }

        //
        if( bufFmt==0x0001)
        {

            //
            MUINT32 bufMagicNum = -1;
            {
                IMetadata::IEntry const& entry = metaDa->entryFor(MTK_P1NODE_PROCESSOR_MAGICNUM);
                if(!entry.isEmpty())
                {
                    bufMagicNum = entry.itemAt(0, Type2Type<MINT32>());
                }
                else
                {
                    bufMagicNum = -1;
                }
            }

            //
            IImageBuffer *imgBuf = reinterpret_cast<IImageBuffer *>(iImgbuf);
            IImageBufferHeap *imgBufHeap = imgBuf->getImageBufferHeap();

            if(m_bDebugEnable)
            {
                CAM_LOGD_IF( m_bDebugEnable,
                             "%s raw info for pd processing : frmNum(%d) w(%d) h(%d) stride(0x%lx BYTE) heapID(%d) va(%p) format(%d), sz(0x%lx BYTE), bit depth per pixel(%lu), plane (%lu)",
                             __FUNCTION__,
                             bufMagicNum,
                             imgBuf->getImgSize().w,
                             imgBuf->getImgSize().h,
                             imgBuf->getBufStridesInBytes(0),
                             imgBufHeap->getHeapID(),
                             (void *)(imgBuf->getBufVA(0)),
                             bufFmt,
                             imgBuf->getBufSizeInBytes(0),
                             imgBuf->getImgBitsPerPixel(),
                             imgBuf->getPlaneCount());

            }


            //
            PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "processing %d", bufMagicNum);
            CAM_LOGD_IF( m_bDebugEnable, "%s wait semSWPDE +", __FUNCTION__);
            while(1)
            {
                ::sem_wait( &m_semSWPDE);
                {
                    Mutex::Autolock lock( m_Lock_RawBufCtrl);

                    vector<RAW_BUF_CTRL_T>::iterator itr;

                    for(itr=m_vRawBufCtrl.begin(); itr!=m_vRawBufCtrl.end(); itr++)
                    {
                        if( /**/
                            (itr->mMagicNumber == bufMagicNum) &&
                            (itr->mImgHeapID   == (MUINT32)(imgBufHeap->getHeapID())))
                        {
                            CAM_LOGD_IF( m_bDebugEnable,
                                         "%s pure raw buffer(#%d) is processed : heapID(%d) format(%d/%d)",
                                         __FUNCTION__,
                                         bufMagicNum,
                                         imgBufHeap->getHeapID(),
                                         bufFmt,
                                         itr->mImgoFmt);
                            break;
                        }
                    }

                    if(itr!=m_vRawBufCtrl.end())
                    {
                        m_vRawBufCtrl.erase(m_vRawBufCtrl.begin(), itr++);
                        break;
                    }
                    else
                    {
                        CAM_LOGD_IF( m_bDebugEnable,
                                     "%s pure raw buffer(#%d) is processing!! : heapID(%d) format(%d)",
                                     __FUNCTION__,
                                     bufMagicNum,
                                     imgBufHeap->getHeapID(),
                                     bufFmt);
                    }
                }
            }
            CAM_LOGD_IF( m_bDebugEnable, "%s wait semSWPDE -", __FUNCTION__);
            PD_TRACE_FMT_END(TRACE_LEVEL_DEF);
        }

    }
    else
    {
        CAM_LOGE( "%s : input data for SW PDE error !! meta(%p), buf(%p), pd sensor type(%d)",
                  __FUNCTION__,
                  iHalMetaData,
                  iImgbuf,
                  m_profile.u4PDAF_support_type);
    }

    CAM_LOGD_IF( m_bDebugEnable, "%s -", __FUNCTION__);
    PD_TRACE_FMT_END(TRACE_LEVEL_DEF);

    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
PDMgr::postToPDTask( StatisticBufInfo *pSttData, mcuMotorInfo *pLensInfo)
{
    PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DBG, "postToPDTask");

    MRESULT ret = E_3A_ERR;

    if(m_bEnPDBufThd)
    {
        //Run thread if PD task is not busy.
        int Val;
        ::sem_getvalue( &m_semPDBufThdEnd, &Val);

        if( m_bDebugEnable)
        {
            if( Val==1)
            {
                CAM_LOGD_IF( m_bDebugEnable,
                             "#(%d) [%s] Value of semaphore(m_semPDBufThdEnd) is [1] : Normal case:PDBufThd is waiting signal for calculation.",
                             m_MagicNumber,
                             __FUNCTION__);
            }
            else
            {
                CAM_LOGD_IF( m_bDebugEnable,
                             "#(%d) [%s] Value of semaphore(m_semPDBufThdEnd) is [%d] : Abnormal case:Calculation is still executed in PDBufThd",
                             m_MagicNumber,
                             __FUNCTION__,
                             Val);
            }
        }

        /***************************************************************
         * (1) checking PD thread is busy or not.
         ***************************************************************/
        if( Val==1)
        {
            Mutex::Autolock lock(m_Lock);

            /***************************************************************
             * (2) prepare data buffer and post to calculation thread
             ***************************************************************/
            if(pSttData && m_pLensNVRam)
            {
                {
                    //
                    Mutex::Autolock lock(m_Lock_NvRamIdx);

                    NVRAM_LENS_DATA_PARA_STRUCT *pNVRam = &(m_pLensNVRam->AF[m_i4NVRamIdx]);
                    //
                    if(pNVRam)
                    {
                        //
                        if(pNVRam!=m_pNVRam)
                        {
                            /*************************************************************************************************
                             *
                             * m_pLensNVRam is initialized during configuring pd mgr.
                             * If NVRAM data is updated, configuration of algorithm should be updated, too.
                             *
                             *************************************************************************************************/
                            CAM_LOGD( "#(%d) [%s] Updata NVRAM tuning data as index %d. (%s -> %s)",
                                      m_MagicNumber,
                                      __FUNCTION__,
                                      m_i4NVRamIdx,
                                      m_pNVRam->rFilePath,
                                      pNVRam->rFilePath);

                            // updata NVRAM data
                            m_pNVRam = pNVRam;

                            //
                            if( m_pIPdAlgo && m_bEnPDBufThd)
                            {
                                ConfigurePDHWSetting( &m_profile, &(m_pNVRam->rPDNVRAM.rTuningData));
                                setPDCaliData( &(m_pNVRam->rPDNVRAM));
                            }
                        }
                    }
                    else
                    {
                        CAM_LOGE( "#(%d) [%s] Can not update NVRam tuning data to algorithm. NVRAM data is NULL at index(%d). Please check IdxMgr.", m_MagicNumber, __FUNCTION__, m_i4NVRamIdx);
                    }
                }


                /***************************************************************
                 * (3) update magic number for current statistic data.
                 ***************************************************************/
                switch(m_profile.u4PDAF_support_type)
                {
                    case PDAF_SUPPORT_RAW_LEGACY:
                    case PDAF_SUPPORT_RAW:
                    case PDAF_SUPPORT_RAW_DUALPD:
                    {
                        m_MagicNumber = pSttData->mMagicNumber; /* PDO : magic number is from the HW buffer information*/
                    }
                    break;

                    case PDAF_SUPPORT_CAMSV:
                    case PDAF_SUPPORT_CAMSV_LEGACY:
                    case PDAF_SUPPORT_CAMSV_DUALPD:
                    {
                        m_MagicNumber = m_requestNum - 2; /* CamSv : magic number is calculated by the request number*/
                    }
                    break;

                    default:
                    {
                        m_MagicNumber = 0;
                        CAM_LOGE( "#(%d) Should not be happened : PD calculation thread is enabled but profile information is not correct", m_MagicNumber);
                    }
                    break;
                }


                if(m_bConfigCompleted && m_pAnalysisData_Calculate)
                {
                    /***************************************************************
                     * (4) check need pd result or not
                     ***************************************************************/
#define PERFRAME_CALCULATE_PD 1
                    //
                    MBOOL fdexist = MFALSE;
                    for( MUINT32 i=0; i<(MUINT32)m_pAnalysisData_Calculate->numDa; i++)
                    {
                        if((m_pAnalysisData_Calculate->Da[i].sRoiInfo.sType==eAF_ROI_TYPE_OT) || (m_pAnalysisData_Calculate->Da[i].sRoiInfo.sType==eAF_ROI_TYPE_FD))
                        {
                            fdexist = MTRUE;
                            break;
                        }
                    }

                    //
                    switch(m_profile.u4PDAF_support_type)
                    {
                        case PDAF_SUPPORT_RAW_LEGACY:
                        {
                            if( /* IMGO pure raw buffer is vaild*/
                                (pSttData->mImgoFmt==1) &&
                                (pSttData->mImgoSizeH!=1) &&
                                (pSttData->mImgoSizeW!=0))
                            {
                                m_bNeedPDResult = MTRUE;
                            }
                            else
                            {
                                m_bNeedPDResult = MFALSE;
                            }

                        }
                        break;

                        case PDAF_SUPPORT_RAW:
                        case PDAF_SUPPORT_RAW_DUALPD:
                        case PDAF_SUPPORT_CAMSV:
                        case PDAF_SUPPORT_CAMSV_LEGACY:
                        case PDAF_SUPPORT_CAMSV_DUALPD:
                        {
                            if( /* checkin pd calculation task can be executed or not */
                                (m_bForceCalculation) /* request from host */||
                                (m_pNVRam->rAFNVRAM.i4EasyTuning[2]==PERFRAME_CALCULATE_PD) /*perframe calculating pd*/ ||
                                (fdexist==MTRUE) /* face ROI is exist in the calculation list*/)
                            {
                                m_bNeedPDResult = MTRUE;

                            }
                            else
                            {
                                m_bNeedPDResult = 1-m_bNeedPDResult; /* Normally, do pd sw path per 2 frame */
                            }
                        }
                        break;

                        default:
                        {
                            m_bNeedPDResult = MFALSE;
                            CAM_LOGE( "#(%d) Should not be happened : PD calculation thread is enabled but profile information is not correct", m_MagicNumber);
                        }
                        break;
                    }

                    //
                    if( m_bNeedPDResult)
                    {
                        // Lens information
                        m_lensInfo.u4CurrentPosition = pSttData->mLensPosition;
                        if(pLensInfo)
                        {
                            if( /**/
                                (0 < pLensInfo->u4PreviousPosition        ) &&
                                (    pLensInfo->u4PreviousPosition <= 1023) &&
                                (0 < pLensInfo->u4CurrentPosition         ) &&
                                (    pLensInfo->u4CurrentPosition  <= 1023) &&
                                (0 < pLensInfo->u8CurrentTimestamp ))
                            {
                                memcpy( &m_lensInfo, pLensInfo, sizeof(mcuMotorInfo));
                            }
                            else
                            {
                                CAM_LOGW( "#(%d) [%s] wrong lens information for lens adaptive compensation. pre:(%d,%d) cur:(%d,%d)",
                                          m_MagicNumber,
                                          __FUNCTION__,
                                          pLensInfo->u4PreviousPosition,
                                          pLensInfo->u8PreviousTimestamp,
                                          pLensInfo->u4CurrentPosition,
                                          pLensInfo->u8CurrentTimestamp);
                            }
                        }
                        else
                        {
                            CAM_LOGW( "#(%d) [%s] No enough lens information for lens adaptive compensation",
                                      m_MagicNumber,
                                      __FUNCTION__);
                        }

                        /***************************************************************
                         * (5) To get qualified PD result, lens position should be greater than 0
                         ***************************************************************/
                        if( /* */
                            (0 < m_lensInfo.u4CurrentPosition        ) &&
                            (    m_lensInfo.u4CurrentPosition <= 1023))
                        {

                            /***************************************************************
                             * (6) get information which is corresponding to current buffer.
                             ***************************************************************/
                            IResultPool* pResultPoolObj = IResultPool::getInstance(m_i4CurrSensorDev);
                            AllResult_T *pAllResult = pResultPoolObj->getResultByReqNum(m_MagicNumber, __FUNCTION__);
                            MUINT32 isFlashOn = MFALSE;
                            if(pAllResult)
                            {
                                //
                                FLASHResultToMeta_T* pFLASHResult = (FLASHResultToMeta_T*)(pAllResult->ModuleResult[E_FLASH_RESULTTOMETA]->read());
                                if(pFLASHResult)
                                {
                                    isFlashOn = ((pFLASHResult->u1FlashState == MTK_FLASH_STATE_FIRED) || (pFLASHResult->u1FlashState == MTK_FLASH_STATE_PARTIAL)) ? MTRUE : MFALSE;
                                }

                                //
                                AEResultInfo_T  *pAEResultInfo = (AEResultInfo_T*)(pAllResult->ModuleResult[E_AE_RESULTINFO]->read());
                                if(pAEResultInfo)
                                {
                                    m_afeGain      = pAEResultInfo->AEPerframeInfo.rAEISPInfo.u4P1SensorGain;
                                    m_iso          = pAEResultInfo->AEPerframeInfo.rAEISPInfo.u4P1RealISOValue;
                                    m_bAEStable    = pAEResultInfo->AEPerframeInfo.rAEISPInfo.bAEStable;
                                    m_exposureTime = pAEResultInfo->AEPerframeInfo.rAEISPInfo.u8P1Exposuretime_ns;
                                }
                                else
                                {
                                    m_afeGain   = 0;
                                    m_iso       = 0;
                                    m_bAEStable = 0;
                                    CAM_LOGW( "#(%d) [%s] can not get AE status from result pool.",
                                              m_MagicNumber,
                                              __FUNCTION__);
                                }
                            }
                            pResultPoolObj->returnResult(pAllResult, __FUNCTION__);

                            //
                            m_time_stamp_sof = pSttData->mTimeStamp;

                            //
                            CAM_LOGD_IF( m_bDebugEnable,
                                         "#(%d) [%s] Input buffer : va:%" PRIxPTR ", sz:0x%x, stride:0x%x, TS:%" PRId64 ", exposure time %" PRIu64 " ns, AGain:%d, iso:%d, flashOn:%d, lens info[ pre:(DAC:%d, TS:%d) cur:(DAC:%d, TS:%d)], minlens:%d, maxlens:%d)",
                                         m_MagicNumber,
                                         __FUNCTION__,
                                         pSttData->mVa,
                                         pSttData->mSize,
                                         pSttData->mStride,
                                         m_time_stamp_sof,
                                         m_exposureTime,
                                         m_afeGain,
                                         m_iso,
                                         isFlashOn,
                                         m_lensInfo.u4PreviousPosition,
                                         m_lensInfo.u8PreviousTimestamp,
                                         m_lensInfo.u4CurrentPosition,
                                         m_lensInfo.u8CurrentTimestamp,
                                         m_minLensPos,
                                         m_maxLensPos);

                            if( m_i4DbgPdDump)
                            {
                                static MUINT32 dump_raw_data_cnt = 0;

                                if( /**/
                                    ((m_profile.BufType & MSK_PDBUF_TYPE_RAW_LEGACY)==0) ||
                                    ((m_profile.BufType & MSK_PDBUF_TYPE_RAW_LEGACY) && (dump_raw_data_cnt%10==0)) /*dump source buffer (raw data) for each 10 frames*/)
                                {
                                    dump_raw_data_cnt = 0;

                                    char fileName[256];
                                    FILE *fp = nullptr;

                                    //
                                    sprintf(fileName,
                                            "/sdcard/pdo/%" PRId64 "_%05d_srcBuf_type_0x%x_size_0x%x_stride_0x%x_sofTS_%" PRId64 "_expTime_%" PRIu64 "_curLens_%d_%d_preLens_%d_%d_pixelclk_%d_linelength_%d.raw",
                                            _GET_TIME_STAMP_US_(),
                                            m_MagicNumber,
                                            m_profile.BufType,
                                            pSttData->mSize,
                                            pSttData->mStride,
                                            m_time_stamp_sof,
                                            m_exposureTime,
                                            m_lensInfo.u4CurrentPosition,
                                            m_lensInfo.u8CurrentTimestamp,
                                            m_lensInfo.u4PreviousPosition,
                                            m_lensInfo.u8PreviousTimestamp,
                                            m_Sensor_pixel_clk,
                                            m_Sensor_line_length);

                                    fp = fopen(fileName, "w");
                                    if( fp)
                                    {
                                        CAM_LOGD( "dump file : %s", fileName);
                                        fwrite(reinterpret_cast<void *>(pSttData->mVa), 1, pSttData->mSize, fp);
                                        fclose(fp);
                                    }
                                    else
                                    {
                                        CAM_LOGW( "[%s] can not dump srcBuf to %s !!", __FUNCTION__, fileName);
                                    }
                                    if( m_profile.BufType & MSK_PDBUF_TYPE_VC)
                                    {
                                        if( pSttData->mSize<((m_PDVCInfo.VC_SIZEH_BYTE)*(m_PDVCInfo.VC_SIZEV)))
                                        {
                                            CAM_LOGE( "#(%d) CamSV buffer size is mismatch : %d < %d",
                                                      m_MagicNumber,
                                                      pSttData->mSize,
                                                      ((m_PDVCInfo.VC_SIZEH_BYTE)*(m_PDVCInfo.VC_SIZEV)));
                                        }
                                    }
                                }

                                dump_raw_data_cnt++;
                            }

                            /***************************************************************
                             * (7) prepare data buffer and post to calculation thread
                             ***************************************************************/
                            if( m_profile.BufType & MSK_CATEGORY_OPEN)
                            {
                                if( /* buffer information check*/
                                    (m_databuf_size  !=pSttData->mSize  ) ||
                                    (m_databuf_stride!=pSttData->mStride))
                                {
                                    // Release buffer if need
                                    if( m_databuf)
                                        delete []m_databuf;
                                    m_databuf = nullptr;

                                    //
                                    m_databuf_size   = pSttData->mSize;
                                    m_databuf_stride = pSttData->mStride;
                                    m_databuf        = new MUINT8 [m_databuf_size];

                                    CAM_LOGD( "#(%d) allocate buffer(%p) for CamSV data : size %d(%d), stride %d(%d)",
                                              m_MagicNumber,
                                              m_databuf,
                                              m_databuf_size,
                                              pSttData->mSize,
                                              m_databuf_stride,
                                              pSttData->mStride);
                                }
                                PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "ProcessBuf#%d", m_MagicNumber);
                                memcpy( reinterpret_cast<void *>(m_databuf), reinterpret_cast<void *>(pSttData->mVa), sizeof(MUINT8)*(m_databuf_size));
                                PD_TRACE_FMT_END(TRACE_LEVEL_DEF);

                                //post to thread
                                ::sem_wait( &m_semPDBufThdEnd); //to be 1, it won't block, 0 means PD task not ready yet
                                ::sem_post( &m_semPDBuf);
                            }
                            else
                            {
                                PD_EXTRACT_INPUT_T sPDExtractInput;

                                //
                                PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "ConvertPDBufFormat#%d", m_MagicNumber);
                                if( (m_profile.BufType==EPDBUF_VC || m_profile.BufType==EPDBUF_PDO) && m_pPDBufMgrCore)
                                {

                                    MUINT16  *ptrConvertedLRBuf = m_pPDBufMgrCore->ConvertPDBufFormat( pSttData->mSize,
                                                                  pSttData->mStride,
                                                                  reinterpret_cast<MUINT8 *>(pSttData->mVa),
                                                                  m_sPDOHWInfo.u4BitDepth);

                                    if( m_i4DbgPdDump)
                                    {
                                        char fileName[256];
                                        FILE *fp = nullptr;

                                        if( ptrConvertedLRBuf && m_PDBlockInfo.i4SubBlkW && m_PDBlockInfo.i4SubBlkH)
                                        {
                                            MUINT32 _xsz   = m_PDBlockInfo.i4BlockNumX * (m_PDBlockInfo.i4PitchX/m_PDBlockInfo.i4SubBlkW);
                                            MUINT32 _ysz   = m_PDBlockInfo.i4BlockNumY * (m_PDBlockInfo.i4PitchY/m_PDBlockInfo.i4SubBlkH) * 2; //2: L and R pixels
                                            MUINT32 _bufSz = _xsz*_ysz; //Pixels

                                            // get porting result
                                            MUINT32 PDXsz, PDYsz, PDBufSz;
                                            m_pPDBufMgrCore->GetLRBufferInfo(PDXsz, PDYsz, PDBufSz);

                                            if( (PDXsz==_xsz) && (PDYsz==_ysz) && (PDBufSz==_bufSz))
                                            {
                                                sprintf(fileName,
                                                        "/sdcard/pdo/%" PRId64 "_%05d_cvtBuf_size_0x%lx_stride_0x%lx_pixelNumX_%d_pixelNumY_%d_sofTS_%" PRId64 "_expTime_%" PRIu64 "_curLens_%d_%d_preLens_%d_%d_pixelclk_%d_linelength_%d.raw",
                                                        _GET_TIME_STAMP_US_(),
                                                        m_MagicNumber,
                                                        _bufSz*sizeof(MUINT16),
                                                        _xsz*sizeof(MUINT16),
                                                        _xsz,
                                                        _ysz,
                                                        m_time_stamp_sof,
                                                        m_exposureTime,
                                                        m_lensInfo.u4CurrentPosition,
                                                        m_lensInfo.u8CurrentTimestamp,
                                                        m_lensInfo.u4PreviousPosition,
                                                        m_lensInfo.u8PreviousTimestamp,
                                                        m_Sensor_pixel_clk,
                                                        m_Sensor_line_length);
                                                fp = fopen(fileName, "w");
                                                if( fp)
                                                {
                                                    CAM_LOGD( "dump file : %s", fileName);
                                                    fwrite(reinterpret_cast<void *>(ptrConvertedLRBuf), 1, _bufSz*sizeof(MUINT16), fp);
                                                    fclose(fp);
                                                }
                                                else
                                                {
                                                    CAM_LOGW( "[%s] can not dump cvtBuf to %s !!", __FUNCTION__, fileName);
                                                }
                                            }
                                            else
                                            {
                                                CAM_LOGE( "#(%d) porting task is not correct !! size mismatch : PDXsz(%d!=%d) PDYsz(%d!=%d) PDBufSz(%d!=%d)",
                                                          m_MagicNumber,
                                                          PDXsz,
                                                          _xsz,
                                                          PDYsz,
                                                          _ysz,
                                                          PDBufSz,
                                                          _bufSz);
                                            }
                                        }
                                    }

                                    sPDExtractInput.pRawBuf = NULL;
                                    sPDExtractInput.pPDBuf = reinterpret_cast<MVOID *>(ptrConvertedLRBuf);

                                    //
                                    m_pAnalysisData_Calculate->pBufForExtractPD = sPDExtractInput.pPDBuf;
                                }
                                else if( m_profile.BufType & MSK_CATEGORY_DUALPD)
                                {
                                    sPDExtractInput.pRawBuf = NULL;
                                    sPDExtractInput.pPDBuf = reinterpret_cast<MVOID *>(pSttData->mVa);

                                    //
                                    m_pAnalysisData_Calculate->pBufForExtractPD = sPDExtractInput.pPDBuf;

                                }
                                else //EPDBUF_RAW_LEGACY
                                {
                                    sPDExtractInput.pRawBuf = reinterpret_cast<MVOID *>(pSttData->mVa);
                                    sPDExtractInput.pPDBuf  = NULL;

                                    //
                                    m_pAnalysisData_Calculate->pBufForExtractPD = sPDExtractInput.pRawBuf;
                                }
                                PD_TRACE_FMT_END(TRACE_LEVEL_DEF);

                                PD_EXTRACT_INPUT_T *aExtPDInput[AF_PSUBWIN_NUM],sExtPDInputList[AF_PSUBWIN_NUM];
                                PD_EXTRACT_DATA_T *aPDExtractData[AF_PSUBWIN_NUM];
                                for( MUINT32 i=0; i<m_pAnalysisData_Calculate->numDa; i++)
                                {
                                    sExtPDInputList[i].sFocusWin.i4X = m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI.i4X;
                                    sExtPDInputList[i].sFocusWin.i4Y = m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI.i4Y;
                                    sExtPDInputList[i].sFocusWin.i4W = m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI.i4W;
                                    sExtPDInputList[i].sFocusWin.i4H = m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI.i4H;
                                    sExtPDInputList[i].sFocusWin.sAFType = (eAF_ROI_TYPE_T)(m_pAnalysisData_Calculate->Da[i].sRoiInfo.sType);
                                    sExtPDInputList[i].pRawBuf = sPDExtractInput.pRawBuf;
                                    sExtPDInputList[i].pPDBuf  = sPDExtractInput.pPDBuf;
                                    aExtPDInput[i] = &sExtPDInputList[i];

                                    // prepare input argument for handlePD
                                    m_pAnalysisData_Calculate->Da[i].sHandlePDInput.i4CurLensPos  = GetLensAdpCompPos( m_Sensor_pixel_clk, m_Sensor_line_length, m_exposureTime, m_time_stamp_sof, m_lensInfo, m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI);
                                    m_pAnalysisData_Calculate->Da[i].sHandlePDInput.i4SensorAGain = m_afeGain;
                                    m_pAnalysisData_Calculate->Da[i].sHandlePDInput.i4ISO         = m_iso;

                                    aPDExtractData[i] = &m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData;
                                }
                                PD_TRACE_FMT_BEGIN(TRACE_LEVEL_DEF, "extractAllPD");
                                ret = m_pIPdAlgo->extractAllPD( m_pAnalysisData_Calculate->numDa,
                                                                aExtPDInput,
                                                                aPDExtractData,
                                                                NULL);
                                PD_TRACE_FMT_END(TRACE_LEVEL_DEF);


                                CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] extractPD + (buffer address(%p/%p))", m_MagicNumber, __FUNCTION__, sPDExtractInput.pRawBuf, sPDExtractInput.pPDBuf);
                                for( MUINT32 i=0; i<m_pAnalysisData_Calculate->numDa; i++)
                                {
                                    sPDExtractInput.sFocusWin.i4X     = m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI.i4X;
                                    sPDExtractInput.sFocusWin.i4Y     = m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI.i4Y;
                                    sPDExtractInput.sFocusWin.i4W     = m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI.i4W;
                                    sPDExtractInput.sFocusWin.i4H     = m_pAnalysisData_Calculate->Da[i].sRoiInfo.sPDROI.i4H;
                                    sPDExtractInput.sFocusWin.sAFType = (eAF_ROI_TYPE_T)(m_pAnalysisData_Calculate->Da[i].sRoiInfo.sType);

                                    if( m_bDebugEnable || ret==E_3A_ERR)
                                    {
                                        PD_EXTRACT_DATA_T *pExtractedData = &(m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData);

                                        CAM_LOGD( "#(%d) [%s] extractPD : win#%d type(%d->%d/%d), ROI[x(%4d->%4d/%4d),y(%4d->%4d/%4d),w(%4d->%4d/%4d),h(%4d->%4d/%4d)], buf[%p/%p/%p/%p] enLensAdpComp:DAC(%d->%d)",
                                                  m_MagicNumber,
                                                  __FUNCTION__,
                                                  i,
                                                  sPDExtractInput.sFocusWin.sAFType,
                                                  pExtractedData->sPdWin.sAFType,
                                                  pExtractedData->sPdBlk.sAFType,
                                                  sPDExtractInput.sFocusWin.i4X,
                                                  pExtractedData->sPdWin.i4X,
                                                  pExtractedData->sPdBlk.i4X,//refined focus PD window (refer to PD image coordinate));
                                                  sPDExtractInput.sFocusWin.i4Y,
                                                  pExtractedData->sPdWin.i4Y,
                                                  pExtractedData->sPdBlk.i4Y,
                                                  sPDExtractInput.sFocusWin.i4W,
                                                  pExtractedData->sPdWin.i4W,
                                                  pExtractedData->sPdBlk.i4W,
                                                  sPDExtractInput.sFocusWin.i4H,
                                                  pExtractedData->sPdWin.i4H,
                                                  pExtractedData->sPdBlk.i4H,
                                                  pExtractedData->pPDLData,
                                                  pExtractedData->pPDRData,
                                                  pExtractedData->pPDLPos,
                                                  pExtractedData->pPDRPos,
                                                  m_lensInfo.u4CurrentPosition,
                                                  m_pAnalysisData_Calculate->Da[i].sHandlePDInput.i4CurLensPos);
                                    }

                                    if( ret==E_3A_ERR)
                                    {
                                        CAM_LOGE( "#(%d) extractPD fail #%d, rest extractPD result", m_MagicNumber, i);
                                        memset( &m_pAnalysisData_Calculate->Da[i].sHandlePDInput.sPDExtractData, 0, sizeof(PD_EXTRACT_DATA_T));
                                    }
                                }
                                CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s] extractPD -", m_MagicNumber, __FUNCTION__);

                                //post to thread
                                ::sem_wait( &m_semPDBufThdEnd); //to be 1, it won't block, 0 means PD task not ready yet
                                ::sem_post( &m_semPDBuf);
                            }

                        }
                        else
                        {
                            CAM_LOGW( "#(%d) [%s] Current lens position %d should be in the range (0,1023]. Please check lens driver or hybrid AF flow!!",
                                      m_MagicNumber,
                                      __FUNCTION__,
                                      m_lensInfo.u4CurrentPosition);
                        }
                    }
                    else
                    {

                        CAM_LOGD_IF( m_bDebugEnable,
                                     "#(%d) [%s] abort[No Need PD Result : force trigger calculation(%d), perframe calculation(%d), face exist(%d), imgo_format(%d), imgo_H(%d), imgo_W(%d)]",
                                     m_MagicNumber,
                                     __FUNCTION__,
                                     m_bForceCalculation,
                                     m_pNVRam->rAFNVRAM.i4EasyTuning[2],
                                     fdexist,
                                     pSttData->mImgoFmt,
                                     pSttData->mImgoSizeH,
                                     pSttData->mImgoSizeW);

                    }
                }
                else
                {
                    CAM_LOGD( "#(%d) [%s] PD manager is not completed configed(Please check hybrid AF flow for updating PD parameters) or Data buffer(%p) is NULL",
                              m_MagicNumber,
                              __FUNCTION__,
                              m_pAnalysisData_Calculate);
                }
                ret = S_3A_OK;

            }
            else
            {
                CAM_LOGE( "#(%d) [%s] close thread : sttData(%p), lensNVRam(%p)", m_MagicNumber, __FUNCTION__, pSttData, m_pLensNVRam);
                ret = E_3A_ERR;
            }

            if( ret==E_3A_ERR)
            {
                closeThread();
            }
        }
        else
        {
            CAM_LOGD( "#(%d) [%s] semaphore(m_semPDBufThdEnd=%d), Can not post command to calculation thread(PDBufThd), because calculation still executed.",
                      m_MagicNumber,
                      __FUNCTION__,
                      Val);
        }
    }
    else
    {
        CAM_LOGW( "#(%d) [%s] Can not post to pd thread because of thread is closed", m_MagicNumber, __FUNCTION__);
        ret = E_3A_ERR;
    }


    if( /*buffer life cycle control(imgo)*/
        (m_profile.u4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY) &&
        (pSttData))
    {
        if( (pSttData->mImgoSizeH!=1) &&(pSttData->mImgoSizeW!=0))
        {
            {
                Mutex::Autolock lock( m_Lock_RawBufCtrl);

                //
                RAW_BUF_CTRL_T ctrl( pSttData->mMagicNumber, pSttData->mImgoFmt, pSttData->mImgHeapID);
                m_vRawBufCtrl.push_back(ctrl);


                if( m_bDebugEnable)
                {
                    //
                    char  dbgMsgBuf_bufctrl[DBG_MSG_BUF_SZ];
                    char* ptrMsgBuf_bufctrl = dbgMsgBuf_bufctrl;
                    ptrMsgBuf_bufctrl += sprintf( ptrMsgBuf_bufctrl, "[%s]:raw buffer control vector information: Size(%lu), MagicNum/ImgoFmt/HeapID:", __FUNCTION__, m_vRawBufCtrl.size());
                    for( MUINT32 i=0; i<m_vRawBufCtrl.size(); i++)
                    {
                        ptrMsgBuf_bufctrl += sprintf( ptrMsgBuf_bufctrl,
                                                      "(%d/%d/%d),",
                                                      m_vRawBufCtrl[i].mMagicNumber,
                                                      m_vRawBufCtrl[i].mImgoFmt,
                                                      m_vRawBufCtrl[i].mImgHeapID);
                    }
                    CAM_LOGD_IF( m_bDebugEnable, "%s", dbgMsgBuf_bufctrl);

                }
            }
            // Notify PDENode can release buffer
            ::sem_post( &m_semSWPDE);
        }
    }



    PD_TRACE_FMT_END(TRACE_LEVEL_DBG);

    return ret;
}

//
MRESULT
PDMgr::getPDTaskResult( PD_CALCULATION_OUTPUT **ptrResultOutput)
{
    MRESULT ret = E_3A_ERR;

    if( /* checking API's condition */
        (m_bEnPDBufThd           ) &&
        ((*ptrResultOutput)==nullptr))
    {
        Mutex::Autolock lock(m_Lock_Result);

        if( m_vPDOutput.size())
        {
            vector<SPDResult_T>::iterator itr = m_vPDOutput.begin();

            //allocate result buffer
            (*ptrResultOutput) = new PD_CALCULATION_OUTPUT(itr->numROIs);

            //output
            (*ptrResultOutput)->frmNum     = itr->magicNumber;

            if( itr->numROIs!=((*ptrResultOutput)->numRes))
            {
                CAM_LOGE( "[%s] Result size is mismatch (%d != %d). !!! should not be happened !!!",
                          __FUNCTION__,
                          itr->numROIs,
                          (*ptrResultOutput)->numRes);

                //
                (*ptrResultOutput)->numRes = (itr->numROIs<((*ptrResultOutput)->numRes)) ? itr->numROIs : (*ptrResultOutput)->numRes;
            }
            memcpy( (*ptrResultOutput)->Res, itr->ROIRes, sizeof(SPDROIResult_T)*((*ptrResultOutput)->numRes));

            if( m_bDebugEnable)
            {
                CAM_LOGD_IF( m_bDebugEnable,
                             "[%s] frmNum(%d) numRes(%d)",
                             __FUNCTION__,
                             (*ptrResultOutput)->frmNum,
                             (*ptrResultOutput)->numRes);

                for( MUINT32 i=0; i<(*ptrResultOutput)->numRes;)
                {
                    char  dbgMsgBuf[DBG_MSG_BUF_SZ];
                    char* ptrMsgBuf = dbgMsgBuf;

                    ptrMsgBuf += sprintf( ptrMsgBuf, "[%s]", __FUNCTION__);
                    for( MUINT32 j=0; j<3 && i<(*ptrResultOutput)->numRes; j++)
                    {
                        ptrMsgBuf += sprintf( ptrMsgBuf,
                                              " #%d(%d,%4d,%4d,%4d,%4d,%3d->%3d,%3d,%3lu,%3lu,%d)",
                                              i,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sType,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sPDROI.i4X,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sPDROI.i4Y,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sPDROI.i4W,
                                              (*ptrResultOutput)->Res[i].sROIInfo.sPDROI.i4H,
                                              (*ptrResultOutput)->Res[i].CurLensPos,
                                              (*ptrResultOutput)->Res[i].DesLensPos,
                                              (*ptrResultOutput)->Res[i].Confidence,
                                              (*ptrResultOutput)->Res[i].ConfidenceLevel,
                                              (*ptrResultOutput)->Res[i].PhaseDifference,
                                              (*ptrResultOutput)->Res[i].SaturationPercent);

                        i++;
                    }
                    CAM_LOGD_IF( m_bDebugEnable, "%s", dbgMsgBuf);
                }
            }

            //pop_front
            m_vPDOutput.erase( m_vPDOutput.begin());
            ret = S_3A_OK;
        }
        else
        {
            CAM_LOGD_IF( m_bDebugEnable,
                         "[%s] No result in the queue. Result may be still calculated.",
                         __FUNCTION__);
            ret = E_AF_BUSY;
        }
    }
    else
    {
        CAM_LOGD_IF( m_bDebugEnable,
                     "[%s] Wrong input argument. PD buffer thread enable(%d), Output buffer(%p)",
                     __FUNCTION__,
                     m_bEnPDBufThd,
                     ptrResultOutput);

    }

    return ret;

}

//
MRESULT
PDMgr::GetDebugInfo( AF_DEBUG_INFO_T &sOutDbgInfo)
{
    MRESULT ret = E_3A_ERR;

    //Not support open pd library.
    switch( m_profile.BufType)
    {
        case EPDBUF_VC :
        case EPDBUF_RAW_LEGACY :
            if( m_pIPdAlgo)
            {
                ret = m_pIPdAlgo->getDebugInfo(sOutDbgInfo);
            }
            ret = S_AE_OK;
            break;
        default :
            break;
    }

    return ret;
}

//
MRESULT
PDMgr::getPDOHWCfg(ISP_PDO_CFG_T *pCfg)
{
    CAM_LOGD_IF( m_bDebugEnable, "#(%d) [%s]", m_MagicNumber, __FUNCTION__);
    (*pCfg) = m_sPDOHWCfg;
    return S_3A_OK;
}

//
MRESULT
PDMgr::setPBNen(MBOOL bEnable)
{
    CAM_LOGD("[%s] (%d->%d)", __FUNCTION__, m_bEnablePBIN, bEnable);
    m_bEnablePBIN = bEnable;
    return S_3A_OK;
}

//
MRESULT
PDMgr::setAETargetMode(eAETargetMODE eAETargetMode)
{
    CAM_LOGD("[%s] (%d->%d)", __FUNCTION__, m_eAETargetMode, eAETargetMode);
    m_eAETargetMode = eAETargetMode;
    return S_3A_OK;
}

//
MRESULT
PDMgr::isModuleEnable(MBOOL &oModuleEn)
{
    //
    oModuleEn = MFALSE;

    //
    if( /**/
        ( (m_profile.u4PDAF_support_type==PDAF_SUPPORT_RAW_LEGACY)   && (m_profile.BufType==EPDBUF_RAW_LEGACY)                                ) ||
        ( (m_profile.u4PDAF_support_type==PDAF_SUPPORT_RAW)          && (m_profile.BufType==EPDBUF_RAW_LEGACY||m_profile.BufType==EPDBUF_PDO) ) ||
        ( (m_profile.u4PDAF_support_type==PDAF_SUPPORT_CAMSV)        && (m_profile.BufType==EPDBUF_VC)                                        ) ||
        ( (m_profile.u4PDAF_support_type==PDAF_SUPPORT_RAW_DUALPD)   && (m_profile.BufType==EPDBUF_DUALPD_RAW)                                ) ||
        ( (m_profile.u4PDAF_support_type==PDAF_SUPPORT_CAMSV_DUALPD) && (m_profile.BufType==EPDBUF_DUALPD_VC)                                 ) ||
        ( (m_profile.u4PDAF_support_type==PDAF_SUPPORT_CAMSV)        && (m_profile.BufType==EPDBUF_VC_OPEN)                                   ))
    {
        oModuleEn = m_profile.bSensorModeSupportPD;
    }
    else
    {
        CAM_LOGD("[%s] PD Flow is not enabled!! Please checking configuration (%d/0x%x)", __FUNCTION__, m_profile.u4PDAF_support_type, m_profile.BufType);
    }

    return S_3A_OK;

}

//
MRESULT
PDMgr::setRequest(MINT32 iRequestNum)
{
    m_requestNum = iRequestNum;
    return S_3A_OK;
}

};  //  namespace NS3A


