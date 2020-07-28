/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_REQUESTFRAME_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_REQUESTFRAME_H_

// MTKCAM
#include <mtkcam/utils/std/common.h>
#include <mtkcam/pipeline/extension/IVendorManager.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

// AOSP
#include <utils/RefBase.h> // android::sp, android::wp

// STL
#include <memory> // std::memory
#include <mutex> // std::lock_guard
#include <vector> // std::vector

// [Compile RequestFrame with vtable or not]
//
//  W/o vtable that may reduce time for searching virtual table, may have a litte
//  performance speed up
#define _ENABLE_REQUESTFRAME_VTABLE_  0

#if defined(_RF_VIRTUAL)
#   error "Symbols has beed duplicated defined."
#endif

#if _ENABLE_REQUESTFRAME_VTABLE_
#   define  _RF_VIRTUAL     virtual
#else
#   define  _RF_VIRTUAL
#endif


using android::sp;
using android::wp;

namespace NSCam {
namespace plugin {

// * NOTICE: DO NOT use android::sp<IImageBuffer> to contain the IImageBuffer*
//           returns from RequestFrame::getImageBuffer or
//           RequestFrame::operator []. Or un-expected result may happend.
//
// RequestFrame is a frame-based praser for IVendor. While IVendor::queue has
// been invoked, caller can create a new RequestFrame with arguments from
// IVendor::queue. The image buffer(s) and metadata(s) will be parsed after
// constructed a RequestFrame. Caller can use operator [] to retrieve
// IImageBuffer* or IMetadata* by passing in eRequestImg or eRequestMeta.
//
//   auto pImg  = reqFrm[eRequestImg_FullSrc];
//   auto pMeta = reqFrm[eRequestMeta_InHalP1];
//
// Or to use function "getImageBuffer" or "getMetadata" to retrieve them.
//
//   auto pImg  = pReqFrm->getImageBuffer(eRequestImg_FullSrc);
//   auto pMeta = pReqFrm->getMetadata(eRequestMeta_InHalP1);
//
// If the image buffer or metadata doesn't exist, it returns nullptr.
//
// However, not only a parser, but also an object-oriented design for managing
// buffers lifetime and dispatching mechanism. All the resource will be released
// and dispatched to the next owner(says iVendor instance) while RequestFrame is
// being destroyed.
//
// One thing you have to know: caller has responsibility to set up the StreamID
// map for creating RequestFrame. The StreamID can be retrieved while invoking
// IVendor::get method. E.g.:
//
//      IVendorDeriv::get(
//              MINT32                      openId,
//              const InputInfo&            in,
//              FrameInfoSet&               out)
//      {
//          RequestFrame::StreamIdMap_Img idMap;
//
//          // generate a StreamID map for image buffers,
//          if (in.fullRaw.get()) {
//              idMap[RequestFrame::eRequestImg_FullSrc]
//                  = in.fullRaw->getStreamId();
//          }
//
//          if (in.resizedRaw.get()) {
//              idMap[RequestFram::eRequestImg_ResizedSrc]
//                  = in.resizedRaw->getStreamId();
//          }
//
//          // ...
//
//          // assume there's member named m_streamIdMapImage.
//          m_streamIdMapImage = std::move(idMap);
//      }
//
// Now we had the StreamID map of image buffer, we can create RequestFrame.
// By the way, we don't need StreamID of metadata now. So just gives a empty
// map.
//
//      IVendorDeriv::queue(
//              MINT32 const                openId,
//              MUINT32 const               requestNo,
//              wp<IVendor::IDataCallback>  cb,
//              BufferParam                 bufParam,
//              MetaParam                   metaParam)
//      {
//          std::shared_ptr<RequestFrame> pFrame(new RequestFrame(
//                  "callerName",
//                  bufParam,
//                  metaParam,
//                  cb,
//                  m_streamIdMapImage,
//                  RequestFrame::StreamIdMap_Meta()
//          ));
//
//          // retrieve IImageBuffer* from RequestFrame
//          IImageBuffer* pFullRaw = pFrame->getImageBuffer(RequestFrame::eRequestImg_FullSrc);
//          IImageBuffer* pResizedRaw = (*pFrame)[RequestFrame::eRequestImg_ResizedSrc];
//
//          if (pFullRaw == nullptr) {
//              MY_LOGE("Full size src img is NULL");
//          }
//          else {
//              MY_LOGD("full size=(%d,%d)",
//                  pFullRaw->getImgSize().w,
//                  pFullRaw->getImgSize().h);
//          }
//
//          if (pResizedRaw == nullptr) {
//              MY_LOGW("Resized src img is NULL");
//          }
//          else {
//              MY_LOGD("resized=(%d,%d)",
//                  pResizedRaw->getImgSize().w,
//                  pResizedRaw->getImgSize().h);
//          }
//
//          // saves pFrame to somewhere.
//          m_myRFrames.push_back(pFrame);
//  }
//
// Note: All methods in this class are reentrant.

class RequestFrame
{
//
// Enumerations
//
public:
    // Image buffers we care about
    enum eRequestImg
    {
        // Unknown (index = 0)
        eRequestImg_Unknown = 0,
        // source
        eRequestImg_FullSrc,
        eRequestImg_ResizedSrc,
        eRequestImg_LcsoRaw,
        eRequestImg_WorkingBufferIn,
        eRequestImg_ReservedSrc1,
        eRequestImg_ReservedSrc2,
        eRequestImg_ReservedSrc3,
        eRequestImg_ReservedSrc4,
            // out
        eRequestImg_FullOut,
        eRequestImg_ResizedOut,
        eRequestImg_ThumbnailOut,
        eRequestImg_PostviewOut,
        eRequestImg_WorkingBufferOut,
        eRequestImg_ReservedOut1,
        eRequestImg_ReservedOut2,
        eRequestImg_ReservedOut3,
        eRequestImg_ReservedOut4,
        // size
        eRequestImg_Size,
        // indexer
        eRequestImg_Src_Start       = eRequestImg_FullSrc,
        eRequestImg_Src_End         = eRequestImg_ReservedSrc4 + 1,
        eRequestImg_Src_Extra_Start = eRequestImg_ReservedSrc1,
        eRequestImg_Src_Extra_End   = eRequestImg_Src_End,
        eRequestImg_Out_Start       = eRequestImg_FullOut,
        eRequestImg_Out_End         = eRequestImg_ReservedOut4 + 1,
        eRequestImg_Out_Extra_Start = eRequestImg_ReservedOut1,
        eRequestImg_Out_Extra_End   = eRequestImg_Out_End,
        eRequestImg_All             = eRequestImg_Size,
    };

