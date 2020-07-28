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
#ifndef _CAM_DFS_H_
#define _CAM_DFS_H_
#include <vector>
#include <mutex>
#include "isp_drv_cam.h"


using namespace std;
using android::Mutex;

#ifndef MFALSE
#define MFALSE 0
#endif
#ifndef MTRUE
#define MTRUE 1
#endif
#ifndef MUINT8
typedef unsigned char MUINT8;
#endif

#ifndef MUINT32
typedef unsigned int MUINT32;
#endif
#ifndef MINT32
typedef int MINT32;
#endif
#ifndef MBOOL
typedef int MBOOL;
#endif

#ifndef MVOID
typedef void MVOID;
#endif

class CAM_DFS_Mgr
{
public:
    CAM_DFS_Mgr(char const* szCallerName, ISP_HW_MODULE hwmodule);
    ~CAM_DFS_Mgr();
public:
    static CAM_DFS_Mgr* createInstance(char const* szCallerName, ISP_HW_MODULE hwmodule);
    MVOID               destroyInstance(char const* szCallerName);
    //
    //list size: supported level,
    //list iterator: isp clk. e.g.: 1mhz -> 1000000
    MBOOL               CAM_DFS_INIT(vector<MUINT32>* clk_gear);

    //return level, not a specific clk rate
    MUINT32             CAM_DFS_GetCurLv(void);
    //set level: level bigger , clk higher
    MBOOL               CAM_DFS_SetLv(MINT32 level,MBOOL immediate = MFALSE);
    //invoked at each SOF
    MBOOL               CAM_DFS_Vsync(void);
    //change module target lv to lowest
    MBOOL                CAM_DFS_STOP(void);
    //change m_FSM to init state
    MBOOL                CAM_DFS_START(void);
    //BW
    MBOOL                CAM_DBS_Vsync(void);
    MBOOL                CAM_DFS_SetBw(void);
    MBOOL                CAM_DFS_SetBw(MUINT32 total_bw,MUINT32 fps,MBOOL immediate);

    IspDrvVir*          m_pIspDrv;              // obj ptr of current sw target Isp_Drv

private:
    #define SET_NO_CHANGE       (2) //if clk is no needed to change.
    #define SET_HIGH_LATENCY    (1) //if clk is high to low, need 1 frame latency to sync with isp hw fetch-timing
    #define SET_LOW_LATENCY     (0) //if clk is low to high, need 0 frame latency for sync with isp hw fetch-timing


    MUINT32                m_Switch_Latnecy;//To record is needed to pull clk low/high
    MUINT32                m_TargetLv;           //Target level we needed after calculating the throughput.
    MUINT32                m_TargetBw;
    MUINT32                m_TargetBw_fps;
    MBOOL                  m_delayOneFrame_bw;
    ISP_HW_MODULE          m_hwModule; //Record which hw module
    ISP_DRV_CAM*           m_pDrv;     //for casting type , coding convenience only
    typedef enum{
        e_unknown   =   0,
        e_init      =   1,
        e_get       =   2,
        e_set       =   3,
        e_stop     =    4,
    }E_FSM;
    E_FSM           m_FSM;

    static Mutex                m_Lock;     //this lock is for multi-sensor to protect the clk can't be changed at right the time.
    static vector<MUINT32>      m_SupportedClk;   //To record how many level is supported in this platform.
    static MINT32               m_moduleTargetLevel[CAM_MAX]; //Record each cam's target level
    static MINT32               m_curClkLevel; //Record cur level
    static MBOOL                m_bFirstInit;
private:
    MBOOL               CAM_DFS_FSM_CHECK(E_FSM op);
    MBOOL               CAM_DFS_setLv(void);
};

#endif
