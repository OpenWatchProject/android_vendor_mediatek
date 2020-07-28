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

#define LOG_TAG "MtkCam/DefaultAdapter"
//
#include "MyUtils.h"
//
#include <mtkcam/utils/fwk/MtkCamera.h>
//
#include <mtkcam/def/PriorityDefs.h>
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
using namespace NSCam::Utils;
//
#include <inc/ImgBufProvidersManager.h>
#include <mtkcam/middleware/v1/IParamsManager.h>

//
#include <mtkcam/middleware/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
//
#include "inc/v3/DefaultAdapter.h"
using namespace NSDefaultAdapter;
//
#include <sys/prctl.h>
//
#include <MTKDngOp.h>
//
#include <system/camera_metadata.h>
//
// for debug dump
#include <cutils/properties.h>
#include <inc/CamUtils.h>
//
#include <mtkcam/aaa/IDngInfo.h>
//
#include <mtkcam/utils/hw/HwInfoHelper.h>

#include <mtkcam/feature/effectHalBase/ICallbackClientMgr.h>


using namespace NSCamHW;


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
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
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata const* metadata,
    MUINT32 const tag,
    T & rVal
)
{

    if( metadata == NULL ) {
        //MY_LOGE("metadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = metadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}


template<typename T>
inline MBOOL
getMetadataArray(IMetadata const* metadata, MINT32 entry_tag, T* array, MUINT32 size)
{
    IMetadata::IEntry entry = metadata->entryFor(entry_tag);
    if (entry.tag() != IMetadata::IEntry::BAD_TAG &&
        entry.count() == size)
    {
        for (MUINT32 i = 0; i < size; i++)
        {
            *array++ = entry.itemAt(i, Type2Type< T >());
        }
        return MTRUE;
    }
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Error (CAMERA_MSG_ERROR)
//
//  Arguments:
//      ext1
//          [I] extend argument 1.
//
//      ext2
//          [I] extend argument 2.
//
bool
CamAdapter::
onCB_Error(
    int32_t ext1,
    int32_t ext2
)
{
    MY_LOGW("CAMERA_MSG_ERROR %d %d", ext1, ext2);
    doNotifyCallback(CAMERA_MSG_ERROR, ext1, ext2);
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
namespace {
struct ShutterThread : public Thread
{
protected:  ////                Data Members.
    sp<CamMsgCbInfo>            mpCamMsgCbInfo;
    int32_t                     mi4PlayShutterSound;

public:
    ShutterThread(
        sp<CamMsgCbInfo> pCamMsgCbInfo,
        int32_t i4PlayShutterSound
    )
        : Thread()
        , mpCamMsgCbInfo(pCamMsgCbInfo)
        , mi4PlayShutterSound(i4PlayShutterSound)
    {}

    // Good place to do one-time initializations
    status_t
    readyToRun()
    {
        ::prctl(PR_SET_NAME, "ShutterThread", 0, 0, 0);
        //
        //
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_SHOTCB;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        CAM_LOGD(
            "[ShutterThread] policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%x, 0x%x)"
            , expect_policy, policy, expect_priority, priority
        );
        return OK;
    }

private:
    bool
    threadLoop()
    {
        CAM_LOGD("(%d)[ShutterThread] +", ::gettid());
#if 1   //defined(MTK_CAMERA_BSP_SUPPORT)
        mpCamMsgCbInfo->mNotifyCb(MTK_CAMERA_MSG_EXT_NOTIFY, MTK_CAMERA_MSG_EXT_NOTIFY_SHUTTER, mi4PlayShutterSound, mpCamMsgCbInfo->mCbCookie);
#else
        mpCamMsgCbInfo->mNotifyCb(CAMERA_MSG_SHUTTER, 0, 0, mpCamMsgCbInfo->mCbCookie);
#endif
        CAM_LOGD("(%d)[ShutterThread] -", ::gettid());
        return  false;  // returns false, the thread will exit upon return.
    }
};
}; // namespace


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Shutter (CAMERA_MSG_SHUTTER)
//
//      Invoking this callback may play a shutter sound.
//
//  Arguments:
//      bPlayShutterSound
//          [I] Play a shutter sound if ture; otherwise play no sound.
//
//      u4CallbackIndex
//          [I] Callback index. 0 by default.
//              If more than one shutter callback must be invoked during
//              captures, for example burst shot & ev shot, this value is
//              the callback index; and 0 indicates the first one callback.
//
bool
CamAdapter::
onCB_Shutter(
    bool const bPlayShutterSound,
    uint32_t const /*u4CallbackIndex*/,
    uint32_t const  u4ShotMode
)
{
    sp<ZipImageCallbackThread> pThread = mpZipCallbackThread.promote();
    if  ( pThread.get() == 0 )
    {
        MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
        return false;
    }

    bool isMsgTypeEnabled = msgTypeEnabled(CAMERA_MSG_SHUTTER);
    uint32_t shotMod = pThread->getShotMode();
    bool isSpecialShotEnable = shotMod == eShotMode_ZsdHdrShot ||
                               shotMod == eShotMode_ZsdMfllShot ||
                               shotMod == eShotMode_MfllShot ||
                               shotMod == eShotMode_FaceBeautyShotCc ||
                               shotMod == eShotMode_BMDNShot ||
                               shotMod == eShotMode_MFHRShot ||
                               shotMod == eShotMode_ZsdShot ||
                               shotMod == eShotMode_NormalShot; // for fast P1 drain (workaround)

    MY_LOGD("isMsgTypeEnabled:%d, playShutter:%d, shotMod: %d, isSpecialShotEnable: %d",
                isMsgTypeEnabled, bPlayShutterSound, shotMod, isSpecialShotEnable);

    if  ( isMsgTypeEnabled || isSpecialShotEnable)
    {
        ZipImageCallbackThread::callback_data callbackData;
        //
        if ( pThread->isExtCallback() )
        {
            callbackData.bMtkExt = true;
        }
        else
        {
            callbackData.bMtkExt = false;
        }
        //
        callbackData.type = ZipImageCallbackThread::callback_type_shutter;
        callbackData.spCamMsgCbInfo = mpCamMsgCbInfo;
        callbackData.i4PlayShutterSound = bPlayShutterSound;
        callbackData.u4ShotMode = u4ShotMode;
        pThread->addCallbackData(&callbackData);
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Postview for Client (CAMERA_MSG_POSTVIEW_FRAME)
//
//  Arguments:
//      i8Timestamp
//          [I] Postview timestamp
//
//      u4PostviewSize
//          [I] Postview buffer size in bytes.
//
//      puPostviewBuf
//          [I] Postview buffer with its size = u4PostviewSize
//
bool
CamAdapter::
onCB_PostviewClient(
    int64_t const   i8Timestamp,
    uint32_t const  u4PostviewSize,
    uint8_t const*  puPostviewBuf,
    uint32_t const  u4CbId
)
{
    MY_LOGD("+ timestamp(%lld), size/buf=%d/%p", i8Timestamp, u4PostviewSize, puPostviewBuf);

    camera_memory* pmem = mpCamMsgCbInfo->mRequestMemory(-1, u4PostviewSize, 1, mpCamMsgCbInfo->mCbCookie);
    if( pmem )
    {
        ::memcpy(pmem->data, puPostviewBuf, u4PostviewSize);
        //
        sp<ZipImageCallbackThread> pThread = NULL;
        //
        pThread = mpZipCallbackThread.promote();
        if  ( pThread.get() == 0 )
        {
            MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
            pmem->release(pmem);
            return false;
        }
        //
        ZipImageCallbackThread::callback_data callbackData =
        {
            ZipImageCallbackThread::callback_type_postview,
            pmem,
            0,
            false
        };
        pThread->addCallbackData(&callbackData);
        //
        pThread = 0;
        pmem=0;
    }
    else
    {
        MY_LOGW("pmem is NULL, mpCamMsgCbInfo->mRequestMemory failed, postview callback skipped");
    }

    MY_LOGD("-");

    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Postview for Client (CAMERA_MSG_POSTVIEW_FRAME)
//
//  Arguments:
//      i8Timestamp
//          [I] Postview timestamp
//
//      pPostviewBuf
//          [I] Postview IImageBuffer
//
bool
CamAdapter::
onCB_PostviewClient(
    int64_t const   i8Timestamp,
    NSCam::IImageBuffer const *pPostviewBuf,
    uint32_t const u4BufOffset4Ts /* = 0 */,
    uint32_t const u4BgServ /* = 0 */,
    uint32_t const  u4CbId
)
{
    /* In this method, we check if using BackgroundService first, if yes, send postview via MMSDK */
    if (MMSDK_CALLBACK_EN_POSTVIEW && u4BgServ) {
        MY_LOGD("send PostviewClient via MMSDK, buf offset(%u) will be discard " \
            "since MMSDK will insert it automatically", u4BufOffset4Ts);
        return ICallbackClientMgr::getInstance()->onCB_PostviewClient(
                    i8Timestamp,
                    pPostviewBuf,
                    u4CbId
                    );
    }

    /* if compress timestamp into data, the leading N bytes is for store of timestamp (used only 8 bytes) */
    /* where N is u4BufOffset4Ts */
    uint32_t  u4PostviewSize = u4BufOffset4Ts;
    for ( size_t i = 0; i < pPostviewBuf->getPlaneCount(); ++i ) {
        u4PostviewSize += pPostviewBuf->getBufSizeInBytes(i);
    }

    camera_memory* pmem = mpCamMsgCbInfo->mRequestMemory(-1, u4PostviewSize, 1, mpCamMsgCbInfo->mCbCookie);
    if( pmem )
    {
        uint32_t  u4PostviewSize_plane = u4BufOffset4Ts;

        /* saves timestamp if requested */
        if (u4BufOffset4Ts > 0) {
            if (__builtin_expect( sizeof(int64_t) >  u4BufOffset4Ts, false )) {
                MY_LOGF("offset(%u) is samller than data is going to be saved (%zu byte)",
                        u4BufOffset4Ts, sizeof(int64_t));
                *(volatile uint32_t*)(0) = 0xDEADFEED;
            }
            int64_t *pTimeStamp = reinterpret_cast<int64_t*>(pmem->data);
            *pTimeStamp = i8Timestamp;

            MY_LOGD("inserted timestamp(%" PRId64 ") at %p, offset until %p",
                    i8Timestamp,
                    (void*)pmem->data, (MUINT8*)pmem->data + u4PostviewSize_plane
                   );
        }

        for ( size_t i = 0; i < pPostviewBuf->getPlaneCount(); ++i ) {
            MY_LOGD("postview[%d] size_plane:%u, addr:%p, sizeInBytes:%d",
                    i, u4PostviewSize_plane, (MUINT8*)pPostviewBuf->getBufVA(i), pPostviewBuf->getBufSizeInBytes(i));
            ::memcpy((MUINT8*)pmem->data + u4PostviewSize_plane, (MUINT8*)pPostviewBuf->getBufVA(i), pPostviewBuf->getBufSizeInBytes(i));
            u4PostviewSize_plane += pPostviewBuf->getBufSizeInBytes(i);
        }

        sp<ZipImageCallbackThread> pThread = NULL;
        //
        pThread = mpZipCallbackThread.promote();
        if  ( pThread.get() == 0 )
        {
            MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
            pmem->release(pmem);
            return false;
        }
        //
        ZipImageCallbackThread::callback_data callbackData =
        {
            ZipImageCallbackThread::callback_type_postview,
            pmem,
            0,
            false
        };
        pThread->addCallbackData(&callbackData);
        //
        pThread = 0;
        pmem=0;
    }
    else
    {
        MY_LOGW("pmem is NULL, mpCamMsgCbInfo->mRequestMemory failed, postview callback skipped");
    }

    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Raw Image (MTK_CAMERA_MSG_EXT_DATA_RAW16)
//
//  Arguments:
//      pRawImgBuf
//          [I] Raw16 IImageBuffer
//
//
bool
CamAdapter::
onCB_Raw16Image(
    NSCam::IImageBuffer const *pRawImgBuf
)
{
    IImageBuffer const *scrBuffer = pRawImgBuf;
    uint32_t const  u4RawImgSize = pRawImgBuf->getBufSizeInBytes(0);
    uint8_t const*  puRawImgBuf = (MUINT8*)pRawImgBuf->getBufVA(0);

    MY_LOGD("mIsRaw16CBEnable=%d, addr:%p, size:%d",mIsRaw16CBEnable,puRawImgBuf,u4RawImgSize);

    //do raw16 data cb
    if  ( mIsRaw16CBEnable )//CAMERA_CMD_ENABLE_RAW16_CALLBACK
    {
        /** Start to simulate pack... **/

        MTKDngOp *MyDngop;
        DngOpResultInfo MyDngopResultInfo;
        DngOpImageInfo MyDngopImgInfo;

        /** Initialize **/
        MY_LOGD("Start to simulate pack...\n");
        //
        MINT32 rawBitDepth = 10;
        HwInfoHelper helper(getOpenId());
        if( ! helper.updateInfos() ) {
            MY_LOGE("cannot properly update infos");
            rawBitDepth = 10;
        }
        else
        {
            helper.getRecommendRawBitDepth(rawBitDepth);
        }
        //
        MUINT32 bitDstNum = MAKE_DngInfo(LOG_TAG, getOpenId())->getRawBitDepth();//query raw bit num from 3A dnginfo //rawBitDepth;
        //
        MyDngop = MTKDngOp::createInstance(DRV_DNGOP_UNPACK_OBJ_SW);
        MyDngopImgInfo.Width = scrBuffer->getImgSize().w;
        MyDngopImgInfo.Height = scrBuffer->getImgSize().h;
        MyDngopImgInfo.Stride_src = scrBuffer->getBufStridesInBytes(0);
        MyDngopImgInfo.Stride_dst = scrBuffer->getImgSize().w*2;
        MyDngopImgInfo.BIT_NUM = rawBitDepth;//rSensorParam.u4Bitdepth , buffer bit num
        //MyDngopImgInfo.BIT_NUM_DST = MAKE_DngInfo(LOG_TAG, getOpenId())->getRawBitDepth();//query raw bit num from 3A dnginfo
        MyDngopImgInfo.BIT_NUM_DST = ((rawBitDepth == 12) && (bitDstNum == 12))?12:10;
        MyDngopImgInfo.Bit_Depth = 10; //for pack

        // query buffer size
        MUINT32 buf_size = (scrBuffer->getImgSize().w*2*scrBuffer->getImgSize().h);
        MyDngopImgInfo.Buff_size = buf_size;


        // set buffer address
        unsigned char *p_buffer = new unsigned char[buf_size];
        MyDngopResultInfo.ResultAddr = p_buffer;

        // assign raw data
        MyDngopImgInfo.srcAddr = reinterpret_cast<void*>(const_cast<uint8_t*>(puRawImgBuf));

        MyDngop->DngOpMain((void*)&MyDngopImgInfo, (void*)&MyDngopResultInfo);

        MY_LOGD("after Unpack raw16: w (%d),h (%d),src-s (%d),dst-s (%d),buf-size(%d), BIT_NUM_DST(%d)",
                 scrBuffer->getImgSize().w, scrBuffer->getImgSize().h,
                 scrBuffer->getBufStridesInBytes(0), scrBuffer->getImgSize().w*2,
                 (scrBuffer->getImgSize().w*2*scrBuffer->getImgSize().h), MyDngopImgInfo.BIT_NUM_DST);

        //
        MyDngop->destroyInstance(MyDngop);
        //
        //debug dump raw buffer
        {
            // debug dump option
            // 0: disable, 1:dumpbuffer
            char value[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("vendor.debug.dngraw.dump", value, "0");
            MUINT32 muDumpFlag = ::atoi(value);
            //
            if(muDumpFlag)
            {
            #define dumppath "/sdcard/cameradump_dngraw"
                char fname[256];

                if(!makePath(dumppath,0660))
                {
                    MY_LOGE("makePath [%s] fail",dumppath);
                }

                sprintf(fname, "%s_%dx%d.raw",
                        dumppath,
                        scrBuffer->getImgSize().w,
                        scrBuffer->getImgSize().h
                        );
                saveBufToFile(fname, p_buffer, buf_size);
            #undef dumppath
            }
            //
        }
        //
        uint8_t* pImage = NULL;
        uint32_t const u4DataSize = buf_size + sizeof(uint32_t)*(1);//uint32_t const u4DataSize = u4RawImgSize + sizeof(uint32_t)*(1);
        //
        camera_memory* pmem = NULL;
        if  ( ! u4RawImgSize|| ! puRawImgBuf )
        {
            MY_LOGD("raw dummy callback");
            pmem = mpCamMsgCbInfo->mRequestMemory(-1, 1, 1, mpCamMsgCbInfo->mCbCookie);
        }
        else
        {
            MY_LOGD("do raw16 data cb");
            pmem = mpCamMsgCbInfo->mRequestMemory(-1, u4DataSize, 1, mpCamMsgCbInfo->mCbCookie);
            if  ( pmem )
            {
                uint32_t*const pCBData = reinterpret_cast<uint32_t*>(pmem->data);
                pCBData[0] = MTK_CAMERA_MSG_EXT_DATA_RAW16;
                pImage = reinterpret_cast<uint8_t*>(&pCBData[1]);
                ::memcpy(pImage, p_buffer, buf_size);//
                MY_LOGD("MTK_CAMERA_MSG_EXT_DATA_RAW16: %d", pCBData[0]);
                delete []p_buffer;
                p_buffer = NULL;
            }

        }

        if  ( pmem )
        {
            sp<ZipImageCallbackThread> pThread = NULL;
            //
            pThread = mpZipCallbackThread.promote();
            if  ( pThread.get() == 0 )
            {
                MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
                pmem->release(pmem);
                return false;
            }
            //
            ZipImageCallbackThread::callback_data callbackData =
            {
                ZipImageCallbackThread::callback_type_raw16,
                pmem,
                0,
                false
            };
            pThread->addCallbackData(&callbackData);
            //
            pThread = 0;
            pmem=0;
            //
        }
        else
        {
            MY_LOGW("pmem is NULL, mpCamMsgCbInfo->mRequestMemory failed, raw16 callback skipped");
        }
    }
    //
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Metadata (MTK_CAMERA_MSG_EXT_NOTIFY_METADATA_DONE)
//
//  Arguments:
//      i8Timestamp
//          [I] Raw image timestamp
//
//
bool
CamAdapter::
onCB_DNGMetaData(
       MUINTPTR const pDngMeta
)
{
    //
    //do raw16 data cb
    if  ( mIsRaw16CBEnable )//CAMERA_CMD_ENABLE_RAW16_CALLBACK
    {
        MY_LOGD("onCB_DNGMetaData-start");
        //
        IMetadata pDngResultAppMetaInfo;
        pDngResultAppMetaInfo = *(const_cast<IMetadata *>(reinterpret_cast<IMetadata const*>(pDngMeta)));
        //
        camera_metadata* pDstStaticMetadata;
        camera_metadata* pDstResultMetadata;
        //
        //metadata converter
        //static query from metadata provider
        {
            //staticmetainfo
            MINT32 i4BlackLevel[4];
            MINT32 i4whiteLevel;
            MRect  rActiveArray;
            MSize  rSadingMapSize;
            MSize  rPixelArraySize;
            MRational fColorTransform[9];
            MRational fCalibrationTransform[9];
            MRational fForwardMatrix[9];
            MUINT8 u1RefIlluminant1;
            MUINT8 uColorFilterArrangement;
            MUINT8 i4timestapsource;
            //
            sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
            if( ! pMetadataProvider.get() ) {
                MY_LOGE(" ! pMetadataProvider.get() ");
                return false;
            }

            IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();

            if( !getMetadataArray<MINT32>(&static_meta,
                        MTK_SENSOR_BLACK_LEVEL_PATTERN,
                        i4BlackLevel, 4)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_BLACK_LEVEL_PATTERN");
                return false;
                }

            if( !tryGetMetadata<MINT32>(&static_meta,
                        MTK_SENSOR_INFO_WHITE_LEVEL,
                        i4whiteLevel)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_INFO_WHITE_LEVEL");
                return false;
            }

            if( !tryGetMetadata<MRect>(&static_meta,
                        MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION,
                        rActiveArray)
             ) {
                MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
                return false;
            }

            if( !tryGetMetadata<MSize>(&static_meta,
                        MTK_LENS_INFO_SHADING_MAP_SIZE,
                        rSadingMapSize)
             ) {
                MY_LOGE("no static info: MTK_LENS_INFO_SHADING_MAP_SIZE");
                return false;
            }

            if( !tryGetMetadata<MSize>(&static_meta,
                        MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE,
                        rPixelArraySize)
             ) {
                MY_LOGE("no static info: MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE");
                return false;
            }

            if( !getMetadataArray<MRational>(&static_meta,
                        MTK_SENSOR_COLOR_TRANSFORM1,
                        fColorTransform,
                        9)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_COLOR_TRANSFORM1");
                return false;
            }

            if( !getMetadataArray<MRational>(&static_meta,
                        MTK_SENSOR_CALIBRATION_TRANSFORM1,
                        fCalibrationTransform,
                        9)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_CALIBRATION_TRANSFORM1");
                return false;
            }

            if( !getMetadataArray<MRational>(&static_meta,
                        MTK_SENSOR_FORWARD_MATRIX1,
                        fForwardMatrix,
                        9)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_FORWARD_MATRIX1");
                return false;
            }

            if( !tryGetMetadata<MUINT8>(&static_meta,
                        MTK_SENSOR_REFERENCE_ILLUMINANT1,
                        u1RefIlluminant1)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_REFERENCE_ILLUMINANT1");
                return false;
            }

            if( !tryGetMetadata<MUINT8>(&static_meta,
                        MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT,
                        uColorFilterArrangement)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT");
                return false;
            }

            if( !tryGetMetadata<MUINT8>(&static_meta,
                        MTK_SENSOR_INFO_TIMESTAMP_SOURCE,
                        i4timestapsource)
               ) {
                MY_LOGE("no static info: MTK_SENSOR_INFO_TIMESTAMP_SOURCE");
                return false;
            }
        //static
            size_t entryCount = 0;
            size_t dataCount = 0;
            //calculate
            entryCount = 5;
            dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_INT32, 13);//
            entryCount += 3;
            dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_RATIONAL, 27);//
            entryCount += 3;
            dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_BYTE, 3);//

            //allocate
            pDstStaticMetadata = ::allocate_camera_metadata(entryCount, dataCount);//

            //update
            {
                //
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_BLACK_LEVEL_PATTERN, &i4BlackLevel[0], 4);
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_INFO_WHITE_LEVEL, &i4whiteLevel, 1);
                //
                MINT32 activeArray[4];
                activeArray[0]=rActiveArray.p.x; //start X
                activeArray[1]=rActiveArray.p.y; //start y
                activeArray[2]=rActiveArray.s.w; //w
                activeArray[3]=rActiveArray.s.h; //H
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_INFO_PRE_CORRECTION_ACTIVE_ARRAY_SIZE, &activeArray[0], 4);
                //
                MINT32  sadingMapSize[2];
                sadingMapSize[0]=rSadingMapSize.w; //w
                sadingMapSize[1]=rSadingMapSize.h; //h
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_LENS_INFO_SHADING_MAP_SIZE, &sadingMapSize[0], 2);
                //
                MINT32  pixelArraySize[2];
                pixelArraySize[0]=rPixelArraySize.w; //w
                pixelArraySize[1]=rPixelArraySize.h; //h
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_INFO_PIXEL_ARRAY_SIZE, &pixelArraySize[0], 2);
                //
                camera_metadata_rational_t colorTransform[9];
                for(int i=0 ; i<9 ; i++)
                {
                    colorTransform[i].numerator=fColorTransform[i].numerator;
                    colorTransform[i].denominator=fColorTransform[i].denominator;
                }
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_COLOR_TRANSFORM1, &colorTransform[0], 9);
                //
                camera_metadata_rational_t calibrationTransform[9];
                for(int i=0 ; i<9 ; i++)
                {
                    calibrationTransform[i].numerator=fCalibrationTransform[i].numerator;
                    calibrationTransform[i].denominator=fCalibrationTransform[i].denominator;
                }
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_CALIBRATION_TRANSFORM1, &calibrationTransform[0], 9);
                //
                camera_metadata_rational_t forwardTransform[9];
                for(int i=0 ; i<9 ; i++)
                {
                    forwardTransform[i].numerator=fForwardMatrix[i].numerator;
                    forwardTransform[i].denominator=fForwardMatrix[i].denominator;
                }
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_FORWARD_MATRIX1, &forwardTransform[0], 9);
                //
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_REFERENCE_ILLUMINANT1, &u1RefIlluminant1, 1);
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, &uColorFilterArrangement, 1);
                add_camera_metadata_entry(pDstStaticMetadata,
                        ANDROID_SENSOR_INFO_TIMESTAMP_SOURCE, &i4timestapsource, 1);
                //
            }
            //Dynamicmetainfo
            MINT64 i8SensorExposureTime;
            MINT32 i4SensorSensitivity;
            MRational i4SensorNeutralColorPointNumerator[3];
            MFLOAT  fLensFocalLength;
            MFLOAT  fLensAperture;
            MDOUBLE dSensorNoiseProfile[8];
            MINT32 shadingMapSize = 4*(rSadingMapSize.w)*(rSadingMapSize.h);
            MFLOAT vecShadingMap[shadingMapSize];

            if( !tryGetMetadata<MINT64>(&pDngResultAppMetaInfo,
                        MTK_SENSOR_EXPOSURE_TIME,
                        i8SensorExposureTime)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_EXPOSURE_TIME");
                return false;
                }

            if( !tryGetMetadata<MINT32>(&pDngResultAppMetaInfo,
                        MTK_SENSOR_SENSITIVITY,
                        i4SensorSensitivity)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_SENSITIVITY");
                return false;
            }

            if( !getMetadataArray<MRational>(&pDngResultAppMetaInfo,
                        MTK_SENSOR_NEUTRAL_COLOR_POINT,
                        i4SensorNeutralColorPointNumerator,
                        3)
             ) {
                MY_LOGE("no static info: MTK_SENSOR_NEUTRAL_COLOR_POINT");
                return false;
            }

            if( !tryGetMetadata<MFLOAT>(&pDngResultAppMetaInfo,
                        MTK_LENS_FOCAL_LENGTH,
                        fLensFocalLength)
             ) {
                MY_LOGE("no static info: MTK_LENS_FOCAL_LENGTH");
                return false;
            }

            if( !tryGetMetadata<MFLOAT>(&pDngResultAppMetaInfo,
                        MTK_LENS_APERTURE,
                        fLensAperture)
             ) {
                MY_LOGE("no static info: MTK_LENS_APERTURE");
                return false;
            }
            if( !getMetadataArray<MDOUBLE>(&pDngResultAppMetaInfo,
                        MTK_SENSOR_NOISE_PROFILE,
                        dSensorNoiseProfile,
                        8)
              ) {
                MY_LOGE("no static info: MTK_SENSOR_NOISE_PROFILE");
                return false;
            }

            if( !getMetadataArray<MFLOAT>(&pDngResultAppMetaInfo,
                        MTK_STATISTICS_LENS_SHADING_MAP,
                        vecShadingMap,
                        shadingMapSize)
              ) {
                MY_LOGE("no static info: MTK_STATISTICS_LENS_SHADING_MAP");
                return false;
            }
            //dynamic
            {
                size_t entryCount = 0;
                size_t dataCount = 0;
                //calculate
                entryCount = 1;
                dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_INT64, 1);//
                entryCount += 1;
                dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_INT32, 1);//
                entryCount += 1;
                dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_RATIONAL, 3);//
                entryCount += 2;
                dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_FLOAT, 2);//
                entryCount += 1;
                dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_DOUBLE, 8);//
                entryCount += 1;
                dataCount += ::calculate_camera_metadata_entry_data_size(TYPE_FLOAT, (shadingMapSize));//

                //allocate
                pDstResultMetadata = ::allocate_camera_metadata(entryCount, dataCount);//

                //update
                {
                    //
                    add_camera_metadata_entry(pDstResultMetadata,
                            ANDROID_SENSOR_EXPOSURE_TIME, &i8SensorExposureTime, 1);
                    add_camera_metadata_entry(pDstResultMetadata,
                            ANDROID_SENSOR_SENSITIVITY, &i4SensorSensitivity, 1);
                    //
                    /*NOTE: This Tag will be checked from Android O*/
                    MINT32 tempBaselineE = 100;
                    add_camera_metadata_entry(pDstResultMetadata,
                        ANDROID_CONTROL_POST_RAW_SENSITIVITY_BOOST, &tempBaselineE, 1);
                    //
                    camera_metadata_rational_t neutralColorPoint[3];
                    for(int i=0 ; i<3 ; i++)
                    {
                        neutralColorPoint[i].numerator=i4SensorNeutralColorPointNumerator[i].numerator;
                        neutralColorPoint[i].denominator=i4SensorNeutralColorPointNumerator[i].denominator;
                    }
                    add_camera_metadata_entry(pDstResultMetadata,
                            ANDROID_SENSOR_NEUTRAL_COLOR_POINT, &neutralColorPoint[0], 3);
                    //
                    add_camera_metadata_entry(pDstResultMetadata,
                            ANDROID_LENS_FOCAL_LENGTH, &fLensFocalLength, 1);
                    add_camera_metadata_entry(pDstResultMetadata,
                            ANDROID_LENS_APERTURE, &fLensAperture, 1);
                    add_camera_metadata_entry(pDstResultMetadata,
                            ANDROID_SENSOR_NOISE_PROFILE, &dSensorNoiseProfile[0], 8);
                    //
                    MFLOAT vecLensShadingMap[shadingMapSize];
                    for(int i=0 ; i< (shadingMapSize) ; i++)
                    {
                        vecLensShadingMap[i]=vecShadingMap[i];
                    }
                    add_camera_metadata_entry(pDstResultMetadata,
                            ANDROID_STATISTICS_LENS_SHADING_MAP, &vecLensShadingMap[0], (shadingMapSize));
                }
            }
        }
        MY_LOGD("onCB_DNGMetaData convert done!");
        sp<ZipImageCallbackThread> pThread = NULL;
        //
        pThread = mpZipCallbackThread.promote();
        if  ( pThread.get() == 0 )
        {
            MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
            return false;
        }
        //
        ZipImageCallbackThread::callback_data callbackData;
        callbackData.type = ZipImageCallbackThread::callback_type_metadata_raw16;
        callbackData.pResult = pDstResultMetadata;
        callbackData.pCharateristic = pDstStaticMetadata;
        pThread->addCallbackData(&callbackData);
        //
        pThread = 0;
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Raw Image (CAMERA_MSG_RAW_IMAGE/CAMERA_MSG_RAW_IMAGE_NOTIFY)
//
//  Arguments:
//      i8Timestamp
//          [I] Raw image timestamp
//
//      u4RawImgSize
//          [I] Raw image buffer size in bytes.
//
//      puRawImgBuf
//          [I] Raw image buffer with its size = u4RawImgSize
//
bool
CamAdapter::
onCB_RawImage(
    int64_t const   i8Timestamp,
    uint32_t const  u4RawImgSize,
    uint8_t const*  puRawImgBuf
)
{
    MY_LOGD("timestamp(%lld), size/buf=%d/%p", i8Timestamp, u4RawImgSize, puRawImgBuf);
    //
    if  ( msgTypeEnabled(CAMERA_MSG_RAW_IMAGE_NOTIFY) )
    {
        MY_LOGD("CAMERA_MSG_RAW_IMAGE_NOTIFY");
        sp<ZipImageCallbackThread> pThread = NULL;
        //
        pThread = mpZipCallbackThread.promote();
        if  ( pThread.get() == 0 )
        {
            MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
            return false;
        }
        //
        ZipImageCallbackThread::callback_data callbackData =
        {
            ZipImageCallbackThread::callback_type_raw_notify,
            NULL,
            0,
            false
        };
        pThread->addCallbackData(&callbackData);
        //
        pThread = 0;
        return  true;
    }
    //
    if  ( msgTypeEnabled(CAMERA_MSG_RAW_IMAGE) )
    {
        MY_LOGD("CAMERA_MSG_RAW_IMAGE");
        if  ( ! u4RawImgSize || ! puRawImgBuf )
        {
            MY_LOGD("dummy callback");
            camera_memory* pmem = mpCamMsgCbInfo->mRequestMemory(-1, 1, 1, mpCamMsgCbInfo->mCbCookie);
            if  ( pmem )
            {
                sp<ZipImageCallbackThread> pThread = NULL;
                //
                pThread = mpZipCallbackThread.promote();
                if  ( pThread.get() == 0 )
                {
                    MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
                    pmem->release(pmem);
                    return false;
                }
                //
                ZipImageCallbackThread::callback_data callbackData =
                {
                    ZipImageCallbackThread::callback_type_raw,
                    pmem,
                    0,
                    false
                };
                pThread->addCallbackData(&callbackData);
                //
                pThread = 0;
                pmem=0;
            }
            else
            {
                MY_LOGW("pmem is NULL, mpCamMsgCbInfo->mRequestMemory failed, raw callback skipped");
            }
        }
        else
        {
            camera_memory* pmem = mpCamMsgCbInfo->mRequestMemory(-1, u4RawImgSize, 1, mpCamMsgCbInfo->mCbCookie);
            {
                ::memcpy(pmem->data, puRawImgBuf, u4RawImgSize);
                if  ( pmem )
                {
                    sp<ZipImageCallbackThread> pThread = NULL;
                    //
                    pThread = mpZipCallbackThread.promote();
                    if  ( pThread.get() == 0 )
                    {
                        MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
                        pmem->release(pmem);
                        return false;
                    }
                    //
                    ZipImageCallbackThread::callback_data callbackData =
                    {
                        ZipImageCallbackThread::callback_type_raw,
                        pmem,
                        0,
                        false
                    };
                    pThread->addCallbackData(&callbackData);
                    //
                    pThread = 0;
                    pmem=0;
                }
                else
                {
                    MY_LOGW("pmem is NULL, mpCamMsgCbInfo->mRequestMemory failed, raw callback skipped");
                }
            }
        }
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of Compressed Image (CAMERA_MSG_COMPRESSED_IMAGE)
//
//      [Compressed Image] = [Header] + [Bitstream],
//      where
//          Header may be jpeg exif (including thumbnail)
//
//  Arguments:
//      i8Timestamp
//          [I] Compressed image timestamp
//
//      u4BitstreamSize
//          [I] Bitstream buffer size in bytes.
//
//      puBitstreamBuf
//          [I] Bitstream buffer with its size = u4BitstreamSize
//
//      u4HeaderSize
//          [I] Header size in bytes; header may be jpeg exif.
//
//      puHeaderBuf
//          [I] Header buffer with its size = u4HeaderSize
//
//      u4CallbackIndex
//          [I] Callback index. 0 by default.
//              If more than one compressed callback must be invoked during
//              captures, for example burst shot & ev shot, this value is
//              the callback index; and 0 indicates the first one callback.
//
//      fgIsFinalImage
//          [I] booliean value to indicate whether it is the final image.
//              true if this is the final image callback; otherwise false.
//              For single captures, this value must be true.
//
bool
CamAdapter::
onCB_CompressedImage(
    int64_t const   i8Timestamp,
    uint32_t const  u4BitstreamSize,
    uint8_t const*  puBitstreamBuf,
    uint32_t const  u4HeaderSize,
    uint8_t const*  puHeaderBuf,
    uint32_t const  u4CallbackIndex,
    bool            fgIsFinalImage,
    uint32_t const  msgType,
    uint32_t const  u4ShotMode
)
{
    bool ret = true;
    camera_memory* image = NULL;
    uint8_t* pImage = NULL;
    uint32_t const u4DataSize = u4HeaderSize + u4BitstreamSize + sizeof(uint32_t)*(1+1);
    sp<ZipImageCallbackThread> pThread = NULL;
    //
    MY_LOGD(
        "timestamp(%lld), bitstream:size/buf=%d/%p, header:size/buf=%d/%p, index(%d), IsFinalImage(%d)",
        i8Timestamp, u4BitstreamSize, puBitstreamBuf, u4HeaderSize, puHeaderBuf, u4CallbackIndex, fgIsFinalImage
    );
    //
    if  ( ! msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE) )
    {
        MY_LOGW("msgTypeEnabled=%#x", msgTypeEnabled(0xFFFFFFFF));
        sp<IParamsManager> pParamsMgr = getParamsManager();
        if( pParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE) == MtkCameraParameters::CAMERA_MODE_NORMAL )
        {
            MY_LOGW("compressed image callback ignored");
            goto lbExit;
        }
    }
    //
    pThread = mpZipCallbackThread.promote();
    if  ( pThread.get() == 0 )
    {
        MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
        ret = false;
        goto lbExit;
    }
    //
    if ( pThread->isExtCallback() )
    {
        image = mpCamMsgCbInfo->mRequestMemory(-1, u4DataSize, 1, mpCamMsgCbInfo->mCbCookie);
        if  ( image )
        {
            uint32_t*const pCBData = reinterpret_cast<uint32_t*>(image->data);
            pCBData[0] = msgType;
            pCBData[1] = u4CallbackIndex;
            pImage = reinterpret_cast<uint8_t*>(&pCBData[2]);
        }
    }
    else
    {
        image = mpCamMsgCbInfo->mRequestMemory(-1, u4HeaderSize + u4BitstreamSize, 1, mpCamMsgCbInfo->mCbCookie);
        if  ( image )
        {
            pImage = reinterpret_cast<uint8_t*>(image->data);
        }
    }

    if  ( ! image )
    {
        MY_LOGW("mRequestMemory fail");
        ret = false;
        goto lbExit;
    }
    //
    if  ( image )
    {
        if  ( 0 != u4HeaderSize && 0 != puHeaderBuf )
        {
            ::memcpy(pImage, puHeaderBuf, u4HeaderSize);
            pImage += u4HeaderSize;
        }
        if  ( 0 != u4BitstreamSize && 0 != puBitstreamBuf )
        {
            ::memcpy(pImage, puBitstreamBuf, u4BitstreamSize);
        }
        //
        ZipImageCallbackThread::callback_data callbackData;
        callbackData.type = ZipImageCallbackThread::callback_type_image;
        callbackData.pImage = image;
        callbackData.u4CallbackIndex = u4CallbackIndex;
        callbackData.isFinalImage = fgIsFinalImage;
        callbackData.u4ShotMode = u4ShotMode;
        pThread->addCallbackData(&callbackData);
        pThread = 0;
        image = 0;
    }
    //
lbExit:
    //
    return  ret;
}


