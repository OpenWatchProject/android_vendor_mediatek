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


#define LOG_TAG "gepfstream_test"

#include <vector>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/prctl.h>

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
#include <mtkcam/drv/iopipe/PostProc/IGepfStream.h>
#include <drv/gepfunittest.h>
#include <mtkcam/drv/iopipe/PostProc/GepfUtility.h>

//#include <mtkcam/iopipe/PostProc/IFeatureStream.h>

//#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <drv/imem_drv.h>
//#include <mtkcam/drv/isp_drv.h>

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
//
//#include "gepf/gepf_testcommon.h"

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSGepf;

/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size);
/******************************************************************************
* GEPF Test Case
*******************************************************************************/
//#include "frame_0_gepf_setting_00.h"

int gepf_default(int type);
int gepf_dvt_frame0(int type);
int gepf_dvt_frame1(int type);
bool multi_enque_gepf_tc00_frames_GEPF_Config();

MBOOL g_bGEPFCallback;

MVOID GEPFCallback(GEPFParams& rParams);
MVOID GEPF_multi_enque_gepf_tc00_request1_GEPFCallback(GEPFParams& rParams);
MVOID GEPF_multi_enque_gepf_tc00_request0_GEPFCallback(GEPFParams& rParams);

MBOOL g_b_multi_enque_gepf_tc00_request0_GEPFCallback;
MBOOL g_b_multi_enque_gepf_tc00_request1_GEPFCallback;

int g_GepfMultipleBuffer = 0;
int g_GepfCount = 0;

pthread_t       GepfUserThread;
sem_t           GepfSem;
volatile bool            g_bGepfThreadTerminated = 0;

/*******************************************************************************
*  Main Function 
*  
********************************************************************************/

int test_gepfStream(int argc, char** argv)
{
    int ret = 0; 
    int testCase = 0;
	int testType = 0;
    int i;
    if (argc>=1)
    {
        testCase = atoi(argv[0]);
		testType = atoi(argv[1]);
    }

    switch(testCase)
    {
        case 0:  //Unit Test Case One GEPF Request.
            g_GEPF_UnitTest_Num = GEPF_DEFAULT_UT; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            ret = gepf_default(testType);
            break;
        case 1:  // DVT case with 2 frame
            ret = gepf_dvt_frame0(testType);
            ret = gepf_dvt_frame1(testType);
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
        case 2: //Two gepf requests at the same time.
            g_GEPF_UnitTest_Num = GEPF_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            g_GepfMultipleBuffer = 0;
            ret = multi_enque_gepf_tc00_frames_GEPF_Config();

            //g_frame2_bOneRequestHaveManyBuffer = 0;
            //ret = multi_enque_rsc_test_case_00_frame_2_RSC_Config();
            break;
#if 0
        case 3: //Main and Sub thread do Two dve request and Two wmfe request at the same time.
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            g_GepfMultipleBuffer = 0;
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
        case 4: //One gepf request with two buffers
            g_GEPF_UnitTest_Num = GEPF_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter

            g_GepfMultipleBuffer = 1;
            ret = multi_enque_gepf_tc00_frames_GEPF_Config();

            //g_frame2_bOneRequestHaveManyBuffer = 1;
            //ret = multi_enque_rsc_test_case_00_frame_2_RSC_Config();
            break;
#if 0
        case 5: //Main and Sub thread do 1 rsc request with 2 buffers respectively.
            g_RSC_UnitTest_Num = RSC_TESTCASE_UT_0; // some parameter can't be controled by user, but the bit-ture test case use the parameter
            g_GepfMultipleBuffer = 1;
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

MVOID GEPFCallback(GEPFParams& rParams)
{
    printf("--- [GEPF callback func]\n");
    g_bGEPFCallback = MTRUE;
}

#include "gepf/mode_0/default/gepf_depth_rtl_0.h"
#include "gepf/mode_0/default/gepf_new_table_0.h"
#include "gepf/mode_0/default/gepf_temp_ans_debug_blur_wr_0.h"
#include "gepf/mode_0/default/gepf_temp_ans_debug_wr_0.h"
#include "gepf/mode_0/default/gepf_temp_cur_y_0.h"
#include "gepf/mode_0/default/gepf_temp_pre_src_0.h"
#include "gepf/mode_0/default/gepf_temp_pre_src_blur_0.h"
#include "gepf/mode_0/default/gepf_temp_pre_y_0.h"
#include "gepf/mode_0/default/gepf_uv420_rtl_0.h"
#include "gepf/mode_0/default/gepf_y_rtl_0.h"
#include "gepf/mode_1/default/gepf_depth_rtl_1.h"
#include "gepf/mode_1/default/gepf_new_table_1.h"
#include "gepf/mode_1/default/gepf_temp_ans_debug_blur_wr_1.h"
#include "gepf/mode_1/default/gepf_temp_ans_debug_wr_1.h"
#include "gepf/mode_1/default/gepf_temp_cur_y_1.h"
#include "gepf/mode_1/default/gepf_temp_pre_src_1.h"
#include "gepf/mode_1/default/gepf_temp_pre_src_blur_1.h"
#include "gepf/mode_1/default/gepf_temp_pre_y_1.h"
#include "gepf/mode_1/default/gepf_uv420_rtl_1.h"
#include "gepf/mode_1/default/gepf_y_rtl_1.h"

/*********************************************************************************/
int gepf_default(int type)
{
    int ret=0;
    printf("--- [gepf_default: Mode %d] ---\n", type);
    /*
    *  1. GepfStream Instance
    */ 
    NSCam::NSIoPipe::NSGepf::IGepfStream* pStream;
    pStream= NSCam::NSIoPipe::NSGepf::IGepfStream::createInstance("test_gepf_default");
    pStream->init();   
    printf("--- [test_gepf_default...GepfStream init done] ---\n");

    /*
    *  2. IMemDrv Instance
    */
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMA memory asignment
    */
	//MUINT32 Width = 240;
	//MUINT32 Align = 8;
	MUINT32 WidthYuv = 0xF0, HeightYuv = 0x87;
	MUINT32 WidthDep = 0xF0, HeightDep = 0x87;
	
    // GEPF +++++
    IMEM_BUF_INFO buf_gepf_depth_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_depth_rtl.size = sizeof(gepf_depth_rtl_0);
            break;
        case 1:
        default:
            buf_gepf_depth_rtl.size = sizeof(gepf_depth_rtl_1);
            break;
    }
	//buf_gepf_depth_rtl.size = gepf_MemAlign(buf_gepf_depth_rtl.size, Width, Align);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_depth_rtl.size *= 4;	// key
	//buf_gepf_depth_rtl.size *= 16;	
    printf("buf_gepf_depth_rtl.size:%d\n",buf_gepf_depth_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_depth_rtl.virtAddr), (MUINT8*)(gepf_depth_rtl_0), sizeof(gepf_depth_rtl_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_depth_rtl.virtAddr), (MUINT8*)(gepf_depth_rtl_1), sizeof(gepf_depth_rtl_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_rtl);

	IMEM_BUF_INFO buf_gepf_new_table;
    switch (type)
    {
        case 0:
            buf_gepf_new_table.size = sizeof(gepf_new_table_0);
            break;
        case 1:
        default:
            buf_gepf_new_table.size = sizeof(gepf_new_table_1);
            break;
    }
	//buf_gepf_new_table.size = gepf_MemAlign(buf_gepf_new_table.size, Width, Align);
	//buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size);
	buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size, 0, 0, type);
    printf("buf_gepf_new_table.size:%d\n",buf_gepf_new_table.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_new_table);
    mpImemDrv->mapPhyAddr(&buf_gepf_new_table);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_new_table.virtAddr), (MUINT8*)(gepf_new_table_0), sizeof(gepf_new_table_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_new_table.virtAddr), (MUINT8*)(gepf_new_table_1), sizeof(gepf_new_table_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_new_table);

	IMEM_BUF_INFO buf_gepf_temp_cur_y;
    switch (type)
    {
        case 0:
            buf_gepf_temp_cur_y.size = sizeof(gepf_temp_cur_y_0);
            break;
        case 1:
        default:
            buf_gepf_temp_cur_y.size = sizeof(gepf_temp_cur_y_1);
            break;
    }
	//buf_gepf_temp_cur_y.size = gepf_MemAlign(buf_gepf_temp_cur_y.size, Width, Align);
	buf_gepf_temp_cur_y.size = GEPFQueryDMASize(DMA_GEPF_TEMP_Y, buf_gepf_temp_cur_y.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_temp_cur_y.size *= 4;
	//buf_gepf_temp_cur_y.size *= 16;	
    printf("buf_gepf_temp_cur_y.size:%d\n",buf_gepf_temp_cur_y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_cur_y);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_cur_y);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_cur_y.virtAddr), (MUINT8*)(gepf_temp_cur_y_0), sizeof(gepf_temp_cur_y_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_cur_y.virtAddr), (MUINT8*)(gepf_temp_cur_y_1), sizeof(gepf_temp_cur_y_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_cur_y);

	IMEM_BUF_INFO buf_gepf_temp_pre_y;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_y.size = sizeof(gepf_temp_pre_y_0);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_y.size = sizeof(gepf_temp_pre_y_1);
            break;
    }
	//buf_gepf_temp_pre_y.size = gepf_MemAlign(buf_gepf_temp_pre_y.size, Width, Align);
	buf_gepf_temp_pre_y.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_Y, buf_gepf_temp_pre_y.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_y.size *= 4;
	//buf_gepf_temp_pre_y.size *= 16;
    printf("buf_gepf_temp_pre_y.size:%d\n",buf_gepf_temp_pre_y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_y);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_y);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_y.virtAddr), (MUINT8*)(gepf_temp_pre_y_0), sizeof(gepf_temp_pre_y_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_y.virtAddr), (MUINT8*)(gepf_temp_pre_y_1), sizeof(gepf_temp_pre_y_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_y);

	IMEM_BUF_INFO buf_gepf_temp_pre_src;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_src.size = sizeof(gepf_temp_pre_src_0);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_src.size = sizeof(gepf_temp_pre_src_1);
            break;
    }
	//buf_gepf_temp_pre_src.size = gepf_MemAlign(buf_gepf_temp_pre_src.size, Width, Align);
	buf_gepf_temp_pre_src.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_DEPTH, buf_gepf_temp_pre_src.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_src.size *= 4;
	//buf_gepf_temp_pre_src.size *= 16;
    printf("buf_gepf_temp_pre_src.size:%d\n",buf_gepf_temp_pre_src.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src.virtAddr), (MUINT8*)(gepf_temp_pre_src_0), sizeof(gepf_temp_pre_src_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src.virtAddr), (MUINT8*)(gepf_temp_pre_src_1), sizeof(gepf_temp_pre_src_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src);

	IMEM_BUF_INFO buf_gepf_temp_pre_src_blur;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_src_blur.size = sizeof(gepf_temp_pre_src_blur_0);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_src_blur.size = sizeof(gepf_temp_pre_src_blur_1);
            break;
    }
	//buf_gepf_temp_pre_src_blur.size = gepf_MemAlign(buf_gepf_temp_pre_src_blur.size, Width, Align);
	buf_gepf_temp_pre_src_blur.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_BLUR, buf_gepf_temp_pre_src_blur.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_src_blur.size *= 4;
	//buf_gepf_temp_pre_src_blur.size *= 16;
    printf("buf_gepf_temp_pre_src_blur.size:%d\n",buf_gepf_temp_pre_src_blur.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src_blur);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src_blur);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src_blur.virtAddr), (MUINT8*)(gepf_temp_pre_src_blur_0), sizeof(gepf_temp_pre_src_blur_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src_blur.virtAddr), (MUINT8*)(gepf_temp_pre_src_blur_1), sizeof(gepf_temp_pre_src_blur_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src_blur);

	IMEM_BUF_INFO buf_gepf_uv420_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_uv420_rtl.size = sizeof(gepf_uv420_rtl_0);
            break;
        case 1:
        default:
            buf_gepf_uv420_rtl.size = sizeof(gepf_uv420_rtl_1);
            break;
    }
	//buf_gepf_uv420_rtl.size = gepf_MemAlign(buf_gepf_uv420_rtl.size, Width, Align);
	buf_gepf_uv420_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV, buf_gepf_uv420_rtl.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_uv420_rtl.size *= 4;
	//buf_gepf_uv420_rtl.size *= 16;
    printf("buf_gepf_uv420_rtl.size:%d\n",buf_gepf_uv420_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_uv420_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_uv420_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_uv420_rtl.virtAddr), (MUINT8*)(gepf_uv420_rtl_0), sizeof(gepf_uv420_rtl_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_uv420_rtl.virtAddr), (MUINT8*)(gepf_uv420_rtl_1), sizeof(gepf_uv420_rtl_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_uv420_rtl);

	IMEM_BUF_INFO buf_gepf_y_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_y_rtl.size = sizeof(gepf_y_rtl_0);
            break;
        case 1:
        default:
            buf_gepf_y_rtl.size = sizeof(gepf_y_rtl_1);
            break;
    }
	//buf_gepf_y_rtl.size = gepf_MemAlign(buf_gepf_y_rtl.size, Width, Align);
	buf_gepf_y_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y, buf_gepf_y_rtl.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_y_rtl.size *= 4;
	//buf_gepf_y_rtl.size *= 16;
    printf("buf_gepf_y_rtl.size:%d\n",buf_gepf_y_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_y_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_y_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_y_rtl.virtAddr), (MUINT8*)(gepf_y_rtl_0), sizeof(gepf_y_rtl_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_y_rtl.virtAddr), (MUINT8*)(gepf_y_rtl_1), sizeof(gepf_y_rtl_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_y_rtl);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_blur_wr;
    switch (type)
    {
        case 0:
            buf_gepf_temp_ans_debug_blur_wr.size = sizeof(gepf_temp_ans_debug_blur_wr_0);
            break;
        case 1:
        default:
            buf_gepf_temp_ans_debug_blur_wr.size = sizeof(gepf_temp_ans_debug_blur_wr_1);
            break;
    }
	//buf_gepf_temp_ans_debug_blur_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_blur_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_blur_wr.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_gepf_temp_ans_debug_blur_wr.size, WidthDep, HeightDep, type);
	//buf_gepf_temp_ans_debug_blur_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_blur_wr.size:%d\n",buf_gepf_temp_ans_debug_blur_wr.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_blur_wr);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_blur_wr);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_blur_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_blur_wr_0), sizeof(gepf_temp_ans_debug_blur_wr_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_blur_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_blur_wr_1), sizeof(gepf_temp_ans_debug_blur_wr_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_blur_wr);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_wr;
    switch (type)
    {
        case 0:
            buf_gepf_temp_ans_debug_wr.size = sizeof(gepf_temp_ans_debug_wr_0);
            break;
        case 1:
        default:
            buf_gepf_temp_ans_debug_wr.size = sizeof(gepf_temp_ans_debug_wr_1);
            break;
    }
	//buf_gepf_temp_ans_debug_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_wr.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_gepf_temp_ans_debug_wr.size, WidthDep, HeightDep, type);
	//buf_gepf_temp_ans_debug_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_wr.size:%d\n",buf_gepf_temp_ans_debug_wr.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_wr);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_wr);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_wr_0), sizeof(gepf_temp_ans_debug_wr_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_wr_1), sizeof(gepf_temp_ans_debug_wr_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_wr);

	///
	IMEM_BUF_INFO buf_gepf_blur_base;
    //buf_gepf_blur_base.size = buf_gepf_depth_rtl.size;	// key
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	//buf_gepf_blur_base.size *= 16;
    printf("buf_gepf_blur_base.size:%d\n",buf_gepf_blur_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_base);
    memset( (MUINT8*)(buf_gepf_blur_base.virtAddr), 0x00000000, buf_gepf_blur_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_base);

	IMEM_BUF_INFO buf_gepf_yuv_base;
    //buf_gepf_yuv_base.size = buf_gepf_depth_rtl.size * 2;	// key
	buf_gepf_yuv_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_YUV, buf_gepf_yuv_base.size, WidthYuv, HeightYuv, type);
	//buf_gepf_yuv_base.size *= 16;
    printf("buf_gepf_yuv_base.size:%d\n",buf_gepf_yuv_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_yuv_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_yuv_base);
    memset( (MUINT8*)(buf_gepf_yuv_base.virtAddr), 0x00000000, buf_gepf_yuv_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_yuv_base);

	IMEM_BUF_INFO buf_temp_depth_base;
    //buf_temp_depth_base.size = buf_gepf_depth_rtl.size;
	buf_temp_depth_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH, buf_temp_depth_base.size, WidthDep, HeightDep, type);
	//buf_temp_depth_base.size *= 16;
    printf("buf_temp_depth_base.size:%d\n",buf_temp_depth_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_base);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_base);
    memset( (MUINT8*)(buf_temp_depth_base.virtAddr), 0x00000000, buf_temp_depth_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_base);

	IMEM_BUF_INFO buf_temp_blur_base;
    //buf_temp_blur_base.size = buf_gepf_depth_rtl.size;
	buf_temp_blur_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR, buf_temp_blur_base.size, WidthDep, HeightDep, type);
	//buf_temp_blur_base.size *= 16;
    printf("buf_temp_blur_base.size:%d\n",buf_temp_blur_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_base);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_base);
    memset( (MUINT8*)(buf_temp_blur_base.virtAddr), 0x00000000, buf_temp_blur_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_base);

	IMEM_BUF_INFO buf_temp_blur_wr_base;
    //buf_temp_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_temp_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_temp_blur_wr_base.size, WidthDep, HeightDep, type);
	//buf_temp_blur_wr_base.size *= 16;	
    printf("buf_temp_blur_wr_base.size:%d\n",buf_temp_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_wr_base);
    memset( (MUINT8*)(buf_temp_blur_wr_base.virtAddr), 0x00000000, buf_temp_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_wr_base);

	IMEM_BUF_INFO buf_temp_depth_wr_base;
    //buf_temp_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_temp_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_temp_depth_wr_base.size, WidthDep, HeightDep, type);
	//buf_temp_depth_wr_base.size *= 16;
    printf("buf_temp_depth_wr_base.size:%d\n",buf_temp_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_wr_base);
    memset( (MUINT8*)(buf_temp_depth_wr_base.virtAddr), 0x00000000, buf_temp_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_wr_base);

	IMEM_BUF_INFO buf_gepf_depth_wr_base;
    //buf_gepf_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH_WR, buf_gepf_depth_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_depth_wr_base.size *= 16;
    printf("buf_gepf_depth_wr_base.size:%d\n",buf_gepf_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_wr_base);
    memset( (MUINT8*)(buf_gepf_depth_wr_base.virtAddr), 0x00000000, buf_gepf_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_wr_base);

	IMEM_BUF_INFO buf_gepf_blur_wr_base;
    //buf_gepf_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR_WR, buf_gepf_blur_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_blur_wr_base.size *= 16;
    printf("buf_gepf_blur_wr_base.size:%d\n",buf_gepf_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_wr_base);
    memset( (MUINT8*)(buf_gepf_blur_wr_base.virtAddr), 0x00000000, buf_gepf_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_wr_base);

	IMEM_BUF_INFO buf_gepf_480_Y;
    //buf_gepf_480_Y.size = buf_gepf_depth_rtl.size;
	buf_gepf_480_Y.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y_480, buf_gepf_480_Y.size, WidthYuv, HeightYuv, type);
	//buf_gepf_480_Y.size *= 16;
    printf("buf_gepf_480_Y.size:%d\n",buf_gepf_480_Y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_Y);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_Y);
            memset( (MUINT8*)(buf_gepf_480_Y.virtAddr), 0x00000000, buf_gepf_480_Y.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_Y);

	IMEM_BUF_INFO buf_gepf_480_UV;
    //buf_gepf_480_UV.size = buf_gepf_depth_rtl.size;
	buf_gepf_480_UV.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV_480, buf_gepf_480_UV.size, WidthYuv, HeightYuv, type);
	//buf_gepf_480_UV.size *= 16;
    printf("buf_gepf_480_UV.size:%d\n",buf_gepf_480_UV.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_UV);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_UV);
            memset( (MUINT8*)(buf_gepf_480_UV.virtAddr), 0x00000000, buf_gepf_480_UV.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_UV);
    // GEPF -----
 
    printf("#########################################################\n");
    printf("###########GEPF Mode%d Start to Test !!!!###########\n", type);
    printf("#########################################################\n");

    /*
    *  5-1. GEPF stream's request configuration: rGepfParams
    */
    GEPFParams rGepfParams;
    GEPFConfig gepfconfig;
    rGepfParams.mpfnCallback = GEPFCallback;  

	switch (type)
	{
		case 0:
			gepfconfig.Gepf_Ctr_Iteration = 1;
			gepfconfig.Gepf_Ctr_Iteration_480 = 1;
			gepfconfig.Gepf_Ctr_Gepf_Mode = 0;
			gepfconfig.Temp_Depth.u4Stride = 0xF0;
			break;
		case 1:
		default:
			gepfconfig.Gepf_Ctr_Iteration = 3;
			gepfconfig.Gepf_Ctr_Iteration_480 = 3;
			gepfconfig.Gepf_Ctr_Gepf_Mode = 1;
			gepfconfig.Temp_Depth.u4Stride = 0x1E0;
			break;
	}

    //gepfconfig.Gepf_Ctr_Iteration;
    gepfconfig.Gepf_Ctr_Max_Filter_Val_Weight = 0x1F4;
    //gepfconfig.Gepf_Ctr_Iteration_480;
    //gepfconfig.Gepf_Ctr_Gepf_Mode;
    gepfconfig.Gepf_Ctr_Focus_Value = 0x6F;
    gepfconfig.Gepf_Ctr_Lamda = 0x3200;
    gepfconfig.Gepf_Ctr_Dof_M = 0x10;
    gepfconfig.Gepf_Ctr_Lamda_480 = 0x3200;
    gepfconfig.Temp_Ctr_Coeff_Value = 0;
    gepfconfig.Temp_Ctr_Bypass_En = 0;

    gepfconfig.Gepf_Focus.dmaport = DMA_GEPF_GEPF_FOCUS;
	gepfconfig.Gepf_Focus.u4BufVA = buf_gepf_new_table.virtAddr;
	gepfconfig.Gepf_Focus.u4BufPA = buf_gepf_new_table.phyAddr;
	gepfconfig.Gepf_Focus.u4BufSize = buf_gepf_new_table.size;
    gepfconfig.Gepf_YUV.dmaport = DMA_GEPF_GEPF_YUV;
	gepfconfig.Gepf_YUV.u4BufVA = buf_gepf_yuv_base.virtAddr;
	gepfconfig.Gepf_YUV.u4BufPA = buf_gepf_yuv_base.phyAddr;
    gepfconfig.Gepf_YUV.u4BufSize = buf_gepf_yuv_base.size;
	gepfconfig.Gepf_YUV.u4ImgWidth = WidthYuv;//0xF0;
	gepfconfig.Gepf_YUV.u4ImgHeight = HeightYuv;//0x87;
    gepfconfig.Gepf_Y.dmaport = DMA_GEPF_GEPF_Y;
	gepfconfig.Gepf_Y.u4BufVA = buf_gepf_y_rtl.virtAddr;
	gepfconfig.Gepf_Y.u4BufPA = buf_gepf_y_rtl.phyAddr;
	gepfconfig.Gepf_Y.u4BufSize = buf_gepf_y_rtl.size;
	gepfconfig.Gepf_Y.u4Stride = 0xF0;
    gepfconfig.Gepf_UV.dmaport = DMA_GEPF_GEPF_UV;
	gepfconfig.Gepf_UV.u4BufVA = buf_gepf_uv420_rtl.virtAddr;
	gepfconfig.Gepf_UV.u4BufPA = buf_gepf_uv420_rtl.phyAddr;
	gepfconfig.Gepf_UV.u4BufSize = buf_gepf_uv420_rtl.size;
	gepfconfig.Gepf_UV.u4Stride = 0xF0;
    gepfconfig.Gepf_Y_480.dmaport = DMA_GEPF_GEPF_Y_480;
	gepfconfig.Gepf_Y_480.u4BufVA = buf_gepf_480_Y.virtAddr;
	gepfconfig.Gepf_Y_480.u4BufPA = buf_gepf_480_Y.phyAddr;
	gepfconfig.Gepf_Y_480.u4BufSize = buf_gepf_480_Y.size;
	gepfconfig.Gepf_Y_480.u4Stride = 0x1E0;
    gepfconfig.Gepf_UV_480.dmaport = DMA_GEPF_GEPF_UV_480;
	gepfconfig.Gepf_UV_480.u4BufVA = buf_gepf_480_UV.virtAddr;
	gepfconfig.Gepf_UV_480.u4BufPA = buf_gepf_480_UV.phyAddr;
	gepfconfig.Gepf_UV_480.u4BufSize = buf_gepf_480_UV.size;
	gepfconfig.Gepf_UV_480.u4Stride = 0x1E0;
    gepfconfig.Gepf_Depth.dmaport = DMA_GEPF_GEPF_DEPTH;
	gepfconfig.Gepf_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;
	gepfconfig.Gepf_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;
	gepfconfig.Gepf_Depth.u4BufSize = buf_gepf_depth_rtl.size;
	gepfconfig.Gepf_Depth.u4Stride = 0xF0;
	gepfconfig.Gepf_Depth.u4ImgWidth = WidthDep;//0xF0;
	gepfconfig.Gepf_Depth.u4ImgHeight = HeightDep;//0x87;
    gepfconfig.Gepf_Depth_WR.dmaport = DMA_GEPF_GEPF_DEPTH_WR;
	gepfconfig.Gepf_Depth_WR.u4BufVA = buf_gepf_depth_wr_base.virtAddr;
	gepfconfig.Gepf_Depth_WR.u4BufPA = buf_gepf_depth_wr_base.phyAddr;
	gepfconfig.Gepf_Depth_WR.u4BufSize = buf_gepf_depth_wr_base.size;
    gepfconfig.Gepf_Blur.dmaport = DMA_GEPF_GEPF_BLUR;
	gepfconfig.Gepf_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;
	gepfconfig.Gepf_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;
	gepfconfig.Gepf_Blur.u4BufSize = buf_gepf_blur_base.size;
    gepfconfig.Gepf_Blur_WR.dmaport = DMA_GEPF_GEPF_BLUR_WR;
	gepfconfig.Gepf_Blur_WR.u4BufVA = buf_gepf_blur_wr_base.virtAddr;
	gepfconfig.Gepf_Blur_WR.u4BufPA = buf_gepf_blur_wr_base.phyAddr;
	gepfconfig.Gepf_Blur_WR.u4BufSize = buf_gepf_blur_wr_base.size;
	    	gepfconfig.Temp_Y.dmaport = DMA_GEPF_TEMP_Y;
	gepfconfig.Temp_Y.u4BufVA = buf_gepf_temp_cur_y.virtAddr;
	gepfconfig.Temp_Y.u4BufPA = buf_gepf_temp_cur_y.phyAddr;
	gepfconfig.Temp_Y.u4BufSize = buf_gepf_temp_cur_y.size;
	gepfconfig.Temp_Pre_Y.dmaport = DMA_GEPF_TEMP_PRE_Y;
	gepfconfig.Temp_Pre_Y.u4BufVA = buf_gepf_temp_pre_y.virtAddr;
	gepfconfig.Temp_Pre_Y.u4BufPA = buf_gepf_temp_pre_y.phyAddr;
	gepfconfig.Temp_Pre_Y.u4BufSize = buf_gepf_temp_pre_y.size;
	gepfconfig.Temp_Depth.dmaport = DMA_GEPF_TEMP_DEPTH;
	gepfconfig.Temp_Depth.u4BufVA = buf_temp_depth_base.virtAddr;
	gepfconfig.Temp_Depth.u4BufPA = buf_temp_depth_base.phyAddr;
	gepfconfig.Temp_Depth.u4BufSize = buf_temp_depth_base.size;
    gepfconfig.Temp_Pre_Depth.dmaport = DMA_GEPF_TEMP_PRE_DEPTH;
	gepfconfig.Temp_Pre_Depth.u4BufVA = buf_gepf_temp_pre_src.virtAddr;
	gepfconfig.Temp_Pre_Depth.u4BufPA = buf_gepf_temp_pre_src.phyAddr;
	gepfconfig.Temp_Pre_Depth.u4BufSize = buf_gepf_temp_pre_src.size;
	gepfconfig.Temp_Depth_WR.dmaport = DMA_GEPF_TEMP_DEPTH_WR;
	gepfconfig.Temp_Depth_WR.u4BufVA = buf_temp_depth_wr_base.virtAddr;
	gepfconfig.Temp_Depth_WR.u4BufPA = buf_temp_depth_wr_base.phyAddr;
	gepfconfig.Temp_Depth_WR.u4BufSize = buf_temp_depth_wr_base.size;
    gepfconfig.Temp_Blur.dmaport = DMA_GEPF_TEMP_BLUR;
	gepfconfig.Temp_Blur.u4BufVA = buf_temp_blur_base.virtAddr;
	gepfconfig.Temp_Blur.u4BufPA = buf_temp_blur_base.phyAddr;
	gepfconfig.Temp_Blur.u4BufSize = buf_temp_blur_base.size;
    gepfconfig.Temp_Pre_Blur.dmaport = DMA_GEPF_TEMP_PRE_BLUR;
	gepfconfig.Temp_Pre_Blur.u4BufVA = buf_gepf_temp_pre_src_blur.virtAddr;
	gepfconfig.Temp_Pre_Blur.u4BufPA = buf_gepf_temp_pre_src_blur.phyAddr;
	gepfconfig.Temp_Pre_Blur.u4BufSize = buf_gepf_temp_pre_src_blur.size;
    gepfconfig.Temp_Blur_WR.dmaport = DMA_GEPF_TEMP_BLUR_WR;
	gepfconfig.Temp_Blur_WR.u4BufVA = buf_temp_blur_wr_base.virtAddr;
	gepfconfig.Temp_Blur_WR.u4BufPA = buf_temp_blur_wr_base.phyAddr;
	gepfconfig.Temp_Blur_WR.u4BufSize = buf_temp_blur_wr_base.size;
    gepfconfig.Bypass_Depth.dmaport = DMA_GEPF_BYPASS_DEPTH;
	gepfconfig.Bypass_Depth.u4BufVA = buf_gepf_depth_wr_base.virtAddr;//buf_gepf_depth_rtl.virtAddr;
	gepfconfig.Bypass_Depth.u4BufPA = buf_gepf_depth_wr_base.phyAddr;//buf_gepf_depth_rtl.phyAddr;
	gepfconfig.Bypass_Depth.u4BufSize = buf_gepf_depth_wr_base.size;//buf_gepf_depth_rtl.size;
    gepfconfig.Bypass_Depth_WR.dmaport = DMA_GEPF_BYPASS_DEPTH_WR;
	gepfconfig.Bypass_Depth_WR.u4BufVA = buf_gepf_depth_wr_base.virtAddr;
	gepfconfig.Bypass_Depth_WR.u4BufPA = buf_gepf_depth_wr_base.phyAddr;
	gepfconfig.Bypass_Depth_WR.u4BufSize = buf_gepf_depth_wr_base.size;
    gepfconfig.Bypass_Blur.dmaport = DMA_GEPF_BYPASS_BLUR;
	gepfconfig.Bypass_Blur.u4BufVA = buf_gepf_blur_wr_base.virtAddr;//buf_gepf_blur_base.virtAddr;
	gepfconfig.Bypass_Blur.u4BufPA = buf_gepf_blur_wr_base.phyAddr;//buf_gepf_blur_base.phyAddr;
	gepfconfig.Bypass_Blur.u4BufSize = buf_gepf_blur_wr_base.size;//buf_gepf_blur_base.size;
    gepfconfig.Bypass_Blur_WR.dmaport = DMA_GEPF_BYPASS_BLUR_WR;
	gepfconfig.Bypass_Blur_WR.u4BufVA = buf_gepf_blur_wr_base.virtAddr;
	gepfconfig.Bypass_Blur_WR.u4BufPA = buf_gepf_blur_wr_base.phyAddr;
	gepfconfig.Bypass_Blur_WR.u4BufSize = buf_gepf_blur_wr_base.size;
	
    //printf("####apli.c.PA(%p), apli.p.PA(%p), mvi.PA(%p), imgi.c.PA(%p), imgi.p.PA(%p), mvo.PA(%p), bvo.PA(%p)\n", rscconfig.Rsc_Apli_c.u4BufPA,rscconfig.Rsc_Apli_p.u4BufPA, rscconfig.Rsc_mvi.u4BufPA, rscconfig.Rsc_Imgi_c.u4BufPA, rscconfig.Rsc_Imgi_p.u4BufPA, rscconfig.Rsc_mvo.u4BufPA, rscconfig.Rsc_bvo.u4BufPA);

    rGepfParams.mGEPFConfigVec.push_back(gepfconfig);
    g_bGEPFCallback = MFALSE;


    /*
    *   5-2. request enqued to gepfstream
    */
    //enque
    ret=pStream->GEPFenque(rGepfParams);
    if(!ret)
    {
        printf("--- ERRRRRRRRR [gepf_default..gepf enque fail] ---\n");
    }
    else
    {
        printf("--- [gepf_default..gepf enque done] ---\n]");
    }

    do{
        usleep(100000);
        if (MTRUE == g_bGEPFCallback)
        {
            break;
        }
    }while(1);


    /*
    *   6. Bittrue Comparison
    */
	//dump image
	#if 0
	char filename[256];
	sprintf(filename, "/data/GEPF_default_process0x%x_mode%d_temp_depth_wr.bin",(MUINT32) getpid (), type);
	saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_temp_depth_wr_base.virtAddr), buf_temp_depth_wr_base.size);
	char filename2[256];
	sprintf(filename2, "/data/GEPF_default_process0x%x_mode%d_temp_blur_wr.bin",(MUINT32) getpid (), type);
	saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_temp_blur_wr_base.virtAddr), buf_temp_blur_wr_base.size);
	printf("--- [GepfStream save file done] ---\n");
	#endif

	// Bittrue comparison +++++
	printf("--- [GEPF bittrue start] ----\n");
	MUINT32	BitTrueCount=0; //fail count
	MUINT32 size;
	MUINT32 BittrueAdd = 0;
	MUINT32 DmaoAdd = 0;
	MUINT32 sizeTotal = 0;
	switch (type)
	{
	    case 0:
			sizeTotal = sizeof(gepf_temp_ans_debug_wr_0);
			break;
		case 1:
		default:
			sizeTotal = sizeof(gepf_temp_ans_debug_wr_1);
		break;
	}
	printf("--- [GEPF bittrue temp_depth] ----\n");
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_wr.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_depth_wr_base.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			//VV_MSG("OK #%4d	0x%8x == 0x%8x\n", size, BittrueAdd,DmaoAdd);
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_wr.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_wr.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_depth FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));

	printf("--- [GEPF bittrue temp_blur] ----\n");
	BitTrueCount = 0;
	switch (type)
	{
	    case 0:
			sizeTotal = sizeof(gepf_temp_ans_debug_blur_wr_0);
			break;
		case 1:
		default:
			sizeTotal = sizeof(gepf_temp_ans_debug_blur_wr_1);
		break;
	}
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_blur_wr.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_blur_wr_base.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			//VV_MSG("OK #%4d	0x%8x == 0x%8x\n", size, BittrueAdd,DmaoAdd);
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_blur_wr.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_blur_wr.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_blur FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));
	printf("--- [GEPF bittrue end] ----\n");
	// Bittrue comparison -----

	

    /*
    *  7. cleanup the mass
    */

    mpImemDrv->freeVirtBuf(&buf_gepf_depth_rtl);
    mpImemDrv->freeVirtBuf(&buf_gepf_new_table);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_blur_wr);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_wr);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_cur_y);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_y);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src_blur);
    mpImemDrv->freeVirtBuf(&buf_gepf_uv420_rtl);
    mpImemDrv->freeVirtBuf(&buf_gepf_y_rtl);

    mpImemDrv->freeVirtBuf(&buf_gepf_blur_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_yuv_base);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_base);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_base);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_wr_base);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_blur_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_Y);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_UV);
   
    pStream->uninit();   
    printf("--- [GepfStream uninit done] ---\n");

    mpImemDrv->uninit();
    printf("--- [Imem uninit done] ---\n");

    return ret;
}

