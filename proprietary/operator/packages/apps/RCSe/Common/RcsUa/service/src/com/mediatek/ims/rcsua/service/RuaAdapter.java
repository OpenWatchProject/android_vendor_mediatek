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
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.HwBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.telephony.CellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;

import com.mediatek.ims.rcsua.Capability;
import com.mediatek.ims.rcsua.Configuration;
import com.mediatek.ims.rcsua.RcsUaService;
import com.mediatek.ims.rcsua.RegistrationInfo;
import com.mediatek.ims.rcsua.service.ril.RcsRIL;
import com.mediatek.ims.rcsua.service.utils.Logger;
import com.mediatek.ims.rcsua.service.utils.Utils;

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicLong;
import java.util.Date;

/**
 * The Class RcsUaAdapter.
 */
public class RuaAdapter {

    private boolean DEBUG_TRACE = false;

    private static final int BASE = 10000;

    /* request ims registartion info */
    private static final int CMD_REQ_REG_INFO                   = BASE + 1;
    /* request VoLTE stack for IMS Registration */
    private static final int CMD_IMS_REGISTER                   = BASE + 2;
    /* request VoLTE stack for IMS Deregistration */
    private static final int CMD_IMS_DEREGISTER                 = BASE + 3;
    /* send SIP request */
    static final int CMD_SEND_SIP_MSG                           = BASE + 4;
    /* [ROI]update VoPS info to volte stack */
    private static final int CMD_IMS_NOTIFY_VOPS_INFO           = BASE + 5;
    /* [ROI]update IMS settings */
    private static final int CMD_IMS_UPDATE_SETTING             = BASE + 6;
    /* [ROI]add capability for rcs service tag */
    private static final int CMD_IMS_ADD_CAPABILITY             = BASE + 7;
    /* [ROI]notify RAT change info */
    private static final int CMD_IMS_NOTIFY_RAT_CHANGE          = BASE + 8;
    /* [ROI]notify network change info */
    private static final int CMD_IMS_NOTIFY_NETWORK_CHANGE      = BASE + 9;
    /* [ROI]send back authentication request result */
    private static final int CMD_IMS_AUTH_REQ_RESULT            = BASE + 10;
    /* [ROI]send back geolocation request result */
    private static final int CMD_IMS_GEOLOCATION_REQ_RESULT     = BASE + 11;
    /* [ROI]send back query state result */
    private static final int CMD_IMS_QUERY_STATE_RESULT         = BASE + 12;
    /* [ROI]notify 3GPP RAT change info */
    private static final int CMD_IMS_NOTIFY_3GPP_RAT_CHANGE     = BASE + 13;
    /* notify rcs status to rds */
    private static final int CMD_RDS_NOTIFY_RCS_CONN_INIT       = BASE + 14;
    /* notify rcs connection status to rds */
    private static final int CMD_RDS_NOTIFY_RCS_CONN_ACTIVE     = BASE + 15;
    /* notify rcs connection status to rds */
    private static final int CMD_RDS_NOTIFY_RCS_CONN_INACTIVE   = BASE + 16;
    /* [ROI]update IMS RT settings */
    private static final int CMD_IMS_UPDATE_RT_SETTING          = BASE + 17;
    /* [ROI]update Digits Line Reg Info */
    private static final int CMD_IMS_REG_DIGITLINE              = BASE + 18;
    /* RCS service activation */
    private static final int CMD_RCS_ACTIVATION                 = BASE + 19;
    /* RCS service deactivation */
    private static final int CMD_RCS_DEACTIVATION               = BASE + 20;
    /* operator on SIM card */
    private static final int CMD_SIM_OPERATOR                   = BASE + 21;
    /*set Service Activation State, sync with rild*/
    private static final int CMD_SERVICE_ACTIVATION_STATE       = BASE + 22;
    /* update service feature tags, sync with rild */
    private static final int CMD_UPDATE_SERVICE_TAGS            = BASE + 23;
    private static final int CMD_IMS_UNPUBLISH_COMPLETED        = BASE + 24;
    /* active acct id for rcs */
    private static final int CMD_ACTIVE_RCS_SLOT_ID             = BASE + 25;
    private static final int CMD_IMS_RESTORATION                = BASE + 26;
    private static final int CMD_ENABLE_IMS_DEREG_SUSPEND       = BASE + 27;

    /* response ims registartion info */
    private static final int RSP_REQ_REG_INFO                   = BASE + 31;
    /* intermediate response for IMS Registration */
    private static final int RSP_IMS_REGISTERING                = BASE + 32;
    /* final response of IMS Registration */
    private static final int RSP_IMS_REGISTER                   = BASE + 33;
    /* intermediate response for IMS Deregistration */
    private static final int RSP_IMS_DEREGISTERING              = BASE + 34;
    /* final response of IMS Deregistration */
    private static final int RSP_IMS_DEREGISTER                 = BASE + 35;
    /* a SIP request response or a SIP request from server */
    private static final int RSP_EVENT_SIP_MSG                  = BASE + 36;

    /* indicate that IMS is de-registering */
    private static final int EVENT_IMS_DEREGISTER_IND           = BASE + 51;
    /* [ROI]authentication request form volte stack */
    static final int EVENT_IMS_AUTH_REQ                         = BASE + 52;
    /* [ROI]geolocation request form volte stack */
    static final int EVENT_IMS_GEOLOCATION_REQ                  = BASE + 53;
    /* [ROI]query state */
    static final int EVENT_IMS_QUERY_STATE                      = BASE + 54;
    /* [ROI]EMS mode info indication */
    static final int EVENT_IMS_EMS_MODE_INFO                    = BASE + 55;
    /* [ROI]AT command response */
    static final int EVENT_IMS_DIGITLING_REG_IND                = BASE + 56;

    // RCS over 2/3G, follow VoLTE Stack Definition
    private static final int VOLTE_MAX_CELL_ID_LENGTH        = 64;
    private static final int VOLTE_MAX_SSID_LENGTH           = 32;
    private static final int VOLTE_MAX_TIME_STAMP_LENGTH     = 32;
    private static final int VOLTE_MAX_REG_CAPABILITY_LENGTH = 256;
    private static final int VOLTE_MAX_AUTH_NC               = 12;
    private static final int VOLTE_MAX_AUTH_RESPONSE         = 256;
    private static final int VOLTE_MAX_AUTH_AUTS             = 256;
    private static final int VOLTE_MAX_AUTH_CK               = 256;
    private static final int VOLTE_MAX_AUTH_IK               = 256;
    private static final int VOLTE_MAX_AT_CMDLINE_LEN        = 2028;

    /* align with VoLTE_Event_Reg_State_e */
    final static int IMS_REG_STATE_REGISTERED       = 1;
    final static int IMS_REG_STATE_UNREGISTERED     = 2;
    final static int IMS_REG_STATE_REGISTERING      = 3;
    final static int IMS_REG_STATE_DEREGISTERING    = 4;
    final static int IMS_REG_STATE_DISCONNECTED     = 5;
    final static int IMS_REG_STATE_AUTHENTICATING   = 6;
    final static int IMS_REG_STATE_OOS              = 7;
    final static int IMS_REG_STATE_CONNECTING       = 8;

    private static final int MSG_UA_CONNECTED       = 1;
    private static final int MSG_UA_DISCONNECTED    = 2;
    private static final int MSG_SET_ROI_SUPPORT    = 4;
    private static final int MSG_SET_DEREG_SUSPEND  = 8;
    private static final int MSG_STORE_PLANI        = 16;
    private static final int MSG_SYNC_VOPS          = 32;
    private static final int MSG_HAL_WRITE_EVENT    = 100;
    private static final int MSG_HAL_INCOMING_REG   = 200;
    private static final int MSG_HAL_INCOMING_SIP   = 400;
    private static final int MSG_HAL_OUTGOING_SIP   = 800;
    private static final int MSG_HAL_INCOMING_ROI   = 1600;
    private static final int MSG_HAL_DEREG_IND      = 3200;

    /* keep same value as definition in RcsUaService */
    private static final int REG_STATE_NOT_REGISTERED = 0;
    private static final int REG_STATE_REGISTERING = 1;
    private static final int REG_STATE_REGISTERED = 2;
    private static final int REG_STATE_DEREGISTERING = 3;

    private static final int REG_MODE_IMS = 1;
    private static final int REG_MODE_INTERNET = 2;

    private RuaService service;
    //IMS specific information
    private MyRegistrationInfo regInfo = new MyRegistrationInfo();
    private SipHandler sipHandler;
    private SessionHandler sessionHdlr;

    private int incomingOwner;
    private int incomingImsState;

    /* Rcs HIDL */
    private volatile vendor.mediatek.hardware.rcs.V2_0.IRcs halv2;
    private volatile vendor.mediatek.hardware.rcs.V1_0.IRcs halv1;
    private RcsHalCallbackv2 halCallbackv2 = new RcsHalCallbackv2();
    private RcsHalCallbackv1 halCallbackv1 = new RcsHalCallbackv1();
    private final IRcsDeathRecipient halDeathRecipient = new IRcsDeathRecipient();
    private final AtomicLong halCookie = new AtomicLong(0);

    private static volatile RuaAdapter INSTANCE;

    //RCS SIP stack related details
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private UaConnectTask uaConnectTask;
    private boolean uaConnected;
    private boolean uaDisconnected;
    private HandlerThread workerThread;
    private Handler uaHandler;
    private boolean uaEnabled;
    private boolean roiSupport;
    private int deregSuspend;
    private int deregId;
    private PhoneStateListener mPhoneStateListener = null;
    private static final String RUA_SERVICE_PREFS_NAME = "rua_service_prefs";
    private static final String SEMICOLON = ";";
    private static final String EQUAL_SIGN = "=";
    private static final String UTRAN_CELL_ID = "utran-cell-id-3gpp";
    private static final String CGI = "cgi-3gpp";
    private static final String STRING_FORMAT_FOR_CGI = "%04x%04x";
    private static final String STRING_FORMAT_FOR_UTRAN = "%04x%07x";
    private static final String GERAN = "3GPP-GERAN";
    private static final String UTRAN_FDD = "3GPP-UTRAN-FDD";
    private static final String E_UTRAN_FDD = "3GPP-E-UTRAN-FDD";
    private int mMobileRegRat = ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN;
    private int mCid = -1;
    private int mLac = -1;
    private String mPlmn = "";
    private int mLastRegRat;
    private int mLastLac;
    private int mLastCid;
    private String mLastPlmn;
    private long mLastOosTime = -1;
    private RcsRIL mRil;
    private int mVops = -1;

    /*
     * RcsUaEvent for RCS UA Event Transmission
     */
    public static class RcsUaEvent {
        static final int MAX_DATA_LENGTH = 70960;
        private int requestId;
        private int dataLen;
        private int readOffset;
        private byte data[];
        private int eventmaxdataLen = MAX_DATA_LENGTH;

        RcsUaEvent(int id) {
            requestId = id;
            data = new byte[eventmaxdataLen];
            dataLen = 0;
            readOffset = 0;
        }

        RcsUaEvent(int id, int length) {
            requestId = id;
            eventmaxdataLen = length;
            data = new byte[eventmaxdataLen];
            dataLen = 0;
            readOffset = 0;
        }

        double putDouble(double value) {
            if (dataLen > eventmaxdataLen - 8) {
                return -1;
            }

            synchronized(this) {
                byte[] bytes = new byte[8];
                ByteBuffer.wrap(bytes).order(ByteOrder.LITTLE_ENDIAN).putDouble(value);

                for (int i = 0; i < 8; ++i) {
                    data[dataLen] = bytes[i];
                    dataLen++;
                }
            }
            return 0;
        }

        int putInt(int value) {
            if (dataLen > eventmaxdataLen - 4) {
                return -1;
            }
            synchronized(this) {
                for (int i = 0; i < 4; ++i) {
                    data[dataLen] = (byte)((value >> (8 * i)) & 0xFF);
                    dataLen++;
                }
            }
            return 0;
        }

        int putShort(int value) {
            if (dataLen > eventmaxdataLen - 2) {
                return -1;
            }
            synchronized(this) {
                for (int i = 0; i < 2; ++i) {
                    data[dataLen] = (byte)((value >> (8 * i)) & 0xFF);
                    dataLen++;
                }
            }
            return 0;
        }

        int putByte(int value) {
            if (dataLen > eventmaxdataLen - 1) {
                return -1;
            }

            synchronized(this) {
                data[dataLen] = (byte)(value & 0xFF);
                dataLen++;
            }
            return 0;
        }

        /**
         * Put string. Truncate last byte to 0 or fill remain with 0.
         * So reserve additional byte for your string.
         *
         */
        int putString(String str, int len) {
            if (dataLen > eventmaxdataLen - len) {
                return -1;
            }
            synchronized(this) {
                byte s[] = str.getBytes();
                if (len < str.length()) {
                    System.arraycopy(s, 0, data, dataLen, len);
                    dataLen += len;
                    data[dataLen-1] = 0;
                } else {
                    System.arraycopy(s, 0, data, dataLen, s.length);
                    Arrays.fill(data, dataLen+s.length, dataLen+len, (byte)0);
                    dataLen += len;
                }
            }
            return 0;
        }

        /**
         * Put n bytes. Only fill remain with 0.
         * User need to set last byte to 0.
         * Int/Double need to be in LITTLE_ENDIAN(host)
         *
         */
        int putNBytes(byte[] value, int len) {
            if (dataLen > eventmaxdataLen - len) {
                return -1;
            }
            synchronized(this) {
                if (len < value.length) {
                    System.arraycopy(value, 0, data, dataLen, len);
                    dataLen += len;
                } else {
                    System.arraycopy(value, 0, data, dataLen, value.length);
                    Arrays.fill(data, dataLen+value.length, dataLen+len, (byte)0);
                    dataLen += len;
                }
            }
            return 0;
        }

        int putBytes(byte[] value) {
            if (value.length > eventmaxdataLen) {
                return -1;
            }

            synchronized(this) {
                System.arraycopy(value, 0, data, dataLen, value.length);
                dataLen += value.length;
            }
            return 0;
        }

        byte[] getData() {
            return data;
        }

        byte[] getDataCopy() {
            byte[] newData = new byte[dataLen];
            System.arraycopy(data, 0, newData, 0, dataLen);
            return newData;
        }

        int getDataLen() {
            return dataLen;
        }

        int getRequestId() {
            return requestId;
        }

        public int getInt() {
            int ret = 0;
            synchronized(this) {
                ret = ((data[readOffset + 3] & 0xff) << 24 |
                    (data[readOffset + 2] & 0xff) << 16 |
                    (data[readOffset + 1] & 0xff) << 8 | (data[readOffset] & 0xff));
                readOffset += 4;
            }
            return ret;
        }

        int getShort() {
            int ret = 0;
            synchronized(this) {
                ret = ((data[readOffset + 1] & 0xff) << 8 | (data[readOffset] & 0xff));
                readOffset += 2;
            }
            return ret;
        }

        /**
         * Notice: getByte is to get int8 type from VA, not get one byte.
         */
        int getByte() {
            int ret = 0;
            synchronized(this) {
                ret = (data[readOffset] & 0xff);
                readOffset += 1;
            }
            return ret;
        }

        byte[] getBytes(int length) {
            if (length > dataLen - readOffset) {
                return null;
            }
            byte[] ret = new byte[length];
            synchronized(this) {
                for (int i = 0; i < length; i++) {
                    ret[i] = data[readOffset];
                    readOffset++;
                }
                return ret;
            }
        }

        String getString(int len) {
            byte[] buf = new byte[len];

            synchronized(this) {
                System.arraycopy(data, readOffset, buf, 0, len);
                readOffset += len;
            }

            // to fix byte array including C string end '0x00'
            int index = 0;
            while (index < len) {
               if (buf[index] == 0x00 ||
                       buf[index] == '\0') {
                   break;
               }
               index++;
            }//while

            try {
               return (new String(buf, 0, index, "US-ASCII"));
            } catch (UnsupportedEncodingException e) {
               return (new String(buf)).trim();
            }
        }

        @Override
        public String toString() {
            StringBuilder builder = new StringBuilder("UaEvent:");

            builder.append('[');
            switch (requestId) {

                case CMD_REQ_REG_INFO:
                    builder.append("CMD_REQ_REG_INFO");
                    break;

                case CMD_IMS_REGISTER:
                    builder.append("CMD_IMS_REGISTER");
                    break;

                case CMD_IMS_DEREGISTER:
                    builder.append("CMD_IMS_DEREGISTER");
                    break;

                case CMD_SEND_SIP_MSG:
                    builder.append("CMD_SEND_SIP_MSG");
                    break;

                case CMD_IMS_NOTIFY_VOPS_INFO:
                    builder.append("CMD_IMS_NOTIFY_VOPS_INFO");
                    break;

                case CMD_IMS_UPDATE_SETTING:
                    builder.append("CMD_IMS_UPDATE_SETTING");
                    break;

                case CMD_IMS_ADD_CAPABILITY:
                    builder.append("CMD_IMS_ADD_CAPABILITY");
                    break;

                case CMD_IMS_NOTIFY_RAT_CHANGE:
                    builder.append("CMD_IMS_NOTIFY_RAT_CHANGE");
                    break;

                case CMD_IMS_NOTIFY_NETWORK_CHANGE:
                    builder.append("CMD_IMS_NOTIFY_NETWORK_CHANGE");
                    break;

                case CMD_IMS_AUTH_REQ_RESULT:
                    builder.append("CMD_IMS_AUTH_REQ_RESULT");
                    break;

                case CMD_IMS_GEOLOCATION_REQ_RESULT:
                    builder.append("CMD_IMS_GEOLOCATION_REQ_RESULT");
                    break;

                case CMD_IMS_QUERY_STATE_RESULT:
                    builder.append("CMD_IMS_QUERY_STATE_RESULT");
                    break;

                case CMD_IMS_NOTIFY_3GPP_RAT_CHANGE:
                    builder.append("CMD_IMS_NOTIFY_3GPP_RAT_CHANGE");
                    break;

                case CMD_RDS_NOTIFY_RCS_CONN_INIT:
                    builder.append("CMD_RDS_NOTIFY_RCS_CONN_INIT");
                    break;

                case CMD_RDS_NOTIFY_RCS_CONN_ACTIVE:
                    builder.append("CMD_RDS_NOTIFY_RCS_CONN_ACTIVE");
                    break;

                case CMD_RDS_NOTIFY_RCS_CONN_INACTIVE:
                    builder.append("CMD_RDS_NOTIFY_RCS_CONN_INACTIVE");
                    break;

                case CMD_IMS_UPDATE_RT_SETTING:
                    builder.append("CMD_IMS_UPDATE_RT_SETTING");
                    break;

                case CMD_IMS_REG_DIGITLINE:
                    builder.append("CMD_IMS_REG_DIGITLINE");
                    break;

                case CMD_RCS_ACTIVATION:
                    builder.append("CMD_RCS_ACTIVATION");
                    break;

                case CMD_RCS_DEACTIVATION:
                    builder.append("CMD_RCS_DEACTIVATION");
                    break;

                case CMD_SIM_OPERATOR:
                    builder.append("CMD_SIM_OPERATOR");
                    break;

                case CMD_SERVICE_ACTIVATION_STATE:
                    builder.append("CMD_SERVICE_ACTIVATION_STATE");
                    break;

                case RSP_REQ_REG_INFO:
                    builder.append("RSP_REQ_REG_INFO");
                    break;

                case RSP_IMS_REGISTERING:
                    builder.append("RSP_IMS_REGISTERING");
                    break;

                case RSP_IMS_REGISTER:
                    builder.append("RSP_IMS_REGISTER");
                    break;

                case RSP_IMS_DEREGISTERING:
                    builder.append("RSP_IMS_DEREGISTERING");
                    break;

                case RSP_IMS_DEREGISTER:
                    builder.append("RSP_IMS_DEREGISTER");
                    break;

                case RSP_EVENT_SIP_MSG:
                    builder.append("RSP_EVENT_SIP_MSG");
                    break;

                case EVENT_IMS_DEREGISTER_IND:
                    builder.append("EVENT_IMS_DEREGISTER_IND");
                    break;

                case EVENT_IMS_AUTH_REQ:
                    builder.append("EVENT_IMS_AUTH_REQ");
                    break;

                case EVENT_IMS_GEOLOCATION_REQ:
                    builder.append("EVENT_IMS_GEOLOCATION_REQ");
                    break;

                case EVENT_IMS_QUERY_STATE:
                    builder.append("EVENT_IMS_QUERY_STATE");
                    break;

                case EVENT_IMS_EMS_MODE_INFO:
                    builder.append("EVENT_IMS_EMS_MODE_INFO");
                    break;

                case EVENT_IMS_DIGITLING_REG_IND:
                    builder.append("EVENT_IMS_DIGITLING_REG_IND");
                    break;

                case CMD_ACTIVE_RCS_SLOT_ID:
                    builder.append("CMD_ACTIVE_RCS_SLOT_ID");
                    break;

                case CMD_IMS_RESTORATION:
                    builder.append("CMD_IMS_RESTORATION");
                    break;

                default:
                    builder.append("UNDEF");
                    break;
            }
            builder.append(']');

            builder.append(" datalen[");
            builder.append(dataLen);
            builder.append(']');

            return builder.toString();
        }
    }

    private static class MyRegistrationInfo {
        MyRegistrationInfo() {
            stateUpdated = false;
            configUpdated = false;
            rereg = false;
            rcsTags = 0;
            info = new RegistrationInfo();
        }

        public int getRegState() {
            return info.getRegState();
        }

        public int getRadioTech() {
            return info.getRadioTech();
        }

        public int getRegMode() {
            return info.getRegMode();
        }

        public Configuration getImsConfig() {
            return info.getImsConfig();
        }

        public void setRegState(int regState) {
            this.info.setRegState(regState);
        }

        public void setRadioTech(int radioTech) {
            this.info.setRadioTech(radioTech);
        }

        public void setRadioTech(String pani) {
            final String prefixWifi = "IEEE-802.11";
            if (pani.startsWith(prefixWifi))
                this.info.setRadioTech(RcsUaService.REG_RADIO_IEEE802);
            else
                this.info.setRadioTech(RcsUaService.REG_RADIO_3GPP);
        }

