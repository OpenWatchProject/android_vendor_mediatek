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

#define LOG_TAG "Test3AMain"
//
#include <stdio.h>
#include <stdlib.h>
//
#include <mtkcam/utils/std/Log.h>
#include <vector>
#include <utils/String8.h>

#include <IEventIrq.h>

#include <mtkcam/drv/IHalSensor.h>
#include <private/aaa_hal_private.h>
#include <mtkcam/aaa/IHal3A.h>
#include "aaa_result.h"

// #include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>

#if CAM3_FAKE_SENSOR_DRV
#include "fake_sensor_drv/INormalPipe.h"
#else
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#endif

#include <semaphore.h>
#include <utils/threads.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <hardware/camera3.h>
//#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include "aaa_hal_if.h"
#include "aaa_utils.h"

#include <IEventIrq.h>
// #include "../EventIrq/DefaultEventIrq.h"

#include "test_3a_runner.h"
#include "ICaseControl.h"
#include "test_include.h"

//
using namespace std;
using namespace NSCam;
using namespace NS3Av3;
using namespace android;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;

#define MY_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

/*******************************************************************************
*   test IEvenIrq
********************************************************************************/
class TestIrq
{
public:
    TestIrq()
        : m_dTest(0)
        , m_prIrq(NULL)
    {
        MY_LOG("");
    }

    ~TestIrq()
    {
        ::pthread_join(m_Thread, NULL);
        ::sem_destroy(&m_SemDone);
        m_prIrq->destroyInstance(LOG_TAG);
        MY_LOG("-");
    }

    MVOID run()
    {
        MY_LOG("+");
        m_prIrq = IEventIrq::createInstance(IEventIrq::ConfigParam(0, 0, 5000, IEventIrq::E_Event_Vsync), LOG_TAG);
        ::sem_init(&m_SemDone, 0, 0);
        ::pthread_create(&m_Thread, NULL, threadLoop, this);
        ::sem_wait(&m_SemDone);
        MY_LOG("-");
    }

private:
    static MVOID* threadLoop(MVOID* arg)
    {
        TestIrq* _this = reinterpret_cast<TestIrq*>(arg);
        MINT32 i = 0;
        MY_LOG("+");
        while (i < 30)
        {
            MY_LOG("(%d)", i);
            _this->calculate();
            IEventIrq::Duration rDuration;
            MINT32 i4EventCnt = _this->m_prIrq->wait(rDuration);
            MY_LOG("(%d, %d, %d)", i4EventCnt, rDuration.i4Duration0, rDuration.i4Duration1);
            _this->apply();
            i ++;
        }
        MY_LOG("-");
        ::sem_post(&_this->m_SemDone);
        return NULL;
    }

    MINT32 calculate()
    {
        MY_LOG("+");
        MINT32 i;
        MFLOAT dTest = 3e20;
        for (i = 0; i < 50; i++)
        {
            dTest = dTest / ((MFLOAT)i+1);
            MY_LOG("%d:dTest(%f)", i, dTest);
        }
        m_dTest = dTest;
        MY_LOG("- dTest(%f)", dTest);
        return 0;
    }

    MINT32 apply()
    {
        MY_LOG("m_dTest(%f)", m_dTest);
        m_prIrq->mark();
        return 0;
    }

    MFLOAT              m_dTest;
    IEventIrq*          m_prIrq;
    pthread_t           m_Thread;
    sem_t               m_SemDone;
};

/*******************************************************************************
*
********************************************************************************/
static void usage()
{
    MY_LOG("Usage: test_3a_main -s <SCENARIO> -m <MODULE> -t <TEST_CASE>\n");
    MY_LOG("<SCENARIO>: PREVIEW | ZSD_PREVIEW | RECORDING \n");
}

