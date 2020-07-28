/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
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

#include <mcContainer.h>

#define CONTAINER_BUFFER_SIZE 4096

// container sizes are and must be used for memory allocation only, since there is possibility to increase the buffer size these
// are informative and RootPA should work fine also when the container sizes change after RootPA deployment
#define SIZEOFAUTHTOKENCONTAINER (sizeof(mcSoAuthTokenCont_t))
#define SIZEOFROOTCONTAINER (sizeof(mcSoRootCont_t))
#define SIZEOFSPCONTAINER (sizeof(mcSoSpCont_t))
#define SIZEOFTLTCONTAINER (sizeof(mcSoTltCont_2_1_t))

typedef mcSoAuthTokenCont_t* AUTHTOKENCONTAINERP;
typedef mcSoRootCont_t* ROOTCONTAINERP;
typedef mcSoSpCont_t* SPCONTAINERP;
typedef mcSoTltCont_t* TLTCONTAINERP;

int regReadAuthToken(AUTHTOKENCONTAINERP* atP, uint32_t* containerSize);
int regWriteAuthToken(const AUTHTOKENCONTAINERP atP, uint32_t containerSize);
int regDeleteAuthToken(void);

int regReadRoot(ROOTCONTAINERP* rootP, uint32_t* containerSize);
int regWriteRoot(const ROOTCONTAINERP rootP, uint32_t containerSize);
int regCleanupRoot(void);

int regReadSp(mcSpid_t spid, SPCONTAINERP* spP, uint32_t* containerSize);
int regWriteSp(mcSpid_t spid, const SPCONTAINERP spP, uint32_t containerSize);
int regCleanupSp(mcSpid_t spid);

int regGetSpState(mcSpid_t spid, mcContainerState_t* stateP);

int regReadTlt(const mcUuid_t* uuidP, TLTCONTAINERP* tltP, uint32_t* containerSize, mcSpid_t spid);
int regWriteTlt(const mcUuid_t* uuidP, const TLTCONTAINERP tltP, uint32_t containerSize, mcSpid_t spid);
int regCleanupTlt(const mcUuid_t* uuidP, mcSpid_t spid);

int regStoreTA(mcSpid_t spid, const mcUuid_t* uuidP, const uint8_t* taBinary, uint32_t taBinLength);
int regGetTaState(mcSpid_t spid, const mcUuid_t* uuidP, mcContainerState_t* stateP);