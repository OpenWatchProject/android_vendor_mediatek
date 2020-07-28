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

#include <mtkcam3/feature/utils/p2/P2PlatInfo.h>

#include <DpDataType.h>
#include <DpIspStream.h>
#include <mtkcam/aaa/INvBufUtil.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <camera_custom_nvram.h>
#include <camera_nvram_query.h>
#include <camera_custom_pq.h>
#include <camera_custom_dsdn.h>
#include <string.h>

#include <cutils/properties.h>

#include <mtkcam/drv/iopipe/Port.h>

using NSCam::NSIoPipe::EPortCapbility;
using NSCam::NSIoPipe::EPortCapbility_None;
using NSCam::NSIoPipe::EPortCapbility_Disp;
using NSCam::NSIoPipe::EPortCapbility_Rcrd;
using NSImageio::NSIspio::EPortIndex_VIPI;
using NSImageio::NSIspio::EPortIndex_IMG3O;

#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif // MTK_CAM_NEW_NVRAM_SUPPORT
#include <isp_tuning/isp_tuning.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "MtkCam/P2PlatInfo"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

#ifdef MY_LOGD
#undef MY_LOGD
#endif
#ifdef MY_LOGW
#undef MY_LOGW
#endif
#ifdef MY_LOGE
#undef MY_LOGE
#endif

#define MY_LOGD(s,...) CAM_ULOGMD("[%s]" s, __FUNCTION__, ##__VA_ARGS__)
#define MY_LOGW(s,...) CAM_ULOGMW("[%s]" s, __FUNCTION__, ##__VA_ARGS__)
#define MY_LOGE(s,...) CAM_ULOGME("[%s]" s, __FUNCTION__, ##__VA_ARGS__)

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

#define MAX_DSDN_RATIO_VALUE (256)

using NSCam::NSIoPipe::EDIPHWVersion_40;
using NSCam::NSIoPipe::EDIPHWVersion_50;
using NSCam::NSIoPipe::EDIPHWVersion_60;
using NSCam::NSIoPipe::EDIPINFO_DIPVERSION;
using NSCam::NSIoPipe::NSPostProc::INormalStream;
using NSCam::NSIoPipe::PortID;

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
using NSCam::NSIoPipe::PORT_UNKNOWN;

using NSImageio::NSIspio::EPortIndex_UNKNOW;
using NSCam::NSIoPipe::EPortType_Memory;
using NSCam::NSIoPipe::EPortCapbility_None;

namespace NSCam {
namespace Feature {
namespace P2Util {

template<typename T>
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
}

template<typename T>
MBOOL tryGet(const IMetadata *meta, MUINT32 tag, T &val)
{
    return (meta != NULL) ? tryGet<T>(*meta, tag, val) : MFALSE;
}

template<typename T>
MBOOL trySet(IMetadata &meta, MUINT32 tag, const T &val)
{
    MBOOL ret = MFALSE;
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    ret = (meta.update(tag, entry) == android::OK);
    return ret;
}

template<typename T>
MBOOL trySet(IMetadata *meta, MUINT32 tag, const T &val)
{
    return (meta != NULL) ? trySet<T>(*meta, tag, val) : MFALSE;
}

class P2PlatInfoImp : public P2PlatInfo
{
public:
    P2PlatInfoImp(MUINT32 sensorID);
    ~P2PlatInfoImp();
    virtual MBOOL isDip50() const;
    virtual MBOOL supportDefaultPQ() const;
    virtual MBOOL supportClearZoom() const;
    virtual MBOOL supportDRE() const;
    virtual MBOOL supportHFG() const;
    virtual MBOOL supportSecure3DNR() const;
    virtual MRect getActiveArrayRect() const;
    virtual P2PlatInfo::NVRamDSDN queryDSDN(MINT32 magic3A, MUINT8 ispProfile) const;
    virtual P2PlatInfo::NVRamData queryNVRamData(FeatureID fID, MINT32 magic3A, MUINT8 ispProfile) const;
    virtual MUINT32 getImgoAlignMask() const;