        public void setRegMode(int regMode) {
            this.info.setRegMode(regMode);
        }

        public void setImsConfiguration(final Configuration imsConfig) {
            this.info.setImsConfiguration(imsConfig);
        }

        public void reset() {
            stateUpdated = false;
            configUpdated = false;
            rereg = false;
            rcsTags = 0;
            info.setRegState(RcsUaService.REG_STATE_NOT_REGISTERED);
            info.setRadioTech(RcsUaService.REG_RADIO_NONE);
            info.setRegMode(RcsUaService.REG_MODE_IMS);
            info.setImsConfiguration(new Configuration());
        }

        boolean stateUpdated;
        boolean configUpdated;
        boolean rereg;
        int rcsTags;
        RegistrationInfo info;
        String regTimestamp;
    }

    public static RuaAdapter createInstance() {
        if (INSTANCE == null) {
            synchronized (RuaAdapter.class) {
                if (INSTANCE == null) {
                    INSTANCE = new RuaAdapter();
                }
            }
        }

        return INSTANCE;
    }

    public static void destroyInstance() {
        if (INSTANCE != null) {
            if (INSTANCE.uaEnabled) {
                RoiHandler.deleteInstance();
                INSTANCE.uaConnectTask.stop();
            }
            SipHandler.destroyInstance();
            if (INSTANCE.sessionHdlr != null)
                INSTANCE.sessionHdlr.initInstanceCount();
            INSTANCE.workerThread.quitSafely();
            INSTANCE.unregisterNetworkCallback();
            INSTANCE = null;
        }
    }

    public static RuaAdapter getInstance() {
        if (INSTANCE == null) {
            synchronized (RuaAdapter.class) {
                if (INSTANCE == null) {
                    INSTANCE = new RuaAdapter();
                }
            }
        }

        return INSTANCE;
    }

    void setRoiSupport(boolean roiSupport) {
        if (this.roiSupport != roiSupport)
            Message.obtain(uaHandler, MSG_SET_ROI_SUPPORT, roiSupport).sendToTarget();
    }

    void setDeregSuspend(int deregSuspend) {
        if (this.deregSuspend != deregSuspend)
            Message.obtain(uaHandler, MSG_SET_DEREG_SUSPEND, deregSuspend).sendToTarget();
    }

    void bindService(RuaService service) {
        this.service = service;
        if (roiSupport && isRoiCapable())
            RoiHandler.createInstance(Utils.getApplicationContext());
    }

    void unbindService() {
        RoiHandler.deleteInstance();
        this.service = null;
    }

    RegistrationInfo getRegistrationInfo() {
        String pLANI = "";
        int radioTech = regInfo.getRadioTech();
        if (radioTech == RcsUaService.REG_RADIO_IEEE802) {
            if (mLastOosTime == -1) {
                mLastRegRat = Integer.parseInt(readParameter("KEY_RAT", "0"));
                mLastLac = Integer.parseInt(readParameter("KEY_LAC", "0"));
                mLastCid = Integer.parseInt(readParameter("KEY_CID", "0"));
                mLastPlmn = readParameter("KEY_PLMN", "0");
                mLastOosTime = Long.parseLong(readParameter("KEY_OOS", "0"));
                if (logger.isActivated()) {
                    logger.debug("getRegistrationInfo, load file, last rat:" + mLastRegRat);
                }
            }

            pLANI = getPLANI(mLastRegRat, mLastLac, mLastCid, mLastPlmn);
            if (pLANI != null && !pLANI.isEmpty()) {
                if ((mMobileRegRat == ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN)
                    && (mLastRegRat != ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN)) {
                    try {
                        pLANI += SEMICOLON + "\""
                            + URLEncoder.encode(toRFC3339DateFormat(mLastOosTime), "utf-8") + "\"";
                        String regTime = regInfo.regTimestamp;
                        if ((regTime != null) && !regTime.isEmpty()) {
                            pLANI += SEMICOLON + "\"" + URLEncoder.encode(regTime) + "\"";
                        }
                    } catch(UnsupportedEncodingException e) {
                        logger.debug("not support encoding" + e);
                    }
                }
            }
        }

        Configuration imsConfig = this.regInfo.getImsConfig();
        if (!pLANI.equals(imsConfig.getPLastAccessNetworkInfo())) {
            imsConfig.setPLastAccessNetworkInfo(pLANI);
        }
        return this.regInfo.info;
    }

    public String getMsisdn() {
        String[] uris = this.regInfo.getImsConfig().getPAssociatedUri();
        String  uriPicked = null;
        String msisdn = null;

        if (uris != null) {
            for (String uri : uris) {
                if (uri.startsWith("tel:")) {
                    uriPicked = uri;
                    break;
                }
                if (uri.startsWith("sip:+")) {
                    uriPicked = uri;
                } else if (uri.startsWith("sip:") && (uriPicked == null)) {
                    int length = uri.indexOf('@') - "sip:".length();
                    if (length >= 5 && length <= 13) {
                        uriPicked = uri;
                    }
                }
            }

            if (uriPicked == null)
                return null;

            try {
                if (uriPicked.indexOf('@') > 0) {
                    msisdn = uriPicked.substring(4, uriPicked.indexOf('@'));
                } else if(uriPicked.indexOf(';') > 0) {
                    msisdn = uriPicked.substring(4, uriPicked.indexOf(';'));
                } else {
                    msisdn = uriPicked.substring(4);
                }

                if (msisdn.charAt(0) != '+')
                    msisdn = "+" + msisdn;
            } catch (StringIndexOutOfBoundsException e) {
                e.printStackTrace();
            }
        }

        return msisdn;
    }

    boolean isAvailable() {
        return uaEnabled && uaConnected;
    }

    synchronized void enable() {

        logger.debug("enable with uaEnabled[" + uaEnabled + "]");

        if (!uaEnabled) {
            if (uaConnectTask == null)
                uaConnectTask = new UaConnectTask();

            uaConnectTask.start();

            uaEnabled = true;

            if (sessionHdlr == null) {
                if (Utils.isSimSupported(7))
                    sessionHdlr = SessionHandler.getInstance();
            } else {
                if (!Utils.isSimSupported(7))
                    sessionHdlr = null;
            }
            if (roiSupport && isRoiCapable())
                RoiHandler.createInstance(Utils.getApplicationContext());
        }
    }

    synchronized void disable() {

        logger.debug("disable with uaEnabled[" + uaEnabled + "]");

        if (uaEnabled) {
            RoiHandler.deleteInstance();
            uaConnectTask.stop();
            uaEnabled = false;
        }
    }

    boolean isRoiCapable() {
        return Utils.isForceRoiSupport()
                || (!Utils.isTestSim() &&
                (Utils.isOP08Supported() || Utils.isSimSupported(8)));
    }

    void notifySessionIncreased() {
        logger.debug("notifySessionIncreased");
        if (sessionHdlr != null)
            sessionHdlr.incrementInstanceCount();
    }

    void notifySessionDecreased() {
        logger.debug("notifySessionDecreased");
        if (sessionHdlr != null)
            sessionHdlr.decrementInstanceCount();
    }

    private RuaAdapter() {

        workerThread = new HandlerThread("UaWorkerThread");
        workerThread.start();
        uaHandler = new Handler(workerThread.getLooper(), new UaHandler());

        sipHandler = SipHandler.createInstance(this);

        if (Utils.isSimSupported())
            enable();

        registerNetworkCallback();

        /* Initialize RIL HILD */
        //mRadioIndication = new RcsRadioIndication();
        //mRadioResponse = new RcsRadioResponse(this, instanceId);

        logger.debug("RuaAdapter, init RCSRIL");
        mRil = new RcsRIL(Utils.getApplicationContext(), Utils.getMainCapabilityPhoneId());
        Message.obtain(uaHandler, MSG_SYNC_VOPS).sendToTarget();
    }

    class UaHandler implements Handler.Callback {

        @Override
        public boolean handleMessage(Message msg) {
            logger.debug("UaHandler handle Message:" + msg.what);
            switch (msg.what) {

                case MSG_UA_CONNECTED:
                    sendRegistrationInfoRequest();
                    sendSimOperator();
                    // Always reset ROI status when UA get connected
                    if (RoiHandler.getInstance() == null)
                        Utils.sendAtCommand("AT+EIMSRCS=4");
                    else if (uaDisconnected) {
                        RoiHandler.deleteInstance();
                        if (roiSupport && isRoiCapable())
                            RoiHandler.createInstance(Utils.getApplicationContext());
                        uaDisconnected = false;
                    }
                    uaConnected = true;
                    break;

                case MSG_UA_DISCONNECTED:
                    uaConnected = false;
                    uaDisconnected = true;
                    regInfo.reset();
                    if (uaEnabled)
                        uaConnectTask.start();
                    break;

                case MSG_SET_ROI_SUPPORT:
                    boolean support = (boolean)msg.obj;
                    if (!support) {
                        RoiHandler.deleteInstance();
                    }
                    else if (isRoiCapable()) {
                        RoiHandler.createInstance(Utils.getApplicationContext());
                    }
                    roiSupport = support;
                    break;

                case MSG_SET_DEREG_SUSPEND:
                    deregSuspend = (int)msg.obj;
                    enableImsDeregSuspend(deregSuspend);
                    break;

                case MSG_HAL_INCOMING_REG:
                    handleRegistrationInfo((RcsUaEvent)msg.obj);
                    break;

                case MSG_HAL_DEREG_IND:
                    handleDeregIndication((RcsUaEvent)msg.obj);
                    break;

                case MSG_HAL_INCOMING_ROI:
                    if (RoiHandler.getInstance() != null) {
                        RoiHandler.getInstance().handleEvent((RcsUaEvent)msg.obj);
                    }
                    break;

                case MSG_HAL_WRITE_EVENT:
                    sendMsgToRCSUAProxy((RcsUaEvent)msg.obj);
                    break;

                case MSG_STORE_PLANI:
                    if (mLastRegRat > ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN) {
                        writeParameter("KEY_RAT", String.valueOf(mLastRegRat));
                        writeParameter("KEY_LAC", String.valueOf(mLastLac));
                        writeParameter("KEY_CID", String.valueOf(mLastCid));
                        writeParameter("KEY_PLMN", mLastPlmn);
                        writeParameter("KEY_OOS", String.valueOf(mLastOosTime));
                    }
                    if (logger.isActivated()) {
                        logger.debug("MSG_STORE_PLANI regRat:" + mLastRegRat
                                     + ",OosTime:" + mLastOosTime);
                    }
                    break;

                case MSG_SYNC_VOPS:
                    boolean vops = Utils.isVopsOn();
                    if (logger.isActivated()) {
                        logger.debug("MSG_SYNC_VOPS, vops:" + vops);
                    }
                    notifyVopsIndication(vops ? 1 : 0);
                    break;

                default:
                    break;
            }

            return true;
        }
    }

    public SipHandler getSipHandler() {
        return sipHandler;
    }

    public boolean sendSipMessage(byte[] message, int id) {
        if (isAvailable()) {
            sipHandler.sendMessage(message, id);
            return true;
        }

        return false;
    }

