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

package com.mediatek.camera;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.MediaStore;
import android.view.KeyEvent;
import android.view.OrientationEventListener;
import android.view.View;
import android.view.WindowManager;

import com.mediatek.camera.common.IAppUi;
import com.mediatek.camera.common.IAppUiListener.OnThumbnailClickedListener;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;
import com.mediatek.camera.common.debug.profiler.IPerformanceProfile;
import com.mediatek.camera.common.debug.profiler.PerformanceTracker;
import com.mediatek.camera.common.mode.IModeListener;
import com.mediatek.camera.common.mode.ModeManager;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.common.utils.PriorityConcurrentSkipListMap;
import com.mediatek.camera.common.widget.RotateLayout;
import com.mediatek.camera.portability.WifiDisplayStatusEx;
import com.mediatek.camera.portability.pq.PictureQuality;
import com.mediatek.camera.ui.CameraAppUI;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * Camera app's activity.
 * used to manager the app's life cycle, transfer system information
 * (such as key event, configuration change event ....).
 * Create app common UI and add to the activity view tree.
 */
public class CameraActivity extends PermissionActivity implements IApp {
    private static final Tag TAG = new Tag(CameraActivity.class.getSimpleName());
    private static final int MSG_CLEAR_SCREEN_ON_FLAG = 0;
    private static final int MSG_SET_SCREEN_ON_FLAG = 1;
    private static final int DELAY_MSG_SCREEN_SWITCH = 2 * 60 * 1000; // 2min
    // Orientation hysteresis amount used in rounding, in degrees
    private static final int ORIENTATION_HYSTERESIS = 5;
    private static final String REVIEW_ACTION = "com.android.camera.action.REVIEW";
    private static final String IS_CAMERA = "isCamera";

    private CameraAppUI mCameraAppUI;
    private PriorityConcurrentSkipListMap<String, KeyEventListener> mKeyEventListeners =
            new PriorityConcurrentSkipListMap<String, KeyEventListener>(true);

    private PriorityConcurrentSkipListMap<String, BackPressedListener> mBackPressedListeners =
            new PriorityConcurrentSkipListMap<String, BackPressedListener>(true);

    private IModeListener mIModeListener;
    private boolean mIsResumed;

    private final List<OnOrientationChangeListener>
            mOnOrientationListeners = new ArrayList<>();
    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
    private OrientationEventListener mOrientationListener;
    protected Uri mUri;
    protected OnThumbnailClickedListener mThumbnailClickedListener =
            new OnThumbnailClickedListener() {
                @Override
                public void onThumbnailClicked() {
                    goToGallery(mUri);
                }
            };

    @Override
    protected void onNewIntentTasks(Intent newIntent) {
        super.onNewIntentTasks(newIntent);
    }

