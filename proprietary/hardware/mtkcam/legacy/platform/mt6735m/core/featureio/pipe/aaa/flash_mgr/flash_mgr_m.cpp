#ifdef WIN32
#include "stdafx.h"
#include "FlashSim.h"
#include "sim_MTKAECommon.h"
#include "sim_MTKAE.h"
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
#include "flash_mgr.h"
#else
#endif

#define LOG_TAG "flash_mgr_m.cpp"

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <dbg_aaa_param.h>
#include <dbg_isp_param.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_param.h>
#include <camera_custom_AEPlinetable.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_mgr.h>
#include <mtkcam/algorithm/lib3a/ae_algo_if.h>
#include <mtkcam/hal/sensor_hal.h>
//#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <aaa_sensor_mgr.h>
#include <flash_awb_tuning_custom.h>
#include <flash_param.h>
#include <flash_error_code.h>
#include <mtkcam/algorithm/lib3a/FlashAlg.h>
#include "flash_mgr.h"
#include "flash_tuning_custom.h"
#include "strobe_drv.h"
#include <time.h>
#include <kd_camera_feature.h>
#include "dbg_flash_param.h"
#include <isp_mgr.h>
#include <ispdrv_mgr.h>
#include <isp_tuning_mgr.h>
#include <nvram_drv.h>
//#include <nvram_drv_mgr.h>
#include "flash_util.h"
#include <vector>
#include <mtkcam/acdk/cct_feature.h>
#include "nvbuf_util.h"
#include "flash_mgr_m.h"
#include <algorithm>
#include <vector>
#include <deque>
#include <flash_pline_tool.h>
#include <aaa_common_custom.h>
#include <awb_tuning_custom.h>
//name space
using namespace NS3A;
using namespace NSIspTuning;

#define DEF_ALWAYS_VERBOSE 1
#define PROP_BIN_EN_STR        "vendor.flash_bin_en"
#define PROP_PF_BMP_EN_STR    "vendor.flash_pf_bmp_en"
#define PROP_MF_BMP_EN_STR    "vendor.flash_mf_bmp_en"
#define PROP_MF_ON_STR         "vendor.flash_mf_on"
#define PROP_MF_DUTY_STR     "vendor.flash_mf_duty"
#define PROP_MF_DUTYLT_STR     "vendor.flash_mf_dutylt"
#define PROP_MF_EXP_STR     "vendor.flash_mf_exp"
#define PROP_MF_ISO_STR     "vendor.flash_mf_iso"
#define PROP_MF_AFE_STR     "vendor.flash_mf_afe"
#define PROP_MF_ISP_STR     "vendor.flash_mf_isp"


#define PROP_PF_EXP_FOLLOW_PLINE     "vendor.flash_pf_by_pline"
#define PROP_PF_MAX_AFE    "vendor.flash_pf_max_afe"
#define PROP_VERBOSE_EN    "vendor.flash_verbose_en"
#define ALG_TAG_SIZE 6000//5824  //1456*4    //578*4 2312

#define logI(fmt, arg...) ALOGD(LOG_TAG " " fmt, ##arg) //ALOGD(fmt, ##arg)
#define logV(fmt, arg...) {if(g_isVerboseEn) logI(fmt, ##arg);}
#define logE(fmt, arg...) ALOGE("MError: func=%s line=%d: " fmt, __FUNCTION__, __LINE__, ##arg)
#define LogW(fmt, arg...) ALOGE("FlashWarning: func=%s line=%d: " fmt, __FUNCTION__, __LINE__, ##arg)
//==============================

//==============================
// variable
static int g_uiSpModeLock=0;
static int g_uiSpMode=e_SpModeNormal;
static int g_spMode=e_SpModeNormal;
static int g_isVerboseEn=0;
static int g_isManualFlashEn=0;
static int g_manualDuty=0;
static int g_manualDutyLt=0;


//==============================
// function
//==============================
void dispStrobeNvram(NVRAM_CAMERA_STROBE_STRUCT* nv);

int getStrobeType(int sensorDev)
{
    if (sensorDev == DUAL_CAMERA_MAIN_SENSOR)
        return 1;
    else if (sensorDev == DUAL_CAMERA_SUB_SENSOR)
        return 2;
    else if (sensorDev == DUAL_CAMERA_MAIN_2_SENSOR ||
            sensorDev == DUAL_CAMERA_MAIN_SECOND_SENSOR)
        return 1;
    else if (sensorDev == DUAL_CAMERA_SUB_2_SENSOR)
        return 2;
    else
        return 0;
}

int getStrobeCtNum(int typeId)
{
    if (cust_isDualFlashSupport(typeId) == 1)
        return 2;
    return 1;
}

FlashMgrM::FlashMgrM(int sensorDev)
{
    logV("FlashMgrM sensorDev=%d",sensorDev);
    mIsInited=0;
    g_flashQuickCalibrationResult=0;

    mEngName = new char[200];
    mEngName[0]=0;
    g_previewMode=FlashMgrM::e_NonePreview;;

    mCapRep.errCnt=0;
    m_sensorDev=sensorDev;
    m_iteration=0;
    mIsNeedFiredFlashChecked = 0;
    mFlash1Mode = LIB3A_FLASH_MODE_FORCE_OFF;
    m_flashOnPrecapture = 0;
    m_digRatio = 1;
    m_pfFrameCount=0;
    m_evComp=0;
    mStrobeState=e_StrobeOff;
    mTorchLevel=-1;
    m_isAfState=0;

    /* acquire strobe instance */
    int typeId, ctNum;
    typeId = getStrobeType(sensorDev);
    ctNum = getStrobeCtNum(typeId);
    g_pStrobe = StrobeDrv::getInstance(typeId);
    if (ctNum == 2) {
        g_pStrobe2 = StrobeDrv::getInstance(m_sensorDev, 2);
    } else {
    g_pStrobe2 = NULL;
    }

    m_bRunPreFlash = 0;

    g_onOffCallBackFunc=0;
    mDebugSceneCnt=0;

    int partId;
    g_pStrobe->getPartId(&partId);
    logV("getPartId %d",partId);
    if(partId>2)
        logV("error: if(partId>2)");

    cust_setFlashPartId(sensorDev, partId);

    //@@ 2led part id
    /*
       partId = g_pStrobe->getPartId(&partId);
       if(partId>2)
       logI("error: if(partId>2)");
     */



#if USE_OPEN_SOURCE_FLASH_AE

    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
        mpFlashAlg = FlashAlg::getInstanceLite(eFlashMainSingle);
    else if(sensorDev==DUAL_CAMERA_SUB_SENSOR)
        mpFlashAlg = FlashAlg::getInstanceLite(eFlashSubSingle);
    else
        mpFlashAlg = FlashAlg::getInstanceLite(eFlashMain2Single);
#else

    if(cust_isDualFlashSupport(m_sensorDev)==1)
    {
        if(m_sensorDev==ESensorDev_Main)
            mpFlashAlg = FlashAlg::getInstance(eFlashMainDual);
        else
            mpFlashAlg = FlashAlg::getInstance(eFlashSubDual);
    }
    else
    {
        if(m_sensorDev==ESensorDev_Main)
            mpFlashAlg = FlashAlg::getInstance(eFlashMainSingle);
        else if(m_sensorDev==ESensorDev_Sub)
            mpFlashAlg = FlashAlg::getInstance(eFlashSubSingle);
        else
            mpFlashAlg = FlashAlg::getInstance(eFlashMain2Single);
    }
#endif

    memset(&mCapRep, 0, sizeof(FlashCapRep));
    memset(&g_expPara, 0, sizeof(FlashAlgExpPara));
    memset(&g_pfExpPara, 0, sizeof(FlasExpParaEx));
    mCoolingStartTime = 0;
    mCoolingWaitTime = 0;
    mFlashState = 0;
    mFlashPreSt = 0;
    mPreRunFrm = 0;
    mFlashHwState = 0;
    mRunCycleFrame = 0;


    if(isFlashAWBv2Enabled())
    {
        MY_LOG("[Flash Awb] Prepare to do flash awb init");

        FLASH_AWB_INIT_T FlashAwbInitParam;
        NVRAM_CAMERA_3A_STRUCT* buf;
        int err;
        err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_sensorDev, (void*&)buf);
        if(err!=0)
            logE("nvbuf get CAMERA_NVRAM_DATA_3A");

        logV("flash_awb_para for:%d bk: %d", (int)buf->rFlashAWBNVRAM.rTuningParam.ForeGroundPercentage, (int)buf->rFlashAWBNVRAM.rTuningParam.BackGroundPercentage);

        logV("flash_awb_para th1,2,3,4:%d %d %d %d",
                buf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th1,
                buf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th2,
                buf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th3,
                buf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th4);

        FlashAwbInitParam.flash_awb_tuning_param = buf->rFlashAWBNVRAM.rTuningParam;
        mpFlashAlg->Flash_Awb_Init(FlashAwbInitParam);
    }

}
void FlashMgrM::setFlashOn(int duty, int timeOut, int dutyLT, int timeOut2, int reTrig)
{
    if(g_onOffCallBackFunc!=0)
        g_onOffCallBackFunc(1);

    logV("setFlashOn %d %d %d %d %d ln=%d",duty, timeOut, dutyLT, timeOut2, reTrig, __LINE__);

    int isLow;
    g_pStrobe->isLowPower(&isLow);
    if(isLow==1)
    {
        logV("isLow=1 ln=%d",__LINE__);
        return;
    }

    int bOn;
    int dutyCur;
    g_pStrobe->isOn(&bOn);
    g_pStrobe->getDuty(&dutyCur);
    if(duty==-1)
        g_pStrobe->setOnOff(0);
    else
    {
        //else if(bOn==0 || dutyCur!=duty ||reTrig==1)
        if(dutyCur!=duty)
        {
            g_pStrobe->setDuty(duty);
            g_pStrobe->setTimeOutTime(timeOut);
        }
        if(reTrig==1)
            g_pStrobe->setOnOff(0);
        if(bOn==0)
        {
            //g_pStrobe->setDuty(duty);
            g_pStrobe->setTimeOutTime(timeOut);
            //g_pStrobe->setOnOff(0);
            g_pStrobe->setOnOff(1);
        }
    }
    //2led
    if(cust_isDualFlashSupport(m_sensorDev)==1)
    {
        g_pStrobe2->isOn(&bOn);
        g_pStrobe2->getDuty(&dutyCur);
        if(dutyLT==-1)
            g_pStrobe2->setOnOff(0);
        else
        {
            //else if(bOn==0 || dutyCur!=dutyLT ||reTrig==1)
            if( dutyCur!=dutyLT)
            {
                g_pStrobe2->setDuty(dutyLT);
                g_pStrobe2->setTimeOutTime(timeOut2);
            }
            if(reTrig==1)
                g_pStrobe2->setOnOff(0);
            if(bOn==0)
            {
                //g_pStrobe2->setDuty(dutyLT);
                g_pStrobe2->setTimeOutTime(timeOut2);
                //g_pStrobe2->setOnOff(0);
                g_pStrobe2->setOnOff(1);
            }
        }
    }
}

void FlashMgrM::hwSetFlashOff()
{
    Mutex::Autolock lock(mLock);
    mStrobeState = e_StrobeOff;
    g_pStrobe->setOnOff(0);
    if(cust_isDualFlashSupport(m_sensorDev)==1)
        g_pStrobe2->setOnOff(0);

    if(g_onOffCallBackFunc!=0)
        g_onOffCallBackFunc(0);
}

void FlashMgrM::setTorchLevel(int level)
{
    {
        Mutex::Autolock lock(mLock);
        logV("setTorchLevel %d", level);
        mTorchLevel = level;
    }
    if(mStrobeState==e_StrobeTorch)
        turnOnTorch();
}


void FlashMgrM::turnOnTorch()
{
    Mutex::Autolock lock(mLock);
    mStrobeState = e_StrobeTorch;
    int level;
    if(mTorchLevel==-1)
        setFlashOn(mpStrobeNvbuf->engLevel.torchDuty, 0, mpStrobeNvbuf->engLevelLT.torchDuty, 0, 0);
    else //if(level<20)
    {
        int duty1 = mpStrobeNvbuf->engLevel.torchDutyEx[mTorchLevel];
        int duty2 = mpStrobeNvbuf->engLevelLT.torchDutyEx[mTorchLevel];
        setFlashOn(duty1, 0, duty2, 0, 0);
    }
}
void FlashMgrM::turnOnPf()
{
    Mutex::Autolock lock(mLock);
    mStrobeState = e_StrobePf;
    int duty;
    int dutyLT;
    if(mCapInfo.vBat< mpStrobeNvbuf->engLevel.vBatL  && mpStrobeNvbuf->engLevel.IChangeByVBatEn)
    {
        duty=mpStrobeNvbuf->engLevel.pfDutyL;
        dutyLT=mpStrobeNvbuf->engLevelLT.pfDutyL;
    }
    else if( mIsMultiCapture==1 && mpStrobeNvbuf->engLevel.IChangeByBurstEn)
    {
        duty=mpStrobeNvbuf->engLevel.pfDutyB;
        dutyLT=mpStrobeNvbuf->engLevelLT.pfDutyB;
    }
    else
    {
        duty=mpStrobeNvbuf->engLevel.pfDuty;
        dutyLT=mpStrobeNvbuf->engLevelLT.pfDuty;
    }
    setFlashOn(duty, 20000, dutyLT, 20000, 0);

}
void FlashMgrM::turnOnAf()
{
    Mutex::Autolock lock(mLock);
    mStrobeState = e_StrobeAF;
    setFlashOn(mpStrobeNvbuf->engLevel.afDuty, 0, mpStrobeNvbuf->engLevelLT.afDuty, 0, 0);
}
int FlashMgrM::getFlashModeStyle(int /*sensorType*/, int flashMode)
{
    return cust_getFlashModeStyle(m_sensorDev, flashMode);
}
FLASH_PROJECT_PARA& FlashMgrM::getAutoProjectPara()
{
    return getFlashProjectPara(LIB3A_AE_SCENE_AUTO, 0);
}

