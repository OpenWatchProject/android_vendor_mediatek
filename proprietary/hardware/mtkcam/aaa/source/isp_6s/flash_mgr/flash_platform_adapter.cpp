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

#define LOG_TAG "FlashPlatformAdapter"

#include <array>

/* aaa common headers */
#include "log_utils.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* kernel headers */
#include "kd_camera_feature.h"

/* custom headers */
#include "ae_mgr.h"

/* flash headers */
#include "flash_platform_adapter.h"
#include "flash_utils.h"

#define AFE_GAIN_BASE 1024.0
#define ISP_GAIN_BASE 4096.0
#define MAX_RGB_VALUE  255

/***********************************************************
 * AA statistic
 **********************************************************/
template <class T>
int convertAaSttToY(void *buf, int w, int h, T *y, int gain,
        NS3Av3::EBitMode_T mode)
{
    int index = 0;
    int shift = 4;
    if (mode == NS3Av3::EBitMode_10Bit)
        shift = 2;
    else if (mode == NS3Av3::EBitMode_12Bit)
        shift = 4;
    else if (mode == NS3Av3::EBitMode_14Bit)
        shift = 6;
    else if (mode == NS3Av3::EBitMode_16Bit)
        shift = 8;

    // ISP60 AAO parsing
    int i = 0;
    int j = 0;
    unsigned short *pY = NULL;
    for(j = 0; j < h; j++)
    {
        pY = ((unsigned short*)buf) + 12*w*j + 8*w;
        for(i = 0; i < w; i++)
        {
            y[index] = ((*pY)*gain) >> shift;
            pY++;
            index++;
        }
    }
    return 0;
}
template int convertAaSttToY<short>(void *, int, int, short *, int,
        NS3Av3::EBitMode_T);

template <class T>
int convertAaSttToYrgb(void *buf, int w, int h, T *y, T *r, T *g, T *b,
        NS3Av3::EBitMode_T mode, int linearOutputEn)
{
    int index = 0;
    int shift = 4;
    if (mode == NS3Av3::EBitMode_10Bit)
        shift = 2;
    else if (mode == NS3Av3::EBitMode_12Bit)
        shift = 4;
    else if (mode == NS3Av3::EBitMode_14Bit)
        shift = 6;
    else if (mode == NS3Av3::EBitMode_16Bit)
        shift = 8;

    // ISP60 AAO parsing
    int i = 0;
    int j = 0;

    unsigned int *pRGB = NULL;
    unsigned short *pY = NULL;

    for(j = 0; j < h; j++)
    {
        pRGB = ((unsigned int*)buf) + 6*w*j;
        pY = ((unsigned short*)buf) + 12*w*j + 8*w;

        for(i = 0; i < w; i++)
        {
            if(linearOutputEn == 1)
            {
                double rSum = (double)(*pRGB);
                double gSum = (double)(*(pRGB+1));
                double bSum = (double)(*(pRGB+2));
                double rCount = (double)((*(pRGB+3)) & 0xFF800000 >> 23);
                double gCount = (double)((*(pRGB+3)) & 0x7FC000 >> 14);
                double bCount = (double)((*(pRGB+3)) & 0x3FE0 >> 5);

                if(rCount > 0.0)
                {
                    rSum = rSum / rCount;
                }
                else
                {
                    rSum = 0.0;
                }

                if(gCount > 0.0)
                {
                    gSum = gSum / gCount;
                }
                else
                {
                    gSum = 0.0;
                }

                if(bCount > 0.0)
                {
                    bSum = bSum / bCount;
                }
                else
                {
                    bSum = 0.0;
                }

                r[index] = (((unsigned int)rSum >> (shift-1)) + 1) >> 1;
                if(r[index] > MAX_RGB_VALUE)
                {
                    r[index] = MAX_RGB_VALUE;
                }
                g[index] = (((unsigned int)gSum >> (shift-1)) + 1) >> 1;
                if(g[index] > MAX_RGB_VALUE)
                {
                    g[index] = MAX_RGB_VALUE;
                }
                b[index] = (((unsigned int)bSum >> (shift-1)) + 1) >> 1;
                if(b[index] > MAX_RGB_VALUE)
                {
                    b[index] = MAX_RGB_VALUE;
                }
                // TODO: if shift <= 0
            }
            else
            {
                r[index] = (((*pRGB) >> (shift-1)) + 1) >> 1;
                if(r[index] > MAX_RGB_VALUE)
                {
                    r[index] = MAX_RGB_VALUE;
                }
                g[index] = (((*(pRGB+1)) >> (shift-1)) + 1) >> 1;
                if(g[index] > MAX_RGB_VALUE)
                {
                    g[index] = MAX_RGB_VALUE;
                }
                b[index] = (((*(pRGB+2)) >> (shift-1)) + 1) >> 1;
                if(b[index] > MAX_RGB_VALUE)
                {
                    b[index] = MAX_RGB_VALUE;
                }
                // TODO: if shift <= 0
            }

            y[index] = (*pY) >> shift;

            pRGB+=4;
            pY++;
            index++;
        }
    }
    return 0;
}
template int convertAaSttToYrgb<short>(void *, int, int,
        short *, short *, short *, short *,
        NS3Av3::EBitMode_T, int);
