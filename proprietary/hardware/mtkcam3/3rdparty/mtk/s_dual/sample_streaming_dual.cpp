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
//
#include <DpBlitStream.h>
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

class DpBlitStream;

#ifdef LOG_TAG
#undef LOG_TAG
#endif // LOG_TAG
#define LOG_TAG "MtkCam/Plugin_S_DUAL"
#include <log/log.h>
#include <android/log.h>

#define PURE_3RDPARTY_FLW

#define MY_LOGI(fmt, arg...)  ALOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)  ALOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)  ALOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)  ALOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD_IF(cond, ...) do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...) do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...) do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define FUNCTION_IN   MY_LOGD("%s +", __FUNCTION__)
#define FUNCTION_OUT  MY_LOGD("%s -", __FUNCTION__)

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

// namespace NSCam {
// namespace NSPipelinePlugin {


class S_DUAL_Plugin : public JoinPlugin::IProvider
{
public:
    typedef JoinPlugin::Property Property;
    typedef JoinPlugin::Selection Selection;
    typedef JoinPlugin::Request::Ptr RequestPtr;
    typedef JoinPlugin::RequestCallback::Ptr RequestCallbackPtr;

public:
    S_DUAL_Plugin();
    ~S_DUAL_Plugin();
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
        IImageBuffer* pInMain1Yuv = nullptr; // Main1 YUV(YV21) Image buffer
        IImageBuffer* pInMain2Yuv = nullptr; // Depthmap(Y8)    Image buffer
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
        MINT8         inDistance                               =  0;    // Distance Info
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
     * @Debug Purpose to confirm output buffer
     * @param [in] target buffer to dump
     * @param [in] dump scenario. 1:Main1 Origin, 2:Main2 Origin
     *                            3:Resize Main2, 4:Merge Main1&Main2
     * @param [in] corresponding metatdata, i.e. main1 or main2
     */
    void save2file(IImageBuffer* img, int idx, IMetadata *meta);

private:
    MRect        mActiveCropSift;

    MINT32 mTpBokehFdTolerence = 0;
    bool   mbTpBokehLogEnable  = false;
    bool   mbShowMain2         = false;
    bool   mbDumpBuffer        = false;
    bool   mSetImgSize         = false;

    DpBlitStream* mpDpBlitStream   = nullptr;
    sp<IImageBuffer> mpResizeMain2 = nullptr;

    TuningUtils::FILE_DUMP_NAMING_HINT mDumpHint_Main1;
    TuningUtils::FILE_DUMP_NAMING_HINT mDumpHint_Main2;

};

S_DUAL_Plugin::S_DUAL_Plugin()
{
    FUNCTION_IN;

    mTpBokehFdTolerence = ::property_get_int64("vendor.debug.vsdof.3rdflow.fd.tolerence",
                                                700000000);
    mbTpBokehLogEnable  = ::property_get_bool("vendor.debug.vsdof.3rdflow.tpBokehLogEn", 1);
    mbShowMain2         = ::property_get_bool("vendor.debug.vsdof.3rdflow.showMain2"   , 0);
    mbDumpBuffer        = ::property_get_bool("vendor.debug.vsdof.3rdflow.dumpbuffer"  , 0);
    mSetImgSize         = ::property_get_bool("vendor.debug.vsdof.3rdflow.specifySize" , 1);

    if (mbShowMain2 > 0)
        mpDpBlitStream = new DpBlitStream();

    int main1Idx, main2Idx;
    StereoSettingProvider::getStereoSensorIndex(main1Idx, main2Idx);

    if (mbDumpBuffer > 0) {
        extract_by_SensorOpenId(&mDumpHint_Main1, main1Idx);
        extract_by_SensorOpenId(&mDumpHint_Main2, main2Idx);
    }
    MY_LOGI("TPI create S_DUAL plugin: Tolerence:%d, DpBit:%p DumpBuffer:%d",
            mTpBokehFdTolerence, (void *)mpDpBlitStream, mbDumpBuffer);

    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(main1Idx);
    if( ! pMetadataProvider.get() ) {
        MY_LOGE("MetadataProvider is NULL");
        return;
    }

    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    //Mapping to active domain by ratio (preview size)
    if (tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveCropSift))
    {
        CropUtil::cropRectByImageRatio(mActiveCropSift, StereoSettingProvider::imageRatio());

        MY_LOGD("mActiveCropSift %d %d %d %d",
                 mActiveCropSift.p.x, mActiveCropSift.p.y,
                 mActiveCropSift.s.w, mActiveCropSift.s.h);
    } else {
        MY_LOGE("tryGetMetadata mActieArrayCrop is NULL");
        return;
    }

    FUNCTION_OUT;
}

