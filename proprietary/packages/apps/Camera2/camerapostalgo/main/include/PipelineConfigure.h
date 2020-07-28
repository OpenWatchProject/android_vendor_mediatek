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

#ifndef MAIN_CORE_INCLUDE_PIPELINECONFIGURE_H_
#define MAIN_CORE_INCLUDE_PIPELINECONFIGURE_H_

#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <utils/Errors.h>
#include <binder/IBinder.h>
#include <PABuffer.h>
#include <gui/Surface.h>
#include <utils/def/UITypes.h>

#include <campostalgo/pipe/PipeInfo.h>
#include <PABuffer.h>
#include <streammgr/ImageStreamInfo.h>

using android::sp;
using android::Vector;
using android::KeyedVector;
using android::IBinder;
using android::status_t;
using ::com::mediatek::campostalgo::AlgoType;
using NSCam::MSize;
using NSPA::PABufferItem;


namespace NSPA {

class PipelineConfigure : virtual public android::RefBase{
public:
    PipelineConfigure();
    void destroy();
    virtual ~PipelineConfigure();

protected:
    mutable android::Mutex mPipeConfigLock;
    std::vector<AlgoType> mStreamAlgos;
    KeyedVector<sp<IBinder>, int32_t> mAppStreamMap;
    KeyedVector<int32_t, sp<ImageStreamInfo>> mAppStreamInfoMap;
    KeyedVector<sp<IBinder>, int32_t> mHalStreamMap;
    KeyedVector<int32_t, sp<android::Surface>> mHalStreams;
    KeyedVector<int32_t, sp<PABufferItem>> mPABuffers;
    friend class StreamManager;
    friend class FeatureProcessor;
};

} /* namespace NSPA */

#endif /* MAIN_CORE_INCLUDE_PIPELINECONFIGURE_H_ */
