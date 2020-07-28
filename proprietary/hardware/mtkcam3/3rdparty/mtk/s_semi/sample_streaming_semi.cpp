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
using namespace NSCam::Utils::Format;
using NSCam::MSize;
using NSCam::MERROR;
using NSCam::IImageBuffer;
using NSCam::IMetadata;
using NSCam::Utils::makePath;
using VSDOF::util::sMDP_Config;

#ifdef LOG_TAG
#undef LOG_TAG
#endif // LOG_TAG
#define LOG_TAG "MtkCam/Plugin_S_SEMI"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

#include <log/log.h>
#include <android/log.h>

#define ROTATED_DEPTH_NAME "ROTATED_DEPTH_NAME"
#define SEMI_FLOW //Vendor Need to Replace below section for real implement

#define MY_LOGI(fmt, arg...)  ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)  ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)  ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)  ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD_IF(cond, ...)   do { if ( (cond) ) { MY_LOGD(__VA_ARGS__);   } } while(0)
#define LOGD_IF(cond, ...)      do { if ( (cond) ) { CAM_ULOGMD(__VA_ARGS__); } } while(0)
#define LOGI_IF(cond, ...)      do { if ( (cond) ) { CAM_ULOGMI(__VA_ARGS__); } } while(0)

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_TPI_PLUGIN);

template <class T>
inline bool
tryGetMetadata(IMetadata const *pMetadata, MUINT32 tag, T& rVal)
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
//
template <typename T>
inline bool
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGE("pMetadata == NULL");
        return false;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
    return true;
}

// namespace NSCam {
// namespace NSPipelinePlugin {
class S_SEMI_Plugin : public JoinPlugin::IProvider
{
public:
    typedef JoinPlugin::Property Property;
    typedef JoinPlugin::Selection Selection;
    typedef JoinPlugin::Request::Ptr RequestPtr;
    typedef JoinPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    S_SEMI_Plugin();
    ~S_SEMI_Plugin();
    const Property& property();
    void set(MINT32 openID1, MINT32 openID2);
    void init();
    void uninit();
    MERROR negotiate(Selection& sel);
    MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback);
    void abort(std::vector<RequestPtr> &pRequests);

private:
    MERROR getConfigSetting(Selection &sel);
    MERROR getP2Setting(Selection &sel);

private:
    int    mOpenID1 = 0;
    int    mOpenID2 = 0;
    static constexpr uint8_t THIRDPARY_BOKEH_MAX_FD_FACES_NUM = 15; //Max number of faces can be detected based on MTK FD spec

private:
    struct TpBkhInFdInfo
    {
        MINT32 fdRotationAngle = 0;            // FD rotation information
        MPoint fdRoiTopLeft    = MPoint(0, 0); // FD ROI top left point
        MPoint fdRoiBotRight   = MPoint(0, 0); // FD ROI bottom right point
    };
    //
    struct TpBkhInAFInfo
    {
        MUINT8 afMode        = 0;
        MUINT8 afStatus      = 0;              // af Status(Follow google af state defination, 0~6)
        MPoint afRoiTopLeft  = MPoint(0, 0);   // FD ROI top left point
        MPoint afRoiBotRight = MPoint(0, 0);   // FD ROI bottom right point
    };
    //
    struct TpBkhInBuffer
    {
        IImageBuffer* pInMain1Yuv       = nullptr; // Main1 YUV(YV21)       Image buffer
        IImageBuffer* pInDepth          = nullptr; // Depthmap(Y8)          Image buffer
        IImageBuffer* pInRotatedDepth   = nullptr; // RotatedDepthmap(Y8)   Image buffer
    };
    //
    struct TpBkhOutBuffer
    {
        IImageBuffer* pOutBokehImg     = nullptr; // Bokeh Image for Preview
        IImageBuffer* pOutBokehPreview = nullptr; // Bokeh Image for PreviewCallback
    };
    //
    struct TpBkhInParam
    {
        MINT32        prvBokehLevel                            =  0;    // Bokeh level from AP
        TpBkhInAFInfo afInfo                                   = {0};
        MINT32        faceCounts                               =  0;
                                                        // Face detection result, number of faces
        TpBkhInFdInfo fdInfo[THIRDPARY_BOKEH_MAX_FD_FACES_NUM] = {{0}};
                                                        // Face detection result container
        MFLOAT        inDistance                               =  0.0f;    // Distance Info
        MINT8         inDAC                                    =  0;
        MINT8         inGravity                                =  0;
    };
    /**
     * @brief Config Vendor lib IO buffers
     * @param [in] input vendorbuffer
     * @param [in] input vendorMeta
     * @param [in] input data for 3rd vendor lib to be config
     * @param [in] output data for 3rd vendor lib to be used
     * @param [in] output data for 3rd vendor lib to be config
     * @return
     * - TRUE indicates success.
     * - FALSE indicates failure.
     */
    bool prepareTpBkhInOutData(
            RequestPtr const  pRequest,
            TpBkhInBuffer*    inBuffers,
            TpBkhOutBuffer*   ouTbuffers,
            TpBkhInParam*     inParam);

    /**
     * @brief process third party vendor lib
     * @param [in] input buffers for vendor lib
     * @param [in] output buffers for vendor lib
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    bool processTpBkh(
            RequestPtr const  pRequest,
            TpBkhInBuffer       inBuf,
            TpBkhOutBuffer      outBuf,
            const TpBkhInParam& inParam);
    /**
     * @brief an example to process 2 src image
     * @param [in] source1 buffer
     * @param [in] source2 buffer
     * @param [in] target buffer
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    bool imgProcess(
            RequestPtr const  pRequest,
            IImageBuffer* pSrc1,
            IImageBuffer* pSrc2,
            IImageBuffer* target);

    /**
     * @brief an example to rotate buffer
     * @param [in] source buffer
     * @param [in] source size
     * @param [in] output buffer
     * @param [in] rotation direction
     * @return
     *  - true roate success
     *  - false roate failure
     */
    bool rotateBuffer(
            const MUINT8 *src,
            const MSize src_size,
            MUINT8 *dst,
            ENUM_ROTATION rotation);

    /**
     * @Debug Purpose to confirm output buffer
     * @param [in] target buffer to dump
     * @param [in] dump scenario. 1:Main1 Origin, 2:Main2 Origin
     *                            3:Resize Main2, 4:Merge Main1&Main2
     * @param [in] corresponding metatdata, i.e. main1 or main2
     */
    void save2file(IImageBuffer* img, int idx, IMetadata *meta);

    sp<IImageBuffer> createWorkingBuffer(
                        const char* name,
                        MINT32 format,
                        const MSize& size);

    void releaseWorkingBuffer(
                const char* name,
                sp<IImageBuffer>& imgPtr);

