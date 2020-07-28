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


import com.mediatek.camera.portability.ReflectUtil;
import com.mediatek.pq.PictureQuality;

import java.lang.reflect.Method;

/**
 * This class packages the APIs in PictureQuality class in the MTK framework.
 */
class PictureQualityWrapper {
    private static final int MIN_STEP_ESS_LED = 4;
    private static final int MIN_STEP_SSO_LED = 2;

    /**
     * Enter camera picture quality mode.
     */
    public static void enterCameraMode() {
        PictureQuality.setMode(PictureQuality.MODE_CAMERA);
    }

    /**
     * Exit camera picture quality mode.
     */
    public static void exitCameraMode() {
        PictureQuality.setMode(PictureQuality.MODE_NORMAL);
    }

    /**
     * Get current ESS LEDMinStep.
     *
     * @return the current ESS LEDMinStep.
     */
    public static int getMinStepOfESSLED() {
        Method getESSLEDMinStepMethod = ReflectUtil.getMethod(PictureQuality.class,
                "getESSLEDMinStep");
        if (getESSLEDMinStepMethod == null) {
            return MIN_STEP_ESS_LED;
        }
        return (int) ReflectUtil.callMethodOnObject(null, getESSLEDMinStepMethod);
    }

    /**
     * Get ESS OLEDMinStep.
     *
     * @return The ESS OLEDMinStep.
     */
    public static int getMinStepOfESSOLED() {
        Method getESSOLEDMinStepMethod = ReflectUtil.getMethod(PictureQuality.class,
                "getESSOLEDMinStep");
        if (getESSOLEDMinStepMethod == null) {
            return MIN_STEP_SSO_LED;
        }
        return (int) ReflectUtil.callMethodOnObject(null, getESSOLEDMinStepMethod);
    }

    /**
     * Set ESS LEDMinStep.
     *
     * @param value the ESS LEDMinStep.
     */
    public static void setMinStepOfESSLED(int value) {
        Method setESSLEDMinStepMethod = ReflectUtil.getMethod(PictureQuality.class,
                "setESSLEDMinStep", int.class);
        if (setESSLEDMinStepMethod != null) {
            ReflectUtil.callMethodOnObject(null, setESSLEDMinStepMethod, value);
        }
    }

    /**
     * Set ESS OLEDMinStep.
     *
     * @param value the ESS OLEDMinStep.
     */
    public static void setMinStepOfESSOLED(int value) {
        Method setESSOLEDMinStepMethod = ReflectUtil.getMethod(PictureQuality.class,
                "setESSOLEDMinStep", int.class);
        if (setESSOLEDMinStepMethod != null) {
            ReflectUtil.callMethodOnObject(null, setESSOLEDMinStepMethod, value);
        }
    }

}
