/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.services.telephony;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.ServiceState;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.os.SomeArgs;
import com.android.internal.telephony.Phone;

/// M: CC: ECC phone selection rule @{
import com.mediatek.internal.telephony.RadioManager;
/// @}

/// M: CC: [ALPS04973557][ALPS04982579] wait for service state ready
import android.os.SystemProperties;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.telephony.MtkTelephonyManagerEx;

/**
 * Helper class that listens to a Phone's radio state and sends an onComplete callback when we
 * return true for isOkToCall.
 */
public class RadioOnStateListener {

    interface Callback {
        /**
         * Receives the result of the RadioOnStateListener's attempt to turn on the radio.
         */
        void onComplete(RadioOnStateListener listener, boolean isRadioReady);

        /**
         * Given the Phone and the new service state of that phone, return whether or not this
         * phone is ok to call. If it is, onComplete will be called shortly after.
         */
        boolean isOkToCall(Phone phone, int serviceState);
    }

    // Number of times to retry the call, and time between retry attempts.
    // not final for testing
    private static int MAX_NUM_RETRIES = 5;
    // not final for testing
    private static long TIME_BETWEEN_RETRIES_MILLIS = 5000;  // msec

    // Handler message codes; see handleMessage()
    private static final int MSG_START_SEQUENCE = 1;
    @VisibleForTesting
    public static final int MSG_SERVICE_STATE_CHANGED = 2;
    private static final int MSG_RETRY_TIMEOUT = 3;

    // M: CC: [ALPS04079763] Wait for radio off first before turning on radio.
    public static final int MSG_RADIO_OFF_OR_NOT_AVAILABLE = 4;

