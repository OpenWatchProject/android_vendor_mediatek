/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _HalAdaptationLayer_H_
#define _HalAdaptationLayer_H_

#include <gatekeeper/UniquePtr.h>
#include <hardware/hardware.h>
#include <hardware/gatekeeper.h>
#include "GateKeeperAdaptationLayer.h"
#include <memory>

struct HalAdaptationLayer
{
    // device_ must always be first member of a struct
    gatekeeper_device_t          device_;
    std::unique_ptr<GateKeeperAdaptationLayer>   impl_;

/* -----------------------------------------------------------------------------
 * @brief   Method implements HAL interface of gatekeeper. It wraps Android's
 *          gatekeeper::GateKeeper::Verify method.
 *          For detailed description regarding parameters please see:
 *          https://android.googlesource.com/platform/hardware/libhardware/+/android-6.0.0_r1/include/hardware/gatekeeper.h
 *
 * @return  Returned values are members of gatekeeper::gatekeeper_error_t enum.
 *          Definition of the enum can be found here:
 *          https://android.googlesource.com/platform/system/gatekeeper/+/android-6.0.0_r1/include/gatekeeper/gatekeeper_messages.h
 *
 *          ERROR_NONE: Success
 *          ERROR_INVALID: unexpected value of input parameters or verification
 *                          of current_password has failed.
 *          ERROR_UNKNOWN: any unexpected behaviour (see logs)
 *          ERROR_RETRY:   unused currently
 *          Any positive value T > 0 if the call should not be re-attempted
 *          until T milliseconds have elapsed.
-------------------------------------------------------------------------------- */
    static int enroll( const struct gatekeeper_device *dev, uint32_t uid,
                const uint8_t *current_password_handle, uint32_t current_password_handle_length,
                const uint8_t *current_password, uint32_t current_password_length,
                const uint8_t *desired_password, uint32_t desired_password_length,
                uint8_t **enrolled_password_handle, uint32_t *enrolled_password_handle_length);

/* -----------------------------------------------------------------------------
 * @brief   Method implements HAL interface of gatekeeper. It wraps Android's
 *          gatekeeper::GateKeeper::Verify method.
 *          For detailed description regarding parameters please see:
 *          https://android.googlesource.com/platform/hardware/libhardware/+/android-6.0.0_r1/include/hardware/gatekeeper.h
 *
 * @return  Returned values are members of gatekeeper::gatekeeper_error_t enum.
 *          Definition of the enum can be found here:
 *          https://android.googlesource.com/platform/system/gatekeeper/+/android-6.0.0_r1/include/gatekeeper/gatekeeper_messages.h
 *
 *          ERROR_NONE: Success
 *          ERROR_INVALID: unexpected value of input parameters or verification
 *                          of provided_password failed.
 *          ERROR_UNKNOWN: any unexpected behaviour (see logs)
 *          ERROR_RETRY:   unused currently
 *          Any positive value T > 0 if the call should not be re-attempted
 *          until T milliseconds have elapsed.
-------------------------------------------------------------------------------- */
    static int verify(const struct gatekeeper_device *dev, uint32_t uid, uint64_t challenge,
            const uint8_t *enrolled_password_handle, uint32_t enrolled_password_handle_length,
            const uint8_t *provided_password, uint32_t provided_password_length,
            uint8_t **auth_token, uint32_t *auth_token_length, bool *request_reenroll);

    HalAdaptationLayer(hw_module_t* module);

private:

    // Faciltates on-demand TEE session creation/destruction
    class SecureDeviceProxy
    {
        GateKeeperAdaptationLayer* gatekeeper_;
        bool is_openeed_;

    public:

        // Opens a Gatekeeper TEE session
        SecureDeviceProxy(const gatekeeper_device_t* dev)
        : gatekeeper_(NULL)
        , is_openeed_(false)
        {
            gatekeeper_ = reinterpret_cast<HalAdaptationLayer*>(
                    const_cast<gatekeeper_device_t*>(dev))->impl_.get();
            is_openeed_ = gatekeeper_->Open();
        }

        // Ensures that Gatekeeper TEE session is closed before object is destructed
        ~SecureDeviceProxy()
        {
            if(!gatekeeper_->Close())
            {
                // This should never happen, but if it happens then system is not in
                // a consistent state and must be aborted
                ::abort();
            }
        }

        // Method must be used before any access to the GateKeeperAdapationLayer pointer
        bool is_initialized() const
        { return is_openeed_; }

        GateKeeperAdaptationLayer* operator->()
        { return gatekeeper_; }
    };

    // Struct is non-copyable and not default constructible
    HalAdaptationLayer();
    HalAdaptationLayer(const HalAdaptationLayer&);
    HalAdaptationLayer& operator=(const HalAdaptationLayer&);
};

#endif /* _HalAdaptationLayer_H_ */