private:
    bool   mbTpBokehMetaLogEn  = false;
    bool   mbShowMark          = false;
    bool   mbDumpBuffer        = false;
    bool   mSetImgSize         = false;

    MRect  mActiveCropSift;
    MINT32 mTpBokehFdTolerence = 0;
    MFLOAT DISTANCE_THREADHOLD = 0.0f;

    sp<IImageBuffer>    mRotatedDepthImgPtr = nullptr;
    ENUM_ROTATION       mTarRotate          = eRotate_0;

    TuningUtils::FILE_DUMP_NAMING_HINT mDumpHint_Main1;
};

S_SEMI_Plugin::S_SEMI_Plugin()
{
    CAM_ULOGM_APILIFE();
    mTpBokehFdTolerence = ::property_get_int64("vendor.debug.vsdof.semi.fd.tolerence",
                                                700000000);
    mbTpBokehMetaLogEn  = ::property_get_bool("vendor.debug.vsdof.semi.MetaLogEn"  , 1);
    mbShowMark          = ::property_get_bool("vendor.debug.vsdof.semi.showMark"   , 1);
    mbDumpBuffer        = ::property_get_bool("vendor.debug.vsdof.semi.dumpbuffer" , 0);
    mSetImgSize         = ::property_get_bool("vendor.debug.vsdof.semi.specifySize", 0);

    DISTANCE_THREADHOLD = ::property_get_int32("vendor.debug.vsdof.semi.DistTooFarTh", 255);

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);

    if (mbDumpBuffer)
        extract_by_SensorOpenId(&mDumpHint_Main1, main1Idx);

    CAM_ULOGMD("create S_SEMI plugin: Tolerence:%d, DumpBuffer:%d",
              mTpBokehFdTolerence, mbDumpBuffer);

    sp<IMetadataProvider> pMetadataProvider =
                                    NSMetadataProviderManager::valueFor(main1Idx);
    if( ! pMetadataProvider.get() ) {
        CAM_ULOGME("MetadataProvider is NULL");
        return;
    }

    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    //Mapping to active domain by ratio (preview size)
    if (tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION,
                              mActiveCropSift))
    {
        CropUtil::cropRectByImageRatio(mActiveCropSift,
                                       StereoSettingProvider::imageRatio());

        CAM_ULOGMD("mActiveCropSift %d %d %d %d", mActiveCropSift.p.x, mActiveCropSift.p.y,
                 mActiveCropSift.s.w, mActiveCropSift.s.h);
    } else {
        CAM_ULOGME("tryGetMetadata mActieArrayCrop is NULL");
        return;
    }

}

S_SEMI_Plugin::~S_SEMI_Plugin()
{
    CAM_ULOGMD("destroy  S_SEMI plugin");
}

void S_SEMI_Plugin::set(MINT32 openID1, MINT32 openID2)
{
    CAM_ULOGMD("set openID1:%d openID2:%d", openID1, openID2);
    mOpenID1 = openID1;
    mOpenID2 = openID2;
}

const S_SEMI_Plugin::Property& S_SEMI_Plugin::property()
{
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName     = "MTK TK_3rdPartyPlugin";
        prop.mFeatures = MTK_FEATURE_DUAL_HWDEPTH;//@ mtk_platform_metadata_tag.h
        //prop.mInPlace = MTRUE;
        //prop.mFaceData = eFD_Current;
        //prop.mPosition = 0;
        inited = true;
    }
    return prop;
}

