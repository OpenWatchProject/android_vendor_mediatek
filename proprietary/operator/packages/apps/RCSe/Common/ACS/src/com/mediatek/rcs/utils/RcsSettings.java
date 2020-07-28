/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.rcs.utils;

import android.app.Activity;
import android.content.SharedPreferences;
import android.net.Uri;
import android.util.SparseArray;

import com.mediatek.rcs.provisioning.ProvisioningInfo;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.HashMap;
import java.util.Map;

import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class RcsSettings {
    public static final boolean ATT_SUPPORT = true;
    //public static final String USERPROFILE_IMS_PASSWORD = "ImsPassword";
    public static final String USERPROFILE_IMS_PRIVATE_ID = "ImsPrivateId";
    /* package private */static final String TC_RESPONSE = "TCResp";
    public static final String CONFIGURATION_VALID = "ConfigurationValidity";
    public static final String PROVISIONING_VERSION = "ProvisioningVersion";
    static final String PROV_ACCEPT_BUTTON = "Accept_btn";
    /* package private */static final String PROV_REJECT_BUTTON = "Reject_btn";
    //public static final String CONFIGURATION_VALID = "ConfigurationValidity";

    /**
     * UUID value for populating SIP instance
     */
    public static final String UUID = "UUID";
    /* package private */static final String DEFAULT_UUID = null;

    /**
     * Provisioning version
     */
    public static final String PROVISIONING_TOKEN = "ProvisioningToken";
    /* package private */static final String DEFAULT_PROVISIONING_TOKEN = null;

    /**
     * Activate or not the RCS service
     */
    public static final String SERVICE_ACTIVATED = "ServiceActivated";
    /* package private */static final Boolean DEFAULT_SERVICE_ACTIVATED = true;

    /**
     * Key and associated values for GSMA release of the device as provisioned by the network
     */
    public static final String KEY_GSMA_RELEASE = "GsmaRelease";
    /* package private */static final Integer DEFAULT_KEY_GSMA_RELEASE = 1;

    /**
     * IMS username or username part of the IMPU (for HTTP Digest only)
     */
    public static final String USERPROFILE_IMS_USERNAME = "MyContactId";
    /* package private */static final ContactId DEFAULT_USERPROFILE_IMS_USERNAME = null;

    /**
     * Secondary provisioning address
     */
    public static final String SECONDARY_PROVISIONING_ADDRESS = "SecondaryProvisioningAddress";
    /* package private */static final String DEFAULT_SECONDARY_PROV_ADDR = null;

    /**
     * Use only the secondary provisioning address
     */
    public static final String SECONDARY_PROVISIONING_ADDRESS_ONLY = "SecondaryProvisioningAddressOnly";
    /* package private */static final Boolean DEFAULT_SECONDARY_PROV_ADDR_ONLY = false;

    /**
     * Mobile Country Code (0 if undefined)
     */
    /* package private */static final String MOBILE_COUNTRY_CODE = "mcc";
    /* package private */static final Integer DEFAULT_MOBILE_COUNTRY_CODE = 0;

    /**
     * Mobile Network Code (0 if undefined)
     */
    /* package private */static final String MOBILE_NETWORK_CODE = "mnc";
    /* package private */static final Integer DEFAULT_MOBILE_NETWORK_CODE = 0;

    /**
     * Provisioning optional user message title associated with the result of the configuration
     * server response
     */
    /* package private */static final String PROV_USER_MSG_TITLE = "title";
    /* package private */static final String DEFAULT_PROV_USER_MSG_TITLE = null;

    /**
     * Provisioning optional user message content associated with the result of the configuration
     * server response
     */
    /* package private */static final String PROV_USER_MSG_CONTENT = "message";
    /* package private */static final String DEFAULT_PROV_USER_MSG_CONTENT = null;

    private static final String CONFIG_FILE_CONTENT = "config_file_content";
    private static final String DEFAULT_CONFIG_FILE_CONTENT = null;

    // private static final String CONFIG_STATUS = "config_status";
    // private static final Integer DEFAULT_CONFIG_STATUS = 0;

    private static final String CONFIG_CLIENT_VERSION = "client_version";
    private static final String DEFAULT_CONFIG_CLIENT_VERSION = null;

    private static final String CONFIG_CLIENT_VENDOR = "client_vendor";
    private static final String DEFAULT_CONFIG_CLIENT_VENDOR = null;

    private static final String CONFIG_DEFAULT_SMS_APP = "default_sms_app";
    private static final String DEFAULT_CONFIG_DEFAULT_SMS_APP = null;

    static final String BUILD_VERSION_RELEASE = "ro.build.version.release";
    static final String DEFAULT_BUILD_VERSION_RELEASE = "0";

    /**
     * Publish expire period
     */
    public static final String PUBLISH_EXPIRE_PERIOD = "PublishExpirePeriod";
    /* package private */static final Long DEFAULT_PUBLISH_EXPIRE_PERIOD = 3600000L;

    /**
     * Publish source throttle
     */
    public static final String PUBLISH_SOURCE_THROTTLE = "source-throttlepublish";
    /* package private */static final Long DEFAULT_PUBLISH_SOURCE_THROTTLE = 30L;

    public static final String PUBLISH_DISABLE_INITIAL_ADDRESS_BOOK_SCAN
    = "disableInitialAddressBookScan";
    static final Long DEFAULT_PUBLISH_DISABLE_INITIAL_ADDRESS_BOOK_SCAN = 0L;

    /**
     * Polling period used to decide when to refresh contacts capabilities
     */
    public static final String CAPABILITY_POLLING_PERIOD = "pollingPeriod";
    /* package private */static final Long DEFAULT_CAPABILITY_POLLING_PERIOD = 0L; //3600000L;

    public static final String CAPABILITY_POLLIGN_RATE = "pollingRate";
    static final Long DEFAULT_CAPABILITY_POLLIGN_RATE = 0L;

    public static final String CAPABILITY_POLLIGN_RATE_PERIOD = "pollingRatePeriod";
    static final Long DEFAULT_CAPABILITY_POLLIGN_RATE_PERIOD = 0L;

    /**
     * Capability refresh timeout used to decide when to refresh contact capabilities
     */
    public static final String CAPABILITY_INFO_EXPIRY = "capInfoExpiry";
    static final Long DEFAULT_CAPABILITY_EXPIRY_TIMEOUT = 300L;

    public static final String CAPABILITY_NON_RCSCAPINFO_EXPIRAY = "nonRCScapInfoExpiry";
    static final Long DEFAULT_CAPABILITY_NON_RCSCAPINFO_EXPIRAY = 259200L;

    public static final String CAPABILITY_DEFAULT_DISC = "defaultDisc";
    static final Long DEFAULT_CAPABILITY_DEFAULT_DISC = 0L;

    public static final String CAPABILITY_DISC_COMMON_STACK = "capDiscCommonStack";
    static final Long DEFAULT_CAPABILITY_DISC_COMMON_STACK = 0L;

    /**
     * File transfer HTTP server address & port
     */
    public static final String FT_HTTP_SERVER = "FtHttpServerAddr";
    static final Uri DEFAULT_FT_HTTP_SERVER = null;

    /**
     * File transfer HTTP server login
     */
    public static final String FT_HTTP_LOGIN = "FtHttpServerLogin";
    static final String DEFAULT_FT_HTTP_LOGIN = null;

    /**
     * File transfer HTTP server password
     */
    public static final String FT_HTTP_PASSWORD = "FtHttpServerPassword";
    static final String DEFAULT_FT_HTTP_PASSWORD = null;

    // User Disable ACS
    public static final String USER_DISABLE_ACS = "UserDisabled";
    static final Boolean DEFAULT_USER_DISABLE_ACS = false;

    // User Provisoning Addr
    public static final String USER_PROVISIONING_ADDR = "UserProvisioningAddress";
    static final String DEFAULT_USER_PROVISIONING_ADDR = null;

    public enum TermsAndConditionsResponse {
        NO_ANSWER(0), DECLINED(1), ACCEPTED(2);

        private final int mValue;

        private static SparseArray<TermsAndConditionsResponse> mValueToEnum = new SparseArray<TermsAndConditionsResponse>();

        static {
            for (TermsAndConditionsResponse entry : TermsAndConditionsResponse.values()) {
                mValueToEnum.put(entry.toInt(), entry);
            }
        }

        TermsAndConditionsResponse(int value) {
            mValue = value;
        }

        /**
         * @return value
         */
        public final int toInt() {
            return mValue;
        }

        /**
         * @param value the value representing the Terms and Condition response
         * @return TermAndConditionsResponse
         */
        public static TermsAndConditionsResponse valueOf(int value) {
            TermsAndConditionsResponse entry = mValueToEnum.get(value);
            if (entry != null) {
                return entry;
            }
            throw new IllegalArgumentException("No enum const class "
                                               + TermsAndConditionsResponse.class.getName() + "." + value);
        }

    }

    /**
     * The GSMA release enumerated type.
     */
    public enum GsmaRelease {
        /**
         * Albatros release
         */
        ALBATROS(0),
        /**
         * Blackbird release
         */
        BLACKBIRD(1),
        /**
         * Crane release
         */
        CRANE(2);

        private int mValue;

        private static SparseArray<GsmaRelease> mValueToEnum = new SparseArray<GsmaRelease>();

        static {
            for (GsmaRelease entry : GsmaRelease.values()) {
                mValueToEnum.put(entry.toInt(), entry);
            }
        }

        GsmaRelease(int value) {
            mValue = value;
        }

        /**
         * @return value
         */
        public final int toInt() {
            return mValue;
        }

        /**
         * @param value the value representing the GSMA release
         * @return GsmaRelease
         */
        public static GsmaRelease valueOf(int value) {
            GsmaRelease entry = mValueToEnum.get(value);
            if (entry != null) {
                return entry;
            }
            throw new IllegalArgumentException("No enum const class " + GsmaRelease.class.getName()
                                               + "." + value);
        }

    }

    final private Map<String, Object> mCache;
    private static volatile RcsSettings sInstance;

    /**
     * Get or Create Singleton instance of RcsSettings.
     *
     * @param localContentResolver Local content resolver
     * @return RcsSettings instance
     */
    public static RcsSettings getInstance() {
        if (sInstance != null) {
            return sInstance;
        }

        synchronized (RcsSettings.class) {
            if (sInstance == null) {
                sInstance = new RcsSettings();
            }

            return sInstance;
        }
    }

    /**
     * Constructor
     *
     * @param localContentResolver Local content resolver
     */
    private RcsSettings() {
        super();
        mCache = new HashMap<String, Object>();
    }

    /**
     * Is configuration valid
     *
     * @return Boolean
     */
    public boolean isConfigurationValid() {
        return readBoolean(CONFIGURATION_VALID);
    }

    /**
     * Set configuration valid
     *
     * @param valid true if configuration is valid
     */
    public void setConfigurationValid(boolean valid) {
        writeBoolean(CONFIGURATION_VALID, valid);
    }

    /**
     * Get user profile IMS private Id (i.e. IMPI)
     *
     * @return SIP-URI
     */
    public String getUserProfileImsPrivateId() {
        return readString(USERPROFILE_IMS_PRIVATE_ID);
    }

    /**
     * Set user profile IMS private Id (i.e. IMPI)
     *
     * @param uri SIP-URI
     */
    public void setUserProfileImsPrivateId(String uri) {
        writeString(USERPROFILE_IMS_PRIVATE_ID, uri);
    }


    /**
     * Get provisioning version
     *
     * @return Version
     */
    public int getProvisioningVersion() {
        return readInteger(PROVISIONING_VERSION);
    }

    // /**
    //  * Get the latest positive provisioning version
    //  *
    //  * @param context Application context
    //  * @return the latest positive provisioning version
    //  */
    // public static int getProvisioningVersion() {
    //     SharedPreferences preferences = AndroidFactory.getApplicationContext().getSharedPreferences(
    //             LauncherUtils.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
    //     return preferences.getInt(PROVISIONING_VERSION,
    //             ProvisioningInfo.Version.RESETED.toInt());
    // }

    /**
     * Set provisioning version
     *
     * @param version Version
     */
    public void setProvisioningVersion(int version) {
        writeInteger(PROVISIONING_VERSION, version);
    }

    /**
     * Sets whether an Accept button is shown with the message in the terms and conditions pop-up
     *
     * @param accept True if an Accept button is shown with the message in the terms and conditions
     *            pop-up
     */
    public void setProvisioningAcceptButton(boolean accept) {
        writeBoolean(PROV_ACCEPT_BUTTON, accept);
    }

    /**
     * Is Accept button shown with the message in the terms and conditions pop-up
     *
     * @return Boolean True if Accept button shown with the message in the terms and conditions
     *         pop-up
     */
    public boolean isProvisioningAcceptButton() {
        return readBoolean(PROV_ACCEPT_BUTTON);
    }

    /**
     * Sets whether a Decline button is shown with the message in the terms and conditions pop-up
     *
     * @param reject True if a Decline button is shown with the message in the terms and conditions
     *            pop-up
     */
    public void setProvisioningRejectButton(boolean reject) {
        writeBoolean(PROV_REJECT_BUTTON, reject);
    }

    /**
     * Is Decline button shown with the message in the terms and conditions pop-up
     *
     * @return Boolean True if a Decline button is shown with the message in the terms and
     *         conditions pop-up
     */
    public boolean isProvisioningRejectButton() {
        return readBoolean(PROV_REJECT_BUTTON);
    }

    public TermsAndConditionsResponse getTermsAndConditionsResponse() {
        return TermsAndConditionsResponse.valueOf(readInteger(TC_RESPONSE));
    }

    /**
     * Set terms and conditions response
     *
     * @param resp The terms and conditions response
     */
    public void setTermsAndConditionsResponse(TermsAndConditionsResponse resp) {
        writeInteger(TC_RESPONSE, resp.toInt());
    }

    /**
     * Read boolean parameter
     * <p/>
     * If parsing of the value fails, method return false.
     *
     * @param key the key field
     * @return the value field
     */
    public boolean readBoolean(String key) {
        Boolean value = (Boolean) mCache.get(key);
        if (value == null) {
            value = Boolean.parseBoolean(readParameter(key));
            mCache.put(key, value);
        }
        return value;
    }

    /**
     * Write boolean parameter
     *
     * @param key the key field
     * @param value the boolean value
     */
    public void writeBoolean(String key, Boolean value) {
        if (writeParameter(key, value.toString())) {
            mCache.put(key, value);
        }
    }

    /**
     * Read int parameter
     * <p/>
     * If parsing of the value fails, method return default value.
     *
     * @param key the key field
     * @return the value field
     */
    public int readInteger(String key) {
        Integer value = (Integer) mCache.get(key);
        if (value == null) {
            value = Integer.parseInt(readParameter(key));
            mCache.put(key, value);
        }
        return value;
    }

    /**
     * Read long parameter
     * <p/>
     * If parsing of the value fails, method return default value.
     *
     * @param key the key field
     * @return the value field
     */
    public long readLong(String key) {
        Long value = (Long) mCache.get(key);
        if (value == null) {
            value = Long.parseLong(readParameter(key));
            mCache.put(key, value);
        }
        return value;
    }

    /**
     * Read String parameter
     *
     * @param key the key field
     * @return the value field or defaultValue (if read fails)
     */
    public String readString(String key) {
        String value = (String) mCache.get(key);
        if (value == null && !mCache.containsKey(key)) {
            value = readParameter(key);
            mCache.put(key, value);
        }
        return value;
    }

    /**
     * Write integer parameter
     *
     * @param key the key field
     * @param value the integer value
     */
    public void writeInteger(String key, Integer value) {
        if (writeParameter(key, value.toString())) {
            mCache.put(key, value);
        }
    }

    /**
     * Write long parameter
     *
     * @param key the key field
     * @param value the long value
     */
    public void writeLong(String key, Long value) {
        if (writeParameter(key, value.toString())) {
            mCache.put(key, value);
        }
    }

    /**
     * Write String parameter
     *
     * @param key the key field
     * @param value the long value
     */
    public void writeString(String key, String value) {
        if (writeParameter(key, value)) {
            mCache.put(key, value);
        }
    }

    /**
     * Read Uri parameter
     *
     * @param key the key field
     * @return the value field or defaultValue (if read fails)
     */
    public Uri readUri(String key) {
        Uri value = (Uri) mCache.get(key);
        if (value == null && !mCache.containsKey(key)) {
            String dbValue = readParameter(key);
            if (dbValue != null) {
                value = Uri.parse(dbValue);
            }
            mCache.put(key, value);
        }
        return value;
    }

    /**
     * Write uri parameter
     *
     * @param key the key field
     * @param value the long value
     */
    public void writeUri(String key, Uri value) {
        if (writeParameter(key, value == null ? null : value.toString())) {
            mCache.put(key, value);
        }
    }

    /**
     * Read ContactId parameter
     *
     * @param key the key field
     * @return the value field or defaultValue (if read fails)
     */
    public ContactId readContactId(String key) {
        ContactId value = (ContactId) mCache.get(key);
        if (value == null && !mCache.containsKey(key)) {
            String dbValue = readParameter(key);
            if (dbValue != null) {
                value = ContactUtil.createContactIdFromTrustedData(dbValue);
            }
            mCache.put(key, value);
        }
        return value;
    }

    /**
     * Write ContactId parameter
     *
     * @param key the key field
     * @param value the long value
     */
    public void writeContactId(String key, ContactId value) {
        if (writeParameter(key, value == null ? null : value.toString())) {
            mCache.put(key, value);
        }
    }

    /**
     * Read a parameter from database
     *
     * @param key Key
     * @return Value
     */
    private String readParameter(String key) {
        SharedPreferences preferences = AndroidFactory.getApplicationContext()
            .getSharedPreferences(LauncherUtils.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        String defaultValue = null;
        String value = preferences.getString(key, defaultValue);
        if (value != null) {
            if (sUseEncrypt) {
                return decrypt(hexStringToByte(value), sPassword);
            }
            return value;
        }

        if (TC_RESPONSE.equals(key)) {
            defaultValue = String.valueOf(TermsAndConditionsResponse.NO_ANSWER.toInt());
        } else if (PROVISIONING_VERSION.equals(key)) {
            defaultValue = String.valueOf(ProvisioningInfo.Version.RESETED.toInt());
        } else if (KEY_GSMA_RELEASE.equals(key)) {
            defaultValue = DEFAULT_KEY_GSMA_RELEASE.toString();
        } else if (SERVICE_ACTIVATED.equals(key)) {
            defaultValue = DEFAULT_SERVICE_ACTIVATED.toString();
        } else if (PUBLISH_DISABLE_INITIAL_ADDRESS_BOOK_SCAN.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_PUBLISH_DISABLE_INITIAL_ADDRESS_BOOK_SCAN);
        } else if (CAPABILITY_POLLING_PERIOD.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_CAPABILITY_POLLING_PERIOD);
        } else if (CAPABILITY_INFO_EXPIRY.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_CAPABILITY_EXPIRY_TIMEOUT);
        } else if (CAPABILITY_NON_RCSCAPINFO_EXPIRAY.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_CAPABILITY_NON_RCSCAPINFO_EXPIRAY);
        } else if (CAPABILITY_DEFAULT_DISC.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_CAPABILITY_DEFAULT_DISC);
        } else if (CAPABILITY_DISC_COMMON_STACK.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_CAPABILITY_DISC_COMMON_STACK);
        } else if (CAPABILITY_POLLIGN_RATE.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_CAPABILITY_POLLIGN_RATE);
        } else if (CAPABILITY_POLLIGN_RATE_PERIOD.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_CAPABILITY_POLLIGN_RATE_PERIOD);
        } else if (PUBLISH_SOURCE_THROTTLE.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_PUBLISH_SOURCE_THROTTLE);
        } else if (MOBILE_NETWORK_CODE.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_MOBILE_NETWORK_CODE);
        } else if (MOBILE_COUNTRY_CODE.equals(key)) {
            defaultValue = String.valueOf(DEFAULT_MOBILE_COUNTRY_CODE);
        } else if (USER_DISABLE_ACS.equals(key)){
            defaultValue = String.valueOf(DEFAULT_USER_DISABLE_ACS);
        }

        return defaultValue;
    }

    /**
     * Write a string setting parameter to Database
     *
     * @param key the key setting
     * @param value the value setting
     * @return the write parameter succeed or not
     */
    private boolean writeParameter(String key, String value) {
        SharedPreferences preferences = AndroidFactory.getApplicationContext()
            .getSharedPreferences(LauncherUtils.RCS_PREFS_NAME, Activity.MODE_PRIVATE);
        SharedPreferences.Editor editor = preferences.edit();
        if (sUseEncrypt) {
            String encryptedValue = encrypt(value, sPassword);
            editor.putString(key, encryptedValue);
        } else {
            editor.putString(key, value);
        }
        return editor.commit();
    }

    /**
     * Reset configuration parameters to default values
     */
    public void resetConfigParameters() {
        mCache.clear();
    }

    // /**
    //  * Set terms and conditions response
    //  *
    //  * @param resp The terms and conditions response
    //  */
    // public void setTermsAndConditionsResponse(TermsAndConditionsResponse resp) {
    //     writeInteger(TC_RESPONSE, resp.toInt());
    // }

    /**
     * Get UUID(Universally Unique Identifier) format: 8-4-4-4-12 hex digits
     *
     * @return uuid value
     */
    public String getUUID() {
        return readString(UUID);
    }

    /**
     * Set provisioning token
     *
     * @param token the token
     */
    public void setProvisioningToken(String token) {
        writeString(PROVISIONING_TOKEN, token);
    }

    /**
     * @return provisioning token
     */
    public String getProvisioningToken() {
        return readString(PROVISIONING_TOKEN);
    }

    /**
     * Is RCS service activated
     *
     * @return Boolean
     */
    public boolean isServiceActivated() {
        return readBoolean(SERVICE_ACTIVATED);
    }

    /**
     * Set the RCS service activation state
     *
     * @param state State
     */
    public void setServiceActivationState(boolean state) {
        writeBoolean(SERVICE_ACTIVATED, state);
    }

    /**
     * Get the GSMA release
     *
     * @return the GSMA release
     */
    public GsmaRelease getGsmaRelease() {
        return GsmaRelease.valueOf(readInteger(KEY_GSMA_RELEASE));
    }

    /**
     * Set the GSMA release
     *
     * @param release Release
     */
    public void setGsmaRelease(GsmaRelease release) {
        writeInteger(KEY_GSMA_RELEASE, release.toInt());
    }

    /**
     * Get user profile username (i.e. username part of the IMPU)
     *
     * @return Username part of SIP-URI or null if not provisioned
     */
    public ContactId getUserProfileImsUserName() {
        return readContactId(USERPROFILE_IMS_USERNAME);
    }

    /**
     * Set user profile IMS username (i.e. username part of the IMPU)
     *
     * @param contact the contact ID
     */
    public void setUserProfileImsUserName(ContactId contact) {
        writeContactId(USERPROFILE_IMS_USERNAME, contact);
    }


    /**
     * Get secondary provisioning address
     *
     * @return Address
     */
    public String getSecondaryProvisioningAddress() {
        return readString(SECONDARY_PROVISIONING_ADDRESS);
    }

    /**
     * Is secondary provisioning address only used
     *
     * @return Boolean
     */
    public boolean isSecondaryProvisioningAddressOnly() {
        return readBoolean(SECONDARY_PROVISIONING_ADDRESS_ONLY);
    }

    /**
     * Get mobile country code
     *
     * @return mobile country code or 0 if undefined
     */
    public int getMobileCountryCode() {
        return readInteger(MOBILE_COUNTRY_CODE);
    }

    /**
     * Set the mobile country code
     *
     * @param mcc the mobile country code
     */
    public void setMobileCountryCode(int mcc) {
        writeInteger(MOBILE_COUNTRY_CODE, mcc);
    }

    /**
     * Get mobile network code
     *
     * @return mobile network code or 0 if undefined
     */
    public int getMobileNetworkCode() {
        return readInteger(MOBILE_NETWORK_CODE);
    }

    /**
     * Set the mobile network code
     *
     * @param mnc the mobile network code
     */
    public void setMobileNetworkCode(int mnc) {
        writeInteger(MOBILE_NETWORK_CODE, mnc);
    }

    /**
     * Sets the user message title
     *
     * @param title the user message title
     */
    public void setProvisioningUserMessageTitle(String title) {
        writeString(PROV_USER_MSG_TITLE, title);
    }

    /**
     * Gets the user message title
     *
     * @return the user message title
     */
    public String getProvisioningUserMessageTitle() {
        return readString(PROV_USER_MSG_TITLE);
    }

    /**
     * Sets the user message content
     *
     * @param message the user message content
     */
    public void setProvisioningUserMessageContent(String message) {
        writeString(PROV_USER_MSG_CONTENT, message);
    }

    /**
     * Gets the user message content
     *
     * @return the user message content
     */
    public String getProvisioningUserMessageContent() {
        return readString(PROV_USER_MSG_CONTENT);
    }

    public String getLastBuildVersionRelease() {
        return readString(BUILD_VERSION_RELEASE);
    }

    public void setLastBuildVersionRelease(String version) {
        writeString(BUILD_VERSION_RELEASE, version);
    }

    public String getConfigFileContent() {
        return readString(CONFIG_FILE_CONTENT);
    }

    public void setConfigFileContent(String content) {
        writeString(CONFIG_FILE_CONTENT, content);
    }

    // public int getConfigState() {
    //     return readInteger(CONFIG_STATUS);
    // }

    // public void setConfigState(int state) {
    //     writeInteger(CONFIG_STATUS, state);
    // }

    public String getClientVersion() {
        return readString(CONFIG_CLIENT_VERSION);
    }

    public void setClientVersion(String version) {
        if (version != null) {
            writeString(CONFIG_CLIENT_VERSION, version);
        }
    }

    public String getClientVendor() {
        return readString(CONFIG_CLIENT_VENDOR);
    }

    public void setClientVendor(String vendor) {
        if (vendor != null) {
            writeString(CONFIG_CLIENT_VENDOR, vendor);
        }
    }

    public String getDefaultSmsApp() {
        return readString(CONFIG_DEFAULT_SMS_APP);
    }

    public void setDefaultSmsApp(String defaultSmsApp) {
        if (defaultSmsApp != null) {
            writeString(CONFIG_DEFAULT_SMS_APP, defaultSmsApp);
        }
    }

    public long getPublishSourceThrottle() {
        return readLong(PUBLISH_SOURCE_THROTTLE);
    }

    public void setPublishSourceThrottle(long publishSourceThrottle) {
        writeLong(PUBLISH_SOURCE_THROTTLE, publishSourceThrottle);
    }

    public long getPollingPeriod() {
        return readLong(CAPABILITY_POLLING_PERIOD);
    }

    public void setPollingPeriod(long pollingPeriod) {
        writeLong(CAPABILITY_POLLING_PERIOD, pollingPeriod);
    }

    public long getNonRcscapInfoExpiray() {
        return readLong(CAPABILITY_NON_RCSCAPINFO_EXPIRAY);
    }

    public void setNonRcscapInfoExpiray(long nonRcscapInfoExpiray) {
        writeLong(CAPABILITY_NON_RCSCAPINFO_EXPIRAY, nonRcscapInfoExpiray);
    }

    public long getInfoExpiry() {
        return readLong(CAPABILITY_INFO_EXPIRY);
    }

    public void setInfoExpiry(long infoExpiry) {
        writeLong(CAPABILITY_INFO_EXPIRY, infoExpiry);
    }

    /**
     * Get file transfer HTTP server address
     *
     * @return Address
     */
    public Uri getFtHttpServer() {
        return readUri(FT_HTTP_SERVER);
    }

    /**
     * Set file transfer HTTP server address
     *
     * @param addr Address
     */
    public void setFtHttpServer(Uri addr) {
        writeUri(FT_HTTP_SERVER, addr);
    }

    /**
     * Get file transfer HTTP server login
     *
     * @return String value
     */
    public String getFtHttpLogin() {
        return readString(FT_HTTP_LOGIN);
    }

    /**
     * Set file transfer HTTP server login
     *
     * @param value Value
     */
    public void setFtHttpLogin(String value) {
        writeString(FT_HTTP_LOGIN, value);
    }

    /**
     * Get file transfer HTTP server password
     *
     * @return String value
     */
    public String getFtHttpPassword() {
        return readString(FT_HTTP_PASSWORD);
    }

    /**
     * Set file transfer HTTP server password
     *
     * @param value Value
     */
    public void setFtHttpPassword(String value) {
        writeString(FT_HTTP_PASSWORD, value);
    }

    public boolean isUserDisableAcs() {
        return readBoolean(USER_DISABLE_ACS);
    }

    public void setUserDisableAcs(boolean value) {
        writeBoolean(USER_DISABLE_ACS, value);
    }

    public String getUserProvisioningAddr() {
        return readString(USER_PROVISIONING_ADDR);
    }

    public void setUserProvisioningAddr(String value) {
        writeString(USER_PROVISIONING_ADDR, value);
    }

    /*
     * encrypte & decrypte string algorithm
     *
     */
    private static final boolean sUseEncrypt = true;
    private static String sAlgorithm = "AES";
    private static String sPassword = "FEDCBA9876543210";
    private static String encrypt(String buf, String passwd) {
        if (buf != null) {
            SecretKey key = getKey(passwd);
            SecretKeySpec ss = new SecretKeySpec(passwd.getBytes(), sAlgorithm);
            try {
                IvParameterSpec iv = new IvParameterSpec(passwd.getBytes("UTF-8"));
                Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
                cipher.init(Cipher.ENCRYPT_MODE, ss, iv);
                byte bt[] = cipher.doFinal(buf.getBytes());
                return byteToHexString(bt);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return null;
    }

    private static String decrypt(byte[] buf, String passwd) {
        SecretKeySpec ss = new SecretKeySpec(passwd.getBytes(), sAlgorithm);
        try {
            IvParameterSpec iv = new IvParameterSpec(passwd.getBytes("UTF-8"));
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.DECRYPT_MODE, ss, iv);
            byte bt[] = cipher.doFinal(buf);
            return new String(bt);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    private static SecretKey getKey(String passwd) {
        try {
            KeyGenerator generator = KeyGenerator.getInstance(sAlgorithm);
            SecureRandom secureRandom = SecureRandom.getInstance("SHA1PRNG");
            secureRandom.setSeed(passwd.getBytes());
            generator.init(128, secureRandom);
            return generator.generateKey();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return null;
    }

    private static String byteToHexString(byte[] buf) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < buf.length; i++) {
            String s = Integer.toHexString(buf[i] & 0xFF);
            if (s.length() == 1) {
                s = '0' + s;
            }
            sb.append(s.toUpperCase());
        }

        return sb.toString();
    }

    private static byte[] hexStringToByte(String hexStr) {
        if (hexStr == null || hexStr.equals("")) {
            return null;
        }

        hexStr = hexStr.toUpperCase();
        int length = hexStr.length() / 2;
        char[] hexChars = hexStr.toCharArray();
        byte[] d = new byte[length];
        for (int i = 0; i < length; i++) {
            int pos = i * 2;
            d[i] = (byte) (charToByte(hexChars[pos]) << 4 | charToByte(hexChars[pos + 1]));
        }
        return d;
    }

    private static byte charToByte(char c) {
        int index = "0123456789abcdef".indexOf(c);
        if (index == -1) {
            index = "0123456789ABCDEF".indexOf(c);
        }
        return (byte)index;
    }

}