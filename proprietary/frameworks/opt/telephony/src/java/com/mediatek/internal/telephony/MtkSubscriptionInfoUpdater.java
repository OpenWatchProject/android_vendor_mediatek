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

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.AsyncResult;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.provider.Settings.Global;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.Rlog;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.internal.telephony.CarrierAppUtils;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.IntentBroadcaster;
import com.android.internal.telephony.MccTable;
import com.android.internal.telephony.MultiSimSettingController;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.SubscriptionInfoUpdater;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.uicc.IccCardStatus.CardState;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.UiccSlot;

import com.mediatek.internal.telephony.MtkDefaultSmsSimSettings;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.uicc.MtkSIMRecords;
import com.mediatek.internal.telephony.uicc.MtkSpnOverride;
import com.mediatek.telephony.internal.telephony.vsim.ExternalSimManager;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 *@hide
 */
public class MtkSubscriptionInfoUpdater extends SubscriptionInfoUpdater {
    private static final boolean DBG = true;
    private static final String LOG_TAG = "MtkSubscriptionInfoUpdater";
    private static final boolean ENGDEBUG = TextUtils.equals(android.os.Build.TYPE, "eng");

    private static final int EVENT_RADIO_AVAILABLE = 101;
    private static final int EVENT_RADIO_UNAVAILABLE = 102;
    // For the feature SIM Hot Swap with Common Slot
    private static final int EVENT_SIM_NO_CHANGED = 103;
    private static final int EVENT_TRAY_PLUG_IN = 104;
    private static final int EVENT_SIM_PLUG_OUT = 105;
    // For the feature SIM ME LOCK - SML SBP
    private static final int EVENT_SIM_MOUNT_CHANGED = 106;

    private static final String ICCID_STRING_FOR_NO_SIM = "N/A";

    private final Object mLock = new Object();
    // SIM ME LOCK - Start
    private final Object mLockUpdateNew = new Object();
    private final Object mLockUpdateOld = new Object();
    // lock info: {detected type[slotId], sub count, SIM1 valid, SIM2 valid, SIM3 valid, SIM4 valid}
    private int[] newSmlInfo = {4, 0, -1, -1, -1, -1};
    private int[] oldSmlInfo = {4, 0, -1, -1, -1, -1};
    // detected type info: {dt[slot0], dt[slot1], dt[slot2], dt[slot3]}
    private int[] newSmlDt = {4, 4, 4, 4};
    private int[] oldSmlDt = {4, 4, 4, 4};
    private boolean mSimMountChangeState = false;
    private static MtkSubscriptionInfoUpdater sInstance = null;
    // The property shows SIM ME LOCK mode
    private static final String PROPERTY_SML_MODE = "ro.vendor.sim_me_lock_mode";
    private boolean mIsSmlLockMode = SystemProperties.get(PROPERTY_SML_MODE, "").equals("3");
    // SIM ME LOCK - End


    private CommandsInterface[] mCis = null;

    private int[] mIsUpdateAvailable = new int[PROJECT_SIM_NUM];
    private int mReadIccIdCount = 0;
    private int oldDensityDpi;

    // For the feature SIM Hot Swap with Common Slot
    private static final String COMMON_SLOT_PROPERTY = "ro.vendor.mtk_sim_hot_swap_common_slot";
    private boolean mCommonSlotResetDone = false;

