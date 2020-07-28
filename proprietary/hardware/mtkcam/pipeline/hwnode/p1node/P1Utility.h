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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_UTILITY_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_UTILITY_H_

#include "P1Common.h"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace NSCam {
namespace v3 {
namespace NSP1Node {


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef P1_TIMING_CHECK
#undef P1_TIMING_CHECK
#endif
#if (IS_P1_LOGD) // for system use LOGD
#define P1_TIMING_CHECK(str, timeout_ms, type)\
    sp<TimingChecker::Client> TimingCheckerClient =\
    (mpTimingCheckerMgr != NULL) ?\
    (mpTimingCheckerMgr->createClient(str, timeout_ms, type)) :\
    (NULL);
#else
#define P1_TIMING_CHECK(str, timeout_ms, type)  ;
#endif

#ifdef TC_W
#undef TC_W
#endif
#define TC_W TimingChecker::EVENT_TYPE_WARNING
#ifdef TC_E
#undef TC_E
#endif
#define TC_E TimingChecker::EVENT_TYPE_ERROR
#ifdef TC_F
#undef TC_F
#endif
#define TC_F TimingChecker::EVENT_TYPE_FATAL


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/******************************************************************************
 *
 ******************************************************************************/

INormalPipeModule* getNormalPipeModule();

MUINT32 getResizeMaxRatio(MUINT32 imageFormat);

MBOOL calculateCropInfoFull(
    MUINT32 pixelMode,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect,
    MRect& resultSrcRect,
    MSize& resultDstSize,
    MINT32 mLogLevelI = 0
);


MBOOL calculateCropInfoResizer(
    MUINT32 pixelMode,
    MUINT32 imageFormat,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect,
    MRect& resultSrcRect,
    MSize& resultDstSize,
    MINT32 mLogLevelI = 0
);


MBOOL verifySizeResizer(
    MUINT32 pixelMode,
    MUINT32 imageFormat,
    MSize const& sensorSize,
    MSize const& streamBufSize,
    MSize const& queryBufSize,
    MSize& resultBufSize,
    MINT32 mLogLevelI
);


void
queryRollingSkew(
    MUINT const openId,
    MUINT32 const sensorMode,
    MUINT32 & nsRolling,
    MINT32 mLogLevelI
);

void
queryStartExpTs(
    MUINT const openId,
    MUINT32 const sensorMode,
    MINT64 const & nsExpDuration,
    MINT64 const & nsStartFrm,
    MINT64 & nsStartExp,
    MINT32 mLogLevelI
);

void generateMetaInfoStr(
    IMetadata::IEntry const & entry,
    android::String8 & string
);

void logMeta(
    MINT32 option,
    IMetadata const * pMeta,
    char const * pInfo,
    MUINT32 tag = 0x0
);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AAAResult {
    protected:
        struct info{
            sp<IPipelineFrame> spFrame;
            IMetadata          resultVal;
            MUINT32            mFlag;
            info()
                : spFrame(0)
                , resultVal()
                , mFlag(0)
                {}
        };

    protected:
        enum KeyType {
            KeyType_StrobeState = 1,
            KeyType_Rest        = 2, //1,2,4,8,...
        };

    protected:
        mutable Mutex              mLock;
        KeyedVector<MUINT32, info> mData; //key: magicnum, val: info
        MUINT32                    mAllKey;

    public:

        AAAResult()
            : mLock()
            , mData()
            , mAllKey(KeyType_Rest)
            //, mAllKey(KeyType_StrobeState|KeyType_Rest)
            {}

        void add(MUINT32 magicNum, MUINT32 key, MUINT32 val)
        {
             Mutex::Autolock lock(mLock);
             if(key != MTK_FLASH_STATE) {
                 //unSupported
                 return;
             }

             IMetadata::IEntry entry(MTK_FLASH_STATE);
             entry.push_back(val, Type2Type< MUINT8 >()); //{MTK_FLASH_STATE, MUINT8}
             ssize_t i = mData.indexOfKey(magicNum);
             if(i < 0) {
                 info data;
                 data.resultVal.update(MTK_FLASH_STATE, entry);

    data.mFlag |= KeyType_StrobeState;
                 mData.add(magicNum, data);
             } else {
                 info& data = mData.editValueFor(magicNum);
                 data.resultVal.update(MTK_FLASH_STATE, entry);

    data.mFlag |= KeyType_StrobeState;
             }
        }

        void add(MUINT32 magicNum, sp<IPipelineFrame> pframe, IMetadata &rVal)
        {
             Mutex::Autolock lock(mLock);
             ssize_t i = mData.indexOfKey(magicNum);
             if(i < 0) {
                 info data;
                 data.spFrame = pframe;
                 data.resultVal = rVal;

data.mFlag |= KeyType_Rest;
                 mData.add(magicNum, data);
             } else {
                 info& data = mData.editValueFor(magicNum);
                 data.spFrame = pframe;
                 data.resultVal += rVal;
                 data.mFlag |= KeyType_Rest;
             }
        }

        const info& valueFor(const MUINT32& magicNum) const {
            return mData.valueFor(magicNum);
        }

        bool isCompleted(MUINT32 magicNum) {
            Mutex::Autolock lock(mLock);
            return (mData.valueFor(magicNum).mFlag & mAllKey) == mAllKey;
        }

        void removeItem(MUINT32 key) {
            Mutex::Autolock lock(mLock);
            mData.removeItem(key);
        }

        void clear() {
            debug();
            Mutex::Autolock lock(mLock);
            mData.clear();
        }

        void debug() {
            Mutex::Autolock lock(mLock);
            for(size_t i = 0; i < mData.size(); i++) {
                MY_LOGW_IF((mData.valueAt(i).mFlag & KeyType_StrobeState) == 0,
                           "No strobe result: (%d)", mData.keyAt(i));
                MY_LOGW_IF((mData.valueAt(i).mFlag & KeyType_Rest) == 0,
                           "No rest result: (%d)", mData.keyAt(i));
            }
        }
};
#endif
#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Storage {

    protected:
        typedef DefaultKeyedVector<MINTPTR, sp<IImageBuffer> >  MapType;
        MapType                    mvStorageQ;
        mutable Mutex              mStorageLock;
        MINT32                     mLogEnable;
    public:
                                   Storage()
                                       : mvStorageQ()
                                       , mStorageLock()
                                       , mLogEnable(0)
                                       {}

        virtual                   ~Storage(){};

        void                       init(MINT32 logEnable)
                                   {
                                       mvStorageQ.clear();
                                       mLogEnable = logEnable;
                                   }

        void                       uninit()
                                   {
                                       mvStorageQ.clear();
                                   }

        void                       enque(sp<IImageStreamBuffer> const& key, sp<IImageBuffer> &value) {
                                       Mutex::Autolock lock(mStorageLock);
                                       MY_LOGD_IF(mLogEnable, "Storage-enque::(key)0x%x/(val)0x%x",
                                           key.get(), value.get());
                                       MY_LOGD_IF(mLogEnable, "Info::(val-pa)0x%x/%d/%d/%d/%d/%d",
                                        value->getBufPA(0),value->getImgSize().w, value->getImgSize().h,
                                        value->getBufStridesInBytes(0), value->getBufSizeInBytes(0), value->getPlaneCount());

                                       mvStorageQ.add(reinterpret_cast<MINTPTR>(key.get()), value);
                                   };


