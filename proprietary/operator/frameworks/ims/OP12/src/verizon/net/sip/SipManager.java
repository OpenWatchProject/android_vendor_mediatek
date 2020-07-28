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

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.ConditionVariable;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.net.sip.SipRegistrationListener;
import android.content.ServiceConnection;
import android.net.sip.SipException;
import android.net.Uri;
import android.telephony.ims.ImsCallSession;
import android.telephony.ims.ImsCallProfile;
import android.util.Pair;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.hardware.camera2.CameraManager;
import android.telephony.ims.ImsConferenceState;

import com.android.ims.ImsManager;
import com.android.ims.internal.IImsCallSession;
import com.android.internal.telecom.IVideoCallback;

import com.mediatek.ims.MtkImsConnectionStateListener;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.MtkImsConstants;
import com.mediatek.ims.internal.MtkImsCallSession;
import com.mediatek.ims.internal.IMtkImsCallSession;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Hashtable;

public class SipManager {
    private static final String TAG = "SipManager";
    private static Context mContext;
    public int mRegType = 87;
    private SipRegistrationListener mSipRegistrationListener;
    //private ISipService iSipService;
    private static ServiceConnection mConnection;
    private static SipMediaCall mSipMediaCall;
    private static SipManager sInstance = null;
    private static EABService mEABService;
    protected static final Object mLock = new Object();

    private ArrayList<Pair<Integer, PendingIntent>> mIncomingCallPendingIntentList;
    private ArrayList<Pair<Integer, PendingIntent>> mincomingMessagePendingIntentList;
    private int mPhoneId = -1;
    private MtkImsManager mMtkImsManager;
    private PendingIntent mIncomingCallPendingIntent;
    private PendingIntent mIncomingMessagePendingIntent;
    private HashMap<String, SipMediaCall> mCallList;
    private ArrayList <SipMediaCall> CallListenerList  ;
    private String mPendingMTCallId;
    private String[] mIncomingCallInfo;
    private boolean mCallWaitingRules = false;
    private ConditionVariable mConferenceConditionVariable = new ConditionVariable();
    private ImsCallSession mConferenceSession = null;

    private SipManager(Context paramContext) {
        if (paramContext == null) {
            throw new NullPointerException("context must not be null");
        }
        mContext = paramContext;
        //create aidl service if needed
        /*
        iSipService = ISipService.Stub.asInterface(
                ServiceManager.getService("ClientAPI"));
        if (iSipService == null) {
            Rlog.d(LOG_TAG, "Can not access ISipService");
            return;
        }*/
        mPhoneId = MtkImsManager.getMainPhoneIdForSingleIms(mContext);
        mMtkImsManager = (MtkImsManager) ImsManager.getInstance(mContext, mPhoneId);
        mCallList = new HashMap<String, SipMediaCall>();
        CallListenerList = new ArrayList();
        Log.v(TAG, "new SipManager Success : " );
    }

    public static SipManager newInstance(Context context) {
        Log.v(TAG, "Client api TC: newInstance " );
        synchronized (mLock) {
            if (sInstance == null && context != null) {
                sInstance = new SipManager(context);
                mEABService = new EABService(context);
                Log.d(TAG, "SipManager is created!");
            }
            return sInstance;
        }
        // return new SipManager(context);
    }

    public void open(PendingIntent incomingCallPendingIntent,
                     SipRegistrationListener listener,
                     int priority) {
        Log.v(TAG, "Client api TC: open " );

        /* TODO: Mutiple apk version.
        Log.d(TAG, "priority: " + priority + "PendingIntent: " + incomingCallPendingIntent);
        Pair p = new Pair(prority, incomingCallPendingIntent);
        mIncomingCallPendingIntentList.add(p);
        Collections.sort(mIncomingCallPendingIntentList, new Comparator<Pair<Integer, PendingIntent>>() {
            @Override
            public int compare(Pair<Integer, PendingIntent> p1, Pair<Integer, PendingIntent> p2) {
                if (p1.first > p2.first) {
                    return 1;
                }
                return -1;
            }
        });
        */
        mIncomingCallPendingIntent = incomingCallPendingIntent;

        if (mSipRegistrationListener == null) {
            setRegistrationListener(listener);
        }

        try {
            mMtkImsManager.setMTRedirect(true);
        } catch (Throwable ex) {
            Log.e (TAG, "setMTRedirect fail" + ex);
        }
    }

