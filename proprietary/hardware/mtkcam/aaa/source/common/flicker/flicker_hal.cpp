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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "FlickerHal"
#define MTK_LOG_ENABLE 1

/***********************************************************
 * Headers
 **********************************************************/
/* standard headers */
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/threads.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <array>

/* camera headers */
#include <mtkcam/def/common.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <private/IopipeUtils.h>

/* custom headers */
#include <aaa_error_code.h>

/* aaa headers */
#include <nvram_drv.h>

#include <ae_mgr_if.h>

/* aaa common headers */
#include "file_utils.h"
#include "property_utils.h"
#include "time_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* flicker headers */
#include "flicker_hal.h"
#include "flicker_utils.h"
#include "flicker_platform_adapter.h"
//#include "flicker_hal_type.h"

#include <mtkcam/utils/std/Trace.h>

using namespace NS3Av3;


/***********************************************************
 * Define macros
 **********************************************************/
#define FLICKER_MAX_LENG (6000)
//#define NUM_X (3)

#define PROP_FLK_DEF_HZ         "vendor.debug.flk_def_hz" /* (50|60) */
#define PROP_FLK_READ_FILE      "vendor.debug.flk_read_file"
#define PROP_FLK_RATIO          "vendor.debug.flk_ratio"
#define PROP_FLK_CLK            "vendor.debug.flk_clk"
#define PROP_FLK_PIXEL          "vendor.debug.flk_pixel"
#define RPOP_FLK_H              "vendor.debug.flk_h"
#define RPOP_FLK_W              "vendor.debug.flk_w"
#define PROP_FLK_DUMP           "vendor.debug.flk_dump"
#define PROP_FLK_DUMP_PURE      "vendor.debug.flk_dump_pure"
#define PROP_FLK_SWITCH_EN      "vendor.debug.flk_switch_en"
#define PROP_FLK_NOT_ATTACH     "vendor.debug.flk_not_attach"
#define PROP_FLK_DISABLE        "vendor.debug.aaa_flk.disable"
#define PROP_FLK_PROP_DEBUG     "vendor.debug.flk_prop_debug"
#define PROP_FLK_LOG            "vendor.debug.flicker.log"
#define PROP_FLK_OUT_LINE_DATA  "vendor.z.flicker_out_line_data"

#define FLK_PARA_PREVIEW_FILE     "/data/vendor/flicker/para/flk_para_preview.txt"
#define FLK_PARA_VIDEO_FILE       "/data/vendor/flicker/para/flk_para_video.txt"
#define FLK_PARA_CAPTURE_FILE     "/data/vendor/flicker/para/flk_para_capture.txt"
#define FLK_PARA_SLIM_VIDEO1_FILE "/data/vendor/flicker/para/flk_para_slim_video1.txt"
#define FLK_PARA_SLIM_VIDEO2_FILE "/data/vendor/flicker/para/flk_para_slim_video2.txt"
#define FLK_PARA_CUSTOM1_FILE     "/data/vendor/flicker/para/flk_para_custom1.txt"
#define FLK_PARA_CUSTOM2_FILE     "/data/vendor/flicker/para/flk_para_custom2.txt"
#define FLK_PARA_CUSTOM3_FILE     "/data/vendor/flicker/para/flk_para_custom3.txt"
#define FLK_PARA_CUSTOM4_FILE     "/data/vendor/flicker/para/flk_para_custom4.txt"
#define FLK_PARA_CUSTOM5_FILE     "/data/vendor/flicker/para/flk_para_custom5.txt"


#define FLK_DUMP_COUNT_FILE "/data/vendor/flicker/flko/flicker_file_cnt.txt"

/***********************************************************
 * Flicker HAL
 **********************************************************/
int FlickerHal::m_flickerState = HAL_FLICKER_AUTO_OFF;
int FlickerHal::m_attachCount = 0;
Mutex FlickerHal::m_attachLock;

