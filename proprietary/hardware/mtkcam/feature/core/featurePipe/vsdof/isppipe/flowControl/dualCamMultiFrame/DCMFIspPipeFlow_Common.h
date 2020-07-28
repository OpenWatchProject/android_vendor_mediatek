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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

/**
 * @file DefaultIspPipeFlow_Common.h
 * @brief Common utility for DefaultIspPipeFlow
*/

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_DCMF_ISPPIPE_FLOW_COMMON_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_DCMF_ISPPIPE_FLOW_COMMON_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "../../IspPipe_Common.h"


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

/******************************************************************************
* Enum Definition
********************************************************************************/
/**
  * @brief Node ID inside the IspPipe
 */
enum eDCMFIspPipeNodeID {
    eISP_PIPE_DCMF_NODEID_ROOT = eISP_PIPE_NODEID_END,
    eISP_PIPE_DCMF_NODEID_MFNR,
    eISP_PIPE_DCMF_NODEID_DEPTH,
    eISP_PIPE_DCMF_NODEID_HDR,
    eISP_PIPE_DCMF_NODEID_BOKEH,
    eISP_PIPE_DCMF_NODEID_FD,
    // any node need to put upon this line
    eISP_PIPE_NODE_SIZE
};
/**
  * @brief Data ID used in handleData inside the IspPipe
 */
enum eDCMFIspPipeDataID {
    ROOT_TO_MFNR = eISP_PIPE_DATAID_END,
    ROOT_TO_DEPTH,
    ROOT_TO_HDR,
    ROOT_TO_FD,
    MFNR_TO_BOKEH,
    DEPTH_TO_BOKEH,
    FD_TO_BOKEH,
    FD_TO_HDR,
    FD_TO_DEPTH,
    HDR_TO_BOKEH,
    BOKEH_OUT
};
/**
  * @brief Buffer ID inside the IspPipe
 */
// TODO: check enums are correct or not?
// source: include\mtkcam\feature\stereo\pipe\IIspPipe.h
typedef enum eDCMFIspBufferDataTypes_Internal{
    BID_IN_FSYUV_MAIN1  = PBID_IN_FULLSIZE_YUV_MAIN1,
    BID_IN_FSYUV_MAIN2  = PBID_IN_FULLSIZE_YUV_MAIN2,
    BID_IN_SMYUV_MAIN1  = PBID_IN_BINING_YUV_MAIN1,
    BID_IN_RSYUV_MAIN1  = PBID_IN_RESIZE_YUV_MAIN1,
    BID_IN_RSYUV_MAIN2  = PBID_IN_RESIZE_YUV_MAIN2,
    BID_OUT_CLEAN_FSYUV = PBID_OUT_CLEAN_YUV,
    BID_OUT_BOKEH_FSYUV = PBID_OUT_BOKEH_YUV,
    BID_OUT_DEPTHMAP = PBID_OUT_THIRDPARTY_DEPTH,
    // woring buffer buffer ids
    BID_FD_INERNAL_YUV = PUBLIC_PBID_END,
    BID_DEPTHMAP_INERNAL_Y8,
    BID_CLEAN_INERNAL_YV12,
    BID_POSTVIEW_NV21

} InternalDCMFIspPipeBufferID;



}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif