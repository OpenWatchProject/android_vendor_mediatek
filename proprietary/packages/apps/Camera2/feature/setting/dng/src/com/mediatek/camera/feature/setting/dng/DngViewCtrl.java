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
package com.mediatek.camera.feature.setting.dng;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.View;

import com.mediatek.camera.R;
import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ISettingManager;
import com.mediatek.camera.common.widget.RotateImageView;

import java.util.List;
/**
 * Dng view controller.
 */

public class DngViewCtrl {
    private static final LogUtil.Tag TAG =
            new LogUtil.Tag(DngViewCtrl.class.getSimpleName());
    private MainHandler mMainHandler;
    private IApp mApp;
    private IAppUi mAppUi;
    private DngSettingView mDngSettingView = new DngSettingView();
    private View mDngIndicatorView;

    // View control
    private static final int VIEW_PRIORITY = 5;
    private static final int INDICATOR_VIEW_INIT = 0;
    private static final int INDICATOR_VIEW_UNINIT = 1;
    private static final int INDICATOR_VIEW_UPDATE = 2;

    /**
     * Listener with dng view.
     */
    interface OnDngSettingViewListener {
        void onItemViewClick(boolean isOn);
        boolean onCachedValue();
        boolean onUpdatedValue();
    }

    /**
     * Init the view.
     * @param app The camera activity.
     */
    public void init(IApp app) {
        mApp = app;
        mAppUi = app.getAppUi();
        mMainHandler = new MainHandler(app.getActivity().getMainLooper());
    }

    /**
     * This is to set dng view update listener.
     * @param  dngViewListener the dng setting view listener.
     */
    public void setDngSettingViewListener(OnDngSettingViewListener dngViewListener) {
        mDngSettingView.setDngViewListener(dngViewListener);
    }

    /**
     * Set current dng entry values.
     *
     * @param entryValue Dng entry values.
     */
    public void setEntryValue(List<String> entryValue) {
        mDngSettingView.setEntryValue(entryValue);
    }

    /**
     * Set setting device request.
     * @param settingRequester the setting device request.
     */
    public void setSettingDeviceRequest(ISettingManager.SettingDevice2Requester settingRequester) {
        mDngSettingView.setSettingRequester(settingRequester);
    }
    /**
     * Sets whether this Preference is enabled.
     *
     * @param enabled Set true to enable it.
     */
    public void setEnabled(boolean enabled) {
        mDngSettingView.setEnabled(enabled);
    }

    /**
     * This is to get dng setting view.
     * @return  the instance of dng setting View.
     */
    public DngSettingView getDngSettingView() {
        return mDngSettingView;
    }


    /**
     * This is to show dng indicator view.
     * @param isNeed true, if need to show.
     */
    public void showDngIndicatorView(boolean isNeed) {
        if (isNeed) {
            mMainHandler.sendEmptyMessage(INDICATOR_VIEW_INIT);
        } else {
            mMainHandler.sendEmptyMessage(INDICATOR_VIEW_UNINIT);
        }
    }

    /**
     * THis is to update dng view when dng state changed.
     */
    public void updateDngView() {
        mMainHandler.sendEmptyMessage(INDICATOR_VIEW_UPDATE);
    }

    /**
     * Handler let some task execute in main thread.
     */
    private class MainHandler extends Handler {
        public MainHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "[handleMessage]msg.what = " + msg.what);
            switch (msg.what) {
                case INDICATOR_VIEW_INIT:
                    initDngIndicatorView();
                    break;
                case INDICATOR_VIEW_UNINIT:
                    unInitDngIndicatorView();
                    break;
                case INDICATOR_VIEW_UPDATE:
                    updateDngSettingView();
                    break;
                default:
                    break;
            }
        }
    }

    private void initDngIndicatorView() {
        if (mDngIndicatorView == null) {
            mDngIndicatorView = (RotateImageView) mApp.getActivity().getLayoutInflater().inflate(
                    R.layout.dng_indicator, null);
        }
        mAppUi.addToIndicatorView(mDngIndicatorView, VIEW_PRIORITY);
    }

    private void unInitDngIndicatorView() {
        if (mDngIndicatorView != null) {
            mAppUi.removeFromIndicatorView(mDngIndicatorView);
        }
    }

    private void updateDngSettingView() {
        mDngSettingView.refreshView();
    }
}
