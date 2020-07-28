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

import android.content.Context;
import android.os.AsyncResult;
import android.os.SystemProperties;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.UserHandle;
import android.telephony.Rlog;
import com.android.internal.telephony.uicc.UiccController;
import com.android.internal.telephony.uicc.IccCardStatus;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.CommandsInterface;
import android.app.ActivityManagerNative;
import static android.Manifest.permission.READ_PHONE_STATE;
import android.os.UserHandle;
import android.provider.Settings;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import android.content.SharedPreferences;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.content.Intent;

import android.app.PendingIntent;
import android.app.Notification;
import android.app.NotificationManager;

import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppState;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.PersoSubState;
import com.android.internal.telephony.uicc.IccRefreshResponse;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.SubscriptionController;
import android.os.Handler;
import android.os.Bundle;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.Message;
import android.os.storage.StorageManager;
import android.telephony.TelephonyManager;

import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkIccCardConstants;
// External SIM [START]
import com.mediatek.internal.telephony.MtkTelephonyProperties;
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.RadioManager;
import com.mediatek.telephony.internal.telephony.vsim.ExternalSimManager;
// External SIM [END]
import com.mediatek.internal.telephony.MtkSubscriptionController;
import com.mediatek.internal.telephony.MtkSubscriptionInfoUpdater;
import com.mediatek.internal.telephony.OpTelephonyCustomizationUtils;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.Arrays;
import java.util.List;

public class MtkUiccController extends UiccController {
    private static final String LOG_TAG_EX = "MtkUiccCtrl";
    protected static final int EVENT_BASE_ID = 100;
    protected static final int EVENT_VIRTUAL_SIM_ON = EVENT_BASE_ID + 1;
    protected static final int EVENT_VIRTUAL_SIM_OFF = EVENT_BASE_ID + 2;
    // MTK-START: SIM MISSING/RECOVERY
    protected static final int EVENT_SIM_MISSING = EVENT_BASE_ID + 3;
    protected static final int EVENT_SIM_RECOVERY = EVENT_BASE_ID + 4;
    protected static final int EVENT_GET_ICC_STATUS_DONE_FOR_SIM_MISSING = EVENT_BASE_ID + 5;
    protected static final int EVENT_GET_ICC_STATUS_DONE_FOR_SIM_RECOVERY = EVENT_BASE_ID + 6;
    // MTK-END

    // MTK-START: SIM HOT SWAP
    protected static final int EVENT_SIM_PLUG_OUT = EVENT_BASE_ID + 7;
    protected static final int EVENT_SIM_PLUG_IN = EVENT_BASE_ID + 8;
    // MTK-END

    protected static final int EVENT_INVALID_SIM_DETECTED = EVENT_BASE_ID + 9;
    // MTK-START: CMCC DUAL SIM DEPENDENCY LOCK
    protected static final int EVENT_REPOLL_SML_STATE = EVENT_BASE_ID + 10;
    // MTK-END
    // MTK-START: SIM COMMON SLOT
    protected static final int EVENT_COMMON_SLOT_NO_CHANGED = EVENT_BASE_ID + 11;
    // MTK-END
    // SIM ME LOCK - Start
    protected static final int EVENT_SML_SLOT_LOCK_INFO_CHANGED = EVENT_BASE_ID + 12;
    protected static final int EVENT_SUPPLY_DEVICE_LOCK_DONE = EVENT_BASE_ID + 13;
    // SIM ME LOCK - End
    // SIM POWER
    protected static final int EVENT_SIM_POWER_CHANGED = EVENT_BASE_ID + 14;
    protected static final int EVENT_CARD_DETECTED_IND = EVENT_BASE_ID + 15;
    // MTK-START: CMCC DUAL SIM DEPENDENCY LOCK
    private static final int SML_FEATURE_NO_NEED_BROADCAST_INTENT = 0;
    private static final int SML_FEATURE_NEED_BROADCAST_INTENT = 1;
    // MTK-END

    // MTK START: CRYPTKEEPER
    protected static final String DECRYPT_STATE = "trigger_restart_framework";
    // MTK-END

