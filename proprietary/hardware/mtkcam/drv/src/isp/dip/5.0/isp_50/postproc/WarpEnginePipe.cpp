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
#define LOG_TAG "WarpEngine"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//mutex

#include <vector>
#include <cutils/atomic.h>
#include <cutils/properties.h>


//#include <ispio_utility.h>
#include <mtkcam/drv/iopipe/PostProc/IHalWpePipeDef.h>

#include "WarpEnginePipe.h"

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif


//tpipe
#include <tpipe_config.h>
#define TRANSFORM_MAX   8  //eTransform in ImageFormat.h

TransformMapping mMDPTransformMapping[TRANSFORM_MAX]=
{
    {0,                 NSImageio::NSIspio::eImgRot_0,  NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H, NSImageio::NSIspio::eImgRot_0,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V, NSImageio::NSIspio::eImgRot_180,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_ROT_180,NSImageio::NSIspio::eImgRot_180,NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_ROT_90, NSImageio::NSIspio::eImgRot_90, NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H|eTransform_ROT_90, NSImageio::NSIspio::eImgRot_270,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V|eTransform_ROT_90, NSImageio::NSIspio::eImgRot_90,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_ROT_270,NSImageio::NSIspio::eImgRot_270,NSImageio::NSIspio::eImgFlip_OFF}
};

using namespace NSImageio;
using namespace NSIspio;



#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(WarpEngine);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (WarpEngine_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (WarpEngine_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (WarpEngine_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (WarpEngine_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (WarpEngine_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (WarpEngine_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//

#define MAX_TPIPE_WIDTH_WPE                  (512)
#define MAX_TPIPE_HEIGHT_WPE                 (65536)
// tpipe irq mode
#define TPIPE_IRQ_FRAME_WPE     (0)
#define TPIPE_IRQ_LINE_WPE      (1)
#define TPIPE_IRQ_TPIPE_WPE     (2)
// for crop function
#define WPE_CROP_FLOAT_PECISE_BIT      31    // precise 31 bit
#define WPE_CROP_TPIPE_PECISE_BIT      24
#define WPE_CROP_TPIPE_PECISE_BIT_20   20

#define TPIPE_SHIFT (1 << 24)

// for  tpipe dump information
#define DUMP_TPIPE_SIZE         100
#define DUMP_TPIPE_NUM_PER_LINE 10




#define TRANSFORM_MAX   8  //eTransform in ImageFormat.h



enum WPEStreamHint
   {
        HINT_WPE_TILE_MODE = 0x0,       // need to do tpipe calculation
        HINT_WPE_TPIPE_NO_CHANGE = 0x1, // can skip tpipe calculation
        HINT_WPE_FRAME_MODE = 0x2,      // ISP-only frame mode
   };

// Default setting
#define VGEN_EN                    1
#define CACHI_EN                   1
#define VECI_EN                    1
#define VEC2I_EN1                  1
#define CACHE_BURST_LEN            2

#define MSKO_EN                    0

#define CTL_EXTENSION              1
#define CACHE_VURST_LEN            2 // 1:burst-2; 2:burst-4; 3:burst-8
#define WPE_DONE_EN                1 
#define TDR_ERR_INT_EN             1
#define Wpe_SW_RST_ST              1
#define Wpe_VGEN_VERTICAL_FIRST    1
#define Wpe_ADDR_GEN_MAX_BURST_LEN 4

#define Wpe_WPEO_SOFT_RST_STAT     1
#define Wpe_MSKO_SOFT_RST_STAT     1
#define Wpe_VECI_SOFT_RST_STAT     1 
#define Wpe_VEC2I_SOFT_RST_STAT    1 
#define Wpe_VEC3I_SOFT_RST_STAT    1
#define ADDR_GEN_SOFT_RSTSTAT      1

WpeDrv*             WarpEnginePipe::m_pWpeDrv = NULL;



//Debug Utility
#define DUMP_WPEPREFIX "/sdcard/wpedump_wpe"


/******************************************************************************
 *
 ******************************************************************************/
IWarpEnginePipe*
IWarpEnginePipe::createInstance()
{
    WarpEnginePipe* pWarpEngineImp = new WarpEnginePipe();
    if  ( ! pWarpEngineImp )
    {
        LOG_ERR("[IWarpEnginePipe] fail to new WarpEngine");
        return  NULL;
    }

    //
    return  pWarpEngineImp;
}

MVOID IWarpEnginePipe:: destroyInstance(void)
{
    delete  this;       //  Finally, delete myself.
}

WarpEnginePipe:: WarpEnginePipe()
    : mWeapEngInitCount (0)
    , mTotalCmdNum (0)
    , m_wpeEQDupIdx (0)
    , m_wpeEQBurstIdx(0)
    , module_idx (WPE_MODULE_IDX_WPE_A)
    , m_wpeCallerID(0)
    , m_wpeDupCmdIdx (0)
    , m_pIMemDrv (NULL)
    , m_pMdpMgr (NULL)
    , m_WarpEngInit(MFALSE)
    , m_iWpeEnqueReq (0)
    , m_bSaveReqToFile (0)
    , mDumpCount (0)
    , wCmdData (NULL)
    , gpWPEMdpMgrCfgData(NULL)
{

}

WarpEnginePipe:: ~WarpEnginePipe()
{

}

MBOOL WarpEnginePipe::
init(ISP_WPE_MODULE_IDX module_idx, MUINT32 secTag)
{
    Mutex::Autolock autoLock(mModuleMtx);

    MBOOL ret = MTRUE;
    MINT32 i,j,k;
    MUINT32 offset;
    MINT32 tmp=0;
    char szBuf[MAX_USER_NAME_SIZE];
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int n;


    tmp = android_atomic_inc(&mWeapEngInitCount);
    LOG_DBG("+,mWpeInitCnt(%d)",mWeapEngInitCount);

    if(mWeapEngInitCount == 1)
    {
        this->module_idx = module_idx;

        if(m_WarpEngInit == MFALSE)
        {
            char WPEStreamName[] = "Normal_Stream";
            strncpy((char*)(m_UserName),(char const*)WPEStreamName, strlen(WPEStreamName));
            m_UserName[strlen(WPEStreamName)]='\0';
            snprintf(szBuf, MAX_USER_NAME_SIZE, "_WPE_%d", mWeapEngInitCount);
            strncat(m_UserName, szBuf, strlen(szBuf));
            m_pWpeDrv = WpeDrv::createInstance();
            if(m_pWpeDrv == NULL )
            {
                LOG_ERR("WarpEngine WpeDrv init fail");
                ret = MFALSE;
                goto EXIT;
            }

            m_pWpeDrv->init(m_UserName);
            // do initial setting
            /*============================================
                    imem driver
            =============================================*/
            m_pIMemDrv = DipIMemDrv::createInstance();
            LOG_DBG("[m_pIMemDrv]m_pIMemDrv(0x%08x)",m_pIMemDrv);
            if ( NULL == m_pIMemDrv || !(m_pIMemDrv->init()) ) {
                LOG_ERR("[Error]DipIMemDrv::createInstance fail.");
                ret = MFALSE;
                goto EXIT;
            }

            /*============================================
                         alloc wpe command buffer
               =============================================*/
#if 1
            WPE_Config **pWPE_ConfigLayer1;
            WPE_Config *pWPE_ConfigLayer2;
            int dup_cmds = 1;

            if (module_idx == WPE_MODULE_IDX_WPE_A)
                dup_cmds = WPE_MODULE_IDX_MAX;

            this->wCmdBufInfo.size = (MAX_WPE_BURST_IDX_NUM * sizeof(int**)+ \
                                MAX_WPE_BURST_IDX_NUM * MAX_WPE_DUP_IDX_NUM * sizeof(int*) + \
                                MAX_WPE_BURST_IDX_NUM * MAX_WPE_DUP_IDX_NUM * dup_cmds * sizeof(MDPMGR_CFG_STRUCT))+(15+1);
            this->wCmdBufInfo.useNoncache = 1; //alloc non-cacheable mem for tpipe.
            if ( m_pIMemDrv->allocVirtBuf(&this->wCmdBufInfo) ) {
                LOG_ERR("[ERROR]allocVirtBuf(wCmdBufInfo) fail,size(0x%x)",this->wCmdBufInfo.size);
            }

            LOG_DBG("[MDP]WPE cmd size(0x%x),Va(0x%x)",this->wCmdBufInfo.size,this->wCmdBufInfo.virtAddr);

            this->wCmdData = (WPE_Config ***)((this->wCmdBufInfo.virtAddr+15)&(~15));

            pWPE_ConfigLayer1 = (WPE_Config**)(this->wCmdData + MAX_WPE_BURST_IDX_NUM);
            pWPE_ConfigLayer2 = (WPE_Config*)(this->wCmdData + MAX_WPE_BURST_IDX_NUM + MAX_WPE_BURST_IDX_NUM * MAX_WPE_DUP_IDX_NUM);

            for(i=0;i<MAX_WPE_BURST_IDX_NUM;i++){
                this->wCmdData[i] = pWPE_ConfigLayer1;
                for(j=0;j<MAX_WPE_DUP_IDX_NUM;j++){
                    this->wCmdData[i][j] = pWPE_ConfigLayer2;
                    pWPE_ConfigLayer2 += (dup_cmds);
                }
                pWPE_ConfigLayer1 += (MAX_WPE_DUP_IDX_NUM);
            }
#endif
#if 0
            for(i=0;i<MAX_WPE_BURST_IDX_NUM;i++)
                for(j=0;j<MAX_WPE_DUP_IDX_NUM;j++)
                    for(k=0;k<dup_cmds;k++)
                        wCmdData[i][j][k] = gCmdData[i][j][k];
#endif

            /*============================================
                    alloc tpipe buffer
               =============================================*/
            int dup_tpipe_buffer_size = 2; // one for old tpipe; the other for new tpipe
            if (module_idx == WPE_MODULE_IDX_WPE_A)
            dup_tpipe_buffer_size = WPE_MODULE_IDX_MAX * dup_tpipe_buffer_size;

            this->gWPETpipeBufInfo.size = MAX_WPE_BURST_IDX_NUM * MAX_WPE_DUP_IDX_NUM * dup_tpipe_buffer_size * MAX_ISP_TILE_TDR_HEX_NO_WPE +(15+1);
            this->gWPETpipeBufInfo.useNoncache = 1; //alloc non-cacheable mem for tpipe.
            if ( m_pIMemDrv->allocVirtBuf(&gWPETpipeBufInfo) ) {
                LOG_ERR("[ERROR]allocVirtBuf(mdpBufInfo) fail,size(0x%x)",this->gWPETpipeBufInfo.size);
                ret = MFALSE;
                goto EXIT;
            }
            if ( m_pIMemDrv->mapPhyAddr(&gWPETpipeBufInfo) ) {
                LOG_ERR("[ERROR]m_pIMemDrv->mapPhyAddr(tpipeBufInfo)");
                ret = MFALSE;
                goto EXIT;
            }
            LOG_DBG("[TPIPE]size(0x%x),Va(0x%x),Pa(0x%x)",this->gWPETpipeBufInfo.size,this->gWPETpipeBufInfo.virtAddr,this->gWPETpipeBufInfo.phyAddr);

            /*============================================
                         control MDP buffer & obj
               =============================================*/
            //control mdp object
            m_pMdpMgr = MdpMgr::createInstance();
            if(m_pMdpMgr == NULL){
                LOG_ERR("[Error]fail to create MDP instance");
                ret = MFALSE;
                goto EXIT;
            }
            if(MDPMGR_NO_ERROR != m_pMdpMgr->init(1)){
                LOG_ERR("[Error]m_pMdpMgr->init fail");
                ret = MFALSE;
                goto EXIT;
            }

#if 1
            // mdp buffer
            MDPMGR_CFG_STRUCT **pDataLayer1;
            MDPMGR_CFG_STRUCT *pDataLayer2;
            //
            this->gWPEMdpBufInfo.size = (MAX_WPE_BURST_IDX_NUM * sizeof(int**)+ \
                                    MAX_WPE_BURST_IDX_NUM * MAX_WPE_DUP_IDX_NUM * sizeof(int*) + \
                                    MAX_WPE_BURST_IDX_NUM * MAX_WPE_DUP_IDX_NUM * dup_cmds * sizeof(MDPMGR_CFG_STRUCT))+(15+1);
            this->gWPEMdpBufInfo.useNoncache = 1; //alloc non-cacheable mem for tpipe.
            if ( m_pIMemDrv->allocVirtBuf(&this->gWPEMdpBufInfo) ) {
                LOG_ERR("[ERROR]allocVirtBuf(gWPEMdpBufInfo) fail,size(0x%x)",this->gWPEMdpBufInfo.size);
            }

            LOG_INF("[MDP]size(0x%x),Va(0x%x)",this->gWPEMdpBufInfo.size,this->gWPEMdpBufInfo.virtAddr);

            this->gpWPEMdpMgrCfgData = (MDPMGR_CFG_STRUCT ***)((this->gWPEMdpBufInfo.virtAddr+15)&(~15));

            pDataLayer1 = (MDPMGR_CFG_STRUCT**)(this->gpWPEMdpMgrCfgData + MAX_WPE_BURST_IDX_NUM);
            pDataLayer2 = (MDPMGR_CFG_STRUCT*)(this->gpWPEMdpMgrCfgData + MAX_WPE_BURST_IDX_NUM + MAX_WPE_BURST_IDX_NUM * MAX_WPE_DUP_IDX_NUM);


            for(i=0;i<MAX_WPE_BURST_IDX_NUM;i++){
                this->gpWPEMdpMgrCfgData[i] = pDataLayer1;
                for(j=0;j<MAX_WPE_DUP_IDX_NUM;j++){
                    this->gpWPEMdpMgrCfgData[i][j] = pDataLayer2;
                    pDataLayer2 += (dup_cmds);
                }
                pDataLayer1 += (MAX_WPE_DUP_IDX_NUM);
            }
#endif
#if 0
            for(i=0;i<ISP_WPE_MODULE_IDX_MAX;i++)
                for(j=0;j<MAX_WPE_BURST_QUE_NUM;j++)
                    for(k=0;k<MAX_WPE_DUP_CQ_NUM;k++)
                        this->gpWPEMdpMgrCfgData[i][j][k] = this->gMDPCfgData[i][j][k];
#endif

            // set tpipe address to mdp buffer
            offset = 0;
            for(i=0;i<MAX_WPE_BURST_IDX_NUM;i++){
            for(j=0;j<MAX_WPE_DUP_IDX_NUM;j++){
                for(k=0;k<dup_cmds;k++){
#if 0
                        gpWPEMdpMgrCfgData[i][j][k].ispTpipeCfgInfo.drvinfo.tpipeTableVa = (unsigned int*)&gpTpipeBufVa[i][j][k];
                        gpWPEMdpMgrCfgData[i][j][k].ispTpipeCfgInfo.drvinfo.tpipeTablePa = (unsigned long)&gpTpipeBufPa[i][j][k];
#else
                        this->gpWPEMdpMgrCfgData[i][j][k].ispTpipeCfgInfo.drvinfo.tpipeTableVa_wpe = (unsigned int*)(this->gWPETpipeBufInfo.virtAddr + offset);
                        this->gpWPEMdpMgrCfgData[i][j][k].ispTpipeCfgInfo.drvinfo.tpipeTablePa_wpe = this->gWPETpipeBufInfo.phyAddr + offset;
                        //this->gpWPEMdpMgrCfgData[i][j][k].ispTpipeCfgInfo.drvinfo.tpipeTableVa_wpeb = (unsigned int*)(this->gWPETpipeBufInfo.virtAddr + offset + offset);
                        //this->gpWPEMdpMgrCfgData[i][j][k].ispTpipeCfgInfo.drvinfo.tpipeTablePa_wpeb = this->gWPETpipeBufInfo.phyAddr + offset + offset;
                        offset += 2 * MAX_ISP_TILE_TDR_HEX_NO_WPE;
#endif
                        LOG_DBG("i(%d),j(%d),k(%d),va(0x%x),pa(0x%x)",
                            i, j, k, this->gpWPEMdpMgrCfgData[i][j][k].ispTpipeCfgInfo.drvinfo.tpipeTableVa,this->gpWPEMdpMgrCfgData[i][j][k].ispTpipeCfgInfo.drvinfo.tpipeTablePa);
                    }
                }
            }
            LOG_DBG("UserName:%s, mInitCount:%d\n", m_UserName, mWeapEngInitCount);
            m_iWpeEnqueReq = 0;

        }
        m_WarpEngInit = MTRUE;
    }

EXIT:
    return ret;
}

MBOOL WarpEnginePipe::
uninit(ISP_WPE_MODULE_IDX module_idx)
{
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    MINT32 subRet;
    MINT32 tmp=0;

    //[1] decrease user count and record user
    tmp = android_atomic_dec(&mWeapEngInitCount);
    LOG_DBG("+,mWpeUnitCnt(%d)",mWeapEngInitCount);

    if(mWeapEngInitCount==0)
    {
        /*============================================
                 wpe command buffer
        =============================================*/

        if (MTRUE == m_WarpEngInit)
        {
            this->m_pWpeDrv->uninit(m_UserName);
            strncpy(m_UserName,"",MAX_USER_NAME_SIZE);
            LOG_INF("UserName:%s, mInitCount:%d\n", m_UserName, mWeapEngInitCount);

            this->m_WarpEngInit = MFALSE;
            this->m_iWpeEnqueReq = 0;
        }
#if 1
        if ( this->m_pIMemDrv->freeVirtBuf(&this->wCmdBufInfo) ) {
            LOG_ERR("[ERROR]freeVirtBuf(wCmdBufInfo) fail,size(0x%x)",this->gWPEMdpBufInfo.size);
        }
#endif
        this->wCmdData = NULL;
        this->wCmdBufInfo.size = 0;

        /*============================================
                 control tpipe buffer
        =============================================*/
        if ( this->m_pIMemDrv->unmapPhyAddr(&this->gWPETpipeBufInfo) ) {
            LOG_ERR("[ERROR]m_pIMemDrv->unmapPhyAddr (gWPETpipeBufInfo)");
        }

        if ( this->m_pIMemDrv->freeVirtBuf(&this->gWPETpipeBufInfo) ) {
            LOG_ERR("[ERROR]freeVirtBuf(gWPETpipeBufInfo) fail,size(0x%x)",this->gWPEMdpBufInfo.size);
        }
        //gpTpipeBufVa = NULL;
        //gpTpipeBufPa = NULL;
        this->gWPETpipeBufInfo.size = 0;

        /*============================================
                 control MDP buffer & obj
        =============================================*/

        subRet = m_pMdpMgr->uninit(1);
        if(MDPMGR_NO_ERROR != subRet) {
            LOG_ERR("[Error]m_pMdpMgr->uninit fail(%d)",subRet);
            ret = MFALSE;
        }
        m_pMdpMgr->destroyInstance();
        m_pMdpMgr = NULL;
        //
#if 1
        if ( m_pIMemDrv->freeVirtBuf(&this->gWPEMdpBufInfo) ) {
            LOG_ERR("[ERROR]freeVirtBuf(gWPEMdpBufInfo) fail,size(0x%x)",this->gWPEMdpBufInfo.size);
        }
#endif
        this->gWPEMdpBufInfo.size = 0;
        this->gpWPEMdpMgrCfgData = NULL;

        m_pIMemDrv->uninit();
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }

    return ret;
}

