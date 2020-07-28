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

package com.android.camera.v2.bridge;

import java.util.ArrayList;

import junit.framework.Assert;
import android.app.Activity;
import android.content.Intent;
import android.graphics.RectF;
import android.net.Uri;

import com.android.camera.v2.app.location.LocationManager;
import com.android.camera.v2.ui.PreviewStatusListener;

import com.mediatek.camera.v2.platform.ModeChangeListener;
import com.mediatek.camera.v2.platform.app.AppContext;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.platform.app.AppUi;
import com.mediatek.camera.v2.platform.device.CameraDeviceManager;
import com.mediatek.camera.v2.platform.module.ModuleUi;
import com.mediatek.camera.v2.platform.module.ModuleUi.PreviewAreaChangedListener;
import com.mediatek.camera.v2.services.CameraAppContext;
import com.mediatek.camera.v2.services.CameraServices;

/**
 *  This adapter is used to adapt module level calls to app level.
 */
public class AppControllerAdapter implements AppController {

    private final com.android.camera.v2.app.AppController   mAppController;
    private CameraAppContext                                mCameraContext;
    private ModuleUIAdapter                                 mPreviewStatusListner;
    private AppUIAdapter                                    mAppUIAdapter;
    private ModeChangeAdapter                               mModeChangeAdapter;
    private ModulePreviewAreaChangedListener                mPreviewAreaChangedListener;
    private CameraDeviceManager                             mCameraDeviceManager;

    public AppControllerAdapter(com.android.camera.v2.app.AppController app) {
        Assert.assertNotNull(app);
        mAppController = app;
    }

    @Override
    public Activity getActivity() {
        return mAppController.getActivity();
    }

    @Override
    public int getCurrentModeIndex() {
        return ModeChangeAdapter.getModeIndexFromKey(mAppController.getCurrentMode());
    }

    @Override
    public int getOldModeIndex() {
        return ModeChangeAdapter.getModeIndexFromKey(mAppController.getOldMode());
    }

    @Override
    public CameraDeviceManager getCameraManager() {
        if (mCameraDeviceManager == null) {
            mCameraDeviceManager = CameraDeviceManager.get(getActivity());
        }
        return mCameraDeviceManager;
    }

    @Override
    public CameraDeviceManager getNewCameraManager() {
        return CameraDeviceManager.getNew(getActivity());
    }

    @Override
    public void updateStorageSpaceAndHint() {
        mAppController.updateStorageSpaceAndHint();
    }

    @Override
    public LocationManager getLocationManager() {
        return mAppController.getLocationManager();
    }

    @Override
    public void notifyNewMedia(Uri uri) {
        mAppController.notifyNewMedia(uri);
    }

    @Override
    public AppUi getCameraAppUi() {
        if (mAppUIAdapter == null) {
            mAppUIAdapter = new AppUIAdapter(mAppController);
        }
        return mAppUIAdapter;
    }

    @Override
    public void lockOrientation() {
        mAppController.lockOrientation();
    }

    @Override
    public void unlockOrientation() {
        mAppController.unlockOrientation();
    }

    @Override
    public void setModuleUiListener(ModuleUi moduleUi) {
        if (mPreviewStatusListner == null || moduleUi != mPreviewStatusListner.getModuleUi()) {
            mPreviewStatusListner = new ModuleUIAdapter(moduleUi);
        }
        mAppController.setPreviewStatusListener(mPreviewStatusListner);
    }

    @Override
    public void updatePreviewSize(int previewWidth, int previewHeight) {
        mAppController.updatePreviewSize(previewWidth, previewHeight);
    }

    @Override
    public void onPreviewStarted() {
        mAppController.onPreviewStarted();
    }



    @Override
    public void showErrorAndFinish(int messageId) {
        mAppController.showErrorAndFinish(messageId);
    }
    @Override
    public void updatePreviewAspectRatioAndSize(float aspectRatio,
            int previewWidth, int previewHeight) {
    }

    @Override
    public void addPreviewAreaSizeChangedListener(PreviewAreaChangedListener listener) {
        if (mPreviewAreaChangedListener == null) {
            mPreviewAreaChangedListener = new ModulePreviewAreaChangedListener();
            mAppController.updatePreviewAreaChangedListener(mPreviewAreaChangedListener, true);
        }
        mPreviewAreaChangedListener.addPreviewAreaChangedListener(listener);
    }

    @Override
    public void removePreviewAreaSizeChangedListener(PreviewAreaChangedListener listener) {
        if (mPreviewAreaChangedListener != null) {
            mPreviewAreaChangedListener.removePreviewAreaChangedListener(listener);
            if (mPreviewAreaChangedListener.getListenerCount() == 0) {
                mAppController.updatePreviewAreaChangedListener(mPreviewAreaChangedListener, false);
                mPreviewAreaChangedListener = null;
            }
        }
    }

    @Override
    public void enableKeepScreenOn(boolean enabled) {
        mAppController.enableKeepScreenOn(enabled);
    }

    @Override
    public AppContext getAppContext() {
        if (mCameraContext == null) {
            mCameraContext = new CameraAppContext(this);
        }
        return mCameraContext;
    }

    @Override
    public CameraServices getServices() {
        if (mCameraContext == null) {
            mCameraContext = new CameraAppContext(this);
        }
        return mCameraContext;
    }

    @Override
    public void setModeChangeListener(ModeChangeListener modeChangeListener) {
        mModeChangeAdapter = new ModeChangeAdapter(modeChangeListener);
        mAppController.setModeChangeListener(mModeChangeAdapter);
    }

    @Override
    public void setResultExAndFinish(int resultCode) {
        mAppController.setResultExAndFinish(resultCode);
    }

    @Override
    public void setResultExAndFinish(int resultCode, Intent data) {
        mAppController.setResultExAndFinish(resultCode, data);
    }


    private class ModulePreviewAreaChangedListener implements
            PreviewStatusListener.OnPreviewAreaChangedListener {
        private RectF mPreviewArea = new RectF();
        private ArrayList<PreviewAreaChangedListener> mListeners =
                new ArrayList<PreviewAreaChangedListener>();

        public void addPreviewAreaChangedListener(PreviewAreaChangedListener listener) {
            if (listener != null && !mListeners.contains(listener)) {
                mListeners.add(listener);
            }
            if (mPreviewArea.width() != 0 || mPreviewArea.height() != 0) {
                listener.onPreviewAreaChanged(mPreviewArea);
            }
        }

        public void removePreviewAreaChangedListener(PreviewAreaChangedListener listener) {
            if (listener != null && mListeners.contains(listener)) {
                mListeners.remove(listener);
            }
        }

        public int getListenerCount() {
            return mListeners.size();
        }

        @Override
        public void onPreviewAreaChanged(RectF previewArea) {
            mPreviewArea = previewArea;
            for (PreviewAreaChangedListener listener: mListeners) {
                listener.onPreviewAreaChanged(previewArea);
            }
        }
    }
}
