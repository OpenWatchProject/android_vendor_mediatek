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
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief



#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>


#include "isp_drv_cam.h"
#include "uni_drv.h"

#undef LOG_TAG
#define LOG_TAG "Test_IspDrv"

#define MY_LOGD(fmt, arg...)    printf(fmt, ##arg)
#define MY_LOGE(fmt, arg...)    printf("TS' [%s:%d][%s] \033[1;31m" fmt"\033[0m\n", __FILE__, __LINE__, __FUNCTION__, ##arg)

int UniDrvCam(MUINT32 _case)
{
    int ret= 0;
    switch(_case){
        case 0:
            {
                IspDrvImp* pDrvIsp = static_cast<IspDrvImp*>(IspDrvImp::createInstance(CAM_A));
                if(pDrvIsp == NULL){
                    MY_LOGD("CAM_A create fail\n");
                    return -1;
                }

                UniDrvImp* pDrvUni = (UniDrvImp*)UniDrvImp::createInstance(UNI_A);
                ISP_DRV_REG_IO_STRUCT Regs_W[6];
                if(pDrvUni == NULL){
                    MY_LOGD("UNI_A create fail\n");
                    return -1;
                }

                if(pDrvUni->init("Test_UniDrvCam_A",pDrvIsp) == MFALSE){
                    pDrvUni->destroyInstance();
                    MY_LOGD("UNI_A init failed\n");
                    return -1;
                }

                MY_LOGD("signle RW test\n");
                pDrvUni->writeReg(0x0010,0x76);
                MY_LOGD("magic number access : 0x%x\n",pDrvUni->readReg(0x0010));

                MY_LOGD("marco RW test\n");
                UNI_WRITE_REG(pDrvUni,CAM_UNI_TOP_MOD_EN,0x67);
                MY_LOGD("magic number access: 0x%x\n",UNI_READ_REG(pDrvUni,CAM_UNI_TOP_MOD_EN));

                MY_LOGD("marco bit test(bef:0x%x)\n",UNI_READ_REG(pDrvUni,CAM_UNI_TOP_DMA_EN));
                UNI_WRITE_BITS(pDrvUni,CAM_UNI_TOP_DMA_EN,RAWI_A_EN,1);
                MY_LOGD("p1 enable bit-access: 0x%x\n",UNI_READ_BITS(pDrvUni,CAM_UNI_TOP_DMA_EN,RAWI_A_EN));
                MY_LOGD("p1 enable access: 0x%x\n",UNI_READ_REG(pDrvUni,CAM_UNI_TOP_DMA_EN));

                MY_LOGD("burst RW test\n");
                for(int i=0;i<6;i++){
                    Regs_W[i].Addr = (0x03e0 + i*0x4);
                    Regs_W[i].Data = i + 0x100;
                    MY_LOGD("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
                }
                pDrvUni->writeRegs(Regs_W,6);
                for(int i=0;i<6;i++)
                    Regs_W[i].Data = 0x0;

                pDrvUni->readRegs(Regs_W,6);
                for(int i=0;i<6;i++)
                    MY_LOGD("eis spare access: 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data);

                MY_LOGD("burst marco RW test\n");

                UNI_BURST_WRITE_REGS(pDrvUni,CAM_UNI_FLKO_FH_SPARE_2,Regs_W,6);
                memset(Regs_W,0,sizeof(Regs_W));
                UNI_BURST_READ_REGS(pDrvUni,CAM_UNI_FLKO_FH_SPARE_2,Regs_W,6);
                for(int i=0;i<6;i++)
                    MY_LOGD("flk spare access: 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data);


                pDrvUni->uninit("Test_UniDrvCam_A");
                pDrvUni->destroyInstance();
                pDrvIsp->destroyInstance();

            }
            break;
        default:
            MY_LOGD("RW path err(0x%x)",_case);
            ret = -1;
            break;
    }
    return ret;
}
int CAM_SIGNAL_CTRL(void)
{
#define IRQ_CLEAR_WAIT

    int ret = 0;
    ISP_DRV_CAM* pDrvCam[CAM_MAX];
    ISP_WAIT_IRQ_ST WaitIrq;
    ISP_REGISTER_USERKEY_STRUCT RegIrq;
    char str[32];

    for(int sidx=0; sidx < CAM_MAX; sidx++) {
        if(sidx == CAM_A)
            strncpy(str, "Test_IspDrvCam_A", sizeof(str));
        else if(sidx == CAM_B)
            strncpy(str, "Test_IspDrvCam_B", sizeof(str));

        pDrvCam[sidx] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance((ISP_HW_MODULE)sidx, ISP_DRV_CQ_THRE0,0,str);

        if(pDrvCam[sidx] == NULL) {
            MY_LOGE("CAM_%d create fail\n", sidx);
            ret = -1;
        }

        if(pDrvCam[sidx]->init(str) == MFALSE){
            pDrvCam[sidx]->destroyInstance();
            MY_LOGE("CAM_%d init fail\n", sidx);
            ret = -1;
        }
    }

    if(ret == -1) {
        for(int sidx =0; sidx<CAM_MAX; sidx++) {
            if(sidx == CAM_A)
                strncpy(str, "Test_IspDrvCam_A", sizeof(str));
            else if(sidx == CAM_B)
                strncpy(str, "Test_IspDrvCam_B", sizeof(str));

            pDrvCam[sidx]->uninit(str);
            pDrvCam[sidx]->destroyInstance();
        }

        return ret;
    }

    for(int i=0;i<CAM_MAX;i++){
        WaitIrq.St_type = SIGNAL_INT;
        WaitIrq.Status = VS_INT_ST;
        WaitIrq.Timeout = 0xA02;

        MY_LOGD("#######################\n");
        MY_LOGD("    CAM_%d    \n", i);
        MY_LOGD("#######################\n");

        pDrvCam[i]->start();
        MY_LOGD("register irq\n");
        RegIrq.userName[0] = '\0';
        if(i==0){
            MY_LOGD(RegIrq.userName,"CAM_A");
        } else {
            MY_LOGD(RegIrq.userName,"CAM_B");
        }
        pDrvCam[i]->registerIrq(&RegIrq);
        MY_LOGD("register result:0x%x\n",RegIrq.userKey);
        WaitIrq.UserKey = RegIrq.userKey;

#if defined(IRQ_CLEAR_NONE)
        MY_LOGD("non-clear wait\n");
        WaitIrq.Clear = ISP_IRQ_CLEAR_NONE;
#elif defined(IRQ_CLEAR_WAIT)
        MY_LOGD("clear wait\n");
        WaitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
#else
        MY_LOGD("signal signal test\n");
        WaitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
        pDrvCam[i]->waitIrq(&WaitIrq);
        pDrvCam[i]->signalIrq(&WaitIrq);
#endif

        if(MFALSE == pDrvCam[i]->waitIrq(&WaitIrq)) {
            MY_LOGE("wait signal fail\n");
        } else {
            MY_LOGD("wait pass\n");
        }

        pDrvCam[i]->stop();
    }

    for(int sidx =0; sidx<CAM_MAX; sidx++) {
        if(sidx == CAM_A)
            strncpy(str, "Test_IspDrvCam_A", sizeof(str));
        else if(sidx == CAM_B)
            strncpy(str, "Test_IspDrvCam_B", sizeof(str));

        pDrvCam[sidx]->uninit(str);
        pDrvCam[sidx]->destroyInstance();
    }

    return ret;
}
/*thread_call_back*/
#define MACRO_RW (1)
static void*    RunningThread_1(void *arg)
{
    int ret = 0;
    ISP_DRV_CAM *ptr = reinterpret_cast<ISP_DRV_CAM*>(arg);
    ISP_DRV_REG_IO_STRUCT* Regs_W;
    ISP_DRV_REG_IO_STRUCT* Regs_R;
    MUINT32 bFail = MFALSE;


    /*Test code*/
    MY_LOGD("#######################\n");
    MY_LOGD("    Thread 1       \n");
    MY_LOGD("#######################\n");
    ptr->start();

    Regs_W = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
    Regs_R = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
    MY_LOGD("CAM_1 burst RW test\n");
    for(int i=0;i<6;i++){
        Regs_W[i].Addr = (0x0E70 + i*0x4);
        Regs_W[i].Data = i + 0xA00;
        Regs_R[i].Addr = Regs_W[i].Addr;
        //MY_LOGD("CAM_A:%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
    }
#ifndef MACRO_RW
    MY_LOGD("CAM_1 is writing...\n");
    ptr->getPhyObj()->writeRegs(Regs_W,6);
    MY_LOGD("CAM_1 is reading...\n");
    ptr->getPhyObj()->readRegs(Regs_R,6);
    MY_LOGD("CAM_1 reading completed...\n");

    for(int i=0;i<6;i++){
        MY_LOGD("CAM_A:imgo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
        if(Regs_W[i].Data != Regs_R[i].Data)
            bFail = MTRUE;
    }
    if(bFail){
        MY_LOGD("CAM_1:########################\n");
        MY_LOGD("CAM_1:Test 4:Reg compare fail\n");
        MY_LOGD("CAM_1:########################\n");
        bFail = MFALSE;
    }
#else
    MY_LOGD("CAM_1 burst  marco RW test\n");
    /*Use ptr->getPhyObj() for physical registers
     *     ptr              for virtuel registers
     * in the 1st arument of the macros.
     */
    MY_LOGD("CAM_1 is writing...\n");
    CAM_BURST_WRITE_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_W,6);
    MY_LOGD("CAM_1 is reading...\n");
    CAM_BURST_READ_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_R,6);
    MY_LOGD("CAM_1 reading completed...\n");
    for(int i=0;i<6;i++){
        MY_LOGD("CAM_1:rrzo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
        if(Regs_W[i].Data != Regs_R[i].Data)
            bFail = MTRUE;
    }
    if(bFail){
        MY_LOGD("CAM_1:########################\n");
        MY_LOGD("CAM_1:Test 5:5Reg compare fail\n");
        MY_LOGD("CAM_1:########################\n");
        bFail = MFALSE;
    }
#endif
    free(Regs_W);
    free(Regs_R);
    ptr->stop();

    pthread_exit((void *)0);
}
static void*    RunningThread_2(void *arg)
{
    int ret = 0;
    ISP_DRV_CAM *ptr = reinterpret_cast<ISP_DRV_CAM*>(arg);
    ISP_DRV_REG_IO_STRUCT* Regs_W;
    ISP_DRV_REG_IO_STRUCT* Regs_R;
    MUINT32 bFail = MFALSE;

    /*Test code*/
    MY_LOGD("#######################\n");
    MY_LOGD("    Thread 2           \n");
    MY_LOGD("#######################\n");

    ptr->start();
    /*MOT LOCKED*/
    Regs_W = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
    Regs_R = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
    MY_LOGD("CAM_2 burst RW test\n");
    for(int i=0;i<6;i++){
        Regs_W[i].Addr = (0x0E70 + i*0x4);
        Regs_W[i].Data = i + 0xB00;
        Regs_R[i].Addr = Regs_W[i].Addr;
        //MY_LOGD("  CAM_B:%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
    }
#ifndef MACRO_RW
    MY_LOGD("CAM_2 is writing...\n");
    ptr->getPhyObj()->writeRegs(Regs_W,6);
    MY_LOGD("CAM_2 is reading...\n");
    ptr->getPhyObj()->readRegs(Regs_R,6);
    MY_LOGD("CAM_2 reading completed...\n");
    for(int i=0;i<6;i++){
        MY_LOGD("    CAM_2:imgo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
        if(Regs_W[i].Data != Regs_R[i].Data)
            bFail = MTRUE;
    }
    if(bFail){
        MY_LOGD("    CAM_2:########################\n");
        MY_LOGD("    CAM_2:Test 4:Reg compare fail\n");
        MY_LOGD("    CAM_2:########################\n");
        bFail = MFALSE;
    }
#else
    /*LOCKED*/
    MY_LOGD("    CAM_2:burst marco RW test\n");

    MY_LOGD("CAM_2 is writing...\n");
    /*Use ptr->getPhyObj() for physical registers
     *     ptr              for virtuel registers
     * in the 1st arument of the macros.
     */
    CAM_BURST_WRITE_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_W,6);
    MY_LOGD("CAM_2 is reading...\n");
    CAM_BURST_READ_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_R,6);
    MY_LOGD("CAM_2 reading completed...\n");

    for(int i=0;i<6;i++){
        MY_LOGD("    rrzo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
        if(Regs_W[i].Data != Regs_R[i].Data)
            bFail = MTRUE;
    }
    if(bFail){
        MY_LOGD("    CAM_2:########################\n");
        MY_LOGD("    CAM_2:Reg compare fail\n");
        MY_LOGD("    CAM_2:########################\n");
        bFail = MFALSE;
    }
#endif
    free(Regs_W);
    free(Regs_R);
    ptr->stop();

    pthread_exit((void *)0);
}

int IspDrvCam(MUINT32 _case, E_ISP_CAM_CQ cq)
{
    int ret = 0;
    ISP_DRV_REG_IO_STRUCT* Regs_W;
    ISP_DRV_REG_IO_STRUCT* Regs_R;
    MUINT32 bFail = MFALSE;
    int i;
    switch(_case){  // 0 for phy, 1 for vir
        case 0:
            {
                ISP_DRV_CAM* pDrvCam = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_A");
                ISP_DRV_CAM* pDrvCam2 = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_B");
                ISP_DRV_CAM* ptr = NULL;
                MUINT32 addrofst,regnum;

                if(pDrvCam == NULL){
                    MY_LOGE("CAM_A create fail\n");
                    return -1;
                }

                if(pDrvCam->init("Test_IspDrvCam_A") == MFALSE){
                    pDrvCam->destroyInstance();
                    MY_LOGE("CAM_A init failure\n");
                    return -1;
                }


                if(pDrvCam2 == NULL){
                    MY_LOGE("CAM_B create fail\n");
                    ret = -1;
                    goto EXIT;
                }

                if(pDrvCam2->init("Test_IspDrvCam_B") == MFALSE){
                    pDrvCam2->destroyInstance();
                    MY_LOGE("CAM_B init failure\n");
                    ret = -1;
                    goto EXIT;
                }
                ptr = pDrvCam;

                MY_LOGD("#######################\n");
                MY_LOGD("    CAM_A    \n");
                MY_LOGD("#######################\n");

                for(int i=0;i<2;i++){
                    ptr->start();
                    MY_LOGD("signle RW test\n");
                    ptr->getPhyObj()->writeReg(0x0004,0x9876);
                    MY_LOGD("magic number access : 0x%x\n",ptr->getPhyObj()->readReg(0x0004));

                    MY_LOGD("marco RW test\n");
                    CAM_WRITE_REG(ptr->getPhyObj(),CAM_CTL_DMA_EN,0x6543);
                    MY_LOGD("magic number access: 0x%x\n",CAM_READ_REG(ptr->getPhyObj(),CAM_CTL_DMA_EN));

                    MY_LOGD("marco bit test(bef:0x%x)\n",CAM_READ_REG(ptr->getPhyObj(),CAM_CTL_EN));
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CTL_EN,PBN_EN,1);
                    MY_LOGD("p1 enable bit-access: 0x%x\n",CAM_READ_BITS(ptr->getPhyObj(),CAM_CTL_EN,PBN_EN));
                    MY_LOGD("p1 enable access: 0x%x\n",CAM_READ_REG(ptr->getPhyObj(),CAM_CTL_EN));

                    ptr->getCQModuleInfo(CAM_DMA_FH_IMGO_SPARE_,addrofst,regnum);
                    Regs_W = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
                    MY_LOGD("burst RW test\n");
                    for(int i=0;i<6;i++){
                        Regs_W[i].Addr = (addrofst + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                        Regs_R[i].Addr = Regs_W[i].Addr;
                        MY_LOGD("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
                    }
                    ptr->getPhyObj()->writeRegs(Regs_W,6);
                    ptr->getPhyObj()->readRegs(Regs_R,6);
                    for(int i=0;i<6;i++){
                        MY_LOGD("imgo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        MY_LOGE("########################\n");
                        MY_LOGE("Reg compare fail\n");
                        MY_LOGE("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }

                    MY_LOGD("burst marco RW test\n");

                    CAM_BURST_WRITE_REGS(ptr->getPhyObj(),CAM_RRZO_FH_SPARE_2,Regs_W,6);
                    CAM_BURST_READ_REGS(ptr->getPhyObj(),CAM_RRZO_FH_SPARE_2,Regs_R,6);
                    for(int i=0;i<6;i++){
                        MY_LOGD("rrzo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        MY_LOGE("########################\n");
                        MY_LOGE("Reg compare fail\n");
                        MY_LOGE("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }
                    free(Regs_W);
                    free(Regs_R);
                    ptr->stop();
                    ptr = pDrvCam2;
                    MY_LOGD("#######################\n");
                    MY_LOGD("    CAM_B    \n");
                    MY_LOGD("#######################\n");
                }

                pDrvCam2->uninit("Test_IspDrvCam_B");
                pDrvCam2->destroyInstance();
EXIT:

                pDrvCam->uninit("Test_IspDrvCam_A");
                pDrvCam->destroyInstance();

            }
            break;
        case 1:
            {
                ISP_DRV_CAM* ptr = NULL;
                ISP_DRV_CAM* pDrvCam_1 = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE1,0,"Test_IspDrvCam_A_1");
                ISP_DRV_CAM* pDrvCam2_1 = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE1,0,"Test_IspDrvCam_B_1");
                ISP_DRV_CAM* pDrvCam_2 = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE1,1,"Test_IspDrvCam_A_2");
                ISP_DRV_CAM* pDrvCam2_2 = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE1,1,"Test_IspDrvCam_B_2");
                MUINT32 addrofst,regnum;
                if(pDrvCam_1 == NULL){
                    MY_LOGE("CAM_A page 1 create fail");
                    return -1;
                }
                if(pDrvCam_1->init("Test_IspDrvCam_A_1") == MFALSE){
                    pDrvCam_1->destroyInstance();
                    return -1;
                }

                if(pDrvCam_2 == NULL){
                    MY_LOGE("CAM_A page 2 create fail");
                    ret = -1;
                    goto EXIT2;
                }
                if(pDrvCam_2->init("Test_IspDrvCam_A_2") == MFALSE){
                    pDrvCam_2->destroyInstance();
                    ret = -1;
                    goto EXIT2;

                }

                if(pDrvCam2_1 == NULL){
                    MY_LOGE("CAM_B page 1 create fail");
                    ret = -1;
                    goto EXIT3;
                }
                if(pDrvCam2_1->init("Test_IspDrvCam_B_1") == MFALSE){
                    pDrvCam2_1->destroyInstance();
                    ret = -1;
                    goto EXIT3;
                }

                if(pDrvCam2_2 == NULL){
                    MY_LOGE("CAM_B page 2 create fail");
                    ret = -1;
                    goto EXIT4;
                }
                if(pDrvCam2_2->init("Test_IspDrvCam_B_2") == MFALSE){
                    pDrvCam2_2->destroyInstance();
                    ret = -1;
                    goto EXIT4;

                }

                ptr = pDrvCam_1;
                do{
                    MY_LOGD("#########################\n");
                    MY_LOGD("ptr = %p\n",(MUINT8*)ptr);
                    MY_LOGD("#########################\n");
                    ptr->start();

                    MY_LOGD("signle RW test\n");
                    ptr->cqAddModule(CAM_CTL_EN_);
                    ptr->writeReg(0x0004,0x9876);
                    MY_LOGD("magic number access : 0x%x\n",ptr->readReg(0x0004));

                    MY_LOGD("marco RW test\n");
                    CAM_WRITE_REG(ptr,CAM_CTL_DMA_EN,0x6543);
                    MY_LOGD("magic number access: 0x%x\n",CAM_READ_REG(ptr,CAM_CTL_DMA_EN));



                    MY_LOGD("marco bit test(bef:0x%x)\n",CAM_READ_REG(ptr,CAM_CTL_EN));
                    CAM_WRITE_BITS(ptr,CAM_CTL_EN,PBN_EN,1);
                    MY_LOGD("p1 enable bit-access: 0x%x\n",CAM_READ_BITS(ptr,CAM_CTL_EN,PBN_EN));
                    MY_LOGD("p1 enable access: 0x%x\n",CAM_READ_REG(ptr,CAM_CTL_EN));

                    MY_LOGD("burst RW test\n");
                    ptr->cqAddModule(CAM_DMA_FH_IMGO_SPARE_);
                    ptr->getCQModuleInfo(CAM_DMA_FH_IMGO_SPARE_,addrofst,regnum);
                    Regs_W = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        Regs_W[i].Addr = (addrofst + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                        MY_LOGD("%d/%d: addr:0x%x,data:0x%x\n",i,regnum,Regs_W[i].Addr,Regs_W[i].Data);
                        Regs_R[i].Addr = Regs_W[i].Addr;
                    }
                    ptr->writeRegs(Regs_W,regnum);
                    ptr->readRegs(Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        MY_LOGD("imgo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        MY_LOGE("########################\n");
                        MY_LOGE("Reg compare fail\n");
                        MY_LOGE("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }
                    free(Regs_W);
                    free(Regs_R);

                    MY_LOGD("burst marco RW test\n");
                    ptr->cqAddModule(CAM_DMA_FH_RRZO_SPARE_);
                    ptr->getCQModuleInfo(CAM_DMA_FH_RRZO_SPARE_,addrofst,regnum);
                    Regs_W = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        Regs_W[i].Addr = (addrofst + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                        MY_LOGD("%d/%d: addr:0x%x,data:0x%x\n",i,regnum,Regs_W[i].Addr,Regs_W[i].Data);
                    }
                    CAM_BURST_WRITE_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_W,regnum);
                    ptr->flushCmdQ();
                    CAM_BURST_READ_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        MY_LOGD("rrzo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        MY_LOGE("########################\n");
                        MY_LOGE("Reg compare fail\n");
                        MY_LOGE("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }
                    free(Regs_W);
                    free(Regs_R);

                    MY_LOGD("CmdQ descriptor dump(ptr =%p)\n",(MUINT8*)ptr);
                    ptr->dumpCQTable();

                    ptr->cqDelModule(CAM_CTL_EN_);
                    ptr->cqDelModule(CAM_DMA_FH_IMGO_SPARE_);
                    ptr->cqDelModule(CAM_DMA_FH_RRZO_SPARE_);

                    MY_LOGD("CmdQ descriptor dump after delete(ptr =%p)\n",(MUINT8*)ptr);
                    ptr->dumpCQTable();
                    ptr->stop();
                    if(ptr == pDrvCam2_2)
                        ptr = NULL;
                    else if(ptr == pDrvCam2_1)
                        ptr = pDrvCam2_2;
                    else if(ptr == pDrvCam_2)
                        ptr = pDrvCam2_1;
                    else if(ptr == pDrvCam_1)
                        ptr = pDrvCam_2;

                }while(ptr != NULL);

                pDrvCam2_2->uninit("Test_IspDrvCam_B_2");
                pDrvCam2_2->destroyInstance();
EXIT4:
                pDrvCam2_1->uninit("Test_IspDrvCam_B_1");
                pDrvCam2_1->destroyInstance();
EXIT3:
                pDrvCam_2->uninit("Test_IspDrvCam_A_2");
                pDrvCam_2->destroyInstance();
EXIT2:
                pDrvCam_1->uninit("Test_IspDrvCam_A_1");
                pDrvCam_1->destroyInstance();
            }
            break;
        case 2:
            ret = CAM_SIGNAL_CTRL();
            break;
        /* This case is for Jessy test camera_isp from isp_drv.cpp API */
        case 3:
            {
                IspDrvImp* pDrvCam = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
                ISP_WAIT_IRQ_ST irq = {
                    ISP_IRQ_CLEAR_WAIT,
                    SIGNAL_INT,
                    VS_INT_ST,
                    0,
                    2000,
                    {0,0,0,0,0,0,0}  };
                pDrvCam->init("jessy test");
                /* Test model */
                pDrvCam->setRWMode(ISP_DRV_RW_IOCTL);
                pDrvCam->writeReg(0x0604,0x10001);
                pDrvCam->writeReg(0x0608,0x404C1);
                pDrvCam->writeReg(0x060C,0x10001000);
                pDrvCam->writeReg(0x0200,0x1001);
                pDrvCam->writeReg(0x0D00,0x96DF1083);
                pDrvCam->writeReg(0x0D00,0x96DF1080);


                pDrvCam->setRWMode(ISP_DRV_RW_MMAP);
                pDrvCam->writeReg(0x4,0x1); // CAM_A_REG_CTL_EN
                pDrvCam->writeReg(0x20,0x1); // CAM_A_REG_CTL_RAW_INT_EN
                pDrvCam->writeReg(0x500,0x1); //CAM_A_REG_TG_SEN_MODE
                pDrvCam->writeReg(0x504,0x1);
                pDrvCam->writeReg(0x508,0xCC00000);
                pDrvCam->writeReg(0x50C,0x9900000);

                /* The size of test model should be 0x1000 x 0x1000*/

                MY_LOGD("CAM_A:0x4508:0x%x\n",pDrvCam->readReg(0x508));

                for(i=0;i<10;i++){
                    MY_LOGD("wait vsync :0x%x\n",i);
                    bFail = pDrvCam->waitIrq(&irq );

                    if(bFail == MTRUE)
                        break;
                }

                pDrvCam->writeReg(0x500,0x0); //CAM_A_REG_TG_SEN_MODE
                pDrvCam->writeReg(0x504,0x0);

                if(i < 10)
                    MY_LOGD("Wait Vsync interrupt!!!! i: 0x%x\n",i);
                else
                    MY_LOGE("No Vsync!!!!!!!!!!!");

                pDrvCam->uninit("jessy test");
                pDrvCam->destroyInstance();
            }
            break;
        case 4:
            {
                int m = cq;

                ISP_DRV_CAM* ptr = NULL;
                /*change from CQ_THRE1 to CQ_THRE0*/
                ISP_DRV_CAM* pDrvCam_1  = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_A_1");
                ISP_DRV_CAM* pDrvCam2_1 = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_B_1");
                ISP_DRV_CAM* pDrvCam_2  = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,1,"Test_IspDrvCam_A_2");
                ISP_DRV_CAM* pDrvCam2_2 = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE0,1,"Test_IspDrvCam_B_2");
                MUINT32 addrofst,regnum;
                if(pDrvCam_1 == NULL){
                    MY_LOGE("CAM_A page 1 create fail");
                    return -1;
                }
                if(pDrvCam_1->init("Test_IspDrvCam_A_1") == MFALSE){
                    pDrvCam_1->destroyInstance();
                    return -1;
                }

                if(pDrvCam_2 == NULL){
                    MY_LOGE("CAM_A page 2 create fail");
                    ret = -1;
                    goto EXIT5;
                }
                if(pDrvCam_2->init("Test_IspDrvCam_A_2") == MFALSE){
                    pDrvCam_2->destroyInstance();
                    ret = -1;
                    goto EXIT5;

                }

                if(pDrvCam2_1 == NULL){
                    MY_LOGE("CAM_B page 1 create fail");
                    ret = -1;
                    goto EXIT6;
                }
                if(pDrvCam2_1->init("Test_IspDrvCam_B_1") == MFALSE){
                    pDrvCam2_1->destroyInstance();
                    ret = -1;
                    goto EXIT6;
                }

                if(pDrvCam2_2 == NULL){
                    MY_LOGE("CAM_B page 2 create fail");
                    ret = -1;
                    goto EXIT7;
                }
                if(pDrvCam2_2->init("Test_IspDrvCam_B_2") == MFALSE){
                    pDrvCam2_2->destroyInstance();
                    ret = -1;
                    goto EXIT7;

                }

                ptr = pDrvCam_1;
                do{
                    MY_LOGD("#########################\n");
                    MY_LOGD("ptr = 0x%lx \n",(unsigned long)ptr);
                    MY_LOGD("#########################\n");
                    ptr->start();

                    MY_LOGD("Test 1: DMA_FH_IMGO_SPARE virtual2physical write-by-CQ test\n");
                    /*CQ desc was allocated during init with virtual addr and physical base addr
                     * 0. virtual address, is used by cqAddModule to write module des
                     * 1. physical address is used to set desc base address HW register
                     */
#if 0
                    ptr->cqAddModule(CAM_DMA_FH_IMGO_SPARE_);
#else
                    ptr->cqAddModule((E_CAM_MODULE)m);
#endif
                    //write cq base addr
                    CAM_WRITE_REG(ptr->getPhyObj(),CAM_CQ_THR0_BASEADDR,(MUINTPTR)ptr->getCQDescBufPhyAddr());

                    MY_LOGD("CQ0 INFO: desc base address=0x%lu \n",(MUINTPTR)ptr->getCQDescBufPhyAddr());

                    //set cq trig mode
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_MODE,1);
                    MY_LOGD("CQ0 INFO: trigger mode:imme\n");
#if 0
                    ptr->getCQModuleInfo(CAM_DMA_FH_IMGO_SPARE_,addrofst,regnum);
#else
                    ptr->getCQModuleInfo((E_CAM_MODULE)m,addrofst,regnum);
                    if (regnum > 50) break;
#endif
                    Regs_W = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        Regs_W[i].Addr = (addrofst + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                        Regs_R[i].Addr = Regs_W[i].Addr;
                        MY_LOGD("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
                    }
                    ptr->writeRegs(Regs_W,regnum);
                    ptr->readRegs(Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        MY_LOGD("imgo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        MY_LOGE("########################\n");
                        MY_LOGE("ERR:Vir Reg Write failure\n");
                        MY_LOGE("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }

                    MY_LOGD("########################\n");
                    MY_LOGD("# Vir2Phy Reg CQ Update#\n");
                    MY_LOGD("########################\n");
                    //enable cq
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,1);
                    //trig cq
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CTL_START,CQ_THR0_START,1);

                    //CAM_BURST_READ_REGS(ptr->getPhyObj(),CAM_IMGO_FH_SPARE_2,Regs_R,regnum);
                    ptr->getPhyObj()->readRegs(Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        MY_LOGD("imgoo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        MY_LOGE("########################\n");
                        MY_LOGE("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Test Result:module %d FAIL.\n",m);
                        MY_LOGE("########################\n");
                        bFail = MFALSE;
                    }else
                        MY_LOGD(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Test Result:module %d PASS.\n",m);

                    free(Regs_W);
                    free(Regs_R);
#if 0
                    MY_LOGD("Test 2: DMA_FH_RRZO_SPARE virtual2physical write-by-CQ test\n");
                    ptr->cqAddModule(CAM_DMA_FH_RRZO_SPARE_);

                    /*1. desc base addr:
                     * CAM_CQ_THR0_BASEADDR (1A00 4168[31:0]) to dma_cfg.memBuf.base_pAddr
                     * which is allocated in init
                     */
                    cq0.dma_cfg.memBuf.base_pAddr = (MUINTPTR) ptr->getCQDescBufPhyAddr();
                    cq0.config();
                    //CAM_WRITE_REG(ptr->getPhyObj(),CAM_CQ_THR0_BASEADDR,cq0.dma_cfg.memBuf.base_pAddr);
                    MY_LOGD("CQ0 INFO: desc base address=0x%x\n",cq0.dma_cfg.memBuf.base_pAddr);

                    /*burst mode : cqAddModule(CAM_CQ_THRE0_ADDR_)*/
                    //cq0.write2CQ();

                    /*2. Trigger mode :CQ_DB_LOAD_MODE(1A00 4160[8])=1, CQ_THR0_MODE(1A00 4164[5:4])=1*/
                    cq0.setCQTriggerMode(DMA_CQ0::_immediate_);
                    //CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_MODE,0x1);
                    MY_LOGD("CQ0 INFO: trigger mode=%d\n",DMA_CQ0::_immediate_);

                    ptr->getCQModuleInfo(CAM_DMA_FH_RRZO_SPARE_,addrofst,regnum);
                    Regs_W = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        Regs_W[i].Addr = (addrofst + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                        MY_LOGD("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
                        Regs_R[i].Addr = Regs_W[i].Addr;
                    }
                    ptr->writeRegs(Regs_W,regnum);
                    //CAM_BURST_WRITE_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_W,regnum);
                    ptr->flushCmdQ();
                    ptr->readRegs(Regs_R,regnum);
                    //CAM_BURST_READ_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        MY_LOGD("rrzo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        MY_LOGD("########################\n");
                        MY_LOGD("Reg compare fail\n");
                        MY_LOGD("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }else{
                        MY_LOGD("########################\n");
                        MY_LOGD("Virtual reg R/W PASS.\n");
                        MY_LOGD("########################\n");
                    }


                    /*3. Enable: CQ_THR0_EN (1A00 4164[0])=1*/
                    cq0.enable(NULL);
                    //CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,0x1);

                    /*4. trigger to start : CQ_THR0_START=0 and wait for clr*/
                    cq0.TrigCQ();
                    //          CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,0x11);

                    MY_LOGD("########################\n");
                    MY_LOGD("# Vir2Phy Reg CQ Update#\n");
                    MY_LOGD("########################\n");

                    //CAM_BURST_READ_REGS(ptr->getPhyObj(),CAM_RRZO_FH_SPARE_2,Regs_R,regnum);
                    ptr->getPhyObj()->readRegs(Regs_R,regnum);

                    for(unsigned int i=0;i<regnum;i++){
                        MY_LOGD("rrzo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        MY_LOGD("########################\n");
                        MY_LOGD("#NG: Physical Reg set to virtual register failure\n");
                        MY_LOGD("########################\n");
                        bFail = MFALSE;
                    }else
                        MY_LOGD("PASS : physical Reg is updated with virtual registers.\n");

                    free(Regs_W);
                    free(Regs_R);

                    MY_LOGD("CmdQ descriptor dump(ptr =0x%x)\n",ptr);
                    ptr->dumpCQTable();

                    ptr->cqDelModule(CAM_DMA_FH_IMGO_SPARE_);
                    ptr->cqDelModule(CAM_DMA_FH_RRZO_SPARE_);

                    MY_LOGD("CmdQ descriptor dump after delete(ptr =0x%x)\n",ptr);
                    ptr->dumpCQTable();
                    ptr->stop();
                    cq0.disable();

                    if(ptr == pDrvCam2_2)
                        ptr = NULL;
                    else if(ptr == pDrvCam2_1)
                        ptr = pDrvCam2_2;
                    else if(ptr == pDrvCam_2)
                        ptr = pDrvCam2_1;
                    else if(ptr == pDrvCam_1)
                        ptr = pDrvCam_2;
                }while(ptr != NULL);
#else
                    m++;
                }while(m<139);

#endif
                pDrvCam2_2->uninit("Test_IspDrvCam_B_2");
                pDrvCam2_2->destroyInstance();
EXIT7:
                pDrvCam2_1->uninit("Test_IspDrvCam_B_1");
                pDrvCam2_1->destroyInstance();
EXIT6:
                pDrvCam_2->uninit("Test_IspDrvCam_A_2");
                pDrvCam_2->destroyInstance();
EXIT5:
                pDrvCam_1->uninit("Test_IspDrvCam_A_1");
                pDrvCam_1->destroyInstance();
            }
            break;
        case 5:
            {
                ISP_DRV_CAM* pDrvCamA  = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,cq,0,"Test_IspDrvCam_A");
                ISP_DRV_CAM* pDrvCamB = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,cq,0,"Test_IspDrvCam_B");
                ISP_DRV_CAM* ptr = NULL;

                pthread_t thread_camA, thread_camB;
                void* thread_camA_rst = NULL;
                void* thread_camB_rst = NULL;

                if(pDrvCamA == NULL){
                    MY_LOGE("CAM_A create fail\n");
                    return -1;
                }

                if(pDrvCamA->init("Test_IspDrvCam_A") == MFALSE){
                    pDrvCamA->destroyInstance();
                    MY_LOGE("CAM_A init failure\n");
                    return -1;
                }


                if(pDrvCamB == NULL){
                    MY_LOGE("CAM_B create fail\n");
                    ret = -1;
                    goto EXIT_B;
                }

                if(pDrvCamB->init("Test_IspDrvCam_B") == MFALSE){
                    pDrvCamB->destroyInstance();
                    MY_LOGE("CAM_B init failure\n");
                    ret = -1;
                    goto EXIT_B;
                }
                /*thread_call_back*/
                /*Racing between CAMA and CAMA using unlocked API to RW*/
                MY_LOGD("\n\nRacing between CAM_A and CAM_B Test:\n");
                pthread_create(&thread_camA, NULL, RunningThread_1, pDrvCamA);
                pthread_create(&thread_camB, NULL, RunningThread_2, pDrvCamB);

                pthread_join(thread_camA, &thread_camA_rst);
                pthread_join(thread_camB, &thread_camB_rst);

                /*Racing between CAMA and CAMA using unlocked API to RW*/
                MY_LOGD("\n\nRacing betweeen CAM_B and CAM_B Test:\n");
                pthread_create(&thread_camA, NULL, RunningThread_1, pDrvCamB);
                pthread_create(&thread_camB, NULL, RunningThread_2, pDrvCamB);

                pthread_join(thread_camA, &thread_camA_rst);
                pthread_join(thread_camB, &thread_camB_rst);

                pDrvCamB->uninit("Test_IspDrvCam_B");
                pDrvCamB->destroyInstance();
EXIT_B:

                pDrvCamA->uninit("Test_IspDrvCam_A");
                pDrvCamA->destroyInstance();

            }
            break;

        default:
            MY_LOGE("RW path err(0x%x)",_case);
            ret = -1;
            break;
    }

    return ret;
}

extern int IspDrvCam_LDVT(void);
int Test_IspDrv(int argc, char** argv)
{
    MUINT32 HwModoule;
    MUINT32 RW_Path = 0;
    E_ISP_CAM_CQ cq = ISP_DRV_CQ_THRE0;
    (void)argc;(void)argv;
    MY_LOGD("##############################\n");
    MY_LOGD("case 0: CAM\n");
    MY_LOGD("case 2: UNI\n");
    MY_LOGD("case 3: CAM's LDVT\n");
    MY_LOGD("select test hw module\n");
    MY_LOGD("##############################\n");

    char s = getchar();
    HwModoule = (MUINT32)atoi((const char*)&s);

    if(getchar() == EOF) {
        MY_LOGE("getchar is error");
    }

    switch(HwModoule){
        case 0:
        case 2:
            MY_LOGD("##############################\n");
            MY_LOGD("case 0: R/W to physical\n");
            MY_LOGD("case 1: R/W to virtual\n");
            if(HwModoule != 1){
                MY_LOGD("case 2: wait signal (vsync only so far)\n");
            }
            MY_LOGD("case 3: (Jessy UT test) R/W to physical, directly from isp_drv.cpp\n");
            MY_LOGD("case 4: WR to virtual and WR to physical via CQ0\n");
            MY_LOGD("case 5: Concurrent CAM_A and CAM_B register R/W access\n");

            MY_LOGD("select path\n");
            MY_LOGD("##############################\n");
            s = getchar();
            RW_Path = atoi((const char*)&s);
            //cq = (E_ISP_CAM_CQ) atoi((const char*)argv[2]);

            MY_LOGD("test case :HW module:0x%x, RW path:0x%x, CQ=%d\n",HwModoule,RW_Path,cq);
            break;
        default:
            break;
    }

    switch(HwModoule){
        case 0:
            return IspDrvCam(RW_Path,cq);
            break;
        case 2:
            return UniDrvCam(RW_Path);
            break;
        case 3:
            return IspDrvCam_LDVT();
            break;
    }
    return 0;
}