FlickerHal::FlickerHal(int sensorDev)
    : mUsers(0)
    , m_sensorDev(sensorDev)
    , m_sensorIdx(0)
    , m_isAttach(0)
    , mTgInfo(CAM_TG_NONE)
    , m_u4TgWReal(0)
    , m_u4TgHReal(0)
    , mSensorMode(SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
    , m_u4TgW(0)
    , m_u4TgH(0)
    , m_imgW(0)
    , m_imgH(0)
    , m_winW(0)
    , m_winH(0)
    , m_u4SensorPixelClkFreq(0)
    , m_u4PixelsInLine(0)
    , m_flickerMode(MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO)
    , m_maxDetExpUs(70000)
    , m_currentFrame(0)
    , m_currentCycle(0)
    , m_alreadyGetFlickerPara(0)
    , m_p3ASttCtrl(NULL)
{
    void* para_ptr = calloc(FLK_MAX_MODE_NUM, sizeof(FLICKER_CUST_PARA*));
    if(para_ptr != nullptr)
    {
        m_flkParaArray = (FLICKER_CUST_PARA**)para_ptr;

        for(int i=0;i<FLK_MAX_MODE_NUM;i++)
        {
            para_ptr = calloc(FLK_MAX_BIN_NUM, sizeof(FLICKER_CUST_PARA));
            if(para_ptr != nullptr)
            {
                m_flkParaArray[i] = (FLICKER_CUST_PARA*)para_ptr;
            }
            else
            {
                logE("Can not locate memory for parameters.");
                m_flkParaArray[i] = nullptr;
            }
        }
    }
    else
    {
        logE("Can not locate memory for parameters.");
        m_flkParaArray = nullptr;
    }

    //memset(&m_flkParaArray, 0, FLK_MAX_MODE_NUM*FLK_MAX_BIN_NUM*sizeof(FLICKER_CUST_PARA));

    logD("FlickerHal(): sensorDev(%d).", sensorDev);

    m_maxAttachNum = cust_getMaxAttachNum();
    m_detectCycle = cust_getFlickerDetectFrequency();
    if (m_detectCycle < 2)
        m_detectCycle = 2;
}

FlickerHal::~FlickerHal()
{
    for(int i=0;i<FLK_MAX_MODE_NUM;i++)
    {
        if (m_flkParaArray[i])
        {
            free(m_flkParaArray[i]);
            m_flkParaArray[i] = NULL;
        }
    }
    if (m_flkParaArray)
    {
        free(m_flkParaArray);
        m_flkParaArray = NULL;
    }
}

#define MY_INST NS3Av3::INST_T<FlickerHal>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

FlickerHal* FlickerHal::getInstance(int sensorDev)
{
    int sensorOpenIndex = NS3Av3::mapSensorDevToIdx(sensorDev);
    if(sensorOpenIndex >= SENSOR_IDX_MAX || sensorOpenIndex < 0)
    {
        logE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, sensorOpenIndex);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[sensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<FlickerHal>(sensorDev);
    } );

    return rSingleton.instance.get();

}

/***********************************************************
 * Flicker life cycle
 **********************************************************/
MRESULT FlickerHal::init(MINT32 const i4SensorIdx)
{
    Mutex::Autolock lock(m_lock);
    logI("init().");

    /* set debug */
    setDebug();
    logI("init flicker custom adaptor.");
    m_sensorIdx = i4SensorIdx;
    FlickerPlatformAdapter::getInstance(m_sensorDev)->init(m_sensorIdx);

    if (m_alreadyGetFlickerPara == 0)
    {
        logI("init(). getFlickerParametersAll");
        getFlickerParametersAll(FLK_MAX_MODE_NUM, FLK_MAX_BIN_NUM);
        m_alreadyGetFlickerPara = 1;
    }

    return S_3A_OK;
}

MRESULT FlickerHal::uninit()
{
    Mutex::Autolock lock(m_lock);
    logI("uninit().");
    FlickerPlatformAdapter::getInstance(m_sensorDev)->uninit();

    return S_3A_OK;
}

MBOOL FlickerHal::config()
{
    Mutex::Autolock lock(m_lock);
    setWindowInfo(&m_imgW, &m_imgH, &m_winW, &m_winH);
    return MTRUE;
}

MBOOL FlickerHal::reconfig(MVOID *in, MVOID *out)
{
    return MTRUE;
}


MBOOL FlickerHal::reconfig(MUINT32 *tgWidth, MUINT32 *tgHeight, MUINT32 *binWidth, MUINT32 *binHeight)
{
    Mutex::Autolock lock(m_lock);
    setWindowInfoReconfig(&m_imgW, &m_imgH, &m_winW, &m_winH, tgWidth, tgHeight, binWidth, binHeight);
    return MTRUE;
}

#if ((!CAM3_3A_ISP_30_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_40_EN && CAM3_3A_IP_BASE) && (!CAM3_3A_ISP_50_EN && CAM3_3A_IP_BASE))
MVOID FlickerHal::getHWCfgReg(FLKResultConfig_T *pResultConfig)
{
    FlickerPlatformAdapter::getInstance(m_sensorDev)->getFLKResultConfig((MVOID *)pResultConfig);
    logD("HW-%s Enable(%d) m_sensorDev(%d)", __FUNCTION__, pResultConfig->enableFLKHw, m_sensorDev);
}
#endif


MBOOL FlickerHal::start(FLK_ATTACH_PRIO_T prio)
{
    Mutex::Autolock lock(m_lock);

    logI("start(): begin. m_sensorDev(%d)", m_sensorDev);
    if (attach(prio))
    {
        logI("start(): failed to attach. m_sensorDev(%d)", m_sensorDev);
        return MFALSE;
    }

    m_p3ASttCtrl = Hal3ASttCtrl::getInstance(m_sensorDev);
    int type = m_p3ASttCtrl->isMvHDREnable();
    int sensorType = 0;
    FlickerPlatformAdapter::getInstance(m_sensorDev)->getSensorType(type, &sensorType);

    logD("start(): sensorType(%d). m_sensorDev(%d)", sensorType, m_sensorDev);

    if (init_algo(sensorType)) {
        logE("config(): failed to init. m_sensorDev(%d)", m_sensorDev);
        return MFALSE;
    }

    /*
     * Reset framerate active because of that:
     * 1. Sensor driver will set default 'off' when chaning sensor mode.
     * 2. setFlickerFrameRateActive() only update driver when settings changed
     */
    if (m_flickerMode == MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO) {
        IAeMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 0);
        IAeMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 1);
    }

    m_currentFrame = 0;
    m_currentCycle = 0;

    int32_t logFlk = 0;
    int32_t outlinedataFlk = 0;
    getPropInt(PROP_FLK_LOG, &logFlk, 0);
    getPropInt(PROP_FLK_OUT_LINE_DATA, &outlinedataFlk, 0);
    int32_t loglevel = (outlinedataFlk<<1) + logFlk;
    logD("prop_log:%d, prop_data:%d, loglevel:%d", logFlk, outlinedataFlk, loglevel);
    FlickerPlatformAdapter::getInstance(m_sensorDev)->setLogLevel(loglevel);

    logD("start(): end. m_sensorDev(%d)", m_sensorDev);

    return MTRUE;
}

