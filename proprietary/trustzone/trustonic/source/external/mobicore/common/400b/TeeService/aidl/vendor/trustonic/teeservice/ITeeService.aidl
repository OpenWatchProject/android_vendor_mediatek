/*
 * Copyright (c) 2018 TRUSTONIC LIMITED
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

package vendor.trustonic.teeservice;

import vendor.trustonic.teeservice.ITeeServiceListener;
import vendor.trustonic.teeservice.TeeServiceGpContextIn;
import vendor.trustonic.teeservice.TeeServiceGpSharedMemoryIn;
import vendor.trustonic.teeservice.TeeServiceGpOpenSessionIn;
import vendor.trustonic.teeservice.TeeServiceGpOpenSessionOut;
import vendor.trustonic.teeservice.TeeServiceGpCloseSessionIn;
import vendor.trustonic.teeservice.TeeServiceGpInvokeCommandIn;
import vendor.trustonic.teeservice.TeeServiceGpInvokeCommandOut;
import vendor.trustonic.teeservice.TeeServiceGpRequestCancellationIn;
import vendor.trustonic.teeservice.TeeServiceMcOpenSessionIn;
import vendor.trustonic.teeservice.TeeServiceMcOpenSessionOut;
import vendor.trustonic.teeservice.TeeServiceMcOpenTrustletIn;
import vendor.trustonic.teeservice.TeeServiceMcMapIn;
import vendor.trustonic.teeservice.TeeServiceMcMapOut;
import vendor.trustonic.teeservice.TeeServiceMcUnmapIn;
import vendor.trustonic.teeservice.TeeServiceMcGetSessionErrorCodeOut;
import vendor.trustonic.teeservice.TeeServiceMcGetMobiCoreVersionOut;

/**
 * System API for talking with the TEE service.
 *
 * {@hide}
 */
interface ITeeService {
    int registerClient(
        int client_version,
        in ITeeServiceListener client);

    int TEEC_InitializeContext(
        in TeeServiceGpContextIn params_in);

    void TEEC_FinalizeContext(
        in TeeServiceGpContextIn params_in);

    int TEEC_RegisterSharedMemory(
        in TeeServiceGpSharedMemoryIn params_in);

    void TEEC_ReleaseSharedMemory(
        in TeeServiceGpSharedMemoryIn params_in);

    int TEEC_OpenSession(
        in TeeServiceGpOpenSessionIn params_in,
        out TeeServiceGpOpenSessionOut params_out);

    void TEEC_CloseSession(
        in TeeServiceGpCloseSessionIn params_in);

    int TEEC_InvokeCommand(
        in TeeServiceGpInvokeCommandIn params_in,
        out TeeServiceGpInvokeCommandOut params_out);

    void TEEC_RequestCancellation(
        in TeeServiceGpRequestCancellationIn params_in);

    int mcOpenDevice();

    int mcCloseDevice();

    int mcOpenSession(
        in TeeServiceMcOpenSessionIn params_in,
        out TeeServiceMcOpenSessionOut params_out);

    int mcOpenTrustlet(
        in TeeServiceMcOpenTrustletIn params_in,
        out TeeServiceMcOpenSessionOut params_out);

    int mcCloseSession(
        int id);

    int mcNotify(
        int id);

    int mcWaitNotification(
        int id,
        int timeout,
        boolean partial);

    int mcMap(
        in TeeServiceMcMapIn params_in,
        out TeeServiceMcMapOut params_out);

    int mcUnmap(
        in TeeServiceMcUnmapIn params_in);

    int mcGetSessionErrorCode(
        int id,
        out TeeServiceMcGetSessionErrorCodeOut params_out);

    int mcGetMobiCoreVersion(
        out TeeServiceMcGetMobiCoreVersionOut params_out);
}
