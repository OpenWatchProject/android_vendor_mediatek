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
#include "main_DrvKernelCtrl.h"

#undef LOG_TAG
#define LOG_TAG "Test_IspDrv"


int UniDrvCam(MUINT32 _case)
{
    int ret= 0;
    printf("########## ERROR TO_DO Fix Cannon_ep ##########\n");
    switch(_case){
        case 0:
            {
                IspDrvImp* pDrvIsp = static_cast<IspDrvImp*>(IspDrvImp::createInstance(CAM_A));
                if(pDrvIsp == NULL){
                    printf("CAM_A create fail\n");
                    return -1;
                }

                UniDrvImp* pDrvUni = (UniDrvImp*)UniDrvImp::createInstance(UNI_A);
                ISP_DRV_REG_IO_STRUCT Regs_W[6];
                if(pDrvUni == NULL){
                    printf("UNI_A create fail\n");
                    return -1;
                }

                if(pDrvUni->init("Test_UniDrvCam_A",pDrvIsp) == MFALSE){
                    pDrvUni->destroyInstance();
                    printf("UNI_A init failed\n");
                    return -1;
                }

                printf("signle RW test\n");
                pDrvUni->writeReg(0x0010,0x76);
                printf("magic number access : 0x%x\n",pDrvUni->readReg(0x0010));

                printf("marco RW test\n");
                UNI_WRITE_REG(pDrvUni,CAM_UNI_TOP_MOD_EN,0x67);
                printf("magic number access: 0x%x\n",UNI_READ_REG(pDrvUni,CAM_UNI_TOP_MOD_EN));

                printf("marco bit test(bef:0x%x)\n",UNI_READ_REG(pDrvUni,CAM_UNI_TOP_DMA_EN));
                UNI_WRITE_BITS(pDrvUni,CAM_UNI_TOP_DMA_EN,RAWI_A_EN,1);
                printf("p1 enable bit-access: 0x%x\n",UNI_READ_BITS(pDrvUni,CAM_UNI_TOP_DMA_EN,RAWI_A_EN));
                printf("p1 enable access: 0x%x\n",UNI_READ_REG(pDrvUni,CAM_UNI_TOP_DMA_EN));

                printf("burst RW test\n");
                for(int i=0;i<6;i++){
                    Regs_W[i].Addr = (0x03e0 + i*0x4);
                    Regs_W[i].Data = i + 0x100;
                    printf("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
                }
                pDrvUni->writeRegs(Regs_W,6);
                for(int i=0;i<6;i++)
                    Regs_W[i].Data = 0x0;

                pDrvUni->readRegs(Regs_W,6);
                for(int i=0;i<6;i++)
                    printf("eis spare access: 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data);

                printf("burst marco RW test\n");

                /* cannon_ep reg *///UNI_BURST_WRITE_REGS(pDrvUni,CAM_UNI_FLKO_FH_SPARE_2,Regs_W,6);
                memset(Regs_W,0,sizeof(Regs_W));
                /* cannon_ep reg *///UNI_BURST_READ_REGS(pDrvUni,CAM_UNI_FLKO_FH_SPARE_2,Regs_W,6);
                for(int i=0;i<6;i++)
                    printf("flk spare access: 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data);


                pDrvUni->uninit("Test_UniDrvCam_A");
                pDrvUni->destroyInstance();
                pDrvIsp->destroyInstance();

            }
            break;
        default:
            printf("RW path err(0x%x)",_case);
            ret = -1;
            break;
    }
    return ret;
}
int CAM_SIGNAL_CTRL(void)
{
    int ret = 0;
    MUINT32 method;
    printf("##############################\n");
    printf("Need to open sensor 1st\n");
    printf("case 0: non-clear wait\n");
    printf("case 1: clear wait\n");
    printf("case 2: clear wait 1st, then signal signal, and then non-clear wait\n");
    printf("select test method\n");
    printf("##############################\n");
    int s = getchar();
    method = atoi((const char*)&s);

    ISP_DRV_CAM* pDrvCam = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_A");
    ISP_DRV_CAM* pDrvCam2 = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_B");
    ISP_WAIT_IRQ_ST WaitIrq;
    ISP_REGISTER_USERKEY_STRUCT RegIrq;
    ISP_DRV_CAM* ptr = NULL;

    if(pDrvCam == NULL){
        printf("CAM_A create fail\n");
        return -1;
    }

    if(pDrvCam->init("Test_IspDrvCam_A") == MFALSE){
        pDrvCam->destroyInstance();
        return -1;
    }


    if(pDrvCam2 == NULL){
        printf("CAM_B create fail\n");
        ret = -1;
        goto EXIT5;
    }

    if(pDrvCam2->init("Test_IspDrvCam_B") == MFALSE){
        pDrvCam2->destroyInstance();
        ret = -1;
        goto EXIT5;
    }
    ptr = pDrvCam;
    WaitIrq.St_type = SIGNAL_INT;
    WaitIrq.Status = VS_INT_ST;
    WaitIrq.Timeout = 0x502;

    printf("#######################\n");
    printf("    CAM_A    \n");
    printf("#######################\n");
    for(int i=0;i<2;i++){
        ptr->start();
        printf("register irq\n");
        RegIrq.userName[0] = '\0';
        if(i==0){
            sprintf(RegIrq.userName,"CAM_A");
        }
        else{
            sprintf(RegIrq.userName,"CAM_B");
        }
        ptr->registerIrq(&RegIrq);
        printf("register result:0x%x\n",RegIrq.userKey);
        WaitIrq.UserKey = RegIrq.userKey;

        switch(method){
            case 0:
                printf("non-clear wait\n");
                WaitIrq.Clear = ISP_IRQ_CLEAR_NONE;
                break;
            case 1:
                printf("clear wait\n");
                WaitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
                break;
            case 2:
                printf("signal signal test\n");
                WaitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
                ptr->waitIrq(&WaitIrq);
                ptr->signalIrq(&WaitIrq);

                break;
        }
        ptr->waitIrq(&WaitIrq);
        printf("wait pass\n");

        ptr->stop();
        ptr = pDrvCam2;
        printf("#######################\n");
        printf("    CAM_B    \n");
        printf("#######################\n");
    }

    pDrvCam2->uninit("Test_IspDrvCam_B");
    pDrvCam2->destroyInstance();
EXIT5:

    pDrvCam->uninit("Test_IspDrvCam_A");
    pDrvCam->destroyInstance();
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
    printf("#######################\n");
    printf("    Thread 1       \n");
    printf("#######################\n");
    ptr->start();

    Regs_W = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
    Regs_R = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
    printf("CAM_1 burst RW test\n");
    for(int i=0;i<6;i++){
        Regs_W[i].Addr = (0x0E70 + i*0x4);
        Regs_W[i].Data = i + 0xA00;
        Regs_R[i].Addr = Regs_W[i].Addr;
        //printf("CAM_A:%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
    }
#ifndef MACRO_RW
    printf("CAM_1 is writing...\n");
    ptr->getPhyObj()->writeRegs(Regs_W,6);
    printf("CAM_1 is reading...\n");
    ptr->getPhyObj()->readRegs(Regs_R,6);
    printf("CAM_1 reading completed...\n");

    for(int i=0;i<6;i++){
        printf("CAM_A:imgo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
        if(Regs_W[i].Data != Regs_R[i].Data)
            bFail = MTRUE;
    }
    if(bFail){
        printf("CAM_1:########################\n");
        printf("CAM_1:Test 4:Reg compare fail\n");
        printf("CAM_1:########################\n");
        bFail = MFALSE;
    }
#else
    printf("CAM_1 burst  marco RW test\n");
    /*Use ptr->getPhyObj() for physical registers
     *     ptr              for virtuel registers
     * in the 1st arument of the macros.
     */
    printf("CAM_1 is writing...\n");
    CAM_BURST_WRITE_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_W,6);
    printf("CAM_1 is reading...\n");
    CAM_BURST_READ_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_R,6);
    printf("CAM_1 reading completed...\n");
    for(int i=0;i<6;i++){
        printf("CAM_1:rrzo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
        if(Regs_W[i].Data != Regs_R[i].Data)
            bFail = MTRUE;
    }
    if(bFail){
        printf("CAM_1:########################\n");
        printf("CAM_1:Test 5:5Reg compare fail\n");
        printf("CAM_1:########################\n");
        bFail = MFALSE;
    }
#endif
    free(Regs_W);
    free(Regs_R);
    ptr->stop();

    ret = 0;

    //
    if(ret)
        pthread_exit((void *)1);
    else
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
    printf("#######################\n");
    printf("    Thread 2           \n");
    printf("#######################\n");

    ptr->start();
    /*MOT LOCKED*/
    Regs_W = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
    Regs_R = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
    printf("CAM_2 burst RW test\n");
    for(int i=0;i<6;i++){
        Regs_W[i].Addr = (0x0E70 + i*0x4);
        Regs_W[i].Data = i + 0xB00;
        Regs_R[i].Addr = Regs_W[i].Addr;
        //printf("  CAM_B:%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
    }
#ifndef MACRO_RW
    printf("CAM_2 is writing...\n");
    ptr->getPhyObj()->writeRegs(Regs_W,6);
    printf("CAM_2 is reading...\n");
    ptr->getPhyObj()->readRegs(Regs_R,6);
    printf("CAM_2 reading completed...\n");
    for(int i=0;i<6;i++){
        printf("    CAM_2:imgo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
        if(Regs_W[i].Data != Regs_R[i].Data)
            bFail = MTRUE;
    }
    if(bFail){
        printf("    CAM_2:########################\n");
        printf("    CAM_2:Test 4:Reg compare fail\n");
        printf("    CAM_2:########################\n");
        bFail = MFALSE;
    }
#else
    /*LOCKED*/
    printf("    CAM_2:burst marco RW test\n");

    printf("CAM_2 is writing...\n");
    /*Use ptr->getPhyObj() for physical registers
     *     ptr              for virtuel registers
     * in the 1st arument of the macros.
     */
    CAM_BURST_WRITE_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_W,6);
    printf("CAM_2 is reading...\n");
    CAM_BURST_READ_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_R,6);
    printf("CAM_2 reading completed...\n");

    for(int i=0;i<6;i++){
        printf("    rrzo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
        if(Regs_W[i].Data != Regs_R[i].Data)
            bFail = MTRUE;
    }
    if(bFail){
        printf("    CAM_2:########################\n");
        printf("    CAM_2:Reg compare fail\n");
        printf("    CAM_2:########################\n");
        bFail = MFALSE;
    }
#endif
    free(Regs_W);
    free(Regs_R);
    ptr->stop();

    ret = 0;
    if(ret)
        pthread_exit((void *)1);
    else
        pthread_exit((void *)0);
}

int IspDrvCam(MUINT32 _case, E_ISP_CAM_CQ cq)
{
#define CAM_LOOP_NUM    (3)

    int ret = 0;
    ISP_DRV_REG_IO_STRUCT* Regs_W;
    ISP_DRV_REG_IO_STRUCT* Regs_R;
    MUINT32 bFail = MFALSE;
    int i;
    switch(_case){  // 0 for phy, 1 for vir
        case 0:
            {
                const char *drvCamName[CAM_LOOP_NUM] = {"Test_IspDrvCam_A", "Test_IspDrvCam_B", "Test_IspDrvCam_C"};
                ISP_DRV_CAM* drvCam[CAM_LOOP_NUM];
                ISP_DRV_CAM* ptr = NULL;

                drvCam[0] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,drvCamName[0]);
                drvCam[1] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_B,ISP_DRV_CQ_THRE0,0,drvCamName[1]);
                drvCam[2] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_C,ISP_DRV_CQ_THRE0,0,drvCamName[2]);

                for (i = 0; i < CAM_LOOP_NUM; i++) {
                    if(drvCam[i] == NULL){
                        printf("CAM_A create fail\n");
                        return -1;
                    }

                    if(drvCam[i]->init(drvCamName[i]) == MFALSE){
                        drvCam[i]->destroyInstance();
                        printf("CAM_%c init failure\n", 'A'+i);
                        return -1;
                    }
                }

                for (i = 0; i < CAM_LOOP_NUM; i++) {
                    ptr = drvCam[i];

                    printf("#######################\n");
                    printf("    CAM_%c    \n", 'A'+i);
                    printf("#######################\n");

                    ptr->start();
                    printf("signle RW test\n");
                    ptr->getPhyObj()->writeReg(0x0004,0x9876);
                    printf("magic number access : 0x%x\n",ptr->getPhyObj()->readReg(0x0004));

                    printf("marco RW test\n");
                    CAM_WRITE_REG(ptr->getPhyObj(),CAM_CTL_DMA_EN,0x6543);
                    printf("magic number access: 0x%x\n",CAM_READ_REG(ptr->getPhyObj(),CAM_CTL_DMA_EN));

                    printf("marco bit test(bef:0x%x)\n",CAM_READ_REG(ptr->getPhyObj(),CAM_CTL_EN));
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CTL_EN,PBN_EN,1);
                    printf("p1 enable bit-access: 0x%x\n",CAM_READ_BITS(ptr->getPhyObj(),CAM_CTL_EN,PBN_EN));
                    printf("p1 enable access: 0x%x\n",CAM_READ_REG(ptr->getPhyObj(),CAM_CTL_EN));
                    Regs_W = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * 6);
                    printf("burst RW test\n");
                    for(int i=0;i<6;i++){
                        Regs_W[i].Addr = (0x4e4 + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                    Regs_R[i].Addr = Regs_W[i].Addr;
                        printf("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
                    }
                    ptr->getPhyObj()->writeRegs(Regs_W,6);
                    ptr->getPhyObj()->readRegs(Regs_R,6);
                    for(int i=0;i<6;i++){
                        printf("imgo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        printf("########################\n");
                        printf("\033[1;31mIMGO CAM_%c Reg compare fail\033[0m\n", 'A'+i);
                        printf("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    } else {
                        printf("########################\n");
                        printf("\033[1;32mIMGO CAM_%c Reg compare success\033[0m\n", 'A'+i);
                        printf("########################\n");
                    }

                    printf("burst marco RW test\n");
                    memset((void *)Regs_R, 0, sizeof(ISP_DRV_REG_IO_STRUCT)*6);

                    CAM_BURST_WRITE_REGS(ptr->getPhyObj(),CAM_RRZO_FH_SPARE_2,Regs_W,6);
                    CAM_BURST_READ_REGS(ptr->getPhyObj(),CAM_RRZO_FH_SPARE_2,Regs_R,6);
                    for(int i=0;i<6;i++){
                        printf("rrzo spare access: 0x%x_0x%x, 0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        printf("########################\n");
                        printf("\033[1;31mRRZO CAM_%c Reg compare fail\033[0m\n", 'A'+i);
                        printf("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    } else {
                        printf("########################\n");
                        printf("\033[1;32mRRZO CAM_%c Reg compare success\033[0m\n", 'A'+i);
                        printf("########################\n");
                    }
                    free(Regs_W);
                    free(Regs_R);
                    ptr->stop();
                }

                for (i = 0; i < CAM_LOOP_NUM; i++) {
                    drvCam[i]->uninit(drvCamName[i]);
                    drvCam[i]->destroyInstance();
                }
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
                    printf("CAM_A page 1 create fail");
                    return -1;
                }
                if(pDrvCam_1->init("Test_IspDrvCam_A_1") == MFALSE){
                    pDrvCam_1->destroyInstance();
                    return -1;
                }

                if(pDrvCam_2 == NULL){
                    printf("CAM_A page 2 create fail");
                    ret = -1;
                    goto EXIT2;
                }
                if(pDrvCam_2->init("Test_IspDrvCam_A_2") == MFALSE){
                    pDrvCam_2->destroyInstance();
                    ret = -1;
                    goto EXIT2;

                }

                if(pDrvCam2_1 == NULL){
                    printf("CAM_B page 1 create fail");
                    ret = -1;
                    goto EXIT3;
                }
                if(pDrvCam2_1->init("Test_IspDrvCam_B_1") == MFALSE){
                    pDrvCam2_1->destroyInstance();
                    ret = -1;
                    goto EXIT3;
                }

                if(pDrvCam2_2 == NULL){
                    printf("CAM_B page 2 create fail");
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
                    printf("#########################\n");
                    printf("ptr = %p\n",(MUINT8*)ptr);
                    printf("#########################\n");
                    ptr->start();

                    printf("signle RW test\n");
                    ptr->cqAddModule(CAM_CTL_EN_);
                    ptr->writeReg(0x0004,0x9876);
                    printf("magic number access : 0x%x\n",ptr->readReg(0x0004));

                    printf("marco RW test\n");
                    CAM_WRITE_REG(ptr,CAM_CTL_DMA_EN,0x6543);
                    printf("magic number access: 0x%x\n",CAM_READ_REG(ptr,CAM_CTL_DMA_EN));



                    printf("marco bit test(bef:0x%x)\n",CAM_READ_REG(ptr,CAM_CTL_EN));
                    CAM_WRITE_BITS(ptr,CAM_CTL_EN,PBN_EN,1);
                    printf("p1 enable bit-access: 0x%x\n",CAM_READ_BITS(ptr,CAM_CTL_EN,PBN_EN));
                    printf("p1 enable access: 0x%x\n",CAM_READ_REG(ptr,CAM_CTL_EN));

                    printf("burst RW test\n");
                    ptr->cqAddModule(CAM_DMA_FH_IMGO_SPARE_);
                    ptr->getCQModuleInfo(CAM_DMA_FH_IMGO_SPARE_,addrofst,regnum);
                    Regs_W = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        Regs_W[i].Addr = (addrofst + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                        printf("%d/%d: addr:0x%x,data:0x%x\n",i,regnum,Regs_W[i].Addr,Regs_W[i].Data);
                    Regs_R[i].Addr = Regs_W[i].Addr;
                    }
                    ptr->writeRegs(Regs_W,regnum);
                    ptr->readRegs(Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        printf("imgo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        printf("########################\n");
                        printf("Reg compare fail\n");
                        printf("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }
                    free(Regs_W);
                    free(Regs_R);

                    printf("burst marco RW test\n");
                    ptr->cqAddModule(CAM_DMA_FH_RRZO_SPARE_);
                    ptr->getCQModuleInfo(CAM_DMA_FH_RRZO_SPARE_,addrofst,regnum);
                    Regs_W = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        Regs_W[i].Addr = (addrofst + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                    printf("%d/%d: addr:0x%x,data:0x%x\n",i,regnum,Regs_W[i].Addr,Regs_W[i].Data);
                    }
                    CAM_BURST_WRITE_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_W,regnum);
                    ptr->flushCmdQ();
                    CAM_BURST_READ_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        printf("rrzo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        printf("########################\n");
                        printf("Reg compare fail\n");
                        printf("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }
                    free(Regs_W);
                    free(Regs_R);

                    printf("CmdQ descriptor dump(ptr =%p)\n",(MUINT8*)ptr);
                    ptr->dumpCQTable();

                    ptr->cqDelModule(CAM_CTL_EN_);
                    ptr->cqDelModule(CAM_DMA_FH_IMGO_SPARE_);
                    ptr->cqDelModule(CAM_DMA_FH_RRZO_SPARE_);

                    printf("CmdQ descriptor dump after delete(ptr =%p)\n",(MUINT8*)ptr);
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
                pDrvCam->writeReg(0x0608,0x2804C1);// dummy_pixel >= 8
                pDrvCam->writeReg(0x060C,0xF000B40);
                pDrvCam->writeReg(0x0610,0x8);//SENINF_TG1_TM_CLOCK
                pDrvCam->writeReg(0x0200,0x1001);
                pDrvCam->writeReg(0x0D00,0x96DF1083);
                pDrvCam->writeReg(0x0D00,0x96DF1080);


                pDrvCam->setRWMode(ISP_DRV_RW_MMAP);
                pDrvCam->writeReg(0x4,0x1); // CAM_A_REG_CTL_EN
                pDrvCam->writeReg(0x8,0x0); // CAM_A_REG_CTL_DMA_EN
                pDrvCam->writeReg(0x20,0x1); // CAM_A_REG_CTL_RAW_INT_EN
                pDrvCam->writeReg(0x500,0x1); //CAM_A_REG_TG_SEN_MODE
                pDrvCam->writeReg(0x504,0x1);
                pDrvCam->writeReg(0x508,0xCC00000);
                pDrvCam->writeReg(0x50C,0x9900000);

                /* The size of test model should be 0x1000 x 0x1000*/

                printf("CAM_A:0x4508:0x%x\n",pDrvCam->readReg(0x508));

                for(i=0;i<10;i++){
                    printf("wait vsync :0x%x\n",i);
                    bFail = pDrvCam->waitIrq(&irq );

                    if(bFail == MTRUE)
                        break;
                }

                pDrvCam->writeReg(0x500,0x0); //CAM_A_REG_TG_SEN_MODE
                pDrvCam->writeReg(0x504,0x0);

                if(i < 10)
                    printf("Wait Vsync interrupt!!!! i: 0x%x\n",i);
                else
                    printf("No Vsync!!!!!!!!!!!");

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
                    printf("CAM_A page 1 create fail");
                    return -1;
                }
                if(pDrvCam_1->init("Test_IspDrvCam_A_1") == MFALSE){
                    pDrvCam_1->destroyInstance();
                    return -1;
                }

                if(pDrvCam_2 == NULL){
                    printf("CAM_A page 2 create fail");
                    ret = -1;
                    goto EXIT5;
                }
                if(pDrvCam_2->init("Test_IspDrvCam_A_2") == MFALSE){
                    pDrvCam_2->destroyInstance();
                    ret = -1;
                    goto EXIT5;

                }

                if(pDrvCam2_1 == NULL){
                    printf("CAM_B page 1 create fail");
                    ret = -1;
                    goto EXIT6;
                }
                if(pDrvCam2_1->init("Test_IspDrvCam_B_1") == MFALSE){
                    pDrvCam2_1->destroyInstance();
                    ret = -1;
                    goto EXIT6;
                }

                if(pDrvCam2_2 == NULL){
                    printf("CAM_B page 2 create fail");
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
                    printf("#########################\n");
                    printf("ptr = 0x%lu \n",(unsigned long)ptr);
                    printf("#########################\n");
                    ptr->start();

                    printf("Test 1: DMA_FH_IMGO_SPARE virtual2physical write-by-CQ test\n");
                    /*CQ desc was allocated during init with virtual addr and physical base addr
                     * 0. virtual address, is used by cqAddModule to write module des
                     * 1. physical address is used to set desc base address HW register
                     */
#if 0
                    ptr->cqAddModule(CAM_DMA_FH_IMGO_SPARE_);
#else
                    //avoid modify CQ0 setting
                    if((E_CAM_MODULE)m == CAM_CQ_THRE0_ADDR_ || (E_CAM_MODULE)m == CAM_CQ_THRE0_SIZE_){
                        m++;
                        continue;
                    }

                    ptr->cqAddModule((E_CAM_MODULE)m);
#endif
                    //write cq base addr
                    CAM_WRITE_REG(ptr->getPhyObj(),CAM_CQ_THR0_BASEADDR,(MUINTPTR)ptr->getCQDescBufPhyAddr());

                    printf("CQ0 INFO: desc base address=0x%lu \n",(MUINTPTR)ptr->getCQDescBufPhyAddr());

                    //set cq trig mode
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_EN,CQ_DB_LOAD_MODE,1);
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_MODE,1);
                    printf("CQ0 INFO: trigger mode:imme\n");
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
                        printf("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
                    }
                    ptr->writeRegs(Regs_W,regnum);
                    ptr->readRegs(Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        printf("imgo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        printf("########################\n");
                        printf("ERR:Vir Reg Write failure\n");
                        printf("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }

                    printf("########################\n");
                    printf("# Vir2Phy Reg CQ Update#\n");
                    printf("########################\n");
                    //enable cq
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,1);
                    //trig cq
                    CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CTL_START,CQ_THR0_START,1);
                    //pulling cq0 done
                    for(unsigned int i=0;i<10;i++){
                        if(CAM_READ_BITS(ptr->getPhyObj(),CAM_CTL_RAW_INT2_STATUSX,CQ_THR0_DONE_ST))
                            break;
                    }

                    //CAM_BURST_READ_REGS(ptr->getPhyObj(),CAM_IMGO_FH_SPARE_2,Regs_R,regnum);
                    ptr->getPhyObj()->readRegs(Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        printf("imgoo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        printf("########################\n");
                        printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<Test Result:module %d FAIL.\n",m);
                        printf("########################\n");
                        bFail = MFALSE;
                    }else
                        printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Test Result:module %d PASS.\n",m);

                    free(Regs_W);
                    free(Regs_R);
#if 0
                    printf("Test 2: DMA_FH_RRZO_SPARE virtual2physical write-by-CQ test\n");
                    ptr->cqAddModule(CAM_DMA_FH_RRZO_SPARE_);

                    /*1. desc base addr:
                     * CAM_CQ_THR0_BASEADDR (1A00 4168[31:0]) to dma_cfg.memBuf.base_pAddr
                     * which is allocated in init
                     */
                    cq0.dma_cfg.memBuf.base_pAddr = (MUINTPTR) ptr->getCQDescBufPhyAddr();
                    cq0.config();
                    //CAM_WRITE_REG(ptr->getPhyObj(),CAM_CQ_THR0_BASEADDR,cq0.dma_cfg.memBuf.base_pAddr);
                    printf("CQ0 INFO: desc base address=0x%x\n",cq0.dma_cfg.memBuf.base_pAddr);

                    /*burst mode : cqAddModule(CAM_CQ_THRE0_ADDR_)*/
                    //cq0.write2CQ();

                    /*2. Trigger mode :CQ_DB_LOAD_MODE(1A00 4160[8])=1, CQ_THR0_MODE(1A00 4164[5:4])=1*/
                    cq0.setCQTriggerMode(DMA_CQ0::_immediate_);
                    //CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_MODE,0x1);
                    printf("CQ0 INFO: trigger mode=%d\n",DMA_CQ0::_immediate_);

                    ptr->getCQModuleInfo(CAM_DMA_FH_RRZO_SPARE_,addrofst,regnum);
                    Regs_W = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    Regs_R = (ISP_DRV_REG_IO_STRUCT *)malloc(sizeof(ISP_DRV_REG_IO_STRUCT) * regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        Regs_W[i].Addr = (addrofst + i*0x4);
                        Regs_W[i].Data = i + 0x100;
                        printf("%d: addr:0x%x,data:0x%x\n",i,Regs_W[i].Addr,Regs_W[i].Data);
                        Regs_R[i].Addr = Regs_W[i].Addr;
                    }
                    ptr->writeRegs(Regs_W,regnum);
                    //CAM_BURST_WRITE_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_W,regnum);
                    ptr->flushCmdQ();
                    ptr->readRegs(Regs_R,regnum);
                    //CAM_BURST_READ_REGS(ptr,CAM_RRZO_FH_SPARE_2,Regs_R,regnum);
                    for(unsigned int i=0;i<regnum;i++){
                        printf("rrzo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        printf("########################\n");
                        printf("Reg compare fail\n");
                        printf("########################\n");
                        ret = -1;
                        bFail = MFALSE;
                    }else{
                        printf("########################\n");
                        printf("Virtual reg R/W PASS.\n");
                        printf("########################\n");
                    }


                    /*3. Enable: CQ_THR0_EN (1A00 4164[0])=1*/
                    cq0.enable(NULL);
                    //CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,0x1);

                    /*4. trigger to start : CQ_THR0_START=0 and wait for clr*/
                    cq0.TrigCQ();
                    //          CAM_WRITE_BITS(ptr->getPhyObj(),CAM_CQ_THR0_CTL,CQ_THR0_EN,0x11);

                    printf("########################\n");
                    printf("# Vir2Phy Reg CQ Update#\n");
                    printf("########################\n");

                    //CAM_BURST_READ_REGS(ptr->getPhyObj(),CAM_RRZO_FH_SPARE_2,Regs_R,regnum);
                    ptr->getPhyObj()->readRegs(Regs_R,regnum);

                    for(unsigned int i=0;i<regnum;i++){
                        printf("rrzo spare access: 0x%x_0x%x,0x%x_0x%x\n",Regs_W[i].Addr,Regs_W[i].Data,Regs_R[i].Addr,Regs_R[i].Data);
                        if(Regs_W[i].Data != Regs_R[i].Data)
                            bFail = MTRUE;
                    }
                    if(bFail){
                        printf("########################\n");
                        printf("#NG: Physical Reg set to virtual register failure\n");
                        printf("########################\n");
                        bFail = MFALSE;
                    }else
                        printf("PASS : physical Reg is updated with virtual registers.\n");

                    free(Regs_W);
                    free(Regs_R);

                    printf("CmdQ descriptor dump(ptr =0x%x)\n",ptr);
                    ptr->dumpCQTable();

                    ptr->cqDelModule(CAM_DMA_FH_IMGO_SPARE_);
                    ptr->cqDelModule(CAM_DMA_FH_RRZO_SPARE_);

                    printf("CmdQ descriptor dump after delete(ptr =0x%x)\n",ptr);
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
                    printf("CAM_A create fail\n");
                    return -1;
                }

                if(pDrvCamA->init("Test_IspDrvCam_A") == MFALSE){
                    pDrvCamA->destroyInstance();
                    printf("CAM_A init failure\n");
                    return -1;
                }


                if(pDrvCamB == NULL){
                    printf("CAM_B create fail\n");
                    ret = -1;
                    goto EXIT_B;
                }

                if(pDrvCamB->init("Test_IspDrvCam_B") == MFALSE){
                    pDrvCamB->destroyInstance();
                    printf("CAM_B init failure\n");
                    ret = -1;
                    goto EXIT_B;
                }
                /*thread_call_back*/
                /*Racing between CAMA and CAMA using unlocked API to RW*/
                printf("\n\nRacing between CAM_A and CAM_B Test:\n");
                pthread_create(&thread_camA, NULL, RunningThread_1, pDrvCamA);
                pthread_create(&thread_camB, NULL, RunningThread_2, pDrvCamB);

                pthread_join(thread_camA, &thread_camA_rst);
                pthread_join(thread_camB, &thread_camB_rst);

                /*Racing between CAMA and CAMA using unlocked API to RW*/
                printf("\n\nRacing betweeen CAM_B and CAM_B Test:\n");
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
            printf("RW path err(0x%x)",_case);
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
    printf("##############################\n");
    printf("case 0: CAM\n");
    printf("case 1: DIP\n");
    printf("case 2: UNI\n");
    printf("case 3: CAM's LDVT\n");
    printf("case 4: CAM's kernel ctrl\n");
    printf("select test hw module\n");
    printf("##############################\n");

    char s = getchar();
    HwModoule = (MUINT32)atoi((const char*)&s);

    getchar();

    switch(HwModoule){
        case 0:
        case 1:
        case 2:
            printf("##############################\n");
            printf("case 0: R/W to physical\n");
            printf("case 1: R/W to virtual\n");
            printf("case 3: (Jessy UT test) R/W to physical, directly from isp_drv.cpp\n");
            printf("case 4: WR to virtual and WR to physical via CQ0\n");
            printf("case 5: Concurrent CAM_A and CAM_B register R/W access\n");

            if(HwModoule != 1){
                printf("case 2: wait signal (vsync only so far)\n");
            }
            printf("select path\n");
            printf("##############################\n");
            s = getchar();
            RW_Path = atoi((const char*)&s);
            cq = (E_ISP_CAM_CQ) atoi((const char*)argv[2]);

            printf("test case :HW module:0x%x, RW path:0x%x, CQ=%d\n",HwModoule,RW_Path,cq);
            break;
        default:
            break;
    }

    #if 0 // force config smi larb mmu setting to PA, for ion carve-out
    {

        ISP_DRV_CAM *ptr;
        ISP_LARB_MMU_STRUCT larbInfo;
        int idx = 0;

        printf("#################################################");
        printf("#################################################");
        printf("Force config SMI_LARB to PA temporarily");
        printf("#################################################");
        printf("#################################################");

        ptr = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance(CAM_A,ISP_DRV_CQ_THRE0,0,"Test_IspDrvCam_A");
        if(ptr == NULL){
            printf("rror: CAM_A create fail\n");
            return -1;
        }
        printf("Create ISP_DRV_CAM obj : %p", ptr);

        if(ptr->init("CONFIG_ISP_DMA_PORT") == MFALSE){
            ptr->destroyInstance();
            printf("error: CAM_A init failure\n");
            ptr = NULL;
            return -1;
        }

        larbInfo.regVal = 0;
/*
        larbInfo.LarbNum = 2;
        for (idx = 0; idx < 3; idx++) {
            larbInfo.regOffset = 0x380 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            larbInfo.regOffset = 0xf80 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
        }

        larbInfo.LarbNum = 3;
        for (idx = 0; idx < 5; idx++) {
            larbInfo.regOffset = 0x380 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            larbInfo.regOffset = 0xf80 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
        }

        larbInfo.LarbNum = 5;
        for (idx = 0; idx < 12; idx++) {
            larbInfo.regOffset = 0x380 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            larbInfo.regOffset = 0xf80 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);
        }
*/
        larbInfo.LarbNum = 6;
        for (idx = 0; idx < 25; idx++) {
            larbInfo.regOffset = 0x380 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

            larbInfo.regOffset = 0xf80 + (idx << 2);
            ptr->setDeviceInfo(_SET_LABR_MMU, (MUINT8 *)&larbInfo);

        }

        ptr->uninit("CONFIG_ISP_DMA_PORT");
        ptr->destroyInstance();
        printf("Config SMI LARB done");
    }
    #endif

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
        case 4:
            return DrvKernelCtrl();
            break;
    }
    return 0;
}
