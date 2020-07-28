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

/* aaa headers */
#include <isp_mgr.h>
#include <isp_mgr_flk.h>

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
using namespace NSIspTuningv3;

/***********************************************************
 * Define macros
 **********************************************************/
#define FLICKER_MAX_LENG (6000)
#define NUM_X (3)

#define MHAL_FLICKER_WORKING_BUF_SIZE (FLICKER_MAX_LENG * sizeof(int32_t) * NUM_X)


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

    m_sensorId = i4SensorIdx;

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

    m_pVectorData1 = (int32_t *)malloc(MHAL_FLICKER_WORKING_BUF_SIZE);

    if (!m_pVectorData1) {
        logE("createBuf(): falied to allocate buffer 1.");
        return -1;
    }

    m_pVectorData2 = (int32_t *)malloc(MHAL_FLICKER_WORKING_BUF_SIZE);

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
    *sensorType = 0;
    (void)hdrType;
    return 0;
}

int32_t FlickerPlatformAdapter::getInfo(MBOOL *flkInfo)
{

    /* get flicker info from normal pipe */
    NSCam::NSIoPipe::NSCamIOPipe::INormalPipe* pCamIO =
        (NSCam::NSIoPipe::NSCamIOPipe::INormalPipe *)
        INormalPipeUtils::get()->createDefaultNormalPipe(m_sensorId, LOG_TAG);
    if (!pCamIO) {
        logE("getInfo(): failed to create normal pipe.");
        return MFALSE;
    }

    pCamIO->sendCommand(
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeCmd_GET_FLK_INFO,
            (MINTPTR)flkInfo, 0, 0);
    pCamIO->destroyInstance(LOG_TAG);

    return 0;
}

int32_t FlickerPlatformAdapter::setTGInfo(int32_t tgInfo)
{
    mTgInfo = tgInfo;
    return 0;
}

int32_t FlickerPlatformAdapter::setWindowInfo(uint32_t *imgW, uint32_t *imgH, uint32_t *winW, uint32_t *winH, uint32_t u4TgW, uint32_t u4TgH)
{
    logD("setWindowInfo(): begin");
    FLKWinCFG_T flkWinCfg;
    prepareFlickerCfg(u4TgW, u4TgH, &flkWinCfg);
    ISP_MGR_FLK_CONFIG_T::getInstance(
            static_cast<ESensorDev_T>(m_sensorDev)).config(flkWinCfg, mTgInfo);

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
    (void)imgW;
    (void)imgH;
    (void)winW;
    (void)winH;
    (void)tgWidth;
    (void)tgHeight;
    (void)binWidth;
    (void)binHeight;

    return 0;
}

int32_t FlickerPlatformAdapter::getSensorMode(int32_t sensorScenario,int32_t sensorType, int32_t *sensorMode)
{
    if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_NORMAL_PREVIEW)
    {
        *sensorMode = e_sensorModePreview;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_NORMAL_CAPTURE)
    {
        *sensorMode = e_sensorModeCapture;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_NORMAL_VIDEO)
    {
        *sensorMode = e_sensorModeVideoPreview;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_SLIM_VIDEO1)
    {
        *sensorMode = e_sensorModeVideo1;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_SLIM_VIDEO2)
    {
        *sensorMode = e_sensorModeVideo2;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_CUSTOM1)
    {
        *sensorMode = e_sensorModeCustom1;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_CUSTOM2)
    {
        *sensorMode = e_sensorModeCustom2;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_CUSTOM3)
    {
        *sensorMode = e_sensorModeCustom3;
    }
    else if (sensorScenario == NSCam::SENSOR_SCENARIO_ID_CUSTOM4)
    {
        *sensorMode = e_sensorModeCustom4;
    }
    else /* if (sensorScenario == SENSOR_SCENARIO_ID_CUSTOM5) */
    {
        *sensorMode = e_sensorModeCustom5;
    }
    (void)sensorType;
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
        flkWinCfg->m_u4NumX * flkWinCfg->m_u4NumY * flkWinCfg->m_u4SizeY * 2;

    flkWinCfg->m_u4SGG3_PGN = 16;
    flkWinCfg->m_u4SGG3_GMR1 = 20;
    flkWinCfg->m_u4SGG3_GMR2 = 29;
    flkWinCfg->m_u4SGG3_GMR3 = 42;
    flkWinCfg->m_u4SGG3_GMR4 = 62;
    flkWinCfg->m_u4SGG3_GMR5 = 88;
    flkWinCfg->m_u4SGG3_GMR6 = 126;
    flkWinCfg->m_u4SGG3_GMR7 = 180;


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

}

int32_t FlickerPlatformAdapter::getFLKResultConfig(MVOID * p_sFLKResultConfig)
{
    (void)p_sFLKResultConfig;
    return 0;
}

int32_t FlickerPlatformAdapter::init_algo(uint32_t pixelsLine,uint32_t winH,uint32_t winW,uint32_t pixelClkFreq,int32_t sensorType)
{
    m_u4SensorPixelClkFreq = pixelClkFreq;
    m_u4PixelsInLine = pixelsLine;

    logD("[%s] flicker_init", __FUNCTION__);
    flicker_init(pixelsLine, winH*3, winW, pixelClkFreq); // algo
    (void)sensorType;
    return 0;
}


int32_t FlickerPlatformAdapter::uninit_algo()
{
    flicker_uninit();
    return 0;
}

