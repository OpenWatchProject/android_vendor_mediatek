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

#define LOG_TAG "FlickerPlatformAdapter"
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
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <private/IopipeUtils.h>

/* custom headers */
#include <aaa_error_code.h>

/* aaa common headers */
#include "file_utils.h"
#include "property_utils.h"
#include "time_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* flicker headers */
#include "flicker_platform_adapter.h"
#include "flicker_utils.h"
#include <aaa_hal_sttCtrl.h>

#include <mtkcam/utils/std/Trace.h>

using namespace NS3Av3;

/***********************************************************
 * Define macros
 **********************************************************/
#define FLICKER_MAX_LENG (6000)
#define NUM_X (6)

#define MHAL_FLICKER_WORKING_BUF_SIZE (FLICKER_MAX_LENG * sizeof(int64_t) * NUM_X)
#define DATA_LENGTH_3EXPO_PREVIEW (67*3)
#define DATA_LENGTH_3EXPO_VIDEO (50*3)

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

#define FLK_DUMP_COUNT_FILE "/data/vendor/flicker/flko/flicker_file_cnt.txt"

/***********************************************************
 * Flicker HAL
 **********************************************************/

FlickerPlatformAdapter::FlickerPlatformAdapter(int32_t sensorDev)
    : m_sensorDev(sensorDev)
    , m_sensorId(0)
    , m_sensorMode(SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
    , mTgInfo(NSCam::CAM_TG_NONE)
    , m_pVectorData1(NULL)
    , m_pVectorData2(NULL)
    , m_imgW(0)
    , m_imgH(0)
    , m_winW(0)
    , m_winH(0)
    , m_dataLen(0)
    , m_u4SensorPixelClkFreq(0)
    , m_u4PixelsInLine(0)
    , m_currentCycle(0)
    , m_sensorIdx(0)
{

}

FlickerPlatformAdapter::~FlickerPlatformAdapter()
{

}

#define MY_INST NS3Av3::INST_T<FlickerPlatformAdapter>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

FlickerPlatformAdapter *FlickerPlatformAdapter::getInstance(int32_t sensorDev)
{
    int sensorOpenIndex = NS3Av3::mapSensorDevToIdx(sensorDev);
    if(sensorOpenIndex >= SENSOR_IDX_MAX || sensorOpenIndex < 0) {
        logE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, sensorOpenIndex);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[sensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<FlickerPlatformAdapter>(sensorDev);
    } );

    return rSingleton.instance.get();
}

int32_t FlickerPlatformAdapter::init(int32_t const i4SensorIdx)
{
    setDebug();

    m_sensorIdx = i4SensorIdx;
    m_sensorId = NS3Av3::mapSensorIdxToSensorId(m_sensorIdx);
    logD("SensorIdx(%d), SensorId(%d)", m_sensorIdx, m_sensorId);
    createBuf();

    return 0;
}

int32_t FlickerPlatformAdapter::uninit()
{
    releaseBuf();

    return 0;
}

int32_t FlickerPlatformAdapter::createBuf()
{
    logD("createBuf().");

    if (m_pVectorData1 || m_pVectorData2)
        logE("createBuf(): buffer is not empty.");

    m_pVectorData1 = (int64_t *)malloc(MHAL_FLICKER_WORKING_BUF_SIZE);

    if (!m_pVectorData1) {
        logE("createBuf(): falied to allocate buffer 1.");
        return -1;
    }

    m_pVectorData2 = (int64_t *)malloc(MHAL_FLICKER_WORKING_BUF_SIZE);

    if (!m_pVectorData2) {
        logE("createBuf(): falied to allocate buffer 2.");
        free(m_pVectorData1);
        return -1;
    }

    logI("createBuf(). Create buf success.");

    return 0;
}

MVOID FlickerPlatformAdapter::releaseBuf()
{
    logD("releaseBuf().");

    if (m_pVectorData1) {
        free(m_pVectorData1);
        m_pVectorData1 = NULL;
    }
    if (m_pVectorData2) {
        free(m_pVectorData2);
        m_pVectorData2 = NULL;
    }

    logI("releaseBuf(). Release buf success.");
}

