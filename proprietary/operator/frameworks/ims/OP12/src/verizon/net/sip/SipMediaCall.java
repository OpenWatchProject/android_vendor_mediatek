/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package verizon.net.sip;


import java.util.HashMap;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Set;

import android.os.Bundle;
import android.os.Message;
import android.os.Handler;
import android.os.HandlerThread;
import android.content.Context;
import android.os.ServiceManager;
import android.content.Intent;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraManager;
import android.media.AudioManager;
import android.media.IAudioService;
import android.telecom.ConferenceParticipant;
import android.telecom.Connection;
import android.telecom.Connection.VideoProvider;
import android.telecom.VideoProfile;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.os.RemoteException;
import android.net.sip.SipException;
import android.net.Uri;

import android.telephony.ims.ImsCallSession;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsConferenceState;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsReasonInfo;
import com.android.ims.internal.IImsCallSession;
import com.android.internal.telecom.IVideoCallback;
import com.android.internal.telecom.IVideoProvider;
import com.android.ims.internal.IImsVideoCallProvider;
import com.android.ims.internal.ImsVideoCallProviderWrapper;

import com.mediatek.ims.internal.ImsVTProvider;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.internal.MtkImsCallSession;

public class SipMediaCall {
    public static final int AUDIO_CALL = 0;
    public static final int VIDEO_SHARE = 1;
    public static final int VIDEO_CALL = 2;
    public static final int INBOUND_ONLY_VIDEO_CALL = 3;

    //private Listener listener ;
    private static final String TAG = "SipMediaCallImpl";
    private static final int IMS_CALL_MODE_CLIENT_API = 2;

    public Context mContext;
    //public MtkImsCall mCall = null;
    public MtkImsCallSession mCallSession = null;
    public ImsVideoCallProviderWrapper mWrapper = null;
    public ImsVTProvider mVTProvider = null;
    public int mCurrentType;
    public VideoProfile mCurrentVideoProfile;
    public boolean isOnVideoHold = false;
    private boolean mInCall = false;
    public int mHold = 1;
    private boolean mMute = false;

    public String mCameraId = null;
    private Surface mPreviewSurface = null;
    private Surface mDisplaySurface = null;

    private VideoProfile currentProfile = null;
    private VideoProfile requestProfile = null;

    private Handler mHandler;
    public Listener mListener = null;
    //private CallListener mCallListener = null;
    private VideoCallListenerBinder mVideoCallListenerBinder = null;
    private CameraAvailableListener mCameraAvailableListener = null;
    protected ImsCallSessionListenerProxy mImsCallSessionListenerProxy;
    private ImsCallProfile mImsCallProfile;
    private AudioManager mAudioManager;
    private IAudioService mAudioService;
    public HashMap<String, Bundle> mParticipants = new HashMap<String, Bundle>();
    private SipManager mSipManager;
    private String mCallId = null;

    public SipMediaCall(Context InContext){
        mContext = InContext;
        try {
            mListener = new Listener();
            mListener.setOwner(this);
        } catch (Throwable ex) {
            Log.e (TAG, "retrieve SipMediaCall Listener fail" + ex);
        }
        try {
            mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
            mAudioService = IAudioService.Stub.asInterface(
                    ServiceManager.getService(Context.AUDIO_SERVICE));
        } catch (Throwable ex) {
            Log.e (TAG, "retrieve AudioManager fail" + ex);
        }
        mSipManager = SipManager.newInstance(mContext);
        try {
            mVideoCallListenerBinder = new VideoCallListenerBinder();
            mVideoCallListenerBinder.setOwner(this);
        } catch (Throwable ex) {
            Log.e (TAG, "retrieve mVideoCallListenerBinder fail" + ex);
        }

        if (mHandler == null) {
            HandlerThread thread = new HandlerThread(TAG);
            thread.start();
            mHandler = new Handler(thread.getLooper());
        }

        mCameraAvailableListener = new CameraAvailableListener();
        CameraManager cameraManager =
                (CameraManager) mContext.getSystemService(Context.CAMERA_SERVICE);
        cameraManager.registerAvailabilityCallback(mCameraAvailableListener, mHandler);
    }

    protected ImsCallSession.Listener createCallSessionListener() throws SipException{
        try{
            mImsCallSessionListenerProxy = new ImsCallSessionListenerProxy();
            mImsCallSessionListenerProxy.setOwner(this);
            Log.v(TAG, "createCallSessionListener : " + mImsCallSessionListenerProxy);
        }catch (Throwable ex) {
            Log.e (TAG, "retrieve ImsCallSessionListenerProxy fail" + ex);
        }
        return mImsCallSessionListenerProxy;
    }

    public void start(MtkImsCallSession session, String callee, ImsCallProfile CallProfile) throws SipException{
        Log.v(TAG,"start(1) :: session=" + session);
        try{
            attachSession(session);
            session.start(callee, CallProfile);
        }catch (Throwable ex) {
            Log.e (TAG, "retrieve start session fail" + ex);
        }
    }

