/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.ims;

import java.util.Objects;

import android.content.Context;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSession;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.util.Log;

import com.android.ims.ImsCall;
import com.android.ims.ImsException;

import com.android.internal.annotations.VisibleForTesting;
import com.mediatek.ims.internal.MtkImsCallSession;

public class MtkImsCall extends ImsCall {
    private static final String TAG = "MtkImsCall";

    private static final int UPDATE_ECT = 7;
    private static final int UPDATE_DEVICE_SWITCH = 8;

    /**
     * Create an IMS call object.
     *
     * @param context the context for accessing system services
     * @param profile the call profile to make/take a call
     */
    public MtkImsCall(Context context, ImsCallProfile profile) {
        super(context, profile);
    }

    public static class Listener extends ImsCall.Listener {
        // ALPS02256671. For PAU information changed.
        /**
         * Notifies of a change to the PAU information for this {@code ImsCall}.
         *
         * @param imsCall The IMS call.
         */
        public void onPauInfoChanged(ImsCall imsCall) {
        }

        /**
         * Called when the call is transferred.
         *
         * @param call the call object that carries out the IMS call
         */
        public void onCallTransferred(ImsCall call) {
            // no-op
        }

        /**
         * Called when the call transfer is failed.
         * The default implementation calls {@link #onCallError}.
         *
         * @param call the call object that carries out the IMS call
         * @param reasonInfo detailed reason of the call transfer failure
         */
        public void onCallTransferFailed(ImsCall call, ImsReasonInfo reasonInfo) {
            onCallError(call, reasonInfo);
        }

        public void onTextCapabilityChanged(ImsCall call,
                int localCapability, int remoteCapability,
                int localTextStatus, int realRemoteCapability) {
            // no-op
        }

        public void onRttEventReceived(ImsCall call, int event) {
            // no-op
        }

        public void onCallDeviceSwitched(ImsCall imsCall) {
            // no-op
        }

        public void onCallDeviceSwitchFailed(ImsCall imsCall, ImsReasonInfo reasonInfo) {
            // no-op
        }

        public void onCallRedialEcc(ImsCall imsCall, boolean isNeedUserConfirm) {
            // no-op
        }
    }

    // used for merging a call to a conference call
    private boolean mIsConferenceMerging = false;

    // used for conference XML update.
    private String mAddress = null;

    /**
     * Initiates an IMS call with the call profile which is provided
     * when creating a {@link ImsCall}.
     *
     * @param session the {@link ImsCallSession} for carrying out the call
     * @param callee callee information to initiate an IMS call
     * @throws ImsException if the IMS service fails to initiate the call
     */
    @Override
    public void start(ImsCallSession session, String callee)
            throws ImsException {
        super.start(session, callee);
        mAddress = callee;
    }

