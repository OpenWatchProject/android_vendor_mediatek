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

#define LOG_TAG "SWNRPlugin"
#include "SWNRImpl.h"
//
//#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
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

#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <camera_custom_nvram.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif
#include <camera_custom_capture_nr.h>
using namespace NSCam;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace NSCam::TuningUtils; //dump buffer

#define LPCNR_TEN_BITS 1


/******************************************************************************
 *
 ******************************************************************************/
CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_SWNR);
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
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
    bool bEnableLog = !!::property_get_int32("vendor.debug.camera.swnr.log", 0);
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
REGISTER_PLUGIN_PROVIDER(Yuv, SwnrPluginProviderImp);

/******************************************************************************
 *
 ******************************************************************************/
void*
SwnrPluginProviderImp::
getTuningFromNvram(MUINT32 openId, MINT32 magicNo, MINT64 featureCombination, const IMetadata* pInHalMeta) const
{
#if MTK_CAM_NEW_NVRAM_SUPPORT
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    const void *pNRNvram = nullptr;
    MUINT32 idx;
    // load some setting from nvram
    MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    MBOOL bIspHidl = MFALSE;
    IMetadata::Memory mapInfo;
    CAM_IDX_QRY_COMB rMapping_Info;
    CAM_IDX_QRY_COMB *pMapInfo;
    if (tryGetMetadata<IMetadata::Memory>(pInHalMeta, MTK_ISP_ATMS_MAPPING_INFO, mapInfo)) {
        bIspHidl = MTRUE;
    } else {
        pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);
    }
#if 1
    // need to select isp profile to get correct idx
    if (featureCombination & MTK_FEATURE_MFNR)
    {
        rMapping_Info.eIspProfile = EIspProfile_MFNR_Single;//EIspProfile_MFNR_After_Blend;
        MY_LOGD("get threshold from ISP profile(%d)", rMapping_Info.eIspProfile);
    }
    else
    {
        rMapping_Info.eIspProfile = EIspProfile_Capture;
        MY_LOGD("get threshold from ISP profile(%d)", rMapping_Info.eIspProfile);
    }
#else
    // follow customized ISP profile,
    // make sure there is SWNR threshold defined in that ISP profile
    rMapping_Info.eIspProfile = mIspProfile;
    MY_LOGD("current ISP profile = %d", rMapping_Info.eIspProfile);
#endif
    pMapInfo = bIspHidl ? (CAM_IDX_QRY_COMB *)mapInfo.array() : &rMapping_Info;
    idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_SWNR_THRES, *pMapInfo, __func__);

    if (idx >= EISO_NUM) {
        MY_LOGE("wrong nvram idx %d", idx);
        return NULL;
    }

    MY_LOGD("query nvram(%d) index: %d ispprofile: %d sensor: %d featureCombination: %#" PRIx64"", magicNo, idx, pMapInfo->eIspProfile, pMapInfo->eSensorMode, featureCombination);
    auto pNvBufUtil = MAKE_NvBufUtil();
    if (pNvBufUtil == NULL) {
        MY_LOGE("pNvBufUtil==0");
        return NULL;
    }

    auto result = pNvBufUtil->getBufAndRead(
        CAMERA_NVRAM_DATA_FEATURE,
        sensorDev, (void*&)pNvram);
    if (result != 0) {
        MY_LOGE("read buffer chunk fail");
        return NULL;
    }


    pNRNvram = &(pNvram->SWNR_THRES[idx]);

    return (void*)pNRNvram;
#else
    return nullptr;
#endif
}
/******************************************************************************
 *
 ******************************************************************************/