FLASH_PROJECT_PARA& FlashMgrM::getFlashProjectPara(int aeScene, int isisForceFlash)
{
    logV("sensorDev=%d aeMode=%d",m_sensorDev,aeScene);
    return cust_getFlashProjectPara_V3(m_sensorDev, aeScene, isisForceFlash, mpStrobeNvbuf);
}

//==============================
FlashMgrM::~FlashMgrM()
{
    delete []mEngName;
}
//==============================

int FlashMgrM::SaveFlashMgrInfo(const char *fname)
{
    strncpy(mEngName, fname, 200*sizeof(char)); // size defined in constructor
    return 0;
}

void FlashMgrM::hw_setPfPline(FlashAlg* pStrobeAlg)
{
    logV("hw_setPfPline() line=%d\n",__LINE__);
    strAETable pfPlineTab;
    strAETable capPlineTab;
    strAETable strobePlineTab;
    strAFPlineInfo pfPlineInfo;
    AE_DEVICES_INFO_T devInfo;
    AeMgr::getInstance(m_sensorDev).getCurrentPlineTable(pfPlineTab, capPlineTab, strobePlineTab, pfPlineInfo);
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);
    PLine pfPline;

    FlashPline plineAdapter;
    plineAdapter.aePline2FlashAlgPline(&pfPline, &pfPlineTab, 0);
    pStrobeAlg->setPreflashPLine(&pfPline, devInfo.u4MiniISOGain);
    logV("hw_setPfPline() line=%d u4MiniISOGain=%d\n",__LINE__,devInfo.u4MiniISOGain);
}

//==============================
void FlashMgrM::hw_setCapPline(FLASH_PROJECT_PARA* pPrjPara, FlashAlg* pStrobeAlg)
{
    logV("line=%d hw_setCapPline()\n",__LINE__);
    strAETable pfPlineTab;
    strAETable capPlineTab;
    strAETable strobePlineTab;
    strAFPlineInfo pfPlineInfo;
    AE_DEVICES_INFO_T devInfo;

    AeMgr::getInstance(m_sensorDev).getCurrentPlineTable(pfPlineTab, capPlineTab, strobePlineTab, pfPlineInfo);
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);
    //FlashPlineTool::aePlineWrite("/sdcard/aep.txt", &capPlineTab);

    PLine capPline;
    logV("hw_setCapPline() line=%d pPrjPara->maxCapExpTimeUs=%d\n",__LINE__, pPrjPara->maxCapExpTimeUs);

    FlashPline fpline;
    fpline.aePline2FlashAlgPline(&capPline, &capPlineTab, pPrjPara->maxCapExpTimeUs); //capPline:algPline
    //FlashPlineTool::flashAlgPlineWrite("/sdcard/flashAlgP.txt", &capPline);

    int cap2PreRatio;
    if(eAppMode_ZsdMode==mCamMode)
        cap2PreRatio=1024;
    else
        cap2PreRatio=devInfo.u4Cap2PreRatio;
    pStrobeAlg->setCapturePLine(&capPline, devInfo.u4MiniISOGain* cap2PreRatio/1024);  //u4Cap2PreRatio: 1024 base, <1

    logV("line=%d u4MiniISOGain=%d cap/preview = %d(device), %d(real)\n",__LINE__,devInfo.u4MiniISOGain, devInfo.u4Cap2PreRatio, cap2PreRatio);
}
//==============================
int FlashMgrM::setManualFlash(int duty, int dutyLt)
{
    logV("setManualFlash %d %d",duty, dutyLt);
    setFlashMode(LIB3A_FLASH_MODE_FORCE_ON);
    g_manualDuty=-1;
    g_manualDutyLt=-1;
    if(duty==0 && dutyLt==0)
    {
        g_isManualFlashEn=0;
    }
    else
    {
        g_isManualFlashEn=1;
        if(duty!=0)
            g_manualDuty = mpStrobeNvbuf->engLevel.mfDutyMax;

        if(dutyLt!=0)
            g_manualDutyLt = mpStrobeNvbuf->engLevelLT.mfDutyMax;
    }
    logV("setManualFlash -- en=%d %d %d", g_isManualFlashEn, g_manualDuty, g_manualDutyLt);
    return 0;
}
int FlashMgrM::clearManualFlash()
{
    logV("clearManualFlash ");
    setFlashMode(LIB3A_FLASH_MODE_FORCE_OFF);
    g_isManualFlashEn=0;
    logV("clearManualFlash --");
    return 0;
}
#define max(a,b) ((a)>(b)?(a):(b));
#define min(a,b) ((a)>(b)?(b):(a));
void FlashMgrM::hw_setFlashProfile(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara, NVRAM_CAMERA_STROBE_STRUCT* pNvram)
{
    logV("hw_setFlashProfile()\n");
    int mfDutyMin;
    int mfDutyMax;
    int mfDutyMinLT;
    int mfDutyMaxLT;
    int isLow=0;
    g_pStrobe->isLowPower(&isLow);
    mfDutyMin = mpStrobeNvbuf->engLevel.mfDutyMin;
    mfDutyMax = mpStrobeNvbuf->engLevel.mfDutyMax;
    mfDutyMinLT = mpStrobeNvbuf->engLevelLT.mfDutyMin;
    mfDutyMaxLT = mpStrobeNvbuf->engLevelLT.mfDutyMax;
    mCapRep.isBatLow=0;
    if(mIsMultiCapture==1)
    {
        mfDutyMin = mpStrobeNvbuf->engLevel.mfDutyMinB;
        mfDutyMax = mpStrobeNvbuf->engLevel.mfDutyMaxB;
        mfDutyMinLT = mpStrobeNvbuf->engLevelLT.mfDutyMinB;
        mfDutyMaxLT = mpStrobeNvbuf->engLevelLT.mfDutyMaxB;
    }
    else if( (mCapInfo.vBat<mpStrobeNvbuf->engLevel.vBatL && mpStrobeNvbuf->engLevel.IChangeByVBatEn==1)
            || (isLow==1))
    {
        mCapRep.isBatLow=1;
        mfDutyMin = mpStrobeNvbuf->engLevel.mfDutyMinL;
        mfDutyMax = mpStrobeNvbuf->engLevel.mfDutyMaxL;
        mfDutyMinLT = mpStrobeNvbuf->engLevelLT.mfDutyMinL;
        mfDutyMaxLT = mpStrobeNvbuf->engLevelLT.mfDutyMaxL;
    }


    AE_DEVICES_INFO_T devInfo;
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);

    FlashAlgStrobeProfile pf;
    pf.iso =  devInfo.u4MiniISOGain*(pNvram->engTab.afe_gain*pNvram->engTab.isp_gain)/1024/1024 ;
    pf.exp = pNvram->engTab.exp;
    pf.distance = pNvram->engTab.distance;
    int dutyN;
    if(cust_isDualFlashSupport(m_sensorDev)==1)
    {
        dutyN = (pPrjPara->dutyNum+1)*(pPrjPara->dutyNumLT+1);
        logV("dual %d, %d %d %d", __LINE__, dutyN, pPrjPara->dutyNum, pPrjPara->dutyNumLT);
    }
    else
    {
        dutyN = pPrjPara->dutyNum;
        logV("single dutyN=%d", dutyN);
    }
    pf.dutyNum = dutyN;
    pf.stepNum = 1;
    pf.dutyTickNum = dutyN;
    pf.stepTickNum = 1;
    MemBuf mb;
    int* dutyTick;
    dutyTick = mb.newInt( pf.dutyNum);
    int stepTick[16];
    int i;
    for(i=0; i<pf.dutyNum; i++)
        dutyTick[i]=i;
    for(i=0; i<pf.stepNum; i++)
        stepTick[i]=i;
    pf.dutyTick = dutyTick;
    pf.stepTick = stepTick;

    float *engTable;
    engTable = new float[pf.dutyNum];

    for(i=0; i<pf.dutyNum; i++)
    {
        engTable[i]=-1;;
    }
    if(cust_isDualFlashSupport(m_sensorDev)==1)
    {
        int ind;
        int j;
        FlashIMapFP iFuncPtr;
        iFuncPtr = cust_getFlashIMapFunc(m_sensorDev);
        double iMax;
        double iMin;
        //iMax = iFuncPtr(mfDutyMax, mfDutyMaxLT);
        //iMin = iFuncPtr(mfDutyMin, mfDutyMinLT);

        double iMaxl;
        double iMinl;
        double iMaxLT;
        double iMinLT;
        if(mIsMultiCapture){
            iMaxl= iFuncPtr(mfDutyMax, -1);
            iMinl= iFuncPtr(mfDutyMin, -1);
            iMaxLT= iFuncPtr(-1, mfDutyMaxLT);
            iMinLT= iFuncPtr(-1, mfDutyMinLT);
            iMax = min(iMaxl, iMaxLT);
            iMin = max(iMinl, iMinLT);
        }
        else{
            iMax = iFuncPtr(mfDutyMax, mfDutyMaxLT);
            iMin = iFuncPtr(mfDutyMin, mfDutyMinLT);
        }



        FlashIndConvPara conv;
        conv.dutyNum = pPrjPara->dutyNum;
        conv.dutyNumLt = pPrjPara->dutyNumLT;
        conv.isDual = 1;

        int mfPropOn;
        mfPropOn = FlashUtil::getPropInt(PROP_MF_ON_STR,-1);


        if( (mfPropOn==-1 || mfPropOn==0) && (g_isManualFlashEn==0))
        {
            for (j = mfDutyMinLT; j <= mfDutyMaxLT; j++)
                for (i = mfDutyMin; i <= mfDutyMax; i++) {
                    conv.duty = i;
                    conv.dutyLt = j;
                    duty2ind(conv);
                    engTable[conv.ind] = pNvram->engTab.yTab[conv.ind];
                }
            /*
               for(j=-1; j<pPrjPara->dutyNumLT; j++)
               for(i=-1; i<pPrjPara->dutyNum; i++)
               {
               conv.duty = i;
               conv.dutyLt = j;
               duty2ind(conv);
               double eng;
               eng=  pNvram->engTab.yTab[conv.ind];
               if(eng!=-1)
               {
               int iCur;
               iCur = iFuncPtr(i,j);
               if(iCur>=iMin && iCur<=iMax)
               engTable[conv.ind]=eng;
               }
               }
             */
        }
        else
        {
            int d1;
            int d2;
            d1 = FlashUtil::getPropInt(PROP_MF_DUTY_STR,-1);
            d2 = FlashUtil::getPropInt(PROP_MF_DUTYLT_STR,-1);
            if(g_isManualFlashEn==1)
            {
                d1=g_manualDuty;
                d2=g_manualDutyLt;
            }
            conv.duty = d1;
            conv.dutyLt = d2;
            duty2ind(conv);
            double eng;
            eng=  pNvram->engTab.yTab[conv.ind];
            int iCur;
            iCur = iFuncPtr(d1,d2);
            engTable[conv.ind]=eng;
            if(eng==-1)
                engTable[conv.ind]=100;
        }
    }
    else
    {
        // error check for mfDutyMin/mfDutyMax
        if(mfDutyMin < 0 || mfDutyMin > pf.dutyNum-1){
            logE("mfDutyMin(%d) out of bound and set to 0, check tuning", mfDutyMin);
            mfDutyMin = 0;
        }
        if(mfDutyMax < 0 || mfDutyMax > pf.dutyNum-1){
            mfDutyMax = pf.dutyNum-1;
            logE("mfDutyMax(%d) out of bound and set to dutyNum-1, check tuning", mfDutyMax);
        }

        for(i=mfDutyMin; i<=mfDutyMax; i++)
        {
            engTable[i]=pNvram->engTab.yTab[i];
        }
    }
    pf.engTab = engTable;

    // dual-flash
    if(cust_isDualFlashSupport(m_sensorDev)==1)
    {
        int ind = (mpStrobeNvbuf->engLevel.pfDuty+1)+(mpStrobeNvbuf->engLevelLT.pfDuty+1)*(pPrjPara->dutyNum+1);
        pf.pfEng = pNvram->engTab.yTab[ind];
    }
    else
    {
        pf.pfEng = pNvram->engTab.yTab[mpStrobeNvbuf->engLevel.pfDuty];
    }

    int err;
    NVRAM_CAMERA_3A_STRUCT* buf;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_sensorDev, (void*&)buf);
    if(err!=0)
        logE("err=%d",err);
    pf.pFlWbCalTbl  = buf->rFlashAWBNVRAM.rCalibrationData.flashWBGain;

    pStrobeAlg->setFlashProfile(&pf);
    delete []engTable;

}
//==============================
void FlashMgrM::hw_setPreference(FlashAlg* pStrobeAlg, FLASH_PROJECT_PARA* pPrjPara)
{
    int bv = AeMgr::getInstance(m_sensorDev).getBVvalue();

    pStrobeAlg->setDefaultPreferences();
    pStrobeAlg->setTuningPreferences(&pPrjPara->tuningPara, bv+50);
    int maxTar;
    int num;
    float* evIndTab;
    float* evTab;
    float* evLevel;

    cust_getEvCompPara(maxTar, num, evIndTab, evTab, evLevel);

    float evComp;
    float evLevelRet;
    evComp = FlashUtil::flash_calYFromXYTab(num, evIndTab, evTab, (float)mCapInfo.evComp);
    evLevelRet = FlashUtil::flash_calYFromXYTab(num, evIndTab, evLevel, (float)mCapInfo.evComp);

    logV("evcompx=%d %d %d",(int)(mCapInfo.evComp*100), (int)(evComp*100), (int)(evLevelRet*100));
    pStrobeAlg->setEVCompEx(evComp, maxTar, evLevelRet);
    pStrobeAlg->setDebugDataSize(ALG_TAG_SIZE);
    //logI("hw_setPreference() yTar=%d",tune_p.yTar);
}

//==============================
void FlashMgrM::hw_turnOffFlash()
{
    hwSetFlashOff();

}
//==============================

