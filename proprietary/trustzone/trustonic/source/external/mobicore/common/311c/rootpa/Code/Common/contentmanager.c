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
#include <MobiCoreDriverApi.h>

#include "tools.h"
#include "logging.h"
#include "pacmp3.h"
#include "pacmtl.h"
#include "registry.h"
#include "trustletchannel.h"
#include "contentmanager.h"

static CMTHANDLE handle_=NULL;

void closeCmtlSession()
{
    tltChannelClose(handle_);
    handle_=NULL;
}

rootpaerror_t openCmtlSession()
{
    mcResult_t error=0;
    rootpaerror_t ret=ROOTPA_OK;

    if(handle_)
    {
        closeCmtlSession();
    }

    handle_=tltChannelOpen(sizeOfCmp(), &error);
    if(NULL==handle_)
    {
        if(MC_DRV_ERR_NO_FREE_MEMORY==error)
        {
            ret=ROOTPA_ERROR_OUT_OF_MEMORY;
        }
        else
        {
            ret=ROOTPA_ERROR_MOBICORE_CONNECTION;
        }
    }
    return ret;
}

rootpaerror_t executeOneCmpCommand(CMTHANDLE handle, CmpMessage* commandP, CmpMessage* responseP);

rootpaerror_t executeContentManagementCommands(int numberOfCommands, CmpMessage* commandsP, CmpMessage* responsesP, uint32_t* internalError)
{
    rootpaerror_t ret=ROOTPA_OK ;
    rootpaerror_t iRet=ROOTPA_OK ;
    bool selfOpened=false;
    CMTHANDLE handle;
    LOGD(">>executeContentManagementCommands");

    *internalError=0;

    if(handle_==NULL)
    {
        // doProvisioining opens session earlier. Lock opens and closes session when called by client
        // this is for commands that do not require the client to call the lock.

        ret=openCmtlSession();
        selfOpened=true;
    }
    handle=handle_;

    if (handle)
    {
        int i;
        for(i=0; i<numberOfCommands;i++)
        {
            responsesP[i].hdr.id=commandsP[i].hdr.id; // match the id;
            responsesP[i].hdr.ignoreError=commandsP[i].hdr.ignoreError;

            if(commandsP[i].length>0)
            {
                if(((iRet=executeOneCmpCommand(handle, &commandsP[i], &responsesP[i]))!=ROOTPA_OK))
                {
                    // returning actual error in case of the command failed
                    ret=iRet;
                    if(ROOTPA_OK==responsesP[i].hdr.ret)
                    {
                        responsesP[i].hdr.ret=ret;
                    }

                    if(commandsP[i].hdr.ignoreError==false)
                    {
                        LOGE("executeContentManagementCommands, ignoreError==false, returning %d", ret);
                        return ret;
                    }
                }
            }
            else
            {
                LOGE("executeContentManagementCommands, empty command");
            }
        }

        if(ret!=ROOTPA_OK)
        {
            *internalError = handle->lasterror;
        }
    }
    else
    {
        LOGE("no handle %d", *internalError);
        if(MC_DRV_ERR_NO_FREE_MEMORY == *internalError)
        {
            ret=ROOTPA_ERROR_OUT_OF_MEMORY;
        }
        else
        {
            ret=ROOTPA_ERROR_MOBICORE_CONNECTION;
        }
    }

    if(selfOpened)
    {
        closeCmtlSession();
    }

    LOGD("<<executeContentManagementCommands %d", ret);
    return ret;
}