        sp<IImageBuffer>           deque(MINTPTR key) {
                                       Mutex::Autolock lock(mStorageLock);
                                       sp<IImageBuffer> pframe = mvStorageQ.valueFor(key);
                                       if (pframe != NULL)
                                       {
                                           mvStorageQ.removeItem(key); //should un-mark
                                           MY_LOGD_IF(mLogEnable, "Storage-deque::(key)0x%x/(val)0x%x",
                                            key, pframe.get());
                                           MY_LOGD_IF(mLogEnable, "(val-pa)0x%x",
                                            pframe->getBufPA(0));
                                           return pframe;
                                       }
                                       return NULL;
                                   }
        sp<IImageBuffer>           query(MINTPTR key) {
                                       Mutex::Autolock lock(mStorageLock);
                                       sp<IImageBuffer> pframe = mvStorageQ.valueFor(key);
                                       if (pframe != NULL)
                                       {
                                           MY_LOGD_IF(mLogEnable, "Storage-deque::(key)0x%x/(val)0x%x",
                                            key, pframe.get());
                                           MY_LOGD_IF(mLogEnable, "Info::(val-pa)0x%x",
                                            pframe->getBufPA(0));
                                           return pframe;
                                       }
                                       return NULL;
                                   }
};
#endif

#if 1
/******************************************************************************
 *
 ******************************************************************************/
class StuffBufferPool
    : public virtual android::RefBase
{

    enum BUF_STATE
    {
        BUF_STATE_RELEASED  = 0,
        BUF_STATE_ACQUIRED
    };

    struct BufNote {
        public:
                            BufNote()
                                : msName("")
                                , mState(BUF_STATE_RELEASED)
                            {};
                            BufNote(android::String8 name, BUF_STATE state)
                                : msName(name)
                                , mState(state)
                            {};
            virtual         ~BufNote()
                            {};

            android::String8    msName;
            BUF_STATE           mState;
    };

public:
                        StuffBufferPool(MBOOL isSecure = MFALSE,
                            SecType secType = SecType::mem_normal)
                            : mOpenId(-1)
                            , mLogLevel(0)
                            , mLogLevelI(0)
                            , msName("")
                            , mFormat(0)
                            , mSize(0, 0)
                            , mStride0(0)
                            , mStride1(0)
                            , mStride2(0)
                            , mPlaneCnt(0)
                            , mUsage(0)
                            , mSerialNum(0)
                            , mWaterMark(P1NODE_STUFF_BUFFER_WATER_MARK)
                            , mMaxAmount(P1NODE_STUFF_BUFFER_MAX_AMOUNT)
                            , mbSecure(isSecure)
                            , mSecType(secType)
                        {
                            MY_LOGD2("+++");
                            mUsage = (GRALLOC_USAGE_SW_READ_OFTEN |
                                        GRALLOC_USAGE_HW_CAMERA_READ |
                                        GRALLOC_USAGE_HW_CAMERA_WRITE);
                            mvInfoMap.clear();
                            MY_LOGD2("---");
                        };

                        StuffBufferPool(
                            char const * szName,
                            MINT32 format,
                            MSize size,
                            MUINT32 stride0,
                            MUINT32 stride1,
                            MUINT32 stride2,
                            MUINT8 amountBasis, // the basis amount
                            MUINT8 amountMax,   // the max amount
                            MUINT8 multiple,    // for burst mode
                            MBOOL writable,     // for SW write
                            MINT32  id,
                            MINT32  log,
                            MINT32  logi,
                            MBOOL   isSecure = MFALSE,
                            SecType secType = SecType::mem_normal
                        )
                            : mOpenId(id)
                            , mLogLevel(log)
                            , mLogLevelI(logi)
                            , msName(szName)
                            , mFormat(format)
                            , mSize(size)
                            , mStride0(stride0)
                            , mStride1(stride1)
                            , mStride2(stride2)
                            , mPlaneCnt(0)
                            , mUsage(0)
                            , mSerialNum(0)
                            , mWaterMark(P1NODE_STUFF_BUFFER_WATER_MARK)
                            , mMaxAmount(P1NODE_STUFF_BUFFER_MAX_AMOUNT)
                            , mbSecure(isSecure)
                            , mSecType(secType)
                        {
                            MY_LOGD2("+++");
                            //
                            if (mStride2 > 0) {
                                if (mStride1 > 0 && mStride0 > 0) {
                                    mPlaneCnt = 3;
                                }
                            } else if (mStride1 > 0) {
                                if (mStride0 > 0) {
                                    mPlaneCnt = 2;
                                }
                            } else if (mStride0 > 0) {
                                mPlaneCnt = 1;
                            }
                            if (mPlaneCnt == 0) {
                                MY_LOGW("[%s] stride invalid (%d.%d.%d)",
                                    msName.string(),
                                    mStride0, mStride1, mStride2);
                            }
                            //
                            if (amountBasis > 0) {
                                mWaterMark = amountBasis;
                            }
                            if (amountMax > 0) {
                                mMaxAmount = amountMax;
                            }
                            //
                            if (multiple > 0) {
                                mWaterMark *= multiple;
                                mMaxAmount *= multiple;
                            }
                            if (mMaxAmount < mWaterMark) {
                                mMaxAmount = mWaterMark;
                            }
                            //
                            mUsage = (GRALLOC_USAGE_SW_READ_OFTEN |
                                        GRALLOC_USAGE_HW_CAMERA_READ |
                                        GRALLOC_USAGE_HW_CAMERA_WRITE);
                            if(writable) {
                                mUsage |= GRALLOC_USAGE_SW_WRITE_OFTEN;
                            }
                            MY_LOGI1("[%s] "
                                "[0x%x-%dx%d-%d.%d.%d (%d:%d) *%d +%d] (%d:%d) "
                                "(0x%x)", szName, format, size.w, size.h,
                                stride0, stride1, stride2,
                                amountBasis, amountMax, multiple, writable,
                                mWaterMark, mMaxAmount, mUsage);
                            mvInfoMap.clear();
                            MY_LOGD2("---");
                        };

    virtual             ~StuffBufferPool()
                        {
                            MY_LOGD2("+++");
                            MY_LOGI2("InfoMap.size(%zu)", mvInfoMap.size());
                            while (mvInfoMap.size() > 0) {
                                destroyBuffer(0); // it remove one of mvInfoMap
                            }
                            mvInfoMap.clear();
                            MY_LOGI2(
                                "[%s] 0x%x-%dx%d-%d.%d.%d", msName.string(),
                                mFormat, mSize.w, mSize.h,
                                mStride0, mStride1, mStride2);
                            MY_LOGD2("---");
                        };

    MBOOL               compareLayout(
                            MINT32 format,
                            MSize size,
                            MUINT32 stride0,
                            MUINT32 stride1,
                            MUINT32 stride2
                        );

    MERROR              acquireBuffer(
                            sp<IImageBuffer> & imageBuffer
                        );

    MERROR              releaseBuffer(
                            sp<IImageBuffer> & imageBuffer
                        );

    MERROR              createBuffer(
                            sp<IImageBuffer> & imageBuffer
                        );

    MERROR              destroyBuffer(
                            sp<IImageBuffer> & imageBuffer
                        );

    MERROR              destroyBuffer(
                            size_t index
                        );

private:
    MINT32              mOpenId;
    MINT32              mLogLevel;
    MINT32              mLogLevelI;
    android::String8    msName;
    MINT32              mFormat;
    MSize               mSize;
    MUINT32             mStride0;
    MUINT32             mStride1;
    MUINT32             mStride2;
    MUINT8              mPlaneCnt;
    MUINT               mUsage;
    MUINT32             mSerialNum;
    // it will destroy buffer while releasing, if pool_size > WaterMark
    MUINT32             mWaterMark;
    // it will not create buffer while acquiring, if pool_size >= MaxAmount
    MUINT32             mMaxAmount;
    DefaultKeyedVector< sp<IImageBuffer>, BufNote >
                        mvInfoMap;
    MBOOL               mbSecure;
    SecType             mSecType;
};


/******************************************************************************
 *
 ******************************************************************************/
class StuffBufferManager {

private:
    class InfoSet {
        public:
                        InfoSet()
                            : mOpenId(-1)
                            , mLogLevel(0)
                            , mLogLevelI(0)
                            , mFormat((MINT32)eImgFmt_UNKNOWN)
                            , mSize()
                            , mvStride()
                        {
                            MY_LOGD2("+++");
                            mvStride.clear();
                            mvStride.reserve(P1NODE_IMG_BUF_PLANE_CNT_MAX);
                            MY_LOGD2("---");
                        };

                        InfoSet(MINT32 id, MINT32 log, MINT32 logi)
                            : mOpenId(id)
                            , mLogLevel(log)
                            , mLogLevelI(logi)
                            , mFormat((MINT32)eImgFmt_UNKNOWN)
                            , mSize()
                            , mvStride()
                        {
                            MY_LOGD2("+++");
                            mvStride.clear();
                            mvStride.reserve(P1NODE_IMG_BUF_PLANE_CNT_MAX);
                            MY_LOGD2("---");
                        };

            virtual     ~InfoSet()
                        {
                            MY_LOGD2("+++");
                            mvStride.clear();
                            MY_LOGD2("---");
                        };

        public:
            MINT32      mOpenId;
            MINT32      mLogLevel;
            MINT32      mLogLevelI;
            MINT32      mFormat;
            MSize       mSize;
            std::vector< MUINT32 >
                        mvStride;
    };

public:
                        StuffBufferManager()
                            : mOpenId(-1)
                            , mLogLevel(0)
                            , mLogLevelI(0)
                            , mLock()
                            , mvPoolSet()
                        {
                            MY_LOGD2("+++");
                            mvPoolSet.clear();
                            mvPoolSet.reserve(32);
                            mvInfoSet.clear();
                            mvInfoSet.reserve(32);
                            MY_LOGD2("---");
                        };

                        StuffBufferManager(MINT32 id, MINT32 log, MINT32 logi)
                            : mOpenId(id)
                            , mLogLevel(log)
                            , mLogLevelI(logi)
                            , mLock()
                        {
                            MY_LOGD2("+++");
                            mvPoolSet.clear();
                            mvPoolSet.reserve(32);
                            mvInfoSet.clear();
                            mvInfoSet.reserve(32);
                            MY_LOGD2("---");
                        };

    virtual             ~StuffBufferManager()
                        {
                            MY_LOGD2("+++");
                            MY_LOGD2("InfoSet.size(%zu)", mvInfoSet.size());
                            mvInfoSet.clear();
                            MY_LOGD2("PoolSet.size(%zu)", mvPoolSet.size());
                            mvPoolSet.clear();
                            MY_LOGD2("---");
                        };

    void                setLog(MINT32 id, MINT32 log, MINT32 logi)
                        {
                            Mutex::Autolock _l(mLock);
                            mOpenId = id;
                            mLogLevel = log;
                            mLogLevelI = logi;
                            MY_LOGD2(
                                "StuffBufferManager id(%d) log(%d,%d)",
                                id, log, logi);
                        }

    MERROR              acquireStoreBuffer(
                            sp<IImageBuffer> & imageBuffer,
                            char const * szName,
                            MINT32 format,
                            MSize size,
                            std::vector<MUINT32> & vStride,
                            MUINT8 amountBasis = P1NODE_STUFF_BUFFER_WATER_MARK,
                            MUINT8 amountMax = P1NODE_STUFF_BUFFER_MAX_AMOUNT,
                            MUINT8 multiple = 1,    // for burst mode
                            MBOOL writable = MFALSE, // for SW write
                            MBOOL isSecure = MFALSE,
                            SecType secType = SecType::mem_normal
                        );

    MERROR              releaseStoreBuffer(
                            sp<IImageBuffer> & imageBuffer
                        );

