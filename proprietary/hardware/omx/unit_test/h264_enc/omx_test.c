/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010 -2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/** 
 * @file 
 *   omx_test.c
 *
 * @par Project:
 *   6575 
 *
 * @par Description:
 *   OMX IL testing
 *
 * @par Author:
 *   Fantasia Lin (mtk03850)
 *
 * @par $Revision: #1$
 * @par $Modtime:$
 * @par $Log:$
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include "omx_test.h"

//please note that the OMXCoreIface table should allocate by caller
 int LoadOMXCore(char *pStrOMXCorePathFileName, pOMXCoreIfaceTable pOMXCoreIfaceTableInst)
{
    int iRetValue = 0;
    
    #if EXTERNAL_LOAD_SO
    void* handle = NULL;

    if (NULL == pStrOMXCorePathFileName){
        iRetValue = -1;
        goto GOOUT;
    }
    #endif
    
    if(NULL == pOMXCoreIfaceTableInst){
        iRetValue = -2;
        goto GOOUT;
    }
    #if EXTERNAL_LOAD_SO
    memset(pOMXCoreIfaceTableInst, 0, sizeof(OMXCoreIfaceTable));
    // load the OMX core share library
    if ((handle = dlopen (pStrOMXCorePathFileName, RTLD_LAZY/*| RTLD_GLOBAL*/)) == NULL){
        iRetValue = -10;
        goto GOOUT;
    }
    UT_LOG_D(printf("Got OMX core share library\n");)
    pOMXCoreIfaceTableInst->pHandle = handle;
    #endif    
    
    // get the interfaces...
    pOMXCoreIfaceTableInst->_OMX_Init = (pfnOMX_Init)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_Init");
    pOMXCoreIfaceTableInst->_OMX_Deinit = (pfnOMX_Deinit)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_Deinit");
    pOMXCoreIfaceTableInst->_OMX_ComponentNameEnum = (pfnOMX_ComponentNameEnum)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_ComponentNameEnum");
    pOMXCoreIfaceTableInst->_OMX_GetHandle = (pfnOMX_GetHandle)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_GetHandle");
    pOMXCoreIfaceTableInst->_OMX_FreeHandle = (pfnOMX_FreeHandle)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_FreeHandle"); 
    pOMXCoreIfaceTableInst->_OMX_GetComponentsOfRole = (pfnOMX_GetComponentsOfRole)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_GetComponentsOfRole"); 
    pOMXCoreIfaceTableInst->_OMX_GetRolesOfComponent = (pfnOMX_GetRolesOfComponent)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_GetRolesOfComponent"); 

    /*
    pfnOMX_SetupTunnel
    pfnOMX_GetContentPipe
    */

    return 1;
GOOUT:
    return iRetValue;
}

int ReleaseOMXCore(pOMXCoreIfaceTable pOMXCoreIfaceTableInst)
{
    int iRetValue = 0;
    
    if(NULL == pOMXCoreIfaceTableInst){
        iRetValue = -1;
        goto GOOUT;
    }

    if(NULL != pOMXCoreIfaceTableInst->pInputBuffPool){
        free(pOMXCoreIfaceTableInst->pInputBuffPool);
    }

    if(NULL != pOMXCoreIfaceTableInst->pOutBuffPool){
        free(pOMXCoreIfaceTableInst->pOutBuffPool);
    }
    #if EXTERNAL_LOAD_SO
    if(NULL != pOMXCoreIfaceTableInst->pHandle){
        dlclose(pOMXCoreIfaceTableInst->pHandle);
    }
    #endif
    //memset(pOMXCoreIfaceTableInst, 0, sizeof(OMXCoreIfaceTable));

    return 1;
GOOUT:
    return iRetValue;
}


int GetOmxComponentHandle(char *pStrOMXComponentName, pOMXCoreIfaceTable pOMXCoreIfaceTableInst, void *pAppData)
{
    int iRetValue = 0;
    int iGotComponent = 0;
    unsigned int uStrLen = 0;
    OMX_U32 index = 0;
    OMX_U32 numRoles = 0;
    OMX_U32 numRoles2 = 0;
    char name[256];
    OMX_ERRORTYPE err;
    OMX_U8 **p2DArray = NULL;
    OMX_PARAM_COMPONENTROLETYPE roleParams;
    
    if((NULL == pOMXCoreIfaceTableInst) || (NULL == pStrOMXComponentName) || (NULL == pAppData)){
        iRetValue = -1;
        goto GOOUT;
    }

    if(NULL== pOMXCoreIfaceTableInst->pHandle){
        iRetValue = -2;
        goto GOOUT;
    }

    //enumerate all components
    index = 0;
    iGotComponent = 0;
    uStrLen = strlen(pStrOMXComponentName);
    while((err = pOMXCoreIfaceTableInst->_OMX_ComponentNameEnum(name, sizeof(name), index)) == OMX_ErrorNone){
        //UT_LOG_D(printf("[%d] %s\n", (int)index, name);)
        if(strlen(name) == uStrLen){
            if(strncasecmp(name, pStrOMXComponentName , uStrLen) == 0){
                UT_LOG_D(printf("Got %s idx %d\n", name, (int)index);)
                iGotComponent = 1;
                strcpy(pOMXCoreIfaceTableInst->szComponentName, pStrOMXComponentName);
            }
        }
        if(1 == iGotComponent){
            //get roles of component
            if(OMX_ErrorNone == pOMXCoreIfaceTableInst->_OMX_GetRolesOfComponent(name, &numRoles, NULL)){
                UT_LOG_D(printf(">> Role Count %d\n", (int)numRoles);)
                if(numRoles == 1){
                    p2DArray = (OMX_U8**)malloc(sizeof(OMX_U8*)*numRoles);
                    p2DArray[0] = (OMX_U8*)malloc(OMX_MAX_STRINGNAME_SIZE);
                    if(OMX_ErrorNone == pOMXCoreIfaceTableInst->_OMX_GetRolesOfComponent(name, &numRoles2, p2DArray)){
                        UT_LOG_D(printf ("numRoles=%d, Role[%d] is %s\n", (int)numRoles2, 0, p2DArray[0]); )
                        if(1 == iGotComponent){
                            strcpy(pOMXCoreIfaceTableInst->szComponentRole, (const char*)p2DArray[0]);
                            break;// just check the component exist?
                        }
                    }
                    free((void*)p2DArray[0]);
                    free((void*)p2DArray);
                }
                else{
                    //we only check first role
                }
            }
        }
        index++;
    }

    UT_LOG_D(printf("Success to get OMX compoent handle 0x%x\n", (unsigned int)pOMXCoreIfaceTableInst->CompHandle);)
    if(1 == iGotComponent){
        //get component handle
        if(OMX_ErrorNone != pOMXCoreIfaceTableInst->_OMX_GetHandle(&pOMXCoreIfaceTableInst->CompHandle, pStrOMXComponentName, pAppData, &pOMXCoreIfaceTableInst->OMX_CallBack)){
            iRetValue = -10;
            goto GOOUT;
        }
        UT_LOG_D(printf("Success to get OMX compoent handle 0x%x\n", (unsigned int)pOMXCoreIfaceTableInst->CompHandle);)

        //set component role?
         roleParams.nSize = sizeof(OMX_PARAM_COMPONENTROLETYPE);    
        roleParams.nVersion.s.nVersionMajor = 1;    
        roleParams.nVersion.s.nVersionMinor = 0;    
        roleParams.nVersion.s.nRevision = 0;    
        roleParams.nVersion.s.nStep = 0;    
        strcpy((char *)roleParams.cRole, pOMXCoreIfaceTableInst->szComponentRole);    
        if(OMX_ErrorNone != OMX_SetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamStandardComponentRole, &roleParams)){
            UT_LOG_E(printf("Fail to set compoent role");)
            // release omx component
            if(OMX_ErrorNone != pOMXCoreIfaceTableInst->_OMX_FreeHandle(pOMXCoreIfaceTableInst->CompHandle)){
                iRetValue = -39;
                goto GOOUT;
            }
            iRetValue = -20;
            goto GOOUT;
        }
        UT_LOG_D(printf("set compoent role as %s\n", pOMXCoreIfaceTableInst->szComponentRole);)

        //success
    }
    else{
        UT_LOG_E(printf("Cannot find compoent %s\n", pStrOMXComponentName);)
        iRetValue = -100;
        goto GOOUT;
    }
    return 1;
