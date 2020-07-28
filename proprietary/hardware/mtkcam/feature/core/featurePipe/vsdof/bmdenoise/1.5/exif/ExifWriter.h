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
#ifndef _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_EXIF_WRITER_H_
#define _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_EXIF_WRITER_H_

#include "BMDeNoisePipe_Common.h"
#include "BMDeNoisePipeNode.h"

/* Exif info */
#include <debug_exif/ver2/dbg_id_param.h>
#include <debug_exif/cam/ver2/dbg_cam_param.h>

using namespace dbg_cam_reservec_param_0;

// #include <custom/feature/mfnr/camera_custom_mfll.h>
// #include <custom/debug_exif/dbg_exif_param.h>
// #if (MFLL_MF_TAG_VERSION > 0)
// using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
// or using namespace dbg_cam_mf_param_2;
// #endif

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

class ExifWriter
{
    public:
        struct BMDN_EXIF_DATA_T
        {
            MINT32 version = -1;
            MINT32 type = -1;
            MINT32 composition = -1;
            MINT32 ra = -1;
            MRect  ratioCrop = MRect(MPoint(-1,-1),MSize(-1,-1));
            MRect  fovCrop = MRect(MPoint(-1,-1),MSize(-1,-1));
        };

    public:
        ExifWriter(const char *name);
        ~ExifWriter();

        MBOOL                       doExifUpdate(
                                            PipeRequestPtr request,
                                            BMDN_EXIF_DATA_T exifData);

        uint32_t                    doExifUpdate(
                                            PipeRequestPtr request,
                                            BMDeNoiseFeatureType type,
                                            map<MINT32, MINT32>& data);

        MBOOL                       doExifUpdate_MFNR(
                                            PipeRequestPtr request,
                                            const std::map<unsigned int, uint32_t>& data);

        MBOOL                       sendData(
                                            MINT32 reqId,
                                            MINT32 tag,
                                            MINT32 value);

        MBOOL                       makeExifFromCollectedData(
                                            PipeRequestPtr request,
                                            BMDeNoiseFeatureType type);
    private:
        const char *                tagToName(int tag);

    private:
        const char*                 mName;

        MINT32                      mEnable = -1;

        static Mutex&                                      gLock = *new Mutex();
        static KeyedVector< MINT32, map<MINT32, MINT32> > gvCollectedData;
};
};
};
};
#endif // _MTK_CAMERA_VSDOF_BMDENOISE_FEATURE_PIPE_EXIF_WRITER_H_
