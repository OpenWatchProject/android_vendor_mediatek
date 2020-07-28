/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define CPP_STL_SUPPORT 1
#if CPP_STL_SUPPORT
#include <vector>
using namespace std;
#endif
#include "Core/OMX_Core.h"
#include "Core/OMX_Component.h"

#define LOGD printf
#define LOGE printf

#define ENABLE_DATA_FLOW 0

typedef OMX_ERRORTYPE (*InitFunc)();
typedef OMX_ERRORTYPE (*DeinitFunc)();
typedef OMX_ERRORTYPE (*ComponentNameEnumFunc)(OMX_STRING, OMX_U32, OMX_U32);
typedef OMX_ERRORTYPE (*GetHandleFunc)(OMX_HANDLETYPE*, OMX_STRING, OMX_PTR, OMX_CALLBACKTYPE*);
typedef OMX_ERRORTYPE (*FreeHandleFunc)(OMX_HANDLETYPE*);
typedef OMX_ERRORTYPE (*GetRolesOfComponentFunc)(OMX_STRING, OMX_U32*, OMX_U8**);


InitFunc                             _pfnOmxInit;
DeinitFunc                         _pfnOmxDeInit;
ComponentNameEnumFunc _pfnOmxComponentNameEnum;
GetRolesOfComponentFunc _pfnGetRolesOfComponent;
GetHandleFunc                   _pfnOmxGetHandle;
FreeHandleFunc                  _pfnOmxFreeHandle;

#define INPUT_PORT_INDEX    0
#define OUTPUT_PORT_INDEX 1
#define ALL_PORT_INDEX        0xFFFFFFFF

#define NUM_INPUT_BUF    10
#define NUM_OUTPUT_BUF   8

OMX_BUFFERHEADERTYPE* pInBufHdrs[NUM_INPUT_BUF];
OMX_BUFFERHEADERTYPE* pOutBufHdrs[NUM_OUTPUT_BUF];

sem_t gEmptyBufSem;
sem_t gFillBufSem;

pthread_mutex_t gAvailInBufQLock;
pthread_mutex_t gAvailOutBufQLock;

OMX_TICKS gInputTs = 0;;

const char* StateToString(OMX_U32 state) {
    switch (state) {
        case OMX_StateInvalid:
            return "Invalid";
        case OMX_StateLoaded:
            return "OMX_StateLoaded";
        case OMX_StateIdle:
            return "OMX_StateIdle";
        case OMX_StateExecuting:
            return "OMX_StateExecuting";
        case OMX_StatePause:
            return "OMX_StatePause";
        case OMX_StateWaitForResources:
            return "OMX_StateWaitForResources";
        default:
            return "Unknown";
    }
}

#if CPP_STL_SUPPORT
vector<int> gAvailInBufQ;
vector<int> gAvailOutBufQ;
#endif

int FindMatchingBufferHdr(int port_index, OMX_BUFFERHEADERTYPE* pBufHdr) {
    if (port_index == INPUT_PORT_INDEX) {
        for (int i = 0 ; i < NUM_INPUT_BUF ; i++) {
            if (pBufHdr == pInBufHdrs[i]) {
                return i;
            }
        }
    }
    else if (port_index == OUTPUT_PORT_INDEX) {
        for (int i = 0 ; i < NUM_OUTPUT_BUF ; i++) {
            if (pBufHdr == pOutBufHdrs[i]) {
                return i;
            }
        }
    }

    return -1;
}

