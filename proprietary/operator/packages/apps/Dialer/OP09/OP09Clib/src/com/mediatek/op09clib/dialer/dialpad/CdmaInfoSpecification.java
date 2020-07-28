package com.mediatek.op09clib.dialer.dialpad;

import android.app.ActivityManager;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;

import android.os.Environment;
import android.os.StatFs;
import android.os.SystemProperties;

import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.provider.Settings;
import android.telephony.cdma.CdmaCellLocation;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.CellLocation;
import android.telephony.CellInfo;
import android.telephony.CellInfoCdma;
import android.telephony.CellInfoLte;
import android.telephony.CellInfoGsm;
import android.telephony.CellInfoWcdma;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.custom.CustomProperties;
import com.mediatek.op09clib.dialer.R;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.lang.IllegalArgumentException;
import java.util.concurrent.Executors;
import java.util.HashMap;
import java.util.List;
import java.util.Set;

import android.content.pm.PackageManager;
import static android.Manifest.permission.ACCESS_FINE_LOCATION;

/**
 * implement display for version info plugin for op09.
 */
public class CdmaInfoSpecification extends PreferenceActivity {

    private static final String TAG = "CdmaInfoSpecification";
    private static final String PACKAGE_NAME = "com.mediatek.sysinfo";
    private static final String FILENAME_MSV = "/sys/board_properties/soc/msv";

    private static final String KEY_PRODUCT_MODEL = "product_model";
    private static final String KEY_HARDWARE_VERSION = "hardware_version";
    private static final String KEY_SOFTWARE_VERSION = "software_version";

    public static final String KEY_CDMA_INFO = "cdma_info";

    public static final String KEY_MEID = "meid";
    public static final String KEY_IMEI_1 = "imei1";
    public static final String KEY_IMEI_2 = "imei2";

    public static final String KEY_ICCID = "iccid";
    public static final String KEY_ICCID_1 = "iccid1";
    public static final String KEY_ICCID_2 = "iccid2";

    public static final String KEY_CDMA_IMSI = "cdma_imsi";
    public static final String KEY_CDMA_IMSI_1 = "cdma_imsi1";
    public static final String KEY_CDMA_IMSI_2 = "cdma_imsi2";

    public static final String KEY_LTE_IMSI = "imsi_lte";
    public static final String KEY_LTE_IMSI_1 = "imsi_lte1";
    public static final String KEY_LTE_IMSI_2 = "imsi_lte2";

    public static final String KEY_NETWORK_1 = "network1";
    public static final String KEY_NETWORK_2 = "network2";

    public static final String KEY_ANDROID_VERSION = "android";
    public static final String KEY_STORAGE = "storage";

    public static final String KEY_OPERATOR_1 = "operator1";
    public static final String KEY_OPERATOR_2 = "operator2";

    public static final String KEY_CELLID = "cellId";
    public static final String KEY_CELLID_1 = "cellId1";
    public static final String KEY_CELLID_2 = "cellId2";

    public static final String KEY_RAM_SIZE = "ram";

    public static final String KEY_SUB_ID = "subid";

    public static final String SOFTWARE_VERSION_DEFAULT = Build.MODEL + ".P1";
    public static final String HARDWARE_DEFAULT = "V1";

    private static final int SLOT1 = 0;
    private static final int SLOT2 = 1;

    private TelephonyManager mTelephonyManager;
    private boolean mFlightMode;
    private boolean mSinglePhone = false;

    private static final int     LOG_LENGTH = 5;
    private static final boolean DEBUG = ("eng".equals(Build.TYPE) ||
                                        "userdebug".equals(Build.TYPE));

    private static final boolean FORCE_PRINT = false;

    private HashMap<String, String> mInfoMap = new HashMap<String, String>();

    private static final int LOCATION_PERMISSION_REQUEST_CODE = 1;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        log("onCreate()");
        //Init all information
        mTelephonyManager = TelephonyManager.from(this);

        initDeviceInfo();
        addPreferencesFromResource(R.xml.cdma_info_specifications);
    }


///M: Add for ALPS04830757 @{
  @Override
  public void onRequestPermissionsResult(
      int requestCode, String[] permissions, int[] grantResults) {
      Log.i(TAG," onRequestPermissionsResult enter ");
      if (requestCode == LOCATION_PERMISSION_REQUEST_CODE) {
          if (grantResults.length >= 1 && PackageManager.PERMISSION_GRANTED == grantResults[0]) {
              Log.i(TAG," onRequestPermissionsResult return  length = " + grantResults.length );
              return;
          } else {
              Log.i(TAG,"finish()");
              finish();
         }
     }
  }
