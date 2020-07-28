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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.internal.telephony.MtkTelephonyProperties;
import com.mediatek.internal.telephony.MtkRIL;

public class WorldMode extends Handler{
    private static final String LOG_TAG = "WORLDMODE";

    public static final int MD_WORLD_MODE_UNKNOWN = 0;
    public static final int MD_WORLD_MODE_LTG     = 8;   //uLTG
    public static final int MD_WORLD_MODE_LWG     = 9;   //uLWG
    public static final int MD_WORLD_MODE_LWTG    = 10;  //uLWTG
    public static final int MD_WORLD_MODE_LWCG    = 11;  //uLWCG
    public static final int MD_WORLD_MODE_LWCTG   = 12;  //uLWTCG(Auto mode)
    public static final int MD_WORLD_MODE_LTTG    = 13;  //LtTG
    public static final int MD_WORLD_MODE_LFWG    = 14;  //LfWG
    public static final int MD_WORLD_MODE_LFWCG   = 15;  //uLfWCG
    public static final int MD_WORLD_MODE_LCTG    = 16;  //uLCTG
    public static final int MD_WORLD_MODE_LTCTG   = 17;  //uLtCTG
    public static final int MD_WORLD_MODE_LTWG    = 18;  //uLtWG
    public static final int MD_WORLD_MODE_LTWCG   = 19;  //uLTWCG
    public static final int MD_WORLD_MODE_LFTG    = 20;  //uLfTG
    public static final int MD_WORLD_MODE_LFCTG   = 21;  //uLfCTG

    public static final int MD_WM_CHANGED_UNKNOWN = -1;
    public static final int MD_WM_CHANGED_START   = 0;
    public static final int MD_WM_CHANGED_END     = 1;

    static final int EVENT_RADIO_ON_1 = 1;
    static final int EVENT_RADIO_ON_2 = 2;
    static final int EVENT_RADIO_ON_3 = 3;
    static final int EVENT_RADIO_ON_4 = 4;

    /* bitmask */
    /* the defination must be sync with ratconfig.c */
    public static final int MASK_CDMA    = (1 << 5);
    public static final int MASK_LTEFDD  = (1 << 4);
    public static final int MASK_LTETDD  = (1 << 3);
    public static final int MASK_WCDMA   = (1 << 2);
    public static final int MASK_TDSCDMA = (1 << 1);
    public static final int MASK_GSM     = (1);

    // World mode result cause for EN.
    static final int WORLD_MODE_RESULT_SUCCESS            = 100;
    static final int WORLD_MODE_RESULT_ERROR              = 101;
    static final int WORLD_MODE_RESULT_WM_ID_NOT_SUPPORT  = 102;

    static final String ACTION_ADB_SWITCH_WORLD_MODE =
               "android.intent.action.ACTION_ADB_SWITCH_WORLD_MODE";
    static final String EXTRA_WORLDMODE = "worldMode";

    /**
     * Broadcast Action: The world mode changed.
     */
    public static final String ACTION_WORLD_MODE_CHANGED
            = "mediatek.intent.action.ACTION_WORLD_MODE_CHANGED";
    /**
     * Broadcast world mode change state.
     */
    public static final String EXTRA_WORLD_MODE_CHANGE_STATE = "worldModeState";

    /** The singleton instance. */
    private static WorldMode sInstance;

    private static final int PROJECT_SIM_NUM = WorldPhoneUtil.getProjectSimNum();
    private static int sCurrentWorldMode = updateCurrentWorldMode();
    private static int sActiveWorldMode = MD_WORLD_MODE_UNKNOWN;
    private static boolean sSwitchingState = false;
    private static Phone[] sProxyPhones = null;
    private static Phone[] sActivePhones = new Phone[PROJECT_SIM_NUM];
    private static Context sContext = null;
    private static CommandsInterface[] smCi = new CommandsInterface[PROJECT_SIM_NUM];
    private static MtkRIL[] sCi = new MtkRIL[PROJECT_SIM_NUM];
    private static int sUpdateSwitchingFlag = 0;

