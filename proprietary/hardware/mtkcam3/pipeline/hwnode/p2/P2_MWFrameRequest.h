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

#ifndef _MTKCAM_HWNODE_P2_MW_FRAME_REQUEST_H_
#define _MTKCAM_HWNODE_P2_MW_FRAME_REQUEST_H_

#include "P2_Param.h"
#include "P2_Request.h"
#include "P2_MWData.h"

namespace P2
{

class MWFrameRequest : public P2FrameRequest
{
public:
    MWFrameRequest(const ILog &log, const P2Pack &pack, const sp<P2DataObj> &p2Data, const sp<MWInfo> &mwInfo, const sp<MWFrame> &frame, const sp<P2InIDMap> &p2IdMap, double fps = 0);
    virtual ~MWFrameRequest();

    virtual MVOID beginBatchRelease();
    virtual MVOID endBatchRelease();
    virtual MVOID notifyNextCapture(int requestCnt, MBOOL bSkipCheck);
    virtual MVOID metaResultAvailable(IMetadata* partialMeta);
    virtual std::vector<sp<P2Request>> extractP2Requests();

private:
    typedef std::map<ID_IMG, sp<P2Img>> P2ImgMap;
    typedef std::map<StreamId_T, sp<P2Meta>> P2MetaMap;
    typedef std::map<StreamId_T, sp<P2Img>> P2ImgOutMap;
    typedef std::set<StreamId_T> StreamSet;

    MBOOL addP2Img(P2ImgMap &imgMap, const StreamId_T &sID, IO_DIR dir);
    P2ImgMap createP2ImgMap(IPipelineFrame::ImageInfoIOMapSet &imgSet);
    MBOOL addP2Meta(P2MetaMap &metaMap, const StreamId_T &sID, ID_META id, IO_DIR dir, const META_INFO &info);
    MBOOL addP2Meta(P2MetaMap &metaMap, const StreamId_T &sID, IO_DIR dir);
    MBOOL addP2Meta(P2MetaMap &metaMap, ID_META id, IO_DIR dir);
    MBOOL removeP2Meta(P2MetaMap &metaMap, ID_META id);
    P2MetaMap createP2MetaMap(IPipelineFrame::MetaInfoIOMapSet &metaSet);
    MVOID configInputBuffer(MUINT32 sensorID, const sp<Cropper> &cropper, const std::vector<MINT64> &timestamp);
    MSize queryInputImgSize(ID_IMG imgID, const sp<Cropper> &cropper) const;
    sp<P2Meta> findP2Meta(const P2MetaMap &metaMap, ID_META id) const;
    sp<P2Meta> findP2Meta(ID_META id) const;
    sp<P2Img> findP2InImg(const P2ImgMap &imgMap, ID_IMG id) const;
    sp<P2Img> findP2InImg(ID_IMG id) const;
    MVOID initP2FrameData();
    MVOID updateP2FrameData();
    MVOID updateP2SensorData();
    MVOID updateP1TS(const sp<P2Meta> &p1App, const sp<P2Meta> &p1Hal, P2SensorData &data);
    MVOID updateP2Metadata(MUINT32 sensorID, const sp<Cropper> &cropper);
    MBOOL fillP2Img(const sp<P2Request> &request, const sp<P2Img> &img);
    MVOID fillP2Img(const sp<P2Request> &request, const IPipelineFrame::ImageInfoIOMap &imgInfoMap, const P2ImgMap &p2ImgMap);
    MBOOL fillP2Meta(const sp<P2Request> &request, const sp<P2Meta> &meta);
    MVOID fillP2Meta(const sp<P2Request> &request, const IPipelineFrame::MetaInfoIOMap &metaInfoMap, const P2MetaMap &p2MetaMap);
    MVOID printIOMap(const IPipelineFrame::InfoIOMapSet &ioMap);
    std::vector<sp<P2Request>> createRequests(IPipelineFrame::InfoIOMapSet &ioMap);
    MVOID doRegisterPlugin();
    MUINT32 decideMasterID();

private:
    sp<P2DataObj> mP2Data;
    sp<MWInfo> mMWInfo;
    sp<MWFrame> mMWFrame;
    MBOOL mExtracted;
    P2MetaMap mMetaMap;
    P2ImgMap mImgMap;
    P2ImgOutMap mImgOutMap;
    StreamSet mStreamSet;
    MUINT32 mImgStreamCount;
    double mFPS = 0;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_MW_FRAME_REQUEST_H_
