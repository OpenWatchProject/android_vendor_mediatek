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

package com.mediatek.camera.platform;

public interface IFocusManager {
    public interface FocusListener {

        public void autoFocus();

        public void cancelAutoFocus();

        public boolean capture();

        public void startFaceDetection();

        public void stopFaceDetection();

        public void setFocusParameters();

        void playSound(int soundId);
    }

    /**
     * set focus manager listener
     *
     * @param listener
     *            receive callback
     * @return
     */
    public boolean setListener(FocusListener listener);

    /**
     * lock ae
     *
     * @param lock
     *            whether lock ae, true -> lock, false -> unlock
     * @return
     */
    public boolean setAeLock(boolean lock);

    /**
     * lock awb
     *
     * @param lock
     *            true -> lock, false -> unlock
     * @return
     */
    public boolean setAwbLock(boolean lock);

    /**
     *
     * @return
     */
    public boolean resetTouchFocus();

    public void removeMessages();
    /**
     *
     * @return
     */
    public boolean updateFocusUI();

    /**
     * Triggers the auto focus and capture
     */
    public void focusAndCapture();

    /**
     * Set multi-zone af data to focusmanager.
     * @param data The data of multi-zone af.
     */
    public void setAfData(byte[] data);

    public void onAutoFocusMoving(boolean moving);

    public void onPreviewStarted();

    public void onPreviewStopped();

    public String getFocusMode();

    public boolean isFocusingSnapOnFinish();

    public void onShutterDown();

    public void onShutterUp();

    public void onAutoFocus(boolean focused);


    public void onSingleTapUp(int x, int y);

    /**
     * Get whether auto focus is supported.
     *
     * @return True if auto focus is supported ,otherwise,false.
     */
    public boolean getFocusAreaSupported();

    /**
     * Override the focus mode.
     * @param focusMode The focus mode you want to override.
     */
    public void overrideFocusMode(String focusMode);

    /**
     * Clear focus and face UI when neccessary.
     */
    public void clearFocusAndFaceUi();

    /**
     * Set the focus distance information.
     * @param info The value of focus disctance.
     */
    public void setDistanceInfo(String info);

    /**
     * Get the value of focus distance.
     * @return The distance.
     */
    public String getDistanceInfo();

    /**
     * Cancel auto focus when neccessary.
     */
    public void cancelAutoFocus();
}
