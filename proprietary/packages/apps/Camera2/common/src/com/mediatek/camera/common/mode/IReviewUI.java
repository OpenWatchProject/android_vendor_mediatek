/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.common.mode;

import android.graphics.Bitmap;
import android.view.View;
import javax.annotation.Nonnull;
/**
 * This interface used for 3th use camera to capture or recording.
 * after capture show the review.
 */

public interface IReviewUI {
    /**
     * use spec to config review ui.
     * @param spec config review ui
     * if listener is null then should not show the corresponding icon.
     */
    void initReviewUI(@Nonnull ReviewSpec spec);

    /**
     * show review after capture/recording.
     * @param bitmap the review picture.
     */
    void showReviewUI(@Nonnull Bitmap bitmap);

    /**
     * hide review.
     */
    void hideReviewUI();
    /**
     * update orientation to review ui,maybe use to rotate.
     * @param orientation the orientation want to set,the value only supported
     * 0,90,180,270 and other value will ignore.
     */
    void updateOrientation(int orientation);
    /**
     * this class used to config review ui.
     */
    static class ReviewSpec {
        /**
         * this is used for monitor ui's retake button click.
         * and notify those who want know it.
         * this button will if isIntentVideo is true and after stop recording.
         */
        public View.OnClickListener retakeListener = null;
        /**
         * this is used for monitor ui's play button click.
         * and notify those who want know it.
         * this button will if isIntentVideo is true and after stop recording.
         */
        public View.OnClickListener playListener = null;
        /**
         * this is used for monitor ui's save button click.
         * and notify those who want know it.
         * this button will if isIntentVideo is true and after stop recording.
         */
        public View.OnClickListener saveListener = null;
    }
}
