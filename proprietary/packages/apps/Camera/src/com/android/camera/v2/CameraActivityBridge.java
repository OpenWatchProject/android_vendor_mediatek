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
package com.android.camera.v2;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Configuration;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
import android.provider.MediaStore.Images.Media;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.FrameLayout;

import com.android.camera.CameraActivity;
import com.android.camera.ICameraActivityBridge;
import com.android.camera.R;
import com.android.camera.Util;
import com.android.camera.v2.app.AppController;
import com.android.camera.v2.app.CameraAppUI;
import com.android.camera.v2.app.GestureManager;
import com.android.camera.v2.app.GestureManagerImpl;
import com.android.camera.v2.app.ModuleManager;
import com.android.camera.v2.app.ModuleManagerImpl;
import com.android.camera.v2.app.OrientationManager;
import com.android.camera.v2.app.OrientationManagerImpl;
import com.android.camera.v2.app.PreviewManager;
import com.android.camera.v2.app.PreviewManagerImpl;
import com.android.camera.v2.app.SettingAgent;
import com.android.camera.v2.app.SettingAgent.SettingChangedListener;
import com.android.camera.v2.app.location.LocationManager;
import com.android.camera.v2.bridge.AppContextAdapter;
import com.android.camera.v2.bridge.AppControllerAdapter;
import com.android.camera.v2.bridge.ModeChangeAdapter;
import com.android.camera.v2.bridge.ModuleControllerAdapter;
import com.android.camera.v2.bridge.SettingAdapter;
import com.android.camera.v2.module.ModuleController;
import com.android.camera.v2.module.ModulesInfo;
import com.android.camera.v2.ui.PreviewStatusListener;
import com.android.camera.v2.ui.PreviewStatusListener.OnPreviewAreaChangedListener;
import com.android.camera.v2.uimanager.preference.PreferenceManager;
import com.android.camera.v2.util.CameraUtil;
import com.android.camera.v2.util.SettingKeys;
import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.regex.PatternSyntaxException;

