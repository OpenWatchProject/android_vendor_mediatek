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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
DepthMapBufferID bufIDs_P2A_TO_N3D_RECT2_FEO[] =
{
    BID_P2A_OUT_FE2BO, BID_P2A_OUT_FE2CO, BID_P2A_OUT_RECT_IN2,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_TO_N3D_CAP_RECT2[] =
{
    BID_P2A_OUT_FE1BO, BID_P2A_OUT_FE2BO, BID_P2A_OUT_FE1CO, BID_P2A_OUT_FE2CO,
    BID_P2A_OUT_FMBO_LR, BID_P2A_OUT_FMBO_RL, BID_P2A_OUT_FMCO_LR, BID_P2A_OUT_FMCO_RL,
    BID_P2A_OUT_RECT_IN2, BID_N3D_OUT_MV_Y, BID_P2A_IN_YUV1, BID_P2A_IN_YUV2,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_TO_N3D_NOFEFM_RECT2[] =
{
    BID_P2A_OUT_RECT_IN2,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_TO_N3D_FEOFMO[] =
{
    BID_P2A_OUT_FE2BO, BID_P2A_OUT_FE2CO, BID_P2A_OUT_FE1BO, BID_P2A_OUT_FE1CO,
    BID_P2A_OUT_FMBO_LR, BID_P2A_OUT_FMBO_RL, BID_P2A_OUT_FMCO_LR, BID_P2A_OUT_FMCO_RL,
    BID_P2A_FE1B_INPUT, BID_P2A_FE1C_INPUT,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_WPE_TO_DLDEPTH_MV_SV[] =
{
    BID_WPE_OUT_SV_Y,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_N3D_TO_WPE_RECT2_WARPMTX[] =
{
    BID_N3D_OUT_WARPMTX_MAIN2_X, BID_N3D_OUT_WARPMTX_MAIN2_Y,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_WPE_TO_DPE_WARP_IMG[] =
{
    BID_WPE_IN_MASK_S, BID_N3D_OUT_MV_Y, BID_N3D_OUT_MASK_M,
    BID_WPE_OUT_SV_Y, BID_WPE_OUT_MASK_S,
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

DepthMapBufferID bufIDs_TO_DUMP_RAWS[] =
{
    BID_P2A_IN_FSRAW1, BID_P2A_IN_RSRAW1,
    BID_P2A_IN_RSRAW2, BID_P2A_IN_LCSO1,
    BID_P2A_IN_LCSO2,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_TO_DUMP_BUFFERS[] =
{
    BID_P2A_FE2B_INPUT, BID_P2A_FE1B_INPUT,
    BID_P2A_FE1C_INPUT, BID_P2A_FE2C_INPUT,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_TO_DUMP_IMG3O[] =
{
    BID_P2A_INTERNAL_IMG3O,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_TO_DPE_MY_S[] =
{
    BID_P2A_OUT_MY_S, BID_P2A_INTERNAL_FD,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_P2A_NORMAL_FRAME_DONE[] =
{
    BID_GF_OUT_DMBG, BID_DLDEPH_DPE_OUT_DEPTH,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_GF_OUT_DMBG[] =
{
    BID_GF_OUT_DMBG,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_GF_OUT_INTERNAL_DMBG[] =
{
    BID_GF_INTERNAL_DMBG,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_DPE_OUT_INTERNAL_DEPTH[] =
{
    BID_OCC_OUT_NOC_M,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_QUEUED_FLOW_DONE[] =
{
    BID_WMF_OUT_DMW,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_DLDEPTH_OUT_DEPTHMAP[] =
{
    BID_DLDEPH_DPE_OUT_DEPTH,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_DPE_TO_GF_DMW_N_DEPTH[] =
{
    BID_DVS_OUT_DV_LR, BID_OCC_OUT_CFM_M, BID_OCC_OUT_NOC_M,
    BID_ASF_OUT_CRM, BID_ASF_OUT_RD, BID_ASF_OUT_HF, BID_WMF_OUT_DMW,
    BID_P2A_OUT_MY_S,
    // end with this
    BID_INVALID
};

DepthMapBufferID bufIDs_N3D_TO_P2ABYER_P1YUV_USED[] =
{
    BID_N3D_OUT_MV_Y, BID_N3D_OUT_MASK_M, BID_N3D_OUT_WARPMTX_MAIN2_X,
    BID_N3D_OUT_WARPMTX_MAIN2_Y, BID_P2A_IN_RECT_IN1,
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
    #define ADD_TO_DBIDMAP_SEC(DATAID, DATAID2) \
        addDataIDToBIDMapItem(dBIDMap, DATAID, bufIDs_##DATAID);\
        addDataIDToBIDMapItem(dBIDMap, DATAID2, bufIDs_##DATAID);

    // data ID to BIDs map
    DataIDToBIDMap dBIDMap;
    ADD_TO_DBIDMAP(P2A_TO_N3D_RECT2_FEO);
    ADD_TO_DBIDMAP(P2A_TO_N3D_NOFEFM_RECT2);
    ADD_TO_DBIDMAP(P2A_TO_N3D_CAP_RECT2);
    ADD_TO_DBIDMAP(P2A_TO_DPE_MY_S);
    ADD_TO_DBIDMAP(N3D_TO_WPE_RECT2_WARPMTX);
    ADD_TO_DBIDMAP(WPE_TO_DPE_WARP_IMG);
    ADD_TO_DBIDMAP(WPE_TO_DLDEPTH_MV_SV);
    ADD_TO_DBIDMAP(P2A_TO_N3D_FEOFMO);
    ADD_TO_DBIDMAP(DPE_TO_GF_DMW_N_DEPTH);
    ADD_TO_DBIDMAP(N3D_TO_P2ABYER_P1YUV_USED);

    ADD_TO_DBIDMAP(P2A_OUT_MV_F);
    ADD_TO_DBIDMAP(P2A_OUT_FD);
    ADD_TO_DBIDMAP(P2A_OUT_MV_F_CAP);
    ADD_TO_DBIDMAP(P2A_OUT_YUV_DONE);
    ADD_TO_DBIDMAP(P2A_NORMAL_FRAME_DONE);
    ADD_TO_DBIDMAP(GF_OUT_DMBG);
    ADD_TO_DBIDMAP(GF_OUT_INTERNAL_DMBG);
    ADD_TO_DBIDMAP(DPE_OUT_INTERNAL_DEPTH);

    ADD_TO_DBIDMAP(DLDEPTH_OUT_DEPTHMAP);
    ADD_TO_DBIDMAP(TO_DUMP_RAWS);
    ADD_TO_DBIDMAP(TO_DUMP_BUFFERS);
    ADD_TO_DBIDMAP(TO_DUMP_IMG3O);
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
