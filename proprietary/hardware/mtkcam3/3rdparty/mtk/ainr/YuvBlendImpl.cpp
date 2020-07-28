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
#define LOG_TAG "AINRYuvPlugin"
//
#include <mtkcam/utils/std/Log.h>
//
#include <stdlib.h>
#include <string.h>
#include <utils/Errors.h>
#include <utils/List.h>
#include <utils/RefBase.h>
#include <utils/String8.h>
#include <sstream>
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
#include <mtkcam3/pipeline/hwnode/NodeId.h>

#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <mtkcam3/3rdparty/plugin/PipelinePlugin.h>
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <mtkcam3/feature/ainr/IAinrCore.h>
// MTKCAM
#include <mtkcam/aaa/IHal3A.h> // setIsp, CaptureParam_T
//
#include <isp_tuning/isp_tuning.h>
//
// Debug exif
#include <mtkcam/utils/exif/DebugExifUtils.h>
//#include <custom/debug_exif/IDebugExif.h>
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/cam/dbg_cam_param.h>
//
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming
#include <sys/stat.h> // mkdir
#include <sys/prctl.h> //prctl set name
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
#include <cutils/properties.h>
//
#include <custom/feature/mfnr/camera_custom_mfll.h> //CUST_MFLL_AUTO_MODE
#include "AinrCtrler.h"
//
using namespace NSCam;
using namespace plugin;
using namespace android;
using namespace std;
using namespace NSCam::NSPipelinePlugin;
using namespace NS3Av3;
using namespace NSCam::TuningUtils;
using namespace NSIoPipe;
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define ainrLogD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define ainrLogW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define ainrLogE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define ainrLogD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define ainrLogW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define ainrLogE_IF(cond, ...)       do { if ( (cond) ) { ainrLogE(__VA_ARGS__); } }while(0)
//
#define ASSERT(cond, msg)           do { if (!(cond)) { printf("Failed: %s\n", msg); return; } }while(0)
//
#define FUNCTION_IN                 ainrLogD("%s +", __FUNCTION__)
#define FUNCTION_OUT                ainrLogD("%s -", __FUNCTION__)
//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define AINR_TRACE_CALL()                      ATRACE_CALL()
#define AINR_TRACE_NAME(name)                  ATRACE_NAME(name)
#define AINR_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define AINR_TRACE_END()                       ATRACE_END()
#else
#define HDR_TRACE_CALL()
#define HDR_TRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)
#define HDR_TRACE_END()
#endif

#define FUNCTION_SCOPE          auto __scope_logger__ = create_scope_logger(__FUNCTION__)

static std::shared_ptr<char> create_scope_logger(const char* functionName)
{
    bool bEnableLog = !!::property_get_int32("vendor.debug.camera.ainr.log", 1);
    char* pText = const_cast<char*>(functionName);
    CAM_LOGD_IF(bEnableLog, "[%s] + ", pText);
    return std::shared_ptr<char>(pText, [bEnableLog](char* p){ CAM_LOGD_IF(bEnableLog, "[%s] -", p); });
}

/******************************************************************************
*
******************************************************************************/
class YuvBlendImpl : public YuvPlugin::IProvider
{
    typedef YuvPlugin::Property Property;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::Request::Ptr RequestPtr;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:

    virtual const Property& property()
    {
        AINR_TRACE_CALL();
        FUNCTION_SCOPE;
        static Property prop;
        static bool inited;

        if (!inited) {
            prop.mName              = "MTK AINR YUV";
            prop.mFeatures          = MTK_FEATURE_AINR_YUV;
            prop.mFaceData          = eFD_None;
            prop.mPosition          = 0;
            prop.mPriority          = 5;
            inited                  = MTRUE;
        }
        return prop;
    };

    //if capture number is 4, "negotiate" would be called 4 times
    virtual MERROR negotiate(Selection& sel)
    {
        AINR_TRACE_CALL();
        FUNCTION_SCOPE;
        /*********************************************Judge whether to do AINR or not*****************************************/

        /************************************************************End*******************************************************/

        enum EImageFormat fmt = ((mSupport10bit == true) &&
                                 (sel.mMultiCamFeatureMode !=
                                 MTK_MULTI_CAM_FEATURE_MODE_VSDOF)) ?
                                 eImgFmt_MTK_YUV_P010 : eImgFmt_NV12;

        sel.mIBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(fmt)
            .addAcceptedSize(eImgSize_Full);
        sel.mIBufferClean
            .setRequired(MTRUE)
            .addAcceptedFormat(fmt)
            .addAcceptedSize(eImgSize_Full);

        sel.mIMetadataDynamic.setRequired(MTRUE);
        sel.mIMetadataApp.setRequired(MTRUE);
        sel.mIMetadataHal.setRequired(MTRUE);

        //Only main frame has output buffer
        // TODO: Need to set outputbuffer as unpack16 format

        sel.mOBufferFull
            .setRequired(MTRUE)
            .addAcceptedFormat(fmt)
            .addAcceptedSize(eImgSize_Full);

        sel.mOMetadataApp.setRequired(MTRUE);
        sel.mOMetadataHal.setRequired(MTRUE);

        ainrLogD("Need to do ainr yuv blending");

        return OK;
    };

