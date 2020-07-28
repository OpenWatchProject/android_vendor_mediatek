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

#include <string.h>
#include <stdlib.h>

#include <wrapper.h>
#ifdef WIN32
	#include <windows.h>
#else
	#include <pthread.h>
#endif
#include <TlCm/3.0/tlCmApi.h>
#include <MobiCoreDriverApi.h>

#include "rootpaErrors.h"
#include "logging.h"
#include "provisioningagent.h"
#include "registry.h"
#include "contentmanager.h"
#include "provisioningengine.h"
#include "xmlmessagehandler.h"
#include "seclient.h"


#define GET_VERSION_COMMAND_LENGTH 4
#define GET_SUID_COMMAND_LENGTH 4

/*
See provisioningagent.h for description of this function.
*/
rootpaerror_t executeCmpCommands(int numberOfCommands, CmpMessage* commandsP, CmpMessage* responsesP, uint32_t* internalError)
{
    LOGD("executeCmpCommands");
    return executeContentManagementCommands(numberOfCommands, commandsP, responsesP, internalError);
}

rootpaerror_t openSessionToCmtl()
{
    return openCmtlSession();
}

void closeSessionToCmtl()
{
    closeCmtlSession();
}

rootpaerror_t getVersion(int* tag, mcVersionInfo_t* versionP)
{
    rootpaerror_t ret=ROOTPA_OK;
    uint32_t internalError=0;
    CmpMessage command;
    CmpMessage response;

    LOGD(">>getVersion");
    memset(&command,0,sizeof(CmpMessage));
    memset(&response,0,sizeof(CmpMessage));

    command.length=GET_VERSION_COMMAND_LENGTH;
    command.contentP=(uint8_t*)malloc(GET_VERSION_COMMAND_LENGTH);
    if(!command.contentP)
    {
        return ROOTPA_ERROR_OUT_OF_MEMORY;
    }

    *((uint32_t*)command.contentP)=MC_CMP_CMD_GET_VERSION;
    command.hdr.ignoreError=false;

    ret=executeContentManagementCommands(1, &command, &response, &internalError);

    if(ROOTPA_OK==ret && 0 == internalError)
    {
        if(response.length != sizeof(cmpRspGetVersion_t))
        {
            ret=ROOTPA_ERROR_INTERNAL;
        }
        else
        {
            *tag=((cmpRspGetVersion_t*)(response.contentP))->tag;

            if (CMP_VERSION_TAG2 == *tag)
            {
                memcpy(versionP, &((cmpRspGetVersion_t*)(response.contentP))->data.versionData2.versionInfo, sizeof(*versionP));
            }
            else
            {
                LOGE("getVersion, unsupported version tag %d", *tag);
                ret=ROOTPA_ERROR_INTERNAL;
            }
        }
    }
    else
    {
        LOGE("getVersion, ERROR %d %d", ret, internalError);
    }
    free(response.contentP);
    free(command.contentP);
    LOGD("<<getVersion %d", ret);
    return ret;
}

rootpaerror_t getSuid(mcSuid_t* suidP)
{
    rootpaerror_t  ret=ROOTPA_OK;
    uint32_t internalError=0;
    CmpMessage command;
    CmpMessage response;

    LOGD(">>getSuid");
    memset(&command,0,sizeof(CmpMessage));
    memset(&response,0,sizeof(CmpMessage));

    command.length=GET_SUID_COMMAND_LENGTH;
    command.contentP=(uint8_t*)malloc(GET_SUID_COMMAND_LENGTH);
    if(!command.contentP)
    {
        return ROOTPA_ERROR_OUT_OF_MEMORY;
    }

    *((uint32_t*)command.contentP)=MC_CMP_CMD_GET_SUID;
    command.hdr.ignoreError=false;

    ret=executeContentManagementCommands(1, &command, &response, &internalError);

    if(ROOTPA_OK==ret && 0 == internalError)
    {
        if(response.length != sizeof(cmpRspGetSuid_t))
        {
            ret=ROOTPA_ERROR_INTERNAL;
        }
        else
        {
            memcpy(suidP, &((cmpRspGetSuid_t*)(response.contentP))->suid, sizeof(*suidP));
        }
    }
    free(response.contentP);
    free(command.contentP);
    LOGD("<<getSuid %d", ret);
    return ret;
}