    public void makeMediaCall(String peerUri, int type,int timeout, HashMap<String,String> extraHeaders, String fromUri) {
        //what to pass to createSession??what is profile?
        //type needs to look at callSession
        //createSession + attachSession + start

        //if(extraHeaders == null)
            //if(fromUri==null)
                //mCallSession = MtkImsManager.createCallSession(0,type); //input service type = 0 ??
            //else
               // mCallSession = MtkImsManager.createCallSession(0,type);
       // else
           // mCallSession = MtkImsManager.createCallSession(0,type);
    }
    public void makeCall(String peerUri,HashMap<String,String> extraHeaders)throws SipException{
        try {
            mSipManager.makeCall(peerUri, 0, extraHeaders, this);
        } catch (Throwable ex) {
            Log.e (TAG, "MakeCall from SipMediaCall fail" + ex);
            throw new SipException("makeCall()");
        }
    }
    public void makeCall(String peerUri, int timeout,HashMap<String,String> extraHeaders)throws SipException{
        try {
            mSipManager.makeCall(peerUri, timeout, extraHeaders, this);
        } catch (Throwable ex) {
            Log.e (TAG, "MakeCall from SipMediaCall fail" + ex);
            throw new SipException("makeCall()");
        }
    }

    public void takeMediaCall(Intent incomingCallIntent, HashMap<String,String> headers){
        // createSession + attachSession
    }

    public void attachSession(MtkImsCallSession session){
        try {
            mCallSession = session;
            mCallSession.setListener(createCallSessionListener());
            mCallSession.setImsCallMode(IMS_CALL_MODE_CLIENT_API);
            IImsVideoCallProvider imsVideoCallProvider = mCallSession.getVideoCallProvider();
            mWrapper = new ImsVideoCallProviderWrapper(imsVideoCallProvider);
            mWrapper.getInterface().addVideoCallback(mVideoCallListenerBinder);
            Log.v(TAG, "attachSession :: session = " + session +" mWapper = " + mWrapper);
        } catch (Throwable ex) {
            Log.e (TAG, "mWrapper load fail" + ex);
        }
        //mVTProvider.addVideoProviderStateListener(mVideoCallListenerBinder);
    }

    public void answerCall(int timeout) {
        Log.v(TAG, "answerCall :: Type = " + mCurrentType);
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        mCallSession.accept(mCurrentType, null); //callType unknown
    }

    public void answerCallAudioOnly(int timeout) {
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        mCallSession.accept(AUDIO_CALL, null);
    }

    public void close() {
        Log.v(TAG, "close()");
        if (mCallSession != null) {
            mCallSession.close();
            mCallSession = null;
        } else {
            Log.i(TAG, "close :: Cannot close Null call session!");
        }
        mAudioManager.abandonAudioFocusForCall();
        mAudioManager.setMode(AudioManager.MODE_NORMAL);
        // set profile and ... to null
    }

    public void continueCall(int timeout) {
        Log.v(TAG, "continueCall() timeout: " + timeout);
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        mCallSession.resume(createResumeMediaProfile(mCallSession.getCallProfile()));
    }

    public void endCall() {
        mCallSession.terminate(0);//need input reason
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
    }

    public SipProfile[] getPeerProfile() {
        return null;
    }

    public int getState() {
        if (mCallSession == null)
            return ImsCallSession.State.IDLE;
        else
            return mCallSession.getState();
    }

    public void holdCall(int timeout) {
        Log.v(TAG, "holdCall");
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        mCallSession.hold(createHoldMediaProfile());
    }

    public boolean isInCall() {
        //maintatin by here
        return mInCall;
    }

    public boolean isMuted() {
        //maintatin by here
        mMute = mAudioManager.isMicrophoneMute();
        return mMute;
    }

    public int isOnHold() {
        //maintatin by here
        /*Returns the following values:
        0 = not on hold
        1 = held locally
        2 = held remotely
        3 = held by both
        */
        return mHold;
    }

    public void sendDtmf(int code) {
        Message result = null;
        char c = (char) (code + '0');
        Log.v(TAG, "sendDtmf: "+ c );
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        ((MtkImsCallSession) mCallSession).sendDtmf((char)c, result);
    }

    public void setListener(Listener listener, boolean callbackImmediately) {
        Log.v(TAG, "setListener:" + listener);
        this.mListener = listener;
    }

    public void setSpeakerMode(boolean speakerMode) {
        Log.v(TAG, "setSpeakerMode");
        synchronized (this) {
            mAudioManager.setSpeakerphoneOn(speakerMode);
            // TODO: need to setAudioGroupMode like SipAudioCall?
        }
    }

    /*
    private boolean isSpeakerOn() {
        return ((AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE))
                .isSpeakerphoneOn();
    }
    */

