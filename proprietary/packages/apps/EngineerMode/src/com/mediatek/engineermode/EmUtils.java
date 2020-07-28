package com.mediatek.engineermode;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.widget.Toast;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.engineermode.aospradio.EmRadioHidlAosp;

import java.util.NoSuchElementException;

import vendor.mediatek.hardware.engineermode.V1_2.IEmd;

/**
 * Common functions.
 */
public class EmUtils {
    public static final String TAG = "EmUtils";
    public static final int OEM_RIL_REQUEST_PHOHE = 0;
    public static final int OEM_RIL_REQUEST_HIDL = 1;
    public static final int OEM_RIL_REQUEST_MODE = OEM_RIL_REQUEST_HIDL;
    public static Toast mToast = null;
    public static IEmd mEmHIDLService = null;
    public static Phone mPhone = null;
    public static Phone mPhoneSlot1 = null;
    public static Phone mPhoneSlot2 = null;
    public static Phone mPhoneMain = null;
    public static Phone mPhoneCdma = null;
    public static final int RADIO_INDICATION_TYPE_NONE = -1;
    public static final int RADIO_INDICATION_TYPE_URCINFO = 0;
    public static final int RADIO_INDICATION_TYPE_NETWORKINFO = 1;
    public static final int RADIO_INDICATION_TYPE_TXPOWER_INFO = 2;
    public static final int RADIO_INDICATION_TYPE_PHONE_STAUS_CHANGE = 3;
    public static Phone getmPhone(int phoneid) {
        switch (phoneid) {
            case -1:
                mPhoneMain = PhoneFactory.getPhone(ModemCategory.getCapabilitySim());
                mPhone = mPhoneMain;
                break;
            case PhoneConstants.SIM_ID_1:
                if (mPhoneSlot1 == null) {
                    mPhoneSlot1 = PhoneFactory.getPhone(PhoneConstants.SIM_ID_1);
                }
                mPhone = mPhoneSlot1;
                break;
            case PhoneConstants.SIM_ID_2:
                if (mPhoneSlot2 == null) {
                    mPhoneSlot2 = PhoneFactory.getPhone(PhoneConstants.SIM_ID_2);
                }
                mPhone = mPhoneSlot2;
                break;
            case 0xff:
                mPhoneCdma = ModemCategory.getCdmaPhone();
                mPhone = mPhoneCdma;
                break;
        }
        Elog.v(TAG, "getmPhone,phoneid = " + phoneid);
        return mPhone;
    }

    private static void invokeOemRilRequestStringsEmPhone(int phoneid, String[] command, Message
            response) {
        try {
            getmPhone(phoneid).invokeOemRilRequestStrings(command, response);
        } catch (Exception e) {
            Elog.v(TAG, e.getMessage());
            Elog.v(TAG, "get phone invokeOemRilRequestStrings failed");
        }
    }

    private static void invokeOemRilRequestStringsEmHidl(int phoneid, String[] command, Message
            response) {
        EmRadioHidl.invokeOemRilRequestStringsEm(phoneid, command, response);
    }

    public static void invokeOemRilRequestStringsEm(int phoneid, String[] command, Message
            response) {
        if (OEM_RIL_REQUEST_MODE == OEM_RIL_REQUEST_HIDL) {
            invokeOemRilRequestStringsEmHidl(phoneid, command, response);
        } else {
            invokeOemRilRequestStringsEmPhone(phoneid, command, response);
        }
    }

    public static void invokeOemRilRequestStringsEm(String[] command, Message
            response) {
        invokeOemRilRequestStringsEm(-1, command, response);
    }

    public static void invokeOemRilRequestStringsEm(boolean isCdma, String[] command, Message
            response) {
        if (isCdma)
            invokeOemRilRequestStringsEm(0xff, command, response);
        else
            invokeOemRilRequestStringsEm(-1, command, response);
    }

    public static void invokeOemRilRequestRawEmPhone(int phoneid, byte[] command, Message
            response) {
        try {
            getmPhone(phoneid).invokeOemRilRequestRaw(command, response);
        } catch (Exception e) {
            Elog.v(TAG, e.getMessage());
            Elog.v(TAG, "get phone invokeOemRilRequestRaw failed");
        }
    }

