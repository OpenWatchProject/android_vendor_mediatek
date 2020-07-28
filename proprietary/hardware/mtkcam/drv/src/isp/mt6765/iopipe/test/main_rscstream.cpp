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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.

 
#define LOG_TAG "rscstream_test"

#include <vector>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/def/common.h>

//
#include <semaphore.h>
#include <pthread.h>
#include <utils/threads.h>
#include <mtkcam/drv/iopipe/PostProc/IRscStream.h>
#include <drv/rscunittest.h>

//#include <mtkcam/iopipe/PostProc/IFeatureStream.h>

//#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <drv/imem_drv.h>
//#include <mtkcam/drv/isp_drv.h>

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
//
#include "rsc/rsc_testcommon.h"
#if 0
#include "rsc/rsc_test_case_00/frame_0/rsc_test_case_00_frame_0_rsc_setting_00.h"
#include "rsc/rsc_test_case_00/frame_1/rsc_test_case_00_frame_1_rsc_setting_00.h"
#include "rsc/rsc_test_case_00/frame_2/rsc_test_case_00_frame_2_rsc_setting_00.h"
#include "rsc/rsc_test_case_00/frame_3/rsc_test_case_00_frame_3_rsc_setting_00.h"
#include "rsc/rsc_test_case_00/frame_4/rsc_test_case_00_frame_4_rsc_setting_00.h"
#include "rsc/rsc_test_case_00/MultiEnque_rsc_test_case_00_frame_2_rsc_setting_00.h"
#else
#include "rsc/rsc_test_case_00/MultiEnque_rsc_test_case_00_frame_0_rsc_setting_00.h"
#endif
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSRsc;

/******************************************************************************
* save the buffer to the file
*******************************************************************************/
/*MUINT32 comp_roi_mem_with_file(char* pGolden, short mode, MUINTPTR base_addr, int width, int height, MUINT32 stride,
 *                               int start_x, int start_y, int end_x, int end_y, MUINTPTR mask_base_addr, 
 *                               MUINT32 mask_size, MUINT32 mask_stride);
 */
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size);
/******************************************************************************
* RSC Test Case
*******************************************************************************/
#define EIS_OFFLINE
#define WINDOW
//#define DEFAULT

//#include "frame_0_rsc_setting_00.h"
extern "C" {
#include <rsc/rsc_default/frame_0/frame_0.h>
#include <rsc/rsc_test_case_00/frame_1/rsc_test_case_00_frame_1.h>
#ifndef WINDOW 
#include <rsc/eis_offline/flat_surface/eis_frame.h>
#else
#include <rsc/eis_offline/window/eis_frame.h>
#endif
}
#if 0
extern "C" void getframe_0GoldPointer(
    unsigned long* golden_rsc_dvo_l_frame,
    unsigned long* golden_rsc_dvo_r_frame,
    unsigned long* golden_rsc_confo_l_frame,
    unsigned long* golden_rsc_confo_r_frame,
    unsigned long* golden_rsc_respo_l_frame,
    unsigned long* golden_rsc_respo_r_frame,
    unsigned long* golden_rsc_wmf_dpo_frame_0,
    unsigned long* golden_rsc_wmf_dpo_frame_1,
    unsigned long* golden_rsc_wmf_dpo_frame_2
);
#endif

int rsc_default();

MBOOL g_bRSCCallback;

//typedef MVOID                   (*PFN_CALLBACK_T)(DVEParams& rParams);

MVOID RSCCallback(RSCParams& rParams);

pthread_t       RscUserThread;
sem_t           RscSem;
volatile bool            g_bRscThreadTerminated = 0;

/*******************************************************************************
*  Main Function 
*  
********************************************************************************/

