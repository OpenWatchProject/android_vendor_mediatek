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

#ifndef PROVISIONINGAGENT_H
#define PROVISIONINGAGENT_H
#ifdef __cplusplus
extern "C" {
#endif


#include <wrapper.h>
#include <TlCm/3.0/cmp.h>
#include <mcVersionInfo.h>

#include "rootpaErrors.h"
#include "rootpa.h"

/**
since the CMP commands that require authentication need to be executed during
the same session the actual authentication, the client needs to handle opening
and closing the session before
*/
rootpaerror_t openSessionToCmtl();
void closeSessionToCmtl();

/**
Executes all given content management protocol commands in the order they are given and returns response to all of them.

The calling operating system specific part has to take care that no other calls are executed before
executeCmpCommands has exited.

@param numberOfCommands number of commands given in this request. The array of
                        commands and responses must be allocated with the same
                        number of CmpMessage structs.
@param commandsP an array of commands to be executed. The commands will be executed in the given order.
@param responsesP an array of responses that have to be empty when the call is made.
                  Memory for the responses need to be freed (with free) by the caller,
                  after the call.
@param internalError if returning an error, rootPA copies here error code it received from Cmtl or MC.
@return one of the return values defined in rootpaErrors.h ROOTPA_OK in case the call is successful.
        Note that when ROOTPA_ERROR_COMMAND_EXECUTION is returned, execution of some of the commands
        may have been successful. The status of individual commands can be checked from the actual
        content of the individual response.
*/
rootpaerror_t executeCmpCommands(int numberOfCommands, CmpMessage* commandsP, CmpMessage* responsesP, uint32_t* internalError);


/**
Obtains and returns version information from CMTL

The calling operating system specific part has to take care that no other calls are executed before
the command has exited.

@param tag version of the version. See mcVersionInfo_t for more information.
@param versionP version information. In case version info tag is 1, the version
               is written in the first four bytes of mcVersionInfo_t.productId

@return one of the return values defined in rootpaErrors.h ROOTPA_OK in case the call is successful.

*/
rootpaerror_t getVersion(int* tag, mcVersionInfo_t* versionP);

/**
Returns SUID

The calling operating system specific part has to take care that no other calls are executed before
the command has exited.

@param suidP pointer to the emory area where the suid is copied to
@return one of the return values defined in rootpaErrors.h ROOTPA_OK in case the call is successful.

*/
rootpaerror_t getSuid(mcSuid_t* suidP);

/**

@param isRegisteredP writes here true if the container is registered, false otherwise
@return one of the return values defined in rootpaErrors.h ROOTPA_OK in case the call is successful.

*/
rootpaerror_t isRootContainerRegistered(bool* isRegisteredP);

/**


@param spid service provider id
@param isRegisteredP writes here true if the container is registered, false otherwise
@return one of the return values defined in rootpaErrors.h ROOTPA_OK in case the call is successful.

*/
rootpaerror_t isSpContainerRegistered(mcSpid_t spid, bool* isRegisteredP);

/**


@param spid service provider id
@param stateP writes here the state of the container
@return one of the return values defined in rootpaErrors.h ROOTPA_OK in case the call is successful,
        ROOTPA_ERROR_INTERNAL_NO_CONTAINER if the container does not exist.

*/
rootpaerror_t getSpContainerState(mcSpid_t spid, mcContainerState_t* stateP);


/**

@param spid service provider id
@param spContainerStructureP writes here the structure of the container. The structure must be allocated before the call.
@return one of the return values defined in rootpaErrors.h ROOTPA_OK in case the call is successful,
        ROOTPA_ERROR_INTERNAL_NO_CONTAINER if the container does not exist.

*/
rootpaerror_t getSpContainerStructure(mcSpid_t spid, SpContainerStructure* spContainerStructureP);

/**
Creates a thread and returns, the thread contacts SE and executes the commands received from SE.

The state of the execution is informed in the calls to callback. The last callback, just before
the thread exits contains always state PROVISIONING_STATE_THREAD_EXITING.

The calling operating system specific part has to take care that no other calls are executed before
doProvisioning and the actual provisioining thread have exited.

@param spid service provider id

@param callbackP callback function that handles information delivery to operating system specific client.
       This is called at different states of provisioining (see type ProvisioningState to find out more
       about the states). Since doProvisioining executes it's own thread the callback function has to be
       thread safe.

@param systemInfoCallbackP pointer to a function that can provide RootPA system information
       that is only available in the operting system specific part. Since doProvisioining executes it's own thread the
       callback function has to be thread safe.


@return ROOTPA_OK on success and and error code if thread creation fails.  The results of actual execution of
the provisioining are returned in the callback functions.
*/
rootpaerror_t doProvisioning(mcSpid_t spid, CallbackFunctionP callbackP, SystemInfoCallbackFunctionP systemInfoCallbackP);

/**
Creates a thread and returns, the thread contacts SE and executes the commands received from SE.
This is similar to do provisioning but takes in information on trustlet to be installed and asks SE
to "install the trustlet". This is used for testing and developer trustlet installation.

The state of the execution is informed in the calls to callback. The last callback, just before
the thread exits contains always state PROVISIONING_STATE_THREAD_EXITING.

The calling operating system specific part has to take care that no other calls are executed before
doProvisioning and the actual provisioining thread have exited.

@param spid service provider id

@param callbackP callback function that handles information delivery to operating system specific client.
       This is called at different states of provisioining (see type ProvisioningState to find out more
       about the states). Since doProvisioining executes it's own thread the callback function has to be
       thread safe.

@param systemInfoCallbackP pointer to a function that can provide RootPA system information
       that is only available in the operting system specific part. Since doProvisioining executes it's own thread the
       callback function has to be thread safe.

@param dataP pointer to the data needed in trutlet installation

@return ROOTPA_OK on success and and error code if thread creation fails. ROOTPA_ERROR_ILLEGAL_ARGUMENT if dataP is NULL.
The results of actual execution of the provisioining are returned in the callback functions.
*/
rootpaerror_t installTrustlet(mcSpid_t spid, CallbackFunctionP callbackP, SystemInfoCallbackFunctionP systemInfoCallbackP, trustletInstallationData_t* dataP);


/**
This is helper function for unregistering root container.

@param callbackP callback function that handles information delivery to operating system specific client.
       This is called at different states of provisioining (see type ProvisioningState to find out more
       about the states). Since doProvisioining executes it's own thread the callback function has to be
       thread safe.

@param systemInfoCallbackP pointer to a function that can provide RootPA system information
       that is only available in the operating system specific part. Since doProvisioining executes it's own thread the
       callback function has to be thread safe.

@return ROOTPA_OK is unregistering root container succeeds, an error code otherwise
*/
rootpaerror_t unregisterRootContainer(CallbackFunctionP callbackP, SystemInfoCallbackFunctionP systemInfoCallbackP);


/**
Store's the GP TA binary to the registry. The corresponding TA container has to exists and contain correct information for decrypting the TA.

@param spid service provider ID
@param uuidP pointer to the UUID of the TA binary. This is the UUID that all t-base TA's have, NOT the UUID specific to GP TA's
@param taBinP pointer to the actual TA binary
@param taBinLength size of the actual TA binary

@return ROOTPA_OK is unregistering root container succeeds, an error code otherwise
*/
rootpaerror_t storeTA(mcSpid_t spid, const mcUuid_t* uuidP, const uint8_t* taBinP, uint32_t taBinLength);

/**
This is helper function for the platform dependent part to inform the platform independent part
on the file storage location

@param storageDirP NULL terminated char array containing the path to the storage location
@param certDirP NULL terminated char array containing the path to the location where ssl should look for ce certificates.
  note that since the certificates are also hardcoded, it is possible that this path is not used, however it must be given anyway
*/

void setPaths(const char* storageDirP, const char* certDirP);

/**
This is helper function for setting SE address.

@param addrP pointer to the address, it can but does not need to be null terminated. The address needs
       to begin with "http(s)://" and end with "/".
@param length length of the address
@return ROOTPA_OK is setting succeeded, an error code otherwise
*/
rootpaerror_t setSeAddress(const char* addrP, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif // PROVISIONINGAGENT_H