    /**
     * Initiates an IMS conferenca call with the call profile which is provided
     * when creating a {@link ImsCall}.
     *
     * @param session the {@link ImsCallSession} for carrying out the call
     * @param participants participant list to initiate an IMS conference call
     * @throws ImsException if the IMS service fails to initiate the call
     */
    @Override
    public void start(ImsCallSession session, String[] participants)
            throws ImsException {
        super.start(session, participants);
        /// If not set conference host for one-key conference, the conference will not be
        /// conferenceable in ImsConferenceController.recalculateConferenceable(). This will
        /// cause there is no merge button in UI.
        if (mCallProfile != null
                && mCallProfile.getCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE)) {
            mIsConferenceHost = true;
        }
    }

    @Override
    protected void setTransientSessionAsPrimary(ImsCallSession transientSession) {
        synchronized (MtkImsCall.this) {
            // ALPS01995879
            if (mSession != null) {
                mSession.setListener(null);
            }
            mSession = transientSession;
            // ALPS01995879.
            if (mSession != null) {
                mSession.setListener(createCallSessionListener());
                //ALPS02444330
                // When merging video call with voice call, call type of transient conference
                // session might be difference from call type of final host call. Need to update
                // call type of transient conference session to final host call.
                ImsCallProfile imsCallProfile = mSession.getCallProfile();
                if (imsCallProfile != null) {
                    mCallProfile.updateCallType(imsCallProfile);
                }
            }
        }
    }

    // For one-key conference MT displayed as incoming conference call.
    /**
     * Determines if the incoming call is a multiparty call.
     *
     * @return {@code True} if the incoming call is a multiparty call.
     * @hide
     */
    public boolean isIncomingCallMultiparty() {
        synchronized (mLockObj) {
            if (mSession == null || !(mSession instanceof MtkImsCallSession)) {
                return false;
            }

            return ((MtkImsCallSession)mSession).isIncomingCallMultiparty();
        }
    }

    /**
     * Approve ECC redial or not.
     *
     * @param isAprroved User approve or disapprove ECC redial
     *
     */
    public void approveEccRedial(boolean isAprroved) {
        logi("approveEccRedial :: ");
        synchronized (mLockObj) {
            if (mSession == null || !(mSession instanceof MtkImsCallSession)) {
                return;
            }

            ((MtkImsCallSession)mSession).approveEccRedial(isAprroved);
        }
    }

    /**
     * Explicit call transfer.
     *
     * @throws ImsException if the IMS service fails to hold the call
     */
    public void explicitCallTransfer() throws ImsException {
        logi("explicitCallTransfer :: ");

        synchronized (mLockObj) {
            if (mUpdateRequest != UPDATE_NONE) {
                loge("explicitCallTransfer :: update is in progress; request=" +
                        updateRequestToString(mUpdateRequest));
                throw new ImsException("Call update is in progress",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
            }

            if (mSession == null) {
                throw new ImsException("No call session",
                        ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED);
            }

            if (!(mSession instanceof MtkImsCallSession)) {
                return;
            }
            ((MtkImsCallSession)mSession).explicitCallTransfer();
            // FIXME: We should update the state on the callback because that is where
            // we can confirm that the hold request was successful or not.

            mUpdateRequest = UPDATE_ECT;
        }
    }

    /**
     * Unattended call transfer.
     *
     * @param number The transfer target number.
     * @param type ECT type
     * @throws ImsException if the IMS service fails to transfer the call
     */
    public void unattendedCallTransfer(String number, int type) throws ImsException {
        logi("explicitCallTransfer :: ");

        synchronized (mLockObj) {
            if (mUpdateRequest != UPDATE_NONE) {
                loge("explicitCallTransfer :: update is in progress; request=" +
                        updateRequestToString(mUpdateRequest));
                throw new ImsException("Call update is in progress",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
            }

            if (mSession == null) {
                throw new ImsException("No call session",
                        ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED);
            }

            if (!(mSession instanceof MtkImsCallSession)) {
                return;
            }
            ((MtkImsCallSession)mSession).unattendedCallTransfer(number, type);
            // FIXME: We should update the state on the callback because that is where
            // we can confirm that the hold request was successful or not.

            mUpdateRequest = UPDATE_ECT;
        }
    }

    /**
     * Switch the active call to specific device.
     *
     * @param number The switch target number.
     * @param deviceId The switch target deviceId.
     * @throws ImsException if the IMS service fails to transfer the call
     */
    public void deviceSwitch(String number, String deviceId) throws ImsException {
        logi("deviceSwitch :: ");

        synchronized (mLockObj) {
            if (mUpdateRequest != UPDATE_NONE) {
                loge("deviceSwitch :: update is in progress; request=" +
                        updateRequestToString(mUpdateRequest));
                throw new ImsException("Call update is in progress",
                        ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
            }

            if (mSession == null) {
                throw new ImsException("No call session",
                        ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED);
            }

            if (!(mSession instanceof MtkImsCallSession)) {
                return;
            }
            ((MtkImsCallSession)mSession).deviceSwitch(number, deviceId);

            mUpdateRequest = UPDATE_DEVICE_SWITCH;
        }
    }

    /**
     * Cancel the device switch
     */
    public void cancelDeviceSwitch() throws ImsException {
        logi("cancelDeviceSwitch :: ");

        synchronized (mLockObj) {
            if (!(mSession instanceof MtkImsCallSession)) {
                return;
            }

            ((MtkImsCallSession)mSession).cancelDeviceSwitch();
        }
    }

    @Override
    protected ImsCall createNewCall(ImsCallSession session, ImsCallProfile profile) {
        ImsCall call = new MtkImsCall(mContext, profile);

        try {
            call.attachSession(session);
        } catch (ImsException e) {
            if (call != null) {
                call.close();
                call = null;
            }
        }

        // Do additional operations...

        return call;
    }

    /**
     * Creates an IMS call session listener.
     */
    @Override
    protected ImsCallSession.Listener createCallSessionListener() {
        mImsCallSessionListenerProxy = new MtkImsCallSessionListenerProxy();
        return mImsCallSessionListenerProxy;
    }

    @VisibleForTesting
    public class MtkImsCallSessionListenerProxy extends ImsCallSessionListenerProxy {

        // ALPS03894085, Peer.mSession is terminated before processmergesuccess,
        // JE will happened if we perform mSession.getCallId().
        @Override
        protected boolean doesCallSessionExistsInMerge(ImsCallSession cs) {
            String callId = cs.getCallId();
            // ALPS03909130, ImsCall might be null object
            String hostId = (mMergeHost == null) ? "" : (mMergeHost.mSession == null) ? "" :
                            mMergeHost.mSession.getCallId();
            String peerId = (mMergePeer == null) ? "" : (mMergePeer.mSession == null) ? "" :
                            mMergePeer.mSession.getCallId();
            String sessionId = (mSession == null) ? "" : mSession.getCallId();

            return ((isMergeHost() && Objects.equals(peerId, callId)) ||
                    (isMergePeer() && Objects.equals(hostId, callId)) ||
                    Objects.equals(sessionId, callId));
        }

        @Override
        public void callSessionMergeComplete(ImsCallSession newSession) {
            logi("callSessionMergeComplete :: newSession =" + newSession);
            if (!isMergeHost()) {
                // ALPS02502328, while merging 1A1H, user answer the incoming call,
                // the held call will be terminated and merge info will be clear.
                // mMergeHost will set to null.
                // Terminate the conference call if mMergeHost is null.
                if (mMergeHost == null) {
                    logd("merge host is null, terminate conf");
                    if (newSession != null) {
                        // terminate the created conference session
                        newSession.terminate(ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
                    }
                    return;
                }
                // Handles case 4
                mMergeHost.processMergeComplete();
            } else {
                // Handles case 1, 2, 3
                if (newSession != null) {
                    mTransientConferenceSession = doesCallSessionExistsInMerge(newSession) ?
                            null: newSession;
                    if(mTransientConferenceSession == null) {
                        logi("callSessionMergeComplete :: callSessionExisted.");
                    }
                }
                // Handles case 5
                processMergeComplete();
            }
        }

        // MTK added listener methods
        @Override
        public void callSessionTransferred(ImsCallSession session) {
            loge("callSessionTransferred :: session=" + session);

            ImsCall.Listener listener;

            synchronized (MtkImsCall.this) {
                mUpdateRequest = UPDATE_NONE;
                listener = mListener;
            }

            if (listener != null && listener instanceof MtkImsCall.Listener) {
                try {
                    ((MtkImsCall.Listener)listener).onCallTransferred(MtkImsCall.this);
                } catch (Throwable t) {
                    loge("callSessionTransferred :: ", t);
                }
            }
        }

        @Override
        public void callSessionTransferFailed(ImsCallSession session, ImsReasonInfo reasonInfo) {
            loge("callSessionTransferFailed :: session=" + session + " reasonInfo=" + reasonInfo);

            ImsCall.Listener listener;

            synchronized (MtkImsCall.this) {
                mUpdateRequest = UPDATE_NONE;
                listener = mListener;
            }

            if (listener != null && listener instanceof MtkImsCall.Listener) {
                try {
                    ((MtkImsCall.Listener)listener).onCallTransferFailed(MtkImsCall.this, reasonInfo);
                } catch (Throwable t) {
                    loge("callSessionTransferFailed :: ", t);
                }
            }
        }

        @Override
        public void callSessionDeviceSwitched(ImsCallSession session) {
            loge("callSessionTransferred :: session=" + session);

            ImsCall.Listener listener;

            synchronized (MtkImsCall.this) {
                mUpdateRequest = UPDATE_NONE;
                listener = mListener;
            }

            if (listener != null && listener instanceof MtkImsCall.Listener) {
                try {
                    ((MtkImsCall.Listener)listener).onCallDeviceSwitched(MtkImsCall.this);
                } catch (Throwable t) {
                    loge("callSessionDeviceSwitched :: ", t);
                }
            }
        }

        @Override
        public void callSessionDeviceSwitchFailed(
                ImsCallSession session, ImsReasonInfo reasonInfo) {
            loge("callSessionDeviceSwitchedFailed :: session=" +
                    session + " reasonInfo=" + reasonInfo);

            ImsCall.Listener listener;

            synchronized (MtkImsCall.this) {
                mUpdateRequest = UPDATE_NONE;
                listener = mListener;
            }

            if (listener != null && listener instanceof MtkImsCall.Listener) {
                try {
                    ((MtkImsCall.Listener)listener).onCallDeviceSwitchFailed(
                            MtkImsCall.this, reasonInfo);
                } catch (Throwable t) {
                    loge("callSessionDeviceSwitchedFailed :: ", t);
                }
            }
        }

        @Override
        public void callSessionTextCapabilityChanged(ImsCallSession session,
                int localCapability, int remoteCapability,
                int localTextStatus, int realRemoteCapability) {
            ImsCall.Listener listener;

            synchronized (MtkImsCall.this) {
                listener = mListener;
            }

            if (listener != null && listener instanceof MtkImsCall.Listener) {
                try {
                    ((MtkImsCall.Listener)listener).onTextCapabilityChanged(MtkImsCall.this,
                        localCapability, remoteCapability, localTextStatus, realRemoteCapability);
                } catch (Throwable t) {
                    loge("callSessionTextCapabilityChanged :: ", t);
                }
            }
        }

        @Override
        public void callSessionRttEventReceived(ImsCallSession session, int event) {
            ImsCall.Listener listener;
            synchronized (MtkImsCall.this) {
                listener = mListener;
            }
            if (listener != null && listener instanceof MtkImsCall.Listener) {
                try {
                    ((MtkImsCall.Listener)listener).onRttEventReceived(MtkImsCall.this, event);
                } catch (Throwable t) {
                    loge("callSessionRttEventReceived :: ", t);
                }
            }
        }

        @Override
        public void callSessionRedialEcc(ImsCallSession session, boolean isNeedUserConfirm) {
            loge("callSessionRedialEcc :: session=" + session);
            ImsCall.Listener listener;
            synchronized (MtkImsCall.this) {
                listener = mListener;
            }
            if (listener != null && listener instanceof MtkImsCall.Listener) {
                try {
                    ((MtkImsCall.Listener)listener).onCallRedialEcc(MtkImsCall.this, isNeedUserConfirm);
                } catch (Throwable t) {
                    loge("callSessionRedialEcc :: ", t);
                }
            }
        }

        @Override
        public void callSessionRinging(ImsCallSession session, ImsCallProfile callProfile) {
            // no-op
        }

        @Override
        public void callSessionBusy(ImsCallSession session) {
            // no-op
        }

        @Override
        public void callSessionCalling(ImsCallSession session) {
            // no-op
        }
    }

    /**
     * Provides a human-readable string representation of an update request.
     *
     * @param updateRequest The update request.
     * @return The string representation.
     */
    @Override
    protected String updateRequestToString(int updateRequest) {
        switch (updateRequest) {
            case UPDATE_ECT:
                return "ECT";
            case UPDATE_DEVICE_SWITCH:
                return "DEVICE_SWITCH";
            default:
                return super.updateRequestToString(updateRequest);
        }
    }

    /**
     * Log a string to the radio buffer at the info level.
     * @param s The message to log
     */
    @Override
    protected void logi(String s) {
        Log.i(TAG, appendImsCallInfoToString(s));
    }

    /**
     * Append the ImsCall information to the provided string. Usefull for as a logging helper.
     * @param s The original string
     * @return The original string with {@code ImsCall} information appended to it.
     */
    private String appendImsCallInfoToString(String s) {
        StringBuilder sb = new StringBuilder();
        sb.append(s);
        sb.append(" MtkImsCall=");
        sb.append(MtkImsCall.this);
        return sb.toString();
    }

    @Override
    protected void copyCallProfileIfNecessary(ImsStreamMediaProfile profile) {
        if (mCallProfile != null) {
            mCallProfile.mMediaProfile.copyFrom(profile);
        }
    }

    @Override
    protected void checkIfConferenceMerge(ImsReasonInfo reasonInfo) {
        // ALPS02064606.
        // If it goes here, it should be belong to merging a normal call into a existing
        // conference case, and there is no transient conference session. If it happens,
        // processMergeFailed() should be done to clear any merge info.
        if (isCallSessionMergePending() && isConferenceHost()) {
            logi("this is a conference host during merging, and is disconnected..");
            processMergeFailed(reasonInfo);
        }
    }

    @Override
    protected void updateHoldStateIfNecessary(boolean hold) {
        // ALPS02327894.
        // Workaround for swapping before normal call merge conference call. Update hold
        // state. Need to remove when swapping behavior is removed.
        mHold = hold;
    }

    @Override
    protected boolean shouldSkipResetMergePending() {
        return true;
    }

    @Override
    protected void resetConferenceMergingFlag() {
        mIsConferenceMerging = false;
    }

    public void setTerminationRequestFlag(boolean result) {
        mTerminationRequestPending = result;
    }
}
