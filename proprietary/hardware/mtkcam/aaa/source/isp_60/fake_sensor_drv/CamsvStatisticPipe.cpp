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

#define LOG_TAG "fakeCamsvSttPipe"
//
#include <stdio.h>
#include <stdlib.h>
//
#include <mtkcam/utils/std/Log.h>
#include <vector>

#include <IEventIrq.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/aaa_hal_common.h>
#include <mtkcam/aaa/IHal3A.h>
#include "aaa_hal_if.h"
#include "aaa_utils.h"
#include "aaa_result.h"
#include "CamsvStatisticPipe.h"

#include <semaphore.h>
#include <utils/threads.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <hardware/camera3.h>
//#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <drv/isp_reg.h>

#define PIPE_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define PIPE_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

#define THIS_NAME   "CamsvStatisticPipe"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

/******************************************************************************
 *
 ******************************************************************************/
// #include <mtkcam/def/PriorityDefs.h>
// #define PTHREAD_CTRL_M_STT (1)
#define MAX_FILE_PATH_LEN 100
#define MAX_BUF_COUNT 20
#define MAX_BUF_NAME_LEN 20

Mutex           CamsvStatisticPipe::SPipeGLock;
CamsvStatisticPipe*  CamsvStatisticPipe::pCamsvStatisticPipe[MAX_SENSOR_CNT] = {NULL, NULL, NULL, NULL};


/******************************************************************************
 *
 ******************************************************************************/
ICamsvStatisticPipe* ICamsvStatisticPipe::createInstance(MUINT32 SensorIndex, char const* szCallerName,  MUINT32 FeatureIdx)
{
    PIPE_LOG("+ SenIdx=%d %s", SensorIndex, szCallerName);

    CamsvStatisticPipe* pPipe = 0;

    if (SensorIndex >= MAX_SENSOR_CNT) {
        PIPE_ERR("InvalidSensorIdx = %d", SensorIndex);
        return MFALSE;
    }

    CamsvStatisticPipe::SPipeGLock.lock();

    pPipe = CamsvStatisticPipe::pCamsvStatisticPipe[SensorIndex];
    if (NULL == pPipe)
    {
        CamsvStatisticPipe::pCamsvStatisticPipe[SensorIndex] = new CamsvStatisticPipe(SensorIndex, THIS_NAME);
        pPipe = CamsvStatisticPipe::pCamsvStatisticPipe[SensorIndex];
    }

    pPipe->addUser(szCallerName);

    CamsvStatisticPipe::SPipeGLock.unlock();

    PIPE_LOG("-");

    return pPipe;
}

void ICamsvStatisticPipe::destroyInstance(char const* szCallerName)
{
    // (void)szCallerName;
    return;
}

