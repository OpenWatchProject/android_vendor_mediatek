/*
 * Copyright (c) 2013-2017 TRUSTONIC LIMITED
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
#include <inttypes.h>
#include <stdlib.h>
#include <assert.h>
#include "tlcTeeKeymaster_if.h"
#include "km_util.h"
#include "km_shared_util.h"
#include "cust_tee_keymaster_impl.h"
#include "cust_tee_keymaster_utils.h"

#define DEFAULT_OBJECT_FILE "/data/misc/keybox_sample.so"

keymaster_error_t HAL_Configure(TEE_SessionHandle sessionHandle)
{
    struct TEE_Session *session = (struct TEE_Session *)sessionHandle;
    keymaster_error_t ret = KM_ERROR_OK;
    mcSessionHandle_t *session_handle = &session->sessionHandle;
    tciMessage_ptr tci = session->pTci;
    mcBulkMap_t srcMapInfo = {0, 0};
    uint8_t *so_p = NULL;
    uint32_t so_len = 0;

    LOG_D("HAL_CONFIGURE");

    /* TODO Reference implementation rely on attestation data is stored in
       FileSystem. If the attestation data is stored in RPMB, this part
       should be removed, and add the RPMB read/write interface in
       get_attestation_data() and set_attestation_data() in hal_attestation.c */

    so_len = getFileContent(DEFAULT_OBJECT_FILE, &so_p);
    if (so_len > 0) {
        LOG_D("so_p(%p) size %d", so_p, so_len);
        //return KM_ERROR_INVALID_ARGUMENT;
        CHECK_RESULT_OK(map_buffer(session_handle,
                so_p , so_len, &srcMapInfo));

        tci->command.header.commandId = CMD_ID_TEE_LOAD_ATTESTATION_DATA;
        tci->load_attestation_data.data.data = (uint32_t)srcMapInfo.sVirtualAddr;
        tci->load_attestation_data.data.data_length = (uint32_t)srcMapInfo.sVirtualLen;

        CHECK_RESULT_OK(transact(session_handle, tci));
    }
    else {
        LOG_D("%s(): could not load %s now", __func__, DEFAULT_OBJECT_FILE);
    }

end:
    unmap_buffer(session_handle, so_p, &srcMapInfo);
    free(so_p);
    return ret;
}
