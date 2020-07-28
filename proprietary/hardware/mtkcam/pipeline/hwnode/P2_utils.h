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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2_UTILS_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2_UTILS_H_

#include "hwnode_utilities.h"
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/utils/hw/HwTransform.h>
//
using namespace android;
//using namespace NSCam;
//using namespace NSCam::v3;
using namespace NSCamHW;
using namespace NSCam::NSIoPipe;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
/******************************************************************************
 *
 ******************************************************************************/
class Cropper
{
    public:
        typedef struct crop_info : public RefBase
        {
            // port
            MBOOL                isResized;
            //
            MSize                sensor_size;
            // p1 crop infos
            MRect                crop_p1_sensor;
            MSize                dstsize_resizer;
            MRect                crop_dma;
            // p1 bin infos
            MRect                crop_p1_bin;
            MSize                bin_size;
            //
            //simpleTransform      tranActive2Sensor;
            HwMatrix             matActive2Sensor;
            HwMatrix             matSensor2Active;
            simpleTransform      tranSensor2Resized;
            //
            // target crop: cropRegion
            // not applied eis's mv yet, but the crop area is already reduced by
            // EIS ratio.
            // _a: active array coordinates
            // _s: sensor coordinates
            // active array coordinates
            MRect                crop_a;
            MBOOL                isFSCEnabled;
            MRectF               cropf_a;
            //MPoint               crop_a_p;
            //MSize                crop_a_size;
            // sensor coordinates
            //MPoint               crop_s_p;
            //MSize                crop_s_size;
            // resized coordinates
            //
            MBOOL                isEisEabled;
            vector_f             eis_mv_a; //active array coor.
            vector_f             eis_mv_s; //sensor coor.
            vector_f             eis_mv_r; //resized coor.

        } crop_info_t;

        static MVOID calcViewAngle(
            MBOOL bEnableLog,
            crop_info_t const& cropInfos,
            MSize const& dstSize,
            MCropRect& result
        );

        static MVOID calcViewAngleF(
            MBOOL bEnableLog,
            MBOOL bUseSubPixel,
            crop_info_t const& cropInfos,
            MSize const& dstSize,
            MCropRect& result,
            MRectF& resultf
        );

        static MBOOL refineBoundary(
            MSize const& bufSize,
            MCropRect& crop,
            MBOOL hwLimit = MTRUE
        );

        static MVOID dump(
            crop_info_t const& cropInfos
        );

};
/******************************************************************************
 *
 ******************************************************************************/
} // namespace v3
} // namespace NSCam
/******************************************************************************
 *
 ******************************************************************************/

#endif // _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P2_UTILS_H_
