/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015-2016. All rights reserved.
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
#define LOG_TAG "MfllCore/Bss"

#include "MfllBss.h"
#include "MfllUtilities.h"

#include <mtkcam3/feature/mfnr/MfllLog.h>
#include <mtkcam3/feature/mfnr/MfllProperty.h>

//MTKCAM
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/hw/IFDContainer.h>

// ALGORITHM
#include <MTKBss.h>

#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <custom/debug_exif/dbg_exif_param.h>
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#include <tuple>
#include <string>
#endif
#include <fstream>
#include <sstream>

// CUSTOM (platform)
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include "MfllNvram.h"
#endif
#include <camera_custom_nvram.h>


#define MFLLBSS_DUMP_PATH       "/data/vendor/camera_dump/"
#define MFLLBSS_DUMP_FILENAME   "fd-data.txt"


// describes that how many frames we update to exif,
#define MFLLBSS_FD_RECT0_FRAME_COUNT    8
// describes that how many fd rect0 info we need at a frame,
#define MFLLBSS_FD_RECT0_PER_FRAME      4


using std::vector;
using namespace mfll;
using namespace NSCam;

static bool mDumpBssInputData2File(char const* sFilepath, BSS_INPUT_DATA_G& inParam);

IMfllBss* IMfllBss::createInstance()
{
    return reinterpret_cast<IMfllBss*>(new MfllBss);
}

void IMfllBss::destroyInstance()
{
    decStrong((void*)this);
}

MfllBss::MfllBss()
: m_roiPercetange(MFLL_BSS_ROI_PERCENTAGE)
, m_pCore(NULL)
, m_uniqueKey(0)
, m_skipFrmCnt(0)
{
}

enum MfllErr MfllBss::init(sp<IMfllNvram> &nvramProvider)
{
    if (nvramProvider.get() == NULL) {
        mfllLogE("%s: init MfllBss fail, needs NVRAM provider but it's NULL", __FUNCTION__);
        return MfllErr_BadArgument;
    }
    m_nvramProvider = nvramProvider;
    return MfllErr_Ok;
}

