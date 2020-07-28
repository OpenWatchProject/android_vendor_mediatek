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
#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_GYROCOLLECTOR_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_GYROCOLLECTOR_H_

/**
 *  Defines MTKCAM_HAVE_GYROCOLLECTOR_SUPPORT to makes GyroCollector work, or
 *  defines it to 0 to disable.
 */
#define MTKCAM_HAVE_GYROCOLLECTOR_SUPPORT   0

/**
 *  Define this definition to force GyroCollector::GyroInfo be as 4 bytes algined.
 *  In x64 machine, it's better to leave free alignment by compiler. But if there's
 *  memory concern, you can force as 4 bytes alignment.
 */
#define GYROCOLLECTOR_GYROINFO_4_BYTES      0


// STL
#include <cstdint>
#include <vector>
#include <memory>
#include <type_traits>


#if GYROCOLLECTOR_GYROINFO_4_BYTES
#define __GYROINFO_PACK__                   __attribute__((packed, aligned(4)))
#else
#define __GYROINFO_PACK__
#endif


namespace NSCam {
namespace Utils {


// GyroCollector is a class for caller to retrieve gyro information.
//  [ This class is thread-safe ]
class GyroCollector {
//
// Constants
//
public:
    // For better performance, gyro information queue is a limited size queue.
    constexpr static size_t             STACK_SIZE      = 50;

    // Gyro info retrieve interval, in milliseconds.
    constexpr static size_t             INTERVAL        = 33; // ms

    // GyroCollector is an event drived mechanism. Which means if the module
    // is idle for a while, IDLE_TIMEOUT, we will stop listening gyro information
    // anymore. As-Is: the GyroCollector is now triggered by P1 node, which means
    // triggered every frame. The minimum interval is 66 ms, the maximum may
    // be around 100 ms. Please consider it to decide the IDLE_TIMEOUT.
    // Note: It's better to use more than 500ms due to SW overhead.
    constexpr static size_t             IDLE_TIMEOUT    = 1500; // ms

    // We have to limit data collecting interval be greater than 15
    static_assert(GyroCollector::INTERVAL >= 15,
            "The interval of GyroCollector collecting data is supposed to be " \
            "greater than 15, or it may have performance issue");

//
// Types
//
public:
    // GyroInfo is a trivial copyable structure contains gyro information
    struct __GYROINFO_PACK__ GyroInfo
    {
        float       x;
        float       y;
        float       z;
        int64_t     timestamp; // nanosecond, including deep sleep duration
        GyroInfo() noexcept : x(0), y(0), z(0), timestamp(0) {};
        ~GyroInfo() = default;
    };

    static_assert(std::is_trivially_copyable<GyroInfo>::value,
            "GyroCollector::GyroInfo is not a trivial copyable structure, "    \
            "please makes it as a trivial copyable struct for the better "     \
            "performance"
            );


    // GyroInfoContainer is a fixed size container, where means there's always
    // a memory chunk be created with size STACK_SIZE * sizeof(GyroInfo).
    // Caller can invoke GyroInfoContainer::size to get the available size of
    // gyro info in this container.
    struct GyroInfoContainer
    {
    // methods
        inline size_t           size() const { return _size; }
        inline void             setSize(size_t s) { _size = s; }
        inline void             clear() { _data.clear(); _data.resize(STACK_SIZE); _size = 0; }
        inline GyroInfo*        data() { return _data.data(); }
        inline const GyroInfo*  data() const { return _data.data(); }
        inline size_t           dataSize() const { return _size * sizeof(GyroInfo); }
    // operator
        inline GyroInfo&        operator [](size_t i) { return _data[i]; }
        inline const GyroInfo&  operator [](size_t i) const { return _data[i]; }
    // constructor
        GyroInfoContainer() noexcept
        {
            _size = 0;
            _data.resize(STACK_SIZE);
        }
        ~GyroInfoContainer() = default;
    // attributes
    private:
        std::vector<GyroInfo>   _data;
        size_t                  _size;
    };


//
// Constructor, destructor are forbidden
//
public:
    GyroCollector() = delete;
    ~GyroCollector() = delete;


//
// Method
//
public:
    // GyroCollector has to be triggered at least once during IDLE_TIMEOUT, or
    // it will stop listening gyro information anymore. Once it stopped, all
    // data in GyroCollector will be cleared.
    static void                         trigger();


    // Get the GyroInfo in range [ts_start, ts_end].
    //  @param ts_start                 The timestamp in the unit ns, excluding
    //                                  deep sleep duration.
    //  @param ts_end                   The timestamp in the unit ns, excluding
    //                                  deep sleep duration.
    //  @return                         A container contains matched items.
    //  @note                           The item is sorted by timestamp ascending.
    static GyroInfoContainer           getData(
                                            int64_t ts_start = 0,
                                            int64_t ts_end = INT64_MAX
                                        );


};  // class GyroCollector
};  // namespace Utils
};  // namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_GYROCOLLECTOR_H_
