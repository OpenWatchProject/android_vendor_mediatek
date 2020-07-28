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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.LinkedList;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;

import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Environment;
import android.os.SystemProperties;
import android.text.TextUtils;

import com.android.ims.ImsManager;
import com.mediatek.ims.rcsua.Client;
import com.mediatek.ims.rcsua.RcsUaService;
import com.orangelabs.rcs.provider.settings.RcsSettingsData;

import com.orangelabs.rcs.core.ims.service.ContactInfo;
import com.orangelabs.rcs.core.ims.service.capability.Capabilities;
import com.orangelabs.rcs.service.UaServiceManager;
import com.orangelabs.rcs.utils.logger.Logger;
import com.orangelabs.rcs.service.LauncherUtils;
import java.security.spec.KeySpec;
import java.util.ArrayList;
import java.util.LinkedList;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;

import com.gsma.services.rcs.Intents;
import com.gsma.services.rcs.ft.FileTransferServiceConfiguration.ImageResizeOption;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMode;

import android.content.SharedPreferences;
import android.app.Activity;
import android.content.Intent;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.provider.eab.RichAddressBookProvider;
import com.orangelabs.rcs.provider.messaging.ChatProvider;
import com.orangelabs.rcs.provider.messaging.FileTransferProvider;
import com.orangelabs.rcs.provider.sharing.ImageSharingProvider;
import com.orangelabs.rcs.provider.sharing.VideoSharingProvider;


import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMethod;
import com.gsma.services.rcs.CommonServiceConfiguration.MessagingMode;
import com.gsma.services.rcs.CommonServiceConfiguration.MinimumBatteryLevel;

import com.mediatek.telephony.MtkTelephonyManagerEx;

import com.orangelabs.rcs.provider.settings.RcsSettingsData.EnableRcseSwitch;
import com.orangelabs.rcs.utils.ContactUtil;

/**
 * RCS settings
 *
 * @author jexa7410
 */
public class RcsSettings {

    private static final int MAX_BYTE_COUNT = 256;
    public final static String CONFIGURATION_STATUS = "com.orangelabs.rcs.CONFIGURATION_STATUS_TO_APP";
    public static final String CORE_CONFIGURATION_STATUS = "status";
    private final static String ALGORITHM = "AES";
    private final static String keyCipher = "super-secret-key-0123123451";
    private final static String TAG = "RcsSettings";
    private final static Logger LOGGER = Logger.getLogger(TAG);
    private static RcsSettings instance = null;
    private ContentResolver cr;
    private Uri databaseUri = RcsSettingsData.CONTENT_URI;

    private final static String BLANK = "";
    private final static String DEFAULT_VALUE = "";
    private final static String RECONFIG_VERSION_VALUE = "0";
    private final static String FORBIDDEN_VERSION_VALUE = "-1";
    private final static int RECONFIG_VALIDITY_VALUE = 0;
    private final static int FORBIDDEN_VALIDITY_VALUE = -1;
    private static String PROFILE_VALUE = null;

    private int issupportOP08 = -1;

    private static final int DEFAULT_MAX_NUMBER_OF_PRESENCE_SUBSCRIPTIONS = 100;
    private boolean mCurrentFTCapability = false;
    public boolean mIsEncrypted = false;
    private final static String OP08_OPERATOR_NAME = "T-Mobile";
    private static Context mContext = null;

    /**
     * Create instance
     *
     * @param ctx Context
     */
    public static synchronized void createInstance(Context ctx) {
        if (instance == null) {
            instance = new RcsSettings(ctx);
            mContext = ctx;
        }
    }

    /**
     * Returns instance
     *
     * @return Instance
     */
    public static RcsSettings getInstance() {
        if (instance == null) {
            createInstance(AndroidFactory.getApplicationContext());
        }
        if (instance == null) {
            if (LOGGER.isActivated()) {
                LOGGER.error("getInstance(): instance is still NULL!");
            }
        }
        return instance;
    }

    /**
     * Constructor
     *
     * @param ctx Application context
     */
    private RcsSettings(Context ctx) {
        super();

        this.cr = ctx.getContentResolver();
    }

