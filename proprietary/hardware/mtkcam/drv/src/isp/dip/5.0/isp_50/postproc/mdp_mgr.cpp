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

//! \file  mdp_mgr.cpp

#define LOG_TAG "MdpMgr"

#include <utils/Errors.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include <cutils/properties.h>  // For property_get().
#include "isp_datatypes.h"
#include <ispio_pipe_scenario.h>    // For enum EDrvScenario.
#include <ispio_pipe_ports.h>        // For enum EPortCapbility.
#include <isp_drv.h>
#include "mdp_mgr_imp.h"

#include "DpDataType.h" // For DP_STATUS_ENUM

#include <utils/Trace.h> //for systrace


/*************************************************************************************
* Log Utility
*************************************************************************************/

#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include "imageio_log.h"    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(mdpmgr);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (mdpmgr_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (mdpmgr_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

pthread_mutex_t mMutex_DpStreamMap = PTHREAD_MUTEX_INITIALIZER;    //mutex for cq user number when enqueue/dequeue
static volatile MDPMGR_STREAMSELECT_STRUCT_s mDpStreamMap[DIP_RING_BUFFER_CQ_SIZE][MAX_CMDQ_RING_BUFFER_SIZE_NUM]; //mapping table for selecting dpStream object
/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
 MDP_PORT_MAPPING mdpPortMapping[ISP_OUTPORT_NUM] = {
     {ISP_MDP_DL_WDMAO, MDPMGR_ISP_MDP_DL},
     {ISP_MDP_DL_WROTO, MDPMGR_ISP_MDP_DL},
     {ISP_MDP_DL_JPEGO, MDPMGR_ISP_MDP_JPEG_DL},
     {ISP_ONLY_OUT_TPIPE, MDPMGR_ISP_ONLY_TPIPE},
     {ISP_ONLY_OUT_FRAME, MDPMGR_ISP_ONLY_FRAME}};



MINT32 getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}


/**************************************************************************
*
**************************************************************************/
template <typename T>
MINT32 _wpestartMdp(MDPMGR_CFG_STRUCT &cfgData,T* pDpStream);
MVOID dumpWpeTPipeInfo(ISP_TPIPE_CONFIG_STRUCT a_info, MBOOL dump_en);


