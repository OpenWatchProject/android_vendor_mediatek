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
import java.util.ArrayList;
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
import com.mediatek.internal.telephony.RadioCapabilitySwitchUtil;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.telephony.MtkTelephonyManagerEx;


/**
 *@hide
 */
public class WorldPhoneOm extends Handler implements IWorldPhone {
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
    };
    private static final String[] PLMN_TABLE_TYPE1_EXT = {
        "45412"
    };
    private static final String[] PLMN_TABLE_TYPE3 = {
        "46001", "46006", "46009", "45407",
        "46003", "46005", "45502", "46011"
    };
    private static final String[] MCC_TABLE_DOMESTIC = {
        "460"
    };
    private static final int EMSR_STANDBY_TIMER = 8;
    private static ArrayList<String> sPlmnType1;
    private static ArrayList<String> sPlmnType1Ext;
    private static ArrayList<String> sPlmnType3;
    private static ArrayList<String> sMccDomestic;

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
    // private static boolean sIsAutoSelectEnable;  //[ALPS01881475]
    private static boolean sIsResumeCampingFail1; //[ALPS01974750]
    private static boolean sIsResumeCampingFail2;
    private static boolean sIsResumeCampingFail3;
    private static boolean sIsResumeCampingFail4;
    private static int sBtSapState; //[ALPS02302039]
    private boolean mIsRegisterEccStateReceiver = false;
    private static int sIsWaintInTddTimeOut;
    private static int sIsWaintInFddTimeOut;
    private static int sSimLocked;
    private static int sSimLockedSlotId = -1;

    public WorldPhoneOm() {
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
            // C2K world phone - start
            if (WorldPhoneUtil.isC2kSupport()) {
                sCi[i].registerForGmssRatChanged(this, EVENT_WP_GMSS_RAT_CHANGED_1 + i, null);
            }
            // C2K world phone - end
        }

        sModemSwitchHandler = new ModemSwitchHandler();
        ModemSwitchHandler.getActiveModemType();

        IntentFilter intentFilter =
                new IntentFilter(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);
        intentFilter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
        intentFilter.addAction(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intentFilter.addAction(ACTION_SHUTDOWN_IPO);
        intentFilter.addAction(ACTION_ADB_SWITCH_MODEM);
        intentFilter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
        //[ALPS02302039]
        intentFilter.addAction(ACTION_SAP_CONNECTION_STATE_CHANGED);
        intentFilter.addAction(ACTION_TEST_WORLDPHONE);

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

        //For Test
        sPlmnType1 = new ArrayList<String>();
        sPlmnType1Ext = new ArrayList<String>();
        sPlmnType3 = new ArrayList<String>();
        sMccDomestic = new ArrayList<String>();
        loadDefaultData();

        if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_MANUAL) {
            logd("Auto select disable");
            //sMajorSim = AUTO_SWITCH_OFF;    //[ALPS01881475]
            // sIsAutoSelectEnable = false;
            SystemProperties.set(
                    WORLD_PHONE_AUTO_SELECT_MODE, Integer.toString(SELECTION_MODE_MANUAL));
        } else {
            logd("Auto select enable");
            // sIsAutoSelectEnable = true;    //[ALPS01881475]
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

    private boolean getSimLockState() {
        int policy = MtkTelephonyManagerEx.getDefault().getSimLockPolicy();
        if (policy >= 1 && policy <= 7) {
            return true;
        }
        return false;
    }

    private void handleSimApplicationStateChanged(int slotId, int state) {
        if ((true == WorldPhoneUtil.getSimLockedState(state)) &&
                (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) &&
                ((slotId != sMajorSim) || (slotId == sSimLockedSlotId))) {
            boolean lockPolicy = getSimLockState();
            logd("handle sim applicatin state changed," + lockPolicy + "," + state);
            if ((lockPolicy == true) && (state == TelephonyManager.SIM_STATE_NETWORK_LOCKED)) {
                sSimLocked = 0;
                sSimLockedSlotId = -1;
            } else {
                sSimLocked = 1;
                sSimLockedSlotId = slotId;
            }
        }
        if ((false == WorldPhoneUtil.getSimLockedState(state)) &&
                (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) &&
                ((slotId != sMajorSim) || (slotId == sSimLockedSlotId)) && (sSimLocked == 1)) {
            logd("retry to world mode change after not major sim pin unlock");
            sSimLocked = 0;
            sSimLockedSlotId = -1;
            handleSimSwitched();
        }

        if (state == (TelephonyManager.SIM_STATE_LOADED)) {
            logd("reset sIsInvalidSim by solt:" + slotId);
            sIsInvalidSim[slotId] = false;
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
            if ((WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) &&
                    (slotId == sMajorSim)) {
                logd("Major SIM");
                sUserType = getUserType(sImsi[slotId]);
                if (sFirstSelect[slotId]) {
                    sFirstSelect[slotId] = false;
                    if (sUserType == TYPE1_USER) {
                        if (sRegion == REGION_DOMESTIC) {
                            handleSwitchModem(ModemSwitchHandler.MD_TYPE_TDD);
                        } else if (sRegion == REGION_FOREIGN) {
                            handleSwitchModem(ModemSwitchHandler.MD_TYPE_FDD);
                        } else {
                            logd("Region unknown");
                        }
                    } else if (sUserType == TYPE2_USER || sUserType == TYPE3_USER) {
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
                logd("Not major SIM or in maual selection mode");
                getUserType(sImsi[slotId]);
                if (sSuspendWaitImsi[slotId]) {
                    sSuspendWaitImsi[slotId] = false;
                    logd("IMSI for slot" + slotId + ", resuming with ID:" + sSuspendId[slotId]);
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
                logd("slotId:" + slotId + " state:" + state + " sMajorSim:" + sMajorSim +
                        "sSimLockedSlotId:" + sSimLockedSlotId);
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
                int toModem = intent.getIntExtra(
                        ModemSwitchHandler.EXTRA_MD_TYPE, ModemSwitchHandler.MD_TYPE_UNKNOWN);
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
            } else if (action.equals(ACTION_TEST_WORLDPHONE)) {
                int fakeUserType = intent.getIntExtra(EXTRA_FAKE_USER_TYPE, UNKNOWN_USER);
                int fakeRegion = intent.getIntExtra(EXTRA_FAKE_REGION, REGION_UNKNOWN);
                handleWorldPhoneTest(fakeUserType, fakeRegion);
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
        String currMajorSim = SystemProperties.get(PROPERTY_MAJOR_SIM, "");
        if (currMajorSim != null && !currMajorSim.equals("")) {
            sMajorSim = (Integer.parseInt(currMajorSim)) - 1;
            logd("[getMajorSim]: " + sMajorSim);
        } else {
            sMajorSim = slotId;
            logd("[getMajorSim]: fail to get major SIM");
        }
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
            if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) {
                //To speed up performance in foreign countries,once get PLMN(no matter which slot)
                //determine region right away and switch modem type if needed
                sRegion = getRegion(plmnString[0]);
                if (sMajorSim == slotId && sUserType == TYPE1_USER
                        && sDenyReason != CAMP_ON_DENY_REASON_NEED_SWITCH_TO_FDD) {
                    searchForDesignateService(plmnString[0]);
                }
                //[ALPS01881475]
                //if (sUserType != TYPE3_USER && sRegion == REGION_FOREIGN
                //        && sMajorSim != AUTO_SWITCH_OFF && sMajorSim != MAJOR_CAPABILITY_OFF) {
                if (sUserType != TYPE3_USER && sRegion == REGION_FOREIGN
                        && sMajorSim != MAJOR_CAPABILITY_OFF) {

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
        int[] info = null;
        String mccString = null;

        sMajorSim = WorldPhoneUtil.getMajorSim();
        logd("Slot:" + slotId + " sMajorSim:" + sMajorSim);

        if (ar.exception == null && ar.result != null) {
            info = (int[]) ar.result;
            mccString = Integer.toString(info[1]);
            logd("[handleGmssRatChange] mccString=" + mccString);
            if ((slotId == sMajorSim) && (mccString.length() >= 3)) {
                initNWPlmnString();
                sNwPlmnStrings[0] = mccString;
            }
            if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) {
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
                        "sIsAutoSelectEnable: " + WorldPhoneUtil.getModemSelectionMode());
                ModemSwitchHandler.getActiveModemType();
                if (slotId == sMajorSim) {
                    if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) {
                        if (isNoService() && (sCi[slotId].getRadioState() !=
                                TelephonyManager.RADIO_POWER_UNAVAILABLE)) {
                            handleNoService();
                        } else if (isInService()) {
                            sLastPlmn = sPlmnSs;
                            removeModemStandByTimer();
                            logd("reset sIsInvalidSim");
                            sIsInvalidSim[slotId] = false;
                        }
                    } else {
                        if (isInService()) {
                            logd("reset sIsInvalidSim in manual mode");
                            sLastPlmn = sPlmnSs;
                            sIsInvalidSim[slotId] = false;
                        }
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
            //[ALPS01881475]
            //if (sMajorSim == slotId) {
            if ((WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) &&
                    (sMajorSim == slotId)) {
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
                logd("Not major slot or in maual selection mode, camp on OK");
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

        CommandsInterface ci = null;
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

        if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO &&
                WorldPhoneUtil.isCdmaLteDcSupport()) {
            if (isNeedSwitchModem(mMajorSim) == false) {
                logd("[handleSwitchModem]No need to handle, switch not executed!");
                return false;
            }
        }

        if (sSimLocked == 1) {
            logd("sim has been locked!");
            return false;
        }
        if (mMajorSim >= 0 && sIsInvalidSim[mMajorSim]
                && WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_AUTO) {
            logd("[handleSwitchModem] Invalid SIM, switch not executed!");
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

        if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_MANUAL) {
            logd("[handleSwitchModem] Auto select disable, storing modem type: " + toModem);
            sCi[PhoneConstants.SIM_ID_1].storeModemType(toModem, null);
        } else {
            if (sDefaultBootuUpModem == ModemSwitchHandler.MD_TYPE_UNKNOWN) {
                logd("[handleSwitchModem] Storing modem type: " + toModem);
                sCi[PhoneConstants.SIM_ID_1].storeModemType(toModem, null);
            } else if (sDefaultBootuUpModem == ModemSwitchHandler.MD_TYPE_FDD) {
                if (WorldPhoneUtil.isLteSupport()) {
                    logd("[handleSwitchModem] Storing modem type: " +
                            ModemSwitchHandler.MD_TYPE_LWG);
                    sCi[PhoneConstants.SIM_ID_1].storeModemType(
                           ModemSwitchHandler.MD_TYPE_LWG, null);
                } else {
                    logd("[handleSwitchModem] Storing modem type: " +
                            ModemSwitchHandler.MD_TYPE_WG);
                    sCi[PhoneConstants.SIM_ID_1].storeModemType(
                            ModemSwitchHandler.MD_TYPE_WG, null);
                }
            } else if (sDefaultBootuUpModem == ModemSwitchHandler.MD_TYPE_TDD) {
                if (WorldPhoneUtil.isLteSupport()) {
                    logd("[handleSwitchModem] Storing modem type: " +
                            ModemSwitchHandler.MD_TYPE_LTG);
                    sCi[PhoneConstants.SIM_ID_1].storeModemType(
                            ModemSwitchHandler.MD_TYPE_LTG, null);
                } else {
                    logd("[handleSwitchModem] Storing modem type: " +
                            ModemSwitchHandler.MD_TYPE_TG);
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
        } else if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_MANUAL) {
            logd("Auto modem selection disabled");
            removeModemStandByTimer();
        } else if (sMajorSim == MAJOR_SIM_UNKNOWN) {
            logd("Major SIM unknown");
        } else {
            logd("Major capability in slot" + sMajorSim);
            if (sImsi[sMajorSim] == null || sImsi[sMajorSim].equals("")) {
                // may caused by receive 3g switched intent when boot up
                logd("Major slot IMSI not ready");
                sUserType = UNKNOWN_USER;
                return;
            }
            sUserType = getUserType(sImsi[sMajorSim]);
            if (sUserType == TYPE1_USER) {
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
            } else if (sUserType == TYPE2_USER || sUserType == TYPE3_USER) {
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
        IccCardConstants.State iccState = (
                sProxyPhones[sMajorSim]).getIccCard().getState();
        if (iccState == IccCardConstants.State.READY) {
            if (sUserType == TYPE1_USER) {
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
                        logd("Standby in FDD modem");
                    }
                }
            } else if (sUserType == TYPE2_USER || sUserType == TYPE3_USER) {
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
        logd("[isAllowCampOn] " + plmnString + "User type: " + sUserType);
        int mdType = ModemSwitchHandler.MD_TYPE_UNKNOWN;
        sRegion = getRegion(plmnString);
        if (WorldPhoneUtil.isSimSwitching() == true) {
            mdType = WorldPhoneUtil.getToModemType();
            logd("SimSwitching mdType:" + ModemSwitchHandler.modemToString(mdType));
        } else {
            mdType = ModemSwitchHandler.getActiveModemType();
        }
        if (sUserType == TYPE1_USER) {
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
        } else if (sUserType == TYPE2_USER || sUserType == TYPE3_USER) {
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
        //[ALPS02297462] if plmn is update via EGMSS then plmn.length ==3
        //if (plmn == null || plmn.equals("") || plmn.length() < 5) {
        if (plmn == null || plmn.equals("") || plmn.length() < 3) {
            logd("[getRegion] Invalid PLMN");
            return REGION_UNKNOWN;
        }
        currentMcc = plmn.substring(0, 3);
        for (String mcc : sMccDomestic) {
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
            for (String mccmnc : sPlmnType1) {
                if (imsi.equals(mccmnc)) {
                    logd("[getUserType] Type1 user");
                    return TYPE1_USER;
                }
            }
            for (String mccmnc : sPlmnType1Ext) {
                if (imsi.equals(mccmnc)) {
                   logd("[getUserType] Extended Type1 user");
                   return TYPE1_USER;
                }
            }
            for (String mccmnc : sPlmnType3) {
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
        String plmnString;
        boolean switchModem = false;
        if (sNwPlmnStrings != null && sNwPlmnStrings[0] != null) {
            plmnString = sNwPlmnStrings[0];
            if (sSimLocked == 1 || isAllowCampOn(plmnString, slotId)) {
                switchModem = false;
            } else {
                logd("Because: " + sDenyReason);
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
        } else {
            logd("sNwPlmnStrings[0] is null");
        }
    }

    private Runnable mTddStandByTimerRunnable = new Runnable() {
        public void run() {
            sTddStandByCounter++;
            if (sTddStandByCounter >= TDD_STANDBY_TIMER.length) {
                sTddStandByCounter = TDD_STANDBY_TIMER.length - 1;
            }
            //[ALPS02302039]
            if (sBtSapState == 0 ){
                logd("TDD time out!");
                sIsWaintInTddTimeOut = 1;
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
            //[ALPS02302039]
            if (sBtSapState == 0 ){
                logd("FDD time out!");
                sIsWaintInFddTimeOut = 1;
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
        sIsResumeCampingFail1 = false; //[ALPS01974750]
        sIsResumeCampingFail2 = false;
        sIsResumeCampingFail3 = false;
        sIsResumeCampingFail4 = false;
        sBtSapState = 0; //[ALPS02302039]
        resetSimProperties();
        resetNetworkProperties();
        sSimLocked = 0;
        sSimLockedSlotId = -1;
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
        if ((strPlmn == null) || (strPlmn.length() < 5)) {
            logd("[searchForDesignateService]- null source");
            return;
        }
        strPlmn = strPlmn.substring(0, 5);
        for (String mccmnc : sPlmnType1) {
            if (strPlmn.equals(mccmnc)) {
                logd("Find TD service");
                logd("sUserType: " + sUserType + " sRegion: " + sRegion);
                if (sRegion == REGION_DOMESTIC) {
                    ModemSwitchHandler.getActiveModemType();
                    handleSwitchModem(ModemSwitchHandler.MD_TYPE_TDD);
                }
                break;
            }
        }

        return;
    }

    public void setModemSelectionMode(int mode, int modemType) {
        SystemProperties.set(WORLD_PHONE_AUTO_SELECT_MODE, Integer.toString(mode));
        if (mode == SELECTION_MODE_AUTO) {
            logd("Modem Selection <AUTO>");
            // sIsAutoSelectEnable = true;    //[ALPS01881475]
            sMajorSim = WorldPhoneUtil.getMajorSim();
            handleSimSwitched();
        } else {
            logd("Modem Selection <MANUAL>");
            //sMajorSim = AUTO_SWITCH_OFF;    //[ALPS01881475]
            // sIsAutoSelectEnable = false;
            handleSwitchModem(modemType);
            if (modemType == ModemSwitchHandler.getActiveModemType()) {
                removeModemStandByTimer();
            }
        }
    }

    public void notifyRadioCapabilityChange(int capailitySimId) {
        int majorSimId = RadioCapabilitySwitchUtil.getMainCapabilityPhoneId();
        int activeSvlteModeSlotId = WorldPhoneUtil.getActiveSvlteModeSlotId(); //c2k world phone
        int toModem = 0;

        logd("[setRadioCapabilityChange] majorSimId:" + majorSimId +
                " capailitySimId=" + capailitySimId);
        removeEmsrResumeByTimer();
        if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_MANUAL) {
            logd("[setRadioCapabilityChange] Auto modem selection disabled");
            removeModemStandByTimer();
        } else {
            if (sImsi[capailitySimId] == null || sImsi[capailitySimId].equals("")) {
                // may caused by receive 3g switched intent when boot up
                logd("Capaility slot IMSI not ready");
                sUserType = UNKNOWN_USER;
                return;
            }
            sUserType = getUserType(sImsi[capailitySimId]);
            if (sUserType == TYPE1_USER) {
                if (sNwPlmnStrings != null) {
                    sRegion = getRegion(sNwPlmnStrings[0]);
                }
                if (sRegion == REGION_DOMESTIC) {
                    sFirstSelect[capailitySimId] = false;
                    toModem = ModemSwitchHandler.MD_TYPE_TDD;
                } else if (sRegion == REGION_FOREIGN) {
                    sFirstSelect[capailitySimId] = false;
                    toModem = ModemSwitchHandler.MD_TYPE_FDD;
                } else {
                    logd("Unknown region");
                    return;
                }
            } else if (sUserType == TYPE2_USER || sUserType == TYPE3_USER) {
                sFirstSelect[capailitySimId] = false;
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
            MtkRIL ci = null;
            if (majorSimId != MAJOR_SIM_UNKNOWN) {
                if (WorldPhoneUtil.isCdmaLteDcSupport()) {
                    if (!isSpecialCardMode()) {
                        if (capailitySimId != activeSvlteModeSlotId) {
                            //new Radio Tech. Mode will be CSFB mode
                            logd("new RT mode is CSFB");
                            ci = sCi[majorSimId];
                        } else {
                            //[ALPS02095187]-start
                            //new RT mode is SVLTE mode
                            if (toModem == ModemSwitchHandler.MD_TYPE_LWG) {
                                logd("new RT mode is SVLTE and new type is LWG");
                                ci = sCi[majorSimId];
                            }
                            //[ALPS02095187]-end
                        }
                    } else {
                        logd("isSpecialCardMode=true, ignore this change!");
                    }
                } else {
                    ci = sCi[PhoneConstants.SIM_ID_1];
                }

                if (ci != null) {
                    ci.reloadModemType(toModem, null);
                    resetNetworkProperties();
                    WorldPhoneUtil.setSimSwitchingFlag(true);
                    WorldPhoneUtil.saveToModemType(toModem);
                    ci.storeModemType(toModem, null);
                    //sCi[PhoneConstants.SIM_ID_1].reloadModemType(toModem, null);
                }
            } else {
                //wait for radio capa. switch done
                logd("notifyRadioCapabilityChange: major sim is unknown");
            }
        }
    }

    public void handleRadioTechModeSwitch() {
        int toModem = 0;
        logd("[handleRadioTechModeSwitch]");
        if (WorldPhoneUtil.getModemSelectionMode() == SELECTION_MODE_MANUAL) {
            logd("Auto modem selection disabled");
            removeModemStandByTimer();
        } else {
            logd("Auto modem selection enabled");
            if (sImsi[sMajorSim] == null || sImsi[sMajorSim].equals("")) {
                // may caused by receive 3g switched intent when boot up
                logd("Capaility slot IMSI not ready");
                sUserType = UNKNOWN_USER;
                return;
            }
            sUserType = getUserType(sImsi[sMajorSim]);
            if (sUserType == TYPE1_USER) {
                if (sNwPlmnStrings != null) {
                    sRegion = getRegion(sNwPlmnStrings[0]);
                }
                if (sRegion == REGION_DOMESTIC) {
                    sFirstSelect[sMajorSim] = false;
                    toModem = ModemSwitchHandler.MD_TYPE_TDD;
                } else if (sRegion == REGION_FOREIGN) {
                    sFirstSelect[sMajorSim] = false;
                    toModem = ModemSwitchHandler.MD_TYPE_FDD;
                } else {
                    logd("Unknown region");
                    return;
                }
            } else if (sUserType == TYPE2_USER || sUserType == TYPE3_USER) {
                sFirstSelect[sMajorSim] = false;
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

            logd("[handleRadioTechModeSwitch]: switch type: " + toModem);
            handleSwitchModem(toModem);
            resetNetworkProperties();
        }
    }

    private boolean isNeedSwitchModem(int majorSimId) {
        boolean isNeed = true;
        if (WorldPhoneUtil.isC2kSupport()) {
            int activeSvlteModeSlotId = WorldPhoneUtil.getActiveSvlteModeSlotId();
            if ((sUserType == TYPE2_USER) &&
                    (majorSimId >= 0) && (majorSimId == activeSvlteModeSlotId)) {
                if (ModemSwitchHandler.getActiveModemType() ==  ModemSwitchHandler.MD_TYPE_LWG) {
                    isNeed = false;
                }
            }
        }
        logd("[isNeedSwitchModem] isNeed = " + isNeed);
        return isNeed;
    }

    private boolean isSpecialCardMode() {
        boolean specialCardMode = false;
        int[] cardType = WorldPhoneUtil.getC2KWPCardType();

        if ((is4GCdmaCard(cardType[0]) && is4GCdmaCard(cardType[1])) ||
                (is3GCdmaCard(cardType[0]) && is3GCdmaCard(cardType[1]))) {
            logd("isSpecialCardMode cardType1=" + cardType[0] +
                    ", cardType2=" + cardType[1]);
            specialCardMode = true;
        }
        logd("isSpecialCardMode:" + specialCardMode);
        return specialCardMode;
    }

    private boolean is4GCdmaCard(int cardType) {
        if ((cardType & WorldPhoneUtil.CARD_TYPE_USIM) > 0 &&
            containsCdma(cardType)) {
            return true;
        }
        return false;
    }

    private boolean is3GCdmaCard(int cardType) {
        if ((cardType & WorldPhoneUtil.CARD_TYPE_USIM) == 0 &&
            (cardType & WorldPhoneUtil.CARD_TYPE_SIM) == 0 &&
            containsCdma(cardType)) {
            return true;
        }
        return false;
    }

    private boolean containsCdma(int cardType) {
        if ((cardType & WorldPhoneUtil.CARD_TYPE_RUIM) > 0 ||
                (cardType & WorldPhoneUtil.CARD_TYPE_CSIM) > 0) {
            return true;
        }
        return false;
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

    private static void loadDefaultData() {
        for (String plmn : PLMN_TABLE_TYPE1) {
            sPlmnType1.add(plmn);
        }
        for (String plmn : PLMN_TABLE_TYPE1_EXT) {
            sPlmnType1Ext.add(plmn);
        }
        for (String plmn : PLMN_TABLE_TYPE3) {
            sPlmnType3.add(plmn);
        }
        for (String mcc : MCC_TABLE_DOMESTIC) {
            sMccDomestic.add(mcc);
        }
    }

    private void handleWorldPhoneTest(int fakeUserType, int fakeRegion) {
        boolean hasChanged = false;

        if ((fakeUserType == UNKNOWN_USER) && (fakeRegion == REGION_UNKNOWN)) {
            logd("Leave ADB Test mode");
            // reset default data
            sPlmnType1.clear();
            sPlmnType1Ext.clear();
            sPlmnType3.clear();
            sMccDomestic.clear();
            loadDefaultData();
        } else {
            sMajorSim = WorldPhoneUtil.getMajorSim();
            if (sMajorSim != MAJOR_SIM_UNKNOWN && sMajorSim != MAJOR_CAPABILITY_OFF) {
                String imsi =  sImsi[sMajorSim];
                if (imsi != null && !imsi.equals("")) {
                    imsi = imsi.substring(0, 5);
                    switch (fakeUserType) {
                        case TYPE1_USER:
                            sPlmnType1.add(imsi);
                            hasChanged = true;
                                break;
                        case TYPE3_USER:
                            sPlmnType3.add(imsi);
                            hasChanged = true;
                            break;
                        default:
                            logd("Unknown fakeUserType:" + fakeUserType);
                    } // switch
                } else {
                    logd("Imsi of sMajorSim is unknown");
                }

                String currentMcc = sNwPlmnStrings[0];
                if (currentMcc == null || currentMcc.equals("") || currentMcc.length() < 5) {
                    logd("Invalid sNwPlmnStrings");
                } else {
                    currentMcc = currentMcc.substring(0, 3);
                    if (fakeRegion == REGION_DOMESTIC) {
                        sMccDomestic.add(currentMcc);
                        hasChanged = true;
                    } else if (fakeRegion == REGION_FOREIGN) {
                        sMccDomestic.remove(currentMcc);
                        hasChanged = true;
                    } else {
                        logd("Unknown fakeRegion:" + fakeRegion);
                    }
                } // currentMcc == null
            } else { // sMajorSim != MAJOR_SIM_UNKNOWN
                logd("sMajorSim is Unknown or Capability OFF");
            }

            if (hasChanged) {
                logd("sPlmnType1:" + sPlmnType1);
                logd("sPlmnType1Ext:" + sPlmnType1Ext);
                logd("sPlmnType3:" + sPlmnType3);
                logd("sMccDomestic:" + sMccDomestic);
                handleRadioTechModeSwitch();
            }
        } // sMajorSim != MAJOR_SIM_UNKNOWN else
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, "[WPOM]" + msg);
    }
}
