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

#ifndef SECLIENT_H
#define SECLIENT_H


typedef enum
{
	httpMethod_GET     = 0,   /* HTTP GET method */
	httpMethod_POST    = 1,   /* HTTP POST method */
	httpMethod_PUT     = 2,   /* HTTP PUT method */
	httpMethod_DELETE  = 3    /* HTTP DELETE method */
} httpMethod_t;

/**
    @param linkP note that client has to free the memory from *linkP after using it
    @param relP note that client has to free he memory from *relP after using it
    @param commandP note that client has to free he memory from *commandP after using it

    @return ROOTPA_OK on success. In case of an error *relP and *commandP are set to NULL;
            linkP is not change but the original link remains.
*/

typedef rootpaerror_t (*SeclientFuntionP)(const char* const inputP, const char** linkP, const char** relP, const char** commandP);

rootpaerror_t openSeClientAndInit();
void closeSeClientAndCleanup();

rootpaerror_t httpGetAndReceiveCommand(const char** linkP, const char** relP, const char** commandP);
rootpaerror_t httpPostAndReceiveCommand(const char* const inputP, const char** linkP, const char** relP, const char** commandP);
rootpaerror_t httpPutAndReceiveCommand(const char* const inputP, const char** linkP, const char** relP, const char** commandP);
rootpaerror_t httpDeleteAndReceiveCommand(const char** linkP, const char** relP, const char** commandP);

void setCertPath(const char* localPathP, const char* certPathP);


#endif //SECLIENT_H
