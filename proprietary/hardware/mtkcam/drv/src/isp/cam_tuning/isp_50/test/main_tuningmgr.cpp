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

#include <tuning_mgr.h>
#include <tuning_drv.h>


#undef LOG_TAG
#define LOG_TAG "TuningMgrTest"

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/


 //-----------------------------------------------------------------------------
#include <cutils/properties.h>              // For property_get().

#undef  DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define DBG_LOG_TAG     LOG_TAG
#include "drv_log.h"                    // Note: DBG_LOG_TAG will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(tuning_mgr_test);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (tuning_mgr_test_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (tuning_mgr_test_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
//#define LOG_DBG(fmt, arg...)        do { if (tuning_mgr_DbgLogEnable_INFO  ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (tuning_mgr_test_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (tuning_mgr_test_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (tuning_mgr_test_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (tuning_mgr_test_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
//-----------------------------------------------------------------------------


//using namespace NSCam;

void TuningMgr_DeEnQue(TuningMgr* tuningMgr, MINT32* magicNum, MINT32 BQNum, MBOOL* isVerify = NULL){
    if(NULL == isVerify)
        tuningMgr->dequeBuffer(magicNum);
    else
        tuningMgr->dequeBuffer(magicNum, isVerify);

    // Update tuning node
    MUINT32 uMagicN = (MUINT32)magicNum[0];
    //
    for(int i = 0; i < BQNum; i++)
    {
     MBOOL ctlEn = MTRUE;

     // Update OBC
     tuningMgr->updateEngine(eTuningMgrFunc_OBC, ctlEn, i);

     // Test tuningMgrWriteReg
     tuningMgr->tuningMgrWriteReg(0x600, uMagicN + 1, i); // CAM_REG_OBC_GAIN0
     tuningMgr->tuningMgrWriteReg(0x604, uMagicN + 2, i); // CAM_REG_OBC_GAIN1
     tuningMgr->tuningMgrWriteReg(0x608, uMagicN + 3, i); // CAM_REG_OBC_GAIN2
     tuningMgr->tuningMgrWriteReg(0x60C, uMagicN + 4, i); // CAM_REG_OBC_GAIN3



     // Update DBS
     tuningMgr->updateEngine(eTuningMgrFunc_DBS, ctlEn, i);
     TUNING_MGR_REG_IO_STRUCT regs[5] = {{0xB40, uMagicN + 5},
                                         {0xB44, uMagicN + 6},
                                         {0xB48, uMagicN + 7},
                                         {0xB4C, uMagicN + 8},
                                         {0xB50, uMagicN + 9}};

     tuningMgr->tuningMgrWriteRegs(regs, 5, i);


     // Test for over range
     tuningMgr->tuningMgrWriteReg(0x4612, uMagicN + 0xFF, i);

     // Update FLK
     tuningMgr->updateEngine(eTuningMgrFunc_FLK, ctlEn, i);
     TUNING_MGR_REG_IO_STRUCT regs2[3] = {{0x534, uMagicN + 0xA},
                                          {0x538, uMagicN + 0xB},
                                          {0x53C, uMagicN + 0xC}};

     tuningMgr->tuningMgrWriteRegs_Uni(regs2, 3, i);


    }

    tuningMgr->enqueBuffer();

}

