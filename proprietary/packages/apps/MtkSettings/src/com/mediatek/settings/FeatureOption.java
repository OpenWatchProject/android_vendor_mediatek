package com.mediatek.settings;
import android.os.SystemProperties;

public class FeatureOption {
    public static final boolean MTK_NFC_ADDON_SUPPORT = getValue("ro.vendor.mtk_nfc_addon_support");
    public static final boolean MTK_MIRAVISION_SETTING_SUPPORT =
            getValue("ro.vendor.mtk_miravision_support");
    public static final boolean MTK_AOD_SUPPORT = getValue("ro.vendor.mtk_aod_support");
    public static final boolean MTK_OMACP_SUPPORT = getValue("ro.vendor.mtk_omacp_support");
    public static final boolean MTK_PRODUCT_IS_TABLET =
            SystemProperties.get("ro.build.characteristics").equals("tablet");
    public static final boolean MTK_VOLTE_SUPPORT = getValue("persist.vendor.volte_support");
    public static final boolean MTK_CTA_SET = getValue("ro.vendor.mtk_cta_set");
    public static final boolean MTK_AGPS_APP = getValue("ro.vendor.mtk_agps_app");
    public static final boolean MTK_GPS_SUPPORT = getValue("ro.vendor.mtk_gps_support");
    public static final boolean MTK_SYSTEM_UPDATE_SUPPORT =
        getValue("ro.vendor.mtk_system_update_support");
    public static final boolean MTK_FOTA_ENTRY = getValue("ro.vendor.mtk_fota_entry");
    public static final boolean MTK_SCOMO_ENTRY = getValue("ro.vendor.mtk_scomo_entry");
    public static final boolean MTK_MDM_SCOMO = getValue("ro.vendor.mtk_mdm_scomo");
    public static final boolean MTK_MDM_FUMO = getValue("ro.vendor.mtk_mdm_fumo");
    public static final boolean MTK_A1_FEATURE = getValue("ro.vendor.mtk_a1_feature");
    public static final boolean MTK_GMO_RAM_OPTIMIZE = getValue("ro.vendor.gmo.ram_optimize");

    public static final boolean MTK_WAPI_SUPPORT = getValue("ro.vendor.mtk_wapi_support");

    public static final boolean MTK_BESLOUDNESS_SUPPORT = getValue("ro.vendor.mtk_besloudness_support");
    public static final boolean MTK_ANC_SUPPORT = getValue("ro.vendor.mtk_active_noise_cancel");
    public static final boolean MTK_HIFI_AUDIO_SUPPORT = getValue("ro.vendor.mtk_hifiaudio_support");

    public static final boolean MTK_DRM_APP = getValue("ro.vendor.mtk_oma_drm_support");

  // Important!!!  the SystemProperties key's length must less than 31 , or will have JE
      /* get the key's value */
    private static boolean getValue(String key) {
        return SystemProperties.get(key).equals("1");
    }
}