void FlashMgrM::hw_setExpPara(FlashAlgExpPara* expPara, int /*sensorType*/, FLASH_PROJECT_PARA* /*pPrjPara*/)
{
    logV("hw_setExpPara pfexp1 %d %d %d exp=%d iso=%d",
            expPara->isFlash,    expPara->duty,    expPara->step,    expPara->exp,    expPara->iso);

    int exp;
    int iso;
    int afe;
    int isp;
    int isFlash;
    int duty;
    int step;

    int propFollowPline;
    int propMaxAfe;
    propFollowPline = FlashUtil::getPropInt(PROP_PF_EXP_FOLLOW_PLINE,-1);
    propMaxAfe = FlashUtil::getPropInt(PROP_PF_MAX_AFE,-1);

    if(propFollowPline==-1)
    {
        strAETable pfPlineTab;
        strAETable capPlineTab;
        strAETable strobePlineTab;
        strAFPlineInfo pfPlineInfo;
        AeMgr::getInstance(m_sensorDev).getCurrentPlineTable(pfPlineTab, capPlineTab, strobePlineTab, pfPlineInfo);

        logV("info_en %d", (int)pfPlineInfo.bAFPlineEnable);

        if(pfPlineInfo.bAFPlineEnable)
            hw_speedUpExpPara(expPara, 0);
    }
    else if(propFollowPline==0)
    {
        if(propMaxAfe==-1)
            hw_speedUpExpPara(expPara, 0);
        else
            hw_speedUpExpPara(expPara, propMaxAfe);
    }

    exp = expPara->exp;
    iso = expPara->iso;
    hw_isoToGain(iso, &afe, &isp);
    step = expPara->step;
    duty = expPara->duty;
    isFlash = expPara->isFlash;

    int err;
    if(isFlash)
    {
        turnOnPf();
    }
    else
    {
        hwSetFlashOff();
    }

    logV("hw_setExpPara pfexp2 %d %d %d exp %d %d, %d %d",
            isFlash, duty, step, exp, iso,
            afe,    isp    );

    err = AAASensorMgr::getInstance().setSensorExpTime(m_sensorDev, exp);
    if (FAILED(err))
        return;

    err = AAASensorMgr::getInstance().setSensorGain(m_sensorDev, afe);
    if (FAILED(err))
        return;
    /* Modify
       AE_INFO_T rAEInfo2ISP;
       rAEInfo2ISP.u4Eposuretime = exp;
       rAEInfo2ISP.u4AfeGain = afe;
       rAEInfo2ISP.u4IspGain = isp;
       rAEInfo2ISP.u4RealISOValue = iso;
       IspTuningMgr::getInstance().setAEInfo(m_sensorDev, rAEInfo2ISP);
       IspTuningMgr::getInstance().setISPAEGain((ESensorDev_T)sensorType, MFALSE, isp>>1);
     */

    AeMgr::getInstance(m_sensorDev).setPfPara(exp,afe,isp);
    g_pfExpPara.exp = exp;
    g_pfExpPara.afeGain = afe;
    g_pfExpPara.ispGain = isp;

}
//==============================
void FlashMgrM::hw_setCapExpPara(FlashAlgExpPara* expPara)
{

    logV("hw_setCapExpPara mfexp1 %d %d %d exp=%d iso=%d",
            expPara->isFlash,    expPara->duty,    expPara->step,    expPara->exp,    expPara->iso);

    int propExp;
    int propIso;
    int propAfe;
    int propIsp;
    propExp = FlashUtil::getPropInt(PROP_MF_EXP_STR,-1);
    propIso = FlashUtil::getPropInt(PROP_MF_ISO_STR,-1);
    propAfe = FlashUtil::getPropInt(PROP_MF_AFE_STR,-1);
    propIsp = FlashUtil::getPropInt(PROP_MF_ISP_STR,-1);

    int iso;
    int exp;
    int afe;
    int isp;
    exp = expPara->exp;
    iso = expPara->iso;
    hw_capIsoToGain(iso, &afe, &isp);

    //prop
    if(propExp!=-1)
        exp = propExp;
    if(propIso!=-1)
    {
        iso=propIso;
        hw_capIsoToGain(iso, &afe, &isp);
    }
    if(propAfe!=-1)
        afe = propAfe;
    if(propIsp!=-1)
        isp = propIsp;

    AE_MODE_CFG_T capInfo;
    AeMgr::getInstance(m_sensorDev).getCaptureParams(capInfo);
    capInfo.u4Eposuretime = exp;
    capInfo.u4AfeGain = afe;
    capInfo.u4IspGain = isp;

    AeMgr::getInstance(m_sensorDev).updateCaptureParams(capInfo);

    logV("hw_setExpPara mfexp2 %d %d %d exp %d %d, %d %d",
            expPara->isFlash,    expPara->duty,    expPara->step, exp, iso,
            afe,    isp    );

}
//==============================
//int whAll, float r, int* whStart, int* whDiv, int* bin, int whDivMin, int whDivMax
//for x dirction example (120x90):
// whAll (in): 120
// r (in): ratio = 3
// whStart (out)  = 40
// whDiv (out): 20
// bin (out): 2
// whDivMin (in)
// whDivMax (out)
void calFlashDigWinNum(int whAll, float r, int* whStart, int* whDiv, int* bin, int whDivMin, int whDivMax)
{
    float whTar;
    whTar = whAll/r;
    whDivMin = (whDivMin+1) & 0xffe;
    whDivMax = whDivMax & 0xffe;

    float* TestErr;
    int* TestV;
    int testNum;
    testNum = (whDivMax-whDivMin)/2+1;
    TestErr = new float [testNum];
    TestV = new int [testNum];

    int whDigRet;
    int binRet;

    if((int)whTar<=whDivMax)
    {
        whDigRet = ((int)whTar)/2*2;
        binRet = 1;
    }
    else
    {
        float minErr;
        minErr=10000;
        binRet=1;
        whDigRet = ((int)whTar)/2*2;
        int i;
        for(i=0; i<=whTar/2; i++)
        {
            int ind;
            ind=whDivMax-2*i;
            if(ind<whDivMin)
                break;
            TestV[i]= ind;
            TestErr[i] =  whTar-(int)(whTar/ind)*ind;
            if(TestErr[i]==0)
            {
                whDigRet=ind;
                binRet=(int)(whTar/ind);
                break;
            }
            else
            {
                if(minErr>TestErr[i])
                {
                    minErr=TestErr[i];
                    whDigRet=ind;
                    binRet=(int)(whTar/ind);
                }
            }
        }
    }
    *whDiv = whDigRet;
    *bin = binRet;
    *whStart = (whAll - whDigRet*binRet)/2;

    delete []TestErr;
    delete []TestV;
}
//r: digital zoom
//w: data w
//h: data h
//z0Wdiv: no digital zoom's wdiv
//z0Wdiv: no digital zoom's hdiv
//rzData: resized data
void resizeYData(double r, short* data, int w, int h, int z0Wdiv, int z0Ydiv, short* rzData, int* rzW, int* rzH)
{
    int i;
    int j;
    int wst;
    int wdiv;
    int wbin;
    int hst;
    int hdiv;
    int hbin;
    if(r<1.05)
    {
        wdiv = z0Wdiv;
        hdiv = z0Ydiv;
        wbin=w/wdiv;
        hbin=h/hdiv;
        wst= (w-wbin*wdiv)/2;
        hst= (h-hbin*hdiv)/2;
    }
    else
    {

        calFlashDigWinNum(w, r, &wst, &wdiv, &wbin, 20, 24);
        calFlashDigWinNum(h, r, &hst, &hdiv, &hbin, 15, 18);
        double werr;
        double herr;
        werr = (double)(w/r - wdiv*wbin)/ (w/r);
        herr = (double)(h/r - hdiv*hbin)/ (h/r);
        if(werr>0.1 || werr<-0.1)
        {
            calFlashDigWinNum(w, r, &wst, &wdiv, &wbin, 10, 24);
        }
        if(herr>0.1 || herr<-0.1)
        {
            calFlashDigWinNum(h, r, &hst, &hdiv, &hbin, 10, 18);
        }
    }
    for(i=0; i<wdiv*hdiv; i++)
        rzData[i]=0;

    for(j=hst; j<hst+hbin*hdiv; j++)
        for(i=wst; i<wst+wbin*wdiv; i++)
        {
            int id;
            int jd;
            id = (i-wst)/wbin;
            jd = (j-hst)/hbin;
            rzData[id+wdiv*jd]+=data[i+j*w];
        }
    for(i=0; i<wdiv*hdiv; i++)
        rzData[i]=rzData[i]/(wbin*hbin);
    *rzW = wdiv;
    *rzH = hdiv;
}
//==============================

int FlashMgrM::setDigZoom(int digx100)
{
    m_digRatio = digx100/100.0;
    return 0;
}
//==============================
void FlashMgrM::hw_convert3ASta(FlashAlgStaData* staData, void* staBuf, int winX, int winY)
{

    short* A3Y = new short[winX*winY]();
    short* pData;
    pData = staData->data;
    convert3aSta_romeHw_y(staBuf, winX, winY, A3Y, 4);

    /*
       FILE* fp;
       char ss[100];
       sttatic int kkk=0;
       kkk++;
       sprintf(ss,"/sdcard/flash_%03d.raw",kkk);
       fp = fopen(ss,"wb");
       fwrite(A3Y, 1, winX*winY,fp);
       fclose(fp);
       sprintf(ss,"/sdcard/flash_%03d.bmp",kkk);
       arrayToBmp(ss, A3Y, A3Y, A3Y, winX, winY, 1024);
     */

    int rzW;
    int rzH;
    resizeYData(mCapInfo.digRatio, A3Y, winX, winY, 24, 18, pData, &rzW, &rzH);
    logV("line=%d hw_convert3ASta digRatio=%lf, rzW=%d, rzH=%d", __LINE__, (double)mCapInfo.digRatio, rzW, rzH);
    staData->row = rzH;
    staData->col = rzW;
    staData->bit = 10;
    staData->normalizeFactor =1;
    staData->dig_row = 0;
    staData->dig_col = 0;
    delete []A3Y;
}
//==============================
void FlashMgrM::hw_getAEExpPara(FlashAlgExpPara* aePara)
{
    AE_MODE_CFG_T previewInfo;
    AeMgr::getInstance(m_sensorDev).getPreviewParams(previewInfo);
    AE_DEVICES_INFO_T devInfo;
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);

    double gain;
    gain = (double)previewInfo.u4AfeGain*previewInfo.u4IspGain/1024/1024;
    int iso;
    iso = gain* devInfo.u4MiniISOGain;
    aePara->iso=iso;
    aePara->isFlash=0;
    aePara->exp=previewInfo.u4Eposuretime;

    logV("aeexp %d %d %d %d minIsoGain=%d", previewInfo.u4Eposuretime, iso, previewInfo.u4AfeGain, previewInfo.u4IspGain, devInfo.u4MiniISOGain);

}
//==============================
void FlashMgrM::hw_speedUpExpPara(FlashAlgExpPara* expPara, int maxAfe)
{
    //re-calculate iso
    strAETable pfPlineTab;
    strAETable capPlineTab;
    strAETable strobePlineTab;
    strAFPlineInfo pfPlineInfo;
    int now_bv;
    AeMgr::getInstance(m_sensorDev).getCurrentPlineTable(pfPlineTab, capPlineTab, strobePlineTab, pfPlineInfo);
    now_bv = AeMgr::getInstance(m_sensorDev).getBVvalue();

    logV("hw_speedUpExpPara bv=%d maxAfe=%d",now_bv, maxAfe);

    int maxIso;
    AE_DEVICES_INFO_T devInfo;
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);

    if(maxAfe!=0)
        hw_gainToIso(maxAfe, 15*1024, &maxIso);
    else
        hw_gainToIso(devInfo.u4MaxGain, 15*1024, &maxIso);

    logV("maxIso=%d",maxIso);

    logV("info_en %d", (int)pfPlineInfo.bAFPlineEnable);
    logV("info_frm1 %d %d", (int)pfPlineInfo.i2FrameRate[0][0], (int)pfPlineInfo.i2FrameRate[0][1]);
    logV("info_frm2 %d %d", (int)pfPlineInfo.i2FrameRate[1][0], (int)pfPlineInfo.i2FrameRate[1][1]);
    logV("info_frm3 %d %d", (int)pfPlineInfo.i2FrameRate[2][0], (int)pfPlineInfo.i2FrameRate[2][1]);
    logV("info_frm4 %d %d", (int)pfPlineInfo.i2FrameRate[3][0], (int)pfPlineInfo.i2FrameRate[3][1]);
    logV("info_frm5 %d %d", (int)pfPlineInfo.i2FrameRate[4][0], (int)pfPlineInfo.i2FrameRate[4][1]);

    int lvTab[5];
    int fpsTab[5];
    int i;
    for(i=0; i<5; i++)
    {
        lvTab[i]=pfPlineInfo.i2FrameRate[i][0];
        fpsTab[i]=pfPlineInfo.i2FrameRate[i][1];
    }
    int fpsRet;
    int reducedExp;
    FlashUtil::flash_sortxy_xinc(5, lvTab, fpsTab);
    fpsRet = FlashUtil::flash_calYFromXYTab(5, lvTab, fpsTab, now_bv+50);
    reducedExp = 1000000/fpsRet;

    float g;
    g = (float)expPara->exp/reducedExp;
    float maxG;
    maxG = (float)maxIso*0.95/expPara->iso;
    logV("hw_speedUpExpPara exp=%d iso=%d g=%f mxG=%f", expPara->exp, expPara->iso, g, maxG);
    if(g>maxG)
        g=maxG;
    if(g>1)
    {
        int expNew;
        expNew = reducedExp;
        double align;
        if(mCapInfo.flickerMode==FlashMgrM::e_Flicker60)
            align=1e6/(double)120;
        else  //50
            align=10000;

        int expNew1;
        int expNew2;
        expNew1 = ((int)(expNew/align))*align;
        expNew2 = ((int)(expNew/align)+1)*align;

        double g1=0;
        if(expNew1!=0)
            g1 = (double)expPara->exp/expNew1;

        if(expNew2>expPara->exp)
        {
            if(g1>maxG || expNew1==0)
            {
                expNew=expPara->exp;
            }
            else
            {
                expNew=expNew1;
            }
        }
        else
        {
            if(expNew1==expNew)
                expNew=expNew1;
            else
                expNew=expNew2;
        }
#if 1
        double m;
        if(expNew==0) expNew=1;
        m= expPara->exp/(double)expNew;
        expPara->exp = expNew;
        expPara->iso = m* expPara->iso;
#else
        expPara->exp = expPara->exp/g;
        expPara->iso = g* expPara->iso

#endif
    }
    logV("hw_speedUpExpPara exp=%d iso=%d", expPara->exp, expPara->iso);
}

