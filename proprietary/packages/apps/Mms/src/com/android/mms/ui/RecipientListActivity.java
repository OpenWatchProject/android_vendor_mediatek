/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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
 */

package com.android.mms.ui;

import android.app.ActionBar;
import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Telephony.Mms;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.QuickContactBadge;
import android.widget.TextView;

import com.android.mms.data.Contact;
import com.android.mms.data.ContactList;
import com.android.mms.data.Conversation;
import com.android.mms.R;

import com.mediatek.contacts.common.ContactPhotoManager;
import com.mediatek.contacts.common.ContactPhotoManager.DefaultImageRequest;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SettingListActivity;

import android.os.UserHandle;

/**
 * Display a list of recipients for a group conversation. This activity expects to receive a
 * threadId in the intent's extras.
 */
public class RecipientListActivity extends ListActivity implements Contact.UpdateListener {
    private final static String TAG = "RecipientListActivity";

    private long mThreadId;

    /// M: fix bug ALPS00433593, For posting UI update Runnables from other threads
    private Handler mHandler = new Handler();
    private ContactList mContacts;
    private Conversation mConv;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        if (icicle != null) {
            // Retrieve previously saved state of this activity.
            mThreadId = icicle.getLong(ComposeMessageActivity.THREAD_ID);
        } else {
            mThreadId = getIntent().getLongExtra(ComposeMessageActivity.THREAD_ID, 0);
        }
        if (mThreadId == 0) {
            Log.w(TAG, "No thread_id specified in extras or icicle. Finishing...");
            finish();
            return;
        }

        mConv = Conversation.get(this, mThreadId, true);
        if (mConv == null) {
            Log.w(TAG, "No conversation found for threadId: " + mThreadId + ". Finishing...");
            finish();
            return;
        }
        mContacts = mConv.getRecipients();
        getListView().setAdapter(new RecipientListAdapter(this, R.layout.recipient_list_item,
                mContacts));
        getListView().setDivider(null);
        ActionBar actionBar = getActionBar();
        if (actionBar.getCustomView() == null) {
            actionBar.setCustomView(R.layout.actionbar_recipient_list);
        }
        actionBar.setDisplayHomeAsUpEnabled(true);
        actionBar.setDisplayShowCustomEnabled(true);
        actionBar.setDisplayShowTitleEnabled(false);
        int cnt = mContacts.size();
        View customView = actionBar.getCustomView();
        TextView topTitle = (TextView) customView.findViewById(R.id.tv_top_title);
        TextView subTitle = (TextView) customView.findViewById(R.id.tv_top_subtitle);
        topTitle.setText(getResources().getText(R.string.recipient_list_activity));
        subTitle.setText(getResources().getQuantityString(R.plurals.recipient_count,
                cnt, cnt));

        /// M: fix bug ALPS00433593, refresh View when update Contact
        Contact.addListener(this);
        mFirstEnterActivity = true;
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putLong(ComposeMessageActivity.THREAD_ID, mThreadId);
        super.onSaveInstanceState(outState);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                finish();
                break;
            case R.id.action_settings:
                Intent settingIntent = new Intent(this, SettingListActivity.class);
                startActivityIfNeeded(settingIntent, -1);
                break;
        }
        return true;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.recipient_list_menu, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem itemSetting = menu.findItem(R.id.action_settings);
        if (UserHandle.myUserId() != UserHandle.USER_OWNER) {
            itemSetting.setVisible(false);
        } else {
            itemSetting.setVisible(true);
        }
        return true;
    }

    /// M: fix bug ALPS00433593, refresh View when update Contact @{
    public void onUpdate(Contact updated) {
        Log.v(TAG, "Contact onUpdate");
        mHandler.post(new Runnable() {
            public void run() {
                getListView().invalidateViews();
            }
        });
    }

    private boolean mFirstEnterActivity;

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        if (!mFirstEnterActivity) {
            Log.v(TAG, "onStart: Contact reload thread start");
            new Thread(new Runnable() {
                public void run() {
                    mContacts = mConv.getRecipients();
                    for (int i = 0; i < mContacts.size(); i++) {
                        mContacts.get(i).reload(false);
                    }
                }
            }).start();
        }
        mFirstEnterActivity = false;
    }

    @Override
    protected void onDestroy() {
        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            super.onDestroy();
            return;
        }

        // TODO Auto-generated method stub
        Contact.removeListener(this);
        super.onDestroy();
    }
    /// @}

    private static class RecipientListAdapter extends ArrayAdapter<Contact> {
        private final int mResourceId;
        private final LayoutInflater mInflater;
        private final Drawable mDefaultContactImage;
        private final ContactPhotoManager mContactPhotoManager;

        public RecipientListAdapter(Context context, int resource,
                ContactList recipients) {
            super(context, resource, recipients);

            mResourceId = resource;
            mInflater = LayoutInflater.from(context);
            mDefaultContactImage =
                    context.getResources().getDrawable(R.drawable.ic_default_contact);
            mContactPhotoManager = ContactPhotoManager.getInstance(context);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            final View listItemView =  mInflater.inflate(mResourceId, null);

            final TextView nameView = (TextView) listItemView.findViewById(R.id.name);
            final TextView numberView = (TextView) listItemView.findViewById(R.id.number);

            final Contact contact = getItem(position);
            final Uri photoUri = contact.getPhotoUri();
            final String name = contact.getName();
            final String number = contact.getNumber();
            if (!name.equals(number)) {
                nameView.setText(name);
                numberView.setText(number);
            } else {
                nameView.setText(number);
                numberView.setText(null);
            }

            QuickContactBadge badge = (QuickContactBadge) listItemView.findViewById(R.id.avatar);
            badge.setOverlay(null);
            if (Mms.isEmailAddress(number)) {
                badge.assignContactFromEmail(number, true);
            } else {
                if (contact.existsInDatabase()) {
                    badge.assignContactUri(contact.getUri());
                } else {
                    badge.assignContactFromPhone(contact.getNumber(), true);
                }
            }

            final Drawable avatarDrawable =
                contact.getAvatar(getContext(), mDefaultContactImage, -1);
            if (avatarDrawable != mDefaultContactImage) {
                final DefaultImageRequest request = new DefaultImageRequest(null, null,
                        ContactPhotoManager.TYPE_DEFAULT, true /* isCircular */);
                mContactPhotoManager.loadDirectoryPhoto(badge, photoUri,
                        false /* darkTheme */, true /* isCircular */, request);
            } else {
                badge.setImageDrawable(avatarDrawable);
            }
            return listItemView;
        }
    }
}
