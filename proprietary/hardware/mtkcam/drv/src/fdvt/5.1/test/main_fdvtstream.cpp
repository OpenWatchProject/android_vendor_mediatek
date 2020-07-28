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


#define LOG_TAG "fdvtstream_test"

#include <vector>

#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/def/common.h>


#include <semaphore.h>
#include <pthread.h>
#include <utils/threads.h>
#include <mtkcam/drv/iopipe/PostProc/IEgnStream.h>
//#include <IEgnStream.h>

//#include <mtkcam/iopipe/PostProc/IFeatureStream.h>

#include <imem_drv.h>
#include <time.h>

//#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/std/common.h>
//#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>

#include <mtkcam/drv/def/fdvtcommon.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "../hardware/fdvt_drv.h"
#include "test_config.h"


unsigned int *gFD_golden_y2r_config;
unsigned int *gFD_golden_rs_config;
unsigned int *gFD_golden_fd_config;

unsigned int *gFD_golden_result_19_va;
unsigned int *gFD_golden_result_39_va;
unsigned int *gFD_golden_result_65_va;
unsigned int *gATTR_golden_result_16_va;
unsigned int *gATTR_golden_result_17_va;
unsigned int *gPOSE_golden_result_16_va;
unsigned int *gPOSE_golden_result_17_va;
unsigned int gFD_result_19_size;
unsigned int gFD_result_39_size;
unsigned int gFD_result_65_size;
unsigned int gATTR_result_16_size;
unsigned int gATTR_result_17_size;
unsigned int gPOSE_result_16_size;
unsigned int gPOSE_result_17_size;



#include "golden/fd/golden_fmap/gold_01_01_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_01_02_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_01_03_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_01_04_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_01_05_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_01_06_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_01_07_fd_out_loop19_0.h"

#include "golden/fd/golden_fmap/gold_02_01_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_02_02_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_02_03_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_02_04_fd_out_loop19_0.h"
#include "golden/fd/golden_fmap/gold_02_05_fd_out_loop19_0.h"

#include "golden/fd/golden_fmap/gold_01_01_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_01_02_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_01_03_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_01_04_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_01_05_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_01_06_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_01_07_fd_out_loop39_0.h"

#include "golden/fd/golden_fmap/gold_02_01_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_02_02_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_02_03_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_02_04_fd_out_loop39_0.h"
#include "golden/fd/golden_fmap/gold_02_05_fd_out_loop39_0.h"

#include "golden/fd/golden_fmap/gold_01_01_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_01_02_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_01_03_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_01_04_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_01_05_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_01_06_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_01_07_fd_out_loop65_0.h"

#include "golden/fd/golden_fmap/gold_02_01_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_02_02_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_02_03_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_02_04_fd_out_loop65_0.h"
#include "golden/fd/golden_fmap/gold_02_05_fd_out_loop65_0.h"

#include "golden/fd/golden_fmap/gold_03_01_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_03_02_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_03_03_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_03_04_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_03_05_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_03_06_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_03_07_fd_out_loop16_0.h"

#include "golden/fd/golden_fmap/gold_04_01_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_04_02_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_04_03_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_04_04_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_04_05_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_04_06_fd_out_loop16_0.h"
#include "golden/fd/golden_fmap/gold_04_07_fd_out_loop16_0.h"

#include "golden/fd/golden_fmap/gold_03_01_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_03_02_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_03_03_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_03_04_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_03_05_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_03_06_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_03_07_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_04_01_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_04_02_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_04_03_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_04_04_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_04_05_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_04_06_fd_out_loop17_0.h"
#include "golden/fd/golden_fmap/gold_04_07_fd_out_loop17_0.h"

#include "golden/fd/golden_fmap/gold_01_01_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_01_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_01_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_02_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_02_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_02_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_03_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_03_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_03_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_04_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_04_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_04_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_05_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_05_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_05_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_06_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_06_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_06_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_07_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_07_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_01_07_rs_confi_frame01.h"

#include "golden/fd/golden_fmap/gold_02_01_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_01_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_01_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_02_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_02_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_02_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_03_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_03_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_03_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_04_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_04_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_04_rs_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_05_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_05_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_02_05_rs_confi_frame01.h"

#include "golden/fd/golden_fmap/gold_03_01_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_01_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_02_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_02_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_03_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_03_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_04_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_04_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_05_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_05_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_06_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_06_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_07_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_03_07_fd_confi_frame01.h"

#include "golden/fd/golden_fmap/gold_04_01_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_01_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_02_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_02_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_03_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_03_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_04_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_04_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_05_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_05_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_06_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_06_fd_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_07_yuv2rgb_confi_frame01.h"
#include "golden/fd/golden_fmap/gold_04_07_fd_confi_frame01.h"

#include "pic/fd_in1_0101_600x733_FMT_YUV_2P.h"
#include "pic/fd_in2_0101_600x733_FMT_YUV_2P.h"
#include "pic/fd_in1_0102_338x600_FMT_YVU_2P.h"
#include "pic/fd_in2_0102_338x600_FMT_YVU_2P.h"
#include "pic/fd_in1_0103_450x600_FMT_YUYV.h"
#include "pic/fd_in1_0104_600x491_FMT_YVYU.h"
#include "pic/fd_in1_0105_600x1067_FMT_UYVY.h"
#include "pic/fd_in1_0106_600x600_FMT_VYUY.h"
#include "pic/fd_in1_0107_800x600_FMT_MONO.h"

#include "pic/fd_in1_0201_300x225_FMT_YUV_2P.h"
#include "pic/fd_in2_0201_300x225_FMT_YUV_2P.h"
#include "pic/fd_in1_0202_300x225_FMT_YUV_2P.h"
#include "pic/fd_in2_0202_300x225_FMT_YUV_2P.h"
#include "pic/fd_in1_0203_300x225_FMT_YUV_2P.h"
#include "pic/fd_in2_0203_300x225_FMT_YUV_2P.h"
#include "pic/fd_in1_0204_300x225_FMT_YUV_2P.h"
#include "pic/fd_in2_0204_300x225_FMT_YUV_2P.h"
#include "pic/fd_in1_0205_300x225_FMT_YUV_2P.h"
#include "pic/fd_in2_0205_300x225_FMT_YUV_2P.h"

#include "pic/attr_in1_0301_596x596_FMT_YUV_2P.h"
#include "pic/attr_in2_0301_596x596_FMT_YUV_2P.h"
#include "pic/attr_in1_0302_298x298_FMT_YVU_2P.h"
#include "pic/attr_in2_0302_298x298_FMT_YVU_2P.h"
#include "pic/attr_in1_0303_484x484_FMT_YUYV.h"
#include "pic/attr_in1_0304_338x338_FMT_YVYU.h"
#include "pic/attr_in1_0305_92x92_FMT_UYVY.h"
#include "pic/attr_in1_0306_108x108_FMT_VYUY.h"
#include "pic/attr_in1_0307_84x84_FMT_MONO.h"

#include "pic/pose_in1_0401_500x500_FMT_YUV_2P.h"
#include "pic/pose_in2_0401_500x500_FMT_YUV_2P.h"
#include "pic/pose_in1_0402_346x346_FMT_YVU_2P.h"
#include "pic/pose_in2_0402_346x346_FMT_YVU_2P.h"
#include "pic/pose_in1_0403_348x348_FMT_YUYV.h"
#include "pic/pose_in1_0404_354x354_FMT_YVYU.h"
#include "pic/pose_in1_0405_306x306_FMT_UYVY.h"
#include "pic/pose_in1_0406_344x344_FMT_VYUY.h"
#include "pic/pose_in1_0407_344x344_FMT_MONO.h"


#include "../stream/log.h"

#define ABS(a) ((a < 0) ? -a : a )
#define IMAGE_SRC_FROM_PHONE 0
#define IMAGE_ONE_PLANE      0
DipIMemDrv *ImemDrv = NULL;
IMEM_BUF_INFO Imem_Buffer_Y;
IMEM_BUF_INFO Imem_Buffer_UV;
IMEM_BUF_INFO Imem_Buffer_Info;
MUINT8 *Imem_pLogVir_Y=NULL, *Imem_pLogVir_UV=NULL;
MINT32 Imem_MemID_Y, Imem_MemID_UV;
MUINT32 Imem_Size_Y, Imem_Size_UV;
MUINT32 Imem_Alloc_Num = 0;

