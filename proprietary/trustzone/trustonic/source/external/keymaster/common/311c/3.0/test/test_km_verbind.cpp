/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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

#include <api_keymaster2.h>

#include "test_km_verbind.h"
#include "test_km_util.h"

#include "tee_keymaster_device.h"
#define KEYMASTER_WANTED_VERSION 3
#include "trustonic_tee_keymaster_impl.h"

#include "log.h"

#ifdef DEBUG
static keymaster_error_t make_device(const struct hw_module_t *module,
    keymaster2_device_t **device_r)
{
    int err;

    if (*device_r) {
        keymaster2_close(*device_r);
        *device_r = NULL;
    }
    err = keymaster2_open(module, device_r);
    if (err < 0) {
        LOG_E("make_device: keymaster2_open: %s", strerror(-err));
        return KM_ERROR_UNKNOWN_ERROR;
    }
    return KM_ERROR_OK;
}

extern keymaster_error_t tee__set_debug_lies(TEE_SessionHandle,
    uint32_t os_version, uint32_t os_patchlevel);

static keymaster_error_t configure_with_lies(keymaster2_device_t *device,
    uint32_t os_version, uint32_t os_patchlevel)
{
    keymaster_error_t res = KM_ERROR_OK;
    TEE_SessionHandle session =
        ((TeeKeymasterDevice *)device)->impl_->session_handle_;

    CHECK_RESULT_OK(tee__set_debug_lies(session, os_version, os_patchlevel));

end:
    return res;
}

static keymaster_error_t make_key(keymaster2_device_t *device,
    keymaster_key_blob_t *blob)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_key_param_t param[5];
    keymaster_key_param_set_t pset;
    size_t nparam = 0;

    SETPARAM(param, nparam, enumerated, KM_TAG_ALGORITHM, KM_ALGORITHM_EC);
    SETPARAM(param, nparam, integer, KM_TAG_KEY_SIZE, 256);
    SETPARAM(param, nparam, enumerated, KM_TAG_PURPOSE, KM_PURPOSE_SIGN);
    SETPARAM(param, nparam, enumerated, KM_TAG_DIGEST, KM_DIGEST_NONE);
    SETPARAM(param, nparam, boolean, KM_TAG_NO_AUTH_REQUIRED, true);
    pset = { param, nparam };
    CHECK_RESULT_OK(device->generate_key(device, &pset, blob, NULL));

end:
    return res;
}

static keymaster_error_t test_verbind(const struct hw_module_t *module)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster2_device_t *olddev = NULL, *newdev = NULL;
    keymaster_key_blob_t oldblob = { NULL, 0 }, newblob = { NULL, 0 },
        tmpblob = { NULL, 0 };
    keymaster_key_param_set_t params = { NULL, 0 };

    /* Make old and new devices. */
    CHECK_RESULT_OK(make_device(module, &olddev));
    CHECK_RESULT_OK(configure_with_lies(olddev, 10203, 187601));
    CHECK_RESULT_OK(make_device(module, &newdev));
    CHECK_RESULT_OK(configure_with_lies(newdev, 10101, 195904));

    /* Make a blob on the old device.  It shouldn't work on the new one. */
    CHECK_RESULT_OK(make_key(olddev, &oldblob));
    CHECK_RESULT_OK(check_blob(olddev, &oldblob, NULL, 0));
    CHECK_TRUE(check_blob(newdev, &oldblob, NULL, 0) ==
        KM_ERROR_KEY_REQUIRES_UPGRADE);

    /* Upgrade the blob on the new device.  Then it should work OK on the new
     * device, but the old one (paradoxically) should say it needs upgrading.
     */
    CHECK_RESULT_OK(newdev->upgrade_key(newdev, &oldblob, &params, &newblob));
    CHECK_RESULT_OK(check_blob(newdev, &newblob, NULL, 0));
    CHECK_TRUE(check_blob(olddev, &newblob, NULL, 0) ==
        KM_ERROR_KEY_REQUIRES_UPGRADE);

    /* Trying to `upgrade' the new blob on the old device shouldn't work. */
    CHECK_TRUE(olddev->upgrade_key(olddev, &newblob, &params, &tmpblob) ==
        KM_ERROR_INVALID_ARGUMENT);

end:
    if (olddev) keymaster2_close(olddev);
    if (newdev) keymaster2_close(newdev);
    km_free_key_blob(&oldblob);
    km_free_key_blob(&newblob);
    km_free_key_blob(&tmpblob);
    return res;
}
#endif

keymaster_error_t test_km_verbind(keymaster2_device_t *main_device)
{
    keymaster_error_t res = KM_ERROR_OK;

#ifdef DEBUG
    /* Test key binding.  (This needs the `set_debug_lies' request, which is
     * compiled out of release builds.)
     */
    CHECK_RESULT_OK(test_verbind(main_device->common.module));

end:
#else
    (void)main_device;
#endif
    return res;
}