vector<int> MfllBss::bss(
        const vector< sp<IMfllImageBuffer> >    &imgs,
        vector<MfllMotionVector_t>              &mvs,
        vector<int64_t>                         &timestamps
        )
{
    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    MTKBss *mtkBss = NULL;
    int width = 0;
    int height = 0;
    size_t size = 0;
    int roiPercentage = m_roiPercetange;
    MUINT32 stride = 0;
    MUINT32 bayerOrder = 0;
    MUINT32 bitNum = 0;

    BSS_WB_STRUCT workingBufferInfo;
    std::unique_ptr<MUINT8[]> bss_working_buffer;

    // thread priority usage
    int _priority = 0;
    int _oripriority = 0;
    int _result = 0;

    vector<int> newIndex;
    // check buffer amount
    if (CC_UNLIKELY( imgs.size() <= 0 || mvs.size() <= 0 )) {
        mfllLogE("%s: no image buffers or GMVs", __FUNCTION__);
        goto lbExit;
    }
    //
    if (imgs.size() != mvs.size()) {
        mfllLogE("%s: the amount of image(%zu) and GMV(%zu) is different",
                __FUNCTION__, imgs.size(), mvs.size());
        goto lbExit;
    }
    // check if input is nullptr
    for (size_t i = 0; i < imgs.size(); i++) {
        if (CC_UNLIKELY( imgs[i].get() == nullptr )) {
            mfllLogE("%s: the input images(idx=%zu) is NULL", __FUNCTION__, i);
            goto lbExit;
        }
        else {
            if (CC_UNLIKELY( ! imgs[i]->isInited() )) {
                mfllLogE("%s: the image buffer(idx=%zu) hasn't been inited",
                        __FUNCTION__, i);
                goto lbExit;
            }
        }
    }

    width = imgs[0]->getAlignedWidth();
    height = imgs[0]->getAlignedHeight();
    stride = imgs[0]->getBufStridesInBytes(0);
    size = imgs.size();

    // query some sensor static info
    {
        IHalSensorList* sensorList = MAKE_HalSensorList();
        if(sensorList == NULL) {
            mfllLogE("%s: get sensor list failed", __FUNCTION__);
            goto lbExit;
        }
        else {
            int sensorDev = sensorList->querySensorDevIdx(m_pCore->getSensorId());

            SensorStaticInfo sensorStaticInfo;
            sensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

            bayerOrder = sensorStaticInfo.sensorFormatOrder;
            bitNum = [&, this]() {
                switch (sensorStaticInfo.rawSensorBit) {
                    case RAW_SENSOR_8BIT:   return 8;
                    case RAW_SENSOR_10BIT:  return 10;
                    case RAW_SENSOR_12BIT:  return 12;
                    case RAW_SENSOR_14BIT:  return 14;
                    default:
                        mfllLogE("%s: get sensor raw bitnum failed", __FUNCTION__);
                        return 0xFF;
                }
            }();
        }
    }

    mtkBss = MTKBss::createInstance(DRV_BSS_OBJ_SW);
    if (mtkBss == NULL) {
        mfllLogE("%s: create MTKBss instance failed", __FUNCTION__);
        goto lbExit;
    }

    // change the current thread's priority, the algorithm threads will inherits
    // this value.
    _priority = MfllProperty::readProperty(Property_AlgoThreadsPriority, MFLL_ALGO_THREADS_PRIORITY);
    _oripriority = 0;
    _result = setThreadPriority(_priority, _oripriority);
    if (CC_UNLIKELY( _result != 0 )) {
        mfllLogW("set algo threads priority failed(err=%d)", _result);
    }
    else {
        mfllLogD3("set algo threads priority to %d", _priority);
    }

    if (mtkBss->BssInit(NULL, NULL) != S_BSS_OK) {
        mfllLogE("%s: init MTKBss failed", __FUNCTION__);
        goto lbExit;
    }

    // algorithm threads have been forked,
    // if priority set OK, reset it back to the original one
    if (CC_LIKELY( _result == 0 )) {
        _result = setThreadPriority( _oripriority, _oripriority );
        if (CC_UNLIKELY( _result != 0 )) {
            mfllLogE("set priority back failed, weird!");
        }
    }

    /* set feature control to get bss working buffer size */
    {
        workingBufferInfo.rProcId    = BSS_PROC2;
        workingBufferInfo.u4Width    = width;
        workingBufferInfo.u4Height   = height;
        workingBufferInfo.u4FrameNum = size;
        workingBufferInfo.u4WKSize   = 0; //it will return working buffer require size
        workingBufferInfo.pu1BW      = nullptr; // assign working buffer latrer.

        auto b = mtkBss->BssFeatureCtrl(BSS_FTCTRL_GET_WB_SIZE, (void*)&workingBufferInfo, NULL);
        if (b != S_BSS_OK) {
            mfllLogE("%s: get working buffer size from MTKBss failed (%d)", __FUNCTION__, (int)b);
            goto lbExit;
        }
        if (workingBufferInfo.u4WKSize <= 0) {
            mfllLogE("%s: unexpected bss working buffer size: %u", __FUNCTION__, workingBufferInfo.u4WKSize);
            goto lbExit;
        }
    }

    bss_working_buffer = std::unique_ptr<MUINT8[]>(new MUINT8[workingBufferInfo.u4WKSize]{0});
    workingBufferInfo.pu1BW = bss_working_buffer.get(); // assign working buffer for bss algo.

    /* print out bss working buffer information */
    mfllLogD3("%s: rProcId    = %d", __FUNCTION__, workingBufferInfo.rProcId);
    mfllLogD3("%s: u4Width    = %u", __FUNCTION__, workingBufferInfo.u4Width);
    mfllLogD3("%s: u4Height   = %u", __FUNCTION__, workingBufferInfo.u4Height);
    mfllLogD3("%s: u4FrameNum = %u", __FUNCTION__, workingBufferInfo.u4FrameNum);
    mfllLogD3("%s: u4WKSize   = %u", __FUNCTION__, workingBufferInfo.u4WKSize);
    mfllLogD3("%s: pu1BW      = %p", __FUNCTION__, workingBufferInfo.pu1BW);

    /* set feature control to set bss working buffer */
    {
        auto b = mtkBss->BssFeatureCtrl(BSS_FTCTRL_SET_WB_SIZE, (void*)&workingBufferInfo, NULL);
        if (b != S_BSS_OK) {
            mfllLogE("%s: set working buffer to MTKBss failed, size=%d (%u)", __FUNCTION__, (int)b, workingBufferInfo.u4WKSize);
            goto lbExit;
        }
    }

    /* set feature control */
    {
#define MAKE_TAG(prefix, tag, id)   prefix##tag##id
#define MAKE_TUPLE(tag, id)         std::make_tuple(#tag, id)
#define DECLARE_BSS_ENUM_MAP()      std::map<std::tuple<std::string, int>, MUINT32> enumMap
#define BUILD_BSS_ENUM_MAP(tag) \
        do { \
            if (enumMap[MAKE_TUPLE(tag,-1)] == 1) break; \
            enumMap[MAKE_TUPLE(tag,-1)] = 1; \
            enumMap[MAKE_TUPLE(tag, 0)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _00); \
            enumMap[MAKE_TUPLE(tag, 1)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _01); \
            enumMap[MAKE_TUPLE(tag, 2)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _02); \
            enumMap[MAKE_TUPLE(tag, 3)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _03); \
        } while (0)
#define SET_CUST_MFLL_BSS(tag, idx, value) \
        do { \
            BUILD_BSS_ENUM_MAP(tag); \
            enumMap[MAKE_TUPLE(tag, idx)] = (MUINT32)value; \
        } while (0)
#define GET_CUST_MFLL_BSS(tag) \
        [&, this]() { \
            BUILD_BSS_ENUM_MAP(tag); \
            return enumMap[MAKE_TUPLE(tag, m_pCore->getSensorId())]; \
        }()

        DECLARE_BSS_ENUM_MAP();

        //replace by NVRAM
        {
            /* read NVRAM for tuning data */
            size_t chunkSize = 0;
            const char *pChunk = m_nvramProvider->getChunk(&chunkSize);
            if (CC_UNLIKELY(pChunk == NULL)) {
                mfllLogE("%s: read NVRAM failed, use default", __FUNCTION__);
            }
            else {
                char *pMutableChunk = const_cast<char*>(pChunk);
                NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunk);

                if (CC_LIKELY( pNvram->bss_iso_th0 != 0 )) {
                    /* update bad range and bad threshold */
                    int currIso = m_pCore->getCurrentIso();
                    int sensorId = m_pCore->getSensorId();

                    // get memc_noise_level by the current ISO
                    if (currIso < static_cast<int>(pNvram->bss_iso_th0)) {
                        SET_CUST_MFLL_BSS(CLIP_TH0, sensorId, pNvram->bss_iso0_clip_th0);
                        SET_CUST_MFLL_BSS(CLIP_TH1, sensorId, pNvram->bss_iso0_clip_th1);
                        SET_CUST_MFLL_BSS(CLIP_TH2, sensorId, pNvram->bss_iso0_clip_th2);
                        SET_CUST_MFLL_BSS(CLIP_TH3, sensorId, pNvram->bss_iso0_clip_th3);
                        SET_CUST_MFLL_BSS(ADF_TH,   sensorId, pNvram->bss_iso0_adf_th);
                        SET_CUST_MFLL_BSS(SDF_TH,   sensorId, pNvram->bss_iso0_sdf_th);
                    } else if (currIso < static_cast<int>(pNvram->bss_iso_th1)) {
                        SET_CUST_MFLL_BSS(CLIP_TH0, sensorId, pNvram->bss_iso1_clip_th0);
                        SET_CUST_MFLL_BSS(CLIP_TH1, sensorId, pNvram->bss_iso1_clip_th1);
                        SET_CUST_MFLL_BSS(CLIP_TH2, sensorId, pNvram->bss_iso1_clip_th2);
                        SET_CUST_MFLL_BSS(CLIP_TH3, sensorId, pNvram->bss_iso1_clip_th3);
                        SET_CUST_MFLL_BSS(ADF_TH,   sensorId, pNvram->bss_iso1_adf_th);
                        SET_CUST_MFLL_BSS(SDF_TH,   sensorId, pNvram->bss_iso1_sdf_th);
                    } else if (currIso < static_cast<int>(pNvram->bss_iso_th2)) {
                        SET_CUST_MFLL_BSS(CLIP_TH0, sensorId, pNvram->bss_iso2_clip_th0);
                        SET_CUST_MFLL_BSS(CLIP_TH1, sensorId, pNvram->bss_iso2_clip_th1);
                        SET_CUST_MFLL_BSS(CLIP_TH2, sensorId, pNvram->bss_iso2_clip_th2);
                        SET_CUST_MFLL_BSS(CLIP_TH3, sensorId, pNvram->bss_iso2_clip_th3);
                        SET_CUST_MFLL_BSS(ADF_TH,   sensorId, pNvram->bss_iso2_adf_th);
                        SET_CUST_MFLL_BSS(SDF_TH,   sensorId, pNvram->bss_iso2_sdf_th);
                    } else if (currIso < static_cast<int>(pNvram->bss_iso_th3)) {
                        SET_CUST_MFLL_BSS(CLIP_TH0, sensorId, pNvram->bss_iso3_clip_th0);
                        SET_CUST_MFLL_BSS(CLIP_TH1, sensorId, pNvram->bss_iso3_clip_th1);
                        SET_CUST_MFLL_BSS(CLIP_TH2, sensorId, pNvram->bss_iso3_clip_th2);
                        SET_CUST_MFLL_BSS(CLIP_TH3, sensorId, pNvram->bss_iso3_clip_th3);
                        SET_CUST_MFLL_BSS(ADF_TH,   sensorId, pNvram->bss_iso3_adf_th);
                        SET_CUST_MFLL_BSS(SDF_TH,   sensorId, pNvram->bss_iso3_sdf_th);
                    } else {
                        SET_CUST_MFLL_BSS(CLIP_TH0, sensorId, pNvram->bss_iso4_clip_th0);
                        SET_CUST_MFLL_BSS(CLIP_TH1, sensorId, pNvram->bss_iso4_clip_th1);
                        SET_CUST_MFLL_BSS(CLIP_TH2, sensorId, pNvram->bss_iso4_clip_th2);
                        SET_CUST_MFLL_BSS(CLIP_TH3, sensorId, pNvram->bss_iso4_clip_th3);
                        SET_CUST_MFLL_BSS(ADF_TH,   sensorId, pNvram->bss_iso4_adf_th);
                        SET_CUST_MFLL_BSS(SDF_TH,   sensorId, pNvram->bss_iso4_sdf_th);
                    }
                    mfllLogD3("%s: bss clip/adf/sdf apply nvram setting.", __FUNCTION__);
                }
            }
        }

        /* calcuate ROI cropping width */
        int w = (width * roiPercentage + 5) / 100;
        int h = (height * roiPercentage + 5) / 100;
        int x = (width - w) / 2;
        int y = (height - h) / 2;

        BSS_PARAM_STRUCT p;
        ::memset(&p, 0x00, sizeof(decltype(p)));
        p.BSS_ON            = 1;
        p.BSS_VER           = 2;
        p.BSS_ROI_WIDTH     = w;
        p.BSS_ROI_HEIGHT    = h;
        p.BSS_ROI_X0        = x;
        p.BSS_ROI_Y0        = y;
        p.BSS_SCALE_FACTOR  = GET_CUST_MFLL_BSS(SCALE_FACTOR);
        p.BSS_CLIP_TH0      = GET_CUST_MFLL_BSS(CLIP_TH0);
        p.BSS_CLIP_TH1      = GET_CUST_MFLL_BSS(CLIP_TH1);
        p.BSS_CLIP_TH2      = GET_CUST_MFLL_BSS(CLIP_TH2);
        p.BSS_CLIP_TH3      = GET_CUST_MFLL_BSS(CLIP_TH3);
        p.BSS_ZERO          = GET_CUST_MFLL_BSS(ZERO);
        p.BSS_FRAME_NUM     = (MUINT32)imgs.size();
        p.BSS_ADF_TH        = GET_CUST_MFLL_BSS(ADF_TH);
        p.BSS_SDF_TH        = GET_CUST_MFLL_BSS(SDF_TH);
        p.BSS_GAIN_TH0      = GET_CUST_MFLL_BSS(GAIN_TH0);
        p.BSS_GAIN_TH1      = GET_CUST_MFLL_BSS(GAIN_TH1);
        p.BSS_MIN_ISP_GAIN  = GET_CUST_MFLL_BSS(MIN_ISP_GAIN);
        p.BSS_LCSO_SIZE     = 0; // TODO: query lcso size for AE compensation

        p.BSS_YPF_EN        = GET_CUST_MFLL_BSS(YPF_EN);
        p.BSS_YPF_FAC       = GET_CUST_MFLL_BSS(YPF_FAC);
        p.BSS_YPF_ADJTH     = GET_CUST_MFLL_BSS(YPF_ADJTH);
        p.BSS_YPF_DFMED0    = GET_CUST_MFLL_BSS(YPF_DFMED0);
        p.BSS_YPF_DFMED1    = GET_CUST_MFLL_BSS(YPF_DFMED1);
        p.BSS_YPF_TH0       = GET_CUST_MFLL_BSS(YPF_TH0);
        p.BSS_YPF_TH1       = GET_CUST_MFLL_BSS(YPF_TH1);
        p.BSS_YPF_TH2       = GET_CUST_MFLL_BSS(YPF_TH2);
        p.BSS_YPF_TH3       = GET_CUST_MFLL_BSS(YPF_TH3);
        p.BSS_YPF_TH4       = GET_CUST_MFLL_BSS(YPF_TH4);
        p.BSS_YPF_TH5       = GET_CUST_MFLL_BSS(YPF_TH5);
        p.BSS_YPF_TH6       = GET_CUST_MFLL_BSS(YPF_TH6);
        p.BSS_YPF_TH7       = GET_CUST_MFLL_BSS(YPF_TH7);

        p.BSS_FD_EN         = GET_CUST_MFLL_BSS(FD_EN);
        p.BSS_FD_FAC        = GET_CUST_MFLL_BSS(FD_FAC);
        p.BSS_FD_FNUM       = GET_CUST_MFLL_BSS(FD_FNUM);

        p.BSS_EYE_EN        = GET_CUST_MFLL_BSS(EYE_EN);
        p.BSS_EYE_CFTH      = GET_CUST_MFLL_BSS(EYE_CFTH);
        p.BSS_EYE_RATIO0    = GET_CUST_MFLL_BSS(EYE_RATIO0);
        p.BSS_EYE_RATIO1    = GET_CUST_MFLL_BSS(EYE_RATIO1);
        p.BSS_EYE_FAC       = GET_CUST_MFLL_BSS(EYE_FAC);

        if (CC_UNLIKELY(getForceBss(reinterpret_cast<void*>(&p), sizeof(BSS_PARAM_STRUCT)))) {
            mfllLogI("%s: force set BSS param as manual setting", __FUNCTION__);
        }
        /* print out information */
        mfllLogD3("%s: BSS_ON  = %d", __FUNCTION__, p.BSS_ON);
        mfllLogD3("%s: BSS_VER = %d", __FUNCTION__, p.BSS_VER);
        mfllLogD3("%s: BSS_ROI(x,y,w,h) =(%d,%d,%d,%d)", __FUNCTION__,
                      p.BSS_ROI_X0, p.BSS_ROI_Y0, p.BSS_ROI_WIDTH, p.BSS_ROI_HEIGHT);
        mfllLogD3("%s: BSS_SCALE_FACTOR = %d", __FUNCTION__, p.BSS_SCALE_FACTOR);
        mfllLogD3("%s: BSS_CLIP_TH0     = %d", __FUNCTION__, p.BSS_CLIP_TH0);
        mfllLogD3("%s: BSS_CLIP_TH1     = %d", __FUNCTION__, p.BSS_CLIP_TH1);
        mfllLogD3("%s: BSS_CLIP_TH2     = %d", __FUNCTION__, p.BSS_CLIP_TH2);
        mfllLogD3("%s: BSS_CLIP_TH3     = %d", __FUNCTION__, p.BSS_CLIP_TH3);
        mfllLogD3("%s: BSS_ZERO         = %d", __FUNCTION__, p.BSS_ZERO);
        mfllLogD3("%s: BSS_FRAME_NUM    = %d", __FUNCTION__, p.BSS_FRAME_NUM);
        mfllLogD3("%s: BSS_ADF_TH       = %d", __FUNCTION__, p.BSS_ADF_TH);
        mfllLogD3("%s: BSS_SDF_TH       = %d", __FUNCTION__, p.BSS_SDF_TH);
        mfllLogD3("%s: BSS_GAIN_TH0     = %d", __FUNCTION__, p.BSS_GAIN_TH0);
        mfllLogD3("%s: BSS_GAIN_TH1     = %d", __FUNCTION__, p.BSS_GAIN_TH1);
        mfllLogD3("%s: BSS_MIN_ISP_GAIN = %d", __FUNCTION__, p.BSS_MIN_ISP_GAIN);
        mfllLogD3("%s: BSS_LCSO_SIZE    = %d", __FUNCTION__, p.BSS_LCSO_SIZE);

        mfllLogD3("%s: BSS_YPF_EN       = %d", __FUNCTION__, p.BSS_YPF_EN);
        mfllLogD3("%s: BSS_YPF_FAC      = %d", __FUNCTION__, p.BSS_YPF_FAC);
        mfllLogD3("%s: BSS_YPF_ADJTH    = %d", __FUNCTION__, p.BSS_YPF_ADJTH);
        mfllLogD3("%s: BSS_YPF_DFMED0   = %d", __FUNCTION__, p.BSS_YPF_DFMED0);
        mfllLogD3("%s: BSS_YPF_DFMED1   = %d", __FUNCTION__, p.BSS_YPF_DFMED1);
        mfllLogD3("%s: BSS_YPF_TH0      = %d", __FUNCTION__, p.BSS_YPF_TH0);
        mfllLogD3("%s: BSS_YPF_TH1      = %d", __FUNCTION__, p.BSS_YPF_TH1);
        mfllLogD3("%s: BSS_YPF_TH2      = %d", __FUNCTION__, p.BSS_YPF_TH2);
        mfllLogD3("%s: BSS_YPF_TH3      = %d", __FUNCTION__, p.BSS_YPF_TH3);
        mfllLogD3("%s: BSS_YPF_TH4      = %d", __FUNCTION__, p.BSS_YPF_TH4);
        mfllLogD3("%s: BSS_YPF_TH5      = %d", __FUNCTION__, p.BSS_YPF_TH5);
        mfllLogD3("%s: BSS_YPF_TH6      = %d", __FUNCTION__, p.BSS_YPF_TH6);
        mfllLogD3("%s: BSS_YPF_TH7      = %d", __FUNCTION__, p.BSS_YPF_TH7);

        mfllLogD3("%s: BSS_FD_EN        = %d", __FUNCTION__, p.BSS_FD_EN);
        mfllLogD3("%s: BSS_FD_FAC       = %d", __FUNCTION__, p.BSS_FD_FAC);
        mfllLogD3("%s: BSS_FD_FNUM      = %d", __FUNCTION__, p.BSS_FD_FNUM);

        mfllLogD3("%s: BSS_EYE_EN       = %d", __FUNCTION__, p.BSS_EYE_EN);
        mfllLogD3("%s: BSS_EYE_CFTH     = %d", __FUNCTION__, p.BSS_EYE_CFTH);
        mfllLogD3("%s: BSS_EYE_RATIO0   = %d", __FUNCTION__, p.BSS_EYE_RATIO0);
        mfllLogD3("%s: BSS_EYE_RATIO1   = %d", __FUNCTION__, p.BSS_EYE_RATIO1);
        mfllLogD3("%s: BSS_EYE_FAC      = %d", __FUNCTION__, p.BSS_EYE_FAC);

        /* update debug info */
#if (MFLL_MF_TAG_VERSION > 0)
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_WIDTH          ,(uint32_t)p.BSS_ROI_WIDTH      );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_HEIGHT         ,(uint32_t)p.BSS_ROI_HEIGHT     );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_SCALE_FACTOR       ,(uint32_t)p.BSS_SCALE_FACTOR   );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_CLIP_TH0           ,(uint32_t)p.BSS_CLIP_TH0       );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_CLIP_TH1           ,(uint32_t)p.BSS_CLIP_TH1       );