public class CameraActivityBridge implements ICameraActivityBridge,
            AppController, OrientationManager.OnOrientationChangeListener {
    private static final Tag TAG = new Tag(CameraActivityBridge.class.getSimpleName());
    private final CameraActivity               mCameraActivity;
    private static final int                   MSG_NOTIFY_PREFERENCES_READY = 0;
    private static final int                   NOT_SHOW_STORAGE_HINT = 1;
    private static final int                   MSG_CLEAR_SCREEN_ON_FLAG = 1;
    private static final long                  SCREEN_DELAY_MS = 2 * 60 * 1000; // 2 mins.
    private final Context                      mAppContext;
    private final AppControllerAdapter         mAppControllerAdapter;
    private final AppContextAdapter            mAppContextAdapter;

    private boolean                            mPaused;
    private Handler                            mMainHandler;
    // module related
    private ModuleManager                      mModuleManager;
    private int                                mCurrentModuleIndex;
    private String                             mCurrentModeKey;
    private String                             mOldModeKey;
    private CameraModule                       mCurrentModule;
    private ModeChangeAdapter                  mModeChangeAdapter;
    // UI/Preview related
    private CameraAppUI                        mCameraAppUI;
    private PreferenceManager                  mPreferenceManager;
    private PreviewManagerImpl                 mPreviewManager;
    private SettingAgent                       mSettingAgent;
    // orientation
    private OrientationManager                 mOrientationManager;
    private int                                mLastRawOrientation;
    private static AlertDialog sAlertDialog;

    // gesture
    private GestureManagerImpl                 mGestureManagerImpl;

    /**
     * Whether the screen is kept turned on.
     */
    private boolean                             mKeepScreenOn;
    private int                                 mCurrentCameraId = 0;
    // True if the camera is started from secure lock screen.
    private boolean mSecureCamera;
    private boolean mIsGotoGallery = false;
    private boolean mIsLockScreen = false;
    private boolean mNeedShowThumbnail = true;
    private ActivityManager mActivityManager;
    private ArrayList<String> mSecureArray = new ArrayList<String>();
    private String mPath = null;

    // Secure album id. This should be incremented every time the camera is
    // launched from the secure lock screen. The id should be the same when
    // switching between camera, camcorder, and panorama.
    protected static int sSecureAlbumId;
    private static final String INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE =
            "android.media.action.STILL_IMAGE_CAMERA_SECURE";
    public static final String ACTION_IMAGE_CAPTURE_SECURE =
            "android.media.action.IMAGE_CAPTURE_SECURE";
    // The intent extra for camera from secure lock screen. True if the gallery
    // should only show newly captured pictures. sSecureAlbumId does not
    // increment. This is used when switching between camera, camcorder, and
    // panorama. If the extra is not set, it is in the normal camera mode.
    public static final String SECURE_CAMERA_EXTRA = "secure_camera";
    private LocationManager mLocationManager;
    private Object mStorageSpaceLock = new Object();
    public CameraActivityBridge(CameraActivity activity) {
        mCameraActivity = activity;
        mAppContext = mCameraActivity.getApplication().getBaseContext();
        // App controller adapter
        mAppControllerAdapter = new AppControllerAdapter(this);
        mAppContextAdapter    = new AppContextAdapter(mAppControllerAdapter);
    }

    /************************************** Camera Activity ************************************/

    @Override
    public void onCreate(Bundle icicle) {
        LogHelper.i(TAG, "[onCreate]...");
        Intent intent = mCameraActivity.getIntent();
        String action = intent.getAction();
        if (INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE.equals(action)) {
            mSecureCamera = true;
            mIsLockScreen = true;
            // Use a new album when this is started from the lock screen.
            sSecureAlbumId++;
        } else if (ACTION_IMAGE_CAPTURE_SECURE.equals(action)) {
            mSecureCamera = true;
        } else {
            mSecureCamera = intent.getBooleanExtra(SECURE_CAMERA_EXTRA, false);
        }
        mActivityManager = (ActivityManager) mCameraActivity.
                getSystemService(Context.ACTIVITY_SERVICE);
        LogHelper.i(TAG, "[onCreate]...mSecureCamera = " + mSecureCamera);
        if (mSecureCamera) {
            mNeedShowThumbnail = !mIsLockScreen;
            mPath = "/secure/all/" + sSecureAlbumId;
        }
        mAppContextAdapter.onCreate();

        // activity setup
        mMainHandler = new MainHandler(mCameraActivity, mCameraActivity.getMainLooper());
        mCameraActivity.setContentView(R.layout.camera_activity);
        mSettingAgent = new SettingAdapter(mAppControllerAdapter);
        // clear modes' value in sharedPreferences.
        mSettingAgent.clearSharedPreferencesValue(SettingKeys.MODE_KEYS,
                String.valueOf(mCurrentCameraId));
        mPreferenceManager = new PreferenceManager(getActivity(), mSettingAgent);

        // common ui
        mCameraAppUI = new CameraAppUI(this);
        mCameraAppUI.setSettingAgent(mSettingAgent);
        View viewRoot = mCameraActivity.findViewById(R.id.camera_view_container);
        viewRoot.bringToFront(); // bring to the top of surface view
        mCameraAppUI.init(viewRoot, mSecureCamera, isCaptureIntent());
        mCameraAppUI.prepareModuleUI();
        mCameraAppUI.updateSecureThumbnail(mNeedShowThumbnail);

        mOrientationManager = new OrientationManagerImpl(mCameraActivity);
        mOrientationManager.addOnOrientationChangeListener(mMainHandler, this);

        // LocationManager
        mLocationManager = new LocationManager(mAppContext);
        mSettingAgent
                .registerSettingChangedListener(mLocationSettingChangedListener, mMainHandler);

        // module
        mModuleManager = new ModuleManagerImpl();
        ModulesInfo.setupModules(mAppContext, mModuleManager);
        mCurrentModeKey = SettingKeys.KEY_NORMAL;
        mOldModeKey = SettingKeys.KEY_NORMAL;
        setModuleFromModeIndex(ModuleControllerAdapter.CAMERA_MODULE_INDEX);
        mCurrentModule.init(mCameraActivity, mSecureCamera, isCaptureIntent());
    }

    @Override
    public void onRestart() {
        LogHelper.i(TAG, "[onRestart]...");
    }

    @Override
    public void onResume() {
        LogHelper.i(TAG, "[onResume]...");
        hideAlertDialog();
        mPaused = false;
        initializePreferences(mCurrentCameraId, true);
        mAppContextAdapter.onResume();
        keepScreenOnForAWhile();
        mOrientationManager.resume();
        mCurrentModule.resume();
        updateStorageSpaceAndHint();
        updateSecureThumbnail();
        mCameraAppUI.updateSecureThumbnail(mNeedShowThumbnail);
        mCameraAppUI.onResume();
        LogHelper.i(TAG, "[onResume] end...");
    }

    @Override
    public void onPause() {
        LogHelper.i(TAG, "[onPause]...");
        mPaused = true;
        mNeedShowThumbnail = true;
        hideAlertDialog();
        if (mIsLockScreen && mSecureArray.isEmpty()) {
            mNeedShowThumbnail = false;
        }
        if (!mIsGotoGallery) {
            mSecureArray.clear();
        }
        mCameraAppUI.updateSecureThumbnail(mNeedShowThumbnail);
        mIsGotoGallery = false;
        mAppContextAdapter.onPause();
        mCameraAppUI.onPause();

        mOrientationManager.pause();
        resetScreenOn();

        mCurrentModule.pause();
    }

    @Override
    public void onDestroy() {
        LogHelper.i(TAG, "[onDestroy]...");

        mAppContextAdapter.onDestroy();
        mCameraAppUI.onDestroy();
        if (mOrientationManager != null) {
            mOrientationManager.removeOnOrientationChangeListener(mMainHandler, this);
            mOrientationManager = null;
        }

        mCurrentModule.destroy();
        mPreferenceManager.clearSharedPreferencesValue();
        closeGpsLocation();
    }

    /**
     * return true will pass onBackPressed to super class, app will exit.
     */
    @Override
    public boolean onBackPressed() {
        LogHelper.i(TAG, "[onBackPressed]...");
        if (mCurrentModule.onBackPressed() || mCameraAppUI.onBackPressed()) {
            // not call super.onBackPressed
            return false;
        }
        return true;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        LogHelper.i(TAG, "[onConfigurationChanged]... newConfig = " + newConfig);

        mCameraAppUI.onConfigurationChanged(newConfig);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {

    }

    @Override
    public boolean onUserInteraction() {
        if (!mCameraActivity.isFinishing()) {
            keepScreenOnForAWhile();
        }
        return false;
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {

    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        return false;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return false;
    }

    /************************************** App Controller  ************************************/

    @Override
    public Activity getActivity() {
        return mCameraActivity;
    }

    @Override
    public Context getAndroidContext() {
        return mAppContext;
    }

    @Override
    public String getModuleScope() {
        return null;
    }

    @Override
    public String getCameraScope() {
        return null;
    }

    @Override
    public void launchActivityByIntent(Intent intent) {

    }

    @Override
    public void openContextMenu(View view) {

    }

    @Override
    public void registerForContextMenu(View view) {

    }

    @Override
    public boolean isPaused() {
        return mCameraActivity.isActivityOnpause();
    }

    @Override
    public ModuleController getCurrentModuleController() {
        return null;
    }

    @Override
    public int getCurrentModuleIndex() {
        return mCurrentModuleIndex;
    }

    @Override
    public String getCurrentMode() {
        return mCurrentModeKey;
    }

    @Override
    public String getOldMode() {
        return mOldModeKey;
    }

    @Override
    public int getQuickSwitchToModuleId(int currentModuleIndex) {
        return 0;
    }

    @Override
    public int getPreferredChildModeIndex(int modeIndex) {
        return 0;
    }

    @Override
    public void onModeChanged(Map<String, String> changedModes) {
        LogHelper.i(TAG, "onModeChanged changedModes = " + changedModes);
        mOldModeKey = mCurrentModeKey;
        // compute new mode
        Set<String> set = changedModes.keySet();
        Iterator<String> iterator = set.iterator();
        List<String> modeKeys = new ArrayList<String>();
        while (iterator.hasNext()) {
            String modeKey = iterator.next();
            modeKeys.add(modeKey);
        }
        int size = modeKeys.size();
        String key = modeKeys.get(size - 1);
        String value = changedModes.get(key);

        String oldModeKey = mCurrentModeKey;
        mCurrentModeKey = key;
        if ("off".equals(value)) {
            mCurrentModeKey = SettingKeys.KEY_NORMAL;
        }

        boolean needSwitchModule = ModeChangeAdapter
                .isNeedSwitchModule(oldModeKey, mCurrentModeKey);
        LogHelper.i(TAG, "onModeChanged needSwitchModule =" + needSwitchModule
                + ",mCurrentModeKey = " + mCurrentModeKey);
        if (needSwitchModule) {
            // change module
            int moduleIndex = ModeChangeAdapter.getModuleIndex(mCurrentModeKey);
            if (moduleIndex != mCurrentModuleIndex) {
                closeModule(mCurrentModule);
                setModuleFromModeIndex(moduleIndex);
                openModule(mCurrentModule);
            }
        } else if (mModeChangeAdapter != null) {
            // change mode
            mModeChangeAdapter.onModeChanged(mCurrentModeKey);
        }
    }

    @Override
    public void setModeChangeListener(ModeChangeAdapter modeAdapter) {
        mModeChangeAdapter = modeAdapter;
    }

    @Override
    public void onSettingsSelected() {

    }

    @Override
    public void onPreviewVisibilityChanged(int visibility) {
        mCurrentModule.onPreviewVisibilityChanged(visibility);
    }

    @Override
    public PreviewManager getPreviewManager() {
        if (mPreviewManager == null) {
            mPreviewManager = new PreviewManagerImpl(getActivity());
        }
        return mPreviewManager;
    }

    @Override
    public void freezeScreenUntilPreviewReady() {

    }

    @Override
    public SurfaceTexture getPreviewBuffer() {
        return null;
    }

    @Override
    public void onPreviewReadyToStart() {

    }

    @Override
    public void onPreviewStarted() {
        mCameraAppUI.onPreviewStarted();
    }

    @Override
    public void setupOneShotPreviewListener() {

    }

    @Override
    public void updatePreviewAspectRatio(float aspectRatio) {

    }

    @Override
    public void updatePreviewTransformFullscreen(Matrix matrix,
            float aspectRatio) {

    }

    @Override
    public RectF getFullscreenRect() {
        return null;
    }

    @Override
    public void updatePreviewTransform(Matrix matrix) {

    }

    @Override
    public void setPreviewStatusListener(
            PreviewStatusListener previewStatusListener) {
        mCameraAppUI.setPreviewStatusListener(previewStatusListener);
    }

    @Override
    public void updatePreviewAreaChangedListener(
            OnPreviewAreaChangedListener listener, boolean isAddListener) {
        if (isAddListener) {
            mCameraAppUI.addPreviewAreaSizeChangedListener(listener);
        } else {
            mCameraAppUI.removePreviewAreaSizeChangedListener(listener);
        }
    }

    @Override
    public void updatePreviewSize(int previewWidth, int previewHeight) {
        mCameraAppUI.updatePreviewSize(previewWidth, previewHeight);
    }

    @Override
    public FrameLayout getModuleLayoutRoot() {
        return mCameraAppUI.getModuleRootView();
    }

    @Override
    public void lockOrientation() {
        if (mOrientationManager != null) {
            mOrientationManager.lockOrientation();
        }
    }

    @Override
    public void unlockOrientation() {
        if (mOrientationManager != null) {
            mOrientationManager.unlockOrientation();
        }
    }

    @Override
    public void setShutterButtonEnabled(boolean enabled, boolean videoShutter) {
        mCameraAppUI.setShutterButtonEnabled(enabled, videoShutter);
    }

    @Override
    public void setShutterEventListener(ShutterEventsListener eventListener,
            boolean videoShutter) {
        mCameraAppUI.setShutterEventListener(eventListener, videoShutter);
    }

    @Override
    public void setOkCancelClickListener(OkCancelClickListener listener) {
        mCameraAppUI.setOkCancelClickListener(listener);
    }

    @Override
    public boolean isShutterButtonEnabled(boolean videoShutter) {
        return mCameraAppUI.isShutterButtonEnabled(videoShutter);
    }

    @Override
    public void performShutterButtonClick(boolean clickVideoButton) {
        mCameraAppUI.performShutterButtonClick(clickVideoButton);
    }

    @Override
    public void startPreCaptureAnimation(boolean shortFlash) {

    }

    @Override
    public void startPreCaptureAnimation() {

    }

    @Override
    public void cancelPreCaptureAnimation() {

    }

    @Override
    public void startPostCaptureAnimation() {

    }

    @Override
    public void startPostCaptureAnimation(Bitmap thumbnail) {

    }

    @Override
    public void cancelPostCaptureAnimation() {

    }

    @Override
    public void notifyNewMedia(Uri uri) {
        LogHelper.i(TAG, "notifyNewMedia uri = " + uri + " mPaused = " + mPaused);
        if (mPaused) {
            return;
        }
        updateStorageSpaceAndHint();
        ContentResolver cr = mCameraActivity.getContentResolver();
        String mimeType = cr.getType(uri);
        boolean isVideo = false;
        if (CameraUtil.isMimeTypeVideo(mimeType)) {
            isVideo = true;
        } else if (CameraUtil.isMimeTypeImage(mimeType)) {
            isVideo = false;
        }
        if (mimeType.endsWith("image/x-adobe-dng")) {
            LogHelper.w(TAG, "DNG type, no need update thumbnail");
            return;
        }
        addSecureAlbumItemIfNeeded(isVideo, uri);
        if (mCameraAppUI != null) {
            mCameraAppUI.notifyMediaSaved(uri);
        }
    }

    @Override
    public void onCameraPicked(String newCameraId) {
        mCurrentModule.onBeforeCameraPicked(newCameraId);
        mCurrentCameraId = Integer.parseInt(newCameraId);
        initializePreferences(mCurrentCameraId, false);
        mCurrentModule.onCameraPicked(newCameraId);
    }

    @Override
    public void enableKeepScreenOn(boolean enabled) {
        if (mPaused) {
            return;
        }

        mKeepScreenOn = enabled;
        if (mKeepScreenOn) {
            mMainHandler.removeMessages(MSG_CLEAR_SCREEN_ON_FLAG);
            mCameraActivity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        } else {
            keepScreenOnForAWhile();
        }
    }

    @Override
    public OrientationManager getOrientationManager() {
        return mOrientationManager;
    }

    @Override
    public GestureManager getGestureManager() {
        if (mGestureManagerImpl == null) {
            mGestureManagerImpl = new GestureManagerImpl(this);
        }
        return mGestureManagerImpl;
    }

    @Override
    public PreferenceManager getPreferenceManager() {
        return mPreferenceManager;
    }

    @Override
    public CameraAppUI getCameraAppUI() {
        return mCameraAppUI;
    }

    @Override
    public void showErrorAndFinish(int messageId) {
        DialogInterface.OnClickListener buttonListener = new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface diaLogHelper, int which) {
                mCameraActivity.finish();
            }
        };
        if (mCameraActivity.isFinishing() || sAlertDialog != null || mPaused == true) {
            return;
        }
        sAlertDialog = new AlertDialog.Builder(mCameraActivity).setCancelable(false)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setTitle("").setMessage(messageId)
                .setNeutralButton(R.string.dialog_ok, buttonListener).show();
    }

    /**
     * Activity onPause will call this to do some thing that care.
     */
    public void hideAlertDialog() {
        if (sAlertDialog != null) {
            mCameraActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    sAlertDialog.dismiss();
                    sAlertDialog = null;
                }
            });
        }
    }

    @Override
    public AppControllerAdapter getAppControllerAdapter() {
        return mAppControllerAdapter;
    }

    @Override
    public void gotoGallery() {

        Intent intent = null;
        if (mCameraAppUI.getThumbnailMimeType().contains("image/")) {
            intent = new Intent(Intent.ACTION_VIEW);
        } else {
            intent = new Intent(Util.REVIEW_ACTION);
        }
        intent.setDataAndType(mCameraAppUI.getThumbnailUri(), mCameraAppUI.getThumbnailMimeType());
        intent.putExtra(Util.IS_CAMERA, true);
        if (mSecureCamera) {
            intent.putExtra(Util.IS_SECURE_CAMERA, true);
            intent.putExtra(Util.SECURE_ALBUM, getSecureAlbum());
            intent.putExtra(Util.SECURE_PATH, mPath);
            notifyGotoGallery();
        }
        // add this for screen pinning
        if (mActivityManager.LOCK_TASK_MODE_PINNED == mActivityManager.getLockTaskModeState()) {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_DOCUMENT |
                    Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
        }
        try {
            mCameraActivity.startActivity(intent);
        } catch (ActivityNotFoundException ex) {
            LogHelper.e(TAG, "[startGalleryActivity] Couldn't view " , ex);
        }
    }


    @Override
    public void setResultExAndFinish(int resultCode) {
        mCameraActivity.setResultExAndFinish(resultCode);
    }

    @Override
    public void setResultExAndFinish(int resultCode, Intent data) {
        mCameraActivity.setResultExAndFinish(resultCode, data);
    }

    public void setPlayButtonClickListener(PlayButtonClickListener listener) {
        mCameraAppUI.setPlayButtonClickListener(listener);
    }

    public void setRetakeButtonClickListener(RetakeButtonClickListener listener) {
        mCameraAppUI.setRetakeButtonClickListener(listener);
    }


    @Override
    public LocationManager getLocationManager() {
        return mLocationManager;
    }

    /************************************** App Controller  ************************************/

    @Override
    public void onOrientationChanged(int orientation) {
        if (orientation != mLastRawOrientation) {
            LogHelper.i(TAG, "orientation changed (from:to) " + mLastRawOrientation +
                    ":" + orientation);
        }
        // We keep the last known orientation. So if the user first orient
        // the camera then point the camera to floor or sky, we still have
        // the correct orientation.
        if (orientation == OrientationManager.ORIENTATION_UNKNOWN) {
            return;
        }
        mLastRawOrientation = orientation;
        if (mCurrentModule != null) {
            mCurrentModule.onOrientationChanged(orientation);
        }

        if (mCameraAppUI != null) {
            mCameraAppUI.onOrientationChanged(orientation);
        }
    }

    private void keepScreenOnForAWhile() {
        if (mKeepScreenOn) {
            return;
        }
        mMainHandler.removeMessages(MSG_CLEAR_SCREEN_ON_FLAG);
        mCameraActivity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mMainHandler.sendEmptyMessageDelayed(MSG_CLEAR_SCREEN_ON_FLAG, SCREEN_DELAY_MS);
    }

    private void resetScreenOn() {
        mKeepScreenOn = false;
        mMainHandler.removeMessages(MSG_CLEAR_SCREEN_ON_FLAG);
        mCameraActivity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private void openModule(CameraModule module) {
        module.init(mCameraActivity, mSecureCamera, isCaptureIntent());
        module.resume();
    }

    private void closeModule(CameraModule module) {
        module.pause();
        module.destroy();
        mCameraAppUI.clearModuleUI();
    }

    /**
     * Initialize preferences in an independent thread or in the caller's thread. Suggesting to
     * initialize preferences in an independent thread when resuming camera to decrease the time
     * of opening camera. Initializing preferences in the caller's thread which always is main
     * thread to avoid get the camera id is wrong from setting.
     * @param cameraId The camera id.
     * @param runnabled If true, initialize preferences in an independent thread, else in the
     * caller's thread.
     */
    private void initializePreferences(final int cameraId, boolean runnabled) {
        LogHelper.i(TAG, "[initializePreferences], cameraId:"
                + cameraId + ", runnabled:" + runnabled);
        if (!runnabled) {
            mPreferenceManager.initializePreferences(R.xml.camera_preferences_v2, cameraId);
            mMainHandler.sendEmptyMessage(MSG_NOTIFY_PREFERENCES_READY);
            return;
        }
        Thread t = new Thread(new Runnable() {

            @Override
            public void run() {
                mPreferenceManager.initializePreferences(R.xml.camera_preferences_v2, cameraId);
                mMainHandler.sendEmptyMessage(MSG_NOTIFY_PREFERENCES_READY);
            }
        }, "initialize-preferences-thread");

        t.start();
    }

    private class MainHandler extends Handler {
        final WeakReference<CameraActivity> mActivity;

        public MainHandler(CameraActivity activity, Looper looper) {
            super(looper);
            mActivity = new WeakReference<CameraActivity>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            CameraActivity activity = mActivity.get();
            if (activity == null) {
                return;
            }
            switch (msg.what) {
            case MSG_NOTIFY_PREFERENCES_READY:
                if (mCameraAppUI != null) {
                    mCameraAppUI.notifyPreferenceReady();
                }
                break;
            case MSG_CLEAR_SCREEN_ON_FLAG:
                if (!mPaused) {
                    mCameraActivity.getWindow().clearFlags(
                            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
                break;
            default:
                break;
            }
        }
    }

    /**
     * Sets the mCurrentModuleIndex, creates a new module instance for the given
     * index an sets it as mCurrentModule.
     */
    private void setModuleFromModeIndex(int modeIndex) {
        ModuleManagerImpl.ModuleAgent agent = mModuleManager.getModuleAgent(modeIndex);
        if (agent == null) {
            return;
        }
//        if (!agent.requestAppForCamera()) {
//            mCameraController.closeCamera(true);
//        }
        mCurrentModuleIndex = agent.getModuleId();
        mCurrentModule = (CameraModule) agent.createModule(this);
    }

    private boolean isCaptureIntent() {
        if (MediaStore.ACTION_VIDEO_CAPTURE.equals(mCameraActivity.getIntent().getAction())
                || MediaStore.ACTION_IMAGE_CAPTURE.equals(mCameraActivity.getIntent().getAction())
                || MediaStore.ACTION_IMAGE_CAPTURE_SECURE.equals(
                        mCameraActivity.getIntent().getAction())) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    public void updateStorageSpaceAndHint() {
        /*
         * We execute disk operations on a background thread in order to
         * free up the UI thread.  Synchronizing on the lock below ensures
         * that when getStorageSpaceBytes is called, the main thread waits
         * until this method has completed.
         *
         * However, .execute() does not ensure this execution block will be
         * run right away (.execute() schedules this AsyncTask for sometime
         * in the future. executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR)
         * tries to execute the task in parellel with other AsyncTasks, but
         * there's still no guarantee).
         * e.g. don't call this then immediately call getStorageSpaceBytes().
         * Instead, pass in an OnStorageUpdateDoneListener.
         */
        (new AsyncTask<Void, Void, Integer>() {
            @Override
            protected Integer doInBackground(Void ... arg) {
                LogHelper.i(TAG, "updateStorageSpaceAndHint doInBackground");
                synchronized (mStorageSpaceLock) {
                    return mAppControllerAdapter.getServices()
                            .getStorageService().getStorageHintInfo();
                }
            }

            @Override
            protected void onPostExecute(Integer bytes) {
                LogHelper.d(TAG, "[updateStorageSpaceAndHint]," +
                     " onPostExecute, bytes:" + bytes.intValue());
                updateStorageHint(bytes.intValue());

            }
        }).executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    /**
     * the result for permission check.
     * @param permissions the permission list.
     * @param grantResults the result for permission list.
     */
    public void onRequestLocationPermissionResult(String permissions[], int[] grantResults) {
        if (mCameraActivity.getPermissionManager().isCameraLocationPermissionsResultReady(
                permissions, grantResults)) {
            mLocationManager.recordLocation(true);
        } else {
            //set location off to setting
            mSettingAgent.doSettingChange(SettingKeys.KEY_RECORD_LOCATION, "off");
        }
    }

    private void updateStorageHint(int info) {
        LogHelper.d(TAG, "[updateStorageHint], info:" + info);
        String message = null;
        if (info != NOT_SHOW_STORAGE_HINT) {
            message = mAppContext.getResources().getString(info);
        }
        if (message != null) {
            mCameraAppUI.showHint(message);
        } else {
            mCameraAppUI.hideHint();
        }

    }

    private SettingChangedListener mLocationSettingChangedListener = new SettingChangedListener() {

        @Override
        public void onSettingResult(final Map<String, String> values,
                Map<String, String> overrideValues) {
            String isOn = values.get(SettingKeys.KEY_RECORD_LOCATION);
            LogHelper.i(TAG, "[onSettingResult], loaction is : " + isOn);
            // if the GPS location have not change from normal mode to PIP/CFB,
            // the value will be null,so we don't need do only action.
            //check location permission
            if (isOn == null) {
                return;
            }

            mCameraActivity.runOnUiThread(new Runnable() {

                @Override
                public void run() {
                    /**
                     * it send permission request, and show dialog for user check
                     * the result will be in onRequestPermissionsResult.
                     */
                    if ("on".equalsIgnoreCase(values
                                    .get(SettingKeys.KEY_RECORD_LOCATION))) {
                        boolean isLocationPermissionReady =
                                mCameraActivity.getPermissionManager()
                                .requestCameraLocationPermissions();
                        if (isLocationPermissionReady) {
                            mLocationManager.recordLocation(true);
                        }
                    } else if ("off".equalsIgnoreCase(values
                                    .get(SettingKeys.KEY_RECORD_LOCATION))) {
                        mLocationManager.recordLocation(false);
                    }
                }
            });
        }
    };

    private void closeGpsLocation() {
        if (mLocationManager != null) {
            mLocationManager.recordLocation(false);
        }
    }

    private void notifyGotoGallery() {
        mIsGotoGallery = true;
    }
    protected ArrayList<String> getSecureAlbum() {
        return mSecureArray;
    }

    public void setPath(String setPath) {
        // mAppBridge.setCameraPath(setPath);
    }

    private void addSecureAlbumItemIfNeeded(boolean isVideo, Uri uri) {
        if (mSecureCamera) {
            LogHelper.i(TAG, "addSecureAlbumItemIfNeeded uri = " + uri);
            int id = Integer.parseInt(uri.getLastPathSegment());
            // Notify Gallery the secure albums through String format
            // such as "4321+true", means file's id = 4321 and is video
            String videoIndex = isVideo ? "+true" : "+false";
            String secureAlbum = String.valueOf(id) + videoIndex;
            mSecureArray.add(secureAlbum);
        }
    }
    private void updateSecureThumbnail() {
        if (mIsLockScreen && !mSecureArray.isEmpty()) {
            if (checkSecureAlbumLive()) {
                mNeedShowThumbnail = true;
            } else {
                mNeedShowThumbnail = false;
            }
            LogHelper.i(TAG, "mNeedShowThumbnail = " + mNeedShowThumbnail);
        }
    }
    private boolean isSecureUriLive(int id) {
        Cursor cursor = null;
        try {
            // for file kinds of uri, query media database
            cursor = Media.query(mCameraActivity.getContentResolver(), MediaStore.Files
                    .getContentUri("external"), null, "_id=(" + id + ")", null, null);
            if (null != cursor) {
                LogHelper.w(TAG, "<isSecureUriLive> cursor " + cursor.getCount());
                return cursor.getCount() > 0;
            }
        } finally {
            if (null != cursor) {
                cursor.close();
                cursor = null;
            }
        }
        return true;
    }
    private boolean checkSecureAlbumLive() {
        if (mSecureArray != null && !mSecureArray.isEmpty()) {
            int albumCount = mSecureArray.size();
            LogHelper.d(TAG, "<checkSecureAlbum> albumCount " + albumCount);
            for (int i = 0; i < albumCount; i++) {
                try {
                    String[] albumItem = mSecureArray.get(i).split("\\+");
                    int albumItemSize = albumItem.length;
                    LogHelper.d(TAG, "<checkSecureAlbum> albumItemSize " + albumItemSize);
                    if (albumItemSize == 2) {
                        int id = Integer.parseInt(albumItem[0].trim());
                        boolean isVideo = Boolean.parseBoolean(albumItem[1]
                                .trim());
                        LogHelper.d(TAG, "<checkSecureAlbum> secure item : id " + id
                                + ", isVideo " + isVideo);
                        if (isSecureUriLive(id)) {
                            return true;
                        }
                    }
                } catch (NullPointerException ex) {
                    LogHelper.e(TAG, "<checkSecureAlbum> exception " + ex);
                } catch (PatternSyntaxException ex) {
                    LogHelper.e(TAG, "<checkSecureAlbum> exception " + ex);
                } catch (NumberFormatException ex) {
                    LogHelper.e(TAG, "<checkSecureAlbum> exception " + ex);
                }
            }
        }
        return false;
    }
}