    @Override
    protected void onCreateTasks(Bundle savedInstanceState) {
        if (!isThirdPartyIntent(this)) {
            CameraAppService.launchCamera(this);
        }
        IPerformanceProfile profile = PerformanceTracker.create(TAG, "onCreate").start();
        super.onCreateTasks(savedInstanceState);
        if (CameraUtil.isTablet() || WifiDisplayStatusEx.isWfdEnabled(this)) {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED);
            setRequestedOrientation(CameraUtil.calculateCurrentScreenOrientation(this));
        }
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_LAYOUT_FLAGS
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        setContentView(R.layout.activity_main);
        mOrientationListener = new OrientationEventListenerImpl(this);
        //create common ui module.
        mCameraAppUI = new CameraAppUI(this);
        profile.mark("CameraAppUI initialized.");
        mCameraAppUI.onCreate();
        profile.mark("CameraAppUI.onCreate done.");
        mIModeListener = new ModeManager();
        mIModeListener.create(this);
        profile.mark("ModeManager.create done.");
        profile.stop();
    }

    @Override
    protected void onStartTasks() {
        super.onStartTasks();
    }

    @Override
    protected void onResumeTasks() {
        CameraAppService.setCurrentActivity(this);
        IPerformanceProfile profile = PerformanceTracker.create(TAG, "onResume").start();
        mIsResumed = true;
        mOrientationListener.enable();
        super.onResumeTasks();
        PictureQuality.enterCameraMode();
        mIModeListener.resume();
        profile.mark("ModeManager resume done.");
        mCameraAppUI.onResume();
        profile.mark("CameraAppUI resume done.");
        mCameraAppUI.setThumbnailClickedListener(mThumbnailClickedListener);
        keepScreenOnForAWhile();
        profile.stop();
    }

    @Override
    protected void onPauseTasks() {
        mIsResumed = false;
        super.onPauseTasks();
        PictureQuality.exitCameraMode();
        mIModeListener.pause();
        mCameraAppUI.onPause();
        mOrientationListener.disable();
        resetScreenOn();
        CameraAppService.exitCamera(this);
    }

    @Override
    protected void onStopTasks() {
        super.onStopTasks();
    }

    @Override
    protected void onDestroyTasks() {
        super.onDestroyTasks();
        mIModeListener.destroy();
        mCameraAppUI.onDestroy();
        CameraAppService.release(this);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        RotateLayout root = (RotateLayout) findViewById(R.id.app_ui);
        LogHelper.d(TAG, "onConfigurationChanged orientation = " + newConfig.orientation);
        if (root != null) {
            if (newConfig.orientation == Configuration.ORIENTATION_PORTRAIT) {
                root.setOrientation(0, false);
            } else if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) {
                root.setOrientation(90, false);
            }
            mCameraAppUI.onConfigurationChanged(newConfig);
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        Iterator iterator = mKeyEventListeners.entrySet().iterator();
        KeyEventListener listener = null;
        while (iterator.hasNext()) {
            Map.Entry map = (Map.Entry) iterator.next();
            listener = (KeyEventListener) map.getValue();
            if (listener != null && listener.onKeyDown(keyCode, event)) {
                return true;
            }
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        Iterator iterator = mKeyEventListeners.entrySet().iterator();
        KeyEventListener listener = null;
        while (iterator.hasNext()) {
            Map.Entry map = (Map.Entry) iterator.next();
            listener = (KeyEventListener) map.getValue();
            if (listener != null && listener.onKeyUp(keyCode, event)) {
                return true;
            }
        }
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public void onBackPressed() {
        Iterator iterator = mBackPressedListeners.entrySet().iterator();
        BackPressedListener listener = null;
        while (iterator.hasNext()) {
            Map.Entry map = (Map.Entry) iterator.next();
            listener = (BackPressedListener) map.getValue();
            if (listener != null && listener.onBackPressed()) {
                return;
            }
        }
        super.onBackPressed();
    }

    @Override
    public void onUserInteraction() {
        if (mIModeListener == null || !mIModeListener.onUserInteraction()) {
            super.onUserInteraction();
        }
    }

    @Override
    public Activity getActivity() {
        return this;
    }

    @Override
    public IAppUi getAppUi() {
        return mCameraAppUI;
    }

    @Override
    public void enableKeepScreenOn(boolean enabled) {
        LogHelper.d(TAG, "enableKeepScreenOn enabled " + enabled);
        if (mIsResumed) {
            mMainHandler.removeMessages(MSG_SET_SCREEN_ON_FLAG);
            Message msg = Message.obtain();
            msg.arg1 = enabled ? 1 : 0;
            msg.what = MSG_SET_SCREEN_ON_FLAG;
            mMainHandler.sendMessage(msg);
        }

    }

    @Override
    public void notifyNewMedia(Uri uri, boolean needNotify) {
        mUri = uri;
    }

    @Override
    public boolean notifyCameraSelected(String newCameraId) {
        return mIModeListener.onCameraSelected(newCameraId);
    }

    @Override
    public void registerKeyEventListener(KeyEventListener keyEventListener, int priority) {
        if (keyEventListener == null) {
            LogHelper.e(TAG, "registerKeyEventListener error [why null]");
        }
        mKeyEventListeners.put(mKeyEventListeners.getPriorityKey(priority, keyEventListener),
                keyEventListener);
    }

    @Override
    public void registerBackPressedListener(BackPressedListener backPressedListener,
            int priority) {
        if (backPressedListener == null) {
            LogHelper.e(TAG, "registerKeyEventListener error [why null]");
        }
        mBackPressedListeners.put(mBackPressedListeners.getPriorityKey(priority,
                backPressedListener), backPressedListener);
    }

    @Override
    public void unRegisterKeyEventListener(KeyEventListener keyEventListener) {
        if (keyEventListener == null) {
            LogHelper.e(TAG, "unRegisterKeyEventListener error [why null]");
        }
        if (mKeyEventListeners.containsValue(keyEventListener)) {
            mKeyEventListeners.remove(mKeyEventListeners.findKey(keyEventListener));
        }
    }

    @Override
    public void unRegisterBackPressedListener(BackPressedListener backPressedListener) {
        if (backPressedListener == null) {
            LogHelper.e(TAG, "unRegisterBackPressedListener error [why null]");
        }
        if (mBackPressedListeners.containsValue(backPressedListener)) {
            mBackPressedListeners.remove(mBackPressedListeners.findKey(backPressedListener));
        }
    }

    @Override
    public void registerOnOrientationChangeListener(OnOrientationChangeListener listener) {
        synchronized (mOnOrientationListeners) {
            if (!mOnOrientationListeners.contains(listener)) {
                if (mOrientation != OrientationEventListener.ORIENTATION_UNKNOWN) {
                    listener.onOrientationChanged(mOrientation);
                }
                mOnOrientationListeners.add(listener);
            }
        }
    }

    @Override
    public void unregisterOnOrientationChangeListener(OnOrientationChangeListener listener) {
        synchronized (mOnOrientationListeners) {
            if (mOnOrientationListeners.contains(listener)) {
                mOnOrientationListeners.remove(listener);
            }
        }
    }

    @Override
    public int getGSensorOrientation() {
        synchronized (mOnOrientationListeners) {
            return mOrientation;
        }
    }

    @Override
    public void enableGSensorOrientation() {
        mOrientationListener.enable();
    }

    @Override
    public void disableGSensorOrientation() {
        mOrientationListener.disable();
    }

    /**
     * start gallery activity to browse the file withe specified uri.
     *
     * @param uri The specified uri of file to browse.
     */
    protected void goToGallery(Uri uri) {
        if (uri == null) {
            LogHelper.d(TAG, "uri is null, can not go to gallery");
            return;
        }
        String mimeType = getContentResolver().getType(uri);
        LogHelper.d(TAG, "[goToGallery] uri: " + uri + ", mimeType = " + mimeType);
        Intent intent = new Intent(REVIEW_ACTION);
        intent.setDataAndType(uri, mimeType);
        intent.putExtra(IS_CAMERA, true);
        // add this for screen pinning
        ActivityManager activityManager = (ActivityManager)
                getSystemService(Context.ACTIVITY_SERVICE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (activityManager.LOCK_TASK_MODE_PINNED == activityManager
                    .getLockTaskModeState()) {
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_DOCUMENT
                        | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
            }
        }
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException ex) {
            LogHelper.e(TAG, "[startGalleryActivity] Couldn't view ", ex);
        }
    }

    private Handler mMainHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            LogHelper.d(TAG, "handleMessage what = " + msg.what + " arg1 = " + msg.arg1);
            switch (msg.what) {
                case MSG_CLEAR_SCREEN_ON_FLAG:
                    getWindow().clearFlags(
                            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                    break;
                case MSG_SET_SCREEN_ON_FLAG:
                    boolean enabled = msg.arg1 == 1;
                    if (enabled) {
                        keepScreenOn();
                    } else {
                        keepScreenOnForAWhile();
                    }
                    break;
                default:
                    break;
            }
        };
    };

    private void resetScreenOn() {
        mMainHandler.removeMessages(MSG_SET_SCREEN_ON_FLAG);
        mMainHandler.removeMessages(MSG_CLEAR_SCREEN_ON_FLAG);
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private void keepScreenOnForAWhile() {
        mMainHandler.removeMessages(MSG_CLEAR_SCREEN_ON_FLAG);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mMainHandler.sendEmptyMessageDelayed(MSG_CLEAR_SCREEN_ON_FLAG,
                DELAY_MSG_SCREEN_SWITCH);
    }

    private void keepScreenOn() {
        mMainHandler.removeMessages(MSG_CLEAR_SCREEN_ON_FLAG);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    /**
     * The implementer of OrientationEventListener.
     */
    private class OrientationEventListenerImpl extends OrientationEventListener {
        public OrientationEventListenerImpl(Context context) {
            super(context);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            // We keep the last known orientation. So if the user first orient
            // the camera then point the camera to floor or sky, we still have
            // the correct orientation.
            if (orientation == ORIENTATION_UNKNOWN) {
                return;
            }
            synchronized (mOnOrientationListeners) {
                final int roundedOrientation = roundOrientation(orientation, mOrientation);
                if (mOrientation != roundedOrientation) {
                    mOrientation = roundedOrientation;
                    LogHelper.i(TAG, "mOrientation = " + mOrientation);
                    for (OnOrientationChangeListener l : mOnOrientationListeners) {
                        l.onOrientationChanged(mOrientation);
                    }
                }
            }
        }
    }

    private static int roundOrientation(int orientation, int orientationHistory) {
        boolean changeOrientation = false;
        if (orientationHistory == OrientationEventListener.ORIENTATION_UNKNOWN) {
            changeOrientation = true;
        } else {
            int dist = Math.abs(orientation - orientationHistory);
            dist = Math.min(dist, 360 - dist);
            changeOrientation = (dist >= 45 + ORIENTATION_HYSTERESIS);
        }
        if (changeOrientation) {
            return ((orientation + 45) / 90 * 90) % 360;
        }
        return orientationHistory;
    }

    /**
     * Judge current is launch by intent.
     * @param activity the launch activity.
     * @return true means is launch by intent; otherwise is false.
     */
    private boolean isThirdPartyIntent(Activity activity) {
        Intent intent = activity.getIntent();
        String action = intent.getAction();
        boolean value = MediaStore.ACTION_IMAGE_CAPTURE.equals(action) ||
                MediaStore.ACTION_IMAGE_CAPTURE_SECURE.equals(action) ||
                MediaStore.ACTION_VIDEO_CAPTURE.equals(action);
        return value;
    }
}
