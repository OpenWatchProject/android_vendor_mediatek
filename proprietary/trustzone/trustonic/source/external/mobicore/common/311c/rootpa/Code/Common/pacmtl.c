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
#include "logging.h"

#include "pacmp3.h"
#include "registry.h"
#include "pacmtl.h"

#define ILLEGAL_ELEMENT 0
#define FIRST_ELEMENT 1
#define FIRST_ELEMENT_OFFSET 0

void setCmdElementInfo(uint32_t* elementNbrP, uint8_t* wsmP, uint32_t* elementOffsetP, uint32_t elementLength)
{
    cmpMapOffsetInfo_t* elementP;
    if(NULL==elementNbrP || NULL == elementOffsetP || NULL == wsmP)
    {
        LOGE("pacmtl setCmdElementInfo NULL's in input, not setting the element %ld %ld", (long int) elementNbrP, (long int) elementOffsetP);
        return;
    }

    if(ILLEGAL_ELEMENT==*elementNbrP || (*elementNbrP==FIRST_ELEMENT && *elementOffsetP != FIRST_ELEMENT_OFFSET))
    {
        LOGE("pacmtl setCmdElementInfo error in input, not setting the element %d %d", *elementNbrP, *elementOffsetP);
        return;
    }

    elementP=(&((cmpCommandHeaderTci_t*)wsmP)->cmpCmdMapOffsetInfo);
    elementP+=((*elementNbrP)-1);

    elementP->offset=*elementOffsetP;
    elementP->len=elementLength;
    (*elementNbrP)++;                  // returning number of next element
    (*elementOffsetP)+=elementLength;  // returning offset to next element
}


void setCmdMapInfo(uint8_t* wsmP, const mcBulkMap_t* mapInfoP)
{
    // mapInfo and *mapinfoP are of different type, thats why assignment instead of memcpy.

   ((cmpCommandHeaderTci_t*)wsmP)->mapInfo.addr=mapInfoP->sVirtualAddr;
   ((cmpCommandHeaderTci_t*)wsmP)->mapInfo.len=mapInfoP->sVirtualLen;
}

void setCmdCmpVersionAndCmdId(uint8_t* wsmP, cmpCommandId_t commandId)
{
    ((cmpCommandHeaderTci_t*)wsmP)->version=CMP_VERSION;
    ((cmpCommandHeaderTci_t*)wsmP)->commandId=commandId;
}

bool getRspElementInfo(uint32_t* elementNbrP, CMTHANDLE handle, uint32_t* elementOffsetP, uint32_t* elementLengthP)
{
    uint8_t* wsmP=NULL;
    cmpMapOffsetInfo_t* elementP=NULL;

    if(NULL==handle)
    {
        LOGE("pacmtl setCmdElementInfo ho handle");
        *elementLengthP=0;
        return false;
    }
    wsmP=handle->wsmP;
    LOGD(">>pacmtl getRspElementInfo %x %x %d %d %d %d", ((cmpResponseHeaderTci_t*)wsmP)->version,
                                                         ((cmpResponseHeaderTci_t*)wsmP)->responseId,
                                                         ((cmpResponseHeaderTci_t*)wsmP)->len,
                                                         *((uint32_t*)(wsmP+12)),
                                                         *((uint32_t*)(wsmP+16)),
                                                         *((uint32_t*)(wsmP+20)));
    if(NULL==elementNbrP || NULL == elementOffsetP || NULL == elementLengthP || NULL == handle->wsmP)
    {
        LOGE("pacmtl getRspElementInfo NULL's in input, not setting the element %ld %ld", (long int) elementNbrP, (long int) elementOffsetP);
        return false;
    }


    if(ILLEGAL_ELEMENT==*elementNbrP)
    {
        LOGE("pacmtl getRspElementInfo error in input (illegal element), not getting the element %d", *elementNbrP);
        *elementLengthP=0;
        return false;
    }

    elementP=(cmpMapOffsetInfo_t*)(wsmP+sizeof(cmpResponseHeaderTci_t));
    elementP+=((*elementNbrP)-1);

    if(elementP->offset+elementP->len > handle->mappedSize)
    {
        LOGE("pacmtl getRspElementInfo error in input (offset+len too big), not getting the element %d", *elementNbrP);
        *elementLengthP=0;
        return false;
    }

    *elementOffsetP=elementP->offset;
    *elementLengthP=elementP->len;
    LOGD("<<pacmtl getRspElementInfo element %d offset %d length %d", *elementNbrP, *elementOffsetP, *elementLengthP);
    (*elementNbrP)++;  // returning number of next element
    return true;
}

