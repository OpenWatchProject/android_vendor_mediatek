/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
//
#define LOG_TAG "Drv/HWsync"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <cutils/atomic.h>
#include <semaphore.h>
#include <cutils/properties.h>  // For property_get().
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include "hwsync_drv_imp.h"
#include "hwsync_drv_para.h"
#include <cutils/log.h>
#include <system/thread_defs.h>
#include <utils/ThreadDefs.h>
#include <utils/threads.h>
#include <vector>
//for access N3D register in SENINF
#include "seninf_drv.h"
#include "seninf_reg.h"


/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG

#undef  __func__
#define __func__    __FUNCTION__


#ifndef USING_MTK_LDVT
#define LOG_VRB(fmt, arg...)       CAM_ULOGMV(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_DBG(fmt, arg...)       CAM_ULOGMD(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_INF(fmt, arg...)       CAM_ULOGMI(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_WRN(fmt, arg...)       CAM_ULOGMW(DBG_LOG_TAG "[%s] WARNING: " fmt , __func__, ##arg)
#define LOG_ERR(fmt, arg...)       CAM_ULOGME(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt , __FILE__, __func__, __LINE__, ##arg)


#else   // LDVT
#include "uvvf.h"

#define LOG_MSG(fmt, arg...)    VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define LOG_DBG(fmt, arg...)    VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define LOG_INF(fmt, arg...)    VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d): " fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

#endif

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

pthread_mutex_t mStateMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mEnterIdleMutex = PTHREAD_MUTEX_INITIALIZER;

static HWSyncDrvImp singleton;


MINT32 getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}