MERROR S_SEMI_Plugin::negotiate(Selection &sel)
{
    MERROR ret = OK;
    //
    switch (sel.mSelStage) {
    case eSelStage_CFG:
        ret = getConfigSetting(sel);
        break;
    case eSelStage_P2:
        ret = getP2Setting(sel);
        break;
    default:
        mbTpBokehMetaLogEn = true;
        break;
    }
    //
    LOGD_IF(mbTpBokehMetaLogEn > 0, "mSelStage input (%d)", sel.mSelStage);

    return ret;
}

MERROR S_SEMI_Plugin::getConfigSetting(Selection &sel)

{
    MERROR ret = OK;

    sel.mCfgOrder     = 1;//1st Plugin
    sel.mCfgJoinEntry = eJoinEntry_S_YUV;
    sel.mCfgEnableFD  = MTRUE;
    sel.mCfgRun       = true;

    enum EImageFormat fmt = (property_get_bool("vendor.debug.tpi.s.fb.nv21", 0)) ?
                              eImgFmt_YV12 : eImgFmt_NV21;
    //
    sel.mIBufferMain1.setRequired(true)
                        .addAcceptedFormat(fmt);

    sel.mIBufferDepth.setRequired(true)
                        .addAcceptedFormat(fmt);

    sel.mOBufferDepth.setRequired(true)
                        .addAcceptedFormat(fmt);

    if (mSetImgSize) {
        sel.mCfgInfo.mMaxOutSize = StereoSizeProvider::getInstance()->getPreviewSize();
        //
        sel.mIBufferMain1.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
        sel.mIBufferDepth.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
        sel.mOBufferDepth.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
    } else {
        sel.mIBufferMain1.addAcceptedSize(eImgSize_Full);
        sel.mIBufferDepth.addAcceptedSize(eImgSize_Full);
        sel.mOBufferDepth.addAcceptedSize(eImgSize_Full);
    }
    sel.mIMetadataApp.setRequired(true);
    sel.mIMetadataHal1.setRequired(true);
    sel.mIMetadataDynamic1.setRequired(true);
    sel.mOMetadataApp.setRequired(true);

    LOGD_IF(sel.mCfgRun == false, "S_SEMI_Plugin is not config as run state(%d)",
               sel.mCfgRun);

    return ret;
}

sp<IImageBuffer> S_SEMI_Plugin::createWorkingBuffer(
                                    const char* name, MINT32 format, const MSize& size)
{
    MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(format);
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    size_t bufStridesInBytes[3] = {0};

    for (MUINT32 i = 0; i < plane; i++) {
        bufStridesInBytes[i] = queryPlaneWidthInPixels(format, i, size.w)*
                               queryPlaneBitsPerPixel(format, i)/8;
    }

    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
                            static_cast<EImageFormat>(format), size, bufStridesInBytes,
                            bufBoundaryInBytes, plane);

    sp<IImageBufferHeap> pHeap = IIonImageBufferHeap::create(name, imgParam);
    if (pHeap == nullptr) {
        MY_LOGE("Create heap failed");
        return nullptr;
    }

    sp<IImageBuffer> imgPtr = pHeap->createImageBuffer();
    if (imgPtr == nullptr) {
        MY_LOGE("Create image buffer failed");
        return nullptr;
    }

    const MUINT usage = ( GRALLOC_USAGE_SW_READ_OFTEN  | GRALLOC_USAGE_SW_WRITE_OFTEN |
                          GRALLOC_USAGE_HW_CAMERA_READ | GRALLOC_USAGE_HW_CAMERA_WRITE );
    if (!(imgPtr->lockBuf(name, usage))) {
        MY_LOGE("Lock image buffer failed");
        return nullptr;
    }
    MY_LOGI("Create woring buffer, addr:%p, name:%s, size:%dx%d, format:%s",
        imgPtr.get(), name, size.w, size.h, queryImageFormatName(format).c_str());
    return imgPtr;
}

void S_SEMI_Plugin::releaseWorkingBuffer(const char* name, sp<IImageBuffer>& imgPtr)
{
    if (imgPtr != nullptr) {
        const MSize size = imgPtr->getImgSize();
        const MINT format = imgPtr->getImgFormat();
        MY_LOGI("Release woring buffer, addr:%p, name:%s, size:%dx%d, format:%s",
                imgPtr.get(), name, size.w, size.h, queryImageFormatName(format).c_str());
        imgPtr->unlockBuf(name);
        imgPtr.clear();
    }
}

