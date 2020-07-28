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

#include "P2_Util.h"
#include "P2_DebugControl.h"
#define P2_CLASS_TAG    P2Util
#define P2_TRACE        TRACE_P2_UTIL
#include "P2_LogHeader.h"

#define toBool(x) (!!(x))

#define FORCE_TEST_MDP 0

namespace P2
{

P2Util::SimpleIO::SimpleIO()
    : mResized(MFALSE)
    , mUseLMV(MFALSE)
    , mUseMargin(MFALSE)
{
}

auto P2Util::getDebugExif()
{
    static auto const inst = MAKE_DebugExif();
    return inst;
}

MVOID P2Util::SimpleIO::setUseLMV(MBOOL useLMV)
{
    mUseLMV = useLMV;
}

MBOOL P2Util::SimpleIO::hasInput() const
{
    return isValid(mIMGI);
}

MBOOL P2Util::SimpleIO::hasOutput() const
{
    return isValid(mIMG2O) ||
           isValid(mIMG3O) ||
           isValid(mWROTO) ||
           isValid(mWDMAO);
}

MBOOL P2Util::SimpleIO::isResized() const
{
    return mResized;
}

MSize P2Util::SimpleIO::getInputSize() const
{
    MSize size(0, 0);
    if( isValid(mIMGI) )
    {
        size = mIMGI->getIImageBufferPtr()->getImgSize();
    }
    return size;
}

MVOID P2Util::SimpleIO::addMargin(const char* name, const MSize &margin)
{
    TRACE_FUNC_ENTER();
    mUseMargin = MTRUE;
    mMargin += margin;
    TRACE_FUNC("%s margin=%dx%d, total margin=%dx%d", name, margin.w, margin.h, mMargin.w, mMargin.h);
    TRACE_FUNC_EXIT();
}

MBOOL P2Util::SimpleIO::hasMargin() const
{
    return mUseMargin;
}

MVOID P2Util::SimpleIO::updateResult(MBOOL result) const
{
    if( mIMG2O != NULL ) mIMG2O->updateResult(result);
    if( mWROTO != NULL ) mWROTO->updateResult(result);
    if( mWDMAO != NULL ) mWDMAO->updateResult(result);
}

MVOID P2Util::SimpleIO::dropRecord() const
{
    if( mWROTO != NULL && mWROTO->isRecord() )
    {
        mWROTO->updateResult(MFALSE);
    }
    if( mWDMAO != NULL && mWDMAO->isRecord() )
    {
        mWDMAO->updateResult(MFALSE);
    }
}

MVOID P2Util::SimpleIO::earlyRelease(MUINT32 mask, MBOOL result)
{
    if( mask & P2Util::RELEASE_DISP )
    {
        mIMGI = NULL;
        mLCEI = NULL;
        if( mWROTO != NULL && mWROTO->isDisplay() )
        {
            mWROTO->updateResult(result);
            mWROTO = NULL;
        }
        if( mWDMAO != NULL && mWDMAO->isDisplay() )
        {
            mWDMAO->updateResult(result);
            mWDMAO = NULL;
        }
    }
    if( mask & P2Util::RELEASE_FD )
    {
        if( mIMG2O != NULL )
        {
            mIMG2O->updateResult(result);
            mIMG2O = NULL;
        }
    }
}

sp<P2Img> P2Util::SimpleIO::getMDPSrc() const
{
    if( mWDMAO != NULL )
    {
        return mWDMAO;
    }
    else if( mWROTO != NULL )
    {
        return mWROTO;
    }
    return NULL;
}

P2Util::SimpleIO P2Util::extractSimpleIO(const sp<P2Request> &request, MUINT32 portFlag)
{
    TRACE_S_FUNC_ENTER(request);
    SimpleIO io;
    MBOOL useVenc = !!(portFlag & P2Util::USE_VENC);
    if( isValid(request->mImg[IN_RESIZED]) )
    {
        io.mResized = MTRUE;
        io.mIMGI = std::move(request->mImg[IN_RESIZED]);
    }
    else if( isValid(request->mImg[IN_FULL]) )
    {
        io.mResized = MFALSE;
        io.mIMGI = std::move(request->mImg[IN_FULL]);
    }
    if( isValid(request->mImg[IN_LCSO]) )
    {
        io.mLCEI = std::move(request->mImg[IN_LCSO]);
    }
    if( isValid(request->mImg[OUT_FD]) )
    {
        io.mIMG2O = std::move(request->mImg[OUT_FD]);
    }
#if FORCE_TEST_MDP
    io.mWDMAO = useVenc ? NULL : P2Util::extractOut(request, P2Util::FIND_NO_ROTATE);
    if( io.mWDMAO == NULL )
    {
        io.mWROTO = P2Util::extractOut(request, P2Util::FIND_ROTATE);
    }
#else
    io.mWROTO = P2Util::extractOut(request, P2Util::FIND_ROTATE);
    if( io.mWROTO == NULL )
    {
        io.mWROTO = P2Util::extractOut(request, P2Util::FIND_NO_ROTATE);
    }
    io.mWDMAO = useVenc ? NULL : P2Util::extractOut(request, P2Util::FIND_NO_ROTATE);
#endif // FORCE_TEST_MDP
    TRACE_S_FUNC_EXIT(request);
    return io;
}

MBOOL P2Util::getActiveArrayRect(MUINT32 sensorID, MRect &rect, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    MBOOL ret = MFALSE;
    MRect activeArray(1600, 1200);
    sp<IMetadataProvider> metaProvider = NSCam::NSMetadataProviderManager::valueFor(sensorID);
    if( metaProvider == NULL )
    {
        MY_S_LOGE(logger, "get NSMetadataProvider failed");
    }
    else
    {
        IMetadata staticMeta = metaProvider->getMtkStaticCharacteristics();
        ret = tryGet<MRect>(staticMeta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray);
        if( !ret )
        {
            MY_S_LOGE(logger, "no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
        }
    }
#ifdef USING_MTK_LDVT
    if( !ret )
    {
        MY_S_LOGD(logger, "simulate active array for LDVT");
        activeArray = MRect(1600, 1200);
        ret = MTRUE;
    }
#endif // USING_MTK_LDVT
    if( ret )
    {
        rect = activeArray;
        MY_S_LOGD(logger, "active array (%d,%d)(%dx%d)", rect.p.x, rect.p.y, rect.s.w, rect.s.h);
    }
    TRACE_S_FUNC_EXIT(logger);
    return ret;
}

MVOID P2Util::process3A(const SimpleIO &io, MUINT32 tuningSize, IHal3A_T *hal3A, TuningParam &tuning, P2MetaSet &metaSet, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    MetaSet_T inMetaSet, outMetaSet, *pOutMetaSet = NULL;
    inMetaSet.MagicNum = 0;
    inMetaSet.appMeta = metaSet.mInApp;
    inMetaSet.halMeta = metaSet.mInHal;
    pOutMetaSet = metaSet.mHasOutput ? &outMetaSet : NULL;

    tuning = P2Util::prepareTuning(io, tuningSize, hal3A, inMetaSet, pOutMetaSet, logger);

    if( metaSet.mHasOutput )
    {
        metaSet.mOutApp = outMetaSet.appMeta;
        metaSet.mOutHal = outMetaSet.halMeta;
    }

    TRACE_S_FUNC_EXIT(logger);
}

TuningParam P2Util::prepareTuning(const SimpleIO &io, MUINT32 tuningSize, IHal3A_T *hal3A, MetaSet_T inMetaSet, MetaSet_T *pOutMetaSet, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    TuningParam tuning;

    if( hal3A )
    {
        tuning.pRegBuf = ::malloc(tuningSize);
        if( tuning.pRegBuf == NULL )
        {
            MY_S_LOGW(logger, "OOM: cannot allocate tunning[%d]", tuningSize);
        }
        if( isValid(io.mLCEI) )
        {
            tuning.pLcsBuf = io.mLCEI->getIImageBufferPtr();
        }

        trySet<MUINT8>(inMetaSet.halMeta, MTK_3A_PGN_ENABLE,
                       io.mResized ? 0 : 1);

        if( hal3A->setIsp(0, inMetaSet, &tuning, pOutMetaSet) < 0 )
        {
            MY_S_LOGW(logger, "hal3A->setIsp failed, memset to 0");
            if( tuning.pRegBuf )
            {
                memset(tuning.pRegBuf, 0, tuningSize);
            }
        }
    }

    TRACE_S_FUNC_EXIT(logger);
    return tuning;
}

MVOID P2Util::releaseTuning(TuningParam &tuning, const Logger &logger)
{
    (void)logger;
    TRACE_S_FUNC_ENTER(logger);
    if( tuning.pRegBuf )
    {
        ::free(tuning.pRegBuf);
        tuning.pRegBuf = NULL;
    }
    TRACE_S_FUNC_EXIT(logger);
}

sp<P2Img> P2Util::extractOut(const sp<P2Request> &request, MUINT32 target)
{
    TRACE_S_FUNC_ENTER(request);
    sp<P2Img> out;
    MBOOL useRotate = toBool(target & P2Util::FIND_ROTATE);
    MBOOL checkRotate = useRotate != toBool(target & P2Util::FIND_NO_ROTATE);
    MBOOL useDisp = toBool(target & P2Util::FIND_DISP);
    MBOOL useVideo = toBool(target & P2Util::FIND_VIDEO);
    MBOOL checkType = useDisp || useVideo;
    if( request != NULL )
    {
        MSize max(0, 0);
        auto maxIt = request->mImgOutArray.end();
        for( auto it = request->mImgOutArray.begin(), end = request->mImgOutArray.end(); it != end; ++it )
        {
            if( isValid(*it) )
            {
                if( checkRotate &&
                    (useRotate != toBool((*it)->getTransform())) )
                {
                    continue;
                }
                if( checkType &&
                    !(useDisp && toBool((*it)->isDisplay())) &&
                    !(useVideo && toBool((*it)->isRecord())) )
                {
                    continue;
                }
                MSize size = (*it)->getImgSize();
                if( size.w*size.h > max.w*max.h )
                {
                    max = size;
                    maxIt = it;
                }
            }
        }
        if( maxIt != request->mImgOutArray.end() )
        {
            out = *maxIt;
            (*maxIt) = NULL;
        }
    }
    TRACE_S_FUNC_EXIT(request);
    return out;
}

QParams P2Util::makeSimpleQParams(ENormalStreamTag tag, const SimpleIO &io, const TuningParam &tuning, const Cropper &cropper, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    QParams qparams;
    qparams.mDequeSuccess = MFALSE;

    #if NEW_CODE
    QFRAME_T frame;
    #else
    QFRAME_T &frame = qparams;
    #endif // NEW_CODE

    MUINT32 cropFlag = 0;
    cropFlag |= io.mUseLMV ? Cropper::USE_EIS_12 : 0;
    cropFlag |= io.mUseMargin ? Cropper::USE_MARGIN : 0;
    cropFlag |= io.mResized ? Cropper::USE_RESIZED : 0;

    MSize margin = io.mUseMargin ? io.mMargin : MSize(0,0);

    MUINT32 dipVersion = getDipVersion();

    #if NEW_CODE
    frame.mStreamTag = tag;
    #else
    frame.mvStreamTag.push_back(tag);
    #endif // NEW_CODE
    if( tuning.pRegBuf )
    {
        #if NEW_CODE
        frame.mTuningData = tuning.pRegBuf;
        #else
        frame.mvTuningData.push_back(tuning.pRegBuf);
        #endif // NEW_CODE
    }

    if( isValid(io.mIMGI) )
    {
        prepareIn(frame, PORT_IMGI, io.mIMGI->getIImageBufferPtr());
    }
    if( tuning.pLsc2Buf )
    {
        prepareIn(frame, isDip50(dipVersion) ? PORT_IMGCI : PORT_DEPI, (IImageBuffer*)tuning.pLsc2Buf);
    }
    if( tuning.pBpc2Buf )
    {
        prepareIn(frame, isDip50(dipVersion) ? PORT_IMGBI : PORT_DMGI, (IImageBuffer*)tuning.pBpc2Buf);
    }
    if( tuning.pLcsBuf )
    {
        prepareIn(frame, PORT_LCEI, (IImageBuffer*)tuning.pLcsBuf);
    }

    if( isValid(io.mIMG2O) )
    {
        MCropRect crop = cropper.calcViewAngle(io.mIMG2O->getTransformSize(), cropFlag, margin, logger);
        prepareOut(frame, PORT_IMG2O, io.mIMG2O);
        prepareCrop(frame, CROP_IMG2O, crop, io.mIMG2O->getImgSize());
    }
    if( isValid(io.mWROTO) )
    {
        MCropRect crop = cropper.calcViewAngle(io.mWROTO->getTransformSize(), cropFlag, margin, logger);
        prepareOut(frame, PORT_WROTO, io.mWROTO);
        prepareCrop(frame, CROP_WROTO, crop, io.mWROTO->getImgSize());
    }
    if( isValid(io.mWDMAO) )
    {
        MCropRect crop = cropper.calcViewAngle(io.mWDMAO->getTransformSize(), cropFlag, margin, logger);
        prepareOut(frame, PORT_WDMAO, io.mWDMAO);
        prepareCrop(frame, CROP_WDMAO, crop, io.mWDMAO->getImgSize());
    }

    #if NEW_CODE
    qparams.mvFrameParams.push_back(frame);
    #endif // NEW_CODE

    TRACE_S_FUNC_EXIT(logger);
    return qparams;
}

MVOID P2Util::prepareExtraModule(QParams &qparams, const P2ExtraData &extraData, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    #if NEW_CODE
    if( qparams.mvFrameParams.size() )
    {
        QFRAME_T &frame = qparams.mvFrameParams.editItemAt(0);
        for( unsigned i = 0; i < frame.mvOut.size(); ++i )
        {
            if( frame.mvOut[i].mPortID.capbility == EPortCapbility_Disp )
            {
                /*
                ClearZoomParam *clearZoom = new ClearZoomParam();
                if( clearZoom == NULL )
                {
                    MY_S_LOGW(logger, "OOM: cannot allocate ClearZoomParam");
                }
                else
                {
                    clearZoom->lensId = extraData.mOpenID;
                    clearZoom->captureShot = CAPTURE_SINGLE;
                    clearZoom->FrameNo = extraData.mMWFrameNo;
                    clearZoom->RequestNo = extraData.mMWFrameRequestNo;
                    clearZoom->Timestamp = extraData.mMWUniqueKey;

                    ExtraParam extraParam;
                    extraParam.CmdIdx = EPIPE_CZ_CMD;
                    extraParam.moduleStruct = (void*)clearZoom;
                    frame.mvExtraParam.push_back(extraParam);
                }
                */
            }
        }
    }
    #endif // NEW_CODE
    TRACE_S_FUNC_EXIT(logger);
}

MVOID P2Util::releaseExtraModule(QParams &qparams, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    #if NEW_CODE
    if( qparams.mvFrameParams.size() )
    {
        QFRAME_T &frame = qparams.mvFrameParams.editItemAt(0);
        for( unsigned i = 0; i < frame.mvExtraParam.size(); ++i )
        {
            switch( frame.mvExtraParam[i].CmdIdx )
            {
/*
            case EPIPE_CZ_CMD:
                delete (ClearZoomParam*)frame.mvExtraParam[i].moduleStruct;
                frame.mvExtraParam.editItemAt(i).moduleStruct = NULL;
                break;
*/
            default:
                MY_S_LOGW(logger, "unknown extra module(%d), not released", frame.mvExtraParam[i].CmdIdx);
                break;
            }
        }
    }
    #endif // NEW_CODE
    TRACE_S_FUNC_EXIT(logger);
}

MVOID P2Util::printQParams(const QParams &params, const Logger &logger)
{
    for( unsigned f = 0, fCount = params.mvFrameParams.size(); f < fCount; ++f )
    {
        const FrameParams &frame = params.mvFrameParams[f];
        for( unsigned i = 0, n = frame.mvIn.size(); i < n; ++i )
        {
            unsigned index = frame.mvIn[i].mPortID.index;
            MSize size;
            if( frame.mvIn[i].mBuffer )
            {
                size = frame.mvIn[i].mBuffer->getImgSize();
            }
            MY_S_LOGD(logger, "Run %d of %d mvIn[%d] idx=%d size=(%d,%d)", f, fCount, i, index, size.w, size.h);
        }
        for( unsigned i = 0, n = frame.mvOut.size(); i < n; ++i )
        {
            unsigned index = frame.mvOut[i].mPortID.index;
            MUINT32 cap = frame.mvOut[i].mPortID.capbility;
            MINT32 transform = frame.mvOut[i].mTransform;
            MSize size;
            MINT fmt = 0;
            if( frame.mvOut[i].mBuffer )
            {
                size = frame.mvOut[i].mBuffer->getImgSize();
                fmt = frame.mvOut[i].mBuffer->getImgFormat();
            }
            MY_S_LOGD(logger, "Run %d of %d mvOut[%d] idx=%d size=(%d,%d) cap=%d trans=%d fmt=%d", f, fCount, i, index, size.w, size.h, cap, transform, fmt);
        }
        for( unsigned i = 0, n = frame.mvCropRsInfo.size(); i < n; ++i )
        {
            const MCrpRsInfo &crop = frame.mvCropRsInfo[i];
            MY_S_LOGD(logger, "Run %d of %d crop[%d] group=%d offset=(%d,%d) size=(%d,%d) tar=(%d,%d)", f, fCount, i, crop.mGroupID, crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y, crop.mCropRect.s.w, crop.mCropRect.s.h, crop.mResizeDst.w, crop.mResizeDst.h);
        }
    }
}

MVOID P2Util::updateDebugExif(const IMetadata &inHal, IMetadata &outHal, const Logger &logger)
{
    (void)logger;
    TRACE_S_FUNC_ENTER(logger);
    MUINT8 needExif = 0;
    if( tryGet<MUINT8>(inHal, MTK_HAL_REQUEST_REQUIRE_EXIF, needExif) &&
        needExif )
    {
        MINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
        if( tryGet<MINT32>(inHal, MTK_P1NODE_SENSOR_VHDR_MODE, vhdrMode) &&
            vhdrMode != SENSOR_VHDR_MODE_NONE )
        {
            std::map<MUINT32, MUINT32> debugInfoList;
            debugInfoList[getDebugExif()->getTagId_MF_TAG_IMAGE_HDR()] = 1;

            IMetadata exifMeta;
            tryGet<IMetadata>(outHal, MTK_3A_EXIF_METADATA, exifMeta);
            if( DebugExifUtils::setDebugExif(
                    DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
                    static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
                    static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
                    debugInfoList, &exifMeta) != NULL )
            {
                trySet<IMetadata>(outHal, MTK_3A_EXIF_METADATA, exifMeta);
            }
        }
    }
    TRACE_S_FUNC_EXIT(logger);
}

MVOID P2Util::updateExtraMeta(const P2ExtraData &extraData, IMetadata &outHal, const Logger &logger)
{
    TRACE_S_FUNC_ENTER(logger);
    trySet<MINT32>(outHal, MTK_PIPELINE_FRAME_NUMBER, extraData.mMWFrameNo);
    trySet<MINT32>(outHal, MTK_PIPELINE_REQUEST_NUMBER, extraData.mMWFrameRequestNo);
    TRACE_S_FUNC_EXIT(logger);
}

EPortCapbility P2Util::toCapability(MUINT32 usage)
{
    EPortCapbility cap = EPortCapbility_None;
    if( usage & (GRALLOC_USAGE_HW_COMPOSER|GRALLOC_USAGE_HW_TEXTURE) )
    {
        cap = EPortCapbility_Disp;
    }
    else if( usage & GRALLOC_USAGE_HW_VIDEO_ENCODER )
    {
        cap = EPortCapbility_Rcrd;
    }
    return cap;
}


MVOID P2Util::prepareIn(QFRAME_T &frame, const PortID &portID, IImageBuffer *buffer)
{
    Input input;
    input.mPortID = portID,
    input.mPortID.group = 0;
    input.mBuffer = buffer;
    frame.mvIn.push_back(input);
}

MVOID P2Util::prepareOut(QFRAME_T &frame, const PortID &portID, const sp<P2Img> &img)
{
    Output output;
    output.mPortID = portID;
    output.mPortID.group = 0;
    output.mPortID.capbility = toCapability(img->getUsage());
    output.mTransform = img->getTransform();
    output.mBuffer = img->getIImageBufferPtr();
    frame.mvOut.push_back(output);
}

MVOID P2Util::prepareCrop(QFRAME_T &frame, MUINT32 cropID, const MCropRect &crop, const MSize size)
{
    MCrpRsInfo cropInfo;
    cropInfo.mGroupID = cropID;
    cropInfo.mCropRect = crop;
    cropInfo.mResizeDst = size;
    frame.mvCropRsInfo.push_back(cropInfo);
}

MBOOL P2Util::updateCropRegion(IMetadata *meta, const Cropper &cropper)
{
    MBOOL ret = MFALSE;
    if( meta && cropper.isValid() )
    {
        MRect cropRegion = cropper.getCropRegion();
        ret = trySet<MRect>(*meta, MTK_SCALER_CROP_REGION, cropRegion);
    }
    return ret;
}

MBOOL P2Util::is4K2K(const MSize &size)
{
    #define UHD_VR_WIDTH  (3840)
    #define UHD_VR_HEIGHT (2160)
    MBOOL is4K = (size.w >= UHD_VR_WIDTH && size.h >= UHD_VR_HEIGHT);
    #undef UHD_VR_WIDTH
    #undef UHD_VR_HEIGHT
    return is4K;
}

MUINT32 P2Util::getDipVersion()
{
    std::map<NSCam::NSIoPipe::EDIPInfoEnum, MUINT32> dipInfo;
    dipInfo[NSCam::NSIoPipe::EDIPINFO_DIPVERSION] = NSCam::NSIoPipe::EDIPHWVersion_40;
    MBOOL r = NSCam::NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(dipInfo);
    if (!r)
    {
        MY_LOGE("queryDIPInfo fail!");
    }
    return dipInfo[NSCam::NSIoPipe::EDIPINFO_DIPVERSION];
}

MBOOL P2Util::isDip50(MUINT32 version)
{
    return version == NSCam::NSIoPipe::EDIPHWVersion_50;
}


} // namespace P2
