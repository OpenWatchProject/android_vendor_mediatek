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
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
///////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>

#include "camera_isp.h"
#include <mtkcam/common.h>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv/imem_drv.h>   // For IMemDrv*.
#include "../imageio/inc/isp_function.h"
//#include <mtkcam/imageio/ispio_pipe_scenario.h>    // For enum EDrvScenario.

#undef LOG_TAG
#define LOG_TAG "Test_IspDrv"
#define ISP_DRV_CQ_DUMMY_WR_TOKEN 0x75EC
#define WR_NUM_MAX 1023
#define WR_START_OFFSET 0x4004

int IspDrvCam(MUINT32 _case, ISP_DRV_CQ_ENUM cq)
{
    int ret = 0, i;
    ISP_DRV_REG_IO_STRUCT* Regs_W;
    ISP_DRV_REG_IO_STRUCT* Regs_R;
    ISP_DRV_REG_IO_STRUCT* Regs_virR;
    MUINT32 bFail = MFALSE;

    switch (_case){
        case 0:
        {
            /* initial */
            IspDrv* pDrv = IspDrv::createInstance();

            printf("pDrv:0x%08x\n",pDrv);
            if (!pDrv) {
                printf("pDrv createInstance fail\n");
                ret = -1;
                return ret;
            }
            ret = pDrv->init("IspDrv_TestCam");
            if (ret < 0) {
                pDrv->destroyInstance();
                printf("pDrv init fail\n");
                return ret;
            }

            /* write & read register */
            Regs_W = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 20);
            Regs_R = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 20);
            for (i = 0; i < 20; i++) {
                Regs_W[i].Addr = (0x4004 + i*0x4);
                Regs_W[i].Data = 0x4;//i + 0x100;
                Regs_R[i].Addr = Regs_W[i].Addr;
                printf("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
            }
            pDrv->writeRegs(Regs_W, 20, ISP_DRV_USER_ISPF, 0);
            printf("IspDrv_Test: write done.\n");
            pDrv->readRegs(Regs_R, 20, 0);
            printf("IspDrv_Test: read done.\n");
            /* compare registers */
            for (i = 0; i < 20; i++) {
                printf("IspDrv_Test: reg=%x wValue=0x%x rValue=0x%x\n",
                        Regs_R[i].Addr, Regs_W[i].Data, Regs_R[i].Data);
                if (Regs_W[i].Data != Regs_R[i].Data)
                    bFail = MTRUE;
            }
            if(bFail){
                printf("########################\n");
                printf("Reg compare fail\n");
                printf("########################\n");
                ret = -1;
                bFail = MFALSE;
            }
            /* uninitial */
            free(Regs_W);
            free(Regs_R);
            pDrv->uninit("IspDrv_TestCam");
            pDrv->destroyInstance();
        }
        break;
        case 1:
        {
            /* initial */
            IspDrv* pDrv = IspDrv::createInstance();
            IspDrv* pVirtDrv = NULL;
            isp_reg_t* pVirtReg = NULL;
            int j = 0;
            ISP_DRV_CQ_CMD_DESC_STRUCT *pIspDesCqVa;
            MINT32 curBurstQNum=1;
            printf("pDrv:0x%08x\n",pDrv);
            if (!pDrv) {
                printf("pDrv createInstance fail\n");
                ret = -1;
                return ret;
            }
            ret = pDrv->init("IspDrv_TestCam");
            if (ret < 0) {
                pDrv->destroyInstance();
                printf("pDrv init fail\n");
                return ret;
            }

            pVirtDrv = pDrv->getCQInstance((ISP_DRV_CQ_ENUM)cq);
            if (!pVirtDrv){
                printf("pVirtDrv createInstance fail\n");
                ret = -1;
                return ret;
            }
            printf("pVirtDrv:0x%08x\n",pVirtDrv);
            ret = pVirtDrv->init();
            if (ret < 0) {
                pVirtDrv->destroyInstance();
                printf("pVirtDrv[%d] init fail\n", cq);
                return ret;
            }
            pVirtReg = (isp_reg_t*)(pVirtDrv->getRegAddr());
            printf("pVirtReg:0x%08x, cq%d\n",pVirtReg, cq);
            if (pVirtReg == NULL) {
                printf("getVirRegAddr fail ");
                return -1;
            }
            /* CQ0 baseaddress */
            ISP_WRITE_REG(pDrv,CAM_CTL_CQ0_BASEADDR,
                (unsigned long)pDrv->getCQDescBufPhyAddr(cq,0,0),ISP_DRV_USER_ISPF);
            printf("CQ addr:0x%x\n", pDrv->getCQDescBufPhyAddr(cq,0,0));
            /* CQ0 trigger mode & enable */
            pDrv->setCQTriggerMode((ISP_DRV_CQ_ENUM)cq,CQ_SINGLE_IMMEDIATE_TRIGGER,CQ_TRIG_BY_START);
            //ISP_WRITE_REG(pDrv, CAM_CTL_CQ_EN,0x10000000,ISP_DRV_USER_ISPF);
            //ISP_WRITE_REG(pDrv, CAM_CTL_START,0x00000020,ISP_DRV_USER_ISPF);
            /* CQ descriptor table */
            /*pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_EN_P1); //0x15004004
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_EN_P1_D); //0x15004010
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_EN_P2); //0x15004018
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_SCEN); //0x15004024
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_FMT_P1); //0x15004028
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_FMT_P1_D); //0x1500402c
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_FMT_P2); //0x15004030
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_SEL_P1); //0x15004034
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_SEL_P1_D); //0x15004038
            pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_SEL_P2);*/ //0x1500403c
            for (i = 0; i <= CAM_DMA_IMGO_SV2; i++) {
                pDrv->cqAddModule((ISP_DRV_CQ_ENUM)cq, 0, 0, (CAM_MODULE_ENUM)i);
            }

            /* Write to virtual register */
            Regs_W = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * WR_NUM_MAX);
            Regs_R = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * WR_NUM_MAX);
            Regs_virR = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * WR_NUM_MAX);
            printf("IspDrv_Test: write registers:\n");
            for (i = 0; i < WR_NUM_MAX; i++) {
                Regs_W[i].Addr = (WR_START_OFFSET + i*0x4);
                Regs_W[i].Data = 0x4;
                Regs_R[i].Addr = Regs_W[i].Addr;
                Regs_virR[i].Addr = Regs_W[i].Addr;
                printf("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
            }
            pVirtDrv->writeRegs(Regs_W, WR_NUM_MAX, ISP_DRV_USER_ISPF, 0);
            printf("IspDrv_Test: write done.\n");

            /* Dump CQ table & CQ start */
            pIspDesCqVa = (ISP_DRV_CQ_CMD_DESC_STRUCT*)pDrv->getCQDescBufVirAddr(ISP_DRV_CQ0,0,0);
            printf("##########################\n");
            printf("CQ0 Descriptor table\n");
            for(i=0;i <= CAM_DMA_IMGO_SV2; i++){
                if(pIspDesCqVa[i].u.cmd != ISP_DRV_CQ_DUMMY_WR_TOKEN) {
                    printf("[CQ%03X]:[%02d][0x%08x]\n",0x0,i,pIspDesCqVa[i].u.cmd);
                }
            }
			ISP_WRITE_REG(pDrv, CAM_CTL_CQ_EN, 0x10000000,ISP_DRV_USER_ISPF);
            ISP_WRITE_REG(pDrv, CAM_CTL_START, 0x00000020, ISP_DRV_USER_ISPF);

            /* Read from virtual & physical register */
            pVirtDrv->readRegs(Regs_virR, WR_NUM_MAX, 0);
            pDrv->readRegs(Regs_R, WR_NUM_MAX, 0);
            printf("IspDrv_Test: read done.\n");
            /* Compare registers */
            printf("##########################\n");
            printf("Register compare\n");
            for (i = 0; i < WR_NUM_MAX; i++) {
                printf("Reg=%x write=0x%x virtual=0x%x physical=0x%x\n",
                    Regs_R[i].Addr, Regs_W[i].Data, Regs_virR[i].Data, Regs_R[i].Data);
                if ((Regs_W[i].Data != Regs_R[i].Data) || (Regs_W[i].Data != Regs_virR[i].Data))
                    bFail = MTRUE;
            }
            pDrv->dumpCQTable(ISP_DRV_CQ0, 0, 0);

            if(bFail){
                printf("########################\n");
                printf("Reg compare fail\n");
                ret = -1;
                bFail = MFALSE;
            }
            /* uninitial */
            free(Regs_W);
            free(Regs_R);
            free(Regs_virR);
            /*pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_EN_P1);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_EN_P1_D);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_EN_P2);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_SCEN);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_FMT_P1);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_FMT_P1_D);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_FMT_P2);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_SEL_P1);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_SEL_P1_D);
            pVirtDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0,CAM_TOP_CTL_SEL_P2);*/
            for (i = 0; i <= CAM_DMA_IMGO_SV2; i++) {
                pDrv->cqDelModule((ISP_DRV_CQ_ENUM)cq, 0, 0, (CAM_MODULE_ENUM)i);
            }
            pDrv->uninit("IspDrv_TestCam");
            pVirtDrv->uninit();
            pDrv->destroyInstance();
            pVirtDrv->destroyInstance();

        }
        break;
        default:
            printf("RW path err(0x%x)\n",_case);
            ret = -1;
            break;
    }
    return ret;
}

int Test_IspDrv(void)
{
    MUINT32 HwModoule;
    MUINT32 RW_Path;
    int ret = 0;
    int s;
    printf("##############################\n");
    printf("Test hw module: CAM\n");
    printf("Path:\n");
    printf("case 0: R/W to physical\n");
    printf("case 1: WR to virtual and WR to physical via CQ0\n");
    printf("select <path>:\n");
    printf("##############################\n");

    s = getchar();
    s = s - '0';
    if ((s < 2) && (s >= 0)) {
        RW_Path = s;
    } else {
        RW_Path = 0;
        printf("invalid test case, default: 0\n");
    }
    ISP_DRV_CQ_ENUM cq = ISP_DRV_CQ0;
    printf("test case :HW module:CAM, RW path:0x%x, CQ=%d\n", RW_Path, cq);
    printf("##############################\n");
    printf("Start of IspDrvCam\n");
    ret = IspDrvCam(RW_Path,cq);
    printf("End of IspDrvCam\n");
    printf("##############################\n");
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
