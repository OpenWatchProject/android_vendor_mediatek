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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFLLNODE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFLLNODE_H_

#include "MFCNode.h"
#include <mtkcam/feature/mfnr/IMfllCore.h>
#include <mtkcam/aaa/IHal3A.h>

#define MFBMODE_NONE    0
#define MFBMODE_MFLL    1
#define MFBMODE_AIS     2

namespace NSCam {
namespace v3 {

/**
 *  MfllNode is a node to perform MFNR (Multi-Frame Noise Reduction) feature. To
 *  create MfllNode please invoke MfllNode::createInstance() to create.
 */
class MfllNode : public MFCNode
{
public:
    /* To create MfllNode instance */
    static android::sp<MfllNode> createInstance(const MFCNode::MfcAttribute &attr = MFCNode::MfcAttribute());

    /**
     *  To query if it's time to use MfllShot
     *  @param mfbMode              0: none, 1: MFNR, 2: AIS
     *  @param [out] isUseMfllShot  0: use the original shot, 1: use MfllShot
     *  @param sensorId             sensor id
     *  @param custom_arg           customized argument
     */
    static MERROR getCaptureInfo(int mfbMode, int &isUseMfllShot, int sensorId, void *custom_arg = NULL);

    /**
     *  To set capture information directly
     */
    static MERROR setCaptureInfo(
            int mfbMode,
            int sensorId,
            int isUseMfllShot,
            int iso,
            int exp,
            int frameBlend,
            int frameCapture,
            void *custom_arg = NULL);

    /* To get capture parameters */
    static MERROR getCaptureParamSet(
            MINT32 openId,
            android::Vector<NS3Av3::CaptureParam_T>& vCaptureParams,
            android::Vector<NS3Av3::CaptureParam_T>* vOrigCaptureParams = NULL);

}; /*MfllNode*/
}; /*namespace v3*/
}; /*namespace NSCam*/
#endif /*_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_MFLLNODE_H_*/

