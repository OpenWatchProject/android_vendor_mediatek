/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.orangelabs.rcs.provider.settings;

import java.util.ArrayList;

import javax2.sip.ListeningPoint;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.net.Uri;
import android.os.Environment;
import android.os.SystemProperties;
import android.util.Log;

import com.orangelabs.rcs.R;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.service.LauncherUtils;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * RCS settings provider
 *
 * @author jexa7410
 */
public class RcsSettingsProvider extends ContentProvider {

    public static boolean mIsEncrypted = false;
    /**
     * Modified to achieve the auto configuration feature. @{
     */
    /**
     * Database table
     */
    public static final String FIRST_USER_ACCOUNT_TABLE = "settings";
    //   public static final String SECOND_USER_ACCOUNT_TABLE = "settings_2nd";
    //   public static final String THIRD_USER_ACCOUNT_TABLE = "settings_3rd";

    /**
     * Database table
     */
    private static final String TABLE = "settings";

    // Create the constants used to differentiate between the different URI requests
    private static final int SETTINGS = 1;
    private static final int SETTINGS_ID = 2;

    // Allocate the UriMatcher object, where a URI ending in 'settings'
    // will correspond to a request for all settings, and 'settings'
    // with a trailing '/[rowID]' will represent a single settings row.
    private static final UriMatcher uriMatcher = new UriMatcher(UriMatcher.NO_MATCH);

    static {
        uriMatcher.addURI("com.orangelabs.rcs.settings", "settings", SETTINGS);
        uriMatcher.addURI("com.orangelabs.rcs.settings", "settings/#", SETTINGS_ID);
    }

    /**
     * Database helper class
     */
    private SQLiteOpenHelper openHelper;

    /**
     * Database name
     */
    public static final String DATABASE_NAME = "rcs_settings.db";

    /**
     * Helper class for opening, creating and managing database version control
     */
    private static class DatabaseHelper extends SQLiteOpenHelper {
        private static final int DATABASE_VERSION = 86;

        private Context ctx;

        public DatabaseHelper(Context ctx) {
            super(ctx, DATABASE_NAME, null, DATABASE_VERSION);

            this.ctx = ctx;
        }

