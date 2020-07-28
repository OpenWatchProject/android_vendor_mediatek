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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.camera.v2.detection.asd;

import android.app.Activity;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.platform.app.AppUi;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.SettingConvertor;
import com.mediatek.camera.v2.util.SettingKeys;

import java.util.List;

/**
 * The class acts as a role of a view(The auto scene view which can not be interacted with user).
 *
 */
public class AsdView implements IAsdView {
    private static final Tag TAG = new Tag(AsdView.class.getSimpleName());

    private final AppUi mAppUi;
    private final Activity mActivity;
    private ISettingServant mSettingServant;
    private String mDetectedScene;

/**
 *
 * @param activity The {@link android.content.Context} being used.
 * @param appUi The camera application UI.
 * @param settingServant The current setting instance.
 */
    public AsdView(Activity activity, AppUi appUi, ISettingServant settingServant) {
        mActivity = activity;
        mAppUi = appUi;
        mSettingServant = settingServant;
    }

    @Override
    public void updateAsdView(final int mode) {
        String appliedSceneMode = SettingConvertor.convertModeEnumToString(
                SettingKeys.KEY_SCENE_MODE, mode);
        mDetectedScene = appliedSceneMode;
        // update UI indicator
        mAppUi.updateAsdDetectedScene(mDetectedScene);
        if (appliedSceneMode != null) {
            List<String> supportedScene = mSettingServant
                    .getSupportedValues(SettingKeys.KEY_SCENE_MODE);
            // Force set the auto detected scene to auto when no scene is supported by
            // current camera or it is not include in the supported scenes.Particularly, hdr and
            // back-light-portrait scene will influence the further detection,forece set the scene
            // mode to auto when detecte the two scenes.
            boolean isHdrScene = appliedSceneMode.equalsIgnoreCase(SettingConvertor.SceneMode.HDR
                    .toString().toLowerCase());
            boolean isBacklightPortrait = appliedSceneMode
                    .equalsIgnoreCase(SettingConvertor.SceneMode.BACKLIGHT_PORTRAIT.toString()
                            .toLowerCase());
            if (supportedScene == null || !supportedScene.contains(appliedSceneMode) || isHdrScene
                    || isBacklightPortrait) {
                appliedSceneMode = SettingConvertor.SceneMode.AUTO.toString().toLowerCase();
            }
            LogHelper.d(TAG, "onAsdDetectedScene mode = " + mode + " ,and appliedSceneMode = "
                    + appliedSceneMode);
            // update scene mode
            mSettingServant.doSettingChange(SettingKeys.KEY_SCENE_MODE, appliedSceneMode, false);
        }
    }

    @Override
    public void hideAsdView() {
        // Do remove mViewUpdateRunnable before clear ASD indicator to avoid indicator update after
        // stop face detection.
        mAppUi.updateAsdDetectedScene(null);
    }
}
