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

// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
// Module header file

// Local header file
#include "BaseBufferConfig.h"
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

/*******************************************************************************
* Global Define
********************************************************************************/


// bufferID inside the DataID : P2A_TO_N3D_FEFM_CCin
DepthMapBufferID bufIDs_P2A_TO_N3D_FEFM_CCin[] =
{
    BID_P2A_OUT_FE1BO, BID_P2A_OUT_FE2BO, BID_P2A_OUT_FE1CO, BID_P2A_OUT_FE2CO,
    BID_P2A_OUT_RECT_IN1, BID_P2A_OUT_RECT_IN2, BID_P2A_OUT_CC_IN1, BID_P2A_OUT_CC_IN2,
    BID_P2A_OUT_FMBO_LR, BID_P2A_OUT_FMBO_RL, BID_P2A_OUT_FMCO_LR, BID_P2A_OUT_FMCO_RL,
    // end with this
    BID_INVALID
};


DepthMapBufferID bufIDs_P2A_TO_FD_IMG[] =
{
    BID_P2A_OUT_FDIMG,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_N3D_TO_DPE_MVSV_MASK[] =
{
    BID_N3D_OUT_MV_Y, BID_N3D_OUT_MASK_M,
    BID_N3D_OUT_SV_Y, BID_N3D_OUT_MASK_S,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_N3D_TO_OCC_LDC[] =
{
    BID_N3D_OUT_LDC,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_DPE_TO_OCC_MVSV_DMP_CFM[] =
{
    BID_DPE_OUT_DMP_L, BID_DPE_OUT_DMP_R,
    BID_DPE_OUT_CFM_L, BID_DPE_OUT_CFM_R,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_OCC_TO_WMF_OMYSN[] =
{
    BID_OCC_OUT_OCC, BID_OCC_OUT_NOC,
	BID_P2A_OUT_MY_S,BID_OCC_OUT_DS_MVY,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_OCC_OUT_INTERNAL_DEPTHMAP[] =
{
    BID_OCC_INTERAL_DEPTHMAP,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_GF_OUT_DEPTH_WRAPPER[] =
{
    BID_GF_OUT_DEPTH_WRAPPER,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_WMF_TO_GF_OND[] =
{
    BID_WMF_OUT_DMW,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_OUT_MV_F[] =
{
    BID_P2A_OUT_MV_F,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_OUT_FD[] =
{
    BID_P2A_OUT_FDIMG,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_OUT_MV_F_CAP[] =
{
    BID_P2A_OUT_MV_F_CAP,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_OUT_YUV_DONE[] =
{
    BID_P2A_OUT_POSTVIEW,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_OUT_DEPTHMAP[] =
{
    BID_P2A_OUT_DEPTHMAP,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_WMF_TO_GF_DMW_MY_S[] =
{
    BID_WMF_OUT_DMW,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_FD_OUT_EXTRADATA[] =
{
    BID_FD_OUT_EXTRADATA,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_N3D_OUT_JPS_WARPMTX[] =
{
    BID_N3D_OUT_JPS_MAIN1, BID_N3D_OUT_JPS_MAIN2,
    BID_N3D_OUT_WARPING_MATRIX,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_TO_DUMP_RAWS[] =
{
    BID_P2A_IN_FSRAW1, BID_P2A_IN_FSRAW2,
    BID_P2A_IN_RSRAW1, BID_P2A_IN_RSRAW2,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_TO_DUMP_BUFFERS[] =
{
    BID_P2A_FE2B_INPUT, BID_P2A_FE1B_INPUT,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_DPE_OUT_DISPARITY[] =
{
    BID_DPE_OUT_DMP_L, BID_DPE_OUT_DMP_R,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_TO_OCC_MY_S[] =
{
    BID_P2A_OUT_MY_S,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_GF_OUT_DMBG[] =
{
    BID_GF_OUT_DMBG, BID_GF_INTERNAL_DEPTHMAP,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_TO_GF_DMW_MYS[] =
{
    BID_P2A_OUT_MY_S, BID_WMF_OUT_DMW,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_QUEUED_FLOW_DONE[] =
{
    BID_WMF_OUT_DMW,
    // end with this
    BID_INVALID
};


#ifdef GTEST
DepthMapBufferID bufIDs_UT_OUT_FE[] =
{
    BID_FE2_HWIN_MAIN1, BID_FE2_HWIN_MAIN2,
    BID_FE3_HWIN_MAIN1, BID_FE3_HWIN_MAIN2,
    // end with this
    BID_INVALID
};
#endif

MVOID addDataIDToBIDMapItem(
    DataIDToBIDMap& outMap,
    DepthMapDataID dataID,
    DepthMapBufferID* bidArray
)
{
    Vector<DepthMapBufferID> bidMap;
    DepthMapBufferID* pBufferID = bidArray;
    while(*pBufferID != BID_INVALID)
    {
        bidMap.add(*pBufferID);
        pBufferID++;
    }
    outMap.add(dataID, bidMap);
}

DataIDToBIDMap getDataIDToBIDMap()
{
    #define ADD_TO_DBIDMAP(DATAID) \
        addDataIDToBIDMapItem(dBIDMap, DATAID, bufIDs_##DATAID);

    // data ID to BIDs map
    DataIDToBIDMap dBIDMap;

    ADD_TO_DBIDMAP(P2A_TO_N3D_FEFM_CCin);
    ADD_TO_DBIDMAP(P2A_TO_FD_IMG);
    ADD_TO_DBIDMAP(P2A_TO_OCC_MY_S);
    ADD_TO_DBIDMAP(P2A_TO_GF_DMW_MYS);
    ADD_TO_DBIDMAP(N3D_TO_DPE_MVSV_MASK);
    ADD_TO_DBIDMAP(N3D_TO_OCC_LDC);
    ADD_TO_DBIDMAP(DPE_TO_OCC_MVSV_DMP_CFM);
    ADD_TO_DBIDMAP(OCC_TO_WMF_OMYSN);
    ADD_TO_DBIDMAP(WMF_TO_GF_DMW_MY_S);
    ADD_TO_DBIDMAP(WMF_TO_GF_OND);

    ADD_TO_DBIDMAP(P2A_OUT_MV_F);
    ADD_TO_DBIDMAP(P2A_OUT_FD);
    ADD_TO_DBIDMAP(P2A_OUT_MV_F_CAP);
    ADD_TO_DBIDMAP(P2A_OUT_YUV_DONE);
    ADD_TO_DBIDMAP(P2A_OUT_DEPTHMAP);
    ADD_TO_DBIDMAP(GF_OUT_DMBG);
    ADD_TO_DBIDMAP(FD_OUT_EXTRADATA);
    ADD_TO_DBIDMAP(N3D_OUT_JPS_WARPMTX);
    ADD_TO_DBIDMAP(DPE_OUT_DISPARITY);
    ADD_TO_DBIDMAP(GF_OUT_DEPTH_WRAPPER);
    ADD_TO_DBIDMAP(OCC_OUT_INTERNAL_DEPTHMAP);

    ADD_TO_DBIDMAP(TO_DUMP_RAWS);
    ADD_TO_DBIDMAP(TO_DUMP_BUFFERS);
    ADD_TO_DBIDMAP(QUEUED_FLOW_DONE);
    #ifdef GTEST
    ADD_TO_DBIDMAP(UT_OUT_FE);
    #endif
    //
    return dBIDMap;

}

}; // namespace NSFeaturePipe_DepthMap
}; // namespace NSCamFeature
}; // namespace NSCam
