/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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
package com.mediatek.rcse.plugin.phone;

import android.app.Activity;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothHeadset;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PixelFormat;
import android.graphics.drawable.Drawable;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.OrientationEventListener;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcs.R;
import com.mediatek.rcse.rtp.format.video.Orientation;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcse.settings.RcsSettings;
//import com.orangelabs.rcs.utils.PhoneUtils;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

import com.gsma.services.rcs.RcsServiceException;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.sharing.video.VideoSharing;
import com.gsma.services.rcs.sharing.video.VideoSharingIntent;
import com.gsma.services.rcs.sharing.video.VideoSharingListener;
import com.gsma.services.rcs.sharing.video.VideoSharing.ReasonCode;
import com.gsma.services.rcs.sharing.video.VideoSharing.State;

/**
 * This class defined to implement the function interface of ICallScreenPlugIn,
 * and acheive  the main function here.When user starts a video share the method
 * {@link #start(String)} will be called. If client receive a video share, it
 * will go to {@link VideoSharingInvitationReceiver#onReceive(Context, Intent)}.
 */
public class VideoSharingPlugin extends SharingPlugin implements
        SurfaceHolder.Callback, OnClickListener,
        DialogInterface.OnClickListener, SensorEventListener {
    /**
     * The Constant TAG.
     */
    private static final String TAG = "VideoSharingPlugin";
    /* package *//**
                  * The Constant VIDEO_SHARING_INVITATION_ACTION.
                  */
    static final String VIDEO_SHARING_INVITATION_ACTION =
            "com.orangelabs.rcs.richcall.VIDEO_SHARING_INVITATION";
    /* package *//**
                  * The Constant VIDEO_SHARING_ACCEPT_ACTION.
                  */
    static final String VIDEO_SHARING_ACCEPT_ACTION =
            "com.mediatek.phone.plugin.VIDEO_SHARING_ACCEPT_ACTION";
    /* package *//**
                  * The Constant VIDEO_SHARING_DECLINE_ACTION.
                  */
    static final String VIDEO_SHARING_DECLINE_ACTION =
            "com.mediatek.phone.plugin.VIDEO_SHARING_DECLINE_ACTION";
    /* package *//**
                  * The Constant SERVICE_STATUS.
                  */
    static final String SERVICE_STATUS = "com.orangelabs.rcs.SERVICE_STATUS";
    /* package *//**
                  * The Constant SERVICE_REGISTRATION.
                  */
    static final String SERVICE_REGISTRATION =
            "com.orangelabs.rcs.SERVICE_REGISTRATION";
    /* package *//**
                  * The Constant VIDEO_NAME.
                  */
    static final String VIDEO_NAME = "videoName";
    /* package *//**
                  * The Constant VIDEO_DURATION.
                  */
    static final String VIDEO_DURATION = "videoDuration";
    /* package *//**
                  * The Constant VIDEO_ENCODING.
                  */
    static final String VIDEO_ENCODING = "videoEncoding";
    /* package *//**
                  * The Constant VIDEO_WIDTH.
                  */
    static final String VIDEO_WIDTH = "videoWidth";
    /* package *//**
                  * The Constant VIDEO_HEIGHT.
                  */
    static final String VIDEO_HEIGHT = "videoHeight";
    /**
     * The Constant QVGA.
     */
    private static final String QVGA = "QVGA";
    /**
     * The Constant TEL_URI_SCHEMA.
     */
    private static final String TEL_URI_SCHEMA = "tel:";
    /**
     * The Constant CAMERA_ID.
     */
    private static final String CAMERA_ID = "camera-id";
    /**
     * The Constant QVGA_WIDTH.
     */
    private static final int QVGA_WIDTH = 480;
    /**
     * The Constant QVGA_HEIGHT.
     */
    private static final int QVGA_HEIGHT = 320;
    /**
     * The Constant QCIF_WIDTH.
     */
    private static final int QCIF_WIDTH = 176;
    /**
     * The Constant QCIF_HEIGHT.
     */
    private static final int QCIF_HEIGHT = 144;
    /**
     * The Constant MIN_WINDOW_WIDTH.
     */
    private static final int MIN_WINDOW_WIDTH = 83;
    /**
     * The Constant MIN_WINDOW_HEIGHT.
     */
    private static final int MIN_WINDOW_HEIGHT = 133;
    /**
     * The Constant SWITCH_BUTTON_WIDTH.
     */
    private static final int SWITCH_BUTTON_WIDTH = 70;
    /**
     * The Constant SWITCH_BUTTON_HEIGHT.
     */
    private static final int SWITCH_BUTTON_HEIGHT = 70;
    /**
     * The Constant TERMINATE_BUTTON_WIDTH.
     */
    private static final int TERMINATE_BUTTON_WIDTH = 32;
    /**
     * The Constant TERMINATE_BUTTON_HEIGHT.
     */
    private static final int TERMINATE_BUTTON_HEIGHT = 32;
    /**
     * The Constant SWITCH_BUTTON_MARGIN_TOP.
     */
    private static final int SWITCH_BUTTON_MARGIN_TOP = 20;
    /**
     * The Constant SWITCH_BUTTON_MARGIN_BOTTOM.
     */
    private static final int SWITCH_BUTTON_MARGIN_BOTTOM = 0;
    /**
     * The Constant SWITCH_BUTTON_MARGIN_LEFT.
     */
    private static final int SWITCH_BUTTON_MARGIN_LEFT = 0;
    /**
     * The Constant SWITCH_BUTTON_MARGIN_RIGHT.
     */
    private static final int SWITCH_BUTTON_MARGIN_RIGHT = 20;
    /**
     * The Constant TERMINATE_BUTTON_MARGIN_LEFT.
     */
    private static final int TERMINATE_BUTTON_MARGIN_LEFT = 20;
    /**
     * The Constant TERMINATE_BUTTON_MARGIN_RIGHT.
     */
    private static final int TERMINATE_BUTTON_MARGIN_RIGHT = 20;
    /**
     * The Constant LOCAL_CAMERA_MARGIN_TOP.
     */
    private static final int LOCAL_CAMERA_MARGIN_TOP = 10;
    /**
     * The Constant LOCAL_CAMERA_MARGIN_BOTTOM.
     */
    private static final int LOCAL_CAMERA_MARGIN_BOTTOM = 0;
    /**
     * The Constant LOCAL_CAMERA_MARGIN_LEFT.
     */
    private static final int LOCAL_CAMERA_MARGIN_LEFT = 0;
    /**
     * The Constant LOCAL_CAMERA_MARGIN_RIGHT.
     */
    private static final int LOCAL_CAMERA_MARGIN_RIGHT = 10;
    /**
     * The Constant DELAY_TIME.
     */
    private static final int DELAY_TIME = 500;
    /**
     * The Constant CMA_MODE.
     */
    private static final String CMA_MODE = "cam-mode";
    /**
     * The Constant ROTATION_BY_HW.
     */
    private static final int ROTATION_BY_HW = 3;
    /**
     * The Constant SINGLE_CAMERA.
     */
    private static final int SINGLE_CAMERA = 1;
    /**
     * The Constant ROTATION_0.
     */
    private static final int ROTATION_0 = 0;
    /**
     * The Constant ROTATION_90.
     */
    private static final int ROTATION_90 = 90;
    /**
     * The Constant ROTATION_180.
     */
    private static final int ROTATION_180 = 180;
    /**
     * The Constant ROTATION_270.
     */
    private static final int ROTATION_270 = 270;
    /**
     * The m share status.
     */
    private int mShareStatus = ShareStatus.UNKNOWN;
    /**
     * The m orientation listener.
     */
    private OrientationEventListener mOrientationListener;
    /**
     * The m orient.
     */
    private int mOrient = 0;
    /**
     * The m video rotated.
     */
    boolean mVideoRotated = false;
    /**
     * Session has been established (i.e. 200 OK/ACK exchanged)
     */
    public final static int ESTABLISHED = 1;
    /**
     * Session has been terminated (i.e. SIP BYE exchanged)
     */
    public final static int TERMINATED = 2;
    /**
     * Session is pending (not yet accepted by a final response by the remote).
     */
    public final static int PENDING = 3;
    // Indicate whether click start button
    /**
     * The m is started.
     */
    private final AtomicBoolean mIsStarted = new AtomicBoolean(false);
    /**
     * The m video sharing state.
     */
    private final AtomicInteger mVideoSharingState = new AtomicInteger(
            Constants.SHARE_VIDEO_STATE_IDLE);
    /**
     * The m outgoing display area.
     */
    private RelativeLayout mOutgoingDisplayArea = null;
    /**
     * The m incoming display area.
     */
    private ViewGroup mIncomingDisplayArea = null;
    // In current phase ,just care live video share, as a result it's value
    // never false.
    // Surface holder for video preview
    /**
     * The m preview surface holder.
     */
    private SurfaceHolder mPreviewSurfaceHolder = null;
    
    private String mlast_sharing_Id = "";
    /**
     * The m camera.
     */
    private Camera mCamera = null;
    /**
     * The m lock.
     */
    private final Object mLock = new Object();
    // Camera preview started flag
    /**
     * The m camera preview running.
     */
    private boolean mCameraPreviewRunning = false;
    /**
     * The m outgoing video sharing session.
     */
    private VideoSharing mOutgoingVideoSharingSession = null;
    /**
     * The m outgoing video player.
     */
    private LiveVideoPlayer mOutgoingVideoPlayer = null;
    /**
     * The m outgoing local video surface view.
     */
    private VideoSurfaceView mOutgoingLocalVideoSurfaceView = null;
    /**
     * The m outgoing remote video surface view.
     */
    private VideoSurfaceView mOutgoingRemoteVideoSurfaceView = null;
    /**
     * The m outgoing remote video renderer.
     */
    //private LiveVideoRenderer mOutgoingRemoteVideoRenderer = null;
    /**
     * The m outgoing video format.
     */
    private String mOutgoingVideoFormat = null;
    /**
     * The m video width.
     */
    private int mVideoWidth = 176;
    /**
     * The m video height.
     */
    private int mVideoHeight = 144;
    // Video surface holder
    /**
     * The m surface holder.
     */
    private SurfaceHolder mSurfaceHolder;
    /**
     * The m is video sharing sender.
     */
    private final AtomicBoolean mIsVideoSharingSender =
            new AtomicBoolean(
            false);
    /**
     * The m is video sharing receiver.
     */
    private final AtomicBoolean mIsVideoSharingReceiver =
            new AtomicBoolean(
            false);
    /**
     * The m incoming session event listener.
     */
    private IncomingSessionEventListener mIncomingSessionEventListener = null;
    /**
     * The m outgoing session event listener.
     */
    private OutgoingSessionEventListener mOutgoingSessionEventListener = null;
    // For incoming video share information
    /**
     * The m incoming video sharing session.
     */
    private volatile VideoSharing mIncomingVideoSharingSession = null;
    /**
     * The m incoming local video surface view.
     */
    private VideoSurfaceView mIncomingLocalVideoSurfaceView = null;
    /**
     * The m incoming remote video surface view.
     */
    private static VideoSurfaceView mIncomingRemoteVideoSurfaceView = null;
    /**
     * The m incoming remote video renderer.
     */
   // public LiveVideoRenderer mIncomingRemoteVideoRenderer = null;
    /**
     * The m camer number.
     */
    private int mCamerNumber = 0;
    /**
     * The m opened camera id.
     */
    private int mOpenedCameraId = 0;
    /**
     * The m incoming session id.
     */
    String mIncomingSessionId = null;
    /**
     * The m incoming video format.
     */
    String mIncomingVideoFormat = null;
    /**
     * The m audio button.
     */
    private ImageButton mAudioButton = null;
    /**
     * The m switch camer image view.
     */
    private ImageView mSwitchCamerImageView = null;
    /**
     * The m end sender session image view.
     */
    private ImageView mEndSenderSessionImageView = null;
    /**
     * The m end receiver session image view.
     */
    private ImageView mEndReceiverSessionImageView = null;
    /**
     * The m get incoming session when api connected.
     */
    private final AtomicBoolean mGetIncomingSessionWhenApiConnected =
            new AtomicBoolean(
            false);
    /**
     * The m start outgoing session when api connected.
     */
    private final AtomicBoolean mStartOutgoingSessionWhenApiConnected =
            new AtomicBoolean(
            false);
    // Save a set of number to be listened, the number was passed from phone
    /**
     * The m numbers to be listened.
     */
    private final CopyOnWriteArraySet<String> mNumbersToBeListened =
            new CopyOnWriteArraySet<String>();
    /**
     * The m call screen dialog set.
     */
    private final CopyOnWriteArraySet<CallScreenDialog> mCallScreenDialogSet =
            new CopyOnWriteArraySet<CallScreenDialog>();
    /**
     * The m call screen saved dialog set.
     */
    private List<CallScreenDialog> mCallScreenSavedDialogSet =
            new ArrayList<CallScreenDialog>();
    /**
     * The m waiting progress dialog.
     */
    private WaitingProgressDialog mWaitingProgressDialog = null;
    /**
     * The m video sharing dialog manager.
     */
    private VideoSharingDialogManager mVideoSharingDialogManager = null;
    /**
     * The m video duration.
     */
    private long mVideoDuration;
    /**
     * The Constant VIDEO_LIVE.
     */
    public static final String VIDEO_LIVE = "videolive";
    /**
     * The Constant AUDIO_BUTTON.
     */
    private static final String AUDIO_BUTTON = "audioButton";
    /**
     * The Constant VIEW_ID.
     */
    private static final String VIEW_ID = "id";
    
    private ImageView mDummyImageView;
    /**
     * The m headset connected.
     */
    private boolean mHeadsetConnected = false;
    /**
     * The m video sharing invitation receiver.
     */
    private VideoSharingInvitationReceiver mVideoSharingInvitationReceiver =
            new VideoSharingInvitationReceiver();
    /**
     * The m video sharing call back.
     */
    private ISharingPlugin mVideoSharingCallBack = new ISharingPlugin() {
        @Override
        public void onApiConnected() {
            Logger.d(TAG, "onApiConnected() entry!");
            mCountDownLatch.countDown();
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    // there may be an case that content sharing plugin starts
                    // earlier than core service, so we need to refresh data
                    // that may be modified by auto-configuration
                    //PhoneUtils.initialize(mContext);
                }
            });
            if (mGetIncomingSessionWhenApiConnected.compareAndSet(
                    true, false)) {
                Logger.v(
                        TAG,
                        "onApiConnected(), "
                                + "Richcall api connected, and need to get" +
                                " incoming video share session");
                getIncomingVideoSharingSession();
            } else {
                Logger.v(
                        TAG,
                        "onApiConnected(), "
                                + "Richcall api connected, but need not to get" +
                                " incoming video share session");
            }
            if (mStartOutgoingSessionWhenApiConnected.compareAndSet(
                    true, false)) {
                Logger.v(
                        TAG,
                        "onApiConnected(), "
                                + "Richcall api connected, and need to start" +
                                " outgoing video share session");
                startOutgoingVideoShareSession();
            } else {
                Logger.v(
                        TAG,
                        "onApiConnected(), "
                                + "Richcall api connected, but need not to" +
                                " start outgoing video share session");
            }
        }
        @Override
        public void onFinishSharing() {
            destroy();
        }
    };

    /**
     * Constructor.
     *
     * @param context the context
     */
    public VideoSharingPlugin(Context context) {
        super(context);
        Logger.v(TAG, "VideoSharingPlugin constructor. context = "
                + context);
        IntentFilter intentFilter = new IntentFilter();
        intentFilter
                .addAction(VideoSharingIntent.ACTION_NEW_INVITATION);
        intentFilter.addAction(VIDEO_SHARING_ACCEPT_ACTION);
        intentFilter.addAction(VIDEO_SHARING_DECLINE_ACTION);
        intentFilter.addAction(Intent.ACTION_HEADSET_PLUG);
        intentFilter
                .addAction(BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED);
        mContext.registerReceiver(mVideoSharingInvitationReceiver,
                intentFilter);
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.v(TAG,
                        "CallScreenPlugin constructor. Thread start run.");
                if (RcsSettings.getInstance() != null) {
                    mOutgoingVideoFormat = RcsSettings.getInstance()
                            .getCShVideoFormat();
                    String cshVideoSize = RcsSettings.getInstance()
                            .getCShVideoSize();
                    Logger.v(TAG, "cshVideoSize = " + cshVideoSize);
                    if (QVGA.equals(cshVideoSize)) {
                        // QVGA
                        mVideoWidth = QVGA_WIDTH;
                        mVideoHeight = QVGA_HEIGHT;
                    } else {
                        // QCIF
                        mVideoWidth = QCIF_WIDTH;
                        mVideoHeight = QCIF_HEIGHT;
                    }
                } else {
                    Logger.e(TAG,
                            "RcsSettings.getInstance() return null");
                }
                Logger.v(TAG, "mOutgoingVideoFormat = "
                        + mOutgoingVideoFormat);
                //mOutgoingVideoFormat = "h264";
                Logger.v(TAG, "mOutgoingVideoFormat = "
                        + mOutgoingVideoFormat);
                MediatekFactory.setApplicationContext(mContext);
                // Get the number of the camera
                mCamerNumber = Utils.getCameraNums();
                Logger.v(TAG, "mCamerNumber = " + mCamerNumber);
                return null;
            }
            @Override
            protected void onPostExecute(Void result) {
                Logger.v(TAG, "onPostExecute() ");
                Logger.v(TAG,
                        "After register mVideoSharingInvitationReceiver");
                if (mCallScreenHost != null) {
                    boolean isSupportVideoShare = getCapability(sNumber);
                    mCallScreenHost.onCapabilityChange(sNumber,
                            isSupportVideoShare);
                }
            }
        } .execute();
        mOrientationListener = new OrientationEventListener(context,
                SensorManager.SENSOR_DELAY_NORMAL) {
            public void onOrientationChanged(int orientation) {
                final int iLookup[] = { 0, 0, 0, 90, 90, 90, 90, 90,
                        90, 180, 180, 180, 180, 180, 180, 270, 270,
                        270, 270, 270, 270, 0, 0, 0 }; // 15-degree increments
                if (orientation != ORIENTATION_UNKNOWN) {
                    int iNewOrientation = iLookup[orientation / 15];
                    if (mOrient != iNewOrientation) {
                        mOrient = iNewOrientation;
                        if (mCameraPreviewRunning) {
                            Logger.d(TAG,
                                    "onOrientationChanged mOrient = "
                                            + mOrient);
                            cameraRotation();
                        }
                    }
                }
            }
        };
        // To display if orientation detection will work and enable it
        if (mOrientationListener.canDetectOrientation()) {
            Logger.d(TAG, "onOrientationChanged Enabled");
            mOrientationListener.enable();
        } else {
            Logger.d(TAG, "onOrientationChanged Cannot detect");
        }
        mSwitchCamerImageView = new ImageView(mContext);
        mSwitchCamerImageView
                .setImageResource(R.drawable.ic_rotate_camera_disabled_holo_dark);
        mSwitchCamerImageView
                .setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        switchCamera();
                    }
                });
        mEndSenderSessionImageView = new ImageView(mContext);
        mEndSenderSessionImageView
                .setImageResource(R.drawable.btn_terminate_video_share_pre_sender);
        mEndReceiverSessionImageView = new ImageView(mContext);
        mEndReceiverSessionImageView
                .setImageResource(R.drawable.btn_terminate_video_share_pre_receiver);
        mEndSenderSessionImageView
                .setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Logger.v(TAG,
                                "onClick() listener Sender Button share status ="
                                        + mShareStatus);
                        if (mIsVideoSharingReceiver.get()) {
                            Logger.v(TAG,
                                    "onClick() listener Sender Button LIVE_TWOWAY");
                            finishSenderTwoWaylocal();
                            mShareStatus = ShareStatus.LIVE_IN;
                        } else {
                            Logger.v(TAG,
                                    "onClick()listener receiver Button LIVE_OUT");
                            VideoSharingPlugin.this.stop();
                            mShareStatus = ShareStatus.UNKNOWN;
                        }
                        mIsVideoSharingSender.set(false);
                    }
                });
        mEndReceiverSessionImageView
                .setOnClickListener(new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Logger.v(TAG,
                                "onClick listener() receiver Button status = "
                                        + mShareStatus);
                        if (mIsVideoSharingSender.get()) {
                            Logger.v(TAG,
                                    "onClick() listener receiver Button LIVE_TWOWAY");
                            finishReceiverTwoWayLocal();
                            mShareStatus = ShareStatus.LIVE_OUT;
                        } else {
                            Logger.v(TAG,
                                    "onClick() listener receiver Button LIVE_IN");
                            VideoSharingPlugin.this.stop();
                            mShareStatus = ShareStatus.UNKNOWN;
                        }
                        mIsVideoSharingReceiver.set(false);
                    }
                });
        mVideoSharingDialogManager = new VideoSharingDialogManager();
        mInterface = mVideoSharingCallBack;
    }
    /**
     * Inits the audio button.
     */
    private void initAudioButton() {
        Logger.d(TAG, "initAudioButton entry, mAudioButton: "
                + mAudioButton + " mCallScreenHost: "
                + mCallScreenHost);
        if (mAudioButton == null) {
            if (mCallScreenHost != null) {
                Activity inCallScreen = mCallScreenHost
                        .getCallScreenActivity();
                String packageName = inCallScreen.getPackageName();
                Resources resource = inCallScreen.getResources();
                mAudioButton = (ImageButton) inCallScreen
                        .findViewById(resource.getIdentifier(
                                AUDIO_BUTTON, VIEW_ID, packageName));
            } else {
                Logger.d(TAG,
                        "initAudioButton mCallScreenHost is null");
            }
        }
        Logger.d(TAG, "initAudioButton exit: " + mAudioButton);
    }
    // Implements ICallScreenPlugIn
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#start(java.lang.String)
     */
    @Override
    public void start(String number) {
        Logger.d(TAG, "start() entry, number: " + number
                + " mShareStatus: " + mShareStatus);
        super.start(number);
        // Image share is ongoing.
        if (Utils.isInImageSharing()) {
            alreadyOnGoing();
            return;
        }
        // Video share is of full status.
        if (mShareStatus == ShareStatus.LIVE_OUT
                || mShareStatus == ShareStatus.LIVE_TWOWAY) {
            alreadyOnGoing();
            return;
        }
        if (!isVideoShareSupported(number)) {
            videoShareNotSupported();
            return;
        }
        if (RCSeInCallUIExtension.getInstance().getmShareFilePlugIn()
				.getCurrentState() == Constants.SHARE_FILE_STATE_DISPLAYING) {
			RCSeInCallUIExtension.getInstance().getmShareFilePlugIn().clearImageSharingViews();
			Logger.v(TAG,
                    "Start SHARE_FILE_STATE_DISPLAYING");
        }
        if (mCallScreenHost != null) {
            mDisplayArea = mCallScreenHost.requestAreaForDisplay();
        } else {
            Logger.d(TAG, "start mCallScreenHost is null");
        }
        startVideoShare();
        // Keep the Screen light ON
        if(RCSeInCallUIExtension.getInstance().mActivity != null) {
        RCSeInCallUIExtension.getInstance().mActivity.getWindow()
            .addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        }
        RCSeInCallUIExtension.getInstance().mActivity.convertToTranslucent(null, null);
    }
    
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#clearImageSharingViews()
     */
    @Override
	public void clearImageSharingViews() {
	}
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#stop()
     */
    @Override
    public void stop() {
        Logger.v(TAG, "stop button is clicked");
        // Turn OFF ScreenLight ON Demand
        if(RCSeInCallUIExtension.getInstance().mActivity != null) {
        RCSeInCallUIExtension.getInstance().mActivity.getWindow()
            .clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        RCSeInCallUIExtension.getInstance().mActivity.convertFromTranslucent();
        }
        destroy();
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#getState()
     */
    @Override
    public int getState() {
        Logger.v(TAG, "getState(), getState() = "
                + mVideoSharingState.get());
        return mVideoSharingState.get();
    }
    /**
     * Start a live video sharing based on current status.
     * Start sharing may not execute.
     */
    private void startVideoShare() {
        Logger.v(TAG, "startVideoShare(), number = " + sNumber
                + ", then request a view group");
        if (sNumber == null) {
            Logger.e(TAG, "startVideoShare() number is null");
            return;
        }
        if (allowOutgoingLiveSharing()) {
            int networkType = Utils.getNetworkType(mContext);
            startByNetwork(networkType);
        } else {
            Logger.w(TAG, "startVideoShare() Start has been clicked,"
                    + " please wait until the session failed");
            alreadyOnGoing();
        }
    }
    /**
     * Start a video sharing( according to current network
     * type. First set sharing status and then check network type, if can not
     * start a video sharing, then restore the sharing status to the previous.
     *
     * @param networkType network type
     */
    private void startByNetwork(int networkType) {
        Logger.d(TAG, "start a live sharing, modify status");
        if (mShareStatus == ShareStatus.LIVE_IN) {
            mShareStatus = ShareStatus.LIVE_TWOWAY;
        } else {
            mShareStatus = ShareStatus.LIVE_OUT;
        }
        // 2G & 2.5
        if (networkType == Utils.NETWORK_TYPE_GSM
                || networkType == Utils.NETWORK_TYPE_GPRS
                || networkType == Utils.NETWORK_TYPE_EDGE) {
            Logger.v(TAG,
                    "startVideoShare()-2G or 2.5G mobile network, fibbiden video share");
            Resources resources = mContext.getResources();
            String message = resources
                    .getString(R.string.now_allowed_video_share_by_network);
            showToast(message);
            setSharingStatusToPreviousStatus();
            return;
        } else if (networkType == Utils.NETWORK_TYPE_UMTS
                || networkType == Utils.NETWORK_TYPE_HSUPA
                || networkType == Utils.NETWORK_TYPE_HSDPA
                || networkType == Utils.NETWORK_TYPE_1XRTT
                || networkType == Utils.NETWORK_TYPE_EHRPD) { // 2.75G
            // or
            // 3G
            Logger.v(TAG,
                    "startVideoShare()-2.75G or 3G mobile network, "
                            + "allow single line video share");
            if (mIncomingVideoSharingSession != null) {
                try {
                    if (mIncomingVideoSharingSession.getState() == VideoSharing.State.STARTED
                            || mIncomingVideoSharingSession
                                    .getState() == VideoSharing.State.INVITED) {
                        Resources resources = mContext.getResources();
                        String message = resources
                                .getString(R.string.now_allowed_video_share_by_network);
                        showToast(message);
                        setSharingStatusToPreviousStatus();
                        return;
                    }
                } catch (Exception e) {
                    Logger.e(TAG, e.toString());
                }
            } else {
                Logger.d(TAG,
                        "startByNetwork(), mIncomingVideoSharingSession is null");
            }
        } else if (networkType == Utils.NETWORK_TYPE_HSPA
                || networkType == Utils.NETWORK_TYPE_LTE
                || networkType == Utils.NETWORK_TYPE_UMB) { // 4G
            Logger.v(TAG,
                    "startVideoShare()-4G mobile network, allow two-way video share");
        } else if (networkType == Utils.NETWORK_TYPE_WIFI) { // WI-FI
            Logger.v(TAG,
                    "startVideoShare()-WI-FI network, allow two-way video share");
        } else { // Unknown
            Logger.v(
                    TAG,
                    "startVideoShare()-Unknown network, default to allow two-way video share");
        }
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
            Logger.v(TAG,
                    "startVideoShare() when start, acquire a wake lock");
        } else {
            Logger.v(TAG,
                    "startVideoShare() when start, the wake lock has been acquired,"
                            + " so do not acquire");
        }
        mNumbersToBeListened.add(sNumber);
        if (mShareStatus == ShareStatus.LIVE_OUT
                || mShareStatus == ShareStatus.LIVE_TWOWAY) {
            // Create the live video player
            Logger.v(
                    TAG,
                    "startVideoShare() start createLiveVideoPlayer mOutgoingVideoFormat is: "
                            + mOutgoingVideoFormat);
            mOutgoingVideoPlayer = new LiveVideoPlayer();
        }
        // Tell that it is the vs sender if start vs before receive vs
        // invitation
        mIsVideoSharingSender.set(true);
        Logger.w(TAG, "startVideoShare() mCallScreenHost = "
                + mCallScreenHost);
        if (mCallScreenHost != null) {
            mOutgoingDisplayArea = (RelativeLayout) mCallScreenHost
                    .requestAreaForDisplay();
            mOutgoingDisplayArea.setVisibility(View.VISIBLE);
        }
        showLocalView();
    }
    /**
     * Set Sharing status to previous status when the client failed to start a
     * video sharing.
     */
    private void setSharingStatusToPreviousStatus() {
        Logger.d(TAG, "setSharingStatusToPreviousStatus");
        if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
            mShareStatus = ShareStatus.LIVE_IN;
        } else {
            mShareStatus = ShareStatus.UNKNOWN;
        }
    }
    /**
     * Check whether allow a outgoing live sharing.
     *
     * @return True if allow a outgoing live sharing
     */
    private boolean allowOutgoingLiveSharing() {
        boolean allow = false;
        if (mShareStatus == ShareStatus.LIVE_OUT
                || mShareStatus == ShareStatus.LIVE_TWOWAY) {
            allow = false;
        } else {
            allow = true;
        }
        Logger.d(TAG, "allowOutgoingLiveSharing(): return " + allow);
        return allow;
    }
    /**
     * Show local view.
     */
    private void showLocalView() {
        Logger.d(TAG, "showLocalView(), mShareStatus: "
                + mShareStatus + " mIsVideoSharingSender: "
                + mIsVideoSharingSender);
        if (mShareStatus != ShareStatus.UNKNOWN) {
            if (mIsVideoSharingReceiver.get()) {
                Logger.v(
                        TAG,
                        "startVideoShare() After recevie vs invitation" +
                        ", then send vs invitation");
                showReceiverLocalView();
                showWaitRemoteAcceptMessage();
            } else if (mIsVideoSharingSender.get()) {
                Logger.v(TAG,
                        "startVideoShare() First send vs invitation");
                showSenderLocalView();
                showWaitRemoteAcceptMessage();
            }
        }
        mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
        Utils.setInVideoSharing(true);
        if (mRichCallStatus == RichCallStatus.CONNECTED) {
            Logger.v(TAG,
                    "startVideoShare(), then call startOutgoingVideoShareSession()");
            startOutgoingVideoShareSession();
        } else {
            Logger.v(
                    TAG,
                    "startVideoShare(), call startOutgoingVideoShareSession()" +
                    " when richcall api connected.");
            mStartOutgoingSessionWhenApiConnected.set(true);
            resetVideoShaingState();
        }
    }
    /**
     * Already on going.
     */
    private void alreadyOnGoing() {
        Logger.v(TAG, "alreadyOnGoing entry");
        if (Utils.isInImageSharing()
                && RCSeInCallUIExtension.getInstance()
                        .getmShareFilePlugIn().getCurrentState() ==
                        Constants.SHARE_FILE_OUTGOING) {
            Logger.v(TAG,
                    "alreadyOnGoing isInImageSharing outgoing file true");
            mVideoSharingDialogManager.alreadyOnGoingImageShare();
        } else if (Utils.isInImageSharing()
                && RCSeInCallUIExtension.getInstance()
                        .getmShareFilePlugIn().getCurrentState() ==
                        Constants.SHARE_FILE_INCOMING) {
            Logger.v(TAG,
                    "alreadyOnGoing isInImageSharing Incmoing file true");
            startVideoShare();
        } else if (RCSeInCallUIExtension.getInstance().getmShareFilePlugIn()
				.getCurrentState() == Constants.SHARE_FILE_STATE_DISPLAYING) {
			RCSeInCallUIExtension.getInstance().getmShareFilePlugIn().stop();
			Logger.v(TAG,
                    "alreadyOnGoing isInImageSharing SHARE_FILE_STATE_DISPLAYING");
			startVideoShare();
		} 
        else {
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    Logger.v(TAG,
                            "alreadyOnGoing isInImageSharing false");
                    String message = mContext
                            .getResources()
                            .getString(
                                    R.string.video_sharing_is_on_going);
                    showToast(message);
                }
            });
        }
        Logger.v(TAG, "alreadyOnGoing exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#saveAlertDialogs()
     */
    @Override
    public void saveAlertDialogs() {
        Logger.v(TAG, "saveAlertDialogs() Video entry");
        mCallScreenSavedDialogSet.clear();
        mCallScreenSavedDialogSet.addAll(mCallScreenDialogSet);
        Logger.v(TAG, "saveAlertDialogs() Video exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#showAlertDialogs()
     */
    @Override
    public void showAlertDialogs() {
        Logger.v(TAG, "showAlertDialogs() Video entry");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mCallScreenSavedDialogSet.size() > 1) {
                    mCallScreenDialogSet
                            .add(mCallScreenSavedDialogSet
                                    .get(mCallScreenSavedDialogSet
                                            .size() - 1));
                    mCallScreenSavedDialogSet.get(
                            mCallScreenSavedDialogSet.size() - 1)
                            .show();
                } else if (mCallScreenSavedDialogSet.size() == 1) {
                    mCallScreenDialogSet
                            .add(mCallScreenSavedDialogSet.get(0));
                    mCallScreenSavedDialogSet.get(0).show();
                }
                mCallScreenSavedDialogSet.clear();
            }
        });
        Logger.v(TAG, "showAlertDialogs() Video exit");
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#clearSavedDialogs()
     */
    @Override
    public void clearSavedDialogs() {
        Logger.v(TAG,
                "clearSavedDialogs() entry mCallScreenSavedDialogSet size is "
                        + mCallScreenSavedDialogSet.size());
        mCallScreenSavedDialogSet.clear();
        Logger.v(TAG,
                "clearSavedDialogs() exit mCallScreenSavedDialogSet size is "
                        + mCallScreenSavedDialogSet.size());
    }
    /* (non-Javadoc)
     * @see com.mediatek.rcse.plugin.phone.SharingPlugin#dismissDialog()
     */
    @Override
    public boolean dismissDialog() {
        Logger.v(TAG, "dismissDialog()");
        dismissAllDialogs();
        return false;
    }
    /**
     * Dismiss all dialogs.
     */
    private void dismissAllDialogs() {
        Logger.v(TAG, "dismissAllDialog()");
        for (CallScreenDialog dialog : mCallScreenDialogSet) {
            dialog.dismissDialog();
            mCallScreenDialogSet.remove(dialog);
            Logger.v(TAG, "have dismissed a dialog: " + dialog);
        }
        mVideoSharingDialogManager
                .dismissWaitingInitializeConextProgressDialog();
    }
    /**
     * Start the outgoing session.
     */
    private void startOutgoingVideoShareSession() {
        Logger.v(TAG, "startOutgoingVideoShareSession");
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    // Initiate sharing
                    Logger.d(TAG,
                            "startOutgoingVideoShareSession(), mShareStatus: "
                                    + mShareStatus);
                    if (mShareStatus == ShareStatus.LIVE_OUT
                            || mShareStatus == ShareStatus.LIVE_TWOWAY) {
                        String operatorAccount = getOperatorAccount(sNumber);
                        if (operatorAccount != null) {
                            mOutgoingSessionEventListener =
                                    new OutgoingSessionEventListener();                            
                            mOutgoingVideoSharingSession = mVideoSharingApi
                                    .shareVideo(null,
                                            mOutgoingVideoPlayer); // ToDo
                        } else {
                            Logger.w(TAG, "operatorAccount is null");
                        }
                    }
                    if (mOutgoingVideoSharingSession != null) {
                        Logger.w(TAG,
                                "mOutgoingVideoSharingSession started.");
                    } else {
                        Logger.w(TAG,
                                "mOutgoingVideoSharingSession is null.");
                        handleStartVideoSharingFailed();
                    }
                } catch (RcsServiceException e) {
                    e.printStackTrace();
                    handleStartVideoSharingFailed();
                }
            }
        });
    }
    /**
     * Handle start video sharing failed.
     */
    private void handleStartVideoSharingFailed() {
        Logger.d(TAG, "handleStartVideoSharingFailed entry");
        if (mIsVideoSharingSender.get()) {
            Logger.v(TAG, "handleStartVideoSharingFailed sender fail");
            destroy();
        } else {
            Logger.v(TAG,
                    "handleStartVideoSharingFailed receiver fail");
            if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
                mShareStatus = ShareStatus.LIVE_IN;
            } else {
                mShareStatus = ShareStatus.UNKNOWN;
            }
            destroyOutgoingViewOnly();
        }
        Logger.d(TAG, "handleStartVideoSharingFailed exit");
    }

    /**
     * Outgoing video sharing session event listener.
     *
     * @see OutgoingSessionEventEvent
     */
    private class OutgoingSessionEventListener extends
            VideoSharingListener {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "OutgoingSessionEventListener";
        
        public void onStateChanged(ContactId contact, String sharingId, State state,
                ReasonCode reasonCode) {
            
            if(state == State.STARTED) {
                Logger.v(TAG, "onSharingStarted(), mShareStatus: "
                        + mShareStatus + " mHeadsetConnected: "
                        + mHeadsetConnected);
                try {
                    if (mOutgoingVideoSharingSession != null) {
                        if (is3GMobileNetwork()) {
                            if (mIncomingVideoSharingSession != null) {
                                Logger.d(
                                        TAG,
                                        "Reject the incoming session because the" +
                                        " device is upder "
                                                + "3G network and the outgoing" +
                                                " video share session is established");
                                mIncomingVideoSharingSession
                                        .abortSharing();
                            } else {
                                Logger.v(TAG,
                                        "onSharingStarted(),Entered in else of" +
                                        " mIncomingVideoSharingSession");
                                // Tell the host to update & show toast
                                showRemoteAcceptMessage();
                            }
                        } else {
                            Logger.v(TAG,
                                    "onSharingStarted(),Entered in else of 3g");
                            // Tell the host to update & show toast
                            showRemoteAcceptMessage();
                        }
                    } else {
                        Logger.w(TAG,
                                "mOutgoingVideoSharingSession is null");
                    }
                } catch (RcsServiceException e) {
                    e.printStackTrace();
                }            
            } else if (state == State.ABORTED) {                
                Logger.v(TAG, "onSharingAborted()");
                mIsStarted.set(false);
                // At this time we are receiving vs from remote, so
                // should only remove outgoing view
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mIsVideoSharingReceiver.get()) {
                            removeVeiwsAtSenderTerminatedByRemote();
                            String message = mContext
                                    .getString(R.string.video_sharing_terminated_due_to_network);
                            showToast(message);
                        } else {
                            RCSeInCallUIExtension.getInstance()
                                    .resetDisplayArea();
                            mVideoSharingDialogManager
                                    .showTerminatedByNetworkDialog();
                        }
                    }
                });
            
            } else if (state == State.REJECTED) {
                Logger.v(TAG, "SESSION_INITIATION_DECLINED");
                mVideoSharingDialogManager.showRejectedByRemoteDialog();
            } else if (state == State.FAILED) {
                Logger.v(TAG,
                        "SESSION_INITIATION_FAILED, at most case it is a 408 error(time out)");
                // At this time we are receiving vs from remote,
                // so should only remove outgoing view
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mIsVideoSharingReceiver.get()) {
                            removeVeiwsAtSenderTerminatedByRemote();
                        } else {
                            RCSeInCallUIExtension.getInstance()
                                    .resetDisplayArea();
                            mVideoSharingDialogManager
                                    .showTerminatedByNetworkDialog();
                        }
                    }
                });
            }
            
        }

        // Session is started
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.video.VideoSharingListener#onSharingStarted()
         */
        public void onSharingStarted() {/*
            Logger.v(TAG, "onSharingStarted(), mShareStatus: "
                    + mShareStatus + " mHeadsetConnected: "
                    + mHeadsetConnected);
            try {
                if (mOutgoingVideoSharingSession != null) {
                    if (is3GMobileNetwork()) {
                        if (mIncomingVideoSharingSession != null) {
                            Logger.d(
                                    TAG,
                                    "Reject the incoming session because the" +
                                    " device is upder "
                                            + "3G network and the outgoing" +
                                            " video share session is established");
                            mIncomingVideoSharingSession
                                    .abortSharing();
                        } else {
                            Logger.v(TAG,
                                    "onSharingStarted(),Entered in else of" +
                                    " mIncomingVideoSharingSession");
                            // Tell the host to update & show toast
                            showRemoteAcceptMessage();
                        }
                    } else {
                        Logger.v(TAG,
                                "onSharingStarted(),Entered in else of 3g");
                        // Tell the host to update & show toast
                        showRemoteAcceptMessage();
                    }
                } else {
                    Logger.w(TAG,
                            "mOutgoingVideoSharingSession is null");
                }
            } catch (JoynServiceException e) {
                e.printStackTrace();
            }
        */}
        // Video stream has been resized
        /**
         * Handle video resized.
         *
         * @param width the width
         * @param height the height
         */
        public void handleVideoResized(int width, int height) {
        }
        // Session has been aborted
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.video.VideoSharingListener#onSharingAborted()
         */
       
        public void onSharingAborted() {/*
            Logger.v(TAG, "onSharingAborted()");
            mIsStarted.set(false);
            // At this time we are receiving vs from remote, so
            // should only remove outgoing view
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mIsVideoSharingReceiver.get()) {
                        removeVeiwsAtSenderTerminatedByRemote();
                        String message = mContext
                                .getString(R.string.video_sharing_terminated_due_to_network);
                        showToast(message);
                    } else {
                        RCSeInCallUIExtension.getInstance()
                                .resetDisplayArea();
                        mVideoSharingDialogManager
                                .showTerminatedByNetworkDialog();
                    }
                }
            });
        */}
        
        public void onDeleted(ContactId contact, Set<String> sharingIds) {
         // To Do   
        }
        // Session has been terminated by remote
        /**
         * Handle session terminated by remote.
         */
        public void handleSessionTerminatedByRemote() {
            Logger.v(TAG, "handleSessionTerminatedByRemote()");
            // At this time we are receiving vs from remote, so
            // should only remove outgoing view
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mIsVideoSharingReceiver.get()) {
                        removeVeiwsAtSenderTerminatedByRemote();
                    } else {
                        RCSeInCallUIExtension.getInstance()
                                .resetDisplayArea();
                        mVideoSharingDialogManager
                                .showTerminatedByRemoteDialog();
                    }
                }
            });
        }
        // Content sharing error
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.video.VideoSharingListener#onSharingError(int)
         */
        
        public void onSharingError(final int error) {/*
            Logger.v(TAG, "onSharingError(), error = " + error);
            switch (error) {
            case VideoSharing.Error.INVITATION_DECLINED:
                Logger.v(TAG, "SESSION_INITIATION_DECLINED");
                mVideoSharingDialogManager
                        .showRejectedByRemoteDialog();
                return;
            case VideoSharing.Error.SHARING_FAILED:
                Logger.v(TAG,
                        "SESSION_INITIATION_FAILED, at most case it is a 408 error(time out)");
                // At this time we are receiving vs from remote,
                // so should only remove outgoing view
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mIsVideoSharingReceiver.get()) {
                            removeVeiwsAtSenderTerminatedByRemote();
                        } else {
                            RCSeInCallUIExtension.getInstance()
                                    .resetDisplayArea();
                            mVideoSharingDialogManager
                                    .showTerminatedByNetworkDialog();
                        }
                    }
                });
                return;
                default:
                break;
            }
            // At this time we are receiving vs from remote, so
            // should only remove outgoing view
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mIsVideoSharingReceiver.get()) {
                        removeVeiwsAtSenderTerminatedByRemote();
                    } else {
                        mVideoSharingDialogManager
                                .showTerminatedByRemoteDialog();
                    }
                }
            });
        */}
    }

    /**
     * Removes the veiws at receiver terminated by remote.
     */
    private void removeVeiwsAtReceiverTerminatedByRemote() {
        Logger.v(TAG, "removeVeiwsAtReceiverTerminatedByRemote");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mIncomingDisplayArea != null) {
                    Logger.v(
                            TAG,
                            "removeVeiwsAtReceiverTerminatedByRemote()" +
                            " mIncomingDisplayArea Entry");
                    mIncomingDisplayArea
                            .removeView(mIncomingLocalVideoSurfaceView);
                    mIncomingDisplayArea
                            .removeView(mIncomingRemoteVideoSurfaceView);
                    mIncomingDisplayArea
                            .removeView(mSwitchCamerImageView);
                    mIncomingDisplayArea
                            .removeView(mEndReceiverSessionImageView);
                    mIncomingDisplayArea
                            .removeView(mEndSenderSessionImageView);
                } else {
                    Logger.w(
                            TAG,
                            " removeVeiwsAtReceiverTerminatedByRemote" +
                            " mIncomingDisplayArea is null");
                }
                if (mOutgoingDisplayArea != null) {
                    mOutgoingDisplayArea
                            .removeView(mOutgoingLocalVideoSurfaceView);
                    mOutgoingDisplayArea
                            .removeView(mOutgoingRemoteVideoSurfaceView);
                    mOutgoingDisplayArea
                            .removeView(mSwitchCamerImageView);
                    mOutgoingDisplayArea
                            .removeView(mEndSenderSessionImageView);
                    mOutgoingDisplayArea
                            .removeView(mEndReceiverSessionImageView);
                } else {
                    Logger.w(
                            TAG,
                            " removeVeiwsAtReceiverTerminatedByRemote" +
                            " mOutgoingDisplayArea is null");
                }
                mIsVideoSharingReceiver.set(false);
                if (mIsVideoSharingSender.get()) {
                    Logger.w(TAG,
                            " removeVeiwsAtReceiverTerminatedByRemote sender not null");
                    showSenderLocalView();
                    mShareStatus = ShareStatus.LIVE_OUT;
                } else {
                    Logger.w(TAG,
                            " removeVeiwsAtReceiverTerminatedByRemote sender is null");
                    RCSeInCallUIExtension.getInstance()
                            .resetDisplayArea();
                    mVideoSharingDialogManager
                            .showTerminatedByRemoteDialog();
                    resetVideoShaingState();
                }
            }
        });
    }

    /**
     * Incoming video sharing session event listener.
     *
     * @see IncomingSessionEventEvent
     */
    private class IncomingSessionEventListener extends
            VideoSharingListener {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "IncomingSessionEventListener";
        
        
        public void onStateChanged(ContactId contact, String sharingId, State state,
                ReasonCode reasonCode) {
            
            if(state == State.STARTED) {
                Logger.v(TAG, "onSharingStarted(), mShareStatus: "
                        + mShareStatus + " mHeadsetConnected: "
                        + mHeadsetConnected);
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        Logger.d(TAG,
                                "onFirstTimeRecevie(), mShareStatus: "
                                        + mShareStatus);
                        mVideoSharingDialogManager
                                .dismissWaitingInitializeConextProgressDialog();
                        if(mIncomingDisplayArea!=null)
                        {
                            mIncomingDisplayArea.setVisibility(View.VISIBLE);
                            if(mIncomingRemoteVideoSurfaceView !=null)
                            {
                                mIncomingRemoteVideoSurfaceView.setZOrderMediaOverlay(false);
                            }
                            //mIncomingRemoteVideoSurfaceView.clearImage();
                        }
                        Logger.d(TAG,
                                "onFirstTimeRecevie(), is incoming live video share!");
                    }
                });
            } else if (state == State.ABORTED) {
                Logger.v(TAG, "onSharingAborted()");
                mMainHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mIsVideoSharingSender.get()) {
                            Logger.v(TAG,
                                    "handleSessionAborted() sender true");
                            removeVeiwsAtReceiverTerminatedByRemote();
                        } else {
                            Logger.v(TAG,
                                    "handleSessionAborted() sender false");
                            RCSeInCallUIExtension.getInstance()
                                    .resetDisplayArea();
                            mVideoSharingDialogManager
                                    .showTerminatedByNetworkDialog();
                        }
                    }
                });
            } else if (state == State.REJECTED) {
                Logger.v(TAG, "INVITATION_DECLINED");
            } else if (state == State.FAILED) {
                Logger.v(TAG, "SHARING_FAILED");
            }
            
        }
        
        public void onDeleted(ContactId contact, Set<String> sharingIds) {
            // To Do   
           }

        // Session is started
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.video.VideoSharingListener#onSharingStarted()
         */
        //@Override
        public void onSharingStarted() {/*
            Logger.v(TAG, "onSharingStarted(), mShareStatus: "
                    + mShareStatus + " mHeadsetConnected: "
                    + mHeadsetConnected);
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    Logger.d(TAG,
                            "onFirstTimeRecevie(), mShareStatus: "
                                    + mShareStatus);
                    mVideoSharingDialogManager
                            .dismissWaitingInitializeConextProgressDialog();
                    if(mIncomingDisplayArea!=null)
                    {
                    	mIncomingDisplayArea.setVisibility(View.VISIBLE);
                    	if(mIncomingRemoteVideoSurfaceView !=null)
                    	{
                    		mIncomingRemoteVideoSurfaceView.setZOrderMediaOverlay(false);
                    	}
                    	//mIncomingRemoteVideoSurfaceView.clearImage();
                    }
                    Logger.d(TAG,
                            "onFirstTimeRecevie(), is incoming live video share!");
                }
            });
        */}
        /**
         * Handle video resized.
         *
         * @param width the width
         * @param height the height
         */
        public void handleVideoResized(int width, int height) {
        }
        // Session has been aborted
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.video.VideoSharingListener#onSharingAborted()
         */
       
        public void onSharingAborted() {/*
            Logger.v(TAG, "onSharingAborted()");
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (mIsVideoSharingSender.get()) {
                        Logger.v(TAG,
                                "handleSessionAborted() sender true");
                        removeVeiwsAtReceiverTerminatedByRemote();
                    } else {
                        Logger.v(TAG,
                                "handleSessionAborted() sender false");
                        RCSeInCallUIExtension.getInstance()
                                .resetDisplayArea();
                        mVideoSharingDialogManager
                                .showTerminatedByNetworkDialog();
                    }
                }
            });
        */}
        // Session has been terminated by remote
        /**
         * Handle session terminated by remote.
         */
        public void handleSessionTerminatedByRemote() {
            Logger.v(TAG, "handleSessionTerminatedByRemote()");
            removeVeiwsAtReceiverTerminatedByRemote();
        }
        // Sharing error
        /* (non-Javadoc)
         * @see com.gsma.services.rcs.sharing.video.VideoSharingListener#onSharingError(int)
         */
        
        public void onSharingError(final int error) {/*
            Logger.v(TAG, "onSharingError(), error = " + error);
            switch (error) {
            case VideoSharing.Error.INVITATION_DECLINED:
                Logger.v(TAG, "INVITATION_DECLINED");
                break;
            case VideoSharing.Error.SHARING_FAILED:
                Logger.v(TAG, "SHARING_FAILED");
                break;
            default:
                break;
            }
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (!mIsVideoSharingSender.get()) {
                        Logger.v(TAG,
                                "handleSharingError(), mIsVideoSharingSender false entry");
                        RCSeInCallUIExtension.getInstance()
                                .resetDisplayArea();
                        mVideoSharingDialogManager
                                .showTerminatedByRemoteDialog();
                    }
                }
            });
        */}
    }

    // Implements SurfaceHolder.Callback
    /* (non-Javadoc)
     * @see android.view.SurfaceHolder.Callback#
     * surfaceCreated(android.view.SurfaceHolder)
     */
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        synchronized (mLock) {
            try {
            boolean isLive = (mShareStatus == ShareStatus.LIVE_IN
                    || mShareStatus == ShareStatus.LIVE_OUT || mShareStatus ==
                    ShareStatus.LIVE_TWOWAY);
            Logger.d(TAG, "surfaceCreated(), mCamerNumber: "
                    + mCamerNumber + ", mCamera: " + mCamera
                    + ", mShareStatus: " + mShareStatus);
            if (mCamera == null && isLive) {
                // Start camera preview
                if (mCamerNumber > SINGLE_CAMERA) {
                    // Try to open the front camera
                    Logger.v(TAG,
                            "surfaceCreated(), try to open the front camera");
                    mCamera = Camera
                            .open(CameraInfo.CAMERA_FACING_FRONT);
                    mOpenedCameraId = CameraInfo.CAMERA_FACING_FRONT;
                } else {
                    Logger.v(TAG,
                            "surfaceCreated(), try to open the front camera");
                    openDefaultCamera();
                }
                addPreviewCallback();
            }
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
    /**
     * Adds the preview callback.
     */
    private void addPreviewCallback() {
        Logger.v(TAG, "addPreviewCallback entry");
        if (mCamera == null) {
            Logger.v(TAG, "addPreviewCallback mCamera is null");
            return;
        }
        mCamera.setPreviewCallback(new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] data, Camera camera) {
                onCameraPreviewFrame(data, camera);
            }
        });
        Logger.v(TAG, "addPreviewCallback exit");
    }
    
    public void pauseSharing(){
        if((mShareStatus == ShareStatus.LIVE_TWOWAY || mShareStatus == ShareStatus.LIVE_IN) && 
                mIncomingRemoteVideoSurfaceView != null /*&& mIncomingRemoteVideoRenderer != null*/){
         //mIncomingRemoteVideoRenderer.stopDecoder();
         //mIncomingRemoteVideoRenderer.doRendering = false;       
        }
    }
    
    public void restartSharing(){
        Logger.v(TAG, "restartVideoSharing entry");        
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if((mShareStatus == ShareStatus.LIVE_TWOWAY || mShareStatus == ShareStatus.LIVE_IN) 
                        && mIncomingRemoteVideoSurfaceView != null/* && mIncomingRemoteVideoRenderer != null*/) {
                    //recreateReceiverSurfaceView();
                    synchronized (this) {
                        while (!mIncomingRemoteVideoSurfaceView.mSurfaceCreated) {
                               try {
                                   this.wait();
                               } catch (InterruptedException e) {
                                   Logger.v(TAG, "restartVideoSharing exception");
                               }
                         }
                    }
                Logger.v(TAG, "restartVideoSharing mSurfaceCreated is" + mIncomingRemoteVideoSurfaceView.mSurfaceCreated);                
                //mIncomingRemoteVideoRenderer.doRendering = true;
                //mIncomingRemoteVideoRenderer.restartDecoder(mIncomingRemoteVideoSurfaceView);
               }
            }
        });
    }
    /**
     * On camera preview frame.
     *
     * @param data the data
     * @param camera the camera
     */
    private void onCameraPreviewFrame(byte[] data, Camera camera) {
        if (mOutgoingVideoPlayer != null) {
            mOutgoingVideoPlayer.onPreviewFrame(data, camera);
        } else {
            Logger.d(
                    TAG,
                    "onPreviewFrame(), addPreviewCallback" +
                    " mOutgoingLiveVideoPlayer is null");
        }
    }
    /* (non-Javadoc)
     * @see android.view.SurfaceHolder.Callback#surfaceChanged
     * (android.view.SurfaceHolder, int, int, int)
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format,
            int width, int height) {
        Logger.v(TAG, "surfaceChanged(), mShareStatus: "
                + mShareStatus);
        mPreviewSurfaceHolder = holder;
        synchronized (mLock) {
            if (mCamera != null) {
                if (mCameraPreviewRunning) {
                    mCameraPreviewRunning = false;
                    mCamera.stopPreview();
                }
            } else {
                Logger.w(TAG, "mCamera is null");
            }
        }
        boolean isLive = (mShareStatus == ShareStatus.LIVE_IN
                || mShareStatus == ShareStatus.LIVE_OUT ||
                mShareStatus == ShareStatus.LIVE_TWOWAY);
        if (isLive) {
            startCameraPreview();
        }
    }
    /* (non-Javadoc)
     * @see android.view.SurfaceHolder.Callback#
     * surfaceDestroyed(android.view.SurfaceHolder)
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        Logger.v(TAG, "surfaceDestroyed(), holder = " + holder
                + ",set holder to null");
        stopCameraPreview();
    }
    /**
     * Start the camera preview.
     */
    private void startCameraPreview() {
        try {
        Logger.v(TAG, "startCameraPreview()");
        synchronized (mLock) {
            if (mCamera != null) {
                Camera.Parameters p = mCamera.getParameters();
                // Init Camera
                p.setPreviewSize(mVideoWidth, mVideoHeight);
                p.setPreviewFormat(PixelFormat.YCbCr_420_SP);
                // Try to set front camera if back camera doesn't support size
                List<Camera.Size> sizes = p
                        .getSupportedPreviewSizes();
                if (sizes != null
                        && !sizes.contains(mCamera.new Size(
                                mVideoWidth, mVideoHeight))) {
                    Logger.v(TAG, "Does not contain");
                    String camId = p.get(CAMERA_ID);
                    if (camId != null) {
                        p.set(CAMERA_ID, 2);
                        p.set(CMA_MODE, ROTATION_BY_HW);
                    } else {
                        Logger.v(TAG, "cam_id is null");
                    }
                } else {
                    Logger.v(TAG,
                            "startCameraPreview(), sizes object = "
                                    + sizes
                                    + ". contains the size object. "
                                    + "mOpenedCameraId = "
                                    + mOpenedCameraId);
                    p.set(CMA_MODE, ROTATION_BY_HW);
                }
                android.hardware.Camera.CameraInfo info =
                        new android.hardware.Camera.CameraInfo();
                android.hardware.Camera.getCameraInfo(
                        mOpenedCameraId, info);
                if (info != null && mOutgoingVideoPlayer != null) {
                    switch (mOrient) {
                    case ROTATION_0:
                        Logger.e(TAG, "ROTATION_0");
                        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                            mOutgoingVideoPlayer
                                    .setOrientation(Orientation.ROTATE_90_CCW);
                        } else {
                            mOutgoingVideoPlayer
                                    .setOrientation(Orientation.ROTATE_90_CW);
                        }
                        mCamera.setDisplayOrientation(90);
                        mSwitchCamerImageView.setRotation(0);
                        break;
                    case ROTATION_90:
                        Logger.e(TAG, "ROTATION_90");
                        mCamera.setDisplayOrientation(90);
                        mOutgoingVideoPlayer
                                .setOrientation(Orientation.NONE);
                        mSwitchCamerImageView.setRotation(270);
                        break;
                    case ROTATION_180:
                        Logger.e(TAG, "ROTATION_180");
                        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                            mOutgoingVideoPlayer
                                    .setOrientation(Orientation.ROTATE_90_CW);
                        } else {
                            mOutgoingVideoPlayer
                                    .setOrientation(Orientation.ROTATE_90_CCW);
                        }
                        mCamera.setDisplayOrientation(90);
                        mSwitchCamerImageView.setRotation(180);
                        break;
                    case ROTATION_270:
                        Logger.e(TAG, "ROTATION_270");
                        mCamera.setDisplayOrientation(90);
                        if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                            mOutgoingVideoPlayer
                                    .setOrientation(Orientation.ROTATE_180);
                        } else {
                            mOutgoingVideoPlayer
                                    .setOrientation(Orientation.ROTATE_180);
                        }
                        mSwitchCamerImageView.setRotation(90);
                        break;
                     default:
                    }
                }
                mCamera.setParameters(p);
                try {
                    mCamera.setPreviewDisplay(mPreviewSurfaceHolder);
                    mCamera.startPreview();
                    mCameraPreviewRunning = true;
                } catch (IOException e) {
                    e.printStackTrace();
                    mCamera = null;
                }
            } else {
                Logger.e(TAG, "mCamera is null");
            }
        }
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
    /**
     * Camera rotation.
     */
    private void cameraRotation() {
        Logger.e(TAG, "cameraRotation entry");
        android.hardware.Camera.CameraInfo info =
                new android.hardware.Camera.CameraInfo();
        android.hardware.Camera.getCameraInfo(mOpenedCameraId, info);
        if (info != null) {
            switch (mOrient) {
            case ROTATION_0:
                Logger.e(TAG, "ROTATION_0");
                if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                    mOutgoingVideoPlayer
                            .setOrientation(Orientation.ROTATE_90_CCW);
                } else {
                    mOutgoingVideoPlayer
                            .setOrientation(Orientation.ROTATE_90_CW);
                }
                mSwitchCamerImageView.setRotation(0);
                break;
            case ROTATION_90:
                Logger.e(TAG, "ROTATION_90");
                if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                    mOutgoingVideoPlayer
                            .setOrientation(Orientation.ROTATE_90_CCW);
                } else {
                    mOutgoingVideoPlayer
                            .setOrientation(Orientation.ROTATE_90_CW);
                }
                mSwitchCamerImageView.setRotation(270);
                break;
            case ROTATION_180:
                Logger.e(TAG, "ROTATION_180");
                if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                    mOutgoingVideoPlayer
                            .setOrientation(Orientation.ROTATE_90_CCW);
                } else {
                    mOutgoingVideoPlayer
                            .setOrientation(Orientation.ROTATE_90_CW);
                }
                mSwitchCamerImageView.setRotation(180);
                break;
            case ROTATION_270:
                Logger.e(TAG, "ROTATION_270");
                if (info.facing == Camera.CameraInfo.CAMERA_FACING_FRONT) {
                    mOutgoingVideoPlayer
                            .setOrientation(Orientation.ROTATE_90_CCW);
                } else {
                    mOutgoingVideoPlayer
                            .setOrientation(Orientation.ROTATE_90_CW);
                }
                mSwitchCamerImageView.setRotation(90);
                break;
            default:
            }
        }
    }
    /**
     * Stop camera preview.
     */
    private void stopCameraPreview() {
        Logger.v(TAG, "stopCameraPreview()");
        // Release the camera
        synchronized (mLock) {
            if (mCamera != null) {
                mCamera.setPreviewCallback(null);
                mCamera.stopPreview();
                mCamera.release();
                mCamera = null;
                mCameraPreviewRunning = false;
            } else {
                Logger.v(TAG, "mCamera is null, so do not release");
            }
        }
    }
    /**
     * Removes the incoming video sharing vews.
     */
    private void removeIncomingVideoSharingVews() {
        Logger.v(TAG, "removeIncomingVideoSharingVews");
        if (mIncomingDisplayArea != null) {
            mIncomingDisplayArea
                    .removeView(mIncomingRemoteVideoSurfaceView);
            mIncomingDisplayArea
                    .removeView(mIncomingLocalVideoSurfaceView);
            mIncomingDisplayArea.removeView(mSwitchCamerImageView);
        } else {
            Logger.w(TAG, "mIncomingDisplayArea is null");
        }
    }
    /**
     * Removes the incoming video sharing remote vews.
     */
    private void removeIncomingVideoSharingRemoteVews() {
        Logger.v(TAG, "removeIncomingVideoSharingRemoteVews");
        if (mIncomingDisplayArea != null) {
            mIncomingDisplayArea
                    .removeView(mIncomingRemoteVideoSurfaceView);
            mIncomingDisplayArea
                    .removeView(mEndReceiverSessionImageView);
        } else {
            Logger.w(TAG, "mIncomingDisplayArea is null");
        }
    }
    /**
     * Removes the incoming video sharing local vews.
     */
    private void removeIncomingVideoSharingLocalVews() {
        Logger.v(TAG, "removeIncomingVideoSharingLocalVews");
        if (mIncomingDisplayArea != null) {
            mIncomingDisplayArea
                    .removeView(mIncomingLocalVideoSurfaceView);
            mIncomingDisplayArea.removeView(mSwitchCamerImageView);
            mIncomingDisplayArea
                    .removeView(mEndReceiverSessionImageView);
        } else {
            Logger.w(TAG, "mIncomingDisplayArea is null");
        }
    }
    /**
     * Removes the veiws at sender terminated by remote.
     */
    private void removeVeiwsAtSenderTerminatedByRemote() {
        Logger.v(TAG, "removeVeiwsAtSenderTerminatedByRemote");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOutgoingDisplayArea != null) {
                    Logger.v(TAG,
                            "removeVeiwsAtSenderTerminatedByRemote" +
                            ", outgoing is not null");
                    mOutgoingDisplayArea
                            .removeView(mOutgoingLocalVideoSurfaceView);
                    mOutgoingDisplayArea
                            .removeView(mSwitchCamerImageView);
                    mOutgoingDisplayArea
                            .removeView(mEndSenderSessionImageView);
                } else {
                    Logger.w(TAG,
                            "removeVeiwsAtSenderTerminatedByRemote" +
                            ", mOutgoingDisplayArea is null");
                }
                if (mIncomingDisplayArea != null) {
                    Logger.v(TAG,
                            "removeVeiwsAtSenderTerminatedByRemote" +
                            ", mincomingarea not null");
                    mIncomingDisplayArea
                            .removeView(mIncomingLocalVideoSurfaceView);
                    mIncomingDisplayArea
                            .removeView(mSwitchCamerImageView);
                    mIncomingDisplayArea
                            .removeView(mEndSenderSessionImageView);
                } else {
                    Logger.w(TAG,
                            "removeVeiwsAtSenderTerminatedByRemote" +
                            ", mIncomingDisplayArea is null");
                }
                mIsVideoSharingSender.set(false);
                mShareStatus = ShareStatus.LIVE_IN;
                mVideoSharingState
                        .set(Constants.SHARE_VIDEO_STATE_SHARING);
                if (mCallScreenHost != null) {
                    mCallScreenHost
                            .onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
                }
            }
        });
    }
    /**
     * Removes the outgoing video sharing vews.
     */
    private void removeOutgoingVideoSharingVews() {
        Logger.v(TAG, "removeOutgoingVideoSharingVews");
        if (mOutgoingDisplayArea != null) {
            mOutgoingDisplayArea
                    .removeView(mOutgoingLocalVideoSurfaceView);
            mOutgoingDisplayArea
                    .removeView(mOutgoingRemoteVideoSurfaceView);
            mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
            mOutgoingDisplayArea
                    .removeView(mEndSenderSessionImageView);
        } else {
            Logger.w(TAG, "mOutgoingDisplayArea is null");
        }
    }
    /**
     * Remove2 way outgoing video sharing vews.
     */
    private void remove2WayOutgoingVideoSharingVews() {
        Logger.v(TAG, "removeOutgoingVideoSharingVews");
        if (mOutgoingDisplayArea != null) {
            mOutgoingDisplayArea
                    .removeView(mOutgoingLocalVideoSurfaceView);
            mOutgoingDisplayArea
                    .removeView(mOutgoingRemoteVideoSurfaceView);
            mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
            mOutgoingDisplayArea
                    .removeView(mEndSenderSessionImageView);
            mOutgoingDisplayArea
                    .removeView(mEndReceiverSessionImageView);
        } else {
            Logger.w(TAG, "mOutgoingDisplayArea is null");
        }
    }
    // This is only called by the first sender.
    /**
     * Show sender local view.
     */
    private void showSenderLocalView() {
        Logger.v(TAG,
                "showSenderLocalView() entry, mOutgoingVideoFormat is: "
                        + mOutgoingVideoFormat);
        removeOutgoingVideoSharingVews();
        mOutgoingLocalVideoSurfaceView = new VideoSurfaceView(
                mContext);
        mOutgoingLocalVideoSurfaceView.setZOrderMediaOverlay(true);
        mOutgoingLocalVideoSurfaceView.mWidth = RCSeCallCardExtension.sWidth;
        int width = 0;
        int height = 0;
        if(mOutgoingDisplayArea != null)
        {
            width = mOutgoingDisplayArea.getWidth();
            height = mOutgoingDisplayArea.getHeight();
        }
        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(
        		Utils.dip2px(mContext, RCSeCallCardExtension.sWidth),
        		ViewGroup.LayoutParams.FILL_PARENT);
        LayoutParams params = new LayoutParams(
        		RCSeCallCardExtension.sWidth,
                ViewGroup.LayoutParams.MATCH_PARENT);
        mOutgoingLocalVideoSurfaceView.setLayoutParams(layoutParams);
        mOutgoingLocalVideoSurfaceView.setAspectRatio(mVideoHeight,
                mVideoWidth);
        mSurfaceHolder = mOutgoingLocalVideoSurfaceView.getHolder();
        //mSurfaceHolder
             //   .setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        mSurfaceHolder.addCallback(VideoSharingPlugin.this);
        Logger.w(TAG, "showSenderLocalView(), mOutgoingVideoFormat "
                + mOutgoingVideoFormat);
       
        //layoutParams.addRule(RelativeLayout.CENTER_IN_PARENT);
        Logger.v(TAG, "showSenderLocalView2(), width = " + RCSeCallCardExtension.sWidth
                + ",height = " + height + "mVideoWidth = "
                + mVideoWidth + ",mVideoHeight = " + mVideoHeight + "layoutparams =" + layoutParams);
        if(mOutgoingDisplayArea!= null)
        ((ViewGroup)mOutgoingDisplayArea).addView(mOutgoingLocalVideoSurfaceView,
                layoutParams);
        layoutParams = new RelativeLayout.LayoutParams(Utils.dip2px(
                mContext, SWITCH_BUTTON_WIDTH), Utils.dip2px(
                mContext, SWITCH_BUTTON_HEIGHT));
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT,
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_TOP),
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
                SWITCH_BUTTON_MARGIN_BOTTOM);
        if(mOutgoingDisplayArea!= null)
        mOutgoingDisplayArea.addView(mSwitchCamerImageView,
                layoutParams);
        //Adding terminate first sender video button
        layoutParams = new RelativeLayout.LayoutParams(Utils.dip2px(
                mContext, TERMINATE_BUTTON_WIDTH), Utils.dip2px(
                mContext, TERMINATE_BUTTON_HEIGHT));
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        layoutParams.setMargins(TERMINATE_BUTTON_MARGIN_LEFT,
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_TOP),
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_LEFT),
                SWITCH_BUTTON_MARGIN_BOTTOM);
        if(mOutgoingDisplayArea!= null)
        mOutgoingDisplayArea.addView(mEndSenderSessionImageView,
                layoutParams);
        if (mCallScreenHost != null) {
            mCallScreenHost
                    .onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
        }
    }
    // When first sender receive invitation
    /**
     * Update first sender view.
     */
    private void updateFirstSenderView() {
        Logger.v(TAG,
                "updateFirstSenderView() entry, mOutgoingVideoFormat is: "
                        + mOutgoingVideoFormat);
        remove2WayOutgoingVideoSharingVews();
        mOutgoingRemoteVideoSurfaceView = new VideoSurfaceView(
                mContext);
        mOutgoingRemoteVideoSurfaceView.isReceiver = true;
        mOutgoingRemoteVideoSurfaceView.mWidth = RCSeCallCardExtension.sWidth;
        LayoutParams params = new LayoutParams(
                Utils.dip2px(mContext, RCSeCallCardExtension.sWidth),
                ViewGroup.LayoutParams.MATCH_PARENT);
        mOutgoingRemoteVideoSurfaceView.setLayoutParams(params);
        Logger.v(TAG, "FirstSenderView ,mOutgoingVideoFormat"
                + mOutgoingVideoFormat);
        //mOutgoingRemoteVideoRenderer = new LiveVideoRenderer();
       // mOutgoingRemoteVideoRenderer
                //.setVideoSurface(mOutgoingRemoteVideoSurfaceView);
        // Remote video view
        RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(
        		Utils.dip2px(mContext, RCSeCallCardExtension.sWidth),
                ViewGroup.LayoutParams.MATCH_PARENT);
        mOutgoingDisplayArea.addView(mOutgoingRemoteVideoSurfaceView,
                layoutParams);
        Logger.v(TAG, "mOutgoingRemoteVideoSurfaceView = "
                + mOutgoingRemoteVideoSurfaceView
                + ", mOutgoingRemoteVideoSurfaceView.height = "
                + mOutgoingRemoteVideoSurfaceView.getHeight()
                + ", mOutgoingRemoteVideoSurfaceView.width = "
                + mOutgoingRemoteVideoSurfaceView.getWidth());
        // Local video view
        mOutgoingLocalVideoSurfaceView.setAspectRatio(mVideoWidth,
                mVideoHeight);
        mSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
        mOutgoingLocalVideoSurfaceView.setZOrderMediaOverlay(true);
        mOutgoingLocalVideoSurfaceView.mWidth = -1;
        layoutParams = null;
        layoutParams = new RelativeLayout.LayoutParams(Utils.dip2px(
                mContext, MIN_WINDOW_WIDTH), Utils.dip2px(mContext,
                MIN_WINDOW_HEIGHT));
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        layoutParams.setMargins(LOCAL_CAMERA_MARGIN_LEFT,
                Utils.dip2px(mContext, LOCAL_CAMERA_MARGIN_TOP),
                Utils.dip2px(mContext, LOCAL_CAMERA_MARGIN_RIGHT),
                LOCAL_CAMERA_MARGIN_BOTTOM);
        mOutgoingDisplayArea.addView(mOutgoingLocalVideoSurfaceView,
                layoutParams);
        //Adding terminate sender video button
        layoutParams = new RelativeLayout.LayoutParams(Utils.dip2px(
                mContext, TERMINATE_BUTTON_WIDTH), Utils.dip2px(
                mContext, TERMINATE_BUTTON_HEIGHT));
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT,
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_TOP),
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
                SWITCH_BUTTON_MARGIN_BOTTOM);
        mOutgoingDisplayArea.addView(mEndSenderSessionImageView,
                layoutParams);
        //Adding terminate receiver video button
        layoutParams = new RelativeLayout.LayoutParams(Utils.dip2px(
                mContext, TERMINATE_BUTTON_WIDTH), Utils.dip2px(
                mContext, TERMINATE_BUTTON_HEIGHT));
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        layoutParams.setMargins(TERMINATE_BUTTON_MARGIN_LEFT,
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_TOP),
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_LEFT),
                SWITCH_BUTTON_MARGIN_BOTTOM);
        mOutgoingDisplayArea.addView(mEndReceiverSessionImageView,
                layoutParams);
        // Switch button
        layoutParams = new RelativeLayout.LayoutParams(
                SWITCH_BUTTON_WIDTH, SWITCH_BUTTON_HEIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT,
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_TOP),
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
                SWITCH_BUTTON_MARGIN_BOTTOM);
        mOutgoingDisplayArea.addView(mSwitchCamerImageView,
                layoutParams);
        mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
        if (mCallScreenHost != null) {
            mCallScreenHost
                    .onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
        }
    }
    /**
     * Initialize receiver remote view.
     */
    private void initializeReceiverRemoteView() {
        Logger.v(TAG,
                "initializeReceiverRemoteView() entry, mIncomingVideoFormat: "
                        + mIncomingVideoFormat + "mShareStatus: "
                        + mShareStatus);
        removeIncomingVideoSharingRemoteVews();
        Logger.v(TAG,
                "initializeReceiverRemoteView ,mIncomingVideoFormat"
                        + mIncomingVideoFormat);
        if (mIncomingDisplayArea != null) {
            //mIncomingRemoteVideoRenderer = new LiveVideoRenderer();
            mIncomingRemoteVideoSurfaceView = new VideoSurfaceView(
                    mContext);
            mIncomingRemoteVideoSurfaceView.isReceiver = true;
            mIncomingRemoteVideoSurfaceView.mWidth = RCSeCallCardExtension.sWidth;
            //mIncomingRemoteVideoRenderer
                    //.setVideoSurface(mIncomingRemoteVideoSurfaceView);
            mIncomingRemoteVideoSurfaceView
                    .setVisibility(View.VISIBLE);
            mDummyImageView = new ImageView(mContext);
            //mSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
            //mIncomingRemoteVideoSurfaceView.setZOrderMediaOverlay(true);
            LayoutParams params = new LayoutParams(
                    Utils.dip2px(mContext, RCSeCallCardExtension.sWidth),
                    ViewGroup.LayoutParams.MATCH_PARENT);
            mIncomingRemoteVideoSurfaceView.setLayoutParams(params);
            mIncomingRemoteVideoSurfaceView.getHolder().setFormat(
                    PixelFormat.OPAQUE);
        } else {
            Logger.w(TAG, "mIncomingDisplayArea is null");
        }
       /* if (mOutgoingDisplayArea != null) {
            mOutgoingDisplayArea
                    .removeView(mOutgoingLocalVideoSurfaceView);
        } else {
            Logger.w(TAG, "mOutgoingDisplayArea is null");
        }*/
    }
    
    public void recreateReceiverSurfaceView(){
        removeIncomingVideoSharingRemoteVews();
        if (mIncomingDisplayArea != null) {            
            mIncomingRemoteVideoSurfaceView = new VideoSurfaceView(
                    mContext);
            mIncomingRemoteVideoSurfaceView.isReceiver = true;
            mIncomingRemoteVideoSurfaceView.mWidth = RCSeCallCardExtension.sWidth;
           // mIncomingRemoteVideoRenderer
                   // .setVideoSurface(mIncomingRemoteVideoSurfaceView);
            mIncomingRemoteVideoSurfaceView
                    .setVisibility(View.VISIBLE);
            mDummyImageView = new ImageView(mContext);
            //mSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
            //mIncomingRemoteVideoSurfaceView.setZOrderMediaOverlay(true);
            LayoutParams params = new LayoutParams(
                    Utils.dip2px(mContext, RCSeCallCardExtension.sWidth),
                    ViewGroup.LayoutParams.MATCH_PARENT);
            mIncomingRemoteVideoSurfaceView.setLayoutParams(params);
            mIncomingRemoteVideoSurfaceView.getHolder().setFormat(
                    PixelFormat.OPAQUE);
        }
        showReceiverRemoteView();
    }
    // When receive video share invitation
    /**
     * Show receiver remote view.
     */
    private void showReceiverRemoteView() {
        Logger.v(TAG, "showReceiverRemoteView(), mShareStatus: "
                + mShareStatus);
        if (mIncomingDisplayArea != null) {
            RelativeLayout.LayoutParams layoutParams = null;
            Logger.d(TAG, "showReceiverRemoteView, is live show!");
            layoutParams = new RelativeLayout.LayoutParams(
            		Utils.dip2px(
                            mContext, RCSeCallCardExtension.sWidth),
                    ViewGroup.LayoutParams.MATCH_PARENT);
            mIncomingDisplayArea.addView(
                    mIncomingRemoteVideoSurfaceView, layoutParams);
            mIncomingRemoteVideoSurfaceView.setAspectRatio(
                    mVideoHeight, mVideoWidth);
            //Adding terminate button to first receiver of video
            layoutParams = new RelativeLayout.LayoutParams(
                    Utils.dip2px(mContext, TERMINATE_BUTTON_WIDTH),
                    Utils.dip2px(mContext, TERMINATE_BUTTON_HEIGHT));
            layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
            layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
            layoutParams
                    .setMargins(TERMINATE_BUTTON_MARGIN_LEFT, Utils
                            .dip2px(mContext,
                                    SWITCH_BUTTON_MARGIN_TOP), Utils
                            .dip2px(mContext,
                                    SWITCH_BUTTON_MARGIN_LEFT),
                            SWITCH_BUTTON_MARGIN_BOTTOM);
            mIncomingDisplayArea.addView(
                    mEndReceiverSessionImageView, layoutParams);
            Logger.v(TAG, "mIncomingRemoteVideoSurfaceView = "
                    + mIncomingRemoteVideoSurfaceView
                    + ", mIncomingRemoteVideoSurfaceView.height = "
                    + mIncomingRemoteVideoSurfaceView.getHeight()
                    + ", mIncomingRemoteVideoSurfaceView.width = "
                    + mIncomingRemoteVideoSurfaceView.getWidth());
            mVideoSharingState
                    .set(Constants.SHARE_VIDEO_STATE_SHARING);
            if (mCallScreenHost != null) {
                mCallScreenHost
                        .onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
            }
        } else {
            Logger.w(TAG, "mIncomingDisplayArea is null");
        }
    }
    // When the receiver send invitation, this method will be called
    /**
     * Show receiver local view.
     */
    private void showReceiverLocalView() {
        Logger.v(TAG,
                "showReceiverLocalView() entry, mIncomingVideoFormat is: "
                        + mIncomingVideoFormat);
        removeIncomingVideoSharingLocalVews();
        Logger.v(TAG, "showReceiverLocalView ,mIncomingVideoFormat"
                + mIncomingVideoFormat);
        mIncomingLocalVideoSurfaceView = new VideoSurfaceView(
                mContext);
        mIncomingLocalVideoSurfaceView.setAspectRatio(mVideoWidth,
                mVideoHeight);
        mSurfaceHolder = mIncomingLocalVideoSurfaceView.getHolder();
        mSurfaceHolder.addCallback(VideoSharingPlugin.this);
        mSurfaceHolder.setFormat(PixelFormat.TRANSPARENT);
        if(mIncomingRemoteVideoSurfaceView !=null)
        {
        mIncomingRemoteVideoSurfaceView.setZOrderMediaOverlay(false);
        }
        mIncomingLocalVideoSurfaceView.setZOrderMediaOverlay(true);
        RelativeLayout.LayoutParams layoutParams = null;
        // Local video view
        layoutParams = new RelativeLayout.LayoutParams(Utils.dip2px(
                mContext, MIN_WINDOW_WIDTH), Utils.dip2px(mContext,
                MIN_WINDOW_HEIGHT));
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        layoutParams.setMargins(LOCAL_CAMERA_MARGIN_LEFT,
                Utils.dip2px(mContext, LOCAL_CAMERA_MARGIN_TOP),
                Utils.dip2px(mContext, LOCAL_CAMERA_MARGIN_RIGHT),
                LOCAL_CAMERA_MARGIN_BOTTOM);
        mIncomingDisplayArea.addView(mIncomingLocalVideoSurfaceView,
                layoutParams);
        //Adding terminate button to receiver when sending the video, will end send video
        layoutParams = new RelativeLayout.LayoutParams(Utils.dip2px(
                mContext, TERMINATE_BUTTON_WIDTH), Utils.dip2px(
                mContext, TERMINATE_BUTTON_HEIGHT));
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT,
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_TOP),
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
                SWITCH_BUTTON_MARGIN_BOTTOM);
        mIncomingDisplayArea.addView(mEndSenderSessionImageView,
                layoutParams);
        //Adding terminate button to receiver when receiving the video, will end send video
        layoutParams = new RelativeLayout.LayoutParams(Utils.dip2px(
                mContext, TERMINATE_BUTTON_WIDTH), Utils.dip2px(
                mContext, TERMINATE_BUTTON_HEIGHT));
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        layoutParams.setMargins(TERMINATE_BUTTON_MARGIN_LEFT,
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_TOP),
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_LEFT),
                SWITCH_BUTTON_MARGIN_BOTTOM);
        mIncomingDisplayArea.addView(mEndReceiverSessionImageView,
                layoutParams);
        // switch button
        layoutParams = new RelativeLayout.LayoutParams(
                SWITCH_BUTTON_WIDTH, SWITCH_BUTTON_HEIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        layoutParams.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        layoutParams.setMargins(SWITCH_BUTTON_MARGIN_LEFT,
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_TOP),
                Utils.dip2px(mContext, SWITCH_BUTTON_MARGIN_RIGHT),
                SWITCH_BUTTON_MARGIN_BOTTOM);
        mIncomingDisplayArea.addView(mSwitchCamerImageView,
                layoutParams);
        mVideoSharingState.set(Constants.SHARE_VIDEO_STATE_SHARING);
        if (mCallScreenHost != null) {
            mCallScreenHost
                    .onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
        }
        if (mOutgoingDisplayArea != null) {
            mOutgoingDisplayArea
                    .removeView(mOutgoingLocalVideoSurfaceView);
        } else {
            Logger.w(TAG, "mOutgoingDisplayArea is null");
        }
    }
    /**
     * Show remote accept message.
     */
    private void showRemoteAcceptMessage() {
        Logger.v(TAG, "showRemoteAcceptMessage()");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                String message = mContext.getResources().getString(
                        R.string.remote_has_accepted);
                showToast(message);
                mVideoSharingState
                        .set(Constants.SHARE_VIDEO_STATE_SHARING);
                if (mCallScreenHost != null) {
                    mCallScreenHost
                            .onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
                }
                Logger.d(TAG,
                        "showRemoteAcceptMessage(), is live video share.");
            }
        });
    }
    /**
     * Show wait remote accept message.
     */
    private void showWaitRemoteAcceptMessage() {
        Logger.v(TAG, "showWaitRemoteAcceptMessage()");
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                String message = mContext.getResources().getString(
                        R.string.wait_remote_to_accept);
                showToast(message);
            }
        };
        mMainHandler.postDelayed(runnable, DELAY_TIME);
    }
    /**
     * Reset atomic bolean.
     */
    private void resetAtomicBolean() {
        Logger.v(TAG, "resetAtomicBolean()");
        mIsVideoSharingSender.set(false);
        mIsVideoSharingReceiver.set(false);
        mIsStarted.set(false);
    }
    /**
     * When receiving stream is terminated by receiver in two way , from the local End.
     */
    private void finishReceiverTwoWayLocal() {
        Logger.v(TAG, "finishReceiverTwoWayLocal() entry");
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.v(TAG, "destroy in a background thread.");
                if (mIncomingVideoSharingSession != null) {
                    try {
                        mVideoSharingApi
                                .removeEventListener(mIncomingSessionEventListener);
                        mIncomingVideoSharingSession.abortSharing();
                    } catch (RcsServiceException e) {
                        e.printStackTrace();
                    } finally {
                        mIncomingVideoSharingSession = null;
                    }
                }
                return null;
            }
        } .execute();
        if (mIncomingDisplayArea != null) {
            Logger.v(TAG,
                    "finishReceiverTwoWayLocal() mIncomingDisplayArea Entry");
            mIncomingDisplayArea
                    .removeView(mIncomingLocalVideoSurfaceView);
            mIncomingDisplayArea
                    .removeView(mIncomingRemoteVideoSurfaceView);
            mIncomingDisplayArea.removeView(mSwitchCamerImageView);
            mIncomingDisplayArea
                    .removeView(mEndReceiverSessionImageView);
            mIncomingDisplayArea
                    .removeView(mEndSenderSessionImageView);
        } else {
            Logger.w(TAG,
                    " finishReceiverTwoWayLocal mIncomingDisplayArea is null");
        }
        if (mOutgoingDisplayArea != null) {
            Logger.w(TAG,
                    " finishReceiverTwoWayLocal mOutgoingDisplayArea entry");
            mOutgoingDisplayArea
                    .removeView(mOutgoingLocalVideoSurfaceView);
            mOutgoingDisplayArea
                    .removeView(mOutgoingRemoteVideoSurfaceView);
            mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
            mOutgoingDisplayArea
                    .removeView(mEndSenderSessionImageView);
            mOutgoingDisplayArea
                    .removeView(mEndReceiverSessionImageView);
        } else {
            Logger.w(TAG,
                    " finishReceiverTwoWayLocal mOutgoingDisplayArea is null");
        }
        mIsVideoSharingReceiver.set(false);
        showSenderLocalView();
    }
    /**
     * Finish sender two waylocal.
     */
    public void finishSenderTwoWaylocal() {
        Logger.v(TAG, "finishSenderTwoWaylocal() entry");
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.v(TAG, "destroy in a background thread.");
                stopCameraPreview();
                if (mOutgoingVideoPlayer != null) {
                    mOutgoingVideoPlayer.stop();
                    mOutgoingVideoPlayer = null;
                }
                if (mOutgoingVideoSharingSession != null) {
                    try {
                        mVideoSharingApi
                                .removeEventListener(mOutgoingSessionEventListener);
                        mOutgoingVideoSharingSession.abortSharing();
                    } catch (RcsServiceException e) {
                        e.printStackTrace();
                    } finally {
                        mOutgoingVideoSharingSession = null;
                    }
                }
                return null;
            }
        } .execute();
        if (mIncomingDisplayArea != null) {
            Logger.v(TAG,
                    "finishSenderTwoWaylocal() mIncomingDisplayArea Entry");
            mIncomingDisplayArea
                    .removeView(mIncomingLocalVideoSurfaceView);
            mIncomingDisplayArea.removeView(mSwitchCamerImageView);
            mIncomingDisplayArea
                    .removeView(mEndSenderSessionImageView);
        } else {
            Logger.w(TAG,
                    " finishSenderTwoWaylocal mIncomingDisplayArea is null");
        }
        if (mOutgoingDisplayArea != null) {
            Logger.v(TAG,
                    "finishSenderTwoWaylocal() mOutgoingDisplayArea Entry");
            mOutgoingDisplayArea
                    .removeView(mOutgoingLocalVideoSurfaceView);
            mOutgoingDisplayArea.removeView(mSwitchCamerImageView);
            mOutgoingDisplayArea
                    .removeView(mEndSenderSessionImageView);
        } else {
            Logger.w(TAG,
                    " finishSenderTwoWaylocal mOutgoingDisplayArea is null");
        }
        mIsVideoSharingSender.set(false);
        if (mCallScreenHost != null) {
            mCallScreenHost
                    .onStateChange(Constants.SHARE_VIDEO_STATE_SHARING);
        }
    }
    /**
     * Release resource. Please call me at any thread including UI thread
     */
    private void destroy() {
        Logger.v(TAG, "destroy() entry");
        new AsyncTask<Void, Void, Void>() {
            @Override
            protected Void doInBackground(Void... params) {
                Logger.v(TAG, "destroy in a background thread.");
                resetAtomicBolean();
                // fix NE
                stopCameraPreview();
                if (mOutgoingVideoPlayer != null) {
                    mOutgoingVideoPlayer.stop();
                    mOutgoingVideoPlayer = null;
                }
                cancelSession();
                return null;
            }
        } .execute();
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                removeIncomingVideoSharingVews();
                removeOutgoingVideoSharingVews();
                dismissAllDialogs();
                clearSavedDialogs();
                RCSeInCallUIExtension.getInstance()
                        .resetDisplayArea();
                if (mSurfaceHolder != null) {
                    mSurfaceHolder
                            .removeCallback(VideoSharingPlugin.this);
                } else {
                    Logger.w(TAG, "mSurfaceHolder is null");
                }
            }
        });
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
            Logger.v(TAG, "when destroy, release the wake lock");
        }
        resetVideoShaingState();
        Logger.d(TAG, "destroy() exit");
    }
    /**
     * Cancel session.
     *
     * @return true, if successful
     */
    private boolean cancelSession() {
        // Should make sure cancelSession run a different thread with
        // RemoteCallback
        Logger.d(TAG, "cancelSession()");
        if (mOutgoingVideoSharingSession != null) {
            try {
                mVideoSharingApi
                        .removeEventListener(mOutgoingSessionEventListener);
                mOutgoingVideoSharingSession.abortSharing();
            } catch (RcsServiceException e) {
                e.printStackTrace();
            } finally {
                mOutgoingVideoSharingSession = null;
            }
        }
        if (mIncomingVideoSharingSession != null) {
            try {
                mVideoSharingApi
                        .removeEventListener(mIncomingSessionEventListener);
                mIncomingVideoSharingSession.abortSharing();
            } catch (RcsServiceException e) {
                e.printStackTrace();
            } finally {
                mIncomingVideoSharingSession = null;
            }
        }
        return true;
    }
    /**
     * Destroy incoming session only.
     */
    private void destroyIncomingSessionOnly() {
        Logger.v(TAG, "destroyIncomingSession()");
        if (mIncomingVideoSharingSession != null) {
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    try {
                        Logger.v(TAG,
                                "reject session in backgroun thread.");
                        mVideoSharingApi
                                .removeEventListener(mIncomingSessionEventListener);
                        mIncomingVideoSharingSession
                                .rejectInvitation();
                    } catch (RcsServiceException e) {
                        e.printStackTrace();
                    } finally {
                        mIncomingVideoSharingSession = null;
                        mIsVideoSharingReceiver.set(false);
                    }
                }
            });
        }
    }
    // Sometime a receiver send a viedo sharing invitation may be failed.
    // This method run on ui thread, so you can call it in any thread
    /**
     * Destroy outgoing view only.
     */
    private void destroyOutgoingViewOnly() {
        Logger.v(TAG, "destroyOutgoingViewOnly()");
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mIncomingDisplayArea != null) {
                    mIncomingDisplayArea
                            .removeView(mIncomingLocalVideoSurfaceView);
                    mIncomingDisplayArea
                            .removeView(mSwitchCamerImageView);
                } else {
                    Logger.w(TAG, "mIncomingDisplayArea is null.");
                }
            }
        });
    }
    /**
     * Reset video shaing state.
     */
    private void resetVideoShaingState() {
        mMainHandler.post(new Runnable() {
            @Override
            public void run() {
                Logger.v(TAG, "resetVideoShaingState");
                mVideoSharingState
                        .set(Constants.SHARE_VIDEO_STATE_IDLE);
                Utils.setInVideoSharing(false);
                mShareStatus = ShareStatus.UNKNOWN;
                if (mCallScreenHost != null) {
                    mCallScreenHost
                            .onStateChange(Constants.SHARE_VIDEO_STATE_IDLE);
                } else {
                    Logger.w(TAG, "mCallScreenHost is null");
                }
            }
        });
    }
    /**
     * Rotate camera.
     */
    private void rotateCamera() {
        Logger.v(TAG, "rotateCamera");
        // release camera
        stopCameraPreview();
        // open the other camera
        if (mOpenedCameraId == CameraInfo.CAMERA_FACING_FRONT) {
            mCamera = Camera.open(CameraInfo.CAMERA_FACING_FRONT);
        } else {
            mCamera = Camera.open(CameraInfo.CAMERA_FACING_BACK);
        }
        // restart the preview
        startCameraPreview();
        addPreviewCallback();
    }
    /**
     * Switch camera.
     */
    private void switchCamera() {
        Logger.v(TAG, "switchCamera");
        if (mCamerNumber == SINGLE_CAMERA) {
            Logger.w(TAG,
                    "The device only has one camera, so can not switch");
            return;
        }
        // release camera
        stopCameraPreview();
        // open the other camera
        if (mOpenedCameraId == CameraInfo.CAMERA_FACING_BACK) {
            mCamera = Camera.open(CameraInfo.CAMERA_FACING_FRONT);
            mOpenedCameraId = CameraInfo.CAMERA_FACING_FRONT;
        } else {
            mCamera = Camera.open(CameraInfo.CAMERA_FACING_BACK);
            mOpenedCameraId = CameraInfo.CAMERA_FACING_BACK;
        }
        startCameraPreview();
        addPreviewCallback();
    }
    /**
     * Open default camera.
     */
    private void openDefaultCamera() {
        Logger.v(TAG, "openDefaultCamera");
        mCamera = Camera.open();
        mOpenedCameraId = CameraInfo.CAMERA_FACING_BACK;
    }

    /**
     * Video sharing invitation receiver.
     */
    private class VideoSharingInvitationReceiver extends
            BroadcastReceiver {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "VideoSharingInvitationReceiver";

        /* (non-Javadoc)
         * @see android.content.BroadcastReceiver#
         * onReceive(android.content.Context, android.content.Intent)
         */
        @Override
        public void onReceive(Context context, Intent intent) {
            Logger.v(TAG, "onReceive(), context = " + context
                    + ", intent = " + intent);
            if (intent != null) {
                String action = intent.getAction();
                Logger.v(TAG, "action = " + action);
                if (VideoSharingIntent.ACTION_NEW_INVITATION
                        .equals(action)) {
                    //initAudioButton();
                    String sharing_Id = intent.getStringExtra(VideoSharingIntent.EXTRA_SHARING_ID);
                    if(sharing_Id.equals(mlast_sharing_Id)){
                        Logger.v(TAG, "onReceive(), sharing id matches = " + sharing_Id
                                + ", mlast_sharing_Id = " + mlast_sharing_Id);
                        return;
                    }
                    Logger.v(TAG, "onReceive(), sharing id = " + sharing_Id);
                    mlast_sharing_Id = sharing_Id;
                    handleVideoSharingInvitation(context, intent);
                } else if (intent.ACTION_HEADSET_PLUG.equals(action)) {
                    mHeadsetConnected = (intent.getIntExtra("state",
                            0) == 1);
                    Logger.d(TAG, "onReceive() ACTION_HEADSET_PLUG: "
                            + mHeadsetConnected);
                } else if (BluetoothHeadset.ACTION_CONNECTION_STATE_CHANGED
                        .equals(action)) {
                    int state = intent.getIntExtra(
                            BluetoothHeadset.EXTRA_STATE,
                            BluetoothHeadset.STATE_DISCONNECTED);
                    Logger.d(TAG,
                            "onReceive() ACTION_CONNECTION_STATE_CHANGED: "
                                    + state);
                } else {
                    Logger.w(TAG, "Unknown action");
                }
            } else {
                Logger.w(TAG, "intent is null");
            }
        }
    } // end VideoSharingInvitationReceiver

    /**
     * Handle video sharing invitation.
     *
     * @param context the context
     * @param intent the intent
     */
    private void handleVideoSharingInvitation(Context context,
            Intent intent) {
        Logger.v(TAG, "handleVideoSharingInvitation()");
        String mIncomingContact = "";/*intent
                .getStringExtra(VideoSharingIntent.EXTRA_CONTACT)*/// ToDo
        // Display invitation dialog
        mIncomingSessionId = intent
                .getStringExtra(VideoSharingIntent.EXTRA_SHARING_ID);
        mIncomingVideoFormat = ""; /*intent
                .getStringExtra(VideoSharingIntent.EXTRA_ENCODING);*///ToDo
        mVideoDuration = intent.getLongExtra("videosize", -1);
        Logger.v(TAG,
                "handleVideoSharingInvitation Received Duration is()"
                        + mVideoDuration + "number is"
                        + mIncomingContact + " & mNumber is"
                        + sNumber);
        String mediaType = intent
                .getStringExtra(RichcallProxyActivity.MEDIA_TYPE);
        // Video share is of full status.
        boolean canShare = (mShareStatus == ShareStatus.UNKNOWN ||
                mShareStatus == ShareStatus.LIVE_OUT);
        boolean supported = true;
        supported = getCapability(mIncomingContact);
        Logger.d(TAG, "handleVideoSharingInvitation() mShareStatus: "
                + mShareStatus + " supported: " + supported);
        if (!canShare || !supported) {
            try {
                VideoSharing sharingSession = mVideoSharingApi
                        .getVideoSharing(mIncomingSessionId);
                if (sharingSession != null) {
                    try {
                        sharingSession.rejectInvitation();
                    } catch (RcsServiceException e) {
                        e.printStackTrace();
                    }
                }
            } catch (RcsServiceException e) {
                e.printStackTrace();
            }
            return;
        }
        if (true) {
            if (mShareStatus == ShareStatus.LIVE_OUT) {
                if (is3GMobileNetwork()
                        && mOutgoingVideoSharingSession != null) {
                    try {
                        if (mOutgoingVideoSharingSession.getState() == VideoSharing.State.STARTED
                                || mOutgoingVideoSharingSession
                                        .getState() == VideoSharing.State.INVITED) {
                            Logger.d(
                                    TAG,
                                    "Reject the incoming session because the device "
                                            + "is upder 3G network and the outgoing video share "
                                            + "session is established or on pending");
                            try {
                                VideoSharing sharingSession = mVideoSharingApi
                                        .getVideoSharing(mIncomingSessionId);
                                if (sharingSession != null) {
                                    try {
                                        sharingSession
                                                .rejectInvitation();
                                    } catch (RcsServiceException e) {
                                        e.printStackTrace();
                                    }
                                }
                            } catch (RcsServiceException e) {
                                e.printStackTrace();
                            }
                            return;
                        }
                    } catch (RcsServiceException e) {
                        Logger.e(TAG, e.toString());
                    }
                } else {
                    mShareStatus = ShareStatus.LIVE_TWOWAY;
                }
            } else {
                mShareStatus = ShareStatus.LIVE_IN;
            }
        }
        // Tell that it is vs receiver if recevie a vs invitation before
        // start vs
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
            Logger.v(TAG, "when start, acquire a wake lock");
        } else {
            Logger.v(TAG,
                    "when start, the wake lock has been acquired, so do not acquire");
        }
        mIsVideoSharingReceiver.set(true);
        if (RCSeInCallUIExtension.getInstance().getmShareFilePlugIn()
				.getCurrentState() == Constants.SHARE_FILE_STATE_DISPLAYING || RCSeInCallUIExtension.getInstance().getmShareFilePlugIn()
                .getCurrentState() == Constants.SHARE_FILE_STATE_IDLE) {
			RCSeInCallUIExtension.getInstance().getmShareFilePlugIn().stop();
			Logger.v(TAG,
                    "Start SHARE_FILE_STATE_DISPLAYING");
        }
        // Set application context
        MediatekFactory.setApplicationContext(context);
        if (mRichCallStatus == RichCallStatus.CONNECTED) {
            // Initialize a vs session
            mGetIncomingSessionWhenApiConnected.set(false);
            getIncomingVideoSharingSession();
        } else {
            Logger.v(TAG,
                    "Richcall api not connected, and then connect api");
            mGetIncomingSessionWhenApiConnected.set(true);
            try {
                mVideoSharingApi.connect();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
    }
    /**
     * Checks if is 3 g mobile network.
     *
     * @return true, if is 3 g mobile network
     */
    protected boolean is3GMobileNetwork() {
        int networkType = Utils.getNetworkType(mContext);
        boolean is3G = false;
        if (networkType == Utils.NETWORK_TYPE_UMTS
                || networkType == Utils.NETWORK_TYPE_EDGE
                || networkType == Utils.NETWORK_TYPE_HSUPA
                || networkType == Utils.NETWORK_TYPE_HSDPA
                || networkType == Utils.NETWORK_TYPE_1XRTT
                || networkType == Utils.NETWORK_TYPE_EHRPD) {
            is3G = true;
        }
        return is3G;
    }
    /**
     * Handle user accept video sharing.
     */
    private void handleUserAcceptVideoSharing() {
        Logger.v(TAG, "handleUserAcceptVideoSharing()");
        if (mCallScreenHost != null) {
            mIncomingDisplayArea = mCallScreenHost
                    .requestAreaForDisplay();
        } else {
            Logger.w(TAG, "mCallScreenHost is null");
        }
        if (mIncomingVideoSharingSession != null) {
            Utils.setInVideoSharing(true);
            if (mIsVideoSharingSender.get()) {
                // Receive vs invitation after invitation other
                Logger.v(TAG,
                        "Receive vs invitation after invitation other");
                handleSecondReceiveInvitation();
            } else if (mIsVideoSharingReceiver.get()) {
                // First receive vs invitation and accept
                Logger.v(TAG,
                        "First receive vs invitation and accept");
                handleFirstReceiveInvitation();
            }
            // Put the screen on wake while video share
            if(RCSeInCallUIExtension.getInstance().mActivity != null) {
                RCSeInCallUIExtension.getInstance().mActivity.getWindow()
                    .addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                RCSeInCallUIExtension.getInstance().mActivity.convertToTranslucent(null, null);
            }
        } else {
            Logger.w(TAG, "mIncomingVideoSharingSession is null");
        }
    }
    /**
     * Handle user decline video sharing.
     */
    private void handleUserDeclineVideoSharing() {
        Logger.v(TAG,
                "handleUserDeclineVideoSharing() mShareStatus: "
                        + mShareStatus);
        if (mWakeLock.isHeld()) {
            mWakeLock.release();
            Logger.v(TAG,
                    "handleUserDeclineVideoSharing() release wake lock");
        } else {
            Logger.v(TAG,
                    "handleUserDeclineVideoSharing() no need to release wake lock");
        }
        // status here can be: LIVE_TWOWAY, LIVE_IN.
        if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
            mShareStatus = ShareStatus.LIVE_OUT;
        } else {
            mShareStatus = ShareStatus.UNKNOWN;
        }
        if (mIncomingVideoSharingSession != null) {
            // In such case, only need remove incoming ui
            destroyIncomingSessionOnly();
        } else {
            Logger.w(TAG, "vmIncomingVideoSharingSession is null");
        }
    }
    // Receive invitaiton before send invitation
    /**
     * Handle first receive invitation.
     */
    private void handleFirstReceiveInvitation() {
        Logger.v(TAG, "handleFirstReceiveInvitation");
        try {
            initializeReceiverRemoteView();
            mIncomingVideoSharingSession
                    .acceptInvitation(mOutgoingVideoPlayer);
            mVideoSharingDialogManager
                    .showWaitingInitializeConextProgressDialog();
            showReceiverRemoteView();
        } catch (RcsServiceException e) {
            e.printStackTrace();
        }
    }
    // Receive invitation after send invitation
    /**
     * Handle second receive invitation.
     */
    private void handleSecondReceiveInvitation() {
        Logger.v(TAG, "handleSecondReceiveInvitation");
        try {
            Logger.v(TAG, "Receive invitation after send invitation");
            mVideoSharingDialogManager
                    .showWaitingInitializeConextProgressDialog();
            updateFirstSenderView();
            mIncomingVideoSharingSession
                    .acceptInvitation(mOutgoingVideoPlayer);
        } catch (RcsServiceException e) {
            e.printStackTrace();
        }
    }
    /* (non-Javadoc)
     * @see android.content.DialogInterface.OnClickListener#
     * onClick(android.content.DialogInterface, int)
     */
    @Override
    public void onClick(DialogInterface dialog, int which) {
        Logger.v(TAG, "onClick(), which = " + which);
        switch (which) {
        case DialogInterface.BUTTON_POSITIVE:
            handleUserAcceptVideoSharing();
            break;
        case DialogInterface.BUTTON_NEGATIVE:
            handleUserDeclineVideoSharing();
            break;
        default:
            Logger.v(TAG, "Unknown option");
            break;
        }
    }
    /**
     * Gets the incoming video sharing session.
     *
     * @return the incoming video sharing session
     */
    private void getIncomingVideoSharingSession() {
        Logger.v(TAG, "getIncomingVideoSharingSession()");
        try {
            mIncomingVideoSharingSession = mVideoSharingApi
                    .getVideoSharing(mIncomingSessionId);
            if (mIncomingVideoSharingSession != null) {
                mIncomingSessionEventListener = new IncomingSessionEventListener();
                mVideoSharingApi
                        .addEventListener(mIncomingSessionEventListener);
            } else {
                Logger.w(TAG, "mIncomingVideoSharingSession is null");
            }
        } catch (RcsServiceException e) {
            e.printStackTrace();
        }
        mVideoSharingDialogManager.showInvitationDialog();
    }

    /**
     * Wait to receive live video from remote Progress dialog.
     */
    public static class WaitingProgressDialog extends DialogFragment {
        /**
         * The Constant TAG.
         */
        public static final String TAG = "WaitingProgressDialog";
        /**
         * The s context.
         */
        private static Context sContext = null;
        /**
         * The s activity.
         */
        private static Activity sActivity = null;

        /**
         * New instance.
         *
         * @param context the context
         * @param activity the activity
         * @return the waiting progress dialog
         */
        public static WaitingProgressDialog newInstance(
                Context context, Activity activity) {
            WaitingProgressDialog f = new WaitingProgressDialog();
            sContext = context;
            sActivity = activity;
            return f;
        }
        /* (non-Javadoc)
         * @see android.app.DialogFragment#onCreateDialog(android.os.Bundle)
         */
        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Logger.v(TAG, "onCreateDialog()");
            ProgressDialog dialog = new ProgressDialog(sActivity);
            dialog.setIndeterminate(true);
            dialog.setMessage(sContext
                    .getString(R.string.wait_for_video));
            dialog.setCanceledOnTouchOutside(false);
            return dialog;
        }
        /* (non-Javadoc)
         * @see android.app.DialogFragment#onDismiss(android.content.DialogInterface)
         */
        @Override
        public void onDismiss(DialogInterface dialog) {
            super.onDismiss(dialog);
        }
    }

    /**
     * The Class VideoSharingDialogManager.
     */
    private class VideoSharingDialogManager {
        /**
         * The Constant TAG.
         */
        private static final String TAG = "VideoSharingDialogManager";
        /**
         * The m on click listener.
         */
        private DialogInterface.OnClickListener mOnClickListener =
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Logger.v(TAG, "onClick(), which = " + which);
                switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    Logger.v(TAG, "onClick() stop video sharing");
                    destroy();
                    break;
                case DialogInterface.BUTTON_NEGATIVE:
                    Logger.v(TAG, "onClick() Do nothing");
                    break;
                default:
                    Logger.v(TAG, "onClick() Unknown option");
                    break;
                }
                dismissDialog();
                clearSavedDialogs();
            }
        };

        /**
         * Show terminated by remote dialog.
         */
        public void showTerminatedByRemoteDialog() {
            Logger.v(TAG, "showTerminatedByRemoteDialog() entry");
            destroy();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    dismissAllDialogs();
                    clearSavedDialogs();
                    String msg = mContext
                            .getString(R.string.remote_terminated);
                    Logger.v(TAG,
                            "showTerminatedByRemoteDialog msg = "
                                    + msg);
                    createAndShowAlertDialog(msg,
                            mTerminatedDialogListener);
                }
            });
            Logger.v(TAG, "showTerminatedByRemoteDialog() exit");
        }

        /**
         * The m terminated dialog listener.
         */
        DialogInterface.OnClickListener mTerminatedDialogListener =
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Logger.v(TAG, "mTerminatedDialogListener, which = "
                        + which);
                switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    Logger.v(TAG, "remvoe all Dialogs");
                    break;
                case DialogInterface.BUTTON_NEGATIVE:
                    Logger.v(TAG, "Do nothing");
                    break;
                default:
                    Logger.v(TAG, "Unknown option");
                    break;
                }
                dismissDialog();
                clearSavedDialogs();
            }
        };

        /**
         * Show terminated by network dialog.
         */
        private void showTerminatedByNetworkDialog() {
            Logger.d(TAG, "showTerminatedByNetworkDialog() entry");
            destroy();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    dismissAllDialogs();
                    clearSavedDialogs();
                    String msg = mContext
                            .getString(R.string.video_sharing_terminated_due_to_network);
                    Logger.v(TAG,
                            "showTerminatedByNetworkDialog msg = "
                                    + msg);
                    createAndShowAlertDialog(msg, mOnClickListener);
                }
            });
            Logger.d(TAG, "showTerminatedByNetworkDialog() exit");
        }
        /**
         * Show waiting initialize conext progress dialog.
         */
        public void showWaitingInitializeConextProgressDialog() {
            Logger.d(TAG,
                    "showWaitingInitializeConextProgressDialog() entry");
            if (mCallScreenHost != null) {
                dismissAllDialogs();
                clearSavedDialogs();
                Activity activity = mCallScreenHost
                        .getCallScreenActivity();
                mWaitingProgressDialog = WaitingProgressDialog
                        .newInstance(mContext, mCallScreenHost
                                .getCallScreenActivity());
                mWaitingProgressDialog.show(
                        activity.getFragmentManager(), TAG);
            } else {
                Logger.d(TAG, "mCallScreenHost is null");
            }
            Logger.d(TAG,
                    "showWaitingInitializeConextProgressDialog() exit");
        }
        /**
         * Dismiss waiting initialize conext progress dialog.
         */
        public void dismissWaitingInitializeConextProgressDialog() {
            Logger.d(TAG,
                    "dismissWaitingInitializeConextProgressDialog");
            if (mWaitingProgressDialog != null) {
                mWaitingProgressDialog.dismiss();
            } else {
                Logger.d(TAG, "mWaitingProgressDialog is null");
            }
        }
        /**
         * Show rejected by remote dialog.
         */
        public void showRejectedByRemoteDialog() {
            Logger.v(TAG,
                    "showRejectedByRemoteDialog() mShareStatus: "
                            + mShareStatus);
            if (mIsVideoSharingReceiver.get()) {
                Logger.v(TAG,
                        "showRejectedByRemoteDialog(), sender is rejected by remote");
                destroy();
            } else {
                Logger.v(TAG,
                        "showRejectedByRemoteDialog(), receiver is rejected by remote");
                // share status here can be: LIVE_TWOWAY LIVE_OUT
                if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
                    mShareStatus = ShareStatus.LIVE_IN;
                } else {
                    mShareStatus = ShareStatus.UNKNOWN;
                }
                mIsStarted.set(false);
                removeVeiwsAtSenderTerminatedByRemote();
            }
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    dismissAllDialogs();
                    clearSavedDialogs();
                    String msg = mContext
                            .getString(R.string.remote_reject);
                    Logger.v(TAG, "showRejectedByRemoteDialog msg = "
                            + msg);
                    createAndShowAlertDialog(msg,
                            mRejectDialogListener);
                }
            });
        }

        /**
         * The m reject dialog listener.
         */
        DialogInterface.OnClickListener mRejectDialogListener =
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Logger.v(TAG, "onClick(), which = " + which);
                switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    Logger.v(TAG, "remvoe local video view");
                    break;
                case DialogInterface.BUTTON_NEGATIVE:
                    Logger.v(TAG, "Do nothing");
                    break;
                default:
                    Logger.v(TAG, "Unknown option");
                    break;
                }
                dismissDialog();
                clearSavedDialogs();
            }
        };

        /**
         * Creates the and show already going dialog.
         *
         * @param msg the msg
         * @param DialogListener the dialog listener
         * @return the call screen dialog
         */
        private CallScreenDialog createAndShowAlreadyGoingDialog(
                String msg,
                DialogInterface.OnClickListener dialogListener) {
            Logger.v(TAG, "createAlertDialog(), msg = " + msg);
            if (mCallScreenHost != null
                    && mCallScreenHost.getCallScreenActivity() != null) {
                Logger.v(TAG,
                        "createAndShowAlertDialog(), call screen host entry!");
                Activity activity = mCallScreenHost
                        .getCallScreenActivity();
                final CallScreenDialog callScreenDialog = new CallScreenDialog(
                        activity);
                callScreenDialog
                        .setIcon(android.R.attr.alertDialogIcon);
                callScreenDialog.setTitle(mContext
                        .getString(R.string.attention_title));
                callScreenDialog.setMessage(msg);
                callScreenDialog
                        .setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                dialogListener);
                callScreenDialog
                        .setNegativeButton(
                                mContext.getString(R.string.rcs_dialog_negative_button),
                                dialogListener);
                callScreenDialog
                        .setCancelListener(new DialogInterface.OnCancelListener() {
                            @Override
                            public void onCancel(
                                    DialogInterface dialog) {
                                mCallScreenDialogSet
                                        .remove(callScreenDialog);
                                clearSavedDialogs();
                            }
                        });
                clearSavedDialogs();
                callScreenDialog.show();
                mCallScreenDialogSet.add(callScreenDialog);
                saveAlertDialogs();
                return callScreenDialog;
            } else {
                Logger.w(TAG,
                        "mCallScreenHost is null or activity is null.");
                return null;
            }
        }
        /**
         * Already on going image share.
         */
        public void alreadyOnGoingImageShare() {
            Logger.v(TAG, "alreadyOnGoingImageShare entry");
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    String message = mContext
                            .getResources()
                            .getString(
                                    R.string.image_share_ongoing_video_start);
                    Logger.v(TAG, "alreadyOnGoingImageShare msg = "
                            + message);
                    createAndShowAlreadyGoingDialog(message,
                            mAlreadyOnGoingShareListener);
                }
            });
            Logger.v(TAG, "alreadyOnGoing exit");
        }

        /**
         * The m already on going share listener.
         */
        DialogInterface.OnClickListener mAlreadyOnGoingShareListener =
                new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                Logger.v(TAG, "onClick(), which = " + which);
                switch (which) {
                case DialogInterface.BUTTON_POSITIVE:
                    Logger.v(TAG, "stop image share first");
                    RCSeInCallUIExtension.getInstance()
                            .getmShareFilePlugIn().stop();
                    startVideoShare();
                    break;
                case DialogInterface.BUTTON_NEGATIVE:
                    Logger.v(TAG, "Do nothing");
                    break;
                default:
                    Logger.v(TAG, "Unknown option");
                    break;
                }
                dismissDialog();
                clearSavedDialogs();
            }
        };

        /**
         * Show time out dialog.
         */
        public void showTimeOutDialog() {
            Logger.d(TAG, "showTimeOutDialog entry");
            dismissAllDialogs();
            clearSavedDialogs();
            mMainHandler.post(new Runnable() {
                @Override
                public void run() {
                    Logger.d(TAG,
                            "showTimeOutDialog() create time out dialog!");
                    String msg = mContext
                            .getString(R.string.video_sharing_invitation_time_out);
                    createAndShowAlertDialog(msg, mOnClickListener);
                }
            });
            Logger.d(TAG, "showTimeOutDialog exit");
        }
        /**
         * Creates the and show alert dialog.
         *
         * @param msg the msg
         * @param posiviteListener the posivite listener
         * @return the call screen dialog
         */
        private CallScreenDialog createAndShowAlertDialog(String msg,
                DialogInterface.OnClickListener posiviteListener) {
            Logger.v(TAG, "createAlertDialog(), msg = " + msg);
            if (mCallScreenHost != null
                    && mCallScreenHost.getCallScreenActivity() != null) {
                Logger.v(TAG,
                        "createAndShowAlertDialog(), call screen host entry!");
                Activity activity = mCallScreenHost
                        .getCallScreenActivity();
                final CallScreenDialog callScreenDialog = new CallScreenDialog(
                        activity);
                callScreenDialog
                        .setIcon(android.R.attr.alertDialogIcon);
                callScreenDialog.setTitle(mContext
                        .getString(R.string.attention_title));
                callScreenDialog.setMessage(msg);
                callScreenDialog
                        .setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                posiviteListener);
                callScreenDialog
                        .setCancelListener(new DialogInterface.OnCancelListener() {
                            @Override
                            public void onCancel(
                                    DialogInterface dialog) {
                                mCallScreenDialogSet
                                        .remove(callScreenDialog);
                                clearSavedDialogs();
                            }
                        });
                clearSavedDialogs();
                callScreenDialog.show();
                mCallScreenDialogSet.add(callScreenDialog);
                saveAlertDialogs();
                return callScreenDialog;
            } else {
                Logger.w(TAG,
                        "mCallScreenHost is null or activity is null.");
                return null;
            }
        }
        /**
         * Show the video sharing invitation dialog.
         *
         * @return True if show dialog successfully, otherwise return false.
         */
        private boolean showInvitationDialog() {
            Logger.v(TAG, "showInvitationDialog()");
            dismissAllDialogs();
            clearSavedDialogs();
            // Vibrate
            Utils.vibrate(mContext, Utils.MIN_VIBRATING_TIME);
            CallScreenDialog callScreenDialog = createInvitationDialog();
            if (callScreenDialog != null) {
                callScreenDialog.show();
                saveAlertDialogs();
                return true;
            }
            return false;
        }
        /**
         * Create a video sharing invitation dialog, and add it to the set.
         *
         * @return The dialog created.
         */
        private CallScreenDialog createInvitationDialog() {
            Logger.v(TAG, "createInvitationDialog()");
            CallScreenDialog callScreenDialog = null;
            if (mCallScreenHost != null) {
                callScreenDialog = new CallScreenDialog(
                        mCallScreenHost.getCallScreenActivity());
                callScreenDialog
                        .setPositiveButton(
                                mContext.getString(R.string.rcs_dialog_positive_button),
                                VideoSharingPlugin.this);
                callScreenDialog
                        .setNegativeButton(
                                mContext.getString(R.string.rcs_dialog_negative_button),
                                VideoSharingPlugin.this);
                callScreenDialog
                        .setTitle(mContext
                                .getString(R.string.video_sharing_invitation_dialog_title));
                callScreenDialog
                        .setMessage(mContext
                                .getString(R.string.video_sharing_invitation_dialog_content));
                callScreenDialog.setCancelable(false);
                mCallScreenDialogSet.add(callScreenDialog);
            }
            Logger.d(TAG,
                    "createInvitationDialog() exit, callScreenDialog = "
                            + callScreenDialog);
            return callScreenDialog;
        }
    }

    /**
     * Share status.
     */
    private static final class ShareStatus {
        /**
         * The Constant UNKNOWN.
         */
        public static final int UNKNOWN = 0;
        /**
         * The Constant LIVE_OUT.
         */
        public static final int LIVE_OUT = 1;
        /**
         * The Constant LIVE_IN.
         */
        public static final int LIVE_IN = 2;
        /**
         * The Constant LIVE_TWOWAY.
         */
        public static final int LIVE_TWOWAY = 3;
    }

    /* (non-Javadoc)
     * @see android.view.View.OnClickListener#onClick(android.view.View)
     */
    @Override
    public void onClick(View v) {
        Logger.v(TAG, "onClick()");
        switch (v.getId()) {
        case R.drawable.btn_terminate_video_share_pre_receiver: {
            Logger.v(TAG, "onClick() receiver Button");
            if (mShareStatus == ShareStatus.LIVE_IN) {
                Logger.v(TAG, "onClick() receiver Button LIVE_IN");
                VideoSharingPlugin.this.stop();
            } else if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
                Logger.v(TAG, "onClick() receiver Button LIVE_TWOWAY");
                finishReceiverTwoWayLocal();
                mShareStatus = ShareStatus.LIVE_OUT;
            }
        }
            break;
        case R.drawable.btn_terminate_video_share_pre_sender: {
            Logger.v(TAG, "onClick() Sender Button");
            if (mShareStatus == ShareStatus.LIVE_OUT) {
                Logger.v(TAG, "onClick() receiver Button LIVE_OUT");
                VideoSharingPlugin.this.stop();
            } else if (mShareStatus == ShareStatus.LIVE_TWOWAY) {
                Logger.v(TAG, "onClick() Sender Button LIVE_TWOWAY");
                finishSenderTwoWaylocal();
                mShareStatus = ShareStatus.LIVE_IN;
            }
        }
            break;
         default:
        }
    }
    /* (non-Javadoc)
     * @see android.hardware.SensorEventListener#
     * onAccuracyChanged(android.hardware.Sensor, int)
     */
    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO Auto-generated method stub
    }
    /* (non-Javadoc)
     * @see android.hardware.SensorEventListener#
     * onSensorChanged(android.hardware.SensorEvent)
     */
    @Override
    public void onSensorChanged(SensorEvent event) {
        Logger.v(TAG, "onSensorChanged() " + event);
        synchronized (mLock) {
            if (mCamera != null) {
                if (mCameraPreviewRunning) {
                    mCameraPreviewRunning = false;
                    mCamera.stopPreview();
                }
            } else {
                Logger.w(TAG, "mCamera is null");
            }
        }
        boolean isLive = (mShareStatus == ShareStatus.LIVE_IN
                || mShareStatus == ShareStatus.LIVE_OUT ||
                mShareStatus == ShareStatus.LIVE_TWOWAY);
        if (isLive) {
            startCameraPreview();
        }
    }
}
