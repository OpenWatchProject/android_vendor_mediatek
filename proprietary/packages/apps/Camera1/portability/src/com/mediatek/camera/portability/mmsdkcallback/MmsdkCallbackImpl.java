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

package com.mediatek.camera.portability.mmsdkcallback;

import android.content.Context;
import android.util.Log;
import android.view.Surface;

import com.mediatek.mmsdk.BaseParameters;
import com.mediatek.mmsdk.CameraEffectHalException;
import com.mediatek.mmsdk.callback.MmsdkCallbackClient;

import java.util.ArrayList;
import java.util.List;

/**
 * Mmsdk stereo data callback impl.
 */
public class MmsdkCallbackImpl {
    private static final String TAG = "CamAp_" + MmsdkCallbackImpl.class.getSimpleName();
    private static final String CLEAR_IMAGE = "ci";
    private static final String BOKEH_IMAGE = "bi";
    private static final String MTK_BOKEH_DEPTH = "mbd";
    private static final String MTK_DEBUG_BUFFER = "mdb";
    private static final String MTK_BOKEH_META = "mbm";
    private static final String MTK_DEPTH_WRAPPER = "mdw";
    private static final String LDC = "ldc";
    // Stereo Camera JPS
    private static final int MTK_CAMERA_MSG_EXT_DATA_JPS = 0x00000011;
    // Stereo Debug Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG = 0x00000012;
    // Stereo Camera Depth Map Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHMAP = 0x00000014;
    // Stereo Camera Clear Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE = 0x00000015;
    // Stereo Camera LDC Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC = 0x00000016;
    // Stereo Camera n3d Data
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D = 0x00000019;
    // Stereo Camera Depth wrapper
    private static final int MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHWRAPPER = 0x00000020;
    private static final int CAMERA_MSG_COMPRESSED_IMAGE = 0x100;
    private MmsdkCallbackClient mMmsdkCallbackClient;
    private BaseParameters mBaseParameters;
    private String mCallbackList;

    /**
     * Constructor of Mmsdk stereo data callback impl.
     * @param context context
     */
    public MmsdkCallbackImpl(Context context) {
        Log.d(TAG, "MmsdkCallbackImpl constructor" + this);
        mMmsdkCallbackClient = new MmsdkCallbackClient(context);
    }

    public boolean isMmsdkCallbackSupported() {
        return mMmsdkCallbackClient.isCallbackClientSupported();
    }

    /**
     * Start mmsdk callback.
     */
    public void start() {
        try {
            mMmsdkCallbackClient.start();
        } catch (CameraEffectHalException e) {
            Log.e(TAG, "start exception :" + e);
        }
    }

    /**
     * config surface.
     * @param surfaces surfaces
     */
    public void configSurfaces(List<Surface> surfaces, String callbackList) {
        try {
            mCallbackList = callbackList;
            mMmsdkCallbackClient.setOutputSurfaces(surfaces, getParameters());
        } catch (CameraEffectHalException e) {
            Log.e(TAG, "config surface exception :" + e);
        }
    }

    /**
     * stop mmsdk callback.
     */
    public void stop() {
        try {
            mMmsdkCallbackClient.stop();
        } catch (CameraEffectHalException e) {
            Log.e(TAG, "stop exception :" + e);
        }
    }

    private List<BaseParameters> getParameters() {
        List<BaseParameters> mSurfaceParameters = new ArrayList<BaseParameters>();
        if (mCallbackList.contains(CLEAR_IMAGE)) {
             // add clear image
            mBaseParameters = new BaseParameters();
            mBaseParameters.set(BaseParameters.KEY_CALLBACK_MSG_TYPE,
                    MTK_CAMERA_MSG_EXT_DATA_STEREO_CLEAR_IMAGE);
            mSurfaceParameters.add(mBaseParameters);
        }
        if (mCallbackList.contains(BOKEH_IMAGE)) {
            // add bokeh image
            mBaseParameters = new BaseParameters();
            mBaseParameters.set(BaseParameters.KEY_CALLBACK_MSG_TYPE, CAMERA_MSG_COMPRESSED_IMAGE);
            mSurfaceParameters.add(mBaseParameters);
        }
        if (mCallbackList.contains(MTK_BOKEH_DEPTH)) {
            // add depth wrapper
            mBaseParameters = new BaseParameters();
            mBaseParameters.set(BaseParameters.KEY_CALLBACK_MSG_TYPE,
                    MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHMAP);
            mSurfaceParameters.add(mBaseParameters);
        }
        if (mCallbackList.contains(MTK_DEBUG_BUFFER)) {
            // add n3d
            mBaseParameters = new BaseParameters();
            mBaseParameters.set(BaseParameters.KEY_CALLBACK_MSG_TYPE,
                    MTK_CAMERA_MSG_EXT_DATA_STEREO_N3D);
            mSurfaceParameters.add(mBaseParameters);
        }
        if (mCallbackList.contains(MTK_BOKEH_META)) {
            // add mask debug
            mBaseParameters = new BaseParameters();
            mBaseParameters.set(BaseParameters.KEY_CALLBACK_MSG_TYPE,
                    MTK_CAMERA_MSG_EXT_DATA_STEREO_DBG);
            mSurfaceParameters.add(mBaseParameters);
        }
        if (mCallbackList.contains(MTK_DEPTH_WRAPPER)) {
            // add depth wrapper
            mBaseParameters = new BaseParameters();
            mBaseParameters.set(BaseParameters.KEY_CALLBACK_MSG_TYPE,
                    MTK_CAMERA_MSG_EXT_DATA_STEREO_DEPTHWRAPPER);
            mSurfaceParameters.add(mBaseParameters);
        }
        if (mCallbackList.contains(LDC)) {
            // add ldc
            mBaseParameters = new BaseParameters();
            mBaseParameters.set(BaseParameters.KEY_CALLBACK_MSG_TYPE,
                    MTK_CAMERA_MSG_EXT_DATA_STEREO_LDC);
            mSurfaceParameters.add(mBaseParameters);
        }
        return mSurfaceParameters;
    }
}