/*******************************************************************************
*
********************************************************************************/
HWSyncDrv* HWSyncDrv::createInstance()
{
    return HWSyncDrvImp::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
HWSyncDrvImp::HWSyncDrvImp()
{
    mHalSensor=NULL;
    mHalSensorList=NULL;
    mSensorDevArray[0]=mSensorDevArray[1]=SENSOR_DEV_NONE;
    mSensorTgArray[0]=mSensorTgArray[1]=CAM_TG_NONE;
    mSensorCurFpsArray[0]=mSensorCurFpsArray[1]=0;
    mSensorScenArray[0]=mSensorScenArray[1]=SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    mremainUninitIdx=0;
    mbPostSem4Idle=false;
    mSyncCount=0;
    m_DVFS=E_SMI_HIGH; // default set to E_SMI_HIGH
    mAECyclePeriod=0xffffffff; // default set to 0xffffffff
    mAECalledNum=0;
    m_DBG_t.DBG_T = (pthread_t)NULL;
    m_DBG_t.m_working_tg = 0;

    memset(&mSemHWSyncLvMain1, 0, sizeof(sem_t));
    memset(&mSemHWSyncLvMain2, 0, sizeof(sem_t));
    memset(&mSemJumpoutSync, 0, sizeof(sem_t));
    memset(&mSem2Idle, 0, sizeof(sem_t));
    memset(&mSemGeneralSyncDoneMain1, 0, sizeof(sem_t));
    memset(&mSemGeneralSyncDoneMain2, 0, sizeof(sem_t));
    mUser = 0;
    mState = HW_SYNC_STATE_NONE;
    mPreAdjustFrmTime = false;
    mThreadMain1 = 0;
    mThreadMain2 = 0;
    memset(m_pIspDrv, 0, sizeof(m_pIspDrv));
    m_MaxFrameTime = 0;
}

/*******************************************************************************
*
********************************************************************************/
HWSyncDrvImp::~HWSyncDrvImp()
{
}

/*******************************************************************************
*
********************************************************************************/
HWSyncDrv* HWSyncDrvImp::getInstance(void)
{
    LOG_DBG("singleton[0x%p],getpid[0x%08x],gettid[0x%08x] ",&singleton,getpid() ,gettid());
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
void HWSyncDrvImp::destroyInstance(void)
{
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HWSyncDrvImp::init(HW_SYNC_USER_ENUM user,MUINT32 sensorIdx,MINT32 aeCyclePeriod)
{
    Mutex::Autolock lock(mLock);
    LOG_INF("+, user&cnt(%s,%d),srIdxNum(%zu)",(user==HW_SYNC_USER_HALSENSOR)?("HW_SYNC_USER_HALSENSOR"):("HW_SYNC_USER_AE")\
        ,mUser,mSensorIdxList.size());
    MINT32 ret = 0, tmp=0;

    switch(user)
    {
        case HW_SYNC_USER_HALSENSOR:
                {
                    SeninfDrv *const pSeninf_Drv = SeninfDrv::createInstance();
                    seninf_reg_t *pSeninf = NULL;
                    pSeninf_Drv->sendCommand(CMD_SENINF_GET_SENINF_ADDR, (long unsigned int)&pSeninf);

                    //[1] record sensor index if user power on sensor
                    if(mSensorIdxList.size()<2)
                    {
                        MBOOL match=false;
                        for (vector<MUINT32>::iterator it = mSensorIdxList.begin(); it != mSensorIdxList.end();)
                        {
                            if((*it)==sensorIdx)
                            {
                                match=true;
                                break;
                            }
                            it++;
                        }
                        if(!match)
                        {
                            mSensorIdxList.push_back(sensorIdx);
                        }
                    }
                    mremainUninitIdx=0;

                    //[2] judge user powen on two sensors or not
                    if(mSensorIdxList.size() ==1)
                    {
                        //create sensor related object to get/set sensor related information
                        mHalSensorList=IHalSensorList::get();
                        MUINT const pArrayOfIndex[1]={mSensorIdxList[0]};
                        mHalSensor=mHalSensorList->createSensor("HWSyncDrv", 1, &pArrayOfIndex[0]); //get sensor handler


                        SENINF_REG_N3D_A_CTL RegIO;
                        RegIO.Raw = 0;
                        RegIO.Bits.MODE= 0x2;
                        RegIO.Bits.I2C1_EN = 0x1;
                        RegIO.Bits.N3D_EN = 0x1;
                        RegIO.Bits.DIFF_EN = 0x1;
                        RegIO.Bits.DDBG_SEL = 0x1;
                        RegIO.Bits.SEN1_TIM_EN = 0x1;
                        RegIO.Bits.SEN2_TIM_EN = 0x1;
                        if(pSeninf)
                            SENINF_WRITE_REG(pSeninf, SENINF_N3D_A_CTL, RegIO.Raw);
                        else
                            LOG_ERR("enable N3D reg fail");

                    }
                    else if(mSensorIdxList.size() ==2)
                    {
                        SENINF_REG_N3D_A_CTL RegIO;
                        RegIO.Raw = 0;
                        RegIO.Bits.MODE= 0x2;
                        RegIO.Bits.I2C1_EN = 0x1;
                        RegIO.Bits.N3D_EN = 0x1;
                        RegIO.Bits.DIFF_EN = 0x1;

                        if(pSeninf)
                            SENINF_WRITE_REG(pSeninf, SENINF_N3D_A_CTL, RegIO.Raw);
                        else
                            LOG_ERR("enable N3D reg fail");

                        //initial state and variables
                        this->m_MaxFrameTime = 0xFFFFFFFF;
                        pthread_mutex_lock(&mStateMutex);
                        mState=HW_SYNC_STATE_READY2RUN;
                        pthread_mutex_unlock(&mStateMutex);
                        ::sem_init(&mSemHWSyncLvMain1, 0, 0);
                        ::sem_init(&mSemHWSyncLvMain2, 0, 0);
                        ::sem_init(&mSemJumpoutSync, 0, 0);
                        ::sem_init(&mSem2Idle, 0, 0);
                        ::sem_init(&mSemGeneralSyncDoneMain1, 0, 0);
                        ::sem_init(&mSemGeneralSyncDoneMain2, 0, 0);

                        //create exection thread
                        this->createThread();
                    }
                    pSeninf_Drv->destroyInstance();
            }
            break;
        case HW_SYNC_USER_AE:
            this->mAECyclePeriod = aeCyclePeriod;
            this->mAECalledNum = 0;
            LOG_INF("mAECyclePeriod(%d)",this->mAECyclePeriod);
            break;
        default:
            LOG_ERR("unsupported user:%d\b",user);
            break;
    }

    if(this->mUser == 0) {
        this->mSyncCount = 0;
    }

    //
    tmp = android_atomic_inc(&mUser);
    //
    LOG_DBG("-");
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HWSyncDrvImp::uninit(HW_SYNC_USER_ENUM user,MUINT32 sensorIdx)
{
    Mutex::Autolock lock(mLock);
    LOG_INF("+, user&cnt(%s,%d),srIdxNum(%zu)",(user==HW_SYNC_USER_HALSENSOR)?("HW_SYNC_USER_HALSENSOR"):("HW_SYNC_USER_AE")\
        ,mUser,mSensorIdxList.size());
    MINT32 ret = 0, tmp=0;

    if(mUser<=0)
    {
        LOG_ERR("strange flow mUser(%d)",mUser);
        return -1;
    }
    //
    tmp = android_atomic_dec(&mUser);

    switch(user)
    {
        case HW_SYNC_USER_HALSENSOR:
                {
                    //[1] remove sensor index if user power off sensor
                    MBOOL match=false;
                    int uninitIdx=0;
                    for (vector<MUINT32>::iterator it = mSensorIdxList.begin(); it != mSensorIdxList.end();)
                    {
                        if((*it)==sensorIdx)
                        {
                            match=true;
                            if((uninitIdx==0) && mSensorIdxList.size()==2)
                            {
                                mremainUninitIdx=1;
                            }
                            it = mSensorIdxList.erase(it);
                            break;
                        }
                        it++;
                        uninitIdx++;
                    }
                    if(!match)
                    {
                        LOG_ERR("uninit fail, wrong sensorIdx(%d)",sensorIdx);
                        return -1;
                    }

                    //[3]when used sensor number from 2 to 1, disable hwsync driver function and halsensor/isp_drv object
                    if(mSensorIdxList.size() ==0)
                    {
                            uninitIdx=uninitIdx+mremainUninitIdx;   //last one for uninit needed to be shift

                            //sensor
                            mHalSensor->destroyInstance("HWSyncDrv");
                            mHalSensor=NULL;
                            mHalSensorList=NULL;

                    }
                    else if(mSensorIdxList.size() ==1)
                    {
                        //leave state
                        pthread_mutex_lock(&mStateMutex);
                        mState=HW_SYNC_STATE_READY2LEAVE;
                        pthread_mutex_unlock(&mStateMutex);
                        //destroy thread and leave
                        destroyThread();
                        //wait leave signal
                        ::sem_wait(&mSemHWSyncLvMain1);
                        ::sem_wait(&mSemHWSyncLvMain2);
                    }

                    //[4]remove sensor device info if user power off sensor(need after destroyThread cuz function "revertSenssorFps need senDev info")
                    if(uninitIdx<=1)
                    {
                        LOG_INF("idx(%d,remain_%d),tg(%d)",uninitIdx,mremainUninitIdx,mSensorTgArray[uninitIdx]);
                        switch(mSensorTgArray[uninitIdx])
                        {
                            case CAM_TG_1:
                                mSensorDevArray[0]=SENSOR_DEV_NONE;
                                mSensorCurFpsArray[0]=0;
                                break;
                            case CAM_TG_2:
                                mSensorDevArray[1]=SENSOR_DEV_NONE;
                                mSensorCurFpsArray[1]=0;
                                break;
                            default:
                                LOG_WRN("wtginf, sindex(0x%x),tg(0x%x)",mSensorIdxList[uninitIdx],mSensorTgArray[uninitIdx]);
                                break;
                        }

                    }
                    else
                    {
                        LOG_ERR("wrong uninitIdx(%d) in mSensorIdxList",uninitIdx);
                        return -2;
                    }

            }
            break;
        case HW_SYNC_USER_AE:
            this->mAECyclePeriod = 0xffffffff;
            this->mAECalledNum = 0;
            LOG_INF("mAECyclePeriod(%d)",this->mAECyclePeriod);
            break;
        default:
            LOG_ERR("unsupported user:%d\b",user);
            break;
    }
    //
    LOG_DBG("-");
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
#define str(x)  ({\
   static char _str[48];\
   _str[0] = '\0';\
   switch(x){\
       case HW_SYNC_CMD_SET_MODE:\
           sprintf(_str,"HW_SYNC_CMD_SET_MODE");\
           break;\
       case HW_SYNC_CMD_DISABLE_SYNC:\
           sprintf(_str,"HW_SYNC_CMD_DISABLE_SYNC");\
           break;\
       case HW_SYNC_CMD_ENABLE_SYNC:\
           sprintf(_str,"HW_SYNC_CMD_ENABLE_SYNC");\
           break;\
       case HW_SYNC_CMD_GET_FRAME_TIME:\
           sprintf(_str,"HW_SYNC_CMD_GET_FRAME_TIME");\
           break;\
       case HW_SYNC_CMD_SET_PARA:\
           sprintf(_str,"HW_SYNC_CMD_SET_PARA");\
           break;\
       case HW_SYNC_CMD_SYNC_TYPE:\
           sprintf(_str,"HW_SYNC_CMD_SYNC_TYPE");\
           break;\
   }\
   _str;\
})

MINT32 HWSyncDrvImp::sendCommand(HW_SYNC_CMD_ENUM cmd,MUINT32 arg1,MUINTPTR arg2,MUINTPTR arg3,MUINTPTR arg4)
{
    LOG_DBG("+ cmd(%s)",str(cmd));
    arg4;
    Mutex::Autolock lock(mLock);    //protect from halsensor and aaasensormgr
    MINT32 ret=0, tmp=0;
    MBOOL needWait=false;
    switch(cmd)
    {
        case HW_SYNC_CMD_SET_MODE:
            {
                //[1]get mapping relationship between tg and sensro device cuz we only could know tg info after user config sensor,
                //   then sensor would call sendCommand of hwysnc after config two sensors
                MUINT32 localsenDev=SENSOR_DEV_NONE;
                MUINT32 senTg=CAM_TG_NONE;
                SensorDynamicInfo dynamicInfo[1];
                SensorStaticInfo sensorStaticInfo;
                MUINT32 frameTimeDelay=1;
                MINT32 fps=0; //unit: the first digit after the decimal point, ex:30fps -> 300

                LOG_INF("mSensorIdxList.size(%zu)",mSensorIdxList.size());
                for(unsigned int i=0;i<mSensorIdxList.size();i++)
                {
                    localsenDev=mHalSensorList->querySensorDevIdx(mSensorIdxList[i]);
                    if(localsenDev==arg1)
                    {   //set sensor dev if sensorHal.Control do configure
                        mHalSensor->querySensorDynamicInfo(arg1,&dynamicInfo[0]);
                        mHalSensorList->querySensorStaticInfo(localsenDev,&sensorStaticInfo);
                        senTg=dynamicInfo[0].TgInfo;
                        frameTimeDelay = sensorStaticInfo.FrameTimeDelayFrame;
                        if(0 == frameTimeDelay) {
                            LOG_WRN("NO FrameTimeDelayFrame from sensor_dev:%d, set to 1", arg1);
                            frameTimeDelay = 1;
                        }
                        mSensorTgArray[i]=senTg;
                        switch(senTg)
                        {
                            case CAM_TG_1:
                                mSensorDevArray[0]=arg1;
                                mSensorDevMapHwsyncIndex[arg1] = 0;
                                mSensorScenArray[0]=arg2;
                                mSensorCurFpsArray[0]=calCurrentFps(0);
                                mFrameTimeDelayFrame[0]= frameTimeDelay;
                                LOG_INF("dev(0x%x),scen(%d) cFps(%d), userFps(%llu), frmTimeDelay(%d), hwsync_idx(%d)", \
                                    mSensorDevArray[0],mSensorScenArray[0],mSensorCurFpsArray[0],(long long)arg3, \
                                    mFrameTimeDelayFrame[0], mSensorDevMapHwsyncIndex[arg1]);
                                break;
                            case CAM_TG_2:
                                mSensorDevArray[1]=arg1;
                                mSensorDevMapHwsyncIndex[arg1] = 1;
                                mSensorScenArray[1]=arg2;
                                mSensorCurFpsArray[1]=calCurrentFps(1);
                                mFrameTimeDelayFrame[1]= frameTimeDelay;
                                LOG_INF("dev(0x%x),scen(%d) cFps(%d), userFps(%llu), frmTimeDelay(%d), hwsync_idx(%d)", \
                                    mSensorDevArray[1],mSensorScenArray[1],mSensorCurFpsArray[1],(long long)arg3, \
                                    mFrameTimeDelayFrame[1], mSensorDevMapHwsyncIndex[arg1]);
                                break;
                            default:
                                LOG_ERR("wrong tg information, sensor index(0x%x),dev(0x%x),tg(0x%x)",mSensorIdxList[i],localsenDev,senTg);
                                return -3;
                                break;
                        }
                    }
                }
                LOG_DBG("cursenSce(%llu), tg_dev info (0x%x/0x%x)",(long long)arg2,mSensorDevArray[0],mSensorDevArray[1]);

                //[2] only do hwsync in stereo cam feature
                if(mSensorIdxList.size() < 2)
                {
                    LOG_DBG("no need hwsync,size(%zu)",mSensorIdxList.size());
                    return 0;
                }



                //[3]change mode only when two sensor are configured
                if((mSensorDevArray[0] != SENSOR_DEV_NONE) && (mSensorDevArray[1] != SENSOR_DEV_NONE))
                {
                    LOG_INF("initial dev(0x%x/0x%x),initial fps(%d/%d)",mSensorDevArray[0], mSensorDevArray[1], mSensorCurFpsArray[0],mSensorCurFpsArray[1]);
                }
            }
            break;
        case HW_SYNC_CMD_DISABLE_SYNC:
            {
                tmp = android_atomic_dec(&this->mSyncCount);

                if(this->mSyncCount == 0){
                    LOG_INF("m_pIspDrv uninit & destroyInstance");

                    //m_pIspDrv
                    for(int i = 0; i < E_TG_MAX; i++) {
                        this->m_pIspDrv[i]->uninit("Hwsync");
                        this->m_pIspDrv[i]->destroyInstance();
                    }
                }
                else{
                    HWSyncInit* pInit;

                    //reset vsync cnt
                    pInit = new HWSyncInit(this);
                    pInit->DeviceInfo(HWSyncInit::E_RESET_VSYNC_CNT,0);
                    delete pInit;
                }

                pthread_mutex_lock(&mStateMutex);
                if(mState==HW_SYNC_STATE_SYNCING)
                {
                    mState=HW_SYNC_STATE_IDLE ;
                    pthread_mutex_unlock(&mStateMutex);

                    if(arg1 == HW_SYNC_BLOCKING_EXE){
                        LOG_INF("sem_wait mSemGeneralSyncDone +!!");
                        ::sem_wait(&mSemGeneralSyncDoneMain1);
                        ::sem_wait(&mSemGeneralSyncDoneMain2);
                        LOG_INF("sem_wait mSemGeneralSyncDone -!!");
                    }
                    else{
                        ret = 1;
                        LOG_ERR("unsupported execution(HW_SYNC_NON_BLOCKING_EXE under SYNCING status)");
                    }
                }
                else //need wait sync related procedure stop before do AE operation, assume do AE operation after 2 sensors are configured done
                {
                    mState=HW_SYNC_STATE_IDLE ;
                    pthread_mutex_unlock(&mStateMutex);
                }
                if(ret == 0){
                    clearExecCmdsMain1();
                    clearExecCmdsMain2();
                }

                for(MUINT32 i=0;i<Dev_temp.size();i++){
                    if(Dev_temp[i] == arg2){
                        Dev_temp.erase(Dev_temp.begin()+i);
                        TG_Shutter.erase(TG_Shutter.begin()+i);
                        break;
                    }
                }
            }
            break;
        case HW_SYNC_CMD_SET_PARA:
            if(Dev_temp.size()>2){
                LOG_ERR("Dev_temp.size(%zu), can't more than 2 for hwsync",Dev_temp.size());
                ret = 1;
            }
            else{
                if(Dev_temp.size()!=0){
                    if(Dev_temp[0] == arg1){
                        LOG_ERR("cant set the same sen DevID twince(%d)",arg1);
                        ret = 1;
                    }
                }
                Dev_temp.push_back(arg1); //Save the DevID untill call HW_SYNC_CMD_ENABLE_SYNC
                TG_Shutter.push_back(arg3);
                for(unsigned int i=0; i<Dev_temp.size(); i++){
                    LOG_INF("Dev_temp[%d]=%d DVFS level:0x%llu", i, Dev_temp[i], (long long)arg2);
                }
                for(unsigned int i=0; i<TG_Shutter.size(); i++){
                    LOG_INF("TG_Shutter[%d]=%d", i, TG_Shutter[i]);
                }
            }
            switch(arg2){//scenario here is dvfs level
                case E_SMI_LOW://case number ref to ihalcamio.h
                    this->m_DVFS = E_SMI_LOW;
                    break;
                case E_SMI_HIGH:
                    this->m_DVFS = E_SMI_HIGH;
                    break;
                default:
                    this->m_DVFS = E_SMI_HIGH;
                    LOG_ERR("unsupported DVFS level:%llu", (long long)arg2);
                    break;
            }
            break;
        case HW_SYNC_CMD_ENABLE_SYNC:

            //HW_SYNC_CMD_SET_PARA
            if(Dev_temp.size()==2){

            }
            else{
                 LOG_INF("Total amount of Dev not 2(%zu)",Dev_temp.size());
            }

            if(this->mSyncCount == 0){
                //m_pIspDrv
                for(int i = 0; i < E_TG_MAX; i++) {
                    switch(i) {
                        case E_TG_A:
                            this->m_pIspDrv[i] = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
                            break;
                        case E_TG_B:
                            this->m_pIspDrv[i] = (IspDrvImp*)IspDrvImp::createInstance(CAM_B);
                            break;
                        default:
                            LOG_ERR("Unsupported TG index(0x%x)", i);
                            break;
                    }

                    if (MFALSE == this->m_pIspDrv[i]->init("Hwsync")) {
                        LOG_ERR("init drv fail");
                        ret = 1;
                    }

                }

            }

            tmp = android_atomic_inc(&this->mSyncCount);

            switch(this->mSyncCount){
                case 1:
                    {
                        HWSyncInit* pInit;

                        //reset vsync cnt
                        pInit = new HWSyncInit(this);
                        pInit->DeviceInfo(HWSyncInit::E_RESET_VSYNC_CNT,0);
                        delete pInit;
                    }
                    break;
                case 2:
                    {
                        SensorDynamicInfo dynamicInfo;
                        this->mHalSensor->querySensorDynamicInfo(arg2,&dynamicInfo);
                        for(MUINT32 i=0;i<this->Dev_temp.size();i++){
                            LOG_INF("previous configured_%d devID:%d,shutter:%d",i, this->Dev_temp[i],\
                                this->TG_Shutter[i]);
                        }
                        LOG_INF("2nd lunch devID:%llu, TG:%d", (long long)arg2, dynamicInfo.TgInfo);
                        //change order
                        if(this->Dev_temp[0] == arg2){
                            this->Dev_temp[0] = this->Dev_temp[0] ^ this->Dev_temp[1];
                            this->Dev_temp[1] = this->Dev_temp[0] ^ this->Dev_temp[1];
                            this->Dev_temp[0] = this->Dev_temp[0] ^ this->Dev_temp[1];
                            this->TG_Shutter[0] = this->TG_Shutter[0] ^ this->TG_Shutter[1];
                            this->TG_Shutter[1] = this->TG_Shutter[0] ^ this->TG_Shutter[1];
                            this->TG_Shutter[0] = this->TG_Shutter[0] ^ this->TG_Shutter[1];
                            LOG_INF("Change ord:Dev_temp[0]=%d,Dev_temp[1]=%d",this->Dev_temp[0],this->Dev_temp[1]);
                        }

                        if(arg3 == HW_SYNC_BLOCKING_EXE){
                            //start init flow
                            changeMode(MTRUE);
                        }
                        else{
                            LOG_INF("start HWSync without init sync");
                            changeMode(MFALSE);
                        }
                        this->checkNeedSync_INIT();

                        pthread_mutex_lock(&mStateMutex);
                        mState=HW_SYNC_STATE_READY2RUN;
                        LOG_INF("mState (%d)",mState);
                        pthread_mutex_unlock(&mStateMutex);
                    }
                    break;
                default:
                    ret = 1;
                    break;
            }

            break;

        /**
         * Notice!!!
         * AE only call HW_SYNC_CMD_GET_FRAME_TIME once for two sensors and their device order of exp time is according to arg1's device
         * That is, the arg1 device id is the 1st element of expTime.
         * But, in hwsync design, 1st element of expTime and frameTime is TG1's device. The order may be different to AE's order.
         * So,
         * (1) get 1st element of expTime to be same as arg1's device and
         * (2) return 1st element of frameTime be same as arg1's device (not TG1)
         *
         */
        case HW_SYNC_CMD_GET_FRAME_TIME:
            {
                    MUINT32* p_expTime = (MUINT32*)arg2;
                    MUINT32* p_frmTime = (MUINT32*)arg3;

                    // get the hwsync index of arg1's device id
                    // if hwsync index of arg1's device = 1, then swap exp time for sync AE order
                    // i.e. AE 1st element is tg2, 2nd element is tg1
                    if(1 == mSensorDevMapHwsyncIndex[arg1]){
                        // swap exp time
                        *(p_expTime + 0) = *(p_expTime + 0) ^ *(p_expTime + 1);
                        *(p_expTime + 1) = *(p_expTime + 0) ^ *(p_expTime + 1);
                        *(p_expTime + 0) = *(p_expTime + 0) ^ *(p_expTime + 1);
                    }

                    this->m2ndData.expTime[0] = *(p_expTime + 0);
                    this->m2ndData.expTime[1] = *(p_expTime + 1);

                    LOG_DBG("senDev(%d), expTime[0]=%d, expTime[1]=%d, frmTime[0]=%d, frmTime[1]=%d",arg1, *(p_expTime+0),*(p_expTime+1),*(p_frmTime+0),*(p_frmTime+1));
                    ret = checkNeedSync_AE(arg1, arg2, arg3);

                    // if hwsync index of arg1's device = 1, then swap exp/frame time for sync AE order
                    // i.e. index 0 is tg2, index 1 is tg1
                    if(1 == mSensorDevMapHwsyncIndex[arg1]){
                        // swap frame time
                        *(p_frmTime + 0) = *(p_frmTime + 0) ^ *(p_frmTime + 1);
                        *(p_frmTime + 1) = *(p_frmTime + 0) ^ *(p_frmTime + 1);
                        *(p_frmTime + 0) = *(p_frmTime + 0) ^ *(p_frmTime + 1);

                        // swap exp time
                        *(p_expTime + 0) = *(p_expTime + 0) ^ *(p_expTime + 1);
                        *(p_expTime + 1) = *(p_expTime + 0) ^ *(p_expTime + 1);
                        *(p_expTime + 0) = *(p_expTime + 0) ^ *(p_expTime + 1);
                    }
            }
            break;
        case HW_SYNC_CMD_SYNC_TYPE:
            {
                *((MUINTPTR*)arg2) = (MUINT32)SYNC_TYPE;
                ret = 0;
            }
            break;
    }
    if(ret){
        LOG_ERR("cmd fail_%s",str(cmd));
    }
    LOG_DBG("-cmd(%s)",str(cmd));
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HWSyncDrvImp::changeMode(MBOOL bTrigInitFlow)
{
    LOG_DBG("+");
    MINT32 ret=0;

    //[1]get sensor initial information
    MUINT32 periodPara=0x0;
    MINT32 pclk1=0,pclk2=0;
    MUINT32 InitperiodSrc[E_TG_MAX][2];
    ret=mHalSensor->sendCommand(mSensorDevArray[0],SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,(MINTPTR)&pclk1,0,0);
    if(ret<0)
    {
        LOG_INF("get pclk of sensorDev(0x%x) failed",mSensorDevArray[0]);
        return -1;
    }

    ret=mHalSensor->sendCommand(mSensorDevArray[0],SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,(MINTPTR)&periodPara,0,0);
    if(ret<0)
    {
        LOG_ERR("get initial period information of sensorDev(0x%x) failed",mSensorDevArray[0]);
    }
    InitperiodSrc[E_TG_A][0] =0x0000FFFF & periodPara;
    InitperiodSrc[E_TG_A][1] =(0xFFFF0000 & periodPara)>>16;

    periodPara=0x0;

    ret=mHalSensor->sendCommand(mSensorDevArray[1],SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,(MINTPTR)&pclk2,0,0);
    if(ret<0)
    {
        LOG_INF("get pclk of sensorDev(0x%x) failed",mSensorDevArray[0]);
        return -1;
    }

    ret=mHalSensor->sendCommand(mSensorDevArray[1],SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,(MINTPTR)&periodPara,0,0);
    if(ret<0)
    {
        LOG_ERR("get initial period information of sensorDev(0x%x) failed",mSensorDevArray[1]);
    }
    InitperiodSrc[E_TG_B][0]=0x0000FFFF & periodPara;
    InitperiodSrc[E_TG_B][1]=(0xFFFF0000 & periodPara)>>16;



    LOG_INF("Init info: dev_0x%x/dev_0x%x pclk(%d/%d),period(%d,%d/%d,%d),fps(%d/%d us),frmtime_delay(%d/%d),isp_clk(%d)",\
        mSensorDevArray[0],mSensorDevArray[1],pclk1,pclk2,\
        InitperiodSrc[E_TG_A][0],InitperiodSrc[E_TG_A][1],InitperiodSrc[E_TG_B][0],InitperiodSrc[E_TG_B][1],\
        (InitperiodSrc[E_TG_A][0]*(InitperiodSrc[E_TG_A][1]))*100/(pclk1/10000),\
        (InitperiodSrc[E_TG_B][0]*(InitperiodSrc[E_TG_B][1]))*100/(pclk2/10000), \
        mFrameTimeDelayFrame[0], mFrameTimeDelayFrame[1], SMI_CLOCK(this->m_DVFS) / 1000);

    if(bTrigInitFlow)
        ret=doOneTimeSync();
    else{
        //have init maxframeTime
        MUINT32 framerate;
        this->mHalSensor->sendCommand(this->mSensorDevArray[E_TG_A],\
                    SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,(MINTPTR)&this->mSensorScenArray[E_TG_A],\
                    (MINTPTR)&framerate,0);

        this->mHalSensor->sendCommand(this->mSensorDevArray[E_TG_B],\
                    SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,(MINTPTR)&this->mSensorScenArray[E_TG_B],\
                    (MINTPTR)&this->m_MaxFrameTime,0);

        this->m_MaxFrameTime = (framerate > this->m_MaxFrameTime)?(this->m_MaxFrameTime):\
            (framerate);

        this->m_MaxFrameTime = 1000000 / (this->m_MaxFrameTime/10);
    }

    LOG_DBG("-");
    return ret;
}

MUINT32 HWSyncDrvImp::checkNeedSync_INIT(void)
{
    //reset record to avoid current scenario is affected by the result of previous scenario
    this->mPreAdjustFrmTime = MFALSE;
    this->mCurFrameTime[0] = this->mCurFrameTime[1] = 0xFFFFFFFF;

    return 0;
}

/**
 * 1. Check wether need to do general sync first
 * 2.
    (A)If need to do general sync, perform the following and return frame_time

     * GENERAL SYNC: To adjust vsync timing to let to sensor's vsync can align
     *
     * Description:
     * 1. @n frame, adjust the leading one sensor where its vsync is leading @ (n+2)
     * 2. @n+1 frame, adjust the frame time of sensor to be the slower one
     *
     * Sync time is @n+2 and n+3 frame

     (B)If don't need to do general sync, return 0
     (C)If error is happened, return 0 value

 */
MINT32 HWSyncDrvImp::checkNeedSync_AE(MUINT32 arg1, MUINTPTR arg2, MUINTPTR arg3)
{


    MINT32 ret=0;
    static MUINT32 pre_aeExp[2];
    SensorN3dDiffCnt _N3DregInfo;
    MUINT32 order=0x0;
    MUINT32 n3dDiffCnt=0x0;
    MUINT32 cnt1=0, cnt2=0;
    MUINT32 expTime_MAIN1, expTime_MAIN2;
    ExecCmd nextCmd;
    MUINT32* pp_expTime = (MUINT32*)arg2;
    MUINT32* pp_frmTime = (MUINT32*)arg3;
    MUINT32 sofCnt[2];
    MBOOL PreAdjFrmTimeFromDarkToLight = MFALSE;

    expTime_MAIN1 = *(pp_expTime+0);
    expTime_MAIN2 = *(pp_expTime+1);


    // Set adjust frame time to be 0 first
    *(pp_frmTime+0) = 0;
    *(pp_frmTime+1) = 0;
    LOG_DBG("+");

    pthread_mutex_lock(&mStateMutex);
    if( mState==HW_SYNC_STATE_IDLE)
    {
        pthread_mutex_unlock(&mStateMutex);
        LOG_INF("State is in disable sync, don't need to check sync!!");
        ::sem_post(&mSemGeneralSyncDoneMain1);
        ::sem_post(&mSemGeneralSyncDoneMain2);
        goto EXIT;
    }
    else
    {
        //LOG_DBG("mState (%d)",mState);
        pthread_mutex_unlock(&mStateMutex);
    }

    if(0xffffffff == this->mAECyclePeriod) {
        LOG_ERR("ERROR!! mAECyclePeriod is not set!!! Please called init() to set this value!");
        goto EXIT;
    }

#if 0
    //if AE exposure changes too much. don't do general sync
    if((0 != pre_aeExp[0]) && (0 != pre_aeExp[1])){

        if((expTime_MAIN1 > pre_aeExp[0]*2) || (expTime_MAIN1*2 < pre_aeExp[0])){
            LOG_INF("[MAIN1]no adj, pre_exp(%d/%d),exp(%d/%d)", pre_aeExp[0], pre_aeExp[1], expTime_MAIN1, expTime_MAIN2);
            pre_aeExp[0] = expTime_MAIN1;
            pre_aeExp[1] = expTime_MAIN2;
            goto EXIT;
        }
        else if((expTime_MAIN2 > pre_aeExp[1]*2) || (expTime_MAIN2*2 < pre_aeExp[1])){
            LOG_INF("[MAIN2]no adj, pre_exp(%d/%d),exp(%d/%d)", pre_aeExp[0], pre_aeExp[1], expTime_MAIN1, expTime_MAIN2);
            pre_aeExp[0] = expTime_MAIN1;
            pre_aeExp[1] = expTime_MAIN2;
            goto EXIT;
        }
    }
#endif

    // Observe vs diff every time
    ret=!!mHalSensor->sendCommand(mSensorDevArray[0],SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT,(MINTPTR)&_N3DregInfo,0,0);
    if(ret)
    {
        LOG_ERR("query N3D diff fail");
        goto EXIT;
    }

    mN3dInfo.order = (_N3DregInfo.u4DiffCnt & 0x80000000)>>31;  //current frame leading
    mN3dInfo.vdDiffCnt = _N3DregInfo.u4DiffCnt & 0x7fffffff;    //preframe diff
    mN3dInfo.vs_regCnt[0] =_N3DregInfo.u4Vsync1Cnt;
    mN3dInfo.vs_regCnt[1] =_N3DregInfo.u4Vsync2Cnt;
    mN3dInfo.vs2_vs1Cnt =_N3DregInfo.u4DebugPort;

    order=(_N3DregInfo.u4DiffCnt & 0x80000000)>>31;
    if(order==1)
    {//vsync1 leads vsync2
        n3dDiffCnt=(_N3DregInfo.u4Vsync2Cnt-_N3DregInfo.u4DebugPort);
    }
    else if(order==0)
    {//vsync2 leads vsync1
        n3dDiffCnt=_N3DregInfo.u4DebugPort;
    }

    cnt1 = this->calPixCnt2US(_N3DregInfo.u4Vsync1Cnt);
    cnt2 = this->calPixCnt2US(_N3DregInfo.u4Vsync2Cnt);

    // Get current sof cnf of main1/2
    sofCnt[0] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_A);
    sofCnt[1] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_B);


    // If hwsync thread adjust frm time in previes time, then we need to use the adjust value to regard as cur frame time
    // Cause when delay frame of frametime to a sensor is N + 2 and the adjust time @N+1
    // Then the N3D counter will be the old value @ N + 3 since it will be take effect @N+4
    // In this case, cannot use n3d counter VS value to regard as A/B
    if(this->mPreAdjustFrmTime == MTRUE) {
        this->mCurFrameTime[0] = this->m2ndData.FrameTime[0];
        this->mCurFrameTime[1] = this->m2ndData.FrameTime[1];
    } else {
        this->mCurFrameTime[0] = cnt1;
        this->mCurFrameTime[1] = cnt2;
    }

    //pp_frmTime can't be 0. need to have current or previous adjusted result.
    *(pp_frmTime+0) = this->mCurFrameTime[0];
    *(pp_frmTime+1) = this->mCurFrameTime[1];

    LOG_INF("order(0x%x), V-diff(%d_%d us), Pre_FrmTime(%d/%d us), Cur_FrmTime(%d/%d us) pre_exp(%d/%d),exp(%d/%d), SOF(%d/%d)",\
        order,this->calPixCnt2US(n3dDiffCnt),this->calPixCnt2US(_N3DregInfo.u4DebugPort),\
        cnt1,cnt2,this->mCurFrameTime[0], this->mCurFrameTime[1],  \
        pre_aeExp[0], pre_aeExp[1], expTime_MAIN1, expTime_MAIN2, sofCnt[0], sofCnt[1]);


    pre_aeExp[0] = expTime_MAIN1;
    pre_aeExp[1] = expTime_MAIN2;


    // n3dDiffCnt<DIFF_THRECNT_NEW: Don't need to do general sync
    if(n3dDiffCnt<DIFF_THRECNT_NEW(this->m_DVFS))
    {

        // For case that shutter < frametime && frametime > init frame time
        // where this case is move device from dark place to light place (e.x.: 15fps -> 24fps)
        // Then the frame time need to be adjusted to 24 fps or the frame rate will be keep in 15fps
        // Note: add 3 ms to this->m_MaxFrameTime for avoding some rounding issue
        if(expTime_MAIN1 < cnt1 && expTime_MAIN2 < cnt2 && \
           expTime_MAIN1 < this->m_MaxFrameTime && expTime_MAIN2 < this->m_MaxFrameTime && \
           cnt1 > this->m_MaxFrameTime + 3000 && cnt2 > this->m_MaxFrameTime + 3000 && \
           PreAdjFrmTimeFromDarkToLight == MFALSE)
        {
            // decide frame time 2a/b to be max(largerExpTime, initFrameTime)
            this->m2ndData.FrameTime[1] = this->m2ndData.FrameTime[0] = this->m_MaxFrameTime;

            LOG_INF("From dark to light! Need adjust frametime! exp(%d/%d),VS(%d/%d),adjFrmTime(%d/%d)", \
                expTime_MAIN1, expTime_MAIN2, cnt1, cnt2, this->m2ndData.FrameTime[0], this->m2ndData.FrameTime[1]);

            //[3] add next cmd and return
            nextCmd=ExecCmd_SET_SYNC_FRMTIME_MAIN1;
            addExecCmdMain1(nextCmd);
            nextCmd=ExecCmd_SET_SYNC_FRMTIME_MAIN2;
            addExecCmdMain2(nextCmd);

            // record adj frame time in previous checkNeedSync. Cause the effect time  may in N+4 frame and we check every 3 SOF time
            // If still check it will need to adjust again. It is no use to adjust it again.
            PreAdjFrmTimeFromDarkToLight = MTRUE;
            this->mPreAdjustFrmTime = MTRUE;
        }
        else {
            LOG_DBG("n3dDiffCnt<DIFF_THRECNT_NEW: Don't need to do general sync");
            PreAdjFrmTimeFromDarkToLight = MFALSE;
            this->mPreAdjustFrmTime = MFALSE;
        }
        goto EXIT;
    }

#if 0
    //do not do general sync if fps difference of two sensors is too large(>10 ms)
    if(cnt1 > cnt2)
    {
        if((cnt1-cnt2) >= 10000)
        {
            LOG_INF("fps difference too large, do not sync(%d/%d) us",cnt1,cnt2);
            goto EXIT;
        }
    }
    else
    {
        if((cnt2-cnt1) >= 10000)
        {
            LOG_INF("fps difference too large, do not sync(%d/%d) us",cnt1,cnt2);
            goto EXIT;
        }
    }
#endif

    // Don't do general sync if previous PreAdjFrmTimeFromDarkToLight is performed
    // Or in current general sync. the n3d counter will still not update. And will be wrong in this case
    if(PreAdjFrmTimeFromDarkToLight != MTRUE) {
        LOG_INF("Need Sync");
        if(doGeneralSync_AE(arg1, arg2, arg3)) {
            //[3] add next cmd and return
            nextCmd=ExecCmd_SET_SYNC_FRMTIME_MAIN1;
            addExecCmdMain1(nextCmd);
            nextCmd=ExecCmd_SET_SYNC_FRMTIME_MAIN2;
            addExecCmdMain2(nextCmd);
        }
    }

    this->mPreAdjustFrmTime = MTRUE;

EXIT:
    LOG_DBG("-");

    return ret;
}



MUINT32 HWSyncDrvImp::calPixCnt2US(MUINT32 n3dCnt)
{
    MUINT64 time;
    time = (MUINT64)n3dCnt * 1000   + (SMI_CLOCK(this->m_DVFS) / 1000) - 1;
    time = time / (SMI_CLOCK(this->m_DVFS) / 1000);

    if(time > 0xFFFFFFFF){
        LOG_ERR("pix2time over flow:0x%x",n3dCnt);
    }

    return (MUINT32)time;
}



/*******************************************************************************
*
********************************************************************************/
MINT32 HWSyncDrvImp::calCurrentFps(int index)
{
    //[1]get sensor information
    int ret=0;
    MUINT32 periodPara=0x0;
    MUINT32 pclk=0;
    MUINT32 period[2];
    ret=mHalSensor->sendCommand(mSensorDevArray[index],SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,(MUINTPTR)&pclk,0,0);
    if(ret<0)
    {
        LOG_INF("get pclk of sensorDev(0x%x) failed",mSensorDevArray[index]);
        return -1;
    }
    ret=mHalSensor->sendCommand(mSensorDevArray[index],SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,(MUINTPTR)&periodPara,0,0);
    if(ret<0)
    {
        LOG_ERR("get initial period information of sensorDev(0x%x) failed",mSensorDevArray[index]);
    }
    period[0]=0x0000FFFF & periodPara;
    period[1]=(0xFFFF0000 & periodPara)>>16;

    //[2] calculate fps
    MINT32 fps=0;
    fps=(pclk*1.0)/(period[0]*period[1])*10;

    LOG_INF("idx(%d) pclk(%d),period(%d,%d), fps(%d)",index,pclk,period[0],period[1],fps);

    return fps;
}

/*******************************************************************************
*
********************************************************************************/

MVOID* HWSyncDrvImp::DBG_Thread::DBG_Trig(MVOID* arg)
{
    HWSyncDrvImp *_this = reinterpret_cast<HWSyncDrvImp*>(arg);
    int const policy    = SCHED_OTHER;
    int const priority  = ANDROID_PRIORITY_NORMAL-2;
    struct sched_param sched_p;
    ISP_WAIT_IRQ_ST waitIrq;
    ISP_REGISTER_USERKEY_STRUCT regirq;
    SensorN3dDiffCnt N3DregInfo;
    MUINT32 m,n;


    LOG_INF("start +[%d]",_this->m_DBG_t.m_working_tg);


    ::sched_getparam(0, &sched_p);
    //
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    //
    waitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
    waitIrq.St_type = SIGNAL_INT;
    waitIrq.Status = VS_INT_ST;
    waitIrq.Timeout = 1000; // 150 msec
    sprintf((char*)regirq.userName,LOG_TAG);
    _this->m_pIspDrv[_this->m_DBG_t.m_working_tg]->registerIrq(&regirq);
    waitIrq.UserKey = regirq.userKey;


    //check sync result
    for(m=0;m<2;m++){
        if (_this->m_pIspDrv[_this->m_DBG_t.m_working_tg]->waitIrq(&waitIrq) <= 0){
            LOG_ERR("[%d]:wait vsync timeout",_this->m_DBG_t.m_working_tg);

            goto EXIT;
        }

        if(!_this->m_pIspDrv[_this->m_DBG_t.m_working_tg]->getDeviceInfo(_GET_VSYNC_CNT,(MUINT8 *)&n)) {
            LOG_ERR("_GET_VSYNC_CNT error!!!!!!!");
        }
        LOG_INF("[%d]:vsync counter:%d",_this->m_DBG_t.m_working_tg,n);
    }
    //
    if(_this->mHalSensor->sendCommand(_this->mSensorDevArray[_this->m_DBG_t.m_working_tg],\
        SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT,(MINTPTR)&N3DregInfo,0,0) < 0){
        LOG_ERR("[%d]:query N3D diff fail",_this->mSensorDevArray[_this->m_DBG_t.m_working_tg]);

        goto EXIT;
    }

    m = (N3DregInfo.u4Vsync1Cnt > N3DregInfo.u4Vsync2Cnt) ? \
    (N3DregInfo.u4Vsync1Cnt - N3DregInfo.u4Vsync2Cnt) : \
    (N3DregInfo.u4Vsync2Cnt - N3DregInfo.u4Vsync1Cnt);

    m = _this->calPixCnt2US(m);
    n = _this->calPixCnt2US(N3DregInfo.u4DebugPort);

    if(n > (_this->m_MaxFrameTime >> 1) ){
        n = _this->m_MaxFrameTime - n;
    }

    LOG_INF("HWSync result: ord:%d. period diff:%d us. vsync diff:%d us. [%d_%d_%d in us]",\
        (N3DregInfo.u4DiffCnt & 0x80000000)>>31,m,n,\
        _this->calPixCnt2US(N3DregInfo.u4Vsync1Cnt),_this->calPixCnt2US(N3DregInfo.u4Vsync2Cnt),\
        _this->calPixCnt2US(N3DregInfo.u4DebugPort));

    //check sync result
    for(m=0;m<1;m++){
        if (_this->m_pIspDrv[_this->m_DBG_t.m_working_tg]->waitIrq(&waitIrq) <= 0){
            LOG_ERR("[%d]:wait vsync timeout",_this->m_DBG_t.m_working_tg);

            goto EXIT;
        }

        if(!_this->m_pIspDrv[_this->m_DBG_t.m_working_tg]->getDeviceInfo(_GET_VSYNC_CNT,(MUINT8 *)&n)) {
            LOG_ERR("_GET_VSYNC_CNT error!!!!!!!");
        }
        LOG_INF("[%d] vsync counter:%d",_this->m_DBG_t.m_working_tg,n);
    }
    //
    if(_this->mHalSensor->sendCommand(_this->mSensorDevArray[_this->m_DBG_t.m_working_tg],\
        SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT,(MINTPTR)&N3DregInfo,0,0) < 0){
        LOG_ERR("[%d]:query N3D diff fail",_this->mSensorDevArray[_this->m_DBG_t.m_working_tg]);

        goto EXIT;
    }

    m = (N3DregInfo.u4Vsync1Cnt > N3DregInfo.u4Vsync2Cnt) ? \
    (N3DregInfo.u4Vsync1Cnt - N3DregInfo.u4Vsync2Cnt) : \
    (N3DregInfo.u4Vsync2Cnt - N3DregInfo.u4Vsync1Cnt);

    m = _this->calPixCnt2US(m);
    n = _this->calPixCnt2US(N3DregInfo.u4DebugPort);

    if(n > (_this->m_MaxFrameTime >> 1) ){
        n = _this->m_MaxFrameTime - n;
    }

    LOG_INF("HWSync result: ord:%d. period diff:%d us. vsync diff:%d us. [%d_%d_%d in clk]",\
        (N3DregInfo.u4DiffCnt & 0x80000000)>>31,m,n,\
        _this->calPixCnt2US(N3DregInfo.u4Vsync1Cnt),_this->calPixCnt2US(N3DregInfo.u4Vsync2Cnt),\
        _this->calPixCnt2US(N3DregInfo.u4DebugPort));
EXIT:
    pthread_detach(pthread_self());
    return NULL;
}


MINT32 HWSyncDrvImp::doOneTimeSync()
{
    MINT32 ret = 0;
    HWSyncInit* pInit = NULL;
    MUINT32 r;
    SensorDynamicInfo dynamicInfo[2];
    LOG_INF("+");


    if( (pInit = new HWSyncInit((void*)this)) == NULL){
        LOG_ERR("MEM Init fail");
        return 1;
    }

    this->mHalSensor->querySensorDynamicInfo(this->Dev_temp[0],&dynamicInfo[0]);
    this->mHalSensor->querySensorDynamicInfo(this->Dev_temp[1],&dynamicInfo[1]);

    for(int i = 0; i < 2; i++){

        if(dynamicInfo[i].TgInfo == CAM_TG_1){
            pInit->m_Shutter[E_TG_A] = this->TG_Shutter[i];//fps here is shutter speed
        }
        else if(dynamicInfo[i].TgInfo == CAM_TG_2){
            pInit->m_Shutter[E_TG_B] = this->TG_Shutter[i];
        }
        else{
            LOG_ERR("unsupported TG:%d",dynamicInfo[i].TgInfo);
        }
    }

    if(dynamicInfo[0].TgInfo == CAM_TG_1){
        pInit->mThread1_TG = E_TG_A;
        pInit->mThread2_TG = E_TG_B;
    }
    else if(dynamicInfo[0].TgInfo == CAM_TG_2){
        pInit->mThread1_TG = E_TG_B;
        pInit->mThread2_TG = E_TG_A;
    }
    else{
        pInit->mThread1_TG = E_TG_A;
        pInit->mThread2_TG = E_TG_B;
        LOG_ERR("unsupported TG:%d",dynamicInfo[0].TgInfo);
    }

    if(pInit->Trig() == MFALSE){
        LOG_ERR("HWSync init flow fail");
        ret = 1;
    }

    delete pInit;
    //
    if(dynamicInfo[0].TgInfo == CAM_TG_1){
        this->m_DBG_t.m_working_tg = E_TG_B;
    }
    else{
        this->m_DBG_t.m_working_tg = E_TG_A;
    }

    if ((r = pthread_create(&this->m_DBG_t.DBG_T, NULL, DBG_Thread::DBG_Trig, this)) != 0) {
        LOG_ERR("thread create [%s] fail: %d", "DBG_Trig create", r);
        return MFALSE;
    }

    LOG_INF("-");
    return ret;
}

/**
 * GENERAL SYNC AE: To adjust vsync timing to let to sensor's vsync can align
 *  Need to take framelength delay time of sensor into account: D(A) --  delay of main1 or D(B) --  delay of main2
 *
 * Description:
 * 1. @n frame, adjust the leading one sensor where its vsync is leading, which will take effect @n + delay frame
 * 2. adjust the frame time of sensor to be the slower one via 2 hwsync thread
 *
 * Sync time is @n+2 or n+3 frame
 *
 * Required: framelength delay time of sensor cannot >= 3
 */

MUINT32 HWSyncDrvImp::doGeneralSync_AE(MUINT32 arg1, MUINTPTR arg2, MUINTPTR arg3)
{
    LOG_DBG("+");
    arg1;
    MUINT32 ret=1;
    E_N3D_SOURCE adj_sensor = E_TG_A;
    MINT32 frmTimeDiffPrd;
    MUINT32 frmTime_1[MAX_SEN] = {0, 0}; // vs1' or vs2'
    MUINT32 sofCnt[2];
    MUINT32 vdiff = 0;
    MUINT32* ppp_expTime = (MUINT32*)arg2;
    MUINT32* ppp_frmTime = (MUINT32*)arg3;
    MUINT32 largerExpTime = (*(ppp_expTime+0) >= *(ppp_expTime+1)) ? *(ppp_expTime+0) : *(ppp_expTime+1);
    #define ADJUST_FRAME_TIME_EXP_MARGIN_ADD_TIME 100 // for adjust frame time when exp time is larger, then add 0.1ms for margin value
    MUINT32 FinalSyncFrmT = (this->m_MaxFrameTime >= largerExpTime) ? (this->m_MaxFrameTime)\
                                : (largerExpTime + ADJUST_FRAME_TIME_EXP_MARGIN_ADD_TIME);
    //[1] state is syncing
    pthread_mutex_lock(&mStateMutex);
    mState=HW_SYNC_STATE_SYNCING;
    pthread_mutex_unlock(&mStateMutex);


    // Get current sof cnf of main1/2
    sofCnt[0] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_A);
    sofCnt[1] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_B);

    if(1 == mN3dInfo.order) {
        vdiff = this->calPixCnt2US(mN3dInfo.vs_regCnt[1] - mN3dInfo.vs2_vs1Cnt); // VD = pre_B - vs2_vs1Cnt
    } else {
        vdiff = this->calPixCnt2US(mN3dInfo.vs2_vs1Cnt); // VD = vs2_vs1Cnt
    }

    /// 1. @n frame, adjust the leading one sensor where its vsync is leading @ (n+ framelength delay frame)

    if(this->mFrameTimeDelayFrame[0] > MAX_SUPPORT_FRAMELENGTH_DELAY_NUM || this->mFrameTimeDelayFrame[1] > MAX_SUPPORT_FRAMELENGTH_DELAY_NUM || \
       this->mFrameTimeDelayFrame[0] == 0 || this->mFrameTimeDelayFrame[1] == 0) {
        LOG_ERR("current frame-delay(%d_%d) r not support frame delay num, max(%d)", \
            this->mFrameTimeDelayFrame[0], this->mFrameTimeDelayFrame[1],\
            MAX_SUPPORT_FRAMELENGTH_DELAY_NUM);

        ret = 0;
        goto EXIT;
    }
    else if( 1 == this->mFrameTimeDelayFrame[0] && 1 == this->mFrameTimeDelayFrame[1]) {
        // vs1 is leading

        if(1 == mN3dInfo.order) {

            // 2A - (2B + VD)
            frmTimeDiffPrd = 2 * this->mCurFrameTime[0] - ( 2 * this->mCurFrameTime[1] + vdiff);

            // if 2A - (2B + VD) <= 0, adjust vs1
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[E_TG_A] = frmTimeDiffPrd * -1 + this->mCurFrameTime[E_TG_A]; // A' = -(2A - (2B + VD)) + A
            }
            // if 2A - (2B + VD) > 0, adjust vs2
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[E_TG_B] = frmTimeDiffPrd + this->mCurFrameTime[E_TG_B]; // B' = 2A - (2B + VD) + B
            }

        }
        // VS2 is leading
        else {

            // (2A + VD) - 2B
            frmTimeDiffPrd =  2 * this->mCurFrameTime[0] + vdiff - 2 * this->mCurFrameTime[1];

            // if (2A + VD) - 2B <= 0, adjust vs1
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[adj_sensor] = frmTimeDiffPrd * -1 + this->mCurFrameTime[0]; // A' = -((2A + VD) - 2B) + A
            }
            // if 2B - (2A + VD) <= 0, adjust vs2
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[adj_sensor] = frmTimeDiffPrd + this->mCurFrameTime[1]; // B' = (2A + VD) - 2B + B
            }
        }
    }
    // Current case of N3d: main1 -- D(A) = 2 imx258; main1 -- main2 -- D(B) = 1 s5k5e2
    else if( 2 == this->mFrameTimeDelayFrame[0] && 1 == this->mFrameTimeDelayFrame[1]) {
        // vs1 is leading
        if(1 == mN3dInfo.order) {

            // 2A - (2B + VD) + (A - B) = 3A - 3B - VD
            frmTimeDiffPrd = 3 * this->mCurFrameTime[0] - 3 * this->mCurFrameTime[1] - vdiff;

            // if 3A - 3B - VD <= 0, adjust vs1
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[adj_sensor] = 3 * this->mCurFrameTime[1] - 2 * this->mCurFrameTime[0] + vdiff; // A' = 3B + VD - 2A
            }
            // if 3A - 3B - VD > 0, adjust vs2
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[adj_sensor] = (3 * this->mCurFrameTime[0] - this->mCurFrameTime[1] - vdiff) / 2; // B' = (3A - B - VD) / 2
            }

        }
        // VS2 is leading
        else {

            // (2A + VD) - 2B + (A - B) = 3A + VD - 3B
            frmTimeDiffPrd =  3 * this->mCurFrameTime[0] + vdiff - 3 * this->mCurFrameTime[1];

            // if 3A + VD - 3B <= 0, adjust vs1
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[adj_sensor] = 3 * this->mCurFrameTime[1] - 2 * this->mCurFrameTime[0] - vdiff;; // A' = 3B - 2A - VD
            }
            // if 3A + VD - 3B, adjust vs2
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[adj_sensor] = (3 * this->mCurFrameTime[0] - this->mCurFrameTime[1] + vdiff) / 2; // B' = (3A - B + VD) / 2
            }

        }
    }
    else if( 1 == this->mFrameTimeDelayFrame[0] && 2 == this->mFrameTimeDelayFrame[1]) {
        // vs1 is leading
        if(1 == mN3dInfo.order) {

            // 2A - (2B + VD) + (A - B) = 3A - 3B - VD
            frmTimeDiffPrd = 3 * this->mCurFrameTime[0] - 3 * this->mCurFrameTime[1] - vdiff;

            // if 3A - 3B - VD <= 0, adjust vs1
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[adj_sensor] = (3 * this->mCurFrameTime[1] - this->mCurFrameTime[0] + vdiff) / 2; // A' = (3B - A + VD) / 2
            }
            // if 3A - 3B - VD > 0, adjust vs2
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[adj_sensor] = 3 * this->mCurFrameTime[0] - 2 * this->mCurFrameTime[1] - vdiff; // B' = 3A - 2B - VD
            }

        }
        // VS2 is leading
        else {

            // (2A + VD) - 2B + (A - B) = 3A + VD - 3B
            frmTimeDiffPrd =  3 * this->mCurFrameTime[0] + vdiff - 3 * this->mCurFrameTime[1];

            // if 3A + VD - 3B <= 0, adjust vs1
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[adj_sensor] = (3 * this->mCurFrameTime[1] - this->mCurFrameTime[0] - vdiff) / 2;; // A' = (3B - A - VD) / 2
            }
            // if 3A + VD - 3B, adjust vs2
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[adj_sensor] = 3 * this->mCurFrameTime[0] - 2 * this->mCurFrameTime[1] + vdiff; // B' = 3A - 2B + VD
            }

        }
    }
    else if( 2 == this->mFrameTimeDelayFrame[0] && 2 == this->mFrameTimeDelayFrame[1]) {
        // vs1 is leading
        if(1 == mN3dInfo.order) {

            // 2A - (2B + VD) + (A - B) = 3A - 3B - VD
            frmTimeDiffPrd = 3 * this->mCurFrameTime[0] - 3 * this->mCurFrameTime[1] - vdiff;

            // if 3A - 3B - VD <= 0, adjust vs1
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[adj_sensor] = (frmTimeDiffPrd * -1) + this->mCurFrameTime[0]; // A' = 3B - 2A + VD
            }
            // if 3A - 3B - VD > 0, adjust vs2
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[adj_sensor] = frmTimeDiffPrd + this->mCurFrameTime[1]; // B' = 3A - 2B - VD
            }

        }
        // VS2 is leading
        else {

            // (2A + VD) - 2B + (A - B) = 3A + VD - 3B
            frmTimeDiffPrd =  3 * this->mCurFrameTime[0] + vdiff - 3 * this->mCurFrameTime[1];

            // if 3A + VD - 3B <= 0, adjust vs1
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[adj_sensor] = (frmTimeDiffPrd * -1) + this->mCurFrameTime[0]; // A' = 3B - 2A - VD
            }
            // if 3A + VD - 3B, adjust vs2
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[adj_sensor] = frmTimeDiffPrd + this->mCurFrameTime[1]; // B' = 3A - 2B + VD
            }

        }
    }

    // Check wether AE exp time is larger than adj frame time. If true, we need adjust frame time tobe
    // shutter + (adj frame time - orig frame time)

    if(*(ppp_expTime+adj_sensor) > frmTime_1[adj_sensor]) {
        frmTime_1[adj_sensor] = *(ppp_expTime+adj_sensor) + (frmTime_1[adj_sensor] - this->mCurFrameTime[adj_sensor]);
    }


    LOG_INF("adjSenIdx(0x%x), adjFrmT(%d) us, PreFrmTime(%d/%d) us, CurFrmTime(%d/%d) us, VD(%d) us, order(%d), SOF(%d/%d)",adj_sensor,\
        frmTime_1[adj_sensor], \
        this->calPixCnt2US(mN3dInfo.vs_regCnt[0]),this->calPixCnt2US(mN3dInfo.vs_regCnt[1]), \
        this->mCurFrameTime[0], this->mCurFrameTime[1], \
        vdiff, mN3dInfo.order, sofCnt[0], sofCnt[1]);

    if(frmTime_1[0])
        *(ppp_frmTime+0) = frmTime_1[0];
    if(frmTime_1[1])
        *(ppp_frmTime+1) = frmTime_1[1];


    LOG_INF("dev0/dev1(0x%x/0x%x), expTime(%d/%d), frmTime_1(%d/%d), frmTime_2(%d/%d), delay(%d/%d)",\
        this->mSensorDevArray[0], this->mSensorDevArray[1], \
        *(ppp_expTime+0),*(ppp_expTime+1),*(ppp_frmTime+0),*(ppp_frmTime+1), \
        FinalSyncFrmT, FinalSyncFrmT, \
        this->mFrameTimeDelayFrame[0], this->mFrameTimeDelayFrame[1]);

    this->m2ndData.FrameTime[0] = FinalSyncFrmT;
    this->m2ndData.FrameTime[1] = FinalSyncFrmT;

