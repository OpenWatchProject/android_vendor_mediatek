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
package com.mediatek.internal.telephony.worldphone;

import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.TelephonyManager;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import com.android.internal.telecom.ITelecomService;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.android.internal.telephony.IccCard;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccController;
import mediatek.telephony.MtkServiceState;
import com.mediatek.internal.telephony.ModemSwitchHandler;
import com.mediatek.internal.telephony.MtkRIL;

/**
 *@hide
 */
public class WorldPhoneOp01 extends Handler implements IWorldPhone {
    private static Object sLock = new Object();
    private static final int PROJECT_SIM_NUM = WorldPhoneUtil.getProjectSimNum();
    private static final int[] FDD_STANDBY_TIMER = {
        60
    };
    private static final int[] TDD_STANDBY_TIMER = {
        40
    };
    private static final String[] PLMN_TABLE_TYPE1 = {
        "46000", "46002", "46004", "46007", "46008",
        // Lab test IMSI
        "00101", "00211", "00321", "00431", "00541", "00651",
        "00761", "00871", "00902", "01012", "01122", "01232",
        "46602", "50270"
    };
    private static final String[] PLMN_TABLE_TYPE3 = {
        "46001", "46006", "46009", "45407",
        "46003", "46005", "45502", "46011"
    };
    private static final String[] MCC_TABLE_DOMESTIC = {
        "460",
        // Lab test PLMN
        "001", "002", "003", "004", "005", "006",
        "007", "008", "009", "010", "011", "012"
    };

    /**
     * The property is used to check if the card is cdma 3G dual mode card in the slot.
     * @hide
     */
     private static final String[] PROPERTY_RIL_CT3G = {
         "vendor.gsm.ril.ct3g",
         "vendor.gsm.ril.ct3g.2",
         "vendor.gsm.ril.ct3g.3",
         "vendor.gsm.ril.ct3g.4",
     };
    private static final int EMSR_STANDBY_TIMER = 8;
    private static Context sContext = null;
    private static Phone sDefultPhone = null;
    private static Phone[] sProxyPhones = null;
    private static Phone[] sActivePhones = new Phone[PROJECT_SIM_NUM];
    private static CommandsInterface[] smCi = new CommandsInterface[PROJECT_SIM_NUM];
    private static MtkRIL[] sCi = new MtkRIL[PROJECT_SIM_NUM];
    private static String sPlmnSs;
    private static String sLastPlmn;
    private static String[] sImsi = new String[PROJECT_SIM_NUM];
    private static String[] sNwPlmnStrings;
    private static int sVoiceRegState;
    private static int sDataRegState;
    private static int sRilVoiceRegState;
    private static int sRilDataRegState;
    private static int sRilVoiceRadioTechnology;
    private static int sRilDataRadioTechnology;
    private static int sUserType;
    private static int sRegion;
    private static int sDenyReason;
    private static int sMajorSim;
    private static int sDefaultBootuUpModem = ModemSwitchHandler.MD_TYPE_UNKNOWN;
    private static int[] sSuspendId = new int[PROJECT_SIM_NUM];
    private static boolean sVoiceCapable;
    private static boolean[] sIsInvalidSim = new boolean[PROJECT_SIM_NUM];
    private static boolean[] sSuspendWaitImsi = new boolean[PROJECT_SIM_NUM];
    private static boolean[] sFirstSelect = new boolean[PROJECT_SIM_NUM];
    private static UiccController sUiccController = null;
    private static IccRecords[] sIccRecordsInstance = new IccRecords[PROJECT_SIM_NUM];
    private static MtkServiceState sServiceState;
    private static ModemSwitchHandler sModemSwitchHandler = null;
    private static int sTddStandByCounter;
    private static int sFddStandByCounter;
    private static boolean sWaitInTdd;
    private static boolean sWaitInFdd;
    private static boolean sWaitInEmsrResume;
    private static boolean sIsAutoSelectEnable;  //[ALPS01881475]
    private static boolean sIsResumeCampingFail1; //[ALPS01974750]
    private static boolean sIsResumeCampingFail2;
    private static boolean sIsResumeCampingFail3;
    private static boolean sIsResumeCampingFail4;
    private static int sBtSapState; //[ALPS02302039]
    private static int sSwitchModemCauseType;
    private boolean mIsRegisterEccStateReceiver = false;
    private static int sIsWaintInTddTimeOut;
    private static int sIsWaintInFddTimeOut;
    private static int sSimLocked;

