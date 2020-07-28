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

#include <mtkcam3/feature/mfnr/MfllLog.h>

// CUSTOM (common)
#include <custom/debug_exif/dbg_exif_param.h>
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#endif

// CUSTOM (platform)
#include <camera_custom_nvram.h>

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
, m_pCore(NULL)
{
    m_mfbll = MTKMfbll::createInstance(DRV_MFBLL_OBJ_SW);
    if (m_mfbll == NULL) {
        mfllLogE("%s: create MTKMfbll failed", __FUNCTION__);
    }
}

MfllMemc::~MfllMemc()
{
    if (m_mfbll)
        m_mfbll->destroyInstance();
}

enum MfllErr MfllMemc::init(sp<IMfllNvram> &nvramProvider)
{
    if (m_widthMe == 0 || m_heightMe == 0) {
        mfllLogE("%s: init MTKMfbll needs information of the resolution of ME buffer", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    if (nvramProvider.get() == NULL) {
        mfllLogE("%s: init MfllMemc failed due to no NVRAM provider", __FUNCTION__);
        return MfllErr_BadArgument;
    }

    m_nvramProvider = nvramProvider;

    size_t chunkSize = 0;
    unsigned int maxFrameNum =4;
    const char *pChunk = m_nvramProvider->getChunk(&chunkSize);
    if (pChunk == NULL) {
        mfllLogE("%s: read NVRAM failed, use default", __FUNCTION__);
    }
    else {
        char *pMutableChunk = const_cast<char*>(pChunk);
        NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunk);
        /* update maxFrameNum */
        maxFrameNum = pNvram->max_frame_number;
    }

    MFBLL_INIT_PARAM_STRUCT initParam = {
        .Proc1_Init.Proc1_imgW = static_cast<MUINT16>(m_widthMe), // image width
        .Proc1_Init.Proc1_imgH = static_cast<MUINT16>(m_heightMe), // image height
        .Proc1_Init.Proc1_tuning.core_num = 4,
        .Proc1_Init.Proc1_tuning.ISO_Value = 0,
        .Proc1_Init.Proc1_tuning.ISO_HiTh = 0,
        .Proc1_Init.Proc1_tuning.ISO_LowTh = 0,
        .Proc1_Init.Proc1_tuning.MaxFrmNum = maxFrameNum,
    };

    if(m_mfbll->MfbllInit(&initParam, NULL) != S_MFBLL_OK) {
        mfllLogE("%s: init MTKMfbll failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    mfllLogD("%s: init MEMC with MC threads num -> %d",
            __FUNCTION__,
            initParam.Proc1_Init.Proc1_tuning.core_num);

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

    if (S_MFBLL_OK != ret) {
        mfllLogE("%s: get MTKMfbll information failed", __FUNCTION__);
        return MfllErr_UnexpectedError;
    }

    *size = (size_t)info.Ext_mem_size;
    mfllLogD("%s: MTKMfbll working buffer size = %zu", __FUNCTION__, *size);
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
    if (m_imgSrcMe.get() == NULL) {
        mfllLogE("%s: cannot do ME due to no source image", __FUNCTION__);
        err = MfllErr_BadArgument   ;
    }
    if (m_imgRefMe.get() == NULL) {
        mfllLogE("%s: cannot do ME due to no destination image", __FUNCTION__);
        err = MfllErr_BadArgument;
    }
    if (m_imgWorkingBuffer.get() == NULL) {
        mfllLogE("%s: cannot do ME due to no working buffer", __FUNCTION__);
        err = MfllErr_BadArgument;
    }
    if (err != MfllErr_Ok)
        return err;

    mfllLogD("%s: is full size MC = %d", __FUNCTION__, m_isUsingFullMc);

    /* read NVRAM for tuning data */
    unsigned int badRange = 255;
    unsigned int badTh    = 12707;

    size_t chunkSize = 0;
    const char *pChunk = m_nvramProvider->getChunk(&chunkSize);
    if (pChunk == NULL) {
        mfllLogE("%s: read NVRAM failed, use default", __FUNCTION__);
    }
    else {
        char *pMutableChunk = const_cast<char*>(pChunk);
        NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunk);
        /* update bad range and bad threshold */
        badRange = pNvram->memc_bad_mv_range;
        badTh    = (pNvram->memc_bad_mv_rate_th/255.0)*((2*m_widthMe*2*m_heightMe)>>8);
    }

    MFBLL_SET_PROC1_INFO_STRUCT param = {
        .workbuf_addr   = (MUINT8*)m_imgWorkingBuffer->getVa(),
        .buf_size       = static_cast<MUINT32>(m_imgWorkingBuffer->getRealBufferSize()),
        .base     = (MUINT8*)m_imgSrcMe->getVa(),
        .ref      = (MUINT8*)m_imgRefMe->getVa(),
        .Width    = (MUINT32)m_imgSrcMe->getAlignedWidth(), // the buffer width
        .Height   = (MUINT32)m_imgSrcMe->getAlignedHeight(), // the buffer height
        .Proc1_V1       = m_globalMv.x / (m_isUsingFullMc ? 1 : 2),
        .Proc1_V2       = m_globalMv.y / (m_isUsingFullMc ? 1 : 2),
        .bad_mv_range = static_cast<MUINT8>(badRange),
        .bad_mv_TH   = badTh,
        .ImgFmt   = PROC1_FMT_YV16
    };

#if (MFLL_MF_TAG_VERSION > 0)
    /* update debug info */
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_BAD_MV_RANGE, (uint32_t)param.bad_mv_range);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_BAD_MB_TH, (uint32_t)param.bad_mv_TH);
#endif

    /* print out information */
    mfllLogD("%s:---------------------------", __FUNCTION__);
    mfllLogD("%s: mc buf_size=%d", __FUNCTION__,param.buf_size);
    mfllLogD("%s: mc size %dx%d", __FUNCTION__, param.Width, param.Height);
    mfllLogD("%s: mc v1,v2=%d,%d", __FUNCTION__, param.Proc1_V1, param.Proc1_V2);
    mfllLogD("%s: mc badRange=%d", __FUNCTION__, param.bad_mv_range);
    mfllLogD("%s: mc badTh=%d", __FUNCTION__, param.bad_mv_TH);
    mfllLogD("%s:---------------------------", __FUNCTION__);

    auto ret = m_mfbll->MfbllFeatureCtrl(MFBLL_FTCTRL_SET_PROC1_INFO, &param, &param);
    if (S_MFBLL_OK != ret) {
        mfllLogE("%s: MTKMfbll set proc1 info fail, skip MC too", __FUNCTION__);
        m_isIgnoredMc = 1;
        return MfllErr_UnexpectedError;
    }

    MFBLL_PROC_ME_OUT_STRUCT paramOut;
    ret = m_mfbll->MfbllMain(MFBLL_PROC_ME, NULL, &paramOut);
    if (S_MFBLL_OK != ret) {
        mfllLogE("%s: do ME failed", __FUNCTION__);
        m_isIgnoredMc = 1;
        return MfllErr_UnexpectedError;
    }

    mfllLogD("%s: is skip MC --> %d", __FUNCTION__, paramOut.bSkip_MEMC);
    m_isIgnoredMc = paramOut.bSkip_MEMC;

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

    /* Check if the MC should be ignored */
    if (isIgnoredMotionCompensation()) {
        mfllLogD("%s: Ignored MC", __FUNCTION__);
        return MfllErr_Ok;
    }

    MFBLL_PROC_MC_OUT_STRUCT param = {
        .ImgFmt         = PROC1_FMT_YUY2,
        .Height = (MUINT32)m_heightMe, // the buffer height
        .Width    = (MUINT32)m_widthMe, // the buffer width
        .ResultImg = NULL
    };

#if (MFLL_MF_TAG_VERSION > 0)
    /* update dbg info */
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_INPUT_FORMAT, (uint32_t)PROC1_FMT_YV16);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_OUTPUT_FORMAT, (uint32_t)param.ImgFmt);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_WIDTH, (uint32_t)param.Width);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_HEIGHT, (uint32_t)param.Height);
#endif

    auto ret = m_mfbll->MfbllMain(MFBLL_PROC_MC, NULL, &param);
    if (S_MFBLL_OK != ret) {
        mfllLogE("%s: do MC failed", __FUNCTION__);
        m_isIgnoredMc = 1;
        return MfllErr_UnexpectedError;
    }

    mfllLogD("do MC ok!");

    return MfllErr_Ok;
}

