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

#ifndef ROOTPAERRORS_H
#define ROOTPAERRORS_H

#include<stdint.h>

typedef uint32_t rootpaerror_t;

/*
NOTE to the maintainer. These values and documentation needs to be in line with the ones in CommandResult.java
*/


/**
 No errors detected, successful execution.
*/
#define ROOTPA_OK                               0x00000000

/**
 Client has requested unsupported CMP command or command that it can not execute via the used interface.
 Possible steps to recover: send only supported CMP commands or update to RootPA that supports handling the particular command in the used interface.
*/
#define ROOTPA_COMMAND_NOT_SUPPORTED            0x00000001
#define STRING_ROOTPA_COMMAND_NOT_SUPPORTED    "COMMAND_NOT_SUPPORTED_ERROR"

/**
Either rootpa is locked by another client, or the client requests lock or unlock when it is not allowed to do that.
Possible steps to recover: wait until the lock is released
*/
#define ROOTPA_ERROR_LOCK                       0x00000002
#define STRING_ROOTPA_ERROR_LOCK                "BUSY_ERROR"

/**
 Error in one of the CMP commands, see command specific response for more details.
 */
#define ROOTPA_ERROR_COMMAND_EXECUTION          0x00000003
#define STRING_ROOTPA_ERROR_COMMAND_EXECUTION   "COMMAND_EXECUTION_ERROR"

/**
Registry returned an error when trying to write a container.  mcDaemon could be dead or something seriously wrong in the file system.
Possible steps to recover: rebooting the device may help
*/
#define ROOTPA_ERROR_REGISTRY                   0x00000004
#define STRING_ROOTPA_ERROR_REGISTRY            "REGISTRY_ERROR"

/**
Error in communicating with t-base secure side. This is returned when any of the mcDeamon API calls related to communication with secure side fails.
Possible steps to recover: rebooting the device may help
*/
#define ROOTPA_ERROR_MOBICORE_CONNECTION        0x00000005
#define STRING_ROOTPA_ERROR_MOBICORE_CONNECTION "MOBICORE_CONNECTION_ERROR"

/**
Either Nwd or Swd software is out of memory.
Possible steps to recover: release memory
*/
#define ROOTPA_ERROR_OUT_OF_MEMORY              0x00000006
#define STRING_ROOTPA_ERROR_OUT_OF_MEMORY       "OUT_OF_MEMORY_ERROR"

/**
Rootpa internal error. This error is returned in various situations when something unexpected went wrong e.g. message from CMTL can‘t be interpreted, SE returned an error indicating invalid data, bad request or similar or base64 decoding failed
Possible steps to recover: rebooting or updating the device may help
*/
#define ROOTPA_ERROR_INTERNAL                   0x00000007
#define STRING_ROOTPA_ERROR_INTERNAL            "INTERNAL_ERROR"

/**
Given argument is not allowed (in many cases it is NULL) or e.g. the format of xml is unsupported.
Possible steps to recover: give correct argument
*/
#define ROOTPA_ERROR_ILLEGAL_ARGUMENT           0x00000008


/**
Error in network connection or use of networking library.
Possible steps to recover: create working network connection (avoid firewalls and proxies that require password)
*/
#define ROOTPA_ERROR_NETWORK                    0x00000009


/**
Error returned by XML library. Problems in parsing received XML command or creating new XML response.
*/
#define ROOTPA_ERROR_XML                        0x0000000A
#define STRING_ROOTPA_ERROR_XML                 "XML_ERROR"

/**
Registry returned an error when trying to read a container. Most likely the container does not exist.
*/
#define ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE          0x0000000B
#define STRING_ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE   "REGISTRY_OBJECT_NOT_AVAILABLE"

/**
CMP version of the device is not supported by SE.
Possible steps to recover: use CMP version supported by SE (>=3.0)
*/
#define ROOTPA_ERROR_SE_CMP_VERSION                0x0000000C

/**
Precoditions for SP container installation are not met in SE.
Possible steps to recover: register used SPID to SE
*/
#define ROOTPA_ERROR_SE_PRECONDITION_NOT_MET        0x0000000D

/**
Requested SP container does not exist. This is not always considered an error but is used as an informative return code. As this is internal return code, user of RootPA services should never see this.
Possible steps to recover: add SP container or request container with different SPID
*/
#define ROOTPA_ERROR_INTERNAL_NO_CONTAINER      0x00000030

#endif // ROOTPAERRORS_H
