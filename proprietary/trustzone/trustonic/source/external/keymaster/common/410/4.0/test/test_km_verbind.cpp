/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#include "TrustonicKeymaster4DeviceImpl.h"

#include "test_km_verbind.h"
#include "test_km_util.h"

#define KEYMASTER_WANTED_VERSION 4

#include "log.h"

/* From km_shared_util.h */
typedef struct {
    uint32_t os_version;
    uint32_t os_patchlevel;
    uint32_t vendor_patchlevel;
    uint32_t boot_patchlevel;
} bootimage_info_t;

#ifndef NDEBUG

extern keymaster_error_t tee__set_debug_lies(TEE_SessionHandle,
    const bootimage_info_t *bootinfo);

static keymaster_error_t make_key(TrustonicKeymaster4DeviceImpl *impl,
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
    CHECK_RESULT_OK(impl->generate_key(&pset, blob, NULL));

end:
    return res;
}

static keymaster_error_t check_blob(TrustonicKeymaster4DeviceImpl *impl,
    const keymaster_key_blob_t *blob,
    const keymaster_key_param_t *param, size_t nparam)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_operation_handle_t op;
    keymaster_key_param_set_t pset;

    pset.params = const_cast</*unconst*/ keymaster_key_param_t *>(param);
    pset.length = nparam;
    res = impl->begin(KM_PURPOSE_SIGN, blob, &pset, NULL, NULL, &op);
    if (res != KM_ERROR_OK) return res;
    CHECK_RESULT_OK(impl->abort(op));
end:
    return res;
}

static keymaster_error_t test_verbind()
{
    keymaster_error_t res = KM_ERROR_OK;
    TrustonicKeymaster4DeviceImpl *oldimpl = NULL, *newimpl = NULL;
    keymaster_key_blob_t oldblob = { NULL, 0 }, newblob = { NULL, 0 },
        tmpblob = { NULL, 0 };
    keymaster_key_param_set_t params = { NULL, 0 };
    bootimage_info_t bootinfo = {0,0,0,0};

    /* Make old and new impls. */
    oldimpl = new TrustonicKeymaster4DeviceImpl();
    bootinfo = {10203, 187601, 0, 0};
    CHECK_RESULT_OK(tee__set_debug_lies(oldimpl->session_handle_, &bootinfo));
    newimpl = new TrustonicKeymaster4DeviceImpl();
    bootinfo = {10301, 195904, 0, 0};
    CHECK_RESULT_OK(tee__set_debug_lies(newimpl->session_handle_, &bootinfo));

    /* Make a blob on the old impl.  It shouldn't work on the new one. */
    CHECK_RESULT_OK(make_key(oldimpl, &oldblob));
    CHECK_RESULT_OK(check_blob(oldimpl, &oldblob, NULL, 0));
    CHECK_TRUE(check_blob(newimpl, &oldblob, NULL, 0) ==
        KM_ERROR_KEY_REQUIRES_UPGRADE);

    /* Upgrade the blob on the new impl.  Then it should work OK on the new
     * impl, but the old one (paradoxically) should say it needs upgrading.
     */
    CHECK_RESULT_OK(newimpl->upgrade_key(&oldblob, &params, &newblob));
    CHECK_RESULT_OK(check_blob(newimpl, &newblob, NULL, 0));
    CHECK_TRUE(check_blob(oldimpl, &newblob, NULL, 0) ==
        KM_ERROR_KEY_REQUIRES_UPGRADE);

    /* Trying to `upgrade' the new blob on the old impl shouldn't work. */
    CHECK_TRUE(oldimpl->upgrade_key(&newblob, &params, &tmpblob) ==
        KM_ERROR_INVALID_ARGUMENT);

end:
    delete(oldimpl);
    delete(newimpl);
    km_free_key_blob(&oldblob);
    km_free_key_blob(&newblob);
    km_free_key_blob(&tmpblob);
    return res;
}
#endif

keymaster_error_t test_km_verbind()
{
    keymaster_error_t res = KM_ERROR_OK;

#ifndef NDEBUG
    /* Test key binding.  (This needs the `set_debug_lies' request, which is
     * compiled out of release builds.)
     */
    CHECK_RESULT_OK(test_verbind());
end:
#endif
    return res;
}
