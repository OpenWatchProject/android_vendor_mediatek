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

#include <mtkcam/custom/ExifFactory.h>
#include <mtkcam/def/common.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam3/feature/utils/p2/P2IOClassfier.h>
#include <mtkcam3/feature/lcenr/lcenr.h>
#include <DpDataType.h>

#include <mtkcam3/feature/utils/p2/P2Util.h>
#include <mtkcam3/feature/utils/p2/P2Trace.h>
#define ILOG_MODULE_TAG P2Util
#include <mtkcam3/feature/utils/log/ILogHeader.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

using android::sp;
using NS3Av3::TuningParam;
using NS3Av3::IHal3A;
using NS3Av3::IHalISP;
using NS3Av3::MetaSet_T;
using NSCam::NSIoPipe::NSPostProc::INormalStream;
using NSCam::NSIoPipe::EPortCapbility;
using NSCam::NSIoPipe::EPortCapbility_None;
using NSCam::NSIoPipe::EPortCapbility_Disp;
using NSCam::NSIoPipe::EPortCapbility_Rcrd;
using NSCam::NSIoPipe::ModuleInfo;
using NSCam::NSIoPipe::EDIPInfoEnum;

using NSImageio::NSIspio::EPortIndex;
using NSImageio::NSIspio::EPortIndex_IMGI;
using NSImageio::NSIspio::EPortIndex_IMGBI;
using NSImageio::NSIspio::EPortIndex_IMGCI;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSImageio::NSIspio::EPortIndex_DEPI;
using NSImageio::NSIspio::EPortIndex_LCEI;
using NSImageio::NSIspio::EPortIndex_DMGI;
using NSImageio::NSIspio::EPortIndex_BPCI;
using NSImageio::NSIspio::EPortIndex_LSCI;
using NSImageio::NSIspio::VirDIPPortIdx_YNR_FACEI;
using NSImageio::NSIspio::VirDIPPortIdx_YNR_LCEI;
using NSImageio::NSIspio::EPortIndex_IMG2O;
using NSImageio::NSIspio::EPortIndex_IMG3O;
using NSImageio::NSIspio::EPortIndex_WDMAO;
using NSImageio::NSIspio::EPortIndex_WROTO;
using NSImageio::NSIspio::EPortIndex_DCESO;

using NSCam::NSIoPipe::PORT_IMGI;
using NSCam::NSIoPipe::PORT_IMGBI;
using NSCam::NSIoPipe::PORT_IMGCI;
using NSCam::NSIoPipe::PORT_VIPI;
using NSCam::NSIoPipe::PORT_DEPI;
using NSCam::NSIoPipe::PORT_LCEI;
using NSCam::NSIoPipe::PORT_DMGI;
using NSCam::NSIoPipe::PORT_BPCI;
using NSCam::NSIoPipe::PORT_LSCI;
using NSCam::NSIoPipe::PORT_YNR_FACEI;
using NSCam::NSIoPipe::PORT_YNR_LCEI;
using NSCam::NSIoPipe::PORT_IMG2O;
using NSCam::NSIoPipe::PORT_IMG3O;
using NSCam::NSIoPipe::PORT_WDMAO;
using NSCam::NSIoPipe::PORT_WROTO;
using NSCam::NSIoPipe::PORT_DCESO;

using NSCam::NSIoPipe::FEInfo;
using NSCam::NSIoPipe::FMInfo;
using NSCam::NSIoPipe::CrspInfo;



