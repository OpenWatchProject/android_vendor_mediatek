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

#ifndef RAW_DUMP_CMDQ_THREAD_H
#define RAW_DUMP_CMDQ_THREAD_H
//
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <semaphore.h>
#include <mtkcam/middleware/v1/IParamsManager.h>
#include <mtkcam/def/common.h>

#include <mtkcam/middleware/v1/camutils/CamInfo.h>
#include <mtkcam/middleware/v1/LegacyPipeline/processor/StreamingProcessor.h>

using namespace NSCam;
using namespace NSCam::v1;

struct RawImageBufInfo
{
    MUINT32 u4Width;
    MUINT32 u4Height;
    MUINT32 u4Stride;
    MUINTPTR u4VirtAddr;
    MUINT32 u4Size;
    MINT32 i4RequestNo;

    RawImageBufInfo(
        MUINT32 _u4Width,
        MUINT32 _u4Height,
        MUINT32 _u4Stride,
        MUINTPTR _u4VirtAddr,
        MUINT32 _u4Size,
        MINT32  _i4RequestNo
    )
    : u4Width(_u4Width)
    , u4Height(_u4Height)
    , u4Stride(_u4Stride)
    , u4VirtAddr(_u4VirtAddr)
    , u4Size(_u4Size)
    , i4RequestNo(_i4RequestNo)
    {
    }
};

/******************************************************************************
*
*******************************************************************************/
class RawDumpCmdCookie : public virtual RefBase
{
public:
    RawDumpCmdCookie(
        MUINT32 _frame_count,
        RawImageBufInfo *_pRawImageBufInfo,
        String8 _s8RawFilePath
        )
        : mFrameCnt (_frame_count)
        , mpRawImageBufInfo(_pRawImageBufInfo)
        , ms8RawFilePath(_s8RawFilePath)
    {
    }
    MUINT32 getFrameCnt() const { return mFrameCnt; }
    String8 getRawFilePath() const {return ms8RawFilePath;}
    RawImageBufInfo* getRawImageBufInfo() const { return mpRawImageBufInfo; }
private:
    MUINT32 mFrameCnt;
    RawImageBufInfo *mpRawImageBufInfo;
    String8   ms8RawFilePath;

};


/******************************************************************************
*
*******************************************************************************/
class IRawDumpCmdQueThread : public Thread
{
public:
    static IRawDumpCmdQueThread*    createInstance(MUINT32 u4BitOrder, MUINT32 u4BitDepth, sp<IParamsManager> pParamsMgr);
public:
    virtual bool postCommand(IImageBuffer* pImgBuffer,MINT32 rRequestNo)= 0;
    virtual void setCallbacks(sp<INotifyCallback> const& rpCamMsgCbInfo) = 0;
};

#endif

