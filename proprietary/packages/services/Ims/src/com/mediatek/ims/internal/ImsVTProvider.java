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

package com.mediatek.ims.internal;

import android.telephony.ims.ImsVideoCallProvider;

import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.telecom.VideoProfile.CameraCapabilities;
import android.telecom.Connection;
import android.telecom.VideoProfile;
import android.view.Surface;
import android.util.Log;

import android.telephony.CarrierConfigManager;
import android.content.Context;
import android.os.ServiceManager;

import java.util.Arrays;
import java.util.Collections;
import java.util.concurrent.ConcurrentHashMap;
import java.util.List;
import java.util.Set;
import java.lang.Integer;
import java.lang.Thread;
import java.lang.Integer;
import java.lang.Thread;

// for async the action to anthor thread
import com.android.internal.os.SomeArgs;
import com.android.internal.telephony.IccCardConstants;

import com.android.ims.ImsException;
import com.mediatek.ims.internal.ImsVTProviderUtil.Size;
import com.mediatek.ims.internal.ImsVTUsageManager;
import com.mediatek.ims.internal.ImsVTUsageManager.ImsVTUsage;

import com.mediatek.ims.internal.VTSource;
import android.hardware.camera2.CameraCharacteristics;

import com.mediatek.ims.ext.OpImsServiceCustomizationUtils;
import com.mediatek.ims.ImsCallSessionProxy;

// for External component
import com.mediatek.ims.plugin.ExtensionFactory;
import com.mediatek.ims.plugin.impl.ImsSelfActivatorBase;
import com.mediatek.ims.plugin.impl.ImsCallPluginBase;

import com.mediatek.ims.ImsCommonUtil;
import com.mediatek.ims.ImsService;