#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        BASE_LOG_VRB(fmt, ##arg);
#define LOG_DBG(fmt, arg...)        do { if (g_isFDVTLogEnable) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        BASE_LOG_INF(fmt, ##arg);
#define LOG_WRN(fmt, arg...)        BASE_LOG_WRN(fmt, ##arg);
#define LOG_ERR(fmt, arg...)        BASE_LOG_ERR(fmt, ##arg);
#define LOG_AST(cond, fmt, arg...)  BASE_LOG_AST(cond, fmt, ##arg);


using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSEgn;

MBOOL g_bFDVTCallback;

MVOID FDVTCallback(EGNParams<FDVTConfig>& rParams);

pthread_t       FdvtUserThread;
sem_t           FdvtSem;
volatile bool            g_bFdvtThreadTerminated = 0;


MINT32 Imem_alloc(MUINT32 size,MINT32 *memId,MUINTPTR *vAddr,MUINTPTR *pAddr)
{
    if ( NULL == ImemDrv ) {
        ImemDrv = DipIMemDrv::createInstance();
        ImemDrv->init();
    }
    //
    Imem_Buffer_Info.size = size;
    //Imem_Buffer_Info.useNoncache = 1;
    if (ImemDrv->allocVirtBuf(&Imem_Buffer_Info) != 0)
        LOG_INF("Imem Allocate Virtual Buffer Fail!\n");

    *memId = Imem_Buffer_Info.memID;
    *vAddr = (MUINTPTR)Imem_Buffer_Info.virtAddr;
    //
    if (ImemDrv->mapPhyAddr(&Imem_Buffer_Info) != 0)
        LOG_INF("Imem Map Physical Address Fail!\n");

    *pAddr = (MUINTPTR)Imem_Buffer_Info.phyAddr;
    Imem_Alloc_Num ++;

    LOG_INF("Imem_Alloc_Num(%d)\n",Imem_Alloc_Num);
    LOG_INF("vAddr(0x%lx) pAddr(0x%lx) Imem_Alloc_Num(%d)\n",(unsigned long)*vAddr,(unsigned long)*pAddr,Imem_Alloc_Num);

    return 0;
}


MINT32 Imem_free(MUINT8 *vAddr, MUINTPTR phyAddr,MUINT32 size,MINT32 memId)
{
    Imem_Buffer_Info.size = size;
    Imem_Buffer_Info.memID = memId;
    Imem_Buffer_Info.virtAddr = (MUINTPTR)vAddr;
    Imem_Buffer_Info.phyAddr = (MUINTPTR)phyAddr;
    //

    if(ImemDrv)
    {
        if (ImemDrv->unmapPhyAddr(&Imem_Buffer_Info) != 0)
            LOG_INF("Imem Unmap Physical Address Fail!\n");
        //
        if (ImemDrv->freeVirtBuf(&Imem_Buffer_Info) != 0)
            LOG_INF("Imem Free Virtual Buffer Fail!\n");

        if(Imem_Alloc_Num)
        {
            Imem_Alloc_Num--;
            if(Imem_Alloc_Num==0)
            {
                ImemDrv->uninit();
                ImemDrv = NULL;
            }
        }
    }
    else
    {
        LOG_INF("Warning! unmapPhyAddr Fail!");
    }

    LOG_INF("Imem_Alloc_Num(%d)\n",Imem_Alloc_Num);

    return 0;
}


unsigned int FDVT_BitTrueCheck(unsigned int *hw, unsigned int *golden, unsigned int size)
{
    unsigned int i;
    unsigned int errCount = 0;
    unsigned int timesToBreak = 1000;
    unsigned int comparetimes = size / 4;

    //LOG_INF("Total Byte Size: %d", size);
    for (i = 0;i < comparetimes; i++)
    {
        //LOG_INF("Bit-true(%d of %d), OFFSET (0x%x) : HW (0x%08x) : GOLD (0x%08x)", i, comparetimes, i*sizeof(int), hw[i], golden[i]);
        if (hw[i] != golden[i])
        {
            errCount ++;
            if (errCount < timesToBreak)
            {
                LOG_INF("Bit-true(%d of %d) Fail!, OFFSET (0x%04x) : HW (0x%08x) : GOLD (0x%08x)", i, comparetimes, i*sizeof(int), hw[i], golden[i]);
            }
            //return false;
        }
    }
    if (errCount == 0)
        LOG_INF("Bit-true PASS\n");

    return errCount;
}


unsigned int FDVT_BitTrueCheck_withDummy(unsigned int *hw, unsigned int *golden, unsigned int totalSize, unsigned int validSize, unsigned int dummySize)
{
    unsigned int i = 0, j = 0, groupIndex = 0;
    unsigned int errCount = 0;
    unsigned int timesToBreak = 1000;
    unsigned int groupNumbers = totalSize / (validSize + dummySize);
    unsigned int comparetimes = validSize / sizeof(int);
    unsigned int jumpDistance_of_int = (validSize + dummySize) / sizeof(int);

    //LOG_MSG("Total Byte Size: %d, Valid Byte Size: %d, Dummy Byte Size: %d, Group Numbers: %d", totalSize, validSize, dummySize, groupNumbers);
    //LOG_MSG("Total Int Size: %d, Valid Int Size: %d, Dummy Int Size: %d", totalSize/4, validSize/4, dummySize/4);
    //LOG_MSG("Compare Times per Group: %d, Jump Distance of int: %d", comparetimes, jumpDistance_of_int);
    for (groupIndex = 0; groupIndex < groupNumbers; groupIndex ++)
    {
        //LOG_MSG("GroupIndex = %d, comparetimes = %d, jumpDistance_of_int = %d", groupIndex, comparetimes, jumpDistance_of_int);
        for (j = 0;j < comparetimes; j++)
        {
            //LOG_MSG("Bit-true(%d of %d), OFFSET (0x%x) : HW (0x%08x) : GOLD (0x%08x)", i+j, totalSize/4, (i+j)*sizeof(int), hw[i+j], golden[i+j]);
            if (hw[i+j] != golden[i+j])
            {
                errCount ++;
                if (errCount < timesToBreak)
                {
                    LOG_INF("valid size = %d, dummy size = %d, groupIndex = %d, groupNumbers = %d, i = %d, j = %d", validSize, dummySize, groupIndex, groupNumbers, i, j);
                    LOG_INF("Bit-true(%d of %d) Fail!, OFFSET (0x%x) : HW (0x%08x) : GOLD (0x%08x)", i+j, totalSize/4, (i+j)*sizeof(int), hw[i+j], golden[i+j]);
                }
                //return false;
            }
        }
        //LOG_MSG("jump distance %d from %d", jumpDistance_of_int, i);
        i = i + jumpDistance_of_int;
    }
    if (errCount == 0)
        LOG_INF("Bit-true PASS\n");
    return errCount;
}


