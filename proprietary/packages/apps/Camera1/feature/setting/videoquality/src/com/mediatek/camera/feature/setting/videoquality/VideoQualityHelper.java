/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.videoquality;

import android.app.Activity;
import android.graphics.Point;
import android.hardware.Camera;
import android.media.CamcorderProfile;
import android.view.Display;

import com.mediatek.camera.R;

import com.mediatek.camera.portability.CamcorderProfileEx;

import java.util.Iterator;
import java.util.List;

/**
 * Video quality helper to provide util methods.
 */
public class VideoQualityHelper {

    private static final int QUALITY_4K_1 = 3840 * 2160;
    private static final int QUALITY_4K_2 = 3840 * 2176;
    private static final int QUALITY_2K = 2560 * 1440;
    private static final int QUALITY_FHD_1 = 1920 * 1080;
    private static final int QUALITY_FHD_2 = 1920 * 1088;
    private static final int QUALITY_HD = 1280 * 720;
    private static final int QUALITY_VGA_1 = 640 * 480;
    private static final int QUALITY_VGA_2 = 720 * 480;
    private static final int QUALITY_QVGA = 320 * 240;
    private static final int QUALITY_CIF = 352 * 288;
    private static final int QUALITY_QCIF = 176 * 144;

     static String getCurrentResolution(int cameraId, String quality) {
        CamcorderProfile profile = CamcorderProfileEx.getProfile(cameraId,
                Integer.parseInt(quality));
        return profile.videoFrameWidth + "x" + profile.videoFrameHeight;
    }

     static String getQualityTitle(Activity activity, String quality, int cameraID) {
        CamcorderProfile profile = CamcorderProfileEx.getProfile(cameraID,
                Integer.parseInt(quality));
        String title = "";
        switch (profile.videoFrameHeight * profile.videoFrameWidth) {
            case QUALITY_4K_1:
            case QUALITY_4K_2:
                title = activity.getResources().getString(R.string.quality_4k);
                break;
            case QUALITY_2K:
                title = activity.getResources().getString(R.string.quality_2k);
                break;
            case QUALITY_FHD_1:
            case QUALITY_FHD_2:
                title = activity.getResources().getString(R.string.quality_fhd);
                break;
            case QUALITY_HD:
                title = activity.getResources().getString(R.string.quality_hd);
                break;
            case QUALITY_VGA_1:
            case QUALITY_VGA_2:
                title = activity.getResources().getString(R.string.quality_vga);
                break;
            case QUALITY_QVGA:
                title = activity.getResources().getString(R.string.quality_qvga);
                break;
            case QUALITY_CIF:
                title = activity.getResources().getString(R.string.quality_cif);
                break;
            case QUALITY_QCIF:
                title = activity.getResources().getString(R.string.quality_qcif);
                break;
            default:
                break;
        }
        return title;
    }

    /**
     * The google default Video qualities sorted by size.
     */
    static int[] sVideoQualities = new int[] {
            CamcorderProfile.QUALITY_2160P,
            CamcorderProfile.QUALITY_1080P,
            CamcorderProfile.QUALITY_720P,
            CamcorderProfile.QUALITY_480P,
            CamcorderProfile.QUALITY_CIF,
            CamcorderProfile.QUALITY_QVGA,
            CamcorderProfile.QUALITY_QCIF
    };

    /**
     * The mtk default Video qualities sorted by size.
     */
    static int[] sMtkVideoQualities = new int[] {
            CamcorderProfileEx.QUALITY_FINE_4K2K,
            CamcorderProfileEx.QUALITY_FINE,
            CamcorderProfileEx.QUALITY_HIGH,
            CamcorderProfileEx.QUALITY_MEDIUM,
            CamcorderProfileEx.QUALITY_LOW
    };



}
