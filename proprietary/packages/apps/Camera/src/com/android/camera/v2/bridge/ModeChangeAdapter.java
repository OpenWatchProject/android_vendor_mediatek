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

package com.android.camera.v2.bridge;

import java.util.HashMap;
import java.util.Map;

import com.android.camera.v2.util.SettingKeys;

import com.mediatek.camera.v2.platform.ModeChangeListener;

/**
 *  This Adapter is used to adapt mode change.
 */
public class ModeChangeAdapter {
    private final ModeChangeListener mModeChangeListener;
    private static Map<String, Integer> sModeMapping = new HashMap<String, Integer>();
    static {
        sModeMapping.put(SettingKeys.KEY_NORMAL, ModeChangeListener.MODE_CAPTURE);
        sModeMapping.put(SettingKeys.KEY_HDR, ModeChangeListener.MODE_HDR);
        sModeMapping.put(SettingKeys.KEY_PHOTO_PIP, ModeChangeListener.MODE_PIP);
    }
    public ModeChangeAdapter(ModeChangeListener modeChangeListener) {
        mModeChangeListener = modeChangeListener;
    }

    public void onModeChanged(String newMode) {
        mModeChangeListener.onModeSelected(sModeMapping.get(newMode));
    }

    public static int getModeIndexFromKey(String modeKey) {
        return sModeMapping.get(modeKey);
    }

    /**
     * Check change mode whether need change module or not.
     * @param oldeMode
     *            which mode will leave.
     * @param newMode
     *            which mode will create.
     * @return true means change mode need change module first. false means not
     *         need change module.
     */
    public static boolean isNeedSwitchModule(String oldeMode, String newMode) {
        if (SettingKeys.KEY_PHOTO_PIP.equals(oldeMode)
                || SettingKeys.KEY_PHOTO_PIP.equals(newMode)) {
            return true;
        }
        return false;
    }

    /**
     *Get the module index according current mode.
     * @param currentMode current mode value,such normal mode.
     * @return the index of module.
     */
    public static int getModuleIndex(String currentMode) {
        int index = ModuleControllerAdapter.CAMERA_MODULE_INDEX;
        if (SettingKeys.KEY_PHOTO_PIP.equals(currentMode)) {
            index = ModuleControllerAdapter.DUAL_CAMERA_MODULE_INDEX;
        }
        return index;
    }
}