void* EmptyBufferThread (void* pData) {
    OMX_ERRORTYPE err;
    OMX_COMPONENTTYPE *CompHandle = (OMX_COMPONENTTYPE*)pData;

    while (ENABLE_DATA_FLOW) {
        sem_wait(&gEmptyBufSem);
        pthread_mutex_lock(&gAvailInBufQLock);
#if CPP_STL_SUPPORT
        int input_idx = *gAvailInBufQ.begin();
        gAvailInBufQ.erase(gAvailInBufQ.begin());
#endif
        pthread_mutex_unlock(&gAvailInBufQLock);
        // TODO: read input bitstream here
        pInBufHdrs[input_idx]->nTimeStamp = gInputTs;
        gInputTs+=33; // update timestamp

        pInBufHdrs[input_idx]->nFilledLen = 1024;
        err = OMX_EmptyThisBuffer(CompHandle, pInBufHdrs[input_idx]);
        if (OMX_ErrorNone != err) {
            LOGE ("OMX_EmptyThisBuffer[%d] error A (0x%08X)\r\n", input_idx, err);
        }
    }

    return NULL;
}


void* FillBufferThread (void* pData) {
    OMX_ERRORTYPE err;
    OMX_COMPONENTTYPE *CompHandle = (OMX_COMPONENTTYPE*)pData;

    while (ENABLE_DATA_FLOW) {
        sem_wait(&gFillBufSem);
        pthread_mutex_lock(&gAvailOutBufQLock);
#if CPP_STL_SUPPORT
        int output_idx = *gAvailOutBufQ.begin();
        gAvailOutBufQ.erase(gAvailOutBufQ.begin());
#endif
        pthread_mutex_unlock(&gAvailOutBufQLock);

        err = OMX_FillThisBuffer(CompHandle, pOutBufHdrs[output_idx]);
        if (OMX_ErrorNone != err) {
            LOGE ("OMX_FillThisBuffer[%d] error A (0x%08X)\r\n", output_idx, err);
        }
    }

    return NULL;
}

