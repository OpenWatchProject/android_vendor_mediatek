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

#include "../FrameUtils.h"
#include "../Processor.h"
#include <mtkcam/pipeline/hwnode/P2Node.h>
class MdpProcedure : virtual public android::RefBase {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Processor Creator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct CreateParams {
        MINT32 uOpenId;
        MBOOL bEnableLog;
    };

    static sp<Processor> createProcessor(CreateParams &params);

    ~MdpProcedure() { }

    MdpProcedure(CreateParams const &params)
            : mbEnableLog(params.bEnableLog),
              muOpenId(params.uOpenId)
    {
        muDumpBuffer = ::property_get_int32("vendor.debug.camera.dump.mdp", 0);
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Frame Parameter
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct FrameInput {
        sp<BufferHandle> mHandle;
    };

    struct FrameOutput {
        sp<BufferHandle> mHandle;
        MINT32 mTransform;
    };

    struct FrameParams {
        MUINT32 uUniqueKey;
        MUINT32 uRequestNo;
        MUINT32 uFrameNo;       
        sp<Cropper::CropInfo> pCropInfo;
        FrameInput in;
        Vector<FrameOutput> vOut;
        StreamId_T capStreamId;
        MINT32 iso;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Processor Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
public:
    static const MBOOL isAsync = MFALSE;

    MERROR onMdpPullParams(
            sp<Request> pRequest,
            FrameParams &params
    );

    MERROR onMdpExecute(
            sp<Request> const pRequest,
            FrameParams const &params
    );

    MERROR onMdpFinish(
            FrameParams const &params,
            MBOOL const success
    );

    MVOID onMdpFlush() { };

    MVOID onMdpNotify(
            __attribute__((unused))MUINT32 const event,
            __attribute__((unused))MINTPTR const arg1,
            __attribute__((unused))MINTPTR const arg2,
            __attribute__((unused))MINTPTR const arg3
    ) { };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Procedure Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL const mbEnableLog;
    MUINT32 const muOpenId;
    MUINT32 muDumpBuffer;

};

DECLARE_PROC_TRAIT(Mdp);
