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
#define LOG_TAG "cam_dfs"

#include "cam_dfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cutils/properties.h>  // For property_get().

#ifdef USING_MTK_LDVT
#include <sys/types.h>  //for getid() in log when using ldvt
#include <unistd.h>     //for getid() in log when using ldvt
#endif
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif
#define  ALWAYS_SET (0)

DECLARE_DBG_LOG_VARIABLE(dfs);

#undef CAM_DBG
#undef CAM_INF
#undef CAM_WRN
#undef CAM_ERR

#define CAM_DBG(fmt, arg...)        do {\
    if (dfs_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_INF(fmt, arg...)        do {\
    if (dfs_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_WRN(fmt, arg...)        do {\
        if (dfs_DbgLogEnable_ERROR  ) { \
            BASE_LOG_WRN("[0x%x]:" fmt,this->m_hwModule, ##arg); \
        }\
    } while(0)

#define CAM_ERR(fmt, arg...)        do {\
    if (dfs_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define MHZ 1000000
#define UNINIT_VAR  (0xFF)
Mutex                   CAM_DFS_Mgr::m_Lock;
vector<MUINT32>         CAM_DFS_Mgr::m_SupportedClk;
MINT32                  CAM_DFS_Mgr::m_moduleTargetLevel[CAM_MAX] = {-1,-1,-1};
MINT32                  CAM_DFS_Mgr::m_curClkLevel = UNINIT_VAR; //always use the lowest clk level at first.
MBOOL                  CAM_DFS_Mgr::m_bFirstInit = MTRUE;

CAM_DFS_Mgr::CAM_DFS_Mgr(char const* szCallerName, ISP_HW_MODULE hwmodule)
{
    DBG_LOG_CONFIG(imageio, dfs);

    if (m_bFirstInit == MTRUE) {
        memset(m_moduleTargetLevel, -1, sizeof(m_moduleTargetLevel));
        m_bFirstInit = MFALSE;
    }

    m_pIspDrv = 0;
    m_Switch_Latnecy = 0;
    m_TargetLv = 0;
    m_TargetBw = 0;
    m_TargetBw_fps = 0;
    m_delayOneFrame_bw = MFALSE;
    m_FSM = e_unknown;
    m_pDrv = 0;
    m_hwModule = hwmodule;
    m_moduleTargetLevel[m_hwModule] = 0;

    CAM_INF("%s",szCallerName);
}

CAM_DFS_Mgr::~CAM_DFS_Mgr()
{
    Mutex::Autolock lock(m_Lock);

    m_moduleTargetLevel[m_hwModule] = -1;

    for(MUINT32 i=0; i < CAM_MAX; i++) {
        if (m_moduleTargetLevel[i] != -1)
            break;
        else if (i == (CAM_MAX -1))
            m_curClkLevel = UNINIT_VAR; //reset to lowest clk
    }
}


CAM_DFS_Mgr* CAM_DFS_Mgr::createInstance(char const* szCallerName, ISP_HW_MODULE hwmodule)
{
    return new CAM_DFS_Mgr(szCallerName, hwmodule);
}

MVOID CAM_DFS_Mgr::destroyInstance(char const* szCallerName)
{
    CAM_INF("%s",szCallerName);
    delete this;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_FSM_CHECK(E_FSM op)
{
    MBOOL ret = MTRUE;

    switch(op){
        case e_init:
            if(this->m_FSM != e_unknown)
                ret = MFALSE;
            break;
        case e_get:
            if(this->m_FSM == e_get)
                ret = MFALSE;
            break;
        case e_set:
            if(this->m_FSM <= e_init)
                ret = MFALSE;
            break;
        case e_stop:
            if(this->m_FSM == e_unknown)
                ret = MFALSE;
            break;
        default:
            ret = MFALSE;
            break;
    }

    if(ret == MFALSE){
        CAM_ERR("FSM error: targe op:%d, cur op:%d\n",op,this->m_FSM);
    }
    return ret;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_INIT(vector<MUINT32>* clk_gear)
{
    MBOOL ret = MTRUE;
    ISP_CLK_INFO supportedClk;

    memset(&supportedClk, 0x0, sizeof(ISP_CLK_INFO));

    if(this->CAM_DFS_FSM_CHECK(e_init) == MFALSE)
        return MFALSE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if(m_SupportedClk.size() == 0) {
        // fake isp clk: 320 364 546
        char fakeclk[3][PROPERTY_VALUE_MAX] = {0};

        property_get("vendor.isp.fakeclk1", fakeclk[0], "0");
        property_get("vendor.isp.fakeclk2", fakeclk[1], "0");
        property_get("vendor.isp.fakeclk3", fakeclk[2], "0");

        if(atoi(fakeclk[0]) || atoi(fakeclk[1]) || atoi(fakeclk[2])){
            // use fake clock
            if(atoi(fakeclk[0])){
                CAM_INF("SupportedFakeClk[0]=%d",atoi(fakeclk[0]));
                m_SupportedClk.push_back(atoi(fakeclk[0])*MHZ);
                clk_gear->push_back(atoi(fakeclk[0])*MHZ);
            }
            if(atoi(fakeclk[1])){
                CAM_INF("SupportedFakeClk[1]=%d",atoi(fakeclk[1]));
                m_SupportedClk.push_back(atoi(fakeclk[1])*MHZ);
                clk_gear->push_back(atoi(fakeclk[1])*MHZ);
            }
            if(atoi(fakeclk[2])){
                CAM_INF("SupportedFakeClk[2]=%d",atoi(fakeclk[2]));
                m_SupportedClk.push_back(atoi(fakeclk[2])*MHZ);
                clk_gear->push_back(atoi(fakeclk[2])*MHZ);
            }
        }
        else{
            if((ret = this->m_pDrv->getDeviceInfo(_GET_SUPPORTED_ISP_CLOCKS, (MUINT8*)&supportedClk)) == MFALSE)
                CAM_ERR("Error in _GET_SUPPORTED_ISP_CLOCKS!!");

            for(MINT32 i=(supportedClk.clklevelcnt-1); i >= 0; i--){
                CAM_INF("SupportedClk[%d]=%d",i, supportedClk.clklevel[i]);
                m_SupportedClk.push_back(supportedClk.clklevel[i]*MHZ);
                clk_gear->push_back(supportedClk.clklevel[i]*MHZ);
            }
        }
    }
    else {
        for(MUINT32 i=0; i < m_SupportedClk.size(); i++) {
            clk_gear->push_back(m_SupportedClk.at(i));
        }
    }

    this->m_FSM = e_init;
    return ret;
}

MUINT32 CAM_DFS_Mgr::CAM_DFS_GetCurLv(void)
{
    if(this->CAM_DFS_FSM_CHECK(e_get) == MFALSE)
        return MFALSE;

    Mutex::Autolock lock(m_Lock);

    //
    this->m_FSM = e_get;
    if(this->m_curClkLevel == UNINIT_VAR)
        return 0;
    else
        return m_curClkLevel;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_SetLv(MINT32 level,MBOOL immediate)
{
    MBOOL ret = MTRUE;
    typedef enum{
            e_nochange  =   0,
            e_low2high  =   1,
            e_high2low  =   2,
        }E_DFS_DIRECT;
        E_DFS_DIRECT direct = e_nochange;

    if (this->CAM_DFS_FSM_CHECK(e_set) == MFALSE)
        return MFALSE;

    direct = (level < m_curClkLevel) ? e_high2low : ((level > m_curClkLevel) ? e_low2high : e_nochange);

    switch(direct){
        case e_high2low:
            if(immediate == MTRUE)//for those cases which pipeline is not started yet!
                this->m_Switch_Latnecy = SET_LOW_LATENCY;
            else {
                this->m_Switch_Latnecy = SET_HIGH_LATENCY;
            }
            break;
        case e_low2high:
            this->m_Switch_Latnecy = SET_LOW_LATENCY;
            break;
        case e_nochange:
            this->m_Switch_Latnecy = SET_NO_CHANGE;
            break;
    }

    m_moduleTargetLevel[m_hwModule] = level;

    //case of SET_HIGH_LATENCY is configure at next SOF which is imp. at CAM_DFS_Vsync()
    if((this->m_Switch_Latnecy == SET_LOW_LATENCY) || ALWAYS_SET) {
        if((ret = this->CAM_DFS_setLv()) == MFALSE)
            CAM_ERR("Error in CAM_DFS_setLv");
    }


    this->m_FSM = e_set;

    return ret;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_setLv(void)
{
    MBOOL ret = MTRUE;
    MUINT32 setClk;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("camsys.dfsdisable", value, "0");
    MINT32 dfsdisable = atoi(value);

    this->m_TargetLv = 0;
    for(MUINT32 i = 0; i < CAM_MAX; i++){
        if((MINT32)this->m_TargetLv < m_moduleTargetLevel[i])
            this->m_TargetLv = m_moduleTargetLevel[i];
    }

    //If ALWAYS_SET is 0, only target level doesn't equal to current level is needed to set clock level.
    if ((this->m_TargetLv != (MUINT32)m_curClkLevel) || ALWAYS_SET) {
        if(dfsdisable) {
            CAM_DBG("Disable DFS!! No need to set level");
        } else {
            if(m_SupportedClk.at(this->m_TargetLv)/MHZ > 0){
                setClk = m_SupportedClk.at(this->m_TargetLv)/MHZ;

                CAM_INF("isp clk lv switching to %dMhz\n", setClk);

                Mutex::Autolock lock(m_Lock);
                if((ret = this->m_pDrv->setDeviceInfo(_SET_DFS_UPDATE, (MUINT8*)&setClk)) == MFALSE)
                    CAM_ERR("Error in _SET_DFS_UPDATE");

                m_curClkLevel = this->m_TargetLv;
            } else {
                CAM_ERR("Set clk level err(%dMhz)", m_SupportedClk.at(this->m_TargetLv)/MHZ);
                ret = MFALSE;
            }
        }
    }

    return ret;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_SetBw(MUINT32 total_bw,MUINT32 fps,MBOOL immediate)
{
    MBOOL ret = MTRUE;
    this->m_TargetBw = total_bw;
    this->m_TargetBw_fps = fps;

    if(immediate == MTRUE) {
        if((ret = this->CAM_DFS_SetBw()) == MFALSE)
            CAM_ERR("Error in CAM_DFS_SetBw");
    } else {
          this->m_delayOneFrame_bw = MTRUE;
    }

    return ret;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_SetBw(void)
{
    MBOOL ret = MTRUE;
    ISP_PM_QOS_INFO_STRUCT pm_qos_info;
    pm_qos_info.bw_value = this->m_TargetBw;
    pm_qos_info.fps = this->m_TargetBw_fps;//for per frame calculation
    pm_qos_info.module = m_hwModule;

    Mutex::Autolock lock(m_Lock);
    if((ret = this->m_pDrv->setDeviceInfo(_SET_PM_QOS_INFO, (MUINT8*)&pm_qos_info)) == MFALSE)
        CAM_ERR("Error in SET_PM_QOS_INFO");

    if((ret = this->m_pDrv->setDeviceInfo(_SET_PM_QOS_ON, (MUINT8*)&m_hwModule)) == MFALSE)
        CAM_ERR("Error in SET_PM_QOS");

    this->m_delayOneFrame_bw = MFALSE;

    return ret;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_Vsync(void)
{
    MBOOL ret = MTRUE;

    //previous frame's operation
    if (this->m_Switch_Latnecy ==  SET_HIGH_LATENCY) {
        this->m_Switch_Latnecy = SET_LOW_LATENCY;
        CAM_DBG("Delay one frame to set DFS");
        if ((ret = this->CAM_DFS_setLv()) == MFALSE)
            CAM_ERR("Error in CAM_DFS_setLv");
    }

    return ret;
}

MBOOL CAM_DFS_Mgr::CAM_DBS_Vsync(void)
{
    MBOOL ret = MTRUE;

    //previous frame's operation
    if (this->m_delayOneFrame_bw ==  MTRUE) {
        CAM_DBG("Delay one frame to set DBS");
        if ((ret = this->CAM_DFS_SetBw()) == MFALSE)
            CAM_ERR("Error in CAM_DFS_setBw");
    }

    return ret;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_STOP(void)
{
    if (this->CAM_DFS_FSM_CHECK(e_stop) == MFALSE)
        return MFALSE;

    m_moduleTargetLevel[m_hwModule] = -1;

    this->m_FSM = e_stop;

    return MTRUE;
}

MBOOL CAM_DFS_Mgr::CAM_DFS_START(void)
{
    if (this->CAM_DFS_FSM_CHECK(e_init) == MFALSE)
        return MFALSE;

    this->m_FSM = e_init;

    return MTRUE;
}

