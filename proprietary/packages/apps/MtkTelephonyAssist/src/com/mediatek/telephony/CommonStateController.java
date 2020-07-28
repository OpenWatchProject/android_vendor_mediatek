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

package com.mediatek.telephony;

import android.content.Context;
import android.hardware.display.DisplayManager;
import android.view.Display;

/**
 * Sync sreen status to modem.
 */
public class CommonStateController extends BaseController {
    // Consistent with modem screen status flag.
    private static final int SCREEN_STATUS = 3;
    private int[] mScreenStatus;

    private final DisplayManager.DisplayListener mDisplayListener =
            new DisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int displayId) {
        }

        @Override
        public void onDisplayRemoved(int displayId) {
        }

        @Override
        public void onDisplayChanged(int displayId) {
            int screenStatus = (isScreenOn() ? 1 : 0);
            for (int i = 0; i < mPhones.length; i++) {
                if (mScreenStatus[i] != screenStatus) {
                    syncScreenStateToMd(i, screenStatus);
                }
            }
        }
    };

    public CommonStateController(Context context) {
        super(context);
    }

    @Override
    public void onRadioChanged(int phoneId) {
        if (mPhones[phoneId].isRadioAvailable()) {
            syncScreenStateToMd(phoneId, (isScreenOn() ? 1 : 0));
        }
    }

    @Override
    public void initialize() {
        mScreenStatus = new int[mPhones.length];
        for (int i = 0; i < mPhones.length; i++) {
            mScreenStatus[i] = -1;
        }

        DisplayManager dm = (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
        dm.registerDisplayListener(mDisplayListener, null);
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        DisplayManager dm = (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
        dm.unregisterDisplayListener(mDisplayListener);
    }

    private void syncScreenStateToMd(int phoneId, int screenStatus) {
        if (mAssistRIL[phoneId] != null) {
            mAssistRIL[phoneId].syncScreenState(SCREEN_STATUS, screenStatus, -1, null);
            mScreenStatus[phoneId] = screenStatus;
        }
    }

    /**
     * @return True if one the device's screen (e.g. main screen, wifi display, HDMI display, or
     *         Android auto, etc...) is on.
     */
    private boolean isScreenOn() {
        // Note that we don't listen to Intent.SCREEN_ON and Intent.SCREEN_OFF because they are no
        // longer adequate for monitoring the screen state since they are not sent in cases where
        // the screen is turned off transiently such as due to the proximity sensor.
        final DisplayManager dm =
                (DisplayManager) mContext.getSystemService(Context.DISPLAY_SERVICE);
        Display[] displays = dm.getDisplays();

        if (displays != null) {
            for (Display display : displays) {
                // Anything other than STATE_ON is treated as screen off, such as STATE_DOZE,
                // STATE_DOZE_SUSPEND, etc...
                if (display.getState() == Display.STATE_ON) {
                    return true;
                }
            }
            return false;
        }

        return false;
    }
}