#include "gepf/mode_0/frame0/gepf_depth_rtl_0_0.h"
#include "gepf/mode_0/frame0/gepf_new_table_0_0.h"
#include "gepf/mode_0/frame0/gepf_temp_ans_debug_blur_wr_0_0.h"
#include "gepf/mode_0/frame0/gepf_temp_ans_debug_wr_0_0.h"
#include "gepf/mode_0/frame0/gepf_temp_cur_y_0_0.h"
#include "gepf/mode_0/frame0/gepf_temp_pre_src_0_0.h"
#include "gepf/mode_0/frame0/gepf_temp_pre_src_blur_0_0.h"
#include "gepf/mode_0/frame0/gepf_temp_pre_y_0_0.h"
#include "gepf/mode_0/frame0/gepf_uv420_rtl_0_0.h"
#include "gepf/mode_0/frame0/gepf_y_rtl_0_0.h"
#include "gepf/mode_1/frame0/gepf_depth_rtl_1_0.h"
#include "gepf/mode_1/frame0/gepf_new_table_1_0.h"
#include "gepf/mode_1/frame0/gepf_temp_ans_debug_blur_wr_1_0.h"
#include "gepf/mode_1/frame0/gepf_temp_ans_debug_wr_1_0.h"
#include "gepf/mode_1/frame0/gepf_temp_cur_y_1_0.h"
#include "gepf/mode_1/frame0/gepf_temp_pre_src_1_0.h"
#include "gepf/mode_1/frame0/gepf_temp_pre_src_blur_1_0.h"
#include "gepf/mode_1/frame0/gepf_temp_pre_y_1_0.h"
#include "gepf/mode_1/frame0/gepf_uv420_rtl_1_0.h"
#include "gepf/mode_1/frame0/gepf_y_rtl_1_0.h"
#include "gepf/mode_1/frame0/gepf_uv420_480_rtl_1_0.h"
#include "gepf/mode_1/frame0/gepf_y_480_rtl_1_0.h"

