/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.feature.setting.imageproperties;

import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.setting.SettingBase;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nonnull;

/**
 * Image properties setting.
 */

public class ImageProperties extends SettingBase
        implements ImagePropertiesSettingView.OnValueChangeListener {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(ImageProperties.class.getSimpleName());

    private static final String KEY_IMAGE_PROPERTIES = "key_image_properties";
    private ImagePropertiesSettingView mSettingView;
    private List<ImagePropertiesItem> mItems = new ArrayList<>();
    List<ImagePropertiesSettingView.ItemInfo> mItemsInfo = new ArrayList<>();

    @Override
    public void init(IApp app, ICameraContext cameraContext,
                     ISettingManager.SettingController settingController) {
        super.init(app, cameraContext, settingController);
        for (ImagePropertiesItem item : mItems) {
            item.init(app, cameraContext, settingController);
            mSettingController.registerSettingItem(item);
        }
    }

    @Override
    public void setSettingDeviceRequester(
            @Nonnull ISettingManager.SettingDeviceRequester settingDeviceRequester,
            @Nonnull ISettingManager.SettingDevice2Requester settingDevice2Requester) {
        super.setSettingDeviceRequester(settingDeviceRequester, settingDevice2Requester);
        for (ImagePropertiesItem item : mItems) {
            item.setSettingDeviceRequester(mSettingDeviceRequester, mSettingDevice2Requester);
        }
    }

    @Override
    public void unInit() {
        for (ImagePropertiesItem item : mItems) {
            item.unInit();
            mSettingController.unRegisterSettingItem(item);
        }
    }

    @Override
    public void postRestrictionAfterInitialized() {

    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return KEY_IMAGE_PROPERTIES;
    }

    @Override
    public void addViewEntry() {
        if (mSettingView == null) {
            mSettingView = new ImagePropertiesSettingView(getKey(), mActivity);
            mSettingView.setOnValueChangeListener(this);
            for (ImagePropertiesItem item : mItems) {
                ImagePropertiesSettingView.ItemInfo itemInfo
                        = mSettingView.new ItemInfo();
                itemInfo.key = item.getKey();
                itemInfo.value = item.getValue();
                itemInfo.title = item.getTitle();
                mItemsInfo.add(itemInfo);
            }
            mSettingView.setItemsInfo(mItemsInfo);
        }
        mAppUi.addSettingView(mSettingView);
    }

    @Override
    public void removeViewEntry() {
        mAppUi.removeSettingView(mSettingView);
    }

    @Override
    public void refreshViewEntry() {
        boolean enabled = true;
        for (ImagePropertiesItem item : mItems) {
            if (item.getEntryValues().size() <= 1) {
                enabled = false;
                break;
            }
        }
        if (mSettingView != null) {
            mSettingView.setEnabled(enabled);
        }
        // Update image properties values for UI showing.
        for (int i = 0; i < mItemsInfo.size(); i++) {
            ImagePropertiesSettingView.ItemInfo itemInfo = mItemsInfo.get(i);
            itemInfo.value = mItems.get(i).getValue();
        }
    }

    @Override
    public IParametersConfigure getParametersConfigure() {
        return null;
    }

    @Override
    public ICaptureRequestConfigure getCaptureRequestConfigure() {
        return null;
    }

    @Override
    public void onValueChanged(String key, String value) {
        LogHelper.d(TAG, "[onValueChanged], key:" + key + ", value:" + value);
        for (ImagePropertiesItem item : mItems) {
            if (item.getKey().equals(key)) {
                item.onValueChanged(value);
                break;
            }
        }
    }

    /**
     * Add image properties child item.
     *
     * @param item The child item.
     */
    public void addItems(ImagePropertiesItem item) {
        mItems.add(item);
    }

}