void Allocate_TestPattern_Buffer(int subcase_index, int para1)
{
    if (subcase_index == 0)
    {
        switch (para1) {
            case 0: //0101
                Imem_Size_Y = sizeof(fd_in1_0101_600x733_FMT_YUV_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(fd_in2_0101_600x733_FMT_YUV_2P); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0101_600x733_FMT_YUV_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(fd_in2_0101_600x733_FMT_YUV_2P), Imem_Size_UV);
                break;
            case 1: //0102
                Imem_Size_Y = sizeof(fd_in1_0102_338x600_FMT_YVU_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(fd_in2_0102_338x600_FMT_YVU_2P); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0102_338x600_FMT_YVU_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(fd_in2_0102_338x600_FMT_YVU_2P), Imem_Size_UV);
                break;
            case 2: //0103
                Imem_Size_Y = sizeof(fd_in1_0103_450x600_FMT_YUYV); // 16-byte alignment
                Imem_Size_UV = 0; // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0103_450x600_FMT_YUYV), Imem_Size_Y);
                break;
            case 3: //0104
                Imem_Size_Y = sizeof(fd_in1_0104_600x491_FMT_YVYU); // 16-byte alignment
                Imem_Size_UV = 0; // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0104_600x491_FMT_YVYU), Imem_Size_Y);
                break;
            case 4: //0105
                Imem_Size_Y = sizeof(fd_in1_0105_600x1067_FMT_UYVY); // 16-byte alignment
                Imem_Size_UV = 0; // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0105_600x1067_FMT_UYVY), Imem_Size_Y);
                break;
            case 5: //0106
                Imem_Size_Y = sizeof(fd_in1_0106_600x600_FMT_VYUY); // 16-byte alignment
                Imem_Size_UV = 0; // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0106_600x600_FMT_VYUY), Imem_Size_Y);
                break;
            case 6: //0107
                Imem_Size_Y = sizeof(fd_in1_0107_800x600_FMT_MONO); // 16-byte alignment
                Imem_Size_UV = 0; // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0107_800x600_FMT_MONO), Imem_Size_Y);
                break;
            case 7: //0201
                Imem_Size_Y = sizeof(fd_in1_0201_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(fd_in2_0201_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0201_300x225_FMT_YUV_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(fd_in2_0201_300x225_FMT_YUV_2P), Imem_Size_UV);
                //Imem_Buffer_Y.useNoncache = 1;
                //Imem_Buffer_UV.useNoncache = 1;
                break;
            case 8: //0202
                Imem_Size_Y = sizeof(fd_in1_0202_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(fd_in2_0202_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0202_300x225_FMT_YUV_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(fd_in2_0202_300x225_FMT_YUV_2P), Imem_Size_UV);
                //Imem_Buffer_Y.useNoncache = 1;
                //Imem_Buffer_UV.useNoncache = 1;
                break;
            case 9: //0203
                Imem_Size_Y = sizeof(fd_in1_0203_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(fd_in2_0203_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0203_300x225_FMT_YUV_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(fd_in2_0203_300x225_FMT_YUV_2P), Imem_Size_UV);
                //Imem_Buffer_Y.useNoncache = 1;
                //Imem_Buffer_UV.useNoncache = 1;
                break;
            case 10: //0204
                Imem_Size_Y = sizeof(fd_in1_0204_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(fd_in2_0204_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0204_300x225_FMT_YUV_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(fd_in2_0204_300x225_FMT_YUV_2P), Imem_Size_UV);
                //Imem_Buffer_Y.useNoncache = 1;
                //Imem_Buffer_UV.useNoncache = 1;
                break;
            case 11: //0205
                Imem_Size_Y = sizeof(fd_in1_0205_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(fd_in2_0205_300x225_FMT_YUV_2P); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(fd_in1_0205_300x225_FMT_YUV_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(fd_in2_0205_300x225_FMT_YUV_2P), Imem_Size_UV);
                //Imem_Buffer_Y.useNoncache = 1;
                //Imem_Buffer_UV.useNoncache = 1;
                break;
            default:
                break;
            }
    }
    else if (subcase_index == 1)
    {
        switch (para1) {
            case 0:
                Imem_Size_Y = sizeof(attr_in1_0301_596x596_FMT_YUV_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(attr_in2_0301_596x596_FMT_YUV_2P); // dummy
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(attr_in1_0301_596x596_FMT_YUV_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(attr_in2_0301_596x596_FMT_YUV_2P), Imem_Size_UV); // dummy
                break;
            case 1:
                Imem_Size_Y = sizeof(attr_in1_0302_298x298_FMT_YVU_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(attr_in2_0302_298x298_FMT_YVU_2P); // dummy
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(attr_in1_0302_298x298_FMT_YVU_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(attr_in2_0302_298x298_FMT_YVU_2P), Imem_Size_UV); // dummy
                break;
            case 2:
                Imem_Size_Y = sizeof(attr_in1_0303_484x484_FMT_YUYV); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(attr_in1_0303_484x484_FMT_YUYV), Imem_Size_Y);
                break;
            case 3:
                Imem_Size_Y = sizeof(attr_in1_0304_338x338_FMT_YVYU); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(attr_in1_0304_338x338_FMT_YVYU), Imem_Size_Y);
                break;
            case 4:
                Imem_Size_Y = sizeof(attr_in1_0305_92x92_FMT_UYVY); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(attr_in1_0305_92x92_FMT_UYVY), Imem_Size_Y);
                break;
            case 5:
                Imem_Size_Y = sizeof(attr_in1_0306_108x108_FMT_VYUY); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(attr_in1_0306_108x108_FMT_VYUY), Imem_Size_Y);
                break;
            case 6:
                Imem_Size_Y = sizeof(attr_in1_0307_84x84_FMT_MONO); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(attr_in1_0307_84x84_FMT_MONO), Imem_Size_Y);
                break;
            default:
                break;
        }

    }
    else if (subcase_index == 2)
    {

        switch (para1) {
            case 0:
                Imem_Size_Y = sizeof(pose_in1_0401_500x500_FMT_YUV_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(pose_in2_0401_500x500_FMT_YUV_2P); // dummy
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(pose_in1_0401_500x500_FMT_YUV_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(pose_in2_0401_500x500_FMT_YUV_2P), Imem_Size_UV); // dummy
                break;
            case 1:
                Imem_Size_Y = sizeof(pose_in1_0402_346x346_FMT_YVU_2P); // 16-byte alignment
                Imem_Size_UV = sizeof(pose_in2_0402_346x346_FMT_YVU_2P); // dummy
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_alloc(Imem_Size_UV, &Imem_MemID_UV, &Imem_Buffer_UV.virtAddr, &Imem_Buffer_UV.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_UV.memID=Imem_MemID_UV;
                Imem_Buffer_Y.size=Imem_Size_Y;
                Imem_Buffer_UV.size=Imem_Size_UV;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(pose_in1_0402_346x346_FMT_YVU_2P), Imem_Size_Y);
                memcpy( (MUINT8*)(Imem_Buffer_UV.virtAddr), (MUINT8*)(pose_in2_0402_346x346_FMT_YVU_2P), Imem_Size_UV); // dummy
                break;
            case 2:
                Imem_Size_Y = sizeof(pose_in1_0403_348x348_FMT_YUYV); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(pose_in1_0403_348x348_FMT_YUYV), Imem_Size_Y);
                break;
            case 3:
                Imem_Size_Y = sizeof(pose_in1_0404_354x354_FMT_YVYU); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(pose_in1_0404_354x354_FMT_YVYU), Imem_Size_Y);
                break;
            case 4:
                Imem_Size_Y = sizeof(pose_in1_0405_306x306_FMT_UYVY); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(pose_in1_0405_306x306_FMT_UYVY), Imem_Size_Y);
                break;
            case 5:
                Imem_Size_Y = sizeof(pose_in1_0406_344x344_FMT_VYUY); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(pose_in1_0406_344x344_FMT_VYUY), Imem_Size_Y);
                break;
            case 6:
                Imem_Size_Y = sizeof(pose_in1_0407_344x344_FMT_MONO); // 16-byte alignment
                Imem_alloc(Imem_Size_Y, &Imem_MemID_Y, &Imem_Buffer_Y.virtAddr, &Imem_Buffer_Y.phyAddr);
                Imem_Buffer_Y.memID=Imem_MemID_Y;
                Imem_Buffer_Y.size=Imem_Size_Y;
                memcpy( (MUINT8*)(Imem_Buffer_Y.virtAddr), (MUINT8*)(pose_in1_0407_344x344_FMT_MONO), Imem_Size_Y);
                break;
            default:
                break;

        }

    }
    LOG_INF("Imem_Buffer_Y PA/VA: 0x%lx/0x%lx\n", (unsigned long)Imem_Buffer_Y.phyAddr, (unsigned long)Imem_Buffer_Y.virtAddr);
}

void Free_TestPattern_Buffer()
{
    Imem_free((MUINT8 *)Imem_Buffer_Y.virtAddr, Imem_Buffer_Y.phyAddr, Imem_Buffer_Y.size, Imem_Buffer_Y.memID);
    Imem_free((MUINT8 *)Imem_Buffer_UV.virtAddr, Imem_Buffer_UV.phyAddr, Imem_Buffer_UV.size, Imem_Buffer_UV.memID);
}

