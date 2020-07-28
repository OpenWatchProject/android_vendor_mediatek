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

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.internal.telephony.MtkTelephonyProperties;
import com.mediatek.internal.telephony.worldphone.WorldPhoneUtil;
import com.mediatek.internal.telephony.MtkRIL;
import vendor.mediatek.hardware.mtkradioex.V1_0.VendorSetting;

public class ModemSwitchHandler {
    private static final String LOG_TAG = "WORLDMODE";

    /** @internal */
    public static final int MD_TYPE_UNKNOWN = 0;
    /** @internal */
    public static final int MD_TYPE_WG      = 3;
    /** @internal */
    public static final int MD_TYPE_TG      = 4;
    /** @internal */
    public static final int MD_TYPE_LWG     = 5;
    /** @internal */
    public static final int MD_TYPE_LTG     = 6;
    /** @internal */
    public static final int MD_TYPE_FDD     = 100;
    /** @internal */
    public static final int MD_TYPE_TDD     = 101;
    private static final int PROJECT_SIM_NUM = WorldPhoneUtil.getProjectSimNum();

    private static int sCurrentModemType = initActiveModemType();
    private static Phone[] sProxyPhones = null;
    private static Phone[] sActivePhones = new Phone[PROJECT_SIM_NUM];
    private static Context sContext = null;
    private static CommandsInterface[] smCi = new CommandsInterface[PROJECT_SIM_NUM];
    private static MtkRIL[] sCi = new MtkRIL[PROJECT_SIM_NUM];

    private static final int EVENT_RIL_CONNECTED = 1;
    private static final int EVENT_RADIO_NOT_AVAILABLE = 2;
    private static final int MD_SWITCH_DEFAULT = 0;
    private static final int MD_SWITCH_RESET_START = 1;
    private static final int MD_SWITCH_RADIO_UNAVAILABLE = 2;
    private static int sModemSwitchingFlag = MD_SWITCH_DEFAULT;
    public static final String ACTION_MODEM_SWITCH_DONE =
            "mediatek.intent.action.ACTION_MODEM_SWITCH_DONE";
    /**
     * Broadcast Action: The modem type changed.
     */
    public static final String ACTION_MD_TYPE_CHANGE
            = "mediatek.intent.action.ACTION_MD_TYPE_CHANGE";
    public static final String EXTRA_MD_TYPE = "mdType";

    public ModemSwitchHandler() {
        logd("Constructor invoked");
        logd("Init modem type: " + sCurrentModemType);
        sProxyPhones = PhoneFactory.getPhones();
        for (int i = 0; i < PROJECT_SIM_NUM; i++) {
            sActivePhones[i] = sProxyPhones[i];
            smCi[i] = sActivePhones[i].mCi;
            sCi[i] = (MtkRIL)smCi[i];
        }
        if (PhoneFactory.getDefaultPhone() != null) {
            sContext = PhoneFactory.getDefaultPhone().getContext();
        } else {
            logd("DefaultPhone = null");
        }
    }

    /**
     * Trigger TRM to switch modem type
     * @param modemType 3 : switch to WG(MD_TYPE_WG)
     *                  4 : switch to TG(MD_TYPE_TG)
     *                  5 : switch to FDD CSFB(MD_TYPE_LWG)
     *                  6 : switch to TDD CSFB(MD_TYPE_LTG)
     */
    public static void switchModem(int modemType) {
        int protocolSim = WorldPhoneUtil.getMajorSim();
        logd("protocolSim: " + protocolSim);
        if (protocolSim >= PhoneConstants.SIM_ID_1 && protocolSim <= PhoneConstants.SIM_ID_4) {
            switchModem(sCi[protocolSim], modemType);
        } else {
            logd("switchModem protocolSim is invalid");
        }
    }

    /**
     * Trigger TRM to switch modem type
     * @param modemType 3 : switch to WG(MD_TYPE_WG)
     *                  4 : switch to TG(MD_TYPE_TG)
     *                  5 : switch to FDD CSFB(MD_TYPE_LWG)
     *                  6 : switch to TDD CSFB(MD_TYPE_LTG)
     *
     * @param isStoreModemType 1: need to store modem type
     */
    public static void switchModem(int isStoreModemType, int modemType) {
        int protocolSim = WorldPhoneUtil.getMajorSim();
        logd("protocolSim: " + protocolSim);
        if (protocolSim >= PhoneConstants.SIM_ID_1 && protocolSim <= PhoneConstants.SIM_ID_4) {
            switchModem(isStoreModemType, sCi[protocolSim], modemType);
        } else {
            logd("switchModem protocolSim is invalid");
        }
    }