namespace NSCam {
namespace Feature {
namespace P2Util {

/*******************************************
Static Internal function
*******************************************/

const char* toName(PqDebugIndex index)
{
    switch( index )
    {
        case PQ_DEBUG_S_P2A:         return "s-p2a";
        case PQ_DEBUG_VMDP:          return "vmdp";
        default:                     return "unknown";
    };
}

static MVOID initBuffer(IImageBuffer *buffer)
{
    if(buffer)
    {
        for (size_t i = 0; i < buffer->getPlaneCount(); i++)
        {
            size_t size = buffer->getBufSizeInBytes(i);
            memset( (void*)buffer->getBufVA(i), 0, size);
        }
    }
}

static MUINT32 getCropGroupIDByIndex(MUINT32 port)
{
    if( port == EPortIndex_WDMAO )
        return CROP_WDMAO;
    if( port == EPortIndex_WROTO )
        return CROP_WROTO;
    if( port == EPortIndex_IMG2O )
        return CROP_IMG2O;
    MY_LOGE("[ERROR] Can not find crop group for port(%d)!", port);
    return 0;
}

static MUINT32 getCropGroupID(const NSCam::NSIoPipe::PortID &portID)
{
    return getCropGroupIDByIndex(portID.index);
}

static MBOOL isExistPort(const DIPFrameParams &f, const PortID &portID)
{
    for(auto&& out : f.mvOut)
    {
        if(out.mPortID.index == portID.index)
        {
            return MTRUE;
        }
    }
    return MFALSE;
}

static MVOID* findPqParamPtr(const DIPFrameParams &param)
{
    for(auto&& ext : param.mvExtraParam)
    {
        if(ext.CmdIdx == NSCam::NSIoPipe::EPIPE_MDP_PQPARAM_CMD)
        {
            return ext.moduleStruct;
        }
    }
    return NULL;
}

static MVOID addNewFrame(DIPParams &dipParam, MUINT32 refFrameInd)
{
    dipParam.mvDIPFrameParams.push_back(dipParam.mvDIPFrameParams.at(refFrameInd));
    DIPFrameParams &newF = dipParam.mvDIPFrameParams.at(dipParam.mvDIPFrameParams.size() - 1);

    newF.mvOut.clear();
    newF.mvCropRsInfo.clear();
    newF.mvExtraParam.clear();

    Output dceso;
    if(findOutput(dipParam.mvDIPFrameParams.at(refFrameInd), PORT_DCESO, dceso))
    {
        newF.mvOut.push_back(dceso);
    }
}

static MVOID clearPQParam(MVOID* pqParam)
{
    if(pqParam != NULL)
    {
        PQParam *p = static_cast<PQParam*>(pqParam);
        p->WDMAPQParam = NULL;
        p->WROTPQParam = NULL;
    }
}

static MVOID addPQParam(DIPFrameParams &frame, MVOID* pqParam)
{
    ExtraParam extra;
    extra.CmdIdx = NSIoPipe::EPIPE_MDP_PQPARAM_CMD;
    extra.moduleStruct = pqParam;
    frame.mvExtraParam.push_back(extra);
}

static MVOID pushPQParam(DIPFrameParams &frame, MVOID* pqParam, MVOID* dpParam, MUINT32 port)
{
    if(dpParam == NULL || pqParam == NULL)
        return;
    if(port != EPortIndex_WDMAO && port != EPortIndex_WROTO)
        return;

    MVOID* targetParam = findPqParamPtr(frame);

    if(targetParam == NULL)
    {
        addPQParam(frame, pqParam);
        clearPQParam(pqParam);
        targetParam = pqParam;
    }

    PQParam *p = static_cast<PQParam*>(targetParam);
    if(port == NSImageio::NSIspio::EPortIndex_WDMAO)
        p->WDMAPQParam = dpParam;
    else if(port == NSImageio::NSIspio::EPortIndex_WROTO)
        p->WROTPQParam = dpParam;

}

static MVOID prepareFrame(DIPFrameParams &frame, P2IOClassfier &collect)
{
    // put in wdma port
    PortID target = PORT_WDMAO;
    if( !isExistPort(frame, target) && collect.hasNonRotateIO())
    {
        pushToMDP(frame, target, collect.popNonRotate());
    }

    // put in wrot port
    target = PORT_WROTO;
    if( !isExistPort(frame, target))
    {
        if( collect.hasRotateIO())
        {
            pushToMDP(frame, target, collect.popRotate());
        }
        else if(collect.hasNonRotateIO())
        {
            pushToMDP(frame, target, collect.popNonRotate());
        }
    }
}

static MVOID refinePQParam(std::vector<P2IO> &mdpOuts, const MDPObjPtr &mdpObj)
{
    for(P2IO &out : mdpOuts)
    {
        if(mdpObj.ispTuningBuf)
        {
            DpPqParam* pq = (DpPqParam*)out.mDpPqParam;
            if( pq && (pq->enable & PQ_HFG_EN) )
            {
                pq->u.isp.dpHFGParam.p_slkParam = mdpObj.ispTuningBuf;
            }
        }
    }
}

/*******************************************
Common function
*******************************************/

MBOOL is4K2K(const MSize &size)
{
    const MINT32 UHD_VR_WIDTH = 3840;
    const MINT32 UHD_VR_HEIGHT = 2160;
    return (size.w >= UHD_VR_WIDTH && size.h >= UHD_VR_HEIGHT);
}

MCropRect getCropRect(const MRectF &rectF) {
    #define MAX_MDP_FRACTION_BIT (20) // MDP use 20bits
    MCropRect cropRect(rectF.p.toMPoint(), rectF.s.toMSize());
    cropRect.p_fractional.x = (rectF.p.x-cropRect.p_integral.x) * (1<<MAX_MDP_FRACTION_BIT);
    cropRect.p_fractional.y = (rectF.p.y-cropRect.p_integral.y) * (1<<MAX_MDP_FRACTION_BIT);
    cropRect.w_fractional = (rectF.s.w-cropRect.s.w) * (1<<MAX_MDP_FRACTION_BIT);
    cropRect.h_fractional = (rectF.s.h-cropRect.s.h) * (1<<MAX_MDP_FRACTION_BIT);
    return cropRect;
}

auto getDebugExif()
{
    static auto const sInst = MAKE_DebugExif();
    return sInst;
}

/*******************************************
P2IO function
*******************************************/
Output toOutput(const P2IO &io)
{
    Output out;
    out.mBuffer = io.mBuffer;
    out.mTransform = io.mTransform;
    out.mPortID.capbility = io.mCapability;
    return out;
}

Output toOutput(const P2IO &io, MUINT32 index)
{
    Output out = toOutput(io);
    out.mPortID.index = index;
    return out;
}

MCropRect toMCropRect(const P2IO &io)
{
    MCropRect crop = getCropRect(io.mCropRect);
    applyDMAConstrain(crop, io.mDMAConstrain);
    return crop;
}

MCrpRsInfo toMCrpRsInfo(const P2IO &io)
{
    MCrpRsInfo info;
    info.mCropRect = toMCropRect(io);
    info.mResizeDst = io.mCropDstSize;
    return info;
}

MVOID applyDMAConstrain(MCropRect &crop, MUINT32 constrain)
{
    if( constrain & (DMAConstrain::NOSUBPIXEL|DMAConstrain::ALIGN2BYTE) )
    {
        crop.p_fractional.x = 0;
        crop.p_fractional.y = 0;
        crop.w_fractional = 0;
        crop.h_fractional = 0;
        if( constrain & DMAConstrain::ALIGN2BYTE )
        {
            crop.p_integral.x &= (~1);
            crop.p_integral.y &= (~1);
        }
    }
}

/*******************************************
Tuning function
*******************************************/

void* allocateRegBuffer(MBOOL zeroInit)
{
    MINT32 bCount = INormalStream::getRegTableSize();
    void *buffer = NULL;
    if(bCount > 0)
    {
        buffer = ::malloc(bCount);
        if( buffer && zeroInit )
        {
            memset(buffer, 0, bCount);
        }
    }
    return buffer;
}

MVOID releaseRegBuffer(void* &buffer)
{
    ::free(buffer);
    buffer = NULL;
}

TuningParam makeTuningParam(const ILog &log, const P2Pack &p2Pack, IHalISP *halISP, MetaSet_T &inMetaSet, MetaSet_T *pOutMetaSet, MBOOL resized, void *regBuffer, IImageBuffer *lcso, MINT32 dcesoMagic, IImageBuffer *dceso, MBOOL isSlave, void *syncTuningBuf, IImageBuffer *lcsho)
{
    (void)log;
    TRACE_S_FUNC_ENTER(log);
    TuningParam tuning;
    tuning.pRegBuf = regBuffer;
    tuning.pLcsBuf = lcso;
    tuning.pLceshoBuf = lcsho;
    tuning.pDcsBuf = (dcesoMagic >= 0) ? dceso : NULL;
    tuning.i4DcsMagicNo = dcesoMagic;
    tuning.bSlave = isSlave;
    tuning.pDualSynInfo = syncTuningBuf;

    if(dceso)
    {
        dceso->syncCache(eCACHECTRL_INVALID);
    }
    if(lcsho)
    {
        lcsho->syncCache(eCACHECTRL_INVALID);
    }

    trySet<MUINT8>(inMetaSet.halMeta, MTK_3A_PGN_ENABLE,
                   resized ? 0 : 1);
    P2_CAM_TRACE_BEGIN(TRACE_DEFAULT, "P2Util:Tuning");
    if( halISP && regBuffer )
    {
        MINT32 ret3A = halISP->setP2Isp(0, inMetaSet, &tuning, pOutMetaSet);
        if( ret3A < 0 )
        {
            MY_S_LOGW(log, "hal3A->setIsp failed, memset regBuffer to 0");
            if( tuning.pRegBuf )
            {
                memset(tuning.pRegBuf, 0, INormalStream::getRegTableSize());
            }
        }
        if( pOutMetaSet )
        {
            updateExtraMeta(p2Pack, pOutMetaSet->halMeta);
            updateDebugExif(p2Pack, inMetaSet.halMeta, pOutMetaSet->halMeta);
        }
    }
    else
    {
        MY_S_LOGE(log, "cannot run setIsp: hal3A=%p reg=%p", halISP, regBuffer);
    }
    P2_CAM_TRACE_END(TRACE_DEFAULT);

    TRACE_S_FUNC_EXIT(log);
    return tuning;
}

/*******************************************
Metadata function
*******************************************/

MVOID updateExtraMeta(const P2Pack &p2Pack, IMetadata &outHal)
{
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_S_FUNC_ENTER(p2Pack.mLog);
    trySet<MINT32>(outHal, MTK_PIPELINE_FRAME_NUMBER, p2Pack.getFrameData().mMWFrameNo);
    trySet<MINT32>(outHal, MTK_PIPELINE_REQUEST_NUMBER, p2Pack.getFrameData().mMWFrameRequestNo);
    TRACE_S_FUNC_EXIT(p2Pack.mLog);
}

MVOID updateDebugExif(const P2Pack &p2Pack, const IMetadata &inHal, IMetadata &outHal)
{
    (void)p2Pack;
    P2_CAM_TRACE_CALL(TRACE_ADVANCED);
    TRACE_S_FUNC_ENTER(p2Pack.mLog);
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
    TRACE_S_FUNC_EXIT(p2Pack.mLog);
}

MBOOL updateCropRegion(IMetadata &outHal, const MRect &rect)
{
    // TODO: set output crop w/ margin setting
    MBOOL ret = MFALSE;
    ret = trySet<MRect>(outHal, MTK_SCALER_CROP_REGION, rect);
    return ret;
}

/*******************************************
DIPParams util function
*******************************************/

EPortCapbility toCapability(MUINT32 usage)
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

const char* toName(EPortIndex index)
{
    switch( index )
    {
    case EPortIndex_IMGI:           return "imgi";
    case EPortIndex_IMGBI:          return "imgbi";
    case EPortIndex_IMGCI:          return "imgci";
    case EPortIndex_VIPI:           return "vipi";
    case EPortIndex_DEPI:           return "depi";
    case EPortIndex_LCEI:           return "lcei";
    case EPortIndex_DMGI:           return "dmgi";
    case EPortIndex_BPCI:           return "bpci";
    case EPortIndex_LSCI:           return "lsci";
    case VirDIPPortIdx_YNR_FACEI:   return "ynr_facei";
    case VirDIPPortIdx_YNR_LCEI:    return "ynr_lcei";
    case EPortIndex_IMG2O:          return "img2o";
    case EPortIndex_IMG3O:          return "img3o";
    case EPortIndex_WDMAO:          return "wdmao";
    case EPortIndex_WROTO:          return "wroto";
    case EPortIndex_DCESO:          return "dceso";
    default:                        return "unknown";
    };
    return NULL;
}

const char* toName(MUINT32 index)
{
    return toName((EPortIndex)index);
}

const char* toName(const NSCam::NSIoPipe::PortID &port)
{
    return toName((EPortIndex)port.index);
}

const char* toName(const Input &input)
{
    return toName((EPortIndex)input.mPortID.index);
}

const char* toName(const Output &output)
{
    return toName((EPortIndex)output.mPortID.index);
}

MBOOL is(const PortID &port, EPortIndex index)
{
    return port.index == index;
}

MBOOL is(const Input &input, EPortIndex index)
{
    return input.mPortID.index == index;
}

MBOOL is(const Output &output, EPortIndex index)
{
    return output.mPortID.index == index;
}

MBOOL is(const PortID &port, const PortID &rhs)
{
    return port.index == rhs.index;
}

MBOOL is(const Input &input, const PortID &rhs)
{
    return input.mPortID.index == rhs.index;
}

MBOOL is(const Output &output, const PortID &rhs)
{
    return output.mPortID.index == rhs.index;
}

MBOOL findInput(const DIPFrameParams &frame, const PortID &portID, Input &input)
{
    MBOOL ret = MFALSE;
    for(const Input &i : frame.mvIn)
    {
        if(is(i, portID))
        {
            input = i;
            ret = MTRUE;
            break;
        }
    }
    return ret;
}

MBOOL findOutput(const DIPFrameParams &frame, const PortID &portID, Output &output)
{
    MBOOL ret = MFALSE;
    for(const Output &o : frame.mvOut)
    {
        if(is(o, portID))
        {
            output = o;
            ret = MTRUE;
            break;
        }
    }
    return ret;
}

IImageBuffer* findInputBuffer(const DIPFrameParams &frame, const PortID &portID)
{
    IImageBuffer *buffer = NULL;
    Input input;
    if(findInput(frame, portID, input))
    {
        buffer = input.mBuffer;
    }

    return buffer;
}

MBOOL isGraphicBuffer(const IImageBuffer *imageBuffer)
{
    IImageBufferHeap *imageBufferHeap = (imageBuffer == NULL) ? NULL : imageBuffer->getImageBufferHeap();
    return ((imageBufferHeap != NULL) && (imageBufferHeap->getHWBuffer() != NULL));
}

MVOID printDIPParams(const ILog &log, unsigned i, const Input &input)
{
    unsigned index = input.mPortID.index;
    if( input.mBuffer != NULL)
    {
        MSize size = input.mBuffer->getImgSize();
        MINT fmt = input.mBuffer->getImgFormat();
        MINT32 transform = input.mTransform;
        MINTPTR pa = input.mBuffer->getBufPA(0);
        MINTPTR va = input.mBuffer->getBufVA(0);
        MUINTPTR secHandle = input.mSecHandle;
        S_LOGD(log, "[PrintQ]: mvIn[%d] idx=%d(%s) size=(%dx%d) fmt=0x%08x transform=%d pa=0x%016" PRIxPTR " va=0x%016" PRIxPTR " sec=0x%016" PRIxPTR, i, index, toName(index), size.w, size.h, fmt, transform, pa, va, secHandle);
    }
    else
    {
        S_LOGD(log, "[PrintQ]: mvIn[%d] idx=%d(%s) buffer is NULL", i, index, toName(index));
    }
}

MVOID printDIPParams(const ILog &log, unsigned i, const Output &output)
{
    unsigned index = output.mPortID.index;
    if( output.mBuffer != NULL)
    {
        MSize size = output.mBuffer->getImgSize();
        MBOOL isGraphic = isGraphicBuffer(output.mBuffer);
        MINT fmt = output.mBuffer->getImgFormat();
        MUINT32 cap = output.mPortID.capbility;
        MINT32 transform = output.mTransform;
        MINTPTR pa = output.mBuffer->getBufPA(0);
        MINTPTR va = output.mBuffer->getBufVA(0);
        S_LOGD(log, "[PrintQ]: mvOut[%d] idx=%d(%s) size=(%dx%d) fmt=0x%08x, cap=%02x, isGraphic=%d transform=%d pa=0x%016" PRIxPTR " va=0x%16" PRIxPTR, i, index, toName(index), size.w, size.h, fmt, cap, isGraphic, transform, pa, va);
    }
    else
    {
        S_LOGD(log, "[PrintQ]: mvOut[%d] idx=%d(%s) buffer is NULL", i, index, toName(index));
    }
}

MVOID printDIPParams(const ILog &log, unsigned i, const MCrpRsInfo &crop)
{
    S_LOGD(log, "[PrintQ]: crop[%d] groupID=%d frameGroup=%d i(%dx%d) f(%dx%d) s(%dx%d) r(%dx%d)", i, crop.mGroupID, crop.mFrameGroup, crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
        crop.mCropRect.p_fractional.x, crop.mCropRect.p_fractional.y, crop.mCropRect.s.w, crop.mCropRect.s.h, crop.mResizeDst.w, crop.mResizeDst.h);

}

MVOID printSRZ(const ILog &log, unsigned i, unsigned srzNum, const _SRZ_SIZE_INFO_ *sizeInfo)
{
    if(sizeInfo != NULL)
    {
        S_LOGD(log, "[PrintQ]: moduleInfo[%d] SRZ%d in(%dx%d) crop(%lux%lu) crop_start=(%d.%u, %d.%u) out(%dx%d)", i, srzNum, sizeInfo->in_w, sizeInfo->in_h, sizeInfo->crop_w, sizeInfo->crop_h,
            sizeInfo->crop_x, sizeInfo->crop_floatX, sizeInfo->crop_y, sizeInfo->crop_floatY, sizeInfo->out_w, sizeInfo->out_h);
    }
    else
    {
        S_LOGD(log, "[PrintQ]: moduleInfo[%d] SRZ%d moduleStruct is NULL", i, srzNum);
    }
}

MVOID printDIPParams(const ILog &log, unsigned i, const ModuleInfo &info)
{
    _SRZ_SIZE_INFO_ *sizeInfo = (_SRZ_SIZE_INFO_*)info.moduleStruct;
    switch(info.moduleTag)
    {
        case EDipModule_SRZ1:
            printSRZ(log, i, 1, sizeInfo);
            break;
        case EDipModule_SRZ3:
            printSRZ(log, i, 3, sizeInfo);
            break;
        case EDipModule_SRZ4:
            printSRZ(log, i, 4, sizeInfo);
            break;
        default:
            S_LOGD(log, "[PrintQ]: moduleInfo[%d] unknow tag(%d)", i, info.moduleTag);
            break;
    };
}

MVOID printFeInfo(const ILog &log, unsigned i, FEInfo *feInfo)
{
    if(feInfo != NULL)
    {
        S_LOGD(log, "[PrintQ]: extra[%d] FE_CMD DSCR_SBIT=%d TH_C=%d TH_G=%d FLT_EN=%d PARAM=%d MODE=%d YIDX=%d XIDX=%d START_X=%d START_Y=%d IN_HT=%d IN_WD=%d", i,
            feInfo->mFEDSCR_SBIT, feInfo->mFETH_C, feInfo->mFETH_G, feInfo->mFEFLT_EN, feInfo->mFEPARAM, feInfo->mFEMODE, feInfo->mFEYIDX, feInfo->mFEXIDX, feInfo->mFESTART_X, feInfo->mFESTART_Y, feInfo->mFEIN_HT, feInfo->mFEIN_WD);
    }
}

MVOID printFmInfo(const ILog &log, unsigned i, FMInfo *fmInfo)
{
    if(fmInfo != NULL)
    {
        S_LOGD(log, "[PrintQ]: extra[%d] FM_CMD HEIGHT=%d WIDTH=%d SR_TYPE=%d OFFSET_X=%d OFFSET_Y=%d RES_TH=%d SAD_TH=%d MIN_RATIO=%d", i,
            fmInfo->mFMHEIGHT, fmInfo->mFMWIDTH, fmInfo->mFMSR_TYPE, fmInfo->mFMOFFSET_X, fmInfo->mFMOFFSET_Y, fmInfo->mFMRES_TH, fmInfo->mFMSAD_TH, fmInfo->mFMMIN_RATIO);
    }
}

MVOID printDIPParamsInfo(const ILog &log, unsigned i, PQParam *pqParam)
{
    if(pqParam != NULL)
    {
        S_LOGD(log, "[PrintQ]: extra[%d] PQ_CMD pqParam=%p pqParam.WDMA=%p pqParam.WROT=%p", i, pqParam, pqParam->WDMAPQParam, pqParam->WROTPQParam);
        if( pqParam->WDMAPQParam )
        {
            DpPqParam *pq = (DpPqParam*) pqParam->WDMAPQParam;
            S_LOGD(log, "[PrintQ]: extra[%d] PQ_WDMA_CMD scenario=%d enable=%d isp.iso=%d isp.LV=%d isp.p_faceInfor=%p cz.captureShot=%d cz.p_customSetting=%p dre.cmd=%d dre.userId=%llu dre.buffer=%p dre.p_customSetting=%p dre.customIndex=%d ",
                i, pq->scenario, pq->enable, pq->u.isp.iso, pq->u.isp.LV, pq->u.isp.p_faceInfor, pq->u.isp.clearZoomParam.captureShot, pq->u.isp.clearZoomParam.p_customSetting,
                pq->u.isp.dpDREParam.cmd, pq->u.isp.dpDREParam.userId, pq->u.isp.dpDREParam.buffer, pq->u.isp.dpDREParam.p_customSetting, pq->u.isp.dpDREParam.customIndex);
        }
        if( pqParam->WROTPQParam )
        {
            DpPqParam *pq = (DpPqParam*) pqParam->WROTPQParam;
            S_LOGD(log, "[PrintQ]: extra[%d] PQ_WROT_CMD scenario=%d enable=%d isp.iso=%d isp.LV=%d isp.p_faceInfor=%p cz.captureShot=%d cz.p_customSetting=%p dre.cmd=%d dre.userId=%llu dre.buffer=%p dre.p_customSetting=%p dre.customIndex=%d ",
                i, pq->scenario, pq->enable, pq->u.isp.iso, pq->u.isp.LV, pq->u.isp.p_faceInfor, pq->u.isp.clearZoomParam.captureShot, pq->u.isp.clearZoomParam.p_customSetting,
                pq->u.isp.dpDREParam.cmd, pq->u.isp.dpDREParam.userId, pq->u.isp.dpDREParam.buffer, pq->u.isp.dpDREParam.p_customSetting, pq->u.isp.dpDREParam.customIndex);
        }
    }
}

MVOID printCrspInfo(const ILog &log, unsigned i, CrspInfo *crspInfo)
{
    if(crspInfo != NULL)
    {
        S_LOGD(log, "[PrintQ]: extra[%d] CRSPINFO_CMD OFFSET_X=%d OFFSET_Y=%d WIDTH=%d HEIGHT=%d", i, crspInfo->m_CrspInfo.p_integral.x, crspInfo->m_CrspInfo.p_integral.y, crspInfo->m_CrspInfo.s.w, crspInfo->m_CrspInfo.s.h);
    }
}

MVOID printDIPParams(const ILog &log, unsigned i, const ExtraParam &ext)
{
    switch(ext.CmdIdx)
    {
        case NSIoPipe::EPIPE_FE_INFO_CMD:         printFeInfo(log, i, (FEInfo *) ext.moduleStruct);         break;
        case NSIoPipe::EPIPE_FM_INFO_CMD:         printFmInfo(log, i, (FMInfo *) ext.moduleStruct);         break;
        case NSIoPipe::EPIPE_MDP_PQPARAM_CMD:     printDIPParamsInfo(log, i, (PQParam *) ext.moduleStruct);   break;
        case NSIoPipe::EPIPE_IMG3O_CRSPINFO_CMD:  printCrspInfo(log, i, (CrspInfo *) ext.moduleStruct);     break;
        default:                                  S_LOGD(log, "[PrintQ]: unknown ext: idx=%d", ext.CmdIdx); break;
    };
}

MVOID printDIPParams(const ILog &log, const DIPParams &params)
{
    for( unsigned f = 0, fCount = params.mvDIPFrameParams.size(); f < fCount; ++f )
    {
        const DIPFrameParams &frame = params.mvDIPFrameParams[f];
        struct timeval curr;
        gettimeofday(&curr, NULL);
        S_LOGD(log, "[PrintQ]: DIPParams frame(%d/%d) FrameNo=%u RequestNo=%u Timestamp=%u UniqueKey=%d StreamTag=%d SensorIdx=%d SecureFra=%d", f, fCount, frame.FrameNo, frame.RequestNo, frame.Timestamp, frame.UniqueKey, frame.mStreamTag, frame.mSensorIdx, frame.mSecureFra);
        S_LOGD(log, "[PrintQ]: DIPParams frame(%d/%d) mTuningData=%p CurrentTime=%d.%03d EndTime=%d.%03d", f, fCount, frame.mTuningData,(MUINT32)curr.tv_sec%1000, (MUINT32)curr.tv_usec/1000, (MUINT32)frame.ExpectedEndTime.tv_sec%1000, (MUINT32)frame.ExpectedEndTime.tv_usec/1000);

        for( unsigned i = 0, n = frame.mvIn.size(); i < n; ++i )
        {
            printDIPParams(log, i, frame.mvIn[i]);
        }
        for( unsigned i = 0, n = frame.mvOut.size(); i < n; ++i )
        {
            printDIPParams(log, i, frame.mvOut[i]);
        }
        for( unsigned i = 0, n = frame.mvCropRsInfo.size(); i < n; ++i )
        {
            printDIPParams(log, i, frame.mvCropRsInfo[i]);
        }
        for( unsigned i = 0, n = frame.mvModuleData.size(); i < n; ++i )
        {
            printDIPParams(log, i, frame.mvModuleData[i]);
        }
        for( unsigned i = 0, n = frame.mvExtraParam.size(); i < n; ++i )
        {
            printDIPParams(log, i, frame.mvExtraParam[i]);
        }
    }
}

MVOID printTuningParam(const ILog &log, const TuningParam &tuning)
{
    MY_S_LOGD(log, "reg=%p lcs=%p", tuning.pRegBuf, tuning.pLcsBuf);
}

MVOID push_in(DIPFrameParams &frame, const PortID &portID, IImageBuffer *buffer)
{
    Input input;
    input.mPortID = portID,
    input.mPortID.group = 0;
    input.mBuffer = buffer;
    frame.mvIn.push_back(input);
}

MVOID push_in(DIPFrameParams &frame, const PortID &portID, const P2IO &in)
{
    push_in(frame, portID, in.mBuffer);
}

MVOID push_out(DIPFrameParams &frame, const PortID &portID, IImageBuffer *buffer)
{
    push_out(frame, portID, buffer, EPortCapbility_None, 0);
}

MVOID push_out(DIPFrameParams &frame, const PortID &portID, IImageBuffer *buffer, EPortCapbility cap, MINT32 transform)
{
    Output output;
    output.mPortID = portID;
    output.mPortID.group = 0;
    output.mPortID.capbility = cap;
    output.mTransform = transform;
    output.mBuffer = buffer;
    frame.mvOut.push_back(output);
}

MVOID push_out(DIPFrameParams &frame, const PortID &portID, const P2IO &out)
{
    push_out(frame, portID, out.mBuffer, out.mCapability, out.mTransform);
}

MVOID push_crop(DIPFrameParams &frame, MUINT32 cropID, const MCropRect &crop, const MSize &size)
{
    MCrpRsInfo cropInfo;
    cropInfo.mGroupID = cropID;
    cropInfo.mCropRect = crop;
    cropInfo.mResizeDst = size;
    frame.mvCropRsInfo.push_back(cropInfo);
}

MVOID push_crop(DIPFrameParams &frame, MUINT32 cropID, const MRectF &crop, const MSize &size, const MUINT32 dmaConstrain)
{
    MCrpRsInfo cropInfo;
    cropInfo.mGroupID = cropID;
    cropInfo.mCropRect = getCropRect(crop);

    if( (dmaConstrain & DMAConstrain::NOSUBPIXEL) ||
        (dmaConstrain & DMAConstrain::ALIGN2BYTE) )
    {
        cropInfo.mCropRect.p_fractional.x = 0;
        cropInfo.mCropRect.p_fractional.y = 0;
        cropInfo.mCropRect.w_fractional = 0;
        cropInfo.mCropRect.h_fractional = 0;
        if( dmaConstrain & DMAConstrain::ALIGN2BYTE )
        {
            cropInfo.mCropRect.p_integral.x &= (~1);
            cropInfo.mCropRect.p_integral.y &= (~1);
        }
    }
    cropInfo.mResizeDst = size;
    frame.mvCropRsInfo.push_back(cropInfo);
}

MVOID pushToMDP(DIPFrameParams &f, const PortID &portID, const P2IO &output)
{
    push_out(f, portID, output);
    if(output.isCropValid())
    {
        push_crop(f, getCropGroupID(portID), output.mCropRect, output.mCropDstSize, output.mDMAConstrain);
    }
    pushPQParam(f, output.mPqParam, output.mDpPqParam, portID.index);
}
/*******************************************
DIPParams function
*******************************************/

MBOOL push_srz4_LCENR(DIPFrameParams &frame, _SRZ_SIZE_INFO_ *srz4, const P2Pack &p2Pack, MBOOL resized, const MSize &imgiSize, const MSize &lcsoSize)
{
    if( srz4 )
    {
        LCENR_IN_PARAMS in;
        LCENR_OUT_PARAMS out;
        const P2SensorData &data = p2Pack.getSensorData();

        in.resized = resized;
        in.p2_in = imgiSize;
        in.rrz_in = data.mP1BinSize;
        in.rrz_crop_in = data.mP1BinCrop;
        in.rrz_out = data.mP1OutSize;
        in.lce_full = lcsoSize;
        calculateLCENRConfig(in, out);

        *srz4 = out.srz4Param;

        ModuleInfo info;
        info.moduleTag = EDipModule_SRZ4;
        info.frameGroup = 0;
        info.moduleStruct = reinterpret_cast<MVOID*>(srz4);
        frame.mvModuleData.push_back(info);
    }
    return (srz4 != NULL);
}

MBOOL push_srz3_FACENR(DIPFrameParams &frame, _SRZ_SIZE_INFO_ *srz3, const MSize &imgiSize, const MSize &faceiSize)
{
    if( srz3 )
    {
        FACENR_IN_PARAMS in;
        FACENR_OUT_PARAMS out;

        in.p2_in = imgiSize;
        in.face_map = faceiSize;
        calculateFACENRConfig(in, out);

        *srz3 = out.srz3Param;

        ModuleInfo info;
        info.moduleTag = EDipModule_SRZ3;
        info.frameGroup = 0;
        info.moduleStruct = reinterpret_cast<MVOID*>(srz3);
        frame.mvModuleData.push_back(info);
    }
    return (srz3 != NULL);
}


DpPqParam* makeDpPqParam(DpPqParam *param, const P2Pack &p2Pack, const Output &out)
{
    if( !param )
    {
        MY_S_LOGE(p2Pack.mLog, "Invalid DpPqParam buffer = nullptr, port:%s(%d)",
                  toName(out.mPortID), out.mPortID.index);
        return nullptr;
    }

    return makeDpPqParam(param, p2Pack, out.mPortID.capbility);
}

eP2_PQ_PATH toP2PQPath(MUINT32 cap /* EPortCapbility*/)
{
    if (cap == EPortCapbility_Disp)
    {
        return P2_PQ_PATH_DISPLAY;
    }
    else if (cap == EPortCapbility_Rcrd)
    {
        return P2_PQ_PATH_RECORD;
    }
    else if (cap == EPortCapbility_None)
    {
        return P2_PQ_PATH_VSS;
    }
    else
    {
        return P2_PQ_PATH_OTHER;
    }
}

DpPqParam* makeDpPqParam(DpPqParam *param, const P2Pack &p2Pack, const MUINT32 portCapabitity, const MDPObjPtr &mdpObj)
{
    if( !param )
    {
        MY_S_LOGE(p2Pack.mLog, "Invalid DpPqParam buffer = nullptr, portCapabitity:(%d)",
                  portCapabitity);
        return nullptr;
    }

    DpIspParam &ispParam = param->u.isp;

    param->scenario = MEDIA_ISP_PREVIEW;
    param->enable = false;
    const P2SensorData &sensorData = p2Pack.getSensorData(p2Pack.getFrameData().mMasterSensorID);

    ispParam.iso = sensorData.mISO;
    ispParam.LV = sensorData.mLV;
    ispParam.timestamp = sensorData.mMWUniqueKey;
    ispParam.frameNo = p2Pack.getFrameData().mMWFrameNo;
    ispParam.requestNo = p2Pack.getFrameData().mMWFrameRequestNo;
    ispParam.lensId = sensorData.mSensorID;
    ispParam.enableDump = mdpObj.needDump;
    if( mdpObj.pqDebug != PQ_DEBUG_NONE )
    {
        const char* dbgName = toName(mdpObj.pqDebug);
        const size_t len = std::min<size_t>(strlen(dbgName), sizeof(ispParam.userString)-1);
        memcpy(ispParam.userString, dbgName, len);
        ispParam.userString[len] = '\0';
    }
    else
    {
        ispParam.userString[0] = '\0';
    }

    const P2PlatInfo *plat = p2Pack.getSensorInfo(sensorData.mSensorID).mPlatInfo;

    if( p2Pack.getConfigInfo().mSupportPQ &&
        plat->supportDefaultPQ() &&
        plat->supportDefaultPQPath(toP2PQPath(portCapabitity))
      )
    {
        param->enable = true;
    }

    if( ! mdpObj.isDummyPQ &&
        p2Pack.getConfigInfo().mSupportPQ &&
        p2Pack.getConfigInfo().mSupportClearZoom &&
        plat->supportClearZoom() &&
        plat->supportCZPath(p2Pack.getConfigInfo().mUsageHint.mP2PQIndex, toP2PQPath(portCapabitity))
      )
    {
        param->enable |= (PQ_COLOR_EN|PQ_ULTRARES_EN);
        ClearZoomParam &cz = ispParam.clearZoomParam;
        cz.captureShot = CAPTURE_SINGLE;

        P2PlatInfo::NVRamData nvramData = plat->queryNVRamData(P2PlatInfo::FID_CLEARZOOM, sensorData.mMagic3A, sensorData.mIspProfile);
        cz.p_customSetting = nvramData.mLowIsoData;
    }

    if( ! mdpObj.isDummyPQ &&
        p2Pack.getConfigInfo().mSupportPQ &&
        p2Pack.getConfigInfo().mSupportHFG &&
        plat->supportHFG() &&
        plat->supportHFGPath(p2Pack.getConfigInfo().mUsageHint.mP2PQIndex, toP2PQPath(portCapabitity))
      )
    {
        param->enable |= (PQ_HFG_EN);
        DpHFGParam &hfg = ispParam.dpHFGParam;

        P2PlatInfo::NVRamData nvramData = plat->queryNVRamData(P2PlatInfo::FID_HFG, sensorData.mMagic3A, sensorData.mIspProfile);
        hfg.p_lowerSetting = nvramData.mLowIsoData;
        // TODO wait choo interface ready
        //hfg.p_upperSetting = nvramData.mHighIsoData;
        //hfg.lowerISO = nvramData.mLowerIso;
        //hfg.upperISO = nvramData.mHighIso;
        // TEST USE
        hfg.p_upperSetting = nvramData.mLowIsoData;
        hfg.lowerISO = ispParam.iso;
        hfg.upperISO = ispParam.iso;
    }

    if( ! mdpObj.isDummyPQ &&
        p2Pack.getConfigInfo().mSupportPQ &&
        p2Pack.getConfigInfo().mSupportDRE &&
        plat->supportDRE() &&
        plat->supportDREPath(p2Pack.getConfigInfo().mUsageHint.mP2PQIndex, toP2PQPath(portCapabitity))
      )
    {
        param->enable |= (PQ_DRE_EN);
        DpDREParam &dre = ispParam.dpDREParam;
        dre.cmd = DpDREParam::Cmd::Initialize | DpDREParam::Cmd::Default;
        dre.userId = ((unsigned long long)MEDIA_ISP_PREVIEW)<<32;
        if(portCapabitity == EPortCapbility_Rcrd && mdpObj.isExtraRecSrc)
        {
            dre.userId += 1; // using different SRAM
        }
        dre.buffer = nullptr;
        P2PlatInfo::NVRamData nvramData = plat->queryNVRamData(P2PlatInfo::FID_DRE, sensorData.mMagic3A, sensorData.mIspProfile);
        dre.p_customSetting = nvramData.mLowIsoData;
        dre.customIndex = nvramData.mLowIsoIndex;

        // actvie region setting
        dre.activeWidth  = plat->getActiveArrayRect().s.w;
        dre.activeHeight = plat->getActiveArrayRect().s.h;
    }

    return param;
}

MVOID updateExpectEndTime(DIPParams &dipParams, MUINT32 ms, MUINT32 expectFps)
{
    if(expectFps == 0)
    {
        struct timeval diff, curr, next;
        diff.tv_sec = 0;
        diff.tv_usec = ms * 1000;
        gettimeofday(&curr, NULL);
        timeradd(&curr, &diff, &next);
        for( unsigned i = 0, n = dipParams.mvDIPFrameParams.size(); i < n; ++i )
        {
            dipParams.mvDIPFrameParams.at(i).ExpectedEndTime = next;
        }
    }
    else
    {
        struct timeval next;
        next.tv_sec = 0;
        next.tv_usec = 0;
        for( unsigned i = 0, n = dipParams.mvDIPFrameParams.size(); i < n; ++i )
        {
            dipParams.mvDIPFrameParams.at(i).ExpectedEndTime = next;
        }
    }
}

MVOID push_PQParam(DIPFrameParams &frame, const P2Pack &p2Pack, const P2ObjPtr &obj)
{
    if( !obj.pqParam )
    {
        MY_S_LOGE(p2Pack.mLog, "Invalid pqParam buffer = NULL");
        return;
    }

    obj.pqParam->WDMAPQParam = NULL;
    obj.pqParam->WROTPQParam = NULL;
    for( const Output &out : frame.mvOut )
    {
        if( out.mPortID.index == EPortIndex_WDMAO &&
            obj.pqParam->WDMAPQParam == NULL )
        {
            obj.pqParam->WDMAPQParam = makeDpPqParam(obj.pqWDMA, p2Pack, out);
        }
        else if( out.mPortID.index == EPortIndex_WROTO &&
                 obj.pqParam->WROTPQParam == NULL )
        {
            obj.pqParam->WROTPQParam = makeDpPqParam(obj.pqWROT, p2Pack, out);
        }
    }

    ExtraParam extra;
    extra.CmdIdx = NSIoPipe::EPIPE_MDP_PQPARAM_CMD;
    extra.moduleStruct = (void*)obj.pqParam;
    frame.mvExtraParam.push_back(extra);
}

MVOID updateDIPParams(DIPParams &dipParams, const P2Pack &p2Pack, const P2IOPack &io, const P2ObjPtr &obj, const TuningParam &tuning)
{
    updateDIPFrameParams(dipParams.mvDIPFrameParams.at(0), p2Pack, io, obj, tuning);
}

DIPParams makeDIPParams(const P2Pack &p2Pack, ENormalStreamTag tag, const P2IOPack &io, const P2ObjPtr &obj, const TuningParam &tuning)
{
    DIPParams dipParams;
    dipParams.mvDIPFrameParams.push_back(makeDIPFrameParams(p2Pack, tag, io, obj, tuning));
    return dipParams;
}

DIPParams makeDIPParams(const P2Pack &p2Pack, ENormalStreamTag tag, const P2IOPack &io, const P2ObjPtr &obj)
{
    DIPParams dipParams;
    dipParams.mvDIPFrameParams.push_back(makeDIPFrameParams(p2Pack, tag, io, obj));
    return dipParams;
}


MVOID updateDIPFrameParams(DIPFrameParams &frame, const P2Pack &p2Pack, const P2IOPack &io, const P2ObjPtr &obj, const TuningParam &tuning)
{
    TRACE_S_FUNC_ENTER(p2Pack.mLog);

    const P2PlatInfo* platInfo = p2Pack.getPlatInfo();

    MSize inSize = io.mIMGI.getImgSize();
    for( auto &in : frame.mvIn )
    {
        if( is(in, PORT_IMGI) && in.mBuffer )
        {
            inSize = in.mBuffer->getImgSize();
            break;
        }
    }

    if( tuning.pRegBuf )
    {
        frame.mTuningData = tuning.pRegBuf;
    }
    if( tuning.pLsc2Buf )
    {
        push_in(frame, platInfo->getLsc2Port(), (IImageBuffer*)tuning.pLsc2Buf);
    }
    if( tuning.pBpc2Buf )
    {
        push_in(frame, platInfo->getBpc2Port(), (IImageBuffer*)tuning.pBpc2Buf);
    }

    if( tuning.pLcsBuf )
    {
        IImageBuffer *lcso = (IImageBuffer*)tuning.pLcsBuf;
        push_in(frame, PORT_LCEI, lcso);
        if( platInfo->hasYnrLceiPort() )
        {
            push_in(frame, platInfo->getYnrLceiPort(), lcso);
            push_srz4_LCENR(frame, obj.srz4, p2Pack, io.isResized(), inSize, lcso->getImgSize());
        }
    }

    if( tuning.pFaceAlphaBuf && platInfo->hasYnrFacePort())
    {
        IImageBuffer *facei = (IImageBuffer*)tuning.pFaceAlphaBuf;
        push_in(frame, platInfo->getYnrFaceiPort(), (IImageBuffer*)tuning.pFaceAlphaBuf);
        push_srz3_FACENR(frame, obj.srz3, inSize, facei->getImgSize());
    }

    if( tuning.bDCES_Enalbe )
    {
        if(io.mDCESO.mBuffer)
        {
            initBuffer(io.mDCESO.mBuffer);
            io.mDCESO.mBuffer->syncCache(eCACHECTRL_FLUSH);
            push_out(frame, PORT_DCESO, io.mDCESO.mBuffer);
        }
        else
        {
            MY_LOGW("DCES enabled but dceso is NULL !! Need Check!");
        }
    }

    TRACE_S_FUNC_EXIT(p2Pack.mLog);
}

DIPFrameParams makeDIPFrameParams(const P2Pack &p2Pack, ENormalStreamTag tag, const P2IOPack &io, const P2ObjPtr &obj, const TuningParam &tuning, MBOOL dumpReg)
{
    TRACE_S_FUNC_ENTER(p2Pack.mLog);
    DIPFrameParams fparam = makeDIPFrameParams(p2Pack, tag, io, obj, dumpReg);
    updateDIPFrameParams(fparam, p2Pack, io, obj, tuning);
    TRACE_S_FUNC_EXIT(p2Pack.mLog);
    return fparam;
}

DIPFrameParams makeDIPFrameParams(const P2Pack &p2Pack, ENormalStreamTag tag, const P2IOPack &io, const P2ObjPtr &obj, MBOOL dumpReg)
{
    const ILog &log = p2Pack.mLog;
    TRACE_S_FUNC_ENTER(log);

    const sp<Cropper> cropper = p2Pack.getSensorData().mCropper;
    MUINT32 cropFlag = 0;
    cropFlag |= io.isResized() ? Cropper::USE_RESIZED : 0;
    cropFlag |= io.useLMV() ? Cropper::USE_EIS_12 : 0;

    DIPFrameParams frame;
    frame.FrameNo = p2Pack.getFrameData().mMWFrameNo;
    frame.RequestNo = p2Pack.getFrameData().mMWFrameRequestNo;
    frame.Timestamp = p2Pack.getSensorData().mP1TS;
    frame.UniqueKey = p2Pack.getSensorData().mMWUniqueKey;
    frame.mStreamTag = tag;
    frame.mSensorIdx = p2Pack.getSensorData().mSensorID;
    frame.SensorDev = p2Pack.getSensorData().mNDDHint.SensorDev;
    frame.IspProfile = (obj.profile >= 0) ? obj.profile : (MINT32) p2Pack.getSensorData().mIspProfile;
    frame.mRunIdx = obj.run;
    frame.NeedDump = dumpReg;

    if( io.mIMGI.isValid() )
    {
        push_in(frame, PORT_IMGI, io.mIMGI);
    }

    if( io.mIMG2O.isValid() )
    {
        push_out(frame, PORT_IMG2O, io.mIMG2O);
        push_crop(frame, CROP_IMG2O, io.mIMG2O.mCropRect, io.mIMG2O.getImgSize(), io.mIMG2O.mDMAConstrain);
    }
    if( io.mIMG3O.isValid() )
    {
        push_out(frame, PORT_IMG3O, io.mIMG3O);
    }
    if( io.mWDMAO.isValid() )
    {
        push_out(frame, PORT_WDMAO, io.mWDMAO);
        push_crop(frame, CROP_WDMAO, io.mWDMAO.mCropRect, io.mWDMAO.getImgSize(), io.mWDMAO.mDMAConstrain);
    }
    if( io.mWROTO.isValid() )
    {
        push_out(frame, PORT_WROTO, io.mWROTO);
        push_crop(frame, CROP_WROTO, io.mWROTO.mCropRect, io.mWROTO.getImgSize(), io.mWROTO.mDMAConstrain);
    }

    if(obj.hasPQ)
    {
        push_PQParam(frame, p2Pack, obj);
    }

    TRACE_S_FUNC_EXIT(log);
    return frame;
}

MBOOL prepareMDPFrameParam(DIPParams &dipParam, MUINT32 refFrameInd, std::vector<P2IO> &mdpOuts, const MDPObjPtr &mdpObj)
{
    if(mdpOuts.size() == 0)
        return MFALSE;
    refinePQParam(mdpOuts, mdpObj);
    P2IOClassfier collect(mdpOuts);
    DIPFrameParams& frame = dipParam.mvDIPFrameParams.at(refFrameInd);
    // prepare reference frame
    prepareFrame(frame, collect);
    // put remain output to new frames
    while(collect.hasIO())
    {
        addNewFrame(dipParam, refFrameInd);
        DIPFrameParams &f = dipParam.mvDIPFrameParams.at(dipParam.mvDIPFrameParams.size() - 1);
        prepareFrame(f, collect);
    }
    return MTRUE;
}

MBOOL prepareMDPFrameParam(DIPParams &dipParam, DIPFrameParams &frame, std::vector<P2IO> &mdpOuts, const MDPObjPtr &mdpObj)
{
    if(mdpOuts.size() == 0)
        return MFALSE;
    refinePQParam(mdpOuts, mdpObj);
    P2IOClassfier collect(mdpOuts);
    // prepare first frame
    dipParam.mvDIPFrameParams.push_back(frame);
    DIPFrameParams &firstFrame = dipParam.mvDIPFrameParams.at(dipParam.mvDIPFrameParams.size() - 1);
    prepareFrame(firstFrame, collect);
    // put remain output to new frames
    while(collect.hasIO())
    {
        dipParam.mvDIPFrameParams.push_back(frame);
        DIPFrameParams &f = dipParam.mvDIPFrameParams.at(dipParam.mvDIPFrameParams.size() - 1);
        prepareFrame(f, collect);
    }
    return MTRUE;
}

MBOOL prepareOneMDPFrameParam(DIPFrameParams &frame, std::vector<P2IO> &mdpOuts, std::vector<P2IO> &remainList, const MDPObjPtr &mdpObj)
{
    if(mdpOuts.size() == 0)
        return MFALSE;

    refinePQParam(mdpOuts, mdpObj);
    P2IOClassfier collect(mdpOuts);
    // prepare reference frame
    prepareFrame(frame, collect);
    // put remain output to remainList
    if( collect.hasIO())
    {
        collect.popAll(remainList);
    }
    return MTRUE;
}

} // namespace P2Util
} // namespace Feature
} // namespace NSCam
