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

#ifndef XMLMESSAGEHANDLER_H
#define XMLMESSAGEHANDLER_H
#include "rootpaErrors.h"
#include "rootpa.h"

/**
    @param xmlMessageP received xml message to be handled
    @param xmlResponseP response to the received message in xml format. The caller is
           responsible on freeing the memory the pointer points to.
    @return ROOTPA_OK on success, ROOTPA_ERROR_ILLEGAL_ARGUMENT if
            xmlMessageP is NULL or if the message is not valid xml
            If error is returned, the error is also included in xmlResponse
            (unless there is problem in allocating memory) and the response
            should still be returned to SE.

*/
rootpaerror_t handleXmlMessage(const char* xmlMessageP, const char** xmlResponseP);

/**
From the given system info creates a xml string that can be returned to SE.

@param responseP pointer to a char pointer where the response to the received message in xml
       format is to be copied. The caller is responsible on freeing the memory the pointer points to.

@param mcVersionTag version of Mobicore version info
@param mcVersionP pointer to the mobicore version struct
@param osSpecificInfoP pointer to struct containing operating system specific information

@return ROOTPA_OK on success, ROOTPA_ERROR_INTERNAL if
            xmlMessageP is NULL or if the message is not valid xml
            If error is returned, the error is also included in xmlResponse
            (unless there is problem in allocating memory) and the response
            should still be returned to SE.

*/
rootpaerror_t buildXmlSystemInfo(const char** responseP, int mcVersionTag, const mcVersionInfo_t* mcVersionP, const osInfo_t* osSpecificInfoP);

/**
Builds request for trustlet installation

@param responseP pointer to a char pointer where the response to the received message in xml
       format is to be copied. The caller is responsible on freeing the memory the pointer points to.

@param data, information needed for trustlet installation


@return ROOTPA_OK on success, ROOTPA_ERROR_ILLEGAL_ARGUMENT if data.dataP is NULL, responseP is NULL or if data.dataType
        has something else than the values in the type.

*/
rootpaerror_t buildXmlTrustletInstallationRequest(const char** responseP, trustletInstallationData_t data );

/**
Stores the operating system specific location of xsd files.

@param xsdpathP location of the xsd files, or place when they can be dtored if they do not yet exists
*/
void setXsdPaths(const char* xsdpathP);

#endif // XMLMESSAGEHANDLER_H
