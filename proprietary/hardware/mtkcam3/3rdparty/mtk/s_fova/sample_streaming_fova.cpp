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

#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <cutils/properties.h>
//
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
// FD
#include <mtkcam/utils/hw/IFDContainer.h>
#include <cmath>
#include <bitset>
//
#include <vsdof_util.h>
#include <DebugUtil.h>
#include <stereo_crop_util.h>
//
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>

using NSCam::NSPipelinePlugin::Interceptor;
using NSCam::NSPipelinePlugin::PipelinePlugin;
using NSCam::NSPipelinePlugin::PluginRegister;
using NSCam::NSPipelinePlugin::Join;
using NSCam::NSPipelinePlugin::JoinPlugin;

using namespace NSCam::NSPipelinePlugin;
using NSCam::MSize;
using NSCam::MERROR;
using NSCam::IImageBuffer;
using NSCam::IMetadata;
using NSCam::Utils::makePath;
using VSDOF::util::sMDP_Config;

#ifdef LOG_TAG
#undef LOG_TAG
#endif // LOG_TAG
#define LOG_TAG "MtkCam/Plugin_S_FOVA"
#include <log/log.h>
#include <android/log.h>

#define MY_LOGI(fmt, arg...)  ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)  ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)  ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)  ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI_IF(cond, ...) do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...) do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...) do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define FUNCTION_IN   MY_LOGD("%s +", __FUNCTION__)
#define FUNCTION_OUT  MY_LOGD("%s -", __FUNCTION__)

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_TPI_PLUGIN);
#define THIRDPARTY_REPLACEMENT

template<typename T>
inline MBOOL
GET_ENTRY_ARRAY(IMetadata const *metadata, MINT32 entry_tag, T* array, MUINT32 size)
{
    NSCam::IMetadata::IEntry entry = metadata->entryFor(entry_tag);
    if ( (entry.tag() != NSCam::IMetadata::IEntry::BAD_TAG) && (entry.count() == size) )
    {
        for (MUINT32 i = 0; i < size; i++)
        {
            *array++ = entry.itemAt(i, NSCam::Type2Type< T >());
        }
        return MTRUE;
    }
    return MFALSE;
}

template <typename T>
inline bool
tryGetMetadata(IMetadata const *pMetadata, MUINT32 tag, T& rVal)
{
    if (pMetadata == nullptr) return MFALSE;

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty())
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
//
// namespace NSCam {
// namespace NSPipelinePlugin {
class S_FOVA_Plugin : public JoinPlugin::IProvider
{
public:
    typedef JoinPlugin::Property Property;
    typedef JoinPlugin::Selection Selection;
    typedef JoinPlugin::Request::Ptr RequestPtr;
    typedef JoinPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    S_FOVA_Plugin();
    ~S_FOVA_Plugin();
    const Property& property();
    void set(MINT32 openID1, MINT32 openID2);
    void init();
    void uninit();
    MERROR negotiate(Selection& sel);
    MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback);
    void abort(std::vector<RequestPtr> &pRequests);

private:
    MERROR getConfigSetting(Selection &sel);

private:
    int    mOpenID1 = 0;
    int    mOpenID2 = 0;

private:
    enum valid_input_state {
        ALL_MISS        = -1,
        INIT_STATE      = 0,
        MAIN1_ONLY      = 1,
        MAIN2_ONLY      = 2,
        BOTH_VALID      = 3,
        MISS_MAIN1_META = 4,
        MISS_MAIN2_META = 5,
        MISS_BOTH_META  = 6,
    };

    std::map<enum valid_input_state, std::string> stateMap = {
        {ALL_MISS,        "ALL_MISS"},
        {INIT_STATE,      "INIT_STATE"},
        {MAIN1_ONLY,      "MAIN1_ONLY"},
        {MAIN2_ONLY,      "MAIN2_ONLY"},
        {BOTH_VALID,      "BOTH_VALID"},
        {MISS_MAIN1_META, "MISS_MAIN1_META"},
        {MISS_MAIN2_META, "MISS_MAIN2_META"},
        {MISS_BOTH_META,  "MISS_BOTH_META"}
    };
    /**
     * @brief an example to process 2 src image
     * @param [in] source1 buffer
     * @param [in] source2 buffer
     * @param [in] target buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    bool imgCpy2Target( RequestPtr const  pRequest,
            IImageBuffer* pSrc1,
            IImageBuffer* target
            );
    /**
     * @Debug Purpose to confirm output buffer
     * @param [in] target buffer to dump
     * @param [in] dump scenario. 1:Main1 Origin, 2:Main2 Origin
     *                            3:Resize Main2, 4:Merge Main1&Main2
     * @param [in] corresponding metatdata, i.e. main1 or main2
     */
    void save2file(IImageBuffer* img, int idx, IMetadata *meta);