int32_t FlickerPlatformAdapter::getSensorType(int32_t hdrType,int32_t *sensorType)
{
    if(hdrType == FEATURE_MVHDR_SUPPORT_3EXPO_VIRTUAL_CHANNEL)
    {
        *sensorType = FLK_SENSOR_TYPE_3EXPO;
    }
    else
    {
        *sensorType = FLK_SENSOR_TYPE_NORMAL;
    }
    return 0;
}

int32_t FlickerPlatformAdapter::getInfo(MBOOL *flkInfo)
{

    /* get flicker info from normal pipe */
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* pCamIO =
        (NSCam::NSIoPipe::NSCamIOPipe::INormalPipe *)
        INormalPipeUtils::get()->createDefaultNormalPipe(m_sensorIdx, LOG_TAG);
    if (!pCamIO) {
        logE("getInfo(): failed to create normal pipe.");
        return MFALSE;
    }

    pCamIO->sendCommand(
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_FLK_INFO,
            (MINTPTR)flkInfo, 0, 0);
    pCamIO->destroyInstance(LOG_TAG);

    if (*flkInfo) {
        m_sFLKResultConfig.enableFLKHw = 1;
    } else {
        m_sFLKResultConfig.enableFLKHw = 0;
    }

    return 0;
}

int32_t FlickerPlatformAdapter::setTGInfo(int32_t tgInfo)
{
    mTgInfo = tgInfo;
    return 0;
}

int32_t FlickerPlatformAdapter::setWindowInfo(MUINT32 *imgW, MUINT32 *imgH, MUINT32 *winW, MUINT32 *winH, MUINT32 u4TgW, MUINT32 u4TgH)
{
    logD("setWindowInfo(): begin");
    FLKWinCFG_T flkWinCfg;
    prepareFlickerCfg(u4TgW, u4TgH, &flkWinCfg);
    FLKConfig(&flkWinCfg, (MVOID *)&m_sFLKResultConfig);

    logI("setWindowInfo(): sensorDev(%d) sensorId(%d) tg(%d) tgImg(%d,%d) configImg(%d,%d) offset(%d,%d) win(%d,%d) dma(%d)",
            m_sensorDev, m_sensorId, mTgInfo, u4TgW, u4TgH,
            flkWinCfg.m_uImageW, flkWinCfg.m_uImageH,
            flkWinCfg.m_u4OffsetX, flkWinCfg.m_u4OffsetY,
            flkWinCfg.m_u4SizeX, flkWinCfg.m_u4SizeY, flkWinCfg.m_u4DmaSize);

    /* update size */
    *imgW = u4TgW - 3;
    *imgH = u4TgH - 25;
    *winW = flkWinCfg.m_u4SizeX;
    *winH = flkWinCfg.m_u4SizeY;

    m_imgW = *imgW;
    m_imgH = *imgH;

    return 0;
}

int32_t FlickerPlatformAdapter::setWindowInfoReconfig(
        uint32_t *imgW, uint32_t *imgH,
        uint32_t *winW, uint32_t *winH,
        uint32_t *tgWidth, uint32_t *tgHeight,
        uint32_t *binWidth, uint32_t *binHeight)
{
    logI("setWindowInfoReconfig(): sensorDev(%d), sensorId(%d), tg(%d), img(%d,%d).",
            m_sensorDev, m_sensorId, mTgInfo, *binWidth, *binHeight);

    FLKWinCFG_T flkWinCfg;
    prepareFlickerCfg(*binWidth, *binHeight, &flkWinCfg);
    FLKConfig(&flkWinCfg, (MVOID *)&m_sFLKResultConfig);

    logI("setWindowInfoReconfig(): config img(%d,%d), offset(%d,%d), win(%d,%d), dma(%d).",
            flkWinCfg.m_uImageW, flkWinCfg.m_uImageH,
            flkWinCfg.m_u4OffsetX, flkWinCfg.m_u4OffsetY,
            flkWinCfg.m_u4SizeX, flkWinCfg.m_u4SizeY, flkWinCfg.m_u4DmaSize);

    // update size
    *imgW = *binWidth - 3;
    *imgH = *binHeight - 25;
    *winW = flkWinCfg.m_u4SizeX;
    *winH = flkWinCfg.m_u4SizeY;

    m_imgW = *imgW;
    m_imgH = *imgH;

    return 0;
}