    public WorldPhoneOp01() {
        logd("Constructor invoked");
        sDefultPhone = PhoneFactory.getDefaultPhone();
        sProxyPhones = PhoneFactory.getPhones();
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            sActivePhones[i] = sProxyPhones[i];
            smCi[i] = sActivePhones[i].mCi;
            sCi[i] = (MtkRIL)smCi[i];
            sActivePhones[i].registerForServiceStateChanged(
                    this, EVENT_SERVICE_STATE_CHANGED_1 + i, null);
        }
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            sCi[i].setOnPlmnChangeNotification(this, EVENT_REG_PLMN_CHANGED_1 + i, null);
            sCi[i].setOnRegistrationSuspended(this, EVENT_REG_SUSPENDED_1 + i, null);
            sCi[i].registerForOn(this, EVENT_RADIO_ON_1 + i, null);
            sCi[i].setInvalidSimInfo(this, EVENT_INVALID_SIM_NOTIFY_1 + i, null);
            if (WorldPhoneUtil.isC2kSupport()) {
                sCi[i].registerForGmssRatChanged(this, EVENT_WP_GMSS_RAT_CHANGED_1 + i, null);
            }
        }

        sModemSwitchHandler = new ModemSwitchHandler();
        logd(ModemSwitchHandler.modemToString(ModemSwitchHandler.getActiveModemType()));
        IntentFilter intentFilter =
                new IntentFilter(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);
        intentFilter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(ACTION_SHUTDOWN_IPO);
        intentFilter.addAction(ACTION_ADB_SWITCH_MODEM);
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        intentFilter.addAction(ACTION_SAP_CONNECTION_STATE_CHANGED);
        if (sDefultPhone != null) {
            sContext = sDefultPhone.getContext();
        } else {
            logd("DefaultPhone = null");
        }
        sVoiceCapable = sContext.getResources().getBoolean(
                com.android.internal.R.bool.config_voice_capable);
        sContext.registerReceiver(mWorldPhoneReceiver, intentFilter);

        sTddStandByCounter = 0;
        sFddStandByCounter = 0;
        sWaitInTdd = false;
        sWaitInFdd = false;
        sWaitInEmsrResume = false;
        sRegion = REGION_UNKNOWN;
        sLastPlmn = null;
        sBtSapState = 0; //[ALPS02302039]
        sIsWaintInTddTimeOut = 0;
        sIsWaintInFddTimeOut = 0;
        resetAllProperties();
        if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_MANUAL) {
            logd("Auto select disable");
            //sMajorSim = AUTO_SWITCH_OFF;    //[ALPS01881475]
            sIsAutoSelectEnable = false;
            SystemProperties.set(
                    WORLD_PHONE_AUTO_SELECT_MODE, Integer.toString(SELECTION_MODE_MANUAL));
        } else {
            logd("Auto select enable");
            sIsAutoSelectEnable = true;    //[ALPS01881475]
            SystemProperties.set(
                    WORLD_PHONE_AUTO_SELECT_MODE, Integer.toString(SELECTION_MODE_AUTO));
        }
        FDD_STANDBY_TIMER[sFddStandByCounter] =
                SystemProperties.getInt(
                        WORLD_PHONE_FDD_MODEM_TIMER, FDD_STANDBY_TIMER[sFddStandByCounter]);
        SystemProperties.set(
                WORLD_PHONE_FDD_MODEM_TIMER,
                Integer.toString(FDD_STANDBY_TIMER[sFddStandByCounter]));
        logd("FDD_STANDBY_TIMER = " + FDD_STANDBY_TIMER[sFddStandByCounter] + "s");
        logd("sDefaultBootuUpModem = " + sDefaultBootuUpModem);
    }

    private void handleSimCardStateChanged(int slotId, int state) {
        if (state == (TelephonyManager.SIM_STATE_ABSENT)) {
            sLastPlmn = null;
            sImsi[slotId] = "";
            sFirstSelect[slotId] = true;
            sIsInvalidSim[slotId] = false;
            sSuspendWaitImsi[slotId] = false;
            if (slotId == sMajorSim) {
                logd("Major SIM removed, no world phone service");
                removeModemStandByTimer();
                sUserType = UNKNOWN_USER;
                sDenyReason = CAMP_ON_DENY_REASON_UNKNOWN;
                sMajorSim = MAJOR_SIM_UNKNOWN;
                sRegion = REGION_UNKNOWN;
            } else {
                logd("SIM" + slotId + " is not major SIM");
            }
        }
    }

    private void handleSimApplicationStateChanged(int slotId, int state) {
        if ((true == WorldPhoneUtil.getSimLockedState(state)) &&
                (sIsAutoSelectEnable) && (slotId != sMajorSim)) {
            sSimLocked = 1;
        }
        if ((false == WorldPhoneUtil.getSimLockedState(state)) &&
                (sIsAutoSelectEnable) && (slotId != sMajorSim) &&
                (sSimLocked == 1)) {
            logd("retry to world mode change after not major sim pin unlock");
            sSimLocked = 0;
            handleSimSwitched();
        }
        if (state == (TelephonyManager.SIM_STATE_LOADED)) {
            if (sMajorSim == MAJOR_SIM_UNKNOWN) {
                sMajorSim = WorldPhoneUtil.getMajorSim();
            }
            sUiccController = UiccController.getInstance();
            if (sUiccController != null) {
                sIccRecordsInstance[slotId] = sProxyPhones[slotId].getIccCard().getIccRecords();
            } else {
                logd("Null sUiccController");
                return;
            }
            if (sIccRecordsInstance[slotId] != null) {
                sImsi[slotId] = sIccRecordsInstance[slotId].getIMSI();
            } else {
                logd("Null sIccRecordsInstance");
                return;
            }
            if ((sIsAutoSelectEnable) && (slotId == sMajorSim)) {
                logd("Major SIM");
                sUserType = getUserType(sImsi[slotId]);
                if (sFirstSelect[slotId]) {
                    sFirstSelect[slotId] = false;
                    if (sUserType == TYPE1_USER || sUserType == TYPE2_USER) {
                        sSwitchModemCauseType = CAUSE_TYPE_PLMN_CHANGE;
                        logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
                        if (sRegion == REGION_DOMESTIC) {
                            handleSwitchModem(ModemSwitchHandler.MD_TYPE_TDD);
                        } else if (sRegion == REGION_FOREIGN) {
                            handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
                        } else {
                            logd("Region unknown");
                        }
                    } else if (sUserType == TYPE3_USER) {
                        sSwitchModemCauseType = CAUSE_TYPE_OTHERS;
                        logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
                        handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
                    }
                } // sFirstSelect[slotId]

                if (sSuspendWaitImsi[slotId]) {
                    sSuspendWaitImsi[slotId] = false;
                    if (sNwPlmnStrings != null) {
                        logd("IMSI fot slot" + slotId + " now ready, resuming PLMN:"
                                + sNwPlmnStrings[0] + " with ID:" + sSuspendId[slotId]
                                + " sWaitInEmsrResume:" + sWaitInEmsrResume);
                        if (sWaitInEmsrResume == false) {
                            resumeCampingProcedure(slotId, false);
                        } else {
                            resumeCampingProcedure(slotId, true);
                        }
                    } else {
                        logd("sNwPlmnStrings is Null");
                    }
                } // sSuspendWaitImsi[slotId]
            } else {
                logd("Not major SIM");
                getUserType(sImsi[slotId]);
                if (sSuspendWaitImsi[slotId]) {
                    sSuspendWaitImsi[slotId] = false;
                    logd("IMSI fot slot" + slotId + ", resuming with ID:" + sSuspendId[slotId]);
                    sCi[slotId].setResumeRegistration(sSuspendId[slotId], null);
                }
            }
        } // TelephonyManager.SIM_STATE_LOADED
    }

    private final BroadcastReceiver mWorldPhoneReceiver = new  BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logd("Action: " + action);
            int slotId;
            int state;
            if (action.equals(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)) {
                state = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                        TelephonyManager.SIM_STATE_UNKNOWN);
                slotId = intent.getIntExtra(PhoneConstants.SLOT_KEY, PhoneConstants.SIM_ID_1);
                sMajorSim = WorldPhoneUtil.getMajorSim();
                logd("slotId: " + slotId + " state: " + state + " sMajorSim:" + sMajorSim);
                handleSimApplicationStateChanged(slotId, state);
            } else if (action.equals(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED)) {
                state = intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE,
                        TelephonyManager.SIM_STATE_UNKNOWN);
                slotId = intent.getIntExtra(PhoneConstants.SLOT_KEY, PhoneConstants.SIM_ID_1);
                sMajorSim = WorldPhoneUtil.getMajorSim();
                logd("slotId: " + slotId + " state: " + state + " sMajorSim:" + sMajorSim);
                handleSimCardStateChanged(slotId, state);
            } else if (action.equals(ACTION_SHUTDOWN_IPO)) {
                if (sDefaultBootuUpModem == ModemSwitchHandler.MD_TYPE_FDD) {
                    if (WorldPhoneUtil.isLteSupport()) {
                        ModemSwitchHandler.reloadModem(sCi[PhoneConstants.SIM_ID_1],
                                ModemSwitchHandler.MD_TYPE_LWG);
                        logd("Reload to FDD CSFB modem");
                    } else {
                        ModemSwitchHandler.reloadModem(sCi[PhoneConstants.SIM_ID_1],
                                ModemSwitchHandler.MD_TYPE_WG);
                        logd("Reload to WG modem");
                    }
                } else if (sDefaultBootuUpModem == ModemSwitchHandler.MD_TYPE_TDD) {
                    if (WorldPhoneUtil.isLteSupport()) {
                        ModemSwitchHandler.reloadModem(sCi[PhoneConstants.SIM_ID_1],
                                ModemSwitchHandler.MD_TYPE_LTG);
                        logd("Reload to TDD CSFB modem");
                    } else {
                        ModemSwitchHandler.reloadModem(sCi[PhoneConstants.SIM_ID_1],
                                ModemSwitchHandler.MD_TYPE_TG);
                        logd("Reload to TG modem");
                    }
                }
            } else if (action.equals(ACTION_ADB_SWITCH_MODEM)) {
                int toModem = intent.getIntExtra(ModemSwitchHandler.EXTRA_MD_TYPE,
                        ModemSwitchHandler.MD_TYPE_UNKNOWN);
                logd("toModem: " + toModem);
                if (toModem == ModemSwitchHandler.MD_TYPE_WG
                        || toModem == ModemSwitchHandler.MD_TYPE_TG
                        || toModem == ModemSwitchHandler.MD_TYPE_LWG
                        || toModem == ModemSwitchHandler.MD_TYPE_LTG) {
                    setModemSelectionMode(IWorldPhone.SELECTION_MODE_MANUAL, toModem);
                } else {
                    setModemSelectionMode(IWorldPhone.SELECTION_MODE_AUTO, toModem);
                }
            } else if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                if (intent.getBooleanExtra("state", false) == false) {
                    logd("Leave flight mode");
                    sLastPlmn = null;
                    for (int i = 0; i < PROJECT_SIM_NUM; i++) {
                        sIsInvalidSim[i] = false;
                    }
                } else {
                    logd("Enter flight mode");
                    for (int i = 0; i < PROJECT_SIM_NUM; i++) {
                        sFirstSelect[i] = true;
                    }
                    sRegion = REGION_UNKNOWN;
                }
            } else if (action.equals(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE)) {
                //if (sMajorSim != AUTO_SWITCH_OFF) {  //[ALPS01881475]
                    sMajorSim = WorldPhoneUtil.getMajorSim();
                    if (WorldPhoneUtil.isSimSwitching() == true) {
                        WorldPhoneUtil.setSimSwitchingFlag(false);
                        ModemSwitchHandler.setActiveModemType(WorldPhoneUtil.getToModemType());
                    }
                //}
                handleSimSwitched();
            //[ALPS02302039]
            } else if (action.equals(ACTION_SAP_CONNECTION_STATE_CHANGED)) {
                int sapState = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, 0);
                if (sapState == BluetoothProfile.STATE_CONNECTED){
                    logd("BT_SAP connection state is CONNECTED");
                    sBtSapState = 1;
                } else if (sapState == BluetoothProfile.STATE_DISCONNECTED){
                    logd("BT_SAP connection state is DISCONNECTED");
                    sBtSapState = 0;
                } else {
                    logd("BT_SAP connection state is "+sapState);
                }
            }
            logd("Action: " + action + " handle end");
        }
    };

    public void handleMessage(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        switch (msg.what) {
            case EVENT_RADIO_ON_1:
                logd("handleMessage : <EVENT_RADIO_ON_1>");
                handleRadioOn(PhoneConstants.SIM_ID_1);
                break;
            case EVENT_REG_PLMN_CHANGED_1:
                logd("handleMessage : <EVENT_REG_PLMN_CHANGED_1>");
                handlePlmnChange(ar, PhoneConstants.SIM_ID_1);
                break;
            case EVENT_REG_SUSPENDED_1:
                logd("handleMessage : <EVENT_REG_SUSPENDED_1>");
                handleRegistrationSuspend(ar, PhoneConstants.SIM_ID_1);
                break;
            case EVENT_RADIO_ON_2:
                logd("handleMessage : <EVENT_RADIO_ON_2>");
                handleRadioOn(PhoneConstants.SIM_ID_2);
                break;
            case EVENT_REG_PLMN_CHANGED_2:
                logd("handleMessage : <EVENT_REG_PLMN_CHANGED_2>");
                handlePlmnChange(ar, PhoneConstants.SIM_ID_2);
                break;
            case EVENT_REG_SUSPENDED_2:
                logd("handleMessage : <EVENT_REG_SUSPENDED_2>");
                handleRegistrationSuspend(ar, PhoneConstants.SIM_ID_2);
                break;
            case EVENT_RADIO_ON_3:
                logd("handleMessage : <EVENT_RADIO_ON_3>");
                handleRadioOn(PhoneConstants.SIM_ID_3);
                break;
            case EVENT_REG_PLMN_CHANGED_3:
                logd("handleMessage : <EVENT_REG_PLMN_CHANGED_3>");
                handlePlmnChange(ar, PhoneConstants.SIM_ID_3);
                break;
            case EVENT_REG_SUSPENDED_3:
                logd("handleMessage : <EVENT_REG_SUSPENDED_3>");
                handleRegistrationSuspend(ar, PhoneConstants.SIM_ID_3);
                break;
            case EVENT_RADIO_ON_4:
                logd("handleMessage : <EVENT_RADIO_ON_4>");
                handleRadioOn(PhoneConstants.SIM_ID_4);
                break;
            case EVENT_REG_PLMN_CHANGED_4:
                logd("handleMessage : <EVENT_REG_PLMN_CHANGED_4>");
                handlePlmnChange(ar, PhoneConstants.SIM_ID_4);
                break;
            case EVENT_REG_SUSPENDED_4:
                logd("handleMessage : <EVENT_REG_SUSPENDED_4>");
                handleRegistrationSuspend(ar, PhoneConstants.SIM_ID_4);
                break;
            case EVENT_INVALID_SIM_NOTIFY_1:
                logd("handleMessage : <EVENT_INVALID_SIM_NOTIFY_1>");
                handleInvalidSimNotify(PhoneConstants.SIM_ID_1, ar);
                break;
            case EVENT_INVALID_SIM_NOTIFY_2:
                logd("handleMessage : <EVENT_INVALID_SIM_NOTIFY_2>");
                handleInvalidSimNotify(PhoneConstants.SIM_ID_2, ar);
                break;
            case EVENT_INVALID_SIM_NOTIFY_3:
                logd("handleMessage : <EVENT_INVALID_SIM_NOTIFY_3>");
                handleInvalidSimNotify(PhoneConstants.SIM_ID_3, ar);
                break;
            case EVENT_INVALID_SIM_NOTIFY_4:
                logd("handleMessage : <EVENT_INVALID_SIM_NOTIFY_4>");
                handleInvalidSimNotify(PhoneConstants.SIM_ID_4, ar);
                break;
            case EVENT_WP_GMSS_RAT_CHANGED_1:
                logd("handleMessage : <EVENT_WP_GMSS_RAT_CHANGED_1>");
                handleGmssRatChange(ar, PhoneConstants.SIM_ID_1);
                break;
            case EVENT_WP_GMSS_RAT_CHANGED_2:
                logd("handleMessage : <EVENT_WP_GMSS_RAT_CHANGED_2>");
                handleGmssRatChange(ar, PhoneConstants.SIM_ID_2);
                break;
            case EVENT_WP_GMSS_RAT_CHANGED_3:
                logd("handleMessage : <EVENT_WP_GMSS_RAT_CHANGED_3>");
                handleGmssRatChange(ar, PhoneConstants.SIM_ID_3);
                break;
            case EVENT_WP_GMSS_RAT_CHANGED_4:
                logd("handleMessage : <EVENT_WP_GMSS_RAT_CHANGED_4>");
                handleGmssRatChange(ar, PhoneConstants.SIM_ID_4);
                break;
            case EVENT_RESUME_CAMPING_1:
                if (ar.exception != null) {
                    logd("handleMessage : <EVENT_RESUME_CAMPING_1> with exception");
                    sIsResumeCampingFail1 = true;
                }
                break;
            case EVENT_RESUME_CAMPING_2:
                if (ar.exception != null) {
                    logd("handleMessage : <EVENT_RESUME_CAMPING_2> with exception");
                    sIsResumeCampingFail2 = true;
                }
                break;
            case EVENT_RESUME_CAMPING_3:
                if (ar.exception != null) {
                    logd("handleMessage : <EVENT_RESUME_CAMPING_3> with exception");
                    sIsResumeCampingFail3 = true;
                }
                break;
            case EVENT_RESUME_CAMPING_4:
                if (ar.exception != null) {
                    logd("handleMessage : <EVENT_RESUME_CAMPING_4> with exception");
                    sIsResumeCampingFail4 = true;
                }
                break;
            case EVENT_SERVICE_STATE_CHANGED_1:
                logd("handleMessage : <EVENT_SERVICE_STATE_CHANGED_1>");
                handleServiceStateChange(ar, PhoneConstants.SIM_ID_1);
                break;
            case EVENT_SERVICE_STATE_CHANGED_2:
                logd("handleMessage : <EVENT_SERVICE_STATE_CHANGED_2>");
                handleServiceStateChange(ar, PhoneConstants.SIM_ID_2);
                break;
            case EVENT_SERVICE_STATE_CHANGED_3:
                logd("handleMessage : <EVENT_SERVICE_STATE_CHANGED_3>");
                handleServiceStateChange(ar, PhoneConstants.SIM_ID_3);
                break;
            case EVENT_SERVICE_STATE_CHANGED_4:
                logd("handleMessage : <EVENT_SERVICE_STATE_CHANGED_4>");
                handleServiceStateChange(ar, PhoneConstants.SIM_ID_4);
                break;
            default:
                logd("Unknown msg:" + msg.what);
        }
    }

    private void handleRadioOn(int slotId) {
        sMajorSim = WorldPhoneUtil.getMajorSim();
        logd("handleRadioOn Slot:" + slotId + " sMajorSim:" + sMajorSim);
        sIsInvalidSim[slotId] = false;
        switch (slotId) {
            case 0:
                if (sIsResumeCampingFail1) {
                    logd("try to resume camping again");
                    sCi[slotId].setResumeRegistration(sSuspendId[slotId], null);
                    sIsResumeCampingFail1 = false;
                }
                break;
            case 1:
                if (sIsResumeCampingFail2) {
                    logd("try to resume camping again");
                    sCi[slotId].setResumeRegistration(sSuspendId[slotId], null);
                    sIsResumeCampingFail2 = false;
                }
                break;
            case 2:
                if (sIsResumeCampingFail3) {
                    logd("try to resume camping again");
                    sCi[slotId].setResumeRegistration(sSuspendId[slotId], null);
                    sIsResumeCampingFail3 = false;
                }
                break;
            case 3:
                if (sIsResumeCampingFail4) {
                    logd("try to resume camping again");
                    sCi[slotId].setResumeRegistration(sSuspendId[slotId], null);
                    sIsResumeCampingFail4 = false;
                }
                break;
            default:
                logd("unknow slotid");
        }
    }

    private void handlePlmnChange(AsyncResult ar, int slotId) {
        sMajorSim = WorldPhoneUtil.getMajorSim();
        logd("Slot:" + slotId + " sMajorSim:" + sMajorSim);
        if (ar.exception == null && ar.result != null) {
            String[] plmnString = (String[]) ar.result;
            if (slotId == sMajorSim) {
                sNwPlmnStrings = plmnString;
            }
            for (int i = 0; i < plmnString.length; i++) {
                logd("plmnString[" + i + "]=" + plmnString[i]);
            }
            //[ALPS01881475]
            if (sIsAutoSelectEnable) {
                if (sMajorSim == slotId && (sUserType == TYPE1_USER || sUserType == TYPE2_USER)
                        && sDenyReason != CAMP_ON_DENY_REASON_NEED_SWITCH_TO_FDD) {
                    searchForDesignateService(plmnString[0]);
                }
                // To speed up performance in foreign countries, once get PLMN(no matter which slot)
                // determine region right away and switch modem type if needed
                sRegion = getRegion(plmnString[0]);

                //[ALPS01881475]
                //if (sUserType != TYPE3_USER && sRegion == REGION_FOREIGN
                //        && sMajorSim != AUTO_SWITCH_OFF && sMajorSim != MAJOR_CAPABILITY_OFF) {
                if (sUserType != TYPE3_USER && sRegion == REGION_FOREIGN
                        && sMajorSim != MAJOR_CAPABILITY_OFF) {
                    sSwitchModemCauseType = CAUSE_TYPE_PLMN_CHANGE;
                    logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
                    handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
                }
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private static synchronized void initNWPlmnString() {
        if (sNwPlmnStrings == null) {
            sNwPlmnStrings = new String[1];
        }
    }

    private void handleGmssRatChange(AsyncResult ar, int slotId) {
        sMajorSim = WorldPhoneUtil.getMajorSim();
        logd("Slot:" + slotId + " sMajorSim:" + sMajorSim);

        if (ar.exception == null && ar.result != null) {
            int[] info = (int[]) ar.result;
            String mccString = Integer.toString(info[1]);
            logd("[handleGmssRatChange] mccString=" + mccString);
            if ((slotId == sMajorSim) && (mccString.length() >= 3)) {
                initNWPlmnString();
                sNwPlmnStrings[0] = mccString;
            }
            if (sIsAutoSelectEnable) {
                sRegion = getRegion(mccString);
                if (sUserType != TYPE3_USER && sRegion == REGION_FOREIGN
                        && sMajorSim != MAJOR_CAPABILITY_OFF) {
                    handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
                }
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private void handleServiceStateChange(AsyncResult ar, int slotId) {
        sMajorSim = WorldPhoneUtil.getMajorSim();
        logd("Slot:" + slotId + " sMajorSim:" + sMajorSim +
                "RadioState:" + sCi[slotId].getRadioState());
        if (ar.exception == null && ar.result != null) {
            sServiceState = (MtkServiceState) ar.result;
            if (sServiceState != null) {
                sPlmnSs = sServiceState.getOperatorNumeric();
                sVoiceRegState = sServiceState.getVoiceRegState();
                sRilVoiceRegState = sServiceState.getRilVoiceRegState();
                sRilVoiceRadioTechnology = sServiceState.getRilVoiceRadioTechnology();
                sDataRegState = sServiceState.getDataRegState();
                sRilDataRegState = sServiceState.getRilDataRegState();
                sRilDataRadioTechnology = sServiceState.getRilDataRadioTechnology();
                logd("slotId: " + slotId + ", " +
                        "sMajorSim: " + sMajorSim + ", " +
                        "sPlmnSs: " + sPlmnSs + ", " +
                        "sVoiceRegState: " + sVoiceRegState);
                logd("sRilVoiceRegState: " + sRilVoiceRegState + ", " +
                        "sRilVoiceRadioTech: " +
                        sServiceState.rilRadioTechnologyToString(sRilVoiceRadioTechnology) + ", " +
                        "sDataRegState: " + sDataRegState);
                logd("sRilDataRegState: " + sRilDataRegState + ", " +
                        "sRilDataRadioTech: " + ", " +
                        sServiceState.rilRadioTechnologyToString(sRilDataRadioTechnology) + ", " +
                        "sIsAutoSelectEnable: " + sIsAutoSelectEnable); //[ALPS01881475]
                logd(ModemSwitchHandler.modemToString(
                        ModemSwitchHandler.getActiveModemType()));
                if ((sIsAutoSelectEnable) && (slotId == sMajorSim)) {
                        if (isNoService() && (sCi[slotId].getRadioState() !=
                                TelephonyManager.RADIO_POWER_UNAVAILABLE)) {
                        handleNoService();
                    } else if (isInService()) {
                        sLastPlmn = sPlmnSs;
                        removeModemStandByTimer();
                        sIsInvalidSim[slotId] = false;
                    }
                }
            } else { // if (sServiceState != null) {
                logd("Null sServiceState");
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private void handleRegistrationSuspend(AsyncResult ar, int slotId) {
        logd("Registration Suspend Slot" + slotId);
        if (ModemSwitchHandler.isModemTypeSwitching()) {
            return;
        }
        if (ar.exception == null && ar.result != null) {
            sSuspendId[slotId] = ((int[]) ar.result)[0];
            logd("Suspending with Id=" + sSuspendId[slotId]);
            if ((sIsAutoSelectEnable) && (sMajorSim == slotId)) {
                if (sUserType != UNKNOWN_USER) {
                    resumeCampingProcedure(slotId, true);
                } else {
                    sSuspendWaitImsi[slotId] = true;
                    if (!sWaitInEmsrResume) {
                        sWaitInEmsrResume = true;
                        logd("Wait EMSR:" + EMSR_STANDBY_TIMER + "s");
                        postDelayed(mEmsrResumeByTimerRunnable, EMSR_STANDBY_TIMER * 1000);
                    } else {
                        logd("Emsr Resume Timer already set:" + EMSR_STANDBY_TIMER + "s");
                    }
                    logd("User type unknown, wait for IMSI");
                }
            } else {
                logd("Not major slot, camp on OK");
                sCi[slotId].setResumeRegistration(sSuspendId[slotId], null);
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private void handleInvalidSimNotify(int slotId, AsyncResult ar) {
        logd("Slot" + slotId);
        if (ar.exception == null && ar.result != null) {
            String[] invalidSimInfo = (String[]) ar.result;
            String plmn = invalidSimInfo[0];
            int cs_invalid = Integer.parseInt(invalidSimInfo[1]);
            int ps_invalid = Integer.parseInt(invalidSimInfo[2]);
            int cause = Integer.parseInt(invalidSimInfo[3]);
            int testMode = -1;
            testMode = SystemProperties.getInt("vendor.gsm.gcf.testmode", 0);
            if (testMode != 0) {
                logd("Invalid SIM notified during test mode: " + testMode);
                return;
            }
            logd("testMode:" + testMode + ", cause: " + cause + ", cs_invalid: " + cs_invalid +
                    ", ps_invalid: " + ps_invalid + ", plmn: " + plmn);
            if (sVoiceCapable && cs_invalid == 1) {
                if (sLastPlmn == null) {
                    logd("CS reject, invalid SIM");
                    sIsInvalidSim[slotId] = true;
                    return;
                }
            }
            if (ps_invalid == 1) {
                if (sLastPlmn == null) {
                    logd("PS reject, invalid SIM");
                    sIsInvalidSim[slotId] = true;
                    return;
                }
            }
        } else {
            logd("AsyncResult is wrong " + ar.exception);
        }
    }

    private boolean handleSwitchModem(int toModem) {
        int mMajorSim = WorldPhoneUtil.getMajorSim();

        if ((sIsWaintInFddTimeOut == 0) && (sIsWaintInTddTimeOut == 0)) {
            if (isEccInProgress()) {
                logd("[handleSwitchModem]In ECC:" + mIsRegisterEccStateReceiver);
                if (!mIsRegisterEccStateReceiver) {
                    registerEccStateReceiver();
                }
                return false;
            }
        } else {
            sIsWaintInFddTimeOut = 0;
            sIsWaintInTddTimeOut = 0;
        }
        if (sSimLocked == 1) {
            logd("sim has been locked!");
            return false;
        }
        if (mMajorSim >= 0 && sIsInvalidSim[mMajorSim]
                && WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) {
            logd("Invalid SIM, switch not executed!");
            return false;
        }

        if (sIsAutoSelectEnable && isNeedSwitchModem() == false) {
            logd("[handleSwitchModem]No need to handle, switch not executed!");
            return false;
        }

        if (toModem == ModemSwitchHandler.MD_TYPE_TDD) {
            if (WorldPhoneUtil.isLteSupport()) {
                toModem = ModemSwitchHandler.MD_TYPE_LTG;
            } else {
                toModem = ModemSwitchHandler.MD_TYPE_TG;
            }
        } else if (toModem == ModemSwitchHandler.MD_TYPE_FDD) {
            if (WorldPhoneUtil.isLteSupport()) {
                toModem = ModemSwitchHandler.MD_TYPE_LWG;
            } else {
                toModem = ModemSwitchHandler.MD_TYPE_WG;
            }
        }

        if (toModem == ModemSwitchHandler.getActiveModemType()) {
            if (toModem == ModemSwitchHandler.MD_TYPE_WG) {
                logd("Already in WG modem");
            } else if (toModem == ModemSwitchHandler.MD_TYPE_TG) {
                logd("Already in TG modem");
            } else if (toModem == ModemSwitchHandler.MD_TYPE_LWG) {
                logd("Already in FDD CSFB modem");
            } else if (toModem == ModemSwitchHandler.MD_TYPE_LTG) {
                logd("Already in TDD CSFB modem");
            }
            return false;
        }

        if (!sIsAutoSelectEnable) {
            logd("Storing modem type: " + toModem);
            sCi[PhoneConstants.SIM_ID_1].storeModemType(toModem, null);
        } else {
            if (sDefaultBootuUpModem == ModemSwitchHandler.MD_TYPE_UNKNOWN) {
                logd("Storing modem type: " + toModem);
                sCi[PhoneConstants.SIM_ID_1].storeModemType(toModem, null);
            } else if (sDefaultBootuUpModem == ModemSwitchHandler.MD_TYPE_FDD) {
                if (WorldPhoneUtil.isLteSupport()) {
                    logd("Storing modem type: " + ModemSwitchHandler.MD_TYPE_LWG);
                    sCi[PhoneConstants.SIM_ID_1].storeModemType(
                            ModemSwitchHandler.MD_TYPE_LWG, null);
                } else {
                    logd("Storing modem type: " + ModemSwitchHandler.MD_TYPE_WG);
                    sCi[PhoneConstants.SIM_ID_1].storeModemType(
                            ModemSwitchHandler.MD_TYPE_WG, null);
                }
            } else if (sDefaultBootuUpModem == ModemSwitchHandler.MD_TYPE_TDD) {
                if (WorldPhoneUtil.isLteSupport()) {
                    logd("Storing modem type: " + ModemSwitchHandler.MD_TYPE_LTG);
                    sCi[PhoneConstants.SIM_ID_1].storeModemType(
                            ModemSwitchHandler.MD_TYPE_LTG, null);
                } else {
                    logd("Storing modem type: " + ModemSwitchHandler.MD_TYPE_TG);
                    sCi[PhoneConstants.SIM_ID_1].storeModemType(
                            ModemSwitchHandler.MD_TYPE_TG, null);
                }
            }
        }

        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            if (sActivePhones[i].getState() != PhoneConstants.State.IDLE) {
                logd("Phone" + i + " is not idle, modem switch not allowed");
                return false;
            }
        }

        removeModemStandByTimer();

        if (toModem == ModemSwitchHandler.MD_TYPE_WG) {
            logd("Switching to WG modem");
        } else if (toModem == ModemSwitchHandler.MD_TYPE_TG) {
            logd("Switching to TG modem");
        } else if (toModem == ModemSwitchHandler.MD_TYPE_LWG) {
            logd("Switching to FDD CSFB modem");
        } else if (toModem == ModemSwitchHandler.MD_TYPE_LTG) {
            logd("Switching to TDD CSFB modem");
        }
        if (WorldPhoneUtil.isSimSwitching() == true) {
            if (toModem == WorldPhoneUtil.getToModemType()) {
                logd("sim switching, already will to set modem:" + toModem);
                return false;
            }
        }
        // put real cause type before switch modem
        ModemSwitchHandler.reloadModemCauseType(sCi[PhoneConstants.SIM_ID_1],
                sSwitchModemCauseType);
        //no need store modem type again
        ModemSwitchHandler.switchModem(0, toModem);
        resetNetworkProperties();
        return true;
    }

    private void handleSimSwitched() {
        if (sMajorSim == MAJOR_CAPABILITY_OFF) {
            logd("Major capability turned off");
            removeModemStandByTimer();
            sUserType = UNKNOWN_USER;
        } else if (!sIsAutoSelectEnable) {
            logd("Auto modem selection disabled");
            removeModemStandByTimer();
        } else if (sMajorSim == MAJOR_SIM_UNKNOWN) {
            logd("Major SIM unknown");
        } else {
            logd("Auto modem selection enabled");
            logd("Major capability in slot" + sMajorSim);
            if (sImsi[sMajorSim] == null || sImsi[sMajorSim].equals("")) {
                // may caused by receive 3g switched intent when boot up
                logd("Major slot IMSI not ready");
                sUserType = UNKNOWN_USER;
                return;
            }
            sSwitchModemCauseType = CAUSE_TYPE_OTHERS;
            logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
            sUserType = getUserType(sImsi[sMajorSim]);
            if (sUserType == TYPE1_USER || sUserType == TYPE2_USER) {
                if (sNwPlmnStrings != null) {
                    sRegion = getRegion(sNwPlmnStrings[0]);
                }
                if (sRegion == REGION_DOMESTIC) {
                    sFirstSelect[sMajorSim] = false;
                    handleSwitchModem(ModemSwitchHandler.MD_TYPE_TDD);
                } else if (sRegion == REGION_FOREIGN) {
                    sFirstSelect[sMajorSim] = false;
                    handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
                } else {
                    logd("Unknown region");
                }
            } else if (sUserType == TYPE3_USER) {
                sFirstSelect[sMajorSim] = false;
                handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
            } else {
                logd("Unknown user type");
            }
        }
    }

    private void handleNoService() {
        logd("[handleNoService]+ Can not find service");
        logd("Type" + sUserType + " user");
        logd(WorldPhoneUtil.regionToString(sRegion));
        int mdType = ModemSwitchHandler.getActiveModemType();
        logd(ModemSwitchHandler.modemToString(mdType));
        IccCardConstants.State iccState =
                sProxyPhones[sMajorSim].getIccCard().getState();
        if (iccState == IccCardConstants.State.READY) {
            if (sUserType == TYPE1_USER || sUserType == TYPE2_USER) {
                if (mdType == ModemSwitchHandler.MD_TYPE_LTG
                        || mdType == ModemSwitchHandler.MD_TYPE_TG) {
                    if (TDD_STANDBY_TIMER[sTddStandByCounter] >= 0) {
                        if (!sWaitInTdd) {
                            sWaitInTdd = true;
                            logd("Wait " + TDD_STANDBY_TIMER[sTddStandByCounter] +
                                    "s. Timer index = " + sTddStandByCounter);
                            postDelayed(mTddStandByTimerRunnable,
                                    TDD_STANDBY_TIMER[sTddStandByCounter] * 1000);
                        } else {
                            logd("Timer already set:" +
                                    TDD_STANDBY_TIMER[sTddStandByCounter] + "s");
                        }
                    } else {
                        logd("Standby in TDD modem");
                    }
                } else if (mdType == ModemSwitchHandler.MD_TYPE_LWG
                        || mdType == ModemSwitchHandler.MD_TYPE_WG) {
                    if (FDD_STANDBY_TIMER[sFddStandByCounter] >= 0) {
                        if (sRegion == REGION_FOREIGN) {
                            if (!sWaitInFdd) {
                                sWaitInFdd = true;
                                logd("Wait " + FDD_STANDBY_TIMER[sFddStandByCounter] +
                                        "s. Timer index = " + sFddStandByCounter);
                                postDelayed(mFddStandByTimerRunnable,
                                        FDD_STANDBY_TIMER[sFddStandByCounter] * 1000);
                            } else {
                                logd("Timer already set:" +
                                        FDD_STANDBY_TIMER[sFddStandByCounter] + "s");
                            }
                        } else {
                            sSwitchModemCauseType = CAUSE_TYPE_OOS;
                            logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
                            handleSwitchModem(ModemSwitchHandler.MD_TYPE_TDD);
                        }
                    } else {
                        logd("Standby in FDD modem");
                    }
                }
            } else if (sUserType == TYPE3_USER) {
                if (mdType == ModemSwitchHandler.MD_TYPE_LWG
                        || mdType == ModemSwitchHandler.MD_TYPE_WG) {
                    logd("Standby in FDD modem");
                } else {
                    logd("Should not enter this state");
                }
            } else {
                logd("Unknow user type");
            }
        } else {
            logd("IccState not ready");
        }
        logd("[handleNoService]-");
        return;
    }

    private boolean isAllowCampOn(String plmnString, int slotId) {
        logd("[isAllowCampOn]" + plmnString + "User type: " + sUserType);
        int mdType = ModemSwitchHandler.MD_TYPE_UNKNOWN;
        sRegion = getRegion(plmnString);
        if (WorldPhoneUtil.isSimSwitching() == true) {
            mdType = WorldPhoneUtil.getToModemType();
            logd("SimSwitching mdType:" + ModemSwitchHandler.modemToString(mdType));
        } else {
            mdType = ModemSwitchHandler.getActiveModemType();
            logd("mdType:" + ModemSwitchHandler.modemToString(mdType));
        }
        if (sUserType == TYPE1_USER || sUserType == TYPE2_USER) {
            if (sRegion == REGION_DOMESTIC) {
                if (mdType == ModemSwitchHandler.MD_TYPE_LTG
                        || mdType == ModemSwitchHandler.MD_TYPE_TG) {
                    sDenyReason = CAMP_ON_NOT_DENIED;
                    logd("Camp on OK");
                    return true;
                } else if (mdType == ModemSwitchHandler.MD_TYPE_LWG
                        || mdType == ModemSwitchHandler.MD_TYPE_WG) {
                    sDenyReason = CAMP_ON_DENY_REASON_NEED_SWITCH_TO_TDD;
                    logd("Camp on REJECT");
                    return false;
                }
            } else if (sRegion == REGION_FOREIGN) {
                if (mdType == ModemSwitchHandler.MD_TYPE_LTG
                        || mdType == ModemSwitchHandler.MD_TYPE_TG) {
                    sDenyReason = CAMP_ON_DENY_REASON_NEED_SWITCH_TO_FDD;
                    logd("Camp on REJECT");
                    return false;
                } else if (mdType == ModemSwitchHandler.MD_TYPE_LWG
                        || mdType == ModemSwitchHandler.MD_TYPE_WG) {
                    sDenyReason = CAMP_ON_NOT_DENIED;
                    logd("Camp on OK");
                    return true;
                }
            } else {
                logd("Unknow region");
            }
        } else if (sUserType == TYPE3_USER) {
            if (mdType == ModemSwitchHandler.MD_TYPE_LTG
                    || mdType == ModemSwitchHandler.MD_TYPE_TG) {
                sDenyReason = CAMP_ON_DENY_REASON_NEED_SWITCH_TO_FDD;
                logd("Camp on REJECT");
                return false;
            } else if (mdType == ModemSwitchHandler.MD_TYPE_LWG
                    || mdType == ModemSwitchHandler.MD_TYPE_WG) {
                sDenyReason = CAMP_ON_NOT_DENIED;
                logd("Camp on OK");
                return true;
            }
        } else {
            logd("Unknown user type");
        }
        sDenyReason = CAMP_ON_DENY_REASON_UNKNOWN;
        logd("Camp on REJECT");

        return false;
    }

    private boolean isInService() {
        boolean inService = false;

        if (sVoiceRegState == MtkServiceState.STATE_IN_SERVICE
                || sDataRegState == MtkServiceState.STATE_IN_SERVICE) {
            inService = true;
        }
        logd("inService: " + inService);
        return inService;
    }

    private boolean isNoService() {
        boolean noService = false;

        if (sVoiceRegState == MtkServiceState.STATE_OUT_OF_SERVICE
                && (sRilVoiceRegState == NetworkRegistrationInfo.
                        REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING
                || sRilVoiceRegState == MtkServiceState.
                        REGISTRATION_STATE_NOT_REGISTERED_AND_NOT_SEARCHING_EMERGENCY_CALL_ENABLED)
                && sDataRegState == MtkServiceState.STATE_OUT_OF_SERVICE
                && sRilDataRegState == NetworkRegistrationInfo.
                        REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING) {
            noService = true;
        } else {
            noService = false;
        }
        logd("noService: " + noService);
        return noService;
    }

    private int getRegion(String plmn) {
        String currentMcc;
        if (plmn == null || plmn.equals("") || plmn.length() < 3) {
            logd("[getRegion] Invalid PLMN");
            return REGION_UNKNOWN;
        }
        // Lab test PLMN 46602 & 50270 are Type1 & Domestic region
        // Other real world PLMN 466xx & 502xx are Type2 & Foreign region
        currentMcc = plmn.length() >= 5 ? plmn.substring(0, 5) : null;
        if (currentMcc != null && (currentMcc.equals("46602") || currentMcc.equals("50270"))) {
            return REGION_DOMESTIC;
        }
        currentMcc = plmn.substring(0, 3);
        for (String mcc : MCC_TABLE_DOMESTIC) {
            if (currentMcc.equals(mcc)) {
                logd("[getRegion] REGION_DOMESTIC");
                return REGION_DOMESTIC;
            }
        }
        logd("[getRegion] REGION_FOREIGN");
        return REGION_FOREIGN;
    }

    private int getUserType(String imsi) {
        if (imsi != null && !imsi.equals("")) {
            imsi = imsi.substring(0, 5);
            for (String mccmnc : PLMN_TABLE_TYPE1) {
                if (imsi.equals(mccmnc)) {
                    logd("[getUserType] Type1 user");
                    return TYPE1_USER;
                }
            }
            for (String mccmnc : PLMN_TABLE_TYPE3) {
                if (imsi.equals(mccmnc)) {
                    logd("[getUserType] Type3 user");
                    return TYPE3_USER;
                }
            }
            logd("[getUserType] Type2 user");
            return TYPE2_USER;
        } else {
            logd("[getUserType] null IMSI");
            return UNKNOWN_USER;
        }
    }

    private void resumeCampingProcedure(int slotId, boolean isResumeModem) {
        logd("Resume camping slot:" + slotId + " isResumeModem:" + isResumeModem +
                ", sSimLocked:" + sSimLocked);
        String plmnString = sNwPlmnStrings[0];
        boolean switchModem = false;
        if (sSimLocked == 1 || isAllowCampOn(plmnString, slotId) || isNeedSwitchModem() == false) {
            switchModem = false;
        } else {
            sSwitchModemCauseType = CAUSE_TYPE_PLMN_CHANGE;
            logd("sSwitchModemCauseType=" + sSwitchModemCauseType + ",DenyReason=" + sDenyReason);
            if (sDenyReason == CAMP_ON_DENY_REASON_NEED_SWITCH_TO_FDD) {
                switchModem = handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
            } else if (sDenyReason == CAMP_ON_DENY_REASON_NEED_SWITCH_TO_TDD) {
                switchModem = handleSwitchModem(ModemSwitchHandler.MD_TYPE_TDD);
            }
        }
        if (switchModem == false) {
            removeModemStandByTimer();
            removeEmsrResumeByTimer();
            if (isResumeModem) {
                sCi[slotId].setResumeRegistration(
                        sSuspendId[slotId], obtainMessage(EVENT_RESUME_CAMPING_1 + slotId));
            }
        }
    }

    private Runnable mTddStandByTimerRunnable = new Runnable() {
        public void run() {
            sTddStandByCounter++;
            if (sTddStandByCounter >= TDD_STANDBY_TIMER.length) {
                sTddStandByCounter = TDD_STANDBY_TIMER.length - 1;
            }
            if (sBtSapState == 0 ){
                logd("TDD time out!");
                sSwitchModemCauseType = CAUSE_TYPE_OOS;
                sIsWaintInTddTimeOut = 1;
                logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
                handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
            } else {
                logd("TDD time out but BT SAP is connected, switch not executed!");
            }
        }
    };

    private Runnable mFddStandByTimerRunnable = new Runnable() {
        public void run() {
            sFddStandByCounter++;
            if (sFddStandByCounter >= FDD_STANDBY_TIMER.length) {
                sFddStandByCounter = FDD_STANDBY_TIMER.length - 1;
            }
            if (sBtSapState == 0 ){
                logd("FDD time out!");
                sSwitchModemCauseType = CAUSE_TYPE_OOS;
                sIsWaintInFddTimeOut = 1;
                logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
                handleSwitchModem(ModemSwitchHandler.MD_TYPE_TDD);
            } else {
                logd("FDD time out but BT SAP is connected, switch not executed!");
            }
        }
    };

    private void removeModemStandByTimer() {
        if (sWaitInTdd) {
            logd("Remove TDD wait timer. Set sWaitInTdd = false");
            sWaitInTdd = false;
            removeCallbacks(mTddStandByTimerRunnable);
        }
        if (sWaitInFdd) {
            logd("Remove FDD wait timer. Set sWaitInFdd = false");
            sWaitInFdd = false;
            removeCallbacks(mFddStandByTimerRunnable);
        }
    }

    private Runnable mEmsrResumeByTimerRunnable = new Runnable() {
        public void run() {
            sWaitInEmsrResume = false;
            sMajorSim = WorldPhoneUtil.getMajorSim();
            if (sMajorSim != MAJOR_SIM_UNKNOWN
                        && sMajorSim != MAJOR_CAPABILITY_OFF) {
                if (sSuspendWaitImsi[sMajorSim]) {
                    sCi[sMajorSim].setResumeRegistration(
                        sSuspendId[sMajorSim], obtainMessage(EVENT_RESUME_CAMPING_1 + sMajorSim));
                }
            }
        }
    };

    private void removeEmsrResumeByTimer() {
        if (sWaitInEmsrResume) {
            logd("Remove EMSR wait timer. Set sWaitInEmsrResume = false");
            sWaitInEmsrResume = false;
            removeCallbacks(mEmsrResumeByTimerRunnable);
        }
    }

    private void resetAllProperties() {
        logd("[resetAllProperties]");
        sNwPlmnStrings = null;
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            sFirstSelect[i] = true;
        }
        sDenyReason = CAMP_ON_DENY_REASON_UNKNOWN;
        resetSimProperties();
        resetNetworkProperties();
        sSimLocked = 0;
    }

    private void resetNetworkProperties() {
        logd("[resetNetworkProperties]");
        synchronized (sLock) {
            for (int i = 0; i < PROJECT_SIM_NUM; i++) {
                sSuspendWaitImsi[i] = false;
            }
            if (sNwPlmnStrings != null) {
                for (int i = 0; i < sNwPlmnStrings.length; i++) {
                    sNwPlmnStrings[i] = "";
                }
            }
            removeEmsrResumeByTimer();
            sSwitchModemCauseType = CAUSE_TYPE_OTHERS;
            logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
        }
    }

    private void resetSimProperties() {
        logd("[resetSimProperties]");
        synchronized (sLock) {
            for (int i = 0; i < PROJECT_SIM_NUM; i++) {
                sImsi[i] = "";
            }
            sUserType = UNKNOWN_USER;
            sMajorSim = WorldPhoneUtil.getMajorSim();
        }
    }

    private void searchForDesignateService(String strPlmn) {
        if (strPlmn == null) {
            logd("[searchForDesignateService]- null source");
            return;
        }
        strPlmn = strPlmn.substring(0, 5);
        for (String mccmnc : PLMN_TABLE_TYPE1) {
            if (strPlmn.equals(mccmnc)) {
                logd("Find TD service");
                logd("sUserType: " + sUserType + " sRegion: " + sRegion);
                logd(ModemSwitchHandler.modemToString(ModemSwitchHandler.getActiveModemType()));
                sSwitchModemCauseType = CAUSE_TYPE_PLMN_CHANGE;
                logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
                handleSwitchModem(ModemSwitchHandler.MD_TYPE_TDD);
                break;
            }
        }

        return;
    }

    public void setModemSelectionMode(int mode, int modemType) {
        SystemProperties.set(WORLD_PHONE_AUTO_SELECT_MODE, Integer.toString(mode));

        if (mode == SELECTION_MODE_AUTO) {
            logd("Modem Selection <AUTO>");
            sIsAutoSelectEnable = true;    //[ALPS01881475]
            sMajorSim = WorldPhoneUtil.getMajorSim();
            handleSimSwitched();
        } else {
            logd("Modem Selection <MANUAL>");
            //sMajorSim = AUTO_SWITCH_OFF;    //[ALPS01881475]
            sIsAutoSelectEnable = false;
            sSwitchModemCauseType = CAUSE_TYPE_OTHERS;
            logd("sSwitchModemCauseType = " + sSwitchModemCauseType);
            handleSwitchModem(modemType);
            if (modemType == ModemSwitchHandler.getActiveModemType()) {
                removeModemStandByTimer();
            }
        }
    }

    public void notifyRadioCapabilityChange(int capabilitySimId) {
        int toModem = 0;
        logd("[setRadioCapabilityChange]");
        logd("Major capability will be set to slot:" + capabilitySimId);
        removeEmsrResumeByTimer();
        if (!sIsAutoSelectEnable) {
            logd("Auto modem selection disabled");
            removeModemStandByTimer();
        } else {
            logd("Auto modem selection enabled");
            if (sImsi[capabilitySimId] == null || sImsi[capabilitySimId].equals("")) {
                // may caused by receive 3g switched intent when boot up
                logd("Capaility slot IMSI not ready");
                sUserType = UNKNOWN_USER;
                return;
            }
            sUserType = getUserType(sImsi[capabilitySimId]);
            if (sUserType == TYPE1_USER || sUserType == TYPE2_USER) {
                if (sNwPlmnStrings != null) {
                    sRegion = getRegion(sNwPlmnStrings[0]);
                }
                if (sRegion == REGION_DOMESTIC) {
                    sFirstSelect[capabilitySimId] = false;
                    toModem = ModemSwitchHandler.MD_TYPE_TDD;
                } else if (sRegion == REGION_FOREIGN) {
                    sFirstSelect[capabilitySimId] = false;
                    toModem = ModemSwitchHandler.MD_TYPE_FDD;
                } else {
                    logd("Unknown region");
                    return;
                }
            } else if (sUserType == TYPE3_USER) {
                sFirstSelect[capabilitySimId] = false;
                toModem = ModemSwitchHandler.MD_TYPE_FDD;
            } else {
                logd("Unknown user type");
                return;
            }

            if (toModem == ModemSwitchHandler.MD_TYPE_TDD) {
                if (WorldPhoneUtil.isLteSupport()) {
                    toModem = ModemSwitchHandler.MD_TYPE_LTG;
                } else {
                    toModem = ModemSwitchHandler.MD_TYPE_TG;
                }
            } else if (toModem == ModemSwitchHandler.MD_TYPE_FDD) {
                if (WorldPhoneUtil.isLteSupport()) {
                    toModem = ModemSwitchHandler.MD_TYPE_LWG;
                } else {
                    toModem = ModemSwitchHandler.MD_TYPE_WG;
                }
            }

            logd("notifyRadioCapabilityChange: Storing modem type: " + toModem);

            if (isNeedReloadModem(capabilitySimId) == false) {
                return;
            }
            sCi[PhoneConstants.SIM_ID_1].reloadModemType(toModem, null);
            resetNetworkProperties();
            WorldPhoneUtil.setSimSwitchingFlag(true);
            WorldPhoneUtil.saveToModemType(toModem);
        }
    }

    private boolean isNeedSwitchModem() {
        boolean isNeed = true;
        int majorSimId = WorldPhoneUtil.getMajorSim();
        if (WorldPhoneUtil.isC2kSupport()) {
            int activeSvlteModeSlotId = WorldPhoneUtil.getActiveSvlteModeSlotId();
            if ((sUserType == TYPE2_USER) &&
                    (((majorSimId >= 0) && (majorSimId == activeSvlteModeSlotId)) ||
                    (isCdmaCard(majorSimId)))) {
                if (ModemSwitchHandler.getActiveModemType() ==  ModemSwitchHandler.MD_TYPE_LWG) {
                    isNeed = false;
                }
            }
        }
        logd("[isNeedSwitchModem] isNeed = " + isNeed);
        return isNeed;
    }

    private boolean isNeedReloadModem(int capabilitySimId) {
        boolean isNeed = true;
        if (WorldPhoneUtil.isC2kSupport()) {
            int activeSvlteModeSlotId = WorldPhoneUtil.getActiveSvlteModeSlotId();
            logd("[isNeedReloadModem] activeSvlteModeSlotId = " + activeSvlteModeSlotId +
                    ", sUserType = " + sUserType + ", capabilitySimId = " + capabilitySimId);
            if ((sUserType == TYPE2_USER) &&
                    (((capabilitySimId >= 0) && (capabilitySimId == activeSvlteModeSlotId)) ||
                    (isCdmaCard(capabilitySimId)))) {
                if (ModemSwitchHandler.getActiveModemType() ==  ModemSwitchHandler.MD_TYPE_LWG) {
                    isNeed = false;
                }
            }
        }
        logd("[isNeedReloadModem] isNeed = " + isNeed);
        return isNeed;
    }

    private boolean isCdmaCard(int slotId) {
        if (!SubscriptionManager.isValidPhoneId(slotId)) {
            return false;
        }
        int[] cardType = WorldPhoneUtil.getC2KWPCardType();
        logd("isCdmaCard(), cardType=" + cardType[slotId]);
        boolean retCdmaCard = ((cardType[slotId] & WorldPhoneUtil.CARD_TYPE_RUIM) > 0)
                || ((cardType[slotId] & WorldPhoneUtil.CARD_TYPE_CSIM) > 0)
                || isCt3gDualMode(slotId);
        logd("isCdmaCard(), slotId=" + slotId + " retCdmaCard=" + retCdmaCard);
        return retCdmaCard;
    }

    /**
     * Check if the specified slot is CT 3G dual mode card.
     * @param slotId slot ID
     * @return if it's CT 3G dual mode card
     * @hide
     */
    private boolean isCt3gDualMode(int slotId) {
        if (slotId < 0 || slotId >= PROPERTY_RIL_CT3G.length) {
            logd("isCt3gDualMode: invalid slotId " + slotId);
            return false;
        }
        String result = SystemProperties.get(PROPERTY_RIL_CT3G[slotId], "");
        logd("isCt3gDualMode: " + result);
        return ("1".equals(result));
    }

    private BroadcastReceiver mWorldPhoneEccStateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            logd("mWorldPhoneEccStateReceiver, received " + intent.getAction());
            if (!isEccInProgress()) {
                unRegisterEccStateReceiver();
                handleSimSwitched();
            }
        }
    };

    private void registerEccStateReceiver() {
        if (sContext == null) {
            logd("registerEccStateReceiver, context is null => return");
            return;
        }
        IntentFilter filter = new IntentFilter(TelephonyManager.ACTION_PHONE_STATE_CHANGED);
        filter.addAction(TelephonyIntents.ACTION_EMERGENCY_CALLBACK_MODE_CHANGED);
        sContext.registerReceiver(mWorldPhoneEccStateReceiver, filter);
        mIsRegisterEccStateReceiver = true;
    }

    private void unRegisterEccStateReceiver() {
        if (sContext == null) {
            logd("unRegisterEccStateReceiver, context is null => return");
            return;
        }
        sContext.unregisterReceiver(mWorldPhoneEccStateReceiver);
        mIsRegisterEccStateReceiver = false;
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
                logd("Exception of isEccInProgress");
            }
        }
        logd("isEccInProgress, value:" + value + ", inEcm:" + inEcm + ", isInEcc:" + isInEcc);
        return inEcm || isInEcc;
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, "[WPOP01]" + msg);
    }
}