private:
    bool     mLogEn       = true;
    bool     mSetImgSize  = false;
    bool     mbDumpBuffer = false;
    bool     mbROIUpdate  = true;
    uint32_t mCoordinate  = 0;
    DpBlitStream* mpDpBlitStream = nullptr;

    TuningUtils::FILE_DUMP_NAMING_HINT mDumpHint_Main1;
};

S_FOVA_Plugin::S_FOVA_Plugin()
{
    CAM_ULOGM_APILIFE();

    mbDumpBuffer = ::property_get_bool("vendor.debug.vsdof.fova.dumpbuffer"  , 0);
    mSetImgSize  = ::property_get_bool("vendor.debug.vsdof.fova.specifysize" , 0);
    mCoordinate  = ::property_get_int32("vendor.debug.vsdof.fova.cooridinate", 0);

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);

    if (mbDumpBuffer)
        extract_by_SensorOpenId(&mDumpHint_Main1, main1Idx);

    if (mbDumpBuffer)
        mpDpBlitStream = new DpBlitStream();
}

S_FOVA_Plugin::~S_FOVA_Plugin()
{
    MY_LOGI("destroy S_FOVA plugin");

    if (mbDumpBuffer)
        delete mpDpBlitStream;

    mpDpBlitStream = nullptr;
}

void S_FOVA_Plugin::set(MINT32 openID1, MINT32 openID2)
{
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    mOpenID1 = openID1;
    mOpenID2 = openID2;
}

const S_FOVA_Plugin::Property& S_FOVA_Plugin::property()
{
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName     = "MTK TK_3RD_FOVA_Plugin";
        prop.mFeatures = MTK_FEATURE_FOVA;//@ mtk_platform_metadata_tag.h
        //prop.mInPlace = MTRUE;
        //prop.mFaceData = eFD_Current;
        //prop.mPosition = 0;
        inited = true;
    }
    return prop;
}

MERROR S_FOVA_Plugin::negotiate(Selection &sel)
{
    MERROR ret = OK;

    int stereoMode = StereoSettingProvider::getStereoFeatureMode();
    if (!(stereoMode & v1::Stereo::E_DUALCAM_FEATURE_ZOOM)) {
        ret = BAD_VALUE;
        MY_LOGI("Dual Feature Mode is (%d)", stereoMode);
        return ret;
    }

    if (sel.mSelStage == eSelStage_CFG) {
        ret = getConfigSetting(sel);
    }
    MY_LOGD_IF(mLogEn > 0, "mSelStage input is (%d) mode(%d)",
               sel.mSelStage, stereoMode);

    return ret;
}

MERROR S_FOVA_Plugin::getConfigSetting(Selection &sel)