#   if (MFLL_MF_TAG_VERSION >= 9)
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_CLIP_TH2           ,(uint32_t)p.BSS_CLIP_TH2       );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_CLIP_TH3           ,(uint32_t)p.BSS_CLIP_TH3       );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_FRAME_NUM          ,(uint32_t)p.BSS_FRAME_NUM      );
#   endif

        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ZERO               ,(uint32_t)p.BSS_ZERO           );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_X0             ,(uint32_t)p.BSS_ROI_X0         );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_Y0             ,(uint32_t)p.BSS_ROI_Y0         );

#   if (MFLL_MF_TAG_VERSION >= 3)
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ADF_TH             ,(uint32_t)p.BSS_ADF_TH         );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_SDF_TH             ,(uint32_t)p.BSS_SDF_TH         );
#   endif

#   if (MFLL_MF_TAG_VERSION >= 9)
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ON                 ,(uint32_t)p.BSS_ON             );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_VER                ,(uint32_t)p.BSS_VER            );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_GAIN_TH0           ,(uint32_t)p.BSS_GAIN_TH0       );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_GAIN_TH1           ,(uint32_t)p.BSS_GAIN_TH1       );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_MIN_ISP_GAIN       ,(uint32_t)p.BSS_MIN_ISP_GAIN   );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_LCSO_SIZE          ,(uint32_t)p.BSS_LCSO_SIZE      );
        /* YPF info */
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_EN             ,(uint32_t)p.BSS_YPF_EN         );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_FAC            ,(uint32_t)p.BSS_YPF_FAC        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_ADJTH          ,(uint32_t)p.BSS_YPF_ADJTH      );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_DFMED0         ,(uint32_t)p.BSS_YPF_DFMED0     );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_DFMED1         ,(uint32_t)p.BSS_YPF_DFMED1     );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_TH0            ,(uint32_t)p.BSS_YPF_TH0        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_TH1            ,(uint32_t)p.BSS_YPF_TH1        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_TH2            ,(uint32_t)p.BSS_YPF_TH2        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_TH3            ,(uint32_t)p.BSS_YPF_TH3        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_TH4            ,(uint32_t)p.BSS_YPF_TH4        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_TH5            ,(uint32_t)p.BSS_YPF_TH5        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_TH6            ,(uint32_t)p.BSS_YPF_TH6        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_YPF_TH7            ,(uint32_t)p.BSS_YPF_TH7        );
        /* FD & eye info*/
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_FD_EN              ,(uint32_t)p.BSS_FD_EN          );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_FD_FAC             ,(uint32_t)p.BSS_FD_FAC         );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_FD_FNUM            ,(uint32_t)p.BSS_FD_FNUM        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_EYE_EN             ,(uint32_t)p.BSS_EYE_EN         );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_EYE_CFTH           ,(uint32_t)p.BSS_EYE_CFTH       );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_EYE_RATIO0         ,(uint32_t)p.BSS_EYE_RATIO0     );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_EYE_RATIO1         ,(uint32_t)p.BSS_EYE_RATIO1     );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_EYE_FAC            ,(uint32_t)p.BSS_EYE_FAC        );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_AEVC_EN            ,(uint32_t)p.BSS_AEVC_EN        );
#   endif
#endif
        auto b = mtkBss->BssFeatureCtrl(BSS_FTCTRL_SET_PROC_INFO, (void*)&p, NULL);
        if (b != S_BSS_OK) {
            mfllLogE("%s: set info to MTKBss failed (%d)", __FUNCTION__, (int)b);
            goto lbExit;
        }
    }

    /* main process */
    {
        BSS_OUTPUT_DATA outParam;
        BSS_INPUT_DATA_G inParam;
        memset(&outParam, 0, sizeof(outParam));
        memset(&inParam, 0, sizeof(inParam));

        inParam.Bitnum = bitNum;
        inParam.BayerOrder = bayerOrder;
        inParam.Stride = stride;
        inParam.inWidth = width;
        inParam.inHeight = height;
        /* print out inParam information */
        mfllLogD3("%s: Bitnum     = %d", __FUNCTION__, inParam.Bitnum);
        mfllLogD3("%s: BayerOrder = %u", __FUNCTION__, inParam.BayerOrder);
        mfllLogD3("%s: Stride     = %u", __FUNCTION__, inParam.Stride);
        mfllLogD3("%s: inWidth    = %u", __FUNCTION__, inParam.inWidth);
        mfllLogD3("%s: inHeight   = %u", __FUNCTION__, inParam.inHeight);
#if (MFLL_MF_TAG_VERSION > 0)
        unsigned int dbgIndex = (unsigned int)MF_TAG_GMV_00;
#endif
        for (size_t i = 0; i < size; i++) {
            inParam.apbyBssInImg[i] = (MUINT8*)imgs[i]->getVa();
            inParam.gmv[i].x = mvs[i].x;
            inParam.gmv[i].y = mvs[i].y;

#if (MFLL_MF_TAG_VERSION > 0)
            /* update debug info */
            m_pCore->updateExifInfo(
                    dbgIndex + i,
                    mfll::makeGmv32bits((short)mvs[i].x, (short)mvs[i].y)
                    );
#endif
        }

        /* set fd info */
        {
            // 1. create IFDContainer instance
            auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);

            // 2. query fd info by timestamps, fdData must be return after use
            auto fdData = fdReader->queryLock(timestamps);

            if (MfllProperty::readProperty(Property_BssFdDump) == 1)
                fdReader->dumpInfo();

            // 3. fill fd info to bss
            {
                if (CC_LIKELY( fdData.size() == size )) {
                    for (size_t idx = 0 ; idx < fdData.size() ; idx++) {
                        if (fdData[idx] != nullptr)
                            inParam.Face[idx] = &fdData[idx]->facedata;
                        else
                            inParam.Face[idx] = nullptr;
                    }
                }
                else {
                    mfllLogE("%s: query fdData size is not sync. input_ts(%zu), query(%zu), expect(%zu)", __FUNCTION__, timestamps.size(), fdData.size(), size);
                }
            }
            // 4. fill fd rect0 to exif
            {
                for (size_t i = 0; i < fdData.size(); i++) {
                    if (fdData[i] != nullptr) {
                        m_pCore->updateExifInfo(MF_TAG_FD_RECT0_X0_00 + i*MFLLBSS_FD_RECT0_PER_FRAME, (uint32_t)fdData[i]->faces[0].rect[0]);
                        m_pCore->updateExifInfo(MF_TAG_FD_RECT0_Y0_00 + i*MFLLBSS_FD_RECT0_PER_FRAME, (uint32_t)fdData[i]->faces[0].rect[1]);
                        m_pCore->updateExifInfo(MF_TAG_FD_RECT0_X1_00 + i*MFLLBSS_FD_RECT0_PER_FRAME, (uint32_t)fdData[i]->faces[0].rect[2]);
                        m_pCore->updateExifInfo(MF_TAG_FD_RECT0_Y1_00 + i*MFLLBSS_FD_RECT0_PER_FRAME, (uint32_t)fdData[i]->faces[0].rect[3]);
                    }
                }
            }

            // 5. fdData must be return after use
            fdReader->queryUnlock(fdData);

            // 6. dump bss input info to text file for bss simulation
            if (MfllProperty::readProperty(Property_DumpRaw) == 1) {
                char filepath[256] = {0};
                snprintf(filepath, sizeof(filepath)-1, "%s/%09d-%04d-%04d-""%s", MFLLBSS_DUMP_PATH, m_uniqueKey, 0, 0, MFLLBSS_DUMP_FILENAME);
                mDumpBssInputData2File(filepath, inParam);
            }
        }

        auto b = mtkBss->BssMain(BSS_PROC2, &inParam, &outParam);
        if (b != S_BSS_OK) {
            mfllLogE("%s: MTKBss::Main returns failed (%d)", __FUNCTION__, (int)b);
            goto lbExit;
        }

        for (size_t i = 0; i < size; i++) {
            newIndex.push_back(outParam.originalOrder[i]);
            mvs[i].x = outParam.gmv[i].x;
            mvs[i].y = outParam.gmv[i].y;
        }

        mfllLogD3("%s: test", __FUNCTION__);
        if (MfllProperty::getForceBssOrder(newIndex)) {
            mfllLogD3("%s: MTKBss:: forced bss order", __FUNCTION__);
        }

        // skip frame count
        m_skipFrmCnt = outParam.i4SkipFrmCnt;

