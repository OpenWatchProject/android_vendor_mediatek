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

#ifndef _MTKCAM_FEATURE_UTILS_P2_UTIL_H_
#define _MTKCAM_FEATURE_UTILS_P2_UTIL_H_

#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam/drv/def/ISyncDump.h>
#include <DpDataType.h>

#include <mtkcam3/feature/utils/log/ILogger.h>
#include <mtkcam3/feature/utils/p2/P2Pack.h>
#include <mtkcam3/feature/utils/p2/P2IO.h>
#include <mtkcam3/feature/utils/p2/DIPStream.h>

using NSImageio::NSIspio::EPortIndex;

using NSCam::NSIoPipe::PortID;
using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::FrameParams;
using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::EPortCapbility;
using NSCam::NSIoPipe::MCropRect;
using NSCam::NSIoPipe::MCrpRsInfo;
using NSCam::NSIoPipe::ExtraParam;
using NSCam::NSIoPipe::NSPostProc::ENormalStreamTag;
using NSCam::NSIoPipe::PQParam;

using NS3Av3::TuningParam;

//#define CROP_IMGO  1
//#define CROP_IMG2O 1
//#define CROP_IMG3O 1
//#define CROP_WDMAO 2
//#define CROP_WROTO 3


namespace NSCam {
namespace Feature {
namespace P2Util {

constexpr const MINT32 INVALID_P2_PROFILE = -1;
constexpr const char* DUMP_CAM_PIPE_PROPERTIES = "vendor.debug.camera.dump.campipe";

enum
{
    CROP_IMGO  = 1,
    CROP_IMG2O = 1,
    CROP_IMG3O = 1,
    CROP_WDMAO = 2,
    CROP_WROTO = 3,
};

class P2ObjPtr
{
public:
    _SRZ_SIZE_INFO_ *srz3 = NULL;
    _SRZ_SIZE_INFO_ *srz4 = NULL;
    NSCam::NSIoPipe::PQParam *pqParam = NULL;
    DpPqParam *pqWDMA = NULL;
    DpPqParam *pqWROT = NULL;
    MBOOL hasPQ = MTRUE;
    MINT32 profile = -1;
    NSCam::NSIoPipe::P2_RUN_INDEX run = NSIoPipe::P2_RUN_UNKNOWN;
};

class P2ISPObj
{
public:
    mutable _SRZ_SIZE_INFO_ srz3;
    mutable _SRZ_SIZE_INFO_ srz4;
    MINT32 profile = -1;
    NSCam::NSIoPipe::P2_RUN_INDEX run = NSIoPipe::P2_RUN_UNKNOWN;

    P2ObjPtr toPtrTable() const
    {
        P2ObjPtr ptr;
        ptr.srz3 = &srz3;
        ptr.srz4 = &srz4;
        ptr.hasPQ = MFALSE;
        ptr.profile = profile;
        ptr.run = run;
        return ptr;
    }
};

class P2Obj
{
public:
    mutable P2ISPObj ispObj;
    mutable NSCam::NSIoPipe::PQParam pqParam;
    mutable DpPqParam pqWDMA;
    mutable DpPqParam pqWROT;

