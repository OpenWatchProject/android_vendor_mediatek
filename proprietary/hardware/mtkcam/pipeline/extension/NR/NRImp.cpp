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

#define LOG_TAG "NRvendorImp"

#include "NRImp.h"
#include "../MyUtils.h"
#include "../utils/VendorUtils.h"
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

/*MTKCAM*/
#include <mtkcam/utils/std/Trace.h>

// 3rdPartyFaceBeauty+++
/*MTK3rdParty*/
#include <mtkcam/pipeline/extension/ThirdPartyFeatures.h>
// 3rdPartyFaceBeauty---

/* MNR */
#include <mtkcam/aaa/IHal3A.h>
#include <isp_tuning/isp_tuning.h>     // EIspProfile_*
using namespace NSIspTuning;

/*abf*/
#if SUPPORT_ABF
#include <mtkcam/feature/abf/AbfAdapter.h>
#endif

/*crop*/
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/feature/eis/EisInfo.h>
#include <camera_custom_eis.h>
using namespace NSCam::EIS;

/*aosp*/
#include <cutils/compiler.h>

/*std*/
#include <functional>

/*POSIX*/
#include <sys/resource.h> //::setpriority

using namespace NSCamHW;

using namespace NS3Av3;

#include <sys/prctl.h>

#include <mtkcam/drv/IHalSensor.h>
#include <camera_custom_capture_nr.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <camera_custom_nvram.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif
// clear zoom
#include "DpDataType.h"
#include <mtkcam/utils/exif/DebugExifUtils.h>

using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;

using namespace android;
using namespace NSCam::plugin;

// Thread priority of SWNR, the normal priority is 120 + this value
#define NRVENDOR_SWNR_THREAD_PRIORITY           0

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
#define FUNCTION_IN             MY_LOGD("+");
#define FUNCTION_OUT            MY_LOGD("-");

#define NR_DEBUG_DUMP_PATH  "/sdcard/camera_dump"

#if (HWNODE_HAVE_AEE_FEATURE)
#include <aee.h>
#ifdef AEE_ASSERT
#undef AEE_ASSERT
#endif
#define AEE_ASSERT(String) \
    do { \
        CAM_LOGE("ASSERT("#String") fail"); \
        aee_system_exception( \
            LOG_TAG, \
            NULL, \
            DB_OPT_DEFAULT, \
            String); \
    } while(0)
#else
#define AEE_ASSERT(String)
#endif

#define CHECK_TRUE(x)                                   \
    do{                                                 \
        if ( !(x) ) MY_LOGE("error bool: %s", #x);    \
    } while(0)

void *
NRvendorImp::
getTuningFromNvram(MUINT32 openId, MINT32 magicNo, MINT32 scen) const
{
#if MTK_CAM_NEW_NVRAM_SUPPORT
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    const void *pNRNvram = nullptr;
    MUINT32 idx;
    // load some setting from nvram
    MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);
    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
    CAM_IDX_QRY_COMB rMapping_Info;
    pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);
    // need replace isp profile to get correct idx
    if (scen & NRvendorImp::SCEN_MFLL)
    {
        rMapping_Info.eIspProfile = EIspProfile_MFNR_Single;//EIspProfile_MFNR_After_Blend;
        MY_LOGD("EIspProfile_MFNR_After_Blend = %d",EIspProfile_MFNR_After_Blend);
    }
    else
    {
        rMapping_Info.eIspProfile = EIspProfile_Capture;
    }
    idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_SWNR_THRES, rMapping_Info, __FUNCTION__);

    if (idx >= EISO_NUM) {
        MY_LOGE("wrong nvram idx %d", idx);
        return NULL;
    }

    MY_LOGD("query nvram(%d) index: %d ispprofile: %d sensor: %d scenario: %d", magicNo, idx, rMapping_Info.eIspProfile, rMapping_Info.eSensorMode, scen);
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
android::sp<NRvendor>
NRvendor::
createInstance(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
)
{
    return new NRvendorImp(pcszName, i4OpenId, vendorMode);
}


/******************************************************************************
 *
 ******************************************************************************/
NRvendorImp::
~NRvendorImp()
{
#if SUPPORT_ABF
    if(mpAbf) {
        MY_LOGD("delete ABF");
        delete mpAbf;
        mpAbf = NULL;
    }
#endif
    if(mpSwnr) {
        MY_LOGD("delete SWNR");
        delete mpSwnr;
        mpSwnr = NULL;
    }
    {
      Mutex::Autolock _l(mFutureLock);
      mbRequestExit = MTRUE;
      mFutureCond.signal();
    }
}
/******************************************************************************
 *
 ******************************************************************************/
NRvendorImp::
NRvendorImp(
    char const*  pcszName,
    MINT32 const i4OpenId,
    MINT64 const vendorMode
)
    : BaseVendor(i4OpenId, vendorMode, String8::format("NR vendor"))
    , mOpenId(i4OpenId)
    , mMode(vendorMode)
    , mbEnableLog(MTRUE)
    , muEnqueCnt(0)
    , muDequeCnt(0)
    , mnStreamTag(ENormalStreamTag_Normal)
    , mpSwnr(NULL)
#if SUPPORT_ABF
    , mpAbf(NULL)
#endif
    , mbRequestExit(MFALSE)
    , mbIsMfll(MFALSE)
{
    muNRDebug    = ::property_get_int32("debug.camera.nr.debug", 0);
    muDumpBuffer = ::property_get_int32("debug.camera.dump.nr", 0);
    mForceNRType = ::property_get_int32("vendor.debug.shot.forcenr",-1);
    mForceCloseAbf    = ::property_get_int32("debug.abf.close",0);
    // query active array size
    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    if (tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArray)) {
        MY_LOGD("active array(%d, %d, %dx%d)",
                mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h);
    } else {
        MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
    }

    MY_LOGD("[%d]create vendor %" PRId64 " from %s", i4OpenId, vendorMode, pcszName);

    // create 3rd FaceBeauty pool
    // 3rdPartyFaceBeauty+++
    m3rdFbWorkPool = new WorkPool();
    // 3rdPartyFaceBeauty---
}

MBOOL NRvendorImp::createPipe(myparam_t* param)
{
    MBOOL ret = MTRUE;
    CAM_TRACE_BEGIN("NRvendor:NormalStream:create");
    param->pPipe = INormalStream::createInstance(mOpenId);
    if (param->pPipe == NULL)
    {
        MY_LOGE("create normal stream failed");
        ret = MFALSE;
        goto lbExit;
    }
    CAM_TRACE_END();
    CAM_TRACE_BEGIN("NRvendor:NormalStream:init");
    if (!param->pPipe->init(LOG_TAG))
    {
        MY_LOGE("initialize normal stream failed");
        ret = MFALSE;
        goto lbExit;
    }
    CAM_TRACE_END();
    CAM_TRACE_BEGIN("NRvendor:3A:create");
    param->p3A = MAKE_Hal3A(
            mOpenId, LOG_TAG);
    if (param->p3A == NULL)
    {
        MY_LOGE("create 3A HAL failed");
        ret = MFALSE;
        goto lbExit;
    }
    CAM_TRACE_END();
    MY_LOGD("pipe created: pipe(%p) 3AHal(%p)", param->pPipe, param->p3A);

lbExit:
    // fall back if cannot create pipe
    if ((param->pPipe == NULL) || (param->p3A == NULL))
    {
        destroyPipe(param);
    }

    return ret;
}
MVOID NRvendorImp::destroyPipe(myparam_t* param)
{
    if (param->pPipe)
    {
        // uninit pipe
        CAM_TRACE_BEGIN("NRvendor:NormalStream:uninit");
        if (!param->pPipe->uninit(LOG_TAG))
        {
            CAM_TRACE_END();
            MY_LOGE("pipe uninit failed");
        }
        CAM_TRACE_END();
        param->pPipe->destroyInstance();
        param->pPipe = NULL;
    }

    if (param->p3A)
    {
        param->p3A->destroyInstance(LOG_TAG);
        param->p3A = NULL;
    }

    MY_LOGD("pipe destroyed");
}

/******************************************************************************
 *
 ******************************************************************************/
