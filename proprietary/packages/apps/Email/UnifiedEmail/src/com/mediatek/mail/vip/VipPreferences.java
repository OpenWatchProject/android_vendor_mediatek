package com.mediatek.mail.vip;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

public class VipPreferences {
    // Preferences file
    public static final String VIP_PREFERENCES_FILE = "VipPreferences";
    /**M: Support for VIP settings @{*/
    public static final String VIP_NOTIFICATION = "vip_notification";
    public static final String VIP_RINGTONE = "vip_ringtone";
    public static final String VIP_VIBRATE = "vip_vibrate";
    public static final boolean VIP_NOTIFICATION_DEFAULT = true;
    public static final String VIP_RINGTONE_DEFAULT = "content://settings/system/notification_sound";
    public static final boolean VIP_VIBATATE_DEFAULT = false;
    /** @} */
    /**M: Support for VIP settings @{*/
    public static final String PERFERENCE_KEY_VIPSETTINGS = "vip_settings";
    public static final String PERFERENCE_KEY_VIP_MEMBERS = "vip_members";
    /** @} */

    private static VipPreferences sInstance;

    private final SharedPreferences mSharedPreferences;
    private final Editor mEditor;

    private VipPreferences(Context context) {
        mSharedPreferences = context.getSharedPreferences(VIP_PREFERENCES_FILE, Context.MODE_PRIVATE);
        mEditor = mSharedPreferences.edit();
    }

    public static VipPreferences get(Context context) {
        if (sInstance == null) {
            sInstance = new VipPreferences(context);
        }
        return sInstance;
    }

    /**M: Support for VIP settings @{*/
    public boolean getVipNotification() {
        return mSharedPreferences.getBoolean(VIP_NOTIFICATION, VIP_NOTIFICATION_DEFAULT);
    }

    public void setVipNotification(boolean notify) {
        mEditor.putBoolean(VIP_NOTIFICATION, notify).apply();
    }

    public String getVipRingtone() {
        return mSharedPreferences.getString(VIP_RINGTONE, VIP_RINGTONE_DEFAULT);
    }

    public void setVipRingtone(String ringtone) {
        mEditor.putString(VIP_RINGTONE, ringtone).apply();
    }

    public boolean getVipVebarate() {
        return mSharedPreferences.getBoolean(VIP_VIBRATE, VIP_VIBATATE_DEFAULT);
    }

    public void setVipVebarate(boolean vibrate) {
        mEditor.putBoolean(VIP_VIBRATE, vibrate).apply();
    }
    /** @} */
}
