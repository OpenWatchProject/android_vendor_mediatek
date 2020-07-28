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
 * CHARGE PAID BY RECEVIER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_EXTENSION_IVENDORMANAGER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_EXTENSION_IVENDORMANAGER_H_
//
#include <inttypes.h>
//
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>
#include <mtkcam/pipeline/stream/IStreamInfo.h>
using namespace NSCam::v3;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace plugin {

enum BufferStatus_T {
    BUFFER_IN          = 1 << 0,// source buffer for vendor
    BUFFER_OUT         = 1 << 1,// dst buffer vendor need to output
    BUFFER_EMPTY       = 1 << 2,
    BUFFER_WRITE_OK    = 1 << 3,
    BUFFER_WRITE_ERROR = 1 << 4,
    BUFFER_READ_OK     = 1 << 5,
    BUFFER_READ_ERROR  = 1 << 6,
    BUFFER_DST         = 1 << 7,
};

enum IN_OUT_FORMAT {
    FORMAT_RAW = 0,
    FORMAT_YUV = 1,
};

/**
 * pipeline frame constructor
 * for vendor life control
 * type : MINT64
 */
enum CALLER_ID {
    CALLER_PIPELINEMODEL_DEFAULT = 1 << 0,
    //
    CALLER_FLOWCONTROL_DEFAULT   = 1 << 1,
    CALLER_FLOWCONTROL_FEATURE   = 1 << 2,
    //
    CALLER_SHOT_GENERAL          = 1 << 3,
    CALLER_SHOT_SMART            = 1 << 4,
    //
    CALLER_SHOT_MAX_NUMBER       = 1 << 5,
};


enum IVENDOR_STREAM_ID {
    IVENDOR_STREAMID_IMAGE_PIPE_WORKING = 0x10000UL,
    //
    IVENDOR_STREAMID_META_PIPE_NULL,
};

enum IVENDOR_BUFFER_TYPE {
    BUFFER_IN_PLACE  = 1,
    BUFFER_WORKING   = 2,
    BUFFER_UNKNOWN   = 3,
    BUFFER_NO_WORKING_OUTPUT = 4, // only dst buffer out
};

/**
 *  The information of IVendor::sendCommand.
 *
 *  Notice that, each vendor can define its own IVENDOR_QUERY_INFO with value
 *  greater than INFO_USER_DEFINE_START, the values smaller than
 *  INFO_USER_DEFINE_START are reserved for IVendor mechanism.
 */
enum IVENDOR_QUERY_INFO {
    /* An unknown info, no need to handle */
    INFO_UNKNOWN                = 0,

    /**
     * // TODO: add description
     * arg1 : this vendor need to do or not
     * arg2 : BUFFER_IN_PLACE / BUFFER_WORKING
     */
    INFO_PROCESS_AND_BUFFER     = 1,

    /**
     * Request IVendor to cancel the certain request frame.
     *
     * arg1 : request number
     */
    INFO_CANCEL                 = 2,

    /* The values which are greater than this enumeration are free for customization */
    INFO_USER_DEFINE_START      = 0x40000UL,
};

enum NOTIFY_MSG
{
    MSG_ON_NEXT_CAPTURE_READY,
    MSG_ON_SHUTTER_CALLBACK
};

class INotifyCallback
    : public virtual android::RefBase
{
public:
    virtual android::status_t   onMsgReceived(
                                    MINT32 cmd, /*NOTIFY_MSG*/
                                    MINT32 arg1 = 0,
                                    MINT32 arg2 = 0,
                                    void*  arg3 = 0
                                ) = 0;
};

struct MetaSet
{
    IMetadata appMeta;
    IMetadata halMeta;
};

struct MetaItem
{
    MetaSet setting;
    // extend
    android::Vector<IMetadata> others;
};

struct InputInfo
{
    android::Vector<MINT64>                         combination;
    //
    IMetadata                                       appCtrl;
    IMetadata                                       halCtrl;
    android::Vector<MetaItem>                       otherMeta;
    // size...
    android::sp<IImageStreamInfo>                   fullRaw;
    android::sp<IImageStreamInfo>                   lcsoRaw;
    android::sp<IImageStreamInfo>                   resizedRaw;
    android::Vector<android::sp<IImageStreamInfo> > vYuv;
    android::sp<IImageStreamInfo>                   postview;
    android::sp<IImageStreamInfo>                   jpegYuv;
    android::sp<IImageStreamInfo>                   thumbnailYuv;
    android::sp<IImageStreamInfo>                   jpeg;
    //
    android::sp<IImageStreamInfo>                   workingbuf; // no need to fill
    //
    MBOOL                                           isZsdMode;
    MUINT32                                         sensorMode;

    void dump() const;
    //
    InputInfo()
        : isZsdMode(MFALSE)
        , sensorMode(SENSOR_SCENARIO_ID_UNNAMED_START)
    {};
    ~InputInfo() = default;
};

struct VendorInfo
{
    MINT64 vendorMode;
    //
    // i/o for each vendor
    android::Vector<MINT32> appIn; // ?
    android::Vector<MINT32> appOut;
    //
    /*android::Vector<MINT64> workIn;
    android::Vector<MINT64> workOut;*/
    //
    // working buffer in/out format
    android::Vector<MINT32> inFormat;
    android::Vector<MINT32> outFormat;
};

struct FrameInfo
{
    MINT32 frameNo;
    //
    IMetadata curAppControl;
    IMetadata curHalControl;
    //
    android::KeyedVector< MINT64, VendorInfo >   vVendorInfo;
};

struct InputSetting
{
    android::Vector<FrameInfo>   vFrame;
    //
    // ext data
    android::wp<INotifyCallback> pMsgCb;

    void dump() const;
};

struct Capability
{
    MINT64 vendorMode;
    // working buffer in/out format support
    android::Vector<MINT32> inFormat;
    android::Vector<MINT32> outFormat;
    //
    MINT32 inCategory;
    MINT32 outCategory;
    //
    MINT32 bufferType; // in-place / working

    Capability()
        : vendorMode(0)
        , inCategory(FORMAT_YUV)
        , outCategory(FORMAT_RAW)
        , bufferType(BUFFER_UNKNOWN)
    {}
};


struct OutputInfo
{
    // key vendor mode
    android::KeyedVector<MINT64, Capability> table;
    //
    // setting for captured frame
    android::Vector<MetaItem> settings;
    //
    // setting for dummy frame (optional for 3A stable before capture)
    android::Vector<MetaItem> dummySettings;
    //
    // setting for back delayed frame (optional for 3A stable befor resume preview)
    android::Vector<MetaItem> delayedSettings;
    //
    MINT32 inCategory;
    MINT32 outCategory;
    //
    // captured frames count
    MINT32 frameCount;
    //
    // front dummy frames count (optional for 3A stable before capture)
    MINT32 dummyCount;
    //
    // back delayed frames count (optional for 3A stable before resume preview)
    MINT32 delayedCount;

    OutputInfo()
        : inCategory(FORMAT_YUV)
        , outCategory(FORMAT_RAW)
        , frameCount(1)
        , dummyCount(0)
        , delayedCount(0)
    {}
};

struct BufferItem
{
    MINT32                              bufferStatus;
    android::sp<IImageBufferHeap>       heap;
    android::sp<IImageStreamInfo>       streamInfo;
    //
    android::String8 log() const {
        return android::String8::format("[%#" PRIx64 "](0x%d %dx%d %p)",
            streamInfo->getStreamId(),
            streamInfo->getImgFormat(),
            streamInfo->getImgSize().w, streamInfo->getImgSize().h,
            heap.get()
        );
    }
};

class IVendor;
class IVendorManager
    : public virtual android::RefBase
{
public:     ////
    typedef android::status_t           status_t;

public:     ////
    class IDataCallback;
    friend class IVendor;

public:     ////
    static IVendorManager*  createInstance(
                                char const*  pcszName,
                                MINT32 const i4OpenId
                            );

    class MetaHandle : public virtual android::RefBase
    {
    public:
                            MetaHandle(IMetadata meta, android::String8 s)
                                : mStatue(BUFFER_EMPTY)
                                , mMetadata(meta)
                                , str(s)
                            {}
                            MetaHandle(android::String8 s)
                                : mStatue(BUFFER_EMPTY)
                                , str(s)
                            {}
                            ~MetaHandle() {};

        status_t            updateStatus(MINT32 status);
        MINT32              getStatus() { return mStatue; }
        IMetadata*          getMetadata() { return &mMetadata; }
        android::String8    log() { return str; }

    private:
        MINT32              mStatue;
        IMetadata           mMetadata;
        android::String8    str;
    };

    class BufferHandle : public virtual android::RefBase
    {
    public:
        virtual                                 ~BufferHandle() {};
        virtual status_t                        updateStatus(MINT32 status) = 0;
        virtual MINT32                          getStatus()                 = 0;
        virtual android::sp<IImageBufferHeap>   getHeap()                   = 0;
        virtual IImageBuffer*                   getImageBuffer()            = 0;
        virtual android::sp<IImageStreamInfo>   getStreamInfo()             = 0;

    public:
        virtual android::String8 log()                                      = 0;
    };

    class WorkingBufferHandle
        : public IVendorManager::BufferHandle
    {
    public:
        WorkingBufferHandle(android::sp<IVendor> owner, MINT32 openId, MetaItem setting, MINT32 initSate)
            : mOpenId(openId)
            , mSetting(setting)
            , mOwner(owner)
        {
            mBuffer.bufferStatus = initSate;
        }

        ~WorkingBufferHandle();
        //
        virtual status_t                        updateStatus(MINT32 status);
        virtual MINT32                          getStatus();
        virtual android::sp<IImageBufferHeap>   getHeap();
        virtual IImageBuffer*                   getImageBuffer();
        virtual android::sp<IImageStreamInfo>   getStreamInfo();
        virtual android::String8                log();

    private:
        MINT32                                  mOpenId;
        //
        BufferItem                              mBuffer;
        MetaItem                                mSetting;
        android::sp<IVendor>                    mOwner;
    };

    class UserBufferHandle
        : public BufferHandle
    {
    public:
        typedef android::sp<IVendorManager::IDataCallback> spCb;

    public:
        UserBufferHandle( BufferItem buffer, spCb owner, MUINT32 requestNo)
            : mRequestNo(requestNo)
            , mBuffer(buffer)
            , mOwner(owner)
        {
            mBuffer.bufferStatus = BUFFER_EMPTY;
        }

        ~UserBufferHandle();
        //
        virtual status_t                        updateStatus(MINT32 status);
        virtual MINT32                          getStatus();
        virtual android::sp<IImageBufferHeap>   getHeap();
        virtual IImageBuffer*                   getImageBuffer();
        virtual android::sp<IImageStreamInfo>   getStreamInfo();
        virtual android::String8                log();

    private:
        MUINT32                                 mRequestNo;
        //
        BufferItem                              mBuffer;
        spCb                                    mOwner;
    };

public:     ////
    virtual                         ~IVendorManager() {};

    virtual char const*             getName( MINT64 vendorMode)             = 0;

    virtual int32_t                 getOpenId() const                       = 0;

/**
 * API for constructing pipeline frame.
 * Caller get setting requirement from callee and set necessory
 * information to callee.
 *
 * Potential users :
 *     PipelineModel_Default
 *     FlowControl
 *     Shot
 */
public:     ////
    /**
     * query information to construct pipeline frame.
     *
     * @param[in]  userID : caller's id
     * @param[in]  in     : user(pipeline model) send
     *                      current setting & stream configure size to each vendor
     * @param[out] out    : description for each frame's setting & vendor info
     *                      setting : app / hal control
     *                      vendor info : in/out format support, in-place / working buffer
     *
     * @return
     *     0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                get(
                                        MUINT64          userID,
                                        InputInfo&       in,
                                        /*out*/
                                        OutputInfo&      out
                                    )                                       = 0;

    /**
     * Set information to vendor
     *
     * @param[in]  userID : caller's id
     * @param[in]  in     : description for each frame's setting & vendor info
     *                      setting : app / hal control
     *                      vendor info : in/out format
     *                                    i/o setting
     *
     * @return
     *     0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                set(
                                        MUINT64              userID,
                                        const InputSetting&  in
                                    )                                       = 0;

    /**
     * Register vnedor by user.
     * @param  userID
     * @param  pVendor
     * @return         : 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                registerItem(
                                        MUINT64                 userID,
                                        android::sp<IVendor>    pVendor
                                    )                                       = 0;
    /**
     * Register vnedors by user.
     * If the Vendor already exists in the VendorManager of userID,
     * it won't be created but reuse it.
     *
     * @param  userID
     * @param[in]  in  : description for each frame's setting & vendor info
     * @return         : 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                registerItems(
                                        MUINT64        userID,
                                        InputInfo&     in
                                    )                                       = 0;

    /**
     * Remove vendor by user.
     * @param  vendorMode
     * @param  userID
     * @return            : 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                removeItem(
                                        MINT64         vendorMode,
                                        MUINT64        userID
                                    )                                       = 0;
/**
 * API for buffer/result communication.
 *
 * Potential users :
 *     hwnode
 */
public:     ////
    /**
     * Callback buffer or metadata
     */
    class IDataCallback
        : public virtual android::RefBase
    {
    public:
        /**
         * Result callback.
         *
         * @param  requestNo
         * @param  isLastCb  : indicate this callback is the last or not
         * @param  result    : result metadata, partial update.
         * @param  buffers   : result buffers.
         * @return           : 0 indicates success; non-zero indicates an error code.
         */
        virtual status_t            onDataReceived(
                                        MBOOL   const        isLastCb,
                                        MUINT32 const        requestNo,
                                        MetaSet              result,
                                        android::Vector< BufferItem > buffers
                                    )                                       = 0;
    };

public:     ////
    /**
     * Non-blocking function call.
     *
     * @param  requestNo
     * @param  cb
     * @param  srcBuffer : working buffer provide by vendor. use acquireWorkingBuffer()
     * @param  setting   : control metadata.
     * @return           : 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                queue(
                                        MUINT32 const                   requestNo,
                                        android::sp<IDataCallback>      cb,
                                        android::Vector< BufferItem >   srcBuffer,
                                        MetaItem                        setting
                                    )                                       = 0;

    /**
     * Acquire working buffer.
     * Working buffer as source for vendor.
     *
     * @param  userID  : caller's ID
     * @param  setting : app/hal control metadata
     * @param  buf     : working buffer
     * @return         : 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                acquireWorkingBuffer(
                                        MUINT64            userID,
                                        MetaItem           setting,
                                        /*output*/
                                        BufferItem&        buf
                                    )                                       = 0;

    /**
     * Release working buffer.
     * Working buffer as source for MDP.
     *
     * @param  userID  : caller's ID
     * @param  setting : input metadata
     * @param  buf     : working buffer
     * @return         : 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                releaseWorkingBuffer(
                                        MUINT64            userID,
                                        MetaItem           setting,
                                        BufferItem&        buf
                                    )                                       = 0;

    /**
     * Flush.
     * All related vendor will flush.
     *     *
     * @param  userID : caller's ID.
     * @return        : 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                beginFlush( MUINT64 userID )            = 0;
    virtual status_t                endFlush( MUINT64 userID )              = 0;

public:     //// debug / extension
    /**
     * Send command to specific vendor.
     */
    virtual status_t                sendCommand(
                                        MINT32 cmd,
                                        MINT64 vendor,
                                        MetaItem& meta,
                                        MINT32& arg1, MINT32& arg2,
                                        void*  arg3
                                    )                                       = 0;

    /**
     * Dump related vendor status.
     */
    virtual status_t                dump( MUINT64 userID )                  = 0;

};

