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

#ifndef ROOTPA_H
#define ROOTPA_H
#ifdef __cplusplus
extern "C" {
#endif

#include <wrapper.h>
#include <mcUuid.h>
#include <mcContainer.h>

#include "rootpaErrors.h"

#define UUID_LENGTH 16

typedef enum {
    CONNECTING_SERVICE_ENABLER=1,
    AUTHENTICATING_SOC=2,
    CREATING_ROOT_CONTAINER=3,
    AUTHENTICATING_ROOT=4,
    CREATING_SP_CONTAINER=5,
    FINISHED_PROVISIONING=6,
    ERROR_STATE=7,
    UNREGISTERING_ROOT_CONTAINER=8,
    PROVISIONING_STATE_INSTALL_TRUSTLET=0xFEED,
    PROVISIONING_STATE_THREAD_EXITING=0xDEAD
}  ProvisioningState;

typedef struct
{
    char* imeiEsnP;      // IMEI or ESN (CDMA) code
	char* mnoP;          // network operator (based on the SIM card, not current network)
	char* brandP;
	char* manufacturerP;
	char* hardwareP;
	char* modelP;
	char* versionP;
} osInfo_t;

typedef struct
{
    uint8_t* trustletP;
    uint32_t trustletSize;
} tltInfo_t;

/**
 callback function that has to be imlemented in the os specific wrapper. RootPA calls this at various stages of
 provisioning, depending on the messages sent by SE. Note that PROVISIONING_STATE_THREAD_EXITING is always the
 last state, even if errors are retrned beforehand. This alows the wrapper to perform necessary cleanup actions
 just before the provisioning thread exists

 @param st state of the provisioning
 @param err in case the state is ERROR, this field contains error code, otherwise the value is indetermined
 @param tltInfo, in case the state is PROVISIONING_STATE_INSTALL_TRUSTLET this field contains information on
        the trustlet to be installed. The callback function has to copy the trustlet before it returns.
        In other states the field is NULL.
*/
typedef void (*CallbackFunctionP)(ProvisioningState st, rootpaerror_t err, tltInfo_t* tltInfo);

/**
 callback function for RootPA to get information on the device. The os specific part needs to reserve the memory
 to the pointers of osInfo_t with malloc, RootPA the frees it when it does not need it anymore. Possible memory
 allocation and relase for the actual osInfo_t struct is in the hands of the wrapper.
*/
typedef rootpaerror_t (*SystemInfoCallbackFunctionP)(osInfo_t* );

typedef struct
{
    /**
    used with commands, true if the execution should continue even if error is received in this command, false otherwise.
    */
    bool     ignoreError;

    /**
    possible error that occurred when executing this command
    */
    rootpaerror_t ret;

    /**
    possible internal error that occurred when executing this command
    */
    uint32_t intRet;

    /**
    used for matching reply with corresponding command in xml messages
    */
    uint32_t id;

} CommonMessage;

typedef struct
{

    /**
    length of the memory allocated in the contentP
    */
    uint32_t length;

    /**
    pointer to the actual content of the message. Care has to be taken on allocating and freeing the memory properly.
    */
    uint8_t* contentP;

    /**
    data needed to
    */
    CommonMessage hdr;

}  CmpMessage;

/**
*/
typedef struct
{
    /**
    UUID of the trustlet container
    */
    mcUuid_t uuid;

    /**
    state of the trustlet container
    */
    int state;
}  TltContainerData;

typedef struct
{
    /**
    state of the servce provider container
    */
    int state;

    /**
    number of trustlets in the container
    */
    int nbrOfTlts;

    /**
    array of trustlet containers in the service provider container. Only the number of elements indicated in nbrOfTlts are set.
    */
    TltContainerData tltContainers[MC_CONT_CHILDREN_COUNT];
}  SpContainerStructure;

typedef enum {
    REQUEST_DATA_NO_DATA=0,
    REQUEST_DATA_TLT=1,
    REQUEST_DATA_KEY=2
}TltInstallationRequestDataType;

typedef struct {
    /**
    pointer to either the trustlet binary or encryption key, depending on the request type
    */
    const uint8_t* dataP;
    /**
    length of the data pointed by the pointer
    */
    uint32_t dataLength;
    /**
    tells whether dataP points to trustlet binary (REQUEST_DATA_TLT) or encryption key (REQUEST_DATA_KEY)
    */
    TltInstallationRequestDataType dataType;
    /**
    uuid of the trustlet
    */
    mcUuid_t uuid;

    /**
    minimum version of the trustlet
    */
    uint32_t minTltVersion;

    /**
    pointer to tltPukHash
    */
    const uint8_t* tltPukHashP;

    /**
    length of data pointed by tltPukHashP
    */
    uint32_t tltPukHashLength;

    /**
    memory where the trustlet is to be loaded and executed:
    0 - if enough space is available, load the Trustlet into the internal memory, otherwise into the external memory,
    1 - internal memory,
    2 - external memory
    */
    uint32_t memoryType;

    /**
    indicates how many instances of a trustlet can be installed (run) in parallel
    */
    uint32_t numberOfInstances;

    /**
    current flags are: 1 - permanent, 2 - service has no WSW control interface, 4 - debuggable
    */
    uint32_t flags;

}trustletInstallationData_t;

#ifdef __cplusplus
}
#endif

#endif // ROOTPA_H