OMX_ERRORTYPE EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                           OMX_IN OMX_PTR pAppData,
                           OMX_IN OMX_EVENTTYPE eEvent,
                           OMX_IN OMX_U32 nData1, OMX_IN OMX_U32 nData2,
                           OMX_IN OMX_PTR pEventData)
{
    switch (eEvent) {
        case OMX_EventCmdComplete:
            if (nData1 == OMX_CommandStateSet) {
                LOGD ("OMX_EventCmdComplete [OMX_CommandStateSet: %s]\r\n", StateToString(nData2));
            }
            else if (nData1 == OMX_CommandPortDisable) {
                LOGD ("OMX_EventCmdComplete [OMX_CommandPortDisable: nPortIndex(%d)]\r\n", nData2);
            }
            else if (nData1 == OMX_CommandPortEnable) {
                LOGD ("OMX_EventCmdComplete [OMX_CommandPortEnable: nPortIndex(%d)]\r\n", nData2);
            }
            else if (nData1 == OMX_CommandFlush) {
                LOGD ("OMX_EventCmdComplete [OMX_CommandFlush: nPortIndex(%d)]\r\n", nData2);
            }
            break;

        case OMX_EventError:
            LOGD ("OMX_EventError (0x%08X)\r\n", nData1);
            break;

        default:
            break;
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
                              OMX_IN OMX_PTR pAppData,
                              OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
{
    LOGD ("EmptyBufferDone pBuffer(0x%08X)\r\n", pBuffer);
    int index = FindMatchingBufferHdr(INPUT_PORT_INDEX, pBuffer);
    if (index < 0) {
        LOGE ("Error invalid index in EmptyBufferDone\r\n");
    }
#if CPP_STL_SUPPORT
    gAvailInBufQ.push_back(index);
#endif
    sem_post(&gEmptyBufSem);
    return OMX_ErrorNone;
}

OMX_ERRORTYPE FillBufferDone(OMX_OUT OMX_HANDLETYPE hComponent,
                             OMX_OUT OMX_PTR pAppData,
                             OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    LOGD ("FillBufferDone pBuffer(0x%08X)\r\n", pBuffer);
    int index = FindMatchingBufferHdr(OUTPUT_PORT_INDEX, pBuffer);
    if (index < 0) {
        LOGE ("Error invalid index in FillBufferDone\r\n");
    }
#if CPP_STL_SUPPORT
    gAvailOutBufQ.push_back(index);
#endif
    sem_post(&gFillBufSem);
    return OMX_ErrorNone;
}

static OMX_CALLBACKTYPE call_back = {&EventHandler, &EmptyBufferDone, &FillBufferDone};


 void GetRolesOfComponent(OMX_STRING name)
 {
     LOGD ("----- GetRolesOfComponent ----- (%s)\r\n", name);
     // get component role
    // get component role step 1: check role number:
    OMX_U32 numRoles;
    OMX_ERRORTYPE err = (*_pfnGetRolesOfComponent)(const_cast<OMX_STRING>(name), &numRoles, NULL);
    if (err != OMX_ErrorNone) {
        LOGE ("_pfnGetRolesOfComponent error (%d)\r\n", err);
    }
    LOGD ("Role count of %s is %d\r\n", name, numRoles);

    // get component role step 2: get role name:
    if (numRoles == 1) {
        OMX_U8 **array = new OMX_U8 *[numRoles];
        array[0] = new OMX_U8[OMX_MAX_STRINGNAME_SIZE];

        OMX_U32 numRoles2;
        err = (*_pfnGetRolesOfComponent)(const_cast<OMX_STRING>(name), &numRoles2, array);
        LOGD ("numRoles=%d, Role[%d] is %s\r\n\r\n", numRoles2, 0, array[0]);
        delete [] array;
    }
 }


int main (int argc, char** argv)
{
    sem_init(&gEmptyBufSem, 0, 0);
    sem_init(&gFillBufSem, 0, 0);
    pthread_mutex_init(&gAvailInBufQLock, NULL);
    pthread_mutex_init(&gAvailOutBufQLock, NULL);

#if CPP_STL_SUPPORT
    gAvailInBufQ.clear();
    gAvailOutBufQ.clear();
#endif

    void* handle;
    handle = dlopen ("./libMtkOmxCore.so", RTLD_LAZY);
    if (!handle) {
        LOGE ("dlopen failed, %s\r\n", dlerror());
        return 0;
    }

    _pfnOmxInit = (InitFunc)dlsym(handle, "Mtk_OMX_Init");
    _pfnOmxDeInit = (DeinitFunc)dlsym(handle, "Mtk_OMX_Deinit");
    _pfnOmxComponentNameEnum = (ComponentNameEnumFunc)dlsym(handle, "Mtk_OMX_ComponentNameEnum");
    _pfnGetRolesOfComponent = (GetRolesOfComponentFunc)dlsym(handle, "Mtk_OMX_GetRolesOfComponent");
    _pfnOmxGetHandle = (GetHandleFunc)dlsym(handle, "Mtk_OMX_GetHandle");
    _pfnOmxFreeHandle = (FreeHandleFunc)dlsym(handle, "Mtk_OMX_FreeHandle");

// init component
    (*_pfnOmxInit)();

// enumerate all components
    OMX_U32 index = 0;
    char name[128];
    OMX_ERRORTYPE err;
    while ((err = (*_pfnOmxComponentNameEnum)(name, sizeof(name), index++)) == OMX_ErrorNone) {
        LOGD ("%s\r\n", name);
        GetRolesOfComponent(name);
    }

// get handle
    strcpy(name, "OMX.MTK.VIDEO.DECODER.AVC");
    OMX_COMPONENTTYPE *CompHandle;
    err = (*_pfnOmxGetHandle)(reinterpret_cast<OMX_HANDLETYPE *>(&CompHandle), name, NULL /*appData*/, &call_back);
    if (err != OMX_ErrorNone) {
        LOGE ("_pfnOmxGetHandle error (0x%08X)\r\n", err);
        return 0;
    }

    // create empty buffer thread
    pthread_t mEmptyBufThread;
    int ret = pthread_create (&mEmptyBufThread, NULL, &EmptyBufferThread, (void*)CompHandle);
    if (ret) {
        LOGE ("mEmptyBufThread creation failure\r\n");
        return 0;
    };

    // create fill buffer thread
    pthread_t mFillBufThread;
    ret = pthread_create (&mFillBufThread, NULL, &FillBufferThread, (void*)CompHandle);
    if (ret) {
        LOGE ("mFillBufThread creation failure\r\n");
        return 0;
    };

// component test
    OMX_STATETYPE state;
    err = OMX_GetState(CompHandle, &state);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_GetState error (0x%08X)\r\n", err);
    }
    LOGD ("OMX state = %d\r\n", state);


// set component role
    OMX_PARAM_COMPONENTROLETYPE roleParams;
    roleParams.nSize = sizeof(OMX_PARAM_COMPONENTROLETYPE);
    roleParams.nVersion.s.nVersionMajor = 1;
    roleParams.nVersion.s.nVersionMinor = 0;
    roleParams.nVersion.s.nRevision = 0;
    roleParams.nVersion.s.nStep = 0;
    strcpy((char *)roleParams.cRole, "video_decoder.avc");
    err = OMX_SetParameter(CompHandle, OMX_IndexParamStandardComponentRole, &roleParams);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SetParameter(OMX_IndexParamStandardComponentRole) error (0x%08X)\r\n", err);
    }

// check video input port format
    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    format.nVersion.s.nVersionMajor = 1;
    format.nVersion.s.nVersionMinor = 0;
    format.nVersion.s.nRevision = 0;
    format.nVersion.s.nStep = 0;

    format.nPortIndex = INPUT_PORT_INDEX; // input port
    format.nIndex = 0;

    int idx = 0;
    while (OMX_ErrorNone == OMX_GetParameter(CompHandle, OMX_IndexParamVideoPortFormat, &format)) {
        LOGD ("OMX_GetParameter OMX_IndexParamVideoPortFormat for INPUT port eCompressionFormat(0x%08X)\r\n", format.eCompressionFormat);
        format.nIndex++;
    }

    err = OMX_SetParameter(CompHandle, OMX_IndexParamVideoPortFormat, &format);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SetParameter(OMX_IndexParamVideoPortFormat) A error (0x%08X)\r\n", err);
    }

