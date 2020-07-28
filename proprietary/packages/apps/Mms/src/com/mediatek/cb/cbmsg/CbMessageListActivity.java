/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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

package com.mediatek.cb.cbmsg;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Telephony;
import android.provider.Telephony.Threads;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.View.OnKeyListener;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.Conversation;
import com.android.mms.ui.ConversationList;
import com.android.mms.ui.ConversationList.BaseProgressQueryHandler;
import com.android.mms.ui.MessageListItem;
import com.android.mms.util.MmsLog;
import com.android.mms.widget.MmsWidgetProvider;
import com.google.android.mms.util.SqliteWrapper;
import com.mediatek.mms.util.PermissionCheckUtil;

import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import mediatek.telephony.MtkTelephony;

/**
 * M: This activity provides a list view of existing conversations.
 */
public class CbMessageListActivity extends Activity implements
        View.OnClickListener {
    private static final String TAG = "MmsCB/CBMessageListActivity";
    private static final String THREADID = "thread_id";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = DEBUG;

    private static final int THREAD_LIST_QUERY_TOKEN = 1901;
    public static final int DELETE_MESSAGE_TOKEN = 1902;
    public static final int HAVE_LOCKED_MESSAGES_TOKEN = 1903;

    // IDs of the context menu items for the list of conversations.
    public static final int MENU_DELETE_MSG = 0;

    private MessageListQueryHandler mQueryHandler;
    private CbMessageListAdapter mMsgListAdapter;
    private SharedPreferences mPrefs;
    private Handler mHandler;
    // Conversation we are working in
    private Conversation mConversation;
    private ListView mMsgListView;
    private LinearLayout mBottomPanel;

    // add for multi-delete
    // View containing the delete and cancel buttons
    private View mDeletePanel;
    private ImageButton mDeleteButton;
    private ImageButton mCancelButton;
    private ImageButton mSelectAllButton;
    private ContentResolver mContentResolver;
    private boolean mIsSelectedAll;
    private Menu mOptionMenu;
    private static final String CHECKED_MESSAGE_LIMITS = "checked_message_limits";
    private static final String FOR_MULTIDELETE = "ForMultiDelete";
    public static final Uri CONTENT_URI = Uri.parse("content://cb/messages/#");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        setContentView(R.layout.cb_message_activity);
        Intent intent = getIntent();
        MmsLog.dpi(TAG, "CBMessageListActivity onCreate intent = " + intent);
        Uri intentData = intent.getData();
        boolean bFromLaunch = false;
        bFromLaunch = intent.getBooleanExtra("bFromLaunch", false);
        MmsLog.d(TAG, "bFromLaunch = " + bFromLaunch);

        if (bFromLaunch && !isThreadidExist(intentData)) {
            finish();
        } else {
            initResourceRefs();
            initListAdapter();
            initialize(savedInstanceState);
        }
    }

    protected void onResume() {
        super.onResume();
        if (!MmsConfig.isSmsEnabled(this)) {
            if (mMsgListAdapter.mIsDeleteMode) {
                mMsgListAdapter.mIsDeleteMode = false;
                mIsSelectedAll = false;
                markCheckedState(false);
                checkDeleteMode();
            }
            if (mOptionMenu != null && mOptionMenu.findItem(MENU_DELETE_MSG) != null) {
                mOptionMenu.findItem(MENU_DELETE_MSG).setVisible(false);
            }
        } else if (!mMsgListAdapter.mIsDeleteMode) {
            if (mOptionMenu != null && mOptionMenu.findItem(MENU_DELETE_MSG) != null) {
                mOptionMenu.findItem(MENU_DELETE_MSG).setVisible(true);
            }
        }
    }

    public boolean isThreadidExist(Uri intentData) {
        MmsLog.dpi(TAG, "intentData = " + intentData);
        ContentResolver contentResolver = getContentResolver();
        if (contentResolver != null) {
            Cursor cursor = contentResolver.query(intentData, new String[] {
                Threads._ID }, null, null, null);
            if (cursor != null) {
                try {
                    if (cursor.getCount() >= 1) {
                        return true;
                    }
                } finally {
                    cursor.close();
                }
            }
        }
        return false;
    }

    /**
     * Initialize all UI elements from resources.
     */
    private void initResourceRefs() {
        mQueryHandler = new MessageListQueryHandler(getContentResolver());

        // Initialize members for UI elements.
        mMsgListView = (ListView) findViewById(R.id.history);
        mMsgListView.setDivider(null); // no divider so we look like IM conversation.
        mMsgListView.setVisibility(View.VISIBLE); // property in xml is gone. So strange
        mMsgListView.setClipToPadding(false);
        mMsgListView.setClipChildren(false);
        mDeletePanel = findViewById(R.id.delete_panel);
        mSelectAllButton = (ImageButton) findViewById(R.id.select_all);
        mSelectAllButton.setOnClickListener(this);
        mCancelButton = (ImageButton) findViewById(R.id.cancel);
        mCancelButton.setOnClickListener(this);
        mDeleteButton = (ImageButton) findViewById(R.id.delete);
        mDeleteButton.setOnClickListener(this);
        mDeleteButton.setEnabled(false);
        mBottomPanel = (LinearLayout) findViewById(R.id.bottom_panel);
        mBottomPanel.setVisibility(View.GONE);
        mContentResolver = getContentResolver();

        mMsgListView.setOnCreateContextMenuListener(mConvListOnCreateContextMenuListener);
        mMsgListView.setOnKeyListener(mThreadListKeyListener);
        mMsgListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (view != null) {
                    ((CbMessageListItem) view).onMessageListItemClick();
                }
            }
        });
    }

    private void initialize(Bundle savedInstanceState) {
        Intent intent = getIntent();

        // Read parameters or previously saved state of this activity.
        initActivityState(savedInstanceState, intent);

        // Mark the current thread as read.
        mConversation.markAsRead();
    }

    private void initActivityState(Bundle bundle, Intent intent) {
        long threadId = 0;
        if (bundle != null) {
            threadId = bundle.getLong(THREADID, 0);
        } else {
            threadId = intent.getLongExtra(THREADID, 0);
            MmsLog.d(TAG, "initActivityState from intent threadId = " + threadId);
        }

        if (threadId > 0) {
            mConversation = Conversation.get(this, threadId, false);
        } else {
            Uri intentData = intent.getData();
            if (intentData != null) {
                // try to get a conversation based on the data URI passed to our intent.
                MmsLog.dpi(TAG, "Get a conversation based on the intentdata intentData = "
                        + intentData);
                mConversation = Conversation.get(this, intent.getData(), false,
                    MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD);
            } else {
                mConversation = Conversation.createNew(this);
            }
        }
        if (bundle != null) {
            mIsSelectedAll = bundle.getBoolean("is_all_selected");
            if (mIsSelectedAll) {
                mMsgListAdapter.setItemsValue(true, null);
                return;
            }
            long[] selectedItems = bundle.getLongArray("select_list");
            if (selectedItems != null) {
                mMsgListAdapter.setItemsValue(true, selectedItems);
            }
        }
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        if (mMsgListAdapter != null) {
            if (mMsgListAdapter.getSelectedNumber() == mMsgListAdapter.getCount()) {
                outState.putBoolean("is_all_selected", true);
            } else {
                long[] checkedArray = new long[mMsgListAdapter.getSelectedNumber()];
                Iterator iter = mMsgListAdapter.getItemList().entrySet().iterator();
                int i = 0;
                while (iter.hasNext()) {
                    @SuppressWarnings("unchecked")
                    Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
                    if (entry.getValue()) {
                        checkedArray[i] = entry.getKey();
                        i++;
                    }
                }
                outState.putLongArray("select_list", checkedArray);
            }
        }
        // save the current thread id
        outState.putLong(THREADID, mConversation.getThreadId());
        MmsLog.i(LogTag.CB, "saved thread id:" + mConversation.getThreadId());
    }

    private final CbMessageListAdapter.OnContentChangedListener mContentChangedListener
            = new CbMessageListAdapter.OnContentChangedListener() {
        public void onContentChanged(CbMessageListAdapter adapter) {
            if (mConversation != null) {
                mConversation.setHasUnreadMessages(true);
            }
            startAsyncQuery();
        }
    };

    private void initListAdapter() {
        mMsgListAdapter = new CbMessageListAdapter(this, null);
        mMsgListAdapter.setOnContentChangedListener(mContentChangedListener);
        mMsgListAdapter.setMsgListItemHandler(mMessageListItemHandler);
        mMsgListView.setAdapter(mMsgListAdapter);
        mMsgListView.setRecyclerListener(mMsgListAdapter);
    }

    // ==========================================================
    // Inner classes
    // ==========================================================
    private final Handler mMessageListItemHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            String type;
            switch (msg.what) {
            case MessageListItem.ITEM_CLICK:
                if (mMsgListAdapter.mIsDeleteMode) {
                    mMsgListAdapter.changeSelectedState(msg.arg1);
                    MmsLog.d(TAG, "On messageListItem click, changeSelectedState msg.arg1 = "
                            + msg.arg1);
                    if (mMsgListAdapter.getSelectedNumber() > 0) {
                        mDeleteButton.setEnabled(true);
                        if (mMsgListAdapter.getSelectedNumber() == mMsgListAdapter.getCount()) {
                            mIsSelectedAll = true;
                            return;
                        }
                    } else {
                        mDeleteButton.setEnabled(false);
                    }
                    mIsSelectedAll = false;
                }
                break;

            case CbMessageListItem.UPDATE_CHANNEL:
                // TODO: Update title here. The function should be implement when we can
                // show CB messages with same channel id but receive from different slot
                // in separate conversations. Otherwise, which name should be shown?
                /* falls through */
                ActionBar actionBar = getActionBar();
                MmsLog.d(TAG, "mConversation.getRecipients().size() = "
                        + mConversation.getRecipients().size());
                if (mConversation.getRecipients().size() == 0) {
                    actionBar.setTitle(MmsApp.getApplication().getApplicationContext()
                            .getString(R.string.cb_default_channel_name));
                } else {
                    actionBar.setTitle(formatCbMessageTitle());
                }

            default:
                return;
            }
        }
    };

    @Override
    protected void onNewIntent(Intent intent) {
        // Handle intents that occur after the activity has already been created.
        setIntent(intent);
        MmsLog.dpi(TAG, "onNewIntent intent = " + intent);
        initialize(null);
        privateOnStart();
    }

    @Override
    protected void onStart() {
        super.onStart();

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        CbMessage.cleanup(this);

        ActionBar actionBar = getActionBar();
        actionBar.setTitle(formatCbMessageTitle());
        actionBar.setDisplayHomeAsUpEnabled(true);

        // We used to refresh the DraftCache here, but
        // refreshing the DraftCache each time we go to the ConversationList seems overly
        // aggressive. We already update the DraftCache when leaving CMA in onStop() and
        // onNewIntent(), and when we delete threads or delete all in CMA or this activity.
        // I hope we don't have to do such a heavy operation each time we enter here.
        privateOnStart();

        // we invalidate the contact cache here because we want to get updated presence
        // and any contact changes. We don't invalidate the cache by observing presence and contact
        // changes (since that's too untargeted), so as a tradeoff we do it here.
        // If we're in the middle of the app initialization where we're loading the conversation
        // threads, don't invalidate the cache because we're in the process of building it.
        // TODO: think of a better way to invalidate cache more surgically or based on actual
        // TODO: changes we care about
        if (!CbMessage.loadingMessages()) {
            MmsLog.d(LogTag.CB, "onStart: loadingMessages = false");
        }
    }

    protected void privateOnStart() {
        startAsyncQuery();
    }

    @Override
    protected void onStop() {
        super.onStop();
        MmsLog.d(TAG, "CBMessageListActivity onStop");
        mConversation.markAsRead();
        //mMsgListAdapter.changeCursor(null);
    }

    private void startAsyncQuery() {
        try {
            long threadId = mConversation.getThreadId();
            MmsLog.d(TAG, "startAsyncQuery threadId = " + threadId);
            CbMessage.startQueryForThreadId(mQueryHandler, threadId, THREAD_LIST_QUERY_TOKEN);
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (mMsgListAdapter != null && mMsgListAdapter.mIsDeleteMode) {
            MmsLog.d(TAG, "mMsgListAdapter.mIsDeleteMode is true");
            return true;
        }
        menu.clear();
        if (mOptionMenu != null) {
            mOptionMenu.clear();
        }
        if (mMsgListAdapter != null) {
            menu.add(0, MENU_DELETE_MSG, 0, R.string.delete_message).setIcon(
                android.R.drawable.ic_menu_delete).setShowAsAction(
                MenuItem.SHOW_AS_ACTION_ALWAYS);
            if (!MmsConfig.isSmsEnabled(this) || mMsgListAdapter.mIsDeleteMode) {
                menu.findItem(MENU_DELETE_MSG).setVisible(false);
            } else {
                menu.findItem(MENU_DELETE_MSG).setVisible(true);
            }
        }
        mOptionMenu = menu;
        return true;
    }

    @Override
    public boolean onSearchRequested() {
        startSearch(null, false, null /* appData */, false);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_DELETE_MSG:
                // confirmDeleteThread(threadId, mQueryHandler, this);
                mMsgListAdapter.mIsDeleteMode = true;
                item.setVisible(false);
                markCheckedState(false);
                checkDeleteMode();
                break;
            case android.R.id.home:
                if (mMsgListAdapter.mIsDeleteMode) {
                    mMsgListAdapter.mIsDeleteMode = false;
                    if (mOptionMenu != null) {
                        mOptionMenu.findItem(MENU_DELETE_MSG).setVisible(true);
                    }
                    checkDeleteMode();
                }
                finish();
                break;
            default:
                return true;
        }

        return true;
    }

    private final OnCreateContextMenuListener mConvListOnCreateContextMenuListener
            = new OnCreateContextMenuListener() {
        public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
            if (!MmsConfig.isSmsEnabled(CbMessageListActivity.this)) {
                return;
            }
            Cursor cursor = mMsgListAdapter.getCursor();
            MmsLog.d(TAG, "onCreateContextMenu cursor.getPosition() = " + cursor.getPosition());
            if (cursor.getPosition() < 0) {
                return;
            }
            // String channelName = conv.getChannelName();
            menu.setHeaderTitle(R.string.message_options);

            AdapterView.AdapterContextMenuInfo info =
                (AdapterView.AdapterContextMenuInfo) menuInfo;
            if (info.position >= 0) {
                menu.add(0, MENU_DELETE_MSG, 0, R.string.delete_message);
            }
        }
    };

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        Cursor cursor = mMsgListAdapter.getCursor();
        if (cursor.getPosition() >= 0) {
            CbMessage message = CbMessage.from(CbMessageListActivity.this, cursor);
            long messageId = message.getMessageId();
            switch (item.getItemId()) {
            case MENU_DELETE_MSG:
                confirmDeleteMessage(messageId, mQueryHandler, this);
                break;

            default:
                break;
            }
        }
        return super.onContextItemSelected(item);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        // We override this method to avoid restarting the entire
        // activity when the keyboard is opened (declared in
        // AndroidManifest.xml). Because the only translatable text
        // in this activity is "New Message", which has the full width
        // of phone to work with, localization shouldn't be a problem:
        // no abbreviated alternate words should be needed even in
        // 'wide' languages like German or Russian.

        super.onConfigurationChanged(newConfig);
        MmsLog.d(LogTag.CB, "onConfigurationChanged: " + newConfig);
    }

    /**
     * Start the process of putting up a dialog to confirm deleting a thread, but first
     * start a background query to see if any of the threads or thread contain locked
     * messages so we'll know how detailed of a UI to display.
     *
     * @param threadId
     *            id of the thread to delete or -1 for all threads
     * @param handler
     *            query handler to do the background locked query
     */
    public static void confirmDeleteMessage(long messageId, AsyncQueryHandler handler,
            Context context) {
        confirmDeleteMessageDialog(new DeleteMessageListener(messageId, handler, context),
            messageId == -1, false, context);
    }

    /**
     * Build and show the proper delete thread dialog. The UI is slightly different depending
     * on whether there are locked messages in the thread(s) and whether we're deleting a
     * single thread or all threads.
     *
     * @param listener
     *            gets called when the delete button is pressed
     * @param deleteAll
     *            whether to show a single thread or all threads UI
     * @param hasLockedMessages
     *            whether the thread(s) contain locked messages
     * @param context
     *            used to load the various UI elements
     */
    public static void confirmDeleteMessageDialog(final DeleteMessageListener listener,
            boolean deleteAll, boolean hasLockedMessages, Context context) {
        View contents = View.inflate(context, R.layout.delete_thread_dialog_view, null);
        TextView msg = (TextView) contents.findViewById(R.id.message);
        msg.setText(R.string.confirm_delete_message);
        final CheckBox checkbox = (CheckBox) contents.findViewById(R.id.delete_locked);
        if (!hasLockedMessages) {
            checkbox.setVisibility(View.GONE);
        } else {
            listener.setDeleteLockedMessage(checkbox.isChecked());
            checkbox.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    listener.setDeleteLockedMessage(checkbox.isChecked());
                }
            });
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(R.string.confirm_dialog_title).setIconAttribute(
            android.R.attr.alertDialogIcon).setCancelable(true).setPositiveButton(R.string.delete,
            listener).setNegativeButton(R.string.no, null).setView(contents).show();
    }

    private final OnKeyListener mThreadListKeyListener = new OnKeyListener() {
        public boolean onKey(View v, int keyCode, KeyEvent event) {
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                switch (keyCode) {
                case KeyEvent.KEYCODE_DEL:
                    // TODO, need change (maybe)
                    long id = mMsgListView.getSelectedItemId();
                    if (id > 0) {
                        confirmDeleteMessage(id, mQueryHandler, CbMessageListActivity.this);
                    }
                    return true;

                default:
                    break;
                }
            }
            return false;
        }
    };

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
        case KeyEvent.KEYCODE_BACK:
            if (mMsgListAdapter.mIsDeleteMode) {
                mMsgListAdapter.mIsDeleteMode = false;
                mIsSelectedAll = false;
                if (mOptionMenu != null && mOptionMenu.findItem(MENU_DELETE_MSG) != null) {
                    mOptionMenu.findItem(MENU_DELETE_MSG).setVisible(true);
                }
                markCheckedState(false);
                checkDeleteMode();
                return true;
            }

        default:
            break;
        }

        return super.onKeyDown(keyCode, event);
    }

    public static class DeleteMessageListener implements OnClickListener {
        private final long mMessageId;
        private final AsyncQueryHandler mHandler;
        private final Context mContext;
        private static boolean mDeleteLockedMessages = true;
        private final Uri mDeleteUri;

        public DeleteMessageListener(long messageId, AsyncQueryHandler handler, Context context) {
            mMessageId = messageId;
            mHandler = handler;
            mContext = context;
            mDeleteUri = ContentUris.withAppendedId(CONTENT_URI, messageId);
        }

        public void setDeleteLockedMessage(boolean deleteLockedMessages) {
            mDeleteLockedMessages = deleteLockedMessages;
        }

        public void onClick(DialogInterface dialog, final int whichButton) {
            mHandler.startDelete(DELETE_MESSAGE_TOKEN, null, mDeleteUri, null, null);
        }
    }

    private final class MessageListQueryHandler extends BaseProgressQueryHandler {
        private NewProgressDialog mDialog;

        public MessageListQueryHandler(ContentResolver contentResolver) {
            super(contentResolver);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            switch (token) {
                case THREAD_LIST_QUERY_TOKEN:
                    mMsgListAdapter.initListMap(cursor);
                    mMsgListAdapter.changeCursor(cursor);
                    int i = cursor.getCount();
                    MmsLog.d(TAG, "onQueryComplete i = " + i);
                    mConversation.setMessageCount(i);
                    checkDeleteMode();
                    if (i == 0) {
                        cursor.close();
                        Toast.makeText(CbMessageListActivity.this,
                                  R.string.no_messages, Toast.LENGTH_LONG).show();
                        finish();
                    }
                    break;

                case HAVE_LOCKED_MESSAGES_TOKEN:
                    long threadId = (Long) cookie;
                    confirmDeleteMessageDialog(new DeleteMessageListener(threadId, mQueryHandler,
                            CbMessageListActivity.this), threadId == -1, cursor != null
                        && cursor.getCount() > 0, CbMessageListActivity.this);
                    break;

                default:
                    MmsLog.e(LogTag.CB, "onQueryComplete called with unknown token " + token);
            }
        }

        @Override
        protected void onDeleteComplete(int token, Object cookie, int result) {
            switch (token) {
            case DELETE_MESSAGE_TOKEN:
                // Make sure the conversation cache reflects the threads in the DB.
                CbMessage.init(CbMessageListActivity.this);
                CbMessagingNotification.updateNewMessageIndicator(CbMessageListActivity.this);
                startAsyncQuery();
                onContentChanged();
                if (mMsgListAdapter.mIsDeleteMode) {
                    mMsgListAdapter.mIsDeleteMode = false;
                    checkDeleteMode();
                    if (mOptionMenu != null) {
                        MenuItem item = mOptionMenu.findItem(MENU_DELETE_MSG);
                        if (item != null) {
                            item.setVisible(true);
                        }
                    }
                }
                if (progress()) {
                    dismissProgressDialog();
                }
                break;

            default:
                break;
            }
            MmsWidgetProvider.notifyDatasetChanged(getApplicationContext());
            // If we're deleting the whole conversation, throw away
            // our current working message and bail.
            if (token == ConversationList.DELETE_CONVERSATION_TOKEN) {
                if (progress()) {
                    dismissProgressDialog();
                }
                Conversation.init(CbMessageListActivity.this);
                finish();
            }
        }

        /** M:
         * Sets the progress dialog.
         * @param dialog the progress dialog.
         */
        public void setProgressDialog(NewProgressDialog dialog) {
            // Patch back ALPS00457128 which the "deleting" progress display for a long time
            if (mDialog == null) {
                mDialog = dialog;
            }
        }
    }

    // add for multi-delete
    private void markCheckedState(boolean checkedState) {
        mMsgListAdapter.setItemsValue(checkedState, null);
        mDeleteButton.setEnabled(checkedState);
        int count = mMsgListView.getChildCount();
        CbMessageListItem item = null;
        for (int i = 0; i < count; i++) {
            item = (CbMessageListItem) mMsgListView.getChildAt(i);
            if (null != item) {
                item.setSelectedBackGroud(checkedState);
            }
        }
    }

    private void checkDeleteMode() {
        if (mMsgListAdapter.mIsDeleteMode) {
            mDeletePanel.setVisibility(View.VISIBLE);
        } else {
            mDeletePanel.setVisibility(View.GONE);
        }
        if (!mMsgListAdapter.mIsDeleteMode) {
            mMsgListAdapter.clearList();
        }
    }

    private void log(String format, Object... args) {
        String s = String.format(format, args);
        MmsLog.d(LogTag.CB, "[" + Thread.currentThread().getId() + "] " + s);
    }

    public static Intent createIntent(Context context, long threadId) {
        Intent intent = new Intent(context, CbMessageListActivity.class);
        if (threadId > 0) {
            intent.setData(Conversation.getUri(threadId));
        }
        return intent;
    }

    // add for multi-delete
    public void onClick(View v) {
        if (v == mDeleteButton) {
            if (mMsgListAdapter.getSelectedNumber() >= mMsgListAdapter.getCount()) {
                confirmThreadDelete();
            } else {
                confirmMultiDelete();
            }
        } else if (v == mCancelButton) {
            if (mMsgListAdapter.getSelectedNumber() > 0) {
                mIsSelectedAll = false;
                markCheckedState(mIsSelectedAll);
            }
        } else if (v == mSelectAllButton) {
            mIsSelectedAll = true;
            markCheckedState(mIsSelectedAll);
        }
    }

    private void confirmThreadDelete() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.delete_thread).setIconAttribute(android.R.attr.alertDialogIcon)
                .setCancelable(true)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    public final void onClick(DialogInterface dialog, int which) {
                        new Thread(new Runnable() {
                            public void run() {
                                Cursor cursor = null;
                                int SmsCbId = 0;
                                cursor = getApplicationContext().getContentResolver().query(
                                        MtkTelephony.SmsCb.CONTENT_URI, new String[] {
                                            "max(_id)"
                                        }, null, null, null);
                                if (cursor != null) {
                                    try {
                                        if (cursor.moveToFirst()) {
                                            SmsCbId = cursor.getInt(0);
                                            MmsLog.d(TAG,
                                                    "confirmMultiDeleteMsgDialog max SMS id = "
                                                            + SmsCbId);
                                        }
                                    } finally {
                                        cursor.close();
                                        cursor = null;
                                    }
                                }
                                int token = ConversationList.DELETE_CONVERSATION_TOKEN;
                                Conversation.startDelete(mQueryHandler, token,
                                        DeleteMessageListener.mDeleteLockedMessages,
                                        mConversation.getThreadId(), SmsCbId, SmsCbId);
                            }
                        }, "DeleteThread").start();
                    }
                }).setNegativeButton(android.R.string.cancel, null).show();
    }

    private void confirmMultiDelete() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.confirm_dialog_title);
        builder.setIconAttribute(android.R.attr.alertDialogIcon);
        builder.setCancelable(true);
        builder.setMessage(R.string.confirm_delete_selected_messages);
        builder.setPositiveButton(R.string.delete, new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int which) {
                mQueryHandler.setProgressDialog(getProgressDialog(CbMessageListActivity.this));
                mQueryHandler.showProgressDialog();
                new Thread(new Runnable() {
                    public void run() {

                        Iterator iter = mMsgListAdapter.getItemList().entrySet().iterator();
                        Uri deleteCbUri = null;
                        String[] argsCb = new String[mMsgListAdapter.getSelectedNumber()];
                        int i = 0;
                        while (iter.hasNext()) {
                            @SuppressWarnings("unchecked")
                            Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
                            if (entry.getValue()) {
                                if (entry.getKey() > 0) {
                                    argsCb[i] = Long.toString(entry.getKey());
                                    Log.i(TAG, "argsCb[i]" + argsCb[i]);
                                    deleteCbUri = CONTENT_URI;
                                    i++;
                                }
                            }
                        }
                        mQueryHandler.startDelete(DELETE_MESSAGE_TOKEN, null, deleteCbUri,
                            FOR_MULTIDELETE, argsCb);
                    }
                }).start();
            }
        });
        builder.setNegativeButton(R.string.no, null);
        builder.show();
    }

    /**
     * Gets a delete progress dialog.
     * @param context the activity context.
     * @return the delete progress dialog.
     */
    public static NewProgressDialog getProgressDialog(Context context) {
        NewProgressDialog dialog = new NewProgressDialog(context);
        dialog.setCancelable(false);
        dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER);
        dialog.setMessage(context.getString(R.string.deleting));
        dialog.setMax(1); /* default is one complete */
        // ignore the search key, when deleting we do not want the search bar come out.
        dialog.setOnKeyListener(new DialogInterface.OnKeyListener() {
            public boolean onKey(DialogInterface dialog, int keyCode, KeyEvent event) {
                return (keyCode == KeyEvent.KEYCODE_SEARCH);
            }
        });
        return dialog;
    }

    private String formatCbMessageTitle() {
        int channelId = 0;
        String from = null;
        if (mConversation.getRecipients().size() == 0) {
            return MmsApp.getApplication().getApplicationContext()
                    .getString(R.string.cb_default_channel_name);
        }
        String number = mConversation.getRecipients().get(0).getNumber();
        if (!TextUtils.isEmpty(number)) {
            try {
                channelId = Integer.parseInt(number);
            } catch (NumberFormatException e) {
                MmsLog.e(TAG, "format number error!");
            }
        }

        String name = "";
        List<SubscriptionInfo> subInfoList
                = SubscriptionManager.from(MmsApp.getApplication()
                        .getApplicationContext()).getActiveSubscriptionInfoList();
        int subCount = (subInfoList != null && !subInfoList.isEmpty()) ? subInfoList.size() : 0;
        for (int i = 0; i < subCount; i++) {
            name = CbMessage.getCBChannelName(subInfoList.get(i).getSubscriptionId(), channelId);
            if (!name.equals(MmsApp.getApplication().getApplicationContext()
                    .getString(R.string.cb_default_channel_name))) {
                break;
            }
        }

        if (TextUtils.isEmpty(name)) {
            name = MmsApp.getApplication().getApplicationContext()
                    .getString(R.string.cb_default_channel_name);
        }
        from = name + "(" + channelId + ")";
        MmsLog.dpi(TAG, "CB message title: " + from);
        return from;
    }
}

class NewProgressDialog extends ProgressDialog {
    private boolean mIsDismiss = false;
    public NewProgressDialog(Context context) {
        super(context);
    }

    public void dismiss() {
       if (isDismiss()) {
           super.dismiss();
       }
    }

    public synchronized void setDismiss(boolean isDismiss) {
        this.mIsDismiss = isDismiss;
    }

    public synchronized boolean isDismiss() {
        return mIsDismiss;
    }
}