    virtual PortID getLsc2Port() const;
    virtual PortID getBpc2Port() const;
    virtual PortID getYnrFaceiPort() const;
    virtual PortID getYnrLceiPort() const;
    virtual MBOOL hasYnrFacePort() const;
    virtual MBOOL hasYnrLceiPort() const;

    virtual MBOOL supportDefaultPQPath(eP2_PQ_PATH pqPath) const;
    virtual MBOOL supportCZPath(MUINT32 pqIdx, eP2_PQ_PATH pqPath) const;
    virtual MBOOL supportDREPath(MUINT32 pqIdx, eP2_PQ_PATH pqPath) const;
    virtual MBOOL supportHFGPath(MUINT32 pqIdx, eP2_PQ_PATH pqPath) const;
private:
    MVOID initSensorDev();
    MVOID initActiveArrayRect();
    MVOID initNVRam();
    MVOID initIdxMgr();
    MVOID initTuningPort();
    MVOID initP2PQTable();
    const char* toP2PQIdxName(MUINT32 idx);
    MVOID dumpP2PQTable(const char *pKeyword);

    MVOID queryLowIsoNVRam(MINT32 magic3A, MUINT8 ispProfile, NvramFeatureID nfid, P2PlatInfo::NVRamData &out) const;
    MVOID queryAllNVRam(MINT32 magic3A, MUINT8 ispProfile, NvramFeatureID nfid, P2PlatInfo::NVRamData &out) const;

private:

    MUINT32 mSensorID = -1;

    std::map<NSIoPipe::EDIPInfoEnum, MUINT32> mDipInfo;
    MUINT32 mDipVersion = NSIoPipe::EDIPHWVersion_40;

    std::map<DP_ISP_FEATURE_ENUM, bool> mIspFeature;
    MBOOL mSupportClearZoom = MFALSE;
    MBOOL mSupportDRE = MFALSE;
    MBOOL mSupportHFG = MFALSE;
    MBOOL mSupportSecure3DNR = MFALSE;

    NSCam::IHalSensorList *mHalSensorList = NULL;
    ESensorDev_T mSensorDev;

    MRect mActiveArrayRect;
    NVRAM_CAMERA_FEATURE_STRUCT *mpNVRam = NULL;
    NvramQueryHelper *mNVRamHelper = NULL;
    DSDNCustom *mDSDNCustom = NULL;
#if MTK_CAM_NEW_NVRAM_SUPPORT
    IdxMgr *mIdxMgr = NULL;
#endif// MTK_CAM_NEW_NVRAM_SUPPORT

