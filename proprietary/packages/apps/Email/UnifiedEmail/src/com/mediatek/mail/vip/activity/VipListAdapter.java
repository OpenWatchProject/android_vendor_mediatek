/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.mail.vip.activity;

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.LinkedBlockingQueue;

import com.android.mail.R;
import com.android.emailcommon.Logging;
import com.mediatek.mail.vip.VipMember;

import android.app.Activity;
import android.content.Context;
import android.content.CursorLoader;
import android.content.Loader;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;
import android.widget.ImageView;

/**
 * M: an adapter for listView, add some extra contacts information in its MatrixCursor.
 *
 */
public class VipListAdapter extends CursorAdapter {
    public static final String TAG = "VipListAdapter";
    private static final long NULL_PHOTO_ID = -1;

    /** email address -> photo_id, diplay_name */
    /* package */ static final String[] PROJECTION_PHOTO_ID_PRESENCE = new String[] {
            Contacts.PHOTO_ID,
            Contacts.DISPLAY_NAME
            };
    private static final int COLUMN_PHOTO_ID = 0;
    private static final int COLUMN_DISPLAY_NAME = 1;

    private final LayoutInflater mInflater;
    private Activity mActivity;
    private final AvatarLoader mAvatarLoader;
    private AvatarLoaderCallback mAvatarCallback;
    private boolean mLoadAvatarPause = false;

    /**
     * Using for do something after avatar information has been loaded.
     */
    public interface AvatarLoaderCallback {
        // callback when contact's name has been changed.
        void onAvatarNameChanged(AvatarInfo avatarInfo);
    }

    public VipListAdapter(Context context, AvatarLoaderCallback callback) {
        super(context, null, 0 /* flags; no content observer */);
        mActivity = (Activity) context;
        mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mAvatarCallback = callback;
        mAvatarLoader = new AvatarLoader(mActivity);
        mAvatarLoader.startLoading();
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        return mInflater.inflate(R.layout.email_vip_item, parent, false);
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        VipListItem item = (VipListItem) view;
        item.resetViews();
        item.setTargetActivity(mActivity);
        String name = cursor.getString(VipMember.DISPLAY_NAME_COLUMN);
        String address = cursor.getString(VipMember.EMAIL_ADDRESS_COLUMN);
        item.setVipId(cursor.getLong(VipMember.ID_PROJECTION_COLUMN));
        item.setVipName(name);
        item.setVipEmailAddress(address);
        // update the view which depend on avatar's data.
        updateViewWithAvatarInfo(item, name, address);
    }

    /**
     * Using for update item view with contact's data, if it has not been prepared, trigger loading it.
     * @param item
     * @param localName
     * @param address
     */
    private void updateViewWithAvatarInfo(VipListItem item, String localName, String address) {
        AvatarInfo avatarInfo = mAvatarLoader.getAvatarInfo(address);
        if (avatarInfo != null) {
            if (avatarInfo.mLookUpUri != null) {
                item.setQuickContactLookupUri(Uri.parse(avatarInfo.mLookUpUri));
            }
            if (avatarInfo.mPhotoId != NULL_PHOTO_ID) {
                ImageView avatar = (ImageView) item.findViewById(R.id.contact_icon);
                item.loadContactAvatar(avatar, avatarInfo.mPhotoId);
            }

            // According the design, we must sync local vip name with system contacts, otherwise there will be a lot of
            // logic conflicts. So if local name changed, we need recover it base on contact's.
            // mLookUpUri not null indicated that this avatar was saved in contacts.
            if (avatarInfo.mLookUpUri != null && !localName.equals(avatarInfo.mDisplayName)) {
                Logging.d(TAG, " local vip name has been changed, we need re-sync it with system contacts : " + address);
                mAvatarCallback.onAvatarNameChanged(avatarInfo);
            }
        } else if (!mLoadAvatarPause) {
            // trigger avatar data loading.
            mAvatarLoader.reloadAvatar(address);
        }
    }

    static Loader<Cursor> createVipContentLoader(Context context, long accountId) {
        return new VipContentLoader(context, accountId);
    }

    /**
     * Loads vip members and add the corresponding avatar info.
     *
     * The returned {@link Cursor} is always a {@link ClosingMatrixCursor}.
     */
    private static class VipContentLoader extends CursorLoader {

        public VipContentLoader(Context context, long accountId) {
            super(context, VipMember.CONTENT_URI, VipMember.CONTENT_PROJECTION,
                    VipMember.SELECTION_ACCCOUNT_ID, new String[] { Long.toString(accountId)
                            }, VipMember.DISPLAY_NAME + " COLLATE LOCALIZED ASC");
        }

        @Override
        public Cursor loadInBackground() {
            return super.loadInBackground();
        }

    }

    /**
     * Get the position of the vip with the email address.
     * This method must called from UI thread.
     * @param emailAddress the email address of the vip
     * @return the position in the vip list
     */
    int getPosition(String emailAddress) {
        if (emailAddress == null) {
            return -1;
        }
        Cursor c = getCursor();
        c.moveToPosition(-1);
        while (c.moveToNext()) {
            if (emailAddress.equalsIgnoreCase(c.getString(VipMember.EMAIL_ADDRESS_COLUMN))) {
                return c.getPosition();
            }
        }
        return -1;
    }