static bool saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        LOG_INF(": failed to create file [%s]: %s \n", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            LOG_INF(": failed to write to file [%s]: %s\n", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}

static bool appendBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_APPEND , S_IRWXU);
    if (fd < 0) {
        LOG_INF(": failed to create file [%s]: %s \n", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            LOG_INF(": failed to write to file [%s]: %s\n", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}

int test_fdvtStream(int subcase_index,int para1)
{
#if 0
    LOG_INF("getchar()");
    getchar();
#endif
    clock_t start_tick, end_tick;
    double elapsed;

    INITParams<FDVTConfig> rInitParams;
    rInitParams.srcImg_maxWidth = 1200;
    rInitParams.srcImg_maxHeight = 1200;
    if(subcase_index == 0) // FDMODE
    {
        switch (para1) {
            case 0:
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 65472;
                break;
            case 1:
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 65472;
                break;
            case 2:
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 65472;
                break;
            case 3:
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 65472;
                break;
            case 4:
                rInitParams.pyramid_width = 640;
                rInitParams.feature_threshold = 65472;
                break;
            case 5:
                rInitParams.pyramid_width = 640;
                rInitParams.feature_threshold = 65472;
                break;
            case 6:
                rInitParams.pyramid_width = 640;
                rInitParams.feature_threshold = 65472;
                break;
            case 7: //0201
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 300;
                break;
            case 8:
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 100;
                break;
            case 9:
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 0;
                break;
            case 10:
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 65236;
                break;
            case 11:
                rInitParams.pyramid_width = 400;
                rInitParams.feature_threshold = 65113;
                break;
            default:
                break;

        }
    }
    else if (subcase_index == 1) {
        rInitParams.pyramid_width = 400;
        rInitParams.feature_threshold = 65472;

    }
    else if (subcase_index == 2) {
        rInitParams.pyramid_width = 400;
        rInitParams.feature_threshold = 65472;

    }


    NSCam::NSIoPipe::NSEgn::IEgnStream<FDVTConfig>* pStream;
    pStream= NSCam::NSIoPipe::NSEgn::IEgnStream<FDVTConfig>::createInstance("FDVT unit test");

    pStream->init(rInitParams);

    EGNParams<FDVTConfig> rFdvtParams;
    rFdvtParams.mpEngineID = eFDVT;
    rFdvtParams.mpfnCallback = FDVTCallback;

    NSCam::NSIoPipe::FDVTConfig fdvtconfig;

    LOG_INF("=====> start of test_fd (subcase_index: %d  para1: %d)\n", subcase_index, para1);

    int ret = 0;
    if(subcase_index == 0) // FDMODE
    {
        fdvtconfig.FD_MODE = FDMODE;
        switch (para1) {
            case 0: //0101
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 600;
                fdvtconfig.SRC_IMG_HEIGHT = 733;
                fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
                fdvtconfig.SRC_IMG_STRIDE = 608;
                gFD_golden_result_19_va = (unsigned int *)&gold_01_01_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_01_01_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_01_01_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_01_01_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_01_01_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_01_01_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_01_01_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_01_01_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_01_01_fd_out_loop65_0);
                break;
            case 1: //0102
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_90;
                fdvtconfig.SRC_IMG_WIDTH = 338;
                fdvtconfig.SRC_IMG_HEIGHT = 600;
                fdvtconfig.SRC_IMG_FMT = FMT_YVU_2P;
                fdvtconfig.SRC_IMG_STRIDE = 352;
                gFD_golden_result_19_va = (unsigned int *)&gold_01_02_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_01_02_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_01_02_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_01_02_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_01_02_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_01_02_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_01_02_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_01_02_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_01_02_fd_out_loop65_0);

                break;
            case 2: //0103
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_270;
                fdvtconfig.SRC_IMG_WIDTH = 450;
                fdvtconfig.SRC_IMG_HEIGHT = 600;
                fdvtconfig.SRC_IMG_FMT = FMT_YUYV;
                fdvtconfig.SRC_IMG_STRIDE = 912;
                gFD_golden_result_19_va = (unsigned int *)&gold_01_03_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_01_03_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_01_03_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_01_03_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_01_03_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_01_03_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_01_03_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_01_03_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_01_03_fd_out_loop65_0);

                break;
            case 3: //0104
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_180;
                fdvtconfig.SRC_IMG_WIDTH = 600;
                fdvtconfig.SRC_IMG_HEIGHT = 491;
                fdvtconfig.SRC_IMG_FMT = FMT_YVYU;
                fdvtconfig.SRC_IMG_STRIDE = 1200;
                gFD_golden_result_19_va = (unsigned int *)&gold_01_04_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_01_04_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_01_04_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_01_04_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_01_04_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_01_04_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_01_04_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_01_04_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_01_04_fd_out_loop65_0);

                break;
            case 4: //0105
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 600;
                fdvtconfig.SRC_IMG_HEIGHT = 1067;
                fdvtconfig.SRC_IMG_FMT = FMT_UYVY;
                fdvtconfig.SRC_IMG_STRIDE = 1200;
                gFD_golden_result_19_va = (unsigned int *)&gold_01_05_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_01_05_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_01_05_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_01_05_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_01_05_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_01_05_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_01_05_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_01_05_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_01_05_fd_out_loop65_0);

                break;
            case 5: //0106
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_90;
                fdvtconfig.SRC_IMG_WIDTH = 600;
                fdvtconfig.SRC_IMG_HEIGHT = 600;
                fdvtconfig.SRC_IMG_FMT = FMT_VYUY;
                fdvtconfig.SRC_IMG_STRIDE = 1200;
                gFD_golden_result_19_va = (unsigned int *)&gold_01_06_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_01_06_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_01_06_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_01_06_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_01_06_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_01_06_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_01_06_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_01_06_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_01_06_fd_out_loop65_0);

                break;
            case 6: //0107
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_270;
                fdvtconfig.SRC_IMG_WIDTH = 800;
                fdvtconfig.SRC_IMG_HEIGHT = 600;
                fdvtconfig.SRC_IMG_FMT = FMT_MONO;
                fdvtconfig.SRC_IMG_STRIDE = 800;
                gFD_golden_result_19_va = (unsigned int *)&gold_01_07_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_01_07_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_01_07_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_01_07_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_01_07_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_01_07_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_01_07_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_01_07_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_01_07_fd_out_loop65_0);

                break;
            case 7: //0201
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 300;
                fdvtconfig.SRC_IMG_HEIGHT = 225;
                fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
                fdvtconfig.SRC_IMG_STRIDE = 304;
                gFD_golden_result_19_va = (unsigned int *)&gold_02_01_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_02_01_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_02_01_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_02_01_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_02_01_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_02_01_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_02_01_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_02_01_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_02_01_fd_out_loop65_0);
                break;
            case 8: //0202
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 300;
                fdvtconfig.SRC_IMG_HEIGHT = 225;
                fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
                fdvtconfig.SRC_IMG_STRIDE = 304;
                gFD_golden_result_19_va = (unsigned int *)&gold_02_02_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_02_02_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_02_02_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_02_02_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_02_02_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_02_02_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_02_02_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_02_02_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_02_02_fd_out_loop65_0);
                break;
            case 9: //0203
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 300;
                fdvtconfig.SRC_IMG_HEIGHT = 225;
                fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
                fdvtconfig.SRC_IMG_STRIDE = 304;
                gFD_golden_result_19_va = (unsigned int *)&gold_02_03_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_02_03_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_02_03_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_02_03_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_02_03_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_02_03_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_02_03_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_02_03_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_02_03_fd_out_loop65_0);
                break;
            case 10: //0204
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 300;
                fdvtconfig.SRC_IMG_HEIGHT = 225;
                fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
                fdvtconfig.SRC_IMG_STRIDE = 304;
                gFD_golden_result_19_va = (unsigned int *)&gold_02_04_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_02_04_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_02_04_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_02_04_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_02_04_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_02_04_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_02_04_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_02_04_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_02_04_fd_out_loop65_0);
                break;
            case 11: //0205
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 300;
                fdvtconfig.SRC_IMG_HEIGHT = 225;
                fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
                fdvtconfig.SRC_IMG_STRIDE = 304;
                gFD_golden_result_19_va = (unsigned int *)&gold_02_05_fd_out_loop19_0[0];
                gFD_golden_result_39_va = (unsigned int *)&gold_02_05_fd_out_loop39_0[0];
                gFD_golden_result_65_va = (unsigned int *)&gold_02_05_fd_out_loop65_0[0];
                gFD_golden_y2r_config = (unsigned int *)&gold_02_05_yuv2rgb_confi_frame01[0];
                gFD_golden_rs_config = (unsigned int *)&gold_02_05_rs_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_02_05_fd_confi_frame01[0];
                gFD_result_19_size = sizeof(gold_02_05_fd_out_loop19_0);
                gFD_result_39_size = sizeof(gold_02_05_fd_out_loop39_0);
                gFD_result_65_size = sizeof(gold_02_05_fd_out_loop65_0);
                break;
            default:
                break;
        }

        Allocate_TestPattern_Buffer(subcase_index, para1);
        fdvtconfig.source_img_address = (MUINT64*)Imem_Buffer_Y.phyAddr;
        fdvtconfig.source_img_address_UV = (MUINT64*)Imem_Buffer_UV.phyAddr;
        LOG_INF("source_img_address: 0x%lx  source_img_address_UV: 0x%lx\n", (unsigned long)fdvtconfig.source_img_address, (unsigned long)fdvtconfig.source_img_address_UV);
        LOG_INF("--- [test_fdvt_default...start to push_back\n");
        rFdvtParams.mEGNConfigVec.push_back(fdvtconfig);
    }
    else if(subcase_index == 1) // ATTRIBUTEMODE
    {
        fdvtconfig.FD_MODE = ATTRIBUTEMODE;

        switch (para1) {
            case 0:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 596;
                fdvtconfig.SRC_IMG_HEIGHT = 596;
                fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
                fdvtconfig.SRC_IMG_STRIDE = 800;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 0;
                fdvtconfig.src_roi.y1 = 0;
                fdvtconfig.src_roi.x2 = 457;
                fdvtconfig.src_roi.y2 = 493;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 136;
                fdvtconfig.src_padding.right = 2;
                fdvtconfig.src_padding.up = 100;
                fdvtconfig.src_padding.down = 2;
                gFD_golden_y2r_config = (unsigned int *)&gold_03_01_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_03_01_fd_confi_frame01[0];
                gATTR_golden_result_16_va = (unsigned int *)&gold_03_01_fd_out_loop16_0[0];
                gATTR_golden_result_17_va = (unsigned int *)&gold_03_01_fd_out_loop17_0[0];
                gATTR_result_16_size = sizeof(gold_03_01_fd_out_loop16_0);
                gATTR_result_17_size = sizeof(gold_03_01_fd_out_loop17_0);
                break;
            case 1:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_90;
                fdvtconfig.SRC_IMG_WIDTH = 298;
                fdvtconfig.SRC_IMG_HEIGHT = 298;
                fdvtconfig.SRC_IMG_FMT = FMT_YVU_2P;
                fdvtconfig.SRC_IMG_STRIDE = 800;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 542;
                fdvtconfig.src_roi.y1 = 221;
                fdvtconfig.src_roi.x2 = 799;
                fdvtconfig.src_roi.y2 = 516;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 0;
                fdvtconfig.src_padding.right = 40;
                fdvtconfig.src_padding.up = 2;
                fdvtconfig.src_padding.down = 0;
                gFD_golden_y2r_config = (unsigned int *)&gold_03_02_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_03_02_fd_confi_frame01[0];
                gATTR_golden_result_16_va = (unsigned int *)&gold_03_02_fd_out_loop16_0[0];
                gATTR_golden_result_17_va = (unsigned int *)&gold_03_02_fd_out_loop17_0[0];
                gATTR_result_16_size = sizeof(gold_03_02_fd_out_loop16_0);
                gATTR_result_17_size = sizeof(gold_03_02_fd_out_loop17_0);
                break;
            case 2:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_270;
                fdvtconfig.SRC_IMG_WIDTH = 484;
                fdvtconfig.SRC_IMG_HEIGHT = 484;
                fdvtconfig.SRC_IMG_FMT = FMT_YUYV;
                fdvtconfig.SRC_IMG_STRIDE = 1600;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 180;
                fdvtconfig.src_roi.y1 = 1;
                fdvtconfig.src_roi.x2 = 661;
                fdvtconfig.src_roi.y2 = 412;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 0;
                fdvtconfig.src_padding.right = 2;
                fdvtconfig.src_padding.up = 68;
                fdvtconfig.src_padding.down = 4;
                gFD_golden_y2r_config = (unsigned int *)&gold_03_03_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_03_03_fd_confi_frame01[0];
                gATTR_golden_result_16_va = (unsigned int *)&gold_03_03_fd_out_loop16_0[0];
                gATTR_golden_result_17_va = (unsigned int *)&gold_03_03_fd_out_loop17_0[0];
                gATTR_result_16_size = sizeof(gold_03_03_fd_out_loop16_0);
                gATTR_result_17_size = sizeof(gold_03_03_fd_out_loop17_0);
                break;
            case 3:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_180;
                fdvtconfig.SRC_IMG_WIDTH = 338;
                fdvtconfig.SRC_IMG_HEIGHT = 338;
                fdvtconfig.SRC_IMG_FMT = FMT_YVYU;
                fdvtconfig.SRC_IMG_STRIDE = 1600;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 474;
                fdvtconfig.src_roi.y1 = 0;
                fdvtconfig.src_roi.x2 = 799;
                fdvtconfig.src_roi.y2 = 323;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 2;
                fdvtconfig.src_padding.right = 10;
                fdvtconfig.src_padding.up = 12;
                fdvtconfig.src_padding.down = 2;
                gFD_golden_y2r_config = (unsigned int *)&gold_03_04_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_03_04_fd_confi_frame01[0];
                gATTR_golden_result_16_va = (unsigned int *)&gold_03_04_fd_out_loop16_0[0];
                gATTR_golden_result_17_va = (unsigned int *)&gold_03_04_fd_out_loop17_0[0];
                gATTR_result_16_size = sizeof(gold_03_04_fd_out_loop16_0);
                gATTR_result_17_size = sizeof(gold_03_04_fd_out_loop17_0);
                break;
            case 4:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 92;
                fdvtconfig.SRC_IMG_HEIGHT = 92;
                fdvtconfig.SRC_IMG_FMT = FMT_UYVY;
                fdvtconfig.SRC_IMG_STRIDE = 1600;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 382;
                fdvtconfig.src_roi.y1 = 505;
                fdvtconfig.src_roi.x2 = 471;
                fdvtconfig.src_roi.y2 = 594;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 0;
                fdvtconfig.src_padding.right = 2;
                fdvtconfig.src_padding.up = 0;
                fdvtconfig.src_padding.down = 2;
                gFD_golden_y2r_config = (unsigned int *)&gold_03_05_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_03_05_fd_confi_frame01[0];
                gATTR_golden_result_16_va = (unsigned int *)&gold_03_05_fd_out_loop16_0[0];
                gATTR_golden_result_17_va = (unsigned int *)&gold_03_05_fd_out_loop17_0[0];
                gATTR_result_16_size = sizeof(gold_03_05_fd_out_loop16_0);
                gATTR_result_17_size = sizeof(gold_03_05_fd_out_loop17_0);
                break;
            case 5:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_90;
                fdvtconfig.SRC_IMG_WIDTH = 108;
                fdvtconfig.SRC_IMG_HEIGHT = 108;
                fdvtconfig.SRC_IMG_FMT = FMT_VYUY;
                fdvtconfig.SRC_IMG_STRIDE = 1600;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 20;
                fdvtconfig.src_roi.y1 = 371;
                fdvtconfig.src_roi.x2 = 125;
                fdvtconfig.src_roi.y2 = 476;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 0;
                fdvtconfig.src_padding.right = 2;
                fdvtconfig.src_padding.up = 0;
                fdvtconfig.src_padding.down = 2;
                gFD_golden_y2r_config = (unsigned int *)&gold_03_06_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_03_06_fd_confi_frame01[0];
                gATTR_golden_result_16_va = (unsigned int *)&gold_03_06_fd_out_loop16_0[0];
                gATTR_golden_result_17_va = (unsigned int *)&gold_03_06_fd_out_loop17_0[0];
                gATTR_result_16_size = sizeof(gold_03_06_fd_out_loop16_0);
                gATTR_result_17_size = sizeof(gold_03_06_fd_out_loop17_0);
                break;
            case 6:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_270;
                fdvtconfig.SRC_IMG_WIDTH = 84;
                fdvtconfig.SRC_IMG_HEIGHT = 84;
                fdvtconfig.SRC_IMG_FMT = FMT_MONO;
                fdvtconfig.SRC_IMG_STRIDE = 800;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 296;
                fdvtconfig.src_roi.y1 = 513;
                fdvtconfig.src_roi.x2 = 377;
                fdvtconfig.src_roi.y2 = 594;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 0;
                fdvtconfig.src_padding.right = 2;
                fdvtconfig.src_padding.up = 0;
                fdvtconfig.src_padding.down = 2;
                gFD_golden_y2r_config = (unsigned int *)&gold_03_07_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_03_07_fd_confi_frame01[0];
                gATTR_golden_result_16_va = (unsigned int *)&gold_03_07_fd_out_loop16_0[0];
                gATTR_golden_result_17_va = (unsigned int *)&gold_03_07_fd_out_loop17_0[0];
                gATTR_result_16_size = sizeof(gold_03_07_fd_out_loop16_0);
                gATTR_result_17_size = sizeof(gold_03_07_fd_out_loop17_0);
                break;
            default:
                break;
        }
        Allocate_TestPattern_Buffer(subcase_index, para1);
        fdvtconfig.source_img_address = (MUINT64*)Imem_Buffer_Y.phyAddr;
        fdvtconfig.source_img_address_UV = (MUINT64*)Imem_Buffer_UV.phyAddr;
        LOG_INF("source_img_address: 0x%lx  source_img_address_UV: 0x%lx\n", (unsigned long)fdvtconfig.source_img_address, (unsigned long)fdvtconfig.source_img_address_UV);
        LOG_INF("--- [test_fdvt_default...start to push_back\n");
        rFdvtParams.mEGNConfigVec.push_back(fdvtconfig);
    }
    else if(subcase_index == 2) // POSEMODE
    {
        fdvtconfig.FD_MODE = POSEMODE;
        switch (para1) {
            case 0:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 500;
                fdvtconfig.SRC_IMG_HEIGHT = 500;
                fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
                fdvtconfig.SRC_IMG_STRIDE = 1184;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 336;
                fdvtconfig.src_roi.y1 = 196;
                fdvtconfig.src_roi.x2 = 833;
                fdvtconfig.src_roi.y2 = 693;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 0;
                fdvtconfig.src_padding.right = 2;
                fdvtconfig.src_padding.up = 0;
                fdvtconfig.src_padding.down = 2;
                gFD_golden_y2r_config = (unsigned int *)&gold_04_01_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_04_01_fd_confi_frame01[0];
                gPOSE_golden_result_16_va = (unsigned int *)&gold_04_01_fd_out_loop16_0[0];
                gPOSE_golden_result_17_va = (unsigned int *)&gold_04_01_fd_out_loop17_0[0];
                gPOSE_result_16_size = sizeof(gold_04_01_fd_out_loop16_0);
                gPOSE_result_17_size = sizeof(gold_04_01_fd_out_loop17_0);
                break;
            case 1:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 346;
                fdvtconfig.SRC_IMG_HEIGHT = 346;
                fdvtconfig.SRC_IMG_FMT = FMT_YVU_2P;
                fdvtconfig.SRC_IMG_STRIDE = 1184;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 412;
                fdvtconfig.src_roi.y1 = 1;
                fdvtconfig.src_roi.x2 = 755;
                fdvtconfig.src_roi.y2 = 288;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 0;
                fdvtconfig.src_padding.right = 2;
                fdvtconfig.src_padding.up = 52;
                fdvtconfig.src_padding.down = 6;
                gFD_golden_y2r_config = (unsigned int *)&gold_04_02_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_04_02_fd_confi_frame01[0];
                gPOSE_golden_result_16_va = (unsigned int *)&gold_04_02_fd_out_loop16_0[0];
                gPOSE_golden_result_17_va = (unsigned int *)&gold_04_02_fd_out_loop17_0[0];
                gPOSE_result_16_size = sizeof(gold_04_02_fd_out_loop16_0);
                gPOSE_result_17_size = sizeof(gold_04_02_fd_out_loop17_0);
                break;
            case 2:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 348;
                fdvtconfig.SRC_IMG_HEIGHT = 348;
                fdvtconfig.SRC_IMG_FMT = FMT_YUYV;
                fdvtconfig.SRC_IMG_STRIDE = 2368;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 0;
                fdvtconfig.src_roi.y1 = 265;
                fdvtconfig.src_roi.x2 = 277;
                fdvtconfig.src_roi.y2 = 610;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 64;
                fdvtconfig.src_padding.right = 6;
                fdvtconfig.src_padding.up = 0;
                fdvtconfig.src_padding.down = 2;
                gFD_golden_y2r_config = (unsigned int *)&gold_04_03_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_04_03_fd_confi_frame01[0];
                gPOSE_golden_result_16_va = (unsigned int *)&gold_04_03_fd_out_loop16_0[0];
                gPOSE_golden_result_17_va = (unsigned int *)&gold_04_03_fd_out_loop17_0[0];
                gPOSE_result_16_size = sizeof(gold_04_03_fd_out_loop16_0);
                gPOSE_result_17_size = sizeof(gold_04_03_fd_out_loop17_0);
                break;
            case 3:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 354;
                fdvtconfig.SRC_IMG_HEIGHT = 354;
                fdvtconfig.SRC_IMG_FMT = FMT_YVYU;
                fdvtconfig.SRC_IMG_STRIDE = 2368;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 0;
                fdvtconfig.src_roi.y1 = 0;
                fdvtconfig.src_roi.x2 = 279;
                fdvtconfig.src_roi.y2 = 291;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 74;
                fdvtconfig.src_padding.right = 0;
                fdvtconfig.src_padding.up = 62;
                fdvtconfig.src_padding.down = 0;
                gFD_golden_y2r_config = (unsigned int *)&gold_04_04_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_04_04_fd_confi_frame01[0];
                gPOSE_golden_result_16_va = (unsigned int *)&gold_04_04_fd_out_loop16_0[0];
                gPOSE_golden_result_17_va = (unsigned int *)&gold_04_04_fd_out_loop17_0[0];
                gPOSE_result_16_size = sizeof(gold_04_04_fd_out_loop16_0);
                gPOSE_result_17_size = sizeof(gold_04_04_fd_out_loop17_0);

                break;
            case 4:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 306;
                fdvtconfig.SRC_IMG_HEIGHT = 306;
                fdvtconfig.SRC_IMG_FMT = FMT_UYVY;
                fdvtconfig.SRC_IMG_STRIDE = 2368;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 0;
                fdvtconfig.src_roi.y1 = 585;
                fdvtconfig.src_roi.x2 = 283;
                fdvtconfig.src_roi.y2 = 886;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 16;
                fdvtconfig.src_padding.right = 6;
                fdvtconfig.src_padding.up = 2;
                fdvtconfig.src_padding.down = 2;
                gFD_golden_y2r_config = (unsigned int *)&gold_04_05_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_04_05_fd_confi_frame01[0];
                gPOSE_golden_result_16_va = (unsigned int *)&gold_04_05_fd_out_loop16_0[0];
                gPOSE_golden_result_17_va = (unsigned int *)&gold_04_05_fd_out_loop17_0[0];
                gPOSE_result_16_size = sizeof(gold_04_05_fd_out_loop16_0);
                gPOSE_result_17_size = sizeof(gold_04_05_fd_out_loop17_0);

                break;
            case 5:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 344;
                fdvtconfig.SRC_IMG_HEIGHT = 344;
                fdvtconfig.SRC_IMG_FMT = FMT_VYUY;
                fdvtconfig.SRC_IMG_STRIDE = 2368;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 900;
                fdvtconfig.src_roi.y1 = 595;
                fdvtconfig.src_roi.x2 = 1181;
                fdvtconfig.src_roi.y2 = 886;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 0;
                fdvtconfig.src_padding.right = 62;
                fdvtconfig.src_padding.up = 0;
                fdvtconfig.src_padding.down = 52;
                gFD_golden_y2r_config = (unsigned int *)&gold_04_06_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_04_06_fd_confi_frame01[0];
                gPOSE_golden_result_16_va = (unsigned int *)&gold_04_06_fd_out_loop16_0[0];
                gPOSE_golden_result_17_va = (unsigned int *)&gold_04_06_fd_out_loop17_0[0];
                gPOSE_result_16_size = sizeof(gold_04_06_fd_out_loop16_0);
                gPOSE_result_17_size = sizeof(gold_04_06_fd_out_loop17_0);

                break;
            case 6:
                fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
                fdvtconfig.SRC_IMG_WIDTH = 344;
                fdvtconfig.SRC_IMG_HEIGHT = 344;
                fdvtconfig.SRC_IMG_FMT = FMT_MONO;
                fdvtconfig.SRC_IMG_STRIDE = 1184;
                fdvtconfig.enROI = true;
                fdvtconfig.src_roi.x1 = 412;
                fdvtconfig.src_roi.y1 = 605;
                fdvtconfig.src_roi.x2 = 753;
                fdvtconfig.src_roi.y2 = 886;
                fdvtconfig.enPadding = true;
                fdvtconfig.src_padding.left = 2;
                fdvtconfig.src_padding.right = 0;
                fdvtconfig.src_padding.up = 2;
                fdvtconfig.src_padding.down = 60;
                gFD_golden_y2r_config = (unsigned int *)&gold_04_07_yuv2rgb_confi_frame01[0];
                gFD_golden_fd_config = (unsigned int *)&gold_04_07_fd_confi_frame01[0];
                gPOSE_golden_result_16_va = (unsigned int *)&gold_04_07_fd_out_loop16_0[0];
                gPOSE_golden_result_17_va = (unsigned int *)&gold_04_07_fd_out_loop17_0[0];
                gPOSE_result_16_size = sizeof(gold_04_07_fd_out_loop16_0);
                gPOSE_result_17_size = sizeof(gold_04_07_fd_out_loop17_0);
                break;
            default:
                break;
        }

        Allocate_TestPattern_Buffer(subcase_index, para1);
        fdvtconfig.source_img_address = (MUINT64*)Imem_Buffer_Y.phyAddr;
        fdvtconfig.source_img_address_UV = (MUINT64*)Imem_Buffer_UV.phyAddr;
        LOG_INF("source_img_address: 0x%lx  source_img_address_UV: 0x%lx\n", (unsigned long)fdvtconfig.source_img_address, (unsigned long)fdvtconfig.source_img_address_UV);
        LOG_INF("--- [test_fdvt_default...start to push_back\n");
        rFdvtParams.mEGNConfigVec.push_back(fdvtconfig);
    }

    LOG_INF("=====> Initial Driver Start\n");

    g_bFDVTCallback = MFALSE;

