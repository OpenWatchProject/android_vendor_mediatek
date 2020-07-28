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

#ifndef _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_DEFAULT_ISPPIPE_FLOW_COMMON_H_
#define _MTK_CAMERA_FEATURE_PIPE_THIRD_PARTY_DEFAULT_ISPPIPE_FLOW_COMMON_H_

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
enum eDefaultIspPipeNodeID {
    eISP_PIPE_NODEID_P2A = eISP_PIPE_NODEID_END,
    eISP_PIPE_NODEID_TP,
    eISP_PIPE_NODEID_MDP,
    // any node need to put upon this line
    eISP_PIPE_NODE_SIZE
};
/**
  * @brief Data ID used in handleData inside the IspPipe
 */
enum eDefaultIspPipeDataID {
    TP_OUT_DEPTH_BOKEH = eISP_PIPE_DATAID_END,
    MDP_OUT_THUMBNAIL,
    MDP_OUT_YUVS,
    //
    P2A_TO_TP_YUV_DATA,
    TP_TO_MDP_BOKEHYUV,
    TP_TO_MDP_PVYUV,
};
/**
  * @brief Buffer ID inside the IspPipe
 */
typedef enum eDefaultIspBufferDataTypes_Internal{
    BID_P2A_OUT_YUV_MAIN1 = PUBLIC_PBID_END,
    BID_P2A_OUT_YUV_MAIN2,
    BID_P2A_OUT_PV_YUV,

    BID_P2A_TUNING,
} InternalDefaultIspPipeBufferID;



}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#endif