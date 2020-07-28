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


#define LOG_TAG "eafstream_test"

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
#include <mtkcam/drv/iopipe/PostProc/IEafStream.h>
//#include <drv/eafunittest.h>
#include <mtkcam/drv/iopipe/PostProc/EafUtility.h>

//#include <mtkcam/iopipe/PostProc/IFeatureStream.h>

//#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <drv/imem_drv.h>
//#include <mtkcam/drv/isp_drv.h>

#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
//
//#include "eaf/eaf_testcommon.h"

using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSEaf;

/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size);
/******************************************************************************
* EAF Test Case
*******************************************************************************/
//#include "frame_0_eaf_setting_00.h"

int eaf_phase1(int num);
int eaf_phase1_many(int num);
int eaf_phase4(int num);

MBOOL g_bEAFCallback;
int g_bEAFCallback_count;

MVOID EAFCallback(EAFParams& rParams);
 
int g_EafMultipleBuffer = 0;
int g_EafCount = 0;

pthread_t       EafUserThread;
sem_t           EafSem;
volatile bool   g_bEafThreadTerminated = 0;

/*******************************************************************************
*  Main Function 
*  
********************************************************************************/

int test_eafStream(int argc, char** argv)
{
    int ret = 0; 
    int testCase = 0;
	int testNum = 0;
    int i;
	
    testCase = atoi(argv[0]);
	testNum = atoi(argv[1]);

    switch(testCase)
    {
        case 0:  //OK
			printf(" Unit Test Case  phase 1 (frame mode):One EAF Request. \n");
            ret = eaf_phase1(1);
            break;
			
		case 1:  //OK
			printf(" Unit Test Case phase 1 (frame mode):One EAF Request + Reset + One EAF Request 	 \n");
			ret = eaf_phase1(1);
			ret = eaf_phase1(1);			
			break;
			
		case 2:  	//OK
			printf(" Unit Test Casephase 1 (frame mode):SEQUENTIAL TWO EAF Request. \n");
			ret = eaf_phase1(testNum);
			break;
			
		case 3: 	//OK
			printf(" Unit Test Casephase 1 (frame mode):MULTI (TWO)EAF PACKAGE IN ONE Request. \n");
			ret = eaf_phase1_many(testNum);
			break;
			
		case 4:  //OK (Fake Tile Calculation)	
			printf(" Unit Test Case phase 4 (tile mode):One EAF Request. \n");
			ret = eaf_phase4(1);
			break;	
			
		case 5: //On-Going (Fake Tile Calculation)
			printf(" Unit Test Case phase 4 (tile mode):SEQUENTIAL EAF Request. \n");
			ret = eaf_phase4(10);
			break;	
			
		case 6: //On-Going (Tile Calculation)
			printf(" Unit Test Case phase 4 (tile mode):One EAF Request. \n");
			ret = eaf_phase4(1);
			break;	
			
       	default:
            break;
    }
    
    ret = 1;
    return ret; 
}

MVOID EAFCallback(EAFParams& rParams)
{
    printf("--- [EAF callback func]\n");
    g_bEAFCallback = MTRUE;
	g_bEAFCallback_count ++;
}



