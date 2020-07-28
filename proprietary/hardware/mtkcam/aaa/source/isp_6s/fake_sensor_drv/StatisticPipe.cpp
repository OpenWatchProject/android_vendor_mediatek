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

#define LOG_TAG "fakeSttPipe"
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
#include "StatisticPipe.h"

#include <semaphore.h>
#include <utils/threads.h>
#include <mtkcam/utils/metastore/ITemplateRequest.h>
#include <hardware/camera3.h>
//#include <system/camera_metadata.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <drv/isp_reg.h>

#define PIPE_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define PIPE_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

#define THIS_NAME   "StatisticPipe"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

/******************************************************************************
 *
 ******************************************************************************/
// #include <mtkcam/def/PriorityDefs.h>
// #define PTHREAD_CTRL_M_STT (1)
// Enum use for 3A UT to do mapping
typedef enum
{
    AAO_PORT_IDX,
    AFO_PORT_IDX,
    FLKO_PORT_IDX,
    PDO_PORT_IDX,
    BUF_PORT_NUM
} BUF_ENUM;

#define MAX_FILE_PATH_LEN 100
#define MAX_BUF_COUNT 60
#define MAX_BUF_NAME_LEN 20
static char gBufName[BUF_PORT_NUM][MAX_BUF_NAME_LEN]=
{
    "aao",
    "afo",
    "flko",
    "pdo",
};

//map portId to deque container index
#define _PortMap(PortIdx, id, outName)   ({\
    if (PortIdx == PORT_AAO.index)\
        id = AAO_PORT_IDX;\
    else if (PortIdx == PORT_AFO.index)\
        id = AFO_PORT_IDX;\
    else if (PortIdx == PORT_FLKO.index)\
        id = FLKO_PORT_IDX;\
    else if (PortIdx == PORT_PDO.index)\
        id = PDO_PORT_IDX;\
    else PIPE_ERR("error:portidx:0x%x\n",PortIdx);\
    strcpy(outName, gBufName[id]);\
})

Mutex           StatisticPipe::SPipeGLock;
StatisticPipe*  StatisticPipe::pStatisticPipe[MAX_SENSOR_CNT] = {NULL, NULL, NULL, NULL};


/******************************************************************************
 *
 ******************************************************************************/
IStatisticPipe* IStatisticPipe::createInstance(MUINT32 SensorIndex, char const* szCallerName)
{
    PIPE_LOG("+ SenIdx=%d %s", SensorIndex, szCallerName);

    StatisticPipe* pPipe = 0;

    if (SensorIndex >= MAX_SENSOR_CNT) {
        PIPE_ERR("InvalidSensorIdx = %d", SensorIndex);
        return MFALSE;
    }

    StatisticPipe::SPipeGLock.lock();

    pPipe = StatisticPipe::pStatisticPipe[SensorIndex];
    if (NULL == pPipe)
    {
        StatisticPipe::pStatisticPipe[SensorIndex] = new StatisticPipe(SensorIndex, THIS_NAME);
        pPipe = StatisticPipe::pStatisticPipe[SensorIndex];
    }

    pPipe->addUser(szCallerName);

    StatisticPipe::SPipeGLock.unlock();

    PIPE_LOG("-");

    return pPipe;
}

void IStatisticPipe::destroyInstance(char const* szCallerName)
{
    // (void)szCallerName;
    return;
}