MINT32 WarpEnginePipe::
configwpe(WpePackage wpepackage, MBOOL configUVplane)
{
    int ret = MTRUE;
    NSCam::NSIoPipe::WPEConfig pWpeConfig;
    NSCam::NSIoPipe::ExtraParam CmdInfo;
    android::Vector<NSCam::NSIoPipe::ExtraParam>::const_iterator iter;
    NSCam::NSIoPipe::PQParam* pPQParam;

    //do buffer related configuration for each frame
    MINT32 outNum = 0;   //output dma num for each frame
    vector<NSImageio::NSIspio::PortInfo const*> vPostProcOutPorts;
    NSImageio::NSIspio::PortInfo ports[5];
    int planeNum=0, pixelInByte=0;
    MUINT32 p2PortIdx=0x0;
    MUINT32 vegn_in_width, vegn_in_height;
    MUINT32 vegn_out_width, vegn_out_height;
    MINT32 wpei_uv_h_ratio=2; // for UV plane
    MINT32 wpei_uv_v_ratio=2;
    UINT32 wpei_420_y_uv_plane=0;
    UINT32 PSP_CTL=0;
    UINT32 PSP2_CTL=0;

    NSCam::NSIoPipe::Input *portInfo_wpei;
    //NSCam::NSIoPipe::Output *portInfo_wpe_wroto;
    //NSCam::NSIoPipe::Output *portInfo_wpe_wdmao;
    //NSCam::NSIoPipe::Output *portInfo_wpe_jpego;
    //NSImageio::NSIspio::PortInfo portInfo_wpe_wpeo;
    WPECropInfo CropInfo_wpei;
    NSCam::EImageFormat eImgFmt_wpei = eImgFmt_UNKNOWN;
    //NSCam::EImageFormat eImgFmt_wroto = eImgFmt_UNKNOWN;
    //NSCam::EImageFormat eImgFmt_wdmao = eImgFmt_UNKNOWN;
    //NSCam::EImageFormat eImgFmt_jpego = eImgFmt_UNKNOWN;
    //NSCam::EImageFormat eImgFmt_wpeo = eImgFmt_UNKNOWN;

    //NSImageio::NSIspio::PortInfo portInfo_wpei_1st_plane;
    //NSImageio::NSIspio::PortInfo portInfo_wpei_2nd_plane;
    NSImageio::NSIspio::PortInfo portInfo_wpeo;
    NSImageio::NSIspio::PortInfo portInfo_wroto;
    NSImageio::NSIspio::PortInfo portInfo_wdmao;
    NSImageio::NSIspio::PortInfo portInfo_jpego;

    QParams qParam = wpepackage.rParams;
    MUINT32 m_curframeindex = wpepackage.wpeBurstIdx;

    NSCam::NSIoPipe::NSWpe::WPEQParams *wpeqpararms = NULL;

    this->m_wpeEQDupIdx = wpepackage.wpeDupIdx;
    this->m_wpeEQBurstIdx = wpepackage.wpeBurstIdx;
    this->m_wpeCallerID = wpepackage.callerID;
    this->m_wpeDupCmdIdx = wpepackage.m_wpeDupCmdIdx;

    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].drvScenario = (MUINT32)module_idx;
    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe = wpepackage.wpeDupIdx;
    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].isWaitBuf = MTRUE;
    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].imgxoEnP2 = 0;
    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpWDMAPQParam = NULL;
    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpWROTPQParam = NULL;

    for(iter = qParam.mvFrameParams[m_curframeindex].mvExtraParam.begin(); iter < qParam.mvFrameParams[m_curframeindex].mvExtraParam.end() ; iter++)
    {
        CmdInfo = (*iter);
        switch (CmdInfo.CmdIdx)
        {
            case NSCam::NSIoPipe::EPIPE_WPE_INFO_CMD:
                wpeqpararms = (NSCam::NSIoPipe::NSWpe::WPEQParams *)CmdInfo.moduleStruct;
                break;
            case NSCam::NSIoPipe::EPIPE_MDP_PQPARAM_CMD:
                pPQParam = (NSCam::NSIoPipe::PQParam*)CmdInfo.moduleStruct;
                this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpWDMAPQParam = (MVOID *)pPQParam->WDMAPQParam;
                this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpWROTPQParam = (MVOID *)pPQParam->WROTPQParam;
                break;
            case NSCam::NSIoPipe::EPIPE_FM_INFO_CMD:
            case NSCam::NSIoPipe::EPIPE_FE_INFO_CMD:
            default:
                LOG_ERR("receive extra cmd(%d)\n",CmdInfo.CmdIdx);
                break;
        }
    }

    if (wpeqpararms == NULL)
    {
        LOG_ERR("WPE EQ Parameter is NULL\n");
        return MFALSE;
    }

    outNum = qParam.mvFrameParams[m_curframeindex].mvOut.size();

    //Handle Warp Out Crop Info.

    switch(wpeqpararms->wpe_mode)
    {
         case 2:
            pWpeConfig.Wpe_ISP_CROP_EN = 1;
            {
                pWpeConfig.Wpe_ISPCROP_STR_X = wpeqpararms->wpecropinfo.x_start_point;
                pWpeConfig.Wpe_ISPCROP_END_X = wpeqpararms->wpecropinfo.x_end_point;
                pWpeConfig.Wpe_ISPCROP_STR_Y = wpeqpararms->wpecropinfo.y_start_point;
                pWpeConfig.Wpe_ISPCROP_END_Y = wpeqpararms->wpecropinfo.y_end_point;
            }
            /*
                if ((EImageFormat)(qParam.mvFrameParams[0].mvIn[0].mBuffer->getImgFormat() == eImgFmt_YUY2) || (EImageFormat)(qParam.mvFrameParams[0].mvIn[0].mBuffer->getImgFormat() == eImgFmt_YVYU) ||
                     (EImageFormat)(qParam.mvFrameParams[0].mvIn[0].mBuffer->getImgFormat() == eImgFmt_UYVY) || (EImageFormat)(qParam.mvFrameParams[0].mvIn[0].mBuffer->getImgFormat() == eImgFmt_VYUY))
                {
                    ret = MFALSE;
                }*/
        // Asiign Tile Mode
            this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.frameflag = HINT_WPE_TILE_MODE;
            LOG_DBG("WPE: Wpe_ISP_CROP_EN");
            break;
        case 1:
            pWpeConfig.Wpe_MDP_CROP_EN = 1;
            pWpeConfig.Wpe_C24_EN = 1;
            /*if ((EImageFormat)(qParam.mvFrameParams[0].mvIn[0].mBuffer->getImgFormat() == eImgFmt_YUY2) || (EImageFormat)(qParam.mvFrameParams[0].mvIn[0].mBuffer->getImgFormat() == eImgFmt_YVYU) ||
                    (EImageFormat)(qParam.mvFrameParams[0].mvIn[0].mBuffer->getImgFormat() == eImgFmt_UYVY) || (EImageFormat)(qParam.mvFrameParams[0].mvIn[0].mBuffer->getImgFormat() == eImgFmt_VYUY)
                */
            {
                pWpeConfig.Wpe_MDP_CROP_STR_X = wpeqpararms->wpecropinfo.x_start_point;
                pWpeConfig.Wpe_MDP_CROP_END_X = wpeqpararms->wpecropinfo.x_end_point;
                pWpeConfig.Wpe_MDP_CROP_STR_Y = wpeqpararms->wpecropinfo.y_start_point;
                pWpeConfig.Wpe_MDP_CROP_END_Y = wpeqpararms->wpecropinfo.y_end_point;
            }
          // Asiign Tile Mode
            this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.frameflag = HINT_WPE_TILE_MODE;
            LOG_DBG("WPE: Wpe_MDP_CROP_EN");
            break;
        case 0:
            pWpeConfig.Wpe_WPEO_EN = 1;
            if(outNum > 1)
            {
                LOG_ERR("WPEO: Wrong output Bufferer Size (%d)", outNum);
                ret = MFALSE;
            }
         // Asiign Tile Mode
            this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.frameflag = HINT_WPE_TILE_MODE;
            LOG_DBG("WPE: Wpe_WPEO_EN");
            break;
        default:
            LOG_ERR("WPE: Wrong WPE Mode");
            ret = MFALSE;
            break;
    }

    if ((pWpeConfig.Wpe_WPEO_EN == 1 && pWpeConfig.Wpe_MDP_CROP_EN == 1) || (pWpeConfig.Wpe_WPEO_EN == 1 && pWpeConfig.Wpe_ISP_CROP_EN == 1) ||
        (pWpeConfig.Wpe_ISP_CROP_EN == 1 && pWpeConfig.Wpe_MDP_CROP_EN == 1))
    {
        LOG_ERR("WPE: Wrong Out Config. setting");
       ret = MFALSE;
    }

    //Handle Warp Matrix
    if(wpeqpararms->warp_veci_info.width > 0 && wpeqpararms->warp_veci_info.height > 0 && wpeqpararms->warp_veci_info.stride > 0 && \
       wpeqpararms->warp_veci_info.width <= 640 && wpeqpararms->warp_veci_info.phyAddr != 0)
    {
        pWpeConfig.Wpe_VECI_EN = 1;
        pWpeConfig.Wpe_VECI.u4ImgWidth = wpeqpararms->warp_veci_info.width;
        pWpeConfig.Wpe_VECI.u4ImgHeight = wpeqpararms->warp_veci_info.height;
        pWpeConfig.Wpe_VECI.u4Stride = wpeqpararms->warp_veci_info.stride;
        pWpeConfig.Wpe_VECI.u4BufVA = wpeqpararms->warp_veci_info.virtAddr;
        pWpeConfig.Wpe_VECI.u4BufPA = wpeqpararms->warp_veci_info.phyAddr;
        switch(wpeqpararms->warp_veci_info.bus_size)
        {
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS:
                pWpeConfig.Wpe_VECI.u4ElementSzie = 4; // 32-bits
                break;
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_8_BITS:
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_16_BITS:
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_24_BITS:
            default:
                LOG_ERR("WPE: VECI Input ElementSzie Error.");
                ret = MFALSE;
                break;
        }

        vegn_in_width  = wpeqpararms->warp_veci_info.width;
        vegn_in_height = wpeqpararms->warp_veci_info.height;
    }
    else
    {
        LOG_ERR("WPE: VECI Input error.warp_veci_info.width (%d), warp_veci_info.height (%d), warp_veci_info.stride(%d),wpeqpararms->warp_veci_info.phyAddr (0x%x)  ",
              wpeqpararms->warp_veci_info.width, wpeqpararms->warp_veci_info.height, wpeqpararms->warp_veci_info.stride, wpeqpararms->warp_veci_info.phyAddr);
        ret = MFALSE;
    }

    if(wpeqpararms->warp_vec2i_info.width > 0 && wpeqpararms->warp_vec2i_info.height > 0 && wpeqpararms->warp_vec2i_info.stride > 0 && \
       wpeqpararms->warp_vec2i_info.width <= 640 && wpeqpararms->warp_vec2i_info.phyAddr != 0)
    {
        pWpeConfig.Wpe_VEC2I_EN = 1;
        pWpeConfig.Wpe_VEC2I.u4ImgWidth = wpeqpararms->warp_vec2i_info.width;
        pWpeConfig.Wpe_VEC2I.u4ImgHeight = wpeqpararms->warp_vec2i_info.height;
        pWpeConfig.Wpe_VEC2I.u4Stride = wpeqpararms->warp_vec2i_info.stride;
        switch(wpeqpararms->warp_vec2i_info.bus_size)
        {
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS:
                pWpeConfig.Wpe_VEC2I.u4ElementSzie = 4; // 32-bits
                break;
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_8_BITS:
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_16_BITS:
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_24_BITS:
            default:
                LOG_ERR("WPE: VEC2I Input ElementSzie Error.");
                ret = MFALSE;
            break;
        }
        pWpeConfig.Wpe_VEC2I.u4BufVA = wpeqpararms->warp_vec2i_info.virtAddr;
        pWpeConfig.Wpe_VEC2I.u4BufPA = wpeqpararms->warp_vec2i_info.phyAddr;
    }
    else
    {
        LOG_ERR("WPE: VEC2I Input error.");
        ret = MFALSE;
    }

    if(pWpeConfig.Wpe_VECI.u4ImgWidth != pWpeConfig.Wpe_VEC2I.u4ImgWidth || pWpeConfig.Wpe_VECI.u4ImgHeight != pWpeConfig.Wpe_VEC2I.u4ImgHeight)
    {
        LOG_ERR("WPE: Width of Height of VECI and VEC2I are not equal");
        ret = MFALSE;
    }

    //Handle VECI3
    pWpeConfig.Wpe_VGEN_HMG_MODE = wpeqpararms->vgen_hmg_mode;

    if(wpeqpararms->warp_vec3i_info.width > 0 && wpeqpararms->warp_vec3i_info.height > 0 && wpeqpararms->warp_vec3i_info.stride > 0 && \
       wpeqpararms->warp_vec3i_info.width <= 640  && wpeqpararms->warp_vec3i_info.phyAddr != 0 && wpeqpararms->vgen_hmg_mode == 1)
    {
        pWpeConfig.Wpe_VEC3I_EN = 1;

        pWpeConfig.Wpe_VEC3I.u4ImgWidth = wpeqpararms->warp_vec3i_info.width;
        pWpeConfig.Wpe_VEC3I.u4ImgHeight = wpeqpararms->warp_vec3i_info.height;
        pWpeConfig.Wpe_VEC3I.u4Stride = wpeqpararms->warp_vec3i_info.stride;
        pWpeConfig.Wpe_VEC3I.u4BufVA = wpeqpararms->warp_vec3i_info.virtAddr;
        pWpeConfig.Wpe_VEC3I.u4BufPA = wpeqpararms->warp_vec3i_info.phyAddr;
        switch(wpeqpararms->warp_vec3i_info.bus_size)
        {
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS:
                pWpeConfig.Wpe_VEC3I.u4ElementSzie = 4; // 32-bits
                break;
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_8_BITS:
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_16_BITS:
            case NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_24_BITS:
            default:
                LOG_ERR("WPE: VEC3I Input ElementSzie Error.");
                ret = MFALSE;
            break;
        }
    }
    else
    {
        pWpeConfig.Wpe_VEC3I_EN = 0;
    }

    if (pWpeConfig.Wpe_VECI_EN !=  1 || pWpeConfig.Wpe_VEC2I_EN != 1)
    {
        LOG_ERR("WPE: VEC3I Input error.");
        ret = MFALSE;
    }

    if(wpeqpararms->mwVgenCropInfo.size() < 1)
    {
        LOG_ERR("WPE: VgenCropInfo error.");
        ret = MFALSE;
    }

    vegn_out_width = (wpeqpararms->wpecropinfo.x_end_point + 1) - wpeqpararms->wpecropinfo.x_start_point;
    vegn_out_height = (wpeqpararms->wpecropinfo.y_end_point + 1) - wpeqpararms->wpecropinfo.y_start_point;

    pWpeConfig.Wpe_VGEN_HORI_INT_OFST  = wpeqpararms->mwVgenCropInfo[0].mCropRect.p_integral.x;
    pWpeConfig.Wpe_VGEN_HORI_SUB_OFST = wpeqpararms->mwVgenCropInfo[0].mCropRect.p_fractional.x;
    pWpeConfig.Wpe_VGEN_VERT_INT_OFST  = wpeqpararms->mwVgenCropInfo[0].mCropRect.p_integral.y;
    pWpeConfig.Wpe_VGEN_VERT_SUB_OFST = wpeqpararms->mwVgenCropInfo[0].mCropRect.p_fractional.y;

    pWpeConfig.Wpe_PSP_TBL_SEL_H = pWpeConfig.Wpe_PSP2_TBL_SEL_H = wpeqpararms->tbl_sel_h;
    pWpeConfig.Wpe_PSP_TBL_SEL_V = pWpeConfig.Wpe_PSP2_TBL_SEL_V = wpeqpararms->tbl_sel_v;

    //out dma for each frame unit
    vPostProcOutPorts.resize(outNum);

    // initialize
    for(MINT32 i = 0; i < ISP_OUTPORT_NUM; i++){
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].dstPortCfg[i] = 0;
    }

    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].isSecureFra = qParam.mvFrameParams[m_curframeindex].mSecureFra;
    //input port
    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].srcSecureTag = qParam.mvFrameParams[m_curframeindex].mvIn[0].mSecureTag;
    planeNum = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getPlaneCount();

    CropInfo_wpei.crop1.x = 0;
    CropInfo_wpei.crop1.floatX = 0;
    CropInfo_wpei.crop1.y = 0;
    CropInfo_wpei.crop1.floatY = 0;
    CropInfo_wpei.crop1.w = 0;
    CropInfo_wpei.crop1.h = 0;
    CropInfo_wpei.resize1.tar_w = 0;
    CropInfo_wpei.resize1.tar_h = 0;
    CropInfo_wpei.crop2.x = 0;
    CropInfo_wpei.crop2.floatX = 0;
    CropInfo_wpei.crop2.y = 0;
    CropInfo_wpei.crop2.floatY = 0;
    CropInfo_wpei.crop2.w = 0;
    CropInfo_wpei.crop2.h = 0;
    CropInfo_wpei.resize2.tar_w = 0;
    CropInfo_wpei.resize2.tar_h = 0;
    CropInfo_wpei.crop3.x = 0;
    CropInfo_wpei.crop3.floatX = 0;
    CropInfo_wpei.crop3.y = 0;
    CropInfo_wpei.crop3.floatY = 0;
    CropInfo_wpei.crop3.w = 0;
    CropInfo_wpei.crop3.h = 0;
    CropInfo_wpei.resize3.tar_w = 0;
    CropInfo_wpei.resize3.tar_h = 0;

    for(int k=0;k<qParam.mvFrameParams[m_curframeindex].mvCropRsInfo.size();k++)
    {
        switch(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mGroupID)
        {
            case 1: // for CRZ
                CropInfo_wpei.crop1.x       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_integral.x;
                CropInfo_wpei.crop1.floatX  = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_fractional.x;
                CropInfo_wpei.crop1.y       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_integral.y;
                CropInfo_wpei.crop1.floatY  = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_fractional.y;
                CropInfo_wpei.crop1.w       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.w;
                CropInfo_wpei.crop1.h       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.h;
                CropInfo_wpei.resize1.tar_w = (qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.w == 0)?(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.w):(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.w);
                CropInfo_wpei.resize1.tar_h = (qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.h == 0)?(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.h):(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.h);
                break;
            case 2:// for WDMAO
                CropInfo_wpei.crop2.x       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_integral.x;
                CropInfo_wpei.crop2.floatX  = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_fractional.x;
                CropInfo_wpei.crop2.y       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_integral.y;
                CropInfo_wpei.crop2.floatY  = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_fractional.y;
                CropInfo_wpei.crop2.w       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.w;
                CropInfo_wpei.crop2.h       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.h;
                CropInfo_wpei.resize2.tar_w = (qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.w == 0)?(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.w):(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.w);
                CropInfo_wpei.resize2.tar_h = (qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.h == 0)?(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.h):(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.h);
                break;
            case 3:// for WROTO
                CropInfo_wpei.crop3.x       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_integral.x;
                CropInfo_wpei.crop3.floatX  = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_fractional.x;
                CropInfo_wpei.crop3.y       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_integral.y;
                CropInfo_wpei.crop3.floatY  = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.p_fractional.y;
                CropInfo_wpei.crop3.w       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.w;
                CropInfo_wpei.crop3.h       = qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.h;
                CropInfo_wpei.resize3.tar_w = (qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.w == 0)?(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.w):(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.w);
                CropInfo_wpei.resize3.tar_h = (qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.h == 0)?(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mCropRect.s.h):(qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mResizeDst.h);
                break;
            default:
                LOG_ERR("DO not support crop group (%d)",qParam.mvFrameParams[m_curframeindex].mvCropRsInfo[k].mGroupID);
                break;
            }
    }

    if(CropInfo_wpei.crop1.w > vegn_out_width || CropInfo_wpei.crop2.w > vegn_out_width || CropInfo_wpei.crop3.w > vegn_out_width ||
       CropInfo_wpei.crop1.h > vegn_out_height || CropInfo_wpei.crop2.h > vegn_out_height || CropInfo_wpei.crop3.h > vegn_out_height)
    {
        LOG_ERR("WPE: Out Crop size is larger than WPE out, portInfo_wpei_1st_plane.crop1.w:%d, portInfo_wpei_1st_plane.crop1.h:%d, vegn_out_width:%d, vegn_out_height:%d", CropInfo_wpei.crop1.w, CropInfo_wpei.crop1.h, vegn_out_width, vegn_out_height);
        LOG_ERR("WPE: Out Crop size is larger than WPE out, portInfo_wpei_1st_plane.crop2.w:%d, portInfo_wpei_1st_plane.crop2.h:%d, portInfo_wpei_1st_plane.crop3.w:%d, portInfo_wpei_1st_plane.crop2.h:%d", CropInfo_wpei.crop2.w, CropInfo_wpei.crop2.h, CropInfo_wpei.crop3.w, CropInfo_wpei.crop3.h);
        ret = MFALSE;
    }

    //input dma inforamtion
    portInfo_wpei =  (NSCam::NSIoPipe::Input *)&qParam.mvFrameParams[m_curframeindex].mvIn[0];
    eImgFmt_wpei  = (EImageFormat)(qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getImgFormat());
    //portInfo_wpei_1st_plane.u4ImgWidth  = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getImgSize().w;
    //portInfo_wpei_1st_plane.u4ImgHeight = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getImgSize().h;
    //portInfo_wpei_1st_plane.eImgFmt      = (EImageFormat)(qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getImgFormat());
    //portInfo_wpei_1st_plane.type     = NSImageio::NSIspio::EPortType_Memory;//mPortTypeMapping[qParam.mvIn[0].mPortID.type].eImgIOPortType;
    //portInfo_wpei_1st_plane.index    = qParam.mvFrameParams[m_curframeindex].mvIn[0].mPortID.index; //need map to index defined in imageio
    //portInfo_wpei_1st_plane.inout    = qParam.mvFrameParams[m_curframeindex].mvIn[0].mPortID.inout;
    //portInfo_wpei_1st_plane.capbility   = qParam.mvFrameParams[m_curframeindex].mvIn[0].mPortID.capbility;
    //portInfo_wpei_1st_plane.eImgRot      = mMDPTransformMapping[qParam.mvFrameParams[m_curframeindex].mvIn[0].mTransform].eImgRot;
    //portInfo_wpei_1st_plane.eImgFlip    = mMDPTransformMapping[qParam.mvFrameParams[m_curframeindex].mvIn[0].mTransform].eImgFlip;
    for(int k=0;k<planeNum;k++)
    {
        pWpeConfig.Wpe_CACHI_AllFormats.u4BufSize[k] = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufSizeInBytes(k);
        pWpeConfig.Wpe_CACHI_AllFormats.u4Stride[k] = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufStridesInBytes(k);
        pWpeConfig.Wpe_CACHI_AllFormats.u4BufVA[k] = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufVA(k);
        pWpeConfig.Wpe_CACHI_AllFormats.u4BufPA[k] = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufPA(k);
        //portInfo_wpei_1st_plane[cnt].memID[k]	 = qParam.mvFrameParams[m_curframeindex].mvIn[i].mBuffer->getFD(k);
    }

    pWpeConfig.Wpe_CACHI_AllFormats.planeNum = planeNum;
    pWpeConfig.Wpe_CACHI_AllFormats.u4imgFmt = eImgFmt_wpei;
    pWpeConfig.Wpe_CACHI_AllFormats.u4ImgWidth = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getImgSize().w;//portInfo_wpei_1st_plane.u4ImgWidth;
    pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getImgSize().h;//portInfo_wpei_1st_plane.u4ImgHeight;
    pWpeConfig.Wpe_CACHI_AllFormats.secureTag = qParam.mvFrameParams[m_curframeindex].mvIn[0].mSecureTag;
    pWpeConfig.Wpe_VGEN_VEC_WD = pWpeConfig.Wpe_CACHI_AllFormats.u4ImgWidth;//portInfo_wpei_1st_plane.u4ImgWidth;
    pWpeConfig.Wpe_VGEN_VEC_HT = pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight;//portInfo_wpei_1st_plane.u4ImgHeight;


    if(eImgFmt_wpei == eImgFmt_NV12 || eImgFmt_wpei == eImgFmt_NV21)
    {
        if(planeNum == 2)  // check plane number
        {
            //portInfo_wpei_2nd_plane = portInfo_wpei_1st_plane;

            LOG_DBG("WPE configUVplane : pWpeConfig.Wpe_CACHI_AllFormats.u4ImgWidth  (%d), pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight (%d) ",pWpeConfig.Wpe_CACHI_AllFormats.u4ImgWidth , pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight);

            if(configUVplane == MTRUE)
            {
                wpei_420_y_uv_plane = 3;  // for 420 UV plane
                PSP_CTL = 1;
                PSP2_CTL = 1;
                //pWpeConfig.Wpe_CACHI_AllFormats.u4ImgWidth = portInfo_wpei_2nd_plane.u4ImgWidth;
                //pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight = portInfo_wpei_2nd_plane.u4ImgHeight;
                pWpeConfig.Wpe_VGEN_VEC_WD = pWpeConfig.Wpe_CACHI_AllFormats.u4ImgWidth / wpei_uv_h_ratio;//portInfo_wpei_2nd_plane.u4ImgWidth / wpei_uv_h_ratio; // UV plane  W_UV  =  W_Y/2
                pWpeConfig.Wpe_VGEN_VEC_HT = pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight/ wpei_uv_v_ratio;//portInfo_wpei_2nd_plane.u4ImgHeight / wpei_uv_v_ratio;
                pWpeConfig.Wpe_UV_INTER_EN = 1;

                LOG_DBG("WPE configUVplane : pWpeConfig.Wpe_VGEN_VEC_WD(%d), pWpeConfig.Wpe_VGEN_VEC_HT(%d) ",pWpeConfig.Wpe_VGEN_VEC_WD , pWpeConfig.Wpe_VGEN_VEC_HT);
                pWpeConfig.Wpe_VGEN_POST_SUB_H = 1;
                pWpeConfig.Wpe_VGEN_POST_SUB_V = 1;
            }
            else
            {
                if(pWpeConfig.Wpe_MDP_CROP_EN == 1)
                    pWpeConfig.Wpe_SYNC_EN = 1; //for 420 direct link Y palne, sync should enable
            }
        }
        else
        {
            LOG_ERR("NOT SUPPORT 420 planeNum(%d)",planeNum);
            ret = MFALSE;
        }
    }

    switch (eImgFmt_wpei)
    {
        case eImgFmt_YUY2:            //< 422 format, 1 plane (YVYU)
            pWpeConfig.Wpe_CACHI_FMT = 5;
            pWpeConfig.Wpe_CACHI_FMT_SEQ = 0;
            pWpeConfig.Wpe_PSP_UNSIGN_TO_SIGN = 0;
            pWpeConfig.Wpe_PSP_SIGN_EXT = 0;
            pWpeConfig.Wpe_PSP2_UNSIGN_TO_SIGN = 1;
            pWpeConfig.Wpe_PSP2_SIGN_EXT = 1;
            break;
        case eImgFmt_YVYU:
            pWpeConfig.Wpe_CACHI_FMT = 5;
            pWpeConfig.Wpe_CACHI_FMT_SEQ = 1;
            pWpeConfig.Wpe_PSP_UNSIGN_TO_SIGN = 0;
            pWpeConfig.Wpe_PSP_SIGN_EXT = 0;
            pWpeConfig.Wpe_PSP2_UNSIGN_TO_SIGN = 1;
            pWpeConfig.Wpe_PSP2_SIGN_EXT = 1;
            break;
        case eImgFmt_UYVY:
            pWpeConfig.Wpe_CACHI_FMT = 5;
            pWpeConfig.Wpe_CACHI_FMT_SEQ = 2;
            pWpeConfig.Wpe_PSP_UNSIGN_TO_SIGN = 0;
            pWpeConfig.Wpe_PSP_SIGN_EXT = 0;
            pWpeConfig.Wpe_PSP2_UNSIGN_TO_SIGN = 1;
            pWpeConfig.Wpe_PSP2_SIGN_EXT = 1;
            break;
        case eImgFmt_VYUY:
            pWpeConfig.Wpe_CACHI_FMT = 5;
            pWpeConfig.Wpe_CACHI_FMT_SEQ = 3;
            pWpeConfig.Wpe_PSP_UNSIGN_TO_SIGN = 0;
            pWpeConfig.Wpe_PSP_SIGN_EXT = 0;
            pWpeConfig.Wpe_PSP2_UNSIGN_TO_SIGN = 1;
            pWpeConfig.Wpe_PSP2_SIGN_EXT = 1;
            break;
        case eImgFmt_NV12:
            pWpeConfig.Wpe_CACHI_FMT = 0 + wpei_420_y_uv_plane;
            pWpeConfig.Wpe_CACHI_FMT_SEQ = 0;
            pWpeConfig.Wpe_PSP_UNSIGN_TO_SIGN = 0 + PSP_CTL;
            pWpeConfig.Wpe_PSP_SIGN_EXT = 0 + PSP_CTL;
            pWpeConfig.Wpe_PSP2_UNSIGN_TO_SIGN = 0 + PSP2_CTL;
            pWpeConfig.Wpe_PSP2_SIGN_EXT = 0 + PSP2_CTL;
            break;
        case eImgFmt_NV21:
            pWpeConfig.Wpe_CACHI_FMT = 0 + wpei_420_y_uv_plane;
            pWpeConfig.Wpe_CACHI_FMT_SEQ = 1;
            pWpeConfig.Wpe_PSP_UNSIGN_TO_SIGN = 0 + PSP_CTL;
            pWpeConfig.Wpe_PSP_SIGN_EXT = 0 + PSP_CTL;
            pWpeConfig.Wpe_PSP2_UNSIGN_TO_SIGN = 0 + PSP2_CTL;
            pWpeConfig.Wpe_PSP2_SIGN_EXT = 0 + PSP2_CTL;
            break;
        default:
            LOG_ERR("NOT SUPPORT imgFmt(%d)",pWpeConfig.Wpe_CACHI_AllFormats.u4imgFmt);
            ret = MFALSE;
            break;
    }

    LOG_INF("WPE:CmdIdx (%d), CACHI_FMT:(%d), FMT_SEQ:(%d), Security(%d), EQBurstIdx :(%d), EQDupIdx:(%d), EQDupCmdIdx(%d), WPE Mode:(%d), WARP_Engine_ID(%d), TBL_SEL_H(%d), TBL_SEL_V(%d)\n\
            [WPE mvIn (%d)] fmt(0x%x),w/h(%d/%d),oft(%d),stride(%d/0/0), Security(%d),cap(%d),mem(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0)/Size(0x%08x,0x0,0x0),pixelInByte(%d)), \n\
            [crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)", \
            qParam.mvFrameParams[m_curframeindex].mvExtraParam[0].CmdIdx, pWpeConfig.Wpe_CACHI_FMT, pWpeConfig.Wpe_CACHI_FMT_SEQ,  qParam.mvFrameParams[m_curframeindex].mSecureFra, this->m_wpeEQBurstIdx, this->m_wpeEQDupIdx, this->m_wpeDupCmdIdx, wpeqpararms->wpe_mode, this->module_idx, \
            wpeqpararms->tbl_sel_h, wpeqpararms->tbl_sel_v, \
            qParam.mvFrameParams[m_curframeindex].mvIn[0].mPortID.index,eImgFmt_wpei,pWpeConfig.Wpe_CACHI_AllFormats.u4ImgWidth,pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight,qParam.mvFrameParams[m_curframeindex].mvIn[0].mOffsetInBytes,\
            qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),qParam.mvFrameParams[m_curframeindex].mvIn[0].mSecureTag,\
            qParam.mvFrameParams[m_curframeindex].mvIn[0].mPortID.capbility, qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufVA(0), qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufPA(0),\
            qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufSizeInBytes(0), pixelInByte, \
            CropInfo_wpei.crop1.x,CropInfo_wpei.crop1.floatX,CropInfo_wpei.crop1.y,CropInfo_wpei.crop1.floatY, \
            CropInfo_wpei.crop1.w,CropInfo_wpei.crop1.h,CropInfo_wpei.resize1.tar_w,CropInfo_wpei.resize1.tar_h, \
            CropInfo_wpei.crop2.x,CropInfo_wpei.crop2.floatX,CropInfo_wpei.crop2.y,CropInfo_wpei.crop2.floatY, \
            CropInfo_wpei.crop2.w,CropInfo_wpei.crop2.h,CropInfo_wpei.resize2.tar_w,CropInfo_wpei.resize2.tar_h,
            CropInfo_wpei.crop3.x,CropInfo_wpei.crop3.floatX,CropInfo_wpei.crop3.y,CropInfo_wpei.crop3.floatY, \
            CropInfo_wpei.crop3.w,CropInfo_wpei.crop3.h,CropInfo_wpei.resize3.tar_w,CropInfo_wpei.resize3.tar_h);


    if(this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.frameflag != HINT_WPE_FRAME_MODE)
    {
        pWpeConfig.Wpe_TDR_EN = 1;

        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].srcVirAddr = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufVA(0);
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].srcPhyAddr = qParam.mvFrameParams[m_curframeindex].mvIn[0].mBuffer->getBufPA(0);
        if(pWpeConfig.Wpe_MDP_CROP_EN == 1)
        {
            this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcW= vegn_out_width;
            this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcH = vegn_out_height;
        }
        else
        {
            this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcW= pWpeConfig.Wpe_CACHI_AllFormats.u4ImgWidth;
            this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcH = pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight;
        }

        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcFmt = eImgFmt_wpei;

        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcYStride = pWpeConfig.Wpe_CACHI_AllFormats.u4Stride[NSImageio::NSIspio::ESTRIDE_1ST_PLANE];
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcUVStride = 0;
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcBufSize = pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight * pWpeConfig.Wpe_CACHI_AllFormats.u4Stride[NSImageio::NSIspio::ESTRIDE_1ST_PLANE];
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcCBufSize = pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight * pWpeConfig.Wpe_CACHI_AllFormats.u4Stride[NSImageio::NSIspio::ESTRIDE_2ND_PLANE];
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcVBufSize = pWpeConfig.Wpe_CACHI_AllFormats.u4ImgHeight * pWpeConfig.Wpe_CACHI_AllFormats.u4Stride[NSImageio::NSIspio::ESTRIDE_3RD_PLANE];
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].mdpSrcPlaneNum = planeNum;

        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe = wpepackage.wpeDupIdx;
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].isWaitBuf = MTRUE;
        this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].imgxoEnP2 = 0;
    }

    for(int cnt=0; cnt < qParam.mvFrameParams[m_curframeindex].mvOut.size();cnt++)
    {
        ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]= 0;
        //p2PortIdx=(mPortIdxMappingAlltoWPE[qParam.mvOut[i].mPortID.index].eP2PortIdx);

        planeNum = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getPlaneCount();

        ports[cnt].eImgFmt     = (EImageFormat)(qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getImgFormat());
        ports[cnt].type        = NSImageio::NSIspio::EPortType_Memory;//mPortTypeMapping[qParam.mvOut[i].mPortID.type].eImgIOPortType;
        ports[cnt].index       = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mPortID.index; //need map to index defined in imageio
        ports[cnt].inout       = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mPortID.inout;
        ports[cnt].capbility   = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mPortID.capbility;
        ports[cnt].eImgRot     = mMDPTransformMapping[qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mTransform].eImgRot;
        ports[cnt].eImgFlip    = mMDPTransformMapping[qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mTransform].eImgFlip;
        ports[cnt].u4ImgWidth  = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getImgSize().w;
        ports[cnt].u4ImgHeight = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getImgSize().h;
        for(int k=0;k<planeNum;k++)
        {
            ports[cnt].u4Stride[k] = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getBufStridesInBytes(k);
            ports[cnt].u4BufSize[k]= qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getBufSizeInBytes(k);
            ports[cnt].u4BufVA[k]  = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getBufVA(k);
            ports[cnt].u4BufPA[k]  = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getBufPA(k);
            //ports[cnt].memID[k]    = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mBuffer->getFD(k);
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
        ports[cnt].xoffset = (qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mOffsetInBytes % ports[cnt].u4Stride[0])/pixelInByte;//dma x-offset for tile calculation
        ports[cnt].yoffset = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mOffsetInBytes / ports[cnt].u4Stride[0];//dma y-offset for tile calculation
        LOG_INF("[WPE mvOut (%d)] fmt(0x%x), Security(%d), w/h(%d/%d),oft(%d/%d/%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                ports[cnt].index,ports[cnt].eImgFmt,qParam.mvFrameParams[0].mvOut[cnt].mSecureTag, ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mOffsetInBytes,\
                ports[cnt].xoffset,ports[cnt].yoffset,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);

        vPostProcOutPorts.at(cnt)=&(ports[cnt]);
        MdpRotDMACfg mdp_wdmao;
        MdpRotDMACfg mdp_wroto;

        switch(qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mPortID.index)
        {
            case NSImageio::NSIspio::EPortIndex_WDMAO:
                portInfo_wdmao = ports[cnt];
                this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].dstPortCfg[ISP_MDP_DL_WDMAO] = 1;
                mdp_wdmao.enSrcCrop=true;
                mdp_wdmao.srcCropX = CropInfo_wpei.crop2.x;
                mdp_wdmao.srcCropFloatX = \
                  ((CropInfo_wpei.crop2.floatX>>(WPE_CROP_FLOAT_PECISE_BIT-WPE_CROP_TPIPE_PECISE_BIT_20)) & ((1<<WPE_CROP_TPIPE_PECISE_BIT_20)-1));// tpipe calculate via 0~19 bits 
                mdp_wdmao.srcCropY= CropInfo_wpei.crop2.y;
                mdp_wdmao.srcCropFloatY =  \
                    ((CropInfo_wpei.crop2.floatY>>(WPE_CROP_FLOAT_PECISE_BIT-WPE_CROP_TPIPE_PECISE_BIT_20)) & ((1<<WPE_CROP_TPIPE_PECISE_BIT_20)-1));// tpipe calculate via 0~19 bits 
                mdp_wdmao.srcCropW = CropInfo_wpei.crop2.w;
                mdp_wdmao.srcCropH = CropInfo_wpei.crop2.h;

                this->wpeconfigMdpOutPort(vPostProcOutPorts[cnt], mdp_wdmao, qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mSecureTag);

                memcpy(&this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].dstDma[ISP_MDP_DL_WDMAO], &mdp_wdmao, sizeof(MdpRotDMACfg));
                break;
            case NSImageio::NSIspio::EPortIndex_WROTO:
                portInfo_wdmao = ports[cnt];
                this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].dstPortCfg[ISP_MDP_DL_WROTO] = 1;
                mdp_wroto.enSrcCrop=true;
                mdp_wroto.srcCropX = CropInfo_wpei.crop3.x;
                mdp_wroto.srcCropFloatX = \
                  ((CropInfo_wpei.crop1.floatX>>(WPE_CROP_FLOAT_PECISE_BIT-WPE_CROP_TPIPE_PECISE_BIT_20)) & ((1<<WPE_CROP_TPIPE_PECISE_BIT_20)-1));// tpipe calculate via 0~19 bits 
                mdp_wroto.srcCropY= CropInfo_wpei.crop3.y;
                mdp_wroto.srcCropFloatY =  \
                    ((CropInfo_wpei.crop3.floatY>>(WPE_CROP_FLOAT_PECISE_BIT-WPE_CROP_TPIPE_PECISE_BIT_20)) & ((1<<WPE_CROP_TPIPE_PECISE_BIT_20)-1));//tpipe calculate via 0~19 bits 
                mdp_wroto.srcCropW = CropInfo_wpei.crop3.w;
                mdp_wroto.srcCropH = CropInfo_wpei.crop3.h;

                this->wpeconfigMdpOutPort(vPostProcOutPorts[cnt], mdp_wroto, qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mSecureTag);
                memcpy(&this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].dstDma[ISP_MDP_DL_WROTO], &mdp_wroto, sizeof(MdpRotDMACfg));
                break;
            case NSImageio::NSIspio::EPortIndex_JPEGO:
                portInfo_jpego = ports[cnt];
                this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].dstPortCfg[ISP_MDP_DL_JPEGO] = 1;
                break;
            case NSImageio::NSIspio::EPortIndex_WPEO:
                portInfo_wpeo = ports[cnt];
                this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].dstPortCfg[ISP_ONLY_OUT_TPIPE] = 1;

                for(int k=0;k<planeNum;k++)
                {
                    pWpeConfig.Wpe_WPEO_AllFormats.u4BufSize[k] = ports[cnt].u4BufSize[k];
                    pWpeConfig.Wpe_WPEO_AllFormats.u4Stride[k] = ports[cnt].u4Stride[k];
                    pWpeConfig.Wpe_WPEO_AllFormats.u4BufVA[k] = ports[cnt].u4BufVA[k];
                    pWpeConfig.Wpe_WPEO_AllFormats.u4BufPA[k] = ports[cnt].u4BufPA[k];
                    //portInfo_wpei_1st_plane[cnt].memID[k]  = qParam.mvFrameParams[0].mvIn[i].mBuffer->getFD(k);
                }

                pWpeConfig.Wpe_WPEO_AllFormats.u4ImgWidth = ports[cnt].u4ImgWidth;
                pWpeConfig.Wpe_WPEO_AllFormats.u4ImgHeight = ports[cnt].u4ImgHeight;
                pWpeConfig.Wpe_WPEO_AllFormats.secureTag = qParam.mvFrameParams[m_curframeindex].mvOut[cnt].mSecureTag;
                vegn_out_width = ports[cnt].u4ImgWidth;
                vegn_out_height = ports[cnt].u4ImgHeight;

                if(configUVplane == MTRUE)
                {
                    vegn_out_width = vegn_out_width /wpei_uv_h_ratio;
                    vegn_out_height = vegn_out_height /wpei_uv_v_ratio;

                LOG_DBG("WPE configUVplane : vegn_out_width (%d), vegn_out_height (%d) ",vegn_out_width, vegn_out_height);
                }
                break;
        }
    }

    pWpeConfig.Wpe_VGEN_IN_WD = vegn_in_width;
    pWpeConfig.Wpe_VGEN_IN_HT = vegn_in_height;
    pWpeConfig.Wpe_VGEN_OUT_WD = vegn_out_width;
    pWpeConfig.Wpe_VGEN_OUT_HT = vegn_out_height;

    if(vegn_out_width > 16384 || vegn_out_width <= 0 || vegn_out_height > 16384 || vegn_out_height <= 0)
    {
        LOG_ERR("WPE: CropInfo Size Error.vegn_out_width (%d), vegn_out_height (%d)! ",vegn_out_width, vegn_out_height);
        ret = MFALSE;
    }

    if(vegn_out_width > 0)
        {
            if((vegn_out_width % 2) == 1)
            {
                LOG_ERR("WPE: Output Vgen width should be even.vegn_out_width (%d), vegn_out_height (%d)! ",vegn_out_width, vegn_out_height);
                ret = MFALSE;
            }
    }

    //update floating offset for tpipe   24 bit base//
    pWpeConfig.Wpe_VGEN_HORI_INT_OFST = wpeqpararms->mwVgenCropInfo[0].mCropRect.p_integral.x;
    pWpeConfig.Wpe_VGEN_HORI_SUB_OFST = ((wpeqpararms->mwVgenCropInfo[0].mCropRect.p_fractional.x >> (WPE_CROP_FLOAT_PECISE_BIT - WPE_CROP_TPIPE_PECISE_BIT)) & ((1 << WPE_CROP_TPIPE_PECISE_BIT)-1));// tpipe calculate via 0~23 bits 

    pWpeConfig.Wpe_VGEN_VERT_INT_OFST = wpeqpararms->mwVgenCropInfo[0].mCropRect.p_integral.y;
    pWpeConfig.Wpe_VGEN_VERT_SUB_OFST = ((wpeqpararms->mwVgenCropInfo[0].mCropRect.p_fractional.y >> (WPE_CROP_FLOAT_PECISE_BIT - WPE_CROP_TPIPE_PECISE_BIT)) & ((1 << WPE_CROP_TPIPE_PECISE_BIT)-1));// tpipe calculate via 0~23 bits 

    pWpeConfig.Wpe_VGEN_HORI_STEP = ((((unsigned long long)(vegn_in_width - 1)) * 16777216)/(vegn_out_width - 1)); // 1<<24 = 17777216
    pWpeConfig.Wpe_VGEN_VERT_STEP = ((((unsigned long long)(vegn_in_height - 1)) * 16777216)/(vegn_out_height - 1));


    LOG_INF("[WPE]VGEN INT w/h(%d/%d), OUT w/h(%d/%d),  Max Vec w/h(%d/%d), Wpe_VGEN_HORI_STEP: (%d) (0x%x), Wpe_VGEN_VERT_STEP : (%d) (0x%x)\n", 
            vegn_in_width, vegn_in_height, vegn_out_width, vegn_out_height,  pWpeConfig.Wpe_VGEN_VEC_WD, pWpeConfig.Wpe_VGEN_VEC_HT,       
            pWpeConfig.Wpe_VGEN_HORI_STEP, pWpeConfig.Wpe_VGEN_HORI_STEP, pWpeConfig.Wpe_VGEN_VERT_STEP, pWpeConfig.Wpe_VGEN_VERT_STEP);

    if(ret == MTRUE)
    {
        ret = this->configWpeCommandData(pWpeConfig, configUVplane);
    }

    return ret;
}