void FlashMgrM::hw_gainToIso(int afe, int isp, int* iso)
{
    AE_DEVICES_INFO_T devInfo;
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);
    double isoV;
    isoV = (double)devInfo.u4MiniISOGain*afe*isp/1024/1024;
    *iso = (int)isoV;

    logV("dev_1xGainIso %d", (int)devInfo.u4MiniISOGain);
    logV("dev_minG %d", (int)devInfo.u4MinGain);
    logV("dev_maxG %d", (int)devInfo.u4MaxGain);
    logV("line=%d hw_gainToIso afe=%d isp=%d iso=%d",__LINE__, afe, isp, *iso);
}
void FlashMgrM::hw_isoToGain(int iso, int* afe, int* isp)
{
    AE_DEVICES_INFO_T devInfo;
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);

    float g;
    g = (float)iso/devInfo.u4MiniISOGain;
    logV("line=%d hw_isoToGain=iso=%d gain=%5.3f",__LINE__, iso, g);

    if(g<devInfo.u4MaxGain/1024.0f)
    {
        *afe=g*1024;
        *isp = 1024;
    }
    else
    {
        *afe = devInfo.u4MaxGain;
        *isp = (g*1024/devInfo.u4MaxGain)*1024;
    }
    logV("dev_1xGainIso %d", (int)devInfo.u4MiniISOGain);
    logV("dev_minG %d", (int)devInfo.u4MinGain);
    logV("dev_maxG %d", (int)devInfo.u4MaxGain);
    logV("line=%d hw_isoToGain iso=%d afe=%d isp=%d (a=%5.3f)",__LINE__, iso, *afe, *isp, g);
}
//==============================
void FlashMgrM::hw_capIsoToGain(int iso, int* afe, int* isp)
{

    //eer recalculate iso
    AE_DEVICES_INFO_T devInfo;
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);
    float g;

    int cap2PreRatio;
    if(eAppMode_ZsdMode==mCamMode)
        cap2PreRatio=1024;
    else
        cap2PreRatio=devInfo.u4Cap2PreRatio;

    g = (float)iso/((double)devInfo.u4MiniISOGain* cap2PreRatio/1024);
    logV("line=%d hw_capIsoToGain=iso=%d gain=%5.3f",__LINE__, iso, g);

    if(g<devInfo.u4MaxGain/1024.0f)
    {
        *afe=g*1024;
        *isp = 1024;
    }
    else
    {
        *afe = devInfo.u4MaxGain;
        *isp = (g*1024/devInfo.u4MaxGain)*1024;
    }

    logV("hw_capIsoToGain dev_1xGainIso %d", (int)devInfo.u4MiniISOGain);
    logV("hw_capIsoToGain dev_minG %d", (int)devInfo.u4MinGain);
    logV("hw_capIsoToGain dev_maxG %d", (int)devInfo.u4MaxGain);
    logV("hw_capIsoToGain iso=%d afe=%d isp=%d (a=%5.3f)", iso, *afe, *isp, g);

}
//==============================
FlashMgrM* FlashMgrM::getInstance(int sensorDev)
{

    if(sensorDev==DUAL_CAMERA_MAIN_SENSOR)
    {
        static FlashMgrM objMain(sensorDev);
        FlashMgr::getInstance().setChildMain(&objMain);
        return &objMain;
    }
    else if(sensorDev==DUAL_CAMERA_SUB_SENSOR)
    {
        static FlashMgrM objSub(sensorDev);
        FlashMgr::getInstance().setChildSub(&objSub);
        return &objSub;
    }
    else //DUAL_CAMERA_MAIN2_SENSOR
    {
        static FlashMgrM objMain2(sensorDev);
        FlashMgr::getInstance().setChildMain2(&objMain2);
        return &objMain2;
    }
}
//==============================
/*
//torch set level by external thread test
#include <pthread.h>
MVOID* onSetTorchThread(MVOID *arg)
{
while(1)
{

static int torchLevelCur=0;
int torchLevel;
torchLevel = FlashUtil::getPropInt("z.flash_torch_level",0);
if(torchLevel!=torchLevelCur)
{
logI("torchLevel change from %d to %d %d",torchLevelCur, torchLevel, __LINE__);
torchLevelCur=torchLevel;
FlashMgr::getInstance().setTorchLevel(torchLevel);
}
usleep(100*1000);
};
return 0;
}*/
int FlashMgrM::init(int sensorId)
{
    /*
       static int vv=0;
       if(vv==0)
       {
       vv=1;
       pthread_t mThread;
       pthread_create(&mThread, NULL, onSetTorchThread, this);
       }*/
    int bCheckCaliResult;
    bCheckCaliResult = FlashUtil::getPropInt("vendor.flash_check_cali",0);
    logV("FlashMgrM::init(int sensorId) bCheckCaliResult=%d",bCheckCaliResult);
    if(bCheckCaliResult==1)
    {
        FLASH_PROJECT_PARA* pPrjPara;
        NVRAM_CAMERA_STROBE_STRUCT* pDef;
        NVRAM_CAMERA_STROBE_STRUCT* buf;
        pPrjPara = new FLASH_PROJECT_PARA[1];
        pDef = new NVRAM_CAMERA_STROBE_STRUCT[1];
        int err;
        int err2;
        *pPrjPara = getAutoProjectPara();
        err = NvBufUtil::getInstance().readDefault(CAMERA_NVRAM_DATA_STROBE, m_sensorDev, (void*)pDef);
        err2 = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_STROBE, m_sensorDev, (void*&)buf, 1);

        int caliSz;
        caliSz = (pPrjPara->dutyNum +1)*(pPrjPara->dutyNumLT+1);
        int i ;
        int dif=0;
        for(i=0;i<caliSz;i++)
        {
            if(pDef->engTab.yTab[i]!=buf->engTab.yTab[i])
            {
                dif=1;
                break;
            }
        }

        FILE* fp;
        char fname[]="/sdcard/flash_check_cali.txt";
        fp = fopen(fname,"wt");
        if(fp==0)
            logE("oepn file error(%s)",fname);
        else
        {
            if(dif==1)
                fprintf(fp, "@result with calibration");
            else
                fprintf(fp, "@result no calibration");
            fclose(fp);
        }

        delete []pDef;
        delete []pPrjPara;
    }

    return initSub(sensorId, 0);
}
int FlashMgrM::initSub(int sensorId, int isTemp)
{
    logV("initSub+ ln=%d",__LINE__);
    mIsInited=1;
    mSensorId = sensorId;
    g_isVerboseEn = FlashUtil::getPropInt(PROP_VERBOSE_EN,0);
    static FLASH_PROJECT_PARA prjPara;
    prjPara = getAutoProjectPara();



    g_pStrobe->setStrobeInfo(prjPara.dutyNum, prjPara.coolTimeOutPara.tabNum, prjPara.coolTimeOutPara.tabId, prjPara.coolTimeOutPara.timOutMs, prjPara.coolTimeOutPara.coolingTM);




    if(isTemp==1)
        g_pStrobe->initTemp();
    else
        g_pStrobe->init();

    if(cust_isDualFlashSupport(m_sensorDev)==1)
    {
        g_pStrobe2->setStrobeInfo(prjPara.dutyNumLT, prjPara.coolTimeOutParaLT.tabNum, prjPara.coolTimeOutParaLT.tabId, prjPara.coolTimeOutParaLT.timOutMs, prjPara.coolTimeOutParaLT.coolingTM);
        if(isTemp==1)
            g_pStrobe2->initTemp();
        else
            g_pStrobe2->init();
    }
    nvGetBuf(mpStrobeNvbuf);
    m_isAfState=0;
    logV("initSub- ln=%d",__LINE__);
    return 0;
}


int FlashMgrM::initTemp()
{
    return initSub(0, 1);
}

int FlashMgrM::create()
{
    return 0;
}

int FlashMgrM::destroy()
{
    if(mIsInited==1)
        uninit();
    return 0;
}


//==============================
int FlashMgrM::uninit()
{
    logV("uninitt");
    mIsInited=0;
    hwSetFlashOff();
    g_pStrobe->uninit();
    if(cust_isDualFlashSupport(m_sensorDev)==1)
        g_pStrobe2->uninit();
    //mFlash1Mode = LIB3A_FLASH_MODE_UNSUPPORTED;
    g_previewMode=e_NonePreview;
    g_uiSpMode=e_SpModeNormal;
    g_spMode=e_SpModeNormal;
    return 0;
}
//==============================
int FlashMgrM::isAFLampOn()
{
    int bOn;
    g_pStrobe->isOn(&bOn);
    return bOn;
}
int FlashMgrM::setAFLampOnOff(int en)
{
    if(en==1)
    {
        logV("setAFLampOnOff 1");
        turnOnAf();
    }
    else
    {
        logV("setAFLampOnOff 0");
        turnOffFlashDevice();
    }
    return 0;
}
//==============================
int FlashMgrM::setTorchOnOff(int en)
{
    if(en==1)
    {
        logV("setTorchOnOff 1");
        turnOnTorch();
    }
    else
    {
        logV("setTorchOnOff 0");
        turnOffFlashDevice();
    }
    return 0;
}
//==============================
void FlashMgrM::addErr(int err)
{
    mCapRep.errCnt++;
    mCapRep.err3=mCapRep.err2;
    mCapRep.err2=mCapRep.err1;
    mCapRep.err1=err;

    mCapRep.errTime3=mCapRep.errTime2;
    mCapRep.errTime2=mCapRep.errTime1;
    mCapRep.errTime1=FlashUtil::getMs();

}
//==============================
int FlashMgrM::getDebugInfo(FLASH_DEBUG_INFO_T* p)
{
    logV("getDebugInfo");
    int sz;
    sz = sizeof(FLASH_DEBUG_INFO_T);
    memset(p, 0, sz);

    setDebugTag(*p, FL_T_VERSION, (MUINT32) FLASH_DEBUG_TAG_VERSION);
    setDebugTag(*p, FL_T_IS_FLASH_ON, (MUINT32) m_flashOnPrecapture);
    setDebugTag(*p, FL_T_DUTY, (MUINT32) mCapRep.capDuty);
    setDebugTag(*p, FL_T_DUTY_LT, (MUINT32) mCapRep.capDutyLT);
    setDebugTag(*p, FL_T_EXP_TIME, (MUINT32) mCapRep.capExp);
    setDebugTag(*p, FL_T_ISO, (MUINT32) mCapRep.capIso);
    setDebugTag(*p, FL_T_AFE_GAIN, (MUINT32) mCapRep.capAfeGain);
    setDebugTag(*p, FL_T_ISP_GAIN, (MUINT32) mCapRep.capIspGain);
    setDebugTag(*p, FL_T_DUTY_NUM, (MUINT32) mCapRep.dutyNum);
    setDebugTag(*p, FL_T_DUTY_LT_NUM, (MUINT32) mCapRep.dutyNumLT);
    setDebugTag(*p, FL_T_PF_START, (MUINT32) mCapRep.pfStartTime);
    setDebugTag(*p, FL_T_PF_RUN, (MUINT32) mCapRep.pfRunTime);
    setDebugTag(*p, FL_T_PF_END, (MUINT32) mCapRep.pfEndTime);
    setDebugTag(*p, FL_T_MF_START, (MUINT32) mCapRep.capStartTime);
    setDebugTag(*p, FL_T_MF_END, (MUINT32) mCapRep.capEndTime);
    setDebugTag(*p, FL_T_TIME_OUT, (MUINT32) mCapRep.capSetTimeOutTime);
    setDebugTag(*p, FL_T_TIME_OUT_LT, (MUINT32) mCapRep.capSetTimeOutTimeLT);
    setDebugTag(*p, FL_T_PF_EPOCH_FRAMES, (MUINT32) mCapRep.pfIteration*10000+mCapRep.pfRunFrms);
    setDebugTag(*p, FL_T_VBAT, (MUINT32) mCapInfo.vBat);
    setDebugTag(*p, FL_T_FLASH_MODE, (MUINT32) mCapInfo.flash1Mode);
    setDebugTag(*p, FL_T_SCENE_MODE, (MUINT32) mCapInfo.aeScene);
    setDebugTag(*p, FL_T_EV_COMP, (MUINT32) mCapInfo.evComp);
    setDebugTag(*p, FL_T_DIG_ZOOM_100, (MUINT32) (mCapInfo.digRatio*100+0.5));
    setDebugTag(*p, FL_T_IS_BURST, (MUINT32) mCapInfo.isBurst);
    setDebugTag(*p, FL_T_IS_BATLOW_TIMEOUT, (MUINT32) mCapRep.isBatLow*10000+mCapRep.capIsTimeOut);
    setDebugTag(*p, FL_T_ERR_NUM, (MUINT32) mCapRep.errCnt);
    setDebugTag(*p, FL_T_ERR1, (MUINT32) mCapRep.err1);
    setDebugTag(*p, FL_T_ERR2, (MUINT32) mCapRep.err2);
    setDebugTag(*p, FL_T_ERR3, (MUINT32) mCapRep.err3);
    setDebugTag(*p, FL_T_ERR1_TIME, (MUINT32) mCapRep.errTime1);
    setDebugTag(*p, FL_T_ERR2_TIME, (MUINT32) mCapRep.errTime2);
    setDebugTag(*p, FL_T_ERR3_TIME, (MUINT32) mCapRep.errTime3);




    //setDebugTag(*p, FL_T_SCENE_MODE, (MUINT32) mCapRep.sceneMode);



    if(m_flashOnPrecapture==1)
    {
        int algDebug[ALG_TAG_SIZE/4];
        int i;
        if(m_bRunPreFlash)
        {
            logV("alg debug data+ ln=%d",__LINE__);
            mpFlashAlg->fillDebugData2(algDebug);
            logV("alg debug data- ln=%d",__LINE__);
        }
        else
        {
            for(i=0; i<ALG_TAG_SIZE/4; i++)
                algDebug[i]=0;
        }

        for(i=0; i<ALG_TAG_SIZE/4; i++)
        {
            setDebugTag(*p, FL_T_NUM+i, (MUINT32)algDebug[i]);
        }
    }
    //if(strlen(mEngName)!=0)
    //mpFlashAlg->SaveFlashCoreInfo(mEngName);
    return 0;
}
///////////////////////////////////////////

