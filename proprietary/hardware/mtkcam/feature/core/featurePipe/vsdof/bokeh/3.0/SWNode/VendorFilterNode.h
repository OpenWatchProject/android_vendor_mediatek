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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_VENDOR_FILTER_NODE_H_
#define _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_VENDOR_FILTER_NODE_H_
//
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
//
#include "../BokehPipeNode.h"
// Dump naming rule
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
//
using namespace android;
//
//
class DpBlitStream;
//
namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

const int VENDOR_NODE_MAX_FD_FACES_NUM = 15;  // Max Number of faces can be detected based on MTK FD spec

class VendorFilterNode : public BokehPipeNode
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    VendorFilterNode() = delete;
    VendorFilterNode(
            const char *name,
            Graph_T *graph,
            MINT8 mode = GENERAL);
    virtual ~VendorFilterNode();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BokehPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL onData(DataID id, EffectRequestPtr &request);
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();
//************************************************************************
// BokehPipeNode Private Operations
//************************************************************************
private:
    /**
     * @brief cleanup function - release hal/resources
    */
    MVOID cleanUp();
    //
    struct VendorInputFdInfo
    {
        MPoint fdRoiTopLeft      = MPoint(0, 0);               // FD ROI top left point
        MPoint fdRoiBotRight     = MPoint(0, 0);               // FD ROI bottom right point
        MINT32 fdRotationDeg     = 0;                          // FD rotation information
    };
    //
    struct VendorInputBuffer
    {
        sp<IImageBuffer> pInClearImage       = nullptr;        // Main1 Image buffer
        sp<IImageBuffer> pInDepthmap         = nullptr;        // Depthmap buffer
        sp<IImageBuffer> pInDepthmapRotate   = nullptr;        // Rotated depthmap buffer (Align to clear image)
    };
    struct VendorOutputBuffer
    {
        sp<IImageBuffer> pOutBokehImage = nullptr;              // Bokeh Image for capture result
    };
    struct VendorInputParameter
    {
        MUINT8 afState           = 0;                           // af State(Follow google af state defination, 0~6)
        MINT32 bokehLevel        = 0;                           // Bokeh level from AP
        MPoint afRoi             = MPoint(0, 0);                // AF ROI
        MINT32 faceCounts        = 0;                           // Face detection result, number of faces
        MFLOAT iDistance         = 0;                           // Distance Measurement
        VendorInputFdInfo fdInfo[VENDOR_NODE_MAX_FD_FACES_NUM]; // Face detection result container
    };
    /**
     * @brief Config Vendor lib IO buffers
     * @param [in] request data
     * @param [in] input data for 3rd vendor lib to be config
     * @param [in] output data for 3rd vendor lib to be config
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL prepareVendorInOutData(
            const EffectRequestPtr& request,
            VendorInputBuffer* inputBuffers,
            VendorOutputBuffer* outputBuffers,
            VendorInputParameter* inputParameters);
    /**
     * @brief process third party vendor lib
     * @param [in] request data for vendor lib
     * @param [in] input buffers for vendor lib
     * @param [in] output buffers for vendor lib
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL processVendor(
            const VendorInputParameter& inputParameters,
            VendorInputBuffer inputBuffers,
            VendorOutputBuffer outputBuffers);
    /**
     * @brief an example to copy source to target buffer by MDP hardware
     * @param [in] source buffer
     * @param [in] target buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL copy(
            sp<IImageBuffer> source,
            sp<IImageBuffer> target);

    /**
     * @brief an example to rotate buffer
     * @param [in] source buffer
     * @param [in] source size
     * @param [in] output buffer
     * @param [in] rotation direction
     * @return
     *  - true roate success
     *  - false roate failure
     */
    bool rotateBuffer(const MUINT8 *SRC_BUFFER,
                      const MSize SRC_SIZE,
                      MUINT8 *dstBuffer,
                      ENUM_ROTATION targetRotation);

    /**
     * @brief Dump Buffer
     * @param [in] EffectRquest Pointer
     * @param [in] Input Buffer
     * @Param [in] Output Buffer
     * @Return
     *  - N/A
     */
    void dumpBuffers(
            const EffectRequestPtr&   request,
            const VendorInputBuffer&  inBuffers,
            const VendorOutputBuffer& outBuffers);
//************************************************************************
// BokehPipeNode Private member
//************************************************************************
private:
    WaitQueue<EffectRequestPtr>             mRequests;
    DpBlitStream*                           mpDpStream = nullptr;
    // Vendor process, default is on
    MINT32                                  mVendorProcess = 1;
    MINT64                                  mVendorFdTolerence = 0;
    // Dump hint, for generating dump path
    TuningUtils::FILE_DUMP_NAMING_HINT      mDumpHint;
    sp<ImageBufferPool>                     mpVendorDepthMapBufPool = nullptr;
    IImageBuffer                            *mRotatedDepthmap = nullptr;
    ENUM_ROTATION                           mTargeRotation   = eRotate_0;
    // Map to 4:3 or 16:9 sensor domain buffer information
    MRect                                   mActiveCropSift;
};

};
};
};
#endif // _MTK_CAMERA_VSDOF_BOKEH_FEATURE_PIPE_SW_FILTER_NODE_H_