template int convertAaSttToYrgb<double>(void *, int, int,
        double *, double *, double *, double *,
        NS3Av3::EBitMode_T, int);

FlashPlatformAdapter::FlashPlatformAdapter(int sensorDev)
    : mSensorDev(sensorDev)
    , mLinearOutputEnPrev(0)
    , mLinearOutputEnCurr(0)
{
    /* set debug */
    setDebug();
    memset(&m_rAWBStatInfo, 0 ,sizeof(AWB_STAT_INFO_T));
}

FlashPlatformAdapter::~FlashPlatformAdapter()
{
}

int FlashPlatformAdapter::init()
{
    IAwbMgr::getInstance().getAWBStatInfo(mSensorDev, m_rAWBStatInfo);
    logI("m_rAWBStatInfo.i4LinearOutputEn(%d).", m_rAWBStatInfo.i4LinearOutputEn);
    return 0;
}

int FlashPlatformAdapter::uninit()
{
    return 0;
}

#define MY_INST NS3Av3::INST_T<FlashPlatformAdapter>
static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

FlashPlatformAdapter *FlashPlatformAdapter::getInstance(int sensorDev)
{
    int sensorOpenIndex = NS3Av3::mapSensorDevToIdx(sensorDev);
    if(sensorOpenIndex >= SENSOR_IDX_MAX || sensorOpenIndex < 0) {
        logE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, sensorOpenIndex);
        return NULL;
    }

    MY_INST& rSingleton = gMultiton[sensorOpenIndex];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<FlashPlatformAdapter>(sensorDev);
    } );

    return rSingleton.instance.get();
}

int FlashPlatformAdapter::getIspGainBase()
{
    return ISP_GAIN_BASE;
}

int FlashPlatformAdapter::getAfeGainBase()
{
    return AFE_GAIN_BASE;
}

int FlashPlatformAdapter::getAwbLinearOutputEn()
{
    return m_rAWBStatInfo.i4LinearOutputEn; 
}

int FlashPlatformAdapter::getAwbModuleVariationGain(
        AWB_GAIN_T *awbPreGain1Prev, AWB_GAIN_T *awbPreGain1Curr)
{
    IAwbMgr::getInstance().getPostgain(mSensorDev, *awbPreGain1Prev);

    IAwbMgr::getInstance().setPostgain(mSensorDev, *awbPreGain1Curr);
    IAwbMgr::getInstance().setPostgainLock(mSensorDev, MTRUE);

    mLinearOutputEnPrev = m_rAWBStatInfo.i4LinearOutputEn;
    logI("cctCalibration(): backup m_rAWBStatInfo.i4LinearOutputEn(%d) to mLinearOutputEnPrev(%d).", m_rAWBStatInfo.i4LinearOutputEn, mLinearOutputEnPrev);

    m_rAWBStatInfo.i4LinearOutputEn = mLinearOutputEnCurr;
    IAwbMgr::getInstance().setAWBStatInfo(mSensorDev, m_rAWBStatInfo);

    return 0;
}

int FlashPlatformAdapter::restoreAwbModuleVariationGain(
        AWB_GAIN_T *awbPreGain1Prev)
{
    IAwbMgr::getInstance().setPostgainLock(mSensorDev, MFALSE);
    IAwbMgr::getInstance().setPostgain(mSensorDev, *awbPreGain1Prev);

    m_rAWBStatInfo.i4LinearOutputEn = mLinearOutputEnPrev;
    IAwbMgr::getInstance().setAWBStatInfo(mSensorDev, m_rAWBStatInfo);

    return 0;
}

int FlashPlatformAdapter::clearAwbModuleVariationGain(
        AWB_GAIN_T *awbPreGain1Curr)
{
    (void)awbPreGain1Curr;
    return 0;
}


int FlashPlatformAdapter::updateAePreviewParams(
        AE_MODE_CFG_T *aeParam, int bestInd)
{
    int bestIndF = aeParam->u4AEFinerEVIdxBase * bestInd;
    AeMgr::getInstance(mSensorDev).updatePreviewParams(*aeParam, bestInd, bestIndF);
    return 0;
}

int FlashPlatformAdapter::setAeCCUOnOff(int enable)
{
    (void)enable;
    return 0;
}

int FlashPlatformAdapter::setAeExpSetting(int exp, int afe, int isp)
{
    IAeMgr::getInstance().sendAECtrl(mSensorDev, EAECtrl_SetExposureSetting, exp, afe, isp, NULL);
    return 0;
}