static void TestCamInfo()
{
    // NSIspTuning::RAWIspCamInfo_U rCamInfo;
    // rCamInfo.eIspProfile = NSIspTuning::EIspProfile_Preview;
    // rCamInfo.eSensorMode = NSIspTuning::ESensorMode_Capture;
    // rCamInfo.eIdx_Scene = MTK_CONTROL_SCENE_MODE_DISABLED;
    // rCamInfo.u4ISOValue = 200;
    // rCamInfo.eIdx_ISO = NSIspTuning::eIDX_ISO_1;
    // rCamInfo.eIdx_PCA_LUT = NSIspTuning::eIDX_PCA_MIDDLE;
    // rCamInfo.eIdx_CCM = NSIspTuning::eIDX_CCM_A;
    // rCamInfo.eIdx_Shading_CCT = NSIspTuning::eIDX_Shading_CCT_CWF;
    // rCamInfo.i4ZoomRatio_x100 = 300;
    // rCamInfo.i4LightValue_x10 = 1000;

    // IMetadata m;
    // UPDATE_ENTRY_ARRAY(m, MTK_PROCESSOR_CAMINFO, rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U));

    // NSIspTuning::RAWIspCamInfo_U rCamInfo2;
    // if (GET_ENTRY_ARRAY(m, MTK_PROCESSOR_CAMINFO, rCamInfo2.data, sizeof(NSIspTuning::RAWIspCamInfo_U)))
    // {
    //     MY_LOG("TestCamInfo");
    //     MY_LOG("%d, %d", rCamInfo.eIspProfile, rCamInfo2.eIspProfile);
    //     MY_LOG("%d, %d", rCamInfo.eSensorMode, rCamInfo2.eSensorMode);
    //     MY_LOG("%d, %d", rCamInfo.eIdx_Scene, rCamInfo2.eIdx_Scene);
    //     MY_LOG("%d, %d", rCamInfo.u4ISOValue, rCamInfo2.u4ISOValue);
    //     MY_LOG("%d, %d", rCamInfo.eIdx_ISO, rCamInfo2.eIdx_ISO);
    //     MY_LOG("%d, %d", rCamInfo.eIdx_PCA_LUT, rCamInfo2.eIdx_PCA_LUT);
    //     MY_LOG("%d, %d", rCamInfo.eIdx_CCM, rCamInfo2.eIdx_CCM);
    //     MY_LOG("%d, %d", rCamInfo.eIdx_Shading_CCT, rCamInfo2.eIdx_Shading_CCT);
    //     MY_LOG("%d, %d", rCamInfo.i4ZoomRatio_x100, rCamInfo2.i4ZoomRatio_x100);
    //     MY_LOG("%d, %d", rCamInfo.i4LightValue_x10, rCamInfo2.i4LightValue_x10);
    // }
}

