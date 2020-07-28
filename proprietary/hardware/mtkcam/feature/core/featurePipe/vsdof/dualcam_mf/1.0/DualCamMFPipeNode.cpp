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


// Standard C header file

// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>

// Module header file

// Local header file
#include "DualCamMFPipeNode.h"

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSDCMF;
using namespace NSCam::NSIoPipe::NSSImager;
using namespace NS3Av3;
using namespace NSCam::TuningUtils;
/*******************************************************************************
* Global Define
********************************************************************************/
#define PIPE_MODULE_TAG "DualCamMF"
#define PIPE_CLASS_TAG "BaseNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

#include <featurePipe/core/include/PipeLog.h>

#define VSDOF_LOGD(fmt, arg...)     do { if(this->mbDebugLog) {MY_LOGD("%d: " fmt, __LINE__, ##arg);} } while(0)

#define MAGIC_ID_SCENARIO_CTRL 918
/*******************************************************************************
* External Function
********************************************************************************/



/*******************************************************************************
* Enum Define
********************************************************************************/




/*******************************************************************************
* Structure Define
********************************************************************************/




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
DualCamMFPipeNode::
DualCamMFPipeNode(const char *name, Graph_T *graph)
  : CamThreadNode(name)
{
    miDumpStartIdx =     ::property_get_int32(STRING_DUMP_START, 0);
    miDumpBufSize =      ::property_get_int32(STRING_DUMP_SIZE, 0);
    miTuningDump =       ::property_get_int32(STRING_TUNING_DUMP, 0);
    miCustomerTuning =   ::property_get_int32(STRING_CUSTOMER_TUNING, 0);

    MY_LOGD("miDumpStartIdx:%d, miDumpBufSize:%d, miTuningDump:%d miCustomerTuning:%d",
        miDumpStartIdx, miDumpBufSize, miTuningDump, miCustomerTuning
    );

    #ifdef BIT_TRUE
    MY_LOGD("bit-true mode");
    #endif
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualCamMFPipeDataHandler Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char*
DualCamMFPipeDataHandler::
ID2Name(DataID id)
{
    #define MAKE_NAME_CASE(name) \
        case name: return #name;

    switch(id)
    {
        MAKE_NAME_CASE(ID_INVALID);
        // 1
        MAKE_NAME_CASE(ROOT_ENQUE);
        MAKE_NAME_CASE(NORMAL_POSTVIEW);
        MAKE_NAME_CASE(P2_DONE);
        MAKE_NAME_CASE(SHUTTER);
        MAKE_NAME_CASE(ERROR_OCCUR_NOTIFY);
        // 6
        MAKE_NAME_CASE(FINAL_RESULT);
        MAKE_NAME_CASE(PREPROCESS_TO_VENDOR);
        MAKE_NAME_CASE(QUICK_POSTVIEW);
        MAKE_NAME_CASE(BSS_RESULT_ORIGIN);
        MAKE_NAME_CASE(BSS_RESULT_ORDERED);
        // 11
        MAKE_NAME_CASE(PREPROCESS_YUV_DONE);
    }
    MY_LOGW("unknown DataID:%d", id);

    return "unknown";
    #undef MAKE_NAME_CASE
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualCamMFPipeNode Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
DualCamMFPipeNode::
onInit()
{
    mbDebugLog = StereoSettingProvider::isLogEnabled(PERPERTY_DCMF_NODE_LOG);
    mbProfileLog = StereoSettingProvider::isProfileLogEnabled();

    MY_LOGD("%d,%d", mbDebugLog, mbProfileLog);

    if(StereoSettingProvider::getStereoSensorIndex(mSensorIdx_Main1, mSensorIdx_Main2) != MTRUE){
        MY_LOGE("Can't get sensor id from StereoSettingProvider!");
        return MFALSE;
    }

    if(!prepareModuleSettings()){
        MY_LOGE("prepareModuleSettings Error! Please check the error msg above!");
    }

    if(!NSCam::NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(mDipInfo)){
        MY_LOGE("queryDIPInfo Error! Please check the error msg above!");
    }

    return MTRUE;
}

MBOOL
DualCamMFPipeNode::
handleDump(IImageBuffer* pBuf, DualCamMFBufferID BID, MUINT32 iReqIdx)
{
    MY_LOGD("BID:%d +", BID);

    if(!shouldDumpForTuning(BID)){
        return MFALSE;
    }

    if(pBuf == nullptr){
        MY_LOGE("pBuf is nullptr!");
        return MFALSE;
    }

    char filepath[1024];
    snprintf(filepath, 1024, "%s/%d/%s", STRING_DUMP_PATH, iReqIdx, getName());

    // make path
    MY_LOGD("makePath: %s", filepath);
    makePath(filepath, 0660);

    const char* fileName = getBIDName(BID);
    const char* filePostfix = getBIDPostfix(BID);

    char writepath[1024];
    snprintf(writepath,
        1024, "%s/%s_%dx%d_%d.%s",
        filepath, fileName,
        pBuf->getImgSize().w, pBuf->getImgSize().h, pBuf->getBufStridesInBytes(0),
        filePostfix
    );

    pBuf->saveToFile(writepath);

    MY_LOGD("BID:%d -", BID);
    return MTRUE;
}

MBOOL
DualCamMFPipeNode::
handleDumpWithExtName(IImageBuffer* pBuf, DualCamMFBufferID BID, MUINT32 iReqIdx, string& extName)
{
    MY_LOGD("BID:%d +", BID);

    if(!shouldDumpForTuning(BID)){
        return MFALSE;
    }

    if(pBuf == nullptr){
        MY_LOGE("pBuf is nullptr!");
        return MFALSE;
    }

    char filepath[1024];
    snprintf(filepath, 1024, "%s/%d/%s", STRING_DUMP_PATH, iReqIdx, getName());

    // make path
    MY_LOGD("makePath: %s", filepath);
    makePath(filepath, 0660);

    const char* fileName = getBIDName(BID);
    const char* filePostfix = getBIDPostfix(BID);

    char writepath[1024];
    snprintf(writepath,
        1024, "%s/%s_%s.%s",
        filepath, fileName, extName.c_str(),
        filePostfix
    );

    pBuf->saveToFile(writepath);

    MY_LOGD("BID:%d -", BID);
    return MTRUE;
}

MBOOL
DualCamMFPipeNode::
onDump(DataID id, ImgInfoMapPtr &data, const char* fileName, const char* postfix)
{
    MY_LOGD("dataID:%d +", id);

    sp<PipeRequest> pPipeReq = data->getRequestPtr();
    MUINT iReqIdx = pPipeReq->getRequestNo();

    if(!shouldDumpRequest(pPipeReq) || !data->shouldDump()){
        return MFALSE;
    }

    // IImageBuffer
    {
        IImageBufferSet set = data->getIImageBufferSet();
        for(auto const &e : set) {
            const DualCamMFBufferID& BID = e.first;
            const sp<IImageBuffer>& spBuf = e.second;

            string extName = "";
            if(data->getExtBufName(BID, extName)){
                handleDumpWithExtName(spBuf.get(), BID, iReqIdx, extName);
            }else{
                handleDump(spBuf.get(), BID, iReqIdx);
            }
        }
    }

    // SmartImageBuffer
    {
        SmartImageBufferSet set = data->getSmartBufferSet();
        for(size_t i=0 ; i<set.size() ; ++i){
            const DualCamMFBufferID& BID = set.keyAt(i);
            const SmartImageBuffer spBuf = set.valueAt(i);
            string extName = "";
            if(data->getExtBufName(BID, extName)){
                handleDumpWithExtName(spBuf->mImageBuffer.get(), BID, iReqIdx, extName);
            }else{
                handleDump(spBuf->mImageBuffer.get(), BID, iReqIdx);
            }
        }
    }

    // GraphicBuffer
    {
        GraphicImageBufferSet set = data->getGraphicBufferSet();
        for(size_t i=0 ; i<set.size() ; ++i){
            const DualCamMFBufferID& BID = set.keyAt(i);
            const SmartGraphicBuffer spBuf = set.valueAt(i);
            string extName = "";
            if(data->getExtBufName(BID, extName)){
                handleDumpWithExtName(spBuf->mImageBuffer.get(), BID, iReqIdx, extName);
            }else{
                handleDump(spBuf->mImageBuffer.get(), BID, iReqIdx);
            }
        }
    }

    // MfllBuffer
    {
        MfllImageBufferSet set = data->getMfllBufferSet();
        for(auto const &e : set) {
            const DualCamMFBufferID& BID = e.first;
            const sp<mfll::IMfllImageBuffer>& spBuf = e.second;

            IImageBuffer* pBuf = static_cast<IImageBuffer*>(spBuf->getImageBuffer());
            string extName = "";
            if(data->getExtBufName(BID, extName)){
                handleDumpWithExtName(pBuf, BID, iReqIdx, extName);
            }else{
                handleDump(pBuf, BID, iReqIdx);
            }
        }
    }

    MY_LOGD("dataID:%d -", id);
    return MTRUE;
}

MBOOL
DualCamMFPipeNode::
onDump(DataID id, PipeRequestPtr &data, const char* fileName, const char* postfix)
{
    MY_LOGD("dataID:%d +", id);

    MUINT iReqIdx = data->getRequestNo();

    if(!shouldDumpRequest(data)){
        return MFALSE;
    }

    if(id != PREPROCESS_TO_VENDOR){
        return MFALSE;
    }

    IMetadata* pMeta_in = data->getMetadata(DualCamMFBufferID::BID_META_IN_HAL);

    MINT32 uniqueKey = 0, frameNumber = 0, requestNumber = 0;
    tryGetMetadata<MINT32>(pMeta_in, MTK_PIPELINE_UNIQUE_KEY, uniqueKey);
    tryGetMetadata<MINT32>(pMeta_in, MTK_PIPELINE_FRAME_NUMBER, frameNumber);
    tryGetMetadata<MINT32>(pMeta_in, MTK_PIPELINE_REQUEST_NUMBER, requestNumber);

    String8 path = String8::format("sdcard/camera_dump");
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);

    if( isCustomerTuning() ){
        // dump all IImageBuffer in request
        IImageBufferSet set = data->getImageBufferSet();
        for(auto const &e : set) {
            const DualCamMFBufferID& BID = e.first;
            const sp<IImageBuffer>& spBuf = e.second;

            MY_LOGD("[Customer] tuning dump for data(%d) BID(%d,%s) spBuf(%p)",
                id, BID, getBIDName(BID), spBuf.get());

            if(spBuf == nullptr){
                MY_LOGD("(%s) spBuf is nullptr, skipped", getBIDName(BID));
                continue;
            }

            if( BID == DualCamMFBufferID::BID_INPUT_RSRAW_1 ||
                BID == DualCamMFBufferID::BID_INPUT_RSRAW_2
            ){
                MY_LOGD("(%s) not used for mfll tuning, skipped", getBIDName(BID));
                continue;
            }

            // [1] normal data dump key
            String8 name = String8::format("%09d-%04d-%04d", uniqueKey, frameNumber, requestNumber);

            // [2] extra naming for yuv buffers
            switch(BID) {
                case DualCamMFBufferID::BID_INPUT_FSRAW_1:
                    name = name + String8::format("-imgo");
                    break;
                case DualCamMFBufferID::BID_INPUT_FSRAW_2:
                    name = name + String8::format("-imgo-main2");
                    break;
                case DualCamMFBufferID::BID_LCS_1:
                    name = name + String8::format("-lcso");
                    break;
                case DualCamMFBufferID::BID_LCS_2:
                    name = name + String8::format("-lcso-main2");
                    break;
                default:
                    name = name + String8::format("-%s-%s", getBIDName(BID), getFormatString(spBuf.get()));
            }

            // [3] image size
            MINT32 width = spBuf->getImgSize().w;
            MINT32 height = spBuf->getImgSize().h;
            name = name + String8::format("__%dx%d", width, height);

            // [4] sensor bit format
            if( BID == DualCamMFBufferID::BID_INPUT_FSRAW_1 ||
                BID == DualCamMFBufferID::BID_INPUT_FSRAW_2 ||
                BID == DualCamMFBufferID::BID_INPUT_RSRAW_1 ||
                BID == DualCamMFBufferID::BID_INPUT_RSRAW_2
            ){
                MINT32 bitNum = 0;
                MINT32 order = 0;
                auto _get_sensor_bit = [](int enumRawBit)->int
                {
                    switch (enumRawBit) {
                        case RAW_SENSOR_8BIT:       return 8;
                        case RAW_SENSOR_10BIT:      return 10;
                        case RAW_SENSOR_12BIT:      return 12;
                        case RAW_SENSOR_14BIT:      return 14;
                        default:                    return 0xFF;
                    }
                    return 0xFF;
                };

                const IHalSensorList* pSlist = MAKE_HalSensorList();
                if( CC_UNLIKELY( pSlist == nullptr) ){
                    MY_LOGW("make hal sensor list failed");
                }else{
                    MUINT32 sensorDev = static_cast<MUINT32>(pSlist->querySensorDevIdx(mSensorIdx_Main1));
                    NSCam::SensorStaticInfo _sinfo;
                    pSlist->querySensorStaticInfo(sensorDev, &_sinfo);
                    bitNum = _get_sensor_bit(_sinfo.rawSensorBit);
                    order = static_cast<int>(_sinfo.sensorFormatOrder);
                }

                name = name + String8::format("_%d_%d", bitNum, order);
            }

            // [5] extension
            switch(BID) {
                case DualCamMFBufferID::BID_INPUT_FSRAW_1:
                case DualCamMFBufferID::BID_INPUT_FSRAW_2:
                case DualCamMFBufferID::BID_INPUT_RSRAW_1:
                case DualCamMFBufferID::BID_INPUT_RSRAW_2:
                    name = name + String8::format(".packed_word");
                    break;
                case DualCamMFBufferID::BID_LCS_1:
                case DualCamMFBufferID::BID_LCS_2:
                    name = name + String8::format(".lcso");
                    break;
                default:
                    name = name + String8::format(".yuv");
            }

            String8 writepath = String8::format("%s/%s", path.c_str(), name.c_str());

            spBuf->saveToFile(writepath.c_str());
        }
    }else{
        // dump all IImageBuffer in request
        IImageBufferSet set = data->getImageBufferSet();
        for(auto const &e : set) {
            const DualCamMFBufferID& BID = e.first;
            const sp<IImageBuffer>& spBuf = e.second;

            MY_LOGD("[Internel] tuning dump for data(%d) BID(%d,%s) spBuf(%p)",
                id, BID, getBIDName(BID), spBuf.get());

            if(spBuf == nullptr){
                MY_LOGD("(%s) spBuf is nullptr, skipped", getBIDName(BID));
                continue;
            }

            if( BID == DualCamMFBufferID::BID_INPUT_FSRAW_2 ||
                BID == DualCamMFBufferID::BID_INPUT_RSRAW_2 ||
                BID == DualCamMFBufferID::BID_LCS_2
            ){
                MY_LOGD("(%s) not used for mfll tuning, skipped", getBIDName(BID));
                continue;
            }

            // [1] normal data dump key
            String8 name = String8::format("%09d-%04d-%04d-mfll", uniqueKey, frameNumber, requestNumber);

            // [2] iso and exposure
            MINT32 iso = getISOFromMeta(pMeta_in);
            MINT32 exp = getEXPFromMeta(pMeta_in);

            name = name + String8::format("-iso-%d-exp-%d", iso, exp);

            // [3] mfll stage, always bfbld(before blend) in DCMFShot
            name = name + String8::format("-bfbld");

            // [3.1] extra naming for yuv buffers
            switch(BID) {
                case DualCamMFBufferID::BID_INPUT_FSRAW_1:
                case DualCamMFBufferID::BID_INPUT_FSRAW_2:
                case DualCamMFBufferID::BID_INPUT_RSRAW_1:
                case DualCamMFBufferID::BID_INPUT_RSRAW_2:
                case DualCamMFBufferID::BID_LCS_1:
                case DualCamMFBufferID::BID_LCS_2:
                    // no extra naming
                    break;
                default:
                    name = name + String8::format("-%s-%s", getBIDName(BID), getFormatString(spBuf.get()));
            }

            // [4] image type
            switch(BID) {
                case DualCamMFBufferID::BID_INPUT_FSRAW_1:
                case DualCamMFBufferID::BID_INPUT_FSRAW_2:
                    name = name + String8::format("-raw");
                    break;
                case DualCamMFBufferID::BID_INPUT_RSRAW_1:
                case DualCamMFBufferID::BID_INPUT_RSRAW_2:
                    name = name + String8::format("-rrzo");
                    break;
                case DualCamMFBufferID::BID_LCS_1:
                case DualCamMFBufferID::BID_LCS_2:
                    name = name + String8::format("-lcso");
                    break;
                default:
                    name = name + String8::format("-yuv");
            }

            // [5] image size
            MINT32 width = spBuf->getImgSize().w;
            MINT32 height = spBuf->getImgSize().h;
            name = name + String8::format("__%dx%d", width, height);

            // [6] sensor bit format
            if( BID == DualCamMFBufferID::BID_INPUT_FSRAW_1 ||
                BID == DualCamMFBufferID::BID_INPUT_FSRAW_2 ||
                BID == DualCamMFBufferID::BID_INPUT_RSRAW_1 ||
                BID == DualCamMFBufferID::BID_INPUT_RSRAW_2
            ){
                MINT32 bitNum = 0;
                MINT32 order = 0;
                auto _get_sensor_bit = [](int enumRawBit)->int
                {
                    switch (enumRawBit) {
                        case RAW_SENSOR_8BIT:       return 8;
                        case RAW_SENSOR_10BIT:      return 10;
                        case RAW_SENSOR_12BIT:      return 12;
                        case RAW_SENSOR_14BIT:      return 14;
                        default:                    return 0xFF;
                    }
                    return 0xFF;
                };

                const IHalSensorList* pSlist = MAKE_HalSensorList();
                if( CC_UNLIKELY( pSlist == nullptr) ){
                    MY_LOGW("make hal sensor list failed");
                }else{
                    MUINT32 sensorDev = static_cast<MUINT32>(pSlist->querySensorDevIdx(mSensorIdx_Main1));
                    NSCam::SensorStaticInfo _sinfo;
                    pSlist->querySensorStaticInfo(sensorDev, &_sinfo);
                    bitNum = _get_sensor_bit(_sinfo.rawSensorBit);
                    order = static_cast<int>(_sinfo.sensorFormatOrder);
                }

                name = name + String8::format("_%d_%d", bitNum, order);
            }

            // [7] extension
            switch(BID) {
                case DualCamMFBufferID::BID_INPUT_FSRAW_1:
                case DualCamMFBufferID::BID_INPUT_FSRAW_2:
                case DualCamMFBufferID::BID_INPUT_RSRAW_1:
                case DualCamMFBufferID::BID_INPUT_RSRAW_2:
                    name = name + String8::format(".packed_word");
                    break;
                case DualCamMFBufferID::BID_LCS_1:
                case DualCamMFBufferID::BID_LCS_2:
                    name = name + String8::format(".lcso");
                    break;
                default:
                    name = name + String8::format(".yuv");
            }

            String8 writepath = String8::format("%s/%s", path.c_str(), name.c_str());

            spBuf->saveToFile(writepath.c_str());
        }
    }

    MY_LOGD("dataID:%d -", id);
    return MTRUE;
}

MBOOL
DualCamMFPipeNode::
handleData(DataID id, PipeRequestPtr pReq)
{
    if(pReq != nullptr){
        VSDOF_LOGD("handleData(request): %d", pReq->getRequestNo());
    }
    this->onDump(id, pReq);
    return CamThreadNode<DualCamMFPipeDataHandler>::handleData(id, pReq);
}

MBOOL
DualCamMFPipeNode::
handleData(DataID id, ImgInfoMapPtr data)
{
    if(data != nullptr){
        VSDOF_LOGD("handleData(imgInfo): regId:%d", data->getRequestPtr()->getRequestNo());
    }
    this->onDump(id, data);
    return CamThreadNode<DualCamMFPipeDataHandler>::handleData(id, data);
}

MBOOL
DualCamMFPipeNode::
handleData(DataID id, MINT32 data)
{
    VSDOF_LOGD("handleData(MINT32): %d", data);
    return CamThreadNode<DualCamMFPipeDataHandler>::handleData(id, data);
}

MBOOL
DualCamMFPipeNode::
handleData(DataID id, Vector<MINT32>& data)
{
    VSDOF_LOGD("handleData(Vector<MINT32>): data.size:%d", data.size());
    return CamThreadNode<DualCamMFPipeDataHandler>::handleData(id, data);
}

MVOID
DualCamMFPipeNode::
dumpRequestData(DualCamMFBufferID BID, PipeRequestPtr pReq)
{
    VSDOF_LOGD("%d", BID);
    if(shouldDumpRequest(pReq)){
        if( pReq->getImageBuffer(BID) != nullptr){
            handleDump(pReq->getImageBuffer(BID).get(), BID, pReq->getRequestNo());
        }else{
            MY_LOGE("no data(%d)(%s) in req!", BID, getBIDName(BID));
        }
    }
}

MVOID
DualCamMFPipeNode::
setP2Operator(sp<P2Operator> spP2Op)
{
    VSDOF_LOGD("setP2Operator: %p", spP2Op.get());
    mwpP2Operator = spP2Op;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  DualCamMFPipeNode Protect Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
const char*
DualCamMFPipeNode::
getBIDName(DualCamMFBufferID BID)
{
    switch(BID){
        case DualCamMFBufferID::BID_INVALID:
            return "BID_INVALID";

        case DualCamMFBufferID::BID_INPUT_FSRAW_1:
            return "BID_INPUT_FSRAW_1";
        case DualCamMFBufferID::BID_INPUT_FSRAW_2:
            return "BID_INPUT_FSRAW_2";
        case DualCamMFBufferID::BID_INPUT_RSRAW_1:
            return "BID_INPUT_RSRAW_1";
        case DualCamMFBufferID::BID_INPUT_RSRAW_2:
            return "BID_INPUT_RSRAW_2";

        case DualCamMFBufferID::BID_LCS_1:
            return "BID_LCS_1";
        case DualCamMFBufferID::BID_LCS_2:
            return "BID_LCS_2";

        case DualCamMFBufferID::BID_POSTVIEW:
            return "BID_POSTVIEW";
        case DualCamMFBufferID::BID_FS_YUV_1:
            return "BID_FS_YUV_1";
        case DualCamMFBufferID::BID_FS_YUV_2:
            return "BID_FS_YUV_2";
        case DualCamMFBufferID::BID_RS_YUV_1:
            return "BID_RS_YUV_1";
        case DualCamMFBufferID::BID_RS_YUV_2:
            return "BID_RS_YUV_2";
        case DualCamMFBufferID::BID_SM_YUV:
            return "BID_SM_YUV";
        default:
            MY_LOGW("unknown BID:%d", BID);
            return "unknown";
    }
}

const char*
DualCamMFPipeNode::
getBIDPostfix(DualCamMFBufferID BID)
{
    switch(BID){
        case DualCamMFBufferID::BID_INPUT_FSRAW_1:
        case DualCamMFBufferID::BID_INPUT_FSRAW_2:
        case DualCamMFBufferID::BID_INPUT_RSRAW_1:
        case DualCamMFBufferID::BID_INPUT_RSRAW_2:
            return "raw";
        default:
            return "yuv";
    }
}

const char*
DualCamMFPipeNode::
getProcessName(DualCamMFProcessId PID)
{
    #define MAKE_NAME_CASE(name) \
        case name: return #name;

    switch(PID)
    {
        MAKE_NAME_CASE(UNKNOWN);
        MAKE_NAME_CASE(FSRAW_TO_YUV_1);
        MAKE_NAME_CASE(FSRAW_TO_YUV_2);
        MAKE_NAME_CASE(RSRAW_TO_YUV_1);
        MAKE_NAME_CASE(RSRAW_TO_YUV_2);
        MAKE_NAME_CASE(FINAL_PROCESS);
    }
    MY_LOGW("unknown PID:%d", PID);

    return "unknown";
    #undef MAKE_NAME_CASE
}

MBOOL
DualCamMFPipeNode::
formatConverter(
        IImageBuffer *imgSrc,
        IImageBuffer *imgDst)
{
    MY_LOGD("%s +", __FUNCTION__);
    MBOOL ret = MFALSE;

    ISImager *pISImager = NULL;
    pISImager = ISImager::createInstance(imgSrc);
    if (!pISImager) {
        MY_LOGE("ISImager::createInstance() failed!!!");
        goto lbExit;
    }

    if (!pISImager->setTargetImgBuffer(imgDst)) {
        MY_LOGE("setTargetImgBuffer failed!!!");
        goto lbExit;
    }

    if (!pISImager->execute()) {
        MY_LOGE("execute failed!!!");
        goto lbExit;
    }

    ret = MTRUE;

lbExit:
    if (pISImager) {
        pISImager->destroyInstance();
    }

    MY_LOGD("%s -", __FUNCTION__);
    return ret;
}

MVOID
DualCamMFPipeNode::
MET_START(const char* tag_name)
{
#if MET_USER_EVENT_SUPPORT
    met_tag_start(0, tag_name);
#endif
}

MVOID
DualCamMFPipeNode::
MET_END(const char* tag_name)
{
#if MET_USER_EVENT_SUPPORT
    met_tag_end(0, tag_name);
#endif
}

MVOID
DualCamMFPipeNode::
initBufferPool()
{
    MY_LOGD("do nothing");
}

MBOOL
DualCamMFPipeNode::
doBufferPoolAllocation(MUINT32 count)
{
    MY_LOGD("do nothing");
    return MFALSE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualCamMFPipeNode::
prepareModuleSettings()
{
    MY_LOGD("+");

    // module rotation
    ENUM_ROTATION eRot = StereoSettingProvider::getModuleRotation();
    mModuleTrans = eRotationToeTransform(eRot);
    MY_LOGD("mModuleTrans=%d", mModuleTrans);

    MY_LOGD("-");
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32
DualCamMFPipeNode::
eRotationToeTransform(ENUM_ROTATION rotation)
{
    MINT32 ret = -1;
    switch(rotation)
    {
        case eRotate_0:
            ret = 0;
            break;
        case eRotate_90:
            ret = eTransform_ROT_90;
            break;
        case eRotate_180:
            ret = eTransform_ROT_180;
            break;
        case eRotate_270:
            ret = eTransform_ROT_270;
            break;
        default:
            MY_LOGE("unknow format = %d", rotation);
            break;
    }
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
ENUM_ROTATION
DualCamMFPipeNode::
eTransformToRotation(MINT32 eTrans)
{
    ENUM_ROTATION ret = eRotate_0;
    switch(eTrans)
    {
        case 0:
            ret = eRotate_0;
            break;
        case eTransform_ROT_90:
            ret = eRotate_90;
            break;
        case eTransform_ROT_180:
            ret = eRotate_180;
            break;
        case eTransform_ROT_270:
            ret = eRotate_270;
            break;
        default:
            MY_LOGE("unknow format = %d", eTrans);
            break;
    }
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32
DualCamMFPipeNode::
eTransformToDegree(MINT32 eTrans)
{
    MINT32 ret = 0;
    switch(eTrans)
    {
        case 0:
            ret = 0;
            break;
        case eTransform_ROT_90:
            ret = 90;
            break;
        case eTransform_ROT_180:
            ret = 180;
            break;
        case eTransform_ROT_270:
            ret = 270;
            break;
        default:
            MY_LOGE("Not support eTransform=%d", eTrans);
    }
    MY_LOGD("trans:%d -> degree:%d", eTrans, ret);
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
DualCamMFPipeNode::
DegreeToeTransform(MINT32 degree)
{
    MINT32 ret = 0;
    switch(degree)
    {
        case 0:
            ret = 0;
            break;
        case 90:
            ret = eTransform_ROT_90;
            break;
        case 180:
            ret = eTransform_ROT_180;
            break;
        case 270:
            ret = eTransform_ROT_270;
            break;
        default:
            MY_LOGE("Not support degree=%d", degree);
    }
    MY_LOGD("degree:%d -> trans:%d", degree, ret);
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
TuningParam
DualCamMFPipeNode::
applyISPTuning(
    const char* name,
    SmartTuningBuffer& targetTuningBuf,
    const ISPTuningConfig& ispConfig,
    IMetadata* pMeta_hal_out
)
{
    MY_LOGD("+, [%s] reqID=%d bIsResized=%d", name, ispConfig.reqNo, ispConfig.bInputResizeRaw);

    NS3Av3::TuningParam tuningParam = {NULL, NULL};
    tuningParam.pRegBuf = reinterpret_cast<void*>(targetTuningBuf->mpVA);

    // LCEI
    if(ispConfig.pLcsBuf != nullptr){
        if(ispConfig.bInputResizeRaw){
            MY_LOGE("use LCSO for RRZO buffer, should not happended!");
            return tuningParam;
        }
        MY_LOGD("have LCSO Buf");
        tuningParam.pLcsBuf = reinterpret_cast<void*>(ispConfig.pLcsBuf);
    }

    MetaSet_T inMetaSet;
    IMetadata* pMeta_InApp  = ispConfig.pInAppMeta;
    IMetadata* pMeta_InHal  = ispConfig.pInHalMeta;

    inMetaSet.appMeta = *pMeta_InApp;
    inMetaSet.halMeta = *pMeta_InHal;

    MUINT8 profile = -1;
    if(tryGetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, profile)){
        MY_LOGD("Profile:%d", profile);
    }else{
        MY_LOGW("Failed getting profile!");
    }

    // USE resize raw-->set PGN 0
    if(ispConfig.bInputResizeRaw){
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
    }else{
        updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
    }

#ifndef GTEST
    MetaSet_T resultMeta;
    ispConfig.p3AHAL->setIsp(0, inMetaSet, &tuningParam, &resultMeta);

    if(pMeta_hal_out != nullptr){
        // write ISP resultMeta to output hal Meta
        (*pMeta_hal_out) += resultMeta.halMeta;
    }
#else
    MY_LOGW("GTEST do nothing");
#endif

    MY_LOGD("-, [%s] reqID=%d", name, ispConfig.reqNo);
    return tuningParam;
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
DualCamMFPipeNode::
getJpegRotation(IMetadata* pMeta)
{
    MINT32 jpegRotationApp = 0;
    if(!tryGetMetadata<MINT32>(pMeta, MTK_JPEG_ORIENTATION, jpegRotationApp))
    {
        MY_LOGE("Get jpegRotationApp failed!");
    }
    MINT32 rotDegreeJpeg = jpegRotationApp;
    if(rotDegreeJpeg < 0){
        rotDegreeJpeg = rotDegreeJpeg + 360;
    }
    MY_LOGD("jpegRotationApp:%d, rotDegreeJpeg:%d", jpegRotationApp, rotDegreeJpeg);
    return DegreeToeTransform(rotDegreeJpeg);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualCamMFPipeNode::
shouldDumpRequest(PipeRequestPtr pReq)
{
    // MUINT iReqIdx = pReq->getRequestNo();
    // if(iReqIdx < miDumpStartIdx || iReqIdx >= miDumpStartIdx + miDumpBufSize){
    //     return MFALSE;
    // }else{
    //     return MTRUE;
    // }
    return miTuningDump;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualCamMFPipeNode::
shouldDumpForTuning(DualCamMFBufferID BID)
{
    return MFALSE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualCamMFPipeNode::
isCustomerTuning()
{
    return (miCustomerTuning == 1) ? MTRUE : MFALSE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DualCamMFPipeNode::
systemCall(const char* cmd)
{
    MY_LOGD("%s", cmd);
    system(cmd);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DualCamMFPipeNode::
updateSrcCropByOutputRatio(MRect& rSrcCrop, sp<IImageBuffer> src, sp<IImageBuffer> dst, MINT32 dstTrans)
{
    MRect srcCrop_origin = rSrcCrop;
    MSize dstSize = dst->getImgSize();
    if(dstTrans & eTransform_ROT_90){
        swap(dstSize.w, dstSize.h);
    }

    MSize srcSize = src->getImgSize();
    if(srcCrop_origin.s.w > srcSize.w){
        MY_LOGW("crop width exceed src width, changed to source width");
        srcCrop_origin.s.w = srcSize.w;
    }
    if(srcCrop_origin.s.h > srcSize.h){
        MY_LOGW("crop height exceed src height, changed to source height");
        srcCrop_origin.s.h = srcSize.h;
    }

    #define THRESHOLD (0.01)
    float ratioSrc = (float)srcSize.w/srcSize.h;
    float ratioDst = (float)dstSize.w/dstSize.h;
    float ratioDiff = std::abs(ratioDst - ratioSrc);
    MBOOL isSameRatio = (ratioDiff < THRESHOLD);

    MY_LOGD("src ratio(%f), dst ratio(%f), diff(%f) thres(%f) isSameRatio(%d)",
        ratioSrc, ratioDst, ratioDiff,
        THRESHOLD, isSameRatio
    );
    #undef THRESHOLD

    if( isSameRatio ){
        // same ratio, no need to crop
    }else{
        // calculate the required image hight according to image ratio
        #define APPLY_2_ALIGN(x)  ((int(x)>>1)<<1)
        const double OUTPUT_RATIO = ((double)dstSize.w) / ((double)dstSize.h);
        rSrcCrop.s = srcCrop_origin.s;
        rSrcCrop.s.h = APPLY_2_ALIGN(rSrcCrop.s.w / OUTPUT_RATIO);
        if(rSrcCrop.s.h > srcCrop_origin.s.h) {
            rSrcCrop.s.h = APPLY_2_ALIGN(srcCrop_origin.s.h);
            rSrcCrop.s.w = APPLY_2_ALIGN(rSrcCrop.s.h * OUTPUT_RATIO);
        } else {
            rSrcCrop.s.w = APPLY_2_ALIGN(srcCrop_origin.s.w);
        }
        #undef APPLY_2_ALIGN
        rSrcCrop.p.x = (srcCrop_origin.s.w - rSrcCrop.s.w)/2;
        rSrcCrop.p.y = (srcCrop_origin.s.h - rSrcCrop.s.h)/2;
    }

    MY_LOGD("srcSize(%dx%d), dstSize(%dx%d), dstTrans(%d) isSameRatio(%d) crop(%d,%d,%dx%d)->(%d,%d,%dx%d)",
        srcSize.w, srcSize.h, dstSize.w, dstSize.h, dstTrans, isSameRatio,
        srcCrop_origin.p.x, srcCrop_origin.p.y, srcCrop_origin.s.w, srcCrop_origin.s.h,
        rSrcCrop.p.x, rSrcCrop.p.y, rSrcCrop.s.w, rSrcCrop.s.h
    );
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DualCamMFPipeNode::
setRawDumpExtNames(ImgInfoMapPtr imgInfoMap, const DualCamMFBufferID& bid)
{
    // pass1 raw is dump in unpak format
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!shouldDumpRequest(imgInfoMap->getRequestPtr())){
        return;
    }

    // width
    string extBufName = to_string(imgInfoMap->getIImageBuffer(bid)->getImgSize().w) + "x";

    // height
    extBufName = extBufName + to_string(imgInfoMap->getIImageBuffer(bid)->getImgSize().h) + "_";

    // Strides
    extBufName = extBufName + to_string(imgInfoMap->getIImageBuffer(bid)->getBufStridesInBytes(0)) + "_";

    // bayer order
    MINT32 bayerOrder = -1;
    if(bid == DualCamMFBufferID::BID_INPUT_FSRAW_1 || bid == DualCamMFBufferID::BID_INPUT_FSRAW_2){
        bayerOrder = mBayerOrder_main1;
    }else if(bid == DualCamMFBufferID::BID_INPUT_RSRAW_1 || bid == DualCamMFBufferID::BID_INPUT_RSRAW_2){
        bayerOrder = mBayerOrder_main2;
    }else{
        MY_LOGE("un-supported BID!");
    }
    extBufName = extBufName + to_string(bayerOrder);

    MY_LOGD("extBufName:%s", extBufName.c_str());
    imgInfoMap->addExtBufName(bid, extBufName);
}

/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DualCamMFPipeNode::
fillInPQParam(PQParam* const pPQParam, MUINT32 processId)
{
    auto fiilPQ = [&](DpPqParam* rISPParam){
        rISPParam->enable           = false;
        rISPParam->scenario         = MEDIA_ISP_CAPTURE;

        rISPParam->u.isp.iso        = 0; // dummy value because PQ should be disabled
        rISPParam->u.isp.lensId     = DCMF_FEAUTRE_UNIQUE_ID;
        rISPParam->u.isp.timestamp  = processId;
        rISPParam->u.isp.frameNo    = processId;
        rISPParam->u.isp.requestNo  = processId;

        rISPParam->u.isp.clearZoomParam.captureShot = CAPTURE_SINGLE;
    };

    // WROT
    DpPqParam*  ISPParam_WROT = new DpPqParam();
    fiilPQ(ISPParam_WROT);
    pPQParam->WROTPQParam = static_cast<void*>(ISPParam_WROT);

    // WDMA
    DpPqParam*  ISPParam_WDMA = new DpPqParam();
    fiilPQ(ISPParam_WDMA);
    pPQParam->WDMAPQParam = static_cast<void*>(ISPParam_WDMA);

    MY_LOGD("PQ params: lensId(%d) captureShot(%d) timestamp(%d) frameNo(%d) requestNo(%d) enable(%d) scenario(%d) iso(%d)",
        ISPParam_WROT->u.isp.lensId,
        ISPParam_WROT->u.isp.clearZoomParam.captureShot,
        ISPParam_WROT->u.isp.timestamp,
        ISPParam_WROT->u.isp.frameNo,
        ISPParam_WROT->u.isp.requestNo,
        ISPParam_WROT->enable,
        ISPParam_WROT->scenario,
        ISPParam_WROT->u.isp.iso
    );
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualCamMFPipeNode::
fillInSrzParam(ModuleInfo* pModuleInfo, MUINT32 processId, sp<IImageBuffer> pBuf_lcso, sp<IImageBuffer> pBuf_raw)
{
    if( mDipInfo[EDIPINFO_DIPVERSION] != EDIPHWVersion_50 ){
        MY_LOGD("isp version(%d) < 5.0, dont need SrzInfo", mDipInfo[EDIPINFO_DIPVERSION]);
        return MFALSE;
    }
    // srz4 config
    // ModuleInfo srz4_module;
    pModuleInfo->moduleTag = EDipModule_SRZ4;
    pModuleInfo->frameGroup=0;

    _SRZ_SIZE_INFO_    *pSrzParam = new _SRZ_SIZE_INFO_;
    pSrzParam->in_w = pBuf_lcso->getImgSize().w;
    pSrzParam->in_h = pBuf_lcso->getImgSize().h;
    pSrzParam->crop_floatX = 0;
    pSrzParam->crop_floatY = 0;
    pSrzParam->crop_x = 0;
    pSrzParam->crop_y = 0;
    pSrzParam->crop_w = pBuf_lcso->getImgSize().w;
    pSrzParam->crop_h = pBuf_lcso->getImgSize().h;
    pSrzParam->out_w = pBuf_raw->getImgSize().w;
    pSrzParam->out_h = pBuf_raw->getImgSize().h;

    pModuleInfo->moduleStruct   = reinterpret_cast<MVOID*> (pSrzParam);

    MY_LOGD("Srz params: processId(%d) in(lcso)(%dx%d) crop(%d,%d,%dx%d) out(raw)(%dx%d)", processId,
        pSrzParam->in_w, pSrzParam->in_h,
        pSrzParam->crop_x, pSrzParam->crop_y, pSrzParam->crop_w, pSrzParam->crop_h,
        pSrzParam->out_w, pSrzParam->out_h
    );

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MRect
DualCamMFPipeNode::
getFovRatioCrop(const MSize srcImgSize, const MSize dstImgSize, MBOOL isCropMain)
{
    MY_LOGW("naive version, no crop");
    MRect ret(srcImgSize.w, srcImgSize.h);
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualCamMFPipeNode::
waitAndAddExclusiveJob(const MUINT32 reqNo)
{
    MY_LOGD("req(%d) +", reqNo);
    Mutex::Autolock _l(mLock);

    std::chrono::nanoseconds maxWaitTimeNS(std::chrono::seconds(3));

    while(!mProcessingRequestSet.empty()){
        MY_LOGD("mProcessingRequestSet not empty(%lu), wait for it +", mProcessingRequestSet.size());
        auto ret = mCond.waitRelative(mLock, maxWaitTimeNS.count());
        MY_LOGD("mProcessingRequestSet not empty(%lu), wait for it -", mProcessingRequestSet.size());
        if(ret != OK){
            MY_LOGW("mProcessingRequestSet wait timeout!");
            break;
        }
    }

    if(mProcessingRequestSet.count(reqNo) == 0){
        mProcessingRequestSet.insert(reqNo);
    }else{
        MY_LOGE("reqNo:%d already in mProcessingRequestSet, should not happened!", reqNo);
        return MFALSE;
    }

    MY_LOGD("req(%d) -", reqNo);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualCamMFPipeNode::
finishExclusiveJob(const MUINT32 reqNo)
{
    MY_LOGD("req(%d) +", reqNo);
    Mutex::Autolock _l(mLock);

    if(mProcessingRequestSet.count(reqNo) == 0){
        CAM_LOGD("cant find req:%d in mProcessingRequestSet!", reqNo);
        return MFALSE;
    }else{
        mProcessingRequestSet.erase(reqNo);
        mCond.signal();
    }

    MY_LOGD("req(%d) -", reqNo);
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32
DualCamMFPipeNode::
getISOFromMeta(const IMetadata* const pHalMeta)
{
    MINT32 __iso = 100;

    IMetadata exifMeta;
    if( IMetadata::getEntry<IMetadata>(pHalMeta, MTK_3A_EXIF_METADATA, exifMeta) ){
        if( IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, __iso) ){
            MY_LOGD("MTK_3A_EXIF_AE_ISO_SPEED:%d", __iso);
        }else{
            MY_LOGW("Get ISO failed");
        }
    }else{
        MY_LOGW("Get 3A exif failed");
    }

    return __iso;;
}

/*******************************************************************************
 *
 ********************************************************************************/
const char*
DualCamMFPipeNode::
getFormatString(const IImageBuffer* const pBuf)
{
    switch (pBuf->getImgFormat()){
        case eImgFmt_NV12:      return "nv12";
        case eImgFmt_NV21:      return "nv21";
        case eImgFmt_YV12:      return "yv12";
        case eImgFmt_YUY2:      return "yuy2";
        case eImgFmt_Y8:        return "y8";
        default:break;
    }
    return "unknown";
}

/*******************************************************************************
 *
 ********************************************************************************/
MINT32
DualCamMFPipeNode::
getEXPFromMeta(const IMetadata* const pHalMeta)
{
    MINT32 __exp = 0;

    IMetadata exifMeta;
    if( IMetadata::getEntry<IMetadata>(pHalMeta, MTK_3A_EXIF_METADATA, exifMeta) ){
        if( IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME, __exp) ){
            MY_LOGD("MTK_3A_EXIF_CAP_EXPOSURE_TIME:%d", __exp);
        }else{
            MY_LOGW("Get EXP failed");
        }
    }else{
        MY_LOGW("Get 3A exif failed");
    }

    return __exp;;
}

/*******************************************************************************
 *
 ********************************************************************************/
AutoScenCtrl::
AutoScenCtrl(const char *userName, MINT32 openId)
: mUserName(userName)
{
    mpScenarioCtrl = IScenarioControl::create(MAGIC_ID_SCENARIO_CTRL);

    NSCamHW::HwInfoHelper helper(openId);
    MSize sensorSize;
    if( ! helper.updateInfos() ) {
        MY_LOGE("cannot properly update infos!");
        return;
    }
    //
    if( ! helper.getSensorSize( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, sensorSize) ) {
        MY_LOGE("cannot get params about sensor!");
        return;
    }

    MUINT featureFlagStereo = 0;
    FEATURE_CFG_ENABLE_MASK(featureFlagStereo, IScenarioControl::FEATURE_BMDENOISE_CAPTURE);

    IScenarioControl::ControlParam controlParam;
    controlParam.scenario = IScenarioControl::Scenario_ZsdPreview;
    controlParam.sensorSize = sensorSize;
    controlParam.sensorFps = 30;
    controlParam.featureFlag = featureFlagStereo;
    controlParam.enableBWCControl = MFALSE;

    MY_LOGD("[%s] Enter Scenario (%d)", mUserName, IScenarioControl::FEATURE_BMDENOISE_CAPTURE);
    mpScenarioCtrl->enterScenario(controlParam);
}

/*******************************************************************************
 *
 ********************************************************************************/
AutoScenCtrl::
~AutoScenCtrl()
{
   MY_LOGD("[%s] Exit Scenario (%d)", mUserName, IScenarioControl::FEATURE_BMDENOISE_CAPTURE);
    mpScenarioCtrl->exitScenario();
 }