MBOOL TuningDrv_DeEnQue(TuningDrv* p1TuningDrvObj, ETuningDrvUser p1TuningDrvUser, MINT32* magicNum)
{
    MBOOL ret = MTRUE;
    vector <stTuningNode*> vpTuningNodes;
    MUINT32 regValue = 0x0;
    MUINT32 addr;
    MUINT32 shift;

    ret = p1TuningDrvObj->deTuningQueByDrv(p1TuningDrvUser, magicNum, vpTuningNodes);
    if(ret == MFALSE)
    {
        LOG_ERR("ERROR!!! deTuningQueByDrv fail!!!, magic#(0x%x)\n", magicNum[0]);
        return ret;
    }

    ret = p1TuningDrvObj->enTuningQueByDrv(p1TuningDrvUser);

    // verify tuning node data
    for(int i = 0; i < p1TuningDrvObj->getBQNum(); i++)
    {
       // Check OBC
       shift = 0;
       for(int j = 0; j < 4; j++)
       {
           addr = 0x600 + j * 4;
           regValue = vpTuningNodes[i]->pTuningRegBuf[(addr + shift) >> 2];
           LOG_INF("0x4%3x(0x%x)\n", 0x600 + j * 4, regValue);
           if(regValue != magicNum[0] + j + 1)
           {
               LOG_ERR("Error!! 0x4%3x(0x%x) should be (0x%x)\n", 0x600 + j * 4, regValue, magicNum[0] + j + 1);
               ret = MFALSE;
               return ret;
           }
       }

       // Check DBS
       for(int j = 0; j < 5; j++)
       {
           addr = 0xB40 + j * 4;
           regValue = vpTuningNodes[i]->pTuningRegBuf[(addr + shift) >> 2];
           LOG_INF("0x4%3x(0x%x)\n", 0xB40 + j * 4, regValue);
           if(regValue != magicNum[0] + j + 5)
           {
               LOG_ERR("Error!! 0x4%3x(0x%x) should be (0x%x)\n", 0xB40 + j * 4, regValue, magicNum[0] + j + 5);
               ret = MFALSE;
               return ret;
           }
       }

       shift = 0x1000;
       // Check FLK
       for(int j = 0; j < 3; j++)
       {
           addr = 0x534 + j * 4;
           regValue = vpTuningNodes[i]->pTuningRegBuf[(addr + shift) >> 2];
           LOG_INF("0x3%3x(0x%x)\n", 0x534 + j * 4, regValue);
           if(regValue != magicNum[0] + j + 0xA)
           {
               LOG_ERR("Error!! 0x3%3x(0x%x) should be (0x%x)\n", 0x534 + j * 4, regValue, magicNum[0] + j + 0xA);
               ret = MFALSE;
               return ret;
           }
       }
    }

    return ret;
}

MBOOL TestCase_01(){
    MBOOL ret = true;

    printf("+Test case 1: Support pass1 tuning data\n");

    int sensorIndex = 0; // Test Main cam
    char* userName = "Jessy Test";
    int BQNum = 1;  // Default value is 1
    MINT32 magicNum[1] = {0};
    MUINT32 regValue = 0x0;

    TuningMgr* tuningMgr = TuningMgr::getInstance(sensorIndex);

    tuningMgr->init(userName, BQNum);

    // deque BQNum tuning nodes at once
    tuningMgr->dequeBuffer(magicNum);

    // Update tuning node
    //
    for(int i = 0; i < BQNum; i++)
    {
        MBOOL ctlEn = MTRUE;
        tuningMgr->updateEngine(eTuningMgrFunc_OBC, ctlEn, i);
        //tuningMgr->tuningMgrWriteReg(0x600, 0x123, i); // Only for test, need modify this. CAM_REG_OBC_GAIN0

        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_OBC_GAIN0, magicNum[0] + 1, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_OBC_GAIN1, magicNum[0] + 2, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_OBC_GAIN2, magicNum[0] + 3, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_OBC_GAIN3, magicNum[0] + 4, i);
    }

    // Read tuning node
    //
    for(int i = 0; i < BQNum; i++)
    {
        regValue = TUNING_MGR_READ_REG_CAM(tuningMgr, CAM_OBC_GAIN0, i);
        printf("CAM_REG_OBC_GAIN0(0x%x)\n", regValue);
        if(regValue != magicNum[0] + 1)
        {
            printf("Error!! CAM_REG_OBC_GAIN0(0x%x) should be (0x%x)\n", regValue, magicNum[0] + 1);
            ret = MFALSE;
            goto EXIT;
        }
        regValue = TUNING_MGR_READ_REG_CAM(tuningMgr, CAM_OBC_GAIN1, i);
        printf("CAM_REG_OBC_GAIN1(0x%x)\n", regValue);
        if(regValue != magicNum[0] + 2)
        {
            printf("Error!! CAM_REG_OBC_GAIN1(0x%x) should be (0x%x)\n", regValue, magicNum[0] + 2);
            ret = MFALSE;
            goto EXIT;
        }
        regValue = TUNING_MGR_READ_REG_CAM(tuningMgr, CAM_OBC_GAIN2, i);
        printf("CAM_REG_OBC_GAIN2(0x%x)\n", regValue);
        if(regValue != magicNum[0] + 3)
        {
            printf("Error!! CAM_REG_OBC_GAIN2(0x%x) should be (0x%x)\n", regValue, magicNum[0] + 3);
            ret = MFALSE;
            goto EXIT;
        }
        regValue = TUNING_MGR_READ_REG_CAM(tuningMgr, CAM_OBC_GAIN3, i);
        printf("CAM_REG_OBC_GAIN3(0x%x)\n", regValue);
        if(regValue != magicNum[0] + 4)
        {
            printf("Error!! CAM_REG_OBC_GAIN3(0x%x) should be (0x%x)\n", regValue, magicNum[0] + 4);
            ret = MFALSE;
            goto EXIT;
        }
    }

    tuningMgr->enqueBuffer();

EXIT:
    tuningMgr->uninit(userName);

    printf("===================Test case 1===================\n");
    if(ret == MTRUE)
    {
        printf("Success!!\n");
    }
    else
    {
        printf("Fail!!\n");
    }
    printf("===================Test case 1===================\n");

    printf("-Test case 1: Support pass1 tuning data\n");

    return ret;

}

MBOOL TestCase_02()
{

    MBOOL ret = true;

    printf("+Test case 2: Tuning manager enq/deq with single buffer flow\n");

    int sensorIndex = 0; // Test Main cam
    char* userName = "Jessy Test";
    int BQNum = 1;  // Default value is 1
    MINT32 magicNum[1] = {0};
    MUINT32 regValue = 0x0;

    TuningMgr* tuningMgr = (TuningMgr*)TuningMgr::getInstance(sensorIndex);

    tuningMgr->init(userName, BQNum);

#if 0
    // deque BQNum tuning nodes at once
    tuningMgr->dequeBuffer(magicNum);

    // Update tuning node
    //
    for(int i = 0; i < BQNum; i++)
    {
       MBOOL ctlEn = MTRUE;

       // Update OBC
       tuningMgr->updateEngine(eTuningMgrFunc_OBC, ctlEn, i);

       // Test tuningMgrWriteReg
       tuningMgr->tuningMgrWriteReg(0x600, magicNum[0] + 1, i); // CAM_REG_OBC_GAIN0
       tuningMgr->tuningMgrWriteReg(0x604, magicNum[0] + 2, i); // CAM_REG_OBC_GAIN1
       tuningMgr->tuningMgrWriteReg(0x608, magicNum[0] + 3, i); // CAM_REG_OBC_GAIN2
       tuningMgr->tuningMgrWriteReg(0x60C, magicNum[0] + 4, i); // CAM_REG_OBC_GAIN3



       // Update DBS
       tuningMgr->updateEngine(eTuningMgrFunc_DBS, ctlEn, i);
       TUNING_MGR_REG_IO_STRUCT regs[5] = {{0xB40, magicNum[0] + 5},
                                           {0xB44, magicNum[0] + 6},
                                           {0xB48, magicNum[0] + 7},
                                           {0xB4C, magicNum[0] + 8},
                                           {0xB50, magicNum[0] + 9}};

       tuningMgr->tuningMgrWriteRegs(regs, 5, i);


       // Test for over range
       tuningMgr->tuningMgrWriteReg(0x4612, magicNum[0] + 0xFF, i);

       // Update FLK
       tuningMgr->updateEngine(eTuningMgrFunc_FLK, ctlEn, i);
       TUNING_MGR_REG_IO_STRUCT regs2[3] = {{0x534, magicNum[0] + 0xA},
                                            {0x538, magicNum[0] + 0xB},
                                            {0x53C, magicNum[0] + 0xC}};

       tuningMgr->tuningMgrWriteRegs_Uni(regs2, 3, i);


    }
#endif

    TuningMgr_DeEnQue(tuningMgr, magicNum, BQNum);

    // Read tuning node
    //
    for(int i = 0; i < BQNum; i++)
    {
       // Check OBC
       for(int j = 0; j < 4; j++)
       {
           regValue = tuningMgr->tuningMgrReadReg(0x600 + j * 4,i);
           printf("0x4%3x(0x%x)\n", 0x600 + j * 4, regValue);
           if(regValue != magicNum[0] + j + 1)
           {
               printf("Error!! 0x4%3x(0x%x) should be (0x%x)\n", 0x600 + j * 4, regValue, magicNum[0] + j + 1);
               ret = MFALSE;
               goto EXIT;
           }
       }

       // Check DBS
       for(int j = 0; j < 5; j++)
       {
           regValue = tuningMgr->tuningMgrReadReg(0xB40 + j * 4,i);
           printf("0x4%3x(0x%x)\n", 0xB40 + j * 4, regValue);
           if(regValue != magicNum[0] + j + 5)
           {
               printf("Error!! 0x4%3x(0x%x) should be (0x%x)\n", 0xB40 + j * 4, regValue, magicNum[0] + j + 5);
               ret = MFALSE;
               goto EXIT;
           }
       }

       // Test for over range
       regValue = tuningMgr->tuningMgrReadReg(0x4612,i);
       printf("0x4612(0x%x)\n", regValue);

       // Check FLK
       for(int j = 0; j < 3; j++)
       {
           regValue = tuningMgr->tuningMgrReadReg_Uni(0x534 + j * 4,i);
           printf("0x3%3x(0x%x)\n", 0x534 + j * 4, regValue);
           if(regValue != magicNum[0] + j + 0xA)
           {
               printf("Error!! 0x3%3x(0x%x) should be (0x%x)\n", 0x534 + j * 4, regValue, magicNum[0] + j + 0xA);
               ret = MFALSE;
               goto EXIT;
           }
       }


    }

EXIT:
    tuningMgr->uninit(userName);



    printf("===================Test case 2===================\n");
    if(ret == MTRUE)
    {
        printf("Success!!\n");
    }
    else
    {
        printf("Fail!!\n");
    }
    printf("===================Test case 2===================\n");

    printf("-Test case 2: Tuning manager enq/deq with single buffer flow\n");
    return ret;
}

