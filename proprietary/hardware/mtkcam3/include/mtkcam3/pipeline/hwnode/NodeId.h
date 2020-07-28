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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_HWNODE_NODEID_H_
#define _MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_HWNODE_NODEID_H_
//
#include <inttypes.h>
#include <stdint.h>
//
#include <cutils/compiler.h>
//
#include <mtkcam3/pipeline/def/types.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3 {

/******************************************************************************
 * HW Pipeline Node ID
 ******************************************************************************/
//  Notes:
//  Eveny node Id should belong to the global namespace, not to a specific
//  pipeline's namespace, so that we can easily reuse node's instances.
enum
{
    eNODEID_UNKNOWN         = 0x00,
    //
    eNODEID_P1Node_NUM      = 3,
    eNODEID_P1Node_BEGIN    = 0x01,
    eNODEID_P1Node          = eNODEID_P1Node_BEGIN, /*DEPRECATED*/
    eNODEID_P1Node_main2,                           /*DEPRECATED*/
    eNODEID_P1Node_main3,                           /*DEPRECATED*/
    eNODEID_P1Node_END      = eNODEID_P1Node_BEGIN + eNODEID_P1Node_NUM,
    //

    eNODEID_PDENode         = 0x13,
    eNODEID_P2CaptureNode   = 0x14,
    eNODEID_P2StreamNode    = 0x15,
    eNODEID_P2Node          = eNODEID_P2StreamNode,
    eNODEID_FDNode          = 0x16,
    //
    eNODEID_JpegNode        = 0x23,
    eNODEID_JpsNode         = 0x24,
    //
    eNODEID_RAW16_BEGIN     = 0x31,
    eNODEID_RAW16           = eNODEID_RAW16_BEGIN,  /*DEPRECATED*/
    eNODEID_RAW16_main2,                            /*DEPRECATED*/
    eNODEID_RAW16_main3,                            /*DEPRECATED*/
    eNODEID_RAW16_END,
    //
    eNODEID_P1RawIspTuningDataPackNode  = 0x40, //IspTuningDataPackNode after P1Node for Raw
    eNODEID_P2YuvIspTuningDataPackNode  = 0x41, //IspTuningDataPackNode after P2CaptureNode for Yuv
    eNODEID_P2RawIspTuningDataPackNode  = 0x42, //IspTuningDataPackNode after P2CaptureNode for Raw
};


};  //namespace NSCam::v3
namespace NSCam::v3::pipeline {


/**
 * node id helper
 */
class NodeIdUtils
{
public:

    /**
     * Get the node Id of P1Node
     *
     * @param index The index of physical sensors, starting from 0.
     *
     * @return The node Id.
     */
    static constexpr NodeId_T getP1NodeId(size_t index)
    {
        auto const id = eNODEID_P1Node_BEGIN + index;
        if (CC_LIKELY(  eNODEID_P1Node_END > id  )) {
            return id;
        }
        return eNODEID_UNKNOWN;
    }


    /**
     * Get the node Id of Raw16Node.
     *
     * @param index The index of physical sensors, starting from 0.
     *
     * @return The node Id.
     */
    static constexpr NodeId_T getRaw16NodeId(size_t index)
    {
        auto const id = eNODEID_RAW16_BEGIN + index;
        if (CC_LIKELY(  eNODEID_RAW16_END > id  )) {
            return id;
        }
        return eNODEID_UNKNOWN;
    }

};

};  //namespace NSCam::v3::pipeline
#endif  //_MTK_HARDWARE_MTKCAM3_INCLUDE_MTKCAM3_PIPELINE_HWNODE_NODEID_H_

