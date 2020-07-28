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
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_CONTEXT_INODECONFIGBASE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_CONTEXT_INODECONFIGBASE_H_
//
#include <utils/RefBase.h>
#include <utils/StrongPointer.h>
#include <utils/String8.h>
//
#include <mtkcam/pipeline/pipeline/PipelineContext.h>
#include "StereoBasicParameters.h"
//
#include <utils/KeyedVector.h>
#include <map>
#include <algorithm>
//
using namespace android;
using namespace NSCam::v3::NSPipelineContext;
//
namespace NSCam
{
class BasicParameters;
namespace v3
{
class MetaStreamManager;
class ImageStreamManager;
//
class INodeConfigBase
    : public virtual RefBase
{
    public:
        INodeConfigBase(
            MINT32 openId,
            NodeId_T nodeId,
            char const* nodeName)
                : mOpenId(openId),
                  mNodeId(nodeId),
                  mNodeName(nodeName)
                  {}
        virtual ~INodeConfigBase(){}
        virtual void                                configNode(
                                                        MetaStreamManager* metaManager,
                                                        ImageStreamManager* imageManager,
                                                        NSCam::StereoBasicParameters* userParams,
                                                        PipelineContext* pipeConext) = 0;
        virtual sp<INodeActor>                      getNode() = 0;
        virtual void                                dump() = 0;
        virtual void                                destroy() = 0;
        // [Deprecated]
        virtual void                                setInOutStream() {};
        //
        virtual IOMapSet                           getImageIOMaps()    final { return mvImageIOSet; }
        virtual IOMapSet                           getMetaIOMaps()     final { return mvMetaIOSet; }
        virtual StreamSet                          getInStreamSet()    final { return mvInStreamSet; }
        virtual StreamSet                          getOutStreamSet()   final { return mvOutStreamSet; }
        virtual DefaultKeyedVector<StreamId_T, MINT32 >
                                                    getStreamUsageSet() final { return mvStreamUsageSet; }
        virtual NodeId_T                            getNodeId()         final {return mNodeId;}

        // Utility function to add stream.
        virtual void    addStreamBegin(MBOOL isMetaStream) final
        {
            if(isMetaStream)
            {
                mvMetaIOMap.vIn.clear();
                mvMetaIOMap.vOut.clear();
                mvMetaIOSet.clear();
            }
            else
            {
                mvImageIOMap.vIn.clear();
                mvImageIOMap.vOut.clear();
                mvImageIOSet.clear();
            }
        }
        virtual void    addStreamBegin_keepIOSet(MBOOL isMetaStream) final
        {
            if(isMetaStream)
            {
                mvMetaIOMap.vIn.clear();
                mvMetaIOMap.vOut.clear();
            }
            else
            {
                mvImageIOMap.vIn.clear();
                mvImageIOMap.vOut.clear();
            }
        }
        virtual void    addStreamEnd(MBOOL isMetaStream) final
        {
            if(isMetaStream)
            {
                if(mvMetaIOMap.vIn.size()>0 || mvMetaIOMap.vOut.size()>0)
                    mvMetaIOSet.add(mvMetaIOMap);
            }
            else
            {
                if(mvImageIOMap.vIn.size()>0 || mvImageIOMap.vOut.size()>0)
                    mvImageIOSet.add(mvImageIOMap);
            }
        }
        virtual void    addStream(sp<IMetaStreamInfo>& streamInfo, MBOOL bInType) final
                {
                    if(streamInfo != 0)
                    {
                        if(bInType)
                        {
                            mvInStreamSet.add(streamInfo->getStreamId());
                            mvMetaIOMap.addIn(streamInfo->getStreamId());
                        }
                        else
                        {
                            mvOutStreamSet.add(streamInfo->getStreamId());
                            mvMetaIOMap.addOut(streamInfo->getStreamId());
                        }
                    }
                }
        virtual void    addStream(sp<IImageStreamInfo>& streamInfo, MBOOL bInType) final
                {
                    if(streamInfo != 0)
                    {
                        if(bInType)
                        {
                            mvInStreamSet.add(streamInfo->getStreamId());
                            mvImageIOMap.addIn(streamInfo->getStreamId());
                        }
                        else
                        {
                            mvOutStreamSet.add(streamInfo->getStreamId());
                            mvImageIOMap.addOut(streamInfo->getStreamId());
                        }
                        mvStreamUsageSet.add(streamInfo->getStreamId(), streamInfo->getUsageForAllocator());
                    }
                }
        virtual MBOOL    isDstStreamId(StreamId_T streamId)
            {
                if(mvOutStreamSet.empty())
                {
                    return MFALSE;
                }
                else
                {
                    if(std::find(mvOutStreamSet.begin(), mvOutStreamSet.end(), streamId) != mvOutStreamSet.end()) {
                        return MTRUE;
                    } else {
                        return MFALSE;
                    }
                }
            }

    protected:
        MINT32                          mOpenId = -1;
        NodeId_T                        mNodeId = 0;
        String8                         mNodeName;
        StreamSet                       mvInStreamSet;
        StreamSet                       mvOutStreamSet;
        //
        IOMap                           mvImageIOMap;
        IOMapSet                        mvImageIOSet;
        IOMap                           mvMetaIOMap;
        IOMapSet                        mvMetaIOSet;
        //
        DefaultKeyedVector<StreamId_T, MINT32 >   mvStreamUsageSet;
};
};
};
#endif //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_CONTEXT_INODECONFIGBASE_H_