    void resumeImsDeregistration() {
        if (deregSuspend > 0 && deregId > 0) {
            RcsUaEvent event = new RcsUaEvent(CMD_IMS_UNPUBLISH_COMPLETED);
            event.putInt(deregId);
            sendMsgToRCSUAProxy(event);
            deregId = 0;
        }
    }

    void enableImsDeregSuspend(int suspend) {
        RcsUaEvent event = new RcsUaEvent(CMD_ENABLE_IMS_DEREG_SUSPEND);
        event.putInt(suspend);
        sendMsgToRCSUAProxy(event);
    }

    void requestImsRestoration() {
        RcsUaEvent event = new RcsUaEvent(CMD_IMS_RESTORATION);
        writeEvent(event);
    }

    /**
     * Write event to socket.
     *
     * @param event the event
     */
    void writeEvent(RcsUaEvent event) {
        if (event != null) {
            Message.obtain(uaHandler, MSG_HAL_WRITE_EVENT, event).sendToTarget();
        }
    }

    long getCurrentRcsFeatures() {
        if (service != null) {
            Capability capability = service.getCapabilities();
            if (capability != null) {
                return capability.toNumeric();
            }
        }
        return -1;
    }

    /**
     * Send request to RCS_proxy to get the current info about the registration.
     */
    void sendRegistrationInfoRequest() {
        logger.debug("sendRegistrationInfoRequest");
        RcsUaEvent event = new RcsUaEvent(CMD_REQ_REG_INFO);
        sendMsgToRCSUAProxy(event);
    }

    /**
     * Update ims volte setting.
     */
    public void sendImsVolteConfig() {

        if (DEBUG_TRACE) {
            logger.debug("sendImsVolteConfig");
        }

        ImsVolteConfig mtkImsCfg = new ImsVolteConfig(Utils.getApplicationContext());
        mtkImsCfg.doAssignParam();

        // refer to ImsVolteConfig.java, remove runtime configuration
        int totalSettings = 110-12;
        byte[] byteParam = null;

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_UPDATE_SETTING);
        event.putInt(0); // account id
        event.putInt(0); // total data length, reserved field
        event.putInt(totalSettings); // total setting amounts

