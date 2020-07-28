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
#ifdef WIN32
	#include <cstring>
	#include <sys/stat.h>
#endif
#include <wrapper.h>
#include <stdlib.h>
#include <TlCm/tlCmUuid.h>
#include "tools.h"
#include "logging.h"
#include "trustletchannel.h"


/* keep this in global variable with file scope so that we can easily start
start using other than default device id if need arises */

static uint32_t tltChannelDeviceId=MC_DEVICE_ID_DEFAULT;

#ifdef WIN32

#define MAX_TL_FILENAME 1024

/**
Since Windows version of "mcDaemon" does not access registry, this function is used to load system TA and open session to it.
*/
mcResult_t OpenSysTaFromRegistry(
               mcSessionHandle_t * session,
               const mcUuid_t * uuid,
               uint8_t  * tci,
               uint32_t   tciLen)
  {


    size_t      taSize;
    int         result;
    struct stat fstat;
    mcResult_t  status = MC_DRV_ERR_UNKNOWN;
    uint8_t *   taBlob;
    int lastErr;


    // get registry path
    // TODO-2013-07-17-jearig01 import registry from global variable

	char registryPath[MAX_TL_FILENAME] = "C:\\Windows\\tbaseregistry\\";
	char trustedAppPath[MAX_TL_FILENAME];
	char hx[MAX_TL_FILENAME];

	 for (size_t i = 0; i < sizeof(*uuid); i++) {
        sprintf(&hx[i * 2], "%02x", ((uint8_t *)uuid)[i]);
    }

	 snprintf(trustedAppPath, sizeof(trustedAppPath), "%s%s%s", registryPath,hx, ".tlbin");

	 printf("app path--> %s\n",trustedAppPath);
     printf("registryPath path--> %s\n",registryPath);
	 printf("hx--> %s\n",hx);

    //check file
    result = stat(trustedAppPath, &fstat);
	if (result!=0) return  MC_DRV_ERR_TRUSTLET_NOT_FOUND;
    taSize = fstat.st_size;

    // import file in a blob
    FILE *infile = fopen(trustedAppPath, "rb");

    if (infile == NULL) return MC_DRV_ERR_TRUSTLET_NOT_FOUND;

    taBlob = (uint8_t *) malloc(taSize);
	if (taBlob == NULL)
    {
      fclose(infile);
      return MC_DRV_ERR_NO_FREE_MEMORY;
    }

    result = fread (taBlob, 1, taSize, infile);

	printf("FREAD--> %d - %d\n",result, taSize);
    if (result == taSize)
    {
      // Call OpenTrustlet
		printf("app path--> %d - %d\n",tciLen, taSize);
      status = mcOpenTrustlet(session, 0, taBlob, taSize, tci, tciLen);
    }

    // free blobs, necessary data are supposed to have been sent to SWd and are now useless in NWd
    fclose(infile);
    free(taBlob);

    return status;
  }
#endif

/*
Open session to content management trustlet and allocate enough memory for communication
*/
CMTHANDLE tltChannelOpen(int sizeOfWsmBuffer,  mcResult_t* result)
{
    const mcUuid_t      UUID = TL_CM_UUID;
    return taChannelOpen(sizeOfWsmBuffer, result, &UUID, NULL, 0,0);
}


/*
*/
CMTHANDLE taChannelOpen(int sizeOfWsmBuffer,  mcResult_t* result, const mcUuid_t* uuidP, uint8_t* taBinaryP, uint32_t taLength, mcSpid_t spid)
{
    CMTHANDLE           handle = (CMTHANDLE)malloc(sizeof(CMTSTRUCT));

    if (unlikely( NULL==handle ))
    {
        *result=MC_DRV_ERR_NO_FREE_MEMORY;
        return NULL;
    }

    memset(handle,0,sizeof(CMTSTRUCT));

    *result = mcOpenDevice(tltChannelDeviceId);

    if (MC_DRV_OK != *result)
    {
      LOGE("taChannelOpen: Unable to open device, error: %d", *result);
      free(handle);

      return NULL;
    }


    *result = mcMallocWsm(tltChannelDeviceId, 0, sizeOfWsmBuffer, &handle->wsmP, 0);
    if (MC_DRV_OK != *result)
    {
        LOGE("taChannelOpen: Allocation of CMP WSM failed, error: %d", *result);
        mcCloseDevice(tltChannelDeviceId);
        free(handle);
        return NULL;
    }

    if(taBinaryP!=NULL && taLength!=0)
    {
        *result = mcOpenTrustlet(&handle->session, spid, taBinaryP, taLength, handle->wsmP,(uint32_t)sizeOfWsmBuffer);
    }
    else
    {
#ifdef WIN32
        *result = OpenSysTaFromRegistry(&handle->session,uuidP,handle->wsmP,(uint32_t)sizeOfWsmBuffer);
#else
        *result = mcOpenSession(&handle->session,uuidP, handle->wsmP,(uint32_t)sizeOfWsmBuffer);
#endif
    }

    if (MC_DRV_OK != *result)
    {
        LOGE("taChannelOpen: Open session failed, error: %d", *result);
        mcFreeWsm(tltChannelDeviceId,handle->wsmP);
        mcCloseDevice(tltChannelDeviceId);
        free(handle);
        return NULL;
    }
    return handle;
}


/*
Close the communication channel and free resources
*/
void tltChannelClose(CMTHANDLE handle){
    mcResult_t          result;

    if (!bad_read_ptr(handle,sizeof(CMTSTRUCT)))
    {
        result = mcCloseSession(&handle->session);
        if (MC_DRV_OK != result)
        {
        LOGE("tltChannelClose: Closing session failed:, error: %d", result);
        }

        if (NULL!=handle->wsmP) mcFreeWsm(tltChannelDeviceId, handle->wsmP);

        result = mcCloseDevice(tltChannelDeviceId);
        if (MC_DRV_OK != result)
        {
            LOGE("tltChannelClose: Closing MobiCore device failed, error: %d", result);
        }
        free(handle);
    }
}

/*
Initiate transfer of the data between NWD and SWD. The actual data needs to be copied to wsmP beforehand
(and from it afterwards in case of response)
*/
bool tltChannelTransmit(CMTHANDLE handle, int timeout){
    if (unlikely(bad_write_ptr(handle,sizeof(CMTSTRUCT)))) return false;

    // Send CMP message to content management trustlet.

    handle->lasterror = mcNotify(&handle->session);

    if (unlikely( MC_DRV_OK!=handle->lasterror ))
    {
        LOGE("tltChannelTransmit: mcNotify failed, error: %d",handle->lasterror);
        return false;
    }

  // Wait for trustlet response.

    handle->lasterror = mcWaitNotification(&handle->session, timeout);

    if (unlikely( MC_DRV_OK!=handle->lasterror ))
    {
        LOGE("tltChannelTransmit: Wait for response notification failed, error: %d", handle->lasterror);
        return false;
    }
    return true;
}