MINT32 WarpEnginePipe::
configWpeCommandData(NSCam::NSIoPipe::WPEConfig pWpeConfig, MBOOL configUVplane)
{

    MINT32 ret = true;
    MUINT32 pixelInByte = 0;
    MUINT32 plane_offset = 0;
    MUINT32 wpeo_uv_h_ratio = 2;
    MUINT32 wpeo_uv_v_ratio = 2;

    pWpeConfig.Wpe_CACHE_BURST_LEN = CACHE_VURST_LEN;
    pWpeConfig.Wpe_CACHI_EN = CACHI_EN;
    pWpeConfig.Wpe_WPE_DONE_EN = WPE_DONE_EN;
    pWpeConfig.Wpe_MSKO_EN = MSKO_EN;
    pWpeConfig.Wpe_TDR_ERR_INT_EN = TDR_ERR_INT_EN;
    pWpeConfig.Wpe_VGEN_EN = VGEN_EN;
    pWpeConfig.Wpe_CACHE_BURST_LEN = CACHE_BURST_LEN;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_MOD_EN = (pWpeConfig.Wpe_TDR_EN)         |
                                                                                                    (pWpeConfig.Wpe_VGEN_EN << 4)     |
                                                                                                    (pWpeConfig.Wpe_SYNC_EN << 5)     |
                                                                                                    (pWpeConfig.Wpe_MDP_CROP_EN << 6) |
                                                                                                    (pWpeConfig.Wpe_ISP_CROP_EN << 7) |
                                                                                                    (pWpeConfig.Wpe_C24_EN << 8)      |
                                                                                                    (pWpeConfig.Wpe_TDR_SIZE_DETECT << 31);

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DMA_EN = (pWpeConfig.Wpe_CACHI_EN)      |
                                                                                                     (pWpeConfig.Wpe_VECI_EN << 8)   |
                                                                                                     (pWpeConfig.Wpe_VEC2I_EN << 9)  |
                                                                                                     (pWpeConfig.Wpe_VEC3I_EN << 10) |
                                                                                                     (pWpeConfig.Wpe_WPEO_EN << 16)  |
                                                                                                     (pWpeConfig.Wpe_MSKO_EN << 24);

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_CFG = pWpeConfig.Wpe_CACHE_BURST_LEN |
                                                                                              (pWpeConfig.Wpe_UV_INTER_EN << 4);


    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_FMT_SEL = (pWpeConfig.Wpe_CACHI_FMT) | (pWpeConfig.Wpe_CACHI_FMT_SEQ << 4); //0x5;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_INT_EN = 0;
                                                                                                    //pWpeConfig.Wpe_WPE_DONE_EN	 |
                                                                                                    //pWpeConfig.Wpe_TDR_ERR_INT_EN << 4;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_INT_STATUS = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_INT_STATUSX = 0x00000000;
    if (this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.frameflag == HINT_WPE_TILE_MODE)
    {
        this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_TDR_TILE = 0x0000000F;
    }
    else
    {
        this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_TDR_TILE = 0x00000000;
    }
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_TDR_DBG_STATUS = 0x00000D00;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_TDR_TCM_EN = pWpeConfig.Wpe_CACHI_EN     |
                                                                                                    (pWpeConfig.Wpe_VECI_EN << 1)     |
                                                                                                    (pWpeConfig.Wpe_VEC2I_EN << 2)    |
                                                                                                    (pWpeConfig.Wpe_VEC3I_EN << 3)    |
                                                                                                    (pWpeConfig.Wpe_WPEO_EN << 4)     |
                                                                                                    (pWpeConfig.Wpe_MSKO_EN << 5)     |
                                                                                                    (pWpeConfig.Wpe_VGEN_EN << 6)     |
                                                                                                    (pWpeConfig.Wpe_MDP_CROP_EN << 8) |
                                                                                                    (pWpeConfig.Wpe_ISP_CROP_EN << 9) |
                                                                                                    (pWpeConfig.Wpe_C24_EN << 10);

    if (this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_TDR_TCM_EN > 0)
    {
        this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_MOD_EN |= 0x20000000;
    }

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_SW_CTL = 0x00000002;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_SPARE0 = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_SPARE1 = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_SPARE2 = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DONE_SEL = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DBG_SET = 0x00008000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DBG_PORT = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DATE_CODE = 0x20160919;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_PROJ_CODE = 0x10233000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_WPE_DCM_DIS = 0xFFFFFFFF;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DMA_DCM_DIS = 0xFFFFFFFF;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_WPE_DCM_STATUS = 0x00020AAA;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DMA_DCM_STATUS = 0x00000000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_WPE_REQ_STATUS = 0x00000000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DMA_REQ_STATUS = 0x00000000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_WPE_RDY_STATUS = 0x00006FFB;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DMA_RDY_STATUS = 0x0000001C;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_CTL = (1 << 4)    |
                                                                                                    (pWpeConfig.Wpe_VGEN_HMG_MODE << 16);

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_IN_IMG = (pWpeConfig.Wpe_VGEN_IN_WD) | (pWpeConfig.Wpe_VGEN_IN_HT << 16);
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_OUT_IMG = (pWpeConfig.Wpe_VGEN_OUT_WD) | (pWpeConfig.Wpe_VGEN_OUT_HT << 16);
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_STEP = pWpeConfig.Wpe_VGEN_HORI_STEP;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_STEP = pWpeConfig.Wpe_VGEN_VERT_STEP;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_INT_OFST = pWpeConfig.Wpe_VGEN_HORI_INT_OFST;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_SUB_OFST = pWpeConfig.Wpe_VGEN_HORI_SUB_OFST;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_INT_OFST = pWpeConfig.Wpe_VGEN_VERT_INT_OFST;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_SUB_OFST = pWpeConfig.Wpe_VGEN_VERT_SUB_OFST;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_POST_CTL = pWpeConfig.Wpe_VGEN_POST_SUB_H      |
                                                                                                        (pWpeConfig.Wpe_VGEN_POST_SUB_V << 4) ;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_POST_COMP_X = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_POST_COMP_Y = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_MAX_VEC = (pWpeConfig.Wpe_VGEN_VEC_WD) | (pWpeConfig.Wpe_VGEN_VEC_HT<< 16);

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VFIFO_CTL = 0x0000007C;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CFIFO_CTL = 0x0000007C;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_RWCTL_CTL = 0x00000002;

    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_SPECIAL_FUN_EN = 0x82300000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MDP_CROP_X = (pWpeConfig.Wpe_MDP_CROP_STR_X) | ( pWpeConfig.Wpe_MDP_CROP_END_X << 16);
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MDP_CROP_Y = (pWpeConfig.Wpe_MDP_CROP_STR_Y) | ( pWpeConfig.Wpe_MDP_CROP_END_Y << 16);

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_C24_TILE_EDGE = 0x0000000F;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ISPCROP_CON1 = (pWpeConfig.Wpe_ISPCROP_STR_X) | ( pWpeConfig.Wpe_ISPCROP_END_X << 16);
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ISPCROP_CON2 = (pWpeConfig.Wpe_ISPCROP_STR_Y) | ( pWpeConfig.Wpe_ISPCROP_END_Y << 16);

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_PSP_CTL = (pWpeConfig.Wpe_PSP_TBL_SEL_V << 16) |(pWpeConfig.Wpe_PSP_TBL_SEL_H << 4)|
                                                                                                   (pWpeConfig.Wpe_PSP_SIGN_EXT << 1)|(pWpeConfig.Wpe_PSP_UNSIGN_TO_SIGN);
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_PSP2_CTL = (pWpeConfig.Wpe_PSP2_TBL_SEL_V << 16) |(pWpeConfig.Wpe_PSP2_TBL_SEL_H << 4)|
                                                                                                    (pWpeConfig.Wpe_PSP2_SIGN_EXT << 1)|(pWpeConfig.Wpe_PSP2_UNSIGN_TO_SIGN);

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_SOFT_RSTSTAT_0 = 0x00000001;

    switch(pWpeConfig.Wpe_CACHI_AllFormats.u4imgFmt)
    {
        case eImgFmt_YUY2:
        case eImgFmt_YVYU:
        case eImgFmt_UYVY:
        case eImgFmt_VYUY:
            pixelInByte = 2;
            break;
        case eImgFmt_NV12:
        case eImgFmt_NV21:
            pixelInByte = 1;
            if(pWpeConfig.Wpe_WPEO_EN == 1 && configUVplane == MTRUE)  // 420 stand alone,   UV plane BPP = 16
            {
                pixelInByte = 2;
                plane_offset = 1;
                pWpeConfig.Wpe_WPEO_AllFormats.u4ImgWidth /= wpeo_uv_h_ratio;
                pWpeConfig.Wpe_WPEO_AllFormats.u4ImgHeight /= wpeo_uv_v_ratio;
            }
            break;
    }

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_BASE_ADDR_0 = pWpeConfig.Wpe_CACHI_AllFormats.u4BufPA[plane_offset];
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_OFFSET_ADDR_0 = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_STRIDE_0 = pWpeConfig.Wpe_CACHI_AllFormats.u4Stride[plane_offset];

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON_0 = 0x40000040; //Burs length
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON2_0 =	0x00400040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON3_0 = 0x00400040;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_CTRL_0 = 0x00000000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_STAT_0 = 0x8F6F0000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_RSV1_0 = 0xA73B474E;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_DEBUG_SEL_0 = 0x009AA19B;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_SOFT_RSTSTAT_1 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_SOFT_RSTSTAT_2 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_SOFT_RSTSTAT_3 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_SOFT_RSTSTAT_0;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_BASE_ADDR_1 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_BASE_ADDR_2 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_BASE_ADDR_3 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_BASE_ADDR_0 ;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_OFFSET_ADDR_1 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_OFFSET_ADDR_2 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_OFFSET_ADDR_3 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_OFFSET_ADDR_0;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_STRIDE_1 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_STRIDE_2 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_STRIDE_3 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_STRIDE_0;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON_1 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON_2 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON_3 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON_0;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON2_1 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON2_2 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON2_3 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON2_0;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON3_1 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON3_2 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON3_3 =
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CACHI_CON3_0;

    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_CTRL_1 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_CTRL_2 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_CTRL_3 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_CTRL_0;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_STAT_1 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_STAT_2 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_STAT_3 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_ERR_STAT_0;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_RSV1_1 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_RSV1_2 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_RSV1_3 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_RSV1_0;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_DEBUG_SEL_1 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_DEBUG_SEL_2 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_DEBUG_SEL_3 =
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_DEBUG_SEL_0;

    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_DMA_SOFT_RSTSTAT = 0x00070003;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_TDRI_BASE_ADDR =
    this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.tpipeTablePa_wpe;//pWpeConfig.Wpe_TDR.u4BufPA;

    if(pWpeConfig.Wpe_WPEO_EN == 1 && configUVplane == MTRUE)  // 420 stand alone,   UV plane BPP = 16
    {

    }
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_TDRI_OFST_ADDR = 0x00000000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_TDRI_XSIZE = 0x000000B7;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VERTICAL_FLIP_EN = 0x00000000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_DMA_SOFT_RESET = 0x00000000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_LAST_ULTRA_EN = 0x00000000;
    //this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_SPECIAL_FUN_EN = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_BASE_ADDR = pWpeConfig.Wpe_WPEO_AllFormats.u4BufPA[plane_offset];

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_OFST_ADDR = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_XSIZE = pWpeConfig.Wpe_WPEO_AllFormats.u4ImgWidth * pixelInByte - 1;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_YSIZE = pWpeConfig.Wpe_WPEO_AllFormats.u4ImgHeight - 1;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_STRIDE = pWpeConfig.Wpe_WPEO_AllFormats.u4Stride[plane_offset];
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_CON = 0x80000040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_CON2 = 0x00200020;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_CON3 = 0x00100010;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_CROP = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_BASE_ADDR = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_OFST_ADDR = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_XSIZE = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_YSIZE = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_STRIDE = 0x00000000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_CON = 0x80000040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_CON2 = 0x00400040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_CON3 = 0x00400040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_CROP = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_BASE_ADDR = pWpeConfig.Wpe_VECI.u4BufPA;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_OFST_ADDR = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_XSIZE = pWpeConfig.Wpe_VECI.u4ImgWidth * pWpeConfig.Wpe_VECI.u4ElementSzie - 1;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_YSIZE = pWpeConfig.Wpe_VECI.u4ImgHeight - 1;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_STRIDE = pWpeConfig.Wpe_VECI.u4Stride;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_CON = 0x80000080;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_CON2 = 0x00800080;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_CON3 = 0x00800080;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_BASE_ADDR = pWpeConfig.Wpe_VEC2I.u4BufPA;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_OFST_ADDR = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_XSIZE = pWpeConfig.Wpe_VEC2I.u4ImgWidth * pWpeConfig.Wpe_VEC2I.u4ElementSzie - 1;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_YSIZE = pWpeConfig.Wpe_VEC2I.u4ImgHeight - 1;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_STRIDE = pWpeConfig.Wpe_VEC2I.u4Stride;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_CON = 0x80000040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_CON2 = 0x00400040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_CON3 = 0x00400040;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_BASE_ADDR = pWpeConfig.Wpe_VEC3I.u4BufPA;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_OFST_ADDR = 0x00000000;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_XSIZE = pWpeConfig.Wpe_VEC3I.u4ImgWidth * pWpeConfig.Wpe_VEC3I.u4ElementSzie - 1;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_YSIZE = pWpeConfig.Wpe_VEC3I.u4ImgHeight - 1;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_STRIDE = pWpeConfig.Wpe_VEC3I.u4Stride;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_CON = 0x80000040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_CON2 = 0x00400040;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_CON3 = 0x00400040;

    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_ERR_STAT = 0xFFFF0000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_ERR_STAT = 0xFFFF0000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_ERR_STAT = 0xFFFF0000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_ERR_STAT = 0xFFFF0000;
    this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_ERR_STAT = 0xFFFF0000;

    LOG_INF("WPE_CTL_MOD_EN: 0x%x, WPE_CTL_DMA_EN: 0x%x, WPE_CTL_CFG: 0x%x, WPE_CTL_FMT_SEL: 0x%x,\
            WPE_CTL_FMT_SEL: 0x%x, WPE_CTL_TDR_TCM_EN: 0x%x,  WPE_VGEN_CTL: 0x%x,\n\
            WPE_VGEN_IN_IMG: 0x%x, WPE_VGEN_OUT_IMG: 0x%x, WPE_VGEN_HORI_STEP: 0x%x, WPE_VGEN_VERT_STEP: 0x%x,\n\
            WPE_VGEN_HORI_INT_OFST: 0x%x, WPE_VGEN_HORI_SUB_OFST: 0x%x, WPE_VGEN_VERT_INT_OFST: 0x%x, WPE_VGEN_VERT_SUB_OFST: 0x%x,\n\
            WPE_MDP_CROP_X: 0x%x, WPE_MDP_CROP_Y: 0x%x, WPE_PSP_CTL: 0x%x, WPE_PSP2_CTL: 0x%x, \n\
            WPE_ADDR_GEN_BASE_ADDR_0: 0x%x, WPE_ADDR_GEN_OFFSET_ADDR_0: 0x%x, WPE_ADDR_GEN_STRIDE_0: 0x%x", this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_MOD_EN,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_DMA_EN, this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_CFG,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_FMT_SEL,this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_FMT_SEL,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_CTL_TDR_TCM_EN, this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_CTL, \
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_IN_IMG, this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_OUT_IMG,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_STEP, this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_STEP,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_INT_OFST,this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_SUB_OFST,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_INT_OFST,this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_SUB_OFST,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MDP_CROP_X, this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MDP_CROP_Y,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_PSP_CTL, this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_PSP2_CTL, \
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_BASE_ADDR_0, this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_OFFSET_ADDR_0,\
                this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ADDR_GEN_STRIDE_0);

    ret = this->configTpipeData(pWpeConfig, configUVplane);

return ret;
}

