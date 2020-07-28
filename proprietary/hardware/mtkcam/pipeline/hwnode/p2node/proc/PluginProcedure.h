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
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_PLUGINPROCEDURE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2NODE_PLUGINPROCEDURE_H_

#define PROCESSOR_NAME_PLUGIN ("PG")

#include "../P2Common.h"
#include "../FrameUtils.h"
#include "../Processor.h"
#include "plugin/PostProcessing.h"
#include <map>

class PluginProcedure : virtual public android::RefBase {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Processor Creator
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct CreateParams {
        MUINT32 uOpenId;                 // [in]
        MBOOL bEnableLog;                // [in]
        MBOOL bRawDomain;                // [in]
        PostProcessing *pPostProcessing; // [out]
    };

    template<typename T>
    static sp<Processor> createProcessor(CreateParams &params);

    ~PluginProcedure();

    PluginProcedure(CreateParams const &params);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Frame Parameter
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    struct FrameParams {
        sp<BufferHandle> pInHandle;
        sp<BufferHandle> pOutHandle;
        sp<MetaHandle> pInHalMeta;
        sp<MetaHandle> pInAppMeta;
        MINT32 nIso;
        MUINT32 uFrameNo;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Processor Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static const MBOOL isAsync = MFALSE;

    MERROR onPluginPullParams(
            sp<Request> pRequest,
            FrameParams &params
    );

    MERROR onPluginExecute(
            sp<Request> const pRequest,
            FrameParams const &params
    );

    MERROR onPluginFinish(
            FrameParams const &params,
            MBOOL const success
    );

    MVOID onPluginFlush() { };

    MVOID onPluginNotify(
            __attribute__((unused)) MUINT32 const event,
            __attribute__((unused)) MINTPTR const arg1,
            __attribute__((unused)) MINTPTR const arg2,
            __attribute__((unused)) MINTPTR const arg3
    ) { };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Procedure Member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MUINT32 const muOpenId;
    MBOOL const mbEnableLog;
    MBOOL const mbRawDomain;
    PostProcessing *mpPostProcessing;
    MUINT32 muDumpBuffer;
};

DECLARE_PROC_TRAIT(Plugin);


template<typename T>
sp<Processor>
PluginProcedure::createProcessor(PluginProcedure::CreateParams &params) {
    PostProcessing *pPostProcessing = new T(params.uOpenId);
    if (pPostProcessing == NULL) {
        MY_LOGE("create plugin processor failed");
        return NULL;
    }
    params.pPostProcessing = pPostProcessing;
    return new ProcessorBase<PluginProcedure>(
            params.uOpenId, params, PROCESSOR_NAME_PLUGIN);
}


class PluginRegistry {

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Registry structure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    enum Domain {
        RawDomain,
        YuvDomain
    };
    typedef sp<Processor> (*Creator)(PluginProcedure::CreateParams&);
    typedef map<Domain, Creator> CreatorRegistry;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Static function
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static CreatorRegistry& registry() {
        static CreatorRegistry* gRegistry = new CreatorRegistry();
        return *gRegistry;
    }

    static void addCreator(Domain domain, Creator creator) {
        CreatorRegistry& reg = registry();
        if (reg.count(domain) != 0) {
            MY_LOGE("The domain[%d] was already registered.", domain);
            return;
        }
        reg[domain] = creator;
    }

    static sp<Processor> createProcessor(
            Domain domain,
            PluginProcedure::CreateParams &params)
    {
        CreatorRegistry& reg = registry();
        if (reg.count(domain) == 1) {
            return reg[domain](params);
        }
        return NULL;
    }
};

class PluginRegisterer {
public:
    PluginRegisterer(PluginRegistry::Domain domain,
                     char const* type,
                     sp<Processor> (*creator)(PluginProcedure::CreateParams&)) {
        MY_LOGD("register a plugin. type:%s, domain:%d",type ,domain);
        PluginRegistry::addCreator(domain, creator);
    }
};

#define REGISTER_POSTPROCESSING(domain, type)                   \
  sp<Processor> _CreatePluginProcessor_##type(PluginProcedure::CreateParams &params)  \
  {                                                             \
      return PluginProcedure::createProcessor<type>(params);    \
  };                                                            \
  static PluginRegisterer gPluginCreator_##type(PluginRegistry::domain##Domain, #type,_CreatePluginProcessor_##type);

#endif
