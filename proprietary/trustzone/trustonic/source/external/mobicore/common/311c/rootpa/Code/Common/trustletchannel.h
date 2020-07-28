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

#ifndef TRUSTLETCHANNEL_H
#define TRUSTLETCHANNEL_H

#include<wrapper.h>

#include<MobiCoreDriverApi.h>

typedef struct cmtstruct CMTSTRUCT;
struct cmtstruct
{
  uint8_t*              wsmP;         ///< World Shared Memory (WSM) to the TCI buffer
  uint8_t*              mappedP;      ///< pointer to the buffer that is mapped to be used with SWd. Nwd uses this.
  uint32_t              mappedSize;   ///< size of the buffer pointed by mappedP. Nwd uses this.
  mcBulkMap_t           mapInfo;      ///< information on mapped memory
  mcSessionHandle_t     session;      ///< session handle
  uint32_t              lasterror;    ///< last MC driver (MobiCoreDriverAPI.h 0x00000000... MC_DRV_OK(0)) error
                                      ///< or CMTL ( tlCmError.h 0xE0000000...(SUCCESSFUL(0))) error
};

typedef CMTSTRUCT* CMTHANDLE;

/**
Open session to content management trustlet and allocate enough memory for communication
*/
CMTHANDLE tltChannelOpen(int sizeOfWsmBuffer, mcResult_t* result);

/**
Open session to TA and allocate enough memory for communication. There are two way to do this, give TA uuid or TA binary, binary length and spid.
The former works with system TA's the latter with SP TA's.
*/
CMTHANDLE taChannelOpen(int sizeOfWsmBuffer,  mcResult_t* result, const mcUuid_t* uuidP, uint8_t* taBinaryP, uint32_t taLength, mcSpid_t spid);
/**
*/
void tltChannelClose(CMTHANDLE handle);
/**
*/
bool tltChannelTransmit(CMTHANDLE handle, int timeout);

#endif // TRUSTLETCHANNEL_H
