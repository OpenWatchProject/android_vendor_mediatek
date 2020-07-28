/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#define LOG_TAG "MfllCore/Memc"

#include "MfllMemc.h"
#include "MfllUtilities.h"

#include <mtkcam3/feature/mfnr/MfllLog.h>
#include <mtkcam3/feature/mfnr/MfllProperty.h>

// AOPS
#include <cutils/compiler.h>

// CUSTOM (common)
#include <custom/debug_exif/dbg_exif_param.h>
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#endif

// CUSTOM (platform)
#if MTK_CAM_NEW_NVRAM_SUPPORT
#include "MfllNvram.h"
#endif
#include <camera_custom_nvram.h>

// AOSP
#include <cutils/compiler.h>

using android::sp;
using namespace mfll;

IMfllMemc* IMfllMemc::createInstance()
{
    return (IMfllMemc*)new MfllMemc();
}

void IMfllMemc::destroyInstance()
{
    decStrong((void*)this);
}

MfllMemc::MfllMemc()
: m_widthMe(0)
, m_heightMe(0)
, m_widthMc(0)
, m_heightMc(0)
, m_mcThreadNum(MFLL_MC_THREAD_NUM)
, m_isUsingFullMc(0)
, m_isIgnoredMc(0)
, m_bldType(0)
, m_currIso(0)
, m_pCore(NULL)
{
    m_mfbll = MTKMfbll::createInstance(DRV_MFBLL_OBJ_SW);
    if (CC_UNLIKELY(m_mfbll == NULL)) {
        mfllLogE("%s: create MTKMfbll failed", __FUNCTION__);
    }
}

MfllMemc::~MfllMemc()
{
    if (CC_UNLIKELY(m_mfbll))
        m_mfbll->destroyInstance();
}

