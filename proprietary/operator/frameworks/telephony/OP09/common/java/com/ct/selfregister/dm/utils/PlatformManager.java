package com.ct.selfregister.dm.utils;

import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.AlarmManager.OnAlarmListener;
import android.content.ContentResolver;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.os.Build;
import android.os.Environment;
import android.os.RemoteException;
import android.os.StatFs;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.telephony.CellInfo;
import android.telephony.CellInfoCdma;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoWcdma;
import android.telephony.CellLocation;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.ct.selfregister.dm.Const;
import com.mediatek.custom.CustomProperties;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import vendor.mediatek.hardware.nvram.V1_0.INvram;

public class PlatformManager {

    private static final String TAG = Const.TAG_PREFIX + "PlatformManager";

    private TelephonyWrapper mTelephonyWrapper;
    private ConnectivityWrapper mConnectivityWrapper;
    private MacAddrWrapper mMacAddrWrapper;
    private Context mContext;

    public PlatformManager(Context context) {
        mTelephonyWrapper = new TelephonyWrapper(context);
        mConnectivityWrapper = new ConnectivityWrapper(context);
        mMacAddrWrapper = new MacAddrWrapper();
        mContext = context;
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
        // Engineer mode -> telephony -> misc
        String config = SystemProperties.get(Const.PROPERTY_FEATURE_CONFIG, "11");
        if (!config.equals("11") && !config.equals("10") && !config.equals("01")
                && !config.equals("00")) {
            config = "11";
        }
        return config.charAt(1) == '1';
    }