{
    CAM_ULOGM_APILIFE();
    MERROR ret = OK;

    sel.mCfgOrder     = 1;//1st Plugin
    sel.mCfgJoinEntry = eJoinEntry_S_YUV;
    sel.mCfgEnableFD  = MTRUE;
    sel.mCfgRun       = true;

    enum EImageFormat fmt = (property_get_bool("vendor.debug.tpi.s.fova.yv21", 0)) ?
                              eImgFmt_YV12 : eImgFmt_NV21 ;

    {// INPUT
        sel.mIBufferMain1.setRequired(true).addAcceptedFormat(fmt);
        sel.mIBufferMain2.setRequired(true).addAcceptedFormat(fmt);

        sel.mIMetadataApp.setRequired(true);
        sel.mIMetadataHal1.setRequired(true);
        sel.mIMetadataHal2.setRequired(true);
        sel.mIMetadataDynamic1.setRequired(true);
        sel.mIMetadataDynamic2.setRequired(true);
    }

    {// OUTPUT
        sel.mOBufferMain1 .setRequired(true).addAcceptedFormat(fmt);

        sel.mOMetadataApp.setRequired(MTRUE);
        sel.mOMetadataHal.setRequired(MTRUE);
    }
    //
    if (mSetImgSize) {
        sel.mCfgInfo.mMaxOutSize = StereoSizeProvider::getInstance()->getPreviewSize();
        //
        sel.mIBufferMain1.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
        sel.mIBufferMain2.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
        sel.mOBufferMain1.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
    } else {
        sel.mIBufferMain1.addAcceptedSize(eImgSize_Full);
        sel.mIBufferMain2.addAcceptedSize(eImgSize_Full);
        sel.mOBufferMain1.addAcceptedSize(eImgSize_Full);
    }

    MY_LOGI_IF(sel.mCfgRun == false, "S_FOVA_Plugin is not config as run state(%d)",
               sel.mCfgRun);

    return ret;
}

void S_FOVA_Plugin::init()
{
    MY_LOGI("init S_FOVA plugin");
}

void S_FOVA_Plugin::uninit()
{
    MY_LOGI("uninit S_FOVA plugin");
}

void S_FOVA_Plugin::abort(std::vector<RequestPtr> &pRequests)
{
    (void)pRequests;
    MY_LOGD("uninit S_FOVA plugin");
};
/******************************************************************************
 *
 ******************************************************************************/

