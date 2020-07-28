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

#ifndef PROVISIONINGENGINE_H
#define PROVISIONINGENGINE_H

#include <mcSpid.h>
#include "rootpa.h"

/**
 * Enum definition for initial provisioning relations.
 * Used while calling doProvisioningWithSe(..)
 */
typedef enum
{
	initialRel_POST    = 0,
	initialRel_DELETE  = 1
} initialRel_t;

rootpaerror_t setInitialAddress(const char* addrP, uint32_t length);

typedef rootpaerror_t (*GetVersionFunctionP) (int*, mcVersionInfo_t*);

void doProvisioningWithSe(
	mcSpid_t spid,
	mcSuid_t suid,
	CallbackFunctionP callbackP,
	SystemInfoCallbackFunctionP getSysInfoP,
	GetVersionFunctionP getVersionP,
	initialRel_t initialRel,
    trustletInstallationData_t* tltDataP);

rootpaerror_t uploadTrustlet(uint8_t* containerDataP, uint32_t containerLength);
#endif // PROVISIONINGENGINE_H