void S_SEMI_Plugin::init()
{
    CAM_ULOGM_APILIFE();
    CAM_ULOGMD("init S_SEMI plugin");

    switch (StereoSettingProvider::getModuleRotation()) {
    case eRotate_90:  mTarRotate = eRotate_270; break;
    case eRotate_270: mTarRotate = eRotate_90;  break;
    case eRotate_180: mTarRotate = eRotate_180; break;
    case eRotate_0:
    default:
        break;
    }

    if (eRotate_0 != mTarRotate) {
        MSize targetSize = StereoSizeProvider::getInstance()->getBufferSize(E_DEPTH_MAP);
        if (mTarRotate & 0x02)//Need to Rotate
            std::swap(targetSize.w, targetSize.h);
        //
        mRotatedDepthImgPtr = createWorkingBuffer(ROTATED_DEPTH_NAME, eImgFmt_Y8, targetSize);
        if (mRotatedDepthImgPtr == nullptr) {
            MY_LOGI("Failed to create rotatedDepthImgPtr!!");
        }
        MY_LOGI("Rotate Buffer with %d , Rotate_target WxH(%03dx%03d)",
                mTarRotate, targetSize.w, targetSize.h);
    }
}

void S_SEMI_Plugin::uninit()
{
    CAM_ULOGM_APILIFE();
    CAM_ULOGMD("uninit S_SEMI plugin");
    if (mRotatedDepthImgPtr != nullptr)
        releaseWorkingBuffer(ROTATED_DEPTH_NAME, mRotatedDepthImgPtr);
}

void S_SEMI_Plugin::abort(std::vector<RequestPtr> &pRequests)
{
    (void)pRequests;
    MY_LOGD("uninit S_SEMI plugin");
    if (mRotatedDepthImgPtr != nullptr)
        releaseWorkingBuffer(ROTATED_DEPTH_NAME, mRotatedDepthImgPtr);
};
/******************************************************************************
 *
 ******************************************************************************/

