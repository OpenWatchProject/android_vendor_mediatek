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

#define LOG_TAG "ABFPlugin"
#include "ABFImpl.h"
//
#include <mtkcam/utils/std/Log.h>
//#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/utils/std/Trace.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
//
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/std/Format.h>
//
#include <cutils/properties.h>

#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <mtkcam/def/ImageFormat.h>


#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <camera_custom_nvram.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif
//#include <camera_custom_capture_nr.h>
using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::TuningUtils; //dump buffer
//#define eImgFmt_I420 0x2008



/******************************************************************************
 *
 ******************************************************************************/
//CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_SWNR);
#define MY_LOGV(fmt, arg...)        ALOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        ALOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        ALOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        ALOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
//
#define FUNCTION_IN                 MY_LOGD("%s +", __FUNCTION__)
#define FUNCTION_OUT                MY_LOGD("%s -", __FUNCTION__)
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)
#define CHECK_OBJECT_EXIT(x) do{                                                  \
        if(x == nullptr) { MY_LOGE("NULL %s Object", #x); return false; }         \
        return true;                                                              \
} while(0)

#define DEBUG_MODE (1)

#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)

static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    bool bEnableLog = !!::property_get_int32("vendor.debug.camera.Abf.log", 0);
    char* pText = const_cast<char*>(functionName);
    MY_LOGD_IF(bEnableLog, "[%s] + ", pText);
    return std::shared_ptr<char>(pText, [bEnableLog](char* p){ MY_LOGD_IF(bEnableLog, "[%s] -", p); });
}

template <class T>
inline bool
tryGetMetadata( IMetadata const *pMetadata, MUINT32 tag, T& rVal )
{
    if(pMetadata == nullptr) return MFALSE;

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if(!entry.isEmpty())
    {
        rVal = entry.itemAt(0,Type2Type<T>());
        return true;
    }
    else
    {
#define var(v) #v
#define type(t) #t
        MY_LOGW("no metadata %s in %s", var(tag), type(pMetadata));
#undef type
#undef var
    }
    return false;
}
/******************************************************************************
 *
 ******************************************************************************/
REGISTER_PLUGIN_PROVIDER(Yuv, AbfPluginProviderImp);

/******************************************************************************
 *
 ******************************************************************************/
 #if 1
 void
 AbfPluginProviderImp::
 waitForIdle()
{
    FUNCTION_SCOPE;
    onProcessFuture();
    return;
}
 #endif
/******************************************************************************
 *
 ******************************************************************************/

AbfPluginProviderImp::
AbfPluginProviderImp()
{
    FUNCTION_SCOPE;
    muDumpBuffer = ::property_get_int32("vendor.debug.camera.dump.abf", 0);
    mEnable = ::property_get_int32("vendor.debug.camera.abf.enable", -1); // -1
}

/******************************************************************************
 *
 ******************************************************************************/
AbfPluginProviderImp::
~AbfPluginProviderImp()
{
    FUNCTION_SCOPE;
}

/******************************************************************************
 *
 ******************************************************************************/
void
AbfPluginProviderImp::
set(MINT32 iOpenId,MINT32 iOpenId2)
{
    MY_LOGD("set openId:%d openId2:%d", iOpenId, iOpenId2);
    mOpenId = iOpenId;
}

/******************************************************************************
 *
 ******************************************************************************/
const
AbfPluginProviderImp::Property&
AbfPluginProviderImp::
property()
{
    FUNCTION_SCOPE;
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName = "MTK ABF";
        prop.mFeatures = MTK_FEATURE_ABF;
        prop.mInPlace = MTRUE;
        prop.mFaceData = eFD_None;
        prop.mPosition = 1;
        prop.mPriority = ePriority_Highest;
        inited = true;
    }

    return prop;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
AbfPluginProviderImp::
isAbfEnable(IMetadata* pIMetadata_App)
{
  MINT64 abfmode;
  bool result;

  abfmode = MTK_ABF_MODE_OFF;
  result = false;
  switch(mEnable)
  {
    case 0:
      result = false;
      break;
    case 1:
      result = true;
      break;
    default:
      if(CC_LIKELY(pIMetadata_App != NULL)){
        if(!tryGetMetadata<MINT64>(pIMetadata_App, MTK_ABF_FEATURE_ABF_MODE, abfmode)){
          if(MTK_ABF_MODE_ON == abfmode){
            result = true;
          }
        }
      }
      break;
  }
  return result;
}
 /******************************************************************************
 *
 ******************************************************************************/
