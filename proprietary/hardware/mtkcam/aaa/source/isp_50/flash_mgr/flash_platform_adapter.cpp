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
#include "isp_tuning_mgr.h"
#include "isp_mgr.h"
#include "isp_mgr_awb_stat.h"
#include <private/aaa_utils.h>
#include <private/aaa_hal_private.h>

/* kernel headers */
#include "kd_camera_feature.h"

/* custom headers */
#include "ae_mgr.h"

/* flash headers */
#include "flash_platform_adapter.h"

using namespace NSIspTuning;
using namespace NSIspTuningv3;

#include "flash_utils.h"

#define AFE_GAIN_BASE 1024.0
#define ISP_GAIN_BASE 1024.0

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

    /* Platform: ISP50 */
    int i;
    int j;
    unsigned short *py;
    for(j=0;j<h;j++)
    {
        py = ((unsigned short*)buf)+(23*w/2)*j+10*w;
        for(i=0;i<w;i++)
        {
            y[index]=((*py)*gain) >> shift;
            py++;
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
    (void)linearOutputEn;
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

    /* Platform: ISP50 */
    int i;
    int j;
    unsigned short *py;
    unsigned short *prgb;
    for(j=0;j<h;j++)
    {
        prgb = ((unsigned short*)buf)+23*w*j/2;
        py = ((unsigned short*)buf)+(23*w/2)*j+10*w;
        for(i=0;i<w;i++)
        {
            r[index]=(*prgb) >> shift;
            g[index]=(*(prgb+1)) >> shift;
            b[index]=(*(prgb+2)) >> shift;
            y[index]=(*py) >> shift;
            py++;
            prgb+=4;
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
{
    /* set debug */
    setDebug();

    memset(m_FlashOnAAOBuffer, 0, sizeof(m_FlashOnAAOBuffer));
}

FlashPlatformAdapter::~FlashPlatformAdapter()
{
}

int FlashPlatformAdapter::init()
{
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
    return 0;
}

int FlashPlatformAdapter::getAwbModuleVariationGain(
        AWB_GAIN_T *awbPreGain1Prev, AWB_GAIN_T *awbPreGain1Curr)
{
    (void)awbPreGain1Curr;
    ISP_MGR_AWB_STAT_CONFIG_T::getInstance((ESensorDev_T)mSensorDev).getIspAWBPreGain1(awbPreGain1Prev);
    return 0;
}

int FlashPlatformAdapter::restoreAwbModuleVariationGain(
        AWB_GAIN_T *awbPreGain1Prev)
{
    ISP_MGR_AWB_STAT_CONFIG_T::getInstance((ESensorDev_T)mSensorDev).setIspAWBPreGain1(*awbPreGain1Prev);
    return 0;
}

int FlashPlatformAdapter::clearAwbModuleVariationGain(
        AWB_GAIN_T *awbPreGain1Curr)
{
    ISP_MGR_AWB_STAT_CONFIG_T::getInstance((ESensorDev_T)mSensorDev).setIspAWBPreGain1(*awbPreGain1Curr);
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
    IAeMgr::getInstance().setCCUOnOff(mSensorDev, enable);
    return 0;
}

int FlashPlatformAdapter::setAeExpSetting(int exp, int afe, int isp)
{
    AeMgr::getInstance((ESensorDev_T)mSensorDev).setPfPara(exp, afe, isp);
    return 0;
}

void FlashPlatformAdapter::setFlashOnAAOBuffer(void *buf)
{
    memcpy(m_FlashOnAAOBuffer, buf, FLASH_AAO_BUF_SIZE);
    logD("setFlashOnAAOBuffer(): buf(%x).", buf);
}

void* FlashPlatformAdapter::getFlashOnAAOBuffer()
{
    return m_FlashOnAAOBuffer;
}
