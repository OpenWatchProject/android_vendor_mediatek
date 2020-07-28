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

 /**
 * {@hide}
 */

package com.mediatek.internal.telephony.uicc;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.IccCardConstants.State;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.MccTable;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.SubscriptionController;

import com.android.internal.telephony.uicc.IccCardApplicationStatus;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.PersoSubState;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccCardStatus;
import com.android.internal.telephony.uicc.IccCardStatus.CardState;
import com.android.internal.telephony.uicc.IccCardStatus.PinState;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccProfile;

import com.mediatek.internal.telephony.MtkIccCardConstants;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkSubscriptionInfo;


public class MtkUiccProfile extends UiccProfile {

    protected static final int EVENT_BASE_ID = 100;
    private static final int EVENT_SIM_IO_EX_DONE = EVENT_BASE_ID + 1;
    private static final int EVENT_GET_ATR_DONE = EVENT_BASE_ID + 2;
    private static final int EVENT_OPEN_CHANNEL_WITH_SW_DONE = EVENT_BASE_ID + 3;
    private static final int EVENT_ICC_FDN_CHANGED = EVENT_BASE_ID + 4;

    private RegistrantList mFdnChangedRegistrants = new RegistrantList();

    // MTK-START: SIM ME LOCK
    private PersoSubState mNetworkLockState = PersoSubState.PERSOSUBSTATE_UNKNOWN;
    // MTK-END
    private int mLastAppType = UiccController.APP_FAM_3GPP;

