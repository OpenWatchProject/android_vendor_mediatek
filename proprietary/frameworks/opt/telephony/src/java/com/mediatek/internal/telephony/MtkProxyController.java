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

package com.mediatek.internal.telephony;

import java.util.ArrayList;
import java.util.Random;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.telephony.RadioAccessFamily;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandException.Error;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneSwitcher;
import com.android.internal.telephony.ProxyController;
import com.android.internal.telephony.RadioCapability;
import com.android.internal.telecom.ITelecomService;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.uicc.UiccController;

import com.mediatek.internal.telephony.RadioManager;
import com.mediatek.internal.telephony.MtkPhoneSubInfoControllerEx;
import com.mediatek.internal.telephony.ModemSwitchHandler;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.worldphone.WorldPhoneUtil;
import com.mediatek.internal.telephony.MtkUiccSmsController;
import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
// PHB START
import com.mediatek.internal.telephony.phb.MtkUiccPhoneBookController;
// PHB END
import com.mediatek.internal.telephony.devreg.DeviceRegisterController;
// External SIM [Start]
import com.mediatek.telephony.internal.telephony.vsim.ExternalSimManager;
// External SIM [End]
import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.HashSet;

public class MtkProxyController extends ProxyController {
    private static final String PROPERTY_CAPABILITY_SWITCH = "persist.vendor.radio.simswitch";
    private static final String PROPERTY_CAPABILITY_SWITCH_STATE
            = "persist.vendor.radio.simswitchstate";

    // event 1-5 is defined in ProxyController
    private static final int EVENT_RADIO_AVAILABLE = 6;
    private static final int EVENT_ON_REQUEST      = 7;

    // marker for retry cause
    private static final int RC_RETRY_CAUSE_NONE                  = 0;
    private static final int RC_RETRY_CAUSE_WORLD_MODE_SWITCHING  = 1;
    private static final int RC_RETRY_CAUSE_CAPABILITY_SWITCHING  = 2;
    private static final int RC_RETRY_CAUSE_IN_CALL               = 3;
    private static final int RC_RETRY_CAUSE_RADIO_UNAVAILABLE     = 4;
    private static final int RC_RETRY_CAUSE_AIRPLANE_MODE         = 5;
    private static final int RC_RETRY_CAUSE_RESULT_ERROR          = 6;

    // marker for switch conditions pre-checking
    private static final int RC_DO_SWITCH       = 0;
    private static final int RC_NO_NEED_SWITCH  = 1;
    private static final int RC_CANNOT_SWITCH   = 2;

    // rild mode
    private static final int G5M_1RILD   = 0;
    private static final int C6M_3RILD   = 1;
    private static final int C6M_1RILD   = 2;

    //***** Class Variables
    private boolean mIsCapSwitching = false;
    private boolean mHasRegisterWorldModeReceiver = false;
    private boolean mHasRegisterCallStateReceiver = false;

    RadioAccessFamily[] mNextRafs = null;
    RadioAccessFamily[] mCurrRafs = null;
    private int mSetRafRetryCause;
    private int mRildMode;
    // Exception counter
    private int onExceptionCount = 0;
    private MtkPhoneSubInfoControllerEx mMtkPhoneSubInfoControllerEx;
    //MtkUiccSmsController to use proper MtkIccSmsInterfaceManager object
    private MtkUiccSmsController mMtkUiccSmsController;

    // PHB START
    //MtkUiccPhoneBookController to use proper MtkIccPhoneBookInterfaceManager object
    protected MtkUiccPhoneBookController mMtkUiccPhoneBookController;
    // PHB END

    private DeviceRegisterController mDeviceRegisterController;

    private OpTelephonyCustomizationFactoryBase mTelephonyCustomizationFactory = null;
    private IMtkProxyControllerExt mProxyControllerExt = null;

    //***** Class Methods
    public MtkProxyController(Context context, Phone[] phone, UiccController uiccController,
            CommandsInterface[] ci, PhoneSwitcher phoneSwitcher) {
        super(context, phone, uiccController, ci, phoneSwitcher);

        String rilMode = SystemProperties.get("ro.vendor.mtk_ril_mode", "c6m_1rild");

        if (rilMode.equals("c6m_1rild")) {
            mRildMode = C6M_1RILD;
        } else if (rilMode.equals("c6m_3rild")) {
            mRildMode = C6M_3RILD;
        } else {
            mRildMode = G5M_1RILD;
        }

        mtkLogd("Constructor - Enter, rild mode = " + mRildMode);

        // PHB START
        mMtkUiccPhoneBookController = new MtkUiccPhoneBookController(mPhones);
        // PHB END
        mMtkPhoneSubInfoControllerEx = new MtkPhoneSubInfoControllerEx(mContext, mPhones);
        mMtkUiccSmsController = new MtkUiccSmsController(mPhones);
        mtkLogd("Constructor - Exit");

        // init Device Register plugin instance
        mDeviceRegisterController = new DeviceRegisterController(
                mContext, mPhones, mMtkUiccSmsController);

        if (mRildMode != C6M_1RILD) {
            // init operater plug-in
            try {
                mTelephonyCustomizationFactory =
                        OpTelephonyCustomizationUtils.getOpFactory(context);
                mProxyControllerExt =
                        mTelephonyCustomizationFactory.makeMtkProxyControllerExt(context);
            } catch (Exception e) {
                mtkLogd("mProxyControllerExt init fail");
                e.printStackTrace();
            }
        }
    }

