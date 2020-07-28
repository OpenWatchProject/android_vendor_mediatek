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

#ifndef _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_PIPELINEBUILDERBASE_H_
#define _MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_PIPELINEBUILDERBASE_H_
//
#include <utils/RefBase.h>
#include <utils/Vector.h>

#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <utils/String8.h>


using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
//
namespace NSCam {
namespace v1 {
namespace NSLegacyPipeline {

class PipelineBuilderBase
    : public LegacyPipelineBuilder
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
                                    PipelineBuilderBase(
                                        MINT32 const openId,
                                        char const* pipeName,
                                        ConfigParams const & rParam
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LegacyPipelineBuilder Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Interface for setup PipelineContext.
    /**
     * set pipeline's source: sensor or memory
     *
     * @param[in] rParam: sensor parameters if use sensor as source.
     *                    or the the stream info of the source memory
     *
     */
    virtual MERROR                  setSrc(PipelineSensorParam const& rParam) {
                                        mbIsSensor = MTRUE;
                                        mSrc_Sensor = rParam;
                                        return OK;
                                    }
    virtual MERROR                  setSrc(Vector<PipelineImageParam> const& rParam) {
                                        mbIsSensor = MFALSE;
                                        mSrc_Image = rParam;
                                        return OK;
                                    }

    /**
     * set pipeline's destination buffers
     *
     * @param[in] vParams: the stream infos of the requested output streams
     *
     */
    virtual MERROR                  setDst(Vector<PipelineImageParam> const& vParams);

    /**
     * set pipeline's dispatcher
     *
     * @param[in] spDispatcher: the dispatcher for pipeline
     *
     */
    virtual MERROR                  setDispatcher(sp<IDispatcher> const& spDispatcher) {
                                        mpDispatcher = spDispatcher;
                                        return OK;
                                    }

    /**
     * set pipeline's scenario control
     *
     * @param[in] pControl: the scenario control
     *
     */
    virtual MERROR                  setScenarioControl(sp<IScenarioControl> pControl) {
                                        mpControl = pControl;
                                        return OK;
                                    }

    /**
     * update 3A Config
     *
     * @param[in] param: the hal3A Config for P1Node
     *
     */
     virtual MERROR                 updateHal3AConfig(const Hal3AParams &param) {
                                        mConfigParams.hal3AParams = param;
                                        return OK;
                                    }

    /**
     * create the ILegacyPipeline
     *
     * PipelineManager will hold the sp of this ILegacyPipeline.
     *
     */
    virtual sp<ILegacyPipeline>     create()                                          = 0;

protected:

    MINT32                          getOpenId() const { return mOpenId; }
    const char*                     getName() const { return mPipeName.string(); }
    MINT32                          getMode() const { return mConfigParams.mode; }

    MBOOL                           querySrcParam(PipelineSensorParam& rParam) {
                                        if( mbIsSensor )
                                            rParam = mSrc_Sensor;
                                        return mbIsSensor;
                                    }
    MBOOL                           querySrcParam(Vector<PipelineImageParam>& rParam) {
                                        if( ! mbIsSensor )
                                            rParam = mSrc_Image;
                                        return !mbIsSensor;
                                    }
    MBOOL                           queryDispatcher(sp<IDispatcher>& rParam) {
                                        rParam = mpDispatcher;
                                        return ( rParam.get() ? MTRUE : MFALSE);
                                    }
    MBOOL                           queryScenarioControl(sp<IScenarioControl>& rParam) {
                                        rParam = mpControl;
                                        return ( rParam.get() ? MTRUE : MFALSE);
                                    }
    MBOOL                           queryRawDst(Vector<PipelineImageParam>& vParams) {
                                        vParams = mDst_Raw;
                                        return mDst_Raw.size();
                                    }
    MBOOL                           queryYuvDst(Vector<PipelineImageParam>& vParams) {
                                        vParams = mDst_Yuv;
                                        return mDst_Yuv.size();
                                    }
    MBOOL                           queryJpegDst(Vector<PipelineImageParam>& vParams) {
                                        vParams = mDst_Jpeg;
                                        return mDst_Jpeg.size();
                                    }

    MVOID                           update3AConfigMeta(IMetadata *appMeta, IMetadata *halMeta);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    #define HighSpeedVideoFpsBase           (30)
    //
    MINT32                          mLogLevel;
    ConfigParams                    mConfigParams;
    MUINT8                          mFps;
    MUINT8                          mBurstNum;

private:     ////
    MINT32 const                    mOpenId;
    String8 const                   mPipeName;
    //
    MBOOL                           mbIsSensor;
    PipelineSensorParam             mSrc_Sensor;
    Vector<PipelineImageParam>      mSrc_Image;
    sp<IDispatcher>                 mpDispatcher;
    sp<IScenarioControl>            mpControl;
    //
    Vector<PipelineImageParam>      mDst_Raw;
    Vector<PipelineImageParam>      mDst_Yuv;
    Vector<PipelineImageParam>      mDst_Jpeg;
};

};  //namespace NSLegacyPipeline
};  //namespace v1
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_LEGACYPIPELINE_PIPELINEBUILDERBASE_H_
