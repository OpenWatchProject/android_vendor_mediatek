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

#include "P2_MDPProcessor.h"

#define P2_MDP_THREAD_NAME "p2_mdp"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    MDPProcessor
#define P2_TRACE        TRACE_MDP_PROCESSOR
#include "P2_LogHeader.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_PROC_COMMON);

namespace P2
{

MDPProcessor::MDPProcessor(const std::string &name)
    : Processor(name)
    , mSImager(NULL)
{
}

MDPProcessor::~MDPProcessor()
{
    this->uninit();
}

MBOOL MDPProcessor::onInit(const MUINT32 &unused)
{
    (void)unused;
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_MDP:init()");
    MBOOL ret = MTRUE;
    mSImager = IImageTransform::createInstance();
    if( mSImager == NULL )
    {
        MY_LOGE("OOM: cannot create SImager::IImageTransform");
        ret = MFALSE;
    }
    return ret;
}

MVOID MDPProcessor::onUninit()
{
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_MDP:uninit()");
    if( mSImager )
    {
        mSImager->destroyInstance();
        mSImager = NULL;
    }
}

MVOID MDPProcessor::onThreadStart()
{
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_MDP:threadStart()");
}

MVOID MDPProcessor::onThreadStop()
{
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_MDP:threadStop()");
}

MBOOL MDPProcessor::onConfig(const MUINT32 &unused)
{
    (void)unused;
    return MTRUE;
}

#define MAX_PORT_NUM 2

MBOOL MDPProcessor::onEnque(const MDPParam &param)
{
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_MDP:enque()");
    MBOOL ret = MTRUE;
    if( param.mDst.size() && isValid(param.mSrc) )
    {
        ILog log = param.mRequest->mLog;
        IImageBuffer *src = param.mSrc->getIImageBufferPtr();
        unsigned i = 0, n = param.mDst.size();
        while( i < n )
        {
            IImageBuffer* dst[MAX_PORT_NUM] = { NULL, NULL };
            MRect crop[MAX_PORT_NUM];
            MUINT32 tran[MAX_PORT_NUM];
            unsigned out = 0;
            while( i < n && out < MAX_PORT_NUM )
            {
                if( isValid(param.mDst[i]) )
                {
                    dst[out] = param.mDst[i]->getIImageBufferPtr();
                    crop[out] = MRect();
                    tran[out] = param.mDst[i]->getTransform();
                    ++out;
                }
                ++i;
            }

            if( out == 1 )
            {
                ret = mSImager->execute(src, dst[0], dst[1], crop[0], tran[0], 0xFFFFFFFF);
            }
            else if (out == 2 )
            {
                ret = mSImager->execute(src, dst[0], dst[1], crop[0], crop[1], tran[0], tran[1], 0xFFFFFFFF);
            }
        }
        for( i = 0; i < n; ++i)
        {
            if( isValid(param.mDst[i]) )
            {
                param.mDst[i]->updateResult(MTRUE);
            }
        }
    }
    return ret;
}

MVOID MDPProcessor::onNotifyFlush()
{
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_MDP:notifyFlush()");
}

MVOID MDPProcessor::onWaitFlush()
{
    P2_CAM_TRACE_NAME(TRACE_ADVANCED, "P2_MDP:waitFlush()");
}

} // namespace P2