MBOOL FlickerHal::stop()
{
    Mutex::Autolock lock(m_lock);

    logI("stop(): begin. m_sensorDev(%d)", m_sensorDev);

    if (detach()) {
        logI("stop(): failed to detach.  m_sensorDev(%d)", m_sensorDev);
        return MFALSE;
    }

    uninit_algo();

    logD("stop(): end. m_sensorDev(%d)", m_sensorDev);

    return MTRUE;
}

/***********************************************************
 * Flicker life cycle - init
 **********************************************************/


/***********************************************************
 * Flicker life cycle - config
 **********************************************************/
MBOOL FlickerHal::getInfo()
{
    logD("getInfo(): m_sensorDev(%d)", m_sensorDev);

    MBOOL flkInfo = MFALSE;
    int disableFlk = 0;

    getPropInt(PROP_FLK_DISABLE, &disableFlk, 0);
    if (disableFlk)
    {
        logI("getInfo(): Flicker disable");
        return MFALSE;
    }
    FlickerPlatformAdapter::getInstance(m_sensorDev)->getInfo(&flkInfo);

    logD("getInfo(): m_sensorDev(%d), flkInfo(%d)", m_sensorDev, flkInfo);
    return flkInfo;
}

int FlickerHal::setTGInfo(int tgInfo, int width, int height)
{
    mTgInfo = tgInfo;
    m_u4TgWReal = width;
    m_u4TgHReal = height;
    FlickerPlatformAdapter::getInstance(m_sensorDev)->setTGInfo(mTgInfo);
    return 0;
}

int FlickerHal::setSensorMode(int sensorMode, int width, int height)
{
    mSensorMode = sensorMode;
    m_u4TgW = width;
    m_u4TgH = height;
    return 0;
}

MINT32 FlickerHal::setWindowInfo(
        MUINT32 *imgW, MUINT32 *imgH,
        MUINT32 *winW, MUINT32 *winH)
{
    logD("setWindowInfo(): begin");
    FlickerPlatformAdapter::getInstance(m_sensorDev)->setWindowInfo(imgW, imgH, winW, winH, m_u4TgW, m_u4TgH);
    int flkHwInfo = getInfo();
    logI("setWindowInfo():flkHwInfo(%d)", flkHwInfo);
    return 0;
}



MINT32 FlickerHal::setWindowInfoReconfig(
        MUINT32 *imgW, MUINT32 *imgH,
        MUINT32 *winW, MUINT32 *winH,
        MUINT32 *tgWidth, MUINT32 *tgHeight,
        MUINT32 *binWidth, MUINT32 *binHeight)
{
    logI("setWindowInfoReconfig(): sensorDev(%d), sensorIdx(%d), tg(%d), img(%d,%d).",
            m_sensorDev, m_sensorIdx, mTgInfo, *binWidth, *binHeight);
    m_u4TgWReal = *tgWidth;
    m_u4TgHReal = *tgHeight;
    m_u4TgW = *binWidth;
    m_u4TgH = *binHeight;
    FlickerPlatformAdapter::getInstance(m_sensorDev)->setWindowInfoReconfig(imgW, imgH, winW, winH, tgWidth, tgHeight, binWidth, binHeight);
    return 0;
}


/***********************************************************
 * Flicker life cycle - start
 **********************************************************/
int FlickerHal::isAttach()
{
    Mutex::Autolock lock(m_attachLock);

    int notAttach = 0;
    getPropInt(PROP_FLK_NOT_ATTACH, &notAttach, 0);
    if (notAttach)
        return 0;

    return m_isAttach;
}

int FlickerHal::attach(FLK_ATTACH_PRIO_T prio)
{
    Mutex::Autolock lock(m_attachLock);

    if (!getInfo()) {
        logI("attach(): no flicker. m_sensorDev(%d)", m_sensorDev);
        return -1;
    }

    if (prio == FLK_ATTACH_PRIO_LOW) {
        logI("attach(): No attach due to low priority sensor. m_sensorDev(%d)", m_sensorDev);
        return -1;
    } else { /* FLK_ATTACH_PRIO_MEDIUM or FLK_ATTACH_PRIO_HIGH */
        if (m_attachCount >= m_maxAttachNum) {
            logI("attach(): achieve max attach number(%d). m_sensorDev(%d)", m_maxAttachNum, m_sensorDev);
            return -1;
        }

        m_attachCount++;
        m_isAttach = 1;
    }

    logI("attach(): isAttach(%d), count(%d/%d). m_sensorDev(%d)",
            m_isAttach, m_attachCount, m_maxAttachNum, m_sensorDev);

    return 0;
}

int FlickerHal::detach()
{
    Mutex::Autolock lock(m_attachLock);

    if (!m_isAttach) {
        logI("detach(): not attach before. m_sensorDev(%d)", m_sensorDev);
        return -1;
    }

    m_attachCount--;
    m_isAttach = 0;
    logI("detach(): count(%d/%d). m_sensorDev(%d)", m_attachCount, m_maxAttachNum, m_sensorDev);

    return 0;
}

