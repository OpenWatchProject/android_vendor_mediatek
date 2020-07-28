/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#include <mtkcam/utils/hw/IFDContainer.h>
#include <mtkcam/utils/hw/IBssContainer.h>


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

#include <fstream>


#define MFLLMEMC_DUMP_PATH              "/data/vendor/camera_dump/"
#define MFLLMEMC_DUMP_PARAM_FILENAME    "memc-param.bin"
#define MFLLMEMC_DUMP_OUT_FILENAME      "memc-out.bin"

#define __TRANS_FD_TO_NOR(value, max)           (((value*2000)+(max/2))/max-1000)
#define __TRANS_NOR_TO_ME(value, max)           (((value+1000)*max+1000)/2000)
#define __TRANS_FD_TO_ME(value, maxFD, maxME)   __TRANS_NOR_TO_ME(__TRANS_FD_TO_NOR(value, maxFD), maxME)


using android::sp;
using namespace mfll;
using namespace NSCam;

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
, m_frameLevelConfidence(0)
, m_bldType(0)
, m_currIso(0)
, m_dump_uniqueKey(0)
, m_dump_requestNum(0)
, m_dump_framNum(0)
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
    //initParam.Proc1_MPME      = 1;
    //initParam.Proc1_ConfMap   = 1;
    //initParam.Proc1_DS        = getMeDnRatio();
    //initParam.Proc_422        = 0;
    initParam.Proc1_DSUS_mode = m_pCore->isMemcDownscale();

    /* print init information */
    mfllLogD("%s: me size(%dx%d), core_num(%d), DSUS_mode(%d)",
            __FUNCTION__,
            initParam.Proc1_imgW, initParam.Proc1_imgH, initParam.core_num, initParam.Proc1_DSUS_mode);

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
    const char *pChunk = m_nvramProvider->getChunkMfnr(&chunkSize);
    MFBLL_SET_PROC_INFO_STRUCT param;
    {
        param.workbuf_addr   = (MUINT8*)m_imgWorkingBuffer->getVa();
        param.buf_size       = static_cast<MUINT32>(m_imgWorkingBuffer->getRealBufferSize());
        param.Proc1_base     = (MUINT8*)m_imgSrcMe->getVa();
        param.Proc1_ref      = (MUINT8*)m_imgRefMe->getVa();
        param.Proc1_width    = (MUINT32)m_imgSrcMe->getAlignedWidth(); // the buffer width
        param.Proc1_height   = (MUINT32)m_imgSrcMe->getAlignedHeight(); // the buffer height
        param.Proc1_V1       = m_globalMv.x / (m_isUsingFullMc ? 1 : 2);
        param.Proc1_V2       = m_globalMv.y / (m_isUsingFullMc ? 1 : 2);
        param.Proc1_bad_range = static_cast<MUINT8>(badRange);
        param.Proc1_bad_TH   = badTh;
        param.Proc1_ImgFmt   = PROC1_FMT_Y;
        param.Noise_lvl      = static_cast<MINT8>(lvlvalue);
        param.Proc1_noise_level = static_cast<MUINT32>(lvlvalue);
    }

    if (CC_UNLIKELY(pChunk == NULL)) {
        mfllLogE("%s: read NVRAM failed, use default", __FUNCTION__);
    } else {
        /* if there has nvram, update parameters from it */
        char *pMutableChunk = const_cast<char*>(pChunk);
        NVRAM_CAMERA_FEATURE_MFLL_STRUCT* pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunk);

        /* update bad range and bad threshold */
        badRange = pNvram->memc_bad_mv_range;
        badTh    = pNvram->memc_bad_mv_rate_th;
        // get memc_noise_level by the current ISO
        lvlvalue = pNvram->me_noise_lv;

        param.Proc1_bad_range = static_cast<MUINT8>(pNvram->memc_bad_mv_range);
        param.Proc1_bad_TH   = pNvram->memc_bad_mv_rate_th;
        param.Proc1_noise_level = (MUINT32)pNvram->me_noise_lv;
        {
            auto bssReader = IBssContainer::createInstance(LOG_TAG,  IBssContainer::eBssContainer_Opt_Read);
            auto bssData = bssReader->queryLock(m_dump_uniqueKey);

            if (bssData != nullptr) {
                auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
                vector<int64_t> vecTs;
                for (size_t i = 0 ; i < MAX_FRAME_NUM ; i++)
                    vecTs.push_back(bssData->timestamps[i]);
                auto fdData = fdReader->queryLock(vecTs);

                BSS_OUTPUT_DATA& bssOut = bssData->bssdata;
                auto bstIdx = bssOut.originalOrder[0];
                auto refIdx = bssOut.originalOrder[m_dump_framNum+1];

                mfllLogD3("%s: bstIdx=%d, refIdx=%d", __FUNCTION__, bstIdx, refIdx);

                MUINT32 face_count = 0;
                MUINT32 LEYE_X0 = 0;
                MUINT32 LEYE_X1 = 0;
                MUINT32 LEYE_Y0 = 0;
                MUINT32 LEYE_Y1 = 0;
                MUINT32 LEYE_UX = 0;
                MUINT32 LEYE_UY = 0;
                MUINT32 LEYE_DX = 0;
                MUINT32 LEYE_DY = 0;
                MUINT32 REYE_X0 = 0;
                MUINT32 REYE_X1 = 0;
                MUINT32 REYE_Y0 = 0;
                MUINT32 REYE_Y1 = 0;
                MUINT32 REYE_UX = 0;
                MUINT32 REYE_UY = 0;
                MUINT32 REYE_DX = 0;
                MUINT32 REYE_DY = 0;
                for (size_t i = 0 ; i < fdData.size() ; i++) {
                    if (fdData[i] == nullptr || fdData[i]->facedata.number_of_faces == 0)
                        continue;
                    face_count++;
                    LEYE_X0 += fdData[i]->facedata.leyex0[0];
                    LEYE_X1 += fdData[i]->facedata.leyex1[0];
                    LEYE_Y0 += fdData[i]->facedata.leyey0[0];
                    LEYE_Y1 += fdData[i]->facedata.leyey1[0];
                    LEYE_UX += fdData[i]->facedata.leyeux[0];
                    LEYE_UY += fdData[i]->facedata.leyeuy[0];
                    LEYE_DX += fdData[i]->facedata.leyedx[0];
                    LEYE_DY += fdData[i]->facedata.leyedy[0];
                    REYE_X0 += fdData[i]->facedata.reyex0[0];
                    REYE_X1 += fdData[i]->facedata.reyex1[0];
                    REYE_Y0 += fdData[i]->facedata.reyey0[0];
                    REYE_Y1 += fdData[i]->facedata.reyey1[0];
                    REYE_UX += fdData[i]->facedata.reyeux[0];
                    REYE_UY += fdData[i]->facedata.reyeuy[0];
                    REYE_DX += fdData[i]->facedata.reyedx[0];
                    REYE_DY += fdData[i]->facedata.reyedy[0];
                }

                auto fdW = m_widthMe*getMeDnRatio();
                auto fdH = m_heightMe*getMeDnRatio();
                if (fdW > 0 && fdH > 0) {
                    if (fdData[bstIdx] != nullptr && fdData[bstIdx]->facedata.number_of_faces > 0) {
                        param.Proc1_Bst_FDROI_X0 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.faces->rect[0], fdW, m_widthMc);
                        param.Proc1_Bst_FDROI_Y0 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.faces->rect[1], fdH, m_heightMc);
                        param.Proc1_Bst_FDROI_X1 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.faces->rect[2], fdW, m_widthMc);
                        param.Proc1_Bst_FDROI_Y1 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.faces->rect[3], fdH, m_heightMc);
                        //
                        param.Proc1_Bst_LEYE_X0 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.leyex0[0], fdW, m_widthMc);
                        param.Proc1_Bst_LEYE_X1 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.leyex1[0], fdW, m_widthMc);
                        param.Proc1_Bst_LEYE_Y0 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.leyey0[0], fdH, m_heightMc);
                        param.Proc1_Bst_LEYE_Y1 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.leyey1[0], fdH, m_heightMc);
                        param.Proc1_Bst_LEYE_UX = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.leyeux[0], fdW, m_widthMc);
                        param.Proc1_Bst_LEYE_UY = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.leyeuy[0], fdH, m_heightMc);
                        param.Proc1_Bst_LEYE_DX = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.leyedx[0], fdW, m_widthMc);
                        param.Proc1_Bst_LEYE_DY = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.leyedy[0], fdH, m_heightMc);
                        param.Proc1_Bst_REYE_X0 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.reyex0[0], fdW, m_widthMc);
                        param.Proc1_Bst_REYE_X1 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.reyex1[0], fdW, m_widthMc);
                        param.Proc1_Bst_REYE_Y0 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.reyey0[0], fdH, m_heightMc);
                        param.Proc1_Bst_REYE_Y1 = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.reyey1[0], fdH, m_heightMc);
                        param.Proc1_Bst_REYE_UX = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.reyeux[0], fdW, m_widthMc);
                        param.Proc1_Bst_REYE_UY = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.reyeuy[0], fdH, m_heightMc);
                        param.Proc1_Bst_REYE_DX = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.reyedx[0], fdW, m_widthMc);
                        param.Proc1_Bst_REYE_DY = (MUINT32)__TRANS_FD_TO_ME(fdData[bstIdx]->facedata.reyedy[0], fdH, m_heightMc);
                    } else if (fdData[bstIdx] == nullptr && face_count > 0) {
                        param.Proc1_Bst_LEYE_X0 = (MUINT32)__TRANS_FD_TO_ME((LEYE_X0/face_count), fdW, m_widthMc);
                        param.Proc1_Bst_LEYE_X1 = (MUINT32)__TRANS_FD_TO_ME((LEYE_X1/face_count), fdW, m_widthMc);
                        param.Proc1_Bst_LEYE_Y0 = (MUINT32)__TRANS_FD_TO_ME((LEYE_Y0/face_count), fdH, m_heightMc);
                        param.Proc1_Bst_LEYE_Y1 = (MUINT32)__TRANS_FD_TO_ME((LEYE_Y1/face_count), fdH, m_heightMc);
                        param.Proc1_Bst_LEYE_UX = (MUINT32)__TRANS_FD_TO_ME((LEYE_UX/face_count), fdW, m_widthMc);
                        param.Proc1_Bst_LEYE_UY = (MUINT32)__TRANS_FD_TO_ME((LEYE_UY/face_count), fdH, m_heightMc);
                        param.Proc1_Bst_LEYE_DX = (MUINT32)__TRANS_FD_TO_ME((LEYE_DX/face_count), fdW, m_widthMc);
                        param.Proc1_Bst_LEYE_DY = (MUINT32)__TRANS_FD_TO_ME((LEYE_DY/face_count), fdH, m_heightMc);
                        param.Proc1_Bst_REYE_X0 = (MUINT32)__TRANS_FD_TO_ME((REYE_X0/face_count), fdW, m_widthMc);
                        param.Proc1_Bst_REYE_X1 = (MUINT32)__TRANS_FD_TO_ME((REYE_X1/face_count), fdW, m_widthMc);
                        param.Proc1_Bst_REYE_Y0 = (MUINT32)__TRANS_FD_TO_ME((REYE_Y0/face_count), fdH, m_heightMc);
                        param.Proc1_Bst_REYE_Y1 = (MUINT32)__TRANS_FD_TO_ME((REYE_Y1/face_count), fdH, m_heightMc);
                        param.Proc1_Bst_REYE_UX = (MUINT32)__TRANS_FD_TO_ME((REYE_UX/face_count), fdW, m_widthMc);
                        param.Proc1_Bst_REYE_UY = (MUINT32)__TRANS_FD_TO_ME((REYE_UY/face_count), fdH, m_heightMc);
                        param.Proc1_Bst_REYE_DX = (MUINT32)__TRANS_FD_TO_ME((REYE_DX/face_count), fdW, m_widthMc);
                        param.Proc1_Bst_REYE_DY = (MUINT32)__TRANS_FD_TO_ME((REYE_DY/face_count), fdH, m_heightMc);
                    }
                    //
                    if (fdData[refIdx] != nullptr && fdData[refIdx]->facedata.number_of_faces > 0) {
                        param.Proc1_Ref_FDROI_X0 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.faces->rect[0], fdW, m_widthMc);
                        param.Proc1_Ref_FDROI_Y0 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.faces->rect[1], fdH, m_heightMc);
                        param.Proc1_Ref_FDROI_X1 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.faces->rect[2], fdW, m_widthMc);
                        param.Proc1_Ref_FDROI_Y1 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.faces->rect[3], fdH, m_heightMc);
                        //
                        param.Proc1_Ref_LEYE_X0 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.leyex0[0], fdW, m_widthMc);
                        param.Proc1_Ref_LEYE_X1 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.leyex1[0], fdW, m_widthMc);
                        param.Proc1_Ref_LEYE_Y0 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.leyey0[0], fdH, m_heightMc);
                        param.Proc1_Ref_LEYE_Y1 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.leyey1[0], fdH, m_heightMc);
                        param.Proc1_Ref_LEYE_UX = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.leyeux[0], fdW, m_widthMc);
                        param.Proc1_Ref_LEYE_UY = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.leyeuy[0], fdH, m_heightMc);
                        param.Proc1_Ref_LEYE_DX = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.leyedx[0], fdW, m_widthMc);
                        param.Proc1_Ref_LEYE_DY = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.leyedy[0], fdH, m_heightMc);
                        param.Proc1_Ref_REYE_X0 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.reyex0[0], fdW, m_widthMc);
                        param.Proc1_Ref_REYE_X1 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.reyex1[0], fdW, m_widthMc);
                        param.Proc1_Ref_REYE_Y0 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.reyey0[0], fdH, m_heightMc);
                        param.Proc1_Ref_REYE_Y1 = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.reyey1[0], fdH, m_heightMc);
                        param.Proc1_Ref_REYE_UX = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.reyeux[0], fdW, m_widthMc);
                        param.Proc1_Ref_REYE_UY = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.reyeuy[0], fdH, m_heightMc);
                        param.Proc1_Ref_REYE_DX = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.reyedx[0], fdW, m_widthMc);
                        param.Proc1_Ref_REYE_DY = (MUINT32)__TRANS_FD_TO_ME(fdData[refIdx]->facedata.reyedy[0], fdH, m_heightMc);
                    } else if (fdData[refIdx] == nullptr && face_count > 0) {
                        param.Proc1_Ref_LEYE_X0 = (MUINT32)__TRANS_FD_TO_ME((LEYE_X0/face_count), fdW, m_widthMc);
                        param.Proc1_Ref_LEYE_X1 = (MUINT32)__TRANS_FD_TO_ME((LEYE_X1/face_count), fdW, m_widthMc);
                        param.Proc1_Ref_LEYE_Y0 = (MUINT32)__TRANS_FD_TO_ME((LEYE_Y0/face_count), fdH, m_heightMc);
                        param.Proc1_Ref_LEYE_Y1 = (MUINT32)__TRANS_FD_TO_ME((LEYE_Y1/face_count), fdH, m_heightMc);
                        param.Proc1_Ref_LEYE_UX = (MUINT32)__TRANS_FD_TO_ME((LEYE_UX/face_count), fdW, m_widthMc);
                        param.Proc1_Ref_LEYE_UY = (MUINT32)__TRANS_FD_TO_ME((LEYE_UY/face_count), fdH, m_heightMc);
                        param.Proc1_Ref_LEYE_DX = (MUINT32)__TRANS_FD_TO_ME((LEYE_DX/face_count), fdW, m_widthMc);
                        param.Proc1_Ref_LEYE_DY = (MUINT32)__TRANS_FD_TO_ME((LEYE_DY/face_count), fdH, m_heightMc);
                        param.Proc1_Ref_REYE_X0 = (MUINT32)__TRANS_FD_TO_ME((REYE_X0/face_count), fdW, m_widthMc);
                        param.Proc1_Ref_REYE_X1 = (MUINT32)__TRANS_FD_TO_ME((REYE_X1/face_count), fdW, m_widthMc);
                        param.Proc1_Ref_REYE_Y0 = (MUINT32)__TRANS_FD_TO_ME((REYE_Y0/face_count), fdH, m_heightMc);
                        param.Proc1_Ref_REYE_Y1 = (MUINT32)__TRANS_FD_TO_ME((REYE_Y1/face_count), fdH, m_heightMc);
                        param.Proc1_Ref_REYE_UX = (MUINT32)__TRANS_FD_TO_ME((REYE_UX/face_count), fdW, m_widthMc);
                        param.Proc1_Ref_REYE_UY = (MUINT32)__TRANS_FD_TO_ME((REYE_UY/face_count), fdH, m_heightMc);
                        param.Proc1_Ref_REYE_DX = (MUINT32)__TRANS_FD_TO_ME((REYE_DX/face_count), fdW, m_widthMc);
                        param.Proc1_Ref_REYE_DY = (MUINT32)__TRANS_FD_TO_ME((REYE_DY/face_count), fdH, m_heightMc);
                    }
                    mfllLogD("%s: pNvram->ext_setting = %d", __FUNCTION__, pNvram->ext_setting);
                    if (pNvram->ext_setting == 0) {
                        param.Proc1_me_large_mv_thd = 120;
                        param.Proc1_me_large_mv_SAD_thd = 1500;
                        param.Proc1_me_large_mv_ratio = 3;
                        param.Proc1_me_large_mv_txtr_en = 1;
                        param.Proc1_me_large_mv_txtr_wei = 12;
                        param.Proc1_me_large_mv_txtr_thd = 3;
                        param.Proc1_me_blink_eye_en = MFALSE;
                    } else {
                        param.Proc1_me_large_mv_thd = (MUINT32)pNvram->me_large_mv_thd;
                        param.Proc1_me_large_mv_SAD_thd = (MUINT32)pNvram->me_large_mv_SAD_thd;
                        param.Proc1_me_large_mv_ratio = (MUINT32)pNvram->me_large_mv_ratio;
                        param.Proc1_me_large_mv_txtr_en = (MBOOL)pNvram->me_large_mv_txtr_en;
                        param.Proc1_me_large_mv_txtr_wei = (MUINT32)pNvram->me_large_mv_txtr_wei;
                        param.Proc1_me_large_mv_txtr_thd = (MUINT32)pNvram->me_large_mv_txtr_thd;
                        param.Proc1_me_blink_eye_en = (MBOOL)pNvram->Blink_eye_en;
                    }
                }
                param.iBssOrgScore_base = (MUINT32)bssOut.final_score[bstIdx];
                param.iBssOrgScore_ref = (MUINT32)bssOut.final_score[refIdx];
                fdReader->queryUnlock(fdData);
                bssReader->queryUnlock(bssData);
            } else {
                mfllLogE("%s: BSS info(%d) is not found", __FUNCTION__, m_dump_uniqueKey);
            }
        }
        param.Proc1_me_refine_en = m_pCore->getMiddlewareInfo().postrefine_mfb;
        param.Proc1_me_refine_mv_ratio = (MUINT32)pNvram->post_me_refine_mv_ratio;
        param.Proc1_me_refine_face_ratio_ThL = (MUINT32)pNvram->post_me_refine_face_ratio_ThL;
        param.Proc1_me_refine_face_ratio_ThH = (MUINT32)pNvram->post_me_refine_face_ratio_ThH;
        param.Proc1_me_refine_full_ratio_ThL = (MUINT32)pNvram->post_me_refine_full_ratio_ThL;
        param.Proc1_me_refine_full_ratio_ThH = (MUINT32)pNvram->post_me_refine_full_ratio_ThH;
        param.Proc1_me_refine_edge_ccl_Th0 = (MUINT32)pNvram->post_me_refine_edge_ccl_Th[0];
        param.Proc1_me_refine_edge_ccl_Th1 = (MUINT32)pNvram->post_me_refine_edge_ccl_Th[1];
        param.Proc1_me_refine_edge_cclnum_Th = (MUINT32)pNvram->post_me_refine_edge_cclnum_Th;
        param.Proc1_me_refine_edge_FDAreaThL = (MUINT32)pNvram->post_me_refine_edge_FDAreaThL;
        param.Proc1_me_refine_edge_FDAreaThH = (MUINT32)pNvram->post_me_refine_edge_FDAreaThH;
        param.Proc1_me_lcl_deconf_en = (MUINT32)pNvram->lcl_deconf_en;
        param.Proc1_me_lcl_deconf_noise_lv = (MUINT32)pNvram->lcl_deconf_noise_lv;
        param.Proc1_me_lcl_deconf_bg_bss_ratio = (MUINT32)pNvram->lcl_deconf_bg_bss_ratio;
        param.Proc1_me_lcl_deconf_fd_bss_ratio = (MUINT32)pNvram->lcl_deconf_fd_bss_ratio;
        param.Proc1_me_lcl_deconf_dltvar_en = (MUINT32)pNvram->lcl_deconf_dltvar_en;

        param.Proc1_ImgFmt   = PROC1_FMT_Y;
        param.Noise_lvl      = static_cast<MINT8>(pNvram->conf_noise_lv);
        param.Proc_idx       = (MUINT8)m_dump_framNum;