    public SipDialog openSipDialog(PendingIntent incomingMessagePendingIntent,
                                   HashMap <String,String> filter,
                                   SipRegistrationListener listener,
                                   int priority) throws SipException{
        Log.v(TAG, "Client api TC: openSipDialog " );

        mIncomingMessagePendingIntent = incomingMessagePendingIntent;

        if (mSipRegistrationListener == null) {
            setRegistrationListener(listener);
        }

        SipDialog sipdialog = null;
        try {

            sipdialog = new SipDialog (mContext,mIncomingMessagePendingIntent);
            //sipdialog.setListener(listener, true);


        } catch (Throwable ex) {
            Log.e (TAG, "SipDialog initial fail" + ex);
            throw new SipException("openSipDialog()");
        }
        Log.v(TAG, "Client api TC: openSipDialog success" );
        return sipdialog;

    }

    public void setRegistrationListener(SipRegistrationListener listener) {
        mSipRegistrationListener = listener;
        try {
            mMtkImsManager.addImsConnectionStateListener(mImsStateListener);
        } catch (Throwable ex) {
            Log.e (TAG, "addImsConnectionStateListener fail" + ex);
        }
    }

    public void setCallingListener(SipMediaCall.Listener listener) {
        Log.v(TAG, "setCallingListener");

        MediaCall mMediaCall = new MediaCall(mContext);
        mMediaCall.setListener(listener, true);
        CallListenerList.add(listener.mOwner);

        SipProfile caller = null;
        //SipDialog sipdialog = null;
        //SipDialogListener DialogListener = new SipDialogListener();
        //DialogListener.setup(mMediaCall, listener);
        try {
            
            Log.v(TAG, "whattt1" );
            Thread.sleep(1000);
            listener.onRinging(mMediaCall,caller);
            /*Log.v(TAG, "whattt2" );
            Thread.sleep(1000);
            listener.onCallEstablished(mMediaCall);
            Log.v(TAG, "whattt3" );
            Thread.sleep(1000);
            listener.onCallEnded(mMediaCall);
            Log.v(TAG, "whattt4" );
            sipdialog = new SipDialog (mContext,mIncomingMessagePendingIntent);
            SipDialog.setListener(null, DialogListener);*/
            
        } catch (Throwable ex) {
            Log.e (TAG, "setCallingListener fail" + ex);
        }



    }

    public void resetCallingListener(SipMediaCall.Listener listener) {
        Log.v(TAG, "resetCallingListener");
        CallListenerList.remove(listener.mOwner);
        //mSipMediaCall.setListener(null,true);
    }

    public SipMediaCall makeMediaCall(String peerUri,
            SipMediaCall.Listener listener, int type, int timeout) throws SipException{
        try {
            return makeMediaCall(peerUri, listener, type, timeout, null, null);
        } catch (Throwable ex) {
            Log.e (TAG, "SipMediaCall initial fail" + ex);
        }
        return null;
    }

    public SipMediaCall makeMediaCall(String peerUri, SipMediaCall.Listener listener,
            int type, int timeout, HashMap<String,String> extraHeaders) throws SipException{
        try {
            return makeMediaCall(peerUri, listener, type, timeout, extraHeaders, null);
        } catch (Throwable ex) {
            Log.e (TAG, "SipMediaCall initial fail" + ex);
        }
        return null;
    }