//int FlashMgrM::cctGetFlashInfo(void* in, int inSize, void* out, int outSize, MUINT32* realOutSize)
int FlashMgrM::cctGetFlashInfo(int* isOn)
{
    logV("cctGetFlashInfo line=%d",__LINE__);
    if(getFlashMode()==FLASHLIGHT_FORCE_ON)
    {
        *isOn = 1;
    }
    else
    {
        *isOn = 0;
    }
    return 0;
}

int FlashMgrM::cctFlashLightTest(void* /*pIn*/)
{
    logV("%s() line=%d", __FUNCTION__, __LINE__);
    //int* p;
    //p = (int*)pIn;
    int duration;
    duration = 300000;

    nvGetBuf(mpStrobeNvbuf);

    //logI("cctFlashLightTest() p[0]=%d, p[1]=%d", p[0], p[1]);
    logV("%s() line=%d", __FUNCTION__, __LINE__);
    int err=0;
    int e;
    g_pStrobe->init();
    if(cust_isDualFlashSupport(m_sensorDev)==1)
        g_pStrobe2->init();
    logV("%s() line=%d", __FUNCTION__, __LINE__);
    g_pStrobe->setDuty(mpStrobeNvbuf->engLevel.torchDuty);
    logV("cctFlashLightTest() duty=%d", mpStrobeNvbuf->engLevel.torchDuty);
    logV("%s() line=%d", __FUNCTION__, __LINE__);
    g_pStrobe->setTimeOutTime(2000);
    logV("%s() line=%d", __FUNCTION__, __LINE__);
    err = g_pStrobe->setOnOff(0);
    e   = g_pStrobe->setOnOff(1);
    logV("%s() line=%d", __FUNCTION__, __LINE__);
    if(err==0)
        err = e;
    usleep(duration);
    e   = g_pStrobe->setOnOff(0);
    if(err==0)
        err = e;
    usleep(duration);
    g_pStrobe->uninit();
    if(cust_isDualFlashSupport(m_sensorDev)==1)
        g_pStrobe2->uninit();
    logV("%s() line=%d", __FUNCTION__, __LINE__);
    logV("cctFlashLightTest() err=%d", err);
    return err;
}

int FlashMgrM::isNeedWaitCooling(int curMs, int* a_waitTimeMs)
{
    int isNeedWait=1;
    int tLeft;
    float strobeTm;
    float strobeTm2;
    int waitTime;
    int waitTime2;
    g_pStrobe->getCoolTM(mCapRep.capDuty, &strobeTm);
    waitTime =  mCapRep.capStrobeEndTime+(mCapRep.capStrobeEndTime-mCapRep.capStrobeStartTime)*strobeTm - (curMs+300);
    if(cust_isDualFlashSupport(m_sensorDev)==1)
    {
        g_pStrobe2->getCoolTM(mCapRep.capDutyLT, &strobeTm2);
        waitTime2 =  mCapRep.capStrobeEndTime+(mCapRep.capStrobeEndTime-mCapRep.capStrobeStartTime)*strobeTm2 - (curMs+300);
        if(waitTime2>waitTime)
            waitTime=waitTime2;
    }
    if(waitTime>5000)
        waitTime=5000;
    else if(waitTime<0)
    {
        isNeedWait = 0;
        waitTime = 0;
    }
    *a_waitTimeMs = waitTime;
    return isNeedWait;
}

int FlashMgrM::cctSetSpModeNormal()
{
    if(g_uiSpModeLock==1)
        return 0;
    g_uiSpMode=e_SpModeNormal;
    return 0;
}

int FlashMgrM::cctSetSpModeQuickCalibration()
{
    if(g_uiSpModeLock==1)
        return 0;
    g_uiSpMode=e_SpModeQuickCalibration;
    return 0;
}
int FlashMgrM::cctSetSpModeQuickCalibration2()
{
    if(g_uiSpModeLock==1)
        return 0;
    g_uiSpMode=e_SpModeQuickCalibration2;
    return 0;
}
int FlashMgrM::cctSetSpModeCalibration()
{
    if(g_uiSpModeLock==1)
        return 0;
    g_uiSpMode=e_SpModeCalibration;
    return 0;
}
int FlashMgrM::cctSetSpModeLock(int bLock)
{
    g_uiSpModeLock=bLock;
    return 0;
}

int FlashMgrM::getFlashSpMode()
{
    return g_spMode;
}
int FlashMgrM::doPfOneFrame(FlashExePara* para, FlashExeRep* rep)
{
    rep->isFlashCalibrationMode=0;
    if(m_pfFrameCount==0)
    {
        if(g_uiSpMode!=e_SpModeNormal)
            g_spMode=g_uiSpMode;
        else
        {
            int spMode;
            int ratioEn;
            int ratioQEn;
            int ratioQ2En;
            ratioEn = FlashUtil::getPropInt("vendor.flash_ratio",0);
            ratioQEn = FlashUtil::getPropInt("vendor.flash_ratio_quick",0);
            ratioQ2En = FlashUtil::getPropInt("vendor.flash_ratio_quick2",0);
            if(ratioEn==1)
            {
                g_spMode=e_SpModeCalibration;
            }
            else if(ratioQEn==1)
            {
                g_spMode=e_SpModeQuickCalibration;
            }
            else if(ratioQ2En==1)
            {
                g_spMode=e_SpModeQuickCalibration2;
            }
            else
                g_spMode=e_SpModeNormal;
        }
    }
    int err=0;
    if(g_spMode==e_SpModeNormal)
    {
        logV("doPfOneFrame normal");
        err = doPfOneFrameNormal(para, rep);
    }
    else if(g_spMode==e_SpModeUserExp)
    {
        logV("doPfOneFrame userExp");
        rep->isEnd=1;
    }
    else if(g_spMode==e_SpModeCalibration)
    {
        logV("doPfOneFrame calibration frm=%d",m_pfFrameCount);
        rep->isFlashCalibrationMode=1;
        err = cctCalibration(para, rep);
    }
    else if(g_spMode==e_SpModeQuickCalibration2 || g_spMode==e_SpModeQuickCalibration)
    {
        logV("doPfOneFrame quick calibration2 frm=%d",m_pfFrameCount);
        rep->isFlashCalibrationMode=1;
        err = cctCalibrationQuick2(para, rep);
    }
    m_pfFrameCount++;
    return err;
}

int FlashMgrM::doPfOneFrameNormal(FlashExePara* in, FlashExeRep* rep)
{
    int curT = FlashUtil::getMs();
    logV("doPfOneFrame + normal input: w,h %d %d curTime=%d", in->staX, in->staY, curT);

#define FLASH_STATE_START 0
#define FLASH_STATE_COOLING 1
#define FLASH_STATE_RUN 2
#define FLASH_STATE_END 3

#define FLASH_STATUS_OFF 0
#define FLASH_STATUS_ON 1

    rep->isEnd=0;
    rep->isCal=0;
    if(m_pfFrameCount==0)
    {
        mFlashState=FLASH_STATE_START;
        mFlashPreSt=FLASH_STATE_START;
    }
    logV("doPfOneFrame frame=%d mFlashState=%d preExeFrame=%d pfRunFrames=%d",m_pfFrameCount,mFlashState,mPreRunFrm,mRunCycleFrame);
    if(mFlashState==FLASH_STATE_START)
    {
        if(g_isManualFlashEn==1)
            setFlashMode(LIB3A_FLASH_MODE_FORCE_ON);
        mCapRep.pfStartTime = curT;
        logV("doPfOneFrame state=start ms=%d", mCapRep.pfStartTime);
        MINT32 i4SutterDelay, i4SensorGainDelay, i4IspGainDelay, i4CaptureDelayFrame;
        AAASensorMgr::getInstance().getSensorSyncinfo(m_sensorDev, &i4SutterDelay, &i4SensorGainDelay, &i4IspGainDelay, &i4CaptureDelayFrame);
        int maxDelay;
        maxDelay = i4SutterDelay;
        if(maxDelay<i4SensorGainDelay)
            maxDelay=i4SensorGainDelay;
        if(maxDelay<i4IspGainDelay)
            maxDelay=i4IspGainDelay;
        mRunCycleFrame = maxDelay+1; //if sensor delay frame > 2, the value should be change. ( = delay frame +1 )
        if(mRunCycleFrame<3)
            mRunCycleFrame=3;



        start(in);
        if(m_bRunPreFlash==0)
        {
            rep->isEnd=1;
            mFlashState=FLASH_STATE_END;
        }
        else
        {
            mFlashState=FLASH_STATE_COOLING;
        }
        mPreRunFrm=m_pfFrameCount;
        mFlashPreSt=FLASH_STATE_START;
        mFlashHwState = FLASH_STATUS_OFF;
    }
    else if(mFlashState==FLASH_STATE_COOLING)
    {

        logV("doPfOneFrame state=cooling");
        int waitMs;
        int currentMs;
        currentMs = FlashUtil::getMs();
        if(mFlashPreSt!=FLASH_STATE_COOLING)
        {
            if(isNeedWaitCooling(currentMs, &waitMs)==1)
            {
                logV("cooling cur=%d wait:%d",currentMs, waitMs);
                mCoolingStartTime = currentMs;
                mCoolingWaitTime = waitMs;
                mFlashState=FLASH_STATE_COOLING;
            }
            else
            {
                mFlashState=FLASH_STATE_RUN;
            }
        }
        else
        {
            if(currentMs-mCoolingStartTime>mCoolingWaitTime)
                mFlashState=FLASH_STATE_RUN;
        }
        mFlashHwState = FLASH_STATUS_OFF;
        mFlashPreSt=FLASH_STATE_COOLING;
    }
    if(mFlashState==FLASH_STATE_RUN)
    {
        logV("doPfOneFrame state=run");
        if(mFlashPreSt!=FLASH_STATE_RUN)
        {
            mCapRep.pfRunTime=FlashUtil::getMs();
            mCapRep.pfRunStartFrm = m_pfFrameCount;
        }
        if(m_pfFrameCount-mPreRunFrm>=mRunCycleFrame)
        {
            run(in, rep);
            if(rep->nextIsFlash==1)
                mFlashHwState = FLASH_STATUS_ON;
            else
                mFlashHwState = FLASH_STATUS_OFF;
            mFlashState=FLASH_STATE_RUN;
            mPreRunFrm=m_pfFrameCount;
            if(rep->isEnd==1)
                mFlashState=FLASH_STATE_END;
        }
        mFlashPreSt=FLASH_STATE_RUN;
    }
    if(mFlashState==FLASH_STATE_END)
    {
        mCapRep.pfEndTime = FlashUtil::getMs();
        mCapRep.pfRunFrms =   m_pfFrameCount -  mCapRep.pfRunStartFrm + 1;
        mFlashPreSt=FLASH_STATE_END;

    }

    if(mFlashHwState == FLASH_STATUS_ON)
        rep->isCurFlashOn=1;
    else
        rep->isCurFlashOn=0;

    int pfBmpEn;
    pfBmpEn  = FlashUtil::getPropInt(PROP_PF_BMP_EN_STR,0);
    if(pfBmpEn==1)
    {
        char aeF[256];
        char awbF[256];
        sprintf(aeF, "/sdcard/flashdata/bmp/pf_ae_%03d_%02d.bmp",mDebugSceneCnt,m_pfFrameCount);
        sprintf(awbF, "/sdcard/flashdata/bmp/pf_awb_%03d_%02d.bmp",mDebugSceneCnt,m_pfFrameCount);
        convert3aSta_romeHw_bmp((void*)in->staBuf, in->staX, in->staY,  aeF, awbF);
    }
    logV("doPfOneFrame -: isEnd=%d",rep->isEnd);
    return 0;
}

int FlashMgrM::endPrecapture()
{
    int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getFlashModeStyle(m_sensorDev, flashMode);
    logV("endPrecapture %d %d",flashMode,flashStyle);
    if(flashStyle==(int)e_FLASH_STYLE_ON_ON || flashStyle==(int)e_FLASH_STYLE_ON_TORCH)
    {
        turnOnTorch();
    }
    else
    {
        turnOffFlashDevice();
    }
    mIsNeedFiredFlashChecked = 0;
    //m_bRunPreFlash=0;
    m_pfFrameCount=0;


    if(m_flashOnPrecapture==1)
    {
        if(cust_isDualFlashSupport(m_sensorDev)==1)
        {
            g_pStrobe2->setPreOn();
        }
        g_pStrobe->setPreOn();
    }
    FLASH_INFO_T finfo;
    finfo.flashMode = FLASHLIGHT_FORCE_OFF;
    finfo.isFlash = 0; // off
    IspTuningMgr::getInstance().setFlashInfo(m_sensorDev, finfo);

    //int err2;
    //err2 = NvBufUtil::getInstance().write(CAMERA_DATA_AE_PLINETABLE, 1);
    return 0;

}