        @Override
        public void onCreate(SQLiteDatabase db) {
            db.execSQL("CREATE TABLE " + TABLE + " ("
                    + RcsSettingsData.KEY_ID + " integer primary key autoincrement,"
                    + RcsSettingsData.KEY_KEY + " TEXT,"
                    + RcsSettingsData.KEY_VALUE + " TEXT);");

            // Insert default values for parameters

            addParameter(db, RcsSettingsData.SERVICE_ACTIVATED, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.PRESENCE_INVITATION_RINGTONE, "");
            addParameter(db, RcsSettingsData.PRESENCE_INVITATION_VIBRATE, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CSH_INVITATION_RINGTONE, "");
            addParameter(db, RcsSettingsData.CSH_INVITATION_VIBRATE, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CSH_AVAILABLE_BEEP, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.FILETRANSFER_INVITATION_RINGTONE, "");
            addParameter(db, RcsSettingsData.FILETRANSFER_INVITATION_VIBRATE, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CHAT_INVITATION_RINGTONE, "");
            addParameter(db, RcsSettingsData.CHAT_INVITATION_VIBRATE, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CHAT_DISPLAYED_NOTIFICATION, getDefaultUseIMReports());
            addParameter(db, RcsSettingsData.FREETEXT1, ctx.getString(R.string.rcs_settings_label_default_freetext_1));
            addParameter(db, RcsSettingsData.FREETEXT2, ctx.getString(R.string.rcs_settings_label_default_freetext_2));
            addParameter(db, RcsSettingsData.FREETEXT3, ctx.getString(R.string.rcs_settings_label_default_freetext_3));
            addParameter(db, RcsSettingsData.FREETEXT4, ctx.getString(R.string.rcs_settings_label_default_freetext_4));
            addParameter(db, RcsSettingsData.MIN_BATTERY_LEVEL, "0");
            addParameter(db, RcsSettingsData.MAX_PHOTO_ICON_SIZE, "200");
            addParameter(db, RcsSettingsData.MAX_FREETXT_LENGTH, "200");
            addParameter(db, RcsSettingsData.MAX_GEOLOC_LABEL_LENGTH, "200");
            addParameter(db, RcsSettingsData.GEOLOC_EXPIRATION_TIME, "3600");
            addParameter(db, RcsSettingsData.MIN_STORAGE_CAPACITY, "10240");
            addParameter(db, RcsSettingsData.MAX_CHAT_PARTICIPANTS, "21");
            addParameter(db, RcsSettingsData.MAX_CHAT_MSG_LENGTH, getDefaultMessagingTextLimit());
            addParameter(db, RcsSettingsData.MAX_GROUPCHAT_MSG_LENGTH, getDefaultMessagingTextLimit());
            addParameter(db, RcsSettingsData.CHAT_IDLE_DURATION, "300");
            addParameter(db, RcsSettingsData.JOYN_DISABLE_STATUS, "0");
            addParameter(db, RcsSettingsData.MAX_FILE_TRANSFER_SIZE, "10240");
            addParameter(db, RcsSettingsData.WARN_FILE_TRANSFER_SIZE, "9999999");
            addParameter(db, RcsSettingsData.MAX_IMAGE_SHARE_SIZE, "5120");
            addParameter(db, RcsSettingsData.WARN_IMAGE_SHARE_SIZE, "0");
            addParameter(db, RcsSettingsData.MAX_VIDEO_SHARE_DURATION, "54000");
            addParameter(db, RcsSettingsData.MAX_CHAT_SESSIONS, "10");
            addParameter(db, RcsSettingsData.MAX_FILE_TRANSFER_SESSIONS, "10");
            addParameter(db, RcsSettingsData.MAX_IP_CALL_SESSIONS, "5");
            addParameter(db, RcsSettingsData.SMS_FALLBACK_SERVICE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.WARN_SF_SERVICE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.AUTO_ACCEPT_CHAT, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.IM_SESSION_START, "1");
            addParameter(db, RcsSettingsData.USERPROFILE_IMS_USERNAME, "");
            addParameter(db, RcsSettingsData.USERPROFILE_IMS_USERNAME_FULL, "");//@tct-stack wuquan add for full public_user_identify but not only number part
            addParameter(db, RcsSettingsData.USERPROFILE_IMS_DISPLAY_NAME, "");
            addParameter(db, RcsSettingsData.USERPROFILE_IMS_HOME_DOMAIN, "");
            addParameter(db, RcsSettingsData.USERPROFILE_IMS_PRIVATE_ID, "");
            addParameter(db, RcsSettingsData.USERPROFILE_IMS_PASSWORD, "");
            addParameter(db, RcsSettingsData.USERPROFILE_IMS_REALM, "");
            addParameter(db, RcsSettingsData.IMS_PROXY_ADDR_MOBILE, "");
            addParameter(db, RcsSettingsData.IMS_PROXY_PORT_MOBILE, "5060");
            addParameter(db, RcsSettingsData.IMS_PROXY_ADDR_WIFI, "");
            addParameter(db, RcsSettingsData.IMS_PROXY_PORT_WIFI, "5060");
            addParameter(db, RcsSettingsData.XDM_SERVER, "");
            addParameter(db, RcsSettingsData.XDM_LOGIN, "");
            addParameter(db, RcsSettingsData.XDM_PASSWORD, "");
            addParameter(db, RcsSettingsData.XDM_AUTH_TYPE, RcsSettingsData.DIGEST_AUTHENT);
            addParameter(db, RcsSettingsData.FT_HTTP_SERVER, "");
            addParameter(db, RcsSettingsData.FT_HTTP_LOGIN, "");
            addParameter(db, RcsSettingsData.FT_HTTP_PASSWORD, "");
            addParameter(db, RcsSettingsData.FT_PROTOCOL, RcsSettingsData.FT_PROTOCOL_MSRP);
            addParameter(db, RcsSettingsData.IM_CONF_URI, RcsSettingsData.DEFAULT_GROUP_CHAT_URI);
            addParameter(db, RcsSettingsData.IM_MULTI_CONF_URI, RcsSettingsData.DEFAULT_MULTI_CHAT_URI);
            addParameter(db, RcsSettingsData.ENDUSER_CONFIRMATION_URI, "");
            addParameter(db, RcsSettingsData.COUNTRY_CODE, "+33");
            addParameter(db, RcsSettingsData.COUNTRY_AREA_CODE, "0");
            addParameter(db, RcsSettingsData.MSISDN, "");
            addParameter(db, RcsSettingsData.CAPABILITY_CS_VIDEO, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_IMAGE_SHARING, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_VIDEO_SHARING, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_IP_VOICE_CALL, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CAPABILITY_IP_VIDEO_CALL, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CAPABILITY_IM_SESSION, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CAPABILITY_IM_GROUP_SESSION, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CAPABILITY_FILE_TRANSFER, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CAPABILITY_FILE_TRANSFER_HTTP, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_FILE_TRANSFER_THUMBNAIL, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_GROUP_CHAT_SF, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_FILE_TRANSFER_SF, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_SMSOverIP, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_ICSI_MMTEL, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_ICSI_EMERGENCY, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_RCS_EXTENSIONS, "");
            addParameter(db, RcsSettingsData.IMS_SERVICE_POLLING_PERIOD, "300");
            addParameter(db, RcsSettingsData.SIP_DEFAULT_PORT, "5060");
            addParameter(db, RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_MOBILE, ListeningPoint.TCP); // [TBD]
            /**M : added new columns not prestn in stack */
            addParameter(db, RcsSettingsData.SECURE_MSRP_OVER_MOBILE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.SECURE_RTP_OVER_MOBILE, RcsSettingsData.FALSE);
            /**@*/

            /**M : TEMP CHANGE changed from TCP to UDP to allow conection via WIFI */
            addParameter(db, RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_WIFI, ListeningPoint.UDP);
            /**@*/
            addParameter(db, RcsSettingsData.TLS_CERTIFICATE_ROOT, "");
            addParameter(db, RcsSettingsData.TLS_CERTIFICATE_INTERMEDIATE, "");
            addParameter(db, RcsSettingsData.SIP_TRANSACTION_TIMEOUT, "120");
            addParameter(db, RcsSettingsData.MSRP_DEFAULT_PORT, "20000");
            addParameter(db, RcsSettingsData.RTP_DEFAULT_PORT, "10000");
            addParameter(db, RcsSettingsData.MSRP_TRANSACTION_TIMEOUT, "5");
            addParameter(db, RcsSettingsData.REGISTER_EXPIRE_PERIOD, "600000");
            addParameter(db, RcsSettingsData.REGISTER_RETRY_BASE_TIME, "30");
            addParameter(db, RcsSettingsData.REGISTER_RETRY_MAX_TIME, "1800");
            addParameter(db, RcsSettingsData.PUBLISH_EXPIRE_PERIOD, "3600");
            addParameter(db, RcsSettingsData.REVOKE_TIMEOUT, "300");
            addParameter(db, RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE, RcsSettingsData.DIGEST_AUTHENT);
            addParameter(db, RcsSettingsData.IMS_AUTHENT_PROCEDURE_WIFI, RcsSettingsData.DIGEST_AUTHENT);
            addParameter(db, RcsSettingsData.TEL_URI_FORMAT, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.RINGING_SESSION_PERIOD, "60");
            addParameter(db, RcsSettingsData.SUBSCRIBE_EXPIRE_PERIOD, "3600");
            addParameter(db, RcsSettingsData.IS_COMPOSING_TIMEOUT, "5");
            addParameter(db, RcsSettingsData.SESSION_REFRESH_EXPIRE_PERIOD, "1800");
            addParameter(db, RcsSettingsData.PERMANENT_STATE_MODE, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.TRACE_ACTIVATED, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.TRACE_LEVEL, "" + Logger.DEBUG_LEVEL);
            addParameter(db, RcsSettingsData.SIP_TRACE_ACTIVATED, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.SIP_TRACE_FILE, Environment.getDataDirectory() + "/sip.txt");
            addParameter(db, RcsSettingsData.MEDIA_TRACE_ACTIVATED, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CAPABILITY_REFRESH_TIMEOUT, "1");
            addParameter(db, RcsSettingsData.CAPABILITY_EXPIRY_TIMEOUT, "86400");
            addParameter(db, RcsSettingsData.CAPABILITY_POLLING_PERIOD, "604800");
            addParameter(db, RcsSettingsData.IM_CAPABILITY_ALWAYS_ON, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.FT_CAPABILITY_ALWAYS_ON, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.IM_USE_REPORTS, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.NETWORK_ACCESS, "" + RcsSettingsData.ANY_ACCESS);
            addParameter(db, RcsSettingsData.CURRENT_ACCESS_NETWORKINFO, "");
            addParameter(db, RcsSettingsData.LAST_ACCESS_NETWORKINFO, "");
            addParameter(db, RcsSettingsData.SIP_TIMER_T1, "2000");
            addParameter(db, RcsSettingsData.SIP_TIMER_T2, "16000");
            addParameter(db, RcsSettingsData.SIP_TIMER_T4, "17000");
            addParameter(db, RcsSettingsData.SIP_KEEP_ALIVE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.SIP_KEEP_ALIVE_PERIOD, "60");
            addParameter(db, RcsSettingsData.RCS_APN, "");
            addParameter(db, RcsSettingsData.RCS_OPERATOR, "");
            addParameter(db, RcsSettingsData.MAX_CHAT_LOG_ENTRIES, "500");
            addParameter(db, RcsSettingsData.MAX_RICHCALL_LOG_ENTRIES, "200");
            addParameter(db, RcsSettingsData.MAX_IPCALL_LOG_ENTRIES, "200");
            addParameter(db, RcsSettingsData.GRUU, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.USE_IMEI_AS_DEVICE_ID, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CPU_ALWAYS_ON, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.AUTO_CONFIG_MODE, "" + RcsSettingsData.HTTPS_AUTO_CONFIG);
            addParameter(db, RcsSettingsData.PROVISIONING_TERMS_ACCEPTED, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.PROVISIONING_VERSION, "0");
            addParameter(db, RcsSettingsData.PROVISIONING_TOKEN, "");
            addParameter(db, RcsSettingsData.SECONDARY_PROVISIONING_ADDRESS, "");
            addParameter(db, RcsSettingsData.SECONDARY_PROVISIONING_ADDRESS_ONLY, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.DIRECTORY_PATH_PHOTOS, "");
            addParameter(db, RcsSettingsData.DIRECTORY_PATH_VIDEOS, "");
            addParameter(db, RcsSettingsData.DIRECTORY_PATH_FILES, "");
            addParameter(db, RcsSettingsData.SECURE_MSRP_OVER_WIFI, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.SECURE_RTP_OVER_WIFI, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.CONVERGENT_MESSAGING_UX, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CAPABILITY_SIP_AUTOMATA, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.KEY_GSMA_RELEASE, RcsSettingsData.VALUE_GSMA_REL_BLACKBIRD);
            addParameter(db, RcsSettingsData.BE_IPVOICECALL_AUTH, "15");
            addParameter(db, RcsSettingsData.BE_IPVIDEOCALL_AUTH, "15");
            addParameter(db, RcsSettingsData.IPVOICECALL_BREAKOUT, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.IPVOICECALL_BREAKOUT_CS, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.IPVIDEOCALL_UPGRADE_FROM_CS, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.IPVIDEOCALL_UPGRADE_ON_CAPERROR, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.IPVIDEOCALL_UPGRADE_ATTEMPT_EARLY, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.IPCALL_E2E_LABEL, "");
            addParameter(db, RcsSettingsData.IPCALL_BREAKOUT_LABEL, "");
            addParameter(db, RcsSettingsData.IPCALL_E2E_VOICECAPABILITYHANDLING, RcsSettingsData.FALSE);
            /** BB Drop 1 */
            addParameter(db, RcsSettingsData.MESSAGING_UX, getDefaultMessagingUx());
            addParameter(db, RcsSettingsData.DELIVERY_TIMEOUT, "300");
            addParameter(db, RcsSettingsData.VOICE_BREAKOUT, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.FT_HTTP_CAP_ALWAYS_ON, "0");
            addParameter(db, RcsSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.ROAMING_AUTHORIZED, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.RCS_ONLY_ROAMING_AUTHORIZED, RcsSettingsData.TRUE);
            /** M: ftAutAccept @{ */
            addParameter(db, RcsSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING,
                    RcsSettingsData.FALSE);

            addParameter(db, RcsSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING,
                    RcsSettingsData.TRUE);
            /** @}*/
            addParameter(db, RcsSettingsData.RCSE_COMPRESSING_IMAGE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.COMPRESS_IMAGE_HINT, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.JOYN_USER_ALIAS, "");
            addParameter(db, RcsSettingsData.MESSAGING_CAPABILITIES_VALIDITY, "30");
            addParameter(db, RcsSettingsData.PROVISION_VALIDITY, "0");
            addParameter(db, RcsSettingsData.BLOCK_XCAP_OPERATION, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.PROVISIONING_TOKEN_VALIDITY, "0");
            addParameter(db, RcsSettingsData.MAX_STANDALONE_SIZE, "0");
            addParameter(db, RcsSettingsData.CPM_TECHNOLOGY, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CPM_LARGE_MSG, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CPM_PAGER_MODE, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.CPM_FT_MSG, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.MULTIMEDIA_IN_CHAT, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.FIRST_MSG_IN_INVITE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER_ON_ROAMING, RcsSettingsData.FALSE);// @tct-stack huangbo add
            addParameter(db, RcsSettingsData.REGISTRATION_STATUS, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.LAUNCH_STATUS, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.RCS_OPERATOR_ADDRESS, "config.rcs.chinamobile.com");
            addParameter(db, RcsSettingsData.NUMBER_OF_PCSCF_ADDRESS, "1");
            addParameter(db, RcsSettingsData.MSGSTORE_ADDRESS, "");
            addParameter(db, RcsSettingsData.MSGSTORE_USER, "");
            addParameter(db, RcsSettingsData.PUBLIC_URI, "");
            addParameter(db, RcsSettingsData.CONFIGURATION_STATE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.IM_MASS_URI, "");
            addParameter(db, RcsSettingsData.PUBLIC_USER_IDENTITY_1, "");
            addParameter(db, RcsSettingsData.PUBLIC_USER_IDENTITY_2, "");
            addParameter(db, RcsSettingsData.PUBLIC_USER_IDENTITY_3, "");
            addParameter(db, RcsSettingsData.PUBLIC_USER_IDENTITY_PC, "");
            addParameter(db, RcsSettingsData.CAPABILITY_CLOUD_FILE_TRANSFER, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.SERVICE_PERMITTED, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.MAX_PAGER_CONTENT_SIZE, "1300");
            addParameter(db, RcsSettingsData.IR94_VIDEO_AUTH, isOP08SupportedByPlatform());
            addParameter(db, RcsSettingsData.STANDALONE_MSG_SUPPORT, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.ENABLE_RCS_SWITCH, "1");
            addParameter(db, RcsSettingsData.RCS_VOLTE_SINGLE_REGISTRATION, "0");
            addParameter(db, RcsSettingsData.CLIENT_OBJ_DATA_LIMIT, "4096");
            addParameter(db, RcsSettingsData.CONTENT_SERVER_URI, "");
            addParameter(db, RcsSettingsData.SOURCE_THROTTLE_PUBLISH, "30");
            addParameter(db, RcsSettingsData.MAX_SUBSCRIPTION_PRESENCE, "100");
            addParameter(db, RcsSettingsData.SERVICE_URI_TEMPLATE, "");
            addParameter(db, RcsSettingsData.RLS_URI, "");
            addParameter(db, RcsSettingsData.NON_VIP_POLL_PERIOD, "200");
            addParameter(db, RcsSettingsData.NON_VIP_MAX_POLL_PER_PERIOD, "1");
            addParameter(db, RcsSettingsData.CLIENT_VENDOR, "MTK");
            addParameter(db, RcsSettingsData.USE_CLIENT_VENDER_SETTINGS, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.SERVICE_CURRENT_STATE, "10");
            addParameter(db, RcsSettingsData.CAPABILITY_CARD_MESSAGE, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.GZIP_SUPPORT, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.IS_COMPOSING, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.IS_LARGE_MODE, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.FORCE_NAT_TRAVERSAL, RcsSettingsData.FALSE);
            addParameter(db, RcsSettingsData.MAX_AUDIO_MESSAGE_DURATION, Long.toString(RcsSettingsData.DEFAULT_MAX_AUDIO_DURATION));
            addParameter(db, RcsSettingsData.AUTO_ACCEPT_FT_IN_ROAMING, Boolean.toString(RcsSettingsData.DEFAULT_AUTO_ACCEPT_FT_IN_ROAMING));
            addParameter(db, RcsSettingsData.AUTO_ACCEPT_FT_CHANGEABLE, Boolean.toString(RcsSettingsData.DEFAULT_AUTO_ACCEPT_FT_CHANGEABLE));
            addParameter(db, RcsSettingsData.MESSAGING_MODE, RcsSettingsData.DEFAULT_KEY_MESSAGING_MODE.toString());
            addParameter(db, RcsSettingsData.MAX_SUBSCRIPTION_PRESENCE_LIST, "100");
            addParameter(db, RcsSettingsData.CHAT_RESPOND_TO_DISPLAY_REPORTS, RcsSettingsData.TRUE);
            addParameter(db, RcsSettingsData.GROUP_MESSAGING_TOGGLE, RcsSettingsData.TRUE);
        }