rootpaerror_t  isRootContainerRegistered(bool* isRegisteredP)
{
    rootpaerror_t ret=ROOTPA_OK;
    ROOTCONTAINERP rootContP=NULL;
    uint32_t rootContSize=0;
    mcResult_t result;
    LOGD(">>isRootContainerRegistered");
    if(NULL==isRegisteredP) return ROOTPA_ERROR_ILLEGAL_ARGUMENT;

    result=regReadRoot(&rootContP, &rootContSize);

    if(MC_DRV_OK == result)
    {
        if(rootContP->cont.attribs.state != MC_CONT_STATE_UNREGISTERED)
        {
            *isRegisteredP=true;
        }
        else
        {
            *isRegisteredP=false;
        }

    }
    else if(MC_DRV_ERR_INVALID_DEVICE_FILE == result)
    {
        *isRegisteredP=false;
    }
    else
    {
        ret=ROOTPA_ERROR_REGISTRY;
    }

    free(rootContP);

    LOGD("<<isRootContainerRegistered %d", *isRegisteredP);
    return ret;
}

rootpaerror_t  isSpContainerRegistered(mcSpid_t spid, bool* isRegisteredP)
{
    rootpaerror_t ret=ROOTPA_OK;
    int state;
    LOGD(">>isSpContainerRegistered");

    if(NULL==isRegisteredP) return ROOTPA_ERROR_ILLEGAL_ARGUMENT;

    ret=getSpContainerState(spid, (mcContainerState_t*)&state);

    if(ROOTPA_OK == ret)
    {
        if(state != MC_CONT_STATE_UNREGISTERED)
        {
            *isRegisteredP=true;
        }
        else
        {
            *isRegisteredP=false;
        }
    }
    else if(ROOTPA_ERROR_INTERNAL_NO_CONTAINER == ret)
    {
        *isRegisteredP=false;
        ret=ROOTPA_OK;
    }

    LOGD("<<isSpContainerRegistered %d", *isRegisteredP);
    return ret;
}


rootpaerror_t getSpContainerState(mcSpid_t spid, mcContainerState_t* stateP)
{
    rootpaerror_t ret=ROOTPA_OK;
     mcResult_t result;

    LOGD(">>getSpContainerState");
    if(NULL==stateP) return ROOTPA_ERROR_ILLEGAL_ARGUMENT;

    result=regGetSpState(spid, stateP);

    if(MC_DRV_ERR_INVALID_DEVICE_FILE == result)
    {
        ret=ROOTPA_ERROR_INTERNAL_NO_CONTAINER; // using this since it is changed to ROOTPA_OK and state NO_CONTAINER in the wrapper.
    }
    else if (result!=MC_DRV_OK)
    {
        ret=ROOTPA_ERROR_REGISTRY;
    }

    LOGD("<<getSpContainerState %d", *stateP);
    return ret;
}

bool containerExists(mcUuid_t uuid)
{
    return (memcmp(&uuid, &MC_UUID_FREE, sizeof(mcUuid_t))!=0);
}

