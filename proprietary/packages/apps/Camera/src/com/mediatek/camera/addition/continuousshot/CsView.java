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
package com.mediatek.camera.addition.continuousshot;

import android.app.Activity;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.RelativeSizeSpan;
import android.text.style.StyleSpan;
import android.view.View;
import android.widget.TextView;

import com.android.camera.R;
import com.mediatek.camera.ui.CameraView;
import com.mediatek.camera.util.Log;

public class CsView extends CameraView {
    private static final String TAG = "CsView";
    // because the CS number is 01~99,
    // so the just need two number [two digit]
    private static final int BEGINING_LOCATION = 0;
    private static final int END_LOCATION = 2;
    private static final float FRONT_SIZE_OF_CS_NUMBER = 1.7f;

    private TextView mCsInfoView;
    private SpannableString mSpannableString;

    public CsView(Activity activity) {
        super(activity);
        Log.i(TAG, "[CsView]constructor...");
    }

    @Override
    public boolean update(int type, Object... args) {
        String text = (String) args[0];
        Log.d(TAG, "[update]text = " + text);
        mSpannableString = new SpannableString(text);
        show();
        return true;
    }

    @Override
    public void refresh() {
        Log.i(TAG, "[refresh]...");
        if (mCsInfoView != null) {
            mSpannableString.setSpan(new RelativeSizeSpan(FRONT_SIZE_OF_CS_NUMBER),
                    BEGINING_LOCATION, END_LOCATION, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            mSpannableString.setSpan(new StyleSpan(android.graphics.Typeface.BOLD),
                    BEGINING_LOCATION, END_LOCATION, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            mCsInfoView.setText(mSpannableString);
            mCsInfoView.setVisibility(View.VISIBLE);
        }
    }

    @Override
    protected View getView() {
        View view = inflate(R.layout.onscreen_cs_speed);
        mCsInfoView = (TextView) view.findViewById(R.id.cs_info_view);
        return view;
    }
}