S_DUAL_Plugin::~S_DUAL_Plugin()
{
    MY_LOGI("TPI destroy  S_DUAL plugin");
    if (mbShowMain2 > 0) {
        delete mpDpBlitStream;
        mpDpBlitStream = nullptr;
    }
}

void S_DUAL_Plugin::set(MINT32 openID1, MINT32 openID2)
{
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    mOpenID1 = openID1;
    mOpenID2 = openID2;
}

const S_DUAL_Plugin::Property& S_DUAL_Plugin::property()
{
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName     = "MTK DUAL 3rdPartyPlugin";
        prop.mFeatures = MTK_FEATURE_DUAL_YUV;//@ mtk_platform_metadata_tag.h
        //prop.mInPlace = MTRUE;
        //prop.mFaceData = eFD_Current;
        //prop.mPosition = 0;
        inited = true;
    }
    return prop;
}

MERROR S_DUAL_Plugin::negotiate(Selection &sel)
{
    MERROR ret = OK;

    if( sel.mSelStage == eSelStage_CFG ) {
        ret = getConfigSetting(sel);
    }
    MY_LOGD_IF(mbTpBokehLogEnable > 0, "mSelStage input Error (%d)", sel.mSelStage);

    return ret;
}

MERROR S_DUAL_Plugin::getConfigSetting(Selection &sel)

{
    FUNCTION_IN;
    MERROR ret = OK;

    sel.mCfgOrder     = 1;//1st Plugin
    sel.mCfgJoinEntry = eJoinEntry_S_YUV;
    sel.mCfgEnableFD  = MTRUE;
    sel.mCfgRun       = true;

    enum EImageFormat fmt = (property_get_bool("vendor.debug.3rdflow.nv21", 0)) ?
                              eImgFmt_NV21 : eImgFmt_YV12;
    //
    sel.mIBufferMain1.setRequired(true)
                        .addAcceptedFormat(fmt);

    sel.mIBufferMain2.setRequired(true)
                        .addAcceptedFormat(fmt);

    sel.mOBufferMain1.setRequired(true)
                        .addAcceptedFormat(fmt);

    sel.mOBufferDepth.setRequired(true)
                        .addAcceptedFormat(fmt);

    if (mSetImgSize) {
        sel.mCfgInfo.mMaxOutSize = StereoSizeProvider::getInstance()->getPreviewSize();
        //
        sel.mIBufferMain1.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
        sel.mIBufferMain2.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
        sel.mOBufferMain1.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
        sel.mOBufferDepth.setSpecifiedSize(sel.mCfgInfo.mMaxOutSize);
    } else {
        sel.mIBufferMain1.addAcceptedSize(eImgSize_Full);
        sel.mIBufferMain2.addAcceptedSize(eImgSize_Full);
        sel.mOBufferMain1.addAcceptedSize(eImgSize_Full);
        sel.mOBufferDepth.addAcceptedSize(eImgSize_Full);
    }

    sel.mIMetadataApp.setRequired(true);
    sel.mIMetadataHal1.setRequired(true);
    sel.mIMetadataDynamic1.setRequired(true);

    MY_LOGI_IF(sel.mCfgRun == false, "S_DUAL_Plugin is not config as run state(%d)", sel.mCfgRun);

    FUNCTION_OUT;
    return ret;
}

void S_DUAL_Plugin::init()
{
    MY_LOGI("init S_DUAL plugin");
}

void S_DUAL_Plugin::uninit()
{
    MY_LOGI("uninit S_DUAL plugin");
}

void S_DUAL_Plugin::abort(std::vector<RequestPtr> &pRequests)
{
    (void)pRequests;
    MY_LOGD("uninit S_DUAL plugin");
};
/******************************************************************************
 *
 ******************************************************************************/