    /**
     * Read a parameter
     *
     * @param key Key
     * @return Value
     */
    public String readParameter(String key) {
        if (key == null) {
            return null;
        }
        if (LOGGER.isActivated()) {
          //  LOGGER.debug(" Key before Encryption "+ key );
        }
        if(mIsEncrypted) {
            try {
                key=cipher(keyCipher, key);
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }
        String result = null;

        Cursor c = cr.query(databaseUri, null, RcsSettingsData.KEY_KEY + "='" + key + "'", null, null);

        if (c != null) {
            if ((c.getCount() > 0) && c.moveToFirst()) {
                result = c.getString(2);
            }
            c.close();
            if(mIsEncrypted) {
                try {
                    if(result!=null) {
                        result = decipher(keyCipher, result);
                    }
                    key=decipher(keyCipher, key);
                } catch (Exception e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
        }

        return result;
    }

    /**
     * Write a parameter
     *
     * @param key Key
     * @param value Value
     */
    public void writeParameter(String key, String value) {
        if ((key == null) || (value == null)) {
            return;
        }
        if(mIsEncrypted)
        {
        try {
            if(value!=null){
            value = cipher(keyCipher, value);
            }
            key=cipher(keyCipher, key);
        } catch (Exception e) {
            e.printStackTrace();
        }
        }
        ContentValues values = new ContentValues();
        values.put(RcsSettingsData.KEY_VALUE, value);
        String where = RcsSettingsData.KEY_KEY + "='" + key + "'";

        try {
            cr.update(databaseUri, values, where, null);
        } catch (android.database.sqlite.SQLiteException ex) {
            // after over-night tests, sometimes can't open database
            ex.printStackTrace();
        }

    }

    /**
     * Insert a parameter
     *
     * @param key Key
     * @param value Value
     */
    public void insertParameter(String key, String value) {
        if ((key == null) || (value == null)) {
            return;
        }
        if(mIsEncrypted)
        {
        try {
            if(value!=null)
            {
                value = cipher(keyCipher, value);
            }
            key=cipher(keyCipher, key);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        }
        ContentValues values = new ContentValues();
        values.put(RcsSettingsData.KEY_KEY, key);
        values.put(RcsSettingsData.KEY_VALUE, value);
        cr.insert(databaseUri, values);
    }


     public void deleteParameter(String key) {
        if ((key == null) ) {
            return;
        }
        if(mIsEncrypted)
        {
        try {

            key=cipher(keyCipher, key);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        }
        String where= RcsSettingsData.KEY_KEY + " LIKE '%" + key +"%' ";
        ContentValues values = new ContentValues();
            values.put(RcsSettingsData.KEY_KEY, key);
    //      values.put(RcsSettingsData.KEY_VALUE, value);
            cr.delete(databaseUri, where,null);
    }

 /**
      * Is RCS service registered
      *
      * @return Boolean
      */
     public boolean isServiceRegistered() {
         boolean result = false;
         if (instance != null) {
             result = Boolean.parseBoolean(readParameter(RcsSettingsData.REGISTRATION_STATUS));
         }
         return result;
     }

     /**
      * Set the RCS service registration state
      *
      * @param state State
      */
     public void setRegistrationState(boolean state) {
         if (instance != null) {
             writeParameter(RcsSettingsData.REGISTRATION_STATUS, Boolean.toString(state));

         }

     }


     /**
      * Is RCS First Launch
      *
      * @return Boolean
      */
     public boolean isFirstLaunch() {
         boolean result = false;
         if (instance != null) {
             result = Boolean.parseBoolean(readParameter(RcsSettingsData.LAUNCH_STATUS));
         }
         return result;
     }

     /**
      * Set the RCS First Launch
      *
      * @param state State
      */
     public void setFristLaunchState(boolean state) {
         if (instance != null) {
             writeParameter(RcsSettingsData.LAUNCH_STATUS, Boolean.toString(state));

         }

     }

    /**
     * Is RCS service activated
     *
     * @return Boolean
     */
    public boolean isServiceActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SERVICE_ACTIVATED));
            if (LOGGER.isActivated()) {
                LOGGER.error("Service Activated through instance not null:-" + result);
            }
        }
        if (LOGGER.isActivated()) {
            LOGGER.error("Service Activated end:-" + result);
        }
        return result;
    }

    /**
     * Is TMO Load
     *
     * @return Boolean
     */
    public boolean isSupportOP08() {
        boolean result = false;
        result = isOP08SupportedByPlatform();
        return result;
    }

    /**
     * Is ATT Load
     *
     * @return Boolean
     */
    public boolean isSupportOP07() {
        boolean status = false;
        String optr = SystemProperties.get("persist.vendor.operator.optr");
        if (optr.equalsIgnoreCase("op07")) {
            status = true;
        }
        return status;
    }

    public boolean isFromUriSupported(){
        return isSupportOP07();
    }

    public boolean isPAccessNetworkInfoSupported(){
        return isSupportOP07();
    }

    /**
     * Set the RCS service activation state
     *
     * @param state State
     */
    public void setServiceActivationState(boolean state) {
        if (instance != null) {
            writeParameter(RcsSettingsData.SERVICE_ACTIVATED, Boolean.toString(state));
            // TODO: service activation property
            if (UaServiceManager.getInstance().getService() == null)
                UaServiceManager.getInstance().startService(AndroidFactory.getApplicationContext(),
                    LauncherUtils.getCurrentUserPhoneId());
            if (state) {
                UaServiceManager.getInstance().activate();
            } else {
                UaServiceManager.getInstance().deactivate();
            }
        }
    }

    /**
     * Is RCS service authorized in roaming
     *
     * @return Boolean
     */
    public boolean isRoamingAuthorized() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.ROAMING_AUTHORIZED));
        }
        return result;
    }

    /**
     * Is RCS Only service authorized in roaming
     *
     * @return Boolean
     */
    public boolean isRcsOnlyRoamingAuthorized() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.RCS_ONLY_ROAMING_AUTHORIZED));
        }
        return result;
    }

    /**
     * Set the roaming authorization state
     *
     * @param state State
     */
    public void setRoamingAuthorizationState(boolean state) {
        if (instance != null) {
            writeParameter(RcsSettingsData.ROAMING_AUTHORIZED, Boolean.toString(state));
        }
    }

    /**
     * Set the Rcs only roaming authorization state
     *
     * @param state State
     */
    public void setRcsOnlyRoamingAuthorizationState(boolean state) {
        if (instance != null) {
            writeParameter(RcsSettingsData.RCS_ONLY_ROAMING_AUTHORIZED, Boolean.toString(state));
        }
    }
    /**
     * Get the ringtone for presence invitation
     *
     * @return Ringtone URI or null if there is no ringtone
     */
    public String getPresenceInvitationRingtone() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.PRESENCE_INVITATION_RINGTONE);
        }
        return result;
    }

    /**
     * Set the presence invitation ringtone
     *
     * @param uri Ringtone URI
     */
    public void setPresenceInvitationRingtone(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.PRESENCE_INVITATION_RINGTONE, uri);
        }
    }

    /**
     * Is phone vibrate for presence invitation
     *
     * @return Boolean
     */
    public boolean isPhoneVibrateForPresenceInvitation() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.PRESENCE_INVITATION_VIBRATE));
        }
        return result;
    }

    /**
     * Set phone vibrate for presence invitation
     *
     * @param vibrate Vibrate state
     */
    public void setPhoneVibrateForPresenceInvitation(boolean vibrate) {
        if (instance != null) {
            writeParameter(RcsSettingsData.PRESENCE_INVITATION_VIBRATE, Boolean.toString(vibrate));
        }
    }

    /**
     * Get the ringtone for CSh invitation
     *
     * @return Ringtone URI or null if there is no ringtone
     */
    public String getCShInvitationRingtone() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.CSH_INVITATION_RINGTONE);
        }
        return result;
    }

    /**
     * Set the CSh invitation ringtone
     *
     * @param uri Ringtone URI
     */
    public void setCShInvitationRingtone(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CSH_INVITATION_RINGTONE, uri);
        }
    }

    /**
     * Is phone vibrate for CSh invitation
     *
     * @return Boolean
     */
    public boolean isPhoneVibrateForCShInvitation() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CSH_INVITATION_VIBRATE));
        }
        return result;
    }

    /**
     * Set phone vibrate for CSh invitation
     *
     * @param vibrate Vibrate state
     */
    public void setPhoneVibrateForCShInvitation(boolean vibrate) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CSH_INVITATION_VIBRATE, Boolean.toString(vibrate));
        }
    }

    /**
     * Is phone beep if the CSh available
     *
     * @return Boolean
     */
    public boolean isPhoneBeepIfCShAvailable() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CSH_AVAILABLE_BEEP));
        }
        return result;
    }

    /**
     * Set phone beep if CSh available
     *
     * @param beep Beep state
     */
    public void setPhoneBeepIfCShAvailable(boolean beep) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CSH_AVAILABLE_BEEP, Boolean.toString(beep));
        }
    }

         /**
     * Get the CSh video format
     *
     * @return Video format as string
     */
    public String getCShVideoFormat() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.CSH_VIDEO_FORMAT);
        }
        return result;
    }

    /**
     * Set the CSh video format
     *
     * @param fmt Video format
     */
    public void setCShVideoFormat(String fmt) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CSH_VIDEO_FORMAT, fmt);
        }
    }

    /**
     * Get the CSh video size
     *
     * @return Size (e.g. QCIF, QVGA)
     */
    public String getCShVideoSize() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.CSH_VIDEO_SIZE);
        }
        return result;
    }

    /**
     * Set the CSh video size
     *
     * @param size Video size
     */
    public void setCShVideoSize(String size) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CSH_VIDEO_SIZE, size);
        }
    }

    /**
     * Get the ringtone for file transfer invitation
     *
     * @return Ringtone URI or null if there is no ringtone
     */
    public String getFileTransferInvitationRingtone() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FILETRANSFER_INVITATION_RINGTONE);
        }
        return result;
    }

    /**
     * Set the file transfer invitation ringtone
     *
     * @param uri Ringtone URI
     */
    public void setFileTransferInvitationRingtone(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FILETRANSFER_INVITATION_RINGTONE, uri);
        }
    }

    /**
     * Is phone vibrate for file transfer invitation
     *
     * @return Boolean
     */
    public boolean isPhoneVibrateForFileTransferInvitation() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.FILETRANSFER_INVITATION_VIBRATE));
        }
        return result;
    }

    /**
     * Set phone vibrate for file transfer invitation
     *
     * @param vibrate Vibrate state
     */
    public void setPhoneVibrateForFileTransferInvitation(boolean vibrate) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FILETRANSFER_INVITATION_VIBRATE, Boolean.toString(vibrate));
        }
    }

    /**
     * Get the ringtone for chat invitation
     *
     * @return Ringtone URI or null if there is no ringtone
     */
    public String getChatInvitationRingtone() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.CHAT_INVITATION_RINGTONE);
        }
        return result;
    }

    /**
     * Set the chat invitation ringtone
     *
     * @param uri Ringtone URI
     */
    public void setChatInvitationRingtone(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CHAT_INVITATION_RINGTONE, uri);
        }
    }

    /**
     * Is phone vibrate for chat invitation
     *
     * @return Boolean
     */
    public boolean isPhoneVibrateForChatInvitation() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CHAT_INVITATION_VIBRATE));
        }
        return result;
    }

    /**
     * Set phone vibrate for chat invitation
     *
     * @param vibrate Vibrate state
     */
    public void setPhoneVibrateForChatInvitation(boolean vibrate) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CHAT_INVITATION_VIBRATE, Boolean.toString(vibrate));
        }
    }

    /**
     * Is send displayed notification activated
     *
     * @return Boolean
     */
    public boolean isImDisplayedNotificationActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CHAT_DISPLAYED_NOTIFICATION));
        }
        return result;
    }

    /**
     * Group Messaging toggle value
     *
     * @return Boolean
     */
    public boolean isGroupMessagingActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.GROUP_MESSAGING_TOGGLE));
        }
        return result;
    }

    /**
     * Set send displayed notification
     *
     * @param state
     */
    public void setImDisplayedNotificationActivated(boolean state) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CHAT_DISPLAYED_NOTIFICATION, Boolean.toString(state));
        }
    }

    /**
     * Get the pre-defined freetext 1
     *
     * @return String
     */
    public String getPredefinedFreetext1() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FREETEXT1);
        }
        return result;
    }

    /**
     * Set the pre-defined freetext 1
     *
     * @param txt Text
     */
    public void setPredefinedFreetext1(String txt) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FREETEXT1, txt);
        }
    }

    /**
     * Get the pre-defined freetext 2
     *
     * @return String
     */
    public String getPredefinedFreetext2() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FREETEXT2);
        }
        return result;
    }

    /**
     * Set the pre-defined freetext 2
     *
     * @param txt Text
     */
    public void setPredefinedFreetext2(String txt) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FREETEXT2, txt);
        }
    }

    /**
     * Get the pre-defined freetext 3
     *
     * @return String
     */
    public String getPredefinedFreetext3() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FREETEXT3);
        }
        return result;
    }

    /**
     * Set the pre-defined freetext 3
     *
     * @param txt Text
     */
    public void setPredefinedFreetext3(String txt) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FREETEXT3, txt);
        }
    }

    /**
     * Get the pre-defined freetext 4
     *
     * @return String
     */
    public String getPredefinedFreetext4() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FREETEXT4);
        }
        return result;
    }

    /**
     * Set the pre-defined freetext 4
     *
     * @param txt Text
     */
    public void setPredefinedFreetext4(String txt) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FREETEXT4, txt);
        }
    }

    /**
     * Get the min battery level
     *
     * @return Battery level in percentage
     */
    public int getMinBatteryLevel() {
        int result = 0;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MIN_BATTERY_LEVEL));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Set the min battery level
     *
     * @param level Battery level in percentage
     */
    public void setMinBatteryLevel(int level) {
        if (instance != null) {
            writeParameter(RcsSettingsData.MIN_BATTERY_LEVEL, "" + level);
        }
    }

    /**
     * Get the min storage capacity
     *
     * @return Capacity in kilobytes
     */
    public int getMinStorageCapacity() {
        int result = 0;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MIN_STORAGE_CAPACITY));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Set the min storage capacity
     *
     * @param capacity Capacity in kilobytes
     */
    public void setMinStorageCapacity(int capacity) {
        if (instance != null) {
            writeParameter(RcsSettingsData.MIN_STORAGE_CAPACITY, "" + capacity);
        }
    }

    /**
     * Get user profile username (i.e. username part of the IMPU)
     *
     * @return Username part of SIP-URI
     */
    public String getUserProfileImsUserName() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.USERPROFILE_IMS_USERNAME);
        }
        return result;
    }

    /**
     * Set user profile IMS username (i.e. username part of the IMPU)
     *
     * @param value Value
     */
    public void setUserProfileImsUserName(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.USERPROFILE_IMS_USERNAME, value);
        }
    }

    /**
     * Get the value of the MSISDN
     *
     * @return MSISDN
     */
    public String getMsisdn() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.MSISDN);
        }
        return result;
    }

    /**
     * Set the value of the MSISDN
     */
    public void setMsisdn(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.MSISDN, value);
        }
    }

    /**
     * Get user profile IMS display name associated to IMPU
     *
     * @return String
     */
    public String getUserProfileImsDisplayName() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.USERPROFILE_IMS_DISPLAY_NAME);
        }
        return result;
    }

    /**
     * Set user profile IMS display name associated to IMPU
     *
     * @param value Value
     */
    public void setUserProfileImsDisplayName(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.USERPROFILE_IMS_DISPLAY_NAME, value);
        }
    }

    /**
     * Get user profile IMS private Id (i.e. IMPI)
     *
     * @return SIP-URI
     */
    public String getUserProfileImsPrivateId() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.USERPROFILE_IMS_PRIVATE_ID);
        }
        return result;
    }

    /**
     * Set user profile IMS private Id (i.e. IMPI)
     *
     * @param uri SIP-URI
     */
    public void setUserProfileImsPrivateId(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.USERPROFILE_IMS_PRIVATE_ID, uri);
        }
    }

    /**
     * Get user profile IMS password
     *
     * @return String
     */
    public String getUserProfileImsPassword() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.USERPROFILE_IMS_PASSWORD);
        }
        return result;
    }

    /**
     * Set user profile IMS password
     *
     * @param pwd Password
     */
    public void setUserProfileImsPassword(String pwd) {
        if (instance != null) {
            writeParameter(RcsSettingsData.USERPROFILE_IMS_PASSWORD, pwd);
        }
    }

    /**
     * Get user profile IMS realm
     *
     * @return String
     */
    public String getUserProfileImsRealm() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.USERPROFILE_IMS_REALM);
        }
        return result;
    }

    /**
     * Set user profile IMS realm
     *
     * @param realm Realm
     */
    public void setUserProfileImsRealm(String realm) {
        if (instance != null) {
            writeParameter(RcsSettingsData.USERPROFILE_IMS_REALM, realm);
        }
    }

    /**
     * Get user profile IMS home domain
     *
     * @return Domain
     */
    public String getUserProfileImsDomain() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.USERPROFILE_IMS_HOME_DOMAIN);
        }
        return result;
    }

    /**
     * Set user profile IMS home domain
     *
     * @param domain Domain
     */
    public void setUserProfileImsDomain(String domain) {
        if (instance != null) {
            writeParameter(RcsSettingsData.USERPROFILE_IMS_HOME_DOMAIN, domain);
        }
    }


    /**
     * Get IMS proxy address for mobile access
     *
     * @return Address
     */
    public String getImsProxyAddrForMobile() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.IMS_PROXY_ADDR_MOBILE);
        }
        return result;
    }

    /**
     * Set IMS proxy address for mobile access
     *
     * @param addr Address
     */
    public void setImsProxyAddrForMobile(String addr) {
        if (instance != null) {
            writeParameter(RcsSettingsData.IMS_PROXY_ADDR_MOBILE, addr);
        }
    }

    /**
     * Get IMS proxy port for mobile access
     *
     * @return Port
     */
    public int getImsProxyPortForMobile() {
        int result = 5060;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.IMS_PROXY_PORT_MOBILE));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Set IMS proxy port for mobile access
     *
     * @param port Port number
     */
    public void setImsProxyPortForMobile(int port) {
        if (instance != null) {
            writeParameter(RcsSettingsData.IMS_PROXY_PORT_MOBILE, "" + port);
        }
    }

    /**
     * Get IMS proxy address for Wi-Fi access
     *
     * @return Address
     */
    public String getImsProxyAddrForWifi() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.IMS_PROXY_ADDR_WIFI);
        }
        return result;
    }

    /**
     * Set IMS proxy address for Wi-Fi access
     *
     * @param addr Address
     */
    public void setImsProxyAddrForWifi(String addr) {
        if (instance != null) {
            writeParameter(RcsSettingsData.IMS_PROXY_ADDR_WIFI, addr);
        }
    }

    /**
     * Get IMS proxy address for mobile access
     *
     * @return Address
     */
    public String[] getAllImsProxyAddrForMobile() {
        int count= getNumberOfPCssfAddresses();
        String [] result = new String[count];
        for (int i=0; i<count;i++)
        {
        if (instance != null) {
            int sufix=i+1;
            String key= "ImsOutboundProxyAddrForMobile"+sufix;
                        if(i==0)
                            {
                             key="ImsOutboundProxyAddrForMobile";
                            }
            result[i] = readParameter(key);

        }
        }
        return result;
    }

    public String getImsProxyAddrForMobile(int wch) {


        if (instance != null) {

                String key= "ImsOutboundProxyAddrForMobile"+wch;
                                if(wch==0)
                                    {
                                     key="ImsOutboundProxyAddrForMobile";
                                     }
                return readParameter(key);

        }


        return null;
    }

         /**
     * Set IMS proxy address for mobile access
     *
     * @param addr Address
     */
    public void setImsProxyAddrForMobile(String addr, int wch ) {
        if (instance != null) {
            String key= "ImsOutboundProxyAddrForMobile"+wch;
            if(wch==0)
                            {
                             key="ImsOutboundProxyAddrForMobile";
                            }
            writeParameter(key,addr);

        }
    }

         /**
     * Set IMS proxy address for mobile access
     *
     * @param addr Address
     */

    public void delAllImsProxyAddrForMobile( ) {

        int count= getNumberOfPCssfAddresses();
        writeParameter("ImsOutboundProxyAddrForMobile","");
        for (int i=1; i<count;i++)
        {
        if (instance != null) {
            int sufix=i+1;
            String key= "ImsOutboundProxyAddrForMobile"+sufix;

            deleteParameter(key);

        }
    }
    }

        /**
     * Get IMS proxy port for mobile access
     *
     * @return Port
     */
    public int[] getAllImsProxyPortForMobile() {
    //    int [] result = {5060,5060,5060,5060,5060};

        int count= getNumberOfPCssfAddresses();
        int [] result = new int[count];
        for (int i=0; i<count;i++)
        {
        if (instance != null) {
            int sufix=i+1;
            String key= "ImsOutboundProxyPortForMobile"+sufix;
                        if(i==0)
                            {
                             key="ImsOutboundProxyPortForMobile";
                            }
            if(readParameter(key)!=null){
                            try {
                result[i] = Integer.parseInt(readParameter(key));
                            } catch(Exception e) {}
            }
            else{
                result[i] = 5060;
            }
        }
        }
        return result;
    }

         public int getImsProxyPortForMobile(int wch) {
        int result = 5060;
        if (instance != null) {
            try {
                String key= "ImsOutboundProxyPortForMobile"+wch;
                                if(wch==0)
                                    {
                                     key="ImsOutboundProxyPortForMobile";
                                    }
                return Integer.parseInt(readParameter(key));

            } catch(Exception e) {}
        }
        return result;
    }

          /**
     * Set IMS proxy port for mobile access
     *
     * @param port Port number
     */
    public void setImsProxyPortForMobile(int port, int wch) {
        if (instance != null) {
            String key= "ImsOutboundProxyPortForMobile"+wch;
                        if(wch==0)
                            {
                             key="ImsOutboundProxyPortForMobile";
                            }

                writeParameter(key, "" + port);


        }
    }

    public void delAllImsProxyPortForMobile() {


        int count= getNumberOfPCssfAddresses();
        writeParameter("ImsOutboundProxyPortForMobile","");
        for (int i=0; i<count;i++)
        {
        if (instance != null) {
                int sufix=i+1;
                String key= "ImsOutboundProxyPortForMobile"+sufix;

                deleteParameter(key);
            }
        }
        }

         /**
     * Get IMS proxy address for Wi-Fi access
     *
     * @return Address
     */
    public String[] getAllImsProxyAddrForWifi() {


        int count= getNumberOfPCssfAddresses();
        String [] result = new String[count];
        for (int i=0; i<count;i++)
        {
        if (instance != null) {
            int sufix=i+1;
            String key= "ImsOutboundProxyAddrForWifi"+sufix;
                        if(i==0)
                            {
                             key="ImsOutboundProxyAddrForWifi";
                            }
                        if (LOGGER.isActivated()) {
                            LOGGER.debug("For wifi interface"+ key );
                        }
            result[i] = readParameter(key);
                            if (LOGGER.isActivated()) {
                                LOGGER.debug("For wifi interface"+ key +":"+result[i]);
                            }

        }
        }

        return result;
    }



         /**
     * Set IMS proxy address for Wi-Fi access
     *
     * @param addr Address
     */
    public void setImsProxyAddrForWifi(String addr, int wch) {
        if (instance != null) {
                String key= "ImsOutboundProxyAddrForMobile"+wch;
                    if(wch==0)
                            {
                             key="ImsOutboundProxyAddrForMobile";
                            }
                writeParameter(key,addr);

        }
    }

          /**
     * Set IMS proxy address for Wi-Fi access
     *
     * @param addr Address
     */
    public void delAllImsProxyPortForWifi() {
        int count= getNumberOfPCssfAddresses();
        writeParameter("ImsOutboundProxyPortForWifi","");
        for (int i=1; i<count;i++)
        {
        if (instance != null) {
                int sufix=i+1;
                String key= "ImsOutboundProxyPortForWifi"+sufix;

                //writeParameter(key, "" + port);
                deleteParameter(key);
            }
        }
    }

    /**
     * Set IMS proxy address for Wi-Fi access
     *
     * @param addr Address
     */

    public void delAllImsProxyAddrForWifi( ) {

        int count= getNumberOfPCssfAddresses();
        writeParameter("ImsOutboundProxyAddrForWifi","");
        for (int i=1; i<count;i++)
        {
        if (instance != null) {
                int sufix=i+1;
                String key= "ImsOutboundProxyAddrForWifi"+sufix;

                deleteParameter(key);
            }
        }

    }

     /** M: ftAutAccept @{ */
    /**
     * Whether it is enable to auto accept ft when roaming.
     *
     * @return whether it is enable.
     */
    public Boolean isEnableFtAutoAcceptWhenRoaming() {
        Boolean result = Boolean.FALSE;
        if (instance != null) {
            result = Boolean
                    .parseBoolean(readParameter(RcsSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING));
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("isEnableFtAutoAcceptWhenRoaming() result: " + result);
        }
        return result;
    }

/**
     * Enable or disable to auto-accept ft when roaming
     *
     * @param enable True to be enable, otherwise false.
     */
    public void setEnableFtAutoAcceptWhenRoaming(Boolean enable) {
        if (instance != null) {
            writeParameter(RcsSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING, Boolean.toString(enable));
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("setEnableFtAutoAcceptWhenRoaming() enable: " + enable);
        }
    }

    /**
     * Enable or disable to auto-accept ft when roaming
     *
     * @param enable True to be enable, otherwise false.
     */
    public void setEnableFtAutoAccept(Boolean enable) {
        if (instance != null) {
            writeParameter(RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER, Boolean.toString(enable));
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("setEnableFtAutoAcceptWhenRoaming() enable: " + enable);
        }
    }

        /** M: ftAutAccept no roaming @{ */
    /**
     * Whether it is enable to auto accept ft when no roaming.
     *
     * @return whether it is enable.
     */
    public Boolean isEnableFtAutoAcceptWhenNoRoaming() {
        Boolean result = Boolean.FALSE;
        if (instance != null) {
            result = Boolean
                    .parseBoolean(readParameter(RcsSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING));
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("isEnableFtAutoAcceptWhenNoRoaming() result: " + result);
        }
        return result;
    }

    /**
     * Enable or disable to auto-accept ft when no roaming
     *
     * @param enable True to be enable, otherwise false.
     */
    public void setEnableFtAutoAcceptWhenNoRoaming(Boolean enable) {
        if (instance != null) {
            writeParameter(RcsSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING, Boolean.toString(enable));
    }
        if (LOGGER.isActivated()) {
            LOGGER.debug("setEnableFtAutoAcceptWhenNoRoaming() enable: " + enable);
        }
    }


     /* Get IMS proxy port for Wi-Fi access
     *
     * @return Port
     */
    public int getImsProxyPortForWifi() {
        int result = 5060;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.IMS_PROXY_PORT_WIFI));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Set IMS proxy port for Wi-Fi access
     *
     * @param port Port number
     */
    public void setImsProxyPortForWifi(int port) {
        if (instance != null) {
            writeParameter(RcsSettingsData.IMS_PROXY_PORT_WIFI, "" + port);
        }
    }

    /**
      * M: Multiple Pcscf Entries
     */

      /** @}*/
    /**
     * Get IMS proxy port for Wi-Fi access
     *
     * @return Port
     */
    public int[] getAllImsProxyPortForWifi() {

        int count= getNumberOfPCssfAddresses();
        int [] result = new int[count];
        for (int i=0; i<count;i++)
        {
        if (instance != null) {
            int sufix=i+1;
            String key= "ImsOutboundProxyPortForWifi"+sufix;
                        if(i==0)
                        {key= "ImsOutboundProxyPortForWifi";}
            if(readParameter(key)!=null){
            try {
                    result[i] = Integer.parseInt(readParameter(key));
            } catch(Exception e) {}
        }
            else{

                result[i] = 5060;
            }
        }
        }
        return result;
    }