    // MTK-START: VZW RSU
    private IMtkRsuSml mMtkRsuSml = null;
    // MTK-END

    // MTK START: SIM on/off
    private static final String PROPERTY_SIM_CARD_ONOFF = "ro.vendor.mtk_sim_card_onoff";
    private static final String PROPERTY_SIM_ONOFF_SUPPORT = "vendor.ril.sim.onoff.support";
    private static final String[] PROPERTY_SIM_ONOFF_STATE = {
        "vendor.ril.sim.onoff.state1",
        "vendor.ril.sim.onoff.state2",
        "vendor.ril.sim.onoff.state3",
        "vendor.ril.sim.onoff.state4",
    };
    // MTK-END

    private int[] UICCCONTROLLER_STRING_NOTIFICATION_VIRTUAL_SIM_ON = {
        com.mediatek.internal.R.string.virtual_sim_on_slot1,
        com.mediatek.internal.R.string.virtual_sim_on_slot2,
        com.mediatek.internal.R.string.virtual_sim_on_slot3,
        com.mediatek.internal.R.string.virtual_sim_on_slot4
    };
    // MTK-START: SIM COMMON SLOT
    protected static final String COMMON_SLOT_PROPERTY = "ro.vendor.mtk_sim_hot_swap_common_slot";
    // MTK-END

    // Sim power state
    private int[] mSimPower;
    private int[] mSimPowerExecutingState;

    public MtkUiccController(Context c, CommandsInterface []ci) {
        super(c, ci);
        if (DBG) Rlog.d(LOG_TAG_EX, "Creating MtkUiccController");

        for (int i = 0; i < mCis.length; i++) {
            Integer index = new Integer(i);
            // MTK-START
            mCis[i].unregisterForAvailable(this);
            mCis[i].unregisterForOn(this);
            if (SystemProperties.get("ro.crypto.state").equals("unencrypted")
                    || SystemProperties.get("ro.crypto.state").equals("unsupported")
                    || SystemProperties.get("ro.crypto.type").equals("file")
                    || DECRYPT_STATE.equals(SystemProperties.get("vold.decrypt"))
                    || !StorageManager.inCryptKeeperBounce()) {
            //if  (!StorageManager.inCryptKeeperBounce()) {
                mCis[i].registerForAvailable(this, EVENT_RADIO_AVAILABLE, index);
            } else {
                mCis[i].registerForOn(this, EVENT_RADIO_ON, index);
            }
            // MTK-END

            //TODO: HIDL extention impl.
            ((MtkRIL)mCis[i]).registerForVirtualSimOn(this, EVENT_VIRTUAL_SIM_ON, index);
            ((MtkRIL)mCis[i]).registerForVirtualSimOff(this, EVENT_VIRTUAL_SIM_OFF, index);

            // MTK-START: SIM MISSING/RECOVERY
            ((MtkRIL)mCis[i]).registerForSimMissing(this, EVENT_SIM_MISSING, index);
            ((MtkRIL)mCis[i]).registerForSimRecovery(this, EVENT_SIM_RECOVERY, index);
            // MTK-END
            // MTK-START: SIM HOT SWAP
            ((MtkRIL)mCis[i]).registerForSimPlugOut(this, EVENT_SIM_PLUG_OUT, index);
            ((MtkRIL)mCis[i]).registerForSimPlugIn(this, EVENT_SIM_PLUG_IN, index);
            // MTK-END
            // MTK-START: SIM COMMON SLOT
            ((MtkRIL)mCis[i]).registerForCommonSlotNoChanged(this, EVENT_COMMON_SLOT_NO_CHANGED,
                    index);
            // SIM ME LOCK
            ((MtkRIL)mCis[i]).registerForSmlSlotLockInfoChanged(this,
                    EVENT_SML_SLOT_LOCK_INFO_CHANGED, index);
            // MTK-END
            // SIM POWER
            ((MtkRIL) mCis[i]).registerForSimPower(this, EVENT_SIM_POWER_CHANGED, index);
            ((MtkRIL) mCis[i]).registerForCardDetectedInd(this, EVENT_CARD_DETECTED_IND, index);
        }

        // External SIM [Start]
        if (SystemProperties.getInt("ro.vendor.mtk_external_sim_support", 0) == 1) {
            ExternalSimManager.make(c, ci);
        }
        // Exteranl SIM [End]

        mMdStateReceiver = new ModemStateChangedReceiver();
        IntentFilter filter = new IntentFilter();
        filter.addAction(RadioManager.ACTION_MODEM_POWER_NO_CHANGE);
        mContext.registerReceiver(mMdStateReceiver, filter);
        // Sim power state
        mSimPower = new int[mCis.length];
        mSimPowerExecutingState = new int[mCis.length];
        Arrays.fill(mSimPower, -1);
        Arrays.fill(mSimPowerExecutingState, -1);

        try {
            mMtkRsuSml = OpTelephonyCustomizationUtils.getRsuSmlFactory(mContext)
                    .makeRsuSml(mContext, mCis);
            Rlog.d(LOG_TAG_EX, "[RSU-SIMLOCK] Create RsuSml, mCis=" + mCis);
        } catch (Exception e) {
            Rlog.e(LOG_TAG_EX, "[RSU-SIMLOCK] e = " + e);
        }
    }