rootpaerror_t  getSpContainerStructure(mcSpid_t spid, SpContainerStructure* spContainerStructure)
{
    rootpaerror_t ret=ROOTPA_OK;
    SPCONTAINERP spP=NULL;
    uint32_t spContSize=0;
    mcResult_t result;
    int i;
    TLTCONTAINERP tltP=NULL;
    LOGD(">>getSpContainerStructure");

    if(NULL==spContainerStructure) return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    memset(spContainerStructure, 0xFF, sizeof(SpContainerStructure));
    spContainerStructure->nbrOfTlts=0;

    result=regReadSp(spid, &spP, &spContSize);

    if(MC_DRV_OK == result)
    {
        spContainerStructure->state=spP->cont.attribs.state;

        for(i=0; i<MC_CONT_CHILDREN_COUNT; i++)
        {
            if(containerExists(spP->cont.children[i]))
            {
                memcpy(&spContainerStructure->tltContainers[spContainerStructure->nbrOfTlts].uuid, &(spP->cont.children[i]), sizeof(mcUuid_t));

                if(ROOTPA_OK == ret)
                {
                    uint32_t tltContSize=0;
                    result=regReadTlt(&spP->cont.children[i], &tltP, &tltContSize, spid);
                    if(MC_DRV_OK == result)
                    {
                        spContainerStructure->tltContainers[spContainerStructure->nbrOfTlts].state=((mcTltContCommon_t*)(((uint8_t*)tltP)+sizeof(mcSoHeader_t)))->attribs.state;
                        spContainerStructure->nbrOfTlts++;
                    }
                    else
                    {
                        LOGE("getSpContainerStructure regReadTlt %d returned an error %d", i, result);
                        ret=ROOTPA_ERROR_REGISTRY;
                    }
                    free(tltP);
                    tltP=NULL;
                }
            }
        }
    }
    else if(MC_DRV_ERR_INVALID_DEVICE_FILE == result)
    {
        ret=ROOTPA_ERROR_INTERNAL_NO_CONTAINER; // using this since it is changed to ROOTPA_OK and state NO_CONTAINER in the wrapper.
    }
    else
    {
        ret=ROOTPA_ERROR_REGISTRY;
    }

    free(spP);
    LOGD("<<getSpContainerStructure nr: %d st: %d ret: %d",spContainerStructure->nbrOfTlts, spContainerStructure->state, ret );
    return ret;
}

rootpaerror_t storeTA(mcSpid_t spid, const mcUuid_t* uuidP, const uint8_t* taBinP,  uint32_t taBinLength)
{
    rootpaerror_t ret=ROOTPA_OK;
    mcResult_t result=0;
    mcContainerState_t state;

    result=regGetTaState(spid, uuidP, &state);
    LOGD("storeTA, TA state %d, result 0x%x", state, result);
    if(MC_DRV_ERR_INVALID_DEVICE_FILE == result)
    {
        LOGW("storeTA, not storing, since TA container is missing");
        ret=ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    }
    else if (result!=MC_DRV_OK)
    {
        LOGW("storeTA, not storing, due to TA container read error 0x%x", result);
        ret=ROOTPA_ERROR_REGISTRY;
    }
    else
    {
        result =regStoreTA(spid, uuidP, taBinP, taBinLength);

        if(result != MC_DRV_OK)
        {
            LOGE("storeTA, storing TA failed, result from registry 0x%x", result);
            ret=ROOTPA_ERROR_REGISTRY;
        }
    }

    return ret;
}

void dummyCallback(ProvisioningState state, rootpaerror_t error, tltInfo_t* tltInfoP)
{
    (void) state;
    (void) error;
    (void) tltInfoP;
    LOGD("dummy callback %d %d %ld", state, error, (long int) tltInfoP);
}

rootpaerror_t dummySysInfoCallback(osInfo_t* osSpecificInfoP)
{
    LOGD("dummy sysinfo callback %ld", (long int) osSpecificInfoP);
    if(NULL==osSpecificInfoP) return ROOTPA_ERROR_INTERNAL;
    memset(osSpecificInfoP, 0, sizeof(osInfo_t));
    return ROOTPA_OK;
}

typedef struct{
    mcSpid_t spid;
    mcSuid_t suid;
    CallbackFunctionP callbackP;
    SystemInfoCallbackFunctionP sysInfoCallbackP;
    initialRel_t      initialRel;
	trustletInstallationData_t* tltInstallationDataP;
} provisioningparams_t;
#ifdef WIN32
	void* WINAPI provisioningThreadFunction(void* paramsP)
#else
    void* provisioningThreadFunction(void* paramsP)