    public WorldMode() {
        logd("Constructor Init world mode: " + sCurrentWorldMode +
                "sSwitchingState: " + sSwitchingState);
        sProxyPhones = PhoneFactory.getPhones();
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            sActivePhones[i] = sProxyPhones[i];
            smCi[i] = sActivePhones[i].mCi;
            sCi[i] = (MtkRIL)smCi[i];
            sCi[i].registerForOn(this, EVENT_RADIO_ON_1 + i, null);
        }

        IntentFilter intentFilter = new IntentFilter(ACTION_WORLD_MODE_CHANGED);
        intentFilter.addAction(ACTION_ADB_SWITCH_WORLD_MODE);

        if (PhoneFactory.getDefaultPhone() != null) {
            sContext = PhoneFactory.getDefaultPhone().getContext();
        } else {
            logd("DefaultPhone = null");
        }
        sContext.registerReceiver(mWorldModeReceiver, intentFilter);
    }

    /**
     * Initialize the singleton WorldMode instance.
     * This is only done once, at startup, from PhoneFactory.makeDefaultPhone().
     */
    public static void init() {
        synchronized (WorldMode.class) {
            if (sInstance == null) {
                sInstance = new WorldMode();
            } else {
                logd("init() called multiple times!  sInstance = " + sInstance);
            }
        }
    }

    private final BroadcastReceiver mWorldModeReceiver = new  BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            logd("[Receiver]+");
            String action = intent.getAction();
            logd("Action: " + action);
            int wmState = MD_WM_CHANGED_UNKNOWN;

            if (ACTION_WORLD_MODE_CHANGED.equals(action)) {
                wmState = intent.getIntExtra(EXTRA_WORLD_MODE_CHANGE_STATE, MD_WM_CHANGED_UNKNOWN);
                logd("wmState: " + wmState);
                if (wmState == MD_WM_CHANGED_END) {
                    sCurrentWorldMode = updateCurrentWorldMode();
                }
            } else if (ACTION_ADB_SWITCH_WORLD_MODE.equals(action)){
                int toMode = intent.getIntExtra(EXTRA_WORLDMODE, MD_WORLD_MODE_UNKNOWN);
                logd("toMode: " + toMode);
                if (toMode >= MD_WORLD_MODE_LTG && toMode <= MD_WORLD_MODE_LFCTG) {
                    setWorldMode(toMode);
                }
            }
            logd("[Receiver]-");
        }
    };

    public void handleMessage(Message msg) {
        AsyncResult ar = (AsyncResult) msg.obj;
        int protocolSim = WorldPhoneUtil.getMajorSim();
        switch (msg.what) {
            case EVENT_RADIO_ON_1:
                logd("handleMessage : <EVENT_RADIO_ON_1>");
                protocolSim = WorldPhoneUtil.getMajorSim();
                if (protocolSim == PhoneConstants.SIM_ID_1) {
                    sCurrentWorldMode = updateCurrentWorldMode();
                }
                break;
            case EVENT_RADIO_ON_2:
                logd("handleMessage : <EVENT_RADIO_ON_2>");
                protocolSim = WorldPhoneUtil.getMajorSim();
                if (protocolSim == PhoneConstants.SIM_ID_2) {
                    sCurrentWorldMode = updateCurrentWorldMode();
                }
                break;
            case EVENT_RADIO_ON_3:
                logd("handleMessage : <EVENT_RADIO_ON_3>");
                protocolSim = WorldPhoneUtil.getMajorSim();
                if (protocolSim == PhoneConstants.SIM_ID_3) {
                    sCurrentWorldMode = updateCurrentWorldMode();
                }
                break;
            case EVENT_RADIO_ON_4:
                logd("handleMessage : <EVENT_RADIO_ON_4>");
                protocolSim = WorldPhoneUtil.getMajorSim();
                if (protocolSim == PhoneConstants.SIM_ID_4) {
                    sCurrentWorldMode = updateCurrentWorldMode();
                }
                break;
            default:
                logd("Unknown msg:" + msg.what);
        }
    }

    private static boolean checkWmCapability(int worldMode, int bnadMode) {
        int iRat = 0;
        if (worldMode == MD_WORLD_MODE_LTG) {
            iRat = (MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LTTG) {
            iRat = (MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LWTG) {
            iRat = (MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LFWG) {
            iRat = (MASK_LTEFDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LWG) {
            iRat = (MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LWCTG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LCTG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LTCTG) {
            iRat = (MASK_CDMA |
                    MASK_LTETDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LFWCG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LWCG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LTWG) {
            iRat = (MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LTWCG) {
            iRat = (MASK_CDMA |
                    MASK_LTETDD |
                    MASK_WCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LFTG) {
            iRat = (MASK_LTEFDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        } else if (worldMode == MD_WORLD_MODE_LFCTG) {
            iRat = (MASK_CDMA |
                    MASK_LTEFDD |
                    MASK_TDSCDMA |
                    MASK_GSM);
        }
        if (true == WorldPhoneUtil.isC2kSupport()) {
            bnadMode = bnadMode | MASK_CDMA;
        }
        if (true == WorldPhoneUtil.isWorldPhoneSupport() &&
            (MASK_WCDMA == (iRat & MASK_WCDMA) || MASK_TDSCDMA == (iRat & MASK_TDSCDMA))) {
            bnadMode = bnadMode | MASK_WCDMA | MASK_TDSCDMA;
        }

        logd("checkWmCapability: modem=" + worldMode + " rat=" + iRat + " bnadMode=" + bnadMode);
        /*  1. iRat bit mask sholud be a subset of bandMode.
         *  2. C2K should be support in worldMode and bandMode at the same time.
         */
        if (iRat == (iRat & bnadMode) &&
                (iRat & MASK_CDMA) == (bnadMode & MASK_CDMA)) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Trigger RILD to switch world mode.
     * @param worldMode 0 : world mode is unknown
     *                  8 : uTLG (MD_WORLD_MODE_LTG)
     *                  9 : uLWG (MD_WORLD_MODE_LWG)
     *                  10 : uLWTG (MD_WORLD_MODE_LWTG)
     *                  11 : uLWCG (MD_WORLD_MODE_LWCG)
     *                  12 : uLWTCG (MD_WORLD_MODE_LWCTG)
     *                  13 : LtTG (MD_WORLD_MODE_LTTG)
     *                  14 : LfWG (MD_WORLD_MODE_LFWG)
     *                  15 : uLfWCG (MD_WORLD_MODE_LFWCG)
     *                  16 : uLCTG (MD_WORLD_MODE_LCTG)
     *                  17 : uLtCTG (MD_WORLD_MODE_LTCTG)
     *                  18 : uLtWG (MD_WORLD_MODE_LTWG)
     *                  19 : uLtWCG (MD_WORLD_MODE_LTWCG)
     *                  20 : uLfTG (MD_WORLD_MODE_LFTG)
     *                  21 : uLfCTG (MD_WORLD_MODE_LFCTG)
     *
     * @param bandMode  (1 << 5) : MASK_CDMA
     *                  (1 << 4) : MASK_LTEFDD
     *                  (1 << 3) : MASK_LTETDD
     *                  (1 << 4) : MASK_WCDMA
     *                  (1 << 1) : MASK_TDSCDMA
     *                  (1) : MASK_GSM
     * @return WORLD_MODE_RESULT_SUCCESS if WM-ID is correct.
     */
    public static int setWorldModeWithBand(int worldMode, int bandMode) {
        if (checkWmCapability(worldMode, bandMode) == false) {
            logd("setWorldModeWithBand: not match, modem=" + worldMode + " bandMode=" + bandMode);
            return WORLD_MODE_RESULT_WM_ID_NOT_SUPPORT;
        }
        setWorldMode(worldMode);
        return WORLD_MODE_RESULT_SUCCESS;
    }

    /**
     * Trigger RILD to switch world mode
     * @param worldMode 0 : world mode is unknown
     *                  8 : uTLG (MD_WORLD_MODE_LTG)
     *                  9 : uLWG (MD_WORLD_MODE_LWG)
     *                  10 : uLWTG (MD_WORLD_MODE_LWTG)
     *                  11 : uLWCG (MD_WORLD_MODE_LWCG)
     *                  12 : uLWTCG (MD_WORLD_MODE_LWCTG)
     *                  13 : LtTG (MD_WORLD_MODE_LTTG)
     *                  14 : LfWG (MD_WORLD_MODE_LFWG)
     *                  15 : uLfWCG (MD_WORLD_MODE_LFWCG)
     *                  16 : uLCTG (MD_WORLD_MODE_LCTG)
     *                  17 : uLtCTG (MD_WORLD_MODE_LTCTG)
     *                  18 : uLtWG (MD_WORLD_MODE_LTWG)
     *                  19 : uLtWCG (MD_WORLD_MODE_LTWCG)
     *                  20 : uLfTG (MD_WORLD_MODE_LFTG)
     *                  21 : uLfCTG (MD_WORLD_MODE_LFCTG)
     */
    public static void setWorldMode(int worldMode) {
        int protocolSim = WorldPhoneUtil.getMajorSim();
        logd("[setWorldMode]protocolSim: " + protocolSim);
        if (protocolSim >= PhoneConstants.SIM_ID_1 &&
                protocolSim <= PhoneConstants.SIM_ID_4) {
            setWorldMode(sCi[protocolSim], worldMode);
        } else {
            setWorldMode(sCi[PhoneConstants.SIM_ID_1], worldMode);
        }
    }

    /**
     * Trigger RILD to switch world mode with desinated RIL instance
     * @param worldMode 0 : world mode is unknown
     *                  8 : uTLG (MD_WORLD_MODE_LTG)
     *                  9 : uLWG (MD_WORLD_MODE_LWG)
     *                  10 : uLWTG (MD_WORLD_MODE_LWTG)
     *                  11 : uLWCG (MD_WORLD_MODE_LWCG)
     *                  12 : uLWTCG (MD_WORLD_MODE_LWCTG)
     *                  13 : LtTG (MD_WORLD_MODE_LTTG)
     *                  14 : LfWG (MD_WORLD_MODE_LFWG)
     *                  15 : uLfWCG (MD_WORLD_MODE_LFWCG)
     *                  16 : uLCTG (MD_WORLD_MODE_LCTG)
     *                  17 : uLtCTG (MD_WORLD_MODE_LTCTG)
     *                  18 : uLtWG (MD_WORLD_MODE_LTWG)
     *                  19 : uLtWCG (MD_WORLD_MODE_LTWCG)
     *                  20 : uLfTG (MD_WORLD_MODE_LFTG)
     *                  21 : uLfCTG (MD_WORLD_MODE_LFCTG)
     */
    private static void setWorldMode(MtkRIL ci, int worldMode) {
        logd("[setWorldMode] worldMode=" + worldMode);
        if (worldMode == sCurrentWorldMode) {
            if (worldMode == MD_WORLD_MODE_LTG) {
                logd("Already in uTLG mode");
            } else if (worldMode == MD_WORLD_MODE_LWG) {
                logd("Already in uLWG mode");
            } else if (worldMode == MD_WORLD_MODE_LWTG) {
                logd("Already in uLWTG mode");
            } else if (worldMode == MD_WORLD_MODE_LWCG) {
                logd("Already in uLWCG mode");
            } else if (worldMode == MD_WORLD_MODE_LWCTG) {
                logd("Already in uLWTCG mode");
            } else if (worldMode == MD_WORLD_MODE_LTTG) {
                logd("Already in LtTG mode");
            } else if (worldMode == MD_WORLD_MODE_LFWG) {
                logd("Already in LfWG mode");
            } else if (worldMode == MD_WORLD_MODE_LFWCG) {
                logd("Already in uLfWCG mode");
            } else if (worldMode == MD_WORLD_MODE_LCTG) {
                logd("Already in uLCTG mode");
            } else if (worldMode == MD_WORLD_MODE_LTCTG) {
                logd("Already in uLtCTG mode");
            } else if (worldMode == MD_WORLD_MODE_LTWG) {
                logd("Already in uLtWG mode");
            } else if (worldMode == MD_WORLD_MODE_LTWCG) {
                logd("Already in uLtWCG mode");
            } else if (worldMode == MD_WORLD_MODE_LFTG) {
                logd("Already in uLfTG mode");
            } else if (worldMode == MD_WORLD_MODE_LFCTG) {
                logd("Already in uLfCTG mode");
            }
            return;
        }
        if (ci.getRadioState() ==
                TelephonyManager.RADIO_POWER_UNAVAILABLE) {
            logd("Radio unavailable, can not switch world mode");
            return;
        }
        if ((worldMode >= MD_WORLD_MODE_LTG) &&
                (worldMode <= MD_WORLD_MODE_LFCTG)){
            ci.reloadModemType(worldMode, null);
            ci.storeModemType(worldMode, null);
            ci.restartRILD(null);
        } else {
            logd("Invalid world mode:" + worldMode);
            return;
        }
    }

    /**
     * Returns modem world mode
     *
     * @return 0 : world mode is unknown
     *         8 : uTLG (MD_WORLD_MODE_LTG)
     *         9 : uLWG (MD_WORLD_MODE_LWG)
     *         10 : uLWTG (MD_WORLD_MODE_LWTG)
     *         11 : uLWCG (MD_WORLD_MODE_LWCG)
     *         12 : uLWTCG (MD_WORLD_MODE_LWCTG)
     *         13 : LtTG (MD_WORLD_MODE_LTTG)
     *         14 : LfWG (MD_WORLD_MODE_LFWG)
     *         15 : uLfWCG (MD_WORLD_MODE_LFWCG)
     *         16 : uLCTG (MD_WORLD_MODE_LCTG)
     *         17 : uLtCTG (MD_WORLD_MODE_LTCTG)
     *         18 : uLtWG (MD_WORLD_MODE_LTWG)
     *         19 : uLtWCG (MD_WORLD_MODE_LTWCG)
     *         20 : uLfTG (MD_WORLD_MODE_LFTG)
     *         21 : uLfCTG (MD_WORLD_MODE_LFCTG)
     */
    public static int getWorldMode() {
        sCurrentWorldMode = Integer.valueOf(
                SystemProperties.get(MtkTelephonyProperties.PROPERTY_ACTIVE_MD,
                Integer.toString(MD_WORLD_MODE_UNKNOWN)));
        logd("getWorldMode=" + WorldModeToString(sCurrentWorldMode));
        return sCurrentWorldMode;
    }

    /**
     * Returns modem world mode
     *
     * @return 0 : world mode is unknown
     *         8 : uTLG (MD_WORLD_MODE_LTG)
     *         9 : uLWG (MD_WORLD_MODE_LWG)
     *         10 : uLWTG (MD_WORLD_MODE_LWTG)
     *         11 : uLWCG (MD_WORLD_MODE_LWCG)
     *         12 : uLWTCG (MD_WORLD_MODE_LWCTG)
     *         13 : LtTG (MD_WORLD_MODE_LTTG)
     *         14 : LfWG (MD_WORLD_MODE_LFWG)
     *         15 : uLfWCG (MD_WORLD_MODE_LFWCG)
     *         16 : uLCTG (MD_WORLD_MODE_LCTG)
     *         17 : uLtCTG (MD_WORLD_MODE_LTCTG)
     *         18 : uLtWG (MD_WORLD_MODE_LTWG)
     *         19 : uLtWCG (MD_WORLD_MODE_LTWCG)
     *         20 : uLfTG (MD_WORLD_MODE_LFTG)
     *         21 : uLfCTG (MD_WORLD_MODE_LFCTG)
     */
    private static int updateCurrentWorldMode() {
        sCurrentWorldMode = Integer.valueOf(
                SystemProperties.get(MtkTelephonyProperties.PROPERTY_ACTIVE_MD,
                Integer.toString(MD_WORLD_MODE_UNKNOWN)));
        logd("updateCurrentWorldMode=" + WorldModeToString(sCurrentWorldMode));
        return sCurrentWorldMode;
    }

    /**
     * Returns state of updating state.
     *
     * @param isSwitching false : world mode switching end
     *                    true  : world mode switching start
     * @return false : update fail, updating sate is same with old state
     *         true  : update success
     */
    public static boolean updateSwitchingState(boolean isSwitching) {
        if ((false == isSwitching) && (false == isWorldModeSwitching())) {
            sUpdateSwitchingFlag++;
            logd("sUpdateSwitchingFlag+ =" + sUpdateSwitchingFlag);
            return false;
        } else if ((sUpdateSwitchingFlag > 0) && (true == isSwitching)) {
            sUpdateSwitchingFlag--;
            logd("sUpdateSwitchingFlag- =" + sUpdateSwitchingFlag);
            return false;
        }
        sSwitchingState = isSwitching;
        logd("updateSwitchingState=" + sSwitchingState);
        return true;
    }

    /**
     * Returns state of reset world mode switching state.
     *
     * @param state : world mode switching state
     *
     * @return true  : reset success
     */
    public static boolean resetSwitchingState(int state) {
        logd("reset sUpdateSwitchingFlag = " + sUpdateSwitchingFlag);
        sUpdateSwitchingFlag = 0;
        logd("reset sSwitchingState = " + sSwitchingState);
        sSwitchingState = false;

        return true;
    }

    public static boolean isWorldModeSwitching(){
        if (sSwitchingState){
            return true;
        } else {
            return false;
        }
    }

    public static String WorldModeToString(int worldMode) {
        String worldModeString;
        if (worldMode == MD_WORLD_MODE_LTG) {
            worldModeString = "uTLG";
        } else if (worldMode == MD_WORLD_MODE_LWG) {
            worldModeString = "uLWG";
        } else if (worldMode == MD_WORLD_MODE_LWTG) {
            worldModeString = "uLWTG";
        } else if (worldMode == MD_WORLD_MODE_LWCG) {
            worldModeString = "uLWCG";
        } else if (worldMode == MD_WORLD_MODE_LWCTG) {
            worldModeString = "uLWTCG";
        } else if (worldMode == MD_WORLD_MODE_LTTG) {
            worldModeString = "LtTG";
        } else if (worldMode == MD_WORLD_MODE_LFWG) {
            worldModeString = "LfWG";
        } else if (worldMode == MD_WORLD_MODE_LFWCG) {
            worldModeString = "uLfWCG";
        } else if (worldMode == MD_WORLD_MODE_LCTG) {
            worldModeString = "uLCTG";
        } else if (worldMode == MD_WORLD_MODE_LTCTG) {
            worldModeString = "uLtCTG";
        } else if (worldMode == MD_WORLD_MODE_LTWG) {
            worldModeString = "uLtWG";
        } else if (worldMode == MD_WORLD_MODE_LTWCG) {
            worldModeString = "uLtWCG";
        } else if (worldMode == MD_WORLD_MODE_LFTG) {
            worldModeString = "uLfTG";
        } else if (worldMode == MD_WORLD_MODE_LFCTG) {
            worldModeString = "uLfCTG";
        } else {
            worldModeString = "Invalid world mode";
        }

        return worldModeString;
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, "[WorldMode]" + msg);
    }
}

