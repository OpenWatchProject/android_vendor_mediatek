package com.android.camera;

import android.app.ActivityManager;
import android.app.Service;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ContentProviderClient;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.hardware.Camera.CameraInfo;
import android.hardware.Camera.Face;
import android.hardware.Camera.Parameters;
import android.location.Location;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.MessageQueue;
import android.os.PowerManager;
import android.os.SystemClock;
import android.os.Vibrator;
import android.os.storage.StorageVolume;
import android.provider.MediaStore;
import android.support.v4.app.ActivityCompat;
import android.view.KeyEvent;
import android.view.OrientationEventListener;
import android.view.Surface;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Toast;

import com.android.camera.GestureDispatcher.GestureDispatcherListener;
import com.android.camera.actor.CameraActor;
import com.android.camera.actor.PhotoActor;
import com.android.camera.actor.VideoActor;
import com.android.camera.bridge.CameraAppUiImpl;
import com.android.camera.bridge.CameraDeviceCtrl;
import com.android.camera.bridge.CameraDeviceManagerImpl;
import com.android.camera.bridge.FeatureConfigImpl;
import com.android.camera.bridge.FileSaverImpl;
import com.android.camera.bridge.SelfTimerManager;
import com.android.camera.externaldevice.ExternalDeviceManager;
import com.android.camera.externaldevice.IExternalDeviceCtrl;
import com.android.camera.manager.FrameManager;
import com.android.camera.manager.ModePicker;
import com.android.camera.manager.PickerManager;
import com.android.camera.manager.SettingManager;
import com.android.camera.manager.ViewManager;
import com.android.camera.permission.PermissionManager;
import com.android.camera.ui.FrameView;
import com.android.camera.ui.PreviewFrameLayout;
import com.android.camera.ui.PreviewSurfaceView;
import com.android.camera.ui.RotateLayout;

import com.mediatek.camera.ICameraMode.CameraModeType;
import com.mediatek.camera.ISettingCtrl;
import com.mediatek.camera.ModuleManager;
import com.mediatek.camera.platform.ICameraAppUi;
import com.mediatek.camera.platform.ICameraAppUi.CommonUiType;
import com.mediatek.camera.platform.ICameraAppUi.GestureListener;
import com.mediatek.camera.platform.ICameraAppUi.ViewState;
import com.mediatek.camera.platform.ICameraDeviceManager;
import com.mediatek.camera.platform.IFeatureConfig;
import com.mediatek.camera.platform.IFileSaver;
import com.mediatek.camera.platform.IModuleCtrl;
import com.mediatek.camera.platform.ISelfTimeManager;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.setting.SettingUtils;
import com.mediatek.camera.setting.preference.ListPreference;
import com.mediatek.camera.util.CameraPerformanceTracker;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

/*
 * --ActivityBase
 * ----Camera //will control CameraActor, ModePicker, CameraPicker, SettingChecker,
 *      FocusManager, RemainingManager
 * --CameraActor
 * ------VideoActor
 * ------PhotoActor
 * --------NormalActor //contains continuous shot
 * --------HdrActor
 * --------FaceBeautyActor
 * --------AsdActor
 * --------PanaromaActor
 */
