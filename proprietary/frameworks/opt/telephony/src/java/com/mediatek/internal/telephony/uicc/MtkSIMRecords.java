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

import com.android.internal.telephony.uicc.AdnRecordCache;
import com.android.internal.telephony.uicc.SIMRecords;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.IccRefreshResponse;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.AdnRecordLoader;
import com.android.internal.telephony.uicc.UsimServiceTable;
import com.android.internal.telephony.uicc.AdnRecord;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppState;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.ServiceStateTracker;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.MccTable;
import com.android.internal.telephony.gsm.SimTlv;
import android.app.ActivityManagerNative;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.Context;
import android.text.TextUtils;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.PowerManager;
import android.os.Message;
import android.os.AsyncResult;
import android.os.UserManager;
import android.view.WindowManager;
import android.content.BroadcastReceiver;
import android.content.DialogInterface;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.telephony.Rlog;
import android.content.res.Resources;

import com.mediatek.internal.telephony.uicc.IccServiceInfo;
import com.mediatek.internal.telephony.uicc.MtkIccConstants;
import com.mediatek.internal.telephony.MtkIccUtils;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkSubscriptionManager;

// SS START
import com.mediatek.internal.telephony.MtkSuppServManager;
import com.mediatek.internal.telephony.MtkSuppServHelper;
// SS END

// PHB START
import com.mediatek.internal.telephony.phb.MtkAdnRecordCache;
// PHB END

import static android.Manifest.permission.READ_PHONE_STATE;
// MTK-START: MVNO
import static com.mediatek.internal.telephony.
        MtkTelephonyProperties.PROPERTY_ICC_OPERATOR_DEFAULT_NAME;
// MTK-END

import java.util.ArrayList;

import com.mediatek.internal.telephony.uicc.IMtkSimHandler;
import com.mediatek.internal.telephony.OpTelephonyCustomizationFactoryBase;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;

import java.util.Objects;

/**
 * {@hide}
 */
public class MtkSIMRecords extends SIMRecords {
    protected static final String LOG_TAG_EX = "MtkSIMRecords";
    protected static final boolean ENGDEBUG = TextUtils.equals(android.os.Build.TYPE, "eng");
    protected static final boolean USERDEBUG = TextUtils.equals(android.os.Build.TYPE, "user");
    private static final int MTK_SIM_RECORD_EVENT_BASE = 1000;
    private static final int EVENT_RADIO_AVAILABLE = 1 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_DUAL_IMSI_READY = 4 + MTK_SIM_RECORD_EVENT_BASE;

    private static final int EVENT_QUERY_MENU_TITLE_DONE = 5 + MTK_SIM_RECORD_EVENT_BASE;

    private static final int EVENT_GET_ALL_OPL_DONE = 8 + MTK_SIM_RECORD_EVENT_BASE;;
    private static final int EVENT_GET_CPHSONS_DONE = 9 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_GET_SHORT_CPHSONS_DONE = 10 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_QUERY_ICCID_DONE = 11 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_RADIO_STATE_CHANGED = 12 + MTK_SIM_RECORD_EVENT_BASE;
    // ALPS00302698 ENS
    private static final int EVENT_EF_CSP_PLMN_MODE_BIT_CHANGED = 13 + MTK_SIM_RECORD_EVENT_BASE;
    // ALPS00302702 RAT balancing
    private static final int EVENT_GET_RAT_DONE = 14 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_QUERY_ICCID_DONE_FOR_HOT_SWAP = 15 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_GET_NEW_MSISDN_DONE = 16 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_GET_PSISMSC_DONE = 17 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_GET_SMSP_DONE = 18 + MTK_SIM_RECORD_EVENT_BASE;
    // MTK-START: SIM GBA
    private static final int EVENT_GET_GBABP_DONE = 19 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_GET_GBANL_DONE = 20 + MTK_SIM_RECORD_EVENT_BASE;
    // MTK-END
    private static final int EVENT_CFU_IND = 21 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_IMSI_REFRESH_QUERY = 22 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_IMSI_REFRESH_QUERY_DONE = 23 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_GET_EF_ICCID_DONE = 24 + MTK_SIM_RECORD_EVENT_BASE;

    // PHB START
    private static final int EVENT_DELAYED_SEND_PHB_CHANGE = 26 + MTK_SIM_RECORD_EVENT_BASE;
    private static final int EVENT_PHB_READY = 27 + MTK_SIM_RECORD_EVENT_BASE;
    // PHB END

    private static final int EVENT_GET_ALL_PNN_DONE = 28 + MTK_SIM_RECORD_EVENT_BASE;
    // MTK-START: SIM RSU
    private static final int EVENT_RSU_SIM_LOCK_CHANGED = 29 + MTK_SIM_RECORD_EVENT_BASE;
    // MTK-END

    // PHB START
    /* M: PHB Revise
       To distinguish the PHB event between GSM and C2K,
       we take the first bit as the ready/not ready value
       and the second bit as the type: 0 for GSM and 1 for C2K
    */
    private static final int GSM_PHB_NOT_READY     = 0; // 00
    private static final int GSM_PHB_READY         = 1; // 01

    private static final String SIMRECORD_PROPERTY_RIL_PHB_READY  = "vendor.gsm.sim.ril.phbready";

    private int mSubId = -1;   // Only use for broadcastPhbStateChangedIntent().
    private boolean mPhbReady = false;
    private boolean mPhbWaitSub = false;
    private boolean mIsPhbEfResetDone = false;
    private PhbBroadCastReceiver mPhbReceiver;
    // Send PHB ready after boot complete
    private boolean mPendingPhbNotify = false;
    // PHB END

    private static final String KEY_SIM_ID = "SIM_ID";

    private boolean isValidMBI = false;

    // ALPS00302702 RAT balancing
    private boolean mEfRatLoaded = false;
    private byte[] mEfRat = null;

    private static final String[] LANGUAGE_CODE_FOR_LP = {
        "de", "en", "it", "fr", "es", "nl", "sv", "da", "pt", "fi",
        "no", "el", "tr", "hu", "pl", "",
        "cs", "he", "ar", "ru", "is", "", "", "", "", "",
        "", "", "", "", "", ""
    };

    private BroadcastReceiver mSimReceiver;
    String cphsOnsl;
    String cphsOnss;
    private int iccIdQueryState = -1; // -1: init, 0: query error, 1: query successful
    private boolean hasQueryIccId;

    private int efLanguageToLoad = 0;
    private String mSimImsi = null;
    private byte[] mEfSST = null;
    private byte[] mEfELP = null;
    private byte[] mEfPsismsc = null;
    private byte[] mEfSmsp = null;

    static final String[] SIMRECORD_PROPERTY_RIL_PUK1  = {
        "vendor.gsm.sim.retry.puk1",
        "vendor.gsm.sim.retry.puk1.2",
        "vendor.gsm.sim.retry.puk1.3",
        "vendor.gsm.sim.retry.puk1.4",
    };

    private String[] SIM_RECORDS_PROPERTY_MCC_MNC = {
        "vendor.gsm.ril.uicc.mccmnc",
        "vendor.gsm.ril.uicc.mccmnc.1",
        "vendor.gsm.ril.uicc.mccmnc.2",
        "vendor.gsm.ril.uicc.mccmnc.3",
    };

    public static class OperatorName {
        public String sFullName;
        public String sShortName;
    }

    /*Operator list recode
    * include numeric mcc mnc code
    * and a range of LAC, the operator name index in PNN
    */
    public static class OplRecord {
        public String sPlmn;
        public int nMinLAC;
        public int nMaxLAC;
        public int nPnnIndex;
    }

    //Operator name listed in TS 51.011 EF[PNN] for plmn in operator list(EF[OPL])
    private ArrayList<OperatorName> mPnnNetworkNames = null;
    //Operator list in TS 51.011 EF[OPL]
    private ArrayList<OplRecord> mOperatorList = null;
    // MTK-START: MVNO
    private String mSpNameInEfSpn = null; // MVNO-API
    // MTK-END
    private String mMenuTitleFromEf = null;

    //3g dongle
    private boolean isDispose = false;
    private static final int[] simServiceNumber = {
        1, 17, 51, 52, 54, 55, 56, 0, 12, 3, 7, 0, 0
    };

    private static final int[] usimServiceNumber = {
        0, 19, 45, 46, 48, 49, 51, 71, 12, 2, 0, 42, 0
    };

    private UiccCard mUiccCard;
    private UiccController mUiccController;
    // MTK-START: SIM GBA
    private String mGbabp;
    private ArrayList<byte[]> mEfGbanlList;
    private String[] mGbanl;
    // MTK-END
    private Phone mPhone;
    protected int mSlotId;

    private MtkSpnOverride mSpnOverride = null;
    private OpTelephonyCustomizationFactoryBase mTelephonyCustomizationFactory = null;
    private IMtkSimHandler mMtkSimHandler = null;

    public static final int EVENT_MSISDN = 100; // MSISDN update
    public static final int EVENT_OPL = 101; // Operator name list
    public static final int EVENT_PNN = 102; // Operator name list

    public static final String ATT_OPID = "7";
    public static final String CRICKET_OPID = "145";

    protected String mOldMccMnc = "";
    // AT&T RAT balancing
    public static final int EF_RAT_UNDEFINED = 0xFFFFFF00;
    public static final int EF_RAT_NOT_EXIST_IN_USIM = 0x00000100;
    public static final int EF_RAT_FOR_OTHER_CASE = 0x00000200;

    private String mOldOperatorDefaultName = null;