/**
 * store IVendorManager
 */
namespace NSVendorManager
{
    void                            clear();
    void                            add(int32_t deviceId, IVendorManager* pManager);
    void                            remove(int32_t deviceId);

    android::sp<IVendorManager>     get(int32_t deviceId);

};  //namespace NSVendorManager

class IVendor
    : public virtual android::RefBase
{
public:
    typedef android::status_t               status_t;
    typedef IVendorManager::BufferHandle    BufferHandle;
    typedef IVendorManager::MetaHandle      MetaHandle;

public:     ////
    static android::sp<IVendor>     createInstance(
                                        char const*                pcszName,
                                        MINT32 const               i4OpenId,
                                        MINT64 const               vendorMode
                                    );

public:     //// structure

public:     ////
    /**
     * onDispatch     : [must] dispatch to another vendor or p2 node
     *
     * notice: vendor should call onDispatch() after buffer & meta of same frame are released.
     */
    class IDataCallback
        : public virtual android::RefBase
    {
    public:
        virtual status_t            onDispatch()                            = 0;
    };

public:
    struct FrameInfoSet
    {
        // captured frames count
        MINT32 frameCount;
        //
        // front dummy frames count (optional for 3A stable before capture)
        MINT32 dummyCount;
        //
        // back delayed frames count (optional for 3A stable before resume preview)
        MINT32 delayedCount;
        //
        // key vendor mode
        Capability table;
        //
        // setting for captured frame
        android::Vector<MetaItem> settings;
        //
        // setting for dummy frame (optional for 3A stable before capture)
        android::Vector<MetaItem> dummySettings;
        //
        // setting for delayed frame (optional for 3A stable before resume preview)
        android::Vector<MetaItem> delayedSettings;

        FrameInfoSet()
            : frameCount(1)
            , dummyCount(0)
            , delayedCount(0)
        {}
    };

public:
    struct BufferParam
    {
        android::Vector<android::sp<BufferHandle> > vIn;
        android::Vector<android::sp<BufferHandle> > vOut;

        void dump();
    };

    struct MetaParam
    {
        /*input*/
        android::sp<MetaHandle>                     appCtrl;
        android::sp<MetaHandle>                     halCtrl;
        android::Vector<android::sp<MetaHandle> >   vIn;
        /*output*/
        android::sp<MetaHandle>                     appResult;
        android::sp<MetaHandle>                     halResult;

        void dump();
    };

public:     ////
    virtual                         ~IVendor() {};

    virtual char const*             getName()                               = 0;

    virtual MINT32                  getOpenId() const                       = 0;

    virtual MINT64                  getVendorMode() const                   = 0;

public:     //// construct setting.
    /**
     * query information to construct pipeline frame.
     *
     * @param[in]  openId
     * @param[in]  in
     * @param[out] out
     *
     * @return
     *     0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                get(
                                        MINT32           openId,
                                        const InputInfo& in,
                                        FrameInfoSet&    out
                                    )                                       = 0;

    /**
     * Set information to vendor
     *
     * @param[in]  openId
     * @param[in]  in
     * @return
     *     0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                set(
                                        MINT32              openId,
                                        const InputSetting& in
                                    )                                       = 0;

public:     //// for user.
    /**
     * Non-blocking function call.
     */
    virtual status_t                queue(
                                        MINT32  const       openId,
                                        MUINT32 const       requestNo,
                                        android::wp<IVendor::IDataCallback> cb,
                                        BufferParam         bufParam,
                                        MetaParam           metaParam
                                    )                                       = 0;

    /**
     * Acquire working buffer.
     * Working buffer as source for vendor.
     *
     * @param  openId  : open camera's id
     * @param  setting : input metadata
     * @param  buf     : working buffer
     * @return         : 0 indicates success;
     *                   manager will acquire from next vendor if return non-zero.
     */
    virtual status_t                acquireWorkingBuffer(
                                        MINT32             openId,
                                        const MetaItem&    setting,
                                        /*output*/
                                        BufferItem&        buf
                                    )                                       = 0;

    /**
     * Release working buffer.
     * Working buffer as source for MDP.
     *
     * @param  openId  : open camera's id
     * @param  setting : input metadata
     * @param  buf     : working buffer
     * @return         : 0 indicates success; non-zero indicates an error code.
     */
    virtual status_t                releaseWorkingBuffer(
                                        MINT32             openId,
                                        const MetaItem&    setting,
                                        BufferItem&        buf
                                    )                                       = 0;

    /**
     * Flush.
     * Collee should return all buffer to caller.
     */
    virtual status_t                beginFlush( MINT32 openId )             = 0;
    virtual status_t                endFlush( MINT32 openId )               = 0;

    /**
     * Send command to specific vendor.
     */
    virtual status_t                sendCommand(
                                        MINT32 cmd,
                                        MINT32 openId,
                                        MetaItem& meta,
                                        MINT32& arg1, MINT32& arg2,
                                        void*  arg3
                                    )                                       = 0;

public:     //// debug
    /**
     * Dump current status.
     */
    virtual status_t                dump( MINT32 openId )                   = 0;

};

};  //namespace plugin
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_EXTENSION_IVENDORMANAGER_H_