int32_t FlickerPlatformAdapter::getSensorMode(int32_t sensorScenario,int32_t sensorType, int32_t *sensorMode)
{
    if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModePreview_3HDR;
        else
            *sensorMode = e_sensorModePreview;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeCapture_3HDR;
        else
            *sensorMode = e_sensorModeCapture;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_NORMAL_VIDEO)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeVideoPreview_3HDR;
        else
            *sensorMode = e_sensorModeVideoPreview;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_SLIM_VIDEO1)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeVideo1_3HDR;
        else
            *sensorMode = e_sensorModeVideo1;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_SLIM_VIDEO2)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeVideo2_3HDR;
        else
            *sensorMode = e_sensorModeVideo2;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_CUSTOM1)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeCustom1_3HDR;
        else
            *sensorMode = e_sensorModeCustom1;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_CUSTOM2)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeCustom2_3HDR;
        else
            *sensorMode = e_sensorModeCustom2;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_CUSTOM3)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeCustom3_3HDR;
        else
            *sensorMode = e_sensorModeCustom3;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_CUSTOM4)
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeCustom4_3HDR;
        else
            *sensorMode = e_sensorModeCustom4;
    }
    else /* if (sensorScenario == SENSOR_SCENARIO_ID_CUSTOM5) */
    {
        if (sensorType == FLK_SENSOR_TYPE_3EXPO)
            *sensorMode = e_sensorModeCustom5_3HDR;
        else
            *sensorMode = e_sensorModeCustom5;
    }

    m_sensorMode = *sensorMode;

    return 0;
}

MVOID FlickerPlatformAdapter::prepareFlickerCfg(int32_t width,int32_t height,FLKWinCFG_T * flkWinCfg)
{
    int32_t u4ToleranceLine = 20;
    int32_t imageW = width - 3;
    int32_t imageH = height - 25;
    if (imageH > FLICKER_MAX_LENG - 6)
        imageH = FLICKER_MAX_LENG - 6;

    flkWinCfg->m_uImageW = imageW;
    flkWinCfg->m_uImageH = imageH;
    flkWinCfg->m_u4NumX = NUM_X;
    flkWinCfg->m_u4NumY = 3;
    flkWinCfg->m_u4OffsetX = 0;
    flkWinCfg->m_u4OffsetY = 0 + u4ToleranceLine;
    flkWinCfg->m_u4SizeX = ((imageW - flkWinCfg->m_u4OffsetX) / (NUM_X * 2)) * 2;
    flkWinCfg->m_u4SizeY = ((imageH - flkWinCfg->m_u4OffsetY + u4ToleranceLine) / 6) * 2;
    flkWinCfg->m_u4DmaSize =
        flkWinCfg->m_u4NumX * flkWinCfg->m_u4NumY * flkWinCfg->m_u4SizeY * 6;

    flkWinCfg->m_u4SGG3_PGN = 16;
    flkWinCfg->m_u4SGG3_GMR1 = 5140;
    flkWinCfg->m_u4SGG3_GMR2 = 7453;
    flkWinCfg->m_u4SGG3_GMR3 = 10794;
    flkWinCfg->m_u4SGG3_GMR4 = 15934;
    flkWinCfg->m_u4SGG3_GMR5 = 22616;
    flkWinCfg->m_u4SGG3_GMR6 = 32382;
    flkWinCfg->m_u4SGG3_GMR7 = 46260;
    flkWinCfg->m_u4INPUT_BIT_SEL = 4;   // 16bits
    flkWinCfg->m_u4ZHDR_NOISE_VAL = 2630;
    flkWinCfg->m_u4SGG_OUT_MAX_VAL = 65535;


    logD("prepareFlickerCfg(): flkWinCfg->m_uImageW = %d.", flkWinCfg->m_uImageW);
    logD("prepareFlickerCfg(): flkWinCfg->m_uImageH = %d.", flkWinCfg->m_uImageH);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4NumX = %d.", flkWinCfg->m_u4NumX);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4NumY = %d.", flkWinCfg->m_u4NumY);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4OffsetX = %d.", flkWinCfg->m_u4OffsetX);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4OffsetY = %d.", flkWinCfg->m_u4OffsetY);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SizeX = %d.", flkWinCfg->m_u4SizeX);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SizeY = %d.", flkWinCfg->m_u4SizeY);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4DmaSize = %d.", flkWinCfg->m_u4DmaSize);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG3_PGN = %d.", flkWinCfg->m_u4SGG3_PGN);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG3_GMR1 = %d.", flkWinCfg->m_u4SGG3_GMR1);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG3_GMR2 = %d.", flkWinCfg->m_u4SGG3_GMR2);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG3_GMR3 = %d.", flkWinCfg->m_u4SGG3_GMR3);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG3_GMR4 = %d.", flkWinCfg->m_u4SGG3_GMR4);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG3_GMR5 = %d.", flkWinCfg->m_u4SGG3_GMR5);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG3_GMR6 = %d.", flkWinCfg->m_u4SGG3_GMR6);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG3_GMR7 = %d.", flkWinCfg->m_u4SGG3_GMR7);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4INPUT_BIT_SEL = %d.", flkWinCfg->m_u4INPUT_BIT_SEL);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4ZHDR_NOISE_VAL = %d.", flkWinCfg->m_u4ZHDR_NOISE_VAL);
    logD("prepareFlickerCfg(): flkWinCfg->m_u4SGG_OUT_MAX_VAL = %d.", flkWinCfg->m_u4SGG_OUT_MAX_VAL);

}

