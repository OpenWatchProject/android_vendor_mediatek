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
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
//
#include "DebugNode.h"
//
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "DebugNode"
//
#include <PipeLog.h>
#include <string>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
//
using namespace std;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace StereoHAL;
/*******************************************************************************
 *
 ********************************************************************************/
DebugNode::
DebugNode(const char *name,
    Graph_T *graph,
    MINT32 openId,
    MINT32 scenario)
    : BokehPipeNode(name, graph)
    , miOpenId(openId)
    , miScenario(scenario)
{
    this->addWaitQueue(&mRequests);
    if(miScenario == StereoHAL::ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
    {
        this->addWaitQueue(&mWDMABuffer);
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DebugNode::
onData(
    DataID id,
    SmartImageBuffer &data)
{
    FUNC_START;
    TRACE_FUNC_ENTER();
    //
    MBOOL ret;
    switch(id)
    {
        case P2B_MDP_WDMA:
            mWDMABuffer.enque(data);
        printf("\nenque mWDMASize(%d)\n", mWDMABuffer.size());
            ret = MTRUE;
        break;
        default:
        ret = MFALSE;
        break;
    }

    TRACE_FUNC_EXIT();
    FUNC_END;
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DebugNode::
onData(
    DataID id,
    EffectRequestPtr &request)
{
    FUNC_START;
    TRACE_FUNC_ENTER();
    //
    MBOOL ret;
    switch(id)
    {
        case P2B_MDP_REQUEST:
            mRequests.enque(request);
            printf("\nenque P2B_MDP_REQUEST mRequestSize(%d) RequestID(%d)\n", mRequests.size(), request->getRequestNo());
            ret = MTRUE;
        break;
        case P2B_OUT:
            mRequests.enque(request);
            printf("\nenque P2B_OUT mRequestSize(%d) RequestID(%d)\n", mRequests.size(), request->getRequestNo());
            ret = MTRUE;
        break;
        default:
        ret = MFALSE;
        break;
    }

    TRACE_FUNC_EXIT();
    FUNC_END;
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DebugNode::
onInit()
{
    TRACE_FUNC_ENTER();
    FUNC_START;
    BokehPipeNode::onInit();
    FUNC_END;
    TRACE_FUNC_EXIT();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DebugNode::
onUninit()
{
    FUNC_START;
    cleanUp();
    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DebugNode::
cleanUp()
{
    FUNC_START;
    FUNC_END;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DebugNode::
onThreadStart()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DebugNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DebugNode::
onThreadLoop()
{
    FUNC_START;
    EffectRequestPtr request = nullptr;
    SmartImageBuffer smbuffer = nullptr;
    //
    if( !waitAllQueue() )
    {
        // mJobs.abort() called
        MY_LOGE("waitAllQueue failed");
        return MFALSE;
    }
    //
    if( !mRequests.deque(request) )
    {
        MY_LOGE("mRequests.deque() failed");
        return MFALSE;
    }
    else
    {
        if(miScenario == StereoHAL::ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
        {
            printf("\ndeque mRequestSize(%d) RequestID(%d)\n", mRequests.size(), request->getRequestNo());
            sp<EffectParameter> param = request->getRequestParameter();
            MINT32 scenario = param->getInt(VSDOF_FRAME_SCENARIO);
            if(scenario == StereoHAL::ENUM_STEREO_SCENARIO::eSTEREO_SCENARIO_CAPTURE)
            {
                printf("mWDMABuffer deque +\n");
                if( !mWDMABuffer.deque(smbuffer) )
                {
                    MY_LOGE("mWDMABuffer.deque() failed");
                    return MFALSE;
                }
                makePath("/sdcard/vsdof/bokeh_ut/result/p2b", 0660);
                std::string saveFileName = std::string("/sdcard/vsdof/bokeh_ut/result/p2b/WDMA_CAP_")+
                                    std::to_string(request->getRequestNo())+
                                    std::string(".yuv");
                smbuffer->mImageBuffer->saveToFile(saveFileName.c_str());
                smbuffer = nullptr;
                printf("mWDMABuffer deque -\n");
            }
        }
    }
    //
    if(request->mpOnRequestProcessed!=nullptr)
    {
        request->mpOnRequestProcessed(
            request->mpTag,
            String8("Done"),
            request);
    }
    //
    FUNC_END;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
/*MBOOL
DebugNode::
getFrameInfoFromRequest(
    EffectRequestPtr request,
    FrameInfoPtr& frame,
    MINT32 bufType,
    MBOOL isInputPort)
{
    ssize_t keyIndex = -1;
    if(isInputPort)
    {
        keyIndex = request->vInputFrameInfo.indexOfKey(bufType);
        if(keyIndex>=0)
        {
            frame = request->vInputFrameInfo.valueAt(keyIndex);
            return MTRUE;
        }
        else
        {
            return MFALSE;
        }
    }
    else
    {
        keyIndex = request->vOutputFrameInfo.indexOfKey(bufType);
        if(keyIndex>=0)
        {
            frame = request->vOutputFrameInfo.valueAt(keyIndex);
            return MTRUE;
        }
        else
        {
            return MFALSE;
        }
    }
    return MFALSE;
}*/
/*******************************************************************************
 *
 ********************************************************************************/
/*MBOOL
DebugNode::
dumpRequestParams(MINT32 requestId, const sp<EffectParameter> params)
{
    MINT32 scenario = reqst_para->get(VSDOF_FRAME_SCENARIO, mParams.scenario);
    MINT32 nr3dFlag = reqst_para->get(VSDOF_FRAME_3DNR_FLAG, mParams.is3DNREnable);
    MINT32 gmvx = reqst_para->get(VSDOF_FRAME_GMV_X, mParams.gmv_x);
    MINT32 gmvy = reqst_para->get(VSDOF_FRAME_GMV_Y, mParams.gmv_y);
    MINT32 afFlag = reqst_para->get(VSDOF_FRAME_AF_TRIGGER_FLAG, mParams.isAFTrigger);
    MINT32 afx = reqst_para->get(VSDOF_FRAME_AF_POINT_X, mParams.ptAF_x);
    MINT32 afy = reqst_para->get(VSDOF_FRAME_AF_POINT_Y, mParams.ptAF_y);
    MINT32 bokehLevel = reqst_para->get(VSDOF_FRAME_BOKEH_LEVEL, mParams.bokehLevel);
    MINT32 gsensor = reqst_para->get(VSDOF_FRAME_G_SENSOR_ORIENTATION, mParams.iGSensor);
    if(miPipeLogEnable)
    {
        MY_LOGD("requestId(%d), scenario(%d), nr3dFlag(%d), GMV(%d:%d), afFlag(%d), af(%d:%d), bokehLevel(%d), gsensor(%d)",
                requestId,
                scenario,
                nr3dFlag,
                gmvx,
                gmvy,
                afFlag,
                afx,
                afy,
                bokehLevel,
                gsensor);
    }
}*/