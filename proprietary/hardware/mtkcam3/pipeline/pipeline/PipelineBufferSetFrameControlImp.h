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

#ifndef _MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINEBUFFERSETFRAMECONTROLIMP_H_
#define _MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINEBUFFERSETFRAMECONTROLIMP_H_
//
#include <mtkcam3/pipeline/pipeline/IPipelineBufferSetFrameControl.h>
//
#include <atomic>
#include <list>
#include <vector>

#include <mtkcam3/pipeline/utils/streambuf/StreamBuffers.h>

using AppMetaStreamBufferT  = NSCam::v3::IMetaStreamBuffer;
using AppImageStreamBufferT = NSCam::v3::IImageStreamBuffer;
struct HalMetaStreamBufferT : public NSCam::v3::IMetaStreamBuffer {};
struct HalImageStreamBufferT : public NSCam::v3::IImageStreamBuffer {};
using PipelineFrameNodeIdT  = NSCam::v3::IPipelineFrame::NodeId_T;

using AppMetaSetT   = android::Vector<android::sp<AppMetaStreamBufferT>>;
using AppImageSetT  = std::vector<android::sp<NSCam::v3::IImageStreamBuffer>>;
using HalMetaSetT   = std::list<android::sp<NSCam::v3::IMetaStreamBuffer>>;
using HalImageSetT  = std::list<android::sp<NSCam::v3::IImageStreamBuffer>>;

using AppMetaMapT   = android::DefaultKeyedVector<PipelineFrameNodeIdT,
                                                    std::vector<android::sp<AppMetaStreamBufferT>>>;
