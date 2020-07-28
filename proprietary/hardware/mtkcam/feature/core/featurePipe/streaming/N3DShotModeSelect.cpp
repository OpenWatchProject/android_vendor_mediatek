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

#include <stdlib.h>

#include "DebugControl.h"
#include "N3DShotModeSelect.h"

#define PIPE_CLASS_TAG "N3DShotModelSelect"
#define PIPE_TRACE TRACE_N3D_NODE
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {


SelectShotMode::
~SelectShotMode()
{}

void
SelectShotMode::
setDecisionCriteria( int iso_speed )
{
    if( iso_speed >= mCriteriaSettings.CUSTOMER_ISO_HIG )
    {
        mDecisionParam.var_low  = mCriteriaSettings.CUSTOMER_VAR_L_ISO_EXT ;
        mDecisionParam.var_high = mCriteriaSettings.CUSTOMER_VAR_H_ISO_EXT ;
        mDecisionParam.int_low  = mCriteriaSettings.CUSTOMER_INT_L_ISO_EXT ;
        mDecisionParam.int_high = mCriteriaSettings.CUSTOMER_INT_H_ISO_EXT ;
        mDecisionParam.thr_rect = mCriteriaSettings.CUSTOMER_RECT_ISO_EXT ;
        mDecisionParam.idx_var  = mCriteriaSettings.CUSTOMER_VAR_TOL_DEG_EXT + IDX_VAR_OFST ;
    }
    else if( iso_speed >= mCriteriaSettings.CUSTOMER_ISO_MID )
    {
        mDecisionParam.var_low  = mCriteriaSettings.CUSTOMER_VAR_L_ISO_HIG ;
        mDecisionParam.var_high = mCriteriaSettings.CUSTOMER_VAR_H_ISO_HIG ;
        mDecisionParam.int_low  = mCriteriaSettings.CUSTOMER_INT_L_ISO_HIG ;
        mDecisionParam.int_high = mCriteriaSettings.CUSTOMER_INT_H_ISO_HIG ;
        mDecisionParam.thr_rect = mCriteriaSettings.CUSTOMER_RECT_ISO_HIG ;
        mDecisionParam.idx_var  = mCriteriaSettings.CUSTOMER_VAR_TOL_DEG_HIG + IDX_VAR_OFST ;
    }
    else
    {
        mDecisionParam.var_low  = mCriteriaSettings.CUSTOMER_VAR_L_ISO_MID ;
        mDecisionParam.var_high = mCriteriaSettings.CUSTOMER_VAR_H_ISO_MID ;
        mDecisionParam.int_low  = mCriteriaSettings.CUSTOMER_INT_L_ISO_MID ;
        mDecisionParam.int_high = mCriteriaSettings.CUSTOMER_INT_H_ISO_MID ;
        mDecisionParam.thr_rect = mCriteriaSettings.CUSTOMER_RECT_ISO_MID ;
        mDecisionParam.idx_var  = mCriteriaSettings.CUSTOMER_VAR_TOL_DEG_MID + IDX_VAR_OFST ;
    }

    mDecisionParam_lb.var_low  = mDecisionParam.var_low  - mCriteriaSettings.CUSTOMER_DECISION_MARGIN_VAR ;
    mDecisionParam_lb.var_high = mDecisionParam.var_high - mCriteriaSettings.CUSTOMER_DECISION_MARGIN_VAR ;
    mDecisionParam_lb.int_low  = mDecisionParam.int_low  - mCriteriaSettings.CUSTOMER_DECISION_MARGIN_INT ;
    mDecisionParam_lb.int_high = mDecisionParam.int_high - mCriteriaSettings.CUSTOMER_DECISION_MARGIN_INT ;

    mDecisionParam_ub.var_low  = mDecisionParam.var_low  + mCriteriaSettings.CUSTOMER_DECISION_MARGIN_VAR ;
    mDecisionParam_ub.var_high = mDecisionParam.var_high + mCriteriaSettings.CUSTOMER_DECISION_MARGIN_VAR ;
    mDecisionParam_ub.int_low  = mDecisionParam.int_low  + mCriteriaSettings.CUSTOMER_DECISION_MARGIN_INT ;
    mDecisionParam_ub.int_high = mDecisionParam.int_high + mCriteriaSettings.CUSTOMER_DECISION_MARGIN_INT ;

    TRACE_FUNC("[%d,%d,%d,%d,%d,%d] [%d,%d,%d,%d,%d,%d] [%d,%d,%d] [%d,%d,%d] [%d,%d,%d,%d] [%d] [%d,%d] [%d,%d]",
        mCriteriaSettings.CUSTOMER_VAR_L_ISO_MID,
        mCriteriaSettings.CUSTOMER_VAR_H_ISO_MID,
        mCriteriaSettings.CUSTOMER_VAR_L_ISO_HIG,
        mCriteriaSettings.CUSTOMER_VAR_H_ISO_HIG,
        mCriteriaSettings.CUSTOMER_VAR_L_ISO_EXT,
        mCriteriaSettings.CUSTOMER_VAR_H_ISO_EXT,

        mCriteriaSettings.CUSTOMER_INT_L_ISO_MID,
        mCriteriaSettings.CUSTOMER_INT_H_ISO_MID,
        mCriteriaSettings.CUSTOMER_INT_L_ISO_HIG,
        mCriteriaSettings.CUSTOMER_INT_H_ISO_HIG,
        mCriteriaSettings.CUSTOMER_INT_L_ISO_EXT,
        mCriteriaSettings.CUSTOMER_INT_H_ISO_EXT,

        mCriteriaSettings.CUSTOMER_RECT_ISO_MID,
        mCriteriaSettings.CUSTOMER_RECT_ISO_HIG,
        mCriteriaSettings.CUSTOMER_RECT_ISO_EXT,

        mCriteriaSettings.CUSTOMER_VAR_TOL_DEG_MID,
        mCriteriaSettings.CUSTOMER_VAR_TOL_DEG_HIG,
        mCriteriaSettings.CUSTOMER_VAR_TOL_DEG_EXT,

        mCriteriaSettings.CUSTOMER_ISO_MID,
        mCriteriaSettings.CUSTOMER_ISO_HIG,
        mCriteriaSettings.CUSTOMER_ISO_HR_MIN,
        mCriteriaSettings.CUSTOMER_ISO_BM_MIN,

        mCriteriaSettings.CUSTOMER_FE_NUM_MIN,

        mCriteriaSettings.CUSTOMER_DECISION_SMOOTH_THR_AE,
        mCriteriaSettings.CUSTOMER_DECISION_SMOOTH_THR_AF,

        mCriteriaSettings.CUSTOMER_DECISION_MARGIN_VAR,
        mCriteriaSettings.CUSTOMER_DECISION_MARGIN_INT
    );

    TRACE_FUNC("ISO[%d] Para[%d,%d] [%d %d] [%d %d]",
        iso_speed,
        mDecisionParam.idx_var,
        mDecisionParam.thr_rect,
        mDecisionParam.var_low,
        mDecisionParam.var_high,
        mDecisionParam.int_low,
        mDecisionParam.int_high
    ) ;
}

bool
SelectShotMode::
getStableStatus(int* scene_info)
{
    if(mPrvSceneInfo[0] == -1)
    {
        MY_LOGW("mPrvSceneInfo/mPrvDecision [%d,%d] not set yet", mPrvSceneInfo[0], mPrvDenoiseType);
        return 0 ;
    }
    else
    {
        TRACE_FUNC("AE[%d, %d] AF[%d, %d]",
            mPrvSceneInfo[IDX_INT], scene_info[IDX_INT],
            mPrvSceneInfo[IDX_DAC], scene_info[IDX_DAC]
        );

        if( (abs(mPrvSceneInfo[IDX_INT]-scene_info[IDX_INT]) < mCriteriaSettings.CUSTOMER_DECISION_SMOOTH_THR_AE) &&
            (abs(mPrvSceneInfo[IDX_DAC]-scene_info[IDX_DAC]) < mCriteriaSettings.CUSTOMER_DECISION_SMOOTH_THR_AF)
        )
        {
            return 1 ;
        }
        else
        {
            return 0 ;
        }
    }
}

bool
SelectShotMode::
ruleJudgement( int var_in, int int_in, BMDeNoiseCriteria* rule )
{
    float cal;
    int var_low  = rule->var_low  ;
    int var_high = rule->var_high ;
    int int_low  = rule->int_low  ;
    int int_high = rule->int_high ;

    TRACE_FUNC("getRuleDecision [%d %d] [%d %d] [%d %d]",
        var_in,
        int_in,
        var_low,
        var_high,
        int_low,
        int_high
    ) ;

    if( (var_in <= var_low*100) || (int_in <= int_low) )
    {
        return 1 ;
    }
    else
    {
        cal = int_high + (float)(int_low-int_high)/(var_high-var_low)*(var_in/100.0f-var_low) ;

        TRACE_FUNC("getRuleDecision cal=[%.2f]", cal ) ;

        if( int_in <= cal )
        {
            return 1 ;
        }
        else
        {
            return 0 ;
        }
    }
}

BMDeNoiseType
SelectShotMode::
getAlgDecision(int v, int i)
{
    if( ruleJudgement( v, i, &mDecisionParam ) )
    {
        TRACE_FUNC("DENOISE MFHR - alg");
        return DN_TYPE_MFHR ;
    }
    else
    {
        TRACE_FUNC("DENOISE BMNR - alg");
        return DN_TYPE_BMNR ;
    }
}

bool
SelectShotMode::
getDecisionConf(int v, int i)
{
    if( !ruleJudgement( v, i, &mDecisionParam_lb ) &&
         ruleJudgement( v, i, &mDecisionParam_ub ) )
    {
        return 0 ; // uncertain
    }
    else
    {
        return 1 ;
    }
}

BMDeNoiseType
SelectShotMode::
decideDenoiseType(int* scene_info, int iso_speed, int is_executed)
{
    BMDeNoiseType ret ;

    if( iso_speed <= mCriteriaSettings.CUSTOMER_ISO_BM_MIN ) // iso seg 2DNR, MFNR
    {
        ret = DN_TYPE_TBD ;
    }
    else if( iso_speed <= mCriteriaSettings.CUSTOMER_ISO_HR_MIN ) // iso seg BMNR
    {
        ret = DN_TYPE_BMNR ;
    }
    else // iso seg BMNR, MFHR
    {
        if( is_executed == 0 ) // no execution info, remain as previous decision
        {
            ret = mPrvDenoiseType ;
        }
        else
        {
            setDecisionCriteria( iso_speed ) ;

            bool isInfoValid = getInfoStatus(scene_info) ;

            if( isInfoValid == 0 )
            {
                ret = DN_TYPE_TBD ;
            }
            else
            {
                bool isSceneStable = getStableStatus(scene_info) ;
                bool isDecisionConfirmed = getDecisionConf( scene_info[mDecisionParam.idx_var], scene_info[IDX_INT] ) ;
                BMDeNoiseType algDecision = getAlgDecision( scene_info[mDecisionParam.idx_var], scene_info[IDX_INT] ) ;

                if( isSceneStable || !isDecisionConfirmed )
                {
                    ret = mPrvDenoiseType ;
                }
                else
                {
                    ret = algDecision ;

                    mPrvDenoiseType = ret ;
                    memcpy( mPrvSceneInfo, scene_info, sizeof(int)*SCENE_INFO_SIZE ) ;
                }
                TRACE_FUNC("scene_info[%d], decision conf[%d], use decision:%d", isSceneStable, isDecisionConfirmed, ret);
            }
        }
    }
    TRACE_FUNC("final decision [%d]", ret ) ;
    if(ret == DN_TYPE_BMNR){
        ret = DN_TYPE_TBD;
        TRACE_FUNC("final decision change to [%d]", ret ) ;
    }
    return ret ;
}

bool
SelectShotMode::
getInfoStatus(int* scene_info)
{
    if( scene_info[IDX_N3D] != 0
    ||  scene_info[IDX_REC] >= mDecisionParam.thr_rect
    ||  scene_info[IDX_NFE] <  mCriteriaSettings.CUSTOMER_FE_NUM_MIN
    )
    {
        MY_LOGW("info status abnormal [%d,%d,%d]", scene_info[IDX_N3D], scene_info[IDX_REC], scene_info[IDX_NFE] );
        return 0 ;
    }
    else
        return 1 ;
}


}; //NSFeaturePipe
}; //NSCamFeature
}; //NSCam