/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.emcamera;

import android.app.AlertDialog;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Point;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Face;
import android.hardware.Camera.FaceDetectionListener;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.Size;
import android.media.AudioAttributes;
import android.media.CamcorderProfile;
import android.media.CameraProfile;
import android.media.RingtoneManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.StatFs;
import android.os.SystemClock;
import android.os.storage.StorageManager;
import android.text.format.DateFormat;
import android.view.GestureDetector;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.OrientationEventListener;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.view.KeyEvent;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

//import com.mediatek.camcorder.CamcorderProfileEx;
import com.mediatek.emcamera.R;
//import com.mediatek.storage.StorageManagerEx;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/** The Camera activity which can preview and take pictures. */
public class Camera extends ActivityBase implements FocusManager.Listener, View.OnTouchListener,
        SurfaceHolder.Callback {

    static final String TAG = "camera";
    static final int AF_MODE_AUTO = 0;
    static final int AF_MODE_BRACKET = 1;
    static final int AF_MODE_FULL_SCAN = 2;
    static final int AF_MODE_THROUGH_FOCUS = 3;
    static final int AF_MODE_CONTINUOUS = 4;
    static final int AF_MODE_TEMP_CALI = 5;// Temp.Calibration

    private static final int MENU_ID_AE = 10;
    private static final int MENU_ID_EV = 11;
    private static final int MENU_ID_STROBE_DUTY = 12;

    private static final int INTERVAL_MS_UPDATE_CAMERA_INFO = 200;
    private static final long INTERVAL_MS_AUTO_CAPTURE = 800;

    // Storage
    private static final long UNAVAILABLE = -1L;
    private static final long PREPARING = -2L;
    private static final long UNKNOWN_SIZE = -3L;
    private static final long FULL_SDCARD = -4L;
    private static final long LOW_STORAGE_THRESHOLD = 50000000;
    private static final long PICTURE_SIZE = 1500000;
    // handler msg
    private static final int FIRST_TIME_INIT = 2;
    private static final int CLEAR_SCREEN_DELAY = 3;
    private static final int CHECK_DISPLAY_ROTATION = 5;
    private static final int UPDATE_STORAGE = 8;
    private static final int PICTURES_SAVING_DONE = 9;
    private static final int EVENT_COMPLETE_CAPTURE = 102;
    private static final int EVENT_START_CAPTURE = 103;
    private static final int EVENT_CAPTURE_ACTION = 106;
    private static final int EVENT_PREVIEW_RAW_DUMP = 107;
    private static final int EVENT_WAITING_DONE = 201;
    private static final int EVENT_ABORT_CAPTURE = 105;
    private static final int MSG_ID_UPDATE_CAMERA_INFO = 301;
    private static final int MSG_ID_SHUTTER_DELAY_DONE = 302;
    private static final int MSG_ID_ENABLE_ABORT_BTN = 303;
    private static final int MSG_ID_CHECK_CAPTURE_STATE = 400;
    private static final int MSG_ID_GIS_CALIBRATION_PROC = 500;
    // The subset of parameters we need to update in setCameraParameters().
    private static final int UPDATE_PARAM_INITIALIZE = 1;
    private static final int UPDATE_PARAM_PREFERENCE = 4;
    private static final int UPDATE_PARAM_ALL = -1;
    // App Eng mode key and value
    private static final String PROPERTY_KEY_CLIENT_APPMODE = "vendor.client.em.appmode";
    private static final String APP_MODE_NAME_MTK_ENG = "MtkEng";
    private static final String PROPERTY_AF_WIN_WIDTH = "vendor.client.em.af_win_width";
    private static final String PROPERTY_AF_WIN_HEIGHT = "vendor.client.em.af_win_height";
    // Used for Advanced Full Scan
    private static final String PROPERTY_AF_AFS_ENABLE = "vendor.client.em.af_advfs.enable";
    private static final String PROPERTY_AF_AFS_REPEAT = "vendor.client.em.af_advfs.rpt";
    private static final String PROPERTY_AE_MGR_ENABLE = "vendor.debug.ae_mgr.enable";
    private static final String PROPERTY_AE_MGR_ISPGAIN = "vendor.debug.ae_mgr.ispgain";
    private static final String PROPERTY_AE_MGR_SENSORGAIN = "vendor.debug.ae_mgr.sensorgain";
    private static final String PROPERTY_AE_MGR_SHUTTER = "vendor.debug.ae_mgr.shutter";
    private static final String PROPERTY_AE_MGR_PREVIEW_UPDATE = "vendor.debug.ae_mgr.preview.update";
    // used for Temp Cali
    private static final String PROPERTY_AF_CALI_FLAG = "vendor.client.em.af_cali_flag";
    // HAL3 sensor mode
    private static final String PROPERTY_SENSOR_MODE = "vendor.debug.cameng.force_sensormode";
    // Raw save mode
    private static final int RAW_SAVE_PREVIEW = 1;
    private static final int RAW_SAVE_CAPTURE = 2;
    private static final int RAW_SAVE_JPEG = 3;
    private static final int RAW_SAVE_VIDEO = 4;
    private static final int RAW_SAVE_SLIM_VIDEO1 = 5;
    private static final int RAW_SAVE_SLIM_VIDEO2 = 6;
    // Isp mode
    private static final String KEY_ISP_MODE = "isp-mode";
    // isp profile & sensor mode
    private static final String KEY_ENG_RAW_OUTPUT_PORT = "raw-output-port";
    private static final String KEY_ENG_ISP_PROFILE = "isp-profile";
    private static final String KEY_RAW_SAVE_MODE = "rawsave-mode";
    // private static final int ISP_MODE_PROCESSED = 0;
    // private static final int ISP_MODE_PURE = 1;
    // Raw path and name
    private static final String KEY_RAW_PATH = "rawfname";
    private static final String KEY_FLASH_MODE = "flash-mode";
    private static final int SCREEN_DELAY = 2 * 60 * 1000;
    private static final long RAW_JPEG_MAX_SIZE = 17 * 1024 * 1024;
    private static final long JPEG_MAX_SIZE = 1 * 1024 * 1024;

    private static final String APP_MODE_NAME_MTK_S3D = "MtkStereo";
    private static final String KEY_ENG_MFLL_SUPPORTED = "eng-mfll-s";
    private static final String KEY_MFB_MODE = "mfb";
    private static final String OFF = "off";
    private static final String KEY_MFB_MODE_MFLL = "mfll";
    private static final String KEY_ENG_MFLL_PICTURE_COUNT = "eng-mfll-pc";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_SUPPORTED = "vrd-mfr-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_ENABLE = "vrd-mfr-e";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_RANGE_LOW = "vrd-mfr-low";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_RANGE_HIGH = "vrd-mfr-high";
    private static final String KEY_VIDEO_SIZE = "video-size";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_RESIZE = "vdr-r";
    private static final String KEY_ENG_AF_FS_DAC_STEP = "focus-fs-dac-step";
    private static final int PREVIEW_DUMP_RESOLUTION_NORMAL = 0;
    private static final int PREVIEW_DUMP_RESOLUTION_CROP = 1;
    private static final int PREVIEW_DUMP_RESOLUTION_RESIZE_TO_2M = 2;
    private static final int PREVIEW_DUMP_RESOLUTION_RESIZE_TO_4K2K = 3;
    private static final int PREVIEW_DUMP_RESOLUTION_CROP_CENTER_2M = 4;
    // verification - awb
    private static final String KEY_ENG_MTK_AWB_SUPPORTED = "mtk-awb-s";
    private static final String KEY_ENG_SENSOR_AWB_SUPPORTED = "sr-awb-s";
    private static final String KEY_ENG_MTK_AWB_ENABLE = "mtk-awb-e";
    private static final String KEY_ENG_SENSOR_AWB_ENABLE = "sr-awb-e";
    // verification - shading
    private static final String KEY_ENG_MTK_SHADING_SUPPORTED = "mtk-shad-s";
    private static final String KEY_ENG_MTK_1TO3_SHADING_SUPPORTED = "mtk-123-shad-s";
    private static final String KEY_ENG_SENSOR_SHADNING_SUPPORTED = "sr-shad-s";
    private static final String KEY_ENG_MTK_SHADING_ENABLE = "mtk-shad-e";
    private static final String KEY_ENG_MTK_1TO3_SHADING_ENABLE = "mtk-123-shad-e";
    private static final String KEY_ENG_SENSOR_SHADNING_ENABLE = "sr-shad-e";

    private static final String KEY_ENG_VIDEO_HDR_SUPPORTED = "vhdr-s";
    private static final String KEY_ENG_VIDEO_HDR = "video-hdr";
    private static final String KEY_ENG_VIDEO_HDR_MODE = "vhdr-m";
    private static final String KEY_ENG_VIDEO_HDR_RATIO = "vhdr-ratio";
    private static final String KEY_ENG_MULTI_NR_SUPPORTED = "mnr-s";
    private static final String KEY_ENG_MULTI_NR_ENABLE = "mnr-e";
    private static final String KEY_ENG_MULTI_NR_TYPE = "mnr-t";

    private static final String KEY_ENG_MANUAL_SHUTTER_SPEED = "m-ss";
    private static final String KEY_ENG_MANUAL_SENSOR_GAIN = "m-sr-g";
    private static final String KEY_ENG_FLASH_CALIBRATION = "flash-cali";
    private static final String KEY_ENG_EV_VALUE = "eng-ev-value";
    private static final String KEY_ENG_EVB_ENABLE = "eng-evb-enable";
    private static final String KEY_ENG_3ADB_FLASH_ENABLE = "eng-3adb-flash-enable";
    private static final String KEY_BRIGHTNESS_VALUE = "brightness_value";
    private static final String KEY_ENG_SHUTTER_RATIO = "shutter-ratio";

    private static final String KEY_ENG_PREVIEW_ISO = "prv-iso";
    public static final String KEY_ENG_CAMERA_MODE = "mtk-cam-mode";
    public static final int KEY_ENG_CAMERA_MODE_NORMAL = 0;
    public static final int KEY_ENG_CAMERA_MODE_MTK_PRV = 2;
    public static final int KEY_ENG_FOCUS_ENG_MODE_NONE = 0;
    public static final int KEY_ENG_FOCUS_ENG_MODE_BRACKET = 1;
    public static final int KEY_ENG_FOCUS_ENG_MODE_FULLSCAN = 2;
    public static final int KEY_ENG_FOCUS_ENG_MODE_FULLSCAN_REPEAT = 3;
    public static final int KEY_ENG_FOCUS_ENG_MODE_REPEAT = 4;
    public static final String KEY_ENG_FOCUS_MODE_MANUAL = "manual";
    public static final String KEY_ENG_FOCUS_MODE_FULLSCAN = "fullscan";

    private static final String KEY_ISOSPEED_MODE = "iso-speed";
    private static final String KEY_EDGE_MODE = "edge";
    private static final String KEY_SATURATION_MODE = "saturation";
    private static final String KEY_CAMERA_MODE = "mtk-cam-mode";
    private static final String KEY_FOCUS_ENG_MODE = "afeng-mode";
    private static final String KEY_FOCUS_ENG_STEP = "afeng-pos";
    private static final String KEY_FOCUS_ENG_MAX_STEP = "afeng-max-focus-step";
    private static final String KEY_FOCUS_ENG_MIN_STEP = "afeng-min-focus-step";
    private static final String KEY_FOCUS_ENG_BEST_STEP = "afeng-best-focus-step";
    private static final String KEY_RAW_DUMP_FLAG = "afeng_raw_dump_flag";
    private static final String KEY_PREVIEW_DUMP_RESOLUTION = "prv-dump-res";

    private static final String KEY_FOCUS_AREAS = "focus-areas";
    private static final String KEY_ENG_AE_ENABLE = "ae-e";
    private static final String KEY_ENG_PREVIEW_SHUTTER_SPEED = "prv-ss";
    private static final String KEY_ENG_PREVIEW_SENSOR_GAIN = "prv-sr-g";
    private static final String KEY_ENG_PREVIEW_ISP_GAIN = "prv-isp-g";
    private static final String KEY_ENG_PREVIEW_AE_INDEX = "prv-ae-i";
    private static final String KEY_ENG_CAPTURE_SENSOR_GAIN = "cap-sr-g";
    private static final String KEY_ENG_CAPTURE_ISP_GAIN = "cap-isp-g";
    private static final String KEY_ENG_CAPTURE_SHUTTER_SPEED = "cap-ss";
    private static final String KEY_ENG_CAPTURE_ISO = "cap-iso";
    private static final String KEY_ENG_FLASH_DUTY_VALUE = "flash-duty-value";
    private static final String KEY_ENG_FLASH_DUTY_MIN = "flash-duty-min";
    private static final String KEY_ENG_FLASH_DUTY_MAX = "flash-duty-max";
    private static final String KEY_ENG_ZSD_ENABLE = "eng-zsd-e";
    private static final String KEY_SENSOR_TYPE = "sensor-type";
    private static final String KEY_ENG_PREVIEW_FPS = "eng-prv-fps";
    private static final String KEY_ENG_MSG = "eng-msg";
    private static final String KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL = "focus-fs-fi";
    public static final String KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MAX = "focus-fs-fi-max";
    public static final String KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL_MIN = "focus-fs-fi-min";
    private static final String KEY_ENG_PREVIEW_FRAME_INTERVAL_IN_US = "eng-prv-fius";
    private static final String KEY_ENG_PARAMETER1 = "eng-p1";
    private static final String KEY_ENG_PARAMETER2 = "eng-p2";
    private static final String KEY_ENG_PARAMETER3 = "eng-p3";
    private static final String KEY_ENG_SAVE_SHADING_TABLE = "eng-s-shad-t";
    private static final String KEY_ENG_SHADING_TABLE = "eng-shad-t";
    private static final String KEY_ENG_EV_CALBRATION_OFFSET_VALUE = "ev-cal-o";
    private static final String KEY_ENG_MFLL_ENABLE = "eng-mfll-e";
    private static final String KEY_ENG_SENOSR_MODE_SLIM_VIDEO1_SUPPORTED = "sv1-s";
    private static final String KEY_ENG_SENOSR_MODE_SLIM_VIDEO2_SUPPORTED = "sv2-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_RESIZE_TO_2M_SUPPORTED = "vdr-r2m-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_RESIZE_TO_4K2K_SUPPORTED = "vdr-r4k2k-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_CROP_CENTER_2M_SUPPORTED = "vdr-cc2m-s";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_MIN = "vrd-mfr-min";
    private static final String KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_MAX = "vrd-mfr-max";
    private static final String KEY_ENG_MTK_1to3_SHADING_SUPPORTED = "mtk-123-shad-s";
    private static final String KEY_ENG_MTK_1to3_SHADING_ENABLE = "mtk-123-shad-e";

    private static final String TRUE = "true";
    private static final String FALSE = "false";

    private static final String[] SCENE_STRS_ARRAY = { "auto", "night", "sunset", "party",
            "portrait", "landscape", "night-portrait", "theatre", "beach", "snow", "steadyphoto",
            "fireworks", "sports", "candlelight", "hdr" };

    private static final String NOTIFICATION_CHANNEL_ID = "emcamera_channel_id";
    private static final CharSequence NOTIFICATION_NAME = "EmCamera";
    private static final int SCENE_MODE_AUTO = 0;
    private static final int SCENE_MODE_NIGHT = 1;
    private static final int SCENE_MODE_SUNSET = 2;
    private static final int SCENE_MODE_PARTY = 3;
    private static final int SCENE_MODE_PORT = 4;
    private static final int SCENE_MODE_LAND = 5;
    private static final int SCENE_MODE_NIGHTPORT = 6;
    private static final int SCENE_MODE_THEATRE = 7;
    private static final int SCENE_MODE_BEACH = 8;
    private static final int SCENE_MODE_SNOW = 9;
    private static final int SCENE_MODE_STEADY = 10;
    private static final int SCENE_MODE_FIREWORKS = 11;
    private static final int SCENE_MODE_SPORTS = 12;
    private static final int SCENE_MODE_CANDLE = 13;
    public static final int SCENE_MODE_HDR = 14;

    private Parameters mParameters;
    private Parameters mInitialParams;
    private boolean mFocusAreaSupported;
    private boolean mMeteringAreaSupported;
    private boolean mAeLockSupported;
    private boolean mAwbLockSupported;

    private MyOrientationEventListener mOrientationListener;
    // The degrees of the device rotated clockwise from its natural orientation.
    private int mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
    // The orientation compensation for icons and thumbnails. Ex: if the value
    // is 90, the UI components should be rotated 90 degrees counter-clockwise.
    private int mOrientationCompensation = 0;

    private SurfaceHolder mSurfaceHolder = null;
    private Button mShutterButton;
    private boolean mShutterButtonEnabled;
    // Only abort long exporsure
    private Button mAbortButton;
    private GestureDetector mPopupGestureDetector;

    private View mPreviewPanel; // The container of PreviewFrameLayout.
    private PreviewFrameLayout mPreviewFrameLayout;
    private View mPreviewFrame; // Preview frame area.
    // private FaceView mFaceView;
    private RotateLayout mFocusAreaIndicator;

    // A view group that contains all the small indicators.
    private Rotatable mOnScreenIndicators;

    // The display rotation in degrees. This is only valid when mCameraState is
    // not PREVIEW_STOPPED.
    private int mDisplayRotation;
    // The value for android.hardware.Camera.setDisplayOrientation.
    private int mDisplayOrientation;
    private boolean mPausing;
    private boolean mFirstTimeInitialized;

    private static final int PREVIEW_STOPPED = 0;
    private static final int IDLE = 1; // preview is active
    // Focus is in progress. The exact focus state is in Focus.java.
    private static final int FOCUSING = 2;
    private static final int SNAPSHOT_IN_PROGRESS = 3;
    private static final int SELFTIMER_COUNTING = 4;
    private static final int SAVING_PICTURES = 5;
    private int mCameraState = PREVIEW_STOPPED;
    private boolean mSnapshotOnIdle = false;
    private boolean mDidRegister = false;

    private final ShutterCallback mShutterCallback = new ShutterCallback();
    private final RawPictureCallback mRawPictureCallback = new RawPictureCallback();
    private final AutoFocusCallback mAutoFocusCallback = new AutoFocusCallback();
    private final CameraErrorCallback mErrorCallback = new CameraErrorCallback();

    private static final int CAMERA_ERROR_CALI_FLASH = 1002;
    private long mFocusStartTime;
    private long mCaptureStartTime;
    private long mShutterCallbackTime;
    private long mRawPictureCallbackTime;
    private long mOnResumeTime;
    private long mPicturesRemaining;
    private byte[] mJpegImageData;

    // These latency time are for the CameraLatency test.
    private long mAutoFocusTime;
    private long mShutterLag;

    // This handles everything about focus.
    private FocusManager mFocusManager;

    private final Handler mHandler = new MainHandler();
    private int mCameraId;

    private boolean mSettingRetrieved = false;
    private boolean mIsAutoFocusCallback = false;
    // new
    // private int mIso = 0;
    private int mCaptureMode = 0;
    private int mCaptureSize = 0;
    private int mCaptureRatio = 0;
    private String[] mCaptureNamePrefix;
    private int mCaptureType = 0;
    private int mCaptureNumber = 1;
    private int mVideoResolution = 0;
    private String mFlickerString = "50hz";
    private int mShutterSpeed = 0;
    private int mSensorGain = 0;
    private int ispProfile = 0;
    private int sensorMode = 0;
    private int videoDumpBuffer = 0;
    // Use for 2nd MP(JB2)
    private String mStrobeMode = "auto";
    private int mPreFlashLevel = 1;
    private int mMainFlashLevel = 1;
    private int mAfMode = AF_MODE_AUTO;
    private int mAfBracketRange = 0;
    private int mAfBracketIntervel = 1;
    private int mAfThroughIntervel = 1;
    private int mAfThroughDirect = 0;
    // Through focus: Manual configure
    private int mThroughFocusStartPos = 0;
    private int mThroughFocusEndPos = 1023;
    private int mAfCaptureTimes = 0;
    private int mTripleCount = 0;
    private String mIsoValue = "";
    private ArrayList<String> mIsoValues = null;
    private String mRawCaptureFileName;
    private boolean mTakePicDone = true;
    private StorageManager mStorageManager = null;
    private String mCameraImageName = "";    // For GIS calibration
    public static final String GIS_DIR = "data/vendor/camera_dump/eis/gyro/";
    private static final String GIS_INFO_FILE = GIS_DIR + "CalibrationInfo.txt";
    private static final String GIS_PARAM_FILE = GIS_DIR + "GIS_default_parameter.txt";
    private static final String GIS_PARAM_MAIN_FILE = GIS_DIR + "GIS_default_parameter_main.txt";
    private static final String GIS_PARAM_SUB_FILE = GIS_DIR + "GIS_default_parameter_sub.txt";
    private static final String GIS_PARAM_MAIN2_FILE = GIS_DIR + "GIS_default_parameter_main2.txt";
    private static final String GIS_PARAM_SUB2_FILE = GIS_DIR + "GIS_default_parameter_sub2.txt";
    private static final String ROPERTY_GIS_CALIBRATION = "vendor.debug.eis.EMEnabled";
    private boolean mEnableGisCalibration = false;
    // For AF bracket and Through focus
    private int mBracketBestPos;
    private int mBracketMaxPos;
    private int mBracketMinPos;
    private int mPosValue; // real position
    private int mBracketTimes;
    private boolean mIsBracketAddPos = true; // if true, capture from best
                                             // position to max position or from
                                             // min to max
    private int mAfFullScanFrameInterval;
    private int mAfFullScanDacStep;
    private int mAfFullWinWidth = -1;
    private int mAfFullWinHeight = -1;
    private boolean mAfAdvFullScan = false;
    private int mAfAdvFullRepeat;
    private int mAfAdvFullFocusTimes = 0;
    private int mAfAdvFullSensorGain;
    private int mAfAdvFullIspGain;
    private String[] mAfAdvFullShutter;
    private static final String[] AF_ADV_60HZ_SHUTTER_ARRAY = { "8333", "16667", "25000", "33334" };
    private static final String[] AF_ADV_50HZ_SHUTTER_ARRAY = { "10000", "20000", "30000", "40000" };
    private static final int LONG_EXPOSURE_TIME = 400;
    private int mAfThroughRepeat;
    private int mAfThroughRepearPointer;
    private boolean mIsNeedWait;
    private TextView mTvCameraInfo;
    private int mCaptureDoneNumber = 0;
    private boolean mEnableAeAwbLock = true;
    private String mStrobeDuty = "-1";
    private boolean mManualLockAe = false;
    private String mCustomParameters = null;
    private int mShutterDelay = 0;
    private boolean mEnableShutterDelay;
    private int mShadingTable;
    private int mOutputSdblk;
    private int mIsoValIndex;
    private int mEvCalibration = 0;
    private int mGisCalibration = 0;
    // video hdr & Multi pass nr
    private int mVideoHdr = 0;
    private String mVHdrMode;
    private int mVHdrRatio;
    private int mMultiPass = 0;

    private int mMfllMode = 0;
    private int mMfllCount = 0;
    private int mEnableFameRate = 0;
    private int mHighFrameRate = 0;
    private int mLowFrameRate = 0;
    private int mAwbVeri = 0;
    private int mShadingVeri = 0;

    private boolean mMnrSupport = false;
    private int mSceneMode = 0;
    private int mStereoCameraId = 0;
    private int mFlashCali = 0;
    private boolean mFlashCaliError = false;
    private FaceView mFaceView = null;
    private boolean mFaceDetectionEnabled = false;
    private Object mFaceDetectionSync = new Object();
    private boolean mFaceDetectionRunning = false;
    private AeSettings aeSettings = new AeSettings();
    private int mMinFps;
    private int mMaxFps;

    private static final String FILE_PATH = "data/vendor/camera_dump/CameraEM/";

    // 3A database
    private enum DB3A {
        DB3A_OFF,
        DB3A_AE,
        DB3A_AWB,
        DB3A_AF,
        DB3A_FLASH_AE,
        DB3A_FLASH_AWB,
        DB3A_FLASH
    }
    private interface  DB3AState {
         boolean isStart();
         boolean isEnd();
         boolean setStart(int s);
         boolean setEnd(int e);
         boolean move2NextState();
         boolean setStep(int s);
         boolean setState(int s);
         int     getCurrentState();
         int     getNextState();
         boolean reset();
    }

    private DB3A mDBMode = DB3A.DB3A_OFF;
    // 3A - FLASH
    private int mFlashCount = 0;
    private int mTempFlashCount = 0;
    private String mTempRawCaptureFileName;
    private String mTempCameraImageName;
    private int mSaveCorrectFlag = 1;

    // 3A - AF
    private static String AF_TESTSCENE = "default";
    private static String AF_TIMESTAMP = "";
    private static boolean AF_IS_THROUGH_FOCUS = false;
    private static int AF_EVCOMPENSATE = 0;
    private static String AF_EVSTEPNUM = "";
    private static int AF_EVSTEPSIZE = 0;
    // 3A - AWB
    private String DB3A_AWBTestItem = "001001";
    private String DB3A_AWBTestItemSub = "001";

    private int[] mAWBEVComp = { 0, -100, 100 };
    private int mAWBEVCompIndex = 0;
    private int mAWBTEMPIndex;
    private int mAWBRound = 0;

    // 3A - AE
    private String DB3A_AETestItem = "001001";
    private String DB3A_AETestItemSub = "001";

    class DB3A_AEState implements DB3AState {
        private int i4AE_EVComp;
        private int i4AE_EVStart; // start must be < end
        private int i4AE_EVEnd;
        private int i4AE_Step;

        public DB3A_AEState() {
            i4AE_EVStart = -210;
            i4AE_EVEnd = 200;
            i4AE_Step = 10;
            i4AE_EVComp = i4AE_EVStart;
        }

        @Override
        public boolean isStart() {
            return i4AE_EVComp == i4AE_EVStart;
        }

        @Override
        public boolean isEnd() {
            return i4AE_EVComp == i4AE_EVEnd;
        }

        @Override
        public boolean setStart(int s) {
            i4AE_EVStart = s;
            reset();
            return true;
        }

        @Override
        public boolean setEnd(int e) {
            i4AE_EVEnd = e;
            return true;
        }

        @Override
        public boolean move2NextState() {
            i4AE_EVComp += i4AE_Step;
            if (i4AE_EVComp > i4AE_EVEnd) {
                i4AE_EVComp = i4AE_EVEnd;
                return false;
            }
            if (i4AE_EVComp < i4AE_EVStart) {
                i4AE_EVComp = i4AE_EVStart;
                return false;
            }
            return true;
        }

        @Override
        public boolean setStep(int s) {
            i4AE_Step = s;
            return true;
        }

        @Override
        public boolean setState(int s) {
            if (s >= i4AE_EVStart && s <= i4AE_EVEnd) {
                i4AE_EVComp = s;
                return true;
            } else {
                return false;
            }
        }

        @Override
        public int getCurrentState() {
            Elog.i(TAG, "CurrentEV:" + i4AE_EVComp);
            return i4AE_EVComp;
        }

        @Override
        public int getNextState() {
            int newState = i4AE_EVComp + i4AE_Step;
            return (newState > i4AE_EVEnd ? i4AE_EVEnd : ((newState < i4AE_EVStart) ? i4AE_EVStart
                    : newState));
        }

        @Override
        public boolean reset() {
            i4AE_EVComp = i4AE_EVStart;
            return true;
        }
    }

    private DB3A_AEState mDBAEState;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Elog.i(TAG, "Received intent action=" + action);
            if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {
                checkStorage();
            } else if (action.equals(Intent.ACTION_MEDIA_MOUNTED)
                    || action.equals(Intent.ACTION_MEDIA_CHECKING)) {
                checkStorage();
            } else if (action.equals(Intent.ACTION_MEDIA_SCANNER_FINISHED)) {
                checkStorage();
            }
        }
    };

    private Thread mCameraPreviewThread = new Thread(new Runnable() {
        @Override
        public void run() {
            Elog.i(TAG, "mCameraPreviewThread start");
            initializeCapabilities();
            startPreview();
            Elog.i(TAG, "mCameraPreviewThread end");
        }
    });

    private boolean handleBracketCapture() {
        if (mBracketTimes != 0) { // Auto is best step
            if (mBracketTimes == 1 && mIsBracketAddPos == true) {
                mBracketBestPos = getParameterValue(KEY_FOCUS_ENG_BEST_STEP);
                mBracketMaxPos = getParameterValue(KEY_FOCUS_ENG_MAX_STEP);
                mBracketMinPos = getParameterValue(KEY_FOCUS_ENG_MIN_STEP);
                Elog.i(TAG, "bracket max " + mBracketMaxPos + " bracket min " + mBracketMinPos
                        + " bracket best " + mBracketBestPos);
            }
            if (mIsBracketAddPos) {
                mPosValue = mBracketBestPos + mAfBracketIntervel * mBracketTimes;
                if (mPosValue > mBracketMaxPos) {
                    mIsBracketAddPos = false;
                    mBracketTimes = 1;
                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                    Elog.i(TAG, "bracket add pos capture done");
                    return false;
                }
            } else {
                mPosValue = mBracketBestPos - mAfBracketIntervel * mBracketTimes;
                if (mPosValue < mBracketMinPos || mBracketTimes > mAfBracketRange) {
                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                    return false;
                }
            }
            Elog.i(TAG, "bracket position " + mPosValue);
            mParameters.set(KEY_FOCUS_ENG_STEP, mPosValue);
            mCameraDevice.setParameters(mParameters);
            try { // set step need wait 100ms before capture
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        mBracketTimes++;
        return true;
    }

    private boolean handleThroughFocusCapture() {
        if (mBracketTimes == 0) {
            // 3A - AF
            AF_IS_THROUGH_FOCUS = true;
            AF_TIMESTAMP = createName(System.currentTimeMillis());
            getAvailableSpace();
            setEmCameraParam();

            mBracketMaxPos = getParameterValue(KEY_FOCUS_ENG_MAX_STEP);
            mBracketMinPos = getParameterValue(KEY_FOCUS_ENG_MIN_STEP);
            Elog.i(TAG, "through max " + mBracketMaxPos + " through min " + mBracketMinPos);
            if (mAfThroughDirect == 0) {
                mPosValue = mBracketMaxPos;
                mIsBracketAddPos = false;
            } else if (mAfThroughDirect == 1) {
                mPosValue = mBracketMinPos;
                mIsBracketAddPos = true;
            } else { // Manual configure
                if (mThroughFocusStartPos <= mThroughFocusEndPos) {
                    mIsBracketAddPos = true;
                    if (mThroughFocusStartPos >= mBracketMinPos) {
                        mPosValue = mThroughFocusStartPos;
                    } else {
                        mPosValue = mBracketMinPos;
                    }
                    if (mThroughFocusEndPos <= mBracketMaxPos) {
                        mBracketMaxPos = mThroughFocusEndPos;
                    }
                } else if (mThroughFocusStartPos > mThroughFocusEndPos) {
                    mIsBracketAddPos = false;
                    if (mThroughFocusStartPos <= mBracketMaxPos) {
                        mPosValue = mThroughFocusStartPos;
                    } else {
                        mPosValue = mBracketMaxPos;
                    }
                    if (mThroughFocusEndPos >= mBracketMinPos) {
                        mBracketMinPos = mThroughFocusEndPos;
                    }
                }
            }
        } else {
            if (mIsBracketAddPos) {
                // mPosValue = mBracketMinPos + mAfThroughIntervel * mBracketTimes;
                if (mAfThroughRepearPointer == mAfThroughRepeat) {
                    mPosValue = mPosValue + mAfThroughIntervel;
                    mAfThroughRepearPointer = 0;
                }
                if (mPosValue > mBracketMaxPos) {
                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                    // 3A - AF
                    AF_IS_THROUGH_FOCUS = false;
                    if (mAfThroughRepeat > 1 && AF_EVSTEPSIZE > 0) {
                        mParameters.setExposureCompensation(0);
                        mCameraDevice.setParameters(mParameters);
                    }
                    return false;
                }
            } else {
                // mPosValue = mBracketMaxPos - mAfThroughIntervel * mBracketTimes;
                if (mAfThroughRepearPointer == mAfThroughRepeat) {
                    mPosValue = mPosValue - mAfThroughIntervel;
                    mAfThroughRepearPointer = 0;
                }
                if (mPosValue < mBracketMinPos) {
                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                    // 3A - AF
                    AF_IS_THROUGH_FOCUS = false;
                    if (mAfThroughRepeat > 1 && AF_EVSTEPSIZE > 0) {
                        mParameters.setExposureCompensation(0);
                        mCameraDevice.setParameters(mParameters);
                    }
                    return false;
                }
            }
        }
        Elog.i(TAG, "through position " + mPosValue);
        // 3A - AF
        if (mDBMode == DB3A.DB3A_AF) {
            if (mAfThroughRepeat > 1 && AF_EVSTEPSIZE > 0) {
                AF_EVCOMPENSATE = (-1 * (mAfThroughRepeat >> 1) + mAfThroughRepearPointer)
                        * AF_EVSTEPSIZE * 10;
                Elog.i(TAG, "af ev compensate " + AF_EVCOMPENSATE);
            }
            mRawCaptureFileName = mRawCaptureFileName + "_STEP" + Integer.toString(mPosValue);
            if (mAfThroughRepeat > 1 && AF_EVSTEPSIZE > 0) {
                mRawCaptureFileName += "X" + Integer.toString(AF_EVCOMPENSATE) + "EV";
            }
            mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
            Elog.i(TAG, "update: 3ADB::AF setEmCameraParam mRawCaptureFileName "
                    + mRawCaptureFileName);
        }
        mParameters.set(KEY_FOCUS_ENG_STEP, mPosValue);
        mCameraDevice.setParameters(mParameters);
        try { // set step need wait 100ms before capture
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        mAfThroughRepearPointer++;
        mBracketTimes++;
        return true;
    }

    private void overrideEngParameter() {
        Elog.i(TAG, "3ADB mode:" + mDBMode);
        // restore to default
        mParameters.set(KEY_ENG_PARAMETER1, "AE");
        mParameters.set(KEY_ENG_PARAMETER2, "AE_CMD_FINEREVCOMP_ONOFF");
        mParameters.set(KEY_ENG_PARAMETER3, Integer.toString(0));
        mCameraDevice.setParameters(mParameters);

        // Todo: set other values to em parameters
        mParameters.set(KEY_ENG_PARAMETER1, "em parameter 1");
        mParameters.set(KEY_ENG_PARAMETER2, "em parameter 2");
        mParameters.set(KEY_ENG_PARAMETER3, "em parameter 3");

        String evComp;
        switch (mDBMode) {
        case DB3A_AE:
            mParameters.set(KEY_FLASH_MODE, "off");
            mParameters.set(KEY_ENG_PARAMETER1, "AE");
            mParameters.set(KEY_ENG_PARAMETER2, "AE_CMD_FINEREVCOMP_ONOFF");
            mParameters.set(KEY_ENG_PARAMETER3, Integer.toString(1));
            mCameraDevice.setParameters(mParameters);
            mParameters.set(KEY_ENG_PARAMETER2, "AE_CMD_FINEREVCOMP");
            // mParameters.setEngParameter3(Integer.toString(DBAEState.GetCurrentState()));
            mParameters.set(KEY_ENG_EV_VALUE, Integer.toString(mDBAEState.getCurrentState()));
            mParameters.set(KEY_ENG_EVB_ENABLE, Integer.toString(1));
            evComp = Integer.toString(mDBAEState.getCurrentState()) + "EV";
            mRawCaptureFileName = mRawCaptureFileName + "X" + (0 - aeSettings.getMinus())
                    + aeSettings.getPlus() + "X" + evComp;
            mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
            mCameraDevice.setParameters(mParameters);
            break;
        case DB3A_AF:
            // 3A - AF
            if (mAfThroughRepeat > 1 && AF_EVSTEPSIZE > 0) {
                mParameters.set(KEY_FLASH_MODE, "off");
                mParameters.set(KEY_ENG_PARAMETER1, "AE");
                mParameters.set(KEY_ENG_PARAMETER2, "AE_CMD_FINEREVCOMP_ONOFF");
                mParameters.set(KEY_ENG_PARAMETER3, Integer.toString(1));
                mCameraDevice.setParameters(mParameters);
                mParameters.set(KEY_ENG_PARAMETER2, "AE_CMD_FINEREVCOMP");
                // mParameters.setEngParameter3(Integer.toString(AF_EVCOMPENSATE));
                mParameters.set(KEY_ENG_EV_VALUE, Integer.toString(AF_EVCOMPENSATE));
                mParameters.set(KEY_ENG_EVB_ENABLE, Integer.toString(1));
                mCameraDevice.setParameters(mParameters);
                // mParameters.setExposureCompensation(AF_EVCOMPENSATE);
                // mCameraDevice.setParameters(mParameters);
            } else {
                Elog.i(TAG, "non-3ADB mode:" + mDBMode);
            }
            break;
        case DB3A_AWB:
            mParameters.set(KEY_FLASH_MODE, "off");
            mParameters.set(KEY_ENG_PARAMETER1, "AE");
            mParameters.set(KEY_ENG_PARAMETER2, "AE_CMD_FINEREVCOMP_ONOFF");
            mParameters.set(KEY_ENG_PARAMETER3, Integer.toString(1));
            mCameraDevice.setParameters(mParameters);
            mParameters.set(KEY_ENG_PARAMETER2, "AE_CMD_FINEREVCOMP");
            // mParameters.setEngParameter3(Integer.toString(AWB_EVCOMP[AWB_EVCOMP_Index]));
            mParameters.set(KEY_ENG_EV_VALUE, Integer.toString(mAWBEVComp[mAWBEVCompIndex]));
            mParameters.set(KEY_ENG_EVB_ENABLE, Integer.toString(1));
            evComp = Integer.toString(mAWBEVComp[mAWBEVCompIndex]) + "EV";
            mRawCaptureFileName = mRawCaptureFileName + "X" + DB3A_AWBTestItem
                    + DB3A_AWBTestItemSub + "X" + evComp;
            mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
            mCameraDevice.setParameters(mParameters);
            mAWBEVCompIndex++;

            if (mAWBEVCompIndex == 3) {
                mAWBRound = 1;
            }
            break;
        case DB3A_FLASH:
            // flash on/off saved in the same folder.
            File dir = new File(mTempCameraImageName);
            mkdirs(dir);
            mParameters.set(KEY_ENG_3ADB_FLASH_ENABLE, Integer.toString(1));
            mCameraDevice.setParameters(mParameters);
            break;
        case DB3A_FLASH_AE:
            break;
        case DB3A_FLASH_AWB:
            break;
        default:
            break;
        }
        Elog.i(TAG, "3ADB setEmCameraParam mRawCaptureFileName " + mRawCaptureFileName);
    }

    /**
     * This Handler is used to post message back onto the main thread of the application.
     *
     */
    private class MainHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            // Elog.i(TAG, "MainHandler msg.what " + msg.what);
            switch (msg.what) {
            case CLEAR_SCREEN_DELAY:
                getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                break;
            case FIRST_TIME_INIT:
                initializeFirstTime();
                break;
            case CHECK_DISPLAY_ROTATION:
                if (Util.getDisplayRotation(Camera.this) != mDisplayRotation) {
                    setDisplayOrientation();
                }
                if (SystemClock.uptimeMillis() - mOnResumeTime < 5000) {
                    mHandler.sendEmptyMessageDelayed(CHECK_DISPLAY_ROTATION, 100);
                }
                break;
            case UPDATE_STORAGE:
                checkStorage();
                break;
            case PICTURES_SAVING_DONE:
                if (!mPausing) {
                    if (mFlashCali == 1 && !mFlashCaliError) {
                        showCaliDialog("Succuss", "Calibration result is correct");
                    }
                    if (mCameraState == SAVING_PICTURES) {
                        Elog.i(TAG, "==== PICTURES_SAVING_DONE ====");
                        setCameraState(IDLE);
                        checkStorage();
                        mFocusManager.capture();
                        onShutterButtonFocus(false);

                        if (mAfMode == AF_MODE_AUTO) {
                            mTripleCount++;
                            if (mTripleCount == mCaptureNumber) {
                                mTripleCount = 0;
                                mAfCaptureTimes++;
                            }
                            Elog.i(TAG, "mTripleCount " + mTripleCount + " mAfCaptureTimes "
                                    + mAfCaptureTimes);
                            try {
                                Thread.sleep(200);//avoid picture naming issue
                            } catch (InterruptedException e) {
                                Elog.e(TAG, e.getMessage());
                            }
                        }
                        startPreview();
                    }
                }
                break;
            case EVENT_COMPLETE_CAPTURE:
                Elog.i(TAG, "==== EVENT_COMPLETE_CAPTURE ====");
                mShutterButton.setEnabled(true);
                mShutterButtonEnabled = true;
                if(mShutterSpeed > LONG_EXPOSURE_TIME) {
                    Elog.i(TAG, "capture done, disable abort button");
                    mAbortButton.setEnabled(false);
                }
                mAfCaptureTimes = 0;
                mTripleCount = 0;
                // Init For next capture
                mBracketTimes = 0;
                mIsBracketAddPos = true;
                if (isLockAeAwbAfMode(mAfMode) && !mEnableAeAwbLock) {
                    mEnableAeAwbLock = true;
                    mFocusManager.setAeAwbLock(false);
                }
                if (mAfMode == AF_MODE_FULL_SCAN) {
                    if (mAfAdvFullScan) {
                        sendNotification("EM Camera", "Advanced full scan Done");
                    }
                } else if (mAfMode == AF_MODE_TEMP_CALI) {
                    sendNotification("EM Camera", "Capture complete!");
                }
                startPreview();
                Elog.i(TAG, "Enabled mCaptureBtn");
                break;
            case EVENT_START_CAPTURE:
                mShutterButton.setEnabled(false);
                mShutterButtonEnabled = false;
                Elog.i(TAG, "Disabled mCaptureBtn");
                resetPrivateParams();
                break;
            case EVENT_CAPTURE_ACTION:
                Elog.i(TAG, "==== EVENT_CAPTURE_ACTION ====");
                if (mCameraDevice == null || mPausing) {
                    Elog.i(TAG, "EVENT_CAPTURE_ACTION return");
                    return;
                }

                if (mCameraState == SNAPSHOT_IN_PROGRESS || mCameraDevice == null || mPausing) {
                    Elog.i(TAG, "SNAPSHOT_IN_PROGRESS, EVENT_CAPTURE_ACTION later");
                    sendEmptyMessageDelayed(EVENT_CAPTURE_ACTION, 500);
                    return;
                }

                setIsFocusCallback(false);

                if (!isStorageEnough4Capture()) {
                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                    Toast.makeText(Camera.this, "storage space is not enough or unavailable",
                            Toast.LENGTH_LONG).show();
                    Elog.i(TAG, "[FOR_NATA_CAMERA_NO_AVAILABLE_STORAGE]");
                    return;
                }
                Elog.i(TAG, "mEnableShutterDelay:" + mEnableShutterDelay + " mShutterDelay:"
                        + mShutterDelay);
                if (mEnableShutterDelay && mShutterDelay > 0) {
                    sendEmptyMessageDelayed(MSG_ID_SHUTTER_DELAY_DONE, mShutterDelay * 1000);
                    return;
                }

                mParameters = mCameraDevice.getParameters();
                if (mAfMode == AF_MODE_BRACKET) {
                    if (handleBracketCapture() == false) {
                        return;
                    }
                }
                if (mAfMode == AF_MODE_THROUGH_FOCUS) {
                    if (handleThroughFocusCapture() == false) {
                        return;
                    }
                }
                // override setting
                // parsing DBMode from input 1/2/3
                if (mDBMode != DB3A.DB3A_OFF) {
                    overrideEngParameter();
                }

                capture();
                break;
            case EVENT_ABORT_CAPTURE:
                mCameraDevice.stopPreview();//stop capture
                if (!mTakePicDone) {
                    mTakePicDone = true;
                }
                Elog.i(TAG, "capture aborted");
                mHandler.removeMessages(MSG_ID_CHECK_CAPTURE_STATE);
                break;
            case EVENT_PREVIEW_RAW_DUMP:
                Elog.i(TAG, "EVENT_PREVIEW_RAW_DUMP");
                onShutterButtonFocus(true);
                break;
            case EVENT_WAITING_DONE:
                // Elog.i(TAG, "EVENT_WAITING_DONE: at:" + System.currentTimeMillis());
                // mIsNeedWait = false;
                // sendEmptyMessage(EVENT_CAPTURE_ACTION);
                break;
            case MSG_ID_ENABLE_ABORT_BTN:
                if(mShutterSpeed > LONG_EXPOSURE_TIME) {
                    Elog.i(TAG, "enable abort button");
                    mAbortButton.setEnabled(true);
                }
                break;
            case MSG_ID_UPDATE_CAMERA_INFO:
                if (!mPausing) {
                    if (mCameraState != SNAPSHOT_IN_PROGRESS) {
                        updateUiCameraInfo();
                    }
                    sendEmptyMessageDelayed(MSG_ID_UPDATE_CAMERA_INFO,
                            INTERVAL_MS_UPDATE_CAMERA_INFO);
                }
                break;
            case MSG_ID_SHUTTER_DELAY_DONE:
                if (!mPausing) {
                    mEnableShutterDelay = false;
                    sendEmptyMessage(EVENT_CAPTURE_ACTION);
                }
                break;
            case MSG_ID_CHECK_CAPTURE_STATE:
                if (!mPausing && mShutterButtonEnabled) {
                    switch (mDBMode) {
                    case DB3A_AE: // 3A - AE
                        if (mDBAEState.isEnd()) {
                            mDBAEState.reset();
                            mFocusManager.overrideFocusMode(Parameters.FOCUS_MODE_AUTO);
                            return;
                        }
                        if (!mDBAEState.isStart()) {
                            mFocusManager.overrideFocusMode(KEY_ENG_FOCUS_MODE_MANUAL);
                        }
                        mDBAEState.move2NextState();
                        updateUiCameraInfo();
                        mHandler.sendEmptyMessage(EVENT_START_CAPTURE);
                        mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                        Elog.i(TAG, "AE MSG_ID_CHECK_CAPTURE_STATE");
                        break;
                    case DB3A_AWB: // 3A - AWB
                        if (mAWBRound == 1) {
                            mAWBEVCompIndex = 0;
                            mAWBRound = 0;
                            mFocusManager.overrideFocusMode(Parameters.FOCUS_MODE_AUTO);
                            return;
                        }
                        if (mAWBEVCompIndex == 0) {
                            mFocusManager.overrideFocusMode(Parameters.FOCUS_MODE_AUTO);
                        } else {
                            mFocusManager.overrideFocusMode(KEY_ENG_FOCUS_MODE_MANUAL);
                        }
                        mHandler.sendEmptyMessage(EVENT_START_CAPTURE);
                        mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                        Elog.i(TAG, "AWB MSG_ID_CHECK_CAPTURE_STATE");
                        break;
                    case DB3A_FLASH:
                        mHandler.sendEmptyMessage(EVENT_START_CAPTURE);
                        mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                        return;
                    case DB3A_AF:
                        break;
                    case DB3A_FLASH_AE:
                        break;
                    case DB3A_FLASH_AWB:
                        break;
                    default:
                        break;
                    }
                }
                sendEmptyMessageDelayed(MSG_ID_CHECK_CAPTURE_STATE, 1000);
                break;
            case MSG_ID_GIS_CALIBRATION_PROC:
                Elog.i(TAG, "MSG_ID_GIS_CALIBRATION_PROC");
                cameraGisCalProc();
                break;
            default:
                break;
            }
        }
    }

    private void updateUiCameraInfo() {
        if (mDBMode != DB3A.DB3A_OFF) {
            String info;
            switch (mDBMode) {
            case DB3A_AE:
                // 3A - AE
                info = String.format("[AE] Item: %04d %03d %03d, EV %d",
                        (0 - aeSettings.getMinus()), aeSettings.getPlus(),
                        aeSettings.getInterval(), mDBAEState.getCurrentState());
                mTvCameraInfo.setText(info);
                break;
            case DB3A_AF:
                // 3A - AF
                String afInfo = "[AF MODE] ";
                if (AF_IS_THROUGH_FOCUS && mBracketMinPos <= mPosValue
                        && mPosValue <= mBracketMaxPos) {
                    afInfo += String.format("STEP %d", mPosValue);
                }
                if (AF_IS_THROUGH_FOCUS && mAfThroughRepeat > 1 && AF_EVSTEPSIZE > 0) {
                    afInfo += String.format(" (%dEV)", AF_EVCOMPENSATE);
                }
                mTvCameraInfo.setText(afInfo);
                break;
            case DB3A_AWB: // 3A - AWB
                mAWBTEMPIndex = mAWBEVCompIndex - 1;
                if ((mAWBTEMPIndex < 0) || (mAWBTEMPIndex > 2)) {
                    mAWBTEMPIndex = 0;
                }
                info = String.format("[AWB] Item: %s, EV %d", DB3A_AWBTestItem,
                        mAWBEVComp[mAWBTEMPIndex]);
                mTvCameraInfo.setText(info);
                break;
            case DB3A_FLASH_AE:
                break;
            case DB3A_FLASH_AWB:
                break;
            }
        } else {
            mParameters = mCameraDevice.getParameters();
            float previewFPS = getParameterValue(KEY_ENG_PREVIEW_FRAME_INTERVAL_IN_US);
            if (previewFPS > 0) {
                previewFPS = 1000000 / previewFPS;
            } else {
                previewFPS = 0;
            }

            String format =
                    "AEid:%d PSS:%d PSG:%d PISPG:%d CSS:%d CSG:%d CISPG:%d PFPS:%.1f BV:%d ISO:%d";
            String info = String.format(format,
                      getParameterValue(KEY_ENG_PREVIEW_AE_INDEX),
                      getParameterValue(KEY_ENG_PREVIEW_SHUTTER_SPEED),
                      getParameterValue(KEY_ENG_PREVIEW_SENSOR_GAIN),
                      getParameterValue(KEY_ENG_PREVIEW_ISP_GAIN),
                      getParameterValue(KEY_ENG_CAPTURE_SHUTTER_SPEED),
                      getParameterValue(KEY_ENG_CAPTURE_SENSOR_GAIN),
                      getParameterValue(KEY_ENG_CAPTURE_ISP_GAIN),
                      previewFPS,
                      getParameterValue(KEY_BRIGHTNESS_VALUE),
                      getParameterValue(KEY_ENG_PREVIEW_ISO));
            mTvCameraInfo.setText(info);
        }
    }

    private void resetPrivateParams() {
        mAfThroughRepearPointer = 0;
        mCaptureDoneNumber = 0;
        mEnableShutterDelay = true;
    }

    private void capture() {
        Elog.i(TAG, "==== capture() ====");
        if (mCameraState == SNAPSHOT_IN_PROGRESS || mCameraDevice == null || mPausing) {
            Elog.i(TAG, "capture() return");
            return;
        }
        onShutterButtonFocus(true);
    }

    private boolean isStorageEnough4Capture() {
        Elog.i(TAG, "isStorageEnough() captureType:" + mCaptureType);
        long available = getAvailableSpace();

        if (mCaptureType == 2) { // JPEG_ONLY
            return available > JPEG_MAX_SIZE;
        } else {
            return available > RAW_JPEG_MAX_SIZE;
        }
    }

    // Capture image need wait auto focus done
    @Override
    public void onAutoFocusDone() {
        if (mCameraState == SNAPSHOT_IN_PROGRESS || mCameraDevice == null || mPausing) {
            Elog.i(TAG, "onAutoFocusDone() return 1");
            return;
        }
        Elog.i(TAG, "==== onAutoFocusDone() ====");
        // when touch focus, but no click shutter button, only do auto focus
        if (mFocusManager.isTouchFocusMode() && mShutterButton.isEnabled()) {
            Elog.i(TAG, "onAutoFocusDone() return 2");
            return;
        }
        if (mAfAdvFullScan) {
            setCameraState(IDLE);
            mFocusManager.onPreviewStarted();
            if (mAfAdvFullFocusTimes == 0) {
                mAfAdvFullSensorGain = getParameterValue(KEY_ENG_CAPTURE_SENSOR_GAIN);
                mAfAdvFullIspGain = getParameterValue(KEY_ENG_CAPTURE_ISP_GAIN);
                Elog.i(TAG, "get Parameters Sensor Gain:" + mAfAdvFullSensorGain +
                        "; get Parameters Isp Gain: " + mAfAdvFullIspGain);
                Util.setProperty(PROPERTY_AE_MGR_PREVIEW_UPDATE, "1");
                Elog.i(TAG,
                        "get " + PROPERTY_AE_MGR_PREVIEW_UPDATE + " value: "
                                + Util.getProperty(PROPERTY_AE_MGR_PREVIEW_UPDATE));
                Util.setProperty(PROPERTY_AF_AFS_ENABLE, "1");
                Elog.i(TAG,
                        "get " + PROPERTY_AF_AFS_ENABLE + " value: "
                                + Util.getProperty(PROPERTY_AF_AFS_ENABLE));
            } else if (mAfAdvFullFocusTimes > 3) {
                Elog.i(TAG, "Advanced full scan done, reset status");
                Util.setProperty(PROPERTY_AE_MGR_PREVIEW_UPDATE, "0");
                Elog.i(TAG,
                        "get " + PROPERTY_AE_MGR_PREVIEW_UPDATE + " value: "
                                + Util.getProperty(PROPERTY_AE_MGR_PREVIEW_UPDATE));
                Util.setProperty(PROPERTY_AF_AFS_ENABLE, "0");
                Elog.i(TAG,
                        "get " + PROPERTY_AF_AFS_ENABLE + " value: "
                                + Util.getProperty(PROPERTY_AF_AFS_ENABLE));
                mAfAdvFullFocusTimes = 0;

                mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                Elog.i(TAG, " Advanced Full Scan Done !!");
                return;
            }
            Util.setProperty(PROPERTY_AE_MGR_SHUTTER, mAfAdvFullShutter[mAfAdvFullFocusTimes]);
            Util.setProperty(PROPERTY_AE_MGR_SENSORGAIN, String.valueOf(mAfAdvFullSensorGain));
            Util.setProperty(PROPERTY_AE_MGR_ISPGAIN, String.valueOf(mAfAdvFullIspGain));
            Elog.i(TAG,
                    "get " + PROPERTY_AE_MGR_SHUTTER + " value: "
                            + Util.getProperty(PROPERTY_AE_MGR_SHUTTER));
            mAfAdvFullFocusTimes++;
            setCameraParameters(UPDATE_PARAM_ALL);
            mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
            return;
        }

        if (mCaptureMode == 0) {
            setCameraState(SNAPSHOT_IN_PROGRESS);
            AfCaptureThread afAutoThread = new AfCaptureThread();
            afAutoThread.start();
        } else if (mCaptureMode == 2) {
            mParameters.set(KEY_RAW_DUMP_FLAG, TRUE);
            mCameraDevice.setParameters(mParameters);
            mFocusManager.capture();
            onShutterButtonFocus(false);
            mShutterButton.setEnabled(true);
            mShutterButtonEnabled = true;
        }
    }

    private void sendNotification(String title, String ContentText) {
        Elog.d(TAG, "!! sendNotification !!");
        NotificationManager notificationManager =
                (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

        if (notificationManager.getNotificationChannel(NOTIFICATION_CHANNEL_ID) == null) {
            NotificationChannel channel = new NotificationChannel(NOTIFICATION_CHANNEL_ID,
                    NOTIFICATION_NAME,
                    NotificationManager.IMPORTANCE_LOW);
            channel.setSound(RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION),
                    new AudioAttributes.Builder().setUsage(
                            AudioAttributes.USAGE_NOTIFICATION).build());
            notificationManager.createNotificationChannel(channel);
        }

        Notification notification = new Notification.Builder(this, NOTIFICATION_CHANNEL_ID)
                .setContentTitle(title)
                .setContentText(ContentText)
                .setSmallIcon(R.drawable.ic_em_camera)
                .setAutoCancel(true)
                .build();

        notification.defaults |= Notification.DEFAULT_SOUND;
        notificationManager.notify(99, notification);
    }

    class AfCaptureThread extends Thread {
        public void run() {
            Elog.i(TAG, "==== AfCaptureThread start. ====");
            mTakePicDone = false;
            if (mDBMode == DB3A.DB3A_FLASH) { // 3A - FLASH
                Elog.v(TAG, "[before]Strobe on.");
                if (mFlashCount == 0) {
                    Elog.i(TAG, "Set strobe off.");
                    mParameters.set(KEY_FLASH_MODE, "off");
                } else if (mFlashCount == 1) {
                    Elog.i(TAG, "Set strobe on.");
                    mParameters.set(KEY_FLASH_MODE, "on");
                }
            } else {
                Elog.v(TAG, "StrobeMode: " + mStrobeMode);
            }
            takePicture();
            int countDownAbort = 4;
            while (!mTakePicDone) {
                try {
                    sleep(100);
                } catch (InterruptedException e) {
                    Elog.e(TAG, e.getMessage());
                }
                Elog.i(TAG, "take picture done, count down abort: " + countDownAbort);
                if( --countDownAbort == 0) {
                    if(mShutterSpeed > LONG_EXPOSURE_TIME && SNAPSHOT_IN_PROGRESS == mCameraState) {
                        mHandler.sendEmptyMessage(MSG_ID_ENABLE_ABORT_BTN);
                    }
                }
            }

            if (mDBMode == DB3A.DB3A_FLASH) { // 3A - FLASH
                Elog.v(TAG, "[after]Strobe on.");
                if (mFlashCount == 1) {
                    Elog.i(TAG, "Complete strobe.");
                    mFlashCount = 0;
                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                } else if (mFlashCount == 0) {
                    Elog.i(TAG, "Continue strobe.");
                    mFlashCount++;
                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                }
            } else if (mAfMode == AF_MODE_AUTO) {
                if (mAfCaptureTimes >= mIsoValues.size()) { // count
                    mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                } else {
                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                }
            } else if (mAfMode == AF_MODE_BRACKET) { // bracket
                if (mBracketTimes > mAfBracketRange) {
                    if (mIsBracketAddPos) {
                        mIsBracketAddPos = false;
                        mBracketTimes = 1;
                        mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                    } else {
                        mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
                    }
                } else {
                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                }
            } else if (mAfMode == AF_MODE_FULL_SCAN) { // full scan
                mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
            } else if (mAfMode == AF_MODE_THROUGH_FOCUS) { // through focus
                mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
            } else if (mAfMode == AF_MODE_CONTINUOUS) {
                mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
            } else if (mAfMode == AF_MODE_TEMP_CALI) { // temp cali
                mHandler.sendEmptyMessage(EVENT_COMPLETE_CAPTURE);
            }
            Elog.i(TAG, "AfCaptureThread finish.");
        }
    }

    private void takePicture() {
        if (mCameraDevice == null || mPausing) {
            Elog.i(TAG, "takePicture() return1");
            return;
        }
        Elog.i(TAG, "==== takePicture() start ====");
        mCaptureStartTime = System.currentTimeMillis();
        mJpegImageData = null;
        // Set rotation data.
        Util.setRotationParameter(mParameters, mCameraId, mOrientation);
        mCameraDevice.setParameters(mParameters);
        mCameraDevice.takePicture(mShutterCallback, mRawPictureCallback, new JpegPictureCallback());
    }

    // Snapshots can only be taken after this is called. It should be called
    // once only. We could have done these things in onCreate() but we want to
    // make preview screen appear as soon as possible.
    private void initializeFirstTime() {
        Elog.i(TAG, "initializeFirstTime()");
        if (mFirstTimeInitialized) {
            Elog.i(TAG, "initializeFirstTime() return 1");
            return;
        }
        // Create orientation listenter. This should be done first because it
        // takes some time to get first orientation.
        mOrientationListener = new MyOrientationEventListener(Camera.this);
        mOrientationListener.enable();
        checkStorage();
        // Initialize shutter button.
        mShutterButton.setOnTouchListener(this);
        if(mShutterSpeed > LONG_EXPOSURE_TIME) {
            mAbortButton.setOnTouchListener(this);
        }
        // Initialize focus UI.
        mPreviewFrame = findViewById(R.id.camera_preview);
        mPreviewFrame.setOnTouchListener(this);
        mFocusAreaIndicator = (RotateLayout) findViewById(R.id.focus_indicator_rotate_layout);
        CameraInfo info = CameraHolder.instance().getCameraInfo()[mCameraId];
        boolean mirror = (info.facing == CameraInfo.CAMERA_FACING_FRONT);
        if (mFaceDetectionEnabled) {
            mFaceView = (FaceView) findViewById(R.id.face_view);
        }
        mFocusManager.setFocusWinRatio(mAfFullWinWidth, mAfFullWinHeight);
        mFocusManager.initialize(mFocusAreaIndicator, mPreviewFrame, mFaceView, this, mirror,
                mDisplayOrientation);
        if (mAfMode == AF_MODE_FULL_SCAN) {
            // if (mAfAdvFullScan) {
            Elog.i(TAG, "initializeFirstTime startShowFocusView");
            mFocusManager.showIdleFocusView();
        }
        Util.initializeScreenBrightness(getWindow(), getContentResolver());
        installIntentFilter();
        mFirstTimeInitialized = true;
        Elog.i(TAG, "initializeFirstTime() - end");
    }

    // If the activity is paused and resumed, this method will be called in
    // onResume.
    private void initializeSecondTime() {
        Elog.i(TAG, "initializeSecondTime()");
        // Start orientation listener as soon as possible because it takes
        // some time to get first orientation.
        mOrientationListener.enable();
        installIntentFilter();
        checkStorage();
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent m) {
        // Elog.i(TAG, "dispatchTouchEvent()");
        // Check if the popup window should be dismissed first.
        if (mPopupGestureDetector != null && mPopupGestureDetector.onTouchEvent(m)) {
            return true;
        }
        return super.dispatchTouchEvent(m);
    }

    private final class ShutterCallback implements android.hardware.Camera.ShutterCallback {
        public void onShutter() {
            mShutterCallbackTime = System.currentTimeMillis();
            mShutterLag = mShutterCallbackTime - mCaptureStartTime;
            Elog.i(TAG, "mShutterLag = " + mShutterLag + "ms");
            // mFocusManager.onShutter();
        }
    }

    private final class RawPictureCallback implements PictureCallback {
        public void onPictureTaken(byte[] rawData, android.hardware.Camera camera) {
            mRawPictureCallbackTime = System.currentTimeMillis();
            Elog.i(TAG, "mShutterToRawCallbackTime = "
                    + (mRawPictureCallbackTime - mShutterCallbackTime) + "ms");
        }
    }

    private final class JpegPictureCallback implements PictureCallback {

        @Override
        public void onPictureTaken(byte[] jpegData, android.hardware.Camera camera) {
            if (mPausing) {
                return;
            }
            mAbortButton.setEnabled(false);
            mJpegImageData = jpegData;
            Elog.i(TAG, "==== jpegData.length %s ====" + jpegData.length);
            doAttach();
        }
    }

    private void doAttach() {
        mSaveCorrectFlag = 1;
        Elog.i(TAG, "==== doAttach() ====");
        setCameraState(SAVING_PICTURES);
        String jpegName = mRawCaptureFileName;
        // ISO is Auto, get real value
        String txtName = jpegName + ".txt";
        jpegName = jpegName + ".jpg";
        Elog.i(TAG, "Jpeg name is " + jpegName);
        File fHandle = new File(jpegName);
        OutputStream bos = null;
        try {
            bos = new FileOutputStream(fHandle);
            bos.write(mJpegImageData);
            bos.close();
        } catch (IOException ex) {
            fHandle.delete();
        } finally {
            try {
                if (bos != null) {
                    bos.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        // For save some parameters in txt file.
        StringBuilder strBuilder = new StringBuilder();
        if (mVideoHdr == 1) {
            strBuilder.append(mVHdrMode + "\n");
        }
        if (mMfllMode != 2) {
            strBuilder.append("MFLL\n");
        }
        if (mMnrSupport) {
            String[] mnrArray = { "MNR-AUTO", "MNR-DISABLE", "MNR-HWNR", "MNR-SWNR" };
            strBuilder.append(mnrArray[mMultiPass]);
        }

        String content = strBuilder.toString();
        fHandle = new File(txtName);
        try {
            bos = new FileOutputStream(fHandle);
            bos.write(content.getBytes());
            bos.close();
        } catch (IOException ex) {
            fHandle.delete();
        } finally {
            try {
                if (bos != null) {
                    bos.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        mHandler.sendEmptyMessage(PICTURES_SAVING_DONE);
        mCaptureDoneNumber++;
    }

    private static String createNameJpeg(long dateTaken) {
        return DateFormat.format("yyyy-MM-dd kk.mm.ss", dateTaken).toString();
    }

    private final class AutoFocusCallback implements android.hardware.Camera.AutoFocusCallback {
        public void onAutoFocus(boolean focused, android.hardware.Camera camera) {
            if (mPausing) {
                return;
            }
            if (mIsAutoFocusCallback) {
                Elog.i(TAG, "Ignore second focus callback");
                return; // Ignore second auto focus callback
            }
            if (!mTakePicDone) {
                Elog.i(TAG, "Ignore focus callback when taking picture");
                return;
            }
            Elog.i(TAG, "==== AutoFocusCallback ====");
            mAutoFocusTime = System.currentTimeMillis() - mFocusStartTime;
            Elog.i(TAG,
                    "mAutoFocusTime = " + mAutoFocusTime + "ms" + ", camera State = "
                            + String.valueOf(mCameraState));
            if (mCameraState == FOCUSING) {
                setCameraState(IDLE);
            }
            mFocusManager.doSnap();
            mFocusManager.onAutoFocus(focused);
            setIsFocusCallback(true);
        }
    }
    /**
     * CameraErrorCallback, receive error info.
     *
     */
    private class CameraErrorCallback implements android.hardware.Camera.ErrorCallback {
        public void onError(int error, android.hardware.Camera camera) {
            Elog.e(TAG, "Got camera error callback. error=" + error);
            if (error == android.hardware.Camera.CAMERA_ERROR_SERVER_DIED) {
                // We are not sure about the current state of the app (in preview or
                // snapshot or recording). Closing the app is better than creating a
                // new Camera object.
                throw new RuntimeException("Media server died.");
            } else if (error == CAMERA_ERROR_CALI_FLASH) {
                mFlashCaliError = true;
                showCaliDialog("Error", "Calibration result is not correct");
            }
        }
    }

    private void showCaliDialog(String title, String msg) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(title);
        builder.setMessage(msg);
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                finish();
                Elog.i(TAG, "Press OK");
            }
        });
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    private void setCameraState(int state) {
        Elog.i(TAG, "setCameraState() state " + state);
        mCameraState = state;
    }

    private void setIsFocusCallback(boolean state) {
        Elog.i(TAG, "setIsFocusCallback state " + state);
        mIsAutoFocusCallback = state;
    }

    @Override
    public void setFocusParameters() {
        Elog.i(TAG, "setFocusParameters()");
        if (mCameraDevice == null) {
            return;
        }
        mParameters = mCameraDevice.getParameters();
        if (mFocusAreaSupported && !mIsAutoFocusCallback) {
            mParameters.setFocusAreas(mFocusManager.getFocusAreas());
        }
        if (mMeteringAreaSupported && !mIsAutoFocusCallback) {
            mParameters.setMeteringAreas(mFocusManager.getMeteringAreas());
        }
        mCameraDevice.setParameters(mParameters);
        setIsFocusCallback(false);
    }

    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        Elog.i(TAG, "[onCreate] Bundle = " + String.valueOf(icicle));
        mFocusManager = new FocusManager();
        mCaptureNamePrefix = getResources().getStringArray(R.array.normal_captrue_name_prefix);
        getSettingsFromPref(AutoCalibration.PREFERENCE_KEY);

        setContentView(R.layout.new_camera);
        // don't set mSurfaceHolder here. We have it set ONLY within
        // surfaceChanged / surfaceDestroyed, other parts of the code
        // assume that when it is set, the surface is also set.
        SurfaceView surfaceView = (SurfaceView) findViewById(R.id.camera_preview);
        SurfaceHolder holder = surfaceView.getHolder();
        holder.addCallback(this);
        holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);

        mShutterButton = (Button) findViewById(R.id.capture_btn);
        mShutterButtonEnabled = true;
        mAbortButton = (Button) findViewById(R.id.abort_btn);
        if(mShutterSpeed > LONG_EXPOSURE_TIME) {
            mAbortButton.setVisibility(View.VISIBLE);
            mAbortButton.setEnabled(false);
        } else {
            mAbortButton.setVisibility(View.GONE);
        }
        Intent intent = getIntent();
        mEvCalibration = intent.getIntExtra(getString(R.string.camera_key_ev_calibration),
                AutoCalibration.ID_EV_CALIBRATION_NONE);
        if (mEvCalibration == AutoCalibration.ID_EV_CALIBRATION_ACTION) {
            mShutterButton.setText(getString(R.string.camera_ev_calibration));
        }
        mGisCalibration = intent.getIntExtra(getString(R.string.camera_key_gis_calibration),
                AutoCalibration.ID_GIS_CALIBRATION_NONE);
        if (mGisCalibration == AutoCalibration.ID_GIS_CALIBRATION_ACTION) {
            mShutterButton.setText(getString(R.string.camera_gis_calibration));
            mEnableGisCalibration = true;
        } else {
            mEnableGisCalibration = false;
        }

        mTvCameraInfo = (TextView) findViewById(R.id.camear_info_tv);
        Elog.i(TAG, "[onCreate] Finished");

    }

    @Override
    protected void doOnResume() {
        try {
            Elog.i(TAG, "onResume start");
            setSystemProperties();
            if (mStereoCameraId > 1) {
                openStereoSensor(true);
            }
            mCameraDevice = Util.openCamera(Camera.this, mCameraId);
        } catch (CameraHardwareException e) {
            Util.showErrorAndFinish(this, R.string.cannot_connect_camera);
            return;
        } catch (CameraDisabledException e) {
            Util.showErrorAndFinish(this, R.string.camera_disabled);
            return;
        } catch (Exception e) {
            Util.showErrorAndFinish(this, R.string.cannot_connect_camera);
            return;
        }
        mPausing = false;
        // start preview
        initializeCapabilities();

        if (mSurfaceHolder != null) {
            // If first time initialization is not finished, put it in the
            // message queue.
            if (!mFirstTimeInitialized) {
                mHandler.sendEmptyMessage(FIRST_TIME_INIT);
            } else {
                initializeSecondTime();
            }
        }

        startPreview();

        keepScreenOnAwhile();
        if (mCameraState == IDLE) {
            mOnResumeTime = SystemClock.uptimeMillis();
            mHandler.sendEmptyMessageDelayed(CHECK_DISPLAY_ROTATION, 100);
        }
        if(AutoCalibration.isCameraHal1()) {
            updateUiCameraInfo();
            mHandler.sendEmptyMessageDelayed(MSG_ID_UPDATE_CAMERA_INFO, INTERVAL_MS_UPDATE_CAMERA_INFO);
        }
        Elog.i(TAG, "doOnresume end");
    }

    private void setSystemProperties() {
        // set Eng mode before Camera open
        Util.setProperty(PROPERTY_KEY_CLIENT_APPMODE, APP_MODE_NAME_MTK_ENG);
        Util.setProperty(PROPERTY_SENSOR_MODE, "" + mCaptureSize);
        Elog.i(TAG,
                "get \"" + PROPERTY_KEY_CLIENT_APPMODE + "\" value: "
                        + Util.getProperty(PROPERTY_KEY_CLIENT_APPMODE));
        if (mAfMode == AF_MODE_FULL_SCAN) {
            Util.setProperty(PROPERTY_AF_WIN_WIDTH, String.valueOf(mAfFullWinWidth));
            Util.setProperty(PROPERTY_AF_WIN_HEIGHT, String.valueOf(mAfFullWinHeight));
            Elog.i(TAG,
                    "get " + PROPERTY_AF_WIN_WIDTH + " value: "
                            + Util.getProperty(PROPERTY_AF_WIN_WIDTH));
            Elog.i(TAG,
                    "get " + PROPERTY_AF_WIN_HEIGHT + " value: "
                            + Util.getProperty(PROPERTY_AF_WIN_HEIGHT));

            if (mAfAdvFullScan) {
                Util.setProperty(PROPERTY_AF_AFS_REPEAT, String.valueOf(mAfAdvFullRepeat));
                Util.setProperty(PROPERTY_AE_MGR_ENABLE, "1");
                Elog.i(TAG,
                        "get " + PROPERTY_AE_MGR_ENABLE + " value: "
                                + Util.getProperty(PROPERTY_AE_MGR_ENABLE));
                Elog.i(TAG,
                        "get " + PROPERTY_AF_AFS_REPEAT + " value: "
                                + Util.getProperty(PROPERTY_AF_AFS_REPEAT));
            }
        } else if (mAfMode == AF_MODE_TEMP_CALI) {
            Util.setProperty(PROPERTY_AF_CALI_FLAG, "1");
            Elog.i(TAG,
                    "get " + PROPERTY_AF_CALI_FLAG + " value: "
                            + Util.getProperty(PROPERTY_AF_CALI_FLAG));
        }
    }

    private void resetSystemProperties() {

        // set Eng mode after Camera close
        Util.setProperty(PROPERTY_KEY_CLIENT_APPMODE, "");
        Util.setProperty(PROPERTY_SENSOR_MODE, "-1");
        Elog.i(TAG, "client.appmode = " + Util.getProperty(PROPERTY_KEY_CLIENT_APPMODE));

        if (mAfAdvFullScan) {
            Util.setProperty(PROPERTY_AE_MGR_ENABLE, "0");
            Elog.i(TAG,
                    "get " + PROPERTY_AE_MGR_ENABLE + " value: "
                            + Util.getProperty(PROPERTY_AE_MGR_ENABLE));
            Util.setProperty(PROPERTY_AE_MGR_PREVIEW_UPDATE, "0");
            Elog.i(TAG,
                    "get " + PROPERTY_AE_MGR_PREVIEW_UPDATE + " value: "
                            + Util.getProperty(PROPERTY_AE_MGR_PREVIEW_UPDATE));
            Util.setProperty(PROPERTY_AF_AFS_ENABLE, "0");
            Elog.i(TAG,
                    "get " + PROPERTY_AF_AFS_ENABLE + " value: "
                            + Util.getProperty(PROPERTY_AF_AFS_ENABLE));
            mAfAdvFullFocusTimes = 0;
        } else if (mAfMode == AF_MODE_TEMP_CALI) {
            Util.setProperty(PROPERTY_AF_CALI_FLAG, "0");
            Elog.i(TAG,
                    "get " + PROPERTY_AF_CALI_FLAG + " value: "
                            + Util.getProperty(PROPERTY_AF_CALI_FLAG));
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        Elog.i(TAG, "onPause()");
        if(mGisCaliTask!= null && mGisCaliTask.getStatus() == AsyncTask.Status.RUNNING) {
            mGisCaliTask.cancel(true);
        }
        mPausing = true;
        mSnapshotOnIdle = false;

        stopPreview();
        // Close the camera now because other activities may need to use it.
        closeCamera();
        resetSystemProperties();
        resetScreenOn();
        // Clear UI.
        if (mFirstTimeInitialized) {
            mOrientationListener.disable();
        }

        if (mDidRegister) {
            unregisterReceiver(mReceiver);
            mDidRegister = false;
        }
        // If we are in an image capture intent and has taken
        // a picture, we just clear it in onPause.
        mJpegImageData = null;

        // Remove the messages in the event queue.
        mHandler.removeMessages(FIRST_TIME_INIT);
        mHandler.removeMessages(CHECK_DISPLAY_ROTATION);
        mFocusManager.removeMessages();

        // Mediatek merge begin
        setIsFocusCallback(false);
        // Mediatek merge end
        if (mShutterButtonEnabled) {
            Elog.i(TAG, "onPause, mShutterButton isEnabled!");
            mCaptureDoneNumber = 0;
            mAfCaptureTimes = 0;
            mTripleCount = 0;
            // Init For next capture
            mBracketTimes = 0;
            mIsBracketAddPos = true;
        }
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    private void getSettingsFromPref(String preferenceKey) {
        final SharedPreferences preferences = getSharedPreferences(preferenceKey,
                android.content.Context.MODE_PRIVATE);
        mCaptureMode = preferences.getInt(getString(R.string.auto_clibr_key_capture_mode), 0);
        if (mCaptureMode == 0) {
            mCaptureSize = preferences.getInt(getString(R.string.auto_clibr_key_capture_size), 1);
            mCaptureRatio = preferences.getInt(getString(R.string.auto_clibr_capture_ratio), 0);
            mCaptureType = preferences.getInt(getString(R.string.auto_clibr_key_capture_type), 1);
            mCaptureNumber = preferences.getInt(getString(R.string.auto_clibr_key_capture_number),
                    1);
            Elog.i(TAG, "Normal capture: type = " + mCaptureType);
            ispProfile = preferences.getInt(getString(R.string.auto_clibr_isp_profile), 1);
            Elog.i(TAG, "Normal capture: isp = " + ispProfile);
        } else {//phaseout version2
            videoDumpBuffer = preferences.getInt(getString(R.string.auto_clibr_video_dump_buffer),
                    0);
            Elog.i(TAG, "video dump buffer: " + videoDumpBuffer);
            ispProfile = preferences.getInt(getString(R.string.auto_clibr_isp_profile), 0);
            sensorMode = preferences.getInt(getString(R.string.auto_clibr_sensor_mode), 0);
            mVideoResolution = preferences.getInt(
                    getString(R.string.auto_clibr_key_capture_resolution), 0);
            Elog.i(TAG, "video: isp = " + ispProfile + " sensor = " + sensorMode);
            if (preferences.getInt(getString(R.string.auto_clibr_video_frame_enable), 0) == 0) {
                mEnableFameRate = 0;
            } else {
                mEnableFameRate = 1;
            }
            mHighFrameRate = preferences.getInt(getString(R.string.auto_clibr_video_frame_high), 30);
            mLowFrameRate = preferences.getInt(getString(R.string.auto_clibr_video_frame_low), 15);
        }

        if (preferences.getInt(getString(R.string.auto_clibr_key_flicker), 0) == 0) {
            mFlickerString = "50hz";
            mAfAdvFullShutter = AF_ADV_50HZ_SHUTTER_ARRAY;// For AF advanced full scan
        } else {
            mFlickerString = "60hz";
            mAfAdvFullShutter = AF_ADV_60HZ_SHUTTER_ARRAY;
        }
        mStrobeMode = preferences.getString(getString(R.string.auto_clibr_key_led_flash), "off");
        mPreFlashLevel = preferences.getInt(getString(R.string.auto_clibr_key_pre_flash), 1);
        mMainFlashLevel = preferences.getInt(getString(R.string.auto_clibr_key_main_flash), 1);
        mAfMode = preferences.getInt(getString(R.string.auto_clibr_key_af_mode), 0);
        if (mAfMode == AF_MODE_BRACKET) {
            mAfBracketIntervel = preferences.getInt(
                    getString(R.string.auto_clibr_key_branket_interval), 1);
            mAfBracketRange = preferences.getInt(getString(R.string.auto_clibr_key_branket_range),
                    0);
        } else if (mAfMode == AF_MODE_THROUGH_FOCUS) {
            mAfThroughDirect = preferences.getInt(
                    getString(R.string.auto_clibr_key_through_focus_dirct), 0);
            if (mAfThroughDirect == 2) {
                mThroughFocusStartPos = preferences.getInt(
                        getString(R.string.auto_clibr_key_through_manual_start_pos), 0);
                mThroughFocusEndPos = preferences.getInt(
                        getString(R.string.auto_clibr_key_through_manual_end_pos), 1023);
            }
            mAfThroughIntervel = preferences.getInt(
                    getString(R.string.auto_clibr_key_through_focus_interval), 1);
            mAfThroughRepeat = preferences.getInt(
                    getString(R.string.auto_clibr_key_through_repeat), 1);
        } else if (mAfMode == AF_MODE_FULL_SCAN) {
            mAfFullScanFrameInterval = preferences.getInt(
                    getString(R.string.auto_clibr_key_full_frame_interval), 0);
            mAfFullScanDacStep = preferences.getInt(
                    getString(R.string.auto_clibr_key_full_scan_dac_step), 1);
            mAfFullWinWidth = preferences.getInt(
                    getString(R.string.auto_clibr_key_full_scan_focus_win_width), 18);
            mAfFullWinHeight = preferences.getInt(
                    getString(R.string.auto_clibr_key_full_scan_focus_win_height), 24);
            mAfAdvFullScan = preferences.getBoolean(
                    getString(R.string.auto_clibr_key_advanced_full_scan), false);
            if (mAfAdvFullScan) {
                mAfAdvFullRepeat = preferences.getInt(
                        getString(R.string.auto_clibr_key_full_scan_repeat), 10);
            }
        }
        getIsoValue(mAfMode,
                preferences.getString(getString(R.string.auto_clibr_key_iso_speed), "0"));
        Elog.i(TAG,
                "@getSettingsFromPref, iso speed: "
                        + preferences.getString(getString(R.string.auto_clibr_key_iso_speed), "0"));

        AF_EVSTEPSIZE = mAfThroughRepeat; // 3A - AF
        mCustomParameters = preferences.getString(getString(R.string.camera_key_custom_parameter),
                ";;;");
        Elog.i(TAG, "mCustomParameters: " + mCustomParameters);
        initDbMode(preferences);

        mCameraId = preferences.getInt(getString(R.string.camera_key_selected_sensor_id), 0);
        mShutterDelay = preferences.getInt(getString(R.string.camera_key_shutter_delay), 0);
        mShadingTable = preferences.getInt(getString(R.string.camera_key_shading_table), 4);//phaseout version2
        mOutputSdblk = preferences.getInt(getString(R.string.camera_key_output_sdblk), 1);//phaseout version2

        mMfllMode = preferences.getInt(getString(R.string.auto_clibr_key_mfll_enable), 0);//phaseout version2
        mMfllCount = preferences.getInt(getString(R.string.auto_clibr_key_mfll_count), 1);//phaseout version2

        mAwbVeri = preferences.getInt(getString(R.string.auto_clibr_verification_awb), 1);
        mShadingVeri = preferences.getInt(getString(R.string.auto_clibr_verification_shading), 7);//phaseout version2
        mVideoHdr = preferences.getInt(getString(R.string.video_hdr), 0);
        if (mVideoHdr == 1) {
            mVHdrMode = preferences.getString(getString(R.string.vhdr_set_mode), "0");
            mVHdrRatio = preferences.getInt(getString(R.string.vhdr_set_ratio), 0);
        }
        mMultiPass = preferences.getInt(getString(R.string.multi_pass_nr), 0);//phaseout version2
        mShutterSpeed = preferences.getInt(getString(R.string.auto_clibr_capture_shutter_speed), 0);
        mSensorGain = preferences.getInt(getString(R.string.auto_clibr_capture_sensor_gain), 0);
        mSceneMode = preferences.getInt(getString(R.string.auto_clibr_scene_mode), 0);//phaseout version2
        mStereoCameraId = preferences.getInt(getString(R.string.auto_clibr_camera_sensor_main2), 0);
        mFlashCali = preferences.getInt(getString(R.string.auto_clibr_flash_calibration), 0);
        mFaceDetectionEnabled = preferences
                .getInt(getString(R.string.auto_clibr_face_detection), 0) == 1
                && (mAfMode != AF_MODE_FULL_SCAN) && (mAfMode != AF_MODE_TEMP_CALI);
        mMinFps = preferences.getInt(getString(R.string.auto_clibr_min_fps), 0);
        mMaxFps = preferences.getInt(getString(R.string.auto_clibr_max_fps), 0);
    }

    private void initDbMode(SharedPreferences preferences) {
        // parsing mode/test case/parameters
        mDBMode = DB3A.DB3A_OFF;
        Elog.i(TAG, "P1:" + getCustomParameter(0) + ", P2:" + getCustomParameter(1) + ", P3:"
                + getCustomParameter(2));
        aeSettings = getPrefAeMode(preferences);
        if (aeSettings.getAeMode().equals(AeMode_e.Bracket)) { // 3A - AE
            mDBMode = DB3A.DB3A_AE;
            mDBAEState = new DB3A_AEState();
            mDBAEState.setStart(0 - aeSettings.getMinus() - aeSettings.getInterval());
            mDBAEState.setEnd(aeSettings.getPlus());
            mDBAEState.setStep(aeSettings.getInterval());
            Elog.i(TAG, "DBMode = AE");
        } else if (getCustomParameter(0).toUpperCase().equals("AF")) { // 3A - AF
            mDBMode = DB3A.DB3A_AF;
            AF_TESTSCENE = getCustomParameter(1);
            AF_EVSTEPNUM = getCustomParameter(2);
            if (!AF_EVSTEPNUM.equals("")) {
                AF_EVSTEPSIZE = mAfThroughRepeat;
                mAfThroughRepeat = Integer.parseInt(AF_EVSTEPNUM);
            } else {
                AF_EVSTEPSIZE = 0;
            }
            Elog.i(TAG, "DBMode = AF");
        } else if (getCustomParameter(0).toUpperCase().equals("AWB")) { // 3A - AWB
            mDBMode = DB3A.DB3A_AWB;
            DB3A_AWBTestItem = getCustomParameter(1);
            DB3A_AWBTestItemSub = getCustomParameter(2);
            Elog.i(TAG, "DBMode = AWB");
        } else if (getCustomParameter(0).toUpperCase().equals("F")) { // 3A - FLASH
            mDBMode = DB3A.DB3A_FLASH;
            Elog.i(TAG, "DBMode = Flash");
        } else if (getCustomParameter(0).toUpperCase().equals("FAE")) {
            mDBMode = DB3A.DB3A_FLASH_AE;
            Elog.i(TAG, "DBMode = FAE");
        } else if (getCustomParameter(0).toUpperCase().equals("FAWB")) {
            mDBMode = DB3A.DB3A_FLASH_AWB;
            Elog.i(TAG, "DBMode = FAWB");
        }
    }

    public AeSettings getPrefAeMode(SharedPreferences preferences) {
        AeSettings aeSettings = new AeSettings();
        String aeModeDesc = preferences.getString(getString(R.string.auto_clibr_ae_mode),
                AeMode_e.Auto.getDesc());
        if (aeModeDesc.equals(AeMode_e.Bracket.getDesc())) {
            aeSettings.setAeMode(AeMode_e.Bracket);
            aeSettings.setMinus(preferences.getInt(getString(R.string.auto_clibr_ae_minus), 20));
            aeSettings.setPlus(preferences.getInt(getString(R.string.auto_clibr_ae_plus), 20));
            aeSettings.setInterval(preferences.getInt(getString(R.string.auto_clibr_ae_interval),
                    10));
        } else {
            aeSettings.setAeMode(AeMode_e.Auto);
        }
        Elog.i(TAG, "@getPrefAeMode,toString: " + aeSettings.toString());

        return aeSettings;
    }

    // get custom parameter by index, which start from 0
    String getCustomParameter(int index) {
        if (mCustomParameters == null) {
            Elog.i(TAG, "mCustomParameters is null");
            return "";
        }
        String[] parameters = mCustomParameters.split(";", -1);
        if (index < 0 || index >= parameters.length) {
            throw new IllegalArgumentException("index must be 0 to " + (parameters.length - 1)
                    + "; current is " + index);
        }
        return parameters[index];
    }

    private void getIsoValue(int afMode, String iso) {
        Elog.i(TAG, "getIsoValue iso " + iso + " af mode " + afMode);
        if (afMode != 0) {
            mIsoValue = iso;
            if (mIsoValue.length() == 3) {
                mIsoValue = "00" + mIsoValue;
            }
            if (mIsoValue.length() == 4) {
                mIsoValue = "0" + mIsoValue;
            }
        } else {
            String[] tempStrings = iso.split(",");
            if (tempStrings != null) {
                if (mIsoValues == null) {
                    mIsoValues = new ArrayList<String>();
                }
                for (int i = 0; i < tempStrings.length; i++) {
                    Elog.i(TAG, "get is : " + tempStrings[i]);
                    if (tempStrings[i].length() == 3) { // "100" to "00100"
                        tempStrings[i] = "00" + tempStrings[i];
                    }
                    if (tempStrings[i].length() == 4) { // "1000" to "01000"
                        tempStrings[i] = "0" + tempStrings[i];
                    }
                }
                mIsoValues.addAll(Arrays.asList(tempStrings));
            } else {
                Elog.i(TAG, "mIsoValues == null");
            }
        }
    }

    private class MyOrientationEventListener extends OrientationEventListener {

        public MyOrientationEventListener(Context context) {
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
            mOrientation = Util.roundOrientation(orientation, mOrientation);
            if (mFaceView != null) {
                mFaceView.setOrientation(mOrientation);
            }
            // When the screen is unlocked, display rotation may change. Always
            // calculate the up-to-date orientationCompensation.
            int orientationCompensation = mOrientation + Util.getDisplayRotation(Camera.this);
            if (mOrientationCompensation != orientationCompensation) {
                mOrientationCompensation = orientationCompensation;
                setOrientationIndicator(mOrientationCompensation);
            }
        }
    }

    private void setOrientationIndicator(int orientation) {
        Elog.i(TAG, "setOrientationIndicator() orientation " + orientation);
        Rotatable[] indicators = { mFocusAreaIndicator };
        for (Rotatable indicator : indicators) {
            if (indicator != null) {
                indicator.setOrientation(orientation);
            }
        }

        if (mOnScreenIndicators != null) {
            mOnScreenIndicators.setOrientation(orientation);
        }

        if (mFaceView != null) {
            mFaceView.setOrientationCompensation(orientation);
        }
    }

    private void checkStorage() {
        Elog.i(TAG, "checkStorage()");
        mPicturesRemaining = getAvailableSpace();
        if (mPicturesRemaining == PREPARING) {
            toastOnUiThread(R.string.preparing_sd);
            return;
        } else if (mPicturesRemaining == UNAVAILABLE) {
            toastOnUiThread(R.string.auto_clibr_sdcard_tips);
            return;
        }
        if (mPicturesRemaining > LOW_STORAGE_THRESHOLD) {
            long pictureSize = PICTURE_SIZE;
            mPicturesRemaining = (mPicturesRemaining - LOW_STORAGE_THRESHOLD) / pictureSize;
        } else if (mPicturesRemaining > 0) {
            mPicturesRemaining = 0;
        }
        if (mCameraState == IDLE) {
            if (mPicturesRemaining < 0) {
                toastOnUiThread(R.string.not_enough_space);
            } else {
                Elog.w(TAG, "can take " + mPicturesRemaining + " photos.");
            }
        }
    }

    private void toastOnUiThread(final int strId) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(Camera.this, strId, Toast.LENGTH_LONG).show();
            }

        });
    }

    private long getAvailableSpace() {
        // get default sdcard path
        if (mStorageManager == null) {
            mStorageManager = (StorageManager) getSystemService(Context.STORAGE_SERVICE);
        }
        mCameraImageName = FILE_PATH;
        if (mDBMode != DB3A.DB3A_OFF) {
            // Do database setting
            switch (mDBMode) {
            case DB3A_AE: // 3A - AE
                mCameraImageName = FILE_PATH + "AE/EvBracket/";
                break;
            case DB3A_AF: // 3A - AF
                mCameraImageName = FILE_PATH + "AF/";
                if (AF_IS_THROUGH_FOCUS) {
                    mCameraImageName += AF_TESTSCENE + "-" + AF_TIMESTAMP + "/";
                }
                break;
            case DB3A_AWB: // 3A - AWB
                mCameraImageName = FILE_PATH + "AWB/" + DB3A_AWBTestItem + "/"
                        + DB3A_AWBTestItemSub + "/";
                break;
            case DB3A_FLASH:
                mCameraImageName = FILE_PATH + "Flash/";
                break;
            case DB3A_FLASH_AE: // 3A - FLASH
                mCameraImageName = FILE_PATH + "FlashAE/";
                break;
            case DB3A_FLASH_AWB:
                mCameraImageName = FILE_PATH + "FlashAWB/";
                break;
            }
        }

        File dir = new File(mCameraImageName);
        mkdirs(dir);
        if (!dir.isDirectory() || !dir.canWrite()) {
            Elog.w(TAG, "Camera Image Dir create fail");
            return FULL_SDCARD;
        }

        StatFs stat = new StatFs(mCameraImageName);
        return stat.getAvailableBlocks() * (long) stat.getBlockSize();
    }

    // @Override before auto focus set to true, after set to false
    public void onShutterButtonFocus(boolean pressed) {
        if (mPausing || mCameraState == SNAPSHOT_IN_PROGRESS) {
            return;
        }
        // Do not do focus if there is not enough storage.
        if (pressed && !canTakePicture()) {
            return;
        }
        Elog.i(TAG, "onShutterButtonFocus pressed = " + String.valueOf(pressed));
        if (pressed) {
            mFocusManager.onShutterDown();
        } else {
            mFocusManager.onShutterUp();
        }
    }

    // install an intent filter to receive SD card related events.
    private void installIntentFilter() {
        Elog.i(TAG, "installIntentFilter()");
        IntentFilter intentFilter = new IntentFilter(Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        intentFilter.addAction(Intent.ACTION_MEDIA_CHECKING);
        intentFilter.addDataScheme("file");
        registerReceiver(mReceiver, intentFilter);
        mDidRegister = true;
    }

    protected boolean canTakePicture() {
        boolean retVal = isCameraIdle();
        // retVal = retVal && (mPicturesRemaining > 0);
        Elog.i(TAG, "canTakePicture() " + retVal);
        return retVal;
    }

    @Override
    public void autoFocus() {
        Elog.i(TAG, "==== autoFocus ====");
        mFocusStartTime = System.currentTimeMillis();
        mCameraDevice.autoFocus(mAutoFocusCallback);
        setCameraState(FOCUSING);
    }

    @Override
    public void cancelAutoFocus() {
        Elog.i(TAG, "cancelAutoFocus");
        mCameraDevice.cancelAutoFocus();
        if (mCameraState != SELFTIMER_COUNTING && mCameraState != SNAPSHOT_IN_PROGRESS) {
            setCameraState(IDLE);
        }
        setFocusParameters();
    }

    private void showEvCalibrationDlg() {
        String title = getString(R.string.camera_ev_calibr_input_tip);
        showSingleInputDlg(title, new InputDialogOnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which, String inputStr) {
                int value = 0;
                boolean validInput = true;
                try {
                    value = Integer.parseInt(inputStr);
                } catch (NumberFormatException e) {
                    validInput = false;
                }
                if (validInput) {
                    Parameters param = mCameraDevice.getParameters();
                    int offset = getParameterValue(KEY_ENG_EV_CALBRATION_OFFSET_VALUE) - value + 50;

                    if (offset < -30 || offset > 30) {
                        String msg = "EV Offset is " + offset + ", ";
                        Toast.makeText(Camera.this, msg + "it must be -30 ~ 30", Toast.LENGTH_SHORT)
                                .show();
                    } else {
                        String msg = "EV Offset:" + offset;
                        Util.showMsgDialog(Camera.this, getString(R.string.camera_ev_calibration),
                                msg);
                        dialog.dismiss();
                    }
                } else {
                    Toast.makeText(Camera.this, "Invalid LV", Toast.LENGTH_SHORT).show();
                }
            }
        }, "");
    }

    // Preview area is touched. Handle touch focus.
    @Override
    public boolean onTouch(View v, MotionEvent e) {
        // Elog.i(TAG, "onTouch ");
        if (mPausing || mCameraDevice == null || !mFirstTimeInitialized) {
            return false;
        }
        if(v.equals(mAbortButton)) {
            Elog.i(TAG, "onTouch Abort button");
            mAbortButton.setEnabled(false);
            mHandler.sendEmptyMessage(EVENT_ABORT_CAPTURE);
            return true;
        }

        if (mCameraState == SNAPSHOT_IN_PROGRESS || mCameraState == PREVIEW_STOPPED
                || mCameraState == SAVING_PICTURES) {
            Elog.i(TAG, "onTouch return");
            return false;
        }
        if (v.equals(mShutterButton)) {
            if (e.getAction() == MotionEvent.ACTION_UP) {
                if (mEvCalibration == AutoCalibration.ID_EV_CALIBRATION_ACTION) {
                    showEvCalibrationDlg();
                    return true;
                }
                mShutterButton.setEnabled(false);
                Elog.i(TAG, "mShutterButton onTouch, disabled");
                if (mGisCalibration == AutoCalibration.ID_GIS_CALIBRATION_ACTION) {
                    Elog.i(TAG, "mGisCalibration onTouch");
                    mShutterButtonEnabled = false;
                    // Stop preview.
                    stopPreview();
                    setPreviewDisplay(null);
                    mHandler.sendEmptyMessage(MSG_ID_GIS_CALIBRATION_PROC);
                    return true;
                }
                Elog.i(TAG, "==== mCaptureBtn key up! ====");
                if (mDBMode != DB3A.DB3A_OFF && mDBMode != DB3A.DB3A_AF) {
                    mHandler.sendEmptyMessage(MSG_ID_CHECK_CAPTURE_STATE);
                } else {
                    mHandler.sendEmptyMessage(EVENT_START_CAPTURE);
                    if (mCaptureMode == 0) {
                        mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                    } else if (mCaptureMode == 2) { // video raw dump
                        mHandler.sendEmptyMessage(EVENT_PREVIEW_RAW_DUMP);
                    }
                }
            }
            return true;
        }
        if (v.getId() == R.id.camera_preview) {
            if (e.getAction() == MotionEvent.ACTION_UP) {
                if (mShutterButtonEnabled) {
                    Elog.i(TAG, "surfaceView touch up!");
                    return mFocusManager.onSingleTapUpPreview(v, e);
                }
            }
        }
        return true;
        // now no need support touch foucs
        // else {
        // // Check if metering area or focus area is supported.
        // if (!mFocusAreaSupported && !mMeteringAreaSupported) {
        // Elog.i(TAG, "onTouch return2");
        // return false;
        // }
        //
        // String focusMode = mParameters.getFocusMode();
        // if (focusMode == null ||
        // Parameters.FOCUS_MODE_INFINITY.equals(focusMode)) {
        // Elog.i(TAG, "onTouch return3");
        // return false;
        // }
        // return mFocusManager.onTouch(e);
        // }
    }

    @Override
    public void onBackPressed() {
        if (!isCameraIdle() || !mTakePicDone) {
            return;
        }
        Elog.v(TAG, "onBackPressed");
        mPausing = true;
        super.onBackPressed();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // Make sure we have a surface in the holder before proceeding.
        if (holder != null && holder.getSurface() == null) {
            Elog.i(TAG, "holder.getSurface() == null");
            return;
        }
        Elog.i(TAG, "surfaceChanged. w=" + width + ". h=" + height);

        // We need to save the holder for later use, even when the mCameraDevice
        // is null. This could happen if onResume() is invoked after this
        // function.
        if (holder == null) {
            Elog.i(TAG, "surfaceChanged. holder == null");
        }
        mSurfaceHolder = holder;

        // The mCameraDevice will be null if it fails to connect to the camera
        // hardware. In this case we will show a dialog and then finish the
        // activity, so it's OK to ignore it.
        if (mCameraDevice == null) {
            return;
        }

        // Sometimes surfaceChanged is called after onPause or before onResume.
        // Ignore it.
        if (mPausing || isFinishing()) {
            return;
        }

        // Set preview display if the surface is being created. Preview was
        // already started. Also restart the preview if display rotation has
        // changed. Sometimes this happens when the device is held in portrait
        // and camera app is opened. Rotation animation takes some time and
        // display rotation in onCreate may not be what we want.
        if (mCameraState == PREVIEW_STOPPED) {
            startPreview();
        } else {
            if (Util.getDisplayRotation(this) != mDisplayRotation) {
                setDisplayOrientation();
            }
            if (holder != null && holder.isCreating()) {
                // Set preview display if the surface is being created and
                // preview
                // was already started. That means preview display was set to
                // null
                // and we need to set it now.
                setPreviewDisplay(holder);
            }
        }

        // If first time initialization is not finished, send a message to do
        // it later. We want to finish surfaceChanged as soon as possible to let
        // user see preview first.
        if (!mFirstTimeInitialized) {
            mHandler.sendEmptyMessage(FIRST_TIME_INIT);
        } else {
            initializeSecondTime();
        }
    }

    public void surfaceCreated(SurfaceHolder holder) {
        Elog.d(TAG, "surfaceCreated.");
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        Elog.i(TAG, "surfaceDestroyed()");
        stopPreview();
        mSurfaceHolder = null;
    }

    private void closeCamera() {
        if (mCameraDevice != null) {
            CameraHolder.instance().release();
            mCameraDevice.setErrorCallback(null);
            mCameraDevice = null;
            setCameraState(PREVIEW_STOPPED);
            mFocusManager.onCameraReleased();
            if (mStereoCameraId > 1) {
                openStereoSensor(false);
            }
        }
    }

    private void openStereoSensor(boolean enabled) {
        if (enabled) {
            String enabledStr = String.valueOf(mStereoCameraId);
            Elog.i(TAG, "vendor.debug.camera.open, enable :" + mStereoCameraId);
            Util.setProperty("vendor.debug.camera.open", enabledStr);
        } else {
            Util.setProperty("vendor.debug.camera.open", "-1");
        }
    }

    private void setPreviewDisplay(SurfaceHolder holder) {
        try {
            mCameraDevice.setPreviewDisplay(holder);
        } catch (IOException ex) {
            closeCamera();
            throw new RuntimeException("setPreviewDisplay failed", ex);
        }
    }

    private void setDisplayOrientation() {
        mDisplayRotation = Util.getDisplayRotation(this);
        mDisplayOrientation = Util.getDisplayOrientation(mDisplayRotation, mCameraId);
        mCameraDevice.setDisplayOrientation(mDisplayOrientation);
    }

    private void startPreview() {
        Elog.i(TAG, "set Preview");
        if (mPausing || isFinishing()) {
            Elog.i(TAG, "startPreview() return");
            return;
        }
        mFlashCaliError = false;

        mFocusManager.resetTouchFocus();
        mCameraDevice.setErrorCallback(mErrorCallback);
        // If we're previewing already, stop the preview first (this will blank
        // the screen).
        if (mCameraState != PREVIEW_STOPPED) {
            stopPreview();
        }
        if (mSurfaceHolder == null) {
            Elog.i(TAG, "startPreview() mSurfaceHolder == null");
        }
        setPreviewDisplay(mSurfaceHolder);
        setDisplayOrientation();
        if (!mSnapshotOnIdle) {
            // If the focus mode is continuous autofocus, call cancelAutoFocus
            // to
            // resume it because it may have been paused by autoFocus call.
            if (Parameters.FOCUS_MODE_CONTINUOUS_PICTURE.equals(mFocusManager.getFocusMode())) {
                mCameraDevice.cancelAutoFocus();
            }
            mFocusManager.setAeAwbLock(false); // Unlock AE and AWB.
        }
        setCameraParameters(UPDATE_PARAM_ALL); // set paramters

        Elog.i(TAG, "==== startPreview ====");
        mCameraDevice.startPreview();
        startFaceDetection();

        setCameraState(IDLE);
        mFocusManager.onPreviewStarted();
        if (mAfMode == AF_MODE_FULL_SCAN) {
            mFocusManager.showIdleFocusView();
        }

        if (!mTakePicDone) {
            mTakePicDone = true;
        }
    }

    private void stopPreview() {
        if (mCameraDevice != null && mCameraState != PREVIEW_STOPPED) {
            Elog.i(TAG, "stopPreview");
            // maybe stop capture(stop3DShot) is ongoing,then it is not allowed
            // to stopPreview.
            mCameraDevice.cancelAutoFocus(); // Reset the focus.
            stopFaceDetection();
            mCameraDevice.stopPreview();
        }
        setCameraState(PREVIEW_STOPPED);
        mFocusManager.onPreviewStopped();
    }

    private static boolean isSupported(String value, List<String> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }

    private void updateCameraParametersInitialize() {
        Elog.i(TAG, "updateCameraParametersInitialize()");
        // Reset preview frame rate to the maximum because it may be lowered by
        // video camera application.
        List<Integer> frameRates = mParameters.getSupportedPreviewFrameRates();
        if (frameRates != null) {
            Integer max = frameRates.stream().max(Integer::compare).get();
            if (max > 30)
                max = 30;
            mParameters.setPreviewFrameRate(max);
        }
        mParameters.setRecordingHint(false);
    }

    private void updateCameraParametersPreference(int updataSet) {
        Elog.i(TAG, "updateCameraParametersPreference() updataSet " + updataSet);

        if (CameraJni.getEmcameraVersion() == CameraJni.MTK_EMCAMERA_VERSION1) {
            String sceneMode = SCENE_STRS_ARRAY[mSceneMode];
            if (isSupported(sceneMode, mParameters.getSupportedSceneModes())) {
                if (!mParameters.getSceneMode().equals(sceneMode)) {
                    mParameters.setSceneMode(sceneMode);
                    Elog.i(TAG, "set SCENE Mode :" + sceneMode);
                    mCameraDevice.setParameters(mParameters);
                    // Setting scene mode will change the settings of flash mode,
                    // white balance, iso speed, and focus mode. Here we read back the
                    // parameters, so we can know those settings.
                    mParameters = mCameraDevice.getParameters();
                }
            } else {
                Elog.i(TAG, "SCENE Mode :" + sceneMode + " don't support!");
            }
        } else {
            if (mVideoHdr == 1
                    && isSupported("hdr", mParameters.getSupportedSceneModes())) {
                Elog.i(TAG, "set SCENE Mode hdr");
                mParameters.setSceneMode("hdr");
            }
        }
        mParameters.setExposureCompensation(0);
        if (mAeLockSupported && mAwbLockSupported) {
            Elog.i(TAG, "[EMCamera]mCaptureDoneNumber:" + mCaptureDoneNumber);
            if (mCaptureDoneNumber == 1 && isLockAeAwbAfMode(mAfMode)) {
                mFocusManager.setAeAwbLock(true);
                mEnableAeAwbLock = true;
            }
            if (mEnableAeAwbLock && !mManualLockAe) {
                mParameters.setAutoExposureLock(mFocusManager.getAeAwbLock());
            }

            if (mEnableAeAwbLock) {
                mParameters.setAutoWhiteBalanceLock(mFocusManager.getAeAwbLock());
            }
            if (mCaptureDoneNumber == 1 && isLockAeAwbAfMode(mAfMode)) {
                mEnableAeAwbLock = false;
            }
        }

        int camOri = CameraHolder.instance().getCameraInfo()[mCameraId].orientation;
        Elog.i(TAG, " Sensor[" + mCameraId + "]'s orientation is " + camOri);
        // EM : set picture size
        setPictureSize(camOri);
        // Set the preview frame aspect ratio according to the picture size.
        Size size = mParameters.getPictureSize();
        Elog.i(TAG, "getPictureSize is " + size.width + "x" + size.height);
        /*
         * Tricky code here, should really careful here. picture size has been swap in camOri == 0
         * || camOri == 180 case here size is restore again to maximize preview.
         */
        double aspectWtoH = 0.0;
        if (size.height > size.width) {
            aspectWtoH = (double) size.height / size.width;
        } else {
            aspectWtoH = (double) size.width / size.height;
        }
        mPreviewPanel = findViewById(R.id.frame_layout);
        mPreviewFrameLayout = (PreviewFrameLayout) findViewById(R.id.frame);
        Elog.i(TAG, "setAspectRatio " + aspectWtoH);
        mPreviewFrameLayout.setAspectRatio(aspectWtoH);

        // Set a preview size that is closest to the viewfinder height and has
        // the right aspect ratio.
        List<Size> sizes = mParameters.getSupportedPreviewSizes();
        Size optimalSize = Util.getOptimalPreviewSize(this, sizes, aspectWtoH);
        Size original = mParameters.getPreviewSize();

        if (!original.equals(optimalSize)) {
            /*
             * Tricky code here, should really careful here. swap width and height when camera
             * orientation = 0 or 180
             */
            mParameters.setPreviewSize(optimalSize.width, optimalSize.height);

            // Zoom related settings will be changed for different preview
            // sizes, so set and read the parameters to get lastest values
            mCameraDevice.setParameters(mParameters);
            mParameters = mCameraDevice.getParameters();
        }
        Elog.i(TAG, "Preview size is " + optimalSize.width + "x" + optimalSize.height);

        // Set JPEG quality.
        int jpegQuality = CameraProfile.getJpegEncodingQualityParameter(mCameraId,
                CameraProfile.QUALITY_HIGH);
        mParameters.setJpegQuality(jpegQuality);

        if (mMinFps != 0 || mMaxFps != 0) {
            mParameters.setPreviewFpsRange(mMinFps, mMaxFps);
        }

        if (updataSet == UPDATE_PARAM_ALL) { // just start preview do this
            setEmCameraParam();
        }
    }

    private boolean isLockAeAwbAfMode(int afMode) {
        if (afMode == AF_MODE_BRACKET) {
            return true;
        }
        if (afMode == AF_MODE_THROUGH_FOCUS) {
            return true;
        }
        return false;
    }

    private void setCapturePictureSize(int captureRatio) {
        double ratio = 1.3333;
        if (captureRatio == 1) {
            ratio = 1.7778; // Sync with normal camera, 16:9.
        } else {
            ratio = 1.3333; // Sync with normal camera, 4:3.
        }

        Elog.i(TAG, "setCapturePictureSize captureRatio is " + captureRatio + " ratio" + ratio);

        List<Size> supportedSizes = Util.buildSupportedPictureSize(mParameters, ratio);
        if (supportedSizes != null && supportedSizes.size() > 0) {

            Size maxSize = supportedSizes.get(0);
            for (int i = 1; i < supportedSizes.size(); i++) {
                if ((supportedSizes.get(i).height > maxSize.height)
                        || (supportedSizes.get(i).width > maxSize.width)) {
                    maxSize = supportedSizes.get(i);
                }
            }

            mParameters.setPictureSize(maxSize.width, maxSize.height);
        }
    }

    private void setPictureSize(int camOri) {
        Elog.i(TAG, "setPictureSize camOri " + camOri);
        List<Size> pictureSizes = mParameters.getSupportedPictureSizes();
        if (mCaptureMode == 0) {
            if (mCaptureSize == 1 || mCaptureSize == 2 // 1 ~ 2, capture mode
                || (mCaptureSize > 5 && mCaptureSize <= 10)) { // 6 ~ 10 is custom mode.
                setCapturePictureSize(mCaptureRatio);
            } else if (mCaptureSize == 0) { // preview mode
                Size maxSize = pictureSizes.get(0);
                for (int i = 1; i < pictureSizes.size(); i++) {
                    if ((pictureSizes.get(i).height > maxSize.height)
                            || (pictureSizes.get(i).width > maxSize.width)) {
                        maxSize = pictureSizes.get(i);
                    }
                }
                Elog.i(TAG, "Max picture size is " + maxSize.width + "x" + maxSize.height);
                int x = maxSize.width / 2;
                int y = maxSize.height / 2;
                Size previewSize = Util.getOptimalPreviewCaptureSize(x, y, pictureSizes);
                previewSize = maxSize;
                Elog.i(TAG, "preview size is " + previewSize.width + "x" + previewSize.height);
                mParameters.setPictureSize(previewSize.width, previewSize.height);
            } else { // 3,4,5 video mode
                CamcorderProfile profile = null;
                List<Size> videoSizes = mParameters.getSupportedVideoSizes();
                int quality = Util.getFirstSupportedVideoQuality(mCameraId, videoSizes);
                Elog.i(TAG, "CamcorderProfile getProfile = " + quality);
                profile = CamcorderProfile.get(mCameraId, quality);

                if (profile != null) {
                    int x = profile.videoFrameWidth;
                    int y = profile.videoFrameHeight;
                    Size videoSize = Util.getOptimalVideoCaptureSize(x, y, pictureSizes);
                    if(videoSize != null) {
                        mParameters.setPictureSize(videoSize.width, videoSize.height);
                        Elog.i(TAG, "set video size is " + videoSize.width + "x"
                                + videoSize.height);
                    } else {
                        Elog.w(TAG, "PictureSize don't support quality:" + quality);
                    }
                } else {
                    Elog.w(TAG, "Get profile NULL");
                }
            }
        }
    }

    boolean isSupportSize(List<Size> supported, int width, int height) {
        for (Size size : supported) {
            if (size.width == width && size.height == height) {
                return true;
            }
        }
        return false;
    }

    private void setEmCameraParam() {
        Elog.i(TAG, "setEmCameraParam() set EM parameters.");
        long dateTaken = System.currentTimeMillis();
        if (mCaptureMode == 0) {
            mParameters.set(KEY_ENG_CAMERA_MODE, KEY_ENG_CAMERA_MODE_NORMAL);
            if (mCaptureType == 2) {
                mParameters.set(KEY_RAW_SAVE_MODE, RAW_SAVE_JPEG);
                mRawCaptureFileName = "JpegOnly";
            } else {
                mRawCaptureFileName = mCaptureNamePrefix[mCaptureSize];
                if (mCaptureSize == 2) { // Capture Size(ZSD)
                    mParameters.set(KEY_ENG_ZSD_ENABLE, 1);
                    mParameters.set(KEY_RAW_SAVE_MODE, RAW_SAVE_CAPTURE); // Force save mode is
                                                                          // capture size
                } else {
                    mParameters.set(KEY_RAW_SAVE_MODE, mCaptureSize + 1);
                }
                mParameters.set(KEY_ISP_MODE, mCaptureType);
            }
            mParameters.set(KEY_ENG_ISP_PROFILE, ispProfile);
            if (getFeatureSupported(KEY_ENG_MTK_AWB_SUPPORTED)) {
                mParameters.set(KEY_ENG_MTK_AWB_ENABLE, mAwbVeri & 1);
            }
            if (getFeatureSupported(KEY_ENG_SENSOR_AWB_SUPPORTED)) {
                mParameters.set(KEY_ENG_SENSOR_AWB_ENABLE, (mAwbVeri >> 1) & 1);
            }
            mParameters.set(KEY_ENG_MANUAL_SHUTTER_SPEED, mShutterSpeed);
            mParameters.set(KEY_ENG_MANUAL_SENSOR_GAIN, mSensorGain);
            mParameters.set(KEY_ENG_FLASH_CALIBRATION, mFlashCali);
            if (CameraJni.getEmcameraVersion() == CameraJni.MTK_EMCAMERA_VERSION1) {
                Elog.i(TAG, "EmCamera version1; MFLL/Shading/scene mode");
                if (getFeatureSupported(KEY_ENG_MFLL_SUPPORTED)) {
                    if (mMfllMode == 2) {
                        mParameters.set(KEY_MFB_MODE, OFF);
                    } else {
                        mParameters.set(KEY_MFB_MODE, KEY_MFB_MODE_MFLL);
                        mParameters.set(KEY_ENG_MFLL_PICTURE_COUNT, mMfllCount);
                    }
                }
                if (getFeatureSupported(KEY_ENG_MTK_1TO3_SHADING_SUPPORTED)) {
                    mParameters.set(KEY_ENG_MTK_1TO3_SHADING_ENABLE, (mShadingVeri >> 1) & 1);
                }
                if (getFeatureSupported(KEY_ENG_SENSOR_SHADNING_SUPPORTED)) {
                    mParameters.set(KEY_ENG_SENSOR_SHADNING_ENABLE, (mShadingVeri >> 2) & 1);
                }
                setSceneModeRule(); // sync with normal camera.
            }
        } else if (mCaptureMode == 2) {
            mParameters.set(KEY_ENG_CAMERA_MODE, KEY_ENG_CAMERA_MODE_MTK_PRV);
            mRawCaptureFileName = "VideoClip";
            // mParameters.set(KEY_RAW_SAVE_MODE, 4);
            mParameters.set(KEY_ENG_RAW_OUTPUT_PORT, videoDumpBuffer);
            mParameters.set(KEY_ENG_ISP_PROFILE, ispProfile);
            mParameters.set(KEY_RAW_SAVE_MODE, sensorMode);
            // Todo:+ // *** Force 1920 * 1080p *** Need to Modify
            mParameters.setVideoStabilization(false);
            Elog.i(TAG, "VideoClip: mParameters.setVideoStabilization(false);");
            mParameters.setRecordingHint(true);
            Elog.i(TAG, "VideoClip: mParameters.setRecordingHint(true);");
            mParameters.setPreviewSize(1920, 1080);
            Elog.i(TAG, "VideoClip: Preview size is 1920 x 1080");
            // Todo-
            if (getFeatureSupported(KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_SUPPORTED)) {
                mParameters.set(KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_ENABLE, mEnableFameRate);
                mParameters.set(KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_RANGE_LOW, mLowFrameRate);
                mParameters
                        .set(KEY_ENG_VIDEO_RAW_DUMP_MANUAL_FRAME_RATE_RANGE_HIGH, mHighFrameRate);
            }
            mParameters.set(KEY_ENG_VIDEO_RAW_DUMP_RESIZE, 0);
            if (mVideoResolution == PREVIEW_DUMP_RESOLUTION_NORMAL
                    || mVideoResolution == PREVIEW_DUMP_RESOLUTION_CROP) {
                mParameters.set(KEY_PREVIEW_DUMP_RESOLUTION, mVideoResolution);
            } else if (mVideoResolution == PREVIEW_DUMP_RESOLUTION_RESIZE_TO_4K2K
                    || mVideoResolution == PREVIEW_DUMP_RESOLUTION_CROP_CENTER_2M) {
                Elog.i(TAG, "set 3840x2160 video size");
                mParameters.set(KEY_VIDEO_SIZE, "3840x2160");
                if (mVideoResolution == PREVIEW_DUMP_RESOLUTION_RESIZE_TO_4K2K) {
                    mParameters.set(KEY_ENG_VIDEO_RAW_DUMP_RESIZE, 1); // Resize to 4K2K
                } else {
                    mParameters.set(KEY_ENG_VIDEO_RAW_DUMP_RESIZE, 2); // Crop at center to 2M
                }
            }
        }
        if (mDBMode == DB3A.DB3A_FLASH) { // 3A - FLASH
            if (mFlashCount == 0 && mSaveCorrectFlag == 1) { // no flash
                Elog.i(TAG, "[3A - FLASH] In mFlashCount = 0");
                mCameraImageName = mCameraImageName + createName(dateTaken) + "/";
                // storage/sdcard1/DCIM/CameraEM/FlashAwb/xxx/
                Elog.i(TAG, "[3A - FLASH]  mCameraImageName = " + mCameraImageName);
                Elog.i(TAG, "[3A - FLASH]  mRawCaptureFileName = " + mRawCaptureFileName);// Capture
                Elog.i(TAG, "[3A - FLASH]  createName(dateTaken) = " + createName(dateTaken));

                mRawCaptureFileName = mCameraImageName + mRawCaptureFileName
                        + createName(dateTaken);
                // storage/sdcard1/DCIM/CameraEM/FlashAwb/xxx/Capture20130101-003245
                Elog.i(TAG, "[3A - FLASH]  mRawCaptureFileName = " + mRawCaptureFileName);

                if (mTempFlashCount == 0) {
                    mTempCameraImageName = mCameraImageName;
                    mTempRawCaptureFileName = mRawCaptureFileName;
                    mRawCaptureFileName = mRawCaptureFileName + "flashoff";

                    Elog.i(TAG, "[3A - FLASH] Save current file name");
                    mTempFlashCount = 1;
                } else if (mTempFlashCount == 1) {
                    Elog.i(TAG, "[3A - FLASH] Enter mTempFlashCount == 1");
                    mRawCaptureFileName = mTempRawCaptureFileName + "flashon";
                    // storage/sdcard1/DCIM/CameraEM/FlashAwb/xxx/Capture20130101-003245
                    Elog.i(TAG, "[3A - FLASH]  mRawCaptureFileName = " + mRawCaptureFileName);
                    mTempFlashCount = 0;
                }
                mSaveCorrectFlag = 0;
            }
        } else {
            mRawCaptureFileName = mCameraImageName + mRawCaptureFileName + createName(dateTaken);
        }
        // set focus mode
        if (isSupportAf(mParameters)) {
            String focusMode = "";
            if (mAfMode == AF_MODE_AUTO) {
                focusMode = Parameters.FOCUS_MODE_AUTO;
            } else if (mAfMode == AF_MODE_BRACKET) {
                if (mBracketTimes == 0) {
                    Elog.i(TAG, "AF_MODE_BRACKET(Parameters.FOCUS_MODE_AUTO):" + mBracketTimes);
                    focusMode = Parameters.FOCUS_MODE_AUTO;
                } else {
                    Elog.i(TAG, "AF_MODE_BRACKET(Parameters.KEY_ENG_FOCUS_MODE_MANUAL) " + mBracketTimes);
                    focusMode = KEY_ENG_FOCUS_MODE_MANUAL;
                }
            } else if (mAfMode == AF_MODE_FULL_SCAN) {
                focusMode = KEY_ENG_FOCUS_MODE_FULLSCAN;
                Elog.i(TAG, "setEngFocusFullScanFrameInterval(" + mAfFullScanFrameInterval + ")");
                mParameters.set(KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL, mAfFullScanFrameInterval);
                Elog.i(TAG, "set focus-fs-dac-step: " + mAfFullScanDacStep);
                mParameters.set(KEY_ENG_AF_FS_DAC_STEP, mAfFullScanDacStep);
            } else if (mAfMode == AF_MODE_THROUGH_FOCUS) {
                focusMode = KEY_ENG_FOCUS_MODE_MANUAL;
            } else if (mAfMode == AF_MODE_CONTINUOUS) {
                focusMode = Parameters.FOCUS_MODE_CONTINUOUS_PICTURE;
            } else if (mAfMode == AF_MODE_TEMP_CALI) {
                focusMode = KEY_ENG_FOCUS_MODE_FULLSCAN;
                Elog.i(TAG, "setEngFocusFullScanFrameInterval( 0 )");
                mParameters.set(KEY_ENG_FOCUS_FULLSCAN_FRAME_INTERVAL, 0);
                Elog.i(TAG, "set focus-fs-dac-step: 1");
                mParameters.set(KEY_ENG_AF_FS_DAC_STEP, 1);
            } else {
                Elog.w(TAG, "Unknown mAfMode:" + mAfMode);
            }
            mParameters.setFocusMode(focusMode);
            mFocusManager.overrideFocusMode(focusMode);
        } else {
            mFocusManager.overrideFocusMode(Parameters.FOCUS_MODE_INFINITY);
        }
        // set flicker
        mParameters.setAntibanding(mFlickerString);
        if (mAfMode == AF_MODE_AUTO) { // AF auto has different iso value
            if (mCaptureMode == 2 || (mDBMode == DB3A.DB3A_FLASH)) { // 3A - FLASH
                mParameters.set(KEY_ISOSPEED_MODE, mIsoValues.get(0));
                if (mIsoValues.get(0).equals("0")) {
                    // parameter "0" is iso auto
                    mRawCaptureFileName = mRawCaptureFileName + "ISO" + "Auto";
                } else {
                    mRawCaptureFileName = mRawCaptureFileName + "ISO"
                            + mIsoValues.get(mAfCaptureTimes);
                }
                mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
            } else {
                if (mAfCaptureTimes < mIsoValues.size()) {
                    mIsoValIndex = mAfCaptureTimes;
                    mParameters.set(KEY_ISOSPEED_MODE, mIsoValues.get(mAfCaptureTimes));
                    Elog.i(TAG, "EVENT_CAPTURE_ACTION iso speed " + mIsoValues.get(mAfCaptureTimes));
                    if (mIsoValues.get(mAfCaptureTimes).equals("0")) {
                        // parameter "0" is iso auto
                        mRawCaptureFileName = mRawCaptureFileName + "ISO" + "Auto";
                    } else {
                        mRawCaptureFileName = mRawCaptureFileName + "ISO"
                                + mIsoValues.get(mAfCaptureTimes);
                    }
                    if (mCaptureType != 2) {
                        mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
                    }
                }
            }
        } else { // AF full scan, bracket, through focus just has one iso value
            Elog.i(TAG, "EVENT_CAPTURE_ACTION mAfMode != 0 ");
            mParameters.set(KEY_ISOSPEED_MODE, mIsoValue);
            if (mIsoValue.equals("0")) {
                mRawCaptureFileName = mRawCaptureFileName + "ISO" + "Auto";
            } else {
                mRawCaptureFileName = mRawCaptureFileName + "ISO" + mIsoValue;
            }
            if (mCaptureType != 2) {
                mParameters.set(KEY_RAW_PATH, mRawCaptureFileName + ".raw");
            }
        }
        // set strobe mode
        if (isStrobeSupported(mParameters)) {
            mParameters.set(KEY_FLASH_MODE, mStrobeMode);
        }
        Elog.i(TAG, "setEmCameraParam mRawCaptureFileName " + mRawCaptureFileName);
        if (CameraJni.getEmcameraVersion() == CameraJni.MTK_EMCAMERA_VERSION1) {
            Elog.i(TAG, "setEmCameraParam mShadingTable:" + mShadingTable + " mOutputSdblk:"
                    + mOutputSdblk);
            if (mShadingTable == 5) {
                // "Disable"
                mParameters.set(KEY_ENG_MTK_SHADING_ENABLE, 0);
            } else {
                mParameters.set(KEY_ENG_MTK_SHADING_ENABLE, 1);
                mParameters.set(KEY_ENG_SHADING_TABLE, mShadingTable);
            }
            mParameters.set(KEY_ENG_SAVE_SHADING_TABLE, mOutputSdblk);

            if (getFeatureSupported(KEY_ENG_MULTI_NR_SUPPORTED)) {
                mMnrSupport = true;
                if (mMultiPass == 0) {
                    mParameters.set(KEY_ENG_MULTI_NR_ENABLE, 0);
                } else { // 1, 2, 3
                    mParameters.set(KEY_ENG_MULTI_NR_ENABLE, 1);
                    mParameters.set(KEY_ENG_MULTI_NR_TYPE, mMultiPass - 1);
                }
            }
        } else {
            Elog.i(TAG, "setEmCameraParam mtk-shad-e:1, eng-shad-t:4, eng-s-shad-t:1");
            mParameters.set(KEY_ENG_MTK_SHADING_ENABLE, 1);
            mParameters.set(KEY_ENG_SHADING_TABLE, 4);
            mParameters.set(KEY_ENG_SAVE_SHADING_TABLE, 1);
        }

        mParameters.set(KEY_ENG_VIDEO_HDR, mVideoHdr == 0 ? "off" : "on");
        if (mVideoHdr == 1) {
            mParameters.set(KEY_ENG_VIDEO_HDR_MODE, mVHdrMode);
            mParameters.set(KEY_ENG_VIDEO_HDR_RATIO, mVHdrRatio);
        }
    }

    private String createName(long dateTaken) {
        return DateFormat.format("yyyyMMdd-kkmmss", dateTaken).toString();
    }

    // We separate the parameters into several subsets, so we can update only
    // the subsets actually need updating. The PREFERENCE set needs extra
    // locking because the preference can be changed from GLThread as well.
    private void setCameraParameters(int updateSet) {
        Elog.i(TAG, "setCameraParameters() updateSet " + updateSet);
        mParameters = mCameraDevice.getParameters();
        if ((updateSet & UPDATE_PARAM_INITIALIZE) != 0) {
            updateCameraParametersInitialize();
        }
        if ((updateSet & UPDATE_PARAM_PREFERENCE) != 0) {
            // if (mFirstTimeInitialized) {
            checkStorage();
            // }
            updateCameraParametersPreference(updateSet);
            setIsFocusCallback(false);
        }
        if (mEnableGisCalibration) {
            mParameters.setVideoStabilization(true);
            List<Size> sizes = mParameters.getSupportedPreviewSizes();
            if (isSupportSize(sizes, 1920, 1080)) {
                mParameters.setPreviewSize(1920, 1080);
            }
        } else {
            mParameters.setVideoStabilization(false);
        }
        mCameraDevice.setParameters(mParameters);
    }

    private boolean isCameraIdle() {
        return (mCameraState == IDLE) || (mFocusManager.isFocusCompleted());
    }

    @Override
    public void onUserInteraction() {
        super.onUserInteraction();
        keepScreenOnAwhile();
    }

    private void resetScreenOn() {
        mHandler.removeMessages(CLEAR_SCREEN_DELAY);
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    private void keepScreenOnAwhile() {
        mHandler.removeMessages(CLEAR_SCREEN_DELAY);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        // mHandler.sendEmptyMessageDelayed(CLEAR_SCREEN_DELAY, SCREEN_DELAY);
    }

    private void initializeCapabilities() {
        Elog.i(TAG, "initializeCapabilities()");
        mInitialParams = mCameraDevice.getParameters();
        mFocusManager.initializeParameters(mInitialParams);
        mFocusAreaSupported = (mInitialParams.getMaxNumFocusAreas() > 0 && isSupported(
                Parameters.FOCUS_MODE_AUTO, mInitialParams.getSupportedFocusModes()));
        mMeteringAreaSupported = mInitialParams.getMaxNumMeteringAreas() > 0;
        mAeLockSupported = mInitialParams.isAutoExposureLockSupported();
        mAwbLockSupported = mInitialParams.isAutoWhiteBalanceLockSupported();
    }

    public void resumePreview() {
        if (mCameraState != IDLE) {
            startPreview();
        }
        checkStorage();
    }

    public int getCameraId() {
        return mCameraId;
    }

    public long getPictureRemaining() {
        return mPicturesRemaining;
    }

    protected Parameters getCameraParameters() {
        return mParameters;
    }

    public PreviewFrameLayout getPreviewFrameLayout() {
        return mPreviewFrameLayout;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    public int getOrientation() {
        return mOrientation;
    }

    public long getRemainPictures() {
        return mPicturesRemaining;
    }

    public boolean checkCameraState() {
        Elog.i(TAG, "Check camera state in ModeActor");
        if (mPausing) {
            return false;
        }

        // If the user wants to do a snapshot while the previous one is still
        // in progress, remember the fact and do it after we finish the previous
        // one and re-start the preview. Snapshot in progress also includes the
        // state that autofocus is focusing and a picture will be taken when
        // focus callback arrives.
        if (mFocusManager.isFocusingSnapOnFinish() || mCameraState == SNAPSHOT_IN_PROGRESS
                || mCameraState == SAVING_PICTURES) {
            return false;
        }
        mSnapshotOnIdle = false;
        return true;
    }

    static boolean isSupportAf(Parameters parameters) {
//        MaxFocusAreaNum = 0 Fixed focus or not support AF
//                          1 AF
        return parameters.getMaxNumFocusAreas() > 0;
    }

    static boolean isStrobeSupported(Parameters parameters) {
        boolean isSupport = true;
        List<String> list = parameters.getSupportedFlashModes();
        if (null == list) {
            isSupport = false;
        } else {
            if (list.size() == 1 && list.get(0).equals(Parameters.FLASH_MODE_OFF)) {
                isSupport = false;
            }
        }
        return isSupport;
    }

    static boolean isRawSensor(int facing, Parameters parameters) {
        boolean isRaw = false;

        int sensorType = 0;
        try {
            sensorType = Integer.parseInt(parameters.get(KEY_SENSOR_TYPE));
        } catch (NumberFormatException ex) {
            Elog.e(TAG, "SENSOR_TYPE NumberFormatException - "
                    + parameters.get(KEY_SENSOR_TYPE) );
        }
        if (facing == CameraInfo.CAMERA_FACING_BACK) {
            if ((sensorType & 1) == 0) {
                isRaw = true;
            }
        } else if (facing == CameraInfo.CAMERA_FACING_FRONT) {
            if (((sensorType >> 1) & 1) == 0) {
                isRaw = true;
            }
        }

        return isRaw;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        if (mParameters != null) {
            if (mParameters.isAutoExposureLockSupported()) {
                menu.add(Menu.NONE, MENU_ID_AE, Menu.NONE, "Lock " + getString(R.string.camera_ae));
            }

            if (!(mParameters.getMinExposureCompensation() == 0 && mParameters
                    .getMinExposureCompensation() == 0)) {
                menu.add(Menu.NONE, MENU_ID_EV, Menu.NONE, getString(R.string.camera_ev));
            }

            if (isStrobeSupported(mParameters)) {
                menu.add(Menu.NONE, MENU_ID_STROBE_DUTY, Menu.NONE,
                        getString(R.string.camera_strobe_duty));
            }
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        CharSequence title;
        switch (id) {
        case MENU_ID_AE:
            title = item.getTitle();
            if (title.equals("Lock " + getString(R.string.camera_ae))) {
                mParameters.setAutoExposureLock(true);
                mManualLockAe = true;
            } else {
                mParameters.setAutoExposureLock(false);
                mManualLockAe = false;
            }
            mCameraDevice.setParameters(mParameters);
            break;
        case MENU_ID_EV:
            title = item.getTitle() + " setting";
            showSingleInputDlg(title, new InputDialogOnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which, String inputStr) {
                    int ev = 0;
                    boolean validInput = true;
                    int minev = mParameters.getMinExposureCompensation();
                    int maxev = mParameters.getMaxExposureCompensation();

                    try {
                        ev = Integer.valueOf(inputStr);
                    } catch (NumberFormatException e) {
                        validInput = false;
                    }
                    if (!validInput || ev < minev || ev > maxev) {
                        Toast.makeText(Camera.this,
                                "invalid ev, must range from " + minev + " to " + maxev,
                                Toast.LENGTH_SHORT).show();
                        return;
                    }
                    mParameters.setExposureCompensation(ev);
                    mCameraDevice.setParameters(mParameters);
                    dialog.dismiss();
                }
            }, String.valueOf(mParameters.getExposureCompensation()));
            break;
        case MENU_ID_STROBE_DUTY:
            title = item.getTitle() + " setting";
            showSingleInputDlg(title, new InputDialogOnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which, String inputStr) {
                    int minDuty = getParameterValue(KEY_ENG_FLASH_DUTY_MIN);
                    int maxDuty = getParameterValue(KEY_ENG_FLASH_DUTY_MAX);
                    boolean validInput = true;
                    int duty = -1;
                    try {
                        duty = Integer.valueOf(inputStr);
                    } catch (NumberFormatException e) {
                        validInput = false;
                    }
                    if (!validInput || duty < minDuty || duty > maxDuty) {
                        Toast.makeText(
                                Camera.this,
                                "invalid strobe duty, must range from " + minDuty + " to "
                                        + maxDuty, Toast.LENGTH_SHORT).show();
                        return;
                    }
                    mStrobeDuty = inputStr;
                    mParameters.set(KEY_ENG_FLASH_DUTY_VALUE, duty);
                    mCameraDevice.setParameters(mParameters);
                    dialog.dismiss();
                }
            }, mStrobeDuty);
            break;
        default:
            Elog.w(TAG, "onOptionsItemSelected unknown menu item id:" + id);
            break;
        }
        return true;
    }

    private interface InputDialogOnClickListener {
        public void onClick(DialogInterface dialog, int which, String inputStr);
    }

    private void showSingleInputDlg(CharSequence title, final InputDialogOnClickListener listener,
            String defVal) {
        View inputView = getLayoutInflater().inflate(R.layout.em_single_input_layout, null);
        Button okBtn = (Button) inputView.findViewById(R.id.em_single_input_ok_btn);
        final EditText inputEdit = (EditText) inputView.findViewById(R.id.em_single_input_edit);
        if (defVal != null) {
            inputEdit.setText(defVal);
        }
        final AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(true)
                .setTitle(title).setView(inputView).create();
        okBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                listener.onClick(dialog, DialogInterface.BUTTON_POSITIVE, inputEdit.getText()
                        .toString());
            }
        });
        dialog.show();
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem aeItem = menu.findItem(MENU_ID_AE);
        if (aeItem != null) {
            if (mParameters.getAutoExposureLock()) {
                aeItem.setTitle("Unlock " + getString(R.string.camera_ae));
            } else {
                aeItem.setTitle("Lock " + getString(R.string.camera_ae));
            }
        }
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN) { // sync with onTouch for shutter up
            if (mPausing || mCameraDevice == null || !mFirstTimeInitialized
                    || mShutterButtonEnabled == false) {
                Elog.i(TAG, "onKeyUp return");
                return false;
            }
            Elog.i(TAG, "Volume key up for capture!");
            if (mDBMode != DB3A.DB3A_OFF && mDBMode != DB3A.DB3A_AF) {
                mHandler.sendEmptyMessage(MSG_ID_CHECK_CAPTURE_STATE);
            } else {
                mHandler.sendEmptyMessage(EVENT_START_CAPTURE);
                if (mCaptureMode == 0) {
                    mHandler.sendEmptyMessage(EVENT_CAPTURE_ACTION);
                    Elog.i(TAG, "EVENT_CAPTURE_ACTION - 1!");
                } else if (mCaptureMode == 2) { // video raw dump
                    mHandler.sendEmptyMessage(EVENT_PREVIEW_RAW_DUMP);
                }
            }
            return false;
        }
        return super.onKeyUp(keyCode, event);
    }

    private boolean getFeatureSupported(String para) {
        String str = mParameters.get(para);
        String sTRUE = "true";
        Elog.v(TAG, "getFeatureSupport - " + para + " is " + str);
        return sTRUE.equals(str);
    }

    private int getParameterValue(String para) {
        int val = 0;
        try {
            val = mParameters.getInt(para);
        } catch (NumberFormatException ex) {
//            Elog.i(TAG, "getParameterValue - " + para + " is NumberFormatException");
        }
        return val;
    }

    private void setSceneModeRule() {
        if (mSceneMode == SCENE_MODE_THEATRE) {
            mParameters.set(KEY_SATURATION_MODE, "low");    // saturation
        }
        if (mSceneMode == SCENE_MODE_PORT || mSceneMode == SCENE_MODE_NIGHT
                || mSceneMode == SCENE_MODE_NIGHTPORT) {
            mParameters.set(KEY_EDGE_MODE, "low");  // sharpness
        } else if (mSceneMode == SCENE_MODE_LAND || mSceneMode == SCENE_MODE_THEATRE
                || mSceneMode == SCENE_MODE_BEACH || mSceneMode == SCENE_MODE_SNOW
                || mSceneMode == SCENE_MODE_SUNSET) {
            mParameters.set(KEY_EDGE_MODE, "high");  // sharpness
        }
        if (mSceneMode == SCENE_MODE_LAND || mSceneMode == SCENE_MODE_SUNSET) {
            mParameters.setWhiteBalance("daylight"); // white balance
        } else if (mSceneMode == SCENE_MODE_CANDLE) {
            mParameters.setWhiteBalance("incandescent");
        }
        if (mSceneMode == SCENE_MODE_BEACH || mSceneMode == SCENE_MODE_SNOW) {
            mParameters.setExposureCompensation(1); // exposure
        }
    }

    private GisCaliTask mGisCaliTask;
    private void cameraGisCalProc() {
        mGisCaliTask = new GisCaliTask();
        mGisCaliTask.execute();
    }

    private class GisCaliTask extends AsyncTask<String, Void, Boolean> {
        @Override
        protected Boolean doInBackground(String... params) {
            Elog.i(TAG, "cameraGisCalProc - " + GIS_INFO_FILE);
            // Step1 delete "storage/sdcard0/gyro/CalibrationInfo.txt"
            File file = new File(GIS_INFO_FILE);
            if (file.isFile() && file.exists()) {
                boolean result = file.delete();
                Elog.i(TAG, "CalibrationInfo.txt deleted = " + result);
            }
            // Step2 Setprop debug.eis.EMEnabled 1
            Util.setProperty(ROPERTY_GIS_CALIBRATION, "1");
            Elog.i(TAG, "cameraGisCalProc - set vendor.debug.eid.EMmode property");
            long mGisCalibrationTimeStart = System.currentTimeMillis();
            long mGisCalibrationTimeEnd = mGisCalibrationTimeStart;
            File fileData = new File(GIS_INFO_FILE);
            // Restart preview
            setPreviewDisplay(mSurfaceHolder);
            startPreview();
            while ((mGisCalibrationTimeEnd - mGisCalibrationTimeStart) < 10 * 1000) {
                if (fileData.isFile() && fileData.exists()) {
                    Elog.i(TAG, "cameraGisCalProc - file exit");
                    Util.setProperty(ROPERTY_GIS_CALIBRATION, "0");
                    return true;
                } else {
                    mGisCalibrationTimeEnd = System.currentTimeMillis();
                }
                if(isCancelled()) {
                    break;
                }
            }

            Elog.i(TAG, "cameraGisCalProc - exit while");
            Util.setProperty(ROPERTY_GIS_CALIBRATION, "0");
            return false;
        }

        @Override
        protected void onPostExecute(Boolean gisCaliOk) {
            Elog.i(TAG, "onPostExecute " + gisCaliOk);
            if(mPausing){
                return;
            }

            if(gisCaliOk) {
                cameraGisCalFinish();
            } else {
                showCameraGisCalInfoDlg(getString(R.string.gis_cali_fail_tittle),
                        getString(R.string.gis_cali_fail_msg, GIS_INFO_FILE));
            }
            mShutterButton.setEnabled(true);
            mShutterButtonEnabled = true;
        }
    }

    private void cameraGisCalFinish() {
        String info = readFile(GIS_INFO_FILE);
        showCameraGisCalInfoDlg(getString(R.string.gis_cali_fail_result), info);
        File fileInfo = new File(GIS_INFO_FILE);
        if (fileInfo.isFile() && fileInfo.exists()) {
            fileInfo.delete();
        }
        File fileDefault = new File(GIS_PARAM_FILE);
        if (fileDefault.isFile() && fileDefault.exists()) {
            if (mCameraId == 0) {
                copyFile(GIS_PARAM_FILE, GIS_PARAM_MAIN_FILE);
            } else if (mCameraId == 1) {
                copyFile(GIS_PARAM_FILE, GIS_PARAM_SUB_FILE);
            } else if (mCameraId == 2) {
                copyFile(GIS_PARAM_FILE, GIS_PARAM_MAIN2_FILE);
            } else if (mCameraId == 3) {
                copyFile(GIS_PARAM_FILE, GIS_PARAM_SUB2_FILE);
            }
            fileDefault.delete();
        }
    }

    private String readFile(String fileName) {
        String res = "";
        File fHandle = new File(fileName);
        try {
            FileInputStream fis = new FileInputStream(fHandle);
            int length = fis.available();
            byte[] buffer = new byte[length];
            fis.read(buffer);
            res = new String(buffer);
            fis.close();
        } catch (IOException ex) {
            Elog.e(TAG, "readFile IOException");
            fHandle.delete();
        }
        return res;
    }

    private void copyFile(String oldPath, String newPath) {
        File oldfile = new File(oldPath);
        try {
            int bytesum = 0;
            int byteread = 0;
            if (oldfile.exists()) {
                FileInputStream inStream = new FileInputStream(oldfile);
                FileOutputStream fos = new FileOutputStream(newPath);
                int length = inStream.available();
                byte[] buffer = new byte[length];
                inStream.read(buffer);
                fos.write(buffer);
                inStream.close();
                fos.close();
            }
        } catch (IOException ex) {
            Elog.e(TAG, "copyFile IOException");
        }

    }

    private void showCameraGisCalInfoDlg(String title, String message) {
        AlertDialog.Builder dialog = new AlertDialog.Builder(this);
        dialog.setTitle(title);
        dialog.setMessage(message);
        dialog.setCancelable(false);
        dialog.setPositiveButton(android.R.string.ok, new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (which == DialogInterface.BUTTON_POSITIVE) {
                    dialog.dismiss();
                }
            }
        });
        dialog.show();
    }

    @Override
    public void startFaceDetection() {
        if (!mFaceDetectionEnabled) {
            return;
        }

        Elog.i(TAG, "[startFaceDetection] mFaceDetectionRunning " + mFaceDetectionRunning);
        synchronized (mFaceDetectionSync) {
            if (!mFaceDetectionRunning) {
                Elog.i(TAG, "[startFaceDetection]will call startFaceDetection ");
                mCameraDevice.setFaceDetectionListener(mFaceDetectionListener);
                mFocusManager.resetFaceView();
                mCameraDevice.startFaceDetection();
                mFaceDetectionRunning = true;
            } else {
                Elog.w(TAG, "[startFaceDetection] called twice");
            }
        }
    }

    @Override
    public void stopFaceDetection() {
        if (!mFaceDetectionEnabled) {
            return;
        }

        Elog.i(TAG, "[stopFaceDetection] mFaceDetectionRunning " + mFaceDetectionRunning);
        synchronized (mFaceDetectionSync) {
            if (mFaceDetectionRunning) {
                Elog.i(TAG, "[stopFaceDetection]will call stopFaceDetection ");
                mCameraDevice.stopFaceDetection();
                mCameraDevice.setFaceDetectionListener(null);
                mFaceDetectionRunning = false;
            } else {
                Elog.w(TAG, "[stopFaceDetection]called twice");
            }
        }
    }

    private FaceDetectionListener mFaceDetectionListener = new FaceDetectionListener() {
        @Override
        public void onFaceDetection(Face[] faces, android.hardware.Camera camera) {
            Elog.i(TAG, "[onFaceDetection]length = " + faces.length);
            if (mFaceDetectionEnabled && mFaceDetectionRunning) {
                mFaceView.setFaces(faces);
            }
            return;
        }
    };

    private void mkdirs(File dir) {
        if(dir.isDirectory() && dir.exists()) {
            return;
        }
        dir.mkdirs();
    }

}
