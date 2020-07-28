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

package com.mediatek.rcse.settings;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.mediatek.rcse.api.Logger;

import java.util.ArrayList;

/**
 * RCS settings.
 */
public class AppSettings {
    /**
     * Current instance.
     */
    private static AppSettings sInstance = null;

    /**
     * Content resolver.
     */
    private ContentResolver mCr;

    /**
     * Database URI.
     */
    private Uri mDatabaseUri = AppSettingsData.CONTENT_URI;

    /**
     * Tag.
     */
    private final static String TAG = "AppSettings";

    private final static String OP08_OPERATOR_NAME = "T-Mobile";

    /**
     * Logger instance.
     */
    //private final static Logger Logger = Logger.getLogger(TAG);

    /**
     * Create instance.
     *
     * @param ctx            Context
     */
    public static synchronized void createInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new AppSettings(ctx);
        }
    }

    /**
     * Returns instance.
     *
     * @return Instance
     */
    public static AppSettings getInstance() {
        return sInstance;
    }

    /**
     * Constructor.
     *
     * @param ctx            Application context
     */
    private AppSettings(Context ctx) {
        super();

        this.mCr = ctx.getContentResolver();
    }

    /**
     * Read a parameter.
     *
     * @param key            Key
     * @return Value
     */
    public String readParameter(String key) {
        if (key == null) {
            return null;
        }

        String result = null;
        Cursor c = mCr.query(mDatabaseUri, null, AppSettingsData.KEY_KEY + "='"
                + key + "'", null, null);
        if (c != null) {
            if ((c.getCount() > 0) && c.moveToFirst()) {
                result = c.getString(2);
            }
            c.close();
        }
        return result;
    }

    /**
     * Write a parameter.
     *
     * @param key            Key
     * @param value            Value
     */
    public void writeParameter(String key, String value) {
        if ((key == null) || (value == null)) {
            return;
        }

        ContentValues values = new ContentValues();
        values.put(AppSettingsData.KEY_VALUE, value);
        String where = AppSettingsData.KEY_KEY + "='" + key + "'";
        mCr.update(mDatabaseUri, values, where, null);
    }

    /**
     * Insert a parameter.
     *
     * @param key            Key
     * @param value            Value
     */
    public void insertParameter(String key, String value) {
        if ((key == null) || (value == null)) {
            return;
        }

        ContentValues values = new ContentValues();
        values.put(AppSettingsData.KEY_KEY, key);
        values.put(AppSettingsData.KEY_VALUE, value);
        mCr.insert(mDatabaseUri, values);
    }

    /**
     * Get the ringtone for CSh invitation.
     *
     * @return Ringtone URI or null if there is no ringtone
     */
    public String getCShInvitationRingtone() {
        String result = null;
        if (sInstance != null) {
            result = readParameter(AppSettingsData.CSH_INVITATION_RINGTONE);
        }
        return result;
    }

    /**
     * Set the CSh invitation ringtone.
     *
     * @param uri            Ringtone URI
     */
    public void setCShInvitationRingtone(String uri) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.CSH_INVITATION_RINGTONE, uri);
        }
    }

    /**
     * Is phone vibrate for CSh invitation.
     *
     * @return Boolean
     */
    public boolean isPhoneVibrateForCShInvitation() {
        boolean result = false;
        if (sInstance != null) {
            result = Boolean
                    .parseBoolean(readParameter(AppSettingsData.CSH_INVITATION_VIBRATE));
        }
        return result;
    }

    /**
     * Set phone vibrate for CSh invitation.
     *
     * @param vibrate            Vibrate state
     */
    public void setPhoneVibrateForCShInvitation(boolean vibrate) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.CSH_INVITATION_VIBRATE,
                    Boolean.toString(vibrate));
        }
    }

    /**
     * Get the ringtone for file transfer invitation.
     *
     * @return Ringtone URI or null if there is no ringtone
     */
    public String getFileTransferInvitationRingtone() {
        String result = null;
        if (sInstance != null) {
            result = readParameter(AppSettingsData.FILETRANSFER_INVITATION_RINGTONE);
        }
        return result;
    }

    /**
     * Set the file transfer invitation ringtone.
     *
     * @param uri            Ringtone URI
     */
    public void setFileTransferInvitationRingtone(String uri) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.FILETRANSFER_INVITATION_RINGTONE,
                    uri);
        }
    }

    /**
     * Is phone vibrate for file transfer invitation.
     *
     * @return Boolean
     */
    public boolean isPhoneVibrateForFileTransferInvitation() {
        boolean result = false;
        if (sInstance != null) {
            result = Boolean
                    .parseBoolean(readParameter(AppSettingsData.FILETRANSFER_INVITATION_VIBRATE));
        }
        return result;
    }

    /**
     * Set phone vibrate for file transfer invitation.
     *
     * @param vibrate            Vibrate state
     */
    public void setPhoneVibrateForFileTransferInvitation(boolean vibrate) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.FILETRANSFER_INVITATION_VIBRATE,
                    Boolean.toString(vibrate));
        }
    }

    /**
     * Get the ringtone for chat invitation.
     *
     * @return Ringtone URI or null if there is no ringtone
     */
    public String getChatInvitationRingtone() {
        String result = null;
        if (sInstance != null) {
            result = readParameter(AppSettingsData.CHAT_INVITATION_RINGTONE);
        }
        return result;
    }

    /**
     * Set the image compress size.
     *
     * @param size            Quality
     */
    public void setImageCompressSize(String size) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.IMAGE_COMPRESS_SIZE, size);
        }
    }

    /**
     * Get the compression quality set by user.
     *
     * @return Quality set by user in settings
     */
    public String getImageCompressSize() {
        String result = null;
        if (sInstance != null) {
            result = readParameter(AppSettingsData.IMAGE_COMPRESS_SIZE);
        }
        return result;
    }

    /**
     * Set the chat invitation ringtone.
     *
     * @param uri            Ringtone URI
     */
    public void setChatInvitationRingtone(String uri) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.CHAT_INVITATION_RINGTONE, uri);
        }
    }

    /**
     * Is phone vibrate for chat invitation.
     *
     * @return Boolean
     */
    public boolean isPhoneVibrateForChatInvitation() {
        boolean result = false;
        if (sInstance != null) {
            result = Boolean
                    .parseBoolean(readParameter(AppSettingsData.CHAT_INVITATION_VIBRATE));
        }
        return result;
    }

    /**
     * Set phone vibrate for chat invitation.
     *
     * @param vibrate            Vibrate state
     */
    public void setPhoneVibrateForChatInvitation(boolean vibrate) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.CHAT_INVITATION_VIBRATE,
                    Boolean.toString(vibrate));
        }
    }

    /**
     * Check whether compressing image when send image. Do not call this method
     * in ui thread
     *
     * @return True if compressing image is enabled, otherwise return false.
     */
    public boolean isEnabledCompressingImageFromDB() {
        String result = null;
        if (sInstance != null) {
            result = readParameter(AppSettingsData.RCSE_COMPRESSING_IMAGE);
            if (Logger.isActivated()) {
                Logger.d(TAG,"isEnabledCompressingImageFromDB(), result: "
                        + result);
            }
            return Boolean.valueOf(result);
        }
        return true;
    }

    /**
     * Set the status which indicate whether compressing image when send image.
     *
     * @param state
     *            True if compressing image is enabled, otherwise return false.
     */
    public void setCompressingImage(final boolean state) {
        if (Logger.isActivated()) {
            Logger.d(TAG,"setCompressingImage(), state: " + state);
        }
        writeParameter(AppSettingsData.RCSE_COMPRESSING_IMAGE,
                Boolean.toString(state));
    }

    /**
     * Check whether compressing image when send image. Do not call this method
     * in ui thread
     *
     * @return True if compressing image is enabled, otherwise return false.
     */
    public boolean isEnabledCompressingVideoFromDB() {
        String result = null;
        if (sInstance != null) {
            result = readParameter(AppSettingsData.RCSE_COMPRESSING_VIDEO);
            if (Logger.isActivated()) {
                Logger.d(TAG,"isEnabledCompressingVideoFromDB(), result: "
                        + result);
            }
            return Boolean.valueOf(result);
        }
        return true;
    }

    /**
     * Set the status which indicate whether compressing image when send image.
     *
     * @param state
     *            True if compressing image is enabled, otherwise return false.
     */
    public void setCompressingVideo(final boolean state) {
        if (Logger.isActivated()) {
            Logger.d(TAG,"setCompressingVideo(), state: " + state);
        }
        writeParameter(AppSettingsData.RCSE_COMPRESSING_VIDEO,
                Boolean.toString(state));
    }

    /**
     * Get the remind flag.
     *
     * @return True if need remind compress again when send image, otherwise
     *         return false
     */
    public boolean restoreRemindCompressFlag() {
        String result = null;
        if (sInstance != null) {
            result = readParameter(AppSettingsData.COMPRESS_IMAGE_HINT);
            if (Logger.isActivated()) {
                Logger.d(TAG,"restoreRemindCompressFlag(), result: " + result);
            }
            return Boolean.valueOf(result);
        }
        if (Logger.isActivated()) {
            Logger.d(TAG,"instance is null, return false");
        }
        return false;
    }

    /**
     * Set the remind flag.
     *
     * @param notRemind            Indicates whether need to remind user to compress image when
     *            send image
     */
    public void saveRemindCompressFlag(final boolean notRemind) {
        if (Logger.isActivated()) {
            Logger.d(TAG,"saveRemindFlag(), notRemind: " + notRemind);
        }
        writeParameter(AppSettingsData.COMPRESS_IMAGE_HINT,
                Boolean.toString(!notRemind));
    }

    /**
     * Set the warning remind flag.
     *
     * @param notRemindFlag the not remind flag
     */
    public void saveRemindWarningLargeImageFlag(final boolean notRemindFlag) {
        if (Logger.isActivated()) {
            Logger.d(TAG,"saveRemindWarningLargeImageFlag(), notRemindFlag: "
                    + notRemindFlag);
        }
        writeParameter(AppSettingsData.WARNING_LARGE_IMAGE_HINT,
                Boolean.toString(!notRemindFlag));
    }

    /**
     * Get the warning remind flag.
     *
     * @return True if need remind Large image again when send image, otherwise
     *         return false
     */
    public boolean restoreRemindWarningLargeImageFlag() {
        String result = null;
        if (sInstance != null) {
            result = readParameter(AppSettingsData.WARNING_LARGE_IMAGE_HINT);
            if (Logger.isActivated()) {
                Logger.d(TAG,"restoreRemindWarningLargeFlag(), result: "
                        + result);
            }
            return Boolean.valueOf(result);
        }
        if (Logger.isActivated()) {
            Logger.d(TAG,"instance is null, return false");
        }
        return false;
    }

    /**
     *  M: ftAutAccept @{.
     *
     * @return the boolean
     */
    /**
     * Whether it is enable to auto accept ft when roaming.
     *
     * @return whether it is enable.
     */
    public Boolean isEnableFtAutoAcceptWhenRoaming() {
        Boolean result = Boolean.FALSE;
        if (sInstance != null) {
            result = Boolean
                    .parseBoolean(readParameter(AppSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING));
        }
        if (Logger.isActivated()) {
            Logger.d(TAG,"isEnableFtAutoAcceptWhenRoaming() result: " + result);
        }
        return result;
    }

    /**
     * Enable or disable to auto-accept ft when roaming.
     *
     * @param enable            True to be enable, otherwise false.
     */
    public void setEnableFtAutoAcceptWhenRoaming(Boolean enable) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.ENABLE_AUTO_ACCEPT_FT_ROMING,
                    Boolean.toString(enable));
        }
        if (Logger.isActivated()) {
            Logger.d(TAG,"setEnableFtAutoAcceptWhenRoaming() enable: " + enable);
        }
    }

    /**
     * Is Store & Forward service warning activated
     *
     * @return Boolean
     */
    public boolean isStoreForwardWarningActivated() {
        boolean result = false;
        if (sInstance != null) {
            result = Boolean.parseBoolean(readParameter(AppSettingsData.WARN_SF_SERVICE));
        }
        if (Logger.isActivated()) {
            Logger.d(TAG,"isStoreForwardWarningActivated() enable: " + result);
        }
        return result;
    }

    /**
     * Is IM always-on thanks to the Store & Forward functionality
     *
     * @return Boolean
     */
    public boolean isImAlwaysOn() {
        boolean result = false;
        if (sInstance != null) {
            result = Boolean.parseBoolean(readParameter(AppSettingsData.IM_CAPABILITY_ALWAYS_ON));
        }
        if (Logger.isActivated()) {
            Logger.d(TAG,"isImAlwaysOn() enable: " + result);
        }
        return result;
    }

    /**
     *  M: ftAutAccept no roaming @{.
     *
     * @return the boolean
     */
    /**
     * Whether it is enable to auto accept ft when no roaming.
     *
     * @return whether it is enable.
     */
    public Boolean isEnableFtAutoAcceptWhenNoRoaming() {
        Boolean result = Boolean.FALSE;
        if (sInstance != null) {
            result = Boolean
                    .parseBoolean(readParameter(AppSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING));
        }
        if (Logger.isActivated()) {
            Logger.d(TAG,"isEnableFtAutoAcceptWhenNoRoaming() result: "
                    + result);
        }
        return result;
    }

    /**
     * Enable or disable to auto-accept ft when no roaming.
     *
     * @param enable            True to be enable, otherwise false.
     */
    public void setEnableFtAutoAcceptWhenNoRoaming(Boolean enable) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.ENABLE_AUTO_ACCEPT_FT_NOROMING,
                    Boolean.toString(enable));
        }
        if (Logger.isActivated()) {
            Logger.d(TAG,"setEnableFtAutoAcceptWhenNoRoaming() enable: "
                    + enable);
        }
    }

    /**
     * M: Add to achieve the RCS-e set chat wall paper feature. @{
     */
    // The resource id of chat wall paper copy in memory
    private String mChatWallpaper = null;
    private final ArrayList<OnWallPaperChangedListener> mOnWallPaperChangedListenerList =
            new ArrayList<OnWallPaperChangedListener>();

    /**
     * Register wall paper changed listener.
     *
     * @param listener the listener
     */
    public void registerWallPaperChangedListener(
            OnWallPaperChangedListener listener) {
        if (Logger.isActivated()) {
            Logger.d(TAG,"registerWallPaperChangedListener, listener: "
                    + listener);
        }
        mOnWallPaperChangedListenerList.add(listener);
    }

    /**
     * Unregister wall paper changed listener.
     *
     * @param listener the listener
     */
    public void unregisterWallPaperChangedListener(
            OnWallPaperChangedListener listener) {
        if (Logger.isActivated()) {
            Logger.d(TAG,"unregisterWallPaperChangedListener: " + listener);
        }
        mOnWallPaperChangedListenerList.remove(listener);
    }

    /**
     * Interface definition for a callback to be invoked when the RCS-e chat
     * wall paper is changed.
     *
     * @see OnWallPaperChangedEvent
     */
    public interface OnWallPaperChangedListener {
        /**
         * Called when the RCS-e chat wall paper is changed.
         *
         * @param wallPaper
         *            The wall paper's full file name or resource id.
         */
        public void onWallPaperChanged(String wallPaper);
    }

    /**
     * Set chat wall paper resource. This method can be called on UI thread.
     *
     * @param chatWallpaper
     *            The resource id of chat wall paper or the file name of chat
     *            wall paper.
     */
    public void setChatWallpaper(final String chatWallpaper) {
        if (chatWallpaper == null) {
            if (Logger.isActivated()) {
                Logger.d(TAG,"setChatWallpaperId invalid chatWallpaper. chatWallpaper is null");
            }
            return;
        }
        if (sInstance != null) {
            // Save it in memory when write it to database.
            mChatWallpaper = chatWallpaper;
            for (OnWallPaperChangedListener listener : mOnWallPaperChangedListenerList) {
                listener.onWallPaperChanged(chatWallpaper);
            }
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    writeParameter(AppSettingsData.RCSE_CHAT_WALLPAPER,
                            chatWallpaper);
                }
            });
        }
    }

    /**
     * Get is Messaging Disable and Fully integrated mode is on.
     *
     * @return The Messaging Disable and Fully integrated mode.
     */

    public boolean getJoynMessagingDisabledFullyIntegrated() {
        boolean result = false;
        if (sInstance != null) {
            result = Boolean
                    .parseBoolean(readParameter(
                            AppSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED));
        }
        return result;
    }

    /**
     * Gets the disable service status.
     *
     * @return the disable service status
     */
    public int getDisableServiceStatus() {
        int result = 0;
        if (sInstance != null) {
            try {
                result = Integer
                        .parseInt(readParameter(AppSettingsData.JOYN_DISABLE_STATUS));
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }
        }
        return result;
    }

    /**
     * Sets the disable service status.
     *
     * @param status the new disable service status
     */
    public void setDisableServiceStatus(int status) {
        if (sInstance != null) {
            writeParameter(AppSettingsData.JOYN_DISABLE_STATUS, "" + status);
        }
    }

    /**
     * Set is Messaging Disable and Fully integrated mode is on.
     *
     * @param state the new joyn messaging disabled fully integrated
     */

    public void setJoynMessagingDisabledFullyIntegrated(boolean state) {
        if (sInstance != null) {
            writeParameter(
                    AppSettingsData.JOYN_MESSAGING_DISABLED_FULLY_INTEGRATED,
                    Boolean.toString(state));
        }
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
     * @return The resource id of chat wall paper or the file name of chat wall
     *         paper.
     */
    public String getChatWallpaper() {
        if (Logger.isActivated()) {
            Logger.d(TAG,"getChatWallpaper() from memory, mChatWallpaper: "
                    + mChatWallpaper);
        }
        return mChatWallpaper;
    }
}
