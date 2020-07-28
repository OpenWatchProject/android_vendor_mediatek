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

#ifndef _MTKCAM_FEATURE_UTILS_P2_NORMAL_STREAM_H_
#define _MTKCAM_FEATURE_UTILS_P2_NORMAL_STREAM_H_

#include "DIPStream.h"

#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>

#define MAGIC_PASS_NUM 0xABCDEFAB

namespace NSCam {
namespace Feature {
namespace P2Util {

class DIPStream_NormalStream
    : public DIPStream
{
public:
    DIPStream_NormalStream(MUINT32 sensorIndex);
    virtual ~DIPStream_NormalStream();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MBOOL init(char const* szCallerName,
                NSCam::NSIoPipe::EStreamPipeID pipeID=NSCam::NSIoPipe::EStreamPipeID_Normal,
                MUINT32 secTag=0) override;
    MBOOL uninit(char const* szCallerName = "") override;
    MBOOL enque(const DIPParams &dipParams) override;
    MBOOL sendCommand(NSIoPipe::NSPostProc::ESDCmd cmd, MINTPTR arg1=0, MINTPTR arg2=0, MINTPTR arg3=0) override;
    MBOOL setJpegParam(NSIoPipe::NSPostProc::EJpgCmd jpgCmd,int arg1,int arg2) override;
    MBOOL setFps(MINT32 fps) override;
    static MUINT32 getRegTableSize();
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Internal Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    static MVOID normalStreamCB(QParams &qParams);
    static MVOID normalStreamFailCB(QParams &qParams);
    static MVOID normalStreamBlockCB(QParams &qParams);
    QParams convertToQParams(const DIPParams &dipParams);
    static DIPParams convertToDIPParams(const QParams &qParams);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
private:
    NSCam::NSIoPipe::NSPostProc::INormalStream *mNormalStream = NULL;

    struct CookieExt
    {
        MVOID*                      mpOriCookie = NULL;
        typedef MVOID               (*PFN_DIP_CALLBACK_T)(DIPParams &dipParams);
        PFN_DIP_CALLBACK_T          mpfnDIPCallback = NULL;   //deque call back
        PFN_DIP_CALLBACK_T          mpfnDIPEnQFailCallback = NULL;    //call back for enque fail
        PFN_DIP_CALLBACK_T          mpfnDIPEnQBlockCallback = NULL;   //call back for enque blocking
        uint32_t                    mMagicCheck = MAGIC_PASS_NUM;
        //
        CookieExt()
        {
        }
    };
};

} // namespace P2Util
} // namespace Feature
} // namespace NSCam

#endif // _MTKCAM_FEATURE_UTILS_P2_NORMAL_STREAM_H_