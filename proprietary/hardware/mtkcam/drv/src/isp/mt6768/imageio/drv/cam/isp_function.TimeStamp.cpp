
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
#include <sys/mman.h>
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

#if TEMP_SW_TIMESTAMP
MUINT32  CAM_TIMESTAMP::m_TimeClk = 0;             // 10 -> 1mhz, 20-> 2mhz
#endif

CAM_TIMESTAMP::CAM_TIMESTAMP(void)
{
    for(MUINT32 i=0;i<_cam_max_;i++){
        m_Arithmetic[i].prvClk = m_Arithmetic[i].nOverFlow = 0;
    }
    m_hwModule = CAM_MAX;
    m_pDrv = NULL;
    m_startTime = {0,0};
    m_TimeClk = 0;
    mCCU_disabled = MFALSE;
    DBG_LOG_CONFIG(imageio, cam_ts);
}

CAM_TIMESTAMP* CAM_TIMESTAMP::getInstance(ISP_HW_MODULE module,const ISP_DRV_CAM* pDrv)
{
    static CAM_TIMESTAMP gTimeStamp[CAMSV_MAX];
    //
    if(module == CAM_A_TWIN)
        module = CAM_A;
    //
    gTimeStamp[module].m_hwModule = module;
    gTimeStamp[module].m_pDrv = (ISP_DRV_CAM*)pDrv;

    return &gTimeStamp[module];
}

#if TEMP_SW_TIMESTAMP
MBOOL CAM_TIMESTAMP::TimeStamp_Init(const ISP_DRV_CAM* pDrv)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)pDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    return MTRUE;
}
#endif
MBOOL CAM_TIMESTAMP::TimeStamp_mapping(MUINT32* pSec, MUINT32* pUsec, MUINT32* pSec_B, MUINT32* pUsec_B, _isp_dma_enum_ dma,ISP_HW_MODULE module)
{
    MBOOL ret = MTRUE;
    MUINT64 time;
    MUINT64 time_CCU;

    Mutex::Autolock lock(this->mCCU_Lock);

    //overflow case, *pSec is clk on current header
#if (HW_TIMESTAMP_WORKAROUND == 1)
    //only master cam will update these info.
    if(module == this->m_hwModule){
        if(*pSec < this->m_Arithmetic[dma].prvClk){
            this->m_Arithmetic[dma].nOverFlow++;
        }

        this->m_Arithmetic[dma].prvClk = *pSec;
    }
#else
    if(*pSec < this->m_Arithmetic[dma].prvClk){
        this->m_Arithmetic[dma].nOverFlow++;
    }

    this->m_Arithmetic[dma].prvClk = *pSec;
#endif

    if (mCCU_disabled == MFALSE) {
        /*CCU spare-register value as start-time clock count part*/
        if((this->m_CCU_reg_value_HIGH == 0) && (this->m_CCU_reg_value_LOW == 0)){

            if((ret = this->m_pDrv->getDeviceInfo(_GET_TRANSFOR_CCU_REG, (MUINT8*)(this->m_start_time_CCU))) == MFALSE){
                CAM_FUNC_ERR("get ccu reg transfor fail\n");
            }
            this->m_CCU_reg_value_LOW = this->m_start_time_CCU[0];
            this->m_CCU_reg_value_HIGH = this->m_start_time_CCU[1];
            *pSec_B = this->m_start_time_CCU[0];
            *pUsec_B  = this->m_start_time_CCU[1];
            CAM_FUNC_INF("m_CCU_reg_value_HIGH=%u , m_CCU_reg_value_LOW=%u", m_CCU_reg_value_HIGH, m_CCU_reg_value_LOW);
            /* to avoid 32bits project not support ISP_TRANSFOR_CCU_REG, add disable flag */
            if (this->m_CCU_reg_value_LOW == 0 && this->m_CCU_reg_value_HIGH == 0) {
                mCCU_disabled = MTRUE;
            }
        } else {
            *pSec_B = this->m_start_time_CCU[0];
            *pUsec_B  = this->m_start_time_CCU[1];
        }
    } else {
        *pSec_B = 0;
        *pUsec_B  = 0;
    }

    MUINT32 tmp_time_CCU[2];
    tmp_time_CCU[0] = *pSec_B;
    tmp_time_CCU[1] = *pUsec_B;

    time_CCU = (MUINT64)tmp_time_CCU[0] * 1000000 + tmp_time_CCU[1];
    time_CCU += ( ((MUINT64)(*pSec) + (MUINT64)this->m_Arithmetic[dma].nOverFlow * 0xFFFFFFFF) * 10 / this->m_TimeClk ); //us

    *pSec_B = time_CCU/1000000;
    *pUsec_B = time_CCU % 1000000;

    /*original time-stamp part*/
#if (TEMP_SW_TIMESTAMP == 0)
    if((this->m_startTime.sec + this->m_startTime.usec) == 0){
        this->TimeStamp_calibration();
    }
    time = (MUINT64)this->m_startTime.sec * 1000000 + this->m_startTime.usec;
#else
    {
        MUINT32 start_time[3];

        start_time[0] = dma;//pass dma_id to kernel
        if( (ret = this->m_pDrv->getDeviceInfo(_GET_START_TIME,(MUINT8*) start_time)) == MFALSE){
            CAM_FUNC_ERR("timestamp calibration fail\n");
        }
        time = (MUINT64)start_time[0] * 1000000 + start_time[1];
    }
#endif
    //current time-axis
    if(this->m_TimeClk == 0){
        CAM_FUNC_ERR("no source clk\n");
        this->m_TimeClk = 1;
    }

    time += ( ((MUINT64)(*pSec) + (MUINT64)this->m_Arithmetic[dma].nOverFlow * 0xFFFFFFFF) * 10 / this->m_TimeClk ); //us

    *pUsec = time % 1000000;
    *pSec = time/1000000;

    return ret;
}

MBOOL CAM_TIMESTAMP::TimeStamp_calibration(void)
{
    MBOOL ret = MTRUE;
    MUINT32 start_time[3];

    if( (ret = this->m_pDrv->getDeviceInfo(_GET_START_TIME,(MUINT8*) start_time)) == MFALSE){
        CAM_FUNC_ERR("timestamp calibration fail\n");
    }
    this->m_startTime.sec = start_time[0];
    this->m_startTime.usec = start_time[1];

    CAM_FUNC_INF("SystemT k:%d_%d\n",this->m_startTime.sec,this->m_startTime.usec);
    return ret;
}

MBOOL CAM_TIMESTAMP::TimeStamp_reset(void)
{
    MBOOL ret = MTRUE;

    if((this->m_startTime.sec + this->m_startTime.usec) == 0){
        CAM_FUNC_WRN("System-T reset before calibration\n");
    } else {
        this->m_startTime.sec = this->m_startTime.usec = 0;
        this->m_CCU_reg_value_HIGH = this->m_CCU_reg_value_LOW = 0;
        for(MUINT32 i=0;i<_cam_max_;i++){
            m_Arithmetic[i].prvClk = m_Arithmetic[i].nOverFlow = 0;
        }
    }

    CAM_FUNC_INF("SystemT reset\n");
    return ret;
}

MBOOL CAM_TIMESTAMP::TimeStamp_SrcClk(MUINT32 clk)
{
    this->m_TimeClk = clk;
    //clk : 10 -> 1Mhz, 20 -> 2Mhz
    CAM_FUNC_INF("ClkSrc : %dHz\n",clk*100000);

    return MTRUE;
}
