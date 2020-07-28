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
package com.mediatek.camera.setting;

import com.mediatek.camera.setting.preference.ListPreference;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;

public class SettingItem {
    private int mSettingId;
    private int mType;
    private boolean mEnable = true;
    private String mKey = null;
    private String mLastValue = null;
    private String mValue = null;
    private String mDefaultValue = null;

    private int mOverrideCount = 0;
    private ListPreference mListPeference = null;

    private HashMap<String, Record> mOverrideRecord = new HashMap<String, Record>();

    public SettingItem(int settingId) {
        this.mSettingId = settingId;
    }

    /**
     * Get the setting id.
     *
     * @return setting id.
     */
    public int getSettingId() {
        return this.mSettingId;
    }

    /**
     * Set the setting key value.
     *
     * @param key
     *            the setting key value.
     */
    public void setKey(String key) {
        this.mKey = key;
    }

    /**
     * Get the setting key value.
     *
     * @return return setting key value.
     */
    public String getKey() {
        return this.mKey;
    }

    /**
     * Set the setting value.
     *
     * @param value
     *            the setting value.
     */
    public void setValue(String value) {
        this.mValue = value;
    }

    /**
     * Get the setting value.
     *
     * @return the setting value.
     */
    public String getValue() {
        return this.mValue;
    }

    /**
     * Set the last value of setting
     * @param lastValue the last value of setting
     */
    public void setLastValue(String lastValue) {
        this.mLastValue = lastValue;
    }

    /**
     * Get the last value of setting
     * @return the last value of setting
     */
    public String getLastValue() {
        return this.mLastValue;
    }

    /**
     * Set the count of override
     * @param count the count of override
     */
    public void setOverrideCount(int count) {
        this.mOverrideCount = count;
    }

    /**
     * Get the count of override
     * @return the count of override
     */
    public int getOverrideCount() {
        return mOverrideRecord.size();
    }

    public void addOverrideRecord(String key, Record record) {
        mOverrideRecord.put(key, record);
    }

    public void removeOverrideRecord(String key) {
        mOverrideRecord.remove(key);
    }

    public Record getOverrideRecord(String key) {
        Record record = mOverrideRecord.get(key);
        return record;
    }

    public Record getTopOverrideRecord() {
        Set<String> sets = mOverrideRecord.keySet();
        Iterator<String> iterator = sets.iterator();
        String key = null;
        while (iterator.hasNext()) {
            key = iterator.next();
            break;
        }
        if (key != null) {
            return mOverrideRecord.get(key);
        } else {
            return null;
        }
    }

    public void clearAllOverrideRecord() {
        if (mOverrideRecord != null) {
            mOverrideRecord.clear();
        }
    }
    /**
     * Set the setting default value.
     *
     * @param defaultValue
     *            the setting default value
     */
    public void setDefaultValue(String defaultValue) {
        this.mDefaultValue = defaultValue;
    }

    /**
     * Get the setting default value.
     *
     * @return the setting default value.
     */
    public String getDefaultValue() {
        return this.mDefaultValue;
    }

    /**
     * Set the setting listPeference.
     *
     * @param preference
     *            the setting listPeference.
     */
    public void setListPreference(ListPreference preference) {
        this.mListPeference = preference;
    }

    /**
     * Get the setting listPreference.
     *
     * @return the setting listPreference.
     */
    public ListPreference getListPreference() {
        return this.mListPeference;
    }

    /**
     * Set the setting type.
     *
     * @param type
     *            the setting type.
     */
    public void setType(int type) {
        this.mType = type;
    }

    /**
     * Get the setting type.
     *
     * @return the setting type.
     */
    public int getType() {
        return this.mType;
    }

    /**
     * Set the setting enable
     *
     * @param enable
     *            true or false.
     */
    public void setEnable(boolean enable) {
        this.mEnable = enable;
    }

    /**
     * Get the setting enable state.
     *
     * @return true or false.
     */
    public boolean isEnable() {
        return this.mEnable;
    }

    @Override
    public String toString() {
        return new StringBuilder()
            .append("SettingItem(mKey=").append(mKey)
            .append(", settingId=").append(mSettingId)
            .append(", mType=").append(mType)
            .append(", mValue=").append(mValue)
            .append(", mEnable=").append(mEnable)
            .append(", mListPeference=").append(mListPeference).append(")")
                .toString();
    }

    public class Record {
        private String mValue;
        private String mOverrideValue;

        public Record(String value, String overrideValue) {
            mValue = value;
            mOverrideValue = overrideValue;
        }

        public String getValue() {
            return this.mValue;
        }

        public String getOverrideValue() {
            return this.mOverrideValue;
        }
    }
}
