#define LOG_TAG "flicker_hal.cpp"
#define MTK_LOG_ENABLE 1
#include <stdlib.h>
#include <stdio.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <tuning_mgr.h>
#include <camera_custom_cam_cal.h>
#include <mtkcam/drv/mem/cam_cal_drv.h>
#include <mtkcam/drv/IHalSensor.h>
#include <ctype.h>
#include <sys/stat.h>
#include <aaa_sensor_mgr.h>
#include <time.h>
#include <kd_camera_feature.h>
#include <isp_mgr.h>
#include <nvram_drv.h>
#include <vector>
#include <aaa_error_code.h>
#include "Flicker_type.h"
#include <utils/threads.h>
#include <cutils/log.h>
#include "awb_param.h"
#include "ae_param.h"
#include "af_param.h"
#include "flicker_param.h"
#include "sequential_testing.h"
#include "FlickerDetection.h"
#include "camera_custom_flicker.h"
#include "flicker_hal.h"
#include "flicker_util.h"
#include "camera_custom_flicker_para.h"
#include "ae_feature.h"
#include <string.h>
#include <cutils/log.h>
#include <isp_mgr_flk.h> //hw config
using namespace NS3Av3;
using namespace NSCamCustom;
using namespace NSIspTuningv3;
//=====================================================
// define
//=====================================================
#define FLIKCER_DEBUG 0
#define FLICKER_MAX_LENG 6000
#define MHAL_FLICKER_WORKING_BUF_SIZE (FLICKER_MAX_LENG*4*3)    // flicker support max size
#define FLICKER_SUPPORT_MAX_SIZE (FLICKER_MAX_LENG*2*3)

#define DEF_PROP_DEF_HZ "vendor.flk_def_hz"  //50, 60
#define DEF_PROP_VERBOSE_EN "vendor.flk_verbose_en"

//log
#define logI(fmt, arg...) ALOGD(fmt, ##arg)
#define logV(fmt, arg...) {if(logv_en) logI(fmt, ##arg);}
#define logE(fmt, arg...) ALOGD("MError: func=%s line=%d: " fmt, __FUNCTION__, __LINE__, ##arg)

//MACRO
#define FLK_READ(para) ret = fscanf(paraFile,"%d", &para_buf);\
    para = para_buf; logV("flk_para (%d)",para_buf);

//=====================================================
// variable
//=====================================================
static int g_frmCntPre=0;
FLICKER_EXT_PARA g_flickerExtPara;
static int g_maxDetExpUs=70000;
static int g_flickerHz=0;
static int logv_en=0;
//=====================================================
void debugSetting()
{
#if FLIKCER_DEBUG
    logv_en = 1;
#else
    logv_en = FlickerUtil::getPropInt(DEF_PROP_VERBOSE_EN,0);
#endif
}