enum MfllErr MfllMemc::init(sp<IMfllNvram> &nvramProvider)
{
    if (CC_UNLIKELY(m_widthMe == 0 || m_heightMe == 0)) {
        mfllLogE("%s: init MTKMfbll needs information of the resolution of ME buffer", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    if (CC_UNLIKELY(nvramProvider.get() == NULL)) {
        mfllLogE("%s: init MfllMemc failed due to no NVRAM provider", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    m_nvramProvider = nvramProvider;

    MFBLL_INIT_PARAM_STRUCT initParam;
    initParam.Proc1_imgW      = static_cast<MUINT16>(m_widthMe); // image width
    initParam.Proc1_imgH      = static_cast<MUINT16>(m_heightMe); // image height
    initParam.core_num        = static_cast<MUINT32>(m_mcThreadNum);
    initParam.Proc1_MPME      = 1;
    initParam.Proc1_ConfMap   = 1;
    initParam.Proc1_DS        = getMeDnRatio();
    initParam.Proc_422        = 0;
    initParam.Proc1_DSUS_mode = m_pCore->isDownscale();

    /* print init information */
    mfllLogD("%s: me size(%dx%d), core_num(%d), MPME(%d), ConfMap(%d), DS(%d), DSUS_mode(%d), Proc_422(%d)",
            __FUNCTION__,
            initParam.Proc1_imgW, initParam.Proc1_imgH, initParam.core_num,
            initParam.Proc1_MPME, initParam.Proc1_ConfMap, initParam.Proc1_DS, initParam.Proc1_DSUS_mode, initParam.Proc_422);

    if(m_mfbll->MfbllInit(&initParam, NULL) != S_MFBLL_OK) {
        mfllLogE("%s: init MTKMfbll failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    mfllLogD3("%s: init MEMC with MC threads num -> %d",
            __FUNCTION__,
            initParam.core_num);

    return MfllErr_Ok;
}

/******************************************************************************
 *
******************************************************************************/
enum MfllErr MfllMemc::getAlgorithmWorkBufferSize(size_t *size)
{
    enum MfllErr err = MfllErr_Ok;

    MFBLL_GET_PROC_INFO_STRUCT info;

    auto ret = m_mfbll->MfbllFeatureCtrl(
            MFBLL_FTCTRL_GET_PROC_INFO,
            NULL, // no need
            &info);

    if (CC_UNLIKELY(S_MFBLL_OK != ret)) {
        mfllLogE("%s: get MTKMfbll information failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    *size = (size_t)info.Ext_mem_size;
    mfllLogD3("%s: MTKMfbll working buffer size = %zu", __FUNCTION__, *size);
    return err;
}

enum MfllErr MfllMemc::motionEstimation()
{
    /* release soruce/destination of ME after motionEstimation() */
    typedef IMfllImageBuffer T;
    struct __ResourceHolder{
        sp<T> *a;
        sp<T> *b;
        __ResourceHolder(sp<T> *a, sp<T> *b)
        : a(a), b(b) {}
        ~__ResourceHolder()
        { *a = 0; *b = 0; }
    } __res_holder(&m_imgSrcMe, &m_imgRefMe);

    enum MfllErr err = MfllErr_Ok;
    if (CC_UNLIKELY(m_imgSrcMe.get() == NULL)) {
        mfllLogE("%s: cannot do ME due to no source image", __FUNCTION__);
        err = MfllErr_BadArgument   ;
    }
    if (CC_UNLIKELY(m_imgRefMe.get() == NULL)) {
        mfllLogE("%s: cannot do ME due to no destination image", __FUNCTION__);
        err = MfllErr_BadArgument;
    }
    if (CC_UNLIKELY(m_imgWorkingBuffer.get() == NULL)) {
        mfllLogE("%s: cannot do ME due to no working buffer", __FUNCTION__);
        err = MfllErr_BadArgument;
    }
    if (err != MfllErr_Ok)
        return err;

    mfllLogD3("%s: is full size MC = %d", __FUNCTION__, m_isUsingFullMc);

    /* read NVRAM for tuning data */
    unsigned int badRange = 255;
    unsigned int badTh    = 12707;
    unsigned int lvlvalue = 0;

    size_t chunkSize = 0;
    const char *pChunk = m_nvramProvider->getChunk(&chunkSize);
    if (CC_UNLIKELY(pChunk == NULL)) {
        mfllLogE("%s: read NVRAM failed, use default", __FUNCTION__);
    }
    else {
        char *pMutableChunk = const_cast<char*>(pChunk);
        NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunk);
        /* update bad range and bad threshold */
        badRange = pNvram->memc_bad_mv_range;
        badTh    = pNvram->memc_bad_mv_rate_th;

        // get memc_noise_level by the current ISO
        if      (m_currIso < static_cast<int>(pNvram->memc_iso_th0))
            lvlvalue = pNvram->memc_lv_val0;
        else if (m_currIso < static_cast<int>(pNvram->memc_iso_th1))
            lvlvalue = pNvram->memc_lv_val1;
        else if (m_currIso < static_cast<int>(pNvram->memc_iso_th2))
            lvlvalue = pNvram->memc_lv_val2;
        else if (m_currIso < static_cast<int>(pNvram->memc_iso_th3))
            lvlvalue = pNvram->memc_lv_val3;
        else if (m_currIso < static_cast<int>(pNvram->memc_iso_th4))
            lvlvalue = pNvram->memc_lv_val4;
        else
            lvlvalue = pNvram->memc_lv_val5;
    }

    mfllLogD3("%s: currISO=%d, Noise_lvl=%u, BldType=%d", __FUNCTION__,
            m_currIso, lvlvalue, m_bldType);

    MFBLL_SET_PROC_INFO_STRUCT param = {
        .workbuf_addr   = (MUINT8*)m_imgWorkingBuffer->getVa(),
        .buf_size       = static_cast<MUINT32>(m_imgWorkingBuffer->getRealBufferSize()),
        .Proc1_base     = (MUINT8*)m_imgSrcMe->getVa(),
        .Proc1_ref      = (MUINT8*)m_imgRefMe->getVa(),
        .Proc1_width    = (MUINT32)m_imgSrcMe->getAlignedWidth(), // the buffer width
        .Proc1_height   = (MUINT32)m_imgSrcMe->getAlignedHeight(), // the buffer height
        .Proc1_V1       = m_globalMv.x / (m_isUsingFullMc ? 1 : 2),
        .Proc1_V2       = m_globalMv.y / (m_isUsingFullMc ? 1 : 2),
        .Proc1_bad_range = static_cast<MUINT8>(badRange),
        .Proc1_bad_TH   = badTh,
        .Proc1_ImgFmt   = PROC1_FMT_Y,
        .Proc2_full_size = m_isUsingFullMc, // 0: MC 1/4 size, 1: MC full size
        .Noise_lvl      = static_cast<MINT8>(lvlvalue),
    };

    if (CC_UNLIKELY(getForceMe(reinterpret_cast<void*>(&param), sizeof(MFBLL_SET_PROC_INFO_STRUCT)))) {
        mfllLogI("%s: force set Me param as manual setting", __FUNCTION__);
    }

#if (MFLL_MF_TAG_VERSION > 0)
    /* update debug info */
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_BAD_MV_RANGE, (uint32_t)param.Proc1_bad_range);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_BAD_MB_TH, (uint32_t)param.Proc1_bad_TH);
#   if (MFLL_MF_TAG_VERSION >= 4)
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_NOISE_LVL, (uint32_t)param.Noise_lvl);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_MPME, 1);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_CONFMAP, 1);
#   endif
#endif

    /* print out information */
    mfllLogD3("%s:---------------------------", __FUNCTION__);
    mfllLogD3("%s: mc size %dx%d", __FUNCTION__, param.Proc1_width, param.Proc1_height);
    mfllLogD3("%s: mc v1,v2=%d,%d", __FUNCTION__, param.Proc1_V1, param.Proc1_V2);
    mfllLogD3("%s: mc badRange=%d", __FUNCTION__, param.Proc1_bad_range);
    mfllLogD3("%s: mc badTh=%d", __FUNCTION__, param.Proc1_bad_TH);
    mfllLogD3("%s: mc noise lvl=%d", __FUNCTION__, param.Noise_lvl);
    mfllLogD3("%s:---------------------------", __FUNCTION__);

    auto ret = m_mfbll->MfbllFeatureCtrl(MFBLL_FTCTRL_SET_PROC_INFO, &param, NULL);
    if (CC_UNLIKELY(S_MFBLL_OK != ret)) {
        mfllLogE("%s: MTKMfbll set proc1 info fail, skip MC too", __FUNCTION__);
        m_isIgnoredMc = 1;
        return MfllErr_UnexpectedError;
    }

    MFBLL_PROC1_OUT_STRUCT paramOut;

    if (CC_LIKELY(m_imgConfidenceMap.get())) {
        paramOut.pu1ConfMap = static_cast<MUINT8*>(m_imgConfidenceMap->getVa());
        paramOut.u4MapSize = static_cast<MUINT32>(m_imgConfidenceMap->getRealBufferSize());
    }
    else {
        mfllLogW("Confidence map is NULL, create one here.");
        auto _img = IMfllImageBuffer::createInstance();
        if (CC_UNLIKELY(_img == nullptr)) {
            mfllLogE("create IMfllImageBuffer instance failed");
            m_isIgnoredMc = 1;
            return MfllErr_UnexpectedError;
        }
        static const size_t _blockSize = 16;
#define __ALIGN(w, a) (((w + (a-1)) / a) * a)
        _img->setResolution(
                __ALIGN(m_widthMe,  (_blockSize >> 1)) / (_blockSize >> 1),
                __ALIGN(m_heightMe, (_blockSize >> 1)) / (_blockSize >> 1)
                );
#undef __ALIGN
        _img->setImageFormat(ImageFormat_Raw8);
        _img->setAligned(1, 1);
        if (CC_UNLIKELY(_img->initBuffer() != MfllErr_Ok)) {
            mfllLogE("init confidence map image buffer failed");
            m_isIgnoredMc = 1;
            return MfllErr_UnexpectedError;
        }
        paramOut.pu1ConfMap = static_cast<MUINT8*>(_img->getVa());
        paramOut.u4MapSize = static_cast<MUINT32>(_img->getRealBufferSize());
    }

    mfllLogD3("%s: Confidence map: (addr, size)=(%p, %u)", __FUNCTION__,
            paramOut.pu1ConfMap, paramOut.u4MapSize);

    ret = m_mfbll->MfbllMain(MFBLL_PROC1, NULL, &paramOut);
    if (CC_UNLIKELY(S_MFBLL_OK != ret)) {
        mfllLogE("%s: do ME failed", __FUNCTION__);
        m_isIgnoredMc = 1;
        return MfllErr_UnexpectedError;
    }

    mfllLogD3("%s: is skip MC --> %d", __FUNCTION__, paramOut.bSkip_proc);
    m_isIgnoredMc = paramOut.bSkip_proc;

    return MfllErr_Ok;
}


enum MfllErr MfllMemc::motionCompensation()
{
    /* release soruce/destination of ME after motionEstimation() */
    typedef IMfllImageBuffer T;
    struct __ResourceHolder{
        sp<T> *a;
        sp<T> *b;
        sp<T> *c;
        __ResourceHolder(sp<T> *a, sp<T> *b, sp<T> *c)
        : a(a), b(b), c(c) {}
        ~__ResourceHolder()
        { *a = 0; *b = 0; *c=0; }
    } __res_holder(&m_imgRefMc, &m_imgDstMc, &m_imgWorkingBuffer);

