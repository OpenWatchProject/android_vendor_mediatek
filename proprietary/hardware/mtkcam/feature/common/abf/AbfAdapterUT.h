/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __ABF_ADAPTER_UT_H__
#define __ABF_ADAPTER_UT_H__

#include <unordered_map>
#include <mtkcam/feature/abf/AbfAdapter.h>
#include <mtkcam/def/BuiltinTypes.h>
// Some header does something bad in MTKABF.h, we should always put MTKABF.h
// at the last to avoid C11 atomic(strange!) build errors
#include <MTKABF.h>


class MTKABFMock : public MTKABF
{
public:
    static MTKABF* createInstance(DrvABFObject_e eobject = DRV_ABF_OBJ_IMAGE);
    virtual void   destroyInstance(MTKABF* obj);

    MTKABFMock();
    virtual ~MTKABFMock();

    // Process Control
    virtual MRESULT ABFInit(void *InitInData);  // Env/Cb setting
    virtual MRESULT ABFMain(void);              // START
    virtual MRESULT ABFReset(void);             // RESET

    // Feature Control
    virtual MRESULT ABFFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

    // Template method to be overwritten by UT cases
    virtual MRESULT doABFFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

protected:
    bool isInited;
    bool isMainCalled;
    bool isReset;
    ABFResultInfo mResult;

    // FeatureID -> pParaIn
    std::unordered_map<MUINT32,void*> mCtrlHist;

    void addCtrlHist(MUINT32 FeatureID, void *pParaIn) {
        mCtrlHist[FeatureID] = pParaIn;
    }

    bool isInCtrlHist(MUINT32 FeatureID) {
        return mCtrlHist.count(FeatureID) > 0;
    }
};


class AbfAdapterUT : public AbfAdapter
{
public:
    AbfAdapterUT(
            MUINT32 openId = 0,
            unsigned int debugFlags = DEBUG_INTERMEDIATE) :
            AbfAdapter(openId, debugFlags),
            mTuningInfo(NULL), mOwnTuningInfo(false)
    {
    }

    virtual ~AbfAdapterUT();

    // Note the ownershio of mock will be transfereed
    void setMock(MTKABF *mock = NULL);

    void setTuningInfo(ABFTuningInfo *tuningInfo, bool transferOwnership) {
        mTuningInfo = tuningInfo;
        mOwnTuningInfo = transferOwnership;
    }

protected:
    ABFTuningInfo *mTuningInfo;
    bool mOwnTuningInfo;

    virtual void* getTuningData(const ProcessParam &param);
};

#endif

