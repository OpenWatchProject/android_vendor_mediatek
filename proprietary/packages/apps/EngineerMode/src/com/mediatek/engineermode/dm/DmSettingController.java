package com.mediatek.engineermode.dm;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.SystemProperties;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;

/**
 * Class to control telephony log.
 *
 */
public class DmSettingController {

    static final String TAG = "DmSettingController";

    private Context mContext;

    private static final String DM_SETTING_PREF_NAME = "dm_setting";
    private static final String DM_SETTGIN_PREF_LOG_ENABLED = "dm_setting_log_enabled";
    private static final String DM_SETTGIN_PREF_LOG_PKM_ENABLED = "dm_setting_log_pkm_enabled";

    private static final String PROP_DMC_FO_SYSTEM = "ro.vendor.system.mtk_dmc_support";
    private static final String PROP_DMC_FO_VENDOR = "ro.vendor.mtk_dmc_support";
    private static final String PROP_MAPI_FO_SYSTEM = "ro.vendor.system.mtk_mapi_support";
    private static final String PROP_MAPI_FO_VENDOR = "ro.vendor.mtk_mapi_support";
    private static final String PROP_MDMI_FO_SYSTEM = "ro.vendor.system.mtk_mdmi_support";
    private static final String PROP_MDMI_FO_VENDOR = "ro.vendor.mtk_mdmi_support";
    private static final String PROP_DMC_APM_ACTIVE = "vendor.dmc.apm.active";
    private static final String PROP_LOG_MUCH = "persist.vendor.logmuch";

    private static final String VALUE_ENABLE = "1";
    private static final String VALUE_DISABLE = "0";
    private static final String VALUE_TRUE = "true";
    private static final String VALUE_FALSE = "false";

    private static final int TIMEOUT_PERIOD = 5000;
    private static final int WAIT_TIME = 300;

    private static final String[] PROPERTY_M_LOG_TAG_DMC = {
        "persist.log.tag.DMC-Core",
        "persist.log.tag.DMC-TranslatorLoader",
        "persist.log.tag.DMC-TranslatorUtils",
        "persist.log.tag.DMC-ReqQManager",
        "persist.log.tag.DMC-DmcService",
        "persist.log.tag.DMC-ApmService",
        "persist.log.tag.DMC-SessionManager",
        "persist.log.tag.DMC-EventsSubscriber",
        "persist.log.tag.LCM-Subscriber",
        "persist.log.tag.APM-Subscriber",
        "persist.log.tag.MDM-Subscriber",
        "persist.log.tag.APM-SessionJ",
        "persist.log.tag.APM-SessionN",
        "persist.log.tag.APM-ServiceJ",
        "persist.log.tag.APM-KpiMonitor",
    };
    private static final String[] PROPERTY_M_LOG_TAG_MAPI = {
        "persist.log.tag.MAPI-TranslatorManager",
        "persist.log.tag.MAPI-MdiRedirectorCtrl",
        "persist.log.tag.MAPI-MdiRedirector",
        "persist.log.tag.MAPI-NetworkSocketConnection",
        "persist.log.tag.MAPI-SocketConnection",
        "persist.log.tag.MAPI-SocketListener",
        "persist.log.tag.MAPI-CommandProcessor",
    };
    private static final String[] PROPERTY_M_LOG_TAG_MDMI = {
        "persist.log.tag.MDMI-TranslatorManager",
        "persist.log.tag.MDMI-MdmiRedirectorCtrl",
        "persist.log.tag.MDMI-MdmiRedirector",
        "persist.log.tag.MDMI-Permission",
        "persist.log.tag.MDMI-CoreSession",
        "persist.log.tag.MDMI-NetworkSocketConnection",
        "persist.log.tag.MDMI-SocketConnection",
        "persist.log.tag.MDMI-SocketListener",
        "persist.log.tag.MDMI-CommandProcessor",
        "persist.log.tag.MDMI-Provider",
    };
    private static final String[] PROPERTY_M_LOG_TAG_PKM = {
        "persist.log.tag.PKM-MDM",
        "persist.log.tag.PKM-Lib",
        "persist.log.tag.PKM-Monitor",
        "persist.log.tag.PKM-SA",
        "persist.log.tag.PKM-Service",
    };

    private DmSettingController() {};

    public DmSettingController(Context context) {
        mContext = context;
        // Set to log level [I] in default
        if (isDmLogEnabled()) {
            enableDmLog(true);
        } else {
            enableDmLog(false);
        }
        if (isPkmLogEnabled()) {
            enablePkmLog(true);
        } else {
            enablePkmLog(false);
        }
    }

    public static boolean isDmcSystemEnabled() {
        return VALUE_ENABLE.equals(SystemProperties.get(PROP_DMC_FO_SYSTEM));
    }
    public static boolean isDmcVendorEnabled() {
        return VALUE_ENABLE.equals(SystemProperties.get(PROP_DMC_FO_VENDOR));
    }
    public static boolean isMapiSystemEnabled() {
        return VALUE_ENABLE.equals(SystemProperties.get(PROP_MAPI_FO_SYSTEM));
    }
    public static boolean isMapiVendorEnabled() {
        return VALUE_ENABLE.equals(SystemProperties.get(PROP_MAPI_FO_VENDOR));
    }
    public static boolean isMdmiSystemEnabled() {
        return VALUE_ENABLE.equals(SystemProperties.get(PROP_MDMI_FO_SYSTEM));
    }
    public static boolean isMdmiVendorEnabled() {
        return VALUE_ENABLE.equals(SystemProperties.get(PROP_MDMI_FO_VENDOR));
    }
    public static boolean isDmcApmActivated() {
        return VALUE_ENABLE.equals(SystemProperties.get(PROP_DMC_APM_ACTIVE));
    }

