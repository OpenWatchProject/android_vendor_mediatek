package com.dmyk.android.telephony;

import android.bluetooth.BluetoothAdapter;
import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.media.AudioManager;
import android.net.ConnectivityManager;
import android.net.INetworkStatsService;
import android.net.INetworkStatsSession;
import android.net.NetworkTemplate;
import android.net.NetworkInfo;
import android.net.TrafficStats;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.PowerManager;
import android.os.Process;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.CellLocation;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.ims.ImsManager;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.view.RotationPolicy;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.lang.reflect.Method;

import com.mediatek.custom.CustomProperties;
import com.mediatek.telephony.MtkTelephonyManagerEx;

public final class DmykTelephonyManager extends DmykAbsTelephonyManager {
    private static final int LENGTH_MEID = 14;
    private static final int LENGTH_ICCID = 20;
    private static final String PROPERTY_MULTIPLE_IMS_SUPPORT = "persist.vendor.mims_support";
    private static final String FILENAME_MSV = "/sys/board_properties/soc/msv";

    public static final class MLog {
        private static final String TAG = "CTM";
        private static final boolean sLogEnabled = true;
        private static final boolean isUserLoad = "user".equals(Build.TYPE);

        public static boolean isLogEnabled(){
           return sLogEnabled && (!isUserLoad);
        }
        public static void v(String msg) {
            if (isLogEnabled()) {
                android.util.Log.v(TAG, msg);
            }
        }
        public static void d(String msg) {
            if (isLogEnabled()) {
                android.util.Log.d(TAG, msg);
            }
        }
        public static void i(String msg) {
            if (isLogEnabled()) {
                android.util.Log.i(TAG, msg);
            }
        }
        public static void w(String msg) {
            if (isLogEnabled()) {
                android.util.Log.w(TAG, msg);
            }
        }
        public static void e(String msg) {
            if (isLogEnabled()) {
                android.util.Log.e(TAG, msg);
            }
        }
        public static void wtf(String msg) {
            if (isLogEnabled()) {
                android.util.Log.wtf(TAG, msg);
            }
        }
    }

    private Context mContext;
    private TelephonyManager mTelephonyManager = null;
    private SwitchState mSwitchState = null;
    private LocationMsg mLocation = null;
    private LocationManager mLocationManager = null;
    private final Object mWaitLock = new Object();
    private static final int NETWORK_LOCATION_UPDATE_TIME = 1000;

    public DmykTelephonyManager(Context context) {
        MLog.d("DmykTelephonyManager instantiated");
        if (context == null) {
            MLog.e("DmykTelephonyManager: context is null!");
        }
        mContext = context;
        mSwitchState = new SwitchState();
        mLocation = new LocationMsg();
        mLocationManager = (LocationManager) mContext
                .getSystemService(Context.LOCATION_SERVICE);
    }

