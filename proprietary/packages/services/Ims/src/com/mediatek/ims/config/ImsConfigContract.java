package com.mediatek.ims.config;

import android.content.ContentUris;
import android.net.Uri;
import android.provider.BaseColumns;
import android.telephony.TelephonyManager;

import com.android.ims.ImsConfig;
import com.mediatek.ims.MtkImsConstants;

/**
 * The contract class for application accessing ImsConfigProvider, and providing class Validator to
 * check the validity for any configurable parameter / field / identity.
 */
public class ImsConfigContract {
    public static final String ACTION_CONFIG_UPDATE =
            "com.mediatek.ims.config.action.CONFIG_UPDATE";
    public static final String ACTION_CONFIG_LOADED =
            "com.mediatek.ims.config.action.CONFIG_LOADED";
    public static final String ACTION_DYNAMIC_IMS_SWITCH_TRIGGER =
            "com.mediatek.ims.config.action.DYNAMIC_IMS_SWITCH_TRIGGER";
    public static final String ACTION_DYNAMIC_IMS_SWITCH_COMPLETE =
            "com.mediatek.ims.config.action.DYNAMIC_IMS_SWITCH_COMPLETE";

    public static final String EXTRA_PHONE_ID = "phone_id";
    public static final String EXTRA_CONFIG_ID = "config_id";

    // Same AOSP define in ImsConfig.java
    /**
     * Broadcast action: the feature enable status was changed
     *
     * @hide
     */
    public static final String ACTION_IMS_FEATURE_CHANGED =
            "com.android.intent.action.IMS_FEATURE_CHANGED";

    /**
     * Broadcast action: the configuration was changed
     *
     * @hide
     */
    public static final String ACTION_IMS_CONFIG_CHANGED =
            "com.android.intent.action.IMS_CONFIG_CHANGED";

    /**
     * Extra parameter "item" of intent ACTION_IMS_FEATURE_CHANGED and ACTION_IMS_CONFIG_CHANGED.
     * It is the value of FeatureConstants or ConfigConstants.
     *
     * @hide
     */
    public static final String EXTRA_CHANGED_ITEM = "item";

    /**
     * Extra parameter "value" of intent ACTION_IMS_FEATURE_CHANGED and ACTION_IMS_CONFIG_CHANGED.
     * It is the new value of "item".
     *
     * @hide
     */
    public static final String EXTRA_NEW_VALUE = "value";

    /**
     * Extra parameter "mimetype" of intent ACTION_IMS_CONFIG_CHANGED.
     * It is the value of MimeType (to indicate is INTEGER or STRING).
     *
     * @hide
     */
    // MTK defined EXTRA
    public static final String EXTRA_MIMETYPE = "mimetype";

    public static final String AUTHORITY = "com.mediatek.ims.config.provider";
    public static final Uri CONTENT_URI = Uri.parse("content://" + AUTHORITY);

    public static final String VALUE_NO_DEFAULT = "n/a";

    public static final String TABLE_FEATURE = "tb_feature";
    public static final String TABLE_DEFAULT = "tb_default";
    public static final String TABLE_PROVISION = "tb_provision";
    public static final String TABLE_MASTER = "tb_master";
    public static final String TABLE_CONFIG_SETTING = "tb_config_setting";
    public static final String TABLE_RESOURCE = "tb_resource";

    public static class Operator {
        public static final int OP_NONE = -1;
        public static final int OP_DEFAULT = 0;
        public static final int OP_06 = 6;
        public static final int OP_08 = 8;
        public static final int OP_12 = 12;
    }

    public interface MimeType {
        int INTEGER = 0;
        int STRING = 1;
        int FLOAT = 2;
        int JSON = 3;
    }

    public interface Unit {
        int UNIT_NONE = -1;
        int NANOSECONDS = 0;
        int MICROSECONDS = 1;
        int MILLISECONDS = 2;
        int SECONDS = 3;
        int MINUTES = 4;
        int HOURS = 5;
        int DAYS = 6;
    }

