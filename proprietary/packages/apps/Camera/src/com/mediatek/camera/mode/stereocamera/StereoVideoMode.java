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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.camera.mode.stereocamera;

import android.view.MotionEvent;

import com.android.camera.R;

import com.mediatek.camera.ICameraContext;
import com.mediatek.camera.mode.VideoMode;
import com.mediatek.camera.mode.stereocamera.settingrule.StereoVideoQualityRule;
import com.mediatek.camera.platform.ICameraAppUi.GestureListener;
import com.mediatek.camera.platform.ICameraAppUi.SpecViewType;
import com.mediatek.camera.platform.ICameraView;
import com.mediatek.camera.platform.Parameters;
import com.mediatek.camera.setting.ParametersHelper;
import com.mediatek.camera.setting.SettingConstants;
import com.mediatek.camera.util.Log;
import com.mediatek.camera.util.Util;
/**
 * This class used for Stereo record video.
 */
public class StereoVideoMode extends VideoMode implements StereoView.Listener {
    private static final String TAG = "StereoVideoMode";

    private static final String KEY_VS_DOF_LEVEL = "stereo-dof-level";
    //Notify media recorder that stop audio first then stop video for
    //stop recorder performance in vsdof. Default stop video first,
    //and parameter is "media-param-audio-stop-first=0".
    private static final String MEDIA_RECORDER_AUDIO_STOP_FIRST =
            "media-param-audio-stop-first=1";
    //Event: notify view update
    private static final int TOUCH_EVENT = 0;
    private ICameraView mStereoView;

    /**
     * Create a stereo video mode.
     * @param cameraContext camera context instance.
     */
    public StereoVideoMode(ICameraContext cameraContext) {
        super(cameraContext);
        Log.d(TAG, "[StereoVideoMode]constructor...");
        //setStereoSettingRules(cameraContext);
    }

    @Override
    public boolean open() {
        Log.d(TAG, "[openMode]...");
        super.open();
        initStereoView();
        mICameraAppUi.setGestureListener(mStereoGestureListener);
        return true;
    }

    @Override
    public boolean close() {
        Log.d(TAG, "[closeMode]...");
        uninitStereoView();
        mICameraAppUi.setGestureListener(null);
        super.close();
        return true;
    }

    @Override
    public boolean execute(ActionType type, Object... arg) {
        if (type != ActionType.ACTION_ORITATION_CHANGED) {
            Log.d(TAG, "[execute]type = " + type);
        }
        switch (type) {
        case ACTION_SHUTTER_BUTTON_LONG_PRESS:
            mICameraAppUi
                    .showInfo(mActivity
                            .getString(R.string.accessibility_switch_to_dual_camera)
                            + mActivity
                                    .getString(R.string.camera_continuous_not_supported));
            break;

        case ACTION_ON_CAMERA_OPEN:
            updateDevice();
            ParametersHelper.setVsDofMode(mICameraDevice.getParameters(), true);
            ParametersHelper.setStereoCaptureMode(mICameraDevice.getParameters(), true);
            break;

        case ACTION_ON_SINGLE_TAP_UP:
            Parameters params = mICameraDevice.getParameters();
            if (params == null || (params != null &&
                    params.getMaxNumFocusAreas() > 0)) {
                return super.execute(type, arg);
            }
            mStereoView.update(TOUCH_EVENT, (Integer) arg[1], (Integer) arg[2]);
            break;

        default:
            return super.execute(type, arg);
        }

        return true;
    }
    @Override
    public void resume() {
    }

    @Override
    public void pause() {
    }

    @Override
    public void onVsDofLevelChanged(String level) {
        setVsDofLevelParameter(level);
    }

    @Override
    public void onTouchPositionChanged(String value) {
        setTouchPositionParameter(value);
    }

    @Override
    public CameraModeType getCameraModeType() {
        return CameraModeType.EXT_MODE_VIDEO_STEREO;
    }

    @Override
    protected void initializeShutterStatus() {
        mICameraAppUi.setPhotoShutterEnabled(false);
    }

    @Override
    protected boolean startNormalRecording() {
        boolean isStarted = super.startNormalRecording();
        try {
            Util.setParametersExtra(mMediaRecorder, MEDIA_RECORDER_AUDIO_STOP_FIRST);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        return isStarted;
    }

    @Override
    protected void doStartPreview() {
        Parameters params = mICameraDevice.getParameters();
        if (ParametersHelper.isDenoiseSupported(params)) {
            params.set("preview-frame-rate", 24);
            mICameraDevice.applyParameters();
        }
        super.doStartPreview();
    }

    private void initStereoView() {
        if (mStereoView == null) {
            mStereoView = mICameraAppUi.getCameraView(
                    SpecViewType.MODE_STEREO);
            mStereoView.init(mActivity, mICameraAppUi, mIModuleCtrl);
            mStereoView.setListener(this);
            mStereoView.show();
        }
    }

    private void uninitStereoView() {
        if (mStereoView != null) {
            mStereoView.uninit();
        }
    }

    private void setVsDofLevelParameter(String level) {
        Log.d(TAG, "[setVsDofLevelParameter] level = " + level
                  + "device = " + mICameraDevice);
        if (mICameraDevice != null) {
            mICameraDevice.setParameter(KEY_VS_DOF_LEVEL, level);
            mICameraDevice.applyParameters();
        }
    }

    private void setTouchPositionParameter(String value) {
        Log.i(TAG, "[setTouchPositionParameter] value = " + value
                + "device = " + mICameraDevice);
        if (mICameraDevice != null) {
            mICameraDevice.setParameter("stereo-touch-position", value);
            mICameraDevice.applyParameters();
        }
    }

    private void setStereoSettingRules(ICameraContext cameraContext) {
        Log.d(TAG, "[setStereoSettingRules]...");
        StereoVideoQualityRule videoQualityRule = new StereoVideoQualityRule(cameraContext,
                SettingConstants.KEY_VIDEO_STEREO);
        videoQualityRule.addLimitation("on", null, null);
        mISettingCtrl.addRule(SettingConstants.KEY_VIDEO_STEREO, SettingConstants.KEY_VIDEO_QUALITY,
                videoQualityRule);
    }

    private StereoGestureListener mStereoGestureListener = new StereoGestureListener() {
        @Override
        public boolean onSingleTapUp(float x, float y) {
            return false;
        }

        @Override
        public boolean onLongPress(float x, float y) {
            return false;
        }
    };
}