/*********************************************************************************/
int eaf_phase1(int num)
{
	#include "eaf/phase1/eafi_cur_uv_a.h"
	#include "eaf/phase1/eafi_cur_y_a.h"
	#include "eaf/phase1/eafi_depth_a.h"
	#include "eaf/phase1/eafi_mask_a.h"
	#include "eaf/phase1/eafi_pre_uv_a.h"
	#include "eaf/phase1/eafi_pre_y_a.h"
	#include "eaf/phase1/eafi_prob_a.h"
	#include "eaf/phase1/eafo_fout_a.h"
	#include "eaf/phase1/eafo_pout_a.h"

    int ret=0;
	int i;

	printf("#########################################################\n");
	printf("########### EAF Phase1 Start to Test !!!!###########\n");
	printf("#########################################################\n");
    
    /*
    *  1. EafStream Instance
    */ 
    NSCam::NSIoPipe::NSEaf::IEafStream* pStream;
    pStream= NSCam::NSIoPipe::NSEaf::IEafStream::createInstance("test_eaf");
    pStream->init();   
    printf("--- [test_eaf...EafStream init done] ---\n");

    /*
    *  2. IMemDrv Instance
    */
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMA memory asignment
    */
    
	for (i =0; i<num; i++)
	{
	
		IMEM_BUF_INFO eafi_mask_buffer;		
	    eafi_mask_buffer.size = sizeof(eafi_mask_a);	
	    printf("eafi_mask_buffer.size:%d\n",eafi_mask_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_mask_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_mask_buffer);	
		memcpy( (MUINT8*)(eafi_mask_buffer.virtAddr), (MUINT8*)(eafi_mask_a), sizeof(eafi_mask_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_mask_buffer);


	    IMEM_BUF_INFO eafi_cur_y_buffer; 	
	    eafi_cur_y_buffer.size = sizeof(eafi_cur_y_a);	
	    printf("eafi_cur_y_buffer.size:%d\n",eafi_cur_y_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_cur_y_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_cur_y_buffer);	
		memcpy( (MUINT8*)(eafi_cur_y_buffer.virtAddr), (MUINT8*)(eafi_cur_y_a), sizeof(eafi_cur_y_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_cur_y_buffer);


	    IMEM_BUF_INFO eafi_cur_uv_buffer;    
	    eafi_cur_uv_buffer.size = sizeof(eafi_cur_uv_a);	
	    printf("eafi_cur_uv_buffer.size:%d\n",eafi_cur_uv_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_cur_uv_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_cur_uv_buffer);	
		memcpy( (MUINT8*)(eafi_cur_uv_buffer.virtAddr), (MUINT8*)(eafi_cur_uv_a), sizeof(eafi_cur_uv_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_cur_uv_buffer);


	    IMEM_BUF_INFO eafi_pre_y_buffer;
	    eafi_pre_y_buffer.size = sizeof(eafi_pre_y_a);	
	    printf("eafi_pre_y_buffer.size:%d\n",eafi_pre_y_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_pre_y_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_pre_y_buffer);	
		memcpy( (MUINT8*)(eafi_pre_y_buffer.virtAddr), (MUINT8*)(eafi_pre_y_a), sizeof(eafi_pre_y_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_pre_y_buffer);


	    IMEM_BUF_INFO eafi_pre_uv_buffer;    
	    eafi_pre_uv_buffer.size = sizeof(eafi_pre_uv_a);	
	    printf("eafi_pre_uv_buffer.size:%d\n",eafi_pre_uv_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_pre_uv_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_pre_uv_buffer);	
		memcpy( (MUINT8*)(eafi_pre_uv_buffer.virtAddr), (MUINT8*)(eafi_pre_uv_a), sizeof(eafi_pre_uv_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_pre_uv_buffer);


	    IMEM_BUF_INFO eafi_depth_buffer; 
	    eafi_depth_buffer.size = sizeof(eafi_depth_a);	
	    printf("eafi_depth_buffer.size:%d\n",eafi_depth_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_depth_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_depth_buffer);	
		memcpy( (MUINT8*)(eafi_depth_buffer.virtAddr), (MUINT8*)(eafi_depth_a), sizeof(eafi_depth_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_depth_buffer);


	    IMEM_BUF_INFO eafi_prob_buffer;   
	    eafi_prob_buffer.size = sizeof(eafi_prob_a);	
	    printf("eafi_prob_buffer.size:%d\n",eafi_prob_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_prob_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_prob_buffer);	
		memcpy( (MUINT8*)(eafi_prob_buffer.virtAddr), (MUINT8*)(eafi_prob_a), sizeof(eafi_prob_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_prob_buffer);


	    IMEM_BUF_INFO pBuf_fout_buffer;//EAFO_FOUT_BASE_ADDR
	    pBuf_fout_buffer.size = sizeof(eafo_fout_a);	
	    printf("pBuf_fout_buffer.size:%d\n",pBuf_fout_buffer.size);
	    mpImemDrv->allocVirtBuf(&pBuf_fout_buffer);
	    mpImemDrv->mapPhyAddr(&pBuf_fout_buffer);	
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &pBuf_fout_buffer);


	    IMEM_BUF_INFO pBuf_pout_buffer;//EAFO_FOUT_BASE_ADDR
	    pBuf_pout_buffer.size = sizeof(eafo_pout_a);	
	    printf("pBuf_pout_buffer.size:%d\n",pBuf_pout_buffer.size);
	    mpImemDrv->allocVirtBuf(&pBuf_pout_buffer);
	    mpImemDrv->mapPhyAddr(&pBuf_pout_buffer);	
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &pBuf_pout_buffer);
	 

		printf("--- [eaf_phase1] ---\n");

	    /*
	    *  5-1. EAF stream's request configuration: rEafParams
	    */
	    EAFParams rEafParams;
	    EAFConfig eafconfig;
	    rEafParams.mpfnCallback = EAFCallback;  

	// top control
		eafconfig.mode = 1;// Phase 1 : bilinear filter

		eafconfig.MAIN_CFG0.eaf_en = 1;
		eafconfig.MAIN_CFG0.jbfr_ref_sel = 1;
		eafconfig.MAIN_CFG0.eaf_hist_en = 1;
		eafconfig.MAIN_CFG0.eaf_jbfr_en = 1;
		eafconfig.MAIN_CFG0.eaf_prob_en = 1;
		eafconfig.MAIN_CFG0.eaf_hist_pk8_en = 1;	
		eafconfig.MAIN_CFG0.eraf_jbfr_uvch_en = 1;


		eafconfig.MAIN_CFG1.eafi_mask_en = 1;
		eafconfig.MAIN_CFG1.eafi_cur_y_en = 1;
		eafconfig.MAIN_CFG1.eafi_cur_uv_en = 1;
		eafconfig.MAIN_CFG1.eafi_pre_y_en = 1;
		eafconfig.MAIN_CFG1.eafi_pre_uv_en = 1;
		eafconfig.MAIN_CFG1.eafi_dep_en = 1;
		eafconfig.MAIN_CFG1.eafi_prob_en = 1;
		eafconfig.MAIN_CFG1.eafo_fout_en = 1;
		eafconfig.MAIN_CFG1.eafo_pout_en = 1;
		
		eafconfig.MAIN_CFG1.eafi_en = 1;
		eafconfig.MAIN_CFG1.eaf2i_en = 1;
		eafconfig.MAIN_CFG1.eaf3i_en = 1;
		eafconfig.MAIN_CFG1.eaf4i_en = 1;
		eafconfig.MAIN_CFG1.eaf5i_en = 1;
		eafconfig.MAIN_CFG1.eaf6i_en = 1;
		eafconfig.MAIN_CFG1.eafo_en = 1;

		eafconfig.EAF_HIST_CFG.ctrl = 0x1f006496;	
		eafconfig.EAF_HIST_CFG.w = 0x1e0;		
		eafconfig.EAF_HIST_CFG.h = 0x10e;	           

		eafconfig.EAF_SRZ_CFG.w = 0x1e0;
		eafconfig.EAF_SRZ_CFG.h = 0x10e;
		
		eafconfig.EAF_BOXF_CFG.w = 0x1e0;
		eafconfig.EAF_BOXF_CFG.h = 0x10e;	          
		
		eafconfig.EAF_DIV_CFG.w = 0x1e0;	
		eafconfig.EAF_DIV_CFG.h= 0x10e;
		
		eafconfig.EAF_LKHF_CFG.w = 0x1e0;	
		eafconfig.EAF_LKHF_CFG.h = 0x10e;
		eafconfig.EAF_LKHF_CFG.ctrl = 0x80808000;
		
		eafconfig.EAF_MASK_LB_CTL.ctrl = 0x70b1501;	           
		eafconfig.EAF_MASK_LB_CTL.w = 0x1e0;	
		eafconfig.EAF_MASK_LB_CTL.h = 0x10e;	
		
		eafconfig.EAF_PRE_Y_LB_CTL.ctrl = 0x70b1501;	           
		eafconfig.EAF_PRE_Y_LB_CTL.w = 0x1e0;	
		eafconfig.EAF_PRE_Y_LB_CTL.h = 0x10e;

		eafconfig.EAF_PRE_UV_LB_CTL.ctrl = 0x7060b03;	           
		eafconfig.EAF_PRE_UV_LB_CTL.w = 0x01e0;           
		eafconfig.EAF_PRE_UV_LB_CTL.h = 0x0087;           

		eafconfig.EAF_CUR_UV_LB_CTL.ctrl = 0x3;	           
		eafconfig.EAF_CUR_UV_LB_CTL.w = 0x01e0;           
		eafconfig.EAF_CUR_UV_LB_CTL.h = 0x0087; 

	//jbfr config
		eafconfig.jbfrcfg.cfg0 = 0x940780a;
		eafconfig.jbfrcfg.cfg1 = 0x6081b16;
		eafconfig.jbfrcfg.cfg2 = 0x353b3f3f;
		eafconfig.jbfrcfg.cfg3 = 0x181f272e;
		eafconfig.jbfrcfg.cfg4 = 0x90d12;
		eafconfig.jbfrcfg.cfg5 = 0x0;
		eafconfig.jbfrcfg.size = 0x10e01e0;
		eafconfig.jbfrcfg.cfg6 = 0x0;
		eafconfig.jbfrcfg.cfg7 = 0x0;
		eafconfig.jbfrcfg.cfg8 = 0x0;

	//buffer config
		eafconfig.EAFI_MASK.dmaport = EAF_EAFI_MASK;
		eafconfig.EAFI_MASK.u4BufVA = eafi_mask_buffer.virtAddr;
		eafconfig.EAFI_MASK.u4BufPA = eafi_mask_buffer.phyAddr;
		eafconfig.EAFI_MASK.u4BufSize = eafi_mask_buffer.size;
		eafconfig.EAFI_MASK.u4Stride = 0x1e0;
		eafconfig.EAFI_MASK.u4ImgWidth = 0x1df;
		eafconfig.EAFI_MASK.u4ImgHeight = 0x10d;

		eafconfig.EAFI_CUR_Y.dmaport = EAF_EAFI_CUR_Y;
		eafconfig.EAFI_CUR_Y.u4BufVA = eafi_cur_y_buffer.virtAddr;
		eafconfig.EAFI_CUR_Y.u4BufPA = eafi_cur_y_buffer.phyAddr;
		eafconfig.EAFI_CUR_Y.u4BufSize = eafi_cur_y_buffer.size;
		eafconfig.EAFI_CUR_Y.u4Stride = 0x1e0;	
		eafconfig.EAFI_CUR_Y.u4ImgWidth = 0x1df;
		eafconfig.EAFI_CUR_Y.u4ImgHeight = 0x10d;

		eafconfig.EAFI_CUR_UV.dmaport = EAF_EAFI_CUR_UV;
		eafconfig.EAFI_CUR_UV.u4BufVA = eafi_cur_uv_buffer.virtAddr;
		eafconfig.EAFI_CUR_UV.u4BufPA = eafi_cur_uv_buffer.phyAddr;
		eafconfig.EAFI_CUR_UV.u4BufSize = eafi_cur_uv_buffer.size;
		eafconfig.EAFI_CUR_UV.u4Stride = 0x1e0;	
		eafconfig.EAFI_CUR_UV.u4ImgWidth = 0x1df;
		eafconfig.EAFI_CUR_UV.u4ImgHeight = 0x86;

		eafconfig.EAFI_PRE_Y.dmaport = EAF_EAFI_PRE_Y;
		eafconfig.EAFI_PRE_Y.u4BufVA = eafi_pre_y_buffer.virtAddr;
		eafconfig.EAFI_PRE_Y.u4BufPA = eafi_pre_y_buffer.phyAddr;
		eafconfig.EAFI_PRE_Y.u4BufSize = eafi_pre_y_buffer.size;
		eafconfig.EAFI_PRE_Y.u4Stride = 0x1e0;
		eafconfig.EAFI_PRE_Y.u4ImgWidth = 0x1df;
		eafconfig.EAFI_PRE_Y.u4ImgHeight = 0x10d;

		eafconfig.EAFI_PRE_UV.dmaport = EAF_EAFI_PRE_UV;
		eafconfig.EAFI_PRE_UV.u4BufVA = eafi_pre_uv_buffer.virtAddr;
		eafconfig.EAFI_PRE_UV.u4BufPA = eafi_pre_uv_buffer.phyAddr;
		eafconfig.EAFI_PRE_UV.u4BufSize = eafi_pre_uv_buffer.size;
		eafconfig.EAFI_PRE_UV.u4Stride= 0x1e0;
		eafconfig.EAFI_PRE_UV.u4ImgWidth = 0x1df;
		eafconfig.EAFI_PRE_UV.u4ImgHeight = 0x86;

		eafconfig.EAFI_DEP.dmaport = EAF_EAFI_DEP;
		eafconfig.EAFI_DEP.u4BufVA = eafi_depth_buffer.virtAddr;
		eafconfig.EAFI_DEP.u4BufPA = eafi_depth_buffer.phyAddr;
		eafconfig.EAFI_DEP.u4BufSize = eafi_depth_buffer.size;
		eafconfig.EAFI_DEP.u4Stride= 0x1e0;
		eafconfig.EAFI_DEP.u4ImgWidth = 0x1df;
		eafconfig.EAFI_DEP.u4ImgHeight = 0x10d;


		eafconfig.EAFI_PROB.dmaport = EAF_EAFI_PROB;
		eafconfig.EAFI_PROB.u4BufVA = eafi_prob_buffer.virtAddr;
		eafconfig.EAFI_PROB.u4BufPA = eafi_prob_buffer.phyAddr;
		eafconfig.EAFI_PROB.u4BufSize = eafi_prob_buffer.size;
		eafconfig.EAFI_PROB.u4Stride= 0x2100;
		eafconfig.EAFI_PROB.u4ImgWidth = 0x20ff;
		eafconfig.EAFI_PROB.u4ImgHeight = 0x0;

		eafconfig.EAFO_FOUT.dmaport = EAF_EAFO_FOUT;
		eafconfig.EAFO_FOUT.u4BufVA = pBuf_fout_buffer.virtAddr;
		eafconfig.EAFO_FOUT.u4BufPA = pBuf_fout_buffer.phyAddr;
		eafconfig.EAFO_FOUT.u4BufSize = pBuf_fout_buffer.size;
		eafconfig.EAFO_FOUT.u4Stride = 0x1e0;
		eafconfig.EAFO_FOUT.u4ImgWidth = 0x1df;
		eafconfig.EAFO_FOUT.u4ImgHeight = 0x10d;

		eafconfig.EAFO_POUT.dmaport = EAF_EAFO_POUT;
		eafconfig.EAFO_POUT.u4BufVA = pBuf_pout_buffer.virtAddr;
		eafconfig.EAFO_POUT.u4BufPA = pBuf_pout_buffer.phyAddr;
		eafconfig.EAFO_POUT.u4BufSize = pBuf_pout_buffer.size;
		eafconfig.EAFO_POUT.u4Stride= 0x12100;
		eafconfig.EAFO_POUT.u4ImgWidth = 0x20ff;
		eafconfig.EAFO_POUT.u4ImgHeight = 0x0;
		
	    rEafParams.mEAFConfig.push_back(eafconfig);
	    g_bEAFCallback = MFALSE;


	    /*
	    *   5-2. request enqued to eafstream
	    */
	    //enque
	    ret=pStream->EAFenque(rEafParams);
		
	    if(!ret)
	        printf("--- ERRRRRRRRR [eaf enque fail] ---\n");
	    else
	        printf("--- [eaf enque done] ---\n]");

	    do{
	        usleep(100000);
	        if (MTRUE == g_bEAFCallback)
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
		sprintf(filename, "/data/EAF_default_process0x%x_mode%d_temp_depth_wr.bin",(MUINT32) getpid (), type);
		saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_temp_depth_wr_base.virtAddr), buf_temp_depth_wr_base.size);
		char filename2[256];
		sprintf(filename2, "/data/EAF_default_process0x%x_mode%d_temp_blur_wr.bin",(MUINT32) getpid (), type);
		saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_temp_blur_wr_base.virtAddr), buf_temp_blur_wr_base.size);
		printf("--- [EafStream save file done] ---\n");
		#endif

// Bittrue comparison +++++
		printf("--- [EAF bittrue start] ----\n");
		
	    MUINT32 output_data,golden_data;
	    int size,sizeTotal;
	    int err_cnt =0;

	//check fout data
	    IMEM_BUF_INFO pBuf_fout_buffer_golden; 
	    pBuf_fout_buffer_golden.size = sizeof(eafo_fout_a);	
	    mpImemDrv->allocVirtBuf(&pBuf_fout_buffer_golden);
	    mpImemDrv->mapPhyAddr(&pBuf_fout_buffer_golden);	
		memcpy((MUINT8*)(pBuf_fout_buffer_golden.virtAddr), (MUINT8*)(eafo_fout_a), sizeof(eafo_fout_a));

		sizeTotal = sizeof(eafo_fout_a);
		
		for(size=0;size<(sizeTotal/4);size++)
		{
			golden_data = *((MUINT32*)pBuf_fout_buffer_golden.virtAddr+size);
			output_data = *((MUINT32*)pBuf_fout_buffer.virtAddr+size);
			if(golden_data != output_data)
			{		
				printf("EAF fout Bit-ture fail!!index:0x%x,pBuf_fout:0x%x, golden_fout:0x%x \n",size,output_data,golden_data);
				err_cnt++;
				if (err_cnt == 100)
				{
					printf("EAF fout error exceed 100!!, index:%x break",size);
					break;
				}
		    }
		}

		if (err_cnt== 0)
			printf("EAF fout Bit-ture Pass !!\n");
		
	//check pout data
		IMEM_BUF_INFO pBuf_pout_buffer_golden; 
		pBuf_pout_buffer_golden.size = sizeof(eafo_pout_a); 
		mpImemDrv->allocVirtBuf(&pBuf_pout_buffer_golden);
		mpImemDrv->mapPhyAddr(&pBuf_pout_buffer_golden);	
		memcpy((MUINT8*)(pBuf_pout_buffer_golden.virtAddr), (MUINT8*)(eafo_pout_a), sizeof(eafo_pout_a));

		sizeTotal = sizeof(eafo_pout_a);
		
		for(size=0;size<(sizeTotal/4);size++)
		{
			golden_data = *((MUINT32*)pBuf_pout_buffer_golden.virtAddr+size);
			output_data = *((MUINT32*)pBuf_pout_buffer.virtAddr+size);
			if(golden_data != output_data)
			{		
				printf("EAF pout Bit-ture fail!!index:0x%x,pBuf_pout:0x%x, golden_pout:0x%x \n",size,output_data,golden_data);
				err_cnt++;
				if (err_cnt == 100)
				{
					printf("EAF pout error exceed 100!!, index:%x break",size);
					break;
				}
			}
		}
		if (err_cnt== 0)
			printf("EAF pout Bit-ture Pass",size);

		printf("--- [EAF bittrue end] ----\n");
		// Bittrue comparison -----

	    /*
	    *  7. cleanup the mass
	    */

	    mpImemDrv->freeVirtBuf(&eafi_mask_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_cur_y_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_cur_uv_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_pre_y_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_pre_uv_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_depth_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_prob_buffer);
	    mpImemDrv->freeVirtBuf(&pBuf_fout_buffer);
	    mpImemDrv->freeVirtBuf(&pBuf_pout_buffer);	
	    mpImemDrv->freeVirtBuf(&pBuf_fout_buffer_golden);
	    mpImemDrv->freeVirtBuf(&pBuf_pout_buffer_golden);

	}
    pStream->uninit();   
    printf("--- [EafStream uninit done] ---\n");

    mpImemDrv->uninit();
    printf("--- [Imem uninit done] ---\n");

    return ret;
}




/*********************************************************************************/
int eaf_phase1_many(int num)
{
	#include "eaf/phase1/eafi_cur_uv_a.h"
	#include "eaf/phase1/eafi_cur_y_a.h"
	#include "eaf/phase1/eafi_depth_a.h"
	#include "eaf/phase1/eafi_mask_a.h"
	#include "eaf/phase1/eafi_pre_uv_a.h"
	#include "eaf/phase1/eafi_pre_y_a.h"
	#include "eaf/phase1/eafi_prob_a.h"
	#include "eaf/phase1/eafo_fout_a.h"
	#include "eaf/phase1/eafo_pout_a.h"

    int ret=0;
	int i;
	g_bEAFCallback_count = 0;
    printf("--- [eaf_phase1] ---\n");
    
    /*
    *  1. EafStream Instance
    */ 
    NSCam::NSIoPipe::NSEaf::IEafStream* pStream;
    pStream= NSCam::NSIoPipe::NSEaf::IEafStream::createInstance("test_eaf");
    pStream->init();   
    printf("--- [test_eaf...EafStream init done] ---\n");

    /*
    *  2. IMemDrv Instance
    */
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMA memory asignment
    */
    	
		IMEM_BUF_INFO eafi_mask_buffer;		
	    eafi_mask_buffer.size = sizeof(eafi_mask_a);	
	    printf("eafi_mask_buffer.size:%d\n",eafi_mask_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_mask_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_mask_buffer);	
		memcpy( (MUINT8*)(eafi_mask_buffer.virtAddr), (MUINT8*)(eafi_mask_a), sizeof(eafi_mask_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_mask_buffer);


	    IMEM_BUF_INFO eafi_cur_y_buffer; 	
	    eafi_cur_y_buffer.size = sizeof(eafi_cur_y_a);	
	    printf("eafi_cur_y_buffer.size:%d\n",eafi_cur_y_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_cur_y_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_cur_y_buffer);	
		memcpy( (MUINT8*)(eafi_cur_y_buffer.virtAddr), (MUINT8*)(eafi_cur_y_a), sizeof(eafi_cur_y_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_cur_y_buffer);


	    IMEM_BUF_INFO eafi_cur_uv_buffer;    
	    eafi_cur_uv_buffer.size = sizeof(eafi_cur_uv_a);	
	    printf("eafi_cur_uv_buffer.size:%d\n",eafi_cur_uv_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_cur_uv_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_cur_uv_buffer);	
		memcpy( (MUINT8*)(eafi_cur_uv_buffer.virtAddr), (MUINT8*)(eafi_cur_uv_a), sizeof(eafi_cur_uv_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_cur_uv_buffer);


	    IMEM_BUF_INFO eafi_pre_y_buffer;
	    eafi_pre_y_buffer.size = sizeof(eafi_pre_y_a);	
	    printf("eafi_pre_y_buffer.size:%d\n",eafi_pre_y_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_pre_y_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_pre_y_buffer);	
		memcpy( (MUINT8*)(eafi_pre_y_buffer.virtAddr), (MUINT8*)(eafi_pre_y_a), sizeof(eafi_pre_y_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_pre_y_buffer);


	    IMEM_BUF_INFO eafi_pre_uv_buffer;    
	    eafi_pre_uv_buffer.size = sizeof(eafi_pre_uv_a);	
	    printf("eafi_pre_uv_buffer.size:%d\n",eafi_pre_uv_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_pre_uv_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_pre_uv_buffer);	
		memcpy( (MUINT8*)(eafi_pre_uv_buffer.virtAddr), (MUINT8*)(eafi_pre_uv_a), sizeof(eafi_pre_uv_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_pre_uv_buffer);


	    IMEM_BUF_INFO eafi_depth_buffer; 
	    eafi_depth_buffer.size = sizeof(eafi_depth_a);	
	    printf("eafi_depth_buffer.size:%d\n",eafi_depth_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_depth_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_depth_buffer);	
		memcpy( (MUINT8*)(eafi_depth_buffer.virtAddr), (MUINT8*)(eafi_depth_a), sizeof(eafi_depth_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_depth_buffer);


	    IMEM_BUF_INFO eafi_prob_buffer;   
	    eafi_prob_buffer.size = sizeof(eafi_prob_a);	
	    printf("eafi_prob_buffer.size:%d\n",eafi_prob_buffer.size);
	    mpImemDrv->allocVirtBuf(&eafi_prob_buffer);
	    mpImemDrv->mapPhyAddr(&eafi_prob_buffer);	
		memcpy( (MUINT8*)(eafi_prob_buffer.virtAddr), (MUINT8*)(eafi_prob_a), sizeof(eafi_prob_a));
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_prob_buffer);


	    IMEM_BUF_INFO pBuf_fout_buffer;//EAFO_FOUT_BASE_ADDR
	    pBuf_fout_buffer.size = sizeof(eafo_fout_a);	
	    printf("pBuf_fout_buffer.size:%d\n",pBuf_fout_buffer.size);
	    mpImemDrv->allocVirtBuf(&pBuf_fout_buffer);
	    mpImemDrv->mapPhyAddr(&pBuf_fout_buffer);	
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &pBuf_fout_buffer);


	    IMEM_BUF_INFO pBuf_pout_buffer;//EAFO_FOUT_BASE_ADDR
	    pBuf_pout_buffer.size = sizeof(eafo_pout_a);	
	    printf("pBuf_pout_buffer.size:%d\n",pBuf_pout_buffer.size);
	    mpImemDrv->allocVirtBuf(&pBuf_pout_buffer);
	    mpImemDrv->mapPhyAddr(&pBuf_pout_buffer);	
	    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &pBuf_pout_buffer);
	 
	    printf("#########################################################\n");
	    printf("########### EAF Phase1 Start to Test !!!!###########\n");
	    printf("#########################################################\n");

	    /*
	    *  5-1. EAF stream's request configuration: rEafParams
	    */
	    EAFParams rEafParams;
	    EAFConfig eafconfig;
	    rEafParams.mpfnCallback = EAFCallback;  

	// top control
		eafconfig.mode = 1;

		eafconfig.MAIN_CFG0.eaf_en = 1;
		eafconfig.MAIN_CFG0.jbfr_ref_sel = 1;
		eafconfig.MAIN_CFG0.eaf_hist_en = 1;
		eafconfig.MAIN_CFG0.eaf_jbfr_en = 1;
		eafconfig.MAIN_CFG0.eaf_prob_en = 1;
		eafconfig.MAIN_CFG0.eaf_hist_pk8_en = 1;	
		eafconfig.MAIN_CFG0.eraf_jbfr_uvch_en = 1;


		eafconfig.MAIN_CFG1.eafi_mask_en = 1;
		eafconfig.MAIN_CFG1.eafi_cur_y_en = 1;
		eafconfig.MAIN_CFG1.eafi_cur_uv_en = 1;
		eafconfig.MAIN_CFG1.eafi_pre_y_en = 1;
		eafconfig.MAIN_CFG1.eafi_pre_uv_en = 1;
		eafconfig.MAIN_CFG1.eafi_dep_en = 1;
		eafconfig.MAIN_CFG1.eafi_prob_en = 1;
		eafconfig.MAIN_CFG1.eafo_fout_en = 1;
		eafconfig.MAIN_CFG1.eafo_pout_en = 1;
		
		eafconfig.MAIN_CFG1.eafi_en = 1;
		eafconfig.MAIN_CFG1.eaf2i_en = 1;
		eafconfig.MAIN_CFG1.eaf3i_en = 1;
		eafconfig.MAIN_CFG1.eaf4i_en = 1;
		eafconfig.MAIN_CFG1.eaf5i_en = 1;
		eafconfig.MAIN_CFG1.eaf6i_en = 1;
		eafconfig.MAIN_CFG1.eafo_en = 1;

		eafconfig.EAF_HIST_CFG.ctrl = 0x1f006496;	
		eafconfig.EAF_HIST_CFG.w = 0x1e0;		
		eafconfig.EAF_HIST_CFG.h = 0x10e;	           

		eafconfig.EAF_SRZ_CFG.w = 0x1e0;
		eafconfig.EAF_SRZ_CFG.h = 0x10e;
		
		eafconfig.EAF_BOXF_CFG.w = 0x1e0;
		eafconfig.EAF_BOXF_CFG.h = 0x10e;	          
		
		eafconfig.EAF_DIV_CFG.w = 0x1e0;	
		eafconfig.EAF_DIV_CFG.h= 0x10e;
		
		eafconfig.EAF_LKHF_CFG.w = 0x1e0;	
		eafconfig.EAF_LKHF_CFG.h = 0x10e;
		eafconfig.EAF_LKHF_CFG.ctrl = 0x80808000;
		
		eafconfig.EAF_MASK_LB_CTL.ctrl = 0x70b1501;	           
		eafconfig.EAF_MASK_LB_CTL.w = 0x1e0;	
		eafconfig.EAF_MASK_LB_CTL.h = 0x10e;	
		
		eafconfig.EAF_PRE_Y_LB_CTL.ctrl = 0x70b1501;	           
		eafconfig.EAF_PRE_Y_LB_CTL.w = 0x1e0;	
		eafconfig.EAF_PRE_Y_LB_CTL.h = 0x10e;

		eafconfig.EAF_PRE_UV_LB_CTL.ctrl = 0x7060b03;	           
		eafconfig.EAF_PRE_UV_LB_CTL.w = 0x01e0;           
		eafconfig.EAF_PRE_UV_LB_CTL.h = 0x0087;           

		eafconfig.EAF_CUR_UV_LB_CTL.ctrl = 0x3;	           
		eafconfig.EAF_CUR_UV_LB_CTL.w = 0x01e0;           
		eafconfig.EAF_CUR_UV_LB_CTL.h = 0x0087; 

	//jbfr config
		eafconfig.jbfrcfg.cfg0 = 0x940780a;
		eafconfig.jbfrcfg.cfg1 = 0x6081b16;
		eafconfig.jbfrcfg.cfg2 = 0x353b3f3f;
		eafconfig.jbfrcfg.cfg3 = 0x181f272e;
		eafconfig.jbfrcfg.cfg4 = 0x90d12;
		eafconfig.jbfrcfg.cfg5 = 0x0;
		eafconfig.jbfrcfg.size = 0x10e01e0;
		eafconfig.jbfrcfg.cfg6 = 0x0;
		eafconfig.jbfrcfg.cfg7 = 0x0;
		eafconfig.jbfrcfg.cfg8 = 0x0;

	//srz6 config
#if 0
		eafconfig.srz6Cfg.ctrl= 0x13; 
		eafconfig.srz6Cfg.h_step = 0x4424; 
		eafconfig.srz6Cfg.v_step = 0x3c6e; 
		eafconfig.srz6Cfg.inout_size.in_h = 0x0080;
		eafconfig.srz6Cfg.inout_size.in_w = 0x0100;
		eafconfig.srz6Cfg.inout_size.out_h = 0x010e;
		eafconfig.srz6Cfg.inout_size.out_w = 0x01e0;
		eafconfig.srz6Cfg.crop.x = 0;
		eafconfig.srz6Cfg.crop.floatX = 0; 
		eafconfig.srz6Cfg.crop.y = 0;	
		eafconfig.srz6Cfg.crop.floatY = 0; 
#endif

	//buffer config
		eafconfig.EAFI_MASK.dmaport = EAF_EAFI_MASK;
		eafconfig.EAFI_MASK.u4BufVA = eafi_mask_buffer.virtAddr;
		eafconfig.EAFI_MASK.u4BufPA = eafi_mask_buffer.phyAddr;
		eafconfig.EAFI_MASK.u4BufSize = eafi_mask_buffer.size;
		eafconfig.EAFI_MASK.u4Stride = 0x1e0;
		eafconfig.EAFI_MASK.u4ImgWidth = 0x1df;
		eafconfig.EAFI_MASK.u4ImgHeight = 0x10d;

		eafconfig.EAFI_CUR_Y.dmaport = EAF_EAFI_CUR_Y;
		eafconfig.EAFI_CUR_Y.u4BufVA = eafi_cur_y_buffer.virtAddr;
		eafconfig.EAFI_CUR_Y.u4BufPA = eafi_cur_y_buffer.phyAddr;
		eafconfig.EAFI_CUR_Y.u4BufSize = eafi_cur_y_buffer.size;
		eafconfig.EAFI_CUR_Y.u4Stride = 0x1e0;	
		eafconfig.EAFI_CUR_Y.u4ImgWidth = 0x1df;
		eafconfig.EAFI_CUR_Y.u4ImgHeight = 0x10d;

		eafconfig.EAFI_CUR_UV.dmaport = EAF_EAFI_CUR_UV;
		eafconfig.EAFI_CUR_UV.u4BufVA = eafi_cur_uv_buffer.virtAddr;
		eafconfig.EAFI_CUR_UV.u4BufPA = eafi_cur_uv_buffer.phyAddr;
		eafconfig.EAFI_CUR_UV.u4BufSize = eafi_cur_uv_buffer.size;
		eafconfig.EAFI_CUR_UV.u4Stride = 0x1e0;	
		eafconfig.EAFI_CUR_UV.u4ImgWidth = 0x1df;
		eafconfig.EAFI_CUR_UV.u4ImgHeight = 0x86;

		eafconfig.EAFI_PRE_Y.dmaport = EAF_EAFI_PRE_Y;
		eafconfig.EAFI_PRE_Y.u4BufVA = eafi_pre_y_buffer.virtAddr;
		eafconfig.EAFI_PRE_Y.u4BufPA = eafi_pre_y_buffer.phyAddr;
		eafconfig.EAFI_PRE_Y.u4BufSize = eafi_pre_y_buffer.size;
		eafconfig.EAFI_PRE_Y.u4Stride = 0x1e0;
		eafconfig.EAFI_PRE_Y.u4ImgWidth = 0x1df;
		eafconfig.EAFI_PRE_Y.u4ImgHeight = 0x10d;

		eafconfig.EAFI_PRE_UV.dmaport = EAF_EAFI_PRE_UV;
		eafconfig.EAFI_PRE_UV.u4BufVA = eafi_pre_uv_buffer.virtAddr;
		eafconfig.EAFI_PRE_UV.u4BufPA = eafi_pre_uv_buffer.phyAddr;
		eafconfig.EAFI_PRE_UV.u4BufSize = eafi_pre_uv_buffer.size;
		eafconfig.EAFI_PRE_UV.u4Stride= 0x1e0;
		eafconfig.EAFI_PRE_UV.u4ImgWidth = 0x1df;
		eafconfig.EAFI_PRE_UV.u4ImgHeight = 0x86;

		eafconfig.EAFI_DEP.dmaport = EAF_EAFI_DEP;
		eafconfig.EAFI_DEP.u4BufVA = eafi_depth_buffer.virtAddr;
		eafconfig.EAFI_DEP.u4BufPA = eafi_depth_buffer.phyAddr;
		eafconfig.EAFI_DEP.u4BufSize = eafi_depth_buffer.size;
		eafconfig.EAFI_DEP.u4Stride= 0x1e0;
		eafconfig.EAFI_DEP.u4ImgWidth = 0x1df;
		eafconfig.EAFI_DEP.u4ImgHeight = 0x10d;


		eafconfig.EAFI_PROB.dmaport = EAF_EAFI_PROB;
		eafconfig.EAFI_PROB.u4BufVA = eafi_prob_buffer.virtAddr;
		eafconfig.EAFI_PROB.u4BufPA = eafi_prob_buffer.phyAddr;
		eafconfig.EAFI_PROB.u4BufSize = eafi_prob_buffer.size;
		eafconfig.EAFI_PROB.u4Stride= 0x2100;
		eafconfig.EAFI_PROB.u4ImgWidth = 0x20ff;
		eafconfig.EAFI_PROB.u4ImgHeight = 0x0;

		eafconfig.EAFO_FOUT.dmaport = EAF_EAFO_FOUT;
		eafconfig.EAFO_FOUT.u4BufVA = pBuf_fout_buffer.virtAddr;
		eafconfig.EAFO_FOUT.u4BufPA = pBuf_fout_buffer.phyAddr;
		eafconfig.EAFO_FOUT.u4BufSize = pBuf_fout_buffer.size;
		eafconfig.EAFO_FOUT.u4Stride = 0x1e0;
		eafconfig.EAFO_FOUT.u4ImgWidth = 0x1df;
		eafconfig.EAFO_FOUT.u4ImgHeight = 0x10d;

		eafconfig.EAFO_POUT.dmaport = EAF_EAFO_POUT;
		eafconfig.EAFO_POUT.u4BufVA = pBuf_pout_buffer.virtAddr;
		eafconfig.EAFO_POUT.u4BufPA = pBuf_pout_buffer.phyAddr;
		eafconfig.EAFO_POUT.u4BufSize = pBuf_pout_buffer.size;
		eafconfig.EAFO_POUT.u4Stride= 0x12100;
		eafconfig.EAFO_POUT.u4ImgWidth = 0x20ff;
		eafconfig.EAFO_POUT.u4ImgHeight = 0x0;

		for (i=0; i<num; i++)
		{
	    	rEafParams.mEAFConfig.push_back(eafconfig);
		}
		
	    g_bEAFCallback = MFALSE;


	    /*
	    *   5-2. request enqued to eafstream
	    */
	    //enque
	    ret=pStream->EAFenque(rEafParams);
		
	    if(!ret)
	        printf("--- ERRRRRRRRR [eaf enque fail] ---\n");
	    else
	        printf("--- [eaf enque done] ---\n]");
		

	    do{
	        usleep(100000);
	        if (MTRUE == g_bEAFCallback)
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
		sprintf(filename, "/data/EAF_default_process0x%x_mode%d_temp_depth_wr.bin",(MUINT32) getpid (), type);
		saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_temp_depth_wr_base.virtAddr), buf_temp_depth_wr_base.size);
		char filename2[256];
		sprintf(filename2, "/data/EAF_default_process0x%x_mode%d_temp_blur_wr.bin",(MUINT32) getpid (), type);
		saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_temp_blur_wr_base.virtAddr), buf_temp_blur_wr_base.size);
		printf("--- [EafStream save file done] ---\n");
		#endif

// Bittrue comparison +++++
		printf("--- [EAF bittrue start] ----\n");
		
	    MUINT32 output_data,golden_data;
	    int size,sizeTotal;
	    int err_cnt =0;

	//check fout data
	    IMEM_BUF_INFO pBuf_fout_buffer_golden; 
	    pBuf_fout_buffer_golden.size = sizeof(eafo_fout_a);	
	    mpImemDrv->allocVirtBuf(&pBuf_fout_buffer_golden);
	    mpImemDrv->mapPhyAddr(&pBuf_fout_buffer_golden);	
		memcpy((MUINT8*)(pBuf_fout_buffer_golden.virtAddr), (MUINT8*)(eafo_fout_a), sizeof(eafo_fout_a));

		sizeTotal = sizeof(eafo_fout_a);
		
		for(size=0;size<(sizeTotal/4);size++)
		{
			golden_data = *((MUINT32*)pBuf_fout_buffer_golden.virtAddr+size);
			output_data = *((MUINT32*)pBuf_fout_buffer.virtAddr+size);
			if(golden_data != output_data)
			{		
				printf("EAF fout Bit-ture fail!!index:0x%x,pBuf_fout:0x%x, golden_fout:0x%x \n",size,output_data,golden_data);
				err_cnt++;
				if (err_cnt == 100)
				{
					printf("EAF fout error exceed 100!!, index:%x break",size);
					break;
				}
		    }
		}

		if (err_cnt== 0)
			printf("EAF fout Bit-ture Pass !!\n");
		
	//check pout data
		IMEM_BUF_INFO pBuf_pout_buffer_golden; 
		pBuf_pout_buffer_golden.size = sizeof(eafo_pout_a); 
		mpImemDrv->allocVirtBuf(&pBuf_pout_buffer_golden);
		mpImemDrv->mapPhyAddr(&pBuf_pout_buffer_golden);	
		memcpy((MUINT8*)(pBuf_pout_buffer_golden.virtAddr), (MUINT8*)(eafo_pout_a), sizeof(eafo_pout_a));

		sizeTotal = sizeof(eafo_pout_a);
		
		for(size=0;size<(sizeTotal/4);size++)
		{
			golden_data = *((MUINT32*)pBuf_pout_buffer_golden.virtAddr+size);
			output_data = *((MUINT32*)pBuf_pout_buffer.virtAddr+size);
			if(golden_data != output_data)
			{		
				printf("EAF pout Bit-ture fail!!index:0x%x,pBuf_pout:0x%x, golden_pout:0x%x \n",size,output_data,golden_data);
				err_cnt++;
				if (err_cnt == 100)
				{
					printf("EAF pout error exceed 100!!, index:%x break",size);
					break;
				}
			}
		}
		if (err_cnt== 0)
			printf("EAF pout Bit-ture Pass",size);

		printf("--- [EAF bittrue end] ----\n");
		// Bittrue comparison -----

	    /*
	    *  7. cleanup the mass
	    */

	    mpImemDrv->freeVirtBuf(&eafi_mask_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_cur_y_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_cur_uv_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_pre_y_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_pre_uv_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_depth_buffer);
	    mpImemDrv->freeVirtBuf(&eafi_prob_buffer);
	    mpImemDrv->freeVirtBuf(&pBuf_fout_buffer);
	    mpImemDrv->freeVirtBuf(&pBuf_pout_buffer);	
	    mpImemDrv->freeVirtBuf(&pBuf_fout_buffer_golden);
	    mpImemDrv->freeVirtBuf(&pBuf_pout_buffer_golden);

    pStream->uninit();   
    printf("--- [EafStream uninit done] ---\n");

    mpImemDrv->uninit();
    printf("--- [Imem uninit done] ---\n");

    return ret;
}

/*********************************************************************************/
int eaf_phase4(int type)
{
	#include "eaf/phase4/eafi_cur_uv_d.h"
	#include "eaf/phase4/eafi_cur_y_d.h"
	#include "eaf/phase4/eafi_mask_d.h"
	#include "eaf/phase4/eafo_fout_d.h"
	#include "eaf/phase4/tile_conf_map_eaf.h"

    int ret=0;

	
    printf("#########################################################\n");
    printf("########### EAF Phase4 Start to Test !!!!###########\n");
    printf("#########################################################\n");
    /*
    *  1. EafStream Instance
    */ 
    NSCam::NSIoPipe::NSEaf::IEafStream* pStream;
    pStream= NSCam::NSIoPipe::NSEaf::IEafStream::createInstance("test_eaf");
    pStream->init();   
    printf("--- [test_eaf...EafStream init done] ---\n");

    /*
    *  2. IMemDrv Instance
    */
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    /*
    *  3. static DMA memory asignment
    */
	
	IMEM_BUF_INFO eafi_mask_buffer;		
    eafi_mask_buffer.size = sizeof(eafi_mask_d);	
    printf("eafi_mask_buffer.size:%d\n",eafi_mask_buffer.size);
    mpImemDrv->allocVirtBuf(&eafi_mask_buffer);
    mpImemDrv->mapPhyAddr(&eafi_mask_buffer);	
	memcpy( (MUINT8*)(eafi_mask_buffer.virtAddr), (MUINT8*)(eafi_mask_d), sizeof(eafi_mask_d));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_mask_buffer);


    IMEM_BUF_INFO eafi_cur_y_buffer; 	
    eafi_cur_y_buffer.size = sizeof(eafi_cur_y_d);	
    printf("eafi_cur_y_buffer.size:%d\n",eafi_cur_y_buffer.size);
    mpImemDrv->allocVirtBuf(&eafi_cur_y_buffer);
    mpImemDrv->mapPhyAddr(&eafi_cur_y_buffer);	
	memcpy( (MUINT8*)(eafi_cur_y_buffer.virtAddr), (MUINT8*)(eafi_cur_y_d), sizeof(eafi_cur_y_d));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_cur_y_buffer);


    IMEM_BUF_INFO eafi_cur_uv_buffer;    
    eafi_cur_uv_buffer.size = sizeof(eafi_cur_uv_d);	
    printf("eafi_cur_uv_buffer.size:%d\n",eafi_cur_uv_buffer.size);
    mpImemDrv->allocVirtBuf(&eafi_cur_uv_buffer);
    mpImemDrv->mapPhyAddr(&eafi_cur_uv_buffer);	
	memcpy( (MUINT8*)(eafi_cur_uv_buffer.virtAddr), (MUINT8*)(eafi_cur_uv_d), sizeof(eafi_cur_uv_d));
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &eafi_cur_uv_buffer);

    IMEM_BUF_INFO pBuf_fout_buffer;//EAFO_FOUT_BASE_ADDR
    pBuf_fout_buffer.size = sizeof(eafo_fout_d);	
    printf("pBuf_fout_buffer.size:%d\n",pBuf_fout_buffer.size);
    mpImemDrv->allocVirtBuf(&pBuf_fout_buffer);
    mpImemDrv->mapPhyAddr(&pBuf_fout_buffer);	
    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &pBuf_fout_buffer);


