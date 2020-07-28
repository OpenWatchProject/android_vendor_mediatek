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
package com.mediatek.camera.feature.mode.slowmotion;

import android.app.Activity;
import android.content.Context;
import android.content.res.Resources;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.CamcorderProfile;
import android.os.Build;
import android.util.Range;
import android.util.Size;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory.CameraApi;
import com.mediatek.camera.common.loader.DeviceDescription;
import com.mediatek.camera.common.loader.FeatureEntryBase;
import com.mediatek.camera.common.mode.ICameraMode;
import com.mediatek.camera.feature.setting.slowmotionquality.SlowMotionQualityHelper;
import com.mediatek.camera.portability.CamcorderProfileEx;
import com.mediatek.camera.portability.SystemProperties;

import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

/**
 * slow motion mode entry.
 */
public class SlowMotionEntry extends FeatureEntryBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(SlowMotionEntry.class.getSimpleName());

    private static final String MODE_ITEM_TYPE = "Video";
    private static final int MODE_ITEM_PRIORITY = 70;

    private List<String> mStringSupportedIds;
    private final Context mContext;

    /**
     * create an entry.
     *
     * @param context   current activity.
     * @param resources current resources.
     */
    public SlowMotionEntry(Context context, Resources resources) {
        super(context, resources);
        mContext = context;
    }


    @Override
    public boolean isSupport(CameraApi currentCameraApi, Activity activity) {
        boolean isSupported = !isThirdPartyIntent(activity)
                && isFeatureOptionSupported()
                && isPlatFormSupported();
        LogHelper.i(TAG, "[isSupport] isSupported = " + isSupported);
        return isSupported;
    }

    @Override
    public String getFeatureEntryName() {
        return SlowMotionEntry.class.getName();
    }

    @Override
    public Class getType() {
        return ICameraMode.class;
    }

    @Override
    public Object createInstance() {
        return new SlowMotionMode();
    }

    @Override
    public IAppUi.ModeItem getModeItem() {
        IAppUi.ModeItem modeItem = new IAppUi.ModeItem();
        modeItem.mModeUnselectedIcon = mResources.getDrawable(R.drawable.ic_slow_motion_mode_off);
        modeItem.mModeSelectedIcon = mResources.getDrawable(R.drawable.ic_slow_motion_mode_on);
        modeItem.mShutterIcon = mResources.getDrawable(R.drawable.ic_slow_motion_shutter);
        modeItem.mType = MODE_ITEM_TYPE;
        modeItem.mPriority = MODE_ITEM_PRIORITY;
        modeItem.mClassName = getFeatureEntryName();
        modeItem.mModeName = String.valueOf(mResources.getString(R.string.slow_motion_title));
        modeItem.mSupportedCameraIds = new String[mStringSupportedIds.size()];
        for (int i = 0; i < mStringSupportedIds.size(); i++) {
            modeItem.mSupportedCameraIds[i] = mStringSupportedIds.get(i);
        }
        return modeItem;
    }

    private boolean isFeatureOptionSupported() {
        boolean enable =
                SystemProperties.getInt("ro.vendor.mtk_slow_motion_support", 0) == 1 ? true : false;
        LogHelper.i(TAG, "[isFeatureOptionSupported] slow motion enable = " + enable);
        return enable;
    }

    private boolean isPlatFormSupported() {
        ConcurrentHashMap<String, DeviceDescription> deviceDescriptionHashMap
                = mDeviceSpec.getDeviceDescriptionMap();
        int cameraNum = deviceDescriptionHashMap.size();
        DeviceDescription deviceDescription;
        CameraCharacteristics cameraCharacteristics;
        mStringSupportedIds = new ArrayList<String>();
        try {
            CameraManager cameraManager =
                    (CameraManager) mContext.getSystemService(mContext.CAMERA_SERVICE);
            String[] idList = cameraManager.getCameraIdList();
            if (idList == null || idList.length == 0) {
                LogHelper.e(TAG, "<isPlatFormSupported> Camera num is 0," +
                        "Sensor should double check");
                return false;
            }
            for (String id : idList) {
                deviceDescription = deviceDescriptionHashMap.get(id);
                cameraCharacteristics = deviceDescription.getCameraCharacteristics();
                if (cameraCharacteristics == null) {
                    continue;
                }
                if (SlowMotionQualityHelper.isSlowMotionSupported(Integer.parseInt(id),
                        cameraCharacteristics, deviceDescription)) {
                    mStringSupportedIds.add(id);
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        return mStringSupportedIds.size() != 0;
    }
}