#if 0 
void* RscThreadLoop(void *arg)
{
    int ret = 0; 

    ::pthread_detach(::pthread_self());
    ::sem_wait(&RscSem);

    while (g_bRscThreadTerminated) {

        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("Sub Thread do rsc_test_case_00_frame_3_RSC_Config\n");
        ret=rsc_test_case_00_frame_3_RSC_Config();
        printf("Sub Thread do rsc_test_case_00_frame_4_RSC_Config\n");
        ret=rsc_test_case_00_frame_4_RSC_Config();
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
    }
    ::sem_post(&RscSem);
    printf("###################Sub  Wait###################\n");
    printf("###################Sub  Wait###################\n");
    printf("###################Sub  Wait###################\n");
    return NULL;
}
void* RscThreadLoop_OneReqHaveOneBuffer(void *arg)
{
    int ret = 0; 

    ::pthread_detach(::pthread_self());
    ::sem_wait(&RscSem);

    while (g_bRscThreadTerminated) {

        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        ret = multi_enque_rsc_test_case_00_frame_2_RSC_Config();
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
    }
    ::sem_post(&RscSem);
    printf("###################Sub  Wait###################\n");
    printf("###################Sub  Wait###################\n");
    printf("###################Sub  Wait###################\n");
    return NULL;
}
void* RscThreadLoop_OneReqHaveTwoBuffer(void *arg)
{
    int ret = 0; 

    ::pthread_detach(::pthread_self());
    ::sem_wait(&RscSem);

    while (g_bRscThreadTerminated) {

        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        ret = multi_enque_rsc_test_case_00_frame_2_RSC_Config();
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
        printf("###################Sub  Thread###################\n");
    }
    ::sem_post(&RscSem);
    printf("###################Sub  Wait###################\n");
    printf("###################Sub  Wait###################\n");
    printf("###################Sub  Wait###################\n");
    return NULL;
}
#endif
int test_rscStream(int argc, char** argv)
{
    int ret = 0; 
    int testCase = 0;
    int i;
    if (argc>=1)
    {
        testCase = atoi(argv[0]);
    }

    switch(testCase)
    {
        case 0:  //Unit Test Case One RSC Request.
            g_RSC_UnitTest_Num = RSC_DEFAULT_UT; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            ret = rsc_default();
            break;
#if 0
        case 1: //Main and RscUserThread will do the one dve request and one wmfe request at the same time.
            ::sem_init(&RscSem,0, 0);
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            pthread_create(&RscUserThread, NULL, RscThreadLoop, (void*)NULL);
            g_bRscThreadTerminated = 1;
            g_bTestBlockingDeque = 0;
            ::sem_post(&RscSem);
            for (i=0;i<10;i++)
            {
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");

                printf("Main Thread do rsc_test_case_00_frame_0_RSC_Config\n");
                ret=rsc_test_case_00_frame_0_RSC_Config();
                printf("Main Thread do rsc_test_case_00_frame_1_RSC_Config\n");
                ret=rsc_test_case_00_frame_1_RSC_Config();
                printf("Main Thread do rsc_test_case_00_frame_2_RSC_Config\n");
                ret=rsc_test_case_00_frame_2_RSC_Config();
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
            }

            g_bRscThreadTerminated = 0;
            
            ::sem_wait(&RscSem);

            break;
#endif
        case 2: //Two rsc requests at the same time.
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            g_bMultipleBuffer = 0;
            ret = multi_enque_rsc_tc00_frames_RSC_Config();

            //g_frame2_bOneRequestHaveManyBuffer = 0;
            //ret = multi_enque_rsc_test_case_00_frame_2_RSC_Config();
            break;
#if 0
        case 3: //Main and Sub thread do Two dve request and Two wmfe request at the same time.
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            g_bMultipleBuffer = 0;
            g_frame2_bOneRequestHaveManyBuffer = 0;

            ::sem_init(&RscSem,0, 0);
            pthread_create(&RscUserThread, NULL, RscThreadLoop_OneReqHaveOneBuffer, (void*)NULL);
            g_bRscThreadTerminated = 1;
            ::sem_post(&RscSem);
            for (i=0;i<10;i++)
            {
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                ret = multi_enque_rsc_tc00_frames_RSC_Config();
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
            }
            g_bRscThreadTerminated = 0;
            ::sem_wait(&RscSem);

            break;
#endif
        case 4: //One rsc request with two buffers
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter

            g_bMultipleBuffer = 1;
            ret = multi_enque_rsc_tc00_frames_RSC_Config();

            //g_frame2_bOneRequestHaveManyBuffer = 1;
            //ret = multi_enque_rsc_test_case_00_frame_2_RSC_Config();
            break;
#if 0
        case 5: //Main and Sub thread do 1 rsc request with 2 buffers respectively.
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            g_bMultipleBuffer = 1;
            g_frame2_bOneRequestHaveManyBuffer = 1;

            ::sem_init(&RscSem,0, 0);
            pthread_create(&RscUserThread, NULL, RscThreadLoop_OneReqHaveTwoBuffer, (void*)NULL);
            g_bRscThreadTerminated = 1;
            ::sem_post(&RscSem);
            for (i=0;i<10;i++)
            {
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                ret = multi_enque_rsc_tc00_frames_RSC_Config();
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
                printf("===================Main  Thread===================\n");
            }
            g_bRscThreadTerminated = 0;
            ::sem_wait(&RscSem);
            break;
        case 6: 
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            g_bTestBlockingDeque = 1;
            ret=rsc_test_case_00_frame_0_RSC_Config();
            break;
        case 7: 
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            printf("===================Main  Thread===================\n");
            g_bTestBlockingDeque = 0;
            pid_t child_pid;
            child_pid = fork ();

            if (child_pid < 0)
            {
                printf("error in fork!");
            }
            else if (child_pid == 0) {  
                printf("i am the child process, my process id is %d/n",getpid());  
                do{
                    ret=rsc_test_case_00_frame_1_RSC_Config();
                }while(1);
            }  
            else {  
                printf("i am the parent process, my process id is %d/n",getpid());   
                do{

                    ret=rsc_test_case_00_frame_0_RSC_Config();
                }while(1);
            }  
            break;
#endif
        default:
            break;
    }
    
    ret = 1;
    return ret; 
}

