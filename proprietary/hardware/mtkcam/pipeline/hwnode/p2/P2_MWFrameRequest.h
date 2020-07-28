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

class MWFrameRequest : virtual public P2FrameRequest, virtual public LoggerHolder
{
public:
    MWFrameRequest(const sp<P2Info> &p2Info, const sp<MWInfo> &mwInfo, const sp<MWFrame> &frame);
    virtual ~MWFrameRequest();

    virtual MVOID beginBatchRelease();
    virtual MVOID endBatchRelease();
    virtual std::vector<sp<P2Request>> extractP2Requests();

private:
    typedef std::map<StreamId_T, sp<P2Img>> P2ImgMap;
    typedef std::map<StreamId_T, sp<P2Meta>> P2MetaMap;

    MBOOL addP2Img(P2ImgMap &imgMap, const StreamId_T &sID, ID_IMG id, IO_DIR dir, const IMG_INFO &info);
    MBOOL addP2Img(P2ImgMap &imgMap, const StreamId_T &sID, IO_DIR dir);
    MBOOL addP2Img(P2ImgMap &imgMap, ID_IMG id, IO_DIR dir);
    P2ImgMap createP2ImgMap(IPipelineFrame::ImageInfoIOMapSet &imgSet);
    MBOOL addP2Meta(P2MetaMap &metaMap, const StreamId_T &sID, ID_META id, IO_DIR dir, const META_INFO &info);
    MBOOL addP2Meta(P2MetaMap &metaMap, const StreamId_T &sID, IO_DIR dir);
    MBOOL addP2Meta(P2MetaMap &metaMap, ID_META id, IO_DIR dir);
    MBOOL removeP2Meta(P2MetaMap &metaMap, ID_META id);
    P2MetaMap createP2MetaMap(IPipelineFrame::MetaInfoIOMapSet &metaSet);
    MVOID configBufferSize(const Cropper &cropper);
    sp<P2Meta> findP2Meta(const P2MetaMap &metaMap, ID_META id) const;
    sp<P2Img> findP2Img(const P2ImgMap &imgMap, ID_IMG id) const;
    Cropper createCropper(const P2MetaMap &metaMap);
    P2ExtraData createP2ExtraData();
    MVOID updateP2ExtraFromMetadata(P2ExtraData &extra);
    MVOID updateP2Metadata();
    MBOOL fillP2Img(const sp<P2Request> &request, const sp<P2Img> &img);
    MVOID fillP2Img(const sp<P2Request> &request, const IPipelineFrame::ImageInfoIOMap &imgInfoMap, const P2ImgMap &p2ImgMap);
    MBOOL fillP2Meta(const sp<P2Request> &request, const sp<P2Meta> &meta);
    MVOID fillP2Meta(const sp<P2Request> &request, const IPipelineFrame::MetaInfoIOMap &metaInfoMap, const P2MetaMap &p2MetaMap);
    MVOID fillDefaultP2Meta(const sp<P2Request> &request, const P2MetaMap &metaMap);
    MVOID printIOMap(const IPipelineFrame::InfoIOMapSet &ioMap);
    std::vector<sp<P2Request>> createRequests(IPipelineFrame::InfoIOMapSet &ioMap);
    MVOID doP1AppMetaWorkaround();
    MVOID doRegisterPlugin();

    template <typename T>
    T getMeta(ID_META id, MUINT32 tag, T val);
    template <typename T>
    MBOOL tryGetMeta(ID_META id, MUINT32 tag, T &val);

private:
    sp<MWInfo> mMWInfo;
    sp<MWFrame> mMWFrame;
    MBOOL mExtracted;
    P2MetaMap mMetaMap;
    P2ImgMap mImgMap;
    MUINT32 mImgCount;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_MW_FRAME_REQUEST_H_