    MERROR              collectBufferInfo(
                            MUINT32 pixelMode,
                            MBOOL isFull,
                            MINT32 format,
                            MSize size,
                            std::vector< MUINT32 > & stride
                        );

private:
    MINT32              mOpenId;
    MINT32              mLogLevel;
    MINT32              mLogLevelI;
    mutable Mutex       mLock;
    std::vector< sp<StuffBufferPool> >
                        mvPoolSet;
    std::vector< InfoSet >
                        mvInfoSet;
};
#endif

#if 1
/******************************************************************************
 *
 ******************************************************************************/
class TimingChecker
    : public virtual android::RefBase
{
    public:
    enum EVENT_TYPE
    {
        EVENT_TYPE_NONE         = 0,
        EVENT_TYPE_WARNING,
        EVENT_TYPE_ERROR,
        EVENT_TYPE_FATAL
    };
    //
    public:
    class Client
        : public virtual android::RefBase
    {
        public:
                            Client(char const * str, MUINT32 uTimeoutMs,
                                EVENT_TYPE eType)
                                : mLock()
                                , mOpenId(-1)
                                , mLogLevel(0)
                                , mLogLevelI(0)
                                , mStr(str)
                                , mTimeInvMs(uTimeoutMs)
                                , mBeginTsNs(0)
                                , mEndTsNs(0)
                                , mType(eType)
                            {
                                mBeginTsNs = ::systemTime();
                                mEndTsNs = mBeginTsNs +
                                    (ONE_MS_TO_NS * uTimeoutMs);
                                setLog(mOpenId, mLogLevel, mLogLevelI);
                                dump("TC_Client::CTR");
                            };
            virtual         ~Client()
                            {
                                dump("TC_Client::DTR");
                            };
        //
        public:
            MUINT32         getTimeInterval(void)
                            {
                                Mutex::Autolock _l(mLock);
                                return mTimeInvMs;
                            };
            nsecs_t         getTimeStamp(void)
                            {
                                Mutex::Autolock _l(mLock);
                                return mEndTsNs;
                            };
            void            setLog(MINT32 id, MINT32 log, MINT32 logi)
                            {
                                Mutex::Autolock _l(mLock);
                                mOpenId = id;
                                mLogLevel = log;
                                mLogLevelI = logi;
                                MY_LOGD2(
                                    "TimingChecker::Client id(%d) log(%d,%d)",
                                    id, log, logi);
                                return;
                            };
        //
        public:
            void            dump(char const * tag = NULL);
            void            action(void);
        //
        private:
            void            onLastStrongRef(const void* /*id*/);
        //
        private:
            mutable Mutex   mLock;
            MINT32          mOpenId;
            MINT32          mLogLevel;
            MINT32          mLogLevelI;
            android::String8
                            mStr;
            MUINT32         mTimeInvMs; // TimeInterval in msec
            nsecs_t         mBeginTsNs; // Begin TimeStamp in nsec
            nsecs_t         mEndTsNs; // End TimeStamp in nsec
            EVENT_TYPE      mType;
    };
    //
    private:
    class Record {
        public:
                            Record(nsecs_t ns, wp<TimingChecker::Client> pc)
                            : mTimeMarkNs(ns)
                            , mpClient(pc)
                            {
                            };
            virtual         ~Record()
                            {
                            };
            nsecs_t         mTimeMarkNs;
            wp<TimingChecker::Client>
                            mpClient;
    };
    //
    typedef TimingChecker::Record*  RecPtr;
    //
    private:
    class RecCmp {
        public:
                            RecCmp()
                            {
                            };
            virtual         ~RecCmp()
                            {
                            };
            MBOOL           operator()
                            (const RecPtr & rpL, const RecPtr & rpR)
                            {
                                return (rpL->mTimeMarkNs > rpR->mTimeMarkNs);
                            };
    };
    //
    private:
    class RecStore {
        public:
                            RecStore()
                            : mHeap()
                            {
                            };
            virtual         ~RecStore()
                            {
                            };
            size_t          size(void);

            MBOOL           isEmpty(void);

            MBOOL           addRec(RecPtr rp);

            RecPtr const &  getMin(void);

            void            delMin(void);

        //
        private:
            void            dump(char const * tag = NULL);
        //
        private:
            std::priority_queue< RecPtr, std::vector<RecPtr>, RecCmp >
                            mHeap;
    };
    //
    public:
                        TimingChecker(
                            MINT32 nOpenId,
                            MINT32 nLogLevel,
                            MINT32 nLogLevelI
                        )
                        : mLock()
                        , mOpenId(nOpenId)
                        , mLogLevel(nLogLevel)
                        , mLogLevelI(nLogLevelI)
                        , mClientCond()
                        , mWakeTiming(0)
                        , mExitPending(MFALSE)
                        , mRunning(MFALSE)
                        , mExitedCond()
                        , mEnterCond()
                        , mData()
                        {
                            //MY_LOGD0("+++");
                            //MY_LOGD0("---");
                        };

        virtual         ~TimingChecker()
                        {
                            //MY_LOGD0("+++");
                            doRequestExit();
                            //MY_LOGD0("---");
                        };
    //
    public:
        MBOOL           doThreadLoop(void);

        void            doRequestExit(void);

        void            doWaitReady(void);
    //
    public:
        sp<TimingChecker::Client>
                        createClient(char const * str, MUINT32 uTimeoutMs,
                            EVENT_TYPE eType);
    //
    private:
        nsecs_t         checkList(nsecs_t time);
    //
    private:
        mutable Mutex   mLock;
        MINT32          mOpenId;
        MINT32          mLogLevel;
        MINT32          mLogLevelI;
        Condition       mClientCond;
        nsecs_t         mWakeTiming;
        // for loop control
        MBOOL           mExitPending;
        MBOOL           mRunning;
        Condition       mExitedCond;
        Condition       mEnterCond;
        // for client record stroage
        RecStore        mData;
};
#endif


/******************************************************************************
 *
 ******************************************************************************/
class TimingCheckerMgr
    : public virtual android::RefBase
{
    public:
                        TimingCheckerMgr(MUINT32 factor,
                            MINT32 nOpenId,
                            MINT32 nLogLevel,
                            MINT32 nLogLevelI)
                            : mLock()
                            , mOpenId(nOpenId)
                            , mLogLevel(nLogLevel)
                            , mLogLevelI(nLogLevelI)
                            , mIsEn(MFALSE)
                            , mFactor(factor)
                            , mpTimingChecker(NULL)
                        {
                            //MY_LOGD0("+++");
                            #if (IS_P1_LOGD)
                            mpTimingChecker = new TimingChecker(
                                mOpenId, mLogLevel, mLogLevelI);
                            #endif
                            //MY_LOGD0("---");
                        };
        virtual         ~TimingCheckerMgr()
                        {
                            //MY_LOGD0("+++");
                            //MY_LOGD0("---");
                        };
    public:
        void            setEnable(MBOOL en);

        MBOOL           getEnable(void);

        void            waitReady(void);

        void            onCheck(void);

        sp<TimingChecker::Client>
                        createClient(char const * str, MUINT32 uTimeoutMs,
                            TimingChecker::EVENT_TYPE eType =
                            TimingChecker::EVENT_TYPE_WARNING);
        //
    private:
        mutable Mutex   mLock;
        MINT32          mOpenId;
        MINT32          mLogLevel;
        MINT32          mLogLevelI;
        MBOOL           mIsEn;
        MUINT32         mFactor;
        sp<TimingChecker>
                        mpTimingChecker;
};


/******************************************************************************
 *
 ******************************************************************************/
class LongExposureStatus
{
    #define P1_LONG_EXP_TIME_TH (500 * 1000000) //(1ms = 1000000ns)
    public:
                        LongExposureStatus()
                            : mLock()
                            , mOpenId(-1)
                            , mLogLevel(0)
                            , mLogLevelI(0)
                            , mThreshold(P1_LONG_EXP_TIME_TH)
                            , mRunning(MFALSE)
                            , mvSet()
                        {
                            //MY_LOGD0("+++");

                            #if 0 // for Long Expousre IT
                            #warning "[FIXME] force to change P1 LE threshold"
                            {
                                MUINT32 thd_ms =
                                    ::property_get_int32("vendor.debug.camera.p1exp",
                                    0);
                                if (thd_ms > 0) {
                                    mThreshold = thd_ms * 1000000;
                                }
                                MY_LOGI0("debug.camera.p1exp = %d - "
                                    "Threshold = %lld", thd_ms, mThreshold);
                            }
                            #endif
                            mvSet.clear();
                            mvSet.reserve(P1NODE_DEF_QUEUE_DEPTH);

                            //MY_LOGD0("---");
                        };

        virtual         ~LongExposureStatus()
                        {
                            //MY_LOGD0("+++");

                            //MY_LOGD0("---");
                        };

        MVOID           config(
                            MINT32 nOpenId,
                            MINT32 nLogLevel,
                            MINT32 nLogLevelI
                        );

        MBOOL           reset(MINT num);

        MBOOL           set(MINT num, MINT64 exp_ns);

        MBOOL           get(void);

