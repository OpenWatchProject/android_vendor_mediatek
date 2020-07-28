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
package com.android.camera;

import java.util.ArrayList;
import java.util.List;

public class Restriction {
    private static final String TAG = "Restriction";

    public static final int TYPE_SETTING = 0;
    public static final int TYPE_MODE = 1;
    private final int mSettingIndex;
    private boolean mEnable = true;
    private List<String> mValues;
    private List<Restriction> mRestrictions;
    private int mType = TYPE_SETTING;

    public Restriction(int index) {
        mSettingIndex = index;
    }

    public int getType() {
        return mType;
    }

    public int getIndex() {
        return mSettingIndex;
    }

    public boolean getEnable() {
        return mEnable;
    }

    public List<String> getValues() {
        return mValues;
    }

    public List<Restriction> getRestrictioins() {
        return mRestrictions;
    }

    public Restriction setEnable(boolean enable) {
        mEnable = enable;
        return this;
    }

    public Restriction setType(int type) {
        mType = type;
        return this;
    }

    public Restriction setValues(final String... values) {
        if (values != null) {
            mValues = new ArrayList<String>();
            for (String value : values) {
                mValues.add(value);
            }
        }
        return this;
    }

    public Restriction setRestrictions(final Restriction... restrictions) {
        if (restrictions != null) {
            mRestrictions = new ArrayList<Restriction>();
            for (Restriction value : restrictions) {
                mRestrictions.add(value);
            }
        }
        return this;
    }

    private MappingFinder mMappingFinder;

    public String findSupported(String value) {
        String supported = value;
        if (mMappingFinder != null) {
            supported = mMappingFinder.find(value, mValues);
        }
        if (mValues != null && !mValues.contains(supported)) {
            supported = mValues.get(0);
        }
        Log.d(TAG, "findSupported(" + value + ") return " + supported);
        return supported;
    }

    public Restriction setMappingFinder(MappingFinder finder) {
        mMappingFinder = finder;
        return this;
    }

    public interface MappingFinder {
        String find(String current, List<String> supportedList);

        int findIndex(String current, List<String> supportedList);
    }
}