static inline int fileGetFlickerPara(FILE *fp)
{
    if (!fp) {
        logE("fileGetFlickerPara(): invalid fp.");
        return -1;
    }

    int para;
    if (fscanf(fp, "%d", &para) > 0) {
        logI("fileGetFlickerPara(): %d.", para);
        return para;
    } else {
        logE("fileGetFlickerPara(): failed to get para.");
        return -1;
    }
}
static int getFlickerParametersFile(
        int sensorMode, FlickerExtPara *flickerExtPara) __attribute__((unused));
static int getFlickerParametersFile(
        int sensorMode, FlickerExtPara *flickerExtPara)
{
    logI("getFlickerParametersFile(): sensorMode(%d).", sensorMode);

    const char *paraFile;
    if (sensorMode == e_sensorModePreview)
        paraFile = FLK_PARA_PREVIEW_FILE;
    else if (sensorMode == e_sensorModeVideo)
        paraFile = FLK_PARA_VIDEO_FILE;
    else if (sensorMode == e_sensorModeCapture)
        paraFile = FLK_PARA_CAPTURE_FILE;
    else if (sensorMode == e_sensorModeVideo1)
        paraFile = FLK_PARA_SLIM_VIDEO1_FILE;
    else if (sensorMode == e_sensorModeVideo2)
        paraFile = FLK_PARA_SLIM_VIDEO2_FILE;
    else if (sensorMode == e_sensorModeCustom1)
        paraFile = FLK_PARA_CUSTOM1_FILE;
    else if (sensorMode == e_sensorModeCustom2)
        paraFile = FLK_PARA_CUSTOM2_FILE;
    else if (sensorMode == e_sensorModeCustom3)
        paraFile = FLK_PARA_CUSTOM3_FILE;
    else if (sensorMode == e_sensorModeCustom4)
        paraFile = FLK_PARA_CUSTOM4_FILE;
    else if (sensorMode == e_sensorModeCustom5)
        paraFile = FLK_PARA_CUSTOM5_FILE;
    else {
        logE("getFlickerParametersFile(): invalid sensor mode.");
        return -1;
    }

    FILE *fp = fopen(paraFile, "r");;
    if (!fp) {
        logE("getFlickerParametersFile(): failed to open para file.");
        return -1;
    }

    flickerExtPara->flicker_freq[0]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_freq[1]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_freq[2]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_freq[3]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_freq[4]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_freq[5]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_freq[6]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_freq[7]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_freq[8]        = fileGetFlickerPara(fp);
    flickerExtPara->flicker_grad_threshold  = fileGetFlickerPara(fp);
    flickerExtPara->flicker_search_range    = fileGetFlickerPara(fp);
    flickerExtPara->min_past_frames         = fileGetFlickerPara(fp);
    flickerExtPara->max_past_frames         = fileGetFlickerPara(fp);
    flickerExtPara->ev50_l50.m            = fileGetFlickerPara(fp);
    flickerExtPara->ev50_l50.b_l          = fileGetFlickerPara(fp);
    flickerExtPara->ev50_l50.b_r          = fileGetFlickerPara(fp);
    flickerExtPara->ev50_l50.offset       = fileGetFlickerPara(fp);
    flickerExtPara->ev50_l60.m            = fileGetFlickerPara(fp);
    flickerExtPara->ev50_l60.b_l          = fileGetFlickerPara(fp);
    flickerExtPara->ev50_l60.b_r          = fileGetFlickerPara(fp);
    flickerExtPara->ev50_l60.offset       = fileGetFlickerPara(fp);
    flickerExtPara->ev60_l50.m            = fileGetFlickerPara(fp);
    flickerExtPara->ev60_l50.b_l          = fileGetFlickerPara(fp);
    flickerExtPara->ev60_l50.b_r          = fileGetFlickerPara(fp);
    flickerExtPara->ev60_l50.offset       = fileGetFlickerPara(fp);
    flickerExtPara->ev60_l60.m            = fileGetFlickerPara(fp);
    flickerExtPara->ev60_l60.b_l          = fileGetFlickerPara(fp);
    flickerExtPara->ev60_l60.b_r          = fileGetFlickerPara(fp);
    flickerExtPara->ev60_l60.offset       = fileGetFlickerPara(fp);
    flickerExtPara->ev50_thresholds[0]    = fileGetFlickerPara(fp);
    flickerExtPara->ev50_thresholds[1]    = fileGetFlickerPara(fp);
    flickerExtPara->ev60_thresholds[0]    = fileGetFlickerPara(fp);
    flickerExtPara->ev60_thresholds[1]    = fileGetFlickerPara(fp);
    flickerExtPara->freq_feature_index[0] = fileGetFlickerPara(fp);
    flickerExtPara->freq_feature_index[1] = fileGetFlickerPara(fp);
    fclose(fp);

    return 0;
}

