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
#include "dip_aee.h" 
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


long getMontonicUs()
{
   timespec time1;
   clock_gettime(CLOCK_MONOTONIC, &time1);
   return time1.tv_sec * 1000000 + (time1.tv_nsec / 1000);
}
/**************************************************************************
*
**************************************************************************/
template <typename T>
MINT32 _wpestartMdp(MDPMGR_CFG_STRUCT &cfgData,T* pDpStream, MdpMgr* pMdrMgrObj);
MVOID dumpWpeTPipeInfo(ISP_TPIPE_CONFIG_STRUCT a_info, MBOOL dump_en);


/**************************************************************************
*
**************************************************************************/
template <typename T>
MINT32 _startMdp(MDPMGR_DPSTREAM_TYPE dpStreamtype,MDPMGR_CFG_STRUCT &cfgData,T* pDpStream, MdpMgr* pMdrMgrObj)
{
	MINT32 err = MDPMGR_NO_ERROR;
    DP_STATUS_ENUM err2 = DP_STATUS_RETURN_SUCCESS;
    ISP2MDP_STRUCT isp2mdpcfg;

    MUINT32 srcVirList[PLANE_NUM];
    MUINT32 srcPhyList[PLANE_NUM];
    MUINT32 srcSizeList[PLANE_NUM];
    MUINT32 WDMAPQEnable = 0, WROTPQEnable = 0;

    ISP_TRACE_CALL();

    if (cfgData.ispTpipeCfgInfo.drvinfo.frameflag != 2) // ISP ONLY FRAME MODE sould Skip src & dst settings
    {
    //====== Configure Source ======
        if (cfgData.ispTpipeCfgInfo.drvinfo.frameflag == 3) // Force skip setting
        {
#if 1
            DP_COLOR_ENUM srcFmt;

            // format convert
            err = DpColorFmtConvert(cfgData.mdpSrcFmt, &srcFmt);
            if(err != MDPMGR_NO_ERROR)
            {
                LOG_ERR("DpColorFmtConvert fail");
                return MDPMGR_API_FAIL;
            }

            err2= pDpStream->setSrcConfig(cfgData.mdpSrcW ,cfgData.mdpSrcH, \
                cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,srcFmt, \
                DP_PROFILE_FULL_BT601, eInterlace_None, 0, false);
            if(err2 != DP_STATUS_RETURN_SUCCESS)
            {
                LOG_ERR("setSrcConfig fail(%d)",err2);
                return MDPMGR_API_FAIL;
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
            err2 = pDpStream->queueSrcBuffer(&srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);
            if(err2 != DP_STATUS_RETURN_SUCCESS)
            {
                LOG_ERR("queueSrcBuffer fail(%d)",err2);
                return MDPMGR_API_FAIL;
            }

            MVOID *dstVirList[PLANE_NUM];
            MUINT32 dstSizeList[PLANE_NUM];
            MUINT32 dstPhyList[PLANE_NUM];
            DpRect pROI;
            //DpPqParam   ISPParam;
            DpPqParam*   pPQParam;
            DP_COLOR_ENUM dstFmt;

            for(MINT32 index = 0; index < ISP_OUTPORT_NUM; index++) {
                LOG_DBG("index(%d),cfgData.dstPortCfg[index](%d)",index,cfgData.dstPortCfg[index]);

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
                                LOG_ERR("(%d)setSrcCrop fail(%d)",index,err);
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
                                    err2 = pDpStream->setPQParameter(index, ((DpPqParam*)(cfgData.mdpWDMAPQParam)));
                                    if(err2 != DP_STATUS_RETURN_SUCCESS) {
                                        LOG_ERR("index(%d), setPQParameter fail(%d)",index, err2);
                                        return MDPMGR_API_FAIL;
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
                        err2 = pDpStream->setDstConfig(index,
                                                      cfgData.dstDma[index].size.w,
                                                      cfgData.dstDma[index].size.h,
                                                      cfgData.dstDma[index].size.stride,
                                                      cfgData.dstDma[index].size_c.stride,
                                                      dstFmt,
                                                      dp_rofile,
                                                      eInterlace_None,
                                                      &pROI,
                                                      false);
                        if(err2 != DP_STATUS_RETURN_SUCCESS) {
                            LOG_ERR("setDstConfig fail(%d)",err2);
                            return MDPMGR_API_FAIL;
                        }



                        // rotation
                        err2 = pDpStream->setRotation(index, cfgData.dstDma[index].Rotation * 90);
                        if(err2 != DP_STATUS_RETURN_SUCCESS) {
                            LOG_ERR("setRotation fail(%d)",err2);
                            return MDPMGR_API_FAIL;
                        }

                        // flip
                        err2 = pDpStream->setFlipStatus(index, cfgData.dstDma[index].Flip);
                        if(err2 != DP_STATUS_RETURN_SUCCESS) {
                            LOG_ERR("setFlipStatus fail(%d)",err2);
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

                        dstPhyList[0] = cfgData.dstDma[index].memBuf.base_pAddr;
                        dstPhyList[1] = cfgData.dstDma[index].memBuf_c.base_pAddr;
                        dstPhyList[2] = cfgData.dstDma[index].memBuf_v.base_pAddr;

                        err2 = pDpStream->queueDstBuffer(index,&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                        if(err2 != DP_STATUS_RETURN_SUCCESS)
                        {
                            LOG_ERR("queueDstBuffer fail(%d)",err);
                            return MDPMGR_API_FAIL;
                        }




                    }
                    else
                    {
                        // format convert
                        err = DpDmaOutColorFmtConvert(cfgData.dstDma[index],&dstFmt);
                        if(err != MDPMGR_NO_ERROR) {
                            LOG_ERR("DpDmaOutColorFmtConvert fail");
                            return MDPMGR_API_FAIL;
                        }

                        // image info configure
                        err2 = pDpStream->setDstConfig(index,
                                                      cfgData.dstDma[index].size.w,
                                                      cfgData.dstDma[index].size.h,
                                                      cfgData.dstDma[index].size.stride,
                                                      cfgData.dstDma[index].size_c.stride,
                                                      dstFmt,
                                                      DP_PROFILE_FULL_BT601,
                                                      eInterlace_None,
                                                      &pROI,
                                                      false);
                        if(err2 != DP_STATUS_RETURN_SUCCESS) {
                            LOG_ERR("setDstConfig fail(%d)",err2);
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

                        if (dpStreamtype == MDPMGR_DPSTREAM_TYPE_VENC)
                        {
                            err2 = pDpStream->queueDstBuffer(index, &dstVirList[0],&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                        }
                        else
                        {
                            err2 = pDpStream->queueDstBuffer(index,&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                        }

                        if(err2 != DP_STATUS_RETURN_SUCCESS)
                        {
                            LOG_ERR("queueDstBuffer fail(%d)",err2);
                            return MDPMGR_API_FAIL;
                        }


                    }
                }
            }
        }
        else
        {

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
	                DP_PROFILE_FULL_BT601, eInterlace_None, 0, false);
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
                                LOG_ERR("(%d)setSrcCrop fail(%d)",index,err);
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
                                    if (pMdrMgrObj->m_p2disablePQ == 0)
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
                                    if (pMdrMgrObj->m_p2disablePQ == 0)
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
	                                                      false);
	                    }
                        if(err2 != DP_STATUS_RETURN_SUCCESS) {
                            LOG_ERR("setDstConfig fail(%d)",err);
                            return MDPMGR_API_FAIL;
                        }

                        if(dpStreamtype==MDPMGR_DPSTREAM_TYPE_VENC) {
                            // set port type (only for ISP only mode)
                            if(index == ISP_MDP_DL_VENCO)
                            {
                                DpVEnc_Config	DpVEncCfg;
                                //FIXME, use MDPMGR_ISP_MDP_VENC to work around first, cuz port num supported in dpispstream and dpvencstream is 3
                                //          so we may set ISP_MDP_DL_VENCO=ISP_MDP_DL_JPEGO=2 first
                                //          *** should set DpVEncCfg
                                reinterpret_cast<DpVEncStream*>(pDpStream)->setPortType(index, MDPMGR_ISP_MDP_VENC, &DpVEncCfg);
                            }
                        }

                        // For MDP_CROP2
                    if ((index == ISP_MDP_DL_WDMAO) || (index == ISP_MDP_DL_WROTO)) {
                            if (cfgData.dstDma[index].crop_group == 1) // MDP_CROP2
                                pDpStream->setPortType(index, MDPMGR_ISP_MDP_DL2);
                            else // MDP_CROP
                                pDpStream->setPortType(index, mdpPortMapping[index].mdpPortIdx);
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
                        { // for isp only mode      ISP ONLY MODE

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
	                                                      false);
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
    }
    else
    {
        pDpStream->setPortType(ISP_ONLY_OUT_TPIPE, MDPMGR_ISP_ONLY_TPIPE);
    }

    if(pMdrMgrObj->m_p2tpipedumpEnable == 1)
    {
        dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
    }

    if (cfgData.ispTpipeCfgInfo.drvinfo.cqIdx == 1){
        err2 = pDpStream->setCallback(cfgData.MDPCallback, (void*)cfgData.MDPparam);
    }

    if (cfgData.m_iDipMetEn)
    {
        isp2mdpcfg.MET_String = cfgData.ispTpipeCfgInfo.drvinfo.m_pMetLogBuf;
        isp2mdpcfg.MET_String_length = cfgData.ispTpipeCfgInfo.drvinfo.m_MetLogBufSize;
        //mark by hung-wen becuase in early porting stage
       // err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, &isp2mdpcfg, 0);
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
        if ((cfgData.ispTpipeCfgInfo.drvinfo.dip_imgbi_secure_tag != 0) & (cfgData.ispTpipeCfgInfo.top.bpc_en == 1))
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
        if ((cfgData.ispTpipeCfgInfo.drvinfo.dip_imgci_secure_tag != 0) & (cfgData.ispTpipeCfgInfo.top.lsc_en == 1))
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
        LOG_INF("isp2mdpcfg(secIspData_tag(0x%lx)): CqDesVA(0x%lx), CqDesSize(0x%lx), CqVirtVA(0x%lx), CqVirtSize(0x%lx), TpipeVA(0x%lx), TpipeSize(0x%lx),\n\
            CqSecHandle(0x%lx), CqSecSize(0x%x), CqDesOft(0x%x), CqVirtOft(0x%x), TpipeSecHandle(0x%lx), TpipeSecSize(0x%x), TpipeOft(0x%x),\n\
            BpciVA(0x%lx), BpciSize(0x%lx), BpciHdl(0x%lx), LsciVA(0x%lx), LsciSize(0x%lx), LsciHdl(0x%lx), LceiVA(0x%lx), LceiSize(0x%lx), LceiHdl(0x%lx), DepiVA(0x%lx), DepiSize(0x%lx), DepiHdl(0x%lx), DmgiVA(0x%lx), DmgiSize(0x%lx), DmgiHdl(0x%lx)",\
            (unsigned long)cfgData.srcSecureTag,\
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
        //err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, &isp2mdpcfg, 0);
    }


    if ((cfgData.m_iDipMetEn) || (cfgData.isSecureFra))
    {
        err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, &isp2mdpcfg, 0);

        if(err2 != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("isp2mdpcfg of setParameter fail(%d)",err2);
            return MDPMGR_API_FAIL;
        }
    }

    if(pMdrMgrObj->m_p2disableSkipTpipe)
    {
        if((cfgData.ispTpipeCfgInfo.drvinfo.frameflag == 0x0) || (cfgData.ispTpipeCfgInfo.drvinfo.frameflag == 0x1)  || (cfgData.ispTpipeCfgInfo.drvinfo.frameflag == 0x3))
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
        if (cfgData.ispTpipeCfgInfo.drvinfo.frameflag == 0x3)
        {
            err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, 0x1);
        }
        else
        {
            err2 = pDpStream->setParameter(cfgData.ispTpipeCfgInfo, cfgData.ispTpipeCfgInfo.drvinfo.frameflag);
        }
    }
    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("setParameter fail(%d)",err);
        return MDPMGR_API_FAIL;
    }

    //====== Start DpIspStream ======
    //LOG_INF("startStream");
    MUINT32 a=0,b=0;
#if defined(DIP_USERDBG_AND_USER_LOAD_SUPPORT)
    MUINT32 subt=0;
#endif
    a=getUs();
    if ((cfgData.ExpectedEndTime.tv_sec == 0) && (cfgData.ExpectedEndTime.tv_usec == 0))
    {
        err2 = pDpStream->startStream();
    }
    else
    {
        err2 = pDpStream->startStream(&cfgData.ExpectedEndTime);
    }
    b=getUs();
    //update tdri buffer after mdp operation to cover the situation that isp do not update but mdp update

    if(pMdrMgrObj->m_p2disableSkipTpipe)
    {
        LOG_INF("+sceID(%u),cqPa(0x%lx)-Va(0x%lx),tpipVa(0x%lx)-Pa(0x%lx),isWaitB(%d),venc(%d),\
        srcFt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),plane(%d),Stmtype(%d)\n\
        wdma_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        wrot_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        jpg_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        pipe_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        HMyo CQ(%d_%d_%d),(va:0x%lx,dstpa:0x%lx)\n,skip(0x%x),flag(0x%x),startStm(%d us),WDPQ(0x%8x),WRPQ(0x%8x),DpStream(%p),tv_sec(%lu),tv_usec(%lu)",\
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
                cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx, cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,\
                (unsigned long)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa, \
                cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, pMdrMgrObj->m_p2disableSkipTpipe, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, b-a, WDMAPQEnable, WROTPQEnable, pDpStream, cfgData.ExpectedEndTime.tv_sec, cfgData.ExpectedEndTime.tv_usec);

    }
    else
    {
        LOG_INF("+sceID(%u),cqPa(0x%lx)-Va(0x%lx),tpipVa(0x%lx)-Pa(0x%lx),isWaitB(%d),venc(%d),\
        srcFt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),plane(%d),Stmtype(%d)\n\
        wdma_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        wrot_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        jpg_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        pipe_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
        HMyo CQ(%d_%d_%d),(va:0x%lx,dstpa:0x%lx)\n,skip(0x%x),flag(0x%x),startStm(%d us),WDPQ(0x%8x),WRPQ(0x%8x),DpStream(%p),tv_sec(%lu),tv_usec(%lu)",\
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
                cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx, cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,
                (unsigned long)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,
                cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, pMdrMgrObj->m_p2disableSkipTpipe, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, b-a, WDMAPQEnable, WROTPQEnable, pDpStream, cfgData.ExpectedEndTime.tv_sec, cfgData.ExpectedEndTime.tv_usec);

    }
    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("startStream fail(%d)",err);
        LOG_ERR("===dump tpipe structure start===");
        dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
        LOG_ERR("===dump tpipe structure end===");
        AEE_ASSERT("\nCRDISPATCH_KEY:startStream Fail!!\n");
        return MDPMGR_API_FAIL;
    }

#if defined(DIP_USERDBG_AND_USER_LOAD_SUPPORT)
    //Check Start Time, if the Deque is more than 30ms , assert it!
    if (DP_STATUS_RETURN_SUCCESS == err2)
    {
        subt = b-a;
        if ((subt >= ISP_MDP_STARTSTREAM_TIME_US) && (subt > 0))
        {
            LOG_WRN("execution time(%d) of startStream exceed 33ms, it is not unreasonable!!",subt);
            LOG_WRN("startStream of DpStream is too long!!\n");
            //AEE_WARNING_ASSERT_DEFAULT("\nCRDISPATCH_KEY:startStream of DpStream is too long!!\n");
        }
    }
#endif

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
    long a=0,b=0;
#if defined(DIP_USERDBG_AND_USER_LOAD_SUPPORT)
    MUINT32 subt=0;
#endif
    a=getMontonicUs();
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
    b=getMontonicUs();
    if(mdpRet == DP_STATUS_RETURN_SUCCESS){
        ret = MDPMGR_NO_ERROR;
    } else if(mdpRet == DP_STATUS_BUFFER_EMPTY) {
        ret = MDPMGR_VSS_NOT_READY;
    } else {
        LOG_ERR("[Error]dequeueBuffer deqIndex(%d) fail(%d)",deqIndex,mdpRet);

        if(cfgData.ispTpipeCfgInfo.wpe.top_wpe.wpe_en == 1){
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
        LOG_INF("+,cqIndx(%u/%u/%u),sceID(%u),deqIndex(%d),isWaitBuf(%d),\n\
            deqIndex(%d), dequeTime(%d us),DpStream(%p)",\
            cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx,\
            cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.drvScenario,deqIndex,cfgData.isWaitBuf,\
            deqIndex, b-a, pDpStream);
        //Check Deque Time, if the Deque is more than 200ms , assert it!
#if defined(DIP_USERDBG_AND_USER_LOAD_SUPPORT)
        if (MDPMGR_NO_ERROR == ret)
        {
            subt = b-a;
            if ((subt >= ISP_MDP_DEQUE_TIME_US) && (subt > 0))
            {
                LOG_WRN("execution time(%d) of ISP-MDP or ISP-only exceed 200ms, it is not unreasonable!!",subt); 
                LOG_WRN("dequeue time from MDP is too long!!");
                //AEE_ASSERT("\nCRDISPATCH_KEY:dequeue time from MDP is too long!!\n");
            }
        }
#endif
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
    (void)cfgData;
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
    LOG_DBG("+,cqIndx(%u/%u/%u),isWaitBuf(%d),DpStream(%p)",\
        cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx,\
        cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.isWaitBuf,\
        pDpStream);

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
                    m_pDpStream[i][j]->initPQService();
                    pthread_mutex_lock(&mMutex_DpStreamMap);
                    mDpStreamMap[i][j].refCqIdx=static_cast<E_ISP_DIP_CQ>(i);
                    mDpStreamMap[i][j].refRingBufIdx=j;
                    pthread_mutex_unlock(&mMutex_DpStreamMap);
                }
        }
        //venc stream for slow motion
        m_DpVencStream=new DpVEncStream(DpVEncStream::ISP_VR_STREAM);    //CC Scenario with main sensor
    }
    m_p2tpipedumpEnable = ::property_get_int32("vendor.camera.p2tpipedump.enable", 0);
    m_p2disableSkipTpipe = ::property_get_int32("vendor.camera.disable_p2skiptpipe", 0);
    m_p2disablePQ = ::property_get_int32("vendor.camera.P2MDP_PQ.disable", 0);

    android_atomic_inc(&mInitCount);

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

                    // Delete CC Stream
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
    LOG_DBG("+,cqIndx(%u),sceID(%u),cqPA(0x%lx),tpipVa(0x%lx),tpipPa(0x%lx),isWaitBuf(%d),venc(%d)", \
            cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.DesCqPa,(unsigned long)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
            cfgData.isWaitBuf,cfgData.dstPortCfg[ISP_MDP_DL_VENCO]);
    //_startMdp
    //NOTE, cuz we do not support ISP_MDP_DL_JPEGO anymore, we use ISP_MDP_DL_VENCO to judge we enable venc port or not
    if(cfgData.dstPortCfg[ISP_MDP_DL_VENCO] == 1)
    {
        DpVEncStream *pDpStream = NULL;
        pDpStream = selectDpVEncStream();
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpVencStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        err=_startMdp(MDPMGR_DPSTREAM_TYPE_VENC,cfgData,pDpStream,this);
    }

    else if (cfgData.ispTpipeCfgInfo.sw.tpipe_sel_mode == 0x10 && cfgData.ispTpipeCfgInfo.wpe.top_wpe.wpe_en == 1) //for WPE
    {
        DpIspStream *pDpStream = NULL;
        pDpStream = selectWpeStream(cfgData.drvScenario, cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx_wpe);
        if(pDpStream == NULL)
        {
            LOG_ERR("pWpeStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        err= _wpestartMdp(cfgData,pDpStream,this);
    }

    else
    {
        DpIspStream *pDpStream = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.RingBufIdx);
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpIspStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        err=_startMdp(MDPMGR_DPSTREAM_TYPE_ISP,cfgData,pDpStream,this);
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

//YWadd    else if (cfgData.ispTpipeCfgInfo.sw.tpipe_sel_mode == 0x10 && cfgData.ispTpipeCfgInfo.top.wpe_en == 1) //for WPE
    else if (0) //YWadd
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
    else if (cfgData.ispTpipeCfgInfo.sw.tpipe_sel_mode == 0x10 && cfgData.ispTpipeCfgInfo.wpe.top_wpe.wpe_en == 1)//for WPE
    {
        if(cfgData.ispTpipeCfgInfo.wpe.top_wpe.wpe_mdpcrop_en != 1)
        {
            goto EXIT;
        }
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

    else if (cfgData.ispTpipeCfgInfo.sw.tpipe_sel_mode == 0x10 && cfgData.ispTpipeCfgInfo.wpe.top_wpe.wpe_en == 1)//for WPE
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
    E_ISP_DIP_CQ refObjectCqIdx; //for selecting dpStream
    MUINT32 refObjectRingBufIdx; //for selecting dpStream


    pthread_mutex_lock(&mMutex_DpStreamMap);
    if ((flag == 1) || (flag == 3))
    {
        refObjectCqIdx = mDpStreamMap[cqIdx][refringbufidx].refCqIdx;
        refObjectRingBufIdx = mDpStreamMap[cqIdx][refringbufidx].refRingBufIdx;
    }
    else
    {
        refObjectCqIdx = static_cast<E_ISP_DIP_CQ>(cqIdx);
        refObjectRingBufIdx = ringbufIdx;
    }
    mDpStreamMap[cqIdx][ringbufIdx].refCqIdx=static_cast<E_ISP_DIP_CQ>(refObjectCqIdx);
    mDpStreamMap[cqIdx][ringbufIdx].refRingBufIdx=refObjectRingBufIdx;
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
        case eImgFmt_NV61:
            localDpColorFormat = DP_COLOR_NV61;
            break;
        case eImgFmt_MTK_YVU_P210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YVU_P210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_NV16:
            localDpColorFormat = DP_COLOR_NV16;
            break;
        case eImgFmt_MTK_YUV_P210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YUV_P210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_NV21:
            localDpColorFormat = DP_COLOR_NV21;
            break;
        case eImgFmt_MTK_YVU_P010:
            localDpColorFormat = DP_COLOR_NV21_10P;
            break;
        case eImgFmt_YVU_P010:
            localDpColorFormat = DP_COLOR_NV21_10L;
            break;
        case eImgFmt_NV12:
            localDpColorFormat = DP_COLOR_NV12;
            break;
        case eImgFmt_MTK_YUV_P010:
            localDpColorFormat = DP_COLOR_NV12_10P;
            break;
        case eImgFmt_YUV_P010:
            localDpColorFormat = DP_COLOR_NV12_10L;
            break;
        case eImgFmt_YV12:
            localDpColorFormat = DP_COLOR_YV12;
            break;
        case eImgFmt_I420:
            localDpColorFormat = DP_COLOR_I420;
            break;
        case eImgFmt_MTK_YUV_P010_3PLANE:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YUV_P010_3PLANE:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YV16:
            localDpColorFormat = DP_COLOR_YV16;
            break;
        case eImgFmt_I422:
            localDpColorFormat = DP_COLOR_I422;
            break;
        case eImgFmt_MTK_YUV_P210_3PLANE:
            localDpColorFormat = DP_COLOR_I422;//YWtodo
            break;
        case eImgFmt_YUV_P210_3PLANE:
            localDpColorFormat = DP_COLOR_I422;//YWtodo
            break;
        case eImgFmt_YUY2:
            localDpColorFormat = DP_COLOR_YUYV;
            break;
        case eImgFmt_MTK_YUYV_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YUYV_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_UYVY:
            localDpColorFormat = DP_COLOR_UYVY;
            break;
        case eImgFmt_MTK_UYVY_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_UYVY_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_VYUY:
            localDpColorFormat = DP_COLOR_VYUY;
            break;
        case eImgFmt_MTK_VYUY_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_VYUY_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YVYU:
            localDpColorFormat = DP_COLOR_VYUY;
            break;
        case eImgFmt_MTK_YVYU_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YVYU_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        //
        case eImgFmt_STA_BYTE:
        case eImgFmt_UFO_FG:
        case eImgFmt_Y800:  //Y plane only
            localDpColorFormat = DP_COLOR_GREY;
            break;
        case eImgFmt_RGB565:    //RGB 565 (16-bit), 1 plane
            localDpColorFormat = DP_COLOR_RGB565;
            break;
        case eImgFmt_RGB888:    //RGB 888 (24-bit), 1 plane
            localDpColorFormat = DP_COLOR_RGB888;
            break;
        case eImgFmt_ARGB888:   //ARGB (32-bit), 1 plane
            localDpColorFormat = DP_COLOR_ARGB8888;
            break;
        case eImgFmt_BAYER8:    //Bayer format, 8-bit
            localDpColorFormat = DP_COLOR_BAYER8;
            break;
        case eImgFmt_BAYER10:   //Bayer format, 10-bit
        case eImgFmt_BAYER10_MIPI:   //Bayer format, 10-bit(MIPI)
        case eImgFmt_UFO_BAYER10:
            localDpColorFormat = DP_COLOR_BAYER10;
            break;
        case eImgFmt_BAYER12:   //Bayer format, 12-bit
        case eImgFmt_UFO_BAYER12:
            localDpColorFormat = DP_COLOR_BAYER12;
            break;
        case eImgFmt_NV21_BLK:  //420 format block mode, 2 plane (UV)
            localDpColorFormat = DP_COLOR_420_BLKP;
            break;
        case eImgFmt_NV12_BLK:  //420 format block mode, 2 plane (VU)
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
        case eImgFmt_NV61:
            localDpColorFormat = DP_COLOR_NV61;
            break;
        case eImgFmt_MTK_YVU_P210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YVU_P210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_NV16:
            localDpColorFormat = DP_COLOR_NV16;
            break;
        case eImgFmt_MTK_YUV_P210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YUV_P210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_NV21:
            localDpColorFormat = DP_COLOR_NV21;
            break;
        case eImgFmt_MTK_YVU_P010:
            localDpColorFormat = DP_COLOR_NV21_10P;
            break;
        case eImgFmt_YVU_P010:
            localDpColorFormat = DP_COLOR_NV21_10L;
            break;
        case eImgFmt_NV12:
            localDpColorFormat = DP_COLOR_NV12;
            break;
        case eImgFmt_MTK_YUV_P010:
            localDpColorFormat = DP_COLOR_NV12_10P;
            break;
        case eImgFmt_YUV_P010:
            localDpColorFormat = DP_COLOR_NV12_10L;
            break;
        case eImgFmt_YV12:
            localDpColorFormat = DP_COLOR_YV12;
            break;
        case eImgFmt_I420:
            localDpColorFormat = DP_COLOR_I420;
            break;
        case eImgFmt_MTK_YUV_P010_3PLANE:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YUV_P010_3PLANE:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YV16:
            localDpColorFormat = DP_COLOR_YV16;
            break;
        case eImgFmt_I422:
            localDpColorFormat = DP_COLOR_I422;
            break;
        case eImgFmt_MTK_YUV_P210_3PLANE:
            localDpColorFormat = DP_COLOR_I422;//YWtodo
            break;
        case eImgFmt_YUV_P210_3PLANE:
            localDpColorFormat = DP_COLOR_I422;//YWtodo
            break;
        case eImgFmt_YUY2:
            localDpColorFormat = DP_COLOR_YUYV;
            break;
        case eImgFmt_MTK_YUYV_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YUYV_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_UYVY:
            localDpColorFormat = DP_COLOR_UYVY;
            break;
        case eImgFmt_MTK_UYVY_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_UYVY_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_VYUY:
            localDpColorFormat = DP_COLOR_VYUY;
            break;
        case eImgFmt_MTK_VYUY_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_VYUY_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YVYU:
            localDpColorFormat = DP_COLOR_VYUY;
            break;
        case eImgFmt_MTK_YVYU_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        case eImgFmt_YVYU_Y210:
            localDpColorFormat = DP_COLOR_YUYV;//YWtodo
            break;
        //
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
        LOG_INF("top.pixel_id                             (%d)"  ,a_info.top.pixel_id                             );
        LOG_INF("top.cam_in_fmt                           (%d)"  ,a_info.top.cam_in_fmt                           );
        LOG_INF("top.ctl_extension_en                     (%d)"  ,a_info.top.ctl_extension_en                     );
        LOG_INF("top.fg_mode                              (%d)"  ,a_info.top.fg_mode                              );
        LOG_INF("top.pakg_fg_out                          (%d)"  ,a_info.top.pakg_fg_out                          );
        LOG_INF("top.ufo_imgi_en                          (%d)"  ,a_info.top.ufo_imgi_en                          );
        LOG_INF("top.ufdi_fmt                             (%d)"  ,a_info.top.ufdi_fmt                             );
        LOG_INF("top.vipi_fmt                             (%d)"  ,a_info.top.vipi_fmt                             );
        LOG_INF("top.img3o_fmt                            (%d)"  ,a_info.top.img3o_fmt                            );
        LOG_INF("top.crzo_fmt                             (%d)"  ,a_info.top.crzo_fmt                             );
        LOG_INF("top.timgo_fmt                            (%d)"  ,a_info.top.timgo_fmt                            );
        LOG_INF("top.crp2_sel                             (%d)"  ,a_info.top.crp2_sel                             );
        LOG_INF("top.ynr_sel                              (%d)"  ,a_info.top.ynr_sel                              );
        LOG_INF("top.timgo_sel                            (%d)"  ,a_info.top.timgo_sel                            );
        LOG_INF("top.traw_sel                             (%d)"  ,a_info.top.traw_sel                             );
        LOG_INF("top.dgn_sel                              (%d)"  ,a_info.top.dgn_sel                              );
        LOG_INF("top.flc_sel                              (%d)"  ,a_info.top.flc_sel                              );
        LOG_INF("top.g2cx_sel                             (%d)"  ,a_info.top.g2cx_sel                             );
        LOG_INF("top.c24d1_sel                            (%d)"  ,a_info.top.c24d1_sel                            );
        LOG_INF("top.srz_sel                              (%d)"  ,a_info.top.srz_sel                              );
        LOG_INF("top.mix1_sel                             (%d)"  ,a_info.top.mix1_sel                             );
        LOG_INF("top.crz_sel                              (%d)"  ,a_info.top.crz_sel                              );
        LOG_INF("top.nr3d_pre_i_sel                       (%d)"  ,a_info.top.nr3d_pre_i_sel                       );
        LOG_INF("top.mix3_w_sel                           (%d)"  ,a_info.top.mix3_w_sel                           );
        LOG_INF("top.crsp_sel                             (%d)"  ,a_info.top.crsp_sel                             );
        LOG_INF("top.fe_sel                               (%d)"  ,a_info.top.fe_sel                               );
        LOG_INF("top.feo_sel                              (%d)"  ,a_info.top.feo_sel                              );
        LOG_INF("top.imgi_en                              (%d)"  ,a_info.top.imgi_en                              );
        LOG_INF("top.imgbi_en                             (%d)"  ,a_info.top.imgbi_en                             );
        LOG_INF("top.imgci_en                             (%d)"  ,a_info.top.imgci_en                             );
        LOG_INF("top.vipi_en                              (%d)"  ,a_info.top.vipi_en                              );
        LOG_INF("top.vipbi_en                             (%d)"  ,a_info.top.vipbi_en                             );
        LOG_INF("top.vipci_en                             (%d)"  ,a_info.top.vipci_en                             );
        LOG_INF("top.ufdi_en                              (%d)"  ,a_info.top.ufdi_en                              );
        LOG_INF("top.lcei_en                              (%d)"  ,a_info.top.lcei_en                              );
        LOG_INF("top.timgo_en                             (%d)"  ,a_info.top.timgo_en                             );
        LOG_INF("top.unp_en                               (%d)"  ,a_info.top.unp_en                               );
        LOG_INF("top.unp2_en                              (%d)"  ,a_info.top.unp2_en                              );
        LOG_INF("top.unp3_en                              (%d)"  ,a_info.top.unp3_en                              );
        LOG_INF("top.unp4_en                              (%d)"  ,a_info.top.unp4_en                              );
        LOG_INF("top.unp6_en                              (%d)"  ,a_info.top.unp6_en                              );
        LOG_INF("top.unp7_en                              (%d)"  ,a_info.top.unp7_en                              );
        LOG_INF("top.unp8_en                              (%d)"  ,a_info.top.unp8_en                              );
        LOG_INF("top.unp9_en                              (%d)"  ,a_info.top.unp9_en                              );
        LOG_INF("top.unp10_en                             (%d)"  ,a_info.top.unp10_en                             );
        LOG_INF("top.unp11_en                             (%d)"  ,a_info.top.unp11_en                             );
        LOG_INF("top.bs_en                                (%d)"  ,a_info.top.bs_en                                );
        LOG_INF("top.ufd_en                               (%d)"  ,a_info.top.ufd_en                               );
        LOG_INF("top.bpc_en                               (%d)"  ,a_info.top.bpc_en                               );
        LOG_INF("top.lsc_en                               (%d)"  ,a_info.top.lsc_en                               );
        LOG_INF("top.wb_en                                (%d)"  ,a_info.top.wb_en                                );
        LOG_INF("top.rcp_en                               (%d)"  ,a_info.top.rcp_en                               );
        LOG_INF("top.rcp2_en                              (%d)"  ,a_info.top.rcp2_en                              );
        LOG_INF("top.pak_en                               (%d)"  ,a_info.top.pak_en                               );
        LOG_INF("top.pak2_en                              (%d)"  ,a_info.top.pak2_en                              );
        LOG_INF("top.pak3_en                              (%d)"  ,a_info.top.pak3_en                              );
        LOG_INF("top.pak4_en                              (%d)"  ,a_info.top.pak4_en                              );
        LOG_INF("top.pak6_en                              (%d)"  ,a_info.top.pak6_en                              );
        LOG_INF("top.pak7_en                              (%d)"  ,a_info.top.pak7_en                              );
        LOG_INF("top.pak8_en                              (%d)"  ,a_info.top.pak8_en                              );
        LOG_INF("top.c24_en                               (%d)"  ,a_info.top.c24_en                               );
        LOG_INF("top.hlr_en                               (%d)"  ,a_info.top.hlr_en                               );
        LOG_INF("top.dgn_en                               (%d)"  ,a_info.top.dgn_en                               );
        LOG_INF("top.ltm_en                               (%d)"  ,a_info.top.ltm_en                               );
        LOG_INF("top.dm_en                                (%d)"  ,a_info.top.dm_en                                );
        LOG_INF("top.gdr_en                               (%d)"  ,a_info.top.gdr_en                               );
        LOG_INF("top.ldnr_en                              (%d)"  ,a_info.top.ldnr_en                              );
        LOG_INF("top.align_en                             (%d)"  ,a_info.top.align_en                             );
        LOG_INF("top.flc_en                               (%d)"  ,a_info.top.flc_en                               );
        LOG_INF("top.ccm_en                               (%d)"  ,a_info.top.ccm_en                               );
        LOG_INF("top.ccm2_en                              (%d)"  ,a_info.top.ccm2_en                              );
        LOG_INF("top.ggm_en                               (%d)"  ,a_info.top.ggm_en                               );
        LOG_INF("top.ggm2_en                              (%d)"  ,a_info.top.ggm2_en                              );
        LOG_INF("top.wsync_en                             (%d)"  ,a_info.top.wsync_en                             );
        LOG_INF("top.ggm4_en                              (%d)"  ,a_info.top.ggm4_en                              );
        LOG_INF("top.g2c4_en                              (%d)"  ,a_info.top.g2c4_en                              );
        LOG_INF("top.c42d4_en                             (%d)"  ,a_info.top.c42d4_en                             );
        LOG_INF("top.crp4_en                              (%d)"  ,a_info.top.crp4_en                              );
        LOG_INF("top.g2cx_en                              (%d)"  ,a_info.top.g2cx_en                              );
        LOG_INF("top.c42_en                               (%d)"  ,a_info.top.c42_en                               );
        LOG_INF("top.ynr_en                               (%d)"  ,a_info.top.ynr_en                               );
        LOG_INF("top.dmgi_en                              (%d)"  ,a_info.top.dmgi_en                              );
        LOG_INF("top.depi_en                              (%d)"  ,a_info.top.depi_en                              );
        LOG_INF("top.cnr_en                               (%d)"  ,a_info.top.cnr_en                               );
        LOG_INF("top.srz1_en                              (%d)"  ,a_info.top.srz1_en                              );
        LOG_INF("top.mix1_en                              (%d)"  ,a_info.top.mix1_en                              );
        LOG_INF("top.mix2_en                              (%d)"  ,a_info.top.mix2_en                              );
        LOG_INF("top.slk1_en                              (%d)"  ,a_info.top.slk1_en                              );
        LOG_INF("top.slk2_en                              (%d)"  ,a_info.top.slk2_en                              );
        LOG_INF("top.slk3_en                              (%d)"  ,a_info.top.slk3_en                              );
        LOG_INF("top.slk4_en                              (%d)"  ,a_info.top.slk4_en                              );
        LOG_INF("top.slk5_en                              (%d)"  ,a_info.top.slk5_en                              );
        LOG_INF("top.slk6_en                              (%d)"  ,a_info.top.slk6_en                              );
        LOG_INF("top.ndg_en                               (%d)"  ,a_info.top.ndg_en                               );
        LOG_INF("top.ndg2_en                              (%d)"  ,a_info.top.ndg2_en                              );
        LOG_INF("top.c24d3_en                             (%d)"  ,a_info.top.c24d3_en                             );
        LOG_INF("top.c24d2_en                             (%d)"  ,a_info.top.c24d2_en                             );
        LOG_INF("top.c2g_en                               (%d)"  ,a_info.top.c2g_en                               );
        LOG_INF("top.iggm_en                              (%d)"  ,a_info.top.iggm_en                              );
        LOG_INF("top.ccm3_en                              (%d)"  ,a_info.top.ccm3_en                              );
        LOG_INF("top.ggm3_en                              (%d)"  ,a_info.top.ggm3_en                              );
        LOG_INF("top.dce_en                               (%d)"  ,a_info.top.dce_en                               );
        LOG_INF("top.dces_en                              (%d)"  ,a_info.top.dces_en                              );
        LOG_INF("top.dceso_en                             (%d)"  ,a_info.top.dceso_en                             );
        LOG_INF("top.g2c_en                               (%d)"  ,a_info.top.g2c_en                               );
        LOG_INF("top.c42d2_en                             (%d)"  ,a_info.top.c42d2_en                             );
        LOG_INF("top.ee_en                                (%d)"  ,a_info.top.ee_en                                );
        LOG_INF("top.lce_en                               (%d)"  ,a_info.top.lce_en                               );
        LOG_INF("top.mix3_en                              (%d)"  ,a_info.top.mix3_en                              );
        LOG_INF("top.mix4_en                              (%d)"  ,a_info.top.mix4_en                              );
        LOG_INF("top.crz_en                               (%d)"  ,a_info.top.crz_en                               );
        LOG_INF("top.feo_en                               (%d)"  ,a_info.top.feo_en                               );
        LOG_INF("top.c02_en                               (%d)"  ,a_info.top.c02_en                               );
        LOG_INF("top.c02d2_en                             (%d)"  ,a_info.top.c02d2_en                             );
        LOG_INF("top.nr3d_en                              (%d)"  ,a_info.top.nr3d_en                              );
        LOG_INF("top.color_en                             (%d)"  ,a_info.top.color_en                             );
        LOG_INF("top.crsp_en                              (%d)"  ,a_info.top.crsp_en                              );
        LOG_INF("top.img3o_en                             (%d)"  ,a_info.top.img3o_en                             );
        LOG_INF("top.img3bo_en                            (%d)"  ,a_info.top.img3bo_en                            );
        LOG_INF("top.img3co_en                            (%d)"  ,a_info.top.img3co_en                            );
        LOG_INF("top.mcrp_en                              (%d)"  ,a_info.top.mcrp_en                              );
        LOG_INF("top.mcrp2_en                             (%d)"  ,a_info.top.mcrp2_en                             );
        LOG_INF("top.plnr1_en                             (%d)"  ,a_info.top.plnr1_en                             );
        LOG_INF("top.plnr2_en                             (%d)"  ,a_info.top.plnr2_en                             );
        LOG_INF("top.plnw1_en                             (%d)"  ,a_info.top.plnw1_en                             );
        LOG_INF("top.plnw2_en                             (%d)"  ,a_info.top.plnw2_en                             );
        LOG_INF("top.crzo_en                              (%d)"  ,a_info.top.crzo_en                              );
        LOG_INF("top.crzbo_en                             (%d)"  ,a_info.top.crzbo_en                             );
        LOG_INF("top.dfe_en                               (%d)"  ,a_info.top.dfe_en                               );
        LOG_INF("top.obc_en                               (%d)"  ,a_info.top.obc_en                               );
        LOG_INF("top.zfus_en                              (%d)"  ,a_info.top.zfus_en                              );
        LOG_INF("top.srz3_en                              (%d)"  ,a_info.top.srz3_en                              );
        LOG_INF("top.srz4_en                              (%d)"  ,a_info.top.srz4_en                              );
        LOG_INF("top.smt1_en                              (%d)"  ,a_info.top.smt1_en                              );
        LOG_INF("top.smt1i_en                             (%d)"  ,a_info.top.smt1i_en                             );
        LOG_INF("top.smt1o_en                             (%d)"  ,a_info.top.smt1o_en                             );
        LOG_INF("top.smt2_en                              (%d)"  ,a_info.top.smt2_en                              );
        LOG_INF("top.smt2i_en                             (%d)"  ,a_info.top.smt2i_en                             );
        LOG_INF("top.smt2o_en                             (%d)"  ,a_info.top.smt2o_en                             );
        LOG_INF("top.smt3_en                              (%d)"  ,a_info.top.smt3_en                              );
        LOG_INF("top.smt3i_en                             (%d)"  ,a_info.top.smt3i_en                             );
        LOG_INF("top.smt3o_en                             (%d)"  ,a_info.top.smt3o_en                             );
        LOG_INF("top.smt4_en                              (%d)"  ,a_info.top.smt4_en                              );
        LOG_INF("top.smt4i_en                             (%d)"  ,a_info.top.smt4i_en                             );
        LOG_INF("top.smt4o_en                             (%d)"  ,a_info.top.smt4o_en                             );
        LOG_INF("top.adl_en                               (%d)"  ,a_info.top.adl_en                               );
        LOG_INF("sw.log_en                                (%d)"  ,a_info.sw.log_en                                );
        LOG_INF("sw.tpipe_sel_mode                        (0x%x)",a_info.sw.tpipe_sel_mode                        );
        LOG_INF("sw.tpipe_irq_mode                        (%d)"  ,a_info.sw.tpipe_irq_mode                        );
        LOG_INF("sw.src_width                             (%d)"  ,a_info.sw.src_width                             );
        LOG_INF("sw.src_height                            (%d)"  ,a_info.sw.src_height                            );
        LOG_INF("sw.tpipe_width                           (%d)"  ,a_info.sw.tpipe_width                           );
        LOG_INF("sw.tpipe_height                          (%d)"  ,a_info.sw.tpipe_height                          );
        LOG_INF("adl.adl_ctl_en                           (%d)"  ,a_info.adl.adl_ctl_en                           );
        LOG_INF("adl.ipui_stride                          (%d)"  ,a_info.adl.ipui_stride                          );
        LOG_INF("adl.ipuo_stride                          (%d)"  ,a_info.adl.ipuo_stride                          );
        LOG_INF("imgi.imgi_v_flip_en                      (%d)"  ,a_info.imgi.imgi_v_flip_en                      );
        LOG_INF("imgi.imgi_stride                         (%d)"  ,a_info.imgi.imgi_stride                         );
        LOG_INF("imgbi.imgbi_offset                       (%d)"  ,a_info.imgbi.imgbi_offset                       );
        LOG_INF("imgbi.imgbi_xsize                        (%d)"  ,a_info.imgbi.imgbi_xsize                        );
        LOG_INF("imgbi.imgbi_ysize                        (%d)"  ,a_info.imgbi.imgbi_ysize                        );
        LOG_INF("imgbi.imgbi_stride                       (%d)"  ,a_info.imgbi.imgbi_stride                       );
        LOG_INF("imgci.imgci_v_flip_en                    (%d)"  ,a_info.imgci.imgci_v_flip_en                    );
        LOG_INF("imgci.imgci_stride                       (%d)"  ,a_info.imgci.imgci_stride                       );
        LOG_INF("vipi.vipi_v_flip_en                      (%d)"  ,a_info.vipi.vipi_v_flip_en                      );
        LOG_INF("vipi.vipi_xsize                          (%d)"  ,a_info.vipi.vipi_xsize                          );
        LOG_INF("vipi.vipi_ysize                          (%d)"  ,a_info.vipi.vipi_ysize                          );
        LOG_INF("vipi.vipi_stride                         (%d)"  ,a_info.vipi.vipi_stride                         );
        LOG_INF("vipbi.vipbi_v_flip_en                    (%d)"  ,a_info.vipbi.vipbi_v_flip_en                    );
        LOG_INF("vipbi.vipbi_xsize                        (%d)"  ,a_info.vipbi.vipbi_xsize                        );
        LOG_INF("vipbi.vipbi_ysize                        (%d)"  ,a_info.vipbi.vipbi_ysize                        );
        LOG_INF("vipbi.vipbi_stride                       (%d)"  ,a_info.vipbi.vipbi_stride                       );
        LOG_INF("vipci.vipci_v_flip_en                    (%d)"  ,a_info.vipci.vipci_v_flip_en                    );
        LOG_INF("vipci.vipci_xsize                        (%d)"  ,a_info.vipci.vipci_xsize                        );
        LOG_INF("vipci.vipci_ysize                        (%d)"  ,a_info.vipci.vipci_ysize                        );
        LOG_INF("vipci.vipci_stride                       (%d)"  ,a_info.vipci.vipci_stride                       );
        LOG_INF("ufdi.ufdi_v_flip_en                      (%d)"  ,a_info.ufdi.ufdi_v_flip_en                      );
        LOG_INF("ufdi.ufdi_xsize                          (%d)"  ,a_info.ufdi.ufdi_xsize                          );
        LOG_INF("ufdi.ufdi_ysize                          (%d)"  ,a_info.ufdi.ufdi_ysize                          );
        LOG_INF("ufdi.ufdi_stride                         (%d)"  ,a_info.ufdi.ufdi_stride                         );
        LOG_INF("lcei.lcei_v_flip_en                      (%d)"  ,a_info.lcei.lcei_v_flip_en                      );
        LOG_INF("lcei.lcei_xsize                          (%d)"  ,a_info.lcei.lcei_xsize                          );
        LOG_INF("lcei.lcei_ysize                          (%d)"  ,a_info.lcei.lcei_ysize                          );
        LOG_INF("lcei.lcei_stride                         (%d)"  ,a_info.lcei.lcei_stride                         );
        LOG_INF("dmgi.dmgi_v_flip_en                      (%d)"  ,a_info.dmgi.dmgi_v_flip_en                      );
        LOG_INF("dmgi.dmgi_stride                         (%d)"  ,a_info.dmgi.dmgi_stride                         );
        LOG_INF("dmgi.dmgi_offset                         (%d)"  ,a_info.dmgi.dmgi_offset                         );
        LOG_INF("dmgi.dmgi_xsize                          (%d)"  ,a_info.dmgi.dmgi_xsize                          );
        LOG_INF("dmgi.dmgi_ysize                          (%d)"  ,a_info.dmgi.dmgi_ysize                          );
        LOG_INF("depi.depi_v_flip_en                      (%d)"  ,a_info.depi.depi_v_flip_en                      );
        LOG_INF("depi.depi_stride                         (%d)"  ,a_info.depi.depi_stride                         );
        LOG_INF("depi.depi_xsize                          (%d)"  ,a_info.depi.depi_xsize                          );
        LOG_INF("depi.depi_ysize                          (%d)"  ,a_info.depi.depi_ysize                          );
        LOG_INF("timgo.timgo_stride                       (%d)"  ,a_info.timgo.timgo_stride                       );
        LOG_INF("timgo.timgo_xoffset                      (%d)"  ,a_info.timgo.timgo_xoffset                      );
        LOG_INF("timgo.timgo_yoffset                      (%d)"  ,a_info.timgo.timgo_yoffset                      );
        LOG_INF("timgo.timgo_xsize                        (%d)"  ,a_info.timgo.timgo_xsize                        );
        LOG_INF("timgo.timgo_ysize                        (%d)"  ,a_info.timgo.timgo_ysize                        );
        LOG_INF("dceso.dceso_xsize                        (%d)"  ,a_info.dceso.dceso_xsize                        );
        LOG_INF("dceso.dceso_ysize                        (%d)"  ,a_info.dceso.dceso_ysize                        );
        LOG_INF("img3o.img3o_stride                       (%d)"  ,a_info.img3o.img3o_stride                       );
        LOG_INF("img3o.img3o_xoffset                      (%d)"  ,a_info.img3o.img3o_xoffset                      );
        LOG_INF("img3o.img3o_yoffset                      (%d)"  ,a_info.img3o.img3o_yoffset                      );
        LOG_INF("img3o.img3o_xsize                        (%d)"  ,a_info.img3o.img3o_xsize                        );
        LOG_INF("img3o.img3o_ysize                        (%d)"  ,a_info.img3o.img3o_ysize                        );
        LOG_INF("img3bo.img3bo_stride                     (%d)"  ,a_info.img3bo.img3bo_stride                     );
        LOG_INF("img3bo.img3bo_xsize                      (%d)"  ,a_info.img3bo.img3bo_xsize                      );
        LOG_INF("img3bo.img3bo_ysize                      (%d)"  ,a_info.img3bo.img3bo_ysize                      );
        LOG_INF("img3co.img3co_stride                     (%d)"  ,a_info.img3co.img3co_stride                     );
        LOG_INF("img3co.img3co_xsize                      (%d)"  ,a_info.img3co.img3co_xsize                      );
        LOG_INF("img3co.img3co_ysize                      (%d)"  ,a_info.img3co.img3co_ysize                      );
        LOG_INF("crzo.crzo_stride                         (%d)"  ,a_info.crzo.crzo_stride                         );
        LOG_INF("crzo.crzo_xoffset                        (%d)"  ,a_info.crzo.crzo_xoffset                        );
        LOG_INF("crzo.crzo_yoffset                        (%d)"  ,a_info.crzo.crzo_yoffset                        );
        LOG_INF("crzo.crzo_xsize                          (%d)"  ,a_info.crzo.crzo_xsize                          );
        LOG_INF("crzo.crzo_ysize                          (%d)"  ,a_info.crzo.crzo_ysize                          );
        LOG_INF("crzbo.crzbo_stride                       (%d)"  ,a_info.crzbo.crzbo_stride                       );
        LOG_INF("crzbo.crzbo_xoffset                      (%d)"  ,a_info.crzbo.crzbo_xoffset                      );
        LOG_INF("crzbo.crzbo_yoffset                      (%d)"  ,a_info.crzbo.crzbo_yoffset                      );
        LOG_INF("crzbo.crzbo_xsize                        (%d)"  ,a_info.crzbo.crzbo_xsize                        );
        LOG_INF("crzbo.crzbo_ysize                        (%d)"  ,a_info.crzbo.crzbo_ysize                        );
        LOG_INF("feo.feo_stride                           (%d)"  ,a_info.feo.feo_stride                           );
        LOG_INF("ufd.ufd_bs2_au_start                     (%d)"  ,a_info.ufd.ufd_bs2_au_start                     );
        LOG_INF("ufd.ufd_bs3_au_start                     (%d)"  ,a_info.ufd.ufd_bs3_au_start                     );
        LOG_INF("ufd.ufd_bond_mode                        (%d)"  ,a_info.ufd.ufd_bond_mode                        );
        LOG_INF("ufd.ufd_bond2_mode                       (%d)"  ,a_info.ufd.ufd_bond2_mode                       );
        LOG_INF("ufd.ufd_sel                              (%d)"  ,a_info.ufd.ufd_sel                              );
        LOG_INF("lsc.extend_coef_mode                     (%d)"  ,a_info.lsc.extend_coef_mode                     );
        LOG_INF("lsc.sdblk_width                          (%d)"  ,a_info.lsc.sdblk_width                          );
        LOG_INF("lsc.sdblk_xnum                           (%d)"  ,a_info.lsc.sdblk_xnum                           );
        LOG_INF("lsc.sdblk_last_width                     (%d)"  ,a_info.lsc.sdblk_last_width                     );
        LOG_INF("lsc.sdblk_height                         (%d)"  ,a_info.lsc.sdblk_height                         );
        LOG_INF("lsc.sdblk_ynum                           (%d)"  ,a_info.lsc.sdblk_ynum                           );
        LOG_INF("lsc.sdblk_last_height                    (%d)"  ,a_info.lsc.sdblk_last_height                    );
        LOG_INF("ltm.blk_x_num                            (%d)"  ,a_info.ltm.blk_x_num                            );
        LOG_INF("ltm.blk_y_num                            (%d)"  ,a_info.ltm.blk_y_num                            );
        LOG_INF("ltm.blk_width                            (%d)"  ,a_info.ltm.blk_width                            );
        LOG_INF("ltm.blk_height                           (%d)"  ,a_info.ltm.blk_height                           );
        LOG_INF("unp2.yuv_bit                             (%d)"  ,a_info.unp2.yuv_bit                             );
        LOG_INF("unp2.yuv_dng                             (%d)"  ,a_info.unp2.yuv_dng                             );
        LOG_INF("unp3.yuv_bit                             (%d)"  ,a_info.unp3.yuv_bit                             );
        LOG_INF("unp3.yuv_dng                             (%d)"  ,a_info.unp3.yuv_dng                             );
        LOG_INF("unp4.yuv_bit                             (%d)"  ,a_info.unp4.yuv_bit                             );
        LOG_INF("unp4.yuv_dng                             (%d)"  ,a_info.unp4.yuv_dng                             );
        LOG_INF("unp6.yuv_bit                             (%d)"  ,a_info.unp6.yuv_bit                             );
        LOG_INF("unp6.yuv_dng                             (%d)"  ,a_info.unp6.yuv_dng                             );
        LOG_INF("unp7.yuv_bit                             (%d)"  ,a_info.unp7.yuv_bit                             );
        LOG_INF("unp7.yuv_dng                             (%d)"  ,a_info.unp7.yuv_dng                             );
        LOG_INF("unp8.yuv_bit                             (%d)"  ,a_info.unp8.yuv_bit                             );
        LOG_INF("unp8.yuv_dng                             (%d)"  ,a_info.unp8.yuv_dng                             );
        LOG_INF("unp9.yuv_bit                             (%d)"  ,a_info.unp9.yuv_bit                             );
        LOG_INF("unp9.yuv_dng                             (%d)"  ,a_info.unp9.yuv_dng                             );
        LOG_INF("unp10.yuv_bit                            (%d)"  ,a_info.unp10.yuv_bit                            );
        LOG_INF("unp10.yuv_dng                            (%d)"  ,a_info.unp10.yuv_dng                            );
        LOG_INF("unp11.yuv_bit                            (%d)"  ,a_info.unp11.yuv_bit                            );
        LOG_INF("unp11.yuv_dng                            (%d)"  ,a_info.unp11.yuv_dng                            );
        LOG_INF("pak2.yuv_bit                             (%d)"  ,a_info.pak2.yuv_bit                             );
        LOG_INF("pak2.yuv_dng                             (%d)"  ,a_info.pak2.yuv_dng                             );
        LOG_INF("pak3.yuv_bit                             (%d)"  ,a_info.pak3.yuv_bit                             );
        LOG_INF("pak3.yuv_dng                             (%d)"  ,a_info.pak3.yuv_dng                             );
        LOG_INF("pak4.yuv_bit                             (%d)"  ,a_info.pak4.yuv_bit                             );
        LOG_INF("pak4.yuv_dng                             (%d)"  ,a_info.pak4.yuv_dng                             );
        LOG_INF("pak6.yuv_bit                             (%d)"  ,a_info.pak6.yuv_bit                             );
        LOG_INF("pak6.yuv_dng                             (%d)"  ,a_info.pak6.yuv_dng                             );
        LOG_INF("pak7.yuv_bit                             (%d)"  ,a_info.pak7.yuv_bit                             );
        LOG_INF("pak7.yuv_dng                             (%d)"  ,a_info.pak7.yuv_dng                             );
        LOG_INF("pak8.yuv_bit                             (%d)"  ,a_info.pak8.yuv_bit                             );
        LOG_INF("pak8.yuv_dng                             (%d)"  ,a_info.pak8.yuv_dng                             );
        LOG_INF("dm.dm_byp                                (%d)"  ,a_info.dm.dm_byp                                );
        LOG_INF("g2cx.g2cx_shade_en                       (%d)"  ,a_info.g2cx.g2cx_shade_en                       );
        LOG_INF("g2cx.g2cx_shade_xmid                     (%d)"  ,a_info.g2cx.g2cx_shade_xmid                     );
        LOG_INF("g2cx.g2cx_shade_ymid                     (%d)"  ,a_info.g2cx.g2cx_shade_ymid                     );
        LOG_INF("g2cx.g2cx_shade_var                      (%d)"  ,a_info.g2cx.g2cx_shade_var                      );
        LOG_INF("ynr.eny                                  (%d)"  ,a_info.ynr.eny                                  );
        LOG_INF("ynr.enc                                  (%d)"  ,a_info.ynr.enc                                  );
        LOG_INF("ynr.video_mode                           (%d)"  ,a_info.ynr.video_mode                           );
        LOG_INF("ynr.skin_link                            (%d)"  ,a_info.ynr.skin_link                            );
        LOG_INF("ynr.lce_link                             (%d)"  ,a_info.ynr.lce_link                             );
        LOG_INF("cnr.bpc_en                               (%d)"  ,a_info.cnr.bpc_en                               );
        LOG_INF("cnr.enc                                  (%d)"  ,a_info.cnr.enc                                  );
        LOG_INF("cnr.scale_mode                           (%d)"  ,a_info.cnr.scale_mode                           );
        LOG_INF("cnr.mode                                 (%d)"  ,a_info.cnr.mode                                 );
        LOG_INF("cnr.video_mode                           (%d)"  ,a_info.cnr.video_mode                           );
        LOG_INF("cnr.abf_en                               (%d)"  ,a_info.cnr.abf_en                               );
        LOG_INF("slk1.slk_hrz_comp                        (%d)"  ,a_info.slk1.slk_hrz_comp                        );
        LOG_INF("slk1.slk_vrz_comp                        (%d)"  ,a_info.slk1.slk_vrz_comp                        );
        LOG_INF("slk2.slk_hrz_comp                        (%d)"  ,a_info.slk2.slk_hrz_comp                        );
        LOG_INF("slk2.slk_vrz_comp                        (%d)"  ,a_info.slk2.slk_vrz_comp                        );
        LOG_INF("slk3.slk_hrz_comp                        (%d)"  ,a_info.slk3.slk_hrz_comp                        );
        LOG_INF("slk3.slk_vrz_comp                        (%d)"  ,a_info.slk3.slk_vrz_comp                        );
        LOG_INF("slk4.slk_hrz_comp                        (%d)"  ,a_info.slk4.slk_hrz_comp                        );
        LOG_INF("slk4.slk_vrz_comp                        (%d)"  ,a_info.slk4.slk_vrz_comp                        );
        LOG_INF("slk5.slk_hrz_comp                        (%d)"  ,a_info.slk5.slk_hrz_comp                        );
        LOG_INF("slk5.slk_vrz_comp                        (%d)"  ,a_info.slk5.slk_vrz_comp                        );
        LOG_INF("slk6.slk_hrz_comp                        (%d)"  ,a_info.slk6.slk_hrz_comp                        );
        LOG_INF("slk6.slk_vrz_comp                        (%d)"  ,a_info.slk6.slk_vrz_comp                        );
        LOG_INF("ee.ee_out_edge_sel                       (%d)"  ,a_info.ee.ee_out_edge_sel                       );
        LOG_INF("lce.lce_lc_tone                          (%d)"  ,a_info.lce.lce_lc_tone                          );
        LOG_INF("lce.lce_bc_mag_kubnx                     (%d)"  ,a_info.lce.lce_bc_mag_kubnx                     );
        LOG_INF("lce.lce_slm_width                        (%d)"  ,a_info.lce.lce_slm_width                        );
        LOG_INF("lce.lce_bc_mag_kubny                     (%d)"  ,a_info.lce.lce_bc_mag_kubny                     );
        LOG_INF("lce.lce_slm_height                       (%d)"  ,a_info.lce.lce_slm_height                       );
        LOG_INF("lce.lce_full_xoff                        (%d)"  ,a_info.lce.lce_full_xoff                        );
        LOG_INF("lce.lce_full_yoff                        (%d)"  ,a_info.lce.lce_full_yoff                        );
        LOG_INF("lce.lce_full_slm_width                   (%d)"  ,a_info.lce.lce_full_slm_width                   );
        LOG_INF("lce.lce_full_slm_height                  (%d)"  ,a_info.lce.lce_full_slm_height                  );
        LOG_INF("lce.lce_full_out_height                  (%d)"  ,a_info.lce.lce_full_out_height                  );
        LOG_INF("dces.dces_crop_xstart                    (%d)"  ,a_info.dces.dces_crop_xstart                    );
        LOG_INF("dces.dces_crop_xend                      (%d)"  ,a_info.dces.dces_crop_xend                      );
        LOG_INF("dces.dces_crop_ystart                    (%d)"  ,a_info.dces.dces_crop_ystart                    );
        LOG_INF("dces.dces_crop_yend                      (%d)"  ,a_info.dces.dces_crop_yend                      );
        LOG_INF("srz1.srz_input_crop_width                (%d)"  ,a_info.srz1.srz_input_crop_width                );
        LOG_INF("srz1.srz_input_crop_height               (%d)"  ,a_info.srz1.srz_input_crop_height               );
        LOG_INF("srz1.srz_output_width                    (%d)"  ,a_info.srz1.srz_output_width                    );
        LOG_INF("srz1.srz_output_height                   (%d)"  ,a_info.srz1.srz_output_height                   );
        LOG_INF("srz1.srz_luma_horizontal_integer_offset  (%d)"  ,a_info.srz1.srz_luma_horizontal_integer_offset  );
        LOG_INF("srz1.srz_luma_horizontal_subpixel_offset (%d)"  ,a_info.srz1.srz_luma_horizontal_subpixel_offset );
        LOG_INF("srz1.srz_luma_vertical_integer_offset    (%d)"  ,a_info.srz1.srz_luma_vertical_integer_offset    );
        LOG_INF("srz1.srz_luma_vertical_subpixel_offset   (%d)"  ,a_info.srz1.srz_luma_vertical_subpixel_offset   );
        LOG_INF("srz1.srz_horizontal_coeff_step           (%d)"  ,a_info.srz1.srz_horizontal_coeff_step           );
        LOG_INF("srz1.srz_vertical_coeff_step             (%d)"  ,a_info.srz1.srz_vertical_coeff_step             );
        LOG_INF("srz3.srz_input_crop_width                (%d)"  ,a_info.srz3.srz_input_crop_width                );
        LOG_INF("srz3.srz_input_crop_height               (%d)"  ,a_info.srz3.srz_input_crop_height               );
        LOG_INF("srz3.srz_output_width                    (%d)"  ,a_info.srz3.srz_output_width                    );
        LOG_INF("srz3.srz_output_height                   (%d)"  ,a_info.srz3.srz_output_height                   );
        LOG_INF("srz3.srz_luma_horizontal_integer_offset  (%d)"  ,a_info.srz3.srz_luma_horizontal_integer_offset  );
        LOG_INF("srz3.srz_luma_horizontal_subpixel_offset (%d)"  ,a_info.srz3.srz_luma_horizontal_subpixel_offset );
        LOG_INF("srz3.srz_luma_vertical_integer_offset    (%d)"  ,a_info.srz3.srz_luma_vertical_integer_offset    );
        LOG_INF("srz3.srz_luma_vertical_subpixel_offset   (%d)"  ,a_info.srz3.srz_luma_vertical_subpixel_offset   );
        LOG_INF("srz3.srz_horizontal_coeff_step           (%d)"  ,a_info.srz3.srz_horizontal_coeff_step           );
        LOG_INF("srz3.srz_vertical_coeff_step             (%d)"  ,a_info.srz3.srz_vertical_coeff_step             );
        LOG_INF("srz4.srz_input_crop_width                (%d)"  ,a_info.srz4.srz_input_crop_width                );
        LOG_INF("srz4.srz_input_crop_height               (%d)"  ,a_info.srz4.srz_input_crop_height               );
        LOG_INF("srz4.srz_output_width                    (%d)"  ,a_info.srz4.srz_output_width                    );
        LOG_INF("srz4.srz_output_height                   (%d)"  ,a_info.srz4.srz_output_height                   );
        LOG_INF("srz4.srz_luma_horizontal_integer_offset  (%d)"  ,a_info.srz4.srz_luma_horizontal_integer_offset  );
        LOG_INF("srz4.srz_luma_horizontal_subpixel_offset (%d)"  ,a_info.srz4.srz_luma_horizontal_subpixel_offset );
        LOG_INF("srz4.srz_luma_vertical_integer_offset    (%d)"  ,a_info.srz4.srz_luma_vertical_integer_offset    );
        LOG_INF("srz4.srz_luma_vertical_subpixel_offset   (%d)"  ,a_info.srz4.srz_luma_vertical_subpixel_offset   );
        LOG_INF("srz4.srz_horizontal_coeff_step           (%d)"  ,a_info.srz4.srz_horizontal_coeff_step           );
        LOG_INF("srz4.srz_vertical_coeff_step             (%d)"  ,a_info.srz4.srz_vertical_coeff_step             );
        LOG_INF("crz.crz_input_crop_width                 (%d)"  ,a_info.crz.crz_input_crop_width                 );
        LOG_INF("crz.crz_input_crop_height                (%d)"  ,a_info.crz.crz_input_crop_height                );
        LOG_INF("crz.crz_output_width                     (%d)"  ,a_info.crz.crz_output_width                     );
        LOG_INF("crz.crz_output_height                    (%d)"  ,a_info.crz.crz_output_height                    );
        LOG_INF("crz.crz_luma_horizontal_integer_offset   (%d)"  ,a_info.crz.crz_luma_horizontal_integer_offset   );
        LOG_INF("crz.crz_luma_horizontal_subpixel_offset  (%d)"  ,a_info.crz.crz_luma_horizontal_subpixel_offset  );
        LOG_INF("crz.crz_luma_vertical_integer_offset     (%d)"  ,a_info.crz.crz_luma_vertical_integer_offset     );
        LOG_INF("crz.crz_luma_vertical_subpixel_offset    (%d)"  ,a_info.crz.crz_luma_vertical_subpixel_offset    );
        LOG_INF("crz.crz_horizontal_luma_algorithm        (%d)"  ,a_info.crz.crz_horizontal_luma_algorithm        );
        LOG_INF("crz.crz_vertical_luma_algorithm          (%d)"  ,a_info.crz.crz_vertical_luma_algorithm          );
        LOG_INF("crz.crz_horizontal_coeff_step            (%d)"  ,a_info.crz.crz_horizontal_coeff_step            );
        LOG_INF("crz.crz_vertical_coeff_step              (%d)"  ,a_info.crz.crz_vertical_coeff_step              );
        LOG_INF("dfe.dfe_mode                             (%d)"  ,a_info.dfe.dfe_mode                             );
        LOG_INF("nr3d.nr3d_on_en                          (%d)"  ,a_info.nr3d.nr3d_on_en                          );
        LOG_INF("nr3d.nr3d_snr_en                         (%d)"  ,a_info.nr3d.nr3d_snr_en                         );
        LOG_INF("nr3d.nr3d_on_xoffset                     (%d)"  ,a_info.nr3d.nr3d_on_xoffset                     );
        LOG_INF("nr3d.nr3d_on_yoffset                     (%d)"  ,a_info.nr3d.nr3d_on_yoffset                     );
        LOG_INF("nr3d.nr3d_on_width                       (%d)"  ,a_info.nr3d.nr3d_on_width                       );
        LOG_INF("nr3d.nr3d_on_height                      (%d)"  ,a_info.nr3d.nr3d_on_height                      );
        LOG_INF("crsp.crsp_ystep                          (%d)"  ,a_info.crsp.crsp_ystep                          );
        LOG_INF("crsp.crsp_xoffset                        (%d)"  ,a_info.crsp.crsp_xoffset                        );
        LOG_INF("crsp.crsp_yoffset                        (%d)"  ,a_info.crsp.crsp_yoffset                        );
        LOG_INF("smt1.smt1o_sel                           (%d)"  ,a_info.smt1.smt1o_sel                           );
        LOG_INF("smt2.smt2o_sel                           (%d)"  ,a_info.smt2.smt2o_sel                           );
        LOG_INF("smt3.smt3o_sel                           (%d)"  ,a_info.smt3.smt3o_sel                           );
        LOG_INF("smt4.smt4o_sel                           (%d)"  ,a_info.smt4.smt4o_sel                           );
        LOG_INF("smt1i.smt1i_v_flip_en                    (%d)"  ,a_info.smt1i.smt1i_v_flip_en                    );
        LOG_INF("smt1i.smt1i_xsize                        (%d)"  ,a_info.smt1i.smt1i_xsize                        );
        LOG_INF("smt1i.smt1i_ysize                        (%d)"  ,a_info.smt1i.smt1i_ysize                        );
        LOG_INF("smt1o.smt1o_xsize                        (%d)"  ,a_info.smt1o.smt1o_xsize                        );
        LOG_INF("smt1o.smt1o_ysize                        (%d)"  ,a_info.smt1o.smt1o_ysize                        );
        LOG_INF("smt2i.smt2i_v_flip_en                    (%d)"  ,a_info.smt2i.smt2i_v_flip_en                    );
        LOG_INF("smt2i.smt2i_xsize                        (%d)"  ,a_info.smt2i.smt2i_xsize                        );
        LOG_INF("smt2i.smt2i_ysize                        (%d)"  ,a_info.smt2i.smt2i_ysize                        );
        LOG_INF("smt2o.smt2o_xsize                        (%d)"  ,a_info.smt2o.smt2o_xsize                        );
        LOG_INF("smt2o.smt2o_ysize                        (%d)"  ,a_info.smt2o.smt2o_ysize                        );
        LOG_INF("smt3i.smt3i_v_flip_en                    (%d)"  ,a_info.smt3i.smt3i_v_flip_en                    );
        LOG_INF("smt3i.smt3i_xsize                        (%d)"  ,a_info.smt3i.smt3i_xsize                        );
        LOG_INF("smt3i.smt3i_ysize                        (%d)"  ,a_info.smt3i.smt3i_ysize                        );
        LOG_INF("smt3o.smt3o_xsize                        (%d)"  ,a_info.smt3o.smt3o_xsize                        );
        LOG_INF("smt3o.smt3o_ysize                        (%d)"  ,a_info.smt3o.smt3o_ysize                        );
        LOG_INF("smt4i.smt4i_v_flip_en                    (%d)"  ,a_info.smt4i.smt4i_v_flip_en                    );
        LOG_INF("smt4i.smt4i_xsize                        (%d)"  ,a_info.smt4i.smt4i_xsize                        );
        LOG_INF("smt4i.smt4i_ysize                        (%d)"  ,a_info.smt4i.smt4i_ysize                        );
        LOG_INF("smt4o.smt4o_xsize                        (%d)"  ,a_info.smt4o.smt4o_xsize                        );
        LOG_INF("smt4o.smt4o_ysize                        (%d)"  ,a_info.smt4o.smt4o_ysize                        );

        LOG_INF("mfb_en                                   (%d)"  ,a_info.top.mfb_en                               );
        LOG_INF("mfbi_en                                  (%d)"  ,a_info.top.mfbi_en                              );
        LOG_INF("mfbi_b_en                                (%d)"  ,a_info.top.mfbi_b_en                            );
        LOG_INF("mfb3i_en                                 (%d)"  ,a_info.top.mfb3i_en                             );
        LOG_INF("mfb4i_en                                 (%d)"  ,a_info.top.mfb4i_en                             );
        LOG_INF("mfbo_en                                  (%d)"  ,a_info.top.mfbo_en                              );
        LOG_INF("mfbo_b_en                                (%d)"  ,a_info.top.mfbo_b_en                            );
        LOG_INF("mfb2o_en                                 (%d)"  ,a_info.top.mfb2o_en                             );
        LOG_INF("mfb_srz_en                               (%d)"  ,a_info.top.mfb_srz_en                           );
        LOG_INF("bld_mbd_wt_en                            (%d)"  ,a_info.mfb.bld_mbd_wt_en                        );
        LOG_INF("mfbi_stride                              (%d)"  ,a_info.mfbi.mfbi_stride                         );
        LOG_INF("mfbi_v_flip_en                           (%d)"  ,a_info.mfbi.mfbi_v_flip_en                      );
        LOG_INF("mfbi_b_stride                            (%d)"  ,a_info.mfbi_b.mfbi_b_stride                     );
        LOG_INF("mfbi_b_v_flip_en                         (%d)"  ,a_info.mfbi_b.mfbi_b_v_flip_en                  );
        LOG_INF("mfb3i_stride                             (%d)"  ,a_info.mfb3i.mfb3i_stride                       );
        LOG_INF("mfb3i_v_flip_en                          (%d)"  ,a_info.mfb3i.mfb3i_v_flip_en                    );
        LOG_INF("mfb4i_stride                             (%d)"  ,a_info.mfb4i.mfb4i_stride                       );
        LOG_INF("mfb4i_v_flip_en                          (%d)"  ,a_info.mfb4i.mfb4i_v_flip_en                    );
        LOG_INF("mfb4i_xsize                              (%d)"  ,a_info.mfb4i.mfb4i_xsize                        );
        LOG_INF("mfb4i_ysize                              (%d)"  ,a_info.mfb4i.mfb4i_ysize                        );
        LOG_INF("mfbo_stride                              (%d)"  ,a_info.mfbo.mfbo_stride                         );
        LOG_INF("mfbo_xoffset                             (%d)"  ,a_info.mfbo.mfbo_xoffset                        );
        LOG_INF("mfbo_xsize                               (%d)"  ,a_info.mfbo.mfbo_xsize                          );
        LOG_INF("mfbo_yoffset                             (%d)"  ,a_info.mfbo.mfbo_yoffset                        );
        LOG_INF("mfbo_ysize                               (%d)"  ,a_info.mfbo.mfbo_ysize                          );
        LOG_INF("mfbo_b_stride                            (%d)"  ,a_info.mfbo_b.mfbo_b_stride                     );
        LOG_INF("mfb2o_stride                             (%d)"  ,a_info.mfb2o.mfb2o_stride                       );
        LOG_INF("mfb2o_xoffset                            (%d)"  ,a_info.mfb2o.mfb2o_xoffset                      );
        LOG_INF("mfb2o_xsize                              (%d)"  ,a_info.mfb2o.mfb2o_xsize                        );
        LOG_INF("mfb2o_yoffset                            (%d)"  ,a_info.mfb2o.mfb2o_yoffset                      );
        LOG_INF("mfb2o_ysize                              (%d)"  ,a_info.mfb2o.mfb2o_ysize                        );
        LOG_INF("srz_input_crop_width                     (%d)"  ,a_info.mfb_srz.srz_input_crop_width             );
        LOG_INF("srz_input_crop_height                    (%d)"  ,a_info.mfb_srz.srz_input_crop_height            );
        LOG_INF("srz_output_width                         (%d)"  ,a_info.mfb_srz.srz_output_width                 );
        LOG_INF("srz_output_height                        (%d)"  ,a_info.mfb_srz.srz_output_height                );
        LOG_INF("srz_luma_horizontal_integer_offset       (%d)"  ,a_info.mfb_srz.srz_luma_horizontal_integer_offset);
        LOG_INF("srz_luma_horizontal_subpixel_offset      (%d)"  ,a_info.mfb_srz.srz_luma_horizontal_subpixel_offset);
        LOG_INF("srz_luma_vertical_integer_offset         (%d)"  ,a_info.mfb_srz.srz_luma_vertical_integer_offset );
        LOG_INF("srz_luma_vertical_subpixel_offset        (%d)"  ,a_info.mfb_srz.srz_luma_vertical_subpixel_offset);
        LOG_INF("srz_horizontal_coeff_step                (%d)"  ,a_info.mfb_srz.srz_horizontal_coeff_step        );
        LOG_INF("srz_vertical_coeff_step                  (%d)"  ,a_info.mfb_srz.srz_vertical_coeff_step          );
        LOG_INF("crsp_xoffset                             (%d)"  ,a_info.mfb_crsp.crsp_xoffset                    );
        LOG_INF("crsp_yoffset                             (%d)"  ,a_info.mfb_crsp.crsp_yoffset                    );
        LOG_INF("crsp_ystep                               (%d)"  ,a_info.mfb_crsp.crsp_ystep                      );
        LOG_INF("mfb_y_pak_yuv_bit                        (%d)"  ,a_info.mfb_y_pak.yuv_bit                        );
        LOG_INF("mfb_y_pak_yuv_dng                        (%d)"  ,a_info.mfb_y_pak.yuv_dng                        );
        LOG_INF("mfb_c_pak_yuv_bit                        (%d)"  ,a_info.mfb_c_pak.yuv_bit                        );
        LOG_INF("mfb_c_pak_yuv_dng                        (%d)"  ,a_info.mfb_c_pak.yuv_dng                        );
        LOG_INF("mfb_y_unp_yuv_bit                        (%d)"  ,a_info.mfb_y_unp.yuv_bit                        );
        LOG_INF("mfb_y_unp_yuv_dng                        (%d)"  ,a_info.mfb_y_unp.yuv_dng                        );
        LOG_INF("mfb_c_unp_yuv_bit                        (%d)"  ,a_info.mfb_c_unp.yuv_bit                        );
        LOG_INF("mfb_c_unp_yuv_dng                        (%d)"  ,a_info.mfb_c_unp.yuv_dng                        );
        LOG_INF("src_width_mfb                            (%d)"  ,a_info.sw.src_width_mfb                         );
        LOG_INF("src_height_mfb                           (%d)"  ,a_info.sw.src_height_mfb                        );
        LOG_INF("tpipe_width_mfb                          (%d)"  ,a_info.sw.tpipe_width_mfb                       );
        LOG_INF("tpipe_height_mfb                         (%d)"  ,a_info.sw.tpipe_height_mfb                      );

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
        LOG_INF("sw.log_en = %d",                                    a_info.sw.log_en);
        LOG_INF("sw.src_width_wpe = %d",                             a_info.sw.src_width_wpe);
        LOG_INF("sw.src_height_wpe = %d",                            a_info.sw.src_height_wpe);
        LOG_INF("sw.tpipe_sel_mode = %d",                            a_info.sw.tpipe_sel_mode);

        LOG_INF("sw_wpe.tpipe_width_wpe = %d",                       a_info.wpe.sw_wpe.tpipe_width_wpe);
        LOG_INF("sw_wpe.tpipe_height_wpe = %d",                      a_info.wpe.sw_wpe.tpipe_height_wpe);
        LOG_INF("sw_wpe.tpipe_irq_mode = %d",                        a_info.wpe.sw_wpe.tpipe_irq_mode);

        LOG_INF("top_wpe.wpe_en = %d",                                            a_info.wpe.top_wpe.wpe_en);
        LOG_INF("top_wpe.veci_en = %d",                              a_info.wpe.top_wpe.veci_en);
        LOG_INF("top_wpe.vec2i_en = %d",                             a_info.wpe.top_wpe.vec2i_en);
        LOG_INF("top_wpe.vec3i_en = %d",                             a_info.wpe.top_wpe.vec3i_en);
        LOG_INF("top_wpe.wpeo_en = %d",                              a_info.wpe.top_wpe.wpeo_en);
        LOG_INF("top_wpe.wpe_ispcrop_en = %d",                       a_info.wpe.top_wpe.wpe_ispcrop_en);
        LOG_INF("top_wpe.wpe_mdpcrop_en = %d",                       a_info.wpe.top_wpe.wpe_mdpcrop_en);
        LOG_INF("top_wpe.wpe_c24_en = %d",                           a_info.wpe.top_wpe.wpe_c24_en);
        LOG_INF("top_wpe.wpe_cachi_fmt = %d",                        a_info.wpe.top_wpe.wpe_cachi_fmt);
        LOG_INF("top_wpe.wpe_cachi_plane = %d",                      a_info.wpe.top_wpe.wpe_cachi_plane);
        LOG_INF("top_wpe.wpe_cachi_fmt_bit = %d",                    a_info.wpe.top_wpe.wpe_cachi_fmt_bit);

        LOG_INF("wpe.vgen_input_crop_width = %d",                    a_info.wpe.wpe.vgen_input_width);
        LOG_INF("wpe.vgen_input_crop_height = %d",                   a_info.wpe.wpe.vgen_input_height);
        LOG_INF("wpe.vgen_output_width = %d",                        a_info.wpe.wpe.vgen_output_width);
        LOG_INF("wpe.vgen_output_height = %d",                       a_info.wpe.wpe.vgen_output_height);
        LOG_INF("wpe.vgen_luma_horizontal_integer_offset = %d",      a_info.wpe.wpe.vgen_luma_horizontal_integer_offset);
        LOG_INF("wpe.vgen_luma_horizontal_subpixel_offset = %d",     a_info.wpe.wpe.vgen_luma_horizontal_subpixel_offset);
        LOG_INF("wpe.vgen_luma_vertical_integer_offset = %d",        a_info.wpe.wpe.vgen_luma_vertical_integer_offset);
        LOG_INF("wpe.vgen_luma_vertical_subpixel_offset = %d",       a_info.wpe.wpe.vgen_luma_vertical_subpixel_offset);
        LOG_INF("wpe.vgen_horizontal_coeff_step = %d",               a_info.wpe.wpe.vgen_horizontal_coeff_step);
        LOG_INF("wpe.vgen_vertical_coeff_step = %d",                 a_info.wpe.wpe.vgen_vertical_coeff_step);

        LOG_INF("veci.veci_stride = %d",                             a_info.wpe.veci.veci_stride);
        LOG_INF("veci.veci_xsize = %d",                              a_info.wpe.veci.veci_xsize);
        LOG_INF("veci.veci_ysize = %d",                              a_info.wpe.veci.veci_ysize);
        LOG_INF("veci.veci_v_flip_en = %d",                          a_info.wpe.veci.veci_v_flip_en);

        LOG_INF("vec2i.vec2i_stride = %d",                           a_info.wpe.vec2i.vec2i_stride);
        LOG_INF("vec2i.vec2i_xsize = %d",                            a_info.wpe.vec2i.vec2i_xsize);
        LOG_INF("vec2i.vec2i_ysize = %d",                            a_info.wpe.vec2i.vec2i_ysize);
        LOG_INF("vec2i.vec2i_v_flip_en = %d",                        a_info.wpe.vec2i.vec2i_v_flip_en);

        LOG_INF("vec3i.vec3i_stride = %d",                           a_info.wpe.vec3i.vec3i_stride);
        LOG_INF("vec3i.vec3i_xsize = %d",                            a_info.wpe.vec3i.vec3i_xsize);
        LOG_INF("vec3i.vec3i_ysize = %d",                            a_info.wpe.vec3i.vec3i_ysize);
        LOG_INF("vec3i.vec3i_v_flip_en = %d",                        a_info.wpe.vec3i.vec3i_v_flip_en);

        LOG_INF("wpeo.wpeo_stride = %d",                             a_info.wpe.wpeo.wpeo_stride);
        LOG_INF("wpeo.wpeo_xsize = %d",                              a_info.wpe.wpeo.wpeo_xsize);
        LOG_INF("wpeo.wpeo_xoffset = %d",                            a_info.wpe.wpeo.wpeo_xoffset);
        LOG_INF("wpeo.wpeo_ysize = %d",                              a_info.wpe.wpeo.wpeo_ysize);
        LOG_INF("wpeo.wpeo_yoffset = %d",                            a_info.wpe.wpeo.wpeo_yoffset);

        //LOG_INF("msko.msko_stride = %d",                                         a_info.wpe.msko.msko_stride);
        //LOG_INF("msko.msko_xsize = %d",                                          a_info.wpe.msko.msko_xsize);
        //LOG_INF("msko.msko_xoffset = %d",                                       a_info.wpe.msko.msko_xoffset);
        //LOG_INF("msko.msko_ysize = %d",                                          a_info.wpe.msko.msko_ysize);
        //LOG_INF("msko.msko_yoffset = %d",                                       a_info.wpe.msko.msko_yoffset);
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
                }
                break;
            default:
                err = MDPMGR_WRONG_PARAM;
                goto EXIT;
        }
    }
    m_wpetpipedumpEnable = ::property_get_int32("vendor.camera.p2tpipedump.enable", 0);

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
        {   //wpe stream
            case TILE_SEL_MODE_WPE:
                for(MINT32 i = 0; i < MAX_DUP_CQ_NUM; i++)
                {
                    delete m_wpeaDpStream[i];
                    m_wpeaDpStream[i] = NULL;
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
MINT32 _wpestartMdp(MDPMGR_CFG_STRUCT &cfgData,T* pDpStream, MdpMgr* pMdrMgrObj)
{
    MINT32 err = MDPMGR_NO_ERROR;
    DP_STATUS_ENUM err2 = DP_STATUS_RETURN_SUCCESS;
    MUINT32 srcVirList[PLANE_NUM];
    MUINT32 srcPhyList[PLANE_NUM];
    MUINT32 srcSizeList[PLANE_NUM];
    MUINT32 mdpSrcYStride = 0;                  //! Y stride of mdp WPE crop out image
    MUINT32 mdpSrcUVStride = 0;                 //! UV stride of mdp WPE crop our image

    //get property for p2tpipedump

    ISP_TRACE_CALL();

    // image configure
    LOG_INF("[WPE] WARP Engine ID (%d), cqIndx(%d), frameflag(%d), srcFmt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),planeN(%d), \
             tdri va/pa: %p /0x%lx, \n\
             wdmao_en(%d),wdmao_index(%d),capbility(%d),fmt(%d),rot(%d),flip(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpindx(%d),\n\
             wroto_en(%d),wroto_index(%d),capbility(%d),fmt(%d),rot(%d),flip(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpindx(%d)",\
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

    if (cfgData.ispTpipeCfgInfo.wpe.top_wpe.wpe_mdpcrop_en == 1 && cfgData.ispTpipeCfgInfo.wpe.top_wpe.wpe_en == 1) // WPE ONLY FRAME MODE sould Skip src & dst settings
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

        //err2= pDpStream->setSrcConfig(cfgData.mdpSrcW ,cfgData.mdpSrcH, cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,srcFmt,
        err2= pDpStream->setSrcConfig(cfgData.mdpSrcW ,cfgData.mdpSrcH, mdpSrcYStride,mdpSrcUVStride,srcFmt, \
                                      DP_PROFILE_FULL_BT601, eInterlace_None, 0, false);

        if(err2 != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("setSrcConfig fail(%d)",err);
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

        err2 = pDpStream->queueSrcBuffer(&srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);

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
                    err2 = pDpStream->setDstConfig(index,
                                                   cfgData.dstDma[index].size.w,
                                                   cfgData.dstDma[index].size.h,
                                                   cfgData.dstDma[index].size.stride,
                                                   cfgData.dstDma[index].size_c.stride,
                                                   dstFmt,
                                                   dp_rofile,
                                                   eInterlace_None,
                                                   &pROI,
                                                   false);
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
                        LOG_ERR("dst memAddr is 0,VA(0x%8" PRIxPTR "),PA(0x%8" PRIxPTR ")",cfgData.dstDma[index].memBuf.base_vAddr,cfgData.dstDma[index].memBuf.base_pAddr);
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
                    LOG_DBG("dstSizeList=(0x%8x,0x%8x,0x%8x),dstVirList=(%8p,%8p,%8p),dstPhyList=(0x%8x,0x%8x,0x%8x),plane num(%d)",
                        dstSizeList[0],dstSizeList[1],dstSizeList[2],dstVirList[0],dstVirList[1],dstVirList[2],
                        dstPhyList[0],dstPhyList[1],dstPhyList[2],cfgData.dstDma[index].Plane);

                    err2 = pDpStream->queueDstBuffer(index,&dstPhyList[0], dstSizeList,(cfgData.dstDma[index].Plane + 1 - MDPMGR_PLANE_1));
                    if(err2 != DP_STATUS_RETURN_SUCCESS)
                    {
                        LOG_ERR("queueDstBuffer fail(%d)",err);
                        return MDPMGR_API_FAIL;
                    }
              }

             else
             { // for isp only mode      ISP ONLY MODE
                pDpStream->setPortType(ISP_ONLY_OUT_TPIPE, MDPMGR_ISP_ONLY_FRAME);
             }
        }
      }
    }
    else
    {
        pDpStream->setPortType(ISP_ONLY_OUT_TPIPE, MDPMGR_ISP_ONLY_FRAME);
    }

    if(pMdrMgrObj->m_wpetpipedumpEnable == 1)
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


    LOG_INF("HMyo flag(0x%x -> 0x%x), startStream (%d us)...", cfgData.ispTpipeCfgInfo.drvinfo.frameflag, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, b-a);

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