uint32_t getRspCmpVersion(const uint8_t* wsmP)
{
    return ((cmpResponseHeaderTci_t*)wsmP)->version;
}

uint32_t getRspCmpId(const uint8_t* wsmP)
{
    return ((cmpResponseHeaderTci_t*)wsmP)->responseId;
}


bool isValidResponse(const uint8_t* wsmP)
{

    if(NULL==wsmP)
    {
        LOGE("pacmtl isValidResponse returning false due to NULL wsmP");
        return false;
    }
    if(getRspCmpVersion(wsmP) != CMP_VERSION )
    {
        LOGE("pacmtl isValidResponse returning false due to cmpVersion 0x%x", getRspCmpVersion(wsmP));
        return false;
    }

    if(IS_RSP(getRspCmpId(wsmP)))
    {
        return true;
    }
    LOGE("pacmtl isValidResponse returning false IS_RSP %d", getRspCmpId(wsmP));
    return false;
}

bool isValidResponseTo(cmpCommandId_t commandId, const uint8_t* wsmP)
{
    LOGD(">>pacmtl isValidResponseTo %d", commandId);
    if(isValidResponse(wsmP))
    {
        if(getRspCmpId(wsmP)==RSP_ID(commandId))
        {
            LOGD("<<pacmtl isValidResponseTo returning true");
            return true;
        }
    }
    LOGE("<<pacmtl isValidResponseTo returning false");
    return false;
}

uint32_t getNeededBytesFromResponse(const uint8_t* wsmP)
{
    if(!isValidResponse(wsmP))
    {
        return -1;
    }
    return ((cmpResponseHeaderTci_t*)wsmP)->len;
}

