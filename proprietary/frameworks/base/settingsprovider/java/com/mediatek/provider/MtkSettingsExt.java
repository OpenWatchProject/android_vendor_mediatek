package com.mediatek.provider;

import android.content.ContentResolver;
import android.provider.Settings;
import android.util.Log;
import android.annotation.ProductApi;

import java.util.HashSet;
import java.util.Set;

public final class MtkSettingsExt {
    private static final String TAG = "MtkSettingsProviderExt";

    // MTK System Settings
    public static final class System {
        /**
         * IMPORTANT: If you add a new public settings you also have to add it
         * to PUBLIC_SETTINGS below. If the new setting is hidden you have to
         * add it to PRIVATE_SETTINGS below.
         */

        /**
         * Multi SIM mode setting.
         * @hide
         */
        public static final String MSIM_MODE_SETTING = Global.MSIM_MODE_SETTING;

        /**
         * @hide
         */
        public static final String AUTO_TIME_GPS = Global.AUTO_TIME_GPS;

        /**
         * restore this vaule for MTK shut animation
         *
         * @hide
         */
        public static final String ACCELEROMETER_ROTATION_RESTORE
            = "accelerometer_rotation_restore";

        /**
         * package name for background power saving
         * alarm use this to identify feature on/off
         * @hide
         */
        public static final String BG_POWER_SAVING_ENABLE = "background_power_saving_enable";

        /**
         * M: HDMI enabled or not
         * @hide
         */
        public static final String HDMI_ENABLE_STATUS = "hdmi_enable_status";

        /**
         * M: HDMI video resolution config
         * @hide
         */
        public static final String HDMI_VIDEO_RESOLUTION = "hdmi_video_resolution";

        /**
         * M: HDMI video scale config
         * @hide
         */
        public static final String HDMI_VIDEO_SCALE = "hdmi_video_scale";

        /**
         * M: HDMI cable plugged flag
         * @hide
         */
        public static final String HDMI_CABLE_PLUGGED = "hdmi_cable_plugged";

        /**
         * M: HDMI audio output mode
         * @hide
         */
        public static final String HDMI_AUDIO_OUTPUT_MODE = "hdmi_audio_output_mode";

        /**
         * Enable or disable hotspot auto disable function
         * @hide
         */
        public static final String WIFI_HOTSPOT_AUTO_DISABLE = "wifi_hotspot_auto_disable";

        /**
         * Disable hotspot when idle for 5 mins
         * @hide
         */
        public static final int WIFI_HOTSPOT_AUTO_DISABLE_FOR_FIVE_MINS = 1;

        /**
         * The max client num of hotspot
         * @hide
         */
        public static final String WIFI_HOTSPOT_MAX_CLIENT_NUM = "wifi_hotspot_max_client_num";

        /**
         * Whether allow all devices to connect to hotspot
         * @hide
         */
        public static final String WIFI_HOTSPOT_IS_ALL_DEVICES_ALLOWED =
            "wifi_hotspot_is_all_devices_allowed";

        /**
         * Whether wpa_supplicant will connect to access point automatically
         * @hide
         */
        public static final String WIFI_CONNECT_TYPE = "wifi_connect_type";

        /**
         * wpa_supplicant will connect to access point automatically
         * @hide
         */
        public static final int WIFI_CONNECT_TYPE_AUTO = 0;

        /**
         * user need to connect to access point manually
         * @hide
         */
        public static final int WIFI_CONNECT_TYPE_MANUL = 1;

        /**
         * user will be notified when wifi is available
         * @hide
         */
        public static final int WIFI_CONNECT_TYPE_ASK = 2;

        /**
         * Whether wifi settings will connect to access point automatically
         * @hide
         */
        public static final String WIFI_CONNECT_AP_TYPE = "wifi_ap_connect_type";

        /**
         * wifi settings will connect to access point automatically
         * @hide
         */
        public static final int WIFI_CONNECT_AP_TYPE_AUTO = 0;

        /**
         * Whether connect to access point need reminder
         * @hide
         */
        public static final String WIFI_CONNECT_REMINDER = "wifi_connect_reminder";

