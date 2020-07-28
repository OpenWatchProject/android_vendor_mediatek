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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "HDR_ProviderImpl"
#define DEBUG_LOG_TAG "HDR_ProviderImpl"

#include <utils/std/Log.h>
#include <utils/std/Format.h>
#include <stdlib.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <sstream>
#include <sys/stat.h> // mkdir
#include <sys/prctl.h> //prctl set name

#include <mtk/mtk_platform_metadata_tag.h>
#include "plugin/PipelinePlugin.h"
#include "plugin/PipelinePluginType.h"
#include <cutils/properties.h>
#include "BufferUtils.h"
#include "vndk/hardware_buffer.h"
#include <HDRProc2.h>
#include <Debug.h>
// HDR library
#include <ImageBufferUtils.h>
//
using ::vendor::mediatek::hardware::mms::V1_2::IMms;
using ::vendor::mediatek::hardware::mms::V1_0::HwCopybitParam;
using namespace NSCam;
using namespace android;
using namespace std;
using namespace com::mediatek::campostalgo::NSFeaturePipe;
using namespace NSCam::NSPipelinePlugin;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)
//
#include <cutils/properties.h>
#define LOG_DETAILS "debug.camerapostalgo.feature.hdr.logdetails"
static int8_t gLogDetail = ::property_get_bool(LOG_DETAILS, 0);
#define FUNCTION_IN   do { if (gLogDetail) MY_LOGD("%s +", __FUNCTION__); } while(0)
#define FUNCTION_OUT  do { if (gLogDetail) MY_LOGD("%s -", __FUNCTION__); } while(0)
//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define HDR_TRACE_CALL()                      ATRACE_CALL()
#define HDR_TRACE_NAME(name)                  ATRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define HDR_TRACE_END()                       ATRACE_END()
#else
#define HDR_TRACE_CALL()
#define HDR_TRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)
#define HDR_TRACE_END()
#endif
#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)

static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
        bool bEnableLog = !!::property_get_int32("vendor.debug.camera.hdr.log", 0);
            char* pText = const_cast<char*>(functionName);
                CAM_LOGD_IF(bEnableLog, "[%s] + ", pText);
                    return std::shared_ptr<char>(pText, [bEnableLog](char* p){ CAM_LOGD_IF(bEnableLog, "[%s] -", p); });
}

/******************************************************************************
*
******************************************************************************/
class HDRProviderImpl : public MultiFramePlugin::IProvider
{
    typedef MultiFramePlugin::Property Property;
    typedef MultiFramePlugin::Selection Selection;
    typedef MultiFramePlugin::Request::Ptr RequestPtr;
    typedef MultiFramePlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    virtual void set(MINT32 iOpenId, MINT32 iOpenId2)
    {
        HDR_TRACE_CALL();
        FUNCTION_SCOPE;
        MY_LOGD("set openId:%d openId2:%d", iOpenId, iOpenId2);
        m_openId = iOpenId;
    }