MINT32 WarpEnginePipe::
configTpipeData(NSCam::NSIoPipe::WPEConfig pWpeConfig, MBOOL configUVplane)
{
    MINT32 ret = MTRUE;
    MINT32 isSecisSecureFraure = this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].isSecureFra;

    ISP_TPIPE_CONFIG_STRUCT *pTdri = NULL;
    //
    pTdri = &this->gpWPEMdpMgrCfgData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].ispTpipeCfgInfo;

    pTdri->drvinfo.dupCqIdx_wpe= this->m_wpeEQDupIdx;

    pTdri->drvinfo.debugRegDump= 0;

    pTdri->drvinfo.wpecommand= (unsigned int *)&this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx];

    pTdri->top.veci_en = pWpeConfig.Wpe_VECI_EN ;
    pTdri->top.vec2i_en = pWpeConfig.Wpe_VEC2I_EN;
    pTdri->top.vec3i_en = pWpeConfig.Wpe_VEC3I_EN;
    pTdri->top.sync_en = pWpeConfig.Wpe_SYNC_EN;
    pTdri->top.vgen_en = pWpeConfig.Wpe_VGEN_EN;
    pTdri->top.cachi_en = pWpeConfig.Wpe_CACHI_EN;
    pTdri->top.wpe_cachi_fmt = pWpeConfig.Wpe_CACHI_FMT;

    pTdri->top.wpe_en = 1;
    pTdri->top.wpe_ctl_extension_en = pWpeConfig.Wpe_TDR_EN;

    pTdri->top.msko_en = 0;
    pTdri->top.wpeo_en = pWpeConfig.Wpe_WPEO_EN;
    pTdri->top.wpe_mdpcrop_en = pWpeConfig.Wpe_MDP_CROP_EN;
    pTdri->top.wpe_ispcrop_en = pWpeConfig.Wpe_ISP_CROP_EN;

    pTdri->sw.log_en = 1;
    pTdri->sw.src_width_wpe = (this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_XSIZE + 1)/4;
    pTdri->sw.src_height_wpe = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_YSIZE + 1;

    pTdri->sw.tpipe_irq_mode = TPIPE_IRQ_TPIPE_WPE;
    pTdri->sw.tpipe_width_wpe = MAX_TPIPE_WIDTH_WPE;
    pTdri->sw.tpipe_height_wpe = MAX_TPIPE_HEIGHT_WPE;

    if (isSecisSecureFraure == 1 && pWpeConfig.Wpe_CACHI_AllFormats.secureTag != 0)
    {
        pTdri->drvinfo.wpe_secure_tag = isSecisSecureFraure;
        pTdri->drvinfo.wpe_wpei_base_addr = pWpeConfig.Wpe_CACHI_AllFormats.u4BufPA[0];
        pTdri->drvinfo.wpe_wpei_size [0] = pWpeConfig.Wpe_CACHI_AllFormats.u4BufSize[0];
        pTdri->drvinfo.wpe_wpei_size [1] = pTdri->drvinfo.wpe_wpei_size [2] = 0;
        pTdri->drvinfo.wpe_wpei_secure_tag = pWpeConfig.Wpe_CACHI_AllFormats.secureTag;
    }

    if (pTdri->top.wpeo_en == 1)
    {
        pTdri->wpe.vgen_output_width = pWpeConfig.Wpe_WPEO_AllFormats.u4ImgWidth ;
        pTdri->wpe.vgen_output_height = pWpeConfig.Wpe_WPEO_AllFormats.u4ImgHeight;

        pTdri->wpeo.wpeo_stride = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_STRIDE;
        pTdri->wpeo.wpeo_xoffset = 0;
        pTdri->wpeo.wpeo_yoffset = 0;
        pTdri->wpeo.wpeo_xsize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_XSIZE;
        pTdri->wpeo.wpeo_ysize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_WPEO_YSIZE;

        pTdri->sw.tpipe_sel_mode = 0x10;

        pTdri->top.wpe_c24_en = 0;
        pTdri->top.wpe_mdpcrop_en = 0;
        pTdri->top.wpe_ispcrop_en = 0;

        pTdri->sw.tpipe_irq_mode = TPIPE_IRQ_TPIPE_WPE;

        if(isSecisSecureFraure == 1 && pWpeConfig.Wpe_WPEO_AllFormats.secureTag != 0)
        {
            pTdri->drvinfo.wpe_wpeo_base_addr = pWpeConfig.Wpe_WPEO_AllFormats.u4BufPA[0];
            pTdri->drvinfo.wpe_wpeo_size [0] = pWpeConfig.Wpe_WPEO_AllFormats.u4BufSize[0];
            pTdri->drvinfo.wpe_wpeo_size [1] = pTdri->drvinfo.wpe_wpeo_size [2] = 0;
            pTdri->drvinfo.wpe_wpeo_secure_tag = pWpeConfig.Wpe_WPEO_AllFormats.secureTag;
        }

    }
    else if (pTdri->top.wpe_mdpcrop_en == 1)
    {
        MUINT32 start, end;
        start = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MDP_CROP_X  & 0xFFFF;
        end = (this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MDP_CROP_X  & 0xFFFF0000) >> 16;
        pTdri->wpe.vgen_output_width = end - start + 1;
        start = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MDP_CROP_Y  & 0xFFFF;
        end = (this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MDP_CROP_Y  & 0xFFFF0000) >> 16;
        pTdri->wpe.vgen_output_height = end - start + 1;
        pTdri->top.wpe_c24_en = 1;
        pTdri->top.wpe_mdpcrop_en = 1;
        pTdri->top.wpe_ispcrop_en = 0;
        pTdri->top.wpeo_en = 0;

        pTdri->sw.tpipe_sel_mode = 0x10;
    }
    else if(pTdri->top.wpe_ispcrop_en == 1)
    {
        MUINT32 start, end;

        start = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ISPCROP_CON1	& 0xFFFF;
        end = (this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ISPCROP_CON1  & 0xFFFF0000) >> 16;
        pTdri->wpe.vgen_output_width = end - start + 1;
        start = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ISPCROP_CON2	& 0xFFFF;
        end = (this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_ISPCROP_CON2  & 0xFFFF0000) >> 16;
        pTdri->wpe.vgen_output_height = end - start + 1;
        pTdri->top.wpe_c24_en = 0;
        pTdri->top.wpe_mdpcrop_en = 0;
        pTdri->top.wpe_ispcrop_en = 1;
        pTdri->top.wpeo_en = 0;

        pTdri->sw.tpipe_sel_mode = 0x11;
    }

    LOG_INF("tpipe_sel_mode %d\n", pTdri->sw.tpipe_sel_mode);

    pTdri->wpe.vgen_input_crop_width = pTdri->sw.src_width_wpe;//632
    pTdri->wpe.vgen_input_crop_height = pTdri->sw.src_height_wpe;//355

    //update floating offset for tpipe /*24 bit base*/
    pTdri->wpe.vgen_luma_horizontal_integer_offset= this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_INT_OFST;
    pTdri->wpe.vgen_luma_horizontal_subpixel_offset= this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_SUB_OFST;

    pTdri->wpe.vgen_luma_vertical_integer_offset = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_INT_OFST;
    pTdri->wpe.vgen_luma_vertical_subpixel_offset = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_SUB_OFST;

    pTdri->wpe.vgen_horizontal_coeff_step = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_HORI_STEP;
    pTdri->wpe.vgen_vertical_coeff_step = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VGEN_VERT_STEP;

    if(pTdri->top.msko_en == 1)
    {
        pTdri->msko.msko_stride = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_STRIDE;
        pTdri->msko.msko_xoffset = 0;
        pTdri->msko.msko_yoffset = 0;
        pTdri->msko.msko_xsize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_XSIZE;
        pTdri->msko.msko_ysize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_MSKO_YSIZE;
    }

    pTdri->veci.veci_stride = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_STRIDE;
    pTdri->veci.veci_v_flip_en= 0;
    pTdri->veci.veci_xsize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_XSIZE;
    pTdri->veci.veci_ysize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VECI_YSIZE;

    pTdri->vec2i.vec2i_stride = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_STRIDE;
    pTdri->vec2i.vec2i_v_flip_en= 0;
    pTdri->vec2i.vec2i_xsize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_XSIZE;
    pTdri->vec2i.vec2i_ysize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC2I_YSIZE;

    pTdri->vec3i.vec3i_stride = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_STRIDE;
    pTdri->vec3i.vec3i_v_flip_en= 0;
    pTdri->vec3i.vec3i_xsize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_XSIZE;
    pTdri->vec3i.vec3i_ysize = this->wCmdData[this->m_wpeEQBurstIdx][this->m_wpeEQDupIdx][this->m_wpeDupCmdIdx].WPE_VEC3I_YSIZE;