char const*
NRvendorImp::
getName()
{
    return "NRvendorImp";
}
/*******************************************************************************
*
********************************************************************************/
static const char* NRType2String(MINT32 NRType)
{
    switch (NRType)
    {
    case MTK_NR_MODE_OFF:
        return "MTK_NR_MODE_OFF";
    case MTK_NR_MODE_MNR:
        return "MTK_NR_MODE_MNR";
    case MTK_NR_MODE_SWNR:
        return "MTK_NR_MODE_SWNR";
    case MTK_NR_MODE_AUTO:
        return "MTK_NR_MODE_AUTO";
    default:
        return "Unsupported SwNR Type";
    }
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NRvendorImp::
get(
    MINT32              /*openId*/,
    InputInfo const&    in,
    FrameInfoSet&       out
)
{
    FUNCTION_IN;

    out.table.vendorMode   = mMode;
    out.table.inCategory   = FORMAT_YUV;
    out.table.outCategory  = FORMAT_YUV;

    IMetadata meta;
    MINT32 nrMode = MTK_NR_MODE_AUTO;
    tryGetMetadata<MINT32>(&in.halCtrl, MTK_NR_MODE, nrMode);
    if ( mForceNRType != -1 ) {
        // no meta tag -> force nr type
        if(mForceNRType==0) {
            nrMode = MTK_NR_MODE_OFF;
            trySetMetadata<MINT32>(meta, MTK_NR_MODE, MTK_NR_MODE_OFF);
        }
        else if(mForceNRType==1) {
            nrMode = MTK_NR_MODE_MNR;
            trySetMetadata<MINT32>(meta, MTK_NR_MODE, MTK_NR_MODE_MNR);
        }
        else if(mForceNRType==2) {
            nrMode = MTK_NR_MODE_SWNR;
            trySetMetadata<MINT32>(meta, MTK_NR_MODE, MTK_NR_MODE_SWNR);
        }
        else { // default
            nrMode = MTK_NR_MODE_AUTO;
            trySetMetadata<MINT32>(meta, MTK_NR_MODE, MTK_NR_MODE_AUTO);
        }
    } else {
        trySetMetadata<MINT32>(meta, MTK_NR_MODE, nrMode);
        mForceNRType = nrMode;
    }

    mbIsMfll = MFALSE;
    for (size_t i = 0; i < in.combination.size(); ++i) {
        MY_LOGD("in.combination[%zu]:0x%" PRIx64 ", MTK_PLUGIN_MODE_MFNR(0x%X)",
                i, in.combination[i], MTK_PLUGIN_MODE_MFNR);
        if (MTK_PLUGIN_MODE_MFNR == in.combination[i]) {
            mbIsMfll = MTRUE;
            break;
        }
    }

    MY_LOGD("NR type: %s, isMfll(%d)", NRType2String(nrMode), mbIsMfll);
    //
    if (nrMode == MTK_NR_MODE_MNR) {
        out.table.bufferType = BUFFER_UNKNOWN;
        out.table.inFormat.push_back(eImgFmt_YUY2);
        out.table.outFormat.push_back(eImgFmt_YUY2);
    }
    else if (nrMode == MTK_NR_MODE_SWNR) {
        out.table.bufferType = BUFFER_UNKNOWN;//BUFFER_IN_PLACE
        out.table.inFormat.push_back(eImgFmt_I420);
        out.table.outFormat.push_back(eImgFmt_I420);
    }
    else // auto or off
        out.table.bufferType   = BUFFER_UNKNOWN;

    IMetadata::IEntry entry = in.halCtrl.entryFor(MTK_NR_MNR_THRESHOLD_ISO);
    if (entry.isEmpty()) {
        MINT32 threshold_mnr = -1;
        MINT32 threshold_swnr = -1;
        if(!queryNrThreshold(threshold_mnr, threshold_swnr))
            trySetMetadata<MINT32>(meta, MTK_NR_MODE, MTK_NR_MODE_OFF);
        trySetMetadata<MINT32>(meta, MTK_NR_MNR_THRESHOLD_ISO, threshold_mnr);
        trySetMetadata<MINT32>(meta, MTK_NR_SWNR_THRESHOLD_ISO, threshold_swnr);
    }

    if(out.settings.isEmpty())
    {
        MetaItem metaitem;
        metaitem.setting.halMeta = meta;
        out.settings.push_back(metaitem);
    }
    else
    {
        out.settings.editItemAt(0).setting.halMeta += meta;
    }
    //
    if(in.fullRaw.get())
        mRawSize = in.fullRaw.get()->getImgSize();
    else
        MY_LOGE("no size can be set for pool");

    if(in.jpegYuv.get())
        mJpegYuv = in.jpegYuv;
    else
        MY_LOGE("no jpegYuv");

    if(in.thumbnailYuv.get())
            mThumbnailYuv = in.thumbnailYuv;
        else
            MY_LOGE("no thumbnailYuv");

    // create thread
    if(!mThread.valid())
    {
        auto f = [this]() {
                            if (mpSwnr == NULL){
                                CAM_TRACE_BEGIN("NRvendor:MAKE_SwNR instance");
                                mpSwnr = MAKE_SwNR(mOpenId);
                                CAM_TRACE_END();
                             }
                             while ( this->onDequeRequest() )
                             {
                                this->onProcessFuture();
                                Mutex::Autolock _l(this->mFutureLock);
                                if(mbRequestExit) {
                                    MY_LOGD("request to exit.");
                                    break;
                                }
                             }
                        };
        mThread = std::async(std::launch::async, f);
    }

    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NRvendorImp::
set(
    MINT32              /*openId*/,
    InputSetting const& /*in*/
)
{
    FUNCTION_IN;

    FUNCTION_OUT;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NRvendorImp::
onDequeRequest(

)
{
    FUNCTION_IN;
    Mutex::Autolock _l(mFutureLock);
    while ( mvFutures.empty() && ! mbRequestExit)
    {
        status_t status = mFutureCond.wait(mFutureLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mvFutures.size:%zu",
                status, ::strerror(-status), mvFutures.size() );
        }
    }
    if  ( mbRequestExit ) {
        MY_LOGW_IF(1/*!mvFutures.empty()*/, "[flush] mvFutures.size:%zu", mvFutures.size());
        return MFALSE;
    }
    FUNCTION_OUT;
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NRvendorImp::
onProcessFuture(

)
{
    FUNCTION_IN;
    Mutex::Autolock _l(mFutureLock);

    for (std::vector< std::future<MERROR> >::iterator iter = mvFutures.begin(); iter != mvFutures.end();)
    {
        if((*iter).valid())
        {
            MERROR status = (*iter).get();
            MY_LOGE_IF(status!=OK, "status: %d:%s", status, ::strerror(-status));
        }
        iter = mvFutures.erase(iter);
    }

    FUNCTION_OUT;
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
status_t
NRvendorImp::
queue(
    MINT32  const              openId,
    MUINT32 const              requestNo,
    wp<IVendor::IDataCallback> cb,
    BufferParam                bufParam,
    MetaParam                  metaParam
)
{
    FUNCTION_IN;
    CAM_TRACE_FMT_BEGIN("NRvendor:queue R:%d", requestNo);
    // check
    CHECK_TRUE( !bufParam.vIn.isEmpty() );
    CHECK_TRUE( !bufParam.vOut.isEmpty());
    CHECK_TRUE( metaParam.appCtrl.get() );
    CHECK_TRUE( metaParam.halCtrl.get() );
    CHECK_TRUE( metaParam.vIn.size()    ); //p1 dynamic
    // retrieve metadata
    IMetadata pOutAppMeta;
    IMetadata pOutHalMeta;

    if (metaParam.vIn.size() > 1) { // isp p2 out
        pOutAppMeta = *metaParam.vIn[0]->getMetadata();
        pOutHalMeta = *metaParam.vIn[1]->getMetadata();
    }
    else if (metaParam.halResult.get()) {
        pOutAppMeta = *metaParam.appResult->getMetadata();
        pOutHalMeta = *metaParam.halResult->getMetadata();
    }
    else {
        MY_LOGE("cannot get metadata");
        *metaParam.halCtrl->getMetadata();
    }
    // check 3rdFaceBeauty
    const int _3rdFbEnanle = MTK3rdParty::FaceBeauty::isEnable(
            &pOutAppMeta,
            &pOutHalMeta);

    MUINT8 nr_type  = 0;
    MINT32 iso      = -1;
    //
    MetaItem setting;
    setting.setting.halMeta = *metaParam.halCtrl->getMetadata();
    if(metaParam.vIn[0].get())
        setting.others.push_back(*metaParam.vIn[0]->getMetadata());
    decideNrMode(setting, nr_type, iso);

    {
        Mutex::Autolock _l(mLock);
        myparam_t myparam;
        myparam.requestNo   = requestNo;
        myparam.cb          = cb;
        myparam.metaParam   = metaParam;
        myparam.bufParam    = bufParam;
        myparam.iso         = iso;
        myparam.nr_type     = nr_type;
        myparam.openId      = openId;
        myparam.pPipe       = NULL;
        myparam.p3A         = NULL;
        if(nr_type == NR_TYPE_MNR){
            if(!createPipe(&myparam))
            {
                destroyPipe(&myparam);
                return BAD_INDEX;
            }
        }
        mvRunningQueue.add(requestNo, myparam);
        mAsyncCond.signal();
    }

    switch(nr_type){
        case NR_TYPE_MNR:
        {
            Mutex::Autolock _l(mFutureLock);
            mvFutures.push_back(
                    std::async(std::launch::deferred,
                        [=]{
                            ::prctl(PR_SET_NAME, (unsigned long)"Cam@doMNR", 0, 0, 0);
                            return this->doMnr(requestNo);
                        }
                    )
            );
            mFutureCond.signal();
        }
             break;
        case NR_TYPE_SWNR:
        {
            Mutex::Autolock _l(mFutureLock);
            mvFutures.push_back(
                    std::async(std::launch::deferred,
                        [=]{
                            ::prctl(PR_SET_NAME, (unsigned long)"Cam@doSWNR", 0, 0, 0);
                            return this->doSwnr(requestNo);
                        }
                    )
            );
            mFutureCond.signal();
        }
            break;
        default:
        {
#if SUPPORT_ABF
            AbfAdapter::ProcessParam abfparam;
            abfparam.iso = iso;
            if(!mForceCloseAbf && getAbfAdapter()->needAbf(abfparam)) {
                Mutex::Autolock _l(mFutureLock);
                mvFutures.push_back(
                            std::async(std::launch::deferred,
                                [=]{
                                    ::prctl(PR_SET_NAME, (unsigned long)"Cam@doABF", 0, 0, 0);
                                    return this->doAbf(requestNo);
                                }
                            )
                );
                mFutureCond.signal();
                break;
            }
#endif
            // 3rdPartyFaceBeauty+++
            if(_3rdFbEnanle) {
                Mutex::Autolock _l(mFutureLock);
                mvFutures.push_back(
                        std::async( std::launch::deferred,
                            [=] {
                                ::prctl(PR_SET_NAME, (unsigned long)"Cam@do3rdFB", 0, 0, 0);
                                return this->do3rdFB(requestNo);
                            }
                        )
                );
                mFutureCond.signal();
                break;
            }
            // 3rdPartyFaceBeauty---

            MY_LOGE("by pass and drop. iso:%d nrType:%d", iso, nr_type);
            {
                Mutex::Autolock _l(mLock);
                mvRunningQueue.removeItem(requestNo);
                mAsyncCond.signal();
            }
            return BAD_INDEX;
        }
    }
    CAM_TRACE_FMT_END();
    FUNCTION_OUT;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
NRvendorImp::
doMnr(
    MUINT32 const              requestNo
)
{
    CAM_TRACE_NAME("NRvendor:doMnr");
    FUNCTION_IN;
    MERROR err = OK;
    String8 strEnqueLog;

    if(muNRDebug) {
        dump(mOpenId);
    }
    mLock.lock();
    myparam_t& myparam = mvRunningQueue.editValueFor(requestNo);
    MetaParam metaParam = myparam.metaParam;
    BufferParam bufParam = myparam.bufParam;

    wp<IVendor::IDataCallback> cb = myparam.cb;

    std::vector< sp<IImageBuffer> > vCollectLockedImageBuffer;
    QParams qEnqueParams;
    NSCam::NSIoPipe::FrameParams  frameParams;
    MSize srcSize;
    //frame tag
    frameParams.mStreamTag = mnStreamTag;

    // input
    {
        // get buffer
        sp<IImageBufferHeap> pHeap     = bufParam.vIn[0].get() ? bufParam.vIn[0]->getHeap() : NULL;
        sp<IImageBuffer> pImageBuffer  = NULL;

        err = createImageBufferAndLock(
                        pHeap,
                        pImageBuffer
                    );
        myparam.collectLockedImageBuffer.push_back(pImageBuffer);
        if(muDumpBuffer)
        {
            String8 str = String8::format("mnr-in-%d", requestNo);
            dumpBuffer(pImageBuffer, str.string());
        }
        Input src;
        src.mPortID = PORT_IMGI;
        src.mPortID.group = 0;
        src.mBuffer = pImageBuffer.get();
        frameParams.mvIn.push_back(src);
        strEnqueLog += String8::format("EnQ: Src Port(%d) Fmt(0x%x) Size(%dx%d) => ",
                       src.mPortID.index, src.mBuffer->getImgFormat(),
                       src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h);

        srcSize = src.mBuffer->getImgSize();
    }
    sp<Cropper::CropInfo> pCropInfo = new Cropper::CropInfo;
    if (OK != getCropInfo(metaParam.appCtrl->getMetadata(), metaParam.halCtrl->getMetadata(), false, *pCropInfo)) {
        MY_LOGE("getCropInfo failed");
    }
    //output
    {
#if SUPPORT_ABF
        AbfAdapter::ProcessParam abfparam;
        abfparam.iso = myparam.iso;
        if(!mForceCloseAbf && getAbfAdapter()->needAbf(abfparam))
        {
            // BUFFER_WORKING;
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;
            sp<v3::Utils::ImageStreamInfo> pStreamInfo =
                                            createImageStreamInfo(
                                                "Wb:Image:yuv",
                                                IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                                                eSTREAMTYPE_IMAGE_INOUT,
                                                1, 1,
                                                usage, eImgFmt_I420, mRawSize, 0
                                             );
            prepareOneWithoutPool(eDIRECTION_OUT, pStreamInfo, false);
        }
#endif
        MUINT8 occupied = 0;
#define PORT_WDMAO_USED  (0x1)
#define PORT_WROTO_USED  (0x2)
#define PORT_IMG2O_USED  (0x4)
        for (size_t i = 0; i <bufParam.vOut.size(); i++) {
            sp<IImageBufferHeap> pHeap     = bufParam.vOut[i]->getHeap();
            if( !pHeap.get() ) {
                MY_LOGE("No output buffer! vOut[%zu]", i);
            }
            sp<IImageBuffer> pImageBuffer  = NULL;
            err = createImageBufferAndLock(
                            pHeap,
                            pImageBuffer
                        );
            myparam.collectLockedImageBuffer.push_back(pImageBuffer);
            //
            Output dst;
            StreamId_T id = bufParam.vOut[i]->getStreamInfo().get() ? bufParam.vOut[i]->getStreamInfo()->getStreamId() : 0;
            dst.mPortID = (mJpegYuv.get() ? mJpegYuv->getStreamId()==id : id==eSTREAMID_IMAGE_PIPE_YUV_JPEG) ? PORT_WROTO :
                          (mThumbnailYuv.get() ? mThumbnailYuv->getStreamId()==id : id==eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL) ? PORT_IMG2O :
                          PORT_WDMAO ;

            if(dst.mPortID.index == PORT_WDMAO.index )
                     occupied |= PORT_WDMAO_USED;
            else if( dst.mPortID.index == PORT_WROTO.index )
                     occupied |= PORT_WROTO_USED;
            else if( dst.mPortID.index == PORT_IMG2O.index )
                     occupied |= PORT_IMG2O_USED;
            else
                MY_LOGE("occupied: %u index: %d id: %#" PRIx64 "", occupied, dst.mPortID.index, id);

            dst.mPortID.group = 0;
            bool isCap = mJpegYuv.get() ? id==mJpegYuv->getStreamId() :
                         id==eSTREAMID_IMAGE_PIPE_YUV_JPEG ? true : false;
            dst.mPortID.capbility = isCap ? (NSIoPipe::EPortCapbility)(EPortCapbility_Cap)
                                          : (NSIoPipe::EPortCapbility)(EPortCapbility_None);
            dst.mBuffer = pImageBuffer.get();
            dst.mTransform = bufParam.vOut[i]->getStreamInfo()->getTransform();
            frameParams.mvOut.push_back(dst);

            // for output group crop
            MCrpRsInfo crop;
            crop.mGroupID = (dst.mPortID.index==PORT_WDMAO.index) ? 2 :
                            (dst.mPortID.index==PORT_WROTO.index) ? 3 : 1;
            crop.mResizeDst  = dst.mBuffer->getImgSize();
            if(bufParam.vOut[i]->getStreamInfo()->getStreamId() == IVENDOR_STREAMID_IMAGE_PIPE_WORKING) {
                crop.mCropRect.p_fractional = {0, 0};
                crop.mCropRect.p_integral = {0, 0};
                crop.mCropRect.s = dst.mBuffer->getImgSize();
            } else {
                IImageBuffer *pBuf = dst.mBuffer;
                MINT32 const transform = dst.mTransform;
                MSize dstSize = (transform & eTransform_ROT_90)
                                ? MSize(pBuf->getImgSize().h, pBuf->getImgSize().w)
                                : pBuf->getImgSize();
                Cropper::calcViewAngle(mbEnableLog, *pCropInfo, dstSize, crop.mCropRect);
            }
            frameParams.mvCropRsInfo.push_back(crop);

            strEnqueLog += String8::format("StreamId(%#" PRIx64 ") Dst(%p) Grp(%d) Rot(%d) Crop(%d,%d)(%dx%d) Size(%dx%d)",
                                            id,
                                            dst.mBuffer,
                                            crop.mGroupID, dst.mTransform,
                                            crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                                            crop.mCropRect.s.w, crop.mCropRect.s.h,
                                            crop.mResizeDst.w, crop.mResizeDst.h);
        }
    }
#undef PORT_WDMAO_USED
#undef PORT_WROTO_USED
#undef PORT_IMG2O_USED
    {
        DpPqParam *MDP_WDMA_PQParam = nullptr, *MDP_WROT_PQParam = nullptr;
        Vector<Output>::const_iterator iter = frameParams.mvOut.begin();
        while (iter != frameParams.mvOut.end()) {
            ClearZoomParam*  CZConfig = nullptr;
            MUINT32 const _uPortIndex = iter->mPortID.index;
            EPortCapbility _capbility = iter->mPortID.capbility;
            DpPqParam* currPQParam = NULL;
            if (_uPortIndex == PORT_WDMAO.index && MDP_WDMA_PQParam==nullptr) {
                MDP_WDMA_PQParam = new DpPqParam();
                MDP_WDMA_PQParam->scenario = _capbility==EPortCapbility_Cap ? MEDIA_ISP_CAPTURE : MEDIA_ISP_PREVIEW;
                MDP_WDMA_PQParam->enable = false;
                currPQParam = MDP_WDMA_PQParam;
            }
            else if (_uPortIndex == PORT_WROTO.index && MDP_WROT_PQParam==nullptr) {
                MDP_WROT_PQParam = new DpPqParam();
                MDP_WROT_PQParam->scenario = _capbility==EPortCapbility_Cap ? MEDIA_ISP_CAPTURE : MEDIA_ISP_PREVIEW;
                MDP_WROT_PQParam->enable = false;
                currPQParam = MDP_WROT_PQParam;
            }
            else
            {
                iter++;
                continue;
            }
            // Clear Zoom
            MINT32 supportCZ = ::property_get_int32("camera.mdp.cz.enable", 0);
            if(!supportCZ)
            {
                iter++;
                continue;
            }
            if((_capbility == EPortCapbility_Disp) || (_capbility == EPortCapbility_Cap))
            {
                if (currPQParam->scenario == MEDIA_ISP_PREVIEW)
                    currPQParam->enable = (PQ_COLOR_EN | PQ_ULTRARES_EN);
                else if (currPQParam->scenario == MEDIA_ISP_CAPTURE)
                    currPQParam->enable = (PQ_ULTRARES_EN);
                DpIspParam& ispParam = currPQParam->u.isp;
                if(metaParam.halCtrl.get() && metaParam.halCtrl->getMetadata() &&
                   metaParam.vIn.size() > 1 && metaParam.vIn[1].get() && metaParam.vIn[1]->getMetadata())
                {
                    MINT32 czTimeStamp = 0, czFrameNo = 0, czRequestNo = 0, lv_value = 0;
                    tryGetMetadata<MINT32>(metaParam.halCtrl->getMetadata(), MTK_PIPELINE_UNIQUE_KEY, czTimeStamp);
                    ispParam.timestamp = czTimeStamp;
                    tryGetMetadata<MINT32>(metaParam.vIn[1]->getMetadata(), MTK_PIPELINE_FRAME_NUMBER, czFrameNo);
                    ispParam.frameNo = czFrameNo;
                    tryGetMetadata<MINT32>(metaParam.vIn[1]->getMetadata(), MTK_PIPELINE_REQUEST_NUMBER, czRequestNo);
                    ispParam.requestNo = czRequestNo;
                    tryGetMetadata<MINT32>(metaParam.halCtrl->getMetadata(), MTK_REAL_LV, lv_value);
                    ispParam.LV        = lv_value;

                }
                ispParam.iso = myparam.iso;
                ispParam.lensId = mOpenId;
                //
                ClearZoomParam& CZConfig = ispParam.clearZoomParam;
                CZConfig.captureShot    = CAPTURE_SINGLE;//MDP doesn't refer captureShot when preview
                ispParam.p_mdpSetting =  new MDPSetting();
                // debuginfo
                MDPSetting* mdpSet = ispParam.p_mdpSetting;
                mdpSet->size = MDPSETTING_MAX_SIZE;
                mdpSet->buffer = ::malloc(MDPSETTING_MAX_SIZE);
                if(mdpSet->buffer==NULL) {
                    MY_LOGE("alloc mdp dbg buffer fail");
                }
                else
                    ::memset((unsigned char*)(mdpSet->buffer), 0, MDPSETTING_MAX_SIZE);

                strEnqueLog += String8::format("{Port(%d) CZ lengthId:%d captureShot:%d Timestamp:%d FrameNo:%d RequestNo:%d} ",
                    _uPortIndex,
                    ispParam.lensId,
                    CZConfig.captureShot,
                    ispParam.timestamp,
                    ispParam.frameNo,
                    ispParam.requestNo);
            }
            iter++;
        }
        PQParam* framePQParam = new PQParam();
        framePQParam->WDMAPQParam = static_cast<void*>(MDP_WDMA_PQParam);
        framePQParam->WROTPQParam = static_cast<void*>(MDP_WROT_PQParam);
        ExtraParam _extraParam;
        _extraParam.CmdIdx = EPIPE_MDP_PQPARAM_CMD;
        _extraParam.moduleStruct = static_cast<void*>(framePQParam);
        frameParams.mvExtraParam.push_back(_extraParam);
    }

    mLock.unlock();
    MY_LOGD("%s", strEnqueLog.string());
    strEnqueLog.clear();
    // Tuning
    {
        TuningParam rTuningParam = {NULL, NULL, NULL};
        void *pTuning = NULL;
        unsigned int tuningsize = myparam.pPipe->getRegTableSize();
        pTuning = ::malloc(tuningsize);
        if (pTuning == NULL) {
            MY_LOGE("alloc tuning buffer fail");
            return NO_MEMORY;
        }
        rTuningParam.pRegBuf = pTuning;
        // prepare metadata
        IMetadata appmeta = *metaParam.appCtrl->getMetadata();
        IMetadata halmeta = *metaParam.halCtrl->getMetadata();

        MetaSet_T inMetaSet;
        MetaSet_T outMetaSet;
        inMetaSet.appMeta = appmeta;
        inMetaSet.halMeta = halmeta;
        if(metaParam.vIn.size() > 1 && metaParam.vIn[1].get()) //isp p2 out meta
            outMetaSet.halMeta = *metaParam.vIn[1]->getMetadata();
        else
            outMetaSet.halMeta = *metaParam.halResult->getMetadata();

        trySetMetadata<MUINT8>(inMetaSet.halMeta, MTK_3A_PGN_ENABLE, 1);
        if (myparam.p3A) {
            MINT32 outFrameNo = requestNo, outRequestNo = requestNo;
            tryGetMetadata<MINT32>(&outMetaSet.halMeta, MTK_PIPELINE_FRAME_NUMBER, outFrameNo);
            tryGetMetadata<MINT32>(&outMetaSet.halMeta, MTK_PIPELINE_REQUEST_NUMBER, outRequestNo);
            trySetMetadata<MINT32>(inMetaSet.halMeta, MTK_PIPELINE_FRAME_NUMBER, outFrameNo);
            trySetMetadata<MINT32>(inMetaSet.halMeta, MTK_PIPELINE_REQUEST_NUMBER, outRequestNo);
            trySetMetadata<MUINT8>(inMetaSet.halMeta, MTK_3A_ISP_PROFILE, EIspProfile_Capture_MultiPass_HWNR);
            strEnqueLog += String8::format("NRvendor setIsp %p : %d ", pTuning, tuningsize);
            if (0 > myparam.p3A->setIsp(0, inMetaSet, &rTuningParam, &outMetaSet)){
                MY_LOGW("NRvendor setIsp - skip tuning pushing");
                if (pTuning != NULL) {
                    MY_LOGD_IF(mbEnableLog, "NRvendor setIsp free %p : %d", pTuning, tuningsize);
                    ::free(pTuning);
                }
            } else {
                frameParams.mTuningData = pTuning;
                //
            }
        }

        IMetadata* appresult = metaParam.appResult->getMetadata();
        *appresult += outMetaSet.appMeta;
        IMetadata* halresult = metaParam.halResult->getMetadata();
        *halresult += outMetaSet.halMeta;

    }
    // callback
    qEnqueParams.mpfnCallback = pass2CbFunc;
    qEnqueParams.mpCookie = this;
    // push FrameParams into QParams
    qEnqueParams.mvFrameParams.push_back(frameParams);
    strEnqueLog += String8::format("NRvendor enque count:%d, size[in/out]:%zu/%zu",
                   muEnqueCnt, frameParams.mvIn.size(), frameParams.mvOut.size());
    muEnqueCnt++;
    // queue a request into the pipe
    {
        CAM_TRACE_FMT_BEGIN("NRvendor:Driver:enque %d", muEnqueCnt-1);
        MY_LOGD_IF(muNRDebug, "enque pass2");
        MY_LOGD("%s", strEnqueLog.string());
        if (!myparam.pPipe->enque(qEnqueParams))
        {
            MY_LOGE("NRvendor deque count:%d, enque failed", muDequeCnt);
            muDequeCnt++;
            AEE_ASSERT("\nCRDISPATCH_KEY:NRvendor:ISP pass2 deque fail");
        }
        CAM_TRACE_FMT_END();
    }

    FUNCTION_OUT;
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
NRvendorImp::
getCropInfo(
        IMetadata *const inApp,
        IMetadata *const inHal,
        MBOOL const isResized,
        Cropper::CropInfo &cropInfo) const
{
    if (!tryGetMetadata<MSize>(inHal, MTK_HAL_REQUEST_SENSOR_SIZE, cropInfo.sensor_size)) {
        MY_LOGE("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        return BAD_VALUE;
    }


    MSize const sensor = cropInfo.sensor_size;
    MSize const active = mActiveArray.s;

    cropInfo.isResized = isResized;
    // get current p1 buffer crop status
    if (!(tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, cropInfo.crop_p1_sensor) &&
          tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE, cropInfo.dstsize_resizer) &&
          tryGetMetadata<MRect>(inHal, MTK_P1NODE_DMA_CROP_REGION, cropInfo.crop_dma)))
    {
        MY_LOGW_IF(1, "[FIXME] should sync with p1 for rrz setting");

        cropInfo.crop_p1_sensor = MRect(MPoint(0, 0), sensor);
        cropInfo.dstsize_resizer = sensor;
        cropInfo.crop_dma = MRect(MPoint(0, 0), sensor);
    }

    MY_LOGD_IF(mbEnableLog, "SCALAR_CROP_REGION:(%d,%d)(%dx%d) RESIZER_SIZE:(%dx%d) DMA_CROP_REGION:(%d,%d)(%dx%d)",
               cropInfo.crop_p1_sensor.p.x, cropInfo.crop_p1_sensor.p.y,
               cropInfo.crop_p1_sensor.s.w, cropInfo.crop_p1_sensor.s.h,
               cropInfo.dstsize_resizer.w, cropInfo.dstsize_resizer.h,
               cropInfo.crop_dma.p.x, cropInfo.crop_dma.p.y,
               cropInfo.crop_dma.s.w, cropInfo.crop_dma.s.h);

    MINT32 sensorMode;
    if (!tryGetMetadata<MINT32>(inHal, MTK_P1NODE_SENSOR_MODE, sensorMode)) {
        MY_LOGE("cannot get MTK_P1NODE_SENSOR_MODE");
        return BAD_VALUE;
    }

    HwTransHelper hwTransHelper(mOpenId);
    HwMatrix matToActive;
    if (!hwTransHelper.getMatrixToActive(sensorMode, cropInfo.matSensor2Active) ||
        !hwTransHelper.getMatrixFromActive(sensorMode, cropInfo.matActive2Sensor)) {
        MY_LOGE("get matrix fail");
        return UNKNOWN_ERROR;
    }

    cropInfo.tranSensor2Resized = simpleTransform(
            cropInfo.crop_p1_sensor.p,
            cropInfo.crop_p1_sensor.s,
            cropInfo.dstsize_resizer
    );

    MBOOL const isEisOn = isEISOn(inApp);

    MRect cropRegion; //active array domain
    queryCropRegion(inApp, inHal, isEisOn, cropRegion);
    cropInfo.crop_a = cropRegion;

    // query EIS result
    {
        eis_region eisInfo;
        if (isEisOn && queryEisRegion(inHal, eisInfo)) {
            cropInfo.isEisEabled = MTRUE;
            // calculate mv
            vector_f *pMv_s = &cropInfo.eis_mv_s;
            vector_f *pMv_r = &cropInfo.eis_mv_r;
            MBOOL isResizedDomain = MTRUE;
            MSize const resizer = cropInfo.dstsize_resizer;
            MINT64 eisPackedInfo = 0;
            if (!tryGetMetadata<MINT64>(inHal, MTK_EIS_INFO, eisPackedInfo)) {
                MY_LOGE("cannot get MTK_EIS_INFO, current eisPackedInfo = %" PRIi64 " ", eisPackedInfo);
            }
            MUINT32 eis_mode = EisInfo::getMode(eisPackedInfo);
            MUINT32 eis_factor = EisInfo::getFactor(eisPackedInfo);
            MY_LOGD_IF(mbEnableLog, "EIS is ON, eisMode=0x%x, eisFactor=%d", eis_mode, eis_factor);

#if SUPPORT_EIS_MV
            if (eisInfo.is_from_zzr)
            {
                pMv_r->p.x  = eisInfo.x_mv_int;
                pMv_r->pf.x = eisInfo.x_mv_float;
                pMv_r->p.y  = eisInfo.y_mv_int;
                pMv_r->pf.y = eisInfo.y_mv_float;
                cropInfo.eis_mv_s = inv_transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_r);
            }
            else
            {
                isResizedDomain = MFALSE;
                pMv_s->p.x  = eisInfo.x_mv_int;
                pMv_s->pf.x = eisInfo.x_mv_float;
                pMv_s->p.y  = eisInfo.y_mv_int;
                pMv_s->pf.y = eisInfo.y_mv_float;
                cropInfo.eis_mv_r = transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_s);
            }
#else
            //eis in resized domain
            if (EIS_MODE_IS_EIS_12_ENABLED(eis_mode))
            {
                pMv_r->p.x = eisInfo.x_int - (resizer.w * (eis_factor - 100) / 2 / eis_factor);
                pMv_r->pf.x = eisInfo.x_float;
                pMv_r->p.y = eisInfo.y_int - (resizer.h * (eis_factor - 100) / 2 / eis_factor);
                pMv_r->pf.y = eisInfo.y_float;
                cropInfo.eis_mv_s = inv_transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_r);
            }
            else
            {
                pMv_r->p.x = 0;
                pMv_r->pf.x = 0.0f;
                pMv_r->p.y = 0;
                pMv_r->pf.y = 0.0f;
            }
#endif
            MY_LOGD_IF(mbEnableLog, "mv (%s): (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
                       isResizedDomain ? "r->s" : "s->r",
                       pMv_r->p.x,
                       pMv_r->pf.x,
                       pMv_r->p.y,
                       pMv_r->pf.y,
                       pMv_s->p.x,
                       pMv_s->pf.x,
                       pMv_s->p.y,
                       pMv_s->pf.y
            );
            // cropInfo.eis_mv_a = inv_transform(cropInfo.tranActive2Sensor, cropInfo.eis_mv_s);
            cropInfo.matSensor2Active.transform(cropInfo.eis_mv_s.p, cropInfo.eis_mv_a.p);
            // FIXME: float
            //cropInfo.matSensor2Active.transform(cropInfo.eis_mv_s.pf,cropInfo.eis_mv_a.pf);

            MY_LOGD_IF(mbEnableLog, "mv in active %d/%d, %d/%d",
                       cropInfo.eis_mv_a.p.x,
                       cropInfo.eis_mv_a.pf.x,
                       cropInfo.eis_mv_a.p.y,
                       cropInfo.eis_mv_a.pf.y
            );
        }
        else {
            cropInfo.isEisEabled = MFALSE;
            // no need to set 0
            //memset(&cropInfo.eis_mv_a, 0, sizeof(vector_f));
            //memset(&cropInfo.eis_mv_s, 0, sizeof(vector_f));
            //memset(&cropInfo.eis_mv_r, 0, sizeof(vector_f));
        }
    }

    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NRvendorImp::
queryEisRegion(
        IMetadata *const inHal,
        eis_region &region
) const
{
    IMetadata::IEntry entry = inHal->entryFor(MTK_EIS_REGION);

#if SUPPORT_EIS_MV
    // get EIS's motion vector
    if (entry.count() > 8)
    {
        MINT32 x_mv         = entry.itemAt(6, Type2Type<MINT32>());
        MINT32 y_mv         = entry.itemAt(7, Type2Type<MINT32>());
        region.is_from_zzr  = entry.itemAt(8, Type2Type<MINT32>());
        MBOOL x_mv_negative = x_mv >> 31;
        MBOOL y_mv_negative = y_mv >> 31;
        // convert to positive for getting parts of int and float if negative
        if (x_mv_negative) x_mv = ~x_mv + 1;
        if (y_mv_negative) y_mv = ~y_mv + 1;

        region.x_mv_int   = (x_mv & (~0xFF)) >> 8;
        region.x_mv_float = (x_mv & (0xFF)) << 31;
        if(x_mv_negative){
            region.x_mv_int   = ~region.x_mv_int + 1;
            region.x_mv_float = ~region.x_mv_float + 1;
        }

        region.y_mv_int   = (y_mv& (~0xFF)) >> 8;
        region.y_mv_float = (y_mv& (0xFF)) << 31;
        if(y_mv_negative){
            region.y_mv_int   = ~region.y_mv_int + 1;
            region.y_mv_float = ~region.x_mv_float + 1;
        }

        MY_LOGD_IF(mbEnableLog, "EIS MV:%d, %d, %d",
                        region.s.w,
                        region.s.h,
                        region.is_from_zzr);
     }
#endif

    // get EIS's region
    if (entry.count() > 5) {
        region.x_int = entry.itemAt(0, Type2Type<MINT32>());
        region.x_float = entry.itemAt(1, Type2Type<MINT32>());
        region.y_int = entry.itemAt(2, Type2Type<MINT32>());
        region.y_float = entry.itemAt(3, Type2Type<MINT32>());
        region.s.w = entry.itemAt(4, Type2Type<MINT32>());
        region.s.h = entry.itemAt(5, Type2Type<MINT32>());

        MY_LOGD_IF(mbEnableLog, "EIS Region: %d, %d, %d, %d, %dx%d",
                   region.x_int,
                   region.x_float,
                   region.y_int,
                   region.y_float,
                   region.s.w,
                   region.s.h);
        return MTRUE;
    }

    MY_LOGW("wrong eis region count %d", entry.count());
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NRvendorImp::
isEISOn(IMetadata *const inApp) const {
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if (!tryGetMetadata<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode)) {
        MY_LOGW_IF(mbEnableLog, "no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }
#if FORCE_EIS_ON
    eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
#endif
    return eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
NRvendorImp::
queryCropRegion(
        IMetadata *const inApp,
        IMetadata *const inHal,
        MBOOL const isEisOn,
        MRect &cropRegion) const
{
    if (!tryGetMetadata<MRect>(inApp, MTK_SCALER_CROP_REGION, cropRegion)) {
        cropRegion.p = MPoint(0, 0);
        cropRegion.s = mActiveArray.s;
        MY_LOGW_IF(mbEnableLog, "no MTK_SCALER_CROP_REGION, crop full size %dx%d",
                   cropRegion.s.w, cropRegion.s.h);
    }
    MY_LOGD_IF(mbEnableLog, "control: cropRegion(%d, %d, %dx%d)",
               cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

#if SUPPORT_EIS
    if (isEisOn) {
        MINT64 eisPackedInfo = 0;
        if (!tryGetMetadata<MINT64>(inHal, MTK_EIS_INFO, eisPackedInfo)) {
            MY_LOGE("cannot get MTK_EIS_INFO, current eisPackedInfo = %" PRIi64 " ", eisPackedInfo);
        }
        MUINT32 eis_factor = EisInfo::getFactor(eisPackedInfo);
        cropRegion.p.x += (cropRegion.s.w * (eis_factor - 100) / 2 / eis_factor);
        cropRegion.p.y += (cropRegion.s.h * (eis_factor - 100) / 2 / eis_factor);
        cropRegion.s = cropRegion.s * 100 / eis_factor;
        MY_LOGD_IF(mbEnableLog, "EIS: factor %d, cropRegion(%d, %d, %dx%d)",
                   eis_factor, cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
    }
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
NRvendorImp::
pass2CbFunc(QParams &rParams)
{
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc +++");
    NRvendorImp *self = reinterpret_cast<NRvendorImp *>(rParams.mpCookie);
    self->handleDeque(rParams);
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc ---");
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
NRvendorImp::
handleDeque(QParams &rParams)
{
    FUNCTION_IN;
    CAM_TRACE_FMT_BEGIN("NRvendor:handleDeque %d", muDequeCnt);
    MY_LOGD("NRvendor deque count:%d, result:%d", muDequeCnt, rParams.mDequeSuccess);
    muDequeCnt++;
    if(muNRDebug) {
        dump(mOpenId);
    }
    //
    myparam_t myparam;
    {
        Mutex::Autolock _l(mLock);
        myparam = mvRunningQueue.valueAt(0);
        mvRunningQueue.removeItemsAt(0);
        mAsyncCond.signal();
    }
    //
    sp<IImageBuffer> pOutImageBuffer = NULL;
    bool find = false;
    std::vector< sp<IImageBuffer> >::iterator iter = myparam.collectLockedImageBuffer.begin();
    for (; iter != myparam.collectLockedImageBuffer.end(); iter++)
    {
        if( (*iter)->getImageBufferHeap() == myparam.bufParam.vOut[0]->getHeap().get())
        {
            pOutImageBuffer = (*iter);
            find = true;
            break;
        }
    }
    if(!find)
        MY_LOGE("can't find output buffer?");
    else if(muDumpBuffer)
    {
        String8 str = String8::format("mnr-out-%d", myparam.requestNo);
        dumpBuffer(pOutImageBuffer, str.string());
    }
    // abf
#if SUPPORT_ABF
    if(!mForceCloseAbf) {
        CAM_TRACE_BEGIN("NRvendor:ABF");
        AbfAdapter *abf = getAbfAdapter();
        AbfAdapter::ProcessParam abfparam;
        abfparam.iso = myparam.iso;
        if(abf->needAbf(abfparam)) {
            // check out buffer
            MY_LOGD_IF(!abf->process(abfparam, pOutImageBuffer.get()), "Abf process fail");
        }
        else
            MY_LOGD("no need to process Abf");
        CAM_TRACE_END();
    }
#endif

    // unlock imagebuffer
    {
        std::vector< sp<IImageBuffer> >::iterator iter = myparam.collectLockedImageBuffer.begin();
        for (; iter != myparam.collectLockedImageBuffer.end(); iter++)
        {
            MY_LOGD_IF(muNRDebug,"unlock image buffer: %p", (*iter).get());
            (*iter)->unlockBuf(getName());
            myparam.collectLockedImageBuffer.erase(iter--);
        }
    }
    //mark status
    {
        for(size_t i=0; i< myparam.bufParam.vOut.size(); i++)
        {
            sp<BufferHandle> pBufferHandle = myparam.bufParam.vOut[i];
            if(pBufferHandle.get())
            {
                MINT32 status = pBufferHandle->getStatus();
                status &= ~BUFFER_EMPTY;
                status |= rParams.mDequeSuccess ? BUFFER_WRITE_OK|BUFFER_OUT : BUFFER_WRITE_ERROR|BUFFER_OUT;
                pBufferHandle->updateStatus(status);
            }
        }
        myparam.bufParam.vOut.clear();
        if(myparam.bufParam.vIn.size()){
            myparam.bufParam.vIn.clear();
        }

    }
    // free tuning buffer
    for (size_t i = 0; i < rParams.mvFrameParams.size(); i++)
    {
        NSCam::NSIoPipe::FrameParams& param = rParams.mvFrameParams.editItemAt(i);
        void* pTuning = param.mTuningData;
        if (pTuning)
        {
            free(pTuning);
        }
        for (size_t j = 0; j < param.mvExtraParam.size(); j++)
        {
            MUINT cmdIdx = param.mvExtraParam[j].CmdIdx;
            switch (cmdIdx) {
                case EPIPE_MDP_PQPARAM_CMD:
                {
                    PQParam* extraParam = static_cast<PQParam*>(param.mvExtraParam[j].moduleStruct);
                    if(extraParam == NULL) break;
                    auto clearCZandUpdateMeta = [&](MDPSetting* p)
                                    {
                                        if(p != nullptr)
                                        {
                                            MUINT32* buffer = static_cast<MUINT32*>(p->buffer);
                                            MUINT32 size = p->size;
                                            IMetadata exifMeta;
                                            if(buffer)
                                            {
                                                MY_LOGD("Update mnr debug info: addr %p, size %zu", buffer, size);
                                                MY_LOGD("setDebugExif mdp +");
                                                IMetadata* halResult = myparam.metaParam.halResult.get() ? myparam.metaParam.halResult->getMetadata() : nullptr;
                                                if(halResult)
                                                {
                                                    tryGetMetadata<IMetadata>(halResult, MTK_3A_EXIF_METADATA, exifMeta);
                                                    if (DebugExifUtils::setDebugExif(
                                                        DebugExifUtils::DebugExifType::DEBUG_EXIF_RESERVE3,
                                                        static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_KEY),
                                                        static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_DATA),
                                                        size,
                                                        buffer,
                                                        &exifMeta) == nullptr)
                                                    {
                                                        MY_LOGW("set debug exif to metadata fail");
                                                    }
                                                    else
                                                    {
                                                        trySetMetadata<IMetadata>(*halResult, MTK_3A_EXIF_METADATA, exifMeta);
                                                    }
                                                }
                                                free(buffer);
                                                MY_LOGD("setDebugExif mdp -");
                                            }
                                            delete p;
                                        }
                                    };

                    auto clearMem = [&clearCZandUpdateMeta](DpPqParam* _dpParam)
                                    {
                                        if(_dpParam == nullptr) return;
                                        // delete clearzoom param
                                        clearCZandUpdateMeta(_dpParam->u.isp.p_mdpSetting);
                                        // delete DpPqParam
                                        delete _dpParam;
                                    };
                    clearMem((DpPqParam*)extraParam->WDMAPQParam);
                    clearMem((DpPqParam*)extraParam->WROTPQParam);
                    delete extraParam;
                }
                default:
                {
                }
                break;
            }
        }// for
    }
    destroyPipe(&myparam);
    sp<IVendor::IDataCallback> c = myparam.cb.promote();
    if(c.get())
        c->onDispatch();
    else
        MY_LOGE("call back not exit");
    FUNCTION_OUT;
    CAM_TRACE_FMT_END();
    mAsyncCond.signal();


}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
NRvendorImp::
doSwnr(
    MUINT32 const              requestNo
)
{
    CAM_TRACE_NAME("NRvendor:doSwnr");
    FUNCTION_IN;
    myparam_t myparam;
    {
        Mutex::Autolock _l(mLock);
        if(muNRDebug) {
            dump(mOpenId);
        }
        myparam = mvRunningQueue.valueFor(requestNo);
        mvRunningQueue.removeItem(requestNo);
        mAsyncCond.signal();
    }
    MetaParam& metaParam = myparam.metaParam;
    BufferParam& bufParam = myparam.bufParam;
    MINT32  const iso       = myparam.iso;
    MINT32  const openId    = myparam.openId;
    wp<IVendor::IDataCallback> cb = myparam.cb;

    // retrieve metadata
    IMetadata pOutAppMeta;
    IMetadata pOutHalMeta;

    if (metaParam.vIn.size() > 1) { // isp p2 out
        pOutAppMeta = *metaParam.vIn[0]->getMetadata();
        pOutHalMeta = *metaParam.vIn[1]->getMetadata();
    }
    else if (metaParam.halResult.get()) {
        pOutAppMeta = *metaParam.appResult->getMetadata();
        pOutHalMeta = *metaParam.halResult->getMetadata();
    }
    else {
        MY_LOGE("cannot get metadata");
        *metaParam.halCtrl->getMetadata();
    }



    MERROR err = OK;
    // get buffer
    sp<IImageBufferHeap> pHeap     = bufParam.vIn[0]->getHeap();
    sp<IImageBuffer> pImageBuffer  = NULL;

    // output buffer, may be not exist.
    sp<IImageBufferHeap> pHeapOut;
    sp<IImageBuffer> pImageBufferOut;

    // check 3rdFaceBeauty
    const int _3rdFbEnanle = MTK3rdParty::FaceBeauty::isEnable(
            &pOutAppMeta,
            &pOutHalMeta);

    // if enable 3rdPartyFaceBeauty, try to get output image buffer, if failed,
    // force disable it.
    if(_3rdFbEnanle) { // {{{
        MY_LOGD("3rd FB enabled, check output buffer");
        if (CC_UNLIKELY( bufParam.vOut.size() <= 0 )) {
            MY_LOGE("3rd FB enabled, but output buffer size is 0, disable it");
            *const_cast<int*>(&_3rdFbEnanle) = 0;
        }
        else {
            MY_LOGD("output buf size=%zu, try to get Heap", bufParam.vOut.size());
            pHeapOut = bufParam.vOut[0]->getHeap();
            if (pHeapOut.get()) {
                MY_LOGD("get output heap: %p", pHeapOut.get());
                // lock buffer
                if (createImageBufferAndLock(pHeapOut, pImageBufferOut) != OK) {
                    MY_LOGE("cannot create output IImageBuffer from heap, "\
                            "disable 3rdPartyFaceBeauty");
                    *const_cast<int*>(&_3rdFbEnanle) = 0;
                }
                else {
                    MY_LOGD("OK! output buffer is ok for 3rdPartyFaceBeauty");
                }
            }
            else {
                MY_LOGE("can't get output heap, disable 3rdPartyFaceBeauty");
                *const_cast<int*>(&_3rdFbEnanle) = 0;
            }
        }
    } // }}}

    if(pHeap.get())
        MY_LOGD("get heap: %p", pHeap.get());
    else
        MY_LOGE("can't get heap");
    //
    if( createImageBufferAndLock(pHeap, pImageBuffer) != OK )
    {
        MY_LOGE("can't get image buffer");
    }
    else
        MY_LOGD("img buffer: %p", pImageBuffer.get());
    {
        if (mpSwnr == NULL){
            CAM_TRACE_BEGIN("NRvendor:MAKE_SwNR instance");
            mpSwnr = MAKE_SwNR(openId);
            CAM_TRACE_END();
        }

        if(mpSwnr) {
            ISwNR::SWNRParam swnrParam;
            swnrParam.iso    = iso;
            swnrParam.isMfll = mbIsMfll;
            if(muDumpBuffer)
            {
                String8 str = String8::format("swnr-in-%d", requestNo);
                dumpBuffer(pImageBuffer, str.string());
            }
            CAM_TRACE_BEGIN("NRvendor:SwNR processing");

            int _prior = ::property_get_int32("debug.camera.nr.priority",
                    NRVENDOR_SWNR_THREAD_PRIORITY
                    );
            int _ori   = ::getpriority(PRIO_PROCESS, 0);
            int _err   = ::setpriority(PRIO_PROCESS, 0, _prior);
            if (__builtin_expect( _err == 0, true )) {
                MY_LOGD("set SWNR thread priority to %d", _prior);
            }
            else {
                MY_LOGW("set priority to SWNR thread failed,err=%d", _err);
            }
            MINT32 magicNo = -1;
            if(metaParam.vIn.size()>1) {
                tryGetMetadata<MINT32>(metaParam.vIn[1]->getMetadata(), MTK_P1NODE_PROCESSOR_MAGICNUM, magicNo);
                MY_LOGD("magicNo:%d", magicNo);
            }
            else if(metaParam.halResult.get()) {
               tryGetMetadata<MINT32>(metaParam.halResult->getMetadata(), MTK_P1NODE_PROCESSOR_MAGICNUM, magicNo);
               MY_LOGD("magicNo:%d", magicNo);
            }
            else
               MY_LOGW("cannot get magicNo");

#if MTK_CAM_NEW_NVRAM_SUPPORT
            if (!mpSwnr->doSwNR(swnrParam, pImageBuffer.get(), magicNo, metaParam.halCtrl->getMetadata())) {
                MY_LOGE("SWNR failed");
                err = BAD_VALUE;
            }
#else
            if (!mpSwnr->doSwNR(swnrParam, pImageBuffer.get())) {
                MY_LOGE("SWNR failed");
                err = BAD_VALUE;
            }
#endif
            else {
                IMetadata pOutHalMeta;
                if(metaParam.vIn.size()>1)// isp p2 out
                {
                  pOutHalMeta = *metaParam.vIn[1]->getMetadata();
                }
                else if(metaParam.halResult.get()){// previous vendor result
                  pOutHalMeta = *metaParam.halResult->getMetadata();
                }
                else
                  *metaParam.halCtrl->getMetadata();
                /*Vector<sp<MetaHandle> >::iterator iter = metaParam.vIn.begin();
                for (; iter != metaParam.vIn.end(); iter++) {
                    pOutHalMeta += *(*iter)->getMetadata();
                }*/
                if (!pOutHalMeta.isEmpty())
                    mpSwnr->getDebugInfo(pOutHalMeta);
                else
                    MY_LOGW("no hal metadata for dumping debug info");
                IMetadata* halmeta = metaParam.halResult->getMetadata();
                *halmeta = pOutHalMeta;

                // run 3rdPartyFaceBeauty gogo
                // {{{
                if (_3rdFbEnanle) {
                    MERROR _err = OK;
                    MY_LOGD("process 3rdPartyFaceBeauty after SWNR");
                    {
                        CAM_TRACE_NAME("I420->NV21");
                        MY_LOGD("I420->NV21(MDP) +");
                        _err = convertColorFormatMdp( pImageBuffer.get(), pImageBufferOut.get() );
                        MY_LOGD("I420->NV21(MDP) -");
                    }

                    if (__builtin_expect( _err == OK, true )) {
                        CAM_TRACE_NAME("3rdFb_Run");
                        MY_LOGD("3rdFb_Run +");
                        if(muDumpBuffer) {
                            String8 str = String8::format("3rd-fb-in-%d", requestNo);
                            dumpBuffer(pImageBufferOut, str.string());
                        }
                        MTK3rdParty::FaceBeauty::run(
                                pImageBufferOut.get(),
                                &pOutAppMeta,
                                &pOutHalMeta
                                );
                        if(muDumpBuffer) {
                            String8 str = String8::format("3rd-fb-out-%d", requestNo);
                            dumpBuffer(pImageBufferOut, str.string());
                        }
                        // update metadata
                        *metaParam.halResult->getMetadata() += pOutHalMeta;
                        // no need to invoke syncCache
                        MY_LOGD("3rdFb_Run -");
                    }
                    else {
                        MY_LOGE("I420->NV21 failed, ignore 3rdPartyFaceBeauty");
                    }
                }
                // }}}
            }

            // ::setpriority returns OK, hence restore priority back
            if (__builtin_expect( _err == 0, true )) {
                _err = ::setpriority(PRIO_PROCESS, 0, _ori);
                if (__builtin_expect( _err != 0, false )) {
                    MY_LOGE("weird, setpriority back to %d but failed,err=%d",
                            _prior, _err);
                }
            }

            CAM_TRACE_END();
        }
    }
    //
    if(muDumpBuffer)
    {
        String8 str = String8::format("swnr-out-%d", requestNo);
        dumpBuffer(pImageBuffer, str.string());
    }

    // abf
#if SUPPORT_ABF
    if(!mForceCloseAbf) {
        CAM_TRACE_BEGIN("NRvendor:ABF");
        AbfAdapter *abf = getAbfAdapter();
        AbfAdapter::ProcessParam abfparam;
        abfparam.iso = iso;
        if(abf->needAbf(abfparam)) {
            MY_LOGD_IF(!abf->process(abfparam, pImageBuffer.get()), "Abf process fail");
            if(muDumpBuffer)
            {
                String8 str = String8::format("abf-out-%d", requestNo);
                dumpBuffer(pImageBuffer, str.string());
            }
        }
        else
            MY_LOGD("no need to process Abf");
        CAM_TRACE_END();
    }

#endif
    pImageBuffer->unlockBuf(getName());
    //mark status
    {
        sp<BufferHandle> pBufferHandle = bufParam.vIn[0];
        if(pBufferHandle.get())
        {
            MINT32 status = pBufferHandle->getStatus();
            status &= ~BUFFER_EMPTY;
            status &= ~BUFFER_IN;
            status |= err==OK ? BUFFER_WRITE_OK|BUFFER_OUT : BUFFER_WRITE_ERROR|BUFFER_OUT;
            pBufferHandle->updateStatus(status);
        }
    }

    // check if output buffer exists
    if (pImageBufferOut.get() != nullptr) {
        pImageBufferOut->unlockBuf(getName());
        // status
        sp<BufferHandle> pBufferHandle = bufParam.vOut[0];
        if(pBufferHandle.get())
        {
            MINT32 status = pBufferHandle->getStatus();
            status &= ~BUFFER_EMPTY;
            status &= ~BUFFER_OUT;
            status |= err==OK ? BUFFER_WRITE_OK|BUFFER_OUT : BUFFER_WRITE_ERROR|BUFFER_OUT;
            pBufferHandle->updateStatus(status);
        }
    }

    if(bufParam.vOut.size()) {
        bufParam.vOut.clear();
    }

    if(bufParam.vIn.size()) {
        bufParam.vIn.clear();
    }
    sp<IVendor::IDataCallback> c = cb.promote();
    if(c.get())
        c->onDispatch();
    else
        MY_LOGE("no callback?");
    FUNCTION_OUT;
    return err;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NRvendorImp::
createImageBufferAndLock(
    sp<IImageBufferHeap> const& rpImageBufferHeap,
    sp<IImageBuffer>& rpImageBuffer,
    MUINT usage
)
{
    FUNCTION_IN;
    CAM_TRACE_NAME(__FUNCTION__);
    if (CC_UNLIKELY(rpImageBufferHeap.get() == nullptr)) {
        MY_LOGE("rpImageBufferHeap is NULL");
        FUNCTION_OUT;
        return UNKNOWN_ERROR;
    }

    rpImageBuffer = rpImageBufferHeap->createImageBuffer();
    if (rpImageBuffer.get() == NULL) {
        MY_LOGE("heap(%p) create image buffer failed", rpImageBufferHeap.get());
        return BAD_VALUE;
    }
    // lock buffer
    if (usage == 0) {
        usage = (GRALLOC_USAGE_SW_READ_OFTEN  |
                         GRALLOC_USAGE_SW_WRITE_OFTEN |
                         GRALLOC_USAGE_HW_CAMERA_READ |
                         GRALLOC_USAGE_HW_CAMERA_WRITE);
    }

    if (!(rpImageBuffer->lockBuf(getName(), usage))) {
        MY_LOGE("working buffer[%s]: lock image buffer failed", getName());
        return BAD_VALUE;
    }
    FUNCTION_OUT;
    return OK;
}

#if SUPPORT_ABF
/******************************************************************************
 *
 ******************************************************************************/
AbfAdapter*
NRvendorImp::
getAbfAdapter()
{
    if (mpAbf == NULL)
        mpAbf = new AbfAdapter(mOpenId);
    return mpAbf;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
NRvendorImp::
doAbf(
    MUINT32 const              requestNo
)
{
    CAM_TRACE_NAME("NRvendor:doAbf");
    FUNCTION_IN;
    myparam_t myparam;
    {
        Mutex::Autolock _l(mLock);
        myparam = mvRunningQueue.valueFor(requestNo);
        mvRunningQueue.removeItem(requestNo);
        mAsyncCond.signal();
    }
    MetaParam& metaParam = myparam.metaParam;
    BufferParam& bufParam = myparam.bufParam;
    MINT32  const iso       = myparam.iso;
    MINT32  const openId    = myparam.openId;
    wp<IVendor::IDataCallback> cb = myparam.cb;

    MERROR err = OK;
    // get buffer
    sp<IImageBufferHeap> pHeap     = bufParam.vIn[0]->getHeap();
    sp<IImageBuffer> pImageBuffer  = NULL;

    if(pHeap.get())
        MY_LOGD("get heap: %p", pHeap.get());
    else
        MY_LOGE("can't get heap");

    if( createImageBufferAndLock(pHeap, pImageBuffer) != OK )
    {
        MY_LOGE("can't get image buffer");
    }
    else
    {
        MY_LOGD("img buffer: %p", pImageBuffer.get());

        // abf
        {
            AbfAdapter *abf = getAbfAdapter();
            AbfAdapter::ProcessParam abfparam;
            abfparam.iso = iso;
            if(abf->needAbf(abfparam)) {
                err = abf->process(abfparam, pImageBuffer.get()) ? OK : BAD_VALUE;
                MY_LOGD_IF(err!=OK, "Abf process fail");
            }
            else
                MY_LOGD("no need to process Abf");
        }

        pImageBuffer->unlockBuf(getName());
    }

    //mark status
    {
        sp<BufferHandle> pBufferHandle = bufParam.vIn[0];
        if(pBufferHandle.get())
        {
            MINT32 status = pBufferHandle->getStatus();
            status &= ~BUFFER_EMPTY;
            status &= ~BUFFER_IN;
            status |= err==OK ? BUFFER_WRITE_OK|BUFFER_OUT : BUFFER_WRITE_ERROR|BUFFER_OUT;
            pBufferHandle->updateStatus(status);
        }
    }
    if(bufParam.vOut.size()){
        bufParam.vOut.clear();
    }

    if(bufParam.vIn.size()){
        bufParam.vIn.clear();
    }
    sp<IVendor::IDataCallback> c = cb.promote();
    if(c.get())
        c->onDispatch();
    else
        MY_LOGE("no callback?");
    FUNCTION_OUT;
    return err;
}

#endif

// 3rdPartyFaceBeauty+++
/******************************************************************************
 *
 ******************************************************************************/
MERROR
NRvendorImp::
do3rdFB(
    MUINT32 const              requestNo
)
{
    CAM_TRACE_NAME("NRvendor:3rdFb");
    FUNCTION_IN;
    myparam_t myparam;
    {
        Mutex::Autolock _l(mLock);
        myparam = mvRunningQueue.valueFor(requestNo);
        mvRunningQueue.removeItem(requestNo);
    }

    MetaParam& metaParam = myparam.metaParam;
    BufferParam& bufParam = myparam.bufParam;
    MINT32  const iso       = myparam.iso;
    MINT32  const openId    = myparam.openId;
    wp<IVendor::IDataCallback> cb = myparam.cb;

    // retrieve metadata
    IMetadata* pOutAppMeta = nullptr;
    IMetadata* pOutHalMeta = nullptr;

    if (metaParam.vIn.size() > 1) { // isp p2 out
        pOutAppMeta = metaParam.vIn[0]->getMetadata();
        pOutHalMeta = metaParam.vIn[1]->getMetadata();
    }
    else if (metaParam.halResult.get()) {
        pOutAppMeta = metaParam.appResult->getMetadata();
        pOutHalMeta = metaParam.halResult->getMetadata();
    }
    else {
        MY_LOGE("cannot get metadata");
    }

    // check if it's necesary to process 3rd FaceBeauty.
    if ( ! MTK3rdParty::FaceBeauty::isEnable(
                pOutAppMeta,
                pOutHalMeta
    ))
    {
        MY_LOGD("no need to process 3rdPartyFaceBeauty");
        return OK;
    }

    MERROR err = OK;
    // get buffer
    sp<IImageBufferHeap> pHeap     = bufParam.vIn[0]->getHeap(); //
    sp<IImageBuffer> pImageBuffer  = NULL;

    if(pHeap.get())
        MY_LOGD("get heap: %p", pHeap.get());
    else
        MY_LOGE("can't get heap");

    if( createImageBufferAndLock(pHeap, pImageBuffer,
                GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN) != OK )
    {
        MY_LOGE("can't get image buffer");
    }
    else
    {
        MY_LOGD("img buffer: %p", pImageBuffer.get());
        {
            CAM_TRACE_NAME("3rdFb_Run");
            MY_LOGD("3rdFb_Run +");
            if(muDumpBuffer) {
                String8 str = String8::format("3rd-fb-in-%d", requestNo);
                dumpBuffer(pImageBuffer, str.string());
            }
            err=MTK3rdParty::FaceBeauty::run(
                    pImageBuffer.get(),
                    pOutAppMeta,
                    pOutHalMeta
                    );
            if(muDumpBuffer) {
                String8 str = String8::format("3rd-fb-out-%d", requestNo);
                dumpBuffer(pImageBuffer, str.string());
            }
            // no need to invoke syncCache
            MY_LOGD("3rdFb_Run -");
        }

        pImageBuffer->unlockBuf(getName());
    }

    //mark status
    {
        sp<BufferHandle> pBufferHandle = bufParam.vIn[0];
        if(pBufferHandle.get())
        {
            MINT32 status = pBufferHandle->getStatus();
            status &= ~BUFFER_EMPTY;
            status &= ~BUFFER_IN;
            status |= err==OK ? BUFFER_WRITE_OK|BUFFER_OUT : BUFFER_WRITE_ERROR|BUFFER_OUT;
            pBufferHandle->updateStatus(status);
        }
    }
    if(bufParam.vOut.size()){
        bufParam.vOut.clear();
    }
    if(bufParam.vIn.size()){
        bufParam.vIn.clear();
    }
    sp<IVendor::IDataCallback> c = cb.promote();
    if(c.get())
        c->onDispatch();
    else
        MY_LOGE("no callback?");
    FUNCTION_OUT;
    return err;
}
// 3rdPartyFaceBeauty---

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NRvendorImp::
decideNrMode(
    MetaItem const& meta,
    MUINT8& nr_type,
    MINT32& iso
)
{
    iso = -1;
    MINT32 nrMode = -1;

    MINT32 threshold_mnr = -1;
    MINT32 threshold_swnr = -1;

#define getParam(meta, tag, type, param)                \
    do {                                                \
        if( !tryGetMetadata<type>(meta, tag, param) ) { \
            MY_LOGW("no tag: %s", #tag);                \
        }                            \
    } while(0)

#define getHalParam(meta, tag, type, param) getParam(&meta, tag, type, param)
    MINT32 magic = -1;
    getHalParam(meta.setting.halMeta, MTK_P1NODE_PROCESSOR_MAGICNUM , MINT32, magic);
    getHalParam(meta.others[0]      , MTK_SENSOR_SENSITIVITY      , MINT32, iso);
    getHalParam(meta.setting.halMeta, MTK_NR_MODE                 , MINT32, nrMode);
#if MTK_CAM_NEW_NVRAM_SUPPORT
    MINT64 mode = 0;
    MINT32 scenario = 0;
    getHalParam(meta.setting.halMeta, MTK_PLUGIN_MODE, MINT64, mode);
    if(mode & MTK_PLUGIN_MODE_MFNR)
        scenario = SCEN_MFLL;
    else if(mode & MTK_PLUGIN_MODE_HDR)
        scenario = SCEN_HDR;
    else
        scenario = SCEN_DEFAULT;
    NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT* p = (NVRAM_CAMERA_FEATURE_SWNR_THRES_STRUCT*)getTuningFromNvram(mOpenId, magic, scenario);
    if(p!=nullptr){
        threshold_swnr = p->x;
        threshold_mnr = p->x;
    }
    else
        MY_LOGE("NVram thres struct not exit");
#else
    getHalParam(meta.setting.halMeta, MTK_NR_MNR_THRESHOLD_ISO    , MINT32, threshold_mnr);
    getHalParam(meta.setting.halMeta, MTK_NR_SWNR_THRESHOLD_ISO   , MINT32, threshold_swnr);
#endif
 // 0:none, 1:MNR, 2:SWNR
    switch(nrMode)
    {
        case MTK_NR_MODE_MNR:
            nr_type = NR_TYPE_MNR;
            break;
        case MTK_NR_MODE_SWNR:
            nr_type = NR_TYPE_SWNR;
            break;
        case MTK_NR_MODE_OFF:
            nr_type = NR_TYPE_NONE;
            break;
        case MTK_NR_MODE_AUTO:
            if (iso >= threshold_swnr) {
                nr_type = NR_TYPE_SWNR;
            }
            else if (iso >= threshold_mnr) {
                nr_type = NR_TYPE_MNR;
            }
            else {
                nr_type = NR_TYPE_NONE;
            }
            break;

        case -1:
            nr_type = NR_TYPE_IGNORE;
            break;

        default:
            MY_LOGE("no this mode:%d", nrMode);
    }
    MY_LOGD("nrMode: 0x%x, nrType: %d, iso: %d, threshold(H/S)(%d/%d) magic: %d ",
            nrMode, nr_type, iso, threshold_mnr, threshold_swnr, magic);
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<v3::Utils::ImageStreamInfo>
NRvendorImp::
createImageStreamInfo(
    char const*         streamName,
    StreamId_T          streamId,
    MUINT32             streamType,
    size_t              maxBufNum,
    size_t              minInitBufNum,
    MUINT               usageForAllocator,
    MINT                imgFormat,
    MSize const&        imgSize,
    MUINT32             transform
)
{
    FUNCTION_IN;
    IImageStreamInfo::BufPlanes_t bufPlanes;
#define addBufPlane(planes, height, stride)                                      \
        do{                                                                      \
            size_t _height = (size_t)(height);                                   \
            size_t _stride = (size_t)(stride);                                   \
            IImageStreamInfo::BufPlane bufPlane= { _height * _stride, _stride }; \
            planes.push_back(bufPlane);                                          \
        }while(0)
    switch( imgFormat ) {
        case eImgFmt_YV12:
        case eImgFmt_I420:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w >> 1);
            break;
        case eImgFmt_NV21:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w);
            addBufPlane(bufPlanes , imgSize.h >> 1 , imgSize.w);
            break;
        case eImgFmt_YUY2:
            addBufPlane(bufPlanes , imgSize.h      , imgSize.w << 1);
            break;
        default:
            MY_LOGE("format not support yet 0x%x \n", imgFormat);
            break;
    }
#undef  addBufPlane

    sp<v3::Utils::ImageStreamInfo>
        pStreamInfo = new v3::Utils::ImageStreamInfo(
                streamName,
                streamId,
                streamType,
                maxBufNum, minInitBufNum,
                usageForAllocator, imgFormat, imgSize, bufPlanes, transform
                );

    if( pStreamInfo == NULL ) {
        MY_LOGE("create ImageStream failed, %s, %#" PRIx64 "\n",
                streamName, streamId);
    }
    FUNCTION_OUT;
    return pStreamInfo;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NRvendorImp::
beginFlush( MINT32 /*openId*/ )
{
    FUNCTION_IN;
    waitForIdle();
    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NRvendorImp::
endFlush( MINT32 /*openId*/ )
{
    FUNCTION_IN;
    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
NRvendorImp::
waitForIdle()
{
    Mutex::Autolock _l(mLock);
    while (mvRunningQueue.size()) {
        MY_LOGD_IF(1, "wait %zu requests done...", mvRunningQueue.size());
        mAsyncCond.wait(mLock);
    }

    return;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NRvendorImp::
sendCommand(
    MINT32 cmd,
    MINT32 /*openId*/,
    MetaItem& meta,
    MINT32& arg1, MINT32& arg2,
    void*  /*arg3*/
)
{
    FUNCTION_IN;

    CHECK_TRUE(!meta.setting.halMeta.isEmpty());
    CHECK_TRUE(meta.others.size());
    switch(cmd)
    {
        case INFO_PROCESS_AND_BUFFER:
        {
            MUINT8 nr_type = NR_TYPE_NONE;
            MINT32 iso;
            decideNrMode(meta, nr_type, iso);
            switch(nr_type) {
                case  NR_TYPE_MNR:
                {
                    clearOneTimeBuffer();
                    //
                    MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;
                    sp<v3::Utils::ImageStreamInfo> pStreamInfo =
                                                    createImageStreamInfo(
                                                        "Wb:Image:yuv",
                                                        IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                                                        eSTREAMTYPE_IMAGE_INOUT,
                                                        1, 1,
                                                        usage, eImgFmt_YUY2, mRawSize, 0
                                                     );
                    prepareOneWithoutPool(eDIRECTION_IN, pStreamInfo, false);
                    //
                    arg1 = true; // need to do or not
                    arg2 = BUFFER_NO_WORKING_OUTPUT;//BUFFER_WORKING BUFFER_NO_WORKING_OUTPUT;

#if SUPPORT_ABF

                    AbfAdapter::ProcessParam abfparam;
                    abfparam.iso = iso;
                    if(!mForceCloseAbf &&getAbfAdapter()->needAbf(abfparam))
                    {
                        arg2 = BUFFER_WORKING;
                    }

#endif

                }
                    break;
                case  NR_TYPE_SWNR:
                {
                    clearOneTimeBuffer();

                    int _3rdFbEnanle = MTK3rdParty::FaceBeauty::isEnable(
                            &meta.setting.appMeta,
                            &meta.setting.halMeta);

                    if (_3rdFbEnanle == 0) {
                        CAM_TRACE_NAME("allocSwNRBuf");
                        arg1 = true;
                        arg2 = BUFFER_IN_PLACE; //1
                        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;
                        sp<v3::Utils::ImageStreamInfo> pStreamInfo =
                            createImageStreamInfo(
                                    "Wb:Image:yuv",
                                    IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                                    eSTREAMTYPE_IMAGE_INOUT,
                                    1, 1,
                                    usage, eImgFmt_I420, mRawSize, 0
                                    );
                        prepareOneWithoutPool(eDIRECTION_IN, pStreamInfo, false);
                    }
                    else {
                        arg1 = true;
                        arg2 = BUFFER_WORKING;
                        MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN
                            | eBUFFER_USAGE_SW_WRITE_OFTEN
                            | eBUFFER_USAGE_HW_CAMERA_READWRITE;
                        sp<v3::Utils::ImageStreamInfo> pStreamInfoSWNR =
                            createImageStreamInfo(
                                    "Wb:Image:yuv",
                                    IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                                    eSTREAMTYPE_IMAGE_INOUT,
                                    1, 1,
                                    usage, eImgFmt_I420, mRawSize, 0
                                    );
                        sp<v3::Utils::ImageStreamInfo> pStreamInfo3rdFb =
                            createImageStreamInfo(
                                    "Wb:Image:nv21",
                                    IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                                    eSTREAMTYPE_IMAGE_INOUT,
                                    1, 1,
                                    usage, eImgFmt_NV21, mRawSize, 0
                                    );
                        prepareOneWithoutPool(eDIRECTION_IN,     pStreamInfoSWNR,  false);
                        prepareOneWithoutPool(eDIRECTION_OUT,    pStreamInfo3rdFb, false);
                    }
                }
                    break;

                case NR_TYPE_IGNORE:
                {
                    arg1 = false;
                    arg2 = BUFFER_UNKNOWN; //3
                    MY_LOGW("Ignore all NR features.");
                }
                break;

                default:
                {
                    clearOneTimeBuffer();
#if SUPPORT_ABF
                   AbfAdapter::ProcessParam abfparam;
                   abfparam.iso = iso;
                   if (!mForceCloseAbf && getAbfAdapter()->needAbf(abfparam))
                   {
                       arg1 = true;
                       arg2 = BUFFER_IN_PLACE;
                       MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_HW_CAMERA_READWRITE ;
                       sp<v3::Utils::ImageStreamInfo> pStreamInfo =
                                                       createImageStreamInfo(
                                                           "Wb:Image:yuv",
                                                           IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                                                           eSTREAMTYPE_IMAGE_INOUT,
                                                           1, 1,
                                                           usage, eImgFmt_I420, mRawSize, 0
                                                        );
                        prepareOneWithoutPool(eDIRECTION_IN, pStreamInfo, false);
                   }
                   else
#endif
                   {

                       // 3rdPartyFaceBeauty+++
                       if (MTK3rdParty::FaceBeauty::isEnable(
                                   &meta.setting.appMeta,
                                   &meta.setting.halMeta
                        ))
                        {
                           MY_LOGD("single 3rd FaceBeauty gogo");

                           // SW read/write andcamera read/writ
                           const MUINT usage = eBUFFER_USAGE_SW_READ_OFTEN
                               | eBUFFER_USAGE_SW_WRITE_OFTEN
                               | eBUFFER_USAGE_HW_CAMERA_READWRITE ;

                           sp<v3::Utils::ImageStreamInfo> pStreamInfo =
                               createImageStreamInfo(
                                       "3rd:FB:NV21",
                                       IVENDOR_STREAMID_IMAGE_PIPE_WORKING,
                                       eSTREAMTYPE_IMAGE_INOUT,
                                       1, 1, // max, min
                                       usage, eImgFmt_NV21, mRawSize,
                                       0 // no need to transform
                                       );

                           //prepareWorkingBuffer(eDIRECTION_IN, pStreamInfo, false);
                           prepareOneWithoutPool(eDIRECTION_IN, pStreamInfo, false);

                           // responses to VendorManager
                           arg1 = true; // need this vendor
                           arg2 = BUFFER_IN_PLACE; //
                       }
                       // 3rdPartyFaceBeauty---
                       else {
                           arg1 = false;
                           arg2 = BUFFER_UNKNOWN; //3
                           MY_LOGW("No need to process nr, shouldn't queue");
                           break;
                       }
                   }
                }
            }
        };
    }
    MY_LOGD("arg1: %d, arg2: %d", arg1,arg2);
    FUNCTION_OUT;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
NRvendorImp::
dump( MINT32 /*openId*/ )
{
    // debug
    if(!mvRunningQueue.size()) {
        MY_LOGD("RunningQ: %zu", mvRunningQueue.size());
        return OK;
    }
    MY_LOGD("RunningQ: %zu Future: %zu", mvRunningQueue.size(), mvFutures.size());
    for (size_t i=0; i<mvRunningQueue.size(); i++) {
        myparam_t param = mvRunningQueue.valueAt(i);
        MY_LOGD("\tRequestNo: %d", mvRunningQueue.keyAt(i));
        param.dump();
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NRvendorImp::
queryNrThreshold(int& hw_threshold, int& swnr_threshold)
{
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    if( pHalSensorList->queryType(getOpenId()) == NSCam::NSSensorType::eYUV ) {
        // yuv sensor not support multi-pass NR
        return MFALSE;
    }
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    //
    hw_threshold = -1;
    swnr_threshold = -1;
    // get threshold from custom folder
    get_capture_nr_th(
            sensorDev, eShotMode_NormalShot, mbIsMfll,
            &hw_threshold, &swnr_threshold);
    MY_LOGD("threshold(H:%d/S:%d)", hw_threshold, swnr_threshold);
    return MTRUE;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
NRvendorImp::
dumpBuffer(sp<IImageBuffer> pImgBuf, const char* str)
{

    if (!NSCam::Utils::makePath(NR_DEBUG_DUMP_PATH, 0660))
        MY_LOGW("makePath[%s] fails", NR_DEBUG_DUMP_PATH);

    // ouput
    char filename[256] = {0};
    sprintf(filename, NR_DEBUG_DUMP_PATH "/nr-%s-%dx%d.yuv",
            str,
            pImgBuf->getImgSize().w, pImgBuf->getImgSize().h);

    MY_LOGD("[yuv][%s] (%dx%d) fmt(0x%x)",
            str,
            pImgBuf->getImgSize().w, pImgBuf->getImgSize().h,
            pImgBuf->getImgFormat());

    pImgBuf->saveToFile(filename);
    return OK;
}

MERROR
NRvendorImp::
convertColorFormatMdp(
        IImageBuffer* pSrc,
        IImageBuffer* pDst)
{
    CAM_TRACE_CALL();
    FUNCTION_IN;

    // create JPEG YUV and thumbnail YUV using MDP (IImageTransform)
    std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> tf(
            IImageTransform::createInstance(), // constructor
            [](IImageTransform *p){ if (p) p->destroyInstance(); } // deleter
            );

    if (__builtin_expect( tf.get() == nullptr, false )) {
        MY_LOGE("create IImageTransform failed");
        FUNCTION_OUT;
        return UNKNOWN_ERROR;
    }

    auto result = tf->execute(
            pSrc,
            pDst,
            nullptr,
            MRect(MPoint(0, 0), pSrc->getImgSize()),
            0, // no rotation
            3000 // time out in seconds
            );
    if (__builtin_expect( result != MTRUE, false )) {
        MY_LOGE("IImageTransform::execute returns failed");
        FUNCTION_OUT;
        return UNKNOWN_ERROR;
    }

    FUNCTION_OUT;
    return OK;
}


void
NRvendorImp::myparam_t::dump()
{
    MY_LOGI("Dump myparam_t: ");
    MY_LOGI("\trequestNo:%d ", requestNo);
    MY_LOGI("\tiso:%d "      , iso);
    MY_LOGI("\tnrType:%d "   , nr_type);
    MY_LOGI("\topenId:%d "   , openId);
    sp<IVendor::IDataCallback> c = cb.promote();
    MY_LOGI("\tcb:%p "       , c.get());
    //
    metaParam.dump();
    bufParam.dump();
    //
    struct Log
    {
        static  void dump(std::vector< android::sp<IImageBuffer> >& list) {
            for (size_t i = 0; i < list.size(); i++) {
                MY_LOGD("\t\t\t [%d] format:0x%x VA/PA:%" PRIxPTR "/%" PRIxPTR ,
                    list[i]->getStrongCount(),
                    list[i]->getImgFormat(),
                    list[i]->getBufVA(0),
                    list[i]->getBufPA(0));
            }
        }
    };

    Log::dump(collectLockedImageBuffer);
}



