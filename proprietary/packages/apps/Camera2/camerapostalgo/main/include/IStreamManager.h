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

#ifndef MAIN_CORE_INCLUDE_ISTREAMMANAGER_H_
#define MAIN_CORE_INCLUDE_ISTREAMMANAGER_H_
#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <gui/Surface.h>
#include <binder/Status.h>
#include <utils/Mutex.h>
#include <campostalgo/pipe/PipeInfo.h>
#include <PABuffer.h>
#include <PipelineConfigure.h>
#include <campostalgo/pipe/PipeInfo.h>
#include <com/mediatek/campostalgo/ICamPostAlgoCallback.h>
#include <campostalgo/StreamInfo.h>
#include <campostalgo/FeatureConfig.h>
#include <campostalgo/FeatureResult.h>

#include <processor/PA_Request.h>
#include <utils/metadata/IMetadata.h>

using namespace com::mediatek::campostalgo;
using com::mediatek::campostalgo::FeatureConfig;
using android::KeyedVector;

using android::RefBase;
using android::sp;
using android::status_t;
using android::binder::Status;


using android::Vector;
using com::mediatek::campostalgo::StreamInfo;
using com::mediatek::campostalgo::NSFeaturePipe::IMetadata;

namespace NSPA {


class IStreamManager;

class FeatureConfigParam
{
public:
    typedef enum {
        SET_NONE,
        SET_AGLO,
        SET_PARAMS
    } SET_FLAG;

    FeatureConfigParam();
    FeatureConfigParam(const std::vector<AlgoType>& algos);
    virtual ~FeatureConfigParam(){};

public:
//    P2Info mP2Info;
    MINT mFlags;
    std::vector<AlgoType> mAlgos;
    sp<MetadataPack> mMetaPackPtr;

};

struct FeatureInitParam
{
    StreamType mType;
    int32_t mInterfaceId;
    std::vector<AlgoType> mAlgos;
    sp<IStreamManager> mStreamManager;
};

class IStreamManager: virtual public RefBase {
public :
    static sp<IStreamManager> createInstance(std::string name);

    virtual Status configure(const FeatureConfig& config,
            FeatureResult* result) = 0;

    virtual auto configureStreams(const StreamType type,
            const Vector<sp<android::Surface>>& appSurfaces) -> status_t = 0;
    virtual void addInterfaceCallback(
            const ::android::sp<ICamPostAlgoCallback>& callback) = 0;

/*    virtual void configureAlgos(StreamType type,
            const std::vector<AlgoType>& algos) = 0;*/

    virtual auto configureHalStreams(const StreamType type,
            const Vector<sp<StreamInfo>> & infos) -> status_t = 0;

    virtual auto getConfigure(const StreamType type) -> sp<PipelineConfigure> = 0;

    virtual auto updateResult(sp<PA_Request> pr) -> status_t = 0;
    virtual auto getCommonMetadata(
            const StreamType& type) const -> const IMetadata* = 0;
    /**
     * Destroy the instance.
     */
    virtual auto    destroy() -> void                                       = 0;
/*    virtual auto flush() -> status_t = 0;
    virtual auto updateResult() ->void = 0;*/
protected:
    virtual auto initialize() -> status_t = 0;
};
}

#endif /* MAIN_CORE_INCLUDE_ISTREAMMANAGER_H_ */