    virtual const Property& property()
    {
        HDR_TRACE_CALL();
        FUNCTION_SCOPE;
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName = "MTK_HDR";
            prop.mFeatures = MTK_FEATURE_HDR;
            //Feature priority
            prop.mPriority = ePriority_Highest;
            inited = true;
        }
        return prop;
    };

    //if capture number is 4, "negotiate" would be called 4 times
    virtual MERROR negotiate(Selection& sel)
    {
        HDR_TRACE_CALL();
        FUNCTION_SCOPE;
        MY_LOGD("negotiate");
        if(m_enable == 0) {
            MY_LOGD("Force off HDR");
            return BAD_VALUE;
        }
        IMetadata* appInMeta = sel.mIMetadataApp.getControl().get();

        MSize picture_size;
        IMetadata::getEntry<MSize>(
                      appInMeta, MTK_POSTALGO_PICTURE_SIZE, picture_size);

        MY_LOGD("HDR picture size w(%d), h(%d)", picture_size.w, picture_size.h);

        if (sel.mRequestIndex == 0) {
            if(m_enabletkhdr) {
                MY_LOGD_IF(m_enablelog, "init HDR proc");
                // initialize HDR proc
                HDRProc2& hdrproc(HDRProc2::getInstance());
                if (hdrproc.init(m_openId, m_HDRHandle) != MTRUE)
                {
                    MY_LOGE("init HDR proc failed");
                    return BAD_VALUE;
                }
                // get HDR handle and set complete callback
                hdrproc.setCompleteCallback(m_HDRHandle, HDRProcCompleteCallback, this);

                MRect cropRegion = NULL;
                MSize postviewSize(800, 600);//invalid param
                hdrproc.setShotParam(m_HDRHandle, picture_size, postviewSize, cropRegion);
                hdrproc.setParam(m_HDRHandle, HDRProcParam_Set_sensor_type, 1 /*SENSOR_TYPE_RAW*/, 0);
                hdrproc.prepare(m_HDRHandle);
                {
                    // get format and size param from HDRProc
                    MUINT32 empty = 0;
                    hdrproc.getParam(
                            m_HDRHandle,
                            HDRProcParam_Get_src_small_format, m_uSrcSmallFormat, empty);
                    hdrproc.getParam(
                            m_HDRHandle,
                            HDRProcParam_Get_src_small_size, m_uSrcSmallWidth, m_uSrcSmallHeight);
                    MY_LOGD_IF(m_enablelog, "HDR small YUV Format(0x%x) Size(%u,%u)",m_uSrcSmallFormat ,m_uSrcSmallWidth ,m_uSrcSmallHeight);
                }
            }
        }
        sel.mRequestCount = m_captureNum;

        if(m_enabletkhdr) {
            sel.mIBufferFull
                .setRequired(MTRUE)
                .addAcceptedFormat(eImgFmt_YV12)
                .addAcceptedSize(eImgSize_Full);
        }

        //Only main frame has output buffer
        if (sel.mRequestIndex == 0) {
            if(m_enabletkhdr) {
                sel.mOBufferFull
                    .setRequired(MTRUE)
                    .addAcceptedFormat(eImgFmt_YV12)
                    .addAcceptedSize(eImgSize_Full);
                sel.mIMetadataApp.setRequired(MTRUE);
            }
        } else {
            sel.mOBufferFull.setRequired(MFALSE);
            sel.mOMetadataApp.setRequired(MFALSE);
            sel.mOMetadataHal.setRequired(MFALSE);
        }

        return OK;
    };

    virtual void init()
    {
        FUNCTION_SCOPE;
    };

    virtual MERROR process(RequestPtr pRequest,
                           RequestCallbackPtr pCallback)
    {
        HDR_TRACE_CALL();
        FUNCTION_SCOPE;
        //set thread's name
        ::prctl(PR_SET_NAME, "HDRPlugin", 0, 0, 0);
        // restore callback function for abort API
        if(pCallback != nullptr) {
            m_callbackprt = pCallback;
        }
        mvRequests.push_back(pRequest);
        MY_LOGD("collected request(%d/%d)",
                pRequest->mRequestIndex+1,
                pRequest->mRequestCount);

        if (pRequest->mIBufferFull != nullptr) {
            IImageBuffer* pIImgBuffer = pRequest->mIBufferFull->acquire();
            MY_LOGI("[IN] Full image VA: 0x%p", pIImgBuffer->getBufVA(0));
            if (m_dump){
                int result = mkdir(HDR_DUMP_PATH, S_IRWXU | S_IRWXG | S_IRWXO);
                if ((result == -1) && (errno != EEXIST)) {
                    MY_LOGW("mkdir fail, error %d, return", errno);
                }
                // dump input buffer
                String8 fileResultName;
                char    pre_filename[512] = HDR_DUMP_PATH;

                fileResultName = String8::format("%sYV12_Input_%d_%dx%d.yv12"
                    , pre_filename
                    , pRequest->mRequestIndex
                    , pIImgBuffer->getImgSize().w
                    , pIImgBuffer->getImgSize().h);
                pIImgBuffer->saveToFile(fileResultName);
                MY_LOGI("[DUMP][IN] Dump Input Buffer Name");
            }
        }

        if (pRequest->mOBufferFull != nullptr) {
            IImageBuffer* pOImgBuffer = pRequest->mOBufferFull->acquire();
            MY_LOGD("[OUT] Full image VA: 0x%p", pOImgBuffer->getBufVA(0));
        }

        if (pRequest->mRequestIndex == pRequest->mRequestCount - 1 )
        {
            if (mvRequests.size() < pRequest->mRequestCount) {
                MY_LOGE("lost some requests");
                for (auto req : mvRequests) {
                    if (pCallback != nullptr) {
                        pCallback->onCompleted(req, 0);
                    }
                }
                if(m_enabletkhdr) {
                    cleanUp();
                }
                mvRequests.clear();
            }
            else {
                MY_LOGD("have collected all requests");

                if(m_enabletkhdr) {
                    MY_LOGD_IF(m_enablelog, "start do HDR +");
                    m_hdrDo = true;
                    doHdr();
                    MY_LOGD_IF(m_enablelog, "start do HDR -");
                }
                for (auto req : mvRequests) {
                    MY_LOGI("callback request(%d/%d) %p",
                            req->mRequestIndex+1,
                            req->mRequestCount, pCallback.get());
                    if (pCallback != nullptr) {
                        pCallback->onCompleted(req, 0);
                    }
                }
                if(m_enabletkhdr) {
                    m_hdrDo = false;
                    cleanUp();
                }
            }
            mvRequests.clear();
        }
        return 0;
    };

    virtual void abort(vector<RequestPtr>& pRequests)
    {
        HDR_TRACE_CALL();
        FUNCTION_SCOPE;
        bool babort = false;
        if(m_callbackprt == nullptr)
            MY_LOGW("callbackptr is null");

        // if HDR is processing, then we wait for completecallback
        if(m_enabletkhdr) {
            if(m_hdrDo) {
                MY_LOGW("abort is not executed due to HDR process is already started");
                return;
            }
        }
        for (auto req : pRequests){
            babort = false;
            for (std::vector<RequestPtr>::iterator it = mvRequests.begin() ; it != mvRequests.end(); it++){
                if((*it) == req){
                    mvRequests.erase(it);
                    m_callbackprt->onAborted(req);
                    babort = true;
                    break;
                }
            }
            if (!babort){
                MY_LOGW("Desire abort request[%d] is not found", req->mRequestIndex);
            }
        }
        if(mvRequests.empty()) {
            if(m_enabletkhdr) {
                cleanUp();
            }
            MY_LOGD("abort() cleans all the requests");
        } else {
            MY_LOGW("abort() does not clean all the requests");
        }
    };

    virtual void uninit()
    {
        HDR_TRACE_CALL();
        FUNCTION_SCOPE;
    };

    HDRProviderImpl()
        : m_openId(0)
        //, mvCapParam()
        //, m_CurrentCapParams()
        //, m_DelayCapParams()
        , m_captureNum(3)
        , m_delayFrame(0)
        , m_dump(0)
        , m_enable(-1)
        //, m_dumpNamingHint()
        , mvRequests()
        , m_collectmode(0)
        , m_manualAE(true)
        , m_hdrDone(false)
        , m_hdrDo(false)
    {
        HDR_TRACE_CALL();
        FUNCTION_SCOPE;

        // enable HDR plugin
        m_enable = ::property_get_int32("vendor.debug.camera.hdr.enable", -1);
        // turn on collect mode
        m_collectmode = ::property_get_int32("debug.raw.collect", 0);
        // enable input/output dump buffer
        m_dump = ::property_get_int32("vendor.debug.camera.hdr.dump", 0);
        // enable print debug log
        m_enablelog = ::property_get_int32("vendor.debug.camera.hdr.log", 0);

        m_enabletkhdr = ::property_get_int32("vendor.debug.camera.tkhdr.enable", 1);

        if(m_collectmode)
            MY_LOGD("collect mode on");
        else if(m_enabletkhdr)
            MY_LOGD("tk hdr mode on");
        else
            MY_LOGD("3rd party hdr mode on");
    };

    virtual ~HDRProviderImpl() noexcept
    {
        HDR_TRACE_CALL();
        FUNCTION_SCOPE;
    };