MBOOL CamsvStatisticPipe::addUser(char const* szUserName)
{
    MINT32 i = 0, found_idx = -1, len = 0;

    len = strlen(szUserName);
    if ((len == 0) || (len >= IOPIPE_MAX_USER_NAME_LEN)) {
        PIPE_ERR("Ivalide user name length: %d", len);
        return MFALSE;
    }

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            if (strcmp(mUserName[i], szUserName) == 0) {
                found_idx = i;
                break;
            }
        }
    }
    if (found_idx == -1) {
        for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
            if (mUserCnt[i] == 0) {
                break;
            }
        }
        if (i < IOPIPE_MAX_NUM_USERS) {
            found_idx = i;
            strcpy(mUserName[i], szUserName);
        }
        else {
            PIPE_ERR("User count(%d) reaches maximum!", i);
            return MFALSE;
        }
    }

    mUserCnt[found_idx]++;
    mTotalUserCnt++;

    PIPE_LOG("%s ++, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID CamsvStatisticPipe::destroyInstance(char const* szCallerName)
{
    PIPE_LOG("+");
    MINT32 idx = 0;

    SPipeGLock.lock();

    for (idx = 0; idx < MAX_SENSOR_CNT; idx++) {
        if (this == pCamsvStatisticPipe[idx]) {
            break;
        }
    }

    if (MAX_SENSOR_CNT == idx) {
        PIPE_ERR("Can't find the entry");
        SPipeGLock.unlock();
        return;
    }

    if (mTotalUserCnt <= 0) {
        PIPE_LOG("No instance now");
        SPipeGLock.unlock();
        return;
    }

    if (!delUser(szCallerName)) {
        PIPE_ERR("No such user");
        SPipeGLock.unlock();
        return;
    }

    if (mTotalUserCnt == 0)  {
        // mpEventIrq->destroyInstance(THIS_NAME);
        // mpEventIrq = NULL;

        delete pCamsvStatisticPipe[idx];
        pCamsvStatisticPipe[idx] = NULL;
    }

    SPipeGLock.unlock();

    PIPE_LOG("-");
}

MBOOL CamsvStatisticPipe::delUser(char const* szUserName)
{
    MINT32 i = 0, found_idx = -1, len = 0;

    len = strlen(szUserName);
    if ((len == 0) || (len >= IOPIPE_MAX_USER_NAME_LEN)) {
        PIPE_ERR("Invalid user name length: %d", len);
        return MFALSE;
    }

    for (i = 0; i < IOPIPE_MAX_NUM_USERS; i++) {
        if (mUserCnt[i] > 0) {
            if (strcmp(mUserName[i], szUserName) == 0) {
                found_idx = i;
            }
        }
    }

    if (found_idx == -1) {
        PIPE_ERR("Invalid user name", szUserName);
        return MFALSE;
    }

    mUserCnt[found_idx]--;
    mTotalUserCnt--;

    if (mUserCnt[found_idx] == 0) {
        mUserName[found_idx][0] = '\0';
    }

    PIPE_LOG("%s --, %d/%d", szUserName, mUserCnt[found_idx], mTotalUserCnt);

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
CamsvStatisticPipe::CamsvStatisticPipe(MUINT32 SensorIdx, char const* szCallerName, MUINT32 FeatureIdx)
    : mpName(szCallerName)
    , mConfigDone(MFALSE)
    , mBurstQNum(1)
    , mTgInfo(0)
    , mTotalUserCnt(0)
    , mpSensorIdx(SensorIdx)
    , m_bStarted(MFALSE)
    , mOpenedPort(_DMAO_NONE)
{
    memset((void*)PortIndex, 0, sizeof(PortIndex));
    memset((void*)mUserCnt, 0, sizeof(mUserCnt));
    memset((void*)mUserName, 0, sizeof(mUserName));
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::start()
{
    MBOOL   ret = MTRUE;
    // MUINT32 _size = 0, _tmp;

    PIPE_LOG("+");

    PIPE_LOG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::stop()
{
    MBOOL ret = MTRUE;

    PIPE_LOG("+ SenIdx[%d]", mpSensorIdx);

    PIPE_LOG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::init()
{
    MBOOL       ret = MTRUE;

    PIPE_LOG("+ SenIdx[%d]", mpSensorIdx);
    mReadCount = 3;

    // IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    // pHalSensorList->searchSensors();
    // mpSensorDev = pHalSensorList->querySensorDevIdx(mpSensorIdx);
    // IEventIrq::ConfigParam IrqConfig(mpSensorDev, mpSensorIdx, 5000000, IEventIrq::E_Event_Vsync);
    // mpEventIrq = DefaultEventIrq::createInstance(IrqConfig, THIS_NAME);
    PIPE_LOG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::uninit()
{
    MBOOL   ret = MTRUE;

    PIPE_LOG("+ SenIdx[%d]", mpSensorIdx);

    PIPE_LOG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::enque(QBufInfo const& rQBuf)
{
    MBOOL           ret = MTRUE;
    mThreadIdleLock.lock();
    release_hw_buf(rQBuf);
    mThreadIdleLock.unlock();

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::release_hw_buf(QBufInfo const& rQBuf)
{
    MBOOL           ret = MTRUE;

    BufInfo bufInfo = rQBuf.mvOut.at(rQBuf.mvOut.size()-1);
    delete (MVOID*)bufInfo.mVa;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::deque(PortID port, QBufInfo& rQBuf, ESTT_CACHE_SYNC_POLICY cacheSyncPolicy, MUINT32 u4TimeoutMs)
{
    MBOOL           ret = MTRUE;

    mThreadIdleLock.lock();
    // IEventIrq::Duration duration;
    // if(!(mpEventIrq->wait(duration) >= 0))
    // {
    //     PIPE_ERR("wait irq timeout or error\n");
    // }
    ::usleep(2000);
    read_hw_buf(port, rQBuf);
    mThreadIdleLock.unlock();

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::read_hw_buf(PortID& port, QBufInfo& rQBuf)
{
    MBOOL           ret = MTRUE;

    // property_get("vendor.aaout.read.enable", value, "0");
    // MBOOL bEnableRead = atoi(value);
    // CAM_LOGD("[%s] bEnableRead(%d) \n", __FUNCTION__, bEnableRead);
    char portName[MAX_BUF_NAME_LEN] = "mvhdr";

    MBOOL bEnableRead = MTRUE;
    BufInfo bufInfo;
    // read aao hw data for ut
    if (bEnableRead)
    {
        mReadCount++;
        if(mReadCount > MAX_BUF_COUNT) mReadCount = 3;
        // PIPE_LOG("mReadCount(%d)", mReadCount);

        MINT32 readCount = mReadCount;
        char fileName[64];
        // ex: /sdcard/mvhdrHwBuf/mvhdr_1.hwbuf
        sprintf(fileName, "/sdcard/%sHwBuf/%s_%d.hwbuf", portName, portName, readCount);
        FILE *fp = fopen(fileName, "rb");
        if (NULL == fp)
        {
            PIPE_ERR("fail to open file to read img: %s", fileName);
        } else
        {
            PIPE_LOG("read %s hw buf magic(%d) file = %s\n", portName, readCount, fileName);

            fread(&(bufInfo.mMetaData.mMagicNum_tuning), sizeof(MUINT32), 1, fp);
            fread(&(bufInfo.mStride), sizeof(MUINT32), 1, fp);
            fread(&(bufInfo.mSize), sizeof(MUINT32), 1, fp);

            char* pChar = new char[bufInfo.mSize];
            bufInfo.mVa = reinterpret_cast<MUINTPTR>(pChar);

            fread(reinterpret_cast<MVOID *>(bufInfo.mVa), 1, bufInfo.mSize, fp);

            // PIPE_LOG("after read");
            // PIPE_LOG("bufInfo.mMetaData.mMagicNum_tuning(%d)", bufInfo.mMetaData.mMagicNum_tuning);
            // PIPE_LOG("bufInfo.mStride(%d)", bufInfo.mStride);
            // PIPE_LOG("bufInfo.mSize(%d)", bufInfo.mSize);
            fclose(fp);
        }
    }

    rQBuf.mvOut.push_back(bufInfo);

    return ret;
}

MBOOL CamsvStatisticPipe::configPipe(QInitStatParam const& vInPorts, MINT32 burstQnum)
{
    MBOOL       ret = MTRUE;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    MBOOL       ret = MTRUE;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::abortDma(PortID port, char const* szCallerName)
{
    MBOOL       ret = MTRUE;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL CamsvStatisticPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;

    return ret;
}