int32_t FlickerPlatformAdapter::setExtPara(FlickerExtPara para)
{
    FLICKER_EXT_PARA extpara;
    extpara.flickerFreq[0]       = para.flicker_freq[0];
    extpara.flickerFreq[1]       = para.flicker_freq[1];
    extpara.flickerFreq[2]       = para.flicker_freq[2];
    extpara.flickerFreq[3]       = para.flicker_freq[3];
    extpara.flickerFreq[4]       = para.flicker_freq[4];
    extpara.flickerFreq[5]       = para.flicker_freq[5];
    extpara.flickerFreq[6]       = para.flicker_freq[6];
    extpara.flickerFreq[7]       = para.flicker_freq[7];
    extpara.flickerFreq[8]       = para.flicker_freq[8];
    extpara.flickerGradThreshold = para.flicker_grad_threshold;
    extpara.flickerSearchRange   = para.flicker_search_range;
    extpara.minPastFrames        = para.min_past_frames;
    extpara.maxPastFrames        = para.max_past_frames;
    extpara.EV50_L50.m           = para.ev50_l50.m;
    extpara.EV50_L50.b_l         = para.ev50_l50.b_l;
    extpara.EV50_L50.b_r         = para.ev50_l50.b_r;
    extpara.EV50_L50.offset      = para.ev50_l50.offset;
    extpara.EV50_L60.m           = para.ev50_l60.m;
    extpara.EV50_L60.b_l         = para.ev50_l60.b_l;
    extpara.EV50_L60.b_r         = para.ev50_l60.b_r;
    extpara.EV50_L60.offset      = para.ev50_l60.offset;
    extpara.EV60_L50.m           = para.ev60_l50.m;
    extpara.EV60_L50.b_l         = para.ev60_l50.b_l;
    extpara.EV60_L50.b_r         = para.ev60_l50.b_r;
    extpara.EV60_L50.offset      = para.ev60_l50.offset;
    extpara.EV60_L60.m           = para.ev60_l60.m;
    extpara.EV60_L60.b_l         = para.ev60_l60.b_l;
    extpara.EV60_L60.b_r         = para.ev60_l60.b_r;
    extpara.EV60_L60.offset      = para.ev60_l60.offset;
    extpara.EV50_thresholds[0]   = para.ev50_thresholds[0];
    extpara.EV50_thresholds[1]   = para.ev50_thresholds[1];
    extpara.EV60_thresholds[0]   = para.ev60_thresholds[0];
    extpara.EV60_thresholds[1]   = para.ev60_thresholds[1];
    extpara.freq_feature_index[0] = para.freq_feature_index[0];
    extpara.freq_feature_index[1] = para.freq_feature_index[1];

    flicker_setExtPara(&extpara);

    return 0;
}

int32_t FlickerPlatformAdapter::setBuf(int32_t sensorType, MVOID * buf, int32_t currentCycle, int32_t *dataLen, int32_t *imgW, int32_t *imgH)
{
    int32_t *flickerBuf1;
    int32_t *flickerBuf2;

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

    int32_t *flickerDMAAdr = (int32_t *)buf;
    for (int32_t i = 0; i < *dataLen; i++) {
        flickerBuf1[2 * i + 0] = flickerDMAAdr[i] & 0x0000FFFF;
        flickerBuf1[2 * i + 1] = (flickerDMAAdr[i] & 0xFFFF0000) >> 16;
    }

    for (int32_t i = 0; i < *imgH; i += 100) {
        logD("analyzeFlickerFrequency(): sta row =%d, %d %d %d", i,
                flickerBuf1[NUM_X * i],
                flickerBuf1[NUM_X * i + 1],
                flickerBuf1[NUM_X * i + 2]);
    }

    m_dataLen = *dataLen;
    (void)sensorType;
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

int32_t FlickerPlatformAdapter::setFlickerState(int32_t state)
{
    logD("setFlickerState(): %d.", state);
    if (state == HAL_FLICKER_AUTO_50HZ)
        set_flicker_state(Hz50); // algo
    else
        set_flicker_state(Hz60); // algo
    return 0;
}

int32_t FlickerPlatformAdapter::resetQueue()
{
    reset_flicker_queue(); // algo
    return 0;
}

static int dumpFLKO(
        const MVOID *buf1, const MVOID *buf2, const int len,
        const int winW, const int winH, const int flickerState,
        const flkSensorInfo *sensorInfo, const flkAEInfo *AEInfo, const int *afStt)
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
    fwrite(sensorInfo, 1, sizeof(flkSensorInfo), fp);
    fwrite(AEInfo, 1, sizeof(flkAEInfo), fp);
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
    FLICKER_STATUS algoStatus = FK100;
    flkEISVector eisVector;
    flkSensorInfo sensorInfo;
    flkAEInfo aeInfo;
    int32_t afStat[9] = {0};

    memset(&eisVector, 0, sizeof(flkEISVector));

    sensorInfo.pixelClock = m_u4SensorPixelClkFreq;
    sensorInfo.fullLineWidth = m_u4PixelsInLine;

    aeInfo.previewShutterValue = exp;

    int32_t dump = 0;
    getPropInt(PROP_FLK_DUMP, &dump, 0);
    if (dump)
    {
        dumpFLKO((void*)m_pVectorData2, (void*)m_pVectorData1, m_dataLen * 2, m_winW, m_winH,
                flickerState, &sensorInfo, &aeInfo, afStat);
    }

    logD("analyzeFlickerFrequency(): detect flicker win(%d,%d), exp(%d), freq(%d)+.",
                m_winW, m_winH, exp, flickerState);

    CAM_TRACE_BEGIN("FLK algo");
    algoStatus = detectFlicker_SW(m_pVectorData2, m_pVectorData1,
    3, NUM_X, m_winW, m_winH, (int32_t)flickerState, sensorInfo, eisVector, aeInfo, afStat); // algo
    CAM_TRACE_END();
    (void)sensorType;
    return algoStatus;
}

int32_t FlickerPlatformAdapter::setLogLevel(int32_t loglevel)
{
    (void)loglevel;
    return 0;
}