    public static String encryptMessage(int value) {
        return encryptMessage("" + value);
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

    private boolean isInfoValid(String info, int length) {
        return (info != null && info.length() == length);
    }

    public static int getDefaultDataSubId() {
        return SubscriptionManager.getDefaultDataSubscriptionId();
    }

    public static int getDefaultSim() {
        int subId = getDefaultDataSubId();
        int slotId = SubscriptionManager.getSlotIndex(subId);
        Log.i(TAG, "getDefaultSim " + slotId);
        return slotId;
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

    /*
     * Get complex IMSI (For master,slave slot, get CDMA/LTE IMSI;)
     */
    public String[] getComplexImsi(int slotId) {
        String imsiArray[] = {Const.VALUE_EMPTY, Const.VALUE_EMPTY};

        int[] subId = getSubId(slotId);
        if (isSubIdsValid(subId)) {
            if (isCdma4GCard(slotId)) {
                // 4G CT, set both
                imsiArray[0] = getCdmaImsiForCT(slotId);
                imsiArray[1] = getSubscriberId(subId[0]);
            } else if (isCdma3GCard(slotId)) {
                // 3G CT, set CDMA IMSI
                imsiArray[0] = getCdmaImsiForCT(slotId);
            } else {
                // set LTE IMSI
                imsiArray[1] = getSubscriberId(subId[0]);
            }
        }

        Log.i(TAG, "[getComplexImsi] Slot " + slotId + " CDMA/LTE IMSI"
                + encryptMessage(imsiArray[0]) + "/" + encryptMessage(imsiArray[1]));
        return imsiArray;
    }

    //------------------------------------------------------
    // Wrapper for Connectivity
    //------------------------------------------------------
    public boolean hasNetworkConnection(int type) {
        return mConnectivityWrapper.hasNetworkConnection(type);
    }

    //------------------------------------------------------
    // Wrapper for Telephony
    //------------------------------------------------------
    public boolean areSlotsInfoReady(int[] slotArray) {
        return mTelephonyWrapper.areSlotsInfoReady(slotArray);
    }

    public boolean areValidOperatorSim(int[] slotArray) {
        return mTelephonyWrapper.areValidOperatorSim(slotArray);
    }

    public CellLocation getCellLocation(int slotId) {
        return mTelephonyWrapper.getCellLocation(slotId);
    }

    public String getDeviceMeid(int[] slotIdArray) {
        return mTelephonyWrapper.getDeviceMeid(slotIdArray);
    }

    public boolean isInvalidMeid(String meid) {
        return mTelephonyWrapper.isInvalidMeid(meid);
    }

    public String getImei(int slotId) {
        return mTelephonyWrapper.getImei(slotId);
    }

    public boolean isInvalidImei(String imei) {
        return mTelephonyWrapper.isInvalidImei(imei);
    }

    public boolean hasIccCard(int slotId) {
        return mTelephonyWrapper.hasIccCard(slotId);
    }

    public boolean isNetworkRoaming(int slotId) {
        return mTelephonyWrapper.isNetworkRoaming(slotId);
    }

    public boolean isSingleLoad() {
        return mTelephonyWrapper.isSingleLoad();
    }

    public boolean isValidUim(int slotId) {
        return mTelephonyWrapper.isValidUim(slotId);
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

    public boolean isInService(int slotId) {
        if (!hasIccCard(slotId)) {
            Log.i(TAG, "[isInService " + slotId + "] is empty, return false");
            return false;
        }

        int[] subId = getSubId(slotId);
        if (!isSubIdsValid(subId)) {
            Log.i(TAG, "[isInService " + slotId + "] sub not valid, return false");
            return false;
        }

        ServiceState serviceState = mTelephonyWrapper.getServiceState(subId[0]);
        return isInService(serviceState);
    }

    public boolean isInService(ServiceState serviceState ) {
        return (serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE)
                || (serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE);
    }

    public int getCellId(int slotId) {
        if (!hasIccCard(slotId)) {
            Log.i(TAG, "[getCellId " + slotId + "] is empty, return 0");
            return 0;
        }

        int[] subId = getSubId(slotId);
        if (!isSubIdsValid(subId)) {
            Log.i(TAG, "[getCellId " + slotId + "] sub not valid, return 0");
            return 0;
        }

        int cellId = 0;
        boolean isCTCard = isValidUim(slotId);

        // CS(voice) is 4G(LTE) or PS(data) is 4G(LTE)
        ServiceState serviceState = mTelephonyWrapper.getServiceState(subId[0]);
        boolean isLte = (serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE
                && ServiceState.isLte(serviceState.getRilVoiceRadioTechnology()))
                || (serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE
                        && ServiceState.isLte(serviceState.getRilDataRadioTechnology()));

        List<CellInfo> infoList = mTelephonyWrapper.getAllCellInfo(slotId);
        if (infoList != null && infoList.size() > 0) {
            for (CellInfo cellInfo : infoList) {
                if (cellInfo.isRegistered()) {
                    if (isCTCard && !isLte) {
                        if (cellInfo instanceof CellInfoCdma) {
                            cellId = ((CellInfoCdma) cellInfo).getCellIdentity().getBasestationId();
                            Log.i(TAG, "[getCellId " + slotId + "] Cdma baseId is "
                                    + encryptMessage(cellId));
                            break;
                        }
                    } else {
                        if (cellInfo instanceof CellInfoLte) {
                            cellId = ((CellInfoLte) cellInfo).getCellIdentity().getCi();
                            Log.i(TAG, "[getCellId " + slotId + "] Lte cellId is "
                                    + encryptMessage(cellId));
                            break;
                        } else if (cellInfo instanceof CellInfoWcdma) {
                            cellId = ((CellInfoWcdma) cellInfo).getCellIdentity().getCid();
                            Log.i(TAG, "[getCellId " + slotId + "] Wcdma cellId is "
                                    + encryptMessage(cellId));
                            break;
                        } else if (cellInfo instanceof CellInfoGsm) {
                            cellId = ((CellInfoGsm) cellInfo).getCellIdentity().getCid();
                            Log.i(TAG, "[getCellId " + slotId + "] Gsm cellId is "
                                    + encryptMessage(cellId));
                            break;
                        } else if (cellInfo instanceof CellInfoCdma) {
                            cellId = ((CellInfoCdma) cellInfo).getCellIdentity().getBasestationId();
                            Log.i(TAG, "[getCellId " + slotId + "] Cdma cellId is "
                                    + encryptMessage(cellId));
                            break;
                        }
                    }
                }
            }
        } else {
            if (infoList == null) {
                Log.i(TAG, "[getCellId " + slotId + "] infoList is null");
            } else {
                Log.i(TAG, "[getCellId " + slotId + "] infoList.size() " + infoList.size());
            }
        }
        Log.i(TAG, "[getCellId " + slotId + "] cellId is " + encryptMessage(cellId));
        return cellId;
    }

    //------------------------------------------------------
    // Wrapper for Telephony IMSI
    //------------------------------------------------------

    public String getCdmaImsiForCT(int slotId) {
        return mTelephonyWrapper.getCDMAImsi(slotId);
    }

    public String getSubscriberId(int subId) {
        return mTelephonyWrapper.getSubscriberId(subId);
    }

    //------------------------------------------------------
    // Logical process for IMSI
    //------------------------------------------------------

    private boolean isCdma4GCard(int slotId) {
        return mTelephonyWrapper.isCdma4GCard(slotId);
    }

    private boolean isCdma3GCard(int slotId) {
        return mTelephonyWrapper.isCdma3GCard(slotId);
    }

    private String[] getSupportCardType(int slotId) {
        return mTelephonyWrapper.getSupportCardType(slotId);
    }

    public String getProcessedIccid(String iccid) {
        if (!isInfoValid(iccid, TelephonyWrapper.LENGTH_ICCID)) {
            iccid = TelephonyWrapper.VALUE_DEFAULT_ICCID;
        }
        return iccid;
    }

    public String getIccId(int slotId) {
        return mTelephonyWrapper.getIccId(slotId);
    }

    //------------------------------------------------------
    // Function of Alarm
    //------------------------------------------------------

    public static void setRtcAlarm(Context context, OnAlarmListener listener, long delay) {
        setAlarm(context, listener, delay, AlarmManager.RTC_WAKEUP);
    }

    public static void setElapsedAlarm(Context context, OnAlarmListener listener, long delay) {
        setAlarm(context, listener, delay, AlarmManager.ELAPSED_REALTIME_WAKEUP);
    }

    public static void setAlarm(Context context, OnAlarmListener listener, long delay,
            int alarmType) {
        AlarmManager alarm = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);

        long triggerAtMillis = System.currentTimeMillis() + delay;
        if (alarmType != AlarmManager.RTC_WAKEUP) {
            triggerAtMillis = SystemClock.elapsedRealtime() + delay;
        }
        alarm.setExact(alarmType, triggerAtMillis, TAG, listener, null);
    }

    public static void cancelAlarm(Context context, OnAlarmListener listener) {
        AlarmManager alarm = (AlarmManager) context.getSystemService(Context.ALARM_SERVICE);
        alarm.cancel(listener);
    }

    //------------------------------------------------------
    //  MacAddress, RAM and ROM
    //------------------------------------------------------

    public String getMacAddress() {
        return mMacAddrWrapper.getMacAddress();
    }

    public String getRam() {
        ActivityManager activityManager = (ActivityManager) mContext.getSystemService(
                Context.ACTIVITY_SERVICE);
        ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
        activityManager.getMemoryInfo(memoryInfo);

        double size = 1.0 * memoryInfo.totalMem / 1024 / 1024 / 1024;
        int ram = (int) Math.ceil(size);
        Log.i(TAG, "[getRam] " + ram);
        return ram + "G";
    }

    public static String getRom() {
        long systemSize = getDirectorySize(Environment.getRootDirectory());
        long dataSize = getDirectorySize(Environment.getDataDirectory());

        Log.d(TAG, "systemSize:" + systemSize + "\n" + "dataSize:" + dataSize);

        long totalSize = analyseTotalStorage(systemSize + dataSize);

        return totalSize + "G";
    }

    /**
     * If the size locate in the section of 8-16, returns 16;
     * If the size locate in the section of 16-32, returns 32;
     * And so on.
     * @param size
     * @return Integer like "16", "32", "64", Unit: G.
     */
    private static int analyseTotalStorage(long size) {
        double total = ((double) size) / (1024 * 1024 * 1024);
        Log.d(TAG, "analyseTotalStorage(), total: " + total);

        int storageSize = 1;
        while (total > storageSize) {
            storageSize = storageSize << 1;
        }

        return storageSize;
    }

    @SuppressWarnings("deprecation")
    private static long getDirectorySize(File path) {
        StatFs stat = new StatFs(path.getPath());
        long blockSize = stat.getBlockSize();
        long totalBlocks = stat.getBlockCount();
        return blockSize * totalBlocks;
    }

    public static String getDate() {
        String date = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date());
        return date;
    }

    // ------------------------------------------------------
    //     CustomProperties
    // ------------------------------------------------------

    public static String getManufacturer() {
        String manufacturer = CustomProperties.getString(CustomProperties.MODULE_DM,
                CustomProperties.MANUFACTURER, Const.VALUE_DEFAULT_MANUFACTURER);
        return manufacturer;
    }

    public static String getSoftwareVersion() {
        String version = CustomProperties.getString(CustomProperties.MODULE_DM,
                "SoftwareVersion", Const.VALUE_DEFALUT_SOFTWARE_VERSION);
        return version;
    }

    private class MacAddrWrapper {

        private static final String DEFAULT_MAC_ADDRESS = "01:02:03:04:05:06";

        private static final String MAC_ADDRESS_FILENAME = "/mnt/vendor/nvdata/APCFG/APRDEB/WIFI";
        private static final int MAC_ADDRESS_OFFSET = 4;
        private static final int MAC_ADDRESS_DIGITS = 6;

        public String getMacAddress() {
            String result = DEFAULT_MAC_ADDRESS;
            StringBuffer nvramBuf = new StringBuffer();
            try {
                int i = 0;
                String buff = null;
                INvram agent = INvram.getService();
                if (agent != null) {
                    buff = agent.readFileByName(
                            MAC_ADDRESS_FILENAME, MAC_ADDRESS_OFFSET + MAC_ADDRESS_DIGITS);

                    Log.i(TAG, "Raw data:" + encryptMessage(buff));
                    if (buff.length() >= 2 * (MAC_ADDRESS_OFFSET + MAC_ADDRESS_DIGITS)) {
                        // Remove the \0 special character.
                        int macLen = buff.length() - 1;
                        for (i = MAC_ADDRESS_OFFSET * 2; i < macLen; i += 2) {
                            if ((i + 2) < macLen) {
                                nvramBuf.append(buff.substring(i, i + 2));
                                nvramBuf.append(":");
                            } else {
                                nvramBuf.append(buff.substring(i));
                            }
                        }
                        result = nvramBuf.toString();
                    } else {
                        Log.e(TAG, "Fail to read mac address");
                    }
                } else {
                    Log.e(TAG, "Nvram is null");
                }

            } catch (RemoteException e) {
                e.printStackTrace();
            }

            if (result.length() > DEFAULT_MAC_ADDRESS.length()) {
                // remove extra characters if length longer than expected
                result = result.substring(0, DEFAULT_MAC_ADDRESS.length());

            } else if (result.length() < DEFAULT_MAC_ADDRESS.length()) {
                // set to default if length shorted than expected
                result = DEFAULT_MAC_ADDRESS;
            }
            Log.d(TAG, "result: " + encryptMessage(result));
            return result;
        }
    }

    private class ConnectivityWrapper {

        private static final String TAG = Const.TAG_PREFIX + "ConnectivityWrapper";

        private ConnectivityManager mConnectivityManager;
        private Context mContext;

        public ConnectivityWrapper(Context context) {
            mConnectivityManager = (ConnectivityManager) context.getSystemService(
                Context.CONNECTIVITY_SERVICE);
            mContext = context;
        }

        public boolean hasNetworkConnection(int type) {
            if (mConnectivityManager == null) {
                mConnectivityManager = (ConnectivityManager) mContext.getSystemService(
                    Context.CONNECTIVITY_SERVICE);
                Log.e(TAG, "mConnectivityManager is null, reinit");
            }

            Network[] networks = mConnectivityManager.getAllNetworks();
            for (Network network: networks) {
                NetworkInfo networkInfo = mConnectivityManager.getNetworkInfo(network);

                if (networkInfo != null && networkInfo.getType() == type
                    && networkInfo.isConnected()) {
                    Log.i(TAG, "[hasNetworkConnection] " + type + " is connected");
                    return true;
                }
            }
            return false;
        }
    }

    private class TelephonyWrapper {

        private static final String TAG = Const.TAG_PREFIX + "TelephonyWrapper";

        private static final String VALUE_DEFAULT_MEID = "A0000100001000";
        private static final String VALUE_DEFAULT_IMEI = "860001000010000";
        public static final String VALUE_DEFAULT_IMSI = "000000000000000";
        public static final String VALUE_DEFAULT_ICCID = "00000000000000000000";

        private static final int LENGTH_MEID = 14;
        private static final int LENGTH_IMEI = 15;
        public static final int LENGTH_ICCID = 20;
        public static final int LENGTH_IMSI = 15;
        private static final int LENGTH_OPERATOR = 5;

        private final String[] VALID_NETWORK_OPERATOR = {
                "46003", "46011", "45502"
        };
        private final String[] VALID_SIM_OPERATOR = {
                "46003", "46011", "20404", "45403", "45431"
        };

        private final String[] INVALID_MEID = {
                VALUE_DEFAULT_MEID, "00000000000000", ""
        };

        private final String[] INVALID_IMEI = {
                VALUE_DEFAULT_IMEI, "000000000000000", ""
        };

        private final String PREFIX_CN_PLMN = "460";
        private final String[] SPECIAL_CN_PLMN = {
                "45407",  // for CU
                "20404", "45403", "45431"  // for CT
        };

        private TelephonyManager mTelephonyManager;
        private MtkTelephonyManagerEx mMtkTelephonyManagerEx;

        private String[] PROPERTY_ICCID_SIM = {
            "vendor.ril.iccid.sim1",
            "vendor.ril.iccid.sim2",
            "vendor.ril.iccid.sim3",
            "vendor.ril.iccid.sim4",
            };

        public TelephonyWrapper(Context context) {
            mTelephonyManager = (TelephonyManager) context
                .getSystemService(Context.TELEPHONY_SERVICE);
            mMtkTelephonyManagerEx = new MtkTelephonyManagerEx(context);
            if (mTelephonyManager == null) {
                throw new Error("telephony manager is null");
            }
        }


        // ------------------------------------------------------
        //    Wrapper for SystemProperties
        // ------------------------------------------------------

        public String getIccId(int slotId) {
            String value = SystemProperties.get(PROPERTY_ICCID_SIM[slotId], "");

            if (isInfoValid(value, LENGTH_ICCID)) {
                return value;
            }
            return VALUE_DEFAULT_ICCID;
        }

        // ------------------------------------------------------
        //    Wrapper for TelephonyManager
        // ------------------------------------------------------

        public String getDeviceMeid(int[] slots) {
            for (int slotId: slots) {
                String result = mTelephonyManager.getMeid(slotId);
                if (isInfoValid(result, LENGTH_MEID)) {
                    return result;
                }
            }
            return VALUE_DEFAULT_MEID;
        }

        public boolean isInvalidMeid(String meid) {
            return hasElement(INVALID_MEID, meid);
        }

        public String getImei(int slotId) {
            String result = mTelephonyManager.getImei(slotId);
            if (isInfoValid(result, LENGTH_IMEI)) {
                return result;
            }
            return VALUE_DEFAULT_IMEI;
        }

        public boolean isInvalidImei(String imei) {
            return hasElement(INVALID_IMEI, imei);
        }

        public ServiceState getServiceState(int subId) {
            return mTelephonyManager.getServiceStateForSubscriber(subId);
        }

        public String getNetworkOperator(int subId) {
            return mTelephonyManager.getNetworkOperator(subId);
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

        public boolean isSingleLoad() {
            return (mTelephonyManager.getSimCount() == 1);
        }

        public boolean isNetworkRoaming(int slotId) {
            int[] subId = getSubId(slotId);

            if (isSubIdsValid(subId)) {
                return mTelephonyManager.isNetworkRoaming(subId[0]);
            }
            return false;
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

        public CellLocation getCellLocation(int slotId) {
            return mMtkTelephonyManagerEx.getCellLocation(slotId);
        }

        public List<CellInfo> getAllCellInfo(int slotId) {
            return mMtkTelephonyManagerEx.getAllCellInfo(slotId);
        }

        public String[] getSupportCardType(int slotId) {
            return mMtkTelephonyManagerEx.getSupportCardType(slotId);
        }

        public String getUimSubscriberId(int subId) {
            return mMtkTelephonyManagerEx.getUimSubscriberId(subId);
        }

        public boolean isCdma4GCard(int slotId) {
            return getIccAppFamily(slotId) == (MtkTelephonyManagerEx.APP_FAM_3GPP |
                MtkTelephonyManagerEx.APP_FAM_3GPP2);
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
        public boolean areSlotsInfoReady(int[] slotId) {
            for (int i = 0; i < slotId.length; ++i) {
                if (!isSlotInfoReady(slotId[i])) {
                    Log.i(TAG, "isSlotInfoReady " + slotId[i] + " not ready.");
                    return false;
                }
            }
            return true;
        }

        private boolean isSlotInfoReady(int slotId) {
            if (hasIccCard(slotId)) {
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
                }
            } else {
                // No SIM for this slot
                return true;
            }
            return false;
        }

        /**
         * Whether uim's network operator, UIM operator and phone type is correct
         *
         * @param slotId
         * @return true or false
         */
        public boolean isValidUim(int slotId) {
            int[] subId = getSubId(slotId);
            if (!isSubIdsValid(subId)) {
                return false;
            }

            // After register to VoLTE, not check phone type as it may not be CDMA
            String networkOperator = getNetworkOperator(subId[0]);
            Log.i(TAG, "[isValidUim] slot " + slotId + " network operator " + networkOperator);

            if (hasElement(VALID_NETWORK_OPERATOR, networkOperator)) {
                String simOperator = getSimOperator(subId[0]);
                Log.i(TAG, "[isValidUim] slot " + slotId + " sim operator " + simOperator);

                if (hasElement(VALID_SIM_OPERATOR, simOperator)) {
                    return true;
                }
            }
            return false;
        }

        public boolean areValidOperatorSim(int[] slotId) {
            for (int i = 0; i < slotId.length; ++i) {
                if (!isValidOperatorSim(slotId[i])) {
                    Log.i(TAG, "isValidOperatorSim " + slotId[i] + " false.");
                    return false;
                }
            }
            return true;
        }

        private boolean isValidOperatorSim(int slotId) {
            if (hasIccCard(slotId)) {
                int[] subId = getSubId(slotId);

                if (isSubIdsValid(subId)) {
                    String simOperator = getSimOperator(subId[0]);

                    if (isValidPlmn(simOperator)) {
                        return true;
                    } else {
                        Log.i(TAG, "simOptr " + simOperator + " not valid.");
                    }
                }
            } else {
                // No SIM for this slot
                return true;
            }
            return false;
        }

        private boolean isValidPlmn(String plmn) {
            if (plmn != null) {
                if (plmn.startsWith(PREFIX_CN_PLMN) || hasElement(SPECIAL_CN_PLMN, plmn)) {
                    return true;
                }
            }
            return false;
        }
    }

    private boolean hasElement(String[] array, String element) {
        return Arrays.asList(array).contains(element);
    }

}
