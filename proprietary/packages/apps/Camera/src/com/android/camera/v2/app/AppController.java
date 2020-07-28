/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.camera.v2.app;

import java.util.Map;

import com.android.camera.v2.app.location.LocationManager;
import com.android.camera.v2.bridge.AppControllerAdapter;
import com.android.camera.v2.bridge.ModeChangeAdapter;
import com.android.camera.v2.module.ModuleController;
import com.android.camera.v2.ui.PreviewStatusListener;
import com.android.camera.v2.ui.PreviewStatusListener.OnPreviewAreaChangedListener;
import com.android.camera.v2.uimanager.preference.PreferenceManager;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.RectF;
import android.graphics.SurfaceTexture;
import android.net.Uri;
import android.view.View;
import android.widget.FrameLayout;

/**
 * The controller at app level.
 *
 * <p> including following events: <p>
 * <li>UI / Camera preview</li>
 * <li>{@link #getPreviewManager()}</li>
 *
 * <li>Shutter button</li>
 * <li>App-level resources:</li>
 * <li>{@link #enableKeepScreenOn(boolean)}</li>
 * <li>{@link #getGestureManager()}</li>
 * <li>{@link #getPreferenceManager()}</li>
 *
 */
public interface AppController {
    /**
     * An interface which defines the shutter events listener.
     */
    public interface ShutterEventsListener {
        /**
         * Called when the shutter state is changed to pressed.
         */
        public void onShutterPressed();

        /**
         * Called when the shutter state is changed to released.
         */
        public void onShutterReleased();

        /**
         * Called when the shutter is clicked.
         */
        public void onShutterClicked();

        /**
         * Called when the shutter is long pressed.
         */
        public void onShutterLongPressed();
    }

    /**
     * 3rd party launch camera, after capture or recording, ok and cancel button
     * will be shown in shutter manager.
     *
     */
    public interface OkCancelClickListener {
        public void onOkClick();
        public void onCancelClick();
    }

    /**
     * 3rd party launch camera or video, after capture or recording, play button and
     * retake button will be shown in review ui.
     *
     */
    public interface PlayButtonClickListener {
        public void onPlay();
    }

    /**
     * 3rd party launch camera or video, after capture or recording, play button and
     * retake button will be shown in review ui.
     *
     */
    public interface RetakeButtonClickListener {
        public void onRetake();
    }

    /**
     *
     * @return the {@link android.app.Activity} being used.
     */
    public Activity getActivity();

    /**
     * TODO consider remove this, use getActivity instead
     * @return the {@link android.content.Context} being used.
     */
    public Context getAndroidContext();

    /**
     * @return a String scope uniquely identifing the current module.
     */
    public String getModuleScope();

    /**
     * @return a String scope uniquely identifing the current camera id.
     */
    public String getCameraScope();

    /**
     * Starts an activity.
     *
     * @param intent Used to start the activity.
     */
    public void launchActivityByIntent(Intent intent);

    /**
     * See {@link Activity#openContextMenu(View)}
     */
    public void openContextMenu(View view);

    /**
     * See {@link Activity#registerForContextMenu(View)}
     */
    public void registerForContextMenu(View view);

    /**
     * Returns whether the app is currently paused.
     */
    public boolean isPaused();

    /**
     * Returns the current module controller.
     */
    public ModuleController getCurrentModuleController();

    /**
     * Returns the currently active module index.
     */
    public int getCurrentModuleIndex();

    /**
     * Returns the currently active mode.
     * @return Return current active mode.
     */
    public String getCurrentMode();

    /**
     * Get old mode.
     * @return Return old mode.
     */
    public String getOldMode();

    /**
     * Gets the mode that can be switched to from the given mode id through
     * quick switch.
     *
     * @param currentModuleIndex index of the current mode
     * @return mode id to quick switch to if index is valid, otherwise returns
     *         the given mode id itself
     */
    public int getQuickSwitchToModuleId(int currentModuleIndex);

    /**
     * Based on a mode switcher index, choose the correct module index.
     *
     * @param modeIndex mode switcher index.
     * @return module index.
     */
    public int getPreferredChildModeIndex(int modeIndex);

    public void onModeChanged(Map<String, String> changedModes);