EXIT:
    return ret;

}


MBOOL  WarpEnginePipe::
wpeconfigMdpOutPort(NSImageio::NSIspio::PortInfo const* oImgInfo, MdpRotDMACfg &a_rotDma, NSCam::NSIoPipe::EDIPSecureEnum mSecureTag)
{
    MUINT32 plane_num = 1;
    MUINT32 uv_resample = 1;
    MUINT32 uv_h_ratio = 1;
    MUINT32 uv_v_ratio = 1;
    MUINT32 y_plane_size = 0;
    MUINT32 u_plane_size = 0;
    //
    LOG_DBG("[oImgInfo]w(%d),h(%d),stride(%d/%d/%d),crop(%d,%d,%d,%d)_f(0x%x, 0x%x),rot(%d) eImgFmt(%d)",
        oImgInfo->u4ImgWidth,oImgInfo->u4ImgHeight,oImgInfo->u4Stride[NSImageio::NSIspio::ESTRIDE_1ST_PLANE],oImgInfo->u4Stride[NSImageio::NSIspio::ESTRIDE_2ND_PLANE],oImgInfo->u4Stride[NSImageio::NSIspio::ESTRIDE_3RD_PLANE], \
        oImgInfo->crop1.x,oImgInfo->crop1.y,oImgInfo->crop1.w,oImgInfo->crop1.h,oImgInfo->crop1.floatX,oImgInfo->crop1.floatY, \
        oImgInfo->eImgRot,oImgInfo->eImgFmt );
    //
    a_rotDma.uv_plane_swap = 0;
    //
    switch( oImgInfo->eImgFmt ) {
       case eImgFmt_YV12:      //= 0x00008,   //420 format, 3 plane(YVU)
       case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
           a_rotDma.Format = eImgFmt_YV12;
           a_rotDma.Plane = 2;
           plane_num = 3;
           uv_h_ratio = 2;
           uv_v_ratio = 2;
           a_rotDma.uv_plane_swap = (eImgFmt_YV12==oImgInfo->eImgFmt)?1:0;
           break;
        case eImgFmt_I422:      //422 format, 3 plane(YUV)
           a_rotDma.Format = eImgFmt_I422;
           a_rotDma.Plane = 2;
           plane_num = 3;
           uv_h_ratio = 2;
           uv_v_ratio = 1;
           break;
        case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
            a_rotDma.Format = eImgFmt_NV21;
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 2;
            break;
        case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
            a_rotDma.Format = eImgFmt_NV12;
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_rotDma.Format = eImgFmt_YUY2;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_rotDma.Format = eImgFmt_UYVY;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_YVYU:		//= 0x080000,   //422 format, 1 plane (UYVY)
            a_rotDma.Format = eImgFmt_YVYU;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_VYUY:      //= 0x100000,   //422 format, 1 plane (UYVY)
            a_rotDma.Format = eImgFmt_VYUY;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
            a_rotDma.Format = eImgFmt_YV16;
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            a_rotDma.uv_plane_swap = 1;
            break;
        case eImgFmt_NV16:      //422 format, 2 plane
            a_rotDma.Format = eImgFmt_NV16;
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 1;
            break;
        case eImgFmt_NV61:      //422 format, 2 plane
            a_rotDma.Format = eImgFmt_NV61;
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 1;
            a_rotDma.uv_plane_swap = 1;
            break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            a_rotDma.Format = eImgFmt_RGB565;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            a_rotDma.Format = eImgFmt_RGB888;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            a_rotDma.Format = eImgFmt_ARGB8888;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_RGBA8888:
            a_rotDma.Format = eImgFmt_RGBA8888;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_Y800:		//= 0x040000, //Y plane only
        case eImgFmt_STA_BYTE:
        case eImgFmt_Y16:
            a_rotDma.Format = eImgFmt_STA_BYTE;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_JPG_I420:
            a_rotDma.Format = eImgFmt_JPG_I420;
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_JPG_I422:
            a_rotDma.Format = eImgFmt_JPG_I422;
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            break;
        case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit     // for imgo
            a_rotDma.Format = eImgFmt_BAYER8;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit    // for imgo
            a_rotDma.Format = eImgFmt_BAYER10;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit    // for imgo
            a_rotDma.Format = eImgFmt_BAYER12;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
        case eImgFmt_UFO_FG:
        default:
            LOG_ERR("vOutPorts[]->eImgFmt:Format(%d) NOT Support",oImgInfo->eImgFmt);
            break;
    }
    //
    //ROTATION, stride is after, others are before
    a_rotDma.Rotation = (MUINT32)oImgInfo->eImgRot;
    a_rotDma.Flip = (NSImageio::NSIspio::eImgFlip_ON == oImgInfo->eImgFlip)?MTRUE:MFALSE;
    //dma port capbility
    a_rotDma.capbility=oImgInfo->capbility;
    //Y
    a_rotDma.memBuf.base_pAddr = oImgInfo->u4BufPA[0];
    a_rotDma.memBuf.base_vAddr = oImgInfo->u4BufVA[0];
    a_rotDma.memBuf.size = oImgInfo->u4BufSize[0];
    a_rotDma.memBuf.ofst_addr = 0;
    a_rotDma.memBuf.alignment = 0;
    //after ROT
    a_rotDma.size.w = oImgInfo->u4ImgWidth;
    a_rotDma.size.h = oImgInfo->u4ImgHeight;
    //stride info after ROT
    a_rotDma.size.stride = oImgInfo->u4Stride[NSImageio::NSIspio::ESTRIDE_1ST_PLANE];
    //
    y_plane_size = oImgInfo->u4BufSize[NSImageio::NSIspio::ESTRIDE_1ST_PLANE];
    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}
    //
    if (2 <= plane_num) {
        //U
        a_rotDma.memBuf_c.base_pAddr = oImgInfo->u4BufPA[NSImageio::NSIspio::ESTRIDE_2ND_PLANE];
        a_rotDma.memBuf_c.base_vAddr = oImgInfo->u4BufVA[NSImageio::NSIspio::ESTRIDE_2ND_PLANE];
        a_rotDma.memBuf_c.ofst_addr = 0;
        a_rotDma.memBuf_c.alignment = 0;
        //after ROT
        a_rotDma.size_c.w =         a_rotDma.size.w/uv_h_ratio;
        a_rotDma.size_c.h =         a_rotDma.size.h/uv_v_ratio;
        //stride info after ROT
        a_rotDma.size_c.stride =    oImgInfo->u4Stride[NSImageio::NSIspio::ESTRIDE_2ND_PLANE];
        //
        u_plane_size = oImgInfo->u4BufSize[NSImageio::NSIspio::ESTRIDE_2ND_PLANE];
        //tpipemain lib need image info before ROT. stride info after ROT
        //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //    a_rotDma.size_c.w = a_rotDma.size.h/uv_v_ratio;
        //    a_rotDma.size_c.h = a_rotDma.size.w/uv_h_ratio;
        //}
        //
        if ( 3 == plane_num ) {
            //V
            a_rotDma.memBuf_v.base_pAddr = oImgInfo->u4BufPA[NSImageio::NSIspio::ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_v.base_vAddr = oImgInfo->u4BufVA[NSImageio::NSIspio::ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_v.ofst_addr = 0;
            a_rotDma.memBuf_v.alignment = 0;
            //after ROT
            a_rotDma.size_v.w =        a_rotDma.size_c.w;
            a_rotDma.size_v.h =        a_rotDma.size_c.h;
            //stride info after ROT
            a_rotDma.size_v.stride =   oImgInfo->u4Stride[NSImageio::NSIspio::ESTRIDE_3RD_PLANE];
            //tpipemain lib need image info BEFORE ROT. stride info AFTER ROT
            //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
            //    a_rotDma.size_v.w = a_rotDma.size_c.h;
            //    a_rotDma.size_v.h = a_rotDma.size_c.w;
                //}
        }
        //
        /*
        if ( a_rotDma.uv_plane_swap ) {
            a_rotDma.memBuf_c.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_c.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_v.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_2ND_PLANE];
            a_rotDma.memBuf_v.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_2ND_PLANE];
        }
        */
    }

    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
    //    //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}

    /*
        * DONOT USE CRZ CROP due to throughtput issue
        */
    //default set ENABLE, if need to be programmable, check cdp driver as well.
    a_rotDma.crop_en = 1; //always enable crop for rotation issue.
    //
    a_rotDma.crop.x = 0;
    a_rotDma.crop.floatX = 0;
    a_rotDma.crop.y = 0;
    a_rotDma.crop.floatY = 0;
    a_rotDma.crop.w = 0;
    a_rotDma.crop.h = 0;

    a_rotDma.secureTag= mSecureTag;
    //
    LOG_DBG("[a_rotDma]w(%d),h(%d),stride(pxl)(%d),pa(0x%x),va(0x%x),size(%d)",a_rotDma.size.w,a_rotDma.size.h,a_rotDma.size.stride,a_rotDma.memBuf.base_pAddr,a_rotDma.memBuf.base_vAddr,a_rotDma.memBuf.size);
    LOG_DBG("[a_rotDma]crop(%d,%d,%d,%d)_f(0x%x,0x%x)",a_rotDma.crop.x,a_rotDma.crop.y,a_rotDma.crop.w,a_rotDma.crop.h,a_rotDma.crop.floatX,a_rotDma.crop.floatY);
    LOG_DBG("[a_rotDma]rot(%d),fmt(%d),plane(%d),pxlByte((0x%x))",a_rotDma.Rotation,a_rotDma.Format,a_rotDma.Plane,a_rotDma.pixel_byte);
    LOG_DBG("[a_rotDma]c_ofst_addr(0x%x),v_ofst_addr(0x%x),uv_plane_swap(%d)",a_rotDma.memBuf_c.ofst_addr,a_rotDma.memBuf_v.ofst_addr,a_rotDma.uv_plane_swap);
    LOG_DBG("[a_rotDma]va[0x%x,0x%x,0x%x]--pa[0x%x,0x%x,0x%x]",
        a_rotDma.memBuf.base_vAddr,a_rotDma.memBuf_c.base_vAddr,a_rotDma.memBuf_v.base_vAddr,a_rotDma.memBuf.base_pAddr,a_rotDma.memBuf_c.base_pAddr,a_rotDma.memBuf_v.base_pAddr);

    return MTRUE;

}

MINT32 WarpEnginePipe::
start(MINT32  wpeDupIdx, MINT32  wpeBurstIdx, MUINT32 wpeDupCmdIdx)
{
    MINT32 ret = 0;

    LOG_INF("+,cqIndx(%u),tpipVa(0x%x),tpipPa(0x%x),isWaitBuf(%d),tpipe_sel_mode(0x%x)", \
            this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe,\
            this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.tpipeTableVa_wpe,\
            this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx].ispTpipeCfgInfo.drvinfo.tpipeTablePa_wpe, \
            this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx].isWaitBuf,\
            this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx].ispTpipeCfgInfo.sw.tpipe_sel_mode);

    ret = this->m_pMdpMgr->startMdp(this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx]);

    return ret;

}