    private static String[] sConfigNames = new String[] {
            "VOCODER_AMRMODESET",
            "VOCODER_AMRWBMODESET",
            "SIP_SESSION_TIMER",
            "MIN_SE",
            "CANCELLATION_TIMER",
            "TDELAY",
            "SILENT_REDIAL_ENABLE",
            "SIP_T1_TIMER",
            "SIP_T2_TIMER",
            "SIP_TF_TIMER",
            "VLT_SETTING_ENABLED",
            "LVC_SETTING_ENABLED",
            "DOMAIN_NAME",
            "SMS_FORMAT",
            "SMS_OVER_IP",
            "PUBLISH_TIMER",
            "PUBLISH_TIMER_EXTENDED",
            "CAPABILITY_DISCOVERY_ENABLED",
            "CAPABILITIES_CACHE_EXPIRATION",
            "AVAILABILITY_CACHE_EXPIRATION",
            "CAPABILITIES_POLL_INTERVAL",
            "SOURCE_THROTTLE_PUBLISH",
            "MAX_NUMENTRIES_IN_RCL",
            "CAPAB_POLL_LIST_SUB_EXP",
            "GZIP_FLAG",
            "EAB_SETTING_ENABLED",
            "VOICE_OVER_WIFI_ROAMING",
            "VOICE_OVER_WIFI_MODE",
            "VOICE_OVER_WIFI_SETTING_ENABLED",
            "MOBILE_DATA_ENABLED",
            "VOLTE_USER_OPT_IN_STATUS",
            "LBO_PCSCF_ADDRESS",
            "KEEP_ALIVE_ENABLED",
            "REGISTRATION_RETRY_BASE_TIME_SEC",
            "REGISTRATION_RETRY_MAX_TIME_SEC",
            "SPEECH_START_PORT",
            "SPEECH_END_PORT",
            "SIP_INVITE_REQ_RETX_INTERVAL_MSEC",
            "SIP_INVITE_RSP_WAIT_TIME_MSEC",
            "SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC",
            "SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC",
            "SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC",
            "SIP_INVITE_RSP_RETX_INTERVAL_MSEC",
            "SIP_ACK_RECEIPT_WAIT_TIME_MSEC",
            "SIP_ACK_RETX_WAIT_TIME_MSEC",
            "SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC",
            "SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC",
            "AMR_WB_OCTET_ALIGNED_PT",
            "AMR_WB_BANDWIDTH_EFFICIENT_PT",
            "AMR_OCTET_ALIGNED_PT",
            "AMR_BANDWIDTH_EFFICIENT_PT",
            "DTMF_WB_PT",
            "DTMF_NB_PT",
            "AMR_DEFAULT_MODE",
            "SMS_PSI",
            "VIDEO_QUALITY",
            "TH_LTE1",
            "TH_LTE2",
            "TH_LTE3",
            "TH_1x",
            "VOWT_A",
            "VOWT_B",
            "T_EPDG_LTE",
            "T_EPDG_WIFI",
            "T_EPDG_1X",
            "VICE_SETTING_ENABLED",
            "RTT_SETTING_ENABLED"
    };

    private static String[] sMtkConfigNames = new String[] {
            "EPDG_ADDRESS",
            "PUBLISH_ERROR_RETRY_TIMER",
            "VOICE_OVER_WIFI_MDN",
            "VOICE_DOMAIN_PREFERENCE"
    };

    public static class ConfigSetting implements BaseColumns {
        public static final Uri CONTENT_URI =
                Uri.parse("content://" + AUTHORITY + "/" + TABLE_CONFIG_SETTING);
        public static final String PHONE_ID = "phone_id";
        public static final String SETTING_ID = "setting_id";
        public static final String VALUE = "value";

        public static final int SETTING_ID_OPCODE = 0;

        public static Uri getUriWithSettingId(int phoneId, int settingId) {
            Uri result = ContentUris.withAppendedId(CONTENT_URI, phoneId);
            return ContentUris.withAppendedId(result, settingId);
        }
    }

    public static class Feature implements BaseColumns {
        public static final Uri CONTENT_URI =
                Uri.parse("content://" + AUTHORITY + "/" + TABLE_FEATURE);
        public static final String PHONE_ID = "phone_id";
        public static final String FEATURE_ID = "feature_id";
        public static final String NETWORK_ID = "network_id";
        public static final String VALUE = "value";