typedef struct {
    uint32_t cmdId;
    uint32_t cmdRspSize;
    uint32_t cmdContainerSize;
    uint32_t rspContainerSize;
} cmpSizes_t;
//
// note that the container sizes are
//
static const cmpSizes_t sizeTable_[] = {
    {
        MC_CMP_CMD_AUTHENTICATE,
        sizeof(cmpMapAuthenticate_t),
        0,
        0
    },
    {
        MC_CMP_CMD_BEGIN_ROOT_AUTHENTICATION,
        sizeof(cmpMapBeginRootAuthentication_t),
        SIZEOFROOTCONTAINER,
        0
    },
    {
        MC_CMP_CMD_BEGIN_SOC_AUTHENTICATION,
        sizeof(cmpMapBeginSocAuthentication_t),
        SIZEOFAUTHTOKENCONTAINER,
        0
    },
    {
        MC_CMP_CMD_BEGIN_SP_AUTHENTICATION,
        sizeof(cmpMapBeginSpAuthentication_t),
        SIZEOFROOTCONTAINER+SIZEOFSPCONTAINER,
        0
    },
    {
        MC_CMP_CMD_GENERATE_AUTH_TOKEN,
        sizeof(cmpMapGenAuthToken_t),
        0,
        0 //SIZEOFAUTHTOKENCONTAINER
    },
    {
        MC_CMP_CMD_GET_VERSION,
        sizeof(cmpMapGetVersion_t),
        0,
        0
    },
    {
        MC_CMP_CMD_ROOT_CONT_LOCK_BY_ROOT,
        sizeof(cmpMapRootContLockByRoot_t),
        0,
        SIZEOFROOTCONTAINER
    },
    {
        MC_CMP_CMD_ROOT_CONT_REGISTER_ACTIVATE,
        sizeof(cmpMapRootContRegisterActivate_t),
        0,
        SIZEOFROOTCONTAINER
    },
    {
        MC_CMP_CMD_ROOT_CONT_UNLOCK_BY_ROOT,
        sizeof(cmpMapRootContUnlockByRoot_t),
        0,
        SIZEOFROOTCONTAINER
    },
    {
        MC_CMP_CMD_ROOT_CONT_UNREGISTER,
        sizeof(cmpMapRootContUnregister_t),
        0,
        0
    },
    {
        MC_CMP_CMD_SP_CONT_ACTIVATE,
        sizeof(cmpMapSpContActivate_t),
        0,
        SIZEOFSPCONTAINER
    },
    {
        MC_CMP_CMD_SP_CONT_LOCK_BY_ROOT,
        sizeof(cmpMapSpContLockByRoot_t),
        SIZEOFSPCONTAINER,
        SIZEOFSPCONTAINER
    },
    {
        MC_CMP_CMD_SP_CONT_LOCK_BY_SP,
        sizeof(cmpMapSpContLockBySp_t),
        0,
        SIZEOFSPCONTAINER
    },
    {
        MC_CMP_CMD_SP_CONT_REGISTER,
        sizeof(cmpMapSpContRegister_t),
        0,
        SIZEOFROOTCONTAINER+SIZEOFSPCONTAINER
    },
    {
        MC_CMP_CMD_SP_CONT_REGISTER_ACTIVATE,
        sizeof(cmpMapSpContRegisterActivate_t),
        0,
        SIZEOFROOTCONTAINER+SIZEOFSPCONTAINER
    },
    {
        MC_CMP_CMD_SP_CONT_UNLOCK_BY_ROOT,
        sizeof(cmpMapSpContUnlockByRoot_t),
        SIZEOFSPCONTAINER,
        SIZEOFSPCONTAINER
    },
    {
        MC_CMP_CMD_SP_CONT_UNLOCK_BY_SP,
        sizeof(cmpMapSpContUnlockBySp_t),
        0,
        SIZEOFSPCONTAINER
    },
    {
        MC_CMP_CMD_SP_CONT_UNREGISTER,
        sizeof(cmpMapSpContUnregister_t),
        0,
        SIZEOFROOTCONTAINER
    },
    {
        MC_CMP_CMD_TLT_CONT_ACTIVATE,
        sizeof(cmpMapTltContActivate_t),
        SIZEOFTLTCONTAINER,
        SIZEOFTLTCONTAINER
    },
    {
        MC_CMP_CMD_TLT_CONT_LOCK_BY_SP,
        sizeof(cmpMapTltContLockBySp_t),
        SIZEOFTLTCONTAINER,
        SIZEOFTLTCONTAINER
    },
    {
        MC_CMP_CMD_TLT_CONT_PERSONALIZE,
        sizeof(cmpMapTltContPersonalize_t),
        SIZEOFTLTCONTAINER,
        0
    },
    {
        MC_CMP_CMD_TLT_CONT_REGISTER,
        sizeof(cmpMapTltContRegister_t),
        0,
        SIZEOFSPCONTAINER+SIZEOFTLTCONTAINER
    },
    {
        MC_CMP_CMD_TLT_CONT_REGISTER_ACTIVATE,
        sizeof(cmpMapTltContRegisterActivate_t),
        0,
        SIZEOFSPCONTAINER+SIZEOFTLTCONTAINER
    },
    {
        MC_CMP_CMD_TLT_CONT_UNLOCK_BY_SP,
        sizeof(cmpMapTltContUnlockBySp_t),
        SIZEOFTLTCONTAINER,
        SIZEOFTLTCONTAINER
    },
    {
        MC_CMP_CMD_TLT_CONT_UNREGISTER,
        sizeof(cmpMapTltContUnregister_t),
        0,
        SIZEOFSPCONTAINER
    },
    {
        MC_CMP_CMD_GET_SUID,
        sizeof(cmpMapGetSuid_t),
        0,
        0
    },
    {
        MC_CMP_CMD_AUTHENTICATE_TERMINATE,
        sizeof(cmpMapAuthenticateTerminate_t),
        0,
        0
    }
};

const cmpSizes_t* getCmpSizeInfo(uint32_t cmdId)
{
    size_t i = 0;
    for ( i = 0; i < sizeof(sizeTable_)/sizeof(cmpSizes_t); i++)
    {
        if (cmdId == sizeTable_[i].cmdId)
        {
            return &sizeTable_[i];
        }
    }
    LOGE("getCmpSizeInfo command %d not supported", cmdId);
    return NULL;
}

uint32_t bigger(uint32_t first, uint32_t second)
{
    return (first>second?first:second);
}

uint32_t getTotalMappedBufferSize(CmpMessage* commandP)
{
    uint32_t commandSize;
    uint32_t containerSize;
    const cmpSizes_t* sizesP=getCmpSizeInfo(getCmpCommandId(commandP->contentP));
    if(NULL==sizesP) return 0;
    commandSize=bigger(sizesP->cmdRspSize, commandP->length);
    containerSize=bigger(sizesP->cmdContainerSize, sizesP->rspContainerSize);
    LOGD("pacmtl getTotalMappedBufferSize %d returning %d (%d (%d %d) %d (%d %d))", sizesP->cmdId, commandSize+containerSize,
                                                                                    commandSize, sizesP->cmdRspSize, commandP->length,
                                                                                    containerSize, sizesP->cmdContainerSize, sizesP->rspContainerSize);
    return (commandSize+containerSize);
}