EXIT:
    LOG_DBG("-");
    return ret;

}

MUINT32 HWSyncDrvImp::doGeneralSync2_AE(MUINT32 arg1, MUINTPTR arg2, MUINTPTR arg3)
{
    LOG_INF("+");
    arg1;
    MUINT32 ret=1;
    E_N3D_SOURCE adj_sensor;
    MINT32 frmTimeDiffPrd;
    MUINT32 frmTime_1[MAX_SEN] = {0, 0}; // vs1' or vs2'
    MUINT32 sofCnt[2];
    MUINT32 vdiff = 0;
    MUINT32* ppp_expTime = (MUINT32*)arg2;
    MUINT32* ppp_frmTime = (MUINT32*)arg3;
    MUINT32 largerExpTime = (*(ppp_expTime+0) >= *(ppp_expTime+1)) ? *(ppp_expTime+0) : *(ppp_expTime+1);
    #define ADJUST_FRAME_TIME_EXP_MARGIN_ADD_TIME 100 // for adjust frame time when exp time is larger, then add 0.1ms for margin value
    MUINT32 FinalSyncFrmT = (this->m_MaxFrameTime >= largerExpTime) ? (this->m_MaxFrameTime)\
                                : (largerExpTime + ADJUST_FRAME_TIME_EXP_MARGIN_ADD_TIME);

    MINT32 newFrmT[MAX_SEN];
    MINT32 delayFrm;

    //[1] state is syncing
    pthread_mutex_lock(&mStateMutex);
    mState=HW_SYNC_STATE_SYNCING;
    pthread_mutex_unlock(&mStateMutex);


    // Get current sof cnf of main1/2
    sofCnt[0] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_A);
    sofCnt[1] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_B);

    if(1 == mN3dInfo.order) {
        vdiff = this->calPixCnt2US(mN3dInfo.vs_regCnt[1] - mN3dInfo.vs2_vs1Cnt); // VD = pre_B - vs2_vs1Cnt
    } else {
        vdiff = this->calPixCnt2US(mN3dInfo.vs2_vs1Cnt); // VD = vs2_vs1Cnt
    }

    /// 1. @n frame, adjust the leading one sensor where its vsync is leading @ (n+ framelength delay frame)

    if(this->mFrameTimeDelayFrame[0] > MAX_SUPPORT_FRAMELENGTH_DELAY_NUM || this->mFrameTimeDelayFrame[1] > MAX_SUPPORT_FRAMELENGTH_DELAY_NUM || \
       this->mFrameTimeDelayFrame[0] == 0 || this->mFrameTimeDelayFrame[1] == 0) {
        LOG_ERR("current frame-delay(%d_%d) r not support frame delay num, max(%d)", \
            this->mFrameTimeDelayFrame[0], this->mFrameTimeDelayFrame[1],\
            MAX_SUPPORT_FRAMELENGTH_DELAY_NUM);

        ret = 0;
        goto EXIT;
    }
    else if(this->mFrameTimeDelayFrame[0] == this->mFrameTimeDelayFrame[1])
    {
        delayFrm = this->mFrameTimeDelayFrame[0];

        newFrmT[0] = newFrmT[1] = FinalSyncFrmT;

        // vs1 is leading
        if(1 == mN3dInfo.order)
        {
            //(A-B) + (delayfrm-1)(A-B) + (A' - B') - vdiff
            frmTimeDiffPrd = (this->mCurFrameTime[0] - this->mCurFrameTime[1]) + \
                (delayFrm-1)*(this->mCurFrameTime[0] - this->mCurFrameTime[1]) + \
                (newFrmT[0] - newFrmT[1]) - vdiff;

            //shirk b -> enlarge A
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[E_TG_A] = frmTimeDiffPrd * -1 + this->mCurFrameTime[E_TG_A]; // A' = -frmTimeDiffPrd + A
            }
            // enalrge b
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[E_TG_B] = frmTimeDiffPrd + this->mCurFrameTime[E_TG_B]; // B' = frmTimeDiffPrd + B
            }
        }
        else    //VS2 is leading
        {
            //(B-A) + (delayfrm-1)(B-A) + (B' - A') - vdiff
            frmTimeDiffPrd = (this->mCurFrameTime[1] - this->mCurFrameTime[0]) + \
                (delayFrm-1)*(this->mCurFrameTime[1] - this->mCurFrameTime[0]) + \
                (newFrmT[1] - newFrmT[0]) - vdiff;

            //shirk b -> enlarge A
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                frmTime_1[E_TG_A] = frmTimeDiffPrd * -1 + this->mCurFrameTime[E_TG_A]; // A' = -frmTimeDiffPrd + A
            }
            // enalrge b
            else
            {
                adj_sensor = E_TG_B;
                frmTime_1[E_TG_B] = frmTimeDiffPrd + this->mCurFrameTime[E_TG_B]; // B' = frmTimeDiffPrd + B
            }
        }

    }
    // Current case of N3d: main1 -- D(A) = 2 imx258; main1 -- main2 -- D(B) = 1 s5k5e2
    else if(this->mFrameTimeDelayFrame[0] !=  this->mFrameTimeDelayFrame[1])
    {
        delayFrm = (this->mFrameTimeDelayFrame[0] > this->mFrameTimeDelayFrame[1]) ? \
            this->mFrameTimeDelayFrame[0] : this->mFrameTimeDelayFrame[1];

        newFrmT[0] = newFrmT[1] = FinalSyncFrmT;


        // vs1 is leading
        if(1 == mN3dInfo.order)
        {
            //delay A=1,delay B=2
            if(this->mFrameTimeDelayFrame[1] > this->mFrameTimeDelayFrame[0])
            {
                //(A-B) + (delayfrm-1)(A-B) + (A' - B') - vdiff
                frmTimeDiffPrd = (this->mCurFrameTime[0] - this->mCurFrameTime[1]) + \
                    (delayFrm-1)*(newFrmT[0] - this->mCurFrameTime[1]) + \
                    (newFrmT[0] - newFrmT[1]) - vdiff;
            }
            else//dealy A=2, delay B=1
            {
                //(A-B) + (delayfrm-1)(A-B) + (A' - B') - vdiff
                frmTimeDiffPrd = (this->mCurFrameTime[0] - this->mCurFrameTime[1]) + \
                    (delayFrm-1)*(this->mCurFrameTime[0] - newFrmT[1]) + \
                    (newFrmT[0] - newFrmT[1]) - vdiff;
            }

            //shirk b -> enlarge A
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                //TG_A frm delay == 1
                if(this->mFrameTimeDelayFrame[1] > this->mFrameTimeDelayFrame[0])
                {
                    frmTimeDiffPrd = ((frmTimeDiffPrd*-1)>>1)*-1;
                }
                //
                frmTime_1[E_TG_A] = frmTimeDiffPrd * -1 + this->mCurFrameTime[E_TG_A]; // A' = -frmTimeDiffPrd + A
            }
            // enalrge b
            else
            {
                adj_sensor = E_TG_B;
                //TG_B frm delay == 1
                if(this->mFrameTimeDelayFrame[0] > this->mFrameTimeDelayFrame[1])
                {
                    frmTimeDiffPrd = frmTimeDiffPrd>>1;
                }
                //
                frmTime_1[E_TG_B] = frmTimeDiffPrd + this->mCurFrameTime[E_TG_B]; // B' = frmTimeDiffPrd + B
            }
        }
        else // vs2 is leading
        {
            //delay A=1,delay B=2
            if(this->mFrameTimeDelayFrame[1] > this->mFrameTimeDelayFrame[0])
            {
                //(B-A) + (delayfrm-1)(B-A) + (B' - A') - vdiff
                frmTimeDiffPrd = (this->mCurFrameTime[1] - this->mCurFrameTime[0]) + \
                    (delayFrm-1)*(newFrmT[1] - this->mCurFrameTime[0]) + \
                    (newFrmT[1] - newFrmT[0]) - vdiff;
            }
            else//dealy A=2, delay B=1
            {
                //(B-A) + (delayfrm-1)(B-A) + (B' - A') - vdiff
                frmTimeDiffPrd = (this->mCurFrameTime[1] - this->mCurFrameTime[0]) + \
                    (delayFrm-1)*(this->mCurFrameTime[1] - newFrmT[0]) + \
                    (newFrmT[1] - newFrmT[0]) - vdiff;
            }


            //shirk b -> enlarge A
            if(frmTimeDiffPrd <= 0)
            {
                adj_sensor = E_TG_A;
                //TG_A frm delay == 1
                if(this->mFrameTimeDelayFrame[1] > this->mFrameTimeDelayFrame[0])
                {
                    frmTimeDiffPrd = ((frmTimeDiffPrd*-1)>>1)*-1;
                }
                frmTime_1[E_TG_A] = frmTimeDiffPrd * -1 + this->mCurFrameTime[E_TG_A]; // A' = -frmTimeDiffPrd + A
            }
            // enalrge b
            else
            {
                adj_sensor = E_TG_B;
                //TG_B frm delay == 1
                if(this->mFrameTimeDelayFrame[0] > this->mFrameTimeDelayFrame[1])
                {
                    frmTimeDiffPrd = frmTimeDiffPrd>>1;
                }
                frmTime_1[E_TG_B] = frmTimeDiffPrd + this->mCurFrameTime[E_TG_B]; // B' = frmTimeDiffPrd + B
            }
        }
    }
    else{
        LOG_ERR("current frame-delay(%d_%d) r not support frame delay num, max(%d)", \
            this->mFrameTimeDelayFrame[0], this->mFrameTimeDelayFrame[1],\
            MAX_SUPPORT_FRAMELENGTH_DELAY_NUM);

        ret = 0;
        goto EXIT;
    }

    // Check wether AE exp time is larger than adj frame time. If true, we need adjust frame time tobe
    // shutter + (adj frame time - orig frame time)

    if(*(ppp_expTime+adj_sensor) > frmTime_1[adj_sensor]) {
        frmTime_1[adj_sensor] = *(ppp_expTime+adj_sensor) + (frmTime_1[adj_sensor] - this->mCurFrameTime[adj_sensor]);
    }


    LOG_INF("adjSenIdx(0x%x), adjFrmT(%d) us, PreFrmTime(%d/%d) us, CurFrmTime(%d/%d,%d/%d) us, VD(%d) us, order(%d), SOF(%d/%d)",adj_sensor,\
        frmTime_1[adj_sensor], \
        this->calPixCnt2US(mN3dInfo.vs_regCnt[0]),this->calPixCnt2US(mN3dInfo.vs_regCnt[1]), \
        this->mCurFrameTime[0], this->mCurFrameTime[1], \
        newFrmT[0],newFrmT[1], \
        vdiff, mN3dInfo.order, sofCnt[0], sofCnt[1]);

    if(frmTime_1[0]){
        *(ppp_frmTime+0) = frmTime_1[0];
        *(ppp_frmTime+1) = newFrmT[1];
    }
    if(frmTime_1[1]){
        *(ppp_frmTime+1) = frmTime_1[1];
        *(ppp_frmTime+0) = newFrmT[0];
    }

    LOG_INF("dev0/dev1(0x%x/0x%x), expTime(%d/%d), frmTime_1(%d/%d), frmTime_2(%d/%d), delay(%d/%d)",\
        this->mSensorDevArray[0], this->mSensorDevArray[1], \
        *(ppp_expTime+0),*(ppp_expTime+1),*(ppp_frmTime+0),*(ppp_frmTime+1), \
        FinalSyncFrmT, FinalSyncFrmT, \
        this->mFrameTimeDelayFrame[0], this->mFrameTimeDelayFrame[1]);

    this->m2ndData.FrameTime[0] = FinalSyncFrmT;
    this->m2ndData.FrameTime[1] = FinalSyncFrmT;