public class ImsVTProvider extends ImsVideoCallProvider implements
        VTSource.EventCallback {

    static {
        if (ImsVTProviderUtil.isVideoCallOnByPlatform()) {
            System.loadLibrary("mtk_vt_wrapper");
        }
    }

    /**
     * The interface used to notify video call related state/event
     */
    public interface VideoProviderStateListener {
        void onReceivePauseState(boolean isPause);
        void onReceiveWiFiUsage(long usage);
    }

    public static final int VT_PROVIDER_INVALIDE_ID                             = -10000;

    // ===================================================================================
    // state constant
    public static final int VTP_STATE_NORMAL                                       = 0;
    public static final int VTP_STATE_DATA_OFF                                     = 1;
    public static final int VTP_STATE_ROAMING                                      = 2;
    public static final int VTP_STATE_MA_CRASH                                     = 4;

    // ===================================================================================
    // operation constant
    //
    // AOSP original refrence from ImsVideoCallProvider class
    protected static final int MSG_SET_CALLBACK                                    = 1;
    protected static final int MSG_SET_CAMERA                                      = 2;
    protected static final int MSG_SET_PREVIEW_SURFACE                             = 3;
    protected static final int MSG_SET_DISPLAY_SURFACE                             = 4;
    protected static final int MSG_SET_DEVICE_ORIENTATION                          = 5;
    protected static final int MSG_SET_ZOOM                                        = 6;
    protected static final int MSG_SEND_SESSION_MODIFY_REQUEST                     = 7;
    protected static final int MSG_SEND_SESSION_MODIFY_RESPONSE                    = 8;
    protected static final int MSG_REQUEST_CAMERA_CAPABILITIES                     = 9;
    protected static final int MSG_REQUEST_CALL_DATA_USAGE                         = 10;
    protected static final int MSG_SET_PAUSE_IMAGE                                 = 11;
    //
    // MTK extend
    //
    protected static final int MSG_SET_UI_MODE                                     = 701;
    protected static final int MSG_SWITCH_FEATURE                                  = 702;
    protected static final int MSG_SWITCH_ROAMING                                  = 703;
    protected static final int MSG_RESET_WRAPPER                                   = 704;
    protected static final int MSG_UPDATE_PROFILE                                  = 705;
    protected static final int MSG_RECEIVE_CALL_SESSION_EVENT                      = 706;
    protected static final int MSG_UPDATE_CALL_RAT                                 = 707;
    // ===================================================================================

    // ===================================================================================
    // callback event constant
    public static final int SESSION_EVENT_RECEIVE_FIRSTFRAME                       = 1001;
    public static final int SESSION_EVENT_SNAPSHOT_DONE                            = 1002;
    public static final int SESSION_EVENT_RECORDER_EVENT_INFO_UNKNOWN              = 1003;
    public static final int SESSION_EVENT_RECORDER_EVENT_INFO_REACH_MAX_DURATION   = 1004;
    public static final int SESSION_EVENT_RECORDER_EVENT_INFO_REACH_MAX_FILESIZE   = 1005;
    public static final int SESSION_EVENT_RECORDER_EVENT_INFO_NO_I_FRAME           = 1006;
    public static final int SESSION_EVENT_RECORDER_EVENT_INFO_COMPLETE             = 1007;
    public static final int SESSION_EVENT_CALL_END                                 = 1008;
    public static final int SESSION_EVENT_CALL_ABNORMAL_END                        = 1009;
    public static final int SESSION_EVENT_START_COUNTER                            = 1010;
    public static final int SESSION_EVENT_PEER_CAMERA_OPEN                         = 1011;
    public static final int SESSION_EVENT_PEER_CAMERA_CLOSE                        = 1012;
    public static final int SESSION_EVENT_LOCAL_BW_READY_IND                       = 1013;

    public static final int SESSION_EVENT_RECV_SESSION_CONFIG_REQ                  = 4001;
    public static final int SESSION_EVENT_RECV_SESSION_CONFIG_RSP                  = 4002;
    public static final int SESSION_EVENT_HANDLE_CALL_SESSION_EVT                  = 4003;
    public static final int SESSION_EVENT_PEER_SIZE_CHANGED                        = 4004;
    public static final int SESSION_EVENT_LOCAL_SIZE_CHANGED                       = 4005;

    // it should not be used, after N MR1, we do this on java layer
    public static final int SESSION_EVENT_DATA_USAGE_CHANGED                       = 4006;
    public static final int SESSION_EVENT_CAM_CAP_CHANGED                          = 4007; // useless after HAL3

    public static final int SESSION_EVENT_BAD_DATA_BITRATE                         = 4008;
    public static final int SESSION_EVENT_DATA_BITRATE_RECOVER                     = 4009;
    public static final int SESSION_EVENT_RECV_ENHANCE_SESSION_IND                 = 4010;
    public static final int SESSION_EVENT_DATA_PATH_PAUSE                          = 4011;
    public static final int SESSION_EVENT_DATA_PATH_RESUME                         = 4012;
    public static final int SESSION_EVENT_DEFAULT_LOCAL_SIZE                       = 4013;
    public static final int SESSION_EVENT_GET_CAP                                  = 4014;
    public static final int SESSION_EVENT_LOCAL_BUFFER                             = 4015;
    public static final int SESSION_EVENT_UPLINK_STATE_CHANGE                      = 4016;
    public static final int SESSION_EVENT_RESTART_CAMERA                           = 4017;
    public static final int SESSION_EVENT_GET_SENSOR_INFO                          = 4018;
    public static final int SESSION_EVENT_GET_CAP_WITH_SIM                         = 4019;
    public static final int SESSION_EVENT_PACKET_LOSS_RATE                         = 4020;
    public static final int SESSION_EVENT_PACKET_LOSS_RATE_HIGH                    = 4021;
    public static final int SESSION_EVENT_PACKET_LOSS_RATE_MEDIUM                  = 4022;
    public static final int SESSION_EVENT_PACKET_LOSS_RATE_LOW                     = 4023;

    public static final int SESSION_EVENT_ERROR_SERVICE                            = 8001;
    public static final int SESSION_EVENT_ERROR_SERVER_DIED                        = 8002;
    public static final int SESSION_EVENT_ERROR_CAMERA_CRASHED                     = 8003;
    public static final int SESSION_EVENT_ERROR_CODEC                              = 8004;
    public static final int SESSION_EVENT_ERROR_REC                                = 8005;
    public static final int SESSION_EVENT_ERROR_CAMERA_SET_IGNORED                 = 8006;
    public static final int SESSION_EVENT_ERROR_BIND_PORT                          = 8007;

    public static final int SESSION_EVENT_WARNING_SERVICE_NOT_READY                = 9001;
    // ===================================================================================

    // ===================================================================================
    // session modify result constant
    //
    // For UA session modify result
    public static final int SESSION_MODIFY_OK                                      = 0;
    // Wrong video direction cause up/downgrade failed
    public static final int SESSION_MODIFY_WRONGVIDEODIR                           = 1;
    // Acct or session object broken
    public static final int SESSION_MODIFY_INTERNALERROR                           = 2;
    // Bandwidth modify require failed
    public static final int SESSION_MODIFY_RESULT_BW_MODIFYFAILED                  = 3;
    // Call session not in active state
    public static final int SESSION_MODIFY_NOACTIVESTATE                           = 4;
    // Local release:Ex.SRVCC,Hungup,call refresh timer timeout,no rpt packets
    public static final int SESSION_MODIFY_LOCALREL                                = 5;
    // The call at hold state
    public static final int SESSION_MODIFY_ISHOLD                                  = 6;
    // Acorrding the video action state, not need up/downgrade
    public static final int SESSION_MODIFY_NONEED                                  = 7;
    // Input parameter invalid
    public static final int SESSION_MODIFY_INVALIDPARA                             = 8;
    // SIPTX error,transaction timeout
    public static final int SESSION_MODIFY_REQTIMEOUT                              = 9;
    // Reject by remote
    public static final int SESSION_MODIFY_REJECTBYREMOTE                          = 10;
    // Canceled by user
    public static final int SESSION_MODIFY_CANCELED                                = 11;
    // Video action is ongoing
    public static final int SESSION_MODIFY_INVIDEOACTION                           = 12;
    // Is reinvite
    public static final int SESSION_MODIFY_ISREINVITE                              = 13;

    // ===================================================================================

    // ===================================================================================
    // session cancel result constant
    //
    public static final int SESSION_MODIFY_CANCEL_OK                               = 0;
    public static final int SESSION_MODIFY_CANCEL_FAILED_DOWNGRADE                 = 1;
    public static final int SESSION_MODIFY_CANCEL_FAILED_NORMAL                    = 2;
    public static final int SESSION_MODIFY_CANCEL_FAILED_DISABLE                   = 3;
    public static final int SESSION_MODIFY_CANCEL_FAILED_REMOTE                    = 4;
    public static final int SESSION_MODIFY_CANCEL_FAILED_INTERNAL                  = 5;
    public static final int SESSION_MODIFY_CANCEL_FAILED_LOCAL                     = 6;
    public static final int SESSION_MODIFY_CANCEL_FAILED_BW                        = 7;
    public static final int SESSION_MODIFY_CANCEL_FAILED_TIMEOUT                   = 8;
    // ===================================================================================

    public static final Uri REPLACE_PICTURE_PATH = Uri.parse("content://PATH");
    public static final String MTK_VILTE_ROTATE_DELAY = "persist.vendor.vt.rotate_delay";

    public static final int UPLINK_STATE_STOP_RECORDING                            = 0;
    public static final int UPLINK_STATE_START_RECORDING                           = 1;
    public static final int UPLINK_STATE_PAUSE_RECORDING                           = 2;
    public static final int UPLINK_STATE_RESUME_RECORDING                          = 3;
    public static final int UPLINK_STATE_STOP_RECORDING_PREVIEW                    = 4;

    public static final int MODE_PAUSE_BY_HOLD                                     = 1;
    public static final int MODE_PAUSE_BY_TURNOFFCAM                               = 2;

    public static final int POSTEVENT_IGNORE_ID                                    = -10;

    // ===================================================================================
    // session modify action constant when overlapping
    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_WAIT               = 0;
    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_REJECT             = 1;
    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_REJECT_PREVIOUS    = 2;
    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_SKIP               = 3;
    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_DO_IMMEDIATELY     = 4;

    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_APP                = 0;
    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_DATA_OFF           = 1;
    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_ROAMINGG           = 2;
    public static final int SESSION_MODIFICATION_OVERLAP_ACTION_MA_CRASH           = 3;

    // ===================================================================================
    // session modify indication type
    public static final int SESSION_INDICATION_CANCEL                              = 0;
    public static final int SESSION_INDICATION_EARLY_MEDIA                         = 1;

    public static final int EARLY_MEDIA_STOP                                       = 0;
    public static final int EARLY_MEDIA_START                                      = 1;

    // ===================================================================================

    static final String                         TAG = "ImsVT";

    protected int                               mId = 1;
    protected int                               mSimId = 0;
    public int                                  mMode = ImsVTProviderUtil.UI_MODE_FG;
    protected static int                        mDefaultId = VT_PROVIDER_INVALIDE_ID;
    public boolean                              mInitComplete = false;
    protected int                               mState;
    public static ImsVTProviderUtil             mVTProviderUtil = ImsVTProviderUtil.getInstance();

    // usage related variables
    public ImsVTUsageManager                    mUsager;

    // session modify related variables
    protected boolean                           mDuringSessionRequestOperation = false;
    protected boolean                           mDuringSessionRemoteRequestOperation = false;
    protected boolean                           mDuringEarlyMedia = false;
    protected boolean                           mIsDuringResetMode = false;
    protected Object                            mSessionOperationFlagLock = new Object();
    protected VideoProfile                      mLastRequestVideoProfile;

    // MA error handling related variables
    protected boolean                           mIsMaCrashed = false;

    // Roaming handling related variables
    protected boolean                           mIsRoaming = false;

    // Data off handling related variables
    protected boolean                           mIsDataOff = false;

    // Audio call handling related variables
    protected boolean                           mIsAudioCall = false;

    // camera related variables
    public VTSource                             mSource = null;
    public VTDummySource                        mDummySource = new VTDummySource();
    protected String                            mCameraId = null;
    public boolean                              mHasRequestCamCap = false;

    // Call state realted variables
    public  int                                 mPauseCount = 0;
    protected int                               mUplinkState = UPLINK_STATE_STOP_RECORDING;
    protected Size                              mPreviewSize = new Size(320, 240);

    protected HandlerThread                     mProviderHandlerThread;
    protected final Handler                     mProviderHandler;

    protected VideoProfile                      mCurrentProfile = null;

    // Call when mode is UI_MODE_DESTROY
    private boolean                             mIsDestroying = false;

    // handler for MTK only event
    protected final Set<VideoProviderStateListener> mListeners = Collections.newSetFromMap(
            new ConcurrentHashMap<VideoProviderStateListener, Boolean>(8, 0.9f, 1));

    /*start*/
    protected class ConnectionEx {
        public class VideoProvider {
            private static final int SESSION_MODIFY_MTK_BASE = 200;
            public static final int SESSION_MODIFY_CANCEL_UPGRADE_FAIL = SESSION_MODIFY_MTK_BASE;
            public static final int SESSION_MODIFY_CANCEL_UPGRADE_FAIL_AUTO_DOWNGRADE =
                    SESSION_MODIFY_MTK_BASE+1;
            public static final int SESSION_MODIFY_CANCEL_UPGRADE_FAIL_REMOTE_REJECT_UPGRADE =
                    SESSION_MODIFY_MTK_BASE+2;
        }
    }
    /*end*/

    protected int                               mOrientation = 0;
    protected Runnable                          mOrientationRunnable = null;

    protected int                               mCallRat = ImsVTProviderUtil.CALL_RAT_LTE;

    public ImsVTProvider() {
        super();
        Log.d(TAG, "New ImsVTProvider without id");
        mId = VT_PROVIDER_INVALIDE_ID;
        mInitComplete = false;

        mState = VTP_STATE_NORMAL;

        mUsager = OpImsServiceCustomizationUtils.getOpFactory(mVTProviderUtil.mContext).makeImsVTUsageManager();

        mProviderHandlerThread = new HandlerThread("ProviderHandlerThread");
        mProviderHandlerThread.start();

        mProviderHandler = new Handler(mProviderHandlerThread.getLooper()) {

            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_SET_CAMERA:
                        setCameraInternal((String) msg.obj);
                        break;
                    case MSG_SET_PREVIEW_SURFACE:
                        setPreviewSurfaceInternal((Surface) msg.obj);
                        break;
                    case MSG_SET_DISPLAY_SURFACE:
                        setDisplaySurfaceInternal((Surface) msg.obj);
                        break;
                    case MSG_SET_DEVICE_ORIENTATION:
                        setDeviceOrientationInternal(msg.arg1);
                        break;
                    case MSG_SET_ZOOM:
                        setZoomInternal((Float) msg.obj);
                        break;
                    case MSG_SEND_SESSION_MODIFY_REQUEST: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            VideoProfile fromProfile = (VideoProfile) args.arg1;
                            VideoProfile toProfile = (VideoProfile) args.arg2;

                            sendSessionModifyRequestInternal(fromProfile, toProfile);
                        } finally {
                            args.recycle();
                        }
                        break;
                    }
                    case MSG_SEND_SESSION_MODIFY_RESPONSE:
                        sendSessionModifyResponseInternal((VideoProfile) msg.obj);
                        break;
                    case MSG_REQUEST_CAMERA_CAPABILITIES:
                        requestCameraCapabilitiesInternal();
                        break;
                    case MSG_REQUEST_CALL_DATA_USAGE:
                        requestCallDataUsageInternal();
                        break;
                    case MSG_SET_PAUSE_IMAGE:
                        setPauseImageInternal((Uri) msg.obj);
                        break;
                    case MSG_SET_UI_MODE:
                        setUIModeInternal((int) msg.obj, true);
                        break;
                    case MSG_SWITCH_FEATURE: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            int feature = (int) args.arg1;
                            boolean on = (boolean) args.arg2;

                            switchFeatureInternal(feature, on);
                        } finally {
                            args.recycle();
                        }

                        break;
                    }
                    case MSG_SWITCH_ROAMING: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            boolean isRoaming = (boolean) args.arg1;

                            switchRoamingInternal(isRoaming);
                        } finally {
                            args.recycle();
                        }

                        break;
                    }
                    case MSG_RESET_WRAPPER:
                        resetWrapperInternal();
                        break;
                    case MSG_UPDATE_PROFILE: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            int state = (int) args.arg1;

                            updateProfileInternal(state);
                        } finally {
                            args.recycle();
                        }
                        break;
                    }
                    case MSG_RECEIVE_CALL_SESSION_EVENT: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            int event = (int) args.arg1;

                            receiveCallSessionEventInternal(event);
                        } finally {
                            args.recycle();
                        }
                        break;
                    }
                    case MSG_UPDATE_CALL_RAT: {
                        SomeArgs args = (SomeArgs) msg.obj;
                        try {
                            int callRat = (int) args.arg1;
                            updateCallRatInternal(callRat);
                        } finally {
                            args.recycle();
                        }
                    }
                    default:
                        break;
                }
            }
        };

        mOrientationRunnable = new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "setDeviceOrientation, apply orientation:" + mOrientation);
                mSource.setDeviceOrientation(mOrientation);
                nSetDeviceOrientation(mId, mOrientation);
            }
        };
    }

    public void setId(int id) {
        Log.d(TAG, "setId id = " + id + ", mId = " + mId);

        mId = id;
        mUsager.setId(id);

        if (mDefaultId == VT_PROVIDER_INVALIDE_ID) {
            mDefaultId = mId;
        }
    }

    public int getId() {
        return mId;
    }

    public void setSimId(int simid) {
        Log.d(TAG, "setSimId mSimId = " + simid);

        mSimId = simid;
        mUsager.setSimId(simid);
    }

    public int getSimId() {
        return mSimId;
    }

    public int getState() {
        return mState;
    }

    public void setState(int state) {
        mState |= state;
        Log.d(TAG, "setState mState = " + mState);
    }

    public boolean hasState(int state, int vtp_state) {
        return (state & vtp_state) == vtp_state;
    }

    public void resetState(int state) {

        Log.d(TAG, "resetState old mState = " + mState);

        mState = mState & (~state);

        Log.d(TAG, "resetState new mState = " + mState);
    }

    public void setDuringSessionRequest(boolean b) {

        synchronized (mSessionOperationFlagLock) {
            mDuringSessionRequestOperation = b;
            Log.w(TAG, "setDuringSessionRequest : " + mDuringSessionRequestOperation);
        }
    }

    public boolean getDuringSessionRequest() {
        return mDuringSessionRequestOperation;
    }

    public void setDuringSessionRemoteRequest(boolean b) {

        synchronized (mSessionOperationFlagLock) {
            mDuringSessionRemoteRequestOperation = b;
            Log.w(TAG, "setDuringSessionRemoteRequest : " + mDuringSessionRemoteRequestOperation);
        }
    }

    public boolean getDuringSessionRemoteRequest() {
        return mDuringSessionRemoteRequestOperation;
    }

    // set if during MA crash process
    public void setMaCrash(boolean b) {
        mIsMaCrashed = b;
        Log.w(TAG, "setMaCrash : " + mIsMaCrashed);
    }

    // get if during MA crash process
    public boolean getMaCrash() {
        return mIsMaCrashed;
    }

    // set if during Data off process
    public void setDataOff(boolean b) {
        mIsDataOff = b;
        Log.w(TAG, "setDataOff : " + mIsDataOff);
    }

    // get if during Data off process
    public boolean getDataOff() {
        return mIsDataOff;
    }

    // set if during roaming process
    public void setRoaming(boolean b) {
        mIsRoaming = b;
        Log.w(TAG, "setRoaming : " + mIsRoaming);
    }

    // get if during roaming process
    public boolean getRoaming() {
        return mIsRoaming;
    }

    // set if become audio call
    public void setIsAudioCall(boolean result) {
        mIsAudioCall = result;
        Log.w(TAG, "setIsAudioCall : " + mIsAudioCall);
    }

    // set if become audio call
    public boolean getIsAudioCall() {
        return mIsAudioCall;
    }

    public void handleMaErrorProcess() {

        Log.w(TAG, "[ID=" + mId + "] [handleMaErrorProcess] start");

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            Log.w(TAG, "[ID=" + mId + "] [handleMaErrorProcess] call end");
            return;
        }

        // actualy, we don't need the original state for request
        VideoProfile currentProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL, VideoProfile.QUALITY_MEDIUM);
        VideoProfile reqestProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY, VideoProfile.QUALITY_MEDIUM);

        int decision = doSessionModifyDecision(SESSION_MODIFICATION_OVERLAP_ACTION_MA_CRASH, currentProfile, reqestProfile);

        if (SESSION_MODIFICATION_OVERLAP_ACTION_REJECT == decision) {

            Log.e(TAG, "[handleMaErrorProcess] : should not in this case");

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_REJECT_PREVIOUS == decision) {

            //response audio_only if MA crash
            VideoProfile audioResponseProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY,
                    VideoProfile.QUALITY_MEDIUM);
            nResponseLocalConfig(mId,
                    mVTProviderUtil.packFromVdoProfile(audioResponseProfile));
            setDuringSessionRemoteRequest(false);

            setMaCrash(true);
            mLastRequestVideoProfile = reqestProfile;
            nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(reqestProfile));

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_SKIP == decision) {

            Log.d(TAG, "[ID=" + mId + "] [sendSessionModifyRequestByImsInternal] skip");

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_WAIT == decision) {

            waitSessionOperationComplete();

            setMaCrash(true);
            mLastRequestVideoProfile = reqestProfile;
            nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(reqestProfile));
        }

        Log.d(TAG, "[ID=" + mId + "] [handleMaErrorProcess] Finish");

    }

    public void waitSessionOperationComplete() {
        while (true == mDuringSessionRequestOperation || true == mDuringSessionRemoteRequestOperation) {
            try {
                Log.w(TAG, "Wait for Session operation complete!");
                Thread.sleep(500);
            } catch(InterruptedException ex) {
            }
        }
    }

    public void quitThread() {
        mProviderHandlerThread.quitSafely();
    }

    public VTSource getSource() {
        if (null != mSource) {
            return mSource;
        } else {
            Log.w(TAG, "Get dummy vtsource");
            return mDummySource;
        }
    }

    public void waitInitComplete() {
        while (false == mInitComplete && mMode != ImsVTProviderUtil.UI_MODE_DESTROY) {
            try {
                Log.w(TAG, "Wait for initialization complete!");
                Thread.sleep(500);
            } catch(InterruptedException ex) {
            }
        }
    }

    protected static void updateDefaultId() {

        if (!mVTProviderUtil.recordContain(mDefaultId)) {
            if (mVTProviderUtil.recordSize() != 0) {
                mDefaultId = mVTProviderUtil.recordPopId();
                return;
            }
            mDefaultId = VT_PROVIDER_INVALIDE_ID;
        }
        return;
    }

    public static native int nInitRefVTP();
    public static native int nInitialization(int id, int sim_id);
    public static native int nFinalization(int id);
    public static native int nSetCamera(int id, int cam);
    public static native int nSetPreviewSurface(int id, Surface surface);
    public static native int nSetDisplaySurface(int id, Surface surface);
    public static native int nSetCameraParameters(int id, VTSource.Resolution[] cams_resolution);
    public static native int nSetCameraParametersOnly(VTSource.Resolution[] cams_resolution);
    public static native int nSetCameraParametersWithSim(int id, int major_sim_id, VTSource.Resolution[] cams_resolution);
    public static native int nSetDeviceOrientation(int id, int rotation);
    public static native int nSetUIMode(int id, int mode);
    public static native int nRequestPeerConfig(int id, String config);
    public static native int nResponseLocalConfig(int id, String config);
    public static native int nSnapshot(int id, int type, String uri);
    public static native int nStartRecording(int id, int type, String url, long maxSize);
    public static native int nStopRecording(int id);
    public static native int nSwitchFeature(int id, int feature, int on);
    public static native int nUpdateNetworkTable(boolean is_add, int network_id, String if_name);

    public void onSetCamera(String cameraId) {
        mProviderHandler.obtainMessage(MSG_SET_CAMERA, cameraId).sendToTarget();
    }

    public void onSetPreviewSurface(Surface surface) {
        mProviderHandler.obtainMessage(MSG_SET_PREVIEW_SURFACE, surface).sendToTarget();
    }

    public void onSetDisplaySurface(Surface surface) {
        mProviderHandler.obtainMessage(MSG_SET_DISPLAY_SURFACE, surface).sendToTarget();
    }

    public void onSetDeviceOrientation(int rotation) {
        mProviderHandler.obtainMessage(MSG_SET_DEVICE_ORIENTATION, rotation, 0).sendToTarget();
    }

    public void onSetZoom(float value) {
        mProviderHandler.obtainMessage(MSG_SET_ZOOM, value).sendToTarget();
    }

    public void onSendSessionModifyRequest(VideoProfile fromProfile, VideoProfile toProfile) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = fromProfile;
        args.arg2 = toProfile;
        mProviderHandler.obtainMessage(MSG_SEND_SESSION_MODIFY_REQUEST, args).sendToTarget();
    }

    public void onSendSessionModifyResponse(VideoProfile responseProfile) {
        mProviderHandler.obtainMessage(
                MSG_SEND_SESSION_MODIFY_RESPONSE, responseProfile).sendToTarget();
    }

    public void onRequestCameraCapabilities() {
        mProviderHandler.obtainMessage(MSG_REQUEST_CAMERA_CAPABILITIES).sendToTarget();
    }

    public void onRequestCallDataUsage() {
        mProviderHandler.obtainMessage(MSG_REQUEST_CALL_DATA_USAGE).sendToTarget();
    }

    public void onSetPauseImage(Uri uri) {
        mProviderHandler.obtainMessage(MSG_SET_PAUSE_IMAGE, uri).sendToTarget();
    }

    public void onSetUIMode(int mode) {
        mProviderHandler.obtainMessage(MSG_SET_UI_MODE, mode).sendToTarget();
        if (mode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            mIsDestroying = true;
        }
    }

    public void onSwitchFeature(int feature, boolean on) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = feature;
        args.arg2 = on;
        mProviderHandler.obtainMessage(MSG_SWITCH_FEATURE, args).sendToTarget();
    }

    public void onSwitchRoaming(boolean isRoaming) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = isRoaming;
        mProviderHandler.obtainMessage(MSG_SWITCH_ROAMING, args).sendToTarget();
    }

    public void onResetWrapper() {
        mProviderHandler.obtainMessage(MSG_RESET_WRAPPER).sendToTarget();
    }

    public void onUpdateProfile(int state) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = state;
        mProviderHandler.obtainMessage(MSG_UPDATE_PROFILE, args).sendToTarget();
    }

    public void onUpdateCallRat(int rat) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = rat;
        mProviderHandler.obtainMessage(MSG_UPDATE_CALL_RAT, args).sendToTarget();
    }

    public void onReceiveCallSessionEvent(int event) {
        SomeArgs args = SomeArgs.obtain();
        args.arg1 = event;
        mProviderHandler.obtainMessage(MSG_RECEIVE_CALL_SESSION_EVENT, args).sendToTarget();
    }

    public void setCameraInternal(String cameraId) {

        Log.d(TAG, "[ID=" + mId + "] [onSetCamera] id : " + cameraId);

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            return;
        }

        //Do not switch camera when UI mode is destroying, so finalization can be handled soon.
        if (mIsDestroying && cameraId != null) {
            Log.d(TAG, "[ID=" + mId + "] [onSetCamera] UI mode is destroying");
            return;
        }

        int count = 0;

        if (cameraId == null) {
            mHasRequestCamCap = false;
        }

        if (mMode == ImsVTProviderUtil.UI_MODE_BG) {
            Log.w(TAG, "[ID=" + mId + "] [onSetCamera] onSetCamera, not set camera when in BG, only save id=" + cameraId);
        } else {

            if (cameraId != null) {

                while (true) {

                    if (mUplinkState == UPLINK_STATE_PAUSE_RECORDING) {

                        if (count < 10) {
                            Log.w(TAG, "[ID=" + mId + "] [onSetCamera] onSetCamera, Call hold or held, wait and retry");
                        } else {
                            // Set camera when call Hold or Held may cause error in future
                            // because it re-trigger MA connect camera and MA will fall in a wrong state.
                            // So we just skip when call hold & held
                            Log.w(TAG, "[ID=" + mId + "] [onSetCamera] onSetCamera, Call hold or held, ignore setCamera");
                            handleCallSessionEvent(SESSION_EVENT_ERROR_CAMERA_SET_IGNORED);
                            return;
                        }

                    } else if ((mUplinkState == UPLINK_STATE_STOP_RECORDING ||
                            mUplinkState == UPLINK_STATE_STOP_RECORDING_PREVIEW) && count > 0) {
                        Log.w(TAG, "[ID=" + mId + "] [onSetCamera] onSetCamera, recording stopped");
                        return;

                    } else {
                        //Release other call's camera to prevent surface is used by other camera
                        mVTProviderUtil.updateCameraUsage(mId);

                        //VTSource will auto restart preview and recording
                        mSource.open(cameraId);
                        mSource.showMe();
                        nSetCamera(mId, Integer.valueOf(cameraId));
                        break;
                    }

                    try {
                        Thread.sleep(200);
                    } catch(InterruptedException ex) {
                    }
                    count++;
                }

            } else {
                mSource.hideMe();
                mSource.close();
            }
        }
        mCameraId = cameraId;

        Log.d(TAG, "[ID=" + mId + "] [onSetCamera] Finish");

    }

    public void setPreviewSurfaceInternal(Surface surface) {

        Log.d(TAG, "[ID=" + mId + "] [onSetPreviewSurface] Start, surface: " + surface);

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            Log.d(TAG, "[ID=" + mId + "] [onSetPreviewSurface] call end");
            return;
        }

        mSource.setPreviewSurface(surface);

        nSetPreviewSurface(mId, surface);

        Log.d(TAG, "[ID=" + mId + "] [onSetPreviewSurface] Finish");

    }

    public void setDisplaySurfaceInternal(Surface surface) {

        Log.d(TAG, "[ID=" + mId + "] [onSetDisplaySurface] Start, surface: " + surface);

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            Log.d(TAG, "[ID=" + mId + "] [onSetDisplaySurface] call end");
            return;
        }

        nSetDisplaySurface(mId, surface);

        Log.d(TAG, "[ID=" + mId + "] [onSetDisplaySurface] Finish");

    }

    public void setDeviceOrientationInternal(int rotation) {

        Log.d(TAG, "[ID=" + mId + "] [onSetDeviceOrientation] Start, rotation: " + rotation);

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            Log.d(TAG, "[ID=" + mId + "] [onSetDeviceOrientation] call end");
            return;
        }

        waitInitComplete();

        if (mOrientation != rotation) {

            Log.d(TAG, "[ID=" + mId + "] [onSetDeviceOrientation] device orientation change from "
                    + mOrientation + " to " + rotation);
            mOrientation = rotation;

            mProviderHandler.removeCallbacks(mOrientationRunnable);
            // Wait 500 ms to prevent rotation frequently
            mProviderHandler.postDelayed(mOrientationRunnable,
                SystemProperties.getInt(MTK_VILTE_ROTATE_DELAY, 500));
        }

        Log.d(TAG, "[ID=" + mId + "] [onSetDeviceOrientation] Finish");

    }

    public void setZoomInternal(float value) {

        Log.d(TAG, "[ID=" + mId + "] [onSetZoom] Start, value: " + value);

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            return;
        }

        mSource.setZoom(value);

        Log.d(TAG, "[ID=" + mId + "] [onSetZoom] Finish");

    }

    public void sendSessionModifyRequestInternal(VideoProfile fromProfile, VideoProfile toProfile) {

        if (mVTProviderUtil.getImsExtCallUtil().isImsFwkRequest(toProfile.getVideoState())) {

            Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] requst from IMS FWK, swith handle function");

            sendSessionModifyRequestByImsInternal(fromProfile, toProfile);
            return;
        }

        Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] Start, fromProfile:" +
                fromProfile.toString() + ", toProfile:" + toProfile.toString());

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] call end");
            return;
        }

        int mode = getSessionModifyAction(fromProfile, toProfile);
        if (mode != ImsVTProviderUtil.UI_MODE_UNCHANGED) {
            sendFgBgSessionModifyRequestInternal(fromProfile, toProfile, mode, true);
            return;
        }

        if (VideoProfile.isVideo(toProfile.getVideoState())) {
            if (hasState(mState, VTP_STATE_MA_CRASH)) {

                //if MA CRASH, reject upgrade directly
                rejectSessionModifyInternal(
                        Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL,
                        toProfile,
                        fromProfile);
                Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] Reject it by have MA CRASH:" + mState);
                return;

            } else if (!mVTProviderUtil.isVideoCallOn(mSimId) ||
                    hasState(mState, VTP_STATE_DATA_OFF) ||
                    hasState(mState, VTP_STATE_ROAMING)) {

                //vilte off or data unvailable, if not wifi call, reject upgrade directly
                if (!(mCallRat == ImsVTProviderUtil.CALL_RAT_WIFI && mVTProviderUtil.isViWifiOn(mSimId))) {
                    rejectSessionModifyInternal(
                            Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL,
                            toProfile,
                            fromProfile);
                    Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] Reject it by state:" + mState);
                    return;
                }
            }
        }

        int decision = doSessionModifyDecision(SESSION_MODIFICATION_OVERLAP_ACTION_APP, fromProfile, toProfile);

        if (SESSION_MODIFICATION_OVERLAP_ACTION_REJECT == decision) {

            rejectSessionModifyInternal(
                    Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL,
                    toProfile,
                    fromProfile);

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_DO_IMMEDIATELY == decision) {

            Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] send request immediately");
            setDuringSessionRequest(true);
            mLastRequestVideoProfile = toProfile;
            nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(toProfile));

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_WAIT == decision) {

            waitSessionOperationComplete();

            setDuringSessionRequest(true);
            mLastRequestVideoProfile = toProfile;
            nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(toProfile));

        } else {

            Log.e(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] should not in this case");

            rejectSessionModifyInternal(
                    Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL,
                    toProfile,
                    fromProfile);

        }

        Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] Finish");
    }

    protected void sendFgBgSessionModifyRequestInternal(VideoProfile fromProfile, VideoProfile toProfile, int mode, boolean needNotify) {
        // onSetUIMode(mode);
        setUIModeInternal(mode, needNotify);
    }

    public void sendSessionModifyRequestByImsInternal(VideoProfile fromProfile, VideoProfile toProfile) {

        Log.d(TAG, "[ID=" + mId + "] [sendSessionModifyRequestByImsInternal] Start, fromProfile:" +
                fromProfile.toString() + ", toProfile:" + toProfile.toString());

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            Log.d(TAG, "[ID=" + mId + "] [sendSessionModifyRequestByImsInternal] call end");
            return;
        }

        int newState = toProfile.getVideoState();
        int realNewState =  mVTProviderUtil.getImsExtCallUtil().getRealRequest(newState);

        VideoProfile newToProfile = new VideoProfile(
                                        realNewState,
                                        toProfile.getQuality());

        int decision = doSessionModifyDecision(SESSION_MODIFICATION_OVERLAP_ACTION_DATA_OFF, fromProfile, newToProfile);

        if (SESSION_MODIFICATION_OVERLAP_ACTION_REJECT_PREVIOUS == decision) {

            VideoProfile audioResponseProfile =
                new VideoProfile(VideoProfile.STATE_AUDIO_ONLY, VideoProfile.QUALITY_MEDIUM);
            nResponseLocalConfig(mId, mVTProviderUtil.packFromVdoProfile(audioResponseProfile));
            setDuringSessionRemoteRequest(false);

            setDataOff(true);
            mLastRequestVideoProfile = newToProfile;
            nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(newToProfile));

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_SKIP == decision) {

            Log.d(TAG, "[ID=" + mId + "] [sendSessionModifyRequestByImsInternal] skip");

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_WAIT == decision) {

            waitSessionOperationComplete();

            setDataOff(true);
            mLastRequestVideoProfile = newToProfile;
            nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(newToProfile));

        } else {

            Log.e(TAG, "[ID=" + mId + "] [sendSessionModifyRequestByImsInternal] should not in this case");

        }

        Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyRequest] Finish");
    }

    public void sendSessionModifyResponseInternal(VideoProfile responseProfile) {

        Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyResponse] Start, responseProfile:" +
                responseProfile.toString());

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyResponse] call end");
            return;
        }

        if (getDuringSessionRemoteRequest()) {
            nResponseLocalConfig(mId, mVTProviderUtil.packFromVdoProfile(responseProfile));
            if(VideoProfile.isAudioOnly(responseProfile.getVideoState())) {
                setIsAudioCall(true);
            } else {
                setIsAudioCall(false);
            }

        } else {
            // In some case, ex: received remote request then do data off
            // The session modify response has been already send to remote because off data off downgrade.
            // Here we need to ignore session modify response again.
            Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyResponse] Already response, skip it");
        }

        setDuringSessionRemoteRequest(false);

        Log.d(TAG, "[ID=" + mId + "] [onSendSessionModifyResponse] Finish");
    }

    public void receiveSessionModifyResponseInternal(int status, VideoProfile requestedProfile, VideoProfile responseProfile) {

        receiveSessionModifyResponse(status, requestedProfile, responseProfile);

        if (getMaCrash()) {
            setState(VTP_STATE_MA_CRASH);
            setMaCrash(false);
        }

        if (getDataOff()) {
            int responseState = responseProfile.getVideoState();
            if ((VideoProfile.isPaused(responseState) || VideoProfile.isAudioOnly(responseState)) &&
                !mVTProviderUtil.isVideoCallOn(mSimId)) {
                setState(VTP_STATE_DATA_OFF);
            } else {
                resetState(VTP_STATE_DATA_OFF);
            }
            setDataOff(false);
        }

        if (getRoaming()) {
            setState(VTP_STATE_ROAMING);
            setRoaming(false);
        }

        if (getDuringSessionRequest()) {
            setDuringSessionRequest(false);
        }

        // If downgrade complete, set Audio flag.
        if (status == Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS) {
            if (VideoProfile.isAudioOnly(responseProfile.getVideoState())) {
                setIsAudioCall(true);
            } else {
                setIsAudioCall(false);
            }
        }
    }

    public void rejectSessionModifyInternal(int status, VideoProfile requestedProfile, VideoProfile responseProfile) {

        receiveSessionModifyResponse(status, requestedProfile, responseProfile);
    }

    public void requestCameraCapabilitiesInternal() {

        Log.d(TAG, "[ID=" + mId + "] [onRequestCameraCapabilities] Start");

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            return;
        }

        if (mCameraId == null) {
            Log.w(TAG, "onRequestCameraCapabilities: not set camera yet");
            return;
        }

        float zoom_max = 1.0f;
        boolean zoom_support = false;

        CameraCharacteristics camera_cs = mSource.getCameraCharacteristics();
        if (null == camera_cs) {
            Log.w(TAG, "onRequestCameraCapabilities: camera_cs null! Use default value.");
        } else {
            zoom_max = camera_cs.get(CameraCharacteristics.SCALER_AVAILABLE_MAX_DIGITAL_ZOOM);
        }
        zoom_support = (zoom_max > 1)? true:false;

        Log.d(TAG, "[ID=" + mId + "] [onRequestCameraCapabilities] Width: " + mPreviewSize.width +
                " Height: " + mPreviewSize.height + " zoom_support: " + zoom_support +
                " zoom_max: " + zoom_max);

        CameraCapabilities camCap =
            new CameraCapabilities(mPreviewSize.width, mPreviewSize.height, zoom_support, zoom_max);

        changeCameraCapabilities(camCap);

        mHasRequestCamCap = true;

        Log.d(TAG, "[ID=" + mId + "] [onRequestCameraCapabilities] Finish");
    }

    public void requestCallDataUsageInternal() {

        Log.d(TAG, "[ID=" + mId + "] [onRequestCallDataUsage] Start");

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            return;
        }

        ImsVTUsage usage = mUsager.requestCallDataUsage();

        if (usage != null) {
            changeCallDataUsage(usage.getLteUsage(ImsVTUsage.STATE_TXRX));
            notifyWifiUsageChange(usage.getWifiUsage(ImsVTUsage.STATE_TXRX));
        }
    }

    public void setPauseImageInternal(Uri uri) {

        Log.d(TAG, "[ID=" + mId + "] [onSetPauseImage] Start, uri: " + uri);

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            return;
        }

        mSource.setReplacePicture(uri);

        Log.d(TAG, "[ID=" + mId + "] [onSetPauseImage] Finish");

    }

    // MTK proprietary interface
    public void setUIModeInternal(int mode, boolean needNotify) {

        Log.d(TAG, "[ID=" + mId + "] [onSetUIMode] Start, mode: " + mode);

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            Log.d(TAG, "[ID=" + mId + "] [onSetUIMode] call end");
            return;
        }

        switch (mode) {
            case ImsVTProviderUtil.UI_MODE_FG:
                mMode = mode;
                if (needNotify) {
                    notifyVideoPauseStateChange();
                }

                if (mCameraId != null) {
                     //Release other call's camera to prevent surface is used by other camera
                     mVTProviderUtil.updateCameraUsage(mId);

                     //VTSource will auto restart preview and recording
                     mSource.open(mCameraId);
                     mSource.showMe();
                     nSetCamera(mId, Integer.valueOf(mCameraId));
                } else {
                    mSource.hideMe();
                    mSource.close();
                }

                nSetUIMode(mId, mode);
                break;

            case ImsVTProviderUtil.UI_MODE_BG:
                if (mIsDuringResetMode) {
                    Log.d(TAG, "[ID=" + mId + "] [onSetUIMode] reset mode (voice call) should not recv BG, skip");
                    break;
                }

                mMode = mode;
                if (needNotify) {
                    notifyVideoPauseStateChange();
                }

                //Release camera when in BG
                mSource.hideMe();
                mSource.close();

                nSetUIMode(mId, mode);
                break;

            case ImsVTProviderUtil.UI_MODE_RESET:

                if (mMode != ImsVTProviderUtil.UI_MODE_DESTROY) {
                    mMode = ImsVTProviderUtil.UI_MODE_FG;
                }

                mIsDuringResetMode = true;
                Thread resetModeRecoverThread = new Thread(new Runnable() {
                    public synchronized void run() {
                        Log.d(TAG, "[ID=" + mId + "] [onSetUIMode] resetModeRecoverThread start");

                        //Add delay to prevent IMS FWK reset mode earlier than InCallUI go to BG.
                        try {
                            Thread.sleep(600);
                            mIsDuringResetMode = false;
                        } catch(InterruptedException ex) {
                        }
                        Log.d(TAG, "[ID=" + mId + "] [onSetUIMode] resetModeRecoverThread finish");
                    }
                });
                resetModeRecoverThread.start();

                if (needNotify) {
                    notifyVideoPauseStateChange();
                }
                break;

            case ImsVTProviderUtil.UI_MODE_DESTROY:

                // In some case that start MO call and hang up immediately
                // It will always block because requestCallDataUsageInternal() wait for call
                // initialized, but the call will never initialized.
                if (true == mInitComplete) {
                    requestCallDataUsageInternal();
                }

                mMode = mode;
                mIsDestroying = false;
                nFinalization(mId);

                quitThread();
                break;

            default:
                break;
        }

        Log.d(TAG, "[ID=" + mId + "] [onSetUIMode] Finish");
    }

    public void switchFeatureInternal(int feature, boolean on) {

        Log.d(TAG, "[ID=" + mId + "] [onSwitchFeature] Start, feature: " + feature + ", on: " + on);

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            return;
        }

        if (on) {
            nSwitchFeature(mId, feature, 1);
            resetState(VTP_STATE_DATA_OFF);

        } else {
            nSwitchFeature(mId, feature, 0);
        }

        Log.d(TAG, "[ID=" + mId + "] [onSwitchFeature] Finish");
    }

    public void switchRoamingInternal(boolean isRoaming) {

        Log.d(TAG, "[ID=" + mId + "] [onSwitchRoaming] Start, phoneId: " + mSimId +
                ", on: " + isRoaming);

        waitInitComplete();

        if (!isRoaming) {

            resetState(VTP_STATE_ROAMING);

        } else {

            if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
                Log.d(TAG, "[ID=" + mId + "] [onSwitchRoaming] call end");
                return;
            }

            // actualy, we don't need the original state for reqest
            VideoProfile currentProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL, VideoProfile.QUALITY_MEDIUM);
            VideoProfile reqestProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY, VideoProfile.QUALITY_MEDIUM);

            int decision = doSessionModifyDecision(SESSION_MODIFICATION_OVERLAP_ACTION_ROAMINGG, currentProfile, reqestProfile);

            if (SESSION_MODIFICATION_OVERLAP_ACTION_REJECT == decision) {

                Log.e(TAG, "onSwitchRoaming() : should not in this case");
                return;

            } else if (SESSION_MODIFICATION_OVERLAP_ACTION_REJECT_PREVIOUS == decision) {

                receiveSessionModifyResponseInternal(
                        Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL,
                        mLastRequestVideoProfile,
                        mLastRequestVideoProfile);

                setRoaming(true);
                mLastRequestVideoProfile = reqestProfile;
                nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(reqestProfile));

                return;

            } else if (SESSION_MODIFICATION_OVERLAP_ACTION_SKIP == decision) {

                return;

            } else if (SESSION_MODIFICATION_OVERLAP_ACTION_WAIT == decision) {

                waitSessionOperationComplete();

                setRoaming(true);
                mLastRequestVideoProfile = reqestProfile;
                nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(reqestProfile));

                setRoaming(true);
            }
        }

        Log.d(TAG, "[ID=" + mId + "] [onSwitchRoaming] Finish");

    }

    public void resetWrapperInternal() {

        Log.d(TAG, "[ID=" + mId + "] [onResetWrapper] Start");

        waitInitComplete();

        if (mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
            return;
        }

        if (ImsVTProviderUtil.UI_MODE_BG == mMode) {
            setUIModeInternal(ImsVTProviderUtil.UI_MODE_FG, true);
        }

        Log.d(TAG, "[ID=" + mId + "] [onResetWrapper] Finish");

    }

    public void updateProfileInternal(int state) {

        Log.d(TAG, "[ID=" + mId + "] [updateCurrentProfile] Start, state=" + state);

        VideoProfile newVideoprofile = new VideoProfile(state, VideoProfile.QUALITY_MEDIUM);
        mCurrentProfile = newVideoprofile;

        Log.d(TAG, "[ID=" + mId + "] [updateCurrentProfile] Finish");

    }

    public void receiveCallSessionEventInternal(int event) {

        Log.d(TAG, "[ID=" + mId + "] [receiveCallSessionEventInternal] Start, event=" + event);

        switch (event) {
            case ImsCallSessionProxy.CALL_INFO_MSG_TYPE_ACTIVE:
                notifyResume();
                break;

            default:
                break;
        }

        Log.d(TAG, "[ID=" + mId + "] [receiveCallSessionEventInternal] Finish");

    }

    public void notifyResume() {
    }

    public void updateCallRatInternal(int callRat) {
        Log.d(TAG, "[ID=" + mId + "] [updateCallRatInternal] Start, callRat=" + callRat);
        mCallRat = callRat;
        Log.d(TAG, "[ID=" + mId + "] [updateCallRatInternal] Finish");
    }

    // when two session modify is overlapping, we need to decide how to handle the second one
    // we follow the decision table
    //
    //   +-------------------+------------------+------------------+--------------------+-----------+-----------+-----------+
    //   |            \Druing| App              | App              | App                | Data off  | Roaming   | MA error  |
    //   |new operation\     | (Upgrade cancel) |(Local)           | (Remote)           |           |           |           |
    //   +-------------------+------------------+------------------+--------------------+-----------+-----------+-----------+
    //   |App                | NA               | Do it now        | NA                 | Reject it | Reject it | Reject it |
    //   |(Upgrade cancel)   |                  |                  |                    |           |           |           |
    //   +-------------------+------------------+------------------+--------------------+-----------+-----------+-----------+
    //   |App                | Reject it        | Reject it        | Reject it          | Reject it | Reject it | Reject it |
    //   |(Local not cancel) |                  |                  |                    |           |           |           |
    //   +-------------------+------------------+------------------+--------------------+-----------+-----------+-----------+
    //   |App                | Reject it        | Reject it        | Reject it          | Reject it | Reject it | Reject it |
    //   |(Remote not cancel)|                  |                  |                    |           |           |           |
    //   +-------------------+------------------+------------------+--------------------+-----------+-----------+-----------+
    //   |Data off           | Wait it complete | Wait it complete | Reject the 1st one | skip      | skip      | skip      |
    //   +-------------------+------------------+------------------+--------------------+-----------+-----------+-----------+
    //   |Roaming            | Wait it complete | Wait it complete | Reject the 1st one | skip      | skip      | skip      |
    //   +-------------------+------------------+------------------+--------------------+-----------+-----------+-----------+
    //   |MA/Codec error     | Wait it complete | Wait it complete | Reject the 1st one | skip      | skip      | skip      |
    //   +-------------------+------------------+------------------+--------------------+-----------+-----------+-----------+
    public int doSessionModifyDecision(int new_action, VideoProfile fromProfile, VideoProfile toProfile) {

        if (SESSION_MODIFICATION_OVERLAP_ACTION_APP == new_action) {

            if (getRoaming()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : from APP, reject due to during roaming");

                return SESSION_MODIFICATION_OVERLAP_ACTION_REJECT;

            } else if (getMaCrash()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : from APP, reject due to during MA crash");

                return SESSION_MODIFICATION_OVERLAP_ACTION_REJECT;

            } else if (getDataOff()) {

                if (!VideoProfile.isAudioOnly(toProfile.getVideoState())) {

                    Log.i(TAG, "doSessionModifyDecision : new_action : from APP, reject due to during data off and not downgrade");

                    return SESSION_MODIFICATION_OVERLAP_ACTION_REJECT;
                }

            } else if (isDuringSessionModify()) {

                boolean isCancelUpgrade = false;

                if (toProfile.getVideoState() == mVTProviderUtil.getImsExtCallUtil().getUpgradeCancelFlag()) {

                    Log.i(TAG, "doSessionModifyDecision : new_action : from APP, isCancelUpgrade is true");
                    isCancelUpgrade = true;
                }

                if (isCancelUpgrade) {

                    if(getDuringSessionRemoteRequest()) {
                        Log.i(TAG, "doSessionModifyDecision : new_action : from APP, reject due to during App action");
                        return SESSION_MODIFICATION_OVERLAP_ACTION_REJECT;

                    } else {
                        Log.i(TAG, "doSessionModifyDecision : new_action : from APP, send cancel request");
                        return SESSION_MODIFICATION_OVERLAP_ACTION_DO_IMMEDIATELY;
                    }

                }  else {
                    // we reject the second request when during another request
                    // but for cancel request, it is Ok to overlapping
                    Log.i(TAG, "doSessionModifyDecision : new_action : from APP, reject due to during App action");
                    return SESSION_MODIFICATION_OVERLAP_ACTION_REJECT;
                }
            }

            Log.i(TAG, "doSessionModifyDecision : new_action : from APP, wait");

            return SESSION_MODIFICATION_OVERLAP_ACTION_WAIT;

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_DATA_OFF == new_action) {

            if (getRoaming()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : Data off, skip due to during roaming");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (getMaCrash()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : Data off, skip due to during MA crash");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (getDataOff()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : Data off, reject due to data off");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (isDuringSessionModify()) {

                if (getDuringSessionRequest()) {

                    Log.i(TAG, "doSessionModifyDecision : new_action : Data off, wait for App action");

                    return SESSION_MODIFICATION_OVERLAP_ACTION_WAIT;

                } else if (getDuringSessionRemoteRequest()) {

                    Log.i(TAG, "doSessionModifyDecision : new_action : Data off, reject previous due to data off downgrade");

                    return SESSION_MODIFICATION_OVERLAP_ACTION_REJECT_PREVIOUS;
                }
            }

            Log.i(TAG, "doSessionModifyDecision : new_action : Data off, wait");

            return SESSION_MODIFICATION_OVERLAP_ACTION_WAIT;

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_ROAMINGG == new_action) {

            if (getRoaming()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : Roaming, skip due to during roaming");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (getMaCrash()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : Roaming, skip due to during MA crash");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (getDataOff()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : Roaming, skip due to during data off");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (isDuringSessionModify()) {

                if (getDuringSessionRequest()) {

                    Log.i(TAG, "doSessionModifyDecision : new_action : Roaming, wait for App action");

                    return SESSION_MODIFICATION_OVERLAP_ACTION_WAIT;

                } else if (getDuringSessionRemoteRequest()) {

                    Log.i(TAG, "doSessionModifyDecision : new_action : Roaming, reject previous due to roaming downgrade");

                    return SESSION_MODIFICATION_OVERLAP_ACTION_REJECT_PREVIOUS;
                }
            }

            Log.i(TAG, "doSessionModifyDecision : new_action : Roaming, wait");

            return SESSION_MODIFICATION_OVERLAP_ACTION_WAIT;

        } else if (SESSION_MODIFICATION_OVERLAP_ACTION_MA_CRASH == new_action) {

            if (getRoaming()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : MA crash, skip due to during roaming");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (getMaCrash()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : MA crash, skip due to during MA crash");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (getDataOff()) {

                Log.i(TAG, "doSessionModifyDecision : new_action : MA crash, skip due to during data off");

                return SESSION_MODIFICATION_OVERLAP_ACTION_SKIP;

            } else if (isDuringSessionModify()) {

                if (getDuringSessionRequest()) {

                    Log.i(TAG, "doSessionModifyDecision : new_action : MA crash, wait for App action");

                    return SESSION_MODIFICATION_OVERLAP_ACTION_WAIT;

                } else if (getDuringSessionRemoteRequest()) {

                    Log.i(TAG, "doSessionModifyDecision : new_action : MA crash, reject previous due to MA crash");

                    return SESSION_MODIFICATION_OVERLAP_ACTION_REJECT_PREVIOUS;
                }
            }

            Log.i(TAG, "doSessionModifyDecision : new_action : MA crash, wait");

            return SESSION_MODIFICATION_OVERLAP_ACTION_WAIT;

        }

        Log.i(TAG, "doSessionModifyDecision : new_action : wait");

        return SESSION_MODIFICATION_OVERLAP_ACTION_WAIT;
    }

    // implement VTSource.EventCallback
    public void onError() {

        Log.d(TAG, "[ID=" + mId + "] [onError] Start");

        handleMaErrorProcess();
        handleCallSessionEvent(SESSION_EVENT_ERROR_CAMERA_CRASHED);

        Log.d(TAG, "[ID=" + mId + "] [onError] Finish");

    }

    // implement VTSource.EventCallback
    public void onOpenSuccess() {
        Log.d(TAG, "[ID=" + mId + "] [onOpenSuccess] Start");

        handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_CAMERA_READY);

        Log.d(TAG, "[ID=" + mId + "] [onOpenSuccess] Finish");
    }

    // implement VTSource.EventCallback
    public void onOpenFail() {
        Log.d(TAG, "[ID=" + mId + "] [onOpenFail] Start");

        handleCallSessionEvent(Connection.VideoProvider.SESSION_EVENT_CAMERA_FAILURE);

        Log.d(TAG, "[ID=" + mId + "] [onOpenFail] Finish");
    }

    public void addVideoProviderStateListener(VideoProviderStateListener listener) {
        mListeners.add(listener);
    }

    public void removeVideoProviderStateListener(VideoProviderStateListener listener) {
        mListeners.remove(listener);
    }

    public void notifyVideoPauseStateChange() {

        Log.d(TAG, "[ID=" + mId + "] [notifyVideoPauseStateChange] Start");

        boolean isVideoStatePause = false;

        if (ImsVTProviderUtil.UI_MODE_FG == mMode) {
            isVideoStatePause = false;
        } else if (ImsVTProviderUtil.UI_MODE_BG == mMode){
            isVideoStatePause = true;
        }

        for (VideoProviderStateListener listener : mListeners) {
            if (listener != null) {
                listener.onReceivePauseState(isVideoStatePause);
                Log.d(TAG, "[ID=" + mId + "] [notifyVideoPauseStateChange] isVideoStatePause: " + isVideoStatePause);
            }
        }

        Log.d(TAG, "[ID=" + mId + "] [notifyVideoPauseStateChange] Finish");

    }

    public void notifyWifiUsageChange(long usage) {

        Log.d(TAG, "[ID=" + mId + "] [notifyWifiUsageChange] Start, usage : " + usage);

        for (VideoProviderStateListener listener : mListeners) {
            if (listener != null) {
                listener.onReceiveWiFiUsage(usage);
            }
        }

        Log.d(TAG, "[ID=" + mId + "] [notifyWifiUsageChange] Finish");

    }

    protected boolean isDuringSessionModify() {
        if (getDuringSessionRequest() || getDuringSessionRemoteRequest()) {
            return true;
        } else {
            return false;
        }
    }

    protected boolean isDuringNotAppDowngrade() {
        if (getMaCrash() ||
            (getDataOff() && VideoProfile.isAudioOnly(mLastRequestVideoProfile.getVideoState())) ||
            getRoaming()) {
            return true;
        } else {
            return false;
        }
    }

    protected int getSessionModifyAction(VideoProfile fromProfile, VideoProfile toProfile) {
        int result = ImsVTProviderUtil.UI_MODE_UNCHANGED;
        // For pause case, it mean App switch FG & BG
        // We don't go the normal session modify flow
        // We only need to focus on "to" video profile changed
        if (mMode != ImsVTProviderUtil.UI_MODE_BG && VideoProfile.isPaused(toProfile.getVideoState())) {
            result = ImsVTProviderUtil.UI_MODE_BG;
        } else if (mMode == ImsVTProviderUtil.UI_MODE_BG && !VideoProfile.isPaused(toProfile.getVideoState())) {
            result = ImsVTProviderUtil.UI_MODE_FG;
        }
        return result;
    }

    protected void reSendLastSessionModify() {

        Log.d(TAG, "[ID=" + mId + "] [reSendLastSessionModify] Profile:" + mLastRequestVideoProfile.toString());

        try {
            Thread.sleep(500);
        } catch(InterruptedException ex) {
        }

        nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(mLastRequestVideoProfile));
    }

    public static void postEventFromNative(
            int msg,
            int id,
            int arg1,
            int arg2,
            int arg3,
            Object obj1,
            Object obj2,
            Object obj3) {

        ImsVTProvider vp = mVTProviderUtil.recordGet(id);

        if (null == vp &&
            msg != SESSION_EVENT_ERROR_SERVER_DIED &&
            msg != SESSION_EVENT_GET_CAP &&
            msg != SESSION_EVENT_GET_CAP_WITH_SIM &&
            id != POSTEVENT_IGNORE_ID) {
            Log.e(TAG, "Error: post event to Call is already release or has happen error before!");
            return;
        }

        Log.i(TAG, "[ID=" + id + "] [postEventFromNative]: " + msg);
        switch (msg) {
            case SESSION_EVENT_RECEIVE_FIRSTFRAME:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_RECEIVE_FIRSTFRAME");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_SNAPSHOT_DONE:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_SNAPSHOT_DONE");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_RECORDER_EVENT_INFO_UNKNOWN:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_RECORDER_EVENT_INFO_UNKNOWN");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_RECORDER_EVENT_INFO_REACH_MAX_DURATION:
                Log.d(TAG, "postEventFromNative : msg = " +
                     "SESSION_EVENT_RECORDER_EVENT_INFO_REACH_MAX_DURATION");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_RECORDER_EVENT_INFO_REACH_MAX_FILESIZE:
                Log.d(TAG, "postEventFromNative : msg = " +
                     "SESSION_EVENT_RECORDER_EVENT_INFO_REACH_MAX_FILESIZE");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_RECORDER_EVENT_INFO_NO_I_FRAME:
                Log.d(TAG, "postEventFromNative : msg = " +
                    "SESSION_EVENT_RECORDER_EVENT_INFO_NO_I_FRAME");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_RECORDER_EVENT_INFO_COMPLETE:
                Log.d(TAG, "postEventFromNative : msg = " +
                    "SESSION_EVENT_RECORDER_EVENT_INFO_COMPLETE");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_CALL_END:
            case SESSION_EVENT_CALL_ABNORMAL_END:
                Log.d(TAG, "postEventFromNative : msg = " +
                      "SESSION_EVENT_CALL_END / SESSION_EVENT_CALL_ABNORMAL_END");

                // Call release to leave VTSource handler thread
                vp.getSource().release();

                mVTProviderUtil.recordRemove(id);
                updateDefaultId();

                vp.handleCallSessionEvent(msg);

                vp.mProviderHandlerThread.quitSafely();

                vp.mMode = ImsVTProviderUtil.UI_MODE_DESTROY;
                break;

            case SESSION_EVENT_START_COUNTER:
                Log.d(TAG, "postEventFromNative : msg = MSG_START_COUNTER");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_PEER_CAMERA_OPEN:
                Log.d(TAG, "postEventFromNative : msg = MSG_PEER_CAMERA_OPEN");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_PEER_CAMERA_CLOSE:
                Log.d(TAG, "postEventFromNative : msg = MSG_PEER_CAMERA_CLOSE");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_LOCAL_BW_READY_IND:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_LOCAL_BW_READY_IND");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_RECV_SESSION_CONFIG_REQ: {
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_RECV_SESSION_CONFIG_REQ");

                // actualy, we don't need the original state for request
                VideoProfile currentProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL, VideoProfile.QUALITY_MEDIUM);
                VideoProfile requestProfile = mVTProviderUtil.unPackToVdoProfile((String) obj1);

                if (VideoProfile.isVideo(requestProfile.getVideoState())) {
                    if (vp.hasState(vp.mState, VTP_STATE_MA_CRASH)) {
                        //MA crash, reject upgrade request directly
                        VideoProfile audioProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY, VideoProfile.QUALITY_MEDIUM);
                        vp.mLastRequestVideoProfile = audioProfile;

                        vp.onSendSessionModifyResponse(vp.mLastRequestVideoProfile);
                        Log.d(TAG, "[ID=" + vp.getId() + "] [onSendSessionModifyResponse] Reject it by have MA CRASH:" + vp.mState);
                        return;

                    } else if (!mVTProviderUtil.isVideoCallOn(vp.mSimId) ||
                            vp.hasState(vp.mState, VTP_STATE_DATA_OFF) ||
                            vp.hasState(vp.mState, VTP_STATE_ROAMING)) {

                        //vilte off or data unvailable, if not wifi call, reject upgrade directly
                        if (!(vp.mCallRat == ImsVTProviderUtil.CALL_RAT_WIFI && mVTProviderUtil.isViWifiOn(vp.mSimId))) {
                            VideoProfile audioProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY, VideoProfile.QUALITY_MEDIUM);
                            vp.mLastRequestVideoProfile = audioProfile;

                            vp.onSendSessionModifyResponse(vp.mLastRequestVideoProfile);
                            Log.d(TAG, "[ID=" + vp.getId() + "] [onSendSessionModifyResponse] Reject it by state:" + vp.mState);
                            return;
                        }
                    }
                }

                int decision = vp.doSessionModifyDecision(SESSION_MODIFICATION_OVERLAP_ACTION_APP, currentProfile, requestProfile);

                if (SESSION_MODIFICATION_OVERLAP_ACTION_REJECT == decision) {

                    vp.setDuringSessionRemoteRequest(true);
                    vp.onSendSessionModifyResponse(vp.mLastRequestVideoProfile);

                } else if (SESSION_MODIFICATION_OVERLAP_ACTION_WAIT == decision) {

                    vp.waitSessionOperationComplete();

                    // for the not upgrade case, we rsp directly
                    if (!VideoProfile.isBidirectional(requestProfile.getVideoState())) {

                        Log.d(TAG, "Do onSendSessionModifyResponse directly for not upgrade case");

                        vp.setDuringSessionRemoteRequest(true);
                        vp.onSendSessionModifyResponse(requestProfile);

                    } else {
                        vp.setDuringSessionRemoteRequest(true);
                        vp.receiveSessionModifyRequest(requestProfile);
                    }

                } else {

                    Log.e(TAG, "[ID=" + vp.getId() + "] [onSendSessionModifyResponse] should not in this case");
                    vp.setDuringSessionRemoteRequest(true);
                    vp.onSendSessionModifyResponse(requestProfile);

                }
                break;
            }

            case SESSION_EVENT_RECV_ENHANCE_SESSION_IND:
                Log.d(TAG, "postEventFromNative : msg = " + "SESSION_EVENT_RECV_ENHANCE_SESSION_IND");

                if (SESSION_INDICATION_CANCEL == arg1) {
                    Log.d(TAG, "SESSION_INDICATION_CANCEL");
                    vp.setDuringSessionRemoteRequest(false);

                } else if (SESSION_INDICATION_EARLY_MEDIA == arg1) {
                    Log.d(TAG, "SESSION_INDICATION_EARLY_MEDIA, early media=" + arg2);

                    if(arg2 == EARLY_MEDIA_STOP) {
                        vp.mDuringEarlyMedia = false;

                    } else if (arg2 == EARLY_MEDIA_START) {
                        vp.mDuringEarlyMedia = true;
                    }

                    // Here can comment out the "return" to received session modify indication
                    // for customization
                    return;
                }

                VideoProfile IndicationProfile = mVTProviderUtil.unPackToVdoProfile((String) obj1);
                vp.receiveSessionModifyRequest(IndicationProfile);

                break;

            case SESSION_EVENT_RECV_SESSION_CONFIG_RSP: {
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_RECV_SESSION_CONFIG_RSP");

                if (vp.mMode == ImsVTProviderUtil.UI_MODE_DESTROY) {
                    Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_RECV_SESSION_CONFIG_RSP (call end)");
                    return;
                }

                VideoProfile responseProfile = mVTProviderUtil.unPackToVdoProfile((String) obj2);
                int state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL;

                // upgate cancel case will handle in separate enum and flow
                if (responseProfile.getVideoState() == mVTProviderUtil.getImsExtCallUtil().getUpgradeCancelFlag()) {

                    state = ConnectionEx.VideoProvider.SESSION_MODIFY_CANCEL_UPGRADE_FAIL;

                    switch(arg1) {
                        case SESSION_MODIFY_CANCEL_OK:
                            state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
                            break;
                        case SESSION_MODIFY_CANCEL_FAILED_DOWNGRADE:
                            state = ConnectionEx.VideoProvider.
                                    SESSION_MODIFY_CANCEL_UPGRADE_FAIL_AUTO_DOWNGRADE;
                            break;
                        case SESSION_MODIFY_CANCEL_FAILED_REMOTE:
                            state = ConnectionEx.VideoProvider.
                                    SESSION_MODIFY_CANCEL_UPGRADE_FAIL_REMOTE_REJECT_UPGRADE;
                            break;
                        default:
                            state = ConnectionEx.VideoProvider.
                                    SESSION_MODIFY_CANCEL_UPGRADE_FAIL;
                            break;
                    }

                } else {

                    switch (arg1) {
                        case SESSION_MODIFY_OK:

                            if (VideoProfile.isVideo(responseProfile.getVideoState())) {
                                if (vp.hasState(vp.mState, VTP_STATE_MA_CRASH)) {

                                    VideoProfile audioProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY, VideoProfile.QUALITY_MEDIUM);
                                    vp.mLastRequestVideoProfile = audioProfile;
                                    vp.reSendLastSessionModify();

                                    Log.d(TAG, "[ID=" + vp.getId() + "] [reSendLastSessionModify] by have MA CRASH:" + vp.mState);
                                    return;
                                } else if (!mVTProviderUtil.isVideoCallOn(vp.mSimId) ||
                                        vp.hasState(vp.mState, VTP_STATE_DATA_OFF) ||
                                        vp.hasState(vp.mState, VTP_STATE_ROAMING)) {

                                    //vilte off or data unvailable, if not wifi call, downgrade to voice call.
                                    if (!(vp.mCallRat == ImsVTProviderUtil.CALL_RAT_WIFI && mVTProviderUtil.isViWifiOn(vp.mSimId))) {
                                        VideoProfile audioProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY, VideoProfile.QUALITY_MEDIUM);
                                        vp.mLastRequestVideoProfile = audioProfile;
                                        vp.reSendLastSessionModify();

                                        Log.d(TAG, "[ID=" + vp.getId() + "] [reSendLastSessionModify] not viwifi call, downgrade by state:" + vp.mState);
                                        return;
                                    }
                                }
                            }

                            state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
                            break;

                        case SESSION_MODIFY_NOACTIVESTATE: {
                            // If during early medai and session modify fail with call not active, just response success to InCallUI
                            if (vp.mDuringEarlyMedia) {
                                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_RECV_SESSION_CONFIG_RSP (during early media)");

                                state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
                                vp.receiveSessionModifyResponseInternal(
                                        state,
                                        mVTProviderUtil.unPackToVdoProfile((String) obj1),
                                        mVTProviderUtil.unPackToVdoProfile((String) obj1));
                                return;

                            } else {
                                // Need resend session modify when call is not active until call become active
                                vp.reSendLastSessionModify();
                                return;
                            }
                        }

                        case SESSION_MODIFY_NONEED:
                            state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS;
                            break;
                        case SESSION_MODIFY_INVALIDPARA:
                            state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_INVALID;
                            break;
                        case SESSION_MODIFY_REQTIMEOUT:
                            state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_TIMED_OUT;
                            break;
                        case SESSION_MODIFY_REJECTBYREMOTE: {

                            //Need retry downgrade when MA crash or data off
                            if (vp.isDuringNotAppDowngrade()) {
                                vp.reSendLastSessionModify();
                                return;

                            } else {
                                state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_REJECTED_BY_REMOTE;
                            }
                            break;
                        }

                        case SESSION_MODIFY_ISREINVITE:
                        case SESSION_MODIFY_INVIDEOACTION: {
                            vp.reSendLastSessionModify();
                            return;
                        }

                        default:
                            state = Connection.VideoProvider.SESSION_MODIFY_REQUEST_FAIL;
                            break;
                    }
                }

                vp.receiveSessionModifyResponseInternal(
                        state,
                        mVTProviderUtil.unPackToVdoProfile((String) obj1),
                        mVTProviderUtil.unPackToVdoProfile((String) obj2));

                break;
            }

            case SESSION_EVENT_HANDLE_CALL_SESSION_EVT:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_HANDLE_CALL_SESSION_EVT");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_PEER_SIZE_CHANGED:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_PEER_SIZE_CHANGED");
                vp.changePeerDimensions(arg1, arg2);
                break;

            case SESSION_EVENT_LOCAL_SIZE_CHANGED: {
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_LOCAL_SIZE_CHANGED");

                if(vp.mPreviewSize.width == arg1 && vp.mPreviewSize.height == arg2) {
                    Log.d(TAG, "local size no change => Do not notify!");
                    break;
                }

                vp.mPreviewSize.width = arg1;
                vp.mPreviewSize.height = arg2;
                Log.d(TAG, "Update preview size, w=" + vp.mPreviewSize.width + ", h=" + vp.mPreviewSize.height);

                // Need update to App after first request
                if(true == vp.mHasRequestCamCap) {
                    vp.onRequestCameraCapabilities();
                } else {
                    Log.d(TAG, "Not request yet, just only update default w/h");
                }
                break;
            }

            case SESSION_EVENT_DATA_USAGE_CHANGED:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_DATA_USAGE_CHANGED");
                break;

            case SESSION_EVENT_PACKET_LOSS_RATE:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_PACKET_LOSS_RATE");

                //Low packet loss rate (Green bar): 0%~5%
                //Medium packet loss rate (Yellow bar): 5%~10%
                //High packet loss rate (Red bar): 10%~25% upper
                int lossRate = arg1;
                Log.d(TAG, "Packet loss rate = " + lossRate + "%");

                if (0 <= lossRate && lossRate <= 5){
                    Log.d(TAG, "Packet loss rate low, notify: " + SESSION_EVENT_PACKET_LOSS_RATE_LOW);
                    vp.handleCallSessionEvent(SESSION_EVENT_PACKET_LOSS_RATE_LOW);

                } else if (5 < lossRate && lossRate <= 10) {
                    Log.d(TAG, "Packet loss rate medium, notify: " + SESSION_EVENT_PACKET_LOSS_RATE_MEDIUM);
                    vp.handleCallSessionEvent(SESSION_EVENT_PACKET_LOSS_RATE_MEDIUM);

                } else if (10 < lossRate) {
                    Log.d(TAG, "Packet loss rate high, notify: " + SESSION_EVENT_PACKET_LOSS_RATE_HIGH);
                    vp.handleCallSessionEvent(SESSION_EVENT_PACKET_LOSS_RATE_HIGH);

                } else {
                    Log.w(TAG, "Packet loss rate incorrect");
                }

                // native module will post value when RTP Info is ON
                // the value mean the data loss rate and will multiply by -1 (i.e. arg1 < 0 )
                // we pass to App only when the switch is turn on, or just skip
                boolean RTP_DBG_ON = SystemProperties.get("persist.vendor.vt.RTPInfo").equals("1") ? true : false;
                if (RTP_DBG_ON) {
                    vp.changeCallDataUsage(-1 * lossRate);
                }

                break;

            case SESSION_EVENT_BAD_DATA_BITRATE:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_BAD_DATA_BITRATE");

                if (!ImsVTProviderUtil.sIsNoCameraMode) {
                    vp.handleCallSessionEvent(msg);
                }
                break;

            case SESSION_EVENT_GET_CAP_WITH_SIM: {
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_GET_CAP_WITH_SIM");

                VTSource.Resolution [] cams_info;
                if (ImsVTProviderUtil.sIsNoCameraMode) {
                    cams_info = VTDummySource.getAllCameraResolutions();
                } else {
                    cams_info = VTSource.getAllCameraResolutions();
                }
                if(null == cams_info) {
                    Log.e(TAG, "Error: sensor resolution = NULL");
                }

                int major_sim_id = 0;
                int ImsCount = 1;

                ImsCount = ImsService.getInstance(mVTProviderUtil.mContext).getModemMultiImsCount();
                // Not MIMS case, set major sim id to get correct codec capability
                if (ImsCount <= 1) {

                    while (mVTProviderUtil.getImsExtCallUtil().isCapabilitySwitching()) {
                        try {
                            Thread.sleep(200);
                        } catch(InterruptedException ex) {
                        }
                    }

                    major_sim_id = ImsCommonUtil.getMainCapabilityPhoneId();;

                } else {
                    major_sim_id = id;
                }

                mVTProviderUtil.waitSimStateStable(major_sim_id);

                if (mVTProviderUtil.getSimCardState(major_sim_id) !=
                        TelephonyManager.SIM_STATE_PRESENT) {

                    // In some case SIM state has not been updated yet.
                    // Need to check iccid property additionally to ensure SIM is absent
                    String iccid_prop = ImsVTProviderUtil.PROPERTY_RIL_ICCID_SIM + (major_sim_id + 1);
                    String iccid = SystemProperties.get(iccid_prop, "");
                    int count = 0;

                    while (iccid.equals("") && count < 50) {
                        try {
                            Thread.sleep(200);
                        } catch(InterruptedException ex) {
                        }
                        iccid = SystemProperties.get(iccid_prop, "");
                        count++;
                    }

                    if (iccid.equals("N/A") || count >= 50) {
                        Log.d(TAG, "SIM state ABSENT");
                        major_sim_id = ImsVTProviderUtil.VT_SIM_ID_ABSENT;
                    } else {
                        Log.d(TAG, "SIM state READY");
                    }
                }

                nSetCameraParametersWithSim(id, major_sim_id, cams_info);
                break;
            }

            case SESSION_EVENT_GET_CAP: {
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_GET_CAP");

                VTSource.Resolution [] cams_info;
                if (ImsVTProviderUtil.sIsNoCameraMode) {
                    cams_info = VTDummySource.getAllCameraResolutions();
                } else {
                    cams_info = VTSource.getAllCameraResolutions();
                }
                if(null == cams_info) {
                    Log.e(TAG, "Error: sensor resolution = NULL");
                }

                nSetCameraParameters(id, cams_info);
                break;
            }

            case SESSION_EVENT_LOCAL_BUFFER:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_LOCAL_BUFFER");

                vp.getSource().stopRecording();
                vp.getSource().setRecordSurface((Surface)obj3);
                vp.getSource().startRecording();
                vp.mPauseCount = 0;
                break;

            case SESSION_EVENT_UPLINK_STATE_CHANGE:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_UPLINK_STATE_CHANGE");
                vp.mUplinkState = arg1;

                switch (arg1) {
                    case UPLINK_STATE_STOP_RECORDING:
                        if (vp.getSource() != null) {
                            vp.getSource().stopRecording();
                            vp.getSource().setRecordSurface(null);
                        }
                        break;

                    case UPLINK_STATE_START_RECORDING:
                        break;

                    case UPLINK_STATE_PAUSE_RECORDING:
                        if (arg2 == MODE_PAUSE_BY_HOLD) {
                            if (vp.mPauseCount == 0) {
                                vp.getSource().stopRecording();
                            }

                            vp.mPauseCount++;
                        }
                        break;

                    case UPLINK_STATE_RESUME_RECORDING:
                        if (arg2 == MODE_PAUSE_BY_HOLD) {
                            vp.mPauseCount--;

                            //Start recording after all hold/held resume
                            if (vp.mPauseCount == 0) {
                                vp.getSource().startRecording();
                            }
                        }
                        break;

                    case UPLINK_STATE_STOP_RECORDING_PREVIEW:
                        if (vp.getSource() != null) {
                            vp.getSource().stopRecordingAndPreview();
                        }
                        break;

                    default:
                        break;

                }
                break;

            case SESSION_EVENT_DEFAULT_LOCAL_SIZE:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_DEFAULT_LOCAL_SIZE, W=" + arg1 + ", H=" + arg2);
                vp.mPreviewSize.width = arg1;
                vp.mPreviewSize.height = arg2;
                break;

            case SESSION_EVENT_ERROR_SERVICE:
                Log.d(TAG, "postEventFromNative : msg = MSG_ERROR_SERVICE");

                // Call release to leave VTSource handler thread
                vp.getSource().release();

                mVTProviderUtil.recordRemove(id);
                updateDefaultId();

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_ERROR_SERVER_DIED:
                Log.d(TAG, "postEventFromNative : msg = MSG_ERROR_SERVER_DIED");

                mVTProviderUtil.releaseVTSourceAll();

                mVTProviderUtil.quitAllThread();
                mVTProviderUtil.recordRemoveAll();
                updateDefaultId();

                ImsVTProviderUtil.getInstance().reInitRefVTP();
                break;

            case SESSION_EVENT_ERROR_CAMERA_CRASHED:
                Log.d(TAG, "postEventFromNative : msg = MSG_ERROR_CAMERA_CRASHED");

                vp.handleMaErrorProcess();
                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_ERROR_BIND_PORT:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_ERROR_BIND_PORT");

                vp.handleMaErrorProcess();
                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_ERROR_CODEC:
                Log.d(TAG, "postEventFromNative : msg = MSG_ERROR_CODEC");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_ERROR_REC:
                Log.d(TAG, "postEventFromNative : msg = MSG_ERROR_REC");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_ERROR_CAMERA_SET_IGNORED:
                Log.d(TAG, "postEventFromNative : msg = MSG_ERROR_CAMERA_SET_IGNORED");

                vp.handleCallSessionEvent(msg);
                break;

            case SESSION_EVENT_DATA_PATH_PAUSE:
            case SESSION_EVENT_DATA_PATH_RESUME:
                Log.d(TAG, "postEventFromNative : msg = EVENT_DATA_PATH_CHANGED");

                if (!ImsVTProviderUtil.sIsNoCameraMode) {
                    vp.handleCallSessionEvent(msg);
                }
                break;

            case SESSION_EVENT_GET_SENSOR_INFO: {
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_GET_SENSOR_INFO");

                VTSource.Resolution [] cams_info = VTSource.getAllCameraResolutions();
                if(null == cams_info) {
                    Log.e(TAG, "Error: sensor resolution = NULL");
                }

                nSetCameraParametersOnly(cams_info);
                break;
            }

            case SESSION_EVENT_RESTART_CAMERA:
                Log.d(TAG, "postEventFromNative : msg = SESSION_EVENT_RESTART_CAMERA");

                vp.getSource().restart();
                break;

            default:
                Log.d(TAG, "postEventFromNative : msg = UNKNOWB");
                break;
        }
    }
}
