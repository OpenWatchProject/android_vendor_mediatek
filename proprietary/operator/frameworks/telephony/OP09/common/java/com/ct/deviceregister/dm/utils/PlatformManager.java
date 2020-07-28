/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.ct.deviceregister.dm.utils;

import android.app.AlarmManager;
import android.app.AlarmManager.OnAlarmListener;
import android.app.PendingIntent;
import android.content.Context;
import android.os.Build;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SmsMessage;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.ct.deviceregister.dm.Const;
import com.ct.deviceregister.dm.RegisterMessage;
import com.mediatek.custom.CustomProperties;
import com.mediatek.ims.MtkImsConnectionStateListener;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.devreg.DeviceRegisterController;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.util.Arrays;

public class PlatformManager {

    private static final String TAG = Const.TAG_PREFIX + "PlatformManager";

    private static final String VALUE_DEFAULT_MANUFACTURER = "MTK";
    private static final String VALUE_DEFALUT_SOFTWARE_VERSION = Build.MODEL + ".P1";

    private static final String SERVER_ADDRESS = "10659401";
    private static final short PORT = 0;

    private TelephonyWrapper mTelephonyWrapper;

    public PlatformManager(Context context) {
        mTelephonyWrapper = new TelephonyWrapper(context);
    }

    public static boolean isFeatureSupported() {
        int property = SystemProperties.getInt(Const.PROPERTY_FEATURE_SUPPORT, -1);
        if (property != 1) {
            Log.i(TAG, "[isFeatureSupported] not support");
            return false;
        }
        return true;
    }

    public static boolean isFeatureEnabled() {
        // Hardware Testing -> Device Register -> Send Auto Register Message
        String config = SystemProperties.get(Const.PROPERTY_FEATURE_CONFIG, "11");
        if (!config.equals("11") && !config.equals("10") && !config.equals("01")
                && !config.equals("00")) {
            config = "11";
        }
        return config.charAt(0) == '1';
    }

    public static String getVersion() {
        String defaultVersion = Const.VERSION_2019V1;
        String version = SystemProperties.get(Const.PROPERTY_SELFREG_VERSION, defaultVersion);
        Log.i(TAG, "getVersion is " + version);
        if (!version.equals(Const.VERSION_2019V1)
                && !version.equals(Const.VERSION_2018V2)) {
            Log.i(TAG, "Use default " + defaultVersion);
            version = defaultVersion;
        }
        return version;
    }

    public static String encryptMessage(String[] array) {
        if (array == null || array.length == 0) {
            return "";
        }
        return encryptMessage(Arrays.toString(array));
    }

    public static String encryptMessage(String text) {
        if (text == null || text.length() == 0) {
            return text;
        }

        String result = createAsterisks(text.length() / 2)
                + text.substring(text.length() / 2);
        return result;
    }