int TestCase_03()
{
    int ret = MTRUE;
    printf("+Test case 3: Tuning manager enq/deq with multiple buffer flow\n");


    int sensorIndex = 0; // Test Main cam
    char* userName = "Jessy Test";
    int BQNum = 1;  // Default value is 1
    MINT32 magicNum[1] = {0};
    MUINT32 regValue = 0x0;
    int i = 0;
    int totalQueueTime = 36;

    TuningMgr* tuningMgr = (TuningMgr*)TuningMgr::getInstance(sensorIndex);

    tuningMgr->init(userName, BQNum);

    for(i = 0; i < totalQueueTime; i++)
    {
        magicNum[0] = i;
        MUINT32 uMagicN = (MUINT32)magicNum[0];

        printf("curTime: %d, magic#(%d)\n", i, magicNum[0]);

        // deque BQNum tuning nodes at once
        tuningMgr->dequeBuffer(magicNum);

        // Update tuning node
        //
        for(int i = 0; i < BQNum; i++)
        {
           MBOOL ctlEn = MTRUE;

           // Update OBC
           tuningMgr->updateEngine(eTuningMgrFunc_OBC, ctlEn, i);

           // Test tuningMgrWriteReg
           tuningMgr->tuningMgrWriteReg(0x600, uMagicN + 1, i); // CAM_REG_OBC_GAIN0
           tuningMgr->tuningMgrWriteReg(0x604, uMagicN + 2, i); // CAM_REG_OBC_GAIN1
           tuningMgr->tuningMgrWriteReg(0x608, uMagicN + 3, i); // CAM_REG_OBC_GAIN2
           tuningMgr->tuningMgrWriteReg(0x60C, uMagicN + 4, i); // CAM_REG_OBC_GAIN3



           // Update DBS
           tuningMgr->updateEngine(eTuningMgrFunc_DBS, ctlEn, i);
           TUNING_MGR_REG_IO_STRUCT regs[5] = {{0xB40, uMagicN + 5},
                                               {0xB44, uMagicN + 6},
                                               {0xB48, uMagicN + 7},
                                               {0xB4C, uMagicN + 8},
                                               {0xB50, uMagicN + 9}};

           tuningMgr->tuningMgrWriteRegs(regs, 5, i);


           // Test for over range
           tuningMgr->tuningMgrWriteReg(0x4612, uMagicN + 0xFF, i);

           // Update FLK
           tuningMgr->updateEngine(eTuningMgrFunc_FLK, ctlEn, i);
           TUNING_MGR_REG_IO_STRUCT regs2[3] = {{0x534, uMagicN + 0xA},
                                                {0x538, uMagicN + 0xB},
                                                {0x53C, uMagicN + 0xC}};

           tuningMgr->tuningMgrWriteRegs_Uni(regs2, 3, i);


        }

        tuningMgr->enqueBuffer();

        // Read tuning node
        //
        for(int i = 0; i < BQNum; i++)
        {
           // Check OBC
           for(int j = 0; j < 4; j++)
           {
               regValue = tuningMgr->tuningMgrReadReg(0x600 + j * 4,i);
               printf("0x4%3x(0x%x)\n", 0x600 + j * 4, regValue);
               if(regValue != magicNum[0] + j + 1)
               {
                   printf("Error!! 0x4%3x(0x%x) should be (0x%x)\n", 0x600 + j * 4, regValue, magicNum[0] + j + 1);
                   ret = MFALSE;
                   goto EXIT;
               }
           }

           // Check DBS
           for(int j = 0; j < 5; j++)
           {
               regValue = tuningMgr->tuningMgrReadReg(0xB40 + j * 4,i);
               printf("0x4%3x(0x%x)\n", 0xB40 + j * 4, regValue);
               if(regValue != magicNum[0] + j + 5)
               {
                   printf("Error!! 0x4%3x(0x%x) should be (0x%x)\n", 0xB40 + j * 4, regValue, magicNum[0] + j + 5);
                   ret = MFALSE;
                   goto EXIT;
               }
           }

           // Test for over range
           regValue = tuningMgr->tuningMgrReadReg(0x4612,i);
           printf("0x4612(0x%x)\n", regValue);

           // Check FLK
           for(int j = 0; j < 3; j++)
           {
               regValue = tuningMgr->tuningMgrReadReg_Uni(0x534 + j * 4,i);
               printf("0x3%3x(0x%x)\n", 0x534 + j * 4, regValue);
               if(regValue != magicNum[0] + j + 0xA)
               {
                   printf("Error!! 0x3%3x(0x%x) should be (0x%x)\n", 0x534 + j * 4, regValue, magicNum[0] + j + 0xA);
                   ret = MFALSE;
                   goto EXIT;
               }
           }


        }



        usleep(30 * 1000); // sleep for 30ms
    }

EXIT:
    tuningMgr->uninit(userName);



    printf("===================Test case 3===================\n");
    if(ret == MTRUE)
    {
       printf("Success!!\n");
    }
    else
    {
       printf("Fail!!\n");
    }
    printf("===================Test case 3===================\n");

    printf("-Test case 3: Tuning manager enq/deq with multiple buffer flow\n");

    return ret;
}

