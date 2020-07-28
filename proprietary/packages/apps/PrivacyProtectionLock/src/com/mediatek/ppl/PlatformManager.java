/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ppl;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.UserHandle;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.PhoneLookup;
import android.provider.Settings;
import android.telephony.SmsManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.ppl.MessageManager.PendingMessage;
import com.mediatek.ppl.R;
import com.mediatek.ppl.Utils;
import com.mediatek.provider.MtkSettingsExt;
import vendor.mediatek.hardware.pplagent.V1_0.IPplAgent;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;

public class PlatformManager {
    protected static final String TAG = "PPL/PlatformManager";

    private static final String CHANNEL_ID = "PPL_CHANNEL";
    private static final String ENCRYPTION_TYPE = "SHA-256";
    private final Context mContext;
    private final SmsManager mSmsManager;
    private final ConnectivityManager mConnectivityManager;
    private final WakeLock mWakeLock;

    private IPplAgent mAgent;

    private static boolean isForeground = false;
    public static int SIM_NUMBER = 1;

    public PlatformManager(Context context) {
        mContext = context;
        TelephonyManager telephonyManager = new TelephonyManager(context);
        SIM_NUMBER = telephonyManager.getSimCount();
        Log.d(TAG, "SIM_NUMBER=" + SIM_NUMBER);

        try {
            mAgent = IPplAgent.getService();
        } catch (Exception e) {
            Log.e(TAG, "IPplAgent init fail !!!");
        }
        mSmsManager = SmsManager.getDefault();
        mConnectivityManager = (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "PPL_WAKE_LOCK");
    }

    public IPplAgent getPPLAgent() {
        return mAgent;
    }

    public void sendTextMessage(String destinationAddress, long id, String text, Intent sentIntent, int simId) {
        Log.d(TAG, "sendTextMessage(" + Utils.stringForSecureLog(destinationAddress)
                + ", " + id + ", " + simId + ")");
        ArrayList<String> segments = divideMessage(simId, text);
        ArrayList<PendingIntent> pis = new ArrayList<PendingIntent>(segments.size());
        final int total = segments.size();
        for (int i = 0; i < total; ++i) {
            Intent intent = new Intent(sentIntent);
            Uri.Builder builder = new Uri.Builder();
            builder.authority(MessageManager.SMS_PENDING_INTENT_DATA_AUTH)
                    .scheme(MessageManager.SMS_PENDING_INTENT_DATA_SCHEME).appendPath(Long.toString(id))
                    .appendPath(Integer.toString(total)).appendPath(Integer.toString(i));
            Log.d(TAG, "sendTextMessage: uri string is sms://ppl/<number>/" +
                    Integer.toString(total) + '/' + Integer.toString(i));
            intent.setData(builder.build());

            byte type = intent.getByteExtra(PendingMessage.KEY_TYPE, MessageManager.Type.INVALID);
            String number = intent.getStringExtra(PendingMessage.KEY_NUMBER);
            Log.d(TAG, "id is " + id + ", type is " + type + ", number is " +
                        Utils.stringForSecureLog(number));

            PendingIntent pi = PendingIntent.getBroadcast(mContext, 0, intent, PendingIntent.FLAG_ONE_SHOT);
            pis.add(pi);
        }
        sendMultipartTextMessage(destinationAddress, null, segments, pis, simId);
    }

    protected ArrayList<String> divideMessage(int simId, String text) {
        int subId = getSubscriptionId(simId);
        Log.d(TAG, "divideMessage: simId = " + simId + ", subId = "
                + subId);
      return  SmsManager.getSmsManagerForSubscriptionId(subId).divideMessage(text);
    }

    protected void sendMultipartTextMessage(final String destinationAddress,
            final String scAddress, final ArrayList<String> parts,
            final ArrayList<PendingIntent> sentIntents, final int simId) {
        int subId = getSubscriptionId(simId);
        Log.d(TAG, "sendMultipartTextMessage: simId = " + simId + ", subId = "
                + subId);

        if (subId > 0) {
            SmsManager.getSmsManagerForSubscriptionId(subId)
                    .sendMultipartTextMessage(destinationAddress, scAddress,
                            parts, sentIntents, null);
            Log.d(TAG, "sendMultipartTextMessage: message sent out.");
        } else {
            new AsyncTask<Integer, Void, Integer>() {
                @Override
                protected Integer doInBackground(Integer... params) {
                    while (true) {
                        try {
                            Thread.sleep(500);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                        }
                        int subId = getSubscriptionId(params[0]);
                        if (subId > 0) {
                            Log.d(TAG, "doInBackground got subId: simId = "
                                    + params[0] + ", subId = " + subId);
                            return subId;
                        }
                    }
                }

                @Override
                protected void onPostExecute(Integer result) {
                    SmsManager.getSmsManagerForSubscriptionId(result)
                            .sendMultipartTextMessage(destinationAddress,
                                    scAddress, parts, sentIntents, null);
                    Log.d(TAG, "onPostExecute: message sent out.");
                }
            }.execute(simId);
        }
    }