MINT32 WarpEnginePipe::
dequeue(MINT32  wpeDupIdx, MINT32  wpeBurstIdx, MUINT32 wpeDupCmdIdx)
{
    MINT32 mdp_err = -1;
    MDPMGR_DEQUEUE_INDEX dmaChannel = MDPMGR_DEQ_WDMA;

    // De-q source buffer
    mdp_err = this->m_pMdpMgr->dequeueBuf(MDPMGR_DEQ_SRC, this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx]);

    if(mdp_err != MDPMGR_NO_ERROR){
        LOG_ERR("wpeDupIdx(%d), wpeBurstIdx(%d), wpeDupCmdIdx(d), MDPMGR_DEQ_SRC mdpRet(%d)", wpeBurstIdx, wpeDupIdx, wpeDupCmdIdx, mdp_err);
        return mdp_err;
    }

    // De-q destination buffer
    for(MINT32 index = 0; index < ISP_OUTPORT_NUM; index++) {
        if(this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx].dstPortCfg[index] == 1 && index != 3)
        {
            switch(index){
                case 0:
                    dmaChannel = MDPMGR_DEQ_WDMA;
                    break;
                case 1:
                    dmaChannel = MDPMGR_DEQ_WROT;
                    break;
                case 2:
                    dmaChannel = MDPMGR_DEQ_JPEG;
                    break;
                default:
                    LOG_ERR("[Error]Not support this dequeOutBuf index(%d)",index);
                break;
            }

            mdp_err = this->m_pMdpMgr->dequeueBuf(dmaChannel, this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx]);

            if(mdp_err != MDPMGR_NO_ERROR){
                LOG_ERR("wpeDupIdx(%d), wpeBurstIdx(%d), MDPMGR_DEQ_DST_DMAChannel(%d) mdpRet(%d)",wpeDupIdx, wpeBurstIdx, index, mdp_err);
                return mdp_err;
            }
        }
    }

    // De-q Frame End
    mdp_err = this->m_pMdpMgr->dequeueFrameEnd(this->gpWPEMdpMgrCfgData[wpeBurstIdx][wpeDupIdx][wpeDupCmdIdx]);

    if(mdp_err != MDPMGR_NO_ERROR)
    {
        LOG_ERR("WPE wpeDupIdx(%d), wpeBurstIdx(%d),MDPMGR_DEQ_FrameEnd mdpRet(%d)", wpeDupIdx, wpeBurstIdx, mdp_err);
        return mdp_err;
    }

    return mdp_err;

}