    private:
        mutable Mutex   mLock;
        MINT32          mOpenId;
        MINT32          mLogLevel;
        MINT32          mLogLevelI;
        MINT64          mThreshold;
        MBOOL           mRunning;
        std::vector<MINT32>
                        mvSet;
};


/******************************************************************************
 *
 ******************************************************************************/
enum STAGE_DONE
{
    STAGE_DONE_START                = 0,
    STAGE_DONE_INIT_ITEM,
    STAGE_DONE_TOTAL
};

/******************************************************************************
 *
 ******************************************************************************/
class ProcedureStageControl
    : public virtual android::RefBase
{
    class StageNote
        : public virtual android::RefBase
    {
        public:
                            StageNote(MUINT32 uId)
                                : mId(uId)
                                , mLock()
                                , mCond()
                                , mWait(MFALSE)
                                , mDone(MFALSE)
                                , mSuccess(MFALSE)
                            {
                                //MY_LOGI("(%d)", mId);
                            };
            virtual         ~StageNote()
                            {
                                Mutex::Autolock _l(mLock);
                                mDone = MTRUE;
                                if (mWait) {
                                    mCond.broadcast();
                                }
                                mWait = MFALSE;
                                //MY_LOGI("(%d)", mId);
                            };
            //
            MUINT32         mId;
            mutable Mutex   mLock;
            Condition       mCond;
            MBOOL           mWait;
            MBOOL           mDone;
            MBOOL           mSuccess;
    };

    public:
                        ProcedureStageControl(
                            MUINT32 nStageAmount,
                            MINT32 nOpenId,
                            MINT32 nLogLevel,
                            MINT32 nLogLevelI,
                            MINT32 nSysLevel
                        )
                        : mOpenId(nOpenId)
                        , mLogLevel(nLogLevel)
                        , mLogLevelI(nLogLevelI)
                        , mSysLevel(nSysLevel)
                        , mvpStage()
                        {
                            //MY_LOGD0("+++");
                            mvpStage.clear();
                            mvpStage.reserve(nStageAmount);
                            for (MUINT32 i = 0; i < nStageAmount; i++) {
                                sp<StageNote> p = new StageNote(i);
                                mvpStage.push_back(p);
                            }
                            MY_LOGI0("StageNum(%zu)", mvpStage.size());
                            //MY_LOGD0("---");
                        };

        virtual         ~ProcedureStageControl()
                        {
                            //MY_LOGD0("+++");
                            for (MUINT32 i = 0; i < mvpStage.size(); i++) {
                                sp<StageNote> p = mvpStage.at(i);
                                p = NULL;
                            }
                            mvpStage.clear();
                            MY_LOGD2("StageNum(%zu)", mvpStage.size());
                            //MY_LOGD0("---");
                        };

        MBOOL           reset(void);

        MBOOL           wait(MUINT32 eStage, MBOOL& rSuccess);

        MBOOL           done(MUINT32 eStage, MBOOL bSuccess);

    private:
        MINT32          mOpenId;
        MINT32          mLogLevel;
        MINT32          mLogLevelI;
        MINT32          mSysLevel;
        std::vector< sp<StageNote> >
                        mvpStage;
};


/******************************************************************************
 *
 ******************************************************************************/
class ConcurrenceControl
    : public virtual android::RefBase
{
    public:
                        ConcurrenceControl(
                            MINT32 nOpenId,
                            MINT32 nLogLevel,
                            MINT32 nLogLevelI,
                            MINT32 nSysLevel)
                        : mLock()
                        , mOpenId(nOpenId)
                        , mLogLevel(nLogLevel)
                        , mLogLevelI(nLogLevelI)
                        , mSysLevel(nSysLevel)
                        , mIsAssistUsing(MFALSE)
                        , mpBufInfo(NULL)
                        , mpStageCtrl(NULL)
                        {
                            //MY_LOGD0("+++");
                            mpStageCtrl =
                                new ProcedureStageControl(STAGE_DONE_TOTAL,
                                    mOpenId, mLogLevel, mLogLevelI, mSysLevel);
                            if (mpStageCtrl == NULL) {
                                MY_LOGE("ProcedureStageControl create fail");
                            }
                            //MY_LOGD0("---");
                        };

        virtual         ~ConcurrenceControl()
                        {
                            //MY_LOGD0("+++");
                            initBufInfo_clean();
                            if (mpStageCtrl != NULL) {
                                mpStageCtrl = NULL;
                            }
                            //MY_LOGD0("---");
                        };

        MBOOL           initBufInfo_clean(void);

        MBOOL           initBufInfo_get(QBufInfo** ppBufInfo);

        MBOOL           initBufInfo_create(QBufInfo** ppBufInfo);

        void            setAidUsage(MBOOL enable);

        MBOOL           getAidUsage(void);

        void            cleanAidStage(void);

        sp<ProcedureStageControl>
                        getStageCtrl(void);

    private:
        mutable Mutex   mLock;
        MINT32          mOpenId;
        MINT32          mLogLevel;
        MINT32          mLogLevelI;
        MINT32          mSysLevel;
        MBOOL           mIsAssistUsing;
        QBufInfo*       mpBufInfo;
        sp<ProcedureStageControl>
                        mpStageCtrl;
};


/******************************************************************************
 *
 ******************************************************************************/
class HardwareStateControl
    : public virtual android::RefBase
{
    public:
    enum STATE
    {
        STATE_NORMAL         = 0,
        STATE_SUS_WAIT_NUM,     // received the suspend meta and wait the act magic number assign
        STATE_SUS_WAIT_SYNC,    // received the suspend meta and wait the 3A CB to set frame
        //STATE_SUS_WAITING,      // received the suspend meta and wait for the 3A/DRV suspend function execution
        STATE_SUS_READY,        // already called 3A/DRV suspend function
        STATE_SUS_DONE,         // thread was blocked and wait resume
        STATE_RES_WAIT_NUM,     // received the resume meta and wait the act magic number assign
        STATE_RES_WAIT_SYNC,    // called 3A/DRV resume function and wait the 3A CB for EnQ
        STATE_RES_WAIT_DONE,    // received the 3A CB after 3A/DRV resume function and wait the previous frames done
        STATE_MAX
    };

    public:
                        HardwareStateControl()
                        : mLock()
                        , mOpenId(-1)
                        , mLogLevel(0)
                        , mLogLevelI(0)
                        , mSysLevel(P1_SYS_LV_DEFAULT)
                        , mBurstNum(1)
                        , mIsLegacyStandby(MFALSE)
                        , mState(STATE_NORMAL)
                        , mvStoreNum()
                        , mStandbySetNum(0)
                        , mStreamingSetNum(0)
                        , mShutterTimeUs(0)
                        , mRequestPass(MFALSE)
                        , mRequestCond()
                        , mThreadCond()
                        , mpCamIO(NULL)
                        , mp3A(NULL)
                        {
                            //MY_LOGD0("+++");
                            mvStoreNum.clear();
                            mvStoreNum.reserve(P1NODE_DEF_QUEUE_DEPTH);
                            //MY_LOGD0("---");
                        };

        virtual         ~HardwareStateControl()
                        {
                            //MY_LOGD0("+++");
                            mvStoreNum.clear();
                            //MY_LOGD0("---");
                        };

        void            config(
                            MINT32 nOpenId,
                            MINT32 nLogLevel,
                            MINT32 nLogLevelI,
                            MINT32 nSysLevel,
                            MUINT8 nBurstNum,
                            INormalPipe* pCamIO,
                            IHal3A_T* p3A,
                            MBOOL isLegacyStandby
                        );

        MBOOL           isActive(void);

        SENSOR_STATUS_CTRL
                        checkReceiveFrame(IMetadata* pMeta);

        MBOOL           checkReceiveRestreaming(void);

        //void            checkReceiveNode(MINT32 num, MBOOL bSkipEnQ,
        //                    MBOOL & rIsNeedEnQ, MINT32 & rShutterTimeUs);

        void            checkShutterTime(MINT32 & rShutterTimeUs);

        void            checkRestreamingNum(MINT32 num);

        MBOOL           checkCtrlStandby(MINT32 num);

        //MBOOL           checkSetNum(MINT32 num);

        void            checkRequest(void);

        void            checkThreadStandby(void);

        void            checkThreadWeakup(void);

        MBOOL           checkFirstSync(void);

        MBOOL           checkSkipSync(void);

        MBOOL           checkSkipWait(void);

        MBOOL           checkSkipBlock(void);

        MBOOL           checkBufferState(void);

        MBOOL           checkDoneNum(MINT32 num);

        void            checkNotePass(MBOOL pass = MTRUE);

        void            setDropNum(MINT32 num);

        MINT32          getDropNum(void);

        MBOOL           isLegacyStandby(void);

        void            reset(void);

        void            clean(void);

        void            dump(void);

    private:
        mutable Mutex   mLock;
        MINT32          mOpenId;
        MINT32          mLogLevel;
        MINT32          mLogLevelI;
        MINT32          mSysLevel;
        MUINT8          mBurstNum;
        MBOOL           mIsLegacyStandby;
        STATE           mState;
        std::vector<MINT32>
                        mvStoreNum;
        MINT32          mStandbySetNum;
        MINT32          mStreamingSetNum;
        MINT32          mShutterTimeUs;
        MBOOL           mRequestPass;
        Condition       mRequestCond;
        Condition       mThreadCond;
        INormalPipe*    mpCamIO;
        IHal3A_T*       mp3A;

};


/******************************************************************************
 *
 ******************************************************************************/
class FrameNote
{
public:
                    FrameNote(MUINT32 capacity)
                        : mLock()
                        , mLastTv()
                        , mLastTid(0)
                        , mLastNum(P1NODE_FRAME_NOTE_NUM_UNKNOWN)
                        , mSlotCapacity(capacity)
                        , mSlotIndex(0)
                        , mvSlot()
                    {
                        //MY_LOGI("[CTR] Capacity(%d)", mSlotCapacity);
                        mLastTv.tv_sec = 0;
                        mLastTv.tv_usec = 0;
                        U_if (mSlotCapacity == 0) {
                            MY_LOGW("Capacity(%d)", mSlotCapacity);
                            return;
                        } else { // (mSlotCapacity >= 1)
                            mSlotIndex = mSlotCapacity - 1; // mSlotIndex is the position of the last filled data
                        }
                        mvSlot.reserve((size_t)mSlotCapacity);
                        for (MUINT32 i = 0; i < mSlotCapacity; i++) {
                            mvSlot.push_back(P1NODE_FRAME_NOTE_NUM_UNKNOWN);
                        };
                    };

