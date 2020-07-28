/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua.service;

import android.content.Context;
import android.database.ContentObserver;
import android.net.*;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.CellLocation;
import android.telephony.PhoneStateListener;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.gsm.GsmCellLocation;
import android.text.TextUtils;
import android.util.Base64;

import com.android.ims.ImsManager;
import com.android.internal.os.SomeArgs;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.ims.rcsua.service.utils.DateUtils;

import com.mediatek.ims.rcsua.service.ril.RcsRIL;
import com.mediatek.ims.rcsua.service.utils.InetAddressUtils;
import com.mediatek.ims.rcsua.service.utils.Logger;
import com.mediatek.ims.rcsua.service.utils.Utils;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class RoiHandler {

    private boolean DEBUG_TRACE = false;
    private final boolean UT_SUPPORT = true;

    // Network Type
    private static final int NONE_NETWORK      = 0; // 2g
    private static final int GERAN_NETWORK     = 1; // 2g
    private static final int UTRAN_NETWORK     = 2; // 3g
    private static final int EUTRAN_NETWORK    = 3; // LTE
    private static final int WIFI_NETWORK      = 4; // LTE

    private static final int PROTOCOL_IP_VERSION4 = 1; // IPv4
    private static final int PROTOCOL_IP_VERSION6 = 2; // IPv6

    public static final int REASON_DO_REG    = 1;
    public static final int REASON_DO_DEREG  = 2;
    public static final int REASON_REG_DONE  = 3;

    private static volatile RoiHandler sInstance = null;
    private static Context mContext = null;
    private NetworkHelper mHelper = null;
    private ImsManager mImsManager = null;

    private RuaAdapter adapter;
    private boolean syncStart;

    private NetworkType mCurrentNetwork = NetworkType.NONE;
    private int mVolteStatus = RuaAdapter.IMS_REG_STATE_DISCONNECTED;
    private int mRcsStatus = RuaAdapter.IMS_REG_STATE_DISCONNECTED;
    private boolean mVoPS = true;
    private boolean mIsVolteEnabled = true;
    private boolean mIsVowifiEnabled = true;
    private boolean mCallOngoing = false;
    private boolean mInternetPDNConnected = false;
    private boolean mWlanConnected = false;
    private boolean mRegStarted = false;

    private RegState mRegState = RegState.IDLE;

    private static final int MSG_DO_IMS_REG       = 1;
    private static final int MSG_DO_IMS_DREG      = 2;
    private static final int MSG_DO_IMS_AUTH      = 3;
    private static final int MSG_NOTIFY_NETWORK_DOWN = 4;
    private static final int MSG_NOTIFY_RCS_START = 5;
    private static final int MSG_NOTIFY_RCS_DONE = 6;
    private static final int MSG_RCS_REG_TIMEOUT = 7;
    private static final int MSG_CALL_STATE_IDLE = 8;
    private static final int MSG_RCS_DEREG_TIMEOUT = 9;
    private static final int MSG_INIT = 101;

    private static final int CMD_RIL_SET_DIGITS_REG_STATUS_RESPONSE = 11;
    private static final int CMD_RIL_SET_DIGITS_INCOMING_LINE = 12;

    public static final int REG_OWNER_NONE   = -1;
    public static final int REG_OWNER_VOLTE  = 0;
    public static final int REG_OWNER_RCS    = 1;

    public static final int RCS_ACOUNT = 0;

    // VoLTE_Event_Network_State_*
    public static final int NETWORK_STATE_CONNECTED     = 1; // VoLTE_Event_Network_State_Connected
    public static final int NETWORK_STATE_DISCONNECTED  = 2; // VoLTE_Event_Network_State_Disconnected

    // VoLTE_Event_Network_Type_*
    public static final int VOLTE_NETWORK_TYPE_UNSPEC     = 0; // VoLTE_Event_Network_Type_Unspec
    public static final int VOLTE_NETWORK_TYPE_GERAN      = 8; // VoLTE_Event_Network_Type_GERAN
    public static final int VOLTE_NETWORK_TYPE_UTRAN      = 9; // VoLTE_Event_Network_Type_UTRAN_FDD
    public static final int VOLTE_NETWORK_TYPE_EUTRAN     = 13; // VoLTE_Event_Network_Type_E_UTRAN_FDD


    private HandlerThread mCtrlHandlerThread;
    private Handler mHandler = null;

    private int mOldRatType = VOLTE_NETWORK_TYPE_UNSPEC;
    private String mOldCellId = "";

    private final Logger logger = Logger.getLogger(this.getClass().getName());

    // Network Type
    enum NetworkType {
        NONE(0),
        GERAN(1),
        UTRAN(2),
        EUTRAN(3),
        WIFI(4);

        final int type;

        NetworkType(int type) { this.type = type; }

        boolean is23G() {
            return (this == GERAN || this == UTRAN);
        }

        boolean is3GPP() {
            return (this == GERAN || this == UTRAN || this == EUTRAN);
        }

        boolean isIEEE802() {
            return this == WIFI;
        }
    }

    enum RegState {
        IDLE,
        VOLTE_ACTIVE,
        RCS_REG_ING,
        RCS_REG_ED,
        RCS_DEREG_ING,
        RCS_DISC_ING,
        RCS_REG_ABORT;
    }

    public enum InputType {
        IMS_REGINFO_IND(1),

        NETWORK_CHANGED(2),

        VOPS_CHANGED(3),

        INTERNET_PDN_CHANGED(4),

        WLAN_STATUS_CHANGED(5),

        VOLTE_ENABLED_CHANGED(6),

        VOWIFI_ENABLED_CHANGED(7),

        VOICE_CALL_STATUS_CHANGED(8),

        RCS_REG_TIMEOUT(9),

        RCS_DEREG_TIMEOUT(10);

        final int type;
        InputType(int type) { this.type = type; }
    }

    public static class InputEvent {

        InputEvent(InputType type) { this.type = type; }

        InputType getType() {
            return type;
        }

        boolean isVoPS() {
            return vops;
        }

        NetworkType getNetwork() {
            return network;
        }

        int getImsOwner() {
            return imsOwner;
        }

        int getRegStatus() {
            return regStatus;
        }

        boolean isPdnConnected() {
            return pdnConnected;
        }

        boolean isWlanConnected() {
            return wlanConnected;
        }

        boolean isVolteEnabled() {
            return volteEnabled;
        }

        boolean isVowifiEnabled() {
            return vowifiEnabled;
        }

        boolean isCallOngoing() {
            return callOngoing;
        }

        @Override
        public String toString() {
            switch (type) {

                case IMS_REGINFO_IND:
                    return type.toString() + ": "
                            + "owner[" + imsOwner + "] + status[" + regStatus + "]";

                case NETWORK_CHANGED:
                    return type.toString() + ": " + "network[" + network + "]";

                case VOPS_CHANGED:
                    return type.toString() + ": " + "vops[" + vops + "]";

                case INTERNET_PDN_CHANGED:
                    return type.toString() + ": " + "conncted[" + pdnConnected + "]";

                case WLAN_STATUS_CHANGED:
                    return type.toString() + ": " + "connected[" + wlanConnected + "]";

                case VOLTE_ENABLED_CHANGED:
                    return type.toString() + ": " + "enabled[" + volteEnabled + "]";

                case VOWIFI_ENABLED_CHANGED:
                    return type.toString() + ": " + "enabled[" + vowifiEnabled + "]";

                case VOICE_CALL_STATUS_CHANGED:
                    return type.toString() + ": " + "ongoing[" + callOngoing + "]";

                default:
                    return type.toString();
            }
        }

        public static class Builder {

            public Builder(InputType type) { this.type = type; }

            public Builder setVoPS(boolean vops) {
                this.vops = vops;
                return this;
            }

            public Builder setNetwork(NetworkType network) {
                this.network = network;
                return this;
            }

            public Builder setImsOwner(int imsOwner) {
                this.imsOwner = imsOwner;
                return this;
            }

            public Builder setRegStatus(int regStatus) {
                this.regStatus = regStatus;
                return this;
            }

            public Builder setPdnConnected(boolean pdnConnected) {
                this.pdnConnected = pdnConnected;
                return this;
            }

            public Builder setWlanConnected(boolean wlanConnected) {
                this.wlanConnected = wlanConnected;
                return this;
            }

            public Builder setVolteEnabled(boolean volteEnabled) {
                this.volteEnabled = volteEnabled;
                return this;
            }

            public Builder setVowifiEnabled(boolean vowifiEnabled) {
                this.vowifiEnabled = vowifiEnabled;
                return this;
            }

            public Builder setCallOngoing(boolean callOngoing) {
                this.callOngoing = callOngoing;
                return this;
            }

            public InputEvent build() {
                InputEvent input = new InputEvent(type);

                input.vops = vops;
                input.network = network;
                input.imsOwner = imsOwner;
                input.regStatus = regStatus;
                input.pdnConnected = pdnConnected;
                input.wlanConnected = wlanConnected;
                input.volteEnabled = volteEnabled;
                input.vowifiEnabled = vowifiEnabled;
                input.callOngoing = callOngoing;

                return input;
            }

            private InputType type;
            private boolean vops;
            private NetworkType network = NetworkType.NONE;
            private int imsOwner = REG_OWNER_NONE;
            private int regStatus = RuaAdapter.IMS_REG_STATE_DISCONNECTED;
            private boolean pdnConnected;
            private boolean wlanConnected;
            private boolean volteEnabled;
            private boolean vowifiEnabled;
            private boolean callOngoing;
        }

        private InputType type;

        private boolean vops;

        private NetworkType network;

        private int imsOwner;

        private int regStatus;

        private boolean pdnConnected;

        private boolean wlanConnected;

        private boolean volteEnabled;

        private boolean vowifiEnabled;

        private boolean callOngoing;
    }

    public static synchronized void deleteInstance() {
        if (sInstance != null) {
            /*
             * somehow EIMSRCS=3 sent while EIMSRCS=4 not
             * this will block other IMS service registration
             * reset status here
             */
            sInstance.mHandler.obtainMessage(MSG_NOTIFY_RCS_DONE).sendToTarget();
            if (sInstance.mHelper != null) {
                sInstance.mHelper.unregisterNetworkCallback();
                sInstance.mHelper.unregisterPhoneStateListener();
                sInstance.mHelper = null;
            }
            if (sInstance.mImsSwitchChangeObserver != null) {
                mContext.getContentResolver().unregisterContentObserver(
                        sInstance.mImsSwitchChangeObserver);
                sInstance.mImsSwitchChangeObserver = null;
            }
            if (sInstance.mWfcSwitchChangeObserver != null) {
                mContext.getContentResolver().unregisterContentObserver(
                        sInstance.mWfcSwitchChangeObserver);
                sInstance.mImsSwitchChangeObserver = null;
            }
            if (sInstance.mCtrlHandlerThread != null) {
                sInstance.mCtrlHandlerThread.quitSafely();
                sInstance.mCtrlHandlerThread = null;
            }
            mContext = null;
            sInstance = null;
        }
    }

    public static synchronized RoiHandler createInstance(Context ctx) {
        if (sInstance == null) {
            mContext = ctx;
            sInstance = getInstance();
        }
        return sInstance;
    }

    public static synchronized RoiHandler getInstance() {
        if (mContext != null && sInstance == null) {
            sInstance = new RoiHandler();
        }
        return sInstance;
    }
    /*
     * construct
     */
    private RoiHandler() {
        adapter = RuaAdapter.getInstance();
        int vops = adapter.getVops();
        if (vops >= 0) {
            mVoPS = (vops == 1) ? true : false;
        }

        mCtrlHandlerThread = new HandlerThread("RegCore");
        mCtrlHandlerThread.start();
        initHandler();

        mHandler.obtainMessage(MSG_INIT).sendToTarget();

        mImsManager = ImsManager.getInstance(mContext, Utils.getMainCapabilityPhoneId());

        mIsVolteEnabled = mImsManager.isEnhanced4gLteModeSettingEnabledByUser();
        mContext.getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.ENHANCED_4G_MODE_ENABLED), true,
                mImsSwitchChangeObserver);

        mIsVowifiEnabled = mImsManager.isWfcEnabledByUser();
        mContext.getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.WFC_IMS_ENABLED), true,
                mWfcSwitchChangeObserver);

        mWlanConnected = isWifiConnected();

        // MUST initialize NetworkHelper after handlerThread starts
        // make sure network related events can be processed correctly by handlerThread
        mHelper = new NetworkHelper();
        syncStart = false;
    }


    /* Public APIs */
    public void notifyVoPSChanged(boolean vops) {
        logger.debug("previous VoPS: " + mVoPS + ", current VoPS:" + vops);
        processRegistration(new InputEvent.Builder(InputType.VOPS_CHANGED)
                .setVoPS(vops).build());
    }

    public void handleEvent(RuaAdapter.RcsUaEvent event) {

        switch (event.getRequestId()) {

            case RuaAdapter.EVENT_IMS_AUTH_REQ:
                int account_id = event.getInt();
                String nonce = event.getString(64);
                int encry_algo = event.getInt();
                int integrity_algo = event.getInt();
                int userdata = event.getInt();

                if (DEBUG_TRACE) {
                    logger.debug("account_id = " + account_id +
                            ", nonce = " + nonce +
                            ", encry_algo = " + encry_algo +
                            ", integrity_algo = " + integrity_algo +
                            ", userdata = " + userdata);
                }

                SomeArgs args = SomeArgs.obtain();
                args.argi1 = account_id;
                args.arg1 = nonce;
                args.argi2 = encry_algo;
                args.argi3 = integrity_algo;
                args.argi4 = userdata;

                mHandler.obtainMessage(MSG_DO_IMS_AUTH, args).sendToTarget();
                break;

            case RuaAdapter.EVENT_IMS_GEOLOCATION_REQ:
                break;

            case RuaAdapter.EVENT_IMS_QUERY_STATE:
                break;

            case RuaAdapter.EVENT_IMS_EMS_MODE_INFO:
                break;

            case RuaAdapter.EVENT_IMS_DIGITLING_REG_IND:
                // VOLTE_IND_ATCMD_RELAY
                // atcmd_id: VoLTE_ATCMD_DIGITLINE
                // Purpose: send DigitLine Reg response to  DigitsService
                account_id = event.getInt();
                int atcmd_id = event.getInt();
                int pad = event.getInt();
                int pad2 = event.getInt();
                String cmdline = event.getString(2028); // hard-code, IMC_ATCMD_MAX_CMDLINE_LEN (MD_ATCMD_LEN - (MD_ATCMD_PREFIX_LEN))

                if (DEBUG_TRACE) {
                    logger.debug("account_id = " + account_id +
                            ", atcmd_id = " + atcmd_id +
                            ", pad = " + pad +
                            ", pad2 = " + pad2 +
                            ", cmdline = " + cmdline);
                }
                SendDigitLineRegRsp(cmdline);
                break;
        }
    }

    public void notifyRegisterDone() {
        // Notify "IMS Registraion done" to IMS Stack
        boolean status = Utils.sendAtCommand("AT+ERCSREG=1");
        logger.debug("notifyRegisterDone: " + status);
    }

    public void notifyDeRegisterDone() {
        // Notify "IMS de-registraion done" to IMS Stack
        boolean status = Utils.sendAtCommand("AT+ERCSREG=0");
        logger.debug("notifyDeRegisterDone: " + status);
    }

    public void SendDigitLineRegRsp(String digitsinfo) {
        RcsRIL ril = adapter.getRIL();
        if (ril != null) {
            ril.setDigitsRegStatus(digitsinfo,
                mHandler.obtainMessage(CMD_RIL_SET_DIGITS_REG_STATUS_RESPONSE));
        }
    }

    public void sendIncomingDigitsLine(String fromMsisdn, String toMsisdn) {
        RcsRIL ril = adapter.getRIL();
        if (ril != null) {
            ril.setIncomingDigitsLine(fromMsisdn, toMsisdn,
                mHandler.obtainMessage(CMD_RIL_SET_DIGITS_INCOMING_LINE));
        }
    }

    public boolean isVolteActive() {
        boolean active;

        switch (mRegState) {

            case VOLTE_ACTIVE:
                active = true;
                break;

            default:
                active = false;
                break;
        }

        return active;
    }

    public boolean isRcsActive() {
        boolean active;

        switch (mRegState) {

            case RCS_REG_ING:
            case RCS_REG_ED:
            case RCS_DEREG_ING:
            case RCS_DISC_ING:
            case RCS_REG_ABORT:
                active = true;
                break;

            default:
                active = false;
                break;
        }

        return active;
    }

    /* end of Public APIs */

    /* internal APIs */
    private void initHandler() {
        mHandler = new Handler(mCtrlHandlerThread.getLooper()) {
            public void handleMessage(Message msg) {
                super.handleMessage(msg);

                switch(msg.what) {

                    case MSG_DO_IMS_REG:
                        logger.debug("do reg");

                        // Do IMS registration through Internet APN
                        adapter.sendRegNotifyImsvops(RCS_ACOUNT, mVoPS ? 1 : 0);
                        adapter.sendImsVolteConfig();
                        adapter.sendUpdateRcsServiceState(adapter.getCurrentRcsFeatures());
                        updateSimRtSettings();
                        if (!updateNetworkRtSettings()) {
                            processRegistration(new InputEvent.Builder(InputType.IMS_REGINFO_IND)
                                    .setImsOwner(REG_OWNER_RCS)
                                    .setRegStatus(RuaAdapter.IMS_REG_STATE_DISCONNECTED)
                                    .build());
                            break;
                        }
                        update3gppRateChange();
                        updateNetworkInfo(NETWORK_STATE_CONNECTED);

                        adapter.sendRegRegister();
                        break;

                    case MSG_DO_IMS_DREG:
                        logger.debug("do dereg");
                        adapter.sendRegDeregister();
                        break;

                    case MSG_DO_IMS_AUTH:
                        logger.debug("do auth");
                        SomeArgs args = (SomeArgs) msg.obj;
                        String nonce = (String)args.arg1;
                        int userdata = args.argi4;

                        RegAuthResp resp = genRespByNonce(nonce);

                        // VoLTE_Event_Auth_Result_Accept: 0
                        // VoLTE_Event_Auth_Result_Reject: 1
                        int auth_result = resp.isAuthSuccessful()? 0:1;

                        // nounce-count is calculated by RCS VoLTE Stack
                        handle401Unauthorized("00000000", auth_result, resp.getRES(),
                                resp.getCkEsp(), resp.getIkEsp(), resp.getAuts(), userdata);
                        break;

                    case MSG_NOTIFY_NETWORK_DOWN:
                        updateNetworkInfo(NETWORK_STATE_DISCONNECTED);
                        break;

                    case MSG_NOTIFY_RCS_START:
                        Utils.sendAtCommand("AT+EIMSRCS=3");
                        syncStart = true;
                        break;

                    case MSG_NOTIFY_RCS_DONE:
                        if (syncStart) {
                            Utils.sendAtCommand("AT+EIMSRCS=4");
                            syncStart = false;
                        }
                        break;

                    case MSG_RCS_REG_TIMEOUT:
                        processRegistration(new InputEvent.Builder(InputType.RCS_REG_TIMEOUT).build());
                        break;

                    case MSG_RCS_DEREG_TIMEOUT:
                        processRegistration(new InputEvent.Builder(InputType.RCS_DEREG_TIMEOUT).build());
                        break;

                    case MSG_CALL_STATE_IDLE:
                        processRegistration(new InputEvent.Builder(InputType.VOICE_CALL_STATUS_CHANGED)
                                .setCallOngoing(false)
                                .build());
                        break;

                    case CMD_RIL_SET_DIGITS_REG_STATUS_RESPONSE:
                        //setDigitsRegStatusResponse(msg);
                        break;

                    case CMD_RIL_SET_DIGITS_INCOMING_LINE:
                        break;

                    case MSG_INIT:
                        Utils.enableVopsIndication();
                        break;

                    default:
                        break;
                }
            }
        };
    }

    public boolean IsInternetPDNConnected() {
        // check Internet PDN status
        return mInternetPDNConnected;
    }

    public synchronized void processRegistration(InputEvent input) {
        logger.debug("processRegistration->"
                + "state:" + mRegState + ",input:" + input
                + ",vops:" + mVoPS + ",volteEnabled:" + mIsVolteEnabled
                + ",vowifiEnabled:" + mIsVowifiEnabled);

        boolean imsRegChanged = false;
        boolean networkChanged = false;
        boolean vopsChanged = false;
        boolean pdnChanged = false;
        boolean wlanChanged = false;
        boolean volteChanged = false;
        boolean vowifiChanged = false;
        boolean callChanged = false;
        boolean timeoutChanged = false;

        switch (input.getType()) {

            case IMS_REGINFO_IND:
                if (REG_OWNER_VOLTE == input.getImsOwner()
                        && mVolteStatus != input.getRegStatus()) {
                    mVolteStatus = input.getRegStatus();
                    imsRegChanged = true;
                } else if (REG_OWNER_RCS == input.getImsOwner()) {
                    mRcsStatus = input.getRegStatus();
                    imsRegChanged = true;
                }
                break;

            case NETWORK_CHANGED:
                if (mCurrentNetwork != input.getNetwork()) {
                    mCurrentNetwork = input.getNetwork();
                    networkChanged = true;
                }
                break;

            case VOPS_CHANGED:
                if (mVoPS != input.isVoPS()) {
                    mVoPS = input.isVoPS();
                    vopsChanged = true;
                }
                break;

            case INTERNET_PDN_CHANGED:
                if (mInternetPDNConnected != input.isPdnConnected()) {
                    mInternetPDNConnected = input.isPdnConnected();
                    pdnChanged = true;
                }
                break;

            case WLAN_STATUS_CHANGED:
                if (mWlanConnected != input.isWlanConnected()) {
                    mWlanConnected = input.isWlanConnected();
                    wlanChanged = true;
                /* Force update network here to avoid timing issue */
                    if (mCurrentNetwork.isIEEE802() && !mWlanConnected) {
                        mCurrentNetwork = NetworkType.NONE;
                        networkChanged = true;
                    }
                }
                break;

            case VOLTE_ENABLED_CHANGED:
                if (mIsVolteEnabled != input.isVolteEnabled()) {
                    mIsVolteEnabled = input.isVolteEnabled();
                    volteChanged = true;
                }
                break;

            case VOWIFI_ENABLED_CHANGED:
                if (mIsVowifiEnabled != input.isVowifiEnabled()) {
                    mIsVowifiEnabled = input.isVowifiEnabled();
                    vowifiChanged = true;
                }
                break;

            case VOICE_CALL_STATUS_CHANGED:
                if (mCallOngoing != input.isCallOngoing()) {
                    mCallOngoing = input.isCallOngoing();
                    callChanged = true;
                }
                break;

            case RCS_REG_TIMEOUT:
            case RCS_DEREG_TIMEOUT:
                timeoutChanged = true;
                break;

            default:
                logger.warn("processRegistration->Unsupported input");
                break;
        }

        if (!imsRegChanged && !networkChanged && !vopsChanged
                && !pdnChanged && !wlanChanged && !volteChanged && !vowifiChanged
                && !callChanged && !timeoutChanged) {
            /* Nothing changed, drop redundant input */
            logger.warn("processRegistration->No input need to be dealing with");
            return;
        }
        RegState oldState = mRegState;
        switch (mRegState) {

            case IDLE:
                if (InputType.INTERNET_PDN_CHANGED == input.getType()
                        || InputType.VOICE_CALL_STATUS_CHANGED == input.getType()
                        || InputType.NETWORK_CHANGED == input.getType()
                        || InputType.VOPS_CHANGED == input.getType()
                        || InputType.WLAN_STATUS_CHANGED == input.getType()
                        || InputType.VOLTE_ENABLED_CHANGED == input.getType()
                        || InputType.VOWIFI_ENABLED_CHANGED == input.getType()) {
                    if (rcsConditionSatisfied()) {
                        mHandler.obtainMessage(MSG_NOTIFY_RCS_START).sendToTarget();
                        mHandler.obtainMessage(MSG_DO_IMS_REG).sendToTarget();
                        mRegState = RegState.RCS_REG_ING;
                        mRegStarted = false;
                    }
                } else
                if (InputType.IMS_REGINFO_IND == input.getType()) {
                    if (mVolteStatus == RuaAdapter.IMS_REG_STATE_REGISTERED
                            || mVolteStatus == RuaAdapter.IMS_REG_STATE_REGISTERING
                            || mVolteStatus == RuaAdapter.IMS_REG_STATE_DEREGISTERING
                            || mVolteStatus == RuaAdapter.IMS_REG_STATE_AUTHENTICATING) {
                        mRegState = RegState.VOLTE_ACTIVE;
                    }
                }
                break;

            case VOLTE_ACTIVE:

                if (InputType.IMS_REGINFO_IND == input.getType()
                        && mVolteStatus == RuaAdapter.IMS_REG_STATE_DISCONNECTED) {
                    if (rcsConditionSatisfied()) {
                        mHandler.obtainMessage(MSG_NOTIFY_RCS_START).sendToTarget();
                        mHandler.obtainMessage(MSG_DO_IMS_REG).sendToTarget();
                        mRegState = RegState.RCS_REG_ING;
                        mRegStarted = false;
                    } else {
                        mRegState = RegState.IDLE;
                    }
                }
                break;

            case RCS_REG_ING:

                if (InputType.IMS_REGINFO_IND == input.getType()) {
                    if (mRcsStatus == RuaAdapter.IMS_REG_STATE_REGISTERED) {
                        mRegState = RegState.RCS_REG_ED;
                    } else
                    if (mRcsStatus == RuaAdapter.IMS_REG_STATE_REGISTERING) {
                        mRegStarted = true;
                    } else
                    if (mRcsStatus == RuaAdapter.IMS_REG_STATE_UNREGISTERED) {
                    /* Normal registration status changing sequence:
                        UNREGISTERED -> REGISTERING -> AUTHENTICATING -> REGISTERED
                        So UNREGISTERED received before REGISTERING is false alarm, drop it */
                        if (mRegStarted) {
                            mHandler.obtainMessage(MSG_NOTIFY_NETWORK_DOWN).sendToTarget();
                            mRegState = RegState.RCS_DISC_ING;
                            mRegStarted = false;
                        }
                    } else
                    if (mRcsStatus == RuaAdapter.IMS_REG_STATE_DISCONNECTED) {
                        mHandler.obtainMessage(MSG_NOTIFY_RCS_DONE).sendToTarget();
                        mRegState = RegState.IDLE;
                    }
                } else
                if (InputType.INTERNET_PDN_CHANGED == input.getType()
                        && !mInternetPDNConnected) {
                    mHandler.obtainMessage(MSG_NOTIFY_NETWORK_DOWN).sendToTarget();
                    mRegState = RegState.RCS_DISC_ING;
                } else
                if (InputType.NETWORK_CHANGED == input.getType()
                        || InputType.VOPS_CHANGED == input.getType()
                        || InputType.WLAN_STATUS_CHANGED == input.getType()
                        || InputType.VOLTE_ENABLED_CHANGED == input.getType()
                        || InputType.VOWIFI_ENABLED_CHANGED == input.getType()) {
                    if (volteConditionSatisfied()) {
                    /* Give some time waiting RCS registration done and delay VOLTE registration to
                      stop RCS registration gracefully for best effort. */
                        mHandler.sendEmptyMessageDelayed(MSG_RCS_REG_TIMEOUT, 2 * 1000);
                        mRegState = RegState.RCS_REG_ABORT;
                    }
                }
                break;

            case RCS_REG_ED:
                if (InputType.INTERNET_PDN_CHANGED == input.getType()
                        && !mInternetPDNConnected) {
                    mHandler.obtainMessage(MSG_NOTIFY_NETWORK_DOWN).sendToTarget();
                    mRegState = RegState.RCS_DISC_ING;
                } else
                if (InputType.NETWORK_CHANGED == input.getType()
                        || InputType.VOPS_CHANGED == input.getType()
                        || InputType.WLAN_STATUS_CHANGED == input.getType()
                        || InputType.VOLTE_ENABLED_CHANGED == input.getType()
                        || InputType.VOWIFI_ENABLED_CHANGED == input.getType()) {
                    if (volteConditionSatisfied()) {
                        mHandler.obtainMessage(MSG_DO_IMS_DREG).sendToTarget();
                        mRegState = RegState.RCS_DEREG_ING;
                    }
                } else
                if (InputType.IMS_REGINFO_IND == input.getType()
                        && (mRcsStatus == RuaAdapter.IMS_REG_STATE_UNREGISTERED
                        || mRcsStatus == RuaAdapter.IMS_REG_STATE_DISCONNECTED)) {
                    mHandler.obtainMessage(MSG_NOTIFY_RCS_DONE).sendToTarget();
                    mRegState = RegState.IDLE;
                }
                break;

            case RCS_DEREG_ING:
                if (InputType.IMS_REGINFO_IND == input.getType()) {
                    if (mRcsStatus == RuaAdapter.IMS_REG_STATE_UNREGISTERED) {
                        mHandler.obtainMessage(MSG_NOTIFY_NETWORK_DOWN).sendToTarget();
                        mRegState = RegState.RCS_DISC_ING;
                    } else if (mRcsStatus == RuaAdapter.IMS_REG_STATE_DISCONNECTED) {
                        mHandler.obtainMessage(MSG_NOTIFY_RCS_DONE).sendToTarget();
                        mRegState = RegState.IDLE;
                    }
                } else
                if (InputType.INTERNET_PDN_CHANGED == input.getType()
                        && !mInternetPDNConnected) {
                    mHandler.obtainMessage(MSG_NOTIFY_NETWORK_DOWN).sendToTarget();
                    mRegState = RegState.RCS_DISC_ING;
                }
                break;

            case RCS_REG_ABORT:
                if (InputType.RCS_REG_TIMEOUT == input.getType()) {
                    if (volteConditionSatisfied()) {
                        mHandler.obtainMessage(MSG_NOTIFY_NETWORK_DOWN).sendToTarget();
                        mRegState = RegState.RCS_DISC_ING;
                    } else {
                        mRegState = RegState.RCS_REG_ING;
                    }
                } else
                if (InputType.IMS_REGINFO_IND == input.getType()) {
                    if (mRcsStatus == RuaAdapter.IMS_REG_STATE_REGISTERED) {
                        mHandler.obtainMessage(MSG_DO_IMS_DREG).sendToTarget();
                        mRegState = RegState.RCS_DEREG_ING;
                    } else
                    if (mRcsStatus == RuaAdapter.IMS_REG_STATE_DISCONNECTED) {
                        mHandler.obtainMessage(MSG_NOTIFY_RCS_DONE).sendToTarget();
                        mRegState = RegState.IDLE;
                    }
                } else
                if (InputType.INTERNET_PDN_CHANGED == input.getType()
                        && !mInternetPDNConnected) {
                    mHandler.obtainMessage(MSG_NOTIFY_NETWORK_DOWN).sendToTarget();
                    mRegState = RegState.RCS_DISC_ING;
                }

                if (oldState != mRegState) {
                    if (mHandler.hasMessages(MSG_RCS_REG_TIMEOUT))
                        mHandler.removeMessages(MSG_RCS_REG_TIMEOUT);
                }
                break;

            case RCS_DISC_ING:
                if (InputType.IMS_REGINFO_IND == input.getType()
                        && mRcsStatus == RuaAdapter.IMS_REG_STATE_DISCONNECTED) {
                    if (rcsConditionSatisfied()) {
                        mHandler.obtainMessage(MSG_DO_IMS_REG).sendToTarget();
                        mRegState = RegState.RCS_REG_ING;
                        mRegStarted = false;
                    } else {
                        mHandler.obtainMessage(MSG_NOTIFY_RCS_DONE).sendToTarget();
                        mRegState = RegState.IDLE;
                    }
                } else
                if (InputType.IMS_REGINFO_IND == input.getType()
                        && (mVolteStatus == RuaAdapter.IMS_REG_STATE_REGISTERING
                            || mVolteStatus == RuaAdapter.IMS_REG_STATE_REGISTERED
                            || mVolteStatus == RuaAdapter.IMS_REG_STATE_AUTHENTICATING)) {
                    // This is mistake-proofing to correct ROI state, should not get here
                    // in normal senario.
                    mRegState = RegState.VOLTE_ACTIVE;
                }
                break;

            default:
                break;
        }
    }

    private boolean rcsConditionSatisfied() {
        /* Criteria for starting ROI registration:
            1. Internet PDN connected and
            2. No voice call ongoing and
            3  Wifi not connected or VoWifi setting turns off and
            4.1  Data network camp on 2/3G or
            4.2 VoPS flag change to false or
            4.3 VOLTE setting turns off
         */
        return (mInternetPDNConnected
                && !mCallOngoing
                && mCurrentNetwork.is3GPP()
                && !isVoWifiEnabled() && !isVoLteEnabled());
    }

    private boolean volteConditionSatisfied() {
        /* Criteria for starting ROI deRegistration
            either
            1.1 Camp on 4G network and
            1.2 VoPS flag change to true and
            1.3 VOLTE setting enabled
            or
            2.1 Wifi connected
            2.2 WFC setting enabled
        */
        return isVoLteEnabled() || isVoWifiEnabled();
    }

    private boolean updateNetworkRtSettings() {
        /*
            VoLTE_Setting_Net_IF_Name
            VoLTE_Setting_Net_Network_Id
            VoLTE_Setting_Net_Local_Address
            VoLTE_Setting_Net_Local_Protocol_Version
            VoLTE_Setting_Server_PCSCF_List
            VoLTE_Setting_Net_PCSCF_Number
            VoLTE_Setting_Net_Cell_ID
        */

        int networkid;
        int protocolVersion;
        String networkInterface;
        String localAddress = null;
        String[] pCscfList = null;
        int numberOfPcscf = 0;
        String cellId = mHelper.getCellId();

        Network network = mHelper.getNetwork();
        LinkProperties properties = mHelper.getLinkProperties();
        protocolVersion = PROTOCOL_IP_VERSION4;

        if (properties != null) {
            networkid = network.netId;
            networkInterface = properties.getInterfaceName();

            // Choose IPv6 address firstly
            if (properties.hasIPv6DefaultRoute()) {
                localAddress = getIpv6Address(properties.getAddresses());
                pCscfList = mHelper.getPcscfList(PROTOCOL_IP_VERSION6);
                numberOfPcscf = pCscfList.length;
                protocolVersion = PROTOCOL_IP_VERSION6;
                logger.error("IPv6 - localAddress: " + localAddress + ", P-CSCF: " + Arrays.toString(pCscfList));
            }

            // check IPv4 address if needed
            if (localAddress == null || numberOfPcscf == 0) {
                logger.error("No valid local IPv6 address");
                localAddress = getIpv4Address(properties.getAddresses());
                pCscfList = mHelper.getPcscfList(PROTOCOL_IP_VERSION4);
                numberOfPcscf = pCscfList.length;
                protocolVersion = PROTOCOL_IP_VERSION4;
                logger.error("IPv4 - localAddress: " + localAddress + ", P-CSCF: " + Arrays.toString(pCscfList));
            }

            // No valid v6 address & fallback to v4 address
            if (localAddress == null || numberOfPcscf == 0) {
                if (!UT_SUPPORT) {
                    logger.error("[Error] No valid local IPv4/v6 address!");
                    return false;
                }
                if (numberOfPcscf == 0) {
                    pCscfList = new String[6];
                    pCscfList[0] = "10.165.197.44";
                    pCscfList[1] = "10.165.197.45";
                    pCscfList[2] = "10.165.228.77";
                    pCscfList[3] = "10.165.228.78";
                    pCscfList[4] = "10.165.219.132";
                    pCscfList[5] = "10.165.219.134";
                    numberOfPcscf = 6;
                } else {
                    logger.error("[Error] No valid local IPv4/v6 address!");
                    return false;
                }
            }

            adapter.sendNetworkRtSettings(RCS_ACOUNT, networkInterface,
                    networkid, localAddress, pCscfList, numberOfPcscf, protocolVersion, cellId);
        } else {
            logger.error("properties is null in updateNetworkRtSettings");
            return false;
        }

        return true;
    }

    private String getIpv6Address(List<InetAddress> addresses) {
        for (InetAddress addr: addresses) {
            if (addr instanceof java.net.Inet6Address) {
                if (!addr.isLinkLocalAddress()) {
                    return "[" + addr.getHostAddress() + "]";
                }
            }
        }
        return null;
    }

    private String getIpv4Address(List<InetAddress> addresses) {
        for (InetAddress addr: addresses) {
            if (addr instanceof java.net.Inet4Address) {
                return addr.getHostAddress();
            }
        }
        return null;
    }

    private void updateSimRtSettings() {
        /*
            VoLTE_Setting_Account_IMEI
            VoLTE_Setting_Account_Home_URI
            VoLTE_Setting_Account_Private_UID
            VoLTE_Setting_Account_Public_UID
        */

        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        String imei = tm.getDeviceId();
        String domain = tm.getIsimDomain();
        String impi = tm.getIsimImpi();
        String[] impu = tm.getIsimImpu();

        adapter.sendSimRtSettings(RCS_ACOUNT, imei, domain, impi, impu);
    }


    public void updateNetworkInfo(int state) {

        int type;
        String cell_id = mHelper.getCellId();
        double lat = 0.0;
        double lng = 0.0;
        int acc = 0;
        String timestamp = DateUtils.encodeDate(System.currentTimeMillis());

        if (mCurrentNetwork == NetworkType.GERAN) {
            type = VOLTE_NETWORK_TYPE_GERAN;
        } else if (mCurrentNetwork == NetworkType.UTRAN) {
            type = VOLTE_NETWORK_TYPE_UTRAN;
        } else if (mCurrentNetwork == NetworkType.EUTRAN) {
            type = VOLTE_NETWORK_TYPE_EUTRAN;
        } else {
            logger.debug("It's strange!! Need to debug!!");
            type = VOLTE_NETWORK_TYPE_UNSPEC;
        }

        adapter.sendRegNetworkChange(RCS_ACOUNT, state, type, cell_id, lat, lng, acc, timestamp);
    }

    private void update3gppRateChange() {
        // VOLTE_REQ_SIPTX_REG_3GPP_RAT_CHANGE

        int old_rat_type = mOldRatType;
        String old_cell_id = mOldCellId;
        int new_rat_type;
        String new_cell_id;
        double lat = 0.0; // [TBD]
        double lng = 0.0; // [TBD]
        int acc = 0;
        String timestamp = DateUtils.encodeDate(System.currentTimeMillis());

        new_rat_type = mHelper.getRatType();
        new_cell_id = mHelper.getCellId();

        adapter.sendReg3gppRatChange(RCS_ACOUNT, old_rat_type, old_cell_id, new_rat_type,
                new_cell_id, lat, lng, acc, timestamp);

        mOldCellId = new_cell_id;
        mOldRatType = new_rat_type;
    }

    private void handle401Unauthorized(String count, int auth_result, byte[] rsp, String ck, String ik, byte[] auts, int userdata) {
        adapter.sendReqAuthReqResult(RCS_ACOUNT, auth_result, count, rsp, auts, ck==null?"":ck, ik==null?"":ik, userdata);
    }

    private byte[] processAkaAuth(byte[] rand, byte[] autn) {

        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();

        /*
         *  len:     1          16         1          16
         *          -----------------------------------------------
         *  format: |RAND-LEN | RAND DATA |AUTN-LEN | AUTH DATA   |
         *          -----------------------------------------------
         */

        int raw_data_len = rand.length + autn.length + 2;
        byte[] rawData = new byte[raw_data_len];
        int len = 0;


        rawData[len++] = RegAuthResp.RAND_LEN;
        System.arraycopy(rand, 0, rawData, len, rand.length);
        len += rand.length;
        rawData[len++] = RegAuthResp.AUTN_LEN;
        System.arraycopy(autn, 0, rawData, len, autn.length);
        String encryptedData = Base64.encodeToString(rawData, Base64.NO_WRAP);

        // AKA Authentication
        String respAkaFromSim = tm.getIccAuthentication(subId, PhoneConstants.APPTYPE_USIM,
                PhoneConstants.AUTH_CONTEXT_EAP_AKA, encryptedData);

        return Base64.decode(respAkaFromSim, Base64.DEFAULT);
    }

    private RegAuthResp genRespByNonce(String nonce) {

        byte[] decodedData = Base64.decode(nonce, Base64.DEFAULT);

        if (decodedData.length < (RegAuthResp.RAND_LEN+RegAuthResp.AUTN_LEN)) {
            logger.debug("genRespByNonce - length is less than required.");
            return null;
        }

        // Split nonce into RAND + AUTN.
        byte[] rand = new byte[RegAuthResp.RAND_LEN];
        byte[] autn = new byte[RegAuthResp.AUTN_LEN];

        System.arraycopy(decodedData, 0, rand, 0, RegAuthResp.RAND_LEN);
        System.arraycopy(decodedData, RegAuthResp.RAND_LEN, autn, 0, RegAuthResp.AUTN_LEN);

        byte[] response = processAkaAuth(rand, autn);

        if (response == null) {
            logger.debug("genRespByNonce - processAkaAuth failed");
            return null;
        }

        /*
         * The resulting AKA RES parameter is treated as a "password"
         * when calculating the response directive of RFC 2617
         */

        if (response[0] == (byte) 0xDB) {

            /* "Successful 3G authentucatuin flag" tag = 'DB */
            logger.debug("authentication succeeded");

            int res_Len = response[1];
            int ck_Len = response[res_Len+2];
            int ik_Len = response[res_Len+ck_Len+3];

            byte[] auth_RES = new byte[res_Len+1];
            byte[] auth_CK = new byte[ck_Len];
            byte[] auth_IK = new byte[ik_Len];

            auth_RES[0] = (byte) res_Len;
            System.arraycopy(response, 2, auth_RES, 1, res_Len);
            System.arraycopy(response, res_Len + 3, auth_CK, 0, ck_Len);
            System.arraycopy(response, res_Len + ck_Len + 4, auth_IK, 0, ik_Len);

            return new RegAuthResp(auth_RES, RuaAdapter.bytesToHex(auth_CK), RuaAdapter.bytesToHex(auth_IK), new byte[1]);

        } else if (response[0] == (byte) 0xDC) {

            /* "Synchronisation failure" tag = 'DC' */
            logger.error("authentication failed");

            int auts_len = response[1];
            byte[] auts = new byte[auts_len];

            System.arraycopy(response, 2 , auts, 0, auts_len);

            return new RegAuthResp(new byte[1], "", "", auts);
        } else {
            logger.error("genRespByNonce - invalid tag!");
            return null;
        }
    }

    public void sendRegDigitLineInfo(String cmdline) {

        logger.debug("cmdline: " + cmdline);
        adapter.sendRegDigitLine(RCS_ACOUNT,
                7, // VoLTE_ATCMD_DIGITLINE
                0, // pad
                0, // pad2
                cmdline);
    }

    /* end of Internal APIs */

    /*
     * Helper class to keep Authentication Response for 401 Unauthorized
     */
    private class RegAuthResp {
        public static final int RAND_LEN = 16; // 128 bits from RFC 3310
        public static final int AUTN_LEN = 16; // 128 bits from RFC 3310

        private byte[] mRes;
        private String mCkEsp = null;
        private String mIkEsp = null;
        private byte[] mAuts;

        public RegAuthResp(byte[] r, String c, String i, byte[] a) {
            if (DEBUG_TRACE) {
                logger.debug("r: " + r + ", c: " + c + ", i: " + i + ", a:" + a);
            }
            mRes = r;
            mCkEsp = c;
            mIkEsp = i;
            mAuts = a;
        }
        public byte[] getRES() {
            return mRes;
        }
        public String getCkEsp() {
            return mCkEsp;
        }
        public String getIkEsp() {
            return mIkEsp;
        }
        public byte[] getAuts() {
            return mAuts;
        }
        public boolean isAuthSuccessful() {
            return (mAuts.length == 1);
        }
    } // end of RegAuthResp Class


    /*
     * Helper class to monitor network status
     */
    private class NetworkHelper {
        private ConnectivityManager cm = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        private TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        private ConnectivityManager.NetworkCallback cellularCallback = null;
        private ConnectivityManager.NetworkCallback wifiCallback = null;
        private PhoneStateListener mPhoneStateListener = null;
        private Network mNetworkObj = null;
        private LinkProperties mLinkProp = null;
        private String[] mPcscfList;
        private String mLac = "";
        private String mCid = "";
        private String mOldPlmn = "";
        private String mCurrentPlmn = "";


        public NetworkHelper() {
            logger.debug("mVoPS = " + mVoPS);
            registerNetworkCallback();
            registerPhoneStateListener();
        }

        public Network getNetwork() {
            return mNetworkObj;
        }

        public LinkProperties getLinkProperties() {
            return mLinkProp;
        }

        public String[] getPcscfList(int protocolVersion) {
            String[] currentPcscfList;
            final List<String> resultV6 = new ArrayList<String>();
            final List<String> resultV4 = new ArrayList<String>();

            for (String pcscf : mPcscfList) {
                if (TextUtils.isEmpty(pcscf)) {
                    //null element, do nothing
                } else if (InetAddressUtils.isIPv6Address(pcscf)) {
                    resultV6.add(pcscf);
                } else {
                    resultV4.add(pcscf);
                }
            }
            if (protocolVersion == PROTOCOL_IP_VERSION6) {
                currentPcscfList = resultV6.toArray(new String[resultV6.size()]);
            } else {
                currentPcscfList = resultV4.toArray(new String[resultV4.size()]);
            }
            return currentPcscfList;
        }

        private String[] upatePcscfList() {
            String[] pcscfs = tm.getPcscfAddress(PhoneConstants.APN_TYPE_DEFAULT);

            if (pcscfs != null && pcscfs.length > 0) {
                logger.debug("getPcscfList - exists");
            } else {
                logger.debug("No pcscf found, use default!");
                String[] emptyServer = {"",""};
                pcscfs = emptyServer;
            }
            logger.debug("P-CSCF address: " + Arrays.toString(pcscfs));
            return pcscfs;
        }

        public String getCellId() {
            /*
             *   4.3.1 Cell Identity (CI) and Cell Global Identification (CGI) in TS 23.003
             *   Cell_Id = MCC + MNC + LAC + CI
             */

            String plmn = TelephonyManager.getDefault().getNetworkOperator();
            String cell_id = plmn + mLac + mCid;

            return cell_id;
        }

        public int getRatType() {

            if (mCurrentNetwork == NetworkType.GERAN) {
                return VOLTE_NETWORK_TYPE_GERAN;
            } else if (mCurrentNetwork == NetworkType.UTRAN) {
                return VOLTE_NETWORK_TYPE_UTRAN;
            } else if (mCurrentNetwork == NetworkType.EUTRAN) {
                return VOLTE_NETWORK_TYPE_EUTRAN;
            } else {
                logger.debug("It's strange!! Need to debug!!");
                return VOLTE_NETWORK_TYPE_UNSPEC;
            }
        }



        /* monitor Internet PDN status */
        private void registerNetworkCallback() {

            NetworkRequest cellularRequest = new NetworkRequest.Builder()
                    .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
                    //.addCapability(NetworkCapabilities.NET_CAPABILITY_VALIDATED)
                    .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                    .build();

            NetworkRequest wifiRequest = new NetworkRequest.Builder()
                    .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                    .addCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                    .build();

            cellularCallback = new ConnectivityManager.NetworkCallback() {

                @Override
                public void onAvailable(Network network) {
                    super.onAvailable(network);

                    logger.debug("onAvailable is called: " + network);

                    mNetworkObj = network;
                    mLinkProp = cm.getLinkProperties(mNetworkObj);
                    mPcscfList = upatePcscfList();
                    NetworkInfo networkInfo = cm.getNetworkInfo(network);
                    if (networkInfo != null && networkInfo.isConnected()) {
                        processRegistration(new InputEvent.Builder(InputType.INTERNET_PDN_CHANGED)
                                .setPdnConnected(true)
                                .build());
                    }
                }

                @Override
                public void onLost(Network network) {
                    super.onLost(network);
                    logger.debug("onLost is called: " + network);
                    processRegistration(new InputEvent.Builder(InputType.INTERNET_PDN_CHANGED)
                            .setPdnConnected(false)
                            .build());
                }
            };

            wifiCallback = new ConnectivityManager.NetworkCallback() {

                @Override
                public void onAvailable(Network network) {
                    super.onAvailable(network);

                    logger.debug("onAvailable is called: " + network);

                    NetworkInfo networkInfo = cm.getNetworkInfo(network);
                    if (networkInfo != null && networkInfo.isConnected()) {
                        processRegistration(new InputEvent.Builder(InputType.WLAN_STATUS_CHANGED)
                                .setWlanConnected(true)
                                .build());
                    }
                }

                @Override
                public void onLost(Network network) {
                    super.onLost(network);
                    logger.debug("onLost is called: " + network);
                    processRegistration(new InputEvent.Builder(InputType.WLAN_STATUS_CHANGED)
                            .setWlanConnected(false)
                            .build());
                }
            };

            cm.registerNetworkCallback(cellularRequest, cellularCallback);
            cm.registerNetworkCallback(wifiRequest, wifiCallback);
        }

        private void unregisterNetworkCallback() {
            logger.debug("unregisterNetworkCallback");
            if (cm != null) {
                if (cellularCallback != null)
                    cm.unregisterNetworkCallback(cellularCallback);
                if (wifiCallback != null) {
                    cm.unregisterNetworkCallback(wifiCallback);
                }
            }
            cellularCallback = null;
            wifiCallback = null;
        }

        private void registerPhoneStateListener() {

            mPhoneStateListener = new PhoneStateListener() {
                @Override
                public void onCellLocationChanged(CellLocation location) {
                    if (location instanceof GsmCellLocation) {
                        GsmCellLocation loc = (GsmCellLocation)location;
                        mLac = Integer.toHexString(loc.getLac());
                        mCid = Integer.toHexString(loc.getCid());
                    } else  {
                        logger.debug("un-supported CellLocation type");
                    }
                }

                @Override
                public void onCallStateChanged(int state, String incomingNumber) {
                    boolean callOngoing = false;
                    switch (state) {

                        case TelephonyManager.CALL_STATE_IDLE:
                        /* Postpone handling call idle here to deal with the case that
                         CSFB during call and back to LTE quickly after call end */
                            mHandler.sendEmptyMessageDelayed(MSG_CALL_STATE_IDLE, 2 * 1000);
                            return;

                        case TelephonyManager.CALL_STATE_RINGING:
                        case TelephonyManager.CALL_STATE_OFFHOOK:
                            if (mHandler.hasMessages(MSG_CALL_STATE_IDLE)) {
                                mHandler.removeMessages(MSG_CALL_STATE_IDLE);
                            }
                            callOngoing = true;
                            break;

                        default:
                            return;
                    }

                    processRegistration(new InputEvent.Builder(InputType.VOICE_CALL_STATUS_CHANGED)
                            .setCallOngoing(callOngoing)
                            .build());
                }

                @Override
                public void onDataConnectionStateChanged(int state, int networkType) {
                    /*
                     * UE may move from 2/3G -> LTE or LTE -> 2/3G
                     * If nternet PDN is connected, we can't update mCurrentNetwork correctly
                     * Monitor the RAT change by onDataConnectionStateChanged() then update mCurrentNetwork properly
                     *
                     */
                    logger.debug("onDataConnectionStateChanged, state: " + state + ", networkType: " + networkType);

                    NetworkType network = NetworkType.NONE;

                    switch(networkType) {
                        //2G NETWORK
                        case TelephonyManager.NETWORK_TYPE_GPRS: // 1
                        case TelephonyManager.NETWORK_TYPE_EDGE: // 2
                        case TelephonyManager.NETWORK_TYPE_CDMA:
                        case TelephonyManager.NETWORK_TYPE_1xRTT:
                        case TelephonyManager.NETWORK_TYPE_IDEN:
                            network = NetworkType.GERAN;
                            break;
                        //3G NETWORK
                        case TelephonyManager.NETWORK_TYPE_UMTS:   // 3
                        case TelephonyManager.NETWORK_TYPE_EVDO_0:
                        case TelephonyManager.NETWORK_TYPE_EVDO_A:
                        case TelephonyManager.NETWORK_TYPE_HSDPA:  // 8
                        case TelephonyManager.NETWORK_TYPE_HSUPA:  // 9
                        case TelephonyManager.NETWORK_TYPE_HSPA:   // 10
                        case TelephonyManager.NETWORK_TYPE_EVDO_B:
                        case TelephonyManager.NETWORK_TYPE_EHRPD:
                        case TelephonyManager.NETWORK_TYPE_HSPAP: // 15
                            network = NetworkType.UTRAN;
                            break;
                        //LTE NETWORK
                        case TelephonyManager.NETWORK_TYPE_LTE: // 13
                            network = NetworkType.EUTRAN;
                            break;
                        case TelephonyManager.NETWORK_TYPE_IWLAN: //18
                            network = NetworkType.WIFI;
                            break;
                        default:
                            return;
                    }

                    if (state == TelephonyManager.DATA_CONNECTED) {
                        processRegistration(new InputEvent.Builder(InputType.NETWORK_CHANGED)
                                .setNetwork(network).build());
                    }
                }
            };

            tm.listen(mPhoneStateListener,
                    PhoneStateListener.LISTEN_CELL_LOCATION
                            | PhoneStateListener.LISTEN_DATA_CONNECTION_STATE
                            | PhoneStateListener.LISTEN_CALL_STATE);
        }

        private void unregisterPhoneStateListener() {
            logger.debug("unregisterPhoneStateListener");
            if (mPhoneStateListener != null) {
                tm.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
                mPhoneStateListener = null;
            }
        }

        private int getExactNetworkType(int type, int subType) {

            if (DEBUG_TRACE) {
                logger.debug("getExactNetworkType: [" +type + ", " + subType + "]");
            }

            if ( type == ConnectivityManager.TYPE_MOBILE_IMS) { // 11
                return EUTRAN_NETWORK;
            } else if(type == ConnectivityManager.TYPE_MOBILE) { // 0

                switch (subType) {
                    //2G NETWORKS
                    case TelephonyManager.NETWORK_TYPE_GPRS: // 1
                    case TelephonyManager.NETWORK_TYPE_EDGE: // 2
                    case TelephonyManager.NETWORK_TYPE_CDMA:
                    case TelephonyManager.NETWORK_TYPE_1xRTT:
                    case TelephonyManager.NETWORK_TYPE_IDEN:
                        return GERAN_NETWORK;
                    //3G NETWORK
                    case TelephonyManager.NETWORK_TYPE_UMTS:   // 3
                    case TelephonyManager.NETWORK_TYPE_EVDO_0:
                    case TelephonyManager.NETWORK_TYPE_EVDO_A:
                    case TelephonyManager.NETWORK_TYPE_HSDPA:  // 8
                    case TelephonyManager.NETWORK_TYPE_HSUPA:  // 9
                    case TelephonyManager.NETWORK_TYPE_HSPA:   // 10
                    case TelephonyManager.NETWORK_TYPE_EVDO_B:
                    case TelephonyManager.NETWORK_TYPE_EHRPD:
                    case TelephonyManager.NETWORK_TYPE_HSPAP:
                        return UTRAN_NETWORK;
                    //LTE NETWORK
                    case TelephonyManager.NETWORK_TYPE_LTE: // 13
                        return EUTRAN_NETWORK;
                    default:
                        return NONE_NETWORK;
                }
            } else if (type == ConnectivityManager.TYPE_WIFI) {
                return WIFI_NETWORK;
            }
            return NONE_NETWORK;
        }

    } // end of NetworkHelper class


    private boolean isVoLteEnabled() {
        return mIsVolteEnabled
                && mCurrentNetwork == NetworkType.EUTRAN
                && mVoPS;
    }

    private boolean isVoWifiEnabled() {
        return mIsVowifiEnabled
                && mWlanConnected
                && mImsManager.isWfcEnabledByPlatform();
    }

    private boolean isWifiConnected() {
        ConnectivityManager cm =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        if (cm != null) {
            NetworkInfo info = cm.getActiveNetworkInfo();
            if (info != null && info.getType() == ConnectivityManager.TYPE_WIFI) {
                return info.isConnected();
            }
        }
        return false;
    }

    private ContentObserver mImsSwitchChangeObserver  = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            boolean isVolteEnabled = mImsManager.isEnhanced4gLteModeSettingEnabledByUser();

            logger.debug("Volte enable:" + isVolteEnabled);

            processRegistration(new InputEvent.Builder(InputType.VOLTE_ENABLED_CHANGED)
                    .setVolteEnabled(isVolteEnabled).build());
        }
    };

    private ContentObserver mWfcSwitchChangeObserver  = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            boolean isWfcEnabled = mImsManager.isWfcEnabledByUser();

            logger.debug("Wfc enable:" + isWfcEnabled);

            processRegistration(new InputEvent.Builder(InputType.VOWIFI_ENABLED_CHANGED)
                    .setVowifiEnabled(isWfcEnabled).build());
        }
    };

}