int32_t FlickerPlatformAdapter::getFLKResultConfig(MVOID * p_sFLKResultConfig)
{
    logD("getFLKResultCongif(): configNum=%d",m_sFLKResultConfig.configNum);
    memcpy((FLKResultConfig_T *)p_sFLKResultConfig, &m_sFLKResultConfig, sizeof(FLKResultConfig_T));
    return 0;
}

int32_t FlickerPlatformAdapter::init_algo(uint32_t pixelsLine,uint32_t winH,uint32_t winW,uint32_t pixelClkFreq,int32_t sensorType)
{
    m_u4SensorPixelClkFreq = pixelClkFreq;
    m_u4PixelsInLine = pixelsLine;
    FlkInitPara initPara;
    if (sensorType == FLK_SENSOR_TYPE_3EXPO)
    {
        if(m_sensorId == 0x0576)
        {
            initPara.type = IMX576;
            initPara.line_length = pixelsLine*22;
            initPara.column_length = 96;
            initPara.window_width = 2;
            initPara.pclk = pixelClkFreq;
            initPara.block_height = 0;
            initPara.green_only = 0;
            logD("[%s] flicker_init_SONY", __FUNCTION__);
        }
        else if(m_sensorId == 0x2185)
        {
            initPara.type = S5K2X5;
            initPara.line_length = pixelsLine*8;
            initPara.column_length = (m_sensorMode == e_sensorModePreview_3HDR) ? 67:50;
            initPara.window_width = 720;
            initPara.pclk = pixelClkFreq;
            initPara.block_height = 8;
            initPara.green_only = 0;
        }
    }
    else
    {
        initPara.type = NORMAL;
        initPara.line_length = pixelsLine;
        initPara.column_length = winH*3;
        initPara.window_width = winW;
        initPara.pclk = pixelClkFreq;
        initPara.block_height = 0;
        initPara.green_only = 0;
        logD("[%s] flicker_init", __FUNCTION__);
    }

    SendCmdFlk(CMD_INIT, (void*)&initPara, NULL); // algo

    return 0;
}

int32_t FlickerPlatformAdapter::uninit_algo()
{
    SendCmdFlk(CMD_UNINIT, NULL, NULL); // algo
    return 0;
}