MERROR S_SEMI_Plugin::process(RequestPtr pRequest, RequestCallbackPtr /*pCallback*/)
{
    CAM_ULOGM_APILIFE();
    MERROR ret = OK;

    TpBkhInBuffer  inBufs;
    TpBkhOutBuffer outBufs;
    TpBkhInParam   inParams = {0};

    if (pRequest->mIBufferMain1.get() != nullptr &&
        pRequest->mIBufferDepth.get() != nullptr)
    {
        if (mbDumpBuffer) {
            IMetadata* pInHalMeta;
            pInHalMeta = pRequest->mIMetadataHal1->acquire();
            save2file(pRequest->mIBufferMain1->acquire(), 1, pInHalMeta);
        }

        prepareTpBkhInOutData(pRequest, &inBufs, &outBufs, &inParams);

        if (!processTpBkh(pRequest, inBufs, outBufs, inParams)) {
            MY_LOGE("processTpBkh Return Error");
            ret = UNKNOWN_ERROR;
        }

        if (mRotatedDepthImgPtr != nullptr && mbDumpBuffer && (eRotate_0 != mTarRotate))
        {
            IMetadata* pInHalMeta;
            pInHalMeta = pRequest->mIMetadataHal1->acquire();
            save2file(mRotatedDepthImgPtr.get(), 2, pInHalMeta);
        }
    }
    else
    {
        MY_LOGE("input buffer Error! Main1_ptr:%p,  Depth_ptr:%p",
                pRequest->mIBufferMain1.get(), pRequest->mIBufferDepth.get());

        ret = UNKNOWN_ERROR;
    }

    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/

#ifdef SEMI_FLOW
bool S_SEMI_Plugin::imgProcess( RequestPtr const  pRequest,
                    IImageBuffer* pSrc1, IImageBuffer* pSrc2, IImageBuffer* target)
{
    unsigned char *outBuf = nullptr, *src1Buf = nullptr;
    size_t outBufSize    = 0, planeCnt    = 0, outBuf_pos, strides_diff = 0;
    size_t src1BufSize   = 0, src2BufSize = 0, src1Strides = 0, src2Strides = 0;
    size_t outBufStrides = 0;
    IMetadata* pMeta = nullptr;

    CAM_ULOGM_FUNCLIFE();
    MY_LOGD("src1=%p w:%04d-h:%04d\t ; src2=%p w:%04d-h:%04d\t ; out=w:%04d-h:%04d,"
            "src1_bufSize:%zu, src2_bufSize:%zu, src1_stride:%zu, src2_stride:%zu",
            pSrc1, pSrc1->getImgSize().w, pSrc1->getImgSize().h,
            pSrc2, pSrc2->getImgSize().w, pSrc2->getImgSize().h,
            target->getImgSize().w        , target->getImgSize().h     ,
            pSrc1->getBufSizeInBytes(0)   , pSrc2->getBufSizeInBytes(0),
            pSrc1->getBufStridesInBytes(0), pSrc2->getBufStridesInBytes(0));

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
    //
    if (mbShowMark == true) {
        for (size_t i = 0; i < planeCnt; i++) {
            src1Buf = (unsigned char*)pSrc1->getBufVA(i);
            outBuf  = (unsigned char*)target->getBufVA(i);

            src1BufSize = pSrc1->getBufSizeInBytes(i);
            //         src2BufSize = src1BufSize/16;
            src2BufSize = src1BufSize/64;
            outBufSize  = target->getBufSizeInBytes(i);

            src1Strides = pSrc1->getBufStridesInBytes(i);
            //         src2Strides = src1Strides/4;
            src2Strides = src1Strides/16;

            memcpy(outBuf, src1Buf, src1BufSize);
            if (src2Strides > src1Strides) {
                MY_LOGE("Main2 strides(%zu) > Main1 strides(%zu)",
                        src2Strides, src1Strides);
                return false;
            }
            strides_diff = src1Strides - src2Strides;

            outBuf_pos = 0;
            for (size_t j = 0; j < src2BufSize; j++) {
                if (((j % src2Strides)==0) && (j != 0))
                    outBuf_pos += strides_diff;

                if (outBuf_pos < src1BufSize)
                    outBuf[outBuf_pos] = 0;

                outBuf_pos++;
            }
        }
    }

    if (mbShowMark|| mbDumpBuffer)
        target->syncCache();

    if (mbDumpBuffer) {
        pMeta = pRequest->mIMetadataHal1->acquire();
        save2file(target, 4, pMeta);
    }

    return true;
}
#endif
/******************************************************************************
 *
 ******************************************************************************/

bool S_SEMI_Plugin::processTpBkh(RequestPtr const  pRequest,
            TpBkhInBuffer inBuf, TpBkhOutBuffer outBuf, const TpBkhInParam& inParam)
{
    // ---------------------------
    // Get input parameter example
    // ---------------------------

    // Get FD info
    for (MUINT8 i = 0 ; i < inParam.faceCounts && mbTpBokehMetaLogEn == true; i++)
    {
        MPoint fdRoiTopLeft    = inParam.fdInfo[i].fdRoiTopLeft;    // FD ROI top left point
        MPoint fdRoiBotRight   = inParam.fdInfo[i].fdRoiBotRight;   // FD ROI bottom right point
        MINT32 fdRotationAngle = inParam.fdInfo[i].fdRotationAngle; // FD rotation information
        MY_LOGD("faceId:%d, fdRoiTopLeft(%d.%d), fdRoiBotRight(%d.%d), fdRotationAngle(%d)",
                i, fdRoiTopLeft.x, fdRoiTopLeft.y, fdRoiBotRight.x, fdRoiBotRight.y,
                fdRotationAngle);
    }

    // --------------------------------
    // Add IN AND OUT vendor code here
    // and return process result
    // --------------------------------
    // ======================
    // Need to remove below section for real implementation.
#ifdef SEMI_FLOW
    return (true == imgProcess(pRequest, inBuf.pInMain1Yuv, inBuf.pInDepth,
                               outBuf.pOutBokehImg)) ? true : false;
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
bool S_SEMI_Plugin::prepareTpBkhInOutData(RequestPtr const  pRequest,
            TpBkhInBuffer* inBuffers, TpBkhOutBuffer* outBuffers, TpBkhInParam* inParam)

{
    // =============================================================
    //             Prepare In/Out Img Buffer
    // =============================================================
    CAM_ULOGM_FUNCLIFE();

    std::bitset<4> decision;
    // Input Buffer
    if (pRequest->mIBufferMain1 != nullptr) {
        inBuffers->pInMain1Yuv = pRequest->mIBufferMain1->acquire();
        decision.set(0);
    }
    // Input Depth
    if (pRequest->mIBufferDepth != nullptr) {
        inBuffers->pInDepth = pRequest->mIBufferDepth->acquire();
        decision.set(1);
    }
    // Input Depth Rotated Img
    if (mRotatedDepthImgPtr != nullptr && eRotate_0 != mTarRotate) {
        inBuffers->pInRotatedDepth = mRotatedDepthImgPtr.get();
        if (inBuffers->pInRotatedDepth) {//Rotate depthmap
            if (mTarRotate & 0x02)
            {
                if (inBuffers->pInDepth->getImgSize().w ==
                    inBuffers->pInRotatedDepth->getImgSize().h)
                {
                    if (!rotateBuffer((MUINT8*)inBuffers->pInDepth->getBufVA(0),
                         inBuffers->pInDepth->getImgSize(),
                         (MUINT8*)inBuffers->pInRotatedDepth->getBufVA(0), mTarRotate))
                    {
                        MY_LOGE("Missing/Bad argument for rotateBuffer");
                    }
                }
            }
            MY_LOGD_IF(mTarRotate != eRotate_0,
                       "Rotation depthmap, bufAddress(%p), targeRotation(%d)"
                       "src_wxh:%04dx%04d, dst_wxh:%04dx%04d",
                       inBuffers->pInRotatedDepth, mTarRotate,
                       inBuffers->pInDepth->getImgSize().w,
                       inBuffers->pInDepth->getImgSize().h,
                       inBuffers->pInRotatedDepth->getImgSize().w,
                       inBuffers->pInRotatedDepth->getImgSize().h);
        }
    }
    //Output Processed Buffer
    if (pRequest->mOBufferMain1 != nullptr) {
        outBuffers->pOutBokehImg = pRequest->mOBufferMain1->acquire();
        decision.set(2);
    }
    if (0x07 != decision.to_ulong()) {
        MY_LOGE("Input Buffer Gather Failed, idex(%#lx)", decision.to_ulong());
        return false;
    }
    // =============================================================
    //             Prepare Metadata Pointer
    // =============================================================
    IMetadata *pIMetadata_Dyn1 = nullptr, *pIMetadata_App    = nullptr;
    IMetadata *pIMetadata_Hal1 = nullptr, *pIMetadata_outApp = nullptr;
    decision = 0;

    if (pRequest->mIMetadataDynamic1 != nullptr) {
        pIMetadata_Dyn1 = pRequest->mIMetadataDynamic1->acquire();
        decision.set(0);
    }
    if (pRequest->mIMetadataApp != nullptr) {
        pIMetadata_App = pRequest->mIMetadataApp->acquire();
        decision.set(1);
    }
    if (pRequest->mIMetadataHal1 != nullptr) {
        pIMetadata_Hal1 = pRequest->mIMetadataHal1->acquire();
        decision.set(2);
    }
    if (pRequest->mOMetadataApp != nullptr) {
        pIMetadata_outApp = pRequest->mOMetadataApp->acquire();
        decision.set(3);
    }
    if (pIMetadata_Dyn1   != nullptr && pIMetadata_Hal1 != nullptr &&
        pIMetadata_outApp != nullptr && pIMetadata_App  != nullptr)
        MY_LOGD("Dyn1Meta cnt:%03d, AppMeta cnt:%03d, Hal1Meta cnt:%03d"
                "outAppMeta cnt:%03d",
                pIMetadata_Dyn1->count(), pIMetadata_App->count(),
                pIMetadata_Hal1->count(), pIMetadata_outApp->count());
    else
        MY_LOGE("Getd Mete Ptr Error (%#lx)", decision.to_ulong());

    // =============================================================
    //             Prepare Input Parameter (Bokeh level & AF roi)
    // =============================================================
    MINT32 afTopLeftX     = 0, afTopLeftY     = 0;
    MINT32 afBottomRightX = 0, afBottomRightY = 0;
    MINT32 afROINum       = 0, afROITagStart  = 0;
    MINT32 centerPointX   = 0, centerPointY   = 0;
    // Fill in bokeh level
    if (!tryGetMetadata<MINT32>(pIMetadata_Dyn1, MTK_STEREO_FEATURE_DOF_LEVEL,
                                inParam->prvBokehLevel))
        MY_LOGE("Get Meta: MTK_STEREO_FEATURE_DOF_LEVEL Error");
    // Fill in afInfo
    if (!tryGetMetadata<MUINT8>(pIMetadata_App, MTK_CONTROL_AF_MODE,
                                inParam->afInfo.afMode))
        MY_LOGE("Get Meta :MTK_CONTROL_AF_MODE Error");

    if (!tryGetMetadata<MUINT8>(pIMetadata_Dyn1, MTK_CONTROL_AF_STATE,
                                inParam->afInfo.afStatus))
         MY_LOGE("Get Meta :MTK_CONTROL_AF_STATE Error");

    if (pIMetadata_Dyn1 != nullptr)
    {
        IMetadata::IEntry entry = pIMetadata_Dyn1->entryFor(MTK_3A_FEATURE_AF_ROI);
        if (!entry.isEmpty())
        {
            afROINum         = entry.itemAt(1, Type2Type<MINT32>());
            afROITagStart    = (afROINum-1)*5+2; //Special rule to map to 3A metadata tag order
            afTopLeftX       = entry.itemAt(afROITagStart,   Type2Type<MINT32>());
            afTopLeftY       = entry.itemAt(afROITagStart+1, Type2Type<MINT32>());
            afBottomRightX   = entry.itemAt(afROITagStart+2, Type2Type<MINT32>());
            afBottomRightY   = entry.itemAt(afROITagStart+3, Type2Type<MINT32>());
            MY_LOGD_IF(mbTpBokehMetaLogEn,"Get Meta_Dynamic1 done");
        }
        else
        {
            MY_LOGE("%s:Get AF roi fail", __func__);
        }

        //Map source buffer(sensor domain) to 4:3 or 16:9 buffer to make ratio align
        MINT32 width  = inBuffers->pInMain1Yuv->getImgSize().w;
        MINT32 height = inBuffers->pInMain1Yuv->getImgSize().h;
        centerPointX = (afTopLeftX + afBottomRightX) / 2;
        centerPointY = (afTopLeftY + afBottomRightY) / 2;

        float ratioX = (float)(centerPointX - mActiveCropSift.p.x) /
                                        ((float) mActiveCropSift.s.w);
        float ratioY = (float)(centerPointY - mActiveCropSift.p.y) /
                                        ((float) mActiveCropSift.s.h);
        inParam->afInfo.afRoiTopLeft.x = width * ratioX;
        inParam->afInfo.afRoiTopLeft.y = height * ratioY;
        //MY_LOGD("wh(%d,%d)\t ratio(%lf,%lf)\t cP(%d,%d)",width,
        //        height,ratioX, ratioY, centerPointX,centerPointY);
    } else {
        MY_LOGE("Get pIMetadata_App nullPtr");
    }
    // ===================================================================
    //             Prepare Input paramter (FD information)
    // ===================================================================
    // 1. create IFDContainer instance
    auto fdReader = IFDContainer::createInstance(LOG_TAG,
                                                IFDContainer::eFDContainer_Opt_Read);

    // 2. get timestamp
    MINT64 p1timestamp = 0;
    if (!tryGetMetadata<MINT64>(pIMetadata_Hal1, MTK_P1NODE_FRAME_START_TIMESTAMP,
                                p1timestamp))
        MY_LOGE("Cant get p1 timestamp meta.");

    // 3. query fd info by timestamps, fdData must be return after use
    auto fdData = fdReader->queryLock(p1timestamp - mTpBokehFdTolerence, p1timestamp);

    // 4. fd info sameple
    {
        // fdData is the container of face data,
        if (fdData.size() > 0)
        {
            auto fdChunk = fdData.back();

            if (CC_LIKELY( fdChunk != nullptr ))
            {
                MY_LOGD_IF(mbTpBokehMetaLogEn, "Number_of_faces:%d",
                                                fdChunk->facedata.number_of_faces);
                // Check if exceed the maxium faces count
                MUINT8 facesNum = (fdChunk->facedata.number_of_faces >
                                    THIRDPARY_BOKEH_MAX_FD_FACES_NUM) ?
                                    THIRDPARY_BOKEH_MAX_FD_FACES_NUM:
                                    fdChunk->facedata.number_of_faces;
                inParam->faceCounts = facesNum;
                //
                MRect metacrop;// get crop info, ex: from p1node crop in halmeta
                if (!tryGetMetadata<MRect>(pIMetadata_Hal1, MTK_P1NODE_SENSOR_CROP_REGION, metacrop))
                {
                    MY_LOGW("-------------Hal1 Dump + Failed to get meta tag: %#09x(%s)",
                            MTK_P1NODE_SENSOR_CROP_REGION, "MTK_P1NODE_SENOSR_CROP_REGION");
                    if (pIMetadata_Hal1!= nullptr)
                        pIMetadata_Hal1->dump();
                    MY_LOGW("-------------Hal1 Dump -");
                }
                // face,  15 is the max number of faces
                for (MUINT8 i = 0 ; i < facesNum; i++)
                {
                    //Origianl FD ROI set is from -1000 to 1000, need to map to preview size domain
                    MINT32 width  = inBuffers->pInMain1Yuv->getImgSize().w;
                    MINT32 height = inBuffers->pInMain1Yuv->getImgSize().h;
                    MPoint TopLeft(0,0), BotRight(0,0);
                    if (metacrop.s.w != 0 && metacrop.s.h != 0) {
                        TopLeft.x  = (fdChunk->facedata.faces[i].rect[0] - metacrop.p.x) * width / metacrop.s.w;
                        TopLeft.y  = (fdChunk->facedata.faces[i].rect[1] - metacrop.p.y) * height/ metacrop.s.h;
                        BotRight.x = (fdChunk->facedata.faces[i].rect[2] - metacrop.p.x) * width / metacrop.s.w;
                        BotRight.y = (fdChunk->facedata.faces[i].rect[3] - metacrop.p.y) * height/ metacrop.s.h;
                    }
                    MINT32 rotationAngle = fdChunk->facedata.fld_rip[i];
                    // Map to ArcSoft face rotate rule
                    if(rotationAngle < 0) {
                        rotationAngle = abs(rotationAngle);
                    } else {
                        rotationAngle = 360 - rotationAngle;
                    }
                    MY_LOGD_IF(mbTpBokehMetaLogEn, "FD[%d] ROI %d %d %d %d,"
                                "rotationDeg: %d", i, TopLeft.x, TopLeft.y,
                                BotRight.x, BotRight.y, rotationAngle);

                    inParam->fdInfo[i].fdRoiTopLeft.x  = TopLeft.x;
                    inParam->fdInfo[i].fdRoiTopLeft.y  = TopLeft.y;
                    inParam->fdInfo[i].fdRoiBotRight.x = BotRight.x;
                    inParam->fdInfo[i].fdRoiBotRight.y = BotRight.y;
                    inParam->fdInfo[i].fdRotationAngle = rotationAngle;
                }
            }
            else
            {
                MY_LOGW("fdData is not found");
            }
        } else
            MY_LOGD_IF(mbTpBokehMetaLogEn,"FD size:%lu", fdData.size());
    }
    // 4. fdData must be return after use
    fdReader->queryUnlock(fdData);
    // 5. Distance Warning
    if (!tryGetMetadata<MFLOAT>(pIMetadata_outApp, MTK_STEREO_FEATURE_RESULT_DISTANCE,
                                inParam->inDistance))
    {
        MY_LOGW("Failed to get meta tag: %#09x(%s)"
                "-------------outAPP Dump +",
                MTK_STEREO_FEATURE_RESULT_DISTANCE,
                "MTK_STEREO_FEATURE_RESULT_DISTANCE");
        if (pIMetadata_outApp != nullptr)
            pIMetadata_outApp->dump();
        MY_LOGW("-------------outAPP Dump -");

        if (!trySetMetadata<MINT32>(pIMetadata_outApp, MTK_VSDOF_FEATURE_WARNING, 0))
            MY_LOGE("Write MTK_VSDOF_FEATURE_WARNING tag fail!!!");
    }
    else
    {
        MUINT32 warnValue = (inParam->inDistance > DISTANCE_THREADHOLD) ? (1<<31) : 0;

        MY_LOGD("Write Distance Warning to MTK_VSDOF_FEATURE_WARNIONG tag getDist(%f)"
                "Warning Value %#x", inParam->inDistance, warnValue);

        if (!trySetMetadata<MINT32>(pIMetadata_outApp,
                                    MTK_VSDOF_FEATURE_WARNING, warnValue))
            MY_LOGE("Write MTK_VSDOF_FEATURE_WARNING tag fail!!!");
    }
    // ===================================================================
    //             Debug section (log & dump buffer)
    // ===================================================================
    if (pIMetadata_Dyn1 != nullptr && mbTpBokehMetaLogEn == true) {
        MY_LOGD("mActiveCropSift %d %d %d %d",
                 mActiveCropSift.p.x, mActiveCropSift.p.y,
                 mActiveCropSift.s.w, mActiveCropSift.s.h);
        MY_LOGD("af roi %d %d %d %d", afTopLeftX, afTopLeftY,
                 afBottomRightX, afBottomRightY);
        MY_LOGD("centerPoint %d %d %d %d", centerPointX, centerPointY,
                inParam->afInfo.afRoiTopLeft.x, inParam->afInfo.afRoiTopLeft.y);
        MY_LOGD("bokehLevel(%d), afState(%d), inParam->afInfo.afRoiTopLeft.x(%d),"
                "inParam->afInfo.afRoiTopLeft.y(%d)",
                inParam->prvBokehLevel, inParam->afInfo.afStatus,
                inParam->afInfo.afRoiTopLeft.x, inParam->afInfo.afRoiTopLeft.y);
    }

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/

MERROR S_SEMI_Plugin::getP2Setting(Selection &sel)
{
    sel.mP2Run = ::property_get_bool("vendor.debug.tpi.s.semi.run", 1);
    MY_LOGD_IF(sel.mP2Run > 0, "tpi.semi.run :%d", sel.mP2Run);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
bool S_SEMI_Plugin::rotateBuffer(const MUINT8 *src, const MSize src_size, MUINT8 *dst,
                                 ENUM_ROTATION rotation)
{
    if (nullptr == src || nullptr == dst || 0 == src_size.w || 0 == src_size.h) {
        MY_LOGE("Bad argument");
        return false;
    }
    const MUINT32 buf_len = src_size.w * src_size.h;
    //Rotate Section via CPU
    MINT32 writeRow = 0, writeCol = 0, writePos = 0;
    //
    switch (rotation) {
    case eRotate_270:
    {
        writeRow = 0;
        writeCol = src_size.h - 1;
        writePos = writeCol;
        for (int i = buf_len-1; i >= 0; --i) {
            *(dst+ writePos) = *(src+ i);

            writePos += src_size.h;
            ++writeRow;
            if(writeRow >= src_size.w) {
                writeRow = 0;
                --writeCol;
                writePos = writeCol;
            }
        }
    }
        break;
    case eRotate_90:
    {
        writeRow = src_size.w - 1;
        writeCol = 0;
        writePos = buf_len - src_size.h;
        for (int i = buf_len-1; i >= 0; --i) {
            *(dst+ writePos) = *(src+ i);

            writePos -= src_size.h;
            --writeRow;
            if(writeRow < 0) {
                writeRow = src_size.w - 1;
                ++writeCol;
                writePos = buf_len - src_size.h + writeCol;
            }
        }
    }
        break;
    case eRotate_0:
        ::memcpy(dst, src, buf_len);
        break;
    case eRotate_180:
        break;
    }
    return true;
}
/******************************************************************************
 *
 ******************************************************************************/
void S_SEMI_Plugin::save2file(IImageBuffer* img, int idx, IMetadata *meta)
{
    CAM_ULOGM_FUNCLIFE();
    IImageBuffer* srcImg = img; // create src image
    const size_t PATH_SIZE = 1024;
    char writepath[PATH_SIZE] = {0}, filename[PATH_SIZE] = {0};
    char file_prefix[5][50] = {"Ori_Main1", "DepthImg", "Resize_Main2", "Result"};

    switch (idx) {
    case 1://Main1
    case 2://Main1_DepthImg
    case 4://Main1withMain2
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
    case 2://Main1_DepthImg
        snprintf(filename, PATH_SIZE, "%s_Rotate_%d",file_prefix[1], mTarRotate);
        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main1,
                        TuningUtils::YUV_PORT_UNDEFINED, filename);
        break;
    case 4://Main1withMain2
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
REGISTER_PLUGIN_PROVIDER(Join, S_SEMI_Plugin);
// };//namespace NSPipelinePlugin
// };//namespace NSCam