// check video output port format
    format.nPortIndex = OUTPUT_PORT_INDEX; // output port
    format.nIndex = 0;
    err = OMX_GetParameter(CompHandle, OMX_IndexParamVideoPortFormat, &format);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_GetParameter(OMX_IndexParamVideoPortFormat) B error (0x%08X)\r\n", err);
    }
     LOGD ("OMX_GetParameter OMX_IndexParamVideoPortFormat for OUTPUT port eColorFormat(0x%08X)\r\n", format.eColorFormat);
     // CHECK: format.eCompressionFormat must equal to OMX_VIDEO_CodingUnused
     LOGD ("OMX_GetParameter OMX_IndexParamVideoPortFormat for OUTPUT port eCompressionFormat(0x%08X)\r\n", format.eCompressionFormat);

    err = OMX_SetParameter(CompHandle, OMX_IndexParamVideoPortFormat, &format);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SetParameter(OMX_IndexParamVideoPortFormat) C error (0x%08X)\r\n", err);
    }


// check input port definition
    OMX_PARAM_PORTDEFINITIONTYPE def;
    def.nVersion.s.nVersionMajor = 1;
    def.nVersion.s.nVersionMinor = 0;
    def.nVersion.s.nRevision = 0;
    def.nVersion.s.nStep = 0;

    def.nPortIndex = INPUT_PORT_INDEX;  // input port
    OMX_VIDEO_PORTDEFINITIONTYPE *video_def = &def.format.video;

    err = OMX_GetParameter(CompHandle, OMX_IndexParamPortDefinition, &def);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_GetParameter(OMX_IndexParamPortDefinition) D error (0x%08X)\r\n", err);
    }
    // CHECK_EQ(def.eDomain, OMX_PortDomainVideo);
    LOGD ("InputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X), nBufferCountActual(%d)\r\n",
            def.eDomain, video_def->nFrameWidth, video_def->nFrameHeight, video_def->eCompressionFormat, def.nBufferCountActual);

    video_def->nFrameWidth = 640;
    video_def->nFrameHeight = 480;
    video_def->eCompressionFormat = OMX_VIDEO_CodingAVC;
    video_def->eColorFormat = OMX_COLOR_FormatUnused;

    err = OMX_SetParameter(CompHandle, OMX_IndexParamPortDefinition, &def);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SetParameter(OMX_IndexParamPortDefinition) D error (0x%08X)\r\n", err);
    }

 #if 0
    // check if we set the parameters correctly
    err = OMX_GetParameter(CompHandle, OMX_IndexParamPortDefinition, &def);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_GetParameter(OMX_IndexParamPortDefinition) D error (0x%08X)\r\n", err);
    }
    // CHECK_EQ(def.eDomain, OMX_PortDomainVideo);
    LOGD ("InputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X)\r\n", def.eDomain, video_def->nFrameWidth, video_def->nFrameHeight, video_def->eCompressionFormat);