    /**
     * Trigger TRM to switch modem type .
     * @param modemType 3 : switch to WG(MD_TYPE_WG)
     *                  4 : switch to TG(MD_TYPE_TG)
     *                  5 : switch to FDD CSFB(MD_TYPE_LWG)
     *                  6 : switch to TDD CSFB(MD_TYPE_LTG)
     *
     * @param ci : phone interface
     */
    public static void switchModem(MtkRIL ci, int modemType) {
        logd("[switchModem] need store modem type");
        switchModem(1, ci, modemType);
    }

    /**
     * Trigger TRM to switch modem type .
     * @param modemType 3 : switch to WG(MD_TYPE_WG)
     *                  4 : switch to TG(MD_TYPE_TG)
     *                  5 : switch to FDD CSFB(MD_TYPE_LWG)
     *                  6 : switch to TDD CSFB(MD_TYPE_LTG)
     *
     * @param isStoreModemType 1: need to store modem type
     *
     * @param ci : phone interface
     */
    public static void switchModem(int isStoreModemType, MtkRIL ci, int modemType) {
        logd("[switchModem]");
        if (ci.getRadioState() == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
            logd("Radio unavailable, can not switch modem");
            return;
        }

        sCurrentModemType = getActiveModemType();

        if (modemType == sCurrentModemType) {
            if (modemType == MD_TYPE_WG) {
                logd("Already in WG modem");
            } else if (modemType == MD_TYPE_TG) {
                logd("Already in TG modem");
            } else if (modemType == MD_TYPE_LWG) {
                logd("Already in FDD CSFB modem");
            } else if (modemType == MD_TYPE_LTG) {
                logd("Already in TDD CSFB modem");
            }
            return;
        }

        sModemSwitchingFlag = MD_SWITCH_RESET_START;
        ci.registerForNotAvailable(sWorldPhoneHandler, EVENT_RADIO_NOT_AVAILABLE, null);
        ci.registerForRilConnected(sWorldPhoneHandler, EVENT_RIL_CONNECTED, null);

        setModemType(isStoreModemType, ci, modemType);
        // Update sCurrentModemType variable & set vendor.ril.active.md system property
        setActiveModemType(modemType);

        // Broadcast modem switch notification
        logd("Broadcast intent ACTION_MD_TYPE_CHANGE");
        Intent intent = new Intent(ACTION_MD_TYPE_CHANGE);
        intent.putExtra(EXTRA_MD_TYPE, modemType);
        sContext.sendBroadcast(intent);
    }

    /**
     * Trigger RILD to switch world mode with desinated RIL instance
     * @param worldMode 0 : world mode is unknown
     * @param isStoreModemType 1 : need store modem type
     */
    private static boolean setModemType(int isStoreModemType, MtkRIL ci, int modemType) {
        if (ci.getRadioState() == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
            logd("Radio unavailable, can not switch world mode");
            return false;
        }
        if ((modemType >= MD_TYPE_WG) &&
                (modemType <= MD_TYPE_LTG)) {
            logd("silent reboot isStroeModemType=" + isStoreModemType);
            ci.reloadModemType(modemType, null);
            if (1 == isStoreModemType) {
                ci.storeModemType(modemType, null);
            }
            ((MtkRIL)ci).setVendorSetting(VendorSetting.VENDOR_SETTING_RADIO_SILENT_REBOOT,
                    Integer.toString(1), null);
            ci.restartRILD(null);
        } else {
            logd("Invalid modemType:" + modemType);
            return false;
        }
        return true;
    }

    /**
     * Trigger CCCI to reload modem bin
     * @param modemType 3 : reload WG(MD_TYPE_WG)
     *                  4 : reload TG(MD_TYPE_TG)
     *                  5 : reload FDD CSFB(MD_TYPE_LWG)
     *                  6 : reload TDD CSFB(MD_TYPE_LTG)
     */
    public static void reloadModem(int modemType) {
        int majorSim = WorldPhoneUtil.getMajorSim();
        if (majorSim >= PhoneConstants.SIM_ID_1 && majorSim <= PhoneConstants.SIM_ID_4) {
            reloadModem(sCi[majorSim], modemType);
        } else {
            logd("Invalid MajorSIM id" + majorSim);
        }
    }