    // Metadata we care about
    enum eRequestMeta
    {
        eRequestMeta_Unknown = 0,   // Unknown (0)
        eRequestMeta_InAppRequest,  // App control (1)
        eRequestMeta_InHalP1,       // P1 output (2)
        eRequestMeta_InFullRaw,     // not sure what this is (3)
        eRequestMeta_InResizedRaw,  // not sure what this is (4)
        eRequestMeta_OutAppResult,  // result App metadata, will be dispatched to the next vendor. (5)
        eRequestMeta_OutHalResult,  // result App metadata, will be dispatched to the next vendor. (6)
        // all & size
        eRequestMeta_All,
        eRequestMeta_Size = eRequestMeta_All,
    };


//
//  Fixed size container (contiguous memory chunk, for cache-frendly)
//
protected:
    // for samll size elements, to make them in a contiguous memory
    // may speed up performance due to morden CPU cache
    template<class T, size_t SIZE>
    class FixedVector : public std::vector<T>
    {
        typedef typename std::vector<T> _V;
        // marks modifiers as forbidden (private)
        using _V::assign;
        using _V::push_back;
        using _V::pop_back;
        using _V::insert;
        using _V::erase;
        using _V::swap;
        using _V::emplace;
        using _V::emplace_back;
    public:
        FixedVector() { _V::resize(SIZE, T()); }
        inline void clear() { for (auto& itr : *(dynamic_cast<_V*>(this))) itr = T(); }
    };


//
// Typedefs
//
public:
    typedef FixedVector<StreamId_T, eRequestImg_Size>    StreamIdMap_Img;
    typedef FixedVector<StreamId_T, eRequestMeta_Size>   StreamIdMap_Meta;


// These two classes, FrameImage and FrameMetadata are not public, only be used
// by class RequestFrame
protected:
    // Enum for representing input or output image buffer or metadata.
    enum eDirection
    {
        eDirection_In = 0,
        eDirection_Out,
    };

