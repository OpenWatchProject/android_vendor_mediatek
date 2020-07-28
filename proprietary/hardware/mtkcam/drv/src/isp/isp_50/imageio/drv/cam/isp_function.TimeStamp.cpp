
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

#define LOG_TAG "cam_TS"

#include <mtkcam/def/PriorityDefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cutils/properties.h>  // For property_get().


#include "isp_function.timestamp.h"
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


DECLARE_DBG_LOG_VARIABLE(cam_ts);

// Clear previous define, use our own define.
#undef CAM_FUNC_VRB
#undef CAM_FUNC_DBG
#undef CAM_FUNC_INF
#undef CAM_FUNC_WRN
#undef CAM_FUNC_ERR
#undef CAM_FUNC_AST
#define CAM_FUNC_VRB(fmt, arg...)        do { if (cam_ts_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define CAM_FUNC_DBG(fmt, arg...)        do {\
    if (cam_ts_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_INF(fmt, arg...)        do {\
    if (cam_ts_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_WRN(fmt, arg...)        do {\
    if (cam_ts_DbgLogEnable_WARN   ) { \
        BASE_LOG_WRN("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    if (cam_ts_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (cam_ts_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

CAM_TIMESTAMP::CAM_TIMESTAMP(void)
{
    m_hwModule = CAM_MAX;
    m_pDrv = NULL;
    m_pCamSVDrv = NULL;
    m_stmpMode = LOCAL_MODE;
    m_startTime = {0,0};
    m_TimeClk = 0;
    DBG_LOG_CONFIG(imageio, cam_ts);
}

CAM_TIMESTAMP* CAM_TIMESTAMP::getInstance(ISP_HW_MODULE module,const ISP_DRV_CAM* pDrv,const IspDrvCamsv* pSVDrv)
{
    static CAM_TIMESTAMP gTimeStamp[CAMSV_MAX];
    //
    if(module == CAM_A_TWIN_B || module == CAM_A_TWIN_C || module == CAM_A_TRIPLE_B || module == CAM_A_TRIPLE_C)
        module = CAM_A;
    else if (module == CAM_B_TWIN_C)
        module = CAM_B;
    //
    gTimeStamp[module].m_hwModule = module;

    if (module < PHY_CAM) {
        gTimeStamp[module].m_pDrv = (ISP_DRV_CAM*)pDrv;
        CAM_WRITE_BITS(gTimeStamp[module].m_pDrv->getPhyObj(),CAM_TG_SEN_MODE,TIME_STP_EN,1);
    } else if (CAMSV_0 <= module && module < CAMSV_MAX) {
        gTimeStamp[module].m_pCamSVDrv = (IspDrvCamsv*)pSVDrv;
        CAMSV_WRITE_BITS(gTimeStamp[module].m_pCamSVDrv,CAMSV_TG_SEN_MODE,TIME_STP_EN,1);
    }

    return &gTimeStamp[module];
}

MBOOL CAM_TIMESTAMP::TimeStamp_mapping(MUINT64* pSec, MUINT32* pUsec,_isp_dma_enum_ dma, ISP_HW_MODULE masterModule)
{
#if 0
    MBOOL ret = MTRUE;
    MUINT64 time;

#define SECtoUS 1000000

    if (this->m_stmpMode == LOCAL_MODE) {
        //get AP system time
        if((this->m_startTime.sec + this->m_startTime.usec) == 0){
            if (this->TimeStamp_calibration(masterModule) == MFALSE) {
                CAM_FUNC_ERR("SystemTime calibration error");
                ret = MFALSE;
                return ret;
            }
        }

        //usec unit.
        time = (MUINT64)this->m_startTime.sec * SECtoUS + this->m_startTime.usec;

        //current time-axis
        if(this->m_TimeClk == 0){
            CAM_FUNC_ERR("no source clk\n");
            this->m_TimeClk = 1;
        }
        //*10 is for changing time unit to 0.1Mhz to make sure numerator and denominator are the same time unit (0.1Mhz)
        time += ( (*pSec) * 10 / this->m_TimeClk ); //us
        CAM_FUNC_DBG("time stamp test:%" PRIu64 "_%" PRIu64 ",%d\n",*pSec,time,dma);

    } else { //Global timer
        time = *pSec;

        if (this->m_hwModule < PHY_CAM) {
            if( (ret = this->m_pDrv->getDeviceInfo(_GET_GLOBAL_TIME,(MUINT8*)&time)) == MFALSE){
                CAM_FUNC_ERR("get global time fail\n");
            }
        } else if (CAMSV_0 <= this->m_hwModule && this->m_hwModule < CAMSV_MAX){
            if( (ret = this->m_pCamSVDrv->getDeviceInfo(_GET_GLOBAL_TIME,(MUINT8*)&time)) == MFALSE){
                CAM_FUNC_ERR("get global time fail\n");
            }
        }
        CAM_FUNC_DBG("time stamp test:%" PRIu64 "_%" PRIu64 ",%d\n",*pSec,time,dma);
    }

    *pUsec = time % SECtoUS;
    *pSec = time/SECtoUS;

    return ret;
#else
    CAM_FUNC_ERR("phase out for performance saving\n");
    return MFALSE;
#endif
}

MBOOL CAM_TIMESTAMP::TimeStamp_mapping(MUINT64* pUsec,MUINT64* pUsec_B, _isp_dma_enum_ dma, ISP_HW_MODULE masterModule)
{
    MBOOL ret = MTRUE;
    MUINT64 time[_e_TS_max] = {0,};

    if (this->m_stmpMode == LOCAL_MODE) {
        //get AP system time
        if((this->m_startTime.sec + this->m_startTime.usec) == 0){
            if (this->TimeStamp_calibration(masterModule) == MFALSE) {
                CAM_FUNC_ERR("SystemTime calibration error");
                ret = MFALSE;
                return ret;
            }
        }

        //usec unit.
        time[_e_mono_] = (MUINT64)this->m_startTime.sec * 1000000 + this->m_startTime.usec;

        //current time-axis
        if(this->m_TimeClk == 0){
            CAM_FUNC_ERR("no source clk\n");
            this->m_TimeClk = 1;
        }
        //*10 is for changing time unit to 0.1Mhz to make sure numerator and denominator are the same time unit (0.1Mhz)
        time[_e_mono_] += ( (*pUsec) * 10 / this->m_TimeClk ); //us
        CAM_FUNC_DBG("time stamp test:%" PRIu64 "_%" PRIu64 ",%d\n",*pUsec,time[_e_mono_],dma);
#ifdef GLOBAL_BT
        CAM_FUNC_WRN("support only mono-T under local mode\n");
#endif
    } else { //Global timer
        time[_e_mono_] = *pUsec;

        if (this->m_hwModule < PHY_CAM) {
            if( (ret = this->m_pDrv->getDeviceInfo(_GET_GLOBAL_TIME,(MUINT8*)time)) == MFALSE){
                CAM_FUNC_ERR("get global time fail\n");
            }
        } else if (CAMSV_0 <= this->m_hwModule && this->m_hwModule < CAMSV_MAX){
            if( (ret = this->m_pCamSVDrv->getDeviceInfo(_GET_GLOBAL_TIME,(MUINT8*)time)) == MFALSE){
                CAM_FUNC_ERR("get global time fail\n");
            }
        }


#ifdef GLOBAL_BT
        CAM_FUNC_DBG("time stamp test:%" PRIu64 "_%" PRIu64 "_%" PRIu64 ",%d\n",*pUsec,time[_e_mono_],time[_e_boot_],dma);
#else
        CAM_FUNC_DBG("time stamp test:%" PRIu64 "_%" PRIu64 ",%d\n",*pUsec,time[_e_mono_],dma);
#endif
    }

    *pUsec = time[_e_mono_];
#ifdef GLOBAL_BT
    *pUsec_B = time[_e_boot_];
#endif
    return ret;
}


MBOOL CAM_TIMESTAMP::TimeStamp_calibration(ISP_HW_MODULE masterModule)
{
    MBOOL ret = MTRUE;
    MUINT32 start_time[3];

    if (this->m_hwModule < PHY_CAM) {
        start_time[0] = (MUINT32)masterModule;
        if( (ret = this->m_pDrv->getDeviceInfo(_GET_START_TIME,(MUINT8*) start_time)) == MFALSE){
            CAM_FUNC_ERR("timestamp calibration fail\n");
        }
    } else if (CAMSV_0 <= this->m_hwModule && this->m_hwModule < CAMSV_MAX){
        if( (ret = this->m_pCamSVDrv->getDeviceInfo(_GET_START_TIME,(MUINT8*) start_time)) == MFALSE){
            CAM_FUNC_ERR("timestamp calibration fail\n");
        }
    } else {
        CAM_FUNC_ERR("wrong hw module:0x%x", this->m_hwModule);
        ret = MFALSE;
        return ret;
    }

    this->m_startTime.sec = start_time[0];
    this->m_startTime.usec = start_time[1];

    CAM_FUNC_INF("SystemTime calibration:%d_%d\n",this->m_startTime.sec,this->m_startTime.usec);
    return ret;
}

MBOOL CAM_TIMESTAMP::TimeStamp_reset(void)
{
    MBOOL ret = MTRUE;

    if((this->m_startTime.sec + this->m_startTime.usec) == 0){
        CAM_FUNC_WRN("System-T reset before calibration\n");
    }
    else{
        this->m_startTime.sec = this->m_startTime.usec = 0;
    }

    CAM_FUNC_INF("SystemT reset\n");
    return ret;
}

MBOOL CAM_TIMESTAMP::TimeStamp_SrcClk(MUINT32 clk, TSTMP_MODE mode)
{
    this->m_stmpMode = mode;
    this->m_TimeClk = clk;
    //clk : 10 -> 1Mhz, 20 -> 2Mhz
    CAM_FUNC_INF("ClkSrc : %dHz\n",clk*100000);

    if (this->m_stmpMode == LOCAL_MODE) {
        if (this->m_hwModule < PHY_CAM) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAM_TG_TIME_STAMP_CTL, TG_TIME_STAMP_SEL, 0);
        } else if (CAMSV_0 <= this->m_hwModule && this->m_hwModule < CAMSV_MAX){
            CAMSV_WRITE_BITS(this->m_pCamSVDrv, CAMSV_TG_TIME_STAMP_CTL, TG_TIME_STAMP_SEL, 0);
        }
    } else {
        if (this->m_hwModule < PHY_CAM) {
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAM_TG_TIME_STAMP_CTL, TG_TIME_STAMP_SEL, 1);
        } else if (CAMSV_0 <= this->m_hwModule && this->m_hwModule < CAMSV_MAX){
            CAMSV_WRITE_BITS(this->m_pCamSVDrv, CAMSV_TG_TIME_STAMP_CTL, TG_TIME_STAMP_SEL, 1);
        }
    }

    return MTRUE;
}

