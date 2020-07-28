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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wrapper.h>


#include "rootpaErrors.h"
#include "logging.h"
#include "pacmp3.h"  // for setCallbackP
#include "seclient.h"
#include "xmlmessagehandler.h"
#include "provisioningengine.h"


static const char* const SE_URL="https://se.cgbe.trustonic.com:8443/service-enabler/enrollment/"; // note that there has to be slash at the end since we are adding suid to it next


static const char* const RELATION_SELF  =      "relation/self";
static const char* const RELATION_SYSTEMINFO = "relation/system_info";
static const char* const RELATION_RESULT   =   "relation/command_result";
static const char* const RELATION_NEXT    =    "relation/next";
//static const uint8_t* const SLASH= (uint8_t*)"/";

static const char* const RELATION_INITIAL_POST="initial_post"; // this will make us to send HTTP GET, which
                                      // is the right thing to do since we do not
                                      // have any data to send to SE, this will need to be different in RootPA initiated trustet installation
static const char* const RELATION_INITIAL_DELETE="initial_delete"; // this will make us to send HTTP DELETE

#define INT_STRING_LENGTH 12 // (32 bit <= 10 decimal numbers) + "/" + trailing zero.
#define INITIAL_URL_BUFFER_LENGTH 255

static char initialUrl_[INITIAL_URL_BUFFER_LENGTH];
static CallbackFunctionP callbackP_=NULL;

void addSlashToUri(char* uriP)
{
    int uriidx;
    LOGD(">>addSlashToUri");
    uriidx=strlen(uriP);
    uriP[uriidx]='/';
    LOGD("<<addSlashToUri %s", uriP);
}

void addBytesToUri(char* uriP, uint8_t* bytes, uint32_t length, bool uuid )
{
    int uriidx=strlen(uriP);
    uint32_t i;
    uint8_t singleNumber=0;
    LOGD(">>addBytesToUri %d", length);
    for(i=0; i<length; i++)
    {
        singleNumber=(bytes[i]>>4);
        singleNumber=((singleNumber<0xA)?(singleNumber+0x30):(singleNumber+0x57));
        uriP[uriidx++]=singleNumber;
        singleNumber=(bytes[i]&0x0F);
        singleNumber=((singleNumber<0xA)?(singleNumber+0x30):(singleNumber+0x57));
        uriP[uriidx++]=singleNumber;

        if(true==uuid && (3 == i || 5 == i || 7 == i || 9 == i))
        {
            uriP[uriidx++]='-';
        }
    }
    LOGD("<<addBytesToUri %s %d", uriP, uriidx);
}

void addIntToUri(char* uriP, uint32_t addThis)
{
    char intInString[INT_STRING_LENGTH];
    memset(intInString, 0, INT_STRING_LENGTH);
    // using signed integer since this is how SE wants it
    snprintf(intInString, INT_STRING_LENGTH, "/%d", addThis);
    strncpy((uriP+strlen(uriP)), intInString, INT_STRING_LENGTH); // we have earlier made sure there is enough room in uriP, using strncpy here instead strcpy is just to avoid static analysis comments
    LOGD("add int to URI %s %d", uriP, addThis);
}

void cleanup(char** linkP, char** relP, char** commandP)
{
    if(commandP!=NULL)
    {
        free(*commandP);
        *commandP=NULL;
    }

    if(relP!=NULL)
    {
        if((*relP!=RELATION_INITIAL_POST) &&
           (*relP!=RELATION_INITIAL_DELETE)) free(*relP);
        *relP=NULL;
    }

    if(linkP!=NULL)
    {
        free(*linkP);
        *linkP=NULL;
    }
}

rootpaerror_t setInitialAddress(const char* addrP, uint32_t length)
{
    if(NULL==addrP || 0==length)
    {
        return ROOTPA_ERROR_INTERNAL;
    }

    if(INITIAL_URL_BUFFER_LENGTH < (length + 1))
    {
        return ROOTPA_ERROR_ILLEGAL_ARGUMENT;
    }
    memset(initialUrl_, 0, INITIAL_URL_BUFFER_LENGTH);
    memcpy(initialUrl_, addrP, length);
    return ROOTPA_OK;
}

bool empty(const char* zeroTerminatedArray)
{
    return(strlen(zeroTerminatedArray)==0);
}