MERROR
AbfPluginProviderImp::
negotiate(Selection& sel)
{
    FUNCTION_SCOPE;
    if (sel.mIMetadataDynamic.getControl() != NULL) {
        IMetadata* pIMetadata_P1 = sel.mIMetadataDynamic.getControl().get();
        //IMetadata* pIMetadata_Hal = sel.mIMetadataHal.getControl().get();
      IMetadata* pIMetadata_App = sel.mIMetadataApp.getControl().get();
      if(isAbfEnable(pIMetadata_App) != true)
      {
        return -EINVAL;
      }
      MINT32 iso = 0;
      tryGetMetadata<MINT32>(pIMetadata_P1, MTK_SENSOR_SENSITIVITY, iso);

      //MY_LOGD("current ISP profile = %d", mIspProfile);
      if (mpAbf == NULL) {
        mpAbf = getAbfAdapter();
      }
      AbfAdapter::ProcessParam AbfParam;
      AbfParam.iso    = iso;
      if(mpAbf != NULL){
        if (!mpAbf->needAbf(AbfParam)) {
          return -EINVAL;
        }
      }
      else{
        return -EINVAL;
      }
    }
  else{
    return -EINVAL;
  }

    MUINT32 format = eImgFmt_I420;

    sel.mIBufferFull
        .setRequired(true)
        .addAcceptedFormat(format)
        .addAcceptedSize(eImgSize_Full);


    sel.mIMetadataDynamic.setRequired(true);
    sel.mIMetadataApp.setRequired(true);
    sel.mIMetadataHal.setRequired(true);
    sel.mOMetadataApp.setRequired(false);
    sel.mOMetadataHal.setRequired(true);

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
void
AbfPluginProviderImp::
init()
{
    FUNCTION_SCOPE;
    mbRequestExit = MFALSE;

    if(!mThread.valid()) {
         MY_LOGD("create new thread +");
         mThread = std::async(std::launch::async, [this]() {
            MY_LOGD("run in new thread....");
            // TODO: workaround for init time has no OpenId info
           if (mpAbf == NULL) {
             mpAbf = getAbfAdapter();
           }
           #if 1
           while ( this->onDequeRequest()) {
             this->onProcessFuture();
             Mutex::Autolock _l(this->mFutureLock);
             if(mbRequestExit) {
               MY_LOGD("request to exit.");
               break;
             }
           }
           #endif
         }
                             );
        MY_LOGD("create new thread -");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
 AbfAdapter*
 AbfPluginProviderImp::
 getAbfAdapter()
 {
   if (mpAbf == NULL) {
     MY_LOGD("debug: create instance openId %d", mOpenId);
     if(mOpenId < 0 ) {
       MY_LOGE("need set openId before init() !!");
     }
     else {
       mpAbf = new AbfAdapter(mOpenId);
     }
   }
   return mpAbf;
 }
 /******************************************************************************
 *
 ******************************************************************************/
MERROR
AbfPluginProviderImp::
process(RequestPtr pRequest,
        RequestCallbackPtr pCallback)
{
    FUNCTION_SCOPE;
    MERROR r = OK;
    // 1. check params
    if(!pRequest->mIBufferFull.get()) {
        if (pCallback.get() != nullptr) {
            pCallback->onCompleted(pRequest, 0);
            goto lbExit;
        }
    }
    MY_LOGD("debug: new test");
    if(mpAbf ==NULL)
      mpAbf = getAbfAdapter();//get AbfAdapter instance

    // 2. enque
    {

      int debug_sync = ::property_get_int32("debug.plugin.Abf.sync", 0);
      #if 0
      if(pCallback.get()) {
        r = doAbf(pRequest);
        if(pCallback.get())
          pCallback->onCompleted(pRequest, r);
        return r;
      }
      else
      {
        return false;
      }
#endif
        // sync call
        #if 1
        if((!pCallback.get()) || debug_sync) {
            MY_LOGD("Abf call..");
            r = doAbf(pRequest);
            if(pCallback.get())
                pCallback->onCompleted(pRequest, r);
        }
        else {
            MY_LOGD("Abf async call...");
            Mutex::Autolock _l(mFutureLock);
            mvFutures.insert(
                    std::pair<RequestPtr, std::future<int32_t>>(pRequest,
                    std::async(std::launch::deferred,
                        [=]() -> int32_t {
                            ::prctl(PR_SET_NAME, (unsigned long)"Cam@doABF", 0, 0, 0);
                            int32_t err = OK;
                            err = this->doAbf(pRequest);
                            if(pCallback.get() != nullptr)
                                pCallback->onCompleted(pRequest, err);
                            return err;
                        }
                    ))
            );
            mFutureCond.signal();
        }
//else
#endif
    }

lbExit:
    return r;
}

/******************************************************************************
 *
 ******************************************************************************/
void
AbfPluginProviderImp::
abort(vector<RequestPtr>& pRequests)
{
    FUNCTION_SCOPE;
    MY_LOGD("because bg service.skip");
#if 1
    Mutex::Autolock _l(mFutureLock);
    int idx = 0;
    while ( !pRequests.empty())
    {
        for (map<RequestPtr, std::future<int32_t> >::iterator it = mvFutures.begin() ; it != mvFutures.end(); )
        {
            if((*it).first == pRequests[idx]) {
                it = mvFutures.erase(it);
            }

        }
        idx++;

    }
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
void
AbfPluginProviderImp::
uninit()
{
    FUNCTION_SCOPE;

    if(mpAbf) {
        MY_LOGD("debug: delete instance openId %d", mOpenId);
        delete mpAbf;
        mpAbf = nullptr;
    }
#if 1
    if(!mvFutures.empty())
    {
        MY_LOGE("remain %zu requests...", mvFutures.size());
        waitForIdle();
    }
#endif
    {
        Mutex::Autolock _l(mFutureLock);
        mbRequestExit = MTRUE;
        mFutureCond.signal();
    }
    // Make sure mThread is finish
    {
        if(mThread.valid()) {
            MY_LOGD("Uninit make sure mThread finish+");
            mThread.get();
            MY_LOGD("Uninit make sure mThread finish-");
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/
#if 1
bool
AbfPluginProviderImp::
onDequeRequest()
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mFutureLock);
    while ( mvFutures.empty() && ! mbRequestExit)
    {
        MY_LOGD("ABF onDequeRequest waiting ...");
        status_t status = mFutureCond.wait(mFutureLock);
        MY_LOGD("ABF onDequeRequest waiting done");
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mvFutures.size:%zu",
                status, ::strerror(-status), mvFutures.size() );
        }
    }
    if  ( mbRequestExit ) {
        MY_LOGW_IF(1/*!mvFutures.empty()*/, "[flush] mvFutures.size:%zu", mvFutures.size());
        return false;
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
#endif
#if 1
bool
AbfPluginProviderImp::
onProcessFuture()
{
    FUNCTION_SCOPE;
    std::future<int32_t> task;

    for (map<RequestPtr, std::future<int32_t> >::iterator it = mvFutures.begin() ; it != mvFutures.end(); )
    {
        // handle queue in processing time
        {
            Mutex::Autolock _l(mFutureLock);
            task = std::move((*it).second);
            it = mvFutures.erase(it);
        }
        if(task.valid())
        {
            int32_t status = task.get(); // processing
            MY_LOGE_IF(status!=OK, "status: %d:%s", status, ::strerror(-status));
        }
        else
            MY_LOGW("task is not valid?");
    }
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
#endif
int32_t
AbfPluginProviderImp::
doAbf(
    RequestPtr const              pRequest
)
{
    //CAM_ULOGM_TAGLIFE("AbfPluginProviderImp:doAbf");
    FUNCTION_SCOPE;
    int32_t err = OK;

    // buffer
    sp<IImageBuffer> pIBuffer_MainFull = nullptr;
    sp<IImageBuffer> pOBuffer_Full = nullptr;

    if (pRequest->mIBufferFull != nullptr) {
        pIBuffer_MainFull = pRequest->mIBufferFull->acquire( eBUFFER_USAGE_SW_READ_MASK | eBUFFER_USAGE_SW_WRITE_MASK );
        MY_LOGD_IF(DEBUG_MODE, "cache invalid+");
        pIBuffer_MainFull->syncCache(eCACHECTRL_INVALID);
        MY_LOGD_IF(DEBUG_MODE, "cache invalid+");
        MY_LOGD("\tMain Full img VA: 0x%p", (void*)pIBuffer_MainFull->getBufVA(0));
    }

    if (pRequest->mOBufferFull != nullptr) {
        pOBuffer_Full = pRequest->mOBufferFull->acquire();
        MY_LOGD("\tOut Full img VA: 0x%p", (void*)pOBuffer_Full->getBufVA(0));
    }

    // meta
    IMetadata* pIMetadata_P1  = nullptr;
    IMetadata* pIMetadata_App = nullptr;
    IMetadata* pIMetadata_Hal = nullptr;
    IMetadata* pOMetadata_Hal = nullptr;
    if (CC_LIKELY(pRequest->mIMetadataDynamic != nullptr)) {
        pIMetadata_P1 = pRequest->mIMetadataDynamic->acquire();
        if(pIMetadata_P1 != nullptr)
            MY_LOGD("\tIn APP P1 meta count: 0x%x", pIMetadata_P1->count());
    }
    if (CC_LIKELY(pRequest->mIMetadataApp != nullptr)) {
        pIMetadata_App = pRequest->mIMetadataApp->acquire();
        if(pIMetadata_App != nullptr)
            MY_LOGD("\tIn APP meta count: 0x%x", pIMetadata_App->count());
    }
    if (CC_LIKELY(pRequest->mIMetadataHal != nullptr)) {
        pIMetadata_Hal = pRequest->mIMetadataHal->acquire();
        if(pIMetadata_Hal != nullptr)
            MY_LOGD("\tIn HAL meta count: 0x%x", pIMetadata_Hal->count());
    }
    if (pRequest->mOMetadataHal != nullptr && pIMetadata_Hal != nullptr) {
        pOMetadata_Hal = pRequest->mOMetadataHal->acquire();
    }

    // dump hint
    FILE_DUMP_NAMING_HINT hint;
    MSize buffSize = pIBuffer_MainFull->getImgSize();
    hint.ImgFormat = pIBuffer_MainFull->getImgFormat();
    hint.ImgWidth  = buffSize.w;
    hint.ImgHeight = buffSize.h;
    // No need to dump input when lpcnr is processing
    if(muDumpBuffer) {
        char filename[256] = {0};
        if(pIMetadata_Hal) {
            tryGetMetadata<MINT32>(pIMetadata_Hal, MTK_PIPELINE_UNIQUE_KEY, hint.UniqueKey);
            tryGetMetadata<MINT32>(pIMetadata_Hal, MTK_PIPELINE_FRAME_NUMBER, hint.FrameNo);
            tryGetMetadata<MINT32>(pIMetadata_Hal, MTK_PIPELINE_REQUEST_NUMBER, hint.RequestNo);
        }
        MBOOL res = MTRUE;
        res = extract(&hint, pIMetadata_Hal);
        if (!res) {
            MY_LOGW("[DUMP_YUV] extract with metadata fail (%d)", res);
        }
        genFileName_YUV(filename, sizeof(filename), &hint, YUV_PORT_UNDEFINED, "Abf_in");
        MY_LOGD("[Abf][in] filename = %s", filename);
        pIBuffer_MainFull->saveToFile(filename);
    }
    if (mpAbf == nullptr) {
        mpAbf = new AbfAdapter(mOpenId);
    }

    if(mpAbf) {
        MINT32 iso      = -1;
        if(pIMetadata_P1) {
            tryGetMetadata<MINT32>(pIMetadata_P1, MTK_SENSOR_SENSITIVITY, iso);
        }
        MY_LOGD("iso:%d", iso);

        AbfAdapter::ProcessParam AbfParam;
        AbfParam.iso    = iso;
        // TODO: FIXME

        //CAM_ULOGM_TAG_BEGIN("AbfPluginProviderImp:Abf processing");
        MY_LOGD("ABF processing +");
        if (!mpAbf->process(AbfParam, pIBuffer_MainFull.get())) {
            MY_LOGE("ABF failed");
            err = BAD_VALUE;
        }
        else {
            if (pOMetadata_Hal != nullptr) {
                //mpSwnr->getDebugInfo(*pOMetadata_Hal);
            } else
                MY_LOGW("no hal metadata for dumping debug info");
        }
        MY_LOGD("Abf processing -");
        if(muDumpBuffer) {
            char filename[256] = {0};
            genFileName_YUV(filename, sizeof(filename), &hint, YUV_PORT_UNDEFINED, "Abf_out");
            MY_LOGD("[Abf][out] filename = %s", filename);
            pIBuffer_MainFull->saveToFile(filename);
        }

        //CAM_ULOGM_TAG_END();
    }
    return err;
}
