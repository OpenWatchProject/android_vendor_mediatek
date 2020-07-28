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
 
#include <vector>
#include <list>
#include <queue>

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <fcntl.h>

#include <semaphore.h>
#include <pthread.h>
#include <utils/threads.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
using namespace NSCam;
using namespace NSCam::Utils;
using namespace android;
using namespace std;

#include <mtkcam/def/common.h>

#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>


using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;

#define THIS_NAME       "SttpipeUT"

#define LOG_VRB(fmt, arg...)        printf(fmt "\n", ##arg)
#define LOG_DBG(fmt, arg...)        printf(fmt "\n", ##arg)
#define LOG_INF(fmt, arg...)        printf(fmt "\n", ##arg)
#define LOG_WRN(fmt, arg...)        printf(fmt "\n", ##arg)
#define LOG_ERR(fmt, arg...)        printf(fmt "\n", ##arg)

enum _STT_PORTS {
    _STT_AAO = 0,
    _STT_AFO = 1,
    _STT_FLKO = 2,
    _STT_DMAO_NUM
};

PortID          portIDMap[_STT_DMAO_NUM] = {PORT_AAO, PORT_AFO, PORT_FLKO};
pthread_t       dequeThread[_STT_DMAO_NUM];
sem_t           semDeque[_STT_DMAO_NUM];
bool            bStart = 0;
IStatisticPipe* mpSttPipe;

/*******************************************************************************
*  Main Function 
********************************************************************************/

void dumpBuf(QBufInfo &rQBuf)
{
    int i;
    for (i = 0; i < rQBuf.mvOut.size(); i++) {
        LOG_DBG("Port(%d)-(%d) P_x%x V_x%x Size(%d) MagicNum_x%x T(%d)", PORT_AAO.index, i,\
                rQBuf.mvOut.at(i).mPa, rQBuf.mvOut.at(i).mVa, rQBuf.mvOut.at(i).mSize,\
                rQBuf.mvOut.at(i).mMetaData.mMagicNum_tuning,\
                rQBuf.mvOut.at(i).mMetaData.mTimeStamp);
    }
}

void* procLoop(void *arg)
{
    long     portIdx = (long)(arg);

    ::pthread_detach(::pthread_self());

    ::sem_wait(&semDeque[portIdx]);

    while (bStart) {
        QBufInfo    rDQBuf, rEQBuf;
        int         i;

        /**
         * Blocking wait deque done
         */
        mpSttPipe->deque(portIDMap[portIdx], rDQBuf);

        /**
         * Do something after deque buf
         */
        dumpBuf(rDQBuf);

        /**
         * Enque back buffers
         * Only fill necessary fields
         */
        rEQBuf.mvOut.resize(rDQBuf.mvOut.size());
        for (i = 0; i < rDQBuf.mvOut.size(); i++) {
            rEQBuf.mvOut.at(i).mPortID  = portIDMap[portIdx];
            rEQBuf.mvOut.at(i).mPa      = rDQBuf.mvOut.at(i).mPa;
            rEQBuf.mvOut.at(i).mVa      = rDQBuf.mvOut.at(i).mVa;
        }
        mpSttPipe->enque(rEQBuf);
    }

    ::sem_post(&semDeque[portIdx]);

    return NULL;
}

void initThreads(void)
{
    pthread_create(&dequeThread[_STT_AAO], NULL, procLoop, (void*)_STT_AAO);
    pthread_create(&dequeThread[_STT_AFO], NULL, procLoop, (void*)_STT_AFO);
    pthread_create(&dequeThread[_STT_FLKO], NULL, procLoop, (void*)_STT_FLKO);

    ::sem_init(&semDeque[_STT_AAO], 0, 0);
    ::sem_init(&semDeque[_STT_AFO], 0, 0);
    ::sem_init(&semDeque[_STT_FLKO], 0, 0);
}

void startThreads()
{
    bStart = 1;

    ::sem_post(&semDeque[_STT_AAO]);
    ::sem_post(&semDeque[_STT_AFO]);
    ::sem_post(&semDeque[_STT_FLKO]);
}

void stopThreads()
{
    bStart = 0;

    ::sem_wait(&semDeque[_STT_AAO]);
    ::sem_wait(&semDeque[_STT_AFO]);
    ::sem_wait(&semDeque[_STT_FLKO]);
}

int test_sttpipe(void)
{
    /**
     * TODO: needs add NormalPipe control flow
     */
    int             ret = 0;

    initThreads();

    mpSttPipe = IStatisticPipe::createInstance(0, THIS_NAME);

    if (MFALSE == mpSttPipe->init()) {
        LOG_ERR("init fail");
        return -1;
    }

    mpSttPipe->start();

    startThreads();

    stopThreads();

    mpSttPipe->stop();

    mpSttPipe->uninit();

    mpSttPipe->destroyInstance(THIS_NAME);

    return ret;
}