/**
     * Set IMS proxy port for Wi-Fi access
     *
     * @param port Port number
     */
    public void setImsProxyPortForWifi(int port, int wch) {

        if (instance != null) {
            String key= "ImsOutboundProxyPortForMobile"+wch;
            if(wch==0)
                            {
                                key="ImsOutboundProxyPortForMobile";
                            }
                writeParameter(key, "" + port);


        }
    }
    /*@*/


/**
     * M: settings for msrp protocol@{
     */
    /**
     * Get msrp protocol for mobile.
     *
     * @return The msrp protocol.
     */
    public String getMsrpProtocolForMobile() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.MSRP_PROTOCOL_FOR_MOBILE);
        }
        return result;
    }

        /**
     * Get XDM server address
     *
     * @return Address as <host>:<port>/<root>
     */
    public String getXdmServer() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.XDM_SERVER);
        }
        return result;
    }


/**
     * Set msrp protocol for mobile.
     *
     * @param protocol The msrp protocol.
     */
    public void setMsrpProtocolForMobile(String protocol) {
        if (instance != null) {
            writeParameter(RcsSettingsData.MSRP_PROTOCOL_FOR_MOBILE, protocol);
        }
    }

    /**
     * Get msrp protocol for wifi.
     *
     * @return The msrp protocol.
     */
    public String getMsrpProtocolForWifi() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.MSRP_PROTOCOL_FOR_WIFI);
        }
        return result;
    }

    /**
     * Set msrp protocol for wifi.
     *
     * @param protocol The msrp protocol.
     */
    public void setMsrpProtocolForWifi(String protocol) {
        if (instance != null) {
            writeParameter(RcsSettingsData.MSRP_PROTOCOL_FOR_WIFI, protocol);
        }
    }

    /**
     * Set XDM server address
     *
     * @param addr Address as <host>:<port>/<root>
     */
    public void setXdmServer(String addr) {
        if (instance != null) {
            writeParameter(RcsSettingsData.XDM_SERVER, addr);
        }
    }

    /**
     * Get XDM server login
     *
     * @return String value
     */
    public String getXdmLogin() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.XDM_LOGIN);
        }
        return result;
    }

    /**
     * Set XDM server login
     *
     * @param value Value
     */
    public void setXdmLogin(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.XDM_LOGIN, value);
        }
    }

    /**
     * Get XDM server password
     *
     * @return String value
     */
    public String getXdmPassword() {
        String result = null;
        if(isOP08SupportedByPlatform()) {
            return "";
        }
        if (instance != null) {
            result = readParameter(RcsSettingsData.XDM_PASSWORD);
        }
        return result;
    }

    /**
     * Set XDM server password
     *
     * @param value Value
     */
    public void setXdmPassword(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.XDM_PASSWORD, value);
        }
    }

    /**
     * Get file transfer HTTP server address
     *
     * @return Address
     */
    public String getFtHttpServer() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FT_HTTP_SERVER);
        }
        return result;
    }

    /**
     * Set file transfer HTTP server address
     *
     * @param addr Address
     */
    public void setFtHttpServer(String addr) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FT_HTTP_SERVER, addr);
        }
    }

    /**
     * Get file transfer HTTP server login
     *
     * @return String value
     */
    public String getFtHttpLogin() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FT_HTTP_LOGIN);
        }
        return result;
    }

    /**
     * Set file transfer HTTP server login
     *
     * @param value Value
     */
    public void setFtHttpLogin(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FT_HTTP_LOGIN, value);
        }
    }

    /**
     * Get file transfer HTTP server password
     *
     * @return String value
     */
    public String getFtHttpPassword() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FT_HTTP_PASSWORD);
        }
        return result;
    }

    /**
     * Set file transfer HTTP server password
     *
     * @param value Value
     */
    public void setFtHttpPassword(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FT_HTTP_PASSWORD, value);
        }
    }

    /**
     * Get file transfer protocol
     *
     * @return String value
     */
    public String getFtProtocol() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.FT_PROTOCOL);
        }
        return result;
    }

    /**
     * Set file transfer protocol
     *
     * @param value Value
     */
    public void setFtProtocol(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FT_PROTOCOL, value);
        }
    }

    /**
     * Get IM conference URI
     *
     * @return SIP-URI
     */
    public String getImConferenceUri() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.IM_CONF_URI);
        }
        if(isSupportOP07()){
            String conferenceUri =  "sip:n-way_messaging@one.att.net";
            return conferenceUri;
        }
        return result;
    }

    /**
     * Set IM conference URI
     *
     * @param uri SIP-URI
     */
    public void setImConferenceUri(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.IM_CONF_URI, uri);
        }
    }

    /**
     * Get IM Multi conference URI
     *
     * @return SIP-URI
     */
    public String getImMultiConferenceUri() {
        String result = null;
        if(isSupportOP08() || isSupportOP07()){
            return getImConferenceUri();
        }
        if (instance != null) {
            result = readParameter(RcsSettingsData.IM_MULTI_CONF_URI);
        }
        return result;
    }

    /**
     * Set IM Multi conference URI
     *
     * @param uri SIP-URI
     */
    public void setImMultiConferenceUri(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.IM_MULTI_CONF_URI, uri);
        }
    }

    /**
     * Get end user confirmation request URI
     *
     * @return SIP-URI
     */
    public String getEndUserConfirmationRequestUri() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.ENDUSER_CONFIRMATION_URI);
        }
        return result;
    }

    /**
     * Set end user confirmation request
     *
     * @param uri SIP-URI
     */
    public void setEndUserConfirmationRequestUri(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.ENDUSER_CONFIRMATION_URI, uri);
        }
    }

    /**
     * Get country code
     *
     * @return Country code
     */
    public String getCountryCode() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.COUNTRY_CODE);
        }
        return result;
    }

    /**
     * Set country code
     *
     * @param code Country code
     */
    public void setCountryCode(String code) {
        if (instance != null) {
            writeParameter(RcsSettingsData.COUNTRY_CODE, code);
        }
    }

    /**
     * Get country area code
     *
     * @return Area code
     */
    public String getCountryAreaCode() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.COUNTRY_AREA_CODE);
        }
        return result;
    }

    /**
     * Set country area code
     *
     * @param code Area code
     */
    public void setCountryAreaCode(String code) {
        if (instance != null) {
            writeParameter(RcsSettingsData.COUNTRY_AREA_CODE, code);
        }
    }

    /**
     * Get my capabilities
     *
     * @return capability
     */
    public Capabilities getMyCapabilities(){
        Capabilities capabilities = new Capabilities();

        // Add default capabilities
        capabilities.setCsVideoSupport(isCsVideoSupported());
        capabilities.setFileTransferSupport(isFileTransferSupported());
        capabilities.setFileTransferHttpSupport(isFileTransferHttpSupported());
        capabilities.setImageSharingSupport(isImageSharingSupported());
        capabilities.setImSessionSupport(isImSessionSupported());
        capabilities.setPresenceDiscoverySupport(isPresenceDiscoverySupported());
        capabilities.setSocialPresenceSupport(isSocialPresenceSupported());
        capabilities.setVideoSharingSupport(isVideoSharingSupported());
        capabilities.setGeolocationPushSupport(isGeoLocationPushSupported());
        capabilities.setFileTransferThumbnailSupport(isFileTransferThumbnailSupported());
        capabilities.setFileTransferStoreForwardSupport(isFileTransferStoreForwardSupported());
        capabilities.setIPVoiceCallSupport(isIPVoiceCallSupported());
        capabilities.setIPVideoCallSupport(isIPVideoCallSupported());
        capabilities.setGroupChatStoreForwardSupport(isGroupChatStoreForwardSupported());
        capabilities.setSipAutomata(isSipAutomata());
        capabilities.setTimestamp(System.currentTimeMillis());

        //set burn after read capability
        capabilities.setBurnAfterRead(isCPMBurnAfterReadingSupported());

        if (singleRegistrationSupported()) {
            //if registred via single registartion, ir94 voice call is enabled
            capabilities.setIR94_VoiceCall(UaServiceManager.getInstance().imsRegistered());
            capabilities.setIR94_VideoCall(UaServiceManager.getInstance().imsRegistered() && isIR94VideoCallSupported());
            capabilities.setIR94_DuplexMode(UaServiceManager.getInstance().imsRegistered() && isIR94VideoCallSupported());
        }

        // Add extensions
        String exts = getSupportedRcsExtensions();
        if ((exts != null) && (exts.length() > 0)) {
            String[] ext = exts.split(",");
            for(int i=0; i < ext.length; i++) {
                capabilities.addSupportedExtension(ext[i]);
            }
        }

        return capabilities;
    }

    /**
     * Get max photo-icon size
     *
     * @return Size in kilobytes
     */
    public int getMaxPhotoIconSize() {
        int result = 256;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_PHOTO_ICON_SIZE));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get max freetext length
     *
     * @return Number of char
     */
    public int getMaxFreetextLength() {
        int result = 100;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_FREETXT_LENGTH));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get max number of participants in a group chat
     *
     * @return Number of participants
     */
    public int getMaxChatParticipants() {
        int result = 10;
        if(RcsSettings.getInstance().isSupportOP07()){
            result = 11;
        } else if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_CHAT_PARTICIPANTS));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get max length of a chat message
     *
     * @return Number of char
     */
    public long getMaxChatMessageLength() {
        long result = 100;
        if (instance != null) {
            try {
                result = Long.parseLong(readParameter(RcsSettingsData.MAX_CHAT_MSG_LENGTH));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get max length of a group chat message
     *
     * @return Number of char
     */
    public long getMaxGroupChatMessageLength() {
        long result = 100;
        if (instance != null) {
            try {
                result = Long.parseLong(readParameter(RcsSettingsData.MAX_GROUPCHAT_MSG_LENGTH));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get idle duration of a chat session
     *
     * @return Duration in seconds
     */
    public int getChatIdleDuration() {
        int result = 120;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.CHAT_IDLE_DURATION));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Set idle duration of a chat session
     *
     * @return Duration in seconds
     */
    public void setChatIdleDuration(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CHAT_IDLE_DURATION, value);

        }
    }

    /**
     * Get max file transfer size
     *
     * @return Size in kilobytes
     */
    public int getMaxFileTransferSize() {
        int result = 2048;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_FILE_TRANSFER_SIZE));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get warning threshold for max file transfer size
     *
     * @return Size in kilobytes
     */
    public int getWarningMaxFileTransferSize() {
        int result = 2048;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.WARN_FILE_TRANSFER_SIZE));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get warning threshold for max image sharing size
     *
     * @return Size in kilobytes
     */
    public int getWarningMaxImageSharingSize() {
        int result = 2048;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.WARN_IMAGE_SHARE_SIZE));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get max image share size
     *
     * @return Size in kilobytes
     */
    public int getMaxImageSharingSize() {
        int result = 2048;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_IMAGE_SHARE_SIZE));
            } catch(Exception e) {}
        }
        return result;
    }
     /**
     * @}
     */
    /**
     * Get max duration of a video share
     *
     * @return Duration in seconds
     */
    public int getMaxVideoShareDuration() {
        int result = 600;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_VIDEO_SHARE_DURATION));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get max number of simultaneous chat sessions
     *
     * @return Number of sessions
     */
    public int getMaxChatSessions() {
        int result = 1;
        int defaultMaxChatSession = 10;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_CHAT_SESSIONS));
            } catch(Exception e) {}
        }
        if(result == 0) return defaultMaxChatSession;
        return result;
    }

    /**
     * Get max number of simultaneous file transfer sessions
     *
     * @return Number of sessions
     */
    public int getMaxFileTransferSessions() {
        int result = 1;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_FILE_TRANSFER_SESSIONS));
            } catch(Exception e) {}
        }
        return result;
    }



    /**
     * Is SMS fallback service activated
     *
     * @return Boolean
     */
    public boolean isSmsFallbackServiceActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SMS_FALLBACK_SERVICE));
        }
        return result;
    }

    /**
     * Is chat invitation auto accepted
     *
     * @return Boolean
     */
    public boolean isChatAutoAccepted(){
        boolean result = false;
        if(isGroupChatAutoAccepted()) return true;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.AUTO_ACCEPT_CHAT));
        }
        return result;
    }

    /**
     * Is group chat invitation auto accepted
     *
     * @return Boolean
     */
    public boolean isGroupChatAutoAccepted(){
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.AUTO_ACCEPT_GROUP_CHAT));
        }
        return result;
    }

    /**
     * Is file transfer invitation auto accepted
     *
     * @return Boolean
     */
    public boolean isFileTransferAutoAccepted() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER));
        }
        return result;
    }

    /**
     * Is file transfer invitation auto accepted
     *
     * @return Boolean
     */
    public boolean isFallbackToPagerModeSupported() {
        return isSupportOP08();
    }

    /**
     * Is file transfer invitation auto accepted
     *
     * @return Boolean
     */
    public boolean isGroupChat404HandlingSupported() {
        return isSupportOP08() || isSupportOP07();
    }

    /**
     * Set the file transfer config parameter auto accept
     *
     * @param autoAccept auto accept true or false
     */
    public void setFileTransferAutoAccepted(boolean autoAccept) {
        if (instance != null) {
            writeParameter(RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER, Boolean.toString(autoAccept));
        }
    }


    /**
     * Is Store & Forward service warning activated
     *
     * @return Boolean
     */
    public boolean isStoreForwardWarningActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.WARN_SF_SERVICE));
        }
        return result;
    }

    /**
     * Get IM session start mode
     *
     * @return Integer (1: The 200 OK is sent when the receiver starts to type a message back
     * in the chat window. 2: The 200 OK is sent when the receiver sends a message)
     */
    public int getImSessionStartMode() {
        int result = 1;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.IM_SESSION_START));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get max number of entries per contact in the chat log
     *
     * @return Number
     */
    public int getMaxChatLogEntriesPerContact() {
        int result = 200;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_CHAT_LOG_ENTRIES));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get max number of entries per contact in the richcall log
     *
     * @return Number
     */
    public int getMaxRichcallLogEntriesPerContact() {
        int result = 200;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_RICHCALL_LOG_ENTRIES));
            } catch(Exception e) {}
        }
        return result;
    }



    /**
     * Get polling period used before each IMS service check (e.g. test subscription state for presence service)
     *
     * @return Period in seconds
     */
    public int getImsServicePollingPeriod(){
        int result = 300;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.IMS_SERVICE_POLLING_PERIOD));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get default SIP listening port
     *
     * @return Port
     */
    public int getSipListeningPort() {
        int result = 5060;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.SIP_DEFAULT_PORT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get default SIP protocol for mobile
     *
     * @return Protocol (udp | tcp | tls)
     */
    public String getSipDefaultProtocolForMobile() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_MOBILE);
        }
        return result;
    }

    /**
     * Get default SIP protocol for wifi
     *
     * @return Protocol (udp | tcp | tls)
     */
    public String getSipDefaultProtocolForWifi() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.SIP_DEFAULT_PROTOCOL_FOR_WIFI);
        }
        return result;
    }

    /**
     * Get TLS Certificate root
     *
     * @return Path of the certificate
     */
    public String getTlsCertificateRoot() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.TLS_CERTIFICATE_ROOT);
        }
        return result;
    }

    /**
     * Get TLS Certificate intermediate
     *
     * @return Path of the certificate
     */
    public String getTlsCertificateIntermediate() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.TLS_CERTIFICATE_INTERMEDIATE);
        }
        return result;
    }

    /**
     * Get SIP transaction timeout used to wait SIP response
     *
     * @return Timeout in seconds
     */
    public int getSipTransactionTimeout() {
        int result = 30;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.SIP_TRANSACTION_TIMEOUT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Set IM Multi conference URI
     *
     * @param uri SIP-URI
     */
    public void setSipTransactionTimeout(String timeout) {
        if (instance != null) {
            writeParameter(RcsSettingsData.SIP_TRANSACTION_TIMEOUT, timeout);
        }
    }

    /**
     * Get default MSRP port
     *
     * @return Port
     */
    public int getDefaultMsrpPort() {
        int result = 20000;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MSRP_DEFAULT_PORT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get default RTP port
     *
     * @return Port
     */
    public int getDefaultRtpPort() {
        int result = 10000;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.RTP_DEFAULT_PORT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get MSRP transaction timeout used to wait MSRP response
     *
     * @return Timeout in seconds
     */
    public int getMsrpTransactionTimeout() {
        int result = 5;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MSRP_TRANSACTION_TIMEOUT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get default expire period for REGISTER
     *
     * @return Period in seconds
     */
    public int getRegisterExpirePeriod() {
        int result = 3600;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.REGISTER_EXPIRE_PERIOD));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get registration retry base time
     *
     * @return Time in seconds
     */
    public int getRegisterRetryBaseTime() {
        int result = 30;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.REGISTER_RETRY_BASE_TIME));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get registration retry max time
     *
     * @return Time in seconds
     */
    public int getRegisterRetryMaxTime() {
        int result = 1800;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.REGISTER_RETRY_MAX_TIME));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get default expire period for PUBLISH
     *
     * @return Period in seconds
     */
    public int getPublishExpirePeriod() {
        int result = 3600;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.PUBLISH_EXPIRE_PERIOD));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get revoke timeout before to unrevoke a revoked contact
     *
     * @return Timeout in seconds
     */
    public int getRevokeTimeout() {
        int result = 300;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.REVOKE_TIMEOUT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get IMS authentication procedure for mobile access
     *
     * @return Authentication procedure
     */
    public String getImsAuhtenticationProcedureForMobile() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE);
        }
        return result;
    }

    /**
     * Get IMS authentication procedure for Wi-Fi access
     *
     * @return Authentication procedure
     */
    public String getImsAuhtenticationProcedureForWifi() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.IMS_AUTHENT_PROCEDURE_WIFI);
        }
        return result;
    }

    /**
     * Is Tel-URI format used
     *
     * @return Boolean
     */
    public boolean isTelUriFormatUsed() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.TEL_URI_FORMAT));
        }
        return result;
    }

    /**
     * Get ringing period
     *
     * @return Period in seconds
     */
    public int getRingingPeriod() {
        int result = 120;
        if(RcsSettings.getInstance().isSupportOP08()){
            result = 0;
        } else if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.RINGING_SESSION_PERIOD));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get default expire period for SUBSCRIBE
     *
     * @return Period in seconds
     */
    public int getSubscribeExpirePeriod() {
        int result = 3600;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.SUBSCRIBE_EXPIRE_PERIOD));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get "Is-composing" timeout for chat service
     *
     * @return Timer in seconds
     */
    public int getIsComposingTimeout() {
        int result = 15;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.IS_COMPOSING_TIMEOUT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get default expire period for INVITE (session refresh)
     *
     * @return Period in seconds
     */
    public int getSessionRefreshExpirePeriod() {
        int result = 3600;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.SESSION_REFRESH_EXPIRE_PERIOD));
            } catch(Exception e) {}
        }
        return 3600;
    }

    /**
     * Is permanente state mode activated
     *
     * @return Boolean
     */
    public boolean isPermanentStateModeActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.PERMANENT_STATE_MODE));
        }
        return result;
    }

    /**
     * Is trace activated
     *
     * @return Boolean
     */
    public boolean isTraceActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.TRACE_ACTIVATED));
        }
        return result;
    }

    /**
     * Get trace level
     *
     * @return trace level
     */
    public int getTraceLevel() {
        int result = Logger.ERROR_LEVEL;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.TRACE_LEVEL));
            } catch(Exception e) {}
        }
        return result;
    }


    /**
     * Is media trace activated
     *
     * @return Boolean
     */
    public boolean isSipTraceActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SIP_TRACE_ACTIVATED));
        }
        return result;
    }

    /**
     * Get SIP trace file
     *
     * @return SIP trace file
     */
    public String getSipTraceFile() {
        String result = Environment.getDataDirectory().getPath() + "sip.txt";
        if (instance != null) {
            try {
                result = readParameter(RcsSettingsData.SIP_TRACE_FILE);
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Is media trace activated
     *
     * @return Boolean
     */
    public boolean isMediaTraceActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.MEDIA_TRACE_ACTIVATED));
        }
        return result;
    }

    /**
     * Get capability refresh timeout used to avoid too many requests in a short time
     *
     * @return Timeout in seconds
     */
    public int getCapabilityRefreshTimeout() {
        int result = 1;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.CAPABILITY_REFRESH_TIMEOUT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get capability expiry timeout used to decide when to refresh contact capabilities
     *
     * @return Timeout in seconds
     */
    public int getCapabilityExpiryTimeout() {
        int result = 3600;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.CAPABILITY_EXPIRY_TIMEOUT));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get capability polling period used to refresh contacts capabilities
     *
     * @return Timeout in seconds
     */
    public int getCapabilityPollingPeriod() {
        int result = 3600;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.CAPABILITY_POLLING_PERIOD));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Is CS video supported
     *
     * @return Boolean
     */
    public boolean isCsVideoSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_CS_VIDEO));
        }
        return result;
    }

    /**
     * Is file transfer supported
     *
     * @return Boolean
     */
    public boolean isFileTransferSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER));
        }
        return result;
    }

    /**
     * Is file transfer via HTTP supported
     *
     * @return Boolean
     */
    public boolean isFileTransferHttpSupported() {
        boolean result = false;
        if (instance != null) {
            if ((getFtHttpServer().length() > 0) && (getFtHttpLogin().length() > 0) && (getFtHttpPassword().length() > 0)) {
                result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER_HTTP));
            }
        }
        return result;
    }

    /**
     * Is cloud file transfer supported
     *
     * @return Boolean
     */
    public boolean isCloudFileTransferSupported() {
        boolean result = false;
        if (supportOP01() && instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_CLOUD_FILE_TRANSFER));
        }
        return result;
    }

    public boolean isPayedEmoticonSupported() {
        if (supportOP01())
            return true;

        return false;
    }

    public boolean isCardMessageSupported() {
        boolean result = false;

        if (supportOP01() && instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_CARD_MESSAGE));
        }
        return result;
    }

    public boolean isBase64EncodingSupported(){
        if(supportOP01())
            return true;
        else
            return false;
    }


    /**
     * Is IM session supported
     *
     * @return Boolean
     */
    public boolean isImSessionSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_IM_SESSION));
        }
        return result;
    }

    /**
     * Is IM group session supported
     *
     * @return Boolean
     */
    public boolean isImGroupSessionSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_IM_GROUP_SESSION));
        }
        return result;
    }

    /**
     * Is image sharing supported
     *
     * @return Boolean
     */
    public boolean isImageSharingSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_IMAGE_SHARING));
        }
        return result;
    }

    /**
     * Is video sharing supported
     *
     * @return Boolean
     */
    public boolean isVideoSharingSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_VIDEO_SHARING));
        }
        return result;
    }

    /**
     * Is presence discovery supported
     *
     * @return Boolean
     */
    public boolean isPresenceDiscoverySupported() {
        boolean result = false;
        if (instance != null) {
            if (getXdmServer().length() > 0) {
                result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY));
            }
        }
        return false;
    }

    /**
     * Is social presence supported
     *
     * @return Boolean
     */
    public boolean isSocialPresenceSupported() {
        boolean result = false;
        if (instance != null) {
            if (getXdmServer().length() > 0) {
                result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE));
            }
        }
        return false;
    }

    /**
     * Is OPTIONS capability discovery supported
     *
     * @return Boolean
     */
    public boolean isOptionsDiscoverySupported() {
        if(isOP08SupportedByPlatform() || isSupportOP07()){
            return false;
        }else {
            return true;
        }
    }

    /**
     * Is geolocation push supported
     *
     * @return Boolean
     */
    public boolean isGeoLocationPushSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH));
        }
        return result;
    }


    /**
     * Is file transfer thumbnail supported
     *
     * @return Boolean
     */
    public boolean isFileTransferThumbnailSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER_THUMBNAIL));
        }
        return result;
    }

    /**
     * Is file transfer Store & Forward supported
     *
     * @return Boolean
     */
    public boolean isFileTransferStoreForwardSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER_SF));
        }
        return result;
    }

     /**
     * Is IP voice call supported
     *
     * @return Boolean
     */
    public boolean isIPVoiceCallSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_IP_VOICE_CALL));
        }
        return result;
    }

    /**
     * Is IP video call supported
     *
     * @return Boolean
     */
    public boolean isIPVideoCallSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_IP_VIDEO_CALL));
        }
        return result;
    }


    /**
     * Is group chat Store & Forward supported
     *
     * @return Boolean
     */
    public boolean isGroupChatStoreForwardSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_GROUP_CHAT_SF));
        }
        return result;
    }

    /**
     * Get supported RCS extensions
     *
     * @return List of extensions (semicolon separated)
     */
    public String getSupportedRcsExtensions() {
        String result = null;
        if (instance != null) {
            return readParameter(RcsSettingsData.CAPABILITY_RCS_EXTENSIONS);
        }
        return result;
    }

    /**
     * Set supported RCS extensions
     *
     * @param extensions List of extensions (semicolon separated)
     */
    public void setSupportedRcsExtensions(String extensions) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CAPABILITY_RCS_EXTENSIONS, extensions);
        }
    }

    /**
     * Is IM always-on thanks to the Store & Forward functionality
     *
     * @return Boolean
     */
    public boolean isImAlwaysOn() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IM_CAPABILITY_ALWAYS_ON));
        }
        return result;
    }

    /**
     * Is File Transfer always-on thanks to the Store & Forward functionality
     *
     * @return Boolean
     */
    public boolean isFtAlwaysOn() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.FT_CAPABILITY_ALWAYS_ON));
        }
        return result;
    }

    /**
     * Is IM reports activated
     *
     * @return Boolean
     */
    public boolean isImReportsActivated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IM_USE_REPORTS));
        }
        return result;
    }

    /**
     * Get network access
     *
     * @return Network type
     */
    public int getNetworkAccess() {
        int result = RcsSettingsData.ANY_ACCESS;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.NETWORK_ACCESS));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get network access
     *
     * @return Network type
     */
    public String getCurrentNetworkAccessInfo() {
        String result = "";
        if (instance != null) {
            try {
                result = readParameter(RcsSettingsData.CURRENT_ACCESS_NETWORKINFO);
                return result;
            } catch(Exception e) {}
        }
        return result;
    }


    /**
     * Get network access
     *
     * @return Network type
     */
    public String getLastNetworkAccessInfo() {
        String result = "";
        if (instance != null) {
            try {
                result = readParameter(RcsSettingsData.LAST_ACCESS_NETWORKINFO);
                return result;
            } catch(Exception e) {}
        }
        return result;
    }


    /**
     * Get SIP timer T1
     *
     * @return Timer in milliseconds
     */
    public int getSipTimerT1() {
        int result = 2000;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.SIP_TIMER_T1));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get SIP timer T2
     *
     * @return Timer in milliseconds
     */
    public int getSipTimerT2() {
        int result = 16000;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.SIP_TIMER_T2));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get SIP timer T4
     *
     * @return Timer in milliseconds
     */
    public int getSipTimerT4() {
        int result = 17000;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.SIP_TIMER_T4));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Is SIP keep-alive enabled
     *
     * @return Boolean
     */
    public boolean isSipKeepAliveEnabled() {
        boolean result = true;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SIP_KEEP_ALIVE));
        }
        return result;
    }

    /**
     * Get SIP keep-alive period
     *
     * @return Period in seconds
     */
    public int getSipKeepAlivePeriod() {
        int result = 60;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.SIP_KEEP_ALIVE_PERIOD));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get APN used to connect to RCS platform
     *
     * @return APN (null means any APN may be used to connect to RCS)
     */
    public String getNetworkApn() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.RCS_APN);
        }
        return result;
    }

    /**
     * Get operator authorized to connect to RCS platform
     *
     * @return SIM operator name (null means any SIM operator is authorized to connect to RCS)
     */
    public String getNetworkOperator() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.RCS_OPERATOR);
        }
        return result;
    }

    /**
     * Is GRUU supported
     *
     * @return Boolean
     */
    public boolean isGruuSupported() {
        boolean result = true;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.GRUU));
        }
        return result;
    }

    /**
     * Is IMEI used as device ID
     *
     * @return Boolean
     */
    public boolean isImeiUsedAsDeviceId() {
        boolean result = true;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.USE_IMEI_AS_DEVICE_ID));
        }
        return result;
    }

    /**
     * Is CPU Always_on activated
     *
     * @return Boolean
     */
    public boolean isCpuAlwaysOn() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CPU_ALWAYS_ON));
        }
        return result;
    }

    /**
     * Get auto configuration mode
     *
     * @return Mode
     */
    public int getAutoConfigMode() {
        int result = RcsSettingsData.HTTPS_AUTO_CONFIG;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.AUTO_CONFIG_MODE));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Is Terms and conditions via provisioning is accepted
     *
     * @return Boolean
     */
    public boolean isProvisioningTermsAccepted() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.PROVISIONING_TERMS_ACCEPTED));
        }
        return result;
    }

    /**
     * Get provisioning version
     *
     * @return Version
     */
    public String getProvisioningVersion() {
        String result = "0";
        if (instance != null) {
            result = readParameter(RcsSettingsData.PROVISIONING_VERSION);
        }
        return result;
    }

    /**
     * Set provisioning version
     *
     * @param version Version
     */
    public void setProvisioningVersion(String version) {
        if (instance != null) {
            writeParameter(RcsSettingsData.PROVISIONING_VERSION, version);
        }
    }

    /**
     * Set Terms and conditions via provisioning accepted
     *
     * @param state State
     */
    public void setProvisioningTermsAccepted(boolean state) {
        if (instance != null) {
            writeParameter(RcsSettingsData.PROVISIONING_TERMS_ACCEPTED,
                    Boolean.toString(state));
        }
    }

    /**
     * Get secondary provisioning address
     *
     * @return Address
     */
    public String getSecondaryProvisioningAddress() {
        String result = "";
        if (instance != null) {
            result = readParameter(RcsSettingsData.SECONDARY_PROVISIONING_ADDRESS);
        }
        return result;
    }

    /**
     * Set secondary provisioning address
     *
     * @param Address
     */
    public void setSecondaryProvisioningAddress(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.SECONDARY_PROVISIONING_ADDRESS, value);
        }
    }

   public long getProvisioningExpirationDate() {

        long result = 0;
         if (instance != null) {
                result = LauncherUtils.getProvisioningExpirationTime(mContext);
                return result;
         }
         return 0;
   }
    /*


    /**
     * Is secondary provisioning address only used
     *
     * @return Boolean
     */
    public boolean isSecondaryProvisioningAddressOnly() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SECONDARY_PROVISIONING_ADDRESS_ONLY));
        }
        return result;
    }

    /**
     * Set secondary provisioning address only used
     *
     * @param Boolean
     */
    public void setSecondaryProvisioningAddressOnly(boolean value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.SECONDARY_PROVISIONING_ADDRESS_ONLY, Boolean.toString(value));
        }
    }

    /**
     * Reset user profile settings
     */
    public void resetUserProfile() {
        setUserProfileImsUserName("");
        setUserProfileImsDomain("");
        setUserProfileImsPassword("");
        delAllImsProxyAddrForMobile();
        delAllImsProxyPortForMobile();
        delAllImsProxyAddrForWifi();
        delAllImsProxyPortForWifi();
        setUserProfileImsDisplayName("");
        setUserProfileImsPrivateId("");
        setXdmLogin("");
        setXdmPassword("");
        setXdmServer("");
        setProvisioningVersion("0");
        setProvisioningToken("");
        setMsisdn("");
        setConfigurationState(false);
    }

    /**
     * Is user profile configured
     *
     * @return Returns true if the configuration is valid
     */
    public boolean isUserProfileConfigured() {
        if(isOP08SupportedByPlatform()|| isSupportOP07() )
            return true;
        // Check platform settings
         if (TextUtils.isEmpty(getImsProxyAddrForMobile())) {
             return false;
         }

         // Check user profile settings
         if (TextUtils.isEmpty(getUserProfileImsDomain())) {
             return false;
         }
         String mode = RcsSettings.getInstance().getImsAuhtenticationProcedureForMobile();
         if (mode.equals(RcsSettingsData.DIGEST_AUTHENT)) {
             if (TextUtils.isEmpty(getUserProfileImsUserName())) {
                 return false;
             }
             if (TextUtils.isEmpty(getUserProfileImsPassword())) {
                 return false;
             }
             if (TextUtils.isEmpty(this.getUserProfileImsPrivateId())) {
                 return false;
             }
        }

        return true;
    }

    /**
     * Is group chat activated
     *
     * @return Boolean
     */
    public boolean isGroupChatActivated() {
        boolean result = false;
        if (instance != null) {
            String value = getImConferenceUri();
            if ((value != null) &&
                    (value.length() > 0) &&
                        !value.equals(RcsSettingsData.DEFAULT_GROUP_CHAT_URI)) {
                result = true;
            }
        }
        return result;
    }

    /**
     * Backup account settings
     *
     * @param account Account
     */
    public void backupAccountSettings(String account) {

        /*RichAddressBookProvider.backupAddressBookDatabase(account);
        IPCallProvider.backupIPCallDatabase(account);
        FileTransferProvider.backupFileTransferDatabase(account);
        ChatProvider.backupChatDatabase(account);
        ImageSharingProvider.backupImageSharingDatabase(account);
        VideoSharingProvider.backupVideoSharingDatabase(account);
        //Intent intent = new Intent(PluginUtils.ACTION_SIM_CHANGE);
        //intent.putExtra(ACTION_SIM_CHANGE, true);
        //AndroidFactory.getApplicationContext().sendStickyBroadcast(intent);
*/        try {
            String packageName = "com.orangelabs.rcs";
            String dbFile = Environment.getDataDirectory() + "/data/" + packageName + "/databases/" + RcsSettingsProvider.DATABASE_NAME;
            String backupFile = Environment.getDataDirectory() + "/data/" + packageName + "/databases/" + account + ".db";

            OutputStream outStream = new FileOutputStream(backupFile);
            InputStream inStream = new FileInputStream(dbFile);
             byte[] buffer = new byte[1024];
            int length;
            while ((length = inStream.read(buffer))>0) {
                outStream.write(buffer, 0, length);
            }
            outStream.flush();
            outStream.close();
            inStream.close();
        } catch(Exception e) {
            e.printStackTrace();
        }
    }

    /**
     * Restore account settings
     *
     * @param account Account
     */
    public void restoreAccountSettings(String account) {

         /*RichAddressBookProvider.restoreAddressBookDatabase(account);
        IPCallProvider.restoreIPCallDatabase(account);
        FileTransferProvider.restoreFileTransferDatabase(account);
        ChatProvider.restoreChatDatabase(account);
        ImageSharingProvider.restoreImageSharingDatabase(account);
        VideoSharingProvider.restoreVideoSharingDatabase(account);
        Intent intent = new Intent(PluginUtils.ACTION_SIM_CHANGE);
        AndroidFactory.getApplicationContext().sendStickyBroadcast(intent);*/
        try {
            String packageName = "com.orangelabs.rcs";
            String dbFile = Environment.getDataDirectory() + "/data/" + packageName + "/databases/" + RcsSettingsProvider.DATABASE_NAME;
            String restoreFile = Environment.getDataDirectory() + "/data/" + packageName + "/databases/" + account + ".db";

            File file = new File(restoreFile);
            if (!file.exists()) {
                if (LOGGER.isActivated()) {
                    LOGGER.debug("File not exist ");
                }
                return;
            }
            RcsSettings.getInstance().setFristLaunchState(false);
            OutputStream outStream = new FileOutputStream(dbFile);
            InputStream inStream = new FileInputStream(file);
             byte[] buffer = new byte[1024];
            int length;
            while ((length = inStream.read(buffer))>0) {
                outStream.write(buffer, 0, length);
            }
            outStream.flush();
            outStream.close();
            inStream.close();
            if (LOGGER.isActivated()) {
                        LOGGER.debug("File Restored ");
            }
         //   boolean status=isUserProfileConfigured();

        } catch(Exception e) {
            e.printStackTrace();
        }
        //setConfigurationState(true);
    }

    /**
     * Get the root directory for photos
     *
     *  @return Directory path
     */
    public String getPhotoRootDirectory() {
        String result = null;
        if (instance != null) {
            File file = new File(RcsSettingsData.DIRECTORY_PATH_PHOTOS);
            if (file.exists())
                result = readParameter(RcsSettingsData.DIRECTORY_PATH_PHOTOS);
        }
        if (result == null) {
            File file = mContext.getExternalFilesDir(Environment.DIRECTORY_PICTURES);
            if (file == null)
                file = mContext.getFilesDir();
            result = file.getPath();
        }

        return result;
    }

    /**
     * Set the root directory for photos
     *
     *  @param path Directory path
     */
    public void setPhotoRootDirectory(String path) {
        if (instance != null) {
            writeParameter(RcsSettingsData.DIRECTORY_PATH_PHOTOS, path);
        }
    }

    /**
      * Set configuration validity.
     *
     * @param time The configuration validity.
     */
    public void setProvisionValidity(long time) {
        if (instance != null) {
          //LauncherUtils.saveProvisioningValidity(mContext, time);
           writeParameter(RcsSettingsData.PROVISION_VALIDITY, String.valueOf(time));
        }
    }

