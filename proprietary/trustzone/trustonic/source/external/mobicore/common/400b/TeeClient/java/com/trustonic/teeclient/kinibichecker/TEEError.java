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
package com.trustonic.teeclient.kinibichecker;

public enum TEEError {
    TEE_NOT_SUPPORTED, TEE_PROXY_INSTALLATION_REQUIRED, TEE_PROXY_LICENSE_REQUIRED, TEE_PROXY_UNREACHABLE_ERROR, TEE_PROXY_INTERNAL_ERROR;

    @Override
    public String toString(){

        StringBuilder message = new StringBuilder(this.name()).append(" ");
        switch(this){

            case TEE_NOT_SUPPORTED:
                message = message.append("TEE not supported on this device");
                break;

            case TEE_PROXY_INSTALLATION_REQUIRED:
                message = message.append("Downloadable proxy required");
                break;

            case TEE_PROXY_LICENSE_REQUIRED:
                message = message.append("TDP license required");
                break;

            case TEE_PROXY_UNREACHABLE_ERROR:
                message = message.append("TEE proxy cannot reachable");
                break;

            case TEE_PROXY_INTERNAL_ERROR:
                message = message.append("please check issue causes");
                break;

            default:
                message = message.append("unKnown Error");
                break;
        }
        return message.toString();
    }
}