    private final Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_START_SEQUENCE:
                    SomeArgs args = (SomeArgs) msg.obj;
                    try {
                        Phone phone = (Phone) args.arg1;
                        RadioOnStateListener.Callback callback =
                                (RadioOnStateListener.Callback) args.arg2;
                        startSequenceInternal(phone, callback);
                    } finally {
                        args.recycle();
                    }
                    break;
                case MSG_SERVICE_STATE_CHANGED:
                    onServiceStateChanged((ServiceState) ((AsyncResult) msg.obj).result);
                    break;
                case MSG_RETRY_TIMEOUT:
                    onRetryTimeout();
                    break;
                /// M: CC: [ALPS04079763] Wait for radio off first before turning on radio. @{
                case MSG_RADIO_OFF_OR_NOT_AVAILABLE:
                    if (mWaitForRadioOffFirst) {
                        Log.d(this, "MSG_RADIO_OFF_OR_NOT_AVAILABLE");
                        mWaitForRadioOffFirst = false;
                        mNumRetriesSoFar = 0;
                    }
                    break;
                /// @}
                default:
                    Log.wtf(this, "handleMessage: unexpected message: %d.", msg.what);
                    break;
            }
        }
    };


    private Callback mCallback;  // The callback to notify upon completion.
    private Phone mPhone;  // The phone that will attempt to place the call.
    private int mNumRetriesSoFar;

    /**
     * Starts the "wait for radio" sequence. This is the (single) external API of the
     * RadioOnStateListener class.
     *
     * This method kicks off the following sequence:
     * - Listen for the service state change event telling us the radio has come up.
     * - Retry if we've gone {@link #TIME_BETWEEN_RETRIES_MILLIS} without any response from the
     *   radio.
     * - Finally, clean up any leftover state.
     *
     * This method is safe to call from any thread, since it simply posts a message to the
     * RadioOnStateListener's handler (thus ensuring that the rest of the sequence is entirely
     * serialized, and runs only on the handler thread.)
     */
    public void waitForRadioOn(Phone phone, Callback callback) {
        Log.d(this, "waitForRadioOn: Phone " + phone.getPhoneId());

        if (mPhone != null) {
            // If there already is an ongoing request, ignore the new one!
            return;
        }

        /// M: CC: [ALPS04079763] Wait for radio off first before turning on radio.
        mPhone = phone;

        SomeArgs args = SomeArgs.obtain();
        args.arg1 = phone;
        args.arg2 = callback;
        mHandler.obtainMessage(MSG_START_SEQUENCE, args).sendToTarget();

        /// M: CC: [ALPS04079763] Wait for radio off first before turning on radio. @{
        if (mWaitForRadioOffFirst) {
            mPhone.registerForRadioOffOrNotAvailable(mHandler,
                    MSG_RADIO_OFF_OR_NOT_AVAILABLE, null);
        }
        /// @}
    }

    /**
     * Actual implementation of waitForRadioOn(), guaranteed to run on the handler thread.
     *
     * @see #waitForRadioOn
     */
    private void startSequenceInternal(Phone phone, Callback callback) {
        Log.d(this, "startSequenceInternal: Phone " + phone.getPhoneId());

        // First of all, clean up any state left over from a prior RadioOn call sequence. This
        // ensures that we'll behave sanely if another startTurnOnRadioSequence() comes in while
        // we're already in the middle of the sequence.
        cleanup();

        mPhone = phone;
        mCallback = callback;

        registerForServiceStateChanged();
        // Next step: when the SERVICE_STATE_CHANGED event comes in, we'll retry the call; see
        // onServiceStateChanged(). But also, just in case, start a timer to make sure we'll retry
        // the call even if the SERVICE_STATE_CHANGED event never comes in for some reason.
        startRetryTimer();
    }

    /**
     * Handles the SERVICE_STATE_CHANGED event. This event tells us that the radio state has changed
     * and is probably coming up. We can now check to see if the conditions are met to place the
     * call with {@link Callback#isOkToCall}
     */
    private void onServiceStateChanged(ServiceState state) {
        Log.d(this, "onServiceStateChanged(), new state = %s, Phone = %s", state,
                mPhone.getPhoneId());

        /// M: CC: [ALPS04079763] Wait for radio off first before turning on radio. @{
        if (mWaitForRadioOffFirst) {
            Log.i(this, "onServiceStateChanged: need to wait for radio off first.");
            return;
        }
        /// @}

        // Possible service states:
        // - STATE_IN_SERVICE        // Normal operation
        // - STATE_OUT_OF_SERVICE    // Still searching for an operator to register to,
        //                           // or no radio signal
        // - STATE_EMERGENCY_ONLY    // Only emergency numbers are allowed; currently not used
        // - STATE_POWER_OFF         // Radio is explicitly powered off (airplane mode)

        if (isOkToCall(state.getState())) {
            // Woo hoo!  It's OK to actually place the call.
            Log.d(this, "onServiceStateChanged: ok to call!");

            onComplete(true);
            cleanup();
        } else {
            // The service state changed, but we're still not ready to call yet.
            Log.d(this, "onServiceStateChanged: not ready to call yet, keep waiting.");
        }
    }

    /**
     * Callback to see if it is okay to call yet, given the current conditions.
     */
    private boolean isOkToCall(int serviceState) {
        /// M: CC: [ALPS04973557][ALPS04982579] wait for service state ready @{
        Log.i(this, "isOkToCall():  isEmciSupported = %b", isEmciSupported());
        if (!SystemProperties.get("persist.vendor.operator.optr", "OM").equals("OP12") &&
                !isEmciSupported()) {
            int mainPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
            // M: CC: [ALPS051317574]
            // Radio is still off after exiting airplane mode if SIM is off (Gen93 design)
            // Need to force turn on radio by RadioOnHelper after 1st timeout (5 second)
            if (serviceState == ServiceState.STATE_POWER_OFF || !mPhone.isRadioOn()) {
                return false;
            }

            return mPhone.getState() == PhoneConstants.State.OFFHOOK
                    || serviceState == ServiceState.STATE_IN_SERVICE
                    || serviceState == ServiceState.STATE_EMERGENCY_ONLY
                    || mPhone.getServiceState().isEmergencyOnly()
                    || (mNumRetriesSoFar == MAX_NUM_RETRIES &&
                        serviceState == ServiceState.STATE_OUT_OF_SERVICE)
                    /// M: CC: [ALPS03837972] Performance enhancement. With two CDMA cards,
                    // the second card may not be in service or emergency only in time.
                    // Don't waste time for waiting if we already have a phone in service.
                    || (isAllCdmaCard() &&
                        serviceState == ServiceState.STATE_OUT_OF_SERVICE &&
                         mPhone.getPhoneId() != mainPhoneId &&
                         hasInServicePhone())
                    /// M: CC: [ALPS02185470] Only retry once for WFC ECC.
                    || MtkTelephonyManagerEx.getDefault().isWifiCallingEnabled(mPhone.getSubId());
        }
        /// @}

        return (mCallback == null) ? false : mCallback.isOkToCall(mPhone, serviceState);
    }

     /// M: CC: [ALPS04973557][ALPS04982579] wait for service state ready @{
     /**
      * Check if modem supports EMCI
      * @return true for support and false for non-support.
      */
     private boolean isEmciSupported() {
         if (SystemProperties.get("vendor.ril.call.emci_support").equals("1")) {
             return true;
         } else {
             return false;
         }
     }

     private boolean isAllCdmaCard() {
         MtkTelephonyManagerEx tmEx = MtkTelephonyManagerEx.getDefault();
         for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
             int appFamily = tmEx.getIccAppFamily(i);
             if (appFamily == MtkTelephonyManagerEx.APP_FAM_NONE
                     || appFamily == MtkTelephonyManagerEx.APP_FAM_3GPP) {
                 return false;
             }
         }
         return true;
     }

     private boolean hasInServicePhone() {
         for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
             if (ServiceState.STATE_IN_SERVICE
                     == PhoneFactory.getPhone(i).getServiceState().getState()) {
                 return true;
             }
         }
         return false;
     }
     /// @}

    /**
     * Handles the retry timer expiring.
     */
    private void onRetryTimeout() {
        int serviceState = mPhone.getServiceState().getState();
        Log.d(this, "onRetryTimeout():  phone state = %s, service state = %d, retries = %d.",
                mPhone.getState(), serviceState, mNumRetriesSoFar);

        /// M: CC: [ALPS04079763] Wait for radio off first before turning on radio. @{
        if (mWaitForRadioOffFirst) {
            Log.i(this, "onServiceStateChanged: need to wait for radio off first.");
            mNumRetriesSoFar++;
            if (mNumRetriesSoFar > MAX_NUM_RETRIES) {
                Log.w(this, "Hit MAX_NUM_RETRIES in waiting for radio off first.");
                mWaitForRadioOffFirst = false;
                mNumRetriesSoFar = 0;
            } else {
                startRetryTimer();
                return;
            }
        }
        /// @}

        // - If we're actually in a call, we've succeeded.
        // - Otherwise, if the radio is now on, that means we successfully got out of airplane mode
        //   but somehow didn't get the service state change event.  In that case, try to place the
        //   call.
        // - If the radio is still powered off, try powering it on again.

        if (isOkToCall(serviceState)) {
            Log.d(this, "onRetryTimeout: Radio is on. Cleaning up.");

            // Woo hoo -- we successfully got out of airplane mode.
            onComplete(true);
            cleanup();
        } else {
            // Uh oh; we've waited the full TIME_BETWEEN_RETRIES_MILLIS and the radio is still not
            // powered-on.  Try again.

            mNumRetriesSoFar++;
            Log.d(this, "mNumRetriesSoFar is now " + mNumRetriesSoFar);

            if (mNumRetriesSoFar > MAX_NUM_RETRIES) {
                Log.w(this, "Hit MAX_NUM_RETRIES; giving up.");
                cleanup();
            } else {
                Log.d(this, "Trying (again) to turn on the radio.");
                /// M: CC: ECC phone selection rule @{
                //mPhone.setRadioPower(true);
                // Use RadioManager for powering on radio during ECC [ALPS01785370]
                if (RadioManager.isMSimModeSupport()) {
                    //RadioManager will help to turn on radio even this iccid is off by sim mgr
                    Log.d(this, "isMSimModeSupport true, use RadioManager forceSetRadioPower");
                    RadioManager.getInstance().forceSetRadioPower(true, mPhone.getPhoneId());
                } else {
                    //android's default action
                    Log.d(this, "isMSimModeSupport false, use default setRadioPower");
                    mPhone.setRadioPower(true);
                }
                /// @}
                startRetryTimer();
            }
        }
    }

    /**
     * Clean up when done with the whole sequence: either after successfully turning on the radio,
     * or after bailing out because of too many failures.
     *
     * The exact cleanup steps are:
     * - Notify callback if we still hadn't sent it a response.
     * - Double-check that we're not still registered for any telephony events
     * - Clean up any extraneous handler messages (like retry timeouts) still in the queue
     *
     * Basically this method guarantees that there will be no more activity from the
     * RadioOnStateListener until someone kicks off the whole sequence again with another call
     * to {@link #waitForRadioOn}
     *
     * TODO: Do the work for the comment below:
     * Note we don't call this method simply after a successful call to placeCall(), since it's
     * still possible the call will disconnect very quickly with an OUT_OF_SERVICE error.
     */
    // M: CC: change to public method to invoke from EmergencyCallHelper.
    public void cleanup() {
        Log.d(this, "cleanup()");

        // This will send a failure call back if callback has yet to be invoked.  If the callback
        // was already invoked, it's a no-op.
        onComplete(false);

        unregisterForServiceStateChanged();
        cancelRetryTimer();

        // M: CC: Clean up any pending MSG_START_SEQUENCE message.
        mHandler.removeMessages(MSG_START_SEQUENCE);

        // Used for unregisterForServiceStateChanged() so we null it out here instead.
        mPhone = null;
        mNumRetriesSoFar = 0;
    }

    private void startRetryTimer() {
        cancelRetryTimer();
        mHandler.sendEmptyMessageDelayed(MSG_RETRY_TIMEOUT, TIME_BETWEEN_RETRIES_MILLIS);
    }

    private void cancelRetryTimer() {
        mHandler.removeMessages(MSG_RETRY_TIMEOUT);
    }

    private void registerForServiceStateChanged() {
        // Unregister first, just to make sure we never register ourselves twice.  (We need this
        // because Phone.registerForServiceStateChanged() does not prevent multiple registration of
        // the same handler.)
        unregisterForServiceStateChanged();
        mPhone.registerForServiceStateChanged(mHandler, MSG_SERVICE_STATE_CHANGED, null);
    }

    private void unregisterForServiceStateChanged() {
        // This method is safe to call even if we haven't set mPhone yet.
        if (mPhone != null) {
            mPhone.unregisterForServiceStateChanged(mHandler);  // Safe even if unnecessary
        }
        mHandler.removeMessages(MSG_SERVICE_STATE_CHANGED);  // Clean up any pending messages too
    }

    private void onComplete(boolean isRadioReady) {
        if (mCallback != null) {
            Callback tempCallback = mCallback;
            mCallback = null;
            tempCallback.onComplete(this, isRadioReady);
        }
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mHandler;
    }

    @VisibleForTesting
    public void setMaxNumRetries(int retries) {
        MAX_NUM_RETRIES = retries;
    }

    @VisibleForTesting
    public void setTimeBetweenRetriesMillis(long timeMs) {
        TIME_BETWEEN_RETRIES_MILLIS = timeMs;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || !getClass().equals(o.getClass())) return false;

        RadioOnStateListener that = (RadioOnStateListener) o;

        if (mNumRetriesSoFar != that.mNumRetriesSoFar) {
            return false;
        }
        if (mCallback != null ? !mCallback.equals(that.mCallback) : that.mCallback != null) {
            return false;
        }
        return mPhone != null ? mPhone.equals(that.mPhone) : that.mPhone == null;

    }

    /// M: CC: [ALPS04079763] Wait for radio off first before turning on radio. @{
    private boolean mWaitForRadioOffFirst = false;
    void setWaitForRadioOffFirst(boolean waitForRadioOffFirst) {
        mWaitForRadioOffFirst = waitForRadioOffFirst;
    }
    /// @}
}
