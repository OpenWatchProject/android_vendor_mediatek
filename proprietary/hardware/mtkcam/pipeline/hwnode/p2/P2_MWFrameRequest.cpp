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

namespace P2
{

MWFrameRequest::MWFrameRequest(const sp<P2Info> &p2Info, const sp<MWInfo> &mwInfo, const sp<MWFrame> &frame)
    : P2FrameRequest(p2Info, frame->getFrameID())
    , LoggerHolder(frame)
    , mMWInfo(mwInfo)
    , mMWFrame(frame)
    , mExtracted(MFALSE)
    , mImgCount(0)
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( mMWFrame != NULL)
    {
        mExtraData = createP2ExtraData();
        IPipelineFrame::InfoIOMapSet ioMap;
        if( mMWFrame->getInfoIOMapSet(ioMap) )
        {
            printIOMap(ioMap);
            mMetaMap = createP2MetaMap(ioMap.mMetaInfoIOMapSet);
            if( mMWInfo != NULL )
            {
                addP2Meta(mMetaMap, IN_APP, IO_DIR_IN);
                addP2Meta(mMetaMap, IN_P1_HAL, IO_DIR_IN);
                addP2Meta(mMetaMap, IN_P1_APP, IO_DIR_IN);
            }
            updateP2ExtraFromMetadata(mExtraData);
            doP1AppMetaWorkaround();
            removeP2Meta(mMetaMap, IN_P1_APP);
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MWFrameRequest::~MWFrameRequest()
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrameRequest::beginBatchRelease()
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( mMWFrame != NULL )
    {
        mMWFrame->beginBatchRelease();
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrameRequest::endBatchRelease()
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( mMWFrame != NULL )
    {
        mMWFrame->endBatchRelease();
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

std::vector<sp<P2Request>> MWFrameRequest::extractP2Requests()
{
    TRACE_S_FUNC_ENTER(mLogger);
    std::vector<sp<P2Request>> requests;
    if( mExtracted )
    {
        MY_S_LOGE(mLogger, "Requests already extracted, should only extracted once");
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
    TRACE_S_FUNC_EXIT(mLogger);
    return requests;
}

MBOOL MWFrameRequest::addP2Img(P2ImgMap &imgMap, const StreamId_T &sID, ID_IMG id, IO_DIR dir, const IMG_INFO &info)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    auto it = imgMap.find(sID);
    if( it == imgMap.end() )
    {
        if( id != info.id )
        {
            MY_S_LOGW(mLogger, "Invalid img info(%zd/%d:%s) id=(%d)/info.id=(%d)",sID, id, info.name, id, info.id);
        }
        else if( !(info.dir & dir) )
        {
            MY_S_LOGW(mLogger, "Invalid img info(%zd/%d:%s) dir=wanted(%d)/listed(%d)", sID, id, info.name, dir, info.dir);
        }
        else if( info.flag & IO_FLAG_INVALID )
        {
            MY_S_LOGW(mLogger, "Invalid img info(%zd/%d:%s) Invalid IO_INFO: flag(%d)", sID, id, info.name, info.flag);
        }
        else
        {
            sp<P2Img> holder = new MWImg(mMWFrame, sID, dir, info, mImgCount++);
            if( holder == NULL )
            {
                MY_S_LOGW(mLogger, "OOM: cannot create MWImg");
            }
            else
            {
                imgMap[sID] = holder;
                ret = MTRUE;
            }
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL MWFrameRequest::addP2Img(P2ImgMap &imgMap, const StreamId_T &sID, IO_DIR dir)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    ID_IMG id = mMWInfo->toImgID(sID);
    const IMG_INFO &info = P2Img::getImgInfo(id);
    ret = addP2Img(imgMap, sID, id, dir, info);
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL MWFrameRequest::addP2Img(P2ImgMap &imgMap, ID_IMG id, IO_DIR dir)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    auto mwImgInfo = mMWInfo->findImgInfo(id);
    if( mwImgInfo != NULL )
    {
        const IMG_INFO &info = P2Img::getImgInfo(id);
        ret = addP2Img(imgMap, mwImgInfo->getStreamId(), id, dir, info);
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MWFrameRequest::P2ImgMap MWFrameRequest::createP2ImgMap(IPipelineFrame::ImageInfoIOMapSet &imgSet)
{
    TRACE_S_FUNC_ENTER(mLogger);
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
    TRACE_S_FUNC_EXIT(mLogger);
    return holders;
}

MBOOL MWFrameRequest::addP2Meta(P2MetaMap &metaMap, const StreamId_T &sID, ID_META id, IO_DIR dir, const META_INFO &info)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    auto it = metaMap.find(sID);
    if( it == metaMap.end() )
    {
        if( id != info.id )
        {
            MY_S_LOGW(mLogger, "Invalid img info(%zd/%d:%s) id=(%d)/info.id=(%d)",sID, id, info.name, id, info.id);
        }
        else if( !(info.dir & dir) )
        {
            MY_S_LOGW(mLogger, "Invalid meta info(%zd/%d:%s) dir=wanted(%d)/listed(%d)", sID, id, info.name, dir, info.dir);
        }
        else if( info.flag & IO_FLAG_INVALID )
        {
            MY_S_LOGW(mLogger, "Invalid meta info(%zd/%d:%s) Invalid IO_INFO: flag(%d)", sID, id, info.name, info.flag);
        }
        else
        {
            sp<P2Meta> holder = new MWMeta(mMWFrame, sID, dir, info);
            if( holder == NULL )
            {
                MY_S_LOGW(mLogger, "OOM: cannot create MWMeta");
            }
            else
            {
                metaMap[sID] = holder;
                ret = MTRUE;
            }
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL MWFrameRequest::addP2Meta(P2MetaMap &metaMap, const StreamId_T &sID, IO_DIR dir)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    ID_META id = mMWInfo->toMetaID(sID);
    const META_INFO &info = P2Meta::getMetaInfo(id);
    ret = addP2Meta(metaMap, sID, id, dir, info);
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL MWFrameRequest::addP2Meta(P2MetaMap &metaMap, ID_META id, IO_DIR dir)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    auto mwMetaInfo = mMWInfo->findMetaInfo(id);
    if( mwMetaInfo != NULL )
    {
        const META_INFO &info = P2Meta::getMetaInfo(id);
        ret = addP2Meta(metaMap, mwMetaInfo->getStreamId(), id, dir, info);
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL MWFrameRequest::removeP2Meta(P2MetaMap &metaMap, ID_META id)
{
    TRACE_S_FUNC_ENTER(mLogger);
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
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MWFrameRequest::P2MetaMap MWFrameRequest::createP2MetaMap(IPipelineFrame::MetaInfoIOMapSet &metaSet)
{
    TRACE_S_FUNC_ENTER(mLogger);
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
    TRACE_S_FUNC_EXIT(mLogger);
    return holders;
}

MVOID MWFrameRequest::configBufferSize(const Cropper &cropper)
{
    TRACE_S_FUNC_ENTER(mLogger);
    sp<P2Img> rrzo = findP2Img(mImgMap, IN_RESIZED);
    sp<P2Img> rsso = findP2Img(mImgMap, IN_RSSO);
    if( isValid(rrzo) )
    {
        IImageBuffer* buffer = rrzo->getIImageBufferPtr();
        MSize size = cropper.getP1OutSize();
        TRACE_S_FUNC(mLogger, "resize rrzo(%p) from %dx%d to %dx%d", buffer,
                              buffer->getImgSize().w, buffer->getImgSize().h,
                              size.w, size.h);
        buffer->setExtParam(size);
    }
    if( isValid(rsso) )
    {
        MSize size(288, 162);
        if( !tryGetMeta<MSize>(IN_P1_HAL, MTK_P1NODE_RSS_SIZE, size) )
        {
            MY_S_LOGE(mLogger, "cannot get MTK_P1NODE_RSS_ZIE");
        }
        IImageBuffer* buffer = rsso->getIImageBufferPtr();
        TRACE_S_FUNC(mLogger, "resize rsso(%p) from %dx%d to %dx%d", buffer,
                              buffer->getImgSize().w, buffer->getImgSize().h,
                              size.w, size.h);
        buffer->setExtParam(size);
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

sp<P2Meta> MWFrameRequest::findP2Meta(const P2MetaMap &map, ID_META id) const
{
    TRACE_S_FUNC_ENTER(mLogger);
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
    TRACE_S_FUNC(mLogger, "info = %p meta = %p", info.get(), meta.get());
    TRACE_S_FUNC_EXIT(mLogger);
    return meta;
}

sp<P2Img> MWFrameRequest::findP2Img(const P2ImgMap &map, ID_IMG id) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    sp<P2Img> img;
    auto info = mMWInfo->findImgInfo(id);
    if( info != NULL )
    {
        auto it = map.find(info->getStreamId());
        if( it != map.end() && it->second != NULL )
        {
            img = it->second;
        }
    }
    TRACE_S_FUNC(mLogger, "info = %p img = %p", info.get(), img.get());
    TRACE_S_FUNC_EXIT(mLogger);
    return img;
}

Cropper MWFrameRequest::createCropper(const P2MetaMap &metaMap)
{
    TRACE_S_FUNC_ENTER(mLogger);
    IMetadata *appMeta = NULL, *halMeta = NULL;
    sp<P2Meta> appMetaHolder, halMetaHolder;
    appMetaHolder = findP2Meta(metaMap, IN_APP);
    halMetaHolder = findP2Meta(metaMap, IN_P1_HAL);
    if( appMetaHolder != NULL )
    {
        appMeta = appMetaHolder->getIMetadataPtr();
    }
    if( halMetaHolder != NULL )
    {
        halMeta = halMetaHolder->getIMetadataPtr();
    }
    LMVInfo lmvInfo(halMeta, mLogger);
    Cropper cropper(mP2Info, lmvInfo, appMeta, halMeta, mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return cropper;
}

P2ExtraData MWFrameRequest::createP2ExtraData()
{
    TRACE_S_FUNC_ENTER(mLogger);
    P2ExtraData extraData;
    extraData.mOpenID = mP2Info->mSensorID;
    extraData.mMWFrameNo = mMWFrame->getMWFrameID();
    extraData.mMWFrameRequestNo = mMWFrame->getMWFrameRequestID();
    extraData.mBurstNum = mMWInfo->getBurstNum();
    TRACE_S_FUNC_EXIT(mLogger);
    return extraData;
}

MVOID MWFrameRequest::updateP2ExtraFromMetadata(P2ExtraData &extra)
{
    // P1 HAL
    extra.mMWUniqueKey = getMeta<MINT32>(IN_P1_HAL, MTK_PIPELINE_UNIQUE_KEY, 0);
    MINT32 appMode = getMeta<MINT32>(IN_P1_HAL, MTK_FEATUREPIPE_APP_MODE, MTK_FEATUREPIPE_PHOTO_PREVIEW);
    extra.mAppMode = appMode;
    extra.mIsRecording = (appMode == MTK_FEATUREPIPE_VIDEO_RECORD) ||
                         (appMode == MTK_FEATUREPIPE_VIDEO_STOP);

    // P1 APP
    extra.mP1TS = getMeta<MINT64>(IN_P1_APP, MTK_SENSOR_TIMESTAMP, 0);
}

MVOID MWFrameRequest::updateP2Metadata()
{
    sp<P2Meta> inHalMeta;
    inHalMeta = findP2Meta(mMetaMap, IN_P1_HAL);
    if( inHalMeta == NULL)
    {
        return;
    }

    MRect rect;
    if( !inHalMeta->tryGet<MRect>(MTK_3A_PRV_CROP_REGION, rect) )
    {
        sp<P2Img> display;
        auto it = mImgMap.begin();
        while( it != mImgMap.end() )
        {
            if( it->second->isDisplay() )
            {
                display = it->second;
                break;
            }
        }

        if( isValid(display) )
        {
            sp<P2Img> rrzo = findP2Img(mImgMap, IN_RESIZED);
            MBOOL resized = isValid(rrzo);

            MUINT32 cropFlag = 0;
            cropFlag |= resized ? Cropper::USE_RESIZED : 0;

            MCropRect crop = mCropper.calcViewAngle(display->getTransformSize(), cropFlag);
            rect = mCropper.toActive(crop, resized);
            inHalMeta->trySet<MRect>(MTK_3A_PRV_CROP_REGION, rect);
        }
    }
}

MBOOL MWFrameRequest::fillP2Img(const sp<P2Request> &request, const sp<P2Img> &img)
{
    TRACE_S_FUNC_ENTER(mLogger);
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
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MBOOL MWFrameRequest::fillP2Meta(const sp<P2Request> &request, const sp<P2Meta> &meta)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    if( request != NULL && meta != NULL )
    {
        ID_META id = meta->getID();
        request->mMeta[id] = meta;
        ret = MTRUE;
    }
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

MVOID MWFrameRequest::fillP2Img(const sp<P2Request> &request, const IPipelineFrame::ImageInfoIOMap &imgInfoMap, const P2ImgMap &p2ImgMap)
{
    TRACE_S_FUNC_ENTER(mLogger);
    for( unsigned i = 0, n = imgInfoMap.vIn.size(); i < n; ++i )
    {
        auto it = p2ImgMap.find(imgInfoMap.vIn.keyAt(i));
        if( it != p2ImgMap.end() )
        {
            fillP2Img(request, it->second);
        }
    }
    for( unsigned i = 0, n = imgInfoMap.vOut.size(); i < n; ++i )
    {
        auto it = p2ImgMap.find(imgInfoMap.vOut.keyAt(i));
        if( it != p2ImgMap.end() )
        {
            fillP2Img(request, it->second);
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrameRequest::fillP2Meta(const sp<P2Request> &request, const IPipelineFrame::MetaInfoIOMap &metaInfoMap, const P2MetaMap &p2MetaMap)
{
    TRACE_S_FUNC_ENTER(mLogger);
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
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrameRequest::fillDefaultP2Meta(const sp<P2Request> &request, const P2MetaMap &metaMap)
{
    TRACE_S_FUNC_ENTER(mLogger);
    fillP2Meta(request, findP2Meta(metaMap, IN_APP));
    fillP2Meta(request, findP2Meta(metaMap, IN_P1_HAL));
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrameRequest::printIOMap(const IPipelineFrame::InfoIOMapSet &ioMap)
{
    TRACE_S_FUNC_ENTER(mLogger);
    const IPipelineFrame::ImageInfoIOMapSet &imgSet = ioMap.mImageInfoIOMapSet;
    const IPipelineFrame::MetaInfoIOMapSet &metaSet = ioMap.mMetaInfoIOMapSet;
    char buffer[128];
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
            unsigned len;
            used += snprintf(buffer+used, sizeof(buffer)-used,
                              " [%d]=>img[%d/%d], meta[%d/%d]",
                              i, imgIn, imgOut, metaIn, metaOut);
        }
    }
    MY_S_LOGD(mLogger, "%s", buffer);
    TRACE_S_FUNC_EXIT(mLogger);
}

std::vector<sp<P2Request>> MWFrameRequest::createRequests(IPipelineFrame::InfoIOMapSet &ioMap)
{
    TRACE_S_FUNC_ENTER(mLogger);
    std::vector<sp<P2Request>> requests;
    IPipelineFrame::ImageInfoIOMapSet &imgSet = ioMap.mImageInfoIOMapSet;
    IPipelineFrame::MetaInfoIOMapSet &metaSet = ioMap.mMetaInfoIOMapSet;

    if( imgSet.size() == 0 || metaSet.size() == 0 ||
        imgSet.size() != metaSet.size() )
    {
        MY_S_LOGW(mLogger, "iomap image=%zu meta=%zu", imgSet.size(), metaSet.size());
    }

    mImgMap = createP2ImgMap(imgSet);
    mCropper = createCropper(mMetaMap);
    configBufferSize(mCropper);
    updateP2Metadata();

    TRACE_S_FUNC(mLogger, "imgMap=%zu metaMap=%zu", mImgMap.size(), mMetaMap.size());

    doRegisterPlugin();
    for(unsigned i = 0, n = imgSet.size(); i < n; ++i )
    {
        Logger logger = makeRequestLogger(mLogger->getLogLevel(), mLogger->getLogSensorID(), mLogger->getLogFrameID(), i);
        sp<P2Request> request = new P2Request(mP2Info, mMWFrame, mCropper, mExtraData, logger);
        if( request == NULL )
        {
            MY_S_LOGW(logger, "OOM: cannot allocate P2Request (%d/%d)", i, n);
            continue;
        }

        fillP2Img(request, imgSet[i], mImgMap);
        if( i < metaSet.size() )
        {
            fillP2Meta(request, metaSet[i], mMetaMap);
        }
        fillDefaultP2Meta(request, mMetaMap);

        request->initIOInfo();
        requests.push_back(request);
    }

    this->beginBatchRelease();
    mImgMap.clear();
    mMetaMap.clear();
    this->endBatchRelease();

    TRACE_S_FUNC_EXIT(mLogger);
    return requests;
}

MVOID MWFrameRequest::doP1AppMetaWorkaround()
{
    // Pipeline frame have timing issue if MetaInP1App is not release
    // before returning P2Node::queue()
    // Use workaround to copy needed value from appMeta to halMeta
    // and release MetaInP1App before returning P2Node::queue()
    TRACE_S_FUNC_ENTER(mLogger);
    auto p1AppInfo = mMWInfo->findMetaInfo(IN_P1_APP);
    if( mMWInfo != NULL && p1AppInfo != NULL )
    {
        StreamId_T p1AppSID = p1AppInfo->getStreamId();
        auto p1AppMetaIt = mMetaMap.find(p1AppSID);
        if( p1AppMetaIt != mMetaMap.end() )
        {
            sp<P2Meta> p1AppMeta = p1AppMetaIt->second;
            sp<P2Meta> appMeta;
            appMeta = findP2Meta(mMetaMap, IN_APP);
            if( p1AppMeta != NULL && appMeta != NULL )
            {
                MINT32 iso;
                if( p1AppMeta->tryGet<MINT32>(MTK_SENSOR_SENSITIVITY, iso) )
                {
                    appMeta->trySet<MINT32>(MTK_SENSOR_SENSITIVITY, iso);
                }
            }
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID MWFrameRequest::doRegisterPlugin()
{
    TRACE_S_FUNC_ENTER(mLogger);
    for( auto it : mImgMap )
    {
        if( it.second != NULL )
        {
            it.second->registerPlugin(mImgPlugin);
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

template <typename T>
T MWFrameRequest::getMeta(ID_META id, MUINT32 tag, T val)
{
    TRACE_S_FUNC_ENTER(mLogger);
    sp<P2Meta> meta = findP2Meta(mMetaMap, id);
    val = P2::getMeta<T>(meta, tag, val);
    TRACE_S_FUNC_EXIT(mLogger);
    return val;
}

template <typename T>
MBOOL MWFrameRequest::tryGetMeta(ID_META id, MUINT32 tag, T &val)
{
    TRACE_S_FUNC_ENTER(mLogger);
    MBOOL ret = MFALSE;
    sp<P2Meta> meta = findP2Meta(mMetaMap, id);
    ret = P2::tryGet<T>(meta, tag, val);
    TRACE_S_FUNC_EXIT(mLogger);
    return ret;
}

} // namespace P2