    public SipMediaCall makeMediaCall(String peerUri, SipMediaCall.Listener listener, int type,
            int timeout, HashMap<String,String> extraHeaders, String fromUri) throws SipException{

        // TODO: Check peerUri and type valid or not.

        Log.v(TAG, "make Call from normal req 2");
        boolean canMakeCall = checkCallWaitingRules();
        if (!canMakeCall) {
            throw new SipException("makeMediaCall() Can't make new call due to call waiting rules");
        }

        SipMediaCall sipMediaCall = null;
        try {
            mMtkImsManager.setSipHeader(extraHeaders, fromUri);

            sipMediaCall = new SipMediaCall(mContext);
            sipMediaCall.setListener(listener, true);
            sipMediaCall.mCurrentType = type;
            ImsCallProfile imsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                    sipCallTypeToImsCallType(type));
            ImsCallSession session = mMtkImsManager.createCallSession(ImsCallProfile.SERVICE_TYPE_NORMAL,sipCallTypeToImsCallType(type));


            sipMediaCall.start((MtkImsCallSession) session, peerUri, imsCallProfile);
        } catch (Throwable ex) {
            Log.e (TAG, "SipMediaCall initial fail" + ex);
            throw new SipException("makeMediaCall()");
        }
        return sipMediaCall;
    }

    public void makeCall(String peerUri, int timeout, HashMap<String,String> extraHeaders, SipMediaCall sipMediaCall) throws SipException{

        // TODO: Check peerUri and type valid or not.
        Log.v(TAG, "make Call from dep req");
        boolean canMakeCall = checkCallWaitingRules();
        if (!canMakeCall) {
            throw new SipException("makeCall() Can't make new call due to call waiting rules");
        }
        try {
            mMtkImsManager.setSipHeader(extraHeaders, null);
            int type = 0;
            ImsCallProfile imsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                    sipCallTypeToImsCallType(type));
            ImsCallSession session = mMtkImsManager.createCallSession(ImsCallProfile.SERVICE_TYPE_NORMAL,sipCallTypeToImsCallType(type));
            sipMediaCall.start((MtkImsCallSession) session, peerUri, imsCallProfile);
        } catch (Throwable ex) {
            Log.e (TAG, "MakeCall from SipMediaCall fail" + ex);
            throw new SipException("makeCall()");
        }
    }

    private int sipCallTypeToImsCallType(int type) {
        int callType = ImsCallProfile.CALL_TYPE_VOICE;
        switch(type) {
            case SipMediaCall.AUDIO_CALL:
                callType = ImsCallProfile.CALL_TYPE_VOICE;
                break;
            case SipMediaCall.VIDEO_SHARE:
                callType = ImsCallProfile.CALL_TYPE_VT_TX;
                break;
            case SipMediaCall.VIDEO_CALL:
                callType = ImsCallProfile.CALL_TYPE_VT;
                break;
            case SipMediaCall.INBOUND_ONLY_VIDEO_CALL:
                callType = ImsCallProfile.CALL_TYPE_VT_RX;
                break;
            default:
                break;
        }
        return callType;
    }
    public SipMediaCall takeMediaCall(Intent incomingCallIntent, SipMediaCall.Listener listener) {
        try {
            return takeMediaCall(incomingCallIntent, listener, null);
        } catch (Throwable ex) {
            Log.e (TAG, "SipMediaCall initial fail" + ex);
        }
        return null;
    }

    public SipMediaCall takeMediaCall(Intent incomingCallIntent,
            SipMediaCall.Listener listener, HashMap<String,String> headers) throws SipException {
        // will deal with this call, but not sure to answer or reject or ...

        Log.v(TAG, "Client api TC: takeMediaCall : "+mIncomingCallInfo );
        String callId = mIncomingCallInfo[0];
        String dialString = mIncomingCallInfo[1];
        String callMode = mIncomingCallInfo[3];
        String seqNum = mIncomingCallInfo[4];
        String toLineNum = mIncomingCallInfo[6];

        SipMediaCall sipMediaCall;

        try {
            mMtkImsManager.setSipHeader(headers, null);
            mMtkImsManager.setCallIndication(mPhoneId, callId, dialString,
                    Integer.parseInt(seqNum), toLineNum, true);
            IMtkImsCallSession mtkSession = mMtkImsManager.getPendingMtkCallSession(callId);//mPendingMTCallId);
            if (mtkSession == null) {
                throw new SipException(
                        "No pending IMtkImsCallSession for the call with call Id : "+ callId);
            }

            IImsCallSession session = mtkSession.getIImsCallSession();

            sipMediaCall = new SipMediaCall(mContext);
            sipMediaCall.attachSession(new MtkImsCallSession(session, mtkSession));
            sipMediaCall.setListener(listener, true);
            // sipMediaCall.takeMediaCall(incomingCallIntent,headers);

            mCallList.put(callId, sipMediaCall);
        } catch (Throwable ex) {
            Log.e (TAG, "new SipMediaCall fail" + ex);
            throw new SipException("takeMediaCall()");
        }
        return sipMediaCall;
    }

    public boolean isRegistered(String localProfileUri) {
        if (mRegType == MtkImsConstants.IMS_REGISTERED) {
            return true;
        } else {
            return true;
        }
    }
    public String getRegisterResponse(){
        if(mRegType == MtkImsConstants.IMS_REGISTERED){
            Log.v(TAG, "Client api TC: getRegisterResponse :: IMS_REGISTERED");
            return "SIP/2.0 200 OK,SIP/2.0 200 OK";
        }
        else if(mRegType == MtkImsConstants.IMS_REGISTERING){
            Log.v(TAG, "Client api TC: getRegisterResponse :: IMS_REGISTERING");
            return "IMS_REGISTERING,IMS_REGISTERING";
        }
        else {
            Log.v(TAG, "Client api TC: getRegisterResponse :: IMS_REGISTERFAIL type->" + mRegType);
            return "IMS_REGISTERFAIL,IMS_REGISTERFAIL";
        }
    }
    public void enableCallWaitingRules(boolean enableRule) {
        // allows the VoLTE for Business application to enable or disable the automatic limitation of only 2 simultaneous calls
        mCallWaitingRules = enableRule;
    }

    public int getCall() {
        //Returns one of the SipMediaCall types if the device is already in a call or -1 if device is not in a call.
        if (mCallList.isEmpty()) {
            return -1;
        } else {
            for (SipMediaCall call : mCallList.values()) {
                return call.mCurrentType;
            }
            return -1;
        }
    }

    public void ignoreMediaCall(Intent intent) {
        // trigger fallback to normal call
        Log.v(TAG, "Client api TC: ignoreMediaCall " );
        try {
            mMtkImsManager.fallBackAospMTFlow();
        } catch (Throwable ex) {
            Log.e (TAG, "fallBackAospMTFlow exception" + ex);
        }
    }

    // return false if can't handle new call.
    private boolean checkCallWaitingRules() {
        if (mCallWaitingRules && mCallList.size() >= 2) {
            Log.e(TAG, "checkCallWaitingRules call list size : " + mCallList.size());
            return false;
        }
        return true;
    }

    public SipMediaCall mergeMediaCalls(SipMediaCall call1, SipMediaCall call2, int timeout) throws SipException {
        return mergeMediaCalls(call1, call2, timeout, null);
    }

    public SipMediaCall mergeMediaCalls(SipMediaCall call1,
                                        SipMediaCall call2,
                                        int timeout,
                                        HashMap<String,String> extraHeaders) throws SipException {
        // TODO: Check call 1 and call 2 is voice call.
        SipMediaCall sipMediaCall;

        try {
            mMtkImsManager.setSipHeader(extraHeaders, null);
            mConferenceConditionVariable.close();
            call1.merge();
            mConferenceConditionVariable.block();

            sipMediaCall = new SipMediaCall(mContext);
            sipMediaCall.attachSession((MtkImsCallSession) mConferenceSession);
        } catch (Throwable ex) {
            Log.e (TAG, "SipMediaCall merge fail" + ex);
            throw new SipException("mergeMediaCalls()");
        }
        return sipMediaCall;
    }

    public void notifyCallSessionMergeComplete(ImsCallSession newSession) {
        Log.v(TAG, "notifyCallSessionMergeComplete: newSession " + newSession);
        mConferenceSession = newSession;
        mConferenceConditionVariable.open();
    }

    public void notifyCallSessionCalling(String callId, SipMediaCall sipMediaCall) {
        Log.v(TAG, "notifyCallSessionCalling: Add call to list: " + callId
                + ", SipMediaCall " + sipMediaCall);
        mCallList.put(callId, sipMediaCall);
    }

    public void notifyCallSessionTerminated(String callId) {
        if (mCallList.containsKey(callId)) {
            Log.v(TAG, "notifyCallSessionTerminated: Remove call from list: " + callId);
            mCallList.remove(callId);
        }
        Log.e(TAG, "notifyCallSessionTerminated: No this call in the list: " + callId);
    }

    private MtkImsConnectionStateListener mImsStateListener = new MtkImsConnectionStateListener() {
        /**
         * M: Called when IMS registration state changed.
         */
        @Override
        public void onRegistrationImsStateInd(int indType, Uri[] uris, int expireTime, int errCode, String errMsg) {
            Log.v(TAG, "onRegistrationImsStateInd: " + indType + " Uris : "+uris.toString());
            // store the oosState in here and let apk to query
            String localProfileUri = "??";
            mRegType = indType;
            if (mRegType == MtkImsConstants.IMS_REGISTERED){
                mSipRegistrationListener.onRegistrationDone(localProfileUri, expireTime);
            } else if (mRegType == MtkImsConstants.IMS_REGISTERING){
                mSipRegistrationListener.onRegistering(localProfileUri);
            } else {
                mSipRegistrationListener.onRegistrationFailed(localProfileUri, errCode, errMsg);
            }
        }

        @Override
        public void onRedirectIncomingCallInd(int phoneId, String[] info) {
            // +EAIC:<call_id>,<number>,<type>,<call_mode>,<seq_no>[,<redirect_num>[,<digit_line_num>]]
            Log.v(TAG, "onRedirectIncomingCallInd: " + phoneId);
            mIncomingCallInfo = info;
            String callId = info[0];
            String dialString = info[1];
            String callMode = info[3];
            String seqNum = info[4];
            String toLineNum = info[6];
            Log.d(TAG, "onRedirectIncomingCallInd() : call_id = " + callId +
                    " dialString = " +  dialString + " seqNum = " + seqNum +
                    " phoneId = " + phoneId);
            try {
                boolean canHandle = checkCallWaitingRules();
                if (canHandle) {
                    // TODO: need to fullfill the "sip_content" in extras
                    mIncomingCallPendingIntent.send();
                } else {
                    mMtkImsManager.setCallIndication(mPhoneId, callId, dialString,
                            Integer.parseInt(seqNum), toLineNum, false);
                }
            } catch (Throwable ex) {
                Log.e (TAG, "onRedirectIncomingCallInd exception" + ex);
            }
        }
    };
    public class MediaCall extends SipMediaCall{
        //@Override
        public MediaCall(Context InContext){
            super(InContext);
        }
        @Override
        public void start(MtkImsCallSession session, String callee, ImsCallProfile CallProfile) throws SipException{}
        @Override
        public void makeCall(String peerUri,HashMap<String,String> extraHeaders)throws SipException{}
        @Override
        public void makeCall(String peerUri, int timeout,HashMap<String,String> extraHeaders)throws SipException{}
        @Override
        public void takeMediaCall(Intent incomingCallIntent, HashMap<String,String> headers){}
        //@Override
        //public void attachSession(MtkImsCallSession session){}
        @Override
        public void answerCall(int timeout) {}
        @Override
        public void answerCallAudioOnly(int timeout) {}
        @Override
        public void close() {}
        @Override
        public void continueCall(int timeout) {}
        @Override
        public void endCall() {}
        @Override
        public void holdCall(int timeout) {}
        @Override
        public void sendDtmf(int code) {}
        //@Override
        //public void setListener(Listener listener, boolean callbackImmediately) {}
        @Override
        public void setSpeakerMode(boolean speakerMode) {}
        @Override
        public void toggleMute() {}
        @Override
        public void rejectCall(int timeout) {}
        @Override
        public void setVolume(int index) {}
        @Override
        public int getMaxVolume() {return 0;}
        @Override
        public void removeLastParticipant(int timeout) {}
        @Override
        public int typeToProfileState(int type) {return 0;}
        @Override
        public int profileStateToType(int videoState) {return 0;}
        @Override
        public void upgradeCall(int type, String cameraId, Surface incomingVideo, Surface outgoingVideo) {}
        @Override
        public void upgradeCall(int type,int cameraId,SurfaceView incomingVideo,SurfaceHolder outgoingVideo) {}
        @Override
        public void upgradeCall(int type) {}
        @Override
        public void upgradeAudioCall(int type) {}
        @Override
        public void downgradeCall(int type) {}
        @Override
        public void setCamera(int id, SurfaceHolder surfaceHolder) {}
        @Override
        public void setCamera(int id, Surface surface) {}
        @Override
        public void setCamera(String id, Surface surface) {}
        @Override
        public void setDisplay(SurfaceView surfaceView) {}
        @Override
        public void setDisplay(SurfaceHolder incomingVideo) {}
        @Override
        public void setDisplay(Surface surface) {}
        @Override
        public void changeCall(int type) {}
        @Override
        public synchronized void setVideoHold(boolean isHold) {}
        @Override
        public void holdVideo() {}
        @Override
        public void continueVideo() {}
        @Override
        public void acceptChangeRequest(int timeout) {}
        @Override
        public void rejectChangeRequest(int timeout) {}
        @Override
        public void merge() {}
        //@Override
        private synchronized void notifyConferenceStateUpdated(ImsConferenceState state) {}
        public class SmallListener {}
    }
/*
    private class SipDialogListener extends SipDialog.Listener {
        private SipMediaCall.Listener mlistener = null;
        private SipMediaCall mCall = null;
        private SipDialogListener() {
        }
        private setup(SipMediaCall SipCall, SipMediaCall.Listener listener){
            mlistener = listener;
            mCall = SipCall;
        }
        public void onSipReceived(String sipMessage) {
            Log.v(TAG, "setCallingListener :: onSipReceived :"+ sipMessage);
            SipProfile caller = null;
            mlistener.onRinging(mCall, caller)
        }
    }
   */
}