    virtual void init()
    {
        FUNCTION_SCOPE;
    };

    virtual MERROR process(RequestPtr pRequest,
                           RequestCallbackPtr pCallback)
    {
        AINR_TRACE_CALL();
        FUNCTION_SCOPE;
        //set thread's name
        ::prctl(PR_SET_NAME, "AINRYuvPlugin", 0, 0, 0);

        /*
        * Be aware of that metadata and buffer should acquire one time
        */
        IImageBuffer* pIImgBuffer     = nullptr;
        IImageBuffer* pCleanImgBuffer = nullptr;
        IImageBuffer* pOImgBuffer     = nullptr;
        // Get out metadata
        IMetadata* pAppMeta = pRequest->mIMetadataApp->acquire();
        IMetadata* pHalMeta = pRequest->mIMetadataHal->acquire();
        IMetadata* pAppMetaDynamic = pRequest->mIMetadataDynamic->acquire();
        IMetadata* pOMetadata_Hal = pRequest->mOMetadataHal->acquire();

        ainrLogD("Ainr yuv blending process");

        // Check metadata
        if(CC_UNLIKELY(pAppMeta == nullptr)
            || CC_UNLIKELY(pHalMeta == nullptr)
            || CC_UNLIKELY(pOMetadata_Hal == nullptr)
            || CC_UNLIKELY(pAppMetaDynamic == nullptr)) {
            ainrLogE("one of metdata is null!!!");
            return BAD_VALUE;
        }

        // Get proceed input buffer
        if (CC_LIKELY(pRequest->mIBufferFull != nullptr)) {
            pIImgBuffer = pRequest->mIBufferFull->acquire();
            if(CC_UNLIKELY(pIImgBuffer == nullptr)) {
                ainrLogE("Input buffer is null idx(%d)!!!");
                return BAD_VALUE;
            }
            if (mDump & AinrDumpWorking){
                // dump input buffer
                MINT32 sensorId = pRequest->mSensorId;
                bufferDump(sensorId, pHalMeta, pIImgBuffer, YUV_PORT_NULL, "inputProceedYuv");
            }
        }

        // Get  clean input buffer
        if (CC_LIKELY(pRequest->mIBufferClean != nullptr)) {
            pCleanImgBuffer = pRequest->mIBufferClean->acquire();
            if(CC_UNLIKELY(pCleanImgBuffer == nullptr)) {
                ainrLogE("Input buffer is null idx(%d)!!!");
                return BAD_VALUE;
            }
            if (mDump & AinrDumpWorking){
                // dump input buffer
                MINT32 sensorId = pRequest->mSensorId;
                bufferDump(sensorId, pHalMeta, pCleanImgBuffer, YUV_PORT_NULL, "inputCleanYuv");
            }
        }

        // Get output buffer
        if (pRequest->mOBufferFull != nullptr) {
            pOImgBuffer = pRequest->mOBufferFull->acquire();
            if(CC_UNLIKELY(pOImgBuffer == nullptr)) {
                ainrLogE("Output buffer is null idx!!!");
                return BAD_VALUE;
            }
        }

        std::shared_ptr<IAinrCore>  ainrCore = IAinrCore::createInstance();
        if(CC_UNLIKELY(ainrCore.get() == nullptr)) {
            ainrLogE("Ainr core is null!!!");
            return BAD_VALUE;
        }

        {
            MINT32 reqNum   = 0;
            MINT32 frameNum = 0;
            MINT32 processUniqueKey = 0;
            IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_REQUEST_NUMBER, reqNum);
            IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_FRAME_NUMBER, frameNum);
            IMetadata::getEntry<MINT32>(pHalMeta, MTK_PIPELINE_UNIQUE_KEY, processUniqueKey);

            ainrLogD("Ainr MTK_PIPELINE_UNIQUE_KEY(%p)", processUniqueKey);

            AinrConfigYUV cfg;
            cfg.sensor_id = pRequest->mSensorId;;
            cfg.requestNum = reqNum;
            cfg.frameNum  = frameNum;
            cfg.uniqueKey = processUniqueKey;

