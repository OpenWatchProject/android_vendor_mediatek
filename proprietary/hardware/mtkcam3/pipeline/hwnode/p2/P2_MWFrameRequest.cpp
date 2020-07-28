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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#include "P2_MWFrameRequest.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MWFrameRequest
#define P2_TRACE        TRACE_MW_FRAME_REQUEST
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

namespace P2
{

MWFrameRequest::MWFrameRequest(const ILog &log, const P2Pack &pack, const sp<P2DataObj> &p2Data, const sp<MWInfo> &mwInfo, const sp<MWFrame> &frame, const sp<P2InIDMap> &p2IdMap, double fps)
    : P2FrameRequest(log, pack, p2IdMap)
    , mP2Data(p2Data)
    , mMWInfo(mwInfo)
    , mMWFrame(frame)
    , mExtracted(MFALSE)
    , mImgStreamCount(0)
    , mFPS(fps)
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mMWFrame != NULL)
    {
        initP2FrameData();
        IPipelineFrame::InfoIOMapSet ioMap;
        if( mMWFrame->getInfoIOMapSet(ioMap) )
        {
            printIOMap(ioMap);
            if(mLog.getLogLevel() >= 1)
            {
                mMWFrame->print(mLog, ioMap);
            }
            P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "createP2MetaMap");
            mMetaMap = createP2MetaMap(ioMap.mMetaInfoIOMapSet);
            P2_CAM_TRACE_END(TRACE_ADVANCED);
            P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "updateP2FrameData_updateP2SensorData");
            updateP2FrameData();
            updateP2SensorData();
            P2_CAM_TRACE_END(TRACE_ADVANCED);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MWFrameRequest::~MWFrameRequest()
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::beginBatchRelease()
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mMWFrame != NULL )
    {
        mMWFrame->beginBatchRelease();
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::endBatchRelease()
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mMWFrame != NULL )
    {
        mMWFrame->endBatchRelease();
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::notifyNextCapture(int requestCnt, MBOOL bSkipCheck)
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mMWFrame != NULL )
    {
        mMWFrame->notifyNextCapture(requestCnt, bSkipCheck);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::metaResultAvailable(IMetadata* partialMeta)
{
    TRACE_S_FUNC_ENTER(mLog);
    if( mMWFrame != NULL )
    {
        mMWFrame->metaResultAvailable(partialMeta);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

std::vector<sp<P2Request>> MWFrameRequest::extractP2Requests()
{
    TRACE_S_FUNC_ENTER(mLog);
    std::vector<sp<P2Request>> requests;
    if( mExtracted )
    {
        MY_S_LOGE(mLog, "Requests already extracted, should only extracted once");
    }
    else
    {
        mExtracted = MTRUE;
        IPipelineFrame::InfoIOMapSet ioMap;
        if( mMWFrame->getInfoIOMapSet(ioMap) )
        {
            requests = createRequests(ioMap);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return requests;
}

MBOOL MWFrameRequest::addP2Img(P2ImgMap &imgMap, const StreamId_T &streamID, IO_DIR dir)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    if( mStreamSet.count(streamID) == 0 )
    {
        mStreamSet.insert(streamID);
        P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "new MWImg");
        for(const sp<P2Img> &holder : MWImg::createP2Imgs(mLog, mP2Pack, mMWInfo, mMWFrame, streamID, dir, mImgStreamCount++, mNeedImageSWRW))
        {
            if( holder == NULL )
            {
                MY_S_LOGW(mLog, "OOM: cannot create MWImg");
            }
            else if(!holder->isValid())
            {
                MY_S_LOGW(mLog, "MWImg invalid. id(%d), Ignore this img.", holder->getID());
            }
            else
            {
                if( dir & IO_DIR_OUT)
                {
                    mImgOutMap[streamID] = holder;
                }
                imgMap[holder->getID()] = holder;
                ret = MTRUE;
            }
        }
        P2_CAM_TRACE_END(TRACE_ADVANCED);
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MWFrameRequest::P2ImgMap MWFrameRequest::createP2ImgMap(IPipelineFrame::ImageInfoIOMapSet &imgSet)
{
    TRACE_S_FUNC_ENTER(mLog);
    P2ImgMap holders;
    for( unsigned i = 0, n = imgSet.size(); i < n; ++i )
    {
        for( unsigned in = 0, inSize = imgSet[i].vIn.size(); in < inSize; ++in )
        {
            addP2Img(holders, imgSet[i].vIn.keyAt(in), IO_DIR_IN);
        }
        for( unsigned out = 0, outSize = imgSet[i].vOut.size(); out < outSize; ++out )
        {
            addP2Img(holders, imgSet[i].vOut.keyAt(out), IO_DIR_OUT);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return holders;
}

MBOOL MWFrameRequest::addP2Meta(P2MetaMap &metaMap, const StreamId_T &sID, ID_META id, IO_DIR dir, const META_INFO &info)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    auto it = metaMap.find(sID);
    if( it == metaMap.end() )
    {
        if( id != info.id )
        {
            MY_S_LOGW(mLog, "Invalid img info(0x%09" PRIx64 "/%d:%s) id=(%d)/info.id=(%d)",sID, id, info.name.c_str(), id, info.id);
        }
        else if( !(info.dir & dir) )
        {
            MY_S_LOGW(mLog, "Invalid meta info(0x%09" PRIx64 "/%d:%s) dir=wanted(%d)/listed(%d)", sID, id, info.name.c_str(), dir, info.dir);
        }
        else if( info.flag & IO_FLAG_INVALID )
        {
            MY_S_LOGW(mLog, "Invalid meta info(0x%09" PRIx64 "/%d:%s) Invalid IO_INFO: flag(%d)", sID, id, info.name.c_str(), info.flag);
        }
        else
        {
            P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "new MWMeta");
            sp<P2Meta> holder = new MWMeta(mLog, mP2Pack, mMWFrame, sID, dir, info);
            P2_CAM_TRACE_END(TRACE_ADVANCED);
            if( holder == NULL )
            {
                MY_S_LOGW(mLog, "OOM: cannot create MWMeta");
            }
            else
            {
                metaMap[sID] = holder;
                ret = MTRUE;
            }
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MBOOL MWFrameRequest::addP2Meta(P2MetaMap &metaMap, const StreamId_T &sID, IO_DIR dir)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    ID_META id = mMWInfo->toMetaID(sID);
    const META_INFO &info = P2Meta::getMetaInfo(id);
    ret = addP2Meta(metaMap, sID, id, dir, info);
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MBOOL MWFrameRequest::addP2Meta(P2MetaMap &metaMap, ID_META id, IO_DIR dir)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    auto mwMetaInfo = mMWInfo->findMetaInfo(id);
    if( mwMetaInfo != NULL )
    {
        const META_INFO &info = P2Meta::getMetaInfo(id);
        ret = addP2Meta(metaMap, mwMetaInfo->getStreamId(), id, dir, info);
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MBOOL MWFrameRequest::removeP2Meta(P2MetaMap &metaMap, ID_META id)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    auto mwMetaInfo = mMWInfo->findMetaInfo(id);
    if( mwMetaInfo != NULL )
    {
        StreamId_T sID = mwMetaInfo->getStreamId();
        auto it = metaMap.find(sID);
        if( it != metaMap.end() )
        {
            metaMap.erase(it);
            ret = MTRUE;
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MWFrameRequest::P2MetaMap MWFrameRequest::createP2MetaMap(IPipelineFrame::MetaInfoIOMapSet &metaSet)
{
    TRACE_S_FUNC_ENTER(mLog);
    P2MetaMap holders;
    for( unsigned i = 0, n = metaSet.size(); i < n; ++i )
    {
        for( unsigned in = 0, inSize = metaSet[i].vIn.size(); in < inSize; ++in )
        {
            addP2Meta(holders, metaSet[i].vIn.keyAt(in), IO_DIR_IN);
        }
        for( unsigned out = 0, outSize = metaSet[i].vOut.size(); out < outSize; ++out )
        {
            addP2Meta(holders, metaSet[i].vOut.keyAt(out), IO_DIR_OUT);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return holders;
}

MVOID MWFrameRequest::configInputBuffer(MUINT32 sensorID, const sp<Cropper> &cropper, const std::vector<MINT64> &timestamp)
{
    TRACE_S_FUNC_ENTER(mLog);

    for( ID_IMG imgID : P2InIDMap::InputImgIDs )
    {
        sp<P2Img> img = findP2InImg(mImgMap, mapID(sensorID, imgID));
        if( isValid(img) )
        {
            MSize size = queryInputImgSize(imgID, cropper);
            img->setImgInfo(size, timestamp);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MSize MWFrameRequest::queryInputImgSize(ID_IMG imgID, const sp<Cropper> &cropper) const
{
    TRACE_S_FUNC_ENTER(mLog);
    MSize size(0, 0);
    if( imgID == IN_RESIZED )
    {
        size = cropper->getP1OutSize();
    }
    else if( imgID == IN_RSSO )
    {
        size = MSize(288, 162);
        if( !tryGet<MSize>(findP2Meta(IN_P1_HAL), MTK_P1NODE_RSS_SIZE, size) )
        {
            MY_S_LOGW(mLog, "cannot get MTK_P1NODE_RSS_ZIE");
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
    return size;
}

sp<P2Meta> MWFrameRequest::findP2Meta(const P2MetaMap &map, ID_META id) const
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<P2Meta> meta;
    auto info = mMWInfo->findMetaInfo(id);
    if( info != NULL )
    {
        auto it = map.find(info->getStreamId());
        if( it != map.end() && it->second != NULL )
        {
            meta = it->second;
        }
    }
    TRACE_S_FUNC(mLog, "info = %p meta = %p", info.get(), meta.get());
    TRACE_S_FUNC_EXIT(mLog);
    return meta;
}

sp<P2Meta> MWFrameRequest::findP2Meta(ID_META id) const
{
    return findP2Meta(mMetaMap, id);
}

sp<P2Img> MWFrameRequest::findP2InImg(const P2ImgMap &map, ID_IMG id) const
{
    sp<P2Img> img;
    auto it = map.find(id);
    if( it != map.end() )
    {
        img = it->second;
    }
    return img;
}

sp<P2Img> MWFrameRequest::findP2InImg(ID_IMG id) const
{
    return findP2InImg(mImgMap, id);
}

MVOID MWFrameRequest::initP2FrameData()
{
    TRACE_S_FUNC_ENTER(mLog);
    mP2Data->mFrameData.mMWFrameNo = mMWFrame->getMWFrameID();
    mP2Data->mFrameData.mMWFrameRequestNo = mMWFrame->getMWFrameRequestID();
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::updateP2FrameData()
{
    TRACE_S_FUNC_ENTER(mLog);
    P2FrameData &data = mP2Data->mFrameData;
    // Need decide master first
    data.mMasterSensorID = decideMasterID();
    if(data.mMasterSensorID == INVALID_SENSOR_ID)
    {
        data.mMasterSensorID = mP2Pack.getConfigInfo().mMainSensorID;
    }
    // P1 HAL
    sp<P2Meta> inHalMeta = findP2Meta(mMetaMap, mapID(data.mMasterSensorID, IN_P1_HAL));
    MINT32 appMode = getMeta<MINT32>(inHalMeta, MTK_FEATUREPIPE_APP_MODE, MTK_FEATUREPIPE_PHOTO_PREVIEW);
    data.mAppMode = appMode;
    data.mIsRecording = (appMode == MTK_FEATUREPIPE_VIDEO_RECORD) ||
                        (appMode == MTK_FEATUREPIPE_VIDEO_STOP);

    // App
    sp<P2Meta> inApp = findP2Meta(mMetaMap, IN_APP);
    std::vector<MINT32> vTargetFps = getMetaVector<MINT32>(inApp, MTK_CONTROL_AE_TARGET_FPS_RANGE);
    if( vTargetFps.size() == 2)
    {
        data.mMinFps = vTargetFps[0];
        data.mMaxFps = vTargetFps[1];
    }
    else
    {
        MY_S_LOGW(mLog, "targetFps in App Meta not correct! count(%zu)", vTargetFps.size());
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MUINT32 MWFrameRequest::decideMasterID()
{
    MUINT32 masterID = INVALID_SENSOR_ID;
    size_t sensorListSize = mP2Pack.getConfigInfo().mAllSensorID.size();

    //there will only be one sensor in ISP HIDL flow
    if(sensorListSize > 1) {

        for( MUINT32 sensorID : mP2Pack.getConfigInfo().mAllSensorID )
        {
            sp<P2Meta> p1Hal = findP2Meta(mMetaMap, mapID(sensorID,IN_P1_HAL));
            MUINT32 realMasterId = (MUINT32)getMeta<MINT32>(p1Hal, MTK_DUALZOOM_REAL_MASTER, INVALID_SENSOR_ID);
            if(realMasterId != INVALID_SENSOR_ID)
            {
                masterID = realMasterId;
                break;
            }
        }

        if(masterID == INVALID_SENSOR_ID)
        {
            MY_S_LOGW(mLog, "in multicam mode, it cannot find master id. please check flow.");
        }
    }
    return masterID;
}

MVOID MWFrameRequest::updateP2SensorData()
{
    TRACE_S_FUNC_ENTER(mLog);

    for( MUINT32 sensorID : mP2Pack.getConfigInfo().mAllSensorID )
    {
        P2SensorData &data = mP2Data->mSensorDataMap[sensorID];
        sp<P2Meta> inApp = findP2Meta(mMetaMap, mapID(sensorID,IN_APP));
        if(inApp == NULL)
        {
            inApp = findP2Meta(mMetaMap, mapID(sensorID,IN_APP_PHY));
        }
        sp<P2Meta> p1Hal = findP2Meta(mMetaMap, mapID(sensorID,IN_P1_HAL));
        sp<P2Meta> p1App = findP2Meta(mMetaMap, mapID(sensorID,IN_P1_APP));
        const P2SensorInfo &sensorInfo = mP2Pack.getSensorInfo(sensorID);

        // P1 HAL
        data.mSensorID = sensorID;
        data.mMWUniqueKey = getMeta<MINT32>(p1Hal, MTK_PIPELINE_UNIQUE_KEY, 0);
        data.mMagic3A = getMeta<MINT32>(p1Hal, MTK_P1NODE_PROCESSOR_MAGICNUM, 0);
        data.mIspProfile = getMeta<MUINT8>(p1Hal, MTK_3A_ISP_PROFILE, 0);
        data.mLV = getMeta<MINT32>(p1Hal, MTK_REAL_LV, 0);
        data.mLensPosition = getMeta<MINT32>(p1Hal, MTK_3A_AF_LENS_POSITION, 0);
        extract(&data.mNDDHint, toIMetadataPtr(p1Hal));
        extract_by_SensorOpenId(&data.mNDDHint, sensorID);

        // P1 APP
        updateP1TS(p1App, p1Hal, data);
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
                                   MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF) ||
           MTK_EIS_FEATURE_EIS_MODE_ON ==
            getMeta<MINT32>(inApp, MTK_EIS_FEATURE_EIS_MODE,
                                   MTK_EIS_FEATURE_EIS_MODE_OFF) )
        {
            data.mAppEISOn = MTRUE;
        }
        data.mAppCrop = getMeta<MRect>(inApp, MTK_SCALER_CROP_REGION,
                        MRect(MPoint(0,0), sensorInfo.mActiveArray.s));

        data.mSimulatedAppCrop = getMeta<MRect>(p1Hal, MTK_SENSOR_SCALER_CROP_REGION,
                        MRect(0,0));
        LMVInfo lmvInfo;
        if( isValid(p1Hal) )
        {
            lmvInfo = extractLMVInfo(mLog, toIMetadataPtr(p1Hal));
        }
        data.mCropper = new P2Cropper(mLog, &sensorInfo, &data, lmvInfo);
        data.mNvramDsdn = P2PlatInfo::getInstance(sensorID)->queryDSDN(data.mMagic3A, data.mIspProfile);
    }

    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::updateP1TS(const sp<P2Meta> &p1App, const sp<P2Meta> &p1Hal, P2SensorData &data)
{
    // For app:      APP_META: MTK_SENSOR_TIMESTAMP
    // For exposure: HAL_META: MTK_P1NODE_FRAME_START_TIMESTAMP
    //                         = MTK_SENSOR_TIMESTAMP + exposure time
    TRACE_S_FUNC_ENTER(mLog);
    data.mP1TSVector = getMetaVector<MINT64>(p1App, MTK_SENSOR_TIMESTAMP);
    std::vector<MINT64> frameTS = getMetaVector<MINT64>(p1Hal, MTK_P1NODE_FRAME_START_TIMESTAMP);
    data.mP1TSVector.reserve(frameTS.size());
    if( data.mP1TSVector.size() <= 0 )
    {
        data.mP1TSVector.push_back(frameTS.size() ? frameTS[0] : 0);
    }
    for( unsigned i = data.mP1TSVector.size(), size = frameTS.size(); i < size; ++i )
    {
        data.mP1TSVector.push_back(data.mP1TSVector[i-1] + (frameTS[i] - frameTS[i-1]));
    }
    data.mP1TS = data.mP1TSVector[0];
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::updateP2Metadata(MUINT32 sensorID, const sp<Cropper> &cropper)
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<P2Meta> inHalMeta;
    inHalMeta = findP2Meta(mMetaMap, mapID(sensorID, IN_P1_HAL));
    if( !isValid(inHalMeta) )
    {
        return;
    }

    MRect rect;
    if( !mMWInfo->isCapture() && !inHalMeta->tryGet<MRect>(MTK_3A_PRV_CROP_REGION, rect))
    {
        sp<P2Img> display, outImg, target;
        for( auto &it : mImgOutMap )
        {
            if( it.second->isDisplay() )
            {
                display = it.second;
                break;
            }
            else if( !it.second->isFD() && (it.second->getDir() & IO_DIR_OUT) )
            {
                outImg = it.second;
            }

        }
        target = isValid(display) ? display : outImg;
        if( isValid(target) )
        {
            sp<P2Img> rrzo = findP2InImg(mImgMap, mapID(sensorID, IN_RESIZED));
            MBOOL resized = isValid(rrzo);

            MUINT32 cropFlag = 0;
            cropFlag |= resized ? Cropper::USE_RESIZED : 0;

            MCropRect crop = cropper->calcViewAngle(mLog, target->getTransformSize(), cropFlag);
            rect = cropper->toActive(crop, resized);
            P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "inHalMeta->trySet<MRect>");
            inHalMeta->trySet<MRect>(MTK_3A_PRV_CROP_REGION, rect);
            P2_CAM_TRACE_END(TRACE_ADVANCED);
        }
    }
    TRACE_S_FUNC_ENTER(mLog);
}

MBOOL MWFrameRequest::fillP2Img(const sp<P2Request> &request, const sp<P2Img> &img)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    if( request != NULL && img != NULL )
    {
        ID_IMG id = img->getID();
        if( id == OUT_YUV )
        {
            request->mImgOutArray.push_back(img);
        }
        else
        {
            request->mImg[id] = img;
        }
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MBOOL MWFrameRequest::fillP2Meta(const sp<P2Request> &request, const sp<P2Meta> &meta)
{
    TRACE_S_FUNC_ENTER(mLog);
    MBOOL ret = MFALSE;
    if( request != NULL && meta != NULL )
    {
        ID_META id = meta->getID();
        request->mMeta[id] = meta;
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLog);
    return ret;
}

MVOID MWFrameRequest::fillP2Img(const sp<P2Request> &request, const IPipelineFrame::ImageInfoIOMap &imgInfoMap, const P2ImgMap &p2ImgMap)
{
    TRACE_S_FUNC_ENTER(mLog);
    for( unsigned i = 0, n = imgInfoMap.vIn.size(); i < n; ++i )
    {
        for( ID_IMG id : mMWInfo->toImgIDs(imgInfoMap.vIn.keyAt(i), IO_DIR_IN))
        {
            fillP2Img(request, findP2InImg(p2ImgMap, id));
        }
    }
    for( unsigned i = 0, n = imgInfoMap.vOut.size(); i < n; ++i )
    {
        auto it = mImgOutMap.find(imgInfoMap.vOut.keyAt(i));
        if( it != mImgOutMap.end() )
        {
            fillP2Img(request, it->second);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::fillP2Meta(const sp<P2Request> &request, const IPipelineFrame::MetaInfoIOMap &metaInfoMap, const P2MetaMap &p2MetaMap)
{
    TRACE_S_FUNC_ENTER(mLog);
    for( unsigned i = 0, n = metaInfoMap.vIn.size(); i < n; ++i )
    {
        auto it = p2MetaMap.find(metaInfoMap.vIn.keyAt(i));
        if( it != p2MetaMap.end() )
        {
            fillP2Meta(request, it->second);
        }
    }
    for( unsigned i = 0, n = metaInfoMap.vOut.size(); i < n; ++i )
    {
        auto it = p2MetaMap.find(metaInfoMap.vOut.keyAt(i));
        if( it != p2MetaMap.end() )
        {
            fillP2Meta(request, it->second);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID MWFrameRequest::printIOMap(const IPipelineFrame::InfoIOMapSet &ioMap)
{
    TRACE_S_FUNC_ENTER(mLog);
    const IPipelineFrame::ImageInfoIOMapSet &imgSet = ioMap.mImageInfoIOMapSet;
    const IPipelineFrame::MetaInfoIOMapSet &metaSet = ioMap.mMetaInfoIOMapSet;
    char buffer[256];
    unsigned used = 0;

    used = snprintf(buffer, sizeof(buffer), "iomap:");

    unsigned imgSize = imgSet.size();
    unsigned metaSize = metaSet.size();
    unsigned maxSize = (imgSize >= metaSize) ? imgSize : metaSize;
    for( unsigned i = 0; i < maxSize; ++i )
    {
        unsigned imgIn = 0, imgOut = 0, metaIn = 0, metaOut = 0;
        if( i < imgSize )
        {
            imgIn = imgSet[i].vIn.size();
            imgOut = imgSet[i].vOut.size();
        }
        if( i < metaSize )
        {
            metaIn = metaSet[i].vIn.size();
            metaOut = metaSet[i].vOut.size();
        }
        if( used < sizeof(buffer) )
        {
            used += snprintf(buffer+used, sizeof(buffer)-used,
                              " [%d]=>img[%d/%d], meta[%d/%d], fps[%.2f]",
                              i, imgIn, imgOut, metaIn, metaOut, mFPS);
        }
    }
    MY_S_LOGD(mLog, "%s", buffer);
    TRACE_S_FUNC_EXIT(mLog);
}

std::vector<sp<P2Request>> MWFrameRequest::createRequests(IPipelineFrame::InfoIOMapSet &ioMap)
{
    TRACE_S_FUNC_ENTER(mLog);
    std::vector<sp<P2Request>> requests;
    IPipelineFrame::ImageInfoIOMapSet &imgSet = ioMap.mImageInfoIOMapSet;
    IPipelineFrame::MetaInfoIOMapSet &metaSet = ioMap.mMetaInfoIOMapSet;

    if( imgSet.size() == 0 || metaSet.size() == 0 ||
        imgSet.size() != metaSet.size() )
    {
        MY_S_LOGW(mLog, "iomap image=%zu meta=%zu", imgSet.size(), metaSet.size());
    }

    this->beginBatchRelease();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "MWFrameRequest::createRequests->createP2ImgMap");
    mImgMap = createP2ImgMap(imgSet);
    P2_CAM_TRACE_END(TRACE_ADVANCED);

    for(MUINT32 sensorID : mP2Pack.getConfigInfo().mAllSensorID )
    {
        sp<Cropper> cropper = mP2Pack.getSensorData(sensorID).mCropper;
        std::vector<MINT64> ts = mP2Pack.getSensorData(sensorID).mP1TSVector;
        configInputBuffer(sensorID, cropper, ts);
        updateP2Metadata(sensorID, cropper);
    }

    TRACE_S_FUNC(mLog, "imgMap=%zu imgOutMap=%zu metaMap=%zu", mImgMap.size(), mImgOutMap.size(), mMetaMap.size());

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "MWFrameRequest::createRequests->doRegisterPlugin");
    doRegisterPlugin();
    P2_CAM_TRACE_END(TRACE_ADVANCED);

    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "MWFrameRequest::createRequests->newP2Request_fillImg_Meta");
    for(unsigned i = 0, n = imgSet.size(); i < n; ++i )
    {
        ILog reqLog = makeRequestLogger(mLog, i);
        sp<P2Request> request = new P2Request(reqLog, mMWFrame, mP2Pack, mInIDMap);
        if( request == NULL )
        {
            MY_S_LOGW(reqLog, "OOM: cannot allocate P2Request (%d/%d)", i, n);
            continue;
        }

        fillP2Img(request, imgSet[i], mImgMap);
        if( i < metaSet.size() )
        {
            fillP2Meta(request, metaSet[i], mMetaMap);
        }

        request->initIOInfo();
        request->updateSensorID();
        requests.push_back(request);
    }
    P2_CAM_TRACE_END(TRACE_ADVANCED);

    mImgMap.clear();
    mMetaMap.clear();
    mImgOutMap.clear();
    this->endBatchRelease();

    TRACE_S_FUNC_EXIT(mLog);
    return requests;
}

MVOID MWFrameRequest::doRegisterPlugin()
{
    TRACE_S_FUNC_ENTER(mLog);
    for( auto it : mImgMap )
    {
        if( it.second != NULL)
        {
            it.second->registerPlugin(mImgPlugin);
        }
    }
    for( auto it : mImgOutMap )
    {
        if( it.second != NULL)
        {
            it.second->registerPlugin(mImgPlugin);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

} // namespace P2
