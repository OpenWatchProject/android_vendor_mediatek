/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
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

#include "HalAdaptationLayer.h"
#include "module.h"
#include <memory>
#include "buildTag.h"
#include "TeeSession.h"


extern struct gatekeeper_module HAL_MODULE_INFO_SYM;

namespace {

    /**
     * Class facilitates on-demand session creation with the TA.
     *
     * Constructor creates session with the TA, which is automatically closed
     * when object is destructed (RAI).
     *
     * It must not to be possible to copy object of those classes.
     */
    class ScopedSession
    {
        HalAdaptationLayer* _hal;
        bool _is_initialized;

     public:

        // Opens a Gatekeeper TEE session
        explicit ScopedSession(const gatekeeper_device_t* dev)
        : _hal(NULL)
        , _is_initialized(false)
        {
            _hal = reinterpret_cast<HalAdaptationLayer*>(
                    const_cast<gatekeeper_device_t*>(dev));
            _is_initialized = _hal->Initialize();
        }

        // Ensures that Gatekeeper TEE session is closed before object is destructed
        ~ScopedSession()
        {
            _hal->Terminate();
        }

        bool is_initialized() const
        { return _is_initialized; }

        TeeSession* operator->()
        {
            if (_hal)
                return _hal->_session.get();
            return NULL;
        }

        // Objects must be always created on stack
        static void * operator new(std::size_t)                 = delete;
        static void * operator new[] (std::size_t)              = delete;

        // Class is non-copyable and not default constructible
        ScopedSession()                                         = delete;
        ScopedSession(const ScopedSession&)                     = delete;
        ScopedSession& operator=(const ScopedSession&)          = delete;
    };
}


extern "C" {

/******************************************************************************/
__attribute__((visibility("default")))
int kinibi_gatekeeper_open( const struct hw_module_t* module, const char* id,
                            struct hw_device_t** device)
{
    LOG_D("Opening Kinibi Gatekeeper device.");
    LOG_I(MOBICORE_COMPONENT_BUILD_TAG);

    if ( id == NULL )
        return -EINVAL;

    if ( memcmp(id, HARDWARE_GATEKEEPER, strlen(HARDWARE_GATEKEEPER)) != 0)
        return -EINVAL;

    // Make sure we initialize only if module provided is known
    if ((module->tag != HAL_MODULE_INFO_SYM.common.tag) ||
        (module->module_api_version != HAL_MODULE_INFO_SYM.common.module_api_version) ||
        (module->hal_api_version != HAL_MODULE_INFO_SYM.common.hal_api_version) ||
        (0!=memcmp(module->name, HAL_MODULE_INFO_SYM.common.name,
                    sizeof(GATEKEEPER_HARDWARE_MODULE_TEE_NAME)-1)) )
    {
        return -EINVAL;
    }

    std::unique_ptr<HalAdaptationLayer> gatekeeper_device(
        new HalAdaptationLayer(const_cast<hw_module_t*>(module)));

    if (!gatekeeper_device)
    {
        LOG_E("Heap exhausted. Exiting...");
        return -ENOMEM;
    }

    ScopedSession session(&gatekeeper_device->_device);
    if (session.is_initialized() )
    {
        if (ERROR_NONE != session->CheckVersion()) {
            // version is wrong
            return -EINVAL;
        }
    }

    *device = reinterpret_cast<hw_device_t*>(gatekeeper_device.release());
    LOG_D("TEE Gatekeeper device created");
    return 0;
}

/******************************************************************************/
__attribute__((visibility("default")))
int kinibi_gatekeeper_close(hw_device_t *hw)
{
     if (hw == NULL)
        return 0; // Nothing to close closed

    HalAdaptationLayer* gk = reinterpret_cast<HalAdaptationLayer*>(hw);
    if (NULL == gk)
    {
        LOG_E("TEE Gatekeeper not initialized.");
        return -ENODEV;
    }

    delete gk;
    return 0;
}
} // extern "C"

/*
 * Needed in order to make forward declaration of TeeSession possible
 */
HalAdaptationLayer::~HalAdaptationLayer()
{}