EXIT:
    LOG_INF("-");

    return ret;

}


MINT32 HWSyncDrvImp::setSyncFrmTimeMain1()
{
    MINT32 ret = 0;

    // wait 1 vsync, @n+1 frame, adjust the frame time of sensor to be the slower one
    ISP_WAIT_IRQ_ST waitIrq;
    ISP_REGISTER_USERKEY_STRUCT regirq;
    waitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
    waitIrq.St_type = SIGNAL_INT;
    waitIrq.Timeout = 1000;
    waitIrq.Status = VS_INT_ST;

    MUINT32 sofCnt[2];
    MUINT32 waitSofCnt = 1;


    // Do adjust fps of two sensor to be the same if current state is not in HW_SYNC_STATE_IDLE (disable hwsync)
    // or not in HW_SYNC_STATE_READY2LEAVE
    if(mState!=HW_SYNC_STATE_IDLE && mState!= HW_SYNC_STATE_READY2LEAVE){


        // Get current sof cnf of main1/2
        sofCnt[0] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_A);
        sofCnt[1] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_B);

        LOG_DBG("+, sof(%d/%d), frmTimeDelay(%d/%d)", sofCnt[0], sofCnt[1], \
            this->mFrameTimeDelayFrame[0], this->mFrameTimeDelayFrame[1]);

        // wait sof:
        // 1. wait 1 sof: D(A) = 1 & D(B) = 1, D(A) = 2 & D(B) = 1, D(A) = 2 & D(B) = 2
        // 2. wait 2 sof: D(A) = 1 & D(B) = 2
        if(this->mFrameTimeDelayFrame[0] != this->mFrameTimeDelayFrame[1])
            waitSofCnt = this->mFrameTimeDelayFrame[1];

        sprintf((char*)regirq.userName,LOG_TAG);
        this->m_pIspDrv[E_TG_A]->registerIrq(&regirq);
        waitIrq.UserKey = regirq.userKey;

        for(unsigned int i = 0; i < waitSofCnt; i++){
            if(this->m_pIspDrv[E_TG_A]->waitIrq(&waitIrq) <= 0)
            {
                if(mState!=HW_SYNC_STATE_IDLE && mState!= HW_SYNC_STATE_READY2LEAVE)
                {
                    LOG_ERR("wait vsync timeout");
                    return -2;
                }
                else
                {
                    LOG_INF("mState(%d)", mState);
                    return ret;
                }
            }
        }

        if( this->mHalSensor->sendCommand(this->mSensorDevArray[0],\
            SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME,\
            (MINTPTR)&this->m2ndData.expTime[0],\
            (MINTPTR)&this->m2ndData.FrameTime[0],0) < 0){
            LOG_ERR("[Main1]:set frame rate to sensor failed, m2ndData.FrameTime: %d", this->m2ndData.FrameTime[0]);

            return -1;
        }

        // Get current sof cnf of main1/2
        sofCnt[0] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_A);
        sofCnt[1] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_B);

        LOG_INF("dev0/dev1(0x%x/0x%x), expTime(%d/%d), frmTime(%d/%d) us, sof(%d/%d)", \
            this->mSensorDevArray[0], this->mSensorDevArray[1], \
            this->m2ndData.expTime[0], this->m2ndData.expTime[1], \
            this->m2ndData.FrameTime[0], this->m2ndData.FrameTime[1], sofCnt[0], sofCnt[1]);
    }
    else {
        LOG_INF("mState is in HW_SYNC_STATE_IDLE or HW_SYNC_STATE_READY2LEAVE state");
    }


    pthread_mutex_lock(&mStateMutex);
    if(mState==HW_SYNC_STATE_SYNCING)
    {
        mState=HW_SYNC_STATE_READY2RUN;
        pthread_mutex_unlock(&mStateMutex);
    }
    else if(mState==HW_SYNC_STATE_IDLE)
    {
        LOG_INF("sem_post HW_SYNC_STATE_IDLE!!");
        ::sem_post(&mSemGeneralSyncDoneMain1);
        pthread_mutex_unlock(&mStateMutex);
    }
    else
    {
        pthread_mutex_unlock(&mStateMutex);
    }

    return ret;
}