    PortID mLsc2Port = PORT_UNKNOWN;
    PortID mBpc2Port = PORT_UNKNOWN;
    PortID mYnrFaceIPort = PORT_UNKNOWN;
    PortID mYnrLceiPort = PORT_UNKNOWN;

// member
private:
    P2PQCtrl mP2PQCtrlTable[P2_PQ_MAX_NUM];
    MINT32   mPQDebugEnableProp = 0;
};

P2PlatInfoImp::P2PlatInfoImp(MUINT32 sensorID)
{
    mSensorID = sensorID;
    mDipInfo[EDIPINFO_DIPVERSION] = EDIPHWVersion_40;
    if( !INormalStream::queryDIPInfo(mDipInfo) )
    {
        MY_LOGE("queryDIPInfo fail!");
    }
    mDipVersion = mDipInfo[EDIPINFO_DIPVERSION];
    DpIspStream::queryISPFeatureSupport(mIspFeature);
    mSupportClearZoom = mIspFeature[ISP_FEATURE_CLEARZOOM];
    mSupportDRE = mIspFeature[ISP_FEATURE_DRE];
    mSupportHFG = mIspFeature[ISP_FEATURE_HFG];
    mSupportSecure3DNR = INormalStream::needSecureBuffer(EPortIndex_VIPI) &&
                         INormalStream::needSecureBuffer(EPortIndex_IMG3O);
    mNVRamHelper = NvramQueryHelper::getHelper();
    mDSDNCustom = DSDNCustom::getInstance();

    initSensorDev();
    initActiveArrayRect();
    initNVRam();
    initIdxMgr();
    initTuningPort();
    initP2PQTable();
}
P2PlatInfoImp::~P2PlatInfoImp()
{
}

MBOOL P2PlatInfoImp::isDip50() const
{
    return mDipVersion == EDIPHWVersion_50;
}

MBOOL P2PlatInfoImp::supportDefaultPQ() const
{
    return SUPPORT_DEFAULT_PQ;
}

MBOOL P2PlatInfoImp::supportClearZoom() const
{
    return mSupportClearZoom;
}

MBOOL P2PlatInfoImp::supportDRE() const
{
    return mSupportDRE;
}

MBOOL P2PlatInfoImp::supportHFG() const
{
    return mSupportHFG;
}

MBOOL P2PlatInfoImp::supportSecure3DNR() const
{
    return mSupportSecure3DNR;
}

MRect P2PlatInfoImp::getActiveArrayRect() const
{
    return mActiveArrayRect;
}

PortID P2PlatInfoImp::getLsc2Port() const
{
    return mLsc2Port;
}

PortID P2PlatInfoImp::getBpc2Port() const
{
    return mBpc2Port;
}

PortID P2PlatInfoImp::getYnrFaceiPort() const
{
    return mYnrFaceIPort;
}

PortID P2PlatInfoImp::getYnrLceiPort() const
{
    return mYnrLceiPort;
}

MBOOL P2PlatInfoImp::hasYnrFacePort() const
{
    return mYnrFaceIPort.index != EPortIndex_UNKNOW;
}
MBOOL P2PlatInfoImp::hasYnrLceiPort() const
{
    return mYnrLceiPort.index != EPortIndex_UNKNOW;
}

MBOOL P2PlatInfoImp::supportDefaultPQPath(eP2_PQ_PATH pqPath) const
{
    if (pqPath == P2_PQ_PATH_DISPLAY)
    {
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}

MBOOL P2PlatInfoImp::supportCZPath(MUINT32 pqIdx, eP2_PQ_PATH pqPath) const
{
    if (pqIdx < 0 || pqIdx >= P2_PQ_MAX_NUM)
    {
        MY_LOGE("MDPPQ: wrong pqIdx=%d or pqPath=%d", pqIdx, pqPath);
        return MFALSE;
    }

    if (pqPath == P2_PQ_PATH_DISPLAY)
    {
        return P2_PQ_PATH_IS_DISPLAY_ENABLED(mP2PQCtrlTable[pqIdx].czEnableMask);
    }
    else if (pqPath == P2_PQ_PATH_RECORD)
    {
        return P2_PQ_PATH_IS_RECORD_ENABLED(mP2PQCtrlTable[pqIdx].czEnableMask);
    }
    else if (pqPath == P2_PQ_PATH_VSS)
    {
        return P2_PQ_PATH_IS_VSS_ENABLED(mP2PQCtrlTable[pqIdx].czEnableMask);
    }
    else
    {
        return MFALSE;
    }
}

MBOOL P2PlatInfoImp::supportDREPath(MUINT32 pqIdx, eP2_PQ_PATH pqPath) const
{
    if (pqIdx < 0 || pqIdx >= P2_PQ_MAX_NUM)
    {
        MY_LOGE("MDPPQ: wrong pqIdx=%d or pqPath=%d", pqIdx, pqPath);
        return MFALSE;
    }

    if (pqPath == P2_PQ_PATH_DISPLAY)
    {
        return P2_PQ_PATH_IS_DISPLAY_ENABLED(mP2PQCtrlTable[pqIdx].dreEnableMask);
    }
    else if (pqPath == P2_PQ_PATH_RECORD)
    {
        return P2_PQ_PATH_IS_RECORD_ENABLED(mP2PQCtrlTable[pqIdx].dreEnableMask);
    }
    else if (pqPath == P2_PQ_PATH_VSS)
    {
        return P2_PQ_PATH_IS_VSS_ENABLED(mP2PQCtrlTable[pqIdx].dreEnableMask);
    }
    else
    {
        return MFALSE;
    }
}

MBOOL P2PlatInfoImp::supportHFGPath(MUINT32 pqIdx, eP2_PQ_PATH pqPath) const
{
    if (pqIdx < 0 || pqIdx >= P2_PQ_MAX_NUM)
    {
        MY_LOGE("MDPPQ: wrong pqIdx=%d or pqPath=%d", pqIdx, pqPath);
        return MFALSE;
    }

    if (pqPath == P2_PQ_PATH_DISPLAY)
    {
        return P2_PQ_PATH_IS_DISPLAY_ENABLED(mP2PQCtrlTable[pqIdx].hfgEnableMask);
    }
    else if (pqPath == P2_PQ_PATH_RECORD)
    {
        return P2_PQ_PATH_IS_RECORD_ENABLED(mP2PQCtrlTable[pqIdx].hfgEnableMask);
    }
    else if (pqPath == P2_PQ_PATH_VSS)
    {
        return P2_PQ_PATH_IS_VSS_ENABLED(mP2PQCtrlTable[pqIdx].hfgEnableMask);
    }
    else
    {
        return MFALSE;
    }
}

P2PlatInfo::NVRamDSDN P2PlatInfoImp::queryDSDN( MINT32 magic3A, MUINT8 ispProfile) const
{
    P2PlatInfo::NVRamDSDN outData;
    if(mDSDNCustom->isSupport())
    {
        P2PlatInfo::NVRamData dsdnNVRam = queryNVRamData(FID_DSDN, magic3A, ispProfile);
        DSDNCustom::ParsedNVRam parseNVRam;
        if (mDSDNCustom->parseNVRam(dsdnNVRam.mLowIsoData, parseNVRam) )
        {
            outData.mIsoThreshold = parseNVRam.isoThreshold;
            outData.mRatioMultiple = (parseNVRam.ratioMultiple < 1) ? 1
                                     : (parseNVRam.ratioMultiple > MAX_DSDN_RATIO_VALUE) ? MAX_DSDN_RATIO_VALUE
                                     : parseNVRam.ratioMultiple;
            outData.mRatioDivider = (parseNVRam.ratioDivider < 1) ? 1
                                     : (parseNVRam.ratioDivider > MAX_DSDN_RATIO_VALUE) ? MAX_DSDN_RATIO_VALUE
                                     : parseNVRam.ratioDivider;
        }
        else
        {
            MY_LOGE("parse DSDN NVRam FAIL!! profile=%d magic=%d", ispProfile, magic3A);
        }
    }
    return outData;
}

P2PlatInfo::NVRamData P2PlatInfoImp::queryNVRamData(FeatureID fID, MINT32 magic3A, MUINT8 ispProfile) const
{
    P2PlatInfo::NVRamData outData;
    switch(fID)
    {
        case FID_CLEARZOOM:
            queryLowIsoNVRam(magic3A, ispProfile, NvramFeatureID::CLEARZOOM, outData);
            break;
        case FID_DRE:
            queryLowIsoNVRam(magic3A, ispProfile, NvramFeatureID::CALTM, outData);
            break;
        case FID_HFG:
            queryAllNVRam(magic3A, ispProfile, NvramFeatureID::HFG, outData);
            break;
        case FID_DSDN:
            queryLowIsoNVRam(magic3A, ispProfile, NvramFeatureID::DSDN, outData);
            break;
        default:
            MY_LOGE("unknown featureID=%d magic3A=%d", fID, magic3A);
            break;
    }
    return outData;
}

MUINT32 P2PlatInfoImp::getImgoAlignMask() const
{
    const MUINT32 ALIGN_2BYTE  = ~1;
    const MUINT32 ALIGN_32BYTE = ~0x1F;
    return ((mDipVersion >= EDIPHWVersion_60) ? ALIGN_32BYTE : ALIGN_2BYTE);
}

MVOID P2PlatInfoImp::queryLowIsoNVRam(MINT32 magic3A, MUINT8 ispProfile, NvramFeatureID nfid, P2PlatInfo::NVRamData &out) const
{
    (void)magic3A;
    (void)ispProfile;
    (void)nfid;
    (void)out;
#if MTK_CAM_NEW_NVRAM_SUPPORT
    uint8_t moduleEnum = 0;
    if( mIdxMgr && mpNVRam && mNVRamHelper->queryIspModuleID(nfid, moduleEnum))
    {
        CAM_IDX_QRY_COMB rMappingInfo;
        mIdxMgr->getMappingInfo(mSensorDev, rMappingInfo, magic3A);
        MUINT32 prof = (MUINT32)ispProfile;
        mIdxMgr->patchMappingInfo(mSensorDev, rMappingInfo, NSIspTuning::EDim_IspProfile, &prof);

        out.mLowIsoIndex = mIdxMgr->query(mSensorDev, (NSIspTuning::EModule_T)moduleEnum, rMappingInfo, __FUNCTION__);
        NvramQueryHelper::QueryCfg cfg(nfid, out.mLowIsoIndex, mpNVRam);
        if( ! mNVRamHelper->queryFeatureNvram(cfg, out.mLowIsoData) )
        {
            MY_LOGE("query nvram failed! module=%d magic3A=%d", moduleEnum, magic3A);
        }
    }
#endif // MTK_CAM_NEW_NVRAM_SUPPORT
}

MVOID P2PlatInfoImp::queryAllNVRam(MINT32 magic3A, MUINT8 ispProfile, NvramFeatureID nfid, P2PlatInfo::NVRamData &out) const
{
    (void)magic3A;
    (void)ispProfile;
    (void)nfid;
    (void)out;
#if MTK_CAM_NEW_NVRAM_SUPPORT
    MUINT8 moduleEnum = 0;
    if( mIdxMgr && mpNVRam && mNVRamHelper->queryIspModuleID(nfid, moduleEnum))
    {

        CAM_IDX_QRY_COMB rMappingInfo;
        mIdxMgr->getMappingInfo(mSensorDev, rMappingInfo, magic3A);
        MUINT32 prof = (MUINT32)ispProfile;
        mIdxMgr->patchMappingInfo(mSensorDev, rMappingInfo, NSIspTuning::EDim_IspProfile, &prof);
        // TODO get upper & lower iso , index, nvram
        // Now just use low iso nvram query
        //==============
        out.mLowIsoIndex = mIdxMgr->query(mSensorDev, (NSIspTuning::EModule_T)moduleEnum, rMappingInfo, __FUNCTION__);
        NvramQueryHelper::QueryCfg cfg(nfid, out.mLowIsoIndex, mpNVRam);
        if( ! mNVRamHelper->queryFeatureNvram(cfg, out.mLowIsoData) )
        {
            MY_LOGE("query nvram failed! module=%d magic3A=%d", moduleEnum, magic3A);
        }
        //==================

    }
#endif // MTK_CAM_NEW_NVRAM_SUPPORT
}

MVOID P2PlatInfoImp::initSensorDev()
{
    mHalSensorList = MAKE_HalSensorList();
    if(mHalSensorList != NULL)
    {
        mSensorDev = (ESensorDev_T)mHalSensorList->querySensorDevIdx(mSensorID);
    }
    else
    {
        MY_LOGE("MAKE_HalSensorList return null");
    }

}

MVOID P2PlatInfoImp::initActiveArrayRect()
{
    mActiveArrayRect = MRect(1600,1200);
    android::sp<IMetadataProvider> metaProvider = NSCam::NSMetadataProviderManager::valueFor(mSensorID);

    if( metaProvider == NULL )
    {
        MY_LOGE("get NSMetadataProvider failed, use (1600,1200)");
        return;
    }

    IMetadata meta = metaProvider->getMtkStaticCharacteristics();
    if( !tryGet<MRect>(meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArrayRect) )
    {
        MY_LOGE("MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION failed, use (1600,1200)");
        return;
    }

    MY_LOGD("Sensor(%d) Active array(%d,%d)(%dx%d)",
             mSensorID,
             mActiveArrayRect.p.x, mActiveArrayRect.p.y,
             mActiveArrayRect.s.w, mActiveArrayRect.s.h);
}

MVOID P2PlatInfoImp::initNVRam()
{
    auto pNVBufUtil = MAKE_NvBufUtil();
    if( !mHalSensorList || !pNVBufUtil )
    {
        MY_LOGE("MAKE_HalSensorList = %p, MAKE_NvBUfUtil = %p", mHalSensorList, pNVBufUtil);
    }
    else
    {
        if( 0 != pNVBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)mpNVRam) )
        {
            MY_LOGE("NvBufUtil->getBufAndRead() failed");
            mpNVRam = NULL;
        }
    }
}

MVOID P2PlatInfoImp::initIdxMgr()
{
#if MTK_CAM_NEW_NVRAM_SUPPORT
    mIdxMgr = IdxMgr::createInstance(mSensorDev);
#endif // MTK_CAM_NEW_NVRAM_SUPPORT
}

MVOID P2PlatInfoImp::initTuningPort()
{
    switch(mDipVersion)
    {
        case EDIPHWVersion_60:
        {
            mLsc2Port = PORT_LSCI;
            mBpc2Port = PORT_BPCI;
            mYnrFaceIPort = PORT_YNR_FACEI;
            mYnrLceiPort = PORT_YNR_LCEI;
            break;
        }
        case EDIPHWVersion_50:
        {
            mLsc2Port = PORT_IMGCI;
            mBpc2Port = PORT_IMGBI;
            mYnrFaceIPort = PORT_UNKNOWN;
            mYnrLceiPort = PORT_DEPI;
            break;
        }
        default:
        {
            mLsc2Port = PORT_DEPI;
            mBpc2Port = PORT_DMGI;
            mYnrFaceIPort = PORT_UNKNOWN;
            mYnrLceiPort = PORT_UNKNOWN;
            break;
        }
    }
}

const char* P2PlatInfoImp::toP2PQIdxName(MUINT32 idx)
{
    switch(idx)
    {
    case P2_PQ_DEFAULT:        return "def";
    case P2_PQ_NORMAL:         return "normal";
    case P2_PQ_EIS12_VIDEO_4k: return "eis12-v4k";
    case P2_PQ_EIS30_VIDEO:    return "eis30-V";
    case P2_PQ_EIS30_VIDEO_4k: return "eis30-v4k";
    case P2_PQ_EIS35_VIDEO:    return "eis35-V";
    case P2_PQ_EIS35_VIDEO_4k: return "eis35-v4k";
    case P2_PQ_EIS35_NO_VIDEO: return "eis35-noV";
    case P2_PQ_SMVRCONSTRAINT: return "smvrC";
    case P2_PQ_SMVRBATCH:      return "smvrB";
    default:                   return "unknown";
    }
}

MVOID P2PlatInfoImp::dumpP2PQTable(const char *pKeyword)
{
    if(pKeyword == nullptr)
    {
       MY_LOGE("MDPPQ: keyword should not be NULL");
       return;
    }

    const int32_t ONE_ITEM_SIZE = 80;
    const int32_t SIZE = ONE_ITEM_SIZE*P2_PQ_MAX_NUM;
    char tmp[SIZE];
    int32_t wc, ret = 0;

    wc = snprintf(tmp, SIZE, "MDPPQ: P2PQCtrlTable( %s  ): ", pKeyword);

    for (int i = 0; i < P2_PQ_MAX_NUM; ++i)
    {
        ret = snprintf(tmp+wc, SIZE-wc, "[%d]:%s[cz(d%d,r%d,v%d],dre(d%d,r%d,v%d],hfg(d%d,r%d,v%d)], ",
            i, toP2PQIdxName(i),
            P2_PQ_PATH_IS_DISPLAY_ENABLED(mP2PQCtrlTable[i].czEnableMask),
            P2_PQ_PATH_IS_RECORD_ENABLED(mP2PQCtrlTable[i].czEnableMask),
            P2_PQ_PATH_IS_VSS_ENABLED(mP2PQCtrlTable[i].czEnableMask),
            P2_PQ_PATH_IS_DISPLAY_ENABLED(mP2PQCtrlTable[i].dreEnableMask),
            P2_PQ_PATH_IS_RECORD_ENABLED(mP2PQCtrlTable[i].dreEnableMask),
            P2_PQ_PATH_IS_VSS_ENABLED(mP2PQCtrlTable[i].dreEnableMask),
            P2_PQ_PATH_IS_DISPLAY_ENABLED(mP2PQCtrlTable[i].hfgEnableMask),
            P2_PQ_PATH_IS_RECORD_ENABLED(mP2PQCtrlTable[i].hfgEnableMask),
            P2_PQ_PATH_IS_VSS_ENABLED(mP2PQCtrlTable[i].hfgEnableMask)
        );

        if (ret >= 0) wc += ret;

        if (ret < 0 || ((SIZE-wc) < ONE_ITEM_SIZE) )
        {
            MY_LOGW("MDPPQ: snprintf error(ret=%d) or "
                 "overflow might happen soon: bytes_writteen(%d) vs total_bytes(%d)",
                 wc, wc, SIZE);
            break;
        }
    }

    MY_LOGD("%s", tmp);
    return;
}

MVOID P2PlatInfoImp::initP2PQTable()
{
    CustomPQBase *pPQBase = CustomPQBase::getImpl();
    const CustomPQCtrl *pCustomPQTable = pPQBase->queryFeaturePQTablePtr();

    if ((int)P2_PQ_MAX_NUM != (int)CUSTOM_PQ_MAX_NUM)
    {
        MY_LOGE("MDPPQ: !!err: P2PQCtrl init fail: PQ_MAX_NUM(p2=%d, custom=%d)",
            P2_PQ_MAX_NUM, CUSTOM_PQ_MAX_NUM
        );
        return;
    }

    #define _INIT_P2PQTABLE_(p2Idx, cIdx, p2table, cTable) \
        p2table[p2Idx].czEnableMask = cTable[cIdx].czEnableMask; \
        p2table[p2Idx].dreEnableMask = cTable[cIdx].dreEnableMask; \
        p2table[p2Idx].hfgEnableMask = cTable[cIdx].hfgEnableMask;

    _INIT_P2PQTABLE_(P2_PQ_DEFAULT,        CUSTOM_PQ_DEFAULT,        mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_NORMAL,         CUSTOM_PQ_NORMAL,         mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_EIS12_VIDEO_4k, CUSTOM_PQ_EIS12_VIDEO_4k, mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_EIS30_VIDEO,    CUSTOM_PQ_EIS30_VIDEO,    mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_EIS30_VIDEO_4k, CUSTOM_PQ_EIS30_VIDEO_4k, mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_EIS35_VIDEO,    CUSTOM_PQ_EIS35_VIDEO,    mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_EIS35_VIDEO_4k, CUSTOM_PQ_EIS35_VIDEO_4k, mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_EIS35_NO_VIDEO, CUSTOM_PQ_EIS35_NO_VIDEO, mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_SMVRCONSTRAINT, CUSTOM_PQ_SMVRCONSTRAINT, mP2PQCtrlTable, pCustomPQTable);
    _INIT_P2PQTABLE_(P2_PQ_SMVRBATCH,      CUSTOM_PQ_SMVRBATCH,      mP2PQCtrlTable, pCustomPQTable);
    dumpP2PQTable(" custom  ");

    // property decide
    mPQDebugEnableProp = ::property_get_int32("vendor.debug.pq.debug.enable", 0);
    if (mPQDebugEnableProp)
    {
        int32_t pqIdxProp  = ::property_get_int32("vendor.debug.pq.idx", 0);
        MY_LOGD("MDPPQ: pqIdx_by_property=%d", pqIdxProp);

        if (pqIdxProp != 0)
        {
            #define _UPDATE_ADB_(PropName, PQPath, Val)\
                { \
                    int32_t oriVal = P2_PQ_PATH_IS_##PQPath##_ENABLED(Val); \
                    int32_t enable  = ::property_get_int32(PropName, oriVal); \
                    if (enable != oriVal) \
                    { \
                       if (enable) \
                       { \
                           P2_PQ_PATH_ENABLE_##PQPath(Val); \
                       } \
                       else \
                       { \
                           P2_PQ_PATH_DISABLE_##PQPath(Val); \
                       } \
                    } \
                }

            _UPDATE_ADB_("vendor.debug.pq.cz.disp",  DISPLAY, mP2PQCtrlTable[pqIdxProp].czEnableMask );
            _UPDATE_ADB_("vendor.debug.pq.cz.rec",   RECORD,  mP2PQCtrlTable[pqIdxProp].czEnableMask );
            _UPDATE_ADB_("vendor.debug.pq.cz.vss",   VSS,     mP2PQCtrlTable[pqIdxProp].czEnableMask );
            _UPDATE_ADB_("vendor.debug.pq.dre.disp", DISPLAY, mP2PQCtrlTable[pqIdxProp].dreEnableMask);
            _UPDATE_ADB_("vendor.debug.pq.dre.rec",  RECORD,  mP2PQCtrlTable[pqIdxProp].dreEnableMask);
            _UPDATE_ADB_("vendor.debug.pq.dre.vss",  VSS,     mP2PQCtrlTable[pqIdxProp].dreEnableMask);
            _UPDATE_ADB_("vendor.debug.pq.hfg.disp", DISPLAY, mP2PQCtrlTable[pqIdxProp].hfgEnableMask);
            _UPDATE_ADB_("vendor.debug.pq.hfg.rec",  RECORD,  mP2PQCtrlTable[pqIdxProp].hfgEnableMask);
            _UPDATE_ADB_("vendor.debug.pq.hfg.vss",  VSS,     mP2PQCtrlTable[pqIdxProp].hfgEnableMask);

            #undef _UPDATE_ADB_
        }
        dumpP2PQTable(" property");
    }
    #undef _INIT_P2PQTABLE_
    return;
}

template <unsigned ID>
const P2PlatInfo* getPlatInfo_()
{
    static P2PlatInfoImp sPlatInfo(ID);
    return &sPlatInfo;
}

const P2PlatInfo* P2PlatInfo::getInstance(MUINT32 sensorID)
{
    switch(sensorID)
    {
    case 0:   return getPlatInfo_<0>();
    case 1:   return getPlatInfo_<1>();
    case 2:   return getPlatInfo_<2>();
    case 3:   return getPlatInfo_<3>();
    case 4:   return getPlatInfo_<4>();
    case 5:   return getPlatInfo_<5>();
    case 6:   return getPlatInfo_<6>();
    case 7:   return getPlatInfo_<7>();
    case 8:   return getPlatInfo_<8>();
    case 9:   return getPlatInfo_<9>();
    default:  MY_LOGE("invalid sensorID=%d", sensorID);
              return NULL;
    };
}

} // namespace P2Util
} // namespace Feature
} // namespace NSCam