    enum MfllErr err = MfllErr_Ok;
    /* Check if the MC should be ignored */
    if (isIgnoredMotionCompensation()) {
        mfllLogD3("%s: Ignored MC", __FUNCTION__);
        return MfllErr_Ok;
    }

    if (CC_UNLIKELY(m_imgDstMc.get() == NULL)) {
        mfllLogE("%s: cannot do MC due to no destination image", __FUNCTION__);
        err = MfllErr_BadArgument;
    }
    if (CC_UNLIKELY(m_imgRefMc.get() == NULL)) {
        mfllLogE("%s: cannot do MC due to no reference image", __FUNCTION__);
        err = MfllErr_BadArgument;
    }
    if (CC_UNLIKELY(err != MfllErr_Ok))
        return err;

    MFBLL_PROC2_OUT_STRUCT param = {
        .ImgFmt         = PROC1_FMT_NV12,
        /* reference */
        .pbyInImg       = (MUINT8*)m_imgRefMc->getVa(),
        .i4InWidth      = (MUINT32)m_imgRefMc->getAlignedWidth(), // the buffer width
        .i4InHeight     = (MUINT32)m_imgRefMc->getAlignedHeight(), // the buffer height
        /* destination */
        .pbyOuImg       = (MUINT8*)m_imgDstMc->getVa(),
        .i4OuWidth      = (MUINT32)m_imgDstMc->getAlignedWidth(), // the image width
        .i4OuHeight     = (MUINT32)m_imgDstMc->getAlignedHeight() // the image height
    };

