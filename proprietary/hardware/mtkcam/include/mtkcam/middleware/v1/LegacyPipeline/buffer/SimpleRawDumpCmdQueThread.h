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

#ifndef SIMPLE_RAW_DUMP_CMDQ_THREAD_H
#define SIMPLE_RAW_DUMP_CMDQ_THREAD_H
//
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <semaphore.h>
#include <mtkcam/utils/std/common.h>

#include <mtkcam/middleware/v1/camutils/CamInfo.h>
#include <mtkcam/middleware/v1/LegacyPipeline/processor/StreamingProcessor.h>

using namespace NSCam;
using namespace NSCam::v1;

struct SimpleRawImageBufInfo
{
    MUINT32 u4Width;
    MUINT32 u4Height;
    MUINT32 u4Stride0;
    MUINT32 u4Stride1;
    MUINT32 u4Stride2;
    MUINTPTR u4VirtAddr;
    MUINT32 u4Size;
    MINT32 i4RequestNo;
    MINT32 iMagicNum;
    MUINT u4ImgFormat;
    String8 s8Name;
    MINT64 i8Timestamp;

    SimpleRawImageBufInfo(
        MUINT32 _u4Width,
        MUINT32 _u4Height,
        MUINT32 _u4Stride0,
        MUINT32 _u4Stride1,
        MUINT32 _u4Stride2,
        MUINTPTR _u4VirtAddr,
        MUINT32 _u4Size,
        MINT32  _i4RequestNo,
        MINT32  _iMagicNum,
        MUINT32 _u4ImgFormat,
        String8 _s8Name,
        MINT64 _i8Timestamp
    )
    : u4Width(_u4Width)
    , u4Height(_u4Height)
    , u4Stride0(_u4Stride0)
    , u4Stride1(_u4Stride1)
    , u4Stride2(_u4Stride2)
    , u4VirtAddr(_u4VirtAddr)
    , u4Size(_u4Size)
    , i4RequestNo(_i4RequestNo)
    , iMagicNum(_iMagicNum)
    , u4ImgFormat(_u4ImgFormat)
    , s8Name(_s8Name)
    , i8Timestamp(_i8Timestamp)
    {
    }
};

/******************************************************************************
*
*******************************************************************************/
class SimpleRawDumpCmdCookie : public virtual RefBase
{
public:
    SimpleRawDumpCmdCookie(
        MUINT32 _frame_count,
        SimpleRawImageBufInfo *_pSimpleRawImageBufInfo,
        String8 _s8RawFilePath
        )
        : mFrameCnt (_frame_count)
        , mpSimpleRawImageBufInfo(_pSimpleRawImageBufInfo)
        , ms8RawFilePath(_s8RawFilePath)
    {
    }
    MUINT32 getFrameCnt() const { return mFrameCnt; }
    String8 getRawFilePath() const {return ms8RawFilePath;}
    SimpleRawImageBufInfo* getSimpleRawImageBufInfo() const { return mpSimpleRawImageBufInfo; }
private:
    MUINT32 mFrameCnt;
    SimpleRawImageBufInfo *mpSimpleRawImageBufInfo;
    String8   ms8RawFilePath;
};

/******************************************************************************
*
*******************************************************************************/
class SimpleMetaDumpCmdCookie : public virtual RefBase
{
public:
    SimpleMetaDumpCmdCookie(
        MUINT32 _frame_count,
        IMetadata &_SimpleMetaInfo,
        String8 _s8FilePath
        )
        : mFrameCnt (_frame_count)
        , mMetaInfo(_SimpleMetaInfo)
        , ms8FilePath(_s8FilePath)
    {
    }
    MUINT32 getFrameCnt() const { return mFrameCnt; }
    String8 getFilePath() const {return ms8FilePath;}
    MVOID getSimpleMetaInfo(IMetadata& metaInfo) const { metaInfo = mMetaInfo; }
private:
    MUINT32 mFrameCnt;
    IMetadata mMetaInfo;
    String8   ms8FilePath;
};


/******************************************************************************
*
*******************************************************************************/
class ISimpleRawDumpCmdQueThread : public Thread
{
public:
    static ISimpleRawDumpCmdQueThread*    createInstance(MUINT32 u4BitOrder, MUINT32 u4BitDepth);
public:
    virtual bool postCommand(IImageBuffer* pImgBuffer, MINT32 rRequestNo, MINT32 magicNum, String8 sName)= 0;
    virtual bool postCommand(IMetadata &meta, String8 sFilePath)= 0;
};

#endif

