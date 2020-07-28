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
 **************************************************/
#ifndef _ISP_FUNCTION_TIMESTAMP_H_
#define _ISP_FUNCTION_TIMESTAMP_H_


#include "isp_drv_cam.h"
#include "isp_function.h"

/**
    temp SW timestamp patch for IMGO/RRZO under EP,need to OFF after SB,( use HW timestamp to relpace SW timestamp)
    and this temp flow can't support DMAO output with CAM_B's only!!!!!!!
    note: this compile option must be the same as the compile option,TIMESTAMP_QUEUE_EN, in camera_isp.c
*/
#define TEMP_SW_TIMESTAMP   0



/**
    under twin mode, slave cam always have no hw timestamp.
    this compile option is for the protection of overflow.
    Cam_b's timetatmp is still meaningless,due to no clk output!
*/
#define HW_TIMESTAMP_WORKAROUND 1
//////////////////////////////////////////////////////////////////////////////
/**
    class for timestamp asscess
*/
class CAM_TIMESTAMP
{
public:
    CAM_TIMESTAMP(void);

    static CAM_TIMESTAMP*   getInstance(ISP_HW_MODULE module,const ISP_DRV_CAM* pDrv);
#if (TEMP_SW_TIMESTAMP == 1)
           MBOOL            TimeStamp_Init(const ISP_DRV_CAM* pDrv);    //need drvptr to get systemT via ioctrl
#endif
#if HW_TIMESTAMP_WORKAROUND
           MBOOL            TimeStamp_mapping(MUINT32* pSec, MUINT32* pUsec, MUINT32* pSec_B, MUINT32* pUsec_B, _isp_dma_enum_ dma,ISP_HW_MODULE module);
#else
           MBOOL            TimeStamp_mapping(MUINT32* pSec, MUINT32* pUsec, MUINT32* pSec_B, MUINT32* pUsec_B, _isp_dma_enum_ dma,ISP_HW_MODULE nocare = CAM_MAX);
#endif
           MBOOL            TimeStamp_reset(void); //clear systemT of 1st SOF
           MBOOL            TimeStamp_SrcClk(MUINT32 clk);// 10 -> 1mhz, 20-> 2mhz
private:
           MBOOL            TimeStamp_calibration(void);                            //system time calibration


private:
    #define UN_SupportedDma (0xFF)

    ISP_HW_MODULE   m_hwModule;

    ISP_DRV_CAM*    m_pDrv;
    struct{
        MUINT32         prvClk;
        MUINT32         nOverFlow;
    }m_Arithmetic[_cam_max_];                //seperate each dmao channel is for thread-racing.

    typedef struct {
        MUINT32 sec;
        MUINT32 usec;
    } S_START_T;
    S_START_T       m_startTime;            // systemT of 1st SOF
#if TEMP_SW_TIMESTAMP
    static MUINT32  m_TimeClk;             // 10 -> 1mhz, 20-> 2mhz
#else
    MUINT32         m_TimeClk;             // 10 -> 1mhz, 20-> 2mhz
#endif
    MUINT32                 m_CCU_reg_value_HIGH = 0;
    MUINT32                 m_CCU_reg_value_LOW = 0;
    MUINT32                 m_start_time_CCU[2];
    mutable Mutex           mCCU_Lock;
    MBOOL                   mCCU_disabled;
};

#endif
