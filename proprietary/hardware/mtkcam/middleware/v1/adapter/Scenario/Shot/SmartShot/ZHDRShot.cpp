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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#define LOG_TAG "MtkCam/ZHDRShot"
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Trace.h>
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/IIspMgr.h>
//
#include <mtkcam/middleware/v1/IShot.h>
//
#include "ImpShot.h"
#include "ZHDRShot.h"
//
#include <mtkcam/utils/hw/CamManager.h>
using namespace NSCam::Utils;
//
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/camshot/_params.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <utils/Vector.h>
//
#include <mtkcam/aaa/IDngInfo.h>
//
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineUtils.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/feature/utils/FeatureProfileHelper.h> //ProfileParam

#include <mtkcam/aaa/IHal3A.h>
#include <isp_tuning/isp_tuning.h> // EIspProfile_*

// IVendor
//#include <mtkcam/pipeline/extension/ZHDR.h>

using namespace android;
using namespace NSShot::NSSmartShot;
using namespace NSCam;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace NS3Av3;
using namespace NSCam::plugin;
using namespace NSIspTuning;

#define CHECK_OBJECT(x)  do{                                        \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return MFALSE;} \
} while(0)

#include <cutils/properties.h>
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getShotName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")

#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)
#include <memory>
static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD("[%s] + ", pText);
    return std::shared_ptr<char>(pText, [](char* p){ CAM_LOGD("[%s] -", p); });
}

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_ZHDRShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    bool const          isAutoHDR
)
{
    sp<IShot>      pShot        = NULL;
    sp<SmartShot>  pImpShot     = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new ZHDRShot(pszShotName, u4ShotMode, i4OpenId, false,isAutoHDR);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new ZHDRShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}

/******************************************************************************
 *
 ******************************************************************************/