int TestCase_04()
{
    int ret = 0;
    printf("+Test case 4: Tuning drv enq/deq with multiple buffer flow\n");


    int sensorIndex = 0; // Test Main cam
    char* userName = "Jessy Test";
    MINT32 BQNum = 1;  // Default value is 1
    MINT32 magicNum[1] = {0};
    MUINT32 regValue = 0x0;
    TuningDrv* p1TuningDrvObj;
    ETuningDrvUser p1TuningDrvUser = eTuningDrvUser_3A;

    TuningMgr* tuningMgr = (TuningMgr*)TuningMgr::getInstance(sensorIndex);
    p1TuningDrvObj = (TuningDrv*)TuningDrv::getInstance(sensorIndex);

    tuningMgr->init(userName, BQNum);

    printf("=================================================\n");
    printf("========1st time enque/deque test start!!========\n");

    /* Tuning user enque/deque*/
    magicNum[0] = 0; //magic# = 0
    TuningMgr_DeEnQue(tuningMgr, magicNum, BQNum);

    magicNum[0] = 0; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE)
        goto EXIT;

    printf("========1st time enque/deque test end!!========\n");

    printf("=================================================\n");
    printf("========2st time enque/deque test start!!========\n");

    /* Tuning user enque/deque*/
    for(int i = 0; i < 15; i++)
    {
        magicNum[0] = i + 1; //magic# = 1 ~ 15
        TuningMgr_DeEnQue(tuningMgr, magicNum, BQNum);
    }

    magicNum[0] = 1; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE)
        goto EXIT;

    magicNum[0] = 5; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE)
        goto EXIT;

    magicNum[0] = 14; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE)
        goto EXIT;

    magicNum[0] = 15; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE)
        goto EXIT;

    /* Test cannot find this node*/
    magicNum[0] = 16; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE)
        goto EXIT;

    /* Test cannot find this node*/
    magicNum[0] = 0; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE)
        goto EXIT;

    printf("========2st time enque/deque test end!!========\n");