#endif
{
    rootpaerror_t ret=ROOTPA_OK;
    LOGD(">>provisioningThreadFunction %ld", (long int)((provisioningparams_t*)paramsP)->callbackP);
    if((ret=openCmtlSession())==ROOTPA_OK)
    {
        doProvisioningWithSe(((provisioningparams_t*)paramsP)->spid,
                             ((provisioningparams_t*)paramsP)->suid,
                             ((provisioningparams_t*)paramsP)->callbackP,
                             ((provisioningparams_t*)paramsP)->sysInfoCallbackP,
                             getVersion,
                             ((provisioningparams_t*)paramsP)->initialRel,
							 ((provisioningparams_t*)paramsP)->tltInstallationDataP);
        closeCmtlSession();
    }
    else
    {
        ((provisioningparams_t*)paramsP)->callbackP(ERROR_STATE, ret, NULL);
        LOGE("provisioningThreadFunction: was not able to open session %d", ret);
    }

    ((provisioningparams_t*)paramsP)->callbackP(PROVISIONING_STATE_THREAD_EXITING, ROOTPA_OK, NULL);
    if(((provisioningparams_t*)paramsP)->tltInstallationDataP)
    {
        free((char*)((provisioningparams_t*)paramsP)->tltInstallationDataP->dataP);
        free((char*)((provisioningparams_t*)paramsP)->tltInstallationDataP->tltPukHashP);
        free(((provisioningparams_t*)paramsP)->tltInstallationDataP);
    }
    if (paramsP != NULL) {
        free(paramsP);
        paramsP = NULL;
    }

    LOGD("<<provisioningThreadFunction");
#ifdef WIN32
	ExitThread(NULL);
#else
    pthread_exit(NULL);
#endif
    return NULL; // this is required by some compilers with some settings in order to avoid errors.
}

rootpaerror_t provision(mcSpid_t spid, CallbackFunctionP callbackP, SystemInfoCallbackFunctionP systemInfoCallbackP, trustletInstallationData_t* tltDataP, initialRel_t initialRel)
{
    provisioningparams_t* paramsP;
    rootpaerror_t ret;
#ifdef WIN32
    HANDLE provisioningThread;
    DWORD threadID;
#endif
    LOGD(">>provision %ld %ld", (long int) callbackP, (long int) dummyCallback);

    if(NULL==callbackP) callbackP=dummyCallback;
    if(NULL==systemInfoCallbackP) systemInfoCallbackP=dummySysInfoCallback;

    paramsP=(provisioningparams_t*)malloc(sizeof(provisioningparams_t));
    if(!paramsP) return ROOTPA_ERROR_OUT_OF_MEMORY;

    memset(paramsP,0,sizeof(provisioningparams_t)); // initialize in order to satisfy valgrind

    paramsP->callbackP=callbackP;
    paramsP->sysInfoCallbackP=systemInfoCallbackP;
    paramsP->spid=spid;
    if(tltDataP)
    {
		// Coverity complains that paramsP allocated here is not freed. It is done in "provisioningThreadFunction"
        paramsP->tltInstallationDataP=(trustletInstallationData_t*)malloc(sizeof(trustletInstallationData_t));
        if(!paramsP->tltInstallationDataP)
        {
            free(paramsP);
            paramsP = NULL;
            return ROOTPA_ERROR_OUT_OF_MEMORY;
        }

    // copy the whole struct

        memset(paramsP->tltInstallationDataP,0,sizeof(trustletInstallationData_t)); // initialize in order to satisfy valgrind
        memcpy(paramsP->tltInstallationDataP, tltDataP, sizeof(trustletInstallationData_t));

    // malloc and copy data from/to the pointers

        paramsP->tltInstallationDataP->dataP=(const uint8_t *)malloc(tltDataP->dataLength);
        if(!paramsP->tltInstallationDataP->dataP)
        {
            free(paramsP->tltInstallationDataP);
            free(paramsP);
            paramsP = NULL;
            return ROOTPA_ERROR_OUT_OF_MEMORY;
        }
        memset((char*)paramsP->tltInstallationDataP->dataP,0,tltDataP->dataLength); // initialize in order to satisfy valgrind
        memcpy((char*)paramsP->tltInstallationDataP->dataP, tltDataP->dataP, tltDataP->dataLength);

        paramsP->tltInstallationDataP->tltPukHashP=(const uint8_t *)malloc(tltDataP->tltPukHashLength);
        if(!paramsP->tltInstallationDataP->tltPukHashP)
        {
            free((void*) paramsP->tltInstallationDataP->dataP);
            free(paramsP->tltInstallationDataP);
            free(paramsP);
            paramsP = NULL;
            return ROOTPA_ERROR_OUT_OF_MEMORY;
        }
        memset((char*)paramsP->tltInstallationDataP->tltPukHashP,0,tltDataP->tltPukHashLength); // initialize in order to satisfy valgrind
        memcpy((char*)paramsP->tltInstallationDataP->tltPukHashP, tltDataP->tltPukHashP, tltDataP->tltPukHashLength);
    }
    else
    {
        paramsP->tltInstallationDataP=NULL;
    }

	paramsP->initialRel = initialRel;

    ret=ROOTPA_OK;
    ret=getSuid(&paramsP->suid);

    if(ROOTPA_OK==ret)
    {

#ifdef WIN32
        provisioningThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)provisioningThreadFunction, (void*) paramsP, 0, &threadID);

        if (provisioningThread == NULL)
        {
            LOGE("unable to create thread");
            if(paramsP->tltInstallationDataP)
            {
                free((void*)paramsP->tltInstallationDataP->tltPukHashP);
                free((void*)paramsP->tltInstallationDataP->dataP);
                free(paramsP->tltInstallationDataP);
            }
            free(paramsP);
            paramsP = NULL;
            ret=ROOTPA_ERROR_INTERNAL;
        }