    public void pauseAvatarLoading(boolean pause) {
        if (mLoadAvatarPause && pause != mLoadAvatarPause) {
            notifyDataSetChanged();
        }
        mLoadAvatarPause = pause;
    }

    public void onViewDestroy() {
        mAvatarLoader.stopLoading();
    }

    public AvatarInfo getAvatarInfo(String emailAddress) {
        return mAvatarLoader.getAvatarInfo(emailAddress);
    }

    public void updateAvatar(String emailAddress) {
        mAvatarLoader.reloadAvatar(emailAddress);
    }

    public class AvatarInfo {
        long mPhotoId;
        String mLookUpUri;
        String mDisplayName;
        String mAddress;
    }

    /**
     * Load the avatar of vips for good performance
     */
    private class AvatarLoader implements Runnable {
        // update the ui if has loaded 5 avatars
        private static final int UI_UPDATE_FREQUNCY = 5;
        ConcurrentHashMap<String, AvatarInfo> mAvatarMap = new ConcurrentHashMap<String, AvatarInfo>();
        Thread mLoadingThread;
        boolean mStop = false;
        Context mLoaderContext;
        private final BlockingQueue<String> mNeedLoadAvatars = new LinkedBlockingQueue<String>();

        AvatarLoader(Context context) {
            mLoaderContext = context;
        }

        public void reloadAvatar(String emailAddress) {
            // For better performance, we need remove the address which has pending in queue.
            mNeedLoadAvatars.remove(emailAddress);
            mNeedLoadAvatars.add(emailAddress);
        }

        public AvatarInfo getAvatarInfo(String emailAddress) {
            AvatarInfo avatar = null;
            avatar = mAvatarMap.get(emailAddress);
            return avatar;
        }

        public void startLoading() {
            if (mLoadingThread == null) {
                mStop = false;
                mLoadingThread = new Thread(this, toString());
                mLoadingThread.start();
            }
        }

        public void stopLoading() {
            mStop = true;
            mNeedLoadAvatars.add("");
            mLoadingThread = null;
        }

        @Override
        public void run() {
            // UI update throttle count to limit UI update frequency.
            int loadedCount = 0;
            // process the pending email address queue.
            while (!mStop) {
                String emailAddress = null;
                try {
                    int pendingAvatarts = mNeedLoadAvatars.size();
                    // update UI when all info has been loaded.
                    if (pendingAvatarts == 0) {
                        updateUI();
                        loadedCount = 0;
                    }
                    emailAddress = mNeedLoadAvatars.take();
                } catch (InterruptedException ex) {
                    Logging.d(TAG, "AvatarLoader loading thread be interrupted");
                }

                if (mStop) {
                    Logging.d(TAG, "AvatarLoader loading thread stop");
                    return;
                }
                if (!TextUtils.isEmpty(emailAddress)) {
                    AvatarInfo cachedAvatar = mAvatarMap.get(emailAddress);
                    AvatarInfo newAvatar = loadAvatarInfo(emailAddress);
                    // if avatar contact name changed, we should sync it with local vip data.
                    if (mAvatarCallback != null && cachedAvatar != null && !cachedAvatar.mDisplayName.equals(newAvatar.mDisplayName)) {
                        Logging.d(TAG, "emailAddress = " + emailAddress + " oldName = " + cachedAvatar.mDisplayName
                                + " newName = " + newAvatar.mDisplayName);
                        if (mAvatarCallback != null) {
                            mAvatarCallback.onAvatarNameChanged(newAvatar);
                        }
                    }
                    // replace the old info.
                    mAvatarMap.put(emailAddress, newAvatar);
                    // check whether UI need update or not.
                    loadedCount++;
                    if (loadedCount >= UI_UPDATE_FREQUNCY) {
                        updateUI();
                        loadedCount = 0;
                    }
                }
            }
        }

        private void updateUI() {
            ((Activity) mLoaderContext).runOnUiThread(new Runnable() {
                public void run() {
                    notifyDataSetChanged();
                }
            });
        }

        private AvatarInfo loadAvatarInfo(String emailAddress) {
            AvatarInfo avatar = null;
            // Don't deal with null
            if (emailAddress == null) {
                return null;
            }

            Logging.d(TAG, "loadAvatarInfo : emailAddress = " + emailAddress);
            // Load photo-id and lookupUri.
            long photoId = NULL_PHOTO_ID;
            String name = "";
            Uri lookupUri = null;
            Uri uri = Uri.withAppendedPath(Email.CONTENT_LOOKUP_URI, Uri.encode(emailAddress));
            Cursor c = mLoaderContext.getContentResolver().query(uri, PROJECTION_PHOTO_ID_PRESENCE, null, null, null);
            if (c == null) {
                return null;
            }
            try {
                if (c != null && c.moveToFirst()) {
                    photoId = c.getLong(COLUMN_PHOTO_ID);
                    name = c.getString(COLUMN_DISPLAY_NAME);
                    lookupUri = Data.getContactLookupUri(mLoaderContext.getContentResolver(), uri);
                }
                avatar = new AvatarInfo();
                avatar.mPhotoId = photoId;
                avatar.mDisplayName = name;
                avatar.mAddress = emailAddress;
                avatar.mLookUpUri = lookupUri == null ? null : lookupUri.toString();
            } finally {
                c.close();
            }
            return avatar;
        }
    }
}
