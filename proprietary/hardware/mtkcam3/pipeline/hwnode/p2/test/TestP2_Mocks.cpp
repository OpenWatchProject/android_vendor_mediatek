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
#include "TestP2_Mocks.h"

using namespace P2;

//********************************************
// MWMetaMock
//********************************************
MWMetaMock::MWMetaMock(const ILog &log, const P2Pack &p2Pack, const StreamId_T &streamID, IO_DIR dir, const META_INFO &info)
    : P2Meta(log, p2Pack, info.id)
    , mStreamID(streamID)
    , mDir(dir)
    , mStatus(IO_STATUS_INVALID)
    , mMetadata(nullptr)
{
    MY_LOGD("creating [%s] +", info.name.c_str());
    mMetadata = new IMetadata();
    mStatus = IO_STATUS_READY;
    MY_LOGD("creating [%s] -", info.name.c_str());
}

MWMetaMock::~MWMetaMock()
{
    if( mMetadata ){
        delete mMetadata;
        mMetadata = nullptr;
    }
}

StreamId_T MWMetaMock::getStreamID() const
{
    return mStreamID;
}

MBOOL MWMetaMock::isValid() const
{
    return (mMetadata != NULL);
}

IO_DIR MWMetaMock::getDir() const
{
    return mDir;
}

MVOID MWMetaMock::updateResult(MBOOL result)
{
    if( (mDir & IO_DIR_OUT) &&
        mStatus != IO_STATUS_INVALID )
    {
        mStatus = result ? IO_STATUS_OK : IO_STATUS_ERROR;
    }
}


IMetadata* MWMetaMock::getIMetadataPtr() const
{
    return mMetadata;
}

IMetadata::IEntry MWMetaMock::getEntry(MUINT32 tag) const
{
    IMetadata::IEntry entry;
    if( mMetadata )
    {
        entry = mMetadata->entryFor(tag);
    }
    return entry;
}

MBOOL MWMetaMock::setEntry(MUINT32 tag, const IMetadata::IEntry &entry)
{
    MBOOL ret = MFALSE;
    if( mMetadata )
    {
        ret = (mMetadata->update(tag, entry) == OK);
    }
    return ret;
}

//********************************************
// MWImgMock
//********************************************
MWImgMock::MWImgMock(
    const ILog &log,
    const P2Pack &p2Pack,
    const StreamId_T &streamID,
    IO_DIR dir,
    const IMG_INFO &info,
    MUINT32 debugIndex,
    const MWImgMock::Config& config,
    const MUINT32 usage)
: P2Img(log, p2Pack, info.id, debugIndex)
, mStreamID(streamID)
, mDir(dir)
, mStatus(IO_STATUS_INVALID)
, mTransform(0)
, mUsage(usage)
{
    // alloc imgBuffer
    MY_LOGD("creating [%s] +", info.name.c_str());
    mpImagePool = ImageBufferPool::create(
        info.name.c_str(),
        config.size.w, config.size.h,
        (EImageFormat)config.format,
        ImageBufferPool::USAGE_HW,
        true // continuous plane
    );
    if(mpImagePool == nullptr){
        MY_LOGE("create [%s] failed!", info.name.c_str());
    }else{
        mpImagePool->allocate(1);
        mSmpBuf = mpImagePool->request();
    }
    MY_LOGD("creating [%s] -", info.name.c_str());
}

MWImgMock::~MWImgMock()
{
    mSmpBuf = nullptr;
    ImageBufferPool::destroy(mpImagePool);
}

MBOOL MWImgMock::isValid() const
{
    return (mSmpBuf != nullptr && mSmpBuf->mImageBuffer.get() != nullptr);
}

IO_DIR MWImgMock::getDir() const
{
    return mDir;
}

MVOID MWImgMock::registerPlugin(const std::list<sp<P2ImgPlugin>> &plugin)
{
    MY_LOGW("registerPlugin no support");
}

