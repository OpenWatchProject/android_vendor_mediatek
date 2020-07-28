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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_UTILS_STREAMINFO_METASTREAMINFO_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_UTILS_STREAMINFO_METASTREAMINFO_H_
//
#include <string>
#include <utility>
//
#include <mtkcam3/pipeline/stream/IStreamInfo.h>
#include "BaseStreamInfoImp.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {

class MetaStreamInfoBuilder;


/**
 * metadata stream info.
 */
class MetaStreamInfo : public IMetaStreamInfo
{
    friend MetaStreamInfoBuilder;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    struct Data : public CommonStreamInfoImplData
    {
    };
    Data    mData;

public:     ////    Instantiation.
                    MetaStreamInfo() = default;
                    MetaStreamInfo(
                        char const* streamName,
                        StreamId_T  streamId,
                        MUINT32     streamType,
                        size_t      maxBufNum,
                        size_t      minInitBufNum = 0,
                        MINT        sensorId = -1
                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamInfo Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.

    virtual char const*             getStreamName() const;

    virtual StreamId_T              getStreamId() const;

    virtual MUINT32                 getStreamType() const;

    virtual size_t                  getMaxBufNum() const;

    virtual MVOID                   setMaxBufNum(size_t count);

    virtual size_t                  getMinInitBufNum() const;

    virtual android::String8        toString() const override;

    virtual MINT                    getPhysicalCameraId() const override;

};


/**
 * meta stream info builder.
 */
class MetaStreamInfoBuilder
{
protected:  ////    Data Members.
    MetaStreamInfo::Data mData;

public:
    virtual         ~MetaStreamInfoBuilder() = default;
    virtual auto    build() const -> android::sp<IMetaStreamInfo>;

public:
    virtual auto    setStreamName(std::string&& name) -> MetaStreamInfoBuilder& {
                        mData.mStreamName = std::move(name);
                        return *this;
                    }

    virtual auto    setStreamId(StreamId_T streamId) -> MetaStreamInfoBuilder& {
                        mData.mStreamId = streamId;
                        return *this;
                    }

    virtual auto    setStreamType(MUINT32 streamType) -> MetaStreamInfoBuilder& {
                        mData.mStreamType = streamType;
                        return *this;
                    }

    virtual auto    setMaxBufNum(size_t maxBufNum) -> MetaStreamInfoBuilder& {
                        mData.mMaxBufNum = maxBufNum;
                        return *this;
                    }

    virtual auto    setMinInitBufNum(size_t minInitBufNum) -> MetaStreamInfoBuilder& {
                        mData.mMinInitBufNum = minInitBufNum;
                        return *this;
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_PIPELINE_UTILS_STREAMINFO_METASTREAMINFO_H_