    virtual         ~FrameNote()
                    {
                        //MY_LOGI("[DTR] Capacity(%d)", mSlotCapacity);
                    };

    MVOID           set(MINT32 num);

    MVOID           get(android::String8 * pStr);

private:
    mutable RWLock  mLock;
    struct timeval  mLastTv;
    MUINT32         mLastTid;
    MINT32          mLastNum;
    MUINT32         mSlotCapacity;
    MUINT32         mSlotIndex;
    std::vector< MINT32 >
                    mvSlot;
};


/******************************************************************************
 *
 ******************************************************************************/
class LogInfo
{
public:

    #ifdef TEXT_LEN
    #undef TEXT_LEN
    #endif
    #define TEXT_LEN (128)

    #ifdef NOTE_LEN
    #undef NOTE_LEN
    #endif
    #define NOTE_LEN (32)

    #ifdef PARAM_NUM
    #undef PARAM_NUM
    #endif
    #define PARAM_NUM (4)

    #define CC_AMOUNT_MAX           (64)//(MUINT64)

    #define CC_OP_TIMEOUT           (MUINT64)(0x0000000000000001)
    #define CC_OP_TIMEOUT_ALL       (MUINT64)(0x000000000000FFFF)

    #define CC_WAIT_OVERTIME        (MUINT64)(0x0000000000010000)
    #define CC_WAIT_OVERTIME_ALL    (MUINT64)(0x00000000FFFF0000)

    #define CC_DEDUCE               (MUINT64)(0x0000000100000000)
    #define CC_DEDUCE_ALL           (MUINT64)(0xFFFFFFFF00000000)

    #ifdef HAS
    #undef HAS
    #endif
    #define HAS(type, code) ((mCode & ((MUINT64)(type << code))) > 0)

    #ifdef ADD
    #undef ADD
    #endif
    #define ADD(type, code) do { mCode |= ((MUINT64)(type << code)); } while(0);

    #ifdef DIFF_NOW
    #undef DIFF_NOW
    #endif
    #define DIFF_NOW(CP, DURATION) \
        (mData.mNowTime > (mData.mTime[CP] + DURATION))

    #ifdef CHECK_OP
    #undef CHECK_OP
    #endif
    #define CHECK_OP(BGN, END, CODE) \
        if ((mData.mTime[BGN] > mData.mTime[END]) &&\
            (DIFF_NOW(BGN, P1_GENERAL_OP_TIMEOUT_US))) {\
            ADD(CC_OP_TIMEOUT, CODE);\
        }

    #ifdef CHECK_WAIT
    #undef CHECK_WAIT
    #endif
    #define CHECK_WAIT(RET, REV, CODE) \
        if ((mData.mTime[RET] > mData.mTime[REV]) &&\
            (DIFF_NOW(RET, P1_GENERAL_WAIT_OVERTIME_US))) {\
            ADD(CC_WAIT_OVERTIME, CODE);\
        }

    #ifdef CHECK_STUCK
    #undef CHECK_STUCK
    #endif
    #define CHECK_STUCK(BGN, END, CODE) \
        if ((mData.mTime[BGN] > mData.mTime[END]) &&\
            (DIFF_NOW(BGN, P1_GENERAL_STUCK_JUDGE_US))) {\
            ADD(CC_DEDUCE, CODE);\
        }

    enum InspectType
    {
        IT_COMMON_DUMP = 0,
        IT_PERIODIC_CHECK,
        IT_STOP_NO_REQ_IN_GENERAL, // stop but no request received, in the flow of START_SET_GENERAL
        IT_STOP_NO_REQ_IN_CAPTURE, // stop but no request received, in the flow of START_SET_CAPTURE
        IT_STOP_NO_REQ_IN_REQUEST, // stop but no request received, in the flow of START_SET_REQUEST
        IT_WAIT_CATURE,
        IT_WAIT_AAA_CB,
        IT_BUFFER_EXCEPTION,
        IT_NO_DELIVERY,
        IT_FLUSH_BLOCKING,
        IT_EVT_WAIT_DRAIN_TIMEOUT,
        IT_LAUNCH_STATE_TIMEOUT,
        IT_MAX
    };

    enum CheckPoint
    {
        CP_FIRST = 0,
        CP_REQ_ARRIVE = CP_FIRST,
        CP_REQ_ACCEPT,
        CP_REQ_REV,
        CP_REQ_RET,
        CP_REQ_NOTIFY_BGN,
        CP_REQ_NOTIFY_END,
        CP_CB_SYNC_REV,
        CP_CB_SYNC_RET,
        CP_CB_PROC_REV,//CP_CB_DONE_REV,
        CP_CB_PROC_RET,//CP_CB_DONE_RET,
        CP_START_SET_BGN,
        CP_START_SET_END,
        CP_PRE_SET_BGN,
        CP_PRE_SET_END,
        CP_SET_BGN,
        CP_SET_END,
        CP_BUF_BGN,
        CP_BUF_END,
        CP_ENQ_BGN,
        CP_ENQ_END,
        CP_DEQ_BGN,
        CP_DEQ_END,
        CP_OUT_BGN,
        CP_OUT_END,
        //
        CP_OP_START_BGN,
        CP_OP_START_3A_PWRON_BGN,
        CP_OP_START_3A_PWRON_END,
        CP_OP_START_3A_CFG_BGN,
        CP_OP_START_3A_CFG_END,
        CP_OP_START_3A_START_BGN,
        CP_OP_START_3A_START_END,
        CP_OP_START_DRV_INIT_BGN,
        CP_OP_START_DRV_INIT_END,
        CP_OP_START_DRV_CFG_BGN,
        CP_OP_START_DRV_CFG_END,
        CP_OP_START_DRV_START_BGN,
        CP_OP_START_DRV_START_END,
        CP_OP_START_REQ_WAIT_BGN,
        CP_OP_START_REQ_WAIT_END,
        CP_OP_START_END,
        //
        CP_OP_STOP_BGN,
        CP_OP_STOP_3A_PWROFF_BGN,
        CP_OP_STOP_3A_PWROFF_END,
        CP_OP_STOP_3A_STOPSTT_BGN,
        CP_OP_STOP_3A_STOPSTT_END,
        CP_OP_STOP_3A_STOP_BGN,
        CP_OP_STOP_3A_STOP_END,
        CP_OP_STOP_DRV_UNINIT_BGN,
        CP_OP_STOP_DRV_UNINIT_END,
        CP_OP_STOP_DRV_STOP_BGN,
        CP_OP_STOP_DRV_STOP_END,
        CP_OP_STOP_HW_LOCK_BGN,
        CP_OP_STOP_HW_LOCK_END,
        CP_OP_STOP_END,
        //
        CP_API_FLUSH_BGN,
        CP_API_FLUSH_END,
        CP_API_UNINIT_BGN,
        CP_API_UNINIT_END,
        //
        CP_MAX
    };

    enum StartSet
    {
        START_SET_GENERAL = 0,
        START_SET_CAPTURE,
        START_SET_REQUEST,
        START_SET_READY
    };

    enum CcOpTimeout
    {   // for new item, please replace one of reservation
        CcOpTimeout_StartSet = 0,
        CcOpTimeout_PreSet,
        CcOpTimeout_Set,
        CcOpTimeout_Buf,
        CcOpTimeout_Enq,
        CcOpTimeout_Deq,
        CcOpTimeout_Dispatch,
        CcOpTimeout__Reserve07,
        CcOpTimeout__Reserve08,
        CcOpTimeout__Reserve09,
        CcOpTimeout__Reserve10,
        CcOpTimeout__Reserve11,
        CcOpTimeout__Reserve12,
        CcOpTimeout__Reserve13,
        CcOpTimeout__Reserve14,
        CcOpTimeout__Reserve15,
        CcOpTimeout_MAX// = 16
    };
    static_assert((CcOpTimeout_MAX == 16), "CcOpTimeout_MAX != 16");

    enum CcWaitOvertime
    {   // for new item, please replace one of reservation
        CcWaitOvertime_Request = 0,
        CcWaitOvertime_3aCbSyncDone,
        CcWaitOvertime_3aCbProcFinish,
        CcWaitOvertime__Reserve03,
        CcWaitOvertime__Reserve04,
        CcWaitOvertime__Reserve05,
        CcWaitOvertime__Reserve06,
        CcWaitOvertime__Reserve07,
        CcWaitOvertime__Reserve08,
        CcWaitOvertime__Reserve09,
        CcWaitOvertime__Reserve10,
        CcWaitOvertime__Reserve11,
        CcWaitOvertime__Reserve12,
        CcWaitOvertime__Reserve13,
        CcWaitOvertime__Reserve14,
        CcWaitOvertime__Reserve15,
        CcWaitOvertime_MAX// = 16
    };
    static_assert((CcWaitOvertime_MAX == 16), "CcWaitOvertime_MAX != 16");