//////////////////////////// First Time Enque/Deque //////////////////////////////////
    //start_tick = clock();

    LOG_INF("=====> FDVT Enque Start\n");

    ret=pStream->EGNenque(rFdvtParams);
    if(!ret)
    {
        LOG_INF("---FDVT Enque ERRRRRRRRR [.fdvt enque fail\n]");
    }
    else
    {
        LOG_INF("---FDVT Enque [.fdvt enque done\n]");
    }

    //end_tick = clock();
    //elapsed = (double) (end_tick - start_tick) / CLOCKS_PER_SEC;
    //LOG_INF("FD Enque Elapsed Time: %f\n", elapsed);

    //start_tick = clock();
    //LOG_INF("=====> FDVT Deque Start\n");

    int q = 0;
    do{
        usleep(100000);
        LOG_INF("---waiting for callback\n]");
        q++;
        if (MTRUE == g_bFDVTCallback)
        {
            LOG_INF("Deque Done\n");
            break;
        }
    }while(q<50);

    //end_tick = clock();
    //elapsed = (double) (end_tick - start_tick) / CLOCKS_PER_SEC;
    //LOG_INF("FD Deque Elapsed Time: %f\n", elapsed);
//////////////////////////////////////////////////////////////

    NSCam::NSIoPipe::FDVTConfig* pFdvtConfig;
    pFdvtConfig = &(rFdvtParams.mEGNConfigVec.at(0));

    LOG_INF("Detected Face Number: %d\n", pFdvtConfig->FDOUTPUT.FD_TOTAL_NUM);

    if(subcase_index == 4) // Enque multi request
    {
        rFdvtParams.mEGNConfigVec.clear();
        fdvtconfig.FD_MODE = ATTRIBUTEMODE;
        fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
        fdvtconfig.SRC_IMG_WIDTH = 128;
        fdvtconfig.SRC_IMG_HEIGHT = 128;
        fdvtconfig.SRC_IMG_FMT = FMT_UYVY;
        Allocate_TestPattern_Buffer(1, 0);
        fdvtconfig.source_img_address = (MUINT64*)Imem_Buffer_Y.phyAddr;
        fdvtconfig.source_img_address_UV = (MUINT64*)Imem_Buffer_UV.phyAddr;
        LOG_INF("source_img_address: 0x%lx  source_img_address_UV: 0x%lx\n", (unsigned long)fdvtconfig.source_img_address, (unsigned long)fdvtconfig.source_img_address_UV);
        LOG_INF("--- [test_fdvt_default...start to push_back\n");
        rFdvtParams.mEGNConfigVec.push_back(fdvtconfig);

        LOG_INF("=====> Initial Driver Start\n");

        g_bFDVTCallback = MFALSE;

        LOG_INF("=====> FDVT Enque Start\n");

        ret=pStream->EGNenque(rFdvtParams);
        if(!ret)
        {
            LOG_INF("---FDVT second times Enque ERRRRRRRRR [.fdvt enque fail\n]");
        }
        else
        {
            LOG_INF("---FDVT second times Enque [.fdvt enque done\n]");
        }

        q = 0;

        do{
            usleep(100000);
            LOG_INF("---waiting for callback\n]");
            q++;
            if (MTRUE == g_bFDVTCallback)
            {
                LOG_INF("Deque Done\n");
                break;
            }
        }while(q<50);

        rFdvtParams.mEGNConfigVec.clear();
        fdvtconfig.FD_MODE = POSEMODE;
        fdvtconfig.INPUT_ROTATE_DEGREE = DEGREE_0;
        fdvtconfig.SRC_IMG_WIDTH = 128;
        fdvtconfig.SRC_IMG_HEIGHT = 128;
        fdvtconfig.SRC_IMG_FMT = FMT_YUV_2P;
        Allocate_TestPattern_Buffer(2, 0);
        fdvtconfig.source_img_address = (MUINT64*)Imem_Buffer_Y.phyAddr;
        fdvtconfig.source_img_address_UV = (MUINT64*)Imem_Buffer_UV.phyAddr;
        LOG_INF("source_img_address: 0x%lx  source_img_address_UV: 0x%lx\n", (unsigned long)fdvtconfig.source_img_address, (unsigned long)fdvtconfig.source_img_address_UV);
        LOG_INF("--- [test_fdvt_default...start to push_back\n");
        rFdvtParams.mEGNConfigVec.push_back(fdvtconfig);

        LOG_INF("=====> Initial Driver Start\n");

        g_bFDVTCallback = MFALSE;

        LOG_INF("=====> FDVT Enque Start\n");

        ret=pStream->EGNenque(rFdvtParams);
        if(!ret)
        {
            LOG_INF("---FDVT third times Enque ERRRRRRRRR [.fdvt enque fail\n]");
        }
        else
        {
            LOG_INF("---FDVT third times Enque [.fdvt enque done\n]");
        }

        q = 0;

        do{
            usleep(100000);
            LOG_INF("---waiting for callback\n]");
            q++;
            if (MTRUE == g_bFDVTCallback)
            {
                LOG_INF("Deque Done\n");
                break;
            }
        }while(q<50);
    }

    LOG_INF("=====> Free TestPattern Buffer Start\n");
    Free_TestPattern_Buffer();

    LOG_INF("=====> Uninitial Driver Start\n");
    pStream->uninit();

    LOG_INF("=====> end of test_fd\n");
    return ret;
}