        public static Uri getUriWithFeatureId(int phoneId, int featureId, int network) {
            Uri result = ContentUris.withAppendedId(CONTENT_URI, phoneId);
            result = ContentUris.withAppendedId(result, featureId);
            return ContentUris.withAppendedId(result, network);
        }
    }

    public static class Resource implements BaseColumns {
        public static final Uri CONTENT_URI =
                Uri.parse("content://" + AUTHORITY + "/" + TABLE_RESOURCE);
        public static final String PHONE_ID = "phone_id";
        public static final String FEATURE_ID = "feature_id";
        public static final String VALUE = "value";

        public static Uri getUriWithFeatureId(int phoneId, int featureId) {
            Uri result = ContentUris.withAppendedId(CONTENT_URI, phoneId);
            return ContentUris.withAppendedId(result, featureId);
        }
    }

    public static class Provision extends BasicConfigTable {
        public static final Uri CONTENT_URI =
                Uri.parse("content://" + AUTHORITY + "/" + TABLE_PROVISION);
        public static final String DATETIME = "datetime";
        public static Uri getUriWithConfigId(int phoneId, int configId) {
            Uri result = ContentUris.withAppendedId(CONTENT_URI, phoneId);
            return Uri.withAppendedPath(result, ImsConfigContract.configIdToName(configId));
        }
    }

    public static class Default extends BasicConfigTable {
        public static final Uri CONTENT_URI =
                Uri.parse("content://" + AUTHORITY + "/" + TABLE_DEFAULT);
        public static final String UNIT_ID = "unit_id";
        public static Uri getUriWithConfigId(int phoneId, int configId) {
            Uri result = ContentUris.withAppendedId(CONTENT_URI, phoneId);
            return Uri.withAppendedPath(result, ImsConfigContract.configIdToName(configId));
        }
    }

    public static class Master extends BasicConfigTable {
        public static final Uri CONTENT_URI =
                Uri.parse("content://" + AUTHORITY + "/" + TABLE_MASTER);
        public static Uri getUriWithConfigId(int phoneId, int configId) {
            Uri result = ContentUris.withAppendedId(CONTENT_URI, phoneId);
            return Uri.withAppendedPath(result, ImsConfigContract.configIdToName(configId));
        }
    }

    public static abstract class BasicConfigTable implements BaseColumns {
        public static final String PHONE_ID = "phone_id";
        public static final String CONFIG_ID = "config_id";
        public static final String MIMETYPE_ID = "mimetype_id";
        public static final String DATA = "data";
    }

    public static Uri getTableUri(String table) {
        Uri result = null;
        if (!Validator.isValidTable(table)) {
            throw new IllegalArgumentException("Invalid table: " + table);
        }
        switch (table) {
            case ImsConfigContract.TABLE_CONFIG_SETTING:
                result = ConfigSetting.CONTENT_URI;
                break;
            case ImsConfigContract.TABLE_DEFAULT:
                result = Default.CONTENT_URI;
                break;
            case ImsConfigContract.TABLE_PROVISION:
                result = Provision.CONTENT_URI;
                break;
            case ImsConfigContract.TABLE_MASTER:
                result = Master.CONTENT_URI;
                break;
            default:
                // do nothing
                break;
        }
        return result;
    }

    public static Uri getConfigUri(String table, int phoneId, int itemId) {
        Uri result = null;
        if (!Validator.isValidTable(table)) {
            throw new IllegalArgumentException("Invalid table: " + table);
        }
        switch (table) {
            case ImsConfigContract.TABLE_CONFIG_SETTING:
                result = ImsConfigContract.ConfigSetting.getUriWithSettingId(phoneId, itemId);
                break;
            case ImsConfigContract.TABLE_DEFAULT:
                result = ImsConfigContract.Default.getUriWithConfigId(phoneId, itemId);
                break;
            case ImsConfigContract.TABLE_PROVISION:
                result = ImsConfigContract.Provision.getUriWithConfigId(phoneId, itemId);
                break;
            case ImsConfigContract.TABLE_MASTER:
                result = ImsConfigContract.Master.getUriWithConfigId(phoneId, itemId);
                break;
            default:
                // do nothing
                break;
        }
        return result;
    }

