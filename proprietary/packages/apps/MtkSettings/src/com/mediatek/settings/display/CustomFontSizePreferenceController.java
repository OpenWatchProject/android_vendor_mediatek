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
package com.mediatek.settings.display;

import android.content.Context;
import android.content.res.Resources;
import android.provider.Settings;
import android.support.v7.preference.Preference;

import com.android.settings.R;
import com.mediatek.settings.accessibility.CustomToggleFontSizePreferenceFragment;
import com.android.settings.core.BasePreferenceController;

import com.mediatek.settings.ext.IDisplaySettingsExt;
import com.mediatek.settings.UtilsExt;

public class CustomFontSizePreferenceController extends BasePreferenceController {

    private static final String KEY_CUSTOM_FONT_SIZE = "custom_font_size";
    private IDisplaySettingsExt customFontSizePref;
    private Context mContext;

    public CustomFontSizePreferenceController(Context context, String key) {
        super(context, key);
        mContext = context;
    }

    @Override
    public int getAvailabilityStatus() {
        /* ToDo: Only available for OP18 */
        customFontSizePref = UtilsExt.getDisplaySettingsExt(mContext);
        if(customFontSizePref.isCustomPrefPresent()) {
            return AVAILABLE;
        }
        return UNSUPPORTED_ON_DEVICE;
    }

    @Override
    public CharSequence getSummary() {
        final float currentScale = Settings.System.getFloat(mContext.getContentResolver(),
                Settings.System.FONT_SCALE, 1.0f);
        final Resources res = mContext.getResources();
        final String[] entries = res.getStringArray(R.array.custom_entries_font_size);
        final String[] strEntryValues = res.getStringArray(R.array.custom_entryvalues_font_size);
        final int index = CustomToggleFontSizePreferenceFragment.fontSizeValueToIndex(currentScale,
                strEntryValues);
        return entries[index];
    }
}
