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

#ifndef _ISECURE_CAMERA_H_
#define _ISECURE_CAMERA_H_

#include <mtkcam/main/security/Types.h>

#include <type_traits>

namespace NSCam {
namespace security {

// ---------------------------------------------------------------------------

enum class Command : uint32_t {
    STREAMING_ON,
    STREAMING_OFF,
    REGISTER_CB_FUNC,
    RETURN_CB_DATA,
    SET_SRC_DEV,
    SET_SHUTTER_TIME,
    SET_GAIN_VALUE,
    SET_SENSOR_CONFIG,
    GET_SENSOR_CONFIG
};

/// Generic function pointer
typedef void (*iris_callback_function_pointer_t)();

/// Callback function descriptor
/// NOTE: These descriptors are covered in the same mask,
///       and only one can be registered.
typedef enum {
    IRIS_CALLBACK_NONE    = 0x0,
    IRIS_CALLBACK_ADDR    = 0x1,
    IRIS_CALLBACK_MASK    = 0xFUL,
    // add new callback from here
    IRIS_CALLBACK_ALL     = 0x7FFFFFFFUL
} iris_callback_descriptor_t;

/// Callback wrapper
class Callback {
public:
    Callback() = delete;
    virtual ~Callback() = default;

    template <typename PFN, typename HOOK>
    static inline Callback createCallback(
            const iris_callback_descriptor_t desc,
            const HOOK hook)
    {
        return Callback(desc, asFunction<PFN>(hook));
    }

    iris_callback_descriptor_t inline getDescriptor() const { return mDesc; }
    iris_callback_function_pointer_t inline getHook() const { return mHook; }

private:
    Callback(
            const iris_callback_descriptor_t desc,
            const iris_callback_function_pointer_t hook)
        : mDesc(desc),
        mHook(hook)
    {};

    // function loaders
    template<typename PFN, typename T>
    static inline iris_callback_function_pointer_t asFunction(T function)
    {
        static_assert(std::is_same<PFN, T>::value,
                "Mismatched type of callback function pointer");
        return reinterpret_cast<iris_callback_function_pointer_t>(function);
    }

    // indicate a valid callback descriptor
    iris_callback_descriptor_t       mDesc;
    // callback function pointer corresponding to the callback descriptor
    iris_callback_function_pointer_t mHook;
};

// ---------------------------------------------------------------------------

/// Function pointer prototypes
///
/// NOTE: Add new callback prototype below the existing ones.
///       The name must be the combination of NewType and your function name.
///       (NewType should be defined in iris_callback_descriptor_t)
///
/// EXAMPLE:
/// // Function prototype
/// typedef Result (*IRIS_PFN_CALLBACK_ADDR)(Buffer buffer);
///
/// // Function implementation
/// Result __addrCallback(Buffer buffer) {
///     return IRIS_SUCCESS;
/// }
///
/// // Create callback object
/// Callback cbAddr = Callback::createCallback<IRIS_PFN_CALLBACK_ADDR>(
///         IRIS_CALLBACK_ADDR, __addrCallback);
typedef Result (*IRIS_PFN_CALLBACK_ADDR)(const Buffer& buffer);

// ---------------------------------------------------------------------------

class ISecureCamera
{
public:
    virtual ~ISecureCamera() = default;

    static ISecureCamera* loadModule();

    /// Open iris camera device.
    virtual Result open(Path path) = 0;

    /// Close iris camera device.
    virtual Result close() = 0;

    virtual Result init() = 0;

    virtual Result unInit() = 0;

    /// Iris Camera send command.
    /// the commands are defined as IrisCommand
    virtual Result sendCommand
        (Command cmd, intptr_t arg1 = 0, intptr_t arg2 = 0) = 0;
};

} // namespace security
} // namespace NSCam

#endif //_ISECURE_CAMERA_H_