/**
     * Get configuration time.
     *
     * @return The configuration time.
     */
    public long getProvisionTime() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.PROVISION_TIME);
        }
        if(result != null){
            if(BLANK.equals(result)){
                result = DEFAULT_VALUE;
            }
            return Long.parseLong(result);
        }
        return 0;
    }

 /**
     * Set configuration time.
     *
     * @param time The configuration time.
     */
    public void setProvisionTime(long time) {

        if (instance != null) {
            writeParameter(RcsSettingsData.PROVISION_TIME, String.valueOf(time));
        }
    }

    /**
     * Get the root directory for videos
     *
     *  @return Directory path
     */
    public String getVideoRootDirectory() {
        String result = null;
        if (instance != null) {
            File file = new File(RcsSettingsData.DIRECTORY_PATH_VIDEOS);
            if (file.exists())
                result = readParameter(RcsSettingsData.DIRECTORY_PATH_VIDEOS);
        }
        if (result == null) {
            File file = mContext.getExternalFilesDir(Environment.DIRECTORY_MOVIES);
            if (file == null)
                file = mContext.getFilesDir();
            result = file.getPath();
        }

        return result;
    }

    /**
     * Set the root directory for videos
     *
     *  @param path Directory path
     */
    public void setVideoRootDirectory(String path) {
        if (instance != null) {
            writeParameter(RcsSettingsData.DIRECTORY_PATH_VIDEOS, path);
        }
    }

    /**
     * Get the root directory for files
     *
     *  @return Directory path
     */
    public String getFileRootDirectory() {
        String result = null;
        if (instance != null) {
            File file = new File(RcsSettingsData.DIRECTORY_PATH_FILES);
            if (file.exists())
                result = readParameter(RcsSettingsData.DIRECTORY_PATH_FILES);
        }
        if (result == null) {
            File file = mContext.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS);
            if (file == null)
                file = mContext.getFilesDir();
            result = file.getPath();
        }

        return result;
    }


