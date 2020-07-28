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

#include <stdlib.h>
#include <MobiCoreRegistry.h>
#include "registry.h"

// AuthToken
int regWriteAuthToken(const AUTHTOKENCONTAINERP atP, uint32_t containerSize)
{
    return mcRegistryStoreAuthToken(atP, containerSize);
}

int regReadAuthToken(AUTHTOKENCONTAINERP* atP, uint32_t* containerSize)
{
    *containerSize = CONTAINER_BUFFER_SIZE; // this will be updated to actual size with the registry call
    *atP=(AUTHTOKENCONTAINERP)malloc(CONTAINER_BUFFER_SIZE);
    if(NULL==*atP) return MC_DRV_ERR_NO_FREE_MEMORY;
    return mcRegistryReadAuthToken(*atP, containerSize);
}

int regDeleteAuthToken(void)
{
    return mcRegistryDeleteAuthToken();
}

// Root

int regReadRoot(ROOTCONTAINERP* rootP, uint32_t* containerSize)
{
    *containerSize = CONTAINER_BUFFER_SIZE; // this will be updated to actual size with the registry call
    *rootP=(ROOTCONTAINERP)malloc(CONTAINER_BUFFER_SIZE);
    if(NULL==*rootP) return MC_DRV_ERR_NO_FREE_MEMORY;
    return mcRegistryReadRoot(*rootP, containerSize);
}


int regWriteRoot(const ROOTCONTAINERP rootP, uint32_t containerSize)
{
    return mcRegistryStoreRoot(rootP, containerSize);
}


int regCleanupRoot(void)
{
    return mcRegistryCleanupRoot();
}

// sp

int regReadSp(mcSpid_t spid, SPCONTAINERP* spP, uint32_t* containerSize)
{
    *containerSize = CONTAINER_BUFFER_SIZE; // this will be updated to actual size with the registry call
    *spP= (SPCONTAINERP)malloc(CONTAINER_BUFFER_SIZE);
    if(NULL==*spP) return MC_DRV_ERR_NO_FREE_MEMORY;
    return mcRegistryReadSp(spid, *spP, containerSize);
}

int regWriteSp(mcSpid_t spid, const SPCONTAINERP spP, uint32_t containerSize)
{
    return mcRegistryStoreSp(spid, spP, containerSize);
}

int regCleanupSp(mcSpid_t spid)
{
    return mcRegistryCleanupSp(spid);
}


int regGetSpState(mcSpid_t spid, mcContainerState_t* stateP)
{
    int ret;
    SPCONTAINERP spP=NULL;
    uint32_t containerSize=0;
    containerSize = CONTAINER_BUFFER_SIZE; // this will be updated to actual size with the registry call
    ret=regReadSp(spid, &spP, &containerSize);
    if(MC_DRV_OK==ret)
    {
        *stateP=spP->cont.attribs.state;
    }
    free(spP);
    return ret;
}


// tlt

int regReadTlt(const mcUuid_t* uuidP, TLTCONTAINERP* tltP, uint32_t* containerSize, mcSpid_t spid)
{
    *containerSize = CONTAINER_BUFFER_SIZE; // this will be update to actual size with the registry call
    *tltP=(TLTCONTAINERP)malloc(CONTAINER_BUFFER_SIZE);
    if(NULL==*tltP) return MC_DRV_ERR_NO_FREE_MEMORY;
    return mcRegistryReadTrustletCon(uuidP, spid, *tltP, containerSize);
}

int regWriteTlt(const mcUuid_t* uuidP, const TLTCONTAINERP tltP, uint32_t containerSize, mcSpid_t spid)
{
    return mcRegistryStoreTrustletCon(uuidP, spid, tltP, containerSize);
}

int regCleanupTlt(const mcUuid_t* uuidP, mcSpid_t spid)
{
    return mcRegistryCleanupTrustlet(uuidP, spid);
}

int regStoreTA(mcSpid_t spid, const mcUuid_t* uuidP, const uint8_t* taBinary, uint32_t taBinLength)
{
	(void) uuidP;
#ifdef WIN32
	return MC_DRV_ERR_INVALID_OPERATION; // TODO-fix Currently the Windows version of mcRegistry does not support mcRegistryStoreTABlob
#else
    return mcRegistryStoreTABlob(spid, (void*) taBinary, taBinLength);
#endif
}

int regGetTaState(mcSpid_t spid, const mcUuid_t* uuidP, mcContainerState_t* stateP)
{
    TLTCONTAINERP taP=NULL;
    uint32_t containerSize=0;
    containerSize = CONTAINER_BUFFER_SIZE; // this will be updated to actual size with the registry call
    int ret=regReadTlt(uuidP, &taP, &containerSize, spid);
    if(MC_DRV_OK==ret)
    {
        *stateP=taP->cont.common.attribs.state;
    }
    free(taP);
    return ret;
}
