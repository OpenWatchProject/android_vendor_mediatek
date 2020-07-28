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
#include <TlCm/3.0/cmpMap.h>
#include <TlCm/tlCmApiCommon.h>

#include "tools.h"
#include "logging.h"
#include "rootpaErrors.h"
#include "pacmtl.h"
#include "pacmp3.h"
#include "registry.h"

static mcSpid_t spid_;
static mcUuid_t tltUuid_;
static CallbackFunctionP callbackP_=NULL;

void setCallbackP(CallbackFunctionP callbackP)
{
    callbackP_=callbackP;
}

// recovery from factory reset
bool factoryResetAssumed()
{
    uint32_t contSize=0;
    AUTHTOKENCONTAINERP authTokenP=NULL;
    ROOTCONTAINERP containerP=NULL;
    mcResult_t result1=MC_DRV_OK;
    mcResult_t result2=MC_DRV_OK;

    if((result1=regReadAuthToken(&authTokenP, &contSize))==MC_DRV_OK)
    {
        free(authTokenP);
        return false;
    }
    free(authTokenP);

    if((result2=regReadRoot(&containerP, &contSize))==MC_DRV_OK)
    {
        free(containerP);
        return false;
    }
    free(containerP);

    // if neither root container, nor auth token container exists, we assume that factory reset has been performed.
    if(MC_DRV_ERR_INVALID_DEVICE_FILE==result1 && MC_DRV_ERR_INVALID_DEVICE_FILE==result2)
    {
        LOGD("factoryResetAssumed returning true");
        return true;
    }

    return false;
}
// recovery from factory reset

/*
*/
uint32_t sizeOfCmp()
{
    return (sizeof(cmp_t)); // could also use CMP_SIZE, but this way we only allocate the amount we really need
}

cmpCommandId_t getCmpCommandId(const uint8_t* commandP)
{
    if(NULL==commandP) return 0xFFFFFFFF;
    return ((cmpCommandHeaderMap_t*)commandP)->commandId;
}


uint32_t getCmpReturnCode(const uint8_t* cmpMsgP)
{
    return ((cmpResponseHeader_t*)cmpMsgP)->returnCode;
}

rootpaerror_t allocateResponseBuffer(CmpMessage* responseP, CMTHANDLE handle )
{
    uint32_t elementIndex=1;
    uint32_t offset=0;

    if(!getRspElementInfo(&elementIndex, handle, &offset, &(responseP->length)))
    {
        return ROOTPA_ERROR_INTERNAL;
    }
    LOGD("allocateResponseBuffer, size %d", responseP->length);
    responseP->contentP=(uint8_t*)malloc(responseP->length);
    if(responseP->contentP==NULL) return ROOTPA_ERROR_OUT_OF_MEMORY;
    return ROOTPA_OK;
}

bool ensureMappedBufferSize(CMTHANDLE handle, uint32_t neededSize)
{
    if( neededSize > handle->mappedSize)
    {
        uint8_t* newMappedP = (uint8_t*)realloc(handle->mappedP, neededSize);
        if(!newMappedP)
        {
            LOGE("ensureMappedBufferSize, unable to allocate more memory %d", neededSize);
            return false;
        }
        handle->mappedP = newMappedP;
    }
    return true;
}

rootpaerror_t addAuthTokenContainer(uint32_t* indexP, uint32_t* offsetP, CMTHANDLE handle, mcResult_t* mcRetP)
{
    rootpaerror_t ret=ROOTPA_ERROR_OUT_OF_MEMORY;
    AUTHTOKENCONTAINERP authTokenP = NULL;
    uint32_t contSize=0;

    if((*mcRetP=regReadAuthToken(&authTokenP, &contSize))==MC_DRV_OK)
    {
        if(ensureMappedBufferSize(handle, (*offsetP) + contSize))
        {
            memcpy(handle->mappedP+(*offsetP), authTokenP, contSize);
            setCmdElementInfo(indexP, handle->wsmP, offsetP, contSize);
            ret=ROOTPA_OK;
        }
    }
    else if (MC_DRV_ERR_INVALID_DEVICE_FILE==*mcRetP)
    {
        ret=ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE;
    }
    else
    {
        ret=ROOTPA_ERROR_REGISTRY;
    }
    free(authTokenP);
    return ret;
}

