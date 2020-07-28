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

#define LOG_TAG "MtkCam/HwPipeline"

#include "PipelineModelManagerBase.h"
#include "MyUtils.h"
//
#include <mtkcam/v3/pipeline/IPipelineModel.h>
// #include <mtkcam/v3/pipeline/IPipelineResourceMgr.h>            //ISP2.0
#include <mtkcam/v3/pipeline/IPipelineFrameNumberGenerator.h>   //ISP2.0,ISP3.0
#include <mtkcam/v3/hwpipeline/PipelineModelFactory.h>          //ISP2.0,ISP3.0
#include <mtkcam/utils/sys/CpuCtrl.h>                               //ISP3.5
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("%d[PipelineModelManager::%s] " fmt, mId, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
template <class IPipelineCallbackT, class IModelT>
class PipelineModelManagerTemplate
    : public PipelineModelManagerBase
    , public IPipelineCallbackT
{
public:     ////    Operations.

                    PipelineModelManagerTemplate(AppCreation const& params)
                        : PipelineModelManagerBase(params)
                    {
                    }

    virtual auto    getPipeline() const -> android::sp<IModelT>             = 0;

    virtual auto    submitOneRequest(AppRequest const& request) -> int
                    {
                        int err = 0;
                        auto pPipelineModel = getPipeline();
                        if  ( pPipelineModel == nullptr ) {
                            return DEAD_OBJECT;
                        }

                        typename IModelT::AppRequest appRequest;
                        appRequest.requestNo        = request.requestNo;
                        appRequest.vIImageBuffers   = request.vIImageBuffers;
                        appRequest.vOImageBuffers   = request.vOImageBuffers;
                        appRequest.vIMetaBuffers    = request.vIMetaBuffers;
                        err = pPipelineModel->submitRequest(appRequest);
                        if  ( OK != err ) {
                            MY_LOGE("err:%d(%s)", -err, ::strerror(-err));
                            return err;
                        }
                        return err;
                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineCallbackT Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    virtual auto    updateFrame(
                        MUINT32 const requestNo,
                        MINTPTR const userId,
                        ssize_t const nOutMetaLeft,
                        android::Vector<android::sp<IMetaStreamBuffer>> vOutMeta
                    ) -> MVOID override
                    {
                        android::sp<IAppCallback> pCallback = mAppCallback.promote();
                        if  ( pCallback == nullptr ) {
                            MY_LOGE("fail to promote callback");
                            return;
                        }
                        pCallback->onFrameUpdated(requestNo, userId, nOutMetaLeft, vOutMeta);
                    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineModelManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual auto    submitRequest(
                        android::Vector<AppRequest>const& requests,
                        uint32_t& numRequestProcessed
                        ) -> int override
                    {
                        int err = 0;
                        numRequestProcessed = 0;
                        for (size_t i = 0; i < requests.size(); i++, numRequestProcessed++) {
                            err = submitOneRequest(requests[i]);
                            if  ( OK != err ) {
                                MY_LOGE("%u/%zu requests submitted sucessfully - err:%d(%s)",
                                    numRequestProcessed, requests.size(), -err, ::strerror(-err));
                                return err;
                            }
                        }
                        return err;
                    }

    virtual auto    beginFlush() -> int override
                    {
                        auto pPipelineModel = getPipeline();
                        if  ( pPipelineModel != nullptr ) {
                            return pPipelineModel->beginFlush();
                        }
                        return 0;
                    }

    virtual auto    endFlush() -> void override
                    {
                        auto pPipelineModel = getPipeline();
                        if  ( pPipelineModel != nullptr ) {
                            pPipelineModel->endFlush();
                        }
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
typedef PipelineModelManagerTemplate<IPipelineModelMgr::IAppCallback, IPipelineModelMgr> PipelineModelManagerImplT;
class PipelineModelManagerImpl
    : public PipelineModelManagerImplT
{
public:     ////    Definitions.
    typedef IPipelineModelMgr   IModelT;

protected:  ////    Data Members.
    mutable android::Mutex      mPipelineLock;
    android::sp<IModelT>        mPipelineModel = nullptr;
    CpuCtrl*                    mpCpuCtrl = nullptr;

public:     ////    Operations.
                    PipelineModelManagerImpl(AppCreation const& params)
                        : PipelineModelManagerImplT(params)
                    {
                    }

    virtual auto    getPipeline() const -> android::sp<IModelT> override
                    {
                        Mutex::Autolock _l(mPipelineLock);
                        return mPipelineModel;
                    }

public:     ////    IPipelineModelManager Interfaces.
    virtual auto    configure(AppConfiguration const& params) -> int override
                    {
                        int err = 0;

                        Mutex::Autolock _l(mPipelineLock);

                        mPipelineModel = nullptr;
                        mPipelineModel = IModelT::create(mId, this);
                        if ( mPipelineModel == 0 ) {
                            MY_LOGE("IPipelineModel::create");
                            return NO_INIT;
                        }
                        //
                        IModelT::ConfigurePipelineMgr::Params configParams;
                        configParams.openId                 = mId;
                        configParams.pvImageStreams         = &params.vImageStreams;
                        configParams.pvMetaStreams          = &params.vMetaStreams;
                        configParams.pvMinFrameDuration     = &params.vMinFrameDuration;
                        configParams.pvStallFrameDuration   = &params.vStallFrameDuration;
//                        configParams.pPipelineModel         = mPipelineModel;
                       configParams.operation_mode         = params.operationMode;
                        //
                        err = mPipelineModel->configurePipeline(configParams);
                        if  ( OK != err ) {
                            MY_LOGE("fail to configure pipeline");
                            return err;
                        }
                        //
                        return err;
                    }

    virtual auto    dumpState(int fd __unused, const std::vector<std::string>& options __unused) -> void override
                    {
                    }

protected:  ////    RefBase Interfaces.

    virtual void    onFirstRef() override
                    {
                        mpCpuCtrl = CpuCtrl::createInstance();
                        if  (mpCpuCtrl) {
                            MY_LOGD("start CPU boost");
                            mpCpuCtrl->init();
                            CpuCtrl::CPU_CTRL_INFO cpuInfo;
                            cpuInfo.bigCore = 4;
                            cpuInfo.bigFreq = 1950000;
                            cpuInfo.littleCore = 4;
                            cpuInfo.littleFreq = 1144000;
                            cpuInfo.timeout = 1;
                            mpCpuCtrl->enable(cpuInfo);
                        }

                        onInit();
                    }

    virtual void    onLastStrongRef(const void* id __unused) override
                    {
                        onUninit();

                        if  (mpCpuCtrl) {
                            mpCpuCtrl->uninit();
                            mpCpuCtrl->destroyInstance();
                            mpCpuCtrl = nullptr;
                        }
                    }

};


/******************************************************************************
 *
 ******************************************************************************/
auto
IPipelineModelManager::
create(AppCreation const& params) -> android::sp<IPipelineModelManager>
{
    android::sp<PipelineModelManagerImpl> p = new PipelineModelManagerImpl(params);
    if  ( p == nullptr ) {
        CAM_LOGE("Fail to new PipelineModelManagerImpl");
        return nullptr;
    }

    return p;
}