    // MTK-START: Exposed RSU Framework instance with APIs
    /**
     * Get remote sim unlock instatnce.
     *
     * @return remote sim unlock instatnce
     *
     */
    public IMtkRsuSml getRsuSml() {
        if (mMtkRsuSml == null) {
            Rlog.e(LOG_TAG_EX, "getRsuSml : [RSU-SIMLOCK] Sml not supported");
        }
        return mMtkRsuSml;
    }

    public IMtkRsuSml getRsuSml(String operator) {
        mMtkRsuSml = OpTelephonyCustomizationUtils.getRsuSmlFactory(mContext)
                .checkAndMakeRsuSml(mMtkRsuSml, mContext, mCis, operator);

        if (mMtkRsuSml == null) {
            Rlog.e(LOG_TAG_EX, "getRsuSml: [RSU-SIMLOCK] " + operator + "Sml not supported");
        }
        return mMtkRsuSml;
    }
    // MTK-END

    // Easy to use API
    // MTK-START
    public UiccCardApplication getUiccCardApplication(int family) {
        return getUiccCardApplication(SubscriptionController.getInstance().getPhoneId(
                SubscriptionController.getInstance().getDefaultSubId()), family);
    }
    // MTK-END

    // MTK-START, TO be removed
    public int getIccApplicationChannel(int slotId, int family) {
        return 0;
    }
    // MTK-END

