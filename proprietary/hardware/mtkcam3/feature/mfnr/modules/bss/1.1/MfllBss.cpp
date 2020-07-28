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

// ALGORITHM
#include <MTKBss.h>

#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <custom/debug_exif/dbg_exif_param.h>
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#endif

using std::vector;
using namespace mfll;

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
        vector<int64_t>                         &/*timestamps*/
        )
{
    mfllAutoLogFunc();
    mfllAutoTraceFunc();

    MTKBss *mtkBss = NULL;
    int width = 0;
    int height = 0;
    size_t size = 0;
    int roiPercentage = m_roiPercetange;

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
    size = imgs.size();

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
        mfllLogD("set algo threads priority to %d", _priority);
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

    /* set feature control */
    {
        /* calcuate ROI cropping width */
        int w = (width * roiPercentage + 5) / 100;
        int h = (height * roiPercentage + 5) / 100;
        int x = (width - w) / 2;
        int y = (height - h) / 2;

        BSS_PARAM_STRUCT p;
        p.BSS_ON = 1;
        p.BSS_ROI_WIDTH = w;
        p.BSS_ROI_HEIGHT = h;
        p.BSS_ROI_X0 = x;
        p.BSS_ROI_Y0 = y;
        p.BSS_SCALE_FACTOR = CUST_MFLL_BSS_SCALE_FACTOR;
        p.BSS_CLIP_TH0 = CUST_MFLL_BSS_CLIP_TH0;
        p.BSS_CLIP_TH1 = CUST_MFLL_BSS_CLIP_TH1;
        p.BSS_ZERO = CUST_MFLL_BSS_ZERO;
        p.BSS_FRAME_NUM = (MUINT32)imgs.size();
        p.BSS_ADF_TH = CUST_MFLL_BSS_ADF_TH;
        p.BSS_SDF_TH = CUST_MFLL_BSS_SDF_TH;

        if (CC_UNLIKELY(getForceBss(reinterpret_cast<void*>(&p), sizeof(BSS_PARAM_STRUCT)))) {
            mfllLogI("%s: force set BSS param as manual setting", __FUNCTION__);
        }
        /* print out information */
        mfllLogD("%s: BSS_ON = %d", __FUNCTION__, p.BSS_ON);
        mfllLogD("%s: BSS_ROI(x,y,w,h) =(%d,%d,%d,%d)", __FUNCTION__,
                      p.BSS_ROI_X0, p.BSS_ROI_Y0, p.BSS_ROI_WIDTH, p.BSS_ROI_HEIGHT);
        mfllLogD("%s: BSS_SCALE_FACTOR = %d", __FUNCTION__, p.BSS_SCALE_FACTOR);
        mfllLogD("%s: BSS_CLIP_TH0     = %d", __FUNCTION__, p.BSS_CLIP_TH0);
        mfllLogD("%s: BSS_CLIP_TH1     = %d", __FUNCTION__, p.BSS_CLIP_TH1);
        mfllLogD("%s: BSS_ZERO         = %d", __FUNCTION__, p.BSS_ZERO);
        mfllLogD("%s: BSS_FRAME_NUM    = %d", __FUNCTION__, p.BSS_FRAME_NUM);
        mfllLogD("%s: BSS_ADF_TH       = %d", __FUNCTION__, p.BSS_ADF_TH);
        mfllLogD("%s: BSS_SDF_TH       = %d", __FUNCTION__, p.BSS_SDF_TH);


        /* update debug info */
#if (MFLL_MF_TAG_VERSION > 0)
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_WIDTH, (uint32_t)p.BSS_ROI_WIDTH);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_HEIGHT, (uint32_t)p.BSS_ROI_HEIGHT);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_SCALE_FACTOR, (uint32_t)p.BSS_SCALE_FACTOR);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_CLIP_TH0, (uint32_t)p.BSS_CLIP_TH0);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_CLIP_TH1, (uint32_t)p.BSS_CLIP_TH1);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ZERO, (uint32_t)p.BSS_ZERO);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_X0, (uint32_t)p.BSS_ROI_X0);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_Y0, (uint32_t)p.BSS_ROI_Y0);
#   if (MFLL_MF_TAG_VERSION >= 3)
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ADF_TH, (uint32_t)p.BSS_ADF_TH);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_SDF_TH, (uint32_t)p.BSS_SDF_TH);
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
        BSS_INPUT_DATA inParam;

        inParam.inWidth = width;
        inParam.inHeight = height;
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

        auto b = mtkBss->BssMain(BSS_PROC1, &inParam, &outParam);
        if (b != S_BSS_OK) {
            mfllLogE("%s: MTKBss::Main returns failed (%d)", __FUNCTION__, (int)b);
            goto lbExit;
        }

        for (size_t i = 0; i < size; i++) {
            newIndex.push_back(outParam.originalOrder[i]);
            mvs[i].x = outParam.gmv[i].x;
            mvs[i].y = outParam.gmv[i].y;
        }

        // skip frame count
        m_skipFrmCnt = outParam.i4SkipFrmCnt;
    }

