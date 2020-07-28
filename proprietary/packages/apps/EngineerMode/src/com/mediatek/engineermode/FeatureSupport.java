package com.mediatek.engineermode;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.os.RemoteException;
import android.os.SystemProperties;

import java.util.List;

/**
 * Class to store some feature support information.
 */
public class FeatureSupport {
    public static final String FK_FD_SUPPORT = "ro.vendor.mtk_fd_support";
    public static final String FK_WFD_SUPPORT = "ro.vendor.mtk_wfd_support";
    public static final String FK_DEVREG_APP = "ro.vendor.mtk_devreg_app";
    public static final String FK_WCDMA_PREFERRED = "ro.vendor.mtk_rat_wcdma_preferred";
    public static final String FK_AAL_SUPPORT = "ro.vendor.mtk_aal_support";
    public static final String FK_CT4GREG_APP = "ro.vendor.mtk_ct4greg_app";
    public static final String FK_VILTE_SUPPORT = "persist.vendor.vilte_support";
    public static final String FK_MD_WM_SUPPORT = "ro.vendor.mtk_md_world_mode_support";
    public static final String FK_MTK_DSDA_SUPPORT = "persist.vendor.radio.multisim.config";
    public static final String FK_MTK_TEL_LOG_SUPPORT = "persist.vendor.log.tel_log_ctrl";
    public static final String FK_MTK_93_SUPPORT = "ro.vendor.mtk_ril_mode";
    public static final String FK_APC_SUPPORT = "vendor.ril.apc.support";
    public static final String FK_MTK_WFC_SUPPORT = "persist.vendor.mtk_wfc_support";
    public static final String FK_SINGLE_BIN_MODEM_SUPPORT =
            "ro.vendor.mtk_single_bin_modem_support";
    public static final String FK_MTK_TELEPHONY_ADD_ON_POLICY =
            "ro.vendor.mtk_telephony_add_on_policy";
    public static final String FK_BUILD_TYPE = "ro.build.type";
    public static final String FK_GMO_RAM_OPTIMIZE = "ro.vendor.gmo.ram_optimize";
    public static final String FK_IMS_SUPPORT = "persist.vendor.ims_support";
    public static final String FK_BOARD_PLATFORM = "ro.board.platform";
    public static final String FK_GWSD_SUPPORT = "ro.vendor.mtk_gwsd_support";
    public static final String PK_CDS_EM = "com.mediatek.connectivity";
    public static final String ENG_LOAD = "eng";
    public static final String USER_LOAD = "user";
    public static final String USERDEBUG_LOAD = "userdebug";
    private static final String TAG = "FeatureSupport";
    private static final String SUPPORTED = "1";
    private static final String[] MODEM_GEN95_ARRAY = {"mt6785", "mt6779"};
    private static final String[] MODEM_GEN97_ARRAY = {"mt6885"};
    public static boolean is_support_3GOnly_md = false;
    public static boolean is_support_90_md = false;
    public static boolean is_support_91_md = false;
    public static boolean is_support_92_md = false;
    public static boolean is_support_93_md = false;

    public static boolean is3GOnlyModem() {
        if (ChipSupport.getChip() >= ChipSupport.MTK_6570_SUPPORT &&
                ChipSupport.getChip() <= ChipSupport.MTK_6580_SUPPORT) {
            is_support_3GOnly_md = true;
        } else {
            is_support_3GOnly_md = false;
        }
        return is_support_3GOnly_md;
    }

    public static boolean is90Modem() {
        if (ChipSupport.getChip() == ChipSupport.MTK_6735_SUPPORT) {
            is_support_90_md = true;
        } else {
            is_support_90_md = false;
        }
        return is_support_90_md;
    }

    public static boolean is91Modem() {
        if (ChipSupport.getChip() >= ChipSupport.MTK_6755_SUPPORT &&
                ChipSupport.getChip() <= ChipSupport.MTK_6757CH_SUPPORT) {
            is_support_91_md = true;
        } else {
            is_support_91_md = false;
        }
        return is_support_91_md;
    }

    public static boolean is92Modem() {
        if (ChipSupport.getChip() > ChipSupport.MTK_6757CH_SUPPORT &&
                !is93Modem()) {
            is_support_92_md = true;
        } else {
            is_support_92_md = false;
        }
        return is_support_92_md;
    }

    public static boolean is93Modem() {
        is_support_93_md = "c6m_1rild".equals(SystemProperties
                .get(FK_MTK_93_SUPPORT)) ? true : false;
        return is_support_93_md;
    }