rootpaerror_t addRootContainer(uint32_t* indexP, uint32_t* offsetP, CMTHANDLE handle, mcResult_t* mcRetP)
{
    rootpaerror_t ret=ROOTPA_ERROR_OUT_OF_MEMORY;
    ROOTCONTAINERP rootP = NULL;
    uint32_t contSize=0;

    if((*mcRetP=regReadRoot(&rootP, &contSize))==MC_DRV_OK)
    {
        if(ensureMappedBufferSize(handle, (*offsetP) + contSize))
        {
            memcpy(handle->mappedP+(*offsetP), rootP, contSize);
            setCmdElementInfo(indexP, handle->wsmP, offsetP, contSize);
            ret=ROOTPA_OK;
        }
    }
    else if (MC_DRV_ERR_INVALID_DEVICE_FILE==*mcRetP)
    {
        ret=ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE;
    }
    else
    {
        ret=ROOTPA_ERROR_REGISTRY;
    }
    free(rootP);
    return ret;
}

rootpaerror_t addSpContainer(uint32_t* indexP, uint32_t* offsetP, mcSpid_t spid, CMTHANDLE handle, mcResult_t* mcRetP)
{
    rootpaerror_t ret=ROOTPA_ERROR_OUT_OF_MEMORY;
    SPCONTAINERP spP = NULL;
    uint32_t contSize=0;

    if((*mcRetP=regReadSp(spid, &spP, &contSize))==MC_DRV_OK)
    {
        if(ensureMappedBufferSize(handle, (*offsetP) + contSize))
        {
            memcpy(handle->mappedP+(*offsetP),spP,contSize);
            setCmdElementInfo(indexP, handle->wsmP, offsetP, contSize);
            ret=ROOTPA_OK;
        }
    }
    else if (MC_DRV_ERR_INVALID_DEVICE_FILE==*mcRetP)
    {
        ret=ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE;
    }
    else
    {
        ret=ROOTPA_ERROR_REGISTRY;
    }
    free(spP);
    return ret;
}


rootpaerror_t addTltContainer(uint32_t* indexP, uint32_t* offsetP, const mcUuid_t* uuidP, mcSpid_t spid, CMTHANDLE handle, mcResult_t* mcRetP)
{
    rootpaerror_t ret=ROOTPA_ERROR_OUT_OF_MEMORY;
    TLTCONTAINERP tltP = NULL;
    uint32_t contSize=0;

    if((*mcRetP=regReadTlt(uuidP, &tltP, &contSize, spid))==MC_DRV_OK)
    {
        if(ensureMappedBufferSize(handle, (*offsetP) + contSize))
        {
            memcpy(handle->mappedP+(*offsetP),tltP,contSize);
            setCmdElementInfo(indexP, handle->wsmP, offsetP, contSize);
            ret=ROOTPA_OK;
        }
    }
    else if (MC_DRV_ERR_INVALID_DEVICE_FILE==*mcRetP)
    {
        ret=ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE;
    }
    else
    {
        ret=ROOTPA_ERROR_REGISTRY;
    }

    free(tltP);
    return ret;
}


