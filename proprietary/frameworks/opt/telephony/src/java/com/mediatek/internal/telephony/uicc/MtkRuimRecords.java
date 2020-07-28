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

package com.mediatek.internal.telephony.uicc;

import static com.android.internal.telephony.TelephonyProperties.PROPERTY_TEST_CSIM;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.AsyncResult;
import android.os.Message;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.UserManager;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.cdma.sms.UserData;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.MccTable;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccRefreshResponse;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.RuimRecords;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccProfile;
import com.android.internal.util.BitwiseInputStream;
// PHB START @{
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.phb.CsimPhbUtil;
import com.mediatek.internal.telephony.phb.MtkAdnRecordCache;
// PHB END @}
import com.mediatek.internal.telephony.uicc.IccServiceInfo;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Locale;

/**
 * This file is used to process EF related files for other module using
 */
public class MtkRuimRecords extends RuimRecords implements MtkIccConstants {
    static final String LOG_TAG = "MtkRuimRecords";

    // ***** Event Constants
    private static final int EVENT_GET_EST_DONE = 501;
    // PHB START @{
    private static final int EVENT_RADIO_STATE_CHANGED     = 502;
    private static final int EVENT_DELAYED_SEND_PHB_CHANGE = 503;
    private static final int EVENT_PHB_READY               = 504;
    // PHB END @}

    private static final int MCC_LEN = 3;

    private static final int RUIM_FDN_SERVICE_MASK_EXIST_ACTIVE = 0x30;
    private static final int RUIM_FDN_SERVICE_MASK_EXIST_INACTIVE = 0x10;
    private static final int CSIM_FDN_SERVICE_MASK_EXIST = 0x02;
    private static final int CSIM_FDN_SERVICE_MASK_ACTIVE = 0x01;

    private String mRuimImsi = null;
    private int mPhoneId = -1;
    private Phone mPhone;
    private byte[] mSimService;
    private byte[] mEnableService;
    // Send PHB ready after boot complete
    private boolean mPendingPhbNotify = false;

    private static final String[]  PROPERTY_RIL_FULL_UICC_TYPE = {
        "vendor.gsm.ril.fulluicctype",
        "vendor.gsm.ril.fulluicctype.2",
        "vendor.gsm.ril.fulluicctype.3",
        "vendor.gsm.ril.fulluicctype.4",
    };

    // PHB START @{
    // Use different system properties to indicate the GSM/C2K PHB ready state
    static final String PROPERTY_RIL_C2K_PHB_READY  = "vendor.cdma.sim.ril.phbready";
    static final String PROPERTY_RIL_GSM_PHB_READY  = "vendor.gsm.sim.ril.phbready";

    /* To distinguish the PHB event between GSM and C2K,
       we take the first bit as the ready/not ready value
       and the second bit as the type: 0 for GSM and 1 for C2K
    */
    public static final int GSM_PHB_NOT_READY     = 0;
    public static final int GSM_PHB_READY         = 1;
    public static final int C2K_PHB_NOT_READY     = 2;
    public static final int C2K_PHB_READY         = 3;

    public static final int PHB_DELAY_SEND_TIME = 500;