#if (MFLL_MF_TAG_VERSION >= 9)
        /* bss result score */
        const size_t dbgIdxBssScoreCount = 8; // only 8 scores
        size_t dbgIdxBssScoreMSB = static_cast<size_t>(MF_TAG_BSS_FINAL_SCORE_00_MSB);
        size_t dbgIdxBssScoreLSB = static_cast<size_t>(MF_TAG_BSS_FINAL_SCORE_00_LSB);
        size_t dbgIdxBssSharpScoreMSB = static_cast<size_t>(MF_TAG_BSS_SHARP_SCORE_00_MSB);
        size_t dbgIdxBssSharpScoreLSB = static_cast<size_t>(MF_TAG_BSS_SHARP_SCORE_00_LSB);
#endif

        for (size_t i = 0; i < size; i++) {
            mfllLogD3("%s: SharpScore[%zu]  = %lld", __FUNCTION__, i, outParam.SharpScore[i]);
            mfllLogD3("%s: adj1_score[%zu]  = %lld", __FUNCTION__, i, outParam.adj1_score[i]);
            mfllLogD3("%s: adj2_score[%zu]  = %lld", __FUNCTION__, i, outParam.adj2_score[i]);
            mfllLogD3("%s: adj3_score[%zu]  = %lld", __FUNCTION__, i, outParam.adj3_score[i]);
            mfllLogD3("%s: final_score[%zu] = %lld", __FUNCTION__, i, outParam.final_score[i]);

#if (MFLL_MF_TAG_VERSION >= 9)
            /* update final scores */
            if (__builtin_expect( i < dbgIdxBssScoreCount, true )) {
                const long long mask32bits = 0x00000000FFFFFFFF;
                m_pCore->updateExifInfo(dbgIdxBssScoreMSB, (outParam.final_score[i] >> 32) & mask32bits);
                m_pCore->updateExifInfo(dbgIdxBssScoreLSB, outParam.final_score[i] & mask32bits);
                m_pCore->updateExifInfo(dbgIdxBssSharpScoreMSB, (outParam.SharpScore[i] >> 32) & mask32bits);
                m_pCore->updateExifInfo(dbgIdxBssSharpScoreLSB, outParam.SharpScore[i] & mask32bits);
            }
            dbgIdxBssScoreMSB++;
            dbgIdxBssScoreLSB++;
            dbgIdxBssSharpScoreMSB++;
            dbgIdxBssSharpScoreLSB++;
#endif
        }
    }

