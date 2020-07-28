package com.mediatek.camera.feature.setting;

import android.app.Activity;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.view.KeyEvent;
import android.view.View;

import com.mediatek.camera.R;
import com.mediatek.camera.common.ICameraContext;
import com.mediatek.camera.common.app.IApp;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.setting.ISettingManager.SettingController;
import com.mediatek.camera.common.setting.SettingBase;
import com.mediatek.camera.common.utils.CameraUtil;
import com.mediatek.camera.portability.SystemProperties;

import java.util.ArrayList;
import java.util.List;

/**
 * Switch Camera setting item.
 *
 */
public class CameraSwitcher extends SettingBase {
    private static final LogUtil.Tag TAG = new LogUtil.Tag(CameraSwitcher.class.getSimpleName());

    private static final String CAMERA_FACING_BACK = "back";
    private static final String CAMERA_FACING_FRONT = "front";
    private static final String CAMERA_DEFAULT_FACING = CAMERA_FACING_BACK;

    private static final String KEY_CAMERA_SWITCHER = "key_camera_switcher";
    private String mFacing;
    private View mSwitcherView;
    private static final String SWITCH_CAMERA_DEBUG_PROPERTY = "mtk.camera.switch.camera.debug";
    private static final String DEBUG_CAMERA_ID_PROPERTY = "mtk.camera.switch.id.debug";

    // [Add for CCT tool] Receive keycode and switch camera @{
    private KeyEventListenerImpl mKeyEventListener;
    // @}

    @Override
    public void init(IApp app,
                     ICameraContext cameraContext,
                     SettingController settingController) {
        super.init(app, cameraContext, settingController);
        mFacing = mDataStore.getValue(KEY_CAMERA_SWITCHER, CAMERA_DEFAULT_FACING, getStoreScope());

        int numOfCameras = Camera.getNumberOfCameras();
        if (numOfCameras > 1) {
            List<String> camerasFacing = getCamerasFacing(numOfCameras);
            if (camerasFacing.size() == 0) {
                return;
            }
            if (camerasFacing.size() == 1) {
                mFacing = camerasFacing.get(0);
                setValue(mFacing);
                return;
            }

            setSupportedPlatformValues(camerasFacing);
            setSupportedEntryValues(camerasFacing);
            setEntryValues(camerasFacing);

            mSwitcherView = initView();
            mAppUi.addToQuickSwitcher(mSwitcherView, 0);
        } else if (numOfCameras == 1) {
            CameraInfo info = new CameraInfo();
            Camera.getCameraInfo(0, info);
            if (info.facing == CameraInfo.CAMERA_FACING_BACK) {
                mFacing = CAMERA_FACING_BACK;
            } else if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
                mFacing = CAMERA_FACING_FRONT;
            }
        }
        setValue(mFacing);