MVOID MWImgMock::updateResult(MBOOL result)
{
    if( (mDir & IO_DIR_OUT) &&
        mStatus != IO_STATUS_INVALID )
    {
        mStatus = result ? IO_STATUS_OK : IO_STATUS_ERROR;
    }
}

IImageBuffer* MWImgMock::getIImageBufferPtr() const
{
    return (mSmpBuf != nullptr) ? mSmpBuf->mImageBuffer.get() : nullptr;
}

MUINT32 MWImgMock::getTransform() const
{
    return mTransform;
}

MUINT32 MWImgMock::getUsage() const
{
    return mUsage;
}

MBOOL MWImgMock::isDisplay() const
{
    return (mUsage & (GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE));
}

MBOOL MWImgMock::isRecord() const
{
    return mUsage & GRALLOC_USAGE_HW_VIDEO_ENCODER;
}

MBOOL MWImgMock::isCapture() const
{
    return !(mUsage & (GRALLOC_USAGE_HW_COMPOSER | GRALLOC_USAGE_HW_VIDEO_ENCODER));
}

MVOID MWImgMock::processPlugin() const
{
    MY_LOGW("processPlugin no support");
}

//********************************************
// MWFrameRequestMock
//********************************************
MWFrameRequestMock::MWFrameRequestMock(
    const ILog &log,
    const P2Pack &pack,
    const sp<P2DataObj> &p2Data,
    const sp<P2InIDMap> &p2IdMap,
    const set<MWRequestPath>& vRequestPaths)
: P2FrameRequest(log, pack, p2IdMap)
, mP2Data(p2Data)
, mvRequestPaths(vRequestPaths)
{
    updateP2FrameData();
    updateP2SensorData();
}

MWFrameRequestMock::~MWFrameRequestMock()
{
    // TODO
}

MVOID MWFrameRequestMock::beginBatchRelease()
{
    // TODO
}

MVOID MWFrameRequestMock::endBatchRelease()
{
    // TODO
}

MVOID MWFrameRequestMock::notifyNextCapture(int requestCnt, MBOOL bSkipCheck)
{
    // TODO
}
MVOID MWFrameRequestMock::metaResultAvailable(IMetadata* partialMeta)
{
    // TODO
}

std::vector<sp<P2Request>> MWFrameRequestMock::extractP2Requests()
{
    std::vector<sp<P2Request>> ret;
    std::vector<MWRequestPath> allPaths =
    {
        MWRequestPath::eGeneral,
        MWRequestPath::ePhysic_1,
        MWRequestPath::ePhysic_2,
        MWRequestPath::eLarge_1,
        MWRequestPath::eLarge_2
    };

    int i = 0;
    for( const auto& path : allPaths )
    {
        if( mvRequestPaths.find(path) != mvRequestPaths.end() ){
            MY_LOGD("extract path(%d)(%s) request", path, pathToChar(path));
            ILog reqLog = makeRequestLogger(mLog, i);
            sp<IP2Frame> dummyHolder = new P2FrameHolder(nullptr);
            sp<P2Request> request = new P2Request(reqLog, dummyHolder, mP2Pack, mInIDMap);

            fillP2Img(request, path);
            fillP2Meta(request, path);
            for(MUINT32 sensorID : mP2Pack.getConfigInfo().mAllSensorID )
            {
                fillDefaultP2Meta(request);
            }
            request->initIOInfo();
            ret.push_back(request);
            ++i;
        }
    }

    MY_LOGD("Num of requests(%d)", ret.size());
    return ret;
}

