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
import com.trustonic.teeclient.TeeException;

public class TeeMcException {
    public static class MC_DRV_NO_NOTIFICATION extends TeeException {
        public MC_DRV_NO_NOTIFICATION(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_NOTIFICATION extends TeeException {
        public MC_DRV_ERR_NOTIFICATION(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_NOT_IMPLEMENTED extends TeeException {
        public MC_DRV_ERR_NOT_IMPLEMENTED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_OUT_OF_RESOURCES extends TeeException {
        public MC_DRV_ERR_OUT_OF_RESOURCES(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_INIT extends TeeException {
        public MC_DRV_ERR_INIT(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_UNKNOWN extends TeeException {
        public MC_DRV_ERR_UNKNOWN(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_UNKNOWN_DEVICE extends TeeException {
        public MC_DRV_ERR_UNKNOWN_DEVICE(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_UNKNOWN_SESSION extends TeeException {
        public MC_DRV_ERR_UNKNOWN_SESSION(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_INVALID_OPERATION extends TeeException {
        public MC_DRV_ERR_INVALID_OPERATION(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_INVALID_RESPONSE extends TeeException {
        public MC_DRV_ERR_INVALID_RESPONSE(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_TIMEOUT extends TeeException {
        public MC_DRV_ERR_TIMEOUT(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_NO_FREE_MEMORY extends TeeException {
        public MC_DRV_ERR_NO_FREE_MEMORY(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_FREE_MEMORY_FAILED extends TeeException {
        public MC_DRV_ERR_FREE_MEMORY_FAILED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_SESSION_PENDING extends TeeException {
        public MC_DRV_ERR_SESSION_PENDING(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_DAEMON_UNREACHABLE extends TeeException {
        public MC_DRV_ERR_DAEMON_UNREACHABLE(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_INVALID_DEVICE_FILE extends TeeException {
        public MC_DRV_ERR_INVALID_DEVICE_FILE(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_INVALID_PARAMETER extends TeeException {
        public MC_DRV_ERR_INVALID_PARAMETER(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_KERNEL_MODULE extends TeeException {
        public MC_DRV_ERR_KERNEL_MODULE(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_BULK_MAPPING extends TeeException {
        public MC_DRV_ERR_BULK_MAPPING(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_BULK_UNMAPPING extends TeeException {
        public MC_DRV_ERR_BULK_UNMAPPING(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_INFO_NOTIFICATION extends TeeException {
        public MC_DRV_INFO_NOTIFICATION(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_NQ_FAILED extends TeeException {
        public MC_DRV_ERR_NQ_FAILED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_DAEMON_VERSION extends TeeException {
        public MC_DRV_ERR_DAEMON_VERSION(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_CONTAINER_VERSION extends TeeException {
        public MC_DRV_ERR_CONTAINER_VERSION(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_WRONG_PUBLIC_KEY extends TeeException {
        public MC_DRV_ERR_WRONG_PUBLIC_KEY(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_CONTAINER_TYPE_MISMATCH extends TeeException {
        public MC_DRV_ERR_CONTAINER_TYPE_MISMATCH(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_CONTAINER_LOCKED extends TeeException {
        public MC_DRV_ERR_CONTAINER_LOCKED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_SP_NO_CHILD extends TeeException {
        public MC_DRV_ERR_SP_NO_CHILD(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_TL_NO_CHILD extends TeeException {
        public MC_DRV_ERR_TL_NO_CHILD(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_UNWRAP_ROOT_FAILED extends TeeException {
        public MC_DRV_ERR_UNWRAP_ROOT_FAILED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_UNWRAP_SP_FAILED extends TeeException {
        public MC_DRV_ERR_UNWRAP_SP_FAILED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_UNWRAP_TRUSTLET_FAILED extends TeeException {
        public MC_DRV_ERR_UNWRAP_TRUSTLET_FAILED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN extends TeeException {
        public MC_DRV_ERR_DAEMON_DEVICE_NOT_OPEN(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_TA_ATTESTATION_ERROR extends TeeException {
        public MC_DRV_ERR_TA_ATTESTATION_ERROR(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_INTERRUPTED_BY_SIGNAL extends TeeException {
        public MC_DRV_ERR_INTERRUPTED_BY_SIGNAL(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_SERVICE_BLOCKED extends TeeException {
        public MC_DRV_ERR_SERVICE_BLOCKED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_SERVICE_LOCKED extends TeeException {
        public MC_DRV_ERR_SERVICE_LOCKED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_SERVICE_KILLED extends TeeException {
        public MC_DRV_ERR_SERVICE_KILLED(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_NO_FREE_INSTANCES extends TeeException {
        public MC_DRV_ERR_NO_FREE_INSTANCES(String msg) {
            super(msg);
        }
    }
    public static class MC_DRV_ERR_TA_HEADER_ERROR extends TeeException {
        public MC_DRV_ERR_TA_HEADER_ERROR(String msg) {
            super(msg);
        }
    }
    public static class MC_UNKNOWN_ERROR extends TeeException {
        public MC_UNKNOWN_ERROR(String msg) {
            super(msg);
        }
    }
}
