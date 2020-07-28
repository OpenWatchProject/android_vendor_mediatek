package com.mediatek.apmonitor.mdmi;

import android.util.Log;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.IOException;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;


class MdmiCategoryMapLoader {
    private static final String TAG = "MDMI-Provider";
    private static Map<String, String> sCategoryMap;

    private static final String CATEGORY_DEBUG_OID = "1.1";
    private static final String CATEGORY_LTE_OID   = "1.2";
    private static final String CATEGORY_WCDMA_OID = "1.3";
    private static final String CATEGORY_EMBMS_OID = "1.4";
    private static final String CATEGORY_IMS_OID   = "1.6";
    private static final String CATEGORY_WIFI_OID  = "1.7";
    private static final String CATEGORY_GSM_OID   = "1.8";
    private static final String CATEGORY_UMTS_OID  = "1.9";
    private static final String CATEGORY_HSUPA_OID = "1.11";
    private static final String CATEGORY_HSDPA_OID = "1.12";

    private static final String MDMI_LIB_DEBUG = "libVzw_mdmi_debug.so";
    private static final String MDMI_LIB_LTE = "libVzw_mdmi_lte.so";
    private static final String MDMI_LIB_GSM = "libVzw_mdmi_gsm.so";
    private static final String MDMI_LIB_UMTS = "libVzw_mdmi_umts.so";
    private static final String MDMI_LIB_WIFI = "libVzw_mdmi_wifi.so";
    private static final String MDMI_LIB_IMS = "libVzw_mdmi_ims.so";
    private static final String MDMI_LIB_COMMANDS = "libVzw_mdmi_commands.so";
    private static final String MDMI_LIB_EMBMS = "libVzw_mdmi_embms.so";
    private static final String MDMI_LIB_WCDMA = "libVzw_mdmi_wcdma.so";
    private static final String MDMI_LIB_HSUPA = "libVzw_mdmi_hsupa.so";
    private static final String MDMI_LIB_HSDPA = "libVzw_mdmi_hsdpa.so";

    private static final String MDMI_LIB_CUSTOMIZED_FILE = "/system/etc/mdmi/mdmi_lib_customize.prop";

    private MdmiCategoryMapLoader() {}

    MdmiCategoryMapLoader(boolean needCustomizationConfig) {
        sCategoryMap = new HashMap<String, String>();
        loadDefaultConfig();
        Log.d(TAG, "load default config done!");

        if (needCustomizationConfig) {
            loadCustomizationConfig();
        } else {
            Log.d(TAG, "skip customization config");
        }
    }

    public Map<String, String> getMdmiCategoryMap() {
        return sCategoryMap;
    }

    private void loadDefaultConfig() {
        sCategoryMap.put(CATEGORY_DEBUG_OID, MDMI_LIB_DEBUG);
        sCategoryMap.put(CATEGORY_LTE_OID, MDMI_LIB_LTE);
        sCategoryMap.put(CATEGORY_WCDMA_OID, MDMI_LIB_WCDMA);
        sCategoryMap.put(CATEGORY_EMBMS_OID, MDMI_LIB_EMBMS);
        sCategoryMap.put(CATEGORY_IMS_OID, MDMI_LIB_IMS);
        sCategoryMap.put(CATEGORY_WIFI_OID, MDMI_LIB_WIFI);
        sCategoryMap.put(CATEGORY_GSM_OID, MDMI_LIB_GSM);
        sCategoryMap.put(CATEGORY_UMTS_OID, MDMI_LIB_UMTS);
        sCategoryMap.put(CATEGORY_HSUPA_OID, MDMI_LIB_HSUPA);
        sCategoryMap.put(CATEGORY_HSDPA_OID, MDMI_LIB_HSDPA);
    }

    private void loadCustomizationConfig() {
        try {
            InputStream inputStream = new FileInputStream(MDMI_LIB_CUSTOMIZED_FILE);

            Properties properties = new Properties();
            properties.load(inputStream);

            Iterator iter = sCategoryMap.entrySet().iterator();
            while (iter.hasNext()) {
                Map.Entry entry = (Map.Entry)iter.next();
                String key = (String) entry.getKey();
                Log.d(TAG, "Get key: " + key);
                if (properties.getProperty(key) != null) {
                    String value = (String) properties.getProperty(key);
                    sCategoryMap.put(key, value);
                    Log.d(TAG, "Replace value of key: " + key +
                            " from " + sCategoryMap.get(key) + " to " + value);
                }
            }
        } catch (FileNotFoundException e) {
            Log.d(TAG, "No customization config");
        } catch (IOException e) {
            Log.e(TAG, "Got exception: " + e);
        }
    }
}
