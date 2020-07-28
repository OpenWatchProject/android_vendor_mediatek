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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef _MTK_CAMERA_P2_NODE_TEST_MOCKS_H_
#define _MTK_CAMERA_P2_NODE_TEST_MOCKS_H_

#include "TestP2_Common.h"

namespace P2
{
//********************************************
// MWMetaMock
//********************************************
class MWMetaMock : virtual public P2Meta
{
public:
    MWMetaMock(const ILog &log, const P2Pack &p2Pack, const StreamId_T &streamID, IO_DIR dir, const META_INFO &info);
    virtual ~MWMetaMock();
    virtual StreamId_T getStreamID() const;
    virtual MBOOL isValid() const;
    virtual IO_DIR getDir() const;
    virtual MVOID updateResult(MBOOL result);
    virtual IMetadata* getIMetadataPtr() const;
    virtual IMetadata::IEntry getEntry(MUINT32 tag) const;
    virtual MBOOL setEntry(MUINT32 tag, const IMetadata::IEntry &entry);

    // Not Implement
    virtual MVOID detach(){};

private:
    StreamId_T mStreamID;
    IO_DIR mDir;
    IO_STATUS mStatus;
    IMetadata* mMetadata;
};

//********************************************
// MWImgMock
//********************************************
class MWImgMock : virtual public P2Img
{
public:
    struct Config
    {
        MSize size        = MSize(0,0);
        MUINT32 format    = eImgFmt_UNKNOWN;
        MUINT32 transform = 0;
        Config(const MSize& __size, const MUINT32& fmt, const MUINT32& trans)
        : size(__size)
        , format(fmt)
        , transform(trans)
        {};
    };

    MWImgMock(
        const ILog &log, const P2Pack &p2Pack,
        const StreamId_T &streamID,IO_DIR dir,
        const IMG_INFO &info, MUINT32 debugIndex,
        const MWImgMock::Config& config,
        const MUINT32 usage);
    virtual ~MWImgMock();
    virtual MBOOL isValid() const;
    virtual IO_DIR getDir() const;
    MVOID registerPlugin(const std::list<sp<P2ImgPlugin>> &plugin);
    virtual MVOID updateResult(MBOOL result);
    virtual IImageBuffer* getIImageBufferPtr() const;
    virtual MUINT32 getTransform() const;
    virtual MUINT32 getUsage() const;
    virtual MBOOL isDisplay() const;
    virtual MBOOL isRecord() const;
    virtual MBOOL isCapture() const;

    // not implement
    virtual MVOID updateVRTimestamp(MUINT32 count, const MINT64 &cam2FwTs) {}
    virtual std::vector<IImageBuffer*> getIImageBufferPtrs() const { return std::vector<IImageBuffer*>(); }
    virtual MUINT32 getIImageBufferPtrsCount() const {return 0;}
    virtual MBOOL isPhysicalStream() const {return MFALSE;}
    virtual MVOID detach() {}
    virtual IMG_TYPE getImgType() const {return IMG_TYPE_EXTRA;}

private:
    MVOID processPlugin() const;

private:
    StreamId_T mStreamID;
    IO_DIR mDir;
    IO_STATUS mStatus;
    MUINT32 mTransform;
    MUINT32 mUsage;
    std::list<sp<P2ImgPlugin>> mPlugin;

    android::sp<ImageBufferPool> mpImagePool;
    SmartImageBuffer             mSmpBuf;
};

//********************************************
// MWFrameRequestMock
//********************************************
class MWFrameRequestMock : public P2FrameRequest
{
public:
    MWFrameRequestMock(
        const ILog &log,
        const P2Pack &pack,
        const sp<P2DataObj> &p2Data,
        const sp<P2InIDMap> &p2IdMap,
        const set<MWRequestPath>& vRequestPaths
    );

    virtual MVOID beginBatchRelease() override;
    virtual MVOID endBatchRelease() override;
    virtual MVOID notifyNextCapture(int requestCnt, MBOOL bSkipCheck) override;
    virtual MVOID metaResultAvailable(IMetadata* partialMeta) override;
    virtual std::vector<sp<P2Request>> extractP2Requests() override;
    virtual ~MWFrameRequestMock();
private:
    MVOID fillP2Img(sp<P2Request> request, const MWRequestPath type = MWRequestPath::eGeneral);
    MVOID fillP2Meta(sp<P2Request> request, const MWRequestPath type = MWRequestPath::eGeneral);
    MVOID fillDefaultP2Meta(sp<P2Request> request);
    MVOID updateP2FrameData();
    MVOID updateP2SensorData();

    MUINT32 mStreamCount = 0;
    sp<P2DataObj> mP2Data;
    unordered_map<ID_META, sp<P2Meta>> mMetaMap;
    const set<MWRequestPath> mvRequestPaths;
};

} // namespace P2

#endif
