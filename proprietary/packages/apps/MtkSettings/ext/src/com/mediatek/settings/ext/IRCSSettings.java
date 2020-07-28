package com.mediatek.settings.ext;

import android.app.Activity;
import android.content.Context;
import android.support.v7.preference.PreferenceScreen;
import android.telephony.SubscriptionInfo;

import java.util.List;

public interface IRCSSettings {
    /**
     * Add rcs setting preference in wireless settings.
     * @param activity The activity of wireless settings
     * @param screen The PreferenceScreen of wireless settings
     * @internal
     */
    void addRCSPreference(Activity activity, PreferenceScreen screen);

    /**
     * Judge whether or not  the AskFirstItem should be reserved.
     * @return true if plug-in want to go host flow.
     * @internal
     */
    boolean isNeedAskFirstItemForSms();

    /**
     * Get default SmsClickContent.
     * @param subInfoList SubscriptionInfo
     * @param value Value
     * @param subId Subid
     * @return subId.
     * @internal
     */
    int getDefaultSmsClickContentExt(final List<SubscriptionInfo> subInfoList,
            int value, int subId);

    /**
     * Sets the specified package as the default SMS/MMS application.
     * The caller of this method needs to have permission to set AppOps
     * and write to secure settings.
     * @param packageName package name of default SMS application to set
     * @param context Context
     * @internal
     */
    void setDefaultSmsApplication(String packageName, Context context);
}
