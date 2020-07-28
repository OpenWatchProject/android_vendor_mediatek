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
#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_N3D_SHOTMODE_SELECT_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_N3D_SHOTMODE_SELECT_H_

#include <camera_custom_stereo.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

enum BMDeNoiseType
{
    DN_TYPE_TBD = 0,
    DN_TYPE_BMNR,
    DN_TYPE_MFHR,
};

struct BMDeNoiseCriteria
{
    int var_low  ;
    int var_high ;
    int int_low  ;
    int int_high ;
    int thr_rect ;
    int idx_var  ;
};

class SelectShotMode
{
    static const int SCENE_INFO_SIZE = 15;
    static const int IDX_REC = 2;
    static const int IDX_NFE = 3;
    static const int IDX_N3D = 10;
    static const int IDX_INT = 11;
    static const int IDX_DAC = 14;
    static const int IDX_VAR_OFST = 4 ;

public:
    SelectShotMode(){ mPrvSceneInfo[0] = -1 ; mPrvDenoiseType = DN_TYPE_TBD ; } ;
    BMDeNoiseType decideDenoiseType(int* scene_info, int iso_speed, int is_executed) ;
    virtual ~SelectShotMode();

private:
    int                         mPrvSceneInfo[SCENE_INFO_SIZE] ;
    BMDeNoiseType               mPrvDenoiseType ;
    BMDeNoiseCriteria           mDecisionParam ;
    BMDeNoiseCriteria           mDecisionParam_lb ;
    BMDeNoiseCriteria           mDecisionParam_ub ;
    BMDeNoiseCriteriaSettings   mCriteriaSettings ;

    void                        setDecisionCriteria(int iso_speed) ;
    bool                        getInfoStatus(int* scene_info) ;
    bool                        getStableStatus(int* scene_info) ;
    bool                        ruleJudgement(int v, int i, BMDeNoiseCriteria* rule) ;
    BMDeNoiseType               getAlgDecision(int v, int i) ;
    bool                        getDecisionConf(int v, int i) ;
} ;


} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // name

#endif