public class CameraActivity extends ActivityBase implements
        PreviewFrameLayout.OnSizeChangedListener,
        ActivityCompat.OnRequestPermissionsResultCallback {
    private static final String TAG = "CameraActivity";
    private static final String SETTING_ON = "on";
    private static final String SETTING_OFF = "off";
    public static final int UNKNOWN = -1;
    public static final int STATE_PREVIEW_STOPPED = 0;
    public static final int STATE_IDLE = 1; // preview is active
    // ocus is in progress. The exact focus state is in Focus.java.
    public static final int STATE_FOCUSING = 2;
    public static final int STATE_SWITCHING_CAMERA = 4;

    private static final int MSG_CAMERA_PARAMETERS_READY = 2;
    private static final int MSG_CHECK_DISPLAY_ROTATION = 4;
    private static final int MSG_SWITCH_CAMERA = 5;
    private static final int MSG_CLEAR_SCREEN_DELAY = 7;
    private static final int MSG_APPLY_PARAMETERS_WHEN_IDEL = 12;
    private static final int MSG_DELAY_SHOW_ONSCREEN_INDICATOR = 16;
    private static final int MSG_UPDATE_SWITCH_ACTOR_STATE = 17;

    // add for stereo Camera features type
    private static final int DUAL_CAMERA_START = 1;
    private static final int DUAL_CAMERA_ENHANCE_ENABLE = 3;
    private static final int DUAL_CAMERA_ENHANCE_DISABLE = 4;
    private static final int DUAL_CAMERA_SWITCH_IN_REFOCUS = 2;
    private static final int ON = 0;
    private static final int OFF = 1;

    private static final int DELAY_MSG_SCREEN_SWITCH = 2 * 60 * 1000;
    private static final int SHOW_INFO_LENGTH_LONG = 5 * 1000;
    // This is the timeout to keep the camera in onPause for the first time
    // after screen on if the activity is started from secure lock screen.
    private static final int KEEP_CAMERA_TIMEOUT = 1000; // ms

    private int mDelayOtherMessageTime;
    private int mCameraState = STATE_PREVIEW_STOPPED;

    private int mNumberOfCameras;
    private int mPendingSwitchCameraId = UNKNOWN;

    private int mDisplayRotation;
    private int mOrientation = 0;
    private int mOrientationCompensation = 0;
    // TODO
    private long mOnResumeTime;

    private boolean mIsModeChanged;
    private int mStereoCaptureSuptSensorId;
    private int mDenoiseSuptSensorId;
    private boolean mNeedRestoreIfOpenFailed = false;
    protected boolean mIsStereoMode = false;

    private ISettingCtrl mISettingCtrl;
    private ActivityManager mActivityManager;
    private CameraActor mCameraActor;
    private PreviewFrameLayout mPreviewFrameLayout;
    private MyOrientationEventListener mOrientationListener;
    private ModePicker mModePicker;
    private FileSaver mFileSaver;
    private FrameManager mFrameManager;
    private RotateLayout mFocusAreaIndicator;
    private ComboPreferences mPreferences;
    private LocationManager mLocationManager;
    private PermissionManager mPermissionManager;
    // Gesture
    private GestureRecognizer mGestureRecognizer;
    private GestureDispatcher mGestureDispatcher;
    private PowerManager mPowerManager;
    private Vibrator mVibrator;
    private CharSequence mDelayShowInfo;
    private CameraAppUiImpl mCameraAppUi;
    private ISelfTimeManager mISelfTimeManager;
    private ModuleManager mModuleManager;
    private CameraDeviceCtrl mCameraDeviceCtrl;
    private ExternalDeviceManager mOtherDeviceConectedManager;

    private int mNextMode = ModePicker.MODE_PHOTO;
    private int mPrevMode = ModePicker.MODE_PHOTO;
    private boolean mIsBackPressed = false;
    private boolean mIsAPI2Inited = false;
    private boolean mIsCheckingLocationPermission = false;
    private ContentProviderClient mMediaProviderClient;

    // PreviewFrameLayout size has changed.implement
    // PreviewFrameLayout.OnSizeChangedListener @{
    @Override
    public void onSizeChanged(int width, int height) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        mCameraDeviceCtrl.onSizeChanged(width, height);
    }

    // @}

    @Override
    protected ICameraActivityBridge getCameraActivityBridge() {
        if (mCameraActivityBridge == null) {
            mCameraActivityBridge = CameraActivityBridgeFactory
                    .getCameraActivityBridge(this);
        }
        return mCameraActivityBridge;
    }

    @Override
    public void onCreate(Bundle icicle) {
        Log.d(TAG, "onCreate()");
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_ON_CREATE,
                CameraPerformanceTracker.ISBEGIN);
        super.onCreate(icicle);
        mPermissionManager = new PermissionManager(this);

        if (FeatureSwitcher.isApi2Enable(this)) {
            getCameraActivityBridge().onCreate(icicle);
            return;
        }
        mActivityManager = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        mPreferences = new ComboPreferences(this, isSecureCamera());
        Storage.setContext(this);
        SettingUtils.resetCameraId(mPreferences.getGlobal());
        mCameraDeviceCtrl = new CameraDeviceCtrl(this, mPreferences);

        if (mPermissionManager.requestCameraLaunchPermissions()) {
            mCameraDeviceCtrl.openCamera();
        }

        ModuleCtrlImpl moduleCtrl = new ModuleCtrlImpl(this);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_INIT_VIEW_MANAGER,
                CameraPerformanceTracker.ISBEGIN);
        mCameraAppUi = new CameraAppUiImpl(this);
        mCameraAppUi.createCommonView();
        initializeCommonManagers();
        mCameraAppUi.initializeCommonView();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_INIT_VIEW_MANAGER,
                CameraPerformanceTracker.ISEND);

        // just set content view for preview
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_VIEW_OPERATION,
                CameraPerformanceTracker.ISBEGIN);
        setContentView(R.layout.camera);
        ViewGroup appRoot = (ViewGroup) findViewById(R.id.camera_app_root);
        appRoot.bringToFront();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_VIEW_OPERATION,
                CameraPerformanceTracker.ISEND);
        mCameraDeviceCtrl.attachSurfaceViewLayout();

        mCameraDeviceCtrl.setCameraAppUi(mCameraAppUi);
        IFileSaver fileSaver = new FileSaverImpl(mFileSaver);
        IFeatureConfig featureConfig = new FeatureConfigImpl();
        ICameraDeviceManager deviceManager = new CameraDeviceManagerImpl(this,
                mCameraDeviceCtrl);
        mISelfTimeManager = new SelfTimerManager(this, mCameraAppUi);

        parseIntent();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_CREATE_MODULE,
                CameraPerformanceTracker.ISBEGIN);
        mModuleManager = new ModuleManager(this, fileSaver, mCameraAppUi,
                featureConfig, deviceManager, moduleCtrl, mISelfTimeManager);
        mISettingCtrl = mModuleManager.getSettingController();
        mCameraAppUi.setSettingCtrl(mISettingCtrl);
        if (isVideoCaptureIntent() || isVideoWallPaperIntent()) {
            mCameraActor = new VideoActor(this, mModuleManager,
                    ModePicker.MODE_VIDEO);
        } else {
            mCameraActor = new PhotoActor(this, mModuleManager,
                    ModePicker.MODE_PHOTO);
        }
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_CREATE_MODULE,
                CameraPerformanceTracker.ISEND);
        mCameraDeviceCtrl.setModuleManager(mModuleManager);
        mCameraDeviceCtrl.setSettingCtrl(mISettingCtrl);
        mCameraDeviceCtrl.setCameraActor(mCameraActor);
        mFileSaver.bindSaverService();
        mOtherDeviceConectedManager = new ExternalDeviceManager(this);
        mOtherDeviceConectedManager.onCreate();
        mOtherDeviceConectedManager.addListener(mListener);

        // reset, hdr, asd sharepreference as off to avoid error
        // when kill camera through IPO or other abnormal ways.
        if (isNonePickIntent()) {
            SettingUtils.updateSettingCaptureModePreferences(mPreferences
                    .getLocal());
        } else {
            View cover = findViewById(R.id.camera_cover);
            if (cover != null) {
                cover.setVisibility(View.VISIBLE);
            }

            View intentCover = findViewById(R.id.intentcover);
            if (intentCover != null) {
                intentCover.setVisibility(View.VISIBLE);
            }

        }
        // @}
        SettingUtils.upgradeGlobalPreferences(mPreferences.getGlobal(),
                CameraHolder.instance().getNumberOfCameras());
        initializeStereo3DMode();
        mVibrator = (Vibrator) getSystemService(Service.VIBRATOR_SERVICE);
        mDisplayRotation = Util.getDisplayRotation(this);

        // for photo view loading camera folder
        Storage.initializeStorageState();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_CREATE_SCREENNAIL,
                CameraPerformanceTracker.ISBEGIN);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_CREATE_SCREENNAIL,
                CameraPerformanceTracker.ISEND);
        // only initialize some thing for open
        initializeForOpeningProcess();
        initializeAfterPreview();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_ON_CREATE,
                CameraPerformanceTracker.ISEND);

    }

    @Override
    protected void onRestart() {
        super.onRestart();

        if (FeatureSwitcher.isApi2Enable(this)) {
            getCameraActivityBridge().onRestart();
            return;
        }

        if (isNonePickIntent() && isMountPointChanged()) {
            finish();
            mForceFinishing = true;
            startActivity(getIntent());
        } else if (isMountPointChanged()) {
            Storage.updateDefaultDirectory();
        }
        Log.d(TAG, "onRestart() mForceFinishing=" + mForceFinishing);
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume() mForceFinishing=" + mForceFinishing);
        keepMediaProviderInstance();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_ON_RESUME,
                CameraPerformanceTracker.ISBEGIN);
        super.onResume();
        if (FeatureSwitcher.isApi2Enable(this)) {
            if (mPermissionManager.requestCameraLaunchPermissions()) {
                mIsAPI2Inited = true;
                getCameraActivityBridge().onResume();
            }
            return;
        }
        parseIntent();
        mOtherDeviceConectedManager.onResume();
        if (mForceFinishing || mCameraDeviceCtrl.isOpenCameraFail()) {
            mNeedRestoreIfOpenFailed = false;
            return;
        }
        ViewGroup appRoot = (ViewGroup) findViewById(R.id.camera_app_root);
        int flag = View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
        appRoot.setSystemUiVisibility(flag);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_RESUME_NOTIFY,
                CameraPerformanceTracker.ISBEGIN);
        if (mModuleManager != null) {
            mModuleManager.resume();
        }

        // need check permission
        if (mPermissionManager.requestCameraLaunchPermissions()) {
            mCameraDeviceCtrl.onResume();
        }

        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_RESUME_NOTIFY,
                CameraPerformanceTracker.ISEND);
        // For onResume()->onPause()->onResume() quickly case,
        // onFullScreenChanged(true) may be called after onPause() and before
        // onResume().
        // So, force update app view.
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_UPDATE_APP_VIEW,
                CameraPerformanceTracker.ISBEGIN);
        doOnResume();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_UPDATE_APP_VIEW,
                CameraPerformanceTracker.ISEND);
        // for the case camera onPause and then edit pic in gallery, when resume
        // camera, thumbnail will not update to new.
        mCameraAppUi.forceThumbnailUpdate();
        mNeedRestoreIfOpenFailed = true;
        Util.enterCameraPQMode();
        if (!isCameraOpened()) {
            mCameraAppUi.setViewState(ViewState.VIEW_STATE_CAMERA_CLOSED);
            Log.d(TAG, "[onResume],camera device is opening,set view state.");
        }
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_ON_RESUME,
                CameraPerformanceTracker.ISEND);
    }

    /**
     * get permission Manager.
     * @return PermissionManager.
     */
    public PermissionManager getPermissionManager() {
        return mPermissionManager;
    }

    /**
     * the result if CameraActivity permission check.
     * there is four permissions that must be all on, the camera can be launch normally,
     * otherwise exit the camera app.
     * @param requestCode
     *            camera permission check code, used when requested permissions and
     *            the code will be back in the permissions requested result.
     * @param permissions
     *            the dangerous permissions that the activity defined in manifest.
     * @param grantResults
     *            the permission result for every permission.
     */
    public void onRequestPermissionsResult(int requestCode,
            String permissions[], int[] grantResults) {
        mIsCheckingLocationPermission = false;
        if (grantResults == null || grantResults.length <= 0) {
            return;
        }
        if (mPermissionManager.getCameraLaunchPermissionRequestCode()
                == requestCode) {
            if (mPermissionManager.isCameraLaunchPermissionsResultReady(
                    permissions, grantResults)) {
                // permission was granted
                // resume again, and open camera
                if (FeatureSwitcher.isApi2Enable(this)) {
                    mIsAPI2Inited = true;
                }
            } else {
                // more than one critical permission was denied
                // activity finish, exit and destroy
                Toast.makeText(this, R.string.denied_required_permission,
                        Toast.LENGTH_LONG).show();
                finish();
            }
        } else if (mPermissionManager.getCameraLocationPermissionRequestCode()
                == requestCode) {

            if (FeatureSwitcher.isApi2Enable(this)) {
                getCameraActivityBridge().onRequestLocationPermissionResult(
                        permissions, grantResults);
            } else {
                onRequestLocationPermissionResult(permissions, grantResults);
            }
            if (!mPermissionManager.isCameraLocationPermissionsResultReady(
                    permissions, grantResults)) {
                if (!ActivityCompat.shouldShowRequestPermissionRationale(this, permissions[0])) {
                    Toast.makeText(this, R.string.denied_required_permission,
                            Toast.LENGTH_LONG).show();
                }
            }
        } else {
            super.onRequestPermissionsResult(requestCode, permissions,
                    grantResults);
            return;
        }
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause() mForceFinishing=" + mForceFinishing);
        if (mMediaProviderClient != null) {
            Log.d(TAG, "onPause() release mMediaProviderClient");
            mMediaProviderClient.release();
            mMediaProviderClient = null;
        }
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_ON_PAUSE,
                CameraPerformanceTracker.ISBEGIN);
        super.onPause();

        if (FeatureSwitcher.isApi2Enable(this)) {
            if (mIsAPI2Inited) {
                getCameraActivityBridge().onPause();
                mIsAPI2Inited = false;
            }
            return;
        }

        if (mPendingSwitchCameraId != UNKNOWN) {
            mPendingSwitchCameraId = UNKNOWN;
        }

        if (mForceFinishing || mCameraDeviceCtrl.isOpenCameraFail()) {
            mCameraDeviceCtrl.onPause();
            // M: patch for Picutre quality enhance
            Log.d(TAG, "onPause(),release surface texture.");
            Util.exitCameraPQMode();
            // when camera is open fail,need notify otherDeviceConnectMananger
            mOtherDeviceConectedManager.onPause();
            return;
        }
        mCameraDeviceCtrl.onPause();
        // when camera start up thread is after onPause(),
        // so the case :mCameraDeviceCtrl.isOpenCameraFail() is false
        // but when handler msg will receive the msg:open file.
        // in this case we not need do the MSG
        mNeedRestoreIfOpenFailed = false;
        mCameraAppUi.collapseViewManager(true);
        mModuleManager.pause();
        keepCameraForSecure();
        clearFocusAndFace();
        uninstallIntentFilter();
        callResumablePause();
        mOrientationListener.disable();
        mLocationManager.recordLocation(false);
        // when close camera, should reset mOnResumeTime
        mOnResumeTime = 0L;
        // make sure this field is false when resume camera next time
        // mIsNeedUpdateOrientationToParameters = false;
        mMainHandler.removeCallbacksAndMessages(null);
        // actor will set screen on if needed, here reset it.
        resetScreenOn();
        // M: patch for Picutre quality enhance
        Util.exitCameraPQMode();
        mOtherDeviceConectedManager.onPause();
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_ON_PAUSE,
                CameraPerformanceTracker.ISEND);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy() isChangingConfigurations()="
                + isChangingConfigurations() + ", mForceFinishing="
                + mForceFinishing);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_ON_DESTROY,
                CameraPerformanceTracker.ISBEGIN);
        super.onDestroy();

        if (FeatureSwitcher.isApi2Enable(this)) {
            getCameraActivityBridge().onDestroy();
            CameraActivityBridgeFactory.destroyCameraActivityBridge(this);
            return;
        }

        mNextMode = UNKNOWN;
        // we finish worker thread when current activity destroyed.
        callResumableFinish();
        if (mCameraActor != null) {
            mCameraActor.release();
        }
        if (mFileSaver != null) {
            mFileSaver.unBindSaverService();
        }
        if (mISelfTimeManager != null) {
            ((SelfTimerManager) mISelfTimeManager).releaseSelfTimer();
            mISelfTimeManager = null;
        }
        mModuleManager.destory();
        mCameraDeviceCtrl.onDestory();
        if (mForceFinishing) {
            return;
        }
        if (mIsBackPressed) {
            clearUserSettings();
            mIsBackPressed = false;
        }
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_ON_DESTROY,
                CameraPerformanceTracker.ISEND);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (FeatureSwitcher.isApi2Enable(this)) {
            getCameraActivityBridge().onActivityResult(requestCode, resultCode,
                    data);
            return;
        }
        mCameraActor.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    public void onBackPressed() {
        Log.d(TAG, "onBackPressed()");
        if (FeatureSwitcher.isApi2Enable(this)) {
            if (getCameraActivityBridge().onBackPressed()) {
                super.onBackPressed();
                return;
            }
            return;
        }
        if (mPaused || mForceFinishing) {
            return;
        }
        if (mCameraDeviceCtrl.isOpenCameraFail()) {
            super.onBackPressed();
            return;
        }
        if ((!mCameraAppUi.collapseViewManager(false) && !mCameraActor
                .onBackPressed())) {
            super.onBackPressed();
        }
        mIsBackPressed = true;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        if (FeatureSwitcher.isApi2Enable(this)) {
            getCameraActivityBridge().onConfigurationChanged(newConfig);
            return;
        }
        // before the view collapse,get current camera view state
        boolean isSettingsViewState = mCameraAppUi.getViewState() == ViewState.VIEW_STATE_SETTING;
        Log.d(TAG, "mCameraState = " + mCameraAppUi.getViewState()
                + ",isSettingsView = " + isSettingsViewState);
        clearFocusAndFace();

        ViewGroup appRoot = (ViewGroup) findViewById(R.id.camera_app_root);
        appRoot.removeAllViews();
        getLayoutInflater().inflate(R.layout.preview_frame, appRoot, true);
        getLayoutInflater().inflate(R.layout.view_layers, appRoot, true);

        mCameraAppUi.removeAllView();

        // unlock orientation for new config
        setOrientation(false, OrientationEventListener.ORIENTATION_UNKNOWN);
        mCameraDeviceCtrl.setDisplayOrientation();
        initializeForOpeningProcess();
        // Here we should update aspect ratio for reflate preview frame layout.
        mCameraDeviceCtrl.setPreviewFrameLayoutAspectRatio();
        updateFocusAndFace();
        mCameraAppUi.onConfigurationChanged();
        notifyOrientationChanged();
        mModuleManager.configurationChanged();
    }

    public void onSingleTapUp(View view, int x, int y) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        // Gallery use onSingleTapConfirmed() instead of onSingleTapUp().
        Log.d(TAG, "onSingleTapUp(" + view + ", " + x + ", " + y + ")");
        // we do nothing for dialog is showing
        boolean isPogressShowing = mCameraAppUi.getCameraView(
                CommonUiType.ROTATE_PROGRESS).isShowing();
        boolean isDialogShowing = mCameraAppUi.getCameraView(
                CommonUiType.ROTATE_DIALOG).isShowing();
        if (!isDialogShowing && !isPogressShowing) {

            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mCameraAppUi.collapseSubSetting(true); // need to check it?

            }

            if (isCancelSingleTapUp()) {
                Log.i(TAG, "will cancel this singleTapUp event");
                return;
            }

            if (!mCameraAppUi.collapseSetting(true)) {
                if (mCameraActor.getonSingleTapUpListener() != null) {
                    mCameraActor.getonSingleTapUpListener().onSingleTapUp(view,
                            x, y);
                }
            }
        }
    }

    public void onLongPress(View view, int x, int y) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        Log.d(TAG, "OnLongPress(" + view + ", " + x + ", " + y + ")"
                + ",mCurrentViewState = " + mCameraAppUi.getViewState());
        if (mCameraAppUi.getViewState() == ViewState.VIEW_STATE_LOMOEFFECT_SETTING) {
            return;
        }
        // we do nothing for dialog is showing
        boolean isPogressShowing = mCameraAppUi.getCameraView(
                CommonUiType.ROTATE_PROGRESS).isShowing();
        boolean isDialogShowing = mCameraAppUi.getCameraView(
                CommonUiType.ROTATE_DIALOG).isShowing();
        if (!isDialogShowing && !isPogressShowing) {
            if (!mCameraAppUi.collapseSetting(true)) {
                if (mCameraActor.getonLongPressListener() != null) {
                    mCameraActor.getonLongPressListener().onLongPress(view, x, y);
                }
            }
        }
    }

    public void onSingleTapUpBorder(View view, int x, int y) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        // Just collapse setting if touch border
        boolean isPogressShowing = mCameraAppUi.getCameraView(
                CommonUiType.ROTATE_PROGRESS).isShowing();
        boolean isDialogShowing = mCameraAppUi.getCameraView(
                CommonUiType.ROTATE_DIALOG).isShowing();
        if (!isDialogShowing && !isPogressShowing) {
            mCameraAppUi.collapseSetting(true);
            // For tablet
            if (FeatureSwitcher.isSubSettingEnabled()) {
                mCameraAppUi.collapseSubSetting(true);
            }
        }
    }

    @Override
    public void onUserInteraction() {
        if (FeatureSwitcher.isApi2Enable(this)) {
            if (!getCameraActivityBridge().onUserInteraction()) {
                super.onUserInteraction();
            }
            return;
        }
        if (mCameraActor == null || !mCameraActor.onUserInteraction()) {
            super.onUserInteraction();
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            if (!getCameraActivityBridge().onKeyDown(keyCode, event)) {
                return super.onKeyDown(keyCode, event);
            }
            return true;
        }
        // Do not handle any key if the activity is paused.
        if (mPaused) {
            return true;
        }
        if (isFullScreen() && KeyEvent.KEYCODE_MENU == keyCode
                && event.getRepeatCount() == 0
                && mCameraAppUi.performSettingClick()) {
            return true;
        }
        if (!mCameraActor.onKeyDown(keyCode, event)) {
            return super.onKeyDown(keyCode, event);
        }
        return true;
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            if (!getCameraActivityBridge().onKeyUp(keyCode, event)) {
                return super.onKeyUp(keyCode, event);
            }
            return true;
        }
        if (mPaused) {
            return true;
        }
        if (!mCameraActor.onKeyUp(keyCode, event)) {
            return super.onKeyUp(keyCode, event);
        }
        return true;
    }

    public void gotoGallery() {
        Intent intent = new Intent(Util.REVIEW_ACTION);
        intent.setDataAndType(mCameraAppUi.getThumbnailUri(),
                mCameraAppUi.getThumbnailMimeType());
        intent.putExtra(Util.IS_CAMERA, true);
        if (isSecureCamera()) {
            intent.putExtra(Util.IS_SECURE_CAMERA, true);
            intent.putExtra(Util.SECURE_ALBUM, getSecureAlbum());
            intent.putExtra(Util.SECURE_PATH, getPath());
            notifyGotoGallery();
        }
        // add this for screen pinning
        if (mActivityManager.LOCK_TASK_MODE_PINNED == mActivityManager
                .getLockTaskModeState()) {
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_DOCUMENT
                    | Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
        }
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException ex) {
            Log.e(TAG, "[startGalleryActivity] Couldn't view ", ex);
        }
    }

    public ISettingCtrl getISettingCtrl() {
        return mISettingCtrl;
    }

    public void resetScreenOn() {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        Log.d(TAG, "resetScreenOn()");
        mMainHandler.removeMessages(MSG_CLEAR_SCREEN_DELAY);
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }

    public void keepScreenOnAwhile() {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        Log.d(TAG, "keepScreenOnAwhile()");
        mMainHandler.removeMessages(MSG_CLEAR_SCREEN_DELAY);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        mMainHandler.sendEmptyMessageDelayed(MSG_CLEAR_SCREEN_DELAY,
                DELAY_MSG_SCREEN_SWITCH);
    }

    public void keepScreenOn() {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        Log.d(TAG, "keepScreenOn()");
        mMainHandler.removeMessages(MSG_CLEAR_SCREEN_DELAY);
        if (getWindow() != null) {
            getWindow()
                    .addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
    }

    public void setGestureListener(GestureListener listener) {
        mGestureDispatcher.setGestureListener(listener);
    }

    public void setGestureDispatcherListener(GestureDispatcherListener listener) {
        mGestureDispatcher.setGestureDispatcherListener(listener);
    }

    public interface OnOrientationListener {
        void onOrientationChanged(int orientation);
    }

    public interface OnParametersReadyListener {
        void onCameraParameterReady();
    }

    public interface OnPreferenceReadyListener {
        void onPreferenceReady();
    }

    public interface Resumable {
        void begin();

        void resume();

        void pause();

        void finish();
    }

    public interface OnSingleTapUpListener {
        void onSingleTapUp(View view, int x, int y);
    }

    public interface OnLongPressListener {
        void onLongPress(View view, int x, int y);
    }

    public Vibrator getVibrator() {
        return mVibrator;
    }

    public SharedPreferences getSharePreferences() {
        int cameraId = mCameraDeviceCtrl.getCameraId();
        return mPreferences.getSharedPreference((Context) CameraActivity.this,
                cameraId);
    }

    public SharedPreferences getSharePreferences(int cameraId) {
        return mPreferences.getSharedPreference((Context) CameraActivity.this,
                cameraId);
    }

    public void setCameraState(int state) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        Log.d(TAG, "setCameraState(" + state + ")");
        mCameraState = state;
    }

    public int getCameraState() {
        return mCameraState;
    }

    public boolean isCameraOpened() {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return false;
        }
        return mCameraDeviceCtrl.isCameraOpened();
    }

    public boolean isModeChanged() {
        return mIsModeChanged;
    }

    public int getNextMode() {
        return mNextMode;
    }

    public int getPrevMode() {
        return mPrevMode;
    }

    public int getCurrentRecordingFps(Parameters ps) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return -1;
        }
        int value = -1;
        boolean isEISOn = ps.get(ParametersHelper.KEY_VIDEO_STABLILIZATION)
                .equals(ParametersHelper.VIDEO_STABLILIZATION_ON);
        if (isEISOn) {
            String typeValueString = ps
                    .get(ParametersHelper.KEY_VIDEO_RECORIND_FEATURE_MAX_FPS);
            Log.d(TAG, "getCurrentRecordingFps, typeValueString = "
                    + typeValueString);
            if (typeValueString != null) {
                int index = typeValueString.indexOf("@");
                if (index >= 0) {
                    value = Integer.parseInt(typeValueString
                            .substring(0, index));
                }
            }
        }
        Log.d(TAG, "getCurrentRecordingFps, value = " + value);
        return value;
    }

    public ISelfTimeManager getSelfTimeManager() {
        return mISelfTimeManager;
    }

    public ICameraAppUi getCameraAppUI() {
        return mCameraAppUi;
    }

    public FrameView getFrameView() {
        return mFrameManager.getFrameView();
    }

    public FrameManager getFrameManager() {
        return mFrameManager;
    }

    public ComboPreferences getPreferences() { // not recommended
        return mPreferences;
    }

    public ListPreference getListPreference(int row) {
        String key = SettingConstants.getSettingKey(row);
        return getListPreference(key);
    }

    public ListPreference getListPreference(String key) {
        return mISettingCtrl.getListPreference(key);
    }

    public FileSaver getFileSaver() {
        return mFileSaver;
    }

    public LocationManager getLocationManager() {
        return mLocationManager;
    }

    public int getCurrentMode() {
        return mCameraActor.getMode();
    }

    public ModePicker getModePicker() {
        return mModePicker;
    }

    // activity
    public int getDisplayRotation() {
        return mDisplayRotation;
    }

    // sensor
    public int getOrietation() {
        return mOrientation;
    }

    // activity + sensor
    public int getOrientationCompensation() {
        return mOrientationCompensation;
    }

    public int getCameraCount() {
        return mNumberOfCameras;
    }

    public CameraActor getCameraActor() {
        return mCameraActor;
    }

    public ModuleManager getModuleManager() {
        return mModuleManager;
    }

    public int getPreviewFrameWidth() {
        return mCameraDeviceCtrl.getPreviewFrameWidth();
    }

    public int getPreviewFrameHeight() {
        return mCameraDeviceCtrl.getPreviewFrameHeight();
    }

    public View getPreviewFrameLayout() {
        return mPreviewFrameLayout;
    }

    public int getUnCropWidth() {
        return mCameraDeviceCtrl.getUnCropWidth();
    }

    public int getUnCropHeight() {
        return mCameraDeviceCtrl.getUnCropHeight();
    }

    public GestureRecognizer getGestureRecognizer() {
        return mGestureRecognizer;
    }

    public void showBorder(boolean show) {
        if (FeatureSwitcher.isApi2Enable(this)) {
            return;
        }
        mPreviewFrameLayout.showBorder(show);
    }

    public View inflate(int layoutId, int layer) {
        return mCameraAppUi.inflate(layoutId, layer);
    }

    public void addView(View view, int layer) {
        mCameraAppUi.addView(view, layer);
    }

    public void removeView(View view, int layer) {
        mCameraAppUi.removeView(view, layer);
    }

    public boolean addOnPreferenceReadyListener(OnPreferenceReadyListener l) {
        if (!mPreferenceListeners.contains(l)) {
            return mPreferenceListeners.add(l);
        }
        return false;
    }

    public boolean addOnParametersReadyListener(OnParametersReadyListener l) {
        if (!mParametersListeners.contains(l)) {
            return mParametersListeners.add(l);
        }
        return false;
    }

    public boolean removeOnParametersReadyListener(OnParametersReadyListener l) {
        return mParametersListeners.remove(l);
    }

    public boolean addViewManager(ViewManager viewManager) {
        return mCameraAppUi.addViewManager(viewManager);
    }

    public boolean removeViewManager(ViewManager viewManager) {
        return mCameraAppUi.removeViewManager(viewManager);
    }

    public boolean addResumable(Resumable resumable) {
        if (!mResumables.contains(resumable)) {
            return mResumables.add(resumable);
        }
        return false;
    }

    public boolean removeResumable(Resumable resumable) {
        return mResumables.remove(resumable);
    }

    // Becasue SMB will need this
    public void changeOrientationTag(boolean lock, int orientationNum) {
    }

    private Handler mMainHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // Log.i(TAG, "handleMessage(" + msg + ")");
            switch (msg.what) {
            case MSG_CAMERA_PARAMETERS_READY:
                notifyParametersReady();
                break;
            case MSG_CHECK_DISPLAY_ROTATION:
                // Set the display orientation if display rotation has changed.
                // Sometimes this happens when the device is held upside
                // down and camera app is opened. Rotation animation will
                // take some time and the rotation value we have got may be
                // wrong. Framework does not have a callback for this now.
                if (Util.getDisplayRotation(CameraActivity.this) != mDisplayRotation) {
                    mCameraDeviceCtrl.setDisplayOrientation();
                    mOrientation = OrientationEventListener.ORIENTATION_UNKNOWN;
                    mCameraActor.onDisplayRotate();
                }
                if (SystemClock.uptimeMillis() - mOnResumeTime < 5000) {
                    mMainHandler.sendEmptyMessageDelayed(
                            MSG_CHECK_DISPLAY_ROTATION, 100);
                }
                notifyOrientationChanged();
                break;
            case MSG_SWITCH_CAMERA:
                mCameraDeviceCtrl.switchCamera(msg.arg1);
                break;
            case MSG_CLEAR_SCREEN_DELAY:
                getWindow().clearFlags(
                        WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                break;
            case MSG_APPLY_PARAMETERS_WHEN_IDEL:
                mCameraDeviceCtrl.applyParameters(false);
                break;
            case MSG_DELAY_SHOW_ONSCREEN_INDICATOR:
                // will handle the delay hide the remain when is show remain
                mCameraAppUi.showText(mDelayShowInfo);
                mCameraAppUi.showIndicator(mDelayOtherMessageTime);
                break;
            case MSG_UPDATE_SWITCH_ACTOR_STATE:
                mModePicker.setEnabled(true);
                break;
            default:
                break;
            }
        };
    };

    public void onCameraOpenFailed() {
        restoreWhenCameraOpenFailed();
    }

    public void onCameraOpenDone() {
        mPendingSwitchCameraId = UNKNOWN;
        mMainHandler.sendEmptyMessage(MSG_CHECK_DISPLAY_ROTATION);
    }

    public void onCameraPreferenceReady() {
        notifyPreferenceReady();
    }

    public void onCameraParametersReady() {
        notifyParametersReady();
    }

    private void restoreWhenCameraOpenFailed() {
        Log.d(TAG, "restoreWhenCameraOpenFailed(), mNeedRestoreIfOpenFailed:"
                + mNeedRestoreIfOpenFailed);
        // some setting should be restore when camera open failed, because they
        // may
        // be set in onResume() method.
        if (mNeedRestoreIfOpenFailed) {
            uninstallIntentFilter();
            callResumablePause();
            mCameraAppUi.collapseViewManager(true);
            mOrientationListener.disable();
            mLocationManager.recordLocation(false);
            mMainHandler.removeCallbacksAndMessages(null);
            // actor will set screen on if needed, here reset it.
            resetScreenOn();
            // setLoadingAnimationVisible(true);
            // M: patch for Picutre quality enhance
            Util.exitCameraPQMode();
            mNeedRestoreIfOpenFailed = false;
        }
    }

    private void keepCameraForSecure() {
        // When camera is started from secure lock screen for the first time
        // after screen on, the activity gets
        // onCreate->onResume->onPause->onResume.
        // To reduce the latency, keep the camera for a short time so it does
        // not need to be opened again.
        if (isSecureCamera() && isFirstStartAfterScreenOn()) {
            resetFirstStartAfterScreenOn();
            int cameraId = mCameraDeviceCtrl.getCameraId();
            CameraHolder.instance().keep(KEEP_CAMERA_TIMEOUT, cameraId);
        }
    }

    private void initializeAfterPreview() {
        long start = System.currentTimeMillis();
        callResumableBegin();
        mCameraAppUi.initializeAfterPreview();

        addIdleHandler(); // why no flag to disable it after checked.
        long stop = System.currentTimeMillis();

        Log.v(TAG, "initializeAfterPreview() consume:" + (stop - start));
    }

    // Here should be lightweight functions!!!
    private void initializeCommonManagers() {
        mModePicker = new ModePicker(this);
        mFileSaver = new FileSaver(this);
        mFrameManager = new FrameManager(this);
        mModePicker.setListener(mModeChangedListener);
        mCameraAppUi.setSettingListener(mSettingListener);
        mCameraAppUi.setPickerListener(mPickerListener);
        mCameraAppUi.addFileSaver(mFileSaver);
        mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        Log.v(TAG, "getSystemService,mPowerManager =" + mPowerManager);
        // For tablet
        if (FeatureSwitcher.isSubSettingEnabled()) {
            mCameraAppUi.setSubSettingListener(mSettingListener);
        }
    }

    private void initializeForOpeningProcess() {
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_INIT_OPEN_PROCESS,
                CameraPerformanceTracker.ISBEGIN);
        mNumberOfCameras = CameraHolder.instance().getNumberOfCameras();
        mCameraAppUi.initializeViewGroup();
        // for focus manager used
        mFocusAreaIndicator = (RotateLayout) findViewById(R.id.focus_indicator_rotate_layout);
        if (mGestureDispatcher == null) {
            mGestureDispatcher = new GestureDispatcher(this);
            mGestureRecognizer = new GestureRecognizer(this, mGestureDispatcher);
        }
        // startPreview needs this.
        mPreviewFrameLayout = (PreviewFrameLayout) findViewById(R.id.frame);
        // Set touch focus listener.
        mGestureDispatcher.setSingleTapUpListener(mPreviewFrameLayout);
        // Set Long Press for Object Tracking listener.
        mGestureDispatcher.setLongPressListener(mPreviewFrameLayout);
        mPreviewFrameLayout.setOnSizeChangedListener(this);
        // for other info.
        if (mLocationManager == null) {
            mLocationManager = new LocationManager(this, null);
        }
        if (mOrientationListener == null) {
            mOrientationListener = new MyOrientationEventListener(this);
        }
        Log.d(TAG, "initializeForOpeningProcess() mNumberOfCameras="
                + mNumberOfCameras);
        CameraPerformanceTracker.onEvent(TAG,
                CameraPerformanceTracker.NAME_CAMERA_INIT_OPEN_PROCESS,
                CameraPerformanceTracker.ISEND);
    }

    private void updateFocusAndFace() {
        if (mFrameManager != null && getFrameView() != null) {
            getFrameView().clear();
            getFrameView().setVisibility(View.VISIBLE);
            // getFrameView().setDisplayOrientation(mDisplayOrientation);
            int cameraId = mCameraDeviceCtrl.getCameraId();
            CameraInfo info = CameraHolder.instance().getCameraInfo()[cameraId];
            getFrameView().setMirror(
                    info.facing == CameraInfo.CAMERA_FACING_FRONT);
            getFrameView().resume();
        }
        FocusManager focusManager = mCameraDeviceCtrl.getFocusManager();
        if (focusManager != null) {
            focusManager.setFocusAreaIndicator(mFocusAreaIndicator);
            View mFocusIndicator = mFocusAreaIndicator
                    .findViewById(R.id.focus_indicator);
            // Set the length of focus indicator according to preview frame
            // size.
            int len = Math.min(getPreviewFrameWidth(), getPreviewFrameHeight()) / 4;
            ViewGroup.LayoutParams layout = mFocusIndicator.getLayoutParams();
            layout.width = len;
            layout.height = len;
        }
        if (mFrameManager != null && getFrameView() != null) {
            // these case(configuration change), OT is must be false.
            // default view is face view.
            mFrameManager.initializeFrameView(false);
        }
    }

    private void doOnResume() {
        long start = System.currentTimeMillis();
        mOrientationListener.enable();
        /**
         * when launch video camera with MMS open the flash, between the switch
         * camera a call is coming and then reject it,will found the camera view
         * is unclickable because the camera view state is :switch camera so
         * need to restore the view state at here
         * But we should not restore view state when ReviewManager is showing,
         * otherwise, setting/flash/switch camera icons will be shown on UI.
         */

        installIntentFilter();
        callResumableResume();
        mCameraAppUi.checkViewManagerConfiguration();
        // change the file saver listener for foreground Activity.
        long stop = System.currentTimeMillis();
        Log.d(TAG, "doOnResume() consume:" + (stop - start));
    }

    private void clearFocusAndFace() {
        if (getFrameView() != null) {
            getFrameView().clear();
        }
        FocusManager focusManager = mCameraDeviceCtrl.getFocusManager();
        if (focusManager != null) {
            focusManager.removeMessages();
        }
    }

    private boolean isCancelSingleTapUp() {
        if (mCameraAppUi.getViewState() == ViewState.VIEW_STATE_LOMOEFFECT_SETTING) {
            return true;
        }
        return false;
    }
    private List<OnPreferenceReadyListener> mPreferenceListeners =
            new CopyOnWriteArrayList<OnPreferenceReadyListener>();

    private void notifyPreferenceReady() {
        for (OnPreferenceReadyListener listener : mPreferenceListeners) {
            if (listener != null) {
                listener.onPreferenceReady();
            }
        }
    }

    private List<OnParametersReadyListener> mParametersListeners =
            new CopyOnWriteArrayList<OnParametersReadyListener>();

    private void notifyParametersReady() {
        ViewState curState = mCameraAppUi.getViewState();
        if ((isNonePickIntent()
                && mCameraActor.getMode() != ModePicker.MODE_VIDEO
                && mCameraActor.getMode() != ModePicker.MODE_VIDEO_PIP
                && curState != ViewState.VIEW_STATE_SETTING
                && curState != ViewState.VIEW_STATE_SUB_SETTING
                && curState != ViewState.VIEW_STATE_RECORDING)
                || isStereo3DImageCaptureIntent()) {
            mModePicker.show();
        }
        if (!isSecureCamera()) {
            updateCameraLocationInfo();
        }
        for (OnParametersReadyListener listener : mParametersListeners) {
            if (listener != null) {
                listener.onCameraParameterReady();
            }
        }
        mCameraAppUi.notifyParametersReady();
    }

    private void onRequestLocationPermissionResult(String permissions[], int[] grantResults) {
        if (mPermissionManager.isCameraLocationPermissionsResultReady(
                permissions, grantResults)) {
            mLocationManager.recordLocation(true);
        } else {
            //set location off to setting
            ListPreference pref = mISettingCtrl.getListPreference(
                    SettingConstants.KEY_RECORD_LOCATION);
            if (pref != null) {
                pref.setValue(SETTING_OFF);
            }
        }
    }

    private void updateCameraLocationInfo() {
        boolean isLocationOpened = SETTING_ON.equals(mISettingCtrl.getSettingValue(
                SettingConstants.KEY_RECORD_LOCATION));
        boolean isLocationPermissionReady = false;
        if (!isLocationOpened) {
            mLocationManager.recordLocation(false);
            return;
        }
        if (!mIsCheckingLocationPermission) {
            /**
             * it may send permission request when camera launch, so must check whether
             * location is on in camera setting
             */
            isLocationPermissionReady = mPermissionManager.requestCameraLocationPermissions();
            if (isLocationPermissionReady) {
                mLocationManager.recordLocation(true);
            } else {
                /**
                 * it send permission request, and show dialog for user check
                 * the result will be in onRequestPermissionsResult.
                 */
                mIsCheckingLocationPermission = true;
            }
        }
    }

    private List<Resumable> mResumables = new CopyOnWriteArrayList<Resumable>();

    private void callResumableBegin() {
        for (Resumable resumable : mResumables) {
            resumable.begin();
        }
    }

    private void callResumableResume() {
        for (Resumable resumable : mResumables) {
            resumable.resume();
        }
    }

    private void callResumablePause() {
        for (Resumable resumable : mResumables) {
            resumable.pause();
        }
    }

    private void callResumableFinish() {
        for (Resumable resumable : mResumables) {
            resumable.finish();
        }
    }

    // / @}

    // / M: mode change logic @{
    private boolean mIsFromRestore = false;
    private int mOriCameraId = UNKNOWN;
    private ModePicker.OnModeChangedListener mModeChangedListener =
            new ModePicker.OnModeChangedListener() {
        @Override
        public void onModeChanged(int newMode) {
            Log.d(TAG, "onModeChanged(" + newMode + ") current mode = "
                    + mCameraActor.getMode() + ", state=" + mCameraState);
            mPrevMode = mCameraActor.getMode();
            mNextMode = newMode;
            int oldMode = mPrevMode;

            if (mCameraActor.getMode() != newMode) {
                // when mode changed,remaining manager should check whether to
                // show
                mIsModeChanged = true;
                // releaseCameraActor has change mLastMode
                String oldCameraMode = mISettingCtrl
                        .getCameraMode(getModeSettingKey(oldMode));
                String newCameraMode = mISettingCtrl
                        .getCameraMode(getModeSettingKey(newMode));
                if (oldCameraMode == null || newCameraMode == null) {
                    Log.i(TAG, "onModeChanged old or new Camera mode is null!!!");
                    return;
                }
                boolean needRestart = (!oldCameraMode.equals(newCameraMode))
                        || com.mediatek.camera.platform.Parameters.CAMERA_MODE_MTK_VDO == Integer
                                .parseInt(newCameraMode);
                Log.d(TAG, "needRestart = " + needRestart);
                // if need restart preview, should do stop preview in last mode
                if (needRestart) {
                    mCameraActor.stopPreview();
                }
                releaseCameraActor(oldMode, newMode);
                mModuleManager.setModeSettingValue(
                        mCameraActor.getCameraModeType(oldMode), SETTING_OFF);
                // need to select sensor when switching stereo camera.
                judgeSensorSwitchStereoMode();
                if (isPIPModeSwitch(oldMode, newMode)) {
                    // when user does pip mode switch more quickly,
                    // here try to close camera, camera start up thread may not
                    // done,
                    // should wait to not disturb camera status
                    if (!isPIPMode(oldMode)) {
                        mOriCameraId = getCameraId();
                    }
                    mCameraDeviceCtrl.closeCamera(false);
                }
                releaseCameraSwitchStereoMode(oldMode, newMode);
                switch (newMode) {
                case ModePicker.MODE_PHOTO:
                    mCameraActor = new PhotoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;
                case ModePicker.MODE_FACE_BEAUTY:
                    mCameraActor = new PhotoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;
                case ModePicker.MODE_PANORAMA:
                    mCameraActor = new PhotoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;
                case ModePicker.MODE_PHOTO_PIP:
                    mCameraActor = new PhotoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;
                case ModePicker.MODE_VIDEO:
                    mCameraActor = new VideoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;
                case ModePicker.MODE_VIDEO_PIP:
                    mCameraActor = new VideoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;
                case ModePicker.MODE_STEREO_CAMERA:
                    mCameraActor = new PhotoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    mCameraAppUi.resetSettings();
                    break;
                case ModePicker.MODE_PHOTO_STEREO:
                    mCameraActor = new PhotoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;
                case ModePicker.MODE_VIDEO_STEREO:
                    mCameraActor = new VideoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;

                default:
                    mCameraActor = new PhotoActor(CameraActivity.this,
                            mModuleManager, newMode);
                    break;
                }
                // after apply settings, should change preview surface
                // immediately
                mCameraDeviceCtrl.setCameraActor(mCameraActor);
                // startup thread will apply these things after onResume
                if (mPaused || mCameraState == STATE_SWITCHING_CAMERA) {
                    mIsModeChanged = false;
                    Log.d(TAG, "onModeChanged return mPaused = " + mPaused);
                    return;
                }

               if (isStereoModeChanged(oldMode, newMode)) {
                   doStereoModeChanged(false);
                   mIsModeChanged = false;
                   Log.i(TAG, "onModeChanged isStereoModeChanged return");
                   return;
               }

                if (isPIPModeSwitch(oldMode, mCameraActor.getMode())) {
                    // the first time to update PickerManager when pip changed
                    // to other mode
                    doPIPModeChanged(mOriCameraId);
                    mIsModeChanged = false;
                    Log.i(TAG, "onModeChanged isPIPModeSwitch return");
                    return;
                }
                // reset default focus modes.
                notifyOrientationChanged();
                mCameraDeviceCtrl.onModeChanged(needRestart);
                mIsModeChanged = false;
            }
        }
    };

    private String getModeSettingKey(int mode) {
        String key = null;
        switch (mode) {
        case ModePicker.MODE_PHOTO:
            key = SettingConstants.KEY_NORMAL;
            break;
        case ModePicker.MODE_PANORAMA:
            key = SettingConstants.KEY_PANORAMA;
            break;
        case ModePicker.MODE_VIDEO:
            key = SettingConstants.KEY_VIDEO;
            break;
        case ModePicker.MODE_VIDEO_PIP:
            key = SettingConstants.KEY_VIDEO_PIP;
            break;
        case ModePicker.MODE_PHOTO_PIP:
            key = SettingConstants.KEY_PHOTO_PIP;
            break;
        case ModePicker.MODE_FACE_BEAUTY:
            key = SettingConstants.KEY_FACE_BEAUTY;
            break;
        case ModePicker.MODE_STEREO_CAMERA:
            key = SettingConstants.KEY_REFOCUS;
            break;
        case ModePicker.MODE_PHOTO_STEREO:
            key = SettingConstants.KEY_PHOTO_STEREO;
            break;
        case ModePicker.MODE_VIDEO_STEREO:
            key = SettingConstants.KEY_VIDEO_STEREO;
            break;
        default:
            break;
        }
        return key;
    }

    private void releaseCameraActor(int oldMode, int newMode) {
        Log.d(TAG, "releaseCameraActor() mode=" + mCameraActor.getMode());
        boolean shouldHideSetting = false;
        shouldHideSetting = (newMode != ModePicker.MODE_ASD
                && newMode != ModePicker.MODE_HDR
                && oldMode != ModePicker.MODE_ASD && oldMode != ModePicker.MODE_HDR);
        if (shouldHideSetting) {
            mCameraAppUi.collapseViewManager(true);
        }

        mCameraActor.release();
        if (newMode == ModePicker.MODE_VIDEO
                || newMode == ModePicker.MODE_VIDEO_3D
                || newMode == ModePicker.MODE_VIDEO_PIP) {
            Log.d(TAG,
                    "releaseCameraActor setSwipingEnabled(false)  newMode = "
                            + newMode);
        }
    }

    public void onSettingChanged(String key, String value) {
        mISettingCtrl.onSettingChanged(key, value);
    }

    // Camera Decoupling --->begin
    /**
     * because current on Listener and cancel Listener is add on the shutter
     * button and video button so when we want show on and cancel button, first
     * need set the video and photo listener to null
     * @param okOnClickListener callback
     * @param cancelOnClickListener callback
     * @param retatekOnClickListener callback
     * @param playOnClickListener callback
     */
    public void applyReviewCallbacks(OnClickListener okOnClickListener,
            OnClickListener cancelOnClickListener,
            OnClickListener retatekOnClickListener,
            OnClickListener playOnClickListener) {
        mPlayListener = playOnClickListener;
        mRetakeListener = retatekOnClickListener;
    }

    private OnClickListener mPlayListener;
    private OnClickListener mRetakeListener;

    public OnClickListener getPlayListener() {
        return mPlayListener;
    }

    public OnClickListener getRetakeLisenter() {
        return mRetakeListener;
    }

    public void enableOrientationListener() {
        mOrientationListener.enable();
    }

    public void disableOrientationListener() {
        mOrientationListener.disable();
    }

    private SettingManager.SettingListener mSettingListener = new SettingManager.SettingListener() {
        @Override
        public void onSharedPreferenceChanged(ListPreference preference) {
            if (!isCameraOpened()) {
                return;
            }
            if (preference != null) {
                String settingKey = preference.getKey();
                String value = preference.getValue();
                // When change AF mode change bettween multi-zone and single,need clear focus UI and
                // clear multi-zone AF data
                if (SettingConstants.KEY_AF.equals(settingKey)) {
                    Log.d(TAG, "[onSharedPreferenceChanged] change to " + value + " AF");
                    getFocusManager().clearFocusUi();
                }
                mISettingCtrl.onSettingChanged(settingKey, value);
            }
            mCameraDeviceCtrl.applyParameters(false);
        }

        @Override
        public void onRestorePreferencesClicked() {
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "[onRestorePreferencesClicked.run]");
                    mIsFromRestore = true;
                    mCameraActor.onRestoreSettings();
                    mCameraAppUi.collapseViewManager(true);
                    mCameraAppUi.resetSettings();

                    SharedPreferences globalPref = mPreferences.getGlobal();
                    SettingUtils.restorePreferences(globalPref, isNonePickIntent());

                    SettingUtils.upgradeGlobalPreferences(globalPref,
                            CameraHolder.instance().getNumberOfCameras());
                    SettingUtils.writePreferredCameraId(globalPref,
                            mCameraDeviceCtrl.getCameraId());

                    int backCameraId = CameraHolder.instance()
                            .getBackCameraId();
                    SettingUtils.restorePreferences(getSharePreferences(backCameraId),
                            isNonePickIntent());
                    SettingUtils.upgradeLocalPreferences(getSharePreferences(backCameraId));

                    // restore front camera setting
                    int frontCameraId = CameraHolder.instance()
                            .getFrontCameraId();
                    SettingUtils.restorePreferences(getSharePreferences(frontCameraId),
                            isNonePickIntent());
                    SettingUtils.upgradeLocalPreferences(getSharePreferences(frontCameraId));

                    SettingUtils.initialCameraPictureSize(
                            (Context) CameraActivity.this,
                            mCameraDeviceCtrl.getParametersExt(),
                            getSharePreferences());

                    mISettingCtrl.restoreSetting(backCameraId);
                    mISettingCtrl.restoreSetting(frontCameraId);

                    mCameraAppUi.resetZoom();
                    // we should apply parameters if mode is default too.
                    int mode = mCameraActor.getMode();
                    if (ModePicker.MODE_PHOTO == mode || !isNonePickIntent()
                            || ModePicker.MODE_PHOTO_SGINLE_3D == mode
                            || ModePicker.MODE_PHOTO_3D == mode) {
                        if (ModePicker.MODE_VIDEO == mode
                                && !isNonePickIntent()) {
                            mISettingCtrl.onSettingChanged(
                                    SettingConstants.KEY_VIDEO, SETTING_ON);
                        }
                        mCameraDeviceCtrl.applyParameters(false);
                    } else {
                        mModePicker.setModePreference(null);
                        mModePicker.setCurrentMode(ModePicker.MODE_PHOTO);
                    }
                    mIsFromRestore = false;
                }
            };
            mCameraAppUi.showAlertDialog(null,
                    getString(R.string.confirm_restore_message),
                    getString(android.R.string.cancel), null,
                    getString(android.R.string.ok), runnable);
        }

        @Override
        public void onSettingContainerShowing(boolean show) {
            mModuleManager.onSettingContainerShowing(show);
            if (show) {
            } else {
                if (isFaceBeautyEnable()
                        && getCurrentMode() == ModePicker.MODE_FACE_BEAUTY
                        && !mIsFromRestore) {
                    // when face is detected will show the icon
                    // otherwise don't show
                    Log.d(TAG,
                            "onSettingContainerShowing, will set modify icon stautes true,"
                                    + "and show FB icon");
                    // mLomoEffectsManager.show();
                }
            }
        }

        @Override
        public void onStereoCameraPreferenceChanged(ListPreference preference,
                int type) {
            if (preference != null
                    && preference.getKey().equals(
                            SettingConstants.KEY_DUAL_CAMERA_MODE)) {
                Log.d(TAG, "onStereoCameraPreferenceChanged, type = " + type);
                if (getCurrentMode() == ModePicker.MODE_STEREO_CAMERA) {
                    if (type == DUAL_CAMERA_ENHANCE_ENABLE) {
                        enableDualCameraExtras();
                    }
                    if (type == DUAL_CAMERA_ENHANCE_DISABLE) {
                        disableDualCameraExtras();
                    }
                    if (type == DUAL_CAMERA_START) {
                        singleDualCameraExtras();
                        mCameraDeviceCtrl.applyParameters(false);
                        return;
                    }
                    mCameraDeviceCtrl.applyParameters(false);
                    return;
                } else {
                    if (type == DUAL_CAMERA_ENHANCE_ENABLE) {
                        enableDualCameraExtras();
                    }
                    if (type == DUAL_CAMERA_ENHANCE_DISABLE) {
                        disableDualCameraExtras();
                    }
                    if (type == DUAL_CAMERA_START) {
                        singleDualCameraExtras();
                        doStereoModeChanged(false);
                        return;
                    }
                    if (type == DUAL_CAMERA_SWITCH_IN_REFOCUS) {
                        singleDualCameraExtras();
                        mCameraDeviceCtrl.applyParameters(false);
                        return;
                    }
                    doStereoModeChanged(false);
                    return;
                }
            }
        }
    };

    private void enableDualCameraExtras() {
        getListPreference(SettingConstants.ROW_SETTING_DISTANCE).setValueIndex(ON);
        getListPreference(SettingConstants.ROW_SETTING_FAST_AF).setValueIndex(ON);
        onSettingChanged(SettingConstants.KEY_FAST_AF, SETTING_ON);
        onSettingChanged(SettingConstants.KEY_DISTANCE, SETTING_ON);
    }

    private void disableDualCameraExtras() {
        getListPreference(SettingConstants.ROW_SETTING_DISTANCE).setValueIndex(OFF);
        getListPreference(SettingConstants.ROW_SETTING_FAST_AF).setValueIndex(OFF);
        onSettingChanged(SettingConstants.KEY_FAST_AF, SETTING_OFF);
        onSettingChanged(SettingConstants.KEY_DISTANCE, SETTING_OFF);
    }

    private void singleDualCameraExtras() {
        if (SETTING_OFF.equals(getListPreference(SettingConstants.KEY_FAST_AF).getValue())) {
            onSettingChanged(SettingConstants.KEY_FAST_AF, SETTING_OFF);
        } else {
            onSettingChanged(SettingConstants.KEY_FAST_AF, SETTING_ON);
        }
        if (SETTING_OFF.equals(getListPreference(SettingConstants.KEY_DISTANCE).getValue())) {
            onSettingChanged(SettingConstants.KEY_DISTANCE, SETTING_OFF);
        } else {
            onSettingChanged(SettingConstants.KEY_DISTANCE, SETTING_ON);
        }
    }

    public void notifyPreferenceChanged(ListPreference preference) {
        mSettingListener.onSharedPreferenceChanged(preference);
        mCameraAppUi.getCameraView(CommonUiType.SETTING).refresh();
    }

    // / M: for photo info @{
    public String getSelfTimer() {
        String seflTimer = mISettingCtrl
                .getSettingValue(SettingConstants.KEY_SELF_TIMER);
        return seflTimer;
    }

    // / M: orientation case @{
    // / M: for listener and resumable convenient functions @{
    private List<OnOrientationListener> mOrientationListeners =
            new CopyOnWriteArrayList<OnOrientationListener>();

    public boolean addOnOrientationListener(OnOrientationListener l) {
        if (!mOrientationListeners.contains(l)) {
            return mOrientationListeners.add(l);
        }
        return false;
    }

    public boolean removeOnOrientationListener(OnOrientationListener l) {
        return mOrientationListeners.remove(l);
    }

    public void setOrientation(boolean lock, int orientation) {
        // Log.i(TAG, "setOrientation orientation=" + orientation +
        // " mOrientationListener.getLock()="
        // + mOrientationListener.getLock() + " lock = " + lock);
        mOrientationListener.setLock(false);
        if (lock) {
            mOrientationListener.onOrientationChanged(orientation);
            mOrientationListener.setLock(true);

        } else {
            mOrientationListener.restoreOrientation();
        }
    }

    private PickerManager.PickerListener mPickerListener = new PickerManager.PickerListener() {
        @Override
        public boolean onCameraPicked(int cameraId) {
            Log.d(TAG, "onCameraPicked(" + cameraId + ") mPaused=" + mPaused
                    + " mPendingSwitchCameraId=" + mPendingSwitchCameraId);
            if (mPaused
                    || mPendingSwitchCameraId != UNKNOWN
                    || !ModeChecker.getModePickerVisible(CameraActivity.this,
                            cameraId, getCurrentMode())
                    || !mCameraDeviceCtrl.isCameraOpened()) {
                return false;
            }

            // Here we always return false for switchCamera will change
            // preference after real switching.
            int frontCameraId = CameraHolder.instance().getFrontCameraId();
            if (!mModuleManager.switchDevice() && isDualCameraDeviceEnable()
                    && frontCameraId != UNKNOWN) {
                return false;
            }

            // Disable all camera controls.
            // setCameraState(STATE_SWITCHING_CAMERA);
            // We need to keep a preview frame for the animation before
            // releasing the camera. This will trigger onPreviewTextureCopied.
            // mRendererManager.copyTexture();
            // mMainHandler.sendEmptyMessage(MSG_SWITCH_CAMERA);
            mCameraAppUi.setViewState(ViewState.VIEW_STATE_CAMERA_CLOSED);
            mMainHandler.obtainMessage(MSG_SWITCH_CAMERA, cameraId, 0)
                    .sendToTarget();
            mPendingSwitchCameraId = cameraId;
            return false;
        }

        @Override
        public boolean onSlowMotionPicked(String turnon) {
            if (mPaused || mPendingSwitchCameraId != UNKNOWN
                    || !mCameraDeviceCtrl.isCameraOpened()) {
                return false;
            }
            mCameraAppUi.resetSettings();
            mISettingCtrl.onSettingChanged(SettingConstants.KEY_SLOW_MOTION,
                    turnon);
            mCameraDeviceCtrl.applyParameters(false);
            return true;
        }

        @Override
        public boolean onHdrPicked(String value) {
            if (mPaused || mPendingSwitchCameraId != UNKNOWN
                    || !mCameraDeviceCtrl.isCameraOpened()) {
                return false;
            }
            mCameraActor.stopPreview();
            mISettingCtrl.onSettingChanged(SettingConstants.KEY_HDR, value);
            if (SETTING_ON.equals(value)) {
                mCameraAppUi.showInfo(getString(R.string.hdr_guide_capture),
                        SHOW_INFO_LENGTH_LONG);
            }
            // open/close hdr need to stop/start preview.
            mCameraDeviceCtrl.applyParameters(true);
            return true;
        }

        @Override
        public boolean onFlashPicked(String flashMode) {
            if (mPaused || mPendingSwitchCameraId != UNKNOWN
                    || !mCameraDeviceCtrl.isCameraOpened()) {
                return false;
            }
            mISettingCtrl.onSettingChanged(SettingConstants.KEY_FLASH, flashMode);
            mCameraDeviceCtrl.applyParameters(false);
            return true;
        }

        @Override
        public boolean onStereoPicked(boolean stereoType) {
            // in n3d mode, press home key to exit camera, press camera icon to
            // launch camera and click 2d/3d switch icon quickly
            if (mPaused || mPendingSwitchCameraId != UNKNOWN) {
                return false;
            }
            // switch 2d <---> 3d mode
            mIsStereoMode = !SettingUtils.readPreferredCamera3DMode(
                    mPreferences).equals(SettingUtils.STEREO3D_ENABLE);
            SettingUtils.writePreferredCamera3DMode(mPreferences,
                    mIsStereoMode ? SettingUtils.STEREO3D_ENABLE
                            : SettingUtils.STEREO3D_DISABLE);

            if (mModePicker != null) {
                mModePicker.setCurrentMode(getCurrentMode());
            }
            mCameraAppUi.refreshModeRelated();
            // mSettingManager.reInflate();

            return true;
        }

        @Override
        public boolean onModePicked(int mode, String value,
                ListPreference preference) {
            mModePicker.setModePreference(preference);
            // when click hdr, the ModePicker view should be
            // disabled during changed mode
            mModePicker.setEnabled(false);
            if (getCurrentMode() == mode) {
                // if current mode is HDR then should change to
                // photo mode.
                mModePicker.setCurrentMode(ModePicker.MODE_PHOTO);
            } else {
                // if current mode is not HDR ,it must be photo
                // mode, then set current mode as HDR.
                mModePicker.setCurrentMode(mode);
            }
            return true;
        }
    };

    public boolean isSwitchingCamera() {
        return mPendingSwitchCameraId != UNKNOWN;
    }

    public boolean isNonePickIntent() {
        return PICK_TYPE_NORMAL == mPickType;
    }

    public boolean isImageCaptureIntent() {
        return PICK_TYPE_PHOTO == mPickType;
    }

    public boolean isVideoCaptureIntent() {
        return PICK_TYPE_VIDEO == mPickType;
    }

    public boolean isVideoWallPaperIntent() {
        return PICK_TYPE_WALLPAPER == mPickType;
    }

    public boolean isStereo3DImageCaptureIntent() {
        return PICK_TYPE_STEREO3D == mPickType;
    }

    public float getWallpaperPickAspectio() {
        return mWallpaperAspectio;
    }

    public boolean isQuickCapture() {
        return mQuickCapture;
    }

    public Uri getSaveUri() {
        return mSaveUri;
    }

    public PreviewSurfaceView getPreviewSurfaceView() {
        return (mCameraDeviceCtrl == null) ? null : mCameraDeviceCtrl
                .getSurfaceView();
    }

    public String getCropValue() {
        return mCropValue;
    }

    public boolean isDualCameraDeviceEnable() {
        return isPIPMode(getCurrentMode());
    }

    public void setResultExAndFinish(int resultCode) {
        setResultEx(resultCode);
        finish();
        clearUserSettings();
    }

    public void setResultExAndFinish(int resultCode, Intent data) {
        setResultEx(resultCode, data);
        finish();
        clearUserSettings();
    }

    public boolean isVideoMode() {
        Log.d(TAG, "isVideoMode() getCurrentMode()=" + getCurrentMode());
        return (ModePicker.MODE_VIDEO == getCurrentMode()
                || ModePicker.MODE_VIDEO_3D == getCurrentMode()
                || ModePicker.MODE_VIDEO_PIP == getCurrentMode()
                || ModePicker.MODE_VIDEO_STEREO == getCurrentMode());
    }

    private void notifyOrientationChanged() {
        for (OnOrientationListener listener : mOrientationListeners) {
            if (listener != null) {
                listener.onOrientationChanged(mOrientationCompensation);
            }
        }
    }

    private class MyOrientationEventListener extends OrientationEventListener {
        private boolean mLock = false;
        private int mRestoreOrientation;

        public MyOrientationEventListener(Context context) {
            super(context);
        }

        public void setLock(boolean lock) {
            mLock = lock;
        }

        public void restoreOrientation() {
            onOrientationChanged(mRestoreOrientation);
        }

        @Override
        public void onOrientationChanged(int orientation) {
            // We keep the last known orientation. So if the user first orient
            // the camera then point the camera to floor or sky, we still have
            // the correct orientation.
            if (orientation == ORIENTATION_UNKNOWN) {
                Log.w(TAG,
                        "[onOrientationChanged]orientation is ORIENTATION_UNKNOWN,return.");
                return;
            }
            int newOrientation = Util.roundOrientation(orientation,
                    mRestoreOrientation);

            if (!mLock) {
                updateOrientation(newOrientation);
            }

            if (mRestoreOrientation != newOrientation) {
                mRestoreOrientation = newOrientation;
                mModuleManager.onOrientationChanged(mRestoreOrientation);
            }

        }

        private void updateOrientation(int orientation) {
            int newDisplayRotation = Util.getDisplayRotation(CameraActivity.this);
            if (mOrientation == orientation
                    && newDisplayRotation == mDisplayRotation) {
                return;
            }
            Log.d(TAG, "[updateOrientation]orientation:" + orientation
                    + ",mOrientation:" + mOrientation + ",newDisplayRotation:"
                    + newDisplayRotation + ",mDisplayRotation:"
                    + mDisplayRotation);
            if (newDisplayRotation != mDisplayRotation) {
                mDisplayRotation = newDisplayRotation;
                mCameraDeviceCtrl.setDisplayOrientation();
            }

            mOrientation = orientation;
            updateCompensation(mOrientation);
            mCameraDeviceCtrl.onOrientationChanged(mOrientation);

        }

        private void updateCompensation(int orientation) {
            int orientationCompensation = (orientation + Util
                    .getDisplayRotation(CameraActivity.this)) % 360;
            if (mOrientationCompensation != orientationCompensation) {
                Log.d(TAG, "[updateCompensation] mCompensation:"
                        + mOrientationCompensation + ", compensation:"
                        + orientationCompensation);
                mOrientationCompensation = orientationCompensation;
                mModuleManager.onCompensationChanged(mOrientationCompensation);
                notifyOrientationChanged();
            }
        }
    }

    // / @}

    // / M: for pick logic @{
    /**
     * An unpublished intent flag requesting to start recording straight away
     * and return as soon as recording is stopped. TODO: consider publishing by
     * moving into MediaStore.
     */
    private static final String EXTRA_QUICK_CAPTURE = "android.intent.extra.quickCapture";
    private static final String EXTRA_VIDEO_WALLPAPER_IDENTIFY = "identity"; // String
    private static final String EXTRA_VIDEO_WALLPAPER_RATION = "ratio"; // float
    private static final String EXTRA_VIDEO_WALLPAPER_IDENTIFY_VALUE = "com.mediatek.vlw";
    private static final String EXTRA_PHOTO_CROP_VALUE = "crop";
    private static final String ACTION_STEREO3D = "android.media.action.IMAGE_CAPTURE_3D";
    private static final String EXTRA_RESOLUTION_LIMIT =
            "mediatek.intent.extra.EXTRA_RESOLUTION_LIMIT";
    private static final float WALLPAPER_DEFAULT_ASPECTIO = 1.2f;

    private static final int PICK_TYPE_NORMAL = 0;
    private static final int PICK_TYPE_PHOTO = 1;
    private static final int PICK_TYPE_VIDEO = 2;
    private static final int PICK_TYPE_WALLPAPER = 3;
    private static final int PICK_TYPE_STEREO3D = 4;
    private int mPickType;
    private boolean mQuickCapture;
    private float mWallpaperAspectio;
    private Uri mSaveUri;
    private long mLimitedSize;
    private String mCropValue;
    private int mLimitedDuration;
    private int mLimitedResoltion;

    // add for MMS ->VideoCamera->playVideo,and then pause the
    // video,at this time ,we don't want share the video
    public boolean mCanShowVideoShare = true;
    public static final String CAN_SHARE = "CanShare";

    private void parseIntent() {
        Intent intent = getIntent();
        String action = intent.getAction();
        if (MediaStore.ACTION_IMAGE_CAPTURE.equals(action)
                || MediaStore.ACTION_IMAGE_CAPTURE_SECURE.equals(action)) {
            mPickType = PICK_TYPE_PHOTO;
        } else if (EXTRA_VIDEO_WALLPAPER_IDENTIFY_VALUE.equals(intent
                .getStringExtra(EXTRA_VIDEO_WALLPAPER_IDENTIFY))) {
            mWallpaperAspectio = intent.getFloatExtra(EXTRA_VIDEO_WALLPAPER_RATION,
                    WALLPAPER_DEFAULT_ASPECTIO);
            intent.putExtra(EXTRA_QUICK_CAPTURE, true);
            mPickType = PICK_TYPE_WALLPAPER;
        } else if (MediaStore.ACTION_VIDEO_CAPTURE.equals(action)) {
            mPickType = PICK_TYPE_VIDEO;
        } else if (ACTION_STEREO3D.equals(action)) {
            mPickType = PICK_TYPE_STEREO3D;
        } else {
            mPickType = PICK_TYPE_NORMAL;
            SettingUtils.setLimitResolution(0);
        }
        if (mPickType != PICK_TYPE_NORMAL) {
            mQuickCapture = intent.getBooleanExtra(EXTRA_QUICK_CAPTURE, false);
            mSaveUri = intent.getParcelableExtra(MediaStore.EXTRA_OUTPUT);
            mLimitedSize = intent.getLongExtra(MediaStore.EXTRA_SIZE_LIMIT, 0L);
            mCropValue = intent.getStringExtra(EXTRA_PHOTO_CROP_VALUE);
            mLimitedDuration = intent.getIntExtra(MediaStore.EXTRA_DURATION_LIMIT, 0);
            mLimitedResoltion = intent.getIntExtra(EXTRA_RESOLUTION_LIMIT, 0);
            SettingUtils.setLimitResolution(mLimitedResoltion);
        }
        Log.d(TAG, "parseIntent() mPickType=" + mPickType + ", mQuickCapture="
                + mQuickCapture + ", mSaveUri=" + mSaveUri + ", mLimitedSize="
                + mLimitedSize + ", mCropValue=" + mCropValue
                + ", mLimitedDuration=" + mLimitedDuration);
        if (true) {
            Log.d(TAG, "parseIntent() action=" + intent.getAction());
            Bundle extra = intent.getExtras();
            if (extra != null) {
                mCanShowVideoShare = extra.getBoolean(CAN_SHARE, true);
                for (String key : extra.keySet()) {
                    Log.v(TAG, "parseIntent() extra[" + key + "]=" + extra.get(key));
                }
            }
            if (intent.getCategories() != null) {
                for (String key : intent.getCategories()) {
                    Log.v(TAG, "parseIntent() getCategories=" + key);
                }
            }
            Log.v(TAG, "parseIntent() data=" + intent.getData());
            Log.v(TAG, "parseIntent() flag=" + intent.getFlags());
            Log.v(TAG, "parseIntent() package=" + intent.getPackage());
            Log.v(TAG, "mCanShowVideoShare = " + mCanShowVideoShare);
        }
    }

    // / M: We will restart activity for changed default path. @{
    private boolean mForceFinishing; // forcing finish

    private boolean isMountPointChanged() {
        boolean changed = false;
        String mountPoint = Storage.getMountPoint();
        Storage.updateDefaultDirectory();
        if (!mountPoint.equals(Storage.getMountPoint())) {
            changed = true;
        }
        Log.d(TAG, "isMountPointChanged() old=" + mountPoint + ", new="
                + Storage.getMountPoint() + ", return " + changed);
        return changed;
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "mReceiver.onReceive(" + intent + ")");
            String action = intent.getAction();
            if (action == null) {
                Log.d(TAG, "[mReceiver.onReceive] action is null");
                return;
            }

            switch (action) {

            case Intent.ACTION_MEDIA_EJECT:
                if (isSameStorage(intent)) {
                    Storage.setStorageReady(false);
                    mCameraActor.onMediaEject();
                }
                break;

            case Intent.ACTION_MEDIA_UNMOUNTED:
                if (isSameStorage(intent)) {
                    String internal = Storage.getInternalVolumePath();
                    if (internal != null) {
                        if (!Storage.updateDirectory(internal)) {
                            setPath(Storage.getCameraScreenNailPath());
                        }
                    }
                } else {
                    updateStorageDirectory();
                }
                mCameraAppUi.clearRemainAvaliableSpace();
                mCameraAppUi.showRemainHint();
                mCameraAppUi.forceThumbnailUpdate();
                break;

            case Intent.ACTION_MEDIA_MOUNTED:
                updateStorageDirectory();
                if (isSameStorage(intent)) {
                    Storage.setStorageReady(true);
                    mCameraAppUi.clearRemainAvaliableSpace();
                    mCameraAppUi.showRemainHint();
                }
                break;

            case Intent.ACTION_MEDIA_CHECKING:
                if (isSameStorage(intent)) {
                    mCameraAppUi.clearRemainAvaliableSpace();
                    mCameraAppUi.showRemainHint();
                }
                break;

            case Intent.ACTION_MEDIA_SCANNER_STARTED:
                updateStorageDirectory();
                if (isSameStorage(intent.getData())) {
                    mCameraAppUi.showToast(R.string.wait);
                }
                break;

            case Intent.ACTION_MEDIA_SCANNER_FINISHED:
                if (isSameStorage(intent.getData())) {
                    mCameraAppUi.clearRemainAvaliableSpace();
                    mCameraAppUi.showRemainHint();
                    mCameraAppUi.forceThumbnailUpdate();
                }
                break;

            default:
                break;
            }
        }
    };

    private void updateStorageDirectory() {
        if (!Storage.updateDefaultDirectory()) {
            setPath(Storage.getCameraScreenNailPath());
        }
    }

    private boolean isSameStorage(Intent intent) {
        StorageVolume storage = (StorageVolume) intent
                .getParcelableExtra(StorageVolume.EXTRA_STORAGE_VOLUME);

        boolean same = false;
        String mountPoint = null;
        String intentPath = null;
        if (storage != null) {
            mountPoint = Storage.getMountPoint();
            intentPath = storage.getPath();
            if (mountPoint != null && mountPoint.equals(intentPath)) {
                same = true;
            }
        }
        Log.d(TAG, "isSameStorage() mountPoint=" + mountPoint + ", intentPath="
                + intentPath + ", return " + same);
        return same;
    }

    private boolean isSameStorage(Uri uri) {
        if (!Storage.updateDefaultDirectory()) {
            Log.d(TAG, "isSameStorage(uri)/same= updateDefaultDirectory");
            setPath(Storage.getCameraScreenNailPath());
        }
        boolean same = false;
        String mountPoint = null;
        String intentPath = null;
        if (uri != null) {
            mountPoint = Storage.getMountPoint();
            intentPath = uri.getPath();
            if (mountPoint != null && mountPoint.equals(intentPath)) {
                same = true;
            }
        }
        Log.d(TAG, "isSameStorage(" + uri + ") mountPoint=" + mountPoint
                + ", intentPath=" + intentPath + ", return " + same);
        mCameraAppUi.forceThumbnailUpdate();
        return same;
    }

    private void installIntentFilter() {
        // install an intent filter to receive SD card related events.
        IntentFilter intentFilter = new IntentFilter(
                Intent.ACTION_MEDIA_MOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
        intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
        intentFilter.addAction(Intent.ACTION_MEDIA_CHECKING);
        intentFilter.addDataScheme("file");
        registerReceiver(mReceiver, intentFilter);
    }

    private void uninstallIntentFilter() {
        try {
            unregisterReceiver(mReceiver);
        } catch (IllegalArgumentException ex) {
            Log.e(TAG, "[uninstallIntentFilter] error ", ex);
        }
    }

    private void clearUserSettings() {
        Log.d(TAG, "clearUserSettings() isFinishing()=" + isFinishing());
        if (mISettingCtrl != null && isFinishing()) {
            mISettingCtrl.resetSetting();
        }
    }

    // / M: for other things @{
    private void addIdleHandler() {
        MessageQueue queue = Looper.myQueue();
        queue.addIdleHandler(new MessageQueue.IdleHandler() {
            @Override
            public boolean queueIdle() {
                Storage.ensureOSXCompatible();
                return false;
            }
        });
    }

    // 3D Related
    public boolean isStereoMode() {
        return mIsStereoMode;
    }

    // SurfaceView For Native 3D
    // For SurfaceTexture need GPU,Native Need GPU,Then GPU crushed!

    private void initializeStereo3DMode() {
        if (isStereo3DImageCaptureIntent()) {
            mIsStereoMode = true;
            SettingUtils.writePreferredCamera3DMode(mPreferences,
                    SettingUtils.STEREO3D_ENABLE);
        } else {
            // when initialize we always write disable state
            mIsStereoMode = false;
            SettingUtils.writePreferredCamera3DMode(mPreferences,
                    SettingUtils.STEREO3D_DISABLE);
        }
    }

    private boolean isFaceBeautyEnable() {
        String facebautyMode = getPreferences().getString(
                SettingConstants.KEY_MULTI_FACE_BEAUTY,
                getResources().getString(R.string.face_beauty_single_mode));
        // Log.i(TAG, "facebautyMode = " + facebautyMode);
        return !getResources().getString(R.string.pref_face_beauty_mode_off)
                .equals(facebautyMode);
    }

    private IExternalDeviceCtrl.Listener mListener = new IExternalDeviceCtrl.Listener() {

        @Override
        public void onStateChanged(boolean enabled) {
            Log.d(TAG, "[onStateChanged] enable = " + enabled);
            mModuleManager.setVideoRecorderEnable(!enabled);
        }
    };

    //**********************************************************TODO PIP and stereo
    private boolean mIsStereoToVideoMode;
    public boolean isNeedOpenStereoCamera() {
        boolean enable = false;
        enable = (SettingUtils.readPreferredStereoCamera(mPreferences))
                .equals(SettingUtils.STEREO_CAMERA_ON)
                || getCurrentMode() == ModePicker.MODE_STEREO_CAMERA
                || getCurrentMode() == ModePicker.MODE_PHOTO_STEREO
                || getCurrentMode() == ModePicker.MODE_VIDEO_STEREO;
        Log.d(TAG, "[isNeedOpenStereoCamera] mIsStereoToVideoMode:"
                + mIsStereoToVideoMode + ",mode:" + getCurrentMode()
                + ",enable:" + enable);
        if (mIsModeChanged
                && getListPreference(SettingConstants.ROW_SETTING_FAST_AF) != null
                && SETTING_ON.equals(getListPreference(
                        SettingConstants.ROW_SETTING_FAST_AF).getValue())) {
            enable = true;
        }
        if (mIsStereoToVideoMode) {
            enable = false;
            mIsStereoToVideoMode = false;
        }
        if (getCurrentMode() == ModePicker.MODE_PHOTO_PIP) {
            enable = false;
        }
        enable = enable && isNonePickIntent();
        Log.d(TAG, "needOpenStereoCamera enable = " + enable);
        return enable;
    }

    private boolean isPIPMode(int mode) {
        return mode == ModePicker.MODE_VIDEO_PIP
                || mode == ModePicker.MODE_PHOTO_PIP;
    }

    private boolean isPIPModeSwitch(int lastMode, int newMode) {
        return (isPIPMode(lastMode) && !isPIPMode(newMode))
                || (!isPIPMode(lastMode) && isPIPMode(newMode));
    }
    private boolean isStereoMode(int mode) {
        return mode == ModePicker.MODE_VIDEO_STEREO
                || mode == ModePicker.MODE_STEREO_CAMERA;
    }
    private boolean isRefocusSwitchNormal(int lastMode, int newMode) {
        return (lastMode == ModePicker.MODE_STEREO_CAMERA && newMode
                != ModePicker.MODE_VIDEO)
                || (lastMode != ModePicker.MODE_STEREO_CAMERA
                && newMode == ModePicker.MODE_STEREO_CAMERA);
    }
    private boolean isDenoiseSwitchNormal(int lastMode, int newMode) {
        return (lastMode == ModePicker.MODE_PHOTO_STEREO && newMode
                != ModePicker.MODE_VIDEO)
                || (lastMode != ModePicker.MODE_PHOTO_STEREO
                && newMode == ModePicker.MODE_PHOTO_STEREO);
    }

    private boolean isRefocusSwitchVideo(int lastMode, int newMode) {
        Log.d(TAG, "isRefocusSwitchVideo lastMode = " + lastMode
                + ", newMode = " + newMode);
        return lastMode == ModePicker.MODE_STEREO_CAMERA
                && newMode == ModePicker.MODE_VIDEO;
    }
    private boolean isStereoModeSwitch(int lastMode, int newMode) {
        return (isStereoMode(lastMode) && !isStereoMode(newMode))
                || (!isStereoMode(lastMode) && isStereoMode(newMode));
    }
    private boolean isFastAfEnabled() {
        Log.d(TAG, "isFastAfEnabled" + getListPreference(
                SettingConstants.KEY_FAST_AF));
        if (getListPreference(SettingConstants.KEY_FAST_AF) != null &&
                "on".equals(getListPreference(
                        SettingConstants.KEY_FAST_AF).getValue())) {
            return true;
        }
        return false;
    }

    private void judgeSensorSwitchStereoMode() {
        mStereoCaptureSuptSensorId = com.android.camera.ParametersHelper
                .getIdStereoCaptureSupt(getParameters());
        mDenoiseSuptSensorId = com.android.camera.ParametersHelper
                .getIdStereoDenoiseSupt(getParameters());
    }

    // need close camera when stereo mode switch other mode (except video mode).
    private void releaseCameraSwitchStereoMode(int oldMode, int newMode) {
        if (!com.android.camera.ParametersHelper.isVsDofSupported(getParameters())) {
            if (!isFastAfEnabled() && isRefocusSwitchVideo(oldMode, newMode)) {
                if (newMode == ModePicker.MODE_VIDEO) {
                    mIsStereoToVideoMode = true;
                }
                doStereoModeChanged(true);
            }

            if (isRefocusSwitchNormal(oldMode, newMode)) {
                mCameraDeviceCtrl.closeCamera(false);
            }
        } else {
            if (isStereoModeSwitch(oldMode, newMode)) {
                mCameraDeviceCtrl.closeCamera(false);
            }
        }
        // De-noise mode need close camera.
        if (isDenoiseSwitchNormal(oldMode, newMode)) {
            mCameraDeviceCtrl.closeCamera(false);
        }
    }

    private boolean isStereoModeChanged(int oldMode, int newMode) {
        mModuleManager.setModeSettingValue(mCameraActor.getCameraModeType(newMode),
                SETTING_ON);
        // Stereo capture switch other mode (except video mode) check.
        if (!com.android.camera.ParametersHelper.isVsDofSupported(getParameters())) {
            if (isRefocusSwitchNormal(oldMode, newMode)) {
                Log.i(TAG, "isRefocusSwitchNormal return");
                return true;
            }
        // Vsdof Stereo capture switch other mode check.
        } else {
            if (isStereoModeSwitch(oldMode, newMode)) {
                Log.i(TAG, "isStereoModeSwitch return");
                return true;
            }
        }
        // De-noise switch other mode check.
        if (isDenoiseSwitchNormal(oldMode, newMode)) {
            Log.i(TAG, "isDenoiseSwitchNormal return");
            return true;
        }
        return false;
    }
    private void doStereoModeChanged(boolean needSync) {
        // cameraId == 0, need to open sensor 0.
        // cameraId == 1, need to open sensor 1.
        // cameraId == 2, need to open by preference.
        int cameraId = -1;
        if (mNextMode == ModePicker.MODE_STEREO_CAMERA) {
            cameraId = mStereoCaptureSuptSensorId;
        } else if (mNextMode == ModePicker.MODE_PHOTO_STEREO) {
            cameraId = mDenoiseSuptSensorId;
        }
        int id = cameraId == 2 ? -1 : cameraId;
        Log.i(TAG, "doStereoModeChanged id = " + id);
        mCameraDeviceCtrl.openStereoCamera(id, needSync);
    }

    private void doPIPModeChanged(int cameraId) {
        Log.d(TAG, "doPIPModeChanged");
        // when switch to open main and sub camera, should close first
        mCameraAppUi.collapseViewManager(true);
        clearFocusAndFace();
        mCameraDeviceCtrl.unInitializeFocusManager();

        // here set these variables null to initialize them again.
        mPreferences.setLocalId(CameraActivity.this, cameraId);
        SettingUtils.upgradeLocalPreferences(mPreferences.getLocal());
        SettingUtils.writePreferredCameraId(mPreferences, cameraId);

        mCameraDeviceCtrl.openCamera(cameraId);
    }

    // ****************************************************************
    public CameraManager.CameraProxy getCameraDevice() {
        return mCameraDeviceCtrl.getCameraDevice();
    }

    public FocusManager getFocusManager() {
        return mCameraDeviceCtrl.getFocusManager();
    }

    public Parameters getParameters() {
        return mCameraDeviceCtrl.getParameters();
    }

    public Parameters getTopParameters() {
        return mCameraDeviceCtrl.getTopParameters();
    }

    public int getCameraId() {
        return mCameraDeviceCtrl.getCameraId();
    }

    // for PIP
    public int getOriCameraId() {
        return mOriCameraId;
    }

    public int getTopCameraId() {
        return mCameraDeviceCtrl.getTopCameraId();
    }

    public void applyParameterForCapture(final SaveRequest request) {
        mCameraDeviceCtrl.applyParameterForCapture(request);
    }

    public void applyParameterForFocus(final boolean setArea) {
        mCameraDeviceCtrl.applyParameterForFocus(setArea);
    }

    public int getDisplayOrientation() {
        return mCameraDeviceCtrl.getDisplayOrientation();
    }

    public void startAsyncZoom(final int zoomValue) {
        mCameraDeviceCtrl.startAsyncZoom(zoomValue);
    }

    public boolean isCameraIdle() {
        return mCameraDeviceCtrl.isCameraIdle();
    }

    public void hideRootCover() {
        mCameraDeviceCtrl.hideRootCover();
    }

    private void keepMediaProviderInstance() {
        // We want to keep a reference to MediaProvider in camera's lifecycle.
        // TODO: Utilize mMediaProviderClient instance to replace
        // ContentResolver calls.
        Log.d(TAG, "keepMediaProviderInstance() mMediaProviderClient =  "
                + mMediaProviderClient);
        if (mMediaProviderClient == null) {
            mMediaProviderClient = getContentResolver().acquireContentProviderClient(
                    MediaStore.AUTHORITY);
        }
    }

    private class ModuleCtrlImpl implements IModuleCtrl {
        private CameraActivity mCamera;

        public ModuleCtrlImpl(CameraActivity camera) {
            mCamera = camera;
        }

        @Override
        public boolean applyFocusParameters(boolean setArea) {
            mCameraDeviceCtrl.applyParameterForFocus(setArea);
            return true;
        }

        @Override
        public int getOrientation() {
            return mOrientation;
        }

        @Override
        public int getDisplayOrientation() {
            return mCameraDeviceCtrl.getDisplayOrientation();
        }

        @Override
        public int getDisplayRotation() {
            return mDisplayRotation;
        }

        @Override
        public int getOrientationCompensation() {
            return mOrientationCompensation;
        }

        @Override
        public int getJpegOrientation() {
            return mCameraDeviceCtrl.getCurCameraDevice().getJpegRotation();
        }

        @Override
        public boolean lockOrientation() {
            Log.d(TAG, "[lockOrientation]...");
            mCamera.setOrientation(true,
                    OrientationEventListener.ORIENTATION_UNKNOWN);
            return true;
        }

        @Override
        public boolean unlockOrientation() {
            Log.d(TAG, "[unlockOrientation]...");
            mCamera.setOrientation(false,
                    OrientationEventListener.ORIENTATION_UNKNOWN);
            return true;
        }

        @Override
        public void setOrientation(boolean lock, int orientation) {
            Log.d(TAG, "[setOrientation] lock = " + lock + ", orientation = "
                    + orientation);
            mCamera.setOrientation(lock, orientation);
        }

        @Override
        public boolean enableOrientationListener() {
            mOrientationListener.enable();
            return true;
        }

        @Override
        public boolean disableOrientationListener() {
            mOrientationListener.disable();
            return true;
        }

        public Location getLocation() {
            return mLocationManager.getCurrentLocation();
        }

        public Uri getSaveUri() {
            return mSaveUri;
        }

        public String getCropValue() {
            return mCropValue;
        }

        public int getResolution() {
            return mLimitedResoltion;
        }

        public void setResultAndFinish(int resultCode) {
            mCamera.setResultExAndFinish(resultCode);
        }

        public void setResultAndFinish(int resultCode, Intent data) {
            mCamera.setResultExAndFinish(resultCode, data);
        }

        public boolean isSecureCamera() {
            return mSecureCamera;
        }

        public boolean isImageCaptureIntent() {
            return PICK_TYPE_PHOTO == mPickType;
        }

        @Override
        public void startFaceDetection() {
            mCameraActor.startFaceDetection();
        }

        @Override
        public void stopFaceDetection() {
            mCameraActor.stopFaceDetection();
        }

        @Override
        public boolean isVideoCaptureIntent() {
            return PICK_TYPE_VIDEO == mPickType;
        }

        @Override
        public boolean isNonePickIntent() {
            return PICK_TYPE_NORMAL == mPickType;
        }

        @Override
        public Intent getIntent() {
            return mCamera.getIntent();
        }

        @Override
        public boolean isQuickCapture() {
            return mQuickCapture;
        }

        @Override
        public void backToLastMode() {
            mCameraDeviceCtrl.waitCameraStartUpThread(false);
            mModePicker.setCurrentMode(mPrevMode);
        }

        @Override
        public void backToCallingActivity(int resultCode, Intent data) {
            setResultExAndFinish(resultCode, data);
        }

        @Override
        public boolean getSurfaceTextureReady() {
            // TODO
            return true;
        }

        @Override
        public ComboPreferences getComboPreferences() {
            return mCamera.getPreferences();
        }

        @Override
        public void switchCameraDevice() {
            mCameraDeviceCtrl.doSwitchCameraDevice();
        }

        @Override
        public CameraModeType getNextMode() {
            return mCameraActor.getCameraModeType(mNextMode);
        }

        @Override
        public CameraModeType getPrevMode() {
            return mCameraActor.getCameraModeType(mPrevMode);
        }

        @Override
        public boolean setFaceBeautyEnalbe(boolean isEnable) {
            mFrameManager.enableFaceBeauty(isEnable);
            return isEnable;
        }

        @Override
        public boolean initializeFrameView(boolean isEnableObject) {
            mFrameManager.initializeFrameView(isEnableObject);
            return false;
        }

        @Override
        public boolean clearFrameView() {
            getFrameView().clear();
            return true;
        }

        @Override
        public void setFaces(Face[] faces) {
            getFrameView().setFaces(faces);
        }

        @Override
        public Surface getPreviewSurface() {
            return mCameraDeviceCtrl.getSurfaceView().getHolder().getSurface();
        }

        @Override
        public boolean isFirstStartUp() {
            return mCameraDeviceCtrl.isFirstStartUp();
        }

        @Override
        public void previewStarted() {
            mCameraDeviceCtrl.detachSurfaceViewLayout();
        }
    }
}