    if (CC_UNLIKELY(   m_imgDstMc->getWidth() !=  m_imgDstMc->getAlignedWidth()
                    || m_imgDstMc->getHeight() !=  m_imgDstMc->getAlignedHeight() )) {
        mfllLogI("%s: update mc output size(%dx%d) to aligned size(%dx%d)", __FUNCTION__
            , m_imgDstMc->getWidth(), m_imgDstMc->getHeight()
            , m_imgDstMc->getAlignedWidth(), m_imgDstMc->getAlignedHeight());
    }

#if (MFLL_MF_TAG_VERSION > 0)
    /* update dbg info */
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_INPUT_FORMAT, (uint32_t)PROC1_FMT_NV12);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_OUTPUT_FORMAT, (uint32_t)param.ImgFmt);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_WIDTH, (uint32_t)param.i4OuWidth);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_HEIGHT, (uint32_t)param.i4OuHeight);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_MCFULLSIZE, (uint32_t)m_isUsingFullMc);
#endif

    mfllLogD3("%s: input size:  %dx%d", __FUNCTION__, param.i4InWidth, param.i4InHeight);
    mfllLogD3("%s: output size: %dx%d", __FUNCTION__, param.i4OuWidth, param.i4OuHeight);

    auto ret = m_mfbll->MfbllMain(MFBLL_PROC2, NULL, &param);
    if (CC_UNLIKELY(S_MFBLL_OK != ret)) {
        mfllLogE("%s: do MC failed", __FUNCTION__);
        m_isIgnoredMc = 1;
        return MfllErr_UnexpectedError;
    }

    mfllLogD3("do MC ok!");

    return MfllErr_Ok;
}

bool MfllMemc::getForceMe(void* param_addr, size_t param_size)
{
    if ( param_size != sizeof(MFBLL_SET_PROC_INFO_STRUCT)) {
        mfllLogE("%s: invalid sizeof param, param_size:%zu, sizeof(MFBLL_SET_PROC_INFO_STRUCT):%zu",
                 __FUNCTION__, param_size, sizeof(MFBLL_SET_PROC_INFO_STRUCT));
        return false;
    }

    int r = 0;
    bool isForceMeSetting = false;
    MFBLL_SET_PROC_INFO_STRUCT* param = reinterpret_cast<MFBLL_SET_PROC_INFO_STRUCT*>(param_addr);

    r = MfllProperty::readProperty(Property_MvBadRange);
    if (r != -1) {
        mfllLogI("%s: Force Proc1_bad_range = %d (original:%d)", __FUNCTION__, r, param->Proc1_bad_range);
        param->Proc1_bad_range = r;
        isForceMeSetting = true;
    }

    r = MfllProperty::readProperty(Property_MbBadTh);
    if (r != -1) {
        mfllLogI("%s: Force Proc1_bad_TH = %d (original:%d)", __FUNCTION__, r, param->Proc1_bad_TH);
        param->Proc1_bad_TH = r;
        isForceMeSetting = true;
    }

    return isForceMeSetting;
}