EXIT:
    tuningMgr->uninit(userName);

    printf("===================Test case 4===================\n");
    if(ret == MTRUE)
    {
        printf("Success!!\n");
    }
    else
    {
        printf("Fail!!\n");
    }
    printf("===================Test case 4===================\n");

    printf("-Test case 4: Tuning drv enq/deq with multiple buffer flow\n");
    return ret;
}

int TestCase_05()
{
    int ret = 0;
    printf("+Test case 5: User can decide which modules should apply tuning data\n");

    printf("-Test case 5: User can decide which modules should apply tuning data\n");
    return ret;
}

int TestCase_06()
{
    int ret = 0;
    printf("+Test case 6: Verify the timing of tuning parameters set to HW with multiple tuning paramters\n");

    printf("-Test case 6: Verify the timing of tuning parameters set to HW with multiple tuning paramters\n");

    return ret;
}

int TestCase_07()
{
    int ret = 0;
    printf("+Test case 7: magic number validation\n");

    printf("-Test case 7: magic number validation\n");
    return ret;
}


int TestCase_08()
{
    int ret = 0;
    printf("+Test case 8: Support all possible path\n");

for(int index = 0; index < NTUNING_MAX_SENSOR_CNT; index++)
{

    int sensorIndex = index; // Test Main cam
    char* userName = "Jessy Test";
    MINT32 BQNum = 1;  // Default value is 1
    MINT32 magicNum[1] = {0};
    MUINT32 regValue = 0x0;
    TuningDrv* p1TuningDrvObj;
    ETuningDrvUser p1TuningDrvUser = eTuningDrvUser_3A;

    TuningMgr* tuningMgr = (TuningMgr*)TuningMgr::getInstance(sensorIndex);
    p1TuningDrvObj = (TuningDrv*)TuningDrv::getInstance(sensorIndex);

    tuningMgr->init(userName, BQNum);

    printf("=================================================\n");
    printf("========1st time enque/deque test start!!========\n");
    LOG_INF("=================================================\n");
    LOG_INF("========1st time enque/deque test start!!========\n");

    /* Tuning user enque/deque*/
    magicNum[0] = 0; //magic# = 0
    TuningMgr_DeEnQue(tuningMgr, magicNum, BQNum);

    magicNum[0] = 0; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE) {
        tuningMgr->uninit(userName);
        goto EXIT;
    }

    printf("========1st time enque/deque test end!!========\n");

    printf("=================================================\n");
    printf("========2st time enque/deque test start!!========\n");
    LOG_INF("========1st time enque/deque test end!!========\n");

    LOG_INF("=================================================\n");
    LOG_INF("========2st time enque/deque test start!!========\n");

    /* Tuning user enque/deque*/
    for(int i = 0; i < 15; i++)
    {
        magicNum[0] = i + 1; //magic# = 1 ~ 15
        TuningMgr_DeEnQue(tuningMgr, magicNum, BQNum);
    }

    magicNum[0] = 1; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE) {
        tuningMgr->uninit(userName);
        goto EXIT;
    }

    magicNum[0] = 5; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE) {
        tuningMgr->uninit(userName);
        goto EXIT;
    }

    magicNum[0] = 14; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE) {
        tuningMgr->uninit(userName);
        goto EXIT;
    }

    magicNum[0] = 15; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE) {
        tuningMgr->uninit(userName);
        goto EXIT;
    }

    /* Test cannot find this node*/
    magicNum[0] = 16; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE) {
        tuningMgr->uninit(userName);
        goto EXIT;
    }

    /* Test cannot find this node*/
    magicNum[0] = 0; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE) {
        tuningMgr->uninit(userName);
        goto EXIT;
    }

    printf("========2st time enque/deque test end!!========\n");
    LOG_INF("========2st time enque/deque test end!!========\n");


    tuningMgr->uninit(userName);
}
EXIT:
    printf("===================Test case 8===================\n");
    if(ret == MTRUE)
    {
        printf("Success!!\n");
        LOG_INF("Success!!\n");
    }
    else
    {
        printf("Fail!!\n");
        LOG_INF("Fail!!\n");
    }
    printf("===================Test case 8===================\n");

    printf("-Test case 8: Support all possible path\n");
    return ret;
}