    public static void invokeOemRilRequestRawEmHidl(int phoneid, byte[] command, Message
            response) {
        EmRadioHidl.invokeOemRilRequestRawEm(phoneid, command, response);
    }


    public static void invokeOemRilRequestRawEm(int phoneid, byte[] command, Message
            response) {
        if (OEM_RIL_REQUEST_MODE == OEM_RIL_REQUEST_HIDL) {
            invokeOemRilRequestRawEmHidl(phoneid, command, response);
        } else {
            invokeOemRilRequestRawEmPhone(phoneid, command, response);
        }
    }

    public static void invokeOemRilRequestRawEm(byte[] command, Message
            response) {
        invokeOemRilRequestRawEm(-1, command, response);
    }

    public static IEmd getEmHidlService() {
        Elog.v(TAG, "getEmHidlService ...");
        if (mEmHIDLService == null) {
            Elog.v(TAG, "getEmHidlService init...");
            try {
                mEmHIDLService = IEmd.getService("EmHidlServer", true);
            } catch (RemoteException e) {
                e.printStackTrace();
                Elog.e(TAG, "EmHIDLConnection exception1 ...");
                Elog.e(TAG, e.getMessage());
            } catch (NoSuchElementException e) {
                e.printStackTrace();
                Elog.e(TAG, "EmHIDLConnection exception2 ...");
                Elog.e(TAG, e.getMessage());
            }
        }
        return mEmHIDLService;
    }

    public static void rebootModemPhone() {
        try {
            getmPhone(-1).invokeOemRilRequestStrings(new String[]{"SET_TRM", "2"}, null);
        } catch (Exception e) {
            Elog.v(TAG, e.getMessage());
            Elog.v(TAG, "rebootModem SET_TRM 2 failed");
        }
        Elog.d(TAG, "rebootModem SET_TRM 2");
    }

    public static void rebootModemHidl() {
        EmRadioHidl.rebootModemHidl();
        Elog.d(TAG, "rebootModemHidl");
    }

    public static void rebootModem() {
        if (OEM_RIL_REQUEST_MODE == OEM_RIL_REQUEST_HIDL) {
            rebootModemHidl();
        } else {
            rebootModemPhone();
        }
    }

    public static String systemPropertyGet(String property_name, String default_value) {
        String property_value = "";
        try {
            property_value = SystemProperties.get(property_name, default_value);
        } catch (Exception e) {
            Elog.e(TAG, "EmUtils systemPropertyGet failed");
        }

        return property_value;
    }

    public static boolean systemPropertySet(String property_name, String set_value) {
        try {
            SystemProperties.set(property_name, set_value);
            return true;
        } catch (Exception e) {
            Elog.e(TAG, "EmUtils systemPropertySet failed :" + property_name);
            return false;
        }
    }