/**
     * Set configuration version.
     *
     * @param version The configuration version.
     */
/*
    public void setProvisionVersion(String version) {
        if (instance != null) {
            writeParameter(RcsSettingsData.PROVISION_VERSION, version);
        }
    }
*/

/** M: Remove all users profile information @{ */
    public void removeAllUsersProfile() {
        setServiceActivationState(false);
        setUserProfileImsUserName("");
        setUserProfileImsDomain("");
        setUserProfileImsPassword("");
        delAllImsProxyAddrForMobile();
        delAllImsProxyPortForMobile();
        delAllImsProxyAddrForWifi();
        delAllImsProxyPortForWifi();
        setUserProfileImsDisplayName("");
        setUserProfileImsPrivateId("");
        setXdmLogin("");
        setXdmPassword("");
        setXdmServer("");
        setMsrpProtocolForMobile("");
        setMsrpProtocolForWifi("");
        /** M: add for provision validity @{ */
        setProvisionValidity(0);
        setProvisionTime(Long.MAX_VALUE);
        setProvisioningVersion(DEFAULT_VALUE);
        /** @} */

    }

    /**
     * Set the root directory for files
     *
     *  @param path Directory path
     */
    public void setFileRootDirectory(String path) {
        if (instance != null) {
            writeParameter(RcsSettingsData.DIRECTORY_PATH_FILES, path);
        }
    }

    /**
     * Is secure MSRP media over Wi-Fi
     *
     * @return Boolean
     */
    public boolean isSecureMsrpOverWifi() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SECURE_MSRP_OVER_WIFI));
        }
        return result;
    }

    /**
     * Is gzip supported
     *
     * @return Boolean
     */
    public boolean isGzipSupported() {
        boolean result = true;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.GZIP_SUPPORT));
        }
        return result;
    }

    /**
     * Is Composing supported
     *
     * @return Boolean
     */
    public boolean isComposingSupported() {
        boolean result = true;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IS_COMPOSING));
        }
        return result;
    }

    /**
     * Is Composing supported
     *
     * @return Boolean
     */
    public boolean isLargeModeSupported() {
        boolean result = true;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IS_LARGE_MODE));
        }
        return result;
    }


    /**
     * Is secure MSRP media over Mobile
     *
     * @return Boolean
     */
    public boolean isSecureMsrpOverMobile() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SECURE_MSRP_OVER_MOBILE));
        }
        return result;
    }


    /**
     * Is secure RTP media over Wi-Fi
     *
     * @return Boolean
     */
    public boolean isSecureRtpOverWifi() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SECURE_RTP_OVER_WIFI));
        }
        return result;
    }



    /**
     * Is secure RTP media over mobile
     *
     * @return Boolean
     */
    public boolean isSecureRtpOverMobile() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SECURE_RTP_OVER_MOBILE));
        }
        return result;
    }


    /**
     * Get max geolocation label length
     *
     * @return Number of char
     */
    public int getMaxGeolocLabelLength() {
        int result = 100;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_GEOLOC_LABEL_LENGTH));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get geolocation expiration time
     *
     * @return Time in seconds
     */
    public int getGeolocExpirationTime() {
        int result = 1800;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.GEOLOC_EXPIRATION_TIME));
            } catch(Exception e) {}
        }
        return result;
    }

    public void setProvisioningToken(String token) {
        if (instance != null) {
            writeParameter(RcsSettingsData.PROVISIONING_TOKEN, token);
        }
    }

    public void setProvisioningTokenValidity(long tokenValidity) {
        if (instance != null) {
            writeParameter(RcsSettingsData.PROVISIONING_TOKEN_VALIDITY, String.valueOf(tokenValidity));
        }
    }

    public long getProvisionTokenValidity() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.PROVISIONING_TOKEN_VALIDITY);
        }
        if(result != null){
            if(BLANK.equals(result)){
                result = DEFAULT_VALUE;
            }
            return Long.parseLong(result);
        }
        return 0;
    }


    public String getProvisioningToken() {
        String result = "0";
        if (instance != null) {
            result = readParameter(RcsSettingsData.PROVISIONING_TOKEN);
        }
        return result;
    }

    /**
     * Set chat wall paper resource. This method can be called on UI thread.
     *
     * @param chatWallpaper The resource id of chat wall paper or the file name of chat wall paper.
     *//*
    public void setChatWallpaper(final String chatWallpaper) {
        if (chatWallpaper == null) {
            if (LOGGER.isActivated()) {
                LOGGER.debug("setChatWallpaperId invalid chatWallpaper. chatWallpaper is null");
            }
            return;
        }
        if (instance != null) {
            // Save it in memory when write it to database.
            mChatWallpaper = chatWallpaper;
            for (OnWallPaperChangedListener listener : mOnWallPaperChangedListenerList) {
                listener.onWallPaperChanged(chatWallpaper);
            }
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    writeParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER, chatWallpaper);
                }
            });
        }
    }*/

    /**
     * Is SIP device an automata ?
     *
     * @return Boolean
     */
    public boolean isSipAutomata() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_SIP_AUTOMATA));
        }
        return result;
    }

      /**
     * M: Add to achieve the RCS-e set compressing image feature. @{
     */
    private boolean mEnableCompressingImage = true;

    private boolean initCompressingImageStatus() {
        if (LOGGER.isActivated()) {
            LOGGER.debug("initCompressingImageStatus() read from database");
        }
        boolean result = true;
        result = Boolean.parseBoolean(readParameter(RcsSettingsData.RCSE_COMPRESSING_IMAGE));
        if (LOGGER.isActivated()) {
            LOGGER.debug("initCompressingImageStatus() from database, result: " + result);
        }
        return result;
    }

    /**
     * Check whether compressing image when send image.
     *
     * @return True if compressing image is enabled, otherwise return false.
     */
    public boolean isEnabledCompressingImage() {
        return mEnableCompressingImage;
    }

    /**
     * Check whether compressing image when send image. Do not call this method
     * in ui thread
     *
     * @return True if compressing image is enabled, otherwise return false.
     */
    public boolean isEnabledCompressingImageFromDB() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.RCSE_COMPRESSING_IMAGE);
            if (LOGGER.isActivated()) {
                LOGGER.debug("isEnabledCompressingImageFromDB(), result: " + result);
            }
            return Boolean.valueOf(result);
        }
        return true;
    }

    /**
     * Set the status which indicate whether compressing image when send image.
     *
     * @param state True if compressing image is enabled, otherwise return
     *            false.
     */
    public void setCompressingImage(final boolean state) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("setCompressingImage(), state: " + state);
        }
        mEnableCompressingImage = state;
        writeParameter(RcsSettingsData.RCSE_COMPRESSING_IMAGE, Boolean.toString(state));
    }

    /**
     * Get the remind flag
     * @return True if need remind compress again when send image, otherwise
     *         return false
     */
    public boolean restoreRemindCompressFlag() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.COMPRESS_IMAGE_HINT);
            if (LOGGER.isActivated()) {
                LOGGER.debug("restoreRemindCompressFlag(), result: " + result);
            }
            return Boolean.valueOf(result);
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("instance is null, return false");
        }
        return false;
    }

    /**
     * Set the remind flag
     *
     * @param notRemind Indicates whether need to remind user to compress image
     *            when send image
     */
    public void saveRemindCompressFlag(final boolean notRemind) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("saveRemindFlag(), notRemind: " + notRemind);
        }
        writeParameter(RcsSettingsData.COMPRESS_IMAGE_HINT, Boolean.toString(!notRemind));
    }
    /**
     * @}
     */

    /**
     * M: Add to achieve Large Size image Reminder @{
     */
      /**
     * Set the warning remind flag
     *
     * @param notRemind Indicates whether need to remind user to Large image
     *            when send image
     */
    public void saveRemindWarningLargeImageFlag(final boolean notRemindFlag) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("saveRemindWarningLargeImageFlag(), notRemindFlag: " + notRemindFlag);
        }
        writeParameter(RcsSettingsData.WARNING_LARGE_IMAGE_HINT, Boolean.toString(!notRemindFlag));
    }

    /**
     * Get the warning remind flag
     * @return True if need remind Large image again when send image, otherwise
     *         return false
     */
    public boolean restoreRemindWarningLargeImageFlag() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.WARNING_LARGE_IMAGE_HINT);
            if (LOGGER.isActivated()) {
                LOGGER.debug("restoreRemindWarningLargeFlag(), result: " + result);
            }
            return Boolean.valueOf(result);
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("instance is null, return false");
        }
        return false;
    }

    /**
     * M: Add to avoid doing IO in constructor. @{
     *//*
    private void initParaInBackground() {
        if (LOGGER.isActivated()) {
            LOGGER.debug("initParaInBackground()");
        }
        AsyncTask.execute(new Runnable() {
            @Override
            public void run() {
                mChatWallpaper = initChatWallpaper();
                if (LOGGER.isActivated()) {
                    LOGGER.debug("mChatWallpaper = " + mChatWallpaper);
                }
                for (OnWallPaperChangedListener listener : mOnWallPaperChangedListenerList) {
                    listener.onWallPaperChanged(mChatWallpaper);
                }
                mEnableCompressingImage = initCompressingImageStatus();
                mMaxChatParticipants = initMaxChatParticipants();
            }
        });
    }*/
    /**
     * @}
     */

    /**
     * Is SMS Over IP supported. UI should write the parameter of the related
     * KEY(RcsSettingsData.CAPABILITY_SMSOverIP) to indicate its SMS capability.
     *
     * @return Boolean
     */
    public boolean isSMSOverIPSupported() {
        boolean result = false;
        if (instance != null) {
            String resultString = readParameter(RcsSettingsData.CAPABILITY_SMSOverIP);
            if (null != resultString) {
                result = Boolean.parseBoolean(resultString);
            }
        }

        if(LOGGER.isActivated()){
            LOGGER.debug("isSMSOverIPSupported() result is " + result);
        }

        return result;
    }

     /**
     * Is ICSI MMtel supported. Is SMS Over IP supported. UI should write the parameter of the related
     * KEY(RcsSettingsData.CAPABILITY_ICSI_MMTEL) to indicate its ICSI MMTel capability.
     *
     * @return Boolean
     */
    public boolean isICSIMMTelSupported() {
        boolean result = false;
        if (instance != null) {
            String resultString = readParameter(RcsSettingsData.CAPABILITY_ICSI_MMTEL);
            if (null != resultString) {
                result = Boolean.parseBoolean(resultString);
            }
        }

        if(LOGGER.isActivated()){
            LOGGER.debug("isICSIMMTelSupported() result is " + result);
        }

        return result;
    }

    /**
     * Is ICSI Emergency supported. UI should write the parameter of the related
     * KEY(RcsSettingsData.CAPABILITY_ICSI_EMERGENCY) to indicate its ICSI Emergency call capability.
     *
     * @return Boolean
     */
    public boolean isICSIEmergencySupported() {
        boolean result = false;
        if (instance != null) {
            String resultString = readParameter(RcsSettingsData.CAPABILITY_ICSI_EMERGENCY);
            if (null != resultString) {
                result = Boolean.parseBoolean(resultString);
            }
        }

        if(LOGGER.isActivated()){
            LOGGER.debug("isICSIEmergencySupported() result is " + result);
        }

        return result;
    }



    /**
     * Is XCAP operations disabled. Handset client shall accept a custom
     * HTTP response code of HTTP 499, then it shold disable the client
     * from future XCAP oprations.
     *
     * @return Boolean true for block, false for unblock
     */
    public boolean isXCAPOperationBlocked() {
        boolean result = false;
        if (instance != null) {
            String resultString = readParameter(RcsSettingsData.BLOCK_XCAP_OPERATION);
            if (null != resultString) {
                result = Boolean.parseBoolean(resultString);
            }
        }

        if(LOGGER.isActivated()){
            LOGGER.debug("isXCAPOperationBlocked() result is " + result);
        }

        return result;
    }


    /**
     * M: Add to achieve the RCS-e set chat wall paper feature.
     */
    // The resource id of chat wall paper copy in memory
    private String mChatWallpaper = null;
    //private final ArrayList<OnWallPaperChangedListener> mOnWallPaperChangedListenerList = new ArrayList<OnWallPaperChangedListener>();


    /**
     * M: add for providing a listener to notify all the observer that the
     * capabilities of current user has changed @{
     */
    /**
     * This class defined a listener to notify all the observers that the
     * capabilities of current user has changed.
     */
    public static interface SelfCapabilitiesChangedListener {
        public void onCapabilitiesChangedListener(Capabilities capabilities);
    }





   /**
     * Get chat wall paper resource id. If this method returns 0, please call
     * {@link #getChatWallpaper()} to get the wall paper file name. This method
     * can be called on UI thread.
     *
     * @return The resource id of chat wall paper or the file name of chat wall
     *         paper.
     */
    public int getChatWallpaperId() {

            return 0;

    }

    /**
     * Get chat wall paper resource. This method can be called on UI thread.
     *
     * @return The resource id of chat wall paper or the file name of chat wall paper.
     */
    public String getChatWallpaper() {
        if (LOGGER.isActivated()) {
            LOGGER.debug("getChatWallpaper() from memory, mChatWallpaper: " + mChatWallpaper);
        }
        return mChatWallpaper;
    }


    /**
     * M: Added to update the file transfer capability and the database. @{
     */
    /**
     * Set file transfer capability
     *
     * @param isSupport whether to support file transfer.
     * @return wether the databse update operation is successfully.
     */
    public boolean setSupportFileTransfer(boolean isSupport){
        boolean result = false;
        if (instance != null && mContext != null) {
            if (mCurrentFTCapability != isSupport) {
                mCurrentFTCapability = isSupport;
                writeParameter(RcsSettingsData.CAPABILITY_FILE_TRANSFER, String.valueOf(isSupport));
                result = true;
            }
        }
           return result;
       }
           /**
     * @}
     */

 // The observer list
    private final LinkedList<SelfCapabilitiesChangedListener> mListenerList = new LinkedList<SelfCapabilitiesChangedListener>();

    /**
     * Register the observer
     *
     * @param listener The observer to register
     */
    public void registerSelfCapabilitiesListener(SelfCapabilitiesChangedListener listener) {
        if(LOGGER.isActivated()){
            LOGGER.debug("registerSelfCapabilitiesListener() called");
        }
        mListenerList.add(listener);
    }

    /**
     * Unregister the observer
     *
     * @param listener The observer to unregister
     */
    public void unregisterSelfCapabilitiesListener(SelfCapabilitiesChangedListener listener) {
        if(LOGGER.isActivated()){
            LOGGER.debug("unregisterSelfCapabilitiesListener() called");
        }
        mListenerList.remove(listener);
    }

    private void notifySelfCapabilitiesChanged(String key){
        if(LOGGER.isActivated()){
            LOGGER.debug("notifySelfCapabilitiesChanged() called()");
        }

        if(key.equals(RcsSettingsData.CAPABILITY_CS_VIDEO)
         ||key.equals(RcsSettingsData.CAPABILITY_IMAGE_SHARING)
         ||key.equals(RcsSettingsData.CAPABILITY_VIDEO_SHARING)
         ||key.equals(RcsSettingsData.CAPABILITY_FILE_TRANSFER)
         ||key.equals(RcsSettingsData.CAPABILITY_IM_SESSION)
         ||key.equals(RcsSettingsData.CAPABILITY_PRESENCE_DISCOVERY)
         ||key.equals(RcsSettingsData.CAPABILITY_SOCIAL_PRESENCE)
         ||key.equals(RcsSettingsData.CAPABILITY_RCS_EXTENSIONS)){
            if(LOGGER.isActivated()){
                LOGGER.debug("notifySelfCapabilitiesChanged() capabilities changed");
            }
            // Get self capabilities
            Capabilities capabilities = getMyCapabilities();
            // Notify all the listeners
            for(int i=0;i<mListenerList.size();i++){
                SelfCapabilitiesChangedListener listener = mListenerList.get(i);
                if(listener != null){
                    listener.onCapabilitiesChangedListener(capabilities);
                }else{
                    if(LOGGER.isActivated()){
                        LOGGER.debug("notifySelfCapabilitiesChanged() listener is null");
                    }
                }
            }
        }
    }

    public String getIMSProfileValue() {
        if (PROFILE_VALUE == null)
            PROFILE_VALUE = getUserProfileImsDisplayName();
        if (LOGGER.isActivated()) {
            LOGGER.debug("getIMSProfileValue  value is" + PROFILE_VALUE);
        }
        return PROFILE_VALUE;
    }

    public void setIMSProfileValue(String profile) {
        if (LOGGER.isActivated()) {
            LOGGER.debug("setIMSProfileValue  value is" + profile);
        }
        PROFILE_VALUE = profile;
    }
    /**
     * @}
     */

     /** M:When the device reboot, it's necessary to check @{ */
    /**
     * Get configuration validity.
     *
     * @return The configuration validity.
     */
    public long getProvisionValidity() {
    /*
        long result = 0;
         if (instance != null) {
                result = LauncherUtils.getProvisioningValidity(mContext);
                return result;
         }
         return 0;
    */
        String result = null;
        if (instance != null) {
        result = readParameter(RcsSettingsData.PROVISION_VALIDITY);
        }
        if(result != null){
            if(BLANK.equals(result)){
                result = DEFAULT_VALUE;
            }
            return Long.parseLong(result);
        }
        return 0;

    }

  /**
     * Get configuration version.
     *
     * @return The configuration version.
     */
 /*
    public String getProvisionVersion() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.PROVISION_VERSION);
            if(BLANK.equals(result)){
                result = DEFAULT_VALUE;
            }
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("getProvisionVersion result: " + result);
        }
        return result;
    }
    */


 /**
     * M: Add to achieve high performance when query mMaxChatParticipants. @{
     */
    private int mMaxChatParticipants = 5;

    private int initMaxChatParticipants() {
        if (LOGGER.isActivated()) {
            LOGGER.debug("initMaxChatParticipants() read from database");
        }
        int result = 5;
        try {
            result = Integer.parseInt(readParameter(RcsSettingsData.MAX_CHAT_PARTICIPANTS));
        } catch (NumberFormatException e) {
            if (LOGGER.isActivated()) {
                LOGGER.debug("initMaxChatParticipants() NumberFormatException");
            }
        }
        if (LOGGER.isActivated()) {
            LOGGER.debug("initMaxChatParticipants() from database, result: " + result);
        }
        return result;
    }

  private String initChatWallpaper() {
        if (LOGGER.isActivated()) {
            LOGGER.debug("initChatWallpaper() read from database");
        }
        String result = DEFAULT_VALUE;
        result = readParameter(RcsSettingsData.RCSE_CHAT_WALLPAPER);
        if (LOGGER.isActivated()) {
            LOGGER.debug("getChatWallpaper() from database, result: " + result);
        }
        return result;
    }

     /**BB Drop 1**/

     /**
   * Get messaging UX mode.
   *
   * @return The messaging UX.
   */
    public int getMessagingUx() {
        int result = 0;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MESSAGING_UX));
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        return result;
    }

    /**
     * Set the mode to converged - 0, Full Integrated -1
     *
     * @param mode mode(0/1)
     */
    public void setMessagingUx(int mode) {
        if (instance != null) {
            writeParameter(RcsSettingsData.MESSAGING_UX, "" + mode);
        }
    }

     /**
     * Get File Transfer Http capibility.
     *
     * @return File transfer Http capability.
     */

     public int getFtHttpCapAlwaysOn() {
        int result= 0;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.FT_HTTP_CAP_ALWAYS_ON));
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        return result;
    }


     /**
   * Get messaging UX mode.
   *
   * @return The messaging UX.
   */
    public int getMessagingCapbailitiesValidiy() {
        int result = 5;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MESSAGING_CAPABILITIES_VALIDITY));
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        return result;
    }


    /**
     * Get Delivery Timeout.
     *
     * @return The delivery timeout.
     */

    public int getDeliveryTimeout() {
        int result = 300;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.DELIVERY_TIMEOUT));
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        return result;
    }




    /**
     * Get is Messaging Disable and Fully integrated mode is on.
     *
     * @return The Messaging Disable and Fully integrated mode.
     */


    public boolean getJoynMessagingDisabledFullyIntegrated() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED));
        }
        return result;
    }

    public int getDisableServiceStatus()
    {
        int result = 0;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.JOYN_DISABLE_STATUS));
            } catch(Exception e) {}
        }
        return result;
    }
    /**
     * Get max file-icon size
     *
     * @return Size in kilobytes
     */
    public int getMaxFileIconSize() {
        int result = 50;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.MAX_FILE_ICON_SIZE));
            } catch(Exception e) {}
        }
        return result;
    }

    public void setDisableServiceStatus(int status)
    {
        if (instance != null) {
            writeParameter(RcsSettingsData.JOYN_DISABLE_STATUS, "" + status);
        }
    }

    /**
     * Set is Messaging Disable and Fully integrated mode is on.
     *
     */


