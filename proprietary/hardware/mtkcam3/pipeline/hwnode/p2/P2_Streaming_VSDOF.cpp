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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include <cutils/properties.h>
#include "P2_StreamingProcessor.h"

#include "P2_DebugControl.h"
#define P2_CLASS_TAG    Streaming_VSDOF
#define P2_TRACE        TRACE_STREAMING_VSDOF
#include "P2_LogHeader.h"

#include <mtkcam3/feature/stereo/StereoCamEnum.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_P2_STR_PROC);

namespace P2
{

MBOOL StreamingProcessor::isRequestValid_VSDOF(const sp<P2Request>& request) const
{
    MBOOL valid = MTRUE;
    const ILog &log = request->mLog;
    TRACE_S_FUNC(log, "VSDOF check param...");// TODO you can add more log when open property

    if( (mPipeUsageHint.mDualMode & NSCam::v1::Stereo::E_STEREO_FEATURE_VSDOF) ||
              (mPipeUsageHint.mDualMode & NSCam::v1::Stereo::E_STEREO_FEATURE_MTK_DEPTHMAP) )
    {
        if( mP2Info.getConfigInfo().mAllSensorID.size() < 2)
        {
            MY_S_LOGE(log, "VSDOF not allow sensor number < 2!!");
            valid = MFALSE;
        }
        else
        {
            MUINT32 main1ID = mP2Info.getConfigInfo().mAllSensorID[0];
            MUINT32 main2ID = mP2Info.getConfigInfo().mAllSensorID[1];
            //Img Buffer Related
            MBOOL rrz1Valid = request->isValidImg(IN_RESIZED, main1ID);
            MBOOL rrz2Valid = request->isValidImg(IN_RESIZED, main2ID);
            MBOOL lcs1Valid = request->isValidImg(IN_LCSO, main1ID);
            MBOOL lcs2Valid = request->isValidImg(IN_LCSO, main2ID);
            //Correspoding Metadata Related
            MBOOL main1Metahal1Valid = request->isValidMeta(IN_APP     , main1ID);//PBID_IN_HAL_META_MAIN1
            MBOOL main1MetaDynValid  = request->isValidMeta(IN_P1_HAL  , main1ID);//PBID_IN_P1_RETURN_META
            MBOOL main1MetaAppValid  = request->isValidMeta(IN_P1_APP  , main1ID);//PBID_IN_APP_META
            MBOOL main2MetaHalValid  = request->isValidMeta(IN_P1_HAL  , main2ID);//PBID_IN_HAL_META_MAIN2

            // TODO check vsdof version to decide can run or not
            if (rrz1Valid && rrz2Valid && lcs1Valid && lcs2Valid &&
                main1Metahal1Valid && main1MetaDynValid && main1MetaAppValid &&
                main2MetaHalValid)
            {
                valid = MTRUE;
            }
            else
            {
                valid = MFALSE;
                MY_S_LOGW(log, "VSDOF not allow request. ImgBuffer:"
                               "rrz1(%d) rrz2(%d) lcso1(%d) lcso2(%d)",
                               rrz1Valid, rrz2Valid, lcs1Valid, lcs2Valid);
                MY_S_LOGW(log, "VSDOF NOT ALLOW Request. Metadat: "
                               "main1(%d) main1(%d) main1(%d) main2(%d)",
                               main1Metahal1Valid, main1MetaDynValid,
                               main1MetaAppValid, main2MetaHalValid);
            }
        }

    }
    return valid;
}

}; // namespace P2