#if (MFLL_MF_TAG_VERSION > 0)
    /* update debug info from NVRAM */
#   if (MFLL_MF_TAG_VERSION >= 11)
        m_pCore->updateExifInfo((unsigned int)MF_TAG_CONF_NOISE_LV             , (uint32_t)pNvram->conf_noise_lv                );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_DSDN_DIVIDEND             , (uint32_t)m_pCore->getDownscaleDividend()      );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_DSDN_DIVISOR              , (uint32_t)m_pCore->getDownscaleDivisor()       );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_DSUS_MODE            , (uint32_t)m_pCore->isMemcDownscale()           );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_AEVC_LCSO_EN              , (uint32_t)pNvram->aevc_lcso_en                 );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_RAFINE_EN            , (uint32_t)pNvram->post_refine_en               );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_EXT_SETTING               , (uint32_t)pNvram->ext_setting                  );
        m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_REFINE_INT           , (uint32_t)pNvram->post_refine_int              );
#   endif
#endif
    }

    mfllLogD3("%s: currISO=%d, Noise_lvl=%u, BldType=%d", __FUNCTION__,
            m_currIso, lvlvalue, m_bldType);

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
#   if (MFLL_MF_TAG_VERSION >= 11)
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_BAD_MV_RATE_TH           , (uint32_t)param.Proc1_bad_TH                    );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_EN             , (uint32_t)param.Proc1_me_refine_en              );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_MV_RATIO       , (uint32_t)param.Proc1_me_refine_mv_ratio        );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_FACE_RATIO_THL , (uint32_t)param.Proc1_me_refine_face_ratio_ThL  );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_FACE_RATIO_THH , (uint32_t)param.Proc1_me_refine_face_ratio_ThH  );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_FULL_RATIO_THL , (uint32_t)param.Proc1_me_refine_full_ratio_ThL  );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_FULL_RATIO_THH , (uint32_t)param.Proc1_me_refine_full_ratio_ThH  );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_EDGE_CCL_TH0   , (uint32_t)param.Proc1_me_refine_edge_ccl_Th0    );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_EDGE_CCL_TH1   , (uint32_t)param.Proc1_me_refine_edge_ccl_Th1    );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_EDGE_CCLNUM_TH , (uint32_t)param.Proc1_me_refine_edge_cclnum_Th  );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_EDGE_FDAREATHL , (uint32_t)param.Proc1_me_refine_edge_FDAreaThL  );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_POST_ME_REFINE_EDGE_FDAREATHH , (uint32_t)param.Proc1_me_refine_edge_FDAreaThH  );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_LCL_DECONF_EN                 , (uint32_t)param.Proc1_me_lcl_deconf_en          );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_LCL_DECONF_NOISE_LV           , (uint32_t)param.Proc1_me_lcl_deconf_noise_lv    );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_LCL_DECONF_BG_BSS_RATIO       , (uint32_t)param.Proc1_me_lcl_deconf_bg_bss_ratio);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_LCL_DECONF_FD_BSS_RATIO       , (uint32_t)param.Proc1_me_lcl_deconf_fd_bss_ratio);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_LCL_DECONF_DLTVER_EN          , (uint32_t)param.Proc1_me_lcl_deconf_dltvar_en   );
    // replace Noise_lvl by Proc1_noise_level(pNvram->me_noise_lv)
    m_pCore->updateExifInfo((unsigned int)MF_TAG_MEMC_NOISE_LVL                , (uint32_t)param.Proc1_noise_level               );
    // me blink eye

    m_pCore->updateExifInfo((unsigned int)MF_TAG_BLINK_EYE_EN                  , (uint32_t)param.Proc1_me_blink_eye_en          );
    //m_pCore->updateExifInfo((unsigned int)MF_TAG_BLINK_EYE_THL                 , (uint32_t)param.Proc1_me_blink_eye_ThL          );
    //m_pCore->updateExifInfo((unsigned int)MF_TAG_BLINK_EYE_THH                 , (uint32_t)param.Proc1_me_blink_eye_ThH          );
    //m_pCore->updateExifInfo((unsigned int)MF_TAG_BLINK_EYE_QSTEP               , (uint32_t)param.Proc1_me_blink_eye_qstep        );

    m_pCore->updateExifInfo((unsigned int)MF_TAG_ME_LARGE_MV_THD               , (uint32_t)param.Proc1_me_large_mv_thd           );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_ME_LARGE_MV_SAD_THD           , (uint32_t)param.Proc1_me_large_mv_SAD_thd       );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_ME_LARGE_MV_RATIO             , (uint32_t)param.Proc1_me_large_mv_ratio         );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_ME_LARGE_MV_TXTR_EN           , (uint32_t)param.Proc1_me_large_mv_txtr_en       );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_ME_LARGE_MV_TXTR_WEI          , (uint32_t)param.Proc1_me_large_mv_txtr_wei      );
    m_pCore->updateExifInfo((unsigned int)MF_TAG_ME_LARGE_MV_TXTR_THD          , (uint32_t)param.Proc1_me_large_mv_txtr_thd      );