    public static boolean is95Modem() {
        String value = EmUtils.systemPropertyGet(FK_BOARD_PLATFORM, "0");
        Elog.d(TAG, "ro.board.platform = " + value);
        for (String name : MODEM_GEN95_ARRAY) {
            if (name.equals(value)) {
                Elog.d(TAG, "it is Gen95 modem");
                return true;
            }
        }
        return false;
    }

    public static boolean is97Modem() {
        String value = EmUtils.systemPropertyGet(FK_BOARD_PLATFORM, "0");
        Elog.d(TAG, "ro.board.platform = " + value);
        for (String name : MODEM_GEN97_ARRAY) {
            if (name.equals(value)) {
                Elog.d(TAG, "it is Gen97 modem");
                return true;
            }
        }
        return false;
    }

    /**
     * Query feature support information.
     *
     * @param featureKey The feature to query information
     * @return true if the feature is support
     */
    public static boolean isSupported(String featureKey) {
        return SUPPORTED.equals(SystemProperties.get(featureKey));
    }

    /**
     * Query current system property value.
     *
     * @param propertyName The property to query
     * @return value of the system property
     */
    public static String getProperty(String propertyName) {
        return SystemProperties.get(propertyName);
    }

    /**
     * Get em svr support or not.
     *
     * @return true or false.
     */
    public static boolean isSupportedEmSrv() {
        boolean isSupported = true;
        if (SUPPORTED.equals(SystemProperties.get(FK_GMO_RAM_OPTIMIZE))) {
            if (!(ENG_LOAD.equals(SystemProperties.get(FK_BUILD_TYPE)))) {
                isSupported = false;
            }
        }
        return isSupported;
    }

    /**
     * Query if certain package is installed on device.
     *
     * @param context     The context environment
     * @param packageName The package to query
     * @return true if the package is installed
     */
    public static boolean isPackageExisted(Context context, String packageName) {
        PackageManager pm = context.getPackageManager();
        List<ApplicationInfo> packages = pm.getInstalledApplications(0);
        for (ApplicationInfo ai : packages) {
            if (ai.packageName.equals(packageName)) {
                return true;
            }
        }
        return false;
    }

    public static boolean isEngLoad() {
        return ENG_LOAD.equals(SystemProperties.get(FK_BUILD_TYPE));
    }

    public static boolean isUserLoad() {
        return USER_LOAD.equals(SystemProperties.get(FK_BUILD_TYPE));
    }

    public static boolean isUserDebugLoad() {
        return USERDEBUG_LOAD.equals(SystemProperties.get(FK_BUILD_TYPE));
    }

    public static boolean isSupportWfc() {
        return SystemProperties.get(FK_MTK_WFC_SUPPORT, "0").equals("1");
    }

    /**
     * Check if Wifi is support.
     *
     * @param context The context environment
     * @return true for supported while false for unsupported
     */
    public static boolean isWifiSupport(Context context) {
        PackageManager pm = context.getPackageManager();
        return pm.hasSystemFeature(PackageManager.FEATURE_WIFI);
    }

    /**
     * Check if BT EM support.
     *
     * @return true for supported while false for unsupported
     */
    public static boolean isEmBTSupport() {
        try {
            int result = EmUtils.getEmHidlService().btIsEmSupport();
            return (result == 1);
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        }
    }

    /**
     * Check if NFC ST support.
     *
     * @return true for supported while false for unsupported
     */
    public static boolean isNfcSupport() {
        try {
            int result = EmUtils.getEmHidlService().isNfcSupport();
            return (result == 1);
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        }
    }

    static boolean isGpsSupport(Context context) {
        LocationManager locManager = (LocationManager) context.getSystemService(
                Context.LOCATION_SERVICE);
        if ((locManager != null) &&
                (locManager.getProvider(LocationManager.GPS_PROVIDER) != null)) {
            return true;
        }
        return false;
    }

    public static boolean isGauge30Support() {
        try {
            int result = EmUtils.getEmHidlService().isGauge30Support();
            return (result == 1);
        } catch (RemoteException e) {
            e.printStackTrace();
            return false;
        }
    }

    public static boolean isMtkTelephonyAddOnPolicyEnable() {
        return SystemProperties.get(FK_MTK_TELEPHONY_ADD_ON_POLICY, "0").equals("0");
    }

    public static boolean isSupportTelephony(Context context) {
        return context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TELEPHONY);
    }
}
