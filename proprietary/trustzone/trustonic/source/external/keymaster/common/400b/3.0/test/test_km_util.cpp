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

#include <api_keymaster_defs.h>
#include <api_keymaster2.h>

#include "test_km_util.h"

//#undef  LOG_ANDROID
//#undef  LOG_TAG
//#define LOG_TAG "TlcTeeKeyMasterTest"

#include "log.h"

uint32_t block_length(
    keymaster_digest_t digest)
{
    switch (digest) {
        case KM_DIGEST_MD5:
            return 128;
        case KM_DIGEST_SHA1:
            return 160;
        case KM_DIGEST_SHA_2_224:
            return 224;
        case KM_DIGEST_SHA_2_256:
            return 256;
        case KM_DIGEST_SHA_2_384:
            return 384;
        case KM_DIGEST_SHA_2_512:
            return 512;
        default:
            return 0;
    }
}

void km_free_blob(
    keymaster_blob_t *blob)
{
    if (blob != NULL) {
        free((void*)blob->data);
        blob->data = NULL;
        blob->data_length = 0;
    }
}

void km_free_key_blob(
    keymaster_key_blob_t *key_blob)
{
    if (key_blob != NULL) {
        free((void*)key_blob->key_material);
        key_blob->key_material = NULL;
        key_blob->key_material_size = 0;
    }
}

keymaster_error_t save_key_blob(
    const char *name, const keymaster_key_blob_t *blob)
{
    int rc;
    FILE *fp = fopen(name, "wb");

    if (!fp ||
        fwrite(blob->key_material,
            1, blob->key_material_size, fp) < blob->key_material_size ||
        fflush(fp) || ferror(fp) || (rc = fclose(fp), fp = 0, rc))
        goto bad;
    return KM_ERROR_OK;
bad:
    if (fp) fclose(fp);
    LOG_E("failed to write blob file `%s': %s\n", name, strerror(errno));
    return KM_ERROR_UNKNOWN_ERROR;
}

keymaster_error_t check_blob(keymaster2_device_t *device,
    const keymaster_key_blob_t *blob,
    const keymaster_key_param_t *param, size_t nparam)
{
    keymaster_error_t res = KM_ERROR_OK;
    keymaster_operation_handle_t op;
    keymaster_key_param_set_t pset;

    pset.params = const_cast</*unconst*/ keymaster_key_param_t *>(param);
    pset.length = nparam;
    res = device->begin(device, KM_PURPOSE_SIGN, blob, &pset, NULL, &op);
    if (res != KM_ERROR_OK) return res;
    CHECK_RESULT_OK(device->abort(device, op));
end:
    return res;
}