#   endif
#endif

    /* print out information */
    mfllLogD3("%s:---------------------------", __FUNCTION__);
    mfllLogD3("%s: workbuf_addr=%p", __FUNCTION__, param.workbuf_addr);
    mfllLogD3("%s: buf_size=%d", __FUNCTION__, param.buf_size);
    mfllLogD3("%s: Proc1_base=%p", __FUNCTION__, param.Proc1_base);
    mfllLogD3("%s: Proc1_ref=%p", __FUNCTION__, param.Proc1_ref);
    mfllLogD3("%s: mc size %dx%d", __FUNCTION__, param.Proc1_width, param.Proc1_height);
    mfllLogD3("%s: mc v1,v2=%d,%d", __FUNCTION__, param.Proc1_V1, param.Proc1_V2);
    mfllLogD3("%s: mc badRange=%d", __FUNCTION__, param.Proc1_bad_range);
    mfllLogD3("%s: mc badTh=%d", __FUNCTION__, param.Proc1_bad_TH);
    mfllLogD3("%s: mc noise lvl=%d", __FUNCTION__, param.Noise_lvl);

    mfllLogD3("%s: Bst_FDROI X0=%d, Y0=%d, X1=%d, Y1=%d",
              __FUNCTION__,
              param.Proc1_Bst_FDROI_X0,
              param.Proc1_Bst_FDROI_Y0,
              param.Proc1_Bst_FDROI_X1,
              param.Proc1_Bst_FDROI_Y1);
    mfllLogD3("%s: Ref_FDROI X0=%d, Y0=%d, X1=%d, Y1=%d",
              __FUNCTION__,
              param.Proc1_Ref_FDROI_X0,
              param.Proc1_Ref_FDROI_Y0,
              param.Proc1_Ref_FDROI_X1,
              param.Proc1_Ref_FDROI_Y1);
    mfllLogD3("%s: Bst_LEYE X0=%d, X1=%d, Y0=%d, Y1=%d, UX=%d, UY=%d, XD=%d, DY=%d",
              __FUNCTION__,
              param.Proc1_Bst_LEYE_X0,
              param.Proc1_Bst_LEYE_X1,
              param.Proc1_Bst_LEYE_Y0,
              param.Proc1_Bst_LEYE_Y1,
              param.Proc1_Bst_LEYE_UX,
              param.Proc1_Bst_LEYE_UY,
              param.Proc1_Bst_LEYE_DX,
              param.Proc1_Bst_LEYE_DY);
    mfllLogD3("%s: Bst_REYE X0=%d, X1=%d, Y0=%d, Y1=%d, UX=%d, UY=%d, XD=%d, DY=%d",
              __FUNCTION__,
              param.Proc1_Bst_REYE_X0,
              param.Proc1_Bst_REYE_X1,
              param.Proc1_Bst_REYE_Y0,
              param.Proc1_Bst_REYE_Y1,
              param.Proc1_Bst_REYE_UX,
              param.Proc1_Bst_REYE_UY,
              param.Proc1_Bst_REYE_DX,
              param.Proc1_Bst_REYE_DY);
    mfllLogD3("%s: Ref_LEYE X0=%d, X1=%d, Y0=%d, Y1=%d, UX=%d, UY=%d, XD=%d, DY=%d",
              __FUNCTION__,
              param.Proc1_Ref_LEYE_X0,
              param.Proc1_Ref_LEYE_X1,
              param.Proc1_Ref_LEYE_Y0,
              param.Proc1_Ref_LEYE_Y1,
              param.Proc1_Ref_LEYE_UX,
              param.Proc1_Ref_LEYE_UY,
              param.Proc1_Ref_LEYE_DX,
              param.Proc1_Ref_LEYE_DY);
    mfllLogD3("%s: Ref_REYE X0=%d, X1=%d, Y0=%d, Y1=%d, UX=%d, UY=%d, XD=%d, DY=%d",
              __FUNCTION__,
              param.Proc1_Ref_REYE_X0,
              param.Proc1_Ref_REYE_X1,
              param.Proc1_Ref_REYE_Y0,
              param.Proc1_Ref_REYE_Y1,
              param.Proc1_Ref_REYE_UX,
              param.Proc1_Ref_REYE_UY,
              param.Proc1_Ref_REYE_DX,
              param.Proc1_Ref_REYE_DY);

    mfllLogD3("%s: me_refine_en=%d", __FUNCTION__, param.Proc1_me_refine_en);
    mfllLogD3("%s: me_refine_mv_ratio=%d", __FUNCTION__, param.Proc1_me_refine_mv_ratio);
    mfllLogD3("%s: me_refine_face_ratio_ThL=%d, me_refine_face_ratio_ThH=%d",
              __FUNCTION__,
              param.Proc1_me_refine_face_ratio_ThL,
              param.Proc1_me_refine_face_ratio_ThH);

    mfllLogD3("%s: me_refine_full_ratio_ThL=%d, me_refine_full_ratio_ThH=%d",
              __FUNCTION__,
              param.Proc1_me_refine_full_ratio_ThL,
              param.Proc1_me_refine_full_ratio_ThH);

    mfllLogD3("%s: me_refine_edge_ccl_Th0=%d, me_refine_edge_ccl_Th1=%d, me_refine_edge_cclnum_Th=%d",
              __FUNCTION__,
              param.Proc1_me_refine_edge_ccl_Th0,
              param.Proc1_me_refine_edge_ccl_Th1,
              param.Proc1_me_refine_edge_cclnum_Th);

    mfllLogD3("%s: me_refine_edge_FDAreaThL=%d me_refine_edge_FDAreaThH=%d",
              __FUNCTION__,
              param.Proc1_me_refine_edge_FDAreaThL,
              param.Proc1_me_refine_edge_FDAreaThH);

    mfllLogD3("%s: me_lcl_deconf_en=%d, me_lcl_deconf_noise_lv=%d",
              __FUNCTION__,
              param.Proc1_me_lcl_deconf_en,
              param.Proc1_me_lcl_deconf_noise_lv);

    mfllLogD3("%s: me_lcl_deconf_bg_bss_ratio=%d, me_lcl_deconf_fd_bss_ratio=%d",
              __FUNCTION__,
              param.Proc1_me_lcl_deconf_bg_bss_ratio,
              param.Proc1_me_lcl_deconf_fd_bss_ratio);

    mfllLogD3("%s: me_lcl_deconf_dltvar_en=%d", __FUNCTION__, param.Proc1_me_lcl_deconf_dltvar_en);

    mfllLogD3("%s: Proc1_me_large_mv_thd=%d, Proc1_me_large_mv_SAD_thd=%d, Proc1_me_large_mv_ratio=%d", __FUNCTION__,
        param.Proc1_me_large_mv_thd,
        param.Proc1_me_large_mv_SAD_thd,
        param.Proc1_me_large_mv_ratio);

    mfllLogD3("%s: Proc1_me_large_mv_txtr_en=%d, Proc1_me_large_mv_txtr_wei=%d, Proc1_me_large_mv_txtr_thd=%d", __FUNCTION__,
        param.Proc1_me_large_mv_txtr_en,
        param.Proc1_me_large_mv_txtr_wei,
        param.Proc1_me_large_mv_txtr_thd);

    mfllLogD3("%s: Proc1_me_blink_eye_en=%d", __FUNCTION__, param.Proc1_me_blink_eye_en);

    mfllLogD3("%s: ImgFmt=%d", __FUNCTION__, param.Proc1_ImgFmt);
    mfllLogD3("%s: iBssOrgScore_base=%d", __FUNCTION__, param.iBssOrgScore_base);
    mfllLogD3("%s: iBssOrgScore_ref=%d", __FUNCTION__, param.iBssOrgScore_ref);
    mfllLogD3("%s: Proc_idx=%d", __FUNCTION__, param.Proc_idx);
    mfllLogD3("%s:---------------------------", __FUNCTION__);

    //dump binary
    auto dump2Binary = [this](MINT32 uniqueKey, MINT32 requestNum, MINT32 frameNum, const char* buf, size_t size, string fn) -> bool {
        if (mfll::MfllProperty::readProperty(mfll::Property_DumpYuv) != 1 && mfll::MfllProperty::readProperty(mfll::Property_DumpSim) != 1)
            return false;

        char filepath[256] = {0};
        snprintf(filepath, sizeof(filepath)-1, "%s/%09d-%04d-%04d-""%s", MFLLMEMC_DUMP_PATH, uniqueKey, requestNum, frameNum, fn.c_str());
        std::ofstream ofs (filepath, std::ofstream::binary);
        if (!ofs.is_open()) {
            mfllLogW("dump2Binary: open file(%s) fail", filepath);
            return false;
        }
        ofs.write(buf, size);
        ofs.close();
        return true;
    };

    dump2Binary(m_dump_uniqueKey, m_dump_requestNum, m_dump_framNum, (const char*)(&param), sizeof(struct MFBLL_SET_PROC_INFO_STRUCT), MFLLMEMC_DUMP_PARAM_FILENAME);
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

    if (CC_LIKELY(m_imgMotionCompensationMv.get())) {
        paramOut.pu1MV = static_cast<MUINT8*>(m_imgMotionCompensationMv->getVa());
        paramOut.u4MVSize = static_cast<MUINT32>(m_imgMotionCompensationMv->getRealBufferSize());
    }
    else {
        mfllLogW("MotionCompensationMv is NULL, create one here.");
        auto _img = IMfllImageBuffer::createInstance();
        if (CC_UNLIKELY(_img == nullptr)) {
            mfllLogE("create IMfllImageBuffer instance failed");
            m_isIgnoredMc = 1;
            return MfllErr_UnexpectedError;
        }
        static const size_t _blockSize = 16;
#define __ALIGN(w, a) (((w + (a-1)) / a) * a)
        _img->setResolution(
                __ALIGN(m_widthMe,  (_blockSize)) / (_blockSize >> 1),
                __ALIGN(m_heightMe, (_blockSize)) / (_blockSize >> 1)
                );
#undef __ALIGN
        _img->setImageFormat(ImageFormat_Sta32);
        _img->setAligned(1, 1);
        if (CC_UNLIKELY(_img->initBuffer() != MfllErr_Ok)) {
            mfllLogE("init MotionCompensationMv image buffer failed");
            m_isIgnoredMc = 1;
            return MfllErr_UnexpectedError;
        }
        paramOut.pu1MV= static_cast<MUINT8*>(_img->getVa());
        paramOut.u4MVSize = static_cast<MUINT32>(_img->getRealBufferSize());
    }

    mfllLogD3("%s: MotionCompensationMv: (addr, size)=(%p, %u)", __FUNCTION__,
            paramOut.pu1MV, paramOut.u4MVSize);


    ret = m_mfbll->MfbllMain(MFBLL_PROC1, NULL, &paramOut);
    dump2Binary(m_dump_uniqueKey, m_dump_requestNum, m_dump_framNum, (const char*)(&paramOut), sizeof(MFBLL_PROC1_OUT_STRUCT), MFLLMEMC_DUMP_OUT_FILENAME);
    if (CC_UNLIKELY(S_MFBLL_OK != ret)) {
        mfllLogE("%s: do ME failed", __FUNCTION__);
        m_isIgnoredMc = 1;
        return MfllErr_UnexpectedError;
    }

    mfllLogD3("%s: is skip MC --> %d, u4FrmLevel --> %d", __FUNCTION__, paramOut.bSkip_proc, paramOut.u4FrmLevel);
    m_isIgnoredMc = paramOut.bSkip_proc;
    m_frameLevelConfidence = paramOut.u4FrmLevel;

#if (MFLL_MF_TAG_VERSION > 0)
    /* update debug info from output */
#   if (MFLL_MF_TAG_VERSION >= 11)
    m_pCore->updateExifInfo((unsigned int)(MF_TAG_DBG_ME_FRMLEVEL0 + m_dump_framNum), (uint32_t)m_frameLevelConfidence);
#   endif
#endif

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

#if 1
    return MfllErr_NotSupported;
#else
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
        .i4OuWidth      = (MUINT32)m_imgDstMc->getWidth(), // the image width
        .i4OuHeight     = (MUINT32)m_imgDstMc->getHeight() // the image height
    };

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
#endif
}

bool MfllMemc::getForceMe(void* param_addr, size_t param_size)
{
    if ( param_size != sizeof(MFBLL_SET_PROC_INFO_STRUCT)) {
        mfllLogE("%s: invalid sizeof param, param_size:%d, sizeof(MFBLL_SET_PROC_INFO_STRUCT):%d",
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