    boolean activeApm(boolean active) {
        Elog.i(TAG, "activeApm(" + active + ")");
        String activeApmTar = active ? VALUE_ENABLE : VALUE_DISABLE;

        boolean activeApmSuccess = false;
        // Vendor set
        try {
            EmUtils.getEmHidlService().setEmConfigure(PROP_DMC_APM_ACTIVE, activeApmTar);

            for (int time = 0; time < TIMEOUT_PERIOD; ) {
                try {
                    Thread.sleep(WAIT_TIME);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                time += WAIT_TIME;
                if (activeApmTar.equals(SystemProperties.get(PROP_DMC_APM_ACTIVE))) {
                    Elog.i(TAG, "set success:" + SystemProperties.get(PROP_DMC_APM_ACTIVE));
                    activeApmSuccess = true;
                    break;
                }
            }
            if (!activeApmSuccess) {
                Elog.i(TAG, "set " + PROP_DMC_APM_ACTIVE + " to " + activeApmTar + " failed!");
                return false;
            }
        } catch (Exception e) {
            e.printStackTrace();
            Elog.i(TAG, "set " + PROP_DMC_APM_ACTIVE + " to " + activeApmTar + " failed!");
            return false;
        }
        return true;
    }

    boolean isPkmLogEnabled() {
        SharedPreferences preferences = mContext.getSharedPreferences(DM_SETTING_PREF_NAME, 0);
        return preferences.getBoolean(DM_SETTGIN_PREF_LOG_PKM_ENABLED, false);
    }

    boolean isDmLogEnabled() {
        SharedPreferences preferences = mContext.getSharedPreferences(DM_SETTING_PREF_NAME, 0);
        return preferences.getBoolean(DM_SETTGIN_PREF_LOG_ENABLED, false);
    }

    boolean enablePkmLog(boolean enable) {
        Elog.i(TAG, "enablePkmLog(" + enable + ")");
        if (enable) {
            for (String logTag : PROPERTY_M_LOG_TAG_PKM) {
                SystemProperties.set(logTag, "D");
            }
        } else {
            for (String logTag : PROPERTY_M_LOG_TAG_PKM) {
                SystemProperties.set(logTag, "I");
            }
        }
        SharedPreferences settings = mContext
                .getSharedPreferences(DM_SETTING_PREF_NAME, 0);
        Editor editor = settings.edit();
        editor.putBoolean(DM_SETTGIN_PREF_LOG_PKM_ENABLED, enable);
        editor.commit();
        return true;
    }

    boolean enableDmLog(boolean enable) {
        Elog.i(TAG, "enableDmLog(" + enable + ")");

        // Disable log too much if enable
        String logMuchProTar = enable ? VALUE_FALSE : VALUE_TRUE;

        boolean logMuchSetSuccess = false;
        SystemProperties.set(PROP_LOG_MUCH, logMuchProTar);

        for (int time = 0; time < TIMEOUT_PERIOD; ) {
            try {
                Thread.sleep(WAIT_TIME);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            time += WAIT_TIME;
            if (logMuchProTar.equals(SystemProperties.get(PROP_LOG_MUCH))) {
                Elog.i(TAG, "set success:" + SystemProperties.get(PROP_LOG_MUCH));
                logMuchSetSuccess = true;
                break;
            }
        }
        if (!logMuchSetSuccess) {
            Elog.i(TAG, "set " + PROP_LOG_MUCH + " to " + logMuchProTar + " failed!");
            return false;
        }

        updateDebugLog(enable);
        SharedPreferences settings = mContext
                .getSharedPreferences(DM_SETTING_PREF_NAME, 0);
        Editor editor = settings.edit();
        editor.putBoolean(DM_SETTGIN_PREF_LOG_ENABLED, enable);
        editor.commit();
        return true;
    }

    /**
     * Enable or disable Telephony and connectivity debug log.
     * @param enable true: enable log, false: disable log
     */
    private void updateDebugLog(boolean enable) {
        if (enable) {
            for (String logTag : PROPERTY_M_LOG_TAG_DMC) {
                SystemProperties.set(logTag, "D");
            }
            for (String logTag : PROPERTY_M_LOG_TAG_MDMI) {
                SystemProperties.set(logTag, "D");
            }
            for (String logTag : PROPERTY_M_LOG_TAG_MAPI) {
                SystemProperties.set(logTag, "D");
            }
        } else {
            for (String logTag : PROPERTY_M_LOG_TAG_DMC) {
                SystemProperties.set(logTag, "I");
            }
            for (String logTag : PROPERTY_M_LOG_TAG_MDMI) {
                SystemProperties.set(logTag, "I");
            }
            for (String logTag : PROPERTY_M_LOG_TAG_MAPI) {
                SystemProperties.set(logTag, "I");
            }
        }
    }
}