    public void setModeChangeListener(ModeChangeAdapter modeAdapter);

    /**
     * This gets called when settings is selected and settings dialog needs to open.
     */
    public void onSettingsSelected();

    /********************************* UI / Camera preview *******************************/
    /**
     * Called when the preview becomes visible/invisible.
     */
    public void onPreviewVisibilityChanged(int visibility);

    /**
     * Get an instance of {@link PreviewManager}
     * @return a preview manager.
     */
    public PreviewManager getPreviewManager();

    /**
     * Freeze what is currently shown on screen until the next preview frame comes
     * in. This can be used for camera switch to hide the UI changes underneath
     * until preview is ready.
     */
    public void freezeScreenUntilPreviewReady();

    /**
     * Returns the {@link android.graphics.SurfaceTexture} used by the preview
     * UI.
     */
    public SurfaceTexture getPreviewBuffer();

    /**
     * Gets called from module when preview is ready to start.
     */
    public void onPreviewReadyToStart();

    /**
     * Gets called from module when preview is started.
     */
    public void onPreviewStarted();

//    /**
//     * Adds a listener to receive callbacks when preview area changes.
//     */
//    public void addPreviewAreaSizeChangedListener(
//            PreviewStatusListener.PreviewAreaChangedListener listener);
//
//    /**
//     * Removes a listener that receives callbacks when preview area changes.
//     */
//    public void removePreviewAreaSizeChangedListener(
//            PreviewStatusListener.PreviewAreaChangedListener listener);

    /**
     * Sets up one shot preview callback in order to notify UI when the next
     * preview frame comes in.
     */
    public void setupOneShotPreviewListener();

    /**
     * Gets called from module when preview aspect ratio has changed.
     *
     * @param aspectRatio aspect ratio of preview stream
     */
    public void updatePreviewAspectRatio(float aspectRatio);

    /**
     * Gets called from module when the module needs to change the transform
     * matrix of the preview TextureView. It does not modify the matrix before
     * applying it.
     *
     * @param matrix transform matrix to be set on preview TextureView
     * @param aspectRatio the desired aspect ratio of the preview
     */
    public void updatePreviewTransformFullscreen(Matrix matrix, float aspectRatio);

    /**
     * Call this to find the full rect available for a full screen preview
     *
     * @return the rect of the full screen minus any decor.
     */
    public RectF getFullscreenRect();

    /**
     * Gets called from module when the module needs to change the transform
     * matrix of the preview TextureView. It is encouraged to use
     * {@link #updatePreviewAspectRatio(float)} over this function, unless the
     * module needs to rotate the surface texture using transform matrix.
     *
     * @param matrix transform matrix to be set on preview TextureView
     */
    public void updatePreviewTransform(Matrix matrix);

    /**
     * Sets the preview status listener, which will get notified when TextureView
     * surface has changed
     *
     * @param previewStatusListener the listener to get callbacks
     */
    public void setPreviewStatusListener(PreviewStatusListener previewStatusListener);

    public void updatePreviewAreaChangedListener(OnPreviewAreaChangedListener listener,
            boolean isAddListener);

    public void updatePreviewSize(int previewWidth, int previewHeight);

    /**
     * Returns the {@link android.widget.FrameLayout} as the root of the module
     * layout.
     */
    public FrameLayout getModuleLayoutRoot();

    /**
     * Locks the system orientation.
     */
    public void lockOrientation();

    /**
     * Unlocks the system orientation.
     */
    public void unlockOrientation();

    /********************** Shutter button  **********************/

    /**
     * Enable / Disable photo / video shutter button.
     * @param enabled true enable shutter button; false disable shutter button.
     * @param videoShutter true for video button; false for photo button.
     */
    public void setShutterButtonEnabled(boolean enabled, boolean videoShutter);
    /**
     * Set photo / video shutter button event listener.
     * TODO consider refactor this for add/remove shutter event listener
     * @param eventListener
     * @param videoShutter
     */
    public void setShutterEventListener(ShutterEventsListener eventListener, boolean videoShutter);

    /**
     * Set ok and cancel button click listener.
     * @param listener
     */
    public void setOkCancelClickListener(OkCancelClickListener listener);