MERROR S_FOVA_Plugin::process(RequestPtr pRequest, RequestCallbackPtr /*pCallback*/)
{
    CAM_ULOGM_APILIFE();
    MINT32 camInx[2] = {2,2};//set unexpected value on purpose
    MERROR ret = OK;

    auto isValidOutput = [](const RequestPtr& requestPtr) -> MBOOL
    {
        const MBOOL ret = requestPtr->mOBufferMain1 != nullptr &&
                          requestPtr->mOMetadataHal != nullptr &&
                          requestPtr->mOMetadataApp != nullptr;
        if (!ret) {
            MY_LOGE("invalid request with OUTPUT, reqAddr:%p,"
                    "outBufImg:%p, outAppMeta:%p, outHalMeta:%p",
                    requestPtr.get(), requestPtr->mOBufferMain1.get(),
                    requestPtr->mOMetadataApp.get(), requestPtr->mOMetadataHal.get());
        }
        return ret;
    };

    if (!isValidOutput(pRequest)) {
        MY_LOGE("BAD output");
        return BAD_VALUE;
    }

    auto validInputDispatch = [](const RequestPtr& requestPtr) -> valid_input_state
    {
        enum valid_input_state ret = INIT_STATE;
        //
        if (requestPtr->mIBufferMain1 != nullptr &&
                requestPtr->mIBufferMain2 == nullptr) {
            ret = (requestPtr->mIMetadataHal1 != nullptr) ? MAIN1_ONLY : MISS_MAIN1_META;
        } else if (requestPtr->mIBufferMain1 == nullptr &&
                 requestPtr->mIBufferMain2 != nullptr) {
            ret = (requestPtr->mIMetadataHal2 != nullptr) ? MAIN2_ONLY : MISS_MAIN2_META;
        } else if (requestPtr->mIBufferMain1 != nullptr &&
                 requestPtr->mIBufferMain2 != nullptr) {
            ret = (requestPtr->mIMetadataHal1 != nullptr &&
                   requestPtr->mIMetadataHal2 != nullptr) ?
                    BOTH_VALID : MISS_BOTH_META;
        } else {
            ret = ALL_MISS;
        }
        return ret;
    };
    //
    enum valid_input_state state = validInputDispatch(pRequest);
    //
    if (mbROIUpdate)
    {
        MY_LOGD_IF(mLogEn > 0, "input validate state:%s", stateMap.find(state)->second.c_str());
        switch (state) {
            case MAIN1_ONLY:
            case MAIN2_ONLY:
            case BOTH_VALID:
            {
#ifdef THIRDPARTY_REPLACEMENT
                /*
                 * Condition:
                 *      1> If only 1 of inputs validate, and wish input as output, then you
                 *         need to set mOSrcImageID accordingly.
                 */
                {
                    if CC_UNLIKELY(state == MAIN2_ONLY) {
                        pRequest->mOBufferMain1_Info.mOSrcImageID = eJoinImageID_MAIN2;
                    } else {
                        pRequest->mOBufferMain1_Info.mOSrcImageID = eJoinImageID_MAIN1;
                    }
                }
                /*
                 * Condition:
                 *      2> If Set crop flag is true, User would like to assign crop region,
                 *         mCoordinate must be set as true;
                 *      3> We assume crop region is 3rd party necessary draw the output buufer,
                 *         Therefore, mOUseSrcImageBuffer needs to assign as false.
                 *         vise versa.
                 */
                // Assign ROI region
                NSCam::MRectF newRoi;
                if (mCoordinate == 0) {
                    if CC_UNLIKELY(state == MAIN2_ONLY) {
                        newRoi = pRequest->mIBufferMain2_Info.mISrcZoomROI;
                    } else {
                        newRoi = pRequest->mIBufferMain1_Info.mISrcZoomROI;
                    }
                    //
                    pRequest->mOBufferMain1_Info.mOUseSrcImageBuffer = true;
                } else {
                    newRoi.p.x = mCoordinate;
                    newRoi.p.y = mCoordinate;
                    newRoi.s.w = pRequest->mIBufferMain1_Info.mISrcZoomROI.s.w;
                    newRoi.s.h = pRequest->mIBufferMain1_Info.mISrcZoomROI.s.h;
                    //
                    pRequest->mOBufferMain1_Info.mOUseSrcImageBuffer = false;
                }
                pRequest->mOBufferMain1_Info.mODstZoomROI = newRoi;
                MY_LOGD_IF((mCoordinate == 0), "(%f,%f),(%f,%f)", newRoi.p.x, newRoi.p.y,
                                                                  newRoi.s.w, newRoi.s.h);
                MY_LOGD_IF((mCoordinate != 0),"Crop Region is Assigned toward (%f, %f)",
                                                                newRoi.p.x, newRoi.p.y);
#endif
            }
            break;
            default:
            /*do nothing*/
            break;
        }
    }
    else
    {
        switch (state) {
        case MAIN1_ONLY:
            ret = (true == imgCpy2Target(pRequest, pRequest->mIBufferMain1->acquire(),
                          pRequest->mOBufferMain1->acquire())) ? OK : UNKNOWN_ERROR;
            break;
        case MAIN2_ONLY:
            ret = (true == imgCpy2Target(pRequest, pRequest->mIBufferMain2->acquire(),
                          pRequest->mOBufferMain1->acquire())) ? OK : UNKNOWN_ERROR;
            break;
        case BOTH_VALID:
            if (GET_ENTRY_ARRAY(pRequest->mIMetadataHal1->acquire(),
                                MTK_STEREO_SYNC2A_MASTER_SLAVE, camInx, 2)) {
                MY_LOGW("Failed to get meta tag: %#09x(%s)""-------------InMeta1 Dump +",
                        MTK_STEREO_SYNC2A_MASTER_SLAVE, "MTK_STEREO_SYNC2A_MASTER_SLAVE");
                pRequest->mIMetadataHal1->acquire()->dump();
                MY_LOGW("-------------InMeta1 Dump -");
            } else if (GET_ENTRY_ARRAY(pRequest->mIMetadataHal2->acquire(),
                                MTK_STEREO_SYNC2A_MASTER_SLAVE, camInx, 2)) {
                MY_LOGW("Failed to get meta tag: %#09x(%s)""-------------InMeta2 Dump +",
                        MTK_STEREO_SYNC2A_MASTER_SLAVE, "MTK_STEREO_SYNC2A_MASTER_SLAVE");
                pRequest->mIMetadataHal1->acquire()->dump();
                MY_LOGW("-------------InMeta2 Dump -");
            } else {
                MY_LOGE("Not obtain Correct MTK_STEREO_SYNC2A_MASTER_SLAVE tag");
                MY_LOGW("camInx[0]=%d, camInx[1]=%d", camInx[0], camInx[1]);
            }
            //
            if (camInx[0] != 2 || camInx[1] != 2) {
                ret = (imgCpy2Target(pRequest,
                       ((camInx[0] == 0) ?
                        pRequest->mIBufferMain1->acquire(): pRequest->mIBufferMain2->acquire()),
                           pRequest->mOBufferMain1->acquire())) ?  OK : UNKNOWN_ERROR;
            } else {
                MY_LOGE("CAN NOT Get Reasonable MTK_STEREO_SYNC2A_MASTER_SLAVE tag,"
                        "camInx[0]:%d camInx[1]:%d", camInx[0], camInx[1]);
                ret = BAD_VALUE;
            }
            break;
        case MISS_MAIN1_META:
        case MISS_MAIN2_META:
        case MISS_BOTH_META:
            MY_LOGE("Missing  Metadata of corresponding input (%s), Abnormal",
                    stateMap.find(state)->second.c_str());
            ret = UNKNOWN_ERROR;
            break;
        default:
            MY_LOGE("Exception Case");
            ret = BAD_VALUE;
        }
    }
#ifdef THIRDPARTY_REPLACEMENT
    // fill streaming id to output app metadata.
    MINT32 main1_id = -1, main2_id = -1;
    IMetadata::IEntry entry(MTK_MULTI_CAM_STREAMING_ID);
    if(pRequest->mIBufferMain1 != nullptr) {
        main1_id = (MINT32)pRequest->mIBufferMain1_Info.mISensorID;
        entry.push_back(main1_id, Type2Type<MINT32>());
    }
    if(pRequest->mIBufferMain2 != nullptr) {
        main2_id = (MINT32)pRequest->mIBufferMain2_Info.mISensorID;
        entry.push_back(main2_id, Type2Type<MINT32>());
    }
    if(!entry.isEmpty()) {
        auto metadata = pRequest->mOMetadataApp->acquire();
        if(metadata != nullptr)
            metadata->update(entry.tag(), entry);
    }
#endif
    ////
    if (mbDumpBuffer) {
        switch (state) {
        case MAIN1_ONLY:
            save2file(pRequest->mIBufferMain1->acquire(), 1, pRequest->mIMetadataHal1->acquire());
            break;
        case MAIN2_ONLY:
            save2file(pRequest->mIBufferMain2->acquire(), 2, pRequest->mIMetadataHal2->acquire());
            break;
        case BOTH_VALID:
            save2file(pRequest->mIBufferMain1->acquire(), 1, pRequest->mIMetadataHal1->acquire());
            save2file(pRequest->mIBufferMain2->acquire(), 2, pRequest->mIMetadataHal2->acquire());
            save2file(pRequest->mOBufferMain1->acquire(), 3, pRequest->mOMetadataHal->acquire());
            break;
        default:
            MY_LOGE("DumpBuffer Exception Case (%d)", state);
            ret = UNKNOWN_ERROR;
        }
    }

    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
bool S_FOVA_Plugin::imgCpy2Target( RequestPtr const  pRequest,
                    IImageBuffer* pSrc1,/* IImageBuffer* pSrc2,*/ IImageBuffer* target)
{
    unsigned char *outBuf = nullptr, *src1Buf = nullptr;
    size_t outBufSize    = 0, planeCnt    = 0;
    size_t src1BufSize   = 0, src1Strides = 0;
    size_t outBufStrides = 0;
    IMetadata* pMeta = nullptr;

    FUNCTION_IN;
    MY_LOGD("src1=%p w:%04d-h:%04d\t ; out=w:%04d-h:%04d,"
            "src1_bufSize:%zu, src1_stride:%zu",
            pSrc1, pSrc1->getImgSize().w  , pSrc1->getImgSize().h,
            target->getImgSize().w        , target->getImgSize().h,
            pSrc1->getBufSizeInBytes(0)   , pSrc1->getBufStridesInBytes(0));
    //
    if (pSrc1->getImgSize().w == target->getImgSize().w &&
        pSrc1->getImgSize().h == target->getImgSize().h)
    {
        planeCnt = pSrc1->getPlaneCount();
        for (size_t i = 0; i < planeCnt; i++) {     //Copy Main1 to Output
            src1Buf = (unsigned char*)pSrc1->getBufVA(i);
            outBuf  = (unsigned char*)target->getBufVA(i);

            src1Strides   = pSrc1->getBufStridesInBytes(i);
            outBufStrides = target->getBufStridesInBytes(i);

            src1BufSize   = pSrc1->getBufSizeInBytes(i);
            outBufSize    = target->getBufSizeInBytes(i);

            if (src1Strides == outBufStrides)
                memcpy(outBuf, src1Buf, std::min(src1BufSize, outBufSize));
            else {
                unsigned stride = std::min(src1Strides, outBufStrides);
                unsigned height = std::min(src1BufSize/src1Strides, outBufSize/outBufStrides);
                for( unsigned y = 0; y < height; ++y )
                    memcpy(outBuf+y*outBufStrides, src1Buf+y*src1Strides, stride);
            }
        }
    } else {
        if (mbDumpBuffer) {
            sMDP_Config cfg = {
                .pDpStream  = mpDpBlitStream,
                .pSrcBuffer = pSrc1,
                .pDstBuffer = target,
                .rotAngle   = 0,
            };// Resize src to output size.
            excuteMDP(cfg);
        } else
            MY_LOGE("mbDumpBuffer is not set, not allow to use MDP");
    }
    //
    if (mbDumpBuffer) {
        target->syncCache();
        pMeta = pRequest->mIMetadataHal1->acquire();
        target->syncCache();
        save2file(target, 4, pMeta);
    }

    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
void S_FOVA_Plugin::save2file(IImageBuffer* img, int idx, IMetadata *meta)
{
    FUNCTION_IN;

    IImageBuffer* srcImg = img; // create src image
    const size_t PATH_SIZE = 1024;
    char writepath[PATH_SIZE] = {0}, filename[PATH_SIZE] = {0};
    char file_prefix[5][50] = {"Ori_Main1", "Ori_Main2", "Output"};

    switch (idx) {
        case 1://Main1
        case 2://Main2
        case 3://output
            extract(&mDumpHint_Main1, meta);
            extract(&mDumpHint_Main1, srcImg);
            break;
    }

    switch (idx) {
    case 1://Main1
        snprintf(filename, PATH_SIZE, "%s",file_prefix[0]);
        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main1,
                        TuningUtils::YUV_PORT_UNDEFINED, filename);
        break;
    case 2://Main2
        snprintf(filename, PATH_SIZE, "%s",file_prefix[1]);
        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main1,
                        TuningUtils::YUV_PORT_UNDEFINED, filename);
        break;
    case 3://output
        snprintf(filename, PATH_SIZE, "%s",file_prefix[3]);
        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main1,
                        TuningUtils::YUV_PORT_UNDEFINED, filename);
        break;
    }
    srcImg->saveToFile(writepath);

}
/******************************************************************************
 *
 ******************************************************************************/
REGISTER_PLUGIN_PROVIDER(Join, S_FOVA_Plugin);
// };//namespace NSPipelinePlugin
// };//namespace NSCam
