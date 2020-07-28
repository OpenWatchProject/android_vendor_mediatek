/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.rcse.plugin.contacts;

import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.provider.ContactsContract.Contacts;
import android.util.Log;
import com.mediatek.rcse.service.CoreApplication;
import com.mediatek.rcse.service.MediatekFactory;

import com.mediatek.rcse.api.Logger;
import com.mediatek.rcse.service.ApiManager;
import com.mediatek.rcse.service.PluginApiManager;
import com.mediatek.rcs.R;

import java.util.HashMap;
import java.util.List;

/**
 * This class defined to implement the function interface of IContactExtention,
 * and achieve the main function here
 */
public class ContactExtention extends ContextWrapper {

    private static final String TAG = "ContactExtention";
    private static final int RCS_PRESENCE = 1;
    private final HashMap<OnPresenceChangedListener, Long> mOnPresenceChangedListenerList =
            new HashMap<OnPresenceChangedListener, Long>();
    private static final String RCS_MIMETYPE =
            "vnd.android.cursor.item/com.orangelabs.rcs.rcs-status";
    private PluginApiManager mInstance = null;
    private Context mContext = null;
    Resources mResource = null;

    public ContactExtention(Context context) {
        super(context);
        mContext = context;
        PluginApiManager.initialize(context);
        mInstance = PluginApiManager.getInstance();
        try {
            mResource = mContext
                    .getPackageManager()
                    .getResourcesForApplication(CoreApplication.APP_NAME);
        } catch (android.content.pm.PackageManager.NameNotFoundException e) {
            e.printStackTrace();
        }
    }

    public Context getContext() {
    	return mContext;
    }

    public Drawable getAppIcon() {
        Drawable drawable = null;
        if (mResource != null) {
            drawable = mResource.getDrawable(R.drawable.icon_contact_indicaton);
        }
        return drawable;
    }

    public List<String> getNumbersByContactId(long contactId) {
    	return mInstance.getNumbersByContactId(contactId);
    }

    public int getUnreadMessageCount(String contact)
    {
    	return mInstance.getUnreadMessageCount(contact);
    }

    public boolean blockContact(String contact, boolean status) throws RemoteException
    {
        boolean isBlockedSuccess = mInstance.blockContact(contact, status);
        return isBlockedSuccess;
    }

    public boolean getBlockedStatus(String contact) throws RemoteException
    {
        boolean isBlockedAlready = mInstance.getBlockedStatus(contact);
        return isBlockedAlready;
    }

    public boolean isEnabled() {
        boolean isEnable = mInstance.getRegistrationStatus();
        return isEnable;
    }

    public Drawable getContactPresence(long contactId) {
        Drawable drawable = null;
        int presence = mInstance.getContactPresence(contactId);
        if (presence == RCS_PRESENCE) {
            if (mResource != null) {
                drawable = mResource.getDrawable(R.drawable.icon_contact_indicaton);
            }
        }
        Logger.d(TAG, "getContactPresence, contactId is:" + contactId + " presence:" + presence);
        return drawable;
    }

    public Action[] getContactActions() {
        Action[] actions = new Action[2];
        Drawable imDrawable = null;
        Drawable ftDrawable = null;
        if (mResource != null) {
            imDrawable = mResource.getDrawable(R.drawable.btn_start_chat_nor);
            ftDrawable = mResource.getDrawable(R.drawable.btn_start_share_nor);
        }

        Intent imIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
        imIntent.putExtra(PluginApiManager.RcseAction.IM_ACTION, true);
        Intent ftIntent = new Intent(PluginApiManager.RcseAction.PROXY_ACTION);
        ftIntent.putExtra(PluginApiManager.RcseAction.FT_ACTION, true);
        Action imAction = new Action();
        imAction.intentAction = imIntent;
        imAction.icon = imDrawable;
        actions[0] = imAction;
        Action ftAction = new Action();
        ftAction.intentAction = ftIntent;
        ftAction.icon = ftDrawable;
        actions[1] = ftAction;
        return actions;
    }

    public Drawable getBlockingDrawable(String contact)
    {
    	Drawable blockingDrawable = null;
    	boolean isBlocked = false;
    	try{
    	    isBlocked = getBlockedStatus(contact);
    	}
    	catch(Exception e)
    	{
    	    e.printStackTrace();
    	}
    	if (mResource != null) {
    	    if(isBlocked == false)
    	    {
    		blockingDrawable = mResource.getDrawable(R.drawable.block_contact);
        }
    	    else
    	    {
    	        blockingDrawable = mResource.getDrawable(R.drawable.add_to_black);
    	    }
        }
    	return blockingDrawable;
    }