//
//  Callback of Packed Compressed Image (CAMERA_MSG_COMPRESSED_IMAGE)
//
//      [Compressed Image] = [Header] + [Bitstream],
//      where
//          Header may be jpeg exif (including thumbnail)
//
//  Arguments:
//      i8Timestamp
//          [I] Compressed image timestamp
//
//      u4BitstreamSize
//          [I] Compressed Image buffer size in bytes.
//
//      puBitstreamBuf
//          [I] Compressed Image buffer with its size = u4BitstreamSize
//
//      u4CallbackIndex
//          [I] Callback index. 0 by default.
//              If more than one compressed callback must be invoked during
//              captures, for example burst shot & ev shot, this value is
//              the callback index; and 0 indicates the first one callback.
//
//      fgIsFinalImage
//          [I] booliean value to indicate whether it is the final image.
//              true if this is the final image callback; otherwise false.
//              For single captures, this value must be true.
//
bool
CamAdapter::
onCB_CompressedImage_packed(
    int64_t const   i8Timestamp,
    uint32_t const  u4BitstreamSize,
    uint8_t const*  puBitstreamBuf,
    uint32_t const  u4CallbackIndex,
    bool            fgIsFinalImage,
    uint32_t const  msgType,
    uint32_t const  u4ShotMode,
    uint32_t const  u4BgServ /* = 0 */
)
{
    bool ret = true;
    camera_memory* image = NULL;
    uint8_t* pImage = NULL;
    uint32_t const u4DataSize = u4BitstreamSize + sizeof(uint32_t)*(1+1);
    sp<ZipImageCallbackThread> pThread = NULL;
    //
    MY_LOGD(
        "timestamp(%lld), bitstream:size/buf=%d/%p, index(%d), IsFinalImage(%d)",
        i8Timestamp, u4BitstreamSize, puBitstreamBuf, u4CallbackIndex, fgIsFinalImage
    );

    /* --------------------------------------------------------------------- */
    /* Start to callback CAMERA_MSG_COMPRESSED_IMAGE                         */
    /* --------------------------------------------------------------------- */
    // If using BackgroundService, send CAMERA_MSG_COMPRESSED_IMAGE via MMSDK.
    if (MMSDK_CALLBACK_EN_JPEG && u4BgServ) {
        MY_LOGD("send CAMERA_MSG_COMPRESSED_IMAGE via MMSDK");
        ICallbackClientMgr::getInstance()->onCB_CompressedImage_packed(
            i8Timestamp,
            u4BitstreamSize,
            puBitstreamBuf,
            u4CallbackIndex,
            fgIsFinalImage,
            msgType
            );
        goto lbExit;
    }

    // if disabled message, do not send
    if  ( ! msgTypeEnabled(CAMERA_MSG_COMPRESSED_IMAGE) )
    {
        MY_LOGW("msgTypeEnabled=%#x", msgTypeEnabled(0xFFFFFFFF));
        sp<IParamsManager> pParamsMgr = getParamsManager();
        if( pParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE) == MtkCameraParameters::CAMERA_MODE_NORMAL )
        {
            MY_LOGW("compressed image callback ignored");
            goto lbExit;
        }
    }
    //
    pThread = mpZipCallbackThread.promote();
    if  ( pThread.get() == 0 )
    {
        MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
        ret = false;
        goto lbExit;
    }
    //
    if ( pThread->isExtCallback() )
    {
        image = mpCamMsgCbInfo->mRequestMemory(-1, u4DataSize, 1, mpCamMsgCbInfo->mCbCookie);
        if  ( image )
        {
            uint32_t*const pCBData = reinterpret_cast<uint32_t*>(image->data);
            pCBData[0] = msgType;
            pCBData[1] = u4CallbackIndex;
            pImage = reinterpret_cast<uint8_t*>(&pCBData[2]);
        }
    }
    else
    {
        image = mpCamMsgCbInfo->mRequestMemory(-1, u4BitstreamSize, 1, mpCamMsgCbInfo->mCbCookie);
        if  ( image )
        {
            pImage = reinterpret_cast<uint8_t*>(image->data);
        }
    }

    if  ( ! image )
    {
        MY_LOGW("mRequestMemory fail");
        ret = false;
        goto lbExit;
    }
    //
    if  ( image )
    {
        if  ( 0 != u4BitstreamSize && 0 != puBitstreamBuf )
        {
            ::memcpy(pImage, puBitstreamBuf, u4BitstreamSize);
        }
        //
        ZipImageCallbackThread::callback_data callbackData;
        callbackData.type = ZipImageCallbackThread::callback_type_image;
        callbackData.pImage = image;
        callbackData.u4CallbackIndex = u4CallbackIndex;
        callbackData.isFinalImage = fgIsFinalImage;
        callbackData.u4ShotMode = u4ShotMode;
        pThread->addCallbackData(&callbackData);
        pThread = 0;
        image = 0;
    }
    //
