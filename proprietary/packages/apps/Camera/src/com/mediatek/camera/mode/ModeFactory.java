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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.mode;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.ICameraMode;
import com.mediatek.camera.ICameraMode.CameraModeType;
import com.mediatek.camera.mode.facebeauty.FaceBeautyMode;
import com.mediatek.camera.mode.panorama.PanoramaMode;
import com.mediatek.camera.mode.pip.PipPhotoMode;
import com.mediatek.camera.mode.pip.PipVideoMode;
import com.mediatek.camera.mode.stereocamera.StereoCameraMode;
import com.mediatek.camera.mode.stereocamera.StereoPhotoMode;
import com.mediatek.camera.mode.stereocamera.StereoVideoMode;
import com.mediatek.camera.util.Log;

public class ModeFactory {
    private static final String TAG = "ModeFactory";

    private volatile static ModeFactory sModeFactory = null;

    private ModeFactory() {
    }

    public static ModeFactory getInstance() {
        if (null == sModeFactory) {
            synchronized (ModeFactory.class) {
                if (null == sModeFactory) {
                    sModeFactory = new ModeFactory();
                }
            }

        }
        return sModeFactory;
    }

    public ICameraMode createMode(CameraModeType type, ICameraContext cameraContext) {
        switch (type) {
        case EXT_MODE_PHOTO:
            return new PhotoMode(cameraContext);

        case EXT_MODE_FACE_BEAUTY:
            return new FaceBeautyMode(cameraContext);

        case EXT_MODE_PANORAMA:
            return new PanoramaMode(cameraContext);

        case EXT_MODE_VIDEO:
            return new VideoMode(cameraContext);

        case EXT_MODE_PHOTO_PIP:
            return new PipPhotoMode(cameraContext);

        case EXT_MODE_VIDEO_PIP:
            return new PipVideoMode(cameraContext);

        case EXT_MODE_STEREO_CAMERA:
            return new StereoCameraMode(cameraContext);

        case EXT_MODE_PHOTO_STEREO:
            return new StereoPhotoMode(cameraContext);

        case EXT_MODE_VIDEO_STEREO:
            return new StereoVideoMode(cameraContext);
        default:
            return new DummyMode();
        }
    }

}