    // Base class for an image buffer of a BufferParam, this means that there're
    // several FrameImage objects within a BufferParam
    struct FrameImage
    {
        eRequestImg                         reqId;
        size_t                              index; // debug usage.
        bool                                bError;
        eDirection                          inout;
        const char*                         callerName;
        MUINT32                             orientation;
        sp<IImageBuffer>                    imageBuffer;
        bool                                bBufferLocked;
        sp<IImageBufferHeap>                imageBufferHeap;
        sp<IVendorManager::BufferHandle>    handle;
        // constructor / destructor
        FrameImage(eDirection inout);
        _RF_VIRTUAL ~FrameImage();
    };

    // For metadata
    struct FrameMetadata
    {
        eRequestMeta                        reqId;
        size_t                              index;
        bool                                bError;
        eDirection                          inout;
        const char*                         callerName;
        IMetadata*                          pMetadata;
        sp<IVendorManager::MetaHandle>      handle;
        // constructor / destructor
        FrameMetadata(eDirection inout);
        _RF_VIRTUAL ~FrameMetadata();
    };


//
// Methods
//
public:
    // Dispatch this frame to the next owner, all the image buffers and metadatas
    // will be released. Usually caller doesn't have to invoke this method, because
    // this method will be invoked while RequestFrame is being destroyed.
    //  @return                 Dispatched OK or not. If duplicated invoked, it
    //                          returns false to tell caller the frame has been
    //                          already dispatched.
    _RF_VIRTUAL bool            dispatch();

    // Get the frame index, which is assigned pass in constructor of RequestFrame.
    // Default is 0.
    //  @return                 The frame index which is set from constructor.
    _RF_VIRTUAL size_t          getFrameIndex() const;

    // To get the image buffer without extend it's life time, which mean, DO NOT
    // to use android::sp to capture the returned IImageBuffer.
    //  @param reqId            The ID of the image buffer.
    //  @return                 The pointer of IImageBuffer, DO NOT use
    //                          android::sp to capture it.
    //  @note                   To use android::sp to capture the returned
    //                          IImageBuffer may cause UB.
    _RF_VIRTUAL IImageBuffer*   getImageBuffer(eRequestImg reqId) const;

    // To get the metadata.
    //  @param reqId            The ID of the metadata.
    //  @return                 The pointer of the metadata.
    _RF_VIRTUAL IMetadata*      getMetadata(eRequestMeta reqId) const;

    // To get the orientation information of the image buffer. The orientation
    // info is provided from IImageStreamInfo.
    // If the buffer doesn't exist, this method returns 0.
    //  @param reqId            The ID of the image buffer.
    //  @return                 The buffer orientation.
    //
    //  @retval 0                   Degree 0
    //  @retval eTransform_ROT_90   Degree 90
    //  @retval eTransform_ROT_180  Degree 180
    //  @retval eTransform_ROT_270  Degree 270
    _RF_VIRTUAL MUINT32         getOrientation(eRequestImg reqId) const;

    // To get the request number of this RequestFrame. The request number is
    // given while creating RequestFrame.
    //  @return                 Request number.
    _RF_VIRTUAL MUINT32         getRequestNo() const;

    // To check if the frame has been marked as error frame or not. If passes in
    // eRequestImg_All, return true if any frame has been marked as error.
    //  @param reqId            The ID of the image buffer.
    //  @return                 It the frame, or any frame(eRequestImg_All) has
    //                          been marked as error.
    _RF_VIRTUAL bool            isError(eRequestImg reqId) const;

    // To check if the metadata has been marked as error or not. If passes in
    // eRequestMeta_All, return true if any metadata has been marked as error.
    //  @param reqId            The ID of the metadata.
    //  @return                 It the metadata, or any metadata(eRequestMeta_All)
    //                          has been marked as error.
    _RF_VIRTUAL bool            isError(eRequestMeta reqId) const;