MERROR S_DUAL_Plugin::process(RequestPtr pRequest, RequestCallbackPtr /*pCallback*/)
{
    FUNCTION_IN;
    MERROR ret = OK;

    TpBkhInBuffer  inBufs;
    TpBkhOutBuffer outBufs;
    TpBkhInParam   inParams = {0};

    if (pRequest->mIBufferMain1.get()!=nullptr && pRequest->mIBufferMain2.get()!=nullptr)
    {
        if (mbDumpBuffer) {
            IMetadata* pInHalMeta;
            pInHalMeta = pRequest->mIMetadataHal1->acquire();
            save2file(pRequest->mIBufferMain1->acquire(), 1, pInHalMeta);
            pInHalMeta = pRequest->mIMetadataHal2->acquire();
            save2file(pRequest->mIBufferMain2->acquire(), 2, pInHalMeta);
        }
        prepareTpBkhInOutData(pRequest, &inBufs, &outBufs, &inParams);

        if (!processTpBkh(pRequest, inBufs, outBufs, inParams)) {
            MY_LOGE("processTpBkh Return Error");
            ret = UNKNOWN_ERROR;
        }
    }
    else
    {
        MY_LOGE("input buffer Error! Main1_ptr:%p, Main2_ptr:%p,",
                pRequest->mIBufferMain1.get(), pRequest->mIBufferMain2.get());

        ret = UNKNOWN_ERROR;
    }

    FUNCTION_OUT;
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/

#ifdef PURE_3RDPARTY_FLW
bool S_DUAL_Plugin::imgProcess( RequestPtr const  pRequest,
                    IImageBuffer* pSrc1, IImageBuffer* pSrc2, IImageBuffer* target)
{
    unsigned char *outBuf = nullptr, *src1Buf = nullptr, *src2Buf = nullptr;
    size_t outBufSize = 0, planeCnt = 0, outBuf_pos, strides_diff = 0;
    size_t src1BufSize = 0, src2BufSize = 0, src1Strides = 0, src2Strides = 0, outBufStrides = 0;
    IMetadata* pMeta = nullptr;

    FUNCTION_IN;
    if (mbShowMain2) {  //Resize Main2 to 1/4
        if (mpDpBlitStream != nullptr) {
            MY_LOGD("src1=%p w:%04d-h:%04d\t ; src2=%p w:%04d-h:%04d\t ; out=w:%04d-h:%04d" ,
                    pSrc1, pSrc1->getImgSize().w, pSrc1->getImgSize().h,
                    pSrc2, pSrc2->getImgSize().w, pSrc2->getImgSize().h,
                    target->getImgSize().w, target->getImgSize().h);

            if (mpResizeMain2 == nullptr) {
                MY_LOGD("Debug Purpose, Resize Main2 + !");

                MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};
                MUINT32 resizeImg2_strides[3]= {(MUINT32)target->getImgSize().w/4,
                    (MUINT32)target->getImgSize().w/8,
                    (MUINT32)target->getImgSize().w/8};

                NSCam::IImageBufferAllocator::ImgParam resizeImg2Para(eImgFmt_YV12,
                        target->getImgSize()/4, resizeImg2_strides, FULL_RAW_BOUNDARY,
                        3/*plane count*/);

                mpResizeMain2 = VSDOF::util::createEmptyImageBuffer(resizeImg2Para, "resizeImg2",
                        (eBUFFER_USAGE_HW_CAMERA_READWRITE |
                         eBUFFER_USAGE_SW_READ_OFTEN |
                         eBUFFER_USAGE_SW_WRITE_RARELY));
                MY_LOGD("Debug Purpose, Create Resize Main2 *** ");
            }

            sMDP_Config resizeM2Cfg = {
                .pDpStream  = mpDpBlitStream,
                .pSrcBuffer = pSrc2,
                .pDstBuffer = mpResizeMain2.get(),
                .rotAngle   = 0,
            };// Resize src2 to output size.
            excuteMDP(resizeM2Cfg);

            MY_LOGD("Debug Purpose, Resize Main2 - (DONE)!\tresizeImg2:w:%04d,h:%04d",
                    mpResizeMain2->getImgSize().w, mpResizeMain2->getImgSize().h);
            pSrc2 = mpResizeMain2.get();

            if (mbDumpBuffer) {
                pMeta = pRequest->mIMetadataHal1->acquire();
                save2file(pSrc2, 3, pMeta);
            }
        } else {
            MY_LOGE("DpBlitStream is not Init!! Return");
            return false;
        }
    }
    //
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
    if (mbShowMain2) {  //Copy Resize Main2 to Main1
        for (size_t i = 0; i < planeCnt; i++) {
            src1Buf = (unsigned char*)pSrc1->getBufVA(i);
            src2Buf = (unsigned char*)pSrc2->getBufVA(i);
            outBuf  = (unsigned char*)target->getBufVA(i);

            src1BufSize = pSrc1->getBufSizeInBytes(i);
            src2BufSize = pSrc2->getBufSizeInBytes(i);
            outBufSize  = target->getBufSizeInBytes(i);

            src1Strides = pSrc1->getBufStridesInBytes(i);
            src2Strides = pSrc2->getBufStridesInBytes(i);

            memcpy(outBuf, src1Buf, src1BufSize);
            if (src2Strides > src1Strides) {
                MY_LOGE("Main2 strides(%ld) > Main1 strides(%ld)", src2Strides, src1Strides);
                return false;
            }
            strides_diff = src1Strides - src2Strides;

            outBuf_pos = 0;
            for (size_t j = 0; j < src2BufSize; j++) {
                if (((j % src2Strides)==0) && (j != 0))
                    outBuf_pos += strides_diff;

                if (outBuf_pos < src1BufSize)
                    outBuf[outBuf_pos] = src2Buf[j];

                outBuf_pos++;
            }
        }
    }

    if (mbShowMain2 || mbDumpBuffer)
        target->syncCache();

    if (mbDumpBuffer) {
        pMeta = pRequest->mIMetadataHal1->acquire();
        save2file(target, 4, pMeta);
    }

    FUNCTION_OUT;
    return true;
}
#endif
/******************************************************************************
 *
 ******************************************************************************/

