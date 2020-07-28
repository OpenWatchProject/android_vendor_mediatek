/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <ctime>
#include <thread>

#include "testCaseCmn.h"
#include "multiCase.h"
#include "vpu_platform.h"

#define MAX_MULTICORE_TEST_LOOP 100000000
#define MAX_MULTICORE_TEST_THREAD 32

#define VPU_CORE_SUPPORT 2
#define DYNAMIC_LOAD_PATH_CORE0 "/data/local/tmp/vpuDynamicLoadAlgo/core0/vpu_dynamic_algo_test"
#define DYNAMIC_LOAD_PATH_CORE1 "/data/local/tmp/vpuDynamicLoadAlgo/core1/vpu_dynamic_algo_test"
#define DYNAMIC_ALGO_NAME "user_helloInVpu"

void testThread(int inCores, int inLoops)
{
    unsigned int delayMs = 0;
    class vpuTestMultiCore * test = new vpuTestMultiCore(inCores);

    srand(time(NULL));
    delayMs = rand() % 1000; // max 1s

    if(test->execute(inLoops, delayMs) == false)
    {
        MY_LOGE("test fail");
    }
    MY_LOGI("test ok");

    delete test;
}

int main(int argc, char **argv)
{
    int inCores = 0, inThreads = 0, inLoops = 0;
    int i = 0;
    std::vector<std::thread *> threadList;

    /* check arguments */
    if(argc != 4)
    {
        MY_LOGE("test template: <program> <#coreNum> <#thread> <#loops>");
        return -1;
    }

    /* get arguments */
    if(argv[1] != 0)
    {
        inCores = atoi(argv[1]);
    }
    if(argv[2] != 0)
    {
        inThreads = atoi(argv[2]);
    }
    if(argv[3] != 0)
    {
        inLoops = atoi(argv[3]);
    }

    /* check arguments from user */
    MY_LOGI("Dual core test: %d cores, %d thread, %d loops", inCores, inThreads, inLoops);
    if(inThreads <= 0 || inLoops <= 0 || inCores > MAX_VPU_CORE_NUM)
    {
        MY_LOGE("<#thread> <#loops> should be larger than 0, cores should be smaller than %d", MAX_VPU_CORE_NUM);
        return -1;
    }

    /* check argument for boundary */
    if(inLoops > MAX_MULTICORE_TEST_LOOP)
    {
        MY_LOGI("test #loop > %d, apply %d to test", MAX_MULTICORE_TEST_LOOP, MAX_MULTICORE_TEST_LOOP);
        inLoops = MAX_MULTICORE_TEST_LOOP;
    }
    if(inThreads > MAX_MULTICORE_TEST_THREAD)
    {
        MY_LOGI("test #thread > %d, apply %d to test", MAX_MULTICORE_TEST_THREAD, MAX_MULTICORE_TEST_THREAD);
        inThreads = MAX_MULTICORE_TEST_THREAD;
    }

    for(i=0; i<inThreads; i++)
    {
        auto t = new std::thread(testThread, inCores, inLoops);
        threadList.push_back(t);
    }

    for(i=0; i<inThreads; i++)
    {
        threadList.at(i)->join();
    }

    return 0;
}
