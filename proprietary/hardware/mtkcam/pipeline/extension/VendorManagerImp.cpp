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

#define LOG_TAG "MtkCam/vmgr_imp"
//
#include "MyUtils.h"
//
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Time.h>
//
#include <mtkcam/pipeline/extension/IVendorManager.h>
//
#include <vector>
#include <algorithm> // std::find

using namespace android;
using namespace NSCam;
using namespace NSCam::plugin;
using namespace NSCam::Utils;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_IN             MY_LOGD_IF(1, "%d +", getOpenId());
#define FUNCTION_OUT            MY_LOGD_IF(1, "%d -", getOpenId());

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -ENODEV;} \
} while(0)

#define CHECK_BOOL(x)  do{                                              \
    if ( !(x) ) { MY_LOGE("error bool."); return -ENODEV;}               \
} while(0)

#define CHECK_STATUS(x)  do{                                            \
    if ( x != OK ) { MY_LOGE("%s error status.", #x); return -ENODEV;}  \
} while(0)

#define ENABLE_DEBUG_LOG 1


/******************************************************************************
 *
 ******************************************************************************/

class RequestLifeCtrl : public virtual RefBase
{
public:
    RequestLifeCtrl() {};
    ~RequestLifeCtrl() {};

public:
    virtual status_t onRequestEnd( MINT64 key ) = 0;
};

class VendorManagerImp
    : public IVendorManager
    , public RequestLifeCtrl
{
public:         ////
                                    VendorManagerImp(
                                        char const*  pcszName,
                                        MINT32 const i4OpenId
                                    );
    virtual                         ~VendorManagerImp() {}

public:         ////
    virtual char const*             getName( MINT64 vendorMode );
    virtual int32_t                 getOpenId() const { return mOpenId; };

public:        ////
    virtual status_t                get(
                                        MUINT64          userID,
                                        InputInfo&       in,
                                        /*out*/
                                        OutputInfo&      out
                                    );
    virtual status_t                set(
                                        MUINT64             userID,
                                        const InputSetting& in
                                    );
    virtual status_t                registerItem(
                                        MUINT64        userID,
                                        sp<IVendor>    pVendor
                                    );
    virtual status_t                registerItems(
                                        MUINT64        userID,
                                        InputInfo&     in
                                    );
    virtual status_t                removeItem(
                                        MINT64         vendorMode,
                                        MUINT64        userID
                                     );

public:         ////
    virtual status_t                queue(
                                        MUINT32                     const requestNo,
                                        sp<IVendorManager::IDataCallback> cb,
                                        Vector< BufferItem >              srcBuffer,
                                        MetaItem                          setting
                                    );
    virtual status_t                acquireWorkingBuffer(
                                        MUINT64            userID,
                                        MetaItem           setting,
                                        /*output*/
                                        BufferItem&        buf
                                    );
    virtual status_t                releaseWorkingBuffer(
                                        MUINT64            userID,
                                        MetaItem           setting,
                                        BufferItem&        buf
                                    );
    virtual status_t                beginFlush( MUINT64 userID );
    virtual status_t                endFlush( MUINT64 userID );


public:     //// debug / extension
    virtual status_t                sendCommand(
                                        MINT32 cmd,
                                        MINT64 vendor,
                                        MetaItem& meta,
                                        MINT32& arg1, MINT32& arg2,
                                        void*   arg3
                                    );
    virtual status_t                dump( MUINT64 userID );

    virtual MVOID                   onLastStrongRef( const void* /*id*/);

public:         //// Request Life control
    virtual status_t                onRequestEnd( MINT64 key );

protected:      ////

    class RequestHandler :
        public IVendor::IDataCallback
    {
    public:
        enum {
            STATE_WAIT_SET,
            STATE_READY,
            STATE_RUNNING,
            STATE_DONE,
            STATE_ERROR,
        };
        // data for each vendor in/out
        struct VendorPackage
        {
            MINT64 vendorMode;
            IVendor::BufferParam buf;
            IVendor::MetaParam   meta;
        };

        struct CombineInfo
        {
            MINT64 vendorMode;
            MINT32 bufferType;
            sp<IVendor> pVendor;
        };

        struct Param
        {
            MINT32  frameNo;
            MUINT32 openId;
            MINT64  key;
            //
            FrameInfo info; // i/o
            //
            Vector<CombineInfo> vCombine;
            Vector<VendorPackage> list; // size should be = vCombine or vCombine +1
            //
            wp<IVendorManager::IDataCallback> cb;

            Param() : frameNo(0), openId(0), key(0) {}
        };
    public:
        virtual status_t onDispatch();

    public:
        RequestHandler( wp<RequestLifeCtrl> ctrl, Param param, bool enableLog)
            : mEnableLog(enableLog)
            , mpCtrl(ctrl)
            , mParam(param)
            , mStatus(STATE_WAIT_SET)
        {}
        ~RequestHandler() {};
        MVOID  trans( MINT32 status ) { Mutex::Autolock _l(mLock); mStatus = status; }
        MINT32 getState() { Mutex::Autolock _l(mLock); return mStatus; }

        void dump() {
            MY_LOGD("Dump RequestHandler req:%d, key:%" PRId64, mParam.frameNo, mParam.key);
#if 1
            if ( mEnableLog ) {
                String8 str;
                if ( !mParam.vCombine.isEmpty() )
                    str += String8::format("Combination : %" PRId64 " -> ",
                                            mParam.vCombine.editItemAt(0).vendorMode);
                if ( mParam.vCombine.size() > 1 ) {
                    for ( size_t i = 1; i < mParam.vCombine.size() - 1; ++i )
                        str += String8::format("%" PRId64 " -> ", mParam.vCombine.editItemAt(i).vendorMode);
                    str += String8::format("%" PRId64 ,
                        mParam.vCombine.editItemAt(mParam.vCombine.size() - 1).vendorMode);
                }
                MY_LOGD("%s", str.string());
            }

            if ( mParam.list.empty() ) return;
            for ( size_t i = 0; i < mParam.list.size() - 1; ++i ) {
                VendorPackage *package = &(mParam.list.editItemAt(i));
                MY_LOGD("Vendor %" PRId64 ":", mParam.vCombine[i].vendorMode);
                //
                (*package).meta.dump();
                (*package).buf.dump();
            }
#endif
        }

    public:
        bool                mEnableLog;
        //
        wp<RequestLifeCtrl> mpCtrl;
        Param               mParam;
        MINT32              mStatus;
        //
        mutable Mutex       mLock;
    };

    /**
     * struct to store vendor information.
     *
     * @param caller     : combination of all caller.
     * @param vendor     : pointer to IVendor.
     */
    class VendorItem : public RefBase
    {
    protected:
        MUINT64     caller;
        sp<IVendor> vendor;
        MBOOL       enableLog;
        //
        mutable Mutex mLock;

    public:     ////
        VendorItem(sp<IVendor> v, MBOOL enable)
            : caller(0)
            , vendor(v)
            , enableLog(enable)
        {}

    public:     ////
        MBOOL dec(MUINT64 userID) {
            Mutex::Autolock _l(mLock);
            if ( caller&userID ) caller &= ~userID;
            MY_LOGD_IF(enableLog, "remove %s", log().string());
            return (caller==0);
        }
        MBOOL inc(MUINT64 userID) {
            Mutex::Autolock _l(mLock);
            if ( !(caller&userID) ) caller |= userID;
            MY_LOGD_IF(enableLog, "%s", log().string());
            return true;
        }

    public:     ////
        sp<IVendor> getVendor() { Mutex::Autolock _l(mLock); return vendor; }
        MUINT64     getCaller() { Mutex::Autolock _l(mLock); return caller;}

    public:     ////
        String8 log() {
            String8 str;
            if ( vendor.get() ) {
                str = String8::format("[%" PRId64 "] %s caller %" PRIu64
                        , vendor->getVendorMode(), vendor->getName(), caller);
            } else {
                str = String8::format("Error: null vendor");
            }
            return str;
        }
    };

protected:      //// implementation
    sp<IVendor>                      getVendor( MINT64 vendorMode );
    sp<IVendor>                      getVendorAndCheck( MINT64 vendorMode );
    status_t                         addVendor( MINT64 vendorMode, sp<VendorItem> item);
    sp<VendorManagerImp::VendorItem> getVendorItem( MINT64 vendorMode );
    //
    sp<RequestHandler>               getRequestHandler( const IMetadata* halSetting, MINT32 status );
    status_t                         updateRelation(MUINT64 caller, MINT64 vendorMode, MBOOL inc);
    status_t                         updateRelationAndClear(MUINT64 caller, MINT64 vendorMode);
    //
    status_t                         parseBuffer(
                                        MUINT32                     const requestNo,
                                        sp<IVendorManager::IDataCallback> cb,
                                        Vector< BufferItem >              srcBuffer,
                                        MetaItem                          setting,
                                        sp<RequestHandler>                handler
                                    );
    status_t                        parseMeta(
                                        sp<RequestHandler> handler,
                                        MetaItem           setting,
                                        MBOOL              needWorkBuf
                                    );
protected:      ////
    MUINT32                           mOpenId;
    MINT32                            mLogLevel;

protected:
    sp<IImageStreamInfo>              mpWorkingBufferInfo;

protected:      ////
    //relation map : caller & vendor relation --> flush / dump
    mutable Mutex                             mRelationLock;
    KeyedVector< MUINT64, MINT64 >            mRelationMap;// caller&vendor
    //
    mutable Mutex                             mVendorLock;
    KeyedVector< MINT64, sp<VendorItem> >     mVendorMap; // mode & sp
    //
    mutable Mutex                             mCombinationLock;
    KeyedVector< MINT64, Vector<sp<RequestHandler>> >
                                              mCombinationMap; // key & request

    mutable Mutex                             mWorOutLock;
    KeyedVector< MINT64, sp<BufferHandle> >   mWorOutMap; // key & work buf

protected:      ////
    mutable Mutex                             mLock;

};


/******************************************************************************
 *
 ******************************************************************************/
IVendorManager*
IVendorManager::
createInstance(
    char const*  pcszName,
    MINT32 const i4OpenId
)
{
    return new VendorManagerImp(pcszName, i4OpenId);
}

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace plugin {
namespace NSVendorManager{
    namespace
    {
        KeyedVector<MINT32, sp<IVendorManager> > gMap;
        RWLock                                   gMapLock;
    }

    void
    clear() {
        RWLock::AutoWLock _l(gMapLock);
        gMap.clear();
    }

    void
    add(int32_t openId, IVendorManager* pManager) {
        RWLock::AutoWLock _l(gMapLock);
        gMap.add(openId, pManager);
    }

    void
    remove(int32_t openId) {
        RWLock::AutoWLock _l(gMapLock);
        gMap.removeItem(openId);
    }

    sp<IVendorManager>
    get(int32_t openId) {
        RWLock::AutoRLock _l(gMapLock);
        if ( gMap.indexOfKey(openId) >= 0 )
            return gMap.valueFor(openId);
        return nullptr;
    }
};
};
};

