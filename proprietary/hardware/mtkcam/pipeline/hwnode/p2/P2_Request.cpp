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

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2Request
#define P2_TRACE        TRACE_P2_REQUEST
#include "P2_LogHeader.h"

P2Request::P2Request(const sp<P2Info> &p2Info, const sp<IP2Frame> &frameHolder, const Cropper &cropper, const P2ExtraData &extraData, const Logger &logger)
    : P2FrameHolder(frameHolder)
    , LoggerHolder(logger)
    , mP2Info(p2Info)
    , mCropper(cropper)
    , mExtraData(extraData)
    , mIsResized(MFALSE)
    , mIsReprocess(MFALSE)
{
}

P2Request::P2Request(const sp<P2Request> &request)
    : P2FrameHolder(request)
    , LoggerHolder(request)
    , mIsResized(MFALSE)
    , mIsReprocess(MFALSE)
{
    if( request != NULL )
    {
        mP2Info = request->mP2Info;
        mCropper = request->mCropper;
        mExtraData = request->mExtraData;
        mIsResized = request->mIsResized;
        mIsReprocess = request->mIsReprocess;
    }
}

P2Request::~P2Request()
{
}

sp<P2Info> P2Request::getP2Info() const
{
    return mP2Info;
}

const Cropper& P2Request::getCropper() const
{
    return mCropper;
}

MVOID P2Request::initIOInfo()
{
    mIsResized = (mImg[IN_RESIZED] != NULL);
    mIsReprocess = (mImg[IN_REPROCESS] != NULL);
}

MBOOL P2Request::hasInput() const
{
    MBOOL ret = MFALSE;
    ret = isValidImg(IN_FULL) || isValidImg(IN_RESIZED);
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
    ret = ret || isValidImg(OUT_FD);
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

MVOID P2Request::releaseResource(MUINT32 res)
{
    if( res & RES_IN_IMG )
    {
        for( auto &&info : P2Img::InfoMap )
        {
            if( info.second.dir & IO_DIR_IN )
            {
                mImg[info.second.id] = NULL;
            }
        }
    }
    if( res & RES_OUT_IMG )
    {
        for( auto &&info : P2Img::InfoMap )
        {
            if( info.second.dir & IO_DIR_OUT )
            {
                mImg[info.second.id] = NULL;
            }
        }
        mImgOutArray.clear();
    }
    if( res & RES_IN_META )
    {
        for( auto &&info : P2Meta::InfoMap )
        {
            if( info.second.dir & IO_DIR_IN )
            {
                mMeta[info.second.id] = NULL;
            }
        }
    }
    if( res & RES_OUT_META )
    {
        for( auto &&info : P2Meta::InfoMap )
        {
            if( info.second.dir & IO_DIR_OUT )
            {
                mMeta[info.second.id] = NULL;
            }
        }
    }
}

P2MetaSet P2Request::getMetaSet() const
{
    P2MetaSet set;
    sp<P2Meta> inApp = getMeta(IN_APP);
    sp<P2Meta> inHal = getMeta(IN_P1_HAL);
    if( isValid(inApp) )
    {
        IMetadata *meta = inApp->getIMetadataPtr();
        set.mInApp = (*meta);
    }
    if( isValid(inHal) )
    {
        IMetadata *meta = inHal->getIMetadataPtr();
        set.mInHal = (*meta);
    }
    if( isValidMeta(OUT_APP) || isValidMeta(OUT_HAL) )
    {
        set.mHasOutput = MTRUE;
    }
    return set;
}

MVOID P2Request::updateMetaSet(const P2MetaSet &set)
{
    TRACE_S_FUNC_ENTER(mLogger);
    if( set.mHasOutput )
    {
        if( isValidMeta(OUT_APP) )
        {
            IMetadata *meta = mMeta[OUT_APP]->getIMetadataPtr();
            (*meta) = set.mOutApp;
            this->mMeta[OUT_APP]->updateResult(MTRUE);
        }
        if( isValidMeta(OUT_HAL) )
        {
            IMetadata *meta = mMeta[OUT_HAL]->getIMetadataPtr();
            (*meta) = set.mInHal;
            (*meta) += set.mOutHal;
            this->mMeta[OUT_HAL]->updateResult(MTRUE);
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

MVOID P2Request::updateResult(MBOOL result)
{
    TRACE_S_FUNC_ENTER(mLogger);
    for( const auto &it : mImgOutArray )
    {
        if( isValid(it) )
        {
            it->updateResult(result);
        }
    }
    TRACE_S_FUNC_EXIT(mLogger);
}

const P2ExtraData& P2Request::getExtraData() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return mExtraData;
}

MVOID P2Request::dump() const
{
    TRACE_S_FUNC_ENTER(mLogger);
    for( auto &&info : P2Meta::InfoMap )
    {
        sp<P2Meta> meta = getMeta(info.second.id);
        MY_S_LOGD(mLogger, "Meta %s=%p", info.second.name, meta.get());
    }
    for( auto &&info : P2Img::InfoMap )
    {
        sp<P2Img> img = getImg(info.second.id);
        MY_S_LOGD(mLogger, "Img %s=%p", info.second.name, img.get());
    }
    MY_S_LOGD(mLogger, "mImgOutArray.size() = %zu", mImgOutArray.size());
    TRACE_S_FUNC_EXIT(mLogger);
}

sp<P2Meta> P2Request::getMeta(ID_META id) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    auto it = mMeta.find(id);
    sp<P2Meta> meta = (it != mMeta.end()) ? it->second : NULL;
    TRACE_S_FUNC_EXIT(mLogger);
    return meta;
}

sp<P2Img> P2Request::getImg(ID_IMG id) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    auto it = mImg.find(id);
    sp<P2Img> img = (it != mImg.end()) ? it->second : NULL;
    TRACE_S_FUNC_EXIT(mLogger);
    return img;
}

MBOOL P2Request::isValidMeta(ID_META id) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return isValid(getMeta(id));
}

MBOOL P2Request::isValidImg(ID_IMG id) const
{
    TRACE_S_FUNC_ENTER(mLogger);
    TRACE_S_FUNC_EXIT(mLogger);
    return isValid(getImg(id));
}

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2FrameRequest
#define P2_TRACE        TRACE_P2_FRAME_REQUEST
#include "P2_LogHeader.h"

P2FrameRequest::P2FrameRequest(const sp<P2Info> &p2Info, MUINT32 frameID)
    : mP2Info(p2Info)
    , mFrameID(frameID)
{
}


P2FrameRequest::~P2FrameRequest()
{
}

MUINT32 P2FrameRequest::getSensorID() const
{
    return mP2Info != NULL ? mP2Info->mSensorID : INVALID_SENSOR_ID;
}

MUINT32 P2FrameRequest::getFrameID() const
{
    return mFrameID;
}

MVOID P2FrameRequest::registerImgPlugin(const sp<P2ImgPlugin> &plugin)
{
    mImgPlugin.push_back(plugin);
}

} // namespace P2
