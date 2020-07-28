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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG     "fakeNormalPipe"

#include <sys/prctl.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
using namespace android;

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
using namespace NSCam;
using namespace NSCam::Utils;

// for thread priority
#include <mtkcam/def/PriorityDefs.h>

#include <utils/Thread.h>
#include <utils/Condition.h>
#include <cutils/properties.h>

#include <mtkcam/drv/IHalSensor.h>

#include "NormalPipe.h"
// #include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>



// #include "camio_log.h"
// //for N3D
// #include <mtkcam/drv/IHwSyncDrv.h>


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif
#define PTHREAD_CTRL_M      (1)

#define THIS_NAME           "NormalPipe"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;
using namespace NS3Av3;

#undef PIPE_LOG
#undef PIPE_ERR
#define PIPE_LOG(fmt, arg...)                  CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define PIPE_ERR(fmt, arg...)                  CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)

/******************************************************************************
 *
 ******************************************************************************/
static MUINT32      NPipeAllocMemSum = 0;
static MUINT32      NPipeFreedMemSum = 0;

Mutex               NormalPipe::NPipeGLock;
Mutex               NormalPipe::NPipeCfgGLock;
Mutex               NormalPipe::NPipeDbgLock;
NormalPipe*         NormalPipe::pNormalPipe[MAX_SENSOR_CNT] = {NULL, NULL, NULL, NULL};
MUINT32             NormalPipe::mEnablePath = 0;
MUINT32             NormalPipe::mFirstEnableTG = CAM_TG_NONE; /* Set at 1st config, clear when all cam stopped */

/******************************************************************************
 *
 ******************************************************************************/
INormalPipe* INormalPipe::createInstance(MUINT32 SensorIndex, char const* szCallerName)
{
    NormalPipe* pNPipe = NULL;

    PIPE_LOG("+");

    NormalPipe::NPipeGLock.lock();
    pNPipe = NormalPipe::pNormalPipe[SensorIndex];
    if (NULL == pNPipe) {
        NormalPipe::pNormalPipe[SensorIndex] = new NormalPipe(SensorIndex, THIS_NAME);
        pNPipe = NormalPipe::pNormalPipe[SensorIndex];
        PIPE_LOG("%s NPipe[%d]=0x%8x create", szCallerName, SensorIndex, pNPipe);
    }

    pNPipe->addUser(szCallerName); //pNPipe->mUserCnt++;

    NormalPipe::NPipeGLock.unlock();

    PIPE_LOG("- NPipe[%d](0x%8x) mUserCnt(%d)", SensorIndex, pNPipe, pNPipe->mTotalUserCnt);

    return pNPipe;
}