/**
*/
rootpaerror_t executeOneCmpCommand(CMTHANDLE handle, CmpMessage* commandP, CmpMessage* responseP)
{
    mcResult_t mcRet;
    cmpCommandId_t commandId;
    rootpaerror_t ret;
    uint32_t neededBytes;
    LOGD(">>executeOneCmpCommand");
    if (unlikely( bad_write_ptr(handle,sizeof(CMTSTRUCT))))
    {
        return ROOTPA_ERROR_INTERNAL;
    }
    if(unlikely(commandP->contentP==NULL || commandP->length< sizeof(cmpCommandId_t)))
    {
        return ROOTPA_ERROR_INTERNAL;
    }

    mcRet=MC_DRV_OK;
    commandId=getCmpCommandId(commandP->contentP);

    handle->mappedSize=getTotalMappedBufferSize(commandP);
    if(0==handle->mappedSize)
    {
        LOGE("<<executeOneCmpCommand, command %d not supported", commandId);
        return ROOTPA_COMMAND_NOT_SUPPORTED;
    }

    ret=ROOTPA_OK;
    while(true)
    {
        handle->mappedP=(uint8_t*)malloc((size_t) handle->mappedSize);
        if(NULL==handle->mappedP)
        {
            ret=ROOTPA_ERROR_OUT_OF_MEMORY;
            break;
        }
        memset(handle->mappedP, 0,handle->mappedSize);
        mcRet=mcMap(&handle->session, handle->mappedP, handle->mappedSize, &handle->mapInfo);
        if(mcRet!=MC_DRV_OK)
        {
            LOGE("executeOneCmpCommand not able to map memory %d", mcRet);
            ret=ROOTPA_ERROR_MOBICORE_CONNECTION;
            commandP->hdr.intRet=mcRet;
            responseP->hdr.intRet=mcRet;
            break;
        }

        if((ret = prepareCommand(commandId, commandP, handle, responseP))!=ROOTPA_OK)
        {
            LOGE("prepareCommand failed %d", ret);
            break;
        }

        if (unlikely( !tltChannelTransmit(handle, NOTIFICATION_WAIT_TIMEOUT_MS)))
        {
            ret=ROOTPA_ERROR_MOBICORE_CONNECTION;
            commandP->hdr.intRet=handle->lasterror;
            responseP->hdr.intRet=handle->lasterror;
            break;
        }

        neededBytes=getNeededBytesFromResponse(handle->wsmP);

        if(0==neededBytes)
        {
            break;
        }

        if((uint32_t)-1==neededBytes)
        {
            ret=ROOTPA_ERROR_MOBICORE_CONNECTION;
            break;
        }

        if(neededBytes <= handle->mappedSize)
        {
            LOGE("executeOneCmpCommand, there is something wrong. CMTL is requesting smaller buffer than we originally had. Command: %d, original %d requested %d",
	         commandId, handle->mappedSize, neededBytes);
            ret=ROOTPA_ERROR_MOBICORE_CONNECTION;
            break;
        }

        // this is Info level LOGI on purpose
        LOGI("executeOneCmpCommand, updating RootPA recommended (%d bytes was not enough for %d response, allocating %d bytes and retrying)", handle->mappedSize, commandId, neededBytes);
        mcRet=mcUnmap(&handle->session, handle->mappedP, &handle->mapInfo);
        if(mcRet!=MC_DRV_OK)
        {
            LOGE("executeOneCmpCommand not able to free mapped memory %d", mcRet);
            ret=ROOTPA_ERROR_MOBICORE_CONNECTION;
            commandP->hdr.intRet=mcRet;
            responseP->hdr.intRet=mcRet;
            break;
        }

        free(handle->mappedP);
        memset(&handle->mapInfo, 0 , sizeof(handle->mapInfo));
        handle->mappedSize=neededBytes;
    }

    if(ROOTPA_OK==ret)
    {
        ret=handleResponse(commandId, responseP, handle);
    }
    else
    {
        responseP->hdr.ret=ret;
    }
    LOGD("cleaning up mapped memory %ld",(long int) handle->mappedP);
    mcRet=mcUnmap(&handle->session, handle->mappedP, &handle->mapInfo);
    if(mcRet!=MC_DRV_OK)
    {
        LOGE("executeOneCmpCommand not able to free mapped memory %d", mcRet);
        ret=ROOTPA_ERROR_MOBICORE_CONNECTION;
    }
    LOGD("freeing mapped memory %ld", (long int) handle->mappedP);
    free(handle->mappedP);
    if(commandP->hdr.ret==ROOTPA_OK) commandP->hdr.ret=ret;
    if(responseP->hdr.ret==ROOTPA_OK) responseP->hdr.ret=ret;
    LOGD("<<executeOneCmpCommand %d %d",commandId, ret);
    return ret;
}

rootpaerror_t uploadSo(uint8_t* containerDataP, uint32_t containerLength, uint32_t* regRetP)
{
    *regRetP = regWriteAuthToken((AUTHTOKENCONTAINERP) containerDataP, containerLength);
    if( *regRetP != MC_DRV_OK)
    {
        LOGE("uploadSo regWriteAuthToken failed %d", *regRetP);
        return ROOTPA_ERROR_REGISTRY;
    }
    return ROOTPA_OK;
}