MVOID MWFrameRequestMock::fillP2Img(sp<P2Request> request, const MWRequestPath path)
{
    Config globalConfig;
    MY_LOGD("path(%d)(%s)", path, pathToChar(path));

    int debugIndex = 0;

    auto addImg_InOut = [&](
        MSize _size, MUINT32 _fmt, MUINT32 _trans, IMG_INFO& _info, MUINT32 _dbIndex, MUINT32 _usage, MUINT32 _streamID, bool isIn = true)
    {
        MWImgMock::Config conf(_size, _fmt, _trans);
        sp<P2Img> img = new MWImgMock(
           mLog, mP2Pack,
           _streamID,
           _info.dir,
           _info,
           _dbIndex,
           conf,
           GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER | GRALLOC_USAGE_HW_TEXTURE
        );
        MY_LOGD("img(%d)(%s)", img->getID(), P2Img::getName(img->getID()));

        if( isIn )
        {
            request->mImg[img->getID()] = img;
        }
        else
        {
            request->mImgOutArray.push_back(img);
        }
    };

    // inputs
    // rrzo
    if( path == MWRequestPath::eGeneral ||
        path == MWRequestPath::ePhysic_1
    ){
        IMG_INFO info(IN_RESIZED, NO_IMG, IO_DIR_IN, "IN_RESIZED", IO_FLAG_DEFAULT);
        addImg_InOut(
            globalConfig.rrzoSize_1, eImgFmt_BAYER12,
            0, // transform
            info,
            debugIndex++,
            GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER | GRALLOC_USAGE_HW_TEXTURE,
            mStreamCount++ // monotonic increment as stream id
        );
    }

    // rrzo_2
    if( globalConfig.isDualCam &&
        (path == MWRequestPath::eGeneral ||path == MWRequestPath::ePhysic_2)
    ){
        IMG_INFO info(IN_RESIZED_2, NO_IMG, IO_DIR_IN, "IN_RESIZED_2", IO_FLAG_DEFAULT);
        addImg_InOut(
            globalConfig.rrzoSize_2, eImgFmt_BAYER12,
            0, // transform
            info,
            debugIndex++,
            GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER | GRALLOC_USAGE_HW_TEXTURE,
            mStreamCount++ // monotonic increment as stream id
        );
    }

    // IMGO_1
    if( path == MWRequestPath::eLarge_1 ){
        IMG_INFO info(IN_FULL, NO_IMG, IO_DIR_IN, "IN_FULL", IO_FLAG_DEFAULT);
        addImg_InOut(
            globalConfig.imgoSize_1, eImgFmt_BAYER12,
            0, // transform
            info,
            debugIndex++,
            GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER | GRALLOC_USAGE_HW_TEXTURE,
            mStreamCount++ // monotonic increment as stream id
        );
    }

    // IMGO_2
    if( path == MWRequestPath::eLarge_2 ){
        IMG_INFO info(IN_FULL_2, NO_IMG, IO_DIR_IN, "IN_FULL_2", IO_FLAG_DEFAULT);
        addImg_InOut(
            globalConfig.imgoSize_2, eImgFmt_BAYER12,
            0, // transform
            info,
            debugIndex++,
            GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_NEVER | GRALLOC_USAGE_HW_TEXTURE,
            mStreamCount++ // monotonic increment as stream id
        );
    }

    // outputs
    // display & record
    if( path == MWRequestPath::eGeneral ){
        IMG_INFO info(OUT_YUV, NO_IMG, IO_DIR_OUT, "OUT_YUV", IO_FLAG_DEFAULT);
        addImg_InOut(
            globalConfig.previewSize, eImgFmt_YV12,
            0, // transform
            info,
            debugIndex++,
            GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE,
            mStreamCount++, // monotonic increment as stream id
            false // is input
        );
        addImg_InOut(
            globalConfig.videoSize, eImgFmt_YV12,
            0, // transform
            info,
            debugIndex++,
            GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE|GRALLOC_USAGE_HW_VIDEO_ENCODER,
            mStreamCount++, // monotonic increment as stream id
            false // is input
        );
    }

    // physic
    if( path == MWRequestPath::ePhysic_1 || path == MWRequestPath::ePhysic_2 ){
        IMG_INFO info(OUT_YUV, NO_IMG, IO_DIR_OUT, "OUT_YUV", IO_FLAG_DEFAULT);
        addImg_InOut(
            globalConfig.previewSize, eImgFmt_YV12,
            0, // transform
            info,
            debugIndex++,
            GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE,
            mStreamCount++, // monotonic increment as stream id
            false // is input
        );
    }

    // large
    if( path == MWRequestPath::eLarge_1 || path == MWRequestPath::eLarge_2 ){
        IMG_INFO info(OUT_YUV, NO_IMG, IO_DIR_OUT, "OUT_YUV", IO_FLAG_DEFAULT);
        addImg_InOut(
            globalConfig.largeSize, eImgFmt_YV12,
            0, // transform
            info,
            debugIndex++,
            GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE,
            mStreamCount++, // monotonic increment as stream id
            false // is input
        );
    }
}

