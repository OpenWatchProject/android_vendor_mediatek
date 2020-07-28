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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _MTKCAM_FEATURE_UTILS_P2_DIP_STREAM_H_
#define _MTKCAM_FEATURE_UTILS_P2_DIP_STREAM_H_

#include <mtkcam/def/common.h>
#include <mtkcam3/feature/utils/log/ILogger.h>
#include <mtkcam/drv/def/IPostProcDef.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

#define MTK_V4L2_READY 0
#if MTK_V4L2_READY
#include <mtkdip.h>
#endif

namespace NSCam {
namespace Feature {
namespace P2Util {

using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::FrameParams;

#define INVALID_SENSOR_ID (MUINT32)(-1)

class DIPFrameParams
{
public:
    DIPFrameParams()
        : mvIn()
        , mvOut()
        , mvCropRsInfo()
        , mvModuleData()
        , mvExtraParam()
    {
        ExpectedEndTime.tv_sec = 0;
        ExpectedEndTime.tv_usec = 0;
    }
    virtual ~DIPFrameParams(){}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//protected:
public:
    MUINT32 FrameNo = 0;
    MUINT32 RequestNo = 0;
    MUINT32 Timestamp = 0;
    MINT32 UniqueKey = 0;
    MINT32 IspProfile = -1;
    MINT32 SensorDev = -1;
    MINT32 FrameIdentify = 0; //by User Defined
    NSIoPipe::P2_RUN_INDEX mRunIdx = NSIoPipe::P2_RUN_UNKNOWN;
    MBOOL NeedDump = MFALSE;
    MINT32 mStreamTag = -1;
    MINT32 mSensorIdx = -1;
    MBOOL mSecureFra = 0;
    MVOID* mTuningData = NULL;
    MVOID* mpCookie = NULL;
    struct timeval ExpectedEndTime;
    std::vector<NSIoPipe::Input>       mvIn;
    std::vector<NSIoPipe::Output>      mvOut;
    std::vector<NSIoPipe::MCrpRsInfo>  mvCropRsInfo;
    std::vector<NSIoPipe::ModuleInfo>     mvModuleData;
    std::vector<NSIoPipe::ExtraParam>     mvExtraParam;
    typedef bool (*PFN_FRAMECB_T)(const FrameParams& rFrmParams, NSIoPipe::EFrameCallBackMsg FrmCBMsg);  // per frame callback
    PFN_FRAMECB_T mpfnCallback = NULL; //deque call back
};

class DIPParams
{
public:
    DIPParams()
        : mvDIPFrameParams()
    {
    }
    virtual ~DIPParams(){}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    typedef MVOID               (*PFN_DIP_CALLBACK_T)(DIPParams &dipParams);
    PFN_DIP_CALLBACK_T          mpfnDIPCallback = NULL;   //deque call back
    PFN_DIP_CALLBACK_T          mpfnDIPEnQFailCallback = NULL;    //call back for enque fail
    PFN_DIP_CALLBACK_T          mpfnDIPEnQBlockCallback = NULL;   //call back for enque blocking
    MVOID*                      mpCookie = NULL;
    MBOOL                       mDequeSuccess = MFALSE;
    std::vector<DIPFrameParams> mvDIPFrameParams;
    uint64_t                    mSeqNum = 0;
};

class DIPStream
{
public:
    DIPStream(MUINT32 sensorIndex);
    virtual ~DIPStream();
    static  DIPStream* createInstance(MUINT32 sensorIndex);
    virtual MVOID destroyInstance();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual MBOOL init(char const* szCallerName,
                NSCam::NSIoPipe::EStreamPipeID pipeID=NSCam::NSIoPipe::EStreamPipeID_Normal,
                MUINT32 secTag=0) = 0;
    virtual MBOOL uninit(char const* szCallerName = "") = 0;
    virtual MBOOL enque(const DIPParams &dipParams) = 0;
    virtual MBOOL sendCommand(NSIoPipe::NSPostProc::ESDCmd cmd, MINTPTR arg1=0, MINTPTR arg2=0, MINTPTR arg3=0) {(void)cmd; (void)arg1; (void)arg2; (void)arg3; return MTRUE;}
    virtual MBOOL setJpegParam(NSIoPipe::NSPostProc::EJpgCmd jpgCmd, int arg1, int arg2) {(void)jpgCmd; (void)arg1; (void)arg2; return MTRUE;}
    virtual MBOOL setFps(MINT32 fps) {(void)fps; return MTRUE;}
    static  MUINT32 getRegTableSize();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MUINT32 mSensorIndex = INVALID_SENSOR_ID;
    MBOOL   mLogEn = MFALSE;
};

} // namespace P2Util
} // namespace Feature
} // namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_P2_DIP_STREAM_H_