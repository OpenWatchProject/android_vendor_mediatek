/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define DEBUG_LOG_TAG "MfllShotEng"
#define LOG_TAG "MfllShotEng"

#include "MfllShotEng.h"

#include <mtkcam/def/Modes.h>

#include <mtkcam/middleware/v1/camshot/_callbacks.h>

#include <mtkcam/middleware/v1/camshot/CamShotUtils.h>


#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/utils/hw/HwInfoHelper.h>
#include <mtkcam/utils/hw/CamManager.h>

#include <mtkcam/middleware/v1/LegacyPipeline/ILegacyPipeline.h>
#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>
#include <mtkcam/middleware/v1/LegacyPipeline/IResourceContainer.h>
#include <mtkcam/middleware/v1/LegacyPipeline/LegacyPipelineBuilder.h>
#include <mtkcam/middleware/v1/LegacyPipeline/buffer/StreamBufferProviderFactory.h>
#include <mtkcam/middleware/v1/LegacyPipeline/mfc/buffer/Selector.h>

#include <mtkcam/pipeline/hwnode/MfllNode.h>

#include <sys/resource.h>

#include <utils/Thread.h>

#include <mtkcam/feature/mfnr/MfllLog.h>

#include <mtkcam/utils/std/Log.h>

using namespace android;
using namespace NSCam::Utils;
using namespace NSCam::v1;
using namespace NSCam::v1::NSLegacyPipeline;
using namespace NSCamHW;
using namespace NSShot;
using namespace NSCamShot;
using namespace NS3Av3;

#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] @@@error:@@@" fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)