//    IMEM_BUF_INFO pBuf_pout_buffer;//EAFO_FOUT_BASE_ADDR
//    pBuf_pout_buffer.size = sizeof(eafo_pout_d);	
//    printf("pBuf_pout_buffer.size:%d\n",pBuf_pout_buffer.size);
//    mpImemDrv->allocVirtBuf(&pBuf_pout_buffer);
//    mpImemDrv->mapPhyAddr(&pBuf_pout_buffer);	
//    mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &pBuf_pout_buffer);
 
    printf("--- [eaf_phase4] ---\n");

    /*
    *  5-1. EAF stream's request configuration: rEafParams
    */
    EAFParams rEafParams;
    EAFConfig eafconfig;
    rEafParams.mpfnCallback = EAFCallback;  

// top control
	eafconfig.mode = 4;// phase 4 : SRZ + bilinear filter

	eafconfig.MAIN_CFG0.eaf_en = 1;
	eafconfig.MAIN_CFG0.jbfr_ref_sel = 0;	
	eafconfig.MAIN_CFG0.eaf_jbfr_en = 1;
	eafconfig.MAIN_CFG0.eaf_srz_en = 1;
	eafconfig.MAIN_CFG0.eaf_srz_pk8_en = 1;	
	eafconfig.MAIN_CFG0.eraf_jbfr_uvch_en = 1;

	eafconfig.MAIN_CFG1.eafi_en = 1;//bit16
	eafconfig.MAIN_CFG1.eaf2i_en = 1;//bit17
	eafconfig.MAIN_CFG1.eaf4i_en = 1;//bit19
	eafconfig.MAIN_CFG1.eafo_en = 1;//bit22
	eafconfig.MAIN_CFG1.eafi_mask_en = 1;
	eafconfig.MAIN_CFG1.eafi_cur_y_en = 1;
	eafconfig.MAIN_CFG1.eafi_cur_uv_en = 1;
	eafconfig.MAIN_CFG1.eafo_fout_en = 1;


	eafconfig.EAF_HIST_CFG.ctrl = 0x1f008080;	
	eafconfig.EAF_HIST_CFG.w = 0x0780;		
	eafconfig.EAF_HIST_CFG.h = 0x0438;	           

	eafconfig.EAF_SRZ_CFG.w = 0x0200;
	eafconfig.EAF_SRZ_CFG.h = 0x01c0;
	
	eafconfig.EAF_BOXF_CFG.w = 0x0780;
	eafconfig.EAF_BOXF_CFG.h = 0x0438;	          
	
	eafconfig.EAF_DIV_CFG.w = 0x0780;	
	eafconfig.EAF_DIV_CFG.h= 0x0438;
	
	eafconfig.EAF_LKHF_CFG.w = 0x0780;	
	eafconfig.EAF_LKHF_CFG.h = 0x0438;
	eafconfig.EAF_LKHF_CFG.ctrl = 0x80808000;
	
	eafconfig.EAF_MASK_LB_CTL.ctrl = 0x70b1531;	           
	eafconfig.EAF_MASK_LB_CTL.w = 0x0780;	
	eafconfig.EAF_MASK_LB_CTL.h = 0x0438;	
	
	eafconfig.EAF_PRE_Y_LB_CTL.ctrl = 0x70b1531;	           
	eafconfig.EAF_PRE_Y_LB_CTL.w = 0x0780;	
	eafconfig.EAF_PRE_Y_LB_CTL.h = 0x0438;

	eafconfig.EAF_PRE_UV_LB_CTL.ctrl = 0x7060b33;	           
	eafconfig.EAF_PRE_UV_LB_CTL.w = 0x0780;           
	eafconfig.EAF_PRE_UV_LB_CTL.h = 0x021b;           

	eafconfig.EAF_CUR_UV_LB_CTL.ctrl = 0x2;	           
	eafconfig.EAF_CUR_UV_LB_CTL.w = 0x0780;           
	eafconfig.EAF_CUR_UV_LB_CTL.h = 0x021b; 

