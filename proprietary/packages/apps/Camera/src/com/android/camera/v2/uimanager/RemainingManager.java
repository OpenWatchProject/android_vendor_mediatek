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
package com.android.camera.v2.uimanager;

import android.app.Activity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.android.camera.R;
import com.android.camera.v2.app.AppController;
import com.android.camera.v2.uimanager.preference.PreferenceManager;
import com.mediatek.camera.v2.services.storage.IStorageService;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.util.Locale;

public class RemainingManager extends AbstractUiManager {
    private static final Tag TAG = new Tag(RemainingManager.class.getSimpleName());
    private static final Long                    REMAIND_THRESHOLD = 100L;
    private String                               mRemainingText;
    private PreferenceManager                    mPreferenceManager;
    private TextView                             mRemainingView;
    private IStorageService mStorageService;

    /**
    * the constructor of RemainingManager.
    * @param appcontroller use to get service
    * @param activity Which activity use it
    * @param parent The parent view to show Remaining
    * @param preferenceManager Refer to PreferenceManager
    */
    public RemainingManager(AppController appcontroller, Activity activity, ViewGroup parent,
            PreferenceManager preferenceManager) {
        super(activity, parent);
        mPreferenceManager = preferenceManager;
        mStorageService = appcontroller.getAppControllerAdapter()
                .getServices().getStorageService();
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.remaining_v2);
        mRemainingView = (TextView) view.findViewById(R.id.remaining_view);
        return view;
    }

    @Override
    protected void onRefresh() {
        super.onRefresh();
        if (mRemainingView != null) {
            mRemainingView.setText(mRemainingText);
        }
    }

    /**
    * Show left image counts that can be taken.
    * @param bytePerCount The bytes one image occupy.
    * @param showAlways If this is true, always show left counts of image can be taken.
    *  If this is false,only show left counts when counts less than 100L.
    */
    public void showLeftCounts(int bytePerCount, boolean showAlways) {
        long leftSpace = mStorageService.getCaptureStorageSpace();
        long leftCounts = 0L;
        if (leftSpace > 0) {
            leftCounts = leftSpace / bytePerCount;
        } else {
            leftCounts = 0;
        }
        // if showAlways is false and left counts is more then 100L, do not need
        // show remaining view.
        if (!showAlways && leftCounts > REMAIND_THRESHOLD) {
            return;
        }
        mRemainingText = (leftCounts < 0) ? stringForCount(0) : stringForCount(leftCounts);
        super.show();
        LogHelper.i(TAG, "[showLeftCounts], leftCounts:" + leftCounts +
                ", mRemainingText:" + mRemainingText);
    }
    /**
    * Show left times can be recorded.
    * @param bytePerMs
    * The bytes of one millisecond recording
    */
    public void showLeftTime(long bytePerMs) {
        long leftSpace = mStorageService.getRecordStorageSpace();
        long leftTime = 0L;
        if (leftSpace > 0) {
            leftTime = leftSpace / bytePerMs;
        } else {
            leftTime = 0;
        }
        mRemainingText = (leftTime < 0) ? stringForTime(0) : stringForTime(leftTime);
        super.show();
        LogHelper.i(TAG, "[showLeftTime], leftTime:"
                + leftTime + ", mRemainingText:" + mRemainingText);
    }

    private static String stringForTime(final long millis) {
        final int totalSeconds = (int) millis / 1000;
        final int seconds = totalSeconds % 60;
        final int minutes = (totalSeconds / 60) % 60;
        final int hours = totalSeconds / 3600;
        if (hours > 0) {
            return String.format(Locale.ENGLISH, "%d:%02d:%02d", hours, minutes, seconds);
        } else {
            return String.format(Locale.ENGLISH, "%02d:%02d", minutes, seconds);
        }
    }

    private static String stringForCount(final long count) {
        return String.format("%d", count);
    }
}