#if (MFLL_MF_TAG_VERSION > 0)
#   if (MFLL_MF_TAG_VERSION >= 9)
    // encoding for bss order
    {
        /** MF_TAG_BSS_ORDER_IDX
         *
         *  BSS order for top 8 frames (MSB -> LSB)
         *
         *  |     4       |     4       |     4       |     4       |     4       |     4       |     4       |     4       |
         *  | bssOrder[0] | bssOrder[1] | bssOrder[2] | bssOrder[3] | bssOrder[4] | bssOrder[5] | bssOrder[6] | bssOrder[7] |
         */

        uint32_t bssOrder = 0x0;
        size_t i = 0;

        for ( ; i < size && i < 8 ; i++)
            bssOrder = (bssOrder << 4) | ((uint32_t)newIndex[i]<0xf?(uint32_t)newIndex[i]:0xf);
        for ( ; i < 8 ; i++)
            bssOrder = (bssOrder << 4) | 0xf;

        m_pCore->updateExifInfo(MF_TAG_BSS_ORDER_IDX, bssOrder);
    }
#   endif
    m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_BEST_IDX, (uint32_t)newIndex[0]);
#endif

lbExit:
    if (mtkBss)
        mtkBss->destroyInstance();
    return newIndex;
}

bool MfllBss::getForceBss(void* param_addr, size_t param_size)
{
    if ( param_size != sizeof(BSS_PARAM_STRUCT)) {
        mfllLogE("%s: invalid sizeof param, param_size:%d, sizeof(BSS_PARAM_STRUCT):%d",
                 __FUNCTION__, param_size, sizeof(BSS_PARAM_STRUCT));
        return false;
    }

    int r = 0;
    bool isForceBssSetting = false;
    BSS_PARAM_STRUCT* param = reinterpret_cast<BSS_PARAM_STRUCT*>(param_addr);

    r = MfllProperty::readProperty(Property_BssOn);
    if (r != -1) {
        mfllLogI("%s: Force BSS_ON = %d (original:%d)", __FUNCTION__, r, param->BSS_ON);
        param->BSS_ON = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssRoiWidth);
    if (r != -1) {
        mfllLogI("%s: Force BSS_ROI_WIDTH = %d (original:%d)", __FUNCTION__, r, param->BSS_ROI_WIDTH);
        param->BSS_ROI_WIDTH = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssRoiHeight);
    if (r != -1) {
        mfllLogI("%s: Force BSS_ROI_HEIGHT = %d (original:%d)", __FUNCTION__, r, param->BSS_ROI_HEIGHT);
        param->BSS_ROI_HEIGHT = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssRoiX0);
    if (r != -1) {
        mfllLogI("%s: Force BSS_ROI_X0 = %d (original:%d)", __FUNCTION__, r, param->BSS_ROI_X0);
        param->BSS_ROI_X0 = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssRoiY0);
    if (r != -1) {
        mfllLogI("%s: Force BSS_ROI_Y0 = %d (original:%d)", __FUNCTION__, r, param->BSS_ROI_Y0);
        param->BSS_ROI_Y0 = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssScaleFactor);
    if (r != -1) {
        mfllLogI("%s: Force BSS_SCALE_FACTOR = %d (original:%d)", __FUNCTION__, r, param->BSS_SCALE_FACTOR);
        param->BSS_SCALE_FACTOR = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssClipTh0);
    if (r != -1) {
        mfllLogI("%s: Force BSS_CLIP_TH0 = %d (original:%d)", __FUNCTION__, r, param->BSS_CLIP_TH0);
        param->BSS_CLIP_TH0 = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssClipTh1);
    if (r != -1) {
        mfllLogI("%s: Force BSS_CLIP_TH1 = %d (original:%d)", __FUNCTION__, r, param->BSS_CLIP_TH1);
        param->BSS_CLIP_TH1 = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssZero);
    if (r != -1) {
        mfllLogI("%s: Force BSS_ZERO = %d (original:%d)", __FUNCTION__, r, param->BSS_ZERO);
        param->BSS_ZERO = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssAdfTh);
    if (r != -1) {
        mfllLogI("%s: Force BSS_ADF_TH = %d (original:%d)", __FUNCTION__, r, param->BSS_ADF_TH);
        param->BSS_ADF_TH = r;
        isForceBssSetting = true;
    }

    r = MfllProperty::readProperty(Property_BssSdfTh);
    if (r != -1) {
        mfllLogI("%s: Force BSS_SDF_TH = %d (original:%d)", __FUNCTION__, r, param->BSS_SDF_TH);
        param->BSS_SDF_TH = r;
        isForceBssSetting = true;
    }

    return isForceBssSetting;
}