    public MtkSIMRecords(MtkUiccCardApplication app, Context c, CommandsInterface ci) {
        super(app, c, ci);
        mtkLog("MtkSIMRecords constructor");

        mSlotId = app.getPhoneId();//getSlotId();
        mUiccController = UiccController.getInstance();
        mUiccCard = mUiccController.getUiccCard(mSlotId);
        mtkLog("mUiccCard Instance = " + mUiccCard);
        mPhone = PhoneFactory.getPhone(app.getPhoneId());
        // MTK-START: MVNO
        mSpnOverride = MtkSpnOverride.getInstance();
        // MTK-END
        cphsOnsl = null;
        cphsOnss = null;
        hasQueryIccId = false;

        ((MtkRIL)mCi).registerForCallForwardingInfo(this, EVENT_CFU_IND, null);
        mCi.registerForRadioStateChanged(this, EVENT_RADIO_STATE_CHANGED, null);
        mCi.registerForAvailable(this, EVENT_RADIO_AVAILABLE, null);
        //TODO: HIDL extension impl.
        //mCi.registerForEfCspPlmnModeBitChanged(this, EVENT_EF_CSP_PLMN_MODE_BIT_CHANGED, null);
        ((MtkRIL)mCi).registerForImsiRefreshDone(this, EVENT_IMSI_REFRESH_QUERY, null);
        // MTK-START: SIM TMO RSU
        mTelephonyCustomizationFactory = OpTelephonyCustomizationUtils.getOpFactory(c);
        if (mTelephonyCustomizationFactory != null) {
            // Create duplicate sms plug-in
            mMtkSimHandler = mTelephonyCustomizationFactory.makeMtkSimHandler(c, ci);
            mMtkSimHandler.setPhoneId(mSlotId);
        }
        // MTK-END
        mSimReceiver = new SIMBroadCastReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(mSimReceiver, filter);
        // PHB START
        mAdnCache = new MtkAdnRecordCache(mFh, ci, app);
        ((MtkRIL) mCi).registerForPhbReady(this, EVENT_PHB_READY, null);
        mPhbReceiver = new PhbBroadCastReceiver();
        IntentFilter phbFilter = new IntentFilter();
        phbFilter.addAction(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        /** M: Bug Fix for ALPS02189616 */
        // register new receiver for RADIO_TECHNOLOGY_CHANGED
        phbFilter.addAction(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED);
        phbFilter.addAction(Intent.ACTION_BOOT_COMPLETED);
        mContext.registerReceiver(mPhbReceiver, phbFilter);

        // ALPS01099419, mAdnCache is needed before onUpdateIccAvailability.
        if (DBG) mtkLog("SIMRecords updateIccRecords");
        if (mPhone != null && mPhone.getIccPhoneBookInterfaceManager() != null) {
            mPhone.getIccPhoneBookInterfaceManager().updateIccRecords(this);
        }

        //ALPS00566446: Check if phb is ready or not, if phb was already ready,
        //we won't wait for phb ready.
        if (isPhbReady()) {
            if (DBG) mtkLog("Phonebook is ready.");
            mPhbReady = true;
            broadcastPhbStateChangedIntent(mPhbReady, false);
        }
        // PHB END

        // MTK-START: SIM RSU
        ((MtkRIL) mCi).registerForRsuSimLockChanged(this, EVENT_RSU_SIM_LOCK_CHANGED, null);
        // MTK-END
    }

    @Override
    public void dispose() {
        if (DBG) mtkLog("Disposing MtkSIMRecords this=" + this);
        //3g dongle
        isDispose = true;
        // MTK-START: SIM TMO RSU
        if (mMtkSimHandler != null) {
            mMtkSimHandler.dispose();
        }
        // MTK-END
        ((MtkRIL)mCi).unregisterForCallForwardingInfo(this);
        mCi.unregisterForRadioStateChanged(this);
        //TODO: HIDL extension impl.
        //mCi.unregisterForEfCspPlmnModeBitChanged(this);
        mContext.unregisterReceiver(mSimReceiver);
        mIccId = null;
        mImsi = null;
        // PHB START
        ((MtkRIL) mCi).unregisterForPhbReady(this);
        mContext.unregisterReceiver(mPhbReceiver);
        mPhbWaitSub = false;
        // In resetRecords() will reset it, don't reset twice.
        // mAdnCache.reset();
        // setPhbReady(false);
        if (mPhbReady == true || mPendingPhbNotify == true) {
            mtkLog("MtkSIMRecords Disposing  set PHB unready mPendingPhbNotify=" +
                    mPendingPhbNotify + ", mPhbReady=" + mPhbReady);
            mPhbReady = false;
            mPendingPhbNotify = false;
            broadcastPhbStateChangedIntent(mPhbReady, false);
        }
        // PHB END

        mCallForwardingStatus = CALL_FORWARDING_STATUS_DISABLED;
        new Thread(new Runnable() {
            @Override
            public void run() {
                ((MtkGsmCdmaPhone) mPhone).notifyCallForwardingIndicatorWithoutCheckSimState();
            }
        }).start();

        // MTK-START: SIM RSU
        ((MtkRIL) mCi).unregisterForRsuSimLockChanged(this);
        // MTK-END
        super.dispose();
    }

    protected void resetRecords() {
        super.resetRecords();
        setSystemProperty(PROPERTY_ICC_OPERATOR_DEFAULT_NAME, null);
    }

    // MTK-START
    /* Provide a API to GsmCdmaPhone to check EfCfis. However, we don't use AOSP method to
     * distinguish EF_CFIS. Because MD does't check the MSP, we need to align MD's way.
     */
    public boolean checkEfCfis() {
        boolean isValid = (mEfCfis != null) && (mEfCfis.length == 16);
        mtkLog("mEfCfis is null? = " + (mEfCfis == null));
        return isValid;
    }
    // MTK-END

    @Override
    public String getVoiceMailNumber() {
        mtkLog("getVoiceMailNumber " + MtkIccUtilsEx.getPrintableString(mVoiceMailNum, 8));
        return super.getVoiceMailNumber();
    }

    @Override
    public void setVoiceMailNumber(String alphaTag, String voiceNumber,
            Message onComplete) {

        // MTK-START
        mtkLog("setVoiceMailNumber, mIsVoiceMailFixed:" + mIsVoiceMailFixed +
            ", mMailboxIndex:" + mMailboxIndex + ", isCphsMailboxEnabled:" +
            isCphsMailboxEnabled() + ", alphaTag:" + alphaTag +
            ", voiceNumber:" + MtkIccUtilsEx.getPrintableString(voiceNumber, 8));
        // MTK-END
        super.setVoiceMailNumber(alphaTag, voiceNumber, onComplete);
    }

    // MTK-START SS
    @Override
    public void setVoiceCallForwardingFlag(int line, boolean enable, String dialNumber) {
        Rlog.d(LOG_TAG, "setVoiceCallForwardingFlag: " + enable);

        if (line != 1) return; // only line 1 is supported

        mCallForwardingStatus = enable ? CALL_FORWARDING_STATUS_ENABLED :
                CALL_FORWARDING_STATUS_DISABLED;

        mtkLog(" mRecordsEventsRegistrants: size=" + mRecordsEventsRegistrants.size());
        mRecordsEventsRegistrants.notifyResult(EVENT_CFI);

        try {
            if (checkEfCfis()) {
                // lsb is of byte f1 is voice status
                if (enable) {
                    mEfCfis[1] |= 1;
                } else {
                    mEfCfis[1] &= 0xfe;
                }

                mtkLog("setVoiceCallForwardingFlag: enable=" + enable
                        + " mEfCfis=" + IccUtils.bytesToHexString(mEfCfis));

                // Update dialNumber if not empty and CFU is enabled.
                // Spec reference for EF_CFIS contents, TS 51.011 section 10.3.46.
                if (enable && !TextUtils.isEmpty(dialNumber)) {
                    logv("EF_CFIS: updating cf number, " + Rlog.pii(LOG_TAG, dialNumber));

                    // Fix ALPS03414399
                    // If the dial number contains prefix like "tel:", "sip:" or "sips:",
                    // it has to be truncated before passed to numberToCalledPartyBCD().
                    String tmpDialNumber = convertNumberIfContainsPrefix(dialNumber);
                    byte[] bcdNumber = PhoneNumberUtils.numberToCalledPartyBCD(tmpDialNumber);

                    System.arraycopy(bcdNumber, 0, mEfCfis, CFIS_TON_NPI_OFFSET, bcdNumber.length);

                    mEfCfis[CFIS_BCD_NUMBER_LENGTH_OFFSET] = (byte) (bcdNumber.length);
                    mEfCfis[CFIS_ADN_CAPABILITY_ID_OFFSET] = (byte) 0xFF;
                    mEfCfis[CFIS_ADN_EXTENSION_ID_OFFSET] = (byte) 0xFF;
                }

                // Fix ALPS03458004
                if (mFh != null) {
                    mFh.updateEFLinearFixed(
                            EF_CFIS, 1, mEfCfis, null,
                            obtainMessage (EVENT_UPDATE_DONE, EF_CFIS));
                } else {
                    log("setVoiceCallForwardingFlag: mFh is null, skip update EF_CFIS");
                }
            } else {
                mtkLog("setVoiceCallForwardingFlag: ignoring enable=" + enable
                        + " invalid mEfCfis=" + IccUtils.bytesToHexString(mEfCfis));
            }

            if (mEfCff != null) {
                if (enable) {
                    mEfCff[0] = (byte) ((mEfCff[0] & CFF_LINE1_RESET)
                            | CFF_UNCONDITIONAL_ACTIVE);
                } else {
                    mEfCff[0] = (byte) ((mEfCff[0] & CFF_LINE1_RESET)
                            | CFF_UNCONDITIONAL_DEACTIVE);
                }

                if (mFh != null) {
                    mFh.updateEFTransparent(
                            EF_CFF_CPHS, mEfCff,
                            obtainMessage (EVENT_UPDATE_DONE, EF_CFF_CPHS));
                } else {
                    log("setVoiceCallForwardingFlag: mFh is null, skip update EF_CFF_CPHS");
                }
            }
        } catch (ArrayIndexOutOfBoundsException ex) {
            logw("Error saving call forwarding flag to SIM. "
                            + "Probably malformed SIM record", ex);

        }
    }
    // MTK-END

    public String getSIMCPHSOns() {
        if (cphsOnsl != null) {
            return cphsOnsl;
        } else {
            return cphsOnss;
        }
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        AdnRecord adn;

        byte data[];

        boolean isRecordLoadResponse = false;

        if (mDestroyed.get()) {
            // MTK-START
            if (msg.what != EVENT_AKA_AUTHENTICATE_DONE) {
                mtkLoge("Received message " + msg + "[" + msg.what + "] " +
                        " while being destroyed. Ignoring.");
                return;
            } else {
                mtkLoge("Received message " + msg + "[" + msg.what + "] " +
                        " while being destroyed. Keep going!");
            }
            // MTK-END
        }
        try { switch (msg.what) {
            case EVENT_APP_READY:
                onReady();
                break;

            case EVENT_APP_LOCKED:
                super.handleMessage(msg);
                break;

            case EVENT_GET_MBI_DONE:
                boolean isValidMbdn;
                isRecordLoadResponse = true;

                ar = (AsyncResult)msg.obj;
                data = (byte[]) ar.result;

                isValidMbdn = false;
                if (ar.exception == null) {
                    // Refer TS 51.011 Section 10.3.44 for content details
                    mtkLog("EF_MBI: " + IccUtils.bytesToHexString(data));

                    // Voice mail record number stored first
                    mMailboxIndex = data[0] & 0xff;

                    // check if dailing numbe id valid
                    if (mMailboxIndex != 0 && mMailboxIndex != 0xff) {
                        mtkLog("Got valid mailbox number for MBDN");
                        isValidMbdn = true;
                        // MTK-START
                        this.isValidMBI = true; // ALPS00301018
                        // MTK-END
                    }
                }

                // one more record to load
                mRecordsToLoad += 1;

                if (isValidMbdn) {
                    // Note: MBDN was not included in NUM_OF_SIM_RECORDS_LOADED
                    // MTK-START
                    mtkLog("EVENT_GET_MBI_DONE, to load EF_MBDN");
                    // MTK-END
                    // TODO: mark to avoid build error
                    new AdnRecordLoader(mFh).loadFromEF(EF_MBDN, EF_EXT6,
                            mMailboxIndex, obtainMessage(EVENT_GET_MBDN_DONE));
                // MTK-START
                } else if (isCphsMailboxEnabled()) {
                // MTK-END
                    // If this EF not present, try mailbox as in CPHS standard
                    // CPHS (CPHS4_2.WW6) is a european standard.
                    // MTK-START
                    mtkLog("EVENT_GET_MBI_DONE, to load EF_MAILBOX_CPHS");
                    // MTK-END
                    // TODO: mark to avoid build error
                    new AdnRecordLoader(mFh).loadFromEF(EF_MAILBOX_CPHS,
                            EF_EXT1, 1,
                            obtainMessage(EVENT_GET_CPHS_MAILBOX_DONE));
                // MTK-START
                } else {
                    mtkLog("EVENT_GET_MBI_DONE, do nothing");
                    mRecordsToLoad -= 1;
                // MTK-END
                }

                break;

                case EVENT_GET_MSISDN_DONE:
                    isRecordLoadResponse = true;

                    ar = (AsyncResult)msg.obj;

                    if (ar.exception != null) {
                        mtkLoge("Invalid or missing EF[MSISDN]");
                        break;
                    }

                    adn = (AdnRecord)ar.result;

                    mMsisdn = adn.getNumber();
                    mMsisdnTag = adn.getAlphaTag();

                    // MTK-START
                    mRecordsEventsRegistrants.notifyResult(EVENT_MSISDN);
                    // MTK-END
                    mtkLog("MSISDN: " + MtkIccUtilsEx.getPrintableString(mMsisdn, 8));
                break;
                case EVENT_SET_MSISDN_DONE:
                    isRecordLoadResponse = false;
                    ar = (AsyncResult)msg.obj;

                    if (ar.exception == null) {
                        mMsisdn = mNewMsisdn;
                        mMsisdnTag = mNewMsisdnTag;
                        // MTK-START
                        mRecordsEventsRegistrants.notifyResult(EVENT_MSISDN);
                        // MTK-END
                        mtkLog("Success to update EF[MSISDN]");
                    }

                    if (ar.userObj != null) {
                        AsyncResult.forMessage(((Message) ar.userObj)).exception
                                = ar.exception;
                        ((Message) ar.userObj).sendToTarget();
                    }
                break;

                case EVENT_GET_SPN_DONE:
                    isRecordLoadResponse = true;
                    ar = (AsyncResult) msg.obj;
                    //getSpnFsm(false, ar);
                    mSpnState = GetSpnFsmState.IDLE;
                    if (ar != null && ar.exception == null) {
                        data = (byte[]) ar.result;
                        int displayCondition = data[0] & 0xff;
                        mCarrierNameDisplayCondition = 0;

                        if ((displayCondition & 0x1) == 0x1) {
                            mCarrierNameDisplayCondition
                                    |= CARRIER_NAME_DISPLAY_CONDITION_BITMASK_PLMN;
                        }

                        if ((displayCondition & 0x2) == 0) {
                            mCarrierNameDisplayCondition
                                    |= CARRIER_NAME_DISPLAY_CONDITION_BITMASK_SPN;
                        }

                        setServiceProviderName(IccUtils.adnStringFieldToString(data, 1,
                                data.length - 1));
                        mSpNameInEfSpn = getServiceProviderName();
                        if (mSpNameInEfSpn != null && mSpNameInEfSpn.equals("")) {
                            if (DBG) {
                                mtkLog("set mSpNameInEfSpn to null as parsing result is empty");
                            }
                            mSpNameInEfSpn = null;
                        }

                        if (DBG) log("Load EF_SPN: " + getServiceProviderName()
                                + " carrierNameDisplayCondition: " + mCarrierNameDisplayCondition);
                        mTelephonyManager.setSimOperatorNameForPhone(mParentApp.getPhoneId(),
                                getServiceProviderName());
                    } else {
                        if (DBG) mtkLoge("Read EF_SPN fail!");
                        // See TS 51.011 10.3.11.  Basically, default to
                        // show PLMN always, and SPN also if roaming.
                        mCarrierNameDisplayCondition = DEFAULT_CARRIER_NAME_DISPLAY_CONDITION;
                    }
                break;

                case EVENT_GET_ALL_PNN_DONE:
                    // MTK-START: Revert AOSP fetch sim records flow.
                    isRecordLoadResponse = false;

                    ar = (AsyncResult)msg.obj;
                    // MTK-START
                    //data = (byte[])ar.result;
                    // MTK-END

                    if (ar.exception != null) {
                        break;
                    }

                    // MTK-START
                    parseEFpnn((ArrayList) ar.result);
                    mRecordsEventsRegistrants.notifyResult(EVENT_PNN);
                   if (!mReadingOpl) {
                       mRecordsEventsRegistrants.notifyResult(EVENT_OPL);
                   }
                   /*
                    SimTlv tlv = new SimTlv(data, 0, data.length);

                    for ( ; tlv.isValidObject() ; tlv.nextObject()) {
                        if (tlv.getTag() == TAG_FULL_NETWORK_NAME) {
                            mPnnHomeName
                                = IccUtils.networkNameToString(
                                    tlv.getData(), 0, tlv.getData().length);
                            break;
                        }
                    }
                    */
                    // MTK-END
                break;

                case EVENT_GET_SST_DONE:
                    isRecordLoadResponse = true;

                    ar = (AsyncResult)msg.obj;
                    data = (byte[])ar.result;

                    if (ar.exception != null) {
                        break;
                    }

                    mUsimServiceTable = new UsimServiceTable(data);
                    if (DBG) mtkLog("SST: " + mUsimServiceTable);
                    // MTK-START
                    mEfSST = data;
                    //fetchMbiRecords();
                    //fetchMwisRecords();
                    //fetchPnn();
                    //fetchSpn();
                    //fetchSmsp();
                    // MTK-START: SIM GBA
                    //fetchGbaRecords();
                    // MTK-END
                    // MTK-END
                    break;

                case EVENT_GET_INFO_CPHS_DONE:
                    isRecordLoadResponse = true;

                    ar = (AsyncResult)msg.obj;

                    if (ar.exception != null) {
                        break;
                    }

                    mCphsInfo = (byte[])ar.result;

                    if (DBG) mtkLog("iCPHS: " + IccUtils.bytesToHexString(mCphsInfo));
                    // MTK-START
                    // ALPS00301018
                    if (this.isValidMBI == false && isCphsMailboxEnabled()) {
                        mRecordsToLoad += 1;
                        new AdnRecordLoader(mFh).loadFromEF(EF_MAILBOX_CPHS,
                                    EF_EXT1, 1,
                                    obtainMessage(EVENT_GET_CPHS_MAILBOX_DONE));
                    }
                    // MTK-END
                break;
                // MTK-START
                case EVENT_GET_ALL_OPL_DONE:
                    isRecordLoadResponse = false;

                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        break;
                    }
                    parseEFopl((ArrayList) ar.result);
                    mRecordsEventsRegistrants.notifyResult(EVENT_OPL);
                    break;

                case EVENT_GET_CPHSONS_DONE:
                    if (DBG) mtkLog("handleMessage (EVENT_GET_CPHSONS_DONE)");
                    isRecordLoadResponse = false;

                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception == null) {
                        data = (byte[]) ar.result;
                        cphsOnsl = IccUtils.adnStringFieldToString(
                                data, 0, data.length);
                        if (DBG) mtkLog("Load EF_SPN_CPHS: " + cphsOnsl);
                    }
                    break;

                case EVENT_GET_SHORT_CPHSONS_DONE:
                    if (DBG) mtkLog("handleMessage (EVENT_GET_SHORT_CPHSONS_DONE)");
                    isRecordLoadResponse = false;

                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception == null) {
                        data = (byte[]) ar.result;
                        cphsOnss = IccUtils.adnStringFieldToString(
                                data, 0, data.length);

                        if (DBG) mtkLog("Load EF_SPN_SHORT_CPHS: " + cphsOnss);
                    }
                    break;

                case EVENT_EF_CSP_PLMN_MODE_BIT_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception == null)  {
                        processEfCspPlmnModeBitUrc(((int[]) ar.result)[0]);
                    }
                    break;