MBOOL StatisticPipe::addUser(char const* szUserName)
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
MVOID StatisticPipe::destroyInstance(char const* szCallerName)
{
    PIPE_LOG("+");
    MINT32 idx = 0;

    SPipeGLock.lock();

    for (idx = 0; idx < MAX_SENSOR_CNT; idx++) {
        if (this == pStatisticPipe[idx]) {
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

        // for(int i=0; i<MaxStatPortIdx; i++)
        // {
        //     mpEventIrq[i]->destroyInstance(gBufName[i]);
        //     mpEventIrq[i] = NULL;
        // }

        delete pStatisticPipe[idx];
        pStatisticPipe[idx] = NULL;
    }

    SPipeGLock.unlock();

    PIPE_LOG("-");
}

MBOOL StatisticPipe::delUser(char const* szUserName)
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
StatisticPipe::StatisticPipe(MUINT32 SensorIdx, char const* szCallerName)
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
MBOOL StatisticPipe::start()
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
MBOOL StatisticPipe::stop()
{
    MBOOL ret = MTRUE;

    PIPE_LOG("+ SenIdx[%d]", mpSensorIdx);

    PIPE_LOG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::init()
{
    MBOOL       ret = MTRUE;

    PIPE_LOG("+ SenIdx[%d]", mpSensorIdx);
    IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();
    mpSensorDev = pHalSensorList->querySensorDevIdx(mpSensorIdx);
    IEventIrq::ConfigParam IrqConfig(mpSensorDev, mpSensorIdx, 5000000, IEventIrq::E_Event_Vsync);

    for(int i=0; i<MaxStatPortIdx; i++)
    {
        mReadCount[i] = 3;
        // mpEventIrq[i] = DefaultEventIrq::createInstance(IrqConfig, gBufName[i]);
    }


    PIPE_LOG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::uninit()
{
    MBOOL   ret = MTRUE;

    PIPE_LOG("+ SenIdx[%d]", mpSensorIdx);

    PIPE_LOG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::enque(QBufInfo const& rQBuf)
{
    MBOOL           ret = MTRUE;

    char portName[MAX_BUF_NAME_LEN];
    BUF_ENUM id;
    MUINT32 idx = rQBuf.mvOut.at(rQBuf.mvOut.size()-1).mPortID.index;
    _PortMap(idx, id, portName);

    mThreadIdleLock[id].lock();
    release_hw_buf(rQBuf);
    mThreadIdleLock[id].unlock();

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::release_hw_buf(QBufInfo const& rQBuf)
{
    MBOOL           ret = MTRUE;

    BufInfo bufInfo = rQBuf.mvOut.at(rQBuf.mvOut.size()-1);
    delete (MVOID*)bufInfo.mVa;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::deque(PortID port, QBufInfo& rQBuf, ESTT_CACHE_SYNC_POLICY cacheSyncPolicy, MUINT32 u4TimeoutMs)
{
    MBOOL           ret = MTRUE;

    char portName[MAX_BUF_NAME_LEN];
    BUF_ENUM id;
    _PortMap(port.index, id, portName);

    mThreadIdleLock[id].lock();
    ::usleep(2000);
    // IEventIrq::Duration duration;
    // if(!(mpEventIrq[id]->wait(duration) >= 0))
    // {
    //     PIPE_ERR("wait irq timeout or error\n");
    // }

    read_hw_buf(port, rQBuf);
    mThreadIdleLock[id].unlock();
    if(port == PORT_AAO)
    {
        int size = rQBuf.mvOut.size();
        BufInfo rLastBuf = rQBuf.mvOut.at( size-1 );
        mAEOVa = rLastBuf.mVa;
    }
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::read_hw_buf(PortID& port, QBufInfo& rQBuf)
{
    MBOOL           ret = MTRUE;
    char portName[MAX_BUF_NAME_LEN];
    BUF_ENUM id;
    _PortMap(port.index, id, portName);
    // property_get("vendor.aaout.read.enable", value, "0");
    // MBOOL bEnableRead = atoi(value);
    // CAM_LOGD("[%s] bEnableRead(%d) \n", __FUNCTION__, bEnableRead);
    MBOOL bEnableRead = MTRUE;
    BufInfo bufInfo;
    // read hw data for ut
    if (bEnableRead)
    {
        mReadCount[id] ++;
        if(mReadCount[id] > MAX_BUF_COUNT) mReadCount[id] = 3;

        MINT32 readCount = mReadCount[id];
        char fileName[64];
        // ex: /sdcard/aaoHwBuf/aao_1.hwbuf
        sprintf(fileName, "/sdcard/%sHwBuf/%s_%d.hwbuf", portName, portName, readCount);
        FILE *fp = fopen(fileName, "rb");
        if (NULL == fp)
        {
            PIPE_ERR("fail to open file to read img: %s", fileName);
        } else
        {
            PIPE_LOG("read %s hw buf magic(%d) file = %s\n", portName, readCount, fileName);

            bufInfo.mPortID = port;
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

MBOOL StatisticPipe::configPipe(QInitStatParam const& vInPorts, MINT32 burstQnum)
{
    MBOOL       ret = MTRUE;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                            MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    MBOOL       ret = MTRUE;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::abortDma(PortID port, char const* szCallerName)
{
    MBOOL       ret = MTRUE;

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL StatisticPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;
    switch (cmd) {
    case ESPipeCmd_GET_STT_CUR_BUF:
        PIPE_LOG("mAEOVa(%p)", mAEOVa);
        PIPE_LOG("arg2(%p)", arg2);
        *(MINTPTR*)arg2 = mAEOVa;
        break;
    }


    return ret;
}