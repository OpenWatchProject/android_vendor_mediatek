/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 */

package com.mediatek.settings.accessibility;

import android.annotation.Nullable;
import android.content.ContentResolver;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Bundle;
import android.provider.Settings;

import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.mediatek.settings.CustomPreviewSeekBarPreferenceFragment;
import com.android.settings.R;

/**
 * Preference fragment used to control font size.
 */
public class CustomToggleFontSizePreferenceFragment extends CustomPreviewSeekBarPreferenceFragment {

    private float[] mValues;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mActivityLayoutResId = R.layout.font_size_activity;
        mPreviewSampleResIds = new int[]{R.layout.font_size_preview};

        Resources res = getContext().getResources();
        final ContentResolver resolver = getContext().getContentResolver();
        // Mark the appropriate item in the preferences list.
        mEntries = res.getStringArray(R.array.custom_entries_font_size);
        final String[] strEntryValues = res.getStringArray(R.array.custom_entryvalues_font_size);
        final float currentScale =
                Settings.System.getFloat(resolver, Settings.System.FONT_SCALE, 1.0f);
        mInitialIndex = fontSizeValueToIndex(currentScale, strEntryValues);
        mValues = new float[strEntryValues.length];
        for (int i = 0; i < strEntryValues.length; ++i) {
            mValues[i] = Float.parseFloat(strEntryValues[i]);
        }
        getActivity().setTitle(R.string.title_font_size);
    }

    @Override
    protected Configuration createConfig(Configuration origConfig, int index) {
        // Populate the sample layouts.
        final Configuration config = new Configuration(origConfig);
        config.fontScale = mValues[index];
        return config;
    }

    /**
     * Persists the selected font size.
     */
    @Override
    protected void commit() {
        if (getContext() == null) return;
        final ContentResolver resolver = getContext().getContentResolver();
        Settings.System.putFloat(resolver, Settings.System.FONT_SCALE, mValues[mCurrentIndex]);
    }

    @Override
    public int getHelpResource() {
        return R.string.help_url_font_size;
    }

    @Override
    public int getMetricsCategory() {
        return MetricsEvent.ACCESSIBILITY_FONT_SIZE;
    }

    /**
     *  Utility function that returns the index in a string array with
     *  which the represented value is
     *  the closest to a given float value.
     */
    public static int fontSizeValueToIndex(float val, String[] indices) {
        float lastVal = Float.parseFloat(indices[0]);
        for (int i = 1; i < indices.length; i++) {
            float thisVal = Float.parseFloat(indices[i]);
            if (val < (lastVal + (thisVal - lastVal) * .5f)) {
                return i - 1;
            }
            lastVal = thisVal;
        }
        return indices.length - 1;
    }

}
