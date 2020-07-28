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

package com.mediatek.camera.ui;

import android.app.Activity;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import com.android.camera.R;

import com.mediatek.camera.util.Log;

public class ProgressIndicator {
    private static final String TAG = "ProgressIndicator";

    private static int sIndicatorMarginLong = 0;
    private static int sIndicatorMarginShort = 0;
    private int mBlockPadding = 0;
    public int mBlockNumber = 9;

    public View mProgressView;
    public ImageView mProgressBars;
    /**
     * The constructor for ProgressIndicator.
     * @param activity
     *         the camera activity
     * @param blockNumber
     *         total block number
     * @param drawBlockSizes
     *         block array
     */
    public ProgressIndicator(Activity activity, int blockNumber,
            int[] drawBlockSizes) {
        mBlockPadding = 4;

        mProgressView = activity.findViewById(R.id.progress_indicator);
        mProgressView.setVisibility(View.VISIBLE);
        mProgressBars = (ImageView) activity.findViewById(R.id.progress_bars);
        mBlockNumber = blockNumber;
        int[] blockSizes = new int[blockNumber];
        System.arraycopy(drawBlockSizes, 0, blockSizes, 0, blockNumber);
        Resources res = activity.getResources();
        final float scale = res.getDisplayMetrics().density;
        if (scale != 1.0f) {
            mBlockPadding = (int) (mBlockPadding * scale + 0.5f);
            for (int i = 0; i < mBlockNumber; i++) {
                blockSizes[i] = (int) (drawBlockSizes[i] * scale + 0.5f);
            }
        }
        mProgressBars.setImageDrawable(new ProgressBarDrawable(activity,
                mProgressBars, blockSizes, mBlockPadding));
        getIndicatorMargin();
    }

    public ProgressIndicator(Activity activity) {
        mProgressView = activity.findViewById(R.id.progress_indicator);
        if (mProgressView == null) {
            Log.w(TAG, "mProgressView is null,return!");
            return;
        }
        mProgressView.setVisibility(View.VISIBLE);
        mProgressBars = (ImageView) activity.findViewById(R.id.progress_bars);
    }

    public void setVisibility(int visibility) {
        mProgressView.setVisibility(visibility);
    }

    public void setProgress(int progress) {
        mProgressBars.setImageLevel(progress);
    }

    private void getIndicatorMargin() {
        if (sIndicatorMarginLong == 0 && sIndicatorMarginShort == 0) {
            Resources res = mProgressView.getResources();
            sIndicatorMarginLong = res
                    .getDimensionPixelSize(R.dimen.progress_indicator_bottom_long);
            sIndicatorMarginShort = res
                    .getDimensionPixelSize(R.dimen.progress_indicator_bottom_short);
        }
        Log.d(TAG, "[getIndicatorMargin]sIndicatorMarginLong = "
                + sIndicatorMarginLong + " sIndicatorMarginShort = "
                + sIndicatorMarginShort);
    }

    public void setOrientation(int orientation) {
        LinearLayout progressViewLayout = (LinearLayout) mProgressView;
        RelativeLayout.LayoutParams rp = new RelativeLayout.LayoutParams(
                progressViewLayout.getLayoutParams());
        int activityOrientation = mProgressView.getResources()
                .getConfiguration().orientation;
        if ((Configuration.ORIENTATION_LANDSCAPE == activityOrientation
                && (orientation == 0 || orientation == 180))
                || (Configuration.ORIENTATION_PORTRAIT == activityOrientation
                && (orientation == 90 || orientation == 270))) {
            rp.setMargins(rp.leftMargin, rp.topMargin, rp.rightMargin,
                    sIndicatorMarginShort);
        } else {
            rp.setMargins(rp.leftMargin, rp.topMargin, rp.rightMargin,
                    sIndicatorMarginLong);
        }

        rp.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        rp.addRule(RelativeLayout.CENTER_HORIZONTAL);
        progressViewLayout.setLayoutParams(rp);
        progressViewLayout.requestLayout();
    }
}