                // ALPS00302702 RAT balancing
                case EVENT_GET_RAT_DONE:
                    if (DBG) mtkLog("handleMessage (EVENT_GET_RAT_DONE)");

                    ar = (AsyncResult) msg.obj;
                    mEfRatLoaded = true;
                    if (ar != null && ar.exception == null) {
                        mEfRat = ((byte[]) ar.result);
                        mtkLog("load EF_RAT complete: " + mEfRat[0]);
                        boradcastEfRatContentNotify(EF_RAT_FOR_OTHER_CASE);
                    } else {
                        mtkLog("load EF_RAT fail");
                        mEfRat = null;
                        if (mParentApp.getType() == AppType.APPTYPE_USIM) {
                            boradcastEfRatContentNotify(EF_RAT_NOT_EXIST_IN_USIM);
                        } else {
                            boradcastEfRatContentNotify(EF_RAT_FOR_OTHER_CASE);
                        }
                    }
                    break;

                /*
                  Detail description:
                  This feature provides a interface to get menu title string from EF_SUME
                */
                case EVENT_QUERY_MENU_TITLE_DONE:
                    mtkLog("[sume receive response message");
                    isRecordLoadResponse = true;

                    ar = (AsyncResult) msg.obj;
                    if (ar != null && ar.exception == null) {
                        data = (byte[]) ar.result;
                        if (data != null && data.length >= 2) {
                            int tag = data[0] & 0xff;
                            int len = data[1] & 0xff;
                            mtkLog("[sume tag = " + tag + ", len = " + len);
                            mMenuTitleFromEf = IccUtils.adnStringFieldToString(data, 2, len);
                            mtkLog("[sume menu title is " + mMenuTitleFromEf);
                        }
                    } else {
                        mtkLog("[sume null AsyncResult or exception.");

                        mMenuTitleFromEf = null;
                    }

                    break;
                case EVENT_RADIO_AVAILABLE:
                    mMsisdn = "";
                    //setNumberToSimInfo();
                    mRecordsEventsRegistrants.notifyResult(EVENT_MSISDN);
                    break;

                case EVENT_GET_PSISMSC_DONE:

                    ar = (AsyncResult) msg.obj;
                    data = (byte[]) ar.result;

                    if (ar.exception != null) {
                        break;
                    }

                    mtkLog("EF_PSISMSC: " + IccUtils.bytesToHexString(data));

                    if (data != null) {
                        mEfPsismsc = data;
                    }
                    break;

                case EVENT_GET_SMSP_DONE:

                    ar = (AsyncResult) msg.obj;
                    data = (byte[]) ar.result;

                    if (ar.exception != null) {
                        break;
                    }

                    mtkLog("EF_SMSP: " + IccUtils.bytesToHexString(data));

                    if (data != null) {
                        mEfSmsp = data;
                    }
                    break;
                // MTK-START: SIM GBA
                case EVENT_GET_GBABP_DONE:

                    ar = (AsyncResult) msg.obj;

                    if (ar.exception == null) {
                       data = ((byte[]) ar.result);
                       mGbabp = IccUtils.bytesToHexString(data);

                       if (DBG) mtkLog("EF_GBABP=" + mGbabp);
                    } else {
                        mtkLoge("Error on GET_GBABP with exp " + ar.exception);
                    }
                    break;

                case EVENT_GET_GBANL_DONE:

                    ar = (AsyncResult) msg.obj;