    public DeviceRegisterController getDeviceRegisterController() {
        return mDeviceRegisterController;
    }

    /**
     * Set phone radio type and access technology for each phone.
     *
     * @param rafs an RadioAccessFamily array to indicate all phone's
     *        new radio access family. The length of RadioAccessFamily
     *        must equal to phone count.
     * @return false if another session is already active and the request is rejected.
     */
    @Override
    public boolean setRadioCapability(RadioAccessFamily[] rafs) {
        // check if capability switch disabled
        if (SystemProperties.getBoolean("ro.vendor.mtk_disable_cap_switch", false) == true) {
            completeRadioCapabilityTransaction();
            mtkLogd("skip switching because mtk_disable_cap_switch is true");
        } else {
            Message tmsg = mMtkHandler.obtainMessage(EVENT_ON_REQUEST);
            mCurrRafs = rafs;
            mMtkHandler.sendMessage(tmsg);
        }
        return true;
    }

    public boolean onSetRadioCapabilityRequest(RadioAccessFamily[] rafs) {
        if (mRildMode == C6M_1RILD) {
            return super.setRadioCapability(rafs);
        }

        if (rafs.length != mPhones.length) {
            throw new RuntimeException("Length of input rafs must equal to total phone count");
        }

        // set request info to property for retry.
        for (int i = 0; i < rafs.length; i++) {
            if ((rafs[i].getRadioAccessFamily() & RadioAccessFamily.RAF_GPRS) > 0) {
                SystemProperties.set(PROPERTY_CAPABILITY_SWITCH_STATE, String.valueOf(i));
                break;
            }
        }

        // pre-checking switch conditions
        int result = checkRadioCapabilitySwitchConditions(rafs);
        if (result == RC_DO_SWITCH) {
            return super.setRadioCapability(rafs);
        } else {
            return true;
        }
    }

    @Override
    protected boolean doSetRadioCapabilities(RadioAccessFamily[] rafs) {
        if (mRildMode == C6M_1RILD) {
            return super.doSetRadioCapabilities(rafs);
        }
        synchronized (this) {
            mIsCapSwitching = true;
        }
        onExceptionCount = 0;
        return super.doSetRadioCapabilities(rafs);
    }

    private Handler mMtkHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            mtkLogd("mtkHandleMessage msg.what=" + msg.what);