/*********************************************************************************/
int gepf_dvt_frame0(int type)
{
    int ret=0;
    printf("--- [gepf_dvt_frame0: Mode %d] ---\n", type);
    /*
    *  1. GepfStream Instance
    */ 
    NSCam::NSIoPipe::NSGepf::IGepfStream* pStream;
    pStream= NSCam::NSIoPipe::NSGepf::IGepfStream::createInstance("test_gepf_dvt_frame0");
    pStream->init();   
    printf("--- [test_gepf_dvt_frame0...GepfStream init done] ---\n");

    /*
    *  2. IMemDrv Instance
    */
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMA memory asignment
    */
	//MUINT32 Width = 240;
	//MUINT32 Align = 8;
	MUINT32 WidthYuv = 0xF0, HeightYuv = 0x88;//0x87;
	MUINT32 WidthDep = 0xF0, HeightDep = 0x88;//0x87;
	
    // GEPF +++++
    IMEM_BUF_INFO buf_gepf_depth_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_depth_rtl.size = sizeof(gepf_depth_rtl_0_0);
            break;
        case 1:
        default:
            buf_gepf_depth_rtl.size = sizeof(gepf_depth_rtl_1_0);
            break;
    }
	//buf_gepf_depth_rtl.size = gepf_MemAlign(buf_gepf_depth_rtl.size, Width, Align);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_BYPASS_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_depth_rtl.size *= 4;	// key
	//buf_gepf_depth_rtl.size *= 16;	
    printf("buf_gepf_depth_rtl.size:%d\n",buf_gepf_depth_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_depth_rtl.virtAddr), (MUINT8*)(gepf_depth_rtl_0_0), sizeof(gepf_depth_rtl_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_depth_rtl.virtAddr), (MUINT8*)(gepf_depth_rtl_1_0), sizeof(gepf_depth_rtl_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_rtl);

	IMEM_BUF_INFO buf_gepf_new_table;
    switch (type)
    {
        case 0:
            buf_gepf_new_table.size = sizeof(gepf_new_table_0_0);
            break;
        case 1:
        default:
            buf_gepf_new_table.size = sizeof(gepf_new_table_1_0);
            break;
    }
	//buf_gepf_new_table.size = gepf_MemAlign(buf_gepf_new_table.size, Width, Align);
	//buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size);
	buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size, 0, 0, type);
    printf("buf_gepf_new_table.size:%d\n",buf_gepf_new_table.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_new_table);
    mpImemDrv->mapPhyAddr(&buf_gepf_new_table);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_new_table.virtAddr), (MUINT8*)(gepf_new_table_0_0), sizeof(gepf_new_table_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_new_table.virtAddr), (MUINT8*)(gepf_new_table_1_0), sizeof(gepf_new_table_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_new_table);

	IMEM_BUF_INFO buf_gepf_temp_cur_y;
    switch (type)
    {
        case 0:
            buf_gepf_temp_cur_y.size = sizeof(gepf_temp_cur_y_0_0);
            break;
        case 1:
        default:
            buf_gepf_temp_cur_y.size = sizeof(gepf_temp_cur_y_1_0);
            break;
    }
	//buf_gepf_temp_cur_y.size = gepf_MemAlign(buf_gepf_temp_cur_y.size, Width, Align);
	buf_gepf_temp_cur_y.size = GEPFQueryDMASize(DMA_GEPF_TEMP_Y, buf_gepf_temp_cur_y.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_temp_cur_y.size *= 4;
	//buf_gepf_temp_cur_y.size *= 16;
    printf("buf_gepf_temp_cur_y.size:%d\n",buf_gepf_temp_cur_y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_cur_y);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_cur_y);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_cur_y.virtAddr), (MUINT8*)(gepf_temp_cur_y_0_0), sizeof(gepf_temp_cur_y_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_cur_y.virtAddr), (MUINT8*)(gepf_temp_cur_y_1_0), sizeof(gepf_temp_cur_y_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_cur_y);

	IMEM_BUF_INFO buf_gepf_temp_pre_y;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_y.size = sizeof(gepf_temp_pre_y_0_0);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_y.size = sizeof(gepf_temp_pre_y_1_0);
            break;
    }
	//buf_gepf_temp_pre_y.size = gepf_MemAlign(buf_gepf_temp_pre_y.size, Width, Align);
	buf_gepf_temp_pre_y.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_Y, buf_gepf_temp_pre_y.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_y.size *= 4;
	//buf_gepf_temp_pre_y.size *= 16;
    printf("buf_gepf_temp_pre_y.size:%d\n",buf_gepf_temp_pre_y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_y);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_y);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_y.virtAddr), (MUINT8*)(gepf_temp_pre_y_0_0), sizeof(gepf_temp_pre_y_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_y.virtAddr), (MUINT8*)(gepf_temp_pre_y_1_0), sizeof(gepf_temp_pre_y_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_y);

	IMEM_BUF_INFO buf_gepf_temp_pre_src;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_src.size = sizeof(gepf_temp_pre_src_0_0);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_src.size = sizeof(gepf_temp_pre_src_1_0);
            break;
    }
	//buf_gepf_temp_pre_src.size = gepf_MemAlign(buf_gepf_temp_pre_src.size, Width, Align);
	buf_gepf_temp_pre_src.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_DEPTH, buf_gepf_temp_pre_src.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_src.size *= 4;
	//buf_gepf_temp_pre_src.size *= 16;
    printf("buf_gepf_temp_pre_src.size:%d\n",buf_gepf_temp_pre_src.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src.virtAddr), (MUINT8*)(gepf_temp_pre_src_0_0), sizeof(gepf_temp_pre_src_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src.virtAddr), (MUINT8*)(gepf_temp_pre_src_1_0), sizeof(gepf_temp_pre_src_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src);

	IMEM_BUF_INFO buf_gepf_temp_pre_src_blur;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_src_blur.size = sizeof(gepf_temp_pre_src_blur_0_0);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_src_blur.size = sizeof(gepf_temp_pre_src_blur_1_0);
            break;
    }
	//buf_gepf_temp_pre_src_blur.size = gepf_MemAlign(buf_gepf_temp_pre_src_blur.size, Width, Align);
	buf_gepf_temp_pre_src_blur.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_BLUR, buf_gepf_temp_pre_src_blur.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_src_blur.size *= 4;
	//buf_gepf_temp_pre_src_blur.size *= 16;
    printf("buf_gepf_temp_pre_src_blur.size:%d\n",buf_gepf_temp_pre_src_blur.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src_blur);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src_blur);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src_blur.virtAddr), (MUINT8*)(gepf_temp_pre_src_blur_0_0), sizeof(gepf_temp_pre_src_blur_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src_blur.virtAddr), (MUINT8*)(gepf_temp_pre_src_blur_1_0), sizeof(gepf_temp_pre_src_blur_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src_blur);

	IMEM_BUF_INFO buf_gepf_uv420_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_uv420_rtl.size = sizeof(gepf_uv420_rtl_0_0);
            break;
        case 1:
        default:
            buf_gepf_uv420_rtl.size = sizeof(gepf_uv420_rtl_1_0);
            break;
    }
	//buf_gepf_uv420_rtl.size = gepf_MemAlign(buf_gepf_uv420_rtl.size, Width, Align);
	buf_gepf_uv420_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV, buf_gepf_uv420_rtl.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_uv420_rtl.size *= 4;
	//buf_gepf_uv420_rtl.size *= 16;
    printf("buf_gepf_uv420_rtl.size:%d\n",buf_gepf_uv420_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_uv420_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_uv420_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_uv420_rtl.virtAddr), (MUINT8*)(gepf_uv420_rtl_0_0), sizeof(gepf_uv420_rtl_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_uv420_rtl.virtAddr), (MUINT8*)(gepf_uv420_rtl_1_0), sizeof(gepf_uv420_rtl_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_uv420_rtl);

	IMEM_BUF_INFO buf_gepf_y_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_y_rtl.size = sizeof(gepf_y_rtl_0_0);
            break;
        case 1:
        default:
            buf_gepf_y_rtl.size = sizeof(gepf_y_rtl_1_0);
            break;
    }
	//buf_gepf_y_rtl.size = gepf_MemAlign(buf_gepf_y_rtl.size, Width, Align);
	buf_gepf_y_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y, buf_gepf_y_rtl.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_y_rtl.size *= 4;
	//buf_gepf_y_rtl.size *= 16;
    printf("buf_gepf_y_rtl.size:%d\n",buf_gepf_y_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_y_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_y_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_y_rtl.virtAddr), (MUINT8*)(gepf_y_rtl_0_0), sizeof(gepf_y_rtl_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_y_rtl.virtAddr), (MUINT8*)(gepf_y_rtl_1_0), sizeof(gepf_y_rtl_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_y_rtl);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_blur_wr;
    switch (type)
    {
        case 0:
            buf_gepf_temp_ans_debug_blur_wr.size = sizeof(gepf_temp_ans_debug_blur_wr_0_0);
            break;
        case 1:
        default:
            buf_gepf_temp_ans_debug_blur_wr.size = sizeof(gepf_temp_ans_debug_blur_wr_1_0);
            break;
    }
	//buf_gepf_temp_ans_debug_blur_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_blur_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_blur_wr.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_gepf_temp_ans_debug_blur_wr.size, WidthDep, HeightDep, type);
	//buf_gepf_temp_ans_debug_blur_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_blur_wr.size:%d\n",buf_gepf_temp_ans_debug_blur_wr.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_blur_wr);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_blur_wr);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_blur_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_blur_wr_0_0), sizeof(gepf_temp_ans_debug_blur_wr_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_blur_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_blur_wr_1_0), sizeof(gepf_temp_ans_debug_blur_wr_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_blur_wr);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_wr;
    switch (type)
    {
        case 0:
            buf_gepf_temp_ans_debug_wr.size = sizeof(gepf_temp_ans_debug_wr_0_0);
            break;
        case 1:
        default:
            buf_gepf_temp_ans_debug_wr.size = sizeof(gepf_temp_ans_debug_wr_1_0);
            break;
    }
	//buf_gepf_temp_ans_debug_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_wr.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_gepf_temp_ans_debug_wr.size, WidthDep, HeightDep, type);
	//buf_gepf_temp_ans_debug_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_wr.size:%d\n",buf_gepf_temp_ans_debug_wr.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_wr);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_wr);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_wr_0_0), sizeof(gepf_temp_ans_debug_wr_0_0));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_wr_1_0), sizeof(gepf_temp_ans_debug_wr_1_0));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_wr);

	///
	IMEM_BUF_INFO buf_gepf_blur_base;
    //buf_gepf_blur_base.size = buf_gepf_depth_rtl.size;	// key
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_BYPASS_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	//buf_gepf_blur_base.size *= 16;
    printf("buf_gepf_blur_base.size:%d\n",buf_gepf_blur_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_base);
    memset( (MUINT8*)(buf_gepf_blur_base.virtAddr), 0x00000000, buf_gepf_blur_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_base);

	IMEM_BUF_INFO buf_gepf_yuv_base;
    //buf_gepf_yuv_base.size = buf_gepf_depth_rtl.size * 2;	// key
	buf_gepf_yuv_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_YUV, buf_gepf_yuv_base.size, WidthYuv, HeightYuv, type);
	//buf_gepf_yuv_base.size *= 16;
    printf("buf_gepf_yuv_base.size:%d\n",buf_gepf_yuv_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_yuv_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_yuv_base);
    memset( (MUINT8*)(buf_gepf_yuv_base.virtAddr), 0x00000000, buf_gepf_yuv_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_yuv_base);

	IMEM_BUF_INFO buf_temp_depth_base;
    //buf_temp_depth_base.size = buf_gepf_depth_rtl.size;
	buf_temp_depth_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH, buf_temp_depth_base.size, WidthDep, HeightDep, type);
	//buf_temp_depth_base.size *= 16;
    printf("buf_temp_depth_base.size:%d\n",buf_temp_depth_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_base);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_base);
    memset( (MUINT8*)(buf_temp_depth_base.virtAddr), 0x00000000, buf_temp_depth_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_base);

	IMEM_BUF_INFO buf_temp_blur_base;
    //buf_temp_blur_base.size = buf_gepf_depth_rtl.size;
	buf_temp_blur_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR, buf_temp_blur_base.size, WidthDep, HeightDep, type);
	//buf_temp_blur_base.size *= 16;
    printf("buf_temp_blur_base.size:%d\n",buf_temp_blur_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_base);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_base);
    memset( (MUINT8*)(buf_temp_blur_base.virtAddr), 0x00000000, buf_temp_blur_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_base);

	IMEM_BUF_INFO buf_temp_blur_wr_base;
    //buf_temp_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_temp_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_temp_blur_wr_base.size, WidthDep, HeightDep, type);
	//buf_temp_blur_wr_base.size *= 16;	
    printf("buf_temp_blur_wr_base.size:%d\n",buf_temp_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_wr_base);
    memset( (MUINT8*)(buf_temp_blur_wr_base.virtAddr), 0x00000000, buf_temp_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_wr_base);

	IMEM_BUF_INFO buf_temp_depth_wr_base;
    //buf_temp_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_temp_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_temp_depth_wr_base.size, WidthDep, HeightDep, type);
	//buf_temp_depth_wr_base.size *= 16;
    printf("buf_temp_depth_wr_base.size:%d\n",buf_temp_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_wr_base);
    memset( (MUINT8*)(buf_temp_depth_wr_base.virtAddr), 0x00000000, buf_temp_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_wr_base);

	IMEM_BUF_INFO buf_gepf_depth_wr_base;
    //buf_gepf_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH_WR, buf_gepf_depth_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_depth_wr_base.size *= 16;
    printf("buf_gepf_depth_wr_base.size:%d\n",buf_gepf_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_wr_base);
    memset( (MUINT8*)(buf_gepf_depth_wr_base.virtAddr), 0x00000000, buf_gepf_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_wr_base);

	IMEM_BUF_INFO buf_gepf_blur_wr_base;
    //buf_gepf_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR_WR, buf_gepf_blur_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_blur_wr_base.size *= 16;
    printf("buf_gepf_blur_wr_base.size:%d\n",buf_gepf_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_wr_base);
    memset( (MUINT8*)(buf_gepf_blur_wr_base.virtAddr), 0x00000000, buf_gepf_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_wr_base);

	IMEM_BUF_INFO buf_gepf_480_Y;
	switch (type)
    {
        case 0:
            //buf_gepf_480_Y.size = sizeof(gepf_uv420_rtl_0);
            break;
        case 1:
        default:
            buf_gepf_480_Y.size = sizeof(gepf_y_480_rtl_1_0);
            break;
    }
    //buf_gepf_480_Y.size = buf_gepf_depth_rtl.size;
	buf_gepf_480_Y.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y_480, buf_gepf_480_Y.size, WidthYuv, HeightYuv, type);
	//buf_gepf_480_Y.size *= 16;
    printf("buf_gepf_480_Y.size:%d\n",buf_gepf_480_Y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_Y);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_Y);
	switch (type)
    {
        case 0:
            memset( (MUINT8*)(buf_gepf_480_Y.virtAddr), 0x00000000, buf_gepf_480_Y.size);
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_480_Y.virtAddr), (MUINT8*)(gepf_y_480_rtl_1_0), sizeof(gepf_y_480_rtl_1_0));
            break;
    }    
    //memset( (MUINT8*)(buf_gepf_480_Y.virtAddr), 0x00000000, buf_gepf_480_Y.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_Y);

	IMEM_BUF_INFO buf_gepf_480_UV;
	switch (type)
    {
        case 0:
            //buf_gepf_480_UV.size = sizeof(gepf_uv420_rtl_0);
            break;
        case 1:
        default:
            buf_gepf_480_UV.size = sizeof(gepf_uv420_480_rtl_1_0);
            break;
    }
    //buf_gepf_480_UV.size = buf_gepf_depth_rtl.size;
	buf_gepf_480_UV.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV_480, buf_gepf_480_UV.size, WidthYuv, HeightYuv, type);
	//buf_gepf_480_UV.size *= 16;
    printf("buf_gepf_480_UV.size:%d\n",buf_gepf_480_UV.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_UV);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_UV);
	switch (type)
    {
        case 0:
            memset( (MUINT8*)(buf_gepf_480_UV.virtAddr), 0x00000000, buf_gepf_480_UV.size);
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_480_UV.virtAddr), (MUINT8*)(gepf_uv420_480_rtl_1_0), sizeof(gepf_uv420_480_rtl_1_0));
            break;
    }    
    //memset( (MUINT8*)(buf_gepf_480_UV.virtAddr), 0x00000000, buf_gepf_480_UV.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_UV);

	IMEM_BUF_INFO buf_bypass_depth_wr_base;
    //buf_gepf_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_bypass_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_BYPASS_DEPTH_WR, buf_bypass_depth_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_depth_wr_base.size *= 16;
    printf("buf_bypass_depth_wr_base.size:%d\n",buf_bypass_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_bypass_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_bypass_depth_wr_base);
    memset( (MUINT8*)(buf_bypass_depth_wr_base.virtAddr), 0x00000000, buf_bypass_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_bypass_depth_wr_base);

	IMEM_BUF_INFO buf_bypass_blur_wr_base;
    //buf_gepf_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_bypass_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_BYPASS_BLUR_WR, buf_bypass_blur_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_blur_wr_base.size *= 16;
    printf("buf_bypass_blur_wr_base.size:%d\n",buf_bypass_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_bypass_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_bypass_blur_wr_base);
    memset( (MUINT8*)(buf_bypass_blur_wr_base.virtAddr), 0x00000000, buf_bypass_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_bypass_blur_wr_base);
    // GEPF -----
 
    printf("#########################################################\n");
    printf("###########GEPF Mode%d Start to Test !!!!###########\n", type);
    printf("#########################################################\n");

    /*
    *  5-1. GEPF stream's request configuration: rGepfParams
    */
    GEPFParams rGepfParams;
    GEPFConfig gepfconfig;
    rGepfParams.mpfnCallback = GEPFCallback;  

	switch (type)
	{
		case 0:
			gepfconfig.Gepf_Ctr_Iteration = 3;//1;
			gepfconfig.Gepf_Ctr_Iteration_480 = 0;//1;
			gepfconfig.Gepf_Ctr_Gepf_Mode = 0;
			gepfconfig.Temp_Depth.u4Stride = 0xF0;
			gepfconfig.Temp_Pre_Depth.u4Stride = 0xF0;
			gepfconfig.Temp_Y.u4Stride = 0xF0;
			gepfconfig.Temp_Pre_Y.u4Stride = 0xF0;			
			break;
		case 1:
		default:
			gepfconfig.Gepf_Ctr_Iteration = 3;
			gepfconfig.Gepf_Ctr_Iteration_480 = 1;//3;
			gepfconfig.Gepf_Ctr_Gepf_Mode = 1;
			gepfconfig.Temp_Depth.u4Stride = 0x1E0;
			gepfconfig.Temp_Pre_Depth.u4Stride = 0x1E0;
			gepfconfig.Temp_Y.u4Stride = 0x1E0;
			gepfconfig.Temp_Pre_Y.u4Stride = 0x1E0;	
			break;
	}

    //gepfconfig.Gepf_Ctr_Iteration;
    gepfconfig.Gepf_Ctr_Max_Filter_Val_Weight = 0x1F3;//0x1F4;
    //gepfconfig.Gepf_Ctr_Iteration_480;
    //gepfconfig.Gepf_Ctr_Gepf_Mode;
    gepfconfig.Gepf_Ctr_Focus_Value = 0x78;//0x6F;
    gepfconfig.Gepf_Ctr_Lamda = 0x2618;//0x3200;
    gepfconfig.Gepf_Ctr_Dof_M = 0x10;
    gepfconfig.Gepf_Ctr_Lamda_480 = 0x3200;
    gepfconfig.Temp_Ctr_Coeff_Value = 0;
    gepfconfig.Temp_Ctr_Bypass_En = 1;//0;

    gepfconfig.Gepf_Focus.dmaport = DMA_GEPF_GEPF_FOCUS;
	gepfconfig.Gepf_Focus.u4BufVA = buf_gepf_new_table.virtAddr;
	gepfconfig.Gepf_Focus.u4BufPA = buf_gepf_new_table.phyAddr;
	gepfconfig.Gepf_Focus.u4BufSize = buf_gepf_new_table.size;
    gepfconfig.Gepf_YUV.dmaport = DMA_GEPF_GEPF_YUV;
	gepfconfig.Gepf_YUV.u4BufVA = buf_gepf_yuv_base.virtAddr;
	gepfconfig.Gepf_YUV.u4BufPA = buf_gepf_yuv_base.phyAddr;
    gepfconfig.Gepf_YUV.u4BufSize = buf_gepf_yuv_base.size;
	gepfconfig.Gepf_YUV.u4ImgWidth = WidthYuv;//0xF0;
	gepfconfig.Gepf_YUV.u4ImgHeight = HeightYuv;//0x87;
    gepfconfig.Gepf_Y.dmaport = DMA_GEPF_GEPF_Y;
	gepfconfig.Gepf_Y.u4BufVA = buf_gepf_y_rtl.virtAddr;
	gepfconfig.Gepf_Y.u4BufPA = buf_gepf_y_rtl.phyAddr;
	gepfconfig.Gepf_Y.u4BufSize = buf_gepf_y_rtl.size;
	gepfconfig.Gepf_Y.u4Stride = 0xF0;
    gepfconfig.Gepf_UV.dmaport = DMA_GEPF_GEPF_UV;
	gepfconfig.Gepf_UV.u4BufVA = buf_gepf_uv420_rtl.virtAddr;
	gepfconfig.Gepf_UV.u4BufPA = buf_gepf_uv420_rtl.phyAddr;
	gepfconfig.Gepf_UV.u4BufSize = buf_gepf_uv420_rtl.size;
	gepfconfig.Gepf_UV.u4Stride = 0xF0;
    gepfconfig.Gepf_Y_480.dmaport = DMA_GEPF_GEPF_Y_480;
	gepfconfig.Gepf_Y_480.u4BufVA = buf_gepf_temp_pre_y.virtAddr;//buf_gepf_480_Y.virtAddr;
	gepfconfig.Gepf_Y_480.u4BufPA = buf_gepf_temp_pre_y.phyAddr;//buf_gepf_480_Y.phyAddr;
	gepfconfig.Gepf_Y_480.u4BufSize = buf_gepf_temp_pre_y.size;//buf_gepf_480_Y.size;
	gepfconfig.Gepf_Y_480.u4Stride = 0x1E0;
    gepfconfig.Gepf_UV_480.dmaport = DMA_GEPF_GEPF_UV_480;
	gepfconfig.Gepf_UV_480.u4BufVA = buf_gepf_480_UV.virtAddr;
	gepfconfig.Gepf_UV_480.u4BufPA = buf_gepf_480_UV.phyAddr;
	gepfconfig.Gepf_UV_480.u4BufSize = buf_gepf_480_UV.size;
	gepfconfig.Gepf_UV_480.u4Stride = 0x1E0;
    gepfconfig.Gepf_Depth.dmaport = DMA_GEPF_GEPF_DEPTH;
	gepfconfig.Gepf_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;
	gepfconfig.Gepf_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;
	gepfconfig.Gepf_Depth.u4BufSize = buf_gepf_depth_rtl.size;
	gepfconfig.Gepf_Depth.u4Stride = 0xF0;
	gepfconfig.Gepf_Depth.u4ImgWidth = WidthDep;//0xF0;
	gepfconfig.Gepf_Depth.u4ImgHeight = HeightDep;//0x87;
    gepfconfig.Gepf_Depth_WR.dmaport = DMA_GEPF_GEPF_DEPTH_WR;
	gepfconfig.Gepf_Depth_WR.u4BufVA = buf_gepf_depth_wr_base.virtAddr;
	gepfconfig.Gepf_Depth_WR.u4BufPA = buf_gepf_depth_wr_base.phyAddr;
	gepfconfig.Gepf_Depth_WR.u4BufSize = buf_gepf_depth_wr_base.size;
    gepfconfig.Gepf_Blur.dmaport = DMA_GEPF_GEPF_BLUR;
	gepfconfig.Gepf_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;
	gepfconfig.Gepf_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;
	gepfconfig.Gepf_Blur.u4BufSize = buf_gepf_blur_base.size;
    gepfconfig.Gepf_Blur_WR.dmaport = DMA_GEPF_GEPF_BLUR_WR;
	gepfconfig.Gepf_Blur_WR.u4BufVA = buf_gepf_blur_wr_base.virtAddr;
	gepfconfig.Gepf_Blur_WR.u4BufPA = buf_gepf_blur_wr_base.phyAddr;
	gepfconfig.Gepf_Blur_WR.u4BufSize = buf_gepf_blur_wr_base.size;
    gepfconfig.Temp_Y.dmaport = DMA_GEPF_TEMP_Y;
	gepfconfig.Temp_Y.u4BufVA = buf_gepf_temp_pre_y.virtAddr;//buf_gepf_temp_cur_y.virtAddr;
	gepfconfig.Temp_Y.u4BufPA = buf_gepf_temp_pre_y.phyAddr;//buf_gepf_temp_cur_y.phyAddr;
	gepfconfig.Temp_Y.u4BufSize = buf_gepf_temp_pre_y.size;//buf_gepf_temp_cur_y.size;
	gepfconfig.Temp_Pre_Y.dmaport = DMA_GEPF_TEMP_PRE_Y;
	gepfconfig.Temp_Pre_Y.u4BufVA = buf_gepf_temp_pre_y.virtAddr;
	gepfconfig.Temp_Pre_Y.u4BufPA = buf_gepf_temp_pre_y.phyAddr;
	gepfconfig.Temp_Pre_Y.u4BufSize = buf_gepf_temp_pre_y.size;
	gepfconfig.Temp_Depth.dmaport = DMA_GEPF_TEMP_DEPTH;
	gepfconfig.Temp_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;//buf_temp_depth_base.virtAddr;
	gepfconfig.Temp_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;//buf_temp_depth_base.phyAddr;
	gepfconfig.Temp_Depth.u4BufSize = buf_gepf_depth_rtl.size;//buf_temp_depth_base.size;
    gepfconfig.Temp_Pre_Depth.dmaport = DMA_GEPF_TEMP_PRE_DEPTH;
	gepfconfig.Temp_Pre_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;//buf_gepf_temp_pre_src.virtAddr;
	gepfconfig.Temp_Pre_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;//buf_gepf_temp_pre_src.phyAddr;
	gepfconfig.Temp_Pre_Depth.u4BufSize = buf_gepf_depth_rtl.size;//buf_gepf_temp_pre_src.size;
	gepfconfig.Temp_Depth_WR.dmaport = DMA_GEPF_TEMP_DEPTH_WR;
	gepfconfig.Temp_Depth_WR.u4BufVA = buf_temp_depth_wr_base.virtAddr;
	gepfconfig.Temp_Depth_WR.u4BufPA = buf_temp_depth_wr_base.phyAddr;
	gepfconfig.Temp_Depth_WR.u4BufSize = buf_temp_depth_wr_base.size;
    gepfconfig.Temp_Blur.dmaport = DMA_GEPF_TEMP_BLUR;
	gepfconfig.Temp_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;//buf_temp_blur_base.virtAddr;
	gepfconfig.Temp_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;//buf_temp_blur_base.phyAddr;
	gepfconfig.Temp_Blur.u4BufSize = buf_gepf_blur_base.size;//buf_temp_blur_base.size;
    gepfconfig.Temp_Pre_Blur.dmaport = DMA_GEPF_TEMP_PRE_BLUR;
	gepfconfig.Temp_Pre_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;//buf_gepf_temp_pre_src_blur.virtAddr;
	gepfconfig.Temp_Pre_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;//buf_gepf_temp_pre_src_blur.phyAddr;
	gepfconfig.Temp_Pre_Blur.u4BufSize = buf_gepf_blur_base.size;//buf_gepf_temp_pre_src_blur.size;
    gepfconfig.Temp_Blur_WR.dmaport = DMA_GEPF_TEMP_BLUR_WR;
	gepfconfig.Temp_Blur_WR.u4BufVA = buf_temp_blur_wr_base.virtAddr;
	gepfconfig.Temp_Blur_WR.u4BufPA = buf_temp_blur_wr_base.phyAddr;
	gepfconfig.Temp_Blur_WR.u4BufSize = buf_temp_blur_wr_base.size;
    gepfconfig.Bypass_Depth.dmaport = DMA_GEPF_BYPASS_DEPTH;
	gepfconfig.Bypass_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;//buf_gepf_depth_wr_base.virtAddr;//buf_gepf_depth_rtl.virtAddr;
	gepfconfig.Bypass_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;//buf_gepf_depth_wr_base.phyAddr;//buf_gepf_depth_rtl.phyAddr;
	gepfconfig.Bypass_Depth.u4BufSize = buf_gepf_depth_rtl.size;//buf_gepf_depth_wr_base.size;//buf_gepf_depth_rtl.size;
    gepfconfig.Bypass_Depth_WR.dmaport = DMA_GEPF_BYPASS_DEPTH_WR;
	gepfconfig.Bypass_Depth_WR.u4BufVA = buf_bypass_depth_wr_base.virtAddr;
	gepfconfig.Bypass_Depth_WR.u4BufPA = buf_bypass_depth_wr_base.phyAddr;
	gepfconfig.Bypass_Depth_WR.u4BufSize = buf_bypass_depth_wr_base.size;
    gepfconfig.Bypass_Blur.dmaport = DMA_GEPF_BYPASS_BLUR;
	gepfconfig.Bypass_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;//buf_gepf_blur_wr_base.virtAddr;//buf_gepf_blur_base.virtAddr;
	gepfconfig.Bypass_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;//buf_gepf_blur_wr_base.phyAddr;//buf_gepf_blur_base.phyAddr;
	gepfconfig.Bypass_Blur.u4BufSize = buf_gepf_blur_base.size;//buf_gepf_blur_wr_base.size;//buf_gepf_blur_base.size;
    gepfconfig.Bypass_Blur_WR.dmaport = DMA_GEPF_BYPASS_BLUR_WR;
	gepfconfig.Bypass_Blur_WR.u4BufVA = buf_bypass_blur_wr_base.virtAddr;
	gepfconfig.Bypass_Blur_WR.u4BufPA = buf_bypass_blur_wr_base.phyAddr;
	gepfconfig.Bypass_Blur_WR.u4BufSize = buf_bypass_blur_wr_base.size;
	
    //printf("####apli.c.PA(%p), apli.p.PA(%p), mvi.PA(%p), imgi.c.PA(%p), imgi.p.PA(%p), mvo.PA(%p), bvo.PA(%p)\n", rscconfig.Rsc_Apli_c.u4BufPA,rscconfig.Rsc_Apli_p.u4BufPA, rscconfig.Rsc_mvi.u4BufPA, rscconfig.Rsc_Imgi_c.u4BufPA, rscconfig.Rsc_Imgi_p.u4BufPA, rscconfig.Rsc_mvo.u4BufPA, rscconfig.Rsc_bvo.u4BufPA);

    rGepfParams.mGEPFConfigVec.push_back(gepfconfig);
    g_bGEPFCallback = MFALSE;


    /*
    *   5-2. request enqued to gepfstream
    */
    //enque
    ret=pStream->GEPFenque(rGepfParams);
    if(!ret)
    {
        printf("--- ERRRRRRRRR [gepf_dvt_frame0..gepf enque fail] ---\n");
    }
    else
    {
        printf("--- [gepf_dvt_frame0..gepf enque done] ---\n]");
    }

    do{
        usleep(100000);
        if (MTRUE == g_bGEPFCallback)
        {
            break;
        }
    }while(1);


    /*
    *   6. Bittrue Comparison
    */
	//dump image
	#if 0
	char filename[256];
	sprintf(filename, "/data/gepf_dvt_frame0_process0x%x_mode%d_temp_depth_wr.bin",(MUINT32) getpid (), type);
	saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_temp_depth_wr_base.virtAddr), buf_temp_depth_wr_base.size);
	char filename2[256];
	sprintf(filename2, "/data/gepf_dvt_frame0_process0x%x_mode%d_temp_blur_wr.bin",(MUINT32) getpid (), type);
	saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_temp_blur_wr_base.virtAddr), buf_temp_blur_wr_base.size);
	printf("--- [GepfStream save file done] ---\n");
	#endif

	// Bittrue comparison +++++
	printf("--- [GEPF bittrue start] ----\n");
	MUINT32	BitTrueCount=0; //fail count
	MUINT32 size;
	MUINT32 BittrueAdd = 0;
	MUINT32 DmaoAdd = 0;
	MUINT32 sizeTotal = 0;
	switch (type)
	{
	    case 0:
			sizeTotal = sizeof(gepf_temp_ans_debug_wr_0_0);
			break;
		case 1:
		default:
			sizeTotal = sizeof(gepf_temp_ans_debug_wr_1_0);
		break;
	}
	printf("--- [GEPF bittrue temp_depth] ----\n");
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_wr.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_depth_wr_base.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			//VV_MSG("OK #%4d	0x%8x == 0x%8x\n", size, BittrueAdd,DmaoAdd);
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_wr.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_wr.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_depth FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));

	printf("--- [GEPF bittrue temp_blur] ----\n");
	BitTrueCount = 0;
	switch (type)
	{
	    case 0:
			sizeTotal = sizeof(gepf_temp_ans_debug_blur_wr_0_0);
			break;
		case 1:
		default:
			sizeTotal = sizeof(gepf_temp_ans_debug_blur_wr_1_0);
		break;
	}
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_blur_wr.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_blur_wr_base.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			//VV_MSG("OK #%4d	0x%8x == 0x%8x\n", size, BittrueAdd,DmaoAdd);
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_blur_wr.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_blur_wr.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_blur FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));
	printf("--- [GEPF bittrue end] ----\n");
	// Bittrue comparison -----

	

    /*
    *  7. cleanup the mass
    */

    mpImemDrv->freeVirtBuf(&buf_gepf_depth_rtl);
    mpImemDrv->freeVirtBuf(&buf_gepf_new_table);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_blur_wr);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_wr);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_cur_y);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_y);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src_blur);
    mpImemDrv->freeVirtBuf(&buf_gepf_uv420_rtl);
    mpImemDrv->freeVirtBuf(&buf_gepf_y_rtl);

    mpImemDrv->freeVirtBuf(&buf_gepf_blur_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_yuv_base);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_base);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_base);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_wr_base);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_blur_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_Y);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_UV);
    mpImemDrv->freeVirtBuf(&buf_bypass_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_bypass_blur_wr_base);
   
    pStream->uninit();   
    printf("--- [GepfStream uninit done] ---\n");

    mpImemDrv->uninit();
    printf("--- [Imem uninit done] ---\n");

    return ret;
}

