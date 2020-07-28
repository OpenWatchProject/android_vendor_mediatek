/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
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
package com.trustonic.teeclient;
import com.trustonic.teeclient.TeeMcException;

import java.lang.Integer;
import android.util.Log;
import android.content.Context;

class TeeClientJni  {
    private static final String LOG_TAG = TeeClientJni.class.getSimpleName() + "_JAVA";

    // Constants code duplicated from MobiCoreDriverApi.h
    public static final int MC_DEVICE_ID_DEFAULT = 0;

    // Error code duplicated from MobiCoreDriverApi.h
    private static final int MC_DRV_OK                                   = 0x00000000;
    private static final int MC_DRV_NO_NOTIFICATION                      = 0x00000001;
    private static final int MC_DRV_ERR_NOTIFICATION                     = 0x00000002;
    private static final int MC_DRV_ERR_NOT_IMPLEMENTED                  = 0x00000003;
    private static final int MC_DRV_ERR_OUT_OF_RESOURCES                 = 0x00000004;
    private static final int MC_DRV_ERR_INIT                             = 0x00000005;
    private static final int MC_DRV_ERR_UNKNOWN                          = 0x00000006;
    private static final int MC_DRV_ERR_UNKNOWN_DEVICE                   = 0x00000007;
    private static final int MC_DRV_ERR_UNKNOWN_SESSION                  = 0x00000008;
    private static final int MC_DRV_ERR_INVALID_OPERATION                = 0x00000009;
    private static final int MC_DRV_ERR_INVALID_RESPONSE                 = 0x0000000a;
    private static final int MC_DRV_ERR_TIMEOUT                          = 0x0000000b;
    private static final int MC_DRV_ERR_NO_FREE_MEMORY                   = 0x0000000c;
    private static final int MC_DRV_ERR_FREE_MEMORY_FAILED               = 0x0000000d;
    private static final int MC_DRV_ERR_SESSION_PENDING                  = 0x0000000e;
    private static final int MC_DRV_ERR_DAEMON_UNREACHABLE               = 0x0000000f;
    private static final int MC_DRV_ERR_INVALID_DEVICE_FILE              = 0x00000010;
    private static final int MC_DRV_ERR_INVALID_PARAMETER                = 0x00000011;
    private static final int MC_DRV_ERR_KERNEL_MODULE                    = 0x00000012;
    private static final int MC_DRV_ERR_BULK_MAPPING                     = 0x00000013;
    private static final int MC_DRV_ERR_BULK_UNMAPPING                   = 0x00000014;
    private static final int MC_DRV_INFO_NOTIFICATION                    = 0x00000015;
    private static final int MC_DRV_ERR_NQ_FAILED                        = 0x00000016;
    private static final int MC_DRV_ERR_DAEMON_VERSION                   = 0x00000017;
    private static final int MC_DRV_ERR_CONTAINER_VERSION                = 0x00000018;
    private static final int MC_DRV_ERR_WRONG_PUBLIC_KEY                 = 0x00000019;
    private static final int MC_DRV_ERR_CONTAINER_TYPE_MISMATCH          = 0x0000001a;
    private static final int MC_DRV_ERR_CONTAINER_LOCKED                 = 0x0000001b;
    private static final int MC_DRV_ERR_SP_NO_CHILD                      = 0x0000001c;
    private static final int MC_DRV_ERR_TL_NO_CHILD                      = 0x0000001d;
    private static final int MC_DRV_ERR_UNWRAP_ROOT_FAILED               = 0x0000001e;
    private static final int MC_DRV_ERR_UNWRAP_SP_FAILED                 = 0x0000001f;
    private static final int MC_DRV_ERR_UNWRAP_TRUSTLET_FAILED           = 0x00000020;
    private static final int MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN           = 0x00000021;
    private static final int MC_DRV_ERR_TA_ATTESTATION_ERROR             = 0x00000022;
    private static final int MC_DRV_ERR_INTERRUPTED_BY_SIGNAL            = 0x00000023;
    private static final int MC_DRV_ERR_SERVICE_BLOCKED                  = 0x00000024;
    private static final int MC_DRV_ERR_SERVICE_LOCKED                   = 0x00000025;
    private static final int MC_DRV_ERR_SERVICE_KILLED                   = 0x00000026;
    private static final int MC_DRV_ERR_NO_FREE_INSTANCES                = 0x00000027;
    private static final int MC_DRV_ERR_TA_HEADER_ERROR                  = 0x00000028;