///@}

    @Override
    protected void onResume() {
        super.onResume();

        Log.i(TAG," onResume, checkSelfPermission");
        if (this.checkSelfPermission(ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            Log.i(TAG," Call requestPermissions() to get location permission" );
            requestPermissions(new String[]{ACCESS_FINE_LOCATION}, LOCATION_PERMISSION_REQUEST_CODE);
            return;
        }

        mFlightMode = Settings.Global.getInt(this.getContentResolver(),
                                    Settings.Global.AIRPLANE_MODE_ON, 0) > 0;
        log("onResume(), mFlightMode = " + mFlightMode);
        initSimInfo();
        setDeviceValuesToPreferences();
        setInfoToPreference();
    }
    /**
     * set the info from phone to preference.
     *
     * @return void.
     */
    private void setDeviceValuesToPreferences() {
        log("setPhoneValuesToPreferences()");
        PreferenceScreen parent = (PreferenceScreen) getPreferenceScreen();
        Preference preference = parent.findPreference(KEY_PRODUCT_MODEL);
        if (null != preference) {
            preference.setSummary(mInfoMap.get(KEY_PRODUCT_MODEL));
        }

        preference = parent.findPreference(KEY_HARDWARE_VERSION);
        if (null != preference) {
            preference.setSummary(mInfoMap.get(KEY_HARDWARE_VERSION));
        }
        preference = parent.findPreference(KEY_SOFTWARE_VERSION);
        if (null != preference) {
            preference.setSummary(mInfoMap.get(KEY_SOFTWARE_VERSION));
        }

        preference = parent.findPreference(KEY_ANDROID_VERSION);
        if (null != preference) {
            preference.setSummary(mInfoMap.get(KEY_ANDROID_VERSION));
        }

        preference = parent.findPreference(KEY_STORAGE);
        if (null != preference) {
            preference.setSummary(mInfoMap.get(KEY_STORAGE));
        }

        preference = parent.findPreference(KEY_RAM_SIZE);
        if (null != preference) {
            preference.setSummary(mInfoMap.get(KEY_RAM_SIZE));
        }
    }

    private void updatePreferenceByKey(String key, String result) {
        log("updatePreferenceByKey");
        if (mInfoMap == null) {
            return;
        }

        mInfoMap.put(key, result);
        PreferenceScreen parent = (PreferenceScreen) getPreferenceScreen();
        if (parent == null) {
            return;
        }

        Preference preference = findPreference(key);
        if (null != preference) {
            log("updatePreferenceByKey, setSummary");
            preference.setSummary(mInfoMap.get(key));
        }
    }

    /**
     * set the info from cdma phone to preference.
     *
     * @param slot indicator which slot is cdma phone or invalid.
     * @return void.
     */
    private void setInfoToPreference() {
        PreferenceScreen parent = (PreferenceScreen) getPreferenceScreen();

        Preference preference = findPreference(KEY_MEID);
        if (null != preference) {
            preference.setSummary(mInfoMap.get(KEY_MEID));
        }

        preference = findPreference(KEY_IMEI_1);
        if (null != preference && null != mInfoMap.get(KEY_IMEI_1)) {
            preference.setSummary(mInfoMap.get(KEY_IMEI_1));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_IMEI_2);
        if (!mSinglePhone && null != preference && null != mInfoMap.get(KEY_IMEI_2)) {
            preference.setSummary(mInfoMap.get(KEY_IMEI_2));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_ICCID_1);
        if (null != preference && null != mInfoMap.get(KEY_ICCID_1)) {
            preference.setSummary(mInfoMap.get(KEY_ICCID_1));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_ICCID_2);
        if (null != preference && null != mInfoMap.get(KEY_ICCID_2)) {
            preference.setSummary(mInfoMap.get(KEY_ICCID_2));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_LTE_IMSI_1);
        if (null != preference && null != mInfoMap.get(KEY_LTE_IMSI_1)) {
            preference.setSummary(mInfoMap.get(KEY_LTE_IMSI_1));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_LTE_IMSI_2);
        if (null != preference && null != mInfoMap.get(KEY_LTE_IMSI_2)) {
            preference.setSummary(mInfoMap.get(KEY_LTE_IMSI_2));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_CDMA_IMSI_1);
        if (null != preference && null != mInfoMap.get(KEY_CDMA_IMSI_1)) {
            preference.setSummary(mInfoMap.get(KEY_CDMA_IMSI_1));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_CDMA_IMSI_2);
        if (null != preference && null != mInfoMap.get(KEY_CDMA_IMSI_2)) {
            preference.setSummary(mInfoMap.get(KEY_CDMA_IMSI_2));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_NETWORK_1);
        if (null != preference && null != mInfoMap.get(KEY_NETWORK_1)) {
            preference.setSummary(mInfoMap.get(KEY_NETWORK_1));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_NETWORK_2);
        if (null != preference && null != mInfoMap.get(KEY_NETWORK_2)) {
            preference.setSummary(mInfoMap.get(KEY_NETWORK_2));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_OPERATOR_1);
        if (null != preference && null != mInfoMap.get(KEY_OPERATOR_1)) {
            preference.setSummary(mInfoMap.get(KEY_OPERATOR_1));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_OPERATOR_2);
        if (null != preference && null != mInfoMap.get(KEY_OPERATOR_2)) {
            preference.setSummary(mInfoMap.get(KEY_OPERATOR_2));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_CELLID_1);
        if (null != preference && null != mInfoMap.get(KEY_CELLID_1)) {
            preference.setSummary(mInfoMap.get(KEY_CELLID_1));
        } else if (null != preference) {
            parent.removePreference(preference);
        }

        preference = findPreference(KEY_CELLID_2);
        if (null != preference && null != mInfoMap.get(KEY_CELLID_2)) {
            preference.setSummary(mInfoMap.get(KEY_CELLID_2));
        } else if (null != preference) {
            parent.removePreference(preference);
        }
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

    /**
     * simple log info.
     *
     * @param msg need print out string.
     * @return void.
     */
    private static void log(String msg) {
        Log.d(TAG, msg);
    }

    private void fillSimInfo() {
        log("fillSimInfo");
        fillIccid();
        fillImsi();
        fillCdmaImsi();
        //fillNetworkInfo();
        //fillIMEIInfo();
        fillMeid();
        fillCellInfo();
    }

    private void initDeviceInfo() {
        log("initDeviceInfo");
        mInfoMap.clear();
        fillDeviceInfo();
    }

    private void initSimInfo() {
        log("initSimInfo");
        fillSimInfo();
        adjustInfoByRule();
        if (DEBUG || FORCE_PRINT) {
            dumpInfoMap();
        }
    }

    private void adjustInfoByRule() {
        //Decide which need to be display/undisplay.
    }


    private void fillDeviceInfo() {
        mInfoMap.put(KEY_PRODUCT_MODEL, Build.MODEL + getMsvSuffix());

        String hardWare = CustomProperties.getString(CustomProperties.MODULE_DM,
                "HardwareVersion", HARDWARE_DEFAULT);
        mInfoMap.put(KEY_HARDWARE_VERSION, hardWare);

        String softWare = CustomProperties.getString(CustomProperties.MODULE_DM,
                "SoftwareVersion", SOFTWARE_VERSION_DEFAULT);

        mInfoMap.put(KEY_SOFTWARE_VERSION, softWare);

        String version = android.os.Build.VERSION.RELEASE;
        String swVersion = "Android" + version;
        mInfoMap.put(KEY_ANDROID_VERSION, swVersion);

        long storageSize = getCustomizeStorageSize();
        String size = "" + storageSize + "G";
        mInfoMap.put(KEY_STORAGE, size);

        long ram = getRamSize();
        String ramSize = "" + ram + "G";
        mInfoMap.put(KEY_RAM_SIZE, ramSize);
    }

    private String getMeid() {
        String meid = "";
        int count = TelephonyManager.getDefault().getPhoneCount();
        for (int i = 0; i < count; i++) {
            if (TextUtils.isEmpty(meid)) {
                meid = TelephonyManager.getDefault().getMeid(i);
                if (DEBUG || FORCE_PRINT) {
                    log("getMeid, meid = " + formatLogValue(meid));
                }
            }
        }
        return meid;
    }

    private void fillIMEIInfo() {
        Phone[] phones = PhoneFactory.getPhones();
        if (phones != null && phones.length >= 2) {
            String imei1 = "" + TelephonyManager.getDefault().getImei(SLOT1);
            String imei2 = "" + TelephonyManager.getDefault().getImei(SLOT2);
            if (DEBUG || FORCE_PRINT) {
                log("fillIMEIInfo, imei1 = " + formatLogValue(imei1) +
                                    ", imei2 = " + formatLogValue(imei2));
            }
            mInfoMap.put(KEY_IMEI_1, imei1);
            mInfoMap.put(KEY_IMEI_2, imei2);
            mInfoMap.put(KEY_OPERATOR_1, getNetworkId(phones[0]));
            mInfoMap.put(KEY_OPERATOR_2, getNetworkId(phones[1]));
        } else if (phones != null && phones.length == 1) {
            String imei1 = "" + TelephonyManager.getDefault().getImei(SLOT1);
            mInfoMap.put(KEY_IMEI_1, imei1);
            mInfoMap.put(KEY_IMEI_2, null);

            mInfoMap.put(KEY_OPERATOR_1, getNetworkId(phones[0]));
            mInfoMap.put(KEY_OPERATOR_2, null);
            mSinglePhone = true;
        } else {
            mInfoMap.put(KEY_IMEI_1, null);
            mInfoMap.put(KEY_IMEI_2, null);

            mInfoMap.put(KEY_OPERATOR_1, null);
            mInfoMap.put(KEY_OPERATOR_2, null);
        }
    }

    private void fillMeid() {
        mInfoMap.put(KEY_MEID, getMeid());
    }

    private long getSystemStorageSize() {
        File path = null;
        try {
            path = Environment.getRootDirectory();
            if (DEBUG || FORCE_PRINT) {
                log("System storage path is " + formatLogValue(path.toString()));
            }
            StatFs statFs = new StatFs(path.getPath());
            long blockSize = statFs.getBlockSize();
            long blockCount = statFs.getBlockCount();
            log("System storage blockSize is " + blockSize + ", blockCount is " + blockCount);
            long size = blockSize * blockCount;
            return size;
        } catch (IllegalArgumentException e) {
            return 0;
        }
    }

    private long getDataStorageSize() {
        File path = null;
        try {
            path = Environment.getExternalStorageDirectory();
            if (DEBUG || FORCE_PRINT) {
                log("Data storage path is " + formatLogValue(path.toString()));
            }
            StatFs statFs = new StatFs(path.getPath());
            long blockSize = statFs.getBlockSize();
            long blockCount = statFs.getBlockCount();
            log("Data storage blockSize is " + blockSize + ", blockCount is " + blockCount);
            long size = blockSize * blockCount;
            return size;
        } catch (IllegalArgumentException e) {
            return 0;
        }
    }

    private int getCustomizeStorageSize() {
        int count = 0;
        long totalSize = getSystemStorageSize() + getDataStorageSize();
        long size = (totalSize / (1024*1024*1024));
        log("getCustomizeStorageSize, size = " + size);
        while(size >= 1) {
            size = size/2;
            count++;
        }
        return 1<<count;
    }

    private int getRamSize() {
        ActivityManager activityManager = (ActivityManager) this.getSystemService(
                Context.ACTIVITY_SERVICE);
        ActivityManager.MemoryInfo memoryInfo = new ActivityManager.MemoryInfo();
        activityManager.getMemoryInfo(memoryInfo);
        double size = 1.0 * memoryInfo.totalMem / 1024 / 1024 / 1024;
        int ram = (int) Math.ceil(size);
        return ram;
    }

    private void fillIccid() {
/*
        String iccid1 = SystemProperties.get("vendor.ril.iccid.sim1");
        String iccid2 = SystemProperties.get("vendor.ril.iccid.sim2");

        if (iccid1 == null || iccid1.isEmpty() || "N/A".equals(iccid1)) {
            mInfoMap.put(KEY_ICCID_1, null);
        } else {
            mInfoMap.put(KEY_ICCID_1, iccid1);
        }

        if (iccid2 == null || iccid2.isEmpty() || "N/A".equals(iccid2)) {
            mInfoMap.put(KEY_ICCID_2, null);
        } else {
            mInfoMap.put(KEY_ICCID_2, iccid2);
        }
*/

        int phoneCount = TelephonyManager.from(this).getPhoneCount();

        for (int i = 0; i < phoneCount; i++) {
            int id = i + 1;
            int[] subIdList = SubscriptionManager.getSubId(i);
            if (subIdList == null) {
                continue;
            }
            String iccid = TelephonyManager.from(this).getSimSerialNumber(subIdList[0]);
            if (iccid == null || iccid.isEmpty() || "N/A".equals(iccid)) {
                mInfoMap.put(KEY_ICCID + id, null);
            } else {
                mInfoMap.put(KEY_ICCID + id, iccid);
            }
        }
    }

    private void fillImsi() {
        int phoneCount = TelephonyManager.from(this).getPhoneCount();

        for (int i = 0; i < phoneCount; i++) {
            int id = i + 1;
            int[] subIdList = SubscriptionManager.getSubId(i);
            if (subIdList == null) {
                continue;
            }

            String imsi = TelephonyManager.from(this).getSubscriberId(subIdList[0]);
            if (imsi == null || imsi.isEmpty() || "N/A".equals(imsi)) {
                mInfoMap.put(KEY_LTE_IMSI + id, null);
            } else {
                mInfoMap.put(KEY_LTE_IMSI + id, imsi);
            }
        }
    }

    private void fillCdmaImsi() {
        int phoneCount = TelephonyManager.from(this).getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            int id = i + 1;
            int[] subIdList = SubscriptionManager.getSubId(i);
            if (subIdList == null) {
                continue;
            }

            String cdmaImsi = MtkTelephonyManagerEx.getDefault().getUimSubscriberId(subIdList[0]);
            if (cdmaImsi == null || cdmaImsi.isEmpty() || "N/A".equals(cdmaImsi)) {
                mInfoMap.put(KEY_CDMA_IMSI + id, null);
            } else {
                mInfoMap.put(KEY_CDMA_IMSI + id, cdmaImsi);
            }
        }
    }

    private int getNetworkType(int subId) {
        int networkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        if (mFlightMode) {
            return networkType;
        }

        final int dataNetworkType = mTelephonyManager.getDataNetworkType(subId);
        final int voiceNetworkType = mTelephonyManager.getVoiceNetworkType(subId);
        log("updateNetworkType(), dataNetworkType = " + dataNetworkType
                + ", voiceNetworkType = " + voiceNetworkType);
        if (TelephonyManager.NETWORK_TYPE_UNKNOWN != dataNetworkType) {
            networkType = dataNetworkType;
        } else if (TelephonyManager.NETWORK_TYPE_UNKNOWN != voiceNetworkType) {
            networkType = voiceNetworkType;
        }
        return networkType;
    }

    private String parseNetwokType(int network) {
        log("parseNetwokType network = " + network);
        String networkType = null;
        switch (network) {
            case TelephonyManager.NETWORK_TYPE_GPRS:
                networkType = "GPRS";
                break;
            case TelephonyManager.NETWORK_TYPE_EDGE:
                networkType = "EDGE";
                break;
            case TelephonyManager.NETWORK_TYPE_UMTS:
                networkType = "UMTS";
                break;
            case TelephonyManager.NETWORK_TYPE_CDMA:
                networkType = "IS95";
                break;
            case TelephonyManager.NETWORK_TYPE_EVDO_0:
                networkType = "EVDO_0";
                break;
            case TelephonyManager.NETWORK_TYPE_EVDO_A:
                networkType = "EVDO_A";
                break;
            case TelephonyManager.NETWORK_TYPE_1xRTT:
                networkType = "1xRTT";
                break;
            case TelephonyManager.NETWORK_TYPE_HSDPA:
                networkType = "HSDPA";
                break;
            case TelephonyManager.NETWORK_TYPE_HSUPA:
                networkType = "HSUPA";
                break;
            case TelephonyManager.NETWORK_TYPE_HSPA:
                networkType = "HSPA";
                break;
            case TelephonyManager.NETWORK_TYPE_IDEN:
                networkType = "IDEN";
                break;
            case TelephonyManager.NETWORK_TYPE_EVDO_B:
                networkType = "EVDO_B";
                break;
            case TelephonyManager.NETWORK_TYPE_LTE:
                networkType = "LTE";
                break;
            case TelephonyManager.NETWORK_TYPE_EHRPD:
                networkType = "eHRPD";
                break;
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                networkType = "HSPA+";
                break;
            case TelephonyManager.NETWORK_TYPE_GSM:
                networkType = "GSM";
                break;
            case TelephonyManager.NETWORK_TYPE_UNKNOWN:
                networkType = "Unknown";
                break;
            default:
                break;
        }
        return networkType;
    }

    private String getNetworkId(Phone phone) {
        //Because of C2K will return in service state even no uim inserted,
        //so check the card state first.
        String nid = null;
        String mnc = null;
        if (mFlightMode || phone == null) {
            return null;
        }

        ServiceState serviceState = phone.getServiceState();
        if (serviceState != null) {
            if ((serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE) ||
                (serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE)) {
                nid = serviceState.getOperatorNumeric();
                if (nid != null && nid.length() > 3) {
                    mnc = nid.substring(0, 3) + "、" + nid.substring(3);
                }
            }

            if ((serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE) &&
                (serviceState.getDataNetworkType() == TelephonyManager.NETWORK_TYPE_LTE)) {
                nid = serviceState.getDataOperatorNumeric();
                if (nid != null && nid.length() > 3) {
                    mnc = nid.substring(0, 3) + "、" + nid.substring(3);
                }
            }
        }
        if (DEBUG || FORCE_PRINT) {
            log("getNetworkId(), mnc = " + formatLogValue(mnc));
        }
        return mnc;
    }

    private void fillNetworkInfo() {
        int sub1 = getSubIdBySlotId(SLOT1);
        log("getSubId, sub1 = " + sub1);
        if (sub1 > SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            int network = getNetworkType(sub1);
            String networkType = parseNetwokType(network);
            mInfoMap.put(KEY_NETWORK_1, networkType);
        } else {
            mInfoMap.put(KEY_NETWORK_1, null);
        }

        int sub2 = getSubIdBySlotId(SLOT2);
        log("getSubId, sub2 = " + sub2);
        if (sub2 > SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            int network = getNetworkType(sub2);
            String networkType = parseNetwokType(network);
            mInfoMap.put(KEY_NETWORK_2, networkType);
        } else {
            mInfoMap.put(KEY_NETWORK_2, null);
        }
    }

    private void fillCellInfo() {
        mInfoMap.put(KEY_CELLID_1, "");
        mInfoMap.put(KEY_CELLID_2, "");
        int phoneCount = TelephonyManager.from(this).getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            if (!TelephonyManager.from(this).hasIccCard(i)) {
                continue;
            }
            new CellInfoAsyncTask(i).executeOnExecutor(Executors.newCachedThreadPool());
        }
    }

    private int getSubIdBySlotId(int slot) {
        int[] subIds = SubscriptionManager.getSubId(slot);
        if (subIds == null) {
            return SubscriptionManager.INVALID_SIM_SLOT_INDEX;
        }

        return subIds[0];
    }

    private int getMainSlotId() {
        int mainSlotId = SubscriptionManager.INVALID_SIM_SLOT_INDEX;
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        mainSlotId = SubscriptionManager.getSlotIndex(subId);
        if (mainSlotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            log("getMainSlotId(), default data sim = " + mainSlotId);
            return mainSlotId;
        }

        String currMainSim = SystemProperties.get("persist.vendor.radio.simswitch", "");
        log("getMainSlotId(), currMainSim 3/4G Sim = " + currMainSim);
        if (!TextUtils.isEmpty(currMainSim)) {
            mainSlotId = Integer.parseInt(currMainSim) - 1;
            if (mainSlotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX &&
                TelephonyManager.from(this).hasIccCard(mainSlotId)) {
                return mainSlotId;
            }
        }

        int phoneCount = TelephonyManager.from(this).getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            if(TelephonyManager.from(this).hasIccCard(i)) {
                mainSlotId = i;
                log("getMainSlotId(), first slot = " + mainSlotId);
                break;
            }
        }

        return mainSlotId;
    }

    private GsmCdmaPhone getPhoneById(int index) {
        log( "gsmPhoneId(), index = " + index);
        int mainPhoneId = index;
        GsmCdmaPhone gsmPhone = null;
        Phone phone = null;
        Phone[] phones = PhoneFactory.getPhones();
        for (Phone p : phones) {
            if (p.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM ||
                p.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS ||
                p.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA) {
                if (p instanceof GsmCdmaPhone) {
                    gsmPhone = (GsmCdmaPhone) p;
                }

                if (gsmPhone == null) {
                    continue;
                }

                log( "gsmPhoneId = " + gsmPhone.getPhoneId());
                if (mainPhoneId != -1) {
                    if (mainPhoneId == gsmPhone.getPhoneId()) {
                        break;
                    }
                } else {
                    if (gsmPhone.getSubId() > 0) {
                        break;
                    }
                }
                gsmPhone = null;
            }
        }
        log("getPhoneById = " + gsmPhone);
        return gsmPhone;
    }

    private boolean isSupportCdma(int slotId) {
        boolean isSupportCdma = false;
        String[] type = MtkTelephonyManagerEx.getDefault().getSupportCardType(slotId);
        if (type != null) {
            for (int i = 0; i < type.length; i++) {
                if ("RUIM".equals(type[i]) || "CSIM".equals(type[i])) {
                    isSupportCdma = true;
                }
            }
        }
        log("slotId = " + slotId + " isSupportCdma = " + isSupportCdma);
        return isSupportCdma;
    }

    private boolean isCdmaCard(int slotId) {
        boolean isCdmaCard = false;
        if (isSupportCdma(slotId) || MtkTelephonyManagerEx.getDefault().isCt3gDualMode(slotId)) {
            isCdmaCard = true;
        }
        log("slotId = " + slotId + " isCdmaCard = " + isCdmaCard);
        return isCdmaCard;
    }

    private int getCDMAPhoneId() {
        log("getCDMAPhoneId()");
        int phoneCount = TelephonyManager.from(this).getPhoneCount();
        for (int i = 0; i < phoneCount; i++) {
            if(!TelephonyManager.from(this).hasIccCard(i)) {
                continue;
            }

            int[] subIdList = SubscriptionManager.getSubId(i);
            if (subIdList != null && isCdmaCard(i)) {
                return i;
            }
        }
        return SubscriptionManager.INVALID_SIM_SLOT_INDEX;
    }

    private void fillCdmaPhoneInfo(GsmCdmaPhone gsmPhone) {
        fillPhoneInfo(gsmPhone);
    }

    private void fillPhoneInfo(GsmCdmaPhone gsmPhone) {
        //Empty code.
    }

    private void fillCdmaInfo() {
        int phoneId = getMainSlotId();
        if (getCDMAPhoneId() != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            if (!isCdmaCard(phoneId)) {
                phoneId = getCDMAPhoneId();
            }
            GsmCdmaPhone cdmaPhone = getPhoneById(phoneId);
            fillCdmaPhoneInfo(cdmaPhone);
        } else {
            GsmCdmaPhone gsmPhone = getPhoneById(phoneId);
            fillPhoneInfo(gsmPhone);
        }
    }

    private void dumpInfoMap() {
        Set<String> set = mInfoMap.keySet();
        log("CdmaInfoSpecification dump start");

        // Dump device info.
        log(KEY_PRODUCT_MODEL + " = " + formatLogValue(mInfoMap.get(KEY_PRODUCT_MODEL)));
        log(KEY_HARDWARE_VERSION + " = " + formatLogValue(mInfoMap.get(KEY_HARDWARE_VERSION)));
        log(KEY_SOFTWARE_VERSION + " = " + formatLogValue(mInfoMap.get(KEY_SOFTWARE_VERSION)));
        log(KEY_ANDROID_VERSION+ " = " + mInfoMap.get(KEY_ANDROID_VERSION));
        log(KEY_STORAGE+ " = " + formatLogValue(mInfoMap.get(KEY_STORAGE)));
        log(KEY_RAM_SIZE + " = " + formatLogValue(mInfoMap.get(KEY_RAM_SIZE)));
        log(KEY_MEID + " = " + formatLogValue(mInfoMap.get(KEY_MEID)));
        log(KEY_IMEI_1 + " = " + formatLogValue(mInfoMap.get(KEY_IMEI_1)));
        log(KEY_IMEI_2 + " = " + formatLogValue(mInfoMap.get(KEY_IMEI_2)));

        // Dump sim info.
        log(KEY_ICCID_1 + " = " + formatLogValue(mInfoMap.get(KEY_ICCID_1)));
        log(KEY_ICCID_2 + " = " + formatLogValue(mInfoMap.get(KEY_ICCID_2)));
        log(KEY_LTE_IMSI_1 + " = " + formatLogValue(mInfoMap.get(KEY_LTE_IMSI_1)));
        log(KEY_LTE_IMSI_2 + " = " + formatLogValue(mInfoMap.get(KEY_LTE_IMSI_2)));
        log(KEY_CDMA_IMSI_1 + " = " + formatLogValue(mInfoMap.get(KEY_CDMA_IMSI_1)));
        log(KEY_CDMA_IMSI_2 + " = " + formatLogValue(mInfoMap.get(KEY_CDMA_IMSI_2)));
        log(KEY_CELLID_1 + " = " + formatLogValue(mInfoMap.get(KEY_CELLID_1)));
        log(KEY_CELLID_2 + " = " + formatLogValue(mInfoMap.get(KEY_CELLID_2)));
        log(KEY_OPERATOR_1 + " = " + formatLogValue(mInfoMap.get(KEY_OPERATOR_1)));
        log(KEY_OPERATOR_2 + " = " + formatLogValue(mInfoMap.get(KEY_OPERATOR_2)));
        log(KEY_NETWORK_1 + " = " + formatLogValue(mInfoMap.get(KEY_NETWORK_1)));
        log(KEY_NETWORK_2 + " = " + formatLogValue(mInfoMap.get(KEY_NETWORK_2)));
        log("CdmaInfoSpecification dump end");
    }

    private String formatLogValue(String value) {
        String formatValue = value;
        if (DEBUG && !FORCE_PRINT) {
            if (!TextUtils.isEmpty(formatValue)) {
                int i = formatValue.length() - LOG_LENGTH;
                if (i > 0) {
                    formatValue = String.format("XXXXX%s", formatValue.substring(i));
                } else {
                    formatValue = "XXXXX";
                }
            }

        }
        return formatValue;
    }


    private boolean isCTCard(int slotId) {
        int[] subId = SubscriptionManager.getSubId(slotId);
        if (subId == null || subId[0] < 0) {
            return false;
        }

        String networkOperator = mTelephonyManager.getNetworkOperator(subId[0]);
        if (networkOperator.equals("46003")
                || networkOperator.equals("46011")
                || networkOperator.equals("45502")) {

            String simOperator = mTelephonyManager.getSimOperator(subId[0]);
            if (simOperator.equals("46003") || simOperator.equals("46011")
                    || simOperator.equals("20404") || simOperator.equals("45403")
                    || simOperator.equals("45431")) {
                return true;
            }
        }
        return false;
    }

    private class CellInfoAsyncTask extends AsyncTask<Void, Void, Integer> {
        private int mSlotId = -1;

        public CellInfoAsyncTask(int slotId) {
            mSlotId = slotId;
        }

        private int getCellLocationId(int slotId) {
            log("CellInfoAsyncTask.getCellLocationId");
            CellLocation location = MtkTelephonyManagerEx.getDefault().getCellLocation(slotId);
            if (location == null) {
                return 0;
            }

            if (location instanceof CdmaCellLocation) {
                return ((CdmaCellLocation) location).getBaseStationId();
            } else if (location instanceof  GsmCellLocation) {
                return ((GsmCellLocation) location).getCid();
            }
            return 0;
        }

        @Override
        protected Integer doInBackground(Void... params) {
            log("CellInfoAsyncTask.doInBackground");
            int cellId = 0;

            if (!mTelephonyManager.hasIccCard(mSlotId)) {
                return 0;
            }

            int[] subId = SubscriptionManager.getSubId(mSlotId);
            if (subId == null || subId[0] < 0) {
                return 0;
            }

            // CS(voice) is 4G(LTE) or PS(data) is 4G(LTE)
            ServiceState serviceState = mTelephonyManager.getServiceStateForSubscriber(subId[0]);
            if (serviceState.getVoiceRegState() != ServiceState.STATE_IN_SERVICE
                    && serviceState.getDataRegState() != ServiceState.STATE_IN_SERVICE) {
                return 0;
            }

            List<CellInfo> infoList = MtkTelephonyManagerEx.getDefault().getAllCellInfo(mSlotId);

            boolean isCTCard = isCTCard(mSlotId);

            boolean isLte = (serviceState.getVoiceRegState() == ServiceState.STATE_IN_SERVICE
                    && ServiceState.isLte(serviceState.getRilVoiceRadioTechnology()))
                    || (serviceState.getDataRegState() == ServiceState.STATE_IN_SERVICE
                        && ServiceState.isLte(serviceState.getRilDataRadioTechnology()));

            if (infoList != null && infoList.size() > 0) {
                for (CellInfo cellInfo : infoList) {
                    if (cellInfo.isRegistered()) {
                        log("doInBackground, isRegistered");
                        if (isCTCard && !isLte) {
                            if (cellInfo instanceof CellInfoCdma) {
                                cellId = ((CellInfoCdma) cellInfo).getCellIdentity().
                                        getBasestationId();
                                break;
                            }
                        } else {
                            if (cellInfo instanceof CellInfoLte) {
                                cellId = ((CellInfoLte) cellInfo).getCellIdentity().getCi();
                                break;
                            } else if (cellInfo instanceof CellInfoWcdma) {
                                cellId = ((CellInfoWcdma) cellInfo).getCellIdentity().getCid();
                                break;
                            } else if (cellInfo instanceof CellInfoGsm) {
                                cellId = ((CellInfoGsm) cellInfo).getCellIdentity().getCid();
                                break;
                            } else if (cellInfo instanceof CellInfoCdma) {
                                cellId = ((CellInfoCdma) cellInfo).getCellIdentity().
                                        getBasestationId();
                                break;
                            }
                        }
                    }
                }
            }

            if (cellId <= 0) {
                cellId = getCellLocationId(mSlotId);
            }
            if (DEBUG || FORCE_PRINT) {
                log("doInBackground, mSlotId = " + mSlotId + ", cellId = "
                      + formatLogValue(new Integer(cellId).toString()));
            }
           return new Integer(cellId);
        }

        @Override
        protected void onPostExecute(Integer integer) {
            if (integer != null && integer.intValue() > 0) {
                updatePreferenceByKey(KEY_CELLID + (mSlotId + 1), "" + integer.intValue());
            }
        }
    }
}
