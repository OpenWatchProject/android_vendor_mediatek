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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_JPEGNODEPLUGIN_JPGPACK_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_JPEGNODEPLUGIN_JPGPACK_H_

#include "IJpgPlugin.h"
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>
#include <mtkcam3/feature/stereo/libstereoinfoaccessor/StereoInfoAccessor.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

/******************************************************************************
 *
 ******************************************************************************/
class JpgPack : public NSCam::v3::IJpgPlugin {

public:

    struct ConfigJpgParams {
        android::sp<IImageBuffer> pIn_BokehJpg;
        android::sp<IImageBuffer> pIn_CleanJpg;
        android::sp<IImageBuffer> pIn_DepthmapYuv;
        android::sp<IImageBuffer> pOut_ResultJpg;

        ConfigJpgParams()
            :pIn_BokehJpg(NULL),
            pIn_CleanJpg(NULL),
            pIn_DepthmapYuv(NULL),
            pOut_ResultJpg(NULL)
            {}

        ConfigJpgParams(
            android::sp<IImageBuffer> _pIn_BokehJpg,
            android::sp<IImageBuffer> _pIn_CleanhJpg,
            android::sp<IImageBuffer> _pIn_DepthmapYuv,
            android::sp<IImageBuffer> _pIn_ResultJpg
        )
            :pIn_BokehJpg(_pIn_BokehJpg),
            pIn_CleanJpg(_pIn_CleanhJpg),
            pIn_DepthmapYuv(_pIn_DepthmapYuv),
            pOut_ResultJpg(_pIn_ResultJpg)
            {}
    };

    JpgPack();

    ~JpgPack();

    MERROR init();

    MERROR config(ConfigJpgParams const& rparams);

    size_t process();

    MERROR uninit();

private:
    // image buffer
    android::sp<IImageBuffer> mpIn_BokehJpg;
    android::sp<IImageBuffer> mpIn_CleanJpg;
    android::sp<IImageBuffer> mpIn_DepthmapYuv;
    android::sp<IImageBuffer> mpOut_ResultJpg;
};

};  //namespace v3
};  //namespace NSCam
#endif //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_JPEGNODEPLUGIN_JPGPACK_H_