            ainrCore->initYuv(cfg);
        }

        // Do ainr blending
        if (CC_LIKELY(mSupport10bit == true))
            ainrCore->doYuvBlendingTenbits(pIImgBuffer, pCleanImgBuffer, pOImgBuffer, pHalMeta);
        else
            ainrCore->doYuvBlending(pIImgBuffer, pCleanImgBuffer, pOImgBuffer);

        //end memory copy
        if (mDump & AinrDumpWorking){
            // dump input buffer
            MINT32 sensorId = pRequest->mSensorId;
            bufferDump(sensorId, pHalMeta, pOImgBuffer, YUV_PORT_NULL, "output");
        }

        makeDebugInfo(pOMetadata_Hal);

        // Release
        pRequest->mIMetadataApp->release();
        pRequest->mIMetadataHal->release();
        pRequest->mIBufferClean->release();
        pRequest->mIBufferFull->release();
        pRequest->mOBufferFull->release();
        pRequest->mIMetadataDynamic->release();

        /********************************Finish basic flow start to do AINR**********************************************/


        // Callback to upper layer
        int32_t err = OK;
        if(pCallback.get() != nullptr) {
            pCallback->onCompleted(pRequest, err);
        }

        return 0;
    };

    virtual void abort(vector<RequestPtr>& pRequests)
    {
        AINR_TRACE_CALL();
        FUNCTION_SCOPE;
        bool bAbort = false;
    #if 0
        for (auto req : pRequests){
            bAbort = false;
            for (std::vector<RequestPtr>::iterator it = mvRequests.begin() ; it != mvRequests.end(); it++){
                if((*it) == req){
                    mvRequests.erase(it);
                    bAbort = true;
                    break;
                }
            }
            if (!bAbort){
                ainrLogW("Desire abort request is not found");
            }
        }
    #endif
    };

    virtual void uninit()
    {
        AINR_TRACE_CALL();
    };

    YuvBlendImpl()
        : mEnable(0)
    {
        AINR_TRACE_CALL();
        // TODO: Need to set enable back to -1 after AI-NR code done
        mDump         = ::property_get_int32("vendor.debug.camera.ainr.dump", 0);
        mSupport10bit = !!::property_get_int32("vendor.debug.p2c.10bits.enable", 1);
    };

    virtual ~YuvBlendImpl()
    {
        AINR_TRACE_CALL();
    };
private:
    void makeDebugInfo(IMetadata* metadata)
    {
        bool haveExif = false;
        {
            IMetadata::IEntry entry = metadata->entryFor(MTK_HAL_REQUEST_REQUIRE_EXIF);
            if (! entry.isEmpty()  && entry.itemAt(0, Type2Type<MUINT8>()) )
                    haveExif = true;
        }
        //
        if (haveExif)
        {
            IMetadata::Memory memory_dbgInfo;
            memory_dbgInfo.resize(sizeof(DEBUG_MF_INFO_T));
            DEBUG_MF_INFO_T& dbgInfo =
                *reinterpret_cast<DEBUG_MF_INFO_T*>(memory_dbgInfo.editArray());
            ssize_t idx = 0;
#define addPair(debug_info, index, id, value)           \
            do{                                             \
                debug_info.Tag[index].u4FieldID = (0x01000000 | id); \
                debug_info.Tag[index].u4FieldValue = value; \
                index++;                                    \
            } while(0)
            //
            addPair(dbgInfo , idx , MF_TAG_AINR_EN, 1);
            //
#undef addPair
            //
            IMetadata exifMeta;
            // query from hal metadata first
            {
                IMetadata::IEntry entry = metadata->entryFor(MTK_3A_EXIF_METADATA);
                if (! entry.isEmpty() )
                    exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
            }
            // update
            IMetadata::IEntry entry_key(MTK_MF_EXIF_DBGINFO_MF_KEY);
            entry_key.push_back(DEBUG_EXIF_MID_CAM_MF, Type2Type<MINT32>());
            exifMeta.update(entry_key.tag(), entry_key);
            //
            IMetadata::IEntry entry_data(MTK_MF_EXIF_DBGINFO_MF_DATA);
            entry_data.push_back(memory_dbgInfo, Type2Type<IMetadata::Memory>());
            exifMeta.update(entry_data.tag(), entry_data);
            //
            IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
            entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
            metadata->update(entry_exif.tag(), entry_exif);
        }
        else
        {
            ainrLogD("no need to dump exif");
        }
        //
        return;
    }

    void bufferDump(MINT32 sensorId, IMetadata *halMeta, IImageBuffer* buff, YUV_PORT type, const char *pUserString) {
        // dump input buffer
        char                      fileName[512];
        FILE_DUMP_NAMING_HINT     dumpNamingHint;
        //
        MUINT8 ispProfile = NSIspTuning::EIspProfile_Capture;

        if(!halMeta || !buff) {
            ainrLogE("HalMeta or buff is nullptr, dump fail");
            return;
        }

        if (!IMetadata::getEntry<MUINT8>(halMeta, MTK_3A_ISP_PROFILE, ispProfile)) {
            ainrLogW("cannot get ispProfile at ainr capture");
        }

        // Extract hal metadata and fill up file name;
        extract(&dumpNamingHint, halMeta);
        // Extract buffer information and fill up file name;
        extract(&dumpNamingHint, buff);
        // Extract by sensor id
        extract_by_SensorOpenId(&dumpNamingHint, sensorId);
        // IspProfile
        dumpNamingHint.IspProfile = ispProfile; //EIspProfile_Capture;

        genFileName_YUV(fileName, sizeof(fileName), &dumpNamingHint, type, pUserString);
        buff->saveToFile(fileName);
    }
private:

    //
    int                             mEnable;
    int                             mDump;
    MBOOL                           mSupport10bit = true;
    // file dump hint
    FILE_DUMP_NAMING_HINT           mDumpNamingHint;
    // collect request
    std::vector<RequestPtr>         mvRequests;
};
REGISTER_PLUGIN_PROVIDER(Yuv, YuvBlendImpl);
