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

#ifndef _MTK_CAMERA_FEATURE_PIPE_SFPIO_H_
#define _MTK_CAMERA_FEATURE_PIPE_SFPIO_H_

#include <unordered_map>
#include <mtkcam/drv/iopipe/Port.h>
#include <mtkcam/drv/iopipe/PortMap.h>
#include <mtkcam/drv/def/IPostProcDef.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

#include <mtkcam3/feature/featurePipe/util/VarMap.h>
#include <mtkcam3/feature/utils/p2/P2Data.h>
#include <mtkcam3/feature/utils/p2/P2IO.h>
#include <mtkcam3/feature/utils/log/LogString.h>

namespace NSCam {
class IMetadata;
namespace NSCamFeature {
namespace NSFeaturePipe {

enum PathType
{
    PATH_UNKNOWN = 0,
    PATH_GENERAL,
    PATH_PHYSICAL,
    PATH_LARGE,
};

IImageBuffer* get(const std::vector<IImageBuffer*> &imgs);
IImageBuffer* get(const std::vector<IImageBuffer*> &imgs, MUINT32 index);

class SFPOutput
{
public:
    enum OutTargetType
    {
        OUT_TARGET_UNKNOWN = 0,
        OUT_TARGET_DISPLAY,
        OUT_TARGET_RECORD,
        OUT_TARGET_FD,
        OUT_TARGET_PHYSICAL,
    };

public:
    SFPOutput();
    SFPOutput(const std::vector<IImageBuffer*> &buffers, MUINT32 transform, OutTargetType targetType);

    static const char* typeToChar(const OutTargetType &type);
    MVOID appendDumpInfo(LogString &str) const;
    MBOOL isCropValid() const;
    Feature::P2Util::P2IO toP2IO() const;
    std::vector<Feature::P2Util::P2IO> toP2IOVector() const;

private:
    Feature::P2Util::P2IO toP2IO(IImageBuffer *buffer) const;

public:
    std::vector<IImageBuffer*> mBuffers;
    MUINT32 mTransform = eTransform_None;
    NSIoPipe::EPortCapbility mCapability = NSIoPipe::EPortCapbility_None;
    OutTargetType mTargetType = OUT_TARGET_UNKNOWN;

    MRectF mCropRect;// Refer to Master's input
    MSize  mCropDstSize;// Refer to Master's input
    MUINT32 mDMAConstrain = 0;

    MVOID* mpPqParam = NULL;
    MVOID* mpDpPqParam = NULL;
};

class SFPSensorInput
{
public:
    std::vector<IImageBuffer*> mIMGO;
    std::vector<IImageBuffer*> mRRZO;
    std::vector<IImageBuffer*> mLCSO;
    std::vector<IImageBuffer*> mLCSHO;
    std::vector<IImageBuffer*> mPrvRSSO;
    std::vector<IImageBuffer*> mCurRSSO;
    std::vector<IImageBuffer*> mFullYuv;
    std::vector<IImageBuffer*> mRrzYuv1;
    std::vector<IImageBuffer*> mRrzYuv2;

    NSCam::IMetadata* mHalIn = NULL;
    NSCam::IMetadata* mAppIn = NULL;
    NSCam::IMetadata* mAppDynamicIn = NULL;
    NSCam::IMetadata* mAppInOverride = NULL; // for Android Physical Setting

    MVOID appendDumpInfo(LogString &str, MUINT32 sID) const;
};

class SFPSensorTuning
{
public:
    enum Flag
    {
        FLAG_NONE               = 0,
        FLAG_RRZO_IN            = 1 << 0,
        FLAG_IMGO_IN            = 1 << 1,
        FLAG_LCSO_IN            = 1 << 2,
        FLAG_FORCE_DISABLE_3DNR = 1 << 3,
        FLAG_APP_PHY_META_IN    = 1 << 4,
        FLAG_LCSHO_IN           = 1 << 5,

    };

    MUINT32 mFlag = FLAG_NONE;