#endif

// check output port definition
    def.nPortIndex = OUTPUT_PORT_INDEX;  // output port
    err = OMX_GetParameter(CompHandle, OMX_IndexParamPortDefinition, &def);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_GetParameter(OMX_IndexParamPortDefinition) E error (0x%08X)\r\n", err);
    }
    // CHECK_EQ(def.eDomain, OMX_PortDomainVideo);
    LOGD ("InputPortDef eDomain(0x%08X), width(%d). height(%d), eColorFormat(0x%08X), nBufferCountActual(%d)\r\n",
            def.eDomain, video_def->nFrameWidth, video_def->nFrameHeight, video_def->eColorFormat, def.nBufferCountActual);

    video_def->nFrameWidth = 640;
    video_def->nFrameHeight = 480;
    err = OMX_SetParameter(CompHandle, OMX_IndexParamPortDefinition, &def);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SetParameter(OMX_IndexParamPortDefinition) F error (0x%08X)\r\n", err);
    }

 #if 0
    // check if we set the parameters correctly
    err = OMX_GetParameter(CompHandle, OMX_IndexParamPortDefinition, &def);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_GetParameter(OMX_IndexParamPortDefinition) G error (0x%08X)\r\n", err);
    }
    // CHECK_EQ(def.eDomain, OMX_PortDomainVideo);
    LOGD ("OutputPortDef eDomain(0x%08X), width(%d). height(%d), eColorFormat(0x%08X)\r\n", def.eDomain, video_def->nFrameWidth, video_def->nFrameHeight, video_def->eColorFormat);
#endif

// set component to idle state
    err = OMX_SendCommand(CompHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SendCommand(OMX_CommandStateSet, OMX_StateIdle) error (0x%08X)\r\n", err);
    }


// allocate input buffers and provide to OMX
    unsigned char* pInputBufferPool = new unsigned char[10240*NUM_INPUT_BUF];  // 10KB * 10 input buffers
    for (int i = 0 ; i < NUM_INPUT_BUF ; i++) {
        err = OMX_UseBuffer(CompHandle, &pInBufHdrs[i], INPUT_PORT_INDEX, NULL/*appdata*/, 10240, pInputBufferPool + i*10240);
        if (OMX_ErrorNone != err) {
            LOGE ("OMX_UseBuffer input port error (0x%08X)\r\n", err);
        }
    }

// allocate output buffers and provide to OMX
    int outBufSize = (640*480*3>>1);
    unsigned char* pOutputBufferPool = new unsigned char[outBufSize*NUM_OUTPUT_BUF];
    for (int i = 0 ; i < NUM_OUTPUT_BUF ; i++) {
        err = OMX_UseBuffer(CompHandle, &pOutBufHdrs[i], OUTPUT_PORT_INDEX, NULL/*appdata*/, outBufSize, pOutputBufferPool + i*outBufSize);
        if (OMX_ErrorNone != err) {
            LOGE ("OMX_UseBuffer output port error (0x%08X)\r\n", err);
        }
    }