public void setJoynMessagingDisabledFullyIntegrated(boolean state) {
        if (instance != null) {
            writeParameter(RcsSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED, Boolean.toString(state));
        }
    }

     /**
     * Get Joyn Alias.
     *
     * @return Joyne Alias.
     */

    public String getJoynUserAlias() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.JOYN_USER_ALIAS);
            if (!supportOP01()) {
                if(BLANK.equals(result)){
                    result = DEFAULT_VALUE;
                }
            }
        }

        if (supportOP01()) {
            // The Default value for OP01 is ""
            if (result.equals(DEFAULT_VALUE)) {
                result = "";
            }
        }

        if (LOGGER.isActivated()) {
            LOGGER.debug("getJoynUserAlies: " + result);
        }
        return result;
    }

    /**
     * Set Joyn Alias.
     *
     */

    public void setJoynUserAlies(String alias) {
        if (instance != null) {
            writeParameter(RcsSettingsData.JOYN_USER_ALIAS, alias);
        }
    }

    /**
     * Get beIPVoiceCallAuth provisioning parameter (authorized networks for ip voice call feature)
     *
     * @return int value
     */
    public int getIPVoiceCall_Auth() {
        int result = 15;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.BE_IPVOICECALL_AUTH));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Get the GSMA release
     *
     * @return the GSMA release
     */
    public int getGsmaRelease() {
        int result = 1; // Blackbird
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.KEY_GSMA_RELEASE));
            } catch (Exception e) {
            }
        }
        return result;
    }

    /**
     * Get beIPVideoCallAuth provisioning parameter (authorized networks for ip video call feature)
     *
     * @return int value
     */
    public int getIPVideoCall_Auth() {
        int result = 15;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.BE_IPVIDEOCALL_AUTH));
            } catch(Exception e) {}
        }
        return result;
    }

    /**
     * Is device in RCS-AA mode authorized to initiate IP Voice Call even if remote doesn't show its voice service capabilities?
     *
     * @return Boolean
     */
    public boolean isIPVoiceCallBreakout() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IPVOICECALL_BREAKOUT));
        }
        return result;
    }

    /**
     * is device in RCS-CS mode authorized to initiate IP Voice Call even if remote doesn't show its voice service capabilities?
     *
     * @return Boolean
     */
    public boolean isIPVoiceCallBreakoutCS() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IPVOICECALL_BREAKOUT_CS));
        }
        return result;
    }

      /**
     * is device in RCS-CS mode authorized to upgrade to video a CS call?
     *
     * @return Boolean
     */
    public boolean isIPVideoCallUpgradeFromCS() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_FROM_CS));
        }
        return result;
    }


    /**
     * Is device in RCS-AA or RCS-CS mode authorized to upgrade to video even if no answer to capability request (fails with 480 or 408 error)?
     *
     * @return Boolean
     */
    public boolean isIPVideoCallUpgradeOnCapError() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_ON_CAPERROR));
        }
        return result;
    }

    /**
     * Is device in RCS-CS mode authorized to upgrade to video without first tearing down CS call?
     *
     * @return Boolean
     */
    public boolean isIPVideoCallAttemptEarly() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IPVIDEOCALL_UPGRADE_ATTEMPT_EARLY));
        }
        return result;
    }

    /**
     * get label to use when presenting to user the option for sending end-to-end ip call
     *
     * @return Boolean
     */
    public String getIPCallE2ELabel() {
        String result = "";
        if (instance != null) {
            result = readParameter(RcsSettingsData.IPCALL_E2E_LABEL);
        }
        return result;
    }

    /**
     * get label to be use when presenting to user the option for initiating a breakout ip call
     *
     * @return Boolean
     */
    public String getIPCallBreakoutLabel() {
        String result = "";
        if (instance != null) {
            result = readParameter(RcsSettingsData.IPCALL_BREAKOUT_LABEL);
        }
        return result;
    }

    /**
     * Is "E2E" button (case false) or "Breakout" button (case true) presented to user ?
     *
     * @return Boolean
     */
    public boolean isIPCallE2EVoiceCapabilityHandling() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IPCALL_E2E_VOICECAPABILITYHANDLING));
        }
        return result;
    }

    /**
     * Set the GSMA release
     */
    public void setGsmaRelease(String gsmaRelease) {
        if (instance != null) {
            writeParameter(RcsSettingsData.KEY_GSMA_RELEASE, gsmaRelease);
        }
    }

    /**
    * Encrypt data
    * @param secretKey - a secret key used for encryption
    * @param data - data to encrypt
    * @return Encrypted data
    * @throws Exception
     */
    public static String cipher(String secretKey, String data) throws Exception {

        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
        KeySpec spec = new PBEKeySpec(secretKey.toCharArray(), secretKey.getBytes(), 128, 256);
        SecretKey tmp = factory.generateSecret(spec);
        SecretKey key = new SecretKeySpec(tmp.getEncoded(), ALGORITHM);

        Cipher cipher = Cipher.getInstance(ALGORITHM);
        cipher.init(Cipher.ENCRYPT_MODE, key);

        return toHex(cipher.doFinal(data.getBytes()));
    }

    /**
    * Decrypt data
    * @param secretKey - a secret key used for decryption
    * @param data - data to decrypt
    * @return Decrypted data
    * @throws Exception
    */
    public static String decipher(String secretKey, String data) throws Exception {

        SecretKeyFactory factory = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
        KeySpec spec = new PBEKeySpec(secretKey.toCharArray(), secretKey.getBytes(), 128, 256);
        SecretKey tmp = factory.generateSecret(spec);
        SecretKey key = new SecretKeySpec(tmp.getEncoded(), ALGORITHM);

        Cipher cipher = Cipher.getInstance(ALGORITHM);

        cipher.init(Cipher.DECRYPT_MODE, key);

        return new String(cipher.doFinal(toByte(data)));
    }

    // Helper methods

    private static byte[] toByte(String hexString) {
        int len = hexString.length()/2;

        byte[] result = new byte[len];

        for (int i = 0; i < len; i++)
            result[i] = Integer.valueOf(hexString.substring(2*i, 2*i+2), 16).byteValue();
        return result;
    }

    public static String toHex(byte[] stringBytes) {
        StringBuffer result = new StringBuffer(2*stringBytes.length);

        for (int i = 0; i < stringBytes.length; i++) {
            result.append(HEX.charAt((stringBytes[i]>>4)&0x0f)).append(HEX.charAt(stringBytes[i]&0x0f));
        }

        return result.toString();
    }

    public static String getCipherKey() {
        return keyCipher;

    }



    private final static String HEX = "0123456789ABCDEF";


    public int getNumberOfPCssfAddresses() {
        int result = 1;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.NUMBER_OF_PCSCF_ADDRESS));
            } catch(Exception e) {}
        }
        return result;
    }

    public void setCurrentAddressCounter(int nbAddress) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CURRENT_PCSCF_ADDRESS_COUNTER, "" + nbAddress);
        }

    }


