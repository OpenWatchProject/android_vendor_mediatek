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

#include "RSCTuningStream.h"

#include "DebugControl.h"
#define PIPE_CLASS_TAG "RSCTuningStream"
#define PIPE_TRACE TRACE_RSC_TUNING_STREAM
#include <featurePipe/core/include/PipeLog.h>
#include <featurePipe/core/include/DebugUtil.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {

#define RSC_STREAM_NAME "FeaturePipe_RSC"

struct tuning_mapping{
    const char*   name;
    void*   addr;
};

RSCTuningStream* RSCTuningStream::createInstance(IHalRscPipe* pRSCStream)
{
    return new RSCTuningStream(pRSCStream);
}

RSCTuningStream::RSCTuningStream(IHalRscPipe* pRSCStream) : mpRSCStream(pRSCStream)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

RSCTuningStream::~RSCTuningStream()
{
    TRACE_FUNC_ENTER();
    this->uninit();
    if( mpRSCStream )
    {
        mpRSCStream->destroyInstance(RSC_STREAM_NAME);
        mpRSCStream = NULL;
    }
    TRACE_FUNC_EXIT();
}

MBOOL RSCTuningStream::init()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mpRSCStream )
    {
        ret = mpRSCStream->init();
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RSCTuningStream::uninit()
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MFALSE;
    if( mpRSCStream )
    {
        ret = mpRSCStream->uninit();
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RSCTuningStream::RSCenque(const RSCParam &param)
{
    TRACE_FUNC_ENTER();

    mRSCParam = param;

    for(size_t i = 0; i < mRSCParam.mRSCConfigVec.size(); i++ )
    {
        tuning(mRSCParam.mRSCConfigVec[i]);
    }

    MBOOL ret = MFALSE;
    if( mpRSCStream )
    {
        ret = mpRSCStream->RSCenque(mRSCParam);
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL RSCTuningStream::RSCdeque(RSCParam &, MINT64)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MFALSE;
}

MVOID RSCTuningStream::destroyInstance(char const*)
{
    TRACE_FUNC_ENTER();
    delete this;
    TRACE_FUNC_EXIT();
}

const char* RSCTuningStream::getPipeName() const
{
    return "RSCTuningStream";
}

MVOID RSCTuningStream::tuning(RSCConfig& rscConfig)
{
    TRACE_FUNC_ENTER();

    tuning_mapping tuningLUT[] = {

        //lut
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Horz_Lut_0", &rscConfig.Rsc_Rand_Horz_Lut_0},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Horz_Lut_1", &rscConfig.Rsc_Rand_Horz_Lut_1},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Horz_Lut_2", &rscConfig.Rsc_Rand_Horz_Lut_2},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Horz_Lut_3", &rscConfig.Rsc_Rand_Horz_Lut_3},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Vert_Lut_0", &rscConfig.Rsc_Rand_Vert_Lut_0},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Vert_Lut_1", &rscConfig.Rsc_Rand_Vert_Lut_1},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Vert_Lut_2", &rscConfig.Rsc_Rand_Vert_Lut_2},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Vert_Lut_3", &rscConfig.Rsc_Rand_Vert_Lut_3},
        {"vendor.debug.fpipe.rsc.Rsc_Cand_Num",        &rscConfig.Rsc_Cand_Num},

        //sad
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Th",    &rscConfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Th},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Mode",  &rscConfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Mode},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_En",    &rscConfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_En},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Ctrl_Sad_Gain",                   &rscConfig.Rsc_Sad_Ctrl_Sad_Gain},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Ctrl_Prev_Y_Offset",              &rscConfig.Rsc_Sad_Ctrl_Prev_Y_Offset},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Edge_Gain_Step",                  &rscConfig.Rsc_Sad_Edge_Gain_Step},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Edge_Gain_TH_L",                  &rscConfig.Rsc_Sad_Edge_Gain_TH_L},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Edge_Gain",                       &rscConfig.Rsc_Sad_Edge_Gain},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Crnr_Gain_Step",                  &rscConfig.Rsc_Sad_Crnr_Gain_Step},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Crnr_Gain_TH_L",                  &rscConfig.Rsc_Sad_Crnr_Gain_TH_L},
        {"vendor.debug.fpipe.rsc.Rsc_Sad_Crnr_Gain",                       &rscConfig.Rsc_Sad_Crnr_Gain},

        //still
        {"vendor.debug.fpipe.rsc.Rsc_Still_Strip_Zero_Pnlty_Dis",          &rscConfig.Rsc_Still_Strip_Zero_Pnlty_Dis},
        {"vendor.debug.fpipe.rsc.Rsc_Still_Strip_Blk_Th_En",               &rscConfig.Rsc_Still_Strip_Blk_Th_En},
        {"vendor.debug.fpipe.rsc.Rsc_Still_Strip_Var_Step",                &rscConfig.Rsc_Still_Strip_Var_Step},
        {"vendor.debug.fpipe.rsc.Rsc_Still_Strip_Var_Step_Th_L",           &rscConfig.Rsc_Still_Strip_Var_Step_Th_L},
        {"vendor.debug.fpipe.rsc.Rsc_Still_Strip_Sad_Step",                &rscConfig.Rsc_Still_Strip_Sad_Step},
        {"vendor.debug.fpipe.rsc.Rsc_Still_Strip_Sad_Step_Th_L",           &rscConfig.Rsc_Still_Strip_Sad_Step_Th_L},

        //Rand_Pnlty
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty_Bndry",                    &rscConfig.Rsc_Rand_Pnlty_Bndry},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty",                          &rscConfig.Rsc_Rand_Pnlty},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty_Gain",                     &rscConfig.Rsc_Rand_Pnlty_Gain},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty_Edge_Resp_Step",           &rscConfig.Rsc_Rand_Pnlty_Edge_Resp_Step},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty_Edge_Resp_Th_L",           &rscConfig.Rsc_Rand_Pnlty_Edge_Resp_Th_L},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty_Edge_Gain",                &rscConfig.Rsc_Rand_Pnlty_Edge_Gain},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty_Var_Resp_Step",            &rscConfig.Rsc_Rand_Pnlty_Var_Resp_Step},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty_Var_Resp_Th_L",            &rscConfig.Rsc_Rand_Pnlty_Var_Resp_Th_L},
        {"vendor.debug.fpipe.rsc.Rsc_Rand_Pnlty_Avg_Gain",                 &rscConfig.Rsc_Rand_Pnlty_Avg_Gain},
    };

    int lutSize = sizeof(tuningLUT)/sizeof(tuningLUT[0]);
    for( int i = 0; i < lutSize; i++ )
    {
        *(MUINT32*)tuningLUT[i].addr = NSCamFeature::NSFeaturePipe::getPropertyValue(tuningLUT[i].name, *(MUINT32*)tuningLUT[i].addr);
    }

    TRACE_FUNC_EXIT();
}

} // namespace NSCam