    P2ObjPtr toPtrTable() const
    {
        P2ObjPtr ptr = ispObj.toPtrTable();
        ptr.pqParam = &pqParam;
        ptr.pqWDMA = &pqWDMA;
        ptr.pqWROT = &pqWROT;
        ptr.hasPQ = MTRUE;
        return ptr;
    }
};

enum PqDebugIndex
{
    PQ_DEBUG_NONE,
    PQ_DEBUG_S_P2A,
    PQ_DEBUG_VMDP,
};

class MDPObjPtr
{
public:
    void* ispTuningBuf = NULL;
    bool  isExtraRecSrc = false;
    bool  needDump = false;
    bool  isDummyPQ = false;
    PqDebugIndex pqDebug = PQ_DEBUG_NONE;
};


// Camera common function
MBOOL is4K2K(const MSize &size);
MCropRect getCropRect(const MRectF &rectF);

template <typename T>
MBOOL tryGet(const IMetadata &meta, MUINT32 tag, T &val)
{
    MBOOL ret = MFALSE;
    IMetadata::IEntry entry = meta.entryFor(tag);
    if( !entry.isEmpty() )
    {
        val = entry.itemAt(0, Type2Type<T>());
        ret = MTRUE;
    }
    return ret;
};
template <typename T>
MBOOL tryGet(const IMetadata *meta, MUINT32 tag, T &val)
{
    return (meta != NULL) ? tryGet<T>(*meta, tag, val) : MFALSE;
}
template <typename T>
MBOOL trySet(IMetadata &meta, MUINT32 tag, const T &val)
{
    MBOOL ret = MFALSE;
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    ret = (meta.update(tag, entry) == android::OK);
    return ret;
}
template <typename T>
MBOOL trySet(IMetadata *meta, MUINT32 tag, const T &val)
{
    return (meta != NULL) ? trySet<T>(*meta, tag, val) : MFALSE;
}
template <typename T>
T getMeta(const IMetadata &meta, MUINT32 tag, const T &val)
{
    T temp;
    return tryGet(meta, tag, temp) ? temp : val;
}
template <typename T>
T getMeta(const IMetadata *meta, MUINT32 tag, const T &val)
{
    T temp;
    return tryGet(meta, tag, temp) ? temp : val;
}

// P2IO function
Output toOutput(const P2IO &io);
Output toOutput(const P2IO &io, MUINT32 index);
MCropRect toMCropRect(const P2IO &io);
MCrpRsInfo toMCrpRsInfo(const P2IO &io);
MVOID applyDMAConstrain(MCropRect &crop, MUINT32 constrain);

// Tuning function
void* allocateRegBuffer(MBOOL zeroInit = MFALSE);
MVOID releaseRegBuffer(void* &buffer);
TuningParam makeTuningParam(const ILog &log, const P2Pack &p2Pack, NS3Av3::IHalISP *halISP, NS3Av3::MetaSet_T &inMetaSet,
                NS3Av3::MetaSet_T *pOutMetaSet, MBOOL resized, void *regBuffer, IImageBuffer *lcso, MINT32 dcesoMagic, IImageBuffer *dceso, MBOOL isSlave, void *syncTuningBuf, IImageBuffer *lcsho);

// Metadata function
MVOID updateExtraMeta(const P2Pack &p2Pack, IMetadata &outHal);
MVOID updateDebugExif(const P2Pack &p2Pack, const IMetadata &inHal, IMetadata &outHal);
MBOOL updateCropRegion(IMetadata &outHal, const MRect &rect);

// DIPParams util function
const char* toName(MUINT32 index);
const char* toName(EPortIndex index);
const char* toName(const PortID &port);
const char* toName(const Input &input);
const char* toName(const Output &output);
MBOOL is(const PortID &port, EPortIndex index);
MBOOL is(const Input &input, EPortIndex index);
MBOOL is(const Output &output, EPortIndex index);
MBOOL is(const PortID &port, const PortID &rhs);
MBOOL is(const Input &input, const PortID &rhs);
MBOOL is(const Output &output, const PortID &rhs);
MBOOL findInput(const DIPFrameParams &frame, const PortID &portID, Input &input);
MBOOL findOutput(const DIPFrameParams &frame, const PortID &portID, Output &output);
IImageBuffer* findInputBuffer(const DIPFrameParams &frame, const PortID &portID);
MVOID printDIPParams(const ILog &log, const DIPParams &params);
MVOID printTuningParam(const ILog &log, const TuningParam &tuning);

MVOID push_in(DIPFrameParams &frame, const PortID &portID, IImageBuffer *buffer);
MVOID push_in(DIPFrameParams &frame, const PortID &portID, const P2IO &in);
MVOID push_out(DIPFrameParams &frame, const PortID &portID, IImageBuffer *buffer);
MVOID push_out(DIPFrameParams &frame, const PortID &portID, IImageBuffer *buffer, EPortCapbility cap, MINT32 transform);
MVOID push_out(DIPFrameParams &frame, const PortID &portID, const P2IO &out);
MVOID push_crop(DIPFrameParams &frame, MUINT32 cropID, const MCropRect &crop, const MSize &size);
MVOID push_crop(DIPFrameParams &frame, MUINT32 cropID, const MRectF &crop, const MSize &size, const MUINT32 dmaConstrain = DMAConstrain::DEFAULT);

MVOID pushToMDP(DIPFrameParams &f, const PortID &portID, const P2IO &output);

// DIPParams function
MVOID updateDIPParams(DIPParams &dipParams, const P2Pack &p2Pack, const P2IOPack &io, const P2ObjPtr &obj, const TuningParam &tuning);
DIPParams makeDIPParams(const P2Pack &p2Pack, ENormalStreamTag tag, const P2IOPack &io, const P2ObjPtr &obj);
DIPParams makeDIPParams(const P2Pack &p2Pack, ENormalStreamTag tag, const P2IOPack &io, const P2ObjPtr &obj, const TuningParam &tuning);
MVOID updateDIPFrameParams(DIPFrameParams &frame, const P2Pack &p2Pack, const P2IOPack &io, const P2ObjPtr &obj, const TuningParam &tuning);
DIPFrameParams makeDIPFrameParams(const P2Pack &p2Pack, ENormalStreamTag tag, const P2IOPack &io, const P2ObjPtr &obj, MBOOL dumpReg = MFALSE);
DIPFrameParams makeDIPFrameParams(const P2Pack &p2Pack, ENormalStreamTag tag, const P2IOPack &io, const P2ObjPtr &obj, const TuningParam &tuning, MBOOL dumpReg = MFALSE);
MBOOL prepareMDPFrameParam(DIPParams &qparam, MUINT32 refFrameInd, std::vector<P2IO> &mdpOuts, const MDPObjPtr &mdpObj = MDPObjPtr());
MBOOL prepareMDPFrameParam(DIPParams &qparam, DIPFrameParams &frame, std::vector<P2IO> &mdpOuts, const MDPObjPtr &mdpObj = MDPObjPtr());
MBOOL prepareOneMDPFrameParam(DIPFrameParams &frame, std::vector<P2IO> &mdpOuts, std::vector<P2IO> &remainList, const MDPObjPtr &mdpObj = MDPObjPtr());

// DParams uitil function
eP2_PQ_PATH toP2PQPath(const MUINT32 cap);
DpPqParam* makeDpPqParam(DpPqParam *param, const P2Pack &p2Pack, const Output &out);
DpPqParam* makeDpPqParam(DpPqParam *param, const P2Pack &p2Pack, const MUINT32 portCapabitity, const MDPObjPtr &mdpObj = MDPObjPtr());

// If expectFps = 0, using ms to set ExpectedEndTime.
// Else using default ExpectedEndTime.
MVOID updateExpectEndTime(DIPParams &dipParams, MUINT32 ms, MUINT32 expectFps = 0);

MVOID push_PQParam(DIPFrameParams &frame, const P2Pack &p2Pack, const P2ObjPtr &obj);

} // namespace P2Util
} // namespace Feature
} // namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_P2_UTIL_H_