MVOID FDVTCallback(EGNParams<FDVTConfig>& rParams)
{
    (void) rParams;
    int i = 0, j = 0 ;
    int size = rParams.mEGNConfigVec.size();
    unsigned int errCount_FDVT_Result = 0;

    for (; i < size; i++){
        MUINT32 reg1,reg2;
        //reg1 = rParams.mEGNConfigVec.at(i).feedback.reg1;
        //reg2 = rParams.mEGNConfigVec.at(i).feedback.reg2;
        LOG_INF("[FDVTCallback]FD_MODE: %d\n", rParams.mEGNConfigVec.at(i).FD_MODE);
        LOG_INF("[FDVTCallback]source_img_address: 0x%lx  source_img_address_UV: 0x%lx\n", (unsigned long)rParams.mEGNConfigVec.at(i).source_img_address, (unsigned long)rParams.mEGNConfigVec.at(i).source_img_address_UV);


        if(rParams.mEGNConfigVec.at(i).FD_MODE == 0)
        {
            LOG_INF("[FDVTCallback]Detected Face Number: %d\n", rParams.mEGNConfigVec.at(i).FDOUTPUT.FD_TOTAL_NUM);
            LOG_INF("[FDVTCallback]PYRAMID0_RESULT: %d\n", rParams.mEGNConfigVec.at(i).FDOUTPUT.PYRAMID0_RESULT.fd_partial_result);
            LOG_INF("[FDVTCallback]PYRAMID0_anchor_score: %d\n", rParams.mEGNConfigVec.at(i).FDOUTPUT.PYRAMID0_RESULT.anchor_score[0]);
            LOG_INF("[FDVTCallback]PYRAMID1_RESULT: %d\n", rParams.mEGNConfigVec.at(i).FDOUTPUT.PYRAMID1_RESULT.fd_partial_result);
            LOG_INF("[FDVTCallback]PYRAMID1_anchor_score: %d\n", rParams.mEGNConfigVec.at(i).FDOUTPUT.PYRAMID1_RESULT.anchor_score[0]);
            LOG_INF("[FDVTCallback]PYRAMID2_RESULT: %d\n", rParams.mEGNConfigVec.at(i).FDOUTPUT.PYRAMID2_RESULT.fd_partial_result);
            LOG_INF("[FDVTCallback]PYRAMID2_anchor_score: %d\n", rParams.mEGNConfigVec.at(i).FDOUTPUT.PYRAMID2_RESULT.anchor_score[0]);


            LOG_INF("bit true start: Y2R CONFIG");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).pY2R_config, (MUINT32 *)gFD_golden_y2r_config, sizeof(gold_01_01_yuv2rgb_confi_frame01));
            LOG_INF("bit true start: RS CONFIG");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).pRS_config, (MUINT32 *)gFD_golden_rs_config, sizeof(gold_01_01_rs_confi_frame01));
            LOG_INF("bit true start: FD CONFIG");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).pFd_config, (MUINT32 *)gFD_golden_fd_config, sizeof(gold_01_01_fd_confi_frame01));

            LOG_INF("bit true start: FD RESULT19_0");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).FDOUTPUT.FD_raw_result_2_va, (MUINT32 *)gFD_golden_result_19_va, gFD_result_19_size);
            LOG_INF("bit true start: FD RESULT39_0");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).FDOUTPUT.FD_raw_result_1_va, (MUINT32 *)gFD_golden_result_39_va, gFD_result_39_size);
            LOG_INF("bit true start: FD RESULT65_0");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).FDOUTPUT.FD_raw_result_0_va, (MUINT32 *)gFD_golden_result_65_va, gFD_result_65_size);
        }
        else if(rParams.mEGNConfigVec.at(i).FD_MODE == 1)
        {
            LOG_INF("[FDVTCallback]MERGED_RACE_RESULT.RESULT[0]  : %d\n", rParams.mEGNConfigVec.at(i).ATTRIBUTEOUTPUT.MERGED_RACE_RESULT.RESULT[0]);
            LOG_INF("[FDVTCallback]MERGED_RACE_RESULT.RESULT[1]  : %d\n", rParams.mEGNConfigVec.at(i).ATTRIBUTEOUTPUT.MERGED_RACE_RESULT.RESULT[1]);
            LOG_INF("[FDVTCallback]MERGED_RACE_RESULT.RESULT[2]  : %d\n", rParams.mEGNConfigVec.at(i).ATTRIBUTEOUTPUT.MERGED_RACE_RESULT.RESULT[2]);
            LOG_INF("[FDVTCallback]MERGED_RACE_RESULT.RESULT[3]  : %d\n", rParams.mEGNConfigVec.at(i).ATTRIBUTEOUTPUT.MERGED_RACE_RESULT.RESULT[3]);
            LOG_INF("[FDVTCallback]MERGED_GENDER_RESULT.RESULT[0]: %d\n", rParams.mEGNConfigVec.at(i).ATTRIBUTEOUTPUT.MERGED_GENDER_RESULT.RESULT[0]);
            LOG_INF("[FDVTCallback]MERGED_GENDER_RESULT.RESULT[1]: %d\n", rParams.mEGNConfigVec.at(i).ATTRIBUTEOUTPUT.MERGED_GENDER_RESULT.RESULT[1]);

            LOG_INF("bit true start: Y2R CONFIG");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).pY2R_config, (MUINT32 *)gFD_golden_y2r_config, sizeof(gold_03_01_yuv2rgb_confi_frame01));
            LOG_INF("bit true start: FD CONFIG");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).pFd_config, (MUINT32 *)gFD_golden_fd_config, sizeof(gold_03_01_fd_confi_frame01));

            LOG_INF("bit true start: FD RESULT16_0");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).ATTRIBUTEOUTPUT.ATTR_raw_result_0_va, (MUINT32 *)gATTR_golden_result_16_va, gATTR_result_16_size);
            LOG_INF("bit true start: FD RESULT17_0");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).ATTRIBUTEOUTPUT.ATTR_raw_result_1_va, (MUINT32 *)gATTR_golden_result_17_va, gATTR_result_17_size);


        }
        else if(rParams.mEGNConfigVec.at(i).FD_MODE == 2)
        {
              LOG_INF("[FDVTCallback]MERGED_RIP_RESULT.RESULT[0]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_RIP_RESULT.RESULT[0]);
              LOG_INF("[FDVTCallback]MERGED_RIP_RESULT.RESULT[1]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_RIP_RESULT.RESULT[1]);
              LOG_INF("[FDVTCallback]MERGED_RIP_RESULT.RESULT[2]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_RIP_RESULT.RESULT[2]);
              LOG_INF("[FDVTCallback]MERGED_RIP_RESULT.RESULT[3]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_RIP_RESULT.RESULT[3]);
              LOG_INF("[FDVTCallback]MERGED_RIP_RESULT.RESULT[4]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_RIP_RESULT.RESULT[4]);
              LOG_INF("[FDVTCallback]MERGED_RIP_RESULT.RESULT[5]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_RIP_RESULT.RESULT[5]);
              LOG_INF("[FDVTCallback]MERGED_RIP_RESULT.RESULT[6]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_RIP_RESULT.RESULT[6]);
              LOG_INF("[FDVTCallback]MERGED_ROP_RESULT.RESULT[0]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_ROP_RESULT.RESULT[0]);
              LOG_INF("[FDVTCallback]MERGED_ROP_RESULT.RESULT[1]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_ROP_RESULT.RESULT[1]);
              LOG_INF("[FDVTCallback]MERGED_ROP_RESULT.RESULT[2]: %d\n", rParams.mEGNConfigVec.at(i).POSEOUTPUT.MERGED_ROP_RESULT.RESULT[2]);

            LOG_INF("bit true start: Y2R CONFIG");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).pY2R_config, (MUINT32 *)gFD_golden_y2r_config, sizeof(gold_04_01_yuv2rgb_confi_frame01));
            LOG_INF("bit true start: FD CONFIG");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).pFd_config, (MUINT32 *)gFD_golden_fd_config, sizeof(gold_04_01_fd_confi_frame01));

            LOG_INF("bit true start: FD RESULT16_0");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).POSEOUTPUT.POSE_raw_result_0_va, (MUINT32 *)gPOSE_golden_result_16_va, gPOSE_result_16_size);
            LOG_INF("bit true start: FD RESULT17_0");
            errCount_FDVT_Result += FDVT_BitTrueCheck((MUINT32 *)rParams.mEGNConfigVec.at(i).POSEOUTPUT.POSE_raw_result_1_va, (MUINT32 *)gPOSE_golden_result_17_va, gPOSE_result_17_size);

        }
        //LOG_INF("Feedback statistics are:(0x%x,0x%x)\n", reg1,reg2);
    }

    g_bFDVTCallback = MTRUE;
}