    // To check if the IImageBuffer has been locked or not, If passes in
    // eRequestImg_All, this method returns yes if any IImageBuffer has been
    // locked.
    //  @param reqId            The ID of the image buffer.
    //  @return                 If the specific image buffer has been locked,
    //                          returns true. Otherwise false.
    _RF_VIRTUAL bool            isImageBufferLocked(eRequestImg reqId) const;

    // To lock the IImageBuffer with the specific usage. The correct usage may
    // speed up performance.
    //  @param reqId            The ID of the image buffer.
    //  @param usage            The buffer usage, see also eBUFFER_USAGE_SW_READ_OFTEN...
    //  @return                 If locked successfully, returns true.
    _RF_VIRTUAL bool            lockImageBuffer(eRequestImg reqId, MINT usage);

    // To unlock the IImageBuffer.
    //  @param reqId            The ID of the image buffer.
    //  @return                 If unlocked successfully, returns true.
    _RF_VIRTUAL bool            unlockImageBuffer(eRequestImg reqId);

    // To release the image buffer.
    //  @param reqId            The ID of the image buffer to release.
    //  @note                   Caller may want to partial release image buffer,
    //                          it's ok if this method is never invoked, because
    //                          image buffer will be automatically released while
    //                          FrameImage is being destroyed.
    _RF_VIRTUAL void            releaseImageBuffer(eRequestImg reqId);

    // To release the metadata.
    //  @param reqId            The ID of the metadata to release.
    //  @note                   Caller may want to partical release metadata
    //                          (usually not), it's ok if this method is never
    //                          invoked, because metadata will be automatically
    //                          released while FrameMetadata is being destroyed.
    _RF_VIRTUAL void            releaseMetadata(eRequestMeta reqId);

    // To mark the image buffer as an error buffer.
    //  @param reqId            The ID of the image buffer.
    //  @param bError           Mark as error or not.
    //  @note                   While releasing buffer, the error flag will be
    //                          set if marked as error.
    _RF_VIRTUAL void            markError(eRequestImg reqId, bool bError = true);

    // To mark the metadata as an error metadata.
    //  @param reqId            The ID of the metadata.
    //  @param bError           Mark as error or not.
    //  @note                   While releasing metadata, the error flag will be
    //                          set if marked as error.
    _RF_VIRTUAL void            markError(eRequestMeta reqId, bool bError = true);

    // To mark this RequestFrame as a broken RequestFrame.
    //  @param                  Mark to broken or not.
    //  @note                   All the FrameImage and FrameMetadata won't be
    //                          marked as error, caller has to mark them if
    //                          necessary.
    _RF_VIRTUAL void            markBroken(bool bBroken = true);

    // To check if this frame is a broken frame. The RequestFrame will be marked
    // as a broken frame if any condition matched:
    //   1. parse image buffer failed.
    //   2. unexpected NULL pointers of IImageBuffer or IImageBufferHeap.
    //   3. no App control metadata.
    //   4. no Hal control metadata.
    //  @note                   Some buffers or metadatas may be marked as error
    //                          buffer or metadata but won't be marked as a broken
    //                          frame. To use isError to check more completely.
    _RF_VIRTUAL bool            isBroken() const;

    // Get the open id of current FrameRequest.
    _RF_VIRTUAL MINT32          getOpenId() const;

//
// Protected Methods, can be inherited (must define _ENABLE_REQUESTFRAME_VTABLE_ to 1).
//
protected:
    // To parse the IVendor::BufferParam to RequestFrame::FrameImage. This
    // method is invoked while creating RequestFrame (called in constructor).
    //  @param callerName       Caller name.
    //  @param streamIdMapImg   Constant reference of StreamID map of images.
    //  @param bufHandle        BufferHandle from BufferParam queued by
    //                          IVendor::queue.
    //  @param inout            Represents input image or output.
    //  @param index            The index of RequestFrame (for debug usage).
    //  @return                 True for ok. Otherwise it means this RequestFrame
    //                          is supposed to be a broken frame.
    _RF_VIRTUAL bool            parseImageBuffer(
                                    const char*                 callerName,
                                    const StreamIdMap_Img&      streamIdMapImg,
                                    const sp<IVendorManager::BufferHandle> bufHandle,
                                    eDirection                  inout,
                                    size_t                      index
                                    );