//jbfr config
	eafconfig.jbfrcfg.cfg0 = 0x940780a;
	eafconfig.jbfrcfg.cfg1 = 0x6081b16;
	eafconfig.jbfrcfg.cfg2 = 0x353b3f3f;
	eafconfig.jbfrcfg.cfg3 = 0x181f272e;
	eafconfig.jbfrcfg.cfg4 = 0x90d12;
	eafconfig.jbfrcfg.cfg5 = 0x0;
	eafconfig.jbfrcfg.size = 0x4380780;
	eafconfig.jbfrcfg.cfg6 = 0x0;
	eafconfig.jbfrcfg.cfg7 = 0x0;
	eafconfig.jbfrcfg.cfg8 = 0x0;

//srz6 config
	eafconfig.srz6Cfg.ctrl = 0x13; 
	eafconfig.srz6Cfg.h_step = 0x2216; 
	eafconfig.srz6Cfg.v_step = 0x3507; 
	eafconfig.srz6Cfg.inout_size.in_h = 0x01c0;
	eafconfig.srz6Cfg.inout_size.in_w = 0x0200;
	eafconfig.srz6Cfg.inout_size.out_h = 0x0438;
	eafconfig.srz6Cfg.inout_size.out_w = 0x0780;
	eafconfig.srz6Cfg.crop.x = 0x0;
	eafconfig.srz6Cfg.crop.floatX = 0x0; 
	eafconfig.srz6Cfg.crop.y = 0x0;	
	eafconfig.srz6Cfg.crop.floatY = 0x0; 