#if 1 // Start the data flow
    usleep (1000*1000*0.5);
// set component to executing state
    err = OMX_SendCommand(CompHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SendCommand(OMX_CommandStateSet, OMX_StateExecuting) error (0x%08X)\r\n", err);
    }

// start sending input/output buffers to OMX
    for (int i = 0 ; i < NUM_INPUT_BUF ; i++) {
        // TODO: read bitstream into pInBufHdrs[i]->pBuffer
        //pInBufHdrs[i]->pBuffer
        pInBufHdrs[i]->nTimeStamp = gInputTs;
        gInputTs+=33; // update timestamp

        pInBufHdrs[i]->nFilledLen = 1024;
        err = OMX_EmptyThisBuffer(CompHandle, pInBufHdrs[i]);
        if (OMX_ErrorNone != err) {
            LOGE ("OMX_EmptyThisBuffer[%d] error (0x%08X)\r\n", i, err);
        }
    }

    for (int i = 0 ; i < NUM_OUTPUT_BUF ; i++) {
        // TODO: read bitstream into pInBufHdrs[i]->pBuffer
        err = OMX_FillThisBuffer(CompHandle, pOutBufHdrs[i]);
        if (OMX_ErrorNone != err) {
            LOGE ("OMX_FillThisBuffer[%d] error (0x%08X)\r\n", i, err);
        }
    }
#endif


#if 1  // Test port flush
    //usleep (1000*1000*2);
    err = OMX_SendCommand(CompHandle, OMX_CommandFlush, OMX_ALL, NULL);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SendCommand(OMX_CommandFlush) error (0x%08X)\r\n", err);
    }
#endif

#if 0  // Test the output port-reconfiguration
    usleep (1000*1000*5);
    // disable the output port and free output buffers
    err = OMX_SendCommand(CompHandle, OMX_CommandPortDisable, OUTPUT_PORT_INDEX, NULL);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SendCommand(OMX_CommandPortDisable) error (0x%08X)\r\n", err);
    }

    for (int i = 0 ; i < NUM_OUTPUT_BUF ; i++) {
        err = OMX_FreeBuffer(CompHandle, OUTPUT_PORT_INDEX, pOutBufHdrs[i]);
        if (OMX_ErrorNone != err) {
            LOGE ("OMX_FreeBuffer output port error (0x%08X)\r\n", err);
        }
    }

    // note: in real case, the IL client should wait the CmdComplete of OMX_CommandPortDisable before sending the OMX_CommandPortEnable
    usleep (1000*1000*2);

    // test re-enable the output port
    err = OMX_SendCommand(CompHandle, OMX_CommandPortEnable, OUTPUT_PORT_INDEX, NULL);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_SendCommand(OMX_CommandPortEnable) error (0x%08X)\r\n", err);
    }

    for (int i = 0 ; i < NUM_OUTPUT_BUF ; i++) {
        err = OMX_UseBuffer(CompHandle, &pOutBufHdrs[i], OUTPUT_PORT_INDEX, NULL/*appdata*/, outBufSize, pOutputBufferPool + i*outBufSize);
        if (OMX_ErrorNone != err) {
            LOGE ("OMX_UseBuffer output port error (0x%08X)\r\n", err);
        }
    }
#endif

////
#if 1
    usleep (1000*1000*2);

    err = OMX_GetState(CompHandle, &state);
    if (OMX_ErrorNone != err) {
        LOGE ("OMX_GetState error (0x%08X)\r\n", err);
    }
    LOGD ("OMX state = %d\r\n", state);

    usleep (1000*1000*60);
#endif

// clean up
  (*_pfnOmxFreeHandle)(reinterpret_cast<OMX_HANDLETYPE *>(CompHandle));
  (*_pfnOmxDeInit)();

  dlclose(handle);
  return 0;
}
