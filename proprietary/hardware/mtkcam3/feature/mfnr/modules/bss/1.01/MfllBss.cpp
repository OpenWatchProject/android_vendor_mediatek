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
#include <mtkcam3/feature/mfnr/MfllLog.h>

// ALGORITHM
//#include <MTKBss.h>

#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <custom/debug_exif/dbg_exif_param.h>
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#endif


using std::vector;
using namespace mfll;
using namespace NSCamShot;

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

    int mYuvQ_Width = 0;
    int mYuvQ_Height = 0;
    size_t size = 0;
    float  mRawToQyuvFactor = 0.5;

    vector<int> newIndex;
    if (imgs.size() != mvs.size()) {
        mfllLogE("%s: the amount of image(%d) and GMV(%d) is different",
                __FUNCTION__, imgs.size(), mvs.size());
        goto lbExit;
    }

    // check if input is nullptr
    for (const auto& itr : imgs) {
        if (CC_UNLIKELY( itr.get() == nullptr )) {
            mfllLogE("%s: the input images maybe NULL", __FUNCTION__);
            goto lbExit;
        }
    }

    /* main process */
    {

    mYuvQ_Width = imgs[0]->getAlignedWidth();
    mYuvQ_Height = imgs[0]->getAlignedHeight();
    size = imgs.size();

    MUINT32 roiRatio = 95;
    MUINT32 roiW = mYuvQ_Width  * roiRatio / 100;
    MUINT32 roiH = mYuvQ_Height * roiRatio / 100;
    MINT32 roiX = mYuvQ_Width * (100-roiRatio) / 100 / 2;
    MINT32 roiY = mYuvQ_Height * (100-roiRatio) / 100 / 2;

    MF_BSS_PARAM_STRUCT param = {
        MF_BSS_ON                    : 1,
        MF_BSS_ROI_WIDTH             : roiW,    //1520
        MF_BSS_ROI_HEIGHT            : roiH,    //900
        MF_BSS_ROI_X0                : roiX,    //40
        MF_BSS_ROI_Y0                : roiY,    //150
        MF_BSS_SCALE_FACTOR          : CUST_MFLL_BSS_SCALE_FACTOR,
        MF_BSS_CLIP_TH0              : CUST_MFLL_BSS_CLIP_TH0,
        MF_BSS_CLIP_TH1              : CUST_MFLL_BSS_CLIP_TH1,
        MF_BSS_ZERO                  : CUST_MFLL_BSS_ZERO,
    };

    /* print out information */
        mfllLogD("%s: MF_BSS_ROI(x,y,w,h) =(%d,%d,%d,%d)", __FUNCTION__, roiX, roiY, roiW, roiH);
        mfllLogD("%s: MF_BSS_SCALE_FACTOR = %d", __FUNCTION__, param.MF_BSS_SCALE_FACTOR);
        mfllLogD("%s: MF_BSS_CLIP_TH0     = %d", __FUNCTION__, param.MF_BSS_CLIP_TH0);
        mfllLogD("%s: MF_BSS_CLIP_TH1     = %d", __FUNCTION__, param.MF_BSS_CLIP_TH1);
        mfllLogD("%s: MF_BSS_ZERO         = %d", __FUNCTION__, param.MF_BSS_ZERO);

    /* update debug info */
#if (MFLL_MF_TAG_VERSION > 0)
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_WIDTH, (uint32_t)param.MF_BSS_ROI_WIDTH);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_ROI_HEIGHT, (uint32_t)param.MF_BSS_ROI_HEIGHT);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_SCALE_FACTOR, (uint32_t)param.MF_BSS_SCALE_FACTOR);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_CLIP_TH0, (uint32_t)param.MF_BSS_CLIP_TH0);
        m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_CLIP_TH1, (uint32_t)param.MF_BSS_CLIP_TH1);
#endif

        BSS_INPUT_DATA input;
        BSS_OUTPUT_DATA output;
               input.inMEWidth = mYuvQ_Width;
               input.inMEHeight = mYuvQ_Height;
               input.Proc1QBImg = (MUINT8*)imgs[0]->getVa();
               input.Proc1QR1Img = (MUINT8*)imgs[1]->getVa();
               input.Proc1QR2Img = (MUINT8*)imgs[2]->getVa();
               input.Proc1QR3Img = (MUINT8*)imgs[3]->getVa();

        for(size_t i=0; i<size; i++) {  
                   input.gmv[i].x =  mvs[i].x * mRawToQyuvFactor;
                   input.gmv[i].y =  mvs[i].y * mRawToQyuvFactor;
        }

        BestShotSelection *bss = BestShotSelection::createInstance();
        bss->init(param);
        bss->sort(&input, &output);
        bss->destroyInstance();

        for (size_t i = 0; i < size; i++) {
            newIndex.push_back(output.originalOrder[i]);
            mvs[i].x = output.gmv[i].x/mRawToQyuvFactor;
            mvs[i].y = output.gmv[i].y/mRawToQyuvFactor;
        }
   }

#if (MFLL_MF_TAG_VERSION > 0)
    m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_INPUT0, (uint32_t)newIndex[0]);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_INPUT1, (uint32_t)newIndex[1]);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_INPUT2, (uint32_t)newIndex[2]);
    m_pCore->updateExifInfo((unsigned int)MF_TAG_BSS_INPUT3, (uint32_t)newIndex[3]);
#endif

#if(MFLL_MF_TAG_VERSION==1)
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

    return newIndex;
}