//buffer config
	eafconfig.EAFI_MASK.dmaport = EAF_EAFI_MASK;
	eafconfig.EAFI_MASK.u4BufVA = eafi_mask_buffer.virtAddr;
	eafconfig.EAFI_MASK.u4BufPA = eafi_mask_buffer.phyAddr;
	eafconfig.EAFI_MASK.u4BufSize = eafi_mask_buffer.size;
	eafconfig.EAFI_MASK.u4Stride = 0x780;
	eafconfig.EAFI_MASK.u4ImgWidth = 0x01ff;
	eafconfig.EAFI_MASK.u4ImgHeight = 0x01bf;

	eafconfig.EAFI_CUR_Y.dmaport = EAF_EAFI_CUR_Y;
	eafconfig.EAFI_CUR_Y.u4BufVA = eafi_cur_y_buffer.virtAddr;
	eafconfig.EAFI_CUR_Y.u4BufPA = eafi_cur_y_buffer.phyAddr;
	eafconfig.EAFI_CUR_Y.u4BufSize = eafi_cur_y_buffer.size;
	eafconfig.EAFI_CUR_Y.u4Stride = 0x780;	
	eafconfig.EAFI_CUR_Y.u4ImgWidth = 0x077f;
	eafconfig.EAFI_CUR_Y.u4ImgHeight = 0x0437;

	eafconfig.EAFI_CUR_UV.dmaport = EAF_EAFI_CUR_UV;
	eafconfig.EAFI_CUR_UV.u4BufVA = eafi_cur_uv_buffer.virtAddr;
	eafconfig.EAFI_CUR_UV.u4BufPA = eafi_cur_uv_buffer.phyAddr;
	eafconfig.EAFI_CUR_UV.u4BufSize = eafi_cur_uv_buffer.size;
	eafconfig.EAFI_CUR_UV.u4Stride = 0x780;	
	eafconfig.EAFI_CUR_UV.u4ImgWidth = 0x077f;
	eafconfig.EAFI_CUR_UV.u4ImgHeight = 0x21b;

	eafconfig.EAFO_FOUT.dmaport = EAF_EAFO_FOUT;
	eafconfig.EAFO_FOUT.u4BufVA = pBuf_fout_buffer.virtAddr;
	eafconfig.EAFO_FOUT.u4BufPA = pBuf_fout_buffer.phyAddr;
	eafconfig.EAFO_FOUT.u4BufSize = pBuf_fout_buffer.size;
	eafconfig.EAFO_FOUT.u4Stride = 0x780;
	eafconfig.EAFO_FOUT.u4ImgWidth = 0x077f;
	eafconfig.EAFO_FOUT.u4ImgHeight =0x0437;


