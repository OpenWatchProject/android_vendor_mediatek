/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.server.telecom;

import android.content.Context;
import android.os.SystemProperties;
import android.os.SystemVibrator;
import android.os.VibrationEffect;
import android.os.Vibrator;
import com.android.server.telecom.Call;
import com.android.server.telecom.CallState;
import com.android.server.telecom.CallsManagerListenerBase;

/**
 * Vibrate when call state changes to active.
 */
public class CallConnectedVibrator extends CallsManagerListenerBase {

    private final Vibrator mVibrator;
    private static final long VIBRATE_TIME_MILLIS = 200;
    private static final VibrationEffect EFFECT_CONNECTED =
            VibrationEffect.createOneShot(VIBRATE_TIME_MILLIS, VibrationEffect.DEFAULT_AMPLITUDE);
    /**
     * M: This SystemProperties would be configured in the EngineerMode application.
     * By default the vibrator would work. The property would be undefined or 1.
     * User can disable it in the EngineerMode. If so, the property would be set to 0.
     */
    private final static String CONFIG_VIBRATE_ON_CONNECTED =
            "persist.vendor.radio.telecom.vibrate";

    /**
     * Initializes the required Vibrator.
     */
    public CallConnectedVibrator(Context context) {
        mVibrator = new SystemVibrator(context);
    }

    @Override
    public void onCallStateChanged(Call call, int oldState, int newState) {
        // CDMA framework will handle the CDMA vibrate by itself.
        if (!call.isCdma()
                && (oldState == CallState.CONNECTING || oldState == CallState.DIALING)
                && newState == CallState.ACTIVE) {
            // M: Check this SystemProperties.
            if (SystemProperties.getInt(CONFIG_VIBRATE_ON_CONNECTED, 1) == 1) {
                mVibrator.vibrate(EFFECT_CONNECTED);
            }
        }
    }
}