extern
sp<IShot>
createInstance_ZsdZHDRShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId,
    bool const          isAutoHDR
)
{
    sp<IShot>       pShot       = NULL;
    sp<ZHDRShot>    pImpShot    = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new ZHDRShot(pszShotName, u4ShotMode, i4OpenId, true,isAutoHDR);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new ZHDRShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *
 ******************************************************************************/
ZHDRShot::
ZHDRShot(
    char const*const pszShotName,
    uint32_t const   u4ShotMode,
    int32_t const    i4OpenId,
    bool const       isZsd,
    bool const       isAutoHDR
)
    : SmartShot(pszShotName, u4ShotMode, i4OpenId, isZsd)
{
    misAutoHDR = isAutoHDR;
}


/******************************************************************************
 *
 ******************************************************************************/
ZHDRShot::
~ZHDRShot()
{
}

/******************************************************************************
 *
 ******************************************************************************/

bool
ZHDRShot::
onCmd_capture()
{
    CAM_TRACE_NAME("ZHDRShot onCmd_capture");
    //
    mu4Scenario = mShotParam.muSensorMode;
    if ( createNewPipeline() ) {
        MINT32 runtimeAllocateCount = 0;
        beginCapture( runtimeAllocateCount );
        //
        mpManager = NSVendorManager::get(getOpenId());
        CHECK_OBJECT(mpManager);
        //
        constructCapturePipeline();
    }
    //
    plugin::InputInfo inputInfo;
    MINT64 mode = MTK_PLUGIN_MODE_NR;
    // set Input
    inputInfo.combination.push_back(MTK_PLUGIN_MODE_NR);

    inputInfo.appCtrl      = mShotParam.mAppSetting;
    inputInfo.halCtrl      = mShotParam.mHalSetting;
    //
    inputInfo.fullRaw      = mpInfo_FullRaw;
    inputInfo.resizedRaw   = mpInfo_ResizedRaw;
    inputInfo.lcsoRaw      = mpInfo_LcsoRaw;
    inputInfo.jpegYuv      = mpInfo_Yuv;

    inputInfo.thumbnailYuv = mpInfo_YuvThumbnail;
    inputInfo.postview     = mpInfo_YuvPostview;
    inputInfo.jpeg         = mpInfo_Jpeg;
    //
    inputInfo.isZsdMode    = mbZsdFlow;

    MY_LOGD("inputInfo, fullRaw:0x%p, resizedRaw:0x%p, lcsoRaw:0x%p, jpegYuv:0x%p, " \
            "thumbnailYuv:0x%p, postview:0x%p, jpeg:0x%p, isZsd(%d)",
            mpInfo_FullRaw.get(), mpInfo_ResizedRaw.get(), mpInfo_LcsoRaw.get(), mpInfo_Yuv.get(),
            mpInfo_YuvThumbnail.get(), mpInfo_YuvPostview.get(), mpInfo_Jpeg.get(), mbZsdFlow);

    plugin::OutputInfo outputInfo;
    mpManager->get(plugin::CALLER_SHOT_SMART, inputInfo, outputInfo);
    MY_LOGD("combined vendor plugin get done, inCategory:%d, outCategory:%d, frameCount:%d",
            outputInfo.inCategory, outputInfo.outCategory, outputInfo.frameCount);

    // prepare ZHDR app and hal metadata
    {
        MY_LOGD("sensorMode(%d), sensorSize(%d,%d)", mu4Scenario, mSensorSize.w, mSensorSize.h);
        // Prepare query Feature Shot ISP Profile
        ProfileParam profileParam
        {
            mSensorSize,
            SENSOR_VHDR_MODE_ZVHDR, /*VHDR mode*/
            mu4Scenario,
            ProfileParam::FLAG_NONE,
            misAutoHDR? ProfileParam::FMASK_AUTO_HDR_ON:
                    ProfileParam::FMASK_NONE
        };

        MUINT8 profile = 0;
        if (FeatureProfileHelper::getShotProf(profile, profileParam))
        {
            // modify hal control metadata for zHDR
//            if (mShotParam.mbFlashOn) {
//                profile = EIspProfile_zHDR_Flash_Capture;
//                MY_LOGD("flash LED on");
//            }
            IMetadata::setEntry<MUINT8>(
                &(mShotParam.mHalSetting) , MTK_3A_ISP_PROFILE , profile);
            MY_LOGD("ISP profile is set(%u)", profile);
        }
        else
        {
            MY_LOGW("ISP profile is not set");
        }
    }
    // User defined
    plugin::VendorInfo vInfo;
    vInfo.vendorMode = mode;
    //
    plugin::InputSetting inputSetting;
    inputSetting.pMsgCb = this;
    for ( int i = 0; i < outputInfo.frameCount; ++i ) {
        plugin::FrameInfo info;
        info.frameNo = mCapReqNo + i;
        info.curAppControl = mShotParam.mAppSetting;
        info.curHalControl = mShotParam.mHalSetting;
        info.curAppControl += outputInfo.settings[i].setting.appMeta;
        info.curHalControl += outputInfo.settings[i].setting.halMeta;
        //single ZHDR config
        IMetadata::setEntry<MSize>(
            &(info.curHalControl), MTK_HAL_REQUEST_SENSOR_SIZE, mSensorSize);
        IMetadata::setEntry<MUINT8>(
            &(info.curHalControl), MTK_3A_AE_CAP_SINGLE_FRAME_HDR, 1);
        IMetadata::setEntry<MUINT8>(
            &(info.curHalControl), MTK_HAL_REQUEST_REQUIRE_EXIF, 1);
        //
        info.vVendorInfo.add(mode, vInfo);
        inputSetting.vFrame.push_back(info);
    }
    // update p2 control
    updateSetting(outputInfo, inputSetting);
    //
    mpManager->set(plugin::CALLER_SHOT_SMART, inputSetting);
    MY_LOGD("combined vendor plugin set done");
    //
    MINT32 shotCount = inputSetting.vFrame.size();
    Vector<MINT32> vRequestNo;
    if ( mbZsdFlow ) {
        // submit to zsd preview pipeline
        Vector< SettingSet > vSettings;
        for ( MINT32 i = 0; i < shotCount; ++i ) {
            SettingSet s;
            s.appSetting = inputSetting.vFrame[i].curAppControl;
            s.halSetting = inputSetting.vFrame[i].curHalControl;
            vSettings.push_back(s);
        }
        applyRawBufferSettings( vSettings, inputSetting.vFrame.size(), vRequestNo );
    }

    // get multiple raw buffer and send to capture pipeline
    for ( MINT32 i = 0; i < shotCount; ++i ) {
        IMetadata halSetting = mShotParam.mHalSetting;
        if ( mbZsdFlow ) {
            // get Selector Data (buffer & metadata)
            status_t status = OK;
            android::sp<IImageBuffer> pBuf = NULL; // full raw buffer
            IMetadata selectorAppMetadata; // app setting for this raw buffer. Ex.3A infomation
            status = getSelectorData(
                        selectorAppMetadata,
                        halSetting,
                        pBuf
                    );
            if( status != OK ) {
                MY_LOGE("GetSelectorData Fail!");
                return MFALSE;
            }
            //
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata = selectorAppMetadata;
            }
        } else {
            halSetting += inputSetting.vFrame[i].curHalControl;
            IMetadata::IEntry entry(MTK_HAL_REQUEST_SENSOR_SIZE);
            entry.push_back(mSensorSize, Type2Type< MSize >());
            halSetting.update(entry.tag(), entry);
            {
                Mutex::Autolock _l(mResultMetadataSetLock);
                IMetadata resultAppMetadata;
                IMetadata resultHalMetadata;
                IMetadata selectorAppMetadata;
                Vector<ISelector::BufferItemSet> bufferSet;
                mResultMetadataSetMap.add(
                        static_cast<MUINT32>(mCapReqNo),
                        ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, selectorAppMetadata, bufferSet, NONE_CB_DONE}
                        );
            }
        }

        //
        // submit setting to capture pipeline
        if (OK != submitCaptureSetting(
                    i == 0,
                    inputSetting.vFrame[i].curAppControl,
                    halSetting) ) {
            MY_LOGE("Submit capture setting fail.");
            return MFALSE;
        }
        //
        mCapReqNo++;
    }
    //select all image buffer
    onMsgReceived(MSG_ON_SHUTTER_CALLBACK, 0, 0, 0);

    // 1. wait pipeline done
    // 2. set selector back to default zsd selector
    // 3. set full raw buffer count back to normal
    endCapture();
    //
    return MTRUE;
}