        /**
         * The type of how each AP accessing its priority values
         * @hide
         */
        public static final String WIFI_PRIORITY_TYPE = "wifi_priority_type";

        /**
         * The type of how each AP accessing its priority
         * @hide
         */
        public static final int WIFI_PRIORITY_TYPE_DEFAULT = 0;

        /**
         * The type of how each AP accessing its priority
         * @hide
         */
        public static final int WIFI_PRIORITY_TYPE_MAMUAL = 1;

        /**
         * Whether select an access point to connect among SSID automatically
         * @hide
         */
        public static final String WIFI_SELECT_SSID_TYPE = "wifi_select_ssid_type";

        /**
         * select an access point to connect automatically
         * @hide
         */
        public static final int WIFI_SELECT_SSID_AUTO = 0;

        /**
         * M: the last GPRS SIM ID before turned off due to WIFI disconnected for OP01
         * @hide
         */
         public static final String LAST_SIMID_BEFORE_WIFI_DISCONNECTED =
                "last_simid_before_wifi_disconnected";

        /**
         * Voice call setting as Internet call
         * @hide
         */
        public static final long VOICE_CALL_SIM_SETTING_INTERNET = -2;

        /**
         * Voice call and sms setting as always ask
         * @hide
         */
        public static final long DEFAULT_SIM_SETTING_ALWAYS_ASK = -2;

        /**
         * sms setting as auto select
         * @hide
         */
        public static final long SMS_SIM_SETTING_AUTO = -3;

        /**
         * Default SIM not set
         * @hide
         */
        public static final long DEFAULT_SIM_NOT_SET = -5;

        /**
         * SMS default siM
         * @hide
         */
        public static final String SMS_SIM_SETTING = "sms_sim_setting";

        /**
          * M: Voice wake up base command string.
          * @hide
          */
         @ProductApi
         public static String BASE_VOICE_WAKEUP_COMMAND_KEY = "voice_wakeup_app";

         /**
          * M: Set Voice command value.
          * @hide
          */
         public static void setVoiceCommandValue(ContentResolver cr, String baseCommand,
                 int commandId, String launchApp) {
             Settings.System.putString(cr,
                     baseCommand + commandId, launchApp);
         }
         /**
          * M: Get Voice command value.
          * @hide
          */
         @ProductApi
         public static String getVoiceCommandValue(ContentResolver cr, String baseCommand,
                 int commandId) {
             return Settings.System.getString(cr, baseCommand + commandId);
         }

         /**
          * M: Voice wake up mode.
          * 0: voice unlock
          * 1: voice wakeup do not recognize people mode
          * 2: voice wakeup recognize people mode
          *
          * @hide
          */
         public static String VOICE_WAKEUP_MODE = "voice_wakeup_mode";

         /**
          * M: Voice wake up command status
          * 0: no voice wake up command and unchecked
          * 1: has voice wake up command and unchecked
          * 2: has voice wake up command and checked
          *
          * @hide
          */
         public static String VOICE_WAKEUP_COMMAND_STATUS = "voice_wakeup_command_status";

        /**
         * M: Voice wake up command that is currently selected for dynamic keyphrase.
         * bitmask map to command id i.e.
         * 1: active command id is 0
         * 2: active command id is 1
         * 4: active command id is 2
         * @hide
         */
        public static String VOICE_WAKEUP_ACTIVE_COMMAND_ID = "voice_wakeup_active_command_id";

         /**
          * M: Voice trigger mode.
          * 0: voice unlock
          * 1: voice wakeup do not recognize people mode
          * 2: voice wakeup recognize people mode
          * 3: Voice trigger mode
          *
          * @hide
          */
         public static String VOICE_TRIGGER_MODE = "voice_trigger_mode";

         /**
          * M: Voice trigger command status
          * 0: no voice trigger command and unchecked
          * 1: has voice trigger command and unchecked
          * 2: has voice trigger command and checked
          *
          * @hide
          */
         public static String VOICE_TRIGGER_COMMAND_STATUS = "voice_trigger_command_status";

        /**
         * M: Voice trigger command that is currently selected for dynamic keyphrase.
         * bitmask map to command id i.e.
         * 1: active command id is 0
         * 2: active command id is 1
         * 4: active command id is 2
         * @hide
         */
        public static String VOICE_TRIGGER_ACTIVE_COMMAND_ID = "voice_trigger_active_command_id";