int FlashMgrM::hasFlashHw(int& hasHw)
{
    if(g_pStrobe->hasFlashHw()==0)
        hasHw=0;
    else
        hasHw=1;
    return 0;
}
int FlashMgrM::isNeedFiringFlash()
{
    logV("isNeedFiringFlash()");
    mIsNeedFiredFlashChecked = 1;
    int fmode;
    int fstyle;
    fmode = getFlashMode();
    fstyle = getFlashModeStyle(m_sensorDev, fmode);

    int bFlashOn;
    int bRunPreFlash;
    int ispFlashMode;
    if(g_pStrobe->hasFlashHw()==0)
    {
        logV("isNeedFiringFlash() No flash hw");
        bFlashOn=0;
        bRunPreFlash=0;
        ispFlashMode=FLASHLIGHT_FORCE_OFF;
    }
    else if(fstyle==e_FLASH_STYLE_OFF_OFF)
    {
        logV("isNeedFiringFlash() XX");
        bFlashOn=0;
        bRunPreFlash=0;
        ispFlashMode=FLASHLIGHT_FORCE_OFF;
    }
    else if(fstyle==e_FLASH_STYLE_OFF_ON || fstyle==e_FLASH_STYLE_ON_ON)
    {
        logV("isNeedFiringFlash() XO OO");
        int isLow;
        g_pStrobe->isLowPower(&isLow);
        if(isLow==1)
        {
            logV("isLow=1");
            bFlashOn=0;
            bRunPreFlash=0;
            ispFlashMode=FLASHLIGHT_FORCE_OFF;
        }
        else
        {
            bFlashOn=1;
            bRunPreFlash=1;
            ispFlashMode=FLASHLIGHT_FORCE_ON;
        }
    }
    else if(fstyle==e_FLASH_STYLE_ON_TORCH)
    {
        logV("isNeedFiringFlash() OT");
        int isLow;
        g_pStrobe->isLowPower(&isLow);
        if(isLow==1)
        {
            logV("isLow=1");
            bFlashOn=0;
            bRunPreFlash=0;
            ispFlashMode=FLASHLIGHT_FORCE_OFF;
        }
        else
        {
            bFlashOn=1;
            bRunPreFlash=0;
            ispFlashMode=FLASHLIGHT_FORCE_ON;
        }
    }
    else //if(fstyle==e_FLASH_STYLE_OFF_AUTO)
    {
        if(AeMgr::getInstance(m_sensorDev).IsStrobeBVTrigger()==1)
        {
            int isLow;
            g_pStrobe->isLowPower(&isLow);
            if(isLow==1)
            {
                logV("isLow=1");
                bFlashOn=0;
                bRunPreFlash=0;
                ispFlashMode=FLASHLIGHT_FORCE_OFF;
            }
            else
            {
                ispFlashMode = FLASHLIGHT_AUTO;
                bFlashOn=1;
                bRunPreFlash=1;
                logV("isNeedFiringFlash() XA triger");
            }
        }
        else
        {
            int isLow;
            g_pStrobe->isLowPower(&isLow);
            if(isLow==1)
            {
                logV("isLow=1");
                bFlashOn=0;
                bRunPreFlash=0;
                ispFlashMode=FLASHLIGHT_FORCE_OFF;
            }
            else
            {
                ispFlashMode = FLASHLIGHT_AUTO;
                bFlashOn=0;
                bRunPreFlash=0;
                logV("isNeedFiringFlash() XA NOT triger");
            }
        }
    }
    m_bRunPreFlash = bRunPreFlash;

    FLASH_INFO_T finfo;
    finfo.flashMode = ispFlashMode;
    finfo.isFlash = bFlashOn;
    IspTuningMgr::getInstance().setFlashInfo(m_sensorDev, finfo);

    logV("isFlash=%d bRunPf=%d",bFlashOn, m_bRunPreFlash);
    if(bFlashOn==0)
    {
        m_flashOnPrecapture=0;
        return 0;
    }
    else
    {
        m_flashOnPrecapture=1;
    }
    return 1;

}
void FlashMgrM::debugCnt()
{
    logV("debugCnt %d",__LINE__);
    int binEn;
    int pfBmpEn;
    int mfBmpEn;
    binEn = FlashUtil::getPropInt(PROP_BIN_EN_STR,0);
    pfBmpEn  = FlashUtil::getPropInt(PROP_PF_BMP_EN_STR,0);
    mfBmpEn  = FlashUtil::getPropInt(PROP_MF_BMP_EN_STR,0);
    logV("debugCnt binEn, pfBmpEn, mfBmpEn %d %d %d", binEn, pfBmpEn, mfBmpEn);
    if(binEn==1 || pfBmpEn==1 || mfBmpEn==1)
    {
        FlashUtil::getFileCount("/sdcard/flash_file_cnt.txt", &mDebugSceneCnt, 0);
        FlashUtil::setFileCount("/sdcard/flash_file_cnt.txt", mDebugSceneCnt+1);
    }
    if(pfBmpEn==1 || mfBmpEn==1)
    {
        logV("debugCnt %d",__LINE__);
        FlashUtil::createDir("/sdcard/flashdata/");
        FlashUtil::createDir("/sdcard/flashdata/bmp/");
    }
    if(binEn==1)
    {
        logV("binEn = %d",binEn);
        mpFlashAlg->setIsSaveSimBinFile(1);
        char prjName[50];
        sprintf(prjName,"%03d",mDebugSceneCnt);
        mpFlashAlg->setDebugDir("/sdcard/flashdata/",prjName);
    }
}
void FlashMgrM::logProjectPara(FLASH_PROJECT_PARA* pp)
{
    FLASH_TUNING_PARA *pt;
    FLASH_COOL_TIMEOUT_PARA *pct;

    logV("projectPara ds %d",pp->dutyNum);

    pt = &pp->tuningPara;
    logV("prjP tp %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
            pt->yTarget,  // 188 (10bit)
            pt->fgWIncreaseLevelbySize, // 10
            pt->fgWIncreaseLevelbyRef,  // 0
            pt->ambientRefAccuracyRatio,  // 5  5/256=2%
            pt->flashRefAccuracyRatio,  // 1   1/256=0.4%
            pt->backlightAccuracyRatio, // 18 18/256=7%
            pt->safetyExp, // 33322
            pt->maxUsableISO,  // 680
            pt->yTargetWeight,  // 0 base:256
            pt->lowReflectanceThreshold,  // 13  13/256=5%
            pt->flashReflectanceWeight,  // 0 base:256
            pt->bgSuppressMaxDecreaseEV,  // 2EV
            pt->bgSuppressMaxOverExpRatio, // 6  6/256=2%
            pt->fgEnhanceMaxIncreaseEV, // 5EV
            pt->fgEnhanceMaxOverExpRatio, // 6  10/256=2%
            pt->isFollowCapPline,  // 0 for auto mode, 1 for others
            pt->histStretchMaxFgYTarget, // 266 (10bit)
            pt->histStretchBrightestYTarget // 328 (10bit)
        );

    pct = &pp->coolTimeOutPara;
    logV("prjP ct %d", pct->tabNum);
    logV("prjP ct_id %d %d %d %d %d %d %d %d %d %d",
            pct->tabId[0],    pct->tabId[1],    pct->tabId[2],    pct->tabId[3],    pct->tabId[4],
            pct->tabId[5],    pct->tabId[6],    pct->tabId[7],    pct->tabId[8],    pct->tabId[9]);

    logV("prjP ct_clx100 %d %d %d %d %d %d %d %d %d %d",
            (int)(pct->coolingTM[0]*100),(int)(pct->coolingTM[1]*100),(int)(pct->coolingTM[2]*100),(int)(pct->coolingTM[3]*100),(int)(pct->coolingTM[4]*100),
            (int)(pct->coolingTM[5]*100),(int)(pct->coolingTM[6]*100),(int)(pct->coolingTM[7]*100),(int)(pct->coolingTM[8]*100),(int)(pct->coolingTM[9]*100));

    logV("prjP ct_to %d %d %d %d %d %d %d %d %d %d",
            pct->timOutMs[0],    pct->timOutMs[1],    pct->timOutMs[2],    pct->timOutMs[3],    pct->timOutMs[4],
            pct->timOutMs[5],    pct->timOutMs[6],    pct->timOutMs[7],    pct->timOutMs[8],    pct->timOutMs[9]);

    logV("prjP oth %d", pp->maxCapExpTimeUs);

}
void FlashMgrM::genCapInfo(FlashExePara* in)
{
    int err;
    mCapInfo.digRatio = m_digRatio;
    mCapInfo.aeScene = AeMgr::getInstance(m_sensorDev).getAEMode();
    mCapInfo.isFireFlashChecked = mIsNeedFiredFlashChecked;
    int vbat;
    err = g_pStrobe->getVBat(&vbat);
    mCapInfo.vBat = vbat;
    mCapInfo.flickerMode = in->flickerMode;
    mCapInfo.evComp = m_evComp;//in->evComp;
    mCapInfo.flash1Mode = mFlash1Mode;
    mCapInfo.isBurst =  mIsMultiCapture;

}

int FlashMgrM::start(FlashExePara* in)
{

    debugCnt();
    genCapInfo(in);
    FLASH_INFO_T finfo;
    finfo.flashMode = FLASHLIGHT_FORCE_OFF;
    finfo.isFlash = 1; // pre-flash
    IspTuningMgr::getInstance().setFlashInfo(m_sensorDev, finfo);
    if(mCapInfo.isFireFlashChecked==1)
    {
        logV("isFireFlashChecked=1");
    }
    else
    {
        isNeedFiringFlash();
    }
    AeMgr::getInstance(m_sensorDev).setStrobeMode(m_flashOnPrecapture);
    if(m_bRunPreFlash==0)
        return 0;

    FLASH_PROJECT_PARA prjPara;
    if(mCapInfo.flash1Mode == LIB3A_FLASH_MODE_FORCE_ON)
        prjPara = getFlashProjectPara(mCapInfo.aeScene, 1);
    else
        prjPara = getFlashProjectPara(mCapInfo.aeScene,0);

    logProjectPara(&prjPara);

    //log eng
    int i = 0;
    int j = 0;
    logV("logEng dutyN=%d", prjPara.dutyNum);
    if(prjPara.dutyNum<1 || prjPara.dutyNum>39)
    {
        logE("!dutyN is wrong! valid:[1,39]");
    }
    else
    {
        logV("Test dutyN is ok");
    }

    char* chTemp;
    char ch[50];
    chTemp = new char[1024];
    int ii=0;

    chTemp[0]=0;
    sprintf(chTemp, "[%d] ",j);
    for(i=0; i<prjPara.dutyNum; i++)
    {
        sprintf(ch,"%5.2f ",(float)mpStrobeNvbuf->engTab.yTab[i]);
        strncat(chTemp, ch, sizeof(chTemp) - strlen(chTemp) - 1);
    }
    logV("%s",chTemp);

    //eng test
    int bEngErr=0;
    if(cust_isDualFlashSupport(m_sensorDev)==1)
    {
        int ind;
        int ind2;
        for(j=-1; j<prjPara.dutyNumLT; j++)
            for(i=0; i<prjPara.dutyNum; i++)
            {
                ind=(j+1)*(prjPara.dutyNum+1)+i+1;
                ind2=ind-1;
                if( mpStrobeNvbuf->engTab.yTab[ind]!=-1 &&
                        mpStrobeNvbuf->engTab.yTab[ind2]!=-1 &&
                        mpStrobeNvbuf->engTab.yTab[ind]<mpStrobeNvbuf->engTab.yTab[ind2])
                {
                    bEngErr=1;
                    logE("ind=%d, eng=%d ind2=%d eng=%d",ind,mpStrobeNvbuf->engTab.yTab[ind], ind2, mpStrobeNvbuf->engTab.yTab[ind2]);
                }
            }
        for(i=-1; i<prjPara.dutyNum; i++)
            for(j=0; j<prjPara.dutyNumLT; j++)
            {
                ind=(j+1)*(prjPara.dutyNum+1)+i+1;
                ind2=ind-(prjPara.dutyNum+1);
                if( mpStrobeNvbuf->engTab.yTab[ind]!=-1 &&
                        mpStrobeNvbuf->engTab.yTab[ind2]!=-1 &&
                        mpStrobeNvbuf->engTab.yTab[ind]<mpStrobeNvbuf->engTab.yTab[ind2])
                {
                    bEngErr=1;
                    logE("ind=%d, eng=%d ind2=%d eng=%d",ind,mpStrobeNvbuf->engTab.yTab[ind], ind2, mpStrobeNvbuf->engTab.yTab[ind2]);
                }
            }
    }
    else
    {
        for(i=1; i<prjPara.dutyNum; i++)
        {
            if(mpStrobeNvbuf->engTab.yTab[i]<mpStrobeNvbuf->engTab.yTab[i-1])
            {
                bEngErr=1;
                break;
            }
        }
    }

    if(bEngErr==1)
        logE("EngTab is wrong! value are not inc!");
    else
        logV("Test EngTab is ok (inc)");
    delete []chTemp;

    m_iteration=0;
    mpFlashAlg->Reset();
    if(isFlashAWBv2Enabled())
    {
        MY_LOG("[Flash Awb] Prepare to do flash awb init");

        FLASH_AWB_INIT_T FlashAwbInitParam;
        NVRAM_CAMERA_3A_STRUCT* buf;
        int err;
        err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_sensorDev, (void*&)buf);
        if(err!=0)
            logE("nvbuf get CAMERA_NVRAM_DATA_3A");

        logV("flash_awb_para for:%d bk: %d", (int)buf->rFlashAWBNVRAM.rTuningParam.ForeGroundPercentage, (int)buf->rFlashAWBNVRAM.rTuningParam.BackGroundPercentage);

        logV("flash_awb_para th1,2,3,4:%d %d %d %d",
                buf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th1,
                buf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th2,
                buf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th3,
                buf->rFlashAWBNVRAM.rTuningParam.FgPercentage_Th4);

        FlashAwbInitParam.flash_awb_tuning_param = buf->rFlashAWBNVRAM.rTuningParam;
        mpFlashAlg->Flash_Awb_Init(FlashAwbInitParam);
    }

    FlashAlgInputInfoStruct algIn;
    algIn.NonPreferencePrvAWBGain = in->NonPreferencePrvAWBGain;
    algIn.PreferencePrvAWBGain = in->PreferencePrvAWBGain;
    algIn.i4AWBCCT = in->i4AWBCCT;

    int err;
    NVRAM_CAMERA_3A_STRUCT* buf;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, m_sensorDev, (void*&)buf);
    if(err!=0)
        logE("err=%d",err);


    AWB_PARAM_T awbParam;
    switch (m_sensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        awbParam = getAWBParam<ESensorDev_Main>();
        break;
    case ESensorDev_MainSecond: //  Main Second Sensor
        awbParam = getAWBParam<ESensorDev_MainSecond>();
        break;
    case ESensorDev_Sub: //  Sub Sensor
        awbParam = getAWBParam<ESensorDev_Sub>();
        break;
    default:
        MY_ERR("m_eSensorDev = %d", m_sensorDev);
        return MFALSE;
    }
    algIn.AwbRotationMatrix.i4Cos = buf->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rRotationMatrix.i4Cos;
    algIn.AwbRotationMatrix.i4Sin = buf->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rRotationMatrix.i4Sin;
    algIn.i4RotationMatrixUnit = awbParam.rChipParam.i4RotationMatrixUnit;
    mpFlashAlg->setFlashInfo(&algIn);

    //pf pline
    hw_setPfPline(mpFlashAlg);
    //cap pline
    hw_setCapPline(&prjPara, mpFlashAlg);
    //flash profile
    hw_setFlashProfile(mpFlashAlg, &prjPara, mpStrobeNvbuf);
    //preference
    hw_setPreference(mpFlashAlg, &prjPara);
    //
    err = mpFlashAlg->checkInputParaError(0,0);
    if(err!=0)
    {
        addErr(err);
        logE("checkInputParaError err=%d", err);
        m_flashOnPrecapture=0;
        return 0;
    }
    FlashAlgExpPara aePara;

    hw_getAEExpPara(&aePara);
    mpFlashAlg->CalFirstEquAEPara(&aePara, &g_expPara);
    //set exp
    hw_setExpPara(&g_expPara, m_sensorDev, &prjPara);
    return 0;
}
//==============================
int FlashMgrM::run(FlashExePara* para, FlashExeRep* rep)
{
    logV("run() line=%d",__LINE__);
    FLASH_PROJECT_PARA prjPara;
    if(mCapInfo.flash1Mode == LIB3A_FLASH_MODE_FORCE_ON)
        prjPara = getFlashProjectPara(mCapInfo.aeScene, 1);
    else
        prjPara = getFlashProjectPara(mCapInfo.aeScene, 0);
    FlashAlgStaData staData;
    short g_data2[40*30*2];
    staData.data = g_data2;
    MUINT32 u4FlashResultWeight = 0;
    //convert flash3A
    hw_convert3ASta(&staData, para->staBuf, para->staX, para->staY);
    logV("alg data row, col = %d %d", staData.row, staData.col);

    int isNext;
    FlashAlgExpPara paraNext;
    mpFlashAlg->AddStaData10(&staData, &g_expPara, &isNext, &paraNext);

    g_expPara = paraNext;

    {
        int afe;
        int isp;
        hw_capIsoToGain(g_expPara.iso, &afe, &isp);

        rep->nextAfeGain = afe;
        rep->nextIspGain = isp;
        rep->nextExpTime = g_expPara.exp;
        rep->nextIsFlash = g_expPara.isFlash;
        rep->nextDuty = g_expPara.duty;
        //rep->flashAwbWeight = u4FlashResultWeight;
        rep->nextIso = g_expPara.iso;
        rep->isCal = 1;
    }

    m_iteration++;
    if(m_iteration>10 || isNext==0)
    {
        rep->isEnd=1;
        mCapRep.pfIteration = m_iteration;
        logV("Estimate+");
        mpFlashAlg->Estimate(&g_expPara);
        logV("Estimate- exp %d, duty %d",g_expPara.exp, g_expPara.duty);
        if(isFlashAWBv2Enabled())
        {
            mpFlashAlg->Flash_Awb_Algo(&u4FlashResultWeight);
        }
        int afe;
        int isp;
        hw_capIsoToGain(g_expPara.iso, &afe, &isp);

        rep->nextAfeGain = afe;
        rep->nextIspGain = isp;
        rep->nextExpTime = g_expPara.exp;
        rep->nextIsFlash = g_expPara.isFlash;
        rep->nextDuty = g_expPara.duty;
        rep->flashAwbWeight = u4FlashResultWeight;
        rep->nextIso = g_expPara.iso;
        rep->isCal = 1;
        hw_setCapExpPara(&g_expPara);
        mCapRep.capIso = g_expPara.iso;
        hw_capIsoToGain(mCapRep.capIso, &mCapRep.capAfeGain, &mCapRep.capIspGain);
        mCapRep.capExp = g_expPara.exp;
        mCapRep.capDuty = g_expPara.duty;
    }
    else
    {
        rep->isEnd=0;
        rep->nextIsFlash = g_expPara.isFlash;
        g_expPara.duty = mpStrobeNvbuf->engLevelLT.pfDuty;  // dual-flash
        hw_setExpPara(&g_expPara, m_sensorDev, &prjPara);
    }
    logV("run() line=%d isEnd=%d",__LINE__,rep->isEnd);
    return 0;
}
int FlashMgrM::setEvComp(int ind, float ev_step)
{
    if (m_evComp != ind*ev_step)
        logV("setEvComp ind=%d evs=%f origin=%f", ind, ev_step, m_evComp);
    m_evComp = ind*ev_step;
    return 0;
}