#include "gepf/mode_0/frame1/gepf_depth_rtl_0_1.h"
#include "gepf/mode_0/frame1/gepf_new_table_0_1.h"
#include "gepf/mode_0/frame1/gepf_temp_ans_debug_blur_wr_0_1.h"
#include "gepf/mode_0/frame1/gepf_temp_ans_debug_wr_0_1.h"
#include "gepf/mode_0/frame1/gepf_temp_cur_y_0_1.h"
#include "gepf/mode_0/frame1/gepf_temp_pre_src_0_1.h"
#include "gepf/mode_0/frame1/gepf_temp_pre_src_blur_0_1.h"
#include "gepf/mode_0/frame1/gepf_temp_pre_y_0_1.h"
#include "gepf/mode_0/frame1/gepf_uv420_rtl_0_1.h"
#include "gepf/mode_0/frame1/gepf_y_rtl_0_1.h"
#include "gepf/mode_1/frame1/gepf_depth_rtl_1_1.h"
#include "gepf/mode_1/frame1/gepf_new_table_1_1.h"
#include "gepf/mode_1/frame1/gepf_temp_ans_debug_blur_wr_1_1.h"
#include "gepf/mode_1/frame1/gepf_temp_ans_debug_wr_1_1.h"
#include "gepf/mode_1/frame1/gepf_temp_cur_y_1_1.h"
#include "gepf/mode_1/frame1/gepf_temp_pre_src_1_1.h"
#include "gepf/mode_1/frame1/gepf_temp_pre_src_blur_1_1.h"
#include "gepf/mode_1/frame1/gepf_temp_pre_y_1_1.h"
#include "gepf/mode_1/frame1/gepf_uv420_rtl_1_1.h"
#include "gepf/mode_1/frame1/gepf_y_rtl_1_1.h"
#include "gepf/mode_1/frame1/gepf_uv420_480_rtl_1_1.h"
#include "gepf/mode_1/frame1/gepf_y_480_rtl_1_1.h"