MVOID MWFrameRequestMock::fillP2Meta(sp<P2Request> request, const MWRequestPath path)
{
    Config globalConfig;

    // inputs
    {
        META_INFO info;
        info.id  = IN_APP;
        info.dir = IO_DIR_IN;
        info.name = "IN_APP";
        //info.flag = ?
        sp<P2Meta> meta = new MWMetaMock(
            mLog, mP2Pack,
            mStreamCount++,
            info.dir,
            info
        );
        mMetaMap[meta->getID()] = meta;
        request->mMeta[meta->getID()] = meta;
        // TODO:
        // add entries
    }

    if( path == MWRequestPath::eGeneral || path == MWRequestPath::ePhysic_1 ){
        META_INFO info;
        info.id  = IN_P1_APP;
        info.dir = IO_DIR_IN;
        info.name = "IN_P1_APP";
        //info.flag = ?
        sp<P2Meta> meta = new MWMetaMock(
            mLog, mP2Pack,
            mStreamCount++,
            info.dir,
            info
        );
        mMetaMap[meta->getID()] = meta;
        request->mMeta[meta->getID()] = meta;
        // TODO:
        // add entries
    }

    if( path == MWRequestPath::eGeneral || path == MWRequestPath::ePhysic_1 ){
        META_INFO info;
        info.id  = IN_P1_HAL;
        info.dir = IO_DIR_IN;
        info.name = "IN_P1_HAL";
        //info.flag = ?
        sp<P2Meta> meta = new MWMetaMock(
            mLog, mP2Pack,
            mStreamCount++,
            info.dir,
            info
        );
        mMetaMap[meta->getID()] = meta;
        request->mMeta[meta->getID()] = meta;
        // TODO:
        // add entries
    }

    if( globalConfig.isDualCam &&
        (path == MWRequestPath::eGeneral || path == MWRequestPath::ePhysic_2) ){
        META_INFO info;
        info.id  = IN_P1_HAL_2;
        info.dir = IO_DIR_IN;
        info.name = "IN_P1_HAL_2";
        //info.flag = ?
        sp<P2Meta> meta = new MWMetaMock(
            mLog, mP2Pack,
            mStreamCount++,
            info.dir,
            info
        );
        mMetaMap[meta->getID()] = meta;
        request->mMeta[meta->getID()] = meta;
        // TODO:
        // add entries
    }
}

MVOID MWFrameRequestMock::fillDefaultP2Meta(sp<P2Request> request)
{
    // TODO:
}

MVOID MWFrameRequestMock::updateP2FrameData()
{
    P2FrameData &data = mP2Data->mFrameData;
    // P1 HAL
    // sp<P2Meta> inHalMeta = findP2Meta(mMetaMap, IN_P1_HAL);
    MINT32 appMode = MTK_FEATUREPIPE_PHOTO_PREVIEW;
    data.mAppMode = appMode;
    data.mIsRecording = (appMode == MTK_FEATUREPIPE_VIDEO_RECORD) ||
                        (appMode == MTK_FEATUREPIPE_VIDEO_STOP);
    data.mMasterSensorID = mP2Pack.getConfigInfo().mMainSensorID;
}