using HalMetaMapT   = android::DefaultKeyedVector<PipelineFrameNodeIdT,
                                                    std::vector<android::sp<HalMetaStreamBufferT>>>;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam::v3::pipeline::NSPipelineBufferSetFrameControlImp {


/******************************************************************************
 *  Forward Declaration
 ******************************************************************************/
class ReleasedCollector;


/******************************************************************************
 *  Buffer Status
 ******************************************************************************/
enum {
    eBUF_STATUS_ACQUIRE         = 0,    //buffer has been acquired.
    eBUF_STATUS_RELEASE,                //all producers/consumers users release
    eBUF_STATUS_PRODUCERS_RELEASE,      //all producers release
    eBUF_STATUS_ACQUIRE_FAILED          //Has try to acquire buffer but failed
};


/******************************************************************************
 *  Buffer Map
 ******************************************************************************/
struct  IMyMap
{
    struct  IItem
        : public android::RefBase
    {
        virtual IStreamInfo*    getStreamInfo() const                       = 0;
        virtual IUsersManager*  getUsersManager() const                     = 0;
        virtual android::sp<IItem>
                                handleAllUsersReleased()                    = 0;
        virtual MVOID           handleProducersReleased()                   = 0;
        virtual MVOID           handleProducersReleasedByNodeId(NodeId_T nodeId) = 0;
        virtual MBOOL           hasBufferStatus(MUINT32 mask)               = 0;
    };

    virtual                     ~IMyMap() {}

    virtual android::sp<IItem>  itemFor(StreamId_T streamId) const          = 0;
    virtual android::sp<IItem>  itemAt(size_t index) const                  = 0;
    virtual size_t              size() const                                = 0;
    virtual ssize_t             indexOfKey(StreamId_T const key) const      = 0;
    virtual StreamId_T          keyAt(size_t index) const                   = 0;
};


template <
    class _StreamBufferT_,
    class _IStreamBufferT_ = typename _StreamBufferT_::IStreamBufferT
>
class TItemMap
    : public IMyMap
    , public IPipelineBufferSetFrameControl::IMap<_IStreamBufferT_>
{
public:     ////                Definitions.
    using StreamBufferT = _StreamBufferT_;
    using IStreamBufferT= _IStreamBufferT_;
    using IStreamInfoT  = typename StreamBufferT::IStreamInfoT;

    struct Item;
    friend struct Item;
    using ItemT         = Item;
    using MapValueT     = android::sp<ItemT>;
    using MapT          = android::DefaultKeyedVector<StreamId_T, MapValueT>;

public:     ////                Data Members.
    MapT                        mMap;
    ssize_t                     mNonReleasedNum;        // In + Inout
    ReleasedCollector*          mReleasedCollector;

public:     ////                Operations.
                                TItemMap(ReleasedCollector* pReleasedCollector);
    MVOID                       onProducersReleased(ItemT& item);
    MVOID                       onProducersReleasedByNodeId(NodeId_T nodeId, ItemT& item);
    MVOID                       onAllUsersReleased(ItemT& item);

    virtual android::sp<Item>   getItemFor(StreamId_T streamId) const   { return mMap.valueFor(streamId); }

public:     ////                Operations: IMyMap
    virtual android::sp<IItem>  itemFor(StreamId_T streamId) const      { return mMap.valueFor(streamId); }
    virtual android::sp<IItem>  itemAt(size_t index) const              { return mMap.valueAt(index); }

public:     ////                Operations: IMap
    virtual ssize_t             setCapacity(size_t size)                { return mMap.setCapacity(size); }
    virtual bool                isEmpty() const                         { return mMap.isEmpty(); }
    virtual size_t              size() const                            { return mMap.size(); }
    virtual ssize_t             indexOfKey(StreamId_T const key) const  { return mMap.indexOfKey(key); }
    virtual StreamId_T          keyAt(size_t index) const               { return mMap.keyAt(index); }
    virtual IUsersManager*      usersManagerAt(size_t index) const;
    virtual auto       streamInfoAt(size_t index) const -> IStreamInfoT*;

    virtual ssize_t             add(android::sp<IStreamInfoT>, android::sp<IUsersManager>);
    virtual ssize_t             add(android::sp<IStreamBufferT>);
};


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
struct
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
Item : public IMyMap::IItem
{
public:     ////    Data Members.
    android::wp<TItemMap>       mItselfMap;
    android::sp<IStreamBufferT> mBuffer;
    android::sp<IStreamInfoT>   mStreamInfo;
    android::sp<IUsersManager>  mUsersManager;
    android::BitSet32           mBitStatus;

public:     ////    Operations.
                    Item(
                        android::wp<TItemMap> pItselfMap,
                        IStreamBufferT* pStreamBuffer,
                        IStreamInfoT*   pStreamInfo,
                        IUsersManager*  pUsersManager
                    )
                        : mItselfMap(pItselfMap)
                        , mBuffer(pStreamBuffer)
                        , mStreamInfo(pStreamInfo)
                        , mUsersManager(pUsersManager)
                        , mBitStatus(0)
                    {
                        if  ( pStreamBuffer != 0 ) {
                            mBitStatus.markBit(eBUF_STATUS_ACQUIRE);
                        }
                    }

    virtual IStreamInfo*    getStreamInfo() const   { return mStreamInfo.get(); }
    virtual IUsersManager*  getUsersManager() const { return mUsersManager.get(); }

    virtual android::sp<IItem>
                    handleAllUsersReleased()
                    {
                        android::sp<Item> pItem = this;
                        if  ( ! mBitStatus.hasBit(eBUF_STATUS_RELEASE) ) {
                            mBitStatus.markBit(eBUF_STATUS_RELEASE);
                            android::sp<TItemMap> pMap = mItselfMap.promote();
                            if  ( pMap != 0 ) {
                                pMap->onAllUsersReleased(*this);
                            }
                        }
                        return pItem;
                    }

    virtual MVOID   handleProducersReleased()
                    {
                        if  ( ! mBitStatus.hasBit(eBUF_STATUS_PRODUCERS_RELEASE) ) {
                            mBitStatus.markBit(eBUF_STATUS_PRODUCERS_RELEASE);
                            android::sp<TItemMap> pMap = mItselfMap.promote();
                            if  ( pMap != 0 ) {
                                pMap->onProducersReleased(*this);
                            }
                        }
                    }

    virtual MVOID   handleProducersReleasedByNodeId(NodeId_T nodeId)
                    {
                        if  ( ! mBitStatus.hasBit(eBUF_STATUS_PRODUCERS_RELEASE) ) {
                            android::sp<TItemMap> pMap = mItselfMap.promote();
                            if  ( pMap != 0 ) {
                                pMap->onProducersReleasedByNodeId(nodeId, *this);
                            }
                        }
                    }

    virtual MBOOL   hasBufferStatus(MUINT32 mask)
                    {
                        if  ( mBuffer != 0 ) {
                            return mBuffer->hasStatus(mask);
                        }
                        return MFALSE;
                    }
};


typedef TItemMap<AppImageStreamBufferT, IImageStreamBuffer  > ItemMap_AppImageT;
typedef TItemMap<AppMetaStreamBufferT,  IMetaStreamBuffer   > ItemMap_AppMetaT;
typedef TItemMap<HalImageStreamBufferT, IImageStreamBuffer  > ItemMap_HalImageT;
typedef TItemMap<HalMetaStreamBufferT,  IMetaStreamBuffer   > ItemMap_HalMetaT;


/******************************************************************************
 *  Frame Releaser
 ******************************************************************************/
class ReleasedCollector
    : public android::RefBase
{
public:     ////    Data Members.
    mutable android::Mutex
                    mLock;
    AppImageSetT    mAppImageSet_AllUsersReleased;
    HalImageSetT    mHalImageSet_AllUsersReleased;
    HalMetaSetT     mHalMetaSet_AllUsersReleased;

    AppMetaSetT     mAppMetaSetO_ProducersReleased; //out
    ssize_t         mAppMetaNumO_ProducersInFlight; //out
    // note: use AppMetaSetT, since IMetaStreamBuffer is used in callback
    AppMetaSetT     mHalMetaSetO_ProducersReleased; //out
    ssize_t         mHalMetaNumO_ProducersInFlight; //out
    // for each node metadata
    AppMetaMapT     mAppMetaSetOByNodeId_ProducersReleased; //out
    AppMetaMapT     mHalMetaSetOByNodeId_ProducersReleased; //out

public:     ////    Operations.
    MVOID           finishConfiguration(
                        ItemMap_AppImageT const&,
                        ItemMap_AppMetaT const& rItemMap_AppMeta,
                        ItemMap_HalImageT const&,
                        ItemMap_HalMetaT const& rItemMap_HalMeta
                    )
                    {
                        {
                            mAppMetaNumO_ProducersInFlight = 0;
                            ItemMap_AppMetaT const& rItemMap = rItemMap_AppMeta;
                            for (size_t i = 0; i < rItemMap.size(); i++) {
                                if  ( 0 < rItemMap.usersManagerAt(i)->getNumberOfProducers() ) {
                                    mAppMetaNumO_ProducersInFlight++;
                                }
                            }
                        }
                        {
                            mHalMetaNumO_ProducersInFlight = 0;
                            ItemMap_HalMetaT const& rItemMap = rItemMap_HalMeta;
                            for (size_t i = 0; i < rItemMap.size(); i++) {
                                if  ( 0 < rItemMap.usersManagerAt(i)->getNumberOfProducers() ) {
                                    mHalMetaNumO_ProducersInFlight++;
                                }
                            }
                        }
                    }

public:     ////    Operations.
    MVOID           onAllUsersReleased(ItemMap_AppImageT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( rItem.mBuffer != 0 ) {
                            mAppImageSet_AllUsersReleased.push_back(rItem.mBuffer);
                        }
                    }
    MVOID           onAllUsersReleased(ItemMap_AppMetaT::Item&) {}
    MVOID           onAllUsersReleased(ItemMap_HalImageT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( rItem.mBuffer != 0 ) {
                            mHalImageSet_AllUsersReleased.push_back(rItem.mBuffer);
                        }
                    }
    MVOID           onAllUsersReleased(ItemMap_HalMetaT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( rItem.mBuffer != 0 ) {
                            mHalMetaSet_AllUsersReleased.push_back(rItem.mBuffer);
                        }
                    }

public:     ////    Operations.
    MVOID           onProducersReleased(ItemMap_HalImageT::Item&) {}
    MVOID           onProducersReleased(ItemMap_HalMetaT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( 0 < rItem.getUsersManager()->getNumberOfProducers() ) {
                            mHalMetaNumO_ProducersInFlight--;
                            if  ( rItem.mBuffer != 0 ) {
                                mHalMetaSetO_ProducersReleased.push_back(rItem.mBuffer);
                            }
                        }
                    }
    MVOID           onProducersReleased(ItemMap_AppImageT::Item&) {}
    MVOID           onProducersReleased(ItemMap_AppMetaT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( 0 < rItem.getUsersManager()->getNumberOfProducers() ) {
                            mAppMetaNumO_ProducersInFlight--;
                            if  ( rItem.mBuffer != 0 ) {
                                mAppMetaSetO_ProducersReleased.push_back(rItem.mBuffer);
                            }
                        }
                    }
    MVOID           onProducersReleasedByNodeId(NodeId_T, ItemMap_HalImageT::Item&) {}
    MVOID           onProducersReleasedByNodeId(NodeId_T nodeId, ItemMap_HalMetaT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( 0 < rItem.getUsersManager()->getNumberOfProducers() ) {
                            if  ( rItem.mBuffer != 0 ) {
                                ssize_t idx = mHalMetaSetOByNodeId_ProducersReleased.indexOfKey(nodeId);
                                if(idx >= 0)
                                {
                                    mHalMetaSetOByNodeId_ProducersReleased.editValueAt(idx).push_back(rItem.mBuffer);
                                }
                                else
                                {
                                    std::vector<android::sp<IMetaStreamBuffer>> metaStreamBufList;
                                    metaStreamBufList.push_back(rItem.mBuffer);
                                    mHalMetaSetOByNodeId_ProducersReleased.add(nodeId, metaStreamBufList);
                                }
                            }
                        }
                    }
    MVOID           onProducersReleasedByNodeId(NodeId_T, ItemMap_AppImageT::Item&) {}
    MVOID           onProducersReleasedByNodeId(NodeId_T nodeId, ItemMap_AppMetaT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( 0 < rItem.getUsersManager()->getNumberOfProducers() ) {
                            if  ( rItem.mBuffer != 0 ) {
                                ssize_t idx = mAppMetaSetOByNodeId_ProducersReleased.indexOfKey(nodeId);
                                if(idx >= 0)
                                {
                                    mAppMetaSetOByNodeId_ProducersReleased.editValueAt(idx).push_back(rItem.mBuffer);
                                }
                                else
                                {
                                    std::vector<android::sp<IMetaStreamBuffer>> metaStreamBufList;
                                    metaStreamBufList.push_back(rItem.mBuffer);
                                    mAppMetaSetOByNodeId_ProducersReleased.add(nodeId, metaStreamBufList);
                                }
                            }
                        }
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
TItemMap(ReleasedCollector* pReleasedCollector)
    : mMap()
    , mNonReleasedNum(0)
    , mReleasedCollector(pReleasedCollector)
{}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
MVOID
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
onProducersReleasedByNodeId(NodeId_T nodeId, ItemT& item)
{
    if  (CC_LIKELY( mReleasedCollector != 0 )) {
        mReleasedCollector->onProducersReleasedByNodeId(nodeId, item);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
MVOID
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
onProducersReleased(ItemT& item)
{
    if  (CC_LIKELY( mReleasedCollector != 0 )) {
        mReleasedCollector->onProducersReleased(item);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
MVOID
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
onAllUsersReleased(ItemT& item)
{
    android::sp<ItemT>& rpItem = mMap.editValueFor(item.mStreamInfo->getStreamId());
    if  ( rpItem != 0 ) {
        if  (CC_LIKELY( mReleasedCollector != 0 )) {
            mReleasedCollector->onAllUsersReleased(item);
        }
        mNonReleasedNum--;
        rpItem = NULL;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
IUsersManager*
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
usersManagerAt(
    size_t index
) const
{
    if (mMap.valueAt(index) != nullptr)
        return mMap.valueAt(index)->mUsersManager.get();
    else
        return nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
auto
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
streamInfoAt(
    size_t index
) const -> IStreamInfoT*
{
    if (mMap.valueAt(index) != nullptr)
        return mMap.valueAt(index)->mStreamInfo.get();
    else
        return nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
ssize_t
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
add(
    android::sp<IStreamInfoT> pStreamInfo,
    android::sp<IUsersManager> pUsersManager
)
{
    StreamId_T const streamId = pStreamInfo->getStreamId();
    //
    if  ( pUsersManager == 0 ) {
        pUsersManager = new NSCam::v3::Utils::UsersManager(
            streamId, pStreamInfo->getStreamName()
        );
    }
    //
    mNonReleasedNum++;
    return mMap.add(
        streamId,
        new ItemT(this, 0, pStreamInfo.get(), pUsersManager.get())
    );
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
ssize_t
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
add(android::sp<IStreamBufferT> value)
{
    if  (CC_UNLIKELY( value == 0 )) {
        return -EINVAL;
    }
    //
    StreamId_T const streamId = value->getStreamInfo()->getStreamId();
    //
    mNonReleasedNum++;
    return mMap.add(
        streamId,
        new ItemT(this, value.get(), const_cast<IStreamInfoT*>(value->getStreamInfo()), value.get())
    );
}


/**
 * An Implementation of Pipeline Buffer Set Frame Control.
 */
class PipelineBufferSetFrameControlImp
    : public IPipelineBufferSetFrameControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Frame Listener
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct  MyListener
    {
        using IListener = IPipelineFrameListener;
        android::wp<IListener>      mpListener;
        MVOID*                      mpCookie;
                                    //
                                    MyListener(
                                        android::wp<IListener> listener = NULL,
                                        MVOID*const cookie = NULL
                                    )
                                        : mpListener(listener)
                                        , mpCookie(cookie)
                                    {}
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Node Status
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct  NodeStatus
        : public android::LightRefBase<NodeStatus>
    {
        struct IO : public android::LightRefBase<IO>
        {
            android::sp<IMyMap::IItem>
                                    mMapItem;
        };

        struct  IOSet
            : public android::List<android::sp<IO> >
        {
            MBOOL                   mNotified;
                                    //
                                    IOSet()
                                        : mNotified(MFALSE)
                                    {}
        };

        IOSet                       mISetImage;
        IOSet                       mOSetImage;
        IOSet                       mISetMeta;
        IOSet                       mOSetMeta;
    };

    struct  NodeStatusMap
        : public android::KeyedVector<NodeId_T, android::sp<NodeStatus> >
    {
        size_t                      mInFlightNodeCount = 0;
        MUINT32                     mBufferStatus = 0;
    };

    struct  NodeStatusUpdater;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Common.
    MUINT32 const                   mFrameNo;
    MUINT32 const                   mRequestNo;
    MBOOL const                     mbReprocessFrame;
    GroupFrameTypeT const           mGroupFrameType;
    mutable android::RWLock         mRWLock;
    mutable android::Mutex          mApplyReleaseLock;
    android::wp<IAppCallback>const  mpAppCallback;
    android::List<MyListener>       mListeners;

    struct timespec                 mTimestampFrameCreated;
    struct timespec                 mTimestampFrameDone;

    struct DebugLog
    {
        struct timespec timestamp;
        std::string     str;
    };
    using DebugLogListT = std::list<DebugLog>;
    DebugLogListT                   mDebugLogList;
    mutable android::RWLock         mDebugLogListRWLock;
    // physica camera setting
    std::unordered_map<uint32_t, std::vector<StreamId_T> >
                                    mvPhysicalAppStreamIds;

    std::atomic_bool                mIsAborted{false};
    bool                            mIsUnexpectedToAbort{false};

protected:  ////                    Configuration.
    std::weak_ptr<IPipelineFrameStreamBufferProvider>
                                    mStreamBufferProvider;
    std::weak_ptr<IPipelineFrameHalResultProcessor>
                                    mHalResultProcessor;

    android::wp<IPipelineNodeCallback>
                                    mpPipelineCallback;

    android::sp<IStreamInfoSet const>
                                    mpStreamInfoSet;

    android::sp<IPipelineFrameNodeMapControl>
                                    mpNodeMap;

    android::wp<IPipelineNodeMap const>
                                    mpPipelineNodeMap;

    android::sp<IPipelineDAG>       mpPipelineDAG;

    std::shared_ptr<TrackFrameResultParamsT const>
                                    mTrackFrameResultParams;

protected:  ////
    mutable android::Mutex          mItemMapLock;
    NodeStatusMap                   mNodeStatusMap;
    android::sp<ReleasedCollector>  mpReleasedCollector;
    android::sp<ItemMap_AppImageT>  mpItemMap_AppImage;
    android::sp<ItemMap_AppMetaT>   mpItemMap_AppMeta;
    android::sp<ItemMap_HalImageT>  mpItemMap_HalImage;
    android::sp<ItemMap_HalMetaT>   mpItemMap_HalMeta;

protected:  ////                    Timestamp
    /**
     * Android sensor timestamp.
     *
     * Time at start of exposure of first row of the image sensor active array, in nanoseconds.
     *
     * https://developer.android.com/reference/android/hardware/camera2/CaptureResult#SENSOR_TIMESTAMP
     */
    std::atomic_int64_t             mASensorTimestamp{0};

    /**
     * Device 3.5: Active physical ID
     *
     * https://developer.android.com/reference/android/hardware/camera2/CaptureResult#LOGICAL_MULTI_CAMERA_ACTIVE_PHYSICAL_ID
     *
     */
    int32_t                         mActivePhysicalID = -1;

protected:  ////                    Information Keeper

    mutable android::Mutex          mInformationKeeperLock;

    /**
     * https://developer.android.com/reference/android/hardware/camera2/CaptureResult#SENSOR_TIMESTAMP
     */
    mutable int64_t                 mSensorTimestamp = 0;

    /**
     * SetT: HalImageSetT, HalMetaSetT, AppMetaSetT
     */
    template <class SetT>
    struct PendingReleaseSet
    {
        bool                        isPendingToRelease = false;

        /**
         *  If isPendingToRelease == false, this argument is ignored.
         *  If isPendingToRelease == true, this argument indicates a set of target stream id to keep.
         *  If isPendingToRelease == true and this target set is empty, then all stream buffers will be kept.
         */
        std::set<StreamId_T>        targetStreamId;

        /**
         *  Pending release buffer set.
         */
        SetT                        set;
    };
    PendingReleaseSet<HalImageSetT> mPendingRelease_HalImage;
    PendingReleaseSet<HalMetaSetT>  mPendingRelease_HalMeta;
    PendingReleaseSet<AppMetaSetT>  mPendingRelease_AppMeta;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation
                    PipelineBufferSetFrameControlImp(CreationParams const& arg);

public:     ////                    Operations.
    MERROR                          finishConfigurationLocked();

protected:  ////                    Operations.
    MVOID                           handleReleasedBuffers(
                                        UserId_T userId,
                                        android::sp<IAppCallback> pAppCallback
                                    );

protected:  ////                    Operations.
    android::sp<IUsersManager>      findSubjectUsersLocked(
                                        StreamId_T streamId
                                    )   const;

    android::sp<IMyMap::IItem>      getMapItemLocked(
                                        StreamId_T streamId,
                                        IMyMap const& rItemMap
                                    )   const;

    android::sp<IMyMap::IItem>      getMetaMapItemLocked(StreamId_T streamId) const;
    android::sp<IMyMap::IItem>      getImageMapItemLocked(StreamId_T streamId) const;

    template <class ItemMapT>
    android::sp<typename ItemMapT::IStreamBufferT>
                                    getBufferLockedImp(
                                        StreamId_T streamId,
                                        UserId_T userId,
                                        ItemMapT const& rMap
                                    )   const;

    template <class ItemMapT>
    android::sp<typename ItemMapT::IStreamBufferT>
                                    getBufferLocked(
                                        StreamId_T streamId,
                                        UserId_T userId,
                                        ItemMapT const& rMap
                                    )   const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Configuration.
    virtual MERROR                  startConfiguration();
    virtual MERROR                  finishConfiguration();

    virtual MERROR                  setNodeMap(android::sp<IPipelineFrameNodeMapControl> value);
    virtual MERROR                  setPipelineNodeMap(android::sp<IPipelineNodeMap const> value);
    virtual MERROR                  setPipelineDAG(android::sp<IPipelineDAG> value);
    virtual MERROR                  setStreamInfoSet(android::sp<IStreamInfoSet const> value);
    // update physical app metadata stream id list.
    virtual MERROR  setPhysicalAppMetaStreamIds(
                        std::unordered_map<uint32_t, std::vector<StreamId_T> > const&
                                                physicalAppMetaStreamIds
                    ) ;

    virtual auto    setTrackFrameResultParams(
                        std::shared_ptr<TrackFrameResultParamsT const>const& arg
                    ) -> void;

    virtual auto    getTrackFrameResultParams() const
                        -> std::shared_ptr<TrackFrameResultParamsT const>;

    virtual auto    setUnexpectedToAbort(bool enabled) -> void { mIsUnexpectedToAbort = enabled; }

    virtual MVOID                   logDebugInfo(std::string&& str);

    virtual auto    configureInformationKeeping(
                    ConfigureInformationKeepingParams const& arg
                    ) -> void override;

public:     ////    Operations.
    virtual auto    transferPendingReleaseBuffers(
                        android::Vector<android::sp<IImageStreamBuffer>>& out
                    ) -> void override;

    virtual auto    transferPendingReleaseBuffers(
                        android::Vector<android::sp<IMetaStreamBuffer>>& out
                    ) -> void override;

    virtual auto    tryGetSensorTimestamp() const -> int64_t override;

public:     ////    Operations.
    virtual auto    abort() -> void override { mIsAborted = true; }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetControl Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations

#define _EDITMAP_(_NAME_, _TYPE_) \
    virtual android::sp<IMap< ItemMap_##_NAME_##_TYPE_##T::IStreamBufferT > > \
            editMap_##_NAME_##_TYPE_() \
            { \
                return mpItemMap_##_NAME_##_TYPE_; \
            }

    _EDITMAP_(Hal, Image)   // editMap_HalImage
    _EDITMAP_(App, Image)   // editMap_AppImage
    _EDITMAP_(Hal, Meta)    // editMap_HalMeta
    _EDITMAP_(App, Meta)    // editMap_AppMeta

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamBufferSet Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MVOID                   applyPreRelease(UserId_T userId);
    virtual MVOID                   applyRelease(UserId_T userId);

    virtual MUINT32                 markUserStatus(
                                        StreamId_T const streamId,
                                        UserId_T userId,
                                        MUINT32 eStatus
                                    );

    virtual MERROR                  setUserReleaseFence(
                                        StreamId_T const streamId,
                                        UserId_T userId,
                                        MINT releaseFence
                                    );

    virtual MUINT                   queryGroupUsage(
                                        StreamId_T const streamId,
                                        UserId_T userId
                                    )   const;

    virtual MINT                    createAcquireFence(
                                        StreamId_T const streamId,
                                        UserId_T userId
                                    )   const;

public:     ////                    Operations.
    virtual android::sp<IMetaStreamBuffer>
                                    getMetaBuffer(
                                        StreamId_T streamId,
                                        UserId_T userId
                                    )   const;

    virtual android::sp<IImageStreamBuffer>
                                    getImageBuffer(
                                        StreamId_T streamId,
                                        UserId_T userId
                                    )   const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineFrame Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual char const*             getMagicName() const { return IPipelineBufferSetFrameControl::magicName(); }
    virtual void*                   getMagicInstance() const { return (void*)this; }

public:     ////                    Attributes.
    virtual MUINT32                 getFrameNo() const   { return mFrameNo; }
    virtual MUINT32                 getRequestNo() const { return mRequestNo; }
    virtual MBOOL                   IsReprocessFrame() const { return mbReprocessFrame; }

    virtual auto    getGroupFrameType() const -> GroupFrameTypeT { return mGroupFrameType; }

    virtual auto    getSensorTimestamp() const -> int64_t override { return mASensorTimestamp.load(); }
    virtual auto    setSensorTimestamp(int64_t timestamp, char const* callerName) -> int override;

    virtual auto    getActivePhysicalID() const -> int32_t override { return mActivePhysicalID; }
    virtual auto    setActivePhysicalID(int32_t physicalID) -> int override;

    virtual auto    isAborted() const -> bool override { return mIsAborted.load(); }
    virtual auto    isUnexpectedToAbort() const -> bool override { return mIsUnexpectedToAbort; }

public:     ////                    Operations.
    virtual MERROR                  attachListener(
                                        android::wp<IPipelineFrameListener>const&,
                                        MVOID* pCookie
                                    );

    virtual MVOID                   dumpState(
                                        android::Printer& printer __unused,
                                        const std::vector<std::string>& options __unused
                                    )   const;

    virtual android::sp<IPipelineNodeMap const>
                                    getPipelineNodeMap() const;
    virtual IPipelineDAG const&     getPipelineDAG() const;
    virtual android::sp<IPipelineDAG>     getPipelineDAGSp();
    virtual IStreamInfoSet const&   getStreamInfoSet() const;
    virtual IStreamBufferSet&       getStreamBufferSet() const;
    virtual android::sp<IPipelineNodeCallback>
                                    getPipelineNodeCallback() const;

    virtual MERROR                  queryIOStreamInfoSet(
                                        NodeId_T const& nodeId,
                                        android::sp<IStreamInfoSet const>& rIn,
                                        android::sp<IStreamInfoSet const>& rOut
                                    )   const;

    virtual MERROR                  queryInfoIOMapSet(
                                        NodeId_T const& nodeId,
                                        InfoIOMapSet& rIOMapSet
                                    )   const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual void                    onLastStrongRef(const void* id);
};


/******************************************************************************
 *
 ******************************************************************************/
};  //NSCam::v3::pipeline::NSPipelineBufferSetFrameControlImp
#endif  //_MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINEBUFFERSETFRAMECONTROLIMP_H_