int FlickerHal::getFlickerParametersAll(
        int sensorModeTotal, int binRatioTotal)
{
    /* get sensor id */
    int sensorDevId = mapSensorIdxToSensorId(m_sensorIdx);
    logI("getFlickerParametersAll(): sensorDevId(%d), m_sensorDev(%d), sensorMode total(%d), binRatio total(%d).",
            sensorDevId, m_sensorDev, sensorModeTotal, binRatioTotal);

    int err = nvGetFlickerParaAll(
            sensorDevId, m_sensorDev, sensorModeTotal, binRatioTotal, &m_flkParaArray);

    // Debug use
    int i=0, j=0;
    for (i=0; i<FLK_MAX_MODE_NUM; i++)
    {
        for (j=0; j<FLK_MAX_BIN_NUM; j++)
        {
            logD("m_flkParaArray[%d][%d].flickerGradThreshold = %d", i, j, m_flkParaArray[i][j].flickerGradThreshold);
            logD("m_flkParaArray[%d][%d].flickerSearchRange = %d", i, j, m_flkParaArray[i][j].flickerSearchRange);
        }

    }

    if (err)
    {
        logE("getFlickerParametersAll(): nvGetFlickerParaAll fail.");
        return MFALSE;
    }

    /* note that, sdcard could be removed */
    return MTRUE;
}