static
MVOID
PREPARE_STREAM(BufferList& vDstStreams, StreamId id, MBOOL criticalBuffer)
{
    vDstStreams.push_back(
        BufferSet{
            .streamId       = id,
            .criticalBuffer = criticalBuffer,
        }
    );
}

MERROR
ZHDRShot::
submitCaptureSetting(
    MBOOL mainFrame,
    IMetadata appSetting,
    IMetadata halSetting
)
{
    MY_LOGD("current submit requestNo:%d", mCapReqNo);
    ILegacyPipeline::ResultSet resultSet;
    if ( mbZsdFlow ){
        Mutex::Autolock _l(mResultMetadataSetLock);
        resultSet.add(eSTREAMID_META_APP_DYNAMIC_P1, mResultMetadataSetMap.editValueFor(mCapReqNo).selectorAppMetadata);
    }
    auto pPipeline = mpPipeline;
    if( !pPipeline.get() )
    {
        MY_LOGW("get pPipeline fail");
        return UNKNOWN_ERROR;
    }
    //
    {
        MBOOL expResult, isoResult;
        MINT64 expVal;
        MINT32 isoVal;

        isoResult = IMetadata::getEntry<MINT32>(const_cast<IMetadata*>(&appSetting), MTK_SENSOR_SENSITIVITY, isoVal);
        expResult = IMetadata::getEntry<MINT64>(const_cast<IMetadata*>(&appSetting), MTK_SENSOR_EXPOSURE_TIME, expVal); // ms->us

        MY_LOGD("%s:=========================", __FUNCTION__);
        MY_LOGD("%s: App metadata", __FUNCTION__);
        MY_LOGD("%s: Get Iso -> %d, iso->: %d", __FUNCTION__, isoResult, isoVal);
        MY_LOGD("%s: Get Exp -> %d, exp->: %" PRId64 "", __FUNCTION__, expResult, expVal);
        MY_LOGD("%s:=========================", __FUNCTION__);
    }

    {
        BufferList vDstStreams;
        if ( mainFrame ) {
            if( mpInfo_Jpeg.get() ) {
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_JPEG, false);
            }
            if( mpInfo_Yuv.get() ) {
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_JPEG, false);
            }
            if( mpInfo_YuvThumbnail.get() ) {
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL, false);
            }
            if( mpInfo_YuvPostview.get() ) {
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_YUV_00, false);
            }
        }

        if ( !mbZsdFlow ) {
            PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_OPAQUE, false);
            //PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_RESIZER, false);
            if ( mShotParam.mbEnableLtm )
                PREPARE_STREAM(vDstStreams, eSTREAMID_IMAGE_PIPE_RAW_LCSO, false);
        } else {
            Vector<sp<IImageStreamInfo>> rawInputInfos;
            mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
            sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
            if( !pConsumer.get() ) return UNKNOWN_ERROR;
            pConsumer->querySelector()->queryCollectImageStreamInfo( rawInputInfos/*, MTRUE*/);
            for( size_t i = 0; i < rawInputInfos.size() ; i++) {
                PREPARE_STREAM(vDstStreams, rawInputInfos[i]->getStreamId(),  false);
            }
        }
        //
        if( OK != pPipeline->submitRequest(
                    mCapReqNo,
                    appSetting,
                    halSetting,
                    vDstStreams,
                    &resultSet
                    )
          )
        {
            MY_LOGE("submitRequest failed");
            return UNKNOWN_ERROR;
        }
    }


    return OK;
}