int TestCase_09()
{
    int ret = MTRUE;
    printf("+Test case 9: Test dump tuning info\n");
    LOG_INF("+Test case 9: Test dump tuning info\n");

    int sensorIndex = 0; // Test Main cam
    char* userName = "Jessy Test";
    MINT32 BQNum = 1;  // Default value is 1
    MINT32 magicNum[1] = {0};
    MUINT32 regValue = 0x0;
    TuningDrv* p1TuningDrvObj;
    ETuningDrvUser p1TuningDrvUser = eTuningDrvUser_3A;
    MINT32 key = 12345678;
    MBOOL isVerify[1] = {MTRUE};

    TuningMgr* tuningMgr = (TuningMgr*)TuningMgr::getInstance(sensorIndex);
    p1TuningDrvObj = (TuningDrv*)TuningDrv::getInstance(sensorIndex);

    tuningMgr->init(userName, BQNum);

    printf("=================================================\n");
    printf("========1st time enque/deque test start!!========\n");
    LOG_INF("=================================================\n");
    LOG_INF("========1st time enque/deque test start!!========\n");

    /* Tuning user enque/deque*/
    magicNum[0] = 0; //magic# = 0
    TuningMgr_DeEnQue(tuningMgr, magicNum, BQNum, isVerify);


    // Cannot find this tuning node yet
    if(tuningMgr->dumpTuningInfo(0,key)){
        ret = MFALSE;
        goto EXIT;
    }

    magicNum[0] = 0; //magic# = 0
    /* p1 drv enque/deque */
    ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
    if(ret == MFALSE)
        goto EXIT;


    // Can find this tuning node yet
    ret = tuningMgr->dumpTuningInfo(0,key + 1);
    if(ret == MFALSE)
        goto EXIT;

    printf("========1st time enque/deque test end!!========\n");

    printf("=================================================\n");
    printf("========2st time enque/deque test start!!========\n");
    LOG_INF("========1st time enque/deque test end!!========\n");

    LOG_INF("=================================================\n");
    LOG_INF("========2st time enque/deque test start!!========\n");

    /* Tuning user enque/deque*/
    for(int i = 0; i < 16; i++)
    {
        magicNum[0] = i + 1; //magic# = 1 ~ 16
        TuningMgr_DeEnQue(tuningMgr, magicNum, BQNum, isVerify);

        ret = TuningDrv_DeEnQue(p1TuningDrvObj, p1TuningDrvUser, magicNum);
        if(ret == MFALSE)
            goto EXIT;
    }


    // Cannot find this tuning node: 1
    if(tuningMgr->dumpTuningInfo(1,key)){
        ret = MFALSE;
        goto EXIT;
    }




    // Can find this tuning node: 2
    ret = tuningMgr->dumpTuningInfo(2,key);
    if(ret == MFALSE)
        goto EXIT;




    // Can find this tuning node: 16
    ret = tuningMgr->dumpTuningInfo(16,key);
    if(ret == MFALSE)
        goto EXIT;


    // Can find this tuning node: 16
    ret = tuningMgr->dumpTuningInfo(10,key);
    if(ret == MFALSE)
        goto EXIT;


    // Cannot find this tuning node: 0
    if(tuningMgr->dumpTuningInfo(0,key)){
        ret = MFALSE;
        goto EXIT;
    }



    printf("========2st time enque/deque test end!!========\n");
    LOG_INF("========2st time enque/deque test end!!========\n");

EXIT:

    tuningMgr->uninit(userName);

    printf("===================Test case 8===================\n");
    if(ret == MTRUE)
    {
        printf("Success!!\n");
        LOG_INF("Success!!\n");
    }
    else
    {
        printf("Fail!!\n");
        LOG_INF("Fail!!\n");
    }
    printf("===================Test case 8===================\n");

    printf("-Test case 9: Test dump tuning info\n");
    LOG_INF("-Test case 9: Test dump tuning info\n");
    return ret;
}