MVOID RSCCallback(RSCParams& rParams)
{
    printf("--- [RSC callback func]\n");
    g_bRSCCallback = MTRUE;
}


//#include "../../imageio/test/DIP_pics/P2A_FG/imgi_640_480_10.h"
/*********************************************************************************/
int rsc_default()
{
    int ret=0;
    printf("--- [rsc_default]\n");
    /*
    *  1. RscStream Instance
    */ 
    NSCam::NSIoPipe::NSRsc::IRscStream* pStream;
    pStream= NSCam::NSIoPipe::NSRsc::IRscStream::createInstance("test_rsc_default");
    pStream->init();   
    printf("--- [test_rsc_default...RscStream init done\n");

    /*
    *  2. IMemDrv Instance
    */
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMAI memory asignment
    */

    /*static Golden Data*/
#ifdef EIS_OFFLINE
    char * golden_mvo_frame = eis_frame_0_rsc_mvo_frame_00_00;
    char * golden_bvo_frame = eis_frame_0_rsc_bvo_frame_00_00;

    MUINT32 mvo_size_bytes  = frame_0_rsc_mvo_frame_00_00_size;
    MUINT32 bvo_size_bytes  = frame_0_rsc_bvo_frame_00_00_size;

#elif DEFAULT
    char * golden_mvo_frame = frame_0_rsc_mvo_frame_00_00;
    char * golden_bvo_frame = frame_0_rsc_bvo_frame_00_00;

    MUINT32 mvo_size_bytes  = frame_0_rsc_mvo_frame_00_00_size;
    MUINT32 bvo_size_bytes  = frame_0_rsc_bvo_frame_00_00_size;
#else
    char * golden_mvo_frame = tc00_frame_1_rsc_mvo_frame_01_14;
    char * golden_bvo_frame = tc00_frame_1_rsc_bvo_frame_01_14;

    MUINT32 mvo_size_bytes  = tc00_frame_1_rsc_mvo_frame_01_14_size;
    MUINT32 bvo_size_bytes  = tc00_frame_1_rsc_bvo_frame_01_14_size;
#endif
    /*static DMAI memories*/
    char * rsc_apli_c_frame;
    char * rsc_apli_p_frame;
    char * rsc_mvi_frame;
    char * rsc_imgi_c_frame;
    char * rsc_imgi_p_frame;

#ifdef EIS_OFFLINE
    rsc_apli_c_frame = eis_frame_0_rsc_apli_c_frame_00;
    rsc_apli_p_frame = eis_frame_0_rsc_apli_p_frame_00;
    rsc_mvi_frame    = eis_frame_0_rsc_mvi_frame_00;
    rsc_imgi_c_frame = eis_frame_0_rsc_imgi_c_frame_00;
    rsc_imgi_p_frame = eis_frame_0_rsc_imgi_p_frame_00;

    MUINT32 rsc_apli_c_frame_size = eis_frame_0_rsc_apli_c_frame_00_size;
    MUINT32 rsc_apli_p_frame_size = eis_frame_0_rsc_apli_p_frame_00_size;
    MUINT32 rsc_mvi_frame_size    = eis_frame_0_rsc_mvi_frame_00_size;
    MUINT32 rsc_imgi_c_frame_size = eis_frame_0_rsc_imgi_c_frame_00_size;
    MUINT32 rsc_imgi_p_frame_size = eis_frame_0_rsc_imgi_p_frame_00_size;

#elif DEFAULT
    rsc_apli_c_frame = frame_0_rsc_apli_c_frame_00;
    rsc_apli_p_frame = frame_0_rsc_apli_p_frame_00;
    rsc_mvi_frame    = frame_0_rsc_mvi_frame_00;
    rsc_imgi_c_frame = frame_0_rsc_imgi_c_frame_00;
    rsc_imgi_p_frame = frame_0_rsc_imgi_p_frame_00;

    MUINT32 rsc_apli_c_frame_size = frame_0_rsc_apli_c_frame_00_size;
    MUINT32 rsc_apli_p_frame_size = frame_0_rsc_apli_p_frame_00_size;
    MUINT32 rsc_mvi_frame_size    = frame_0_rsc_mvi_frame_00_size;
    MUINT32 rsc_imgi_c_frame_size = frame_0_rsc_imgi_c_frame_00_size;
    MUINT32 rsc_imgi_p_frame_size = frame_0_rsc_imgi_p_frame_00_size;
#else
    rsc_apli_c_frame = tc00_frame_1_rsc_apli_c_frame_01;
    rsc_apli_p_frame = tc00_frame_1_rsc_apli_p_frame_01;
    rsc_mvi_frame    = tc00_frame_1_rsc_mvi_frame_01;
    rsc_imgi_c_frame = tc00_frame_1_rsc_imgi_c_frame_01;
    rsc_imgi_p_frame = tc00_frame_1_rsc_imgi_p_frame_01;

    MUINT32 rsc_apli_c_frame_size = tc00_frame_1_rsc_apli_c_frame_01_size;
    MUINT32 rsc_apli_p_frame_size = tc00_frame_1_rsc_apli_p_frame_01_size;
    MUINT32 rsc_mvi_frame_size    = tc00_frame_1_rsc_mvi_frame_01_size;
    MUINT32 rsc_imgi_c_frame_size = tc00_frame_1_rsc_imgi_c_frame_01_size;
    MUINT32 rsc_imgi_p_frame_size = tc00_frame_1_rsc_imgi_p_frame_01_size;

#endif
    /*static DMAI memory allocation and copy*/
    printf("--- rsc  input  allocate init\n");

    IMEM_BUF_INFO buf_rsc_apli_c_frame;
    buf_rsc_apli_c_frame.size = rsc_apli_c_frame_size;
    printf("buf_rsc_apli_c_frame.size:%d",buf_rsc_apli_c_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_apli_c_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_apli_c_frame);
    memcpy( (MUINT8*)(buf_rsc_apli_c_frame.virtAddr), (MUINT8*)(rsc_apli_c_frame), buf_rsc_apli_c_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_apli_c_frame);
    
    IMEM_BUF_INFO buf_rsc_apli_p_frame;
    buf_rsc_apli_p_frame.size = rsc_apli_p_frame_size;
    printf("buf_rsc_apli_p_frame.size:%d",buf_rsc_apli_p_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_apli_p_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_apli_p_frame);
    memcpy( (MUINT8*)(buf_rsc_apli_p_frame.virtAddr), (MUINT8*)(rsc_apli_p_frame), buf_rsc_apli_p_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_apli_p_frame);

    //IMEM_BUFF_PREPARE(mvi);
    IMEM_BUF_INFO buf_rsc_mvi_frame;
    buf_rsc_mvi_frame.size = rsc_mvi_frame_size;
    printf("buf_rsc_mvi_frame.size:%d",buf_rsc_mvi_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_mvi_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_mvi_frame);
    memcpy( (MUINT8*)(buf_rsc_mvi_frame.virtAddr), (MUINT8*)(rsc_mvi_frame), buf_rsc_mvi_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_mvi_frame);

    //IMEM_BUFF_PREPARE(imgi_p);
    IMEM_BUF_INFO buf_rsc_imgi_p_frame;
    buf_rsc_imgi_p_frame.size = rsc_imgi_p_frame_size;
    printf("buf_rsc_imgi_p_frame.size:%d",buf_rsc_imgi_p_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_imgi_p_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_imgi_p_frame);
    memcpy( (MUINT8*)(buf_rsc_imgi_p_frame.virtAddr), (MUINT8*)(rsc_imgi_p_frame), buf_rsc_imgi_p_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_imgi_p_frame);

    IMEM_BUF_INFO buf_rsc_imgi_c_frame;
    buf_rsc_imgi_c_frame.size = rsc_imgi_c_frame_size;
    printf("buf_rsc_imgi_c_frame.size:%d",buf_rsc_imgi_c_frame.size);
    mpImemDrv->allocVirtBuf(&buf_rsc_imgi_c_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_imgi_c_frame);
    memcpy( (MUINT8*)(buf_rsc_imgi_c_frame.virtAddr), (MUINT8*)(rsc_imgi_c_frame), buf_rsc_imgi_c_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_imgi_c_frame);

    printf("--- rsc DMI allocation done\n");

    /*
    *  4. DMAO memory allocation
    */
    IMEM_BUF_INFO buf_rsc_mvo_frame;    
    buf_rsc_mvo_frame.size = mvo_size_bytes;
    mpImemDrv->allocVirtBuf(&buf_rsc_mvo_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_mvo_frame);
    memset((MUINT8*)buf_rsc_mvo_frame.virtAddr, 0xffffffff, buf_rsc_mvo_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_mvo_frame);

    IMEM_BUF_INFO buf_rsc_bvo_frame;    
    buf_rsc_bvo_frame.size = bvo_size_bytes;
    mpImemDrv->allocVirtBuf(&buf_rsc_bvo_frame);
    mpImemDrv->mapPhyAddr(&buf_rsc_bvo_frame);
    memset((MUINT8*)buf_rsc_bvo_frame.virtAddr, 0xffffffff, buf_rsc_bvo_frame.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_rsc_bvo_frame);

   
    printf("#########################################################\n");
    printf("###########frame_0Start to Test !!!!###########\n");
    printf("#########################################################\n");

    /*
    *  5-1. RSC stream's request configuration: rRscParams
    */
    RSCParams rRscParams;
    RSCConfig rscconfig;
    rRscParams.mpfnCallback = RSCCallback;  

    rscconfig.Rsc_Apli_c.u4BufVA = buf_rsc_apli_c_frame.virtAddr;
    rscconfig.Rsc_Apli_p.u4BufVA = buf_rsc_apli_p_frame.virtAddr;
    rscconfig.Rsc_mvi.u4BufVA    = buf_rsc_mvi_frame.virtAddr;
    rscconfig.Rsc_Imgi_c.u4BufVA = buf_rsc_imgi_c_frame.virtAddr;
    rscconfig.Rsc_Imgi_p.u4BufVA = buf_rsc_imgi_p_frame.virtAddr;
    rscconfig.Rsc_mvo.u4BufVA    = buf_rsc_mvo_frame.virtAddr;
    rscconfig.Rsc_bvo.u4BufVA    = buf_rsc_bvo_frame.virtAddr;

    rscconfig.Rsc_Apli_c.u4BufPA = buf_rsc_apli_c_frame.phyAddr;
    rscconfig.Rsc_Apli_p.u4BufPA = buf_rsc_apli_p_frame.phyAddr;
    rscconfig.Rsc_mvi.u4BufPA    = buf_rsc_mvi_frame.phyAddr;
    rscconfig.Rsc_Imgi_c.u4BufPA = buf_rsc_imgi_c_frame.phyAddr;
    rscconfig.Rsc_Imgi_p.u4BufPA = buf_rsc_imgi_p_frame.phyAddr;
    rscconfig.Rsc_mvo.u4BufPA    = buf_rsc_mvo_frame.phyAddr;
    rscconfig.Rsc_bvo.u4BufPA    = buf_rsc_bvo_frame.phyAddr;


#ifdef EIS_OFFLINE
    rscconfig.Rsc_Imgi_c.u4Stride = 0x120;
    rscconfig.Rsc_Imgi_p.u4Stride = 0x120;
    rscconfig.Rsc_mvi.u4Stride    = 0x150;
    rscconfig.Rsc_mvo.u4Stride    = 0x150;
    rscconfig.Rsc_bvo.u4Stride    = 0x90;
    rscconfig.Rsc_Size_Height  = 0x00A2;
    rscconfig.Rsc_Size_Width   = 0x0120;

    rscconfig.Rsc_Ctrl_Skip_Pre_Mv = 0x0;
#if KY_CURR_BLK_CTRL
    rscconfig.Rsc_Curr_Blk_Out_Var_shift = 0x07;
    rscconfig.Rsc_Curr_Blk_Edge_Th = 0x01;
#endif


#elif DEFAULT
    rscconfig.Rsc_Imgi_c.u4Stride = 0x140;
    rscconfig.Rsc_Imgi_p.u4Stride = 0x150;
    rscconfig.Rsc_mvi.u4Stride    = 0x150;
    rscconfig.Rsc_mvo.u4Stride    = 0x200;
    rscconfig.Rsc_bvo.u4Stride    = 0x160;
    rscconfig.Rsc_Size_Height  = 0x01FF;
    rscconfig.Rsc_Size_Width   = 0x0120;

    rscconfig.Rsc_Horz_Sr   = 0x0008;
    rscconfig.Rsc_Vert_Sr   = 0x0020;

    rscconfig.Rsc_Ctrl_Init_MV_Waddr     = 0x14;
    rscconfig.Rsc_Ctrl_Init_MV_Flush_cnt = 0x3;
    rscconfig.Rsc_Ctrl_Trig_Num          = 0xF;
    rscconfig.Rsc_Ctrl_Imgi_c_Fmt        = RSC_IMGI_Y_FMT;
    rscconfig.Rsc_Ctrl_Imgi_p_Fmt        = RSC_IMGI_Y_FMT;
    rscconfig.Rsc_Ctrl_Gmv_mode          = 0x1;
    rscconfig.Rsc_Ctrl_First_Me          = 0x1;
    rscconfig.Rsc_Ctrl_Skip_Pre_Mv       = 0x1;
    rscconfig.Rsc_Ctrl_Start_Scan_Order  = 0x1;

    rscconfig.Rsc_Cand_Num  = 0x13;
    rscconfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Th  = 0x40;
    rscconfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Mode  = 0x0;
    rscconfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_En  = 0x1;
    rscconfig.Rsc_Sad_Ctrl_Sad_Gain  = 0x4;
    rscconfig.Rsc_Sad_Ctrl_Sad_Coring_Th  = 0x1;
    rscconfig.Rsc_Sad_Ctrl_Prev_Y_Offset  = 0x132;

#else
    rscconfig.Rsc_Imgi_c.u4Stride = 0x1c0;
    rscconfig.Rsc_Imgi_p.u4Stride = 0x150;
    rscconfig.Rsc_mvi.u4Stride    = 0x240;
    rscconfig.Rsc_mvo.u4Stride    = 0x1e0;
    rscconfig.Rsc_bvo.u4Stride    = 0x180;
    rscconfig.Rsc_Size_Height  = 0x01FF;
    rscconfig.Rsc_Size_Width   = 0x0120;

    rscconfig.Rsc_Horz_Sr   = 0x0008;
    rscconfig.Rsc_Vert_Sr   = 0x0020;

    rscconfig.Rsc_Ctrl_Init_MV_Waddr     = 0x14;
    rscconfig.Rsc_Ctrl_Init_MV_Flush_cnt = 0x3;
    rscconfig.Rsc_Ctrl_Trig_Num          = 0xF;
    rscconfig.Rsc_Ctrl_Imgi_c_Fmt        = RSC_IMGI_Y_FMT;
    rscconfig.Rsc_Ctrl_Imgi_p_Fmt        = RSC_IMGI_Y_FMT;
    rscconfig.Rsc_Ctrl_Gmv_mode          = 0x1;
    rscconfig.Rsc_Ctrl_First_Me          = 0x1;
    rscconfig.Rsc_Ctrl_Skip_Pre_Mv       = 0x0;
    rscconfig.Rsc_Ctrl_Start_Scan_Order  = 0x1;

    rscconfig.Rsc_Cand_Num  = 0x13;
    rscconfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Th  = 0x80;
    rscconfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_Mode  = 0x1;
    rscconfig.Rsc_Sad_Ctrl_Prev_Y_Offset_Coring_En  = 0x1;
    rscconfig.Rsc_Sad_Ctrl_Sad_Gain  = 0x4;
    rscconfig.Rsc_Sad_Ctrl_Sad_Coring_Th  = 0x1;
    rscconfig.Rsc_Sad_Ctrl_Prev_Y_Offset  = 0x32;
#endif
    printf("####apli.c.PA(%p), apli.p.PA(%p), mvi.PA(%p), imgi.c.PA(%p), imgi.p.PA(%p), mvo.PA(%p), bvo.PA(%p)\n", rscconfig.Rsc_Apli_c.u4BufPA,rscconfig.Rsc_Apli_p.u4BufPA, rscconfig.Rsc_mvi.u4BufPA, rscconfig.Rsc_Imgi_c.u4BufPA, rscconfig.Rsc_Imgi_p.u4BufPA, rscconfig.Rsc_mvo.u4BufPA, rscconfig.Rsc_bvo.u4BufPA);

#if 0
    rscconfig.Wmfe_Ctrl_0.Wmfe_Enable = true;
    rscconfig.Wmfe_Ctrl_0.WmfeFilterSize = WMFE_FILTER_SIZE_7x7;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Width = 0x30;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Height = 0x30;
    rscconfig.Wmfe_Ctrl_0.WmfeImgiFmt = RSC_IMGI_Y_FMT;
    rscconfig.Wmfe_Ctrl_0.WmfeDpiFmt= WMFE_DPI_D_FMT;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Imgi.u4BufVA = buf_rsc_imgi_c_frame.virtAddr;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Imgi.u4BufPA = buf_rsc_imgi_c_frame.phyAddr;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Imgi.u4Stride = 0x30;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Dpi.u4BufVA = buf_wmf_dpi_frame_0.virtAddr;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Dpi.u4BufPA = buf_wmf_dpi_frame_0.phyAddr;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Dpi.u4Stride = 0x30;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Tbli.u4BufVA = buf_wmf_tbli_frame_0.virtAddr;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Tbli.u4BufPA = buf_wmf_tbli_frame_0.phyAddr;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Tbli.u4Stride = 0x100;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Dpo.u4BufVA = buf_wmf_dpo_frame_0.virtAddr;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Dpo.u4BufPA = buf_wmf_dpo_frame_0.phyAddr;
    rscconfig.Wmfe_Ctrl_0.Wmfe_Dpo.u4Stride = 0x30;
#endif

    rRscParams.mRSCConfigVec.push_back(rscconfig);
    g_bRSCCallback = MFALSE;

#define MANUAL_MODE (0)
#if (MANUAL_MODE)
    printf("TRIGG_NUM[0x15029018] = 0x00143F0F\n");
    printf(" CAND_NUM[0x15029034] = 0x00000013\n");
    getchar();
#endif

    /*
    *   5-2. request enqued to rscstream
    */
    //enque
    ret=pStream->RSCenque(rRscParams);
    if(!ret)
    {
        printf("---ERRRRRRRRR [rsc_default..rsc enque fail\n]");
    }
    else
    {
        printf("---[rsc_default..rsc enque done\n]");
    }

    do{
        usleep(100000);
        if (MTRUE == g_bRSCCallback)
        {
            break;
        }
    }while(1);


    /*
    *   6. Bittrue Comparison
    */

    /*
     * BVO bittrue
     */

#define BVO_STRIDE_ADDR (0x150290C8)
#define RSCO_BVO_XSIZE_ADDR (0x15029840)
#define RSC_REG_RANGE (0x1000)
#define RSC_BASE_HW (0x15029000)
    	MUINTPTR rsc_bvo_frame;
	MUINT32 bvo_w;
	MUINT32 bvo_h;
	MUINT32 bvo_stride;
	MUINT32 *rsc_base_addr;
	MINT32 fd;
	MUINT32 err_cnt_dma, x_end, y_end;

	fd = open("/dev/camera-rsc", O_RDWR);
	if(fd < 0) {
		printf("rsc kernel fil open failed.\n");
		return ret;
	}
	
	rsc_base_addr = (MUINT32 *) mmap(0, RSC_REG_RANGE, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, RSC_BASE_HW);
	if(rsc_base_addr == MAP_FAILED) {
		printf("RSC mmap fail, errno(%d):%s", errno, strerror(errno));
		return ret;
	}

    	rsc_bvo_frame = (MUINTPTR) buf_rsc_bvo_frame.virtAddr;

	bvo_w = -1;
	bvo_stride = rscconfig.Rsc_bvo.u4Stride;
	bvo_h = bvo_size_bytes/bvo_stride;

	x_end = rsc_base_addr[((RSCO_BVO_XSIZE_ADDR - RSC_BASE_HW)>>2)];
	y_end = bvo_h;
	printf("BVO: stride(%d) xsize(%d) ysize(%d)", bvo_stride, x_end, y_end);

	err_cnt_dma = rsc_comp_roi_mem_with_file(golden_bvo_frame, 
							0, 
							(MUINTPTR)rsc_bvo_frame,
							bvo_w,
							bvo_h,
							bvo_stride,
							0,
							0,
							x_end,
							y_end,
							0,
							1,
							0
							);	
	if (err_cnt_dma)
	{
		//Error
		printf("bvo bit true fail: errcnt (%d)!!!\n", err_cnt_dma);
	}
	else
	{
		//Pass
		printf("bvo bit true pass!!!\n");
	}

	/*
 	*    MVO bittrue
	*/
	#define MVO_STRIDE_ADDR (0x150290C0)
	#define RSCI_SIZE (0x1502901C)

	MUINTPTR rsc_mvo_frame;
    	MUINT32 mvo_w;
	MUINT32 mvo_h;
	MUINT32 mvo_stride;
	MUINT32 w, w1,h ,h1;

	mvo_w = -1;
	mvo_stride = rsc_base_addr[((MVO_STRIDE_ADDR - RSC_BASE_HW)>>2)];
	mvo_h = mvo_size_bytes/mvo_stride;

	w  = (rsc_base_addr[((RSCI_SIZE - RSC_BASE_HW)>>2)] & 0x000001FF);
	w1 = ((w+1)>>1) - 1;
	x_end = ((w1 / 7) << 4) + (((((w1 % 7) + 1) * 18) + 7) >> 3) - 1;

	h  = ((rsc_base_addr[((RSCI_SIZE - RSC_BASE_HW)>>2)] & 0x01FF0000) >> 16);
	h1 = ((h+1)>>1);
	y_end = h1 - 1;
	printf("mvo_xsize(%d),%d,IMGI_xsize(%d)\n", x_end, w1, w);
	printf("mvo_ysize(%d),%d,IMGI_ysize(%d)\n", y_end, h1, h);

	rsc_mvo_frame = (MUINTPTR) buf_rsc_mvo_frame.virtAddr;

	err_cnt_dma = rsc_comp_roi_mem_with_file(golden_mvo_frame, 
						0, 
						(MUINTPTR)rsc_mvo_frame,
						mvo_w,
						h1,
						mvo_stride,
						0,
						0,
						x_end,
						y_end,
						0,
						1,
						0
						);	
	if (err_cnt_dma)
	{
		//Error
		printf("mvo  bit true fail: errcnt (%d)!!!\n", err_cnt_dma);
	}
	else
	{
		//Pass
		printf("mvo bit true pass!!!\n");
	}
#if (MANUAL_MODE)
	getchar();
#endif
    /*
    *  7. cleanup the mass
    */
    if(rsc_base_addr != MAP_FAILED)
        munmap(rsc_base_addr, RSC_REG_RANGE);

    if(fd >= 0)
        close(fd);

    mpImemDrv->freeVirtBuf(&buf_rsc_apli_c_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_apli_p_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_mvi_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_imgi_c_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_imgi_p_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_mvo_frame);
    mpImemDrv->freeVirtBuf(&buf_rsc_bvo_frame);
   
    pStream->uninit();   
    printf("--- [RscStream uninit done\n");

    mpImemDrv->uninit();
    printf("--- [Imem uninit done\n");

    return ret;
}



/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        printf(": failed to create file [%s]: %s \n", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            printf(": failed to write to file [%s]: %s\n", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}



