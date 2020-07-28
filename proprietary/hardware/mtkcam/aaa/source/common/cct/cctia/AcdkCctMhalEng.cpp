/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCctMhalEng.cpp

#define LOG_TAG "AcdkCctMhalEng"
#define USING_HIDL

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <vector>
#include <sys/resource.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <cutils/properties.h>
#include <semaphore.h>

#include <mtkcam/def/PriorityDefs.h>

#include "AcdkErrCode.h"
#include "AcdkLog.h"
#include "AcdkCallback.h"
#include "AcdkSurfaceView.h"
#include "AcdkCctBase.h"
#include "AcdkCctMhalBase.h"
#include "AcdkUtility.h"
using namespace NSCam;
using namespace NSACDK;
using namespace NSAcdkCctMhal;

#include <mtkcam/utils/exif/IBaseCamExif.h>

#include <binder/IMemory.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <utils/RefBase.h>
#include <cutils/properties.h>
#include <system/camera.h>
#include <hardware/camera.h>
#include <dlfcn.h>
#include <mtkcam/utils/fwk/MtkCamera.h>
#include <mtkcam/utils/fwk/MtkCameraParameters.h>

#include "AcdkCctMhalEng2.h"

static void __notify_cb(int32_t msg_type, int32_t ext1, int32_t ext2, void *user);
static camera_memory_t* __get_memory(int fd, size_t buf_size, uint_t num_bufs,
                                     void *user __attribute__((unused)));
static void __data_cb(int32_t msg_type,
                      const camera_memory_t *data, unsigned int index,
                      camera_frame_metadata_t *metadata,
                      void *user);
static void __data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                         const camera_memory_t *data, unsigned index,
                         void *user);
namespace android {
namespace hardware {
namespace camera {
namespace device {
namespace V1_0 {
class ACDKCameraDeviceCallback : public ICameraDeviceCallback
{

public:

    //camera_memory_t* (* acdk_get_memory) (int fd, size_t buf_size, uint_t num_bufs, void *user __attribute__((unused))) = __get_memory;
    //void (* acdk_data_cb) (int32_t msg_type, const camera_memory_t *data, unsigned int index, camera_frame_metadata_t *metadata, void *user) = __data_cb;

    ::android::hardware::Return<void> notifyCallback(NotifyCallbackMsg msgType, int32_t ext1, int32_t ext2) override
    {
        (void)msgType;
        (void)ext1;
        (void)ext2;

        return ::android::hardware::Void();
    }

    ::android::hardware::Return<uint32_t> registerMemory(const ::android::hardware::hidl_handle& descriptor, uint32_t bufferSize, uint32_t bufferCount) override
    {
        (void)descriptor;
        (void)bufferSize;
        (void)bufferCount;

        auto _hidl_return = 0;
        return _hidl_return;
    }

    ::android::hardware::Return<void> unregisterMemory(uint32_t memId) override
    {
        return ::android::hardware::Void();
    }

    ::android::hardware::Return<void> dataCallback(DataCallbackMsg msgType, uint32_t data, uint32_t bufferIndex, const CameraFrameMetadata& metadata) override
    {
        (void)msgType;
        (void)data;
        (void)bufferIndex;
        (void)metadata;

        /*
        uint32_t msg_type;
        switch(msgType){
        case DataCallbackMsg::PREVIEW_FRAME:
            msg_type = CAMERA_MSG_PREVIEW_FRAME;
        case DataCallbackMsg::POSTVIEW_FRAME:
            msg_type = CAMERA_MSG_POSTVIEW_FRAME;
        default:
            msg_type = 0;
        }
        acdk_data_cb(msg_type, (camera_memory_t*)data, bufferIndex, (camera_frame_metadata_t*)NULL, NULL);
        */

        return ::android::hardware::Void();
    }

    ::android::hardware::Return<void> dataCallbackTimestamp(DataCallbackMsg msgType, uint32_t data, uint32_t bufferIndex, int64_t timestamp) override
    {
        (void)msgType;
        (void)data;
        (void)bufferIndex;
        (void)timestamp;

        return ::android::hardware::Void();
    }

    ::android::hardware::Return<void> handleCallbackTimestamp(DataCallbackMsg msgType, const ::android::hardware::hidl_handle& frameData, uint32_t data, uint32_t bufferIndex, int64_t timestamp) override
    {
        (void)msgType;
        (void)frameData;
        (void)data;
        (void)bufferIndex;
        (void)timestamp;

        return ::android::hardware::Void();
    }