    public Drawable getXMSDrawable()
    {
    	Drawable XMSDrawable = null;
    	if (mResource != null) {
    		XMSDrawable = mResource.getDrawable(R.drawable.ic_launcher_smsmms);
        }
    	return XMSDrawable;
    }

    public String getBlockingTitle()
    {
        String blockingTitle = null;
        if (mResource != null) {
            blockingTitle = mResource.getString(R.string.block_contact_title);
        }
        return blockingTitle;
    }

    public boolean isImSupported(final String number) {
    	if(mInstance != null)
    	{
    		return mInstance.isImSupportedInCapability(number);
    	} else{
    		return false;
    	}
    }

    public boolean isFtSupported(final String number)
    {
    	if(mInstance != null)
    	{
    		return mInstance.isFtSupportedInCapability(number);
    	} else{
    		return false;
    	}
    }
    public void addOnPresenceChangedListener(OnPresenceChangedListener listener, long contactId) {
        mInstance.addOnPresenceChangedListener(listener, contactId);
    }

    public String getAppTitle() {
        Logger.d(TAG, "getAppTitle entry");
        String title = null;
        if (mResource != null) {
            title = mResource.getString(R.string.joyn_title);
        } else {
            Logger.d(TAG, "getAppTitle mResource is null");
        }
        Logger.d(TAG, "getAppTitle exit");
        return title;
    }

    public String getMimeType() {
        return RCS_MIMETYPE;
    }

    public void onContactDetailOpen(final Uri contactLookUpUri) {
        Logger.d(TAG, "onContactDetailOpen() contactLookUpUri: " + contactLookUpUri);
        if (null != contactLookUpUri) {
            final ContentResolver contentResolver = getContentResolver();
            AsyncTask.execute(new Runnable() {
                @Override
                public void run() {
                    Cursor cursor = null;
                    long contactId = -1;
                    try {
                        cursor = contentResolver.query(contactLookUpUri, new String[] {
                            Contacts._ID
                        }, null, null, null);
                        if (null == cursor || !cursor.moveToFirst()) {
                            Logger.e(TAG, "onContactDetailOpen() error when loading cursor");
                            return;
                        }
                        int indexContactId = cursor.getColumnIndex(Contacts._ID);
                        do {
                            contactId = cursor.getLong(indexContactId);
                        } while (cursor.moveToNext());
                    } finally {
                        if (null != cursor) {
                            cursor.close();
                        }
                    }
                    if (-1 != contactId) {
                        checkCapabilityByContactId(contactId);
                    } else {
                        Logger.w(TAG, "onContactDetailOpen() contactLookUpUri " + contactLookUpUri);
                    }
                }
            });
        } else {
            Logger.w(TAG, "onContactDetailOpen() contactLookUpUri is null");
        }
    }

    private void checkCapabilityByContactId(long contactId) {
        Logger.d(TAG, "checkCapabilityByContactId() contactId: " + contactId);
        PluginApiManager apiManager = PluginApiManager.getInstance();
        apiManager.queryNumbersPresence(apiManager.getNumbersByContactId(contactId));
    }

    public static final class Action {
        public Intent intentAction;

        public Drawable icon;
    }

    /**
     * Interface for plugin to call back host that presence has changed.
     */
    public interface OnPresenceChangedListener {
        /**
         * Call back when presence changed.
         *
         * @param contactId The contact id.
         * @param presence The presence.
         */
        void onPresenceChanged(long contactId, int presence);
    }

    /**
     *show rcs-e icon on the Detail Actvitiy's action bar
     *
     * @return null if there shouldn't show rcs-e icon.
     */
    public Drawable getRCSIcon() {
        // Get the plug-in Resources instance by the plug-in Context instance
        if (mResource == null) {
            Logger.d(TAG, "getRCSIcon()-the plugin mResource is null");
            return null;
        }
        Drawable drawable = mResource.getDrawable(R.drawable.ic_rcse_indicaton);
        return drawable;
    }

    public Drawable getRCSAvailableIcon() {
        // Get the plug-in Resources instance by the plug-in Context instance
        if (mResource == null) {
            Logger.d(TAG, "getRCSIcon()-the plugin mResource is null");
            return null;
        }
        Drawable drawable = mResource.getDrawable(R.drawable.ic_online_indicator_holo_light);
        return drawable;
    }

    public Drawable getRCSOfflineIcon() {
        // Get the plug-in Resources instance by the plug-in Context instance
        if (mResource == null) {
            Logger.d(TAG, "getRCSIcon()-the plugin mResource is null");
            return null;
        }
        Drawable drawable = mResource.getDrawable(R.drawable.ic_online_indicator_holo_gray);
        return drawable;
    }

}