static bool mDumpBssInputData2File(char const* sFilepath, BSS_INPUT_DATA_G& inParam)
{
    if (sFilepath == nullptr) {
        mfllLogW("%s: output file path is null", __FUNCTION__);
        return false;
    }

    {
        //dump txt
        std::ofstream ofs (sFilepath, std::ofstream::out);

        if (!ofs.is_open()) {
            mfllLogW("%s: open file(%s) fail", __FUNCTION__, sFilepath);
            return false;
        }

#define MFLLBSS_WRITE_TO_FILE(pre, val) ofs << pre << " = " << val << std::endl
#define MFLLBSS_WRITE_ARRAY_TO_FILE(pre, array, size) \
        do { \
            ofs << pre << " = "; \
            for (int i = 0 ; i  < size ; i++) { \
                if (i != size-1) \
                    ofs << array[i] << ","; \
                else \
                    ofs << array[i] << std::endl; \
            } \
            if (size == 0) \
                ofs << std::endl; \
        } while (0)
#define MFLLBSS_WRITE_ARRAY_TO_FILE_CAST(pre, array, size) \
                    do { \
                        ofs << pre << " = "; \
                        for (int i = 0 ; i  < size ; i++) { \
                            if (i != size-1) \
                                ofs << static_cast<int32_t>(array[i]) << ","; \
                            else \
                                ofs << static_cast<int32_t>(array[i]) << std::endl; \
                        } \
                        if (size == 0) \
                            ofs << std::endl; \
                    } while (0)
#define MFLLBSS_WRITE_ARRAY_2D_TO_FILE(pre, array, M, N) \
                    do { \
                        ofs << pre << " = "; \
                        for (int i = 0 ; i  < M ; i++) { \
                            ofs << "{"; \
                            for (int j = 0 ; j  < N ; j++) { \
                                ofs << array[i][j]; \
                                if (j != N-1) \
                                    ofs << ","; \
                            } \
                            if (i != M-1) \
                                ofs << "},"; \
                            else \
                                ofs << "}" << std::endl; \
                        } \
                        if (M == 0) \
                            ofs << std::endl; \
                    } while (0)


        for (int f = 0 ; f < MAX_FRAME_NUM ; f++) {
            MFLLBSS_WRITE_TO_FILE("FD frame ", f);
            MtkCameraFaceMetadata* facedata = inParam.Face[f];

            if (facedata == nullptr)
                continue;


            MFLLBSS_WRITE_TO_FILE("number_of_faces",            facedata->number_of_faces);

            //MtkCameraFace
            if (facedata->faces != nullptr) {
                MFLLBSS_WRITE_ARRAY_TO_FILE("faces->rect",          facedata->faces->rect, 4);
                MFLLBSS_WRITE_TO_FILE("faces->score",               facedata->faces->score);
                MFLLBSS_WRITE_TO_FILE("faces->id",                  facedata->faces->id);
                MFLLBSS_WRITE_ARRAY_TO_FILE("faces->left_eye",      facedata->faces->left_eye, 2);
                MFLLBSS_WRITE_ARRAY_TO_FILE("faces->right_eye",     facedata->faces->right_eye, 2);
                MFLLBSS_WRITE_ARRAY_TO_FILE("faces->mouth",         facedata->faces->mouth, 2);
            }

            //MtkFaceInfo
            if (facedata->posInfo != nullptr) {
                MFLLBSS_WRITE_TO_FILE("posInfo->rop_dir",           facedata->posInfo->rop_dir);
                MFLLBSS_WRITE_TO_FILE("posInfo->rip_dir",           facedata->posInfo->rip_dir);
            }

            MFLLBSS_WRITE_ARRAY_TO_FILE("faces_type",           facedata->faces_type, 15);
            MFLLBSS_WRITE_ARRAY_2D_TO_FILE("motion",            facedata->motion, 15, 2);
            MFLLBSS_WRITE_TO_FILE("ImgWidth",                   facedata->ImgWidth);
            MFLLBSS_WRITE_TO_FILE("ImgHeight",                  facedata->ImgHeight);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyex0",               facedata->leyex0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyey0",               facedata->leyey0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyex1",               facedata->leyex1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyey1",               facedata->leyey1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyex0",               facedata->reyex0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyey0",               facedata->reyey0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyex1",               facedata->reyex1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyey1",               facedata->reyey1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("nosex",                facedata->nosex, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("nosey",                facedata->nosey, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("mouthx0",              facedata->mouthx0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("mouthy0",              facedata->mouthy0, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("mouthx1",              facedata->mouthx1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("mouthy1",              facedata->mouthy1, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyeux",               facedata->leyeux, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyeuy",               facedata->leyeuy, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyedx",               facedata->leyedx, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("leyedy",               facedata->leyedy, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyeux",               facedata->reyeux, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyeuy",               facedata->reyeuy, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyedx",               facedata->reyedx, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("reyedy",               facedata->reyedy, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("fa_cv",                facedata->fa_cv, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("fld_rip",              facedata->fld_rip, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("fld_rop",              facedata->fld_rop, 15);
            MFLLBSS_WRITE_ARRAY_2D_TO_FILE("YUVsts",            facedata->YUVsts, 15, 5);
            MFLLBSS_WRITE_ARRAY_TO_FILE_CAST("fld_GenderLabel", facedata->fld_GenderLabel, 15);
            MFLLBSS_WRITE_ARRAY_TO_FILE("fld_GenderInfo",       facedata->fld_GenderInfo, 15);
            MFLLBSS_WRITE_TO_FILE("timestamp",                  facedata->timestamp);
            //MtkCNNFaceInfo
            MFLLBSS_WRITE_TO_FILE("CNNFaces.PortEnable",        facedata->CNNFaces.PortEnable);
            MFLLBSS_WRITE_TO_FILE("CNNFaces.IsTrueFace",        facedata->CNNFaces.IsTrueFace);
            MFLLBSS_WRITE_TO_FILE("CNNFaces.CnnResult0",        facedata->CNNFaces.CnnResult0);
            MFLLBSS_WRITE_TO_FILE("CNNFaces.CnnResult1",        facedata->CNNFaces.CnnResult1);
        }


#undef MFLLBSS_WRITE_TO_FILE
#undef MFLLBSS_WRITE_ARRAY_TO_FILE
#undef MFLLBSS_WRITE_ARRAY_TO_FILE_CAST
#undef MFLLBSS_WRITE_ARRAY_2D_TO_FILE

        ofs.close();
    }

    return true;
}