MVOID FlickerHal::getTargetParameter(int sensorMode, int ratio, void *para)
{
    logD("getTargetParameter(): sensorMode(%d), ratio(%d)", sensorMode, ratio);

    int ratioIndex = ratio - 1;
    ((FlickerExtPara*)para)->flicker_freq[0]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[0];
    ((FlickerExtPara*)para)->flicker_freq[1]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[1];
    ((FlickerExtPara*)para)->flicker_freq[2]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[2];
    ((FlickerExtPara*)para)->flicker_freq[3]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[3];
    ((FlickerExtPara*)para)->flicker_freq[4]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[4];
    ((FlickerExtPara*)para)->flicker_freq[5]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[5];
    ((FlickerExtPara*)para)->flicker_freq[6]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[6];
    ((FlickerExtPara*)para)->flicker_freq[7]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[7];
    ((FlickerExtPara*)para)->flicker_freq[8]        = m_flkParaArray[sensorMode][ratioIndex].flickerFreq[8];
    ((FlickerExtPara*)para)->flicker_grad_threshold  = m_flkParaArray[sensorMode][ratioIndex].flickerGradThreshold;
    ((FlickerExtPara*)para)->flicker_search_range    = m_flkParaArray[sensorMode][ratioIndex].flickerSearchRange;
    ((FlickerExtPara*)para)->min_past_frames         = m_flkParaArray[sensorMode][ratioIndex].minPastFrames;
    ((FlickerExtPara*)para)->max_past_frames         = m_flkParaArray[sensorMode][ratioIndex].maxPastFrames;
    ((FlickerExtPara*)para)->ev50_l50.m            = m_flkParaArray[sensorMode][ratioIndex].EV50_L50.m;
    ((FlickerExtPara*)para)->ev50_l50.b_l          = m_flkParaArray[sensorMode][ratioIndex].EV50_L50.b_l;
    ((FlickerExtPara*)para)->ev50_l50.b_r          = m_flkParaArray[sensorMode][ratioIndex].EV50_L50.b_r;
    ((FlickerExtPara*)para)->ev50_l50.offset       = m_flkParaArray[sensorMode][ratioIndex].EV50_L50.offset;
    ((FlickerExtPara*)para)->ev50_l60.m            = m_flkParaArray[sensorMode][ratioIndex].EV50_L60.m;
    ((FlickerExtPara*)para)->ev50_l60.b_l          = m_flkParaArray[sensorMode][ratioIndex].EV50_L60.b_l;
    ((FlickerExtPara*)para)->ev50_l60.b_r          = m_flkParaArray[sensorMode][ratioIndex].EV50_L60.b_r;
    ((FlickerExtPara*)para)->ev50_l60.offset       = m_flkParaArray[sensorMode][ratioIndex].EV50_L60.offset;
    ((FlickerExtPara*)para)->ev60_l50.m            = m_flkParaArray[sensorMode][ratioIndex].EV60_L50.m;
    ((FlickerExtPara*)para)->ev60_l50.b_l          = m_flkParaArray[sensorMode][ratioIndex].EV60_L50.b_l;
    ((FlickerExtPara*)para)->ev60_l50.b_r          = m_flkParaArray[sensorMode][ratioIndex].EV60_L50.b_r;
    ((FlickerExtPara*)para)->ev60_l50.offset       = m_flkParaArray[sensorMode][ratioIndex].EV60_L50.offset;
    ((FlickerExtPara*)para)->ev60_l60.m            = m_flkParaArray[sensorMode][ratioIndex].EV60_L60.m;
    ((FlickerExtPara*)para)->ev60_l60.b_l          = m_flkParaArray[sensorMode][ratioIndex].EV60_L60.b_l;
    ((FlickerExtPara*)para)->ev60_l60.b_r          = m_flkParaArray[sensorMode][ratioIndex].EV60_L60.b_r;
    ((FlickerExtPara*)para)->ev60_l60.offset       = m_flkParaArray[sensorMode][ratioIndex].EV60_L60.offset;
    ((FlickerExtPara*)para)->ev50_thresholds[0]    = m_flkParaArray[sensorMode][ratioIndex].EV50_thresholds[0];
    ((FlickerExtPara*)para)->ev50_thresholds[1]    = m_flkParaArray[sensorMode][ratioIndex].EV50_thresholds[1];
    ((FlickerExtPara*)para)->ev60_thresholds[0]    = m_flkParaArray[sensorMode][ratioIndex].EV60_thresholds[0];
    ((FlickerExtPara*)para)->ev60_thresholds[1]    = m_flkParaArray[sensorMode][ratioIndex].EV60_thresholds[1];
    ((FlickerExtPara*)para)->freq_feature_index[0] = m_flkParaArray[sensorMode][ratioIndex].freq_feature_index[0];
    ((FlickerExtPara*)para)->freq_feature_index[1] = m_flkParaArray[sensorMode][ratioIndex].freq_feature_index[1];

    logD("getTargetParameter(): para->flickerFreq[0] = %d", ((FlickerExtPara*)para)->flicker_freq[0]);
    logD("getTargetParameter(): para->flickerFreq[1] = %d", ((FlickerExtPara*)para)->flicker_freq[1]);
    logD("getTargetParameter(): para->flickerFreq[2] = %d", ((FlickerExtPara*)para)->flicker_freq[2]);
    logD("getTargetParameter(): para->flickerFreq[3] = %d", ((FlickerExtPara*)para)->flicker_freq[3]);
    logD("getTargetParameter(): para->flickerFreq[4] = %d", ((FlickerExtPara*)para)->flicker_freq[4]);
    logD("getTargetParameter(): para->flickerFreq[5] = %d", ((FlickerExtPara*)para)->flicker_freq[5]);
    logD("getTargetParameter(): para->flickerFreq[6] = %d", ((FlickerExtPara*)para)->flicker_freq[6]);
    logD("getTargetParameter(): para->flickerFreq[7] = %d", ((FlickerExtPara*)para)->flicker_freq[7]);
    logD("getTargetParameter(): para->flickerFreq[8] = %d", ((FlickerExtPara*)para)->flicker_freq[8]);
    logD("getTargetParameter(): para->flickerGradThreshold = %d", ((FlickerExtPara*)para)->flicker_grad_threshold);
    logD("getTargetParameter(): para->flickerSearchRange = %d", ((FlickerExtPara*)para)->flicker_search_range);
    logD("getTargetParameter(): para->minPastFrames = %d", ((FlickerExtPara*)para)->min_past_frames);
    logD("getTargetParameter(): para->maxPastFrames = %d", ((FlickerExtPara*)para)->max_past_frames);
    logD("getTargetParameter(): para->EV50_L50.m = %d", ((FlickerExtPara*)para)->ev50_l50.m);
    logD("getTargetParameter(): para->EV50_L50.b_l = %d", ((FlickerExtPara*)para)->ev50_l50.b_l);
    logD("getTargetParameter(): para->EV50_L50.b_r = %d", ((FlickerExtPara*)para)->ev50_l50.b_r);
    logD("getTargetParameter(): para->EV50_L50.offset = %d", ((FlickerExtPara*)para)->ev50_l50.offset);
    logD("getTargetParameter(): para->EV50_L60.m = %d", ((FlickerExtPara*)para)->ev50_l60.m);
    logD("getTargetParameter(): para->EV50_L60.b_l = %d", ((FlickerExtPara*)para)->ev50_l60.b_l);
    logD("getTargetParameter(): para->EV50_L60.b_r = %d", ((FlickerExtPara*)para)->ev50_l60.b_r);
    logD("getTargetParameter(): para->EV50_L60.offset = %d", ((FlickerExtPara*)para)->ev50_l60.offset);
    logD("getTargetParameter(): para->EV60_L50.m = %d", ((FlickerExtPara*)para)->ev60_l50.m);
    logD("getTargetParameter(): para->EV60_L50.b_l = %d", ((FlickerExtPara*)para)->ev60_l50.b_l);
    logD("getTargetParameter(): para->EV60_L50.b_r = %d", ((FlickerExtPara*)para)->ev60_l50.b_r);
    logD("getTargetParameter(): para->EV60_L50.offset = %d", ((FlickerExtPara*)para)->ev60_l50.offset);
    logD("getTargetParameter(): para->EV60_L60.m = %d", ((FlickerExtPara*)para)->ev60_l60.m);
    logD("getTargetParameter(): para->EV60_L60.b_l = %d", ((FlickerExtPara*)para)->ev60_l60.b_l);
    logD("getTargetParameter(): para->EV60_L60.b_r = %d", ((FlickerExtPara*)para)->ev60_l60.b_r);
    logD("getTargetParameter(): para->EV60_L60.offset = %d", ((FlickerExtPara*)para)->ev60_l60.offset);
    logD("getTargetParameter(): para->EV50_thresholds[0] = %d", ((FlickerExtPara*)para)->ev50_thresholds[0]);
    logD("getTargetParameter(): para->EV50_thresholds[1] = %d", ((FlickerExtPara*)para)->ev50_thresholds[1]);
    logD("getTargetParameter(): para->EV60_thresholds[0] = %d", ((FlickerExtPara*)para)->ev60_thresholds[0]);
    logD("getTargetParameter(): para->EV60_thresholds[1] = %d", ((FlickerExtPara*)para)->ev60_thresholds[1]);
    logD("getTargetParameter(): para->freq_feature_index[0] = %d", ((FlickerExtPara*)para)->freq_feature_index[0]);
    logD("getTargetParameter(): para->freq_feature_index[1] = %d", ((FlickerExtPara*)para)->freq_feature_index[1]);
}


