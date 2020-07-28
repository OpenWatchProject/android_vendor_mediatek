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

package com.mediatek.camera.v2.detection;

import android.app.Activity;
import android.graphics.RectF;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureRequest.Builder;
import android.hardware.camera2.TotalCaptureResult;
import android.view.ViewGroup;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.detection.asd.AsdPresenterImpl;
import com.mediatek.camera.v2.detection.asd.AsdView;
import com.mediatek.camera.v2.detection.asd.IAsdView;
import com.mediatek.camera.v2.detection.facedetection.FdPresenterImpl;
import com.mediatek.camera.v2.detection.facedetection.FdViewManager;
import com.mediatek.camera.v2.module.ModuleListener;
import com.mediatek.camera.v2.module.ModuleListener.CaptureType;
import com.mediatek.camera.v2.module.ModuleListener.RequestType;
import com.mediatek.camera.v2.platform.app.AppController;
import com.mediatek.camera.v2.platform.app.AppUi;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.ISettingServant.ISettingChangedListener;
import com.mediatek.camera.v2.setting.SettingCtrl;
import com.mediatek.camera.v2.util.SettingKeys;

import junit.framework.Assert;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * A manager used to manage additions. It will broadcast events from modules to additions.
 */
public class DetectionManager implements IDetectionManager, ISettingChangedListener,
        IDetectionManager.IDetectionListener {
    private static final Tag TAG = new Tag(DetectionManager.class.getSimpleName());
    private final AppController mAppController;
    private final AppUi mAppUi;
    private final ISettingServant mSettingServant;
    private final ModuleListener mModuleListener;
    private final CopyOnWriteArrayList<IDetectionCaptureObserver> mCaptureObservers
        = new CopyOnWriteArrayList<IDetectionCaptureObserver>();

    private ArrayList<String> mCaredSettingChangedKeys = new ArrayList<String>();

    IDetectionPresenter mPresenter;
    // addition feature
    private IDetectionPresenter mDetectionPresenter;

    private IAsdView mAsdView;
    private AsdPresenterImpl mAsdPresenterImpl;

    private FdPresenterImpl mFdPresenterImpl;
    private FdViewManager mFdViewManager;

    private final static String DETECTION_ON = "on";
    private String mInitializedCameraId;

    /**
     * The constructor.
     *
     * @param app The controller at application level.
     * @param moduleListener The listener used by 3A, Mode, Addition.
     * @param cameraId The current camera id.
     */
    public DetectionManager(AppController app, ModuleListener moduleListener, String cameraId) {
        Assert.assertNotNull(app);
        Assert.assertNotNull(moduleListener);
        mAppController = app;
        mInitializedCameraId = cameraId;
        mAppUi = mAppController.getCameraAppUi();
        mModuleListener = moduleListener;
        mSettingServant = mAppController.getServices().getSettingController()
                .getSettingServant(cameraId);

        // switch camera will not be notified,so we need initiative to listener
        // the switch camera occur.
        addCaredSettingChangedKeys(SettingKeys.KEY_CAMERA_ID);

        // initialize special feature
        mAsdView = new AsdView(mAppController.getActivity(), mAppUi, mSettingServant);
        mAsdPresenterImpl = new AsdPresenterImpl(mAsdView, this, mSettingServant);
        addCaredSettingChangedKeys(SettingKeys.KEY_ASD);

        mFdViewManager = new FdViewManager(mSettingServant);
        mFdPresenterImpl = new FdPresenterImpl(mFdViewManager, this);
        addCaredSettingChangedKeys(SettingKeys.KEY_CAMERA_FACE_DETECT);
    }

    @Override
    public void onSettingChanged(Map<String, String> result) {
        // force update addition state,
        // because feature's state change will not be notified when switch camera.
        if (result.get(SettingKeys.KEY_CAMERA_ID) != null) {
            updateDetectionState(SettingKeys.KEY_ASD);
            updateDetectionState(SettingKeys.KEY_CAMERA_FACE_DETECT);
            return;
        }
        String asdValue = result.get(SettingKeys.KEY_ASD);
        String fdValue = result.get(SettingKeys.KEY_CAMERA_FACE_DETECT);

        if (asdValue != null) {
            updateDetectionState(SettingKeys.KEY_ASD);
        }

        if (fdValue != null) {
            updateDetectionState(SettingKeys.KEY_CAMERA_FACE_DETECT);
        }
    }

    @Override
    public void open(Activity activity, ViewGroup parentView, boolean isCaptureIntent) {
        mSettingServant.registerSettingChangedListener(this, mCaredSettingChangedKeys,
                ISettingChangedListener.MIDDLE_PRIORITY);
        mFdViewManager.open(activity, parentView);
    }

    @Override
    public void resume() {
        LogHelper.i(TAG, "resume");
        resumeDetectionState(SettingKeys.KEY_ASD);
        resumeDetectionState(SettingKeys.KEY_CAMERA_FACE_DETECT);
    }

    @Override
    public void pause() {
        LogHelper.i(TAG, "pause");
    }

    @Override
    public void close() {
        if (mFdViewManager != null) {
            mFdViewManager.close();
        }
        mSettingServant.unRegisterSettingChangedListener(this);
        mAsdPresenterImpl.stopDetection();
    }

    @Override
    public void onSingleTapUp(float x, float y) {

    }

    @Override
    public void onLongPressed(float x, float y) {

    }

    @Override
    public void onPreviewAreaChanged(RectF previewArea) {
        LogHelper.i(TAG, "onPreviewAreaChanged " + " mFdViewManager:" + mFdViewManager);
        if (mFdViewManager != null) {
            mFdViewManager.onPreviewAreaChanged(previewArea);
        }
    }

    @Override
    public void onOrientationChanged(int orientation) {
        if (mFdViewManager != null) {
            mFdViewManager.onOrientationChanged(orientation);
        }
    }

    @Override
    public void configuringSessionRequests(Map<RequestType, Builder> requestBuilders,
            CaptureType captureType) {
        Set<RequestType> keySet = requestBuilders.keySet();
        Iterator<RequestType> iterator = keySet.iterator();
        while (iterator.hasNext()) {
            RequestType requestType = iterator.next();
            CaptureRequest.Builder requestBuilder = requestBuilders.get(requestType);
            for (IDetectionCaptureObserver observer : mCaptureObservers) {
                observer.configuringRequests(requestBuilder, requestType);
            }
        }
    }

    @Override
    public void onCaptureStarted(CaptureRequest request, long timestamp, long frameNumber) {
        for (IDetectionCaptureObserver observer : mCaptureObservers) {
            observer.onCaptureStarted(request, timestamp, frameNumber);
        }
    }

    @Override
    public void onCaptureCompleted(CaptureRequest request, TotalCaptureResult result) {
        LogHelper.i(TAG, "onCaptureCompleted camera id:" + mSettingServant.getCameraId());
        for (IDetectionCaptureObserver observer : mCaptureObservers) {
            observer.onCaptureCompleted(request, result);
        }
    }

    @Override
    public RequestType getRepeatingRequestType() {
        return mModuleListener.getRepeatingRequestType();
    }

    @Override
    public void requestChangeCaptureRequest(boolean sync, RequestType requestType,
            CaptureType captureType) {
        if (mInitializedCameraId == null) {
            mModuleListener.requestChangeCaptureRequets(sync, requestType, captureType);
        } else if (SettingCtrl.BACK_CAMERA.equals(mInitializedCameraId)) {
            mModuleListener.requestChangeCaptureRequets(true, sync, requestType, captureType);
        } else if (SettingCtrl.FRONT_CAMERA.equals(mInitializedCameraId)) {
            mModuleListener.requestChangeCaptureRequets(false, sync, requestType, captureType);
        }
    }

    private void addCaredSettingChangedKeys(String key) {
        if (key != null && !mCaredSettingChangedKeys.contains(key)) {
            mCaredSettingChangedKeys.add(key);
        }
    }

    private void registerCaptureObserver(IDetectionCaptureObserver observer) {
        if (observer != null && !mCaptureObservers.contains(observer)) {
            mCaptureObservers.add(observer);
        }
    }

    private void unregisterCaptureObserver(IDetectionCaptureObserver observer) {
        if (observer != null && mCaptureObservers.contains(observer)) {
            mCaptureObservers.remove(observer);
        }
    }

    private IDetectionPresenter getPresenterInstance(String key) {
        if (key.equals(SettingKeys.KEY_ASD)) {
            mDetectionPresenter = mAsdPresenterImpl;
        } else if (key.equals(SettingKeys.KEY_CAMERA_FACE_DETECT)) {
            mDetectionPresenter = mFdPresenterImpl;
        }
        return mDetectionPresenter;
    }

    private void updateDetectionState(String key) {
        boolean isDetectionOpened = DETECTION_ON.equals(mSettingServant.getSettingValue(key));
        IDetectionPresenter presenter = getPresenterInstance(key);
        IDetectionCaptureObserver observer = presenter.getCaptureObserver();
        if (isDetectionOpened) {
            registerCaptureObserver(observer);
            presenter.startDetection();
        } else {
            presenter.stopDetection();
            unregisterCaptureObserver(observer);
        }
    }

    private void resumeDetectionState(String key) {
        boolean isDetectionOpened = DETECTION_ON.equals(mSettingServant.getSettingValue(key));
        IDetectionPresenter presenter = getPresenterInstance(key);
        IDetectionCaptureObserver observer = presenter.getCaptureObserver();
        if (isDetectionOpened) {
            registerCaptureObserver(observer);
            presenter.startDetection();
        }
    }
}