    private TelephonyManager getTelephonyManager() {
        if (mTelephonyManager == null) {
            if (mContext != null) {
                mTelephonyManager =
                    (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            }
        }
        return mTelephonyManager;
    }

    @Override
    public int getPhoneCount() {
        int result = getTelephonyManager().getSimCount();
        MLog.d("getPhoneCount(): " + result);
        return result;
    }

    @Override
    public String getGsmDeviceId(int phoneId) {
        String result = null;
        if (phoneId >= 0 && phoneId < 2) {
            // the sole parameter of getImei is slotId
            result = getTelephonyManager().getImei(phoneId);
        }
        MLog.d("getGsmDeviceId(" + phoneId + "): " + getSubString(result));
        return result;
    }

    @Override
    public String getCdmaDeviceId() {
        // iterate over all phones to get a valid MEID
        for (int i = 0; i < getPhoneCount(); i++) {
            String result = getTelephonyManager().getMeid(i);
            // sanity check
            if (result != null && result.length() == LENGTH_MEID) {
                MLog.d("getCdmaDeviceId(): " + getSubString(result));
                return result;
            }
        }
        MLog.d("getCdmaDeviceId(): null");
        return null;
    }

    @Override
    public String getSubscriberId(int phoneId) {
        String result = null;
        if (phoneId >= 0 && phoneId < 2) {
            int subId = getSubIdForSlotId(phoneId);
            if (subId >= 0) {
                result = getTelephonyManager().getSubscriberId(subId);
            }
        }
        MLog.d("getSubscriberId(" + phoneId + "): " + getSubString(result));
        return result;
    }

    @Override
    public String getIccId(int phoneId) {
        String result = null;
        MtkTelephonyManagerEx tme = MtkTelephonyManagerEx.getDefault();
        if (phoneId >= 0 && phoneId < 2) {
            result = tme.getSimSerialNumber(phoneId);
        }
        MLog.d("getIccId(" + phoneId + "): " + getSubString(result));
        return result;
    }

    @Override
    public int getDataState(int phoneId) {
        int result = DATA_UNKNOWN;
        if (phoneId >= 0 && phoneId < 2) {
            int subId = getSubIdForSlotId(phoneId);
            int defaultSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            if (subId >= 0) {
                if (subId == defaultSubId) {
                    result = getTelephonyManager().getDataState();
                } else {
                    result = DATA_DISCONNECTED;
                }
            }
        }
        MLog.d("getDataState(" + phoneId + "): " + result);
        return result;
    }

    @Override
    public int getSimState(int phoneId) {
        int result = DATA_UNKNOWN;
        if (phoneId >= 0 && phoneId < 2) {
            // getSimState() requires a SIM slot ID
            result = getTelephonyManager().getSimState(phoneId);
        }
        MLog.d("getSimState(" + phoneId + "): " + result);
        return result;
    }

    @Override
    public int getNetworkType(int phoneId) {
        int result = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        if (phoneId >= 0 && phoneId < 2) {
            int subId = getSubIdForSlotId(phoneId);
            if (subId >= 0) {
                // getNetworkType() requires a sub ID
                result = getTelephonyManager().getNetworkType(subId);
                MLog.d("TelephonyManager.getNetworkType(" + phoneId + "): " + result);
                if (result == 0) {
                    result =
                        getTelephonyManager().getVoiceNetworkType(subId);
                        MLog.d("getVoiceNetworkType(" + phoneId + "): " + result);
                }
            }
        }
        MLog.d("getNetworkType(" + phoneId + "): " + result);
        switch(result){
            case TelephonyManager.NETWORK_TYPE_GPRS:
                return NETWORK_TYPE_GPRS;
            case TelephonyManager.NETWORK_TYPE_EDGE:
                return NETWORK_TYPE_EDGE;
            case TelephonyManager.NETWORK_TYPE_UMTS:
                return NETWORK_TYPE_UMTS;
            case TelephonyManager.NETWORK_TYPE_CDMA:
                return NETWORK_TYPE_CDMA;
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
                return NETWORK_TYPE_EVDO_0;
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
                return NETWORK_TYPE_EVDO_A;
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                return NETWORK_TYPE_1xRTT;
            case TelephonyManager.NETWORK_TYPE_HSDPA:
                return NETWORK_TYPE_HSDPA;
            case TelephonyManager.NETWORK_TYPE_HSUPA:
                return NETWORK_TYPE_HSUPA;
            case TelephonyManager.NETWORK_TYPE_HSPA:
                return NETWORK_TYPE_HSPA;
            case TelephonyManager.NETWORK_TYPE_IDEN:
                return NETWORK_TYPE_IDEN;
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
                return NETWORK_TYPE_EVDO_B;
            case TelephonyManager.NETWORK_TYPE_LTE:
                return NETWORK_TYPE_LTE;
            case TelephonyManager.NETWORK_TYPE_EHRPD:
                return NETWORK_TYPE_EHRPD;
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return NETWORK_TYPE_HSPAP;
            case TelephonyManager.NETWORK_TYPE_GSM:
                return NETWORK_TYPE_GSM;
            case TelephonyManager.NETWORK_TYPE_TD_SCDMA:
                return NETWORK_TYPE_TD_SCDMA;
            case TelephonyManager.NETWORK_TYPE_IWLAN:
                return NETWORK_TYPE_IWLAN;
            case TelephonyManager.NETWORK_TYPE_LTE_CA:
                return NETWORK_TYPE_LTE_CA;
            default:
                return NETWORK_TYPE_UNKNOWN;
            }
    }

    @Override
    public String getDeviceSoftwareVersion() {
        String result = CustomProperties.getString(
                            CustomProperties.MODULE_DM,
                            "SoftwareVersion",
                            "V1");
        MLog.d("getDeviceSoftwareVersion(): " + result);
        return result;
    }

    /*
     * This method should be left to vendor to customize as there is no promising method to detect
     * device type automatically.
     *
     * @return DEVICE_TYPE_CELLPHONE as default
     */
    @Override
    public int getDeviceType() {
        int result = DEVICE_TYPE_CELLPHONE;
        MLog.d("getDeviceType(): " + result);
        return result;
    }

    @Override
    public int getMasterPhoneId() {
        final boolean supportMims = supportsMultiIms();
        if (!supportMims) {
            int result = SystemProperties.getInt(
                    "persist.vendor.radio.simswitch", 1) - 1;
            MLog.d("getMasterPhoneId(), simswitch -1 : " + result);
            if (result >= 0 && result < 2) {
                int subId = getSubIdForSlotId(result);
                if (subId >= 0) {
                    MLog.d("getMasterPhoneId(): " + result);
                    return result;
                }
            }
        } else {
            int subId = SubscriptionManager.getDefaultDataSubscriptionId();
            MLog.d("getMasterPhoneId(), getDefaultDataSubscriptionId: " + subId);
            if (subId >= 0) {
                int result = getSlotId(subId);
                if (result >= 0 && result < 2) {
                    MLog.d("getMasterPhoneId(): " + result);
                    return result;
                }
            }
        }

        MLog.d("getMasterPhoneId(): -1 ");
        return -1;
    }

    @Override
    public boolean isInternationalNetworkRoaming(int phoneId) {
        boolean result = false;
        if (phoneId >= 0 && phoneId < 2) {
            int subId = getSubIdForSlotId(phoneId);
            if (subId >= 0) {
                // getNetworkType() requires a sub ID
                result = getTelephonyManager().isNetworkRoaming(subId);
            }
        }
        MLog.d("isInternationalNetworkRoaming(" + phoneId + "): " + result);
        return result;
    }

    @Override
    public int getVoLTEState(int phoneId) {
        int result = VOLTE_STATE_UNKNOWN;
        if (phoneId >= 0 && phoneId < 2) {
            int subId = getSubIdForSlotId(phoneId);
            final boolean hasSim = (subId >= 0);
            final boolean supportMims = supportsMultiIms();
            if (hasSim &&
                    ((!supportMims && phoneId == 0) || (supportMims && hasSim))) {
                ImsManager imsManager = ImsManager.getInstance(mContext, phoneId);
                if (imsManager.isVolteEnabledByPlatform()) {
                    boolean enabled = imsManager.isEnhanced4gLteModeSettingEnabledByUser();
                    result = enabled ? VOLTE_STATE_ON : VOLTE_STATE_OFF;
                } else {
                    MLog.d("getVoLTEState volte is disabled by platform");
                }
            }
        }
        MLog.d("getVoLTEState(" + phoneId + "): " + result);
        return result;
    }

    public void setVoLTEState(int phoneId, int status) {
        MLog.d("setVoLTEState(" + phoneId + ", " + status + ")");
        if (phoneId >= 0 && phoneId < 2) {
            // phoneId specified in parameter list is actually slot ID
            phoneId = SubscriptionManager.getPhoneId(getSubIdForSlotId(phoneId));
            try {
                ImsManager.getInstance(mContext, phoneId).setEnhanced4gLteModeSetting(
                    status == VOLTE_STATE_ON);
            } catch (Exception e) {
                e.printStackTrace();
            }
        } else {
            MLog.e("Invalid phoneId, do nothing");
        }
    }


    /**
     * Get APN content uri for Slot ID. Internally APN content uris are defined
     * by Sub ID, so we have to map Slot ID to Sub ID first.
     * @param phoneId slot ID
     * @return APN uri of the slot specified as phoneId, null if no SIM in slot
     */
    @Override
    public Uri getAPNContentUri(int phoneId) {
        String result = null;
        Uri uri = null;
        Cursor cr = null;
        if (phoneId >= 0 && phoneId < 2) {
            int subId = getSubIdForSlotId(phoneId);
            if (subId >= 0) {
                result = "content://telephony/carriers/preferapn/subId/" + subId;
                uri = Uri.parse(result);
                cr = mContext.getContentResolver().query(uri,null,null,null,null);
                if((cr == null) || (cr.getCount() == 0))
                {
                     if(cr != null){
                         cr.close();
                     }
                     Uri temp = Uri.parse("content://telephony/carriers/subId/" + subId);
                     cr = mContext.getContentResolver().query(temp,null,null,null,null);
                     if((cr != null) && (cr.moveToNext())){
                         String id = cr.getString(cr.getColumnIndex("_id"));
                         result = "content://telephony/carriers/" + id;
                         uri = Uri.parse(result);
                     }
                }
                if(cr != null){
                    cr.close();
                }
            }
        }
        MLog.d("getContentUri(" + phoneId + "): " + result);
        return uri;
    }

    /**
     * @param phoneId the phoneId here is actually subId
     * @return slot ID
     */
    @Override
    public int getSlotId(int phoneId) {
        return SubscriptionManager.getSlotIndex(phoneId);
    }

    @Override
    public int getCellId(int phoneId) {
        int result = -1;
        if (getSubIdForSlotId(phoneId) == -1){
            MLog.d("getCellId(" + phoneId + "): " + result);
            return  result;
        }
        if (phoneId >= 0 && phoneId < 2) {
            MtkTelephonyManagerEx tme = MtkTelephonyManagerEx.getDefault();
            if (tme == null) {
                MLog.w("tme is null");
            } else {
                // TelephonyManagerEx.getCellLocation(int) accepts slot ID
                CellLocation cl = tme.getCellLocation(phoneId);
                if (cl == null) {
                    MLog.e("CellLocation is null");
                } else if (cl instanceof GsmCellLocation) {
                    result = ((GsmCellLocation)cl).getCid();
                } else if (cl instanceof CdmaCellLocation) {
                    result = ((CdmaCellLocation)cl).getSystemId();
                } else {
                    MLog.e("CellLocation type " + cl.getClass() + " is not supported");
                }
            }
        }
        //MLog.d("getCellId(" + phoneId + "): " + result);
        return result;
    }

    @Override
    public int getLac(int phoneId) {
        int result = -1;
        if (getSubIdForSlotId(phoneId) == -1){
            MLog.d("getLac(" + phoneId + "): " + result);
            return  result;
        }
        if (phoneId >= 0 && phoneId < 2) {
            MtkTelephonyManagerEx tme = MtkTelephonyManagerEx.getDefault();
            if (tme == null) {
                MLog.w("TelephonyManagerEx may not be ready");
            } else {
                // TelephonyManagerEx.getCellLocation(int) accepts slot ID
                CellLocation cl = tme.getCellLocation(phoneId);
                if (cl == null) {
                    MLog.e("CellLocation is null");
                } else if (cl instanceof GsmCellLocation) {
                    result = ((GsmCellLocation)cl).getLac();
                } else if (cl instanceof CdmaCellLocation) {
                    result = ((CdmaCellLocation)cl).getNetworkId();
                } else {
                    MLog.e("CellLocation type " + cl.getClass() + " is not supported");
                }
            }
        }
        MLog.d("getLac(" + phoneId + "): " + result);
        return result;
    }

    @Override
    public String getCTAModel(){
        return "invalid CTA Model";
    }

    @Override
    public String getRomStorageSize(){
        return "16G";
    }

    @Override
    public String getRamStorageSize(){
        return "4G";
    }

    @Override
    public String getMacAddress(){
        WifiManager wifi = (WifiManager) getContext().
                            getSystemService(Context.WIFI_SERVICE);
        if(!wifi.isWifiEnabled()){
            return null;
        }
        WifiInfo wifiInfo = wifi.getConnectionInfo();
        String macAddress =
             wifiInfo == null ? null : wifiInfo.getMacAddress();
        return macAddress;
    }

    @Override
    public String getCPUModel(){
        return "invalid CPU Model";
    }

    @Override
    public String getOSVersion(){
        String OPERATING_SYSTEM = "android";
        return OPERATING_SYSTEM + Build.VERSION.RELEASE;
    }

    @Override
    public long getWiFiTotalBytes(long startTime, long endTime){
        long totalBytes = 0;
        if((endTime - startTime) < 0){
            MLog.d("getWiFiTotalBytes startTime=" + startTime + " endTime" + endTime
                + " invalid parameters" );
            return -1;
        }
        NetworkTemplate templete;
        INetworkStatsService statsService = INetworkStatsService.Stub.asInterface(
                ServiceManager.getService(Context.NETWORK_STATS_SERVICE));
        try {
            templete = NetworkTemplate.buildTemplateWifiWildcard();
            INetworkStatsSession statsSession = statsService.openSession();
            if (statsSession != null) {
                 totalBytes = statsSession.getSummaryForNetwork(
                                    templete, startTime, endTime)
                                    .getTotalBytes();
                 TrafficStats.closeQuietly(statsSession);
             }
        }catch (Exception e) {
                 MLog.e("getMobileTotalBytes Exception" + e);
         }
         MLog.d("getWiFiTotalBytes startTime=" + startTime + " endTime" + endTime
                + " totalBytes=" + totalBytes);
         return totalBytes;
    }

    @Override
    public long getMobileTotalBytes(
        int phoneId, long startTime, long endTime) {
        if((phoneId>2) || (phoneId <0) ||(endTime - startTime) < 0){
            MLog.d("getMobileTotalBytes phoneId=" + phoneId
                + " startTime=" + startTime + " endTime" + endTime
                + " invalid parameters");
            return -1;
        }
        String subscriberId = null;
        long totalBytes = 0;
        NetworkTemplate templete;
        INetworkStatsService statsService = INetworkStatsService.Stub.asInterface(
                ServiceManager.getService(Context.NETWORK_STATS_SERVICE));
        final TelephonyManager tele = TelephonyManager.from(mContext);
        if (tele != null) {
            try {
                if (tele.getSimState(phoneId) == SIM_STATE_READY) {
                    int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
                    int[] subIds = SubscriptionManager.getSubId(phoneId);
                    if ((subIds != null) && (subIds.length != 0)) {
                        subId = subIds[0];
                    }
                    if (!SubscriptionManager.isValidSubscriptionId(subId)){
                         //Dual sim fetch fail, always fetch sim1
                         MLog.e("Dual sim phoneId:" + phoneId
                             + " fetch subID fail, always fetch default");
                             subId = SubscriptionManager.getDefaultDataSubscriptionId();
                    }
                    if (SubscriptionManager.isValidSubscriptionId(subId)) {
                        subscriberId = tele.getSubscriberId(subId);
                        INetworkStatsSession statsSession = statsService.openSession();
                        if (statsSession != null) {
                            templete = NetworkTemplate.buildTemplateMobileAll(subscriberId);
                            totalBytes = statsSession.getSummaryForNetwork(
                                    templete, startTime, endTime)
                                        .getTotalBytes();
                           TrafficStats.closeQuietly(statsSession);
                        } else {
                            totalBytes = 0;
                        }
                    } else {
                            MLog.e("getMobileTotalBytes subId not valid");
                     }
                 } else {
                            MLog.e("getMobileTotalBytes SimState != SIM_STATE_READY");
                }
            } catch (Exception e) {
                MLog.e("getMobileTotalBytes Exception" + e);
            }
        } else {
            MLog.e("getMobileTotalBytes TelephonyManager is null");
        }
        MLog.d("getMobileTotalBytes phoneId=" + phoneId + " subscriberId"
                + getSubString(subscriberId)
                + " startTime=" + startTime + " endTime" + endTime
                + " totalBytes=" + totalBytes);
        return totalBytes;
    }

    @Override
    public int getPriorNetworkType() {
        int result = 0;
        if ("OP01".equals(SystemProperties.get("persist.vendor.operator.optr", ""))) {
            result = 1;
        }
        MLog.v("getPriorNetworkType(): " + result);
        return result;
    }

    @Override
    public String getPhoneNumber(int phoneId) {
        String number = "unknown";
        int subId = 0;
        if (phoneId < 2 || phoneId >= 0) {
            subId = getSubIdForSlotId(phoneId);
            if (subId > 0) {
                number = getTelephonyManager().getLine1Number(subId);
            }
        }
        if (TextUtils.isEmpty(number)) {
            number = "unknown";
        }
        MLog.d("getPhoneNumber phoneId: " + phoneId + ", subId: " + subId
                + ", Number: " + getSubString(number));
        return number;
    }

    private class SwitchState {
        int getWifiState() {
            WifiManager wifi = (WifiManager) mContext
                    .getSystemService(Context.WIFI_SERVICE);
            int wifiState = wifi.getWifiState();
            MLog.d("getWifiState:" + wifiState);
            if (wifiState == WifiManager.WIFI_STATE_ENABLED
                    || wifiState == WifiManager.WIFI_STATE_ENABLING) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }

        }

        int getDataEnable() {
            int defaultSubId = SubscriptionManager
                    .getDefaultDataSubscriptionId();
            TelephonyManager telephonyManager = (TelephonyManager) mContext
                    .getSystemService(Context.TELEPHONY_SERVICE);
            if (telephonyManager.getDataEnabled(defaultSubId)) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }

        int getBluetoothSwitchState() {
            BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
            if (adapter == null) {
                return SWITCH_STATE_UNKNOWN;
            }
            int btState = adapter.getState();
            MLog.d("getBluetoothSwitchState: " + btState);
            if (btState == BluetoothAdapter.STATE_ON
                    || btState == BluetoothAdapter.STATE_TURNING_ON) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }

        int getGPSSwitchState() {
            int LocationMode = Settings.Secure.getInt(
                    mContext.getContentResolver(),
                    Settings.Secure.LOCATION_MODE,
                    Settings.Secure.LOCATION_MODE_OFF);
            MLog.d("getGPSSwitchState, Location Mode: " + LocationMode);
            if (LocationMode == Settings.Secure.LOCATION_MODE_SENSORS_ONLY
                    || LocationMode == Settings.Secure.LOCATION_MODE_HIGH_ACCURACY) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }

        int getCallVibrateSwitchState() {
            int vbState = Settings.System.getInt(mContext.getContentResolver(),
                    Settings.System.VIBRATE_WHEN_RINGING, 0);
            MLog.d("getCallVibrateSwitchState: " + vbState);
            if (vbState == AudioManager.RINGER_MODE_VIBRATE
                    || vbState == AudioManager.RINGER_MODE_NORMAL) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }

        int getSilentSwitchState() {
            int silentState = AudioManager.RINGER_MODE_SILENT;
            AudioManager audioManager = (AudioManager) mContext
                    .getSystemService(Context.AUDIO_SERVICE);
            silentState = audioManager.getRingerModeInternal();
            MLog.d("getSilentSwitchState: " + silentState);
            if (silentState == AudioManager.RINGER_MODE_SILENT) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }

        }