int32_t FlickerPlatformAdapter::setExtPara(FlickerExtPara para)
{
    FlkExtPara extPara;
    extPara.flicker_freq[0]        = para.flicker_freq[0];
    extPara.flicker_freq[1]        = para.flicker_freq[1];
    extPara.flicker_freq[2]        = para.flicker_freq[2];
    extPara.flicker_freq[3]        = para.flicker_freq[3];
    extPara.flicker_freq[4]        = para.flicker_freq[4];
    extPara.flicker_freq[5]        = para.flicker_freq[5];
    extPara.flicker_freq[6]        = para.flicker_freq[6];
    extPara.flicker_freq[7]        = para.flicker_freq[7];
    extPara.flicker_freq[8]        = para.flicker_freq[8];
    extPara.flicker_grad_threshold = para.flicker_grad_threshold;
    extPara.flicker_search_range   = para.flicker_search_range;
    extPara.min_past_frames        = para.min_past_frames;
    extPara.max_past_frames        = para.max_past_frames;
    extPara.ev50_l50.m             = para.ev50_l50.m;
    extPara.ev50_l50.b_l           = para.ev50_l50.b_l;
    extPara.ev50_l50.b_r           = para.ev50_l50.b_r;
    extPara.ev50_l50.offset        = para.ev50_l50.offset;
    extPara.ev50_l60.m             = para.ev50_l60.m;
    extPara.ev50_l60.b_l           = para.ev50_l60.b_l;
    extPara.ev50_l60.b_r           = para.ev50_l60.b_r;
    extPara.ev50_l60.offset        = para.ev50_l60.offset;
    extPara.ev60_l50.m             = para.ev60_l50.m;
    extPara.ev60_l50.b_l           = para.ev60_l50.b_l;
    extPara.ev60_l50.b_r           = para.ev60_l50.b_r;
    extPara.ev60_l50.offset        = para.ev60_l50.offset;
    extPara.ev60_l60.m             = para.ev60_l60.m;
    extPara.ev60_l60.b_l           = para.ev60_l60.b_l;
    extPara.ev60_l60.b_r           = para.ev60_l60.b_r;
    extPara.ev60_l60.offset        = para.ev60_l60.offset;
    extPara.ev50_thresholds[0]     = para.ev50_thresholds[0];
    extPara.ev50_thresholds[1]     = para.ev50_thresholds[1];
    extPara.ev60_thresholds[0]     = para.ev60_thresholds[0];
    extPara.ev60_thresholds[1]     = para.ev60_thresholds[1];
    extPara.freq_feature_index[0] = para.freq_feature_index[0];
    extPara.freq_feature_index[1] = para.freq_feature_index[1];
    extPara.is_zhdr = 0;
    extPara.zhdr_se_small= 0;
    extPara.zhdr_gle_first = 0;

    SendCmdFlk(CMD_SET_EXT_PARA, (void *)&extPara, NULL);//algo

    return 0;
}