            switch (msg.what) {
                case EVENT_RADIO_AVAILABLE:
                    if (mRildMode == C6M_1RILD) {
                        return;
                    }
                    onRetryWhenRadioAvailable(msg);
                    break;

                case EVENT_ON_REQUEST:
                    onSetRadioCapabilityRequest(mCurrRafs);
                    break;

                default:
                    break;
            }
        }
    };

    /**
     * Handle START response
     * @param msg obj field isa RadioCapability
     */
    @Override
    protected void onStartRadioCapabilityResponse(Message msg) {
        synchronized (mSetRadioAccessFamilyStatus) {
            AsyncResult ar = (AsyncResult)msg.obj;
            if (ar.exception != null) {
                if ((onExceptionCount == 0) && (mRildMode != C6M_1RILD)) {
                    CommandException.Error err = null;
                    // counter is to avoid multiple error handle.
                    onExceptionCount = 1;
                    if (ar.exception instanceof CommandException) {
                        err = ((CommandException) (ar.exception)).getCommandError();
                    }

                    if (err == CommandException.Error.RADIO_NOT_AVAILABLE) {
                        // Radio has crashed or turned off
                        mSetRafRetryCause = RC_RETRY_CAUSE_RADIO_UNAVAILABLE;
                        // check radio available
                        for (int i = 0; i < mPhones.length; i++) {
                            mCi[i].registerForAvailable(mMtkHandler, EVENT_RADIO_AVAILABLE, null);
                        }
                        mtkLoge("onStartRadioCapabilityResponse: Retry later due to modem off");
                    }
                }

                // just abort now.  They didn't take our start so we don't have to revert
                mtkLogd("onStartRadioCapabilityResponse got exception=" + ar.exception);
                mRadioCapabilitySessionId = mUniqueIdGenerator.getAndIncrement();
                sendCapabilityFailBroadcast();
                resetSimSwitchState();
                return;
            }
            RadioCapability rc = (RadioCapability) ((AsyncResult) msg.obj).result;
            if ((rc == null) || (rc.getSession() != mRadioCapabilitySessionId)) {
                mtkLogd("onStartRadioCapabilityResponse: Ignore session="
                        + mRadioCapabilitySessionId + " rc=" + rc);
                return;
            }
            mRadioAccessFamilyStatusCounter--;
            int id = rc.getPhoneId();
            if (((AsyncResult) msg.obj).exception != null) {
                mtkLogd("onStartRadioCapabilityResponse: Error response session="
                        + rc.getSession());
                mtkLogd("onStartRadioCapabilityResponse: phoneId=" + id + " status=FAIL");
                mSetRadioAccessFamilyStatus[id] = SET_RC_STATUS_FAIL;
                mTransactionFailed = true;
            } else {
                mtkLogd("onStartRadioCapabilityResponse: phoneId=" + id + " status=STARTED");
                mSetRadioAccessFamilyStatus[id] = SET_RC_STATUS_STARTED;
            }

            if (mRadioAccessFamilyStatusCounter == 0) {
                /* remove Google's code because it causes capability switch fail in 3SIM project.
                 * mNewLogicalModemIds get same modem id in two 2G logical modem then cause WTF.
                 */
                /*
                HashSet<String> modemsInUse = new HashSet<String>(mNewLogicalModemIds.length);
                for (String modemId : mNewLogicalModemIds) {
                    if (!modemsInUse.add(modemId)) {
                        mTransactionFailed = true;
                        Log.wtf(LOG_TAG, "ERROR: sending down the same id for different phones");
                    }
                }
                */
                mtkLogd("onStartRadioCapabilityResponse: success=" + !mTransactionFailed);
                if (mTransactionFailed) {
                    // Sends a variable number of requests, so don't resetRadioAccessFamilyCounter
                    // here.
                    issueFinish(mRadioCapabilitySessionId);
                } else {
                    // All logical modem accepted the new radio access family, issue the APPLY
                    resetRadioAccessFamilyStatusCounter();
                    for (int i = 0; i < mPhones.length; i++) {
                        sendRadioCapabilityRequest(
                            i,
                            mRadioCapabilitySessionId,
                            RadioCapability.RC_PHASE_APPLY,
                            mNewRadioAccessFamily[i],
                            mNewLogicalModemIds[i],
                            RadioCapability.RC_STATUS_NONE,
                            EVENT_APPLY_RC_RESPONSE);

                        mtkLogd("onStartRadioCapabilityResponse: phoneId="
                                + i + " status=APPLYING");
                        mSetRadioAccessFamilyStatus[i] = SET_RC_STATUS_APPLYING;
                    }
                }
            }
        }
    }


    @Override
    protected void onApplyRadioCapabilityErrorHandler(Message msg) {
        if (mRildMode == C6M_1RILD) {
            mRadioCapabilitySessionId = mUniqueIdGenerator.getAndIncrement();
            sendCapabilityFailBroadcast();
            resetSimSwitchState();
            return;
        }

        RadioCapability rc = (RadioCapability) ((AsyncResult) msg.obj).result;
        AsyncResult ar = (AsyncResult) msg.obj;
        CommandException.Error err = null;

        if ((rc == null) && (ar.exception != null) && (onExceptionCount == 0)) {
            // counter is to avoid multiple error handle.
            onExceptionCount = 1;
            if (ar.exception instanceof CommandException) {
                err = ((CommandException) (ar.exception)).getCommandError();
            }

            if (err == CommandException.Error.RADIO_NOT_AVAILABLE) {
                // Radio has crashed or turned off
                mSetRafRetryCause = RC_RETRY_CAUSE_RADIO_UNAVAILABLE;
                // check radio available
                for (int i = 0; i < mPhones.length; i++) {
                    mCi[i].registerForAvailable(mMtkHandler, EVENT_RADIO_AVAILABLE, null);
                }
                mtkLoge("onApplyRadioCapabilityResponse: Retry due to RADIO_NOT_AVAILABLE");
            } else {
                mtkLoge("onApplyRadioCapabilityResponse: exception=" +
                        ar.exception);
            }
            mRadioCapabilitySessionId = mUniqueIdGenerator.getAndIncrement();
            sendCapabilityFailBroadcast();
            resetSimSwitchState();
        }

    }

    @Override
    protected void onApplyExceptionHandler(Message msg) {
        if (mRildMode == C6M_1RILD) {
            super.onApplyExceptionHandler(msg);
            return;
        }
        RadioCapability rc = (RadioCapability) ((AsyncResult) msg.obj).result;
        AsyncResult ar = (AsyncResult) msg.obj;
        int id = rc.getPhoneId();
        CommandException.Error err = null;

        if (ar.exception instanceof CommandException) {
            err = ((CommandException) (ar.exception)).getCommandError();
        }

        if (err == CommandException.Error.RADIO_NOT_AVAILABLE) {
            // Radio has crashed or turned off
            mSetRafRetryCause = RC_RETRY_CAUSE_RADIO_UNAVAILABLE;
            // check radio available
            mCi[id].registerForAvailable(mMtkHandler, EVENT_RADIO_AVAILABLE, null);
            mtkLoge("onApplyRadioCapabilityResponse: Retry later due to modem off");
        } else {
            mtkLoge("onApplyRadioCapabilityResponse: exception=" +
                    ar.exception);
        }

    }

    /**
     * Handle the notification unsolicited response associated with the APPLY
     * @param msg obj field isa RadioCapability
     */
    @Override
    protected void onNotificationRadioCapabilityChanged(Message msg) {
        // update radio capability by other module, such as mode switch, world phone
        RadioCapability rc = (RadioCapability) ((AsyncResult) msg.obj).result;
        if (rc == null) {
            logd("onNotificationRadioCapabilityChanged: rc == null");
            return;
        }
        logd("onNotificationRadioCapabilityChanged: rc=" + rc);

        int id = rc.getPhoneId();
        if (((AsyncResult) msg.obj).exception == null) {
            logd("onNotificationRadioCapabilityChanged: update phone capability");
            mPhones[id].radioCapabilityUpdated(rc);
        }

        super.onNotificationRadioCapabilityChanged(msg);
    }

    /**
     * Handle the FINISH Phase response
     * @param msg obj field isa RadioCapability
     */
    @Override
   protected void onFinishRadioCapabilityResponse(Message msg) {
        RadioCapability rc = (RadioCapability) ((AsyncResult) msg.obj).result;
        if ((rc == null) || (rc.getSession() != mRadioCapabilitySessionId)) {
            //When capability switch on Finish phase,socket may disconnected by other module ,
            //like airplan mode ,in this case rc is null,it will return and can not
            //finish at all.
            if ((rc == null) && (((AsyncResult) msg.obj).exception != null)) {
                synchronized (mSetRadioAccessFamilyStatus) {
                    mtkLogd("onFinishRadioCapabilityResponse C2K mRadioAccessFamilyStatusCounter="
                            + mRadioAccessFamilyStatusCounter);
                    mRadioAccessFamilyStatusCounter--;
                    if (mRadioAccessFamilyStatusCounter == 0) {
                        completeRadioCapabilityTransaction();
                    }
                }
                return;
            }
        }

        if (mRildMode == C6M_1RILD) {
            super.onFinishRadioCapabilityResponse(msg);
            return;
        }

        int phoneId = SystemProperties.getInt(PROPERTY_CAPABILITY_SWITCH_STATE, -1);
        if (phoneId >= 0 && phoneId < mPhones.length && mRadioAccessFamilyStatusCounter == 1) {
            int raf = mPhones[phoneId].getRadioAccessFamily();
            if ((raf & RadioAccessFamily.RAF_GPRS) == 0) {
                mtkLogd("onFinishRadioCapabilityResponse, main phone raf[" + phoneId + "]=" + raf);
                mSetRafRetryCause = RC_RETRY_CAUSE_RESULT_ERROR;
            }
        }
        super.onFinishRadioCapabilityResponse(msg);
    }

    @Override
    protected void onTimeoutRadioCapability(Message msg) {
        // M: don't time out, wait the URC of radio capability update
        Message tmsg = mHandler.obtainMessage(EVENT_TIMEOUT, mRadioCapabilitySessionId, 0);
        mHandler.sendMessageDelayed(tmsg, SET_RC_TIMEOUT_WAITING_MSEC);
    }

    @Override
    protected void issueFinish(int sessionId) {
        if (mRildMode == C6M_1RILD) {
            super.issueFinish(sessionId);
            return;
        }

        // Issue FINISH
        synchronized(mSetRadioAccessFamilyStatus) {
            // Reset counter directly instead of AOSP accumulate, to fix apply stage fail case
            resetRadioAccessFamilyStatusCounter();

            for (int i = 0; i < mPhones.length; i++) {
                mtkLogd("issueFinish: phoneId=" + i + " sessionId=" + sessionId
                        + " mTransactionFailed=" + mTransactionFailed);

                sendRadioCapabilityRequest(
                        i,
                        sessionId,
                        RadioCapability.RC_PHASE_FINISH,
                        mOldRadioAccessFamily[i],
                        mCurrentLogicalModemIds[i],
                        (mTransactionFailed ? RadioCapability.RC_STATUS_FAIL :
                        RadioCapability.RC_STATUS_SUCCESS),
                        EVENT_FINISH_RC_RESPONSE);
                if (mTransactionFailed) {
                    mtkLogd("issueFinish: phoneId: " + i + " status: FAIL");
                    // At least one failed, mark them all failed.
                    mSetRadioAccessFamilyStatus[i] = SET_RC_STATUS_FAIL;
                }
            }
        }
    }

    @Override
    protected void completeRadioCapabilityTransaction() {
        // Create the intent to broadcast
        Intent intent;
        mtkLogd("onFinishRadioCapabilityResponse: success=" + !mTransactionFailed);

        // clear sim switching flag
        SystemProperties.set(PROPERTY_CAPABILITY_SWITCH_STATE, "-1");
        if (!mTransactionFailed) {
            ArrayList<RadioAccessFamily> phoneRAFList = new ArrayList<RadioAccessFamily>();
            for (int i = 0; i < mPhones.length; i++) {
                int raf = mPhones[i].getRadioAccessFamily();
                mtkLogd("radioAccessFamily[" + i + "]=" + raf);
                RadioAccessFamily phoneRC = new RadioAccessFamily(i, raf);
                phoneRAFList.add(phoneRC);
            }
            intent = new Intent(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
            intent.putParcelableArrayListExtra(TelephonyIntents.EXTRA_RADIO_ACCESS_FAMILY,
                    phoneRAFList);

            // make messages about the old transaction obsolete (specifically the timeout)
            mRadioCapabilitySessionId = mUniqueIdGenerator.getAndIncrement();

            // Reinitialize
            resetSimSwitchState();
        } else {
            intent = new Intent(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);

            // now revert.
            mTransactionFailed = false;

            // ASOP revert is not acceptable by user, so clear transaction and retry later.
            resetSimSwitchState();
        }

        // Broadcast that we're done
        mContext.sendBroadcastAsUser(
                intent, UserHandle.ALL, android.Manifest.permission.READ_PHONE_STATE);

        if (mRildMode != C6M_1RILD) {
            if ((mNextRafs != null)
                    && ((mSetRafRetryCause == RC_RETRY_CAUSE_CAPABILITY_SWITCHING)
                    || (mSetRafRetryCause == RC_RETRY_CAUSE_RESULT_ERROR))) {
                mtkLogd("has next request, trigger it, cause = " + mSetRafRetryCause);
                try {
                    if (!setRadioCapability(mNextRafs)) {
                        sendCapabilityFailBroadcast();
                    } else {
                        mSetRafRetryCause = RC_RETRY_CAUSE_NONE;
                        mNextRafs = null;
                    }
                } catch (java.lang.RuntimeException e) {
                    sendCapabilityFailBroadcast();
                }
            }
        }
    }

    // Clear this transaction
    private void resetSimSwitchState() {
        if (isCapabilitySwitching()) {
            mHandler.removeMessages(EVENT_TIMEOUT);
        }
        if (mRildMode == C6M_1RILD) {
            clearTransaction();
            return;
        }

        synchronized (this) {
            mIsCapSwitching = false;
        }
        clearTransaction();
    }

    @Override
    protected void sendRadioCapabilityRequest(int phoneId, int sessionId, int rcPhase,
            int radioFamily, String logicalModemId, int status, int eventId) {
        if (mRildMode == C6M_1RILD) {
            super.sendRadioCapabilityRequest(phoneId, sessionId, rcPhase, radioFamily,
                    logicalModemId, status, eventId);
            return;
        }

        if (logicalModemId == null || logicalModemId.equals("")) {
            logicalModemId = "modem_sys3";
        }

        super.sendRadioCapabilityRequest(phoneId, sessionId, rcPhase, radioFamily,
            logicalModemId, status, eventId);
    }

    // This method will return max number of raf bits supported from the raf
    // values currently stored in all phone objects
    @Override
    public int getMaxRafSupported() {
        int[] numRafSupported = new int[mPhones.length];
        int maxNumRafBit = 0;
        int maxRaf = RadioAccessFamily.RAF_UNKNOWN;

        if (mRildMode == C6M_1RILD) {
            return super.getMaxRafSupported();
        }

        // RAF_GPRS is a marker of main capability
        for (int len = 0; len < mPhones.length; len++) {
            if ((mPhones[len].getRadioAccessFamily() & RadioAccessFamily.RAF_GPRS)
                    == RadioAccessFamily.RAF_GPRS) {
                maxRaf = mPhones[len].getRadioAccessFamily();
            }
        }
        mtkLogd("getMaxRafSupported: maxRafBit=" + maxNumRafBit + " maxRaf=" + maxRaf
            + " flag=" + (maxRaf & RadioAccessFamily.RAF_GPRS));

        // If the phone capability cannot be updated promptly, the max capability should mark with
        // GPRS, to avoid using an unknown RAF to trigger sim switch
        if (maxRaf == RadioAccessFamily.RAF_UNKNOWN) {
            maxRaf |= RadioAccessFamily.RAF_GPRS;
        }

        return maxRaf;
    }

    // This method will return minimum number of raf bits supported from the raf
    // values currently stored in all phone objects
    @Override
    public int getMinRafSupported() {
        int[] numRafSupported = new int[mPhones.length];
        int minNumRafBit = 0;
        int minRaf = RadioAccessFamily.RAF_UNKNOWN;

        if (mRildMode == C6M_1RILD) {
            return super.getMinRafSupported();
        }

        // RAF_GPRS is a marker of main capability
        for (int len = 0; len < mPhones.length; len++) {
            if ((mPhones[len].getRadioAccessFamily() & RadioAccessFamily.RAF_GPRS) == 0) {
                minRaf = mPhones[len].getRadioAccessFamily();
            }
        }
        mtkLogd("getMinRafSupported: minRafBit=" + minNumRafBit + " minRaf=" + minRaf
            + " flag=" + (minRaf & RadioAccessFamily.RAF_GPRS));

        return minRaf;
    }

    protected void mtkLogd(String string) {
        Rlog.d("MtkProxyController", string);
    }

    protected void mtkLoge(String string) {
        Rlog.e("MtkProxyController", string);
    }

    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        try {
            mPhoneSwitcher.dump(fd, pw, args);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Check if under capability switching.
     *
     * @return true if switching
     */
    public boolean isCapabilitySwitching() {
        if (mRildMode == C6M_1RILD) {
            synchronized (mSetRadioAccessFamilyStatus) {
                for (int i = 0; i < mPhones.length; i++) {
                    if (mSetRadioAccessFamilyStatus[i] == SET_RC_STATUS_STARTED
                            || mSetRadioAccessFamilyStatus[i] == SET_RC_STATUS_APPLYING
                            || mSetRadioAccessFamilyStatus[i] == SET_RC_STATUS_SUCCESS) {
                        mtkLogd("isCapabilitySwitching: Phone[" + i + "] status is "
                                + mSetRadioAccessFamilyStatus[i]);
                        return true;
                    }
                }
            }
            return false;
        }
        synchronized (this) {
            return mIsCapSwitching;
        }
    }

    // MTK private functions start
    // These functions should not be used if mRildMode is C6M_1RILD
    private int checkRadioCapabilitySwitchConditions(RadioAccessFamily[] rafs) {
        synchronized (this) {
            mNextRafs = rafs;

            // check if still switching
            if (mIsCapSwitching == true) {
                //throw new RuntimeException("is still switching");
                mtkLogd("keep it and return,because capability swithing");
                mSetRafRetryCause = RC_RETRY_CAUSE_CAPABILITY_SWITCHING;
                return RC_NO_NEED_SWITCH;
            } else if (mSetRafRetryCause == RC_RETRY_CAUSE_CAPABILITY_SWITCHING) {
                mtkLogd("setCapability, mIsCapSwitching is not switching, can switch");
                mSetRafRetryCause = RC_RETRY_CAUSE_NONE;
            }
            mIsCapSwitching = true;
        }

        // check if capability switch disabled
        if (SystemProperties.getBoolean("ro.vendor.mtk_disable_cap_switch", false) == true) {
            mNextRafs = null;
            completeRadioCapabilityTransaction();
            mtkLogd("skip switching because mtk_disable_cap_switch is true");
            return RC_NO_NEED_SWITCH;
        }
        // check FTA mode
        if (SystemProperties.getInt("vendor.gsm.gcf.testmode", 0) == 2) {
            mNextRafs = null;
            completeRadioCapabilityTransaction();
            mtkLogd("skip switching because FTA mode");
            return RC_NO_NEED_SWITCH;
        }
        // check EM disable mode
        if (SystemProperties.getInt("persist.vendor.radio.simswitch.emmode", 1) == 0) {
            mNextRafs = null;
            completeRadioCapabilityTransaction();
            mtkLogd("skip switching because EM disable mode");
            return RC_NO_NEED_SWITCH;
        }

        // check world mode switching
        if (WorldPhoneUtil.isWorldPhoneSupport()) {
            if (!WorldPhoneUtil.isWorldModeSupport()) {
                if (ModemSwitchHandler.isModemTypeSwitching()) {
                    logd("world mode switching.");
                    if (!mHasRegisterWorldModeReceiver) {
                        registerWorldModeReceiverFor90Modem();
                    }
                    mSetRafRetryCause = RC_RETRY_CAUSE_WORLD_MODE_SWITCHING;
                    synchronized (this) {
                        mIsCapSwitching = false;
                    }
                    return RC_CANNOT_SWITCH;
                }
            } else if (mSetRafRetryCause == RC_RETRY_CAUSE_WORLD_MODE_SWITCHING) {
                if (mHasRegisterWorldModeReceiver) {
                    unRegisterWorldModeReceiver();
                    mSetRafRetryCause = RC_RETRY_CAUSE_NONE;
                }
            }
        }

        // check call state
        if (TelephonyManager.getDefault().getCallState() != TelephonyManager.CALL_STATE_IDLE ||
                isEccInProgress()) {
            //throw new RuntimeException("in call, fail to set RAT for phones");
            mtkLogd("setCapability in calling, fail to set RAT for phones");
            if (!mHasRegisterCallStateReceiver) {
                registerCallStateReceiver();
            }
            mSetRafRetryCause = RC_RETRY_CAUSE_IN_CALL;
            synchronized (this) {
                mIsCapSwitching = false;
            }
            return RC_CANNOT_SWITCH;
        } else if (mSetRafRetryCause == RC_RETRY_CAUSE_IN_CALL) {
            if (mHasRegisterCallStateReceiver) {
                unRegisterCallStateReceiver();
                mSetRafRetryCause = RC_RETRY_CAUSE_NONE;
            }
        }

        // check radio available
        for (int i = 0; i < mPhones.length; i++) {
            if (!mPhones[i].isRadioAvailable()) {
                //throw new RuntimeException("Phone" + i + " is not available");
                mSetRafRetryCause = RC_RETRY_CAUSE_RADIO_UNAVAILABLE;
                mCi[i].registerForAvailable(mMtkHandler, EVENT_RADIO_AVAILABLE, null);
                mtkLogd("setCapability fail,Phone" + i + " is not available");
                synchronized (this) {
                    mIsCapSwitching = false;
                }
                return RC_CANNOT_SWITCH;
            } else if (mSetRafRetryCause == RC_RETRY_CAUSE_RADIO_UNAVAILABLE) {
                mCi[i].unregisterForAvailable(mMtkHandler);
                if (i == mPhones.length - 1) {
                    mSetRafRetryCause = RC_RETRY_CAUSE_NONE;
                }
            }
        }

        int switchStatus = Integer.valueOf(
                SystemProperties.get(PROPERTY_CAPABILITY_SWITCH, "1"));
        // check parameter
        boolean bIsboth3G = false;
        boolean bIsMajorPhone = false;
        int newMajorPhoneId = 0;
        // Check we actually need to do anything
        boolean bIsSameRaf = true;
        for (int i = 0; i < rafs.length; i++) {
            bIsMajorPhone = false;
            if ((rafs[i].getRadioAccessFamily() & RadioAccessFamily.RAF_GPRS) > 0) {
                bIsMajorPhone = true;
            }

            if (bIsMajorPhone) {
                newMajorPhoneId = rafs[i].getPhoneId();
                if (newMajorPhoneId == (switchStatus - 1)) {
                    mSetRafRetryCause = RC_RETRY_CAUSE_NONE;
                    // check if mNextRafs is changed while checking sim switch conditions
                    synchronized (this) {
                        if (mNextRafs != null
                                && newMajorPhoneId == mNextRafs[newMajorPhoneId].getPhoneId()) {
                            mNextRafs = null;
                            mtkLogd("no change, skip setRadioCapability");
                        } else {
                            mtkLogd("no change, skip setRadioCapability and trigger next");
                        }
                    }
                    completeRadioCapabilityTransaction();
                    return RC_NO_NEED_SWITCH;
                }
                if (bIsboth3G) {
                    mtkLogd("set more than one 3G phone, fail");
                    synchronized (this) {
                        mIsCapSwitching = false;
                    }
                    throw new RuntimeException("input parameter is incorrect");
                } else {
                    bIsboth3G = true;
                }
            }
            if (mPhones[i].getRadioAccessFamily() != rafs[i].getRadioAccessFamily()) {
                bIsSameRaf = false;
            }
        }
        if (bIsSameRaf) {
            // All phones are already set to the requested raf
            mtkLogd("setRadioCapability: Already in requested configuration, nothing to do.");
            synchronized (this) {
                mIsCapSwitching = false;
            }
            return RC_NO_NEED_SWITCH;
        }
        if (bIsboth3G == false) {
            synchronized (this) {
                mIsCapSwitching = false;
            }
            throw new RuntimeException("input parameter is incorrect - no 3g phone");
        }

        // External SIM [Start]
        if (SystemProperties.getInt("ro.vendor.mtk_external_sim_support", 0) == 1) {
            // To avoid smart switch before vsim plug in
            for (int i = 0; i < mPhones.length; i++) {
                String isVsimEnabled =
                        TelephonyManager.getDefault().getTelephonyProperty(
                        i, MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
                String isVsimInserted =
                        TelephonyManager.getDefault().getTelephonyProperty(
                        i, MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");
                int defaultPhoneId =
                        MtkSubscriptionController.getMtkInstance().getPhoneId(
                        MtkSubscriptionController.getMtkInstance().getDefaultDataSubId());

                if ("1".equals(isVsimEnabled)
                        && ("0".equals(isVsimInserted) || "".equals(isVsimInserted))
                        && (newMajorPhoneId != defaultPhoneId)) {
                    // Vsim not ready, can't switch to another sim!
                    synchronized (this) {
                        mIsCapSwitching = false;
                    }
                    return RC_NO_NEED_SWITCH;
                }
            }

            // To avoid SIM switch for Remote SIM type
            int mainPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
            String isVsimEnabledOnMain =
                    TelephonyManager.getDefault().getTelephonyProperty(
                    mainPhoneId, MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_ENABLED, "0");
            String mainPhoneIdSimType =
                    TelephonyManager.getDefault().getTelephonyProperty(
                    mainPhoneId, MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");
            int rsimPhoneId = ExternalSimManager.getPreferedRsimSlot();

            if ((isVsimEnabledOnMain.equals("1") && mainPhoneIdSimType.equals("2"))
                    || ((rsimPhoneId != -1) && newMajorPhoneId != rsimPhoneId)) {
                // Rsim enabled, can't switch to another sim!
                synchronized (this) {
                    mIsCapSwitching = false;
                }
                return RC_NO_NEED_SWITCH;
            } else if (SystemProperties.getInt("ro.vendor.mtk_non_dsda_rsim_support", 0) == 1
                    && rsimPhoneId != -1 && rsimPhoneId == newMajorPhoneId) {
                //Rsim enabled, must support switch to Rsim.
                return RC_DO_SWITCH;
            }
        }
        // External SIM [End]

        // check operator spec
        if (!mProxyControllerExt.isNeedSimSwitch(newMajorPhoneId, mPhones.length)) {
            logd("check sim card type and skip setRadioCapability");
            mSetRafRetryCause = RC_RETRY_CAUSE_NONE;
            mNextRafs = null;
            completeRadioCapabilityTransaction();
            return RC_NO_NEED_SWITCH;
        }

        if (!WorldPhoneUtil.isWorldModeSupport() && WorldPhoneUtil.isWorldPhoneSupport()) {
            WorldPhoneUtil.getWorldPhone().notifyRadioCapabilityChange(newMajorPhoneId);
        }
        mtkLogd("checkRadioCapabilitySwitchConditions, do switch");
        return RC_DO_SWITCH;
    }

    private void onRetryWhenRadioAvailable(Message msg) {
        mtkLogd("onRetryWhenRadioAvailable,mSetRafRetryCause:" + mSetRafRetryCause);
        for (int i = 0; i < mPhones.length; i++) {
            if (RadioManager.isModemPowerOff(i)) {
                mtkLogd("onRetryWhenRadioAvailable, Phone" + i + " modem off");
                return;
            }
        }
        if ((mNextRafs != null) && (mSetRafRetryCause == RC_RETRY_CAUSE_RADIO_UNAVAILABLE)) {
            try {
                setRadioCapability(mNextRafs);
            } catch (java.lang.RuntimeException e) {
                e.printStackTrace();
            }
        }
    }

    private BroadcastReceiver mWorldModeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            mtkLogd("mWorldModeReceiver: action = " + action);
            if (!WorldPhoneUtil.isWorldModeSupport() && WorldPhoneUtil.isWorldPhoneSupport()) {
                if (ModemSwitchHandler.ACTION_MODEM_SWITCH_DONE.equals(action)) {
                    if ((mNextRafs != null) &&
                        (mSetRafRetryCause == RC_RETRY_CAUSE_WORLD_MODE_SWITCHING)) {
                        try {
                            if (!setRadioCapability(mNextRafs)) {
                                sendCapabilityFailBroadcast();
                            }
                        } catch (java.lang.RuntimeException e) {
                            sendCapabilityFailBroadcast();
                        }
                    }
                }
            }
        }
    };

    private BroadcastReceiver mCallStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            mtkLogd("mCallStateReceiver: action = " + action);
            if (TelephonyManager.getDefault().getCallState() == TelephonyManager.CALL_STATE_IDLE
                    && !isEccInProgress()) {
                if ((mNextRafs != null) &&
                        (mSetRafRetryCause == RC_RETRY_CAUSE_IN_CALL)) {
                    unRegisterCallStateReceiver();
                    try {
                        if (!setRadioCapability(mNextRafs)) {
                            sendCapabilityFailBroadcast();
                        }
                    } catch (java.lang.RuntimeException e) {
                        sendCapabilityFailBroadcast();
                    }
                }
            }
        }
    };

    private void sendCapabilityFailBroadcast() {
        if (mContext != null) {
            Intent intent = new Intent(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);
            mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
        }
    }

    private void registerWorldModeReceiverFor90Modem() {
        if (mContext == null) {
            logd("registerWorldModeReceiverFor90Modem, context is null => return");
            return;
        }
        IntentFilter filter = new IntentFilter();
        filter.addAction(ModemSwitchHandler.ACTION_MODEM_SWITCH_DONE);
        mContext.registerReceiver(mWorldModeReceiver, filter);
        mHasRegisterWorldModeReceiver = true;
    }

    private void unRegisterWorldModeReceiver() {
        if (mContext == null) {
            mtkLogd("unRegisterWorldModeReceiver, context is null => return");
            return;
        }

        mContext.unregisterReceiver(mWorldModeReceiver);
        mHasRegisterWorldModeReceiver = false;
    }

    private void registerCallStateReceiver() {
        if (mContext == null) {
            mtkLogd("registerCallStateReceiver, context is null => return");
            return;
        }

        IntentFilter filter = new IntentFilter(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_EMERGENCY_CALLBACK_MODE_CHANGED);
        mContext.registerReceiver(mCallStateReceiver, filter);
        mHasRegisterCallStateReceiver = true;
    }

    private void unRegisterCallStateReceiver() {
        if (mContext == null) {
            mtkLogd("unRegisterCallStateReceiver, context is null => return");
            return;
        }

        mContext.unregisterReceiver(mCallStateReceiver);
        mHasRegisterCallStateReceiver = false;
    }

    private boolean isEccInProgress() {
        String value = SystemProperties.get(TelephonyProperties.PROPERTY_INECM_MODE, "");
        boolean inEcm = value.contains("true");
        boolean isInEcc = false;
        ITelecomService tm = ITelecomService.Stub.asInterface(
                    ServiceManager.getService(Context.TELECOM_SERVICE));
        if (tm != null) {
            try {
                isInEcc = tm.isInEmergencyCall();
            } catch (RemoteException e) {
                loge("Exception of isEccInProgress");
            }
        }
        logd("isEccInProgress, value:" + value + ", inEcm:" + inEcm + ", isInEcc:" + isInEcc);
        return inEcm || isInEcc;
    }
    // MTK private functions end
}
