/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef __MFLLGYRO_H__
#define __MFLLGYRO_H__

#include <mtkcam3/feature/mfnr/IMfllGyro.h>
#include <mtkcam3/feature/mfnr/IMfllCore.h>

// AOSP
#include <android/sensor.h> // dependecy of SensorListener.h

// MTKCAM
#include <mtkcam/def/BuiltinTypes.h> // dependecy of SensorListener.h
#include <mtkcam/utils/sys/SensorListener.h>

// STL
#include <memory> // std::unique_ptr
#include <functional> // std::function
#include <deque> // std::deque
#include <mutex> // std::mutex
#include <condition_variable> // std::condition_variable

using android::sp;

namespace mfll {

class MfllGyroImp;

class MfllGyro: public IMfllGyro
{
/* implementation from IMfllGyro */
public:
    enum MfllErr init(sp<IMfllNvram>& nvramProvider);

    enum MfllErr start(void);

    enum MfllErr stop(void);

    std::deque<MfllGyroInfo> getGyroInfo(
            const int64_t& ts_start,
            const int64_t& ts_end) const;

    std::deque<MfllGyroInfo> takeGyroInfo(
            const int64_t& ts_start,
            const int64_t& ts_end,
            bool clearInvalidated = true
            );

    std::deque<MfllGyroInfo> takeGyroInfoQueue();

    enum MfllErr sendCommand(
            const std::string& cmd,
            const std::deque<void*>& dataset
            );


/* attributes */
private:
    std::shared_ptr<MfllGyroImp> m_gyroImpl;


/* constructor & destructor */
public:
    MfllGyro();
    virtual ~MfllGyro();
}; // class MfllGyro
}; // namespace mfll
#endif//__MFLLGYRO_H__
