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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
package com.mediatek.camera.portability.pq;

import android.util.Log;

/**
 * This class provides the APIs for users to indirectly use PictureQuality
 * class in MTK framework.
 */
public class PictureQuality {
    private static final String TAG = PictureQuality.class.getSimpleName();

    /**
     * Enter camera picture quality mode.
     */
    public static void enterCameraMode() {
        if (isSupported()) {
            Log.d(TAG, "[enterCameraMode]");
            PictureQualityWrapper.enterCameraMode();
        }
    }

    /**
     * Exit camera picture quality mode.
     */
    public static void exitCameraMode() {
        if (isSupported()) {
            Log.d(TAG, "[exitCameraMode]");
            PictureQualityWrapper.exitCameraMode();
        }
    }

    /**
     * Get current ESS LEDMinStep.
     *
     * @return the current ESS LEDMinStep.
     */
    public static int getMinStepOfESSLED() {
        if (isSupported()) {
            Log.d(TAG, "[getMinStepOfESSLED]");
            return PictureQualityWrapper.getMinStepOfESSLED();
        }
        return -1;
    }

    /**
     * Get ESS OLEDMinStep.
     *
     * @return The ESS OLEDMinStep.
     */
    public static int getMinStepOfESSOLED() {
        if (isSupported()) {
            Log.d(TAG, "[getMinStepOfESSOLED]");
            return PictureQualityWrapper.getMinStepOfESSOLED();
        }
        return -1;
    }

    /**
     * Set ESS LEDMinStep.
     *
     * @param value the ESS LEDMinStep.
     */
    public static void setMinStepOfESSLED(int value) {
        if (isSupported()) {
            Log.d(TAG, "[setMinStepOfESSLED]");
            PictureQualityWrapper.setMinStepOfESSLED(value);
        }
    }

    /**
     * Set ESS OLEDMinStep.
     *
     * @param value the ESS OLEDMinStep.
     */
    public static void setMinStepOfESSOLED(int value) {
        if (isSupported()) {
            Log.d(TAG, "[setMinStepOfESSOLED]");
            PictureQualityWrapper.setMinStepOfESSOLED(value);
        }
    }

    private static boolean isSupported() {
        boolean supported = false;
        try {
            Class.forName("com.mediatek.pq.PictureQuality");
            supported = true;
        } catch (ClassNotFoundException e) {
            supported = false;
        }
        Log.d(TAG, "[isSupported], return:" + supported);
        return supported;
    }
}