/******************************************************************************/
int HalAdaptationLayer::enroll(
    const struct gatekeeper_device *dev, uint32_t uid,
    const uint8_t *current_password_handle, uint32_t current_password_handle_length,
    const uint8_t *current_password, uint32_t current_password_length,
    const uint8_t *desired_password, uint32_t desired_password_length,
    uint8_t **enrolled_password_handle, uint32_t *enrolled_password_handle_length)
{
    if ( NULL == dev )
    {
        LOG_E("Wrong input parameters");
        return (-ERROR_INVALID);
    }

    ScopedSession session(dev);
    if (session.is_initialized() )
    {
        int throttle_ms = 0;
        gatekeeper_error_t ret = ERROR_UNKNOWN;
        ret = session->Enroll( uid,
                               current_password_handle, current_password_handle_length,
                               current_password, current_password_length,
                               desired_password, desired_password_length,
                               enrolled_password_handle, enrolled_password_handle_length,
                               throttle_ms);
        if (throttle_ms>0)
            return throttle_ms;
        return (-ret);
    }
    return -ERROR_UNKNOWN;
}

/******************************************************************************/
int HalAdaptationLayer::verify(
    const struct gatekeeper_device *dev, uint32_t uid, uint64_t challenge,
    const uint8_t *enrolled_password_handle, uint32_t enrolled_password_handle_length,
    const uint8_t *provided_password, uint32_t provided_password_length,
    uint8_t **auth_token, uint32_t *auth_token_length, bool *request_reenroll)
{
    // Validate only dev, rest of parameters is validated in TeeSession
    if (NULL == dev)
    {
        LOG_E("Wrong input parameters");
        return -(ERROR_INVALID);
    }

    ScopedSession session(dev);
    if (session.is_initialized())
    {
        bool is_reenroll = false;
        int throttle_ms = 0;
        gatekeeper_error_t ret = ERROR_UNKNOWN;
        ret = session->Verify(  uid, challenge,
                                enrolled_password_handle, enrolled_password_handle_length,
                                provided_password, provided_password_length,
                                auth_token, auth_token_length,
                                is_reenroll, throttle_ms);

        if (request_reenroll != NULL)
        {
            *request_reenroll = false;
        }
        if (throttle_ms>0)
            return throttle_ms;
        return (-ret);
    }
    return -ERROR_UNKNOWN;
}

int HalAdaptationLayer::delete_user(const struct gatekeeper_device *dev, uint32_t uid)
{
    if (NULL == dev)
    {
        LOG_E("Wrong input parameter");
        return -(ERROR_INVALID);
    }

    ScopedSession session(dev);
    if (session.is_initialized())
    {
        gatekeeper_error_t ret = session->DeleteUser(uid, false);
        return -ret;
    }
    return -ERROR_UNKNOWN;
}

/******************************************************************************/
int HalAdaptationLayer::delete_all_users(const struct gatekeeper_device *dev)
{
    if (NULL == dev)
    {
        LOG_E("Wrong input parameter");
        return -(ERROR_INVALID);
    }

    ScopedSession session(dev);
    if (session.is_initialized())
    {
        gatekeeper_error_t ret = session->DeleteUser(0/*ignored*/, true);
        return -ret;
    }
    return -ERROR_UNKNOWN;
}

/******************************************************************************/
HalAdaptationLayer::HalAdaptationLayer(hw_module_t* module)
: _session(new TeeSession() )
{
    /* -------------------------------------------------------------------------
       Device description
       -------------------------------------------------------------------------*/
    _device.common.tag = HARDWARE_MODULE_TAG;
    _device.common.version = 1;
    _device.common.module = module;
    _device.common.close = kinibi_gatekeeper_close;

    /* -------------------------------------------------------------------------
       All function pointers used by the HAL module
       -------------------------------------------------------------------------*/
    _device.enroll = HalAdaptationLayer::enroll;
    _device.verify = HalAdaptationLayer::verify;

    // Optional functions
    _device.delete_user = HalAdaptationLayer::delete_user;
    _device.delete_all_users = HalAdaptationLayer::delete_all_users;
}

/******************************************************************************/
bool HalAdaptationLayer::Initialize()
{
    if ( _session->IsOpened() ) {
        LOG_D("Gatekeeper's TEE session alredy opened");
        return false;
    } else {
        _session.reset(new TeeSession());
    }
    return (MC_DRV_OK == _session->Open());
}

/******************************************************************************/
void HalAdaptationLayer::Terminate()
{
    if ( _session->IsOpened() ) {
        if ( !_session->Close() ) {
            _session.reset(new TeeSession());
        }
    } else {
        LOG_D("Gatekeeper's TEE session is not opened");
    }
}
