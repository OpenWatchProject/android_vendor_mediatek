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

#define LOG_TAG "MtkCam/PackUtils"
//
#include <mtkcam3/pipeline/utils/packutils/PackUtils.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_PIPELINE_UTILITY);
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::ULog;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if (            (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if (            (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if (            (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( CC_UNLIKELY(cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *  IspTuningDataFilter
 ******************************************************************************/

///////////////////////////////////////////////////////////
// Debug Property
#define DEBUG_STRING_ENABLE_FILTER "vendor.debug.camera.isppack.filter.enable"
#define DEBUG_STRING_ENABLE_CtrlApp_FILTER "vendor.debug.camera.isppack.filter.CtrlApp.enable"
#define DEBUG_STRING_ENABLE_ResultApp_FILTER "vendor.debug.camera.isppack.filter.ResultApp.enable"
#define DEBUG_STRING_ENABLE_ResultHal_FILTER "vendor.debug.camera.isppack.filter.ResultHal.enable"
#define DEBUG_STRING_ENABLE_INSERT_DEBUG_WHITELIST "vendor.debug.camera.isppack.filter.insertTag.enable"
#define DEBUG_INSERT_WHITE_LIST_MAX_COUNT 100

IspTuningDataFilter::
IspTuningDataFilter()
{

}

IspTuningDataFilter::
~IspTuningDataFilter()
{

}

MERROR
IspTuningDataFilter::
getTuningDataWhiteList(
        std::set<IMetadata::Tag_t>& outCtrlAppMetaFinalWhiteList,
        std::set<IMetadata::Tag_t>& outResultAppMetaFinalWhiteList,
        std::set<IMetadata::Tag_t>& outResultHalMetaFinalWhiteList,
        std::set<IMetadata::Tag_t> const& inIspTuningCtrlAppWhiteList,
        std::set<IMetadata::Tag_t> const& inIspTuningResultAppWhiteList,
        std::set<IMetadata::Tag_t> const& inIspTuningResultHalWhiteList
        )
{
    int enableWhiteListAll = property_get_int32(DEBUG_STRING_ENABLE_FILTER, 1);
    int enableWhiteListCtrlApp = property_get_int32(DEBUG_STRING_ENABLE_CtrlApp_FILTER, 1);
    int enableWhiteListResultApp = property_get_int32(DEBUG_STRING_ENABLE_ResultApp_FILTER, 1);
    int enableWhiteListResultHal = property_get_int32(DEBUG_STRING_ENABLE_ResultHal_FILTER, 1);
    int enableWhiteListInsertTag = property_get_int32(DEBUG_STRING_ENABLE_INSERT_DEBUG_WHITELIST, 0);

    int debugInsertWhiteListCtrlApp[DEBUG_INSERT_WHITE_LIST_MAX_COUNT] = {0};
    int debugInsertWhiteListResultApp[DEBUG_INSERT_WHITE_LIST_MAX_COUNT] = {0};
    int debugInsertWhiteListResultHal[DEBUG_INSERT_WHITE_LIST_MAX_COUNT] = {0};
    CAM_LOGD("[%s] enableWhiteListAll(%d) enableWhiteListCtrlApp(%d) enableWhiteListResultApp(%d) enableWhiteListResultHal(%d)",
        __FUNCTION__,enableWhiteListAll,enableWhiteListCtrlApp,enableWhiteListResultApp,enableWhiteListResultHal);

    //vendor.debug.camera.isppack.filter.CtrlApp.insert.count   : debug Ctrl App tag count
    //vendor.debug.camera.isppack.filter.ResultApp.insert.count : debug Result App tag count
    //vendor.debug.camera.isppack.filter.ResultHal.insert.count : debug Result Hal tag count
    //vendor.debug.camera.isppack.filter.CtrlApp.insert.tag1      : insert debug Ctrl App tag #1
    //vendor.debug.camera.isppack.filter.CtrlApp.insert.tag2      : insert debug Ctrl App tag #2
    //vendor.debug.camera.isppack.filter.CtrlApp.insert.tag...    : insert debug Ctrl App tag #...
    //vendor.debug.camera.isppack.filter.CtrlApp.insert.tag99     : insert debug Ctrl App tag #99
    //vendor.debug.camera.isppack.filter.ResultApp.insert.tag1      : insert debug Result App tag #1
    //vendor.debug.camera.isppack.filter.ResultApp.insert.tag2      : insert debug Result Apptag #2
    //vendor.debug.camera.isppack.filter.ResultApp.insert.tag...    : insert debug Result App tag #...
    //vendor.debug.camera.isppack.filter.ResultApp.insert.tag99     : insert debug Result App tag #99
    //vendor.debug.camera.isppack.filter.ResultHal.insert.tag1      : insert debug Result Hal tag #1
    //vendor.debug.camera.isppack.filter.ResultHal.insert.tag2      : insert debug Result Hal tag #2
    //vendor.debug.camera.isppack.filter.ResultHal.insert.tag...    : insert debug Result Hal tag #...
    //vendor.debug.camera.isppack.filter.ResultHal.insert.tag99     : insert debug Result Hal tag #99
    //type : CtrlApp / ResultApp / ResultHal
#define DEBUG_WHITELIST(_type_) \
    if(enableWhiteListInsertTag==1) \
    {   \
        std::set<IMetadata::Tag_t> debugInsertWhiteList##_type_;    \
        for(auto i=0; i<DEBUG_INSERT_WHITE_LIST_MAX_COUNT; i++) \
        {   \
            std::string str = "vendor.debug.camera.isppack.filter." #_type_ ".insert.tag" + std::to_string(i);  \
            MUINT32 tag = property_get_int32(str.c_str(), 0);   \
            if(tag!=0)  \
            {   \
                debugInsertWhiteList##_type_.insert(tag);   \
                CAM_LOGD("[%s]" #_type_ " Whitelist insert tag[%d]: %d",__FUNCTION__,i,tag); \
            }   \
        }   \
        if(debugInsertWhiteList##_type_.size() > 0)   \
        {   \
            out##_type_##MetaFinalWhiteList.insert(debugInsertWhiteList##_type_.begin(),debugInsertWhiteList##_type_.end());  \
        }   \
        CAM_LOGD("[%s]" #_type_ " Whitelist count: %zu",__FUNCTION__,(size_t)debugInsertWhiteList##_type_.size());\
    }
    ///////////////////////////////////////////////////////////
    outCtrlAppMetaFinalWhiteList.clear();
    outResultAppMetaFinalWhiteList.clear();
    outResultHalMetaFinalWhiteList.clear();
    if(enableWhiteListAll==1)
    {
        //control app meta
        if(enableWhiteListCtrlApp==1)
        {
            outCtrlAppMetaFinalWhiteList.insert(inIspTuningCtrlAppWhiteList.begin(),inIspTuningCtrlAppWhiteList.end());
            outCtrlAppMetaFinalWhiteList.insert(appControlMetaCommonWhiteList.begin(),appControlMetaCommonWhiteList.end());
            DEBUG_WHITELIST(CtrlApp)
        }

        //app result meta
        if(enableWhiteListResultApp==1)
        {
            outResultAppMetaFinalWhiteList.insert(inIspTuningResultAppWhiteList.begin(),inIspTuningResultAppWhiteList.end());
            outResultAppMetaFinalWhiteList.insert(appResultMetaCommonWhiteList.begin(),appResultMetaCommonWhiteList.end());
            DEBUG_WHITELIST(ResultApp)
        }
        //hal result meta
        if(enableWhiteListResultHal==1)
        {
            outResultHalMetaFinalWhiteList.insert(inIspTuningResultHalWhiteList.begin(),inIspTuningResultHalWhiteList.end());
            outResultHalMetaFinalWhiteList.insert(halResultMetaCommonWhiteList.begin(),halResultMetaCommonWhiteList.end());
            //for debug: control app meta
            DEBUG_WHITELIST(ResultHal)
        }
    }
    //
    CAM_LOGD("[%s] Metadata White List Count: APP_Ctrl(ISP(%zu)/common(%zu)/final(%zu)) APP_Result(ISP(%zu)/common(%zu)/final(%zu)) Hal_Result(ISP(%zu)/common(%zu)/final(%zu))",__FUNCTION__,
        inIspTuningCtrlAppWhiteList.size(),appControlMetaCommonWhiteList.size(),outCtrlAppMetaFinalWhiteList.size(),
        inIspTuningResultAppWhiteList.size(),appResultMetaCommonWhiteList.size(),outResultAppMetaFinalWhiteList.size(),
        inIspTuningResultHalWhiteList.size(),halResultMetaCommonWhiteList.size(),outResultHalMetaFinalWhiteList.size());
    //
    return 0;
}

/******************************************************************************
 *  IspTuningDataPackUtil
 ******************************************************************************/

MUINT32
IspTuningDataPackUtil::
calcuCheckSum(char *pSrc, size_t length)
{
    MUINT32 ret = 0;
    for(size_t i=0; i<length; i++)
    {
        ret+=(*pSrc+i);
    }
    return ret;
}

MERROR
IspTuningDataPackUtil::
packFeoDataToHalMeta([[maybe_unused]] android::sp<IImageBufferHeap> pInputBufferHeap,
                     [[maybe_unused]] IMetadata &appControlMeta,
                     [[maybe_unused]] IMetadata &halResultMeta)
{
#if MTKCAM_HAVE_AINR_SUPPORT
    CAM_TRACE_CALL();
    CAM_LOGD("[%s] packFeoDataToHalMeta +",__FUNCTION__);
    auto fefm = ainr::IAinrFeFm::createInstance();
    if (CC_UNLIKELY(fefm.get() == nullptr))
    {
        CAM_LOGE("[%s] Get fefm error",__FUNCTION__);
        return UNKNOWN_ERROR;
    }

    ainr::IAinrFeFm::ConfigParams fefmcfg;
    fefmcfg.openId = 0;
    if(!IMetadata::getEntry<MINT32>(&halResultMeta, MTK_HAL_REQUEST_SENSOR_ID, fefmcfg.openId))
    {
        CAM_LOGW("[%s] MTK_HAL_REQUEST_SENSOR_ID is empty",__FUNCTION__);
    }

    // We use rrzo to do FEFM
    fefmcfg.bufferSize.w = pInputBufferHeap->getImgSize().w;  // rrzo/yuvo width
    fefmcfg.bufferSize.h = pInputBufferHeap->getImgSize().h; // rrzo/yuvo height
    fefm->init(fefmcfg);

    MSize feoSize = {}, fmoSize = {};
    fefm->getFeFmSize(feoSize, fmoSize);
    CAM_LOGD("[%s] getFeFmSize feoSize(%d,%d)",__FUNCTION__,feoSize.w,feoSize.h);

    //////////////////////////////////////////////////
    //allocate buffer
    MINT32 bufBoundaryInBytes[3] = {0};
    MUINT32 strideInBytes[3] = {0};
    MUINT32 format = eImgFmt_STA_BYTE;
    size_t planeCount = NSCam::Utils::Format::queryPlaneCount(format);
    for (size_t i = 0; i < planeCount; i++)
    {
        strideInBytes[i] =
            (NSCam::Utils::Format::queryPlaneWidthInPixels(format, i, feoSize.w) *
            NSCam::Utils::Format::queryPlaneBitsPerPixel(format, i) + 7) / 8;

        CAM_LOGD("[%s] allocBuffer (%zu/%zu) strideInBytes(%d)",__FUNCTION__,
            i,planeCount,strideInBytes[i]);
    }
    IIonImageBufferHeap::AllocImgParam_t imgParam(
            format, MSize(feoSize.w,feoSize.h), strideInBytes, bufBoundaryInBytes, planeCount);

    sp<IIonImageBufferHeap> heap =
        IIonImageBufferHeap::create("PackFeo",
                imgParam, IIonImageBufferHeap::AllocExtraParam(), MFALSE);

    if (heap == NULL)
    {
        CAM_LOGE("[%s] heap is NULL",__FUNCTION__);
        return UNKNOWN_ERROR;
    }

    //////////////////////////////////////////////////
    // do FE
    sp<IImageBuffer> feoBuffer = heap->createImageBuffer();
    feoBuffer->lockBuf("feoBuffer",eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_READ_OFTEN  );
    sp<IImageBuffer> rrzoBuffer = pInputBufferHeap->createImageBuffer();
    rrzoBuffer->lockBuf("rrzoBuffer",eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_READ_OFTEN  );
    ainr::IAinrFeFm::DataPackage pack;
    pack.appMeta = &appControlMeta;
    pack.halMeta = &halResultMeta;
    pack.inBuf   = rrzoBuffer.get(); // RRZO/YUVO buffers
    pack.outBuf  = feoBuffer.get(); // FEO buffer
    CAM_LOGD("[%s] doFe() +",__FUNCTION__);
    fefm->doFe(&pack);
    CAM_LOGD("[%s] doFe() -",__FUNCTION__);

    //////////////////////////////////////////////////
    // pack feo
    IMetadata::Memory feoInfo;
    feoInfo.resize(sizeof(ainr::IAinrFeFm::FeoInfo));
    ainr::IAinrFeFm::FeoInfo *infoPtr = reinterpret_cast<ainr::IAinrFeFm::FeoInfo*>(feoInfo.editArray());
    infoPtr->rrzoSize = pInputBufferHeap->getImgSize();
    infoPtr->feoTotalSize = feoBuffer->getBufSizeInBytes(0);
    IMetadata::setEntry<IMetadata::Memory>(&halResultMeta, MTK_ISP_FEO_INFO, feoInfo);
    CAM_LOGD("[%s] feoInfo:rrzoSize(%d,%d) feoTotalSize(%d)",__FUNCTION__,infoPtr->rrzoSize.w,infoPtr->rrzoSize.h,infoPtr->feoTotalSize);
    //
    IMetadata::Memory feoData;
    feoData.resize(infoPtr->feoTotalSize);
    CAM_LOGD("[%s] memcpy src(%" PRIxPTR ") dst(%p) size(%d)",__FUNCTION__,feoBuffer->getBufVA(0),feoData.editArray(),infoPtr->feoTotalSize);
    memcpy(reinterpret_cast<void*>(feoData.editArray()),
                        reinterpret_cast<void*>(feoBuffer->getBufVA(0)),
                        infoPtr->feoTotalSize);
    IMetadata::setEntry<IMetadata::Memory>(&halResultMeta, MTK_ISP_FEO_DATA, feoData);
    feoBuffer->unlockBuf("PackfeoBufferFeo");
    rrzoBuffer->unlockBuf("rrzoBuffer");
    CAM_LOGD("[%s] packFeoDataToHalMeta -",__FUNCTION__);
    return OK;
#else  //else of #if MTKCAM_HAVE_AINR_SUPPORT
    CAM_LOGE("[%s] the project don't define MTKCAM_HAVE_AINR_SUPPORT, can't run pack FEO flow!",__FUNCTION__);
    return INVALID_OPERATION;
#endif //end of #if MTKCAM_HAVE_AINR_SUPPORT
}


MERROR
IspTuningDataPackUtil::
downSampleImageBufferHeap(android::sp<IImageBufferHeap> pInputRrzoBufferHeap,
                                    [[maybe_unused]] android::sp<IImageStreamInfo> pInputRrzoStreamInfo,
                                    android::sp<IImageBufferHeap> &pOutputBufferHeap,
                                    [[maybe_unused]] IMetadata &appControlMeta,
                                    [[maybe_unused]] IMetadata &halResultMeta)
{
    CAM_TRACE_CALL();
    CAM_LOGD("[%s] downSampleImageBufferHeap +",__FUNCTION__);
    MERROR ret = OK;
    //SUPPORT_YUV_BSS must be 0 && MTKCAM_HAVE_MFB_SUPPORT must >= 1, or the flow to use the BSS Core may cause compiler error
#if MTKCAM_HAVE_MFB_SUPPORT >= 1
#ifdef SUPPORT_YUV_BSS
#if (SUPPORT_YUV_BSS==0)
    pOutputBufferHeap.clear();

    std::shared_ptr<BSScore::IBssCore> spBssCore = BSScore::IBssCore::createInstance();
    if (spBssCore.get() == NULL) {
        CAM_LOGE("[%s] create IBssCore instance failed",__FUNCTION__);
        ret = UNKNOWN_ERROR;
        return ret;
    }
    //
    MINT32 sensorId;
    IMetadata::IEntry sensorIdEntry = halResultMeta.entryFor(MTK_HAL_REQUEST_SENSOR_ID);
    if(!sensorIdEntry.isEmpty())
    {
        sensorId = sensorIdEntry.itemAt(0, Type2Type<MINT32>());
    }
    else
    {
        CAM_LOGE("[%s] MTK_HAL_REQUEST_SENSOR_ID is empty",__FUNCTION__);
        ret = UNKNOWN_ERROR;
        return ret;
    }
    spBssCore->Init(sensorId);
    //
    // before create ImageBuffer, need to unlock first,
    // or the ImageBuffer be created with usage (eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_READ_OFTEN)
    android::sp<IImageBuffer> pRrzoImgBuf = nullptr;
    if (pInputRrzoBufferHeap->getImgFormat() == eImgFmt_BLOB && pInputRrzoStreamInfo != nullptr) {
        CAM_LOGD("[%s] rrzo is eImgFmt_BLOB, need to use createImageBuffers_FromBlobHeap()",__FUNCTION__);
        std::vector<IImageBuffer*> vImageBuffer
            = pInputRrzoBufferHeap->createImageBuffers_FromBlobHeap(pInputRrzoStreamInfo->getImageBufferInfo(), LOG_TAG);
        if (vImageBuffer.size() > 0)
            pRrzoImgBuf = vImageBuffer[0];
    }
    else
    {
        CAM_LOGD("[%s] rrzo is NOT eImgFmt_BLOB, need to use createImageBuffer()",__FUNCTION__);
        pRrzoImgBuf = pInputRrzoBufferHeap->createImageBuffer();
    }
    //
    if ( pRrzoImgBuf == nullptr )
    {
        CAM_LOGE("[%s] pRrzoImgBuf is NULL Buffer",__FUNCTION__);
        ret = UNKNOWN_ERROR;
        return ret;
    }
    //
    // calcu & get the final output image size
    MUINT32 BssSize = spBssCore->getZipOutSize(pRrzoImgBuf.get(),&halResultMeta);
    if(BssSize==0)
    {
        CAM_LOGE("[%s] BssSize is 0, can't allocate BSS RRZO image buffer",__FUNCTION__);
        ret = UNKNOWN_ERROR;
        return ret;
    }
    else
    {
        CAM_LOGD("[%s] BssSize is %d",__FUNCTION__,BssSize);
        IImageBufferAllocator::ImgParam imgParam(BssSize,0);
        android::sp<IImageBufferHeap> pDownSampleImgHeap = IIonImageBufferHeap::create("BssRrzoBuffer",imgParam,IIonImageBufferHeap::AllocExtraParam(),MTRUE);
        android::sp<IImageBuffer> pDownSampleImg = pDownSampleImgHeap->createImageBuffer();
        //
        if ( pDownSampleImg.get() == 0 )
        {
            CAM_LOGE("[%s] NULL Buffer",__FUNCTION__);
            ret = UNKNOWN_ERROR;
            return ret;
        }
        MINT32 ispFrameIndex = 0;
        if(IMetadata::getEntry<MINT32>(&appControlMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX, ispFrameIndex))
        {
            CAM_LOGD("[%s] ispFrameIndex(%d) doZip +",__FUNCTION__,ispFrameIndex);
            //
            // before do zip, need to lock image buffer
            if ( !pDownSampleImg->lockBuf( "BssRrzoBuffer", (eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK)) )
            {
                CAM_LOGE("[%s] lock Buffer failed",__FUNCTION__);
                ret = UNKNOWN_ERROR;
                return ret;
            }
            if ( !pRrzoImgBuf->lockBuf( "OriginalRrzoBuffer", (eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_READ_OFTEN)) )
            {
                CAM_LOGE("[%s] lock Buffer failed",__FUNCTION__);
                ret = UNKNOWN_ERROR;
                return ret;
            }
            //  Do Zip
            if(ispFrameIndex == 0)
            {
                spBssCore->doZip(pRrzoImgBuf.get(),pDownSampleImg.get(),BssSize,&halResultMeta,MTRUE);
            }
            else
            {
                spBssCore->doZip(pRrzoImgBuf.get(),pDownSampleImg.get(),BssSize,&halResultMeta,MFALSE);
            }
            CAM_LOGD("[%s] doZip -",__FUNCTION__);
            //
            // after do zip, need to unlock image buffer
            if ( !pDownSampleImg->unlockBuf( "BssRrzoBuffer") )
            {
                CAM_LOGE("[%s] unlock Buffer failed",__FUNCTION__);
            }
            if ( !pRrzoImgBuf->unlockBuf( "OriginalRrzoBuffer") )
            {
                CAM_LOGE("[%s] unlock Buffer failed",__FUNCTION__);
            }
            //
            //update final rrzo image
            if(pDownSampleImg->getImageBufferHeap() == nullptr)
            {
                CAM_LOGE("[%s] pDownSampleImg->getImageBufferHeap() is NULL",__FUNCTION__);
                ret = UNKNOWN_ERROR;
                return ret;
            }
            pOutputBufferHeap = pDownSampleImg->getImageBufferHeap();
        }
        else
        {
            CAM_LOGE("[%s] MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX is NULL",__FUNCTION__);
            ret = UNKNOWN_ERROR;
            return ret;
        }
    }
#else   //else of #if (SUPPORT_YUV_BSS==0)
    CAM_LOGW("[%s] the function don't supprot SUPPORT_YUV_BSS(%d)",__FUNCTION__,SUPPORT_YUV_BSS);
    IImageBuffer* pTemp =  pInputRrzoBufferHeap->createImageBuffer();
    pOutputBufferHeap = pTemp->getImageBufferHeap();
    pTemp = nullptr;
#endif  //end of #if (SUPPORT_YUV_BSS==0)
#else   //else of #ifdef SUPPORT_YUV_BSS
    CAM_LOGW("[%s] the function don't supprot, becuase SUPPORT_YUV_BSS is not defined",__FUNCTION__);
    IImageBuffer* pTemp =  pInputRrzoBufferHeap->createImageBuffer();
    pOutputBufferHeap = pTemp->getImageBufferHeap();
    pTemp = nullptr;
#endif  //end of #ifdef SUPPORT_YUV_BSS
#else   //else of #if MTKCAM_HAVE_MFB_SUPPORT >= 1
    CAM_LOGW("[%s] the function don't supprot, becuase MTKCAM_HAVE_MFB_SUPPORT < 1",__FUNCTION__);
    IImageBuffer* pTemp =  pInputRrzoBufferHeap->createImageBuffer();
    pOutputBufferHeap = pTemp->getImageBufferHeap();
    pTemp = nullptr;
#endif  //end of #if MTKCAM_HAVE_MFB_SUPPORT >= 1
    CAM_LOGD("[%s] downSampleImageBufferHeap -",__FUNCTION__);
    return ret;
}

MERROR
IspTuningDataPackUtil::
packIspTuningDataToHeap(
        android::sp<IImageBufferHeap> pOututImageBufferHeap,    //output
        MINT32& totalUsedBytes,    //output
        PackInputParam & inputParam
        )
{
    CAM_TRACE_CALL();
    CAM_LOGD("[%s] +",__FUNCTION__);
    MERROR ret = OK;
    if(!pOututImageBufferHeap.get())
    {
        CAM_LOGE("[%s] pOututImageBufferHeap is NULL!",__FUNCTION__);
        CAM_LOGD("[%s] -",__FUNCTION__);
        return NO_MEMORY;
    }
    //
    int iDebugLevel = ::property_get_int32("vendor.debug.camera.IspTuningDataPackUtil", 1);
    //
    MINTPTR pOutBufAddr = (MINTPTR)pOututImageBufferHeap->getBufVA(0);
    isp_tuning_data_info *pInfo = (isp_tuning_data_info*)pOutBufAddr;
    size_t planes = pOututImageBufferHeap->getPlaneCount();
    size_t headerSize = sizeof(*pInfo);
    //
    pInfo->requestNo = inputParam.requestNo;
    pInfo->frameNo = inputParam.frameNo;
    pInfo->timestamp = inputParam.timestamp;
    pInfo->haltimestamp = inputParam.haltimestamp;
    //
    size_t outImgSizeInBytes = 0;
    for (size_t i=0; i<planes; i++)
    {
        CAM_LOGD("[%s] plane(%zu/%zu) format(0x%X) size(%dx%d) getBufVA(%" PRIxPTR ") getBufSizeInBytes(%zu) getBufStridesInBytes(%zu)", __FUNCTION__,
            i, planes,
            pOututImageBufferHeap->getImgFormat(),
            pOututImageBufferHeap->getImgSize().w, pOututImageBufferHeap->getImgSize().h,
            pOututImageBufferHeap->getBufVA(i),
            pOututImageBufferHeap->getBufSizeInBytes(i),
            pOututImageBufferHeap->getBufStridesInBytes(i)
            );
        outImgSizeInBytes+=pOututImageBufferHeap->getBufSizeInBytes(i);
    }
    //
    size_t nowAvailBufSize = outImgSizeInBytes;
    MINTPTR pNowOutBufPtr = (MINTPTR)pOutBufAddr;
    ssize_t metaFlattenRet =0;
    size_t flattenRet =0;
    MUINT32 checkSumRet = 0;
    MINTPTR pRawBufPtr = 0;
    size_t rawSize = 0;
    //
    ///////////////////////////////////////////////////////
#undef  CALCU_AVAIL_SIZE
#define CALCU_AVAIL_SIZE(_usedsize_, _msg_) \
        CAM_LOGD("[%s] CALCU_AVAIL_SIZE: (before)nowAvailBufSize(%zu) usedsize(%zu) (after)nowAvailBufSize(%zu) msg(%s)", __FUNCTION__, nowAvailBufSize, _usedsize_, (nowAvailBufSize-_usedsize_), _msg_);    \
        mInsufficientBufferSize = 0;\
        if  ( CC_UNLIKELY( nowAvailBufSize < _usedsize_ ))  {   \
            CAM_LOGE("[%s] (%s) available size is not enough (nowAvailBufSize(%zu) <  usedSize(%zu))", \
                __FUNCTION__, _msg_, nowAvailBufSize, _usedsize_); \
            mInsufficientBufferSize = _usedsize_;   \
            ret = NO_MEMORY;    \
            goto lbExit;    \
        }   \
        nowAvailBufSize -= _usedsize_;
    ///////////////////////////////////////////////////////
#undef  MOVE_OUT_BUF_PTR
#define MOVE_OUT_BUF_PTR(_offset_, _msg_)    \
        CAM_LOGD("[%s] MOVE_OUT_BUF_PTR: (before move)pNowOutBufPtr(%" PRIxPTR ") offset(%zu) (after move)pNowOutBufPtr(%" PRIxPTR ") msg(%s)", __FUNCTION__, pNowOutBufPtr, _offset_, (pNowOutBufPtr+_offset_), _msg_);    \
        pNowOutBufPtr+=_offset_;    \
        if  ( CC_UNLIKELY((pNowOutBufPtr - pOutBufAddr) >= outImgSizeInBytes) ) { \
            CAM_LOGE("[%s] (%s) is out of output buffer addr range : (pNowOutBufPtr(%" PRIxPTR ")-pOutBufAddr(%" PRIxPTR "))>= outImgSizeInBytes(%zu)", \
                __FUNCTION__, _msg_, pNowOutBufPtr, pOutBufAddr, outImgSizeInBytes); \
            ret = UNKNOWN_ERROR;    \
            goto lbExit;    \
        }
    ///////////////////////////////////////////////////////
#undef  CHECK_RAW_SIZE
#define CHECK_RAW_SIZE(_rawsize_, _availsize_, _msg_) \
        CAM_LOGD("[%s] CHECK_RAW_SIZE(%s): raw_size(%zu) avail_size(%zu) (raw_size must be smaller than avail_size)", __FUNCTION__, _msg_, (size_t)_rawsize_, (size_t)_availsize_);    \
        mInsufficientBufferSize = 0;\
        if  ( CC_UNLIKELY(_rawsize_ >= _availsize_) ) { \
            CAM_LOGE("[%s] (%s) output size is not enough : raw size(%zu) > available output buffer size(%zu)", \
                __FUNCTION__, _msg_, (size_t)_rawsize_, (size_t)_availsize_); \
            mInsufficientBufferSize = _rawsize_;   \
            ret = NO_MEMORY;    \
            goto lbExit;    \
        }
#ifdef MTKCAM_METADATA_V2
    //for Metadata V2 method
#define FLATTEN_META(_meta_,_whiteList_,_msg_)    \
        CAM_TRACE_BEGIN(android::String8::format("FLATTEN_META(%s)", _msg_));   \
        mInsufficientBufferSize = 0;    \
        flattenRet = 0; \
        if(_whiteList_.size() > 0)  \
        {   \
            std::vector<IMetadata::Tag_t> V(_whiteList_.cbegin(),_whiteList_.cend());  \
            IMetadata::Tag_t *whitelist = V.data();  \
            metaFlattenRet = _meta_.flatten((void*)pNowOutBufPtr, nowAvailBufSize, whitelist, _whiteList_.size()); \
        }   \
        else    \
        {   \
            metaFlattenRet = _meta_.flatten((void*)pNowOutBufPtr, nowAvailBufSize); \
        }   \
        CAM_LOGD("[%s] FLATTEN_META: (%s) pNowOutBufPtr(%" PRIxPTR ") nowAvailBufSize(%zu) meta entry count(%u) _whiteList_Size(%zu) flattenRet(%zu)", \
            __FUNCTION__, _msg_, pNowOutBufPtr, nowAvailBufSize, _meta_.count(),(size_t)_whiteList_.size(),flattenRet);    \
        if (metaFlattenRet < 0) {   \
            CAM_LOGE("[%s] (%s) flatten fail: ret=%zd, pNowOutBufPtr=%" PRIxPTR ", nowAvailBufSize=%zu",  \
                    __FUNCTION__,_msg_,  \
                    metaFlattenRet, \
                    pNowOutBufPtr,  \
                    nowAvailBufSize);   \
            mInsufficientBufferSize = 1 * 1024 * 1024;  \
            ret = NO_MEMORY;    \
            goto lbExit;    \
        }   \
        flattenRet = (size_t)metaFlattenRet;    \
        if (iDebugLevel >= 2)   \
        {   \
            CAM_LOGD("[%s] FLATTEN_META: DUMP_META to log: (%s) +", __FUNCTION__, _msg_); \
            _meta_.dump();  \
            CAM_LOGD("[%s] FLATTEN_META: DUMP_META to log: (%s) -", __FUNCTION__, _msg_); \
        }   \
        CAM_TRACE_END();
#else
    //for Metadata V1 method
#define FLATTEN_META(_meta_,_whiteList_,_msg_)    \
        flattenRet = _meta_.flatten((void*)pNowOutBufPtr, nowAvailBufSize, _whiteList_); \
        CAM_LOGD("[%s] FLATTEN_META: (%s) pNowOutBufPtr(%" PRIxPTR ") nowAvailBufSize(%zu) meta entry count(%u) _whiteList_Size(%zu) flattenRet(%zu)", \
            __FUNCTION__, _msg_, pNowOutBufPtr, nowAvailBufSize, _meta_.count(),(size_t)_whiteList_.size(),flattenRet);    \
        if (flattenRet < 0) {   \
            CAM_LOGE("[%s] (%s) flatten fail: ret=%zd, pNowOutBufPtr=%" PRIxPTR ", nowAvailBufSize=%zu",  \
                    __FUNCTION__,_msg_,  \
                    flattenRet, \
                    pNowOutBufPtr,  \
                    nowAvailBufSize);   \
            ret = UNKNOWN_ERROR;    \
            goto lbExit;    \
        }   \
        if (iDebugLevel >= 2)   \
        {   \
            CAM_LOGD("[%s] DUMP_META: (%s) +", __FUNCTION__, _msg_); \
            android::String8 path;  \
            path = android::String8::format("/data/vendor/camera_dump/Meta_%" PRId64 "_%d_%d_%s_BeforePack.dump",pInfo->timestamp,pInfo->requestNo,pInfo->frameNo,_msg_);   \
            DUMP_META(_meta_,path.string());    \
            CAM_LOGD("[%s] DUMP_META: (%s) -", __FUNCTION__, _msg_); \
        }
#endif
    ///////////////////////////////////////////////////////
#undef  CALCU_CHECK_SUM
#define CALCU_CHECK_SUM(_buf_,_size_,_msg_)    \
        CAM_LOGD("[%s] CALCU_CHECK_SUM: (%s) buf:(%" PRIxPTR ") size:(%zu) +", __FUNCTION__, _msg_, _buf_, _size_);    \
        checkSumRet = IspTuningDataPackUtil::calcuCheckSum((char *)_buf_,_size_); \
        CAM_LOGD("[%s] CALCU_CHECK_SUM: (%s) checkSumRet(%u) -", __FUNCTION__, _msg_, checkSumRet);
    ///////////////////////////////////////////////////////
#undef  DUMP_BUF
#define DUMP_BUF(_buf_,_path_) \
        CAM_LOGD("[%s] DUMP_BUF: (%" PRIxPTR ") save to path:(%s) ", __FUNCTION__, _buf_, _path_);    \
        if (!(_buf_->saveToFile(_path_)))  \
            CAM_LOGE("[%s] DUMP_BUF: save to path:(%s) fail!", __FUNCTION__, _path_);    \
    ///////////////////////////////////////////////////////
#undef COPY_RAW
#define COPY_RAW(_heap_,_msg_,_size_,_count_)    \
        CAM_TRACE_BEGIN(android::String8::format("COPY_RAW(%s)", _msg_));   \
        CAM_LOGD("[%s] COPY_RAW: (%s)", __FUNCTION__, _msg_);    \
        _size_ = 0;                          \
        MINTPTR _pOutBufPtr_ = pNowOutBufPtr; \
        for (int idx = 0; idx < _count_; idx++)   \
        {                                     \
            pRawBufPtr = _heap_->getBufVA(idx);  \
            _size_ += _heap_->getBufSizeInBytes(idx);  \
            CHECK_RAW_SIZE(_size_, nowAvailBufSize,_msg_); \
            CAM_LOGD("[%s] memcpy(dst(%" PRIxPTR "),src(%" PRIxPTR "),length(%zu))", __FUNCTION__, _pOutBufPtr_, pRawBufPtr, _heap_->getBufSizeInBytes(idx));    \
            memcpy((void*)_pOutBufPtr_,(void*)pRawBufPtr,_heap_->getBufSizeInBytes(idx)); \
            _pOutBufPtr_ += _heap_->getBufSizeInBytes(idx); \
        }                                     \
        flattenRet = _size_;    \
        if (iDebugLevel >= 2)   \
        {   \
            CAM_LOGD("[%s] DUMP_BUF: (%s) +", __FUNCTION__, _msg_); \
            android::String8 path;  \
            path = android::String8::format("/data/vendor/camera_dump/Buf_%" PRId64 "_%d_%d_%s_BeforePack.dump",pInfo->timestamp,pInfo->requestNo,pInfo->frameNo,_msg_);   \
            IImageBuffer *pImgBuf = _heap_->createImageBuffer();  \
            DUMP_BUF((MINTPTR)pImgBuf,path.string()); \
            CAM_LOGD("[%s] DUMP_BUF: (%s) -", __FUNCTION__, _msg_); \
        }   \
        CAM_TRACE_END();
    ///////////////////////////////////////////////////////
#undef  COPY_PRIVATE
#define COPY_PRIVATE(_buf_,_size_,_msg_) \
    CAM_TRACE_BEGIN(android::String8::format("COPY_RAW(%s)", _msg_));   \
    CAM_LOGD("[%s] COPY_PRIVATE[%s]: buf(%p) size(%u) ", __FUNCTION__, _msg_, _buf_, _size_);    \
    CHECK_RAW_SIZE(_size_, nowAvailBufSize,_msg_); \
    memcpy((void*)pNowOutBufPtr,(void*)_buf_,_size_); \
    flattenRet = _size_;    \
    CAM_TRACE_END();
    ///////////////////////////////////////////////////////
    //
    CAM_LOGD("[%s] requestNo(%d) frameNo(%d) timestamp(%" PRId64 ") pOutBufAddr(%" PRIxPTR ") outImgSizeInBytes(%zu) nowAvailBufSize(%zu) headerSize(%zu)",
        __FUNCTION__, inputParam.requestNo, inputParam.frameNo, inputParam.timestamp,
        pOutBufAddr, outImgSizeInBytes, nowAvailBufSize, headerSize);
    //
    std::set<IMetadata::Tag_t> appControlMetaFinalWhiteList;
    std::set<IMetadata::Tag_t> appResultMetaFinalWhiteList;
    std::set<IMetadata::Tag_t> halResultMetaFinalWhiteList;
    std::set<IMetadata::Tag_t> ispTuningCtrlAppWhiteList; // TODO: query isp tuning white list from ISP hal
    std::set<IMetadata::Tag_t> ispTuningResultAppWhiteList; // TODO: query isp tuning white list from ISP hal
    std::set<IMetadata::Tag_t> ispTuningResultHalWhiteList; // TODO: query isp tuning white list from ISP hal
    MBOOL needBssDownSample = 0;
    MUINT8 needPackRrzo = 0;
    MINT32 needPackFeo = 0;
    int forcePackRrzoFlow = ::property_get_int32("vendor.debug.camera.IspTuningDataPackUtil.packRrzo", -1);
    int forceDownSampleFlow = ::property_get_int32("vendor.debug.camera.IspTuningDataPackUtil.DownSample", -1);
    int forcePackFeoFlow = ::property_get_int32("vendor.debug.camera.IspTuningDataPackUtil.packFeo", -1);
    android::sp<IImageBufferHeap> pFinalRrzoImageBufferHeap = nullptr;
    bool bIsFinalRrzoHeapNeedLockByPackUtils = false;
    MERROR err = OK;
    /////////////////////////////////////////
    // RRZO Pre-Process flow

    // save original RRZO info to metadata
    if(inputParam.pRrzoImageBufferHeap != nullptr)
    {
        BSScore::ZipOutData zipData;
        MUINT32 ZipDataHeaderSize = sizeof(zipData);
        MUINT32 ZipTotalDataSize = ZipDataHeaderSize;
        memset(&zipData, 0, ZipDataHeaderSize);
        zipData.imgWidth = inputParam.pRrzoImageBufferHeap->getImgSize().w;
        zipData.imgHeight = inputParam.pRrzoImageBufferHeap->getImgSize().h;
        zipData.imgFormat = inputParam.pRrzoImageBufferHeap->getImgFormat();
        zipData.imgPlaneCount = inputParam.pRrzoImageBufferHeap->getPlaneCount();
        for(auto i=0; i<inputParam.pRrzoImageBufferHeap->getPlaneCount(); i++)
        {
            zipData.imgStride[i] = inputParam.pRrzoImageBufferHeap->getBufStridesInBytes(i);
            zipData.imgSize[i] = inputParam.pRrzoImageBufferHeap->getBufSizeInBytes(i);
        }
        IMetadata::getEntry<MINT32>(&inputParam.appControlMeta, MTK_CONTROL_CAPTURE_HINT_FOR_ISP_FRAME_INDEX, zipData.frameIndex);
        CAM_LOGD("[%s] zipData: width(%d) height(%d) PlaneCount(%d) format(0x%X) stride(%d,%d,%d) size(%d,%d,%d) frameIndex(%d) total size(%d)",__FUNCTION__,
                        zipData.imgWidth,zipData.imgHeight,zipData.imgPlaneCount,zipData.imgFormat,
                        zipData.imgStride[0],zipData.imgStride[1],zipData.imgStride[2],
                        zipData.imgSize[0],zipData.imgSize[1],zipData.imgSize[2],zipData.frameIndex,ZipTotalDataSize);
        IMetadata::Memory RrzoData;
        CAM_LOGD("[%s] RrzoData.resize(%u)",__FUNCTION__,ZipTotalDataSize);
        RrzoData.resize(ZipTotalDataSize);
        CAM_LOGD("[%s] memcpy header (des:%p, src:%p, len:%u)",__FUNCTION__,(void *)RrzoData.editArray(), (void *)(&zipData), ZipDataHeaderSize);
        memcpy((void *)RrzoData.editArray(), (void *)(&zipData), ZipDataHeaderSize);
        IMetadata::setEntry<IMetadata::Memory>(&inputParam.halResultMeta, MTK_FEATURE_BSS_RRZO_DATA, RrzoData);
        CAM_LOGD("[%s] update MTK_FEATURE_BSS_RRZO_DATA done",__FUNCTION__);
    }
    else
    {
        CAM_LOGD("[%s] pRrzoImageBufferHeap is nullptr, no need to save rrzo info",__FUNCTION__);
    }

    // decide pack rrzo strategy
    if(!IMetadata::getEntry<MBOOL>(&inputParam.halResultMeta, MTK_FEATURE_BSS_DOWNSAMPLE, needBssDownSample))
    {
        needBssDownSample = 0;
        CAM_LOGD("[%s] MTK_FEATURE_BSS_DOWNSAMPLE is empty",__FUNCTION__);
    }
    //
    if(!IMetadata::getEntry<MINT32>(&inputParam.halResultMeta, MTK_ISP_FEO_ENABLE, needPackFeo))
    {
        needPackFeo = 0;
        CAM_LOGD("[%s] MTK_ISP_FEO_ENABLE is empty",__FUNCTION__);
    }
    //
    if(inputParam.pRrzoImageBufferHeap != nullptr)
    {
        if(!IMetadata::getEntry<MUINT8>(&inputParam.halResultMeta, MTK_FEATURE_PACK_RRZO, needPackRrzo))
        {
            needPackRrzo = 0;
            CAM_LOGD("[%s] MTK_FEATURE_PACK_RRZO is empty",__FUNCTION__);
        }
    }
    else
    {
        needPackRrzo = 0;
        CAM_LOGD("[%s] NO rrzo, so don't pack rrzo",__FUNCTION__);
    }
    // for debug
    {
        if(forcePackRrzoFlow > 0)
        {
            needPackRrzo = 1;
            if(inputParam.pRrzoImageBufferHeap == nullptr)
            {
                needPackRrzo = 0;
                CAM_LOGW("[%s] rrzo is null, can't force pack rrzo",__FUNCTION__);
            }
        }
        else if(forcePackRrzoFlow == 0)
        {
            needPackRrzo = 0;
        }
        //
        if(forceDownSampleFlow > 0)
        {
            needBssDownSample = 1;
            IMetadata::setEntry<MBOOL>(&inputParam.halResultMeta, MTK_FEATURE_BSS_DOWNSAMPLE, MTRUE);
        }
        else if(forceDownSampleFlow == 0)
        {
            needBssDownSample = 0;
            IMetadata::setEntry<MBOOL>(&inputParam.halResultMeta, MTK_FEATURE_BSS_DOWNSAMPLE, MFALSE);
        }
        //
        if(forcePackFeoFlow > 0)
        {
            needPackFeo = 1;
        }
        else if(forcePackFeoFlow == 0)
        {
            needPackFeo = 0;
        }
        CAM_LOGD("[%s] forcePackRrzoFlow(%d) forceDownSampleFlow(%d) forcePackFeoFlow(%d)",__FUNCTION__,forcePackRrzoFlow,forceDownSampleFlow,forcePackFeoFlow);
    }
    CAM_LOGD("[%s] bssDownsample(%d) needPackFeo(%d) needPackRrzo(%d)",__FUNCTION__,needBssDownSample,needPackFeo,needPackRrzo);
    // process rrzo
    if(needPackRrzo > 0)
    {
        CAM_LOGD("[%s] pRrzoImageBufferHeap w(%d) h(%d) fmt(0x%X)",__FUNCTION__,
            inputParam.pRrzoImageBufferHeap->getImgSize().w,
            inputParam.pRrzoImageBufferHeap->getImgSize().h,
            inputParam.pRrzoImageBufferHeap->getImgFormat());
        for(auto i=0;i<inputParam.pRrzoImageBufferHeap->getPlaneCount();i++)
        {
            CAM_LOGD("[%s] pRrzoImageBufferHeap plane:(%d/%zu) stride[%d](%zu) size[%d](%zu) VA[%d](%" PRIxPTR ")",__FUNCTION__,
                i,inputParam.pRrzoImageBufferHeap->getPlaneCount(),
                i,inputParam.pRrzoImageBufferHeap->getBufStridesInBytes(i),
                i,inputParam.pRrzoImageBufferHeap->getBufSizeInBytes(i),
                i,inputParam.pRrzoImageBufferHeap->getBufVA(i));
        }
        //
        if(needPackFeo > 0)
        {
            err = packFeoDataToHalMeta(inputParam.pRrzoImageBufferHeap,inputParam.appControlMeta,inputParam.halResultMeta);
            if(err != OK)
            {
                CAM_LOGE("[%s] packFeoData() failed!",__FUNCTION__);
            }
        }
        //
        if(needBssDownSample > 0)
        {
            err = downSampleImageBufferHeap(inputParam.pRrzoImageBufferHeap,inputParam.pRrzoStreamInfo,pFinalRrzoImageBufferHeap,inputParam.appControlMeta,inputParam.halResultMeta);
            if(err != OK)
            {
                CAM_LOGE("[%s] downSampleImageBufferHeap() failed!(%d) To use original image",__FUNCTION__,err);
                pFinalRrzoImageBufferHeap = inputParam.pRrzoImageBufferHeap;
                bIsFinalRrzoHeapNeedLockByPackUtils = false;    //RRZO has already locked by IspTuningDataNode
            }
            else
            {
                bIsFinalRrzoHeapNeedLockByPackUtils = true; //new buffer need be locked by PackUtils
            }
        }
        else
        {
            pFinalRrzoImageBufferHeap = inputParam.pRrzoImageBufferHeap;
            bIsFinalRrzoHeapNeedLockByPackUtils = false;    //RRZO has already locked by IspTuningDataNode
        }
        //
        if(bIsFinalRrzoHeapNeedLockByPackUtils)
        {
            pFinalRrzoImageBufferHeap->lockBuf("FinalRrzo", eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_READ_OFTEN);
        }
        // Log
        {
            CAM_LOGD("[%s] pFinalRrzoImageBufferHeap w(%d) h(%d) fmt(0x%X)",__FUNCTION__,
            pFinalRrzoImageBufferHeap->getImgSize().w,
            pFinalRrzoImageBufferHeap->getImgSize().h,
            pFinalRrzoImageBufferHeap->getImgFormat());
            //
            for(auto i=0;i<pFinalRrzoImageBufferHeap->getPlaneCount();i++)
            {
                CAM_LOGD("[%s] pFinalRrzoImageBufferHeap plane:(%d/%zu) stride[%d](%zu) size[%d](%zu) VA[%d](%" PRIxPTR ")",__FUNCTION__,
                    i,pFinalRrzoImageBufferHeap->getPlaneCount(),
                    i,pFinalRrzoImageBufferHeap->getBufStridesInBytes(i),
                    i,pFinalRrzoImageBufferHeap->getBufSizeInBytes(i),
                    i,pFinalRrzoImageBufferHeap->getBufVA(i));
            }
        }
    }
    /////////////////////////////////////////
    // get metadata white list
    IspTuningDataFilter dataFilter;
    if(dataFilter.getTuningDataWhiteList(
            appControlMetaFinalWhiteList,
            appResultMetaFinalWhiteList,
            halResultMetaFinalWhiteList,
            ispTuningCtrlAppWhiteList,
            ispTuningResultAppWhiteList,
            ispTuningResultHalWhiteList) != OK )
    {
        CAM_LOGW("[%s] getTuningDataWhiteList fail!",__FUNCTION__);
    }
    /////////////////////////////////////////
    // Control App Metadata
#define CTRL_APP_META "Control_APP_Metadata"
    MOVE_OUT_BUF_PTR(headerSize,CTRL_APP_META);
    CALCU_AVAIL_SIZE(headerSize,CTRL_APP_META);
    FLATTEN_META(inputParam.appControlMeta,appControlMetaFinalWhiteList,CTRL_APP_META);
    CALCU_CHECK_SUM(pNowOutBufPtr,flattenRet,CTRL_APP_META);
    pInfo->metaInfo[CONTROL_APP].length = flattenRet;
    pInfo->metaInfo[CONTROL_APP].offset = (pNowOutBufPtr-pOutBufAddr);
    pInfo->metaInfo[CONTROL_APP].checkSum = checkSumRet;
    //
    CAM_LOGD("[%s] pNowOutBufPtr(%" PRIxPTR ") metaInfo[CONTROL_APP] offset(%zu) length(%zu) checksum(%u)",
        __FUNCTION__, pNowOutBufPtr, pInfo->metaInfo[CONTROL_APP].offset, pInfo->metaInfo[CONTROL_APP].length, pInfo->metaInfo[CONTROL_APP].checkSum);
#undef CTRL_APP_META
    //
    /////////////////////////////////////////
    // App Result Metadata
#define APP_RESULT_META "APP_Result_Metadata"
    MOVE_OUT_BUF_PTR(flattenRet,APP_RESULT_META);
    CALCU_AVAIL_SIZE(flattenRet,APP_RESULT_META);
    FLATTEN_META(inputParam.appResultMeta,appResultMetaFinalWhiteList,APP_RESULT_META);
    CALCU_CHECK_SUM(pNowOutBufPtr,flattenRet,APP_RESULT_META);
    pInfo->metaInfo[APP_RESULT].length = flattenRet;
    pInfo->metaInfo[APP_RESULT].offset = (pNowOutBufPtr-pOutBufAddr);
    pInfo->metaInfo[APP_RESULT].checkSum = checkSumRet;
    //
    CAM_LOGD("[%s] pNowOutBufPtr(%" PRIxPTR ") metaInfo[APP_RESULT] offset(%zu) length(%zu) checksum(%u)",
        __FUNCTION__, pNowOutBufPtr, pInfo->metaInfo[APP_RESULT].offset, pInfo->metaInfo[APP_RESULT].length, pInfo->metaInfo[APP_RESULT].checkSum);
#undef APP_RESULT_META
    //
    /////////////////////////////////////////
    // Hal Result Metadata
#define HAL_RESULT_META "HAL_Result_Metadata"
    MOVE_OUT_BUF_PTR(flattenRet,HAL_RESULT_META);
    CALCU_AVAIL_SIZE(flattenRet,HAL_RESULT_META);
    FLATTEN_META(inputParam.halResultMeta,halResultMetaFinalWhiteList,HAL_RESULT_META);
    CALCU_CHECK_SUM(pNowOutBufPtr,flattenRet,HAL_RESULT_META);
    pInfo->metaInfo[HAL_RESULT].length = flattenRet;
    pInfo->metaInfo[HAL_RESULT].offset = (pNowOutBufPtr-pOutBufAddr);
    pInfo->metaInfo[HAL_RESULT].checkSum = checkSumRet;
    //
    CAM_LOGD("[%s] pNowOutBufPtr(%" PRIxPTR ") metaInfo[HAL_RESULT] offset(%zu) length(%zu) checksum(%u)",
        __FUNCTION__, pNowOutBufPtr, pInfo->metaInfo[HAL_RESULT].offset, pInfo->metaInfo[HAL_RESULT].length, pInfo->metaInfo[HAL_RESULT].checkSum);
#undef HAL_RESULT_META
    //
    /////////////////////////////////////////
    // FD data
#define HAL_FD_DATA "HAL_FD_DATA"
    MOVE_OUT_BUF_PTR(flattenRet,HAL_FD_DATA);
    CALCU_AVAIL_SIZE(flattenRet,HAL_FD_DATA);
    COPY_PRIVATE(inputParam.pFDData, inputParam.FDDataSize,HAL_FD_DATA);
    CALCU_CHECK_SUM(pNowOutBufPtr,flattenRet,HAL_FD_DATA);
    pInfo->privateInfo[FD_DATA].length = flattenRet;
    pInfo->privateInfo[FD_DATA].offset = (pNowOutBufPtr-pOutBufAddr);
    pInfo->privateInfo[FD_DATA].checkSum = checkSumRet;
    //
    CAM_LOGD("[%s] pNowOutBufPtr(%" PRIxPTR ") privateInfo[FD_DATA] offset(%zu) length(%zu) checksum(%u)",
        __FUNCTION__, pNowOutBufPtr, pInfo->privateInfo[FD_DATA].offset, pInfo->privateInfo[FD_DATA].length, pInfo->privateInfo[FD_DATA].checkSum);
#undef HAL_FD_DATA
    //
    /////////////////////////////////////////
    // STTO
#define STTO_BUF "STTO_Buffer"
    if(inputParam.pSttoImageBufferHeap.get())
    {
        MOVE_OUT_BUF_PTR(flattenRet,STTO_BUF);
        CALCU_AVAIL_SIZE(flattenRet,STTO_BUF);
        COPY_RAW(inputParam.pSttoImageBufferHeap,STTO_BUF, rawSize, 1);
        CALCU_CHECK_SUM(pNowOutBufPtr,rawSize,STTO_BUF);
        pInfo->rawInfo[STTO_RAW].format = inputParam.pSttoImageBufferHeap->getImgFormat();
        pInfo->rawInfo[STTO_RAW].size = inputParam.pSttoImageBufferHeap->getImgSize();
        pInfo->rawInfo[STTO_RAW].stride_in_bytes[0] = inputParam.pSttoImageBufferHeap->getBufStridesInBytes(0);
        pInfo->rawInfo[STTO_RAW].plane_size[0] = inputParam.pSttoImageBufferHeap->getBufSizeInBytes(0);
        pInfo->rawInfo[STTO_RAW].offset = (pNowOutBufPtr-pOutBufAddr);
        pInfo->rawInfo[STTO_RAW].planeCount = inputParam.pSttoImageBufferHeap->getPlaneCount();
        pInfo->rawInfo[STTO_RAW].length = rawSize;
        pInfo->rawInfo[STTO_RAW].checkSum = checkSumRet;
        //
        CAM_LOGD("[%s] pNowOutBufPtr(%" PRIxPTR ") rawInfo[STTO_RAW]: offset(%zu) length(%zu) format(0x%x) size(%dx%d) stride(%zu) planeCount(%zu) checksum(%u)",
            __FUNCTION__, pNowOutBufPtr, pInfo->rawInfo[STTO_RAW].offset, pInfo->rawInfo[STTO_RAW].length, pInfo->rawInfo[STTO_RAW].format,
            pInfo->rawInfo[STTO_RAW].size.w, pInfo->rawInfo[STTO_RAW].size.h, pInfo->rawInfo[STTO_RAW].stride_in_bytes[0],
            pInfo->rawInfo[STTO_RAW].planeCount, pInfo->rawInfo[STTO_RAW].checkSum);
        }
    else
    {
        pInfo->rawInfo[STTO_RAW].offset = 0;
        pInfo->rawInfo[STTO_RAW].length = 0;
        CAM_LOGD("[%s] STTO buffer is empty", __FUNCTION__);
    }
#undef STTO_BUF
    /////////////////////////////////////////
    // RRZO
#define RRZO_BUF "RRZO_Buffer"
    if(needPackRrzo > 0 && pFinalRrzoImageBufferHeap != nullptr)
    {
        //
        MOVE_OUT_BUF_PTR(flattenRet,RRZO_BUF);
        CALCU_AVAIL_SIZE(flattenRet,RRZO_BUF);
        COPY_RAW(pFinalRrzoImageBufferHeap,RRZO_BUF, rawSize, pFinalRrzoImageBufferHeap->getPlaneCount());
        CALCU_CHECK_SUM(pNowOutBufPtr,rawSize,RRZO_BUF);
        pInfo->rawInfo[RRZO_RAW].format = pFinalRrzoImageBufferHeap->getImgFormat();
        pInfo->rawInfo[RRZO_RAW].size = pFinalRrzoImageBufferHeap->getImgSize();
        for (int plane = 0; plane < pFinalRrzoImageBufferHeap->getPlaneCount(); plane++)
        {
            pInfo->rawInfo[RRZO_RAW].stride_in_bytes[plane] = pFinalRrzoImageBufferHeap->getBufStridesInBytes(plane);
            pInfo->rawInfo[RRZO_RAW].plane_size[plane] = pFinalRrzoImageBufferHeap->getBufSizeInBytes(plane);
        }
        pInfo->rawInfo[RRZO_RAW].offset = (pNowOutBufPtr-pOutBufAddr);
        pInfo->rawInfo[RRZO_RAW].planeCount = pFinalRrzoImageBufferHeap->getPlaneCount();
        pInfo->rawInfo[RRZO_RAW].length = rawSize;
        pInfo->rawInfo[RRZO_RAW].checkSum = checkSumRet;
        //
        CAM_LOGD("[%s] pNowOutBufPtr(%" PRIxPTR ") rawInfo[RRZO_RAW]: offset(%zu) length(%zu) format(0x%x) size(%dx%d) stride(%zu) planeCount(%zu) checksum(%u)",
            __FUNCTION__, pNowOutBufPtr, pInfo->rawInfo[RRZO_RAW].offset, pInfo->rawInfo[RRZO_RAW].length, pInfo->rawInfo[RRZO_RAW].format,
            pInfo->rawInfo[RRZO_RAW].size.w, pInfo->rawInfo[RRZO_RAW].size.h, pInfo->rawInfo[RRZO_RAW].stride_in_bytes[0],
            pInfo->rawInfo[RRZO_RAW].planeCount, pInfo->rawInfo[RRZO_RAW].checkSum);
    }
    else
    {
        pInfo->rawInfo[RRZO_RAW].offset = 0;
        pInfo->rawInfo[RRZO_RAW].length = 0;
        CAM_LOGD("[%s] no need to pack RRZO", __FUNCTION__);
    }
#undef RRZO_BUF
    //
    /////////////////////////////////////////
    // TODO: add debug property to control IspTuningDataFilter on/off

    //calcu total used bytes
    pInfo->totalUsedBytes = headerSize +
        pInfo->metaInfo[CONTROL_APP].length + pInfo->metaInfo[APP_RESULT].length + pInfo->metaInfo[HAL_RESULT].length +
        pInfo->privateInfo[FD_DATA].length + pInfo->rawInfo[STTO_RAW].length + pInfo->rawInfo[RRZO_RAW].length;
    //dump total buffer
    if (iDebugLevel >= 2)
    {
        int total = pInfo->totalUsedBytes;
        CAM_LOGD("[%s] DUMP_TOTAL: totalUsedBytes(%d) +", __FUNCTION__,total);
        android::String8 path;
        path = android::String8::format("/data/vendor/camera_dump/Total_%" PRId64 "_%d_%d_BeforePack.dump",pInfo->timestamp,pInfo->requestNo,pInfo->frameNo);
        FILE *myFptr=NULL;
        std::unique_ptr<char[]> chunk(new char[total]);
        myFptr = fopen(path.string(), "wb");
        if(myFptr != NULL)
        {
            ::memset(chunk.get(),0,total);
            memcpy((void*)chunk.get(),(void*)pOutBufAddr,total);
            fwrite(chunk.get(),sizeof(char),total,myFptr);
            fclose(myFptr);
        }
        else
        {
            CAM_LOGE("[%s] DUMP_TOTAL: save to path:(%s) fopen fail!", __FUNCTION__, path.string());
        }
        CAM_LOGD("[%s] DUMP_TOTAL:-", __FUNCTION__);
    }


    lbExit:
    if(bIsFinalRrzoHeapNeedLockByPackUtils)
    {
        pFinalRrzoImageBufferHeap->unlockBuf("FinalRrzo");
    }
    //
    CAM_LOGD("[%s] totalUsedBytes(%d) -", __FUNCTION__,pInfo->totalUsedBytes);
    totalUsedBytes = pInfo->totalUsedBytes;
#undef CALCU_AVAIL_SIZE
#undef MOVE_OUT_BUF_PTR
#undef CHECK_RAW_SIZE
#undef FLATTEN_META
#undef COPY_RAW
#undef DUMP_BUF
#undef CALCU_CHECK_SUM
    return ret;
}

MERROR
IspTuningDataPackUtil::
unpackIspTuningDataFromHeap(
        android::sp<IImageBufferHeap> const pInputImageBufferHeap,    //input
        UnpackOutputParam& outputParam
        )
{
    CAM_TRACE_CALL();
    CAM_LOGD("[%s] +", __FUNCTION__);
    MERROR ret = OK;
    if(!pInputImageBufferHeap.get())
    {
        CAM_LOGE("[%s] pInputImageBufferHeap is NULL!", __FUNCTION__);
        CAM_LOGD("[%s] -", __FUNCTION__);
        return NO_MEMORY;
    }
    //
    int iDebugLevel = ::property_get_int32("vendor.debug.camera.IspTuningDataPackUtil", 0);
    //
    MUINT32 checkSumRet = 0;
    ///////////////////////////////////////////////////////
#undef  CALCU_CHECK_SUM
#define CALCU_CHECK_SUM(_buf_,_size_,_msg_)    \
        CAM_LOGD("[%s] CALCU_CHECK_SUM: (%s) buf:(%" PRIxPTR ") size:(%zu) +", __FUNCTION__, _msg_, _buf_, _size_);    \
        checkSumRet = IspTuningDataPackUtil::calcuCheckSum((char *)_buf_,_size_); \
        CAM_LOGD("[%s] CALCU_CHECK_SUM: (%s) checkSumRet:(%u) -", __FUNCTION__, _msg_, checkSumRet);
    ///////////////////////////////////////////////////////
#undef UNFLATTEN_META
#define UNFLATTEN_META(_meta_,_srcMetaOffset_,_srcMetaLen_,_checkSum_,_msg_) \
        CAM_TRACE_BEGIN(android::String8::format("UNFLATTEN_META(%s)", _msg_));   \
        pTargetPtr = pInBufAddr+_srcMetaOffset_;    \
        unflattenRet = _meta_.unflatten((void*)pTargetPtr,_srcMetaLen_);    \
        CAM_LOGD("[%s] UNFLATTEN_META: pTargetPtr(%" PRIxPTR ") _srcMetaOffset_(%zu) _srcMetaLen_(%zu) _checkSum_(%u) unflattenRet(%zu) msg(%s)", \
            __FUNCTION__, pTargetPtr, _srcMetaOffset_, _srcMetaLen_,_checkSum_,unflattenRet,_msg_);    \
        if(unflattenRet!=_srcMetaLen_)   \
        {   \
            CAM_LOGE("[%s] unflattenRet(%zu)!=srcMetaLen(%zu)", __FUNCTION__, unflattenRet, _srcMetaLen_);  \
            ret = UNKNOWN_ERROR;    \
            goto lbExit;    \
        }   \
        CALCU_CHECK_SUM(pTargetPtr,unflattenRet,_msg_); \
        if(_checkSum_!=checkSumRet) \
        {   \
            CAM_LOGE("[%s] _checkSum_(%u)!=checkSumRet(%u)", __FUNCTION__, _checkSum_, checkSumRet);  \
            ret = UNKNOWN_ERROR;    \
            goto lbExit;    \
        }   \
        if (iDebugLevel >= 2)   \
        {   \
            CAM_LOGD("[%s] UNFLATTEN_META: DUMP_META to log: (%s) +", __FUNCTION__, _msg_); \
            _meta_.dump();  \
            CAM_LOGD("[%s] UNFLATTEN_META: DUMP_META to log: (%s) -", __FUNCTION__, _msg_); \
        }   \
        CAM_TRACE_END();
    ///////////////////////////////////////////////////////
#undef  DUMP_BUF
#define DUMP_BUF(_buf_,_path_,_needLock_) \
        CAM_LOGD("[%s] DUMP_BUF: save to path:(%s) ", __FUNCTION__, _path_);    \
        if(_needLock_) _buf_->lockBuf("UnpackIspTuningData", eBUFFER_USAGE_SW_READ_MASK);  \
        if (!(_buf_->saveToFile(_path_)))  \
            CAM_LOGE("[%s] DUMP_BUF: save to path:(%s) fail!", __FUNCTION__, _path_);    \
        if(_needLock_) _buf_->unlockBuf("UnpackIspTuningData");
    ///////////////////////////////////////////////////////
#undef UNPACK_RAW
#define UNPACK_RAW(_buffer_,_raw_type,_checkSum_,_msg_)    \
    {   \
        CAM_TRACE_BEGIN(android::String8::format("UNFLATTEN_META(%s)", _msg_));   \
        size_t bufStridesInBytes[3] = {pInfo->rawInfo[_raw_type].stride_in_bytes[0], \
                                       pInfo->rawInfo[_raw_type].stride_in_bytes[1], \
                                       pInfo->rawInfo[_raw_type].stride_in_bytes[2]}; \
        size_t bufBoundaryInBytes[3] = {0, 0, 0};   \
        IImageBufferAllocator::ImgParam imgParam(   \
            pInfo->rawInfo[_raw_type].format, pInfo->rawInfo[_raw_type].size, \
            bufStridesInBytes, bufBoundaryInBytes, pInfo->rawInfo[_raw_type].planeCount  \
        );  \
        sp<IImageBufferHeap> pImageBufferHeap = IIonImageBufferHeap::create(    \
                            _msg_,   \
                            imgParam,   \
                            IIonImageBufferHeap::AllocExtraParam(), \
                            MFALSE  \
                        );  \
        if( pImageBufferHeap == NULL )  \
        {   \
            CAM_LOGE("[%s] (%s)IIonImageBufferHeap create failed", __FUNCTION__, _msg_);   \
            ret = UNKNOWN_ERROR;    \
            goto lbExit;    \
        }   \
        _buffer_ = pImageBufferHeap->createImageBuffer();   \
        _buffer_->lockBuf(_msg_,eBUFFER_USAGE_SW_READ_OFTEN|eBUFFER_USAGE_SW_WRITE_OFTEN);  \
        MINTPTR startAddr = (pInBufAddr+pInfo->rawInfo[_raw_type].offset); \
        for (int plane = 0; plane < pInfo->rawInfo[_raw_type].planeCount; plane++) \
        {   \
            MINTPTR outPtr = _buffer_->getBufVA(plane);   \
            CAM_LOGD("[%s] UNPACK_RAW(%s): memcpy(dst:%" PRIxPTR ", src:(%" PRIxPTR "+%zu)=(%" PRIxPTR "), len:%zu)", \
                __FUNCTION__, _msg_,outPtr,pInBufAddr,pInfo->rawInfo[_raw_type].offset,(MINTPTR)(pInBufAddr+pInfo->rawInfo[_raw_type].offset),pInfo->rawInfo[_raw_type].plane_size[plane]);   \
            memcpy((void*)outPtr,(void*)(pInBufAddr+pInfo->rawInfo[_raw_type].offset),pInfo->rawInfo[_raw_type].plane_size[plane]);  \
            pInfo->rawInfo[_raw_type].offset += pInfo->rawInfo[_raw_type].plane_size[plane]; \
        }   \
        CALCU_CHECK_SUM(startAddr,pInfo->rawInfo[_raw_type].length,_msg_); \
        if(_checkSum_!=checkSumRet) \
        {   \
            CAM_LOGE("[%s] _checkSum_(%u)!=checkSumRet(%u)", __FUNCTION__, _checkSum_, checkSumRet);  \
            ret = UNKNOWN_ERROR;    \
            goto lbExit;    \
        }   \
        _buffer_->unlockBuf(_msg_); \
        if (iDebugLevel >= 2)   \
        {   \
            CAM_LOGD("[%s] DUMP_BUF: (%s) +", __FUNCTION__, _msg_); \
            android::String8 path;  \
            path = android::String8::format("/data/vendor/camera_dump/Buf_%" PRId64 "_%d_%d_%s_AfterUnpack.dump",pInfo->timestamp,pInfo->requestNo,pInfo->frameNo,_msg_);   \
            DUMP_BUF(_buffer_,path.string(),true); \
            CAM_LOGD("[%s] DUMP_BUF: (%s) -", __FUNCTION__, _msg_); \
        }\
        CAM_TRACE_END();    \
    }
    ///////////////////////////////////////////////////////
#undef UNPACK_PRIVATE
#define UNPACK_PRIVATE(_pvecout_,_type_,_checkSum_,_msg_)    \
    { \
        CAM_TRACE_BEGIN(android::String8::format("UNFLATTEN_META(%s)", _msg_));   \
        _pvecout_ = std::make_shared<std::vector<MUINT8>>(); \
        _pvecout_->resize(pInfo->privateInfo[_type_].length); \
        memcpy(_pvecout_->data(), (void*)(pInBufAddr + pInfo->privateInfo[_type_].offset), pInfo->privateInfo[_type_].length); \
        CALCU_CHECK_SUM((MINTPTR)(pInBufAddr + pInfo->privateInfo[_type_].offset),pInfo->privateInfo[_type_].length,_msg_); \
        if(_checkSum_!=checkSumRet) \
        {   \
            CAM_LOGE("[%s][%s] _checkSum_(%u)!=checkSumRet(%u)", __FUNCTION__, _msg_, _checkSum_, checkSumRet);  \
            ret = UNKNOWN_ERROR;    \
            goto lbExit;    \
        }   \
        CAM_TRACE_END();    \
    }

    MINTPTR pInBufAddr = (MINTPTR)pInputImageBufferHeap->getBufVA(0);
    isp_tuning_data_info *pInfo = (isp_tuning_data_info*)pInBufAddr;
    //
    ssize_t unflattenRet = 0;
    MINTPTR pTargetPtr = 0;
    //
    CAM_LOGD("[%s] requestNo(%d) frameNo(%d) timestamp(%" PRId64 ") pInBufAddr(%" PRIxPTR ") totalUsedBytes(%d) headerSize(%zu)",
        __FUNCTION__, pInfo->requestNo, pInfo->frameNo, pInfo->timestamp,
        pInBufAddr, pInfo->totalUsedBytes , sizeof(*pInfo));
    //
    //dump total buffer
    if (iDebugLevel >= 2)
    {
        int total = pInfo->totalUsedBytes;
        CAM_LOGD("[%s] DUMP_TOTAL: totalUsedBytes(%d) +", __FUNCTION__,total);
        android::String8 path;
        path = android::String8::format("/data/vendor/camera_dump/Total_%" PRId64 "_%d_%d_AfterUnpack.dump",pInfo->timestamp,pInfo->requestNo,pInfo->frameNo);
        FILE *myFptr=NULL;
        std::unique_ptr<char[]> chunk(new char[total]);
        myFptr = fopen(path.string(), "wb");
        if(myFptr != NULL)
        {
            ::memset(chunk.get(),0,total);
            memcpy((void*)chunk.get(),(void*)pInBufAddr,total);
            fwrite(chunk.get(),sizeof(char),total,myFptr);
            fclose(myFptr);
        }
        else
        {
            CAM_LOGE("[%s] DUMP_TOTAL: save to path:(%s) fopen fail!", __FUNCTION__, path.string());
        }
        CAM_LOGD("[%s] DUMP_TOTAL:-", __FUNCTION__);
    }
    //
    outputParam.haltimestamp = pInfo->haltimestamp;
    //
    UNFLATTEN_META(outputParam.appControlMeta,pInfo->metaInfo[CONTROL_APP].offset,pInfo->metaInfo[CONTROL_APP].length,pInfo->metaInfo[CONTROL_APP].checkSum,"Control_APP_Metadata");
    UNFLATTEN_META(outputParam.appResultMeta,pInfo->metaInfo[APP_RESULT].offset,pInfo->metaInfo[APP_RESULT].length,pInfo->metaInfo[APP_RESULT].checkSum,"APP_Result_Metadata");
    UNFLATTEN_META(outputParam.halResultMeta,pInfo->metaInfo[HAL_RESULT].offset,pInfo->metaInfo[HAL_RESULT].length,pInfo->metaInfo[HAL_RESULT].checkSum,"HAL_Result_Metadata");
    //
    if(pInfo->rawInfo[STTO_RAW].offset != 0)
    {
        UNPACK_RAW(outputParam.pSttoImageBuffer,STTO_RAW,pInfo->rawInfo[STTO_RAW].checkSum,"STTO Buffer");
    }
    //
    if(pInfo->rawInfo[RRZO_RAW].offset != 0)
    {
        UNPACK_RAW(outputParam.pRrzoImageBuffer,RRZO_RAW,pInfo->rawInfo[RRZO_RAW].checkSum,"RRZO Buffer");
    }
    //
    if (pInfo->privateInfo[FD_DATA].offset != 0)
    {
        UNPACK_PRIVATE(outputParam.pFDData, FD_DATA, pInfo->privateInfo[FD_DATA].checkSum, "FD_Data");
    }
    lbExit:
    CAM_LOGD("[%s] -", __FUNCTION__);
#undef UNFLATTEN_META
#undef UNPACK_RAW
#undef DUMP_BUF
    return ret;

}