/*********************************************************************************/
int gepf_dvt_frame1(int type)
{
    int ret=0;
    printf("--- [gepf_dvt_frame1: Mode %d] ---\n", type);
    /*
    *  1. GepfStream Instance
    */ 
    NSCam::NSIoPipe::NSGepf::IGepfStream* pStream;
    pStream= NSCam::NSIoPipe::NSGepf::IGepfStream::createInstance("test_gepf_dvt_frame1");
    pStream->init();   
    printf("--- [test_gepf_dvt_frame1...GepfStream init done] ---\n");

    /*
    *  2. IMemDrv Instance
    */
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMA memory asignment
    */
	//MUINT32 Width = 240;
	//MUINT32 Align = 8;
	MUINT32 WidthYuv = 0xF0, HeightYuv = 0x88;//0x87;
	MUINT32 WidthDep = 0xF0, HeightDep = 0x88;//0x87;
	
    // GEPF +++++
    IMEM_BUF_INFO buf_gepf_depth_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_depth_rtl.size = sizeof(gepf_depth_rtl_0_1);
            break;
        case 1:
        default:
            buf_gepf_depth_rtl.size = sizeof(gepf_depth_rtl_1_1);
            break;
    }
	//buf_gepf_depth_rtl.size = gepf_MemAlign(buf_gepf_depth_rtl.size, Width, Align);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_BYPASS_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_depth_rtl.size *= 4;	// key
	//buf_gepf_depth_rtl.size *= 16;	
    printf("buf_gepf_depth_rtl.size:%d\n",buf_gepf_depth_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_depth_rtl.virtAddr), (MUINT8*)(gepf_depth_rtl_0_1), sizeof(gepf_depth_rtl_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_depth_rtl.virtAddr), (MUINT8*)(gepf_depth_rtl_1_1), sizeof(gepf_depth_rtl_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_rtl);

	IMEM_BUF_INFO buf_gepf_new_table;
    switch (type)
    {
        case 0:
            buf_gepf_new_table.size = sizeof(gepf_new_table_0_1);
            break;
        case 1:
        default:
            buf_gepf_new_table.size = sizeof(gepf_new_table_1_1);
            break;
    }
	//buf_gepf_new_table.size = gepf_MemAlign(buf_gepf_new_table.size, Width, Align);
	//buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size);
	buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size, 0, 0, type);
    printf("buf_gepf_new_table.size:%d\n",buf_gepf_new_table.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_new_table);
    mpImemDrv->mapPhyAddr(&buf_gepf_new_table);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_new_table.virtAddr), (MUINT8*)(gepf_new_table_0_1), sizeof(gepf_new_table_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_new_table.virtAddr), (MUINT8*)(gepf_new_table_1_1), sizeof(gepf_new_table_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_new_table);

	IMEM_BUF_INFO buf_gepf_temp_cur_y;
    switch (type)
    {
        case 0:
            buf_gepf_temp_cur_y.size = sizeof(gepf_temp_cur_y_0_1);
            break;
        case 1:
        default:
            buf_gepf_temp_cur_y.size = sizeof(gepf_temp_cur_y_1_1);
            break;
    }
	//buf_gepf_temp_cur_y.size = gepf_MemAlign(buf_gepf_temp_cur_y.size, Width, Align);
	buf_gepf_temp_cur_y.size = GEPFQueryDMASize(DMA_GEPF_TEMP_Y, buf_gepf_temp_cur_y.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_temp_cur_y.size *= 4;
	//buf_gepf_temp_cur_y.size *= 16;
    printf("buf_gepf_temp_cur_y.size:%d\n",buf_gepf_temp_cur_y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_cur_y);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_cur_y);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_cur_y.virtAddr), (MUINT8*)(gepf_temp_cur_y_0_1), sizeof(gepf_temp_cur_y_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_cur_y.virtAddr), (MUINT8*)(gepf_temp_cur_y_1_1), sizeof(gepf_temp_cur_y_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_cur_y);

	IMEM_BUF_INFO buf_gepf_temp_pre_y;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_y.size = sizeof(gepf_temp_pre_y_0_1);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_y.size = sizeof(gepf_temp_pre_y_1_1);
            break;
    }
	//buf_gepf_temp_pre_y.size = gepf_MemAlign(buf_gepf_temp_pre_y.size, Width, Align);
	buf_gepf_temp_pre_y.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_Y, buf_gepf_temp_pre_y.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_y.size *= 4;
	//buf_gepf_temp_pre_y.size *= 16;
    printf("buf_gepf_temp_pre_y.size:%d\n",buf_gepf_temp_pre_y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_y);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_y);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_y.virtAddr), (MUINT8*)(gepf_temp_pre_y_0_1), sizeof(gepf_temp_pre_y_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_y.virtAddr), (MUINT8*)(gepf_temp_pre_y_1_1), sizeof(gepf_temp_pre_y_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_y);

	IMEM_BUF_INFO buf_gepf_temp_pre_src;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_src.size = sizeof(gepf_temp_pre_src_0_1);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_src.size = sizeof(gepf_temp_pre_src_1_1);
            break;
    }
	//buf_gepf_temp_pre_src.size = gepf_MemAlign(buf_gepf_temp_pre_src.size, Width, Align);
	buf_gepf_temp_pre_src.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_DEPTH, buf_gepf_temp_pre_src.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_src.size *= 4;
	//buf_gepf_temp_pre_src.size *= 16;
    printf("buf_gepf_temp_pre_src.size:%d\n",buf_gepf_temp_pre_src.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src.virtAddr), (MUINT8*)(gepf_temp_pre_src_0_1), sizeof(gepf_temp_pre_src_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src.virtAddr), (MUINT8*)(gepf_temp_pre_src_1_1), sizeof(gepf_temp_pre_src_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src);

	IMEM_BUF_INFO buf_gepf_temp_pre_src_blur;
    switch (type)
    {
        case 0:
            buf_gepf_temp_pre_src_blur.size = sizeof(gepf_temp_pre_src_blur_0_1);
            break;
        case 1:
        default:
            buf_gepf_temp_pre_src_blur.size = sizeof(gepf_temp_pre_src_blur_1_1);
            break;
    }
	//buf_gepf_temp_pre_src_blur.size = gepf_MemAlign(buf_gepf_temp_pre_src_blur.size, Width, Align);
	buf_gepf_temp_pre_src_blur.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_BLUR, buf_gepf_temp_pre_src_blur.size, WidthDep, HeightDep, type);
	//if (type == 1)
	//	buf_gepf_temp_pre_src_blur.size *= 4;
	//buf_gepf_temp_pre_src_blur.size *= 16;
    printf("buf_gepf_temp_pre_src_blur.size:%d\n",buf_gepf_temp_pre_src_blur.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src_blur);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src_blur);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src_blur.virtAddr), (MUINT8*)(gepf_temp_pre_src_blur_0_1), sizeof(gepf_temp_pre_src_blur_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_pre_src_blur.virtAddr), (MUINT8*)(gepf_temp_pre_src_blur_1_1), sizeof(gepf_temp_pre_src_blur_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src_blur);

	IMEM_BUF_INFO buf_gepf_uv420_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_uv420_rtl.size = sizeof(gepf_uv420_rtl_0_1);
            break;
        case 1:
        default:
            buf_gepf_uv420_rtl.size = sizeof(gepf_uv420_rtl_1_1);
            break;
    }
	//buf_gepf_uv420_rtl.size = gepf_MemAlign(buf_gepf_uv420_rtl.size, Width, Align);
	buf_gepf_uv420_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV, buf_gepf_uv420_rtl.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_uv420_rtl.size *= 4;
	//buf_gepf_uv420_rtl.size *= 16;
    printf("buf_gepf_uv420_rtl.size:%d\n",buf_gepf_uv420_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_uv420_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_uv420_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_uv420_rtl.virtAddr), (MUINT8*)(gepf_uv420_rtl_0_1), sizeof(gepf_uv420_rtl_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_uv420_rtl.virtAddr), (MUINT8*)(gepf_uv420_rtl_1_1), sizeof(gepf_uv420_rtl_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_uv420_rtl);

	IMEM_BUF_INFO buf_gepf_y_rtl;
    switch (type)
    {
        case 0:
            buf_gepf_y_rtl.size = sizeof(gepf_y_rtl_0_1);
            break;
        case 1:
        default:
            buf_gepf_y_rtl.size = sizeof(gepf_y_rtl_1_1);
            break;
    }
	//buf_gepf_y_rtl.size = gepf_MemAlign(buf_gepf_y_rtl.size, Width, Align);
	buf_gepf_y_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y, buf_gepf_y_rtl.size, WidthYuv, HeightYuv, type);
	//if (type == 1)
	//	buf_gepf_y_rtl.size *= 4;
	//buf_gepf_y_rtl.size *= 16;
    printf("buf_gepf_y_rtl.size:%d\n",buf_gepf_y_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_y_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_y_rtl);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_y_rtl.virtAddr), (MUINT8*)(gepf_y_rtl_0_1), sizeof(gepf_y_rtl_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_y_rtl.virtAddr), (MUINT8*)(gepf_y_rtl_1_1), sizeof(gepf_y_rtl_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_y_rtl);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_blur_wr;
    switch (type)
    {
        case 0:
            buf_gepf_temp_ans_debug_blur_wr.size = sizeof(gepf_temp_ans_debug_blur_wr_0_1);
            break;
        case 1:
        default:
            buf_gepf_temp_ans_debug_blur_wr.size = sizeof(gepf_temp_ans_debug_blur_wr_1_1);
            break;
    }
	//buf_gepf_temp_ans_debug_blur_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_blur_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_blur_wr.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_gepf_temp_ans_debug_blur_wr.size, WidthDep, HeightDep, type);
	//buf_gepf_temp_ans_debug_blur_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_blur_wr.size:%d\n",buf_gepf_temp_ans_debug_blur_wr.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_blur_wr);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_blur_wr);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_blur_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_blur_wr_0_1), sizeof(gepf_temp_ans_debug_blur_wr_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_blur_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_blur_wr_1_1), sizeof(gepf_temp_ans_debug_blur_wr_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_blur_wr);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_wr;
    switch (type)
    {
        case 0:
            buf_gepf_temp_ans_debug_wr.size = sizeof(gepf_temp_ans_debug_wr_0_1);
            break;
        case 1:
        default:
            buf_gepf_temp_ans_debug_wr.size = sizeof(gepf_temp_ans_debug_wr_1_1);
            break;
    }
	//buf_gepf_temp_ans_debug_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_wr.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_gepf_temp_ans_debug_wr.size, WidthDep, HeightDep, type);
	//buf_gepf_temp_ans_debug_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_wr.size:%d\n",buf_gepf_temp_ans_debug_wr.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_wr);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_wr);
    switch (type)
    {
        case 0:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_wr_0_1), sizeof(gepf_temp_ans_debug_wr_0_1));
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_wr_1_1), sizeof(gepf_temp_ans_debug_wr_1_1));
            break;
    }    
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_wr);

	///
	IMEM_BUF_INFO buf_gepf_blur_base;
    //buf_gepf_blur_base.size = buf_gepf_depth_rtl.size;	// key
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_BYPASS_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, type);
	//buf_gepf_blur_base.size *= 16;
    printf("buf_gepf_blur_base.size:%d\n",buf_gepf_blur_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_base);
    memset( (MUINT8*)(buf_gepf_blur_base.virtAddr), 0x00000000, buf_gepf_blur_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_base);

	IMEM_BUF_INFO buf_gepf_yuv_base;
    //buf_gepf_yuv_base.size = buf_gepf_depth_rtl.size * 2;	// key
	buf_gepf_yuv_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_YUV, buf_gepf_yuv_base.size, WidthYuv, HeightYuv, type);
	//buf_gepf_yuv_base.size *= 16;
    printf("buf_gepf_yuv_base.size:%d\n",buf_gepf_yuv_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_yuv_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_yuv_base);
    memset( (MUINT8*)(buf_gepf_yuv_base.virtAddr), 0x00000000, buf_gepf_yuv_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_yuv_base);

	IMEM_BUF_INFO buf_temp_depth_base;
    //buf_temp_depth_base.size = buf_gepf_depth_rtl.size;
	buf_temp_depth_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH, buf_temp_depth_base.size, WidthDep, HeightDep, type);
	//buf_temp_depth_base.size *= 16;
    printf("buf_temp_depth_base.size:%d\n",buf_temp_depth_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_base);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_base);
    memset( (MUINT8*)(buf_temp_depth_base.virtAddr), 0x00000000, buf_temp_depth_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_base);

	IMEM_BUF_INFO buf_temp_blur_base;
    //buf_temp_blur_base.size = buf_gepf_depth_rtl.size;
	buf_temp_blur_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR, buf_temp_blur_base.size, WidthDep, HeightDep, type);
	//buf_temp_blur_base.size *= 16;
    printf("buf_temp_blur_base.size:%d\n",buf_temp_blur_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_base);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_base);
    memset( (MUINT8*)(buf_temp_blur_base.virtAddr), 0x00000000, buf_temp_blur_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_base);

	IMEM_BUF_INFO buf_temp_blur_wr_base;
    //buf_temp_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_temp_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_temp_blur_wr_base.size, WidthDep, HeightDep, type);
	//buf_temp_blur_wr_base.size *= 16;	
    printf("buf_temp_blur_wr_base.size:%d\n",buf_temp_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_wr_base);
    memset( (MUINT8*)(buf_temp_blur_wr_base.virtAddr), 0x00000000, buf_temp_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_wr_base);

	IMEM_BUF_INFO buf_temp_depth_wr_base;
    //buf_temp_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_temp_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_temp_depth_wr_base.size, WidthDep, HeightDep, type);
	//buf_temp_depth_wr_base.size *= 16;
    printf("buf_temp_depth_wr_base.size:%d\n",buf_temp_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_wr_base);
    memset( (MUINT8*)(buf_temp_depth_wr_base.virtAddr), 0x00000000, buf_temp_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_wr_base);

	IMEM_BUF_INFO buf_gepf_depth_wr_base;
    //buf_gepf_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH_WR, buf_gepf_depth_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_depth_wr_base.size *= 16;
    printf("buf_gepf_depth_wr_base.size:%d\n",buf_gepf_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_wr_base);
    memset( (MUINT8*)(buf_gepf_depth_wr_base.virtAddr), 0x00000000, buf_gepf_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_wr_base);

	IMEM_BUF_INFO buf_gepf_blur_wr_base;
    //buf_gepf_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR_WR, buf_gepf_blur_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_blur_wr_base.size *= 16;
    printf("buf_gepf_blur_wr_base.size:%d\n",buf_gepf_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_wr_base);
    memset( (MUINT8*)(buf_gepf_blur_wr_base.virtAddr), 0x00000000, buf_gepf_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_wr_base);

	IMEM_BUF_INFO buf_gepf_480_Y;
	switch (type)
    {
        case 0:
            //buf_gepf_480_Y.size = sizeof(gepf_uv420_rtl_0);
            break;
        case 1:
        default:
            buf_gepf_480_Y.size = sizeof(gepf_y_480_rtl_1_1);
            break;
    }
    //buf_gepf_480_Y.size = buf_gepf_depth_rtl.size;
	buf_gepf_480_Y.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y_480, buf_gepf_480_Y.size, WidthYuv, HeightYuv, type);
	//buf_gepf_480_Y.size *= 16;
    printf("buf_gepf_480_Y.size:%d\n",buf_gepf_480_Y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_Y);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_Y);
	switch (type)
    {
        case 0:
            memset( (MUINT8*)(buf_gepf_480_Y.virtAddr), 0x00000000, buf_gepf_480_Y.size);
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_480_Y.virtAddr), (MUINT8*)(gepf_y_480_rtl_1_1), sizeof(gepf_y_480_rtl_1_1));
            break;
    }    
    //memset( (MUINT8*)(buf_gepf_480_Y.virtAddr), 0x00000000, buf_gepf_480_Y.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_Y);

	IMEM_BUF_INFO buf_gepf_480_UV;
	switch (type)
    {
        case 0:
            //buf_gepf_480_UV.size = sizeof(gepf_uv420_rtl_0);
            break;
        case 1:
        default:
            buf_gepf_480_UV.size = sizeof(gepf_uv420_480_rtl_1_1);
            break;
    }
    //buf_gepf_480_UV.size = buf_gepf_depth_rtl.size;
	buf_gepf_480_UV.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV_480, buf_gepf_480_UV.size, WidthYuv, HeightYuv, type);
	//buf_gepf_480_UV.size *= 16;
    printf("buf_gepf_480_UV.size:%d\n",buf_gepf_480_UV.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_UV);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_UV);
	switch (type)
    {
        case 0:
            memset( (MUINT8*)(buf_gepf_480_UV.virtAddr), 0x00000000, buf_gepf_480_UV.size);
            break;
        case 1:
        default:
            memcpy( (MUINT8*)(buf_gepf_480_UV.virtAddr), (MUINT8*)(gepf_uv420_480_rtl_1_1), sizeof(gepf_uv420_480_rtl_1_1));
            break;
    }    
    //memset( (MUINT8*)(buf_gepf_480_UV.virtAddr), 0x00000000, buf_gepf_480_UV.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_UV);

	IMEM_BUF_INFO buf_bypass_depth_wr_base;
    //buf_gepf_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_bypass_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_BYPASS_DEPTH_WR, buf_bypass_depth_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_depth_wr_base.size *= 16;
    printf("buf_bypass_depth_wr_base.size:%d\n",buf_bypass_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_bypass_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_bypass_depth_wr_base);
    memset( (MUINT8*)(buf_bypass_depth_wr_base.virtAddr), 0x00000000, buf_bypass_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_bypass_depth_wr_base);

	IMEM_BUF_INFO buf_bypass_blur_wr_base;
    //buf_gepf_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_bypass_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_BYPASS_BLUR_WR, buf_bypass_blur_wr_base.size, WidthDep, HeightDep, type);
	//buf_gepf_blur_wr_base.size *= 16;
    printf("buf_bypass_blur_wr_base.size:%d\n",buf_bypass_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_bypass_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_bypass_blur_wr_base);
    memset( (MUINT8*)(buf_bypass_blur_wr_base.virtAddr), 0x00000000, buf_bypass_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_bypass_blur_wr_base);
    // GEPF -----
 
    printf("#########################################################\n");
    printf("###########GEPF Mode%d Start to Test !!!!###########\n", type);
    printf("#########################################################\n");

    /*
    *  5-1. GEPF stream's request configuration: rGepfParams
    */
    GEPFParams rGepfParams;
    GEPFConfig gepfconfig;
    rGepfParams.mpfnCallback = GEPFCallback;  

	switch (type)
	{
		case 0:
			gepfconfig.Gepf_Ctr_Iteration = 3;//1;
			gepfconfig.Gepf_Ctr_Iteration_480 = 0;//1;
			gepfconfig.Gepf_Ctr_Gepf_Mode = 0;
			gepfconfig.Temp_Depth.u4Stride = 0xF0;
			gepfconfig.Temp_Pre_Depth.u4Stride = 0xF0;
			gepfconfig.Temp_Y.u4Stride = 0xF0;
			gepfconfig.Temp_Pre_Y.u4Stride = 0xF0;			
			break;
		case 1:
		default:
			gepfconfig.Gepf_Ctr_Iteration = 3;
			gepfconfig.Gepf_Ctr_Iteration_480 = 1;//3;
			gepfconfig.Gepf_Ctr_Gepf_Mode = 1;
			gepfconfig.Temp_Depth.u4Stride = 0x1E0;
			gepfconfig.Temp_Pre_Depth.u4Stride = 0x1E0;
			gepfconfig.Temp_Y.u4Stride = 0x1E0;
			gepfconfig.Temp_Pre_Y.u4Stride = 0x1E0;	
			break;
	}

    //gepfconfig.Gepf_Ctr_Iteration;
    gepfconfig.Gepf_Ctr_Max_Filter_Val_Weight = 0x1F3;//0x1F4;
    //gepfconfig.Gepf_Ctr_Iteration_480;
    //gepfconfig.Gepf_Ctr_Gepf_Mode;
    gepfconfig.Gepf_Ctr_Focus_Value = 0x78;//0x6F;
    gepfconfig.Gepf_Ctr_Lamda = 0x2618;//0x3200;
    gepfconfig.Gepf_Ctr_Dof_M = 0x10;
    gepfconfig.Gepf_Ctr_Lamda_480 = 0x3200;
    gepfconfig.Temp_Ctr_Coeff_Value = 0;
    gepfconfig.Temp_Ctr_Bypass_En = 0;//0;

    gepfconfig.Gepf_Focus.dmaport = DMA_GEPF_GEPF_FOCUS;
	gepfconfig.Gepf_Focus.u4BufVA = buf_gepf_new_table.virtAddr;
	gepfconfig.Gepf_Focus.u4BufPA = buf_gepf_new_table.phyAddr;
	gepfconfig.Gepf_Focus.u4BufSize = buf_gepf_new_table.size;
    gepfconfig.Gepf_YUV.dmaport = DMA_GEPF_GEPF_YUV;
	gepfconfig.Gepf_YUV.u4BufVA = buf_gepf_yuv_base.virtAddr;
	gepfconfig.Gepf_YUV.u4BufPA = buf_gepf_yuv_base.phyAddr;
    gepfconfig.Gepf_YUV.u4BufSize = buf_gepf_yuv_base.size;
	gepfconfig.Gepf_YUV.u4ImgWidth = WidthYuv;//0xF0;
	gepfconfig.Gepf_YUV.u4ImgHeight = HeightYuv;//0x87;
    gepfconfig.Gepf_Y.dmaport = DMA_GEPF_GEPF_Y;
	gepfconfig.Gepf_Y.u4BufVA = buf_gepf_y_rtl.virtAddr;
	gepfconfig.Gepf_Y.u4BufPA = buf_gepf_y_rtl.phyAddr;
	gepfconfig.Gepf_Y.u4BufSize = buf_gepf_y_rtl.size;
	gepfconfig.Gepf_Y.u4Stride = 0xF0;
    gepfconfig.Gepf_UV.dmaport = DMA_GEPF_GEPF_UV;
	gepfconfig.Gepf_UV.u4BufVA = buf_gepf_uv420_rtl.virtAddr;
	gepfconfig.Gepf_UV.u4BufPA = buf_gepf_uv420_rtl.phyAddr;
	gepfconfig.Gepf_UV.u4BufSize = buf_gepf_uv420_rtl.size;
	gepfconfig.Gepf_UV.u4Stride = 0xF0;
    gepfconfig.Gepf_Y_480.dmaport = DMA_GEPF_GEPF_Y_480;
	gepfconfig.Gepf_Y_480.u4BufVA = buf_gepf_temp_cur_y.virtAddr;//buf_gepf_temp_pre_y.virtAddr;//buf_gepf_480_Y.virtAddr;
	gepfconfig.Gepf_Y_480.u4BufPA = buf_gepf_temp_cur_y.phyAddr;//buf_gepf_temp_pre_y.phyAddr;//buf_gepf_480_Y.phyAddr;
	gepfconfig.Gepf_Y_480.u4BufSize = buf_gepf_temp_cur_y.size;//buf_gepf_temp_pre_y.size;//buf_gepf_480_Y.size;
	gepfconfig.Gepf_Y_480.u4Stride = 0x1E0;
    gepfconfig.Gepf_UV_480.dmaport = DMA_GEPF_GEPF_UV_480;
	gepfconfig.Gepf_UV_480.u4BufVA = buf_gepf_480_UV.virtAddr;
	gepfconfig.Gepf_UV_480.u4BufPA = buf_gepf_480_UV.phyAddr;
	gepfconfig.Gepf_UV_480.u4BufSize = buf_gepf_480_UV.size;
	gepfconfig.Gepf_UV_480.u4Stride = 0x1E0;
    gepfconfig.Gepf_Depth.dmaport = DMA_GEPF_GEPF_DEPTH;
	gepfconfig.Gepf_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;
	gepfconfig.Gepf_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;
	gepfconfig.Gepf_Depth.u4BufSize = buf_gepf_depth_rtl.size;
	gepfconfig.Gepf_Depth.u4Stride = 0xF0;
	gepfconfig.Gepf_Depth.u4ImgWidth = WidthDep;//0xF0;
	gepfconfig.Gepf_Depth.u4ImgHeight = HeightDep;//0x87;
    gepfconfig.Gepf_Depth_WR.dmaport = DMA_GEPF_GEPF_DEPTH_WR;
	gepfconfig.Gepf_Depth_WR.u4BufVA = buf_gepf_depth_wr_base.virtAddr;
	gepfconfig.Gepf_Depth_WR.u4BufPA = buf_gepf_depth_wr_base.phyAddr;
	gepfconfig.Gepf_Depth_WR.u4BufSize = buf_gepf_depth_wr_base.size;
    gepfconfig.Gepf_Blur.dmaport = DMA_GEPF_GEPF_BLUR;
	gepfconfig.Gepf_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;
	gepfconfig.Gepf_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;
	gepfconfig.Gepf_Blur.u4BufSize = buf_gepf_blur_base.size;
    gepfconfig.Gepf_Blur_WR.dmaport = DMA_GEPF_GEPF_BLUR_WR;
	gepfconfig.Gepf_Blur_WR.u4BufVA = buf_gepf_blur_wr_base.virtAddr;
	gepfconfig.Gepf_Blur_WR.u4BufPA = buf_gepf_blur_wr_base.phyAddr;
	gepfconfig.Gepf_Blur_WR.u4BufSize = buf_gepf_blur_wr_base.size;
	switch (type)
	{
		case 0:
	    	gepfconfig.Temp_Y.dmaport = DMA_GEPF_TEMP_Y;
			gepfconfig.Temp_Y.u4BufVA = buf_gepf_y_rtl.virtAddr;//buf_gepf_temp_pre_y.virtAddr;//buf_gepf_temp_cur_y.virtAddr;
			gepfconfig.Temp_Y.u4BufPA = buf_gepf_y_rtl.phyAddr;//buf_gepf_temp_pre_y.phyAddr;//buf_gepf_temp_cur_y.phyAddr;
			gepfconfig.Temp_Y.u4BufSize = buf_gepf_y_rtl.size;//buf_gepf_temp_pre_y.size;//buf_gepf_temp_cur_y.size;
			break;
		case 1:
		default:
	    	gepfconfig.Temp_Y.dmaport = DMA_GEPF_TEMP_Y;
			gepfconfig.Temp_Y.u4BufVA = buf_gepf_temp_cur_y.virtAddr;//buf_gepf_temp_cur_y.virtAddr;
			gepfconfig.Temp_Y.u4BufPA = buf_gepf_temp_cur_y.phyAddr;//buf_gepf_temp_cur_y.phyAddr;
			gepfconfig.Temp_Y.u4BufSize = buf_gepf_temp_cur_y.size;//buf_gepf_temp_cur_y.size;
			break;
	}					
	gepfconfig.Temp_Pre_Y.dmaport = DMA_GEPF_TEMP_PRE_Y;
	gepfconfig.Temp_Pre_Y.u4BufVA = buf_gepf_temp_pre_y.virtAddr;
	gepfconfig.Temp_Pre_Y.u4BufPA = buf_gepf_temp_pre_y.phyAddr;
	gepfconfig.Temp_Pre_Y.u4BufSize = buf_gepf_temp_pre_y.size;
	gepfconfig.Temp_Depth.dmaport = DMA_GEPF_TEMP_DEPTH;
	gepfconfig.Temp_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;//buf_temp_depth_base.virtAddr;
	gepfconfig.Temp_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;//buf_temp_depth_base.phyAddr;
	gepfconfig.Temp_Depth.u4BufSize = buf_gepf_depth_rtl.size;//buf_temp_depth_base.size;
    gepfconfig.Temp_Pre_Depth.dmaport = DMA_GEPF_TEMP_PRE_DEPTH;
	gepfconfig.Temp_Pre_Depth.u4BufVA = buf_gepf_temp_pre_src.virtAddr;
	gepfconfig.Temp_Pre_Depth.u4BufPA = buf_gepf_temp_pre_src.phyAddr;
	gepfconfig.Temp_Pre_Depth.u4BufSize = buf_gepf_temp_pre_src.size;
	gepfconfig.Temp_Depth_WR.dmaport = DMA_GEPF_TEMP_DEPTH_WR;
	gepfconfig.Temp_Depth_WR.u4BufVA = buf_temp_depth_wr_base.virtAddr;
	gepfconfig.Temp_Depth_WR.u4BufPA = buf_temp_depth_wr_base.phyAddr;
	gepfconfig.Temp_Depth_WR.u4BufSize = buf_temp_depth_wr_base.size;
    gepfconfig.Temp_Blur.dmaport = DMA_GEPF_TEMP_BLUR;
	gepfconfig.Temp_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;//buf_temp_blur_base.virtAddr;
	gepfconfig.Temp_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;//buf_temp_blur_base.phyAddr;
	gepfconfig.Temp_Blur.u4BufSize = buf_gepf_blur_base.size;//buf_temp_blur_base.size;
    gepfconfig.Temp_Pre_Blur.dmaport = DMA_GEPF_TEMP_PRE_BLUR;
	gepfconfig.Temp_Pre_Blur.u4BufVA = buf_gepf_temp_pre_src_blur.virtAddr;
	gepfconfig.Temp_Pre_Blur.u4BufPA = buf_gepf_temp_pre_src_blur.phyAddr;
	gepfconfig.Temp_Pre_Blur.u4BufSize = buf_gepf_temp_pre_src_blur.size;
    gepfconfig.Temp_Blur_WR.dmaport = DMA_GEPF_TEMP_BLUR_WR;
	gepfconfig.Temp_Blur_WR.u4BufVA = buf_temp_blur_wr_base.virtAddr;
	gepfconfig.Temp_Blur_WR.u4BufPA = buf_temp_blur_wr_base.phyAddr;
	gepfconfig.Temp_Blur_WR.u4BufSize = buf_temp_blur_wr_base.size;
    gepfconfig.Bypass_Depth.dmaport = DMA_GEPF_BYPASS_DEPTH;
	gepfconfig.Bypass_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;//buf_gepf_depth_wr_base.virtAddr;//buf_gepf_depth_rtl.virtAddr;
	gepfconfig.Bypass_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;//buf_gepf_depth_wr_base.phyAddr;//buf_gepf_depth_rtl.phyAddr;
	gepfconfig.Bypass_Depth.u4BufSize = buf_gepf_depth_rtl.size;//buf_gepf_depth_wr_base.size;//buf_gepf_depth_rtl.size;
    gepfconfig.Bypass_Depth_WR.dmaport = DMA_GEPF_BYPASS_DEPTH_WR;
	gepfconfig.Bypass_Depth_WR.u4BufVA = buf_bypass_depth_wr_base.virtAddr;
	gepfconfig.Bypass_Depth_WR.u4BufPA = buf_bypass_depth_wr_base.phyAddr;
	gepfconfig.Bypass_Depth_WR.u4BufSize = buf_bypass_depth_wr_base.size;
    gepfconfig.Bypass_Blur.dmaport = DMA_GEPF_BYPASS_BLUR;
	gepfconfig.Bypass_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;//buf_gepf_blur_wr_base.virtAddr;//buf_gepf_blur_base.virtAddr;
	gepfconfig.Bypass_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;//buf_gepf_blur_wr_base.phyAddr;//buf_gepf_blur_base.phyAddr;
	gepfconfig.Bypass_Blur.u4BufSize = buf_gepf_blur_base.size;//buf_gepf_blur_wr_base.size;//buf_gepf_blur_base.size;
    gepfconfig.Bypass_Blur_WR.dmaport = DMA_GEPF_BYPASS_BLUR_WR;
	gepfconfig.Bypass_Blur_WR.u4BufVA = buf_bypass_blur_wr_base.virtAddr;
	gepfconfig.Bypass_Blur_WR.u4BufPA = buf_bypass_blur_wr_base.phyAddr;
	gepfconfig.Bypass_Blur_WR.u4BufSize = buf_bypass_blur_wr_base.size;
	
    //printf("####apli.c.PA(%p), apli.p.PA(%p), mvi.PA(%p), imgi.c.PA(%p), imgi.p.PA(%p), mvo.PA(%p), bvo.PA(%p)\n", rscconfig.Rsc_Apli_c.u4BufPA,rscconfig.Rsc_Apli_p.u4BufPA, rscconfig.Rsc_mvi.u4BufPA, rscconfig.Rsc_Imgi_c.u4BufPA, rscconfig.Rsc_Imgi_p.u4BufPA, rscconfig.Rsc_mvo.u4BufPA, rscconfig.Rsc_bvo.u4BufPA);

    rGepfParams.mGEPFConfigVec.push_back(gepfconfig);
    g_bGEPFCallback = MFALSE;


    /*
    *   5-2. request enqued to gepfstream
    */
    //enque
    ret=pStream->GEPFenque(rGepfParams);
    if(!ret)
    {
        printf("--- ERRRRRRRRR [gepf_dvt_frame1..gepf enque fail] ---\n");
    }
    else
    {
        printf("--- [gepf_dvt_frame1..gepf enque done] ---\n]");
    }

    do{
        usleep(100000);
        if (MTRUE == g_bGEPFCallback)
        {
            break;
        }
    }while(1);


    /*
    *   6. Bittrue Comparison
    */
	//dump image
	#if 0
	char filename[256];
	sprintf(filename, "/data/gepf_dvt_frame1_process0x%x_mode%d_temp_depth_wr.bin",(MUINT32) getpid (), type);
	saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_temp_depth_wr_base.virtAddr), buf_temp_depth_wr_base.size);
	char filename2[256];
	sprintf(filename2, "/data/gepf_dvt_frame1_process0x%x_mode%d_temp_blur_wr.bin",(MUINT32) getpid (), type);
	saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_temp_blur_wr_base.virtAddr), buf_temp_blur_wr_base.size);
	printf("--- [GepfStream save file done] ---\n");
	#endif

	// Bittrue comparison +++++
	printf("--- [GEPF bittrue start] ----\n");
	MUINT32	BitTrueCount=0; //fail count
	MUINT32 size;
	MUINT32 BittrueAdd = 0;
	MUINT32 DmaoAdd = 0;
	MUINT32 sizeTotal = 0;
	switch (type)
	{
	    case 0:
			sizeTotal = sizeof(gepf_temp_ans_debug_wr_0_1);
			break;
		case 1:
		default:
			sizeTotal = sizeof(gepf_temp_ans_debug_wr_1_1);
		break;
	}
	printf("--- [GEPF bittrue temp_depth] ----\n");
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_wr.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_depth_wr_base.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			//VV_MSG("OK #%4d	0x%8x == 0x%8x\n", size, BittrueAdd,DmaoAdd);
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_wr.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_wr.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_depth FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));

	printf("--- [GEPF bittrue temp_blur] ----\n");
	BitTrueCount = 0;
	switch (type)
	{
	    case 0:
			sizeTotal = sizeof(gepf_temp_ans_debug_blur_wr_0_1);
			break;
		case 1:
		default:
			sizeTotal = sizeof(gepf_temp_ans_debug_blur_wr_1_1);
		break;
	}
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_blur_wr.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_blur_wr_base.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			//VV_MSG("OK #%4d	0x%8x == 0x%8x\n", size, BittrueAdd,DmaoAdd);
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_blur_wr.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_blur_wr.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_blur FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));
	printf("--- [GEPF bittrue end] ----\n");
	// Bittrue comparison -----

	

    /*
    *  7. cleanup the mass
    */

    mpImemDrv->freeVirtBuf(&buf_gepf_depth_rtl);
    mpImemDrv->freeVirtBuf(&buf_gepf_new_table);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_blur_wr);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_wr);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_cur_y);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_y);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src_blur);
    mpImemDrv->freeVirtBuf(&buf_gepf_uv420_rtl);
    mpImemDrv->freeVirtBuf(&buf_gepf_y_rtl);

    mpImemDrv->freeVirtBuf(&buf_gepf_blur_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_yuv_base);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_base);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_base);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_wr_base);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_blur_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_Y);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_UV);
    mpImemDrv->freeVirtBuf(&buf_bypass_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_bypass_blur_wr_base);
   
    pStream->uninit();   
    printf("--- [GepfStream uninit done] ---\n");

    mpImemDrv->uninit();
    printf("--- [Imem uninit done] ---\n");

    return ret;
}


