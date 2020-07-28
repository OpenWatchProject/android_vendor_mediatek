#define LOG_TAG "HWsync_ini"
#define LOG_TAG2 "HWsync_ini2"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <cutils/atomic.h>
#include <semaphore.h>

#include <system/thread_defs.h>
#include <utils/ThreadDefs.h>
#include <utils/threads.h>

#include <cutils/properties.h>  // For property_get().
//
#include "hwsync_drv_imp.h"
#include <cutils/log.h>
#include <mtkcam/utils/std/ULog.h>




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
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

#endif

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_SENSOR);

//max fps is from n3d vsync diff if enabled
#define AUTO_MAX_FPS    (0)

HWSyncInit::HWSyncInit(void* pUserObj)
{
    m_this = (HWSyncDrvImp*)pUserObj;
    m_bRst = MTRUE;
    m_MaxFrameCnt = 0xFFFFFFFF;
    this->m_MaxSource = E_TG_A;
    this->mThread1_TG = E_TG_A;
    this->mThread2_TG = E_TG_A;
    this->m_Thread_1 = (pthread_t)NULL;
    this->m_Thread_2 = (pthread_t)NULL;
    this->m_order = 0;
    this->m_Sem_1.count = 0;
    this->m_Sem_2.count = 0;
    this->m_DiffThres = 0;
}

HWSyncInit::~HWSyncInit()
{}