    /**
     * Trigger CCCI to reload modem bin with desinated RIL instance
     * @param modemType 3 : reload WG(MD_TYPE_WG)
     *                  4 : reload TG(MD_TYPE_TG)
     *                  5 : reload FDD CSFB(MD_TYPE_LWG)
     *                  6 : reload TDD CSFB(MD_TYPE_LTG)
     */
    public static void reloadModem(MtkRIL ci, int modemType) {
        logd("[reloadModem]");
        if (ci.getRadioState() == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
            logd("Radio unavailable, can not reload modem");
            return;
        }
        ci.reloadModemType(modemType, null);
    }


    /**
     * Set reload modem type cause type.
     * @param causeType 0 : PLMN change
     *                  1 : out of service
     *                  255 : default value
     */
    public static void reloadModemCauseType(MtkRIL ci, int causeType) {
        logd("[reloadModemCauseType] " + causeType);
        ci.reloadModemType(causeType, null);
    }

    /**
     * Returns current modem type
     * @internal
     * @return 0 : modem type is unknown
     *         3 : switch to WG(MD_TYPE_WG)
     *         4 : switch to TG(MD_TYPE_TG)
     *         5 : switch to FDD CSFB(MD_TYPE_LWG)
     *         6 : switch to TDD CSFB(MD_TYPE_LTG)
     */
    public static int getActiveModemType() {
        if ((WorldPhoneUtil.isWorldPhoneSupport()) && (!WorldPhoneUtil.isWorldModeSupport())) {

        } else {
            sCurrentModemType = Integer.valueOf(
                SystemProperties.get(MtkTelephonyProperties.PROPERTY_ACTIVE_MD,
                Integer.toString(MD_TYPE_UNKNOWN)));
        }
        logd("[getActiveModemType] " + sCurrentModemType);
        return sCurrentModemType;
    }

    public static int initActiveModemType() {
        sCurrentModemType = Integer.valueOf(
            SystemProperties.get(MtkTelephonyProperties.PROPERTY_ACTIVE_MD,
            Integer.toString(MD_TYPE_UNKNOWN)));
        logd("[initActiveModemType] " + sCurrentModemType);
        return sCurrentModemType;
    }

    public static void setActiveModemType(int modemType) {
        sCurrentModemType = modemType;
        logd("[setActiveModemType] " + modemToString(sCurrentModemType));
    }

    public static boolean isModemTypeSwitching() {
        logd("[isModemTypeSwitching]: sModemSwitchingFlag = " + sModemSwitchingFlag);
        if (sModemSwitchingFlag != MD_SWITCH_DEFAULT) {
            return true;
        }
        return false;
    }

    private static Handler sWorldPhoneHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar;
            int protocolSim = WorldPhoneUtil.getMajorSim();
            logd("handleMessage msg.what=" + msg.what +
                    " sModemSwitchingFlag=" + sModemSwitchingFlag +
                    " protocolSim: " + protocolSim);
            switch (msg.what) {
                case EVENT_RIL_CONNECTED:
                    ar = (AsyncResult) msg.obj;
                    logd("[EVENT_RIL_CONNECTED] mRilVersion=" + ar.result);
                    if (sModemSwitchingFlag == MD_SWITCH_RADIO_UNAVAILABLE) {
                        sModemSwitchingFlag = MD_SWITCH_DEFAULT;
                        Intent intent = new Intent(ACTION_MODEM_SWITCH_DONE);
                        sContext.sendBroadcast(intent);
                        if (protocolSim >= PhoneConstants.SIM_ID_1 &&
                                protocolSim <= PhoneConstants.SIM_ID_4) {
                            sCi[protocolSim].unregisterForNotAvailable(sWorldPhoneHandler);
                            sCi[protocolSim].unregisterForRilConnected(sWorldPhoneHandler);
                        }
                    }
                break;
                case EVENT_RADIO_NOT_AVAILABLE:
                    sModemSwitchingFlag = MD_SWITCH_RADIO_UNAVAILABLE;
                break;
                default:
                break;
            }
        }
    };

    public static String modemToString(int modemType) {
        String modemString;
        if (modemType == MD_TYPE_WG) {
            modemString = "WG";
        } else if (modemType == MD_TYPE_TG) {
            modemString = "TG";
        } else if (modemType == MD_TYPE_LWG) {
            modemString = "FDD CSFB";
        } else if (modemType == MD_TYPE_LTG) {
            modemString = "TDD CSFB";
        } else if (modemType == MD_TYPE_UNKNOWN) {
            modemString = "UNKNOWN";
        } else {
            modemString = "Invalid modem type";
        }

        return modemString;
    }

    private static void logd(String msg) {
        Rlog.d(LOG_TAG, "[MSH]" + msg);
    }
}