int32_t FlickerPlatformAdapter::setBuf(int32_t sensorType, MVOID * buf, int32_t currentCycle, int32_t *dataLen, int32_t *imgW, int32_t *imgH)
{
    int64_t *flickerBuf1;
    int64_t *flickerBuf2;

    if (!m_pVectorData1 || !m_pVectorData2)
    {
        logI("m_pVectorData1 or m_pVectorData2 is NULL.");
        return -1;
    }


    if (!currentCycle) {
        flickerBuf1 = m_pVectorData1;
        flickerBuf2 = m_pVectorData2;
    } else {
        flickerBuf1 = m_pVectorData2;
        flickerBuf2 = m_pVectorData1;
    }
    m_currentCycle = currentCycle;

    *dataLen = 0;
    *imgW = m_imgW;

    if (m_imgH > FLICKER_MAX_LENG) {
        *dataLen = NUM_X * FLICKER_MAX_LENG / 2;
        *imgH = FLICKER_MAX_LENG;
    } else {
        *dataLen = NUM_X * m_imgH / 2;
        *imgH = m_imgH;
    }

    if (sensorType == FLK_SENSOR_TYPE_3EXPO)
    {
        if(m_sensorId == 0x0576)
        {
            *dataLen = 4*96/2;
            int32_t *flickerDMAAdr = (int32_t *)buf;
            for (int32_t i = 0; i < *dataLen; i++)
            {
                flickerBuf1[2*i] = flickerDMAAdr[2*i];
                flickerBuf1[2*i+1] = flickerDMAAdr[2*i+1];
            }
        }
        else if(m_sensorId == 0x2185)
        {
            if (m_sensorMode == e_sensorModeVideoPreview_3HDR)
            {
                *dataLen = DATA_LENGTH_3EXPO_VIDEO;
            }
            else
            {
                *dataLen = DATA_LENGTH_3EXPO_PREVIEW;
            }
            memcpy(flickerBuf1, buf, *dataLen*4);
        }
    }
    else
    {
        *dataLen *= 2;
        int16_t *flickerDMAAdr = (int16_t *)buf;
        for (int i = 0; i < *dataLen; i++)
        {
            flickerBuf1[i] = (((int64_t)flickerDMAAdr[3*i])&0xFFFF) + ((((int64_t)flickerDMAAdr[3*i+1])&0xFFFF)<<16) + ((((int64_t)flickerDMAAdr[3*i+2])&0xFFFF)<<32);
        }
    }

    for (int32_t i = 0; i < *imgH; i += 100) {
        logD("analyzeFlickerFrequency(): sta row =%d, %ld %ld %ld", i,
                flickerBuf1[NUM_X * i],
                flickerBuf1[NUM_X * i + 1],
                flickerBuf1[NUM_X * i + 2]);
    }

    m_dataLen = *dataLen;

    return 0;
}

int32_t FlickerPlatformAdapter::getWindowInfo(int32_t imgW, int32_t imgH, int32_t *winW,int32_t *winH)
{
    *winW = ((imgW / NUM_X) >> 1) << 1;
    *winH = ((imgH / 3) >> 1) << 1;
    m_winW = *winW;
    m_winH = *winH;
    return 0;
}