/**************************************************************************
*
**************************************************************************/
template <typename T>
MINT32 _startMdp(MDPMGR_DPSTREAM_TYPE dpStreamtype,MDPMGR_CFG_STRUCT &cfgData,T* pDpStream)
{
	MINT32 err = MDPMGR_NO_ERROR;
    DP_STATUS_ENUM err2 = DP_STATUS_RETURN_SUCCESS;
    ISP2MDP_STRUCT isp2mdpcfg;

    MUINT32 srcVirList[PLANE_NUM];
    MUINT32 srcPhyList[PLANE_NUM];
    MUINT32 srcSizeList[PLANE_NUM];
    MUINT32 WDMAPQEnable = 0, WROTPQEnable = 0;

    //get property for p2tpipedump
    int p2tpipedumpEnable = ::property_get_int32("vendor.camera.p2tpipedump.enable", 0);
    int p2disableSkipTpipe = ::property_get_int32("vendor.camera.disable_p2skiptpipe", 0);
    int p2disablePQ = ::property_get_int32("vendor.camera.P2MDP_PQ.disable", 0);

    ISP_TRACE_CALL();

    if (cfgData.ispTpipeCfgInfo.drvinfo.frameflag != 2) // ISP ONLY FRAME MODE sould Skip src & dst settings
    {
    //====== Configure Source ======

        DP_COLOR_ENUM srcFmt;

        // format convert
        err = DpColorFmtConvert(cfgData.mdpSrcFmt, &srcFmt);
        if(err != MDPMGR_NO_ERROR)
        {
            LOG_ERR("DpColorFmtConvert fail");
            return MDPMGR_API_FAIL;
        }

        if (cfgData.isSecureFra == 1)
        {
            err2= pDpStream->setSrcConfig(cfgData.mdpSrcW ,cfgData.mdpSrcH, \
                cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,srcFmt, \
                DP_PROFILE_FULL_BT601, eInterlace_None, 0, false, (DpSecure)cfgData.srcSecureTag);
        }
        else
        {
            err2= pDpStream->setSrcConfig(cfgData.mdpSrcW ,cfgData.mdpSrcH, \
                cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,srcFmt, \
                DP_PROFILE_FULL_BT601, eInterlace_None, 0, false, DP_SECURE_NONE);
        }

        if(err2 != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("setSrcConfig fail(%d)",err2);
            return MDPMGR_API_FAIL;
        }

        srcSizeList[0] = cfgData.mdpSrcBufSize;
        srcSizeList[1] = cfgData.mdpSrcCBufSize;
        srcSizeList[2] = cfgData.mdpSrcVBufSize;

        // only for dpframework debug, so set imgi va and pa always for each plane
        srcPhyList[0] = cfgData.srcPhyAddr;
        srcPhyList[1] = cfgData.srcPhyAddr;
        srcPhyList[2] = cfgData.srcPhyAddr;
        srcVirList[0] = cfgData.srcVirAddr;
        srcVirList[1] = cfgData.srcVirAddr;
        srcVirList[2] = cfgData.srcVirAddr;

        if (cfgData.isSecureFra == 1)
        {
            //err2 = pDpStream->queueSrcBuffer(srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);
            err2 = pDpStream->queueSrcBuffer((void**)srcPhyList,srcSizeList,cfgData.mdpSrcPlaneNum);
        }
        else
        {
            err2 = pDpStream->queueSrcBuffer(&srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);
        }
        if(err2 != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("queueSrcBuffer fail(%d)",err2);
            return MDPMGR_API_FAIL;
        }


        //====== Configure Output DMA ======

        DP_COLOR_ENUM dstFmt;
        MVOID *dstVirList[PLANE_NUM];
        MUINT32 dstSizeList[PLANE_NUM];
        MUINT32 dstPhyList[PLANE_NUM];
            uintptr_t secdstPhyList[PLANE_NUM];
        DpRect pROI;
        //DpPqParam   ISPParam;
        DpPqParam*   pPQParam;
        for(MINT32 index = 0; index < ISP_OUTPORT_NUM; index++) {

            if(cfgData.dstPortCfg[index] == 1) {
                if (index != ISP_ONLY_OUT_TPIPE ) { // for non isp only
                    if(cfgData.dstDma[index].enSrcCrop) {  //set src crop if need

                        if(cfgData.dstDma[index].srcCropW==0 || cfgData.dstDma[index].srcCropH==0) {
                            LOG_ERR("[Error](%d) wrong crop w(%d),h(%d)",index,cfgData.dstDma[index].srcCropW,cfgData.dstDma[index].srcCropH);
                            return MDPMGR_WRONG_PARAM;
                        } else if(cfgData.dstDma[index].srcCropW>cfgData.mdpSrcW || cfgData.dstDma[index].srcCropH>cfgData.mdpSrcH) {
                            LOG_ERR("[Error](%d) crop size(%d,%d) exceed source size(%d,%d)",index,cfgData.dstDma[index].srcCropW,cfgData.dstDma[index].srcCropH,cfgData.mdpSrcW,cfgData.mdpSrcH);
                            return MDPMGR_WRONG_PARAM;
                        }
                        if(dpStreamtype==MDPMGR_DPSTREAM_TYPE_VENC)
                        {
                            err2 = pDpStream->setSrcCrop(index,
                                              cfgData.dstDma[index].srcCropX,
                                              cfgData.dstDma[index].srcCropFloatX,
                                              cfgData.dstDma[index].srcCropY,
                                              cfgData.dstDma[index].srcCropFloatY,
                                              cfgData.dstDma[index].srcCropW,
                                              cfgData.dstDma[index].srcCropH);
                        } else {
                            err2 = pDpStream->setSrcCrop(index,
                                              cfgData.dstDma[index].srcCropX,
                                              cfgData.dstDma[index].srcCropFloatX,
                                              cfgData.dstDma[index].srcCropY,
                                              cfgData.dstDma[index].srcCropFloatY,
                                              cfgData.dstDma[index].srcCropW,
                                              cfgData.dstDma[index].srcCropFloatW,
                                              cfgData.dstDma[index].srcCropH,
                                              cfgData.dstDma[index].srcCropFloatH);
                        }
                        if(err2 != DP_STATUS_RETURN_SUCCESS) {
                            LOG_ERR("(%d)setSrcCrop fail(%d)",index,err2);
                            return MDPMGR_API_FAIL;
                        }
                    }

                    // format convert
                    err = DpDmaOutColorFmtConvert(cfgData.dstDma[index],&dstFmt);
                    if(err != MDPMGR_NO_ERROR) {
                        LOG_ERR("DpDmaOutColorFmtConvert fail");
                        return MDPMGR_API_FAIL;
                    }

                    // image info configure
                    pROI.x=0;
                    pROI.y=0;
                    pROI.sub_x=0;
                    pROI.sub_y=0;
                    pROI.w=cfgData.dstDma[index].size.w;
                    pROI.h=cfgData.dstDma[index].size.h;
                    DP_PROFILE_ENUM dp_rofile=DP_PROFILE_FULL_BT601;
                    switch(index) {
                        case ISP_MDP_DL_WDMAO:
                            {
                                if (cfgData.mdpWDMAPQParam != NULL)
                                {
                                    pPQParam = (DpPqParam*)(cfgData.mdpWDMAPQParam);
                                    WDMAPQEnable = pPQParam->enable;
                                }
                                if (p2disablePQ == 0)
                                {
                                    err2 = pDpStream->setPQParameter(index, ((DpPqParam*)(cfgData.mdpWDMAPQParam)));
                                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                                        LOG_ERR("index(%d), setPQParameter fail(%d)",index, err2);
                                        return MDPMGR_API_FAIL;
                                    }
                                }
                            }
                            break;
                        case ISP_MDP_DL_WROTO:
                            {
                                if (cfgData.mdpWROTPQParam != NULL)
                                {
                                    pPQParam = (DpPqParam*)(cfgData.mdpWROTPQParam);
                                    WROTPQEnable = pPQParam->enable;
                                }
                                if (p2disablePQ == 0)
                                {
                                    err2 = pDpStream->setPQParameter(index, ((DpPqParam*)(cfgData.mdpWROTPQParam)));
                                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                                        LOG_ERR("index(%d), setPQParameter fail(%d)",index, err2);
                                        return MDPMGR_API_FAIL;
                                    }
                                }
                            }
                            break;
                        default:
                            break;
                    }
                    switch(cfgData.dstDma[index].capbility) {
                        case NSImageio::NSIspio::EPortCapbility_Rcrd:
                            {
                                dp_rofile=DP_PROFILE_BT601;
                            }
                            break;
                        case NSImageio::NSIspio::EPortCapbility_Cap:
                            {
                                dp_rofile=DP_PROFILE_FULL_BT601;
                            }
                            break;
                        case NSImageio::NSIspio::EPortCapbility_Disp:
                            {
                                dp_rofile=DP_PROFILE_FULL_BT601;
                            }
                            break;
        	            default:
        	            	dp_rofile=DP_PROFILE_FULL_BT601;
        	            	break;
        	        }
                    if ((cfgData.isSecureFra == 1) && (cfgData.dstDma[index].secureTag != 0))
                    {
                        err2 = pDpStream->setDstConfig(index,
                                                      cfgData.dstDma[index].size.w,
                                                      cfgData.dstDma[index].size.h,
                                                      cfgData.dstDma[index].size.stride,
                                                      cfgData.dstDma[index].size_c.stride,
                                                      dstFmt,
                                                      dp_rofile,
                                                      eInterlace_None,
                                                      &pROI,
                                                      false,
                                                      (DpSecure)cfgData.dstDma[index].secureTag);
                    }
                    else
                    {
                        err2 = pDpStream->setDstConfig(index,
                                                      cfgData.dstDma[index].size.w,
                                                      cfgData.dstDma[index].size.h,
                                                      cfgData.dstDma[index].size.stride,
                                                      cfgData.dstDma[index].size_c.stride,
                                                      dstFmt,
                                                      dp_rofile,
                                                      eInterlace_None,
                                                      &pROI,
                                                      false,
                                                      DP_SECURE_NONE);
                    }

                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                        LOG_ERR("setDstConfig fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }

                    if(dpStreamtype==MDPMGR_DPSTREAM_TYPE_VENC) {
                        // set port type (only for ISP only mode)
                        if(index == ISP_ONLY_OUT_TPIPE){
                            pDpStream->setPortType(index, mdpPortMapping[index].mdpPortIdx);
                        }
                        else if(index == ISP_MDP_DL_VENCO)
                        {
                            DpVEnc_Config	DpVEncCfg;
                            //FIXME, use MDPMGR_ISP_MDP_VENC to work around first, cuz port num supported in dpispstream and dpvencstream is 3
                            //          so we may set ISP_MDP_DL_VENCO=ISP_MDP_DL_JPEGO=2 first
                            //          *** should set DpVEncCfg
                            reinterpret_cast<DpVEncStream*>(pDpStream)->setPortType(index, MDPMGR_ISP_MDP_VENC, &DpVEncCfg);
                        }
                    } else {
                        // set port type (only for ISP only mode)
                        if(index == ISP_ONLY_OUT_TPIPE){
                            pDpStream->setPortType(index, mdpPortMapping[index].mdpPortIdx);
                        }
                    }

                    // For MDP_CROP2
                    if ((index == ISP_MDP_DL_WDMAO) || (index == ISP_MDP_DL_WROTO))
                        if (cfgData.dstDma[index].crop_group == 1) // MDP_CROP2
                            pDpStream->setPortType(index, MDPMGR_ISP_MDP_DL2);
                        else // MDP_CROP
                            pDpStream->setPortType(index, mdpPortMapping[index].mdpPortIdx);

                    // rotation
                    err2 = pDpStream->setRotation(index, cfgData.dstDma[index].Rotation * 90);
                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                        LOG_ERR("setRotation fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }

                    // flip
                    err2 = pDpStream->setFlipStatus(index, cfgData.dstDma[index].Flip);
                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                        LOG_ERR("setFlipStatus fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }

                    // memory
                    if(cfgData.dstDma[index].memBuf.base_vAddr == 0 || cfgData.dstDma[index].memBuf.base_pAddr == 0) {
                        LOG_ERR("index(%d)",index);
                        LOG_ERR("dst memAddr is 0,VA(0x%lx),PA(0x%lx)",(unsigned long)cfgData.dstDma[index].memBuf.base_vAddr,(unsigned long)cfgData.dstDma[index].memBuf.base_pAddr);
                        LOG_ERR("dst, W(%lu),H(%lu),W_c(%lu),H_c(%lu),W_v(%lu),H_v(%lu)",cfgData.dstDma[index].size.w,
                                                                                    cfgData.dstDma[index].size.h,
                                                                                    cfgData.dstDma[index].size_c.w,
                                                                                    cfgData.dstDma[index].size_c.h,
                                                                                    cfgData.dstDma[index].size_v.w,
                                                                                    cfgData.dstDma[index].size_v.h);

                        LOG_ERR("stride(%lu),stride_c(%lu),stride_v(%lu)",cfgData.dstDma[index].size.stride,
                                                                       cfgData.dstDma[index].size_c.stride,
                                                                       cfgData.dstDma[index].size_v.stride);
                        return MDPMGR_NULL_OBJECT;
                    }

                    dstSizeList[0] = cfgData.dstDma[index].size.h   * (cfgData.dstDma[index].size.stride);
                    dstSizeList[1] = cfgData.dstDma[index].size_c.h * (cfgData.dstDma[index].size_c.stride);
                    dstSizeList[2] = cfgData.dstDma[index].size_v.h * (cfgData.dstDma[index].size_v.stride);

                    dstVirList[0] = (MVOID *)cfgData.dstDma[index].memBuf.base_vAddr;
                    dstVirList[1] = (MVOID *)cfgData.dstDma[index].memBuf_c.base_vAddr;
                    dstVirList[2] = (MVOID *)cfgData.dstDma[index].memBuf_v.base_vAddr;

                    dstPhyList[0] = cfgData.dstDma[index].memBuf.base_pAddr;
                    dstPhyList[1] = cfgData.dstDma[index].memBuf_c.base_pAddr;
                    dstPhyList[2] = cfgData.dstDma[index].memBuf_v.base_pAddr;

                    if ((cfgData.isSecureFra == 1) && (cfgData.dstDma[index].secureTag != 0))
                    {
                            secdstPhyList[0] = (uintptr_t)dstPhyList[0];
                            secdstPhyList[1] = (uintptr_t)dstPhyList[1];
                            secdstPhyList[2] = (uintptr_t)dstPhyList[2];
                        //err2 = pDpStream->queueDstBuffer(index, dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                            err2 = pDpStream->queueDstBuffer(index, (void **)secdstPhyList, dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    }
                    else if (dpStreamtype == MDPMGR_DPSTREAM_TYPE_VENC)
                    {
                        err2 = pDpStream->queueDstBuffer(index, &dstVirList[0],&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    }
                    else
                    {
                        err2 = pDpStream->queueDstBuffer(index,&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    }

                    if(err2 != DP_STATUS_RETURN_SUCCESS)
                    {
                        LOG_ERR("queueDstBuffer fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }
                }
                else
                    { // for isp only mode      ISP ONLY MODE doesn't need to config src & dst after Everest, WAIT FOR DELETE!!!

                    // format convert
                    err = DpDmaOutColorFmtConvert(cfgData.dstDma[index],&dstFmt);
                    if(err != MDPMGR_NO_ERROR) {
                        LOG_ERR("DpDmaOutColorFmtConvert fail");
                        return MDPMGR_API_FAIL;
                    }

                    // image info configure
                    if ((cfgData.isSecureFra == 1) && (cfgData.dstDma[index].secureTag != 0))
                    {
                        err2 = pDpStream->setDstConfig(index,
                                                      cfgData.dstDma[index].size.w,
                                                      cfgData.dstDma[index].size.h,
                                                      cfgData.dstDma[index].size.stride,
                                                      cfgData.dstDma[index].size_c.stride,
                                                      dstFmt,
                                                      DP_PROFILE_FULL_BT601,
                                                      eInterlace_None,
                                                      &pROI,
                                                      false,
                                                      (DpSecure)cfgData.dstDma[index].secureTag);
                    }
                    else
                    {
                        err2 = pDpStream->setDstConfig(index,
                                                      cfgData.dstDma[index].size.w,
                                                      cfgData.dstDma[index].size.h,
                                                      cfgData.dstDma[index].size.stride,
                                                      cfgData.dstDma[index].size_c.stride,
                                                      dstFmt,
                                                      DP_PROFILE_FULL_BT601,
                                                      eInterlace_None,
                                                      &pROI,
                                                      false,
                                                      DP_SECURE_NONE);
                    }
                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                        LOG_ERR("setDstConfig fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }
                    //
                    pDpStream->setPortType(index, mdpPortMapping[index].mdpPortIdx);  // set port type (only for ISP only mode)

                    // memory
                    if(cfgData.dstDma[index].memBuf.base_vAddr == 0 || cfgData.dstDma[index].memBuf.base_pAddr == 0) {
                        LOG_ERR("index(%d)",index);
                        LOG_ERR("dst memAddr is 0,VA(0x%lx),PA(0x%lx)",(unsigned long)cfgData.dstDma[index].memBuf.base_vAddr,(unsigned long)cfgData.dstDma[index].memBuf.base_pAddr);
                        LOG_ERR("dst, W(%lu),H(%lu),W_c(%lu),H_c(%lu),W_v(%lu),H_v(%lu)",cfgData.dstDma[index].size.w,
                                                                                    cfgData.dstDma[index].size.h,
                                                                                    cfgData.dstDma[index].size_c.w,
                                                                                    cfgData.dstDma[index].size_c.h,
                                                                                    cfgData.dstDma[index].size_v.w,
                                                                                    cfgData.dstDma[index].size_v.h);

                        LOG_ERR("stride(%lu),stride_c(%lu),stride_v(%lu)",cfgData.dstDma[index].size.stride,
                                                                       cfgData.dstDma[index].size_c.stride,
                                                                       cfgData.dstDma[index].size_v.stride);
                        return MDPMGR_NULL_OBJECT;
                    }

                    dstSizeList[0] = cfgData.dstDma[index].size.h   * (cfgData.dstDma[index].size.stride);
                    dstSizeList[1] = cfgData.dstDma[index].size_c.h * (cfgData.dstDma[index].size_c.stride);
                    dstSizeList[2] = cfgData.dstDma[index].size_v.h * (cfgData.dstDma[index].size_v.stride);

                    dstVirList[0] = (MVOID *)cfgData.dstDma[index].memBuf.base_vAddr;
                    dstVirList[1] = (MVOID *)cfgData.dstDma[index].memBuf_c.base_vAddr;
                    dstVirList[2] = (MVOID *)cfgData.dstDma[index].memBuf_v.base_vAddr;

                    dstPhyList[0] = cfgData.dstDma[index].memBuf.base_pAddr;
                    dstPhyList[1] = cfgData.dstDma[index].memBuf_c.base_pAddr;
                    dstPhyList[2] = cfgData.dstDma[index].memBuf_v.base_pAddr;

                    if ((cfgData.isSecureFra == 1) && (cfgData.dstDma[index].secureTag != 0))
                    {
                            secdstPhyList[0] = (uintptr_t)dstPhyList[0];
                            secdstPhyList[1] = (uintptr_t)dstPhyList[1];
                            secdstPhyList[2] = (uintptr_t)dstPhyList[2];
                        //err2 = pDpStream->queueDstBuffer(index, dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                            err2 = pDpStream->queueDstBuffer(index, (void **)secdstPhyList, dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    }
                    else if (dpStreamtype == MDPMGR_DPSTREAM_TYPE_VENC)
                    {
                        err2 = pDpStream->queueDstBuffer(index, &dstVirList[0],&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    }
                    else
                    {
                        err2 = pDpStream->queueDstBuffer(index,&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    }

                    if(err2 != DP_STATUS_RETURN_SUCCESS)
                    {
                        LOG_ERR("queueDstBuffer fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }
                }
            }
        }
    }
    else
    {
        pDpStream->setPortType(ISP_ONLY_OUT_TPIPE, MDPMGR_ISP_ONLY_TPIPE);
    }

    if(p2tpipedumpEnable == 1)
    {
        dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
    }


    if (cfgData.m_iDipMetEn)
    {
        isp2mdpcfg.MET_String = cfgData.ispTpipeCfgInfo.drvinfo.m_pMetLogBuf;
        isp2mdpcfg.MET_String_length = cfgData.ispTpipeCfgInfo.drvinfo.m_MetLogBufSize;
        err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, &isp2mdpcfg, 0);
    }

    if (cfgData.isSecureFra)
    {
        // Normal world info
        #if 0
        isp2mdpcfg.secIspData.CqDesPA = cfgData.ispTpipeCfgInfo.drvinfo.DesCqPa;
        isp2mdpcfg.secIspData.CqDesVA = cfgData.ispTpipeCfgInfo.drvinfo.DesCqVa;
        isp2mdpcfg.secIspData.CqDesSize = cfgData.ispTpipeCfgInfo.drvinfo.DesCqSize;
        isp2mdpcfg.secIspData.CqVirtPA = cfgData.ispTpipeCfgInfo.drvinfo.VirtRegPa;
        isp2mdpcfg.secIspData.CqVirtVA = cfgData.ispTpipeCfgInfo.drvinfo.VirtRegVa;
        isp2mdpcfg.secIspData.CqVirtSize = DIP_REG_RANGE;
        isp2mdpcfg.secIspData.TpipePA = cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa;
        isp2mdpcfg.secIspData.TpipeVA = (uint64_t)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa;
        isp2mdpcfg.secIspData.TpipeSize = MAX_ISP_TILE_TDR_HEX_NO;
        #endif
        isp2mdpcfg.secIspData.ispBufs[0].va = cfgData.ispTpipeCfgInfo.drvinfo.DesCqVa;
        isp2mdpcfg.secIspData.ispBufs[0].size = cfgData.ispTpipeCfgInfo.drvinfo.DesCqSize;
        isp2mdpcfg.secIspData.ispBufs[1].va = cfgData.ispTpipeCfgInfo.drvinfo.VirtRegVa;
        isp2mdpcfg.secIspData.ispBufs[1].size = DIP_REG_RANGE;
        isp2mdpcfg.secIspData.ispBufs[2].va = (uint64_t)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa;
        isp2mdpcfg.secIspData.ispBufs[2].size = MAX_ISP_TILE_TDR_HEX_NO;
        // BPCI(IMGBI)
        if ((cfgData.ispTpipeCfgInfo.drvinfo.dip_imgbi_secure_tag != 0) & (cfgData.ispTpipeCfgInfo.top.bnr2_en == 1))
        {
            isp2mdpcfg.secIspData.ispBufs[3].va = cfgData.ispTpipeCfgInfo.drvinfo.dip_imgbi_base_vaddr;
            isp2mdpcfg.secIspData.ispBufs[3].size = cfgData.ispTpipeCfgInfo.drvinfo.dip_imgbi_size[0];
            isp2mdpcfg.secIspData.BpciHandle = cfgData.ispTpipeCfgInfo.drvinfo.dip_imgbi_base_addr;
        }
        else
        {
            isp2mdpcfg.secIspData.ispBufs[3].va = 0;
            isp2mdpcfg.secIspData.ispBufs[3].size = 0;
            isp2mdpcfg.secIspData.BpciHandle = 0;
        }
        // LSCI(IMGCI)
        if ((cfgData.ispTpipeCfgInfo.drvinfo.dip_imgci_secure_tag != 0) & (cfgData.ispTpipeCfgInfo.top.lsc2_en == 1))
        {
            isp2mdpcfg.secIspData.ispBufs[4].va = cfgData.ispTpipeCfgInfo.drvinfo.dip_imgci_base_vaddr;
            isp2mdpcfg.secIspData.ispBufs[4].size = cfgData.ispTpipeCfgInfo.drvinfo.dip_imgci_size[0];
            isp2mdpcfg.secIspData.LsciHandle = cfgData.ispTpipeCfgInfo.drvinfo.dip_imgci_base_addr;
        }
        else
        {
            isp2mdpcfg.secIspData.ispBufs[4].va = 0;
            isp2mdpcfg.secIspData.ispBufs[4].size = 0;
            isp2mdpcfg.secIspData.LsciHandle = 0;
        }
        // LCEI
        if (cfgData.ispTpipeCfgInfo.drvinfo.dip_lcei_secure_tag != 0)
        {
            isp2mdpcfg.secIspData.ispBufs[5].va = cfgData.ispTpipeCfgInfo.drvinfo.dip_lcei_base_vaddr;
            isp2mdpcfg.secIspData.ispBufs[5].size = cfgData.ispTpipeCfgInfo.drvinfo.dip_lcei_size[0];
            isp2mdpcfg.secIspData.LceiHandle = cfgData.ispTpipeCfgInfo.drvinfo.dip_lcei_base_addr;
        }
        else
        {
            isp2mdpcfg.secIspData.ispBufs[5].va = 0;
            isp2mdpcfg.secIspData.ispBufs[5].size = 0;
            isp2mdpcfg.secIspData.LceiHandle = 0;
        }
        // DEPI
        if (cfgData.ispTpipeCfgInfo.drvinfo.dip_depi_secure_tag != 0)
        {
            isp2mdpcfg.secIspData.ispBufs[6].va = cfgData.ispTpipeCfgInfo.drvinfo.dip_depi_base_vaddr;
            isp2mdpcfg.secIspData.ispBufs[6].size = cfgData.ispTpipeCfgInfo.drvinfo.dip_depi_size[0];
            isp2mdpcfg.secIspData.DepiHandle = cfgData.ispTpipeCfgInfo.drvinfo.dip_depi_base_addr;
        }
        else
        {
            isp2mdpcfg.secIspData.ispBufs[6].va = 0;
            isp2mdpcfg.secIspData.ispBufs[6].size = 0;
            isp2mdpcfg.secIspData.DepiHandle = 0;
        }
        // DMGI
        if (cfgData.ispTpipeCfgInfo.drvinfo.dip_dmgi_secure_tag != 0)
        {
            isp2mdpcfg.secIspData.ispBufs[7].va = cfgData.ispTpipeCfgInfo.drvinfo.dip_dmgi_base_vaddr;
            isp2mdpcfg.secIspData.ispBufs[7].size = cfgData.ispTpipeCfgInfo.drvinfo.dip_dmgi_size[0];
            isp2mdpcfg.secIspData.DmgiHandle = cfgData.ispTpipeCfgInfo.drvinfo.dip_dmgi_base_addr;
        }
        else
        {
            isp2mdpcfg.secIspData.ispBufs[7].va = 0;
            isp2mdpcfg.secIspData.ispBufs[7].size = 0;
            isp2mdpcfg.secIspData.DmgiHandle = 0;
        }

        // Secure world info
        isp2mdpcfg.secIspData.CqSecHandle = cfgData.ispTpipeCfgInfo.drvinfo.cqSecHdl;
        isp2mdpcfg.secIspData.CqSecSize = cfgData.ispTpipeCfgInfo.drvinfo.cqSecSize;
        isp2mdpcfg.secIspData.CqDesOft = cfgData.ispTpipeCfgInfo.drvinfo.DesCqOft;
        isp2mdpcfg.secIspData.CqVirtOft = cfgData.ispTpipeCfgInfo.drvinfo.VirtRegOft;
        isp2mdpcfg.secIspData.TpipeSecHandle = cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableSecHdl;
        isp2mdpcfg.secIspData.TpipeSecSize = cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableSecSize;
        isp2mdpcfg.secIspData.TpipeOft = cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableOft;
        LOG_INF("isp2mdpcfg(secIspData): CqDesVA(0x%lx), CqDesSize(0x%lx), CqVirtVA(0x%lx), CqVirtSize(0x%lx), TpipeVA(0x%lx), TpipeSize(0x%lx),\n\
            CqSecHandle(0x%lx), CqSecSize(0x%x), CqDesOft(0x%x), CqVirtOft(0x%x), TpipeSecHandle(0x%lx), TpipeSecSize(0x%x), TpipeOft(0x%x),\n\
            BpciVA(0x%lx), BpciSize(0x%lx), BpciHdl(0x%lx), LsciVA(0x%lx), LsciSize(0x%lx), LsciHdl(0x%lx), LceiVA(0x%lx), LceiSize(0x%lx), LceiHdl(0x%lx), DepiVA(0x%lx), DepiSize(0x%lx), DepiHdl(0x%lx), DmgiVA(0x%lx), DmgiSize(0x%lx), DmgiHdl(0x%lx)",\
            (unsigned long)isp2mdpcfg.secIspData.ispBufs[0].va, (unsigned long)isp2mdpcfg.secIspData.ispBufs[0].size, (unsigned long)isp2mdpcfg.secIspData.ispBufs[1].va, (unsigned long)isp2mdpcfg.secIspData.ispBufs[1].size, (unsigned long)isp2mdpcfg.secIspData.ispBufs[2].va, (unsigned long)isp2mdpcfg.secIspData.ispBufs[2].size, \
            (unsigned long)isp2mdpcfg.secIspData.CqSecHandle, isp2mdpcfg.secIspData.CqSecSize, isp2mdpcfg.secIspData.CqDesOft, isp2mdpcfg.secIspData.CqVirtOft, (unsigned long)isp2mdpcfg.secIspData.TpipeSecHandle, isp2mdpcfg.secIspData.TpipeSecSize, isp2mdpcfg.secIspData.TpipeOft, \
            (unsigned long)isp2mdpcfg.secIspData.ispBufs[3].va, (unsigned long)isp2mdpcfg.secIspData.ispBufs[3].size, (unsigned long)isp2mdpcfg.secIspData.BpciHandle, \
            (unsigned long)isp2mdpcfg.secIspData.ispBufs[4].va, (unsigned long)isp2mdpcfg.secIspData.ispBufs[4].size, (unsigned long)isp2mdpcfg.secIspData.LsciHandle, \
            (unsigned long)isp2mdpcfg.secIspData.ispBufs[5].va, (unsigned long)isp2mdpcfg.secIspData.ispBufs[5].size, (unsigned long)isp2mdpcfg.secIspData.LceiHandle, \
            (unsigned long)isp2mdpcfg.secIspData.ispBufs[6].va, (unsigned long)isp2mdpcfg.secIspData.ispBufs[6].size, (unsigned long)isp2mdpcfg.secIspData.DepiHandle, \
            (unsigned long)isp2mdpcfg.secIspData.ispBufs[7].va, (unsigned long)isp2mdpcfg.secIspData.ispBufs[7].size, (unsigned long)isp2mdpcfg.secIspData.DmgiHandle);
        #if 0 // For debug print
        MUINT32 *pCqDesVA;
        MUINT32 *pCqVirtVA;

        pCqDesVA = (MUINT32 *)isp2mdpcfg.secIspData.ispBufs[0].va;
        pCqVirtVA = (MUINT32 *)isp2mdpcfg.secIspData.ispBufs[1].va;

        for (MINT DesIdx = 0;DesIdx<(isp2mdpcfg.secIspData.ispBufs[0].size/32); DesIdx++)
        {
            LOG_INF("[CqDesc](%02d)-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x\n",DesIdx,
                pCqDesVA[DesIdx*8+0],pCqDesVA[DesIdx*8+1],pCqDesVA[DesIdx*8+2],pCqDesVA[DesIdx*8+3],
                pCqDesVA[DesIdx*8+4],pCqDesVA[DesIdx*8+5],pCqDesVA[DesIdx*8+6],pCqDesVA[DesIdx*8+7]);
        }

        for (MINT VirtIdx = 0;VirtIdx<(isp2mdpcfg.secIspData.ispBufs[1].size/32); VirtIdx++)
        {
            LOG_INF("[CqVirt](%02d)-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x\n",VirtIdx,
                pCqVirtVA[VirtIdx*8+0],pCqVirtVA[VirtIdx*8+1],pCqVirtVA[VirtIdx*8+2],pCqVirtVA[VirtIdx*8+3],
                pCqVirtVA[VirtIdx*8+4],pCqVirtVA[VirtIdx*8+5],pCqVirtVA[VirtIdx*8+6],pCqVirtVA[VirtIdx*8+7]);
        }
        #endif
        err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, &isp2mdpcfg, 0);
    }

    if(p2disableSkipTpipe)
    {
        if((cfgData.ispTpipeCfgInfo.drvinfo.frameflag == 0x0) || (cfgData.ispTpipeCfgInfo.drvinfo.frameflag == 0x1))
        {
            LOG_DBG("temp force frameflag 0");
            err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, 0);
        }
        else
        {
            err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, cfgData.ispTpipeCfgInfo.drvinfo.frameflag);
        }
    }
    else
    {
        err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, cfgData.ispTpipeCfgInfo.drvinfo.frameflag);
    }
    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("setParameter fail(%d)",err);
        return MDPMGR_API_FAIL;
    }

    //====== Start DpIspStream ======
    //LOG_INF("startStream");
    MUINT32 a=0,b=0;
    a=getUs();
    err2 = pDpStream->startStream();
    b=getUs();
    //update tdri buffer after mdp operation to cover the situation that isp do not update but mdp update

    if(p2disableSkipTpipe)
    {
        LOG_INF("+sceID(%u),cqPa(0x%lx)-Va(0x%lx),tpipVa(0x%lx)-Pa(0x%lx),isWaitB(%d),venc(%d),\
        srcFt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),plane(%d),Stmtype(%d)\n\
        wdma_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        wrot_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        jpg_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        pipe_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        HMyo CQ(%d_%d_%d_%d),tpipe(va:0x%lx,dstpa:0x%lx),\n\
        skip(0x%x),flag(0x%x),startStm(%d us),WDPQ(0x%8x),WRPQ(0x%8x)",\
                cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.DesCqPa,cfgData.ispTpipeCfgInfo.drvinfo.DesCqVa,(unsigned long)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
                cfgData.isWaitBuf,cfgData.dstPortCfg[ISP_MDP_DL_VENCO],\
                cfgData.mdpSrcFmt,cfgData.mdpSrcW,cfgData.mdpSrcH,cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,cfgData.mdpSrcBufSize,cfgData.mdpSrcCBufSize,cfgData.mdpSrcVBufSize,cfgData.srcVirAddr,cfgData.srcPhyAddr,\
                cfgData.mdpSrcPlaneNum,dpStreamtype,
                cfgData.dstPortCfg[ISP_MDP_DL_WDMAO],ISP_MDP_DL_WDMAO, cfgData.dstDma[ISP_MDP_DL_WDMAO].capbility, cfgData.dstDma[ISP_MDP_DL_WDMAO].Format, cfgData.dstDma[ISP_MDP_DL_WDMAO].Rotation, cfgData.dstDma[ISP_MDP_DL_WDMAO].Flip,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_WDMAO].mdpPortIdx+cfgData.dstDma[ISP_MDP_DL_WDMAO].crop_group,
                cfgData.dstPortCfg[ISP_MDP_DL_WROTO],ISP_MDP_DL_WROTO, cfgData.dstDma[ISP_MDP_DL_WROTO].capbility, cfgData.dstDma[ISP_MDP_DL_WROTO].Format, cfgData.dstDma[ISP_MDP_DL_WROTO].Rotation, cfgData.dstDma[ISP_MDP_DL_WROTO].Flip,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_WROTO].mdpPortIdx+cfgData.dstDma[ISP_MDP_DL_WROTO].crop_group,
                cfgData.dstPortCfg[ISP_MDP_DL_VENCO],ISP_MDP_DL_VENCO, cfgData.dstDma[ISP_MDP_DL_VENCO].capbility, cfgData.dstDma[ISP_MDP_DL_VENCO].Format, cfgData.dstDma[ISP_MDP_DL_VENCO].Rotation, cfgData.dstDma[ISP_MDP_DL_VENCO].Flip,
                cfgData.dstDma[ISP_MDP_DL_VENCO].size.w,cfgData.dstDma[ISP_MDP_DL_VENCO].size.h,cfgData.dstDma[ISP_MDP_DL_VENCO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_VENCO].size_c.w,cfgData.dstDma[ISP_MDP_DL_VENCO].size_c.h,cfgData.dstDma[ISP_MDP_DL_VENCO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_VENCO].size_v.w,cfgData.dstDma[ISP_MDP_DL_VENCO].size_v.h,cfgData.dstDma[ISP_MDP_DL_VENCO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_VENCO].mdpPortIdx,
                cfgData.dstPortCfg[ISP_ONLY_OUT_TPIPE],ISP_ONLY_OUT_TPIPE, cfgData.dstDma[ISP_ONLY_OUT_TPIPE].capbility, cfgData.dstDma[ISP_ONLY_OUT_TPIPE].Format, cfgData.dstDma[ISP_ONLY_OUT_TPIPE].Rotation, cfgData.dstDma[ISP_ONLY_OUT_TPIPE].Flip,
                cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size.w,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size.h,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size.stride,
                cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_c.w,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_c.h,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_c.stride,
                cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_v.w,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_v.h,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_v.stride,
                mdpPortMapping[ISP_ONLY_OUT_TPIPE].mdpPortIdx,
                cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx, cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx, cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx,
                (unsigned long)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa,
                p2disableSkipTpipe, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, b-a, WDMAPQEnable, WROTPQEnable);

    }
    else
    {
        LOG_INF("+sceID(%u),cqPa(0x%lx)-Va(0x%lx),tpipVa(0x%lx)-Pa(0x%lx),isWaitB(%d),venc(%d),\
        srcFt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),plane(%d),Stmtype(%d),secTag(%d),\n\
        wdma_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),secTag(%d),\n\
        wrot_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),secTag(%d),\n\
        jpg_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        pipe_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        HMyo CQ(%d_%d_%d_%d),tpipe(va:0x%lx,dstpa:0x%lx),\n\
        skip(0x%x),flag(0x%x),startStm(%d us),WDPQ(0x%8x),WRPQ(0x%8x)",\
                cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.DesCqPa,cfgData.ispTpipeCfgInfo.drvinfo.DesCqVa,(unsigned long)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
                cfgData.isWaitBuf,cfgData.dstPortCfg[ISP_MDP_DL_VENCO],\
                cfgData.mdpSrcFmt,cfgData.mdpSrcW,cfgData.mdpSrcH,cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,cfgData.mdpSrcBufSize,cfgData.mdpSrcCBufSize,cfgData.mdpSrcVBufSize,cfgData.srcVirAddr,cfgData.srcPhyAddr,\
                cfgData.mdpSrcPlaneNum,dpStreamtype,cfgData.srcSecureTag,
                cfgData.dstPortCfg[ISP_MDP_DL_WDMAO],ISP_MDP_DL_WDMAO, cfgData.dstDma[ISP_MDP_DL_WDMAO].capbility, cfgData.dstDma[ISP_MDP_DL_WDMAO].Format, cfgData.dstDma[ISP_MDP_DL_WDMAO].Rotation, cfgData.dstDma[ISP_MDP_DL_WDMAO].Flip,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_WDMAO].mdpPortIdx+cfgData.dstDma[ISP_MDP_DL_WDMAO].crop_group,cfgData.dstDma[ISP_MDP_DL_WDMAO].secureTag,
                cfgData.dstPortCfg[ISP_MDP_DL_WROTO],ISP_MDP_DL_WROTO, cfgData.dstDma[ISP_MDP_DL_WROTO].capbility, cfgData.dstDma[ISP_MDP_DL_WROTO].Format, cfgData.dstDma[ISP_MDP_DL_WROTO].Rotation, cfgData.dstDma[ISP_MDP_DL_WROTO].Flip,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_WROTO].mdpPortIdx+cfgData.dstDma[ISP_MDP_DL_WROTO].crop_group,cfgData.dstDma[ISP_MDP_DL_WROTO].secureTag,
                cfgData.dstPortCfg[ISP_MDP_DL_VENCO],ISP_MDP_DL_VENCO, cfgData.dstDma[ISP_MDP_DL_VENCO].capbility, cfgData.dstDma[ISP_MDP_DL_VENCO].Format, cfgData.dstDma[ISP_MDP_DL_VENCO].Rotation, cfgData.dstDma[ISP_MDP_DL_VENCO].Flip,
                cfgData.dstDma[ISP_MDP_DL_VENCO].size.w,cfgData.dstDma[ISP_MDP_DL_VENCO].size.h,cfgData.dstDma[ISP_MDP_DL_VENCO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_VENCO].size_c.w,cfgData.dstDma[ISP_MDP_DL_VENCO].size_c.h,cfgData.dstDma[ISP_MDP_DL_VENCO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_VENCO].size_v.w,cfgData.dstDma[ISP_MDP_DL_VENCO].size_v.h,cfgData.dstDma[ISP_MDP_DL_VENCO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_VENCO].mdpPortIdx,
                cfgData.dstPortCfg[ISP_ONLY_OUT_TPIPE],ISP_ONLY_OUT_TPIPE, cfgData.dstDma[ISP_ONLY_OUT_TPIPE].capbility, cfgData.dstDma[ISP_ONLY_OUT_TPIPE].Format, cfgData.dstDma[ISP_ONLY_OUT_TPIPE].Rotation, cfgData.dstDma[ISP_ONLY_OUT_TPIPE].Flip,
                cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size.w,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size.h,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size.stride,
                cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_c.w,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_c.h,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_c.stride,
                cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_v.w,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_v.h,cfgData.dstDma[ISP_ONLY_OUT_TPIPE].size_v.stride,
                mdpPortMapping[ISP_ONLY_OUT_TPIPE].mdpPortIdx,
                cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx, cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx,
                (unsigned long)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa,
                p2disableSkipTpipe, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, b-a, WDMAPQEnable, WROTPQEnable);

    }
    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("startStream fail(%d)",err);
        LOG_ERR("===dump tpipe structure start===");
        dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
        LOG_ERR("===dump tpipe structure end===");
        return MDPMGR_API_FAIL;
    }

    //vss the same with prv
    //if (NSImageio::NSIspio::eDrvScenario_VSS != cfgData.drvScenario)
    {
        // cut off the previous stream
        err2 = pDpStream->stopStream();
        if(err2 != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("stopStream fail(%d)",err);
            LOG_ERR("===dump tpipe structure start===");
            dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
            LOG_ERR("===dump tpipe structure end===");
            return MDPMGR_API_FAIL;
        }
    }
    LOG_DBG("-");
    return MDPMGR_NO_ERROR;
}


/**************************************************************************
*
**************************************************************************/
template <typename T>
MINT32 _stopMdp(T* pDpStream)
{
    DP_STATUS_ENUM err = DP_STATUS_RETURN_SUCCESS;
    //====== Stop DpIspStream ======
    err = pDpStream->stopStream();
    if(err != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("stopStream fail(%d)",err);
        return MDPMGR_API_FAIL;
    }
    return MDPMGR_NO_ERROR;
}

/**************************************************************************
*
**************************************************************************/
template <typename T>
MINT32 _dequeueBuf(MDPMGR_DEQUEUE_INDEX deqIndex, MDPMGR_CFG_STRUCT &cfgData,T* pDpStream)
{
    MINT32 ret = MDPMGR_NO_ERROR;
    MINT32 mdpRet = DP_STATUS_RETURN_SUCCESS;
    LOG_DBG(" + deqIndex(%d)",deqIndex);
    MVOID *deqBuf[3];
    MUINT32 a=0,b=0;
    a=getUs();
    //====== Dequeue Buffer ======
    switch(deqIndex){
        case MDPMGR_DEQ_SRC:
            mdpRet = pDpStream->dequeueSrcBuffer();
            break;
        case MDPMGR_DEQ_WDMA:
            if(cfgData.dstPortCfg[ISP_MDP_DL_WDMAO])
                mdpRet = pDpStream->dequeueDstBuffer((MINT32)ISP_MDP_DL_WDMAO, &deqBuf[0], cfgData.isWaitBuf);
            break;
        case MDPMGR_DEQ_WROT:
            if(cfgData.dstPortCfg[ISP_MDP_DL_WROTO])
                mdpRet = pDpStream->dequeueDstBuffer((MINT32)ISP_MDP_DL_WROTO, &deqBuf[0], cfgData.isWaitBuf);
            break;
        case MDPMGR_DEQ_IMGXO:
            if(cfgData.dstPortCfg[ISP_ONLY_OUT_TPIPE])
                mdpRet = pDpStream->dequeueDstBuffer((MINT32)ISP_ONLY_OUT_TPIPE, &deqBuf[0], cfgData.isWaitBuf);
            break;
        case MDPMGR_DEQ_JPEG:   //MDPMGR_DEQ_VENCO
            if(cfgData.dstPortCfg[ISP_MDP_DL_JPEGO])
                mdpRet = pDpStream->dequeueDstBuffer((MINT32)ISP_MDP_DL_JPEGO, &deqBuf[0], cfgData.isWaitBuf);
            break;
        default:
            LOG_ERR("[Error]deqIndex(%d) error",deqIndex);
            ret = MDPMGR_API_FAIL;
            goto EXIT;
            //break;
    };
    b=getUs();
    if(mdpRet == DP_STATUS_RETURN_SUCCESS){
        ret = MDPMGR_NO_ERROR;
    } else if(mdpRet == DP_STATUS_BUFFER_EMPTY) {
        ret = MDPMGR_VSS_NOT_READY;
    } else {
        LOG_ERR("[Error]dequeueBuffer deqIndex(%d) fail(%d)",deqIndex,mdpRet);
        if(cfgData.ispTpipeCfgInfo.top.wpe_en == 1){
            dumpWpeTPipeInfo(cfgData.ispTpipeCfgInfo, MTRUE);
        }
        else
        {
            dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
        }
        ret = MDPMGR_DPFRAMEWORK_FAIL;
    }

    if ( deqIndex != MDPMGR_DEQ_SRC)
    {
        LOG_INF("+,cqIndx(%u/%u/%u/%u),sceID(%u),deqIndex(%d),isWaitBuf(%d),\n\
            deqIndex(%d), dequeTime(%d us)",\
            cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,\
            cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx, cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx,cfgData.drvScenario,deqIndex,cfgData.isWaitBuf,\
            deqIndex, b-a);
    }


    LOG_DBG(" - deqIndex(%d)",deqIndex);
EXIT:
    return ret;
}


/**************************************************************************
*
**************************************************************************/
template <typename T>
MINT32 _dequeueFrameEnd(MDPMGR_CFG_STRUCT &cfgData,T* pDpStream)
{
    DP_STATUS_ENUM ret;
    //
    //vss the same with prv
    #if 0
    if (NSImageio::NSIspio::eDrvScenario_VSS == cfgData.drvScenario)
    {
        // cut off the previous stream
        ret = pDpStream->stopStream();
        if(ret != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("startStream fail(%d)",ret);
            return MDPMGR_API_FAIL;
        }
    }
    #endif

    // cut off the previous stream
    ret = pDpStream->dequeueFrameEnd();
    if(ret != DP_STATUS_RETURN_SUCCESS){
        LOG_ERR("[Error]dequeueFrameEnd fail(%d)",ret);
        return MDPMGR_API_FAIL;
    }

    return ret;
}


/**************************************************************************
*
**************************************************************************/
MdpMgrImp::MdpMgrImp()
            :MdpMgr()
            ,mInitCount(0)
            ,mTileSelMode(0)
            ,mwpeInitCount(0)
{
    LOG_INF("+");


    LOG_INF("-");
}

/**************************************************************************
*
**************************************************************************/
MdpMgrImp::~MdpMgrImp()
{
    LOG_INF("+");

    Mutex::Autolock lock(mLock);


    LOG_INF("-");
}

/**************************************************************************
*
**************************************************************************/
MdpMgr *MdpMgr::createInstance( void )
{
    LOG_INF("+");

    DBG_LOG_CONFIG(imageio, mdpmgr);
    static MdpMgrImp singleton;

    LOG_INF("-");
    return &singleton;
}

/**************************************************************************
*
**************************************************************************/
MVOID MdpMgr::destroyInstance( void )
{

}

/**************************************************************************
*
**************************************************************************/
MINT32 MdpMgrImp::init()
{
    LOG_INF("+,mInitCount(%d)",mInitCount);
    Mutex::Autolock lock(mLock);
    //
    MBOOL err = MDPMGR_NO_ERROR;
    //
    if(mInitCount == 0) {
        //isp stream
            m_DpVssIspStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
            for(MINT32 i = 0; i < DIP_RING_BUFFER_CQ_SIZE; i++) {
                for(MINT32 j = 0; j < MAX_CMDQ_RING_BUFFER_SIZE_NUM; j++) {
                    m_pDpStream[i][j] = allocDpStream(i);
                    pthread_mutex_lock(&mMutex_DpStreamMap);
                    mDpStreamMap[i][j].refCqIdx=static_cast<E_ISP_DIP_CQ>(i);
                    mDpStreamMap[i][j].refRingBufIdx=j;
                    pthread_mutex_unlock(&mMutex_DpStreamMap);
                }
        }
        //venc stream for slow motion
        m_DpVencStream=new DpVEncStream(DpVEncStream::ISP_VR_STREAM);    //CC Scenario with main sensor
    }

    android_atomic_inc(&mInitCount);

EXIT:
    LOG_INF("-,mInitCount(%d)",mInitCount);

    return err;
}

/**************************************************************************
*
**************************************************************************/
MINT32 MdpMgrImp::uninit()
{
    LOG_DBG("+,mInitCount(%d)",mInitCount);
    //
    Mutex::Autolock lock(mLock);
    //
    MBOOL err = MDPMGR_NO_ERROR;
    //
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0) {
        err = MDPMGR_STILL_USERS;
        goto EXIT;
    }

    if(mInitCount == 0) {
        //isp stream
            for(MINT32 i = 0; i < DIP_RING_BUFFER_CQ_SIZE; i++) {
                for(MINT32 j = 0; j < MAX_CMDQ_RING_BUFFER_SIZE_NUM; j++) {
                    if (i != ISP_DRV_DIP_CQ_THRE1) {
                        delete m_pDpStream[i][j];
                    }
                    m_pDpStream[i][j] = NULL;
                }
            }
        delete m_DpVssIspStream;
        m_DpVssIspStream = NULL;

        //venc stream for slow motion
        delete m_DpVencStream;
        m_DpVencStream=NULL;
    }

EXIT:
    LOG_DBG("-,mInitCount(%d)",mInitCount);
    return err;
}