    public void startAudio() {
        Log.v(TAG, "startAudio");
    }

    public void toggleMute() {
        Log.v(TAG, "toggleMute");
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        mMute = !mMute;
        // mCallSession.setMute(mMute);
        if (mAudioManager != null) {
            mAudioManager.setMicrophoneMute(mMute);
        }
    }

    public void rejectCall(int timeout) {
        Log.v(TAG, "rejectCall");
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        mCallSession.reject(ImsReasonInfo.CODE_USER_DECLINE);
    }

    public void setVolume(int index) {
        Log.v(TAG, "setVolume");
        mAudioManager.setStreamVolume(AudioManager.STREAM_VOICE_CALL, index, 0);
    }

    public int getMaxVolume() {
        Log.v(TAG, "getMaxVolume");
        return mAudioManager.getStreamMaxVolume(AudioManager.STREAM_VOICE_CALL);
    }

    public void removeLastParticipant(int timeout) {
        Log.v(TAG, "removeLastParticipant");
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        mCallSession.removeLastParticipant();
    }

    public String getCallId() {
        Log.v(TAG, "getCallId()");
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return null;
        }
        String id = mCallSession.getHeaderCallId();
        if (id == null){
            Log.e(TAG, "callId is null");
            id = "";
        }
        return id;
    }

    private ImsStreamMediaProfile createResumeMediaProfile(ImsCallProfile mCallProfile) {
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile();

        if (mCallProfile == null) {
            return mediaProfile;
        }

        mediaProfile.mAudioQuality = mCallProfile.mMediaProfile.mAudioQuality;
        mediaProfile.mVideoQuality = mCallProfile.mMediaProfile.mVideoQuality;
        mediaProfile.mAudioDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;

        if (mediaProfile.mVideoQuality != ImsStreamMediaProfile.VIDEO_QUALITY_NONE) {
            mediaProfile.mVideoDirection = ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        }

        return mediaProfile;
    }

    private ImsStreamMediaProfile createHoldMediaProfile() {
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile();

        if (mImsCallProfile == null) {
            return mediaProfile;
        }

        mediaProfile.mAudioQuality = mImsCallProfile.mMediaProfile.mAudioQuality;
        mediaProfile.mVideoQuality = mImsCallProfile.mMediaProfile.mVideoQuality;
        mediaProfile.mAudioDirection = ImsStreamMediaProfile.DIRECTION_SEND;

        if (mediaProfile.mVideoQuality != ImsStreamMediaProfile.VIDEO_QUALITY_NONE) {
            mediaProfile.mVideoDirection = ImsStreamMediaProfile.DIRECTION_SEND;
        }

        return mediaProfile;
    }

/*
APIS for vedio part
*//*
    public void setCamera(int id, Surface surface) { // ,Surface surface)

    }

    public void setDisplay(Surface surface) { //Surface surface) {

    }

    public void changeCall(int type) {

    }

    public void holdVideo() {

    }

    public void continueVideo() {

    }*/

    public int typeToProfileState(int type) {
        if (AUDIO_CALL == type) {
            return VideoProfile.STATE_AUDIO_ONLY;

        } else if (VIDEO_SHARE == type) {
            return VideoProfile.STATE_TX_ENABLED;

        } else if (VIDEO_CALL == type) {
            return VideoProfile.STATE_BIDIRECTIONAL;

        } else if (INBOUND_ONLY_VIDEO_CALL == type) {
            return VideoProfile.STATE_RX_ENABLED;

        } else {
            Log.e (TAG, "unknown type:" + type);
            return VideoProfile.STATE_AUDIO_ONLY;
        }
    }

    public int profileStateToType(int videoState) {
        if (VideoProfile.STATE_AUDIO_ONLY == videoState) {
            return AUDIO_CALL;

        } else if (VideoProfile.STATE_TX_ENABLED == videoState) {
            return VIDEO_SHARE;

        } else if (VideoProfile.STATE_BIDIRECTIONAL == videoState) {
            return VIDEO_CALL;

        } else if (VideoProfile.STATE_RX_ENABLED == videoState) {
            return INBOUND_ONLY_VIDEO_CALL;

        } else {
            Log.e (TAG, "unknown video state:" + videoState);
            return AUDIO_CALL;
        }
    }

    // MTK
    public void upgradeCall(int type, String cameraId, Surface incomingVideo, Surface outgoingVideo) {

        Log.d(TAG, "incomingVideo: " + incomingVideo + ", outgoingVideo: " + outgoingVideo);

        if (VIDEO_SHARE == type) {

            if (null == outgoingVideo) {
                Log.e (TAG, "outgoingVideo null");
                return;

            } else {
                setCamera(cameraId, outgoingVideo);

                if (null != incomingVideo) {
                    setDisplay(incomingVideo);
                }

                upgradeCall(type);
            }

        } else if(VIDEO_CALL == type) {

            if (null == incomingVideo || null == outgoingVideo) {
                Log.e (TAG, "incomingVideo or outgoingVideo null");
                return;

            } else {
                setCamera(cameraId, outgoingVideo);
                setDisplay(incomingVideo);
                upgradeCall(type);
            }
        }
    }

    public void upgradeCall(int type,int cameraId,SurfaceView incomingVideo,SurfaceHolder outgoingVideo) {
        upgradeCall(type, Integer.toString(cameraId),
                        incomingVideo.getHolder().getSurface(), outgoingVideo.getSurface());
    }

    private int ImsCallTypeTosipCallType(ImsCallProfile profile) {
        int type = profile.mCallType;
        int sipCalltype = mCurrentType;
        switch(type) {
            case ImsCallProfile.CALL_TYPE_VOICE:
                sipCalltype = SipMediaCall.AUDIO_CALL;
                break;
            case ImsCallProfile.CALL_TYPE_VT_TX:
                sipCalltype = SipMediaCall.VIDEO_SHARE;
                break;
            case ImsCallProfile.CALL_TYPE_VT:
                sipCalltype = SipMediaCall.VIDEO_CALL;
                break;
            case ImsCallProfile.CALL_TYPE_VT_RX:
                sipCalltype = SipMediaCall.INBOUND_ONLY_VIDEO_CALL;
                break;
            default:
                break;
        }
        Log.v(TAG, "set call type as Imstype/type: " +type+" / " +sipCalltype);
        return sipCalltype;
    }