MINT32 WarpEnginePipe::HandleExtraCommand(QParams rParams, NSCam::NSIoPipe::NSWpe::WPEQParams *wpeqpararms)
{
    MINT32 ret = 0;

    return ret;

}

/*******************************************************************************
*
********************************************************************************/
#if 0
MBOOL WarpEnginePipe::bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine p2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout)
{
    MBOOL ret=MTRUE;

    MUINT8* data = (MUINT8*)malloc(12 *sizeof(MUINT8));
    data[0]=static_cast<MUINT8>(cmd);
    data[1]=static_cast<MUINT8>(p2Eng);
    data[2]=static_cast<MUINT8>(0);
    data[3]=static_cast<MUINT8>(callerID);
    data[4]=static_cast<MUINT8>(callerID>>8);
    data[5]=static_cast<MUINT8>(callerID>>16);
    data[6]=static_cast<MUINT8>(callerID>>24);
    data[7]=static_cast<MUINT8>(frameNum);
    data[8]=static_cast<MUINT8>(p2CQ);
    data[9]=static_cast<MUINT8>(p2dupCQIdx);
    data[10]=static_cast<MUINT8>(p2burstQIdx);
    data[11]=static_cast<MUINT8>(timeout);

    LOG_INF("input(%d_%d_%d_0x%x_%d_%d_%d_%d_%d)",cmd,p2Eng,0,callerID,frameNum,p2CQ,p2dupCQIdx,p2burstQIdx,timeout);
    LOG_INF("data(%d_%d_%d_0x%x_%x_%x_%x_%d_%d_%d_%d_%d)",data[0],data[1],data[2],data[6],data[5],data[4],data[3],data[7],data[8],data[9],data[10],data[11]);

    ret=this->m_pWpeDrv->setDeviceInfo(_SET_WPE_BUF_INFO, data);
    LOG_INF("cmd(%d),ret(%d)",cmd,ret);

    free(data);

    return ret;

}
#endif
