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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_TASK_CTRL_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_TASK_CTRL_H_

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include "P1Common.h"
#include "P1Utility.h"
//#include "P1ConnectLMV.h"
//#include "P1NodeImp.h"
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::NSP1Node;
using namespace NSCam::Utils::Sync;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NS3Av3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSCam {
namespace v3 {
namespace NSP1Node {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 1
#ifdef P1ACT_ID_NULL
#undef P1ACT_ID_NULL
#endif
#define P1ACT_ID_NULL (P1_QUE_ID_NULL)
#endif

#if 1
#ifdef P1ACT_ID_FIRST
#undef P1ACT_ID_FIRST
#endif
#define P1ACT_ID_FIRST (P1_QUE_ID_FIRST)
#endif

#if 1
#ifdef P1ACT_NUM_NULL
#undef P1ACT_NUM_NULL
#endif
#define P1ACT_NUM_NULL (P1_MAGIC_NUM_NULL)
#endif

#if 1
#ifdef P1ACT_NUM_FIRST
#undef P1ACT_NUM_FIRST
#endif
#define P1ACT_NUM_FIRST (P1_MAGIC_NUM_FIRST)
#endif

#ifdef RET_VOID
#undef RET_VOID
#endif
#define RET_VOID /* return void */

#ifdef GET_ACT_PTR
#undef GET_ACT_PTR
#endif
#define GET_ACT_PTR(pAct, qAct, ret) \
    (qAct).ptr();\
    U_if (pAct == NULL)\
    { MY_LOGE("#%d can-not-get-act", __LINE__); return ret; };


#define P1INFO_ACT_STREAM(LOG_LEVEL, TYPE) MY_LOGD##LOG_LEVEL(\
    P1INFO_STREAM_STR P1INFO_ACT_STR,\
    P1INFO_STREAM_VAR(TYPE), P1INFO_ACT_VAR(*this));

#define P1_CHECK_ACT_STREAM(TYPE, stream)\
    P1_CHECK_MAP_STREAM(TYPE, (*this), stream)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/
class P1NodeImp;


/******************************************************************************
 *
 ******************************************************************************/
class P1NodeAct
    : public android::LightRefBase<P1NodeAct> // // use-act-ptr
{
    //
    enum STREAM_BUF_LOCK
    {
        STREAM_BUF_LOCK_NONE        = 0,
        STREAM_BUF_LOCK_R,
        STREAM_BUF_LOCK_W
    };
    //
    #if 0 // use Common.h
    enum IMG_BUF_SRC
    {
        IMG_BUF_SRC_NULL           = 0,
        IMG_BUF_SRC_POOL,
        IMG_BUF_SRC_STUFF,
        IMG_BUF_SRC_FRAME
    };
    #endif

    struct BatchBuffer {
        enum SUB_STATE
        {
            SUB_STATE_AVAILABLE  = 0,
            SUB_STATE_ACQUIRED,
            SUB_STATE_USED
        };

        struct SubNote {
        public:
                            SubNote()
                                : mState(SUB_STATE_AVAILABLE)
                                , spImgBuf(NULL)
                            {};
                            SubNote(SUB_STATE state)
                                : mState(state)
                                , spImgBuf(NULL)
                            {};
            virtual         ~SubNote()
                            {
                                spImgBuf = NULL;
                            };

            SUB_STATE       mState;
            sp<IImageBuffer>
                            spImgBuf;
        };
        mutable Mutex       mLock;
        std::vector< SubNote >
                            mvMap;
        MUINT8              mLastGotIdx;
        //
        public:
                            BatchBuffer()
                                : mLock()
                                , mLastGotIdx(P1_SUB_IDX_UNKNOWN)
                            {
                                mvMap.clear();
                            };
        //
        size_t lockSizeQuery(void) const // must not be under BatchBuf.mLock locking
        {
            Mutex::Autolock _l(mLock);
            return mvMap.size();
        };
        //
        MUINT8 tryGet(sp<IImageBuffer> & rspImageBuffer) // must not be under BatchBuf.mLock locking
        {
            Mutex::Autolock _l(mLock);
            rspImageBuffer = NULL;
            #if 0 // only use the first buffer
            MUINT8 size = MIN(1, mvMap.size());
            #else
            MUINT8 size = mvMap.size();
            #endif
            MUINT8 res = P1_SUB_IDX_UNKNOWN;
            MUINT8 idx = (mLastGotIdx == P1_SUB_IDX_UNKNOWN) ?
                0 : (mLastGotIdx + 1);
            for (MUINT8 i = 0; i < size; i++) {
                U_if (idx >= size) {
                    idx = idx % size;
                };
                L_if (idx < size) {
                    BatchBuffer::SubNote & note = mvMap.at(idx);
                    if (BatchBuffer::SUB_STATE_AVAILABLE == note.mState) {
                        note.mState = BatchBuffer::SUB_STATE_ACQUIRED;
                        mLastGotIdx = idx;
                        res = idx;
                        rspImageBuffer = note.spImgBuf;
                        break;
                    }
                    idx++;
                };
            }
            return res;
        };
        //
        MBOOL tryPut(MUINT8 idx, sp<IImageBuffer> & rspImageBuffer) // must not be under BatchBuf.mLock locking
        {
            Mutex::Autolock _l(mLock);
            MUINT8 size = mvMap.size();
            MBOOL ret = MFALSE;
            L_if (idx < size) {
                BatchBuffer::SubNote & note = mvMap.at(idx);
                L_if (note.spImgBuf == rspImageBuffer) {
                    note.mState = BatchBuffer::SUB_STATE_USED;
                    ret = MTRUE;
                    return ret;
                }
                MY_LOGW("idx[%d/%d] note.spImgBuf(%p) != tarImgBuf(%p)",
                    idx, size, note.spImgBuf.get(), rspImageBuffer.get());
                return ret;
            }
            MY_LOGE("idx(%d) >= size(%d)", idx, size);
            return ret;
        };
    };

    struct SubStreamImg {
        //ActStreamImg *                  pActStreamImg;
        STREAM_BUF_LOCK                 eLockState;
        sp<IImageBuffer>                spImgBuf;
        MUINT8                          subImgIdx;
        IMG_BUF_SRC                     eSrcType;
        SubStreamImg()
            //: pActStreamImg(NULL)
            : eLockState(STREAM_BUF_LOCK_NONE)
            , spImgBuf(NULL)
            , subImgIdx(P1_SUB_IDX_UNKNOWN)
            , eSrcType(IMG_BUF_SRC_NULL) {
        };
        virtual ~SubStreamImg() {
        };
    };

    struct SubSetting {
        MUINT8                          index;
        MINT32                          magicNum;
        SubStreamImg                    subImg[STREAM_IMG_NUM];
        sp<IImageBuffer>                spEisImgBuf;
        MUINT32                         portBufIdx[P1_OUTPUT_PORT_TOTAL];
        MBOOL                           isFullySizeStuff;
        SubSetting(MUINT8 idx)
            : index(idx)
            , magicNum(P1ACT_NUM_NULL)
            , spEisImgBuf(NULL)
            , isFullySizeStuff(MFALSE) {
            ::memset(portBufIdx, P1_FILL_BYTE, sizeof(portBufIdx));
        };
        virtual ~SubSetting() {
        };
    };

    struct ActStreamImg {
        MBOOL                           bExist;
        MBOOL                           bInited;
        MBOOL                           bWrote;
        sp<IImageStreamBuffer>          spStreamBuf;
        sp<IImageStreamInfo>            spStreamInfo;
        sp<IImageBufferHeap>            spStreamHeap;
        IMG_BUF_SRC                     eSrcType;
        MBOOL                           bBatchEn;
        BatchBuffer                     sBatchBuf;
        ActStreamImg()
            : bExist(MFALSE)
            , bInited(MFALSE)
            , bWrote(MFALSE)
            , spStreamBuf(NULL)
            , spStreamInfo(NULL)
            , spStreamHeap(NULL)
            , eSrcType(IMG_BUF_SRC_NULL)
            , bBatchEn(MFALSE)
            , sBatchBuf() {
        };
        virtual ~ActStreamImg() {
        };
    };

    struct ActStreamMeta {
        MBOOL                           bExist;
        MBOOL                           bWrote;
        sp<IMetaStreamBuffer>           spStreamBuf;
        STREAM_BUF_LOCK                 eLockState;
        IMetadata*                      pMetadata;
        ActStreamMeta()
            : bExist(MFALSE)
            , bWrote(MFALSE)
            , spStreamBuf(NULL)
            , eLockState(STREAM_BUF_LOCK_NONE)
            , pMetadata(NULL)
        {};
    };

public:
    P1NodeAct(P1NodeImp * pP1NodeImp, MINT32 id = P1ACT_ID_NULL);

    virtual ~P1NodeAct();

public:

    char const*                 getNodeName(void) const;

    IPipelineNode::NodeId_T     getNodeId() const;

    MINT32                      getOpenId(void) const;

public:
    MINT32                      getNum(void) const;

    ACT_TYPE                    getType(void) const;

    MBOOL                       getFlush(FLUSH_TYPE type = FLUSH_ALL) const;

    MVOID                       setFlush(FLUSH_TYPE type);

    MERROR                      mapFrameStream(void);

    MERROR                      frameMetadataInit(
                                    STREAM_META const streamMeta,
                                    sp<IMetaStreamBuffer> &
                                    pMetaStreamBuffer
                                );

    MERROR                      frameMetadataGet(
                                    STREAM_META const streamMeta,
                                    IMetadata * pOutMetadata,
                                    MBOOL toWrite = MFALSE,
                                    IMetadata * pInMetadata = NULL
                                );

    MERROR                      frameMetadataPut(
                                    STREAM_META const streamMeta
                                );

    MERROR                      frameImageInit(
                                    STREAM_IMG const streamImg
                                );

    MERROR                      frameImageUninit(
                                    STREAM_IMG const streamImg
                                );

    MERROR                      frameImageGet(
                                    STREAM_IMG const streamImg,
                                    MINT8 const idx,
                                    sp<IImageBuffer> &rImgBuf
                                );

    MERROR                      frameImagePut(
                                    STREAM_IMG const streamImg,
                                    MINT8 const idx
                                );

    MERROR                      poolImageInit(
                                    STREAM_IMG const streamImg
                                );

    MERROR                      poolImageUninit(
                                    STREAM_IMG const streamImg
                                );

    MERROR                      poolImageGet(
                                    STREAM_IMG const streamImg,
                                    MINT8 const idx,
                                    sp<IImageBuffer> &rImgBuf
                                );

    MERROR                      poolImagePut(
                                    STREAM_IMG const streamImg,
                                    MINT8 const idx
                                );

    MERROR                      stuffImageGet(
                                    STREAM_IMG const streamImg,
                                    MINT8 const idx,
                                    MSize const dstSize,
                                    sp<IImageBuffer> &rImgBuf
                                );

    MERROR                      stuffImagePut(
                                    STREAM_IMG const streamImg,
                                    MINT8 const idx
                                );

    MERROR                      streamImageInit(
                                    STREAM_IMG const streamImg,
                                    IMG_BUF_SRC srcType
                                );

    MERROR                      streamImageUninit(
                                    STREAM_IMG const streamImg,
                                    IMG_BUF_SRC srcType
                                );

    MVOID                       updateMetaSet(void);

    MBOOL                       checkImgBufAddr(
                                    IImageBuffer* const imgBuffer
                                );

public:
    // for performance consideration, this act do not hold the sp of P1NodeImp
    // and the P1NodeImp instance should be held in P1TaskCtrl.mspP1NodeImp
    // therefore, it must keep the life-cycle of P1NodeAct within P1TaskCtrl
    P1NodeImp *                 mpP1NodeImp;
    android::String8            mNodeName;
    IPipelineNode::NodeId_T     mNodeId;
    MINT32                      mOpenId;
    MINT32                      mLogLevel;
    MINT32                      mLogLevelI;
    MINT32                      queId;
    MINT32                      magicNum;
    MINT32                      frmNum;
    MINT32                      reqNum;
    MUINT32                     sofIdx; // data type - refer: NSCam::NSIoPipe::NSCamIOPipe::BufInfo.FrameBased.mSOFidx
    sp<IPipelineFrame>          appFrame;
    ActStreamImg                poolBufImg[STREAM_IMG_NUM];
    ActStreamImg                streamBufImg[STREAM_IMG_NUM];
    ActStreamMeta               streamBufMeta[STREAM_META_NUM];
    std::vector< SubSetting >   vSubSetting;
    MUINT8                      mBatchNum;
    MUINT32                     portBufIndex[P1_OUTPUT_PORT_TOTAL];
    MUINT32                     reqType;    /*REQ_TYPE*/
    MUINT32                     reqOutSet;  /*REQ_SET(REQ_OUT)*/
    MUINT32                     expRec;     /*EXP_REC(EXP_EVT)*/
    MUINT32                     flushSet;   /*FLUSH_TYPE*/
    MUINT32                     exeState;   /*EXE_STATE*/
    MUINT32                     capType;    /*E_CAPTURE_TYPE*/
    MUINT32                     uniSwitchState; /*UNI_SWITCH_STATE*/
    MUINT32                     tgSwitchState; /*TG_SWITCH_STATE*/
    MUINT32                     tgSwitchNum;   /*TG_SWITCH_NUM*/
    MUINT32                     qualitySwitchState; /*QUALITY_SWITCH_STATE*/
    MUINT32                     qualitySwitchValue; /*CAM_QUALITY*/
    SENSOR_STATUS_CTRL          ctrlSensorStatus;
    MINT64                      frameExpDuration;
    MINT64                      frameTimeStamp;
    MINT64                      frameTimeStampBoot;
    MINT64                      expTimestamp;
    MBOOL                       isMapped;
    MBOOL                       isReadoutReady;
    MBOOL                       isRawTypeChanged;
    MINT32                      fullRawType;
    MSize                       refBinSize;
    MINT32                      selectCrz; // 0: crz disable, 1: crz1 enable, 2: crz2 enable
    MSize                       dstSize_full;
    MSize                       dstSize_resizer;
    MSize                       dstSize_yuv_full;
    MSize                       dstSize_yuv_resizer1;
    MSize                       dstSize_yuv_resizer2;
    MRect                       cropRect_full;
    MRect                       cropRect_resizer;
    MRect                       cropRect_yuv_full;
    MRect                       cropRect_yuv_resizer1;
    MRect                       cropRect_yuv_resizer2;
    MBOOL                       haveMetaScalerCrop;
    MRect                       rectMetaScalerCrop;
    MBOOL                       haveMetaSensorCrop;
    MRect                       rectMetaSensorCrop;
    MBOOL                       haveMetaYuvResizer1Crop;
    MRect                       rectMetaYuvResizer1Crop;
    MBOOL                       haveMetaYuvResizer2Crop;
    MRect                       rectMetaYuvResizer2Crop;
    MBOOL                       haveMetaResizerSet;
    MSize                       sizeMetaResizerSet;
    MetaSet_T                   metaSet;
    MINT32                      metaCntAaaAPP;
    MINT32                      metaCntAaaHAL;
    MINT32                      metaCntAaaIspAPP;
    MINT32                      metaCntAaaIspHAL;
    MINT32                      metaCntInAPP;
    MINT32                      metaCntInHAL;
    MINT32                      metaCntOutAPP;
    MINT32                      metaCntOutHAL;
    android::String8            msg;
    android::String8            res;
    MBOOL                       needNotifySyncRes;
    MINT                        mReqFmt_Imgo;
    MINT                        mReqFmt_Rrzo;
    MBOOL                       byDeliverReleasing;
    MBOOL                       mbSwtichWithoutWB;
    mutable Mutex               mNodeActLock;
    MBOOL                       mbIsReturned;

};

typedef sp<P1NodeAct> P1Act; // // use-act-ptr
//typedef P1NodeAct* P1Act; // use-act-ptr


/******************************************************************************
 *
 ******************************************************************************/
class P1QueAct {

    friend class P1TaskCtrl;
    friend class P1TaskCollector;

public:
                                P1QueAct()
                                : mKeyId(P1ACT_ID_NULL)
                                , mpAct(NULL)
                                {
                                };

                                P1QueAct(MINT32 id)
                                : mKeyId(id)
                                , mpAct(NULL)
                                {
                                };

                                P1QueAct(P1Act pAct, MINT32 id)
                                : mKeyId(id)
                                , mpAct(pAct)
                                {
                                };


    virtual                     ~P1QueAct()
                                {
                                };

public:
    MINT32                      id()
                                {
                                    return mKeyId;
                                };

    P1Act                       ptr()
                                {
                                    L_if (mpAct != NULL) {
                                        return mpAct;
                                    } else {
                                        MY_LOGI("Act not ready (%d)", mKeyId);
                                    }
                                    return NULL;
                                };

    MINT32                      getNum()
                                {
                                    U_if (mpAct == NULL) {
                                        return P1ACT_NUM_NULL;
                                    }
                                    return mpAct->getNum();
                                };

    ACT_TYPE                    getType()
                                {
                                    U_if (mpAct == NULL) {
                                        return ACT_TYPE_NULL;
                                    }
                                    return mpAct->getType();
                                };

    MBOOL                       ready()
                                {
                                    return (mKeyId != P1ACT_ID_NULL &&
                                        mpAct != NULL);
                                };

    MVOID                       clear()
                                {
                                    mKeyId = P1ACT_ID_NULL;
                                    mpAct = NULL;
                                };

protected:
    void                        set(P1Act ptr, MINT32 id)
                                {
                                    U_if (id == P1ACT_ID_NULL || ptr == NULL) {
                                        MY_LOGI("invalid (%d)[%d]",
                                            id, (ptr == NULL) ? 0 : 1);
                                        return;
                                    }
                                    U_if (mKeyId != P1ACT_ID_NULL ||
                                        mpAct != NULL) {
                                        MY_LOGI("already (%d)[%d]",
                                            mKeyId, (mpAct == NULL) ? 0 : 1);
                                        return;
                                    }
                                    mKeyId = id;
                                    mpAct = ptr;
                                    return;
                                };

private:
    MINT32                      mKeyId;
    P1Act                       mpAct;

};


/******************************************************************************
 *
 ******************************************************************************/
class P1QueJob {

public:
                                P1QueJob()
                                : mSet()
                                , mMaxNum(1)
                                , mFirstMagicNum(P1ACT_ID_NULL)
                                {
                                    mSet.clear();
                                    mSet.reserve(mMaxNum);
                                };

                                P1QueJob(MUINT8 num)
                                : mSet()
                                , mMaxNum(num)
                                , mFirstMagicNum(P1ACT_ID_NULL)
                                {
                                    mSet.clear();
                                    mSet.reserve(mMaxNum);
                                };

    virtual                     ~P1QueJob()
                                {
                                    mSet.clear();
                                };

public:
    MVOID                       setIdx(MINT32 idx)
                                {
                                    mFirstMagicNum = idx;
                                };

    MINT32                      getIdx(void) const
                                {
                                    return mFirstMagicNum;
                                };

    MUINT8                      getMax(void) const
                                {
                                    return mMaxNum;
                                };

    MVOID                       config(MUINT8 maxNum)
                                {
                                    mFirstMagicNum = P1ACT_ID_NULL;
                                    mMaxNum = maxNum;
                                    mSet.clear();
                                    mSet.reserve(mMaxNum);
                                };

    MVOID                       clear(void)
                                {
                                    mFirstMagicNum = P1ACT_ID_NULL;
                                    return mSet.clear();
                                };

    MBOOL                       ready(void)
                                {
                                    return (mSet.size() == mMaxNum &&
                                        mFirstMagicNum != P1ACT_ID_NULL);
                                };

    MBOOL                       empty(void) const
                                {
                                    return mSet.empty();
                                };

    size_t                      size(void) const
                                {
                                    return mSet.size();
                                };

    MVOID                       push(P1QueAct qAct)
                                {
                                    if (mSet.size() < mMaxNum) {
                                        return mSet.push_back(qAct);
                                    }
                                };

    P1QueAct &                  edit(MUINT8 index)
                                {
                                    return mSet.at(index);
                                };

    const P1QueAct &            at(MUINT8 index)
                                {
                                    return mSet.at(index);
                                };

    MINT32                      getLastId(void)
                                {
                                    if (mSet.size() < mMaxNum) {
                                        return P1ACT_ID_NULL;
                                    }
                                    return
                                        mSet.at(mMaxNum - 1).id();
                                };

    MINT32                      getLastNum(void)
                                {
                                    if (mSet.size() < mMaxNum) {
                                        return P1ACT_NUM_NULL;
                                    }
                                    return
                                        mSet.at(mMaxNum - 1).getNum();
                                };

    P1QueAct *                  getLastAct(void)
                                {
                                    if (mSet.size() < mMaxNum) {
                                        return NULL;
                                    }
                                    P1QueAct * qAct =
                                        &(mSet.at(mSet.size() - 1));
                                    return qAct;
                                };

public:
    std::vector< P1QueAct >     mSet;
private:
    MUINT8                      mMaxNum;
    MINT32                      mFirstMagicNum;

};


/******************************************************************************
 *
 ******************************************************************************/
class P1FrameAct {
public:
                                P1FrameAct(
                                    P1QueAct & rAct
                                )
                                : queId(P1ACT_ID_NULL)
                                , magicNum(P1ACT_NUM_NULL)
                                , frmNum(P1_FRM_NUM_NULL)
                                , reqNum(P1_REQ_NUM_NULL)
                                , sofIdx(P1SOFIDX_INIT_VAL)
                                , reqType(REQ_TYPE_UNKNOWN)
                                , reqOutSet(REQ_SET_NONE)
                                , expRec(EXP_REC_NONE)
                                , flushSet(FLUSH_NONEED)
                                , exeState(EXE_STATE_NULL)
                                , capType(E_CAPTURE_NORMAL)
                                , fullRawType(EPipe_PURE_RAW)
                                , frameTimeStamp(0)
                                , frameTimeStampBoot(0)
                                , byDeliverReleasing(MFALSE)
                                , appFrame(NULL)
                                , queAct()
                                {
                                    P1Act act =
                                        GET_ACT_PTR(act, rAct, RET_VOID);
                                    //
                                    queId =         act->queId;
                                    magicNum =      act->magicNum;
                                    frmNum =        act->frmNum;
                                    reqNum =        act->reqNum;
                                    sofIdx =        act->sofIdx;
                                    reqType =       act->reqType;
                                    reqOutSet =     act->reqOutSet;
                                    expRec =        act->expRec;
                                    flushSet =      act->flushSet;
                                    exeState =      act->exeState;
                                    capType =       act->capType;
                                    fullRawType =   act->fullRawType;
                                    frameTimeStamp =
                                        act->frameTimeStamp;
                                    frameTimeStampBoot =
                                        act->frameTimeStampBoot;
                                    //
                                    byDeliverReleasing =
                                        act->byDeliverReleasing;
                                    //
                                    U_if (byDeliverReleasing) {
                                        queAct = rAct;
                                    }
                                    //
                                    appFrame =      act->appFrame;
                                };

    virtual                     ~P1FrameAct()
                                {
                                };
    //
    MBOOL                       ready(void)
                                {
                                    return (queId != P1ACT_ID_NULL);
                                };
    //
    P1QueAct *                  getQueAct(void)
                                {
                                    return ((queAct.ready()) ? &queAct : NULL);
                                };
    //
    MVOID                       delQueAct(void)
                                {
                                    return (queAct.clear());
                                };
    //
    MINT32                      queId;
    MINT32                      magicNum;
    MINT32                      frmNum;
    MINT32                      reqNum;
    MUINT32                     sofIdx;
    //
    MUINT32                     reqType;    /*REQ_TYPE*/
    MUINT32                     reqOutSet;  /*REQ_SET(REQ_OUT)*/
    MUINT32                     expRec;     /*EXP_REC(EXP_EVT)*/
    MUINT32                     flushSet;   /*FLUSH_TYPE*/
    MUINT32                     exeState;   /*EXE_STATE*/
    MUINT32                     capType;    /*E_CAPTURE_TYPE*/
    MINT32                      fullRawType;
    MINT64                      frameTimeStamp;
    MINT64                      frameTimeStampBoot;
    MBOOL                       byDeliverReleasing;
    //
    sp<IPipelineFrame>          appFrame;
    //
private:
    //
    P1QueAct                    queAct;

};


/******************************************************************************
 *
 ******************************************************************************/
class P1TaskCtrl;

class P1TaskCollector
    : public android::LightRefBase<P1TaskCollector>
{
public:
    P1TaskCollector(sp<P1TaskCtrl> spTaskCtrl);

    virtual ~P1TaskCollector();

public:
    MVOID                       config(void);

    MVOID                       reset(void);

    MINT                        queryAct(P1QueAct & rDupAct,
                                    MUINT32 index = 0);

    MINT                        enrollAct(P1QueAct & rNewAct);

    MINT                        verifyAct(P1QueAct & rSetAct);

    MINT                        requireAct(P1QueAct & rGetAct);

    MINT                        requireJob(P1QueJob & rGetJob);

    MINT                        remainder(void);

    MVOID                       dumpRoll(void);

private:
    MINT                        settle(void);

private:
    sp<P1TaskCtrl>              mspP1TaskCtrl;
    MINT32                      mOpenId;
    MINT32                      mLogLevel;
    MINT32                      mLogLevelI;
    MUINT8                      mBurstNum;
    mutable Mutex               mCollectorLock;
    std::vector< P1QueAct >     mvqActRoll;
};


/******************************************************************************
 *
 ******************************************************************************/
class P1TaskCtrl
    : public virtual android::RefBase
{

    friend class P1TaskCollector;

public:
    P1TaskCtrl(sp<P1NodeImp> spP1NodeImp);

    virtual ~P1TaskCtrl();

public:
    MINT32                      generateId()
                                {
                                    Mutex::Autolock _l(mAccIdLock);
                                    MINT32 ret = mAccId ++;
                                    if (ret == 0 || ret == (MINT32)(-1))
                                        ret = mAccId = P1ACT_ID_FIRST;
                                    return ret;
                                };

public:
    MVOID                       config(void);

    MVOID                       reset(void);

    MBOOL                       acquireAct(P1QueAct & rGetAct,
                                    MINT32 keyId, P1Act actPtr = NULL);

    MBOOL                       registerAct(P1QueAct & rGetAct);

    MBOOL                       releaseAct(P1QueAct & rPutAct);

    MBOOL                       flushAct(void);

    MVOID                       sessionLock(void);

    MVOID                       sessionUnLock(void);

    MVOID                       dumpActPool(void);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Member.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    sp<P1NodeImp>               mspP1NodeImp;
    MINT32                      mOpenId;
    MINT32                      mLogLevel;
    MINT32                      mLogLevelI;
    MUINT8                      mBurstNum;
    MUINT8                      mBatchNum;
    MUINT8                      mTuningGroup;
    MINT32                      mCommonShareNum;
    MUINT8                      mCommonRemainCnt;

protected:
    mutable Mutex               mTaskLock;
    mutable Mutex               mSessionLock;
    std::vector< P1Act >        mvpActPool;

private:
    mutable Mutex               mAccIdLock;
    MINT32                      mAccId;
};



};//namespace NSP1Node
};//namespace v3
};//namespace NSCam

#endif//_MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_TASK_CTRL_H_

