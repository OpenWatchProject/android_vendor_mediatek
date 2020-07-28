/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
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

#ifndef _def_H_
#define _def_H_

/**
 * Trustlet UUID
 */
#define TEE_GATEKEEPER_TL_UUID { { 7, 6, 0x10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

/**
 * Length of the password returned by GetPasswordKey() function
 */
#define PASSWORD_KEY_LEN 1024

/**
 * Length of the data that represents per boot unique key and lock status
 */
#define PERBOOT_HMAC_KEY_LEN 32

/**
 * Command ID's
 */
#define CMD_ID_TEE_MILLI_SINCE_BOOT           1
#define CMD_ID_TEE_GET_PASSWORD_KEY           2
#define CMD_ID_TEE_GET_RANDOM                 3
#define CMD_ID_TEE_COMPUTE_PASS_SIGNATURE     4
#define CMD_ID_TEE_GET_AUTH_TOKEN_KEY         5
#define CMD_ID_TEE_FAILURE_RECORD_ENCODE      6
#define CMD_ID_TEE_FAILURE_RECORD_DECODE      7
#define CMD_ID_TEE_FAILURE_RECORD_STORE       8
#define CMD_ID_TEE_FAILURE_RECORD_RETRIEVE    9


#endif /* _def_ */