// fack tile calculation
	IMEM_BUF_INFO pTile_buffer;//TDRI_BASE_ADDR
	pTile_buffer.size = sizeof(tile_conf_map_eaf);	
	printf("pTile_buffer.size:%d\n",pTile_buffer.size);
	mpImemDrv->allocVirtBuf(&pTile_buffer);
	mpImemDrv->mapPhyAddr(&pTile_buffer);	
	memcpy( (MUINT8*)(pTile_buffer.virtAddr), (MUINT8*)(tile_conf_map_eaf), sizeof(tile_conf_map_eaf)); 
	mpImemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_FLUSH, &pTile_buffer);

	eafconfig.EAF_TDRI_BASE_ADDR = pTile_buffer.phyAddr;



    rEafParams.mEAFConfig.push_back(eafconfig);
    g_bEAFCallback = MFALSE;


    /*
    *   5-2. request enqued to eafstream
    */
    //enque
    ret=pStream->EAFenque(rEafParams);
	
    if(!ret)
        printf("--- ERRRRRRRRR [eaf enque fail] ---\n");
    else
        printf("--- [eaf enque done] ---\n]");

    do{
        usleep(100000);
        if (MTRUE == g_bEAFCallback)
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
	sprintf(filename, "/data/EAF_default_process0x%x_mode%d_temp_depth_wr.bin",(MUINT32) getpid (), type);
	saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_temp_depth_wr_base.virtAddr), buf_temp_depth_wr_base.size);
	char filename2[256];
	sprintf(filename2, "/data/EAF_default_process0x%x_mode%d_temp_blur_wr.bin",(MUINT32) getpid (), type);
	saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_temp_blur_wr_base.virtAddr), buf_temp_blur_wr_base.size);
	printf("--- [EafStream save file done] ---\n");
	#endif

	// Bittrue comparison +++++
	printf("--- [EAF bittrue start] ----\n");
	
    MUINT32 output_data,golden_data;
    int size,sizeTotal;
    int err_cnt =0;