        /**
         * M: CTM Requirements
         *    VOLTE status for slot 0
         * @hide
         */
        public static final String VOLTE_DMYK_STATE_0 = "volte_dmyk_state_0";

        /**
         * M: CTM Requirements
         *    VOLTE status for slot 1
         * @hide
         */
        public static final String VOLTE_DMYK_STATE_1 = "volte_dmyk_state_1";

        // Add new settings to PUBLIC_SETTINGS
        public static void putInPublicSettings(Set<String> PUBLIC_SETTINGS) {
            Log.d(TAG, "putInPublicSettings");
            PUBLIC_SETTINGS.add(MSIM_MODE_SETTING);
            PUBLIC_SETTINGS.add(AUTO_TIME_GPS);
            PUBLIC_SETTINGS.add(ACCELEROMETER_ROTATION_RESTORE);
            PUBLIC_SETTINGS.add(BG_POWER_SAVING_ENABLE);
            PUBLIC_SETTINGS.add(HDMI_ENABLE_STATUS);
            PUBLIC_SETTINGS.add(HDMI_VIDEO_RESOLUTION);
            PUBLIC_SETTINGS.add(HDMI_VIDEO_SCALE);
            PUBLIC_SETTINGS.add(HDMI_CABLE_PLUGGED);
            PUBLIC_SETTINGS.add(HDMI_AUDIO_OUTPUT_MODE);
            PUBLIC_SETTINGS.add(WIFI_CONNECT_TYPE);
            PUBLIC_SETTINGS.add(WIFI_CONNECT_AP_TYPE);
            PUBLIC_SETTINGS.add(WIFI_CONNECT_REMINDER);
            PUBLIC_SETTINGS.add(WIFI_PRIORITY_TYPE);
            PUBLIC_SETTINGS.add(WIFI_SELECT_SSID_TYPE);
            PUBLIC_SETTINGS.add(LAST_SIMID_BEFORE_WIFI_DISCONNECTED);
            // CTM
            PUBLIC_SETTINGS.add(VOLTE_DMYK_STATE_0);
            PUBLIC_SETTINGS.add(VOLTE_DMYK_STATE_1);
            PUBLIC_SETTINGS.add(BASE_VOICE_WAKEUP_COMMAND_KEY);
            PUBLIC_SETTINGS.add(VOICE_WAKEUP_MODE);
            PUBLIC_SETTINGS.add(VOICE_WAKEUP_COMMAND_STATUS);
            PUBLIC_SETTINGS.add(VOICE_WAKEUP_ACTIVE_COMMAND_ID);
            PUBLIC_SETTINGS.add(VOICE_TRIGGER_MODE);
            PUBLIC_SETTINGS.add(VOICE_TRIGGER_COMMAND_STATUS);
            PUBLIC_SETTINGS.add(VOICE_TRIGGER_ACTIVE_COMMAND_ID);
        }

        // At one time in System, then in Global
        public static void moveToGlobal(HashSet<String> MOVED_TO_GLOBAL) {
            Log.d(TAG, "System moveToGlobal");
            MOVED_TO_GLOBAL.add(MtkSettingsExt.Global.MSIM_MODE_SETTING);
            MOVED_TO_GLOBAL.add(MtkSettingsExt.Global.AUTO_TIME_GPS);
        }

        // At one time in System, then in Secure
        public static void moveToSecure(HashSet<String> MOVED_TO_SECURE) {
            Log.d(TAG, "System moveToSecure");
        }
    }

    // MTK Secure Settings
    public static final class Secure {

        // At one time in Secure, then in Global
        public static void moveToGlobal(HashSet<String> MOVED_TO_GLOBAL) {
            Log.d(TAG, "Secure moveToGlobal");
        }
    }

    // MTK Global Settings
    public static final class Global {

        /**
         * Multi SIM mode setting.
         * @hide
         */
        public static final String MSIM_MODE_SETTING = "msim_mode_setting";

        /**
         * Whether NFC HCE is enabled/disabled 0=disabled. 1=enabled.
         * @hide
         */
        public static final String NFC_HCE_ON = "nfc_hce_on";