    enum CcDeduce
    {   // for new item, please replace one of reservation
        CcDeduce_FwNoRequestAccept = 0,
        CcDeduce_3aNoFirstCbInGeneral,
        CcDeduce_3aNoFirstCbInCapture,
        CcDeduce_3aNoFirstCbInRequest,
        CcDeduce_3aStuckWithSet,
        CcDeduce_3aStuckWithBuf,
        CcDeduce_3aStuckWithEnq,
        CcDeduce_3aLookForCbSyncDone,
        CcDeduce_3aLookForCbProcFinish,
        CcDeduce_DrvDeqDelay,
        CcDeduce_OpStartBlocking,
        CcDeduce_OpStopBlocking,
        CcDeduce_UninitNotCalledAfterFlush,
        CcDeduce__Reserve13,
        CcDeduce__Reserve14,
        CcDeduce__Reserve15,
        CcDeduce__Reserve16,
        CcDeduce__Reserve17,
        CcDeduce__Reserve18,
        CcDeduce__Reserve19,
        CcDeduce__Reserve20,
        CcDeduce__Reserve21,
        CcDeduce__Reserve22,
        CcDeduce__Reserve23,
        CcDeduce__Reserve24,
        CcDeduce__Reserve25,
        CcDeduce__Reserve26,
        CcDeduce__Reserve27,
        CcDeduce__Reserve28,
        CcDeduce__Reserve29,
        CcDeduce__Reserve30,
        CcDeduce__Reserve31,
        CcDeduce_MAX// = 32
    };
    static_assert((CcDeduce_MAX == 32), "CcDeduce_MAX != 32");

    /*
        ClueCodeListTable
        CCLT(A, B, C, D, E...)
        A: the ENUM of ClueCode in each type
        B: the type of ClueCode CC_OP_TIMEOUT/CC_WAIT_OVERTIME/...
        C: the string of ClueCode for message print
        D: the description of ClueCode for message print
        E...: the related note-tag(s) of ClueCode
    */
    #define ClueCodeListTable \
        CCLT(CcOpTimeout_StartSet,                          (CC_OP_TIMEOUT),\
            "CcOpTimeout_StartSet",\
            "",\
            CP_START_SET_BGN, CP_START_SET_END)\
        \
        CCLT(CcOpTimeout_PreSet,                            (CC_OP_TIMEOUT),\
            "CcOpTimeout_PreSet",\
            "",\
            CP_PRE_SET_BGN, CP_PRE_SET_END)\
        \
        CCLT(CcOpTimeout_Set,                               (CC_OP_TIMEOUT),\
            "CcOpTimeout_Set",\
            "",\
            CP_SET_BGN, CP_SET_END)\
        \
        CCLT(CcOpTimeout_Buf,                               (CC_OP_TIMEOUT),\
            "CcOpTimeout_Buf",\
            "",\
            CP_BUF_BGN, CP_BUF_END)\
        \
        CCLT(CcOpTimeout_Enq,                               (CC_OP_TIMEOUT),\
            "CcOpTimeout_Enq",\
            "",\
            CP_ENQ_BGN, CP_ENQ_END)\
        \
        CCLT(CcOpTimeout_Deq,                               (CC_OP_TIMEOUT),\
            "CcOpTimeout_Deq",\
            "",\
            CP_DEQ_BGN, CP_DEQ_END)\
        \
        CCLT(CcOpTimeout_Dispatch,                          (CC_OP_TIMEOUT),\
            "CcOpTimeout_Dispatch",\
            "",\
            CP_OUT_BGN, CP_OUT_END)\
        \
        \
        CCLT(CcWaitOvertime_Request,                        (CC_WAIT_OVERTIME),\
            "CcWaitOvertime_Request",\
            "",\
            CP_REQ_RET)\
        \
        CCLT(CcWaitOvertime_3aCbSyncDone,                   (CC_WAIT_OVERTIME),\
            "CcWaitOvertime_3aCbSyncDone",\
            "",\
            CP_CB_SYNC_RET)\
        \
        CCLT(CcWaitOvertime_3aCbProcFinish,                 (CC_WAIT_OVERTIME),\
            "CcWaitOvertime_3aCbProcFinish",\
            "",\
            CP_CB_PROC_RET)\
        \
        \
        CCLT(CcDeduce_FwNoRequestAccept,                    (CC_DEDUCE),\
            "CcDeduce_FwNoRequestAccept",\
            "waiting for the next acceptable request by queue() from PipelineContext and PipelineModel",\
            CP_REQ_ARRIVE, CP_REQ_ACCEPT, CP_REQ_REV, CP_REQ_RET, CP_REQ_NOTIFY_BGN, CP_REQ_NOTIFY_END)\
        \
        CCLT(CcDeduce_3aNoFirstCbInGeneral,                 (CC_DEDUCE),\
            "CcDeduce_3aNoFirstCbInGeneral",\
            "cannot get the 3A first callback after the first general 3A.set()",\
            CP_START_SET_BGN, CP_START_SET_END, CP_CB_PROC_REV, CP_CB_PROC_RET, CP_CB_SYNC_REV, CP_CB_SYNC_RET)\
        \
        CCLT(CcDeduce_3aNoFirstCbInCapture,                 (CC_DEDUCE),\
            "CcDeduce_3aNoFirstCbInCapture",\
            "cannot get the 3A first callback after 3A.startCapture()",\
            CP_START_SET_BGN, CP_START_SET_END, CP_CB_PROC_REV, CP_CB_PROC_RET, CP_CB_SYNC_REV, CP_CB_SYNC_RET)\
        \
        CCLT(CcDeduce_3aNoFirstCbInRequest,                 (CC_DEDUCE),\
            "CcDeduce_3aNoFirstCbInRequest",\
            "cannot get the 3A first callback after 3A.startRequestQ()",\
            CP_START_SET_BGN, CP_START_SET_END, CP_CB_PROC_REV, CP_CB_PROC_RET, CP_CB_SYNC_REV, CP_CB_SYNC_RET)\
        \
        CCLT(CcDeduce_3aStuckWithSet,                       (CC_DEDUCE),\
            "CcDeduce_3aStuckWithSet",\
            "the 3A_CB_eID_NOTIFY_VSYNC_DONE is stuck with 3A.set() function",\
            CP_SET_BGN)\
        \
        CCLT(CcDeduce_3aStuckWithBuf,                       (CC_DEDUCE),\
            "CcDeduce_3aStuckWithBuf",\
            "the 3A_CB_eID_NOTIFY_3APROC_FINISH is stuck with buffer acquiring",\
            CP_BUF_BGN)\
        \
        CCLT(CcDeduce_3aStuckWithEnq,                       (CC_DEDUCE),\
            "CcDeduce_3aStuckWithEnq",\
            "the 3A_CB_eID_NOTIFY_3APROC_FINISH is stuck with DRV.enque() function",\
            CP_ENQ_BGN)\
        \
        CCLT(CcDeduce_3aLookForCbSyncDone,                  (CC_DEDUCE),\
            "CcDeduce_3aLookForCbSyncDone",\
            "looking for the next 3A_CB_eID_NOTIFY_VSYNC_DONE",\
            CP_CB_SYNC_REV, CP_CB_SYNC_RET, CP_CB_PROC_REV, CP_CB_PROC_RET)\
        \
        CCLT(CcDeduce_3aLookForCbProcFinish,                (CC_DEDUCE),\
            "CcDeduce_3aLookForCbProcFinish",\
            "looking for the next 3A_CB_eID_NOTIFY_3APROC_FINISH",\
            CP_CB_PROC_REV, CP_CB_PROC_RET, CP_CB_SYNC_REV, CP_SET_BGN, CP_SET_END, CP_CB_SYNC_RET)\
        \
        CCLT(CcDeduce_DrvDeqDelay,                          (CC_DEDUCE),\
            "CcDeduce_DrvDeqDelay",\
            "the DRV.deque() function return delay",\
            CP_DEQ_END, CP_ENQ_BGN, CP_ENQ_END, CP_DEQ_BGN)\
        \
        CCLT(CcDeduce_OpStartBlocking,                      (CC_DEDUCE),\
            "CcDeduce_OpStartBlocking",\
            "the operation of HW-Start flow is blocking in somewhere",\
            CP_OP_START_BGN,\
            CP_OP_START_3A_PWRON_BGN,\
            CP_OP_START_3A_PWRON_END,\
            CP_OP_START_3A_CFG_BGN,\
            CP_OP_START_3A_CFG_END,\
            CP_OP_START_3A_START_BGN,\
            CP_OP_START_3A_START_END,\
            CP_OP_START_DRV_INIT_BGN,\
            CP_OP_START_DRV_INIT_END,\
            CP_OP_START_DRV_CFG_BGN,\
            CP_OP_START_DRV_CFG_END,\
            CP_OP_START_DRV_START_BGN,\
            CP_OP_START_DRV_START_END,\
            CP_OP_START_REQ_WAIT_BGN,\
            CP_OP_START_REQ_WAIT_END,\
            CP_START_SET_BGN,\
            CP_START_SET_END,\
            CP_OP_START_END)\
        \
        CCLT(CcDeduce_OpStopBlocking,                       (CC_DEDUCE),\
            "CcDeduce_OpStopBlocking",\
            "the operation of HW-Stop flow is blocking in somewhere",\
            CP_OP_STOP_BGN,\
            CP_OP_STOP_3A_PWROFF_BGN,\
            CP_OP_STOP_3A_PWROFF_END,\
            CP_OP_STOP_3A_STOPSTT_BGN,\
            CP_OP_STOP_3A_STOPSTT_END,\
            CP_OP_STOP_3A_STOP_BGN,\
            CP_OP_STOP_3A_STOP_END,\
            CP_OP_STOP_DRV_UNINIT_BGN,\
            CP_OP_STOP_DRV_UNINIT_END,\
            CP_OP_STOP_DRV_STOP_BGN,\
            CP_OP_STOP_DRV_STOP_END,\
            CP_OP_STOP_HW_LOCK_BGN,\
            CP_OP_STOP_HW_LOCK_END,\
            CP_OP_STOP_END)\
        \
        CCLT(CcDeduce_UninitNotCalledAfterFlush,            (CC_DEDUCE),\
            "CcDeduce_UninitNotCalledAfterFlush",\
            "the API function uninit() is not called after flush() done and the user also did not queue() acceptable request again",\
            CP_API_FLUSH_BGN, CP_API_FLUSH_END,\
            CP_API_UNINIT_BGN, CP_API_UNINIT_END,\
            CP_REQ_ARRIVE, CP_REQ_ACCEPT, CP_REQ_REV, CP_REQ_RET)\
        \
        \
    //
    //
    #ifdef CC_ENUM
    #undef CC_ENUM
    #endif
    #define CC_ENUM(code) e_##code
    //
    #ifdef CCLT
    #undef CCLT
    #endif
    #define CCLT(code, type, name, info, ...) CC_ENUM(code) = (MUINT64)(type << code),
    enum ClueCode
    {
        CC_NONE                             = (MUINT64)(0x0000000000000000),
        //
        ClueCodeListTable
        //
        CC_ALL                              = (MUINT64)(0xFFFFFFFFFFFFFFFF)
    };
    #undef CCLT

public:
    class AutoMemo {
    public:
                AutoMemo(LogInfo& logInfo,
                    LogInfo::CheckPoint cpEnter,
                    LogInfo::CheckPoint cpExit,
                    MINT64 param0 = 0,
                    MINT64 param1 = 0,
                    MINT64 param2 = 0,
                    MINT64 param3 = 0)
                    : mLogInfo(logInfo)
                    , mCpEnter(cpEnter)
                    , mCpExit(cpExit)
                    , mP0(param0)
                    , mP1(param1)
                    , mP2(param2)
                    , mP3(param3)
                    {
                        mLogInfo.setMemo(mCpEnter, mP0, mP1, mP2, mP3);
                    };
        virtual ~AutoMemo()
                    {
                        mLogInfo.setMemo(mCpExit, mP0, mP1, mP2, mP3);
                    };
    private:
        LogInfo & mLogInfo;
        LogInfo::CheckPoint mCpEnter;
        LogInfo::CheckPoint mCpExit;
        MINT64 mP0;
        MINT64 mP1;
        MINT64 mP2;
        MINT64 mP3;
    };

private:
    struct Slot {
    public:
                        Slot()
                        : mLock()
                        , mTid(0)
                        , mTv()
                        {
                            clear();
                        };

