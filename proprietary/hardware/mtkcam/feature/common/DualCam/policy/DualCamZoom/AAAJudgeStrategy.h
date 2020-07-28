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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
/******************************************************************************
*
*******************************************************************************/
#ifndef _MTKCAM_DUALZOOM_AAA_JUDGE_STRATEGY_H_
#define _MTKCAM_DUALZOOM_AAA_JUDGE_STRATEGY_H_

// Standard C header file

// Android system/core header file

// mtkcam custom header file
#include <camera_custom_dualzoom.h>
// mtkcam global header file

// Module header file

// Local header file
#include "DualZoomPolicy_Common.h"

/******************************************************************************
* namespace start
*******************************************************************************/
namespace NSCam
{
/******************************************************************************
 * AAAJudgeStrategy
******************************************************************************/
class AAAJudgeStrategy : public RefBase
{
public:
    OpenId getOpenId() const;

    const std::string& getName() const;

    MINT32 getAELv() const;

    MINT32 getAFDac() const;

    MBOOL  getIsAFDone() const;

public:
    OpenId calculate(sp<IFrame3AControl>& frame3AControl);

public:
    virtual ~AAAJudgeStrategy();

protected:
    AAAJudgeStrategy(OpenId openId, const std::string& name);

    virtual OpenId onCalculate(const Frame3ASetting_t& settings);

protected:
    MVOID setIsAFDone(MBOOL value);

private:
    const OpenId        mOpenId;
    const std::string   mName;

    MINT32              mAELv;
    MINT32              mAFDac;
    MBOOL               mIsAFDone;
};
//
/******************************************************************************
 * DefauleAAAJudgeStrategy
******************************************************************************/
class DefauleAAAJudgeStrategy : public AAAJudgeStrategy
{
public:
    static sp<AAAJudgeStrategy> createInstance(OpenId openId);

private:
    DefauleAAAJudgeStrategy(OpenId openId);
};
//
/******************************************************************************
 * WideAAAJudgeStrategy
******************************************************************************/
class WideAAAJudgeStrategy : public AAAJudgeStrategy
{
public:
    static sp<AAAJudgeStrategy> createInstance();

protected:
    OpenId onCalculate(const Frame3ASetting_t& settings) override;

private:
    WideAAAJudgeStrategy();

private:
    MINT32 mAEIsoCount;
    MINT32 mAFDacCount;
    MINT32 mPreviousAFDac;
};
//
/******************************************************************************
 * TeleAAAJudgeStrategy
******************************************************************************/
class TeleAAAJudgeStrategy : public AAAJudgeStrategy
{
public:
    static sp<AAAJudgeStrategy> createInstance();

protected:
    OpenId onCalculate(const Frame3ASetting_t& settings) override;

private:
    TeleAAAJudgeStrategy();

private:
    MBOOL   mIsPreviousAFDone;
    MINT32  mAEIsoCount;
};
//
} // NSCam
#endif // _MTKCAM_DUALZOOM_AAA_JUDGE_STRATEGY_H_