        /**
         * Whether NFC Multi-SE is enabled/disabled
         * @hide
         */
        public static final String NFC_MULTISE_ON = "nfc_multise_on";

        /**
         * NFC Multi-SE list String separated by ",", for example,
         * "SIM1,SIM2,OFF"
         * @hide
         */
        public static final String NFC_MULTISE_LIST = "nfc_multise_list";

        /**
         * Active NFC Multi-SE String contained in Multi-SE list, for example,
         * "SIM1" or "OFF"
         * @hide
         */
        public static final String NFC_MULTISE_ACTIVE = "nfc_multise_active";

        /**
         * Previous NFC Multi-SE String contained in Multi-SE list, for example,
         * "SIM1"
         * @hide
         */
        public static final String NFC_MULTISE_PREVIOUS = "nfc_multise_previous";

        /**
         * Whether NFC SEAPI is at transaction state
         * @hide
         */
        public static final String NFC_MULTISE_IN_TRANSACTION = "nfc_multise_in_transation";

        /**
         * Whether NFC Multi-SE is at switching state
         * @hide
         */
        public static final String NFC_MULTISE_IN_SWITCHING = "nfc_multise_in_switching";

        /**
         * Whether NFC RF FIELD is actived/deactived 0=deactived. 1=deactived.
         * @hide
         */
        public static final String NFC_RF_FIELD_ACTIVE = "nfc_rf_field_active";

        /**
         * Whether time and time zone automatically fetched from GPS
         * @hide
         */
        public static final String AUTO_TIME_GPS = "auto_time_gps";

        /**
          * Wifi display display toast time
          *
          * @hide
          */
        public static final String WIFI_DISPLAY_DISPLAY_TOAST_TIME =
                "wifi_display_display_toast_time";

        /**
          * Wifi display display notification time.
          *
          * @hide
          */
        public static final String WIFI_DISPLAY_DISPLAY_NOTIFICATION_TIME =
                "wifi_display_notification_time";

        /**
          * Wifi display quality enhancement on.
          *
          * @hide
          */
        public static final String WIFI_DISPLAY_QE_ON = "wifi_display_qe_on";

        /**
          * Wifi display display SQC info on
          *
          * @hide
          */
        public static final String WIFI_DISPLAY_SQC_INFO_ON = "wifi_display_sqc_info_on";

        /**
         * Whether enable WFD auto channel selection.
         *
         * @hide
         */
        public static final String WIFI_DISPLAY_AUTO_CHANNEL_SELECTION =
                "wifi_display_auto_channel_selection";

        /**
         * Wifi display max resolution
         * The value is an integer.
         * 0: 720p,30fps  (Menu is disabled)
         * 1: 1080p,30fps (Menu is disabled)
         * 2: 1080p,30fps (Menu is enabled)
         * 3: 720p,30fps (Menu is enabled)
         * @hide
         */
        public static final String WIFI_DISPLAY_RESOLUTION = "wifi_display_max_resolution";

        /**
         * Whether enable WFD change resolution remind idalog
         *
         * @hide
         */
        public static final String WIFI_DISPLAY_RESOLUTION_DONOT_REMIND =
            "wifi_display_change_resolution_remind";

        /**
         * Whether show WFD audio output path do not remind again.
         *
         * @hide
         */
        public static final String WIFI_DISPLAY_SOUND_PATH_DONOT_REMIND =
                "wifi_display_sound_path_do_not_remind";

        /**
         * Wifi Display power saving option.
         * 0: off, 1: dim, 2: dim and show image, 3: no power saving
         * @hide
         */
        public static final String WIFI_DISPLAY_POWER_SAVING_OPTION =
                "wifi_display_power_saving_option";

        /**
         * Wifi Display power saving delay.
         * The unit is second
         * @hide
         */
        public static final String WIFI_DISPLAY_POWER_SAVING_DELAY =
                "wifi_display_power_saving_delay";

        /**
         * Wifi Display security option
         * 0: TV shows black. 1: TV shows prohibitted image
         * @hide
         */
        public static final String WIFI_DISPLAY_SECURITY_OPTION = "wifi_display_security_option";