MINT32 HWSyncDrvImp::setSyncFrmTimeMain2()
{
    MINT32 ret = 0;

    // wait 1 vsync, @n+1 frame, adjust the frame time of sensor to be the slower one
    ISP_WAIT_IRQ_ST waitIrq;
    ISP_REGISTER_USERKEY_STRUCT regirq;
    waitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
    waitIrq.St_type = SIGNAL_INT;
    waitIrq.Timeout = 1000;
    waitIrq.Status = VS_INT_ST;

    MUINT32 sofCnt[2];
    MUINT32 waitSofCnt = 1;

    // Do adjust fps of two sensor to be the same if current state is not in HW_SYNC_STATE_IDLE (disable hwsync)
    // or not in HW_SYNC_STATE_READY2LEAVE
    if(mState!=HW_SYNC_STATE_IDLE && mState!= HW_SYNC_STATE_READY2LEAVE){


        // Get current sof cnf of main1/2
        sofCnt[0] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_A);
        sofCnt[1] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_B);

        LOG_DBG("+, sof(%d/%d), frmTimeDelay(%d/%d)", sofCnt[0], sofCnt[1], \
            this->mFrameTimeDelayFrame[0], this->mFrameTimeDelayFrame[1]);

        // wait sof:
        // 1. wait 1 sof: D(A) = 1 & D(B) = 1, D(A) = 2 & D(B) = 1, D(A) = 2 & D(B) = 2
        // 2. wait 2 sof: D(A) = 2 & D(B) = 1
        if(this->mFrameTimeDelayFrame[0] != this->mFrameTimeDelayFrame[1])
            waitSofCnt = this->mFrameTimeDelayFrame[0];

        sprintf((char*)regirq.userName,LOG_TAG);
        this->m_pIspDrv[E_TG_B]->registerIrq(&regirq);
        waitIrq.UserKey = regirq.userKey;

        for(unsigned int i = 0; i < waitSofCnt; i++){
            if(this->m_pIspDrv[E_TG_B]->waitIrq(&waitIrq) <= 0)
            {
                if(mState!=HW_SYNC_STATE_IDLE && mState!= HW_SYNC_STATE_READY2LEAVE)
                {
                    LOG_ERR("wait vsync timeout");
                    return -2;
                }
                else
                {
                    LOG_INF("mState(%d)", mState);
                    return ret;
                }
            }
        }

        if( this->mHalSensor->sendCommand(this->mSensorDevArray[1],\
            SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME,\
            (MINTPTR)&this->m2ndData.expTime[1],\
            (MINTPTR)&this->m2ndData.FrameTime[1],0) < 0){
            LOG_ERR("[Main2]:set frame rate to sensor failed, m2ndData.FrameTime: %d", this->m2ndData.FrameTime[1]);

            return -1;
        }

        // Get current sof cnf of main1/2
        sofCnt[0] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_A);
        sofCnt[1] = this->DeviceInfo(E_GET_SOF_CNT,E_TG_B);

        LOG_INF("dev0/dev1(0x%x/0x%x), expTime(%d/%d), frmTime(%d/%d) us, sof(%d/%d)", \
            this->mSensorDevArray[0], this->mSensorDevArray[1], \
            this->m2ndData.expTime[0], this->m2ndData.expTime[1], \
            this->m2ndData.FrameTime[0], this->m2ndData.FrameTime[1], sofCnt[0], sofCnt[1]);

    }
    else {
        LOG_INF("mState is in HW_SYNC_STATE_IDLE or HW_SYNC_STATE_READY2LEAVE state");
    }


    pthread_mutex_lock(&mStateMutex);
    if(mState==HW_SYNC_STATE_SYNCING)
    {
        mState=HW_SYNC_STATE_READY2RUN;
        pthread_mutex_unlock(&mStateMutex);
    }
    else if(mState==HW_SYNC_STATE_IDLE)
    {
        LOG_INF("sem_post HW_SYNC_STATE_IDLE!!");
        ::sem_post(&mSemGeneralSyncDoneMain2);
        pthread_mutex_unlock(&mStateMutex);
    }
    else
    {
        pthread_mutex_unlock(&mStateMutex);
    }

    return ret;
}

MUINT32 HWSyncDrvImp::DeviceInfo(E_CMD cmd,MUINT32 tgIdx)
{
    MUINT32 _cnt = 0;

    switch(cmd){
        case E_GET_SOF_CNT:
            switch(tgIdx){
                case E_TG_A:
                case E_TG_B:
                    if(!this->m_pIspDrv[tgIdx]->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&_cnt)) {
                        LOG_ERR("_GET_VSYNC_CNT error!!!!!!!");
                    }
                    break;
                default:
                    LOG_ERR("unsupported TG idx:0x%x",tgIdx);
                    return 0;
                    break;
            }
            break;
        default:
            LOG_ERR("unsupported cmd:0x%x",cmd);
            return 0;
            break;
    }

    return _cnt;
}