static int dumpFLKO(
        const MVOID *buf1, const MVOID *buf2, const int len,
        const int winW, const int winH, const int flickerState,
        const FlkSensorInfo *sensorInfo, const FlkAEInfo *AEInfo, const int *afStt)
{
    /* update file count */
    int cnt = 0;
    getFileCount(FLK_DUMP_COUNT_FILE, &cnt, 0);
    setFileCount(FLK_DUMP_COUNT_FILE, cnt + 1);

    /* create folder */
    char s[64];
    int ret = 0;
    ret = createDir("/data/vendor/flicker/flko/");
    if(ret != 0 && errno != EEXIST)
    {
        logE("dumpFLKO(): failed to createDir(/data/vendor/flicker/flko/): %s.", strerror(errno));
        return -1;
    }
    snprintf(s, sizeof(s), "/data/vendor/flicker/flko/%03d", cnt);
    ret = createDir(s);
    if(ret != 0 && errno != EEXIST)
    {
        logE("dumpFLKO(): failed to createDir(%s): %s.", s, strerror(errno));
        return -1;
    }

    FILE *fp;
    snprintf(s, sizeof(s), "/data/vendor/flicker/flko/%03d/flk.raw", cnt);
    fp = fopen(s, "wb");
    if (!fp) {
        logE("dumpFLKO(): failed to open file(%s).", s);
        return -1;
    }

    fwrite(&len, 1, 4, fp);
    fwrite(buf1, 4, len, fp);
    fwrite(buf2, 4, len, fp);
    fwrite(&winW, 1, 4, fp);
    fwrite(&winH, 1, 4, fp);
    fwrite(&flickerState, 1, 4, fp);
    fwrite(sensorInfo, 1, sizeof(FlkSensorInfo), fp);
    fwrite(AEInfo, 1, sizeof(FlkAEInfo), fp);
    fwrite(afStt, 4, 9, fp);
    fclose(fp);

    /* dump pure FLKO */
    int dumpPure = 0;
    getPropInt(PROP_FLK_DUMP_PURE, &dumpPure, 0);
    if (dumpPure)
    {
        snprintf(s, sizeof(s), "/data/vendor/flicker/flko/%03d/flk_pure.raw", cnt);
        fp = fopen(s, "wb");
        if (!fp) {
            logE("dumpFLKO(): failed to open file(%s).", s);
            return -1;
        }

        fwrite(buf1, 4, len, fp);
        fwrite(buf2, 4, len, fp);
        fclose(fp);
    }

    return 0;
}
int32_t FlickerPlatformAdapter::detectFlicker(int32_t sensorType,int32_t flickerState,int32_t exp)
{
    FlkStatus algoStatus = FK100;
    FlkDetectPara detectPara;
    FlkEISVector eisVector;
    FlkSensorInfo sensorInfo;
    FlkAEInfo aeInfo;
    int32_t afStat[9] = {0};


    memset(&eisVector, 0, sizeof(FlkEISVector));

    sensorInfo.pixel_clock = m_u4SensorPixelClkFreq;
    sensorInfo.full_line_width = m_u4PixelsInLine;

    aeInfo.preview_shutter_value = exp;


    detectPara.c_list0 = (void*)m_pVectorData2;
    detectPara.c_list1 = (void*)m_pVectorData1;
    detectPara.cur_freq = (int32_t)flickerState;
    detectPara.sensor_info = sensorInfo;
    detectPara.eis_vec = eisVector;
    detectPara.ae_info = aeInfo;
    detectPara.af_stat = afStat;
    //Use Normal sensor as default settings
    detectPara.type = NORMAL;
    detectPara.n_win_h = 3;
    detectPara.n_win_w = NUM_X;
    detectPara.win_wd = m_winW;
    detectPara.win_ht = m_winH;

    if (sensorType == FLK_SENSOR_TYPE_3EXPO)
    {
        if(m_sensorId == 0x0576)
        {
            detectPara.type = IMX576;
            detectPara.n_win_h = 3;
            detectPara.n_win_w = 4;
            detectPara.win_wd = 2;
            detectPara.win_ht = 32;
        }
        else if(m_sensorId == 0x2185)
        {
            int columnLength = (m_sensorMode== e_sensorModePreview) ? 67:50;
            detectPara.type = S5K2X5;
            detectPara.n_win_h = 3;
            detectPara.n_win_w = 1;
            detectPara.win_wd = 720;
            detectPara.win_ht = columnLength;
        }
        else
        {
            logE("This sensor(%d) is not supported.", m_sensorId);
            return 0;
        }
    }

    int32_t dump = 0;
    getPropInt(PROP_FLK_DUMP, &dump, 0);
    if (dump)
    {
        dumpFLKO(detectPara.c_list0, detectPara.c_list1, m_dataLen * 2, detectPara.win_wd, detectPara.win_ht,
                detectPara.cur_freq, &(detectPara.sensor_info), &(detectPara.ae_info), detectPara.af_stat);
    }

    logD("analyzeFlickerFrequency(): detect flicker win(%d,%d), exp(%d), freq(%d)+.",
                detectPara.win_wd, detectPara.win_ht, exp, detectPara.cur_freq);

    CAM_TRACE_BEGIN("FLK algo");
    SendCmdFlk(CMD_DETECT, (void*)&detectPara, (void*)&algoStatus);

    CAM_TRACE_END();

    return algoStatus;
}


int32_t FlickerPlatformAdapter::setFlickerState(int32_t state)
{
    logD("setFlickerState(): %d.", state);
    FlkStatePara statePara;
    if (state == HAL_FLICKER_AUTO_50HZ)
    {
        statePara.ev = HZ50;
    }
    else
    {
        statePara.ev = HZ60;
    }

    SendCmdFlk(CMD_SET_STATE, (void *)&statePara, NULL);

    return 0;
}

int32_t FlickerPlatformAdapter::resetQueue()
{
    SendCmdFlk(CMD_RESET_QUEUE, NULL, NULL);
    return 0;
}

int32_t FlickerPlatformAdapter::setLogLevel(int32_t loglevel)
{
    logD("setLoglevel(): %d.", loglevel);
    FlkLogPara logpara;
    logpara.log_level = loglevel;
    SendCmdFlk(CMD_SET_LOG_LEVEL, (void*)&logpara, NULL);
    return 0;
}