    // To parse the IVendor::MetaParam to RequestFrame::FrameMetadata. This
    // method is invoked while creating RequestFrame (called in constructor).
    //  @param callerName       Caller name.
    //  @param streamIdMapMeta  Constant reference of StreamID map of metadata,
    //                          but now we don't need this. (since 2016 Sep.)
    //  @param metaHandle       MetaParam frm IVendor::queue.
    //  @param inout            Represents input metadata or output.
    //  @param reqId            Since the metadata is parsed by the specific
    //                          structure, we don't need StreamID map but we have
    //                          to speicify the eRequestMeta enum while parsing
    //                          metadata.
    //  @param index            The index of RequestFrame (for debug usage).
    //  @return                 True for ok. Otherwise it means this RequestFrame
    //                          is supposed to be a broken frame.
    _RF_VIRTUAL bool            parseMetadata(
                                    const char*                 callerName,
                                    const StreamIdMap_Meta&     streamIdMapMeta,
                                    const sp<IVendorManager::MetaHandle> metaHandle,
                                    eDirection                  inout,
                                    eRequestMeta                reqId,
                                    size_t                      index
                                    );


//
// Constructor
//
public:
    // To create a RequestFrame, caller needs to prepare StreamId map for
    // image buffers.
    //  @param callerName               Caller name, for lock,unlock buffers.
    //  @param bufParam                 The argument from IVendor::queue.
    //  @param metaParam                The argument from IVendor::queue.
    //  @param cb                       The argument from IVendor::queue.
    //  @param streamIdMapImg           StreamID map for image buffers.
    //  @param streamIdMapMeta          StreamID map for metadata(s), unuse now.
    //  @param requestNo                Request number, as an unique key.
    //  @param index                    Debug usage, for displaying debug message.
    //  @note                           streamIdMapMeta is not used now.
    RequestFrame(
            const MINT32                openId,
            const char*                 callerName,
            const IVendor::BufferParam& bufParam,
            const IVendor::MetaParam&   metaParam,
            const wp<IVendor::IDataCallback>&   cb,
            const StreamIdMap_Img&      streamIdMapImg,
            const StreamIdMap_Meta&     streamIdMapMeta,
            MUINT32                     requestNo = 0,
            size_t                      index = 0 // not defined is ok
            );

    // Copy-constructor is forbidden, because a frame cannot be dispatched
    // twice
    RequestFrame(const RequestFrame&) = delete;

    // Move-constructor is OK.
    RequestFrame(RequestFrame&&) = default;

    // Release RequestFrame. All resource will be released automatically.
    _RF_VIRTUAL ~RequestFrame();


//
// Operators
//
public:
    _RF_VIRTUAL IImageBuffer*           operator [] (eRequestImg reqId);
    _RF_VIRTUAL const IImageBuffer*     operator [] (eRequestImg reqId) const;

    _RF_VIRTUAL IMetadata*              operator [] (eRequestMeta reqId);
    _RF_VIRTUAL const IMetadata*        operator [] (eRequestMeta reqId) const;

    // Copy assignment is forbidden, see also copy constructor.
    _RF_VIRTUAL RequestFrame&           operator = (const RequestFrame& other) = delete;

    _RF_VIRTUAL RequestFrame&           operator = (RequestFrame&& other);


//
// Attributes
//
#if _ENABLE_REQUESTFRAME_VTABLE_
protected:
#else
private:
#endif
    bool        m_bBroken;      // true if this request frame is a broken frame
    MUINT32     m_requestNo;    // request number
    size_t      m_index;        // for debug usage
    MINT32      m_openId;       // frame open id

    // describes frame images
    FixedVector<std::shared_ptr<FrameImage>, eRequestImg_Size>
        m_frameImages;

    // describes metadata
    FixedVector<std::shared_ptr<FrameMetadata>, eRequestMeta_Size>
        m_frameMetadatas;

    // data callback should be invoked after all resource has been released
    android::wp<IVendor::IDataCallback> m_pDataCallback;

public:
    static int  m_dbgLevel;     // debug log level

}; // class RequestFrame
}; // namespace NSCam
}; // namespace plugin
#endif//_MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_REQUESTFRAME_H_