        /**
         * Enable Wifi Display latency profiling panel.
         *
         * @hide
         */
        public static final String WIFI_DISPLAY_LATENCY_PROFILING =
                "wifi_display_latency_profiling";

        /**
         * Wifi Display chosen capability,"%s,%s,%s" means (Audio, video, resolution/frame rate).
         *
         * @hide
         */
        public static final String WIFI_DISPLAY_CHOSEN_CAPABILITY =
                "wifi_display_chosen_capability";

        /**
         * Wifi Display WIFI info, "%d,%d,%d,%d" means (channdlID, AP num, Score, Data rate).
         *
         * @hide
         */
        public static final String WIFI_DISPLAY_WIFI_INFO = "wifi_display_wifi_info";

        /**
         * Wifi Display WFD Latency, "%d,%d,%d" means(avg latency, max latency, timestamp).
         *
         * @hide
         */
        public static final String WIFI_DISPLAY_WFD_LATENCY = "wifi_display_wfd_latency";

        /**
         * Wifi Display portrait resolution support
         * 0: Not support. 1: Support
         * @hide
         */
        public static final String WIFI_DISPLAY_PORTRAIT_RESOLUTION =
                "wifi_display_portrait_resolution";

        /**
         * Whether primary sim setting is enabled.
         *
         * @hide
         */
        public static final String PRIMARY_SIM = "primary_sim";

        /**
         * Whether current network setting is enabled for SMS.
         *
         * @hide
         */
        public static final String CURRENT_NETWORK_SMS = "current_network_sms";

        /**
         * Whether current network setting is enabled for call.
         *
         * @hide
         */
        public static final String CURRENT_NETWORK_CALL = "current_network_call";


        /**
         * Monitor setting for voice and text during international roaming.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String INTERNATIONAL_VOICE_TEXT_ROAMING =
                "international_voice_text_roaming";

        /**
         * Monitor setting for voice and text during domestic roaming.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String DOMESTIC_VOICE_TEXT_ROAMING = "domestic_voice_text_roaming";

        /**
         * Monitor setting for data during international roaming.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String INTERNATIONAL_DATA_ROAMING = "international_data_roaming";

        /**
         * Monitor setting for data during domestic roaming.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String DOMESTIC_DATA_ROAMING = "domestic_data_roaming";

        /**
         * Monitor setting for LTE data during domestic roaming.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String DOMESTIC_LTE_DATA_ROAMING = "domestic_data_roaming";

        /**
         * Guard setting for voice during international roaming guard.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String INTERNATIONAL_VOICE_ROAMING_GUARD =
                "international_voice_roaming_guard";

        /**
         * Guard setting for voice and text during international roaming guard.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String INTERNATIONAL_TEXT_ROAMING_GUARD =
                "international_text_roaming_guard";

        /**
         * Guard setting for voice and text during domestic roaming guard.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String DOMESTIC_VOICE_TEXT_ROAMING_GUARD =
                "domestic_voice_text_roaming_guard";

        /**
         * Guard setting for data during international roaming guard.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String INTERNATIONAL_DATA_ROAMING_GUARD =
                "international_data_roaming_guard";

        /**
         * Guard setting for data during domestic roaming guard.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String DOMESTIC_DATA_ROAMING_GUARD = "domestic_data_roaming_guard";

        /**
         * For tester to switch some features via engineering mode
         *
         * @hide
         */
        public static final String TELEPHONY_MISC_FEATURE_CONFIG =
                "telephony_misc_feature_config";

        /**
         * Telecom RTT(Real time text) audio mode settings.
         * (0 = Normal voice, 1 = Reduced voice)
         * @hide
         */
        public static final String TELECOM_RTT_AUDIO_MODE =
                "telecom_rtt_adudio_mode";

        /**
         * Whether data service setting is enabled for VoLTE.
         * (0 = false, 1 = true)
         * @hide
         */
        public static final String DATA_SERVICE_ENABLED = "data_service_enabled";

        // At one time in Global, then in Secure
        public static void moveToSecure(HashSet<String> MOVED_TO_SECURE) {
            Log.d(TAG, "Global moveToSecure");
        }
    }
}