    private int getSubscriptionId(int slotId) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        int subId;
        if (subIds != null) {
            subId = subIds[0];
        } else {
            subId = SubscriptionManager.getDefaultSubscriptionId();
        }
        return subId;
    }

    public void setMobileDataEnabled(Context context, boolean enable) {
        try {
            TelephonyManager telephonyManager = new TelephonyManager(context);
            telephonyManager.setDataEnabled(enable);
        } catch (NullPointerException e) {
            Log.e(TAG, "TelephonyManager is not ready");
        }
    }

    public void acquireWakeLock() {
        mWakeLock.acquire();
    }

    public void releaseWakeLock() {
        mWakeLock.release();
    }

    public void stayForeground(Service service) {
        if (isForeground) {
            return;
        }
        Log.i(TAG, "Bring service to foreground");
        final NotificationManager nm = service.getSystemService(NotificationManager.class);
        final NotificationChannel channel = new NotificationChannel(CHANNEL_ID,
                service.getString(R.string.status_control_panel_enabled),
                NotificationManager.IMPORTANCE_LOW);
        nm.createNotificationChannel(channel);
        Notification notification = new Notification.Builder(service)
                .setChannelId(CHANNEL_ID)
                .setSmallIcon(R.drawable.security).build();
        service.startForeground(1, notification);
        isForeground = true;
    }

    public void leaveForeground(Service service) {
        if (isForeground) {
            Log.d(TAG, "Exec stopForeground with para true.");
            service.stopForeground(true);
            isForeground = false;
        }
    }

    /**
     * TelephonyManager may be unavailable in certain circumstance such as data encrypting process of the phone.
     * @return  Whether TelephonyManager is available.
     */
    @SuppressWarnings("unused")
    public static boolean isTelephonyReady(Context context) {
        boolean teleEnable = true;

        try {
            TelephonyManager tm = (TelephonyManager) context
                    .getSystemService(Context.TELEPHONY_SERVICE);
            tm.hasIccCard(0);

        } catch (NullPointerException e) {
            Log.e(TAG, "TelephonyManager is not ready");
            teleEnable = false;
        }

        return teleEnable;
    }


   public static List<PplSimInfo> buildSimInfo(Context context) {
        
        List<SubscriptionInfo> simItem = SubscriptionManager.from(context)
                .getActiveSubscriptionInfoList();

        if (null == simItem) {
            Log.i(TAG, "Subscription info list is null!");
            return null;
        }
        Log.i(TAG, "simItem: " + simItem.size());

        // sort the unordered list
        Collections.sort(simItem, new SimInfoComparable());

        List<PplSimInfo> data = new ArrayList<PplSimInfo>();
        for (int i = 0; i < simItem.size(); i++) {
            PplSimInfo map = new PplSimInfo(simItem.get(i).getDisplayName().toString(),
                                        simItem.get(i).createIconBitmap(context),
                                        simItem.get(i).getIconTint());

            Log.i(TAG, "mSimSlotId: " + simItem.get(i).getSimSlotIndex());
            Log.i(TAG, "mDisplayName: " + simItem.get(i).getDisplayName());
            Log.i(TAG, "Image: " + simItem.get(i).getIconTint());

            data.add(map);
        }

        return data;
    }


    private static class SimInfoComparable implements Comparator<SubscriptionInfo> {
        @Override
         public int compare(SubscriptionInfo sim1, SubscriptionInfo sim2) {
            return sim1.getSimSlotIndex() - sim2.getSimSlotIndex();
         }
     }

    private static boolean isAllRadioOff(Context context) {
        int airMode = Settings.Global.getInt(context.getContentResolver(),
                                             Settings.Global.AIRPLANE_MODE_ON, -1);
        int msimMode = Settings.System.getInt(context.getContentResolver(),
                                              MtkSettingsExt.System.MSIM_MODE_SETTING, -1);
        return airMode == 1 || msimMode == 0;
    }

    /**
     * Query name for number.
     *
     * @param context
     * @param number
     * @return
     */
    public static String getContactNameByPhoneNumber(Context context, String number) {
        ContentResolver contentResolver = context.getContentResolver();
        Uri uri = Uri.withAppendedPath(ContactsContract.PhoneLookup.CONTENT_FILTER_URI, Uri.encode(number));
        String[] projection = new String[] { PhoneLookup.DISPLAY_NAME, PhoneLookup._ID };
        Cursor cursor = contentResolver.query(uri, projection, null, null, null);
        if (null == cursor) {
            return null;
        }
        String id = null;
        if (cursor.moveToFirst()) {
            id = cursor.getString(cursor.getColumnIndex(PhoneLookup._ID));
        }
        cursor.close();
        if (null == id) {
            return null;
        }

        // Build the Entity URI.
        Uri.Builder b = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_URI, id).buildUpon();
        b.appendPath(ContactsContract.Contacts.Entity.CONTENT_DIRECTORY);
        Uri contactUri = b.build();
        Log.d(TAG, "XXX: contactUri is " + contactUri);
        // Create the projection (SQL fields) and sort order.
        projection = new String[] {
                ContactsContract.Contacts.Entity.RAW_CONTACT_ID,
                ContactsContract.Contacts.Entity.DATA1,
                ContactsContract.Contacts.Entity.MIMETYPE
        };
        String sortOrder = ContactsContract.Contacts.Entity.RAW_CONTACT_ID + " ASC";
        cursor = context.getContentResolver().query(contactUri, projection, null, null, sortOrder);
        if (null == cursor) {
            return null;
        }
        String name = null;
        int mimeIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.MIMETYPE);
        int dataIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.DATA1);
        if (cursor.moveToFirst()) {
            do {
                String mime = cursor.getString(mimeIdx);
                if (mime.equalsIgnoreCase(CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)) {
                    name = cursor.getString(dataIdx);
                    break;
                }
            } while (cursor.moveToNext());
        }
        cursor.close();

        return name;
    }

    public static class ContactQueryResult {
        public String name;
        public ArrayList<String> phones;

        public ContactQueryResult() {
            name = null;
            phones = new ArrayList<String>();
        }
    }


    /**
     * Query name and numbers for specified contact.
     *
     * @param context
     * @param uri       URI of contact.
     * @return
     */
    public static ContactQueryResult getContactInfo(Context context, Uri uri) {
        ContactQueryResult result = new ContactQueryResult();
        String id = null;
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        if (null == cursor) {
            return result;
        }
        if (cursor.moveToFirst()) {
            id = cursor.getString(cursor.getColumnIndex(ContactsContract.Contacts._ID));
        }
        cursor.close();
        // Build the Entity URI.
        Uri.Builder b = Uri.withAppendedPath(ContactsContract.Contacts.CONTENT_URI, id).buildUpon();
        b.appendPath(ContactsContract.Contacts.Entity.CONTENT_DIRECTORY);
        Uri contactUri = b.build();
        // Create the projection (SQL fields) and sort order.
        String[] projection = { ContactsContract.Contacts.Entity.RAW_CONTACT_ID,
                ContactsContract.Contacts.Entity.DATA1, ContactsContract.Contacts.Entity.MIMETYPE };
        String sortOrder = ContactsContract.Contacts.Entity.RAW_CONTACT_ID + " ASC";
        cursor = context.getContentResolver().query(contactUri, projection, null, null, sortOrder);
        if (null == cursor) {
            return result;
        }
        String mime;
        int mimeIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.MIMETYPE);
        int dataIdx = cursor.getColumnIndex(ContactsContract.Contacts.Entity.DATA1);
        if (cursor.moveToFirst()) {
            do {
                mime = cursor.getString(mimeIdx);
                if (mime.equalsIgnoreCase(CommonDataKinds.Phone.CONTENT_ITEM_TYPE)) {
                    result.phones.add(cursor.getString(dataIdx));
                } else if (mime.equalsIgnoreCase(CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)) {
                    result.name = cursor.getString(dataIdx);
                }
            } while (cursor.moveToNext());
        }
        cursor.close();
        return result;
    }

    /**
     * Generate new secrets, including new password and new salt.
     *
     * @param password
     * @param salt
     * @return
     */
    public static byte[] generateSecrets(final byte[] password, byte[] salt) {
        // generate salts
        SecureRandom random = new SecureRandom();
        random.nextBytes(salt);
        // generate secret
        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance(ENCRYPTION_TYPE);
            byte[] buffer = new byte[salt.length + password.length];
            System.arraycopy(password, 0, buffer, 0, password.length);
            System.arraycopy(salt, 0, buffer, password.length, salt.length);
            return md.digest(buffer);
        } catch (NoSuchAlgorithmException e) {
            throw new Error(e);
        }
    }


    /**
     * Check whether the password is correct.
     *
     * SHA256(password:salt) == secret
     *
     * @param password
     * @param salt
     * @param secret
     * @return
     */
    public static boolean checkPassword(final byte[] password, final byte[] salt, final byte[] secret) {
        MessageDigest md = null;
        try {
            md = MessageDigest.getInstance(ENCRYPTION_TYPE);
            byte[] buffer = new byte[salt.length + password.length];
            System.arraycopy(password, 0, buffer, 0, password.length);
            System.arraycopy(salt, 0, buffer, password.length, salt.length);
            byte[] digest = md.digest(buffer);
            if (secret.length != digest.length) {
                return false;
            }
            for (int i = 0; i < secret.length; ++i) {
                if (secret[i] != digest[i]) {
                    return false;
                }
            }
            return true;
        } catch (NoSuchAlgorithmException e) {
            throw new Error(e);
        }
    }

    public static boolean isAirplaneModeEnabled(Context context) {
        String state = Settings.Global.getString(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON);
        return "1".equals(state);
    }

    public static void turnOffAirplaneMode(Context context) {
        Settings.Global.putString(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, "0");
        Intent intent = new Intent(Intent.ACTION_AIRPLANE_MODE_CHANGED);
        intent.putExtra("state", false);
        context.sendBroadcastAsUser(intent, UserHandle.ALL);
    }
}