    /**
     * Check shutter button enable status.
     * @param videoShutter true check video shutter button whether is enable.
     * @return whether the specified shutter button is enable.
     */
    public boolean isShutterButtonEnabled(boolean videoShutter);

    /**
     * Trigger video / photo shutter button's click event
     * @param clickVideoButton true, click video button; false, click photo button.
     */
    public void performShutterButtonClick(boolean clickVideoButton);

    /********************** Capture animation **********************/

    /**
     * Starts the pre-capture animation with optional shorter flash.
     *
     * @param shortFlash true for shorter flash (faster cameras).
     */
    public void startPreCaptureAnimation(boolean shortFlash);

    /**
     * Starts normal pre-capture animation.
     */
    public void startPreCaptureAnimation();

    /**
     * Cancels the pre-capture animation.
     */
    public void cancelPreCaptureAnimation();

    /**
     * Starts the post-capture animation with the current preview image.
     */
    public void startPostCaptureAnimation();

    /**
     * Starts the post-capture animation with the given thumbnail.
     *
     * @param thumbnail The thumbnail for the animation.
     */
    public void startPostCaptureAnimation(Bitmap thumbnail);

    /**
     * Cancels the post-capture animation.
     */
    public void cancelPostCaptureAnimation();

    /********************** Media saving *****************************/

    /**
     * Notifies the app of the newly captured media.
     */
    public void notifyNewMedia(Uri uri);

    /********************************* App-level resources *******************************/

    /**
     * Called when user click switch camera icon.
     * @param newCameraId the new camera id
     */
    public void onCameraPicked(String newCameraId);

    /**
     * Keeps the screen turned on.
     *
     * @param enabled Whether to keep the screen on.
     */
    public void enableKeepScreenOn(boolean enabled);

    /**
     * Returns the {@link OrientationManagerImpl}.
     *
     * @return {@code null} if not available yet.
     */
    public OrientationManager getOrientationManager();

    /**
     *  Get an instance of GestureManager.
     * @return An instance of {@link GestureManager}
     */
    public GestureManager getGestureManager();
    /**
     *
     * @return
     */
    public PreferenceManager getPreferenceManager();

    /**
     * Returns the {@link com.android.camera.v2.app.CameraAppUI}.
     *
     * @return {@code null} if not available yet.
     */
    public CameraAppUI getCameraAppUI();
//
//    /**
//     * Returns the {@link com.android.camera.app.ModuleManager}.
//     *
//     * @return {@code null} if not available yet.
//     */
//    public ModuleManager getModuleManager();
//
//    /**
//     * Returns the {@link com.android.camera.ButtonManager}.
//     */
//    public ButtonManager getButtonManager();
//
//    /** Returns a sound player that can be used to play custom sounds. */
//    public SoundPlayer getSoundPlayer();
//
//    /** Whether auto-rotate is enabled.*/
//    public boolean isAutoRotateScreen();
//
//    /**
//     * Shows the given tutorial overlay.
//     */
//    public void showTutorial(AbstractTutorialOverlay tutorial);

    /**
     * Shows and error message on the screen and, when dismissed, exits the
     * activity.
     *
     * @param messageId the ID of the message to show on screen before exiting.
     */
    public void showErrorAndFinish(int messageId);

    public AppControllerAdapter getAppControllerAdapter();

    /**
     * Go to gallery
     */
    public void gotoGallery();

    /**
     * Set result and finish camera activity
     * @param resultCode
     */
    public void setResultExAndFinish(int resultCode);

    /**
     * Set result and finish camera activity
     * @param resultCode
     * @param data
     */
    public void setResultExAndFinish(int resultCode, Intent data);

    /**
     * Set the listener to listen play button that is in review ui click.
     * @param listener
     */
    public void setPlayButtonClickListener(PlayButtonClickListener listener);


    /**
     * Set the listener to listen retake button that is in review ui click.
     * @param listener
     */
    public void setRetakeButtonClickListener(RetakeButtonClickListener listener);


    /**
     * Get location Manager.
     * @return returns the LocationManager
     */
    public LocationManager getLocationManager();

    /**
     * Update storage space and show hint.
     * show string in the camera preview to remind user.
     */
    public void updateStorageSpaceAndHint();

}
