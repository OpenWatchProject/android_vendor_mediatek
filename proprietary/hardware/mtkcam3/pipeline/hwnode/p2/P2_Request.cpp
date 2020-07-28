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

#include "P2_Request.h"

namespace P2
{

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2FrameHolder
#define P2_TRACE        TRACE_P2_FRAME_HOLDER
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

P2FrameHolder::P2FrameHolder(const sp<IP2Frame> &frame)
    : mFrame(frame)
{
}

P2FrameHolder::~P2FrameHolder()
{
}

MVOID P2FrameHolder::beginBatchRelease()
{
    if( mFrame != NULL )
    {
        mFrame->beginBatchRelease();
    }
}

MVOID P2FrameHolder::endBatchRelease()
{
    if( mFrame != NULL )
    {
        mFrame->endBatchRelease();
    }
}

MVOID P2FrameHolder::notifyNextCapture(int requestCnt, MBOOL bSkipCheck)
{
    if( mFrame != NULL )
    {
        mFrame->notifyNextCapture(requestCnt, bSkipCheck);
    }
}

MVOID P2FrameHolder::metaResultAvailable(IMetadata* partialMeta)
{
    if( mFrame != NULL )
    {
        mFrame->metaResultAvailable(partialMeta);
    }
}

sp<IP2Frame> P2FrameHolder::getIP2Frame() const
{
    return mFrame;
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2Request
#define P2_TRACE        TRACE_P2_REQUEST
#include "P2_LogHeader.h"

static MBOOL isMatchAll(const MUINT32 val, const MUINT32 mask)
{
    return ((val & mask) == mask);
}

static MBOOL isMatchPartial(const MUINT32 val, const MUINT32 mask)
{
    return !!(val & mask);
}

P2Request::P2Request(const ILog &log, const sp<IP2Frame> &frameHolder, const P2Pack &p2Pack, const sp<P2InIDMap> &p2IdMap)
    : P2FrameHolder(frameHolder)
    , mSensorID(p2Pack.getFrameData().mMasterSensorID)
    , mLog(log)
    , mP2Pack(p2Pack.getP2Pack(mLog, mSensorID))
    , mInIDMap(p2IdMap)
{
}

P2Request::P2Request(const sp<P2Request> &request)
    : P2FrameHolder(request != NULL ? request->getIP2Frame() : NULL)
    , mLog(request != NULL ? request->mLog : ILog())
    , mP2Pack(request != NULL ? request->mP2Pack : P2Pack())
{
    if( request != NULL )
    {
        mDumpType = request->mDumpType;
        mInIDMap = request->mInIDMap;
        mSensorID = request->mSensorID;
    }
}

P2Request::P2Request(const sp<P2Request> &request, MUINT32 sensorID)
    : P2FrameHolder(request != NULL ? request->getIP2Frame() : NULL)
    , mLog(makeSubSensorLogger(spToILog(request), sensorID))
    , mP2Pack(request != NULL ? request->mP2Pack.getP2Pack(mLog, sensorID) : P2Pack())
{
    mSensorID = sensorID;

    if( request != NULL )
    {
        mDumpType = request->mDumpType;
        mInIDMap = request->mInIDMap;

        ID_META inMeta[] = { IN_APP, IN_P1_APP, IN_P1_HAL, IN_APP_PHY, OUT_APP_PHY };
        ID_IMG inImg[] = { IN_FULL, IN_RESIZED, IN_LCSO, IN_RSSO, IN_FULL_YUV, IN_RESIZED_YUV1, IN_RESIZED_YUV2 };
        for( ID_META meta : inMeta )
        {
            mMeta[meta] = request->mMeta[mInIDMap->getMetaID(sensorID, meta)];
        }
        for( ID_IMG img : inImg )
        {
            mImg[img] = request->mImg[mInIDMap->getImgID(sensorID, img)];
        }

        if( sensorID == mP2Pack.getFrameData().mMasterSensorID )
        {
            mMeta[OUT_APP] = std::move(request->mMeta[OUT_APP]);
            mMeta[OUT_HAL] = std::move(request->mMeta[OUT_HAL]);
            mImg[OUT_FD] = std::move(request->mImg[OUT_FD]);
            mImg[OUT_JPEG_YUV] = std::move(request->mImg[OUT_JPEG_YUV]);
            mImg[OUT_THN_YUV] = std::move(request->mImg[OUT_THN_YUV]);
            mImg[OUT_POSTVIEW] = std::move(request->mImg[OUT_POSTVIEW]);
            mImg[OUT_CLEAN] = std::move(request->mImg[OUT_CLEAN]);
            mImg[OUT_DEPTH] = std::move(request->mImg[OUT_DEPTH]);
            mImg[OUT_BOKEH] = std::move(request->mImg[OUT_BOKEH]);
            mImg[OUT_RAW] = std::move(request->mImg[OUT_RAW]);
            mImgOutArray = std::move(request->mImgOutArray);
        }
    }
}

P2Request::~P2Request()
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID P2Request::initIOInfo()
{
    mIsResized = mImg.count(IN_RESIZED);
    mIsReprocess = mImg.count(IN_REPROCESS);
    const MSize &streamSize = mP2Pack.getConfigInfo().mUsageHint.mStreamingSize;

    for(const auto& it : mImgOutArray)
    {
        if( it->getImgType() == IMG_TYPE_DISPLAY || it->getImgType() == IMG_TYPE_RECORD)
        {
            return; // general
        }
    }
    for(const auto& it : mImgOutArray)
    {
        if( it->isPhysicalStream() )
        {
            mIsPhysic = MTRUE;
            break;
        }
        MSize size = it->getTransformSize();
        if( size.h > streamSize.h || size.w > streamSize.w )
        {
            mIsLarge = MTRUE;
            break;
        }
    }
}

MVOID P2Request::updateSensorID()
{
    for( MUINT32 sID : mP2Pack.getConfigInfo().mAllSensorID )
    {
        if( mIsPhysic )
        {
            if( hasSensorIn(sID) )
            {
                mSensorID = sID;
                break;
            }
        }
        else
        {
            if( sID != mSensorID && hasSensorIn(sID) )
            {
                mSlaveSensorIDs.push_back(sID);
            }
        }
    }
}

MBOOL P2Request::hasSensorIn(MUINT32 sensorID) const
{
    return isValidImg(IN_RESIZED, sensorID) || isValidImg(IN_FULL, sensorID);
}

MUINT32 P2Request::getSensorID() const
{
    return mSensorID;
}

MUINT32 P2Request::getSlaveSensorID() const
{
    return mSlaveSensorIDs.empty() ? INVALID_SENSOR_ID : mSlaveSensorIDs.front();
}

sp<Cropper> P2Request::getCropper() const
{
    return mP2Pack.getSensorData().mCropper;
}

sp<Cropper> P2Request::getCropper(MUINT32 sensorID) const
{
    return mP2Pack.getSensorData(sensorID).mCropper;
}

MBOOL P2Request::hasInput() const
{
    MBOOL ret = MFALSE;
    for( MUINT32 sID : mP2Pack.getConfigInfo().mAllSensorID )
    {
        if(isValidImg(IN_RESIZED, sID) || isValidImg(IN_FULL, sID))
        {
            ret = MTRUE;
            break;
        }
    }
    return ret;
}

MBOOL P2Request::hasOutput() const
{
    MBOOL ret = MFALSE;
    for( const auto &it : mImgOutArray )
    {
        if( isValid(it) )
        {
            ret = MTRUE;
            break;
        }
    }
    ret = ret || isValidImg(OUT_FD) || isValidImg(OUT_JPEG_YUV) || isValidImg(OUT_THN_YUV) || isValidImg(OUT_POSTVIEW) || isValidImg(OUT_RAW);
    return ret;
}

MBOOL P2Request::isResized() const
{
    return mIsResized;
}

MBOOL P2Request::isReprocess() const
{
    return mIsReprocess;
}

MBOOL P2Request::isPhysic() const
{
    return mIsPhysic;
}

MBOOL P2Request::isLarge() const
{
    return mIsLarge;
}

MVOID P2Request::releaseResource(MUINT32 res)
{
    TRACE_S_FUNC_ENTER(mLog,"res=0x%x", res);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2Request::releaseResource");
    if( isMatchAll(res, RES_META) )
    {
        mMeta.clear();
    }
    if( isMatchAll(res, RES_IMG) )
    {
        mImg.clear();
        mImgOutArray.clear();
    }
    else if( isMatchAll(res, RES_OUT_IMG) )
    {
        mImgOutArray.clear();
    }

    if( !mImg.empty() && isMatchPartial(res, RES_IMG) )
    {
        MBOOL realeaseIn = isMatchAll(res, RES_IN_IMG);
        MBOOL realeaseOut = isMatchAll(res, RES_OUT_IMG);
        for( auto &it : mImg )
        {
            if( it.second != NULL )
            {
                if( realeaseIn && (it.second->getDir() & IO_DIR_IN) )
                {
                    it.second = NULL;
                }
                else if( realeaseOut && (it.second->getDir() & IO_DIR_OUT) )
                {
                    it.second = NULL;
                }
            }
        }
    }

    if( !mMeta.empty() && isMatchPartial(res, RES_META) )
    {
        MBOOL realeaseIn = isMatchAll(res, RES_IN_META);
        MBOOL realeaseOut = isMatchAll(res, RES_OUT_META);
        for( auto &it : mMeta )
        {
            if( it.second != NULL )
            {
                if( realeaseIn && (it.second->getDir() & IO_DIR_IN) )
                {
                    it.second = NULL;
                }
                else if( realeaseOut && (it.second->getDir() & IO_DIR_OUT) )
                {
                    it.second = NULL;
                }
            }
        }
    }

    TRACE_S_FUNC_EXIT(mLog);
}

MVOID P2Request::releaseResourceWithLock(MUINT32 res)
{
    Mutex::Autolock _l(mLock);
    releaseResource(res);
}
MVOID P2Request::detachResourceWithLock(MUINT32 res)
{
    TRACE_S_FUNC_ENTER(mLog, "res=0x%x", res);
    Mutex::Autolock _l(mLock);
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2Request::detachResourceWithLock");
    if( isMatchAll(res, RES_ALL) )
    {
        for( auto &&info : P2Img::InfoMap )
        {
            auto img = mImg[info.second.id];
            if( img != NULL )
            {
                img->detach();
            }
        }
        for( auto &&info : P2Meta::InfoMap )
        {
            auto meta = mMeta[info.second.id];
            if( meta != NULL )
            {
                meta->detach();
            }
        }
    }
    else
    {
        MY_S_LOGW(mLog, "doesn't implement for res=0x%x", res);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID P2Request::releaseImageWithLock(ID_IMG id)
{
    TRACE_S_FUNC_ENTER(mLog);
    Mutex::Autolock _l(mLock);
    if ( id == OUT_YUV )
    {
        mImgOutArray.clear();
    }
    else
    {
        mImg[id] = NULL;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID P2Request::releaseMetaWithLock(ID_META id)
{
    TRACE_S_FUNC_ENTER(mLog);
    Mutex::Autolock _l(mLock);
    mMeta[id] = NULL;
    TRACE_S_FUNC_EXIT(mLog);
}

P2MetaSet P2Request::getMetaSet() const
{
    P2MetaSet set;
    sp<P2Meta> inApp = getMeta(IN_APP);
    if( inApp == NULL )
    {
        inApp = getMeta(IN_APP_PHY);
    }
    sp<P2Meta> inHal = getMeta(IN_P1_HAL);
    if( isValid(inApp) )
    {
        IMetadata *meta = inApp->getIMetadataPtr();
        if(meta != NULL)
        {
            set.mInApp = (*meta);
        }
    }
    if( isValid(inHal) )
    {
        IMetadata *meta = inHal->getIMetadataPtr();
        if(meta != NULL)
        {
            set.mInHal = (*meta);
        }
    }
    if( isValidMeta(OUT_APP) || isValidMeta(OUT_HAL) || isValidMeta(OUT_APP_PHY) )
    {
        set.mHasOutput = MTRUE;
    }
    return set;
}

MVOID P2Request::updateMetaSet(const P2MetaSet &set)
{
    TRACE_S_FUNC_ENTER(mLog);
    if( set.mHasOutput )
    {
        if( isValidMeta(OUT_APP) )
        {
            IMetadata *meta = mMeta[OUT_APP]->getIMetadataPtr();
            if(meta != NULL)
            {
                (*meta) = set.mOutApp;
            }
            this->mMeta[OUT_APP]->updateResult(MTRUE);
        }
        if( isValidMeta(OUT_HAL) )
        {
            IMetadata *meta = mMeta[OUT_HAL]->getIMetadataPtr();
            if(meta != NULL)
            {
                (*meta) = set.mInHal;
                (*meta) += set.mOutHal;
            }
            this->mMeta[OUT_HAL]->updateResult(MTRUE);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID P2Request::updateResult(MBOOL result)
{
    TRACE_S_FUNC_ENTER(mLog);
    for( const auto &it : mImgOutArray )
    {
        if( isValid(it) )
        {
            it->updateResult(result);
        }
    }

    for( const auto &it : mImg )
    {
        if( isValid(it.second) && (it.second->getDir() & IO_DIR_OUT))
        {
            it.second->updateResult(result);
        }
    }
    TRACE_S_FUNC_EXIT(mLog);
}

MVOID P2Request::updateMetaResult(MBOOL result)
{
    TRACE_S_FUNC_ENTER(mLog);

    if( isValidMeta(OUT_APP) )
    {
        this->mMeta[OUT_APP]->updateResult(result);
        if(result) updateDummyAppResult(OUT_APP);
    }
    if( isValidMeta(OUT_HAL) )
    {
        this->mMeta[OUT_HAL]->updateResult(result);
    }
    for( MUINT32 sID : mP2Pack.getConfigInfo().mAllSensorID )
    {
        ID_META metaID = mInIDMap->getMetaID(sID, OUT_APP_PHY);
        if( isValidMeta(metaID) )
        {
            this->mMeta[metaID]->updateResult(result);
            if(result) updateDummyAppResult(metaID);
        }
    }

    TRACE_S_FUNC_EXIT(mLog);
}

MVOID P2Request::updateDummyAppResult(ID_META id)
{
    MINT32 recState = mP2Pack.getFrameData().mIsRecording ? MTK_STREAMING_FEATURE_RECORD_STATE_RECORD
                                                          : MTK_STREAMING_FEATURE_RECORD_STATE_PREVIEW;
    this->mMeta[id]->trySet(MTK_STREAMING_FEATURE_RECORD_STATE, recState);

}

MVOID P2Request::dump() const
{
    TRACE_S_FUNC_ENTER(mLog);
    for( auto &&info : P2Meta::InfoMap )
    {
        sp<P2Meta> meta = getMeta(info.second.id);
        if(meta != NULL)
        {
            MY_S_LOGD(mLog, "Meta %s=%p", info.second.name.c_str(), meta.get());
        }
    }
    for( auto &&info : P2Img::InfoMap )
    {
        sp<P2Img> img = getImg(info.second.id);
        MSize size = (img == NULL) ? MSize(0,0) : img->getImgSize();
        if(img != NULL)
        {
            MY_S_LOGD(mLog, "Img %s=%p, size(%dx%d)", info.second.name.c_str(), img.get(), size.w, size.h);
        }
    }
    MY_S_LOGD(mLog, "mImgOutArray.size() = %zu", mImgOutArray.size());
    size_t n = mImgOutArray.size();
    for(size_t i = 0 ; i < n ; i++)
    {
        MY_S_LOGD(mLog, "ImgOut[%zu/%zu] size(%dx%d)",i,n,mImgOutArray[i]->getImgSize().w,  mImgOutArray[i]->getImgSize().h);
    }
    TRACE_S_FUNC_EXIT(mLog);
}

sp<P2Request> P2Request::makeDetachP2Request()
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
    return new P2Request(this, new P2FrameHolder(NULL));
}

sp<P2Meta> P2Request::getMeta(ID_META id) const
{
    TRACE_S_FUNC_ENTER(mLog);
    auto it = mMeta.find(id);
    sp<P2Meta> meta = (it != mMeta.end()) ? it->second : NULL;
    TRACE_S_FUNC_EXIT(mLog);
    return meta;
}

IMetadata *P2Request::getMetaPtr(ID_META id) const
{
    TRACE_S_FUNC_ENTER(mLog);
    sp<P2Meta> spMeta = this->getMeta(id);
    TRACE_S_FUNC_EXIT(mLog);
    return ((spMeta!=NULL) ? spMeta->getIMetadataPtr(): NULL);
}

sp<P2Meta> P2Request::getMeta(ID_META id, MUINT32 sensorID) const
{
    TRACE_S_FUNC_ENTER(mLog);
    auto it = mMeta.find(mInIDMap->getMetaID(sensorID, id));
    sp<P2Meta> meta = (it != mMeta.end()) ? it->second : NULL;
    TRACE_S_FUNC_EXIT(mLog);
    return meta;
}

IMetadata *P2Request::getMetaPtr(ID_META id, MUINT32 sensorID) const
{
    TRACE_S_FUNC_ENTER(mLog);

    sp<P2Meta> spMeta = this->getMeta(id, sensorID);
    if (spMeta == NULL)
    {
        return NULL;
    }

    TRACE_S_FUNC_EXIT(mLog);
    return spMeta->getIMetadataPtr();
}

sp<P2Img> P2Request::getImg(ID_IMG id) const
{
    TRACE_S_FUNC_ENTER(mLog);
    auto it = mImg.find(id);
    sp<P2Img> img = (it != mImg.end()) ? it->second : NULL;
    TRACE_S_FUNC_EXIT(mLog);
    return img;
}

sp<P2Img> P2Request::getImg(ID_IMG id, MUINT32 sensorID) const
{
    TRACE_S_FUNC_ENTER(mLog);
    auto it = mImg.find(mInIDMap->getImgID(sensorID, id));
    sp<P2Img> img = (it != mImg.end()) ? it->second : NULL;
    TRACE_S_FUNC_EXIT(mLog);
    return img;
}

sp<P2Img>&& P2Request::moveImg(ID_IMG id, MUINT32 sensorID)
{
    return std::move(mImg[mInIDMap->getImgID(sensorID, id)]);
}

ID_META P2Request::getMetaID(ID_META metaId, MUINT32 sensorID)
{
    return mInIDMap->getMetaID(sensorID, metaId);
}

ID_IMG P2Request::getImgID(ID_IMG imgId, MUINT32 sensorID)
{
    return mInIDMap->getImgID(sensorID, imgId);
}

MBOOL P2Request::isValidMeta(ID_META id) const
{
    TRACE_S_FUNC_ENTER(mLog);
    TRACE_S_FUNC_EXIT(mLog);
    return isValid(getMeta(id));
}

MBOOL P2Request::isValidMeta(ID_META id, MUINT32 sensorID) const
{
    return isValid(getMeta(id, sensorID));
}

MBOOL P2Request::hasValidMirrorMeta(ID_META id) const
{
    MBOOL ret = MFALSE;
    for( MUINT32 sID : mP2Pack.getConfigInfo().mAllSensorID )
    {
        ID_META metaID = mInIDMap->getMetaID(sID, id);
        if( isValidMeta(metaID) )
        {
            ret = MTRUE;
            break;
        }
    }
    return ret;
}

MBOOL P2Request::isValidImg(ID_IMG id) const
{
    return isValid(getImg(id));
}

MBOOL P2Request::isValidImg(ID_IMG id, MUINT32 sensorID) const
{
    return isValid(getImg(id, sensorID));
}

P2Request::P2Request(const sp<P2Request> &request, const sp<IP2Frame> &frame)
    : P2FrameHolder(frame)
    , mLog(request != NULL ? request->mLog : ILog())
    , mP2Pack(request != NULL ? request->mP2Pack : P2Pack())
{
    TRACE_S_FUNC_ENTER(mLog);
    if( request != NULL )
    {
        mDumpType = request->mDumpType;
        mInIDMap = request->mInIDMap;
        mSensorID = request->mSensorID;
        mSlaveSensorIDs = request->mSlaveSensorIDs;
        mIsPhysic = request->mIsPhysic;
        mMeta = request->mMeta;
        mImg = request->mImg;
        mImgOutArray = request->mImgOutArray;
    }
    TRACE_S_FUNC_EXIT(mLog);
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2FrameRequest
#define P2_TRACE        TRACE_P2_FRAME_REQUEST
#include "P2_LogHeader.h"

P2FrameRequest::P2FrameRequest(const ILog &log, const P2Pack &pack, const sp<P2InIDMap> &p2IdMap)
    : mLog(log)
    , mP2Pack(pack)
    , mInIDMap(p2IdMap)
{
}


P2FrameRequest::~P2FrameRequest()
{
}

MUINT32 P2FrameRequest::getFrameID() const
{
    return mP2Pack.getFrameData().mP2FrameNo;
}

MVOID P2FrameRequest::registerImgPlugin(const sp<P2ImgPlugin> &plugin, MBOOL needSWRW)
{
    mImgPlugin.push_back(plugin);
    mNeedImageSWRW = (mNeedImageSWRW || needSWRW);
}

ID_META P2FrameRequest::mapID(MUINT32 sensorID, ID_META metaId)
{
    return mInIDMap->getMetaID(sensorID, metaId);
}

ID_IMG P2FrameRequest::mapID(MUINT32 sensorID, ID_IMG imgId)
{
    return mInIDMap->getImgID(sensorID, imgId);
}

} // namespace P2