private:

    bool doHdr()
    {
        FUNCTION_SCOPE;
        HDR_TRACE_CALL();
        HDRProc2& hdrproc(HDRProc2::getInstance());
        for (size_t i = 0; i < static_cast<size_t>(mvRequests.size()); i++) {

            RequestPtr pRequest = mvRequests.at(i);
            if (pRequest->mIBufferFull != nullptr) {
                MY_LOGD_IF(m_enablelog, "[IN] Original Full image VA: 0x%p Fmt: 0x%x Size:(%d,%d)",
                    pRequest->mIBufferFull->acquire()->getBufVA(0),pRequest->mIBufferFull->acquire()->getImgFormat(),
                    pRequest->mIBufferFull->acquire()->getImgSize().w, pRequest->mIBufferFull->acquire()->getImgSize().h);
            }

            // alloc I420 working buffer
            ImageBufferUtils::getInstance().allocBuffer(m_inputBuffer[i],
                    pRequest->mIBufferFull->acquire()->getImgSize().w, pRequest->mIBufferFull->acquire()->getImgSize().h, eImgFmt_I420);

            if (m_inputBuffer[i] != nullptr) {
                MY_LOGD_IF(m_enablelog, "[IN] MDP I420 input image VA: 0x%p Fmt: 0x%x Size:(%d,%d)",
                        m_inputBuffer[i]->getBufVA(0),m_inputBuffer[i]->getImgFormat(),
                        m_inputBuffer[i]->getImgSize().w, m_inputBuffer[i]->getImgSize().h);
            }
            //covert I420 buffer by mdp
            BufferUtils::mdpResizeAndConvert(pRequest->mIBufferFull->acquire(),
                    m_inputBuffer[i].get());
            if (m_dump){
                // dump I420 input buffer
                String8 fileResultName;
                char    pre_filename[512] = HDR_DUMP_PATH;

                fileResultName = String8::format("%sI420_Input_%d_%dx%d.i420"
                      , pre_filename
                      , pRequest->mRequestIndex
                      , m_inputBuffer[i]->getImgSize().w
                      , m_inputBuffer[i]->getImgSize().h);
                m_inputBuffer[i]->saveToFile(fileResultName);
                MY_LOGI("[DUMP][IN] Dump I420 Input Buffer");
            }
            // alloc Y8 working buffer
            ImageBufferUtils::getInstance().allocBuffer(m_specifiedBuffer[i],
                    m_uSrcSmallWidth, m_uSrcSmallHeight, m_uSrcSmallFormat);
            if (m_specifiedBuffer[i] != nullptr) {
                MY_LOGD_IF(m_enablelog, "[IN] Y8 small image VA: 0x%p Fmt: 0x%x Size:(%d,%d)",
                        m_specifiedBuffer[i]->getBufVA(0),m_specifiedBuffer[i]->getImgFormat(),
                        m_specifiedBuffer[i]->getImgSize().w, m_specifiedBuffer[i]->getImgSize().h);
            }
            // Convert Y8 small size buffer by mdp
            BufferUtils::mdpResizeAndConvert(pRequest->mIBufferFull->acquire(),
                    m_specifiedBuffer[i].get());
            if (m_dump) {
                // dump Y8 input buffer
                String8 fileResultName;
                char pre_filename[512] = HDR_DUMP_PATH;

                fileResultName = String8::format("%sY8_Input_%d_%dx%d.y8",
                        pre_filename, pRequest->mRequestIndex,
                        m_specifiedBuffer[i]->getImgSize().w,
                        m_specifiedBuffer[i]->getImgSize().h);
                m_specifiedBuffer[i]->saveToFile(fileResultName);
                MY_LOGI("[DUMP][IN] Dump Y8 Small Buffer");
            }
            {
                size_t index = i << 1;
                hdrproc.addInputFrame(m_HDRHandle, index+0, m_inputBuffer[i]);
                hdrproc.addInputFrame(m_HDRHandle, index+1, m_specifiedBuffer[i]);
            }
        } // for-loop: for every request
        // do HDR
        {
            hdrproc.start(m_HDRHandle);

            // wait Hdr done
            std::unique_lock<std::mutex> locker(m_HdrDoneMx);
            MY_LOGI("Wait Hdr");
            if(!m_hdrDone)
            {
                m_HdrDoneCond.wait(locker);
            }

            //copy to output buffer
            {
                RequestPtr pRequest = mvRequests.at(0);

                IImageBuffer* pOImgBuffer = nullptr;
                if (pRequest->mOBufferFull != nullptr) {
                    pOImgBuffer = pRequest->mOBufferFull->acquire();
                    MY_LOGD_IF(m_enablelog, "[OUT] Full image VA: 0x%p Fmt: 0x%x Size:(%d,%d)",
                        pOImgBuffer->getBufVA(0),pOImgBuffer->getImgFormat(),
                        pOImgBuffer->getImgSize().w, pOImgBuffer->getImgSize().h );
                }

                MY_LOGD("[OUT] Result image VA: 0x%p Fmt: 0x%x Size:(%d,%d)",
                    (m_HdrResult.get())->getBufVA(0),(m_HdrResult.get())->getImgFormat(),
                    (m_HdrResult.get())->getImgSize().w, (m_HdrResult.get())->getImgSize().h );
                if (m_dump){
                     // dump input buffer
                     String8 fileResultName;
                     char    pre_filename[512] = HDR_DUMP_PATH;

                     fileResultName = String8::format("%sAlgo_result_%d_%dx%d.yuy2"
                         , pre_filename
                         , pRequest->mRequestIndex
                         , m_HdrResult.get()->getImgSize().w
                         , m_HdrResult.get()->getImgSize().h);
                     m_HdrResult.get()->saveToFile(fileResultName);
                    MY_LOGI("[DUMP][OUT] Dump Result Buffer");
                 }
                // MDP copy
                bool mirror = getMirror(pRequest, getFacing(pRequest));
                BufferUtils::mdpResizeAndConvert(m_HdrResult.get(),
                        pRequest->mOBufferFull->acquire(), getJpegRotation(pRequest),mirror);
            }
        }
        return true;
    }
    int getFacing(RequestPtr pRequest) {
        MINT32 facing = 0;
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata != nullptr && pImetadata->count() > 0) {
            IMetadata::getEntry<MINT32>(
                    			pImetadata, MTK_POSTALGO_LENS_FACING, facing);
        }
        return facing;
    }

    bool getMirror(RequestPtr pRequest,int facing) {
        if (facing != 0) {
            return false;
        }
        MINT32 mirror = 0;
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata != nullptr && pImetadata->count() > 0) {
            IMetadata::getEntry<MINT32>(
                    			pImetadata, MTK_POSTALGO_FACE_BEAUTY_MIRROR, mirror);
        }
        return mirror == 0;
    }

    int getJpegRotation(RequestPtr pRequest) {
        MINT32 jpegRotation = 0;
        IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata != nullptr && pImetadata->count() > 0) {
        	IMetadata::getEntry<MINT32>(
        			pImetadata, MTK_POSTALGO_JPEG_ORIENTATION, jpegRotation);
            MY_LOGI("[getJpegRotation] jpegRotation:%d", jpegRotation);
        }
        return jpegRotation;
    }
    static MBOOL HDRProcCompleteCallback(void* user, const sp<IImageBuffer>& hdrResult, MBOOL ret)
    {
        FUNCTION_SCOPE;
        HDRProviderImpl* self = reinterpret_cast<HDRProviderImpl*>(user);
        if (NULL == self) {
            MY_LOGE("HDRProcCompleteCallback with NULL user");
            return MFALSE;
        }

        MY_LOGI("HDRProcCompleteCallback ret(%d)", ret);

        self->setResultBuffer(hdrResult);
        return MTRUE;
    }
    void cleanUp()
    {
        FUNCTION_SCOPE;
        HDRProc2& hdrproc(HDRProc2::getInstance());
        // temp
        if(m_HdrResult != nullptr)
            ImageBufferUtils::getInstance().deallocBuffer(m_HdrResult);
        for (size_t i = 0; i < static_cast<size_t>(mvRequests.size()); i++) {
            if (m_specifiedBuffer[i] != nullptr) {
                ImageBufferUtils::getInstance().deallocBuffer(m_specifiedBuffer[i]);
                m_specifiedBuffer[i] = NULL;
            }
            if (m_inputBuffer[i] != nullptr) {
                ImageBufferUtils::getInstance().deallocBuffer(m_inputBuffer[i]);
                m_inputBuffer[i] = NULL;
            }
        }

        m_HdrResult        = nullptr;
        m_hdrDone          = false;
        m_hdrDo            = false;

        // release hdrproc
        const HDRHandle& hdrHandle(m_HDRHandle);

        hdrproc.release(m_HDRHandle);
        hdrproc.uninit(m_HDRHandle);
    }
    void setResultBuffer(sp<IImageBuffer> b)
    {
        FUNCTION_SCOPE;

        std::lock_guard<std::mutex> __l(m_HdrDoneMx);
        m_HdrResult = b;
        m_hdrDone = true;
        m_HdrDoneCond.notify_one();
    }

    //
    int                          m_openId;
    int                          m_captureNum;
    int                          m_delayFrame;
    int                          m_dump;
    int                          m_enable;
    int                          m_enabletkhdr;
    int                          m_enablelog;
    int                          m_collectmode;
    bool                         m_manualAE;
    RequestCallbackPtr           m_callbackprt;
    // file dump hint
    //FILE_DUMP_NAMING_HINT        m_dumpNamingHint;
    // collect request
    std::vector<RequestPtr>      mvRequests;
    //======== HDR Proc2 =======
    // a handle get from HDRProc, used to communicate with HDR HAL
    HDRHandle   m_HDRHandle;
    //TODO: remove hard code
    sp<IImageBuffer> m_specifiedBuffer[3];
    sp<IImageBuffer> m_inputBuffer[3];
    MUINT32 m_uSrcSmallFormat = 0;
    MUINT32 m_uSrcSmallWidth = 0;
    MUINT32 m_uSrcSmallHeight = 0;
    // HDR final result
    sp<IImageBuffer>         m_HdrResult;
    mutable std::mutex       m_HdrDoneMx;
    std::condition_variable  m_HdrDoneCond;
    bool                     m_hdrDone;
    bool                     m_hdrDo;
};

REGISTER_PLUGIN_PROVIDER(MultiFrame, HDRProviderImpl);