                    if (ar.exception == null) {
                        mEfGbanlList = ((ArrayList<byte[]>) ar.result);
                        if (DBG) mtkLog("GET_GBANL record count: " + mEfGbanlList.size());
                    } else {
                        mtkLoge("Error on GET_GBANL with exp " + ar.exception);
                    }
                    break;
                // MTK-END
                // MTK-START
                case EVENT_CFU_IND:
                    ar = (AsyncResult) msg.obj;
                    /* Line1 is enabled or disabled while reveiving this EVENT */
                    if (ar != null && ar.exception == null && ar.result != null) {
                       /* Line1 is enabled or disabled while reveiving this EVENT */
                       int[] cfuResult = (int[]) ar.result;
                       mtkLog("handle EVENT_CFU_IND: " + cfuResult[0]);
                    }
                    break;
                // MTK-END
                case EVENT_IMSI_REFRESH_QUERY:
                    if (USERDEBUG) {
                        mtkLog("handleMessage (EVENT_IMSI_REFRESH_QUERY)");
                    } else {
                        mtkLog("handleMessage (EVENT_IMSI_REFRESH_QUERY) mImsi= " + getIMSI());
                    }
                    mCi.getIMSIForApp(mParentApp.getAid(),
                        obtainMessage(EVENT_IMSI_REFRESH_QUERY_DONE));
                    break;
                case EVENT_IMSI_REFRESH_QUERY_DONE:
                    mtkLog("handleMessage (EVENT_IMSI_REFRESH_QUERY_DONE)");

                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        loge("Exception querying IMSI, Exception:" + ar.exception);
                        break;
                    }

                    // Remove trailing F's if present in IMSI.
                    mImsi = IccUtils.stripTrailingFs((String) ar.result);
                    if (!Objects.equals(mImsi, (String) ar.result)) {
                        loge("Invalid IMSI padding digits received.");
                    }

                    if (TextUtils.isEmpty(mImsi)) {
                        mImsi = null;
                    }

                    if (mImsi != null && !mImsi.matches("[0-9]+")) {
                        loge("Invalid non-numeric IMSI digits received.");
                        mImsi = null;
                    }

                    // IMSI (MCC+MNC+MSIN) is at least 6 digits, but not more
                    // than 15 (and usually 15).
                    // This will also handle un-set IMSI records (all Fs)
                    if (mImsi != null && (mImsi.length() < 6 || mImsi.length() > 15)) {
                        loge("invalid IMSI " + mImsi);
                        mImsi = null;
                    }

                    log("IMSI: mMncLength=" + mMncLength);

                    if (mImsi != null && mImsi.length() >= 6) {
                        log("IMSI: " + mImsi.substring(0, 6) + Rlog.pii(VDBG, mImsi.substring(6)));
                    }

                    // IMSI has changed so the PLMN might have changed as well
                    updateOperatorPlmn();

                    if (!mImsi.equals(mSimImsi)) {
                        mSimImsi = mImsi;
                        mImsiReadyRegistrants.notifyRegistrants();
                        mtkLog("SimRecords: mImsiReadyRegistrants.notifyRegistrants");
                    }

                    if (mRecordsToLoad == 0 && mRecordsRequested == true) {
                        onAllRecordsLoaded();
                    }
                    break;
                // PHB START
                // PHB Refactoring ++++
                case EVENT_PHB_READY:
                    ar = (AsyncResult) msg.obj;

                    if (ar != null && ar.exception == null && ar.result != null) {

                        int[] phbReadyState = (int[]) ar.result;
                        int curSimState = SubscriptionController.getInstance().
                                getSimStateForSlotIndex(mSlotId);
                        boolean isSimLocked = false;

                        // if power on and in PUK_REQUIRED state, ap will not receive phb ready.
                        isSimLocked = (curSimState == TelephonyManager.SIM_STATE_NETWORK_LOCKED
                                || curSimState == TelephonyManager.SIM_STATE_PIN_REQUIRED);

                        if (DBG)
                            mtkLog("phbReadyState=" + phbReadyState[0] + ",curSimState = " +
                                    curSimState + ", isSimLocked = " + isSimLocked);
                        updatePHBStatus(phbReadyState[0], isSimLocked);
                        updateIccFdnStatus();
                    }
                    break;
                // PHB Refactoring  ----
                /** M: Bug Fix for ALPS02189616 */
                case EVENT_DELAYED_SEND_PHB_CHANGE:
                    mPhbReady = isPhbReady();
                    mtkLog("[EVENT_DELAYED_SEND_PHB_CHANGE] isReady : " + mPhbReady);
                    broadcastPhbStateChangedIntent(mPhbReady, false);
                    break;
                // PHB END
                // MTK-START: SIM RSU
                case EVENT_RSU_SIM_LOCK_CHANGED:
                    if (DBG) {
                        log("[RSU-SIMLOCK] handleMessage (EVENT_RSU_SIM_LOCK_CHANGED)");
                    }
                    ar = (AsyncResult) msg.obj;

                    if (ar != null && ar.exception == null && ar.result != null) {
                        int[] simMelockEvent = (int []) ar.result;

                        if (DBG) {
                            log("[RSU-SIMLOCK] sim melock event = " + simMelockEvent[0]);
                        }

                        RebootClickListener listener = new RebootClickListener();

                        if (simMelockEvent[0] == 0) {
                            AlertDialog alertDialog = new AlertDialog.Builder(mContext)
                                    .setTitle("Unlock Phone")
                                    .setMessage(
                                    "Please restart the phone now since unlock setting has changed"
                                    + ".")
                                    .setPositiveButton("OK", listener)
                                    .create();

                            alertDialog.setCancelable(false);
                            alertDialog.setCanceledOnTouchOutside(false);

                            alertDialog.getWindow().setType(
                                    WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
                            alertDialog.show();
                        }
                    }
                    break;
                // MTK-END
                case EVENT_GET_FPLMN_DONE:
                    ar = (AsyncResult) msg.obj;
                    data = (byte[]) ar.result;
                    if ((ar.exception != null || data == null)
                            && (msg.arg1 == HANDLER_ACTION_SEND_RESPONSE)) {
                        loge("Failed getting Forbidden PLMNs: " + ar.exception);
                        Message response = retrievePendingResponseMessage(msg.arg2);
                        if (response != null) {
                            AsyncResult.forMessage(response, null, ar.exception);
                            response.sendToTarget();
                        } else {
                            loge("Failed to retrieve a response message for FPLMN");
                        }
                    } else {
                        super.handleMessage(msg);
                    }
                    break;
            default:
                super.handleMessage(msg);   // IccRecords handles generic record load responses

        }}catch (RuntimeException exc) {
            // I don't want these exceptions to be fatal
            logw("Exception parsing SIM record", exc);
            } finally {
                // Count up record load responses even if they are fails
                if (isRecordLoadResponse) {
                    onRecordLoaded();
            }
        }
    }

    protected void handleFileUpdate(int efid) {
        switch(efid) {
            case EF_MBDN:
                mRecordsToLoad++;
                new AdnRecordLoader(mFh).loadFromEF(EF_MBDN, EF_EXT6,
                        mMailboxIndex, obtainMessage(EVENT_GET_MBDN_DONE));
                break;
            case EF_MAILBOX_CPHS:
                mRecordsToLoad++;
                new AdnRecordLoader(mFh).loadFromEF(EF_MAILBOX_CPHS, EF_EXT1,
                        1, obtainMessage(EVENT_GET_CPHS_MAILBOX_DONE));
                break;
            case EF_CSP_CPHS:
                mRecordsToLoad++;
                mtkLog("[CSP] SIM Refresh for EF_CSP_CPHS");
                mFh.loadEFTransparent(EF_CSP_CPHS,
                        obtainMessage(EVENT_GET_CSP_CPHS_DONE));
                break;
            case EF_FDN:
                if (DBG) mtkLog("SIM Refresh called for EF_FDN");
                mParentApp.queryFdn();
            // PHB START
            case EF_ADN:
            case EF_SDN:
            case EF_PBR:
                // ALPS00523253: If the file update is related to PHB efid, set phb ready to false
                if (false == mIsPhbEfResetDone) {
                    mIsPhbEfResetDone = true;
                    mAdnCache.reset();
                    mtkLog("handleFileUpdate ADN like");
                    setPhbReady(false);
                }
                break;
            // PHB END
            case EF_MSISDN:
                mRecordsToLoad++;
                mtkLog("SIM Refresh called for EF_MSISDN");
                new AdnRecordLoader(mFh).loadFromEF(EF_MSISDN, getExtFromEf(EF_MSISDN), 1,
                        obtainMessage(EVENT_GET_MSISDN_DONE));
                break;
            case EF_CFIS:
                mRecordsToLoad++;
                mtkLog("SIM Refresh called for EF_CFIS");
                mFh.loadEFLinearFixed(EF_CFIS,
                        1, obtainMessage(EVENT_GET_CFIS_DONE));
                break;
            case EF_CFF_CPHS:
                mRecordsToLoad++;
                mtkLog("SIM Refresh called for EF_CFF_CPHS");
                mFh.loadEFTransparent(EF_CFF_CPHS,
                        obtainMessage(EVENT_GET_CFF_DONE));
                break;
            default:
                // PHB START
                mtkLog("handleFileUpdate default");
                if (((MtkAdnRecordCache) mAdnCache).isUsimPhbEfAndNeedReset(efid) == true) {
                    if (false == mIsPhbEfResetDone) {
                        mIsPhbEfResetDone = true;
                        mAdnCache.reset();
                        setPhbReady(false);
                    }
                }
                // PHB END
                mLoaded.set(false);
                fetchSimRecords();
                break;
        }
    }

    @Override
    protected void handleRefresh(IccRefreshResponse refreshResponse){
        if (refreshResponse == null) {
            if (DBG) mtkLog("handleSimRefresh received without input");
            return;
        }

        if (refreshResponse.aid != null && !TextUtils.isEmpty(refreshResponse.aid) &&
                !refreshResponse.aid.equals(mParentApp.getAid())  &&
                (refreshResponse.refreshResult !=
                MtkIccRefreshResponse.REFRESH_INIT_FULL_FILE_UPDATED)) {
            // This is for different app. Ignore.
            if (DBG) {
                mtkLog("handleRefresh, refreshResponse.aid = " + refreshResponse.aid
                        + ", mParentApp.getAid() = " + mParentApp.getAid());
            }
            return;
        }

        switch (refreshResponse.refreshResult) {
            case IccRefreshResponse.REFRESH_RESULT_FILE_UPDATE:
                if (DBG) mtkLog("handleRefresh with SIM_REFRESH_FILE_UPDATED");

                handleFileUpdate(refreshResponse.efId);
                mIsPhbEfResetDone = false;
                break;
            case IccRefreshResponse.REFRESH_RESULT_INIT:
                if (DBG) mtkLog("handleRefresh with SIM_REFRESH_INIT");
                // need to reload all files (that we care about)
                // setPhbReady(false);
                handleFileUpdate(-1);
                break;
            case IccRefreshResponse.REFRESH_RESULT_RESET:
                // Refresh reset is handled by the UiccCard object.
                if (DBG) mtkLog("handleRefresh with SIM_REFRESH_RESET");
                if (!(SystemProperties.get("ro.vendor.sim_refresh_reset_by_modem").equals("1"))) {
                    if (DBG) mtkLog("sim_refresh_reset_by_modem false");
                    if (mCi != null) {
                        ((MtkRIL) mCi).restartRILD(null);
                    }
                } else {
                    if (DBG) mtkLog("Sim reset by modem!");
                }
                setPhbReady(false);
                handleFileUpdate(-1);
                break;
            case MtkIccRefreshResponse.REFRESH_INIT_FULL_FILE_UPDATED:
                //ALPS00848917: Add refresh type
                if (DBG) {
                    mtkLog("handleRefresh with REFRESH_INIT_FULL_FILE_UPDATED");
                }
                setPhbReady(false);
                handleFileUpdate(-1);
                break;
            case MtkIccRefreshResponse.REFRESH_INIT_FILE_UPDATED:
                if (DBG) {
                    mtkLog("handleRefresh with REFRESH_INIT_FILE_UPDATED, EFID = "
                            +  refreshResponse.efId);
                }
                handleFileUpdate(refreshResponse.efId);
                mIsPhbEfResetDone = false;

                if (mParentApp.getState() == AppState.APPSTATE_READY) {
                    // This will cause files to be reread
                    sendMessage(obtainMessage(EVENT_APP_READY));
                }
                break;
            case MtkIccRefreshResponse.REFRESH_SESSION_RESET:
                if (DBG) {
                    mtkLog("handleSimRefresh with REFRESH_SESSION_RESET");
                }
                // need to reload all files (that we care about)
                handleFileUpdate(-1);
                break;
            default:
                // unknown refresh operation
                if (DBG) mtkLog("handleSimRefresh callback to parent");
                super.handleRefresh(refreshResponse);
                break;
        }
    }

    // MTK-START
    private String findBestLanguage(byte[] languages) {
        String bestMatch = null;
        String[] locales = mContext.getAssets().getLocales();

        if ((languages == null) || (locales == null)) return null;

        // Each 2-bytes consists of one language
        for (int i = 0; (i + 1) < languages.length; i += 2) {
            try {
                String lang = new String(languages, i, 2, "ISO-8859-1");
                if (DBG) mtkLog ("languages from sim = " + lang);
                for (int j = 0; j < locales.length; j++) {
                    if (locales[j] != null && locales[j].length() >= 2 &&
                            locales[j].substring(0, 2).equalsIgnoreCase(lang)) {
                        return lang;
                    }
                }
                if (bestMatch != null) break;
            } catch(java.io.UnsupportedEncodingException e) {
                mtkLog ("Failed to parse USIM language records" + e);
            }
        }
        // no match found. return null
        return null;
    }
    // MTK-END

    @Override
    protected void onAllRecordsLoaded() {
        // MTK-START: SIM ME LOCK
        // Follow AOSP PIN and PUK flow in super.onAllRecordsLoaded(),
        // we also retun here for SIM ME Lock case.
        if (mParentApp.getState() == AppState.APPSTATE_SUBSCRIPTION_PERSO) {
        // MTK-END
            // We should call this since AOSP do it in PIN and PUK locked case.
            // TODO: mark to avoid build errors
            //setVoiceCallForwardingFlagFromSimRecords();
            // reset recordsRequested, since sim is not loaded really
            mRecordsRequested = false;
            // lock state, only update language
            return ;
        }

        super.onAllRecordsLoaded();
        // Follow AOSP PIN and PUK flow in super.onAllRecordsLoaded().
        if (mParentApp.getState() == AppState.APPSTATE_PIN ||
                mParentApp.getState() == AppState.APPSTATE_PUK) {
            // reset recordsRequested, since sim is not loaded really
            mRecordsRequested = false;
            // lock state, only update language
            return ;
        }

        // MTK-START
        setSpnFromConfig(getOperatorNumeric());
        // MTK-END

        // MTK-START: MVNO
        String operator = getOperatorNumeric();

        mtkLog("onAllRecordsLoaded operator = " + operator + ", imsi = "
                + MtkIccUtilsEx.getPrintableString(getIMSI(), 10));

        if (operator != null) {
            String newName = null;
            if (operator.equals("46002") || operator.equals("46007")) {
                operator = "46000";
            }
            newName = MtkSpnOverride.getInstance().lookupOperatorName(
                    MtkSubscriptionManager.getSubIdUsingPhoneId(mParentApp.getPhoneId()),
                    operator, true, mContext);
            setSystemProperty(PROPERTY_ICC_OPERATOR_DEFAULT_NAME, newName);

        }
        fetchPnnAndOpl();
        fetchCPHSOns();
        fetchRatBalancing();
        fetchSmsp();
        fetchGbaRecords();
        // MTK-END
    }

    @Override
    protected boolean checkCdma3gCard() {
        boolean result = (MtkIccUtilsEx.checkCdma3gCard(mSlotId) <= 0);
        log("checkCdma3gCard result: " + result);

        return result;
    }

    //@Override
    protected void setSystemProperty(String key, String val) {
        if (PROPERTY_ICC_OPERATOR_DEFAULT_NAME.equals(key)) {
            if ((mOldOperatorDefaultName == null && val == null) ||
                    (mOldOperatorDefaultName != null && mOldOperatorDefaultName.equals(val))) {
                log("set PROPERTY_ICC_OPERATOR_DEFAULT_NAME same value. val:" + val);
                return;
            } else {
                mOldOperatorDefaultName = val;
            }
        }
        super.setSystemProperty(key, val);
    }

    // MTK-START: SIM
    protected void setSpnFromConfig(String carrier) {
        if (TextUtils.isEmpty(getServiceProviderName()) && mSpnOverride.containsCarrier(carrier)) {
            mTelephonyManager.setSimOperatorNameForPhone(
                    mParentApp.getPhoneId(), mSpnOverride.getSpn(carrier));
        }
    }
    // MTK-END

    @Override
    protected void setVoiceMailByCountry (String spn) {
        super.setVoiceMailByCountry(spn);
        if (mVmConfig.containsCarrier(spn)) {
            // MTK-START
            mtkLog("setVoiceMailByCountry");
            // MTK-END
        }
    }

    protected void fetchSimRecords() {
        super.fetchSimRecords();
    }

    protected boolean isSpnActive() {
        boolean bSpnActive = false;
        String spn = getServiceProviderName();

        if (mEfSST != null && mParentApp != null) {
            if (mParentApp.getType() == AppType.APPTYPE_USIM) {
                if (mEfSST.length >= 3 && (mEfSST[2] & 0x04) == 4) {
                    bSpnActive = true;
                    mtkLog("isSpnActive USIM mEfSST is " +
                     IccUtils.bytesToHexString(mEfSST) + " set bSpnActive to true");
                }
            } else if ((mEfSST.length >= 5) && (mEfSST[4] & 0x02) == 2) {
                bSpnActive = true;
                mtkLog("isSpnActive SIM mEfSST is " +
                    IccUtils.bytesToHexString(mEfSST) + " set bSpnActive to true");
            }
        }

        return bSpnActive;
    }

    // MTK-START: MVNO
    public String getSpNameInEfSpn() {
        if (DBG) mtkLog("getSpNameInEfSpn(): " + mSpNameInEfSpn);
        return mSpNameInEfSpn;
    }

    public String isOperatorMvnoForImsi() {
        MtkSpnOverride spnOverride = MtkSpnOverride.getInstance();
        String imsiPattern = spnOverride.isOperatorMvnoForImsi(getOperatorNumeric(),
                getIMSI());
        String mccmnc = getOperatorNumeric();
        if (DBG) {
            mtkLog("isOperatorMvnoForImsi(), imsiPattern: " + imsiPattern
                + ", mccmnc: " + mccmnc);
        }
        if (imsiPattern == null || mccmnc == null) {
            return null;
        }
        String result = imsiPattern.substring(mccmnc.length(), imsiPattern.length());
        if (DBG) {
            mtkLog("isOperatorMvnoForImsi(): " + result);
        }
        return result;
    }

    public String getFirstFullNameInEfPnn() {
        if (mPnnNetworkNames == null || mPnnNetworkNames.size() == 0) {
            if (DBG) mtkLog("getFirstFullNameInEfPnn(): empty");
            return null;
        }

        OperatorName opName = mPnnNetworkNames.get(0);
        if (DBG) mtkLog("getFirstFullNameInEfPnn(): first fullname: " + opName.sFullName);
        if (opName.sFullName != null)
            return new String(opName.sFullName);
        return null;
    }

    public String isOperatorMvnoForEfPnn() {
        String MCCMNC = getOperatorNumeric();
        String PNN = getFirstFullNameInEfPnn();
        if (DBG) mtkLog("isOperatorMvnoForEfPnn(): mccmnc = " + MCCMNC + ", pnn = " + PNN);
        if (MtkSpnOverride.getInstance().getSpnByEfPnn(MCCMNC, PNN) != null)
            return PNN;
        return null;
    }

    public String getMvnoMatchType() {
        String IMSI = getIMSI();
        String SPN = getSpNameInEfSpn();
        String PNN = getFirstFullNameInEfPnn();
        String GID1 = getGid1();
        String MCCMNC = getOperatorNumeric();
        if (DBG) {
            if (USERDEBUG) {
                mtkLog("getMvnoMatchType(): imsi = ***, mccmnc = " + MCCMNC +
                        ", spn = " + SPN);
            } else {
                mtkLog("getMvnoMatchType(): imsi = " + IMSI + ", mccmnc = " + MCCMNC +
                        ", spn = " + SPN);
            }
        }

        if (MtkSpnOverride.getInstance().getSpnByEfSpn(MCCMNC, SPN) != null)
            return MtkPhoneConstants.MVNO_TYPE_SPN;

        if (MtkSpnOverride.getInstance().getSpnByImsi(MCCMNC, IMSI) != null)
            return MtkPhoneConstants.MVNO_TYPE_IMSI;

        if (MtkSpnOverride.getInstance().getSpnByEfPnn(MCCMNC, PNN) != null)
            return MtkPhoneConstants.MVNO_TYPE_PNN;

        if (MtkSpnOverride.getInstance().getSpnByEfGid1(MCCMNC, GID1) != null)
            return MtkPhoneConstants.MVNO_TYPE_GID;

        return MtkPhoneConstants.MVNO_TYPE_NONE;
    }
    // MTK-END
    // MTK-START
    private class SIMBroadCastReceiver extends BroadcastReceiver {
        public void onReceive(Context content, Intent intent) {
            String action = intent.getAction();
            if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                String reasonExtra = intent.getStringExtra(
                        IccCardConstants.INTENT_KEY_LOCKED_REASON);
                int id = intent.getIntExtra(PhoneConstants.PHONE_KEY, PhoneConstants.SIM_ID_1);
                String simState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                mtkLog("SIM_STATE_CHANGED: phone id = " + id + ",reason = " + reasonExtra
                        + ", simState = " + simState);
                if (IccCardConstants.INTENT_VALUE_LOCKED_ON_PUK.equals(reasonExtra)) {
                    if (id == mSlotId) {
                        String strPuk1Count = null;
                        strPuk1Count = SystemProperties.get(
                                SIMRECORD_PROPERTY_RIL_PUK1[mSlotId], "0");
                        mtkLog("SIM_STATE_CHANGED: strPuk1Count = " + strPuk1Count);
                        //if (strPuk1Count.equals("0")){
                        //    setPhbReady(false);
                        //}
                        mMsisdn = "";
                        //setNumberToSimInfo();
                        mRecordsEventsRegistrants.notifyResult(EVENT_MSISDN);
                    }
                }
                // PHB START
                if (id == mSlotId) {
                    String strPhbReady = "";
                    strPhbReady = TelephonyManager.getTelephonyProperty(mSlotId,
                            SIMRECORD_PROPERTY_RIL_PHB_READY, "false");

                    //Update phb ready by sim state.
                    mtkLog("sim state: " + simState + ", mPhbReady: " + mPhbReady +
                            ",strPhbReady: " + strPhbReady);
                    if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(simState)) {
                        if (false == mPhbReady && strPhbReady.equals("true")) {
                            mPhbReady = true;
                            broadcastPhbStateChangedIntent(mPhbReady, false);

                        } else if (true == mPhbWaitSub && strPhbReady.equals("true")) {
                            mtkLog("mPhbWaitSub is " + mPhbWaitSub + ", broadcast if need");
                            mPhbWaitSub = false;
                            broadcastPhbStateChangedIntent(mPhbReady, false);
                        }
                    }
                }
                // PHB END
            }
        }
    }

    //ALPS00784072: We don't need to update configure if mnc & mnc not changed.
    private void updateConfiguration(String numeric) {
        if (!TextUtils.isEmpty(numeric) && !mOldMccMnc.equals(numeric)) {
            mOldMccMnc = numeric;
            MccTable.updateMccMncConfiguration(mContext, mOldMccMnc);
        } else {
            mtkLog("Do not update configuration if mcc mnc no change.");
        }
    }

    /**
    *parse pnn list
    */
    private void parseEFpnn(ArrayList messages) {
        int count = messages.size();
        if (DBG) mtkLog("parseEFpnn(): pnn has " + count + " records");

        mPnnNetworkNames = new ArrayList<OperatorName>(count);
        for (int i = 0; i < count; i++) {
            byte[] data = (byte[]) messages.get(i);
            if (DBG) {
                mtkLog("parseEFpnn(): pnn record " + i + " content is " +
                        IccUtils.bytesToHexString(data));
            }

            SimTlv tlv = new SimTlv(data, 0, data.length);
            OperatorName opName = new OperatorName();
            for (; tlv.isValidObject(); tlv.nextObject()) {
                if (tlv.getTag() == TAG_FULL_NETWORK_NAME) {
                    opName.sFullName = IccUtils.networkNameToString(
                                tlv.getData(), 0, tlv.getData().length);
                    if (DBG) mtkLog("parseEFpnn(): pnn sFullName is "  + opName.sFullName);
                } else if (tlv.getTag() == TAG_SHORT_NETWORK_NAME) {
                    opName.sShortName = IccUtils.networkNameToString(
                                tlv.getData(), 0, tlv.getData().length);
                    if (DBG) mtkLog("parseEFpnn(): pnn sShortName is "  + opName.sShortName);
                }
            }

            mPnnNetworkNames.add(opName);
        }
    }

    // ALPS00267605 : PNN/OPL revision
    private boolean mReadingOpl = false;

    private void fetchPnnAndOpl() {
        if (DBG) log("fetchPnnAndOpl()");
        //boolean bPnnOplActive = false;
        boolean bPnnActive = false;
        mReadingOpl = false;

        if (mEfSST != null) {
            if (mParentApp.getType() == AppType.APPTYPE_USIM) {
                if (mEfSST.length >= 6) {
                    bPnnActive = ((mEfSST[5] & 0x10) == 0x10);
                    if (bPnnActive) {
                        mReadingOpl = ((mEfSST[5] & 0x20) == 0x20);
                    }
                }
            } else if (mEfSST.length >= 13) {
                bPnnActive = ((mEfSST[12] & 0x30) == 0x30);
                if (bPnnActive) {
                    mReadingOpl = ((mEfSST[12] & 0xC0) == 0xC0);
                }
            }
        }
        if (DBG) log("bPnnActive = " + bPnnActive + ", bOplActive = " + mReadingOpl);

        if (bPnnActive) {
            mFh.loadEFLinearFixedAll(EF_PNN, obtainMessage(EVENT_GET_ALL_PNN_DONE));
            if (mReadingOpl) {
                mFh.loadEFLinearFixedAll(EF_OPL, obtainMessage(EVENT_GET_ALL_OPL_DONE));
            }
        }
    }

    private void fetchSpn() {
        if (DBG) mtkLog("fetchSpn()");
        boolean bSpnActive = false;

        IccServiceInfo.IccServiceStatus iccSerStatus =  getSIMServiceStatus(IccServiceInfo
                .IccService.SPN);
        if (iccSerStatus == IccServiceInfo.IccServiceStatus.ACTIVATED) {
            setServiceProviderName(null);
            mFh.loadEFTransparent(EF_SPN,
                    obtainMessage(EVENT_GET_SPN_DONE));
            mRecordsToLoad++;
        } else {
            if (DBG) mtkLog("[SIMRecords] SPN service is not activated  ");
        }
    }

    public IccServiceInfo.IccServiceStatus getSIMServiceStatus(IccServiceInfo.IccService
            enService) {
        int nServiceNum = enService.getIndex();
        IccServiceInfo.IccServiceStatus simServiceStatus = IccServiceInfo.IccServiceStatus.UNKNOWN;
        if (DBG) {
            mtkLog("getSIMServiceStatus enService is " + enService +
                    " Service Index is " + nServiceNum);
        }

        if (nServiceNum >= 0 &&
                nServiceNum < IccServiceInfo.IccService.UNSUPPORTED_SERVICE.getIndex() &&
                        mEfSST != null) {
            if (mParentApp.getType() == AppType.APPTYPE_USIM) {
                int nUSTIndex = usimServiceNumber[nServiceNum];
                if (nUSTIndex <= 0) {
                    simServiceStatus = IccServiceInfo.IccServiceStatus.NOT_EXIST_IN_USIM;
                } else {
                    int nbyte = nUSTIndex / 8;
                    int nbit = nUSTIndex % 8 ;
                    if (nbit == 0) {
                        nbit = 7;
                        nbyte--;
                    } else {
                        nbit--;
                    }
                    if (DBG) mtkLog("getSIMServiceStatus USIM nbyte: " + nbyte + " nbit: " + nbit);

                    if (mEfSST.length > nbyte && ((mEfSST[nbyte] & (0x1 << nbit)) > 0)) {
                        simServiceStatus = IccServiceInfo.IccServiceStatus.ACTIVATED;
                    } else {
                        simServiceStatus = IccServiceInfo.IccServiceStatus.INACTIVATED;
                    }
                }
            } else {
                int nSSTIndex = simServiceNumber[nServiceNum];
                if (nSSTIndex <= 0) {
                    simServiceStatus = IccServiceInfo.IccServiceStatus.NOT_EXIST_IN_SIM;
                } else {
                    int nbyte = nSSTIndex / 4;
                    int nbit = nSSTIndex % 4;
                    if (nbit == 0) {
                        nbit = 3;
                        nbyte--;
                    } else {
                        nbit--;
                    }

                    int nMask = (0x2 << (nbit * 2));
                    mtkLog("getSIMServiceStatus SIM nbyte: " + nbyte +
                            " nbit: " + nbit + " nMask: " + nMask);
                    if (mEfSST.length > nbyte && ((mEfSST[nbyte] & nMask) == nMask)) {
                        simServiceStatus = IccServiceInfo.IccServiceStatus.ACTIVATED;
                    } else {
                        simServiceStatus = IccServiceInfo.IccServiceStatus.INACTIVATED;
                    }
                }
            }
        }

        mtkLog("getSIMServiceStatus simServiceStatus: " + simServiceStatus);
        return simServiceStatus;
    }

    private void fetchSmsp() {
        if (DBG) mtkLog("fetchSmsp()");

        //For USim authentication.
        if (mUsimServiceTable != null && mParentApp.getType() != AppType.APPTYPE_SIM) {
            if (mUsimServiceTable.isAvailable(UsimServiceTable.UsimService.SM_SERVICE_PARAMS)) {
                if (DBG) mtkLog("SMSP support.");
                mFh.loadEFLinearFixed(MtkIccConstants.EF_SMSP, 1,
                        obtainMessage(EVENT_GET_SMSP_DONE));

                if (mUsimServiceTable.isAvailable(UsimServiceTable.UsimService.SM_OVER_IP)) {
                    if (DBG) mtkLog("PSISMSP support.");
                    mFh.loadEFLinearFixed(MtkIccConstants.EF_PSISMSC, 1,
                            obtainMessage(EVENT_GET_PSISMSC_DONE));
                }

            }
        }
    }
    // MTK-START: SIM GBA
    private void fetchGbaRecords() {
        if (DBG) mtkLog("fetchGbaRecords");

        if (mUsimServiceTable != null && mParentApp.getType() != AppType.APPTYPE_SIM) {
            if (mUsimServiceTable.isAvailable(UsimServiceTable.UsimService.GBA)) {
                if (DBG) mtkLog("GBA support.");
                mFh.loadEFTransparent(MtkIccConstants.EF_ISIM_GBABP,
                        obtainMessage(EVENT_GET_GBABP_DONE));

                mFh.loadEFLinearFixedAll(MtkIccConstants.EF_ISIM_GBANL,
                        obtainMessage(EVENT_GET_GBANL_DONE));
            }
        }
    }
    // MTK-END
    private void fetchMbiRecords() {
        if (DBG) mtkLog("fetchMbiRecords");

        if (mUsimServiceTable != null && mParentApp.getType() != AppType.APPTYPE_SIM) {
            if (mUsimServiceTable.isAvailable(UsimServiceTable.UsimService.MBDN)) {
                if (DBG) mtkLog("MBI/MBDN support.");
                mFh.loadEFLinearFixed(EF_MBI, 1, obtainMessage(EVENT_GET_MBI_DONE));
                mRecordsToLoad++;
            }
        }
    }

    private void fetchMwisRecords() {
        if (DBG) mtkLog("fetchMwisRecords");

        if (mUsimServiceTable != null && mParentApp.getType() != AppType.APPTYPE_SIM) {
            if (mUsimServiceTable.isAvailable(UsimServiceTable.UsimService.MWI_STATUS)) {
                if (DBG) mtkLog("MWIS support.");
                mFh.loadEFLinearFixed(EF_MWIS, 1, obtainMessage(EVENT_GET_MWIS_DONE));
                mRecordsToLoad++;
            }
        }
    }

    /**
    *parse opl list
    */
    private void parseEFopl(ArrayList messages) {
        int count = messages.size();
        if (DBG) mtkLog("parseEFopl(): opl has " + count + " records");

        mOperatorList = new ArrayList<OplRecord>(count);
        for (int i = 0; i < count; i++) {
            byte[] data = (byte[]) messages.get(i);

            OplRecord oplRec = new OplRecord();

            oplRec.sPlmn = MtkIccUtils.parsePlmnToStringForEfOpl(data, 0, 3); // ALPS00316057

            byte[] minLac = new byte[2];
            minLac[0] = data[3];
            minLac[1] = data[4];
            oplRec.nMinLAC = Integer.parseInt(IccUtils.bytesToHexString(minLac), 16);

            byte[] maxLAC = new byte[2];
            maxLAC[0] = data[5];
            maxLAC[1] = data[6];
            oplRec.nMaxLAC = Integer.parseInt(IccUtils.bytesToHexString(maxLAC), 16);

            byte[] pnnRecordIndex = new byte[1];
            pnnRecordIndex[0] = data[7];
            oplRec.nPnnIndex = Integer.parseInt(IccUtils.bytesToHexString(pnnRecordIndex), 16);
            if (DBG) {
                mtkLog("parseEFopl(): record=" + i + " content=" + IccUtils.bytesToHexString(data) +
                        " sPlmn=" + oplRec.sPlmn + " nMinLAC=" + oplRec.nMinLAC +
                        " nMaxLAC=" + oplRec.nMaxLAC + " nPnnIndex=" + oplRec.nPnnIndex);
            }

            mOperatorList.add(oplRec);
        }
    }

    private void boradcastEfRatContentNotify(int item) {
        Intent intent = new Intent(TelephonyIntents.ACTION_EF_RAT_CONTENT_NOTIFY);
        intent.putExtra("ef_rat_status"/*TelephonyIntents.EXTRA_EF_RAT_STATUS*/, item);
        intent.putExtra(PhoneConstants.SLOT_KEY, mSlotId);
        mtkLog("broadCast intent ACTION_EF_RAT_CONTENT_NOTIFY: item: " + item + ", simId: " +
                mSlotId);
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE, UserHandle.USER_ALL);
    }

    // ALPS00302698 ENS
    private void processEfCspPlmnModeBitUrc(int bit) {
        mtkLog("processEfCspPlmnModeBitUrc: bit = " + bit);
        if (bit == 0) {
            mCspPlmnEnabled = false;
        } else {
            mCspPlmnEnabled = true;
        }
        Intent intent = new Intent(TelephonyIntents.ACTION_EF_CSP_CONTENT_NOTIFY);
        intent.putExtra(TelephonyIntents.EXTRA_PLMN_MODE_BIT, bit);
        intent.putExtra(PhoneConstants.SLOT_KEY, mSlotId);
        mtkLog("broadCast intent ACTION_EF_CSP_CONTENT_NOTIFY, EXTRA_PLMN_MODE_BIT: " +  bit);
        ActivityManagerNative.broadcastStickyIntent(intent, READ_PHONE_STATE, UserHandle.USER_ALL);

    }

    /*
      Detail description:
      This feature provides a interface to get menu title string from EF_SUME
    */
    public String getMenuTitleFromEf() {
        return mMenuTitleFromEf;
    }

    private void fetchCPHSOns() {
        if (DBG) mtkLog("fetchCPHSOns()");
        cphsOnsl = null;
        cphsOnss = null;
        mFh.loadEFTransparent(EF_SPN_CPHS,
               obtainMessage(EVENT_GET_CPHSONS_DONE));
        mFh.loadEFTransparent(
               EF_SPN_SHORT_CPHS, obtainMessage(EVENT_GET_SHORT_CPHSONS_DONE));
    }

    // ALPS00302702 RAT balancing START
    private void fetchRatBalancing() {
        if (!isFetchRatBalancingAndEnsFile(mSlotId)) {
            mtkLog("Not support MTK_RAT_BALANCING");
            return;
        }
        if (mParentApp.getType() == AppType.APPTYPE_USIM) {
            mtkLog("start loading EF_RAT");
            mFh.loadEFTransparent(MtkIccConstants.EF_RAT, obtainMessage(EVENT_GET_RAT_DONE));
        }
        else if (mParentApp.getType() == AppType.APPTYPE_SIM) {
            // broadcast & set no file
            mtkLog("loading EF_RAT fail, because of SIM");
            mEfRatLoaded = false;
            mEfRat = null;
            boradcastEfRatContentNotify(EF_RAT_FOR_OTHER_CASE);
        }
        else {
            mtkLog("loading EF_RAT fail, because of +EUSIM");
        }
    }

    public int getEfRatBalancing() {
        mtkLog("getEfRatBalancing: iccCardType = " + mParentApp.getType()
                + ", mEfRatLoaded = " + mEfRatLoaded + ", mEfRat is null = " + (mEfRat == null));

        if ((mParentApp.getType() == AppType.APPTYPE_USIM) && mEfRatLoaded && mEfRat == null) {
            return EF_RAT_NOT_EXIST_IN_USIM;
        }
        return EF_RAT_FOR_OTHER_CASE;
    }
    // ALPS00359372 for at&t testcase, mnc 2 should match 3 digits
    private boolean isMatchingPlmnForEfOpl(String simPlmn, String bcchPlmn) {
        if (simPlmn == null || simPlmn.equals("") || bcchPlmn == null || bcchPlmn.equals(""))
            return false;

        if (DBG) mtkLog("isMatchingPlmnForEfOpl(): simPlmn = " + simPlmn + ", bcchPlmn = " +
                bcchPlmn);

        /*  3GPP TS 23.122 Annex A (normative): HPLMN Matching Criteria
            For PCS1900 for North America, regulations mandate that a 3-digit MNC shall be used;
            however during a transition period, a 2 digit MNC may be broadcast by the Network and,
            in this case, the 3rd digit of the SIM is stored as 0 (this is the 0 suffix rule). */
        int simPlmnLen = simPlmn.length();
        int bcchPlmnLen = bcchPlmn.length();
        if (simPlmnLen < 5 || bcchPlmnLen < 5)
            return false;

        int i = 0;
        for (i = 0; i < 5; i++) {
            if (simPlmn.charAt(i) == 'd')
                continue;
            if (simPlmn.charAt(i) != bcchPlmn.charAt(i))
                return false;
        }

        if (simPlmnLen == 6 && bcchPlmnLen == 6) {
            if (simPlmn.charAt(5) == 'd' || simPlmn.charAt(5) == bcchPlmn.charAt(5)) {
                return true;
            } else {
                return false;
            }
        } else if (bcchPlmnLen == 6 && bcchPlmn.charAt(5) != '0' && bcchPlmn.charAt(5) != 'd') {
            return false;
        } else if (simPlmnLen == 6 && simPlmn.charAt(5) != '0' && simPlmn.charAt(5) != 'd') {
            return false;
        }

        return true;
    }

    private boolean isPlmnEqualsSimNumeric(String plmn) {
        String mccmnc = getOperatorNumeric();
        if (plmn == null) return false;

        if (mccmnc == null || mccmnc.equals("")) {
            mtkLog("isPlmnEqualsSimNumeric: getOperatorNumeric error: " + mccmnc);
            return false;
        }

        if (plmn.equals(mccmnc)) {
            return true;
        } else {
            if (plmn.length() == 5 && mccmnc.length() == 6
                && plmn.equals(mccmnc.substring(0, 5))) {
                return true;
            }
        }
        return false;
    }


    // ALPS00267605 : PNN/OPL revision
    public String getEonsIfExist(String plmn, int nLac, boolean bLongNameRequired) {
        if (DBG) {
            char mask = '*';
            StringBuilder lac_sb = new StringBuilder(Integer.toHexString(nLac));
            if (lac_sb.length() == 1 || lac_sb.length() == 2) {
                lac_sb.setCharAt(0, mask);
            } else {
                for (int i = 0 ; i < lac_sb.length()/2 ; i++) {
                    lac_sb.setCharAt(i, mask);
                }
            }
            mtkLog("EONS getEonsIfExist: plmn is " + plmn + " nLac is " +
                    lac_sb.toString() + " bLongNameRequired: " + bLongNameRequired);
        }
        if (plmn == null || mPnnNetworkNames == null || mPnnNetworkNames.size() == 0) {
            return null;
        }

        int nPnnIndex = -1;
        boolean isHPLMN = isPlmnEqualsSimNumeric(plmn);

        if (mOperatorList == null) {
            // case for EF_PNN only
            if (isHPLMN) {
                if (DBG) mtkLog("getEonsIfExist: Plmn is HPLMN, return PNN's first record");
                nPnnIndex = 1;
            } else {
                if (DBG) {
                    mtkLog("getEonsIfExist: Plmn is not HPLMN and no mOperatorList, return null");
                }
                return null;
            }
        } else {
            //search EF_OPL using plmn & nLac
            for (int i = 0; i < mOperatorList.size(); i++) {
                OplRecord oplRec = mOperatorList.get(i);

                // ALPS00316057
                //if((plmn.equals(oplRec.sPlmn) ||(!oplRec.sPlmn.equals("") &&
                //      plmn.startsWith(oplRec.sPlmn))) &&
                if (isMatchingPlmnForEfOpl(oplRec.sPlmn, plmn) &&
                        ((oplRec.nMinLAC == 0 && oplRec.nMaxLAC == 0xfffe) ||
                        (oplRec.nMinLAC <= nLac && oplRec.nMaxLAC >= nLac))) {
                    if (DBG) mtkLog("getEonsIfExist: find it in EF_OPL");
                    if (oplRec.nPnnIndex == 0) {
                        if (DBG) {
                            mtkLog("getEonsIfExist: oplRec.nPnnIndex is 0, from other sources");
                        }
                        return null;
                    }
                    nPnnIndex = oplRec.nPnnIndex;
                    break;
                }
            }
        }

        //ALPS00312727, 11603, add check (mOperatorList.size() == 1
        if (nPnnIndex == -1 && isHPLMN && (mOperatorList.size() == 1)) {
            if (DBG) {
                mtkLog("getEonsIfExist: not find it in EF_OPL, but Plmn is HPLMN," +
                        " return PNN's first record");
            }
            nPnnIndex = 1;
        }
        else if (nPnnIndex > 1 && nPnnIndex > mPnnNetworkNames.size() && isHPLMN) {
            if (DBG) {
                mtkLog("getEonsIfExist: find it in EF_OPL, but index in EF_OPL > EF_PNN list" +
                        " length & Plmn is HPLMN, return PNN's first record");
            }
            nPnnIndex = 1;
        }
        else if (nPnnIndex > 1 && nPnnIndex > mPnnNetworkNames.size() && !isHPLMN) {
            if (DBG) {
                mtkLog("getEonsIfExist: find it in EF_OPL, but index in EF_OPL > EF_PNN list" +
                        " length & Plmn is not HPLMN, return PNN's first record");
            }
            nPnnIndex = -1;
        }

        String sEons = null;
        if (nPnnIndex >= 1) {
            OperatorName opName = mPnnNetworkNames.get(nPnnIndex - 1);
            if (bLongNameRequired) {
                if (opName.sFullName != null) {
                    sEons = new String(opName.sFullName);
                } else if (opName.sShortName != null) {
                    sEons = new String(opName.sShortName);
                }
            } else if (!bLongNameRequired) {
                if (opName.sShortName != null) {
                    sEons = new String(opName.sShortName);
                } else if (opName.sFullName != null) {
                    sEons = new String(opName.sFullName);
                }
            }
        }
        if (DBG) mtkLog("getEonsIfExist: sEons is " + sEons);

        return sEons;

        /*int nPnnIndex = -1;
        //check if the plmn is Hplmn, return the first record of pnn
        if (isHPlmn(plmn)) {
            nPnnIndex = 1;
            if (DBG) mtkLog("EONS getEonsIfExist Plmn is hplmn");
        } else {
            //search the plmn from opl and if the LAC in the range of opl
            for (int i = 0; i < mOperatorList.size(); i++) {
                OplRecord oplRec = mOperatorList.get(i);
                //check if the plmn equals with the plmn in the operator list
                //or starts with the plmn in the operator list(which include wild char 'D')
                if((plmn.equals(oplRec.sPlmn) ||(!oplRec.sPlmn.equals("") &&
                        plmn.startsWith(oplRec.sPlmn))) &&
                        ((oplRec.nMinLAC == 0 && oplRec.nMaxLAC == 0xfffe) ||
                        (oplRec.nMinLAC <= nLac && oplRec.nMaxLAC >= nLac))) {
                    nPnnIndex = oplRec.nPnnIndex;
                    break;
                }
                if (DBG) mtkLog("EONS getEonsIfExist record number is " + i + " sPlmn: " +
                        oplRec.sPlmn + " nMinLAC: " + oplRec.nMinLAC + " nMaxLAC: " +
                        oplRec.nMaxLAC + " PnnIndex " + oplRec.nPnnIndex);
            }
            if (nPnnIndex == 0) {
                // not HPLMN and the index is 0 indicates that the
                // name is to be taken from other sources
                return null;
            }
        }
        if (DBG) mtkLog("EONS getEonsIfExist Index of pnn is  " + nPnnIndex);

        String sEons = null;
        if (nPnnIndex >= 1) {
            OperatorName opName = mPnnNetworkNames.get(nPnnIndex - 1);
            if (bLongNameRequired) {
                if (opName.sFullName != null) {
                    sEons = new String(opName.sFullName);
                } else if (opName.sShortName != null) {
                    sEons = new String(opName.sShortName);
                }
            } else if (!bLongNameRequired ) {
                if (opName.sShortName != null) {
                    sEons = new String(opName.sShortName);
                } else if (opName.sFullName != null) {
                    sEons = new String(opName.sFullName);
                }
            }
        }
        if (DBG) mtkLog("EONS getEonsIfExist sEons is " + sEons);
        return sEons;*/
    }

    // MTK-START: SIM GBA
    /**
     * Returns the GBA bootstrapping parameters (GBABP) that was loaded from the USIM.
     * @return GBA bootstrapping parameters or null if not present or not loaded
     */
    public String getEfGbabp() {
        mtkLog("GBABP = " + mGbabp);
        return mGbabp;
    }

    /**
     * Set the GBA bootstrapping parameters (GBABP) value into the USIM.
     * @param gbabp a GBA bootstrapping parameters value in String type
     * @param onComplete
     *        onComplete.obj will be an AsyncResult
     *        ((AsyncResult)onComplete.obj).exception == null on success
     *        ((AsyncResult)onComplete.obj).exception != null on fail
     */
    public void setEfGbabp(String gbabp, Message onComplete) {
        byte[] data = IccUtils.hexStringToBytes(gbabp);

        mFh.updateEFTransparent(MtkIccConstants.EF_GBABP, data, onComplete);
    }
    // MTK-END

    /**
     * Returns the Public Service Identity of the SM-SC (PSISMSC) that was loaded from the USIM.
     * @return PSISMSC or null if not present or not loaded
     */
    public byte[] getEfPsismsc() {
        return mEfPsismsc;
    }

    /**
     * Returns the Short message parameter (SMSP) that was loaded from the USIM.
     * @return PSISMSC or null if not present or not loaded
     */
    public byte[] getEfSmsp() {
        return mEfSmsp;
    }

    /**
     * Returns the MCC+MNC length that was loaded from the USIM.
     * @return MCC+MNC length or 0 if not present or not loaded
     */
    public int getMncLength() {
        mtkLog("mncLength = " + mMncLength);
        return mMncLength;
    }

    // MTK-START: SIM RSU
    /**
     * Reboot listener.
     */
    private class RebootClickListener implements DialogInterface.OnClickListener {
        @Override
        public void onClick(DialogInterface dialog, int which) {
            log("[RSU-SIMLOCK] Unlock Phone onClick");
            PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
            pm.reboot("Unlock state changed");
        }
    }
    // MTK-END

    // add for alps01947090
    public boolean isRadioAvailable() {
        if (mCi != null) {
          return (mCi.getRadioState() != TelephonyManager.RADIO_POWER_UNAVAILABLE);
        }
        return false;
    }
     // FDN support start
     protected void updateIccFdnStatus() {
     }
    // FDN support end

    // PHB START
    private void broadcastPhbStateChangedIntent(boolean isReady, boolean isForceSendIntent) {
        // M: for avoid repeate intent from GSMPhone and CDMAPhone
        // For SimRecords dispose case, ignore this condition because the phone type may be changed
        if (mPhone != null && (mPhone.getPhoneType() != PhoneConstants.PHONE_TYPE_GSM) &&
                !(isDispose && !isReady)) {
            mPendingPhbNotify = true;
            mtkLog("broadcastPhbStateChangedIntent, No active Phone, will notfiy when dispose");
            return;
        }

        mtkLog("broadcastPhbStateChangedIntent, mPhbReady " + mPhbReady + ", " + mSubId);
        if (isReady == true) {
            mSubId = MtkSubscriptionManager.getSubIdUsingPhoneId(mSlotId);

            int curSimState = SubscriptionController.getInstance().
                    getSimStateForSlotIndex(mSlotId);

            if (mSubId <= 0 || curSimState == TelephonyManager.SIM_STATE_UNKNOWN) {
                mtkLog("broadcastPhbStateChangedIntent, mSubId " + mSubId
                    + ", sim state " + curSimState);
                mPhbWaitSub = true;
                return;
            }
        } else {
            if (isForceSendIntent == true && mPhbReady == true) {
                mSubId = MtkSubscriptionManager.getSubIdUsingPhoneId(mSlotId);
            }
            if (mSubId <= 0) {
                mtkLog("broadcastPhbStateChangedIntent, isReady == false and mSubId <= 0");
                return;
            }
        }
        UserManager userManager = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
        boolean isUnlock = userManager.isUserUnlocked();
        if (!SystemProperties.get("sys.boot_completed").equals("1") || !isUnlock) {
            mtkLog("broadcastPhbStateChangedIntent, boot not completed, isUnlock:" + isUnlock);
            mPendingPhbNotify = true;
            return;
        }
        Intent intent = new Intent(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
        intent.putExtra("ready", isReady);
        intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
        if (DBG) mtkLog("Broadcasting intent ACTION_PHB_STATE_CHANGED " + isReady
                    + " sub id " + mSubId + " phoneId " + mParentApp.getPhoneId());
        mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
        // send to package boot complete
        Intent bootIntent = new Intent(TelephonyIntents.ACTION_PHB_STATE_CHANGED);
        bootIntent.putExtra("ready", isReady);
        bootIntent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mSubId);
        bootIntent.setPackage("com.mediatek.simprocessor");
        if (DBG) {
            mtkLog("Broadcasting ACTION_PHB_STATE_CHANGED to package: simprocessor");
        }
        mContext.sendBroadcastAsUser(bootIntent, UserHandle.ALL);

        if (isReady == false) {
            mSubId = -1;
        }
    }

    public boolean isPhbReady() {
        String strPhbReady = "false";
        String strAllSimState = "";
        String strCurSimState = "";
        boolean isSimLocked = false;
        if (DBG) {
            mtkLog("phbReady(): cached mPhbReady = " + (mPhbReady ? "true" : "false"));
        }
        if (mParentApp == null || mPhone == null) {
            return false;
        }

        strPhbReady = TelephonyManager.getTelephonyProperty(mSlotId,
                "vendor.gsm.sim.ril.phbready", "false");
        // after 93 chip, phb should care about sim state
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            if (DBG) {
                mtkLog("phbReady(): strPhbReady = " + strPhbReady);
            }
            return (strPhbReady.equals("true"));
        }
        strAllSimState = SystemProperties.get(TelephonyProperties.PROPERTY_SIM_STATE);
        if ((strAllSimState != null) && (strAllSimState.length() > 0)) {
            String values[] = strAllSimState.split(",");
            if ((mSlotId >= 0) && (mSlotId < values.length) && (values[mSlotId] != null)) {
                strCurSimState = values[mSlotId];
            }
        }
        isSimLocked = (strCurSimState.equals("NETWORK_LOCKED") ||
                       strCurSimState.equals("PIN_REQUIRED"));

        if (DBG) {
            mtkLog("phbReady(): strPhbReady = " + strPhbReady +
                    ", strAllSimState = " + strAllSimState);
        }
        return (strPhbReady.equals("true") && !isSimLocked);

    }

    public void setPhbReady(boolean isReady) {
        if (DBG) mtkLog("setPhbReady(): isReady = " + (isReady ? "true" : "false"));
        if (mPhbReady != isReady) {
            mPhbReady = isReady;
            if (isReady == true) {
                ((MtkRIL) mCi).setPhonebookReady(GSM_PHB_READY, null);
            } else if (isReady == false) {
                ((MtkRIL) mCi).setPhonebookReady(GSM_PHB_NOT_READY, null);
            }
            broadcastPhbStateChangedIntent(mPhbReady, false);
        }
    }

    private class PhbBroadCastReceiver extends BroadcastReceiver {
        public void onReceive(Context content, Intent intent) {
            String action = intent.getAction();
            if ((mPhbWaitSub == true) &&
                    (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED))) {
                mtkLog("SubBroadCastReceiver receive ACTION_SUBINFO_RECORD_UPDATED");
                mPhbWaitSub = false;
                broadcastPhbStateChangedIntent(mPhbReady, false);
            } else if (action.equals(TelephonyIntents.ACTION_RADIO_TECHNOLOGY_CHANGED)) {
                /** M: Bug Fix for ALPS02189616. */
                // add new code for svlte romaing case.
                // Inner private class for revice broad cast ACTION_RADIO_TECHNOLOGY_CHANGED.
                // listener radio technology changed. If it not own object phone
                // broadcast false.
                // if it own object, send delay message to broadcast PHB_CHANGE
                // event. APP will receive PHB_CHANGE broadcast and init phonebook.
                int phoneid = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                mtkLog("[ACTION_RADIO_TECHNOLOGY_CHANGED] phoneid : " + phoneid);
                if (null != mParentApp && mParentApp.getPhoneId() == phoneid) {
                    String cdmaPhoneName = "CDMA";
                    int delayedTime = 500;
                    String activePhoneName = intent.getStringExtra(PhoneConstants.PHONE_NAME_KEY);
                    int subid = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1);
                    mtkLog("[ACTION_RADIO_TECHNOLOGY_CHANGED] activePhoneName : " + activePhoneName
                            + " | subid : " + subid);
                    if (!cdmaPhoneName.equals(activePhoneName)) {
                        broadcastPhbStateChangedIntent(false, true);
                        sendMessageDelayed(obtainMessage(EVENT_DELAYED_SEND_PHB_CHANGE),
                                delayedTime);
                        mAdnCache.reset();
                    }
                }
            } else if (intent.getAction().equals(Intent.ACTION_BOOT_COMPLETED)) {
                mtkLog("[onReceive] ACTION_BOOT_COMPLETED mPendingPhbNotify : " +
                        mPendingPhbNotify);
                if (mPendingPhbNotify) {
                    broadcastPhbStateChangedIntent(isPhbReady(), false);
                    mPendingPhbNotify = false;
                }
            }
        }
    }

    // PHB Refactoring ++++
    private void updatePHBStatus(int status, boolean isSimLocked) {
        mtkLog("[PHBStatus] status : " + status + " | isSimLocked : " + isSimLocked
                + " | mPhbReady : " + mPhbReady);
        boolean simLockedState = false;
        // after Gen93, phb should not care about sim state
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            simLockedState = false;
        } else {
            simLockedState = isSimLocked;
        }
        // M: PHB Revise
        if (status == GSM_PHB_READY) {
            if (false == simLockedState) {
                if (mPhbReady == false) {
                    mPhbReady = true;
                    broadcastPhbStateChangedIntent(mPhbReady, false);
                }
            } else {
                mtkLog("phb ready but sim is not ready.");
            }
        } else if (status == GSM_PHB_NOT_READY) {
            if (mPhbReady == true) {
                mAdnCache.reset();
                mPhbReady = false;
                broadcastPhbStateChangedIntent(mPhbReady, false);
            }
        }
    }
    // PHB Refactoring ----
    // PHB END

    // MTK-START: SS
    @Override
    protected void setVoiceCallForwardingFlagFromSimRecords() {
        if (checkEfCfis()) {
            mCallForwardingStatus = (mEfCfis[1] & 0x01);
            mtkLog("EF_CFIS2: callForwardingEnabled=" + mCallForwardingStatus);
            mRecordsEventsRegistrants.notifyResult(EVENT_CFI);
        } else if (mEfCff != null) {
            mCallForwardingStatus =
                    ((mEfCff[0] & CFF_LINE1_MASK) == CFF_UNCONDITIONAL_ACTIVE) ?
                            CALL_FORWARDING_STATUS_ENABLED : CALL_FORWARDING_STATUS_DISABLED;
            mtkLog("EF_CFF2: callForwardingEnabled=" + mCallForwardingStatus);
            mRecordsEventsRegistrants.notifyResult(EVENT_CFI);
        } else {
            mCallForwardingStatus = CALL_FORWARDING_STATUS_UNKNOWN;
            mtkLog("EF_CFIS and EF_CFF not valid. callForwardingEnabled=" + mCallForwardingStatus);
            mRecordsEventsRegistrants.notifyResult(EVENT_CFI);
        }
    }

    private String convertNumberIfContainsPrefix(String dialNumber) {
        String r = dialNumber;
        if (dialNumber != null &&
            (dialNumber.startsWith("tel:") ||
             dialNumber.startsWith("sip:") ||
             dialNumber.startsWith("sips:"))) {
            r = dialNumber.substring(dialNumber.indexOf(":") + 1);
            Rlog.d(LOG_TAG, "convertNumberIfContainsPrefix: dialNumber = " + dialNumber);
        }
        return r;
    }
    // MTK-END

    public boolean isFetchRatBalancingAndEnsFile(int phoneId) {
        String strPropOperatorId = "persist.vendor.radio.sim.opid";
        boolean isFetch = false;

        if (phoneId > 0) {
            strPropOperatorId = strPropOperatorId + "_" + phoneId;
        }
        if (TextUtils.equals(SystemProperties.get(strPropOperatorId), ATT_OPID) ||
                TextUtils.equals(SystemProperties.get(strPropOperatorId), CRICKET_OPID)) {
            isFetch = true;
        }
        mtkLog("isFetchRatBalancingAndEnsFile is " + isFetch);
        return isFetch;
    }

    protected void mtkLog(String s) {
        Rlog.d(LOG_TAG_EX, "[SIMRecords] " + s + " (slot " + mSlotId + ")");
    }

    protected void mtkLoge(String s) {
        Rlog.e(LOG_TAG_EX, "[SIMRecords] " + s + " (slot " + mSlotId + ")");
    }

    @Override
    protected void log(String s) {
        Rlog.d(LOG_TAG, "[SIMRecords] " + s + " (slot " + mSlotId + ")");
    }

    @Override
    protected void loge(String s) {
        Rlog.e(LOG_TAG, "[SIMRecords] " + s + " (slot " + mSlotId + ")");
    }

    protected void logw(String s, Throwable tr) {
        Rlog.w(LOG_TAG, "[SIMRecords] " + s + " (slot " + mSlotId + ")", tr);
    }

    protected void logv(String s) {
        Rlog.v(LOG_TAG, "[SIMRecords] " + s + " (slot " + mSlotId + ")");
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
    protected void updateOperatorPlmn() {
        // In case of a test override, use the test IMSI
        String imsi = getIMSI();

        if (imsi != null) {
            // First try to guess the length based on a table of known 3-digit MNCs.
            if (((mMncLength == UNINITIALIZED) || (mMncLength == UNKNOWN) || (mMncLength == 2))
                    && imsi.length() >= 6) {
                String mccmncCode = imsi.substring(0, 6);
                for (String mccmnc : MCCMNC_CODES_HAVING_3DIGITS_MNC) {
                    if (mccmnc.equals(mccmncCode)) {
                        mMncLength = 3;
                        log("IMSI: setting1 mMncLength=" + mMncLength);
                        break;
                    }
                }
            }

            // If still unknown, guess using the MCC.
            if ((mMncLength == UNINITIALIZED) || (mMncLength == UNKNOWN)) {
                try {
                    int mcc = Integer.parseInt(imsi.substring(0, 3));
                    mMncLength = MccTable.smallestDigitsMccForMnc(mcc);
                    log("setting2 mMncLength=" + mMncLength);
                } catch (NumberFormatException e) {
                    loge("Corrupt IMSI! setting3 mMncLength=" + mMncLength);
                }
            }

            if (mMncLength != UNKNOWN && mMncLength != UNINITIALIZED
                    && imsi.length() >= 3 + mMncLength) {
                log("update mccmnc=" + imsi.substring(0, 3 + mMncLength));
                // finally have both the imsi and the mncLength and
                // can parse the imsi properly
                // MccTable.updateMccMncConfiguration(mContext, imsi.substring(0, 3 + mMncLength));
                updateConfiguration(imsi.substring(0, 3 + mMncLength));
            }
        }
    }

    /**
     * AOSP maintains a mccmnc list that has 3 digits mcc. For these 3 digits mccmnc that are not
     * in the list, it would conflict with actual usage. This API converts the actual 6 digits
     * mccmnc to 5 digits according to AOSP 3 digits mnc list.
     * @param mccmnc Mccmnc that to be converted.
     * @return converted mccmnc.
     */
    public static String convertMccmncAsAospConfig(String mccmnc) {
        String result = null;

        // Just convert mccmnc that length is 6.
        if ((mccmnc == null) || ((mccmnc != null) && (mccmnc.length() != 6))) {
            result = mccmnc;
        } else {
            // Check 3 gigits mnc list.
            for (String mm : MCCMNC_CODES_HAVING_3DIGITS_MNC) {
                if (mm.equals(mccmnc)) {
                    result = mccmnc;
                    break;
                }
            }

            // Convert to length 5 for the mccmnc that is not in list.
            if (result == null) {
                result = mccmnc.substring(0, 5);
            }
        }

        Rlog.d(LOG_TAG, "convertMccmncAsAospConfig: mccmnc:" + mccmnc + " result:" + result);
        return result;
    }
}
