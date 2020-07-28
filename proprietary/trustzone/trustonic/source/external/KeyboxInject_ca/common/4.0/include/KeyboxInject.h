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

#ifndef TLC_KEYBOX_SAMPLE_H_
#define TLC_KEYBOX_SAMPLE_H_

#include "MobiCoreDriverApi.h"

#define LOG_TAG "KeyboxInject"

/**
 * Open a Secure session to the Keymaster TA.
 *
 * @param[in] spid level to open the TA.
 *
 * @return MC_DRV_OK or error
 */
mcResult_t tlcOpen(mcSpid_t spid);

/**
 * Send the attestation data as XML to the Keymaster TA
 * Get back a Secure Object to store in filesystem.
 *
 * @param[in] data_p pointer to XML data.
 * @param[in] data_len length of XML data.
 * @param[out] blob_p pointer to secure object to store.
 * @param[out] blob_len length of secure object to store.
 *
 * @return MC_DRV_OK or error
 */
#if RPMB_SUPPORT
mcResult_t tlcSetAttestationData(uint8_t *data_p, uint32_t data_len);
mcResult_t tlcCheckAttestationData(uint8_t *data_p, uint32_t data_len);
#else
mcResult_t tlcSetAttestationData(uint8_t *data_p, uint32_t data_len, uint8_t *blob_p, uint32_t *blob_len);
mcResult_t tlcCheckAttestationData(uint8_t *data_p, uint32_t data_len, uint8_t *blob_p, uint32_t blob_len);
#endif

/**
 * Close the Secure session.
 *
 * @return MC_DRV_OK or error
 */
void tlcClose(void);

#endif // TLC_KEYBOX_SAMPLE_H_