        public static String isOP08SupportedByPlatform() {
            String status = RcsSettingsData.FALSE;
            if (RcsSettings.getInstance().isSupportOP08()) {
                //needs to be extracted from API provided by GBA for SIM
                status = RcsSettingsData.TRUE;
            }
            return status;
        }

        public static String getDefaultUseIMReports() {
            String status = RcsSettingsData.TRUE;
            if (RcsSettings.getInstance().isSupportOP07()) {
                //needs to be extracted from API provided by GBA for SIM
                status = RcsSettingsData.FALSE;
            }
            return status;
        }

        public static String getDefaultMessagingUx() {
            int status = 0;
            if (RcsSettings.getInstance().isSupportOP07() || RcsSettings.getInstance().isSupportOP08()) {
                //needs to be extracted from API provided by GBA for SIM
                status = 1;
            }
            return Integer.toString(status);
        }

        public static String getDefaultMessagingTextLimit() {
            int limit = 1000;
            if (RcsSettings.getInstance().isSupportOP07()) {
                //needs to be extracted from API provided by GBA for SIM
                limit = 2000;
            }
            return Integer.toString(limit);
        }

        /**
         * Add a parameter in the database
         *
         * @param db    Database
         * @param key   Key
         * @param value Value
         */
        private void addParameter(SQLiteDatabase db, String key, String value) {

            if (mIsEncrypted) {
                try {
                    String cipherKey = RcsSettings.getCipherKey();
                    if (value != null) {
                        value = RcsSettings.cipher(cipherKey, value);
                    }
                    key = RcsSettings.cipher(cipherKey, key);
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
            String sql = "INSERT INTO " + TABLE + " (" +
                    RcsSettingsData.KEY_KEY + "," +
                    RcsSettingsData.KEY_VALUE + ") VALUES ('" +
                    key + "','" + value + "');";
            db.execSQL(sql);
        }

        @Override
        public void onUpgrade(SQLiteDatabase db, int oldVersion, int currentVersion) {
            // Get old data before deleting the table
            Cursor oldDataCursor = db.query(TABLE, null, null, null, null, null, null);

            // Get all the pairs key/value of the old table to insert them back
            // after update
            ArrayList<ContentValues> valuesList = new ArrayList<ContentValues>();
            while (oldDataCursor.moveToNext()) {
                String key = null;
                String value = null;
                int index = oldDataCursor.getColumnIndex(RcsSettingsData.KEY_KEY);
                if (index != -1) {
                    key = oldDataCursor.getString(index);
                }
                index = oldDataCursor.getColumnIndex(RcsSettingsData.KEY_VALUE);
                if (index != -1) {
                    value = oldDataCursor.getString(index);
                }
                if (key != null && value != null) {
                    ContentValues values = new ContentValues();
                    values.put(RcsSettingsData.KEY_KEY, key);
                    values.put(RcsSettingsData.KEY_VALUE, value);
                    valuesList.add(values);
                }
            }
            oldDataCursor.close();

            // Delete old table
            db.execSQL("DROP TABLE IF EXISTS " + TABLE);

            // Recreate table
            onCreate(db);

            // Put the old values back when possible
            for (int i = 0; i < valuesList.size(); i++) {
                ContentValues values = valuesList.get(i);
                String whereClause = RcsSettingsData.KEY_KEY + "=" + "\"" + values.getAsString(RcsSettingsData.KEY_KEY) + "\"";
                // Update the value with this key in the newly created database
                // If key is not present in the new version, this won't do anything
                db.update(TABLE, values, whereClause, null);
            }
        }
    }

    @Override
    public boolean onCreate() {
        openHelper = new DatabaseHelper(getContext());
        return true;
    }

    @Override
    public String getType(Uri uri) {
        int match = uriMatcher.match(uri);
        switch (match) {
            case SETTINGS:
                return "vnd.android.cursor.dir/com.orangelabs.rcs.settings";
            case SETTINGS_ID:
                return "vnd.android.cursor.item/com.orangelabs.rcs.settings";
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }
    }

    /**
     * M: Added to achieve the auto configuration feature. @{
     */
    private String getTable(Uri uri) {
        String table = null;
        if (LauncherUtils.getDebugMode(getContext())) {
            table = FIRST_USER_ACCOUNT_TABLE;
        } else {
            if (uri != null) {
                String strUri = uri.toSafeString();
                if (strUri.endsWith(FIRST_USER_ACCOUNT_TABLE)) {
                    table = FIRST_USER_ACCOUNT_TABLE;
                } else {
                }
            }
        }
        return table;
    }

    /**
     * @}
     */

    @Override
    public Cursor query(Uri uri, String[] projectionIn, String selection, String[] selectionArgs, String sort) {
        SQLiteQueryBuilder qb = new SQLiteQueryBuilder();
        qb.setTables(TABLE);

        // Generate the body of the query
        int match = uriMatcher.match(uri);
        switch (match) {
            case SETTINGS:
                break;
            case SETTINGS_ID:
                qb.appendWhere(RcsSettingsData.KEY_ID + "=");
                qb.appendWhere(uri.getPathSegments().get(1));
                break;
            default:
                throw new IllegalArgumentException("Unknown URI " + uri);
        }

        SQLiteDatabase db = openHelper.getReadableDatabase();
        Cursor c = qb.query(db, projectionIn, selection, selectionArgs, null, null, sort);

        // Register the contexts ContentResolver to be notified if
        // the cursor result set changes.
        if (c != null) {
            c.setNotificationUri(getContext().getContentResolver(), uri);
        }

        return c;
    }

    @Override
    public int update(Uri uri, ContentValues values, String where, String[] whereArgs) {
        int count;
        SQLiteDatabase db = openHelper.getWritableDatabase();

        int match = uriMatcher.match(uri);
        switch (match) {
            case SETTINGS:
                count = db.update(TABLE, values, where, null);
                break;
            case SETTINGS_ID:
                String segment = uri.getPathSegments().get(1);
                int id = Integer.parseInt(segment);
                count = db.update(TABLE, values, RcsSettingsData.KEY_ID + "=" + id, null);
                break;
            default:
                throw new UnsupportedOperationException("Cannot update URI " + uri);
        }
        getContext().getContentResolver().notifyChange(uri, null);
        return count;
    }

    @Override
    public Uri insert(Uri uri, ContentValues initialValues) {
        SQLiteDatabase db = openHelper.getWritableDatabase();
        String table = getTable(uri);
        if (table == null) {
            return null;
        }
        db.insert(table, null, initialValues);
        return uri;
        //  throw new UnsupportedOperationException("Cannot insert URI " + uri);
    }

    @Override
    public int delete(Uri uri, String where, String[] whereArgs) {
        int rowDel;
        SQLiteDatabase db = openHelper.getWritableDatabase();
        String table = getTable(uri);
        if (table == null) {
            return 0;
        }
        //      String whereClause = RcsSettingsData.KEY_KEY + "=" + "\""+ values.getAsString(RcsSettingsData.KEY_KEY) + "\"";
        return db.delete(table, where, whereArgs);
        //throw new UnsupportedOperationException();
    }
}