bool S_DUAL_Plugin::processTpBkh(RequestPtr const  pRequest,
            TpBkhInBuffer inBuf, TpBkhOutBuffer outBuf, const TpBkhInParam& inParam)
{
    // ---------------------------
    // Get input parameter example
    // ---------------------------

    // Get FD info
    MUINT8 fdFaceNumber = inParam.faceCounts;             // Number of faces
    for (MUINT8 i = 0 ; i < fdFaceNumber; i++)
    {
        MPoint fdRoiTopLeft    = inParam.fdInfo[i].fdRoiTopLeft;    // FD ROI top left point
        MPoint fdRoiBotRight   = inParam.fdInfo[i].fdRoiBotRight;   // FD ROI bottom right point
        MINT32 fdRotationAngle = inParam.fdInfo[i].fdRotationAngle; // FD rotation information
        MY_LOGD_IF(mbTpBokehLogEnable, "faceId:%d, fdRoiTopLeft(%d.%d),"
                   "fdRoiBotRight(%d.%d), fdRotationAngle(%d)",
                   i, fdRoiTopLeft.x, fdRoiTopLeft.y, fdRoiBotRight.x, fdRoiBotRight.y,
                   fdRotationAngle);
    }

    // --------------------------------
    // Add IN AND OUT vendor code here
    // and return process result
    // --------------------------------
    // Need to remove below section for real implementation.
#ifdef PURE_3RDPARTY_FLW
    return imgProcess(pRequest, inBuf.pInMain1Yuv, inBuf.pInMain2Yuv, outBuf.pOutBokehImg);
#endif
}

/******************************************************************************
 *
 ******************************************************************************/
bool S_DUAL_Plugin::prepareTpBkhInOutData(RequestPtr const  pRequest,
            TpBkhInBuffer* inBuffers, TpBkhOutBuffer* outBuffers, TpBkhInParam* inParam)

