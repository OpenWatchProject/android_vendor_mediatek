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
#include "../P2Common.h"

#include <mtkcam/v3/hal/IHal3A.h>

#include <mtkcam/v3/hwnode/P2Node.h>

typedef NSCam::NSIoPipe::NSPostProc_FrmB::QParams QParams;
typedef NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream INormalStream;

class P2Procedure {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Event
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    enum ProcedureEvent {
        eP2_START_VENC_STREAM,  // arg1: fps, arg2: width, arg3: height
        eP2_STOP_VENC_STREAM    //
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Private structures
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    struct eis_region {
        MUINT32 x_int;
        MUINT32 x_float;
        MUINT32 y_int;
        MUINT32 y_float;
        MSize s;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Processor Creator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct CreateParams {
        MINT32 uOpenId;
        MBOOL bEnableLog;
        P2Node::ePass2Type type;
        MRect activeArray;
        NS3Av3::IHal3A *p3A;
        INormalStream *pPipe;
        PoolBufferAllocatorMap *pPoolAllocatorMap;
    };

    static sp<Processor> createProcessor(CreateParams &params);

    ~P2Procedure();

    P2Procedure(CreateParams const &params);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Frame Parameter
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct FrameInput {
        NSIoPipe::PortID mPortId;
        sp<BufferHandle> mHandle;
    };

    struct FrameOutput {
        FrameOutput()
                : mUsage(0), mTransform(0) { };
        NSIoPipe::PortID mPortId;
        sp<BufferHandle> mHandle;
        MINT32 mTransform;
        MUINT32 mUsage;
    };

    struct FrameParams {
        MUINT32 uUniqueKey;
        MUINT32 uRequestNo;
        MUINT32 uFrameNo;
        FrameInput in;
        Vector<FrameOutput> vOut;
        //
        MBOOL bResized;
        MBOOL bYuvReproc;
#if SUPPORT_PLUGIN
        MBOOL bRunWorkBuffer;
#endif
        //
        sp<MetaHandle> inApp;
        sp<MetaHandle> inHal;
        sp<MetaHandle> outApp;
        sp<MetaHandle> outHal;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Processor Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static const MBOOL isAsync = MTRUE;

    MERROR onP2PullParams(
            sp<Request> pRequest,
            FrameParams &params
    );

    MERROR onP2Execute(
            sp<Request> const pRequest,
            FrameParams const &params
    );

    MERROR onP2Finish(
            FrameParams const &params,
            MBOOL const success
    );

    MVOID onP2Flush();

    MVOID onP2Notify(
            MUINT32 const event,
            MINTPTR const arg1,
            MINTPTR const arg2,
            MINTPTR const arg3
    );


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Procedure Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL const mbEnableLog;
    MUINT32 const muOpenId;
    //
    mutable Mutex mLock;
    mutable Condition mCondJob;
    //
    CreateParams const mCreateParams;
    //
    INormalStream *const mpPipe;
    MINT32 mnStreamTag;
    NS3Av3::IHal3A *const mp3A;
    MBOOL mConfigVencStream;
    //
    MUINT32 mSupportPlugin;
    //
    MUINT32 muRequestCnt;
    //
    MUINT32 muEnqueCnt;
    MUINT32 muDequeCnt;
    vector<sp<Request>> mvRunning;
    //
    PoolBufferAllocatorMap *mpPoolAllocatorMap;
#if P2_DEBUG_DEQUE
    vector<QParams> mvParams;
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MERROR mapPortId(
            StreamId_T const streamId, // [in]
            MUINT32 const transform,   // [in]
            MBOOL const isFdStream,    // [in]
            MUINT8 &rOccupied,         // [in/out]
            NSIoPipe::PortID &rPortId  // [out]
    ) const;

    MERROR checkParams(FrameParams const params) const;

    MERROR getCropInfo(
            IMetadata *const inApp,
            IMetadata *const inHal,
            MBOOL const isResized,
            Cropper::CropInfo &cropInfo
    ) const;

    MVOID queryCropRegion(
            IMetadata *const meta_request,
            MBOOL const isEisOn,
            MRect &targetCrop
    ) const;

    MVOID updateCropRegion(
            MRect const crop,
            IMetadata *meta_result
    ) const;

    MBOOL isEISOn(
            IMetadata *const inApp
    ) const;

    MBOOL queryEisRegion(
            IMetadata *const inHal,
            eis_region &region
    ) const;

    static MVOID pass2CbFunc(QParams &rParams);

    MVOID handleDeque(QParams &rParams);

};

DECLARE_PROC_TRAIT(P2);