int Test_TuningMgr(int argc, char** argv)
{
    MUINT32 Tuning_Path;
    int s;

    (void)argc;(void)argv;
    printf("select test case\n");
    printf("##############################\n");
    printf("Test case 1: Support pass1 tuning data\n");
    printf("Test case 2: Tuning manager enq/deq with single buffer flow\n");
    printf("Test case 3: Tuning manager enq/deq with multiple buffer flow\n");
    printf("Test case 4: Tuning drv enq/deq with multiple buffer flow\n");
    printf("(Cancel)Test case 5: User can decide which modules should apply tuning data\n");
    printf("Test case 6: Verify the timing of tuning parameters set to HW with multiple tuning paramters\n");
    printf("(Cancel)Test case 7: magic number validation\n");
    printf("Test case 8: Support all possible path\n");
    printf("Test case 9: Test dump tuning info\n");
    printf("##############################\n");

    s = getchar();
    Tuning_Path = atoi((const char*)&s);


    printf("test case: Tuning_Path:0x%x \n", Tuning_Path);


    switch(Tuning_Path){
        case 1:
            return TestCase_01();
            break;
        case 2:
            return TestCase_02();
            break;
        case 3:
            return TestCase_03();
            break;
        case 4:
            return TestCase_04();
            break;
        case 5:
            return TestCase_05();
            break;
        case 6:
            return TestCase_06();
            break;
        case 7:
            return TestCase_07();
            break;
        case 8:
            return TestCase_08();
            break;
        case 9:
            return TestCase_09();
            break;
        default:
            break;
    }
    return 0;
}