    @Override
    public void handleMessage (Message msg) {
        synchronized (mLock) {
            Integer index = getCiIndex(msg);

            if (index < 0 || index >= mCis.length) {
                Rlog.e(LOG_TAG_EX, "Invalid index : " + index + " received with event " + msg.what);
                return;
            }

            // The msg.obj may be Integer if it is from ModemStateChangedReceiver
            AsyncResult ar = null;
            if(msg.obj != null && msg.obj instanceof AsyncResult) {
                ar = (AsyncResult)msg.obj;
            }

            switch (msg.what) {
                case EVENT_ICC_STATUS_CHANGED:
                    // MTK-START
                    if (DBG) {
                        mtkLog("Received EVENT_ICC_STATUS_CHANGED, calling getIccCardStatus,index: "
                            + index);
                    }

                    //mCis[index].getIccCardStatus(obtainMessage(EVENT_GET_ICC_STATUS_DONE, index));
                    if (ignoreGetSimStatus()) {
                        if (DBG) mtkLog("FlightMode ON, Modem OFF: ignore get sim status");
                    } else {
                    // MTK-END
                        mCis[index].getIccCardStatus(obtainMessage(
                                EVENT_GET_ICC_STATUS_DONE, index));
                    // MTK-START
                    }
                    // MTK-END
                    break;
                case EVENT_RADIO_AVAILABLE:
                case EVENT_RADIO_ON:
                    if (ignoreGetSimStatus()) {
                        if (DBG) {
                            mtkLog("FlightMode ON, Modem OFF: ignore get sim status, index: "
                                    + index);
                        }
                    } else {
                        super.handleMessage(msg);
                    }
                    break;
                // MTK-START: CMCC DUAL SIM DEPENDENCY LOCK
                case EVENT_REPOLL_SML_STATE:
                    if (DBG) mtkLog("Received EVENT_REPOLL_SML_STATE");
                    ar = (AsyncResult) msg.obj;
                    boolean needIntent =
                            ((msg.arg1 == SML_FEATURE_NEED_BROADCAST_INTENT) ? true : false);

                    //Update Uicc Card status.
                    onGetIccCardStatusDone(ar, index);

                    // If we still in Network lock, broadcast intent if caller need this intent.
                    if (needIntent == true) {
                        UiccCardApplication app = getUiccCardApplication(index, APP_FAM_3GPP);
                        if (app == null) {
                            if (DBG) mtkLog("UiccCardApplication = null");
                            break;
                        }
                        if (app.getState() == AppState.APPSTATE_SUBSCRIPTION_PERSO) {
                            Intent lockIntent = new Intent();
                            if (DBG) mtkLog("Broadcast ACTION_UNLOCK_SIM_LOCK");
                            lockIntent.setAction(TelephonyIntents.ACTION_UNLOCK_SIM_LOCK);
                            lockIntent.putExtra(IccCardConstants.INTENT_KEY_ICC_STATE,
                                    IccCardConstants.INTENT_VALUE_ICC_LOCKED);
                            lockIntent.putExtra(IccCardConstants.INTENT_KEY_LOCKED_REASON,
                                    parsePersoType(app.getPersoSubState()));
                            SubscriptionManager.putPhoneIdAndSubIdExtra(lockIntent, index);
                            mContext.sendBroadcast(lockIntent);
                        }
                    }
                    break;
                // MTK-END
                case EVENT_VIRTUAL_SIM_ON:
                    if (DBG) {
                        mtkLog("handleMessage (EVENT_VIRTUAL_SIM_ON)");
                    }
                    setNotificationVirtual(index, EVENT_VIRTUAL_SIM_ON);
                    SharedPreferences shOn = mContext.getSharedPreferences("AutoAnswer",
                            Context.MODE_PRIVATE);
                    SharedPreferences.Editor editorOn = shOn.edit();
                    editorOn.putBoolean("flag", true);
                    editorOn.commit();
                    break;
               case EVENT_VIRTUAL_SIM_OFF:
                    if (DBG) mtkLog("handleMessage (EVENT_VIRTUAL_SIM_OFF)");
                    removeNotificationVirtual(index, EVENT_VIRTUAL_SIM_ON);
                    //setNotification(index, EVENT_SIM_MISSING);
                    SharedPreferences shOff = mContext.getSharedPreferences("AutoAnswer",
                            Context.MODE_PRIVATE);
                    SharedPreferences.Editor editorOff = shOff.edit();
                    editorOff.putBoolean("flag", false);
                    editorOff.commit();
                    break;
                // MTK-START: SIM MISSING/RECOVERY
                case EVENT_SIM_RECOVERY:
                    if (DBG) mtkLog("handleMessage (EVENT_SIM_RECOVERY)");
                    mCis[index].getIccCardStatus(
                            obtainMessage(EVENT_GET_ICC_STATUS_DONE_FOR_SIM_RECOVERY, index));
                    //ALPS01209124
                    Intent intent = new Intent();
                    intent.setAction(TelephonyIntents.ACTION_SIM_RECOVERY_DONE);
                    mContext.sendBroadcast(intent);
                    break;
                case EVENT_SIM_MISSING:
                    if (DBG) mtkLog("handleMessage (EVENT_SIM_MISSING)");
                    //setNotification(index, EVENT_SIM_MISSING);
                    mCis[index].getIccCardStatus(
                            obtainMessage(EVENT_GET_ICC_STATUS_DONE_FOR_SIM_MISSING, index));
                    break;
                case EVENT_GET_ICC_STATUS_DONE_FOR_SIM_MISSING:
                    if (DBG) mtkLog("Received EVENT_GET_ICC_STATUS_DONE_FOR_SIM_MISSING");
                    ar = (AsyncResult) msg.obj;
                    onGetIccCardStatusDone(ar, index);
                    break;
                case EVENT_GET_ICC_STATUS_DONE_FOR_SIM_RECOVERY:
                    if (DBG) mtkLog("Received EVENT_GET_ICC_STATUS_DONE_FOR_SIM_RECOVERY");
                    ar = (AsyncResult) msg.obj;
                    onGetIccCardStatusDone(ar, index);
                    break;
                // MTK-END
                // MTK-START: SIM COMMON SLOT
                case EVENT_COMMON_SLOT_NO_CHANGED:
                    if (DBG) mtkLog("handleMessage (EVENT_COMMON_SLOT_NO_CHANGED)");
                    Intent intentNoChanged = new Intent(
                            TelephonyIntents.ACTION_COMMON_SLOT_NO_CHANGED);
                    int slotId = index.intValue();
                    SubscriptionManager.putPhoneIdAndSubIdExtra(intentNoChanged, slotId);
                    mtkLog("Broadcasting intent ACTION_COMMON_SLOT_NO_CHANGED for mSlotId : " +
                            slotId);
                    mContext.sendBroadcast(intentNoChanged);
                    break;
                // MTK-END
                // SIM ME LOCK - Start
                case EVENT_SML_SLOT_LOCK_INFO_CHANGED:
                    if (DBG) mtkLog("handleMessage (EVENT_SML_SLOT_LOCK_INFO_CHANGED)");
                    ar = (AsyncResult) msg.obj;
                    onSmlSlotLoclInfoChaned(ar, index);
                    triggerUpdateInternalSimMountState(index);
                    break;
                case EVENT_SUPPLY_DEVICE_LOCK_DONE:
                    if (DBG) mtkLog("handleMessage (EVENT_SUPPLY_DEVICE_LOCK_DONE)");
                    int attemptsRemaining = -1;
                    ar = (AsyncResult)msg.obj;
                    if (ar.result != null) {
                        attemptsRemaining = parseUnlockDeviceResult(ar);
                    }
                    Message response = (Message)ar.userObj;
                    AsyncResult.forMessage(response).exception = ar.exception;
                    response.arg1 = attemptsRemaining;
                    response.sendToTarget();
                    break;
                    // SIM ME LOCK - End
                case EVENT_SIM_POWER_CHANGED:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null || ar.result == null) {
                       Rlog.e(LOG_TAG_EX, "EVENT_SIM_POWER_CHANGED exception");
                       return;
                    }
                    int state[];
                    state = (int[]) ar.result;
                    if (state.length == 1) {
                        mSimPower[index] = state[0];
                        if (state[0] == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_OFF
                                || state[0] == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON) {
                            mSimPowerExecutingState[index] = -1;
                        }
                    }

                    if (DBG) {
                        mtkLog("Received EVENT_SIM_POWER_CHANGED, index: " + index + " simPower: "
                                + mSimPower[index] + " mSimPowerExecutingState = "
                                + mSimPowerExecutingState[index]);
                    }
                    break;
                case EVENT_CARD_DETECTED_IND:
                    Intent cardDetectedInd = new Intent(TelephonyIntents.ACTION_CARD_DETECTED);
                    slotId = index.intValue();
                    SubscriptionManager.putPhoneIdAndSubIdExtra(cardDetectedInd, slotId);

                    if (DBG) {
                        mtkLog("Broadcasting intent ACTION_CARD_DETECTED, mSlotId : " + slotId);
                    }
                    mContext.sendBroadcast(cardDetectedInd);
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    }

    // SIM ME LOCK - Start
    private void onSmlSlotLoclInfoChaned(AsyncResult ar, Integer index) {
        if (ar.exception != null || ar.result == null) {
           Rlog.e(LOG_TAG_EX, "onSmlSlotLoclInfoChaned exception");
           return;
        }
        int info[];
        info = (int[]) ar.result;
        if (info.length != 4) {
            Rlog.e(LOG_TAG_EX, "onSmlSlotLoclInfoChaned exception");
            return;
        }

        mtkLog("onSmlSlotLoclInfoChaned, infomation:"
                + ",lock policy:" + info[0]
                + ",lock state:" + info[1]
                + ",service capability:" + info[2]
                + ",sim valid:" + info[3]);

       Intent smlLockInfoChanged = new Intent(
               TelephonyIntents.ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION);
       int slotId = index.intValue();
       SubscriptionManager.putPhoneIdAndSubIdExtra(smlLockInfoChanged, slotId);
       smlLockInfoChanged.putExtra(PhoneConstants.SLOT_KEY, slotId);
       smlLockInfoChanged.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_DEVICE_LOCK_POLICY,
               info[0]);
       smlLockInfoChanged.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_DEVICE_LOCK_STATE,
               info[1]);
       smlLockInfoChanged.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM_SERVICE_CAPABILITY,
               info[2]);
       smlLockInfoChanged.putExtra(MtkIccCardConstants.INTENT_KEY_SML_SLOT_SIM_VALID,
               info[3]);
       mtkLog("Broadcasting intent ACTION_SIM_SLOT_LOCK_POLICY_INFORMATION for mSlotId : " +
               slotId);
       mContext.sendBroadcastAsUser(smlLockInfoChanged, UserHandle.ALL);
    }

    private void triggerUpdateInternalSimMountState(int phoneId) {
        MtkSubscriptionInfoUpdater subInfoUpdator =
                (MtkSubscriptionInfoUpdater)(PhoneFactory.getSubscriptionInfoUpdater());
        if (subInfoUpdator != null) {
            subInfoUpdator.triggerUpdateInternalSimMountState(phoneId);
        } else {
            mtkLog("subInfoUpdate is null.");
        }
    }

    public void supplyDeviceNetworkDepersonalization(String pwd, Message onComplete) {
        ((MtkRIL)mCis[0]).supplyDeviceNetworkDepersonalization(pwd,
                obtainMessage(EVENT_SUPPLY_DEVICE_LOCK_DONE, onComplete));
     }

    private int parseUnlockDeviceResult(AsyncResult ar) {
        int[] result = (int[]) ar.result;
        if (result == null) {
            return -1;
        } else {
            int length = result.length;
            int attemptsRemaining = -1;
            if (length > 0) {
                attemptsRemaining = result[0];
            }
            mtkLog("parseUnlockDeviceResult: attemptsRemaining=" + attemptsRemaining);
            return attemptsRemaining;
        }
    }

    // SIM ME LOCK - End

    private void setNotificationVirtual(int slot, int notifyType) {
        if (DBG) mtkLog("setNotificationVirtual(): notifyType = " + notifyType);
        Notification notification = new Notification();
        notification.when = System.currentTimeMillis();
        notification.flags = Notification.FLAG_AUTO_CANCEL;
        notification.icon = com.android.internal.R.drawable.stat_sys_warning;
        Intent intent = new Intent();
        notification.contentIntent = PendingIntent.getActivity(mContext, 0, intent,
                PendingIntent.FLAG_UPDATE_CURRENT);
        String title = null;

        if (TelephonyManager.getDefault().getSimCount() > 1) {
            title = Resources.getSystem().getText(
                    UICCCONTROLLER_STRING_NOTIFICATION_VIRTUAL_SIM_ON[slot]).toString();
        } else {
            title = Resources.getSystem().getText(
                    com.mediatek.internal.R.string.virtual_sim_on).toString();
        }
        CharSequence detail = mContext.getText(
                com.mediatek.internal.R.string.virtual_sim_on).toString();
        notification.tickerText = mContext.getText(
                com.mediatek.internal.R.string.virtual_sim_on).toString();

        notification.setLatestEventInfo(mContext, title, detail, notification.contentIntent);
        NotificationManager notificationManager = (NotificationManager) mContext.getSystemService(
                Context.NOTIFICATION_SERVICE);
        notificationManager.notify(notifyType + slot, notification);
    }

    private void removeNotificationVirtual(int slot, int notifyType) {
        NotificationManager notificationManager = (NotificationManager) mContext.getSystemService(
                Context.NOTIFICATION_SERVICE);
        notificationManager.cancel(notifyType + slot);
    }

    // MTK-START: CMCC DUAL SIM DEPENDENCY LOCK
    /**
     * Parse network lock reason string.
     *
     * @param state network lock type
     * @return network lock string
     *
     */
    private String parsePersoType(PersoSubState state) {
        if (DBG) mtkLog("parsePersoType, state = " + state);
        switch (state) {
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
            default:
                break;
        }
        return IccCardConstants.INTENT_VALUE_ICC_UNKNOWN;
    }

    //Modem SML change feature.
    public void repollIccStateForModemSmlChangeFeatrue(int slotId, boolean needIntent) {
        if (DBG) mtkLog("repollIccStateForModemSmlChangeFeatrue, needIntent = " + needIntent);
        int arg1 = ((needIntent == true) ?
                SML_FEATURE_NEED_BROADCAST_INTENT : SML_FEATURE_NO_NEED_BROADCAST_INTENT);
        //Use arg1 to determine the intent is needed or not
        //Use object to indicated slotId
        mCis[slotId].getIccCardStatus(obtainMessage(EVENT_REPOLL_SML_STATE, arg1, 0, slotId));
    }
    // MTK-END

    // Check if modem will be power off quickly after boot up device
    public boolean ignoreGetSimStatus() {
        int airplaneMode = Settings.Global.getInt(mContext.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0);

        if (DBG) mtkLog("ignoreGetSimStatus(): airplaneMode - " + airplaneMode);
        if (RadioManager.isFlightModePowerOffModemEnabled() && (airplaneMode == 1)) {
            if (DBG) mtkLog("ignoreGetSimStatus(): return true");
            return true;
        }

        return false;
    }

     protected void mtkLog(String string) {
        Rlog.d(LOG_TAG_EX, string);
    }

    // External SIM [START]
    public boolean isAllRadioAvailable() {
        boolean isRadioReady = true;
        for (int i = 0; i < TelephonyManager.getDefault().getPhoneCount(); i++) {
            if (TelephonyManager.RADIO_POWER_UNAVAILABLE == mCis[i].getRadioState()) {
                isRadioReady = false;
            }
        }

        mtkLog("isAllRadioAvailable = " + isRadioReady);

        return isRadioReady;
    }

    public void resetRadioForVsim() {
        mtkLog("resetRadioForVsim...resetRadio");
        int mainPhoneId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        ((MtkRIL)mCis[mainPhoneId]).restartRILD(null);
    }

    /**
     *  Check if the SIM card is VSIM card.
     *  @param slot id.
     */
    public static MtkIccCardConstants.VsimType getVsimCardType(int slotId) {
        int rSim = SystemProperties.getInt(MtkTelephonyProperties.PROPERTY_PREFERED_REMOTE_SIM, -1);
        int akaSim = SystemProperties.getInt(MtkTelephonyProperties.PROPERTY_PREFERED_AKA_SIM, -1);
        boolean isVsim = false;

        String inserted = TelephonyManager.getDefault().getTelephonyProperty(
                slotId , MtkTelephonyProperties.PROPERTY_EXTERNAL_SIM_INSERTED, "0");
        if (inserted != null && inserted.length() > 0 && !"0".equals(inserted)) {
            isVsim = true;
        }

        if ((slotId == rSim) && isVsim) {
            return MtkIccCardConstants.VsimType.REMOTE_SIM;
        }

        if (slotId == akaSim) {
            if (isVsim == true) {
                return MtkIccCardConstants.VsimType.SOFT_AKA_SIM;
            } else {
                return MtkIccCardConstants.VsimType.PHYSICAL_AKA_SIM;
            }
        }

        if (rSim == -1 && akaSim == -1 && isVsim) {
            return MtkIccCardConstants.VsimType.LOCAL_SIM;
        }

        return MtkIccCardConstants.VsimType.PHYSICAL_SIM;
    }
    // External SIM [END]

    /**
     * Some feature will turn on modem when flight mode is on and the modem is off
     * so we have to listen the intent sent by RadioManager to trigger GET_SIM_STATUS.
     */
    private BroadcastReceiver mMdStateReceiver;

    private class ModemStateChangedReceiver extends BroadcastReceiver {
        public void onReceive(Context content, Intent intent) {
            String action = intent.getAction();
            if (action.equals(RadioManager.ACTION_MODEM_POWER_NO_CHANGE)) {
                for (int i = 0; i < mCis.length; i++) {
                    Integer index = new Integer(i);
                    Message msg = obtainMessage(EVENT_ICC_STATUS_CHANGED, index);
                    sendMessage(msg);
                    mtkLog("Trigger GET_SIM_STATUS due to modem state changed for slot " + i);
                }
            }
        }
    }

    /**
     * Set SIM power state.
     *
     * @param slotId SIM slot id
     * @param state 11: SIM ON; 10: SIMOFF
     * @param onComplete Callback function.
     **/
    public void setSimPower(int slotId, int state, Message onComplete) {
        ((MtkRIL) mCis[slotId]).setSimPower(state, onComplete);
    }

    /**
     * Get SIM on/off state.
     * @param slotId SIM slot id
     * @return -1: unknown, SIM_POWER_STATE_SIM_OFF: SIM off, SIM_POWER_STATE_SIM_ON: SIM on.
     */
    public int getSimOnOffState(int slotId) {
        if (slotId < 0 || slotId >= mSimPower.length) {
            mtkLog("getSimOnOffState: invalid slotId " + slotId);
            return -1;
        }

        boolean onoffAPSupport = SystemProperties.get(PROPERTY_SIM_CARD_ONOFF).equals("2");
        boolean onoffMDSupport = SystemProperties.get(PROPERTY_SIM_ONOFF_SUPPORT).equals("1");
        int onoffState = SystemProperties.getInt(PROPERTY_SIM_ONOFF_STATE[slotId], -1);
        mtkLog("getSimOnOffState slotId = " + slotId + " onoffAPSupport = " + onoffAPSupport
                 + " onoffMDSupport = " + onoffMDSupport + " mSimPower = " + mSimPower[slotId]
                 + " onoffState = " + onoffState);

        if (onoffAPSupport && onoffMDSupport) {
            if (mSimPower[slotId] == -1) {
                // URC not reached now, in fact, a risk is that modem still reports currently.
                return onoffState;
            } else {
                return mSimPower[slotId];
            }
        } else {
            return MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON;
        }
    }

    /**
     * Set SIM on/off executing state.
     * @param slotId SIM slot id
     * @param state -1: not in executing, SIM_POWER_STATE_EXECUTING_SIM_ON: in process of on state,
     * SIM_POWER_STATE_EXECUTING_SIM_OFF: in process of off state
     */
    public void setSimOnOffExecutingState(int slotId, int state) {
        mSimPowerExecutingState[slotId] = state;
    }

    /**
     * Check whether SIM is in process of on/off state.
     * @param slotId SIM slot id
     * @return -1: not in executing, SIM_POWER_STATE_EXECUTING_SIM_ON: in process of on state,
     * SIM_POWER_STATE_EXECUTING_SIM_OFF: in process of off state
     */
    public int getSimOnOffExecutingState(int slotId) {
        if (slotId < 0 || slotId >= mSimPowerExecutingState.length) {
            mtkLog("getSimOnOffExecutingState: invalid slotId " + slotId);
            return -1;
        }
        mtkLog("getSimOnOffExecutingState slotId = " + slotId +
                " mSimPowerExecutingState = " + mSimPowerExecutingState[slotId]);
        return mSimPowerExecutingState[slotId];
    }
}
