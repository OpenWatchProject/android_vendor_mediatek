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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_TPNODEPLUGIN_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_TPNODEPLUGIN_H_

#include <mtkcam/utils/std/common.h>
#include <mtkcam/pipeline/hwnode/TPNode.h>

#define MAX_NUM_OF_FRAME    16

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

/******************************************************************************
 *
 ******************************************************************************/
typedef MINT32 PluginId_T;

class TPNodePlugin {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  3rd Party Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    enum PluginType
    {
        BLENDING,
        DEPTH,
    };

    typedef struct
    {
        PluginId_T id;
        PluginType type;
    } PluginProfile;

    typedef struct {
        MINT32          nSensorId;
        IMetadata*      pAppMeta;
        IMetadata*      pHalMeta;
        IImageBuffer*   pFullYuv;
        IImageBuffer*   pResizedYuv;
        IImageBuffer*   pBinningYuv;
    } FrameInput;

    typedef struct {
        IMetadata*      pAppMeta;
        IMetadata*      pHalMeta;
        IImageBuffer*   pFullYuv;
        IImageBuffer*   pDepth;
        IImageBuffer*   pClean;
    } FrameOutput;

    typedef struct {
        MUINT32             uRequestNo;
        MBOOL               bIsLastPartial;
        FrameInput          in;
        FrameOutput         out;
    } PartialParams;

    typedef struct
    {
        typedef MVOID (*RequestCallback)(MVOID*, MERROR);

        MUINT32             uRequestNo;
        MUINT32             uInCount;
        FrameInput          in[MAX_NUM_OF_FRAME];
        FrameOutput         out;
        MVOID*              pToken;
        RequestCallback     fnCallback;

    } RequestParams;

    virtual MVOID init() = 0;

    virtual MVOID uninit() = 0;

    virtual PluginProfile& profile() = 0;

    /*
     * Invoked when all pipeline frames of a capture are received.
     * must invoke reponse() after the request has done.
     *
     * @param[in] RequestParams:
     *            the input buffers are from pipeline frames' stream buffers
     */
    virtual MVOID process(RequestParams const&) = 0;

    /**
     * Invoked when a pipeline frame queues.
     * Will be invoked multiple times before process() invokes if a request is multi-frame processing.
     *
     * User could implement this to get capture buffers earlier.
     *
     * @param[in] PartialParams:
     *            the input buffers are from pipeline frame's stream buffers
     *            the output buffers exist if it's the last part
     */
    virtual MVOID onFrameQueue(PartialParams const&) {};

    MVOID response(RequestParams const& params, MERROR result) {
        params.fnCallback(params.pToken, result);
    }

    virtual ~TPNodePlugin() {};

};

/******************************************************************************
 *
 ******************************************************************************/
class PluginRegistry {

public:
    typedef TPNodePlugin* (*Creator)();
    typedef android::KeyedVector<PluginId_T, Creator> CreatorRegistry;

    static CreatorRegistry& registry() {
        static CreatorRegistry* gRegistry = new CreatorRegistry();
        return *gRegistry;
    }

    static void addCreator(PluginId_T id, Creator fnCreator) {
        CreatorRegistry& reg = registry();
        if (reg.indexOfKey(id) >= 0) {
            CAM_LOGE("[TPNode] 3rd party plugin(%d) was already registered.", id);
            return;
        }

        reg.add(id, fnCreator);
    }

    static bool exists(PluginId_T id) {
        CreatorRegistry& reg = registry();
        return (reg.indexOfKey(id) >= 0);
    }

    static TPNodePlugin* createInstance(PluginId_T id) {
        CreatorRegistry& reg = registry();

        if (reg.indexOfKey(id) >= 0) {
            TPNodePlugin* pPlugin = reg.valueFor(id)();
            pPlugin->profile().id = id;
            return pPlugin;
        }

        return NULL;
    }
};

class PluginRegisterer {
public:
    PluginRegisterer(PluginId_T id, PluginRegistry::Creator fnCreator) {
        CAM_LOGD("[TPNode] 3rd party plugin(%d) is registered", id);
        PluginRegistry::addCreator(id, fnCreator);
    }
};

#define REGISTER_TPNODE_PLUGIN(id, type)                                            \
  TPNodePlugin* _Create_TPNode_Plugin_##type()                                      \
  {                                                                                 \
      return new type();                                                            \
  };                                                                                \
  static PluginRegisterer gTPNode_Plugin__##type(id, _Create_TPNode_Plugin_##type);


};  //namespace v3
};  //namespace NSCam
#endif //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_TPNODEPLUGIN_H_