//check fout data
    IMEM_BUF_INFO pBuf_fout_buffer_golden; 
    pBuf_fout_buffer_golden.size = sizeof(eafo_fout_d);	
    mpImemDrv->allocVirtBuf(&pBuf_fout_buffer_golden);
    mpImemDrv->mapPhyAddr(&pBuf_fout_buffer_golden);	
	memcpy((MUINT8*)(pBuf_fout_buffer_golden.virtAddr), (MUINT8*)(eafo_fout_d), sizeof(eafo_fout_d));

	sizeTotal = sizeof(eafo_fout_d);
	
	for(size=0;size<(sizeTotal/4);size++)
	{
		golden_data = *((MUINT32*)pBuf_fout_buffer_golden.virtAddr+size);
		output_data = *((MUINT32*)pBuf_fout_buffer.virtAddr+size);
		if(golden_data != output_data)
		{		
			printf("EAF fout Bit-ture fail!!index:0x%x,pBuf_fout:0x%x, golden_fout:0x%x \n",size,output_data,golden_data);
			err_cnt++;
			if (err_cnt == 100)
			{
				printf("EAF fout error exceed 100!!, index:%x break",size);
				break;
			}
	    }
	}

	printf("--- [EAF bittrue end] ----\n");
	// Bittrue comparison -----

    /*
    *  7. cleanup the mass
    */

    mpImemDrv->freeVirtBuf(&eafi_mask_buffer);
    mpImemDrv->freeVirtBuf(&eafi_cur_y_buffer);
    mpImemDrv->freeVirtBuf(&eafi_cur_uv_buffer);
    mpImemDrv->freeVirtBuf(&pBuf_fout_buffer);
    mpImemDrv->freeVirtBuf(&pBuf_fout_buffer_golden);
    mpImemDrv->freeVirtBuf(&pTile_buffer);
   
    pStream->uninit();   
    printf("--- [EafStream uninit done] ---\n");

    mpImemDrv->uninit();
    printf("--- [Imem uninit done] ---\n");

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



