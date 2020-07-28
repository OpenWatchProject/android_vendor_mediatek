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
import android.util.AttributeSet;

import java.util.ArrayList;

/**
 * A collection of <code>CameraPreference</code>s. It may contain other
 * <code>PreferenceGroup</code> and form a tree structure.
 */
public class PreferenceGroup extends CameraPreference {
    private ArrayList<CameraPreference> list = new ArrayList<CameraPreference>();

    public PreferenceGroup(Context context, AttributeSet attrs,
            SharedPreferencesTransfer prefTransfer) {
        super(context, attrs, prefTransfer);
    }

    @Override
    public void reloadValue() {
        for (CameraPreference pref : list) {
            pref.reloadValue();
        }
    }

    public void addChild(CameraPreference child) {
        list.add(child);
    }

    public void removePreference(int index) {
        list.remove(index);
    }

    public CameraPreference get(int index) {
        return list.get(index);
    }

    public int size() {
        return list.size();
    }

    /**
     * Finds the preference with the given key recursively. Returns
     * <code>null</code> if cannot find.
     */
    public ListPreference findPreference(String key) {
        // Find a leaf preference with the given key. Currently, the base
        // type of all "leaf" preference is "ListPreference". If we add some
        // other types later, we need to change the code.
        for (CameraPreference pref : list) {
            if (pref instanceof ListPreference) {
                ListPreference listPref = (ListPreference) pref;
                if (listPref.getKey().equals(key))
                    return listPref;
            } else if (pref instanceof PreferenceGroup) {
                ListPreference listPref = ((PreferenceGroup) pref).findPreference(key);
                if (listPref != null)
                    return listPref;
            }
        }
        return null;
    }
}
