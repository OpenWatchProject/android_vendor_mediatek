/*
 * Copyright (C) 2015 The Android Open Source Project
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

package com.mediatek.camera.util;

import android.os.Environment;
import android.os.Trace;
import android.util.Log;

import java.io.File;
import java.util.HashMap;

/**
 * This class tracks the timing of important state changes
 *  in camera app (e.g launch flow duration, etc). We can then query
 * these values from the instrument tests, which will be helpful
 *  for tracking camera app performance and regression tests.
 */
public class CameraPerformanceTracker {

    public static final boolean ISBEGIN = true;
    public static final boolean ISEND = false;

    public static final String NAME_CAMERA_OPEN = "Open";
    public static final String NAME_CAMERA_START_PREVIEW = "StartPreview";
    public static final String NAME_CAMERA_STOP_PREVIEW = "StopPreview";
    public static final String NAME_CAMERA_RELEASE = "Release";
    public static final String NAME_GET_PARAMETERS = "getParameters";
    public static final String NAME_SET_PARAMETERS = "setParameters";
    public static final String NAME_PHOTO_TAKE_PICTURE = "TakePicture";
    public static final String NAME_PHOTO_APPLY_PARAMETER_FOR_CAPTURE =
            "applyParameterForCapture";
    public static final String NAME_CAMERA_ACTIVITY = "CameraActivity";
    public static final String NAME_CAMERA_ON_CREATE = "CameraOnCreate";
    public static final String NAME_CAMERA_ON_RESUME = "CameraOnResume";
    public static final String NAME_CAMERA_ON_PAUSE = "CameraOnPause";
    public static final String NAME_CAMERA_ON_DESTROY = "CameraOnDestroy";
    public static final String NAME_CAMERA_START_UP = "CameraStartUp";
    public static final String NAME_CAMERA_ON_CONFIG_CHANGE = "OnConfigChange";
    public static final String NAME_CAMERA_ON_ORIENT_CHANGE = "OnOrientChange";
    public static final String NAME_CAMERA_HANDLE_MSG = "handleMessage";
    public static final String NAME_CAMERA_SEND_MSG = "sendMessage";
    public static final String NAME_CAMERA_EXIT_BY_BACK_KEY = "CameraExitByBackKey";
    public static final String NAME_CAMERA_GALLERY_BACK_KEY = "CameraGalleryBackKey";
    public static final String NAME_CAMERA_ACTIVITY_DETAIL = "CameraActivityDetail";
    public static final String NAME_CAMERA_VIEW_OPERATION = "CameraViewOperation";
    public static final String NAME_CAMERA_CREATE_MODULE = "CameraCreateModule";
    public static final String NAME_CAMERA_CREATE_SCREENNAIL = "CreateScreenNail";
    public static final String NAME_CAMERA_PARAMETER_COPY = "CameraParameterCopy";
    public static final String NAME_CAMERA_INIT_VIEW_MANAGER = "InitViewManager";
    public static final String NAME_CAMERA_INIT_OPEN_PROCESS = "InitOpenProcess";
    public static final String NAME_CAMERA_PREVIEW_PRE_READY_OPEN = "CameraPreviewPreReadyOpen";
    public static final String NAME_APPLY_FIRST_PARAMS = "ApplyFirstParameters";
    public static final String NAME_APPLY_SECOND_PARAMS = "ApplySecondParameters";
    public static final String NAME_INIT_CAMERA_PREF = "InitCameraPref";
    public static final String NAME_SET_DISP_ORIENT = "SetDispOrient";
    public static final String NAME_SET_PREVIEW_ASPECT_RATIO = "SetPreviewAspectRatio";
    public static final String NAME_NOTIFY_ORIENT_CHANGED = "NotifyOrientChanged";
    public static final String NAME_SET_PREVIEW_TEXT = "SetPreviewTexture";
    public static final String NAME_RE_INFLATE_VIEW_MGR = "ReInflateViewManager";
    public static final String NAME_UPDATE_SURFACE_TEXTURE = "UpdateSurfaceTexture";
    public static final String NAME_INIT_FOCUS_MGR = "InitFocusManager";
    public static final String NAME_LAYOUT_CHANGE = "onLayoutChange";
    public static final String NAME_SURFACEVIEW_CREATE = "SurfaceViewCreate";
    public static final String NAME_SET_PREVIEW_DISP = "setPreviewDisp";
    public static final String NAME_UPDATE_APP_VIEW = "updateAppView";
    public static final String NAME_RESUME_NOTIFY = "resumeNotify";
    public static final String NAME_SUPER_RESUME = "superResume";
    public static final String NAME_OTHER_DEVICE_CONNECT = "otherDeviceConnect";
    private static final String BEGIN = "->begin";
    private static final String END = "->end";
    private static final String DURATION = "_duration";
    private static final String TAG = "CameraPerformanceTracker";
    private static final String PERFORMANCE_FILE = "/cameraPerformance.txt";
    private static final boolean DEBUG =
            new File(Environment.getExternalStorageDirectory().toString()
                    + PERFORMANCE_FILE).exists();
    private static CameraPerformanceTracker sInstance;

    // Internal tracking time.
    private static HashMap<String, String> mTimeTracker = new HashMap<String, String>();
    private CameraPerformanceTracker() {
        // Private constructor to ensure that it can only be created from within
        // the class.
    }

    /**
     * This gets called when an important state change happens. Based on the type
     * of the event/state change, either we will record the time of the event, or
     * calculate the duration/latency.
     *
     * @param eventType type of a event to track
     */
    public static void onEvent(String fileName, String EventName, boolean isBegin) {
        if (!DEBUG) {
            return;
        }
        if (sInstance == null) {
            sInstance = new CameraPerformanceTracker();
        }
        long currentTime = System.currentTimeMillis();
        if (isBegin) {
            Log.i(fileName, EventName + BEGIN);
            Trace.traceBegin(Trace.TRACE_TAG_VIEW, EventName);
            mTimeTracker.put(EventName, String.valueOf(currentTime));
        } else {
            Log.i(fileName, EventName + END);
            Trace.traceEnd(Trace.TRACE_TAG_VIEW);
            if (mTimeTracker.containsKey(EventName)) {
                Log.i(TAG, EventName + " duration = "
            + (currentTime - Long.parseLong(mTimeTracker.get(EventName))) + "ms");
                mTimeTracker.remove(EventName);
            }
        }
    }
}
