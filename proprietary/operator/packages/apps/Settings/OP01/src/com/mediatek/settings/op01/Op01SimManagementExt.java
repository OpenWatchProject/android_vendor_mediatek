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

package com.mediatek.settings.op01;

import android.app.StatusBarManager;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.database.ContentObserver;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.drawable.Drawable;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import androidx.preference.Preference;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.BackgroundColorSpan;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import com.mediatek.internal.telephony.MtkDefaultSmsSimSettings;
import com.mediatek.internal.telephony.MtkSubscriptionManager;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.ratconfiguration.RatConfiguration;

import com.mediatek.settings.ext.DefaultSimManagementExt;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import com.mediatek.widget.AccountViewAdapter.AccountElements;
import com.mediatek.provider.MtkSettingsExt;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;

public class Op01SimManagementExt extends DefaultSimManagementExt {

    private static final String TAG = "OP01SimManagementExt";

    private Context mContext;
    private IntentFilter mIntentFilter;

    private static final String SIM_COLOR = "sim_color";
    private static final String KEY_CELLULAR_DATA = "sim_cellular_data";
    private static final String KEY_CALLS = "sim_calls";
    private static final String KEY_SMS = "sim_sms";
    private static final String USIM = "USIM";
    private static final String LTE_SUPPORT = "1";
    private static final String VOLTE_SUPPORT = "1";
    private static final String RCS_SUPPORT = "1";
    private static final int VOLTE_DIALOG = 0x01;
    private static final int RCS_DIALOG = 0x02;
    private static final String[] MCCMNC_TABLE_TYPE_CMCC = {
        "46000", "46002", "46004", "46007", "46008"};
    private final static String ONE = "1";
    private final static String TWO = "2";
    private static final int DATA_PICK = 0;
    private static final int CALLS_PICK = 1;
    private static final int SMS_PICK = 2;
    private static final String PACKAGE_NAME = "com.mediatek.settings.op01";
    private static HashMap<Integer, OpInfo> sOpInfo = new HashMap<Integer, OpInfo>();
    private final static int SLOT_NUM = 2;
    private static boolean mIsShutDown = false;

    // Subinfo record change listener.
    private BroadcastReceiver mSubReceiver;

    /**
     * update the preference screen of sim management
     * @param parent parent preference
     */
    public Op01SimManagementExt(Context context) {
        super();
        mContext = context;
        Log.d("@M_" + TAG, "mContext = " + mContext);
    }

