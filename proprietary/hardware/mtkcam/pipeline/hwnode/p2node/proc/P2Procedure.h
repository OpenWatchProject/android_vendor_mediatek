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

#include <mtkcam/pipeline/hwnode/P2Node.h>
#include <mtkcam/utils/std/DebugScanLine.h>

typedef IHalISP IHalISP_T;

class P2Procedure : virtual public android::RefBase {

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
#if SUPPORT_EIS_MV
            MUINT32 x_mv_int;
            MUINT32 x_mv_float;
            MUINT32 y_mv_int;
            MUINT32 y_mv_float;
            MUINT32 is_from_zzr;
#endif
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
        IHalISP_T *pIsp;
        INormalStream *pPipe;
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
                : mTransform(0) , mUsage(0){ };
        NSIoPipe::PortID mPortId;
        sp<BufferHandle> mHandle;
        MINT32 mTransform;
        MUINT32 mUsage;
    };

    struct FrameParams {
        FrameParams()
                : uUniqueKey(0),
                  uRequestNo(0),
                  uFrameNo(0),
                  bBurstEnque(MFALSE),
                  bYuvReproc(MFALSE),
                  bReentry(MFALSE),
                  uPass2Count(0),
#if SUPPORT_PLUGIN || SUPPORT_3RD_PARTY
                  bRunWorkBuffer(MFALSE),
#endif
                  capStreamId(0)
                  {};
        MUINT32 uUniqueKey;
        MUINT32 uRequestNo;
        MUINT32 uFrameNo;
        FrameInput in;
        FrameInput in_lcso;
        MBOOL bBurstEnque;
        Vector<FrameOutput> vOut;
        //
        MBOOL bResized;
        MBOOL bYuvReproc;
        MBOOL bReentry;
        MUINT32 uPass2Count;
#if SUPPORT_PLUGIN || SUPPORT_3RD_PARTY
        MBOOL bRunWorkBuffer;
#endif
        StreamId_T capStreamId;
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
            MUINT32 const /*event*/,
            MINTPTR const /*arg1*/,
            MINTPTR const /*arg2*/,
            MINTPTR const /*arg3*/
    );


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Multi-frame Handler
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    class MultiFrameHandler {
    public:
        MultiFrameHandler(INormalStream *const pPipe, MBOOL bEableLog)
                : mpPipe(pPipe), mbEnableLog(bEableLog), muMfEnqueCnt(0), muMfDequeCnt(0){
        }

        MERROR collect(sp<Request>, QParams &, MBOOL);

        MERROR enque();

        MVOID flush();

        static MVOID callback(QParams &rParams) {
            MultiFrameHandler *pHandler = reinterpret_cast<MultiFrameHandler *>(rParams.mpCookie);
            pHandler->deque(rParams);
        }

        MVOID deque(QParams &rParams);

    private:
        INormalStream *const mpPipe;
        mutable Mutex mLock;
        MBOOL const mbEnableLog;
        QParams mParamCollecting;
        MUINT32 muMfEnqueCnt;
        MUINT32 muMfDequeCnt;
        vector<sp<Request>> mvReqCollecting;
        vector<vector<sp<Request>>> mvRunning;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Procedure Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MultiFrameHandler *mpMultiFrameHandler;
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
    IHalISP_T *const mpIsp;
    MBOOL mConfigVencStream;
    //
    MUINT32 muDumpBuffer;
    MUINT32 muDumpCondIn;
    MUINT32 muDumpPortIn;
    MUINT32 muDumpPortOut;
    MUINT32 muDumpPortImg3o;
    MUINT32 muDumpPortMfbo;
    MUINT muSensorFormatOrder;
    //
    MUINT32 muRequestCnt;
    //
    MUINT32 muEnqueCnt;
    MUINT32 muDequeCnt;
    vector<sp<Request>> mvRunning;
    //
    MUINT32 mDebugScanLineMask;
    DebugScanLine *mpDebugScanLine;
    //
#if P2_DEBUG_DEQUE
    vector<QParams> mvParams;
#endif

    //For SMVR ALPS03996212
    MetaSet_T mMetaSetISP;
    void*     mpTuning;
    mutable Mutex mMetaLock;

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
            Cropper::CropInfo &cropInfo,
            MBOOL const RefEIS = true
    ) const;

    MVOID queryCropRegion(
            IMetadata *const inApp,
            IMetadata *const inHal,
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