#else
        pthread_t provisioningThread;
        pthread_attr_t attributes;
        int r=0;
        r=pthread_attr_init(&attributes);
        if(r)
        {
            LOGE("can not init thread attributes %d",r);
            if(paramsP->tltInstallationDataP)
            {
                free((void*)paramsP->tltInstallationDataP->tltPukHashP);
                free((void*)paramsP->tltInstallationDataP->dataP);
                free(paramsP->tltInstallationDataP);
            }
            free(paramsP);
            paramsP = NULL;
            ret=ROOTPA_ERROR_INTERNAL;
        }
        else
        {
            r=pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
            if(r)
            {
                LOGE("unable to set detached state, trying with defaults %d",r);
            }

            r=pthread_create(&provisioningThread, &attributes, provisioningThreadFunction, (void*) paramsP);
            if(r)
            {
                LOGE("unable to create thread %d",r);
                if(paramsP->tltInstallationDataP)
                {
                    free((void*)paramsP->tltInstallationDataP->tltPukHashP);
                    free((void*)paramsP->tltInstallationDataP->dataP);
                    free(paramsP->tltInstallationDataP);
                }
                free(paramsP);
                paramsP = NULL;
                ret=ROOTPA_ERROR_INTERNAL;
            }
            pthread_attr_destroy(&attributes);
        }
#endif
    }
    else
    {
        LOGE("provisioning can not get suid: %d",ret );
        if(paramsP->tltInstallationDataP)
        {
            free((void*)paramsP->tltInstallationDataP->tltPukHashP);
            free((void*)paramsP->tltInstallationDataP->dataP);
            free(paramsP->tltInstallationDataP);
        }
        free(paramsP);
        paramsP = NULL;
    }
    LOGD("<<provision ret: %d",ret );

    return ret;
}

rootpaerror_t doProvisioning(mcSpid_t spid, CallbackFunctionP callbackP, SystemInfoCallbackFunctionP systemInfoCallbackP)
{
    LOGD("doProvisioning");
    return provision(spid, callbackP, systemInfoCallbackP, NULL, initialRel_POST);
}

rootpaerror_t installTrustlet(mcSpid_t spid, CallbackFunctionP callbackP, SystemInfoCallbackFunctionP systemInfoCallbackP, trustletInstallationData_t* tltDataP)
{
    if(NULL == tltDataP || NULL == tltDataP->dataP || 0 == tltDataP->dataLength||
      (REQUEST_DATA_TLT != tltDataP->dataType &&  REQUEST_DATA_KEY != tltDataP->dataType)) return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    LOGD("installTrustlet");
    return provision(spid, callbackP, systemInfoCallbackP, tltDataP, initialRel_POST);
}

rootpaerror_t setSeAddress(const char* addrP, uint32_t length)
{
    return setInitialAddress(addrP, length);
}

void setPaths(const char* storageDirP, const char* certDirP)
{
    setXsdPaths(storageDirP);
    setCertPath(storageDirP, certDirP);
}

rootpaerror_t unregisterRootContainer(CallbackFunctionP callbackP, SystemInfoCallbackFunctionP systemInfoCallbackP)
{
	mcSpid_t spid;
    LOGD("unregisterRootContainer");
	memset(&spid, 0x0, sizeof(mcSpid_t));
	return provision(spid, callbackP, systemInfoCallbackP, NULL, initialRel_DELETE);
}