rootpaerror_t prepareCommand(cmpCommandId_t commandId, CmpMessage* inCommandP,  CMTHANDLE handle, CmpMessage* responseP)
{
    uint8_t* outCommandP =handle->mappedP;

    uint32_t offset=0;
    uint32_t elementIndex=1;
    rootpaerror_t ret=ROOTPA_OK;
    mcResult_t mcRet=MC_DRV_OK;
    LOGI("prepareCommand command id %d length %d", commandId, inCommandP->length);  // this is LOGI level on purpose to indicate that CMP command has reached RootPA

    memset(handle->wsmP,0,sizeOfCmp());

    setCmdMapInfo(handle->wsmP, &handle->mapInfo);
    setCmdCmpVersionAndCmdId(handle->wsmP, commandId);
    setCmdElementInfo(&elementIndex, handle->wsmP, &offset, inCommandP->length);
    if(ensureMappedBufferSize(handle, inCommandP->length))
    {
        memcpy(handle->mappedP, inCommandP->contentP, inCommandP->length);
    }
    else
    {
        responseP->hdr.ret=ROOTPA_ERROR_OUT_OF_MEMORY;
        return ROOTPA_ERROR_OUT_OF_MEMORY;
    }
    switch(commandId)
    {
        case MC_CMP_CMD_BEGIN_SOC_AUTHENTICATION:
            if (callbackP_) callbackP_(AUTHENTICATING_SOC, ROOTPA_OK, NULL);
            ret=addAuthTokenContainer(&elementIndex, &offset, handle, &mcRet);
            break;

        case MC_CMP_CMD_BEGIN_ROOT_AUTHENTICATION:
            if (callbackP_) callbackP_(AUTHENTICATING_ROOT, ROOTPA_OK, NULL);
            ret=addRootContainer(&elementIndex, &offset, handle, &mcRet);
            break;

        case MC_CMP_CMD_BEGIN_SP_AUTHENTICATION:
            ret=addRootContainer(&elementIndex, &offset, handle, &mcRet);
            if(ROOTPA_OK==ret)
            {
                mcSpid_t spid=((cmpCmdBeginSpAuthentication_t*)outCommandP)->cmd.spid;
                ret=addSpContainer(&elementIndex, &offset, spid, handle, &mcRet);
            }
            break;
        case MC_CMP_CMD_ROOT_CONT_REGISTER_ACTIVATE:
            if (callbackP_) callbackP_(CREATING_ROOT_CONTAINER, ROOTPA_OK, NULL);
            break;

        case MC_CMP_CMD_ROOT_CONT_UNREGISTER:
            if (callbackP_) callbackP_(UNREGISTERING_ROOT_CONTAINER, ROOTPA_OK, NULL);
            break;

        case MC_CMP_CMD_SP_CONT_ACTIVATE:
            spid_=((cmpCmdSpContActivate_t*)outCommandP)->cmd.sdata.spid;
            break;
        case MC_CMP_CMD_SP_CONT_LOCK_BY_ROOT:
            spid_=((cmpCmdSpContLockByRoot_t*)outCommandP)->cmd.sdata.spid;
            ret=addSpContainer(&elementIndex, &offset, spid_, handle, &mcRet);
            break;
        case MC_CMP_CMD_SP_CONT_LOCK_BY_SP :
            spid_=((cmpCmdSpContLockBySp_t*)outCommandP)->cmd.sdata.spid;
            break;
        case MC_CMP_CMD_SP_CONT_REGISTER:
            if (callbackP_) callbackP_(CREATING_SP_CONTAINER, ROOTPA_OK, NULL);
            spid_=((cmpCmdSpContRegister_t*)outCommandP)->cmd.sdata.spid;
            break;
        case MC_CMP_CMD_SP_CONT_REGISTER_ACTIVATE :
            if (callbackP_) callbackP_(CREATING_SP_CONTAINER, ROOTPA_OK, NULL);
            spid_=((cmpCmdSpContRegister_t*)outCommandP)->cmd.sdata.spid;
            break;
        case MC_CMP_CMD_SP_CONT_UNLOCK_BY_ROOT:
            spid_=((cmpCmdSpContUnlockByRoot_t*)outCommandP)->cmd.sdata.spid;
            ret=addSpContainer(&elementIndex, &offset, spid_, handle, &mcRet);
            break;
        case MC_CMP_CMD_SP_CONT_UNLOCK_BY_SP :
            spid_=((cmpCmdSpContUnlockBySp_t*)outCommandP)->cmd.sdata.spid;
            break;
        case MC_CMP_CMD_SP_CONT_UNREGISTER:
            spid_=((cmpCmdSpContUnregister_t*)outCommandP)->cmd.sdata.spid;
            break;
        case MC_CMP_CMD_TLT_CONT_ACTIVATE:
            spid_=((cmpCmdTltContActivate_t*)outCommandP)->cmd.sdata.spid;
            memcpy(&tltUuid_,&((cmpCmdTltContActivate_t*)outCommandP)->cmd.sdata.uuid, sizeof(mcUuid_t));
            ret=addTltContainer(&elementIndex, &offset, &tltUuid_, spid_, handle, &mcRet);
            break;
        case MC_CMP_CMD_TLT_CONT_LOCK_BY_SP:
            spid_=((cmpCmdTltContLockBySp_t*)outCommandP)->cmd.sdata.spid;
            memcpy(&tltUuid_,&((cmpCmdTltContLockBySp_t*)outCommandP)->cmd.sdata.uuid, sizeof(mcUuid_t));
            ret=addTltContainer(&elementIndex, &offset, &tltUuid_, spid_, handle, &mcRet);
            break;
        case MC_CMP_CMD_TLT_CONT_PERSONALIZE:
            ret=addTltContainer(&elementIndex, &offset, &((cmpCmdTltContPersonalize_t*)outCommandP)->cmd.sdata.uuid,
                                                        ((cmpCmdTltContPersonalize_t*)outCommandP)->cmd.sdata.spid,
                                                        handle, &mcRet);
            break;
        case MC_CMP_CMD_TLT_CONT_REGISTER:
            spid_=((cmpCmdTltContRegister_t*)outCommandP)->cmd.sdata.spid;
            memcpy(&tltUuid_,&((cmpCmdTltContRegister_t*)outCommandP)->cmd.sdata.uuid, sizeof(mcUuid_t));
            break;
        case MC_CMP_CMD_TLT_CONT_REGISTER_ACTIVATE:
            spid_=((cmpCmdTltContRegisterActivate_t*)outCommandP)->cmd.sdata.spid;
            memcpy(&tltUuid_,&((cmpCmdTltContRegisterActivate_t*)outCommandP)->cmd.sdata.uuid, sizeof(mcUuid_t));
            break;
        case MC_CMP_CMD_TLT_CONT_UNLOCK_BY_SP:
            spid_=((cmpCmdTltContUnlockBySp_t*)outCommandP)->cmd.sdata.spid;
            memcpy(&tltUuid_,&((cmpCmdTltContUnlockBySp_t*)outCommandP)->cmd.sdata.uuid, sizeof(mcUuid_t));
            ret=addTltContainer(&elementIndex, &offset, &tltUuid_, spid_, handle, &mcRet);
            break;
        case MC_CMP_CMD_TLT_CONT_UNREGISTER:
            spid_=((cmpCmdTltContUnlockBySp_t*)outCommandP)->cmd.sdata.spid;
            memcpy(&tltUuid_,&((cmpCmdTltContUnlockBySp_t*)outCommandP)->cmd.sdata.uuid, sizeof(mcUuid_t));
            break;
        default:
            // nothing extra to do, just return ret at the end of function
            break;

    }
    responseP->hdr.ret=ret;
    responseP->hdr.intRet=mcRet;
    return ret;
}