lbExit:
    //
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of P2done (MTK_CAMERA_MSG_EXT_NOTIFY_P2DONE)
//
bool
CamAdapter::
onCB_P2done(
    int32_t bNofityCaptureDone /* = 0 */
)
{
    MY_LOGD("onCB_P2done()");
    //
    if(isThirdPartyZSD())
    {
        //ThirdPartyZSD no need callback p2done for light up the capture button
        MY_LOGD("ThirdPartyZSD skip P2done callback");
        return true;
    }
    //
    sp<ZipImageCallbackThread> pThread = NULL;
    //
    pThread = mpZipCallbackThread.promote();
    if  ( pThread.get() == 0 )
    {
        MY_LOGW("Fail to run ZipImageCallbackThread (%p)", pThread.get());
        return false;
    }
    //
    ZipImageCallbackThread::callback_data callbackData;
    callbackData.type = ZipImageCallbackThread::callback_type_p2done_notify;
    callbackData.pImage = NULL;
    callbackData.u4CallbackIndex = 0;
    callbackData.isFinalImage = true;
    callbackData.p2done_notifyCaptureDone = bNofityCaptureDone;
    pThread->addCallbackData(&callbackData);
    pThread = NULL;
    return  true;
}

/******************************************************************************
 *
 ******************************************************************************/