    private static final String ICCID_STRING_FOR_NO_SIM = "N/A";
    private String[] PROPERTY_ICCID_SIM = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };

    static final String[] UICCCARD_PROPERTY_RIL_UICC_TYPE = {
        "vendor.gsm.ril.uicctype",
        "vendor.gsm.ril.uicctype.2",
        "vendor.gsm.ril.uicctype.3",
        "vendor.gsm.ril.uicctype.4",
    };

    private static final String[]  PROPERTY_RIL_FULL_UICC_TYPE = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    public MtkUiccProfile(Context c, CommandsInterface ci, IccCardStatus ics, int phoneId,
            UiccCard uiccCard, Object lock) {
        super(c, ci, ics, phoneId, uiccCard, lock);
        // MTK-START
        if (DBG) log("MtkUiccProfile Creating");
        // MTK-END
    }

    @Override
    protected UiccCardApplication makeUiccApplication(UiccProfile uiccProfile,
                        IccCardApplicationStatus as,
                        Context c,
                        CommandsInterface ci) {
        return new MtkUiccCardApplication(uiccProfile, as, c, ci);
    }

    @Override
    protected boolean isSupportAllNetworkLockCategory() {
        return true;
    }

    // MTK-START
    public final Handler mMtkHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (mDisposed) {
                loge("handleMessage: Received " + msg.what
                        + " after dispose(); ignoring the message");
                return;
            }

            if (DBG) {
                log("mHandlerEx Received message " + msg + "[" + msg.what+  "]");
            }

            switch (msg.what) {
                case EVENT_ICC_FDN_CHANGED:
                    mFdnChangedRegistrants.notifyRegistrants();
                    break;
                case EVENT_SIM_IO_EX_DONE:
                case EVENT_GET_ATR_DONE:
                case EVENT_OPEN_CHANNEL_WITH_SW_DONE:
                    AsyncResult ar = (AsyncResult)msg.obj;
                    if (ar.exception != null) {
                        loge("Error in SIM access with exception" + ar.exception);
                    }
                    AsyncResult.forMessage((Message)ar.userObj, ar.result, ar.exception);
                    ((Message)ar.userObj).sendToTarget();
                    break;
               default:
                    mHandler.handleMessage(msg);
            }
        }
    };
    // MTK-END

    @Override
    protected void registerCurrAppEvents() {
        super.registerCurrAppEvents();

        // MTK-START
        if (mUiccApplication != null && mUiccApplication instanceof MtkUiccCardApplication) {
            ((MtkUiccCardApplication)mUiccApplication).registerForFdnChanged(mMtkHandler,
                    EVENT_ICC_FDN_CHANGED, null);
        }
        // MTK-END
    }

    @Override
    protected void unregisterCurrAppEvents() {
        super.unregisterCurrAppEvents();

        // MTK-START
        if (mUiccApplication != null && mUiccApplication instanceof MtkUiccCardApplication) {
            ((MtkUiccCardApplication)mUiccApplication).unregisterForFdnChanged(mMtkHandler);
        }
        // MTK-END
    }

    @Override
    protected void setCurrentAppType(boolean isGsm) {
        mLastAppType = mCurrentAppType;
        super.setCurrentAppType(isGsm);
    }

    @Override
    protected void setExternalState(IccCardConstants.State newState, boolean override) {
        synchronized (mLock) {
            if (!SubscriptionManager.isValidSlotIndex(getPhoneId())) {
                loge("setExternalState: mPhoneId=" + getPhoneId() + " is invalid; Return!!");
                return;
            }

            // MTK-START
            if (DBG) log("setExternalState(): mExternalState = " + mExternalState +
                    " newState =  " + newState + " override = " + override);
            // MTK-END

            if (!override && newState == mExternalState) {
                // MTK-START: SIM ME LOCK
                if (newState == State.NETWORK_LOCKED &&
                        mNetworkLockState != getNetworkPersoType()) {
                    mNetworkLockState = getNetworkPersoType();
                // MTK-END
                } else if (mExternalState == IccCardConstants.State.LOADED &&
                        mLastAppType != mCurrentAppType &&
                        MtkIccUtilsEx.checkCdma3gCard(getPhoneId())
                                    == MtkIccUtilsEx.CDMA_CARD_TYPE_RUIM_SIM) {
                    if (DBG) {
                        log("Update operatorNumeric for CDMA 3G dual mode card");
                    }
                } else {
                    log("setExternalState: !override and newstate unchanged from " + newState);
                    return;
                // MTK-START: SIM ME LOCK
                }
                // MTK-END
            }

            mExternalState = newState;
            // MTK-START: SIM ME LOCK
            if (newState == State.NETWORK_LOCKED) {
                mNetworkLockState = getNetworkPersoType();
                if (DBG) {
                    log("NetworkLockState =  " + mNetworkLockState);
                }
            // MTK-END
            }
            if (mExternalState == IccCardConstants.State.LOADED) {
                // Update the MCC/MNC.
                if (mIccRecords != null) {
                    String operator = mIccRecords.getOperatorNumeric();
                    log("operator=" + operator + " mPhoneId=" + getPhoneId());

                    if (!TextUtils.isEmpty(operator)) {
                        mTelephonyManager.setSimOperatorNumericForPhone(getPhoneId(), operator);
                        String countryCode = operator.substring(0, 3);
                        if (countryCode != null) {
                            mTelephonyManager.setSimCountryIsoForPhone(getPhoneId(),
                                    MccTable.countryCodeForMcc(Integer.parseInt(countryCode)));
                        } else {
                            loge("EVENT_RECORDS_LOADED Country code is null");
                        }
                    } else {
                        loge("EVENT_RECORDS_LOADED Operator name is null");
                    }
                }
            }
            log("setExternalState: set mPhoneId=" + getPhoneId()
                    + " mExternalState=" + mExternalState);
            mTelephonyManager.setSimStateForPhone(getPhoneId(), getState().toString());

            UiccController.updateInternalIccState(mContext, mExternalState,
                    getIccStateReason(mExternalState), getPhoneId());
        }
    }

    @Override
    protected void setExternalState(State newState) {
        // MTK-START
        if (newState == State.PIN_REQUIRED && mUiccApplication != null) {
            PinState pin1State = mUiccApplication.getPin1State();
            if (pin1State == PinState.PINSTATE_ENABLED_PERM_BLOCKED) {
                if (DBG) log("setExternalState(): PERM_DISABLED");
                setExternalState(State.PERM_DISABLED);
                return;
            }
        }
        // MTK-END
        super.setExternalState(newState);
    }

    @Override
    // MTK-START: SIM ME LOCK
    protected String getIccStateReason(IccCardConstants.State state) {
        if (DBG) log("getIccStateReason E");

        if (State.NETWORK_LOCKED == state && mUiccApplication != null) {
            switch (mUiccApplication.getPersoSubState()) {
                case PERSOSUBSTATE_SIM_NETWORK:
                    return IccCardConstants.INTENT_VALUE_LOCKED_NETWORK;
                case PERSOSUBSTATE_SIM_NETWORK_SUBSET:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_NETWORK_SUBSET;
                case PERSOSUBSTATE_SIM_CORPORATE:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_CORPORATE;
                case PERSOSUBSTATE_SIM_SERVICE_PROVIDER:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_SERVICE_PROVIDER;
                case PERSOSUBSTATE_SIM_SIM:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_SIM;
                case PERSOSUBSTATE_SIM_NETWORK_PUK:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_NETWORK_PUK;
                case PERSOSUBSTATE_SIM_NETWORK_SUBSET_PUK:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_NETWORK_SUBSET_PUK;
                case PERSOSUBSTATE_SIM_CORPORATE_PUK:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_CORPORATE_PUK;
                case PERSOSUBSTATE_SIM_SERVICE_PROVIDER_PUK:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_SERVICE_PROVIDER_PUK;
                case PERSOSUBSTATE_SIM_SIM_PUK:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_SIM_PUK;
                case PERSOSUBSTATE_SIM_NS_SP:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_NS_SP;
                case PERSOSUBSTATE_SIM_NS_SP_PUK:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_NS_SP_PUK;
                case PERSOSUBSTATE_SIM_SIM_C:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_SIM_C;
                case PERSOSUBSTATE_SIM_SIM_C_PUK:
                    return MtkIccCardConstants.INTENT_VALUE_LOCKED_SIM_C_PUK;
                default:
                    return null;
            }
        }

        return super.getIccStateReason(state);
    }
    // MTK-END

    @Override
    public boolean hasIccCard() {
        // MTK-START
        boolean isSimInsert = false;

        // To obtain correct status earily,
        // we use system property value to detemine sim inserted state.
        //if (isSimInsert == false) {
        String iccId = null;
        iccId = SystemProperties.get(PROPERTY_ICCID_SIM[getPhoneId()]);

        //if (DBG) mtkLog("iccId = " + iccId);
        if ((iccId != null) && !(iccId.equals("")) &&
                !(iccId.equals(ICCID_STRING_FOR_NO_SIM))) {
            isSimInsert = true;
        }
        //}

        if (isSimInsert == false && mUiccCard != null &&
                mUiccCard.getCardState() != CardState.CARDSTATE_ABSENT) {
            isSimInsert = true;
        }

        if (DBG) {
            log("hasIccCard(): isSimInsert =  " + isSimInsert + " ,CardState = " +
                    ((mUiccCard != null) ? mUiccCard.getCardState() : "") +
                    ", iccId = " + MtkSubscriptionInfo.givePrintableIccid(iccId));
        }

        return isSimInsert;
        // MTK-END
    }

    @Override
    protected String getSubscriptionDisplayName(int subId, Context context) {
        String nameToSet;

        String simNumeric = mTelephonyManager.getSimOperatorNumeric(subId);
        String simMvnoName = MtkSpnOverride.getInstance()
                .lookupOperatorNameForDisplayName(subId, simNumeric, true, context);
        String simCarrierName = mTelephonyManager.getSimOperatorName(subId);

        if (DBG) {
            log("getSubscriptionDisplayName- simNumeric: " + simNumeric + ", simMvnoName: "
                    + simMvnoName + ", simCarrierName: " + simCarrierName);
        }

        if (mExternalState == IccCardConstants.State.LOADED) {
            if (!TextUtils.isEmpty(simMvnoName)) {
                nameToSet = simMvnoName;
            } else {
                nameToSet = simCarrierName;
            }
        } else {
            nameToSet = "";
        }
        return nameToSet;
    }

    /**
     * Check if to udpate sim operater name with new carrier name.
     * @param newCarrierName the new carrier name.
     */
    @Override
    protected boolean isUdpateCarrierName(String newCarrierName) {
        return (!TextUtils.isEmpty(newCarrierName)
                && (mExternalState == IccCardConstants.State.LOADED));
    }

    // MTK-START
    /**
     * Check whether ICC network lock is enabled
     * This is an async call which returns lock state to applications directly
     */
    public void queryIccNetworkLock(int category, Message onComplete) {
        if (DBG) log("queryIccNetworkLock(): category =  " + category);
        synchronized (mLock) {
            if (mUiccApplication != null) {
                ((MtkUiccCardApplication)mUiccApplication).queryIccNetworkLock(category,
                        onComplete);
            } else if (onComplete != null) {
                Exception e = CommandException.fromRilErrno(RILConstants.RADIO_NOT_AVAILABLE);
                log("Fail to queryIccNetworkLock, hasIccCard = " + hasIccCard());
                AsyncResult.forMessage(onComplete).exception = e;
                onComplete.sendToTarget();
                return;
            }
        }
    }

    /**
     * Set the ICC network lock enabled or disabled
     * When the operation is complete, onComplete will be sent to its handler
     */
    public void setIccNetworkLockEnabled(int category, int lockop, String password,
            String data_imsi, String gid1, String gid2, Message onComplete) {
        if (DBG) log("SetIccNetworkEnabled(): category = " + category
            + " lockop = " + lockop + " password = " + password
            + " data_imsi = " + data_imsi + " gid1 = " + gid1 + " gid2 = " + gid2);
        synchronized (mLock) {
            if (mUiccApplication != null) {
                ((MtkUiccCardApplication)mUiccApplication).setIccNetworkLockEnabled(
                    category, lockop, password, data_imsi, gid1, gid2, onComplete);
            } else if (onComplete != null) {
                Exception e = CommandException.fromRilErrno(RILConstants.RADIO_NOT_AVAILABLE);
                log("Fail to setIccNetworkLockEnabled, hasIccCard = " + hasIccCard());
                AsyncResult.forMessage(onComplete).exception = e;
                onComplete.sendToTarget();
                return;
            }
        }
    }

    /**
     * Notifies handler in case of FDN changed
     */
    @Override
    public void registerForFdnChanged(Handler h, int what, Object obj) {
        synchronized (mLock) {
            synchronized (mLock) {
                Registrant r = new Registrant(h, what, obj);

                mFdnChangedRegistrants.add(r);

                if (getIccFdnEnabled()) {
                    r.notifyRegistrant();
                }
            }
        }
    }

    @Override
    public void unregisterForFdnChanged(Handler h) {
        synchronized (mLock) {
            mFdnChangedRegistrants.remove(h);
        }
    }

    // MTK-END


    // MTK-START: SIM ME LOCK
    /**
     * Query the SIM ME Lock type required to unlock.
     *
     * @return SIM ME Lock type
     */
    public PersoSubState getNetworkPersoType() {
        if (DBG) log("getNetworkPersoType E");
        synchronized (mLock) {
            if (mUiccApplication != null) {
                return mUiccApplication.getPersoSubState();
            }
            return PersoSubState.PERSOSUBSTATE_UNKNOWN;
        }
    }
    // MTK-END

    // MTK-START: CMCC DUAL SIM DEPENDENCY LOCK
    /**
     * Used by SIM ME lock related enhancement feature(Modem SML change feature).
     */
    public void repollIccStateForModemSmlChangeFeatrue(boolean needIntent) {
        if (DBG) log("repollIccStateForModemSmlChangeFeatrue, needIntent = " + needIntent);
        synchronized (mLock) {
            MtkUiccController ctrl = (MtkUiccController)UiccController.getInstance();

            if (ctrl != null) {
                ctrl.repollIccStateForModemSmlChangeFeatrue(getPhoneId(), needIntent);
            }
        }
    }
    // MTK-END

    @Override
    protected Exception covertException(String operation) {
        // MTK flight mode power off modem feature and sim hot plug feature might cause
        // this API is called due to extremely time.
        // Return CommandException to ensure AP could handle the error message normally.
        log("Fail to " + operation + ", hasIccCard = " + hasIccCard());
        return CommandException.fromRilErrno(RILConstants.RADIO_NOT_AVAILABLE);
    }

    public void iccExchangeSimIOEx(int fileID, int command,
            int p1, int p2, int p3, String pathID, String data, String pin2, Message onComplete) {
        mCi.iccIO(command, fileID, pathID, p1, p2, p3, data, pin2,
               mMtkHandler.obtainMessage(EVENT_SIM_IO_EX_DONE, onComplete));
    }

    public void iccGetAtr(Message onComplete) {
        ((MtkRIL)mCi).getATR(mMtkHandler.obtainMessage(EVENT_GET_ATR_DONE, onComplete));
    }

    public String getIccCardType() {
        return SystemProperties.get(UICCCARD_PROPERTY_RIL_UICC_TYPE[getPhoneId()]);
    }

    public String[] getFullIccCardType() {
        return SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[getPhoneId()]).split(",");
    }

    @Override
    protected void log(String msg) {
        Rlog.d(LOG_TAG, msg + " (phoneId " + getPhoneId() + ")");
    }

    @Override
    protected void loge(String msg) {
        Rlog.e(LOG_TAG, msg + " (phoneId " + getPhoneId() + ")");
    }

}