/* Deprecated

    public void upgradeAudioCall(int type,int cameraId,SurfaceView incomingVideo,SurfaceHolder outgoingVideo) {
        upgradeCall(type, Integer.toString(cameraId),
                        incomingVideo.getHolder().getSurface(), outgoingVideo.getSurface());
    }

    public void upgradeAudioCall(int type,int cameraId,SurfaceTexture incomingVideo,SurfaceTexture outgoingVideo) {
        Surface incomingSurface = new Surface(incomingVideo);
        Surface outgoingSurface = new Surface(outgoingVideo);
        upgradeCall(type, Integer.toString(cameraId), incomingSurface, outgoingSurface);
    }

    public void upgradeAudioCall(int type,String cameraId,SurfaceView incomingVideo,SurfaceHolder outgoingVideo) {
        upgradeCall(type, cameraId, incomingVideo.getHolder().getSurface(), outgoingVideo.getSurface());
    }

    public void upgradeAudioCall(int type,String cameraId,SurfaceTexture incomingVideo,SurfaceTexture outgoingVideo) {
        Surface incomingSurface = new Surface(incomingVideo);
        Surface outgoingSurface = new Surface(outgoingVideo);
        upgradeCall(type, cameraId, incomingSurface, outgoingSurface);
    }
*/

    // Only applicable if in an audio or video share call
    public void upgradeCall(int type) {
        Log.v(TAG, "upgrade call from : " +mCurrentType+" -> " +type);

        VideoProfile fromProfile = new VideoProfile(typeToProfileState(mCurrentType), VideoProfile.QUALITY_MEDIUM);
        VideoProfile toProfile = new VideoProfile(typeToProfileState(type), VideoProfile.QUALITY_MEDIUM);

        if (AUDIO_CALL == mCurrentType) {

           if (VIDEO_SHARE == type) {

              if (null != mCameraId) {
                  mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);

              } else {
                  //throw new SipException; -> wait for SipEcxeption
              }

           } else if (VIDEO_CALL == type){

               if (null != mCameraId && null != mDisplaySurface) {
                   mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);

               } else {
                   //throw new SipException; -> wait for SipEcxeption
               }
           }

        } else if (VIDEO_SHARE == mCurrentType) {

            if (VIDEO_CALL == type){
                if (null != mCameraId && null != mDisplaySurface) {
                    mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);
                }else {
                    Log.e (TAG, "upgrade to video call error :: mCameraId :"+mCameraId+"mDisplaySurface"+mDisplaySurface);
                }
            }

        } else {
            Log.e (TAG, "ignore upgradeCall, mCurrentType:" + mCurrentType);
            //throw new SipException; -> wait for SipEcxeption
        }
    }

    // Only applicable if in an audio call
    public void upgradeAudioCall(int type) {
        if (AUDIO_CALL != mCurrentType) {
            Log.e (TAG, "ignore upgradeCall, mCurrentType:" + mCurrentType);
        } else {
            upgradeCall(type);
        }
    }

    public void downgradeCall(int type) {

        VideoProfile fromProfile = new VideoProfile(typeToProfileState(mCurrentType), VideoProfile.QUALITY_MEDIUM);
        VideoProfile toProfile = new VideoProfile(typeToProfileState(type), VideoProfile.QUALITY_MEDIUM);

        if (VIDEO_CALL == mCurrentType) {
            int videoState = VideoProfile.STATE_BIDIRECTIONAL;

            if (VIDEO_SHARE == type) {
                mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);

            } else if(INBOUND_ONLY_VIDEO_CALL == type){
                mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);

            } else if(AUDIO_CALL == type) {
                mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);
            }

        } else if (VIDEO_SHARE == mCurrentType) {

            if (AUDIO_CALL == type) {
                mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);
            }

        } else if (AUDIO_CALL == mCurrentType) {
            //throw new SipException; -> wait for SipEcxeption
        }
    }

    public int getCallType() {
        mCurrentType = ImsCallTypeTosipCallType(mImsCallProfile);
        Log.v(TAG, "return call type : " + mCurrentType);
        return mCurrentType;
    }

    public void setCamera(int id, SurfaceHolder surfaceHolder) {
        setCamera(Integer.toString(id), surfaceHolder.getSurface());
    }

    public void setCamera(int id, Surface surface) {
        setCamera(Integer.toString(id), surface);
    }

    public void setCamera(String id, Surface surface) {//throws SipException {
        Log.v(TAG, "setCamera : " + id);

        // If a null surface is provided with a valid camera ID the method must throw a RuntimeEception
        if (id != null && !id.equals("-1") && null == surface) {
            throw new RuntimeException("surface null");
        }
        if (mWrapper == null){
            Log.e (TAG, "mWrapper is null ");
        }

        if (id == null || id.equals("-1")) {
            mWrapper.onSetCamera(null);
            mCameraId = null;

        } else {
            if (!mCameraAvailableListener.isCameraAvailable(id)) {
                throw new RuntimeException("camera not available");
            }

            mWrapper.onSetCamera(id);
            mCameraId = id;
        }

        mWrapper.onSetPreviewSurface(surface);
        mPreviewSurface = surface;
    }

    public void setDisplay(SurfaceView surfaceView) {
        setDisplay(surfaceView.getHolder().getSurface());
    }

    public void setDisplay(SurfaceHolder incomingVideo) {
        setDisplay(incomingVideo.getSurface());
    }

    public void setDisplay(Surface surface) {
        Log.v(TAG, "setdisplay : " + surface);
        if (null != surface) {
            mWrapper.onSetDisplaySurface(surface);
            mDisplaySurface = surface;
        } else {
        Log.e(TAG, "setDisplay fail with : " + surface);
        }
    }