//
//  Callback of P1done (MTK_CAMERA_MSG_EXT_NOTIFY_P1DONE)
//
bool
CamAdapter::
onCB_P1done()
{
    MY_LOGD("onCB_P1done()");
    //
    //if(mpStateManager->isState(IState::eState_ZSLCapture))
    if(mb4CellReMosaicCapFlow&& !isThirdPartyZSD())
    {
        MY_LOGD("APP_MODE_NAME_MTK_ZSD - mbEnable4cellStopPreview(%d), m4CellStartPreviewThreadHandle(%d)", mbEnable4cellStopPreview, m4CellStartPreviewThreadHandle);
        //
        Mutex::Autolock _l(mbEnable4cellStopPreviewLock);
        //
        if((mbEnable4cellStopPreview == MFALSE) && (m4CellStartPreviewThreadHandle == 0))
        {
            MY_LOGD("mbEnable4cellStopPreview = MFALSE");

            Mutex::Autolock lk(m4CellStartPreviewFlagLock);
            if( pthread_create(&m4CellStartPreviewThreadHandle, NULL, start4cellPreviewThread, this) != 0 )
            {
                ALOGE("startPreview pthread create failed");
            }
            else
            {
                m4CellStartPreviewFlag = true; // mark as thread running
            }

            //MY_LOGD("pthread_create(m4CellStartPreviewThreadHandle)");
        }
        else if(mbIsNeedSwitchModeToCapture)
        {
            MY_LOGD("mbIsNeedSwitchModeToCapture = MTRUE");
            Mutex::Autolock lk(m4CellStartPreviewFlagLock);
            if( pthread_create(&m4CellStartPreviewThreadHandle, NULL, start4cellPreviewThread, this) != 0 )
            {
                ALOGE("startPreview pthread create failed");
            }
            else
            {
                m4CellStartPreviewFlag = true; // mark as thread running
            }
        }
        return  true;
    }
    else
    {
        MY_LOGD("No Need startPreview , is4CellReMosaicCapFlow(%d)", mb4CellReMosaicCapFlow);
        return  true;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
bool
CamAdapter::
wait4cellStartPreviewDone()
{
    if(m4CellStartPreviewThreadHandle != 0)
    {
        MY_LOGD("E");
        // check if m4CellStartPreviewFlag is true or not, if it is true, wait it until false
        {
            Mutex::Autolock lk(m4CellStartPreviewFlagLock);
            if (m4CellStartPreviewFlag) {
                auto result = m4CellStartPreviewFlagCond.waitRelative(m4CellStartPreviewFlagLock, 1000LL*1000LL*1000LL*1000LL); // wait for a second (unit: ns)
                if (__builtin_expect( result != OK , false )) {
                    MY_LOGE("wait start4cellPreviewThread timed out (1s)...anyway, ignore it");
                }
            }
        }
        m4CellStartPreviewThreadHandle = 0;
        MY_LOGD("X");
    }
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
void*
CamAdapter::
start4cellPreviewThread(void* arg)
{
    ::prctl(PR_SET_NAME, (unsigned long)"start4cellPreview", 0, 0, 0);
    CAM_LOGD("[CamAdapter::m4CellStartPreviewThreadHandle]+");
    CamAdapter* pCamAdapter = (CamAdapter*)arg;
    //pCamAdapter->startPreview();
    //
    status_t ret = INVALID_OPERATION;
    //
    ret = pCamAdapter->mpFlowControl->startPreview();
    //ret = pCamAdapter->mpFlowControl->resumePreviewP1NodeFlow();
    if (ret != OK)
    {
        CAM_LOGD("resumePreviewP1NodeFlow() fail");
    }
    //
    CAM_LOGD("[CamAdapter::m4CellStartPreviewThreadHandle]- done");
    if(pCamAdapter!=NULL && pCamAdapter->mpFlowControl!=NULL)
    {
        pCamAdapter->m4CellStartPreviewThreadHandle = 0;
    }

    CAM_LOGD("[CamAdapter::m4CellStartPreviewThreadHandle]- %d", pCamAdapter->m4CellStartPreviewThreadHandle);
    // notify start4cellPreviewThread has finished
    {
        Mutex::Autolock lk(pCamAdapter->m4CellStartPreviewFlagLock);
        pCamAdapter->m4CellStartPreviewFlag = false; // clear as done
        pCamAdapter->m4CellStartPreviewFlagCond.broadcast(); // tell everybody
    }
    pthread_detach( pthread_self() ); // never joined, detach the current thread
    pthread_exit(NULL);
    CAM_LOGD("[CamAdapter::m4CellStartPreviewThreadHandle]-");
    return NULL;
}