    private void mcErrorToMcException(int nativeErrorCode, String mcFunctionName) throws TeeException {
        switch (nativeErrorCode) {
            case MC_DRV_OK :
                // No exception
                break;
            case MC_DRV_NO_NOTIFICATION :
                throw new TeeMcException.MC_DRV_NO_NOTIFICATION(mcFunctionName);
            case MC_DRV_ERR_NOTIFICATION :
                throw new TeeMcException.MC_DRV_ERR_NOTIFICATION(mcFunctionName);
            case MC_DRV_ERR_NOT_IMPLEMENTED :
                throw new TeeMcException.MC_DRV_ERR_NOT_IMPLEMENTED(mcFunctionName);
            case MC_DRV_ERR_OUT_OF_RESOURCES :
                throw new TeeMcException.MC_DRV_ERR_OUT_OF_RESOURCES(mcFunctionName);
            case MC_DRV_ERR_INIT :
                throw new TeeMcException.MC_DRV_ERR_INIT(mcFunctionName);
            case MC_DRV_ERR_UNKNOWN :
                throw new TeeMcException.MC_DRV_ERR_UNKNOWN(mcFunctionName);
            case MC_DRV_ERR_UNKNOWN_DEVICE :
                throw new TeeMcException.MC_DRV_ERR_UNKNOWN_DEVICE(mcFunctionName);
            case MC_DRV_ERR_UNKNOWN_SESSION :
                throw new TeeMcException.MC_DRV_ERR_UNKNOWN_SESSION(mcFunctionName);
            case MC_DRV_ERR_INVALID_OPERATION :
                throw new TeeMcException.MC_DRV_ERR_INVALID_OPERATION(mcFunctionName);
            case MC_DRV_ERR_INVALID_RESPONSE :
                throw new TeeMcException.MC_DRV_ERR_INVALID_RESPONSE(mcFunctionName);
            case MC_DRV_ERR_TIMEOUT :
                throw new TeeMcException.MC_DRV_ERR_TIMEOUT(mcFunctionName);
            case MC_DRV_ERR_NO_FREE_MEMORY :
                throw new TeeMcException.MC_DRV_ERR_NO_FREE_MEMORY(mcFunctionName);
            case MC_DRV_ERR_FREE_MEMORY_FAILED :
                throw new TeeMcException.MC_DRV_ERR_FREE_MEMORY_FAILED(mcFunctionName);
            case MC_DRV_ERR_SESSION_PENDING :
                throw new TeeMcException.MC_DRV_ERR_SESSION_PENDING(mcFunctionName);
            case MC_DRV_ERR_DAEMON_UNREACHABLE :
                throw new TeeMcException.MC_DRV_ERR_DAEMON_UNREACHABLE(mcFunctionName);
            case MC_DRV_ERR_INVALID_DEVICE_FILE :
                throw new TeeMcException.MC_DRV_ERR_INVALID_DEVICE_FILE(mcFunctionName);
            case MC_DRV_ERR_INVALID_PARAMETER :
                throw new TeeMcException.MC_DRV_ERR_INVALID_PARAMETER(mcFunctionName);
            case MC_DRV_ERR_KERNEL_MODULE :
                throw new TeeMcException.MC_DRV_ERR_KERNEL_MODULE(mcFunctionName);
            case MC_DRV_ERR_BULK_MAPPING :
                throw new TeeMcException.MC_DRV_ERR_BULK_MAPPING(mcFunctionName);
            case MC_DRV_ERR_BULK_UNMAPPING :
                throw new TeeMcException.MC_DRV_ERR_BULK_UNMAPPING(mcFunctionName);
            case MC_DRV_INFO_NOTIFICATION :
                throw new TeeMcException.MC_DRV_INFO_NOTIFICATION(mcFunctionName);
            case MC_DRV_ERR_NQ_FAILED :
                throw new TeeMcException.MC_DRV_ERR_NQ_FAILED(mcFunctionName);
            case MC_DRV_ERR_DAEMON_VERSION :
                throw new TeeMcException.MC_DRV_ERR_DAEMON_VERSION(mcFunctionName);
            case MC_DRV_ERR_CONTAINER_VERSION :
                throw new TeeMcException.MC_DRV_ERR_CONTAINER_VERSION(mcFunctionName);
            case MC_DRV_ERR_WRONG_PUBLIC_KEY :
                throw new TeeMcException.MC_DRV_ERR_WRONG_PUBLIC_KEY(mcFunctionName);
            case MC_DRV_ERR_CONTAINER_TYPE_MISMATCH :
                throw new TeeMcException.MC_DRV_ERR_CONTAINER_TYPE_MISMATCH(mcFunctionName);
            case MC_DRV_ERR_CONTAINER_LOCKED :
                throw new TeeMcException.MC_DRV_ERR_CONTAINER_LOCKED(mcFunctionName);
            case MC_DRV_ERR_SP_NO_CHILD :
                throw new TeeMcException.MC_DRV_ERR_SP_NO_CHILD(mcFunctionName);
            case MC_DRV_ERR_TL_NO_CHILD :
                throw new TeeMcException.MC_DRV_ERR_TL_NO_CHILD(mcFunctionName);
            case MC_DRV_ERR_UNWRAP_ROOT_FAILED :
                throw new TeeMcException.MC_DRV_ERR_UNWRAP_ROOT_FAILED(mcFunctionName);
            case MC_DRV_ERR_UNWRAP_SP_FAILED :
                throw new TeeMcException.MC_DRV_ERR_UNWRAP_SP_FAILED(mcFunctionName);
            case MC_DRV_ERR_UNWRAP_TRUSTLET_FAILED :
                throw new TeeMcException.MC_DRV_ERR_UNWRAP_TRUSTLET_FAILED(mcFunctionName);
            case MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN :
                throw new TeeMcException.MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN(mcFunctionName);
            case MC_DRV_ERR_TA_ATTESTATION_ERROR :
                throw new TeeMcException.MC_DRV_ERR_TA_ATTESTATION_ERROR(mcFunctionName);
            case MC_DRV_ERR_INTERRUPTED_BY_SIGNAL :
                throw new TeeMcException.MC_DRV_ERR_INTERRUPTED_BY_SIGNAL(mcFunctionName);
            case MC_DRV_ERR_SERVICE_BLOCKED :
                throw new TeeMcException.MC_DRV_ERR_SERVICE_BLOCKED(mcFunctionName);
            case MC_DRV_ERR_SERVICE_LOCKED :
                throw new TeeMcException.MC_DRV_ERR_SERVICE_LOCKED(mcFunctionName);
            case MC_DRV_ERR_SERVICE_KILLED :
                throw new TeeMcException.MC_DRV_ERR_SERVICE_KILLED(mcFunctionName);
            case MC_DRV_ERR_NO_FREE_INSTANCES :
                throw new TeeMcException.MC_DRV_ERR_NO_FREE_INSTANCES(mcFunctionName);
            case MC_DRV_ERR_TA_HEADER_ERROR :
                throw new TeeMcException.MC_DRV_ERR_TA_HEADER_ERROR(mcFunctionName);
            default :
                throw new TeeMcException.MC_UNKNOWN_ERROR(mcFunctionName);
        }
    }

    private Context application_context_ = null;
    public TeeClientJni(Context applicationContext) {
        application_context_ = applicationContext;
    }

    public void mcOpenDevice(int deviceId) throws TeeException {
        int res = jniMcOpenDevice(deviceId, application_context_);
        Log.d(LOG_TAG, "mcOpenDevice deviceId(" + deviceId + ") returned 0x" + Integer.toHexString(res));
        mcErrorToMcException(res, "mcOpenDevice");
    }
    private native int jniMcOpenDevice(int deviceId, Context applicationContext);

    public void mcCloseDevice(int deviceId) throws TeeException {
        int res = jniMcCloseDevice(deviceId);
        Log.d(LOG_TAG, "mcCloseDevice deviceId(" + deviceId + ") returned 0x" + Integer.toHexString(res));
        mcErrorToMcException(res, "mcCloseDevice");
    }
    private native int jniMcCloseDevice(int deviceId);

    static {
        System.loadLibrary("TeeClient");
    }
};