/* Deprecated
    public void startVideo() {
    }

    public void startMedia() {
    }
*/

    public void changeCall(int type) {
        if (VIDEO_CALL == mCurrentType) {
            downgradeCall(type);
        } else if (VIDEO_SHARE == mCurrentType && AUDIO_CALL == type) {
            downgradeCall(type);
        } else if (VIDEO_SHARE == mCurrentType && VIDEO_CALL == type) {
            upgradeCall(type);
        } else {
            upgradeCall(type);
        }
    }

    public synchronized void setVideoHold(boolean isHold) {
        isOnVideoHold = isHold;
    }

    public synchronized boolean getVideoHold() {
        return isOnVideoHold;
    }

    public void holdVideo() {
        Log.v(TAG, " holdVideo, current type: "+mCurrentType);
        if (VIDEO_CALL == mCurrentType || VIDEO_SHARE == mCurrentType) {

            int count = 0;

            // Wait for call state not update yet
            while (getVideoHold() && count < 10) {
                try {
                    Log.w(TAG, "Wait for video resume update, isOnVideoHold: " + getVideoHold());
                    Thread.sleep(100);
                } catch(InterruptedException ex) {
                }

                count++;
            }

            if (count >= 10) {
                Log.w(TAG, "Video still hold!! isOnVideoHold: " + getVideoHold());
            }

            int currentVideoState = typeToProfileState(mCurrentType);
            int toVideoState = currentVideoState | VideoProfile.STATE_PAUSED;

            VideoProfile fromProfile = new VideoProfile(currentVideoState, VideoProfile.QUALITY_MEDIUM);
            VideoProfile toProfile = new VideoProfile(toVideoState, VideoProfile.QUALITY_MEDIUM);

            mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);
        }
    }

    public void continueVideo() {
        Log.v(TAG, " continueVideo ");
        int count = 0;

        // Wait for call state not update yet
        while (!getVideoHold() && count < 10) {
            try {
                Log.w(TAG, "Wait for video hold update, isOnVideoHold: " + getVideoHold());
                Thread.sleep(100);
            } catch(InterruptedException ex) {
            }

            count++;
        }

        if (count >= 10) {
            Log.w(TAG, "Video not hold!! isOnVideoHold: " + getVideoHold());
        }

        int currentVideoState = typeToProfileState(mCurrentType);
        int toVideoState = currentVideoState & ~VideoProfile.STATE_PAUSED;

        VideoProfile fromProfile = new VideoProfile(currentVideoState, VideoProfile.QUALITY_MEDIUM);
        VideoProfile toProfile = new VideoProfile(toVideoState, VideoProfile.QUALITY_MEDIUM);

        mWrapper.onSendSessionModifyRequest(fromProfile, toProfile);
    }

    public void acceptChangeRequest(int timeout) {
        Log.v(TAG, " acceptChangeRequest ");
        mWrapper.onSendSessionModifyResponse(requestProfile);
    }

    public void rejectChangeRequest(int timeout) {
        Log.v(TAG, " rejectChangeRequest ");
        VideoProfile currentProfile = new VideoProfile(typeToProfileState(mCurrentType), VideoProfile.QUALITY_MEDIUM);
        mWrapper.onSendSessionModifyResponse(currentProfile);
    }

    public void merge() {
        Log.v(TAG, " merge()");
        if (mCallSession == null) {
            Log.w(TAG, "Call session end.");
            return;
        }
        mCallSession.merge();
    }

    private synchronized void notifyConferenceStateUpdated(ImsConferenceState state) {
        if (state == null || state.mParticipants == null) {
            return;
        }

        Set<Entry<String, Bundle>> participants = state.mParticipants.entrySet();

        if (participants == null) {
            return;
        }

        Iterator<Entry<String, Bundle>> iterator = participants.iterator();

        while (iterator.hasNext()) {
            Entry<String, Bundle> entry = iterator.next();

            String key = entry.getKey();
            Bundle confInfo = entry.getValue();
            String status = confInfo.getString(ImsConferenceState.STATUS);
            String user = confInfo.getString(ImsConferenceState.USER);
            String displayName = confInfo.getString(ImsConferenceState.DISPLAY_TEXT);
            String endpoint = confInfo.getString(ImsConferenceState.ENDPOINT);

            Log.i(TAG, "notifyConferenceStateUpdated :: key=" + key +
                    ", status=" + status +
                    ", user=" + user +
                    ", displayName= " + displayName +
                    ", endpoint=" + endpoint);

            Uri handle = Uri.parse(user);
            if (endpoint == null) {
                endpoint = "";
            }
            Uri endpointUri = Uri.parse(endpoint);
            int connectionState = ImsConferenceState.getConnectionStateForStatus(status);

            if (mParticipants.containsKey(key)) {
                // TODO: SipProfile
                mListener.onConferenceEvent(this, "JOINED", null);
            }

            if (connectionState == Connection.STATE_DISCONNECTED) {
                // TODO: SipProfile
                mListener.onConferenceEvent(this, "REMOVED", null);;
            }
        }
        // Update local cache
        mParticipants = state.mParticipants;
    }

    public void setOrientation(int orientation){
        Log.v(TAG,"setOrientation :: orientation=" + orientation );
        if (mWrapper == null){
            Log.e (TAG, "mWrapper is null ");
        }
        mWrapper.onSetDeviceOrientation(orientation);
    }
    public int getLocalOrientation(){
        return 90;
    }
    public int getRemoteOrientation(){
        return 0;
    }