        int getWifiApSwitchState() {
            WifiManager wifiManager = (WifiManager) mContext
                    .getSystemService(Context.WIFI_SERVICE);
            int wifiApState = wifiManager.getWifiApState();
            MLog.d("getWifiApSwitchState: " + wifiApState);
            if (wifiApState == WifiManager.WIFI_AP_STATE_ENABLED
                    || wifiApState == WifiManager.WIFI_AP_STATE_ENABLING) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }

        int getAirplaneSwitchState() {
            if (Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.AIRPLANE_MODE_ON, 0) != 0) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }

        private final CameraManager mCameraManager = (CameraManager) mContext
                .getSystemService(Context.CAMERA_SERVICE);

        private Handler mHandler = null;
        private boolean mFlashlightEnabled = false;;

        private synchronized void ensureHandler() {
            if (mHandler == null) {
                HandlerThread thread = new HandlerThread("ctm",
                        Process.THREAD_PRIORITY_BACKGROUND);
                thread.start();
                mHandler = new Handler(thread.getLooper());
            }
        }

        private final CameraManager.TorchCallback mTorchCallback = new CameraManager.TorchCallback() {
            @Override
            public void onTorchModeChanged(String cameraId, boolean enabled) {
                MLog.e("onTorchModeChanged,cameraId: " + cameraId + "enabled: "
                        + enabled);
                synchronized (mWaitLock) {
                    if (enabled) {
                        mFlashlightEnabled = enabled;
                    }
                    mCameraManager.unregisterTorchCallback(mTorchCallback);
                    mWaitLock.notify();
                    MLog.d("onTorchModeChangede, exit");
                }
            }
        };

        private void registerFlashCallback() {
            MLog.d("registerFlashCallback");
            try {
                ensureHandler();
                mCameraManager.registerTorchCallback(mTorchCallback, mHandler);
                mWaitLock.wait(100);
            } catch (Throwable e) {
                MLog.e("Couldn't Register. exception: " + e);
                return;
            }
        }

        synchronized int getFlashSwitchState() {
            synchronized (mWaitLock) {
                mFlashlightEnabled = false;
                registerFlashCallback();
                MLog.d("getFlashSwitchState: " + mFlashlightEnabled);
            }
            if (mFlashlightEnabled) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }

        int getScreenOnSwitchState() {
            PowerManager pm = (PowerManager) mContext
                    .getSystemService(Context.POWER_SERVICE);
            if (pm.isScreenOn()) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }

        int getScreenRotateSwitchState() {
            if (RotationPolicy.isRotationLocked(mContext)) {
                return SWITCH_STATE_OFF;
            } else {
                return SWITCH_STATE_ON;
            }
        }

        /*
         * 4G switch is not exist. always can register 4G network.
         * Return SWITCH_STATE_OFF as the swtich state always on.
         */
        int getEnhanced4GSwitchState() {
            return SWITCH_STATE_ON;
        }

        int getScreenBrightnessAutoSwitchState() {
            int brightnessMode = Settings.System.getInt(
                    mContext.getContentResolver(),
                    Settings.System.SCREEN_BRIGHTNESS_MODE, 0);
            MLog.d("getScreenBrightnessAutoSwitchState: " + brightnessMode);
            if (brightnessMode == Settings.System.SCREEN_BRIGHTNESS_MODE_AUTOMATIC) {
                return SWITCH_STATE_ON;
            } else {
                return SWITCH_STATE_OFF;
            }
        }
    }


    @Override
    public int getSwitchState(int switchId) {
        int result = SWITCH_STATE_UNKNOWN;
        switch (switchId) {
        case SWITCH_WIFI:
            result = mSwitchState.getWifiState();
            break;
        case SWITCH_GPRS:
            result = mSwitchState.getDataEnable();
            break;
        case SWITCH_BLUETOOTH:
            result = mSwitchState.getBluetoothSwitchState();
            break;
        case SWITCH_GPS:
            result = mSwitchState.getGPSSwitchState();
            break;
        case SWITCH_SHOCK:
            result = mSwitchState.getCallVibrateSwitchState();
            break;
        case SWITCH_SILENT:
            result = mSwitchState.getSilentSwitchState();
            break;
        case SWITCH_HOT_SPOT:
            result = mSwitchState.getWifiApSwitchState();
            break;
        case SWITCH_FLYING:
            result = mSwitchState.getAirplaneSwitchState();
            break;
        case SWITCH_FLASHLIGHT:
            result = mSwitchState.getFlashSwitchState();
            break;
        case SWITCH_SCREEN:
            result = mSwitchState.getScreenOnSwitchState();
            break;
        case SWITCH_SCREEN_ROTATE:
            result = mSwitchState.getScreenRotateSwitchState();
            break;
        case SWITCH_LTE:
            result = mSwitchState.getEnhanced4GSwitchState();
            break;
        case SWITCH_AUTO_BRIGHT:
            result = mSwitchState.getScreenBrightnessAutoSwitchState();
            break;
        default:
            result = SWITCH_STATE_UNKNOWN;
        }
        MLog.d("getSwitchState(" + switchId + "): " + result);
        return result;
    }

  private LocationListener mLocationListener = new LocationListener() {
        @Override
        public void onLocationChanged(Location location) {
            //MLog.d("onLocationChanged: " + location);
            synchronized (mLocation) {
                mLocation.longitude = location.getLongitude();
                mLocation.latitude = location.getLatitude();
                mLocation.addrFrom = 1;
                MLog.d("getLocationMsg" + "addrFrom: "
                        + mLocation.addrFrom);
            }
        }
        @Override
        public void onProviderDisabled(String provider) {
        }

        @Override
        public void onProviderEnabled(String provider) {
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
        }
    };

    private void requestLocation() {
        new Thread(new Runnable() {
            public void run() {
                Looper.prepare();
                try {
                    mLocationManager.requestLocationUpdates(
                            LocationManager.NETWORK_PROVIDER,
                            NETWORK_LOCATION_UPDATE_TIME, 0, mLocationListener);
                } catch (Exception e) {
                    e.printStackTrace();
                }
                MLog.d("loop");
                Looper.loop();
            }
        }).start();
    }

     @Override
    public LocationMsg getLocationMsg() {
        synchronized (mWaitLock) {
            if ((mLocationManager == null)
                || (!mLocationManager
                        .isProviderEnabled(LocationManager.NETWORK_PROVIDER))) {
                synchronized (mLocation) {
                    MLog.d("default mLocation" + "addrFrom: "
                        + mLocation.addrFrom);
                    return mLocation;
                }
            }
            try {
                requestLocation();
                Thread.sleep(2000);
            } catch (InterruptedException e) {
                MLog.d("InterruptedException" + e);
            }
            mLocationManager.removeUpdates(mLocationListener);
            synchronized (mLocation) {
                MLog.d("getLocationMsg" + "addrFrom: "
                    + mLocation.addrFrom);
                return mLocation;
            }
        }
    }

    @Override
    public String getModel() {

        final String msvSuffix = getMsvSuffix();
        return Build.MODEL + msvSuffix;
    }

    /**
     * Returns " (ENGINEERING)" if the msv file has a zero value, else returns "".
     *
     * @return a string to append to the model number description.
     */
    private String getMsvSuffix() {
        // Production devices should have a non-zero value. If we can't read it, assume it's a
        // production device so that we don't accidentally show that it's an ENGINEERING device.
        try {
            String msv = readLine(FILENAME_MSV);
            if (msv == null) {
                return "";
            }

            // Parse as a hex number. If it evaluates to a zero, then it's an engineering build.
            if (Long.parseLong(msv, 16) == 0) {
                return " (ENGINEERING)";
            }
        } catch (IOException ioe) {
            // Fail quietly, as the file may not exist on some devices.
        } catch (NumberFormatException nfe) {
            // Fail quietly, returning empty string should be sufficient
        }
        return "";
    }

    /**
     * Reads a line from the specified file.
     *
     * @param filename the file to read from.
     * @return the first line, if any.
     * @throws IOException if the file couldn't be read.
     */
    private String readLine(String filename) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(filename), 256);
        try {
            return reader.readLine();
        } finally {
            reader.close();
        }
    }
    public void setContext(Context context) {
        mContext = context;
    }

    public Context getContext() {
        return mContext;
    }

    public boolean supportsMultiIms() {
        if (SystemProperties.getInt(PROPERTY_MULTIPLE_IMS_SUPPORT, 1) == 1) {
            return false;
        } else {
            return true;
        }
    }

    private int getSubIdForSlotId(int slotId) {
        if (!getTelephonyManager().hasIccCard(slotId)) {
            MLog.d("getSubIdForSlotId(" + slotId + "): -1 (no sim found)");
            return -1;
        }
        int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds == null || subIds.length < 1 || subIds[0] < 0) {
            MLog.e("getSubIdForSlotId(" + slotId + "): -1 (error)");
            return -1;
        }
        MLog.d("getSubIdForSlotId(" + slotId + "): " + subIds[0]);
        return subIds[0];
    }

    private String getSubString(String inputLogString) {
        if(inputLogString == null || inputLogString.length() <= 4) {
            return inputLogString;
        } else {
            return inputLogString.substring(inputLogString.length()-4, inputLogString.length());
        }
    }
}