bool
SwnrPluginProviderImp::
queryNrThreshold(MINT32 sensorId, MINT64 const featureCombination, int& hw_threshold, int& swnr_threshold)
{
    FUNCTION_SCOPE;
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if( pHalSensorList == NULL || pHalSensorList->queryType(sensorId) == NSCam::NSSensorType::eYUV ) {
        // yuv sensor not support multi-pass NR
        return false;
    }
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(sensorId);
    //
    hw_threshold = -1;
    swnr_threshold = -1;
    // get threshold from custom folder
    get_capture_nr_th(
            sensorDev, eShotMode_NormalShot, featureCombination & MTK_FEATURE_MFNR,
            &hw_threshold, &swnr_threshold);
    MY_LOGD("threshold(H:%d/S:%d)", hw_threshold, swnr_threshold);

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
void
SwnrPluginProviderImp::
waitForIdle()
{
    FUNCTION_SCOPE;
    onProcessFuture();
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
SwnrPluginProviderImp::
SwnrPluginProviderImp()
{
    FUNCTION_SCOPE;
    muDumpBuffer = ::property_get_int32("vendor.debug.camera.dump.nr", 0);
    mEnable = ::property_get_int32("vendor.debug.camera.nr.enable", -1); // -1
    mbLpcnrEn = ::property_get_int32("vendor.debug.camera.lpcnr.enable", 1) & ILpcnr::supportLpcnr();
    mbTenBitts = !!::property_get_int32("vendor.debug.p2c.10bits.enable", 1);
}

/******************************************************************************
 *
 ******************************************************************************/
SwnrPluginProviderImp::
~SwnrPluginProviderImp()
{
    FUNCTION_SCOPE;
}

/******************************************************************************
 *
 ******************************************************************************/

/******************************************************************************
 *
 ******************************************************************************/
const
SwnrPluginProviderImp::Property&
SwnrPluginProviderImp::
property()
{
    FUNCTION_SCOPE;
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName = "MTK NR";
        prop.mFeatures = MTK_FEATURE_NR;
        prop.mInPlace = MTRUE;
        prop.mFaceData = eFD_None;
        prop.mPosition = 0;
        prop.mPriority = 4;
        inited = true;
    }

    return prop;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
SwnrPluginProviderImp::
negotiate(Selection& sel)
{
    FUNCTION_SCOPE;
    if (0 == mEnable) {
        return -EINVAL;
    }
    bool isLowRamDevice = ::property_get_bool("ro.config.low_ram", false);
    if (isLowRamDevice) {
        MY_LOGD("ro.confDg.low_ram = true, skip SWNR");
        return -EINVAL;
    }
    if (sel.mIMetadataDynamic.getControl() != NULL) {
        IMetadata* pIMetadata_P1 = sel.mIMetadataDynamic.getControl().get();
        IMetadata* pIMetadata_Hal = sel.mIMetadataHal.getControl().get();
        IMetadata* pIMetadata_App = sel.mIMetadataApp.getControl().get();

        MINT32 threshold = 0;
        MINT32 iso = 0;
        MINT32 magic = 0;
        MINT64 featureCombination = 0;
        MUINT8 nr_mode = 0;
        MINT32 threshold_swnr = 0, threshold_mnr = 0;
        MINT32 sensorId = sel.mSensorId;

        if(!tryGetMetadata<MINT64>(pIMetadata_Hal, MTK_FEATURE_CAPTURE, featureCombination)) {
            MY_LOGE("MTK_FEATURE_CAPTURE is not found");
        } else {
            MY_LOGD("FeatureCombination = %#" PRIx64"", featureCombination);
        }
        tryGetMetadata<MINT32>(pIMetadata_P1, MTK_SENSOR_SENSITIVITY, iso);
        tryGetMetadata<MINT32>(pIMetadata_Hal, MTK_P1NODE_PROCESSOR_MAGICNUM, magic);

        tryGetMetadata<MUINT8>(pIMetadata_Hal, MTK_3A_ISP_PROFILE, mIspProfile);
        MY_LOGD("current ISP profile = %d", mIspProfile);

        if(!tryGetMetadata<MUINT8>(pIMetadata_App, MTK_NOISE_REDUCTION_MODE, nr_mode)) {
            MY_LOGD("can Not get MTK_NOISE_REDUCTION_MODE");
        } else {
            MY_LOGD("MTK_NOISE_REDUCTION_MODE = %d", nr_mode);
        }
        if(nr_mode == MTK_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG) {
            return -EINVAL;
        }
        if(tryGetMetadata<MSize>(pIMetadata_Hal, MTK_HAL_REQUEST_SENSOR_SIZE, mRequestSize)) {
            MY_LOGD("request sensor size: (%dx%d)", mRequestSize.w, mRequestSize.h);
        }
        else {
            MY_LOGE("no size info: MTK_HAL_REQUEST_SENSOR_SIZE");
        }

        MBOOL hal_yuv_reprocess_hint = MFALSE;
        if (tryGetMetadata<MBOOL>(pIMetadata_Hal, MTK_HAL_REQUEST_IS_YUVREPROCESS, hal_yuv_reprocess_hint)) {
            MY_LOGD("MTK_HAL_REQUEST_IS_YUVREPROCESS in-hal %d" , hal_yuv_reprocess_hint);
            if(hal_yuv_reprocess_hint) {
                sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(sensorId);
                if(!pMetadataProvider.get()) {
                    MY_LOGW(" !pMetadataProvider.get() ");
                }
                else {
                    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
                    IMetadata::IEntry const& entryScaler = static_meta.entryFor(MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS);
                    if ( entryScaler.isEmpty() ) {
                        MY_LOGW("no static MTK_SCALER_AVAILABLE_STREAM_CONFIGURATIONS");
                    }
                    else {
                        MINT32 max_width = -1, max_height = -1;
                        for (MUINT i = 0; i < entryScaler.count(); i += 4 ) {
                            if ( max_width < entryScaler.itemAt(i + 1, Type2Type<MINT32>()) &&
                                 max_height< entryScaler.itemAt(i + 2, Type2Type<MINT32>()) ) {
                                max_width  = entryScaler.itemAt(i + 1, Type2Type<MINT32>());
                                max_height = entryScaler.itemAt(i + 2, Type2Type<MINT32>());
                            }
                            MUINT32 scalerWidth  = entryScaler.itemAt(i + 1, Type2Type<MINT32>());
                            MUINT32 scalerHeight = entryScaler.itemAt(i + 2, Type2Type<MINT32>());
                        }
                        if(max_width > mRequestSize.w && max_height > mRequestSize.h) {
                            mRequestSize = MSize(max_width, max_height);
                            MY_LOGD("mRequestSize change to: (%dx%d)", mRequestSize.w, mRequestSize.h);
                        }
                    }
                }
            }
        } else {
            MY_LOGD("cannot find MTK_HAL_REQUEST_IS_YUVREPROCESS");
        }


#if MTK_CAM_NEW_NVRAM_SUPPORT
        NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT* p = (NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT*)getTuningFromNvram(sensorId, magic, featureCombination, pIMetadata_Hal);
        if(p!=nullptr){
            threshold_swnr = p->x;
            threshold_mnr = p->x;
            threshold = threshold_mnr;
        }
        else
            MY_LOGE("NVram thres struct not exit");
#else
        queryNrThreshold(sensorId, featureCombination, threshold_mnr, threshold_swnr);
        threshold = threshold_mnr;
#endif
        MY_LOGD("threshold:%d iso:%d, magic:%d", threshold, iso, magic);
        // return -EINVAL if do not meet the trigger condition
        if (iso < threshold && mEnable == -1) {
            return -EINVAL;
        }
    }

    MUINT32 format = eImgFmt_I420;
    if(mbLpcnrEn) {
        //MY_LOGD("It is LPCNR! We use format eImgFmt_NV12");
        if(mbTenBitts && (sel.mMultiCamFeatureMode != MTK_MULTI_CAM_FEATURE_MODE_VSDOF))
            format = eImgFmt_MTK_YUV_P010;
        else
            format = eImgFmt_NV21;
    }
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
SwnrPluginProviderImp::
init()
{
    FUNCTION_SCOPE;
    mbRequestExit = MFALSE;

    if(!mThread.valid()) {
         MY_LOGD("create new thread +");
         mThread = std::async(std::launch::async, [this]() {
            MY_LOGD("run in new thread....");
            while ( this->onDequeRequest()) {
                this->onProcessFuture();
                Mutex::Autolock _l(this->mFutureLock);
                if(mbRequestExit) {
                    MY_LOGD("request to exit.");
                    break;
                }
            }}
        );
        MY_LOGD("create new thread -");
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
SwnrPluginProviderImp::
process(RequestPtr pRequest,
        RequestCallbackPtr pCallback)
{
    FUNCTION_SCOPE;
    MERROR r = OK;
    MINT32 sensorId = pRequest->mSensorId;
    // 1. check params
    if(!pRequest->mIBufferFull.get()) {
        if (pCallback.get() != nullptr) {
            pCallback->onCompleted(pRequest, 0);
            goto lbExit;
        }
    }
    MY_LOGD("debug: new test");
    {
        // TODO: workaround for init time has no OpenId info
        if ( mpSwnr == NULL && !mbLpcnrEn) {
            CAM_TRACE_FMT_BEGIN("NRplugin:MAKE_SwNR instance(%d)", sensorId);
            MY_LOGD("debug: create instance openId %d", sensorId);
            if(sensorId < 0 ) {
                MY_LOGE("need set openId before init() !!");
            }
            else {
                mpSwnr = (ISwNR*)MAKE_SwNR(sensorId);
                if(mpSwnr != NULL) {
                    MY_LOGD("init mpSwnr in process");
                    mpSwnr->init(mRequestSize);
                }
            }

            CAM_TRACE_FMT_END();
        }
    }

    // 2. enque
    {
        int debug_sync = ::property_get_int32("debug.plugin.swnr.sync", 0);

        // sync call
        if((!pCallback.get()) || debug_sync) {
            MY_LOGD("swnr sync call...");
            r = doSwnr(pRequest);
            if(pCallback.get())
                pCallback->onCompleted(pRequest, r);
        }
        else {
            MY_LOGD("swnr async call...");
            Mutex::Autolock _l(mFutureLock);
            mvFutures.insert(
                    std::pair<RequestPtr, std::future<int32_t>>(pRequest,
                    std::async(std::launch::deferred,
                        [=]() -> int32_t {
                            ::prctl(PR_SET_NAME, (unsigned long)"Cam@doSWNR", 0, 0, 0);
                            int32_t err = OK;
                            err = this->doSwnr(pRequest);
                            if(pCallback.get() != nullptr)
                                pCallback->onCompleted(pRequest, err);
                            return err;
                        }
                    ))
            );
            mFutureCond.signal();
        } //else
    }

lbExit:
    return r;
}

/******************************************************************************
 *
 ******************************************************************************/
void
SwnrPluginProviderImp::
abort(vector<RequestPtr>& pRequests)
{
    FUNCTION_SCOPE;
    MY_LOGD("temp skip");
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
}

/******************************************************************************
 *
 ******************************************************************************/
void
SwnrPluginProviderImp::
uninit()
{
    FUNCTION_SCOPE;

    if(mpSwnr) {
        MY_LOGD("debug: delete instance");
        delete mpSwnr;
        mpSwnr = nullptr;
    }
    if(!mvFutures.empty())
    {
        MY_LOGE("remain %zu requests...", mvFutures.size());
        waitForIdle();
    }
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
bool
SwnrPluginProviderImp::
onDequeRequest()
{
    FUNCTION_SCOPE;
    Mutex::Autolock _l(mFutureLock);
    while ( mvFutures.empty() && ! mbRequestExit)
    {
        MY_LOGD("NR onDequeRequest waiting ...");
        status_t status = mFutureCond.wait(mFutureLock);
        MY_LOGD("NR onDequeRequest waiting done");
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
bool
SwnrPluginProviderImp::
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
int32_t
SwnrPluginProviderImp::
doSwnr(
    RequestPtr const              pRequest
)
{
    CAM_ULOGM_TAGLIFE("SwnrPluginProviderImp:doSwnr");
    FUNCTION_SCOPE;
    int32_t err = OK;
    MINT32 sensorId = pRequest->mSensorId;
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
    if(muDumpBuffer && !mbLpcnrEn) {
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
        genFileName_YUV(filename, sizeof(filename), &hint, YUV_PORT_UNDEFINED, "swnr_in");
        MY_LOGD("[swnr][in] filename = %s", filename);
        pIBuffer_MainFull->saveToFile(filename);
    }

    mLpcnr = lpcnr::ILpcnr::createInstance();

    if(mLpcnr.get() && mbLpcnrEn) {
        ILpcnr::ConfigParams cfg;

        cfg.openId     = sensorId;
        cfg.buffSize   = pIBuffer_MainFull->getImgSize();
        cfg.appMeta    = pIMetadata_App;
        cfg.halMeta    = pIMetadata_Hal;
        cfg.appDynamic = pIMetadata_P1;
        cfg.inputBuff  = pIBuffer_MainFull.get();
        cfg.outputBuff = pIBuffer_MainFull.get();
        mLpcnr->init(cfg);

        MY_LOGD("Lpcnr postprocess+");
        mLpcnr->doLpcnr();
        MY_LOGD("Lpcnr postprocess-");
        //

        if (pOMetadata_Hal != nullptr) {
            mLpcnr->makeDebugInfo(pOMetadata_Hal);
        } else {
            MY_LOGW("no hal metadata for dumping debug info");
        }

    } else {
        if (mpSwnr == nullptr) {
            CAM_TRACE_BEGIN("SwnrPluginProviderImp:MAKE_SwNR instance");
            mpSwnr = MAKE_SwNR(sensorId);
            if(mpSwnr != NULL) {
                MY_LOGD("init mpSwnr in doSwnr");
                mpSwnr->init(mRequestSize);
            }
            CAM_TRACE_END();
        }

        if(mpSwnr) {
            MINT32 iso      = -1;
            MINT32 mode     = -1;
            MINT32 magicNo  = -1;
            if(pIMetadata_Hal) {
                tryGetMetadata<MINT32>(pIMetadata_Hal, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNo);
            }
            if(pIMetadata_P1) {
                tryGetMetadata<MINT32>(pIMetadata_P1, MTK_SENSOR_SENSITIVITY, iso);
            }
            MY_LOGD("magicNo:%d iso:%d", magicNo, iso);

            ISwNR::SWNRParam swnrParam;
            swnrParam.iso    = iso;
            // TODO: FIXME
            swnrParam.isMfll = 0;

            CAM_ULOGM_TAG_BEGIN("SwnrPluginProviderImp:SwNR processing");
            MY_LOGD("SWNR processing +");

#if MTK_CAM_NEW_NVRAM_SUPPORT
            if (!mpSwnr->doSwNR(swnrParam, pIBuffer_MainFull.get(), magicNo, pIMetadata_Hal)) {
                MY_LOGE("SWNR failed");
                err = BAD_VALUE;
            }
#else
            if (!mpSwnr->doSwNR(swnrParam, pIBuffer_MainFull.get())) {
                MY_LOGE("SWNR failed");
                err = BAD_VALUE;
            }
#endif
            else {
                if (pOMetadata_Hal != nullptr) {
                    mpSwnr->getDebugInfo(*pOMetadata_Hal);
                } else
                    MY_LOGW("no hal metadata for dumping debug info");
            }
            MY_LOGD("SWNR processing -");
            if(muDumpBuffer) {
                char filename[256] = {0};
                genFileName_YUV(filename, sizeof(filename), &hint, YUV_PORT_UNDEFINED, "swnr_out");
                MY_LOGD("[swnr][out] filename = %s", filename);
                pIBuffer_MainFull->saveToFile(filename);
            }

            CAM_ULOGM_TAG_END();
        }
    }
    return err;
}