    ::android::hardware::Return<void> handleCallbackTimestampBatch(DataCallbackMsg msgType, const ::android::hardware::hidl_vec<HandleTimestampMessage>& batch) override
    {
        (void)msgType;
        (void)batch;

        return ::android::hardware::Void();
    }
};
}  // namespace V1_0
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace android

namespace
{
#ifdef USING_HIDL
    //ICameraProvider* mCamProvider = NULL;
    //ICameraDevice* mCamDevice = NULL;
    //DefaultCameraDevice1* mCameraDevice = NULL;
#else
    camera_module_t *mModule = NULL;
    camera_device_t *mDevice = NULL;
#endif
    int mNumberOfCameras = 0;
}

using namespace android;

#include "AcdkCctMhalEng.h"

#define MEDIA_PATH "/mnt/vendor/cct"
#define ACDK_MT6582_MDP_WO_IRQ 1 //SL test ACDK_6582_MDP without IRQ don

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

/*******************************************************************************
* Global variable
*******************************************************************************/
static sem_t g_SemMainHigh, g_SemMainHighBack, g_SemMainHighEnd;
static pthread_t g_threadMainHigh;
static AcdkCctMhalEng *g_pAcdkCctMHalEngObj = NULL;
static acdkObserver g_acdkCctMhalObserver;
static acdkObserver g_acdkCctMhalCapObserver;

static MUINT32 is_yuv_sensor = 0;

static MINT32 g_acdkCctMhalEngDebug = 0;
static MBOOL mCaptureDone = MFALSE;

void AcdkCctMhalLoadModule()
{
    if(mNumberOfCameras)
        return;
#ifdef USING_HIDL
    mNumberOfCameras = 4;

#if 0
    //  ICameraProvider
    //::android::sp<ICameraProvider> pProvider = ICameraProvider::getService("internal/0");
    mCamProvider = HIDL_FETCH_ICameraProvider("Default Camera HAL");

    //mCamProvider = new CameraProvider();
    mCamProvider->getCameraIdList(
        [&](auto status, const auto& idList) {
            mNumberOfCameras = idList.size();
            ALOGE("yiyi: get cam num=%d", (int)mNumberOfCameras);
        });
#endif
#else
    if (hw_get_module(CAMERA_HARDWARE_MODULE_ID,
                (const hw_module_t **)&mModule) < 0) {
        ACDK_LOGD("Could not load camera HAL module");
        mNumberOfCameras = 0;
    }
    else {
        ACDK_LOGD("Loaded \"%s\" camera module", mModule->common.name);
        mNumberOfCameras = mModule->get_number_of_cameras();
    }
#endif
}

status_t AcdkCctMhalOpenDevice(const char *name, const int id)
{
    int rc = 0;
    ACDK_LOGD("Opening camera + \n");

#ifdef USING_HIDL
    ACDK_LOGD("Opening camera: instanceName=%s, instanceID=%d \n", name, id);
    string instanceName(name);
    create_CameDev(instanceName, id);
#else
    ACDK_LOGD("Opening camera: mName=%s \n", name);
    rc = mModule->open_legacy((hw_module_t *)mModule, name, CAMERA_DEVICE_API_VERSION_1_0, (hw_device_t **)&mDevice);
    if (rc != OK) {
        ACDK_LOGE("Could not open camera %s: %d", name, rc);
        return rc;
    }
#endif

    ACDK_LOGD("Opening camera - \n");
    return rc;
}

void AcdkCctMhalReleaseDevice() {
    ACDK_LOGD(" AcdkCctMhalReleaseDevice \n");
#ifdef USING_HIDL
    close_CameDev();
    //mNumberOfCameras = 0;
#else
    if (mDevice->ops->release)
    {
        mDevice->ops->release(mDevice);

    }

    //ACDK_LOGD("Destroying camera %s", mName.string());
    if(mDevice) {
    int rc = mDevice->common.close(&mDevice->common);
    if (rc != OK)
        ACDK_LOGD("Could not close camera %d", rc);
    }
    mDevice = NULL;
#endif
}

static void __put_memory(camera_memory_t *data);

class CameraHeapMemory : public RefBase {
public:
    CameraHeapMemory(int fd, size_t buf_size, uint_t num_buffers = 1) :
                     mBufSize(buf_size),
                     mNumBufs(num_buffers)
    {
        mHeap = new MemoryHeapBase(fd, buf_size * num_buffers);
        commonInitialization();
    }

    CameraHeapMemory(size_t buf_size, uint_t num_buffers = 1) :
                     mBufSize(buf_size),
                     mNumBufs(num_buffers)
    {
        mHeap = new MemoryHeapBase(buf_size * num_buffers);
        commonInitialization();
    }

    void commonInitialization()
    {
        handle.data = mHeap->base();
        handle.size = mBufSize * mNumBufs;
        handle.handle = this;

        mBuffers = new sp<MemoryBase>[mNumBufs];
        for (uint_t i = 0; i < mNumBufs; i++)
            mBuffers[i] = new MemoryBase(mHeap,
                                         i * mBufSize,
                                         mBufSize);

        handle.release = __put_memory;
    }

    virtual ~CameraHeapMemory()
    {
        delete [] mBuffers;
    }

    size_t mBufSize;
    uint_t mNumBufs;
    sp<MemoryHeapBase> mHeap;
    sp<MemoryBase> *mBuffers;

