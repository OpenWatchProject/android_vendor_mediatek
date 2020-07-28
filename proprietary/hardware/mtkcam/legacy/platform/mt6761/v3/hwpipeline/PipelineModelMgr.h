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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELMGR_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELMGR_H_
//
#include "MyUtils.h"
//
#include <mtkcam/v3/pipeline/IPipelineModel.h>
#include <mtkcam/v3/hwpipeline/PipelineModelFactory.h>
//#include "PipelineResourceMgr.h"
//
#include <future>
#include <vector>
//
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

using namespace NSCam::v3::NSPipelineFactory;

class PipelineModelMgr
    : public IPipelineBufferSetFrameControl::IAppCallback
    , public IPipelineModelMgr
{

public:  ////                            Operations.
                                    PipelineModelMgr(
                                        MINT32 const openId,
                                        android::wp<IPipelineModelMgr::IAppCallback>const& pAppCallback
                                    );
    virtual                         ~PipelineModelMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
/*
    struct InFlightPipeline
        : public android::RefBase
    {
    public:     ////                    Data Members.
        android::Mutex              mLock;
        android::sp<IPipelineModel> mpPipeline;
        PipelineScene               mSceneMode;
        MUINT32                     mStartReqNo;
        MUINT32                     mEndReqNo;

    public:     ////                    Operations.
                                    InFlightPipeline(
                                        android::sp<IPipelineModel> pPipeline,
                                        PipelineScene iScene,
                                        MUINT32 iStartReqNo
                                    )
                                        : mLock()
                                        , mpPipeline(pPipeline)
                                        , mSceneMode(iScene)
                                        , mStartReqNo(iStartReqNo)
                                        , mEndReqNo(iStartReqNo)
                                    {
                                    }

        MERROR                      updateEndReqNo(MUINT32 iEndReqNo)
                                    {
                                        android::Mutex::Autolock _l(mLock);
                                        mEndReqNo = iEndReqNo;
                                        return 0;
                                    }

        android::sp<IPipelineModel> getPipelineModel()
                                    {
                                        return ( mpPipeline.get() )?
                                                mpPipeline :
                                                NULL ;
                                    }

        PipelineScene               getPipelineScene()
                                    { return mSceneMode; }

        MUINT32                     getStartReqNo()
                                    { return mStartReqNo; }

        MUINT32                     getEndReqNo()
                                    {
                                        android::Mutex::Autolock _l(mLock);
                                        return mEndReqNo;
                                    }

    };

    struct PipelineQueue
        : public android::Vector< android::sp<InFlightPipeline> >
    {
    public:
        typedef android::Vector< android::sp<InFlightPipeline> >
                                                ParentT;

        typedef typename ParentT::value_type    value_type;

    public:     ////                    Data Members.
        MUINT32                     mNumberOfInFlightPipeline;

    public:     ////                    Operations.
                                    PipelineQueue()
                                        : mNumberOfInFlightPipeline(0)
                                    {}

        ssize_t                     addInFlightPipeline(value_type const& p)
                                    {
                                        return ParentT::add(p);
                                    }

        android::sp<InFlightPipeline>
                                    getInFlightPipeline(size_t index)
                                    {
                                        return ( ParentT::size()>=index )?
                                                ParentT::itemAt(index) :
                                                NULL ;
                                    }


        android::sp<InFlightPipeline>
                                    getCurrentPipeline()
                                    { return getInFlightPipeline(0); }

        android::sp<InFlightPipeline>
                                    getNewestPipeline()
                                    {
                                        return ( ParentT::size()>0 )?
                                                ParentT::top() :
                                                NULL ;
                                    }

        android::sp<IPipelineModel> getPipelineModel(
                                        size_t index = 0
                                    ) const
                                    { return ParentT::itemAt(index)->getPipelineModel(); }
    };
*/
    MINT32 const                    mOpenId;
    ConfigurePipeline::Params       mConfigParams;

    // PipelineQueue                   vpPipelineModel;

    android::wp<IPipelineModelMgr::IAppCallback>const
                                    mpAppCallback;

    mutable android::Mutex          mPipelineLock;
    android::Vector<
        android::sp<IPipelineModel>
        >                           mvPipelineDestroying;
    MINT                            mCurrentOperationMode;
	android::sp<IPipelineModel>     mpCurrentPipeline;

    android::sp<IPipelineFrameNumberGenerator>
                                    mpPipelineFrameNumberGenerator;

    std::vector< std::future<MERROR> >
                                    mvFutures;

//    android::sp<PipelineResourceMgr>
//                                    mpPipelineResourceMgr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    static android::sp<IPipelineModelMgr>
                                    create(
                                        android::wp<IPipelineModelMgr::IAppCallback>const& pAppCallback
                                    );

    virtual MERROR                  configurePipeline(
                                        ConfigurePipelineMgr::Params& rParams
                                    );

    virtual char const*             getName() const;

    virtual MINT32                  getOpenId() const;

public:     ////                    Operations.
    /**
     * Submit a request.
     *
     * @param[in] request: App request to submit.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                  submitRequest(IPipelineModelMgr::AppRequest& request);

    /**
     * turn on flush flag as flush begin and do flush
     *
     */
    virtual MERROR                  beginFlush();

    /**
     * turn off flush flag as flush end
     *
     */
    virtual MVOID                   endFlush();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/*private:
    android::sp<IPipelineFrameNumberGenerator>
                                    getPipelineFrameNumberGenerator() const
                                        { return mpPipelineFrameNumberGenerator; };

    MBOOL                           determinePipelineModel(
                                        IPipelineModelMgr::AppRequest& request,
                                        PipelineScene &rScene
                                    );
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl::IAppCallback Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
//    MVOID                           dump();

//    MVOID                           dumpConfigure();

    virtual MVOID                   updateFrame(
                                        MUINT32 const frameNo,
                                        MINTPTR const userId,
                                        ssize_t const nOutMetaLeft,
                                        android::Vector<android::sp<IMetaStreamBuffer> > vOutMeta
                                    );

protected:
    android::sp<IPipelineModel>     getPipeline() const {
                                        android::Mutex::Autolock _l(mPipelineLock);
                                        return mpCurrentPipeline;
                                    }
    //
    //MINT                            evaluatePipelineModel(IPipelineModelMgr::AppRequest& request);
    MINT                            evalPipelineScene(MINT const op_mode) const;
    MBOOL                           isReconfigPipeline(
                                        ConfigurePipeline::Params const& curConfig,
                                        IPipelineModelMgr::AppRequest const& request,
                                        MINT& pipelineScene //new pipeline scene
                                    );
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELMGR_H_

