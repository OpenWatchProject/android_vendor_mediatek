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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_EIS_NODE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_EIS_NODE_H_

#include "StreamingFeatureNode.h"
#include "FMHal.h"

class EisHal;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum EISAlgMode
{
    EISALG_QUEUE_NONE = 0,
    EISALG_QUEUE_INIT = 1,
    EISALG_QUEUE_WAIT = 2,
    EISALG_QUEUE      = 3,
    EISALG_QUEUE_STOP = 4,
};

class EISQData
{
public:
    EISQData()
        : mRequest(NULL)
    {
    }

    EISQData(const RequestPtr &request, const EIS_HAL_CONFIG_DATA &config, const MSizeF &offset = MSizeF())
        : mRequest(request)
        , mConfig(config)
        , mDomainOffset(offset)
    {
    }

    MVOID setAlg(EISAlgMode mode, MINT32 counter)
    {
        mAlgMode = mode;
        mAlgCounter = counter;
    }

    MINT32 getReqNo() const
    {
        return (mRequest == NULL) ? -1 : mRequest->mRequestNo;
    }

    RequestPtr mRequest;
    EIS_HAL_CONFIG_DATA mConfig;
    MSizeF mDomainOffset;
    EISAlgMode mAlgMode = EISALG_QUEUE_NONE;
    MINT32 mAlgCounter = 0;
};

class EISNode : public StreamingFeatureNode
{
public:
    EISNode(const char *name);
    virtual ~EISNode();

public:
    virtual MBOOL onData(DataID id, const RequestPtr &data);
    virtual MBOOL onData(DataID id, const FMData &data);
    virtual MBOOL onData(DataID id, const RSCData &data);
    virtual MBOOL onData(DataID id, const FOVData &data);

protected:
    virtual MBOOL onInit();
    virtual MBOOL onUninit();
    virtual MBOOL onThreadStart();
    virtual MBOOL onThreadStop();
    virtual MBOOL onThreadLoop();

private:
    MBOOL initEIS();
    MVOID uninitEIS();
    MVOID prepareBufferPool();
    MBOOL prepareEIS(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config);
    MVOID extractConfig(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config);
    MBOOL applyConfig(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config);

    MVOID processEIS(const RequestPtr &request, const FMResult &fm, const RSCResult &rsc, const FOVResult &fov);
    MBOOL processEIS22(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config);
    MBOOL processEIS25(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config, const FMResult &fm);
    MBOOL processEIS30(const RequestPtr &request, EIS_HAL_CONFIG_DATA &config, const RSCResult &rsc, const FOVResult &fov);

    MVOID prepareFEFM(const FMResult &fm, FEFM_PACKAGE &fefmCfg);
    MVOID prepareRSC(const RSCResult &rsc, RSCME_PACKAGE &rscCfg, EIS_HAL_CONFIG_DATA &config);
    MVOID prepareFOV(const RequestPtr &request, const FOVResult &fov, EIS_HAL_CONFIG_DATA &config);
    MVOID prepareFSC(const RequestPtr &request, FSC_WARPING_DATA_STRUCT &fsc, FSC_PACKAGE &fscCfg);

    EISQData decideQData(const RequestPtr &request, const EIS_HAL_CONFIG_DATA &config);
    MVOID configAdvEISConfig(const RequestPtr &request, const ImgBuffer &warp, const EISQData &qData, EIS_HAL_CONFIG_DATA &config);

    MSizeF getDomainOffset(const RequestPtr &request);

    MBOOL handleWarpResult(const RequestPtr &request, const ImgBuffer &warp, const MSizeF &domainOffset);
    MBOOL handleQDataWarpResult(const RequestPtr &request, const ImgBuffer &warp, const EISQData &qData);

    MBOOL needFlushAll(const RequestPtr &request);
    MVOID flushAll();

private:
    WaitQueue<FMData> mRequests;
    WaitQueue<RSCData> mRSCDatas;
    WaitQueue<FOVData> mFOVDatas;
    android::sp<FatImageBufferPool> mWarpMapBufferPool;

    EisHal* mpEisHal;
    MSize mWarpGridSize;
    EIS_HAL_CONFIG_DATA mEISHalCfgData;

    FMHal::Config mFEFMConfig[3];
    MULTISCALE_INFO mFEFMScale;
    FSC_INFO        mFSCInfo;

    std::deque<EISQData> mQueue;
    MBOOL mInFlush;
    NSCam::EIS::EisInfo mEisInfo;

    MUINT32 mRefCount = 0;
    MUINT32 mLogCount = 0;
    MBOOL mEnableDump = MFALSE;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_EIS_NODE_H_