status_t
ZHDRShot::
getSelectorData(
    IMetadata& rAppSetting,
    IMetadata& rHalSetting,
    android::sp<IImageBuffer>& pBuffer
)
{
    CAM_TRACE_CALL();
    //
    mpConsumer = IResourceContainer::getInstance(getOpenId())->queryConsumer( eSTREAMID_IMAGE_PIPE_RAW_OPAQUE );
    sp<StreamBufferProvider> pConsumer = mpConsumer.promote();
    if(pConsumer == NULL) {
        MY_LOGE("mpConsumer is NULL!");
        return UNKNOWN_ERROR;
    }
    //
    sp< ISelector > pSelector = pConsumer->querySelector();
    if(pSelector == NULL) {
        MY_LOGE("can't find Selector in Consumer");
        return UNKNOWN_ERROR;
    }
    //
    auto pPipeline = mpPipeline;
    if(pPipeline == NULL) {
        MY_LOGE("pPipeline is nullptr");
        return UNKNOWN_ERROR;
    }
    //
    status_t status = OK;
    MINT32 rRequestNo;
    Vector<ISelector::MetaItemSet> rvResultMeta;
    Vector<ISelector::BufferItemSet> rvBufferSet;
    pSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MTRUE, 0, 0);
    status = pSelector->getResult(rRequestNo, rvResultMeta, rvBufferSet);
    pSelector->sendCommand(ISelector::eCmd_setAllBuffersTransferring, MFALSE, 0, 0);

    // check
    MBOOL foundFullRaw = MFALSE;
    MBOOL foundResizedRaw = MFALSE;
    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        if (rvBufferSet[i].id == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
            foundFullRaw = MTRUE;
        }
        if ( foundFullRaw ) {
            MY_LOGD("found full raw");
            break;
        }
    }

    if ( !foundFullRaw || rvBufferSet.isEmpty() ) {
        MY_LOGE("Selector get input raw buffer failed! bufferSetSize(%zu). foundFullRaw(%d)",
                rvBufferSet.size(), foundFullRaw);
        return UNKNOWN_ERROR;
    }

    if(status != OK) {
        MY_LOGE("Selector getResult Fail!");
        return UNKNOWN_ERROR;
    }

    if(rvResultMeta.size() < 2) {
        MY_LOGE("ZsdSelect getResult rvResultMeta(%zu)", rvResultMeta.size());
        return UNKNOWN_ERROR;
    }

    // get app & hal metadata
    if(    rvResultMeta.editItemAt(0).id == eSTREAMID_META_APP_DYNAMIC_P1
        && rvResultMeta.editItemAt(1).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
    {
        rAppSetting  = rvResultMeta.editItemAt(0).meta;
        rHalSetting += rvResultMeta.editItemAt(1).meta;
    }
    else
    if(    rvResultMeta.editItemAt(1).id == eSTREAMID_META_APP_DYNAMIC_P1
        && rvResultMeta.editItemAt(0).id == eSTREAMID_META_HAL_DYNAMIC_P1 )
    {
        rAppSetting  = rvResultMeta.editItemAt(1).meta;
        rHalSetting += rvResultMeta.editItemAt(0).meta;
    }
    else {
        MY_LOGE("Something wrong for selector metadata.");
        return UNKNOWN_ERROR;
    }

    for ( size_t i = 0; i < rvBufferSet.size(); ++i ) {
        pBuffer = rvBufferSet[i].heap->createImageBuffer();
        //
        if(pBuffer == NULL) {
            MY_LOGE("get buffer is NULL!");
            return UNKNOWN_ERROR;
        }

       //update p1 buffer to pipeline pool
#if MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT
        sp<CallbackBufferPool> pPool = pPipeline->queryBufferPool( rvBufferSet[i].id );
#else
        sp<CallbackBufferPool> pPool = mpCallbackHandler->queryBufferPool( rvBufferSet[i].id );
#endif
        if( pPool == NULL) {
            MY_LOGE("query Pool Fail!");
            return UNKNOWN_ERROR;
        }
        pPool->addBuffer(pBuffer);
    }
    //
    {
        Mutex::Autolock _l(mResultMetadataSetLock);

        IMetadata resultAppMetadata;
        IMetadata resultHalMetadata;
        mResultMetadataSetMap.add(
                static_cast<MUINT32>(mCapReqNo),
                ResultSet_T{static_cast<MUINT32>(mCapReqNo), resultAppMetadata, resultHalMetadata, rAppSetting, rvBufferSet, NONE_CB_DONE }
                );
    }
    return OK;
}

MVOID
ZHDRShot::
onMetaReceived(
        MUINT32         const requestNo,
        StreamId_T      const streamId,
        MBOOL           const errorResult,
        IMetadata       const result
        )
{
    SmartShot::onMetaReceived(requestNo, streamId, errorResult, result);
}


MVOID
ZHDRShot::
onDataReceived(
        MUINT32 const requestNo,
        StreamId_T const streamId,
        MBOOL const errorBuffer,
        android::sp<IImageBuffer>& pBuffer
        )
{
    if (streamId == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
        onMsgReceived(MSG_ON_NEXT_CAPTURE_READY, 0, 0, 0);
    }
    SmartShot::onDataReceived(requestNo, streamId, errorBuffer, pBuffer);
}
