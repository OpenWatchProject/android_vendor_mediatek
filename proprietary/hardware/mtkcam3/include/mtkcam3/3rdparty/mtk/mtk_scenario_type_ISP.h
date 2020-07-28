/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_MTK_MTK_SCENARIOTYPE_ISP_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_MTK_MTK_SCENARIOTYPE_ISP_H_

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace pipeline {
namespace policy {
namespace scenariomgr {

enum eIspHidlScenarioMtk
{
    MTK_ISP_HIDL_SCENARIO_UNKNOW = 300,

    // capture feature:
    MTK_ISP_HIDL_SCENARIO_CAPTURE_ALL,
    // single frame input
    MTK_ISP_HIDL_SCENARIO_CAPTURE_SINGLE_RAW_TO_YUV,
    MTK_ISP_HIDL_SCENARIO_CAPTURE_SINGLE_RAW_TO_JPEG,
    MTK_ISP_HIDL_SCENARIO_CAPTURE_SINGLE_YUV_TO_YUV,
    MTK_ISP_HIDL_SCENARIO_CAPTURE_SINGLE_YUV_TO_JPEG,
    // multiple frame input
    MTK_ISP_HIDL_SCENARIO_CAPTURE_MULTIPLE_RAW_TO_RAW,
    MTK_ISP_HIDL_SCENARIO_CAPTURE_MULTIPLE_RAW_TO_YUV,
    MTK_ISP_HIDL_SCENARIO_CAPTURE_MULTIPLE_RAW_TO_JPEG,
    MTK_ISP_HIDL_SCENARIO_CAPTURE_MULTIPLE_YUV_TO_YUV,
    MTK_ISP_HIDL_SCENARIO_CAPTURE_MULTIPLE_YUV_TO_JPEG,
    // TODO: reserve only for mtk proprietary capture scenario (max:349)
    MTK_ISP_HIDL_SCENARIO_CAPTURE_MAX_NUM,

    // streaming
    // TODO: reserve only for mtk proprietary streaming scenario (max:399)
    MTK_ISP_HIDL_SCENARIO_STREAMING_MAX_NUM,
};

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace scenariomgr
};  //namespace policy
};  //namespace pipeline
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_3RDPARTY_MTK_MTK_SCENARIOTYPE_ISP_H_

