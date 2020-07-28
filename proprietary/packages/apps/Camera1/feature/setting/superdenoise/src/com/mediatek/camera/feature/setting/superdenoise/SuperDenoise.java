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

package com.mediatek.camera.feature.setting.superdenoise;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.ArrayList;
import java.util.List;

/**
 * This class is for super denoise setting.
 */
public class SuperDenoise extends SettingBase {
    private static final Tag TAG = new Tag(SuperDenoise.class.getSimpleName());
    public static final String KEY_SUPER_DENOISE = "key_super_denoise";
    private static final String DENOISE_ON = "on";
    private static final String DENOISE_OFF = "off";
    private List<String> mSupportValues = new ArrayList<>();

    @Override
    public void init(IApp app, ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        initSettingValue();
    }

    @Override
    public void unInit() {
    }

    @Override
    public void postRestrictionAfterInitialized() {
        Relation relation = SuperDenoiseRestriction.getRestriction().getRelation(getValue(), false);
        LogHelper.d(TAG, "[postRestrictionAfterInitialized] value = " + getValue());
        if (relation != null) {
            mSettingController.postRestriction(relation);
        }
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO;
    }

    @Override
    public String getKey() {
        return KEY_SUPER_DENOISE;
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        return new SuperDenoiseParameterConfig();
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        return null;
    }

    private void initSettingValue() {
        LogHelper.d(TAG, "[initSettingValue]");
        mSupportValues.add(DENOISE_OFF);
        mSupportValues.add(DENOISE_ON);
        setSupportedPlatformValues(mSupportValues);
        setSupportedEntryValues(mSupportValues);
        setEntryValues(mSupportValues);
        String value = mDataStore.getValue(getKey(), DENOISE_ON, getStoreScope());
        setValue(value);
    }
}