MINT32 FlickerHal::init_algo(int sensorType)
{
    logD("init_algo(): begin. mUsers(%d), m_sensorDev(%d)", mUsers, m_sensorDev);

    /* get flicker init hz from custom */
    int initFreq = 0;
    cust_getFlickerHalPara(&initFreq, &m_maxDetExpUs);

    /* get flicker init hz from the last result */
    if (m_flickerState)
        initFreq = m_flickerState;

    /* get flicker init hz from property */
    int propDefFlicker = 0;
    getPropInt(PROP_FLK_DEF_HZ, &propDefFlicker, 0);
    if (propDefFlicker)
        initFreq = propDefFlicker;

    /* set flicker init hz  */
    logD("init_algo(): init flicker Hz(%d).", initFreq);
    setFlickerState(initFreq);

    if (mUsers > 0) {
        logE("init_algo(): mUsers(%d) had created. m_sensorDev(%d)", mUsers, m_sensorDev);
    }
    else
    {
        android_atomic_inc(&mUsers);
    }

    /* acquire sensor list */
    IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
    IHalSensor* pHalSensorObj;
    if(!pIHalSensorList)
    {
        logE("init_algo(): failed to MAKE_HalSensorList().");
        return -1;
    }
    pHalSensorObj = pIHalSensorList->createSensor("flicker", m_sensorIdx);
    if (!pHalSensorObj) {
        logE("init_algo(): failed to create instance for pHalSensorObj.");
        return -1;
    }

    /* get pixel clock */
    MINT32 err = 0;
    err = pHalSensorObj->sendCommand(m_sensorDev,
            SENSOR_CMD_GET_PIXEL_CLOCK_FREQ,
            (MUINTPTR)&m_u4SensorPixelClkFreq, 0, 0);
    if (err)
        logE("init_algo(): failed to get pixel clock.");

    /* get line length */
    err = pHalSensorObj->sendCommand(m_sensorDev,
            SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM,
            (MUINTPTR)&m_u4PixelsInLine, 0, 0);
    if (err)
        logE("init_algo(): failed to get line length.");
    m_u4PixelsInLine &= 0x0000FFFF;

    /* release sensor list */
    if (pHalSensorObj)
        pHalSensorObj->destroyInstance("flicker");

    logD("init_algo(): pixel clock(%d), line length(%d), grab height(%d).",
            (int)m_u4SensorPixelClkFreq,
            (int)m_u4PixelsInLine,
            (int)m_imgH);

   /* check binning */
    int ratio = m_u4TgHReal / m_u4TgH;
    getPropInt(PROP_FLK_RATIO, &ratio, ratio);
    if (ratio > 1) {
        m_u4PixelsInLine *= ratio;
        logD("init_algo(): update bin ratio(%d), line length(%u).", ratio, m_u4PixelsInLine);
    }

//    android_atomic_inc(&mUsers);

    /* get sensor mode */
    int sensorMode = 0;
    FlickerPlatformAdapter::getInstance(m_sensorDev)->getSensorMode(mSensorMode, sensorType, &sensorMode);
    logD("init_algo(): sensorDev(%d), sensorIdx(%d), sensorMode(%d), ratio(%d).",
            m_sensorDev, m_sensorIdx, sensorMode, ratio);

    /* get parameters and set to algo */
    FlickerExtPara para = {};
    getTargetParameter(sensorMode, ratio, (void*)&para);

    int propFlkReadFile = 0;
    getPropInt(PROP_FLK_READ_FILE, &propFlkReadFile, 0);
    if (propFlkReadFile)
        getFlickerParametersFile(mSensorMode, &para);

    FlickerPlatformAdapter::getInstance(m_sensorDev)->setExtPara(para);
    /* get pixel line, height, width and pixel clock from property */
    getPropInt(PROP_FLK_CLK, (int *)&m_u4SensorPixelClkFreq, m_u4SensorPixelClkFreq);
    getPropInt(PROP_FLK_PIXEL, (int *)&m_u4PixelsInLine, m_u4PixelsInLine);
    getPropInt(RPOP_FLK_W, (int *)&m_winW, m_winW);
    getPropInt(RPOP_FLK_H, (int *)&m_winH, m_winH);

    /* init flicker algo */
    logD("init_algo(): flicker_init(%u,%u,%u,%u) +.", m_u4PixelsInLine,
            m_winH * 3, m_winW, m_u4SensorPixelClkFreq);
    long int t = getMs();

    FlickerPlatformAdapter::getInstance(m_sensorDev)->init_algo(m_u4PixelsInLine,m_winH,m_winW,m_u4SensorPixelClkFreq,sensorType);

    logI("init_algo(): sensorDev(%d) sensorIdx(%d) sensorMode(%d) sensorType(%d) initFreq(%d) ratio(%d) clkFreq(%d) pixelsInLine(%d) imgH(%d) winH(%d) winW(%d) mUsers(%d) period(%ld ms)",
        m_sensorDev, m_sensorIdx, sensorMode, sensorType, initFreq, ratio,
        m_u4SensorPixelClkFreq, m_u4PixelsInLine, m_imgH, m_winH, m_winW, mUsers,
        getMs()-t);
    return err;
}

MINT32 FlickerHal::uninit_algo()
{
    logI("uninit_algo(): mUsers(%d). m_sensorDev(%d)", mUsers, m_sensorDev);

    if (mUsers <= 0)
        return 0;

    android_atomic_dec(&mUsers);
    if (!mUsers)
    {
        FlickerPlatformAdapter::getInstance(m_sensorDev)->uninit_algo();
    }

    return 0;
}

/***********************************************************
 * Flicker oprations
 **********************************************************/
