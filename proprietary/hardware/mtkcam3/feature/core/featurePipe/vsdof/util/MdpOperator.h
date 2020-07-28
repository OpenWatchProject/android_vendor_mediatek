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

#ifndef _MTK_CAMERA_FEATURE_PIPE_MDP_OPERATOR_H_
#define _MTK_CAMERA_FEATURE_PIPE_MDP_OPERATOR_H_
//
/*******************************************************************************
* MdpOperator is derived from ImageTransform with specilized MDP usage
* such as:
* 1.Bayer12_UNPAK/Bayer14_UNPAK format support
* 2.customization of src/dst buffer size or stride,
* 3.make mdp porcess to single plane among multi-plane image buffers, etc...
*******************************************************************************/

// Standard C header file
#include <string>

// Android system/core header file
#include <utils/RefBase.h>
#include <utils/Mutex.h>

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <DpIspStream.h>

// Module header file

// Local header file

namespace VSDOF{
namespace util{

using namespace std;
using namespace NSCam;

const std::string MY_NAME = "MdpOperator";

struct customConfig{
    MBOOL custStride = MFALSE;
    MSize size = MSize(0, 0);
    MINT32 planeIdx = -1;
};

struct MdpConfig
{
    IImageBuffer* pSrcBuffer = nullptr;
    IImageBuffer* pDstBuffer = nullptr;
    MUINT32 trans = 0;

    customConfig srcCust;
    customConfig dstCust;

    // PQ parameters
    MBOOL usePQParams = MFALSE;
    MINT32 featureId  = 0;
    MINT32 processId  = 0;
};

class MdpOperator
    : public virtual android::RefBase
{
    public:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  MdpOperator Interfaces.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                MdpOperator(
                                    const char* creatorName,
                                    MINT32 openId
                                );

        virtual                 ~MdpOperator();

        MBOOL                   execute(MdpConfig& config);
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        //  RefBase Interface.
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    public:
        virtual MVOID           onLastStrongRef(
                                    const void* /*id*/);

    protected:
        MBOOL             convertTransform(
                          MUINT32 const u4Transform,
                          MUINT32 & u4Rotation,
                          MUINT32 & u4Flip
                      );

        MBOOL             mapDpFormat(
                              NSCam::EImageFormat const fmt,
                              DpColorFormat* dp_fmt
                          );

        MBOOL             configPort(
                              MUINT32 const port,
                              IImageBuffer const *pBufInfo,
                              MINT32 width, MINT32 height, MINT32 stride,
                              MINT32 specifiedPlane = -1,
                              EImageFormat specifiedFmt = eImgFmt_UNKNOWN
                          );

        MBOOL             enqueBuffer(
                              MUINT32 const port,
                              IImageBuffer const *pBufInfo,
                              MINT32 specifiedPlane = -1
                          );

        MBOOL             dequeDstBuffer(
                              MUINT32 const port,
                              IImageBuffer const *pBufInfo,
                              MINT32 specifiedPlane = -1
                          );

        MVOID             setPQParameters(
                          //  MUINT32 port,
                          //  MINT32 featureID,
                          //  MINT32 processId
                          );

    private:
        const char*                             mCreatorName;
        MINT32                                  miOpenId = -1;

        mutable android::Mutex                  mLock;

        DpIspStream*                            mpStream = nullptr;
};

};
};
#endif  //_MTK_CAMERA_FEATURE_PIPE_MDP_OPERATOR_H_

