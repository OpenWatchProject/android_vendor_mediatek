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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_IHalWpePipe_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_IHalWpePipe_H_
//
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>

#include <mtkcam/drv/def/wpecommon.h>


#include <vector>
using namespace std;
using namespace NSCam::NSIoPipe;


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSWpe {

/******************************************************************************
 *
 * @struct WPEParams
 *
 * @brief Queuing parameters for the pipe.
 *      input image, disparity vector,
 *      output disparity vecotr
 *
 * @param[in] mpfnCallback: a pointer to a callback function.
 *      If it is NULL, the pipe must put the result into its result queue, and
 *      then a user will get the result by deque() from the pipe later.
 *      If it is not NULL, the pipe does not put the result to its result queue.
 *      The pipe must invoke a callback with the result.
 *
 * @param[in] mWPEConfigVec: a vector of input requests.
 *
 ******************************************************************************/
 enum WPE_MODE
 {  
    WPE_MODE_WPEO = 0,
    WPE_MODE_MDP,
    WPE_MODE_ISP
 };

enum WPE_BUS_SIZE
 {  
    WPE_BUS_SIZE_8_BITS = 0,
    WPE_BUS_SIZE_16_BITS,
    WPE_BUS_SIZE_24_BITS,
    WPE_BUS_SIZE_32_BITS
 };

struct WpeOutCropInfo
{
        MUINT32    x_start_point;
        MUINT32    x_end_point;
        MUINT32    y_start_point;
        MUINT32    y_end_point;
            WpeOutCropInfo()
                : x_start_point(0)
                , x_end_point(0)
                , y_start_point(0)
                , y_end_point(0)
                {}
};

struct WarpMatrixInfo
{
        MUINT32         width;            //Number of pixels per line
        MUINT32         height;           //Number of lines to read
        MUINT32         stride;           //Byte is the unit
        WPE_BUS_SIZE    bus_size;         //Element sizto of per pixel;Default 3, 0:8-bit, 1:16-bit, 2:24-bit, 3:32-bit
        MUINTPTR        virtAddr;         //Start address of a frame
        MUINTPTR        phyAddr;
        MUINT32         addr_offset;      //tile srart address offset
        MINT32          veci_v_flip_en;
            WarpMatrixInfo()
                : width(0)
                , height(0)
                , stride(0)
                , bus_size(WPE_BUS_SIZE_32_BITS)
                , virtAddr(0x0)
                , phyAddr(0x0)
                , addr_offset(0)
                , veci_v_flip_en(0){}
};

enum ExtraFeatureIndex
{
    EWPE_NONE = 0,
    EWPE_AINR,
    EWPE_TOTAL_INDEX
};

struct WPEAINRParams
{
    MUINT32 AINR_line_mode;    // 0: odd line fro RG; 1:even line for GB
};

struct WPEQParams
{
    MUINT32                      wpe_mode;  // 0: WPEO; 1: MDP;  2: ISP
    MUINT32                      vgen_hmg_mode; // 0: No; 1: Yes using Z plane or not
    WarpMatrixInfo               warp_veci_info;
    WarpMatrixInfo               warp_vec2i_info;
    WarpMatrixInfo               warp_vec3i_info;
    android::Vector<MCrpRsInfo>  mwVgenCropInfo;
    WpeOutCropInfo               wpecropinfo;
    MUINT32                      tbl_sel_v;      //default 2
    MUINT32                      tbl_sel_h;      //default 2
    MUINT32                      psp_border_color;       //bit 24: border color en; bit 0-23: border color
    ExtraFeatureIndex            extra_feature_index;    //default 0: off 
    MVOID*                       extra_feature_params;

        WPEQParams()
            : wpe_mode(1)
            , vgen_hmg_mode(0)
            , tbl_sel_v(2)
            , tbl_sel_h(2)
            , psp_border_color (0)
            , extra_feature_index(EWPE_NONE)
            , extra_feature_params(NULL){}
};

/******************************************************************************
 *
 * @struct FramePackage
 * @brief frame package for enqueue/dequeue (one package may contain multiple frames)
 * @details
 *
 ******************************************************************************/
struct WpeFramePackage
{
    EStreamPipeID callerPipe;   //need or not???
    MUINT32 callerID;       //different thread, special buffer id to recognize the buffer source(in default we use address of dequeuedBufList from caller)
    QParams rParams;        //image buffer information
    MINT32 frameNum;   //total number of frame units in a single package
        WpeFramePackage()
            : callerPipe(EStreamPipeID_WarpEG)
            , callerID(0x0)
            , frameNum(0)
    {}
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSWpe
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_WPE_IHalWpePipe_H_