        virtual         ~Slot()
                        {
                            //clear();
                        };

        void            clear(void)
                        {
                            mTid = 0;
                            mTv.tv_sec = 0;
                            mTv.tv_usec = 0;
                            for (int i = 0; i < PARAM_NUM; i++) {
                                mParam[i] = 0;
                            }
                        };

        mutable RWLock  mLock;
        MUINT32         mTid;
        struct timeval  mTv;
        MINT64          mParam[PARAM_NUM];
    };

    struct Text {
        char main[TEXT_LEN];
    };

    struct Note {
        LogInfo::CheckPoint idx;
        char main[NOTE_LEN];
        char sub[PARAM_NUM][NOTE_LEN];
        /*
        char sub1[32];
        char sub2[32];
        char sub3[32];
        */
    };

    class Data {
    public:
        struct timeval  mTv[CP_MAX];
        MINT64          mTime[CP_MAX];
        MUINT32         mTid[CP_MAX];
        //
        MBOOL           mReady;
        struct timeval  mNowTv;
        MINT64          mNowTime;
        MUINT32         mNowTid;
        //
        MINT32          mCbSyncType;
        MINT32          mCbProcType;
        //
        MINT32          mStartSetType;
        MINT32          mStartSetMn;
        //
        MINT32          mPreSetKey;
        //
        MINT32          mSetFn;
        //
        MINT32          mSetMn;
        MINT32          mEnqMn;
        MINT32          mDeqMn;
        //
        MINT32          mBufStream;
        MINT32          mBufMn;
        MINT32          mBufFn;
        MINT32          mBufRn;
        //
        MINT32          mAcceptFn;
        MINT32          mAcceptRn;
        MINT32          mAcceptResult;
        //
        MINT32          mRevFn;
        MINT32          mRevRn;
        //
        MINT32          mOutFn;
        MINT32          mOutRn;
        //
    public:
        Data()
        : mReady(MFALSE)
        , mNowTv()
        , mNowTime(0)
        , mNowTid(0)
        //
        , mCbSyncType(0)
        , mCbProcType(0)
        //
        , mStartSetType(START_SET_GENERAL)
        , mStartSetMn(P1_MAGIC_NUM_NULL)
        //
        , mPreSetKey(P1_QUE_ID_NULL)
        //
        , mSetFn(P1_FRM_NUM_NULL)
        //
        , mSetMn(P1_MAGIC_NUM_NULL)
        , mEnqMn(P1_MAGIC_NUM_NULL)
        , mDeqMn(P1_MAGIC_NUM_NULL)
        //
        , mBufStream(-1)
        , mBufMn(P1_MAGIC_NUM_NULL)
        , mBufFn(P1_FRM_NUM_NULL)
        , mBufRn(P1_REQ_NUM_NULL)
        //
        , mAcceptFn(P1_FRM_NUM_NULL)
        , mAcceptRn(P1_REQ_NUM_NULL)
        , mAcceptResult(REQ_REV_RES_UNKNOWN)
        //
        , mRevFn(P1_FRM_NUM_NULL)
        , mRevRn(P1_REQ_NUM_NULL)
        //
        , mOutFn(P1_FRM_NUM_NULL)
        , mOutRn(P1_REQ_NUM_NULL)
        {
            memset(mTv, 0, sizeof(mTv));
            memset(mTime, 0, sizeof(mTime));
            memset(mTid, 0, sizeof(mTid));
            mNowTv.tv_sec = 0;
            mNowTv.tv_usec = 0;
        };
        virtual ~Data() {};
    };

public:
                    LogInfo()
                    : mLock()
                    , mOpenId(-1)
                    , mLogLevel(0)
                    , mLogLevelI(0)
                    , mBurstNum(1)
                    , mPid(0)
                    , mIsActive(MFALSE)
                    , mData()
                    , mCode(CC_NONE)
                    {
                        //MY_LOGD("+++");
                        mPid = (MUINT32)getpid();
                        clear();
                        //MY_LOGD("---");
                    };

    virtual         ~LogInfo()
                    {
                        //MY_LOGD("+++");
                        U_if (mLogLevel > 9 || mLogLevelI > 9) {
                            inspect(LogInfo::IT_COMMON_DUMP);
                        };
                        clear();
                        //MY_LOGD("---");
                    };

    void            clear(void);

    void            config(MINT32 nOpenId,
                        MINT32 nLogLevel,
                        MINT32 nLogLevelI,
                        MUINT8 nBurstNum
                        )
                    {
                        mOpenId = nOpenId;
                        mLogLevel = nLogLevel;
                        mLogLevelI = nLogLevelI;
                        mBurstNum = nBurstNum;
                        clear();
                    };

    void            setActive(MBOOL enable) { mIsActive = enable; return; };

    MBOOL           getActive(void) { return (mIsActive); };

    void            setMemo(LogInfo::CheckPoint cp,
                        MINT64 param0 = 0, MINT64 param1 = 0,
                        MINT64 param2 = 0, MINT64 param3 = 0);

    void            getMemo(LogInfo::CheckPoint cp, android::String8 * str);

    void            inspect(LogInfo::InspectType type = IT_COMMON_DUMP,
                        char const * info = NULL);

protected:
    void            write(LogInfo::CheckPoint cp,
                        MINT64 param0 = 0, MINT64 param1 = 0,
                        MINT64 param2 = 0, MINT64 param3 = 0);

    void            read(LogInfo::CheckPoint cp, android::String8 * str);

    void            reset(void) { mCode = CC_NONE; mData.mReady = MFALSE; };

    void            extract(void);

    void            analyze(MBOOL bForceToPrint = MFALSE);

    void            bitStr(MUINT64 bitClueCode, android::String8 * str) {
                        U_if (str == NULL) return;
                        switch (bitClueCode) {
                            #ifdef CCLT
                            #undef CCLT
                            #endif
                            #define CCLT(code, type, name, info, ...) \
                                case CC_ENUM(code) : str->appendFormat("[%s] %s ", name, info);\
                                    break;
                            //
                            ClueCodeListTable
                            //
                            #undef CCLT
                            default:
                                break;
                        };
                        //
                        std::vector< LogInfo::CheckPoint > vTag;
                        vTag.clear();
                        bitTag(bitClueCode, vTag);
                        if (vTag.size() > 0) {
                            str->appendFormat(" - reference tag ");
                            for (size_t i = 0; i < vTag.size(); i++) {
                                if (vTag[i] < LogInfo::CP_MAX) {
                                    str->appendFormat("<%s> ", mNotes[vTag[i]].main);
                                }
                            }
                        }
                    };

    void            bitTag(MUINT64 bitClueCode, std::vector< LogInfo::CheckPoint > & rvTag) {
                        rvTag.clear();
                        switch (bitClueCode) {
                            #ifdef CCLT
                            #undef CCLT
                            #endif
                            #define CCLT(code, type, name, info, ...) \
                                case CC_ENUM(code) : rvTag = { __VA_ARGS__ };\
                                    break;
                            //
                            ClueCodeListTable
                            //
                            #undef CCLT
                            default:
                                break;
                        };
                        #if 0
                        MY_LOGI("bit[0x%" PRIx64 "] tag[%zu]", bitClueCode, rvTag.size());
                        for (size_t i = 0; i < rvTag.size(); i++)
                            MY_LOGI("tag[%zu/%zu] = %d", i, rvTag.size(), rvTag[i]);
                        #endif
                    };

private:
    mutable RWLock  mLock;
    MINT32          mOpenId;
    MINT32          mLogLevel;
    MINT32          mLogLevelI;
    MUINT8          mBurstNum;
    MUINT32         mPid;
    MBOOL           mIsActive;

