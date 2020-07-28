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

#ifndef PACMTL_H
#define PACMTL_H

#include <MobiCoreDriverApi.h>
#include <TlCm/3.0/tlCmApi.h>
#include "trustletchannel.h"
#include "rootpa.h"

#define CMP_VERSION 0x00030000

/**
set the element info (offset and size) to wsmP in correct location

@param elementNbrP number of the element, starting from one, increased by one before return
@param wsmP pointer to the beginnign of wsm buffer
@param &elementOffsetP offset to the element, 0 when the element number is 1, increased by length before return
@param elementLength length of the element
*/
void setCmdElementInfo(uint32_t* elementNbrP, uint8_t* wsmP, uint32_t* elementOffsetP, uint32_t elementLength);

void setCmdMapInfo(uint8_t* wsmP, const mcBulkMap_t* mapInfoP);

void setCmdCmpVersionAndCmdId(uint8_t* wsmP, cmpCommandId_t commandId);

bool getRspElementInfo(uint32_t* elementNbrP, CMTHANDLE handle, uint32_t* elementOffsetP, uint32_t* elementLengthP);

uint32_t getRspCmpVersion(const uint8_t* wsmP);

uint32_t getRspCmpId(const uint8_t* wsmP);

bool isValidResponseTo(cmpCommandId_t commandId, const uint8_t* wsmP);
/**
@param pointer to the buffer containing response from CMTL
@return needed bytes from the response, -1 if wsmP is not valid response. Accoriding to the specification 0 if the response fit into the buffer.
*/
uint32_t getNeededBytesFromResponse(const uint8_t* wsmP);

uint32_t getTotalMappedBufferSize(CmpMessage* commandP);
#endif // PACMTL_H