        /* TLV format, Tag refer to RcsEventSettingTLV.java */
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_System_Operator_ID);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_system_operator_id).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);
        //logger.debug("sendImsVolteConfig() Tag=" + RcsEventSettingTLV.VoLTE_Setting_System_Operator_ID);
        //logger.debug("sendImsVolteConfig() Len=" + byteParam.length);
        //logger.debug("sendImsVolteConfig() Value(decimal)=" + mtkImsCfg.mParam_system_operator_id);
        //logger.debug("sendImsVolteConfig() Value=" + bytesToHex(byteParam));

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Local_Address);
        //byteParam = (mtkImsCfg.mParam_net_local_address+'\0').getBytes();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);
        //logger.debug("sendImsVolteConfig() Tag=" + RcsEventSettingTLV.VoLTE_Setting_Net_Local_Address);
        //logger.debug("sendImsVolteConfig() Len=" + byteParam.length);
        //logger.debug("sendImsVolteConfig() Value(string)=" + mtkImsCfg.mParam_net_local_address);
        //logger.debug("sendImsVolteConfig() Value=" + bytesToHex(byteParam));

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Local_Port);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_local_port).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Local_Protocol_Type);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_local_protocol_type).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Local_Protocol_Version);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_local_protocol_version).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Local_IPSec_Port_Start);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_local_ipsec_port_start).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Local_IPSec_Port_Range);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_local_ipsec_port_range).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_IPSec);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_ipsec).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_IF_Name);
        //byteParam = (mtkImsCfg.mParam_net_if_name+'\0').getBytes();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Network_Id);
        //byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_network_id).array();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_SIP_Dscp);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_sip_dscp).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_SIP_Soc_Priority);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_sip_soc_priority).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_SIP_Soc_Tcp_Mss);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_sip_soc_tcp_mss).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_PCSCF_Port);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_pcscf_port).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_PCSCF_Number);
        //byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_net_pcscf_number).array();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Account_Private_UID);
        //byteParam = (mtkImsCfg.mParam_account_private_uid+'\0').getBytes();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Account_Home_URI);
        //byteParam = (mtkImsCfg.mParam_account_home_uri+'\0').getBytes();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Account_IMEI);
        //byteParam = (mtkImsCfg.mParam_account_imei+'\0').getBytes();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Server_PCSCF_List);
        //byteParam = (mtkImsCfg.mParam_server_pcscf_list+'\0').getBytes();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_VoLTE_Call_UserAgent);
        byteParam = (mtkImsCfg.mParam_volte_call_useragent+'\0').getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Register_Expiry);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_register_expiry).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Contact_With_UserName);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_contact_with_username).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_URI_With_Port);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_uri_with_port).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_IPSec_Algo_Set);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_ipsec_algo_set).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Enable_HTTP_Digest);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_enable_http_digest).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Auth_Name);
        //byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_auth_name).array();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Auth_Password);
        //byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_auth_password).array();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Specific_IPSec_Algo);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_specific_ipsec_algo).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CONTACT_WITH_TRANSPORT);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_contact_with_transport).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CONTACT_WITH_REGID);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_contact_with_regid).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CONTACT_WITH_MOBILITY);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_contact_with_mobility).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CONTACT_WITH_EXPIRES);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_contact_with_expires).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_AUTHORIZATION_WITH_ALGO);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_authorization_with_algo).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REREG_IN_RAT_CHANGE);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_rereg_in_rat_change).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REREG_IN_OOS_END);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_rereg_in_oos_end).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_DE_SUBSCRIBE);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_de_subscribe).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_USE_SPECIFIC_IPSEC_ALGO);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_use_specific_ipsec_algo).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_TRY_NEXT_PCSCF);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_try_next_pcscf).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_DEREG_CLEAR_IPSEC);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_dereg_clear_ipsec).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_INITIAL_REG_WITHOUT_PANI);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_initial_reg_without_pani).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_DEREG_RESET_TCP_CLIENT);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_dereg_reset_tcp_client).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_TREG);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_treg).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REREG_23G4);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_rereg_23g4).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_RESUB_23G4);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_resub_23g4).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_NOT_AUTO_REG_403);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_not_auto_reg_403).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CALL_ID_WITH_HOST_INREG);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_call_id_with_host_inreg).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_KEEP_ALIVE_MODE);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_keep_alive_mode).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_TCP_CONNECT_MAX_TIME_INVITE);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_tcp_connect_max_time_invite).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_EMS_MODE_IND);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_ems_mode_ind).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CONTACT_WITH_ACCESSTYPE);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_contact_with_accesstype).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_WFC_WITH_PLANI);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_wfc_with_plani).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_USE_UDP_ON_TCP_FAIL);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_use_udp_on_tcp_fail).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_IPSEC_FAIL_ALLOWED);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_ipsec_fail_allowed).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CONTACT_WITH_VIDEO_FEATURE_TAG_IN_SUBSCRIBE);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_contact_with_video_feature_tag_in_subscribe).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_VIA_WITHOUT_RPORT);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_via_without_rport).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REG_ROUTE_HDR);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_reg_route_hdr).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_VIA_URI_WITH_DEFAULT_PORT);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_via_uri_with_default_port).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_NOTIFY_SMS_NOTIFY_DONE);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_notify_sms_notify_done).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_EMERGENCY_USE_IMSI);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_emergency_use_imsi).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CHECK_MSISDN);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_check_msisdn).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_RETRY_INTERVAL_AFTER_403);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_retry_interval_after_403).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_SUPPORT_THROTTLING_ALGO);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_support_throttling_algo).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REG_AFTER_NW_DEREG_60S);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_reg_after_nw_dereg_60s).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_SUB_CONTACT_WITH_SIP_INSTANCE);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_sub_contact_with_sip_instance).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_STOP_REG_MD_LOWER_LAYER_ERR);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_stop_reg_md_lower_layer_err).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REG_GRUU_SUPPORT);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_reg_gruu_support).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_oos_end_reset_tcp_client);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_oos_end_reset_tcp_client).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_pidf_country);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_pidf_country).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_A_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_a_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_B_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_b_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_C_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_c_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_D_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_d_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_E_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_e_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_G_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_g_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_H_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_h_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_I_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_i_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_J_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_j_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_K_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_k_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_FAIL_NOT_NEED_REMOVE_BINDING);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_fail_not_need_remove_binding).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        // this should be configured dynamically
        /*
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REG_RCS_State);
        byteParam = (mtkImsCfg.mParam_reg_reg_rcs_state+'\0').getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);
        */

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_PEND_DEREG_IN_INITIAL_REG);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_pend_dereg_in_initial_reg).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_TRY_NEXT_PCSCF_5626);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_try_next_pcscf_5626).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_TRY_SAME_PCSCF_REREG);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_try_same_pcscf_rereg).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        //event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_PUBLIC_UIDS);
        //byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_public_uids).array();
        //event.putInt(byteParam.length);
        //event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_TRY_ALL_PCSCF_5626);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_try_all_pcscf_5626).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REG_Tdelay);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_reg_tdelay).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Try_N_Next_Pcscf_5626);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_try_n_next_pcscf_5626).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Dereg_Delay_Time);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_dereg_delay_time).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CHECK_NOTIFY_INSTANCE_ID);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_check_notify_instance_id).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_ADD_CS_VOLTE_FEATURE_TAG);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_add_cs_volte_feature_tag).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_CHECK_CALL_DOMAIN);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_check_call_domain).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_SUPPORT_SIP_BLOCK);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_support_sip_block).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_WHEN_STOP_B_TIMER);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_when_stop_b_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_support_resub);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_support_resub).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Expire_Time_By_User);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_expire_time_by_user).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Add_CNI_in_WIFI);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_add_cni_in_wifi).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Reg_Over_Tcp);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_reg_over_tcp).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Add_cell_info_age_to_cni);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_add_cell_info_age_to_cni).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_check_reg_contact);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_check_reg_contact).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_REG_Geolocation_Type);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_geolocation_type).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Init_Reg_Delay_Time);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_init_reg_delay_time).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_Update_IPSec_Port_494);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_reg_update_ipsec_port_494).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_T1_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_t1_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_T2_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_t2_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_T4_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_t4_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_Keep_Alive);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_ua_reg_keep_alive).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_Force_Use_UDP);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_force_use_udp).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_TCP_On_Demand);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_tcp_on_demand).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_TCP_MTU_Size);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_tcp_mtu_size).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_SIP_Transaction_Timer);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(mtkImsCfg.mParam_sip_transaction_timer).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        sendMsgToRCSUAProxy(event);
    }

    void sendUpdateRcsServiceState(long features) {
        if (features >= 0) {
            String state;

            if (features > 0) {
                state = "1_" + Long.toHexString(features);
            } else {
                state = "0_0";
            }

            RcsUaEvent event = new RuaAdapter.RcsUaEvent(
                    CMD_IMS_UPDATE_SETTING);
            event.putInt(0); // account id
            event.putInt(0); // total data length, reserved field
            event.putInt(1); // total setting amounts

            event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REG_RCS_State);
            byte[] byteParam = (state + '\0').getBytes();
            event.putInt(byteParam.length);
            event.putNBytes(byteParam, byteParam.length);

            sendMsgToRCSUAProxy(event);
        }
    }

    /**
     * handle the registration info send by rcs_proxy and save it based on state.
     *
     * @param event the event
     */
    void handleRegistrationInfo(RcsUaEvent event) {

        updateRegistrationInfo(event);

        /* Ignore ROI event when RegCore not get chance to be initialized */
        RoiHandler roi = RoiHandler.getInstance();
        if (roi == null && incomingOwner == RoiHandler.REG_OWNER_RCS) {
            if (regInfo.stateUpdated &&
                    regInfo.info.getRegState() == REG_STATE_NOT_REGISTERED &&
                    service != null) {
                service.notifyRegistrationChanged();
            }
            return;
        }

        if (roi != null) {
            roi.processRegistration(
                    new RoiHandler.InputEvent.Builder(RoiHandler.InputType.IMS_REGINFO_IND)
                            .setImsOwner(incomingOwner)
                            .setRegStatus(incomingImsState)
                            .build());
        }

        if (regInfo.stateUpdated) {
            if (regInfo.info.getRegState() == REG_STATE_REGISTERED) {
                if (sessionHdlr != null)
                    sessionHdlr.initInstanceCount();
            } else if (regInfo.info.getRegState() == REG_STATE_NOT_REGISTERED) {
                sipHandler.dispose();
                if (sessionHdlr != null)
                    sessionHdlr.initInstanceCount();
            }
            if (service != null)
                service.notifyRegistrationChanged();
        }

        if (regInfo.rereg && service != null) {
            service.notifyReregistration();
        }
    }

    void handleDeregIndication(RcsUaEvent event) {
        if (deregSuspend > 0 && service != null) {
            deregId = event.getInt();
            service.notifyDeregStart();
        }
    }

    private boolean updateRegistrationInfo(RcsUaEvent incomingEvent) {
        regInfo.stateUpdated = false;
        regInfo.configUpdated = false;
        regInfo.rereg = false;

        Configuration imsConfig = regInfo.getImsConfig();

        // refer _RCS_PROXY_Event_Reg_State2_ structure
        incomingOwner = incomingEvent.getInt();  // 0: VoLTE, 1: ROI
        incomingImsState = incomingEvent.getInt();
        int rcsState = incomingEvent.getInt();
        int rcsTag = incomingEvent.getInt();

        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
            String local = incomingEvent.getString(64);
            imsConfig.setLocalAddress(formatIpAddress(local));
            imsConfig.setLocalPort(incomingEvent.getInt()); // 76
            // TODO: should use IMS stack assigned transport
            imsConfig.setProtocol(incomingEvent.getInt()); // TCP=1,UDP=2,TCPUDP=3
            // Hardcode to avoid IMS stack assign otherwise
            imsConfig.setProtocol(Configuration.TCP);
            imsConfig.setIpVersion(incomingEvent.getInt()); //IPv4=1,IPv6=2
            imsConfig.setIMPU(incomingEvent.getString(256));
            imsConfig.setIMPI(incomingEvent.getString(256));
            imsConfig.setHomeDomain(incomingEvent.getString(256));
            imsConfig.setProxyAddress(incomingEvent.getString(256));
            imsConfig.setProxyPort(incomingEvent.getInt());
            imsConfig.setUserAgent(incomingEvent.getString(128));
            imsConfig.setPAssociatedUri(incomingEvent.getString(512));
            imsConfig.setInstanceId(incomingEvent.getString(128));
            imsConfig.setServiceRoute(incomingEvent.getString(128));
            String pani = incomingEvent.getString(256);
            if (!pani.equals(imsConfig.getPAccessNetworkInfo())) {
                imsConfig.setPAccessNetworkInfo(pani);
                regInfo.setRadioTech(pani);
                regInfo.configUpdated = true;
            }
            imsConfig.setDigitVlineNumber(incomingEvent.getInt());
            imsConfig.setPIdentifier(incomingEvent.getString(256));
            imsConfig.setPPreferredAssociation(incomingEvent.getString(4096));
            regInfo.regTimestamp = incomingEvent.getString(256);
            if (regInfo.rcsTags != rcsTag) {
                regInfo.rcsTags = rcsTag;
                regInfo.configUpdated = true;
            }
            imsConfig.setRegRcsFeatureTags(rcsTag);
        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
            imsConfig.reset();
            regInfo.rcsTags = 0;
            regInfo.regTimestamp = "";
        }

        // TODO: there should be better way to do this
        if (REG_MODE_IMS == regInfo.getRegMode()) {
            if (incomingOwner == RoiHandler.REG_OWNER_VOLTE) {
                switch (regInfo.getRegState()) {
                    case REG_STATE_NOT_REGISTERED:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.setRegState(REG_STATE_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                            regInfo.setRegState(REG_STATE_DEREGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;

                    case REG_STATE_REGISTERING:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.setRegState(REG_STATE_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                            regInfo.setRegState(REG_STATE_DEREGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                            regInfo.setRegState(REG_STATE_NOT_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;

                    case REG_STATE_REGISTERED:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.rereg = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                            regInfo.setRegState(REG_STATE_DEREGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                            regInfo.setRegState(REG_STATE_NOT_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;

                    case REG_STATE_DEREGISTERING:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.setRegState(IMS_REG_STATE_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                            regInfo.setRegState(REG_STATE_NOT_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;
                    default:
                        break;
                }
            } else
            if (incomingOwner == RoiHandler.REG_OWNER_RCS) {
                switch (regInfo.getRegState()) {
                    case REG_STATE_NOT_REGISTERED:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.setRegState(REG_STATE_REGISTERED);
                            regInfo.setRegMode(REG_MODE_INTERNET);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.setRegMode(REG_MODE_INTERNET);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                            regInfo.setRegState(REG_STATE_DEREGISTERING);
                            regInfo.setRegMode(REG_MODE_INTERNET);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;

                    case REG_STATE_REGISTERING:

                        // SHOULD not go here, handle just for abnormal case

                        break;

                    case REG_STATE_REGISTERED:

                        // SHOULD not go here, handle just for abnormal case

                        break;

                    case REG_STATE_DEREGISTERING:

                        // SHOULD not go here, handle just for abnormal case

                        break;
                    default:
                        break;
                }
            }
        } else
        if (REG_MODE_INTERNET == regInfo.getRegMode()) {
            if (incomingOwner == RoiHandler.REG_OWNER_RCS) {
                switch (regInfo.getRegState()) {
                    case REG_STATE_NOT_REGISTERED:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.setRegState(REG_STATE_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                            regInfo.setRegState(REG_STATE_DEREGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;

                    case REG_STATE_REGISTERING:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.setRegState(REG_STATE_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                            regInfo.setRegState(REG_STATE_DEREGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                            regInfo.setRegState(REG_STATE_NOT_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;

                    case REG_STATE_REGISTERED:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.rereg = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                            regInfo.setRegState(REG_STATE_DEREGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                            regInfo.setRegState(REG_STATE_NOT_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;

                    case REG_STATE_DEREGISTERING:

                        if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                            regInfo.setRegState(IMS_REG_STATE_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                        } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                            regInfo.setRegState(REG_STATE_REGISTERING);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                        } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                            regInfo.setRegState(REG_STATE_NOT_REGISTERED);
                            regInfo.stateUpdated = true;
                        } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                        } else if (incomingImsState == IMS_REG_STATE_OOS) {
                        } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                        }

                        break;
                    default:
                        break;
                }
            } else
            if (incomingOwner == RoiHandler.REG_OWNER_VOLTE) {
                    switch (regInfo.getRegState()) {
                        case REG_STATE_NOT_REGISTERED:

                            if (incomingImsState == IMS_REG_STATE_REGISTERED) {
                                regInfo.setRegState(REG_STATE_REGISTERED);
                                regInfo.setRegMode(REG_MODE_IMS);
                                regInfo.stateUpdated = true;
                            } else if (incomingImsState == IMS_REG_STATE_UNREGISTERED) {
                            } else if (incomingImsState == IMS_REG_STATE_REGISTERING) {
                                regInfo.setRegState(REG_STATE_REGISTERING);
                                regInfo.setRegMode(REG_MODE_IMS);
                                regInfo.stateUpdated = true;
                            } else if (incomingImsState == IMS_REG_STATE_DEREGISTERING) {
                                regInfo.setRegState(REG_STATE_DEREGISTERING);
                                regInfo.setRegMode(REG_MODE_IMS);
                                regInfo.stateUpdated = true;
                            } else if (incomingImsState == IMS_REG_STATE_DISCONNECTED) {
                            } else if (incomingImsState == IMS_REG_STATE_AUTHENTICATING) {
                            } else if (incomingImsState == IMS_REG_STATE_OOS) {
                            } else if (incomingImsState == IMS_REG_STATE_CONNECTING) {
                            }

                            break;

                        case REG_STATE_REGISTERING:

                            // SHOULD not go here, handle just for abnormal case

                            break;

                        case REG_STATE_REGISTERED:

                            // SHOULD not go here, handle just for abnormal case

                            break;

                        case REG_STATE_DEREGISTERING:

                            // SHOULD not go here, handle just for abnormal case

                            break;
                        default:
                            break;
                    }
                }
        }

        logger.debug("updateRegistrationInfo " + regInfo.stateUpdated
                + " with owner[" + incomingOwner + "]"
                + ", reg[" + incomingImsState + "], rcs[" + rcsState + "], tag[" + rcsTag + "]");

        return true;
    }


    /**
     * Send msg to rcsua proxy.
     *
     * @param event the event
     */
    protected void sendMsgToRCSUAProxy(RcsUaEvent event) {

        if (halv1 == null && halv2 == null) {
            logger.debug("sendMsgToRCSUAProxy failed, halInterface is null!: " + event.getRequestId());
            return;
        }

        logger.debug( "event to proxy: " + event.getRequestId());

        // send the event to UA
        int request = event.getRequestId();
        int length = event.getDataLen();
        byte[] data = event.getData();

        try {
            if (halv1 != null)
                halv1.writeEvent(request, length, Utils.arrayToList(data, length));
            else
                halv2.writeEvent(0, request, Utils.arrayToList(data, length));
        } catch (RemoteException e) {
            logger.debug("hal writeEvent e: " + e);
        }
    }

    final class UaConnectTask {

        UaConnectTask() {
        }

        synchronized void start() {
            if (uaConnected)
                return;

            if (thread == null) {
                thread = new HandlerThread("ConnectUAThread");
                thread.start();
                handler = new Handler(thread.getLooper());
            }
            if (doConnect != null) {
                handler.removeCallbacks(doConnect);
                tryCount = 0;
            } else {
                doConnect = new DoConnect();
            }
            handler.post(doConnect);
        }

        synchronized void stop() {
            if (thread != null) {
                handler.removeCallbacks(doConnect);
                thread.quit();
                thread = null;
                tryCount = 0;
                doConnect = null;
            }
        }

        final class DoConnect implements Runnable {

            @Override
            public  void run() {
                if (!uaConnected) {
                    try {
                        halv2 = vendor.mediatek.hardware.rcs.V2_0.IRcs.getService("rcs_hal_service");
                        if (halv2 != null) {
                            halv2.linkToDeath(halDeathRecipient, halCookie.incrementAndGet());
                            halv2.setResponseFunctions(halCallbackv2);
                        }
                    } catch (RemoteException | RuntimeException e) {
                        halv2 = null;
                    }
                    if (halv2 == null) {
                        try {
                            halv1 = vendor.mediatek.hardware.rcs.V1_0.IRcs.getService("rcs_hal_service");
                            if (halv1 != null) {
                                halv1.linkToDeath(halDeathRecipient, halCookie.incrementAndGet());
                                halv1.setResponseFunctions(halCallbackv1);
                            }
                        } catch (RemoteException | RuntimeException e) {
                            halv1 = null;
                        }
                    }
                    if (halv1 != null || halv2 != null) {
                        tryCount = 0;
                        Message.obtain(uaHandler, MSG_UA_CONNECTED).sendToTarget();
                        stop();
                    } else {
                        if (thread != null) {
                            if (tryCount++ < MAX_RETRY_TIMES)
                                UaConnectTask.this.handler.postDelayed(this, tryCount * 500);
                            else {
                                tryCount = 0;
                                UaConnectTask.this.handler.postDelayed(this, 1000);
                            }
                        }
                    }
                }
            }
        }

        private final int MAX_RETRY_TIMES = 20;
        private int tryCount = 0;
        private HandlerThread thread;
        private Handler handler;
        private DoConnect doConnect;
    }

    final class IRcsDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            logger.debug("Rcs hal serviceDied");
            Message.obtain(uaHandler, MSG_UA_DISCONNECTED).sendToTarget();
        }
    }

    class RcsHalCallbackv2 extends vendor.mediatek.hardware.rcs.V2_0.IRcsIndication.Stub {
        @Override
        public void readEvent(int account, int request, ArrayList<Byte> data) throws RemoteException {
            if (logger.isActivated()) {
                logger.debug("Hal readEvent[" + request + "] with account[" + account + "]");
            }

            RcsUaEvent event = new RcsUaEvent(request);
            event.putBytes(Utils.listToArray(data));

            switch (request) {
                /* Events for LTE/WFC/234G Registration */
                case RSP_REQ_REG_INFO:
                case RSP_IMS_REGISTER:
                case RSP_IMS_DEREGISTER:
                case RSP_IMS_REGISTERING:
                case RSP_IMS_DEREGISTERING:
                    Message.obtain(uaHandler, MSG_HAL_INCOMING_REG, event).sendToTarget();
                    break;

                /*  Events for SIP */
                case RuaAdapter.RSP_EVENT_SIP_MSG:
                    sipHandler.handleSipMessageReceived(event);
                    break;

                /* Events for 234G Registration */
                case RuaAdapter.EVENT_IMS_AUTH_REQ:
                case RuaAdapter.EVENT_IMS_GEOLOCATION_REQ:
                case RuaAdapter.EVENT_IMS_QUERY_STATE:
                case RuaAdapter.EVENT_IMS_EMS_MODE_INFO:
                case RuaAdapter.EVENT_IMS_DIGITLING_REG_IND:
                    Message.obtain(uaHandler, MSG_HAL_INCOMING_ROI, event).sendToTarget();
                    break;

                case RuaAdapter.EVENT_IMS_DEREGISTER_IND:
                    Message.obtain(uaHandler, MSG_HAL_DEREG_IND, event).sendToTarget();
                    break;

                default:
                    break;
            }
        }

    }

    class RcsHalCallbackv1 extends vendor.mediatek.hardware.rcs.V1_0.IRcsIndication.Stub {
        @Override
        public void readEvent(ArrayList<Byte> data, int requestId, int length) {
            if (logger.isActivated()) {
                logger.debug("Hal readEvent: " + requestId);
            }

            RcsUaEvent event = new RcsUaEvent(requestId);
            event.putBytes(Utils.listToArray(data));

            switch (requestId) {
                /* Events for LTE/WFC/234G Registration */
                case RSP_REQ_REG_INFO:
                case RSP_IMS_REGISTER:
                case RSP_IMS_DEREGISTER:
                case RSP_IMS_REGISTERING:
                case RSP_IMS_DEREGISTERING:
                    Message.obtain(uaHandler, MSG_HAL_INCOMING_REG, event).sendToTarget();
                    break;

                /*  Events for SIP */
                case RuaAdapter.RSP_EVENT_SIP_MSG:
                    sipHandler.handleSipMessageReceived(event);
                    break;

                /* Events for 234G Registration */
                case RuaAdapter.EVENT_IMS_AUTH_REQ:
                case RuaAdapter.EVENT_IMS_GEOLOCATION_REQ:
                case RuaAdapter.EVENT_IMS_QUERY_STATE:
                case RuaAdapter.EVENT_IMS_EMS_MODE_INFO:
                case RuaAdapter.EVENT_IMS_DIGITLING_REG_IND:
                    Message.obtain(uaHandler, MSG_HAL_INCOMING_ROI, event).sendToTarget();
                    break;

                case RuaAdapter.EVENT_IMS_DEREGISTER_IND:
                    Message.obtain(uaHandler, MSG_HAL_DEREG_IND, event).sendToTarget();
                    break;

                default:
                    break;
            }
        }
    }

    public void sendSimOperator() {

        logger.debug("sendSimOperator");

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_SIM_OPERATOR);

        sendMsgToRCSUAProxy(event);
    }

    public void sendServiceActivationState(int state) {

        logger.debug("sendServiceActivationState, state = " + state);

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_SERVICE_ACTIVATION_STATE);

        event.putInt(state);

        sendMsgToRCSUAProxy(event);
    }

    public void sendUpdateServiceTags(long tags) {
        logger.debug("sendUpdateServiceTags, tags = " + tags);

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
                CMD_UPDATE_SERVICE_TAGS);

        event.putInt((int)tags);

        sendMsgToRCSUAProxy(event);
    }

    public void sendActiveRcsSlotId(int slotId) {
        logger.debug("sendActiveRcsSlotId, slotId = " + slotId);

        if (slotId < 0 || slotId >= TelephonyManager.getDefault().getPhoneCount()) {
            logger.error("sendActiveRcsSlotId, invalid slotId = " + slotId);
            return;
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
                CMD_ACTIVE_RCS_SLOT_ID);

        event.putInt(slotId);

        sendMsgToRCSUAProxy(event);
    }

    // RCS for 3g volte
    // VOLTE_REQ_SIPTX_REG_NOTIFY_IMSVOPS
    // Roi_Event_Notify_t
    public void sendRegNotifyImsvops(int acct_id, int data) {

        if (DEBUG_TRACE) {
            logger.debug("sendRegNotifyImsvops with id:" + acct_id + "data:" + data);
        }
        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_NOTIFY_VOPS_INFO);
        event.putInt(acct_id);
        event.putInt(data);

        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_UPDATE_SETTING
    // Roi_Event_Setting_t
    public void sendRegUpdateSettings(int acct_id,
        int num_settings, ArrayList < RcsEventSettingTLV > settings_data) {

        if (DEBUG_TRACE) {
            logger.debug("sendRegUpdateSettings with id:" + acct_id + "num_settings:" + num_settings);
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_UPDATE_SETTING);
        event.putInt(acct_id);
        int total_data_len = 0; //Will be re-calculated at lower layer. For sturcure complete.
        event.putInt(total_data_len);
        event.putInt(num_settings);
        for (int i = 0; i < num_settings; i++) {
            RcsEventSettingTLV item = settings_data.get(i);
            logger.debug("item with[" + i + "] type:" + item.mType + ",data_len:" +
                item.mDatalen + ",data:" + Arrays.toString(item.mData));
            event.putInt(item.mType);
            event.putInt(item.mDatalen);
            event.putNBytes(item.mData, item.mDatalen);
        }
        sendMsgToRCSUAProxy(event);
    }

    public void sendRegAddCapabilties() {
        updateRegAddCapability(true, true);
    }

    public void sendRegRemoveCapabilties() {
        updateRegAddCapability(false, true);
    }

    public void sendRegUpdateCapabilities(boolean activated, Capability capability) {
        /******************************************************************
         *  [Important] Need to change according to "exact" feature tags  *
         *  Purpose: notify info to rcs_volte_stack                       *
         *  parameter: <rcs state>_<feature tag>                          *
         *  <rcs_state>: 0/1                                              *
         *  <feature tag>: hex value, use bit-map                         *
         *         0x01  session                                          *
         *         0x02  filetransfer                                     *
         *         0x04  msg                                              *
         *         0x08  largemsg                                         *
         *         0x10  geopush                                          *
         *         0x20  geopull                                          *
         *         0x40  geopullft                                        *
         *         0x80  imdn aggregation                                 *
         *        0x100  geosms                                           *
         *        0x200  fthttp                                           *
         *         0x00  RCS service tag was removed                      *
         ******************************************************************/
        long features = capability.toNumeric();

        // need to notify IMS stack when RCS activated
        if (activated && features > 0) {
            // notify volte_stack
            String strATCmd = "AT+EIMSRCS=";

            strATCmd += "1," + features;
            Utils.sendAtCommand(strATCmd);
        } else if (features == 0) {
            Utils.sendAtCommand("AT+EIMSRCS=0, 0");
        }
        // need to save here for next boot usage
        sendUpdateServiceTags(features);

        if (RoiHandler.getInstance() != null) {
            sendUpdateRcsServiceState(features);
        }
    }

    //VOLTE_REQ_SIPTX_SETTING
    //VoLTE_Event_Setting_t
    //volte_sip_stack_update_setting
    public void updateRegAddCapability(boolean isRcsSrvEnabled, boolean isATCmdNeed) {

        /******************************************************************
         *  [Important] Need to change according to "exact" feature tags  *
         *  Purpose: notify info to rcs_volte_stack                       *
         *  parameter: <rcs state>_<feature tag>                          *
         *  <rcs_state>: 0/1                                              *
         *  <feature tag>: hex value, use bit-map                         *
         *         0x01  session                                          *
         *         0x02  filetransfer                                     *
         *         0x04  msg                                              *
         *         0x08  largemsg                                         *
         *         0x10  geopush                                          *
         *         0x20  geopull                                          *
         *         0x40  geopullft                                        *
         *         0x80  imdn aggregation                                 *
         *        0x100  geosms                                           *
         *        0x200  fthttp                                           *
         *         0x00  RCS service tag was removed                      *
         ******************************************************************/
        int intRcsSrvTag = SystemProperties.getInt("persist.vendor.service.tag.rcs", 0);

        // error handle
        if (intRcsSrvTag < 0x00 || intRcsSrvTag > 0xFFFF) {
            intRcsSrvTag = 0;
        }

        // notify volte_stack
        String strATCmd = "AT+EIMSRCS=";
        String state_capabiliity = null;
        if (isRcsSrvEnabled && intRcsSrvTag > 0) {
            strATCmd = strATCmd + "1," + intRcsSrvTag;
            state_capabiliity = "1_" + Integer.toHexString(intRcsSrvTag);
        } else {
            strATCmd = strATCmd + "0," + intRcsSrvTag;
            state_capabiliity = "0_" + Integer.toHexString(intRcsSrvTag);
        }
        if (isATCmdNeed) {
            Utils.sendAtCommand(strATCmd);
        }
        logger.debug("updateRegAddCapability()" +
            ": isATCmdNeed=" + isATCmdNeed +
            ", intRcsSrvTag=" + intRcsSrvTag +
            ", strATCmd:-->" + strATCmd + "<--" +
            ", state_capabiliity:-->" + state_capabiliity + "<--");

        // notify volte_rcs_stack: only OP08 needed
        if (!Utils.isOP08Supported()) {
            return;
        }

        byte[] byteParam = null;

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
           CMD_IMS_UPDATE_SETTING);
        event.putInt(0); // account id
        event.putInt(0); // total data length, reserved field
        event.putInt(1); // total setting amounts

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Reg_REG_RCS_State);
        byteParam = (state_capabiliity + '\0').getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_3GPP_RAT_CHANGE
    // Roi_Event_3gpp_Rat_Change_t
    public void sendReg3gppRatChange(int acct_id, int old_rat_type,
        String old_cell_id, int new_rat_type, String new_cell_id,
        double lat, double lng, int acc, String timestamp) {

        if (DEBUG_TRACE) {
            logger.debug("sendReg3gppRatChang with acct_id:" + acct_id +
                ",old_rat_type:" + old_rat_type +
                ",old_cell_id:" + old_cell_id +
                ",new_rat_type:" + new_rat_type +
                ",new_cell_id:" + new_cell_id +
                ",lat:" + lat +
                ",lng:" + lng +
                ",acc:" + acc +
                ",timestamp:" + timestamp);
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_NOTIFY_3GPP_RAT_CHANGE);

        event.putInt(acct_id);
        event.putInt(old_rat_type);
        event.putString(old_cell_id, VOLTE_MAX_CELL_ID_LENGTH);
        event.putInt(new_rat_type);
        event.putString(new_cell_id, VOLTE_MAX_CELL_ID_LENGTH);
        //Roi_Event_LBS_Location_t
        event.putDouble(lat);
        event.putDouble(lng);
        event.putInt(acc);
        event.putString(timestamp, VOLTE_MAX_TIME_STAMP_LENGTH);
        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_RAT_CHANGE
    // Roi_Event_Rat_Change_t
    public void sendRegRatChange(int acct_id, int type, String cell_id,
        double lat, double lng, int acc, String timestamp, String ssid) {

        if (DEBUG_TRACE) {
            logger.debug("sendRegRatChange with acct_id:" + acct_id +
                "type:" + type +
                ",cell_id:" + cell_id +
                ",lat:" + lat +
                ",lng:" + lng +
                ",acc:" + acc +
                ",timestamp:" + timestamp +
                ",ssid:" + ssid);
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_NOTIFY_RAT_CHANGE);
        event.putInt(acct_id);
        event.putInt(type);
        event.putString(cell_id, VOLTE_MAX_CELL_ID_LENGTH);
        //Roi_Event_LBS_Location_t
        event.putDouble(lat);
        event.putDouble(lng);
        event.putInt(acc);
        event.putString(timestamp, VOLTE_MAX_TIME_STAMP_LENGTH);
        event.putString(ssid, VOLTE_MAX_SSID_LENGTH);

        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_NETWORK_CHANGE
    // Roi_Event_Network_Change_t
    public void sendRegNetworkChange(int acct_id, int state,
        int type, String cell_id,
        double lat, double lng, int acc, String timestamp) {

        if (DEBUG_TRACE) {
            logger.debug("sendRegNetworkChange with acct_id:" + acct_id +
                ",state:" + state +
                ",type:" + type +
                ",cell_id:" + cell_id +
                ",lat:" + lat +
                ",lng:" + lng +
                ",acc:" + acc +
                ",timestamp:" + timestamp);
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_NOTIFY_NETWORK_CHANGE);

        event.putInt(acct_id);
        event.putInt(state);
        event.putInt(type);
        event.putString(cell_id, VOLTE_MAX_CELL_ID_LENGTH);
        //Roi_Event_LBS_Location_t
        event.putDouble(lat);
        event.putDouble(lng);
        event.putInt(acc);
        event.putString(timestamp, VOLTE_MAX_TIME_STAMP_LENGTH);
        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_CNF_SIPTX_REG_AUTH_REQ
    // Roi_Event_Reg_Auth_Resp_t
    public void sendReqAuthReqResult(int acct_id, int result,
        String nc, byte[] response, byte[] auts,
        String ck_esp, String ik_esp, int user_data) {

        if (DEBUG_TRACE) {
            logger.debug("sendReqAuthReq with acct_id:" + acct_id +
                ",result:" + result +
                ",nc:" + nc +
                ",response:" + response +
                ",auts:" + auts +
                ",ck_esp:" + ck_esp +
                ",ik_esp:" + ik_esp +
                ",user_data:" + user_data);
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_AUTH_REQ_RESULT);
        event.putInt(acct_id);
        event.putInt(result);
        event.putString(nc, VOLTE_MAX_AUTH_NC);
        event.putNBytes(response, VOLTE_MAX_AUTH_RESPONSE);
        event.putNBytes(auts, VOLTE_MAX_AUTH_AUTS);
        event.putString(ck_esp, VOLTE_MAX_AUTH_CK);
        event.putString(ik_esp, VOLTE_MAX_AUTH_IK);
        event.putInt(user_data);
        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_CNF_SIPTX_GEOLOCATION_REQ
    // Roi_Event_Geolocation_Info_t
    public void sendGeolocationReqResult(int ua_msg_hdr_id, int ua_msg_hdr_hsk,
        byte[] ua_msg_hdr_resv, int type_id,
        double lat, double lng, int accuracy,
        String method, String city, String geo_state, String zip, String country_code) {

        if (DEBUG_TRACE) {
            logger.debug("sendGeolocationReq with ua_msg_hdr id:"
                    + ua_msg_hdr_id + ",hsk:" + ua_msg_hdr_hsk);
            logger.debug("sendGeolocationReq with type_id:" + type_id +
                ",lat:" + lat +
                ",lng:" + lng +
                ",accuracy:" + accuracy +
                ",method:" + method +
                ",city:" + city +
                ",geo_state:" + geo_state +
                ",zip:" + zip +
                ",country_code:" + country_code);
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_GEOLOCATION_REQ_RESULT);
        //Roi_Event_UA_Msg_Hdr_t
        event.putInt(ua_msg_hdr_id);
        event.putByte(ua_msg_hdr_hsk);
        event.putNBytes(ua_msg_hdr_resv, 3);

        event.putInt(type_id);
        event.putDouble(lat);
        event.putDouble(lng);
        event.putInt(accuracy);
        event.putString(method, 16);
        event.putString(city, 32);
        event.putString(geo_state, 32);
        event.putString(zip, 8);
        event.putString(country_code, 8);

        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_UPDATE_SETTING
    // VoLTE_Event_Setting_t
    public void sendNetworkRtSettings(int acct_id, String networkInterface,
        int networkid, String localAddress, String[] pCSCFList,
        int numberOfPcscf, int protocolVersion, String cellId) {

        if (DEBUG_TRACE) {
            logger.debug("sendNetworkRtSettings with acct_id:" + acct_id
                + ",networkInterface:" + networkInterface
                + ",networkid:" + networkid
                + ",localAddress:" + localAddress
                + ",pCSCFList:" + Arrays.toString(pCSCFList)
                + ",numberOfPcscf:" + numberOfPcscf
                + ",protocolVersion:" + protocolVersion
                + ",cellId:" + cellId);
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
            CMD_IMS_UPDATE_RT_SETTING);

        int totalSettings = 6;
        if (cellId.length() > 0)
            totalSettings++;

        byte[] byteParam = null;

        event.putInt(0); // account id
        event.putInt(0); // total data length, reserved field
        event.putInt(totalSettings); // total setting amounts

        networkInterface = networkInterface + '\0';
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_IF_Name);
        byteParam = networkInterface.getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Network_Id);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN)
                .putInt(networkid).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        localAddress = localAddress + '\0';
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Local_Address);
        byteParam = localAddress.getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Local_Protocol_Version);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN)
                .putInt(protocolVersion).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        String result_pcscf_list = new String();
        int result_length = 0;
        for (String s:pCSCFList) {
            result_pcscf_list = result_pcscf_list + s + ",";
            result_length = result_length + s.getBytes().length + 1;
        }
        result_pcscf_list = result_pcscf_list + '\0';
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Server_PCSCF_List);
        byteParam = result_pcscf_list.getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_PCSCF_Number);
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN)
                .putInt(numberOfPcscf).array();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        if (cellId.length() > 0) {
            cellId = cellId + '\0';
            event.putInt(RcsEventSettingTLV.VoLTE_Setting_Net_Cell_ID);
            byteParam = cellId.getBytes();
            event.putInt(byteParam.length);
            event.putNBytes(byteParam, byteParam.length);
        }

        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_UPDATE_SETTING
    // VoLTE_Event_Setting_t
    public void sendSimRtSettings(int acct_id, String imei, String home_uri,
        String priviate_uid, String[] public_uid) {

        if (DEBUG_TRACE) {
            logger.debug("sendSimRtSettings with acct_id:" + acct_id
                + ",imei:" + imei
                + ",home_uri:" + home_uri
                + ",priviate_uid:" + priviate_uid
                + ",public_uid:" + Arrays.toString(public_uid));
        }

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
                CMD_IMS_UPDATE_RT_SETTING);
        int totalSettings = 4;
        byte[] byteParam = null;

        event.putInt(0); // account id
        event.putInt(0); // total data length, reserved field
        event.putInt(totalSettings); // total setting amounts

        priviate_uid = priviate_uid + '\0';
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Account_Private_UID);
        byteParam = priviate_uid.getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        home_uri = home_uri + '\0';
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Account_Home_URI);
        byteParam = home_uri.getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        imei = imei + '\0';
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Account_IMEI);
        byteParam = imei.getBytes();
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        String result_public_uid = new String();
        int result_length = 0;
        if (public_uid != null) {
            for (String s:public_uid) {
                if (s == null || s.length() <= 0) {
                    logger.debug("s is null or lenght ==0");
                    continue;
                }
                result_public_uid = result_public_uid + s + ",";
                result_length = result_length + s.getBytes().length + 1;
            }
            //Replace end ',' with ';'
            result_public_uid = result_public_uid.substring(0,result_public_uid.lastIndexOf(',')) + ';';
            result_public_uid = result_public_uid + '\0';
        }
        event.putInt(RcsEventSettingTLV.VoLTE_Setting_Account_Public_UID);
        byteParam = result_public_uid.getBytes();
        logger.debug("byteParam.length:" + byteParam.length +" = result_length:" + result_length);
        event.putInt(byteParam.length);
        event.putNBytes(byteParam, byteParam.length);

        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_DIGITLINE
    // VoLTE_Event_ATCmd_Relay_t
    public void sendRegDigitLine(int acct_id, int atcmd_id, int pad,
        int pad2, String cmdlnie) {

        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
                   CMD_IMS_REG_DIGITLINE);

        event.putInt(acct_id);
        event.putInt(atcmd_id);
        event.putInt(pad);
        event.putInt(pad2);
        event.putString(cmdlnie, VOLTE_MAX_AT_CMDLINE_LEN);

        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_DEREG
    public void sendRegRegister() {
        logger.debug("sendRegRegister");
        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
                CMD_IMS_REGISTER);
        sendMsgToRCSUAProxy(event);
    }

    // VOLTE_REQ_SIPTX_REG_DEREG
    public void sendRegDeregister() {
        logger.debug("sendRegDeregister");
        RcsUaEvent event = new RuaAdapter.RcsUaEvent(
                   CMD_IMS_DEREGISTER);
        sendMsgToRCSUAProxy(event);
    }

    public void sendRcsActivation(Capability capability) {
        // Use AT command instead of UA command
        long features = capability.toNumeric();
        Utils.sendAtCommand("AT+EIMSRCS=1," + features);
        sendServiceActivationState(1);
    }

    public void sendRcsDeactivation() {
        // Use AT command instead of UA command
        Utils.sendAtCommand("AT+EIMSRCS=0,0");
        sendServiceActivationState(0);
    }

    public void sendRegReregister(Capability capability) {
        // Use AT command instead of UA command
        long features = capability.toNumeric();
        Utils.sendAtCommand("AT+EIMSRCS=2," + features);
    }

    public void UtRcsSendTest() {
        logger.debug("UtRcsSendTest 10:34");
        byte[] byteParam = null;
        //C.K. UT code
        int acct_id = 0;
        int data = 16;
        String imei = "358509014497232";
        String home_uri = "www.google.com"+'0';
        String priviate_uid = "1000";
        String[] public_uid = {"1000","2000"};
        String method = "method1";
        String city = "city1";
        String geo_state = "geo_state1";
        String zip = "zip1";
        String country_code = "country_code1";
        int ua_msg_hdr_id = 1;
        int ua_msg_hdr_hsk = 2;
        int type_id = 3;
        double lat = 4.44;
        double lng = 5.55;
        int accuracy = 6;
        byte[] ua_msg_hdr_resv ={0x30, 0x40, 0x50};
        int result = 1;
        String nc = "nc1";
        String response = "response1";
        String auts = "auts1";
        String ck_esp = "ck_esp1";
        String ik_esp = "ik_esp1";
        int user_data = 1;
        int type = 1;
        int state = 2;
        String cell_id = "cell_id1";
        String new_cell_id = "new_cell_id1";
        String old_cell_id = "old_cell_id1";
        String timestamp = "timestamp1";
        String ssid = "ssid1";
        int acc = 2;
        int old_rat_type = 0;
        int new_rat_type = 1;
        int rat_type = 1;
        int enable = 1;
        String capability = "capability1";
        int num_settings = 2;
        ArrayList < RcsEventSettingTLV > settings_data = new ArrayList<>();
        byteParam = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(data).array();
        RcsEventSettingTLV tlv1 = new RcsEventSettingTLV(100, byteParam.length, byteParam);
        settings_data.add(tlv1);

        byteParam = home_uri.getBytes();
        RcsEventSettingTLV tlv2 = new RcsEventSettingTLV(200, byteParam.length, byteParam);
        settings_data.add(tlv2);

        sendRegNotifyImsvops(acct_id, data);
        sendRegUpdateSettings(acct_id,
            num_settings, settings_data);
        sendReg3gppRatChange(acct_id, old_rat_type,
            old_cell_id, new_rat_type, new_cell_id,
            lat, lng, acc, timestamp);
        sendRegRatChange(acct_id, type, cell_id,
            lat, lng, acc, timestamp, ssid);
        sendRegNetworkChange(acct_id, state,
            type, cell_id,
            lat, lng, acc, timestamp);
        /*sendReqAuthReqResult(acct_id, result,
            nc, response, auts,
            ck_esp, ik_esp, user_data);*/
        sendGeolocationReqResult(ua_msg_hdr_id, ua_msg_hdr_hsk,
            ua_msg_hdr_resv, type_id,
            lat, lng, accuracy,
            method, city, geo_state, zip, country_code);
        sendSimRtSettings(acct_id, imei, home_uri,
            priviate_uid, public_uid);
        //C.K. UT code end
    }

    final protected static char[] hexArray = "0123456789abcdef".toCharArray();

    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];

        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }

        return new String(hexChars);
    }

    private String formatIpAddress(String ip) {
        String ipAddress = ip.trim();
        /*
        // Work around IMS stack IP address wrapped by '[]' issue
        if (ipAddress.startsWith("[")) {
            int index = ipAddress.lastIndexOf(']');
            if (index > 0) {
                ipAddress = ipAddress.substring(1, index);
            }
        }
        */
        return ipAddress;
    }

    private void registerNetworkCallback() {
        if (mPhoneStateListener == null) {
            mPhoneStateListener = new PhoneStateListener() {
                    @Override
                    public void onServiceStateChanged(ServiceState serviceState) {
                        int curRat = serviceState.getRilVoiceRadioTechnology();
                        int dataRat = serviceState.getRilDataRadioTechnology();
                        if (logger.isActivated()) {
                            logger.debug("onServiceStateChanged"
                                         + ",voiceRat:" + curRat
                                         + ",dataRat:" + dataRat
                                         + ",mMobileRegRat:" + mMobileRegRat);
                        }
                        if ((dataRat != ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN)
                            && (dataRat > curRat)) {
                            curRat = dataRat;
                        }
                        if (mMobileRegRat != curRat) {
                            if (curRat == ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN) {
                                storeRegInfo();
                            } else {
                                mPlmn =  TelephonyManager.getDefault().getNetworkOperator();
                            }
                            mMobileRegRat = curRat;
                        }
                    }

                    @Override
                    public void onCellLocationChanged(CellLocation location) {
                        if (location instanceof GsmCellLocation) {
                            GsmCellLocation loc = (GsmCellLocation)location;
                            int lac = loc.getLac();
                            int cid = loc.getCid();
                            if (lac > 0) {
                                mLac = lac;
                            }
                            if (cid > 0) {
                                mCid = cid;
                            }
                        } else {
                            logger.debug("un-supported CellLocation type");
                        }
                    }
                };
        }
        Context context = Utils.getApplicationContext();
        TelephonyManager tm = (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
        tm.listen(mPhoneStateListener,
                  PhoneStateListener.LISTEN_CELL_LOCATION | PhoneStateListener.LISTEN_SERVICE_STATE);
    }

    private void unregisterNetworkCallback() {
        if (mPhoneStateListener != null) {
            Context context = Utils.getApplicationContext();
            TelephonyManager tm =
                (TelephonyManager)context.getSystemService(Context.TELEPHONY_SERVICE);
            tm.listen(mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        }
    }

    private void storeRegInfo() {
        if (mMobileRegRat != ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN) {
            mLastOosTime = System.currentTimeMillis();
            mLastRegRat = mMobileRegRat;
            mLastLac = mLac;
            mLastCid = mCid;
            mLastPlmn = mPlmn;
            Message.obtain(uaHandler, MSG_STORE_PLANI).sendToTarget();
        }
    }

    private boolean writeParameter(String key, String value) {
        Context context = Utils.getApplicationContext();
        SharedPreferences preferences =
            context.getSharedPreferences(RUA_SERVICE_PREFS_NAME, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        editor.putString(key, value);
        return editor.commit();
    }

    private String readParameter(String key, String defaultValue) {
        Context context = Utils.getApplicationContext();
        SharedPreferences preferences =
            context.getSharedPreferences(RUA_SERVICE_PREFS_NAME, Context.MODE_PRIVATE);
        return preferences.getString(key, defaultValue);
    }

    private String getNetworkAccessType(int type) {
        switch (type) {
            //2G NETWORK
            case ServiceState.RIL_RADIO_TECHNOLOGY_GPRS:
            case ServiceState.RIL_RADIO_TECHNOLOGY_EDGE:
            case ServiceState.RIL_RADIO_TECHNOLOGY_GSM:
            case ServiceState.RIL_RADIO_TECHNOLOGY_IS95A:
            case ServiceState.RIL_RADIO_TECHNOLOGY_IS95B:
            case ServiceState.RIL_RADIO_TECHNOLOGY_1xRTT:
                return GERAN;

            // 3G NETWORK
            case ServiceState.RIL_RADIO_TECHNOLOGY_UMTS:
            case ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_0:
            case ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_A:
            case ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA:
            case ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA:
            case ServiceState.RIL_RADIO_TECHNOLOGY_HSPA:
            case ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_B:
            case ServiceState.RIL_RADIO_TECHNOLOGY_EHRPD:
            case ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP:
            case ServiceState.RIL_RADIO_TECHNOLOGY_TD_SCDMA:
                return UTRAN_FDD;

            // 4G NETWORK
            case ServiceState.RIL_RADIO_TECHNOLOGY_LTE:
            case ServiceState.RIL_RADIO_TECHNOLOGY_LTE_CA:
                return E_UTRAN_FDD;

            // IWLAN: "IEEE-802.11n";
            case ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN:
            case ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN:
                break;

            default:
                break;
        }

        return null;
    }

    private String getPLANI(int regRat, int lac, int cid, String plmn) {
        String accessType = getNetworkAccessType(regRat);
        String accessInfo = "";
        if ((lac > 0) && (cid > 0) && (accessType != null)) {
            if (accessType.equals(UTRAN_FDD) || accessType.equals(E_UTRAN_FDD)) {
                // Both lac and cid are fixed length fields, filled with leading
                // zeros if needed.
                // The length of the fields are 16 and 28 bits, respectively
                accessInfo = accessType + SEMICOLON + UTRAN_CELL_ID
                    + EQUAL_SIGN + plmn
                    + String.format(STRING_FORMAT_FOR_UTRAN, lac, cid);
            } else if (accessType.equals(GERAN)) {
                // In 2G case both lac and cid are 16 bit fields
                accessInfo = accessType + SEMICOLON + CGI + EQUAL_SIGN
                    + plmn + String.format(STRING_FORMAT_FOR_CGI, lac, cid);
            } else {
                // Some other case, not sure about the field sizes here...
                accessInfo = accessType + SEMICOLON + CGI + EQUAL_SIGN
                    + plmn + String.format(STRING_FORMAT_FOR_CGI, lac, cid);
            }
        }

        return accessInfo;
    }

    private String toRFC3339DateFormat(long time) {
        String result = null;
        SimpleDateFormat timeFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssXXX");
        result = timeFormat.format(new Date(time));
        return result;
    }

    public void notifyVopsIndication(int vops) {
        if (mVops == vops) {
            logger.debug("ignore same vops:" + vops);
            return;
        }

        mVops = vops;
        if (isRoiCapable()) {
            RoiHandler roiHandler = RoiHandler.getInstance();
            if (roiHandler != null) {
                roiHandler.notifyVoPSChanged((vops == 1) ? true : false);
            } else {
                logger.debug("roi handler is null");
            }
        } else {
            logger.debug("roi not capable");
        }

        if (service != null) {
            service.notifyVopsIndication(vops);
        } else {
            logger.debug("servcie is null, ignore vops ind");
        }
    }

    public RcsRIL getRIL() {
        return mRil;
    }

    int getVops() {
        return mVops;
    }
}