int FlickerHal::update(int sensorType, FlickerInput *in, FlickerOutput *out)
{
    Mutex::Autolock lock(m_lock);

    if (!m_isAttach) {
        logD("update(): sensorDev(%d) skip, not attached.", m_sensorDev);
        return -1;
    }

    if (m_currentFrame++ < 2) {
        logI("update(): skip, not enough frame(%d).", m_currentFrame);
        return -1;
    }

    /* analyze flicker */
    if (m_flickerMode == MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO) {
        /*
         * cycle = 0, copy data.
         * cycle = 1, copy data and do flicker detection.
         * other, do nothing.
         */
        int ret = 0;
        if (m_currentCycle < 2) {
            logD("update(): analyze with sensorDev(%d), exposure time(%d), mVa(%p), cycle(%d).",
                    m_sensorDev, in->aeExpTime, in->pBuf, m_currentCycle);
            ret = analyzeFlickerFrequency(sensorType, in->aeExpTime, in->pBuf);
        }
        m_currentCycle = (m_currentCycle + 1) % m_detectCycle;

        if (ret) {
            logE("update(): failed to analyze(%x).", ret);
            return ret;
        }

    }
    out->flickerResult = m_flickerState;

    return 0;
}

MINT32 FlickerHal::analyzeFlickerFrequency(int sensorType, int exp, MVOID *buf)
{
    int32_t dataLen = 0;
    int32_t imgW = 0;
    int32_t imgH = 0;

    int32_t ret = FlickerPlatformAdapter::getInstance(m_sensorDev)->setBuf(sensorType, buf, m_currentCycle, &dataLen, &imgW, &imgH);
    if(ret == -1)
        return ret;
    /* only copy buffer at cycle 1 */
    if (m_currentCycle == 0)
        return 0;

    /* copy buffer and analyze at cycle 2 */
    int32_t winW = 0;
    int32_t winH = 0;
    FlickerPlatformAdapter::getInstance(m_sensorDev)->getWindowInfo(m_imgW, m_imgH, &winW, &winH);

    /* detect flicker */
    if (exp < m_maxDetExpUs &&
            ((exp > 8200 && m_flickerState == HAL_FLICKER_AUTO_60HZ ) ||
             (exp > 9800 && m_flickerState == HAL_FLICKER_AUTO_50HZ ))) {
        long int t = getMs();
        int32_t algoStatus = 2;//FK100
        algoStatus = FlickerPlatformAdapter::getInstance(m_sensorDev)->detectFlicker(sensorType, m_flickerState, exp);

        logD("analyzeFlickerFrequency(): detect period(%ld ms) -.", getMs() - t);

        int propFlickerSwitch = -1;
        getPropInt(PROP_FLK_SWITCH_EN, &propFlickerSwitch, -1);
        if (!propFlickerSwitch)
            logI("flicker state not changed due to property fixed");
        else {
            if (algoStatus == FK100 &&
                    m_flickerState == HAL_FLICKER_AUTO_60HZ)
                setFlickerState(HAL_FLICKER_AUTO_50HZ);
            else if (algoStatus == FK120 &&
                    m_flickerState == HAL_FLICKER_AUTO_50HZ)
                setFlickerState(HAL_FLICKER_AUTO_60HZ);
        }
    } else if (exp >= m_maxDetExpUs)
        logI("analyzeFlickerFrequency(): skip, exp is too long(%d).", exp);
    else
        logI("analyzeFlickerFrequency(): skip, exp is too short(%d).", exp);

    logD("analyzeFlickerFrequency(): flicker status(%d). m_sensorDev(%d)", m_flickerState, m_sensorDev);

    return 0;
}

int FlickerHal::getFlickerState(int &state)
{
    Mutex::Autolock lock(m_lock);
    logD("getFlickerState(): state(%d).", m_flickerState);
    state = m_flickerState;
    return 0;
}

int FlickerHal::setFlickerState(int state)
{
    logD("setFlickerState(): %d.", state);

    if (m_flickerState == state)
        return 0;

    logI("setFlickerState(): state change(%d->%d).",
            m_flickerState, state);

    m_flickerState = state;

    /* must be called every time we change the flicker table */
    FlickerPlatformAdapter::getInstance(m_sensorDev)->setFlickerState(m_flickerState);
    FlickerPlatformAdapter::getInstance(m_sensorDev)->resetQueue();

    return 0;
}

int FlickerHal::setFlickerMode(int mode)
{
    Mutex::Autolock lock(m_lock);

    logD("setFlickerMode(): %d.", mode);

    if (mode != m_flickerMode) {
        logI("setFlickerMode(): mode change(%d->%d).",
                m_flickerMode, mode);
        if (mode == MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO)
            IAeMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 1);
        else if (m_flickerMode == MTK_CONTROL_AE_ANTIBANDING_MODE_AUTO)
            IAeMgr::getInstance().setFlickerFrameRateActive(m_sensorDev, 0);

        m_flickerMode = mode;
    }

    /* set flicker state */
    if (m_flickerMode == MTK_CONTROL_AE_ANTIBANDING_MODE_60HZ)
        setFlickerState(HAL_FLICKER_AUTO_60HZ);
    else if (m_flickerMode == MTK_CONTROL_AE_ANTIBANDING_MODE_50HZ)
        setFlickerState(HAL_FLICKER_AUTO_50HZ);

    return 0;
}

