package com.mediatek.ims.config;

import com.android.ims.ImsConfig;

import java.util.HashMap;
import java.util.Map;

import com.mediatek.ims.MtkImsConstants;

/**
 * Class mapping setting strings in carrier configuration xml to ImsConfig item,
 * and mapping IMS configuration to class type, inorder to recover string data
 * back to its original class.
 */
public class ImsConfigSettings {
    private static HashMap<Integer, Setting> sImsConfigurations =
            new HashMap<Integer, Setting>();

    static {
        buildConfigSettings();
    }

    private static void buildConfigSettings() {
        sImsConfigurations.put(ImsConfig.ConfigConstants.VOCODER_AMRMODESET,
                new Setting(String.class, ImsConfigContract.MimeType.STRING));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VOCODER_AMRWBMODESET,
                new Setting(String.class, ImsConfigContract.MimeType.STRING));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_SESSION_TIMER,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.MIN_SE,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.CANCELLATION_TIMER,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.TDELAY,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SILENT_REDIAL_ENABLE,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_T1_TIMER,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_T2_TIMER,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_TF_TIMER,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VLT_SETTING_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.LVC_SETTING_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER, "video_over_ps_support"));
        sImsConfigurations.put(ImsConfig.ConfigConstants.DOMAIN_NAME,
                new Setting(String.class, ImsConfigContract.MimeType.STRING));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SMS_FORMAT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SMS_OVER_IP,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.PUBLISH_TIMER,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.CAPABILITY_DISCOVERY_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.PUBLISH_TIMER_EXTENDED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.CAPABILITIES_CACHE_EXPIRATION,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.AVAILABILITY_CACHE_EXPIRATION,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.CAPABILITIES_POLL_INTERVAL,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SOURCE_THROTTLE_PUBLISH,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.MAX_NUMENTRIES_IN_RCL,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.CAPAB_POLL_LIST_SUB_EXP,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.GZIP_FLAG,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.EAB_SETTING_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VOICE_OVER_WIFI_ROAMING,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VOICE_OVER_WIFI_MODE,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.MOBILE_DATA_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VOLTE_USER_OPT_IN_STATUS,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.LBO_PCSCF_ADDRESS,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.KEEP_ALIVE_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.REGISTRATION_RETRY_BASE_TIME_SEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.REGISTRATION_RETRY_MAX_TIME_SEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SPEECH_START_PORT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SPEECH_END_PORT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_INVITE_REQ_RETX_INTERVAL_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_INVITE_RSP_WAIT_TIME_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_INVITE_RSP_RETX_INTERVAL_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_ACK_RECEIPT_WAIT_TIME_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_ACK_RETX_WAIT_TIME_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.AMR_WB_OCTET_ALIGNED_PT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.AMR_WB_BANDWIDTH_EFFICIENT_PT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.AMR_OCTET_ALIGNED_PT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.AMR_BANDWIDTH_EFFICIENT_PT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.DTMF_WB_PT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.DTMF_NB_PT,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.AMR_DEFAULT_MODE,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.SMS_PSI,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VIDEO_QUALITY,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.TH_LTE1,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.TH_LTE2,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.TH_LTE3,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.TH_1x,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VOWT_A,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VOWT_B,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.T_EPDG_LTE,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.T_EPDG_WIFI,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.T_EPDG_1X,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.VICE_SETTING_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(ImsConfig.ConfigConstants.RTT_SETTING_ENABLED,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(MtkImsConstants.ConfigConstants.EPDG_ADDRESS,
                new Setting(String.class, ImsConfigContract.MimeType.STRING, "epdg_identifier"));
        sImsConfigurations.put(MtkImsConstants.ConfigConstants.PUBLISH_ERROR_RETRY_TIMER,
                new Setting(String.class, ImsConfigContract.MimeType.INTEGER));
        sImsConfigurations.put(MtkImsConstants.ConfigConstants.VOICE_OVER_WIFI_MDN,
                new Setting(String.class, ImsConfigContract.MimeType.STRING));
        sImsConfigurations.put(MtkImsConstants.ConfigConstants.VOICE_DOMAIN_PREFERENCE,
                new Setting(Integer.class, ImsConfigContract.MimeType.INTEGER));
    }

    public static class Setting {
        public Setting(Class _clazz, int _mimeType) {
            clazz = _clazz;
            mimeType = _mimeType;
            mIsStoreModem = false;
            mProvisionStr = "";
        }
        public Setting(Class _clazz, int _mimeType, String _provisionStr) {
            clazz = _clazz;
            mimeType = _mimeType;
            if (_provisionStr != "" && _provisionStr != null ) {
                mIsStoreModem = true;
                mProvisionStr = _provisionStr;
            }
        }
        public Class clazz;
        public int mimeType;
        public boolean mIsStoreModem;
        public String mProvisionStr;
    }

    public static Map<Integer, Setting> getConfigSettings() {
        return sImsConfigurations;
    }

    public static int getMimeType(int configId) {
        Setting s = sImsConfigurations.get(configId);
        return s.mimeType;
    }

    public static boolean getIsStoreModem(int configId) {
        Setting s = sImsConfigurations.get(configId);
        return s.mIsStoreModem;
    }

    public static String getProvisionStr(int configId) {
        Setting s = sImsConfigurations.get(configId);
        return s.mProvisionStr;
    }
}