#if (MFLL_MF_TAG_VERSION > 0)
    m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_BEST_IDX, (uint32_t)newIndex[0]);
#endif

#if(MFLL_MF_TAG_VERSION==7)
{
        /* update BssOrderMapping debug info */
        {
            std::deque<IMetadata*> m_vMetaHal = m_pCore->getMetaHal();
            unsigned int dbgIdxBssOrderMapping = static_cast<unsigned int>(MF_TAG_BSS_REORDER_MAPPING_00);
            int frameNum =0;
            Vector<int> vNewIndexMap;
            vNewIndexMap.resize(newIndex.size());
            for (size_t i = 0; i < vNewIndexMap.size(); i++)
            {vNewIndexMap.editItemAt(newIndex[i]) = i;}

            for (size_t i = 0; i < m_vMetaHal.size(); i++){
            int iBssOrderMapping = vNewIndexMap[i];
            IMetadata::getEntry(m_vMetaHal[i], MTK_PIPELINE_FRAME_NUMBER, frameNum);
            iBssOrderMapping += (frameNum%10000)*100;
            mfllLogD("%s: framenum = %d,BssOrderMapping[%zu] = %d", __FUNCTION__,frameNum, i, iBssOrderMapping);
            m_pCore->updateExifInfo(dbgIdxBssOrderMapping, iBssOrderMapping);
            dbgIdxBssOrderMapping++;
            }
        }

        /* encoding for bss order */
        {
            /** MF_TAG_BSS_ORDER_IDX
            *
            *  BSS order for top 8 frames (MSB -> LSB)
            *
            *  |     4       |     4       |     4       |     4       |     4       |     4       |     4       |     4       |
            *  | bssOrder[0] | bssOrder[1] | bssOrder[2] | bssOrder[3] | bssOrder[4] | bssOrder[5] | bssOrder[6] | bssOrder[7] |
            */
            unsigned int bssOrder = 0;
            size_t i = 0;

            for ( ; i < newIndex.size() && i < 8 ; i++)
            {bssOrder = (bssOrder << 4) | ((unsigned int)newIndex[i]<0xf?(unsigned int)newIndex[i]:0xf);}
            for ( ; i < 8 ; i++)
            {bssOrder = (bssOrder << 4) | 0xf;}

            m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ORDER_IDX, bssOrder);
        }
}
#endif

lbExit:
    if (mtkBss)
        mtkBss->destroyInstance();
    return newIndex;
}

bool MfllBss::getForceBss(void* param_addr, size_t param_size)
{
    if ( param_size != sizeof(BSS_PARAM_STRUCT)) {
        mfllLogE("%s: invalid sizeof param, param_size:%zu, sizeof(BSS_PARAM_STRUCT):%zu",
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