int FlashMgrM::getFlashMode()
{
    return mFlash1Mode;
}
int FlashMgrM::setCamMode(int mode)
{
    if (mCamMode != mode)
        logV("setCamMode mode=%d old_mode=%d", mode, mCamMode);
    mCamMode = mode;
    return 0;
}
int FlashMgrM::setAeFlashMode(int aeMode, int flashMode)
{
    int fm=LIB3A_FLASH_MODE_AUTO;
    if(aeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH && flashMode == MTK_FLASH_MODE_SINGLE)
        fm=LIB3A_FLASH_MODE_AUTO;
    else if(aeMode == MTK_CONTROL_AE_MODE_ON && flashMode == MTK_FLASH_MODE_OFF)
        fm=LIB3A_FLASH_MODE_FORCE_OFF;
    else if(aeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH && flashMode == MTK_FLASH_MODE_SINGLE)
        fm=LIB3A_FLASH_MODE_FORCE_ON;
    else if(aeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH_REDEYE && flashMode == MTK_FLASH_MODE_SINGLE)
        fm=LIB3A_FLASH_MODE_AUTO;
    else if(aeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH && flashMode == MTK_FLASH_MODE_TORCH)
        fm=LIB3A_FLASH_MODE_FORCE_TORCH;

    if (mFlash1Mode != fm)
        logV("setAeFlashMode ae_mode=%d flash_mode=%d f1_mode=%d old=%d", aeMode, flashMode, fm, mFlash1Mode);

    mFlash1Mode = fm;
    setFlashMode(fm);
    return 0;
}

int FlashMgrM::setFlashMode(int mode)
{
    logV("setFlashMode+ mode=%d",mode);
    if(mode<LIB3A_FLASH_MODE_MIN || mode>LIB3A_FLASH_MODE_MAX)
    {
        logV("setFlashMode- ln=%d min=%d, max=%d",__LINE__,LIB3A_FLASH_MODE_MIN,LIB3A_FLASH_MODE_MAX);
        return FL_ERR_FlashModeNotSupport;
    }
    if(m_isAfState==1)
    {
        mFlash1Mode = mode;
        logV("setFlashMode- ln=%d afState", __LINE__);
        return 0;
    }
    int fmode;
    int fstyle;
    fmode = mode;
    if(g_previewMode==e_Capture)
    {
        logV("setFlashMode- ln=%d capturing", __LINE__);
        return 0;
    }
    else if(g_previewMode==e_NonePreview)
    {
        logV("nonepreview");
        initTemp();
        if(mode==LIB3A_FLASH_MODE_FORCE_TORCH)
        {
            logV("torch");
            turnOnTorch();
        }
        else
        {
            logV("off");
            turnOffFlashDevice();
        }
    }
    else if(g_previewMode==e_CapturePreview)
    {
        fstyle = getFlashModeStyle(m_sensorDev, fmode);
        if(fstyle==e_FLASH_STYLE_ON_ON || fstyle==e_FLASH_STYLE_ON_TORCH)
        {
            turnOnTorch();
        }
        else
        {
            turnOffFlashDevice();
        }
    }
    else if(g_previewMode==e_VideoPreview)
    {
        logV("videoPrv mode=%d",fmode);
        if(fmode==LIB3A_FLASH_MODE_FORCE_ON || fmode==LIB3A_FLASH_MODE_FORCE_TORCH)
            turnOnTorch();
        else
            turnOffFlashDevice();
    }
    else if(g_previewMode==e_VideoRecording)
    {
        logV("videoRec mode=%d",fmode);

        if(fmode==LIB3A_FLASH_MODE_FORCE_ON || fmode==LIB3A_FLASH_MODE_FORCE_TORCH)
            turnOnTorch();
        else if(CUST_ENABLE_VIDEO_AUTO_FLASH() && fmode==LIB3A_FLASH_MODE_AUTO)
        {
            /*
               if(AeMgr::getInstance(m_sensorDev).IsStrobeBVTrigger()==1)
               turnOnTorch();
               else
               turnOffFlashDevice();
             */
        }
        else
            turnOffFlashDevice();
    }
    else
    {
        logV("preview mode is wrong");
    }
    mFlash1Mode = mode;
    return 0;
}
//==============================
int FlashMgrM::isFlashOnCapture()
{
    logV("isFlashOnCapture() line=%d isFlashOnCapture=%d",__LINE__,m_flashOnPrecapture);
    return m_flashOnPrecapture;
}
/*
   static void test()
   {
//test for nvram read/write
int sz;
sz = sizeof(NVRAM_CAMERA_SHADING_STRUCT);
logI("qq shading size=%d", sz);

int err;
NVRAM_CAMERA_SHADING_STRUCT* pNv;
void* buf;


err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_3A, 1, (void*&)buf, 1);
err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, 1);

err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_SHADING, 1, (void*&)buf, 1);
err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_SHADING, 1);

err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, 1, (void*&)buf, 1);
err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_LENS, 1);


err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_STROBE, 1, (void*&)buf, 1);
err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_STROBE, 1);

err = NvBufUtil::getInstance().getBufAndRead(CAMERA_DATA_TSF_TABLE, 1, (void*&)buf, 1);
err = NvBufUtil::getInstance().write(CAMERA_DATA_TSF_TABLE, 1);

err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_GEOMETRY, 1, (void*&)buf, 1);
err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_GEOMETRY, 1);


err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, 1, (void*&)buf, 1);
err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_FEATURE, 1);





//    FILE* fp;
//    fp = fopen("sdcard/nvs1.bin","wb");
//    fwrite(pNv,1,sz,fp);
//    fclose(fp);
//
//    //pNv->u4Version = 1001;
//
//    fp = fopen("sdcard/nvs2.bin","wb");
//    fwrite(pNv,1,sz,fp);
//    fclose(fp);
//    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_SHADING, 1);





}*/
int FlashMgrM::capCheckAndFireFlash_Start()
{
    // test();
    int curT = FlashUtil::getMs();
    mCapRep.capStartTime = curT;
    g_previewMode=e_Capture;
    int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getFlashModeStyle(m_sensorDev, flashMode);
    logV("capCheckAndFireFlash_Start2 %d %d",flashMode,flashStyle);
    if(flashStyle==(int)e_FLASH_STYLE_OFF_OFF)
    {
        turnOffFlashDevice();
        return 0;
    }
    else if(flashStyle==(int)e_FLASH_STYLE_ON_TORCH)
    {
        turnOnTorch();
        return 0;
    }
    logV("capCheckAndFireFlash_Start line=%d  getMs=%d",__LINE__, FlashUtil::getMs());
    logV("cap mfexp %d %d",m_flashOnPrecapture, mCapRep.capDuty);
    if(m_flashOnPrecapture==1 && g_isManualFlashEn==0)
    {
        mCapRep.capStrobeStartTime = curT;
        int timeOutTime;
        int timeOutTimeLT;
        if(cust_isDualFlashSupport(m_sensorDev)==1)
        {
            int duty;
            int dutyLT;
            FLASH_PROJECT_PARA prjPara;
            prjPara = getAutoProjectPara();

            FlashIndConvPara conv;
            conv.ind = g_expPara.duty;
            conv.isDual=1;
            conv.dutyNum = prjPara.dutyNum;
            conv.dutyNumLt = prjPara.dutyNumLT;
            int err;
            err = ind2duty(conv);
            if(err!=0)
                logE("ind2duty error!!");
            duty = conv.duty;
            dutyLT = conv.dutyLt;
            //duty = g_expPara.duty % (prjPara.dutyNum+1);
            //duty-=1;
            //dutyLT= g_expPara.duty / (prjPara.dutyNum+1);
            //dutyLT-=1;
            logV("mfexp algDuty=%d, duty=%d, dutyLT=%d", g_expPara.duty, duty, dutyLT);
            mCapRep.capDuty = duty;
            mCapRep.capDutyLT = dutyLT;
            mCapRep.dutyNum = prjPara.dutyNum;
            mCapRep.dutyNumLT = prjPara.dutyNumLT;

            g_pStrobe->getTimeOutTime(duty, &timeOutTime);
            mCapRep.capSetTimeOutTime = timeOutTime;
            if(timeOutTime == ENUM_FLASH_TIME_NO_TIME_OUT)
                timeOutTime=0;
            g_pStrobe2->getTimeOutTime(dutyLT, &timeOutTimeLT);
            mCapRep.capSetTimeOutTimeLT = timeOutTimeLT;
            if(timeOutTimeLT == ENUM_FLASH_TIME_NO_TIME_OUT)
                timeOutTimeLT=0;
            {
                Mutex::Autolock lock(mLock);
                mStrobeState = e_StrobeMf;
                setFlashOn(duty, timeOutTime, dutyLT, timeOutTimeLT, 0);
            }


            if(mCapRep.isBatLow==0 && mIsMultiCapture==0)
            {
                if(duty>mpStrobeNvbuf->engLevel.mfDutyMaxL)
                    g_pStrobe->lowPowerDetectStart(mpStrobeNvbuf->engLevel.mfDutyMaxL);
                if(dutyLT>mpStrobeNvbuf->engLevelLT.mfDutyMaxL)
                    g_pStrobe2->lowPowerDetectStart(mpStrobeNvbuf->engLevelLT.mfDutyMaxL);
            }
        }
        else
        {
            int duty;
            FLASH_PROJECT_PARA prjPara;
            prjPara = getAutoProjectPara();
            g_pStrobe->getTimeOutTime(g_expPara.duty, &timeOutTime);
            mCapRep.capSetTimeOutTime = timeOutTime;

            FlashIndConvPara conv;
            conv.ind = g_expPara.duty;
            conv.isDual=0;
            conv.dutyNum = prjPara.dutyNum;
            int err;
            err = ind2duty(conv);
            duty = conv.duty;


            if(timeOutTime == ENUM_FLASH_TIME_NO_TIME_OUT)
                timeOutTime=0;
            {
                Mutex::Autolock lock(mLock);
                mStrobeState = e_StrobeMf;
                setFlashOn(duty, timeOutTime, -1, 0, 0);
            }
            mCapRep.capDuty = duty;
            mCapRep.capDutyLT = 0;
            mCapRep.dutyNum = prjPara.dutyNum;
            mCapRep.dutyNumLT = 0;


            if(mCapRep.isBatLow==0 && mIsMultiCapture==0)
            {
                if(g_expPara.duty>mpStrobeNvbuf->engLevel.mfDutyMaxL)
                    g_pStrobe->lowPowerDetectStart(mpStrobeNvbuf->engLevel.mfDutyMaxL);
            }
        }


        FLASH_INFO_T finfo;
        finfo.flashMode = FLASHLIGHT_FORCE_OFF;
        finfo.isFlash = 2; // main-flash
        IspTuningMgr::getInstance().setFlashInfo(m_sensorDev, finfo);



    }

    int mfOn;
    mfOn = FlashUtil::getPropInt(PROP_MF_ON_STR,-1);
    if((mfOn!=-1) || (g_isManualFlashEn==1))
    {
        if(mfOn==0)
        {
            setFlashOn(-1, 0, -1, 0, 0);
        }
        else
        {
            int d1;
            int d2;
            d1 = FlashUtil::getPropInt(PROP_MF_DUTY_STR,-1);
            d2 = FlashUtil::getPropInt(PROP_MF_DUTYLT_STR,-1);
            if(g_isManualFlashEn==1)
            {
                d1=g_manualDuty;
                d2=g_manualDutyLt;
            }
            setFlashOn(d1, 500, d2, 500, 0);
        }
    }
    return 0;
}