    camera_memory_t handle;
};

static camera_memory_t* __get_memory(int fd, size_t buf_size, uint_t num_bufs,
                                     void *user __attribute__((unused)))
{
    CameraHeapMemory *mem;
    if (fd < 0)
        mem = new CameraHeapMemory(buf_size, num_bufs);
    else
        mem = new CameraHeapMemory(fd, buf_size, num_bufs);
    mem->incStrong(mem);

    return &mem->handle;
}

static void __put_memory(camera_memory_t *data)
{
    if (!data)
        return;

    CameraHeapMemory *mem = static_cast<CameraHeapMemory *>(data->handle);
    mem->decStrong(mem);
}

static void __notify_cb(int32_t msg_type, int32_t ext1,
                      int32_t ext2, void *user)
{
    ACDK_LOGD("%s", __FUNCTION__);
}

static void handleMtkExtDataCompressedImage(const sp<IMemory>& dataPtr)
{
#if 0//<binder/*.h> is not allowed to use anymore.
    MtkCamMsgExtDataHelper MtkExtDataHelper;
    if  ( ! MtkExtDataHelper.init(dataPtr) ) {
        ACDK_LOGE("[%s] MtkCamMsgExtDataHelper::init fail -  \r\n", __FUNCTION__);
        return;
    }
    //
    uint_t const*const pExtParam = (uint_t const*)MtkExtDataHelper.getExtParamBase();
    uint_t const      uShutIndex = pExtParam[0];
    //
    size_t const    imageSize   = MtkExtDataHelper.getExtParamSize()    - sizeof(uint_t) * 1;
    ssize_t const   imageOffset = MtkExtDataHelper.getExtParamOffset()  + sizeof(uint_t) * 1;
    // sp<MemoryBase> image = new MemoryBase(MtkExtDataHelper.getHeap(), imageOffset, imageSize);
    uint8_t* va = (((uint8_t*)MtkExtDataHelper.getHeap()->getBase())+imageOffset);
    ACDK_LOGD("%s  va=%p, size=%d, offset=%d", __FUNCTION__, va, imageSize,imageOffset);

    g_acdkCctMhalCapObserver.notify(ACDK_CB_CAPTURE,va, 0, (MUINT32)imageSize,0);
    //
    MtkExtDataHelper.uninit();
#endif
}

static void __data_cb(int32_t msg_type,
                      const camera_memory_t *data, unsigned int index,
                      camera_frame_metadata_t *metadata,
                      void *user)
{
    sp<CameraHeapMemory> mem(static_cast<CameraHeapMemory *>(data->handle));
    if (index >= mem->mNumBufs) {
        ACDK_LOGE("%s: invalid buffer index %d, max allowed is %d", __FUNCTION__,
             index, mem->mNumBufs);
        return;
    }

    ssize_t offset = 0;
    size_t size = 0;
    sp<IMemoryHeap> heap = mem->mBuffers[index]->getMemory(&offset, &size);
    uint8_t* va = (((uint8_t*)heap->getBase())+offset);
    ACDK_LOGD("%s type %x va=%p, size=%d, offset=%d", __FUNCTION__, msg_type, va, size,offset);


    switch  (msg_type & ~CAMERA_MSG_PREVIEW_METADATA)
    {
        case CAMERA_MSG_PREVIEW_FRAME:
             //Callback to upper layer
             g_acdkCctMhalObserver.notify(ACDK_CB_PREVIEW,va, 0,(MUINT32)size,0);
            break;
        case CAMERA_MSG_POSTVIEW_FRAME:
             //Callback to upper layer
             ACDK_LOGD("CAMERA_MSG_POSTVIEW_FRAME=%d",CAMERA_MSG_POSTVIEW_FRAME);
             g_acdkCctMhalObserver.notify(ACDK_CB_PREVIEW,va, 0,(MUINT32)size,0);
            break;
        case MTK_CAMERA_MSG_EXT_DATA:{
               ACDK_LOGD("MTK_CAMERA_MSG_EXT_DATA=%d",MTK_CAMERA_MSG_EXT_DATA);
               va += sizeof(uint32_t)*2;
               size -= sizeof(uint32_t)*2;
               ACDK_LOGD("%s  va=%p, size=%d, offset=%d", __FUNCTION__, va, size, offset);
               g_acdkCctMhalCapObserver.notify(ACDK_CB_CAPTURE, va, 0, (MUINT32)size,0);
               mCaptureDone = MTRUE;
            }
            break;
        case CAMERA_MSG_COMPRESSED_IMAGE:
            {
                ACDK_LOGD("CAMERA_MSG_COMPRESSED_IMAGE=%d",CAMERA_MSG_COMPRESSED_IMAGE);
                g_acdkCctMhalCapObserver.notify(ACDK_CB_CAPTURE, va, 0, (MUINT32)size, 0);
                mCaptureDone = MTRUE;
            }
            break;
        default:
            return;
    }


#if 0
    MINT32 i4WriteCnt = 0;
    char szFileName[256];

    ACDK_LOGD("%s  va=%p, size=%d, offset=%d", __FUNCTION__, va, size,offset);

    sprintf(szFileName, "%s/acdkCap.yuv",MEDIA_PATH);

    FILE *pFp = fopen(szFileName, "wb");

    if(NULL == pFp)
    {
        ACDK_LOGE("Can't open file to save image");
    }

    i4WriteCnt = fwrite(va, 1, size, pFp);

    fflush(pFp);

    if(0 != fsync(fileno(pFp)))
    {
        fclose(pFp);
    }
#endif

}

static void __data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                         const camera_memory_t *data, unsigned index,
                         void *user)
{
}

String8 flatten(DefaultKeyedVector<String8,String8>const& rMap)
{
    String8 flattened("");
    size_t size = rMap.size();

    for (size_t i = 0; i < size; i++) {
        String8 k, v;
        k = rMap.keyAt(i);
        v = rMap.valueAt(i);

        flattened += k;
        flattened += "=";
        flattened += v;
        if (i != size-1)
            flattened += ";";
    }

    return flattened;
}

void unflatten(const String8 &params, DefaultKeyedVector<String8,String8>& rMap)
{
    const char *a = params.string();
    const char *b;

    rMap.clear();

    for (;;) {
        // Find the bounds of the key name.
        b = strchr(a, '=');
        if (b == 0)
            break;

        // Create the key string.
        String8 k(a, (size_t)(b-a));

        // Find the value.
        a = b+1;
        b = strchr(a, ';');
        if (b == 0) {
            // If there's no semicolon, this is the last item.
            String8 v(a);
            rMap.add(k, v);
            break;
        }

        String8 v(a, (size_t)(b-a));
        rMap.add(k, v);
        a = b+1;
    }
}


void set(DefaultKeyedVector<String8,String8>& rMap, const char *key, const char *value)
{
    // XXX i think i can do this with strspn()
    if (strchr(key, '=') || strchr(key, ';')) {
        //XXX ALOGE("Key \"%s\"contains invalid character (= or ;)", key);
        return;
    }

    if (strchr(value, '=') || strchr(value, ';')) {
        //XXX ALOGE("Value \"%s\"contains invalid character (= or ;)", value);
        return;
    }

    rMap.replaceValueFor(String8(key), String8(value));
}

/*******************************************************************************
* AcdkCctMhalEng
* brif : Constructor
*******************************************************************************/
AcdkCctMhalEng::AcdkCctMhalEng()
{
    mAcdkCctMhalState = ACDK_CCT_MHAL_NONE;



    mFocusDone = MFALSE;
    mFocusSucceed = MFALSE;

    mReadyForCap = MFALSE;
    mCaptureDone = MFALSE;

    GET_PROP("vendor.debug.acdkcct.eng.log", "0", mDebugEnable);

    memset(&mAcdkCctMhalPrvParam,0,sizeof(acdkCctMhalPrvParam_t));

    g_pAcdkCctMHalEngObj = this;

}

/*******************************************************************************
* acdkCctMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
MVOID AcdkCctMhalEng::acdkCctMhalSetState(acdkCctMhalState_e newState)
{
    Mutex::Autolock lock(mLock);

    ACDK_LOGD("Now(0x%04x), Next(0x%04x)", mAcdkCctMhalState, newState);

    if(newState == ACDK_CCT_MHAL_ERROR)
    {
        goto ACDK_CCT_MHAL_SET_STATE_EXIT;
    }

    switch(mAcdkCctMhalState)
    {
    case ACDK_CCT_MHAL_NONE:
        switch(newState)
        {
        case ACDK_CCT_MHAL_INIT:
        case ACDK_CCT_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_CCT_MHAL_NONE");
            ACDK_LOGE("State error ACDK_CCT_MHAL_NONE");
            break;
        }
        break;
    case ACDK_CCT_MHAL_INIT:
        switch(newState)
        {
        case ACDK_CCT_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_INIT");
            ACDK_LOGE("State error ACDK_CCT_MHAL_INIT");
            break;
        }
        break;
    case ACDK_CCT_MHAL_IDLE:
        switch(newState)
        {
        case ACDK_CCT_MHAL_IDLE:
        case ACDK_CCT_MHAL_PREVIEW:
        case ACDK_CCT_MHAL_CAPTURE:
        case ACDK_CCT_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_IDLE");
            ACDK_LOGE("State error ACDK_CCT_MHAL_IDLE");
            break;
        }
        break;
    case ACDK_CCT_MHAL_PREVIEW:
        switch(newState)
        {
        case ACDK_CCT_MHAL_IDLE:
        case ACDK_CCT_MHAL_PREVIEW:
        case ACDK_CCT_MHAL_PRE_CAPTURE:
        case ACDK_CCT_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PREVIEW");
            ACDK_LOGE("State error ACDK_CCT_MHAL_PREVIEW");
            break;
        }
        break;
    case ACDK_CCT_MHAL_PRE_CAPTURE:
        switch(newState)
        {
        case ACDK_CCT_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PRE_CAPTURE");
            ACDK_LOGE("State error ACDK_CCT_MHAL_PRE_CAPTURE");
            break;
        }
        break;
    case ACDK_CCT_MHAL_PREVIEW_STOP:
        switch(newState)
        {
        case ACDK_CCT_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_CCT_MHAL_PREVIEW_STOP");
            ACDK_LOGE("State error ACDK_CCT_MHAL_PREVIEW_STOP");
            break;
        }
        break;
    case ACDK_CCT_MHAL_CAPTURE:
        switch(newState)
        {
        case ACDK_CCT_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_CAPTURE");
            ACDK_LOGE("State error ACDK_CCT_MHAL_CAPTURE");
            break;
        }
        break;
    case ACDK_CCT_MHAL_ERROR:
        switch(newState)
        {
        case ACDK_CCT_MHAL_IDLE:
        case ACDK_CCT_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_CCT_MHAL_ERROR");
            ACDK_LOGE("State error ACDK_CCT_MHAL_ERROR");
            break;
        }
        break;
    default:
        //ACDK_ASSERT(0, "Unknown state");
        ACDK_LOGE("Unknown state");
        break;
    }

ACDK_CCT_MHAL_SET_STATE_EXIT:

    mAcdkCctMhalState = newState;

    ACDK_LOGD("X, state(0x%04x)", mAcdkCctMhalState);
}

/*******************************************************************************
* acdkCctMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
acdkCctMhalState_e AcdkCctMhalEng::acdkCctMhalGetState()
{
    Mutex::Autolock _l(mLock);
    return mAcdkCctMhalState;
}

/*******************************************************************************
* acdkCctMhalReadyForCap
* brif : get status of mReadyForCap falg
*******************************************************************************/
MBOOL AcdkCctMhalEng::acdkCctMhalReadyForCap()
{
    return mReadyForCap;
}

/*******************************************************************************
* acdkCctMhalProcLoop
* brif : preview and capture thread executing function
*******************************************************************************/
static MVOID *acdkCctMhalProcLoop(MVOID *arg)
{
    ::prctl(PR_SET_NAME,"acdkCctMhalProcLoop",0,0,0);

    MINT32 const policy = SCHED_OTHER;
    MINT32 const priority = NICE_CAMERA_PASS1;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    ::sched_getparam(0, &sched_p);
    ACDK_LOGD("policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d),tid=%d", policy,
                                                                                            ::sched_getscheduler(0),
                                                                                            priority,
                                                                                            sched_p.sched_priority,
                                                                                            ::gettid());

     //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());

    acdkCctMhalState_e eState;

    eState = g_pAcdkCctMHalEngObj->acdkCctMhalGetState();
    while(eState != ACDK_CCT_MHAL_UNINIT)
    {
        ACDK_LOGD("Wait semMainHigh");
        ::sem_wait(&g_SemMainHigh); // wait here until someone use sem_post() to wake this semaphore up
        ACDK_LOGD("Got semMainHigh");

        eState = g_pAcdkCctMHalEngObj->acdkCctMhalGetState();

        switch(eState)
        {
        case ACDK_CCT_MHAL_PREVIEW:
            g_pAcdkCctMHalEngObj->acdkCctMhalPreviewProc();
            ::sem_post(&g_SemMainHighBack);
            break;
        case ACDK_CCT_MHAL_CAPTURE:
            g_pAcdkCctMHalEngObj->acdkCctMhalCaptureProc();
            break;
        case ACDK_CCT_MHAL_UNINIT:
            break;
        default:
            ACDK_LOGD("T.B.D");
            break;
        }

        eState = g_pAcdkCctMHalEngObj->acdkCctMhalGetState();
    }


    ::sem_post(&g_SemMainHighEnd);
    ACDK_LOGD("-");

    return NULL;
}

/*******************************************************************************
* destroyInstanc
* brif : destroy AcdkCctMhalEng object
*******************************************************************************/
void AcdkCctMhalEng::destroyInstance()
{
    g_pAcdkCctMHalEngObj = NULL;
    delete this;
}

/*******************************************************************************
* acdkCctMhalInit
* brif : initialize camera
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalInit()
{
    ACDK_LOGD("+");

    //====== Loca Variable Declaration ======

    MINT32  err = ACDK_RETURN_NO_ERROR;

    //====== Acdk Mhal State Setting ======

    mAcdkCctMhalState = ACDK_CCT_MHAL_INIT;    //set state to Init state

    //====== Get Debug Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.camera.acdk.debug", value, "0");
    g_acdkCctMhalEngDebug = atoi(value);

    ACDK_LOGD("g_acdkCctMhalEngDebug(%d)",g_acdkCctMhalEngDebug);

    //====== Init Thread for Preview and Capture ======

    // Init semphore
    ::sem_init(&g_SemMainHigh, 0, 0);
    ::sem_init(&g_SemMainHighBack, 0, 0);
    ::sem_init(&g_SemMainHighEnd, 0, 0);

    // Create main thread for preview and capture
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_PASS1};
    int min = sched_get_priority_min( SCHED_RR );
    int max = sched_get_priority_max( SCHED_RR );
    ACDK_LOGD("(min,max)=(%d,%d)", min, max);

    int result = pthread_create(&g_threadMainHigh, &attr, acdkCctMhalProcLoop, NULL);
    ACDK_LOGD("result of pthread_create : %d", result);

    //=== Set State to Idle State ======

    acdkCctMhalSetState(ACDK_CCT_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkCctMhalUninit
* brif : Uninitialize camera
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalUninit()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkCctMhalState_e eState;

    //====== Uninitialization ======

    // Check it is in the idle mode
    // If it is not, has to wait until idle
    eState = acdkCctMhalGetState();

    ACDK_LOGD("eState(0x%x)",eState);

    if(eState != ACDK_CCT_MHAL_NONE)
    {
        if((eState != ACDK_CCT_MHAL_IDLE) && (eState != ACDK_CCT_MHAL_ERROR))
        {
            ACDK_LOGD("Camera is not in the idle state");
            if(eState & ACDK_CCT_MHAL_PREVIEW_MASK)
            {
                err = acdkCctMhalPreviewStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkCctMhalPreviewStop fail(0x%x)",err);
                }
            }
            else if(eState & ACDK_CCT_MHAL_CAPTURE_MASK)
            {
                err = acdkCctMhalCaptureStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkCctMhalCaptureStop fail(0x%x)",err);
                }
            }

            // Polling until idle
            while(eState != ACDK_CCT_MHAL_IDLE)
            {
                // Wait 10 ms per time
                usleep(10000);
                eState = acdkCctMhalGetState();
            }
            ACDK_LOGD("Now camera is in the idle state");
        }

        //====== Set State to Uninit State ======

        acdkCctMhalSetState(ACDK_CCT_MHAL_UNINIT);

        //====== Semephore Process ======

        //post sem
        ACDK_LOGD("post g_SemMainHigh");
        ::sem_post(&g_SemMainHigh);

        //wait sem
        ACDK_LOGD("wait for g_SemMainHighEnd");
        ::sem_wait(&g_SemMainHighEnd);
        ACDK_LOGD("got g_SemMainHighEnd");
    }
    else
    {
        acdkCctMhalSetState(ACDK_CCT_MHAL_UNINIT);
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkCctMhalCBHandle
* brif : callback handler
*******************************************************************************/
MVOID AcdkCctMhalEng::acdkCctMhalCBHandle(MUINT32 a_type, MUINT8* a_addr1, MUINT8* a_addr2, MUINT32 const a_dataSize)
{
    ACDK_LOGD_DYN(g_acdkCctMhalEngDebug,"+");

    if(!g_acdkCctMhalObserver)
    {
        ACDK_LOGE("callback is NULL");
    }

    // Callback to upper layer
    g_acdkCctMhalObserver.notify(a_type, a_addr1, a_addr2, a_dataSize);
}

/*******************************************************************************
* acdkCctMhal3ASetParm
* brif : set 3A parameter
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhal3ASetParam(MINT32 devID, MUINT8 IsFactory)
{
    ACDK_LOGD("devID(%d)",devID);

    return ACDK_RETURN_NO_ERROR;
}
#define PASS2_FULLG

/*******************************************************************************
* acdkCctMhalPreviewStart
* brif : Start preview
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalPreviewStart(MVOID *a_pBuffIn)
{
    ACDK_LOGD("+");
    char szUser[32] = "acdk_preview";

    ACDK_ASSERT(acdkCctMhalGetState() == ACDK_CCT_MHAL_IDLE, "[acdkCctMhalPreviewStart] Camera State is not IDLE");

    //====== Local Variable Declaration ======
    MINT32 err = ACDK_RETURN_NO_ERROR;
    memcpy(&mAcdkCctMhalPrvParam, a_pBuffIn, sizeof(acdkCctMhalPrvParam_t));

    ACDK_LOGD("%s  test Cam Device", __FUNCTION__);

    AcdkCctMhalLoadModule();

    String8 key = String8("vendor.mtk.client.appmode");
    String8 value = String8("MtkEng");
    property_set(key, value);

    ACDK_LOGD("%s  device.name=%d", __FUNCTION__, mAcdkCctMhalPrvParam.sensorIndex);
#ifdef USING_HIDL
    switch(mAcdkCctMhalPrvParam.sensorIndex){
    case 0:
        AcdkCctMhalOpenDevice("device@1.0/internal/0", 0);
        break;
    case 1:
        AcdkCctMhalOpenDevice("device@1.0/internal/1", 1);
        break;
    default:
        AcdkCctMhalOpenDevice("device@1.0/internal/2", 2);
        break;
    }
#else
    switch(mAcdkCctMhalPrvParam.sensorIndex){
    case 0:
        AcdkCctMhalOpenDevice("0", -1);
        break;
    case 1:
        AcdkCctMhalOpenDevice("1", -1);
        break;
    default:
        AcdkCctMhalOpenDevice("2", -1);
        break;
    }
#endif

    ACDK_LOGD("+ set_callbacks \n");

#ifdef USING_HIDL
    sp<android::hardware::camera::device::V1_0::ACDKCameraDeviceCallback> mCamDevCallback = new android::hardware::camera::device::V1_0::ACDKCameraDeviceCallback();
    open_CameDev(mCamDevCallback,  __notify_cb, __data_cb, __data_cb_timestamp, __get_memory);

    //ICameraDeviceCallback mCamDevCallback = new ICameraDeviceCallback();
    //ICameraDeviceCallback->notifyCallback = ;
    //::android::sp<ICameraDeviceCallback> mCamDevCallback = nullptr;
    //::android::sp<ICameraDeviceCallback> mCamDevCallback = new ::android::sp<ICameraDeviceCallback>();
    //::android::sp<android::hardware::camera::device::V1_0::ACDKCameraDeviceCallback> mCamDevCallback;
    //android::hardware::camera::device::V1_0::ACDKCameraDeviceCallback* mCamDevCallback_IF = &mCamDevCallback;
    //mCameraDevice->open(mCamDevCallback);
#else
    if (mDevice->ops->set_callbacks) {
        mDevice->ops->set_callbacks(mDevice,
                               __notify_cb,
                               __data_cb,
                               __data_cb_timestamp,
                               __get_memory,
                               &mDevice);
    }
#endif

    ACDK_LOGD("- set_callbacks");


    //if (mDevice->ops->enable_msg_type)
    //    mDevice->ops->enable_msg_type(mDevice, CAMERA_MSG_PREVIEW_FRAME);

#ifdef USING_HIDL
    enableMsgType_CameDev( CAMERA_MSG_PREVIEW_FRAME |CAMERA_MSG_COMPRESSED_IMAGE | CAMERA_MSG_POSTVIEW_FRAME );
#else
    if (mDevice->ops->enable_msg_type)
        mDevice->ops->enable_msg_type(mDevice, CAMERA_MSG_PREVIEW_FRAME |CAMERA_MSG_COMPRESSED_IMAGE | CAMERA_MSG_POSTVIEW_FRAME);
#endif

    DefaultKeyedVector<String8,String8> map;
#ifdef USING_HIDL
    {
        string temp;
        getParameters_CameDev (temp);
        String8 str_parms( temp.c_str() );
        unflatten(str_parms, map);
    }
#if 0
    {
        //hidl_string temp
        string temp;
        mCamProvider->getParameters(&temp);
        String8 str_parms( temp.c_str() );
        unflatten(str_parms, map);
    }
#endif
#else
    if (mDevice->ops->get_parameters) {
        char *temp = mDevice->ops->get_parameters(mDevice);
        String8 str_parms(temp);
        if (mDevice->ops->put_parameters)
            mDevice->ops->put_parameters(mDevice, temp);
        else
            free(temp);
        unflatten(str_parms, map);
    }
#endif

    String8 sPrvSize = String8("");
    if( NSCam::eTransform_ROT_90 == mAcdkCctMhalPrvParam.frmParam.orientation || NSCam::eTransform_ROT_270 == mAcdkCctMhalPrvParam.frmParam.orientation )
    {
        //odd means 90 or 270 rotation
        sPrvSize = String8::format("%dx%d", mAcdkCctMhalPrvParam.frmParam.h, mAcdkCctMhalPrvParam.frmParam.w);
    }
    else
    {
        //even means 0 or 180 rotation
        sPrvSize = String8::format("%dx%d", mAcdkCctMhalPrvParam.frmParam.w, mAcdkCctMhalPrvParam.frmParam.h);
    }

    String8 sPrvList = map.valueFor( String8("preview-size-values") );
    ACDK_LOGD("Prv Size: %s; PrvList: %s", sPrvSize.string(), sPrvList.string() );
    if( NULL == strstr( sPrvList, sPrvSize ) )
    {
        ACDK_LOGE("Prv Size : %s is not supported. Try 640x480", sPrvSize.string());
        sPrvSize = String8("640x480");
        if( NSCam::eTransform_ROT_90 == mAcdkCctMhalPrvParam.frmParam.orientation || NSCam::eTransform_ROT_270 == mAcdkCctMhalPrvParam.frmParam.orientation )
        {
            mAcdkCctMhalPrvParam.frmParam.h = 640;
            mAcdkCctMhalPrvParam.frmParam.w = 480;
        }
        else
        {
            mAcdkCctMhalPrvParam.frmParam.h = 480;
            mAcdkCctMhalPrvParam.frmParam.w = 640;
        }

        if( NULL == strstr( sPrvList, sPrvSize ) )
        {
            ACDK_LOGE("640x480 is not supported.");
            err = ACDK_RETURN_INVALID_PARA;
            goto acdkCctMhalPreviewStart_Exit;
        }
    }

    set(map, "preview-size", sPrvSize);

    /*if(mAcdkCctMhalPrvParam.frmParam.orientation == NSCam::eTransform_ROT_90)
    {
        if(mAcdkCctMhalPrvParam.frmParam.h > 1280)
            set(map, "preview-size", "1440x1080");
        else if(mAcdkCctMhalPrvParam.frmParam.h > 800)
            set(map, "preview-size", "1280x720");
        else
            set(map, "preview-size", "800x600");
    }
    else
    {
        if(mAcdkCctMhalPrvParam.frmParam.w > 1280)
            set(map, "preview-size", "1440x1080");
        else if(mAcdkCctMhalPrvParam.frmParam.w > 800)
            set(map, "preview-size", "1280x720");
        else
            set(map, "preview-size", "800x600");
    }*/