    public static void showToast(String msg, int time) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(EmApplication.getContext(), msg, time);
        mToast.show();
    }

    public static void showToast(int msg_id, int time) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(EmApplication.getContext(), msg_id, time);
        mToast.show();
    }

    public static void showToast(int msg_id) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(EmApplication.getContext(), msg_id, Toast.LENGTH_SHORT);
        mToast.show();
    }

    public static void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(EmApplication.getContext(), msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    public static void showToast(String msg, boolean last) {
        if (last == false) {
            if (mToast != null) {
                mToast.cancel();
            }
        }
        mToast = Toast.makeText(EmApplication.getContext(), msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    public static void setAirplaneModeEnabled(boolean enabled) {
        Elog.d(TAG, "setAirplaneModEnabled = " + enabled);
        final ConnectivityManager mgr =
                (ConnectivityManager) EmApplication.getContext().getSystemService(EmApplication
                        .getContext().CONNECTIVITY_SERVICE);
        mgr.setAirplaneMode(enabled);
    }

    public static void initPoweroffmdMode(boolean enabled, boolean RFonly) {
        Elog.d(TAG, "initPoweroffmdMode: " + enabled + ",RFonly: " + RFonly);
        systemPropertySet("vendor.ril.test.poweroffmd", RFonly ? "0" : "1");
        systemPropertySet("vendor.ril.testmode", enabled ? "1" : "0");
    }

    public static boolean ifAirplaneModeEnabled() {
        boolean isAirplaneModeOn = Settings.System.getInt(EmApplication.getContext()
                .getContentResolver(), Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
        Elog.d(TAG, "isAirplaneModeOn: " + isAirplaneModeOn);
        return isAirplaneModeOn;
    }

    public static void writeSharedPreferences(String preferencesName, String key, String value) {
        final SharedPreferences preferences = EmApplication.getContext().
                getSharedPreferences(preferencesName, EmApplication.getContext().MODE_PRIVATE);
        final SharedPreferences.Editor editor = preferences.edit();
        editor.putString(key, value);
        editor.commit();
    }

    public static String readSharedPreferences(String preferencesName,
                                               String key, String default_value) {
        String value = "";
        SharedPreferences preferences = EmApplication.getContext().
                getSharedPreferences(preferencesName, EmApplication.getContext().MODE_PRIVATE);
        value = preferences.getString(key, default_value);
        return value;
    }

    public static void registerForNetworkInfo(int phoneid, Handler handler, int what) {
        EmRadioHidl.mRadioIndicationType = RADIO_INDICATION_TYPE_NETWORKINFO;
        EmRadioHidl.mSetRadioIndicationMtk(phoneid, handler, what, true);
    }

    public static void unregisterForNetworkInfo(int phoneid) {
        EmRadioHidl.mRadioIndicationType =RADIO_INDICATION_TYPE_NONE;
        EmRadioHidl.mSetRadioIndicationMtk(phoneid, null, 0, false);
    }

    public static void registerForTxpowerInfo(Handler handler, int what) {
        EmRadioHidl.mRadioIndicationType = RADIO_INDICATION_TYPE_TXPOWER_INFO;
        EmRadioHidl.mSetRadioIndicationMtk(0, handler, what, true);
    }

    public static void unregisterForTxpowerInfo() {
        EmRadioHidl.mRadioIndicationType = RADIO_INDICATION_TYPE_NONE;
        EmRadioHidl.mSetRadioIndicationMtk(0, null, 0, false);
    }

    public static void registerForUrcInfo(int phoneid, Handler handler, int what) {
        EmRadioHidl.mRadioIndicationType = RADIO_INDICATION_TYPE_URCINFO;
        EmRadioHidl.mSetRadioIndicationMtk(phoneid, handler, what, true);
    }

    public static void unregisterForUrcInfo(int phoneid) {
        EmRadioHidl.mRadioIndicationType = RADIO_INDICATION_TYPE_NONE;
        EmRadioHidl.mSetRadioIndicationMtk(phoneid, null, 0, false);
    }

    public static void registerForradioStateChanged(int phoneid, Handler handler, int what) {
        EmRadioHidlAosp.mRadioIndicationType = RADIO_INDICATION_TYPE_PHONE_STAUS_CHANGE;
        EmRadioHidlAosp.mSetRadioIndicationAosp(phoneid, handler, what, true);
    }

    public static void unregisterradioStateChanged(int phoneid) {
        EmRadioHidlAosp.mRadioIndicationType = RADIO_INDICATION_TYPE_NONE;
        EmRadioHidlAosp.mSetRadioIndicationAosp(phoneid, null, 0, false);
    }

    public static void reloadModemType(int modemType) {
        EmRadioHidl.reloadModemType(modemType);
    }

    public static void storeModemType(int modemType) {
        EmRadioHidl.storeModemType(modemType);
    }

    private static final String FINE_LOCATION_PERMISSION_NAME =
            "android.permission.ACCESS_FINE_LOCATION";
    private static final String COARSE_LOCATION_PERMISSION_NAME =
            "android.permission.ACCESS_COARSE_LOCATION";

    /**
     * Function to check Non-Framework LBS LOCATION permission.
     * @param context Context environment
     * @param showPermissionDlg true for show permission dialog
     * @return true for permission granted while false for not granted
     */
    public static boolean checkLocationProxyAppPermission(Context context,
            boolean showPermissionDlg) {
        PackageManager pkgMgr = context.getPackageManager();

        String gnssProxyPackageName = loadProxyNameFromCarrierConfig();
        Elog.d(TAG, "gnssProxyPackageName from carrierConfig: " + gnssProxyPackageName);

        boolean proxyAppLocationGranted = hasLocationPermission(pkgMgr, gnssProxyPackageName);
        Elog.d(TAG, "proxyAppLocationGranted = " + proxyAppLocationGranted);

        if (showPermissionDlg && (context instanceof Activity)) {
            if (isPackageInstalled(pkgMgr, gnssProxyPackageName)) {
                if (!proxyAppLocationGranted) {
                    showRequestLbsPermissionDialog((Activity) context, "MTK Non-Framework LBS",
                        "Non-Framework LBS LOCATION permission is not granted. "
                        + "EM all location tests may be failed if it is not granted. "
                        + "Would you like to launch the APP?");
                }
            } else {
                Elog.d(TAG, "Skip dialog, because package is not installed: "
                        + gnssProxyPackageName);
                showToast("Non-Framework LBS LOCATION permission is not granted");
                ((Activity) context).finish();
            }
        }

        return proxyAppLocationGranted;
    }

    private static boolean isPackageInstalled(PackageManager pkgMgr, String packagename) {
        try {
            pkgMgr.getPackageInfo(packagename, 0);
            return true;
        } catch (NameNotFoundException e) {
            return false;
        }
    }

    private static void showRequestLbsPermissionDialog(final Activity activity, CharSequence title,
                                           CharSequence message) {
        AlertDialog.Builder builder = new AlertDialog.Builder(activity);
        builder.setTitle(title);
        builder.setMessage(message);
        builder.setCancelable(false);

        builder.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                startComponent();
            }
        });

        builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int id) {
                activity.finish();
            }
        });

        AlertDialog alert = builder.create();
        alert.show();
    }

    private static boolean hasLocationPermission(PackageManager pkgMgr, String pkgName) {
        return (pkgMgr.checkPermission(FINE_LOCATION_PERMISSION_NAME, pkgName)
                == PackageManager.PERMISSION_GRANTED &&
                pkgMgr.checkPermission(COARSE_LOCATION_PERMISSION_NAME, pkgName)
                        == PackageManager.PERMISSION_GRANTED);
    }

    private static void startComponent() {
        String gnssProxyPackageName = loadProxyNameFromCarrierConfig();
        PackageManager pkgMgr = EmApplication.getContext().getPackageManager();
        try {
            Intent launchIntent = pkgMgr.getLaunchIntentForPackage(gnssProxyPackageName);
            EmApplication.getContext().startActivity(launchIntent);
        } catch (Exception e) {
            Elog.d(TAG, "Failed to launch package: " + gnssProxyPackageName);
            launchPackageSettings(gnssProxyPackageName);
        }
    }

    private static void launchPackageSettings(String packageName) {
        try {
            Intent intent = new Intent();
            intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            Uri uri = Uri.fromParts("package", packageName, null);
            intent.setData(uri);
            EmApplication.getContext().startActivity(intent);
        } catch (Exception e) {
            Elog.e(TAG, "Failed to launch package settings: " + packageName);
            showToast("ERR: start settings failed!!", Toast.LENGTH_LONG);
        }
    }

    private static String loadProxyNameFromCarrierConfig() {
        CarrierConfigManager configManager = (CarrierConfigManager)
                EmApplication.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
        if (configManager == null) {
            return "";
        }

        int ddSubId = SubscriptionManager.getDefaultDataSubscriptionId();
        PersistableBundle configs = SubscriptionManager.isValidSubscriptionId(ddSubId)
                ? configManager.getConfigForSubId(ddSubId) : null;
        if (configs == null) {
            Elog.d(TAG, "SIM not ready, use default carrier config.");
            configs = CarrierConfigManager.getDefaultConfig();
        }

        String value = (String) configs.get(CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING);
        Elog.d(TAG, CarrierConfigManager.Gps.KEY_NFW_PROXY_APPS_STRING + ": " + value);
        if (value != null) {
            String[] strings = value.trim().split(" ");
            /// use first one package as proxy package
            return strings[0];
        } else {
            return "";
        }
    }

}