/**************************************************************************
*
**************************************************************************/
MINT32 MdpMgrImp::startMdp(MDPMGR_CFG_STRUCT &cfgData)
{
    MINT32 err = MDPMGR_NO_ERROR;
    DpIspStream *pDpStream = NULL;
    LOG_DBG("+,cqIndx(%u),sceID(%u),cqPA(0x%lx),tpipVa(0x%lx),tpipPa(0x%lx),isWaitBuf(%d),venc(%d)", \
            cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.DesCqPa,(unsigned long)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
            cfgData.isWaitBuf,cfgData.dstPortCfg[ISP_MDP_DL_VENCO]);
    //_startMdp
    //NOTE, cuz we do not support ISP_MDP_DL_JPEGO anymore, we use ISP_MDP_DL_VENCO to judge we enable venc port or not
    if(cfgData.dstPortCfg[ISP_MDP_DL_VENCO] == 1)
    {
        DpVEncStream *pDpStream = NULL;
        DpIspStream *pDpStream_jpgcfg = NULL;
        pDpStream = selectDpVEncStream();
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpVencStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        err=_startMdp(MDPMGR_DPSTREAM_TYPE_VENC,cfgData,pDpStream);
    }

	else if (cfgData.ispTpipeCfgInfo.sw.tpipe_sel_mode == 0x10 && cfgData.ispTpipeCfgInfo.top.wpe_en == 1) //for WPE
	{
		DpIspStream *pDpStream = NULL;
		pDpStream = selectWpeStream(cfgData.drvScenario, cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe);
		if(pDpStream == NULL)
        {
            LOG_ERR("pWpeStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        err= _wpestartMdp(cfgData,pDpStream);
	}

    else
    {
        DpIspStream *pDpStream = NULL;
        DpVEncStream *pDpStream_venccfg = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx);
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpIspStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        err=_startMdp(MDPMGR_DPSTREAM_TYPE_ISP,cfgData,pDpStream);
    }
    LOG_DBG("-,");
    return err;
}

/**************************************************************************
*
**************************************************************************/
MINT32 MdpMgrImp::stopMdp(MDPMGR_CFG_STRUCT &cfgData)
{
    MINT32 err = MDPMGR_NO_ERROR;
    MUINT32 sceID = cfgData.drvScenario;


    LOG_INF("+,cqIndx(%u),sceID(%u)",cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,sceID);

    //====== Select Correct DpIspStream ======
    //NOTE, cuz we do not support ISP_MDP_DL_JPEGO anymore, we use ISP_MDP_DL_VENCO to judge we enable venc port or not
    if(cfgData.dstPortCfg[ISP_MDP_DL_VENCO] == 1)
    {
        DpVEncStream *pDpStream = NULL;
        pDpStream = selectDpVEncStream();
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        //stop stream
        err=_stopMdp(pDpStream);
    }

	else if (cfgData.ispTpipeCfgInfo.sw.tpipe_sel_mode == 0x10 && cfgData.ispTpipeCfgInfo.top.wpe_en == 1) //for WPE
	{
		DpIspStream *pDpStream = NULL;
		pDpStream = selectWpeStream(cfgData.drvScenario, cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe);
		if(pDpStream == NULL)
        {
            LOG_ERR("pWpeStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        err= _stopMdp(pDpStream);
	}
    else
    {
        DpIspStream *pDpStream = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx);
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        //stop stream
        err=_stopMdp(pDpStream);
    }

    return err;
}

/**************************************************************************
*
**************************************************************************/
MINT32 MdpMgrImp::dequeueBuf(MDPMGR_DEQUEUE_INDEX deqIndex, MDPMGR_CFG_STRUCT &cfgData)
{
    MINT32 ret = MDPMGR_NO_ERROR;

    LOG_DBG("+,cqIndx(%u/%u/%u),sceID(%u),deqIndex(%d),isWaitBuf(%d)",cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,\
        cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.drvScenario,deqIndex,cfgData.isWaitBuf);

    //====== Select Correct DpIspStream ======
    //NOTE, cuz we do not support ISP_MDP_DL_JPEGO anymore, we use ISP_MDP_DL_VENCO to judge we enable venc port or not
    if(cfgData.dstPortCfg[ISP_MDP_DL_VENCO] == 1)
    {
        DpVEncStream *pDpStream = NULL;
        pDpStream = selectDpVEncStream();
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpStream is NULL");
            ret = MDPMGR_NULL_OBJECT;
            goto EXIT;
        }
        //deque buffer
        ret=_dequeueBuf(deqIndex,cfgData,pDpStream);
    }
	else if (cfgData.ispTpipeCfgInfo.sw.tpipe_sel_mode == 0x10 && cfgData.ispTpipeCfgInfo.top.wpe_en == 1) //for WPE
	{
        if(cfgData.ispTpipeCfgInfo.top.wpe_mdpcrop_en != 1)
			 goto EXIT;

		DpIspStream *pDpStream = NULL;
		pDpStream = selectWpeStream(cfgData.drvScenario, cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe);
		if(pDpStream == NULL)
        {
            LOG_ERR("pWpeStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
         //deque buffer
        ret=_dequeueBuf(deqIndex,cfgData,pDpStream);
	}
    else
    {
        DpIspStream *pDpStream = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx);
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpStream is NULL");
            ret = MDPMGR_NULL_OBJECT;
            goto EXIT;
        }
        //deque buffer
        ret=_dequeueBuf(deqIndex,cfgData,pDpStream);
    }
EXIT:
    //
    return ret;
}

MINT32 MdpMgrImp::dequeueFrameEnd(MDPMGR_CFG_STRUCT &cfgData)
{
    MINT32 ret = MDPMGR_NO_ERROR;
    //
    LOG_DBG("+,");
    //
    //NOTE, cuz we do not support ISP_MDP_DL_JPEGO anymore, we use ISP_MDP_DL_VENCO to judge we enable venc port or not
    if(cfgData.dstPortCfg[ISP_MDP_DL_VENCO] == 1)
    {
        DpVEncStream *pDpStream = NULL;
        pDpStream = selectDpVEncStream();
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        //
        ret=_dequeueFrameEnd(cfgData,pDpStream);
    }

	else if (cfgData.ispTpipeCfgInfo.sw.tpipe_sel_mode == 0x10 && cfgData.ispTpipeCfgInfo.top.wpe_en == 1) //for WPE
	{
		DpIspStream *pDpStream = NULL;
		pDpStream = selectWpeStream(cfgData.drvScenario, cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe);
		if(pDpStream == NULL)
        {
            LOG_ERR("pWpeStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        ret = _dequeueFrameEnd(cfgData,pDpStream);
        if(ret != MDPMGR_NO_ERROR){
            LOG_ERR("[Error]dequeueBuffer deqIndex(%d) fail(%d)",cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe,ret);
            dumpWpeTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
        }
	}
    else
    {
        DpIspStream *pDpStream = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx);
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        //
        ret=_dequeueFrameEnd(cfgData,pDpStream);
    }
    //
    LOG_DBG("-,");
    return ret;
}



/**************************************************************************
*
**************************************************************************/
DpIspStream *MdpMgrImp::allocDpStream(MUINT32 cqIdx)
{
    //LOG_DBG("+,cqIdx(%d)",cqIdx);
    DpIspStream *pRetDpIspStream = NULL;

    switch(cqIdx)
    {
        case ISP_DRV_DIP_CQ_THRE1:
//        case ISP_DRV_DIP_CQ_THRE11 :
            //pRetDpIspStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
            pRetDpIspStream = this->m_DpVssIspStream;
            break;
        default :
            pRetDpIspStream = new DpIspStream(DpIspStream::ISP_CC_STREAM);
            break;
    }

    //LOG_DBG("-,pRetDpIspStream(0x%x)",pRetDpIspStream);
    return pRetDpIspStream;

}



/**************************************************************************
*
**************************************************************************/
DpIspStream *MdpMgrImp::selectDpIspStream(MUINT32 sceID, MUINT32 cqIdx, MUINT32 dupCqIdx, MUINT32 burstCqIdx, MUINT32 ringBufIdx)
{
    LOG_DBG("+,cqIndx(%d_%d_%d),sceID(%d)",cqIdx, dupCqIdx, burstCqIdx, sceID);

    Mutex::Autolock lock(mLock);

    DpIspStream *pRetDpIspStream = NULL;

    MUINT32 ObjectCqIdx=cqIdx;
    MUINT32 ObjectRingBufIdx=ringBufIdx;

    pthread_mutex_lock(&mMutex_DpStreamMap);
    ObjectCqIdx=mDpStreamMap[cqIdx][ringBufIdx].refCqIdx;
    ObjectRingBufIdx=mDpStreamMap[cqIdx][ringBufIdx].refRingBufIdx;
    pthread_mutex_unlock(&mMutex_DpStreamMap);

    if(m_pDpStream[ObjectCqIdx][ObjectRingBufIdx]){
        pRetDpIspStream = m_pDpStream[ObjectCqIdx][ObjectRingBufIdx];
        LOG_DBG("[selectDpIspStream] cqIdx(%d/%d/%d/%d_%d/%d),sceID(%d)",cqIdx,dupCqIdx, burstCqIdx, ringBufIdx, ObjectCqIdx, ObjectRingBufIdx,sceID);
    } else {
        LOG_ERR("[Error]Null point cqIdx(%d/%d/%d/%d_%d/%d),sceID(%d)",cqIdx,dupCqIdx, burstCqIdx, ringBufIdx, ObjectCqIdx, ObjectRingBufIdx,sceID);
    }


    return pRetDpIspStream;

}

/**************************************************************************
*
**************************************************************************/
DpVEncStream *MdpMgrImp::selectDpVEncStream()
{
    LOG_DBG("+");

    Mutex::Autolock lock(mLock);

    return m_DpVencStream;
}

/**************************************************************************
*
**************************************************************************/
MINT32 MdpMgrImp::getJPEGSize(MDPMGR_CFG_STRUCT cfgData)
{
    MUINT32 jpgfilesize;
    DpIspStream *pDpStream = NULL;

    //====== Select Correct DpIspStream ======
    pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx);
    if(pDpStream == NULL)
    {
        LOG_ERR("pDpStream is NULL");
        return MDPMGR_NULL_OBJECT;
    }
    pDpStream->getJPEGFilesize(&jpgfilesize);
    return jpgfilesize;
}

/**************************************************************************
*
**************************************************************************/
MBOOL MdpMgrImp::startVencLink(MINT32 fps, MINT32 wd,MINT32 ht)
{
    bool ret=MDPMGR_NO_ERROR;
    DpVEncStream *pDpStream = NULL;
    //====== Select Correct DpIspStream ======
    pDpStream = selectDpVEncStream();
    if(pDpStream == NULL)
    {
        LOG_ERR("pDpStream is NULL");
        return MDPMGR_NULL_OBJECT;
    }

    //set
    LOG_INF("StartVR w/h(%d,%d),fps(%d)",wd,ht,fps);
    ret=pDpStream->startVideoRecord(wd,ht,fps);
    if(ret !=MDPMGR_NO_ERROR)
    {
        LOG_ERR("startVideoRecord fail ret(%d)",ret);
    }
    return ret;
}

/**************************************************************************
*
**************************************************************************/
MBOOL MdpMgrImp::stopVencLink()
{
    bool ret=MDPMGR_NO_ERROR;
    DpVEncStream *pDpStream = NULL;
    //====== Select Correct DpIspStream ======
    pDpStream = selectDpVEncStream();
    if(pDpStream == NULL)
    {
        LOG_ERR("pDpStream is NULL");
        return MDPMGR_NULL_OBJECT;
    }

    //set
    LOG_INF("StopVR");
    ret=pDpStream->stopVideoRecord();
    if(ret !=MDPMGR_NO_ERROR)
    {
        LOG_ERR("stopVideoRecord fail ret(%d)",ret);
    }
    return ret;
}

/**************************************************************************
*
**************************************************************************/
MBOOL MdpMgrImp::updateDpStreamMap(MUINT32 flag, MUINT32 cqIdx,MUINT32 ringbufIdx,MUINT32 refringbufidx)
{
    bool ret=MTRUE;
    E_ISP_DIP_CQ refObjectCqIdx = ISP_DRV_DIP_CQ_THRE0; //for selecting dpStream
    MUINT32 refObjectRingBufIdx = 0; //for selecting dpStream


    pthread_mutex_lock(&mMutex_DpStreamMap);
    if (flag == 1)
    {
        refObjectCqIdx = mDpStreamMap[cqIdx][refringbufidx].refCqIdx;
        refObjectRingBufIdx = mDpStreamMap[cqIdx][refringbufidx].refRingBufIdx;

        mDpStreamMap[cqIdx][ringbufIdx].refCqIdx=static_cast<E_ISP_DIP_CQ>(refObjectCqIdx);
        mDpStreamMap[cqIdx][ringbufIdx].refRingBufIdx=refObjectRingBufIdx;
    }
    else
    {
        mDpStreamMap[cqIdx][ringbufIdx].refCqIdx=static_cast<E_ISP_DIP_CQ>(cqIdx);
        mDpStreamMap[cqIdx][ringbufIdx].refRingBufIdx=ringbufIdx;
    }
        pthread_mutex_unlock(&mMutex_DpStreamMap);
    LOG_INF("refObjectCqIdx:%d, refObjectRingBufIdx:%d,cqIdx:%d,ringIdx:%d,refringbufidx:%d\n",refObjectCqIdx,refObjectRingBufIdx, cqIdx, ringbufIdx, refringbufidx);
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////
//general public function used for template function
/**************************************************************************
*
**************************************************************************/
MINT32 DpColorFmtConvert(EImageFormat ispImgFmt,DpColorFormat *dpColorFormat)
{
    MINT32 ret = MDPMGR_NO_ERROR;
    DpColorFormat localDpColorFormat = DP_COLOR_BAYER8;

    LOG_DBG("+,ispImgFmt(%d)",ispImgFmt);

    switch(ispImgFmt)
    {
        case eImgFmt_STA_BYTE:
        case eImgFmt_UFO_FG:
        case eImgFmt_Y800:  //  0x040000,   /*!< Y plane only  */
            localDpColorFormat = DP_COLOR_GREY;
            break;
        case eImgFmt_NV16:  // 0x8000,  422 format, 2 plane
            localDpColorFormat = DP_COLOR_NV16;
            break;
        case eImgFmt_NV21:  // 0x0010, 420 format, 2 plane (VU)
            localDpColorFormat = DP_COLOR_NV21;
            break;
        case eImgFmt_NV12:  // 0x0040, 420 format, 2 plane (UV)
            localDpColorFormat = DP_COLOR_NV12;
            break;
        case eImgFmt_YV12:  // 0x00008, 420 format, 3 plane (YVU)
            localDpColorFormat = DP_COLOR_YV12;
            break;
        case eImgFmt_I420:  // 0x20000, 420 format, 3 plane(YUV)
            localDpColorFormat = DP_COLOR_I420;
            break;
        case eImgFmt_YV16:  // 0x4000, 422 format, 3 plane
            localDpColorFormat = DP_COLOR_YV16;
            break;
        case eImgFmt_I422:  // 0x4000, 422 format, 3 plane
            localDpColorFormat = DP_COLOR_I422;
            break;
        case eImgFmt_YUY2:  // 0x0100,  422 format, 1 plane (YUYV)
            localDpColorFormat = DP_COLOR_YUYV;
            break;
        case eImgFmt_UYVY:  // 0x0200, 422 format, 1 plane
            localDpColorFormat = DP_COLOR_UYVY;
            break;
        case eImgFmt_VYUY:  // 0x100000, 422 format, 1 plane
            localDpColorFormat = DP_COLOR_VYUY;
            break;
        case eImgFmt_YVYU:  // 0x080000, 422 format, 1 plane
            localDpColorFormat = DP_COLOR_VYUY;
            break;
        case eImgFmt_RGB565:    // 0x0400, RGB 565 (16-bit), 1 plane
            localDpColorFormat = DP_COLOR_RGB565;
            break;
        case eImgFmt_RGB888:    // 0x0800, RGB 888 (24-bit), 1 plane
            localDpColorFormat = DP_COLOR_RGB888;
            break;
        case eImgFmt_ARGB888:   // 0x1000, ARGB (32-bit), 1 plane
            localDpColorFormat = DP_COLOR_ARGB8888;
            break;
        case eImgFmt_BAYER8:    // 0x0001,  Bayer format, 8-bit
            localDpColorFormat = DP_COLOR_BAYER8;
            break;
        case eImgFmt_BAYER10:   // 0x0002,  Bayer format, 10-bit
        case eImgFmt_BAYER10_MIPI:   // 0x0002,  Bayer format, 10-bit(MIPI)
        case eImgFmt_UFO_BAYER10:
            localDpColorFormat = DP_COLOR_BAYER10;
            break;
        case eImgFmt_BAYER12:   // 0x0004,  Bayer format, 12-bit
        case eImgFmt_UFO_BAYER12:
            localDpColorFormat = DP_COLOR_BAYER12;
            break;
        case eImgFmt_NV21_BLK:  // 0x0020, 420 format block mode, 2 plane (UV)
            localDpColorFormat = DP_COLOR_420_BLKP;
            break;
        case eImgFmt_NV12_BLK:  // 0x0080,  420 format block mode, 2 plane (VU)
            localDpColorFormat = DP_COLOR_420_BLKI;
            break;
        case eImgFmt_FG_BAYER8:
            localDpColorFormat = DP_COLOR_FULLG8;
            break;
        case eImgFmt_FG_BAYER10:
        case eImgFmt_UFO_FG_BAYER10:
            localDpColorFormat = DP_COLOR_FULLG10;
            break;
        case eImgFmt_FG_BAYER12:
        case eImgFmt_UFO_FG_BAYER12:
            localDpColorFormat = DP_COLOR_FULLG12;
            break;
        case eImgFmt_FG_BAYER14:
            localDpColorFormat = DP_COLOR_FULLG14;
            break;
        case eImgFmt_RGB48:
            localDpColorFormat = DP_COLOR_RGB48;
            break;
        case eImgFmt_BAYER8_UNPAK:
            localDpColorFormat = DP_COLOR_BAYER8_UNPAK;
            break;
        case eImgFmt_BAYER10_UNPAK:
            localDpColorFormat = DP_COLOR_BAYER10_UNPAK;
            break;
        case eImgFmt_BAYER12_UNPAK:
            localDpColorFormat = DP_COLOR_BAYER12_UNPAK;
            break;
        case eImgFmt_BAYER14_UNPAK:
            localDpColorFormat = DP_COLOR_BAYER14_UNPAK;
            break;
        // Unified format
        default:
            LOG_ERR("wrong format(%d)",ispImgFmt);
            ret = MDPMGR_WRONG_PARAM;
         break;
    }

    *dpColorFormat = localDpColorFormat;

    LOG_DBG("-,dpColorFormat(0x%x)",*dpColorFormat);
    return ret;
}

/**************************************************************************
*
**************************************************************************/
MINT32 DpDmaOutColorFmtConvert(MdpRotDMACfg dma_out,DpColorFormat *dpColorFormat)
{
    MBOOL ret = MDPMGR_NO_ERROR;
    DpColorFormat localDpColorFormat = DP_COLOR_YUYV;

    LOG_DBG("+,dma,Fmt(%d),Plane(%d),uv_plane_swap(%d)",dma_out.Format,
                                                                      dma_out.Plane,
                                                                      dma_out.uv_plane_swap);
    switch(dma_out.Format)
    {
        case eImgFmt_I422:
            localDpColorFormat = DP_COLOR_I422; // 422,3P
            break;
        case eImgFmt_YV16:
            localDpColorFormat = DP_COLOR_YV16; // 422,3P
            break;
        case eImgFmt_NV16:
            localDpColorFormat = DP_COLOR_NV16; // 422,2p
            break;
        case eImgFmt_NV61:
            localDpColorFormat = DP_COLOR_NV61; //422,2P
            break;
        case eImgFmt_YUY2:
            localDpColorFormat = DP_COLOR_YUYV; // 422,1p
            break;
        case eImgFmt_UYVY:
            localDpColorFormat = DP_COLOR_UYVY; // 422,1p
            break;
        case eImgFmt_VYUY:
            localDpColorFormat = DP_COLOR_VYUY; // 422,1p
            break;
        case eImgFmt_YVYU:
            localDpColorFormat = DP_COLOR_YVYU; // 422,1p
            break;
        case eImgFmt_YV12:
            if(dma_out.uv_plane_swap == 1)
            {
                localDpColorFormat = DP_COLOR_YV12;
            }
            else
            {
                localDpColorFormat = DP_COLOR_I420;
            }
            break;
        case eImgFmt_NV21:
            localDpColorFormat= DP_COLOR_NV21;  //420_2P_YVYU;
            break;
        case eImgFmt_NV12:
            localDpColorFormat= DP_COLOR_NV12;  //_420_2P_YUYV
            break;
        case eImgFmt_JPG_I420:
            localDpColorFormat = DP_COLOR_I420;
            break;
        case eImgFmt_JPG_I422:
            localDpColorFormat = DP_COLOR_I422;
            break;
        case  eImgFmt_STA_BYTE:
            localDpColorFormat = DP_COLOR_GREY;
            break;
        case eImgFmt_RGB888:
            localDpColorFormat = DP_COLOR_RGB888;
            break;
        case eImgFmt_RGB565:
            localDpColorFormat = DP_COLOR_RGB565;
            break;
        case eImgFmt_ARGB8888:
            localDpColorFormat = DP_COLOR_XRGB8888;
            break;
        case eImgFmt_RGBA8888:
            localDpColorFormat = DP_COLOR_RGBX8888;
            break;
        case eImgFmt_BAYER8:
            localDpColorFormat = eBAYER8;
            break;
        case eImgFmt_BAYER10:
            localDpColorFormat = eBAYER10;
            break;
        case eImgFmt_BAYER12:
            localDpColorFormat = eBAYER12;
            break;
        default:
            LOG_ERR("wrong format(%d)",dma_out.Format);
            localDpColorFormat = DP_COLOR_YUYV;
            ret = MDPMGR_WRONG_PARAM;
        break;
    }

    *dpColorFormat = localDpColorFormat;

    LOG_DBG("-,dpColorFormat(0x%x)",*dpColorFormat);
    return ret;
}

/**************************************************************************
*
**************************************************************************/
MVOID dumpIspTPipeInfo(ISP_TPIPE_CONFIG_STRUCT a_info, MBOOL dump_en)
{
    if(dump_en)
    {
        #if 1
        LOG_ERR("sw.log_en           (%d)"                            ,a_info.sw.log_en           );
        LOG_ERR("sw.src_width        (%d)"                            ,a_info.sw.src_width        );
        LOG_ERR("sw.src_height       (%d)"                            ,a_info.sw.src_height       );
        LOG_ERR("sw.tpipe_width      (%d)"                            ,a_info.sw.tpipe_width      );
        LOG_ERR("sw.tpipe_height     (%d)"                            ,a_info.sw.tpipe_height     );
        LOG_ERR("sw.tpipe_irq_mode   (%d)"                            ,a_info.sw.tpipe_irq_mode   );
        LOG_ERR("sw.tpipe_sel_mode   (%d)"                            ,a_info.sw.tpipe_sel_mode   );
        LOG_ERR("top.pixel_id        (%d)"                            ,a_info.top.pixel_id        );
        LOG_ERR("top.cam_in_fmt      (%d)"                            ,a_info.top.cam_in_fmt      );
        LOG_ERR("top.ctl_extension_en(%d)"                            ,a_info.top.ctl_extension_en);
        LOG_ERR("top.fg_mode        (%d)"                            ,a_info.top.fg_mode         );
        LOG_ERR("top.ufo_imgi_en     (%d)"                            ,a_info.top.ufo_imgi_en     );
        LOG_ERR("top.ufdi_fmt       (%d)"                            ,a_info.top.ufdi_fmt        );
        LOG_ERR("top.vipi_fmt        (%d)"                            ,a_info.top.vipi_fmt        );
        LOG_ERR("top.img3o_fmt       (%d)"                            ,a_info.top.img3o_fmt       );
        LOG_ERR("top.img2o_fmt       (%d)"                            ,a_info.top.img2o_fmt       );
        LOG_ERR("top.pak2_fmt        (%d)"                            ,a_info.top.pak2_fmt        );
        LOG_ERR("top.adl_en          (%d)"                            ,a_info.top.adl_en          );
        LOG_ERR("top.imgi_en         (%d)"                            ,a_info.top.imgi_en         );
        LOG_ERR("top.imgbi_en        (%d)"                            ,a_info.top.imgbi_en        );
        LOG_ERR("top.imgci_en        (%d)"                            ,a_info.top.imgci_en        );
        LOG_ERR("top.ufdi_en         (%d)"                            ,a_info.top.ufdi_en         );
        LOG_ERR("top.unp_en          (%d)"                            ,a_info.top.unp_en          );
        LOG_ERR("top.ufd_en          (%d)"                            ,a_info.top.ufd_en          );
        LOG_ERR("top.ufd_crop_en     (%d)"                            ,a_info.top.ufd_crop_en     );
        LOG_ERR("top.bnr_en          (%d)"                            ,a_info.top.bnr_en          );
        LOG_ERR("top.lsc2_en         (%d)"                            ,a_info.top.lsc2_en         );
        LOG_ERR("top.rcp2_en         (%d)"                            ,a_info.top.rcp2_en         );
        LOG_ERR("top.pak2_en         (%d)"                            ,a_info.top.pak2_en         );
        LOG_ERR("top.c24_en          (%d)"                            ,a_info.top.c24_en          );
        LOG_ERR("top.sl2_en          (%d)"                            ,a_info.top.sl2_en          );
        LOG_ERR("top.rnr_en          (%d)"                            ,a_info.top.rnr_en          );
        LOG_ERR("top.udm_en          (%d)"                            ,a_info.top.udm_en          );
        LOG_ERR("top.vipi_en         (%d)"                            ,a_info.top.vipi_en         );
        LOG_ERR("top.vip2i_en        (%d)"                            ,a_info.top.vip2i_en        );
        LOG_ERR("top.vip3i_en        (%d)"                            ,a_info.top.vip3i_en        );
        LOG_ERR("top.pak2o_en        (%d)"                            ,a_info.top.pak2o_en        );
        LOG_ERR("top.g2c_en          (%d)"                            ,a_info.top.g2c_en          );
        LOG_ERR("top.c42_en          (%d)"                            ,a_info.top.c42_en          );
        LOG_ERR("top.sl2b_en         (%d)"                            ,a_info.top.sl2b_en         );
        LOG_ERR("top.nbc_en          (%d)"                            ,a_info.top.nbc_en          );
        LOG_ERR("top.dmgi_en         (%d)"                            ,a_info.top.dmgi_en         );
        LOG_ERR("top.depi_en         (%d)"                            ,a_info.top.depi_en         );
        LOG_ERR("top.nbc2_en         (%d)"                            ,a_info.top.nbc2_en         );
        LOG_ERR("top.srz1_en         (%d)"                            ,a_info.top.srz1_en         );
        LOG_ERR("top.mix1_en         (%d)"                            ,a_info.top.mix1_en         );
        LOG_ERR("top.srz2_en         (%d)"                            ,a_info.top.srz2_en         );
        LOG_ERR("top.mix2_en         (%d)"                            ,a_info.top.mix2_en         );
        LOG_ERR("top.sl2c_en         (%d)"                            ,a_info.top.sl2c_en         );
        LOG_ERR("top.sl2d_en         (%d)"                            ,a_info.top.sl2d_en         );
        LOG_ERR("top.sl2e_en         (%d)"                            ,a_info.top.sl2e_en         );
        LOG_ERR("top.sl2g_en         (%d)"                            ,a_info.top.sl2g_en         );
        LOG_ERR("top.sl2h_en         (%d)"                            ,a_info.top.sl2h_en         );
        LOG_ERR("top.sl2i_en         (%d)"                            ,a_info.top.sl2i_en         );
        LOG_ERR("top.hfg_en          (%d)"                            ,a_info.top.hfg_en          );
        LOG_ERR("top.ndg_en          (%d)"                            ,a_info.top.ndg_en          );
        LOG_ERR("top.ndg2_en         (%d)"                            ,a_info.top.ndg2_en         );
        LOG_ERR("top.seee_en         (%d)"                            ,a_info.top.seee_en         );
        LOG_ERR("top.lcei_en         (%d)"                            ,a_info.top.lcei_en         );
        LOG_ERR("top.lce_en          (%d)"                            ,a_info.top.lce_en          );
        LOG_ERR("top.mix3_en         (%d)"                            ,a_info.top.mix3_en         );
        LOG_ERR("top.mix4_en         (%d)"                            ,a_info.top.mix4_en         );
        LOG_ERR("top.crz_en          (%d)"                            ,a_info.top.crz_en          );
        LOG_ERR("top.img2o_en        (%d)"                            ,a_info.top.img2o_en        );
        LOG_ERR("top.img2bo_en       (%d)"                            ,a_info.top.img2bo_en       );
        LOG_ERR("top.fe_en           (%d)"                            ,a_info.top.fe_en           );
        LOG_ERR("top.feo_en          (%d)"                            ,a_info.top.feo_en          );
        LOG_ERR("top.c02_en          (%d)"                            ,a_info.top.c02_en          );
        LOG_ERR("top.c02b_en         (%d)"                            ,a_info.top.c02b_en         );
        LOG_ERR("top.nr3d_en         (%d)"                            ,a_info.top.nr3d_en         );
        LOG_ERR("top.color_en        (%d)"                            ,a_info.top.color_en        );
        LOG_ERR("top.crsp_en         (%d)"                            ,a_info.top.crsp_en         );
        LOG_ERR("top.img3o_en        (%d)"                            ,a_info.top.img3o_en        );
        LOG_ERR("top.img3bo_en       (%d)"                            ,a_info.top.img3bo_en       );
        LOG_ERR("top.img3co_en       (%d)"                            ,a_info.top.img3co_en       );
        LOG_ERR("top.c24b_en         (%d)"                            ,a_info.top.c24b_en         );
        LOG_ERR("top.mdp_crop_en     (%d)"                            ,a_info.top.mdp_crop_en     );
        LOG_ERR("top.mdp_crop2_en    (%d)"                            ,a_info.top.mdp_crop2_en    );
        LOG_ERR("top.plnr1_en        (%d)"                            ,a_info.top.plnr1_en        );
        LOG_ERR("top.plnr2_en        (%d)"                            ,a_info.top.plnr2_en        );
        LOG_ERR("top.plnw1_en        (%d)"                            ,a_info.top.plnw1_en        );
        LOG_ERR("top.plnw2_en        (%d)"                            ,a_info.top.plnw2_en        );
        LOG_ERR("top.adbs2_en        (%d)"                            ,a_info.top.adbs2_en        );
        LOG_ERR("top.dbs2_en         (%d)"                            ,a_info.top.dbs2_en         );
        LOG_ERR("top.obc2_en         (%d)"                            ,a_info.top.obc2_en         );
        LOG_ERR("top.rmm2_en         (%d)"                            ,a_info.top.rmm2_en         );
        LOG_ERR("top.rmg2_en         (%d)"                            ,a_info.top.rmg2_en         );
        LOG_ERR("top.gdr1_en         (%d)"                            ,a_info.top.gdr1_en         );
        LOG_ERR("top.gdr2_en         (%d)"                            ,a_info.top.gdr2_en         );
        LOG_ERR("top.bnr2_en         (%d)"                            ,a_info.top.bnr2_en         );
        LOG_ERR("top.fm_en           (%d)"                            ,a_info.top.fm_en           );
        LOG_ERR("top.srz3_en         (%d)"                            ,a_info.top.srz3_en         );
        LOG_ERR("top.srz4_en         (%d)"                            ,a_info.top.srz4_en         );
        LOG_ERR("top.g2g2_en         (%d)"                            ,a_info.top.g2g2_en         );
        LOG_ERR("top.ggm2_en         (%d)"                            ,a_info.top.ggm2_en         );
        LOG_ERR("top.wsync_en        (%d)"                            ,a_info.top.wsync_en        );
        LOG_ERR("top.wshift_en       (%d)"                            ,a_info.top.wshift_en       );
        LOG_ERR("top.interlace_mode  (%d)"                            ,a_info.top.interlace_mode  );
        LOG_ERR("top.pgn_sel         (%d)"                            ,a_info.top.pgn_sel         );
        LOG_ERR("top.g2g_sel         (%d)"                            ,a_info.top.g2g_sel         );
        LOG_ERR("top.g2c_sel         (%d)"                            ,a_info.top.g2c_sel         );
        LOG_ERR("top.srz1_sel        (%d)"                            ,a_info.top.srz1_sel        );
        LOG_ERR("top.mix1_sel        (%d)"                            ,a_info.top.mix1_sel        );
        LOG_ERR("top.crz_sel         (%d)"                            ,a_info.top.crz_sel         );
        LOG_ERR("top.nr3d_sel        (%d)"                            ,a_info.top.nr3d_sel        );
        LOG_ERR("top.fe_sel          (%d)"                            ,a_info.top.fe_sel          );
        LOG_ERR("top.mdp_sel         (%d)"                            ,a_info.top.mdp_sel         );
        LOG_ERR("top.pca_en          (%d)"                            ,a_info.top.pca_en          );
        LOG_ERR("top.pgn_en          (%d)"                            ,a_info.top.pgn_en          );
        LOG_ERR("top.g2g_en          (%d)"                            ,a_info.top.g2g_en          );
        LOG_ERR("top.flc_en          (%d)"                            ,a_info.top.flc_en          );
        LOG_ERR("top.flc2_en         (%d)"                            ,a_info.top.flc2_en         );
        LOG_ERR("top.ggm_en          (%d)"                            ,a_info.top.ggm_en          );
        LOG_ERR("top.dcpn2_en        (%d)"                            ,a_info.top.dcpn2_en        );
        LOG_ERR("top.cpn2_en         (%d)"                            ,a_info.top.cpn2_en         );
        LOG_ERR("top.nbc_sel         (%d)"                            ,a_info.top.nbc_sel         );
        LOG_ERR("top.rcp2_sel        (%d)"                            ,a_info.top.rcp2_sel        );
        LOG_ERR("top.pak2o_sel       (%d)"                            ,a_info.top.pak2o_sel       );
        LOG_ERR("top.crsp_sel        (%d)"                            ,a_info.top.crsp_sel        );
        LOG_ERR("top.imgi_sel        (%d)"                            ,a_info.top.imgi_sel        );
        LOG_ERR("top.ggm_sel         (%d)"                            ,a_info.top.ggm_sel         );
        LOG_ERR("top.wpe_sel         (%d)"                            ,a_info.top.wpe_sel         );
        LOG_ERR("top.feo_sel         (%d)"                            ,a_info.top.feo_sel         );
        LOG_ERR("top.g2g2_sel        (%d)"                            ,a_info.top.g2g2_sel        );
        LOG_ERR("top.nbc_gmap_ltm_mode(%d)"                           ,a_info.top.nbc_gmap_ltm_mode);
        LOG_ERR("top.wuv_mode        (%d)"                            ,a_info.top.wuv_mode        );
        LOG_ERR("top.pakg2_en        (%d)"                            ,a_info.top.pakg2_en        );
        LOG_ERR("top.smx1_en         (%d)"                            ,a_info.top.smx1_en         );
        LOG_ERR("top.smx1i_en        (%d)"                            ,a_info.top.smx1i_en        );
        LOG_ERR("top.smx1o_en        (%d)"                            ,a_info.top.smx1o_en        );
        LOG_ERR("top.smx2_en         (%d)"                            ,a_info.top.smx2_en         );
        LOG_ERR("top.smx2i_en        (%d)"                            ,a_info.top.smx2i_en        );
        LOG_ERR("top.smx2o_en        (%d)"                            ,a_info.top.smx2o_en        );
        LOG_ERR("top.smx3_en         (%d)"                            ,a_info.top.smx3_en         );
        LOG_ERR("top.smx3i_en        (%d)"                            ,a_info.top.smx3i_en        );
        LOG_ERR("top.smx3o_en        (%d)"                            ,a_info.top.smx3o_en        );
        LOG_ERR("top.smx4_en         (%d)"                            ,a_info.top.smx4_en         );
        LOG_ERR("top.smx4i_en        (%d)"                            ,a_info.top.smx4i_en        );
        LOG_ERR("top.smx4o_en        (%d)"                            ,a_info.top.smx4o_en        );
        LOG_ERR("ufd.ufd_bs2_au_start(%d)"                            ,a_info.ufd.ufd_bs2_au_start);
        LOG_ERR("ufd.ufd_bond_mode   (%d)"                            ,a_info.ufd.ufd_bond_mode   );
        LOG_ERR("ufd.ufd_sel         (%d)"                            ,a_info.ufd.ufd_sel         );
        LOG_ERR("imgi.imgi_v_flip_en (%d)"                            ,a_info.imgi.imgi_v_flip_en );
        LOG_ERR("imgi.imgi_stride    (%d)"                            ,a_info.imgi.imgi_stride    );
        LOG_ERR("imgbi.imgbi_v_flip_en(%d)"                           ,a_info.imgbi.imgbi_v_flip_en);
        LOG_ERR("imgbi.imgbi_offset  (%d)"                            ,a_info.imgbi.imgbi_offset  );
        LOG_ERR("imgbi.imgbi_xsize   (%d)"                            ,a_info.imgbi.imgbi_xsize   );
        LOG_ERR("imgbi.imgbi_ysize   (%d)"                            ,a_info.imgbi.imgbi_ysize   );
        LOG_ERR("imgbi.imgbi_stride  (%d)"                            ,a_info.imgbi.imgbi_stride  );
        LOG_ERR("imgci.imgci_v_flip_en(%d)"                           ,a_info.imgci.imgci_v_flip_en);
        LOG_ERR("imgci.imgci_stride   (%d)"                           ,a_info.imgci.imgci_stride  );
        LOG_ERR("ufdi.ufdi_v_flip_en (%d)"                            ,a_info.ufdi.ufdi_v_flip_en );
        LOG_ERR("ufdi.ufdi_xsize     (%d)"                            ,a_info.ufdi.ufdi_xsize     );
        LOG_ERR("ufdi.ufdi_ysize     (%d)"                            ,a_info.ufdi.ufdi_ysize     );
        LOG_ERR("ufdi.ufdi_stride    (%d)"                            ,a_info.ufdi.ufdi_stride    );
        LOG_ERR("bnr.bpc_en          (%d)"                            ,a_info.bnr.bpc_en          );
        LOG_ERR("bnr.bpc_tbl_en      (%d)"                            ,a_info.bnr.bpc_tbl_en      );
        LOG_ERR("rmg.rmg_ihdr_en     (%d)"                            ,a_info.rmg.rmg_ihdr_en     );
        LOG_ERR("rmg.rmg_zhdr_en     (%d)"                            ,a_info.rmg.rmg_zhdr_en     );
        LOG_ERR("lsc2.extend_coef_mode(%d)"                           ,a_info.lsc2.extend_coef_mode);
        LOG_ERR("lsc2.sdblk_width    (%d)"                            ,a_info.lsc2.sdblk_width    );
        LOG_ERR("lsc2.sdblk_xnum     (%d)"                            ,a_info.lsc2.sdblk_xnum     );
        LOG_ERR("lsc2.sdblk_last_width(%d)"                           ,a_info.lsc2.sdblk_last_width);
        LOG_ERR("lsc2.sdblk_height   (%d)"                            ,a_info.lsc2.sdblk_height   );
        LOG_ERR("lsc2.sdblk_ynum     (%d)"                            ,a_info.lsc2.sdblk_ynum     );
        LOG_ERR("lsc2.sdblk_last_height(%d)"                          ,a_info.lsc2.sdblk_last_height);
        LOG_ERR("sl2.sl2_hrz_comp    (%d)"                            ,a_info.sl2.sl2_hrz_comp    );
        LOG_ERR("sl2.sl2_vrz_comp    (%d)"                            ,a_info.sl2.sl2_vrz_comp    );
        LOG_ERR("udm.bayer_bypass    (%d)"                            ,a_info.udm.bayer_bypass    );
        LOG_ERR("vipi.vipi_v_flip_en (%d)"                            ,a_info.vipi.vipi_v_flip_en );
        LOG_ERR("vipi.vipi_xsize     (%d)"                            ,a_info.vipi.vipi_xsize     );
        LOG_ERR("vipi.vipi_ysize     (%d)"                            ,a_info.vipi.vipi_ysize     );
        LOG_ERR("vipi.vipi_stride    (%d)"                            ,a_info.vipi.vipi_stride    );
        LOG_ERR("vip2i.vip2i_v_flip_en(%d)"                           ,a_info.vip2i.vip2i_v_flip_en);
        LOG_ERR("vip2i.vip2i_xsize   (%d)"                            ,a_info.vip2i.vip2i_xsize   );
        LOG_ERR("vip2i.vip2i_ysize   (%d)"                            ,a_info.vip2i.vip2i_ysize   );
        LOG_ERR("vip2i.vip2i_stride  (%d)"                            ,a_info.vip2i.vip2i_stride  );
        LOG_ERR("vip3i.vip3i_v_flip_en(%d)"                           ,a_info.vip3i.vip3i_v_flip_en);
        LOG_ERR("vip3i.vip3i_xsize   (%d)"                            ,a_info.vip3i.vip3i_xsize   );
        LOG_ERR("vip3i.vip3i_ysize   (%d)"                            ,a_info.vip3i.vip3i_ysize   );
        LOG_ERR("vip3i.vip3i_stride  (%d)"                            ,a_info.vip3i.vip3i_stride  );
        LOG_ERR("mfb.bld_deblock_en  (%d)"                            ,a_info.mfb.bld_deblock_en  );
        LOG_ERR("mfb.bld_brz_en      (%d)"                            ,a_info.mfb.bld_brz_en      );
        LOG_ERR("mfb.bld_mbd_wt_en   (%d)"                            ,a_info.mfb.bld_mbd_wt_en   );
        LOG_ERR("pak2o.pak2o_stride  (%d)"                            ,a_info.pak2o.pak2o_stride  );
        LOG_ERR("pak2o.pak2o_xoffset (%d)"                            ,a_info.pak2o.pak2o_xoffset );
        LOG_ERR("pak2o.pak2o_yoffset (%d)"                            ,a_info.pak2o.pak2o_yoffset );
        LOG_ERR("pak2o.pak2o_xsize   (%d)"                            ,a_info.pak2o.pak2o_xsize   );
        LOG_ERR("pak2o.pak2o_ysize   (%d)"                            ,a_info.pak2o.pak2o_ysize   );
        LOG_ERR("g2c.g2c_shade_en    (%d)"                            ,a_info.g2c.g2c_shade_en    );
        LOG_ERR("g2c.g2c_shade_xmid  (%d)"                            ,a_info.g2c.g2c_shade_xmid  );
        LOG_ERR("g2c.g2c_shade_ymid  (%d)"                            ,a_info.g2c.g2c_shade_ymid  );
        LOG_ERR("g2c.g2c_shade_var   (%d)"                            ,a_info.g2c.g2c_shade_var   );
        LOG_ERR("sl2b.sl2b_hrz_comp  (%d)"                            ,a_info.sl2b.sl2b_hrz_comp  );
        LOG_ERR("sl2b.sl2b_vrz_comp  (%d)"                            ,a_info.sl2b.sl2b_vrz_comp  );
        LOG_ERR("nbc.anr_eny(%d)",                                   a_info.nbc.anr_eny);
        LOG_ERR("nbc.anr_enc(%d)",                                   a_info.nbc.anr_enc);
        LOG_ERR("nbc.anr_ltm_link(%d)",                              a_info.nbc.anr_ltm_link);
        LOG_ERR("nbc2.anr2_eny(%d)",                                 a_info.nbc2.anr2_eny);
        LOG_ERR("nbc2.anr2_enc(%d)",                                 a_info.nbc2.anr2_enc);
        LOG_ERR("nbc2.anr2_scale_mode(%d)",                          a_info.nbc2.anr2_scale_mode);
        LOG_ERR("nbc2.anr2_mode(%d)",                                a_info.nbc2.anr2_mode);
        LOG_ERR("nbc2.anr2_bok_mode(%d)",                            a_info.nbc2.anr2_bok_mode);
        LOG_ERR("nbc2.anr2_bok_pf_en(%d)",                           a_info.nbc2.anr2_bok_pf_en);
        LOG_ERR("nbc2.abf_en(%d)",                                   a_info.nbc2.abf_en);
        LOG_ERR("sl2c.sl2c_hrz_comp  (%d)"                            ,a_info.sl2c.sl2c_hrz_comp  );
        LOG_ERR("sl2c.sl2c_vrz_comp  (%d)"                            ,a_info.sl2c.sl2c_vrz_comp  );
        LOG_ERR("sl2d.sl2d_hrz_comp  (%d)"                            ,a_info.sl2d.sl2d_hrz_comp  );
        LOG_ERR("sl2d.sl2d_vrz_comp  (%d)"                            ,a_info.sl2d.sl2d_vrz_comp  );
        LOG_ERR("sl2e.sl2e_hrz_comp  (%d)"                            ,a_info.sl2e.sl2e_hrz_comp  );
        LOG_ERR("sl2e.sl2e_vrz_comp  (%d)"                            ,a_info.sl2e.sl2e_vrz_comp  );
        LOG_ERR("sl2g.sl2g_hrz_comp  (%d)"                            ,a_info.sl2g.sl2g_hrz_comp  );
        LOG_ERR("sl2g.sl2g_vrz_comp  (%d)"                            ,a_info.sl2g.sl2g_vrz_comp  );
        LOG_ERR("sl2h.sl2h_hrz_comp  (%d)"                            ,a_info.sl2h.sl2h_hrz_comp  );
        LOG_ERR("sl2h.sl2h_vrz_comp  (%d)"                            ,a_info.sl2h.sl2h_vrz_comp  );
        LOG_ERR("sl2i.sl2i_hrz_comp  (%d)"                            ,a_info.sl2i.sl2i_hrz_comp  );
        LOG_ERR("sl2i.sl2i_vrz_comp  (%d)"                            ,a_info.sl2i.sl2i_vrz_comp  );
        LOG_ERR("hfg.non_block_base  (%d)"                            ,a_info.hfg.non_block_base  );
        LOG_ERR("ndg.non_block_base  (%d)"                            ,a_info.ndg.non_block_base  );
        LOG_ERR("ndg2.non_block_base (%d)"                            ,a_info.ndg2.non_block_base );
        LOG_ERR("seee.se_edge        (%d)"                            ,a_info.seee.se_edge        );
        LOG_ERR("lcei.lcei_v_flip_en (%d)"                            ,a_info.lcei.lcei_v_flip_en );
        LOG_ERR("lcei.lcei_xsize     (%d)"                            ,a_info.lcei.lcei_xsize     );
        LOG_ERR("lcei.lcei_ysize     (%d)"                            ,a_info.lcei.lcei_ysize     );
        LOG_ERR("lcei.lcei_stride    (%d)"                            ,a_info.lcei.lcei_stride    );
        LOG_ERR("lce.lce_lc_tone     (%d)"                            ,a_info.lce.lce_lc_tone);
        LOG_ERR("lce.lce_bc_mag_kubnx(%d)"                            ,a_info.lce.lce_bc_mag_kubnx);
        LOG_ERR("lce.lce_slm_width   (%d)"                            ,a_info.lce.lce_slm_width);
        LOG_ERR("lce.lce_bc_mag_kubny(%d)"                            ,a_info.lce.lce_bc_mag_kubny);
        LOG_ERR("lce.lce_slm_height  (%d)"                            ,a_info.lce.lce_slm_height);
        LOG_ERR("lce.lce_full_xoff   (%d)"                            ,a_info.lce.lce_full_xoff);
        LOG_ERR("lce.lce_full_yoff   (%d)"                            ,a_info.lce.lce_full_yoff);
        LOG_ERR("lce.lce_full_slm_width(%d)"                          ,a_info.lce.lce_full_slm_width);
        LOG_ERR("lce.lce_full_slm_height(%d)"                         ,a_info.lce.lce_full_slm_height);
        LOG_ERR("lce.lce_full_out_height(%d)"                         ,a_info.lce.lce_full_out_height);
        LOG_ERR("cdrz.cdrz_input_crop_width(%d)"                      ,a_info.cdrz.cdrz_input_crop_width);
        LOG_ERR("cdrz.cdrz_input_crop_height(%d)"                     ,a_info.cdrz.cdrz_input_crop_height);
        LOG_ERR("cdrz.cdrz_output_width(%d)"                          ,a_info.cdrz.cdrz_output_width);
        LOG_ERR("cdrz.cdrz_output_height(%d)"                         ,a_info.cdrz.cdrz_output_height);
        LOG_ERR("cdrz.cdrz_luma_horizontal_integer_offset(%d)"        ,a_info.cdrz.cdrz_luma_horizontal_integer_offset);
        LOG_ERR("cdrz.cdrz_luma_horizontal_subpixel_offset(%d)"       ,a_info.cdrz.cdrz_luma_horizontal_subpixel_offset);
        LOG_ERR("cdrz.cdrz_luma_vertical_integer_offset(%d)"          ,a_info.cdrz.cdrz_luma_vertical_integer_offset);
        LOG_ERR("cdrz.cdrz_luma_vertical_subpixel_offset(%d)"         ,a_info.cdrz.cdrz_luma_vertical_subpixel_offset);
        LOG_ERR("cdrz.cdrz_horizontal_luma_algorithm(%d)"             ,a_info.cdrz.cdrz_horizontal_luma_algorithm);
        LOG_ERR("cdrz.cdrz_vertical_luma_algorithm(%d)"               ,a_info.cdrz.cdrz_vertical_luma_algorithm);
        LOG_ERR("cdrz.cdrz_horizontal_coeff_step(%d)"                 ,a_info.cdrz.cdrz_horizontal_coeff_step);
        LOG_ERR("cdrz.cdrz_vertical_coeff_step(%d)"                   ,a_info.cdrz.cdrz_vertical_coeff_step);
        LOG_ERR("img2o.img2o_stride  (%d)"                            ,a_info.img2o.img2o_stride);
        LOG_ERR("img2o.img2o_xoffset (%d)"                            ,a_info.img2o.img2o_xoffset);
        LOG_ERR("img2o.img2o_yoffset (%d)"                            ,a_info.img2o.img2o_yoffset);
        LOG_ERR("img2o.img2o_xsize   (%d)"                            ,a_info.img2o.img2o_xsize);
        LOG_ERR("img2o.img2o_ysize   (%d)"                            ,a_info.img2o.img2o_ysize);
        LOG_ERR("img2bo.img2bo_stride(%d)"                            ,a_info.img2bo.img2bo_stride);
        LOG_ERR("img2bo.img2bo_xoffset(%d)"                           ,a_info.img2bo.img2bo_xoffset);
        LOG_ERR("img2bo.img2bo_yoffset(%d)"                           ,a_info.img2bo.img2bo_yoffset);
        LOG_ERR("img2bo.img2bo_xsize(%d)"                             ,a_info.img2bo.img2bo_xsize);
        LOG_ERR("img2bo.img2bo_ysize(%d)"                             ,a_info.img2bo.img2bo_ysize);
        LOG_ERR("srz1.srz_input_crop_width(%d)",                   a_info.srz1.srz_input_crop_width);
        LOG_ERR("srz1.srz_input_crop_height(%d)",                  a_info.srz1.srz_input_crop_height);
        LOG_ERR("srz1.srz_output_width(%d)",                       a_info.srz1.srz_output_width);
        LOG_ERR("srz1.srz_output_height(%d)",                      a_info.srz1.srz_output_height);
        LOG_ERR("srz1.srz_luma_horizontal_integer_offset(%d)",     a_info.srz1.srz_luma_horizontal_integer_offset);
        LOG_ERR("srz1.srz_luma_horizontal_subpixel_offset(%d)",    a_info.srz1.srz_luma_horizontal_subpixel_offset);
        LOG_ERR("srz1.srz_luma_vertical_integer_offset(%d)",       a_info.srz1.srz_luma_vertical_integer_offset);
        LOG_ERR("srz1.srz_luma_vertical_subpixel_offset(%d)",      a_info.srz1.srz_luma_vertical_subpixel_offset);
        LOG_ERR("srz1.srz_horizontal_coeff_step(%d)",              a_info.srz1.srz_horizontal_coeff_step);
        LOG_ERR("srz1.srz_vertical_coeff_step(%d)",                a_info.srz1.srz_vertical_coeff_step);
        LOG_ERR("srz2.srz_input_crop_width(%d)",                   a_info.srz2.srz_input_crop_width);
        LOG_ERR("srz2.srz_input_crop_height(%d)",                  a_info.srz2.srz_input_crop_height);
        LOG_ERR("srz2.srz_output_width(%d)",                       a_info.srz2.srz_output_width);
        LOG_ERR("srz2.srz_output_height(%d)",                      a_info.srz2.srz_output_height);
        LOG_ERR("srz2.srz_luma_horizontal_integer_offset(%d)",     a_info.srz2.srz_luma_horizontal_integer_offset);
        LOG_ERR("srz2.srz_luma_horizontal_subpixel_offset(%d)",    a_info.srz2.srz_luma_horizontal_subpixel_offset);
        LOG_ERR("srz2.srz_luma_vertical_integer_offset(%d)",       a_info.srz2.srz_luma_vertical_integer_offset);
        LOG_ERR("srz2.srz_luma_vertical_subpixel_offset(%d)",      a_info.srz2.srz_luma_vertical_subpixel_offset);
        LOG_ERR("srz2.srz_horizontal_coeff_step(%d)",              a_info.srz2.srz_horizontal_coeff_step);
        LOG_ERR("srz2.srz_vertical_coeff_step(%d)",                a_info.srz2.srz_vertical_coeff_step);
        LOG_ERR("srz3.srz_input_crop_width(%d)",                   a_info.srz3.srz_input_crop_width);
        LOG_ERR("srz3.srz_input_crop_height(%d)",                  a_info.srz3.srz_input_crop_height);
        LOG_ERR("srz3.srz_output_width(%d)",                       a_info.srz3.srz_output_width);
        LOG_ERR("srz3.srz_output_height(%d)",                      a_info.srz3.srz_output_height);
        LOG_ERR("srz3.srz_luma_horizontal_integer_offset(%d)",     a_info.srz3.srz_luma_horizontal_integer_offset);
        LOG_ERR("srz3.srz_luma_horizontal_subpixel_offset(%d)",    a_info.srz3.srz_luma_horizontal_subpixel_offset);
        LOG_ERR("srz3.srz_luma_vertical_integer_offset(%d)",       a_info.srz3.srz_luma_vertical_integer_offset);
        LOG_ERR("srz3.srz_luma_vertical_subpixel_offset(%d)",      a_info.srz3.srz_luma_vertical_subpixel_offset);
        LOG_ERR("srz3.srz_horizontal_coeff_step(%d)",              a_info.srz3.srz_horizontal_coeff_step);
        LOG_ERR("srz3.srz_vertical_coeff_step(%d)",                a_info.srz3.srz_vertical_coeff_step);
        LOG_ERR("srz4.srz_input_crop_width(%d)",                   a_info.srz4.srz_input_crop_width);
        LOG_ERR("srz4.srz_input_crop_height(%d)",                  a_info.srz4.srz_input_crop_height);
        LOG_ERR("srz4.srz_output_width(%d)",                       a_info.srz4.srz_output_width);
        LOG_ERR("srz4.srz_output_height(%d)",                      a_info.srz4.srz_output_height);
        LOG_ERR("srz4.srz_luma_horizontal_integer_offset(%d)",     a_info.srz4.srz_luma_horizontal_integer_offset);
        LOG_ERR("srz4.srz_luma_horizontal_subpixel_offset(%d)",    a_info.srz4.srz_luma_horizontal_subpixel_offset);
        LOG_ERR("srz4.srz_luma_vertical_integer_offset(%d)",       a_info.srz4.srz_luma_vertical_integer_offset);
        LOG_ERR("srz4.srz_luma_vertical_subpixel_offset(%d)",      a_info.srz4.srz_luma_vertical_subpixel_offset);
        LOG_ERR("srz4.srz_horizontal_coeff_step(%d)",              a_info.srz4.srz_horizontal_coeff_step);
        LOG_ERR("srz4.srz_vertical_coeff_step(%d)",                a_info.srz4.srz_vertical_coeff_step);
        LOG_ERR("fe.fe_mode(%d)",                                  a_info.fe.fe_mode);
        LOG_ERR("feo.feo_stride(%d)",                              a_info.feo.feo_stride);
        LOG_ERR("nr3d.nr3d_on_en(%d)",                             a_info.nr3d.nr3d_on_en);
        LOG_ERR("nr3d.nr3d_on_xoffset(%d)",                        a_info.nr3d.nr3d_on_xoffset);
        LOG_ERR("nr3d.nr3d_on_yoffset(%d)",                        a_info.nr3d.nr3d_on_yoffset);
        LOG_ERR("nr3d.nr3d_on_width(%d)",                          a_info.nr3d.nr3d_on_width);
        LOG_ERR("nr3d.nr3d_on_height(%d)",                         a_info.nr3d.nr3d_on_height);
        LOG_ERR("crsp.crsp_ystep(%d)",                             a_info.crsp.crsp_ystep);
        LOG_ERR("crsp.crsp_xoffset(%d)",                           a_info.crsp.crsp_xoffset);
        LOG_ERR("crsp.crsp_yoffset(%d)",                           a_info.crsp.crsp_yoffset);
        LOG_ERR("img3o.img3o_stride(%d)",                          a_info.img3o.img3o_stride);
        LOG_ERR("img3o.img3o_xoffset(%d)",                         a_info.img3o.img3o_xoffset);
        LOG_ERR("img3o.img3o_yoffset(%d)",                         a_info.img3o.img3o_yoffset);
        LOG_ERR("img3o.img3o_xsize(%d)",                           a_info.img3o.img3o_xsize);
        LOG_ERR("img3o.img3o_ysize(%d)",                           a_info.img3o.img3o_ysize);
        LOG_ERR("img3bo.img3bo_stride(%d)",                        a_info.img3bo.img3bo_stride);
        LOG_ERR("img3bo.img3bo_xsize(%d)",                         a_info.img3bo.img3bo_xsize);
        LOG_ERR("img3bo.img3bo_ysize(%d)",                         a_info.img3bo.img3bo_ysize);
        LOG_ERR("img3co.img3co_stride(%d)",                        a_info.img3co.img3co_stride);
        LOG_ERR("img3co.img3co_xsize(%d)",                         a_info.img3co.img3co_xsize);
        LOG_ERR("img3co.img3co_ysize(%d)",                         a_info.img3co.img3co_ysize);
        LOG_ERR("dmgi.dmgi_v_flip_en(%d)",                         a_info.dmgi.dmgi_v_flip_en);
        LOG_ERR("dmgi.dmgi_stride(%d)",                            a_info.dmgi.dmgi_stride);
        LOG_ERR("dmgi.dmgi_offset(%d)",                            a_info.dmgi.dmgi_offset);
        LOG_ERR("dmgi.dmgi_xsize(%d)",                             a_info.dmgi.dmgi_xsize);
        LOG_ERR("dmgi.dmgi_ysize(%d)",                             a_info.dmgi.dmgi_ysize);
        LOG_ERR("depi.depi_v_flip_en(%d)",                         a_info.depi.depi_v_flip_en);
        LOG_ERR("depi.depi_stride(%d)",                            a_info.depi.depi_stride);
        LOG_ERR("depi.depi_xsize(%d)",                             a_info.depi.depi_xsize);
        LOG_ERR("depi.depi_ysize(%d)",                             a_info.depi.depi_ysize);
        LOG_ERR("pca.pca_cfc_en(%d)",                              a_info.pca.pca_cfc_en);
        LOG_ERR("pca.pca_cnv_en(%d)",                              a_info.pca.pca_cnv_en);
        LOG_ERR("smx1i.smx1i_v_flip_en(%d)",                       a_info.smx1i.smx1i_v_flip_en);
        LOG_ERR("smx1i.smx1i_xsize(%d)",                           a_info.smx1i.smx1i_xsize);
        LOG_ERR("smx1i.smx1i_ysize(%d)",                           a_info.smx1i.smx1i_ysize);
        LOG_ERR("smx1o.smx1o_xsize(%d)",                           a_info.smx1o.smx1o_xsize);
        LOG_ERR("smx1o.smx1o_ysize(%d)",                           a_info.smx1o.smx1o_ysize);
        LOG_ERR("smx2i.smx2i_v_flip_en(%d)",                       a_info.smx2i.smx2i_v_flip_en);
        LOG_ERR("smx2i.smx2i_xsize(%d)",                           a_info.smx2i.smx2i_xsize);
        LOG_ERR("smx2i.smx2i_ysize(%d)",                           a_info.smx2i.smx2i_ysize);
        LOG_ERR("smx2o.smx2o_xsize(%d)",                           a_info.smx2o.smx2o_xsize);
        LOG_ERR("smx2o.smx2o_ysize(%d)",                           a_info.smx2o.smx2o_ysize);
        LOG_ERR("smx3i.smx3i_v_flip_en(%d)",                       a_info.smx3i.smx3i_v_flip_en);
        LOG_ERR("smx3i.smx3i_xsize(%d)",                           a_info.smx3i.smx3i_xsize);
        LOG_ERR("smx3i.smx3i_ysize(%d)",                           a_info.smx3i.smx3i_ysize);
        LOG_ERR("smx3o.smx3o_xsize(%d)",                           a_info.smx3o.smx3o_xsize);
        LOG_ERR("smx3o.smx3o_ysize(%d)",                           a_info.smx3o.smx3o_ysize);
        LOG_ERR("smx4i.smx4i_v_flip_en(%d)",                       a_info.smx4i.smx4i_v_flip_en);
        LOG_ERR("smx4i.smx4i_xsize(%d)",                           a_info.smx4i.smx4i_xsize);
        LOG_ERR("smx4i.smx4i_ysize(%d)",                           a_info.smx4i.smx4i_ysize);
        LOG_ERR("smx4o.smx4o_xsize(%d)",                           a_info.smx4o.smx4o_xsize);
        LOG_ERR("smx4o.smx4o_ysize(%d)",                           a_info.smx4o.smx4o_ysize);
        LOG_ERR("smx1.smx1o_sel(%d)",                              a_info.smx1.smx1o_sel);
        LOG_ERR("smx2.smx2o_sel(%d)",                              a_info.smx2.smx2o_sel);
        LOG_ERR("smx3.smx3o_sel(%d)",                              a_info.smx3.smx3o_sel);
        LOG_ERR("smx4.smx4o_sel(%d)",                              a_info.smx4.smx4o_sel);
        #endif
    }
}

/**************************************************************************
*
**************************************************************************/
MVOID dumpWpeTPipeInfo(ISP_TPIPE_CONFIG_STRUCT a_info, MBOOL dump_en)
{
    if(dump_en)
    {
        #if 1
        LOG_INF("sw.log_en = %d",                                a_info.sw.log_en);
		LOG_INF("sw.src_width_wpe = %d",                          a_info.sw.src_width_wpe);
        LOG_INF("sw.src_height_wpe = %d",                        a_info.sw.src_height_wpe);
        LOG_INF("sw.tpipe_width_wpe = %d",                       a_info.sw.tpipe_width_wpe);
        LOG_INF("sw.tpipe_height_wpe = %d",                      a_info.sw.tpipe_height_wpe);
        LOG_INF("sw.tpipe_irq_mode = %d",                        a_info.sw.tpipe_irq_mode);
        LOG_INF("sw.tpipe_sel_mode = %d",                        a_info.sw.tpipe_sel_mode);

		//LOG_INF("top.wpe_sel = %d",                              a_info.top.wpe_sel);
		//LOG_INF("top.wpe_ctl_extension_en = %d",                 a_info.top.wpe_ctl_extension_en);
		//LOG_INF("top.wpe_en = %d",                               a_info.top.wpe_en);
		LOG_INF("top.veci_en = %d",                              a_info.top.veci_en);
		LOG_INF("top.vec2i_en = %d",                             a_info.top.vec2i_en);
		LOG_INF("top.vec3i_en = %d",                      a_info.top.vec3i_en);
		LOG_INF("top.wpeo_en = %d",                      a_info.top.wpeo_en);
		LOG_INF("top.msko_en = %d",                      a_info.top.msko_en);

		LOG_INF("top.wpe_ispcrop_en = %d",                      a_info.top.wpe_ispcrop_en);
		LOG_INF("top.wpe_mdpcrop_en = %d",                      a_info.top.wpe_mdpcrop_en);
		LOG_INF("top.wpe_c24_en = %d",                      a_info.top.wpe_c24_en);
		LOG_INF("top.wpe_cachi_fmt = %d",                      a_info.top.wpe_cachi_fmt);

		LOG_INF("wpe.vgen_input_crop_width = %d", 				 a_info.wpe.vgen_input_crop_width);
		LOG_INF("wpe.vgen_input_crop_height = %d", 				 a_info.wpe.vgen_input_crop_height);
		LOG_INF("wpe.vgen_output_width = %d",					 a_info.wpe.vgen_output_width);
		LOG_INF("wpe.vgen_output_height = %d",					 a_info.wpe.vgen_output_height);
		LOG_INF("wpe.vgen_luma_horizontal_integer_offset = %d",	 a_info.wpe.vgen_luma_horizontal_integer_offset);
		LOG_INF("wpe.vgen_luma_horizontal_subpixel_offset = %d",  a_info.wpe.vgen_luma_horizontal_subpixel_offset);
		LOG_INF("wpe.vgen_luma_vertical_integer_offset = %d",	 a_info.wpe.vgen_luma_vertical_integer_offset);
		LOG_INF("wpe.vgen_luma_vertical_subpixel_offset = %d",	 a_info.wpe.vgen_luma_vertical_subpixel_offset);
		LOG_INF("wpe.vgen_horizontal_coeff_step = %d",			 a_info.wpe.vgen_horizontal_coeff_step);
		LOG_INF("wpe.vgen_vertical_coeff_step = %d",				 a_info.wpe.vgen_vertical_coeff_step);

		LOG_INF("veci.veci_stride = %d",                          a_info.veci.veci_stride);
		LOG_INF("veci.veci_xsize = %d",	                         a_info.veci.veci_xsize);
		LOG_INF("veci.veci_ysize = %d",	                         a_info.veci.veci_ysize);
		LOG_INF("veci.veci_v_flip_en = %d",			             a_info.veci.veci_v_flip_en);

		LOG_INF("vec2i.vec2i_stride = %d",                        a_info.vec2i.vec2i_stride);
		LOG_INF("vec2i.vec2i_xsize = %d",	                     a_info.vec2i.vec2i_xsize);
		LOG_INF("vec2i.vec2i_ysize = %d",	                     a_info.vec2i.vec2i_ysize);
		LOG_INF("vec2i.vec2i_v_flip_en = %d",			         a_info.vec2i.vec2i_v_flip_en);

		LOG_INF("vec3i.vec3i_stride = %d",                        a_info.vec3i.vec3i_stride);
		LOG_INF("vec3i.vec3i_xsize = %d",	                     a_info.vec3i.vec3i_xsize);
		LOG_INF("vec3i.vec3i_ysize = %d",	                     a_info.vec3i.vec3i_ysize);
		LOG_INF("vec3i.vec3i_v_flip_en = %d",			         a_info.vec3i.vec3i_v_flip_en);

		LOG_INF("wpeo.wpeo_stride = %d",                          a_info.wpeo.wpeo_stride);
		LOG_INF("wpeo.wpeo_xsize = %d",	                         a_info.wpeo.wpeo_xsize);
		LOG_INF("wpeo.wpeo_xoffset = %d",	                     a_info.wpeo.wpeo_xoffset);
		LOG_INF("wpeo.wpeo_ysize = %d",			                 a_info.wpeo.wpeo_ysize);
		LOG_INF("wpeo.wpeo_yoffset = %d",		             	 a_info.wpeo.wpeo_yoffset);

		LOG_INF("msko.msko_stride = %d",                          a_info.msko.msko_stride);
		LOG_INF("msko.msko_xsize = %d",	                         a_info.msko.msko_xsize);
		LOG_INF("msko.msko_xoffset = %d",	                     a_info.msko.msko_xoffset);
		LOG_INF("msko.msko_ysize = %d",			                 a_info.msko.msko_ysize);
		LOG_INF("msko.msko_yoffset = %d",		             	 a_info.msko.msko_yoffset);
        #endif
    }
}

/**************************************************************************
*
**************************************************************************/
MINT32 MdpMgrImp::init(MUINT32 tileselmode)
{
    LOG_INF("+, mInitCount(%d), mode(%d)",mwpeInitCount, tileselmode);
    Mutex::Autolock lock(mLock);
    //
    MBOOL err = MDPMGR_NO_ERROR;
    //
    mTileSelMode = tileselmode;

    if(mwpeInitCount == 0) {
		switch(mTileSelMode)
		{   //wpe stream
			case TILE_SEL_MODE_WPE:
				for(MINT32 i = 0; i < MAX_DUP_CQ_NUM; i++)
				{
    				m_wpeaDpStream[i] = new DpIspStream(DpIspStream::WPE_STREAM);
					m_wpebDpStream[i] = new DpIspStream(DpIspStream::WPE_STREAM2);
				}
				break;
			default:
				err = MDPMGR_WRONG_PARAM;
				goto EXIT;
    	}
    }

    android_atomic_inc(&mwpeInitCount);

EXIT:
    LOG_INF("-,mInitCount(%d)",mwpeInitCount);

    return err;
}

/**************************************************************************
*
**************************************************************************/
MINT32 MdpMgrImp::uninit(MUINT32 tileselmode)
{
    LOG_DBG("+,mInitCount(%d), mode(%d)",mwpeInitCount, tileselmode);
    //
    Mutex::Autolock lock(mLock);
    //
    MBOOL err = MDPMGR_NO_ERROR;

	if (mTileSelMode != tileselmode)
	{
        err = MDPMGR_WRONG_PARAM;
		goto EXIT;
	}

    //
    android_atomic_dec(&mwpeInitCount);
    //
    //if(mwpeInitCount > 0) {
       // err = MDPMGR_STILL_USERS;
        //goto EXIT;
   // }

    if(mwpeInitCount == 0) {
		switch(mTileSelMode)
		{	//wpe stream
			case TILE_SEL_MODE_WPE:
				for(MINT32 i = 0; i < MAX_DUP_CQ_NUM; i++)
				{
    				delete m_wpeaDpStream[i];
					delete m_wpebDpStream[i];
        			m_wpeaDpStream[i] = NULL;
					m_wpebDpStream[i] = NULL;
				}
				break;
			default:
				err = MDPMGR_WRONG_PARAM;
				goto EXIT;
    	}
    }

EXIT:
    LOG_DBG("-,mInitCount(%d)",mwpeInitCount);
    return err;
}

/**************************************************************************
*
**************************************************************************/
DpIspStream *MdpMgrImp::selectWpeStream(MUINT32 drvScenario, MUINT32 dupCqIdx)
{
    LOG_DBG("+");

    Mutex::Autolock lock(mLock);
	switch(drvScenario)
	{
		case 1:
			return m_wpebDpStream[dupCqIdx];
		case 0:
		default:
			return m_wpeaDpStream[dupCqIdx];
	}
}

/**************************************************************************
*
**************************************************************************/
    // Minimum Y pitch that is acceptable by HW
#define WPE_DP_COLOR_GET_MIN_Y_PITCH(color, width)                                              \
            (((DP_COLOR_BITS_PER_PIXEL(color) * width) + 4) >> 3)

    // Minimum UV pitch that is acceptable by HW
#define WPE_DP_COLOR_GET_MIN_UV_PITCH(color, width)                                             \
            ((1 == DP_COLOR_GET_PLANE_COUNT(color))? 0:                                             \
             (((0 == DP_COLOR_IS_UV_COPLANE(color)) || (1 == DP_COLOR_GET_BLOCK_MODE(color)))?      \
              (WPE_DP_COLOR_GET_MIN_Y_PITCH(color, width)  >> DP_COLOR_GET_H_SUBSAMPLE(color)):         \
              ((WPE_DP_COLOR_GET_MIN_Y_PITCH(color, width) >> DP_COLOR_GET_H_SUBSAMPLE(color)) * 2)))


template <typename T>
MINT32 _wpestartMdp(MDPMGR_CFG_STRUCT &cfgData,T* pDpStream)
{
    MINT32 err = MDPMGR_NO_ERROR;
    DP_STATUS_ENUM err2 = DP_STATUS_RETURN_SUCCESS;
    MUINT32 srcVirList[PLANE_NUM];
    MUINT32 srcPhyList[PLANE_NUM];
    MUINT32 srcSizeList[PLANE_NUM];
    MUINT32 mdpSrcYStride = 0;                  //! Y stride of mdp WPE crop out image
    MUINT32 mdpSrcUVStride = 0;                 //! UV stride of mdp WPE crop our image

    //get property for p2tpipedump
    int p2tpipedumpEnable = ::property_get_int32("vendor.camera.p2tpipedump.enable", 0);
    int p2disableSkipTpipe = ::property_get_int32("vendor.camera.disable_p2skiptpipe", 0);;

    ISP_TRACE_CALL();

    // image configure
    LOG_INF("[WPE] WARP Engine ID (%d), cqIndx(%d), frameflag(%d), srcFmt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),planeN(%d), \
             tdri va/pa: 0x%x/0x%x, \n\
             wdmao_en(%d),wdmao_index(%d),capbility(%d),fmt(%d),rot(%d),flip(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpindx(%d),\n\
             wroto_en(%d),wroto_index(%d),capbility(%d),fmt(%d),rot(%d),flip(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpindx(%d)",\
             cfgData.drvScenario, cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe, cfgData.ispTpipeCfgInfo.drvinfo.frameflag,\
             cfgData.mdpSrcFmt,cfgData.mdpSrcW,cfgData.mdpSrcH,cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,cfgData.mdpSrcBufSize,cfgData.mdpSrcCBufSize,cfgData.mdpSrcVBufSize, \
             cfgData.srcVirAddr,cfgData.srcPhyAddr,cfgData.mdpSrcPlaneNum, \
             cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa_wpe,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa_wpe,\
             cfgData.dstPortCfg[ISP_MDP_DL_WDMAO],ISP_MDP_DL_WDMAO, cfgData.dstDma[ISP_MDP_DL_WDMAO].capbility, cfgData.dstDma[ISP_MDP_DL_WDMAO].Format, cfgData.dstDma[ISP_MDP_DL_WDMAO].Rotation,  \
             cfgData.dstDma[ISP_MDP_DL_WDMAO].Flip,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.stride,\
             cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.stride,\
             cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.stride,mdpPortMapping[ISP_MDP_DL_WDMAO].mdpPortIdx,\
             cfgData.dstPortCfg[ISP_MDP_DL_WROTO],ISP_MDP_DL_WROTO, cfgData.dstDma[ISP_MDP_DL_WROTO].capbility, cfgData.dstDma[ISP_MDP_DL_WROTO].Format, cfgData.dstDma[ISP_MDP_DL_WROTO].Rotation, \
             cfgData.dstDma[ISP_MDP_DL_WROTO].Flip,cfgData.dstDma[ISP_MDP_DL_WROTO].size.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size.stride,
             cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.stride,\
             cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.stride,mdpPortMapping[ISP_MDP_DL_WROTO].mdpPortIdx);

    if (cfgData.ispTpipeCfgInfo.top.wpe_mdpcrop_en == 1 && cfgData.ispTpipeCfgInfo.top.wpe_en == 1) // WPE ONLY FRAME MODE sould Skip src & dst settings
    {
        //====== Configure Source ======

         DP_COLOR_ENUM srcFmt;

        // format convert
        err = DpColorFmtConvert(cfgData.mdpSrcFmt, &srcFmt);
        if(err != MDPMGR_NO_ERROR)
        {
            LOG_ERR("DpColorFmtConvert fail");
        return MDPMGR_API_FAIL;
        }

        mdpSrcYStride = WPE_DP_COLOR_GET_MIN_Y_PITCH(srcFmt, cfgData.mdpSrcW);
        mdpSrcUVStride = WPE_DP_COLOR_GET_MIN_UV_PITCH(srcFmt, cfgData.mdpSrcW);
        LOG_INF(" srcFmt(0x%x),W(%u),H(%u),stride(%u,%u),",srcFmt, cfgData.mdpSrcW, cfgData.mdpSrcH, mdpSrcYStride,mdpSrcUVStride);

        if (cfgData.isSecureFra == 1)
        {
            err2= pDpStream->setSrcConfig(cfgData.mdpSrcW ,cfgData.mdpSrcH, \
                mdpSrcYStride,mdpSrcUVStride,srcFmt, \
                DP_PROFILE_FULL_BT601, eInterlace_None, 0, false, (DpSecure)cfgData.srcSecureTag);
        }
        else
        {
            err2= pDpStream->setSrcConfig(cfgData.mdpSrcW ,cfgData.mdpSrcH, \
                mdpSrcYStride,mdpSrcUVStride,srcFmt, \
                DP_PROFILE_FULL_BT601, eInterlace_None, 0, false, DP_SECURE_NONE);
        }

        if(err2 != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("setSrcConfig fail(%d)",err);
            return MDPMGR_API_FAIL;
        }

        #if 0  //kk test  ==> need to confirm with ping-hsung
        if(cfgData.srcVirAddr == 0 || cfgData.srcPhyAddr == 0){
            LOG_ERR("[Error]src memAddr is 0,VA(0x%8x),PA(0x%8x),size(0x%8x)",cfgData.srcVirAddr,cfgData.srcPhyAddr,cfgData.mdpSrcBufSize);
            return MDPMGR_NULL_OBJECT;
        }
        #endif

        srcSizeList[0] = cfgData.mdpSrcBufSize;
        srcSizeList[1] = cfgData.mdpSrcCBufSize;
        srcSizeList[2] = cfgData.mdpSrcVBufSize;

        // only for dpframework debug, so set imgi va and pa always for each plane
        srcPhyList[0] = cfgData.srcPhyAddr;
        srcPhyList[1] = cfgData.srcPhyAddr;
        srcPhyList[2] = cfgData.srcPhyAddr;
        srcVirList[0] = cfgData.srcVirAddr;
        srcVirList[1] = cfgData.srcVirAddr;
        srcVirList[2] = cfgData.srcVirAddr;

        #if 0
        err2 = pDpStream->queueSrcBuffer((void**)srcVirList,&srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);
        #else
        if (cfgData.isSecureFra == 1)
        {
            //err2 = pDpStream->queueSrcBuffer(srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);
            err2 = pDpStream->queueSrcBuffer((void**)srcPhyList,srcSizeList,cfgData.mdpSrcPlaneNum);
        }
        else
        {
            err2 = pDpStream->queueSrcBuffer(&srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);
        }
        #endif
        if(err2 != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("queueSrcBuffer fail(%d)",err);
            return MDPMGR_API_FAIL;
        }


        //====== Configure Output DMA ======

        DP_COLOR_ENUM dstFmt;
        MVOID *dstVirList[PLANE_NUM];
        MUINT32 dstSizeList[PLANE_NUM];
        MUINT32 dstPhyList[PLANE_NUM];
        DpRect pROI;
        DpPqParam   ISPParam;
        for(MINT32 index = 0; index < ISP_OUTPORT_NUM; index++) {
            LOG_DBG("index(%d),cfgData.dstPortCfg[index](%d)",index,cfgData.dstPortCfg[index]);

        if(cfgData.dstPortCfg[index] == 1) {
            if (index != ISP_ONLY_OUT_TPIPE ) { // for non isp only
                if(cfgData.dstDma[index].enSrcCrop) {  //set src crop if need
                    LOG_DBG("idx[%d],enSrcCrop(%d),X(%u),FloatX(%u),Y(%u),FloatY(%u),W(%u),H(%u)", index,cfgData.dstDma[index].enSrcCrop,\
                            cfgData.dstDma[index].srcCropX,cfgData.dstDma[index].srcCropFloatX,cfgData.dstDma[index].srcCropY,cfgData.dstDma[index].srcCropFloatY,\
                            cfgData.dstDma[index].srcCropW,cfgData.dstDma[index].srcCropH);

                        if(cfgData.dstDma[index].srcCropW==0 || cfgData.dstDma[index].srcCropH==0) {
                            LOG_ERR("[Error](%d) wrong crop w(%d),h(%d)",index,cfgData.dstDma[index].srcCropW,cfgData.dstDma[index].srcCropH);
                            return MDPMGR_WRONG_PARAM;
                        } else if(cfgData.dstDma[index].srcCropW>cfgData.mdpSrcW || cfgData.dstDma[index].srcCropH>cfgData.mdpSrcH) {
                            LOG_ERR("[Error](%d) crop size(%d,%d) exceed source size(%d,%d)",index,cfgData.dstDma[index].srcCropW,cfgData.dstDma[index].srcCropH,cfgData.mdpSrcW,cfgData.mdpSrcH);
                            return MDPMGR_WRONG_PARAM;
                        }
                        err2 = pDpStream->setSrcCrop(index,
                                                    cfgData.dstDma[index].srcCropX,
                                                    cfgData.dstDma[index].srcCropFloatX,
                                                    cfgData.dstDma[index].srcCropY,
                                                    cfgData.dstDma[index].srcCropFloatY,
                                                    cfgData.dstDma[index].srcCropW,
                                                    cfgData.dstDma[index].srcCropH);

                        if(err2 != DP_STATUS_RETURN_SUCCESS) {
                            LOG_ERR("(%d)setSrcCrop fail(%d)",index,err);
                            return MDPMGR_API_FAIL;
                        }
                        LOG_DBG(" (%d) end pDpStream->setSrcCrop",index);
                    }

                // format convert
                    err = DpDmaOutColorFmtConvert(cfgData.dstDma[index],&dstFmt);
                    if(err != MDPMGR_NO_ERROR) {
                        LOG_ERR("DpDmaOutColorFmtConvert fail");
                        return MDPMGR_API_FAIL;
                    }

                    // image info configure
                    pROI.x=0;
                    pROI.y=0;
                    pROI.sub_x=0;
                    pROI.sub_y=0;
                    pROI.w=cfgData.dstDma[index].size.w;
                    pROI.h=cfgData.dstDma[index].size.h;
                    DP_PROFILE_ENUM dp_rofile=DP_PROFILE_FULL_BT601;
#if 0
                    ISPParam.enable = false; // Set sharpness disable because change capbility issue
                    ISPParam.scenario = MEDIA_ISP_PREVIEW;
                    ISPParam.u.isp.iso = cfgData.isoValue;
                    if (cfgData.mdpCZParam != NULL) {
                        ISPParam.u.isp.CZParam = cfgData.mdpCZParam;
                    }
#endif
                    switch(index) {
                        case ISP_MDP_DL_WDMAO:
                            {
                                err2 = pDpStream->setPQParameter(index, ((DpPqParam*)(cfgData.mdpWDMAPQParam)));
                                if(err2 != DP_STATUS_RETURN_SUCCESS) {
                                    LOG_ERR("index(%d), setPQParameter fail(%d)",index, err2);
                                    return MDPMGR_API_FAIL;
                                }
                            }
                            break;
                        case ISP_MDP_DL_WROTO:
                            {
                                err2 = pDpStream->setPQParameter(index, ((DpPqParam*)(cfgData.mdpWROTPQParam)));
                                if(err2 != DP_STATUS_RETURN_SUCCESS) {
                                    LOG_ERR("index(%d), setPQParameter fail(%d)",index, err2);
                                    return MDPMGR_API_FAIL;
                                }
                             }
                             break;
                        default:
                            break;
                    }
                    switch(cfgData.dstDma[index].capbility) {
                        case NSImageio::NSIspio::EPortCapbility_Rcrd:
                            {
                                dp_rofile=DP_PROFILE_BT601;
                            }
                            break;
                        case NSImageio::NSIspio::EPortCapbility_Cap:
                            {
                                dp_rofile=DP_PROFILE_FULL_BT601;
                            }
                            break;
                        case NSImageio::NSIspio::EPortCapbility_Disp:
                            {
                                dp_rofile=DP_PROFILE_FULL_BT601;
                            }
                            break;
                        default:
                            dp_rofile=DP_PROFILE_FULL_BT601;
                            break;
                    }

                    if ((cfgData.isSecureFra == 1) && (cfgData.dstDma[index].secureTag != 0))
                    {
                        err2 = pDpStream->setDstConfig(index,
                                                      cfgData.dstDma[index].size.w,
                                                      cfgData.dstDma[index].size.h,
                                                      cfgData.dstDma[index].size.stride,
                                                      cfgData.dstDma[index].size_c.stride,
                                                      dstFmt,
                                                      dp_rofile,
                                                      eInterlace_None,
                                                      &pROI,
                                                      false,
                                                      (DpSecure)cfgData.dstDma[index].secureTag);
                    }
                    else
                    {
                        err2 = pDpStream->setDstConfig(index,
                                                    cfgData.dstDma[index].size.w,
                                                    cfgData.dstDma[index].size.h,
                                                    cfgData.dstDma[index].size.stride,
                                                    cfgData.dstDma[index].size_c.stride,
                                                    dstFmt,
                                                    dp_rofile,
                                                    eInterlace_None,
                                                    &pROI,
                                                    false,
                                                    DP_SECURE_NONE);
                    }

                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                    LOG_ERR("setDstConfig fail(%d)",err);
                    return MDPMGR_API_FAIL;
                    }

                    // rotation
                    err2 = pDpStream->setRotation(index, cfgData.dstDma[index].Rotation * 90);
                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                        LOG_ERR("setRotation fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }

                    // flip
                    err2 = pDpStream->setFlipStatus(index, cfgData.dstDma[index].Flip);
                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                        LOG_ERR("setFlipStatus fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }

                    // memory
                    if(cfgData.dstDma[index].memBuf.base_vAddr == 0 || cfgData.dstDma[index].memBuf.base_pAddr == 0) {
                        LOG_ERR("index(%d)",index);
                        LOG_ERR("dst memAddr is 0,VA(0x%8x),PA(0x%8x)",cfgData.dstDma[index].memBuf.base_vAddr,cfgData.dstDma[index].memBuf.base_pAddr);
                        LOG_ERR("dst, W(%u),H(%u),W_c(%u),H_c(%u),W_v(%u),H_v(%u)",cfgData.dstDma[index].size.w,
                                                                                cfgData.dstDma[index].size.h,
                                                                                cfgData.dstDma[index].size_c.w,
                                                                                cfgData.dstDma[index].size_c.h,
                                                                                cfgData.dstDma[index].size_v.w,
                                                                                cfgData.dstDma[index].size_v.h);

                        LOG_ERR("stride(%u),stride_c(%u),stride_v(%u)",cfgData.dstDma[index].size.stride,
                                                                   cfgData.dstDma[index].size_c.stride,
                                                                   cfgData.dstDma[index].size_v.stride);
                        return MDPMGR_NULL_OBJECT;
                    }

                    dstSizeList[0] = cfgData.dstDma[index].size.h   * (cfgData.dstDma[index].size.stride);
                    dstSizeList[1] = cfgData.dstDma[index].size_c.h * (cfgData.dstDma[index].size_c.stride);
                    dstSizeList[2] = cfgData.dstDma[index].size_v.h * (cfgData.dstDma[index].size_v.stride);

                    dstVirList[0] = (MVOID *)cfgData.dstDma[index].memBuf.base_vAddr;
                    dstVirList[1] = (MVOID *)cfgData.dstDma[index].memBuf_c.base_vAddr;
                    dstVirList[2] = (MVOID *)cfgData.dstDma[index].memBuf_v.base_vAddr;

                    dstPhyList[0] = cfgData.dstDma[index].memBuf.base_pAddr;
                    dstPhyList[1] = cfgData.dstDma[index].memBuf_c.base_pAddr;
                    dstPhyList[2] = cfgData.dstDma[index].memBuf_v.base_pAddr;
                    LOG_DBG("dstSizeList=(0x%8x,0x%8x,0x%8x),dstVirList=(0x%8x,0x%8x,0x%8x),dstPhyList=(0x%8x,0x%8x,0x%8x),plane num(%d)",
                        dstSizeList[0],dstSizeList[1],dstSizeList[2],dstVirList[0],dstVirList[1],dstVirList[2],
                        dstPhyList[0],dstPhyList[1],dstPhyList[2],cfgData.dstDma[index].Plane);

                    if ((cfgData.isSecureFra == 1) && (cfgData.dstDma[index].secureTag != 0))
                    {
                        //err2 = pDpStream->queueDstBuffer(index, dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                        err2 = pDpStream->queueDstBuffer(index, (void **)dstPhyList, dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    }
                    else
                    {
                        err2 = pDpStream->queueDstBuffer(index,&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    }
                    if(err2 != DP_STATUS_RETURN_SUCCESS)
                    {
                        LOG_ERR("queueDstBuffer fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }
              }

             else
             { // for isp only mode      ISP ONLY MODE doesn't need to config src & dst after ISP40, wait for delete.
                pDpStream->setPortType(ISP_ONLY_OUT_TPIPE, MDPMGR_ISP_ONLY_FRAME);
                LOG_DBG("queueDstBuffer MDPMGR_ISP_ONLY_FRAME %d", index);
             }
        }
      }
    }
    else
    {
        pDpStream->setPortType(ISP_ONLY_OUT_TPIPE, MDPMGR_ISP_ONLY_FRAME);
    }


    if(p2tpipedumpEnable == 1)
    {
        dumpWpeTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
    }
    else
    {
        dumpWpeTPipeInfo(cfgData.ispTpipeCfgInfo,MFALSE);
    }


    err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, cfgData.ispTpipeCfgInfo.drvinfo.frameflag);

    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("setParameter fail(%d)",err);
        return MDPMGR_API_FAIL;
    }

    //====== Start DpIspStream ======
    //LOG_INF("startStream");
    MUINT32 a=0,b=0;
    a=getUs();
    err2 = pDpStream->startStream();
    b=getUs();


    LOG_INF("HMyo flag(%d_0x%x -> 0x%x), startStream (%d us)...", p2disableSkipTpipe, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, b-a);

    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("startStream fail(%d)",err2);
        LOG_ERR("===dump tpipe structure start===");
        dumpWpeTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
        LOG_ERR("===dump tpipe structure end===");
        return MDPMGR_API_FAIL;
    }


    // cut off the previous stream
    err2 = pDpStream->stopStream();
    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("stopStream fail(%d)",err2);
        LOG_ERR("===dump tpipe structure start===");
        dumpWpeTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
        LOG_ERR("===dump tpipe structure end===");
        return MDPMGR_API_FAIL;
     }

    LOG_DBG("-");
    return MDPMGR_NO_ERROR;
}

