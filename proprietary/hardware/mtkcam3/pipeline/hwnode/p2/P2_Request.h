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

#ifndef _MTKCAM_HWNODE_P2_REQUEST_H_
#define _MTKCAM_HWNODE_P2_REQUEST_H_

#include "P2_Param.h"
#include "P2_LMVInfo.h"
#include "P2_Cropper.h"

namespace P2
{

class IP2Frame : virtual public android::RefBase
{
public:
    virtual ~IP2Frame() {}
    virtual MVOID beginBatchRelease() = 0;
    virtual MVOID endBatchRelease() = 0;
    virtual MVOID notifyNextCapture(int requestCnt, MBOOL bSkipCheck) = 0;
    virtual MVOID metaResultAvailable(IMetadata* partialMeta) = 0;
};

class P2FrameHolder : virtual public IP2Frame
{
public:
    P2FrameHolder(const sp<IP2Frame> &frame);
    virtual ~P2FrameHolder();
    virtual MVOID beginBatchRelease();
    virtual MVOID endBatchRelease();
    virtual MVOID notifyNextCapture(int requestCnt, MBOOL bSkipCheck);
    virtual MVOID metaResultAvailable(IMetadata* partialMeta);

protected:
    sp<IP2Frame> getIP2Frame() const;

private:
    sp<IP2Frame> mFrame;
};

class P2Request : virtual public android::RefBase, virtual public P2FrameHolder
{
public:
    enum {  RES_IN_IMG    = (1<<0),
            RES_IN_META   = (1<<1),
            RES_OUT_IMG   = (1<<2),
            RES_OUT_META  = (1<<3),
            RES_IMG       = RES_IN_IMG | RES_OUT_IMG,
            RES_META      = RES_IN_META | RES_OUT_META,
            RES_ALL       = RES_IMG | RES_META };

public:
    P2Request(const ILog &log, const sp<IP2Frame> &frameHolder, const P2Pack &p2Pack, const sp<P2InIDMap> &p2IdMap);
    P2Request(const sp<P2Request> &request);
    P2Request(const sp<P2Request> &request, MUINT32 sensorID);
    virtual ~P2Request();

public:
    MVOID updateSensorID();
    MVOID initIOInfo();
    MUINT32 getSensorID() const;
    MUINT32 getSlaveSensorID() const;
    sp<Cropper> getCropper() const;
    sp<Cropper> getCropper(MUINT32 sensorID) const;
    MBOOL hasInput() const;
    MBOOL hasOutput() const;
    MBOOL isResized() const;
    MBOOL isReprocess() const;
    MBOOL isPhysic() const;
    MBOOL isLarge() const;
    MVOID releaseResource(MUINT32 res);
    MVOID releaseResourceWithLock(MUINT32 res);
    MVOID detachResourceWithLock(MUINT32 res);
    MVOID releaseMetaWithLock(ID_META id);
    MVOID releaseImageWithLock(ID_IMG id);
    P2MetaSet getMetaSet() const;
    MVOID updateMetaSet(const P2MetaSet &set);
    MVOID updateResult(MBOOL result);
    MVOID updateMetaResult(MBOOL result);
    MVOID dump() const;
    sp<P2Request> makeDetachP2Request();

    sp<P2Meta> getMeta(ID_META id) const;
    IMetadata *getMetaPtr(ID_META id) const;
    sp<P2Meta> getMeta(ID_META id, MUINT32 sensorID) const;
    IMetadata *getMetaPtr(ID_META id, MUINT32 sensorID) const;

    sp<P2Img> getImg(ID_IMG id) const;
    sp<P2Img> getImg(ID_IMG id, MUINT32 sensorID) const;
    sp<P2Img>&& moveImg(ID_IMG id, MUINT32 sensorID);

    ID_META getMetaID(ID_META metaId, MUINT32 sensorID);
    ID_IMG getImgID(ID_IMG imgId, MUINT32 sensorID);

    MBOOL isValidMeta(ID_META id) const;
    MBOOL isValidMeta(ID_META id, MUINT32 sensorID) const;
    MBOOL isValidImg(ID_IMG id) const;
    MBOOL isValidImg(ID_IMG id, MUINT32 sensorID) const;
    MBOOL hasValidMirrorMeta(ID_META id) const;

private:
    P2Request(const sp<P2Request> &request, const sp<IP2Frame> &frame);
    MVOID updateDummyAppResult(ID_META id);
    MBOOL hasSensorIn(MUINT32 sensorID) const;

private:
    MUINT32 mSensorID = INVALID_SENSOR_ID;
public:
    const ILog mLog;
    const P2Pack mP2Pack;
    P2DumpType mDumpType = P2_DUMP_NONE;

public:
    /* Below data will NOT be copied during P2Request copy constructor */
    // TODO: replace policy by comment to policy by code
    std::unordered_map<ID_META, sp<P2Meta>> mMeta;
    std::unordered_map<ID_IMG, sp<P2Img>> mImg;
    std::vector<sp<P2Img> > mImgOutArray;

protected:
    /* Below data will be copied during P2Request copy constructor */
    // TODO: replace policy by comment to policy by code
    sp<P2InIDMap> mInIDMap;

private:
    MBOOL mIsResized = MFALSE;
    MBOOL mIsReprocess = MFALSE;
    MBOOL mIsPhysic = MFALSE;
    MBOOL mIsLarge = MFALSE;
    std::list<MUINT32> mSlaveSensorIDs;
    mutable Mutex mLock;
};

class P2FrameRequest : virtual public android::RefBase, public IP2Frame
{
public:
    P2FrameRequest(const ILog &log, const P2Pack &pack, const sp<P2InIDMap> &p2IdMap);
    virtual ~P2FrameRequest();
    virtual std::vector<sp<P2Request>> extractP2Requests() = 0;

public:
    MUINT32 getMainSensorID() const;
    MUINT32 getFrameID() const;
    MVOID registerImgPlugin(const sp<P2ImgPlugin> &plugin, MBOOL needSWRW = MFALSE);

public:
    const ILog mLog;

protected:
    ID_META mapID(MUINT32 sensorID, ID_META metaId);
    ID_IMG mapID(MUINT32 sensorID, ID_IMG imgId);


    P2Pack mP2Pack;
    sp<P2InIDMap> mInIDMap;
    std::list<sp<P2ImgPlugin>> mImgPlugin;
    MBOOL mNeedImageSWRW = MFALSE;
};

} // namespace P2

#endif // _MTKCAM_HWNODE_P2_REQUEST_H_