MVOID GEPF_multi_enque_gepf_tc00_request1_GEPFCallback(GEPFParams& rParams)
{
    printf("--- [request 1 callback func]\n");
    g_b_multi_enque_gepf_tc00_request1_GEPFCallback = MTRUE;
}

MVOID GEPF_multi_enque_gepf_tc00_request0_GEPFCallback(GEPFParams& rParams)
{
    printf("--- [request 0 callback func]\n");
    if (g_GepfMultipleBuffer == 1)
    {
        if (g_GepfCount%2==0)
        {
            printf("--- [GEPF frame(0/1) in request 0 callback func]\n");            
        }
        else
        {
            printf("--- [GEPF frame(1/1) in request 0 callback func]\n");        
        }

        
    }
    else
    {
        printf("--- [GEPF frame(0/0) in request 0 callback func]\n");
    }
    g_b_multi_enque_gepf_tc00_request0_GEPFCallback = MTRUE;
    g_GepfCount++; 
}


bool multi_enque_gepf_tc00_frames_GEPF_Config()
{
    int ret = 0;

    NSCam::NSIoPipe::NSGepf::IGepfStream* pStream;
    pStream= NSCam::NSIoPipe::NSGepf::IGepfStream::createInstance("tc00_multi_enque");
    pStream->init();
    printf("--- [test_gepf_default...GepfStream init done\n");

    IMemDrv* mpImemDrv = NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMAI memory asignment
    */
    MUINT32 WidthYuv = 0xF0, HeightYuv = 0x87;
	MUINT32 WidthDep = 0xF0, HeightDep = 0x87;
	
    // GEPF +++++
    IMEM_BUF_INFO buf_gepf_depth_rtl;
    buf_gepf_depth_rtl.size = sizeof(gepf_depth_rtl_0);
    //buf_gepf_depth_rtl.size = gepf_MemAlign(buf_gepf_depth_rtl.size, Width, Align);
	buf_gepf_depth_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH, buf_gepf_depth_rtl.size, WidthDep, HeightDep, 0);
	//if (type == 1)
	//	buf_gepf_depth_rtl.size *= 4;
	//buf_gepf_depth_rtl.size *= 16;	
    printf("buf_gepf_depth_rtl.size:%d\n",buf_gepf_depth_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_rtl);
    memcpy( (MUINT8*)(buf_gepf_depth_rtl.virtAddr), (MUINT8*)(gepf_depth_rtl_0), sizeof(gepf_depth_rtl_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_rtl);
	
    IMEM_BUF_INFO buf_gepf_depth_rtl_1;
    buf_gepf_depth_rtl_1.size = sizeof(gepf_depth_rtl_1);
	//buf_gepf_depth_rtl.size = gepf_MemAlign(buf_gepf_depth_rtl.size, Width, Align);
	buf_gepf_depth_rtl_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH, buf_gepf_depth_rtl_1.size, WidthDep, HeightDep, 1);
	//buf_gepf_depth_rtl_1.size *= 4;	// key
	//if (type == 1)
	//	buf_gepf_depth_rtl.size *= 4;
	//buf_gepf_depth_rtl.size *= 16;	
    printf("buf_gepf_depth_rtl_1.size:%d\n",buf_gepf_depth_rtl_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_rtl_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_rtl_1);
    memcpy( (MUINT8*)(buf_gepf_depth_rtl_1.virtAddr), (MUINT8*)(gepf_depth_rtl_1), sizeof(gepf_depth_rtl_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_rtl_1);

	IMEM_BUF_INFO buf_gepf_new_table;
    buf_gepf_new_table.size = sizeof(gepf_new_table_0);
    //buf_gepf_new_table.size = gepf_MemAlign(buf_gepf_new_table.size, Width, Align);
	//buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size);
	buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size, 0, 0, 0);
    printf("buf_gepf_new_table.size:%d\n",buf_gepf_new_table.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_new_table);
    mpImemDrv->mapPhyAddr(&buf_gepf_new_table);
    memcpy( (MUINT8*)(buf_gepf_new_table.virtAddr), (MUINT8*)(gepf_new_table_0), sizeof(gepf_new_table_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_new_table);

	IMEM_BUF_INFO buf_gepf_new_table_1;
    buf_gepf_new_table_1.size = sizeof(gepf_new_table_1);
    //buf_gepf_new_table.size = gepf_MemAlign(buf_gepf_new_table.size, Width, Align);
	//buf_gepf_new_table.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table.size);
	buf_gepf_new_table_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_FOCUS, buf_gepf_new_table_1.size, 0, 0, 1);
    printf("buf_gepf_new_table_1.size:%d\n",buf_gepf_new_table_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_new_table_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_new_table_1);
    memcpy( (MUINT8*)(buf_gepf_new_table_1.virtAddr), (MUINT8*)(gepf_new_table_1), sizeof(gepf_new_table_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_new_table_1);

	IMEM_BUF_INFO buf_gepf_temp_cur_y;
    buf_gepf_temp_cur_y.size = sizeof(gepf_temp_cur_y_0);
    //buf_gepf_temp_cur_y.size = gepf_MemAlign(buf_gepf_temp_cur_y.size, Width, Align);
	buf_gepf_temp_cur_y.size = GEPFQueryDMASize(DMA_GEPF_TEMP_Y, buf_gepf_temp_cur_y.size, WidthYuv, HeightYuv, 0);
	//if (type == 1)
	//	buf_gepf_temp_cur_y.size *= 4;
	//buf_gepf_temp_cur_y.size *= 16;
    printf("buf_gepf_temp_cur_y.size:%d\n",buf_gepf_temp_cur_y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_cur_y);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_cur_y);
    memcpy( (MUINT8*)(buf_gepf_temp_cur_y.virtAddr), (MUINT8*)(gepf_temp_cur_y_0), sizeof(gepf_temp_cur_y_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_cur_y);

	IMEM_BUF_INFO buf_gepf_temp_cur_y_1;
	buf_gepf_temp_cur_y_1.size = sizeof(gepf_temp_cur_y_1);
	//buf_gepf_temp_cur_y.size = gepf_MemAlign(buf_gepf_temp_cur_y.size, Width, Align);
	buf_gepf_temp_cur_y_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_Y, buf_gepf_temp_cur_y_1.size, WidthYuv, HeightYuv, 1);
	//if (type == 1)
	//	buf_gepf_temp_cur_y.size *= 4;
	//buf_gepf_temp_cur_y.size *= 16;
	printf("buf_gepf_temp_cur_y_1.size:%d\n",buf_gepf_temp_cur_y_1.size);
	mpImemDrv->allocVirtBuf(&buf_gepf_temp_cur_y_1);
	mpImemDrv->mapPhyAddr(&buf_gepf_temp_cur_y_1);
	memcpy( (MUINT8*)(buf_gepf_temp_cur_y_1.virtAddr), (MUINT8*)(gepf_temp_cur_y_1), sizeof(gepf_temp_cur_y_1));
	mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_cur_y_1);

	IMEM_BUF_INFO buf_gepf_temp_pre_y;
    buf_gepf_temp_pre_y.size = sizeof(gepf_temp_pre_y_0);
    //buf_gepf_temp_pre_y.size = gepf_MemAlign(buf_gepf_temp_pre_y.size, Width, Align);
	buf_gepf_temp_pre_y.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_Y, buf_gepf_temp_pre_y.size, WidthYuv, HeightYuv, 0);
	//if (type == 1)
	//	buf_gepf_temp_pre_y.size *= 4;
	//buf_gepf_temp_pre_y.size *= 16;
    printf("buf_gepf_temp_pre_y.size:%d\n",buf_gepf_temp_pre_y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_y);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_y);
    memcpy( (MUINT8*)(buf_gepf_temp_pre_y.virtAddr), (MUINT8*)(gepf_temp_pre_y_0), sizeof(gepf_temp_pre_y_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_y);

	IMEM_BUF_INFO buf_gepf_temp_pre_y_1;
    buf_gepf_temp_pre_y_1.size = sizeof(gepf_temp_pre_y_1);
    //buf_gepf_temp_pre_y.size = gepf_MemAlign(buf_gepf_temp_pre_y.size, Width, Align);
	buf_gepf_temp_pre_y_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_Y, buf_gepf_temp_pre_y_1.size, WidthYuv, HeightYuv, 1);
	//if (type == 1)
	//	buf_gepf_temp_pre_y.size *= 4;
	//buf_gepf_temp_pre_y.size *= 16;
    printf("buf_gepf_temp_pre_y_1.size:%d\n",buf_gepf_temp_pre_y_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_y_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_y_1);
    memcpy( (MUINT8*)(buf_gepf_temp_pre_y_1.virtAddr), (MUINT8*)(gepf_temp_pre_y_1), sizeof(gepf_temp_pre_y_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_y_1);

	IMEM_BUF_INFO buf_gepf_temp_pre_src;
    buf_gepf_temp_pre_src.size = sizeof(gepf_temp_pre_src_0);
    //buf_gepf_temp_pre_src.size = gepf_MemAlign(buf_gepf_temp_pre_src.size, Width, Align);
	buf_gepf_temp_pre_src.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_DEPTH, buf_gepf_temp_pre_src.size, WidthDep, HeightDep, 0);
	//if (type == 1)
	//	buf_gepf_temp_pre_src.size *= 4;
	//buf_gepf_temp_pre_src.size *= 16;
    printf("buf_gepf_temp_pre_src.size:%d\n",buf_gepf_temp_pre_src.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src);
    memcpy( (MUINT8*)(buf_gepf_temp_pre_src.virtAddr), (MUINT8*)(gepf_temp_pre_src_0), sizeof(gepf_temp_pre_src_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src);

	IMEM_BUF_INFO buf_gepf_temp_pre_src_1;
    buf_gepf_temp_pre_src_1.size = sizeof(gepf_temp_pre_src_1);
    //buf_gepf_temp_pre_src.size = gepf_MemAlign(buf_gepf_temp_pre_src.size, Width, Align);
	buf_gepf_temp_pre_src_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_DEPTH, buf_gepf_temp_pre_src_1.size, WidthDep, HeightDep, 1);
	//if (type == 1)
	//	buf_gepf_temp_pre_src.size *= 4;
	//buf_gepf_temp_pre_src.size *= 16;
    printf("buf_gepf_temp_pre_src_1.size:%d\n",buf_gepf_temp_pre_src_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src_1);
    memcpy( (MUINT8*)(buf_gepf_temp_pre_src_1.virtAddr), (MUINT8*)(gepf_temp_pre_src_1), sizeof(gepf_temp_pre_src_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src_1);

	IMEM_BUF_INFO buf_gepf_temp_pre_src_blur;
    buf_gepf_temp_pre_src_blur.size = sizeof(gepf_temp_pre_src_blur_0);
    //buf_gepf_temp_pre_src_blur.size = gepf_MemAlign(buf_gepf_temp_pre_src_blur.size, Width, Align);
	buf_gepf_temp_pre_src_blur.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_BLUR, buf_gepf_temp_pre_src_blur.size, WidthDep, HeightDep, 0);
	//if (type == 1)
	//	buf_gepf_temp_pre_src_blur.size *= 4;
	//buf_gepf_temp_pre_src_blur.size *= 16;
    printf("buf_gepf_temp_pre_src_blur.size:%d\n",buf_gepf_temp_pre_src_blur.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src_blur);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src_blur);
    memcpy( (MUINT8*)(buf_gepf_temp_pre_src_blur.virtAddr), (MUINT8*)(gepf_temp_pre_src_blur_0), sizeof(gepf_temp_pre_src_blur_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src_blur);

	IMEM_BUF_INFO buf_gepf_temp_pre_src_blur_1;
    buf_gepf_temp_pre_src_blur_1.size = sizeof(gepf_temp_pre_src_blur_1);
    //buf_gepf_temp_pre_src_blur.size = gepf_MemAlign(buf_gepf_temp_pre_src_blur.size, Width, Align);
	buf_gepf_temp_pre_src_blur_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_PRE_BLUR, buf_gepf_temp_pre_src_blur_1.size, WidthDep, HeightDep, 1);
	//if (type == 1)
	//	buf_gepf_temp_pre_src_blur.size *= 4;
	//buf_gepf_temp_pre_src_blur.size *= 16;
    printf("buf_gepf_temp_pre_src_blur_1.size:%d\n",buf_gepf_temp_pre_src_blur_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_pre_src_blur_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_pre_src_blur_1);
    memcpy( (MUINT8*)(buf_gepf_temp_pre_src_blur_1.virtAddr), (MUINT8*)(gepf_temp_pre_src_blur_1), sizeof(gepf_temp_pre_src_blur_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_pre_src_blur_1);

	IMEM_BUF_INFO buf_gepf_uv420_rtl;
    buf_gepf_uv420_rtl.size = sizeof(gepf_uv420_rtl_0);
    //buf_gepf_uv420_rtl.size = gepf_MemAlign(buf_gepf_uv420_rtl.size, Width, Align);
	buf_gepf_uv420_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV, buf_gepf_uv420_rtl.size, WidthYuv, HeightYuv, 0);
	//if (type == 1)
	//	buf_gepf_uv420_rtl.size *= 4;
	//buf_gepf_uv420_rtl.size *= 16;
    printf("buf_gepf_uv420_rtl.size:%d\n",buf_gepf_uv420_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_uv420_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_uv420_rtl);
    memcpy( (MUINT8*)(buf_gepf_uv420_rtl.virtAddr), (MUINT8*)(gepf_uv420_rtl_0), sizeof(gepf_uv420_rtl_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_uv420_rtl);

	IMEM_BUF_INFO buf_gepf_uv420_rtl_1;
    buf_gepf_uv420_rtl_1.size = sizeof(gepf_uv420_rtl_1);
    //buf_gepf_uv420_rtl.size = gepf_MemAlign(buf_gepf_uv420_rtl.size, Width, Align);
	buf_gepf_uv420_rtl_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV, buf_gepf_uv420_rtl_1.size, WidthYuv, HeightYuv, 1);
	//if (type == 1)
	//	buf_gepf_uv420_rtl.size *= 4;
	//buf_gepf_uv420_rtl.size *= 16;
    printf("buf_gepf_uv420_rtl_1.size:%d\n",buf_gepf_uv420_rtl_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_uv420_rtl_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_uv420_rtl_1);
    memcpy( (MUINT8*)(buf_gepf_uv420_rtl_1.virtAddr), (MUINT8*)(gepf_uv420_rtl_1), sizeof(gepf_uv420_rtl_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_uv420_rtl_1);

	IMEM_BUF_INFO buf_gepf_y_rtl;
    buf_gepf_y_rtl.size = sizeof(gepf_y_rtl_0);
    //buf_gepf_y_rtl.size = gepf_MemAlign(buf_gepf_y_rtl.size, Width, Align);
	buf_gepf_y_rtl.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y, buf_gepf_y_rtl.size, WidthYuv, HeightYuv, 0);
	//if (type == 1)
	//	buf_gepf_y_rtl.size *= 4;
	//buf_gepf_y_rtl.size *= 16;
    printf("buf_gepf_y_rtl.size:%d\n",buf_gepf_y_rtl.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_y_rtl);
    mpImemDrv->mapPhyAddr(&buf_gepf_y_rtl);
    memcpy( (MUINT8*)(buf_gepf_y_rtl.virtAddr), (MUINT8*)(gepf_y_rtl_0), sizeof(gepf_y_rtl_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_y_rtl);

	IMEM_BUF_INFO buf_gepf_y_rtl_1;
    buf_gepf_y_rtl_1.size = sizeof(gepf_y_rtl_1);
    //buf_gepf_y_rtl.size = gepf_MemAlign(buf_gepf_y_rtl.size, Width, Align);
	buf_gepf_y_rtl_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y, buf_gepf_y_rtl_1.size, WidthYuv, HeightYuv, 1);
	//if (type == 1)
	//	buf_gepf_y_rtl.size *= 4;
	//buf_gepf_y_rtl.size *= 16;
    printf("buf_gepf_y_rtl_1.size:%d\n",buf_gepf_y_rtl_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_y_rtl_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_y_rtl_1);
    memcpy( (MUINT8*)(buf_gepf_y_rtl_1.virtAddr), (MUINT8*)(gepf_y_rtl_1), sizeof(gepf_y_rtl_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_y_rtl_1);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_blur_wr;
    buf_gepf_temp_ans_debug_blur_wr.size = sizeof(gepf_temp_ans_debug_blur_wr_0);
    //buf_gepf_temp_ans_debug_blur_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_blur_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_blur_wr.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_gepf_temp_ans_debug_blur_wr.size, WidthDep, HeightDep, 0);
	//buf_gepf_temp_ans_debug_blur_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_blur_wr.size:%d\n",buf_gepf_temp_ans_debug_blur_wr.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_blur_wr);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_blur_wr);
    memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_blur_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_blur_wr_0), sizeof(gepf_temp_ans_debug_blur_wr_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_blur_wr);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_blur_wr_1;
    buf_gepf_temp_ans_debug_blur_wr_1.size = sizeof(gepf_temp_ans_debug_blur_wr_1);
    //buf_gepf_temp_ans_debug_blur_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_blur_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_blur_wr_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_gepf_temp_ans_debug_blur_wr_1.size, WidthDep, HeightDep, 1);
	//buf_gepf_temp_ans_debug_blur_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_blur_wr_1.size:%d\n",buf_gepf_temp_ans_debug_blur_wr_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_blur_wr_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_blur_wr_1);
    memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_blur_wr_1.virtAddr), (MUINT8*)(gepf_temp_ans_debug_blur_wr_1), sizeof(gepf_temp_ans_debug_blur_wr_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_blur_wr_1);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_wr;
    buf_gepf_temp_ans_debug_wr.size = sizeof(gepf_temp_ans_debug_wr_0);
    //buf_gepf_temp_ans_debug_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_wr.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_gepf_temp_ans_debug_wr.size, WidthDep, HeightDep, 0);
	//buf_gepf_temp_ans_debug_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_wr.size:%d\n",buf_gepf_temp_ans_debug_wr.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_wr);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_wr);
    memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_wr.virtAddr), (MUINT8*)(gepf_temp_ans_debug_wr_0), sizeof(gepf_temp_ans_debug_wr_0));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_wr);

	IMEM_BUF_INFO buf_gepf_temp_ans_debug_wr_1;
    buf_gepf_temp_ans_debug_wr_1.size = sizeof(gepf_temp_ans_debug_wr_1);
    //buf_gepf_temp_ans_debug_wr.size = gepf_MemAlign(buf_gepf_temp_ans_debug_wr.size, Width, Align);
	buf_gepf_temp_ans_debug_wr_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_gepf_temp_ans_debug_wr_1.size, WidthDep, HeightDep, 1);
	//buf_gepf_temp_ans_debug_wr.size *= 16;
    printf("buf_gepf_temp_ans_debug_wr_1.size:%d\n",buf_gepf_temp_ans_debug_wr_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_temp_ans_debug_wr_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_temp_ans_debug_wr_1);
    memcpy( (MUINT8*)(buf_gepf_temp_ans_debug_wr_1.virtAddr), (MUINT8*)(gepf_temp_ans_debug_wr_1), sizeof(gepf_temp_ans_debug_wr_1));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_temp_ans_debug_wr_1);

	///
	IMEM_BUF_INFO buf_gepf_blur_base;
    //buf_gepf_blur_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_blur_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR, buf_gepf_blur_base.size, WidthDep, HeightDep, 0);
	//buf_gepf_blur_base.size *= 16;
    printf("buf_gepf_blur_base.size:%d\n",buf_gepf_blur_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_base);
    memset( (MUINT8*)(buf_gepf_blur_base.virtAddr), 0x00000000, buf_gepf_blur_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_base);
	
	IMEM_BUF_INFO buf_gepf_blur_base_1;
    //buf_gepf_blur_base_1.size = buf_gepf_depth_rtl_1.size; // key
	buf_gepf_blur_base_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR, buf_gepf_blur_base_1.size, WidthDep, HeightDep, 1);
	//buf_gepf_blur_base.size *= 16;
    printf("buf_gepf_blur_base_1.size:%d\n",buf_gepf_blur_base_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_base_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_base_1);
    memset( (MUINT8*)(buf_gepf_blur_base_1.virtAddr), 0x00000000, buf_gepf_blur_base_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_base_1);

	IMEM_BUF_INFO buf_gepf_yuv_base;
    //buf_gepf_yuv_base.size = buf_gepf_depth_rtl.size * 2;
	buf_gepf_yuv_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_YUV, buf_gepf_yuv_base.size, WidthYuv, HeightYuv, 0);
	//buf_gepf_yuv_base.size *= 16;
    printf("buf_gepf_yuv_base.size:%d\n",buf_gepf_yuv_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_yuv_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_yuv_base);
    memset( (MUINT8*)(buf_gepf_yuv_base.virtAddr), 0x00000000, buf_gepf_yuv_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_yuv_base);

	IMEM_BUF_INFO buf_gepf_yuv_base_1;
    //buf_gepf_yuv_base_1.size = buf_gepf_depth_rtl_1.size * 2; // key
	buf_gepf_yuv_base_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_YUV, buf_gepf_yuv_base_1.size, WidthYuv, HeightYuv, 1);
	//buf_gepf_yuv_base.size *= 16;
    printf("buf_gepf_yuv_base_1.size:%d\n",buf_gepf_yuv_base_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_yuv_base_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_yuv_base_1);
    memset( (MUINT8*)(buf_gepf_yuv_base_1.virtAddr), 0x00000000, buf_gepf_yuv_base_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_yuv_base_1);

	IMEM_BUF_INFO buf_temp_depth_base;
    //buf_temp_depth_base.size = buf_gepf_depth_rtl.size;
	buf_temp_depth_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH, buf_temp_depth_base.size, WidthDep, HeightDep, 0);
	//buf_temp_depth_base.size *= 16;
    printf("buf_temp_depth_base.size:%d\n",buf_temp_depth_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_base);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_base);
    memset( (MUINT8*)(buf_temp_depth_base.virtAddr), 0x00000000, buf_temp_depth_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_base);

	IMEM_BUF_INFO buf_temp_depth_base_1;
    //buf_temp_depth_base_1.size = buf_gepf_depth_rtl_1.size;
	buf_temp_depth_base_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH, buf_temp_depth_base_1.size, WidthDep, HeightDep, 1);
	//buf_temp_depth_base.size *= 16;
    printf("buf_temp_depth_base_1.size:%d\n",buf_temp_depth_base_1.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_base_1);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_base_1);
    memset( (MUINT8*)(buf_temp_depth_base_1.virtAddr), 0x00000000, buf_temp_depth_base_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_base_1);

	IMEM_BUF_INFO buf_temp_blur_base;
    //buf_temp_blur_base.size = buf_gepf_depth_rtl.size;
	buf_temp_blur_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR, buf_temp_blur_base.size, WidthDep, HeightDep, 0);
	//buf_temp_blur_base.size *= 16;
    printf("buf_temp_blur_base.size:%d\n",buf_temp_blur_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_base);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_base);
    memset( (MUINT8*)(buf_temp_blur_base.virtAddr), 0x00000000, buf_temp_blur_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_base);

	IMEM_BUF_INFO buf_temp_blur_base_1;
    //buf_temp_blur_base_1.size = buf_gepf_depth_rtl_1.size;
	buf_temp_blur_base_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR, buf_temp_blur_base_1.size, WidthDep, HeightDep, 1);
	//buf_temp_blur_base.size *= 16;
    printf("buf_temp_blur_base_1.size:%d\n",buf_temp_blur_base_1.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_base_1);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_base_1);
    memset( (MUINT8*)(buf_temp_blur_base_1.virtAddr), 0x00000000, buf_temp_blur_base_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_base_1);

	IMEM_BUF_INFO buf_temp_blur_wr_base;
    //buf_temp_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_temp_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_temp_blur_wr_base.size, WidthDep, HeightDep, 0);
	//buf_temp_blur_wr_base.size *= 16;	
    printf("buf_temp_blur_wr_base.size:%d\n",buf_temp_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_wr_base);
    memset( (MUINT8*)(buf_temp_blur_wr_base.virtAddr), 0x00000000, buf_temp_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_wr_base);

	IMEM_BUF_INFO buf_temp_blur_wr_base_1;
    //buf_temp_blur_wr_base_1.size = buf_gepf_depth_rtl_1.size;
	buf_temp_blur_wr_base_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_BLUR_WR, buf_temp_blur_wr_base_1.size, WidthDep, HeightDep, 1);
	//buf_temp_blur_wr_base.size *= 16;	
    printf("buf_temp_blur_wr_base_1.size:%d\n",buf_temp_blur_wr_base_1.size);
    mpImemDrv->allocVirtBuf(&buf_temp_blur_wr_base_1);
    mpImemDrv->mapPhyAddr(&buf_temp_blur_wr_base_1);
    memset( (MUINT8*)(buf_temp_blur_wr_base_1.virtAddr), 0x00000000, buf_temp_blur_wr_base_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_blur_wr_base_1);

	IMEM_BUF_INFO buf_temp_depth_wr_base;
    //buf_temp_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_temp_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_temp_depth_wr_base.size, WidthDep, HeightDep, 0);
	//buf_temp_depth_wr_base.size *= 16;
    printf("buf_temp_depth_wr_base.size:%d\n",buf_temp_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_wr_base);
    memset( (MUINT8*)(buf_temp_depth_wr_base.virtAddr), 0x00000000, buf_temp_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_wr_base);

	IMEM_BUF_INFO buf_temp_depth_wr_base_1;
    //buf_temp_depth_wr_base_1.size = buf_gepf_depth_rtl_1.size;
	buf_temp_depth_wr_base_1.size = GEPFQueryDMASize(DMA_GEPF_TEMP_DEPTH_WR, buf_temp_depth_wr_base_1.size, WidthDep, HeightDep, 1);
	//buf_temp_depth_wr_base.size *= 16;
    printf("buf_temp_depth_wr_base_1.size:%d\n",buf_temp_depth_wr_base_1.size);
    mpImemDrv->allocVirtBuf(&buf_temp_depth_wr_base_1);
    mpImemDrv->mapPhyAddr(&buf_temp_depth_wr_base_1);
    memset( (MUINT8*)(buf_temp_depth_wr_base_1.virtAddr), 0x00000000, buf_temp_depth_wr_base_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_temp_depth_wr_base_1);

	IMEM_BUF_INFO buf_gepf_depth_wr_base;
    //buf_gepf_depth_wr_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_depth_wr_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH_WR, buf_gepf_depth_wr_base.size, WidthDep, HeightDep, 0);
	//buf_gepf_depth_wr_base.size *= 16;
    printf("buf_gepf_depth_wr_base.size:%d\n",buf_gepf_depth_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_wr_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_wr_base);
    memset( (MUINT8*)(buf_gepf_depth_wr_base.virtAddr), 0x00000000, buf_gepf_depth_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_wr_base);

	IMEM_BUF_INFO buf_gepf_depth_wr_base_1;
    //buf_gepf_depth_wr_base_1.size = buf_gepf_depth_rtl_1.size;
	buf_gepf_depth_wr_base_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_DEPTH_WR, buf_gepf_depth_wr_base_1.size, WidthDep, HeightDep, 1);
	//buf_gepf_depth_wr_base.size *= 16;
    printf("buf_gepf_depth_wr_base_1.size:%d\n",buf_gepf_depth_wr_base_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_depth_wr_base_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_depth_wr_base_1);
    memset( (MUINT8*)(buf_gepf_depth_wr_base_1.virtAddr), 0x00000000, buf_gepf_depth_wr_base_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_depth_wr_base_1);

	IMEM_BUF_INFO buf_gepf_blur_wr_base;
    //buf_gepf_blur_wr_base.size = buf_gepf_depth_rtl.size;
	buf_gepf_blur_wr_base.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR_WR, buf_gepf_blur_wr_base.size, WidthDep, HeightDep, 0);
	//buf_gepf_blur_wr_base.size *= 16;
    printf("buf_gepf_blur_wr_base.size:%d\n",buf_gepf_blur_wr_base.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_wr_base);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_wr_base);
    memset( (MUINT8*)(buf_gepf_blur_wr_base.virtAddr), 0x00000000, buf_gepf_blur_wr_base.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_wr_base);

	IMEM_BUF_INFO buf_gepf_blur_wr_base_1;
    //buf_gepf_blur_wr_base_1.size = buf_gepf_depth_rtl_1.size;
	buf_gepf_blur_wr_base_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_BLUR_WR, buf_gepf_blur_wr_base_1.size, WidthDep, HeightDep, 1);
	//buf_gepf_blur_wr_base.size *= 16;
    printf("buf_gepf_blur_wr_base_1.size:%d\n",buf_gepf_blur_wr_base_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_blur_wr_base_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_blur_wr_base_1);
    memset( (MUINT8*)(buf_gepf_blur_wr_base_1.virtAddr), 0x00000000, buf_gepf_blur_wr_base_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_blur_wr_base_1);

	IMEM_BUF_INFO buf_gepf_480_Y;
    //buf_gepf_480_Y.size = buf_gepf_depth_rtl.size;
	buf_gepf_480_Y.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y_480, buf_gepf_480_Y.size, WidthYuv, HeightYuv, 0);
	//buf_gepf_480_Y.size *= 16;
    printf("buf_gepf_480_Y.size:%d\n",buf_gepf_480_Y.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_Y);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_Y);
    memset( (MUINT8*)(buf_gepf_480_Y.virtAddr), 0x00000000, buf_gepf_480_Y.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_Y);

	IMEM_BUF_INFO buf_gepf_480_Y_1;
    //buf_gepf_480_Y_1.size = buf_gepf_depth_rtl_1.size;
	buf_gepf_480_Y_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_Y_480, buf_gepf_480_Y_1.size, WidthYuv, HeightYuv, 1);
	//buf_gepf_480_Y.size *= 16;
    printf("buf_gepf_480_Y_1.size:%d\n",buf_gepf_480_Y_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_Y_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_Y_1);
    memset( (MUINT8*)(buf_gepf_480_Y_1.virtAddr), 0x00000000, buf_gepf_480_Y_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_Y_1);

	IMEM_BUF_INFO buf_gepf_480_UV;
    //buf_gepf_480_UV.size = buf_gepf_depth_rtl.size;
	buf_gepf_480_UV.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV_480, buf_gepf_480_UV.size, WidthYuv, HeightYuv, 0);
	//buf_gepf_480_UV.size *= 16;
    printf("buf_gepf_480_UV.size:%d\n",buf_gepf_480_UV.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_UV);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_UV);
    memset( (MUINT8*)(buf_gepf_480_UV.virtAddr), 0x00000000, buf_gepf_480_UV.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_UV);
	
	IMEM_BUF_INFO buf_gepf_480_UV_1;
    //buf_gepf_480_UV_1.size = buf_gepf_depth_rtl_1.size;
	buf_gepf_480_UV_1.size = GEPFQueryDMASize(DMA_GEPF_GEPF_UV_480, buf_gepf_480_UV_1.size, WidthYuv, HeightYuv, 1);
	//buf_gepf_480_UV.size *= 16;
    printf("buf_gepf_480_UV_1.size:%d\n",buf_gepf_480_UV_1.size);
    mpImemDrv->allocVirtBuf(&buf_gepf_480_UV_1);
    mpImemDrv->mapPhyAddr(&buf_gepf_480_UV_1);
    memset( (MUINT8*)(buf_gepf_480_UV_1.virtAddr), 0x00000000, buf_gepf_480_UV_1.size);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &buf_gepf_480_UV_1);
    // GEPF -----

    /*
    * default request
    */
    GEPFParams rGepfParams;
    GEPFConfig gepfconfig;
    rGepfParams.mpfnCallback = GEPF_multi_enque_gepf_tc00_request0_GEPFCallback;
	
	gepfconfig.Gepf_Ctr_Iteration = 1;
	gepfconfig.Gepf_Ctr_Iteration_480 = 1;
	gepfconfig.Gepf_Ctr_Gepf_Mode = 0;
	gepfconfig.Temp_Depth.u4Stride = 0xF0;
	
    //gepfconfig.Gepf_Ctr_Iteration;
    gepfconfig.Gepf_Ctr_Max_Filter_Val_Weight = 0x1F4;
    //gepfconfig.Gepf_Ctr_Iteration_480;
    //gepfconfig.Gepf_Ctr_Gepf_Mode;
    gepfconfig.Gepf_Ctr_Focus_Value = 0x6F;
    gepfconfig.Gepf_Ctr_Lamda = 0x3200;
    gepfconfig.Gepf_Ctr_Dof_M = 0x10;
    gepfconfig.Gepf_Ctr_Lamda_480 = 0x3200;
    gepfconfig.Temp_Ctr_Coeff_Value = 0;
    gepfconfig.Temp_Ctr_Bypass_En = 0;

    gepfconfig.Gepf_Focus.dmaport = DMA_GEPF_GEPF_FOCUS;
	gepfconfig.Gepf_Focus.u4BufVA = buf_gepf_new_table.virtAddr;
	gepfconfig.Gepf_Focus.u4BufPA = buf_gepf_new_table.phyAddr;
	gepfconfig.Gepf_Focus.u4BufSize = buf_gepf_new_table.size;
    gepfconfig.Gepf_YUV.dmaport = DMA_GEPF_GEPF_YUV;
	gepfconfig.Gepf_YUV.u4BufVA = buf_gepf_yuv_base.virtAddr;
	gepfconfig.Gepf_YUV.u4BufPA = buf_gepf_yuv_base.phyAddr;
    gepfconfig.Gepf_YUV.u4BufSize = buf_gepf_yuv_base.size;
	gepfconfig.Gepf_YUV.u4ImgWidth = WidthYuv;//0xF0;
	gepfconfig.Gepf_YUV.u4ImgHeight = HeightYuv;//0x87;
    gepfconfig.Gepf_Y.dmaport = DMA_GEPF_GEPF_Y;
	gepfconfig.Gepf_Y.u4BufVA = buf_gepf_y_rtl.virtAddr;
	gepfconfig.Gepf_Y.u4BufPA = buf_gepf_y_rtl.phyAddr;
	gepfconfig.Gepf_Y.u4BufSize = buf_gepf_y_rtl.size;
	gepfconfig.Gepf_Y.u4Stride = 0xF0;
    gepfconfig.Gepf_UV.dmaport = DMA_GEPF_GEPF_UV;
	gepfconfig.Gepf_UV.u4BufVA = buf_gepf_uv420_rtl.virtAddr;
	gepfconfig.Gepf_UV.u4BufPA = buf_gepf_uv420_rtl.phyAddr;
	gepfconfig.Gepf_UV.u4BufSize = buf_gepf_uv420_rtl.size;
	gepfconfig.Gepf_UV.u4Stride = 0xF0;
    gepfconfig.Gepf_Y_480.dmaport = DMA_GEPF_GEPF_Y_480;
	gepfconfig.Gepf_Y_480.u4BufVA = buf_gepf_480_Y.virtAddr;
	gepfconfig.Gepf_Y_480.u4BufPA = buf_gepf_480_Y.phyAddr;
	gepfconfig.Gepf_Y_480.u4BufSize = buf_gepf_480_Y.size;
	gepfconfig.Gepf_Y_480.u4Stride = 0x1E0;
    gepfconfig.Gepf_UV_480.dmaport = DMA_GEPF_GEPF_UV_480;
	gepfconfig.Gepf_UV_480.u4BufVA = buf_gepf_480_UV.virtAddr;
	gepfconfig.Gepf_UV_480.u4BufPA = buf_gepf_480_UV.phyAddr;
	gepfconfig.Gepf_UV_480.u4BufSize = buf_gepf_480_UV.size;
	gepfconfig.Gepf_UV_480.u4Stride = 0x1E0;
    gepfconfig.Gepf_Depth.dmaport = DMA_GEPF_GEPF_DEPTH;
	gepfconfig.Gepf_Depth.u4BufVA = buf_gepf_depth_rtl.virtAddr;
	gepfconfig.Gepf_Depth.u4BufPA = buf_gepf_depth_rtl.phyAddr;
	gepfconfig.Gepf_Depth.u4BufSize = buf_gepf_depth_rtl.size;
	gepfconfig.Gepf_Depth.u4Stride = 0xF0;
	gepfconfig.Gepf_Depth.u4ImgWidth = WidthDep;//0xF0;
	gepfconfig.Gepf_Depth.u4ImgHeight = HeightDep;//0x87;
    gepfconfig.Gepf_Depth_WR.dmaport = DMA_GEPF_GEPF_DEPTH_WR;
	gepfconfig.Gepf_Depth_WR.u4BufVA = buf_gepf_depth_wr_base.virtAddr;
	gepfconfig.Gepf_Depth_WR.u4BufPA = buf_gepf_depth_wr_base.phyAddr;
	gepfconfig.Gepf_Depth_WR.u4BufSize = buf_gepf_depth_wr_base.size;
    gepfconfig.Gepf_Blur.dmaport = DMA_GEPF_GEPF_BLUR;
	gepfconfig.Gepf_Blur.u4BufVA = buf_gepf_blur_base.virtAddr;
	gepfconfig.Gepf_Blur.u4BufPA = buf_gepf_blur_base.phyAddr;
	gepfconfig.Gepf_Blur.u4BufSize = buf_gepf_blur_base.size;
    gepfconfig.Gepf_Blur_WR.dmaport = DMA_GEPF_GEPF_BLUR_WR;
	gepfconfig.Gepf_Blur_WR.u4BufVA = buf_gepf_blur_wr_base.virtAddr;
	gepfconfig.Gepf_Blur_WR.u4BufPA = buf_gepf_blur_wr_base.phyAddr;
	gepfconfig.Gepf_Blur_WR.u4BufSize = buf_gepf_blur_wr_base.size;
    gepfconfig.Temp_Y.dmaport = DMA_GEPF_TEMP_Y;
	gepfconfig.Temp_Y.u4BufVA = buf_gepf_temp_cur_y.virtAddr;
	gepfconfig.Temp_Y.u4BufPA = buf_gepf_temp_cur_y.phyAddr;
	gepfconfig.Temp_Y.u4BufSize = buf_gepf_temp_cur_y.size;
    gepfconfig.Temp_Pre_Y.dmaport = DMA_GEPF_TEMP_PRE_Y;
	gepfconfig.Temp_Pre_Y.u4BufVA = buf_gepf_temp_pre_y.virtAddr;
	gepfconfig.Temp_Pre_Y.u4BufPA = buf_gepf_temp_pre_y.phyAddr;
	gepfconfig.Temp_Pre_Y.u4BufSize = buf_gepf_temp_pre_y.size;
    gepfconfig.Temp_Depth.dmaport = DMA_GEPF_TEMP_DEPTH;
	gepfconfig.Temp_Depth.u4BufVA = buf_temp_depth_base.virtAddr;
	gepfconfig.Temp_Depth.u4BufPA = buf_temp_depth_base.phyAddr;
	gepfconfig.Temp_Depth.u4BufSize = buf_temp_depth_base.size;
    gepfconfig.Temp_Pre_Depth.dmaport = DMA_GEPF_TEMP_PRE_DEPTH;
	gepfconfig.Temp_Pre_Depth.u4BufVA = buf_gepf_temp_pre_src.virtAddr;
	gepfconfig.Temp_Pre_Depth.u4BufPA = buf_gepf_temp_pre_src.phyAddr;
	gepfconfig.Temp_Pre_Depth.u4BufSize = buf_gepf_temp_pre_src.size;
    gepfconfig.Temp_Depth_WR.dmaport = DMA_GEPF_TEMP_DEPTH_WR;
	gepfconfig.Temp_Depth_WR.u4BufVA = buf_temp_depth_wr_base.virtAddr;
	gepfconfig.Temp_Depth_WR.u4BufPA = buf_temp_depth_wr_base.phyAddr;
	gepfconfig.Temp_Depth_WR.u4BufSize = buf_temp_depth_wr_base.size;
    gepfconfig.Temp_Blur.dmaport = DMA_GEPF_TEMP_BLUR;
	gepfconfig.Temp_Blur.u4BufVA = buf_temp_blur_base.virtAddr;
	gepfconfig.Temp_Blur.u4BufPA = buf_temp_blur_base.phyAddr;
	gepfconfig.Temp_Blur.u4BufSize = buf_temp_blur_base.size;
    gepfconfig.Temp_Pre_Blur.dmaport = DMA_GEPF_TEMP_PRE_BLUR;
	gepfconfig.Temp_Pre_Blur.u4BufVA = buf_gepf_temp_pre_src_blur.virtAddr;
	gepfconfig.Temp_Pre_Blur.u4BufPA = buf_gepf_temp_pre_src_blur.phyAddr;
	gepfconfig.Temp_Pre_Blur.u4BufSize = buf_gepf_temp_pre_src_blur.size;
    gepfconfig.Temp_Blur_WR.dmaport = DMA_GEPF_TEMP_BLUR_WR;
	gepfconfig.Temp_Blur_WR.u4BufVA = buf_temp_blur_wr_base.virtAddr;
	gepfconfig.Temp_Blur_WR.u4BufPA = buf_temp_blur_wr_base.phyAddr;
	gepfconfig.Temp_Blur_WR.u4BufSize = buf_temp_blur_wr_base.size;
    gepfconfig.Bypass_Depth.dmaport = DMA_GEPF_BYPASS_DEPTH;
	gepfconfig.Bypass_Depth.u4BufVA = buf_gepf_depth_wr_base.virtAddr;//buf_gepf_depth_rtl.virtAddr;
	gepfconfig.Bypass_Depth.u4BufPA = buf_gepf_depth_wr_base.phyAddr;//buf_gepf_depth_rtl.phyAddr;
	gepfconfig.Bypass_Depth.u4BufSize = buf_gepf_depth_wr_base.size;//buf_gepf_depth_rtl.size;
    gepfconfig.Bypass_Depth_WR.dmaport = DMA_GEPF_BYPASS_DEPTH_WR;
	gepfconfig.Bypass_Depth_WR.u4BufVA = buf_gepf_depth_wr_base.virtAddr;
	gepfconfig.Bypass_Depth_WR.u4BufPA = buf_gepf_depth_wr_base.phyAddr;
	gepfconfig.Bypass_Depth_WR.u4BufSize = buf_gepf_depth_wr_base.size;
    gepfconfig.Bypass_Blur.dmaport = DMA_GEPF_BYPASS_BLUR;
	gepfconfig.Bypass_Blur.u4BufVA = buf_gepf_blur_wr_base.virtAddr;//buf_gepf_blur_base.virtAddr;
	gepfconfig.Bypass_Blur.u4BufPA = buf_gepf_blur_wr_base.phyAddr;//buf_gepf_blur_base.phyAddr;
	gepfconfig.Bypass_Blur.u4BufSize = buf_gepf_blur_wr_base.size;//buf_gepf_blur_base.size;
    gepfconfig.Bypass_Blur_WR.dmaport = DMA_GEPF_BYPASS_BLUR_WR;
	gepfconfig.Bypass_Blur_WR.u4BufVA = buf_gepf_blur_wr_base.virtAddr;
	gepfconfig.Bypass_Blur_WR.u4BufPA = buf_gepf_blur_wr_base.phyAddr;
	gepfconfig.Bypass_Blur_WR.u4BufSize = buf_gepf_blur_wr_base.size;

    rGepfParams.mGEPFConfigVec.push_back(gepfconfig);

    /*
    *   Another Request
    */ 
    GEPFParams rGepfParams1;
    GEPFConfig gepfconfig1;
    rGepfParams1.mpfnCallback = GEPF_multi_enque_gepf_tc00_request1_GEPFCallback;

	gepfconfig1.Gepf_Ctr_Iteration = 3;
	gepfconfig1.Gepf_Ctr_Iteration_480 = 3;
	gepfconfig1.Gepf_Ctr_Gepf_Mode = 1;
	gepfconfig1.Temp_Depth.u4Stride = 0x1E0;
	
    //gepfconfig1.Gepf_Ctr_Iteration;
    gepfconfig1.Gepf_Ctr_Max_Filter_Val_Weight = 0x1F4;
    //gepfconfig1.Gepf_Ctr_Iteration_480;
    //gepfconfig1.Gepf_Ctr_Gepf_Mode;
    gepfconfig1.Gepf_Ctr_Focus_Value = 0x6F;
    gepfconfig1.Gepf_Ctr_Lamda = 0x3200;
    gepfconfig1.Gepf_Ctr_Dof_M = 0x10;
    gepfconfig1.Gepf_Ctr_Lamda_480 = 0x3200;
    gepfconfig1.Temp_Ctr_Coeff_Value = 0;
    gepfconfig1.Temp_Ctr_Bypass_En = 0;

    gepfconfig1.Gepf_Focus.dmaport = DMA_GEPF_GEPF_FOCUS;
	gepfconfig1.Gepf_Focus.u4BufVA = buf_gepf_new_table_1.virtAddr;
	gepfconfig1.Gepf_Focus.u4BufPA = buf_gepf_new_table_1.phyAddr;
	gepfconfig1.Gepf_Focus.u4BufSize = buf_gepf_new_table_1.size;
    gepfconfig1.Gepf_YUV.dmaport = DMA_GEPF_GEPF_YUV;
	gepfconfig1.Gepf_YUV.u4BufVA = buf_gepf_yuv_base_1.virtAddr;
	gepfconfig1.Gepf_YUV.u4BufPA = buf_gepf_yuv_base_1.phyAddr;
    gepfconfig1.Gepf_YUV.u4BufSize = buf_gepf_yuv_base_1.size;
	gepfconfig1.Gepf_YUV.u4ImgWidth = WidthYuv;//0xF0;
	gepfconfig1.Gepf_YUV.u4ImgHeight = HeightYuv;//0x87;
    gepfconfig1.Gepf_Y.dmaport = DMA_GEPF_GEPF_Y;
	gepfconfig1.Gepf_Y.u4BufVA = buf_gepf_y_rtl_1.virtAddr;
	gepfconfig1.Gepf_Y.u4BufPA = buf_gepf_y_rtl_1.phyAddr;
	gepfconfig1.Gepf_Y.u4BufSize = buf_gepf_y_rtl_1.size;
	gepfconfig1.Gepf_Y.u4Stride = 0xF0;
    gepfconfig1.Gepf_UV.dmaport = DMA_GEPF_GEPF_UV;
	gepfconfig1.Gepf_UV.u4BufVA = buf_gepf_uv420_rtl.virtAddr;
	gepfconfig1.Gepf_UV.u4BufPA = buf_gepf_uv420_rtl.phyAddr;
	gepfconfig1.Gepf_UV.u4BufSize = buf_gepf_uv420_rtl.size;
	gepfconfig1.Gepf_UV.u4Stride = 0xF0;
    gepfconfig1.Gepf_Y_480.dmaport = DMA_GEPF_GEPF_Y_480;
	gepfconfig1.Gepf_Y_480.u4BufVA = buf_gepf_480_Y_1.virtAddr;
	gepfconfig1.Gepf_Y_480.u4BufPA = buf_gepf_480_Y_1.phyAddr;
	gepfconfig1.Gepf_Y_480.u4BufSize = buf_gepf_480_Y_1.size;
	gepfconfig1.Gepf_Y_480.u4Stride = 0x1E0;
    gepfconfig1.Gepf_UV_480.dmaport = DMA_GEPF_GEPF_UV_480;
	gepfconfig1.Gepf_UV_480.u4BufVA = buf_gepf_480_UV_1.virtAddr;
	gepfconfig1.Gepf_UV_480.u4BufPA = buf_gepf_480_UV_1.phyAddr;
	gepfconfig1.Gepf_UV_480.u4BufSize = buf_gepf_480_UV_1.size;
	gepfconfig1.Gepf_UV_480.u4Stride = 0x1E0;
    gepfconfig1.Gepf_Depth.dmaport = DMA_GEPF_GEPF_DEPTH;
	gepfconfig1.Gepf_Depth.u4BufVA = buf_gepf_depth_rtl_1.virtAddr;
	gepfconfig1.Gepf_Depth.u4BufPA = buf_gepf_depth_rtl_1.phyAddr;
	gepfconfig1.Gepf_Depth.u4BufSize = buf_gepf_depth_rtl_1.size;
	gepfconfig1.Gepf_Depth.u4Stride = 0xF0;
	gepfconfig1.Gepf_Depth.u4ImgWidth = WidthDep;//0xF0;
	gepfconfig1.Gepf_Depth.u4ImgHeight = HeightDep;//0x87;
    gepfconfig1.Gepf_Depth_WR.dmaport = DMA_GEPF_GEPF_DEPTH_WR;
	gepfconfig1.Gepf_Depth_WR.u4BufVA = buf_gepf_depth_wr_base_1.virtAddr;
	gepfconfig1.Gepf_Depth_WR.u4BufPA = buf_gepf_depth_wr_base_1.phyAddr;
	gepfconfig1.Gepf_Depth_WR.u4BufSize = buf_gepf_depth_wr_base_1.size;
    gepfconfig1.Gepf_Blur.dmaport = DMA_GEPF_GEPF_BLUR;
	gepfconfig1.Gepf_Blur.u4BufVA = buf_gepf_blur_base_1.virtAddr;
	gepfconfig1.Gepf_Blur.u4BufPA = buf_gepf_blur_base_1.phyAddr;
	gepfconfig1.Gepf_Blur.u4BufSize = buf_gepf_blur_base_1.size;
    gepfconfig1.Gepf_Blur_WR.dmaport = DMA_GEPF_GEPF_BLUR_WR;
	gepfconfig1.Gepf_Blur_WR.u4BufVA = buf_gepf_blur_wr_base_1.virtAddr;
	gepfconfig1.Gepf_Blur_WR.u4BufPA = buf_gepf_blur_wr_base_1.phyAddr;
	gepfconfig1.Gepf_Blur_WR.u4BufSize = buf_gepf_blur_wr_base_1.size;
    gepfconfig1.Temp_Y.dmaport = DMA_GEPF_TEMP_Y;
	gepfconfig1.Temp_Y.u4BufVA = buf_gepf_temp_cur_y_1.virtAddr;
	gepfconfig1.Temp_Y.u4BufPA = buf_gepf_temp_cur_y_1.phyAddr;
	gepfconfig1.Temp_Y.u4BufSize = buf_gepf_temp_cur_y_1.size;
    gepfconfig1.Temp_Pre_Y.dmaport = DMA_GEPF_TEMP_PRE_Y;
	gepfconfig1.Temp_Pre_Y.u4BufVA = buf_gepf_temp_pre_y_1.virtAddr;
	gepfconfig1.Temp_Pre_Y.u4BufPA = buf_gepf_temp_pre_y_1.phyAddr;
	gepfconfig1.Temp_Pre_Y.u4BufSize = buf_gepf_temp_pre_y_1.size;
    gepfconfig1.Temp_Depth.dmaport = DMA_GEPF_TEMP_DEPTH;
	gepfconfig1.Temp_Depth.u4BufVA = buf_temp_depth_base_1.virtAddr;
	gepfconfig1.Temp_Depth.u4BufPA = buf_temp_depth_base_1.phyAddr;
	gepfconfig1.Temp_Depth.u4BufSize = buf_temp_depth_base_1.size;
    gepfconfig1.Temp_Pre_Depth.dmaport = DMA_GEPF_TEMP_PRE_DEPTH;
	gepfconfig1.Temp_Pre_Depth.u4BufVA = buf_gepf_temp_pre_src_1.virtAddr;
	gepfconfig1.Temp_Pre_Depth.u4BufPA = buf_gepf_temp_pre_src_1.phyAddr;
	gepfconfig1.Temp_Pre_Depth.u4BufSize = buf_gepf_temp_pre_src_1.size;
    gepfconfig1.Temp_Depth_WR.dmaport = DMA_GEPF_TEMP_DEPTH_WR;
	gepfconfig1.Temp_Depth_WR.u4BufVA = buf_temp_depth_wr_base_1.virtAddr;
	gepfconfig1.Temp_Depth_WR.u4BufPA = buf_temp_depth_wr_base_1.phyAddr;
	gepfconfig1.Temp_Depth_WR.u4BufSize = buf_temp_depth_wr_base_1.size;
    gepfconfig1.Temp_Blur.dmaport = DMA_GEPF_TEMP_BLUR;
	gepfconfig1.Temp_Blur.u4BufVA = buf_temp_blur_base_1.virtAddr;
	gepfconfig1.Temp_Blur.u4BufPA = buf_temp_blur_base_1.phyAddr;
	gepfconfig1.Temp_Blur.u4BufSize = buf_temp_blur_base_1.size;
    gepfconfig1.Temp_Pre_Blur.dmaport = DMA_GEPF_TEMP_PRE_BLUR;
	gepfconfig1.Temp_Pre_Blur.u4BufVA = buf_gepf_temp_pre_src_blur_1.virtAddr;
	gepfconfig1.Temp_Pre_Blur.u4BufPA = buf_gepf_temp_pre_src_blur_1.phyAddr;
	gepfconfig1.Temp_Pre_Blur.u4BufSize = buf_gepf_temp_pre_src_blur_1.size;
    gepfconfig1.Temp_Blur_WR.dmaport = DMA_GEPF_TEMP_BLUR_WR;
	gepfconfig1.Temp_Blur_WR.u4BufVA = buf_temp_blur_wr_base_1.virtAddr;
	gepfconfig1.Temp_Blur_WR.u4BufPA = buf_temp_blur_wr_base_1.phyAddr;
	gepfconfig1.Temp_Blur_WR.u4BufSize = buf_temp_blur_wr_base_1.size;
    gepfconfig1.Bypass_Depth.dmaport = DMA_GEPF_BYPASS_DEPTH;
	gepfconfig1.Bypass_Depth.u4BufVA = buf_gepf_depth_wr_base_1.virtAddr;//buf_gepf_depth_rtl.virtAddr;
	gepfconfig1.Bypass_Depth.u4BufPA = buf_gepf_depth_wr_base_1.phyAddr;//buf_gepf_depth_rtl.phyAddr;
	gepfconfig1.Bypass_Depth.u4BufSize = buf_gepf_depth_wr_base_1.size;//buf_gepf_depth_rtl.size;
    gepfconfig1.Bypass_Depth_WR.dmaport = DMA_GEPF_BYPASS_DEPTH_WR;
	gepfconfig1.Bypass_Depth_WR.u4BufVA = buf_gepf_depth_wr_base_1.virtAddr;
	gepfconfig1.Bypass_Depth_WR.u4BufPA = buf_gepf_depth_wr_base_1.phyAddr;
	gepfconfig1.Bypass_Depth_WR.u4BufSize = buf_gepf_depth_wr_base_1.size;
    gepfconfig1.Bypass_Blur.dmaport = DMA_GEPF_BYPASS_BLUR;
	gepfconfig1.Bypass_Blur.u4BufVA = buf_gepf_blur_wr_base_1.virtAddr;//buf_gepf_blur_base.virtAddr;
	gepfconfig1.Bypass_Blur.u4BufPA = buf_gepf_blur_wr_base_1.phyAddr;//buf_gepf_blur_base.phyAddr;
	gepfconfig1.Bypass_Blur.u4BufSize = buf_gepf_blur_wr_base_1.size;//buf_gepf_blur_base.size;
    gepfconfig1.Bypass_Blur_WR.dmaport = DMA_GEPF_BYPASS_BLUR_WR;
	gepfconfig1.Bypass_Blur_WR.u4BufVA = buf_gepf_blur_wr_base_1.virtAddr;
	gepfconfig1.Bypass_Blur_WR.u4BufPA = buf_gepf_blur_wr_base_1.phyAddr;
	gepfconfig1.Bypass_Blur_WR.u4BufSize = buf_gepf_blur_wr_base_1.size;

    if (g_GepfMultipleBuffer == 0)
    {        
        rGepfParams1.mGEPFConfigVec.push_back(gepfconfig1);
    }
    else if (g_GepfMultipleBuffer == 1)
    {
        rGepfParams.mGEPFConfigVec.push_back(gepfconfig1);
    }
    
    g_b_multi_enque_gepf_tc00_request0_GEPFCallback = MFALSE;
    g_b_multi_enque_gepf_tc00_request1_GEPFCallback = MFALSE;

    /*
    *   enque the request(s)
    */
    ret=pStream->GEPFenque(rGepfParams);
    if(!ret)
    {
        printf("---multi_enque_gepf_tc00_frame_0 ERRRRRRRRR [.gepf enque fail\n]");
    }
    else
    {
        printf("---multi_enque_gepf_tc00_frame_0 [.gepf enque done\n]");
    }
    if (g_GepfMultipleBuffer == 0)
    {
        ret=pStream->GEPFenque(rGepfParams1);
        if(!ret)
        {
            printf("---multi_enque_gepf_tc00_frame_1 ERRRRRRRRR [..frame1 gepf enque fail\n]");
        }
        else
        {
            printf("---multi_enque_gepf_tc00_frame_1 [.frame1 gepf enque done\n]");
        }

    }

    /*
    *  Compare rsc_mvo/bvo_frame_0
    */
    do{
        usleep(100000);
        if (MTRUE == g_b_multi_enque_gepf_tc00_request0_GEPFCallback)
        {
            break;
        }
    }while(1);