/******************************************************************************
 *
 ******************************************************************************/
VendorManagerImp::
VendorManagerImp(
    char const*  pcszName,
    MINT32 const i4OpenId
)
    : mOpenId(i4OpenId)
{
    mLogLevel = ::property_get_int32("debug.camera.log.vmgr", 0);
    MY_LOGI("Manager %d created by %s", mOpenId, pcszName);

    mpWorkingBufferInfo = createImageStreamInfo(
                           "vmgr:sample",
                           IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                           eSTREAMTYPE_IMAGE_INOUT,
                           0, 0,
                           0, eImgFmt_RGBA8888, MSize(), 0
                        );

}

/******************************************************************************
 *
 ******************************************************************************/
char const*
VendorManagerImp::
getName( MINT64 vendorMode )
{
    sp<IVendor> vendor = getVendor(vendorMode);
    if ( !vendor.get() ) return "vendor not found";
    return vendor->getName();
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IVendor>
VendorManagerImp::
getVendor(MINT64 vendorMode)
{
    Mutex::Autolock _l(mVendorLock);
    if ( mVendorMap.indexOfKey(vendorMode) >= 0 ) {
        return (mVendorMap.editValueFor(vendorMode))->getVendor();
    }
    return nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IVendor>
VendorManagerImp::
getVendorAndCheck(MINT64 vendorMode)
{
    Mutex::Autolock _l(mVendorLock);
    if ( mVendorMap.indexOfKey(vendorMode) >= 0 ) {
        sp<IVendor> vendor = (mVendorMap.editValueFor(vendorMode))->getVendor();
        if( !vendor.get() ) MY_LOGE("nullptr");
        return vendor;
    }
    return nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
addVendor(MINT64 vendorMode, sp<VendorItem> item)
{
    Mutex::Autolock _l(mVendorLock);
    mVendorMap.add(vendorMode, item);
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<VendorManagerImp::VendorItem>
VendorManagerImp::
getVendorItem( MINT64 vendorMode )
{
    Mutex::Autolock _l(mVendorLock);
    if ( mVendorMap.indexOfKey(vendorMode) >= 0 ) {
        return mVendorMap.editValueFor(vendorMode);
    }
    return nullptr;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
updateRelationAndClear(MUINT64 caller, MINT64 vendorMode)
{
    // set relation
    Mutex::Autolock _ll(mRelationLock);
    if ( mRelationMap.indexOfKey(caller) >= 0) {
        mRelationMap.editValueFor(caller) &= ~vendorMode;
        if ( mRelationMap[caller] == 0 ) mRelationMap.removeItem(caller);
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
updateRelation(MUINT64 caller, MINT64 vendorMode, MBOOL inc)
{
    // set relation
    Mutex::Autolock _ll(mRelationLock);
    if ( inc ) {
        if ( mRelationMap.indexOfKey(caller) < 0) mRelationMap.add(caller, vendorMode);
        else mRelationMap.editValueFor(caller) |= vendorMode;
    } else {
        if ( mRelationMap.indexOfKey(caller) >= 0) {
            mRelationMap.editValueFor(caller) &= ~vendorMode;
            //if ( mRelationMap[caller] == 0 ) mRelationMap.removeItem(caller);
        }
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<VendorManagerImp::RequestHandler>
VendorManagerImp::
getRequestHandler( const IMetadata* halSetting, MINT32 state )
{
    Mutex::Autolock _l(mCombinationLock);
    MINT64 key = -1;
    sp<RequestHandler> handler;
    if ( !tryGetMetadata<MINT64>( (IMetadata*)halSetting, MTK_PLUGIN_COMBINATION_KEY, key ) ) {
        MY_LOGW("MTK_PLUGIN_COMBINATION_KEY not found.");
        return nullptr;
    }

    if ( mCombinationMap.indexOfKey(key) >= 0 ) {
        Vector<sp<RequestHandler>> *list = &(mCombinationMap.editValueFor(key));
        for (size_t i = 0; i < (*list).size(); ++i) {
            if( (*list).editItemAt(i)->getState() == state ) {
                handler = (*list).editItemAt(i);
                break;
            }
        }
    }
    else {
        MY_LOGE("key not found.");
    }
    //
    // dump for error status
    if ( !handler.get() && key != -1 ) {
        MY_LOGI( "try to find key:%" PRId64 " state:%d from %zu requests", key, state, mCombinationMap.size() );
        for (size_t j = 0; j < mCombinationMap.size(); ++j) {
            Vector<sp<RequestHandler>> *list = &(mCombinationMap.editValueAt(j));
            for (size_t i = 0; i < (*list).size(); ++i) {
                MY_LOGI("Req:%d key:%" PRId64 " [%zu]state:%d ",
                    (*list).editItemAt(i)->mParam.frameNo, key, i, (*list).editItemAt(i)->getState());
            }
        }
    }
    //
    return handler;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
get(
    MUINT64          userID,
    InputInfo&       in,
    /*out*/
    OutputInfo&      out
)
{
    FUNCTION_IN;
    Mutex::Autolock _l(mLock);
    MY_LOGW_IF(in.combination.size() > 3, "more than 3 vendors, may have performance issue.");

    in.workingbuf = mpWorkingBufferInfo;

#if ENABLE_DEBUG_LOG
    // dump setting
    if(mLogLevel >= 1) in.dump();
#endif

    Vector<RequestHandler::CombineInfo> combineInfoList;
    for ( size_t i = 0; i < in.combination.size(); ++i ){
        sp<IVendor> vendor = getVendor(in.combination[i]);
        if ( !vendor.get() ) {
            vendor = IVendor::createInstance("manager", mOpenId, in.combination[i]);
            CHECK_OBJECT(vendor);
            sp<VendorItem> item = new VendorItem(vendor, mLogLevel >= 1);
            addVendor(in.combination[i], item);
        }
        sp<VendorItem> item = getVendorItem(in.combination[i]);
        CHECK_OBJECT(item);
        item->inc(userID);
        updateRelation(userID, in.combination[i], true);
        IVendor::FrameInfoSet info;
        vendor->get(mOpenId, in, /*out*/info);

        CHECK_BOOL( !(info.frameCount>1 &&
            !info.settings.empty() && info.frameCount!=static_cast<int>(info.settings.size())) );
        //
        out.inCategory  &= info.table.inCategory;
        out.outCategory |= info.table.outCategory;
        out.table.add(vendor->getVendorMode(), info.table);
        //
        RequestHandler::CombineInfo cInfo;
        cInfo.vendorMode = in.combination[i];
        cInfo.bufferType = info.table.bufferType;
        cInfo.pVendor    = vendor;
        combineInfoList.push_back(cInfo);
        //
        out.frameCount = (info.frameCount > out.frameCount) ? info.frameCount : out.frameCount;
        if ( info.settings.size() > out.settings.size() ) {
            for ( size_t i = 0; i < out.settings.size(); ++i ) {
                out.settings.editItemAt(i).setting.halMeta += info.settings[i].setting.halMeta;
                out.settings.editItemAt(i).setting.appMeta += info.settings[i].setting.appMeta;
            }
            for ( size_t i = out.settings.size(); i < info.settings.size(); ++i )
                out.settings.push_back(info.settings[i]);
        } else{
            for ( size_t i = 0; i < info.settings.size(); ++i ) {
                out.settings.editItemAt(i).setting.halMeta += info.settings[i].setting.halMeta;
                out.settings.editItemAt(i).setting.appMeta += info.settings[i].setting.appMeta;
            }
        }

        // dummy frames for 3A stable (optional for 3A stable before capture)
        out.dummyCount = (info.dummyCount > out.dummyCount) ? info.dummyCount : out.dummyCount;
        if ( info.dummySettings.size() > out.dummySettings.size() ) {
            for ( size_t i = 0; i < out.dummySettings.size(); ++i ) {
                out.dummySettings.editItemAt(i).setting.halMeta += info.dummySettings[i].setting.halMeta;
                out.dummySettings.editItemAt(i).setting.appMeta += info.dummySettings[i].setting.appMeta;
            }
            for ( size_t i = out.dummySettings.size(); i < info.dummySettings.size(); ++i )
                out.dummySettings.push_back(info.dummySettings[i]);
        } else{
            for ( size_t i = 0; i < info.dummySettings.size(); ++i ) {
                out.dummySettings.editItemAt(i).setting.halMeta += info.dummySettings[i].setting.halMeta;
                out.dummySettings.editItemAt(i).setting.appMeta += info.dummySettings[i].setting.appMeta;
            }
        }

        // delayed frames for 3A stable (optional for 3A stable before resume preview)
        out.delayedCount = (info.delayedCount > out.delayedCount) ? info.delayedCount : out.delayedCount;
        if ( info.delayedSettings.size() > out.delayedSettings.size() ) {
            for ( size_t i = 0; i < out.delayedSettings.size(); ++i ) {
                out.delayedSettings.editItemAt(i).setting.halMeta += info.delayedSettings[i].setting.halMeta;
                out.delayedSettings.editItemAt(i).setting.appMeta += info.delayedSettings[i].setting.appMeta;
            }
            for ( size_t i = out.delayedSettings.size(); i < info.delayedSettings.size(); ++i )
                out.delayedSettings.push_back(info.delayedSettings[i]);
        } else{
            for ( size_t i = 0; i < info.delayedSettings.size(); ++i ) {
                out.delayedSettings.editItemAt(i).setting.halMeta += info.delayedSettings[i].setting.halMeta;
                out.delayedSettings.editItemAt(i).setting.appMeta += info.delayedSettings[i].setting.appMeta;
            }
        }

    }

    if (out.settings.isEmpty()) out.settings.resize(out.frameCount);
    if (out.dummySettings.isEmpty()) out.dummySettings.resize(out.dummyCount);
    if (out.delayedSettings.isEmpty()) out.delayedSettings.resize(out.delayedCount);

    // check combination profile
    if ( out.frameCount != static_cast<int>(out.settings.size()) ) {
        MY_LOGE("Profile check fail. frame:%d setting size:%zu", out.frameCount, out.settings.size());
        return -ENODEV;
    }
    if ( out.dummyCount != static_cast<int>(out.dummySettings.size()) ) {
        MY_LOGE("Profile check fail. dummy frame:%d setting size:%zu", out.dummyCount, out.dummySettings.size());
        return -ENODEV;
    }
    if ( out.delayedCount != static_cast<int>(out.delayedSettings.size()) ) {
        MY_LOGE("Profile check fail. delayed frame:%d setting size:%zu", out.delayedCount, out.delayedSettings.size());
        return -ENODEV;
    }

    MINT64 key = 0;
    if ( tryGetMetadata<MINT64>( const_cast<IMetadata*>(&(in.halCtrl)), MTK_PLUGIN_COMBINATION_KEY, key ) ) {
        Mutex::Autolock _l(mCombinationLock);
        MY_LOGD("Key has set.");
        MY_LOGF_IF(out.frameCount > 1, "Currently not support multiple fram with both raw / yuv post-processing.");
        if ( out.inCategory != FORMAT_YUV || FORMAT_YUV != out.outCategory )
            MY_LOGF("Second plugin must be yuv/yuv.");
        RequestHandler::Param param;
        param.frameNo     = -1;
        param.openId      = mOpenId;
        param.key         = key;
        param.vCombine    = combineInfoList;

        sp<RequestHandler> handler = new RequestHandler(this, param, mLogLevel>=1);
        mCombinationMap.editValueFor(key).push_back(handler);
        return OK;
    }

    for ( MINT32 k = 0; k < out.frameCount; ++k ) {
        key = TimeTool::getReadableTime();
        {
            Mutex::Autolock _l(mCombinationLock);
            while ( mCombinationMap.indexOfKey(key) >= 0 ) key = TimeTool::getReadableTime();
            //
            RequestHandler::Param param;
            param.openId      = mOpenId;
            param.key         = key;
            param.vCombine    = combineInfoList;

            sp<RequestHandler> handler = new RequestHandler(this, param, mLogLevel>=1);
            //
            Vector<sp<RequestHandler>> l;
            l.push_back(handler);
            mCombinationMap.add(key, l);
        }
        //
        CHECK_BOOL( trySetMetadata<MINT64>( out.settings.editItemAt(k).setting.halMeta
                                        , MTK_PLUGIN_COMBINATION_KEY, key) );
    }

    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
set(
    MUINT64             userID,
    const InputSetting& in
)
{
    std::vector< sp<IVendor> >          vendors;   // saves unique vendor
    std::vector< sp<RequestHandler> >   handlers;  // saves all handle w/ state STATE_WAIT_SET

    Mutex::Autolock _l(mLock);
    CHECK_BOOL( in.vFrame.size() > 0 );

    if ( mLogLevel >= 1 ) in.dump();

    if ( CC_LIKELY(in.vFrame.size() > 0) )
        handlers.resize(in.vFrame.size()); // resize vector first

    // for every frames ...
    for ( size_t j = 0; j < in.vFrame.size(); j++ ) {
        sp<RequestHandler> handler = getRequestHandler(&(in.vFrame[j].curHalControl), RequestHandler::STATE_WAIT_SET);
        CHECK_OBJECT(handler);

        handlers[j] = handler; //saves handler
        handler->mParam.info = in.vFrame[j];

        // retrieve all vendors from combination but make vendor unique
        for ( size_t i = 0; i < handler->mParam.vCombine.size(); ++i ) {
            sp<IVendor> vendor = getVendorAndCheck(handler->mParam.vCombine[i].vendorMode);
            // If the vendor doesn't exist in container "vendors", push_back into it
            if (std::find(vendors.begin(), vendors.end(), vendor) == vendors.end()) {
                vendors.push_back(vendor);
                //      ^^^^^^^^^^^^^^^^^
                // Remark: std::vector manages memory as a continuous chunk,
                // CPU cache can cover this tiny chunk. Hence we do not need
                // to use std::deque or other hash-based container.
            }
        }
    }

    // invoke IVendor::set first,
    for (auto& itr : vendors) {
        if ( itr.get() ) {
            itr->set(mOpenId, in);
            MY_LOGD("set [%" PRId64 "]%s by %" PRIu64 ,
                    itr->getVendorMode(),
                    itr->getName(),
                    userID);
        }
    }

    // transform state to STATE_READY
    for (auto& itr : handlers) {
        if (itr.get())
            itr->trans(RequestHandler::STATE_READY);
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
registerItem(
    MUINT64        userID,
    sp<IVendor>    pVendor
)
{
    CHECK_OBJECT(pVendor);
    //
    Mutex::Autolock _l(mLock);
    //
    sp<VendorItem> item = getVendorItem(pVendor->getVendorMode());
    if ( item == nullptr ) {
        item = new VendorItem(pVendor, mLogLevel>=1);
        addVendor(pVendor->getVendorMode(), item);
    }
    item->inc(userID);
    updateRelation(userID, pVendor->getVendorMode(), true);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
registerItems(
    MUINT64          userID,
    InputInfo&       in
)
{
    FUNCTION_IN;
    Mutex::Autolock _l(mLock);
    MY_LOGW_IF(in.combination.size() > 3, "more than 3 vendors, may have performance issue.");

#if ENABLE_DEBUG_LOG
    // dump setting
    if(mLogLevel >= 1) in.dump();
#endif

    for ( size_t i = 0; i < in.combination.size(); ++i ) {
        sp<IVendor> vendor = getVendor(in.combination[i]);
        if ( !vendor.get() ) {
            vendor = IVendor::createInstance("manager", mOpenId, in.combination[i]);
            CHECK_OBJECT(vendor);
            sp<VendorItem> item = new VendorItem(vendor, mLogLevel >= 1);
            addVendor(in.combination[i], item);
        }
        sp<VendorItem> item = getVendorItem(in.combination[i]);
        CHECK_OBJECT(item);
        item->inc(userID);
        updateRelation(userID, in.combination[i], true);
    }

    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
removeItem(
    MINT64         vendorMode,
    MUINT64        userID
)
{
    Mutex::Autolock _l(mLock);
    //
    sp<VendorItem> item = getVendorItem(vendorMode);
    CHECK_OBJECT(item);
    if ( item->dec(userID) ) {
        Mutex::Autolock _l(mVendorLock);
        mVendorMap.removeItem(vendorMode);
    }
    updateRelationAndClear(userID, vendorMode);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
parseBuffer(
    MUINT32                     const requestNo,
    sp<IVendorManager::IDataCallback> cb,
    Vector< BufferItem >              srcBuffer,
    MetaItem                          setting,
    sp<RequestHandler>                handler
)
{
    MINT32 req = requestNo;
    // from raw processor
    if (handler->mParam.list.empty()) CHECK_STATUS( parseMeta(handler, setting, false) );
    if ( setting.others.size() != handler->mParam.list[0].meta.vIn.size() ) {
        android::Vector<android::sp<MetaHandle> >   vIn;
        for ( size_t i = 0; i < setting.others.size(); ++i ) {
            sp<MetaHandle> handle = new MetaHandle(setting.others[i], String8::format("Input %zu", i));
            vIn.push_back(handle);
        }
        for ( size_t i = 0; i < handler->mParam.list.size(); ++i ) {
            handler->mParam.list.editItemAt(i).meta.vIn.clear();
            handler->mParam.list.editItemAt(i).meta.vIn = vIn;
        }
    }
    //
    #define ADD_BUFFER(_INDEX_, _DIR_, _BUFFER_) {                                  \
        handler->mParam.list.editItemAt(_INDEX_).buf.v##_DIR_.push_back(_BUFFER_);  \
    }
    sp<BufferHandle> workingBuf;
    // handle user buffer
    {
        KeyedVector< int64_t, sp<BufferHandle> > input;
        KeyedVector< int64_t, sp<BufferHandle> > output;
        KeyedVector< int64_t, sp<BufferHandle> > OutStatusTrack;
        KeyedVector< int64_t, sp<BufferHandle> > InStatusTrack;

        for ( size_t i = 0; i < srcBuffer.size(); ++i ){
            if (srcBuffer[i].bufferStatus&BUFFER_IN &&
                (srcBuffer[i].streamInfo)->getStreamId() != IVENDOR_STREAMID_IMAGE_PIPE_WORKING
            ) {
                sp<BufferHandle> handle = new UserBufferHandle(srcBuffer[i], cb, requestNo);
                input.add( (srcBuffer[i].streamInfo)->getStreamId(), handle);
            }
            else if (srcBuffer[i].bufferStatus&BUFFER_OUT &&
                (srcBuffer[i].streamInfo)->getStreamId() != IVENDOR_STREAMID_IMAGE_PIPE_WORKING
            ) {
                sp<BufferHandle> handle = new UserBufferHandle(srcBuffer[i], cb, requestNo);
                output.add( (srcBuffer[i].streamInfo)->getStreamId(), handle);
            }
            else if ( (srcBuffer[i].streamInfo)->getStreamId() == IVENDOR_STREAMID_IMAGE_PIPE_WORKING ) {
                workingBuf = handler->mParam.list[0].buf.vIn[0];
                handler->mParam.list.editItemAt(0).buf.vIn.clear();
            }
            else
                MY_LOGF("streamId:%#" PRIx64 " neither input nor output.", (srcBuffer[i].streamInfo)->getStreamId());
        }

        MY_LOGD_IF(mLogLevel >= 1, "user input:%zu output:%zu working:%p", input.size(), output.size(), workingBuf.get());
        //
        // parse user defined i/o
        for( size_t i = 0; i < handler->mParam.vCombine.size(); ++i ){
            MINT64 mode = handler->mParam.list[i].vendorMode;
            if ( handler->mParam.info.vVendorInfo.indexOfKey(mode) < 0 ) continue;
            VendorInfo* info = &(handler->mParam.info.vVendorInfo.editValueFor(mode));
            //
            #define _UPDATE_TRACKING_(_DIR_, _BUFFER_LIST_) {                                      \
                for ( size_t j = 0; j < (*info).app##_DIR_.size(); ++j ) {                         \
                    if( _BUFFER_LIST_.indexOfKey((*info).app##_DIR_[j]) < 0 ) continue;            \
                    sp<BufferHandle> handle = _BUFFER_LIST_.editValueFor((*info).app##_DIR_[j]);   \
                    ADD_BUFFER(i, _DIR_, handle);                                                  \
                    _DIR_##StatusTrack.add((handle->getStreamInfo())->getStreamId(), handle);      \
                }                                                                                  \
            }

                _UPDATE_TRACKING_( In , input  );
                _UPDATE_TRACKING_( In , output );
                _UPDATE_TRACKING_( Out, output );
                _UPDATE_TRACKING_( Out, input  );

            #undef _UPDATE_TRACKING_
        }

        // check if all output buffers have producer
        // if not,
        if ( OutStatusTrack.size() != output.size() ) {
            MY_LOGD_IF(mLogLevel >= 1, "user output not define:%zu", output.size() - OutStatusTrack.size());
            // check last vendor for copy
            size_t last = handler->mParam.vCombine.size() - 1;
            if ( handler->mParam.list[last].vendorMode == MTK_PLUGIN_MODE_COPY ) {
                Vector<RequestHandler::CombineInfo> *category = &(handler->mParam.vCombine);
                // previous vendor output dst buffer, remove copy vendor
                if ((*category).editItemAt(last-1).bufferType == BUFFER_NO_WORKING_OUTPUT) {
                    sp<IVendor> pVendor = getVendorAndCheck(handler->mParam.list[last].vendorMode);
                    pVendor->sendCommand(
                            INFO_CANCEL,
                            mOpenId, setting,
                            req, req, nullptr);
                    category->removeAt(last);
                    handler->mParam.list.removeAt(last);
                    last -= 1;
                }
            }
        #define _UPDATE_PC_RELATION_(_DIR_, _BUFFER_LIST_, _INDEX_) {             \
            for( size_t i = 0; i < _DIR_##StatusTrack.size(); ++i )               \
                _BUFFER_LIST_.removeItem(OutStatusTrack.keyAt(i));                \
            for( size_t i = 0; i < _BUFFER_LIST_.size(); ++i )                    \
                ADD_BUFFER(_INDEX_, _DIR_, _BUFFER_LIST_.editValueAt(i));       \
        }

            _UPDATE_PC_RELATION_(Out, output, last);
        }

        // check if all input buffers have consumer
        // if not, first vendor will be consumer
        if ( InStatusTrack.size() != input.size() ) _UPDATE_PC_RELATION_(In, input, 0);

        #undef _UPDATE_PC_RELATION_
    }

    // handle working buffer
    {}

    bool fromRawProcessor = (workingBuf.get()) ? false : true;
    if ( !fromRawProcessor ) {
        // from yuv processor, may have yuv output
        // parse working buffer, from first vendor
        Vector<RequestHandler::CombineInfo> *category = &(handler->mParam.vCombine);
        for ( size_t i = 0; i < (*category).size(); ++i ) {
            int currentVendor = i;
            int nextVendor    = i+1;
            // inplace buffer, current in/out buffers are same
            // next vendor's input connect with current's output
            if ( (*category).editItemAt(i).bufferType == BUFFER_IN_PLACE ) {
                ADD_BUFFER(currentVendor  , In , workingBuf);
                ADD_BUFFER(currentVendor  , Out, workingBuf);
                ADD_BUFFER(nextVendor     , In , workingBuf);
            }
            // working buffer, current input are from previeous vendor or itself
            // current vendor will generate working buffer as output buffer and next vendor's input
            else if ( (*category).editItemAt(i).bufferType == BUFFER_WORKING ) {
                ADD_BUFFER(currentVendor  , In , workingBuf);
                sp<IVendor> pVendor = getVendorAndCheck(handler->mParam.list[i].vendorMode);
                workingBuf = new WorkingBufferHandle(pVendor, mOpenId, setting, BUFFER_OUT|BUFFER_EMPTY);
                ADD_BUFFER(currentVendor  , Out, workingBuf);
                ADD_BUFFER(nextVendor     , In , workingBuf);
            }
            // dst buffer output, no working buffer output
            else if ( (*category).editItemAt(i).bufferType == BUFFER_NO_WORKING_OUTPUT ) {
                ADD_BUFFER(currentVendor  , In , workingBuf);
            }
        }

        // working buffer to p2 node
        if ( //output.isEmpty() ||
            (*category).editItemAt((*category).size()-1).bufferType != BUFFER_NO_WORKING_OUTPUT ) {
            Mutex::Autolock _l(mWorOutLock);
            mWorOutMap.add(handler->mParam.key, workingBuf);
        }
    }

//#warning "FIXME"
    // check all vendor are connected
    {
        Vector<RequestHandler::CombineInfo> *category = &(handler->mParam.vCombine);
        if ( fromRawProcessor ) {
            // raw processor in, should not have working buffer output to p2 node
            // search backward
            for (size_t i = (*category).size() - 1; i > 0; --i)
            {
                // last vendor has output but does not have input
                Vector< sp<BufferHandle> >* v_out = &handler->mParam.list.editItemAt(i).buf.vOut;
                Vector< sp<BufferHandle> >* v_in  = &handler->mParam.list.editItemAt(i).buf.vIn;
                // MY_LOGD("[%d]connect : out:%d %d in:%d %d", i, v_out->size(), v_out->isEmpty(), v_in->size(), v_in->isEmpty());
                if ( !v_out->isEmpty() && v_in->isEmpty() ) {
                    MINT32 direction = BUFFER_IN;
                    sp<IVendor> pVendor = getVendorAndCheck(handler->mParam.list[i].vendorMode);
                    if ( (*category).editItemAt(i-1).bufferType != BUFFER_NO_WORKING_OUTPUT ) {
                        direction = BUFFER_OUT;
                        pVendor = getVendorAndCheck(handler->mParam.list[i-1].vendorMode);
                    }
                    //
                    sp<BufferHandle> buf = v_out->editItemAt(0);
                    if ( (*category).editItemAt(i).bufferType != BUFFER_IN_PLACE )
                        buf = new WorkingBufferHandle(
                                        pVendor, mOpenId, setting, direction|BUFFER_EMPTY);
                    CHECK_OBJECT(buf);
                    buf->updateStatus(direction);
                    //
                    // connect current vendor and previous.
                    ADD_BUFFER(i  , In , buf);
                    ADD_BUFFER(i-1, Out, buf);
                }
            }
        } else {
            // from yuv processor
            for ( size_t i = 0; i < (*category).size() - 1; ++i ) {
                Vector< sp<BufferHandle> >* v_out = &handler->mParam.list.editItemAt(i).buf.vOut;
                if ( v_out->isEmpty() ) {
                    sp<IVendor> pVendor = getVendorAndCheck(handler->mParam.list[i+1].vendorMode);
                    workingBuf = new WorkingBufferHandle(pVendor, mOpenId, setting, BUFFER_IN|BUFFER_EMPTY);
                    ADD_BUFFER(i  , Out, workingBuf);
                    ADD_BUFFER(i+1, In , workingBuf);
                }
            }
        }
    }
    #undef ADD_BUFFER

    // remove orphan vendor
    {
        Vector<RequestHandler::CombineInfo> *category = &(handler->mParam.vCombine);
        for ( size_t i = 0; i < (*category).size(); ) {
            Vector< sp<BufferHandle> >* v_out = &handler->mParam.list.editItemAt(i).buf.vOut;
            Vector< sp<BufferHandle> >* v_in  = &handler->mParam.list.editItemAt(i).buf.vIn;
            if ( v_out->isEmpty() && v_in->isEmpty() ) {
                sp<IVendor> pVendor = getVendorAndCheck(handler->mParam.list[i].vendorMode);
                pVendor->sendCommand(
                        INFO_CANCEL,
                        mOpenId, setting,
                        req, req, nullptr);
                category->removeAt(i);
                handler->mParam.list.removeAt(i);
                continue;
            }
            ++i;
        }
    }

#if 0
    MY_LOGD("Dump parse result.");
    for ( size_t i = 0; i < handler->mParam.list.size(); ++i ) {
        MY_LOGD("\tVendor:%" PRId64 , handler->mParam.list[i].vendorMode);
        handler->mParam.list.editItemAt(i).meta.dump();
        handler->mParam.list.editItemAt(i).buf.dump();
    }
#endif

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
queue(
    MUINT32                     const requestNo,
    sp<IVendorManager::IDataCallback> cb,
    Vector< BufferItem >              srcBuffer,
    MetaItem                          setting
)
{
    Mutex::Autolock _l(mLock);
    MY_LOGD_IF( mLogLevel >= 1, "Queue request %d from p2", requestNo);
    sp<RequestHandler> handler = getRequestHandler(&(setting.setting.halMeta), RequestHandler::STATE_READY);
    CHECK_OBJECT(handler);
    handler->mParam.cb      = cb;
    handler->mParam.frameNo = requestNo;
    //
    if ( parseBuffer(requestNo, cb, srcBuffer, setting, handler) != OK ) {
        handler->trans(RequestHandler::STATE_ERROR);
        onRequestEnd(handler->mParam.key);
        return -ENODEV;
    }

#if ENABLE_DEBUG_LOG
    if(mLogLevel >= 1) handler->dump();
#endif

    status_t err = handler->onDispatch();
    if ( err != OK ) {
        handler->trans(RequestHandler::STATE_ERROR);
        onRequestEnd(handler->mParam.key);
    } else {
        handler->trans(RequestHandler::STATE_RUNNING);
    }
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
parseMeta(
    sp<RequestHandler> handler,
    MetaItem           setting,
    MBOOL              needWorkBuf
)
{
    // construct meta part
    RequestHandler::VendorPackage package;
    package.meta.appCtrl = new MetaHandle(setting.setting.appMeta, String8::format("App Control"));
    package.meta.halCtrl = new MetaHandle(setting.setting.halMeta, String8::format("Hal Control"));
    for ( size_t i = 0; i < setting.others.size(); ++i ) {
        sp<MetaHandle> handle = new MetaHandle(setting.others[i], String8::format("Input %zu", i));
        package.meta.vIn.push_back(handle);
    }
    {
        sp<MetaHandle> handle = new MetaHandle(String8::format("App Result"));
        package.meta.appResult = handle;
    }
    {
        sp<MetaHandle> handle = new MetaHandle(String8::format("Hal Result"));
        package.meta.halResult = handle;
    }
    //
    // check if vendor still need to work
    for ( size_t i = 0; i < handler->mParam.vCombine.size();){
        sp<IVendor> pVendor = getVendorAndCheck(handler->mParam.vCombine[i].vendorMode);
        if ( handler->mParam.vCombine[i].bufferType == BUFFER_UNKNOWN ) {
            MINT32 needToProcess = 0;
            MINT32 bufferType = BUFFER_WORKING;
            pVendor->sendCommand(
                        INFO_PROCESS_AND_BUFFER,
                        mOpenId, setting,
                        needToProcess, bufferType, nullptr);
            if( !needToProcess ) {
                handler->mParam.vCombine.removeAt(i);
                //
                // if there's no vendor anymore, trans state to DONE and invoke
                // onRequestEnd
                if(!handler->mParam.vCombine.size()) {
#if ENABLE_DEBUG_LOG
                    ALOGD("There's no vendor anymore, trans state to DONE");
#endif
                   handler->trans(RequestHandler::STATE_DONE);
                   onRequestEnd(handler->mParam.key);
                }
                continue;
            }
            handler->mParam.vCombine.editItemAt(i).bufferType = bufferType;
        }
        package.vendorMode = handler->mParam.vCombine[i].vendorMode;
        handler->mParam.list.push_back(package);
        ++i;
    }

    // extra for callback to p2 node
    // only metadata or with working buffer out
    package.vendorMode = MTK_PLUGIN_MODE_COMBINATION;
    handler->mParam.list.push_back(package);

    if ( handler->mParam.vCombine.empty() ) {
        handler->mParam.list.clear();
        return -ENODEV;
    }
    // check bufferType
    for( size_t i = 0; i < handler->mParam.vCombine.size(); ++i ) {
        if ( handler->mParam.vCombine[i].bufferType == BUFFER_UNKNOWN ) {
            MY_LOGE(" cannot be unknown.");
            return -ENODEV;
        }
    }
//#warning "FIXME"
    // create first working buffer
    // query working buffer from first vendor
    if ( needWorkBuf ) {
        // final result from vendor is output to dst buffer
        MINT32 bufStatus = BUFFER_IN|BUFFER_EMPTY;
        if ( handler->mParam.vCombine[0].bufferType == BUFFER_NO_WORKING_OUTPUT ) {
            bufStatus |= BUFFER_DST;
        }
        sp<IVendor> pVendor = getVendorAndCheck(handler->mParam.vCombine[0].vendorMode);
        sp<BufferHandle> pBufHandle = new WorkingBufferHandle(pVendor, mOpenId, setting, bufStatus);
        handler->mParam.list.editItemAt(0).buf.vIn.push_back(pBufHandle);
    }

    return OK;

}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
acquireWorkingBuffer(
    MUINT64            /*userID*/,
    MetaItem           setting,
    /*output*/
    BufferItem&        buf
)
{
    FUNCTION_IN;
    // build MetaParam & first input working buffer
    sp<RequestHandler> handler = getRequestHandler(&(setting.setting.halMeta), RequestHandler::STATE_READY);
    CHECK_OBJECT(handler);
    //
    CHECK_STATUS( parseMeta(handler, setting, true) );

    sp<BufferHandle> pBufHandle = handler->mParam.list[0].buf.vIn[0];
    buf.bufferStatus = pBufHandle->getStatus();
    buf.heap         = pBufHandle->getHeap();
    buf.streamInfo   = pBufHandle->getStreamInfo();

    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
releaseWorkingBuffer(
    MUINT64            /*userID*/,
    MetaItem           setting,
    BufferItem&        buf
)
{
    Mutex::Autolock _l(mWorOutLock);
    MINT64 key = -1;
    if ( !tryGetMetadata<MINT64>( &(setting.setting.halMeta), MTK_PLUGIN_COMBINATION_KEY, key ) ) {
        MY_LOGW("MTK_PLUGIN_COMBINATION_KEY not found.");
        return -ENODEV;
    }

    if ( mWorOutMap.indexOfKey(key) >= 0 ) {
        sp<BufferHandle> handle = mWorOutMap.editValueFor(key);
        MY_LOGW_IF( handle->getStreamInfo()->getStreamId() != buf.streamInfo->getStreamId(),
                    "streamId not match");
        mWorOutMap.removeItem(key);
    } else {
        MY_LOGW("buffer cannot be released.");
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
beginFlush( MUINT64 userID )
{
    Mutex::Autolock _ll(mLock); // mLock is an operation locked, lock it first
    Mutex::Autolock _l(mRelationLock);
    if ( mRelationMap.indexOfKey(userID) < 0 ) return OK;
    MINT64 mask = 1;
    MINT64 vendorMode = mRelationMap.editValueFor(userID);
    MY_LOGD("releted vendor:%" PRId64 " for caller:%" PRIu64, vendorMode, userID);
    while ( mask <= vendorMode) {
        if ( mask&vendorMode ) {
            sp<VendorItem> item = getVendorItem(mask);
            sp<IVendor> vendor  = getVendorAndCheck(mask);
            CHECK_OBJECT(item);
            //
            if ( (item->getCaller()^userID) > 0 ) {
                MY_LOGD("[%" PRId64 "]%s has multiple caller %" PRIu64 ,
                        mask, vendor->getName(), item->getCaller());
            } else {
                vendor->beginFlush(mOpenId);
            }
        }
        mask = mask<<1;
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
endFlush( MUINT64 userID )
{
    Mutex::Autolock _ll(mLock); // mLock is an operation locked, lock it first
    Mutex::Autolock _l(mRelationLock);
    if ( mRelationMap.indexOfKey(userID) < 0 ) return OK;
    MINT64 mask = 1;
    MINT64 vendorMode = mRelationMap.editValueFor(userID);
    MY_LOGD("releted vendor:%" PRId64 " for caller:%" PRIu64, vendorMode, userID);
    while ( mask <= vendorMode) {
        sp<VendorItem> item = getVendorItem(mask);
        sp<IVendor> vendor  = getVendor(mask);
        if ( !(mask&vendorMode) ) goto lbExit;
        //
        CHECK_OBJECT(vendor);
        if ( item == nullptr ) {
            MY_LOGE("[%" PRId64 "] %s cannot endFlush", mask, vendor->getName());
            goto lbExit;
        }
        //
        vendor->endFlush(mOpenId);
        // clear invalid vendor
        if ( item->dec(userID) ) {
            Mutex::Autolock _l(mVendorLock);
            mVendorMap.removeItem(mask);
        }
lbExit:
        mask = mask<<1;
        continue;
    }
    mRelationMap.removeItem(userID);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
sendCommand(
    MINT32 cmd,
    MINT64 vendorMode,
    MetaItem& meta,
    MINT32& arg1, MINT32& arg2,
    void*  arg3
)
{
    sp<IVendor> vendor = getVendorAndCheck(vendorMode);
    //
    if (vendor.get() == nullptr) {
        MY_LOGW("vendorMode(%" PRId64 ") not found, cannot sendCommand(%#x), " \
                "arg1(%d), arg2(%d), arg3(%p)",
                vendorMode, cmd, arg1, arg2, arg3);
        return NAME_NOT_FOUND;
    }
    //MetaItem item;
    vendor->sendCommand( cmd, mOpenId, meta,
                         arg1, arg2, arg3 );
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
VendorManagerImp::
onLastStrongRef(const void* /*id*/)
{
#if ENABLE_DEBUG_LOG
    dump(CALLER_SHOT_MAX_NUMBER);
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
dump( MUINT64 userID )
{
#if 1
    MY_LOGI("Dump information for manager %d", mOpenId);
    {
        Mutex::Autolock _l(mVendorLock);
        MY_LOGI("\t-Number of vendor %zu", mVendorMap.size());
        for(size_t i = 0; i < mVendorMap.size(); ++i) {
            sp<VendorItem> item = mVendorMap.editValueAt(i);
            CHECK_OBJECT(item);
            MY_LOGI("\t%3zu. %s", i, (item->log()).string() );
        }
    }
    //
    {
        Mutex::Autolock _l(mRelationLock);
        MY_LOGI("\t-Number of caller %zu", mRelationMap.size());
        for(size_t i = 0; i < mRelationMap.size(); ++i) {
            MY_LOGI("\t%3zu. Caller %" PRIu64 , i, mRelationMap.keyAt(i));
            MINT64 mask = 1;
            MINT64 vendorMode = mRelationMap.editValueAt(i);
            while ( mask <= vendorMode) {
                sp<IVendor> vendor  = getVendor(mask);
                if (vendor.get()) MY_LOGI("\t\t\t[%" PRId64 "]%s", mask, vendor->getName() );
                mask = mask<<1;
            }
        }
    }
    //
    {
        Mutex::Autolock _l(mCombinationLock);
        MY_LOGI("\t-Request %zu", mCombinationMap.size());
        for(size_t i = 0; i < mCombinationMap.size(); ++i) {
            for(size_t j = 0; j < mCombinationMap[i].size(); ++j) {
                if ( mCombinationMap[i][j].get() )
                    mCombinationMap[i][j]->dump();
            }
        }
    }

    MY_LOGI("\n");
    // call each vendor
    {
        Mutex::Autolock _l(mRelationLock);
        if ( mRelationMap.indexOfKey(userID) >= 0 ) {
            MINT64 mask = 1;
            MINT64 vendorMode = mRelationMap.editValueFor(userID);
            while ( mask <= vendorMode) {
                sp<IVendor> vendor  = getVendor(mask);
                if ( vendor.get() ) vendor->dump(mOpenId);
                mask = mask<<1;
            }
        }
    }
#endif
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
InputInfo::dump() const
{
    if(combination.empty()) {
        MY_LOGE("Empty combination.");
        return;
    }
    MY_LOGD("InputInfo:");
    String8 str = String8::format("combination : %" PRId64 , combination[0]);
    for ( size_t i = 1; i < combination.size(); ++i )
        str += String8::format(" -> %" PRId64 , combination[i]);
    MY_LOGD("\t%s", str.string());
    //
    if(fullRaw.get())
        MY_LOGD("\t[%#" PRIx64 "] FullRaw", fullRaw->getStreamId());
    if(lcsoRaw.get())
        MY_LOGD("\t[%#" PRIx64 "] LcsoRaw", lcsoRaw->getStreamId());
    if(resizedRaw.get())
        MY_LOGD("\t[%#" PRIx64 "] ResizedRaw", resizedRaw->getStreamId());
    if(jpegYuv.get())
        MY_LOGD("\t[%#" PRIx64 "] Jpeg main", jpegYuv->getStreamId());
    if(thumbnailYuv.get())
        MY_LOGD("\t[%#" PRIx64 "] Jpeg thumbnail", thumbnailYuv->getStreamId());
    if(jpeg.get())
        MY_LOGD("\t[%#" PRIx64 "] Jpeg", jpeg->getStreamId());
    if(workingbuf.get())
        MY_LOGD("\t[%#" PRIx64 "] Working buffer", workingbuf->getStreamId());
    for(size_t i = 0; i < vYuv.size(); ++i)
        if(vYuv[i].get())
            MY_LOGD("\t[%#" PRIx64 "] Other yuv %zu", vYuv[i]->getStreamId(), i);
}

/******************************************************************************
 *
 ******************************************************************************/
void
InputSetting::dump() const
{
    MY_LOGI("Dump InputSetting: ");
    sp<INotifyCallback> spCb = pMsgCb.promote();
    MY_LOGI("Set notify callback:%p", spCb.get());
    MY_LOGI("FrameCount:%zu", vFrame.size());
    for (size_t i = 0; i < vFrame.size(); ++i)
    {
        MY_LOGI("\tframeNo:%d ", vFrame[i].frameNo);
        for (size_t j = 0; j < vFrame[i].vVendorInfo.size(); ++j)
        {
            VendorInfo* info = const_cast<VendorInfo*>(&(vFrame[i].vVendorInfo[j]));
            MY_LOGI("\tVendor:%" PRId64 , (*info).vendorMode);
            String8 str;
            if ( !(*info).appIn.empty() ) {
                str = String8::format("\t\t[AppIn]:");
                for (size_t k = 0; k < (*info).appIn.size(); ++k)
                    str += String8::format("[%d]", (*info).appIn[k]);
                MY_LOGI("%s", str.string());
            }
            //
            if ( !(*info).appOut.empty() ) {
                str = String8::format("\t\t[AppOut]:");
                for (size_t k = 0; k < (*info).appOut.size(); ++k)
                    str += String8::format("[%d]", (*info).appOut[k]);
                MY_LOGI("%s", str.string());
            }
            //
            /*str = String8::format("\n\t\t[WorkIn]:");
            for (size_t k = 0; k < (*info).workIn.size(); ++k)
                str += String8::format(" %" PRId64, (*info).workIn[k]);
            MY_LOGI("%s", str.string());
            //
            str = String8::format("\n\t\t[WorkOut]:");
            for (size_t k = 0; k < (*info).workOut.size(); ++k)
                str += String8::format(" %" PRId64, (*info).workOut[k]);
            MY_LOGI("%s", str.string());*/
            //
            if ( !(*info).inFormat.empty() ) {
                str = String8::format("\t\t[WorkInFormat]:");
                for (size_t k = 0; k < (*info).inFormat.size(); ++k)
                    str += String8::format(" %d", (*info).inFormat[k]);
                MY_LOGI("%s", str.string());
            }
            //
            if ( !(*info).outFormat.empty() ) {
                str = String8::format("\t\t[WorkOutFormat]:");
                for (size_t k = 0; k < (*info).outFormat.size(); ++k)
                    str += String8::format(" %d", (*info).outFormat[k]);
                MY_LOGI("%s", str.string());
            }
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
IVendorManager::WorkingBufferHandle::
~WorkingBufferHandle()
{
    if ( mBuffer.heap.get() && mOwner.get()) {
        MetaItem v;
        mOwner->releaseWorkingBuffer(mOpenId, v, mBuffer);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
IVendorManager::WorkingBufferHandle::
getStatus()
{
    return mBuffer.bufferStatus;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
IVendorManager::WorkingBufferHandle::
updateStatus(MINT32 status)
{
    MINT32 dirMask = BUFFER_IN|BUFFER_OUT;
    if ( status&dirMask ) {
        mBuffer.bufferStatus &= ~dirMask;
    }
    mBuffer.bufferStatus |= status;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageBufferHeap>
IVendorManager::WorkingBufferHandle::
getHeap()
{
    if ( !mBuffer.heap.get() && mOwner.get() ) {
        mOwner->acquireWorkingBuffer(mOpenId, mSetting, /*out*/mBuffer);
        MY_LOGD("%s", mOwner->getName());
    }
    MY_LOGD("heap: %p", mBuffer.heap.get());
    return mBuffer.heap;
}

/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
IVendorManager::WorkingBufferHandle::
getImageBuffer()
{
    IImageBuffer* buf = nullptr;
    if ( !mBuffer.heap.get() && mOwner.get() ) {
        mOwner->acquireWorkingBuffer(mOpenId, mSetting, /*out*/mBuffer);
    }
    if ( mBuffer.heap.get() ) buf = mBuffer.heap->createImageBuffer();

    return buf;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamInfo>
IVendorManager::WorkingBufferHandle::
getStreamInfo()
{
    if ( !mBuffer.heap.get() && mOwner.get() ) {
        mOwner->acquireWorkingBuffer(mOpenId, mSetting, /*out*/mBuffer);
    }
    return mBuffer.streamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
String8
IVendorManager::WorkingBufferHandle::
log()
{
    String8 str;
    if ( mOwner.get() )
        str = String8::format("working buffer from [%" PRId64 "]%s : ",
                                    mOwner->getVendorMode(), mOwner->getName());
    else
        str = String8::format("working buffer : ");

    if ( mBuffer.heap.get() )
        str += mBuffer.log();
    return str;
}


/******************************************************************************
 *
 ******************************************************************************/
IVendorManager::UserBufferHandle::
~UserBufferHandle()
{
    if ( mBuffer.heap.get()) {
        MY_LOGD("req:%d streamId: %#" PRIx64 " heap:%p", mRequestNo, mBuffer.streamInfo->getStreamId(), mBuffer.heap.get());
        MetaSet r;
        Vector<BufferItem> bList;
        bList.push_back(mBuffer);
        mOwner->onDataReceived(false, mRequestNo, r, bList);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
IVendorManager::UserBufferHandle::
updateStatus(MINT32 status)
{
    mBuffer.bufferStatus |= status;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageBufferHeap>
IVendorManager::UserBufferHandle::
getHeap()
{
    return mBuffer.heap;
}

/******************************************************************************
 *
 ******************************************************************************/
IImageBuffer*
IVendorManager::UserBufferHandle::
getImageBuffer()
{
    IImageBuffer* buf = nullptr;
    if ( mBuffer.heap.get() ) buf = mBuffer.heap->createImageBuffer();

    return buf;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamInfo>
IVendorManager::UserBufferHandle::
getStreamInfo()
{
    return mBuffer.streamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
String8
IVendorManager::UserBufferHandle::
log()
{
    String8 str = String8::format("outside buffer : ");
    if ( mBuffer.heap.get() )
        str += mBuffer.log();
    return str;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
IVendorManager::UserBufferHandle::
getStatus()
{
    return mBuffer.bufferStatus;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
IVendorManager::MetaHandle::
updateStatus(MINT32 status)
{
    mStatue |= status;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::
onRequestEnd( MINT64 key )
{
    Mutex::Autolock _l(mCombinationLock);
    if ( mCombinationMap.indexOfKey(key) >= 0 ) {
        Vector<sp<RequestHandler>> *list = &(mCombinationMap.editValueFor(key));
        for (size_t i = 0; i < (*list).size(); ++i) {
            for ( size_t j = 0; j < (*list).editItemAt(i)->mParam.vCombine.size(); ++j) {
                sp<IVendor> v = (*list).editItemAt(i)->mParam.vCombine[j].pVendor;
                if( !v.get() ) continue;
                MetaItem m;
                v->sendCommand(
                    INFO_CANCEL,
                    (*list).editItemAt(i)->mParam.openId, m,
                    (*list).editItemAt(i)->mParam.frameNo, (*list).editItemAt(i)->mParam.frameNo, nullptr);
            }
            if( (*list).editItemAt(i)->getState() == RequestHandler::STATE_DONE ||
                (*list).editItemAt(i)->getState() == RequestHandler::STATE_ERROR ) {
                (*list).removeAt(i);
                break;
            }
        }
        if ( mCombinationMap.editValueFor(key).empty() ) {
#if ENABLE_DEBUG_LOG
            MY_LOGD( "Remove key:%" PRId64 " from %zu requests", key, mCombinationMap.size() );
#endif
            mCombinationMap.removeItem(key);
        }
    }
    else {
        MY_LOGE("key not found.");
    }
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
VendorManagerImp::RequestHandler::
onDispatch()
{
    if ( mParam.vCombine.isEmpty() ) {
        CHECK_BOOL( !mParam.list.isEmpty() );
        RequestHandler::VendorPackage *p = &(mParam.list.editItemAt(0));
        //
        // prepare metadata callback
        MetaSet result;
        result.appMeta = *(((*p).meta.appResult)->getMetadata());
        result.halMeta = *(((*p).meta.halResult)->getMetadata());
        //
        // prepare buffer callback
        Vector< BufferItem > bufResult;
        { // working buffer out
            CHECK_BOOL( (*p).buf.vOut.size() == 0 );
            for ( size_t i = 0; i < (*p).buf.vIn.size(); ++i ) {
                BufferItem item;
                item.bufferStatus = (*p).buf.vIn[i]->getStatus()|BUFFER_OUT;
                item.heap         = (*p).buf.vIn[i]->getHeap();
                item.streamInfo   = (*p).buf.vIn[i]->getStreamInfo();
                bufResult.push_back(item);
            }
        }
        //
        // try to get callback instance if exists
        sp<IVendorManager::IDataCallback> _cb = mParam.cb.promote();
        if (_cb.get()) {
            _cb->onDataReceived(true, mParam.frameNo, result, bufResult);
        }
        else {
#if ENABLE_DEBUG_LOG
            MY_LOGD("[%d] Promote IDataCallback fail, ignored", mParam.frameNo);
#endif
        }
        //
        mParam.list.clear();
        trans(STATE_DONE);
        android::sp<RequestLifeCtrl> pCtrl = mpCtrl.promote();
        if(!pCtrl.get()) MY_LOGE("RequestLifeCtrl is not exit.");
        else
            return pCtrl->onRequestEnd(mParam.key);
    }

    sp<IVendor> v = mParam.vCombine[0].pVendor;
    MINT64 next   = mParam.vCombine[0].vendorMode;
    RequestHandler::VendorPackage p = mParam.list.editItemAt(0);
    CHECK_BOOL( next == p.vendorMode );
    mParam.vCombine.removeAt(0);
    mParam.list.removeAt(0);

#if ENABLE_DEBUG_LOG
    if ( mEnableLog ) {
        MY_LOGD("[%d]Queue to [%" PRId64 "]%s", mParam.frameNo, v->getVendorMode(), v->getName());
        p.meta.dump();
        p.buf.dump();
    }
#endif

    status_t err = v->queue(mParam.openId, mParam.frameNo, this, p.buf, p.meta);
    MY_LOGW_IF(err != OK, "[%d]Queue to [%" PRId64 "]%s fail.", mParam.frameNo, v->getVendorMode(), v->getName());
    if ( err != OK && getState() == STATE_RUNNING ) {
        // onDispatch called by vendor, should callback to p2 node
        mParam.vCombine.clear();
        RequestHandler::VendorPackage p = mParam.list[mParam.list.size() - 1];
        mParam.list.clear();
        mParam.list.push_back(p);
        onDispatch();
    }

    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
void
IVendor::BufferParam::dump()
{
    struct Log
    {
        static  void dump(Vector<sp<BufferHandle> >& list) {
            for (size_t i = 0; i < list.size(); i++) {
                MY_LOGD("\t\t [%d][%p] %s", list[i]->getStrongCount(), list[i].get(), list[i]->log().string());
            }
        }
    };
    MY_LOGD("\t I:Image:");
    Log::dump(vIn);
    MY_LOGD("\t O:Image:");
    Log::dump(vOut);
}


/******************************************************************************
 *
 ******************************************************************************/
void
IVendor::MetaParam::dump()
{
    struct Log
    {
        static  void dump(Vector<sp<MetaHandle> >& list) {
            for (size_t i = 0; i < list.size(); i++) {
                MY_LOGD("\t\t [%d][%p] %s", list[i]->getStrongCount(), list[i].get(), list[i]->log().string());
            }
        }
        static  void dump(sp<MetaHandle> pHandle) {
            MY_LOGD("\t\t [%d][%p] %s", pHandle->getStrongCount(), pHandle.get(), pHandle->log().string());
        }
    };
    MY_LOGD("\t I:Meta:");
    Log::dump(appCtrl);
    Log::dump(halCtrl);
    Log::dump(vIn);
    //
    MY_LOGD("\t O:Meta:");
    Log::dump(appResult);
    Log::dump(halResult);
}