MVOID INormalPipe::destroyInstance(char const* szCallerName)
{
    (void)szCallerName;
    return;
}
MBOOL NormalPipe::addUser(char const* szUserName)
{
    /* search user names to find if exist
     * if not found, check max user num
     *      ok => set idx to the 1st empty loc
     *      ng => return err
     * else set idx to found loc
     * add user cnt @idx
     **/
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

MVOID NormalPipe::destroyInstance(char const* szCallerName)
{
    MUINT32 idx = 0;

    PIPE_LOG("+ : %s", szCallerName);

    NPipeGLock.lock();
    for (idx = 0; idx < MAX_SENSOR_CNT; idx++){
        if (this == pNormalPipe[idx]) {
            break;
        }
    }

    if (MAX_SENSOR_CNT == idx) {
        PIPE_ERR("Can't find the entry");
        NPipeGLock.unlock();
        return;
    }

    if ((mTotalUserCnt <= 0)){
        PIPE_LOG("No instance now");
        NPipeGLock.unlock();
        return;
    }

    if (!delUser(szCallerName)) {
        PIPE_ERR("No such user : %s", szCallerName);
        return;
    }

    if ((mTotalUserCnt == 0))  {
        mpEventIrq->destroyInstance(THIS_NAME);
        mpEventIrq = NULL;

        delete pNormalPipe[idx];
        pNormalPipe[idx] = NULL;
    }

    NPipeGLock.unlock();

    PIPE_LOG("-");
}

MBOOL NormalPipe::delUser(char const* szUserName)
{
    /* search user names to find if exist
     * if not found, return err
     * else set idx to found loc
     * dec user cnt @idx
     * if user cnt is 0, set name[0]='\0'
     **/
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
NormalPipe::NormalPipe(MUINT32 SensorIdx, char const* szCallerName)
    : mpHalSensor(NULL)
    , mBurstQNum(1)
    , mpName(szCallerName)
    , mConfigDone(MFALSE)
    , mTotalUserCnt(0)
    , mpSensorIdx(SensorIdx)
    , m_bStarted(MFALSE)
    , mInitSettingApplied(MFALSE)
    , mDynSwtRawType(MTRUE)
    , mPureRaw(MFALSE)
    , mResetCount(0)
    , mPrvEnqSOFIdx(0xFFFFFFFF)
    , mReqDepthPreStart(0)
    , m_b1stEnqLoopDone(MFALSE)
{

    mRrzoOut_size.w = 0;
    mRrzoOut_size.h = 0;
    mImgoOut_size.w = 0;
    mImgoOut_size.h = 0;

    memset((void*)mUserCnt, 0, sizeof(mUserCnt));
    memset((void*)mUserName, 0, sizeof(mUserName));

    init();
}

NormalPipe::~NormalPipe()
{
    PIPE_LOG("+");
    uninit();
    PIPE_LOG("-");
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32 NormalPipe::attach(const char* UserName)
{
    MINT32 user_key = 1;

    PIPE_LOG("+");
    PIPE_LOG("-");

    return user_key;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::init()
{
    MUINT32     i, j;
    MBOOL       ret = MTRUE;

    PIPE_LOG("+ SenIdx=%d", mpSensorIdx);

    IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    pHalSensorList->searchSensors();
    mpSensorDev = pHalSensorList->querySensorDevIdx(mpSensorIdx);
    pHalSensorList->querySensorStaticInfo(mpSensorDev, &mSensorStaticInfo[0]);

    IEventIrq::ConfigParam IrqConfig(mpSensorDev, mpSensorIdx, 5000000, IEventIrq::E_Event_Vsync);
    mpEventIrq = DefaultEventIrq::createInstance(IrqConfig, THIS_NAME);


    PIPE_LOG("-");

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::uninit()
{
    MUINT32 i;
    MBOOL   ret = MTRUE;

    PIPE_LOG("+ SenIdx=%d", mpSensorIdx);
    PIPE_LOG("-");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::wait(EPipeSignal eSignal, EPipeSignalClearType eClear, const MINT32 mUserKey,
                        MUINT32 TimeoutMs, SignalTimeInfo *pTime)
{
    MBOOL ret = MTRUE;

    IEventIrq::Duration duration;
    if(!(mpEventIrq->wait(duration) >= 0))
    {
        PIPE_ERR("wait irq timeout or error\n");
    }

    return ret;
}

MBOOL NormalPipe::signal(EPipeSignal eSignal, const MINT32 mUserKey)
{
    MBOOL ret = MTRUE;

    PIPE_LOG("");

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL NormalPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL   ret = MTRUE;
    MUINT32 tg;
    MINT32  sensorDev = 1;

    switch (cmd) {
    case ENPipeCmd_GET_TG_OUT_SIZE:
        //arg1 : width
        //arg2 : height
        *((MINT32*)arg1) = mSensorStaticInfo[0].previewWidth;
        *((MINT32*)arg2) = mSensorStaticInfo[0].previewHeight;
        break;
    case ENPipeCmd_GET_HBIN_INFO:
        *((MINT32*)arg1) = mSensorStaticInfo[0].previewWidth;
        *((MINT32*)arg2) = mSensorStaticInfo[0].previewHeight;
        break;
    case ENPipeCmd_GET_BIN_INFO:
        *((MINT32*)arg1) = mSensorStaticInfo[0].previewWidth;
        *((MINT32*)arg2) = mSensorStaticInfo[0].previewHeight;
        break;
    case ENPipeCmd_GET_UNI_INFO:
        {
           // get isFlkEnable
            MUINT32 _uni = 1;
            *(MUINT32*)arg1 = _uni;
        }
        break;
    case ENPipeCmd_SET_TG_INT_LINE:
        // if (!mpCamIOPipe) {
        //     PIPE_ERR("[Error]mpCamIOPipe=NULL");
        //     ret = MFALSE;
        //     goto _EXIT_SEND_CMD;
        // }
        // ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_TG_INT_LINE, arg1, (MINTPTR)(_NORMALPIPE_MAP_TG_PXLMODE(mpSensorIdx)), arg3);
        break;
    case ENPipeCmd_GET_CUR_SOF_IDX:
    case ENPipeCmd_HIGHSPEED_AE:
    case ENPIPECmd_AE_SMOOTH:
    default:
        *(MUINT32*)arg1 = 1;
        break;
    }

_EXIT_SEND_CMD:
    if (ret != MTRUE) {
        PIPE_ERR("error: sendCommand fail: (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", cmd, arg1, arg2, arg3);
    }

    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::getIspReg(MUINT32 RegAddr, MUINT32 RegCount, MUINT32 RegData[], MBOOL bPhysical)
{
    PIPE_LOG("");

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT32 NormalPipe::getIspReg(IspP1RegIo *pRegs, MUINT32 RegCount, MBOOL bPhysical)
{
    PIPE_LOG("");

    return MTRUE;
}