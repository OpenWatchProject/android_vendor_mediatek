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
#define LOG_TAG "camsv_dfs"

#include "camsv_dfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cutils/properties.h>  // For property_get().
#include "isp_function_camsv.h"


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

DECLARE_DBG_LOG_VARIABLE(svdfs);

#undef CAM_DBG
#undef CAM_INF
#undef CAM_WRN
#undef CAM_ERR

#define CAM_DBG(fmt, arg...)        do {\
    if (svdfs_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[%s]-[0x%x]:" fmt,__FUNCTION__,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_INF(fmt, arg...)        do {\
    if (svdfs_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[%s]-[0x%x]:" fmt,__FUNCTION__,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_WRN(fmt, arg...)        do {\
        if (svdfs_DbgLogEnable_ERROR  ) { \
            BASE_LOG_WRN("[%s]-[0x%x]:" fmt,__FUNCTION__,this->m_hwModule, ##arg); \
        }\
    } while(0)

#define CAM_ERR(fmt, arg...)        do {\
    if (svdfs_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[%s]-[0x%x]:" fmt,__FUNCTION__,this->m_hwModule, ##arg); \
    }\
} while(0)


CAMSV_DFS_Mgr::CAMSV_DFS_Mgr(char const* szCallerName, ISP_HW_MODULE hwmodule) : CAM_DFS_Mgr(szCallerName,hwmodule)
{
    DBG_LOG_CONFIG(imageio, svdfs);

    m_pDrv = NULL;
    m_all_Latency = 0;
    m_hwModule = hwmodule;
    CAM_INF("%s",szCallerName);
    m_FSM = e_unknown;

    m_vBWo.clear();
}


MVOID CAMSV_DFS_Mgr::destroyInstance(char const* szCallerName)
{
    CAM_INF("%s",szCallerName);
    if(this->m_pDrv){
        this->m_pDrv->uninit(LOG_TAG);
        this->m_pDrv->destroyInstance();
    }

    delete this;
}


MBOOL CAMSV_DFS_Mgr::CAM_DFS_INIT(vector<MUINT32>* clk_gear)
{
    clk_gear;
    if(this->CAM_DFS_FSM_CHECK(e_init) == MFALSE)
        return MFALSE;
    CAM_DBG("\n");
    this->m_pDrv = (IspDrvCamsv*)IspDrvCamsv::createInstance(this->m_hwModule);

    if(this->m_pDrv == NULL)
        return MFALSE;

    if(this->m_pDrv->init(LOG_TAG) == MFALSE)
        return MFALSE;

    this->m_FSM = e_init;
    return MTRUE;
}

MBOOL CAMSV_DFS_Mgr::CAM_DFS_STOP(void)
{
    if (this->CAM_DFS_FSM_CHECK(e_stop) == MFALSE)
        return MFALSE;
    CAM_DBG("\n");
    if (MFALSE == this->m_pDrv->setDeviceInfo(_SET_PM_QOS_RESET, NULL)) {
        CAM_ERR("Error in reset PMQOS");
        return MFALSE;
    }

    this->m_FSM = e_init;//back to init state for suspend/resume
    return MTRUE;
}


MBOOL CAMSV_DFS_Mgr::CAM_DFS_START(void)
{
    if (this->CAM_DFS_FSM_CHECK(e_start) == MFALSE)
        return MFALSE;
    CAM_DBG("\n");
    if(this->m_all_Latency == HOLD_TILL_START){
        if(this->CAM_DBS_SetBw() == MFALSE)
            CAM_ERR("Error in CAM_DBS_SetBw");
        this->m_all_Latency = SET_LOW_LATENCY;
    }


    this->m_FSM = e_start;

    return MTRUE;
}

MBOOL CAMSV_DFS_Mgr::CAM_DBS_SetBw(T_BW_INPUT* pIn)
{
    typedef enum{
        e_nochange  =   0,  //
        e_low2high  =   1,
        e_high2low  =   2,
    }E_DFS_DIRECT;
    E_DFS_DIRECT direct = e_low2high;

    MBOOL ret = MTRUE;
    _isp_dma_enum_ port;
    DMAO_CAMSV_B dmao;
    MUINT32 change = 0;
    if (this->CAM_DFS_FSM_CHECK(e_set) == MFALSE)
        return MFALSE;
    CAM_DBG("\n");
    //
    dmao.m_pDrv = this->m_pDrv;
    if(MFALSE == dmao.BW_Meter(pIn->input,&this->m_vBWo)){
        CAM_ERR("dmao bw cal. fail\n");
        return MFALSE;
    }

    for(MUINT32 i=0;i<this->m_vBWo.size();i++){
        port = this->m_vBWo.at(i).portID;
        if(port<_camsv_max_){
            direct = (this->m_vBWo.at(i).ThruPut > this->m_BW[port].BW_value) ? e_low2high : \
                ((this->m_vBWo.at(i).ThruPut == this->m_BW[port].BW_value)? e_nochange : e_high2low);
            if(this->m_FSM == e_init){//in this case, pipeline is not started yet! hold setting until start.
                this->m_BW[port].Switch_Latnecy = HOLD_TILL_START;
                this->m_all_Latency = HOLD_TILL_START;
                this->m_BW[port].BW_value = this->m_vBWo.at(i).ThruPut;
                this->m_BW[port].BW_avg = this->m_vBWo.at(i).Avg_ThruPut;
                change++;
            }
            else{
                if(direct == e_low2high){
                    this->m_BW[port].Switch_Latnecy = SET_LOW_LATENCY;
                    this->m_BW[port].BW_value = this->m_vBWo.at(i).ThruPut;
                    this->m_BW[port].BW_avg = this->m_vBWo.at(i).Avg_ThruPut;
                    change++;
                }
                else if (direct == e_high2low){
                    this->m_BW[port].Switch_Latnecy = SET_HIGH_LATENCY;
                    this->m_all_Latency = SET_HIGH_LATENCY;
                    change++;
                }
            }
            CAM_DBG("Port_%d: direct:%d,bw(%d_%d),all_latency:%d,chage:%d\n",port,direct,this->m_vBWo.at(i).ThruPut,\
                    this->m_BW[port].Switch_Latnecy,this->m_all_Latency,change);
        }
        else{
            CAM_ERR("logic err\n");
            ret = MFALSE;
        }
    }


    if(change != 0){
        if(this->m_all_Latency == SET_LOW_LATENCY) {
            if((ret = this->CAM_DBS_SetBw()) == MFALSE)
                CAM_ERR("Error in CAM_DBS_SetBw");
        }
    }
    return ret;
}



MBOOL CAMSV_DFS_Mgr::CAM_DBS_SetBw(void)
{
    MBOOL ret = MTRUE;
    ISP_SV_PM_QOS_INFO_STRUCT pm_qos_info;
    pm_qos_info.module = this->m_hwModule;

    for(MUINT32 i=0;i<_camsv_max_;i++) {
        pm_qos_info.port_bw[i].peak = this->m_BW[i].BW_value;
        pm_qos_info.port_bw[i].avg = this->m_BW[i].BW_avg;
    }

    if((ret = this->m_pDrv->setDeviceInfo(_SET_PM_QOS_INFO, (MUINT8*)&pm_qos_info)) == MFALSE)
        CAM_ERR("Error in SET_PM_QOS_INFO");


    return ret;
}

MBOOL CAMSV_DFS_Mgr::CAM_DBS_Vsync(void)
{
    MBOOL ret = MTRUE;
    CAM_DBG("\n");
    //previous frame's operation
    if (this->m_all_Latency ==  SET_HIGH_LATENCY) {
        _isp_dma_enum_ port;

        for(MUINT32 i=0;i<this->m_vBWo.size();i++){
            port = this->m_vBWo.at(i).portID;
            if(port<_camsv_max_){
                this->m_BW[port].BW_value = this->m_vBWo.at(i).ThruPut;
            }
            else{
                CAM_ERR("logic err, port = %d\n",port);
                return MFALSE;
            }
        }


        //
        if ((ret = this->CAM_DBS_SetBw()) == MFALSE)
            CAM_ERR("Error in CAM_DBS_SetBw");
        else
            this->m_all_Latency--;
    }

    return ret;
}