    private static String createAsterisks(int length) {
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < length; i++) {
            builder.append("*");
        }
        return builder.toString();
    }

    public static void setElapsedAlarm(Context context, OnAlarmListener listener, long delay) {
        AlarmManager alarm = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);

        long triggerAtMillis = SystemClock.elapsedRealtime() + delay;
        alarm.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP, triggerAtMillis, TAG, listener, null);
    }

    public static void cancelAlarm(Context context, OnAlarmListener listener) {
        AlarmManager alarm = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        alarm.cancel(listener);
    }

    public String getProcessedImsi(String imsi) {
        if (!isInfoValid(imsi, TelephonyWrapper.LENGTH_IMSI)) {
            imsi = TelephonyWrapper.VALUE_DEFAULT_IMSI;
        }
        return imsi;
    }

    public static int getSlotId(int subId) {
        return SubscriptionManager.getSlotIndex(subId);
    }

    /*
     * Methods related to subId
     */
    public static int[] getSubId(int slotId) {
        return SubscriptionManager.getSubId(slotId);
    }

    public static boolean isSubIdsValid(int[] subId) {
        if (subId == null || subId[0] < 0) {
            if (subId == null) {
                Log.e(TAG, "subId is " + subId);
            } else {
                Log.e(TAG, "subId not valid, subId[0] is " + subId[0]);
            }
            return false;
        }
        return true;
    }

    public static boolean isSlotValid(int slotId) {
        return slotId == Const.SLOT_ID_0 || slotId == Const.SLOT_ID_1;
    }

    /*
     * Methods related to info check, compare
     */
    public static boolean isInfoValid(String info, int length) {
        return (info != null && info.length() == length);
    }

    public int getDefaultSim() {
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        Log.i(TAG, "getDefaultSim " + slotId);
        return slotId;
    }

    //------------------------------------------------------
    // Wrapper for Telephony
    //------------------------------------------------------
    public boolean areSlotsInfoReady(int[] slots) {
        return mTelephonyWrapper.areSlotsInfoReady(slots);
    }

    public String getCdmaImsiForCT(int slotId) {
        return mTelephonyWrapper.getCDMAImsi(slotId);
    }

    public String getDeviceMeid(int[] slots) {
        return mTelephonyWrapper.getDeviceMeid(slots);
    }

    public String getImei(int slotId) {
        return mTelephonyWrapper.getImei(slotId);
    }

    public String getImsiInfo(int slotId) {
        return mTelephonyWrapper.getImsi(slotId);
    }

    public boolean isImsRegistered(int slotId) {
        int[] subId = getSubId(slotId);

        if (isSubIdsValid(subId)) {
            return mTelephonyWrapper.isImsRegistered(subId[0]);
        }
        return false;
    }

    public boolean hasIccCard(int slotId) {
        return mTelephonyWrapper.hasIccCard(slotId);
    }

    public boolean isNetworkRoaming(int slotId) {
        int[] subId = getSubId(slotId);

        if (isSubIdsValid(subId)) {
            return mTelephonyWrapper.isNetworkRoaming(subId[0]);
        }
        return false;
    }

    public boolean isSingleLoad() {
        return mTelephonyWrapper.isSingleLoad();
    }

    public boolean isValidUimForIms(int slotId) {
        return mTelephonyWrapper.isValidUimForIms(slotId);
    }

    public boolean isValidCdmaUim(int slotId) {
        return mTelephonyWrapper.isValidCdmaUim(slotId);
    }

    public boolean isValidCdmaNetwork(int slotId) {
        int[] subId = getSubId(slotId);
        if (!isSubIdsValid(subId)) {
            return false;
        }

        ServiceState serviceState = mTelephonyWrapper.getServiceState(subId[0]);
        return isValidCdmaNetwork(serviceState);
    }

    public boolean isValidCdmaNetwork(ServiceState serviceState) {
        int rilVoiceRadioType = serviceState.getRilVoiceRadioTechnology();
        int rilDataRadioType = serviceState.getRilDataRadioTechnology();

        boolean isVoiceReady = serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE
                && ServiceState.isCdma(rilVoiceRadioType);
        boolean isDataReady = serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE
                && ServiceState.isCdma(rilDataRadioType);
        boolean result = isVoiceReady || isDataReady;
        Log.i(TAG, "isValidCdmaNetwork " + result);
        return result;
    }

    public boolean isValidLteNetwork(ServiceState serviceState) {
        int rilVoiceRadioType = serviceState.getRilVoiceRadioTechnology();
        int rilDataRadioType = serviceState.getRilDataRadioTechnology();
        Log.i(TAG, "isValidLteNetwork " + rilVoiceRadioType + "/" + rilDataRadioType);

        boolean isVoiceReady = serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE
                && ServiceState.isLte(rilVoiceRadioType);
        boolean isDataReady = serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE
                && ServiceState.isLte(rilDataRadioType);
        boolean result = isVoiceReady || isDataReady;
        Log.i(TAG, "isValidLteNetwork " + result);
        return result;
    }

    public void registerPhoneListener(PhoneStateListener listener, int subId) {
        if (listener != null) {
            mTelephonyWrapper.registerPhoneListener(listener, subId);
        } else {
            Log.i(TAG, "[register] listener is null, do nothing");
        }
    }

    public void unRegisterPhoneListener(PhoneStateListener listener, int subId) {
        if (listener != null) {
            mTelephonyWrapper.unRegisterPhoneListener(listener, subId);
        } else {
            Log.i(TAG, "[unRegister] listener is null, do nothing");
        }
    }

    public void addImsConnectionStateListener(ImsManager imsManager, MtkImsConnectionStateListener
            listener) {
        try {
            ((MtkImsManager) imsManager).addImsConnectionStateListener(listener);
            Log.i(TAG, "register ims succeed");
        } catch (ImsException e) {
            // Could not get the ImsService.
            Log.e(TAG,"register ims fail!");
        }
    }

    //------------------------------------------------------
    // Methods related to CustomProperties
    //------------------------------------------------------
    public static String getManufacturer() {
        String manufacturer = CustomProperties.getString(CustomProperties.MODULE_DM,
                CustomProperties.MANUFACTURER, VALUE_DEFAULT_MANUFACTURER);
        return manufacturer;
    }

    public static String getSoftwareVersion() {
        String version = CustomProperties.getString(CustomProperties.MODULE_DM,
                "SoftwareVersion", VALUE_DEFALUT_SOFTWARE_VERSION);
        return version;
    }

    /*
     * Wrapper class to send message and check register result
     */
    public static class SmsWrapper {

        private static final byte COMMAND_TYPE_RECEIVED = RegisterMessage.COMMAND_TYPE_RECEIVED;

        public static void sendRegisterMessage(DeviceRegisterController controller,
                byte[] message, PendingIntent intent, int slotId) {

            int[] subId = getSubId(slotId);
            if (isSubIdsValid(subId)) {
                // Don't use interface of SmsManager which adds a header, as operator won't send
                // the feedback if receiving a message with header.
                controller.sendDataSms(subId[0],
                        SERVER_ADDRESS, null, PORT, PORT, message, intent, null);
            } else {
                Log.e(TAG, "[sendRegisterMessage] Invalid sub id, not send message");
            }
        }

        /*
         * get data from intent, and analyze it to check if register is successful.
         */
        public static boolean checkRegisterResult(byte[] pduByte) {
            return checkRegisterResult(SmsMessage.FORMAT_3GPP2, pduByte);
        }

        public static boolean checkRegisterResult(String format, byte[] pduByte) {
            if (pduByte == null || pduByte.length == 0) {
                Log.i(TAG, "[checkRegisterResult] Pdu is invalid!");
                return false;
            }

            if (!format.equals(SmsMessage.FORMAT_3GPP)
                    && !format.equals(SmsMessage.FORMAT_3GPP2)) {
                Log.i(TAG, "[checkRegisterResult] invalid format " + format);
                return false;
            }

            // parse the raw pdu
            SmsMessage message = SmsMessage.createFromPdu(pduByte, format);
            String address = message.getOriginatingAddress();

            // confirm message address
            if (!address.equals(SERVER_ADDRESS)) {
                Log.i(TAG, "[checkRegisterResult] invalid address " + address);
                return false;
            }

            byte[] data = message.getUserData();
            Log.i(TAG, "message user data:" + Utils.bytesToHexString(data));

            if (data != null && data.length > 1) {
                byte confirmByte = data[1];
                if (confirmByte == COMMAND_TYPE_RECEIVED) {
                    Log.i(TAG, "Register success!");
                    return true;
                }
            } else {
                Log.i(TAG, "[checkRegisterResult] Invalid data");
            }
            return false;
        }
    }

    private class TelephonyWrapper {

        private static final String TAG = Const.TAG_PREFIX + "TelephonyWrapper";

        private static final String VALUE_DEFAULT_MEID = "A0000100002000";
        private static final String VALUE_DEFAULT_IMEI = "860001000020000";
        public static final String VALUE_DEFAULT_IMSI = "000001000030000";

        private static final int LENGTH_MEID = Const.LENGTH_MEID;
        private static final int LENGTH_IMEI = 15;
        public static final int LENGTH_IMSI = 15;
        private static final int LENGTH_OPERATOR = 5;

        private final String[] VALID_NETWORK_OPERATOR = {
                "46003", "46011", "45502"
        };
        private final String[] VALID_SIM_OPERATOR = {
                "46003", "46011", "20404", "45403", "45431"
        };

        private Context mContext;
        private TelephonyManager mTelephonyManager;
        private MtkTelephonyManagerEx mMtkTelephonyManagerEx;

        public TelephonyWrapper(Context context) {
            mContext = context;
            mTelephonyManager = (TelephonyManager) context
                    .getSystemService(Context.TELEPHONY_SERVICE);
            mMtkTelephonyManagerEx = new MtkTelephonyManagerEx(context);
            if (mTelephonyManager == null) {
                throw new Error("telephony manager is null");
            }
        }

        // ------------------------------------------------------
        //    Wrapper for TelephonyManager
        // ------------------------------------------------------
        public boolean isSingleLoad() {
            return (mTelephonyManager.getSimCount() == 1);
        }

        public boolean isNetworkRoaming(int subId) {
            return mTelephonyManager.isNetworkRoaming(subId);
        }

        public String getDeviceMeid(int[] slots) {
            for (int slotId: slots) {
                String result = mTelephonyManager.getMeid(slotId);
                if (isInfoValid(result, LENGTH_MEID)) {
                    return result;
                }
            }
            return VALUE_DEFAULT_MEID;
        }

        public String getNetworkOperator(int subId) {
            return mTelephonyManager.getNetworkOperator(subId);
        }

        public int getCurrentPhoneType(int subId) {
            return mTelephonyManager.getCurrentPhoneType(subId);
        }

        public String getImei(int slotId) {
            String result = mTelephonyManager.getImei(slotId);
            if (isInfoValid(result, LENGTH_IMEI)) {
                return result;
            }
            return VALUE_DEFAULT_IMEI;
        }

        public ServiceState getServiceState(int subId) {
            return mTelephonyManager.getServiceStateForSubscriber(subId);
        }

        public String getSimOperator(int subId) {
            return mTelephonyManager.getSimOperator(subId);
        }

        public String getSubscriberId(int subId) {
            return mTelephonyManager.getSubscriberId(subId);
        }

        public boolean hasIccCard(int slotId) {
            return mTelephonyManager.hasIccCard(slotId);
        }

        public void registerPhoneListener(PhoneStateListener listener, int subId) {
            TelephonyManager telephonyManager = new TelephonyManager(mContext, subId);
            telephonyManager.listen(listener, PhoneStateListener.LISTEN_NONE);
            telephonyManager.listen(listener, PhoneStateListener.LISTEN_SERVICE_STATE);
        }

        public void unRegisterPhoneListener(PhoneStateListener listener, int subId) {
            TelephonyManager telephonyManager = new TelephonyManager(mContext, subId);
            telephonyManager.listen(listener, PhoneStateListener.LISTEN_NONE);
        }

        // ------------------------------------------------------
        //    Wrapper for MtkTelephonyManagerEx
        // ------------------------------------------------------

        public int getIccAppFamily(int slotId) {
            int iccType = mMtkTelephonyManagerEx.getIccAppFamily(slotId);
            Log.i(TAG, "Slot " + slotId + " iccType is : " + iccType);
            return iccType;
        }

        public String getUimSubscriberId(int subId) {
            return mMtkTelephonyManagerEx.getUimSubscriberId(subId);
        }

        public boolean isImsRegistered(int subId) {
            return mMtkTelephonyManagerEx.isImsRegistered(subId);
        }

        // ------------------------------------------------------
        //    IMSI related
        // ------------------------------------------------------

        /**
         * if 3G CT, CDMA IMSI; else, IMSI
         */
        public String getImsi(int slotId) {
            String result = VALUE_DEFAULT_IMSI;
            int[] subId = getSubId(slotId);

            if (isSubIdsValid(subId)) {
                if (isCdma3GCard(slotId)) {
                    result = getUimSubscriberId(subId[0]);
                } else {
                    result = getSubscriberId(subId[0]);
                }
            }

            if (isInfoValid(result, LENGTH_IMSI)) {
                return result;
            }
            return VALUE_DEFAULT_IMSI;
        }

        public boolean isCdma3GCard(int slotId) {
            return getIccAppFamily(slotId) == MtkTelephonyManagerEx.APP_FAM_3GPP2;
        }

        public String getCDMAImsi(int slotId) {
            String result = VALUE_DEFAULT_IMSI;;
            int[] subId = getSubId(slotId);

            if (isSubIdsValid(subId)) {
                result = getUimSubscriberId(subId[0]);
            }

            if (isInfoValid(result, LENGTH_IMSI)) {
                return result;
            }
            return VALUE_DEFAULT_IMSI;
        }

        // ------------------------------------------------------
        //    check SIM state
        // ------------------------------------------------------
        public boolean areSlotsInfoReady(int[] slots) {
            for (int i = 0; i < slots.length; ++i) {
                if (!isSlotInfoReady(slots[i])) {
                    return false;
                }
            }
            return true;
        }

        private boolean isSlotInfoReady(int slotId) {
            int[] subId = getSubId(slotId);

            if (isSubIdsValid(subId)) {
                String networkOperator = getNetworkOperator(subId[0]);
                String simOperator = getSimOperator(subId[0]);

                if (isInfoValid(networkOperator, LENGTH_OPERATOR)
                        && isInfoValid(simOperator, LENGTH_OPERATOR)) {
                    return true;
                } else {
                    Log.i(TAG, "networkOptr/simOptr not all ready");
                }
            } else {
                // no valid sub, return true directly
                return true;
            }
            return false;
        }

        /**
         * Whether uim's network operator and UIM operator (not check phone type)
         *
         * @param slotId
         * @return true or false
         */
        public boolean isValidUimForIms(int slotId) {
            int[] subId = getSubId(slotId);
            if (!isSubIdsValid(subId)) {
                return false;
            }

            int phoneType = getCurrentPhoneType(subId[0]);
            Log.i(TAG, "[isValidUimForIms] slot " + slotId + " phone type " + phoneType);
            if (phoneType == TelephonyManager.PHONE_TYPE_CDMA) {
                Log.i(TAG, "[isValidUimForIms] slot " + slotId + " is for CDMA");
                return false;
            }

            String networkOperator = getNetworkOperator(subId[0]);
            Log.i(TAG, "[isValidUimForIms] slot " + slotId + " network " + networkOperator);

            if (hasElement(VALID_NETWORK_OPERATOR, networkOperator)) {
                String simOperator = getSimOperator(subId[0]);
                Log.i(TAG, "[isValidUimForIms] slot " + slotId + " sim " + simOperator);

                if (hasElement(VALID_SIM_OPERATOR, simOperator)) {
                    return true;
                }
            }
            return false;
        }

        /**
         * Whether uim's network operator, UIM operator and phone type is correct
         *
         * @param slotId
         * @return true or false
         */
        public boolean isValidCdmaUim(int slotId) {
            int[] subId = getSubId(slotId);
            if (!isSubIdsValid(subId)) {
                return false;
            }

            int phoneType = getCurrentPhoneType(subId[0]);
            Log.i(TAG, "[isValidCdmaUim] slot " + slotId + " phone type " + phoneType);

            if (TelephonyManager.PHONE_TYPE_CDMA == phoneType) {
                String networkOperator = getNetworkOperator(subId[0]);
                Log.i(TAG, "[isValidCdmaUim] slot " + slotId + " network " + networkOperator);

                if (hasElement(VALID_NETWORK_OPERATOR, networkOperator)) {
                    String simOperator = getSimOperator(subId[0]);
                    Log.i(TAG, "[isValidCdmaUim] slot " + slotId + " sim " + simOperator);

                    if (hasElement(VALID_SIM_OPERATOR, simOperator)) {
                        return true;
                    }
                }
            }
            return false;
        }
    }

    private boolean hasElement(String[] array, String element) {
        return Arrays.asList(array).contains(element);
    }

}