    // if the text string length is over TEXT_LEN,
    // or the text element number is over IT_MAX,
    // it should be discovered in the compile time.
    Text mTexts[IT_MAX] = {
        /* IT_COMMON_DUMP */
        {"check in common case and dump"},

        //IT_PERIODIC_CHECK
        {"check the status in the regular periodic timing"},

        //IT_STOP_NO_REQ_IN_GENERAL
        {"check while stop and request not arrival in general preview flow"},

        //IT_STOP_NO_REQ_IN_CAPTURE
        {"check while stop but request not ready in start capture flow"},

        //IT_STOP_NO_REQ_IN_REQUEST
        {"check while stop but request not ready in initial request flow"},

        //IT_WAIT_CATURE
        {"check as start capture flow waiting AAA-CB"},

        //IT_WAIT_AAA_CB
        {"check as queue waiting AAA-CB"},

        //IT_BUFFER_EXCEPTION
        {"check as the image buffer cannot acquire"},

        //IT_NO_DELIVERY
        {"check since no more frame delivery"},

        //IT_FLUSH_BLOCKING
        {"check since wait flush but timeout"},

        //IT_EVT_WAIT_DRAIN_TIMEOUT
        {"check since IO event inform streaming off but wait request drain timeout"},

        //IT_LAUNCH_STATE_TIMEOUT
        {"check since wait for the specific launch state but timeout"},

    };

    // if the note string length is over NOTE_LEN,
    // or the note element number is over CP_MAX,
    // it should be discovered in the compile time.
    Note mNotes[CP_MAX] = {
        { CP_REQ_ARRIVE, "Queue@Arrive",
            {"FrameNum", "RequestNum", "", ""}},

        { CP_REQ_ACCEPT, "Queue@Accept",
            {"FrameNum", "RequestNum", "IsAccepted", "ReceivingResult"}},

        { CP_REQ_REV, "Queue@REV",
            {"FrameNum", "RequestNum", "", ""}},

        { CP_REQ_RET, "Queue@RET",
            {"FrameNum", "RequestNum", "", ""}},

        { CP_REQ_NOTIFY_BGN, "ReqNotify+++",
            {"LastFrameNum", "LastRequestNum", "PipelineCbButNotQueueCnt", ""}},

        { CP_REQ_NOTIFY_END, "ReqNotify---",
            {"LastFrameNum", "LastRequestNum", "PipelineCbButNotQueueCnt", ""}},

        { CP_CB_SYNC_REV, "3A_CB_SYNC@REV",
            {"MsgType", "", "", ""}},

        { CP_CB_SYNC_RET, "3A_CB_SYNC@RET",
            {"MsgType", "Skip", "", ""}},

        { CP_CB_PROC_REV, "3A_CB_PROC@REV",
            {"MsgType", "MagicNum", "SofIdx", ""}},

        { CP_CB_PROC_RET, "3A_CB_PROC@RET",
            {"MsgType", "Skip", "", ""}},

        { CP_START_SET_BGN, "3A.StartSet+++",
            {"Type", "MagicNum", "", ""}},

        { CP_START_SET_END, "3A.StartSet---",
            {"Type", "MagicNum", "", ""}},

        { CP_PRE_SET_BGN, "3A.PreSet+++",
            {"PreSetKey", "Dummy", "FrameNum", "RequestNum"}},

        { CP_PRE_SET_END, "3A.PreSet---",
            {"PreSetKey", "Dummy", "FrameNum", "RequestNum"}},

        { CP_SET_BGN, "3A.Set+++",
            {"PreSetKey", "MagicNum", "FrameNum", "RequestNum"}},

        { CP_SET_END, "3A.Set---",
            {"PreSetKey", "MagicNum", "FrameNum", "RequestNum"}},

        { CP_BUF_BGN, "AcqBuf+++",
            {"StreamNum", "StreamId", "FrameNum", "RequestNum"}},

        { CP_BUF_END, "AcqBuf---",
            {"StreamNum", "StreamId", "FrameNum", "RequestNum"}},

        { CP_ENQ_BGN, "DRV.EnQ+++",
            {"MagicNum", "FrameNum", "RequestNum", "SofIdx"}},

        { CP_ENQ_END, "DRV.EnQ---",
            {"MagicNum", "FrameNum", "RequestNum", "SofIdx"}},

        { CP_DEQ_BGN, "DRV.DeQ+++",
            {"", "", "", ""}},

        { CP_DEQ_END, "DRV.DeQ---",
            {"ResultMagicNum", "", "", ""}},

        { CP_OUT_BGN, "DispatchNext+++",
            {"MagicNum", "FrameNum", "RequestNum", ""}},

        { CP_OUT_END, "DispatchNext---",
            {"MagicNum", "FrameNum", "RequestNum", ""}},

        // OPs Checking
        // OP_START

        { CP_OP_START_BGN, "OpStart+++",
            {"BurstMode", "StartCap", "InitReq", ""}},

        { CP_OP_START_3A_PWRON_BGN, "OpStart.3aPwrOn+++",
            {"", "", "", ""}},

        { CP_OP_START_3A_PWRON_END, "OpStart.3aPwrOn---",
            {"", "", "", ""}},

        { CP_OP_START_3A_CFG_BGN, "OpStart.3aCfg+++",
            {"", "", "", ""}},

        { CP_OP_START_3A_CFG_END, "OpStart.3aCfg---",
            {"", "", "", ""}},

        { CP_OP_START_3A_START_BGN, "OpStart.3aStart+++",
            {"", "", "", ""}},

        { CP_OP_START_3A_START_END, "OpStart.3aStart---",
            {"", "", "", ""}},

        { CP_OP_START_DRV_INIT_BGN, "OpStart.DrvInit+++",
            {"", "", "", ""}},

        { CP_OP_START_DRV_INIT_END, "OpStart.DrvInit---",
            {"", "", "", ""}},

        { CP_OP_START_DRV_CFG_BGN, "OpStart.DrvCfg+++",
            {"", "", "", ""}},

        { CP_OP_START_DRV_CFG_END, "OpStart.DrvCfg---",
            {"", "", "", ""}},

        { CP_OP_START_DRV_START_BGN, "OpStart.DrvStart+++",
            {"", "", "", ""}},

        { CP_OP_START_DRV_START_END, "OpStart.DrvStart---",
            {"", "", "", ""}},

        { CP_OP_START_REQ_WAIT_BGN, "OpStart.ReqWait+++",
            {"Type", "", "", ""}},

        { CP_OP_START_REQ_WAIT_END, "OpStart.ReqWait---",
            {"Type", "", "", ""}},

        { CP_OP_START_END, "OpStart---",
            {"BurstMode", "StartCap", "InitReq", "Type"}},

        // OP_STOP
        { CP_OP_STOP_BGN, "OpStop+++",
            {"LastFrameNum", "LastRequestNum", "PipelineCbButNotQueueCnt", ""}},

        { CP_OP_STOP_3A_PWROFF_BGN, "OpStop.3aPwrOff+++",
            {"", "", "", ""}},

        { CP_OP_STOP_3A_PWROFF_END, "OpStop.3aPwrOff---",
            {"", "", "", ""}},

        { CP_OP_STOP_3A_STOPSTT_BGN, "OpStop.3aStopStt+++",
            {"", "", "", ""}},

        { CP_OP_STOP_3A_STOPSTT_END, "OpStop.3aStopStt---",
            {"", "", "", ""}},

        { CP_OP_STOP_3A_STOP_BGN, "OpStop.3aStop+++",
            {"", "", "", ""}},

        { CP_OP_STOP_3A_STOP_END, "OpStop.3aStop---",
            {"", "", "", ""}},

        { CP_OP_STOP_DRV_UNINIT_BGN, "OpStop.DrvUninit+++",
            {"", "", "", ""}},

        { CP_OP_STOP_DRV_UNINIT_END, "OpStop.DrvUninit---",
            {"", "", "", ""}},

        { CP_OP_STOP_DRV_STOP_BGN, "OpStop.DrvStop+++",
            {"", "", "", ""}},

        { CP_OP_STOP_DRV_STOP_END, "OpStop.DrvStop---",
            {"IsAbort", "", "", ""}},

        { CP_OP_STOP_HW_LOCK_BGN, "OpStop.HwLock+++",
            {"", "", "", ""}},

        { CP_OP_STOP_HW_LOCK_END, "OpStop.HwLock---",
            {"", "", "", ""}},

        { CP_OP_STOP_END, "OpStop---",
            {"LastFrameNum", "LastRequestNum", "PipelineCbButNotQueueCnt", ""}},

        // API Checking
        { CP_API_FLUSH_BGN, "ApiFlush+++",
            {"", "", "", ""}},

        { CP_API_FLUSH_END, "ApiFlush---",
            {"", "", "", ""}},

        { CP_API_UNINIT_BGN, "ApiUninit+++",
            {"", "", "", ""}},

        { CP_API_UNINIT_END, "ApiUninit---",
            {"", "", "", ""}},

    };

    Slot mSlots[CP_MAX];
    Data mData;
    MUINT64 mCode; // ClueCode
};


};//namespace NSP1Node
};//namespace v3
};//namespace NSCam



#endif//_MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_UTILITY_H_