    @Override
    public void onCreate() {
        Log.d("@M_" + TAG, "onCreate");
        mIntentFilter = new IntentFilter(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        mIntentFilter.addAction(Intent.ACTION_SHUTDOWN);
        createSubReceiver();
        mContext.registerReceiver(mSubReceiver, mIntentFilter);
        updateOperatorInfo();
    }


    private void createSubReceiver() {
        mSubReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                Log.d("@M_" + TAG, "create mSubReceiver action = " + action);
                if (action.equals(TelephonyIntents.ACTION_SIM_STATE_CHANGED)) {
                    String cardState =
                        intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                    if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(cardState)) {
                        updateOperatorInfo();
                    }
                }
                if (Intent.ACTION_SHUTDOWN.equals(action)) {
                    mIsShutDown = true;
                    onDestroy();
                }
            }
        };
    }

    @Override
    public void onDestroy() {
        Log.d("@M_" + TAG, "onDestroy unregisterObserverd");
        if (null != mSubReceiver) {
            mContext.unregisterReceiver(mSubReceiver);
            mSubReceiver = null;
        }
    }

    private void updateOperatorInfo() {
        List<SubscriptionInfo> subs = SubscriptionManager.from(mContext)
                .getActiveSubscriptionInfoList();
        if (subs != null && subs.size() == SLOT_NUM) {
            for (SubscriptionInfo sub : subs) {
                int slotId = sub.getSimSlotIndex();
                int subId = sub.getSubscriptionId();
                String plmn = getSimOperator(subId);
                Log.d(TAG, "updateOPInfo slot: " + slotId + " sub: " + subId + " OP: " + plmn);
                if (plmn != null && !plmn.equals("")) {
                    OpInfo info = new OpInfo(subId, plmn);
                    sOpInfo.put(slotId, info);
                }
            }
        }
        printOpInfo();
    }
    /**
     * Store sub id and plmn.
     */
    private class OpInfo {
        int mSub;
        String mPlmn;
        OpInfo(int sub, String plmn) {
            mSub = sub;
            mPlmn = plmn;
        }
    }
    private void printOpInfo() {
        for (int i = 0; i < SLOT_NUM; i++) {
            OpInfo info = sOpInfo.get(i);
            if (info != null) {
                Log.d(TAG, "printOpInfo, sub: " + info.mSub + " plmn: " + info.mPlmn);
            }
        }
    }

    /**
     * Get whether airplane mode is in on.
     * @param context Context.
     * @return True for on.
     */
    private static boolean isAirplaneModeOn(Context context) {
        return Settings.System.getInt(context.getContentResolver(),
            Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

    private int getSubIdBySlot(int slotId) {
        if (slotId < 0 || slotId > 1) {
            return -1;
        }
        int[] subids = SubscriptionManager.getSubId(slotId);
        int subid = -1;
        if (subids != null && subids.length >= 1) {
            subid = subids[0];
        }
        Log.d("@M_" + TAG, "GetSimIdBySlot: sub id = " + subid
                + " sim Slot = " + slotId);
        return subid;
    }

    private void switchSmsDefaultSim(int subid) {
        Log.d("@M_" + TAG, "switchSmsDefaultSim() with subid=" + subid);
        if (subid < 0) {
            return;
        }
        SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        subscriptionManager.setDefaultSmsSubId(subid);
    }

    private void switchVoiceCallDefaultSim(int subid) {
        Log.d("@M_" + TAG, "switchVoiceCallDefaultSim() with subid=" + subid);
        if(subid < 0) {
            return;
        }
        TelecomManager telecomMgr = TelecomManager.from(mContext);
        List<PhoneAccountHandle> allHandles = telecomMgr.getAllPhoneAccountHandles();
        for (PhoneAccountHandle handle : allHandles) {
            PhoneAccount phoneAccount = telecomMgr.getPhoneAccount(handle);
            TelephonyManager tm = TelephonyManager.from(mContext);
            int curSubId = tm.getSubIdForPhoneAccount(phoneAccount);
            Log.d("@M_" + TAG, "switchVoiceCallDefaultSim() subId =" + curSubId);
            if (curSubId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                continue;
            } else if (curSubId == subid) {
                Log.d("@M_" + TAG, "switch voice call to subid=" + subid);
                telecomMgr.setUserSelectedOutgoingPhoneAccount(handle);
                break;
            }
        }
        Log.d("@M_" + TAG, "switchVoiceCallDefaultSim() ==>end");
    }

    private boolean switchDefaultDataSubId(Context context, int subId) {
        boolean isShow = false;
        int showType = 0;

        if (isShowVoLTEDialog(subId)) {
            isShow = true;
            showType |= VOLTE_DIALOG;
        }
        if (isShowRcsDialog(subId)) {
            isShow = true;
            showType |= RCS_DIALOG;
        }

        if (isShow) {
            Intent start = new Intent(mContext, DataSwitchDialog.class);
            start.putExtra("subId", subId);
            start.putExtra("showType", showType);
            context.startActivity(start);
            Log.d("@M_" + TAG, "switchDefaultDataSub(), showType" + showType);
            return true;
        } else {
            Log.d("@M_" + TAG, "switchDefaultDataSub(), false");
            return false;
        }
    }

    /**
     * app use to judge the Card is CMCC
     * @param slotId
     * @return true is CMCC
     */
    private boolean isCMCCCard(int subId) {
        String simOperator = null;
        simOperator = getSimOperator(subId);
        if (simOperator != null) {
            Log.d("@M_" + TAG, "isCMCCCard, simOperator = " + simOperator + " subId = " + subId);
            for (String mccmnc : MCCMNC_TABLE_TYPE_CMCC) {
                if (simOperator.equals(mccmnc)) {
                    return true;
                }
            }
        }

        if (simOperator == null || simOperator.equals("")) {
            Collection<OpInfo> c = sOpInfo.values();
            String plmn = null;
            for (OpInfo i : c) {
                if (i.mSub == subId) {
                    plmn = i.mPlmn;
                    break;
                }
            }
            Log.d(TAG, "isCMCCCard??, plmn = " + plmn + " subId = " + subId);
            for (String mccmnc : MCCMNC_TABLE_TYPE_CMCC) {
                if (mccmnc.equals(plmn)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
      * @Get simConfig by TelephonyManager.getDefault().getMultiSimConfiguration()
      * @return true if the device has 2 or more slots
      */
    private boolean isGeminiSupport() {
        TelephonyManager.MultiSimVariants mSimConfig = TelephonyManager
                .getDefault().getMultiSimConfiguration();
        if (mSimConfig == TelephonyManager.MultiSimVariants.DSDS ||
                mSimConfig == TelephonyManager.MultiSimVariants.DSDA) {
            return true;
        }
        return false;
    }

    /**
     * Gets the MCC+MNC (mobile country code + mobile network code)
     * of the provider of the SIM. 5 or 6 decimal digits.
     * Availability: The result of calling getSimState()
     * must be android.telephony.TelephonyManager.SIM_STATE_READY.
     * @param slotId  Indicates which SIM to query.
     * @return MCC+MNC (mobile country code + mobile network code)
     * of the provider of the SIM. 5 or 6 decimal digits.
     */
    private String getSimOperator(int subId) {
        if (subId < 0) {
            return null;
        }
        String simOperator = null;
        int status = TelephonyManager.SIM_STATE_UNKNOWN;
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
             status = TelephonyManager.getDefault().getSimState(slotId);
        }
        if (status == TelephonyManager.SIM_STATE_READY) {
            simOperator = TelephonyManager.getDefault().getSimOperator(subId);
        }
        Log.d("@M_" + TAG, "getSimOperator, simOperator = " + simOperator + " subId = " + subId);
        return simOperator;
    }

    /**
     * app use to judge LTE open.
     * @return true is LTE open
     */
    private boolean isLTESupport() {
        return RatConfiguration.isLteFddSupported()
        		|| RatConfiguration.isLteTddSupported();
    }

    /**
     * app use to judge LTE open.
     * @return true is LTE open
     */
    private boolean isVoLTESupport() {
        boolean isSupport = VOLTE_SUPPORT.equals(
                SystemProperties.get("persist.vendor.volte_support")) ? true : false;
        return isSupport;
    }

    /**
     * app use to judge RCS open.
     * @return true is RCS open
     */
    private boolean isRcsSupport() {
        boolean isSupport = RCS_SUPPORT.equals(
                SystemProperties.get("ro.vendor.mtk_op01_rcs")) ? true : false;
        return isSupport;
    }

    private String getSIMType(int subId) {
        String type = null;
        if (subId > 0) {
            try {
                type = IMtkTelephonyEx.Stub.asInterface(ServiceManager.getService("phoneEx"))
                       .getIccCardType(subId);
            } catch (RemoteException e) {
                Log.d("@M_" + TAG, "getSIMType, exception: ", e);
            }
        }
        Log.d(TAG, "getSIMType type : " + type);
        return type;
     }

    /**
    * app use to judge if need confirm before switch data.
    * @return false is no need confirm
    */
    private boolean isShowRcsDialog(int switchtoSubId) {
        if (!isGeminiSupport() || !isRcsSupport()) {
            Log.d("@M_" + TAG, "isShowRcsDialog(),not support");
            return false;
        }

        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        Log.d("@M_" + TAG, "subId:" + subId);
        //currently no default data card
        if ((isCMCCCard(subId) && USIM.equals(getSIMType(subId)))
            && !(isCMCCCard(switchtoSubId) && USIM.equals(getSIMType(switchtoSubId)))) {
            return true;
        } else {
            return false;
        }
    }
    private boolean is4GSimCard(int subId) {
        if (isMtkSvlteSupport() || isMtkSrlteSupport()) {
            MtkTelephonyManagerEx telephonyManagerEx = MtkTelephonyManagerEx.getDefault();
            String values[] = telephonyManagerEx.getSupportCardType(
                    SubscriptionManager.getSlotIndex(subId));
            Log.d(TAG, "is4GSimCard subId = " + subId + " cardType = " + Arrays.toString(values));
            if (values == null) {
                return false;
            }
            for (String s : values) {
                if (s.equals("USIM")) {
                    return true;
                }
            }
        } else {
            if (USIM.equals(getSIMType(subId))) {
                return true;
            }
        }
        return false;
    }
    /**
    * app use to judge if need confirm before switch data.
    * @return false is no need confirm
    */
    private boolean isShowVoLTEDialog(int switchtoSubId) {
        if (!isGeminiSupport() || !isLTESupport() || !isVoLTESupport()) {
            Log.d("@M_" + TAG, "isShowConfirmDialog(),not support");
            return false;
        }
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        if (is4GSimCard(subId) && !is4GSimCard(switchtoSubId)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Called when SIM dialog is about to show for SIM info changed.
     * @return false if plug-in do not need SIM dialog
     */
    public boolean isSimDialogNeeded() {
        return false;
    }

    /**
     * customizeValue.
     * @param value value
     * @return value
     */
    private int customizeValues(int value) {
        if ((value == 0) && (SubscriptionManager.from(mContext)
                .getActiveSubscriptionInfoCount() == 1) && isGeminiSupport()) {
            value = 1;
            Log.d("@M_" + TAG, "customizeValue, value = " + value);
        }
        return value;
    }

    /**
     * configSimPreferenceScreen.
     * @param simPref simPref
     * @param type type
     * @param size size
     */
//    public void configSimPreferenceScreen(Preference simPref, String type, int size) {
//        Log.d("@M_" + TAG, "configSimPreference: " + type);
//        if (isGeminiSupport()) {
//            switch (type) {
//                case KEY_CELLULAR_DATA: {
//                    int defaultSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
//                    int defaultSlotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
//                    int anotherSubId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
//                    if (SubscriptionManager.from(mContext).
//                             getActiveSubscriptionInfoCount() > 1) {
//                        defaultSubId = SubscriptionManager.getDefaultDataSubscriptionId();
//                        Log.d("@M_" + TAG, "configSimPreference,defaultSub:" + defaultSubId);
//                        if (defaultSubId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
//                            break;
//                        }
//                        defaultSlotId = SubscriptionManager.getSlotIndex(defaultSubId);
//                        if (defaultSlotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
//                            break;
//                        }
//                        anotherSubId = getSubIdBySlot(1 - defaultSlotId);
//                        Log.d("@M_" + TAG, "configSimPreference,anotherSub:" + anotherSubId);
//                        String defaultPlmn = getSimOperator(defaultSubId);
//                        String anotherPlmn = getSimOperator(anotherSubId);
//                        if (defaultPlmn == null || anotherPlmn == null
//                                || defaultPlmn.equals("") || anotherPlmn.equals("")) {
//                            simPref.setEnabled(false);
//                        }
//                        if (isCMCCCard(defaultSubId) && !isCMCCCard(anotherSubId)) {
//                            Log.d("@M_" + TAG, "configSimPreference,setDefault Enabled false");
//                            simPref.setEnabled(false);
//                        } else if (isCMCCCard(anotherSubId) && !isCMCCCard(defaultSubId)) {
//                            Log.d("@M_" + TAG, "configSimPreference,setCMCC Enabled false");
//                            simPref.setEnabled(false);
//                        }
//                    }
//                    break;
//                }
//                case KEY_CALLS:
//                case KEY_SMS: {
//                    boolean hasActiveSubscription
//                        = SubscriptionManager.from(mContext).getActiveSubscriptionInfoCount() >= 1;
//                    Log.i(TAG, "isGeminiSupport configSimPreferenceScreen call or sms "
//                        + " hasActiveSubscription = " + hasActiveSubscription);
//                    Log.d("@M_" + TAG, "configSimPreference, setEnabled size >= 1");
//                    simPref.setEnabled(size >= 1 && hasActiveSubscription);
//                    break;
//                }
//                default:
//                    break;
//            }
//        }
//     }

    private static boolean isMtkSvlteSupport() {
        boolean isSupport = ONE.equals(
                SystemProperties.get("ro.vendor.mtk_c2k_lte_mode")) ? true : false;
        Log.d(TAG, "isMtkSvlteSupport(): " + isSupport);
        return isSupport;
    }

    private static boolean isMtkSrlteSupport() {
        boolean isSupport = TWO.equals(
                SystemProperties.get("ro.vendor.mtk_c2k_lte_mode")) ? true : false;
        Log.d(TAG, "isMtkSrlteSupport(): " + isSupport);
        return isSupport;
    }

    /**
     * simDialogOnClick. Plugin handle onClick.
     * @param id type of sim prefrence
     * @param value value of position selected
     * @param context context
     * @return handled by plugin or not
     */
//    public boolean simDialogOnClick(int id, int value, Context context) {
//        Log.d(TAG, "simDialogOnClick id: " + id + " value: " + value);
//        final SubscriptionManager subscriptionManager = SubscriptionManager.from(context);
//        final List<SubscriptionInfo> subInfoList =
//                subscriptionManager.getActiveSubscriptionInfoList();
//        switch (id) {
//            case DATA_PICK:
//                if (null == subInfoList || subInfoList.size() < 1) {
//                    Log.d(TAG, "null == subInfoList or size =0 do nothing," +
//                            " just return false for google logic");
//                    return false;
//                }
//                SubscriptionInfo sir = subInfoList.get(value);
//                int targetSub = (sir == null ? null : sir.getSubscriptionId());
//                if (switchDefaultDataSubId(context, targetSub)) {
//                    return true;
//                }
//                break;
//            case CALLS_PICK:
//                final TelecomManager telecomManager = TelecomManager.from(context);
//                final List<PhoneAccountHandle> phoneAccountsList =
//                    telecomManager.getCallCapablePhoneAccounts();
//                Log.d(TAG, "phoneAccountsList = " + phoneAccountsList.toString());
//                value = customizeValues(value);
//                Log.d(TAG, "value = " + value);
//                /// M: for ALPS02320816 @{
//                // phone account may changed in background
//                if (value > phoneAccountsList.size()) {
//                    Log.w(TAG, "phone account changed, do noting! value = " +
//                        value + ", phone account size = " + phoneAccountsList.size());
//                    return true;
//                }
//                PhoneAccountHandle phoneAccount =
//                    (value < 1 ? null : phoneAccountsList.get(value - 1));
//                Log.d(TAG, "setUserSelectedOutgoingPhoneAccount phoneAccount = " + phoneAccount);
//                telecomManager.setUserSelectedOutgoingPhoneAccount(phoneAccount);
//                return true;
//            default:
//                return false;
//        }
//        return false;
//    }

    /**
     * for CMCC required, maybe update main capability when ischecked is false
     * @param isChecked  when radio off means false
     * @param subid subId
     */
    @Override
    public void customizeMainCapabily(boolean isChecked, int subId) {
        int slotId = SubscriptionManager.getSlotIndex(subId);
        int simMode = Settings.Global.getInt(mContext.getContentResolver(),
                MtkSettingsExt.Global.MSIM_MODE_SETTING, -1);
        Log.d("@M_" + TAG, "customizeMainCapabily(), isChecked" + isChecked
                + " subId = " + subId + " slotId = " + slotId
                + " simMode = " + simMode);
        if (isChecked) {
            return;
        }
        if (slotId >= 0 && (0 != simMode)
                && SubscriptionManager.from(mContext).getActiveSubscriptionInfoCount() > 1) {
            handleVoiceCallSim(slotId);
            handleSmsSim(slotId);
            dualWithRadioChanged(slotId);
        }
    }

    private void handleVoiceCallSim(int slotId) {
        TelecomManager telecomMgr = TelecomManager.from(mContext);
        PhoneAccountHandle handle = telecomMgr.getUserSelectedOutgoingPhoneAccount();
        if (handle == null) {
            Log.d("@M_" + TAG, "handleVoiceCallSimId handle == null,return");
            return;
        }
        PhoneAccount phoneAccount = telecomMgr.getPhoneAccount(handle);
        TelephonyManager tm = TelephonyManager.from(mContext);
        int curVoiceSubId = tm.getSubIdForPhoneAccount(phoneAccount);
        if (curVoiceSubId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Log.d("@M_" + TAG, "handleVoiceCallSimId current not valid subid, return");
            return;
        }
        int toCloseSubId = getSubIdBySlot(slotId);
        Log.d("@M_" + TAG, "handleVoiceCallSimId curVoiceSubId = " + curVoiceSubId);
        if (toCloseSubId == curVoiceSubId && toCloseSubId >= 0) {
            int subid = getSubIdBySlot(1 - slotId);
            if (subid >= 0) {
                switchVoiceCallDefaultSim(subid);
            }
        }
    }

    private void handleSmsSim(int slotId) {
        int curSmsSubId = SubscriptionManager.getDefaultSmsSubscriptionId();
        int toCloseSubId = getSubIdBySlot(slotId);
        Log.d(TAG, "handleSmsSimId curSmsSubId:" + curSmsSubId + " toCloseSubId:" + toCloseSubId);
        if (toCloseSubId == curSmsSubId && toCloseSubId >= 0) {
            int subid = getSubIdBySlot(1 - slotId);
            if (subid >= 0) {
                switchSmsDefaultSim(subid);
            }
        }
    }

    private void dualWithRadioChanged(int slotId) {
        if (mIsShutDown) {
            Log.d(TAG, "mIsShutDown !");
            return;
        }
        SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        TelephonyManager tm = TelephonyManager.from(mContext);

        if (isAirplaneModeOn(mContext)) {
            Log.d(TAG, "airplane mode on!");
            return;
        }

        if (slotId >= 0
            && subscriptionManager.getActiveSubscriptionInfoCount() > 1) {
            int curConSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            int toCloseSubId = getSubIdBySlot(slotId);
            int anotherSubId = getSubIdBySlot(1 - slotId);
            Log.d("@M_" + TAG, "toCloseSubId = " + toCloseSubId
                    + "curConSubId = " + curConSubId
                    + " anotherSubId = " + anotherSubId );
            if(curConSubId > 0 && toCloseSubId > 0){
                if (toCloseSubId == curConSubId) {
                    subscriptionManager.setDefaultDataSubId(anotherSubId);
                    Log.d("@M_" + TAG, "setdefault is anotherSubId = " + anotherSubId);
                }
            }
        }
    }
}
