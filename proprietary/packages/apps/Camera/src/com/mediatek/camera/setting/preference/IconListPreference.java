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

package com.mediatek.camera.setting.preference;

import android.content.Context;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.util.AttributeSet;

import com.android.camera.R;

import java.util.ArrayList;
import java.util.List;

/** A {@code ListPreference} where each entry has a corresponding icon. */
public class IconListPreference extends ListPreference {
    private int mIconIds[];
    private int mOriginalSupportedIconIds[];
    private int mOriginalIconIds[];

    public IconListPreference(Context context, AttributeSet attrs,
            SharedPreferencesTransfer prefTransfer) {
        super(context, attrs, prefTransfer);
        TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.IconListPreference, 0, 0);
        Resources res = context.getResources();
        mIconIds = getIds(res, a.getResourceId(R.styleable.IconListPreference_icons, 0));
        a.recycle();
        //TODO:
        // Here we remember original values for strange scene mode:
        // backlight and backlight-portrait which were implemented by a strange
        // way.
        // I have sent email to native FPM to correct this logic.
        // Anyway, we keep this logic until they do that.

        // / M: here remember initial values
        mOriginalIconIds = mIconIds;
    }

    @Override
    public void filterUnsupported(List<String> supported) {
        CharSequence originalEntryValues[] = getOriginalEntryValues();
        IntArray iconIds = new IntArray();

        for (int i = 0, len = originalEntryValues.length; i < len; i++) {
            if (supported.indexOf(originalEntryValues[i].toString()) >= 0) {
                if (mIconIds != null) {
                    iconIds.add(mIconIds[i]);
                }
            }
        }
        if (mIconIds != null) {
            mIconIds = iconIds.toArray(new int[iconIds.size()]);
            mOriginalSupportedIconIds = mIconIds; // remember all supported
                                                  // values.
        }
        super.filterUnsupported(supported);
    }

    @Override
    public void filterDisabled(List<String> supported) {
        CharSequence originalSupportedEntryValues[] = getOriginalSupportedEntryValues();
        IntArray iconIds = new IntArray();
        for (int i = 0, len = originalSupportedEntryValues.length; i < len; i++) {
            if (supported.indexOf(originalSupportedEntryValues[i].toString()) >= 0) {
                if (mOriginalSupportedIconIds != null) {
                    iconIds.add(mOriginalSupportedIconIds[i]);
                }
            }
        }
        if (mIconIds != null) {
            mIconIds = iconIds.toArray(new int[iconIds.size()]);
        }
        super.filterDisabled(supported);
    }

    @Override
    public void setOriginalEntryValues(CharSequence values[]) {
        CharSequence originalEntries[] = getOriginalEntries();
        CharSequence originalEntryValues[] = getOriginalEntryValues();

        IntArray iconIds = new IntArray();
        ArrayList<CharSequence> originalEntriesList = new ArrayList<CharSequence>();
        ArrayList<CharSequence> originalEntryValuesList = new ArrayList<CharSequence>();

        for (int i = 0, len = originalEntryValues.length; i < len; i++) {
            for (int j = 0; j < values.length; j++) {
                if (values[j].equals(originalEntryValues[i])) {
                    originalEntriesList.add(originalEntries[i]);
                    originalEntryValuesList.add(originalEntryValues[i]);
                    if (mIconIds != null) {
                        iconIds.add(mIconIds[i]);
                    }
                    break;
                }
            }
        }
        if (mIconIds != null) {
            mIconIds = iconIds.toArray(new int[iconIds.size()]);
            mOriginalIconIds = mIconIds;
        }
        int length = originalEntryValuesList.size();
        super.setOriginalEntries(originalEntriesList.toArray(new CharSequence[length]));
        super.setOriginalEntryValues(originalEntryValuesList.toArray(new CharSequence[length]));
    }

    @Override
    public void restoreSupported() {
        super.restoreSupported();
        if (mOriginalSupportedIconIds != null) {
            mIconIds = mOriginalSupportedIconIds;
        }
    }

    @Override
    public int getIconId(int index) {
        if (mIconIds == null || index < 0 || index >= mIconIds.length) {
            return super.getIconId(index);
        }
        return mIconIds[index];
    }

    public int[] getOriginalSupportedIconIds() {
        return mOriginalSupportedIconIds;
    }

    public int[] getOriginalIconIds() {
        return mOriginalIconIds;
    }

    public int[] getIconIds() {
        return mIconIds;
    }

    public void setIconIds(int[] iconIds) {
        mIconIds = iconIds;
    }

    private int[] getIds(Resources res, int iconsRes) {
        if (iconsRes == 0) {
            return null;
        }
        TypedArray array = res.obtainTypedArray(iconsRes);
        int n = array.length();
        int ids[] = new int[n];
        for (int i = 0; i < n; ++i) {
            ids[i] = array.getResourceId(i, 0);
        }
        array.recycle();
        return ids;
    }
}
