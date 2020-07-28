/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
#ifndef __MTK_STEREO_KERNEL_H__
#define __MTK_STEREO_KERNEL_H__

#include "MTKStereoKernelDef.h"
//#include "dbg_cam_param.h"      // For DEBUG_CAM_MF_MID. It also includes "dbg_cam_mf_param.h (for DEBUG_MF_INFO_T)".

#define MAX_FRAME_NUM                       5
#define CAPTURE_MAX_FRAME_NUM               1

typedef enum
{
    STEREO_KERNEL_FEATURE_BEGIN = 0,
    STEREO_KERNEL_FEATURE_GET_RESULT,
    STEREO_KERNEL_FEATURE_GET_STATUS,
    STEREO_KERNEL_FEATURE_SAVE_LOG,
    STEREO_KERNEL_FEATURE_SET_PROC_INFO,
    STEREO_KERNEL_FEATURE_GET_WORK_BUF_INFO,
    STEREO_KERNEL_FEATURE_SET_WORK_BUF_INFO,
    STEREO_KERNEL_FEATURE_GET_DEFAULT_TUNING, 
    STEREO_KERNEL_FEATURE_GET_WIN_REMAP_INFO, 
    STEREO_KERNEL_FEATURE_MAX,
    STEREO_KERNEL_FEATURE_LOAD_NVRAM,
    STEREO_KERNEL_FEATURE_SAVE_NVRAM,
    STEREO_KERNEL_FEATURE_LOAD_LENS_INFO,
    STEREO_KERNEL_FEATURE_GET_EEPROM_START,

    STEREO_KERNEL_FEATURE_DEBUG,
}
STEREO_KERNEL_FEATURE_ENUM;


/*
    CLASS
*/
class MTKStereoKernel{
public:
    static MTKStereoKernel* createInstance();
    virtual void   destroyInstance() = 0;

    virtual ~MTKStereoKernel(){};
    virtual MRESULT StereoKernelInit(void* InitInData, void* TuneInData);
    virtual MRESULT StereoKernelMain();
    virtual MRESULT StereoKernelReset();

    virtual MRESULT StereoKernelFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:
    
};


#endif
