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
package com.mediatek.camera.v2.mode.pip.combination;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.setting.ISettingRule;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.setting.SettingItem;
import com.mediatek.camera.v2.setting.SettingItem.Record;
import com.mediatek.camera.v2.stream.pip.pipwrapping.PIPCustomization;
import com.mediatek.camera.v2.util.SettingKeys;

import java.util.List;

public class FdAffectedRule implements ISettingRule {
    private static final Tag TAG = new Tag(FdAffectedRule.class.getSimpleName());
    private final SettingCtrl        mSettingCtrl;
    private ISettingServant          mSettingServant;
    private String                   mCurrentCameraId = null;
    private String                   mResultKey;

    public FdAffectedRule(SettingCtrl settingCtrl, String key) {
        mSettingCtrl = settingCtrl;
        mResultKey = key;
        mCurrentCameraId = mSettingCtrl.getCurrentCameraId();
        mSettingServant = mSettingCtrl.getSettingServant(mSettingCtrl.getCurrentCameraId());
    }

    @Override
    public void execute() {
        String cameraId = mSettingCtrl.getCurrentCameraId();
        boolean isPipSwitched = mCurrentCameraId != cameraId;
        if (!mCurrentCameraId.equals(cameraId)) {
            mSettingServant = mSettingCtrl.getSettingServant(cameraId);
            mCurrentCameraId = cameraId;
        }
        String pipKeyValue = mSettingServant.getSettingValue(SettingKeys.KEY_PHOTO_PIP);
        if ("on".equalsIgnoreCase(pipKeyValue)) {
            ISettingServant fdOffSettingServant = null;
            fdOffSettingServant = mSettingCtrl.getSettingServant(SettingCtrl.FRONT_CAMERA);
            closeFdSetting(fdOffSettingServant);
            fdOffSettingServant = mSettingCtrl.getSettingServant(SettingCtrl.BACK_CAMERA);
            closeFdSetting(fdOffSettingServant);
         LogHelper.d(TAG, "Enter pip, " + mResultKey + " must off camera id:" +
                    "" + fdOffSettingServant.getCameraId() + " isPipSwitched:" + isPipSwitched);
        } else if ("off".equalsIgnoreCase(pipKeyValue)) {
            ISettingServant fdOffSettingServant = null;
            fdOffSettingServant = mSettingCtrl.getSettingServant(SettingCtrl.FRONT_CAMERA);
            restoreFdSetting(fdOffSettingServant);
            fdOffSettingServant = mSettingCtrl.getSettingServant(SettingCtrl.BACK_CAMERA);
            restoreFdSetting(fdOffSettingServant);
        }
    }

    @Override
    public void addLimitation(String condition, List<String> result) {
    }

    private void closeFdSetting(ISettingServant fdOffSettingServant) {
        SettingItem fdOffSettingItem = fdOffSettingServant.getSettingItem(mResultKey);
        fdOffSettingItem.setValue("off");
        fdOffSettingItem.setOverrideValue("off");
        Record record = fdOffSettingItem.new Record("off", "off");
        fdOffSettingItem.addOverrideRecord(SettingKeys.KEY_PHOTO_PIP, record);
    }

    private void restoreFdSetting(ISettingServant fdOffSettingServant) {
        SettingItem fdOffSettingItem = fdOffSettingServant.getSettingItem(mResultKey);

        Record record = fdOffSettingItem.getOverrideRecord(SettingKeys.KEY_PHOTO_PIP);
        if (record == null) {
            LogHelper.i(TAG, "[execute], no override record, return");
            return;
        }
        fdOffSettingItem.removeOverrideRecord(SettingKeys.KEY_PHOTO_PIP);

        String value = null;
        String overrideValue = null;
        int count = fdOffSettingItem.getOverrideCount();
        if (count > 0) {
            Record topRecord = fdOffSettingItem.getTopOverrideRecord();
            if (topRecord != null) {
                value = topRecord.getValue();
                overrideValue = topRecord.getOverrideValue();
            }
        } else {
            value = fdOffSettingServant.getSharedPreferencesValue(mResultKey);
        }
        fdOffSettingItem.setValue(value);
        fdOffSettingItem.setOverrideValue(overrideValue);
    }
}