MBOOL HWSyncInit::Trig(void)
{

    pthread_attr_t  attr_1, attr_2;
    MUINT32 r;
    MUINT32 reference_tg = 0;

    LOG_INF("+");
    // temp using 5ms
    this->m_DiffThres = 5000;
    ::sem_init(&this->m_Sem_1, 0, 0);
    ::sem_init(&this->m_Sem_2, 0, 0);

    if ((r = pthread_attr_init(&attr_1)) != 0) {
        LOG_ERR("thread ctrl [%s] fail: %d", "set thread_1 attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_init(&attr_2)) != 0) {
        LOG_ERR("thread ctrl [%s] fail: %d", "set thread_2 attr", r);
        return MFALSE;
    }
    if ((r = pthread_attr_setdetachstate(&attr_1, PTHREAD_CREATE_DETACHED)) != 0) {
        LOG_ERR("thread ctrl [%s] fail: %d", "thread_1 setdetach", r);
        return MFALSE;
    }
    if ((r = pthread_attr_setdetachstate(&attr_2, PTHREAD_CREATE_DETACHED)) != 0) {
        LOG_ERR("thread ctrl [%s] fail: %d", "thread_2 setdetach", r);
        return MFALSE;
    }

    //in order to make sure basis sensor have n3d frame count output under the case of parallel lunch
    if(this->mThread2_TG == E_TG_A)
        reference_tg = E_TG_B;
    else
        reference_tg = E_TG_A;

    if(this->DeviceInfo(E_GET_VSYNC_CNT,reference_tg) < 2){
        MUINT32 _cnt= 0;
        ISP_WAIT_IRQ_ST waitIrq;
        ISP_REGISTER_USERKEY_STRUCT regirq;

        LOG_INF("Start parallel lunch,need to wait TG_%d",reference_tg);

        waitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
        waitIrq.St_type = SIGNAL_INT;
        waitIrq.Timeout = 1000;
        waitIrq.Status=VS_INT_ST;
        sprintf((char*)regirq.userName,"HWSYNC_trig");//register another to avoid vs1 signal have racing condition under thread1 & thread2
        this->m_this->m_pIspDrv[reference_tg]->registerIrq(&regirq);
        waitIrq.UserKey = regirq.userKey;

        do{
            if(this->DeviceInfo(E_GET_VSYNC_CNT,reference_tg) < 2){
                while(1){
                    if (this->m_this->m_pIspDrv[reference_tg]->waitIrq(&waitIrq) <= 0){
                        LOG_ERR("[%d]:wait vsync timeout",reference_tg);
                        if(_cnt++ > 4){
                            LOG_ERR("[%d]:wait no vsync over %d times, fail",reference_tg, _cnt);
                            return MFALSE;
                        }
                    }
                    else
                        break;
                }
            }
            else{
                //
                LOG_INF("parallel lunch -");
                break;
            }
        }while(1);
    }

    if ((r = pthread_create(&this->m_Thread_2, &attr_2, InitThread_2, this)) != 0) {
        LOG_ERR("thread ctrl [%s] fail: %d", "thread_2 create", r);
        return MFALSE;
    }




    ::sem_wait(&this->m_Sem_2);
    if( ( r = pthread_attr_destroy(&attr_2)) != 0 ){
        LOG_ERR("thread ctrl [%s] fail: %d", "thread_2 destroy", r);
        return MFALSE;
    }
    LOG_INF("thread_2 sync end");

    if ((r = pthread_create(&this->m_Thread_1, &attr_1, InitThread_1, this)) != 0) {
        LOG_ERR("thread ctrl [%s] fail: %d", "thread_1 create", r);
        return MFALSE;
    }

    ::sem_wait(&this->m_Sem_1);
    if( ( r = pthread_attr_destroy(&attr_1)) != 0 ){
        LOG_ERR("thread ctrl [%s] fail: %d", "thread_1 destroy", r);
        return MFALSE;
    }
    LOG_INF("thread_1 sync end");

    if(this->m_bRst == 2){
        return this->InitSync();
    }
    else if(m_bRst == MFALSE){
        LOG_ERR("N3D initialize fail");
    }
    return m_bRst;
}

MBOOL HWSyncInit::InitSync(void)
{
    MBOOL rst = MTRUE;
    MUINT32 nextDiffCnt=0;
    MUINT32 D = 0,tmp;
    MUINT32 order;
    ISP_WAIT_IRQ_ST waitIrq;
    ISP_REGISTER_USERKEY_STRUCT regirq;
    SensorN3dDiffCnt N3DregInfo;
    MUINT32 _cnt = 0,_cnt2=0;
    IspDrvImp* ptr;

    waitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
    waitIrq.Timeout = 1000;
    waitIrq.St_type = SIGNAL_INT;
    waitIrq.Status = VS_INT_ST;
    sprintf((char*)regirq.userName,LOG_TAG);
    this->m_this->m_pIspDrv[this->m_MaxSource]->registerIrq(&regirq);
    waitIrq.UserKey = regirq.userKey;


    //(1) wait equal fps output rdy
    _cnt = 1 * 1000000 * 10 / this->m_this->m_MaxFrameTime; //convert into fps



    if (this->m_this->m_pIspDrv[this->m_MaxSource]->waitIrq(&waitIrq) <= 0){
        LOG_ERR("[%d]:wait vsync timeout",this->m_MaxSource);
        return MFALSE;
    }

    //(2)judge sync (always adjust the leading one,it doesn't matter if input is TG_A or B)
    if(this->m_this->mHalSensor->sendCommand(this->m_this->mSensorDevArray[E_TG_B],\
        SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT,(MINTPTR)&N3DregInfo,0,0) < 0){
        LOG_ERR("[%d]:query N3D diff fail",this->m_this->mSensorDevArray[E_TG_B]);
        return MFALSE;
    }
    order = (N3DregInfo.u4DiffCnt & 0x80000000)>>31;
    switch(order)// vsysnc order
    {
        case 1: //vs1 leads vs2
            D = (N3DregInfo.u4Vsync2Cnt-N3DregInfo.u4DebugPort);
            _cnt2 = E_TG_A;
            break;
        case 0: //vs2 leads vs1
            D = N3DregInfo.u4DebugPort;
            _cnt2 = E_TG_B;
            break;
        default:
            break;
    }



    //(3)sync
    LOG_INF("N3D info : [%d,%d,%d us]",this->m_this->calPixCnt2US(N3DregInfo.u4Vsync1Cnt),\
        this->m_this->calPixCnt2US(N3DregInfo.u4Vsync2Cnt),\
        this->m_this->calPixCnt2US(N3DregInfo.u4DebugPort));
    tmp = (N3DregInfo.u4Vsync1Cnt > N3DregInfo.u4Vsync2Cnt)?(N3DregInfo.u4Vsync1Cnt - N3DregInfo.u4Vsync2Cnt):\
        (N3DregInfo.u4Vsync2Cnt - N3DregInfo.u4Vsync1Cnt);

    tmp = this->m_this->calPixCnt2US(tmp);
    if(tmp > this->m_DiffThres){
        LOG_ERR("set fps equal fail, over threshold:%d_%d us",tmp,this->m_DiffThres);
        return MFALSE;
    }
    D = this->m_this->calPixCnt2US(D);
    D = D + this->m_this->mFrameTimeDelayFrame[_cnt2] * tmp;
    LOG_INF("next %d frm diff(%d us)",this->m_this->mFrameTimeDelayFrame[_cnt2],D);

    //adj time
    if (order == 1)
        nextDiffCnt += this->m_this->calPixCnt2US(N3DregInfo.u4Vsync2Cnt);
    else
        nextDiffCnt += this->m_this->calPixCnt2US(N3DregInfo.u4Vsync1Cnt);

    nextDiffCnt += D;
    LOG_INF("Adj TG_%d FrmT to %d us",_cnt2,nextDiffCnt);
    if (this->m_this->mHalSensor->sendCommand(this->m_this->mSensorDevArray[_cnt2],\
        SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME,\
        (MINTPTR)&this->m_Shutter[_cnt2],\
        (MINTPTR)&nextDiffCnt,0) < 0){
        LOG_ERR("[%d]:set frame rate to sensor failed",_cnt2);
       return MFALSE;
    }

    //wait update
    if (this->m_this->m_pIspDrv[_cnt2]->waitIrq(&waitIrq) <= 0){
        LOG_ERR("[%d]:wait vsync timeout",_cnt2);
        return MFALSE;
    }

    //reset to max frame time
    MUINT32 frmT = (this->m_Shutter[_cnt2] > this->m_this->m_MaxFrameTime) ? \
                   (this->m_Shutter[_cnt2]) : \
                   (this->m_this->m_MaxFrameTime);
    LOG_INF("frmT:%d shutter:%d MaxFrmTime:%d",frmT,this->m_Shutter[_cnt2],this->m_this->m_MaxFrameTime);
    if( this->m_this->mHalSensor->sendCommand(this->m_this->mSensorDevArray[_cnt2],\
        SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME,\
        (MINTPTR)&this->m_Shutter[_cnt2],\
        (MINTPTR)&frmT,0) < 0){
        LOG_ERR("[%d]:set frame rate to sensor failed",_cnt2);
        return MFALSE;
    }

    return rst;
}



MUINT32 HWSyncInit::DeviceInfo(E_CMD cmd,MUINT32 tgIdx)
{
    MUINT32 _cnt = 0;

    switch(cmd){
        case E_RESET_VSYNC_CNT:
            //tg_x is don't care
            this->m_this->m_pIspDrv[E_TG_A]->setDeviceInfo(_RESET_VSYNC_CNT,NULL);
            break;
        case E_GET_VSYNC_CNT:
            switch(tgIdx){
                case E_TG_A:
                case E_TG_B:
                    if(!this->m_this->m_pIspDrv[tgIdx]->getDeviceInfo(_GET_VSYNC_CNT,(MUINT8 *)&_cnt)) {
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

MVOID* HWSyncInit::InitThread_1(MVOID* arg)
{
    HWSyncInit *_this = reinterpret_cast<HWSyncInit*>(arg);
    int const policy    = SCHED_OTHER;
    int const priority  = ANDROID_PRIORITY_NORMAL-8;
    struct sched_param sched_p;
    ISP_WAIT_IRQ_ST waitIrq;
    ISP_REGISTER_USERKEY_STRUCT regirq;
    SensorN3dDiffCnt N3DregInfo;
    MUINT32 _cnt = 0,_cnt2 = 0;
    MUINT32 _cnt3,_cnt4;
    MUINT32 working_tg = _this->mThread1_TG;
    MUINT32 reference_tg;

    if(working_tg == E_TG_A)
        reference_tg = E_TG_B;
    else
        reference_tg = E_TG_A;

    LOG_INF("start + [%d]",working_tg);
    ::sched_getparam(0, &sched_p);
    //
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);


    //
    waitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
    waitIrq.St_type = SIGNAL_INT;
    waitIrq.Timeout = 1000;
    waitIrq.Status=VS_INT_ST;
    sprintf((char*)regirq.userName,LOG_TAG);
    _this->m_this->m_pIspDrv[working_tg]->registerIrq(&regirq);
    waitIrq.UserKey = regirq.userKey;

    if(_this->m_MaxSource == reference_tg){
        if (_this->m_this->m_pIspDrv[working_tg]->waitIrq(&waitIrq) <= 0){
            LOG_ERR("[%d]:wait vsync timeout",working_tg);
            if(_cnt++ > 2){
                LOG_ERR("[%d]:wait no vsync over %d times, fail",working_tg, _cnt);

                //
                _this->m_ThreadMtx.lock();
                _this->m_bRst = MFALSE;
                _this->m_ThreadMtx.unlock();
                //
                ::sem_post(&_this->m_Sem_1);
                return NULL;
            }
        }

        LOG_INF("expT,frmT = [%d_%d]",_this->m_Shutter[working_tg],_this->m_this->m_MaxFrameTime);
        MUINT32 frmT = (_this->m_Shutter[working_tg] > _this->m_this->m_MaxFrameTime) ? \
                        (_this->m_Shutter[working_tg]) : \
                        (_this->m_this->m_MaxFrameTime);
        if(_this->m_this->mHalSensor->sendCommand(_this->m_this->mSensorDevArray[working_tg],\
            SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME,\
            (MINTPTR)&_this->m_Shutter[working_tg],\
            (MINTPTR)&frmT,0) < 0){
            LOG_ERR("[%d]:set frame time to sensor failed",working_tg);
            //
            _this->m_ThreadMtx.lock();
            _this->m_bRst = MFALSE;
            _this->m_ThreadMtx.unlock();
            //
            ::sem_post(&_this->m_Sem_1);
            return NULL;
        }

    }

    LOG_INF("- set end(%d)",_this->DeviceInfo(E_GET_VSYNC_CNT,working_tg));

    //
    ::sem_post(&_this->m_Sem_1);
    return NULL;
}

MVOID* HWSyncInit::InitThread_2(MVOID* arg)
{
    HWSyncInit *_this = reinterpret_cast<HWSyncInit*>(arg);
    int const policy    = SCHED_OTHER;
    int const priority  = (ANDROID_PRIORITY_NORMAL-7);//priority lower than thread_1
    struct sched_param sched_p;
    ISP_WAIT_IRQ_ST waitIrq;
    ISP_REGISTER_USERKEY_STRUCT regirq;
    SensorN3dDiffCnt N3DregInfo;
    MUINT32 _cnt = 0,_cnt2 = 0,_cnt3 = 0;
    MUINT32 n;
    MUINT32 trig_point = 2;
    MBOOL   bValidDelta = MFALSE;
    MUINT32 working_tg = _this->mThread2_TG;
    MUINT32 reference_tg;

    if(working_tg == E_TG_A)
        reference_tg = E_TG_B;
    else
        reference_tg = E_TG_A;

    LOG_INF("start + [%d]",working_tg);
    ::sched_getparam(0, &sched_p);
    //
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    //
    waitIrq.Clear = ISP_IRQ_CLEAR_WAIT;
    waitIrq.St_type = SIGNAL_INT;
    waitIrq.Timeout = 1000;
    waitIrq.Status=VS_INT_ST;
    sprintf((char*)regirq.userName,LOG_TAG2);//register another to avoid vs1 signal have racing condition under thread1 & thread2
    _this->m_this->m_pIspDrv[working_tg]->registerIrq(&regirq);
    waitIrq.UserKey = regirq.userKey;

    //get n3d info first , wait until thread2's vysnc cnt>= 2.otherwise, get no correct vsync1cnt && vsycn2cnt
    do{
        if(_this->m_this->mHalSensor->sendCommand(_this->m_this->mSensorDevArray[working_tg],\
            SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT,(MINTPTR)&N3DregInfo,0,0) < 0){
            LOG_ERR("[%d]:query N3D diff fail",_this->m_this->mSensorDevArray[working_tg]);
            //
            _this->m_ThreadMtx.lock();
            _this->m_bRst = MFALSE;
            _this->m_ThreadMtx.unlock();
            //
            ::sem_post(&_this->m_Sem_2);
            return NULL;
        }

        _cnt3 = _this->DeviceInfo(E_GET_VSYNC_CNT,working_tg);

        LOG_INF("[%d]:N3D info : [%d,%d,%d]",_cnt3,_this->m_this->calPixCnt2US(N3DregInfo.u4Vsync1Cnt),\
            _this->m_this->calPixCnt2US(N3DregInfo.u4Vsync2Cnt), _this->m_this->calPixCnt2US(N3DregInfo.u4DebugPort));

        if(_cnt3 < 2){
            _cnt = 0;
            while(1){//using thread2's signal to monitor thread1's vsync counter
                if (_this->m_this->m_pIspDrv[working_tg]->waitIrq(&waitIrq) <= 0){
                    LOG_ERR("[%d]:wait vsync timeout",working_tg);
                    if(_cnt++ > 4){
                        LOG_ERR("[%d]:wait no vsync over %d times, fail",working_tg, _cnt);

                        //
                        _this->m_ThreadMtx.lock();
                        _this->m_bRst = MFALSE;
                        _this->m_ThreadMtx.unlock();
                        //
                        ::sem_post(&_this->m_Sem_2);
                        return NULL;
                    }
                }
                else
                    break;
            }
        }
        else{
            //
            break;
        }


    }while(1);

    //set max fps for folllowing N3D contorl
    _this->m_ThreadMtx.lock();
#if AUTO_MAX_FPS
    if(_this->m_this->m_MaxFrameTime == 0xFFFFFFFF){
        if(N3DregInfo.u4Vsync1Cnt > N3DregInfo.u4Vsync2Cnt){
            _this->m_this->m_MaxFrameTime = _this->m_this->calPixCnt2US(N3DregInfo.u4Vsync1Cnt);
            _this->m_MaxFrameCnt = N3DregInfo.u4Vsync1Cnt;
            _this->m_MaxSource = E_TG_A;
        }
        else{
            _this->m_this->m_MaxFrameTime = _this->m_this->calPixCnt2US(N3DregInfo.u4Vsync2Cnt);
            _this->m_MaxFrameCnt = N3DregInfo.u4Vsync2Cnt;
            _this->m_MaxSource = E_TG_B;
        }
    }
#else
    MUINT32 framerate;
    _this->m_this->mHalSensor->sendCommand(_this->m_this->mSensorDevArray[E_TG_A],\
                SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,(MINTPTR)&_this->m_this->mSensorScenArray[E_TG_A],\
                (MINTPTR)&framerate,0);

    _this->m_this->mHalSensor->sendCommand(_this->m_this->mSensorDevArray[E_TG_B],\
                SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,(MINTPTR)&_this->m_this->mSensorScenArray[E_TG_B],\
                (MINTPTR)&_this->m_this->m_MaxFrameTime,0);


    _this->m_MaxSource = (framerate > _this->m_this->m_MaxFrameTime) ? (E_TG_A): (E_TG_B);

    _this->m_this->m_MaxFrameTime = (framerate > _this->m_this->m_MaxFrameTime)?(_this->m_this->m_MaxFrameTime):\
        (framerate);

    _this->m_this->m_MaxFrameTime = 1000000 / (_this->m_this->m_MaxFrameTime/10);

#endif
    _this->m_ThreadMtx.unlock();
    LOG_INF("Max speed of N3D : [%d us,%d clk,source:%d],[%d,%d]",_this->m_this->m_MaxFrameTime,\
        _this->m_MaxFrameCnt,\
        _this->m_MaxSource,\
        _cnt3,\
        _this->DeviceInfo(E_GET_VSYNC_CNT,reference_tg));

    if(_cnt3 > trig_point){
        MUINT32 nextDiffCnt=0x0;
        MUINT32 D = 0;
        MUINT32 order;
        LOG_INF("[%d]:HWSyncInit timing already shift:%d",working_tg,_cnt2);

        //
        _this->m_ThreadMtx.lock();
        _this->m_bRst = 2;
        _this->m_ThreadMtx.unlock();
        //
        ::sem_post(&_this->m_Sem_2);
        return NULL;
    }
    else{
        MUINT32 A,B,D;
        if(working_tg == E_TG_B){
            A = N3DregInfo.u4Vsync1Cnt;
            B = N3DregInfo.u4Vsync2Cnt;
            D = N3DregInfo.u4Vsync2Cnt - N3DregInfo.u4DebugPort;
        }
        else{
            A = N3DregInfo.u4Vsync2Cnt;
            B = N3DregInfo.u4Vsync1Cnt;
            D = N3DregInfo.u4DebugPort;
        }
        n = 1;

        do{
            //(A-D) + m*A + n*C - B
            _cnt = A - D;
            _cnt += (A + (n*A));// (A-D) + A + n*A
            _cnt -= B;// (A-D) + A + n*A - B

            if(_cnt >= B)
                break;
            else
                n++;
        }while(1);

        LOG_INF("n = [%d]",n);
        _cnt = _this->m_this->calPixCnt2US(_cnt);
        LOG_INF("expT,frmT = [%d_%d]",_this->m_Shutter[working_tg],_cnt);
        if(_this->m_this->mHalSensor->sendCommand(_this->m_this->mSensorDevArray[working_tg],\
            SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME,\
            (MINTPTR)&_this->m_Shutter[working_tg],\
            (MINTPTR)&_cnt,0) < 0){
            LOG_ERR("[%d]:set frame time to sensor failed",working_tg);
            //
            _this->m_ThreadMtx.lock();
            _this->m_bRst = MFALSE;
            _this->m_ThreadMtx.unlock();
            //
            ::sem_post(&_this->m_Sem_2);
            return NULL;
        }

        //
        if (_this->m_this->m_pIspDrv[working_tg]->waitIrq(&waitIrq) <= 0){
            LOG_ERR("[%d]:wait vsync timeout",working_tg);

            //
            _this->m_ThreadMtx.lock();
            _this->m_bRst = MFALSE;
            _this->m_ThreadMtx.unlock();
            //
            ::sem_post(&_this->m_Sem_2);
            return NULL;
        }

        //
        MUINT32 frmT = (_this->m_Shutter[working_tg] > _this->m_this->m_MaxFrameTime) ? \
                        (_this->m_Shutter[working_tg]) : \
                        (_this->m_this->m_MaxFrameTime);
        if(_this->m_this->mHalSensor->sendCommand(_this->m_this->mSensorDevArray[working_tg],\
            SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME,\
            (MINTPTR)&_this->m_Shutter[working_tg],\
            (MINTPTR)&frmT,0) < 0){
            LOG_ERR("[%d]:set frame time to sensor failed",working_tg);
            //
            _this->m_ThreadMtx.lock();
            _this->m_bRst = MFALSE;
            _this->m_ThreadMtx.unlock();
            //
            ::sem_post(&_this->m_Sem_2);
            return NULL;
        }



    }
    LOG_INF("- set end(%d)",_this->DeviceInfo(E_GET_VSYNC_CNT,working_tg));
    //
    ::sem_post(&_this->m_Sem_2);

    return NULL;
}