    private int mSubId = -1;
    private boolean mPhbReady = false;
    private boolean mPhbWaitSub = false;
    private boolean mDispose = false;
    private final BroadcastReceiver mIntentReceiver = new BroadcastReceiver () {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED)) {
                // listener radio technology changed. If it not own object phone
                // broadcast false. if it own object, send delay message to broadcast PHB_CHANGE
                // event. APP will receive PHB_CHANGE broadcast and init phonebook.
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                log("[onReceive] ACTION_RADIO_TECHNOLOGY_CHANGED phoneId : " + phoneId);
                if (null != mParentApp && mParentApp.getPhoneId() == phoneId) {
                    String activePhoneName = intent.getStringExtra(PhoneConstants.PHONE_NAME_KEY);
                    int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1);
                    log("[onReceive] ACTION_RADIO_TECHNOLOGY_CHANGED activePhoneName: " +
                            activePhoneName + ", subId : " + subId + ", phoneId: " + phoneId);
                    if ("CDMA".equals(activePhoneName)) {
                        broadcastPhbStateChangedIntent(false, true);
                        sendMessageDelayed(obtainMessage(EVENT_DELAYED_SEND_PHB_CHANGE),
                                PHB_DELAY_SEND_TIME);
                        mAdnCache.reset();
                    }
                }
            } else if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)
                    && mParentApp != null) {
                // Handle sub ready event to notify phb ready event which not
                // sent to application before because sub not ready
                log("[onReceive] onReceive ACTION_SUBINFO_RECORD_UPDATED mPhbWaitSub: "
                        + mPhbWaitSub);
                if (mPhbWaitSub == true) {
                    mPhbWaitSub = false;
                    broadcastPhbStateChangedIntent(mPhbReady, false);
                    return;
                }
            } else if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)
                    && mParentApp != null) {
                int id = intent.getIntExtra(PhoneConstants.PHONE_KEY, PhoneConstants.SIM_ID_1);
                String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                if (id == mPhoneId) {
                    String strPhbReady = null;
                    if (CsimPhbUtil.isUsingGsmPhbReady(mFh)) {
                        strPhbReady = TelephonyManager.getTelephonyProperty(mPhoneId,
                                PROPERTY_RIL_GSM_PHB_READY, "false");
                    } else {
                        strPhbReady = TelephonyManager.getTelephonyProperty(mPhoneId,
                                PROPERTY_RIL_C2K_PHB_READY, "false");
                    }
                    //Update phb ready by sim state.
                    log("sim state: " + simState + ", mPhbReady: " + mPhbReady +
                            ",strPhbReady: " + strPhbReady.equals("true"));
                    if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(simState)) {
                        if (false == mPhbReady && strPhbReady.equals("true")) {
                            mPhbReady = true;
                            broadcastPhbStateChangedIntent(mPhbReady, false);
                        } else if (true == mPhbWaitSub && strPhbReady.equals("true")) {
                            log("mPhbWaitSub is " + mPhbWaitSub + ", broadcast if need");
                            mPhbWaitSub = false;
                            broadcastPhbStateChangedIntent(mPhbReady, false);
                        }
                    }
                }
            } else if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
                log("[onReceive] ACTION_BOOT_COMPLETED mPendingPhbNotify : " + mPendingPhbNotify);
                if (mPendingPhbNotify) {
                    broadcastPhbStateChangedIntent(isPhbReady(), false);
                    mPendingPhbNotify = false;
                }
            }
        }
    };
    // PHB END @}

    public MtkRuimRecords(MtkUiccCardApplication app, Context c, CommandsInterface ci) {
        super(app, c, ci);
        mPhoneId = app.getPhoneId();
        mPhone = PhoneFactory.getPhone(app.getPhoneId());
        if (DBG) log("MtkRuimRecords X ctor this=" + this);

        // PHB START @{
        mAdnCache = new MtkAdnRecordCache(mFh, ci, app);
        ((MtkRIL)mCi).registerForPhbReady(this, EVENT_PHB_READY, null);
        mCi.registerForRadioStateChanged(this, EVENT_RADIO_STATE_CHANGED, null);

        mAdnCache.reset();

        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        filter.addAction(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        filter.addAction(Intent.ACTION_BOOT_COMPLETED);
        mContext.registerReceiver(mIntentReceiver, filter);

        // mAdnCache is needed before onUpdateIccAvailability.
        if (DBG) log("updateIccRecords in IccPhoneBookeInterfaceManager");
        if (mPhone != null && mPhone.getIccPhoneBookInterfaceManager() != null) {
            mPhone.getIccPhoneBookInterfaceManager().updateIccRecords(this);
        }

        // Check if phb is ready or not, if phb was already ready,
        // we won't wait for phb ready
        if (isPhbReady()) {
            mPhbReady = true;
            broadcastPhbStateChangedIntent(mPhbReady, false);
        }
        // PHB END
    }

    @Override
    public void dispose() {
        if (DBG) log("Disposing MtkRuimRecords " + this);
        // MTK-START
        mDispose = true;
        if (!isCdma4GDualModeCard()) {
            log("dispose, reset operator numeric, name and country iso");
            mTelephonyManager.setSimOperatorNumericForPhone(mParentApp.getPhoneId(), "");
            mTelephonyManager.setSimOperatorNameForPhone(mParentApp.getPhoneId(), "");
            mTelephonyManager.setSimCountryIsoForPhone(mParentApp.getPhoneId(),  "");
        }

        // PHB START @{
        if (mPhbReady == true || mPendingPhbNotify == true) {
            log("MtkRuimRecords Disposing set PHB unready mPendingPhbNotify=" +
                    mPendingPhbNotify + "mPhbReady=" + mPhbReady);
            mPhbReady = false;
            mPendingPhbNotify = false;
            broadcastPhbStateChangedIntent(mPhbReady, false);
        }
        mParentApp.unregisterForReady(this);
        mPhbWaitSub = false;
        mCi.unregisterForRadioStateChanged(this);
        ((MtkRIL)mCi).unregisterForPhbReady(this);
        mContext.unregisterReceiver(mIntentReceiver);
        // PHB END @}

        super.dispose();
    }

    @Override
    protected void resetRecords() {
        super.resetRecords();
    }

    @Override
    public String getOperatorNumeric() {
        String imsi = getIMSI();

        if (imsi == null) {
            return null;
        }

        if (mMncLength != UNINITIALIZED && mMncLength != UNKNOWN) {
            return imsi.substring(0, MCC_LEN + mMncLength);
        }
        int mcc = Integer.parseInt(imsi.substring(0, MCC_LEN));
        return imsi.substring(0, MCC_LEN + MccTable.smallestDigitsMccForMnc(mcc));
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;

        byte data[];

        boolean isRecordLoadResponse = false;

        if (mDestroyed.get()) {
            loge("Received message " + msg +
                    "[" + msg.what + "] while being destroyed. Ignoring.");
            return;
        }

        try {
            switch (msg.what) {
                // TODO: probably EF_CST should be read instead
                case EVENT_GET_SST_DONE:
                    log("Event EVENT_GET_SST_DONE Received");
                    // MTK-START
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        logi("EVENT_GET_SST_DONE failed");
                        break;
                    }
                    mSimService = (byte[]) ar.result;

                    log("mSimService[0]: " + mSimService[0] + ", data.length: "
                            + mSimService.length);
                    updateIccFdnStatus();
                    // MTK-END
                    break;

                case EVENT_GET_CDMA_SUBSCRIPTION_DONE:
                    ar = (AsyncResult)msg.obj;
                    String localTemp[] = (String[])ar.result;
                    if (ar.exception != null) {
                        break;
                    }

                    mMyMobileNumber = localTemp[0];
                    mMin2Min1 = localTemp[3];
                    mPrlVersion = localTemp[4];

                    log("MDN: " + MtkIccUtilsEx.getPrintableString(mMyMobileNumber, 8)
                            + " MIN: " + MtkIccUtilsEx.getPrintableString(mMin2Min1, 8));
                    break;

                // For SIM PHB - FDN
                case EVENT_GET_EST_DONE:
                    log("Event EVENT_GET_EST_DONE Received");
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        logi("EVENT_GET_EST_DONE failed");
                        break;
                    }
                    mEnableService = (byte[]) ar.result;

                    log("mEnableService[0]: " + mEnableService[0] +
                            ", mEnableService.length: " + mEnableService.length);
                    updateIccFdnStatus();
                    break;

                // PHB START @{
                case EVENT_PHB_READY:
                    ar = (AsyncResult) msg.obj;
                    log("[DBG]EVENT_PHB_READY ar:" + ar);
                    if (ar != null && ar.exception == null && ar.result != null) {
                        int[] phbReadyState = (int[]) ar.result;
                        boolean isSimLocked = false;
                        int phoneId = mParentApp.getPhoneId();
                        int curSimState = TelephonyManager.SIM_STATE_UNKNOWN;
                        curSimState = SubscriptionController.getInstance().getSimStateForSlotIndex(
                                mPhoneId);
                        isSimLocked = (curSimState == TelephonyManager.SIM_STATE_NETWORK_LOCKED
                                || curSimState == TelephonyManager.SIM_STATE_PIN_REQUIRED);
                        updatePhbStatus(phbReadyState[0], isSimLocked);
                        updateIccFdnStatus();
                    }
                    break;

                case EVENT_DELAYED_SEND_PHB_CHANGE:
                    mPhbReady = isPhbReady();
                    log("[EVENT_DELAYED_SEND_PHB_CHANGE] isReady : " + mPhbReady);
                    broadcastPhbStateChangedIntent(mPhbReady, false);
                    break;
                // PHB END @}

                default:
                    // call super to handles generic record load responses
                    super.handleMessage(msg);
            }
        } catch (RuntimeException exc) {
            // I don't want these exceptions to be fatal
            Rlog.w(LOG_TAG, "Exception parsing RUIM record", exc);
        } finally {
            // Count up record load responses even if they are fails
            if (isRecordLoadResponse) {
                onRecordLoaded();
            }
        }
    }

    @Override
    protected void onAllRecordsLoaded() {
        super.onAllRecordsLoaded();
        // PHB START @{
        if (DBG) {
            log("onAllRecordsLoaded, mParentApp.getType() = " + mParentApp.getType());
        }
        if (mParentApp.getType() == AppType.APPTYPE_RUIM) {
            mFh.loadEFTransparent(EF_CST, obtainMessage(EVENT_GET_SST_DONE));
        } else if (mParentApp.getType() == AppType.APPTYPE_CSIM) {
            mFh.loadEFTransparent(EF_CST, obtainMessage(EVENT_GET_SST_DONE));
            mFh.loadEFTransparent(EF_EST, obtainMessage(EVENT_GET_EST_DONE));
        }
        // PHB END @}
    }

    /**
     * {@inheritDoc}
     *
     * Display rule for RUIMs.
     */
    @Override
    @CarrierNameDisplayConditionBitmask
    public int getCarrierNameDisplayCondition() {
        int rule;
        String spn = getServiceProviderName();
        UiccProfile uiccProfile = UiccController.getInstance().getUiccProfileForPhone(mPhoneId);
        log("getCarrierNameDisplayCondition uiccProfile is "
                + ((uiccProfile != null) ? uiccProfile : "null"));

        if ((uiccProfile != null) && (uiccProfile.getOperatorBrandOverride() != null)) {
            // If the operator has been overridden, treat as the SPN file on the SIM did not exist.
            log("getCarrierNameDisplayCondition, getOperatorBrandOverride is not null");
            rule = CARRIER_NAME_DISPLAY_CONDITION_BITMASK_PLMN;
        } else if (mCsimSpnDisplayCondition == false) {
            // SPN is not required to display
            log("getCarrierNameDisplayCondition, no EF_SPN");
            rule = CARRIER_NAME_DISPLAY_CONDITION_BITMASK_PLMN;
        } else if (!TextUtils.isEmpty(spn) && !spn.equals("")) {
            log("getCarrierNameDisplayCondition, show spn");
            rule = CARRIER_NAME_DISPLAY_CONDITION_BITMASK_SPN;
        } else {
            log("getCarrierNameDisplayCondition, show plmn");
            rule = CARRIER_NAME_DISPLAY_CONDITION_BITMASK_PLMN;
        }
        return rule;
    }

    @Override
    protected void log(String s) {
        Rlog.d("RuimRecords", "[RuimRecords] " + s + " (phoneId " + mPhoneId + ")");
    }

    @Override
    protected void loge(String s) {
        Rlog.e(LOG_TAG, "[MtkRuimRecords] " + s + " (phoneId " + mPhoneId + ")");
    }

    protected void logi(String s) {
        Rlog.i(LOG_TAG, "[MtkRuimRecords] " + s + " (phoneId " + mPhoneId + ")");
    }

    public IccServiceInfo.IccServiceStatus getSIMServiceStatus(
            IccServiceInfo.IccService enService) {
        IccServiceInfo.IccServiceStatus simServiceStatus = IccServiceInfo.IccServiceStatus.UNKNOWN;
        if (mParentApp == null) {
            log("getSIMServiceStatus enService: " + enService + ", mParentApp = null.");
            return simServiceStatus;
        } else {
            log("getSIMServiceStatus enService: " + enService + ", mParentApp.getType(): "
                    + mParentApp.getType());
        }
        if (enService == IccServiceInfo.IccService.FDN && mSimService != null
                && mParentApp.getType() == AppType.APPTYPE_RUIM) {
            log("getSIMServiceStatus mSimService[0]: " + mSimService[0]);
            if (((int)mSimService[0] & RUIM_FDN_SERVICE_MASK_EXIST_ACTIVE)
                    == RUIM_FDN_SERVICE_MASK_EXIST_ACTIVE) {
                simServiceStatus = IccServiceInfo.IccServiceStatus.ACTIVATED;
            } else if (((int)mSimService[0] & RUIM_FDN_SERVICE_MASK_EXIST_INACTIVE)
                    == RUIM_FDN_SERVICE_MASK_EXIST_INACTIVE) {
                simServiceStatus = IccServiceInfo.IccServiceStatus.INACTIVATED;
            } else {
                // FIXME: AOSP has no IccServiceInfo.IccServiceStatus.NOT_EXIST_IN_RUIM;
                simServiceStatus = IccServiceInfo.IccServiceStatus.NOT_EXIST_IN_SIM;
            }
        } else if (enService == IccServiceInfo.IccService.FDN && mSimService != null
                && mEnableService != null && mParentApp.getType() == AppType.APPTYPE_CSIM) {
            log("getSIMServiceStatus mSimService[0]: " + mSimService[0] +
            ", mEnableService[0]: " + mEnableService[0]);
            // FIXME: C.S0065 FDN allocate state is in bit 2 of EFcsim_st, but FDN enable state
            // is in bit 1 of EFest, seems something wrong, need confirm with MD
            if (((int)mSimService[0] & CSIM_FDN_SERVICE_MASK_EXIST) == CSIM_FDN_SERVICE_MASK_EXIST
                    && ((int)mEnableService[0] & CSIM_FDN_SERVICE_MASK_ACTIVE)
                    == CSIM_FDN_SERVICE_MASK_ACTIVE) {
                simServiceStatus = IccServiceInfo.IccServiceStatus.ACTIVATED;
            } else if (((int)mSimService[0] & CSIM_FDN_SERVICE_MASK_EXIST)
                    == CSIM_FDN_SERVICE_MASK_EXIST) {
                simServiceStatus = IccServiceInfo.IccServiceStatus.INACTIVATED;
            } else {
                // FIXME: AOSP has no IccServiceInfo.IccServiceStatus.NOT_EXIST_IN_CSIM;
                simServiceStatus = IccServiceInfo.IccServiceStatus.NOT_EXIST_IN_USIM;
            }
        }
        return simServiceStatus;
    }

    /**
     * Check if current card type is cdma only card.
     * @return true : cdma only card  false: not cdma only card
     */
    public boolean isCdmaOnly() {
        String prop = null;
        String values[] = null;

        if (mPhoneId < 0 || mPhoneId >= PROPERTY_RIL_FULL_UICC_TYPE.length) {
            log("isCdmaOnly: invalid PhoneId " + mPhoneId);
            return false;
        }
        prop = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[mPhoneId]);
        if ((prop != null) && (prop.length() > 0)) {
            values = prop.split(",");
        }
        log("isCdmaOnly PhoneId " + mPhoneId + ", prop value= " + prop +
                ", size= " + ((values != null) ? values.length : 0));
        if (values != null) {
            return !(Arrays.asList(values).contains("USIM")
                    || Arrays.asList(values).contains("SIM"));
        } else {
            return false;
        }
    }

    /**
     * Check if card type is cdma 4G dual mode card.
     * @return true : cdma 4G dual card  false: not
     */
    public boolean isCdma4GDualModeCard() {
        String prop = null;
        String values[] = null;

        if (mPhoneId < 0 || mPhoneId >= PROPERTY_RIL_FULL_UICC_TYPE.length) {
            log("isCdma4GDualModeCard: invalid PhoneId " + mPhoneId);
            return false;
        }
        prop = SystemProperties.get(PROPERTY_RIL_FULL_UICC_TYPE[mPhoneId]);
        if ((prop != null) && (prop.length() > 0)) {
            values = prop.split(",");
        }
        log("isCdma4GDualModeCard PhoneId " + mPhoneId + ", prop value= " + prop +
                ", size= " + ((values != null) ? values.length : 0));
        if (values != null) {
            return (Arrays.asList(values).contains("USIM")
                    && Arrays.asList(values).contains("CSIM"));
        } else {
            return false;
        }
    }

    // FDN status in modem3 should query after PHB ready
    protected void updateIccFdnStatus() {
        log("updateIccFdnStatus mParentAPP=" + mParentApp
                + "  getSIMServiceStatus(Phone.IccService.FDN)="
                + getSIMServiceStatus(IccServiceInfo.IccService.FDN)
                + "  IccServiceStatus.ACTIVATE="
                + IccServiceInfo.IccServiceStatus.ACTIVATED);
        if (mParentApp != null
                && (getSIMServiceStatus(IccServiceInfo.IccService.FDN)
                == IccServiceInfo.IccServiceStatus.ACTIVATED)) {
            mParentApp.queryFdn();
        }
    }

    // PHB START
    public boolean isPhbReady() {
        String strPhbReady = "false";
        String strAllSimState = "";
        String strCurSimState = "";
        boolean isSimLocked = false;
        if (DBG) {
            log("[phbReady] Start mPhbReady: " + (mPhbReady ? "true" : "false"));
        }
        if (mParentApp == null) {
            return false;
        }
        if (CsimPhbUtil.isUsingGsmPhbReady(mFh)) {
            strPhbReady = TelephonyManager.getTelephonyProperty(mPhoneId,
                    PROPERTY_RIL_GSM_PHB_READY, "false");
        } else {
            strPhbReady = TelephonyManager.getTelephonyProperty(mPhoneId,
                    PROPERTY_RIL_C2K_PHB_READY, "false");
        }
        // after Gen93, phb should not care about sim state
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            return (strPhbReady.equals("true"));
        }
        strAllSimState = SystemProperties.get(TelephonyProperties.PROPERTY_SIM_STATE);
        if ((strAllSimState != null) && (strAllSimState.length() > 0)) {
            String values[] = strAllSimState.split(",");
            if ((mPhoneId >= 0) && (mPhoneId < values.length) && (values[mPhoneId] != null)) {
                strCurSimState = values[mPhoneId];
            }
        }
        isSimLocked = (strCurSimState.equals("NETWORK_LOCKED") ||
                       strCurSimState.equals("PIN_REQUIRED"));
        if (DBG) {
            log("[phbReady] End strPhbReady: " + strPhbReady +
                    ", strAllSimState: " + strAllSimState);
        }
        return (strPhbReady.equals("true") && !isSimLocked);
    }

    private void broadcastPhbStateChangedIntent(boolean isReady, boolean isForceSendIntent) {
        // Avoid repeate intent from GSMPhone and CDMAPhone
        // For RuimRecords dispose case, ignore the condition because the phone type may be changed
        if (mPhone != null && (mPhone.getPhoneType() != PhoneConstants.PHONE_TYPE_CDMA)
                && !(mDispose && !isReady)) {
            mPendingPhbNotify = true;
            log("broadcastPhbStateChangedIntent, No active Phone will notfiy when dispose");
            return;
        }

        log("broadcastPhbStateChangedIntent, mPhbReady " + mPhbReady + ", " + mSubId);
        if (isReady == true) {
            int[] subIds = SubscriptionManager.getSubId(mPhoneId);
            if (subIds != null && subIds.length > 0) {
                mSubId = subIds[0];
            }

            if (mSubId <= 0) {
                log("broadcastPhbStateChangedIntent, mSubId <= 0");
                mPhbWaitSub = true;
                return;
            }
        } else {
            if (isForceSendIntent == true && mPhbReady == true) {
                int[] subIds = SubscriptionManager.getSubId(mPhoneId);
                if (subIds != null && subIds.length > 0) {
                    mSubId = subIds[0];
                }
            }
            if (mSubId <= 0) {
                log("broadcastPhbStateChangedIntent, isReady == false and mSubId <= 0");
                return;
            }
        }
        UserManager userManager = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
        boolean isUnlock = userManager.isUserUnlocked();
        if (!SystemProperties.get("sys.boot_completed").equals("1") || !isUnlock) {
            log("broadcastPhbStateChangedIntent, boot not completed, isUnlock:" + isUnlock);
            mPendingPhbNotify = true;
            return;
        }

        Intent intent = new Intent(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
        intent.putExtra("ready", isReady);
        intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
        if (DBG) log("Broadcasting intent ACTION_PHB_STATE_CHANGED " + isReady
                    + " sub id " + mSubId + " phoneId " + mParentApp.getPhoneId());
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
        // send to package boot complete
        Intent bootIntent = new Intent(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
        bootIntent.putExtra("ready", isReady);
        bootIntent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
        bootIntent.setPackage("com.mediatek.simprocessor");
        if (DBG) {
            log("Broadcasting intent ACTION_PHB_STATE_CHANGED to package: simprocessor");
        }
        mContext.sendBroadcastAsUser(bootIntent, UserHandle.ALL);

        if (isReady == false) {
            mSubId = -1;
        }
    }

    private void updatePhbStatus(int status, boolean isSimLocked) {
        log("[PhbStatus] status: " + status + ", isSimLocked: " + isSimLocked
               + ", mPhbReady: " + mPhbReady);
        boolean isReady = false;
        boolean simLockedState = false;
        // after gen93 chip, phb should not care about sim state
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            simLockedState = false;
        } else {
            simLockedState = isSimLocked;
        }
        if (CsimPhbUtil.isUsingGsmPhbReady(mFh)) {
            if (status == GSM_PHB_READY) {
                isReady = true;
            } else if (status == GSM_PHB_NOT_READY) {
                isReady = false;
            } else {
                // not GSM PHB status
                log("[PhbStatus] not GSM PHB status");
                return;
            }
        } else {
            if (status == C2K_PHB_READY) {
                isReady = true;
            } else if (status == C2K_PHB_NOT_READY) {
                isReady = false;
            } else {
                // not C2K PHB status
                log("[PhbStatus] not C2K PHB status");
                return;
            }
        }

        if (isReady) {
            if (false == simLockedState) {
                if (mPhbReady == false) {
                    mPhbReady = true;
                    broadcastPhbStateChangedIntent(mPhbReady, false);
                } else {
                    broadcastPhbStateChangedIntent(mPhbReady, false);
                }
            } else {
                log("[PhbStatus] phb ready but sim is not ready.");
                mPhbReady = false;
                broadcastPhbStateChangedIntent(mPhbReady, false);
            }
        } else {
            if (mPhbReady == true) {
                mAdnCache.reset();
                mPhbReady = false;
                broadcastPhbStateChangedIntent(mPhbReady, false);
            } else {
                broadcastPhbStateChangedIntent(mPhbReady, false);
            }
        }
    }
    // PHB END @}

    @Override
    protected void onGetImsiDone(String imsi) {
        if (mImsi != null && (!mImsi.equals("") && mImsi.length() >= MCC_LEN)) {
            SystemProperties.set("vendor.cdma.icc.operator.mcc", mImsi.substring(0, MCC_LEN));
        }

        if ((mImsi != null) && !mImsi.equals(mRuimImsi)) {
            mRuimImsi = mImsi;
            mImsiReadyRegistrants.notifyRegistrants();
            log("MtkRuimRecords: mImsiReadyRegistrants.notifyRegistrants");
        }
    }

    @Override
    protected void handleRefresh(IccRefreshResponse refreshResponse) {
        if (refreshResponse == null) {
            if (DBG) log("handleRefresh received without input");
            return;
        }

        if (refreshResponse.aid != null && !TextUtils.isEmpty(refreshResponse.aid) &&
                !refreshResponse.aid.equals(mParentApp.getAid())  &&
                (refreshResponse.refreshResult !=
                MtkIccRefreshResponse.REFRESH_INIT_FULL_FILE_UPDATED)) {
            // This is for different app. Ignore.
            return;
        }

        switch (refreshResponse.refreshResult) {
            case IccRefreshResponse.REFRESH_RESULT_INIT:
                if (DBG) log("handleRefresh with SIM_REFRESH_INIT");
                // need to reload all files (that we care about)
                handleFileUpdate(-1);
                break;
            case IccRefreshResponse.REFRESH_RESULT_RESET:
                // Refresh reset is handled by the UiccCard object.
                if (DBG) log("handleRefresh with SIM_REFRESH_RESET");
                break;
            case MtkIccRefreshResponse.REFRESH_INIT_FULL_FILE_UPDATED:
                if (DBG) {
                    log("handleRefresh with REFRESH_INIT_FULL_FILE_UPDATED");
                }
                handleFileUpdate(-1);
                break;
            default:
                // unknown refresh operation
                if (DBG) log("handleRefresh,callback to super");
                super.handleRefresh(refreshResponse);
                break;
        }
    }

    @Override
    public void onReady() {
        mLockedRecordsReqReason = LOCKED_RECORDS_REQ_REASON_NONE;

        super.onReady();
    }

    @Override
    protected void onLocked(int msg) {
        mRecordsRequested = false;
        mLoaded.set(false);
        // Needn't fetch the same EF files when the same mLockedRecordsReqReason comes.
        if ((mLockedRecordsReqReason != LOCKED_RECORDS_REQ_REASON_NONE)) {
            mLockedRecordsReqReason = msg == EVENT_APP_LOCKED ? LOCKED_RECORDS_REQ_REASON_LOCKED :
                LOCKED_RECORDS_REQ_REASON_NETWORK_LOCKED;
            mRecordsToLoad++;
            onRecordLoaded();
            return;
        }

        super.onLocked(msg);
    }

    @Override
    protected void handleFileUpdate(int efid) {
        mLoaded.set(false);
        super.handleFileUpdate(efid);
    }
}