int FlashMgrM::capCheckAndFireFlash_End()
{
    logV("capCheckAndFireFlash_End line=%d  getMs=%d",__LINE__, FlashUtil::getMs());
    int curT = FlashUtil::getMs();
    mCapRep.capEndTime = curT;
    int flashMode;
    int flashStyle;
    flashMode = getFlashMode();
    flashStyle = getFlashModeStyle(m_sensorDev, flashMode);
    logV("capCheckAndFireFash_End %d %d",flashMode,flashStyle);

    mCapRep.capIsTimeOut=0;
    if(m_flashOnPrecapture==1)
        mCapRep.capStrobeEndTime = curT;
    if(flashStyle==(int)e_FLASH_STYLE_ON_TORCH)
    {
        turnOnTorch();
        mCapRep.capEndTime = 0;
    }
    else if(mIsMultiCapture==1)
    {

    }
    else  //if(flashStyle==(int)e_FLASH_STYLE_OFF_OFF)
    {
        g_pStrobe->lowPowerDetectEnd();
        if(cust_isDualFlashSupport(m_sensorDev)==1)
        {
            g_pStrobe2->lowPowerDetectEnd();
        }


        turnOffFlashDevice();
        int setOnOffTime;
        setOnOffTime = mCapRep.capEndTime - mCapRep.capStartTime;
        mCapRep.capIsTimeOut=0;
        if(cust_isDualFlashSupport(m_sensorDev)==1)
        {
            if((setOnOffTime >  mCapRep.capSetTimeOutTime && mCapRep.capSetTimeOutTime!=0)||
                    (setOnOffTime >  mCapRep.capSetTimeOutTimeLT && mCapRep.capSetTimeOutTimeLT!=0))
                mCapRep.capIsTimeOut=1;
        }
        else
        {
            if(setOnOffTime >  mCapRep.capSetTimeOutTime && mCapRep.capSetTimeOutTime!=0)
                mCapRep.capIsTimeOut=1;
        }
    }
    int mfOn;
    mfOn = FlashUtil::getPropInt(PROP_MF_ON_STR,-1);
    if(mfOn!=-1)
        turnOffFlashDevice();

    FLASH_INFO_T finfo;
    finfo.flashMode = FLASHLIGHT_FORCE_OFF;
    finfo.isFlash = 0; // off
    IspTuningMgr::getInstance().setFlashInfo(m_sensorDev, finfo);


    logV("cpatureEnd line=%d  strobeStart=%d strobeEnd=%d",__LINE__, mCapRep.capStartTime, mCapRep.capEndTime);
    return 0;
}

int FlashMgrM::getFlashStyle()
{
    return 0;
}

int FlashMgrM::videoPreviewStart()
{
    logV("videoPreviewStart+");
    int fstyle;
    fstyle = getFlashModeStyle(m_sensorDev, mFlash1Mode);
    logV("mode=%d fstyle=%d",mFlash1Mode, fstyle);
    if(fstyle==(int)e_FLASH_STYLE_ON_ON)
    {
        turnOnTorch();
        m_flashOnPrecapture=1;
    }
    else
    {
        logV("turn off flash");
        turnOffFlashDevice();
        m_flashOnPrecapture=0;
    }
    g_previewMode=e_VideoPreview;
    logV("videoPreviewStart-");
    return 0;
}

int FlashMgrM::videoRecordingStart()
{
    logV("videoRecordingStart+ m=%d",mFlash1Mode);
    if(mFlash1Mode==LIB3A_FLASH_MODE_FORCE_ON || mFlash1Mode==LIB3A_FLASH_MODE_FORCE_TORCH)
    {
        turnOnTorch();
        m_flashOnPrecapture=1;
    }
    else if(CUST_ENABLE_VIDEO_AUTO_FLASH() && mFlash1Mode==LIB3A_FLASH_MODE_AUTO)
    {
        if(AeMgr::getInstance(m_sensorDev).IsStrobeBVTrigger()==1)
        {
            turnOnTorch();
            m_flashOnPrecapture=1;
        }
        else
        {
            m_flashOnPrecapture=0;
            turnOffFlashDevice();
        }
    }
    else
    {
        m_flashOnPrecapture=0;
        turnOffFlashDevice();
    }
    g_previewMode=e_VideoRecording;
    logV("videoRecordingStart-");
    return 0;
}

int FlashMgrM::videoRecordingEnd()
{
    logV("videoRecordingEnd+");
    if(CUST_ENABLE_VIDEO_AUTO_FLASH() && mFlash1Mode==LIB3A_FLASH_MODE_AUTO)
    {
        turnOffFlashDevice();
    }
    g_previewMode=e_VideoPreview;
    logV("videoRecordingEnd-");
    return 0;
}

int FlashMgrM::cameraPreviewStart()
{
    logV("capturePreviewStart+");
    int fstyle;
    fstyle = getFlashModeStyle(m_sensorDev, mFlash1Mode);
    logV("mode=%d fstyle=%d",mFlash1Mode, fstyle);
    if(fstyle==(int)e_FLASH_STYLE_ON_ON || fstyle==(int)e_FLASH_STYLE_ON_TORCH)
    {
        turnOnTorch();
    }
    else
    {
        turnOffFlashDevice();
    }
    g_previewMode=e_CapturePreview;
    m_flashOnPrecapture=0;
    logV("capturePreviewStart-");
    return 0;
}

int FlashMgrM::cameraPreviewEnd()
{
    logV("capturePreviewEnd+");
    int fstyle;
    fstyle = getFlashModeStyle(m_sensorDev, mFlash1Mode);
    logV("mode=%d fstyle=%d",mFlash1Mode, fstyle);
    if(fstyle==(int)e_FLASH_STYLE_ON_ON || fstyle==(int)e_FLASH_STYLE_ON_TORCH)
    {
        turnOnTorch();
    }
    else
    {
        turnOffFlashDevice();
    }
    logV("capturePreviewEnd-");
    return 0;
}
int FlashMgrM::videoPreviewEnd()
{
    logV("videoPreviewEnd+");
    g_previewMode=e_NonePreview;
    turnOffFlashDevice();
    logV("videoPreviewEnd-");
    return 0;
}

int FlashMgrM::notifyAfEnter()
{
    logV("notifyAfEnter");
    m_isAfState=1;
    return 0;
}

int FlashMgrM::notifyAfExit()
{
    logV("notifyAfExit");
    m_isAfState=0;
    setFlashMode(mFlash1Mode);
    return 0;
}

int FlashMgrM::turnOffFlashDevice()
{
    hwSetFlashOff();
    return 0;
}
//==============================
int FlashMgrM::end()
{
    return 0;
}

int FlashMgrM::nvGetBuf(NVRAM_CAMERA_STROBE_STRUCT*& buf, int isForceRead)
{
    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_STROBE, m_sensorDev, (void*&)buf, isForceRead);

    logV("err=%d",err);
#if  0
    dispStrobeNvram(buf);
#endif
    return err;
}

int FlashMgrM::nvWrite()
{
    int err;
    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_STROBE, m_sensorDev);
    return err;
}

int FlashMgrM::nv3AWrite()
{
    int err;
    err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_3A, m_sensorDev);
    return err;
}




int FlashMgrM::nvReadDefault()
{
    logV("loadDefaultNvram()");
    int err;
    NVRAM_CAMERA_STROBE_STRUCT* pNv;
    err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_STROBE, m_sensorDev, (void*&)pNv);
    err = cust_fillDefaultStrobeNVRam(m_sensorDev, pNv);
#if  0
    dispStrobeNvram(buf);
#endif
    return err;
}

int FlashMgrM::egSetMfDuty(int /*duty*/)
{
    //@@remove
    return 0;
}
int FlashMgrM::egGetDutyRange(int* st, int* ed)
{
    FLASH_PROJECT_PARA prjPara;
    prjPara = getAutoProjectPara();
    *st = 0;
    *ed = prjPara.dutyNum-1;
    logV("egGetDutyRange %d %d\n", *st, *ed);
    return 0;
}
int FlashMgrM::setStrobeCallbackFunc(void (* pFunc)(MINT32 en))
{
    g_onOffCallBackFunc = pFunc;
    return 0;
}

int FlashMgrM::setCapPara()
{
    int fmode;
    int fstyle;
    fmode = getFlashMode();
    fstyle = getFlashModeStyle(m_sensorDev, fmode);
    if(fstyle!=e_FLASH_STYLE_ON_TORCH)
    {
        hw_setCapExpPara(&g_expPara);
    }
    return 0;
}

int FlashMgrM::setPfParaToAe()
{
    strAETable pfPlineTab;
    strAETable capPlineTab;
    strAETable strobePlineTab;
    strAFPlineInfo pfPlineInfo;
    AE_DEVICES_INFO_T devInfo;
    AeMgr::getInstance(m_sensorDev).getCurrentPlineTable(pfPlineTab, capPlineTab, strobePlineTab, pfPlineInfo);
    AeMgr::getInstance(m_sensorDev).getSensorDeviceInfo(devInfo);
    int exp;
    int afe;
    int isp;
    exp = g_pfExpPara.exp;
    afe = g_pfExpPara.afeGain;
    isp = g_pfExpPara.ispGain;
    int bestInd;
    int err;
    err = FlashPlineTool::searchAePlineIndex(&bestInd, &pfPlineTab, exp, afe, isp);
    if(err!=0)
    {
        logE("searchAePlineIndex err");
        return -1;
    }
    exp = pfPlineTab.pCurrentTable->sPlineTable[bestInd].u4Eposuretime;
    afe = pfPlineTab.pCurrentTable->sPlineTable[bestInd].u4AfeGain;
    isp = pfPlineTab.pCurrentTable->sPlineTable[bestInd].u4IspGain;
    AE_MODE_CFG_T previewInfo;
    AeMgr::getInstance(m_sensorDev).getPreviewParams(previewInfo);
    previewInfo.u4Eposuretime = exp;
    previewInfo.u4AfeGain = afe;
    previewInfo.u4IspGain = isp;
    AeMgr::getInstance(m_sensorDev).updatePreviewParams(previewInfo, bestInd);
    ALOGD("setPfParaToAe afe, isp %d %d %d, ind=%d", exp, afe, isp, bestInd);
    return 0;
}
void dispStrobeNvram(NVRAM_CAMERA_STROBE_STRUCT* nv)
{
    logV("-- dispNvram--");
    logV("engTab: exp:%d afe=%d isp=%d dist=%d", nv->engTab.exp, nv->engTab.afe_gain, nv->engTab.isp_gain, nv->engTab.distance);
    logV("engTab: y[0-9]: %d %d %d %d %d %d %d %d %d %d",
            nv->engTab.yTab[0], nv->engTab.yTab[1], nv->engTab.yTab[2], nv->engTab.yTab[3], nv->engTab.yTab[4],
            nv->engTab.yTab[5], nv->engTab.yTab[6], nv->engTab.yTab[7], nv->engTab.yTab[8], nv->engTab.yTab[9]);
    logV("tuning: ytar: %d",nv->tuningPara[0].yTarget);
    //logI("dual tuning: temp: %d",nv->dualTuningPara.temp);
}
int FlashMgrM::setMultiCapture(int bMulti)
{
    mIsMultiCapture=bMulti;
    return 0;
}

int FlashMgrM::ind2duty(FlashIndConvPara& para)
{
    if(para.ind>=1600 || para.ind<0)
        goto ERR_RET;
    if(para.isDual==1)
    {
        if(para.ind>= (para.dutyNum+1)*(para.dutyNumLt+1))
            goto ERR_RET;
        para.duty = para.ind%(para.dutyNum+1)-1;
        para.dutyLt = para.ind/(para.dutyNum+1)-1;
    }
    else
    {
        if(para.ind>=para.dutyNum)
            goto ERR_RET;
        para.duty = para.ind;
        para.dutyLt = -1;
    }
    return 0;
    //return with err
ERR_RET:
    return -1;
}


int FlashMgrM::duty2ind(FlashIndConvPara& para)
{
    if(para.isDual==1)
    {
        if(para.duty<-1 || para.duty>=para.dutyNum || para.dutyLt<-1 || para.dutyLt>=para.dutyNumLt)
            goto ERR_RET;
        para.ind = (para.dutyLt+1)*(para.dutyNum+1)+(para.duty+1);
        if(para.ind>=1600 || para.ind<0)
            goto ERR_RET;
    }
    else
    {
        if(para.duty<0 || para.duty>=para.dutyNum)
            goto ERR_RET;
        para.ind = para.duty;
        if(para.ind>=1600 || para.ind<0)
            goto ERR_RET;
    }
    return 0;
    //return with err
ERR_RET:
    return -1;

}
