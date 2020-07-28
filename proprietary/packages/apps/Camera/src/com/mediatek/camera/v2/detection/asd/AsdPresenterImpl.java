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
package com.mediatek.camera.v2.detection.asd;

import android.os.Handler;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;
import com.mediatek.camera.v2.detection.IDetectionCaptureObserver;
import com.mediatek.camera.v2.detection.IDetectionManager.IDetectionListener;
import com.mediatek.camera.v2.detection.IDetectionPresenter;
import com.mediatek.camera.v2.setting.ISettingServant;
import com.mediatek.camera.v2.setting.SettingConvertor;
import com.mediatek.camera.v2.util.SettingKeys;

/**
 * A presenter which controls auto scene device and view.
 *
 */
public class AsdPresenterImpl implements IAsdPresenterListener, IDetectionPresenter {
    private static final Tag TAG = new Tag(AsdPresenterImpl.class.getSimpleName());
    private IAsdView mAsdView;
    private AsdDeviceImpl mAsdDevice;
    private ISettingServant mSettingServant;
    private final Handler mHandler;
    private Runnable mViewUpdateRunnable;
    private boolean mIsAsdStarted = false;

    /**
     * AsdPresenterImpl constructor.
     * @param view View used to update UI.
     * @param additionListener Listener used to submit capture request.
     * @param settingServant The proxy of setting.
     */
    public AsdPresenterImpl(IAsdView view, IDetectionListener additionListener,
            ISettingServant settingServant) {
        mAsdView = view;
        mHandler = new Handler();
        mSettingServant = settingServant;
        mAsdDevice = new AsdDeviceImpl(additionListener);
        mAsdDevice.setListener(this);
    }

    @Override
    public void onSceneUpdate(final int mode) {
        // In the process of stopping asd, it can still receive detection callback from native.
        // So it posts the runnable of updating asd view to main thread to ensure it is
        // executed after stopping asd finished. In addition, the result of stopping asd is
        // posted to main thread in setting module. So {@link stopDetection} may be
        // executed in the behind of updating asd view. To prevent to update asd view after
        // asd stopped, it gets asd value from setting module to decide to update asd view
        // or not.
        mViewUpdateRunnable = new Runnable() {

            @Override
            public void run() {
                boolean isAsdOn = SettingConvertor.ASDDetectMode.ON.toString()
                        .equalsIgnoreCase(mSettingServant.getSettingValue(SettingKeys.KEY_ASD));
                LogHelper.d(TAG, "onSceneUpdate mIsAsdStarted is " + mIsAsdStarted + "" +
                        ", isAsdOn:" + isAsdOn);
                if (mAsdView != null && isAsdOn) {
                    mAsdView.updateAsdView(mode);
                }
            }
        };
        // Make sure that it updates scene mode in the main thread.
        mHandler.post(mViewUpdateRunnable);
    }

    @Override
    public IDetectionCaptureObserver getCaptureObserver() {
        return mAsdDevice.getCaptureObserver();
    }

    @Override
    public void startDetection() {
        LogHelper.i(TAG, "[startDetection], mIsAsdStarted:" + mIsAsdStarted);
        if (mIsAsdStarted) {
            LogHelper.i(TAG, "is AsdStarted, call twice,so return");
            return;
        }
        mAsdDevice.requestStartDetection();
        mIsAsdStarted = true;
    }

    @Override
    public void stopDetection() {
        LogHelper.i(TAG, "[stopAsd] mIsAsdStarted = " + mIsAsdStarted);
        if (!mIsAsdStarted) {
            LogHelper.i(TAG, "Asd not Started, why call stop,so return");
            return;
        }
        mAsdDevice.requestStopDetection();
        mHandler.removeCallbacks(mViewUpdateRunnable);
        mIsAsdStarted = false;
        mAsdView.hideAsdView();
    }
}