#if 0
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &buf_rsc_mvo_frame);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &buf_rsc_bvo_frame);

    int_data_dma_0 = 0;
    int_data_dma_1 = 0;
    rsc_curr_width = (REG_RSC_RSC_SIZE_0 & 0xfff);
    rsc_curr_height = ((REG_RSC_RSC_SIZE_0 >> 16) & 0xfff);
    int_data_dma_2 = rsc_curr_width-1;
    int_data_dma_3 = rsc_curr_height-1;

    err_cnt_dma = comp_roi_mem_with_file((char*)golden_mvo_frame, 
                          0, 
                          rscconfig.Rsc_Ctrl_0.Rsc_Dpo.u4BufVA,
                          rsc_curr_width,
                          rsc_curr_height,
                          rscconfig.Rsc_Ctrl_0.Rsc_Dpo.u4Stride,
                          int_data_dma_0,
                          int_data_dma_1,
                          int_data_dma_2,
                          int_data_dma_3,
                          0,
                          0,
                          0
                          ); 
    if (err_cnt_dma)
    {
        //Error
        printf("rsc RSC DPO Frame0 bit true fail: errcnt (%d)!!!\n", err_cnt_dma);
        do
        {
            usleep(200000);
            printf("rsc RSC DPO Frame0 error:(%d)!!!\n", err_cnt_dma);
        }while(1);
    }
    else
    {
        //Pass
        printf("rsc RSC DPO Frame0 bit true pass!!!\n");
    }