    set(map, "preview-format", "yuv420p");

    switch( mAcdkCctMhalPrvParam.mHDR_EN )
    {
        case 1: //ivhdr: only video mode supports
            set(map, "video-hdr", "on");
            set(map, "rawsave-mode", "4");
            break;
        case 2: //mvhdr: only preview/capture modes support
            set(map, "video-hdr", "on");
            set(map, "rawsave-mode", "1");
            break;
        default:
            set(map, "video-hdr", "off");
            ACDK_LOGD("PreviewSensorScenario=%d", mAcdkCctMhalPrvParam.PreviewSensorScenario);
            switch(mAcdkCctMhalPrvParam.PreviewSensorScenario)
            {
                case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                    set(map, "rawsave-mode", "1");
                break;
                case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                    set(map, "rawsave-mode", "2");
                break;
                case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                    set(map, "rawsave-mode", "4");
                break;
                case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                    set(map, "rawsave-mode", "5");
                break;
                case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                    set(map, "rawsave-mode", "6");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM1:
                    set(map, "rawsave-mode", "7");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM2:
                    set(map, "rawsave-mode", "8");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM3:
                    set(map, "rawsave-mode", "9");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM4:
                    set(map, "rawsave-mode", "10");
                break;
                case SENSOR_SCENARIO_ID_CUSTOM5:
                    set(map, "rawsave-mode", "11");
                break;
                default:
                    set(map, "rawsave-mode", "1");
                break;
            }
    }

#ifdef USING_HIDL
    setParameters_CameDev (flatten(map).string());
#else
    mDevice->ops->set_parameters(mDevice, flatten(map).string());
#endif