public int getCurrentPCscfAddresCounter() {
        int result = 0;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.CURRENT_PCSCF_ADDRESS_COUNTER));
            } catch(Exception e) {}
        }
        return result;
        }

         /*
        * Delete PCSCF entries fr4om Settings Database
       */

    public void ResetPCSCFAddress() {
        delAllImsProxyAddrForMobile();
            delAllImsProxyPortForMobile();
            delAllImsProxyAddrForWifi();
            delAllImsProxyPortForWifi();
    }

    /**
     * Is voice breakout supported
     *
     * @return Boolean
     */
    public boolean isVoiceBreakoutSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.VOICE_BREAKOUT));
        }
        return result;
    }


      /**
     * Set is First messagein invite is on.
     *
     */


    public void setFirstMessageInInvite(boolean invite) {
        if (instance != null) {
            writeParameter(RcsSettingsData.FIRST_MSG_IN_INVITE, Boolean.toString(invite));
        }
    }

    /**

    * Is First Message in invite

    *

    * @return Boolean

    */

    public boolean isFirstMessageInInvite() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.FIRST_MSG_IN_INVITE));
        }
        return result;
    }

     /**
     * Set is multimedia in chat is on.
     *
     */


    public void setMultimediaInChat(boolean invite) {
        if (instance != null) {
            writeParameter(RcsSettingsData.MULTIMEDIA_IN_CHAT, Boolean.toString(invite));
        }
    }

    /**

    * Is multimedia in Chat

    *

    * @return Boolean

    */

    public boolean isMultimediaInChat() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.MULTIMEDIA_IN_CHAT));
        }
        return result;
    }


    /**
     * Set CPM or IM.
     * True :- CPM
     * False :- IM
     */


    public void setCPMSupported(boolean invite) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CPM_TECHNOLOGY, Boolean.toString(invite));
        }
    }

    /**

    * Is multimedia in Chat

    *

    * @return Boolean

    */

    public boolean isCPMSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CPM_TECHNOLOGY));
        }
        return result;
    }

    public boolean isCPMPagerModeSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CPM_PAGER_MODE));
        }
        return result;
    }

    public boolean isCPMLargeModeSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CPM_LARGE_MSG));
        }
        return result;
    }

    public boolean isCPMFTSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CPM_FT_MSG));
        }
        return result;
    }

    /**
     * Is file transfer invitation auto accepted on roaming
     *
     * @return Boolean
     */
    public boolean isFileTransferAutoAcceptedOnRoaming() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER_ON_ROAMING));
        }
        return result;
    }

    /**
     * set file transfer invitation auto accepted on roaming
     * @param isAuto
     */
    public void setFileTransferAutoAcceptedOnRoaming(boolean isAuto) {
        if (instance != null) {
            writeParameter(RcsSettingsData.AUTO_ACCEPT_FILE_TRANSFER_ON_ROAMING, Boolean.toString(isAuto));
        }
    }
    // @ end

    /**
     * @ add for full public_user_identify bug not only digital part
     * Get user profile username (i.e. username part of the IMPU)
     *
     * @return Username part of SIP-URI
     */
    public String getUserProfileImsUserName_full() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.USERPROFILE_IMS_USERNAME_FULL);
        }
        return result;
    }

    /**
     * Set user profile IMS username (i.e. username part of the IMPU)
     *
     * @param value Value
     */
    public void setUserProfileImsUserName_full(String value) {
        if (instance != null) {
            writeParameter(RcsSettingsData.USERPROFILE_IMS_USERNAME_FULL, value);
        }
    }
    // @add for full public_user_identify bug not only digital part

    public String getSecondaryDeviceUserIdentity() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.PUBLIC_USER_IDENTITY_PC);
        }
        return result;
    }

    /**
     * CPM get max standalone message size
     *
     * @return max standalone message count
     */
    public int getMaxStandaloneSize() {
        int result = 1;
        if (instance != null) {
            try {
                result = Integer
                        .parseInt(readParameter(RcsSettingsData.MAX_STANDALONE_SIZE));
            } catch (Exception e) {
            }
        }
        return result;
    }

    /**
     * BURN AFTER READING CHANGES
     */
    public boolean isCPMBurnAfterReadingSupported(){
        boolean status = false;
        if(supportOP01())
            return true;

        return status;
    }

    public String getNetworkOperatorAddress() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.RCS_OPERATOR_ADDRESS);
        }

        if (isSupportOP08()) {
            result = "config.rcs.sipgeo.t-mobile.com";
        }
        return result;
    }

    /**
     * set operator authorized to connect to RCS platform
     *
     * @param optr
     */

    public void setNetworkOperator(String optr) {
       // String result = null;
        if (instance != null) {
            writeParameter(RcsSettingsData.RCS_OPERATOR, optr);
        }
       // return result;
    }

    /**
     * Get max pager message content size
     *
     * @return max pager message content size
     */
    public int getMaxPagerContentSize() {
        int result = 1300;
        if (instance != null) {
                try {
                     result = Integer.parseInt(readParameter(RcsSettingsData.MAX_PAGER_CONTENT_SIZE));
                } catch (Exception e) {
                }
            }
            return result;
    }

    public void setPublicUri(String uri) {
        if (instance != null) {
            writeParameter(RcsSettingsData.PUBLIC_URI, uri);
        }
    }


    public void setConfigurationState(boolean state) {
        if (instance != null) {
            writeParameter(RcsSettingsData.CONFIGURATION_STATE, Boolean.toString(state));
            Intent intent = new Intent();
            intent.setAction(CONFIGURATION_STATUS);
            intent.putExtra(CORE_CONFIGURATION_STATUS, state);
            mContext.sendBroadcast(intent, "com.gsma.services.permission.RCS");
        }

    }

    /**
     * Is RCS App Has required Permission
     *
     * @return Boolean
     */
    public boolean isServiceGotPermission() {
        boolean result = false;

        /* OP01 ignore permission checking for now */
        if (supportOP01()) {
            return true;
        }
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.SERVICE_PERMITTED));
        }
        return result;
    }

    /**
     * Set the RCS permission activation state
     *
     * @param state State
     */
    public void setServicePermissionState(boolean state) {
        if (instance != null) {
            writeParameter(RcsSettingsData.SERVICE_PERMITTED, Boolean.toString(state));
        }
    }
     /**
     * SSo Address
     *
     * @return address
     */
     public String getSpamAddress() {
         String result = null;
         if (supportOP01()) {
             result="+86100869999";
         } else if(isSupportOP08()) {
             result="";
         } else {
             result="";
         }
         return result;
     }

    //if messages needs to be sent : return true
    //if xcap needs to be tested return : false
    //currently blocking item at jungo lin.
    public boolean isEPDGConnectionRequired() {
        boolean result = true;


        return result;
    }

    /**
     *
     * IS GBA SUPPORTED BY THE PLATFORM
     * @return
     */
    public boolean isGBASupported() {
        boolean status = false;
        if(isSupportOP08()) {
            //needs to be extracted from API provided by GBA for SIM
            status = true;
        }
        return status;
    }

    /**
     *
     * IS Platform support
     * @return
     */
    public boolean isOP08SupportedByPlatform() {
        boolean status = false;
        String optr = SystemProperties.get("persist.vendor.operator.optr");

        if(optr.equalsIgnoreCase("op08")) {
            status = true;
        }
        return status;
    }

    public String getXDMAuth() {
        String result = null;
        if (instance != null) {
            result = readParameter(RcsSettingsData.XDM_AUTH_TYPE);
        }
        return result;
    }

    /**
     * add parameter and set it false in update config for op08. Keep  it true for OPERATRS THAT support WIFI OTP based
     * Configuration
     * @return
     */
    //needs to add a RCS setting parameter and for TMO it should be false.
    public boolean isWIFI_OTPConfigSupported(){
         boolean status = true;
         if (isSupportOP08()) {
             status = false;
         }
         return status;
    }


    public boolean isIR94VideoCallSupported(){
        //check
        //config && platform && swicth support for vilte
        boolean videoCallSupport = false;

        if (LOGGER.isActivated()) {
            LOGGER.debug("isIR94VideoCallSupportedConfig : "+ isIR94VideoCallSupportedConfig() +
                    " ; isVilteSupportedPlatform : "+isVilteSupportedPlatform() +
                    " ; isVilteSwitchEnabled : "+isVilteSwitchEnabled())
                    ;
        }

        if(isIR94VideoCallSupportedConfig()
                && isVilteSupportedPlatform() // commneted due to API bug
                && isVilteSwitchEnabled()) {
            videoCallSupport = true;
        }
        return videoCallSupport;
    }

    /**
     * IR94 Video Call suppoted in configuration.
     * @return boolean
     */
    public boolean isIR94VideoCallSupportedConfig(){
        //read from system property "persist.mtk_vilte_support"
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.IR94_VIDEO_AUTH));
        }
        return result;
    }

    //is vilte supported by platform
    public boolean isVilteSupportedPlatform(){
        //read from system property "persist.mtk_vilte_support"
        boolean result = false;

        try {
            result = ImsManager.isVtEnabledByPlatform(AndroidFactory.getApplicationContext());
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        return result;
    }

    //is vilte switch on
    public boolean isVilteSwitchEnabled(){
        //read from system property "persist.mtk_vilte_support"
        boolean result = false;
        int switchStatus = 0; //disabled
        try {
            switchStatus = android.provider.Settings.Global.getInt(AndroidFactory.getApplicationContext().getContentResolver(), android.provider.Settings.Global.ENHANCED_4G_MODE_ENABLED);
        } catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        result = (switchStatus == 1)?true : false;
        return result;
    }


    /**
     * Single Registration Type (DUAL, SINGLE, ROAMING)
     * @return
     */

    public int getRcsVolteSingleRegistrationType(){

        int result = 0;
        if (instance != null) {
            try {
                result = Integer.parseInt(readParameter(RcsSettingsData.RCS_VOLTE_SINGLE_REGISTRATION));
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
        return result;

    }

    /**
     * Is mobile connected in roaming
     *
     * @return Boolean
     */
    private boolean isMobileRoaming() {
        boolean result = false;
        try {
            ConnectivityManager connectivityMgr = (ConnectivityManager)AndroidFactory.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
            NetworkInfo netInfo = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
            if (netInfo != null) {
                result = netInfo.isRoaming();
            }
        } catch(Exception e) {
            // Nothing to do
        }
        return result;
    }


    /**
     * Single Registration
     * @return
     */
    public boolean singleRegistrationSupported() {
        boolean result = false;
        Context context = AndroidFactory.getApplicationContext();

        if (!RcsUaService.isAvailable(context))
            return false;

        int registraionType = RcsSettings.getInstance()
                .getRcsVolteSingleRegistrationType();
        LOGGER.debug("Operator single registration allowed state :  "
                + registraionType);

        switch (registraionType) {
            case RcsSettingsData.RCS_VOLTE_SINGLE_REGISTRATION_MODE:
                result = true;
                break;

            case RcsSettingsData.RCS_VOLTE_DUAL_REGISTRATION_ROAMING_MODE:
                result = !isMobileRoaming();
                break;

            case RcsSettingsData.RCS_VOLTE_DUAL_REGISTRATION_MODE:
                result = false;
                break;

            default:
                // nothing to do
                break;
        }

        return result;
    }

    /*
     * in case on op08, user agent should be provided by the
     */
    public String getUserAgent(){

        String userAgent = "T-Mobile VoLTE-RCS-ePDG MTK/2.0";
        if(isGBASupported()){
            userAgent += " 3gpp-gba";
        }
        return userAgent;
    }

    /**
     * Is OP01 package
     *
     * @return Boolean
     */
    public boolean supportOP01() {
        boolean status = false;
        return status;
    }

    /**
     * Is standalone message supported
     *
     * @return Boolean
     */
     public boolean isStandaloneMsgSupport() {
        boolean result = false;
        if(RcsSettings.getInstance().isSupportOP08() || RcsSettings.getInstance().isSupportOP07()){
            result = true;
        } else if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.STANDALONE_MSG_SUPPORT));
        }
        return result;
     }

     /**
      * Get max number of subscrition allowed in Presence list
      *
      * @return max number of subescription
      */
     public int getMaxSubscriptionPresenceList() {
         int result = 100;
         if (instance != null) {
             try {
                 result = Integer.parseInt(readParameter(RcsSettingsData.MAX_SUBSCRIPTION_PRESENCE_LIST));
             } catch(Exception e) {}
         }
         return result;
     }

 /**
      * set operator authorized to connect to RCS platform
      *
      * @param optr
      */

     public void setClientObjDataLimit(String optr) {
        // String result = null;
         if (instance != null) {
             writeParameter(RcsSettingsData.CLIENT_OBJ_DATA_LIMIT, optr);
         }
        // return result;
     }

     /**
      * Get Client Data Limit
      *
      * @return Data Limit
      */
     public int getClientObjDataLimit() {
         int result = 4096;
         if (instance != null) {
             try {
                 result = Integer
                         .parseInt(readParameter(RcsSettingsData.CLIENT_OBJ_DATA_LIMIT));
             } catch (Exception e) {
             }
         }
         return result;
     }


     /**
      * Get Content Server URI
      *
      * @return URI
      */
     public String getContentServerUri() {
         String result = "";
         if (instance != null) {
             try {
                 result = readParameter(RcsSettingsData.CONTENT_SERVER_URI);
             } catch (Exception e) {
             }
         }
         return result;
     }


     /**
      * Get Source Throttle publish value
      *
      * @return Throttle publish value
      */
     public int getSourceThrottlePublish() {
         int result = 30;
         if (instance != null) {
             try {
                 result = Integer
                         .parseInt(readParameter(RcsSettingsData.SOURCE_THROTTLE_PUBLISH));
             } catch (Exception e) {
             }
         }
         return result;
     }


     /**
      * Get Max Subscription presence
      *
      * @return Max Value
      */
     public int getMaxSubscriptionPresence() {
         int result = 100;
         if (instance != null) {
             try {
                 result = Integer
                         .parseInt(readParameter(RcsSettingsData.MAX_SUBSCRIPTION_PRESENCE));
             } catch (Exception e) {
             }
         }
         return result;
     }


     /**
      * Get Service URI Template
      *
      * @return URI
      */
     public String getServiceUriTemplate() {
         String result = "";
         if (instance != null) {
             try {
                 result = readParameter(RcsSettingsData.SERVICE_URI_TEMPLATE);
             } catch (Exception e) {
             }
         }
         return result;
     }


     /**
      * Get RLS URI
      *
      * @return RLS URI
      */
     public String getRlsUri() {
         String result = "";
         if (instance != null) {
             try {
                 result = readParameter(RcsSettingsData.RLS_URI);
             } catch (Exception e) {
             }
         }
         return result;
     }


     /**
      * Get max Non VIP poll
      *
      * @return max non VIP poll
      */
     public int getNonVipMaxPollPeriod() {
         int result = 100;
         if (instance != null) {
             try {
                 result = Integer
                         .parseInt(readParameter(RcsSettingsData.NON_VIP_POLL_PERIOD));
             } catch (Exception e) {
             }
         }
         return result;
     }


     /**
      * Get max non VIP pol per period
      *
      * @return max non VIP pol per period
      */
     public int getNonVipMaxPollPerPeriod() {
         int result = 1;
         if (instance != null) {
             try {
                 result = Integer
                         .parseInt(readParameter(RcsSettingsData.NON_VIP_MAX_POLL_PER_PERIOD));
             } catch (Exception e) {
             }
         }
         return result;
     }

    public String getClientVendor() {
         String result = "";
         if (instance != null) {
             try {
                 result = readParameter(RcsSettingsData.CLIENT_VENDOR);
             } catch (Exception e) {
             }
         }
         return result;
    }


    /**
     * Is RCS Debug Mode
     *
     * @return Boolean
     */
    public boolean isUseDefaultClientVendor() {
        boolean result = true;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.USE_CLIENT_VENDER_SETTINGS));
        }
        return result;
    }


    /**
     * Rcs Service Current state
     *
     *@param current RCs service State
     */
    public void setServiceCurrentState(int labelEnum) {
        if (instance != null) {
            writeParameter(RcsSettingsData.SERVICE_CURRENT_STATE, "" + labelEnum);
        }

    }

    public String getServiceCurrentState() {
         String result = "10";
        if (instance != null) {
            try {
                result = readParameter(RcsSettingsData.SERVICE_CURRENT_STATE);
            } catch (Exception e) {
            }
        }
        return result;
    }

    public boolean sendAtCommand(String command) {
        boolean status = false;

        if (executeCommandResponse(command) == null) {
            if (LOGGER.isActivated()) {
                LOGGER.debug("AT command returned error");
            }
        } else {
            status = true;
        }
        return status;
    }

    private static synchronized String executeCommandResponse(String atCmdLine) {
        String atCmdResult = "";
        byte[] rawData = atCmdLine.getBytes();
        byte[] cmdByte = new byte[rawData.length + 1];
        byte[] respByte = new byte[MAX_BYTE_COUNT + 1];
        System.arraycopy(rawData, 0, cmdByte, 0, rawData.length);
        cmdByte[cmdByte.length - 1] = 0;

        if (LOGGER.isActivated()) {
            LOGGER.debug("sending AT command:" + atCmdLine);
        }

        int ret = MtkTelephonyManagerEx.getDefault().
                invokeOemRilRequestRaw(cmdByte, respByte);
        if (ret > 0) {
            atCmdResult = new String(respByte, 0, ret);
        }

        //Handle CME ERROR
        if (atCmdResult.indexOf("+CME ERROR") != -1) {
            atCmdResult = null;
        }
        return atCmdResult;
    }

    public void setDefaultValuesConfig() {
        if(RcsSettings.getInstance().isOP08SupportedByPlatform()|| RcsSettings.getInstance().isSupportOP07()) {
            writeParameter(RcsSettingsData.AUTO_CONFIG_MODE, new String(""+RcsSettingsData.NO_AUTO_CONFIG));
            RcsSettings.getInstance().setServicePermissionState(true);
            RcsSettings.getInstance().writeParameter(RcsSettingsData.RCS_VOLTE_SINGLE_REGISTRATION,new String(""+RcsSettingsData.RCS_VOLTE_SINGLE_REGISTRATION_MODE));
            RcsSettings.getInstance().writeParameter(RcsSettingsData.IMS_AUTHENT_PROCEDURE_WIFI, RcsSettingsData.AKA_AUTHENT);
            RcsSettings.getInstance().writeParameter(RcsSettingsData.IMS_AUTHENT_PROCEDURE_MOBILE, RcsSettingsData.AKA_AUTHENT);
        }
    }

    public boolean isForcedNatTraversal() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.FORCE_NAT_TRAVERSAL));
        }
        return result;
    }

    public int getMinGroupChatParticipants(){
        return 0;
    }

    public int getGroupChatSubjectMaxLength(){
        return 0;
    }

    /**
     * Get max lenght for extensions using real time messaging (MSRP)
     *
     * @return Max length
     */
    public int getMaxMsrpLengthForExtensions() {

        int result = 0;
        if (instance != null) {
            result = Integer
                    .parseInt(readParameter(RcsSettingsData.MAX_MSRP_SIZE_EXTENSIONS));
        }
        return result;
    }


    /**
     * Get call composer inactivity timeout
     *
     * @return Timeout in milliseconds
     */
    public long getCallComposerInactivityTimeout() {

        long result = 0;
        if (instance != null) {
            result = Long
                    .parseLong(readParameter(RcsSettingsData.CALL_COMPOSER_INACTIVITY_TIMEOUT));
}
        return result;

    }

    public boolean isExtensionAuthorized(String ext) {

        return true;
      /*  return ext != null
                && !(EnrichCallingService.CALL_COMPOSER_FEATURE_TAG.equals(ext) && !isCallComposerSupported())
                && !(EnrichCallingService.SHARED_MAP_SERVICE_ID.equals(ext) && !isSharedMapSupported())
                && !(EnrichCallingService.SHARED_SKETCH_SERVICE_ID.equals(ext) && !isSharedSketchSupported())
                && !(EnrichCallingService.POST_CALL_SERVICE_ID.equals(ext) && !isPostCallSupported());
   */ }

    /**
     * Is call composer supported
     *
     * @return Boolean
     */
  /*  public boolean isCallComposerSupported() {
        return Boolean.parseBoolean(readParameter((RcsSettingsData.CAPABILITY_CALL_COMPOSER));
    }*/

    /**
     * Set default messaging method
     *
     * @param method the default messaging method (0: AUTOMATIC, 1: RCS, 2: NON_RCS)
     */
    public void setDefaultMessagingMethod(MessagingMethod method) {
         writeParameter(RcsSettingsData.DEFAULT_MESSAGING_METHOD, method.toString());
    }
    /**
     * Restrict display name length to 256 characters, as allowing infinite length string as display
     * name will eventually crash the stack.
     *
     * @return the maximum characters allowed for display name
     */
    public int getMaxAllowedDisplayNameChars() {
        return 10;//readInteger(RcsSettingsData.MAX_ALLOWED_DISPLAY_NAME_CHARS);
    }


     /** Get the default messaging method
     *
     * @return the default messaging method (0: AUTOMATIC, 1: RCS, 2: NON_RCS)
     */
    public MessagingMethod getDefaultMessagingMethod() {
        return MessagingMethod.valueOf(readParameter(RcsSettingsData.DEFAULT_MESSAGING_METHOD));
    }
    /**
     * Is configuration valid
     *
     * @return Boolean
     */
    public boolean isConfigurationValid() {
        return Boolean.parseBoolean(readParameter(RcsSettingsData.CONFIGURATION_STATE));
    }

     /**
     * Set configuration valid
     *
     * @param valid true if configuration is valid
     */
    public void setConfigurationValid(boolean valid) {
        //writeParameter(RcsSettingsData.CONFIGURATION_STATE, Boolean.toString(valid));
        setConfigurationState(valid);
    }
     /**
     * Read ContactId parameter
     *
     * @param key the key field
     * @return the value field or defaultValue (if read fails)
     */
    public ContactId readContactId(String key) {
        ContactId value=null;

            String dbValue = readParameter(key);
            if (dbValue != null) {
                value = ContactUtil.createContactIdFromTrustedData(dbValue);

           // mCache.put(key, value);
        }
        return value;
    }

 /**
     * Get user profile username (i.e. username part of the IMPU)
     *
     * @return Username part of SIP-URI or null if not provisioned
     */
    public ContactId getUserProfileImsUserNameContact() {
        return readContactId(RcsSettingsData.USERPROFILE_IMS_USERNAME);
    }

    /**
     * Sets RCS activation changeable by the client applications
     *
     * @param enableSwitch the enable switch
     */
    public void setEnableRcseSwitch(EnableRcseSwitch enableSwitch) {
        writeParameter(RcsSettingsData.ENABLE_RCS_SWITCH, enableSwitch.toString());
    }

    /**
     * Set the client messaging mode
     *
     * @param mode the client messaging mode (0: CONVERGED, 1: INTEGRATED, 2: SEAMLESS, 3: NONE)
     */
    public void setMessagingMode(MessagingMode mode) {
        writeParameter(RcsSettingsData.MESSAGING_MODE, mode.toString());
    }
    /**
     * Get the client messaging mode
     *
     * @return the client messaging mode (0: CONVERGED, 1: INTEGRATED, 2: SEAMLESS, 3: NONE)
     */
    public MessagingMode getMessagingMode() {
        String result=readParameter(RcsSettingsData.MESSAGING_MODE);
        if(result !=null)
            return MessagingMode.valueOf(Integer.parseInt(result));
        return MessagingMode.NONE;
    }


    /**
     * Set the min battery level
     *
     * @param level Battery level in percentage
     */
    public void setMinBatteryLevel(MinimumBatteryLevel level) {
        writeParameter(RcsSettingsData.MIN_BATTERY_LEVEL, level.toString());
    }

    /**
     * Returns how to show the RCS enabled/disabled switch
     *
     * @return EnableRcseSwitch
     */
    public EnableRcseSwitch getEnableRcseSwitch() {
        return EnableRcseSwitch.valueOf(Integer.parseInt(readParameter(RcsSettingsData.ENABLE_RCS_SWITCH)));
    }

    public void setImageResizeOption(ImageResizeOption option){

    }

    public ImageResizeOption getImageResizeOption(){
        ImageResizeOption option = null;
        return option;
    }

    /**
     * Set File Transfer Auto Accepted Mode changeable option
     *
     * @param option Option
     */
    public void setFtAutoAcceptedModeChangeable(boolean option) {
        writeParameter(RcsSettingsData.AUTO_ACCEPT_FT_CHANGEABLE, Boolean.toString(option));
    }

    /**
     * Get max audio message duration of a video share
     *
     * @return Duration in milliseconds
     */
    public boolean isFtAutoAcceptedModeChangeable(){
        return Boolean.parseBoolean(readParameter(RcsSettingsData.AUTO_ACCEPT_FT_CHANGEABLE));
    }

    public boolean isFileTransferAutoAcceptedInRoaming(){
        return Boolean.parseBoolean(readParameter(RcsSettingsData.AUTO_ACCEPT_FT_IN_ROAMING));
    }

    public void setFileTransferAutoAcceptedInRoaming(boolean option){
        writeParameter(RcsSettingsData.AUTO_ACCEPT_FT_IN_ROAMING, Boolean.toString(option));
    }

    public long getMaxAudioMessageDuration(){
        return Long.parseLong(RcsSettingsData.MAX_AUDIO_MESSAGE_DURATION);
    }

    /**
     * Is full group chat Store & Forward supported
     *
     * @return Boolean
     */
    public boolean isGroupChatInviteIfFullStoreForwardSupported() {
        boolean result = false;
        if (instance != null) {
            result = Boolean.parseBoolean(readParameter(RcsSettingsData.CAPABILITY_GROUP_CHAT_SF));
        }
        return result;
    }

     public int getMsgDeliveryTimeoutPeriod(){
        return 0;
    }


    /**
     * Get contact cap validity period in one-one messaging
     *
     * @return Period in milliseconds
     *//*
    public long getMsgCapValidityPeriod() {
        //return readLong(RcsSettingsData.MSG_CAP_VALIDITY_PERIOD);
        return 0;
    }*/

    /**
     * Is send displayed notification activated
     *
     * @return Boolean
     */
    public boolean isRespondToDisplayReports() {
        return Boolean.parseBoolean(readParameter(RcsSettingsData.CHAT_RESPOND_TO_DISPLAY_REPORTS));
    }

    /**
     * Set send displayed notification
     *
     * @param state True if respond to display reports
     */
    public void setRespondToDisplayReports(boolean state) {
        writeParameter(RcsSettingsData.CHAT_RESPOND_TO_DISPLAY_REPORTS, Boolean.toString(state));
    }

    public boolean isTestSim() {
        boolean force_roi_enabled = SystemProperties.get("persist.vendor.rcs.force_roi_enabled").equals("1");
        LOGGER.debug("force_roi_enabled: " + force_roi_enabled);
        return (!force_roi_enabled &&
               (SystemProperties.get("vendor.gsm.sim.ril.testsim").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.2").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.3").equals("1") ||
                SystemProperties.get("vendor.gsm.sim.ril.testsim.4").equals("1")));
    }

    public boolean isVopsOn() {
        boolean isVopsOn = true;
        String result = executeCommandResponse("AT+CIREP?");
        if (result != null) {
            // +CIREP: <reporting>,<nwimsvops>
            // <nwimsvops>: support IMSVOPS or not
            // 0 : VoPS no support
            // 1 : VoPS support
            // +CIREP: 1,0
            try {
                String[] tokens = result.split(",|\r|\n");

                /*
                for (int i = 0; i < tokens.length; i++) {
                    LOGGER.debug(i + ":" + tokens[i]);
                }
                */

                if (tokens.length >= 4) {
                    int vops = Integer.parseInt(tokens[3]);
                    isVopsOn = (vops == 1) ? true : false;
                } else {
                    LOGGER.debug("wrong foramt: " + result);
                }
            } catch (Exception e) {
                LOGGER.error("isVopsOn err: " + e);
            }
        }
        LOGGER.debug("isVopsOn: " + isVopsOn);

        // Enable VOPS URC
        sendAtCommand("AT+CIREP=1");
        return isVopsOn;
    }
}
