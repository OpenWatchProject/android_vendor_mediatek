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
#ifndef __IMFLLGYRO_H__
#define __IMFLLGYRO_H__

#include "MfllDefs.h"
#include "MfllTypes.h"
#include "IMfllNvram.h"
#include "IMfllCore.h"

#include <utils/RefBase.h> // android::RefBase

#include <deque> // std::deque
#include <vector> // std::vector
#include <cstdint> // int64_t, intptr_t

// describes that how many gyro info we need at a frame
#define MFLL_GYRO_NUMBER_PER_FRAME      3

// describes the interval that we ask gyro module
#define MFLL_GYRO_DEFAULT_INTERVAL_MS   10

using android::sp;
using std::vector;
using std::deque;

namespace mfll {
class IMfllGyro : public android::RefBase {
/* prototypes */
public:
    struct MfllGyroInfo {
        // timestamp "without" deep sleep time
        int64_t timestamp;

        // vectors
        struct Vector {
            std::vector<float> __data;
            inline float& x() { return __data[0]; }
            inline float& y() { return __data[1]; }
            inline float& z() { return __data[2]; }
            inline const float& x() const { return __data[0]; }
            inline const float& y() const { return __data[1]; }
            inline const float& z() const { return __data[2]; }
            Vector()
            {
                __data.resize(3, 0.0f);
            }
        }
        vector;

        MfllGyroInfo() noexcept : timestamp(0) {}
        MfllGyroInfo(const MfllGyroInfo& other) = default;
        MfllGyroInfo(MfllGyroInfo&& other) = default;
        MfllGyroInfo& operator=(const MfllGyroInfo&) = default;
        MfllGyroInfo& operator=(MfllGyroInfo&&) = default;
    };

/* interfaces */
public:
    /**
     *  To init MfllBSS, and MfllBSS needs a NVRAM provider
     *  @param nvramProvider    Instance of IMfllNvram
     *  @return                 Returns MfllErr_Ok if init ok
     */
    virtual enum MfllErr init(sp<IMfllNvram>& nvramProvider) = 0;

    /**
     *  To start gyro module, update every N ms
     */
    virtual enum MfllErr start(void) = 0;

    /**
     *  To stop gyro module, this method will be automatically invoked while
     *  destroying
     */
    virtual enum MfllErr stop(void) = 0;

    /**
     *  To copy the gyro info in range [ts_start, ts_end]
     */
    virtual std::deque<MfllGyroInfo> getGyroInfo(
            const int64_t& ts_start,
            const int64_t& ts_end) const = 0;

    /**
     *  To take away the gyro info in range [ts_start, ts_end]
     *  @param ts_start     timestamp from
     *  @param ts_end       timestamp until
     *  @param clearInvalidated if clear timestamp < ts_start elements
     */
    virtual std::deque<MfllGyroInfo> takeGyroInfo(
            const int64_t& ts_start,
            const int64_t& ts_end,
            bool clearInvalidated = true
            ) = 0;

    /**
     *  Take away MfllGyroInfo queue, after this method has been invoked, all the
     *  data in IMfllGyro's data queue will be empty
     */
    virtual std::deque<MfllGyroInfo> takeGyroInfoQueue(void) = 0;

    /**
     *  External method for future usage
     */
    virtual enum MfllErr sendCommand(const std::string& cmd, const std::deque<void*>& dataset) = 0;


public:
    static IMfllGyro* createInstance();
    void destroyInstance();


protected:
    virtual ~IMfllGyro(){};
};
}; /* namespace mfll */

#endif//__IMFLLGYRO_H__