char* createBasicLink(mcSuid_t suid)
{
    char* tmpLinkP=NULL;
    size_t urlLength=0;

    urlLength=strlen(initialUrl_) + (sizeof(mcSuid_t)*2) + (sizeof(mcSpid_t)*2) + (sizeof(mcUuid_t)*2)+6; //possible slash and end zero and four dashes
    tmpLinkP=(char*)malloc(urlLength);
    if(tmpLinkP != NULL)
    {
        memset(tmpLinkP,0,urlLength);
        strncpy(tmpLinkP, initialUrl_, urlLength);
        addBytesToUri(tmpLinkP, (uint8_t*) &suid, sizeof(suid), false);
    }
    else
    {
        LOGE("createBasicLink, out of memory");
    }
    return tmpLinkP;
}


void doProvisioningWithSe(
    mcSpid_t spid,
    mcSuid_t suid,
    CallbackFunctionP callbackP,
    SystemInfoCallbackFunctionP getSysInfoP,
    GetVersionFunctionP getVersionP,
    initialRel_t initialRel,
    trustletInstallationData_t* tltDataP)
{
    rootpaerror_t ret=ROOTPA_OK;
    rootpaerror_t tmpRet=ROOTPA_OK;
    bool workToDo = true;
    const char* linkP=NULL;
    const char* relP=NULL;
    const char* pendingLinkP=NULL;
    const char* pendingRelP=NULL;
    const char* commandP=NULL;  // "command" received from SE
    const char* responseP=NULL; // "response" to be sent to SE

    const char* usedLinkP=NULL;
    const char* usedRelP=NULL;
    const char* usedCommandP=NULL;

    LOGD(">>doProvisioningWithSe");

    callbackP_=callbackP;

    if(empty(initialUrl_))
    {
        memset(initialUrl_, 0, INITIAL_URL_BUFFER_LENGTH);
        strncpy(initialUrl_, SE_URL, strlen(SE_URL));
    }

    linkP=createBasicLink(suid);
    if(NULL==linkP)
    {
        callbackP(ERROR_STATE, ROOTPA_ERROR_OUT_OF_MEMORY, NULL);
        return;
    }
    else
    {
        LOGD("SE_ADDRESS %s", linkP);
    }

    if (initialRel == initialRel_DELETE)
    {
    	relP = RELATION_INITIAL_DELETE;
    }
    else
    {
    	relP = RELATION_INITIAL_POST;
        if(spid!=0) // SPID 0 is not legal. We use it for requesting root container creation only (no sp)
        {
            addIntToUri((char*)linkP, (uint32_t) spid);
        }
    }

    LOGD("calling first callback %ld", (long int) callbackP);
    callbackP(CONNECTING_SERVICE_ENABLER, ROOTPA_OK, NULL);

    ret=openSeClientAndInit();
    if(ROOTPA_OK!=ret)
    {
        callbackP(ERROR_STATE, ret, NULL);
        workToDo=false;
    }

    if(tltDataP != NULL) // we are installing trustlet
    {
        ret=buildXmlTrustletInstallationRequest(&responseP, *tltDataP );
        if(ROOTPA_OK!=ret || NULL==responseP)
        {
            if(ROOTPA_OK==ret) ret=ROOTPA_ERROR_XML;
            callbackP(ERROR_STATE, ret, NULL);
            workToDo=false;
        }
        else
        {
            addSlashToUri((char*) linkP);
            addBytesToUri((char*) linkP, (uint8_t*) tltDataP->uuid.value, UUID_LENGTH, true);
        }
    }

// begin recovery from factory reset 1
    if(factoryResetAssumed() && relP != RELATION_INITIAL_DELETE && workToDo == true)
    {
        pendingLinkP=linkP;
        pendingRelP=relP;
        relP=RELATION_INITIAL_DELETE;
        linkP=createBasicLink(suid);
    }
// end recovery from factory reset 1

    while(workToDo)
    {
        LOGD("in loop link: %s\nrel: %s\ncommand: %s\nresponse: %s\n", (linkP==NULL)?"null":linkP,
                                                                       (relP==NULL)?"null":relP,
                                                                       (commandP==NULL)?"null":commandP,
                                                                       (responseP==NULL)?"null":responseP);

        if(NULL==relP)
        {
// begin recovery from factory reset 2
            if(pendingLinkP!=NULL && pendingRelP!=NULL)
            {
                free((void*)linkP);
                linkP=pendingLinkP;
                relP=pendingRelP;
                pendingLinkP=NULL;
                pendingRelP=NULL;
                workToDo=true;
                continue;
            }
// end recovery from factory reset 2


            callbackP(FINISHED_PROVISIONING, ROOTPA_OK, NULL); // this is the only place where we can be sure
                                                         // SE does not want to send any more data to us
                                                         // the other option would be to keep track on the
                                                         // commands received from SE but since we want
                                                         // SE to have option to execute also other commands
                                                         // and also allow modification in provisioning sequence
                                                         // without modifying RootPA we use this simpler way.
            workToDo=false;
        }
        else if(strstr(relP, RELATION_SELF))  // do it again. So we need to restore pointer to previous stuff.
        {
            if(relP!=usedRelP && linkP!=usedLinkP && commandP!=usedCommandP)
            {
                cleanup((char**) &linkP, (char**) &relP, (char**) &commandP);
                relP=usedRelP;
                linkP=usedLinkP;
                commandP=usedCommandP;
            }
        }
        else
        {
            // store the current pointers to "used" pointers just before using them, the current ones will then be updated
            // this is to prepare for the case where we receive RELATION_SELF as next relation.
            usedLinkP=linkP;            // originally linkP
            usedRelP=relP;              // originally NULL
            usedCommandP=commandP;      // originally NULL

            if(strstr(relP, RELATION_SYSTEMINFO))
            {
                osInfo_t osSpecificInfo;
                int mcVersionTag=0;
                mcVersionInfo_t mcVersion;

#ifdef WIN32
// TODO- remove the memory allocation from here and handle it properly on C# code

                osSpecificInfo.brandP = (char*)calloc(64, sizeof(char));
                osSpecificInfo.mnoP = (char*)calloc(64, sizeof(char));
                osSpecificInfo.imeiEsnP = (char*)calloc(64, sizeof(char));
                osSpecificInfo.manufacturerP = (char*)calloc(64, sizeof(char));
                osSpecificInfo.hardwareP = (char*)calloc(64, sizeof(char));
                osSpecificInfo.modelP = (char*)calloc(64, sizeof(char));
                osSpecificInfo.versionP = (char*)calloc(64, sizeof(char));
#endif
                tmpRet=getSysInfoP(&osSpecificInfo);
                if(tmpRet!=ROOTPA_OK) ret=tmpRet;

                tmpRet=getVersionP(&mcVersionTag, &mcVersion);
                if(tmpRet!=ROOTPA_OK) ret=tmpRet;

                tmpRet=buildXmlSystemInfo(&responseP, mcVersionTag, &mcVersion, &osSpecificInfo);
                if(tmpRet!=ROOTPA_OK) ret=tmpRet;

                free(osSpecificInfo.imeiEsnP);
                free(osSpecificInfo.mnoP);
                free(osSpecificInfo.brandP);
                free(osSpecificInfo.manufacturerP);
                free(osSpecificInfo.hardwareP);
                free(osSpecificInfo.modelP);
                free(osSpecificInfo.versionP);

                if(responseP!=NULL)
                {
                    tmpRet=httpPutAndReceiveCommand(responseP, &linkP, &relP, &commandP);
                    if(tmpRet!=ROOTPA_OK) ret=tmpRet;
                }
                else if(ROOTPA_OK==ret)
                {
                    workToDo=false;
                    ret=ROOTPA_ERROR_OUT_OF_MEMORY;
                }

                if(ret!=ROOTPA_OK)
                {
                    LOGE("getSysInfoP, getVersionP or buildXmlSystemInfo or httpPutAndReceiveCommand returned an error %d", ret);
                    callbackP(ERROR_STATE, ret, NULL);
                    if(tmpRet!=ROOTPA_OK) workToDo=false; // if sending response succeeded, we rely on "relP" to tell whether we should continue or not
                }
            }
            else if(strstr(relP, RELATION_INITIAL_DELETE))
            {
                ret=httpDeleteAndReceiveCommand(&linkP, &relP, &commandP);

                if(ret!=ROOTPA_OK)
                {
                    LOGE("httpDeleteAndReceiveCommand returned an error %d", ret);
                    callbackP(ERROR_STATE, ret, NULL);
                    workToDo=false;
                }
            }
            else if(strstr(relP, RELATION_INITIAL_POST))
            {
                // response may be NULL or trustlet installation request
                ret=httpPostAndReceiveCommand(responseP, &linkP, &relP, &commandP);

                if(ret!=ROOTPA_OK)
                {
                    LOGE("httpPostAndReceiveCommand returned an error %d", ret);
                    callbackP(ERROR_STATE, ret, NULL);
                    workToDo=false;
                }
            }
            else if(strstr(relP, RELATION_RESULT))
            {
                setCallbackP(callbackP);
                ret=handleXmlMessage(commandP, &responseP);
                setCallbackP(NULL);

                if(NULL==responseP)
                {
                    if(ROOTPA_OK==ret) ret=ROOTPA_ERROR_XML;
                    // have to set these to NULL since we are not even trying to get them from SE now
                    linkP=NULL;
                    relP=NULL;
                    commandP=NULL;
                    LOGE("no responseP");
                }
                else
                {
                    // attempting to return response to SE even if there was something wrong in handleXmlMessage
                    tmpRet=httpPostAndReceiveCommand(responseP, &linkP, &relP, &commandP);
                    if(tmpRet!=ROOTPA_OK) ret=tmpRet;
                }

                if(ret!=ROOTPA_OK && ret!=ROOTPA_ERROR_REGISTRY_OBJECT_NOT_AVAILABLE) // if container is not found, not sending error intent to SP.PA since it is possible that SE can recover.
                {                                                                     // If it can not, it will return an error code anyway.
                    LOGE("httpPostAndReceiveCommand or handleXmlMessage returned an error %d %d", ret, tmpRet);
                    callbackP(ERROR_STATE, ret, NULL);
                    if(tmpRet!=ROOTPA_OK) workToDo=false; // if sending response succeeded, we rely on "relP" to tell whether we should continue or not
                }

            }
            else if(strstr(relP, RELATION_NEXT))
            {
                ret=httpGetAndReceiveCommand(&linkP, &relP, &commandP);
                if(ret!=ROOTPA_OK)
                {
                    LOGE("httpGetAndReceiveCommand returned an error %d", ret);
                    callbackP(ERROR_STATE, ret, NULL);
                    workToDo=false;
                }
            }
            else
            {
                LOGE("DO NOT UNDERSTAND REL %s", relP);
                ret=ROOTPA_ERROR_ILLEGAL_ARGUMENT;
                callbackP(ERROR_STATE, ret, NULL);
                workToDo=false;
            }

            LOGD("end of provisioning loop work to do: %d, responseP %ld", workToDo, (long int) responseP);
        }

        // last round cleaning in order to make sure both original and user pointers are released, but only once

        if(!workToDo)
        {
            LOGD("no more work to do %ld - %ld %ld - %ld %ld - %ld", (long int) linkP, (long int) usedLinkP,
                                                                     (long int) relP, (long int) usedRelP,
                                                                     (long int) commandP, (long int) usedCommandP);
            // final cleanup

            // ensure that we do not clean up twice in case used pointers opint to the original one
            if(linkP==usedLinkP) usedLinkP=NULL;
            if(relP==usedRelP) usedRelP=NULL;
            if(commandP==usedCommandP) usedCommandP=NULL;

            cleanup((char**) &linkP, (char**) &relP, (char**) &commandP);
        }

        // free the used pointers since all the necessary pointers point to new direction.
        // when relation is self we need to give the previous command again and so we keep the
        // data

        if(relP==NULL || strstr(relP, RELATION_SELF)==NULL)
        {
            cleanup((char**) &usedLinkP, (char**) &usedRelP, (char**) &usedCommandP);
        }

        // responseP can be freed at every round
        free((void*)responseP);
        responseP=NULL;

    } // while
    closeSeClientAndCleanup();

    if(responseP!=NULL) {
    	free((void*)responseP);
    	responseP = NULL;
    }

    if(linkP!=NULL){
    	free((void*)linkP);
    	linkP = NULL;
    }

    if(ROOTPA_OK != ret)  LOGE("doProvisioningWithSe had some problems: %d",ret );
    LOGD("<<doProvisioningWithSe ");

    return;
}

rootpaerror_t uploadTrustlet(uint8_t* containerDataP, uint32_t containerLength)
{
    if(callbackP_)
    {
        tltInfo_t tltInfo;
        tltInfo.trustletP = containerDataP;
        tltInfo.trustletSize = containerLength;
        callbackP_(PROVISIONING_STATE_INSTALL_TRUSTLET, ROOTPA_OK, &tltInfo);
        return ROOTPA_OK;
    }
    LOGE("uploadTrustlet, no callbackP_");
    return ROOTPA_COMMAND_NOT_SUPPORTED;
}