// ---------------------------------------------------------------------------
extern
sp<IShot> createInstance_MfllShotEng(
        char const * const pszShotName,
        uint32_t const u4ShotMode,
        int32_t const  i4OpenId)
{
    sp<IShot> shot;
    sp<MfllShotEng> myShot;

    // new implementator
    myShot = new MfllShotEng(pszShotName, u4ShotMode, i4OpenId);
    if (myShot == NULL)
    {
        mfllLogE("create MfllShot failed");
        goto lbExit;
    }

    // new interface
    shot = new IShot(myShot);
    if (shot == NULL)
    {
        mfllLogE("create IShot failed");
        goto lbExit;
    }

lbExit:
    // free all resources if instantiation failed
    if ((shot == 0) && (myShot != NULL))
    {
        myShot.clear();
    }

    return shot;
}
// ---------------------------------------------------------------------------
template <typename T>
static inline MVOID updateEntry(
        IMetadata* metadata,
        MUINT32 const tag,
        T const& val)
{
    if (NULL == metadata)
    {
        mfllLogE("metadata is NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    metadata->update(entry.tag(), entry);
}
template <class T>
static MBOOL tryGetMetaData(IMetadata *pMetadata, MUINT32 const tag, T &rVal)
{
    if (pMetadata == NULL) {
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if (!entry.isEmpty()) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }

    return MFALSE;
}

static int getCurrent3ALocked(int sensorId, CaptureParam_T &capParam)
{
    static Mutex mx;
    Mutex::Autolock _l(&mx);
    std::shared_ptr<IHal3A> hal3a(
            MAKE_Hal3A(sensorId, LOG_TAG),
                [](auto *p)->void {
                    if (p) p->destroyInstance(LOG_TAG);
                }
            );
    if (hal3a == NULL) {
        CAM_LOGE("create IHal3A instance failed");
        return -1;
    }
    hal3a->send3ACtrl(E3ACtrl_GetExposureParam, (MINTPTR)&capParam, 0);
    return 0;
}
// ---------------------------------------------------------------------------
MfllShotEng::MfllShotEng(
        const char *pszShotName,
        uint32_t const u4ShotMode,
        int32_t const i4OpenId)
: MfllShot(pszShotName, u4ShotMode, i4OpenId)
{
    m_rawCount = 0;
    m_engShot = reinterpret_cast<EngShotWrapper*>
        (new EngShot(pszShotName, u4ShotMode, i4OpenId));
}
// ---------------------------------------------------------------------------
MfllShotEng::~MfllShotEng()
{
}
// ---------------------------------------------------------------------------
bool MfllShotEng::sendCommand(
        uint32_t const  cmd,
        MUINTPTR const  arg1,
        uint32_t const  arg2,
        uint32_t const  arg3 /* = 0 */
)
{
/* assume ISP gain is fixed as 1x (1024) */
#define GAIN_TO_ISO(x)  ((x * 100) / 1024)
#define ISO_TO_GAIN(x)  ((x * 1024) / 100)
    if (cmd == eCmd_setEngParam) {
        m_engParam = *(const_cast<EngParam *>(reinterpret_cast<EngParam const*>(arg1)));
        m_engShot->setEngParam(reinterpret_cast<void const*>(arg1), arg2);
    }
    else if (cmd == eCmd_capture) {
        EngParam *engParam = &m_engShot->engParam();

        /* get current 3A */
        CaptureParam_T capParam;
        if (0 != getCurrent3ALocked(getOpenId(), capParam)) {
            MY_LOGE("get current 3A failed");
        }

        int iso = 0;
        int exp = 0;
        bool forceIso = mShotParam.muSensorGain != 0;
        bool forceExp = mShotParam.muSensorSpeed != 0;

        /**
         *  If use manual 3A, we always set ISP gain to 1x because if the final
         *  gain is greater that sensor gain, AE manager will move overload gain
         *  to ISO gain automatically
         */
        if (!forceIso && !forceExp) {
            /* all auto, use the currect 3A */
            exp = capParam.u4Eposuretime;
            iso = capParam.u4RealISO;
            MY_LOGD("both auto ISO/EXP, (iso,exp)=(%d,%d)", iso, exp);
        }
        else if (forceIso && forceExp) {
            /* force both */
            exp = mShotParam.muSensorSpeed * 1000; // ms -> us
            /* convert sensor gain to ISO within ISP gain 1x (1024) */
            iso = GAIN_TO_ISO(mShotParam.muSensorGain);
            MY_LOGD("both force ISO/EXP, (iso,exp)=(%d,%d)", iso, exp);
        }
        else if (forceIso) {
            /* convert sensor gain to ISO within ISP gain 1x (1024) */
            iso = GAIN_TO_ISO(mShotParam.muSensorGain); // using 1x isp gain
            /* get the current luma first */
            int total = capParam.u4Eposuretime * capParam.u4RealISO;
            /* force ISO, calculate new exposrue */
            exp = total / iso;
            MY_LOGD("force ISO, final (iso,exp) = (%d,%d)", iso, exp);
        }
        else { // force exposure
            exp = mShotParam.muSensorSpeed * 1000; //ms -> us
            /* get the current luma first */
            int total = capParam.u4Eposuretime * capParam.u4RealISO;
            /* calculate iso */
            iso = total / exp;
            MY_LOGD("force EXP, final (iso,exp) = (%d,%d)", iso, exp);
        }

        /* set capture info directly here */
        MfllNode::setCaptureInfo(
                MFBMODE_MFLL, // MFB mode --> MFLL
                getOpenId(), // sensor ID
                1, // is do MFB
                iso, // iso
                exp, // exposure
                engParam->mi4MFLLpc, // frame to blend // TODO: mutable variable fron UI
                engParam->mi4MFLLpc  // frame to capture
                );
    }

    return MfllShot::sendCommand(cmd, arg1, arg2, arg3);
}
// ---------------------------------------------------------------------------
// Metadata result
// ---------------------------------------------------------------------------
void MfllShotEng::onResultReceived(
        MUINT32    const /*requestNo*/,
        StreamId_T const streamId,
        MBOOL      const errorResult,
        IMetadata  const /*result*/)
{
    if (errorResult) {
        MY_LOGE("the result metadata may be wrong...");
    }

    switch (streamId) {
    /**
     *  Note: This is different between MfllShot.
     *
     *  To invoke shutter after MFLL has finished rather than P1 node done
     *  because engineer shot may have a scenario that continuously shots with
     *  Normal MFLL shot without press capture button, which may cause low memory
     *  if MFLL process is using N RAW buffers and the next round of P1 node is
     *  doing too.
     */
    case eSTREAMID_META_APP_DYNAMIC_MFLL:
        mpShotCallback->onCB_Shutter(true, 0);
        break;
    }
}
// ---------------------------------------------------------------------------
// Data
// ---------------------------------------------------------------------------
MERROR MfllShotEng::onResultReceived(
        MUINT32    const requestNo,
        StreamId_T const streamId,
        MBOOL      const errorBuffer,
        android::sp<IImageBuffer>& pBuffer)
{
    MERROR err = OK;

    /* use EngShot::dumpEngRaw to dump RAW files */
    if (streamId == eSTREAMID_IMAGE_PIPE_RAW_OPAQUE) {
        pBuffer->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK);
        EngParam &p = m_engShot->engParam();
        //
        IHalSensorList* const pIHalSensorList = MAKE_HalSensorList();
        if(pIHalSensorList)
        {
            MUINT32 sensorDev = (MUINT32)pIHalSensorList->querySensorDevIdx(getOpenId());

            NSCam::SensorStaticInfo sensorStaticInfo;
            memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
            pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

            p.u4RawPixelID = sensorStaticInfo.sensorFormatOrder;
        }
        //
        p.u4Bitdepth = getShotRawBitDepth();
        p.u4SensorWidth = pBuffer->getImgSize().w;
        p.u4SensorHeight = pBuffer->getImgSize().h;
        p.u4rawStride = pBuffer->getBufStridesInBytes(0);
        m_engShot->dumpEngRaw(pBuffer->getBufStridesInBytes(0), (MUINT8*)pBuffer->getBufVA(0), pBuffer->getImgFormat());
        pBuffer->unlockBuf(LOG_TAG);
    }

    return MfllShot::onResultReceived(
            requestNo,
            streamId,
            errorBuffer,
            pBuffer
            );
}
// ---------------------------------------------------------------------------
