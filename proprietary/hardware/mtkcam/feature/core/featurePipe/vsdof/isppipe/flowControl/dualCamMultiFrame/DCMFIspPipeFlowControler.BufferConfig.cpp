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
 * @file BufferConfig.cpp
 * @brief buffer config header
*/

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file

// Module header file

// Local header file
#include "DCMFIspPipeFlowControler.h"
#include "DCMFIspPipeFlow_Common.h"


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace DualCamThirdParty {

namespace
{
    // TODO
    // 1. add buffer config for DCMF

    // IspPipeBufferID bufIDs_P2A_TO_TP_YUV_DATA[] =
    // {
    //     BID_P2A_OUT_YUV_MAIN1, BID_P2A_OUT_YUV_MAIN2,
    //     // end with this
    //     BID_INVALID
    // };

    // IspPipeBufferID bufIDs_TP_TO_MDP_BOKEHYUV[] =
    // {
    //     BID_PIPE_OUT_BOKEH_YUV, BID_PIPE_OUT_DEPTH,
    //     // end with this
    //     BID_INVALID
    // };

    // IspPipeBufferID bufIDs_TP_TO_MDP_PVYUV[] =
    // {
    //     BID_PIPE_OUT_PV_YUV0,
    //     // end with this
    //     BID_INVALID
    // };

    // IspPipeBufferID bufIDs_MDP_OUT_THUMBNAIL[] =
    // {
    //     BID_MDP_OUT_THUMBNAIL_YUV,
    //     // end with this
    //     BID_INVALID
    // };

    // IspPipeBufferID bufIDs_MDP_OUT_YUVS[] =
    // {
    //     BID_PIPE_OUT_PV_YUV0,
    //     // end with this
    //     BID_INVALID
    // };

    // IspPipeBufferID bufIDs_TO_DUMP_RAWS[] =
    // {
    //     BID_IN_FSRAW_MAIN1, BID_IN_RSRAW_MAIN1,
    //     BID_IN_RSRAW_MAIN2,
    //     // end with this
    //     BID_INVALID
    // };

    MVOID addDataIDToBIDMapItem(
        DataIDToBIDMap& outMap,
        IspPipeDataID dataID,
        IspPipeBufferID* bidArray
    )
    {
        Vector<IspPipeBufferID> bidMap;
        IspPipeBufferID* pBufferID = bidArray;
        while(*pBufferID != BID_INVALID)
        {
            bidMap.add(*pBufferID);
            pBufferID++;
        }
        outMap.add(dataID, bidMap);
    }
} // anonymous name space

const DataIDToBIDMap
DCMFIspPipeFlowControler::IDToBIDMapProvider::
getDataIDToBIDMap()
{
    #define ADD_TO_DBIDMAP(DATAID) \
        addDataIDToBIDMapItem(dBIDMap, DATAID, bufIDs_##DATAID);

    // data ID to BIDs map
    DataIDToBIDMap dBIDMap;
    // ADD_TO_DBIDMAP(ROOT_TO_MFNR);
    // ADD_TO_DBIDMAP(ROOT_TO_DEPTH);
    // ADD_TO_DBIDMAP(MFNR_TO_BOKEH);
    // ADD_TO_DBIDMAP(DEPTH_TO_BOKEH);
    // ADD_TO_DBIDMAP(BOKEH_OUT);
    // ADD_TO_DBIDMAP(TO_DUMP_RAWS);

    #undef ADD_TO_DBIDMAP
    return dBIDMap;
}

}; // DualCamThirdParty
}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam