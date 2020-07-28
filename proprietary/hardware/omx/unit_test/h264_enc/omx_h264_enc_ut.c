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
 *   omx_h264_enc_ut.c
 *
 * @par Project:
 *   6575 
 *
 * @par Description:
 *   video encode driver test code
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

#define VENC_UNUSED(x) (void)x
/*
PLEASE NOTE THAT, MTK's OMX CORE has problem to handle the free OMX component during the multi-thread case.
SO I DID THE WORKAROUND ON THE TEST CODE. -> just free OMX component at all encode tasks end.
In this case, we cannot test over 10 test case, due to the OMX Core maximum support 10 running instance at the same time.
*/
 
 typedef struct _EncoderTestConfig{
    uint32_t iTestFlag;          // 0, not test, 1 do test
    int32_t width;
    int32_t height;
    uint32_t bitrate;
    uint32_t fps;                    // need to x10 ?
    int32_t    firstVop;            // first Vop, start from 0 to (frame_to_encode - 1)
    uint32_t    frame_to_encode;   //total  frame count
    uint32_t   fps_den;
    uint32_t   i_keyint_max;        //Maximum GOP size
    uint32_t   EIS_Support;
    char *pStrInputYUVPathFileName;
    char *pStrOutputBitStreamPathFileName;
    char *pStrGoldenBitStreamPathFileName;
}EncoderTestConfig, *pEncoderTestConfig;
 
 EncoderTestConfig TestArray[]= {
 //{iTestFlag, width, height, bitrate, fps, firstVop, frame_to_encode, fps_den, i_keyint_max, pStrInputYUVPathFileName, pStrOutputBitStreamPathFileName, pStrGoldenBitStreamPathFileName},
    //hvga
    #if 0
    {1,480, 320, 2500000, 300, 0, 212, 1, 60, 0, "/sdcard/hvga/lab_fast_hvga_212f.yuv",                  "/sdcard/6575/hvga_lab_fast_30fps_212f.264",               "/sdcard/golden/hvga_lab_fast_30fps_212f.264"},
    {1,480, 320, 2500000, 300, 0, 540, 1, 60, 0, "/sdcard/hvga/lab_slow_hvga_540f.yuv",                "/sdcard/6575/hvga_lab_slow_30fps_540f.264",              "/sdcard/golden/hvga_lab_slow_30fps_540f.264"},
    {1,480, 320, 2500000, 300, 0, 300, 1, 60, 0, "/sdcard/hvga/mobile_calendar_hvga_300f.yuv",  "/sdcard/6575/hvga_mobile_calendar_30fps_300f.264", "/sdcard/golden/hvga_mobile_calendar_30fps_300f.264"},
    {1,480, 320, 2500000, 300, 0, 223, 1, 60, 0, "/sdcard/hvga/office_fast_hvga_223f.yuv",             "/sdcard/6575/hvga_office_fast_30fps_223f.264",            "/sdcard/golden/hvga_office_fast_30fps_223f.264"},
    {1,480, 320, 2500000, 300, 0, 636, 1, 60, 0, "/sdcard/hvga/office_slow2_hvga_363f.yuv",          "/sdcard/6575/hvga_office_slow2_30fps_363f.264",        "/sdcard/golden/hvga_office_slow2_30fps_363f.264"},   
    {1,480, 320, 2500000, 300, 0, 475, 1, 60, 0, "/sdcard/hvga/office_slow_hvga_475f.yuv",            "/sdcard/6575/hvga_office_slow_30fps_475f.264",          "/sdcard/golden/hvga_office_slow_30fps_475f.264"},
    {1,480, 320, 2500000, 300, 0, 201, 1, 60, 0, "/sdcard/hvga/outdoor1_fast_hvga_201f.yuv",       "/sdcard/6575/hvga_outdoor1_fast_30fps_201f.264",      "/sdcard/golden/hvga_outdoor1_fast_30fps_201f.264"},
    {1,480, 320, 2500000, 300, 0, 560, 1, 60, 0, "/sdcard/hvga/outdoor1_slow_hvga_560f.yuv",      "/sdcard/6575/hvga_outdoor1_slow_30fps_560f.264",    "/sdcard/golden/hvga_outdoor1_slow_30fps_560f.264"},
    {1,480, 320, 2500000, 300, 0, 210, 1, 60, 0, "/sdcard/hvga/outdoor2_fast_hvga_210f.yuv",        "/sdcard/6575/hvga_outdoor2_fast_30fps_210f.264",     "/sdcard/golden/hvga_outdoor2_fast_30fps_210f.264"},
    {1,480, 320, 2500000, 300, 0, 587, 1, 60, 0, "/sdcard/hvga/outdoor2_slow_hvga_587f.yuv",      "/sdcard/6575/hvga_outdoor2_slow_30fps_587f.264",    "/sdcard/golden/hvga_outdoor2_slow_30fps_587f.264"},
    {0,0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,NULL,NULL},//this row will not test.
    #endif
    //vga
    #if 1
    {1,640, 480, 2500000, 150, 0, 281, 1, 60, 0, "/sdcard/vga/indoor_fast_vga_281f.yuv",                 "/sdcard/6575/vga_indoor_fast_15fps_281f.264",            "/sdcard/golden/vga_indoor_fast_15fps_281f.264"},
    {0,0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,NULL,NULL},//this row will not test.
    {1,640, 480, 2500000, 150, 0, 799, 1, 60, 0, "/sdcard/vga/indoor_slow_vga_799f.yuv",               "/sdcard/6575/vga_indoor_slow_15fps_799f.264",           "/sdcard/golden/vga_indoor_slow_15fps_799f.264"},
    {1,640, 480, 2500000, 150, 0, 375, 1, 60, 0, "/sdcard/vga/jaakiekko_25fps_vga_375f.yuv",          "/sdcard/6575/vga_jaakiekko_15fps_375f.264",                "/sdcard/golden/vga_jaakiekko_15fps_375f.264"},
    {1,640, 480, 2500000, 150, 0, 375, 1, 60, 0, "/sdcard/vga/juna_25fps_vga_375f.yuv",                  "/sdcard/6575/vga_juna_15fps_375f.264",                        "/sdcard/golden/vga_juna_15fps_375f.264"},
    {1,640, 480, 2500000, 150, 0, 655, 1, 60, 0, "/sdcard/vga/koski_25fps_vga_655f.yuv",                 "/sdcard/6575/vga_koski_15fps_655f.264",                       "/sdcard/golden/vga_koski_15fps_655f.264"},
    {1,640, 480, 2500000, 150, 0, 246, 1, 60, 0, "/sdcard/vga/lab_fast_vga_246f.yuv",                       "/sdcard/6575/vga_lab_fast_15fps_246f.264",                   "/sdcard/golden/vga_lab_fast_15fps_246f.264"},
    {1,640, 480, 2500000, 150, 0, 705, 1, 60, 0, "/sdcard/vga/lab_slow_vga_705f.yuv",                      "/sdcard/6575/vga_lab_slow_15fps_705f.264",                 "/sdcard/golden/vga_lab_slow_15fps_705f.264"},
    {1,640, 480, 2500000, 150, 0, 300, 1, 60, 0, "/sdcard/vga/mobile_calendar_vga_300f.yuv",        "/sdcard/6575/vga_mobile_calendar_15fps_300f.264",    "/sdcard/golden/vga_mobile_calendar_15fps_300f.264"},
    {1,640, 480, 2500000, 150, 0, 319, 1, 60, 0, "/sdcard/vga/outdoor_fast_vga_319f.yuv",               "/sdcard/6575/vga_outdoor_fast_15fps_319f.264",          "/sdcard/golden/vga_outdoor_fast_15fps_319f.264"},
    {1,640, 480, 2500000, 150, 0, 989, 1, 60, 0, "/sdcard/vga/outdoor_slow_vga_989f.yuv",             "/sdcard/6575/vga_outdoor_slow_15fps_989f.264",         "/sdcard/golden/vga_outdoor_slow_15fps_989f.264"},
    #if 1               //if in GB2 this should be disable
    {1,640, 480, 2500000, 150, 0, 315, 1, 60, 0, "/sdcard/vga/road_fast_vga_315f.yuv",                     "/sdcard/6575/vga_road_fast_15fps_315f.264",                "/sdcard/golden/vga_road_fast_15fps_315f.264"},
    {1,640, 480, 2500000, 150, 0, 1031, 1, 60, 0, "/sdcard/vga/road_slow_vga_1031f.yuv",               "/sdcard/6575/vga_road_slow_15fps_1031f.264",             "/sdcard/golden/vga_road_slow_15fps_1031f.264"},
    {1,640, 480, 2500000, 150, 0, 375, 1, 60, 0, "/sdcard/vga/th_25fps_vga_375f.yuv",                      "/sdcard/6575/vga_th_25fps_vga_15fps_375f.264",           "/sdcard/golden/vga_th_25fps_vga_15fps_375f.264"},
    {1,640, 480, 2500000, 150, 0, 376, 1, 60, 0, "/sdcard/vga/walk_vga_376f.yuv",                             "/sdcard/6575/vga_walk_vga_15fps_376f.264",                  "/sdcard/golden/vga_walk_vga_15fps_376f.264"},
    #endif
    #endif
    {0,0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,NULL,NULL},//this row will not test.
};

#define MAGIC_NUMBER_H264  0xFEED1234

#define OMX_CORE_PATHFILENAME "/vendor/lib/libMtkOmxCore.so"
#define TARGET_TEST_OMX_COMPONENT "OMX.MTK.VIDEO.ENCODER.AVC"
//#define TARGET_TEST_OMX_COMPONENT "OMX.MTK.VIDEO.ENCODER.MPEG4"


typedef struct _TestThreadData{
    unsigned int iMagicNumber;          //
    pthread_t hThreadis;
    int iThreadTermiate;	    // 1 : terminate thread, otherwise keep going
    int iActivate;		    // 0 deactivate, otherwise activate
    int iIndexer;                   // just index number for the testing arrary
    int iTestResult;               //<= -1 fail, 0 testing, >=1 success
    pEncoderTestConfig pEncoderTestConfigInst;
    double durationOfTest;    //just for estimate the decoding time.
    OMXCoreIfaceTable OMXCoreIfaceTableInst;
    OMX_STATETYPE state;              //OMX state
    FILE *pInFile;
    FILE *pOutFile;
    pthread_mutex_t hMutexInputAccess;
    pthread_mutex_t hMutexOutAccess;
}TestThreadData, *pTestThreadData;

//function prototypes
int OMX_TEST_H264Enc(pTestThreadData pTestThreadDataInst);

//how many thread can run at the same time
#define ThreadCount 1
// use mult-thread way to perform the test, just create a thread to perform the test.
#define USE_MULTITHREAD 1
//output bit-stream or not
#define OUTPUT_BIT_STREAM 1

double tdbl(struct timeval *a)
{  
    return (a->tv_sec + a->tv_usec/1e6);
}

int64_t getTime(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (int64_t)(now.tv_sec*1000000 + now.tv_usec);
}




void* UT_TestThread(void *lpData)
{
    int iRetValue = 0;
    struct timeval	  timeis;	
    double dt1, dt2, dt_diff;
    pTestThreadData pTestThreadDataIs = NULL;
    pEncoderTestConfig  pEncoderTestConfigInst = NULL;

    if(NULL == lpData){
        iRetValue = -1;
        goto GoOut; 
    }

    pTestThreadDataIs = (pTestThreadData)lpData;

    pEncoderTestConfigInst = pTestThreadDataIs->pEncoderTestConfigInst;
    pTestThreadDataIs->iActivate = 1;


    gettimeofday(&timeis,NULL);
    dt1 = tdbl(&timeis);
    if((iRetValue = OMX_TEST_H264Enc(pTestThreadDataIs)) > 0){
            gettimeofday(&timeis,NULL);
            dt2 = tdbl(&timeis);        
            dt_diff = dt2 - dt1;
            UT_LOG_E(printf("[ Result ] %s, total frames %d, total time %5.3f, Actual encode fps %5.3f\n\n", 
                    pEncoderTestConfigInst->pStrInputYUVPathFileName, 
                    pEncoderTestConfigInst->frame_to_encode,
                    (dt_diff),
                    (double)pEncoderTestConfigInst->frame_to_encode/dt_diff);)

            
            pTestThreadDataIs->durationOfTest = dt_diff;
            pTestThreadDataIs->iTestResult = 1; //success
    // compare golden.
    
    }
    else{
        UT_LOG_E(printf("**** Test fail!! %s, idx %d **** Error %d\n", pEncoderTestConfigInst->pStrInputYUVPathFileName, pTestThreadDataIs->iIndexer, iRetValue);)
        pTestThreadDataIs->iTestResult = -1; //success
    }

    pTestThreadDataIs->iActivate = 0;
GoOut:
    if(iRetValue <= -1){
        UT_LOG_E(printf("Test Thread error %d\n", iRetValue);)
    }
    return NULL;
}


int main(int argc, char *argv[])
{
    VENC_UNUSED(argc);
    VENC_UNUSED(argv);
    int iIndexer = 0;
    int iIdxTmp = 0;
    int iDataSize = 0;
    int iThreadIdx = 0;
    
    pEncoderTestConfig  pEncoderTestConfigInst = NULL;
    
    #if !(EXTERNAL_LOAD_SO)
    void* pOMX_CORE_handle = NULL;
    //OMXCoreIfaceTable OMXCoreIfaceTabletmp;
    #endif
    
    #if !(USE_MULTITHREAD)
        struct timeval	  timeis;	
        double dt1, dt2, dt_diff;
    #endif
    pTestThreadData pTestThreadDataInst = NULL;
    pTestThreadData pTestThreadDataTmp = NULL;
    int iRet = 0;
    int iStatus=0;
    pthread_t hThreadis[ThreadCount];

    
    iDataSize = sizeof(TestThreadData);

    pTestThreadDataInst = (pTestThreadData)malloc(iDataSize*ThreadCount);
    if(NULL == pTestThreadDataInst){
        UT_LOG_W(printf("fail to create the thread data\n");)
        return -1;
    }
    memset(pTestThreadDataInst, 0, iDataSize*ThreadCount);

    #if !(EXTERNAL_LOAD_SO)
    // load the OMX core share library
    if ((pOMX_CORE_handle = dlopen (OMX_CORE_PATHFILENAME, RTLD_LAZY)) == NULL){
        UT_LOG_W(printf("Fail to load OMX core share library\n");)
        goto GoOut;
    }
    UT_LOG_I(printf("Got OMX core share library\n");)
    /*
    OMXCoreIfaceTabletmp._OMX_Init = (pfnOMX_Init)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_Init");
    OMXCoreIfaceTabletmp._OMX_Deinit = (pfnOMX_Deinit)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_Deinit");
    OMXCoreIfaceTabletmp._OMX_ComponentNameEnum = (pfnOMX_ComponentNameEnum)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_ComponentNameEnum");
    OMXCoreIfaceTabletmp._OMX_GetHandle = (pfnOMX_GetHandle)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_GetHandle");
    OMXCoreIfaceTabletmp._OMX_FreeHandle = (pfnOMX_FreeHandle)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_FreeHandle"); 
    OMXCoreIfaceTabletmp._OMX_GetComponentsOfRole = (pfnOMX_GetComponentsOfRole)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_GetComponentsOfRole"); 
    OMXCoreIfaceTabletmp._OMX_GetRolesOfComponent = (pfnOMX_GetRolesOfComponent)dlsym(pOMXCoreIfaceTableInst->pHandle, "Mtk_OMX_GetRolesOfComponent"); 
    if((NULL ==     OMXCoreIfaceTabletmp._OMX_Init) ||
        (NULL ==     OMXCoreIfaceTabletmp._OMX_Deinit) ||
        (NULL ==     OMXCoreIfaceTabletmp._OMX_ComponentNameEnum) ||
        (NULL ==     OMXCoreIfaceTabletmp._OMX_GetHandle ) ||
        (NULL ==     OMXCoreIfaceTabletmp._OMX_FreeHandle ) ||
        (NULL ==     OMXCoreIfaceTabletmp._OMX_GetComponentsOfRole) ||
        (NULL ==     OMXCoreIfaceTabletmp._OMX_GetRolesOfComponent)){
        UT_LOG_I(printf("Fail to get OMX core interfaces\n");)
        goto GoOut;
    }
    */
    #endif
    
    UT_LOG_D(printf("Start -- Decode UT\n");)
    iIndexer = 0;
    #if !(USE_MULTITHREAD)
        iThreadIdx = 0;
        memset(&hThreadis, 0, sizeof(pthread_t)*ThreadCount);
    #endif
    while(TestArray[iIndexer].iTestFlag != 0){
        #if !(USE_MULTITHREAD)
        memset(pTestThreadDataInst, 0, iDataSize*ThreadCount);
        #endif
        UT_LOG_E(printf("[ Testing ] %s, width %d, height %d, bit-rate %d, fps %d, total frames %d\n", TestArray[iIndexer].pStrInputYUVPathFileName, 
                                                                                                                                                              TestArray[iIndexer].width ,
                                                                                                                                                              TestArray[iIndexer].height, 
                                                                                                                                                              TestArray[iIndexer].bitrate, 
                                                                                                                                                              TestArray[iIndexer].fps,
                                                                                                                                                              TestArray[iIndexer].frame_to_encode);)
        pTestThreadDataInst[iThreadIdx].pEncoderTestConfigInst = &TestArray[iIndexer];
        pEncoderTestConfigInst = pTestThreadDataInst[iThreadIdx].pEncoderTestConfigInst;
        pTestThreadDataInst[iThreadIdx].iIndexer = iIndexer;
        
        // The encoding loop.
        #if !(USE_MULTITHREAD)
            #if !(EXTERNAL_LOAD_SO)
            memset(&pTestThreadDataInst->OMXCoreIfaceTableInst, 0, sizeof(OMXCoreIfaceTable));
            pTestThreadDataInst->OMXCoreIfaceTableInst.pHandle = pOMX_CORE_handle;
            /*
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_Init = OMXCoreIfaceTabletmp._OMX_Init;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_Deinit = OMXCoreIfaceTabletmp._OMX_Deinit;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_ComponentNameEnum = OMXCoreIfaceTabletmp._OMX_ComponentNameEnum;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_GetHandle = OMXCoreIfaceTabletmp._OMX_GetHandle;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_FreeHandle = OMXCoreIfaceTabletmp._OMX_FreeHandle;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_GetComponentsOfRole = OMXCoreIfaceTabletmp._OMX_GetComponentsOfRole;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_GetRolesOfComponent = OMXCoreIfaceTabletmp._OMX_GetRolesOfComponent;
            */
            #endif
            gettimeofday(&timeis,NULL);
            dt1 = tdbl(&timeis);
            if(OMX_TEST_H264Enc(&pTestThreadDataInst[iThreadIdx]) > 0){
                    gettimeofday(&timeis,NULL);
                    dt2 = tdbl(&timeis);        
                    dt_diff = dt2 - dt1;
                    UT_LOG_E(printf("[ Result ] %s, total frames %d, total time %5.3f, Actual encode fps %5.3f\n\n", 
                            pEncoderTestConfigInst->pStrInputYUVPathFileName, 
                            pEncoderTestConfigInst->frame_to_encode,
                            (dt_diff),
                            (double)pEncoderTestConfigInst->frame_to_encode/dt_diff);)
            // compare golden.
            
            }
            else{
                UT_LOG_E(printf("**** Test fail!! %s, idx %d ****", pEncoderTestConfigInst->pStrInputYUVPathFileName, iIndexer);)
            }
        #else
            UT_LOG_D(printf("Use Multi-thread\n");)
            #if !(EXTERNAL_LOAD_SO)
            memset(&pTestThreadDataInst[iThreadIdx].OMXCoreIfaceTableInst, 0, sizeof(OMXCoreIfaceTable));
            pTestThreadDataInst[iThreadIdx].OMXCoreIfaceTableInst.pHandle = pOMX_CORE_handle;
            /*
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_Init = OMXCoreIfaceTabletmp._OMX_Init;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_Deinit = OMXCoreIfaceTabletmp._OMX_Deinit;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_ComponentNameEnum = OMXCoreIfaceTabletmp._OMX_ComponentNameEnum;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_GetHandle = OMXCoreIfaceTabletmp._OMX_GetHandle;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_FreeHandle = OMXCoreIfaceTabletmp._OMX_FreeHandle;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_GetComponentsOfRole = OMXCoreIfaceTabletmp._OMX_GetComponentsOfRole;
            pTestThreadDataInst->OMXCoreIfaceTableInst._OMX_GetRolesOfComponent = OMXCoreIfaceTabletmp._OMX_GetRolesOfComponent;
            */
            #endif
            pTestThreadDataInst[iThreadIdx].iActivate = 1;
            iRet = pthread_create(&pTestThreadDataInst[iThreadIdx].hThreadis, NULL, UT_TestThread, (void *)&pTestThreadDataInst[iThreadIdx]);
            if(iRet != 0){
                UT_LOG_E(printf("Fail to create Test thread!\n");)
                goto GoOut;
            }
            hThreadis[iThreadIdx] = pTestThreadDataInst[iThreadIdx].hThreadis;
            
            TryAgain:
            //search a empty thread slot for
            for(iIdxTmp = 0;   iIdxTmp < ThreadCount; iIdxTmp++){
                pTestThreadDataTmp = &pTestThreadDataInst[iIdxTmp];
                if(0 == pTestThreadDataTmp->iActivate){
                    //got one
                    break;
                }
                
            }
            if( iIdxTmp < ThreadCount){//found
                UT_LOG_D(printf("Find a available slot %d\n", iIdxTmp);)
                iRet = pthread_join(hThreadis[iIdxTmp], (void **) &iStatus);
                if(iRet != 0){
                    UT_LOG_W(printf("Test thread -> Error joining thread %d\n", (int)&hThreadis[iIdxTmp]);)
                }
                else{
                    UT_LOG_I(printf("Test thread -> Status %d\n", iStatus);)
                }

                iThreadIdx = iIdxTmp;
            }
            else{//no free try again
                UT_LOG_I(printf("NO FREE >> TICK \n");)
                usleep(1000000);
                goto TryAgain;
            }
            
        #endif
        
        iIndexer++; // goto next 
    }

    #if (USE_MULTITHREAD)
    for(iIdxTmp = 0;   iIdxTmp < ThreadCount; iIdxTmp++){
        iRet = pthread_join(hThreadis[iIdxTmp], (void **) &iStatus);
                if(iRet != 0){
                    UT_LOG_W(printf("Test thread -> Error joining thread %d\n", (int)&hThreadis[iIdxTmp]);)
                }
                else{
                    UT_LOG_I(printf("Test thread -> Status %d\n", iStatus);)
                }
        }
        #if (ThreadCount > 1)
        // release omx component
        if(OMX_ErrorNone != pTestThreadDataInst[0].OMXCoreIfaceTableInst._OMX_FreeHandle(pTestThreadDataInst[0].OMXCoreIfaceTableInst.CompHandle)){
            goto GoOut;
        }
        UT_LOG_E(printf("(multi-thread)Success to release OMX component\n");)
        //de-init OMX core
        if(OMX_ErrorNone != pTestThreadDataInst[0].OMXCoreIfaceTableInst._OMX_Deinit()){
            goto GoOut;
        }
        UT_LOG_E(printf("(multi-thread)Success de-init OMX core\n");)
        #endif
    #endif
    #if !(EXTERNAL_LOAD_SO)
    if(NULL != pOMX_CORE_handle){
        dlclose(pOMX_CORE_handle);
    }
    #endif
GoOut:
    if(NULL != pTestThreadDataInst){
        free(pTestThreadDataInst);
    }
    #if !(EXTERNAL_LOAD_SO)
    if(NULL != pOMX_CORE_handle){
        dlclose(pOMX_CORE_handle);
    }
    #endif
    UT_LOG_D(printf("End -- Decode UT\n");)
    return 1;
}

int H264_GetYUVData(unsigned int iFrameIdx, pTestThreadData pTestThreadDataInst, unsigned char *pYUVBuffer)
{
    unsigned char *pStrYStartAddr;
    unsigned char *pStrUStartAddr;
    unsigned char *pStrVStartAddr;
    unsigned int uiYUVSize = 0;
    unsigned int uiReadSize = 0;
    unsigned int uiYPlanSize = 0;
    pEncoderTestConfig pEncoderTestConfigInst;
    if((NULL == pTestThreadDataInst) || (NULL == pYUVBuffer)) return -1;
    if(NULL == pTestThreadDataInst->pInFile){
        UT_LOG_E(printf("There is no input YUV file\n");)
        return -2;
    }
    pEncoderTestConfigInst = pTestThreadDataInst->pEncoderTestConfigInst;

    pStrYStartAddr = pYUVBuffer;
    pStrUStartAddr = pStrYStartAddr + (pEncoderTestConfigInst->height*pEncoderTestConfigInst->width);
    pStrVStartAddr = pStrUStartAddr + (pEncoderTestConfigInst->height*pEncoderTestConfigInst->width/4);
    uiYUVSize = pEncoderTestConfigInst->height*pEncoderTestConfigInst->width*3/2;
    uiYPlanSize = pEncoderTestConfigInst->height*pEncoderTestConfigInst->width;
    
    UT_LOG_D(printf("Get YUVData Idx %d, addr 0x%08X\n",iFrameIdx, (unsigned int)pYUVBuffer);)

    if( 0 != fseek(pTestThreadDataInst->pInFile, uiYUVSize*iFrameIdx, SEEK_SET)){
        UT_LOG_W(printf("Fail to Seek file\n");)
    }

    uiReadSize = fread(pStrYStartAddr, 1, uiYPlanSize, pTestThreadDataInst->pInFile);
    if(uiReadSize  != uiYPlanSize){
        UT_LOG_W(printf("Read Y plan fail\n");)
    }

    uiReadSize = fread(pStrUStartAddr, 1, uiYPlanSize/4, pTestThreadDataInst->pInFile);
    if(uiReadSize  != (uiYPlanSize/4)){
        UT_LOG_W(printf("Read U plan fail\n");)
    }
    
    uiReadSize = fread(pStrVStartAddr, 1, uiYPlanSize/4, pTestThreadDataInst->pInFile);
    if(uiReadSize  != (uiYPlanSize/4)){
        UT_LOG_W(printf("Read V plan fail\n");)
    }
    return 1;
}


//return value is the index number
int FindMatchingBufferHdr(int port_index, OMX_BUFFERHEADERTYPE* pBufHdr, pOMXCoreIfaceTable pOMXCoreIfaceTableInst) 
{
    int i;
    if((NULL == pBufHdr) || (NULL == pOMXCoreIfaceTableInst)){
        return -2;
    }
    if (port_index == INPUT_PORT_INDEX) {
        for (i = 0 ; i < H264_NUM_INPUT_BUF ; i++) {
            if (pBufHdr == pOMXCoreIfaceTableInst->pInBufHdrs[i]) {
                return i;
            }
        }
    }
    else if (port_index == OUTPUT_PORT_INDEX) {
        for (i = 0 ; i < H264_NUM_OUTPUT_BUF ; i++) {
            if (pBufHdr == pOMXCoreIfaceTableInst->pOutBufHdrs[i]) {
                return i;
            }
        }
    } 
    return -1;
}

int FindAvailableBuffSlot(int port_index, pOMXCoreIfaceTable pOMXCoreIfaceTableInst){
    int i;
    if(NULL == pOMXCoreIfaceTableInst){
        UT_LOG_E(printf("input a empty pOMXCoreIfaceTable\n");)
        return -2;
    }
    
    if (port_index == INPUT_PORT_INDEX) {
        for (i = 0 ; i < H264_NUM_INPUT_BUF ; i++) {
            if (BOS_IL_CLIENT == pOMXCoreIfaceTableInst->InBufOwner[i]) {
                UT_LOG_I(printf("Find IN buffer idx %d\n", i);)
                return i;
            }
        }
    }
    else if (port_index == OUTPUT_PORT_INDEX) {
        for (i = 0 ; i < H264_NUM_OUTPUT_BUF ; i++) {
            if (BOS_IL_CLIENT == pOMXCoreIfaceTableInst->OutBufOwner[i]) {
                return i;
            }
        }
    }
    return -1;        
}

int CheckAvailableBuffSlot(pOMXCoreIfaceTable pOMXCoreIfaceTableInst){
    int i;
    int inBufferCount = 0;
    int outBufferCount = 0;
    if(NULL == pOMXCoreIfaceTableInst){
        UT_LOG_E(printf("input a empty pOMXCoreIfaceTable\n");)
        return -2;
    }

    inBufferCount = 0;
    for (i = 0 ; i < H264_NUM_INPUT_BUF ; i++) {
        if (BOS_IL_CLIENT == pOMXCoreIfaceTableInst->InBufOwner[i]) {
           inBufferCount++;
        }
    }
    outBufferCount = 0;
    for (i = 0 ; i < H264_NUM_OUTPUT_BUF ; i++) {
        if (BOS_IL_CLIENT == pOMXCoreIfaceTableInst->OutBufOwner[i]) {
            outBufferCount++;
        }
    }
    UT_LOG_D(printf("@@ _Available IN buffer (%d) : OUT buffer (%d)\n", inBufferCount, outBufferCount);)
    return 0;        
}


const char* StateToString(OMX_U32 state) {   
    switch (state) {        
        case OMX_StateInvalid:            
            return ">> Invalid <<";        
        case OMX_StateLoaded:            
            return ">> OMX_StateLoaded <<";        
        case OMX_StateIdle:            
            return ">> OMX_StateIdle <<";        
        case OMX_StateExecuting:            
            return ">> OMX_StateExecuting <<";        
        case OMX_StatePause:            
            return ">> OMX_StatePause <<";        
        case OMX_StateWaitForResources:            
            return ">> OMX_StateWaitForResources <<";        
        default:            
            return ">> Unknown";    
    }           
}



OMX_ERRORTYPE OMX_H264_Enc_EventHandler(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_EVENTTYPE eEvent, OMX_U32 nData1, OMX_U32 nData2, OMX_PTR pEventData)
{
    VENC_UNUSED(hComponent);
    VENC_UNUSED(pEventData);

    pTestThreadData pTestThreadDataInst = (pTestThreadData)pAppData;
    if(NULL == pAppData){
        UT_LOG_D(printf("WARRING!! <<EmptyBufferDone>> AppData empty\n");)
    }
    if(MAGIC_NUMBER_H264 != pTestThreadDataInst->iMagicNumber){
        UT_LOG_D(printf("WARRING!! <<EmptyBufferDone>> Wrong Magic number\n");)
    }
    UT_LOG_I(printf("OMX_H264_Enc_EventHandler invoked >>");)
    switch (eEvent) {
        case OMX_EventCmdComplete:
            if (nData1 == OMX_CommandStateSet) {
                UT_LOG_I(printf("OMX_EventCmdComplete [OMX_CommandStateSet: %s]\r\n", StateToString(nData2));)
                switch(nData2){
                    case OMX_StateInvalid:            
                        pTestThreadDataInst->state = OMX_StateInvalid;
                        break;
                    case OMX_StateLoaded:            
                        pTestThreadDataInst->state = OMX_StateLoaded;
                        break;
                    case OMX_StateIdle:            
                        pTestThreadDataInst->state = OMX_StateIdle;
                        break;
                    case OMX_StateExecuting:            
                        pTestThreadDataInst->state = OMX_StateExecuting;
                        break;
                    case OMX_StatePause:            
                        pTestThreadDataInst->state = OMX_StatePause;
                        break;
                    case OMX_StateWaitForResources:            
                        pTestThreadDataInst->state = OMX_StateWaitForResources; 
                        break;
                    default:            
                        UT_LOG_W(printf("@@!! unkonw State !!!\n");)
                        break;
                }
            }
            else if (nData1 == OMX_CommandPortDisable) {
                UT_LOG_D(printf("OMX_EventCmdComplete [OMX_CommandPortDisable: nPortIndex(%d)]\r\n", (int)nData2);)
            }
            else if (nData1 == OMX_CommandPortEnable) {
                UT_LOG_D(printf("OMX_EventCmdComplete [OMX_CommandPortEnable: nPortIndex(%d)]\r\n", (int)nData2);)
            }
            else if (nData1 == OMX_CommandFlush) {
                UT_LOG_D(printf("OMX_EventCmdComplete [OMX_CommandFlush: nPortIndex(%d)]\r\n", (int)nData2);)
            }
            break;
        case OMX_EventError:
            UT_LOG_D(printf("OMX_EventError (0x%08X)\r\n", (int)nData1);)
            break;
        default:
            UT_LOG_W(printf("Unknow Event (0x%08X)\r\n", (int)eEvent);)
            break;
        }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_H264_Enc_EmptyBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    VENC_UNUSED(hComponent);
    int iIndexOfBuffer = -1;
    int tryCout = 0;
    pOMXCoreIfaceTable  pOMXCoreIfaceTableInst = NULL;
    pTestThreadData pTestThreadDataInst = (pTestThreadData)pAppData;
    if(NULL == pAppData){
        UT_LOG_W(printf("WARRING!! <<EmptyBufferDone>> AppData empty\n");)
    }
    if(MAGIC_NUMBER_H264 != pTestThreadDataInst->iMagicNumber){
        UT_LOG_W(printf("WARRING!! <<EmptyBufferDone>> Wrong Magic number\n");)
    }
    pOMXCoreIfaceTableInst = &pTestThreadDataInst->OMXCoreIfaceTableInst;
    UT_LOG_D(printf(">>[%d:%d] 0x%08X OMX_H264_Enc_EmptyBufferDone invoked, buffer addr 0x%08X\n",getpid(), gettid(),pOMXCoreIfaceTableInst, (unsigned int)pBuffer->pBuffer);)

    //get the buffer from list, then mark as avaiable for OMX IL client
    iIndexOfBuffer = FindMatchingBufferHdr(INPUT_PORT_INDEX, pBuffer, pOMXCoreIfaceTableInst);
    if(iIndexOfBuffer>=0 && iIndexOfBuffer <H264_NUM_INPUT_BUF){
        //set owner as OMX IL Client, the the main loop will re-use this buffer
        if(BOS_OMX_COMPONENT != pOMXCoreIfaceTableInst->InBufOwner[iIndexOfBuffer]){
            UT_LOG_E(printf("@@@@ IN %d\n", pOMXCoreIfaceTableInst->InBufOwner[iIndexOfBuffer]);)
        }
        pOMXCoreIfaceTableInst->InBufOwner[iIndexOfBuffer] = BOS_IL_CLIENT;
        UT_LOG_D(printf(">>[%d] Got return buffer Idx %d, for input YUV buffer\n", tryCout, iIndexOfBuffer);)
    }
    else{
        UT_LOG_E(printf("@@!!%s something wrong about the return buffer from OMX component!!\n", __FUNCTION__);)
    }
    
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_H264_Enc_FillBufferDone(OMX_HANDLETYPE hComponent, OMX_PTR pAppData, OMX_BUFFERHEADERTYPE* pBuffer)
{
    VENC_UNUSED(hComponent);
    int iIndexOfBuffer = -1;
    int tryCout = 0;
    pOMXCoreIfaceTable  pOMXCoreIfaceTableInst = NULL;
    pTestThreadData pTestThreadDataInst = (pTestThreadData)pAppData;
    if(NULL == pAppData){
        UT_LOG_W(printf("WARRING!!<<FillBufferDone>> AppData empty\n");)
        return OMX_ErrorNone;
    }
    if(MAGIC_NUMBER_H264 != pTestThreadDataInst->iMagicNumber){
        UT_LOG_W(printf("WARRING!! <<EmptyBufferDone>> Wrong Magic number\n");)
        return OMX_ErrorNone;
    }
    pOMXCoreIfaceTableInst = &pTestThreadDataInst->OMXCoreIfaceTableInst;
    UT_LOG_D(printf("<<[%d:%d] 0x%08X, OMX_H264_Enc_FillBufferDone invoked, buffer addr 0x%08X, size %d\n",getpid(), gettid(),pOMXCoreIfaceTableInst, (unsigned int)pBuffer->pBuffer, (int)pBuffer->nFilledLen);)

    //get the buffer from list, then mark as avaiable for OMX IL client
    iIndexOfBuffer = FindMatchingBufferHdr(OUTPUT_PORT_INDEX, pBuffer, pOMXCoreIfaceTableInst);
    if(iIndexOfBuffer>=0 && iIndexOfBuffer <H264_NUM_OUTPUT_BUF){
        //output bit-stream
        if(NULL != pTestThreadDataInst->pOutFile){
            fwrite(pBuffer->pBuffer, 1, pBuffer->nFilledLen, pTestThreadDataInst->pOutFile);
            UT_LOG_D(printf("Write Out bit-stream %d\n", (int)pBuffer->nFilledLen);)
        }
        //set owner as OMX IL Client, the the main loop will re-use this buffer
        if(BOS_OMX_COMPONENT != pOMXCoreIfaceTableInst->OutBufOwner[iIndexOfBuffer]){
            UT_LOG_E(printf("@@@@ OUT\n");)
        }
        pOMXCoreIfaceTableInst->OutBufOwner[iIndexOfBuffer] = BOS_IL_CLIENT;
        UT_LOG_D(printf("<< Got return buffer Idx %d, for output bit-stream\n", iIndexOfBuffer);)
    }
    else{
        UT_LOG_E(printf("@@!!%s something wrong about the return buffer from OMX component!!\n", __FUNCTION__);)
    }    
    
    return OMX_ErrorNone;
}


int OMX_TEST_H264Enc(pTestThreadData pTestThreadDataInst)
{
    int  iRetValue = 0;
    int iRet = 0;
    OMX_STATETYPE state;
    OMX_ERRORTYPE err;
    pOMXCoreIfaceTable  pOMXCoreIfaceTableInst = NULL;
    pEncoderTestConfig pEncoderTestConfigInst = NULL;
    unsigned int uiFrameIndexer = 0;
    unsigned int uiOutputFrameBufferCount = 0;
    unsigned int uiIndexer = 0;
    int iBuffIndexer = 0;
    unsigned char *pBufferYUV = NULL;
    unsigned int uiYUVSize = 0;

    if(NULL == pTestThreadDataInst){
        iRetValue = -1;
        //goto GOOUT;
        return -1;
    }
    if(NULL == pTestThreadDataInst->pEncoderTestConfigInst){
        iRetValue = -2;
        goto GOOUT;
    }

    pEncoderTestConfigInst = pTestThreadDataInst->pEncoderTestConfigInst;

    pOMXCoreIfaceTableInst = &pTestThreadDataInst->OMXCoreIfaceTableInst;

    //init mutex
    pthread_mutex_init(&pTestThreadDataInst->hMutexInputAccess, NULL);
    pthread_mutex_init(&pTestThreadDataInst->hMutexOutAccess, NULL);
    pTestThreadDataInst->iMagicNumber = MAGIC_NUMBER_H264;

    pTestThreadDataInst->state = OMX_StateInvalid;

    //open input and output files
    pTestThreadDataInst->pInFile = fopen(pEncoderTestConfigInst->pStrInputYUVPathFileName, "rb");
    if(NULL == pTestThreadDataInst->pInFile){
        UT_LOG_W(printf("@@ Fail to open input file %s\n", pEncoderTestConfigInst->pStrInputYUVPathFileName);)
    }   
    #if OUTPUT_BIT_STREAM
        pTestThreadDataInst->pOutFile = fopen(pEncoderTestConfigInst->pStrOutputBitStreamPathFileName, "wb");
        if(NULL == pTestThreadDataInst->pOutFile){
            UT_LOG_W(printf("@@ Fail to open output file %s\n", pEncoderTestConfigInst->pStrOutputBitStreamPathFileName);)
        }
    #else
        pTestThreadDataInst->pOutFile = NULL;
    #endif    

    uiYUVSize = pEncoderTestConfigInst->height * pEncoderTestConfigInst->width *3/2;

    //configure the OMX test
    if(LoadOMXCore(OMX_CORE_PATHFILENAME, pOMXCoreIfaceTableInst)> 0){
        //init OMX core
        if(OMX_ErrorNone != pOMXCoreIfaceTableInst->_OMX_Init()){
            iRetValue = -10;
            goto GOOUT;
        }
        UT_LOG_I(printf("Success init OMX core\n");)

        //set callback functions for omx component
        pOMXCoreIfaceTableInst->OMX_CallBack.EventHandler = OMX_H264_Enc_EventHandler;
        pOMXCoreIfaceTableInst->OMX_CallBack.EmptyBufferDone = OMX_H264_Enc_EmptyBufferDone;
        pOMXCoreIfaceTableInst->OMX_CallBack.FillBufferDone = OMX_H264_Enc_FillBufferDone;
        iRet = GetOmxComponentHandle(TARGET_TEST_OMX_COMPONENT, pOMXCoreIfaceTableInst, (void *)pTestThreadDataInst);
        if(iRet <= 0){
            iRetValue = -20;
            goto GOOUT;
        }
        UT_LOG_I(printf("Success to get OMX component handle\n");)

        pOMXCoreIfaceTableInst->height = pTestThreadDataInst->pEncoderTestConfigInst->height;
        pOMXCoreIfaceTableInst->width  = pTestThreadDataInst->pEncoderTestConfigInst->width;
        pOMXCoreIfaceTableInst->bitrate = pTestThreadDataInst->pEncoderTestConfigInst->bitrate;
        pOMXCoreIfaceTableInst->fps = pTestThreadDataInst->pEncoderTestConfigInst->fps;
        pOMXCoreIfaceTableInst->i_keyint_max = pTestThreadDataInst->pEncoderTestConfigInst->i_keyint_max;    

        if(OMX_ErrorNone != OMX_GetState(pOMXCoreIfaceTableInst->CompHandle,&state)){
            UT_LOG_E(printf("Fail to Get OMX component state\n");)
        }
        else{
            UT_LOG_D(printf("OMX State = %s\n", StateToString(state));)
        }

        // configure the OMX component
        iRet = ConfigOmxComponentPorts(pOMXCoreIfaceTableInst);
        if(iRet <= 0){
            iRetValue = -30;
            goto GOOUT;
        }

        //wait state to be idle state
        while(OMX_StateIdle != pTestThreadDataInst->state){
            usleep(10000);//wait 10ms
        }

        #if 1  //test port flush
        err = OMX_SendCommand(pOMXCoreIfaceTableInst->CompHandle, OMX_CommandFlush, OMX_ALL, NULL);
        if (OMX_ErrorNone != err) {
            UT_LOG_E(printf("OMX_SendCommand(OMX_CommandFlush, OMX_ALL) error (0x%08X)\r\n", err);)
        }
        usleep(10000);
        #endif

        err = OMX_SendCommand(pOMXCoreIfaceTableInst->CompHandle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
        if (OMX_ErrorNone != err) {
            UT_LOG_E(printf("OMX_SendCommand(OMX_CommandStateSet, OMX_StateExecuting) error (0x%08X)\r\n", err);)
        }
        
        //wait state to be OMX_StateExecuting state
        while(OMX_StateExecuting != pTestThreadDataInst->state){
            usleep(10000);//wait 10ms
        }

        // There is loop to test the OMX component
        #if 0
        usleep(10*1e6);
        #else
        // has input file, some below will do encoding test
        if(NULL != pTestThreadDataInst->pInFile){
            uiFrameIndexer = 0;
            //put all input buffer to OMX component
            #if 0
            for(uiIndexer= 0; uiIndexer < H264_NUM_OUTPUT_BUF; uiIndexer++){
               // read YUV data
                pBufferYUV = pOMXCoreIfaceTableInst->pInBufHdrs[uiIndexer]->pBuffer;
                if(H264_GetYUVData(uiFrameIndexer, pTestThreadDataInst, pBufferYUV)>0){
                    // send buffer to OMX component
                    pOMXCoreIfaceTableInst->pInBufHdrs[uiIndexer]->nTimeStamp = 33+33*uiFrameIndexer;
                    pOMXCoreIfaceTableInst->pInBufHdrs[uiIndexer]->nFilledLen = uiYUVSize;
                    err = OMX_EmptyThisBuffer(pOMXCoreIfaceTableInst->CompHandle, pOMXCoreIfaceTableInst->pInBufHdrs[uiIndexer]);
                    if (OMX_ErrorNone != err) {
                        UT_LOG_E(printf("OMX_EmptyThisBuffer[%d] error (0x%08X)\n", uiIndexer, err);)
                    }
                    //set this version owner as component
                    UT_LOG_D(printf("<< OMX_EmptyThisBuffer >> [%d] buffer Addr (0x%08X)\n",uiIndexer, (unsigned int)pOMXCoreIfaceTableInst->pInBufHdrs[uiIndexer]->pBuffer);)
                    pOMXCoreIfaceTableInst->InBufOwner[uiIndexer] = BOS_OMX_COMPONENT;
                    //set next YUV frame
                    uiFrameIndexer++;
                 }
                else{
                    UT_LOG_E(printf("@@Fail to READ YUV Data >> SOMETHING WRONG!!!\n");)
                    break;
                }
            }
            #endif
            //put all output buffer to OMX component
            for(uiIndexer= 0; uiIndexer < H264_NUM_OUTPUT_BUF; uiIndexer++){
               err = OMX_FillThisBuffer(pOMXCoreIfaceTableInst->CompHandle, pOMXCoreIfaceTableInst->pOutBufHdrs[uiIndexer]);
                if (OMX_ErrorNone != err) {
                    UT_LOG_E(printf("OMX_FillThisBuffer[%d] error (0x%08X)\r\n", uiIndexer, err);)
                } 
                UT_LOG_D(printf("OMX_FillThisBuffer >> [%d] buffer Addr (0x%08X)\n",uiIndexer, pOMXCoreIfaceTableInst->pOutBufHdrs[uiIndexer]->pBuffer);)
                pOMXCoreIfaceTableInst->OutBufOwner[uiIndexer] = BOS_OMX_COMPONENT;
                uiOutputFrameBufferCount++;
            }
            
            UT_LOG_D(printf("PID:TID[%d:%d] 0x%08X\n",getpid(), gettid(), pOMXCoreIfaceTableInst);)
            
            while(1){
                CheckAvailableBuffSlot(pOMXCoreIfaceTableInst);
                //get input YUV data
                iBuffIndexer = FindAvailableBuffSlot(INPUT_PORT_INDEX, pOMXCoreIfaceTableInst);
                if(iBuffIndexer>=0 && iBuffIndexer <H264_NUM_INPUT_BUF){
                    //got buffer
                    UT_LOG_D(printf(">> Got an available buffer idx %d: %d\n", iBuffIndexer, FindMatchingBufferHdr(INPUT_PORT_INDEX, pOMXCoreIfaceTableInst->pInBufHdrs[iBuffIndexer], pOMXCoreIfaceTableInst));)
                    // read YUV data
                    pBufferYUV = pOMXCoreIfaceTableInst->pInBufHdrs[iBuffIndexer]->pBuffer;
                    if(H264_GetYUVData(uiFrameIndexer, pTestThreadDataInst, pBufferYUV)>0){
                        // send buffer to OMX component
                        pOMXCoreIfaceTableInst->pInBufHdrs[iBuffIndexer]->nTimeStamp = 33+33*uiFrameIndexer;
                        pOMXCoreIfaceTableInst->pInBufHdrs[iBuffIndexer]->nFilledLen = uiYUVSize;
                        //set this version owner as component
                        UT_LOG_D(printf("<< OMX_EmptyThisBuffer >> [%d] buffer Addr (0x%08X)\n",iBuffIndexer, (unsigned int)pOMXCoreIfaceTableInst->pInBufHdrs[iBuffIndexer]->pBuffer);)
                        pOMXCoreIfaceTableInst->InBufOwner[iBuffIndexer] = BOS_OMX_COMPONENT;
                        err = OMX_EmptyThisBuffer(pOMXCoreIfaceTableInst->CompHandle, pOMXCoreIfaceTableInst->pInBufHdrs[iBuffIndexer]);
                        if (OMX_ErrorNone != err) {
                            UT_LOG_E(printf("OMX_EmptyThisBuffer[%d] error (0x%08X)\n", iBuffIndexer, err);)
                        }
                        //set next YUV frame
                        uiFrameIndexer++;
                     }
                    else{
                        UT_LOG_E(printf("Fail to READ YUV Data >> SOMETHING WRONG!!!\n");)
                        break;
                    }
                }
                else{
                    //no free buffer
                    UT_LOG_D(printf("<< There is no free buffer for input YUV >> SOMETHING WRONG???\n");) //just no free buffer
                    usleep(100000);// poll next time (100 ms later);
                }
                // get available output bit-stream data and give OMX component again.
                iBuffIndexer = FindAvailableBuffSlot(OUTPUT_PORT_INDEX, pOMXCoreIfaceTableInst);
                if((iBuffIndexer>=0 && iBuffIndexer <H264_NUM_OUTPUT_BUF) && (uiOutputFrameBufferCount < (pEncoderTestConfigInst->frame_to_encode +1))){//add one for SPS+PPS frame output
                    pOMXCoreIfaceTableInst->OutBufOwner[iBuffIndexer] = BOS_OMX_COMPONENT;
                    uiOutputFrameBufferCount++;
                    err = OMX_FillThisBuffer(pOMXCoreIfaceTableInst->CompHandle, pOMXCoreIfaceTableInst->pOutBufHdrs[iBuffIndexer]);
                    if (OMX_ErrorNone != err) {
                        UT_LOG_E(printf("OMX_FillThisBuffer[%d] error (0x%08X)\r\n", iBuffIndexer, err);)
                    } 
                    UT_LOG_D(printf("<< OMX_FillThisBuffer >> [%d] buffer Addr (0x%08X)\n",iBuffIndexer, (unsigned int)pOMXCoreIfaceTableInst->pOutBufHdrs[iBuffIndexer]->pBuffer);)                    
                }
                else{
                    UT_LOG_D(printf("<< There is no free buffer for output bit-stream Data >> SOMETHING WRONG???\n");) //just no free buffer
                }
                
                //jump out condition
                //if(uiFrameIndexer > 20){//pEncoderTestConfigInst->frame_to_encode
                if(uiFrameIndexer >= pEncoderTestConfigInst->frame_to_encode){//
                    break;  // just test to encode first 10 frames
                }
            }
        }
        #endif

        UT_LOG_I(printf(">>> Destroy OMX component\n");)

         #if 0  //test port flush
        err = OMX_SendCommand(pOMXCoreIfaceTableInst->CompHandle, OMX_CommandFlush, OMX_ALL, NULL);
        if (OMX_ErrorNone != err) {
            UT_LOG_E(printf("OMX_SendCommand(OMX_CommandFlush, OMX_ALL) error (0x%08X)\r\n", err);)
        }
        #endif

        #if 1
        //disable port should be done before FreeBuffer
        err = OMX_SendCommand(pOMXCoreIfaceTableInst->CompHandle, OMX_CommandPortDisable, INPUT_PORT_INDEX, NULL);    
        if (OMX_ErrorNone != err) {
            UT_LOG_E(printf("OMX_SendCommand(OMX_CommandPortDisable) error (0x%08X)\r\n", err);)
        }
        err = OMX_SendCommand(pOMXCoreIfaceTableInst->CompHandle, OMX_CommandPortDisable, OUTPUT_PORT_INDEX, NULL);    
        if (OMX_ErrorNone != err) {
            UT_LOG_E(printf("OMX_SendCommand(OMX_CommandPortDisable) error (0x%08X)\r\n", err);)
        }
        // free buffer
        for (uiIndexer = 0 ; uiIndexer < H264_NUM_INPUT_BUF ; uiIndexer++) {
             err = OMX_FreeBuffer(pOMXCoreIfaceTableInst->CompHandle, INPUT_PORT_INDEX, pOMXCoreIfaceTableInst->pInBufHdrs[uiIndexer]);
             if (OMX_ErrorNone != err) {
                 UT_LOG_E(printf("OMX_FreeBuffer intput port error idx %d (0x%08X)\r\n",uiIndexer, err);)
        
             }
         }
         
         for (uiIndexer = 0 ; uiIndexer < H264_NUM_OUTPUT_BUF ; uiIndexer++) {
             err = OMX_FreeBuffer(pOMXCoreIfaceTableInst->CompHandle,  OUTPUT_PORT_INDEX, pOMXCoreIfaceTableInst->pOutBufHdrs[uiIndexer]);        
             if (OMX_ErrorNone != err) { 
                 UT_LOG_E(printf("OMX_FreeBuffer output port error idx %d (0x%08X)\r\n", uiIndexer, err);)
             }
         }
        #endif

        // set commponent to idle state
        if(OMX_ErrorNone != OMX_SendCommand(pOMXCoreIfaceTableInst->CompHandle, OMX_CommandStateSet, OMX_StateIdle, NULL)){
            UT_LOG_E(printf("Fail to set OMX component to Loaded state\n");)   
        }

        //wait state to be idle state
        while(OMX_StateIdle != pTestThreadDataInst->state){
            usleep(10000);//wait 10ms
        }

        // set commponent to loaded state
        if(OMX_ErrorNone != OMX_SendCommand(pOMXCoreIfaceTableInst->CompHandle, OMX_CommandStateSet, OMX_StateLoaded, NULL)){
            UT_LOG_E(printf("Fail to set OMX component to Loaded state\n");)
        }

        //wait state to be idle state
        while(OMX_StateLoaded != pTestThreadDataInst->state){
            usleep(10000);//wait 10ms
        }
        #if !(USE_MULTITHREAD) || ( (USE_MULTITHREAD) && (ThreadCount <= 1))
        // release omx component
        if(OMX_ErrorNone != pOMXCoreIfaceTableInst->_OMX_FreeHandle(pOMXCoreIfaceTableInst->CompHandle)){
            iRetValue = -99;
            goto GOOUT;
        }
        UT_LOG_D(printf("Success to release OMX component\n");)
        pOMXCoreIfaceTableInst->CompHandle = 0x0;
        //de-init OMX core
        if(OMX_ErrorNone != pOMXCoreIfaceTableInst->_OMX_Deinit()){
            iRetValue = -100;
            goto GOOUT;
        }
        UT_LOG_D(printf("Success de-init OMX core\n");)
        #endif
    }
    //release resources of OMX compoent, core and buffer pool
    ReleaseOMXCore(pOMXCoreIfaceTableInst);
    if(NULL != pTestThreadDataInst->pInFile){
        fclose(pTestThreadDataInst->pInFile);
        pTestThreadDataInst->pInFile = NULL;
    }
    if(NULL != pTestThreadDataInst->pOutFile ){
        fclose(pTestThreadDataInst->pOutFile );
        pTestThreadDataInst->pOutFile  = NULL;
    }
    pthread_mutex_destroy(&pTestThreadDataInst->hMutexInputAccess);
    pthread_mutex_destroy(&pTestThreadDataInst->hMutexOutAccess);
    return 1;
GOOUT:
    if(NULL != pOMXCoreIfaceTableInst->pHandle){
        ReleaseOMXCore(pOMXCoreIfaceTableInst);
    }
    if(NULL != pTestThreadDataInst->pInFile){
        fclose(pTestThreadDataInst->pInFile);
        pTestThreadDataInst->pInFile = NULL;
    }
    if(NULL != pTestThreadDataInst->pOutFile ){
        fclose(pTestThreadDataInst->pOutFile );
        pTestThreadDataInst->pOutFile  = NULL;
    }
    pthread_mutex_destroy(&pTestThreadDataInst->hMutexInputAccess);
    pthread_mutex_destroy(&pTestThreadDataInst->hMutexOutAccess);
    return iRetValue;
}

 // END OF FILE