        // [Add for CCT tool] Receive keycode and switch camera @{
        mKeyEventListener = new KeyEventListenerImpl();
        mApp.registerKeyEventListener(mKeyEventListener, IApp.DEFAULT_PRIORITY);
        // @}
    }

    @Override
    public void unInit() {
        if (mSwitcherView != null) {
            mSwitcherView.setOnClickListener(null);
            mAppUi.removeFromQuickSwitcher(mSwitcherView);
        }
        // [Add for CCT tool] Receive keycode and switch camera @{
        mApp.unRegisterKeyEventListener(mKeyEventListener);
        // @}
    }

    @Override
    public void postRestrictionAfterInitialized() {

    }

    @Override
    public void refreshViewEntry() {
        if (mSwitcherView != null) {
            if (getEntryValues().size() <= 1) {
                mSwitcherView.setVisibility(View.GONE);
            } else {
                mSwitcherView.setVisibility(View.VISIBLE);
            }
        }
    }

    @Override
    public SettingType getSettingType() {
        return SettingType.PHOTO_AND_VIDEO;
    }

    @Override
    public String getKey() {
        return KEY_CAMERA_SWITCHER;
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
    public String getStoreScope() {
        return mDataStore.getGlobalScope();
    }

    private List<String> getCamerasFacing(int numOfCameras) {
        List<String> camerasFacing = new ArrayList<>();
        for (int i = 0; i < numOfCameras; i++) {
            CameraInfo info = new CameraInfo();
            Camera.getCameraInfo(i, info);

            String facing = null;
            if (info.facing == CameraInfo.CAMERA_FACING_BACK) {
                facing = CAMERA_FACING_BACK;
            } else if (info.facing == CameraInfo.CAMERA_FACING_FRONT) {
                facing = CAMERA_FACING_FRONT;
            }

            if (!camerasFacing.contains(facing)) {
                camerasFacing.add(facing);
            }
        }
        return camerasFacing;
    }

    private View initView() {
        Activity activity = mApp.getActivity();
        View switcher = activity.getLayoutInflater().inflate(R.layout.camera_switcher, null);

        switcher.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (SystemProperties.getInt(SWITCH_CAMERA_DEBUG_PROPERTY, 0) == 1) {
                    LogHelper.d(TAG, "[onClick], enter debug mode.");
                    switchCameraInDebugMode();
                } else {
                    String nextFacing = mFacing.equals(CAMERA_FACING_BACK) ? CAMERA_FACING_FRONT
                            : CAMERA_FACING_BACK;
                    LogHelper.d(TAG, "[onClick], switch camera to " + nextFacing);
                    String newCameraId = mFacing.equals(CAMERA_FACING_BACK) ? CameraUtil
                            .getCamIdsByFacing(false).get(0) : CameraUtil.getCamIdsByFacing(true)
                            .get(0);
                    boolean success = mApp.notifyCameraSelected(newCameraId);
                    if (success) {
                        LogHelper.d(TAG, "[onClick], switch camera success.");
                        mFacing = nextFacing;
                        mDataStore.setValue(KEY_CAMERA_SWITCHER, mFacing, getStoreScope(), true);
                    }
                    mSwitcherView.setContentDescription(mFacing);
                }

            }
        });
        switcher.setContentDescription(mFacing);
        return switcher;
    }

    private void switchCameraInDebugMode() {
        LogHelper.d(TAG, "[switchCameraInDebugMode]");
        String requestCamera = SystemProperties.getString(DEBUG_CAMERA_ID_PROPERTY, "back-0");
        int cameraIndex = 0;
        String resultCameraId = "0";
        List<String> backIds = CameraUtil.getCamIdsByFacing(true);
        List<String> frontIds = CameraUtil.getCamIdsByFacing(false);
        cameraIndex = Integer.parseInt(requestCamera.substring(requestCamera.indexOf("-") + 1));
        if (requestCamera.contains(CAMERA_FACING_BACK) && backIds == null) {
            LogHelper.e(TAG, "[switchCameraInDebugMode] backIds is null");
            return;
        }
        if (requestCamera.contains(CAMERA_FACING_FRONT) && frontIds == null) {
            LogHelper.e(TAG, "[switchCameraInDebugMode] frontIds is null");
            return;
        }
        if (requestCamera.contains(CAMERA_FACING_BACK)) {
            if (cameraIndex < backIds.size()) {
                resultCameraId = backIds.get(cameraIndex);
            } else {
                LogHelper.e(TAG, "[switchCameraInDebugMode] invalid back camera index "
                        + cameraIndex);
                return;
            }
        } else if (requestCamera.contains(CAMERA_FACING_FRONT)) {
            if (cameraIndex < frontIds.size()) {
                resultCameraId = frontIds.get(cameraIndex);
            } else {
                LogHelper.e(TAG, "[switchCameraInDebugMode] invalid front camera index "
                        + cameraIndex);
                return;
            }
        }
        LogHelper.i(TAG, "[switchCameraInDebugMode] requestCamera " + requestCamera
                + ",resultCameraId " + resultCameraId);
        mApp.notifyCameraSelected(resultCameraId);
        mSwitcherView.setContentDescription(requestCamera);
        mFacing = requestCamera.substring(0, requestCamera.indexOf("-"));
        mDataStore.setValue(KEY_CAMERA_SWITCHER, mFacing, getStoreScope(), true);
    }

    // [Add for CCT tool] Receive keycode and switch camera @{
    // KEYCODE_C: click switch camera icon
    private class KeyEventListenerImpl implements IApp.KeyEventListener {
        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event) {
            if (keyCode != CameraUtil.KEYCODE_SWITCH_CAMERA
                    || !CameraUtil.isSpecialKeyCodeEnabled()) {
                return false;
            }
            return true;
        }

        @Override
        public boolean onKeyUp(int keyCode, KeyEvent event) {
            if (keyCode != CameraUtil.KEYCODE_SWITCH_CAMERA
                    || !CameraUtil.isSpecialKeyCodeEnabled()) {
                return false;
            }
            if (mSwitcherView != null && mSwitcherView.getVisibility() == View.VISIBLE
                    && mSwitcherView.isEnabled()) {
                mSwitcherView.performClick();
            }
            return true;
        }
    }
    // @}
}