//Listeners

    public static class Listener {
        public SipMediaCall mOwner;
        public void setOwner(SipMediaCall owner) {
            mOwner = owner;
        }
        public void onCallBusy(SipMediaCall call) {
        }
        public void onCallEnded(SipMediaCall call) {
        }
        public void onCallEstablished(SipMediaCall call) {
        }
        public void onCallHeld(SipMediaCall call, boolean heldLocally) {
        }
        public void onCalling(SipMediaCall call) {
        }
        public void onChanged(SipMediaCall call) {
        }
        public void onChangeRequest(SipMediaCall call, int type) {
        }
        public void onConferenceEvent(SipMediaCall call, String event, SipProfile participant) {
        }
        public void onError(SipMediaCall call, int errorCode, String errorMessage) {
        }
        public void onOrientationChanged(SipMediaCall call) {
        }
        public void onReadyToCall(SipMediaCall call) {
        }
        public void onRinging(SipMediaCall call, SipProfile caller) {
        }
        public void onRingingBack(SipMediaCall call) {
        }
        public void onVideoAvailable(SipMediaCall call) {
        }
        public void onCallResumed(SipMediaCall call, boolean resumedLocally) {
        }
        public void onVideoHeld(SipMediaCall call) {
            mOwner.setVideoHold(true);
        }
        public void onVideoResumed(SipMediaCall call) {
            mOwner.setVideoHold(false);
        }
    };

    public class VideoCallListenerBinder extends IVideoCallback.Stub {//implements ImsVTProvider.VideoProviderStateListener {

        private SipMediaCall mOwner;

        public void setOwner(SipMediaCall owner) {
            mOwner = owner;
        }

        // IVideoCallback
        @Override
        public void receiveSessionModifyRequest(VideoProfile VideoProfile) {
            Log.v(TAG,"receiveSessionModifyRequest :: VideoProfile=" + VideoProfile );
            mOwner.mListener.onChangeRequest(mOwner, profileStateToType(VideoProfile.getVideoState()));
            mOwner.mCurrentType = profileStateToType(VideoProfile.getVideoState());
            requestProfile = VideoProfile;
        }

        @Override
        public void receiveSessionModifyResponse(
                int status, VideoProfile requestProfile, VideoProfile responseProfile) {
            Log.v(TAG,"receiveSessionModifyResponse :: VideoProfile=" + responseProfile +" status :: " +status);
            if(status != 0)
                mOwner.mListener.onError(mOwner, 700, "Upgrade/downgrade rejected");
            mOwner.mCurrentType = profileStateToType(responseProfile.getVideoState());
        }

        @Override
        public void handleCallSessionEvent(int event) {

            if (ImsVTProvider.SESSION_EVENT_RECEIVE_FIRSTFRAME == event) {
                mOwner.mListener.onVideoAvailable(mOwner);
            } else {
            }
        }

        @Override
        public void changePeerDimensions(int width, int height) {
        }

        @Override
        public void changeVideoQuality(int videoQuality) {
        }

        @Override
        public void changeCallDataUsage(long dataUsage) {
        }

        @Override
        public void changeCameraCapabilities(
            VideoProfile.CameraCapabilities cameraCapabilities) {
        }

        //ImsVTProvider.VideoProviderStateListener
        public void onReceivePauseState(boolean isPause) {
            Log.v(TAG,"onReceivePauseState :: isPause=" + isPause );
            if (isPause) {
                mOwner.mListener.onVideoHeld(mOwner);
            } else {
                mOwner.mListener.onVideoResumed(mOwner);
            }
        }

        public void onReceiveWiFiUsage(long usage) {
        }
    }

    public class CameraAvailableListener extends CameraManager.AvailabilityCallback {

        private ArrayList<String>  mCameraAvailableList = new ArrayList<String>();

        @Override
        public void onCameraAvailable(String cameraId) {
            Log.i(TAG, "Camera:" + cameraId + " is now available");
            updateAvailableList(cameraId, true);
        }

        @Override
        public void onCameraUnavailable(String cameraId) {
            Log.i(TAG, "Camera:" + cameraId + " is now unavailable");
            updateAvailableList(cameraId, false);
        }

        private synchronized void updateAvailableList(String cameraId, boolean available) {

            boolean isIn = mCameraAvailableList.contains(cameraId);

            if (available && !isIn) {
                mCameraAvailableList.add(cameraId);

            } else if (!available && isIn) {
                mCameraAvailableList.remove(cameraId);
            }
        }

        public synchronized boolean isCameraAvailable(String cameraId) {
            return mCameraAvailableList.contains(cameraId);
        }
    }

    public class ImsCallSessionListenerProxy extends ImsCallSession.Listener {
        private SipMediaCall mOwner;

        public void setOwner(SipMediaCall owner) {
            Log.v(TAG, "set ImsCallSessionListenerProxy owner" + owner + " listener :"+ owner.mListener);
            mOwner = owner;
        }
        @Override
        public void callSessionProgressing(ImsCallSession session, ImsStreamMediaProfile profile) {
            Log.v(TAG,"CallSessionProgressing :: session=" + session + " profile=" + profile);
            mOwner.mListener.onRingingBack(mOwner);
            mAudioManager.requestAudioFocusForCall(AudioManager.STREAM_RING,
                    AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
            mAudioManager.setMode(AudioManager.MODE_RINGTONE);
        }

        @Override
        public void callSessionStarted(ImsCallSession session, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionStarted :: session=" + session + " profile=" + profile);
            mOwner.mListener.onCallEstablished(mOwner);
            mAudioManager.requestAudioFocusForCall(AudioManager.STREAM_VOICE_CALL,
                    AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
            mAudioManager.setMode(AudioManager.MODE_IN_CALL);
            mInCall = true;
        }

        @Override
        public void callSessionStartFailed(ImsCallSession session, ImsReasonInfo reasonInfo) {
            Log.v(TAG,"CallSessionStartFailed :: session=" + session + " reasonInfo=" + reasonInfo);
            mOwner.mListener.onCallEnded(mOwner);
            mSipManager.notifyCallSessionTerminated(mCallId);
            mOwner.close();
        }

        @Override
        public void callSessionTerminated(ImsCallSession session, ImsReasonInfo reasonInfo) {
            Log.v(TAG,"CallSessionTerminated :: session=" + session + " reasonInfo=" + reasonInfo);
            mOwner.mListener.onCallEnded(mOwner);
            mSipManager.notifyCallSessionTerminated(mCallId);
            mInCall = false;
            mOwner.close();
        }

        @Override
        public void callSessionHeld(ImsCallSession session, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionHeld :: session=" + session + "profile=" + profile);
            boolean local = true;
            mOwner.mListener.onCallHeld(mOwner, local);

            mOwner.mListener.onReadyToCall(mOwner);
            mHold = 1;
        }

        @Override
        public void callSessionHoldFailed(ImsCallSession session, ImsReasonInfo reasonInfo) {
            Log.v(TAG,"CallSessionHoldFailed :: session" + session + "reasonInfo=" + reasonInfo);

        }

        @Override
        public void callSessionHoldReceived(ImsCallSession session, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionHoldReceived :: session=" + session + "profile=" + profile);
            mOwner.mListener.onCallHeld(mOwner, false);
            mHold = 2;
        }

        @Override
        public void callSessionResumed(ImsCallSession session, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionResumed :: session=" + session + "profile=" + profile);
            boolean local = true;
            mOwner.mListener.onCallResumed(mOwner, local);
            mHold = 0;
        }

        @Override
        public void callSessionResumeFailed(ImsCallSession session, ImsReasonInfo reasonInfo) {
            Log.v(TAG,"CallSessionResumeFailed :: session=" + session + "reasonInfo=" + reasonInfo);

        }

        @Override
        public void callSessionResumeReceived(ImsCallSession session, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionResumeReceived :: session=" + session + "profile=" + profile);
            mOwner.mListener.onCallResumed(mOwner, false);
            mHold = 0;
        }

        @Override
        public void callSessionMergeStarted(ImsCallSession session, ImsCallSession newSession, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionMergeStarted :: session=" + session + " newSession=" + newSession + ", profile=" + profile);

        }

        @Override
        public void callSessionMergeComplete(ImsCallSession newSession) {
            Log.v(TAG,"CallSessionMergeComplete :: newSession =" + newSession);
            mSipManager.notifyCallSessionMergeComplete(newSession);
        }

        @Override
        public void callSessionMergeFailed(ImsCallSession session, ImsReasonInfo reasonInfo) {
            Log.v(TAG,"CallSessionMergeFailed :: session=" + session + "reasonInfo=" + reasonInfo);

        }

        @Override
        public void callSessionUpdated(ImsCallSession session, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionUpdated :: session=" + session + " profile=" + profile);
            mImsCallProfile = profile;
            mInCall = true;
            mOwner.mListener.onChanged(mOwner);
        }

        @Override
        public void callSessionUpdateFailed(ImsCallSession session, ImsReasonInfo reasonInfo) {
            Log.v(TAG,"CallSessionUpdateFailed :: session=" + session + " reasonInfo=" + reasonInfo);

        }

        @Override
        public void callSessionUpdateReceived(ImsCallSession session, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionUpdateReceived :: session=" + session + " profile=" + profile);

        }

        @Override
        public void callSessionRemoveParticipantsRequestDelivered(ImsCallSession session) {
            Log.v(TAG,"CallSessionRemoveParticipantsRequestDelivered :: ");

        }

        @Override
        public void callSessionRemoveParticipantsRequestFailed(ImsCallSession session, ImsReasonInfo reasonInfo) {
            Log.v(TAG,"CallSessionRemoveParticipantsRequestFailed :: reasonInfo=" + reasonInfo);

        }

        @Override
        public void callSessionTtyModeReceived(ImsCallSession session, int mode) {
            Log.v(TAG,"CallSessionTtyModeReceived :: mode=" + mode);

        }

        @Override
        public void callSessionRinging(ImsCallSession session, ImsCallProfile profile) {
            Log.v(TAG,"CallSessionRinging :: session=" + session + " profile=" + profile);
            mImsCallProfile = profile;
            mOwner.mListener.onRinging(mOwner, null);
        }

        @Override
        public void callSessionBusy(ImsCallSession session) {
            Log.v(TAG,"CallSessionBusy :: session=" + session);
            mOwner.mListener.onCallBusy(mOwner);
        }

        @Override
        public void callSessionCalling(ImsCallSession session) {
            Log.v(TAG,"CallSessionCalling :: session=" + session);
            mOwner.mListener.onCalling(mOwner);
            if (mCallSession != null) {
                mCallId = mCallSession.getCallId();
            }
            mAudioManager.requestAudioFocusForCall(AudioManager.STREAM_VOICE_CALL,
                    AudioManager.AUDIOFOCUS_GAIN_TRANSIENT);
            mAudioManager.setMode(AudioManager.MODE_IN_CALL);
            mSipManager.notifyCallSessionCalling(mCallId, mOwner);
        }

        @Override
        public void callSessionConferenceStateUpdated(ImsCallSession session,
                ImsConferenceState state) {
            Log.v(TAG,"CallSessionConferenceStateUpdated :: state=" + state);

            notifyConferenceStateUpdated(state);
        }
    }

}
