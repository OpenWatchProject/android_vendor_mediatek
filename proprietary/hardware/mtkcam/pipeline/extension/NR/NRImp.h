/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_EXTENSION_NR_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_EXTENSION_NR_H_

#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <future>
#include <vector>
#include <thread>
#include <mtkcam/pipeline/extension/Vendor.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/pipeline/utils/streaminfo/ImageStreamInfo.h>
/* SWNR */
#include <mtkcam/aaa/ICaptureNR.h>
typedef NSCam::NSIoPipe::MCropRect MCropRect;
typedef NSCam::NSIoPipe::QParams QParams;
typedef NSCam::NSIoPipe::NSPostProc::INormalStream INormalStream;
using namespace NSCam;
#include "../BaseVendor.h"
#include "../utils/VendorUtils.h"
#define SUPPORT_ABF              (1)
#define SUPPORT_EIS              (1)
#define SUPPORT_EIS_MV           (0)
#define FORCE_EIS_ON             (SUPPORT_EIS  && (0))
/******************************************************************************
 *
 ******************************************************************************/
class AbfAdapter;

namespace NS3Av3{
    class IHal3A;
}
namespace NSCam {
namespace plugin {

class NRvendorImp
    : public BaseVendor
    , public NRvendor
{
public:
                                    NRvendorImp(
                                        char const*  pcszName,
                                        MINT32 const i4OpenId,
                                        MINT64 const vendorMode
                                    );

    virtual                         ~NRvendorImp();

    virtual char const*             getName();

    virtual int32_t                 getOpenId() const { return mOpenId; }

    virtual MINT64                  getVendorMode() const { return mMode; }

public:     //// construct setting.
    virtual status_t                get(
                                        MINT32                     openId,
                                        InputInfo const&           in,
                                        FrameInfoSet&              out
                                    );

    virtual status_t                set(
                                        MINT32                     openId,
                                        InputSetting const&        in
                                    );

public:     //// for user.
    virtual status_t                queue(
                                        MINT32  const              openId,
                                        MUINT32 const              requestNo,
                                        wp<IVendor::IDataCallback> cb,
                                        BufferParam                bufParam,
                                        MetaParam                  metaParam
                                    );

    virtual status_t                beginFlush( MINT32 openId );

    virtual status_t                endFlush( MINT32 openId );

    virtual status_t                sendCommand(
                                        MINT32 cmd,
                                        MINT32 openId,
                                        MetaItem& meta,
                                        MINT32& arg1, MINT32& arg2,
                                        void*  arg3
                                    );

public:     //// debug
    virtual status_t                dump( MINT32 openId );

public:     //// MNR cb
    static MVOID                    pass2CbFunc(QParams& rParams);

protected:  ////
    MVOID                           handleDeque(QParams& rParams);

    MERROR                          doMnr(MUINT32 const requestNo);

    MERROR                          doSwnr(MUINT32 const requestNo);

#if SUPPORT_ABF
    MERROR                          doAbf(MUINT32 const requestNo);
#endif

    // 3rdPartyFaceBeauty+++
    MERROR                          do3rdFB(MUINT32 const requestNo);
    // 3rdPartyFaceBeauty---

    MVOID                           waitForIdle();

    MBOOL                           onDequeRequest();

    MBOOL                           onProcessFuture();

    sp<v3::Utils::ImageStreamInfo>             createImageStreamInfo(
                                        char const*         streamName,
                                        StreamId_T          streamId,
                                        MUINT32             streamType,
                                        size_t              maxBufNum,
                                        size_t              minInitBufNum,
                                        MUINT               usageForAllocator,
                                        MINT                imgFormat,
                                        MSize const&        imgSize,
                                        MUINT32             transform
                                    );

    MBOOL                           decideNrMode(
                                        MetaItem  const& meta,
                                        MUINT8& nr_type,
                                        MINT32& iso
                                    );

    MERROR                          createImageBufferAndLock(
                                        sp<IImageBufferHeap> const& rpImageBufferHeap,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        const MUINT usage = 0 // 0 --> use default
                                    );

    MBOOL                           queryNrThreshold(
                                        int& hw_threshold,
                                        int& swnr_threshold
                                    );

    MERROR                          getCropInfo(
                                        IMetadata *const inApp,
                                        IMetadata *const inHal,
                                        MBOOL const isResized,
                                        Cropper::CropInfo &cropInfo
                                    ) const;

    MERROR                          dumpBuffer(
                                        sp<IImageBuffer> pImgBuf,
                                        const char* str
                                    );

    MERROR                          convertColorFormatMdp(
                                        IImageBuffer* pSrc,
                                        IImageBuffer* pDst
                                    );


protected:  ////

    MBOOL                           isEISOn(IMetadata *const inApp) const ;

    MVOID                           queryCropRegion(
                                        IMetadata *const inApp,
                                        IMetadata *const inHal,
                                        MBOOL const isEisOn,
                                        MRect &cropRegion
                                    ) const;


#if SUPPORT_ABF
    AbfAdapter*                     getAbfAdapter();
#endif

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

    MBOOL                           queryEisRegion(
                                        IMetadata *const inHal,
                                        eis_region &region
                                    ) const;
    //
    void *                          getTuningFromNvram(
                                        MUINT32 openId,
                                        MINT32 magicNo,
                                        MINT32 scen
                                    ) const;
    //
    struct myparam_t
    {
        MUINT32                                 requestNo;
        MUINT32                                 iso;
        MUINT8                                  nr_type;
        MINT32                                  openId;
        android::wp<IVendor::IDataCallback>     cb;
        MetaParam                               metaParam;
        BufferParam                             bufParam;
        NSIoPipe::NSPostProc::INormalStream *   pPipe;
        NS3Av3::IHal3A *                        p3A;
        std::vector< android::sp<IImageBuffer> >     collectLockedImageBuffer;
        void                                    dump();
    };

    MBOOL                           createPipe(myparam_t* param);

    MVOID                           destroyPipe(myparam_t* param);



    typedef enum {
        NR_TYPE_NONE = 0,
        NR_TYPE_MNR,
        NR_TYPE_SWNR,
        NR_TYPE_IGNORE, // ignore all features of NRImp (SWNR/MNR/3rd-party algos...)
    } NR_TYPE_T;

    typedef enum {
        SCEN_DEFAULT = 0,
        SCEN_MFLL,
        SCEN_HDR
    } SCEN_T;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32                                      mOpenId;
    MINT64                                      mMode;
    MBOOL const                                 mbEnableLog;
    // MNR
    MUINT32                                     muEnqueCnt;
    MUINT32                                     muDequeCnt;
    MINT32                                      mnStreamTag;
    // Working buffer
    MSize                                       mRawSize;
    //
    android::sp<IImageStreamInfo>               mJpegYuv;
    android::sp<IImageStreamInfo>               mThumbnailYuv;
    //
    MRect                                       mActiveArray;
    android::KeyedVector< MUINT32 , myparam_t > mvRunningQueue;
    mutable Mutex                               mLock;
    mutable Condition                           mAsyncCond;

    ISwNR *                                     mpSwnr;
#if SUPPORT_ABF
    AbfAdapter *                                mpAbf;
#endif
protected: // thread
    mutable Mutex                               mFutureLock;
    mutable Condition                           mFutureCond;
    std::vector< std::future<MERROR> >          mvFutures;
    volatile MBOOL                              mbRequestExit;
    std::future<void>                           mThread;

    MUINT32                                     muNRDebug;
    MUINT32                                     muDumpBuffer;
    MINT32                                      mForceNRType;
    MINT32                                      mForceCloseAbf;
    MBOOL                                       mbIsMfll;

// 3rdPartyFaceBeauty+++
protected: // 3rd party FaceBeauty
    mutable Mutex                              m3rdFbWorkPoolLock;
    sp<WorkPool>                               m3rdFbWorkPool;
// 3rdPartyFaceBeauty---

};

} // namespace plugin
} // namespace NSCam

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_V3_EXTENSION_NR_H_