{
    FUNCTION_IN;
    // =============================================================
    //             Prepare In/Out Img Buffer
    // =============================================================

    // Input Buffer
    if (pRequest->mIBufferMain1 != nullptr) {
         inBuffers->pInMain1Yuv = pRequest->mIBufferMain1->acquire();
    } else {
        MY_LOGE("Get input Buffer for Main1 Error");
    }
    //
    if (pRequest->mIBufferMain2 != nullptr) {
        inBuffers->pInMain2Yuv = pRequest->mIBufferMain2->acquire();
    } else {
        MY_LOGE("Get input Buffer for Main2 Error");
    }
    //Output Processed Buffer
    if (pRequest->mOBufferMain1 != nullptr) {
        outBuffers->pOutBokehImg = pRequest->mOBufferMain1->acquire();
    } else {
        MY_LOGE("Get output Buffer for ClearImage Error");
    }
    //Output Depth Buffer
    if (pRequest->mOBufferDepth != nullptr) {
        outBuffers->pOutBokehPreview =  pRequest->mOBufferDepth->acquire();
    } else {
        // ### Need to pay attention for next test ##
        MY_LOGD("Get output Buffer for Depth Error,"
                "Since no real integration target");
    }

    // =============================================================
    //             Prepare Metadata Pointer
    // =============================================================
    IMetadata *pIMetadata_Dyn1 = nullptr, *pIMetadata_App = nullptr;
    IMetadata *pIMetadata_Hal1 = nullptr;
    std::bitset<3> metaResult;

    if (pRequest->mIMetadataDynamic1 != nullptr) {
        pIMetadata_Dyn1 = pRequest->mIMetadataDynamic1->acquire();
        metaResult.set(0);
    }
    if (pRequest->mIMetadataApp != nullptr) {
        pIMetadata_App = pRequest->mIMetadataApp->acquire();
        metaResult.set(1);
    }
    if (pRequest->mIMetadataHal1 != nullptr) {
        pIMetadata_Hal1 = pRequest->mIMetadataHal1->acquire();
        metaResult.set(2);
    }
    if (pIMetadata_Dyn1 != nullptr && pIMetadata_Hal1 != nullptr &&
        pIMetadata_App  != nullptr)
        MY_LOGD("Dyn1Meta cnt:%03d, AppMeta cnt:%03d, Hal1Meta cnt:%03d"
                "outAppMeta cnt:%03d",
                pIMetadata_Dyn1->count(), pIMetadata_App->count(),
                pIMetadata_Hal1->count());
    else
        MY_LOGE("Getd Mete Ptr Error (%#lx)", metaResult.to_ulong());

    // =============================================================
    //             Prepare Input Parameter (Bokeh level & AF roi)
    // =============================================================
    MINT32 afTopLeftX     = 0, afTopLeftY     = 0;
    MINT32 afBottomRightX = 0, afBottomRightY = 0;
    MINT32 afROINum       = 0, afROITagStart  = 0;
    MINT32 centerPointX   = 0, centerPointY   = 0;
    // Fill in bokeh level
    if (!tryGetMetadata<MINT32>(pIMetadata_Dyn1, MTK_STEREO_FEATURE_DOF_LEVEL, inParam->prvBokehLevel))
        MY_LOGE("Get Meta: MTK_STEREO_FEATURE_DOF_LEVEL Error");
    // Fill in afInfo
    if (!tryGetMetadata<MUINT8>(pIMetadata_App, MTK_CONTROL_AF_MODE,  inParam->afInfo.afMode))
        MY_LOGE("Get Meta :MTK_CONTROL_AF_MODE Error");

    if (!tryGetMetadata<MUINT8>(pIMetadata_Dyn1, MTK_CONTROL_AF_STATE, inParam->afInfo.afStatus))
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
            MY_LOGD_IF(mbTpBokehLogEnable,"Get Meta_Dynamic1 done");
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
    auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);

    // 2. get timestamp
    MINT64 p1timestamp = 0;
    if (!tryGetMetadata<MINT64>(pIMetadata_Hal1, MTK_P1NODE_FRAME_START_TIMESTAMP, p1timestamp))
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
                MY_LOGD_IF(mbTpBokehLogEnable, "Number_of_faces:%d",
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
                    MY_LOGD_IF(mbTpBokehLogEnable, "FD[%d] ROI %d %d %d %d,"
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
            MY_LOGD_IF(mbTpBokehLogEnable,"FD size:%lu", fdData.size());
    }
    // 4. fdData must be return after use
    fdReader->queryUnlock(fdData);

    // ===================================================================
    //             Debug section (log & dump buffer)
    // ===================================================================
    if (pIMetadata_Dyn1 != nullptr && mbTpBokehLogEnable == true) {
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
    FUNCTION_OUT;
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
void S_DUAL_Plugin::save2file(IImageBuffer* img, int idx, IMetadata *meta)
{
    FUNCTION_IN;

    IImageBuffer* srcImg = img; // create src image
    const size_t PATH_SIZE = 1024;
    char writepath[PATH_SIZE] = {0}, filename[PATH_SIZE] = {0};
    char file_prefix[5][50] = {"Ori_Main1", "Ori_Main2", "Resize_Main2", "Result"};

    switch (idx) {
        case 1://Main1
        case 4://Main1withMain2
            extract(&mDumpHint_Main1, meta);
            extract(&mDumpHint_Main1, srcImg);
            break;
        case 2://Main2
        case 3://ResizeMain2
            extract(&mDumpHint_Main2, meta);
            extract(&mDumpHint_Main2, srcImg);
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
        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main2,
                        TuningUtils::YUV_PORT_UNDEFINED, filename);
        break;
    case 3://ResizeMain2
        snprintf(filename, PATH_SIZE, "%s",file_prefix[2]);
        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main2,
                        TuningUtils::YUV_PORT_UNDEFINED, filename);
        break;
    case 4://Main1withMain2
        snprintf(filename, PATH_SIZE, "%s",file_prefix[3]);
        genFileName_YUV(writepath, PATH_SIZE, &mDumpHint_Main1,
                        TuningUtils::YUV_PORT_UNDEFINED, filename);
        break;
    }
    srcImg->saveToFile(writepath);

    FUNCTION_OUT;
}
/******************************************************************************
 *
 ******************************************************************************/
REGISTER_PLUGIN_PROVIDER(Join, S_DUAL_Plugin);
// };//namespace NSPipelinePlugin
// };//namespace NSCam
