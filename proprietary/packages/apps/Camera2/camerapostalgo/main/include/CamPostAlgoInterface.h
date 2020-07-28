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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef MAIN_FEATUREPIPE_CAMPOSTALGOINTERFACE_H_
#define MAIN_FEATUREPIPE_CAMPOSTALGOINTERFACE_H_

#include<utils/StrongPointer.h>
#include<utils/Errors.h>
#include<utils/RefBase.h>
#include<binder/Status.h>
#include<vector>
#include <chrono>

#include<com/mediatek/campostalgo/BnCamPostAlgoInterface.h>
#include<campostalgo/Stream.h>
#include <processor/PA_FeatureProcessor.h>
#include <IStreamManager.h>
#include <PABuffer.h>
#include "processor/ParamConvert.h"

using android::status_t;

#define LOG_TAG "CamPostAlgoInterface"

using namespace com::mediatek::campostalgo::NSFeaturePipe;
using ::android::binder::Status;

namespace com {

namespace mediatek {

namespace campostalgo {

class CamPostAlgoInterface;

//death recipient
class ClientDeathRecipient: public ::android::IBinder::DeathRecipient {

public:
    ClientDeathRecipient(wp<CamPostAlgoInterface>);
    virtual ~ClientDeathRecipient() override = default;
    virtual void binderDied(const wp<IBinder>& who) override;
private:
    wp<CamPostAlgoInterface> mInterface;
};


class CamPostAlgoInterface: public BnCamPostAlgoInterface,
        public virtual android::RefBase {
public:
    CamPostAlgoInterface(int32_t id);
    virtual ~CamPostAlgoInterface();
    virtual android::binder::Status disconnect() override;
    virtual android::binder::Status flush(int64_t* _aidl_return) override;
    virtual ::android::binder::Status config(
            const ::std::vector<FeatureConfig>& configs,
            const ::android::sp<ICamPostAlgoCallback>& callback,
            FeatureResult* _aidl_return) override;
    virtual Status configParams(int32_t type, const FeatureParam& param)
            override;
    virtual ::android::binder::Status finalizeOutputConfig(
            const std::vector<Stream>& streams) override;
    virtual Status getStat(int32_t* _aidl_return) override;
    virtual status_t initialize();
    NSPA::FeatureProcessor* getFeatureProcessor(StreamType type);
    int32_t getInterfaceId();
    const android::String8 getInterfaceName();

private:
    MBOOL parseConfigParam(const NSPA::ConfigParams &configParam);

    mutable Mutex mInterfaceLock;
    const int mId;
    int32_t mStat;
    android::String8 mName;
    sp<NSPA::IStreamManager> mStreamManager = nullptr;
    sp<ClientDeathRecipient> mClientDeathRecipient = nullptr;
    sp<ICamPostAlgoCallback> mClientCallback = nullptr;
    NSPA::FeatureProcessor mCaptureProcessor { "PA_CaptureProcessor" };
    NSPA::FeatureProcessor mPreviewProcessor { "PA_PreviewProcessor" };
};

}
}
}
#endif /* MAIN_FEATUREPIPE_CAMPOSTALGOINTERFACE_H_ */
