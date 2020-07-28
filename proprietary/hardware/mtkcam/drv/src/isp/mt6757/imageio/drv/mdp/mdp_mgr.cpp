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
static MDPMGR_STREAMSELECT_STRUCT_s mDpStreamMap[ISP_DRV_DIP_CQ_NUM][MAX_DUP_CQ_NUM][MAX_BURST_QUE_NUM]; //mapping table for selecting dpStream object
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
MINT32 _startMdp(MDPMGR_DPSTREAM_TYPE dpStreamtype,MDPMGR_CFG_STRUCT &cfgData,T* pDpStream)
{
	MINT32 err = MDPMGR_NO_ERROR;
    DP_STATUS_ENUM err2 = DP_STATUS_RETURN_SUCCESS;

    MUINT32 srcVirList[PLANE_NUM];
    MUINT32 srcPhyList[PLANE_NUM];
    MUINT32 srcSizeList[PLANE_NUM];

    //get property for p2tpipedump
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.p2tpipedump.enable", value, "0");
    int p2tpipedumpEnable=atoi(value);

    char value_skiptpipeDBG[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.disable_p2skiptpipe", value_skiptpipeDBG, "0"); //temp, need set 0 in default
    int p2disableSkipTpipe=0;
    p2disableSkipTpipe=atoi(value_skiptpipeDBG);
    
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


    // source image configure
    LOG_DBG("[mdpSrc] frameflag(0x%x), srcFmt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),planeN(%d)",\
            cfgData.ispTpipeCfgInfo.drvinfo.frameflag,\
            cfgData.mdpSrcFmt,cfgData.mdpSrcW,cfgData.mdpSrcH,cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,cfgData.mdpSrcBufSize,cfgData.mdpSrcCBufSize,cfgData.mdpSrcVBufSize,cfgData.srcVirAddr,cfgData.srcPhyAddr,\
            cfgData.mdpSrcPlaneNum);

    err2= pDpStream->setSrcConfig(cfgData.mdpSrcW ,cfgData.mdpSrcH, \
        cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,srcFmt, \
        DP_PROFILE_FULL_BT601, eInterlace_None, 0, false);
    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("setSrcConfig fail(%d)",err2);
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
    err = pDpStream->queueSrcBuffer((void**)srcVirList,&srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);
    #else
    err2 = pDpStream->queueSrcBuffer(&srcPhyList[0],srcSizeList,cfgData.mdpSrcPlaneNum);
    #endif
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
    DpRect pROI;
    //DpPqParam   ISPParam;
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
        		        LOG_ERR("(%d)setSrcCrop fail(%d)",index,err2);
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
                    LOG_DBG("index(%d),capbility(%d),isoValue(%d),fmt(%d),rot(%d),flip(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),dpStreamtype(%d),mdpindx(%d)",
                            index, cfgData.dstDma[index].capbility, cfgData.isoValue, dstFmt, cfgData.dstDma[index].Rotation, cfgData.dstDma[index].Flip,
                        cfgData.dstDma[index].size.w,cfgData.dstDma[index].size.h,cfgData.dstDma[index].size.stride,
                        cfgData.dstDma[index].size_c.w,cfgData.dstDma[index].size_c.h,cfgData.dstDma[index].size_c.stride,
                        cfgData.dstDma[index].size_v.w,cfgData.dstDma[index].size_v.h,cfgData.dstDma[index].size_v.stride,
                        dpStreamtype,mdpPortMapping[index].mdpPortIdx);

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
            else
                { // for isp only mode      ISP ONLY MODE doesn't need to config src & dst after Everest, WAIT FOR DELETE!!!

                // format convert
                err = DpDmaOutColorFmtConvert(cfgData.dstDma[index],&dstFmt);
                if(err != MDPMGR_NO_ERROR) {
                    LOG_ERR("DpDmaOutColorFmtConvert fail");
                    return MDPMGR_API_FAIL;
                }

                LOG_DBG("index(%d),capbility(%d),fmt(%d),rot(%d),flip(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),dpStreamtype(%d),mdpindx(%d)",
                        index, cfgData.dstDma[index].capbility, dstFmt, cfgData.dstDma[index].Rotation, cfgData.dstDma[index].Flip,
                        cfgData.dstDma[index].size.w,cfgData.dstDma[index].size.h,cfgData.dstDma[index].size.stride,
                        cfgData.dstDma[index].size_c.w,cfgData.dstDma[index].size_c.h,cfgData.dstDma[index].size_c.stride,
                        cfgData.dstDma[index].size_v.w,cfgData.dstDma[index].size_v.h,cfgData.dstDma[index].size_v.stride,
                        dpStreamtype,mdpPortMapping[index].mdpPortIdx);

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
        pDpStream->setPortType(ISP_ONLY_OUT_TPIPE, MDPMGR_ISP_ONLY_TPIPE);
    }

    //====== Configure TPIPE ======
    /*
    if(cfgData.isWaitBuf==MTRUE)
        cfgData.ispTpipeCfgInfo.pass2.Pass2CmdqPriority = TPIPE_PASS2_CMDQ_PRIOR_HIGH;
    else
        cfgData.ispTpipeCfgInfo.pass2.Pass2CmdqPriority = TPIPE_PASS2_CMDQ_PRIOR_LOW;
    */
    //cfgData.ispTpipeCfgInfo.pass2.Pass2CmdqNum = cfgData.ispTpipeCfgInfo.drvinfo.cqIdx;
    //
    if(p2tpipedumpEnable == 1)
    {
        dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
    }
    else
    {
        dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MFALSE);
    }

    //copy tpipe information if needed
    int refCQidx = 0;
    int refdupCQidx = 0;
    int refburstCQidx = 0;
    unsigned int* srcTdriAddr=NULL;

    if(cfgData.ispTpipeCfgInfo.drvinfo.frameflag & 0x1) //skip tpipe, use the previous dpStream
    {
        
        pthread_mutex_lock(&mMutex_DpStreamMap);
        refCQidx=mDpStreamMap[cfgData.ispTpipeCfgInfo.drvinfo.cqIdx][cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx][cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx].refcqIdx;
        refdupCQidx=mDpStreamMap[cfgData.ispTpipeCfgInfo.drvinfo.cqIdx][cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx][cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx].refdupCqIdx;
        refburstCQidx=mDpStreamMap[cfgData.ispTpipeCfgInfo.drvinfo.cqIdx][cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx][cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx].refburstCqIdx;
        srcTdriAddr=mDpStreamMap[refCQidx][refdupCQidx][refburstCQidx].srcTpipeTableVa;
        pthread_mutex_unlock(&mMutex_DpStreamMap);

        if(p2tpipedumpEnable == 1)
        {
            LOG_INF("HMyo before (0x%x _ 0x%x, %d)", srcTdriAddr, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa, \
                !(::memcmp(srcTdriAddr, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,MAX_ISP_TILE_TDR_HEX_NO)));
            for(int q=0;q<20; q++)
            {
                LOG_INF("HMyp before content.. q(%d), (0x%x <-> 0x%x)",q, *((MUINT32*)srcTdriAddr+q), *((MUINT32*)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa+q));
            }
       }
        ::memcpy(cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa, srcTdriAddr, MAX_ISP_TILE_TDR_HEX_NO);
        LOG_DBG("HMyo CQ info (%d_%d_%d), ref(%d_%d_%d), (va: 0x%x _ 0x%x, dstpa: 0x%x)",cfgData.ispTpipeCfgInfo.drvinfo.cqIdx, \
			cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx, cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,\
            refCQidx,refdupCQidx,refburstCQidx, srcTdriAddr, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa, \
            cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa);
        if(p2tpipedumpEnable ==1)
        {
            LOG_INF("HMyo copy tpipe (va: 0x%x _ 0x%x, dstpa: 0x%x, %d)",srcTdriAddr, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
            !(::memcmp(srcTdriAddr, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,MAX_ISP_TILE_TDR_HEX_NO)));
            for(int q=0;q<20; q++)
            {
                LOG_INF("HMyo after content.. q(%d), (0x%x <-> 0x%x)",q, *((MUINT32*)srcTdriAddr+q), *((MUINT32*)cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa+q));
            }
        }
    }
    else
    {
        LOG_DBG("HMyo CQ info(%d_%d_%d), ref(%d_%d_%d)",cfgData.ispTpipeCfgInfo.drvinfo.cqIdx, cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx, cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,\
            cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx);
    }

    //
    LOG_DBG("HMyo setParameter (tdri va/pa: 0x%x/0x%x _ %d)",cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
                cfgData.ispTpipeCfgInfo.drvinfo.frameflag);
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
        LOG_ERR("setParameter fail(%d)",err2);
        return MDPMGR_API_FAIL;
    }

    //====== Start DpIspStream ======
    //LOG_INF("startStream");
    MUINT32 a=0,b=0;
    a=getUs();
    err2 = pDpStream->startStream();
    b=getUs();
    //update tdri buffer after mdp operation to cover the situation that isp do not update but mdp update
    pthread_mutex_lock(&mMutex_DpStreamMap);
    mDpStreamMap[cfgData.ispTpipeCfgInfo.drvinfo.cqIdx][cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx][cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx].srcTpipeTableVa = \
        cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa;
    pthread_mutex_unlock(&mMutex_DpStreamMap);  
    if(p2disableSkipTpipe)
    {
        LOG_INF("+sceID(%u),cqPa(0x%x)-Va(0x%x),tpipVa(0x%x)-Pa(0x%x),isWaitB(%d),venc(%d),\
        srcFt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),plane(%d),Stmtype(%d)\n\
        wdma_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpidx(%d),\n\
        wrot_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpidx(%d),\n\
        jpg_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpidx(%d),\n\
        pipe_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpidx(%d),\n\
        HMyo CQ(%d_%d_%d),ref(%d_%d_%d),(va:0x%x_0x%x,dstpa:0x%x)\nflag(%d_0x%x->0x%x),startStm(%d us)",\
                cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.DesCqPa,cfgData.ispTpipeCfgInfo.drvinfo.DesCqVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
                cfgData.isWaitBuf,cfgData.dstPortCfg[ISP_MDP_DL_VENCO],\
                cfgData.mdpSrcFmt,cfgData.mdpSrcW,cfgData.mdpSrcH,cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,cfgData.mdpSrcBufSize,cfgData.mdpSrcCBufSize,cfgData.mdpSrcVBufSize,cfgData.srcVirAddr,cfgData.srcPhyAddr,\
                cfgData.mdpSrcPlaneNum,dpStreamtype,
                cfgData.dstPortCfg[ISP_MDP_DL_WDMAO],ISP_MDP_DL_WDMAO, cfgData.dstDma[ISP_MDP_DL_WDMAO].capbility, cfgData.dstDma[ISP_MDP_DL_WDMAO].Format, cfgData.dstDma[ISP_MDP_DL_WDMAO].Rotation, cfgData.dstDma[ISP_MDP_DL_WDMAO].Flip,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_WDMAO].mdpPortIdx,
                cfgData.dstPortCfg[ISP_MDP_DL_WROTO],ISP_MDP_DL_WROTO, cfgData.dstDma[ISP_MDP_DL_WROTO].capbility, cfgData.dstDma[ISP_MDP_DL_WROTO].Format, cfgData.dstDma[ISP_MDP_DL_WROTO].Rotation, cfgData.dstDma[ISP_MDP_DL_WROTO].Flip,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_WROTO].mdpPortIdx,
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
                cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx, cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,\
                refCQidx,refdupCQidx,refburstCQidx, srcTdriAddr, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa, \
                cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, p2disableSkipTpipe, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, 0, b-a);

    }
    else
    {
        LOG_INF("+sceID(%u),cqPa(0x%x)-Va(0x%x),tpipVa(0x%x)-Pa(0x%x),isWaitB(%d),venc(%d),\
        srcFt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),plane(%d),Stmtype(%d)\n\
        wdma_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpidx(%d),\n\
        wrot_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpidx(%d),\n\
        jpg_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpidx(%d),\n\
        pipe_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%d,%d,%d),C(w,h,s)=(%d,%d,%d),V(w,h,s)=(%d,%d,%d),mdpidx(%d),\n\
        HMyo CQ(%d_%d_%d),ref(%d_%d_%d),(va:0x%x_0x%x,dstpa:0x%x)\nflag(%d_0x%x->0x%x),startStm(%d us)",\
                cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.DesCqPa,cfgData.ispTpipeCfgInfo.drvinfo.DesCqVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
                cfgData.isWaitBuf,cfgData.dstPortCfg[ISP_MDP_DL_VENCO],\
                cfgData.mdpSrcFmt,cfgData.mdpSrcW,cfgData.mdpSrcH,cfgData.mdpSrcYStride,cfgData.mdpSrcUVStride,cfgData.mdpSrcBufSize,cfgData.mdpSrcCBufSize,cfgData.mdpSrcVBufSize,cfgData.srcVirAddr,cfgData.srcPhyAddr,\
                cfgData.mdpSrcPlaneNum,dpStreamtype,
                cfgData.dstPortCfg[ISP_MDP_DL_WDMAO],ISP_MDP_DL_WDMAO, cfgData.dstDma[ISP_MDP_DL_WDMAO].capbility, cfgData.dstDma[ISP_MDP_DL_WDMAO].Format, cfgData.dstDma[ISP_MDP_DL_WDMAO].Rotation, cfgData.dstDma[ISP_MDP_DL_WDMAO].Flip,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WDMAO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_WDMAO].mdpPortIdx,
                cfgData.dstPortCfg[ISP_MDP_DL_WROTO],ISP_MDP_DL_WROTO, cfgData.dstDma[ISP_MDP_DL_WROTO].capbility, cfgData.dstDma[ISP_MDP_DL_WROTO].Format, cfgData.dstDma[ISP_MDP_DL_WROTO].Rotation, cfgData.dstDma[ISP_MDP_DL_WROTO].Flip,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size.stride,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_c.stride,
                cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.w,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.h,cfgData.dstDma[ISP_MDP_DL_WROTO].size_v.stride,
                mdpPortMapping[ISP_MDP_DL_WROTO].mdpPortIdx,
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
                cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx, cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,\
                refCQidx,refdupCQidx,refburstCQidx, srcTdriAddr, cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa, \
                cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, p2disableSkipTpipe, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, cfgData.ispTpipeCfgInfo.drvinfo.frameflag, b-a);

    }
    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("startStream fail(%d)",err2);
        LOG_ERR("===dump tpipe structure start===");
        dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE);
        LOG_ERR("===dump tpipe structure end===");
        return MDPMGR_API_FAIL;
    }
    
    
    if (NSImageio::NSIspio::eDrvScenario_VSS != cfgData.drvScenario)
    {
        // cut off the previous stream
        err2 = pDpStream->stopStream();
        if(err2 != DP_STATUS_RETURN_SUCCESS)
        {
            LOG_ERR("stopStream fail(%d)",err2);
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
    MINT32 err2 = DP_STATUS_RETURN_SUCCESS;
    //====== Stop DpIspStream ======
    err2 = pDpStream->stopStream();
    if(err2 != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("stopStream fail(%d)",err2);
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
        dumpIspTPipeInfo(cfgData.ispTpipeCfgInfo,MTRUE); 
        ret = MDPMGR_DPFRAMEWORK_FAIL;
    }

    if (deqIndex != MDPMGR_DEQ_SRC)
        LOG_INF("deqIndex(%d), dequeTime(%d us)", deqIndex, b-a);
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
        pthread_mutex_lock(&mMutex_DpStreamMap);
            for(MINT32 i = 0; i < ISP_DRV_DIP_CQ_NUM; i++) {
                for(MINT32 j = 0; j < MAX_DUP_CQ_NUM; j++) {
                    for(MINT32 k = 0; k < MAX_BURST_QUE_NUM; k++) {
                        m_pDpStream[i][j][k] = allocDpStream(i);
                        mDpStreamMap[i][j][k].frameflag=0x0;
                        mDpStreamMap[i][j][k].dpObjcqIdx=static_cast<E_ISP_DIP_CQ>(i);
                        mDpStreamMap[i][j][k].dpObjdupCqIdx=j;
                        mDpStreamMap[i][j][k].dpObjburstCqIdx=k;
                        mDpStreamMap[i][j][k].refcqIdx=static_cast<E_ISP_DIP_CQ>(i);
                        mDpStreamMap[i][j][k].refdupCqIdx=j;
                        mDpStreamMap[i][j][k].refburstCqIdx=k;
                        mDpStreamMap[i][j][k].srcTpipeTableVa=NULL;
                    }
                }
        }
        pthread_mutex_unlock(&mMutex_DpStreamMap);
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
            for(MINT32 i = 0; i < ISP_DRV_DIP_CQ_NUM; i++) {
                for(MINT32 j = 0; j < MAX_DUP_CQ_NUM; j++) {
                    for(MINT32 k = 0; k < MAX_BURST_QUE_NUM; k++) {
                        delete m_pDpStream[i][j][k];
                        m_pDpStream[i][j][k] = NULL;
                    }
                }
            }

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
    MUINT32 sceID = cfgData.drvScenario;
    DpIspStream *pDpStream = NULL;
    LOG_INF("+,cqIndx(%u),sceID(%u),cqPA(0x%x),tpipVa(0x%x),tpipPa(0x%x),isWaitBuf(%d),venc(%d)", \
            cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,sceID,cfgData.ispTpipeCfgInfo.drvinfo.DesCqPa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTableVa,cfgData.ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
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
    else
    {
        DpIspStream *pDpStream = NULL;
        DpVEncStream *pDpStream_venccfg = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx);
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
    else
    {
        DpIspStream *pDpStream = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx);
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
    MUINT32 sceID = cfgData.drvScenario;

    LOG_DBG("+,cqIndx(%u/%u/%u),sceID(%u),deqIndex(%d),isWaitBuf(%d)",cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,\
        cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx,sceID,deqIndex,cfgData.isWaitBuf);

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
    else
    {
        DpIspStream *pDpStream = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx);
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
    else
    {
        DpIspStream *pDpStream = NULL;
        pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx);
        if(pDpStream == NULL)
        {
            LOG_ERR("pDpStream is NULL");
            return MDPMGR_NULL_OBJECT;
        }
        //
        ret=_dequeueFrameEnd(cfgData,pDpStream);
    }
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
        case ISP_DRV_DIP_CQ_THRE11 :
            pRetDpIspStream = new DpIspStream(DpIspStream::ISP_VSS_STREAM);
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
DpIspStream *MdpMgrImp::selectDpIspStream(MUINT32 sceID, MUINT32 cqIdx, MUINT32 dupCqIdx, MUINT32 burstCqIdx)
{
    LOG_DBG("+,cqIndx(%d_%d_%d),sceID(%d)",cqIdx, dupCqIdx, burstCqIdx, sceID);

    Mutex::Autolock lock(mLock);

    DpIspStream *pRetDpIspStream = NULL;

    MUINT32 cqidx=cqIdx;
    MUINT32 dupcqidx=dupCqIdx;
    MUINT32 burstcqidx=burstCqIdx;
    pthread_mutex_lock(&mMutex_DpStreamMap);
    cqidx=mDpStreamMap[cqIdx][dupCqIdx][burstCqIdx].dpObjcqIdx;
    dupcqidx=mDpStreamMap[cqIdx][dupCqIdx][burstCqIdx].dpObjdupCqIdx;
    burstcqidx=mDpStreamMap[cqIdx][dupCqIdx][burstCqIdx].dpObjburstCqIdx;
    pthread_mutex_unlock(&mMutex_DpStreamMap);    
    
    if(m_pDpStream[cqidx][dupcqidx][burstcqidx]){
        pRetDpIspStream = m_pDpStream[cqidx][dupcqidx][burstcqidx];
        LOG_DBG("HM upt,scenID(%d), cqIndx(%d_%d_%d -> %d_%d_%d), dpStream(0x%x -> 0x%x)",sceID, cqIdx, dupCqIdx, burstCqIdx, cqidx, dupcqidx, burstcqidx, \
            m_pDpStream[cqIdx][dupCqIdx][burstCqIdx], pRetDpIspStream);
    } else {
        LOG_ERR("[Error]Null point cqIdx(%d/%d/%d_%d/%d/%d),sceID(%d)",cqIdx,dupCqIdx, burstCqIdx, cqidx, dupcqidx, burstcqidx,sceID);
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
    pDpStream = selectDpIspStream(cfgData.drvScenario,cfgData.ispTpipeCfgInfo.drvinfo.cqIdx,cfgData.ispTpipeCfgInfo.drvinfo.dupCqIdx,cfgData.ispTpipeCfgInfo.drvinfo.burstCqIdx);
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
MBOOL MdpMgrImp::updateDpStreamMap(MUINT32 flag, MUINT32 cqIdx,MUINT32 dupcqIdx,MUINT32 burstcqIdx,
                            MUINT32 refcqIdx,MUINT32 refdupcqIdx,MUINT32 refburstcqIdx, unsigned int* srcTdriAddr)
{
    bool ret=MTRUE;
    LOG_DBG("+ HM flag(0x%x)", flag);
    MUINT32 debugCqIdx=0x0, debugdupCqIdx=0x0, debugburstCqIdx;
    unsigned int* debugTdrtAddr=NULL;

    if(flag&0x1) //skip tpipe, use the previous dpStream
    {
        pthread_mutex_lock(&mMutex_DpStreamMap);
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].frameflag=flag;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjcqIdx=mDpStreamMap[refcqIdx][refdupcqIdx][refburstcqIdx].dpObjcqIdx;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjdupCqIdx=mDpStreamMap[refcqIdx][refdupcqIdx][refburstcqIdx].dpObjdupCqIdx;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjburstCqIdx=mDpStreamMap[refcqIdx][refdupcqIdx][refburstcqIdx].dpObjburstCqIdx;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].refcqIdx=static_cast<E_ISP_DIP_CQ>(refcqIdx);
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].refdupCqIdx=refdupcqIdx;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].refburstCqIdx=refburstcqIdx;
       // mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].srcTpipeTableVa=mDpStreamMap[refcqIdx][refdupcqIdx][refburstcqIdx].srcTpipeTableVa; //update in startmdp
        //
        debugCqIdx = mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjcqIdx;
        debugdupCqIdx = mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjdupCqIdx;
        debugburstCqIdx = mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjburstCqIdx;
        debugTdrtAddr = mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].srcTpipeTableVa;
        pthread_mutex_unlock(&mMutex_DpStreamMap);    
    }
    else
    {
        pthread_mutex_lock(&mMutex_DpStreamMap);
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].frameflag=flag;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjcqIdx=static_cast<E_ISP_DIP_CQ>(cqIdx);
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjdupCqIdx=dupcqIdx;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjburstCqIdx=burstcqIdx;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].refcqIdx=static_cast<E_ISP_DIP_CQ>(cqIdx);;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].refdupCqIdx=dupcqIdx;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].refburstCqIdx=burstcqIdx;
        mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].srcTpipeTableVa=srcTdriAddr;
        //
        debugCqIdx = mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjcqIdx;
        debugdupCqIdx = mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjdupCqIdx;
        debugburstCqIdx = mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].dpObjburstCqIdx;
        debugTdrtAddr = mDpStreamMap[cqIdx][dupcqIdx][burstcqIdx].srcTpipeTableVa;
        pthread_mutex_unlock(&mMutex_DpStreamMap);    
    }

    LOG_DBG("- HM cq(%d_%d_%d_0x%x), updatedRef(%d_%d_%d_0x%x)", cqIdx, dupcqIdx, burstcqIdx, srcTdriAddr,\
        debugCqIdx, debugdupCqIdx, debugburstCqIdx, debugTdrtAddr);
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
            localDpColorFormat = DP_COLOR_BAYER10;
            break;
        case eImgFmt_BAYER12:   // 0x0004,  Bayer format, 12-bit
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
        case eImgFmt_UFO_BAYER10:
            localDpColorFormat = DP_COLOR_UFO10;
            break;
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

    LOG_DBG("+,dma,Fmt(%d),Plane(%d),uv_plane_swap(%d),Sequence(%d)",dma_out.Format,
                                                                      dma_out.Plane,
                                                                      dma_out.uv_plane_swap,
                                                                      dma_out.Sequence);
    switch(dma_out.Format)
    {
        case CRZ_DRV_FORMAT_YUV422:
            if(dma_out.Plane == CRZ_DRV_PLANE_3)
            {
                if(dma_out.uv_plane_swap == 1)
                {
                    localDpColorFormat = DP_COLOR_YV16; // 422,3P
                }
                else
                {
                    localDpColorFormat = DP_COLOR_I422; // 422,3P
                }
            }
            else if(dma_out.Plane == CRZ_DRV_PLANE_2)
            {
                if(dma_out.uv_plane_swap == 1)
                {
                    localDpColorFormat = DP_COLOR_NV61; //422,2P
                }
                else
                {
                    localDpColorFormat = DP_COLOR_NV16;
                }
            }
            else
            {
                if(dma_out.Sequence == CRZ_DRV_SEQUENCE_YVYU) //MSN->LSB
                {
                    localDpColorFormat = DP_COLOR_UYVY;
                }
                else if(dma_out.Sequence == CRZ_DRV_SEQUENCE_YUYV)
                {
                    localDpColorFormat = DP_COLOR_VYUY;
                }
                else if(dma_out.Sequence == CRZ_DRV_SEQUENCE_VYUY)
                {
                    localDpColorFormat = DP_COLOR_YUYV;
                }
                else if(dma_out.Sequence == CRZ_DRV_SEQUENCE_UYVY)
                {
                    localDpColorFormat = DP_COLOR_YVYU;
                }
            }
            break;
        case CRZ_DRV_FORMAT_YUV420:
            if(dma_out.Plane == CRZ_DRV_PLANE_3)
            {
                if(dma_out.uv_plane_swap == 1)
                {
                    localDpColorFormat = DP_COLOR_YV12;
                }
                else
                {
                    localDpColorFormat = DP_COLOR_I420;
                }
            }
            else if(dma_out.Plane == CRZ_DRV_PLANE_2)
            {
                if(dma_out.Sequence == CRZ_DRV_SEQUENCE_UVUV) //MSB->LSB
                {
                    localDpColorFormat= DP_COLOR_NV21;  //420_2P_YVYU;
                }
                else
                {
                    localDpColorFormat= DP_COLOR_NV12;  //_420_2P_YUYV
                }
            }
            break;
        case CRZ_DRV_FORMAT_JPEG_YUV420:
            localDpColorFormat = DP_COLOR_I420;
            break;
        case CRZ_DRV_FORMAT_JPEG_YUV422:
            localDpColorFormat = DP_COLOR_I422;
            break;
        case  CRZ_DRV_FORMAT_Y:
            localDpColorFormat = DP_COLOR_GREY;
            break;
        case CRZ_DRV_FORMAT_RGB888:
            localDpColorFormat = DP_COLOR_RGB888;
            break;
        case CRZ_DRV_FORMAT_RGB565:
            localDpColorFormat = DP_COLOR_RGB565;
            break;
        case CRZ_DRV_FORMAT_XRGB8888:
             if(dma_out.Sequence == CRZ_DRV_SEQUENCE_XRGB) //MSN->LSB
            {
                localDpColorFormat = DP_COLOR_XRGB8888;
            }
            else if(dma_out.Sequence == CRZ_DRV_SEQUENCE_RGBX)
            {
                localDpColorFormat = DP_COLOR_RGBX8888;
            }
            break;
        case CRZ_DRV_FORMAT_RAW8:
            localDpColorFormat = eBAYER8;
            break;
        case CRZ_DRV_FORMAT_RAW10:
            localDpColorFormat = eBAYER10;
            break;
        case CRZ_DRV_FORMAT_RAW12:
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
        LOG_INF("sw.log_en(%d)",                                   a_info.sw.log_en);
        LOG_INF("sw.src_width(%d)",                                a_info.sw.src_width);
        LOG_INF("sw.src_height(%d)",                               a_info.sw.src_height);
        LOG_INF("sw.tpipe_width(%d)",                              a_info.sw.tpipe_width);
        LOG_INF("sw.tpipe_height(%d)",                              a_info.sw.tpipe_height);
        LOG_INF("sw.tpipe_irq_mode(%d)",                            a_info.sw.tpipe_irq_mode);
        LOG_INF("sw.tpipe_sel_mode(%d)",                            a_info.sw.tpipe_sel_mode);
        LOG_INF("top.pixel_id        (%d)"                        ,a_info.top.pixel_id        );
        LOG_INF("top.cam_in_fmt      (%d)"                            ,a_info.top.cam_in_fmt      );
        LOG_INF("top.ctl_extension_en(%d)"                            ,a_info.top.ctl_extension_en);
        LOG_INF("top.fg_mode        (%d)",                             a_info.top.fg_mode);
        LOG_INF("top.ufo_imgi_en     (%d)",                             a_info.top.ufo_imgi_en);
        LOG_INF("top.ufdi_fmt       (%d)",                       a_info.top.ufdi_fmt);
        LOG_INF("top.vipi_fmt        (%d)"                            ,a_info.top.vipi_fmt        );
        LOG_INF("top.img3o_fmt       (%d)"                            ,a_info.top.img3o_fmt       );
        LOG_INF("top.img2o_fmt       (%d)"                            ,a_info.top.img2o_fmt       );
        LOG_INF("top.pak2_fmt        (%d)"                            ,a_info.top.pak2_fmt        );
        LOG_INF("top.imgi_en         (%d)"                            ,a_info.top.imgi_en         );
        LOG_INF("top.imgbi_en        (%d)"                            ,a_info.top.imgbi_en        );
        LOG_INF("top.imgci_en        (%d)"                            ,a_info.top.imgci_en        );
        LOG_INF("top.ufdi_en         (%d)"                            ,a_info.top.ufdi_en         );
        LOG_INF("top.unp_en          (%d)"                            ,a_info.top.unp_en          );
        LOG_INF("top.ufd_en          (%d)"                            ,a_info.top.ufd_en          );
        LOG_INF("top.ufd_crop_en     (%d)"                            ,a_info.top.ufd_crop_en     );
        LOG_INF("top.bnr_en          (%d)"                            ,a_info.top.bnr_en          );
        LOG_INF("top.lsc2_en         (%d)"                            ,a_info.top.lsc2_en         );
        LOG_INF("top.rcp2_en         (%d)"                            ,a_info.top.rcp2_en         );
        LOG_INF("top.pak2_en         (%d)"                            ,a_info.top.pak2_en         );
        LOG_INF("top.c24_en          (%d)"                            ,a_info.top.c24_en          );
        LOG_INF("top.sl2_en          (%d)"                            ,a_info.top.sl2_en          );
        LOG_INF("top.rnr_en          (%d)"                            ,a_info.top.rnr_en          );
        LOG_INF("top.udm_en          (%d)"                            ,a_info.top.udm_en          );
        LOG_INF("top.vipi_en         (%d)"                            ,a_info.top.vipi_en         );
        LOG_INF("top.vip2i_en        (%d)"                            ,a_info.top.vip2i_en        );
        LOG_INF("top.vip3i_en        (%d)"                            ,a_info.top.vip3i_en        );
        LOG_INF("top.mfb_en          (%d)"                            ,a_info.top.mfb_en          );
        LOG_INF("top.mfbw_en         (%d)"                            ,a_info.top.mfbw_en         );
        LOG_INF("top.mfbo_en         (%d)"                            ,a_info.top.mfbo_en         );
        LOG_INF("top.g2c_en          (%d)"                            ,a_info.top.g2c_en          );
        LOG_INF("top.c42_en          (%d)"                            ,a_info.top.c42_en          );
        LOG_INF("top.sl2b_en         (%d)"                            ,a_info.top.sl2b_en         );
        LOG_INF("top.nbc_en          (%d)"                            ,a_info.top.nbc_en          );
        LOG_INF("top.dmgi_en         (%d)"                            ,a_info.top.dmgi_en         );
        LOG_INF("top.depi_en         (%d)"                            ,a_info.top.depi_en         );
        LOG_INF("top.nbc2_en         (%d)"                            ,a_info.top.nbc2_en         );
        LOG_INF("top.srz1_en         (%d)"                            ,a_info.top.srz1_en         );
        LOG_INF("top.mix1_en         (%d)"                            ,a_info.top.mix1_en         );
        LOG_INF("top.srz2_en         (%d)"                            ,a_info.top.srz2_en         );
        LOG_INF("top.mix2_en         (%d)"                            ,a_info.top.mix2_en         );
        LOG_INF("top.sl2c_en         (%d)"                            ,a_info.top.sl2c_en         );
        LOG_INF("top.sl2d_en         (%d)"                            ,a_info.top.sl2d_en         );
        LOG_INF("top.sl2e_en         (%d)"                            ,a_info.top.sl2e_en         );
        LOG_INF("top.sl2g_en         (%d)"                            ,a_info.top.sl2g_en         );
        LOG_INF("top.sl2h_en         (%d)"                            ,a_info.top.sl2h_en         );
        LOG_INF("top.sl2i_en         (%d)"                            ,a_info.top.sl2i_en         );
        LOG_INF("top.hfg_en          (%d)"                            ,a_info.top.hfg_en          );
        LOG_INF("top.seee_en         (%d)"                            ,a_info.top.seee_en         );
        LOG_INF("top.lcei_en         (%d)"                            ,a_info.top.lcei_en         );
        LOG_INF("top.lce_en          (%d)"                            ,a_info.top.lce_en          );
        LOG_INF("top.mix3_en         (%d)"                            ,a_info.top.mix3_en         );
        LOG_INF("top.crz_en          (%d)"                            ,a_info.top.crz_en          );
        LOG_INF("top.img2o_en        (%d)"                            ,a_info.top.img2o_en        );
        LOG_INF("top.img2bo_en       (%d)"                            ,a_info.top.img2bo_en       );
        LOG_INF("top.fe_en           (%d)"                            ,a_info.top.fe_en           );
        LOG_INF("top.feo_en          (%d)"                            ,a_info.top.feo_en          );
        LOG_INF("top.c02_en          (%d)"                            ,a_info.top.c02_en          );
        LOG_INF("top.c02b_en         (%d)"                            ,a_info.top.c02b_en         );
        LOG_INF("top.nr3d_en         (%d)"                            ,a_info.top.nr3d_en         );
        LOG_INF("top.crsp_en         (%d)"                            ,a_info.top.crsp_en         );
        LOG_INF("top.img3o_en        (%d)"                            ,a_info.top.img3o_en        );
        LOG_INF("top.img3bo_en       (%d)"                            ,a_info.top.img3bo_en       );
        LOG_INF("top.img3co_en       (%d)"                            ,a_info.top.img3co_en       );
        LOG_INF("top.c24b_en         (%d)"                            ,a_info.top.c24b_en         );
        LOG_INF("top.mdp_crop_en     (%d)"                            ,a_info.top.mdp_crop_en     );
        LOG_INF("top.plnr1_en        (%d)"                            ,a_info.top.plnr1_en        );
        LOG_INF("top.plnr2_en        (%d)"                            ,a_info.top.plnr2_en        );
        LOG_INF("top.plnw1_en        (%d)"                            ,a_info.top.plnw1_en        );
        LOG_INF("top.plnw2_en        (%d)"                            ,a_info.top.plnw2_en        );
        LOG_INF("top.dbs2_en         (%d)"                            ,a_info.top.dbs2_en         );
        LOG_INF("top.obc2_en         (%d)"                            ,a_info.top.obc2_en         );
        LOG_INF("top.rmm2_en         (%d)"                            ,a_info.top.rmm2_en         );
        LOG_INF("top.rmg2_en         (%d)"                            ,a_info.top.rmg2_en         );
        LOG_INF("top.gdr1_en         (%d)"                            ,a_info.top.gdr1_en         );
        LOG_INF("top.gdr2_en         (%d)"                            ,a_info.top.gdr2_en         );
        LOG_INF("top.bnr2_en         (%d)"                            ,a_info.top.bnr2_en         );
        LOG_INF("top.fm_en           (%d)"                            ,a_info.top.fm_en           );
        LOG_INF("top.srz3_en         (%d)"                            ,a_info.top.srz3_en         );
        LOG_INF("top.srz4_en         (%d)"                            ,a_info.top.srz4_en         );
        LOG_INF("top.interlace_mode  (%d)"                            ,a_info.top.interlace_mode  );
        LOG_INF("top.pgn_sel         (%d)"                            ,a_info.top.pgn_sel         );
        LOG_INF("top.g2g_sel         (%d)"                            ,a_info.top.g2g_sel         );
        LOG_INF("top.g2c_sel         (%d)"                            ,a_info.top.g2c_sel         );
        LOG_INF("top.srz1_sel        (%d)"                            ,a_info.top.srz1_sel        );
        LOG_INF("top.mix1_sel        (%d)"                            ,a_info.top.mix1_sel        );
        LOG_INF("top.crz_sel         (%d)"                            ,a_info.top.crz_sel         );
        LOG_INF("top.nr3d_sel        (%d)"                            ,a_info.top.nr3d_sel        );
        LOG_INF("top.fe_sel          (%d)"                            ,a_info.top.fe_sel          );
        LOG_INF("top.mdp_sel         (%d)"                            ,a_info.top.mdp_sel         );
        LOG_INF("top.pca_en          (%d)"                            ,a_info.top.pca_en          );
        LOG_INF("top.pgn_en          (%d)"                            ,a_info.top.pgn_en          );
        LOG_INF("top.g2g_en          (%d)"                            ,a_info.top.g2g_en          );
        LOG_INF("top.ggm_en          (%d)"                            ,a_info.top.ggm_en          );
        LOG_INF("top.nbcw_sel        (%d)"                            ,a_info.top.nbcw_sel        );
        LOG_INF("top.nbc_sel         (%d)"                            ,a_info.top.nbc_sel         );
        LOG_INF("top.rcp2_sel        (%d)"                            ,a_info.top.rcp2_sel        );
        LOG_INF("top.mfbo_sel        (%d)"                            ,a_info.top.mfbo_sel        );
        LOG_INF("top.crsp_sel        (%d)"                            ,a_info.top.crsp_sel        );
        LOG_INF("top.feo_sel        (%d)"                             ,a_info.top.feo_sel         );
        LOG_INF("imgi.imgi_v_flip_en(%d)",                         a_info.imgi.imgi_v_flip_en);
        LOG_INF("imgi.imgi_stride(%d)",                            a_info.imgi.imgi_stride);
        LOG_INF("imgbi.imgbi_v_flip_en(%d)",                         a_info.imgbi.imgbi_v_flip_en);
        LOG_INF("imgbi.imgbi_stride(%d)",                            a_info.imgbi.imgbi_stride);
        LOG_INF("imgci.imgci_v_flip_en(%d)",                         a_info.imgci.imgci_v_flip_en);
        LOG_INF("imgci.imgci_stride(%d)",                            a_info.imgci.imgci_stride);
        LOG_INF("ufdi.ufdi_v_flip_en(%d)",                            a_info.ufdi.ufdi_v_flip_en);
        LOG_INF("ufdi.ufdi_xsize(%d)",                            a_info.ufdi.ufdi_xsize);
        LOG_INF("ufdi.ufdi_ysize(%d)",                            a_info.ufdi.ufdi_ysize);
        LOG_INF("ufdi.ufdi_stride(%d)",                            a_info.ufdi.ufdi_stride);
        LOG_INF("bnr.bpc_en(%d)",                                  a_info.bnr.bpc_en);
        LOG_INF("bnr.bpc_tbl_en(%d)",                              a_info.bnr.bpc_tbl_en);
        LOG_INF("rmg.rmg_ihdr_en(%d)",                            a_info.rmg.rmg_ihdr_en);
        LOG_INF("rmg.rmg_zhdr_en(%d)",                            a_info.rmg.rmg_zhdr_en);
        LOG_INF("lsc2.extend_coef_mode(%d)",                      a_info.lsc2.extend_coef_mode);
        LOG_INF("lsc2.sdblk_width(%d)",                            a_info.lsc2.sdblk_width);
        LOG_INF("lsc2.sdblk_xnum(%d)",                            a_info.lsc2.sdblk_xnum);
        LOG_INF("lsc2.sdblk_last_width(%d)",                            a_info.lsc2.sdblk_last_width);
        LOG_INF("lsc2.sdblk_height(%d)",                            a_info.lsc2.sdblk_height);
        LOG_INF("lsc2.sdblk_ynum(%d)",                            a_info.lsc2.sdblk_ynum);
        LOG_INF("lsc2.sdblk_last_height(%d)",                            a_info.lsc2.sdblk_last_height);
        LOG_INF("sl2.sl2_hrz_comp(%d)",                            a_info.sl2.sl2_hrz_comp);
        LOG_INF("sl2.sl2_vrz_comp(%d)",                            a_info.sl2.sl2_vrz_comp);
        LOG_INF("udm.bayer_bypass(%d)",                            a_info.udm.bayer_bypass);
        LOG_INF("vipi.vipi_v_flip_en(%d)",                             a_info.vipi.vipi_v_flip_en);
        LOG_INF("vipi.vipi_xsize(%d)",                             a_info.vipi.vipi_xsize);
        LOG_INF("vipi.vipi_ysize(%d)",                            a_info.vipi.vipi_ysize);
        LOG_INF("vipi.vipi_stride(%d)",                            a_info.vipi.vipi_stride);
        LOG_INF("vip2i.vip2i_v_flip_en(%d)",                           a_info.vip2i.vip2i_v_flip_en);
        LOG_INF("vip2i.vip2i_xsize(%d)",                           a_info.vip2i.vip2i_xsize);
        LOG_INF("vip2i.vip2i_ysize(%d)",                          a_info.vip2i.vip2i_ysize);
        LOG_INF("vip2i.vip2i_stride(%d)",                          a_info.vip2i.vip2i_stride);
        LOG_INF("vip3i.vip3i_v_flip_en(%d)",                           a_info.vip3i.vip3i_v_flip_en);
        LOG_INF("vip3i.vip3i_xsize(%d)",                           a_info.vip3i.vip3i_xsize);
        LOG_INF("vip3i.vip3i_ysize(%d)",                          a_info.vip3i.vip3i_ysize);
        LOG_INF("vip3i.vip3i_stride(%d)",                          a_info.vip3i.vip3i_stride);
        LOG_INF("mfb.bld_deblock_en(%d)",                              a_info.mfb.bld_deblock_en);
        LOG_INF("mfb.bld_brz_en(%d)",                              a_info.mfb.bld_brz_en);
        LOG_INF("mfbo.mfbo_stride(%d)",                            a_info.mfbo.mfbo_stride);
        LOG_INF("mfbo.mfbo_xoffset(%d)",                           a_info.mfbo.mfbo_xoffset);
        LOG_INF("mfbo.mfbo_yoffset(%d)",                           a_info.mfbo.mfbo_yoffset);
        LOG_INF("mfbo.mfbo_xsize(%d)",                             a_info.mfbo.mfbo_xsize);
        LOG_INF("mfbo.mfbo_ysize(%d)",                             a_info.mfbo.mfbo_ysize);
        LOG_INF("g2c.g2c_shade_en(%d)",                            a_info.g2c.g2c_shade_en);
        LOG_INF("g2c.g2c_shade_xmid(%d)",                            a_info.g2c.g2c_shade_xmid);
        LOG_INF("g2c.g2c_shade_ymid(%d)",                            a_info.g2c.g2c_shade_ymid);
        LOG_INF("g2c.g2c_shade_var(%d)",                            a_info.g2c.g2c_shade_var);
        LOG_INF("sl2b.sl2b_hrz_comp(%d)",                          a_info.sl2b.sl2b_hrz_comp);
        LOG_INF("sl2b.sl2b_vrz_comp(%d)",                          a_info.sl2b.sl2b_vrz_comp);
        LOG_INF("nbc.anr_eny(%d)",                                 a_info.nbc.anr_eny);
        LOG_INF("nbc.anr_enc(%d)",                                 a_info.nbc.anr_enc);
        LOG_INF("nbc.anr_scale_mode(%d)",                          a_info.nbc.anr_scale_mode);
        LOG_INF("nbc2.anr2_eny(%d)",                                 a_info.nbc2.anr2_eny);
        LOG_INF("nbc2.anr2_enc(%d)",                                 a_info.nbc2.anr2_enc);
        LOG_INF("nbc2.anr2_scale_mode(%d)",                          a_info.nbc2.anr2_scale_mode);
        LOG_INF("nbc2.anr2_mode(%d)",                                 a_info.nbc2.anr2_mode);
        LOG_INF("nbc2.anr2_bok_mode(%d)",                          a_info.nbc2.anr2_bok_mode);
        LOG_INF("nbc2.anr2_bok_pf_en(%d)",                         a_info.nbc2.anr2_bok_pf_en);
        LOG_INF("nbc2.abf_en(%d)",                                 a_info.nbc2.abf_en);
        LOG_INF("sl2c.sl2c_hrz_comp(%d)",                          a_info.sl2c.sl2c_hrz_comp);
        LOG_INF("sl2c.sl2c_vrz_comp(%d)",                          a_info.sl2c.sl2c_vrz_comp);
        LOG_INF("sl2d.sl2d_hrz_comp(%d)",                          a_info.sl2d.sl2d_hrz_comp);
        LOG_INF("sl2d.sl2d_vrz_comp(%d)",                          a_info.sl2d.sl2d_vrz_comp);
        LOG_INF("sl2e.sl2e_hrz_comp(%d)",                          a_info.sl2e.sl2e_hrz_comp);
        LOG_INF("sl2e.sl2e_vrz_comp(%d)",                          a_info.sl2e.sl2e_vrz_comp);
        LOG_INF("sl2g.sl2g_hrz_comp(%d)",                          a_info.sl2g.sl2g_hrz_comp);
        LOG_INF("sl2g.sl2g_vrz_comp(%d)",                          a_info.sl2g.sl2g_vrz_comp);
        LOG_INF("sl2h.sl2h_hrz_comp(%d)",                          a_info.sl2h.sl2h_hrz_comp);
        LOG_INF("sl2h.sl2h_vrz_comp(%d)",                          a_info.sl2h.sl2h_vrz_comp);
        LOG_INF("sl2i.sl2i_hrz_comp(%d)",                          a_info.sl2i.sl2i_hrz_comp);
        LOG_INF("sl2i.sl2i_vrz_comp(%d)",                          a_info.sl2i.sl2i_vrz_comp);
        LOG_INF("hfg.non_block_base(%d)",                          a_info.hfg.non_block_base);
        LOG_INF("seee.se_edge(%d)",                                a_info.seee.se_edge);
        LOG_INF("seee.itune_see_method(%d)",                                a_info.seee.itune_see_method);
        LOG_INF("lcei.lcei_v_flip_en(%d)",                            a_info.lcei.lcei_v_flip_en);
        LOG_INF("lcei.lcei_xsize(%d)",                            a_info.lcei.lcei_xsize);
        LOG_INF("lcei.lcei_ysize(%d)",                            a_info.lcei.lcei_ysize);
        LOG_INF("lcei.lcei_stride(%d)",                            a_info.lcei.lcei_stride);
        LOG_INF("lce.lce_bc_mag_kubnx(%d)",                           a_info.lce.lce_bc_mag_kubnx);
        LOG_INF("lce.lce_slm_width(%d)",                          a_info.lce.lce_slm_width);
        LOG_INF("lce.lce_bc_mag_kubny(%d)",                        a_info.lce.lce_bc_mag_kubny);
        LOG_INF("lce.lce_slm_height(%d)",                        a_info.lce.lce_slm_height);
        LOG_INF("lce.lce_full_xoff(%d)",                        a_info.lce.lce_full_xoff);
        LOG_INF("lce.lce_full_yoff(%d)",                        a_info.lce.lce_full_yoff);
        LOG_INF("lce.lce_full_slm_width(%d)",                   a_info.lce.lce_full_slm_width);
        LOG_INF("lce.lce_full_slm_height(%d)",                  a_info.lce.lce_full_slm_height);
        LOG_INF("lce.lce_full_out_height(%d)",                  a_info.lce.lce_full_out_height);
        LOG_INF("cdrz.cdrz_input_crop_width(%d)",                  a_info.cdrz.cdrz_input_crop_width);
        LOG_INF("cdrz.cdrz_input_crop_height(%d)",                 a_info.cdrz.cdrz_input_crop_height);
        LOG_INF("cdrz.cdrz_output_width(%d)",                      a_info.cdrz.cdrz_output_width);
        LOG_INF("cdrz.cdrz_output_height(%d)",                     a_info.cdrz.cdrz_output_height);
        LOG_INF("cdrz.cdrz_luma_horizontal_integer_offset(%d)",    a_info.cdrz.cdrz_luma_horizontal_integer_offset);
        LOG_INF("cdrz.cdrz_luma_horizontal_subpixel_offset(%d)",   a_info.cdrz.cdrz_luma_horizontal_subpixel_offset);
        LOG_INF("cdrz.cdrz_luma_vertical_integer_offset(%d)",      a_info.cdrz.cdrz_luma_vertical_integer_offset);
        LOG_INF("cdrz.cdrz_luma_vertical_subpixel_offset(%d)",     a_info.cdrz.cdrz_luma_vertical_subpixel_offset);
        LOG_INF("cdrz.cdrz_horizontal_luma_algorithm(%d)",         a_info.cdrz.cdrz_horizontal_luma_algorithm);
        LOG_INF("cdrz.cdrz_vertical_luma_algorithm(%d)",           a_info.cdrz.cdrz_vertical_luma_algorithm);
        LOG_INF("cdrz.cdrz_horizontal_coeff_step(%d)",             a_info.cdrz.cdrz_horizontal_coeff_step);
        LOG_INF("cdrz.cdrz_vertical_coeff_step(%d)",               a_info.cdrz.cdrz_vertical_coeff_step);
        LOG_INF("img2o.img2o_stride(%d)",                          a_info.img2o.img2o_stride);
        LOG_INF("img2o.img2o_xoffset(%d)",                         a_info.img2o.img2o_xoffset);
        LOG_INF("img2o.img2o_yoffset(%d)",                         a_info.img2o.img2o_yoffset);
        LOG_INF("img2o.img2o_xsize(%d)",                           a_info.img2o.img2o_xsize);
        LOG_INF("img2o.img2o_ysize(%d)",                           a_info.img2o.img2o_ysize);
        LOG_INF("img2bo.img2bo_stride(%d)",                          a_info.img2bo.img2bo_stride);
        LOG_INF("img2bo.img2bo_xoffset(%d)",                         a_info.img2bo.img2bo_xoffset);
        LOG_INF("img2bo.img2bo_yoffset(%d)",                         a_info.img2bo.img2bo_yoffset);
        LOG_INF("img2bo.img2bo_xsize(%d)",                           a_info.img2bo.img2bo_xsize);
        LOG_INF("img2bo.img2bo_ysize(%d)",                           a_info.img2bo.img2bo_ysize);
        LOG_INF("srz1.srz_input_crop_width(%d)",                   a_info.srz1.srz_input_crop_width);
        LOG_INF("srz1.srz_input_crop_height(%d)",                  a_info.srz1.srz_input_crop_height);
        LOG_INF("srz1.srz_output_width(%d)",                       a_info.srz1.srz_output_width);
        LOG_INF("srz1.srz_output_height(%d)",                      a_info.srz1.srz_output_height);
        LOG_INF("srz1.srz_luma_horizontal_integer_offset(%d)",     a_info.srz1.srz_luma_horizontal_integer_offset);
        LOG_INF("srz1.srz_luma_horizontal_subpixel_offset(%d)",    a_info.srz1.srz_luma_horizontal_subpixel_offset);
        LOG_INF("srz1.srz_luma_vertical_integer_offset(%d)",       a_info.srz1.srz_luma_vertical_integer_offset);
        LOG_INF("srz1.srz_luma_vertical_subpixel_offset(%d)",      a_info.srz1.srz_luma_vertical_subpixel_offset);
        LOG_INF("srz1.srz_horizontal_coeff_step(%d)",              a_info.srz1.srz_horizontal_coeff_step);
        LOG_INF("srz1.srz_vertical_coeff_step(%d)",                a_info.srz1.srz_vertical_coeff_step);
        LOG_INF("srz2.srz_input_crop_width(%d)",                   a_info.srz2.srz_input_crop_width);
        LOG_INF("srz2.srz_input_crop_height(%d)",                  a_info.srz2.srz_input_crop_height);
        LOG_INF("srz2.srz_output_width(%d)",                       a_info.srz2.srz_output_width);
        LOG_INF("srz2.srz_output_height(%d)",                      a_info.srz2.srz_output_height);
        LOG_INF("srz2.srz_luma_horizontal_integer_offset(%d)",     a_info.srz2.srz_luma_horizontal_integer_offset);
        LOG_INF("srz2.srz_luma_horizontal_subpixel_offset(%d)",    a_info.srz2.srz_luma_horizontal_subpixel_offset);
        LOG_INF("srz2.srz_luma_vertical_integer_offset(%d)",       a_info.srz2.srz_luma_vertical_integer_offset);
        LOG_INF("srz2.srz_luma_vertical_subpixel_offset(%d)",      a_info.srz2.srz_luma_vertical_subpixel_offset);
        LOG_INF("srz2.srz_horizontal_coeff_step(%d)",              a_info.srz2.srz_horizontal_coeff_step);
        LOG_INF("srz2.srz_vertical_coeff_step(%d)",                a_info.srz2.srz_vertical_coeff_step);
        LOG_INF("srz3.srz_input_crop_width(%d)",                   a_info.srz3.srz_input_crop_width);
        LOG_INF("srz3.srz_input_crop_height(%d)",                  a_info.srz3.srz_input_crop_height);
        LOG_INF("srz3.srz_output_width(%d)",                       a_info.srz3.srz_output_width);
        LOG_INF("srz3.srz_output_height(%d)",                      a_info.srz3.srz_output_height);
        LOG_INF("srz3.srz_luma_horizontal_integer_offset(%d)",     a_info.srz3.srz_luma_horizontal_integer_offset);
        LOG_INF("srz3.srz_luma_horizontal_subpixel_offset(%d)",    a_info.srz3.srz_luma_horizontal_subpixel_offset);
        LOG_INF("srz3.srz_luma_vertical_integer_offset(%d)",       a_info.srz3.srz_luma_vertical_integer_offset);
        LOG_INF("srz3.srz_luma_vertical_subpixel_offset(%d)",      a_info.srz3.srz_luma_vertical_subpixel_offset);
        LOG_INF("srz3.srz_horizontal_coeff_step(%d)",              a_info.srz3.srz_horizontal_coeff_step);
        LOG_INF("srz3.srz_vertical_coeff_step(%d)",                a_info.srz3.srz_vertical_coeff_step);
        LOG_INF("srz4.srz_input_crop_width(%d)",                   a_info.srz4.srz_input_crop_width);
        LOG_INF("srz4.srz_input_crop_height(%d)",                  a_info.srz4.srz_input_crop_height);
        LOG_INF("srz4.srz_output_width(%d)",                       a_info.srz4.srz_output_width);
        LOG_INF("srz4.srz_output_height(%d)",                      a_info.srz4.srz_output_height);
        LOG_INF("srz4.srz_luma_horizontal_integer_offset(%d)",     a_info.srz4.srz_luma_horizontal_integer_offset);
        LOG_INF("srz4.srz_luma_horizontal_subpixel_offset(%d)",    a_info.srz4.srz_luma_horizontal_subpixel_offset);
        LOG_INF("srz4.srz_luma_vertical_integer_offset(%d)",       a_info.srz4.srz_luma_vertical_integer_offset);
        LOG_INF("srz4.srz_luma_vertical_subpixel_offset(%d)",      a_info.srz4.srz_luma_vertical_subpixel_offset);
        LOG_INF("srz4.srz_horizontal_coeff_step(%d)",              a_info.srz4.srz_horizontal_coeff_step);
        LOG_INF("srz4.srz_vertical_coeff_step(%d)",                a_info.srz4.srz_vertical_coeff_step);
        LOG_INF("fe.fe_mode(%d)",                                  a_info.fe.fe_mode);
        LOG_INF("feo.feo_stride(%d)",                              a_info.feo.feo_stride);
        LOG_INF("nr3d.nr3d_on_en(%d)",                             a_info.nr3d.nr3d_on_en);
        LOG_INF("nr3d.nr3d_on_xoffset(%d)",                        a_info.nr3d.nr3d_on_xoffset);
        LOG_INF("nr3d.nr3d_on_yoffset(%d)",                        a_info.nr3d.nr3d_on_yoffset);
        LOG_INF("nr3d.nr3d_on_width(%d)",                          a_info.nr3d.nr3d_on_width);
        LOG_INF("nr3d.nr3d_on_height(%d)",                         a_info.nr3d.nr3d_on_height);
        LOG_INF("crsp.crsp_ystep(%d)",                             a_info.crsp.crsp_ystep);
        LOG_INF("crsp.crsp_xoffset(%d)",                           a_info.crsp.crsp_xoffset);
        LOG_INF("crsp.crsp_yoffset(%d)",                           a_info.crsp.crsp_yoffset);
        LOG_INF("img3o.img3o_stride(%d)",                          a_info.img3o.img3o_stride);
        LOG_INF("img3o.img3o_xoffset(%d)",                         a_info.img3o.img3o_xoffset);
        LOG_INF("img3o.img3o_yoffset(%d)",                         a_info.img3o.img3o_yoffset);
        LOG_INF("img3o.img3o_xsize(%d)",                           a_info.img3o.img3o_xsize);
        LOG_INF("img3o.img3o_ysize(%d)",                           a_info.img3o.img3o_ysize);
        LOG_INF("img3bo.img3bo_stride(%d)",                        a_info.img3bo.img3bo_stride);
        LOG_INF("img3bo.img3bo_xsize(%d)",                         a_info.img3bo.img3bo_xsize);
        LOG_INF("img3bo.img3bo_ysize(%d)",                         a_info.img3bo.img3bo_ysize);
        LOG_INF("img3co.img3co_stride(%d)",                        a_info.img3co.img3co_stride);
        LOG_INF("img3co.img3co_xsize(%d)",                         a_info.img3co.img3co_xsize);
        LOG_INF("img3co.img3co_ysize(%d)",                         a_info.img3co.img3co_ysize);
        LOG_INF("dmgi.dmgi_v_flip_en(%d)",                              a_info.dmgi.dmgi_v_flip_en);
        LOG_INF("dmgi.dmgi_stride(%d)",                              a_info.dmgi.dmgi_stride);
        LOG_INF("dmgi.dmgi_offset(%d)",                             a_info.dmgi.dmgi_offset);
        LOG_INF("dmgi.dmgi_xsize(%d)",                            a_info.dmgi.dmgi_xsize);
        LOG_INF("dmgi.dmgi_ysize(%d)",                        a_info.dmgi.dmgi_ysize);
        LOG_INF("depi.depi_v_flip_en(%d)",                              a_info.depi.depi_v_flip_en);
        LOG_INF("depi.depi_stride(%d)",                             a_info.depi.depi_stride);
        LOG_INF("depi.depi_xsize(%d)",                            a_info.depi.depi_xsize);
        LOG_INF("depi.depi_ysize(%d)",                        a_info.depi.depi_ysize);
        LOG_INF("pca.pca_cfc_en(%d)",                              a_info.pca.pca_cfc_en);
        LOG_INF("pca.pca_cnv_en(%d)",                              a_info.pca.pca_cnv_en);
        #endif
    }
}