    private static final boolean MTK_FLIGHTMODE_POWEROFF_MD_SUPPORT
            = "1".equals(SystemProperties.get("ro.vendor.mtk_flight_mode_power_off_md"));
    private static final int sReadICCID_retry_time = 1000;
    private static final String[] PROPERTY_ICCID_SIM = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };
    private static boolean sIsMultiSimSettingControllerInitialized = false;

    // A big rule, please add you code in the rear of API and file.
    public MtkSubscriptionInfoUpdater(Looper looper, Context context, Phone[] phone,
            CommandsInterface[] ci) {
        super(looper, context, phone, ci);
        logd("MtkSubscriptionInfoUpdater created");

        mCis = ci;
        oldDensityDpi = mContext.getResources().getConfiguration().densityDpi;
        // register MTK self receiver for funtion extension
        IntentFilter intentFilter = new IntentFilter(
                TelephonyIntents.ACTION_COMMON_SLOT_NO_CHANGED);
        if ("OP09".equals(SystemProperties.get("persist.vendor.operator.optr"))) {
            intentFilter.addAction(Intent.ACTION_LOCALE_CHANGED);
        }
        intentFilter.addAction(Intent.ACTION_CONFIGURATION_CHANGED);
        mContext.registerReceiver(mMtkReceiver, intentFilter);

        for (int i = 0; i < mCis.length; i++) {
            Integer index = new Integer(i);
            mCis[i].registerForNotAvailable(this, EVENT_RADIO_UNAVAILABLE, index);
            mCis[i].registerForAvailable(this, EVENT_RADIO_AVAILABLE, index);
            if (SystemProperties.get(COMMON_SLOT_PROPERTY).equals("1")) {
                ((MtkRIL)mCis[i]).registerForSimTrayPlugIn(this, EVENT_TRAY_PLUG_IN, index);
                ((MtkRIL)mCis[i]).registerForSimPlugOut(this, EVENT_SIM_PLUG_OUT, index);
            }
        }
    }

    @Override
    protected boolean isAllIccIdQueryDone() {
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            // MTK speeds SUB update and it may run as slot is not ready.
            // UiccSlot slot = UiccController.getInstance().getUiccSlotForPhone(i);
            // int slotId = UiccController.getInstance().getSlotIdFromPhoneId(i);
            if  (mIccId[i] == null || mIccId[i].equals("")) {
                // if (mIccId[i] == null) {
                    logd("Wait for SIM " + i + " Iccid");
                // } else {
                //    logd(String.format("Wait for slot corresponding to phone %d to be active, "
                //            + "slotId is %d", i, slotId));
                // }
                return false;
            }
        }
        logd("All IccIds query complete");

        return true;
    }

    @Override
    public void handleMessage(Message msg) {
        Integer index = getCiIndex(msg);

        switch (msg.what) {
            case EVENT_SIM_READY: {
                updateSubscriptionInfoIfNeed();

                // If the subinfo is created before card id is ready, it will be null.
                // So it needs to update card Id related information when it is available.
                // updateCardIdInfo();

                super.handleMessage(msg);
                break;
            }

            case EVENT_RADIO_UNAVAILABLE:
                logd("handleMessage : <EVENT_RADIO_UNAVAILABLE> SIM" + (index + 1));
                mIsUpdateAvailable[index] = 0;
                if (SystemProperties.get(COMMON_SLOT_PROPERTY).equals("1")) {
                    logd("[Common slot] reset mCommonSlotResetDone in EVENT_RADIO_UNAVAILABLE");
                    mCommonSlotResetDone = false;
                }
                break;

            case EVENT_RADIO_AVAILABLE:
                logd("handleMessage : <EVENT_RADIO_AVAILABLE> SIM" + (index + 1));
                mIsUpdateAvailable[index] = 1;

                if (checkIsAvailable()) {
                    mReadIccIdCount = 0;
                    if (!checkAllIccIdReady()) {
                        postDelayed(mReadIccIdPropertyRunnable, sReadICCID_retry_time);
                    } else {
                        updateSubscriptionInfoIfNeed();
                    }
                }
                break;

            case EVENT_SIM_NO_CHANGED: {
                if (checkAllIccIdReady()) {
                    updateSubscriptionInfoIfNeed();
                } else {
                    int slotId = msg.arg1;
                    mIccId[slotId] = ICCID_STRING_FOR_NO_SIM;
                    logd("case SIM_NO_CHANGED: set N/A for slot" + slotId);
                    mReadIccIdCount = 0;
                    postDelayed(mReadIccIdPropertyRunnable, sReadICCID_retry_time);
                }
                break;
            }

            case EVENT_TRAY_PLUG_IN: {
                logd("[Common Slot] handle EVENT_TRAY_PLUG_IN " + mCommonSlotResetDone);
                if (!mCommonSlotResetDone) {
                    mCommonSlotResetDone = true;
                    if (!ExternalSimManager.isAnyVsimEnabled()) {
                        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
                            mIccId[i] = "";
                        }

                        mReadIccIdCount = 0;
                        if (!checkAllIccIdReady()) {
                            postDelayed(mReadIccIdPropertyRunnable, sReadICCID_retry_time);
                        } else {
                            updateSubscriptionInfoIfNeed();
                        }
                    }
                }
                break;
            }
            case EVENT_SIM_PLUG_OUT: {
                logd("[Common Slot] handle EVENT_SIM_PLUG_OUT " + mCommonSlotResetDone);
                mCommonSlotResetDone = false;
                break;
            }

            case EVENT_SIM_MOUNT_CHANGED: {
                updateNewSmlInfo(newSmlDt[index], newSmlInfo[1], index);
                resetSimMountChangeState(index);
                break;
            }

            default:
                super.handleMessage(msg);
        }
    }

    @Override
    protected void handleSimLocked(int slotId, String reason) {
        if (mIccId[slotId] != null && mIccId[slotId].equals(ICCID_STRING_FOR_NO_SIM)) {
            logd("SIM" + (slotId + 1) + " hot plug in");
            mIccId[slotId] = null;
        }

        String iccId = mIccId[slotId];
        if (iccId == null) {
            IccCard iccCard = mPhone[slotId].getIccCard();
            if (iccCard == null) {
                logd("handleSimLocked: IccCard null");
                return;
            }
            IccRecords records = iccCard.getIccRecords();
            if (records == null) {
                logd("handleSimLocked: IccRecords null");
                return;
            }

            /* if (IccUtils.stripTrailingFs(records.getFullIccId()) == null) {
                logd("handleSimLocked: IccID null");
                return;
            }
            mIccId[slotId] = IccUtils.stripTrailingFs(records.getFullIccId()); */
        } else {
            logd("NOT Querying IccId its already set sIccid[" + slotId + "]="
                    + MtkSubscriptionInfo.givePrintableIccid(iccId));

            String tempIccid = SystemProperties.get(PROPERTY_ICCID_SIM[slotId], "");
            if (MTK_FLIGHTMODE_POWEROFF_MD_SUPPORT && !checkAllIccIdReady()
                    && (!tempIccid.equals(mIccId[slotId]))) {
                logd("All iccids are not ready and iccid changed");
                mIccId[slotId] = null;
                mSubscriptionManager.clearSubscriptionInfo();
            }
        }

        updateSubscriptionInfoIfNeed();

        // If the subinfo is created before card id is ready, it will be null.
        // So it needs to update card Id related information when it is available.
        // updateCardIdInfo();

        broadcastSimStateChanged(slotId, IccCardConstants.INTENT_VALUE_ICC_LOCKED, reason);
        broadcastSimCardStateChanged(slotId, TelephonyManager.SIM_STATE_PRESENT);
        broadcastSimApplicationStateChanged(slotId, getSimStateFromLockedReason(reason));
        updateSubscriptionCarrierId(slotId, IccCardConstants.INTENT_VALUE_ICC_LOCKED);
        updateCarrierServices(slotId, IccCardConstants.INTENT_VALUE_ICC_LOCKED);
    }

    @Override
    protected void handleSimLoaded(int slotId) {
        logd("handleSimLoaded: slotId: " + slotId);

        // The SIM should be loaded at this state, but it is possible in cases such as SIM being
        // removed or a refresh RESET that the IccRecords could be null. The right behavior is to
        // not broadcast the SIM loaded.
        int loadedSlotId = slotId;
        IccCard iccCard = mPhone[slotId].getIccCard();
        if (iccCard == null) {  // Possibly a race condition.
            logd("handleSimLoaded: IccCard null");
            return;
        }
        IccRecords records = iccCard.getIccRecords();
        if (records == null) {  // Possibly a race condition.
            logd("handleSimLoaded: IccRecords null");
            return;
        }
        if (IccUtils.stripTrailingFs(records.getFullIccId()) == null) {
            logd("handleSimLoaded: IccID null");
            return;
        }

        // Check iccid in updateSubscriptionInfoIfNeed().
        // mIccId[slotId] = IccUtils.stripTrailingFs(records.getFullIccId());

        updateSubscriptionInfoIfNeed();
        List<SubscriptionInfo> subscriptionInfos = MtkSubscriptionController.getMtkInstance()
                .getSubInfoUsingSlotIndexPrivileged(slotId);
        if (subscriptionInfos == null || subscriptionInfos.isEmpty()) {
            loge("empty subinfo for slotId: " + slotId + "could not update ContentResolver");
        } else {
            for (SubscriptionInfo sub : subscriptionInfos) {
                int subId = sub.getSubscriptionId();
                // Shoudn't use getDefault, it will suffer permission issue and can't get
                // operator and line1 number correctly.
                // TelephonyManager tm = TelephonyManager.getDefault();
                TelephonyManager tm = TelephonyManager.from(mContext);
                String operator = tm.getSimOperatorNumeric(subId);

                if (!TextUtils.isEmpty(operator)) {
                    if (subId == MtkSubscriptionController.getMtkInstance().getDefaultSubId()) {
                        MccTable.updateMccMncConfiguration(mContext, operator);
                    }
                    MtkSubscriptionController.getMtkInstance().setMccMnc(operator, subId);
                } else {
                    logd("EVENT_RECORDS_LOADED Operator name is null");
                }

                String iso = tm.getSimCountryIsoForPhone(slotId);

                if (!TextUtils.isEmpty(iso)) {
                    MtkSubscriptionController.getMtkInstance().setCountryIso(iso, subId);
                } else {
                    logd("EVENT_RECORDS_LOADED sim country iso is null");
                }

                String msisdn = tm.getLine1Number(subId);
                if (msisdn != null) {
                    MtkSubscriptionController.getMtkInstance().setDisplayNumber(msisdn, subId);
                }

                String imsi = tm.createForSubscriptionId(subId).getSubscriberId();
                if (imsi != null) {
                    MtkSubscriptionController.getMtkInstance().setImsi(imsi, subId);
                }

                String[] ehplmns = records.getEhplmns();
                String[] hplmns = records.getPlmnsFromHplmnActRecord();
                if (ehplmns != null || hplmns != null) {
                    MtkSubscriptionController.getMtkInstance().setAssociatedPlmns(ehplmns, hplmns,
                            subId);
                }

                SubscriptionInfo subInfo = mSubscriptionManager.getActiveSubscriptionInfo(subId);
                String nameToSet;
                String simCarrierName = tm.getSimOperatorName(subId);

                if (subInfo != null && subInfo.getNameSource() !=
                        SubscriptionManager.NAME_SOURCE_USER_INPUT) {
                    String simNumeric = tm.getSimOperatorNumeric(subId);
                    String simMvnoName = MtkSpnOverride.getInstance()
                            .lookupOperatorNameForDisplayName(subId, simNumeric, true, mContext);
                    logd("[handleSimLoaded]- simNumeric: " + simNumeric + ", simMvnoName: "
                            + simMvnoName + ", simCarrierName: " + simCarrierName);

                    if (!TextUtils.isEmpty(simMvnoName)) {
                        nameToSet = simMvnoName;
                    } else if (!TextUtils.isEmpty(simCarrierName)) {
                        nameToSet = simCarrierName;
                    } else {
                        nameToSet = "CARD " + Integer.toString(slotId + 1);
                    }
                    logd("sim name = " + nameToSet + ", nameSource = " + subInfo.getNameSource());
                    MtkSubscriptionController.getMtkInstance().setDisplayNameUsingSrc(nameToSet,
                            subId, subInfo.getNameSource());
                }

                /* Update preferred network type and network selection mode on SIM change.
                 * Storing last subId in SharedPreference for now to detect SIM change.
                 */
                SharedPreferences sp =
                        PreferenceManager.getDefaultSharedPreferences(mContext);
                int storedSubId = sp.getInt(CURR_SUBID + slotId, -1);

                if (storedSubId != subId) {
                    int networkType = Settings.Global.getInt(
                            mPhone[slotId].getContext().getContentResolver(),
                            Settings.Global.PREFERRED_NETWORK_MODE + subId,
                            -1 /* invalid network mode */);

                    if (networkType == -1) {
                        networkType = RILConstants.PREFERRED_NETWORK_MODE;
                        try {
                            networkType = TelephonyManager.getIntAtIndex(
                                    mContext.getContentResolver(),
                                    Settings.Global.PREFERRED_NETWORK_MODE, slotId);
                        } catch (SettingNotFoundException retrySnfe) {
                            Rlog.e(LOG_TAG, "Settings Exception Reading Value At Index for "
                                    + "Settings.Global.PREFERRED_NETWORK_MODE");
                        }
                        Settings.Global.putInt(
                                mPhone[slotId].getContext().getContentResolver(),
                                Global.PREFERRED_NETWORK_MODE + subId,
                                networkType);
                    }

                    // Set the modem network mode
                    mPhone[slotId].setPreferredNetworkType(networkType, null);

                    // Only support automatic selection mode on SIM change.
                    mPhone[slotId].getNetworkSelectionMode(
                            obtainMessage(EVENT_GET_NETWORK_SELECTION_MODE_DONE,
                                    new Integer(slotId)));

                    // Update stored subId
                    SharedPreferences.Editor editor = sp.edit();
                    editor.putInt(CURR_SUBID + slotId, subId);
                    editor.apply();
                }
            }
        }

        // Update set of enabled carrier apps now that the privilege rules may have changed.
        CarrierAppUtils.disableCarrierAppsUntilPrivileged(mContext.getOpPackageName(),
                mPackageManager, TelephonyManager.getDefault(),
                mContext.getContentResolver(), mCurrentlyActiveUserId);

        /**
         * The sim loading sequence will be
         *  1. ACTION_SUBINFO_CONTENT_CHANGE happens through updateSubscriptionInfoByIccId() above.
         *  2. ACTION_SIM_STATE_CHANGED/ACTION_SIM_CARD_STATE_CHANGED
         *  /ACTION_SIM_APPLICATION_STATE_CHANGED
         *  3. ACTION_SUBSCRIPTION_CARRIER_IDENTITY_CHANGED
         *  4. ACTION_CARRIER_CONFIG_CHANGED
         */
        broadcastSimStateChanged(loadedSlotId, IccCardConstants.INTENT_VALUE_ICC_LOADED, null);
        broadcastSimCardStateChanged(loadedSlotId, TelephonyManager.SIM_STATE_PRESENT);
        broadcastSimApplicationStateChanged(loadedSlotId, TelephonyManager.SIM_STATE_LOADED);
        updateSubscriptionCarrierId(loadedSlotId, IccCardConstants.INTENT_VALUE_ICC_LOADED);
        updateCarrierServices(loadedSlotId, IccCardConstants.INTENT_VALUE_ICC_LOADED);
    }

    @Override
    protected void handleSimAbsent(int slotId, int absentAndInactive) {
        if (mIccId[slotId] != null && !mIccId[slotId].equals(ICCID_STRING_FOR_NO_SIM)) {
            logd("SIM" + (slotId + 1) + " hot plug out, absentAndInactive=" + absentAndInactive);
        }

        // TODO: wait for louis peng's fix.
        /*
        // MTK-SS START
        IccRecords records = mPhone[slotId].getIccRecords();
        if (records == null) {
            logd("handleSimAbsent: IccRecords null");
        }

        boolean hasCFUFlagStatus = false;

        if (records != null && records instanceof MtkSIMRecords &&
                ((MtkSIMRecords) records).checkCFUFlagStatus()) {
            hasCFUFlagStatus = true;
            logd("handleSimAbsent: EFCFIS_STATUS_VALID");
        } else {
            hasCFUFlagStatus = false;
            logd("handleSimAbsent: EFCFIS_STATUS_INVALID");
        }

        if (mPhone[slotId] != null) {
            if (mPhone[slotId] instanceof MtkGsmCdmaPhone && hasCFUFlagStatus) {
                logd("SIM" + (slotId + 1) + " clear CFU status");
                ((MtkGsmCdmaPhone) mPhone[slotId]).cleanCallForwardingIndicatorFromSharedPref();
            }
        }
        // MTK-SS END
        */

        updateSubscriptionInfoIfNeed();

        // Do not broadcast if the SIM is absent and inactive, because the logical slotId here is
        // no longer correct
        if (absentAndInactive == 0) {
            broadcastSimStateChanged(slotId, IccCardConstants.INTENT_VALUE_ICC_ABSENT, null);
            broadcastSimCardStateChanged(slotId, TelephonyManager.SIM_STATE_ABSENT);
            broadcastSimApplicationStateChanged(slotId, TelephonyManager.SIM_STATE_UNKNOWN);
            updateSubscriptionCarrierId(slotId, IccCardConstants.INTENT_VALUE_ICC_ABSENT);
            updateCarrierServices(slotId, IccCardConstants.INTENT_VALUE_ICC_ABSENT);
        }
    }

    @Override
    protected void handleSimError(int slotId) {
        if (mIccId[slotId] != null && !mIccId[slotId].equals(ICCID_STRING_FOR_NO_SIM)) {
            logd("SIM" + (slotId + 1) + " Error ");
        }
        mIccId[slotId] = ICCID_STRING_FOR_NO_SIM;
        updateSubscriptionInfoByIccId(slotId, true);
        broadcastSimStateChanged(slotId, IccCardConstants.INTENT_VALUE_ICC_CARD_IO_ERROR,
                IccCardConstants.INTENT_VALUE_ICC_CARD_IO_ERROR);
        broadcastSimCardStateChanged(slotId, TelephonyManager.SIM_STATE_CARD_IO_ERROR);
        broadcastSimApplicationStateChanged(slotId, TelephonyManager.SIM_STATE_NOT_READY);
        updateSubscriptionCarrierId(slotId, IccCardConstants.INTENT_VALUE_ICC_CARD_IO_ERROR);
        updateCarrierServices(slotId, IccCardConstants.INTENT_VALUE_ICC_CARD_IO_ERROR);
    }

    @Override
    protected void handleSimNotReady(int slotId) {
        logd("handleSimNotReady: slotId: " + slotId);

        IccCard iccCard = mPhone[slotId].getIccCard();
        UiccSlot slot = UiccController.getInstance().getUiccSlotForPhone(slotId);
        if (slot != null && slot.isEuicc() && iccCard.isEmptyProfile()) {
            // ICC_NOT_READY is a terminal state for an eSIM on the boot profile. At this
            // phase, the subscription list is accessible. Treating NOT_READY
            // as equivalent to ABSENT, once the rest of the system can handle it.
            mIccId[slotId] = ICCID_STRING_FOR_NO_SIM;
            updateSubscriptionInfoByIccId(slotId, false);
        } else {
            updateSubscriptionInfoIfNeed();
        }

        // If the subinfo is created before card id is ready, it will be null.
        // So it needs to update card Id related information when it is available.
        // updateCardIdInfo();

        broadcastSimStateChanged(slotId, IccCardConstants.INTENT_VALUE_ICC_NOT_READY,
                null);
        broadcastSimCardStateChanged(slotId, TelephonyManager.SIM_STATE_PRESENT);
        broadcastSimApplicationStateChanged(slotId, TelephonyManager.SIM_STATE_NOT_READY);
    }

    @Override
    synchronized protected void updateSubscriptionInfoByIccId(int slotIndex,
            boolean updateEmbeddedSubs) {
        logd("updateSubscriptionInfoByIccId:+ Start");
        if (!SubscriptionManager.isValidSlotIndex(slotIndex)) {
            loge("[updateSubscriptionInfoByIccId]- invalid slotIndex=" + slotIndex);
            return;
        }

        // Reset the flag because all sIccId are ready.
        mCommonSlotResetDone = false;

        // Record old subinfo to set detect type.
        String oldIccId = null;
        String initIccid = MtkSubscriptionController.getMtkInstance().initIccid[slotIndex];
        if (!TextUtils.isEmpty(initIccid)) {
            oldIccId = initIccid;
            MtkSubscriptionController.getMtkInstance().initIccid[slotIndex] = "";
        } else {
            List<SubscriptionInfo> oldSubInfo = MtkSubscriptionController.getMtkInstance()
                    .getSubInfoUsingSlotIndexPrivileged(slotIndex);
            if (oldSubInfo != null) {
                oldIccId = oldSubInfo.get(0).getIccId();
            } else {
                oldIccId = ICCID_STRING_FOR_NO_SIM;
            }
        }
        logd("updateSubscriptionInfoByIccId: removing subscription info record: slotIndex:"
                + slotIndex + " initIccid:" + MtkSubscriptionInfo.givePrintableIccid(initIccid)
                + " oldIccId:" + MtkSubscriptionInfo.givePrintableIccid(oldIccId));

        // Clear slotIndex only when sim absent is not enough. It's possible to switch SIM profile
        // within the same slot. Need to clear the slot index of the previous sub. Thus always clear
        // for the changing slot first.
        MtkSubscriptionController.getMtkInstance().clearSubInfoRecord(slotIndex);

        // If SIM is not absent, insert new record or update existing record.
        if (!ICCID_STRING_FOR_NO_SIM.equals(mIccId[slotIndex])) {
           logd("updateSubscriptionInfoByIccId: adding subscription info record: iccid: "
                    + MtkSubscriptionInfo.givePrintableIccid(mIccId[slotIndex])
                    + "  slot: " + slotIndex);
           mSubscriptionManager.addSubscriptionInfoRecord(mIccId[slotIndex], slotIndex);
        }

        List<SubscriptionInfo> subInfos = MtkSubscriptionController.getMtkInstance()
                .getSubInfoUsingSlotIndexPrivileged(slotIndex);
        if (subInfos != null) {
            boolean changed = false;
            for (int i = 0; i < subInfos.size(); i++) {
                SubscriptionInfo temp = subInfos.get(i);
                ContentValues value = new ContentValues(1);

                // Shoudn't use getDefault, it will suffer permission issue and can't get
                // line1 number correctly.
                String msisdn = TelephonyManager.from(mContext).getLine1Number(
                        temp.getSubscriptionId());

                if (!TextUtils.equals(msisdn, temp.getNumber())) {
                    value.put(SubscriptionManager.NUMBER, msisdn);
                    mContext.getContentResolver().update(SubscriptionManager
                            .getUriForSubscriptionId(temp.getSubscriptionId()), value, null, null);
                    changed = true;
                }
            }
            if (changed) {
                // refresh Cached Active Subscription Info List
                MtkSubscriptionController.getMtkInstance()
                        .refreshCachedActiveSubscriptionInfoList();
            }
        }

        List<SubscriptionInfo> subInfoList = MtkSubscriptionController.getMtkInstance()
                .getActiveSubscriptionInfoList(mContext.getOpPackageName());

        // TODO investigate if we can update for each slot separately.
        if (isAllIccIdQueryDone()) {
            MtkDefaultSmsSimSettings.setSmsTalkDefaultSim(subInfoList, mContext);

            // External SIM [Start]
            if (SystemProperties.getInt("ro.vendor.mtk_external_sim_support", 0) == 1 &&
                    SystemProperties.getInt("ro.vendor.mtk_non_dsda_rsim_support", 0) == 1) {

                int rsimPhoneId = SystemProperties.getInt(
                        MtkTelephonyProperties.PROPERTY_PREFERED_REMOTE_SIM, -1);
                int rsimSubId[] = MtkSubscriptionController.getMtkInstance().getSubId(rsimPhoneId);
                if (rsimPhoneId >= 0 && rsimPhoneId < PROJECT_SIM_NUM
                        && rsimSubId != null && rsimSubId.length != 0) {
                    MtkSubscriptionController.getMtkInstance().setDefaultDataSubId(
                            rsimSubId[0]);
                }
            }
            // External SIM [End]

            // Comment as JE happens when card plugs out and Uicccard is null. It could be udpated
            // in ready, locked and not ready state.
            // updateCardIdInfo();

            // Ensure the modems are mapped correctly
            if (mSubscriptionManager.isActiveSubId(
                    mSubscriptionManager.getDefaultDataSubscriptionId())) {
                MtkSubscriptionController.getMtkInstance()
                        .setDefaultDataSubIdWithoutCapabilitySwitch(
                        mSubscriptionManager.getDefaultDataSubscriptionId());
            } else {
                logd("bypass reset default data sub if inactive");
            }
            setSubInfoInitialized();
        }

        // Generate detect type.
        int detectedType = -1;
        String decIccId = IccUtils.getDecimalSubstring(mIccId[slotIndex]);

        if (mIccId[slotIndex] != null && mIccId[slotIndex].equals(ICCID_STRING_FOR_NO_SIM)
                && !oldIccId.equals(ICCID_STRING_FOR_NO_SIM)) {
            detectedType = MtkSubscriptionManager.EXTRA_VALUE_REMOVE_SIM;
        } else if (isNewSim(mIccId[slotIndex], decIccId, oldIccId)) {
            detectedType = MtkSubscriptionManager.EXTRA_VALUE_NEW_SIM;
        } else {
            detectedType = MtkSubscriptionManager.EXTRA_VALUE_NOCHANGE;
        }

        int subCount = (subInfoList == null) ? 0 : subInfoList.size();

        // SIM ME LOCK - Start
        if (mIsSmlLockMode) {
            updateNewSmlInfo(detectedType, subCount, slotIndex);
            resetSimMountChangeState(slotIndex);
        }
        // SIM ME LOCK - End

        Intent intent = new Intent(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        MtkSubscriptionController.getMtkInstance().putSubinfoRecordUpdatedExtra(intent, slotIndex,
                detectedType, subCount, null);

        UiccController uiccController = UiccController.getInstance();
        UiccSlot[] uiccSlots = uiccController.getUiccSlots();
        if (uiccSlots != null && updateEmbeddedSubs) {
            List<Integer> cardIds = new ArrayList<>();
            for (UiccSlot uiccSlot : uiccSlots) {
                if (uiccSlot != null && uiccSlot.getUiccCard() != null) {
                    try {
                        int cardId = uiccController.convertToPublicCardId(
                                uiccSlot.getUiccCard().getCardId());
                        cardIds.add(cardId);
                    } catch (NullPointerException e) {
                        logd("updateSubscriptionInfoByIccId uiccSlots.getUiccCard() is null.");
                    }
                }
            }
            updateEmbeddedSubscriptions(cardIds, (hasChanges) -> {
                if (hasChanges) {
                    MtkSubscriptionController.getMtkInstance()
                            .notifySubscriptionInfoChanged();
                }
                if (DBG) logd("updateSubscriptionInfoByIccId: SubscriptionInfo update complete");
            });

        }

        MtkSubscriptionController.getMtkInstance().notifySubscriptionInfoChanged(intent);
        logd("updateSubscriptionInfoByIccId: SubscriptionInfo update complete: slotIndex"
                + slotIndex + " detectedType = " + detectedType + " subCount = "
                + subCount);

        if (isAllIccIdQueryDone()) {
            if (!sIsMultiSimSettingControllerInitialized) {
                sIsMultiSimSettingControllerInitialized = true;
                MultiSimSettingController.getInstance().notifyAllSubscriptionLoaded();
            }
        }
    }

    protected static void setSubInfoInitialized() {
        // Should only be triggered once.
        if (!sIsSubInfoInitialized) {
            sIsSubInfoInitialized = true;
            SubscriptionController.getInstance().notifySubInfoReady();
            // Telecomm needs to create account firstly when notifySubscriptionInfoChanged
            // is received then setDefaultVoiceSubId could success. So invoke it
            // after notifySubscriptionInfoChanged is sent.
            // MultiSimSettingController.getInstance().notifyAllSubscriptionLoaded();
        }
    }

    private boolean isNewSim(String iccId, String decIccId, String oldIccId) {
        boolean newSim = true;
        if ((iccId != null) && (oldIccId != null) && ((oldIccId.indexOf(iccId) == 0)
                || (oldIccId.indexOf(iccId.toLowerCase()) == 0))) {
            newSim = false;
        } else if (decIccId != null && decIccId.equals(oldIccId)) {
            newSim = false;
        }
        logd("isNewSim newSim = " + newSim);

        return newSim;
    }

    public void dispose() {
        logd("[dispose]");
        mContext.unregisterReceiver(mMtkReceiver);
    }

    private boolean checkAllIccIdReady() {
        String iccId = "";
        logd("checkAllIccIdReady +, retry_count = " + mReadIccIdCount);
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            iccId = SystemProperties.get(PROPERTY_ICCID_SIM[i], "");
            if (iccId.length() == 3) {
                logd("No SIM insert :" + i);
            }
            if (iccId.equals("")) {
                return false;
            }
            logd("iccId[" + i + "] = " + MtkSubscriptionInfo.givePrintableIccid(iccId));
        }

        return true;
    }

    private void updateSubscriptionInfoIfNeed() {
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            boolean needUpdate = false;
            logd("[updateSubscriptionInfoIfNeed] before update mIccId[" + i + "]: "
                    + MtkSubscriptionInfo.givePrintableIccid(mIccId[i]));
            if (mIccId[i] == null ||
                    !mIccId[i].equals(SystemProperties.get(PROPERTY_ICCID_SIM[i], ""))) {
                mIccId[i] = SystemProperties.get(PROPERTY_ICCID_SIM[i], "");
                needUpdate = true;
                logd("[updateSubscriptionInfoIfNeed] mIccId[" + i + "]: "
                        + MtkSubscriptionInfo.givePrintableIccid(mIccId[i])
                        + " needUpdate: " + needUpdate + "  !TextUtils.isEmpty(mIccId[" + i
                        + "]): " + (!TextUtils.isEmpty(mIccId[i])));
            }

            // if (isAllIccIdQueryDone() && needUpdate) {
            if (needUpdate && (!TextUtils.isEmpty(mIccId[i]))) {
                updateSubscriptionInfoByIccId(i, true);
            }
        }
    }

    private Integer getCiIndex(Message msg) {
        AsyncResult ar;
        Integer index = new Integer(PhoneConstants.DEFAULT_CARD_INDEX);

        /*
         * The events can be come in two ways. By explicitly sending it using
         * sendMessage, in this case the user object passed is msg.obj and from
         * the CommandsInterface, in this case the user object is msg.obj.userObj
         */
        if (msg != null) {
            if (msg.obj != null && msg.obj instanceof Integer) {
                index = (Integer) msg.obj;
            } else if (msg.obj != null && msg.obj instanceof AsyncResult) {
                ar = (AsyncResult) msg.obj;
                if (ar.userObj != null && ar.userObj instanceof Integer) {
                    index = (Integer) ar.userObj;
                }
            }
        }
        return index;
    }

    private boolean checkIsAvailable() {
        boolean result = true;
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            if (mIsUpdateAvailable[i] <= 0) {
                logd("mIsUpdateAvailable[" + i + "] = " + mIsUpdateAvailable[i]);
                result = false;
                break;
            }
        }
        logd("checkIsAvailable result = " + result);
        return result;
    }

    private final BroadcastReceiver mMtkReceiver = new  BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logd("onReceive, Action: " + action);

            if (action.equals(Intent.ACTION_LOCALE_CHANGED)) {
                int[] subIdList = mSubscriptionManager.getActiveSubscriptionIdList();
                for (int subId : subIdList) {
                    updateSubName(subId);
                }
            } else if (action.equals(TelephonyIntents.ACTION_COMMON_SLOT_NO_CHANGED)) {
                int slotIndex = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                        SubscriptionManager.INVALID_SIM_SLOT_INDEX);
                logd("[Common Slot] NO_CHANTED, slotId: " + slotIndex);
                sendMessage(obtainMessage(EVENT_SIM_NO_CHANGED, slotIndex, -1));
            } else if (action.equals(Intent.ACTION_CONFIGURATION_CHANGED)) {
                if (oldDensityDpi != mContext.getResources().getConfiguration().densityDpi) {
                    oldDensityDpi = mContext.getResources().getConfiguration().densityDpi;
                    // Refresh the Cache of Active Subscription Info List
                    MtkSubscriptionController.getMtkInstance()
                            .refreshCachedActiveSubscriptionInfoList();
                }
            }
        }
    };

    private Runnable mReadIccIdPropertyRunnable = new Runnable() {
        public void run() {
            ++mReadIccIdCount;
            if (mReadIccIdCount <= 10) {
                if (!checkAllIccIdReady()) {
                    postDelayed(mReadIccIdPropertyRunnable, sReadICCID_retry_time);
                } else {
                    updateSubscriptionInfoIfNeed();
                }
            }
        }
    };

    private void updateSubName(int subId) {
        SubscriptionInfo subInfo =
                MtkSubscriptionManager.getSubInfo(null, subId);
        if (subInfo != null
                && subInfo.getNameSource() != SubscriptionManager.NAME_SOURCE_USER_INPUT) {
            MtkSpnOverride spnOverride = MtkSpnOverride.getInstance();
            String nameToSet;
            String carrierName = TelephonyManager.getDefault().getSimOperator(subId);
            int slotId = SubscriptionManager.getSlotIndex(subId);
            logd("updateSubName, carrierName = " + carrierName + ", subId = " + subId);
            if (SubscriptionManager.isValidSlotIndex(slotId)) {
                if (spnOverride.containsCarrierEx(carrierName)) {
                    nameToSet = spnOverride.lookupOperatorName(subId, carrierName,
                        true, mContext);
                    logd("SPN found, name = " + nameToSet);
                } else {
                    nameToSet = "CARD " + Integer.toString(slotId + 1);
                    logd("SPN not found, set name to " + nameToSet);
                }

                MtkSubscriptionController.getMtkInstance().setDisplayNameUsingSrc(nameToSet,
                        subId, SubscriptionManager.NAME_SOURCE_DEFAULT_SOURCE);
            }
        }
    }

    public void updateCardIdInfo() {
        int numPhysicalSlots = mContext.getResources().getInteger(
                com.android.internal.R.integer.config_num_physical_slots);
        // Minimum number of physical slot count should be equals to or greater than phone count,
        // if it is less than phone count use phone count as physical slot count.
        if (numPhysicalSlots < PROJECT_SIM_NUM) {
            numPhysicalSlots = PROJECT_SIM_NUM;
        }

        UiccController uiccController = UiccController.getInstance();
        UiccSlot[] uiccSlots = uiccController.getUiccSlots();

        if (uiccSlots != null) {
            for (int i = 0; i < numPhysicalSlots; i++) {
                if ((uiccSlots[i] != null) && (uiccSlots[i].getUiccCard() != null)
                        && (uiccSlots[i].getCardState() != CardState.CARDSTATE_ABSENT)) {
                    List<SubscriptionInfo> subInfos = MtkSubscriptionController.getMtkInstance()
                            .getSubInfoUsingSlotIndexPrivileged(i);
                    if ((subInfos != null) && !(subInfos.isEmpty())) {
                        try {
                            List<Integer> cardIds = new ArrayList<>();
                            int cardId = uiccController.convertToPublicCardId(uiccSlots[i]
                                    .getUiccCard().getCardId());
                            cardIds.add(cardId);

                            for (int j = 0; j < subInfos.size(); j++) {
                                SubscriptionInfo temp = subInfos.get(j);
                                if ((temp != null) && (temp.getCardId() != cardId)) {
                                    // Update Sub card id.
                                    ContentValues value = new ContentValues(1);
                                    value.put(SubscriptionManager.CARD_ID, cardId);
                                    mContext.getContentResolver().update(SubscriptionManager
                                            .getUriForSubscriptionId(temp.getSubscriptionId()),
                                            value, null, null);

                                    // refresh Cached Active Subscription Info List
                                    MtkSubscriptionController.getMtkInstance()
                                            .refreshCachedActiveSubscriptionInfoList();

                                    // Update embedded subscriptions.
                                    updateEmbeddedSubscriptions(cardIds, null /* callback */);
                                }
                            }
                        } catch (NullPointerException e) {
                            logd("updateCardIdInfo uiccSlots[" + i + "].getUiccCard() is null.");
                        }
                    } else {
                        if (ENGDEBUG) {
                            logd("updateCardIdInfo slot " + i + "subInfos is null.");
                        }
                    }
                } else {
                    if (ENGDEBUG) {
                        logd("updateCardIdInfo uiccSlots[" + i + "] or uiccSlots[" + i
                                + "].getUiccCard() is null.");
                    }
                }
            }
        }
    }

    // SIM ME LOCK - Start
    public void triggerUpdateInternalSimMountState(int slotId) {
        logd("triggerUpdateInternalSimMountState slotId " + slotId);
        sendMessage(obtainMessage(EVENT_SIM_MOUNT_CHANGED, slotId));
    }

    private void resetSimMountChangeState(int slotId) {
        boolean needReport = false;
        newSmlInfo[0] = newSmlDt[slotId];
        oldSmlInfo[0] = oldSmlDt[slotId];
        for (int i = 0; i < 6; i++) {
            if (newSmlInfo[i] != oldSmlInfo[i]) {
                needReport = true;
                break;
            }
        }
        if (needReport) {
            int newDetectedType = newSmlInfo[0];
            int newSimCount = newSmlInfo[1];
            int newValid1 = newSmlInfo[2];
            int newValid2 = newSmlInfo[3];
            int newValid3 = newSmlInfo[4];
            int newValid4 = newSmlInfo[5];
            Intent intent = new Intent(TelephonyIntents.ACTION_SIM_SLOT_SIM_MOUNT_CHANGE);
            intent.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_DETECTED_TYPE, newDetectedType);
            intent.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM_COUNT, newSimCount);
            intent.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM1_VALID, newValid1);
            intent.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM2_VALID, newValid2);
            intent.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM3_VALID, newValid3);
            intent.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM4_VALID, newValid4);
            intent.putExtra(PhoneConstants.PHONE_KEY, slotId);
            intent.putExtra(PhoneConstants.SLOT_KEY, slotId);
            //TODO: remove log after IT done
            logd("Broadcasting ACTION_SIM_SLOT_SIM_MOUNT_CHANGE,  [" + newDetectedType + ", "
                    + newSimCount + ", " + newValid1 + ", " + newValid2 + ", " + newValid3
                    + ", " + newValid4 + ", " + newSmlDt[slotId] + ", " + slotId + "]");
            mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
            updateOldSmlInfo(newDetectedType, newSimCount, newValid1, newValid2, newValid3,
                    newValid4, slotId);
         } else {
             logd("resetSimMountChangeState no  need report ");
         }
     }

    public void updateNewSmlInfo(int detectedType, int simCount, int slotId) {
        synchronized (mLockUpdateNew) {
            newSmlDt[slotId] = detectedType;
            newSmlInfo[0] = detectedType;
            newSmlInfo[1] = simCount;
            newSmlInfo[2] = MtkTelephonyManagerEx.getDefault().checkValidCard(0);
            newSmlInfo[3] = MtkTelephonyManagerEx.getDefault().checkValidCard(1);
            newSmlInfo[4] = MtkTelephonyManagerEx.getDefault().checkValidCard(2);
            newSmlInfo[5] = MtkTelephonyManagerEx.getDefault().checkValidCard(3);
            logd("[updateNewSmlInfo]- [" + newSmlInfo[0] + ", " + newSmlInfo[1] + ", "
                    + newSmlInfo[2] + ", " + newSmlInfo[3] + ", " + newSmlInfo[4] + ", "
                    + newSmlInfo[5] + ", " + newSmlDt[slotId] + ", " + slotId + "]");
        }
   }

    public void updateOldSmlInfo(int detectedType, int simCount, int valid1, int valid2,
            int valid3, int valid4, int slotId) {
        synchronized (mLockUpdateOld) {
            oldSmlDt[slotId] = detectedType;
            oldSmlInfo[0] = detectedType;
            oldSmlInfo[1] = simCount;
            oldSmlInfo[2] = valid1;
            oldSmlInfo[3] = valid2;
            oldSmlInfo[4] = valid3;
            oldSmlInfo[5] = valid4;
        }
   }
   // SIM ME LOCK - End

    private void logd(String message) {
        Rlog.d(LOG_TAG, message);
    }

    private void loge(String message) {
        Rlog.e(LOG_TAG, message);
    }
}