    public static String configIdToName(int configId) {
        if (configId >= MtkImsConstants.ConfigConstants.PROVISIONED_CONFIG_START &&
            configId <= MtkImsConstants.ConfigConstants.PROVISIONED_CONFIG_END) {
            return sMtkConfigNames[configId-MtkImsConstants.MTK_CONFIG_START];
        } else if (configId >= sConfigNames.length) {
            throw new IllegalArgumentException("Invalid config id: " + configId);
        }
        return sConfigNames[configId];
    }

    public static int configNameToId(String configName) {
        for (int i = 0; i < sConfigNames.length; i++) {
            if (sConfigNames[i].equals(configName)) {
                return i;
            }
        }
        for (int i = 0; i < sMtkConfigNames.length; i++) {
            if (sMtkConfigNames[i].equals(configName)) {
                return i+MtkImsConstants.MTK_CONFIG_START;
            }
        }
        throw new IllegalArgumentException("Unknown config: " + configName);
    }

    public static class Validator {
        public static boolean isValidTable(String table) {
            boolean valid = true;
            switch(table) {
                case TABLE_FEATURE:
                case TABLE_DEFAULT:
                case TABLE_CONFIG_SETTING:
                case TABLE_PROVISION:
                case TABLE_MASTER:
                case TABLE_RESOURCE:
                    break;
                default:
                    valid = false;
                    break;
            }
            return valid;
        }

        public static boolean isValidSettingId(int settingId) {
            boolean valid = true;
            switch (settingId) {
                case ConfigSetting.SETTING_ID_OPCODE:
                    break;
                default:
                    valid = false;
                    break;
            }
            return valid;
        }

        public static boolean isValidFeatureId(int featureId) {
            boolean valid = true;
            switch(featureId) {
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE:
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI:
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN:
                default:
                    valid = false;
                    break;
            }
            return valid;
        }

        public static boolean isValidFeatureValue(int featureVal) {
            boolean valid = true;
            switch(featureVal) {
                case ImsConfig.FeatureValueConstants.ON:
                case ImsConfig.FeatureValueConstants.OFF:
                    break;
                default:
                    valid = false;
                    break;
            }
            return valid;
        }

        public static boolean isValidNetwork(int network) {
            return (network != TelephonyManager.NETWORK_TYPE_UNKNOWN) ? true : false;
        }

        public static boolean isValidConfigId(int configId) {
            boolean valid = true;

            if (configId < ImsConfig.ConfigConstants.PROVISIONED_CONFIG_START) {
                valid = false;
            } else if (configId > MtkImsConstants.ConfigConstants.PROVISIONED_CONFIG_END) {
                valid = false;
            } else if (configId > ImsConfig.ConfigConstants.PROVISIONED_CONFIG_END &&
                       configId < MtkImsConstants.ConfigConstants.PROVISIONED_CONFIG_START) {
                valid = false;
            }
            return valid;
        }

        public static boolean isValidMimeTypeId(int mimeTypeId) {
            boolean valid = true;
            switch (mimeTypeId) {
                case MimeType.INTEGER:
                case MimeType.STRING:
                case MimeType.JSON:
                case MimeType.FLOAT:
                    break;
                default:
                    valid = false;
                    break;
            }
            return valid;
        }

        public static boolean isValidUnitId(int unitId) {
            boolean valid = true;
            switch (unitId) {
                case Unit.UNIT_NONE:
                case Unit.NANOSECONDS:
                case Unit.MICROSECONDS:
                case Unit.MILLISECONDS:
                case Unit.SECONDS:
                case Unit.MINUTES:
                case Unit.HOURS:
                case Unit.DAYS:
                    break;
                default:
                    valid = false;
                    break;
            }
            return valid;
        }
    }

    public static int getNetworkTypeByFeature(int imsFeatureType) {
        switch (imsFeatureType) {
            case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE:
                return TelephonyManager.NETWORK_TYPE_LTE;
            case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE:
                return TelephonyManager.NETWORK_TYPE_LTE;
            case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                return TelephonyManager.NETWORK_TYPE_IWLAN;
            default:
                return TelephonyManager.NETWORK_TYPE_UNKNOWN;
        }
    }
}