#endif

    if (g_GepfMultipleBuffer == 0)
    {
        do{
            usleep(100000);
            if (MTRUE == g_b_multi_enque_gepf_tc00_request1_GEPFCallback)
            {
                break;
            }
        }while(1);
    }


#if 0
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &buf_rsc_mvo_frame1);
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &buf_rsc_bvo_frame1);

    int_data_dma_0 = 0;
    int_data_dma_1 = 0;
    rsc_curr_width = (REG_RSC_RSC_SIZE_0 & 0xfff);
    rsc_curr_height = ((REG_RSC_RSC_SIZE_0 >> 16) & 0xfff);
    int_data_dma_2 = rsc_curr_width-1;
    int_data_dma_3 = rsc_curr_height-1;

    err_cnt_dma = comp_roi_mem_with_file((char*)frame1_golden_rsc_wmf_dpo_frame_0, 
                          0, 
                          frame1_rscconfig.Rsc_Ctrl_0.Rsc_Dpo.u4BufVA,
                          rsc_curr_width,
                          rsc_curr_height,
                          frame1_rscconfig.Rsc_Ctrl_0.Rsc_Dpo.u4Stride,
                          int_data_dma_0,
                          int_data_dma_1,
                          int_data_dma_2,
                          int_data_dma_3,
                          0,
                          0,
                          0
                          ); 
    if (err_cnt_dma)
    {
        //Error
        printf("rsc frame1 RSC DPO Frame0 bit true fail: errcnt (%d)!!!\n", err_cnt_dma);
        do
        {
            usleep(200000);
            printf("rsc frame1 RSC DPO Frame0 error:(%d)!!!\n", err_cnt_dma);
        }while(1);
    }
    else
    {
        //Pass
        printf("rsc frame1 RSC DPO Frame0 bit true pass!!!\n");
    }
#endif

	// Bittrue comparison +++++
	printf("--- [GEPF bittrue start] ----\n");
	MUINT32	BitTrueCount=0; //fail count
	MUINT32 size;
	MUINT32 BittrueAdd = 0;
	MUINT32 DmaoAdd = 0;
	MUINT32 sizeTotal = 0;
	sizeTotal = sizeof(gepf_temp_ans_debug_wr_0);
	printf("--- [GEPF bittrue temp_depth_0] ----\n");
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_wr.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_depth_wr_base.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_wr.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_wr.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_depth_0 FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));

	BitTrueCount = 0;
	sizeTotal = sizeof(gepf_temp_ans_debug_blur_wr_0);
	printf("--- [GEPF bittrue temp_blur_0] ----\n");
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_blur_wr.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_blur_wr_base.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_blur_wr.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_blur_wr.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_blur_0 FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));

	BitTrueCount=0;
	sizeTotal = sizeof(gepf_temp_ans_debug_wr_1);
	printf("--- [GEPF bittrue temp_depth_1] ----\n");
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_wr_1.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_depth_wr_base_1.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_wr_1.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_wr_1.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_depth_1 FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));

	BitTrueCount = 0;
	sizeTotal = sizeof(gepf_temp_ans_debug_blur_wr_1);
	printf("--- [GEPF bittrue temp_blur_1] ----\n");
	for(size=0;size<(sizeTotal/4);size++)
	{
		BittrueAdd = *((MUINT32*)buf_gepf_temp_ans_debug_blur_wr_1.virtAddr+size);
		DmaoAdd = *((MUINT32*)buf_temp_blur_wr_base_1.virtAddr+size);
		if(BittrueAdd == DmaoAdd)
		{
			if(size<4)
			{
				//LOG_MSG("OK #%d	0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
			else if(size>=(buf_gepf_temp_ans_debug_blur_wr_1.size/4)-4)
			{
				// LOG_MSG("OK #%d	 0x%x == 0x%x\n", size, BittrueAdd,DmaoAdd);
			}
	    }
		else
		{
			 BitTrueCount++;
			 if(BitTrueCount<16)
			 {
				printf("FAIL #%d	0x%x != 0x%x address 0x%x\n", size, BittrueAdd,DmaoAdd,(MUINT32*)buf_gepf_temp_ans_debug_blur_wr_1.virtAddr+size);
			 }
		}
	}
	printf("GEPF temp_blur_1 FAIL BitTrueCount #%d /(%d)\n", BitTrueCount,(sizeTotal/4));
	printf("--- [GEPF bittrue end] ----\n");
	// Bittrue comparison -----

    mpImemDrv->freeVirtBuf(&buf_gepf_depth_rtl);
    mpImemDrv->freeVirtBuf(&buf_gepf_new_table);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_blur_wr);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_wr);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_cur_y);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_y);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src_blur);
    mpImemDrv->freeVirtBuf(&buf_gepf_uv420_rtl);
    mpImemDrv->freeVirtBuf(&buf_gepf_y_rtl);

    mpImemDrv->freeVirtBuf(&buf_gepf_blur_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_yuv_base);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_base);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_base);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_wr_base);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_depth_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_blur_wr_base);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_Y);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_UV);

	mpImemDrv->freeVirtBuf(&buf_gepf_depth_rtl_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_new_table_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_blur_wr_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_ans_debug_wr_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_cur_y_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_y_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_temp_pre_src_blur_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_uv420_rtl_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_y_rtl_1);

    mpImemDrv->freeVirtBuf(&buf_gepf_blur_base_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_yuv_base_1);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_base_1);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_base_1);
    mpImemDrv->freeVirtBuf(&buf_temp_blur_wr_base_1);
    mpImemDrv->freeVirtBuf(&buf_temp_depth_wr_base_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_depth_wr_base_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_blur_wr_base_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_Y_1);
    mpImemDrv->freeVirtBuf(&buf_gepf_480_UV_1);
   
    pStream->uninit();   
    printf("--- [GepfStream uninit done\n");

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