mcResult_t storeContainers(cmpCommandId_t commandId, CMTHANDLE handle, uint32_t elementIndex, uint32_t  offset)
{
    mcResult_t mcRet=MC_DRV_OK;
    uint32_t length=0;
    LOGD(">>pacmp3 storeContainers for %d element %d offset %d", commandId, elementIndex, offset);

// store the containers when needed
    switch(commandId)
    {
        case MC_CMP_CMD_GENERATE_AUTH_TOKEN:
            if(getRspElementInfo(&elementIndex, handle, &offset, &length))
            {
                mcRet=regWriteAuthToken((AUTHTOKENCONTAINERP) (handle->mappedP+offset), length);
            }
            else
            {
                mcRet=-1;
            }
            break;

        case MC_CMP_CMD_ROOT_CONT_REGISTER_ACTIVATE:
            if(getRspElementInfo(&elementIndex, handle, &offset, &length))
            {
                mcRet=regWriteRoot((ROOTCONTAINERP) (handle->mappedP+offset), length);
            }
            else
            {
                mcRet=-1;
            }

            if(MC_DRV_OK==mcRet)
            {
                mcSoAuthTokenCont_t* authTokenP=NULL;
                uint32_t authTokenSize=0;

                mcRet=regReadAuthToken(&authTokenP, &authTokenSize);
                if(mcRet!=MC_DRV_OK)
                {
                    LOGE("pacmp3 storeContainers for %d regReadAuthToken failed %d, since this was only precaution, continuing", commandId, mcRet);
                }
                mcRet=regDeleteAuthToken();
                if(mcRet!=MC_DRV_OK)
                {
                    LOGE("pacmp3 storeContainers for %d regDeleteAuthToken failed %d, trying to recover", commandId, mcRet);
                    // try to recover, remove root, but only if there is auth token stored
                    if(authTokenP)
                    {
                        mcRet=regWriteAuthToken((AUTHTOKENCONTAINERP) authTokenP, authTokenSize); // trying to be failsafe here. Deleting failed but rewriting the token anyway
                        if(MC_DRV_OK==mcRet)
                        {
                            regCleanupRoot(); // since we were able to restore authToken we delete root (due to an error in registry handling)
                        }
                    }
                }
                free(authTokenP);
            }
            else
            {
                LOGE("pacmp3 storeContainers for %d regWriteRoot failed %d", commandId, mcRet);
            }
            break;


        case MC_CMP_CMD_ROOT_CONT_LOCK_BY_ROOT:
        case MC_CMP_CMD_ROOT_CONT_UNLOCK_BY_ROOT:
            if(getRspElementInfo(&elementIndex, handle, &offset, &length))
            {
                mcRet=regWriteRoot((ROOTCONTAINERP) (handle->mappedP+offset), length);
            }
            else
            {
                mcRet=-1;
            }

            break;

        case MC_CMP_CMD_ROOT_CONT_UNREGISTER:
            mcRet=regCleanupRoot();
            break;

        case MC_CMP_CMD_SP_CONT_UNLOCK_BY_ROOT:
        case MC_CMP_CMD_SP_CONT_UNLOCK_BY_SP:
        case MC_CMP_CMD_SP_CONT_ACTIVATE:
        case MC_CMP_CMD_SP_CONT_LOCK_BY_ROOT:
        case MC_CMP_CMD_SP_CONT_LOCK_BY_SP:
            if(getRspElementInfo(&elementIndex, handle, &offset, &length))
            {
                mcRet=regWriteSp(spid_, (SPCONTAINERP) (handle->mappedP+offset), length);
            }
            else
            {
                mcRet=-1;
            }

            break;

        case MC_CMP_CMD_SP_CONT_REGISTER:
        case MC_CMP_CMD_SP_CONT_REGISTER_ACTIVATE:
        {
            // Root container is in the buffer first, that is why we read it first
            // we write it last since if SP container writing fails we do not want
            // to write root container
            uint32_t rootLength=0;
            ROOTCONTAINERP rootP=NULL;
            if(getRspElementInfo(&elementIndex, handle, &offset, &rootLength))
            {
                rootP=(ROOTCONTAINERP) (handle->mappedP+offset);
                if(getRspElementInfo(&elementIndex, handle, &offset, &length))
                {
                    mcRet=regWriteSp(spid_, (SPCONTAINERP) (handle->mappedP+offset), length);
                }
                else
                {
                    mcRet=-1;
                }
            }
            else
            {
                mcRet=-1;
            }

            if(MC_DRV_OK==mcRet)
            {
                mcRet=regWriteRoot(rootP, rootLength);
            }
            else
            {
                LOGE("pacmp3 storeContainers for %d regWriteSp failed %d", commandId, mcRet);
            }
            break;

        }
        case MC_CMP_CMD_SP_CONT_UNREGISTER:
            mcRet=regCleanupSp(spid_);
            if(MC_DRV_OK!=mcRet)
            {
                LOGE("pacmp3 storeContainers for %d regCleanupSp failed %d, , still attempting storing root", commandId, mcRet);
            }

            if(getRspElementInfo(&elementIndex, handle, &offset, &length))
            {
                mcRet=regWriteRoot((ROOTCONTAINERP) (handle->mappedP+offset), length);
            }
            else
            {
                mcRet=-1;
            }

            break;


        case MC_CMP_CMD_TLT_CONT_REGISTER:
        case MC_CMP_CMD_TLT_CONT_REGISTER_ACTIVATE:
        {
            // SP container is in the buffer first, that is why we read it first
            // we write it last since if TLT container writing fails we do not want
            // to write SP container
            uint32_t spLength=0;
            SPCONTAINERP spP=NULL;
            if(getRspElementInfo(&elementIndex, handle, &offset, &spLength))
            {
                spP=(SPCONTAINERP) (handle->mappedP+offset);
                if(getRspElementInfo(&elementIndex, handle, &offset, &length))
                {
                    mcRet=regWriteTlt(&tltUuid_,(TLTCONTAINERP) (handle->mappedP+offset), length, spid_);
                }
                else
                {
                    mcRet=-1;
                }
            }
            else
            {
                mcRet=-1;
            }

            if(MC_DRV_OK==mcRet)
            {
                mcRet=regWriteSp(spid_, spP, spLength);
                if(MC_DRV_OK!=mcRet)
                {
                    LOGE("pacmp3 storeContainers for %d regWriteSp failed %d", commandId, mcRet);
                    regCleanupTlt(&tltUuid_, spid_);
                }
            }
            else
            {
                LOGE("pacmp3 storeContainers for %d regWriteTlt failed %d", commandId, mcRet);
            }
            break;

        }
        case MC_CMP_CMD_TLT_CONT_ACTIVATE:
        case MC_CMP_CMD_TLT_CONT_LOCK_BY_SP:
        case MC_CMP_CMD_TLT_CONT_UNLOCK_BY_SP:
            if(getRspElementInfo(&elementIndex, handle, &offset, &length))
            {
                mcRet=regWriteTlt(&tltUuid_,(TLTCONTAINERP) (handle->mappedP+offset), length, spid_);
            }
            else
            {
                mcRet=-1;
            }

            break;

        case MC_CMP_CMD_TLT_CONT_UNREGISTER:
            mcRet=regCleanupTlt(&tltUuid_, spid_);
            if(MC_DRV_OK!=mcRet)
            {
                LOGE("pacmp3 storeContainers for %d regCleanupTlt failed %d, still attempting storing sp", commandId, mcRet);
            }

            if(getRspElementInfo(&elementIndex, handle, &offset, &length))
            {
                mcRet=regWriteSp(spid_, (SPCONTAINERP) (handle->mappedP+offset), length);
                if(MC_DRV_OK!=mcRet)
                {
                    LOGE("pacmp3 storeContainers for %d regWriteSp failed %d", commandId, mcRet);
                }
            }
            else
            {
                mcRet=-1;
            }

            break;

         default:
            LOGD("pacmp3 storeContainers nothing to store");
            // nothing to do
            break;
    }
    LOGD("<<pacmp3 storeContainers %d %d", commandId, mcRet);
    return mcRet;
}
/**
handleResponse stores the container received in response and copies the response to the buffer to be returned
to the client Note that the container is not store id cmtl returned an error, but the content of the response is
returned to the client.
*/
rootpaerror_t handleResponse(cmpCommandId_t commandId, CmpMessage* outResponseP, CMTHANDLE handle)
{
    mcResult_t mcRet=MC_DRV_OK;
    rootpaerror_t ret=ROOTPA_OK;
    uint32_t elementIndex=1;
    uint32_t offset=0;
    uint32_t length=0;

    LOGD(">>handleResponse for command %d ", commandId);

    if(isValidResponseTo(commandId, handle->wsmP)==false)
    {
        LOGE("no valid response to %d", commandId);
        outResponseP->hdr.ret=ROOTPA_ERROR_COMMAND_EXECUTION;
        return ROOTPA_ERROR_COMMAND_EXECUTION;
    }

    ret=allocateResponseBuffer(outResponseP, handle);

    if(ROOTPA_OK==ret)
    {
        if(getRspElementInfo(&elementIndex, handle, &offset, &length))
        {
            memcpy(outResponseP->contentP, handle->mappedP+offset, length );
        }
        else
        {
            return ROOTPA_ERROR_INTERNAL;
        }

        if (getCmpReturnCode(handle->mappedP)!=SUCCESSFUL) // this checking is here since we want the response to be returned even in case of CMP error
        {
            LOGE("executeOneCmpCommand: command execution failed 0x%x", getCmpReturnCode(handle->mappedP));
            outResponseP->hdr.intRet=getCmpReturnCode(handle->mappedP);
            return ROOTPA_ERROR_COMMAND_EXECUTION;
        }
    }
    else
    {
        outResponseP->hdr.ret=ret;
        LOGE("executeOneCmpCommand: response buffer allocation failed %d (0x%x)", ret, handle->lasterror);
        return ret;
    }

    mcRet=storeContainers(commandId, handle, elementIndex, offset);

    if(mcRet != MC_DRV_OK)
    {
        LOGE("pacmp3 handleResponse for %d registry failed %d", commandId, mcRet);
        if((mcResult_t)-1==mcRet)
        {
            ret = ROOTPA_ERROR_INTERNAL;
        }
        else
        {
            ret = ROOTPA_ERROR_REGISTRY;
        }
        if(0==outResponseP->hdr.intRet)
        {
            outResponseP->hdr.intRet=mcRet;
        }
        outResponseP->hdr.ret=ret;
    }
    LOGD("<<handleResponse returning %d ", ret);
    return ret;
}