/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char* argv[])
{
    int ret = 0;
    //Test3ARunner* pTest3ARunner = Test3ARunner::createInstance();
    //==========================================================================
    //   Parse arguments
    //==========================================================================
    MY_LOG("Parse arguments");
    String8 scenArg, moduleArg, testArg;
    MUINT32 u4TestArg = 0;
    for (int i = 1; i < argc; i+=2) {
        if (i + 1 < argc){ // Check that we haven't finished parsing already
            if (!strcmp(argv[i], "-s")){
                scenArg = String8(argv[i + 1]);
            } else if (!strcmp(argv[i], "-m")) {
                moduleArg = String8(argv[i + 1]);
            } else if (!strcmp(argv[i], "-t")) {
                testArg = String8(argv[i + 1]);
            } else {
                MY_LOG("Not enough or invalid arguments, please try again.\n");
                usage();
//                goto Exit;
            }
        }
    }
    //
    if (scenArg.isEmpty() || moduleArg.isEmpty()|| testArg.isEmpty())
    {
        printf("arguments is NULL");
        usage();
        goto Exit;
    }
    u4TestArg = atoi(testArg.string());
    MY_LOG("scenario = %s, module = %s, test = %d", scenArg.string(), moduleArg.string(), u4TestArg);
    MY_LOG("3A Framework initialize");
    if (!test3ARunner.initialize()){
        goto Exit;
    }

    //==========================================================================
    //   Prepare Senario Basic Meta
    //==========================================================================
    MY_LOG("Prepare Senario Basic Meta");
    // senario
    if(scenArg == "PREVIEW")
    {
//        test3ARunner.prepareMeta_Set(AAA, case3A.getPrepareMetaFunc_Set(PREVIEW));
//      test3ARunner.prepareMeta_SetIsp(AAA, test3A.getPrepareMetaFunc_SetIsp(PREVIEW));
//        test3ARunner.setUpdateMetaFunc_Set(AAA, case3A.getUpdateMetaFunc_Set(PREVIEW));
//      test3ARunner.updateMetaFunc_SetIsp(AAA, test3A.getUpdateMetaFunc_SetIsp(PREVIEW));
//        test3ARunner.setVerifyFunc_Get(AAA, case3A.getVerifyFunc_Get(PREVIEW));
//      test3ARunner.verifyFunc_SetIsp(AAA, test3A.getVerifyFunc_SetIsp(PREVIEW));
    } else if(scenArg == "ZSD_PREVIEW")
    {
    } else if(scenArg == "RECORDING")
    {
    } else
    {
        // not support
        goto Exit;
    }

    //==========================================================================
    //   Start Test
    //==========================================================================

    MY_LOG("Start Test");
    if(moduleArg == "3A")
    {
        switch(u4TestArg)
        {
            RUN_TEST_3A(TEST_3A_0);
            RUN_TEST_3A(TEST_3A_1);
            RUN_TEST_3A(TEST_3A_2);
            default:
                // not support
                goto Exit;
        }
    }
    else if(moduleArg == "AF")
    {
        switch(u4TestArg)
        {
            RUN_TEST_AF(TEST_AF_0);
            RUN_TEST_AF(TEST_AF_1);
            RUN_TEST_AF(TEST_AF_2);
            default:
                // not support
                goto Exit;
        }
    }
    else
    {
        // not support
        goto Exit;
    }

    // ::usleep(10000000);
    MY_LOG("Test finish, 3A Framework destroy");
    test3ARunner.destroy();

    //==========================================================================
    //   Test Result
    //==========================================================================
    MY_LOG("Test Result");
    if(!test3ARunner.pass())
    {
        MY_LOG("Test Fail");
        MY_LOG("%s", test3ARunner.failMsg());
    }
    else
    {
        MY_LOG("Test Pass");
    }


    // DefaultEventIrq*          mpEventIrq;
    // IEventIrq::ConfigParam IrqConfig(1, 0, 5000000, IEventIrq::E_Event_Vsync);
    // mpEventIrq = DefaultEventIrq::createInstance(IrqConfig, "museIrq");

    // MY_LOG("start run", __FUNCTION__);
    // ::usleep(1000);
    // IEventIrq::Duration duration;
    // for(int i=0;i<10;i++)
    // {
    //     MY_LOG("wait %d", i);
    //     mpEventIrq->wait(duration);
    // }

    // MY_LOG("destroy", __FUNCTION__);

    // mpEventIrq->destroyInstance("museIrq");
    // mpEventIrq = NULL;
    //
/*
     if (0)  // Dump All MetaData
    {
        IMetadataTagSet const &mtagInfo = IDefaultMetadataTagSet::singleton()->getTagSet();
        sp<IMetadataConverter> mMetaDataConverter = IMetadataConverter::createInstance(mtagInfo);

        MY_LOGD("\n\nDump AppMeta:\n");
        MY_LOGD("==========================================================\n");
        IMetadata* pMetadata = pAppMetaControlSB->tryReadLock(LOG_TAG);
        pMetadata->dump();
        MY_LOGD("==========================================================\n");
        mMetaDataConverter->dumpAll(*pMetadata);
        pAppMetaControlSB->unlock(LOG_TAG, pMetadata);
        MY_LOGD("==========================================================\n");
        MY_LOGD("\n\nDump HalMeta:\n");
        MY_LOGD("==========================================================\n");
        pMetadata = pHalMetaControlSB->tryReadLock(LOG_TAG);
        pMetadata->dump();
//        MY_LOGD("==========================================================\n");
//        mMetaDataConverter->dumpAll(*pMetadata);  // Not Support
        pHalMetaControlSB->unlock(LOG_TAG, pMetadata);
        MY_LOGD("==========================================================\n");
    }
*/
    //

    return ret;

Exit:
    MY_LOG("Test abort because of any problem");
    MY_LOG("3A Framework destroy");
//    test3ARunner.destroy();
    ret = -1;

    return ret;
}