MVOID MWFrameRequestMock::updateP2SensorData()
{
    for( MUINT32 sensorID : mP2Pack.getConfigInfo().mAllSensorID )
    {
        MY_LOGD("sensor(%d)", sensorID);
        P2SensorData &data = mP2Data->mSensorDataMap[sensorID];

        auto getP2Meta = [](ID_META id, unordered_map<ID_META, sp<P2Meta>>& metaMap){
                return metaMap.find(id) != metaMap.end() ? metaMap[id] : nullptr;
        };

        sp<P2Meta> inApp = getP2Meta(mapID(sensorID,IN_APP), mMetaMap);
        sp<P2Meta> p1Hal = getP2Meta(mapID(sensorID,IN_P1_HAL), mMetaMap);
        sp<P2Meta> p1App = getP2Meta(mapID(sensorID,IN_P1_APP), mMetaMap);
        const P2SensorInfo &sensorInfo = mP2Pack.getSensorInfo(sensorID);

        // P1 HAL
        data.mSensorID = sensorID;
        data.mMWUniqueKey = getMeta<MINT32>(p1Hal, MTK_PIPELINE_UNIQUE_KEY, 0);
        data.mMagic3A = getMeta<MINT32>(p1Hal, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);
        data.mIspProfile = getMeta<MUINT8>(p1Hal, MTK_3A_ISP_PROFILE, 0);

        // P1 APP
        data.mP1TS = getMeta<MINT64>(p1App, MTK_SENSOR_TIMESTAMP, 0);
        if( !tryGet<MINT32>(p1App, MTK_SENSOR_SENSITIVITY, data.mISO) )
        {
            tryGet<MINT32>(inApp, MTK_SENSOR_SENSITIVITY, data.mISO);
        }

        // Cropper
        data.mSensorMode = getMeta<MINT32>(p1Hal, MTK_P1NODE_SENSOR_MODE, 0);
        data.mSensorSize = getMeta<MSize>(p1Hal, MTK_HAL_REQUEST_SENSOR_SIZE, MSize());
        if( !tryGet<MRect>(p1Hal, MTK_P1NODE_SCALAR_CROP_REGION, data.mP1Crop) ||
            !tryGet<MRect>(p1Hal, MTK_P1NODE_DMA_CROP_REGION, data.mP1DMA) ||
            !tryGet<MSize>(p1Hal, MTK_P1NODE_RESIZER_SIZE, data.mP1OutSize) )
        {
            data.mP1Crop = MRect(MPoint(0,0), data.mSensorSize);
            data.mP1DMA = MRect(MPoint(0,0), data.mSensorSize);
            data.mP1OutSize = data.mSensorSize;
        }
        if( !tryGet<MRect>(p1Hal, MTK_P1NODE_BIN_CROP_REGION, data.mP1BinCrop) ||
            !tryGet<MSize>(p1Hal, MTK_P1NODE_BIN_SIZE, data.mP1BinSize) )
        {
            data.mP1BinCrop = MRect(MPoint(0,0), data.mSensorSize);
            data.mP1BinSize = data.mSensorSize;
        }

       if( MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON ==
            getMeta<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE,
                                   MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF) )
        {
            data.mAppEISOn = MTRUE;
        }
        data.mAppCrop = getMeta<MRect>(inApp, MTK_SCALER_CROP_REGION,
                        MRect(MPoint(0,0), sensorInfo.mActiveArray.s));

        LMVInfo lmvInfo = extractLMVInfo(mLog, toIMetadataPtr(p1Hal));
        data.mCropper = new P2Cropper(mLog, &sensorInfo, &data, lmvInfo);
        MY_LOGD("data.mCropper(%p)", data.mCropper.get());
    }
}
