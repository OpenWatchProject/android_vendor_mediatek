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

#ifndef _MTK_CAMERA_INC_FEATUREIO_CAPTRUENR_H_
#define _MTK_CAMERA_INC_FEATUREIO_CAPTRUENR_H_

#include <mtkcam/common.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/algorithm/libnr/MTKNR.h>
#include <camera_custom_capture_nr.h>

using namespace NS3A;

/*******************************************************************************
*
********************************************************************************/
class MTKNR;
class NRInitInfo;
class NRTuningInfo;
class NRImageInfo;
class NRResultInfo;

MBOOL doTwoPassHwNR(
        MUINT32 openId,
        IImageBuffer* buf,
        NS3A::EIspProfile_T const profile0,
        NS3A::EIspProfile_T const profile1
        );

MBOOL doHwNR(
        MUINT32 const openId,
        IImageBuffer* in,
        IImageBuffer* out0,
        IImageBuffer* out1,
        MRect const crop,
        MUINT32 const transform_0,
        NS3A::EIspProfile_T const profile
        );

// This is a bad API, I don't like it
// This API exists because CapPass2 cannot get iso and scenario information
void SwNR_setParam(MUINT32 iso, MUINT32 scenario, MINT32  perfLevel);

class ISwNR
{
public:
    static ISwNR*   createInstance(MUINT32 openId);
    virtual         ~ISwNR() {};
    virtual MBOOL   doSwNR(NSCam::IImageBuffer* pBuf) = 0;
    //virtual MBOOL getDebugInfo(NSCam::IMetadata& halMetadata) const = 0;
    virtual MVOID   dumpParam(char * const filename) = 0;
};
#endif  //  _MTK_CAMERA_INC_FEATUREIO_CAPTRUENR_H_