GOOUT:
    UT_LOG_E(printf("%s Error code %d\n", __FUNCTION__, iRetValue);)
    return iRetValue;

}

 int ConfigOmxComponentPorts(pOMXCoreIfaceTable pOMXCoreIfaceTableInst)
{
     int iRetValue = 0;
     OMX_VIDEO_PARAM_PORTFORMATTYPE format;
     OMX_PARAM_PORTDEFINITIONTYPE def;
     OMX_VIDEO_PORTDEFINITIONTYPE *video_def;
     OMX_ERRORTYPE err;
     int i;
     unsigned char* pInputBufferPool = NULL;
     unsigned char* pOutputBufferPool = NULL;
     unsigned int uiShiftInputBuffer = 0;
     unsigned int uiShiftOutputBuffer = 0;
     
     if(NULL == pOMXCoreIfaceTableInst){
         iRetValue = -1;
         goto GOOUT;
     }
     if(0 == pOMXCoreIfaceTableInst->CompHandle){
         iRetValue = -2;
         goto GOOUT;
     }

    //input port format
    format.nVersion.s.nVersionMajor = 1;    
    format.nVersion.s.nVersionMinor = 0;    
    format.nVersion.s.nRevision = 0;    
    format.nVersion.s.nStep = 0;        
    format.nPortIndex = INPUT_PORT_INDEX; // input port    
    format.nIndex = 0;
    err = OMX_GetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamVideoPortFormat, &format);
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("Fail to OMX_GetParameter(OMX_IndexParamVideoPortFormat) (0x%08X)\n", err);)
         iRetValue = -20;
         goto GOOUT;
    }
    UT_LOG_D(printf ("OMX_GetParameter INPUT port eCompressionFormat(0x%08X), eColorFormat(0x%08X)\n", format.eCompressionFormat, format.eColorFormat); )
    //format.eCompressionFormat = OMX_VIDEO_CodingAVC , format.eColorFormat = OMX_COLOR_FormatYUV420Planar
    format.eCompressionFormat = OMX_VIDEO_CodingAVC;
    format.eColorFormat = OMX_COLOR_FormatYUV420Planar;
    format.nIndex = 0;
    err = OMX_SetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamVideoPortFormat, &format);   
    if (OMX_ErrorNone != err) {        
        UT_LOG_E(printf("Fail to OMX_SetParameter(OMX_IndexParamVideoPortFormat) (0x%08X)\n", err);)
        iRetValue = -21;
        goto GOOUT;

    }
    #if 1 //for check
    format.nVersion.s.nVersionMajor = 1;    
    format.nVersion.s.nVersionMinor = 0;    
    format.nVersion.s.nRevision = 0;    
    format.nVersion.s.nStep = 0;        
    format.nPortIndex = INPUT_PORT_INDEX; // input port    
    format.nIndex = 0;
    err = OMX_GetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamVideoPortFormat, &format);
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("Fail to OMX_GetParameter(OMX_IndexParamVideoPortFormat) (0x%08X)\n", err);)
         iRetValue = -22;
         goto GOOUT;
    }
    UT_LOG_D(printf ("OMX_GetParameter INPUT port eCompressionFormat(0x%08X), eColorFormat(0x%08X)\n", format.eCompressionFormat, format.eColorFormat); )
    #endif
    
    //output port format
    format.nPortIndex = OUTPUT_PORT_INDEX; // output port    
    format.nIndex = 0;    
    err = OMX_GetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamVideoPortFormat, &format);    
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("Fail to OMX_GetParameter(OMX_IndexParamVideoPortFormat) (0x%08X)\r\n", err);)
        iRetValue = -23;
        goto GOOUT;

    }
    UT_LOG_D(printf("OMX_GetParameter OUTPUT port port eCompressionFormat(0x%08X), eColorFormat(0x%08X)\n",  format.eCompressionFormat, format.eColorFormat);)
    // CHECK: format.eCompressionFormat must equal to OMX_VIDEO_CodingUnused     
 
    err = OMX_SetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamVideoPortFormat, &format);
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("Fail to OMX_SetParameter(OMX_IndexParamVideoPortFormat) (0x%08X)\r\n", err);)
        iRetValue = -24;
        goto GOOUT;

    }

    // check input port definition    
    def.nVersion.s.nVersionMajor = 1;    
    def.nVersion.s.nVersionMinor = 0;    
    def.nVersion.s.nRevision = 0;    
    def.nVersion.s.nStep = 0;    
    def.nPortIndex = INPUT_PORT_INDEX;  // input port    
    video_def = &def.format.video;        
    err = OMX_GetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamPortDefinition, &def);    
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("Fail to OMX_GetParameter(OMX_IndexParamPortDefinition) (0x%08X)\r\n", err);)
        iRetValue = -25;
        goto GOOUT;

    }    
    // CHECK_EQ(def.eDomain, OMX_PortDomainVideo);    
    UT_LOG_D(printf("InputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X), nBufferCountActual(%d)\r\n", def.eDomain, (int)video_def->nFrameWidth, (int)video_def->nFrameHeight, video_def->eCompressionFormat, (int)def.nBufferCountActual);)
    video_def->nFrameWidth = pOMXCoreIfaceTableInst->width;    
    video_def->nFrameHeight = pOMXCoreIfaceTableInst->height;    
    //video_def->nBitrate = pOMXCoreIfaceTableInst->bitrate;
    video_def->xFramerate = (pOMXCoreIfaceTableInst->fps/10)<<16;//Q16 format
    video_def->eColorFormat = OMX_COLOR_FormatYUV420Planar;
    video_def->eCompressionFormat = OMX_VIDEO_CodingUnused;    
    def.nBufferCountActual = H264_NUM_INPUT_BUF;        //set the buffer count
    //video_def->eColorFormat = OMX_COLOR_FormatUnused;    
    err = OMX_SetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamPortDefinition, &def);    
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("OMX_SetParameter(OMX_IndexParamPortDefinition) D error (0x%08X)\r\n", err);)
        iRetValue = -26;
        goto GOOUT;

    }

    #if 1 //for check
    def.nVersion.s.nVersionMajor = 1;    
    def.nVersion.s.nVersionMinor = 0;    
    def.nVersion.s.nRevision = 0;    
    def.nVersion.s.nStep = 0;    
    def.nPortIndex = INPUT_PORT_INDEX;  // input port    
    video_def = &def.format.video;        
    err = OMX_GetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamPortDefinition, &def);    
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("Fail to OMX_GetParameter(OMX_IndexParamPortDefinition) (0x%08X)\r\n", err);)
        iRetValue = -27;
        goto GOOUT;

    }    
    // CHECK_EQ(def.eDomain, OMX_PortDomainVideo);    
    UT_LOG_D(printf("CHECK >> InputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X), nBitrate (%d), xFramerate (%d), nBufferCountActual(%d)\r\n", def.eDomain, (int)video_def->nFrameWidth, (int)video_def->nFrameHeight, video_def->eCompressionFormat, (int)video_def->nBitrate, (int)video_def->xFramerate, (int)def.nBufferCountActual);)
    #endif

    // check output port definition    
    def.nPortIndex = OUTPUT_PORT_INDEX;  // output port    
    err = OMX_GetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamPortDefinition, &def);    
    if (OMX_ErrorNone != err) {        
        UT_LOG_E(printf("Fail to OMX_GetParameter(OMX_IndexParamPortDefinition) E error (0x%08X)\r\n", err);)
        iRetValue = -28;
        goto GOOUT;

    }    
    // CHECK_EQ(def.eDomain, OMX_PortDomainVideo);    
    UT_LOG_D(printf("OutputPortDef eDomain(0x%08X), width(%d). height(%d), eColorFormat(0x%08X), nBufferCountActual(%d)\r\n", def.eDomain, (int)video_def->nFrameWidth, (int)video_def->nFrameHeight, (int)video_def->eColorFormat, (int)def.nBufferCountActual);)
    video_def->nBitrate = pOMXCoreIfaceTableInst->bitrate;
    video_def->xFramerate = (pOMXCoreIfaceTableInst->fps/10)<<16;//Q16 format
    video_def->nFrameWidth = pOMXCoreIfaceTableInst->width;     
    video_def->nFrameHeight = pOMXCoreIfaceTableInst->height;    
    video_def->eColorFormat = OMX_COLOR_FormatUnused; 
    video_def->eCompressionFormat = OMX_VIDEO_CodingAVC;  
    def.nBufferCountActual = H264_NUM_OUTPUT_BUF; //set the buffer count    
    err = OMX_SetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamPortDefinition, &def);    
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("Fail to OMX_SetParameter(OMX_IndexParamPortDefinition) F error (0x%08X)\r\n", err);)
        iRetValue = -29;
        goto GOOUT;

    }

    #if 1 //for check
    def.nVersion.s.nVersionMajor = 1;    
    def.nVersion.s.nVersionMinor = 0;    
    def.nVersion.s.nRevision = 0;    
    def.nVersion.s.nStep = 0;    
    def.nPortIndex = INPUT_PORT_INDEX;  // input port    
    video_def = &def.format.video;        
    err = OMX_GetParameter(pOMXCoreIfaceTableInst->CompHandle, OMX_IndexParamPortDefinition, &def);    
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("Fail to OMX_GetParameter(OMX_IndexParamPortDefinition) (0x%08X)\r\n", err);)
        iRetValue = -30;
        goto GOOUT;

    }    
    // CHECK_EQ(def.eDomain, OMX_PortDomainVideo);    
    UT_LOG_D(printf("CHECK >> OutputPortDef eDomain(0x%08X), width(%d). height(%d), eCompressionFormat(0x%08X), nBufferCountActual(%d)\r\n", def.eDomain, (int)video_def->nFrameWidth, (int)video_def->nFrameHeight, video_def->eCompressionFormat, (int)def.nBufferCountActual);)
    #endif

    #if 1
    err = OMX_SendCommand(pOMXCoreIfaceTableInst->CompHandle, OMX_CommandStateSet, OMX_StateIdle, NULL);
    if (OMX_ErrorNone != err) {
        UT_LOG_E(printf("OMX_SendCommand(OMX_CommandStateSet, OMX_StateIdle) error (0x%08X)\r\n", err);)
        iRetValue = -40;
        goto GOOUT;

    }
    #endif

    // allocate input buffers and provide to OMX    
    int inBufSize = (pOMXCoreIfaceTableInst->width*pOMXCoreIfaceTableInst->height*3>>1);   //YUV420 
    pInputBufferPool = (unsigned char*)malloc((inBufSize*H264_NUM_INPUT_BUF)+32); //+32 for 32 alignment
    if(NULL == pInputBufferPool){
        iRetValue = -50;
        goto GOOUT;

    }
    
    uiShiftInputBuffer =  ((unsigned int)pInputBufferPool + 32) & 0x0000001F;
    UT_LOG_D(printf("input pool buffer 0x%08X, shift %d -> 0x%08X\n", (unsigned int)pInputBufferPool, uiShiftInputBuffer, (unsigned int)(pInputBufferPool+uiShiftInputBuffer));)
    pOMXCoreIfaceTableInst->pInputBuffPool = pInputBufferPool;
    for (i = 0 ; i < H264_NUM_INPUT_BUF ; i++) {
        err = OMX_UseBuffer(pOMXCoreIfaceTableInst->CompHandle, &pOMXCoreIfaceTableInst->pInBufHdrs[i], INPUT_PORT_INDEX, NULL/*appdata*/, inBufSize, (pInputBufferPool+uiShiftInputBuffer) + i*inBufSize);
        if (OMX_ErrorNone != err) {
            UT_LOG_E(printf("OMX_UseBuffer input port error idx %d (0x%08X)\r\n",i, err);)
            iRetValue = -51;
            goto GOOUT;

        }
        pOMXCoreIfaceTableInst->pInBufHdrs[i]->nOffset = 0;
        //set owner to IL client
        pOMXCoreIfaceTableInst->InBufOwner[i] = BOS_IL_CLIENT;
    }
    
    // allocate output buffers and provide to OMX    
    int outBufSize = (1048576);//(10240);//(1048576);    
    pOutputBufferPool = (unsigned char*)malloc((outBufSize*H264_NUM_OUTPUT_BUF)+32);  // 10KB * 10 input buffers    //+32 for 32 alignment
    if(NULL == pOutputBufferPool){
        iRetValue = -60;
        goto GOOUT;
    }
    uiShiftOutputBuffer = ((unsigned int)pOutputBufferPool + 32) & 0x0000001F;
    UT_LOG_D(printf("output pool buffer 0x%08X, shift %d -> 0x%08X\n", (unsigned int)pOutputBufferPool, uiShiftOutputBuffer, (unsigned int)(pOutputBufferPool+uiShiftOutputBuffer));)
    pOMXCoreIfaceTableInst->pOutBuffPool = pOutputBufferPool;
    for (i = 0 ; i < H264_NUM_OUTPUT_BUF ; i++) {
        err = OMX_UseBuffer(pOMXCoreIfaceTableInst->CompHandle, &pOMXCoreIfaceTableInst->pOutBufHdrs[i], OUTPUT_PORT_INDEX, NULL/*appdata*/, outBufSize, (pOutputBufferPool+uiShiftOutputBuffer) + i*outBufSize);        
        if (OMX_ErrorNone != err) { 
            UT_LOG_E(printf("OMX_UseBuffer output port error idx %d (0x%08X)\r\n", i, err); )
            iRetValue = -61;
            goto GOOUT;
        }
        pOMXCoreIfaceTableInst->pOutBufHdrs[i]->nOffset = 0;
        //set owner to IL client
        pOMXCoreIfaceTableInst->OutBufOwner[i] = BOS_IL_CLIENT;
    }
    
    UT_LOG_I(printf("Success to ConfigOmxComponentPorts\n");)

    return 1;
GOOUT:
    UT_LOG_E(printf("ConfigOmxComponentPorts Error %d\n",iRetValue);)
    if(NULL != pOMXCoreIfaceTableInst->pInputBuffPool){
        free(pOMXCoreIfaceTableInst->pInputBuffPool);
        pOMXCoreIfaceTableInst->pInputBuffPool = NULL;
    }
    if(NULL != pOMXCoreIfaceTableInst->pOutBuffPool){
        free(pOMXCoreIfaceTableInst->pOutBuffPool);
        pOMXCoreIfaceTableInst->pOutBuffPool = NULL;
    }
    return iRetValue;
}



 // END OF FILE