    //====== Setting Callback ======
    g_acdkCctMhalObserver = mAcdkCctMhalPrvParam.acdkMainObserver;

    //====== Set State to Preview State ======

    acdkCctMhalSetState(ACDK_CCT_MHAL_PREVIEW);

    //====== Post Sem ======

    ::sem_post(&g_SemMainHigh);

acdkCctMhalPreviewStart_Exit:


    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkCctMhalPreviewStop
* brif : stop preview
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalPreviewStop()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkCctMhalState_e state = acdkCctMhalGetState();

    //====== Check State ======

    //check AcdkCctMhal state
    ACDK_LOGD("state(%d)", state);
    if(state == ACDK_CCT_MHAL_IDLE)
    {
        ACDK_LOGD("is in IDLE state");
        return err;
    }
    else if(state != ACDK_CCT_MHAL_PREVIEW_STOP)
    {
        if(state & ACDK_CCT_MHAL_PREVIEW_MASK)
        {
            acdkCctMhalSetState(ACDK_CCT_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_CCT_MHAL_PRE_CAPTURE)
        {
            // In preCapture, has to wait 3A ready flag
            ACDK_LOGD("it is ACDK_CCT_MHAL_PRE_CAPTURE state");
            acdkCctMhalSetState(ACDK_CCT_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_CCT_MHAL_CAPTURE)
        {
            // It is in capture flow now, preview has been already stopped
            ACDK_LOGD("it is ACDK_CCT_MHAL_CAPTURE state");
            state = acdkCctMhalGetState();
            while(state == ACDK_CCT_MHAL_CAPTURE)
            {
                usleep(20);
                state = acdkCctMhalGetState();
            }

            acdkCctMhalSetState(ACDK_CCT_MHAL_IDLE);
        }
        else
        {
            // Unknown
            ACDK_LOGE("un know state(%d)", state);
        }
    }

    //====== Wait Semaphore ======

    ACDK_LOGD("Wait g_SemMainHighBack");
    ::sem_wait(&g_SemMainHighBack);
    ACDK_LOGD("Got g_SemMainHighBack");

    //====== Stop Preview ======

    //====== Initialize Member Variable =====

    memset(&mAcdkCctMhalPrvParam,0,sizeof(acdkCctMhalPrvParam_t));

    //====== Set Acdk Mhal State ======

    acdkCctMhalSetState(ACDK_CCT_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkCctMhalPreCapture
* brif : change ACDK mhal state to preCapture
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalPreCapture()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Change State ======

    acdkCctMhalSetState(ACDK_CCT_MHAL_PRE_CAPTURE);
    mReadyForCap = MTRUE;

    ACDK_LOGD("-");

    return err;
}


/*******************************************************************************
* acdkCctMhalCaptureStart
* brif : init capture
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalCaptureStart(MVOID *a_pBuffIn)
{
    MINT32 err = ACDK_RETURN_NO_ERROR;
    MUINT32 CaptureMode;
    MUINT32 MFLL_EN;
    MUINT32 HDR_EN;
    eACDK_CAP_FORMAT eImgType;
    char szName[256];

    ACDK_LOGD("+");
    memcpy(&mAcdkCctMhalCapParam, a_pBuffIn, sizeof(acdkCctMhalCapParam_t));
    CaptureMode = mAcdkCctMhalCapParam.CaptureMode;
    eImgType = (eACDK_CAP_FORMAT)mAcdkCctMhalCapParam.mCapType;
    MFLL_EN = mAcdkCctMhalCapParam.mMFLL_EN;
    HDR_EN = mAcdkCctMhalCapParam.mHDR_EN;
    EMultiNR_Mode eMultiNR = (EMultiNR_Mode)mAcdkCctMhalCapParam.mEMultiNR;
    sprintf(szName, "%04dx%04d ", mAcdkCctMhalCapParam.mCapWidth, mAcdkCctMhalCapParam.mCapHeight);

    ACDK_LOGD("Picture Size:%s\n", szName);
    ACDK_LOGD("CaptureMode(%d), eImgType(%d),MFLL_En(%d), eMultiNR(%d)", CaptureMode, eImgType, MFLL_EN, eMultiNR);
    // set parameter
    DefaultKeyedVector<String8,String8> map;
#ifdef USING_HIDL
    {
        string temp;
        getParameters_CameDev (temp);
        String8 str_parms( temp.c_str() );
        unflatten(str_parms, map);
    }
#if 0
    {
        //hidl_string temp
        string temp;
        mCamProvider->getParameters(&temp);
        String8 str_parms( temp.c_str() );
        unflatten(str_parms, map);
    }
#endif
#else
    if (mDevice->ops->get_parameters) {
        char *temp = mDevice->ops->get_parameters(mDevice);
        String8 str_parms(temp);
        if (mDevice->ops->put_parameters)
            mDevice->ops->put_parameters(mDevice, temp);
        else
            free(temp);
        unflatten(str_parms, map);
    }
#endif

    //Set picture size
    set(map, "picture-size", szName);

    // PURE_RAW8_TYPE =0x04, PURE_RAW10_TYPE = 0x08
    // PROCESSED_RAW8_TYPE = 0x10, PROCESSED_RAW10_TYPE = 0x20
    if(eImgType & 0x3C)
    {
        //   rawsave-mode :  1,preview 2,Capture 3,jpeg only 4,video  5,slim video1 6,slim video2
        //                           7,custom1 8,custom2 9,custom3 10,custom4 11,custom5
        //   isp-mode   : 0: process raw, 1:pure raw
        set(map, "camera-mode", "0");
        set(map, "afeng_raw_dump_flag", "1");
        set(map, "rawfname", "/mnt/vendor/cct/");
        switch(eImgType)
        {
            case PURE_RAW8_TYPE:
                set(map, "isp-mode", "1");
                set(map, "rawsave-mode", "2");
                break;
            case PURE_RAW10_TYPE:
                set(map, "isp-mode", "1");
                set(map, "rawsave-mode", "2");
                break;
            case PROCESSED_RAW8_TYPE:
                set(map, "isp-mode", "0");
                set(map, "rawsave-mode", "2");
                break;
            case PROCESSED_RAW10_TYPE:
                set(map, "isp-mode", "0");
                set(map, "rawsave-mode", "2");
                break;
            default:
                break;

        }

    }

    switch(CaptureMode)
    {
         case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            set(map, "rawsave-mode", "1");
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            set(map, "rawsave-mode", "2");
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            set(map, "rawsave-mode", "4");
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            set(map, "rawsave-mode", "5");
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            set(map, "rawsave-mode", "6");
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            set(map, "rawsave-mode", "7");
            break;
  case SENSOR_SCENARIO_ID_CUSTOM2:
      set(map, "rawsave-mode", "8");
      break;
  case SENSOR_SCENARIO_ID_CUSTOM3:
            set(map, "rawsave-mode", "9");
            break;
  case SENSOR_SCENARIO_ID_CUSTOM4:
            set(map, "rawsave-mode", "10");
            break;
  case SENSOR_SCENARIO_ID_CUSTOM5:
            set(map, "rawsave-mode", "11");
            break;
        default:
            set(map, "rawsave-mode", "1");
            break;

    }

    if(MFLL_EN == 1)
    {
        set(map, "mfb", "mfll");
        set(map, "isp-mode", "0");
        set(map, "eng-mfll-pc", "4"); // set num of shot times
    }
    else
        set(map, "mfb", "off");


    if(HDR_EN == 1 || HDR_EN == 2)
        set(map, "video-hdr", "on");
    else
        set(map, "video-hdr", "off");


    set(map, "mnr-e", "1");//enable manual multi-NR

    /* manual multi-NR type:
    0: Disable
    1: HWNR
    2: SWNR
    */
    if(eMultiNR == EMultiNR_Off) set(map, "mnr-t", "0");
    else if (eMultiNR == EMultiNR_HW) set(map, "mnr-t", "1");
    else if (eMultiNR == EMultiNR_SW) set(map, "mnr-t", "2");
    else
    {
        ACDK_LOGD("Error: eMultiNR = %d\n", eMultiNR);
        set(map, "mnr-t", "0");
    }
#ifdef USING_HIDL
    setParameters_CameDev (flatten(map).string());
#else
    mDevice->ops->set_parameters(mDevice, flatten(map).string());
#endif

    //====== Setting Callback ======
    g_acdkCctMhalCapObserver = mAcdkCctMhalCapParam.acdkMainObserver;

#ifdef USING_HIDL
    takePicture_CameDev();
#else
    mDevice->ops->take_picture(mDevice);
#endif

    ACDK_LOGD("-");
    return err;
}


/*******************************************************************************
* acdkCctMhalCaptureStop
* brif : stop capture
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalCaptureStop()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    while(mCaptureDone == MFALSE)
    {
        usleep(200);
    }

    mCaptureDone = MFALSE;

    ACDK_LOGD("-");

    return err;

}

/*******************************************************************************
* acdkCctMhalPreviewProc
* brif : handle flow control of preview
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalPreviewProc()
{
    ACDK_LOGD_DYN(g_acdkCctMhalEngDebug,"+");

    ACDK_LOGD("Preview start");
#ifdef USING_HIDL
    startPreview_CameDev();
#else
    mDevice->ops->start_preview(mDevice);
#endif

    //pass1 continuous
    while((ACDK_CCT_MHAL_PREVIEW_STOP != acdkCctMhalGetState()))
    {
        usleep(500);
    }

#ifdef USING_HIDL
    stopPreview_CameDev();
#else
    mDevice->ops->stop_preview(mDevice);
#endif

    AcdkCctMhalReleaseDevice();

    ACDK_LOGD_DYN(g_acdkCctMhalEngDebug,"-");

    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
* acdkCctMhalCaptureProc
* brif : handle flow control of capture
*******************************************************************************/
MINT32 AcdkCctMhalEng::acdkCctMhalCaptureProc()
{
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkCctMhalEng::acdkCctMhalGetShutTime()
{
return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID AcdkCctMhalEng::acdkCctMhalSetShutTime(MUINT32 a_time)
{
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkCctMhalEng::acdkCctMhalGetAFInfo()
{
    MUINT32 u32AFInfo;

    ACDK_LOGD("acdkCctMhalGetAFInfo");
    u32AFInfo = g_pAcdkCctMHalEngObj->mFocusSucceed;

    return u32AFInfo;

}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkCctMhalEng::acdkCctMhalSetAFMode(MBOOL isAutoMode)
{
    ACDK_LOGD("+");
    ACDK_LOGD("acdkCctMhalSetAFMode : isAutoMode(%d)\n", isAutoMode);

    DefaultKeyedVector<String8,String8> map;
#ifdef USING_HIDL
    {
        string temp;
        getParameters_CameDev (temp);
        String8 str_parms( temp.c_str() );
        unflatten(str_parms, map);
    }
#else
    if (mDevice->ops->get_parameters) {
        char *temp = mDevice->ops->get_parameters(mDevice);
        String8 str_parms(temp);
        if (mDevice->ops->put_parameters)
            mDevice->ops->put_parameters(mDevice, temp);
        else
            free(temp);
        unflatten(str_parms, map);
    }
#endif

    if(isAutoMode)
    {
        set(map, CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_AUTO);
    }
    else
    {
        set(map, CameraParameters::KEY_FOCUS_MODE, CameraParameters::FOCUS_MODE_INFINITY);
    }
    
#ifdef USING_HIDL
    setParameters_CameDev (flatten(map).string());
#else
    mDevice->ops->set_parameters(mDevice, flatten(map).string());
#endif
    ACDK_LOGD("-");
    
    return MTRUE;
}

/*******************************************************************************
* doNotifyCb
*******************************************************************************/
void AcdkCctMhalEng::doNotifyCb(int32_t _msgType,
                                 int32_t _ext1,
                                 int32_t _ext2,
                                 int32_t _ext3)
{
    ACDK_LOGD("_msgType(%d),_ext1(%d)",_msgType,_ext1);

    ACDK_LOGD("-");
}

/*******************************************************************************
* doDataCb
*******************************************************************************/
void AcdkCctMhalEng::doDataCb(int32_t _msgType,
                               void *_data,
                               uint32_t _size)
{

}