FlickerHal* FlickerHal::getInstance(int sensorDev)
{
    static FlickerHal singletonMain(ESensorDev_Main);
    static FlickerHal singletonSub(ESensorDev_Sub);
    static FlickerHal singletonMain2(ESensorDev_MainSecond);
    static FlickerHal singletonSub2(ESensorDev_SubSecond);
    if(sensorDev==ESensorDev_Main)
        return &singletonMain;
    else if(sensorDev==ESensorDev_Sub)
        return &singletonSub;
    else if(sensorDev==ESensorDev_SubSecond)
        return &singletonSub2;
    else  //if(sensorDev==ESensorDev_MainSecond)
        return &singletonMain2;
}
//=====================================================
int FlickerHal::setFlickerModeSub(int mode)
{
    int temp_mode;
    switch(mode)
    {
    case MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ:
        logV("setflickermode 60");
        temp_mode = LIB3A_AE_FLICKER_MODE_60HZ;
        break;
    case MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ:
        logV("setflickermode 50");
        temp_mode = LIB3A_AE_FLICKER_MODE_50HZ;
        break;
    case MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO:
        logV("setflickermode auto");
        temp_mode = LIB3A_AE_FLICKER_MODE_AUTO;
        break;
    case MTK_CONTROL_AE_ANTIBANDING_MODE_OFF:
        logV("setflickermode off");
        temp_mode = LIB3A_AE_FLICKER_MODE_OFF;
        break;
    default:
        logV("setflickermode others (error mode)");
        temp_mode = LIB3A_AE_FLICKER_MODE_50HZ;
        break;
    }

    if(temp_mode != m_flickerMode){
        if(temp_mode == LIB3A_AE_FLICKER_MODE_AUTO)
            AAASensorMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 1);
        else if(m_flickerMode == LIB3A_AE_FLICKER_MODE_AUTO)
            AAASensorMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 0);
    }
    m_flickerMode = temp_mode;
    return 0;
}
//=====================================================
MINT32 FlickerHal::createBufSub()
{
    if(m_pVectorData1 != 0)
        logE("m_pVectorData1 is not empty. The release buffer should be called first!");
    m_pVectorData1 = (MINT32*)malloc(MHAL_FLICKER_WORKING_BUF_SIZE); //vector data, sw buffer
    if(m_pVectorData1 == NULL)
    {
        logE("memory1 is not enough");
        return -1;
    }

    m_pVectorData2 = (MINT32*)malloc(MHAL_FLICKER_WORKING_BUF_SIZE);
    if(m_pVectorData2 == NULL)
    {
        logE("memory2 is not enough");
        return -1;
    }
    return 0;
}
MVOID FlickerHal::releaseBufSub()
{
    if(m_pVectorData1 != NULL)
    {
        free(m_pVectorData1);
        m_pVectorData1 = NULL;
    }
    if(m_pVectorData2 != NULL)
    {
        free(m_pVectorData2);
        m_pVectorData2 = NULL;
    }
}
//=====================================================
FlickerHal::
FlickerHal(int sensorDev)
    : mTgInfo(CAM_TG_NONE)
    , mUniInfo(0)
    , mIsInited(0)
    , mSensorMode(SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
    , mIsEnable(0)
    , mUsers(0)
    , m_u4TgW(0)
    , m_u4TgH(0)
    , m_u4TgWReal(0)
    , m_u4TgHReal(0)
    , m_bFlickerModeChange(0)
    , m_bFlickerEnable(0)
    , m_bFlickerEnablebit(0)
    , m_u4FlickerWidth(0)
    , m_u4FlickerHeight(0)
    , m_u4SensorPixelClkFreq(0)
    , m_u4FreqFrame(0)
    , m_u4PixelsInLine(0)
    , m_flkAlgStatus(INCONCLUSIVE)
    , m_EIS_LMV_Flag(SMALL_MOTION)
    , m_pVectorAddress1(NULL)
    , m_iDataBank(0)
    , m_pVectorData1(NULL)
    , m_pVectorData2(NULL)
    , m_u4FlickerFreq(HAL_FLICKER_AUTO_50HZ)
    , FLK_DMA_Size(0)
    , m_FlickerMotionErrCount(0)
    , m_FlickerMotionErrCount2(0)
    , m_bPause(0)
    , m_flickerMode(LIB3A_AE_FLICKER_MODE_AUTO)
    , m_sensorDev(sensorDev)
    , m_sensorId(0)
    , m_firstOpenId(0)
    , m_currBuf(0)
    , m_i4FlickerDetectCycle(0)
    , mi4DetectedResult(0)
    , mai4GMV_X(0)
    , mai4GMV_Y(0)
{
    logV("func=%s line=%d",__FUNCTION__, __LINE__);
    if(cust_getFlickerDetectFrequency() >= 2)
        m_fgEnableDetectCycle = MTRUE;
    else
        m_fgEnableDetectCycle = MFALSE;

    memset(&strFlkWinCfg, 0, sizeof(FLKWinCFG_T));
    memset(&mai4AFWin[0], 0, eAFWinNum*sizeof(MINT64));
}
//=====================================================
FlickerHal::~FlickerHal()
{
    logV("func=%s line=%d",__FUNCTION__, __LINE__);
}
//=====================================================
void setFlickerStateHal(EV_TABLE val)
{
    set_flicker_state(val);
    if(val==Hz50)
        g_flickerHz=50;
    else
        g_flickerHz=60;
}
//=====================================================
static int getFlickerParameters(int sensorId, int sensorMode, FLICKER_EXT_PARA *flickerExtPara)
{
    /*
     * Priority 1. From file
     * Priority 2. From custom
     */

    /* get flicker parameters from custom */
    logV("Read flicker parameters from custom of sensorMode(%d).", sensorMode);
    FLICKER_CUST_PARA para;
    nvGetFlickerPara(sensorId, sensorMode, &para);

    flickerExtPara->flickerFreq[0]=para.flickerFreq[0];
    flickerExtPara->flickerFreq[1]=para.flickerFreq[1];
    flickerExtPara->flickerFreq[2]=para.flickerFreq[2];
    flickerExtPara->flickerFreq[3]=para.flickerFreq[3];
    flickerExtPara->flickerFreq[4]=para.flickerFreq[4];
    flickerExtPara->flickerFreq[5]=para.flickerFreq[5];
    flickerExtPara->flickerFreq[6]=para.flickerFreq[6];
    flickerExtPara->flickerFreq[7]=para.flickerFreq[7];
    flickerExtPara->flickerFreq[8]=para.flickerFreq[8];
    flickerExtPara->flickerGradThreshold=para.flickerGradThreshold;
    flickerExtPara->flickerSearchRange=para.flickerSearchRange;
    flickerExtPara->minPastFrames=para.minPastFrames;
    flickerExtPara->maxPastFrames=para.maxPastFrames;
    flickerExtPara->EV50_L50.m=para.EV50_L50.m;
    flickerExtPara->EV50_L50.b_l=para.EV50_L50.b_l;
    flickerExtPara->EV50_L50.b_r=para.EV50_L50.b_r;
    flickerExtPara->EV50_L50.offset=para.EV50_L50.offset;
    flickerExtPara->EV50_L60.m=para.EV50_L60.m;
    flickerExtPara->EV50_L60.b_l=para.EV50_L60.b_l;
    flickerExtPara->EV50_L60.b_r=para.EV50_L60.b_r;
    flickerExtPara->EV50_L60.offset=para.EV50_L60.offset;
    flickerExtPara->EV60_L50.m=para.EV60_L50.m;
    flickerExtPara->EV60_L50.b_l=para.EV60_L50.b_l;
    flickerExtPara->EV60_L50.b_r=para.EV60_L50.b_r;
    flickerExtPara->EV60_L50.offset=para.EV60_L50.offset;
    flickerExtPara->EV60_L60.m=para.EV60_L60.m;
    flickerExtPara->EV60_L60.b_l=para.EV60_L60.b_l;
    flickerExtPara->EV60_L60.b_r=para.EV60_L60.b_r;
    flickerExtPara->EV60_L60.offset=para.EV60_L60.offset;
    flickerExtPara->EV50_thresholds[0]=para.EV50_thresholds[0];
    flickerExtPara->EV50_thresholds[1]=para.EV50_thresholds[1];
    flickerExtPara->EV60_thresholds[0]=para.EV60_thresholds[0];
    flickerExtPara->EV60_thresholds[1]=para.EV60_thresholds[1];
    flickerExtPara->freq_feature_index[0]=para.freq_feature_index[0];
    flickerExtPara->freq_feature_index[1]=para.freq_feature_index[1];

    /* get parameters from file */
    int propReadParaFromFile;
    propReadParaFromFile = FlickerUtil::getPropInt("vendor.flk_read_file",0);
    if (propReadParaFromFile != 1)
        return -1;

    FILE *paraFile = NULL; // NOTE, sdcard will be removed often.
    if (sensorMode == e_sensorModePreview)
        paraFile = fopen("/sdcard/flk_para_preview.txt", "r");
    else if (sensorMode == e_sensorModeVideo)
        paraFile = fopen("/sdcard/flk_para_video.txt", "r");
    else if (sensorMode == e_sensorModeCapture)
        paraFile = fopen("/sdcard/flk_para_capture.txt", "r");
    else if (sensorMode == e_sensorModeVideo1)
        paraFile = fopen("/sdcard/flk_para_slim_video1.txt", "r");
    else if (sensorMode == e_sensorModeVideo2)
        paraFile = fopen("/sdcard/flk_para_slim_video2.txt", "r");
    else if (sensorMode == e_sensorModeCustom1)
        paraFile = fopen("/sdcard/flk_para_custom1.txt", "r");
    else if (sensorMode == e_sensorModeCustom2)
        paraFile = fopen("/sdcard/flk_para_custom2.txt", "r");
    else if (sensorMode == e_sensorModeCustom3)
        paraFile = fopen("/sdcard/flk_para_custom3.txt", "r");
    else if (sensorMode == e_sensorModeCustom4)
        paraFile = fopen("/sdcard/flk_para_custom4.txt", "r");
    else if (sensorMode == e_sensorModeCustom5)
        paraFile = fopen("/sdcard/flk_para_custom5.txt", "r");
    else {
        logE("Failed to get flicker parameters.");
        return -1;
    }

    if (!paraFile) {
        logE("Failed to open flicker parameters file.");
        return -1;
    }

    logV("Read flicker parameters from file.");
    int para_buf = 0;
	int ret;
    FLK_READ(flickerExtPara->flickerFreq[0]);
    FLK_READ(flickerExtPara->flickerFreq[1]);
    FLK_READ(flickerExtPara->flickerFreq[2]);
    FLK_READ(flickerExtPara->flickerFreq[3]);
    FLK_READ(flickerExtPara->flickerFreq[4]);
    FLK_READ(flickerExtPara->flickerFreq[5]);
    FLK_READ(flickerExtPara->flickerFreq[6]);
    FLK_READ(flickerExtPara->flickerFreq[7]);
    FLK_READ(flickerExtPara->flickerFreq[8]);
    FLK_READ(flickerExtPara->flickerGradThreshold);
    FLK_READ(flickerExtPara->flickerSearchRange);
    FLK_READ(flickerExtPara->minPastFrames);
    FLK_READ(flickerExtPara->maxPastFrames);
    FLK_READ(flickerExtPara->EV50_L50.m);
    FLK_READ(flickerExtPara->EV50_L50.b_l);
    FLK_READ(flickerExtPara->EV50_L50.b_r);
    FLK_READ(flickerExtPara->EV50_L50.offset);
    FLK_READ(flickerExtPara->EV50_L60.m);
    FLK_READ(flickerExtPara->EV50_L60.b_l);
    FLK_READ(flickerExtPara->EV50_L60.b_r);
    FLK_READ(flickerExtPara->EV50_L60.offset);
    FLK_READ(flickerExtPara->EV60_L50.m);
    FLK_READ(flickerExtPara->EV60_L50.b_l);
    FLK_READ(flickerExtPara->EV60_L50.b_r);
    FLK_READ(flickerExtPara->EV60_L50.offset);
    FLK_READ(flickerExtPara->EV60_L60.m);
    FLK_READ(flickerExtPara->EV60_L60.b_l);
    FLK_READ(flickerExtPara->EV60_L60.b_r);
    FLK_READ(flickerExtPara->EV60_L60.offset);
    FLK_READ(flickerExtPara->EV50_thresholds[0]);
    FLK_READ(flickerExtPara->EV50_thresholds[1]);
    FLK_READ(flickerExtPara->EV60_thresholds[0]);
    FLK_READ(flickerExtPara->EV60_thresholds[1]);
    FLK_READ(flickerExtPara->freq_feature_index[0]);
    FLK_READ(flickerExtPara->freq_feature_index[1]);

    fclose(paraFile);

    return 0;
}
//=====================================================
MINT32 FlickerHal::init(MINT32 i4SensorOpenIndex)
{
    logV("func=%s line=%d mUsers=%d",__FUNCTION__, __LINE__,mUsers);
    int iniFilerHz;
    //1: cust init hz
    cust_getFlickerHalPara(&iniFilerHz, &g_maxDetExpUs);
    //2: last detected hz
    if(g_flickerHz==50)
        iniFilerHz=50;
    else if(g_flickerHz==60)
        iniFilerHz=60;
    else
        ;
    //3: property hz
    int propDefFlicker;
    propDefFlicker = FlickerUtil::getPropInt(DEF_PROP_DEF_HZ,0);
    if(propDefFlicker==50)
        iniFilerHz = 50;
    else if(propDefFlicker==60)
        iniFilerHz = 60;
    else
        ;
    //ini flicker hz
    logV("init flicker Hz: %d",iniFilerHz);
    if(iniFilerHz == 50)
    {
        m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ;
        setFlickerStateHal(Hz50);
    }
    else
    {
        m_u4FlickerFreq = HAL_FLICKER_AUTO_60HZ;
        setFlickerStateHal(Hz60);
    }
    reset_flicker_queue();
    MINT32 err = 0;
    if (mUsers > 0)
    {
        logE("mUsers %d has created", mUsers);
        android_atomic_inc(&mUsers);
        return 0;
    }
    //sensor
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    IHalSensor* pHalSensorObj;
    pHalSensorObj = pIHalSensorList->createSensor("flicker", i4SensorOpenIndex);
    if(pHalSensorObj == NULL)
    {
        logV("error pHalSensorObj=0");
        return -1;
    }


    err = pHalSensorObj->sendCommand(m_sensorDev,SENSOR_CMD_GET_PIXEL_CLOCK_FREQ, (MUINTPTR)&m_u4SensorPixelClkFreq, 0, 0);
    if(err != 0)
    {
        logV("No plck. \n");
    }
    logV("[Flicker Hal]init - m_u4SensorPixelClkFreq: %d \n", m_u4SensorPixelClkFreq);

    err = pHalSensorObj->sendCommand(m_sensorDev,SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM, (MUINTPTR)&m_u4PixelsInLine, 0, 0);
    if(err != 0)
    {
        logV("No pixels per line. \n");
    }
    m_u4PixelsInLine &= 0x0000FFFF;
    android_atomic_inc(&mUsers);

    /* set flicker window, it can be a fixed window */
    int blkH, blkW;
    m_u4FlickerWidth = 0;
    m_u4FlickerHeight = 0;
    setWindowInfo(&blkH, &blkW);
    logV("Flicker parameter config read_freq=%d pixel_line=%d column_length=%d\n", (int)m_u4SensorPixelClkFreq, (int)m_u4PixelsInLine, (int)m_u4FlickerHeight);

    /* get sensor id */
    int sensorId=-1;
    SensorStaticInfo rSensorStaticInfo;

	rSensorStaticInfo.sensorDevID = 0;
    logV("ln=%d %d", __LINE__, m_sensorDev);
    switch (m_sensorDev) {
    case ESensorDev_Main:
        logV("ln=%d %d", __LINE__, m_sensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        break;
    case ESensorDev_Sub:
        logV("ln=%d %d", __LINE__, m_sensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        break;
    case ESensorDev_MainSecond:
        logV("ln=%d %d", __LINE__, m_sensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        break;
    case ESensorDev_SubSecond:
        logV("ln=%d %d", __LINE__, m_sensorDev);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
        break;
    default:    //  Shouldn't happen.
        logV("ln=%d %d", __LINE__, m_sensorDev);
        logE("Invalid sensor device: %d, id=%d", m_sensorDev, sensorId);
        break;
        // return MFALSE;
    }
    logV("ln=%d %d", __LINE__, rSensorStaticInfo.sensorDevID);
    sensorId = rSensorStaticInfo.sensorDevID;

    /* get sensor mode */
    int sensorMode;
    if (mSensorMode == SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
        sensorMode = e_sensorModePreview;
    else if (mSensorMode == SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
        sensorMode = e_sensorModeCapture;
    else if (mSensorMode == SENSOR_SCENARIO_ID_NORMAL_VIDEO)
        sensorMode = e_sensorModeVideoPreview;
    else if (mSensorMode == SENSOR_SCENARIO_ID_SLIM_VIDEO1)
        sensorMode = e_sensorModeVideo1;
    else if (mSensorMode == SENSOR_SCENARIO_ID_SLIM_VIDEO2)
        sensorMode = e_sensorModeVideo2;
    else if (mSensorMode == SENSOR_SCENARIO_ID_CUSTOM1)
        sensorMode = e_sensorModeCustom1;
    else if (mSensorMode == SENSOR_SCENARIO_ID_CUSTOM2)
        sensorMode = e_sensorModeCustom2;
    else if (mSensorMode == SENSOR_SCENARIO_ID_CUSTOM3)
        sensorMode = e_sensorModeCustom3;
    else if (mSensorMode == SENSOR_SCENARIO_ID_CUSTOM4)
        sensorMode = e_sensorModeCustom4;
    else // if (mSensorMode == SENSOR_SCENARIO_ID_CUSTOM5)
        sensorMode = e_sensorModeCustom5;

    /* get parameters */
    getFlickerParameters(sensorId, sensorMode, &g_flickerExtPara);

    /* set parameter to flicker algo */
    flicker_setExtPara(&g_flickerExtPara);

    /* modify piexel line when binning */
    int ratio;
    ratio = m_u4TgHReal / m_u4TgH;
    if (ratio > 1) {
        m_u4PixelsInLine *= ratio;
        logV("Update flicker with bin ration(%d), and new pixel line(%d).", ratio, m_u4PixelsInLine);
    }
    int propRatio;
    propRatio = FlickerUtil::getPropInt("vendor.flk_ratio",0);
    if (propRatio) {
        m_u4PixelsInLine *= propRatio;
        logV("Update flicker from property with bin ration(%d), and new pixel line(%d).", ratio, m_u4PixelsInLine);
    }

    /* get pixel line, height, width and pixel clock from property */
    int propPixel, propH, propW, propClk;
    propPixel = FlickerUtil::getPropInt("vendor.flk_pixel",0);
    if (propPixel) {
        m_u4PixelsInLine = propPixel;
        logV("Update pixel line(%d).", m_u4PixelsInLine);
    }
    propH = FlickerUtil::getPropInt("vendor.flk_h",0);
    if (propH) {
        blkH = propH;
        logV("Update window height(%d).", blkH);
    }
    propW = FlickerUtil::getPropInt("vendor.flk_w",0);
    if (propW) {
        blkW = propW;
        logV("Update window width(%d).", blkW);
    }
    propClk = FlickerUtil::getPropInt("vendor.flk_clk",0);
    if (propClk) {
        m_u4SensorPixelClkFreq = propClk;
        logV("Update pixel clock frequency(%d).", m_u4SensorPixelClkFreq);
    }

	/* init flicker algo */
    int ta, tb;

    logV("flicker_init(%d,%d,%d,%d).",m_u4PixelsInLine, blkH * 3, blkW, (int)m_u4SensorPixelClkFreq);
    ta = FlickerUtil::getMs();
    flicker_init(m_u4PixelsInLine, blkH * 3, blkW, m_u4SensorPixelClkFreq);
    tb = FlickerUtil::getMs();
    logV("flicker algo init time(%d ms).",(int)(tb-ta));

    if( pHalSensorObj )
        pHalSensorObj->destroyInstance("flicker");
    mIsInited=1;
    return err;
}
//=====================================================
MINT32 FlickerHal::uninit()
{
    logV("uninit line=%d mUsers=%d",__LINE__, mUsers);
    MINT32 err = 0;
    if (mUsers <= 0)
    {
        // No more users
        return 0;
    }
    // More than one user
    android_atomic_dec(&mUsers);
    if (mUsers == 0)
    {
        flicker_uninit(); //alg uninit
        mIsInited=0;
    }
    else
    {
        logV("Still %d users \n", mUsers);
    }
    return 0;
}
//=============================================
void focusValueResize(MINT64* inH, MINT64* inV, MINT32* out)
{
    int i;
    for(i=0; i<9; i++)
    {
        int row;
        int col;
        double tmp;
        row=i/3;
        col=i%3;
        tmp	 = inH[(col*2)  +(row*2)*6]+
               inH[(col*2+1)+(row*2)*6]+
               inH[(col*2)  +(row*2+1)*6]+
               inH[(col*2+1)+(row*2+1)*6];
        tmp	 +=inV[(col*2)  +(row*2)*6]+
               inV[(col*2+1)+(row*2)*6]+
               inV[(col*2)  +(row*2+1)*6]+
               inV[(col*2+1)+(row*2+1)*6];
        if(tmp > 0x7fffffff)
            tmp=0x7fffffff;
        out[i]=tmp;
        logV("fvr i=%d, fv=%d",i,out[i]);
    }

}
//=============================================
int FlickerHal::Updated(FlickerInput* in, FlickerOutput* out)
{
    if(mIsInited==0)
        return -1;
/*
    {
        if(m_flickerMode==LIB3A_AE_FLICKER_MODE_AUTO)
        {
            AAASensorMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 1);
        }
        else
        {
            AAASensorMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 0);
        }
    }
*/


    int i;
    static int kk=0;
    MINT32 fv3x3[9] = {0};

    logV("U %d",kk);
    kk++;
    //---------------------
    // log input
    logV("exp=%d",in->aeExpTime);
/* af information currently not used
    for(i=0; i<6; i++)
    {
        logV("fvH row%d=%lf %lf %lf %lf %lf %lf",i,
             (double)(in->afFullStat.i8StatH[i*6+0]), (double)(in->afFullStat.i8StatH[i*6+1]), (double)(in->afFullStat.i8StatH[i*6+2]),
             (double)(in->afFullStat.i8StatH[i*6+3]), (double)(in->afFullStat.i8StatH[i*6+4]), (double)(in->afFullStat.i8StatH[i*6+5]) );
    }
        for(i=0; i<6; i++)
        {
        logV("fvV row%d=%lf %lf %lf %lf %lf %lf",i,
             (double)(in->afFullStat.i8StatV[i*6+0]), (double)(in->afFullStat.i8StatV[i*6+1]), (double)(in->afFullStat.i8StatV[i*6+2]),
             (double)(in->afFullStat.i8StatV[i*6+3]), (double)(in->afFullStat.i8StatV[i*6+4]), (double)(in->afFullStat.i8StatV[i*6+5]) );
        }
    focusValueResize(in->afFullStat.i8StatH, in->afFullStat.i8StatV, fv3x3);

        logV("fv3x3 %d %d %d", out[0], out[1], out[2]);
        logV("fv3x3 %d %d %d", out[3], out[4], out[5]);
        logV("fv3x3 %d %d %d", out[6], out[7], out[8]);
*/
    //---------------------------
    MINT32	err = 0;
    MINT32	i4DetectedResult = -1;
    g_frmCntPre++;
    if(g_frmCntPre<2)
    {
        goto lbExit;
    }
    //	(3) Analyze the flicker by passing EIS information.
    if(m_flickerMode==LIB3A_AE_FLICKER_MODE_AUTO)
    err = analyzeFlickerFrequency(in->aeExpTime, fv3x3, in->pBuf);

    //  (4) Check if enable multi-frames Flicker detection or not.
    if(m_fgEnableDetectCycle)
    {
        m_i4FlickerDetectCycle = (m_i4FlickerDetectCycle+1) % cust_getFlickerDetectFrequency();
        logV("cycle=%d", m_i4FlickerDetectCycle);
    }

    if	(err !=0)
    {
        logV("Updated] mpFlickerHal->analyzeFlickerFrequency() - (err)=(%x)",  err);
        goto lbExit;
    }
    //	(4) Get the flicker result from flicker hal
    err = getFlickerStatus(&i4DetectedResult);
    if	(err !=0)
    {
        logV("[Updated] getFlickerStatus err=%d", err);
        goto lbExit;
    }
    out->flickerResult = i4DetectedResult;

lbExit:
    return	err;
}
//=============================================
MINT32 FlickerHal::analyzeFlickerFrequency(int exp, MINT32 *i4vAFstatisic, MVOID *buf)
{
    // cycle = 0, copy data.
    // cycle = 1, copy data and do Flicker detection.
    // other, do nothing.
    if(m_fgEnableDetectCycle && m_i4FlickerDetectCycle >= 2)
        return 0;

    logV("A ln=%d cnt=%d",__LINE__,g_frmCntPre);
    MINT32 i,i4Buff_idx=0;
    MINT32 i4vAFInfo[9];
    MINT32 i4DataLen = 0;
    MINT32 u4Height;
    MINT32 *m_FickerSW_buff_1;
    MINT32 *m_FickerSW_buff_2;

    MINT32* flickerDMAAdr;

    // get the AF statistic information
    i4vAFInfo[0]=i4vAFstatisic[0];
    i4vAFInfo[1]=i4vAFstatisic[1];
    i4vAFInfo[2]=i4vAFstatisic[2];
    i4vAFInfo[3]=i4vAFstatisic[3];
    i4vAFInfo[4]=i4vAFstatisic[4];
    i4vAFInfo[5]=i4vAFstatisic[5];
    i4vAFInfo[6]=i4vAFstatisic[6];
    i4vAFInfo[7]=i4vAFstatisic[7];
    i4vAFInfo[8]=i4vAFstatisic[8];
//FIXME
    if(m_currBuf == 0)
    {
        m_currBuf=1;
        m_FickerSW_buff_1=m_pVectorData1;  //m_FickerSW_buff_1 is n-1 data
        m_FickerSW_buff_2=m_pVectorData2;  // m_FickerSW_buff_1 is n-2 data
    }
    else
    {
        m_currBuf=0;
        m_FickerSW_buff_1=m_pVectorData2;  //m_FickerSW_buff_1 is n-1 data
        m_FickerSW_buff_2=m_pVectorData1;  //m_FickerSW_buff_2 is n-2 data
   }


/*FIXME
    flickerDMAAdr = (MINT32 *) (8*((flkbufInfo[(i4Buff_idx+1)%2].virtAddr + 7)/8));
*/
    flickerDMAAdr = (MINT32 *) buf;

    if(m_u4FlickerHeight > FLICKER_MAX_LENG)
    {
        i4DataLen = 3*FLICKER_MAX_LENG /2 ;
        u4Height = FLICKER_MAX_LENG;
    }
    else
    {
        i4DataLen = 3*m_u4FlickerHeight /2 ;
        u4Height = m_u4FlickerHeight;
    }

    for(i=0; i<i4DataLen; i++)
    {
        m_FickerSW_buff_1[2*i+0] = flickerDMAAdr[i] &0x0000FFFF;
        m_FickerSW_buff_1[2*i+1] =(flickerDMAAdr[i] &0xFFFF0000)>>16;
    }
    for(i=0; i<u4Height; i+=100)
    {
        logV("sta row =%d, %d %d %d",i,m_FickerSW_buff_1[3*i],m_FickerSW_buff_1[3*i+1],m_FickerSW_buff_1[3*i+2]);
    }

    if(m_fgEnableDetectCycle && m_i4FlickerDetectCycle == 0)
        return 0;

    //switch FKO dst add.  to another buffer

    flkSensorInfo	sensorInfo;
    sensorInfo.pixelClock = m_u4SensorPixelClkFreq;
    sensorInfo.fullLineWidth = m_u4PixelsInLine;
    MINT32 win_wd = ((m_u4FlickerWidth / 3)>>1)<<1;
    MINT32 win_ht = ((u4Height / 3)>>1)<<1;
    flkEISVector	EISvector;
    int curExp;
    curExp = exp;
    flkAEInfo AEInfo;
    AEInfo.previewShutterValue=curExp;
    static int sBinOut=-1;
    int binOut;
    int binOut2;
    int afOn;
    binOut = FlickerUtil::getPropInt("vendor.flk_bin_out",0);
    binOut2 = FlickerUtil::getPropInt("vendor.flk_bin_out2",0);
    afOn = FlickerUtil::getPropInt("vendor.flk_af_on",1);

    if(afOn==0)
    {
        logV("AF OFF");
        int pp;
        for(pp=0; pp<9; pp++)
        {
            i4vAFInfo[pp]=0;
        }
    }
    if(binOut!=1)
    {
        sBinOut = binOut;
    }
    else if( (binOut==1 && sBinOut!=1) || binOut2==1)
    {
        sBinOut=binOut;
        int cnt;
        FlickerUtil::getFileCount("/sdcard/flicker_file_cnt.txt", &cnt, 0);
        FlickerUtil::setFileCount("/sdcard/flicker_file_cnt.txt", cnt+1);
        char s[100];
        FlickerUtil::createDir("/sdcard/flickerdata/");
        sprintf(s,"/sdcard/flickerdata/%03d",cnt);
        FlickerUtil::createDir(s);
        int tmp;
        FILE* fp;
        sprintf(s,"/sdcard/flickerdata/%03d/flk.raw",cnt);
        fp = fopen(s, "wb");
        if(fp)
        {
            tmp = i4DataLen;
            fwrite(&tmp, 1, 4, fp);
            fwrite(m_FickerSW_buff_1, 4, i4DataLen*2, fp);
            fwrite(m_FickerSW_buff_2, 4, i4DataLen*2, fp);
            tmp = win_wd;        fwrite(&tmp, 1, 4, fp);
            tmp = win_ht;        fwrite(&tmp, 1, 4, fp);
            tmp = m_u4FlickerFreq;        fwrite(&tmp, 1, 4, fp);
            fwrite(&sensorInfo, 1, sizeof(flkSensorInfo), fp);
            fwrite(&AEInfo, 1, sizeof(flkAEInfo), fp);
            fwrite(i4vAFInfo, 4, 9, fp);
            fclose(fp);
            logV("line=%d", __LINE__);
        }
    }
    if(   curExp < g_maxDetExpUs &&
            ( (curExp > 8200 && m_u4FlickerFreq == HAL_FLICKER_AUTO_60HZ ) || (curExp > 9800 && m_u4FlickerFreq == HAL_FLICKER_AUTO_50HZ ) ) )
    {
        //logI("do flicker Sw %d %d %d %d",win_wd, win_ht, curExp, m_u4FlickerFreq);
        m_flkAlgStatus = detectFlicker_SW(m_FickerSW_buff_1, m_FickerSW_buff_2, 3, 3, win_wd, win_ht, m_u4FlickerFreq, sensorInfo, EISvector, AEInfo, i4vAFInfo);

        int propFlickerSwitch;
        propFlickerSwitch = FlickerUtil::getPropInt("vendor.flk_switch_en",-1);
        if(propFlickerSwitch==0)
        {
            logV("flicker state not changed due to property fixed");
        }
        else
        {
            if(m_flkAlgStatus == FK100 && m_u4FlickerFreq==HAL_FLICKER_AUTO_60HZ) // if the decision is to change exposure table to 50Hz
            {
                m_u4FlickerFreq = HAL_FLICKER_AUTO_50HZ;
                setFlickerStateHal(Hz50);	// defined in "sequential_testing.cpp", have to call these two functions every time we change the flicker table
                reset_flicker_queue();
            }
            else if (m_flkAlgStatus == FK120 && m_u4FlickerFreq==HAL_FLICKER_AUTO_50HZ) // if the decision is to change exposure table to 60Hz
            {
                m_u4FlickerFreq = HAL_FLICKER_AUTO_60HZ;
                setFlickerStateHal(Hz60);	// defined in "sequential_testing.cpp", have to call these two functions every time we change the flicker table
                reset_flicker_queue();
            }
        }
    }
    else if( curExp >= g_maxDetExpUs)
    {
        logV("The exposure time is too long, skip flicker detection:%d\n", curExp);
    }
    else
    {
        logV("The exposure time is too short, skip flicker detection:%d\n", curExp);
    }
    logV("Status:%d, %d\n",m_u4FlickerFreq,curExp);
    return 0;
}
//=============================================
MINT32 FlickerHal::enableFlickerDetection(MBOOL bEnableFlicker)
{
    MINT32 ret = 0;	// 0: no error.
    logV("enableFlickerDetection() bEnableFlicker= %d\n",bEnableFlicker);
    if(mIsEnable==bEnableFlicker)
    {
        logV("flicker enable state is ame, not set to hardware");
        return 0;
    }
    mIsEnable = bEnableFlicker;
    return ret;
}
//=============================================
MINT32 FlickerHal::setWindowInfo(int* blkH, int*blkW)
{
    logV("func=%s line=%d",__FUNCTION__, __LINE__);
    int imgW=0;
    int imgH=0;
    //--------------------------------------------
    // get sensor info
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    IHalSensor* pIHalSensor = pIHalSensorList->createSensor("FlickerHal", m_sensorId);
    SensorDynamicInfo rSensorDynamicInfo;
    SensorStaticInfo rSensorStaticInfo;

    if(pIHalSensor){
    switch  ( m_sensorDev )
    {
    case ESensorDev_Main:
        pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        break;
    case ESensorDev_Sub:
        pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        break;
    case ESensorDev_MainSecond:
        pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        break;
    case ESensorDev_SubSecond:
        pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorDynamicInfo);
        pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB_2, &rSensorStaticInfo);
        break;
    default:    //  Shouldn't happen.
        logE("Invalid sensor device: %d, id=%d", m_sensorDev,m_sensorId);
        return MFALSE;
    }
	logV("sensorWH, pv %d %d, cap %d %d, vd %d %d, v1 %d %d, v2 %d %d",
	rSensorStaticInfo.previewWidth,
	rSensorStaticInfo.previewHeight,
	rSensorStaticInfo.captureWidth,
	rSensorStaticInfo.captureHeight,
	rSensorStaticInfo.videoWidth,
	rSensorStaticInfo.videoHeight,
	rSensorStaticInfo.video1Width,
	rSensorStaticInfo.video1Height,
	rSensorStaticInfo.video2Width,
	rSensorStaticInfo.video2Height);

    //imgW=rSensorStaticInfo.previewWidth;
    //imgH=rSensorStaticInfo.previewHeight;
	imgW = m_u4TgW;
	imgH = m_u4TgH;


    logV("setFlickerWinConfig TG = %d, w %d, h %d, dev %d, id %d\n", rSensorDynamicInfo.TgInfo, imgW, imgH, m_sensorDev, m_sensorId);
        pIHalSensor->destroyInstance("FlickerHal");
    }else     {
		logE("pIHalSensor is NULL");
    }

    //--------------------------------------------
    int u4ToleranceLine=20;
    imgW-=3;
    imgH-=25;

    m_u4FlickerWidth = imgW;
    m_u4FlickerHeight = imgH;
    //logI("[setWindowInfo] width:%d ,%d height:%d ,%d\n", u4Width, m_u4FlickerWidth, u4Height, m_u4FlickerHeight);
    if(m_u4FlickerHeight > FLICKER_MAX_LENG-6)
    {
        imgH = FLICKER_MAX_LENG-6;
    }
    else
    {
        imgH = m_u4FlickerHeight;
    }
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.slow_motion_debug", value, "0");
//    if(atoi(value))
    {
	    //imgW/=6;
	    //imgH/=6;
    }
    strFlkWinCfg.m_uImageW=imgW;
    strFlkWinCfg.m_uImageH=imgH;
    strFlkWinCfg.m_u4NumX=3;
    strFlkWinCfg.m_u4NumY=3;
    strFlkWinCfg.m_u4OffsetX=0;
    strFlkWinCfg.m_u4OffsetY=0+u4ToleranceLine;

    strFlkWinCfg.m_u4SizeX=((imgW-strFlkWinCfg.m_u4OffsetX)/6)*2;
    strFlkWinCfg.m_u4SizeY=((imgH-strFlkWinCfg.m_u4OffsetY+u4ToleranceLine)/6)*2;

    strFlkWinCfg.m_u4DmaSize = strFlkWinCfg.m_u4NumX*strFlkWinCfg.m_u4NumY*strFlkWinCfg.m_u4SizeY*2;
    strFlkWinCfg.m_u4SGG3_PGN = 16;
    strFlkWinCfg.m_u4SGG3_GMR1 = 20;
    strFlkWinCfg.m_u4SGG3_GMR2 = 29;
    strFlkWinCfg.m_u4SGG3_GMR3 = 42;
    strFlkWinCfg.m_u4SGG3_GMR4 = 62;
    strFlkWinCfg.m_u4SGG3_GMR5 = 88;
    strFlkWinCfg.m_u4SGG3_GMR6 = 126;
    strFlkWinCfg.m_u4SGG3_GMR7 = 180;

    ISP_MGR_FLK_CONFIG_T::getInstance(static_cast<ESensorDev_T>(m_sensorDev)).config(strFlkWinCfg);


    *blkH = strFlkWinCfg.m_u4SizeY;
    *blkW = strFlkWinCfg.m_u4SizeX;
    logV("blkH=%d, blkW = %d, DMA_SIZE = %d",*blkH, *blkW, strFlkWinCfg.m_u4DmaSize);
    logV("w,h,oft_x, oft_y, sz_x, sz_y %d %d %d %d %d %d", strFlkWinCfg.m_uImageW, strFlkWinCfg.m_uImageH, strFlkWinCfg.m_u4OffsetX, strFlkWinCfg.m_u4OffsetY, strFlkWinCfg.m_u4SizeX, strFlkWinCfg.m_u4SizeY);
    return 0;
}
MINT32 FlickerHal::getFlickerStatus(MINT32 *a_flickerStatus)
{
    if(m_flickerMode==LIB3A_AE_FLICKER_MODE_60HZ)
    {
        logV("getFlickerStatus 60");
        *a_flickerStatus= HAL_FLICKER_AUTO_60HZ;
    }
    else if(m_flickerMode==LIB3A_AE_FLICKER_MODE_50HZ)
    {
        logV("getFlickerStatus 50");
        *a_flickerStatus= HAL_FLICKER_AUTO_50HZ;
    }
    else if(m_flickerMode==LIB3A_AE_FLICKER_MODE_OFF)
    {
        logV("getFlickerStatus off");
        *a_flickerStatus = HAL_FLICKER_AUTO_OFF;
    }
    else // LIB3A_AE_FLICKER_MODE_AUTO
    {
        logV("getFlickerStatus auto");
        *a_flickerStatus = m_u4FlickerFreq;
    }
    //------------------------------
    logV("getFlickerStatus %d",(int)*a_flickerStatus);
    return 0;
}
int FlickerHal::enable(int en)
{
    logV("func=%s ln=%d",__FUNCTION__, __LINE__);
    enableFlickerDetection(en);
    return 0;
}
//=====================================================
int FlickerHal::setSensorMode(int i4NewSensorMode, int width, int height)
{
    mSensorMode = i4NewSensorMode;
    m_u4TgW = width;
    m_u4TgH = height;
    return 0;
}
int FlickerHal::setTGSize(int width, int height)
{
    m_u4TgWReal = width;
    m_u4TgHReal = height;
    return 0;
}
int FlickerHal::update(FlickerInput* in, FlickerOutput* out)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.slow_motion_debug", value, "0");
    if(atoi(value))
    {
	    return 0;
    }
    Mutex::Autolock lock(m_lock);
    if(mUniInfo != 1)
    {
        logV("func=%s Not First Open, return",__FUNCTION__);
        return MFALSE;
    }
    return Updated(in, out);
}
int FlickerHal::setFlickerMode(int mode)
{
    Mutex::Autolock lock(m_lock);
    return setFlickerModeSub(mode);
}
int FlickerHal::getFlickerResult(int& flickerResult)
{
    Mutex::Autolock lock(m_lock);
    return getFlickerStatus((MINT32 *)&flickerResult);
}

static int lookupAntiBandingMode(int mode)
{
    int antiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ;

    switch (mode) {
    case LIB3A_AE_FLICKER_MODE_60HZ:
        antiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ;
        break;
    case LIB3A_AE_FLICKER_MODE_50HZ:
        antiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ;
        break;
    case LIB3A_AE_FLICKER_MODE_AUTO:
        antiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO;
        break;
    case LIB3A_AE_FLICKER_MODE_OFF:
        antiBandingMode = MTK_CONTROL_AE_ANTIBANDING_MODE_OFF;
        break;
    default:
        break;
    }

    return antiBandingMode;
}

/* EVEREST Interface */
MBOOL FlickerHal::Start()
{
    debugSetting();
    Mutex::Autolock lock(m_lock);

    logV("func=%s ln=%d tgInfo=%d",__FUNCTION__, __LINE__, mTgInfo);

    if(mUniInfo != 1)
    {
        logV("func=%s Not First Open, return",__FUNCTION__);
        return MFALSE;
    }
    if (init(m_sensorId) != 0)
    {
        logV("singleton.init() fail \n");
        return MFALSE;
    }

    /* reset flicker mode */
    int lastMode = m_flickerMode;
    setFlickerModeSub(MTK_CONTROL_AE_ANTIBANDING_MODE_OFF);
    setFlickerModeSub(lookupAntiBandingMode(lastMode));

    return MTRUE;
}

MBOOL FlickerHal::Stop()
{
    logV("func=%s ln=%d tgInfo=%d",__FUNCTION__, __LINE__,mTgInfo);
    Mutex::Autolock lock(m_lock);
    if(mUniInfo != 1)
    {
       logV("func=%s Not First Open, return",__FUNCTION__);
        return MFALSE;
    }
    uninit();
    return MTRUE;
}

MBOOL FlickerHal::setUniInfo(MBOOL const i4UniInfo)
{
    mUniInfo = i4UniInfo;
    CAM_LOGD("[%s] UniInfo = %d\n", __FUNCTION__, mUniInfo);
    return MTRUE;
}
MRESULT FlickerHal::Init(MINT32 const i4SensorIdx)
{
    logV("func=%s ln=%d",__FUNCTION__, __LINE__);
    Mutex::Autolock lock(m_lock);
    m_sensorId = i4SensorIdx;
    createBufSub();
    return S_3A_OK;
}

MRESULT FlickerHal::Uninit()
{
    logV("func=%s ln=%d",__FUNCTION__, __LINE__);
    releaseBufSub();
    return S_3A_OK;
}