    MBOOL isRRZOin() const;
    MBOOL isIMGOin() const;
    MBOOL isLCSOin() const;
    MBOOL isLCSHOin() const;
    MBOOL isDisable3DNR() const;
    MBOOL isAppPhyMetaIn() const;
    MVOID addFlag(Flag flag);
    MBOOL isValid() const;
    MVOID appendDumpInfo(LogString &str) const;
};

class SFPIOMap
{
private:
    // Input <SensorID -> SFPSensorTuning>
    std::map<MUINT32, SFPSensorTuning> mInputMap;
    SFPSensorTuning mDummy;
public:

    static const char* pathToChar(const PathType &type);
    // Output
    std::vector<SFPOutput> mOutList;
    NSCam::IMetadata* mHalOut = NULL;
    NSCam::IMetadata* mAppOut = NULL;

    // Description
    PathType mPathType = PATH_UNKNOWN;
    //VarMap mAddiVarMap; // TODO Special addition parameter for this IOMap description
    // TODO  Currently same sensor's parameter in different IOMap are all the same

    MVOID addInputTuning(MUINT32 sensorID, const SFPSensorTuning& input);
    MBOOL hasTuning(MUINT32 sensorID) const;
    const SFPSensorTuning& getTuning(MUINT32 sensorID) const;
    MVOID addOutput(const SFPOutput& out);
    MVOID getAllOutput(std::vector<SFPOutput> &outList) const;
    MVOID getAllOutput(std::vector<Feature::P2Util::P2IO> &outList) const;
    MBOOL isValid() const;
    MBOOL isGenPath() const;
    const char* pathName() const;
    MVOID appendDumpInfo(LogString &str) const;
    MUINT32 getFirstSensorID();
    MVOID getAllSensorIDs(std::vector<MUINT32>& ids) const;
    static MBOOL isSameTuning(const SFPIOMap &map1, const SFPIOMap &map2, MUINT32 sensorID);

private:
    MUINT32 mFirstID = INVALID_SENSOR_ID;

};

class SFPIOManager
{
public:
    MBOOL addInput(MUINT32 sensorID, SFPSensorInput &input);
    MBOOL addGeneral(SFPIOMap &sfpio);
    MBOOL addPhysical(MUINT32 sensorID, SFPIOMap &sfpio);
    MBOOL addLarge(MUINT32 sensorID, SFPIOMap &sfpio);
    const SFPSensorInput& getInput(MUINT32 sensorID);
    const std::vector<SFPIOMap>& getGeneralIOs() const;
    MUINT32 countAll() const;
    MUINT32 countNonLarge() const;
    MUINT32 countLarge() const;
    MUINT32 countGeneral() const;

#define MAKE_GET_FUNC(name, list, it, itobj) \
    const SFPIOMap& get##name##IO(MUINT32 sensorID) const\
    { \
        for(auto& it : list) \
        { \
            if(itobj.hasTuning(sensorID) > 0) \
            { \
                return itobj; \
            } \
        } \
        return mDummy; \
    } \
    MBOOL has##name##IO(MUINT32 sensorID) const \
    { \
        const SFPIOMap& ioMap = get##name##IO(sensorID); \
        return ioMap.isValid(); \
    }

MAKE_GET_FUNC(General, mGenerals, it, it);
MAKE_GET_FUNC(Physical, mPhysicals,it, it.second);
MAKE_GET_FUNC(Large, mLarges, it, it.second);
#undef MAKE_GET_FUNC

    const SFPIOMap& getFirstGeneralIO() const;
    MVOID appendDumpInfo(LogString &str) const;
private:
    mutable std::vector<SFPIOMap> mGenerals;
    std::unordered_map<MUINT32, SFPIOMap> mPhysicals;
    std::unordered_map<MUINT32, SFPIOMap> mLarges;
    std::unordered_map<MUINT32, SFPSensorInput> mSensorInputs;
    SFPIOMap mDummy;
    SFPSensorInput mDummyInput;
};


}; // NSFeaturePipe
}; // NSCamFeature
}; // NSCam

#undef MAKE_FEATURE_MASK_FUNC

#endif // _MTK_CAMERA_FEATURE_PIPE_SFPIO_H_
