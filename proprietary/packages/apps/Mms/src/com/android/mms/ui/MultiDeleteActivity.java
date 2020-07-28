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
package com.android.mms.ui;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.database.Cursor;
import android.database.MergeCursor;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.ServiceManager;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.util.Log;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.TelephonyIntents;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.data.Conversation;
import com.android.mms.data.WorkingMessage;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.transaction.SmsReceiver;
import com.android.mms.ui.ConversationList.BaseProgressQueryHandler;
import com.android.mms.util.MmsLog;
import com.android.mms.util.ThreadCountManager;
import com.android.mms.widget.MmsWidgetProvider;
import com.google.android.mms.MmsException;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.android.mms.pdu.MtkPduPersister;
import com.mediatek.android.mms.pdu.MtkSendReq;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.callback.IMultiDeleteActivityCallback;
import com.mediatek.mms.ext.IOpMessageListAdapterExt;
import com.mediatek.mms.ext.IOpMultiDeleteActivityExt;
import com.mediatek.mms.ipmessage.IIpMultiDeleteActivityExt;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.GeneralPreferenceActivity;
import com.mediatek.setting.MmsPreferenceActivity;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.regex.Pattern;

import mediatek.telephony.MtkSmsManager;
import mediatek.telephony.MtkTelephony;
import com.android.mms.MmsApp;

import static com.android.mms.ui.MessageListAdapter.PROJECTION;

/**
 * M: MultiDeleteActivity
 */
public class MultiDeleteActivity extends ListActivity implements IMultiDeleteActivityCallback {

    public static final String TAG = "Mms/MultiDeleteActivity";

    private static final int MESSAGE_LIST_QUERY_TOKEN = 9527;
    private static final int DELETE_MESSAGE_TOKEN = 9700;

    private static final String FOR_MULTIDELETE = "ForMultiDelete";
    //add for multi_forward
    public static final String FORWARD_MESSAGE = "forwarded_message";
    private ListView mMsgListView; // ListView for messages in this conversation
    public MessageListAdapter mMsgListAdapter; // and its corresponding ListAdapter

    private boolean mPossiblePendingNotification; // If the message list has changed, we may have
    // a pending notification to deal with.
    private long mThreadId; // Thread we are working in
    private Conversation mConversation; // Conversation we are working in
    private BackgroundQueryHandler mBackgroundQueryHandler;
    private ThreadCountManager mThreadCountManager = ThreadCountManager.getInstance();

    private MenuItem mSelectAll;
    private MenuItem mCancelSelect;
    private MenuItem mDelete;
//    private TextView mActionBarText;

    private boolean mIsSelectedAll;
    private int mDeleteRunningCount = 0; // The count of running Message-deleting

    private SelectActionMode mSelectActionMode;
    private ActionMode mSelectMode;
    private Button mChatSelect;

    private String mForwardMsgIds;

    public static final int FINISH_ACTIVITY = 1005;

    private static final int REQUEST_CODE_FORWARD = 1000;

    private int mMmsNotificationCount = 0;
    private boolean mMmsNotificationHasRun = false;

    private boolean mShowLastestMsg = false;
    /// M: fix bug ALPS00450886. @{
    private boolean mIsLockOrUnlockFinish = true;
    private ProgressDialog mProgressDialog;

    private static HashMap<String, Long> clickedViewAndTime = new HashMap<String, Long>();
    private int mOldCursorCount = 0;
    /// @}

    // when query message, return cursor max count is 1000, if bigger than 1000,
    // query multi times, and merge the multi cursors
    private static final int MAX_MESSAGE_PER_QUERY = 1000;
    private int queryOffset = 0;
    private ArrayList<Cursor> queryCursors = new ArrayList<Cursor>();

    // Set query state to QUERY_PENDING if already has a running query
    // then after query return, if query state is QUERY_PENDING, start a new query again
    private static int QUERY_IDLE = 0;
    private static int QUERY_RUNNING = 1;
    private static int QUERY_PENDING = 2;
    private int mQueryingState = QUERY_IDLE;

    private IIpMultiDeleteActivityExt mIpMultiDeleteActivity;
    private IOpMultiDeleteActivityExt mOpMultiDeleteActivityExt;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        /** M: add mediatek code @{ */
        mOpMultiDeleteActivityExt = OpMessageUtils.getOpMessagePlugin()
                .getOpMultiDeleteActivityExt();
        mOpMultiDeleteActivityExt.onCreate(this);
        mIpMultiDeleteActivity =
            IpMessageUtils.getIpMessagePlugin(this).getIpMultiDeleteActivity();
        mIpMultiDeleteActivity.multiDeleteActivityInit(this, this);
        setContentView(R.layout.multi_delete_list_screen);
        setProgressBarVisibility(false);
        MmsLog.d(TAG, "onCreate is called");

        mThreadId = getIntent().getLongExtra("thread_id", 0);
        if (mThreadId == 0) {
            MmsLog.w("TAG", "mThreadId can't be zero");
            finish();
        }
        mConversation = Conversation.get(this, mThreadId, false);
        mMsgListView = getListView();
        mMsgListView.setDivider(null);      // no divider so we look like IM conversation.
        mMsgListView.setDividerHeight(getResources().getDimensionPixelOffset(
                R.dimen.ipmsg_message_list_divier_height));
        initMessageList();
        initActivityState(savedInstanceState);

        setUpActionBar();
        mBackgroundQueryHandler = new BackgroundQueryHandler(getContentResolver());
        /// M: update font size, this is common feature.
        float textSize = MessageUtils.getPreferenceValueFloat(this,
                                        GeneralPreferenceActivity.TEXT_SIZE,
                                        GeneralPreferenceActivity.TEXT_SIZE_DEFAULT);
        setTextSize(textSize);
        /// M: add for update sim state dynamically. @{
        IntentFilter intentFilter =
            new IntentFilter(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED);
        registerReceiver(mSimReceiver, intentFilter);
        /// @}
        mShowLastestMsg = true;
    }

    public void setTextSize(float size) {
        if (mMsgListAdapter != null) {
            mMsgListAdapter.setTextSize(size);
        }

        if (mMsgListView != null && mMsgListView.getVisibility() == View.VISIBLE) {
            int count = mMsgListView.getChildCount();
            for (int i = 0; i < count; i++) {
                MessageListItem item =  (MessageListItem) mMsgListView.getChildAt(i);
                if (item != null) {
                    item.setBodyTextSize(size);
                }
            }
        }
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        mConversation.blockMarkAsRead(true);
        startMsgListQuery();
        if (mMsgListAdapter != null) {
            mMsgListAdapter.setOnDataSetChangedListener(mDataSetChangedListener);
        }
        mIsSelectedAll = false;
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onStop();
        if (mMsgListAdapter != null) {
            mMsgListAdapter.setOnDataSetChangedListener(null);
        }
    }
    @Override
    protected void onResume() {
        super.onResume();
        boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
        MmsLog.d(TAG, "onResume isSmsEnabled" + isSmsEnabled);
        if (!isSmsEnabled) {
            finish();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        MmsLog.d(TAG, "onConfigurationChanged " + newConfig);
        super.onConfigurationChanged(newConfig);
    }
    /// M: fix bug ALPS01510040, update sim state dynamically. @{
        private BroadcastReceiver mSimReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action != null &&
                        action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                    MmsLog.d(TAG, "Receive  ACTION_SUB_INFO_UPDATE ");
                    mMsgListAdapter.notifyDataSetChanged();
                }
            }
        };
    /// @}

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        MmsLog.d(TAG, "onSaveInstanceState is called ");
        if (mMsgListAdapter != null) {
            MmsLog.d(TAG, "Selected Number: " + mMsgListAdapter.getSelectedNumber()
                    + " List Count: " + mMsgListAdapter.getCount());

            if (mMsgListAdapter.getSelectedNumber() == 0) {
                return;
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
            outState.putInt("lock_count", mLockCount);
        }
    }

/*
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.compose_multi_select_menu, menu);
        mSelectAll = menu.findItem(R.id.select_all);
        mCancelSelect = menu.findItem(R.id.cancel_select);
        mDelete = menu.findItem(R.id.delete);
        mDelete.setEnabled(false);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        int selectNum = getSelectedCount();
        mActionBarText.setText(getResources().getQuantityString(
            R.plurals.message_view_selected_message_count, selectNum, selectNum));
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.select_all:
            if (!mIsSelectedAll) {
                mIsSelectedAll = true;
                markCheckedState(mIsSelectedAll);
                invalidateOptionsMenu();
            }
            mDelete.setEnabled(true);
            break;
        case R.id.cancel_select:
            if (mMsgListAdapter.getSelectedNumber() > 0) {
                mIsSelectedAll = false;
                markCheckedState(mIsSelectedAll);
                invalidateOptionsMenu();
            }
            mDelete.setEnabled(false);
            break;
        case R.id.delete:
            int mSelectedNumber = mMsgListAdapter.getSelectedNumber();
            if (mSelectedNumber >= mMsgListAdapter.getCount()) {
                Long threadId = mConversation.getThreadId();
                MultiDeleteMsgListener mMultiDeleteMsgListener = new MultiDeleteMsgListener();
                confirmMultiDeleteMsgDialog(mMultiDeleteMsgListener, selectedMsgHasLocked(),
                    true, threadId, MultiDeleteActivity.this);
            } else if (mMsgListAdapter.getSelectedNumber() > 0) {
                MultiDeleteMsgListener mMultiDeleteMsgListener = new MultiDeleteMsgListener();
                confirmMultiDeleteMsgDialog(mMultiDeleteMsgListener, selectedMsgHasLocked(),
                    false, null, MultiDeleteActivity.this);
            }
            break;
        default:
            break;
        }
        return true;
    }
*/

    @Override
    protected void onListItemClick(ListView parent, View view, int position, long id) {
        if (view != null) {
            ((MessageListItem) view).onMessageListItemClick();
        }
    }

    private void initActivityState(Bundle savedInstanceState) {
        if (savedInstanceState != null) {
            long[] selectedItems = savedInstanceState.getLongArray("select_list");
            if (selectedItems != null) {
                mMsgListAdapter.setItemsValue(true, selectedItems);
            }

            mLockCount = savedInstanceState.getInt("lock_count");
        }
    }

    private void setUpActionBar() {
/*
        ActionBar actionBar = getActionBar();

        ViewGroup v = (ViewGroup) LayoutInflater.from(this).inflate(
            R.layout.multi_delete_list_actionbar, null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM
            | ActionBar.DISPLAY_SHOW_HOME | ActionBar.DISPLAY_SHOW_TITLE);
        ImageButton mQuit = (ImageButton) v.findViewById(R.id.cancel_button);
        mQuit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // TODO Auto-generated method stub
                MultiDeleteActivity.this.finish();
            }
        });

        mActionBarText = (TextView) v.findViewById(R.id.select_items);
        actionBar.setCustomView(v);
*/
        mSelectActionMode = new SelectActionMode();
        mSelectMode = startActionMode(mSelectActionMode);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayShowHomeEnabled(false);
        actionBar.setDisplayHomeAsUpEnabled(false);
        actionBar.setDisplayShowTitleEnabled(false);
    }

    private void initMessageList() {

        MmsLog.d(TAG, "initMessageList is called");
        if (mMsgListAdapter != null) {
            MmsLog.d(TAG, "initMessageList is not null");
            return;
        }

        String highlightString = getIntent().getStringExtra("highlight");
        Pattern highlight = highlightString == null ? null : Pattern.compile("\\b"
            + Pattern.quote(highlightString), Pattern.CASE_INSENSITIVE);

        // Initialize the list adapter with a null cursor.
        mMsgListAdapter = new MessageListAdapter(this, null, mMsgListView, true, highlight);
        mMsgListAdapter.mIsDeleteMode = true;
        mMsgListAdapter.setMsgListItemHandler(mMessageListItemHandler);
        /// M: fix bug ALPS00439894, MTK MR1 new feature: Group Mms @{
        boolean isGroupMms = MmsPreferenceActivity.getIsGroupMmsEnabled(MultiDeleteActivity.this)
                                                && mConversation.getRecipients().size() > 1;
        mMsgListAdapter.setIsGroupConversation(isGroupMms);
        /// @}
        mMsgListView.setAdapter(mMsgListAdapter);
        mMsgListView.setItemsCanFocus(false);
        mMsgListView.setVisibility(View.VISIBLE);
        mIpMultiDeleteActivity.initMessageList(mMsgListAdapter.mIpMessageListAdapter);
    }

    private void startMsgListQuery() {
        // Cancel any pending queries
        mBackgroundQueryHandler.cancelOperation(MESSAGE_LIST_QUERY_TOKEN);
        MmsLog.dpi(TAG, "startMsgListQuery, " + " mQueryingState = " + mQueryingState);
        try {
            // check query state, don't start the query if already has a querying
            if (mQueryingState == QUERY_IDLE) {
                mQueryingState = QUERY_RUNNING;
                mBackgroundQueryHandler.postDelayed(new Runnable() {
                    public void run() {
                        Uri conversationUri = mConversation.getUri();
                        long threadId = mConversation.getThreadId();
                        Uri conversationUriLimit = Uri.parse(conversationUri +
                                        "/limit/" + MAX_MESSAGE_PER_QUERY +
                                        "/offset/" + queryOffset);
                        mBackgroundQueryHandler.startQuery(MESSAGE_LIST_QUERY_TOKEN, mThreadId,
                            conversationUriLimit, PROJECTION, null, null, null);
                    }
                }, 50);
            } else {
                mQueryingState = QUERY_PENDING;
            }
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    private void markCheckedState(boolean checkedState) {
        mMsgListAdapter.setItemsValue(checkedState, null);
        int count = mMsgListView.getChildCount();
        MessageListItem item = null;
        /// M: clear counter and record, re-create them.
        mLockCount = 0;
        // add for ipmessage
        mIpMultiDeleteActivity.onIpMarkCheckedState(mMsgListAdapter.getCursor(), checkedState);

        mMmsNotificationCount = 0;
        Iterator iter = mMsgListAdapter.getItemList().entrySet().iterator();
        Cursor cursor = mMsgListAdapter.getCursor();
        if (cursor == null) {
            MmsLog.w(TAG, "[markCheckedState] cursor is null");
            return;
        }
        int position = cursor.getPosition();
        int locked = 0;
        if (checkedState && cursor.moveToFirst()) {
            do {
                Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
                if (entry.getValue()) {
                    long mMmsId = entry.getKey();
                    MessageItem m = null;
                    if (mMmsId < 0) {
                        locked = cursor.getInt(mMsgListAdapter.COLUMN_LOCKED);
                        if (cursor.getInt(mMsgListAdapter.COLUMN_MMS_MESSAGE_TYPE) ==
                                MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
                            mMmsNotificationCount++;
                        }
                    } else {
                        locked = cursor.getInt(mMsgListAdapter.COLUMN_LOCKED);
                    }
                    if (locked == 1) {
                        mLockCount++;
                    }
                }
            } while (cursor.moveToNext());
        }
        cursor.moveToPosition(position);
        for (int i = 0; i < count; i++) {
            item = (MessageListItem) mMsgListView.getChildAt(i);
            if (item == null || item.getMessageItem() == null) {
                continue;
            }
            // add for ipmessage
            mIpMultiDeleteActivity.onAddSelectedIpMessageId(checkedState,
                    item.getMessageItem().mMsgId, item.getMessageItem().mIpMessageId);
            item.setSelectedBackGroud(checkedState);
        }

        updateSelectCount();
    }

    /**
     * @return the number of messages that are currently selected.
     */
    private int getSelectedCount() {
        return mMsgListAdapter.getSelectedNumber();
    }

    @Override
    public void onUserInteraction() {
        checkPendingNotification();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
    }

    private void checkPendingNotification() {
        if (mPossiblePendingNotification && hasWindowFocus()) {
            mConversation.markAsRead();
            mPossiblePendingNotification = false;
        }
    }

    /// M: fix bug ALPS00367594
    private HashSet<Long> mSelectedLockedMsgIds;

    /**
     * Judge weather selected messages include locked messages or not.
     *
     * @return
     */
    private boolean selectedMsgHasLocked() {
        boolean mHasLockedMsg = false;
        if (mMsgListAdapter == null) {
            return false;
        }
        mSelectedLockedMsgIds = new HashSet<Long>();
        Map<Long, Boolean> itemListMap = mMsgListAdapter.getItemList();
        Cursor cursor = mMsgListAdapter.getCursor();

        if (cursor != null) {
            int position = cursor.getPosition();
            int locked = 0;
            if (cursor.moveToFirst()) {
                do {
                    long columnId = 0;
                    if (cursor.getInt(mMsgListAdapter.COLUMN_LOCKED) == 1) {
                        String type = cursor.getString(mMsgListAdapter.COLUMN_MSG_TYPE);
                        long id = cursor.getInt(mMsgListAdapter.COLUMN_ID);
                        columnId = MessageListAdapter.getKey(type, id);
                        if (itemListMap.containsKey(columnId) && itemListMap.get(columnId)) {
                            mHasLockedMsg = true;
                            mSelectedLockedMsgIds.add(columnId);
                        }
                    }
                } while (cursor.moveToNext());
            }
            cursor.moveToPosition(position);
        }
        return mHasLockedMsg;
    }

    private boolean isMsgLocked(Map.Entry<Long, Boolean> entry) {
         if (entry == null) {
             return false;
         }
         long mMmsId = entry.getKey();
         MessageItem m = null;
         for (Long selectedMsgIds : mSelectedLockedMsgIds) {
             if (mMmsId == selectedMsgIds) {
                 return true;
             }
         }
         return false;
    }
    /// @}

    private void confirmMultiDeleteMsgDialog(final MultiDeleteMsgListener listener,
            boolean hasLockedMessages, boolean deleteThread, Long threadIds, Context context) {
        View contents = View.inflate(context, R.layout.delete_thread_dialog_view, null);
        TextView msg = (TextView) contents.findViewById(R.id.message);
        if (!deleteThread) {
            msg.setText(getString(R.string.confirm_delete_selected_messages));
        } else {
            listener.setDeleteThread(deleteThread);
            listener.setHasLockedMsg(hasLockedMessages);
            if (threadIds == null) {
                msg.setText(R.string.confirm_delete_all_conversations);
            } else {
                // Show the number of threads getting deleted in the confirmation dialog.
                msg.setText(context.getResources().getQuantityString(
                    R.plurals.confirm_delete_conversation, 1, 1));
            }
        }

        final CheckBox checkbox = (CheckBox) contents.findViewById(R.id.delete_locked);
        if (hasLockedMessages) {
            listener.setDeleteLockedMessage(checkbox.isChecked());
            checkbox.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    listener.setDeleteLockedMessage(checkbox.isChecked());
                }
            });
        } else {
            checkbox.setVisibility(View.GONE);
        }

        Cursor cursor = null;
        int smsId = 0;
        int mmsId = 0;
        cursor = context.getContentResolver().query(Sms.CONTENT_URI,
                new String[] {"max(_id)"}, null, null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                smsId = cursor.getInt(0);
                MmsLog.d(TAG, "confirmMultiDeleteMsgDialog max SMS id = " + smsId);
                }
            } finally {
                cursor.close();
                cursor = null;
            }
        }
        cursor = context.getContentResolver().query(Mms.CONTENT_URI,
                new String[] {"max(_id)"}, null, null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                mmsId = cursor.getInt(0);
                MmsLog.d(TAG, "confirmMultiDeleteMsgDialog max MMS id = " + mmsId);
                }
            } finally {
                cursor.close();
                cursor = null;
            }
        }
        listener.setMaxMsgId(mmsId, smsId);

        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(R.string.confirm_dialog_title).setIconAttribute(
            android.R.attr.alertDialogIcon).setCancelable(true).setPositiveButton(R.string.delete,
            listener).setNegativeButton(R.string.no, null).setView(contents).show();
    }

    private class MultiDeleteMsgListener implements OnClickListener {
        private boolean mDeleteLockedMessages = false;
        private boolean mDeleteThread = false;
        private boolean mHasLockedMsg = false;
        private int mMaxMmsId;
        private int mMaxSmsId;

        public MultiDeleteMsgListener() {
        }

        public void setMaxMsgId(int mmsId, int smsId) {
            mMaxMmsId = mmsId;
            mMaxSmsId = smsId;
        }

        public void setHasLockedMsg(boolean hasLockedMsg) {
            this.mHasLockedMsg = hasLockedMsg;
        }

        public void setDeleteThread(boolean deleteThread) {
            mDeleteThread = deleteThread;
        }

        public void setDeleteLockedMessage(boolean deleteLockedMessages) {
            mDeleteLockedMessages = deleteLockedMessages;
        }

        public void onClick(DialogInterface dialog, final int whichButton) {
            mBackgroundQueryHandler.setProgressDialog(DeleteProgressDialogUtil
                    .getProgressDialog(MultiDeleteActivity.this));
            mBackgroundQueryHandler.showProgressDialog();

            if (mDeleteThread) {
                if ((!mHasLockedMsg) || (mDeleteLockedMessages && mHasLockedMsg)) {
                      new Thread(new Runnable() {
                        public void run() {
                            HashSet<Long> threads = new HashSet<Long>();
                            threads.add(mThreadId);

                            // add for ipmessage
                            mIpMultiDeleteActivity.onIpDeleteThread(threads, mMaxSmsId);
                            int token = ConversationList.DELETE_CONVERSATION_TOKEN;
                            Conversation.startDelete(mBackgroundQueryHandler,
                                    token, mDeleteLockedMessages,
                            mThreadId, mMaxMmsId, mMaxSmsId);
                        }
                      }).start();
                      return;
                }
            }

            final boolean deleteLocked = mDeleteLockedMessages;
            new Thread(new Runnable() {
                public void run() {
                    Iterator iter = mMsgListAdapter.getItemList().entrySet().iterator();
                    Uri deleteSmsUri = null;
                    Uri deleteMmsUri = null;
                    String[] argsSms = new String[mMsgListAdapter.getSelectedNumber()];
                    String[] argsMms = new String[mMsgListAdapter.getSelectedNumber()];
                    int i = 0;
                    int j = 0;
                    while (iter.hasNext()) {
                        @SuppressWarnings("unchecked")
                        Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
                        if (!mDeleteLockedMessages) {
                            if (isMsgLocked(entry)) {
                                mIpMultiDeleteActivity.onIpDeleteLockedIpMsg(entry.getKey());
                                continue;
                            }
                        }
                        if (entry.getValue()) {
                            if (!mIpMultiDeleteActivity.onIpParseDeleteMsg(entry.getKey())) {
                                if (entry.getKey() > 0) {
                                    argsSms[i] = Long.toString(entry.getKey());
                                    MmsLog.i(TAG, "argsSms[i]" + argsSms[i]);
                                    deleteSmsUri = Sms.CONTENT_URI;
                                    i++;
                                } else {
                                    argsMms[j] = Long.toString(-entry.getKey());
                                    MmsLog.i(TAG, "argsMms[j]" + argsMms[j]);
                                    deleteMmsUri = Mms.CONTENT_URI;
                                    j++;
                                }
                            }
                        }
                    }
                    mBackgroundQueryHandler.setMax((deleteSmsUri != null ? 1 : 0)
                        + (deleteMmsUri != null ? 1 : 0));
                    // add for ipmessage
                    boolean ipDeleted = mIpMultiDeleteActivity.onIpMultiDeleteClick(
                            mBackgroundQueryHandler, DELETE_MESSAGE_TOKEN, null,
                            mDeleteRunningCount, deleteLocked);

                    mOpMultiDeleteActivityExt.onMultiDeleteClick(deleteSmsUri, argsSms);
                    if (deleteSmsUri != null) {
                        mDeleteRunningCount++;
                        mBackgroundQueryHandler.startDelete(DELETE_MESSAGE_TOKEN, null,
                            deleteSmsUri, FOR_MULTIDELETE, argsSms);
                    }
                    if (deleteMmsUri != null) {
                        mDeleteRunningCount++;
                        mBackgroundQueryHandler.startDelete(DELETE_MESSAGE_TOKEN, null,
                            deleteMmsUri, FOR_MULTIDELETE, argsMms);
                    }
                    if (deleteSmsUri == null && deleteMmsUri == null && !ipDeleted) {
                        mBackgroundQueryHandler.dismissProgressDialog();
                    }
                }
            }).start();
        }
    }

    private void updateSendFailedNotification() {
        final long threadId = mConversation.getThreadId();
        if (threadId <= 0) {
            return;
        }

        // updateSendFailedNotificationForThread makes a database call, so do the work off
        // of the ui thread.
        new Thread(new Runnable() {
            public void run() {
                MessagingNotification.updateSendFailedNotificationForThread(
                    MultiDeleteActivity.this, threadId);
            }
        }, "updateSendFailedNotification").start();
    }

    private final Handler mMessageListItemHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            String type;
            switch (msg.what) {
            case MessageListItem.ITEM_CLICK:
                // add for multi-delete
                mMsgListAdapter.changeSelectedState(msg.arg1);
                    if (msg.arg2 == MESSAGE_STATUS_LOCK) {
                    if (mMsgListAdapter.getItemList().get((long) msg.arg1)) {
                            mLockCount++;
                    } else {
                            mLockCount--;
                    }
                }
                MessageItem msgItem = (MessageItem) msg.obj;

                    // add for ipmessage
                    Boolean itemValue = mMsgListAdapter.getItemList().get((long) msg.arg1);
                    boolean selected = (itemValue == null) ? false : itemValue;
                    mIpMultiDeleteActivity.onIpHandleItemClick(msgItem.mIpMessageItem,
                            msgItem.mIpMessageId,
                            selected, (long) msg.arg1);

                if (MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND == msgItem.mMessageType) {
                    if (mMsgListAdapter.getItemList().get((long) msg.arg1)) {
                        mMmsNotificationCount++;
                    } else {
                        mMmsNotificationCount--;
                    }
                }

                mIsSelectedAll = false;
                if (mMsgListAdapter.getSelectedNumber() > 0) {
                    if (mMsgListAdapter.getSelectedNumber() == mMsgListAdapter.getCount()) {
                        mIsSelectedAll = true;
                    }
                }
                updateSelectCount();
                if (mSelectMode != null && !isFinishing()) {
                    mSelectMode.invalidate();
                }
                return;

            /// M: fix bug ALPS00554810, When the cache add new item,
            /// notify the data has been changed .@{
            case MessageListAdapter.MSG_LIST_NEED_REFRASH:
                boolean isClearCache =
                    msg.arg1 == MessageListAdapter.MESSAGE_LIST_REFRASH_WITH_CLEAR_CACHE;
                MmsLog.d(MessageListAdapter.CACHE_TAG,
                        "mMessageListItemHandler#handleMessage(): " +
                            "run adapter notify in mMessageListItemHandler. isClearCache = "
                        + isClearCache);
                mMsgListAdapter.setClearCacheFlag(isClearCache);
                mMsgListAdapter.notifyDataSetChanged();
                return;
            case FINISH_ACTIVITY:
                finish();
                return;
            /// @}
            default:
                Log.w(TAG, "Unknown message: " + msg.what);
                return;
            }
        }
    };

    private final class BackgroundQueryHandler extends BaseProgressQueryHandler {
        public BackgroundQueryHandler(ContentResolver contentResolver) {
            super(contentResolver);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            switch (token) {
            case MESSAGE_LIST_QUERY_TOKEN:
                if (mQueryingState == QUERY_RUNNING) {
                    mQueryingState = QUERY_IDLE;
                } else if (mQueryingState == QUERY_PENDING) {
                    if (cursor != null) {
                        cursor.close();
                    }
                    MmsLog.w(TAG, "onQueryComplete, has pending query, start a new query");
                    mQueryingState = QUERY_IDLE;
                    startMsgListQuery();
                    return;
                }
                /// M: fix bug ALPS00450886. @{
                if (!mIsLockOrUnlockFinish) {
                    /// M: fix bug ALPS00540064, avoid CursorLeak @{
                    if (cursor != null) {
                        cursor.close();
                    }
                    /// @}
                    return;
                }

                /// @}
                if (cursor == null) {
                    MmsLog.w(TAG, "onQueryComplete, cursor is null.");
                    return;
                }
                // check consistency between the query result and
                // 'mConversation'
                long tid = (Long) cookie;

                if (tid != mConversation.getThreadId()) {
                    MmsLog.d(TAG, "onQueryComplete: msg history query result is for threadId "
                        + tid + ", but mConversation has threadId "
                        + mConversation.getThreadId() + " starting a new query");
                    startMsgListQuery();
                    /// M: fix bug ALPS00540064, avoid CursorLeak @{
                    if (cursor != null) {
                        cursor.close();
                    }
                    /// @}
                    return;
                }

                if (!isFinishing()) {
                    int cursorCount = cursor.getCount();
                    MmsLog.d(TAG, "onQueryComplete, cursor.count:" + cursor.getCount());
                    if (queryOffset == 0) {
                        queryCursors.clear();
                    }
                    queryCursors.add(cursor);
                    queryOffset += cursor.getCount();
                    if (cursorCount >= MAX_MESSAGE_PER_QUERY) {
                        // query once more
                        startMsgListQuery();
                        return;
                    } else {
                        int length = queryCursors.size();
                        Cursor[] cursors = new Cursor[length];
                        for (int i = 0; i < length; i++){
                            cursors[i] = queryCursors.get(i);
                        }
                        Cursor merCur = new MergeCursor(cursors);
                        mMsgListAdapter.changeCursor(merCur);
                        queryCursors.clear();
                        queryOffset = 0;
                    }
                    if (mMsgListAdapter.mIsDeleteMode) {
                        Cursor adapterCursor = mMsgListAdapter.getCursor();
                        if (adapterCursor.getCount() != 0) {
                            mMsgListView.setSelection(mMsgListAdapter.getCount() - 1);
                            PopupMenu pop = mSelectActionMode.getPopupMenu();
                            if (pop != null && mOldCursorCount != adapterCursor.getCount()) {
                                MmsLog.w(TAG, "change pop in onQueryComplete");
                                mOldCursorCount = adapterCursor.getCount();
                                Menu popupMenu = pop.getMenu();
                                MenuItem selectAllItem = popupMenu
                                        .findItem(R.id.menu_select_all);
                                MenuItem unSelectAllItem = popupMenu
                                        .findItem(R.id.menu_select_cancel);
                                if (mMsgListAdapter.getSelectedNumber() >= adapterCursor.getCount()) {
                                    if (selectAllItem != null) {
                                        selectAllItem.setVisible(false);
                                    }
                                    if (unSelectAllItem != null) {
                                        unSelectAllItem.setVisible(true);
                                    }
                                } else {
                                    if (selectAllItem != null) {
                                        selectAllItem.setVisible(true);
                                    }
                                    if (unSelectAllItem != null) {
                                        unSelectAllItem.setVisible(false);
                                    }
                                }
                            }
                        } else {
                            if (adapterCursor != null) {
                                MmsLog.w(TAG, "onQueryComplete, cursor count is 0.");
                                adapterCursor.close();
                                setResult(RESULT_OK);
                                finish();
                                return;
                            }
                        }
                    }
                } else {
                    MmsLog.w(TAG, "activity is finishing");
                    if (cursor != null) {
                        cursor.close();
                        return;
                    }
                }

                mConversation.blockMarkAsRead(false);
                if (mShowLastestMsg) {
                    mShowLastestMsg = false;
                    mMsgListView.setSelection(cursor.getCount() - 1);
                }
                if (mSelectMode != null) {

                    MmsLog.w(TAG, "mSelectMode is invalidate after onquerycomplete");
                    mSelectMode.invalidate();
                }
                return;

            default:
                break;
            }
        }

        @Override
        protected void onDeleteComplete(int token, Object cookie, int result) {
            /// M: fix bug ALPS00351620; for requery searchactivity.
            SearchActivity.setNeedRequery();
            Intent mIntent = new Intent();
            switch (token) {
            case ConversationList.DELETE_CONVERSATION_TOKEN:
                try {
                 IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                        ServiceManager.getService("phoneEx"));
                    if (iTelEx.isTestIccCard(0)) {
                        MmsLog.d(TAG, "All threads has been deleted, send notification..");
                            MtkSmsManager
                                    .getSmsManagerForSubscriptionId(
                                        SmsReceiver.sLastIncomingSmsSubId)
                                                .setSmsMemoryStatus(true);
                    }
                } catch (Exception ex) {
                    MmsLog.e(TAG, " " + ex.getMessage());
                }
                // Update the notification for new messages since they
                // may be deleted.
                MessagingNotification.nonBlockingUpdateNewMessageIndicator(
                    MultiDeleteActivity.this, MessagingNotification.THREAD_NONE, false);
                // Update the notification for failed messages since they
                // may be deleted.
                updateSendFailedNotification();
                MessagingNotification
                        .updateDownloadFailedNotification(MultiDeleteActivity.this);
                if (progress()) {
                    dismissProgressDialog();
                }
                mIntent.putExtra("delete_all", true);
                break;

            case DELETE_MESSAGE_TOKEN:
                if (mDeleteRunningCount > 1) {
                    mDeleteRunningCount--;
                    return;
                }
                MmsLog.d(TAG, "onDeleteComplete(): before update mConversation, ThreadId = "
                    + mConversation.getThreadId());
                mConversation = Conversation.upDateThread(MultiDeleteActivity.this,
                    mConversation.getThreadId(), false);
                mThreadCountManager.isFull(mThreadId, MultiDeleteActivity.this,
                    ThreadCountManager.OP_FLAG_DECREASE);
                // Update the notification for new messages since they
                // may be deleted.
                MessagingNotification.nonBlockingUpdateNewMessageIndicator(
                    MultiDeleteActivity.this, MessagingNotification.THREAD_NONE, false);
                // Update the notification for failed messages since they
                // may be deleted.
                updateSendFailedNotification();
                MessagingNotification
                        .updateDownloadFailedNotification(MultiDeleteActivity.this);
                MmsLog.d(TAG, "onDeleteComplete(): MessageCount = "
                    + mConversation.getMessageCount() + ", ThreadId = "
                    + mConversation.getThreadId());
                if (progress()) {
                    dismissProgressDialog();
                }
                mIntent.putExtra("delete_all", false);
                mDeleteRunningCount = 0;
                break;

            default:
                break;
            }
            setResult(RESULT_OK, mIntent);
            finish();
        }
    }

    private final MessageListAdapter.OnDataSetChangedListener mDataSetChangedListener
            = new MessageListAdapter.OnDataSetChangedListener() {
        public void onDataSetChanged(MessageListAdapter adapter) {
            mPossiblePendingNotification = true;
        }

        public void onContentChanged(MessageListAdapter adapter) {
            startMsgListQuery();
            mIsSelectedAll = false;
        }
    };

    private void setSelectAll() {
        if (!mIsSelectedAll) {
            mIsSelectedAll = true;
            markCheckedState(mIsSelectedAll);
            if (mSelectMode != null) {
                mSelectMode.invalidate();
            }
        }
    }

    private void setDeselectAll() {
        if (mMsgListAdapter.getSelectedNumber() > 0) {
            mIsSelectedAll = false;
            markCheckedState(mIsSelectedAll);
            if (mSelectMode != null) {
                mSelectMode.invalidate();
            }
        }
    }

    private void updateSelectCount() {
        int selectNum = getSelectedCount();
        ///M: for ALPS00644772, because fading has some problem, so add "....." to work around
        mChatSelect.setText(getResources().getQuantityString(
            R.plurals.message_view_selected_message_count, selectNum, selectNum) + "     ");
//        mActionBarText.setText(getResources().getQuantityString(
//            R.plurals.message_view_selected_message_count, selectNum, selectNum));
    }

    private int mLockCount = 0;
    public static final int MESSAGE_STATUS_LOCK = 1;
    public static final int MESSAGE_STATUS_NOT_LOCK = 0;
    private class SelectActionMode implements ActionMode.Callback {
        private PopupMenu mPopup = null;
        @Override
        public boolean onCreateActionMode(final ActionMode mode, Menu menu) {
            ViewGroup v = (ViewGroup) LayoutInflater.from(MultiDeleteActivity.this).inflate(
                    R.layout.chat_select_action_bar, null);
            mode.setCustomView(v);
            mChatSelect = ((Button) v.findViewById(R.id.bt_chat_select));
            mChatSelect.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mPopup == null) {
                        mPopup = new PopupMenu(MultiDeleteActivity.this, v);
                        mPopup.getMenuInflater().inflate(R.menu.select_menu, mPopup.getMenu());
                    } else {
                        mPopup.dismiss();
                    }

                    mPopup.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener() {
                        public boolean onMenuItemClick(MenuItem item) {
                            int id = item.getItemId();
                            if (id == R.id.menu_select_all) {
                                setSelectAll();
                            } else if (id == R.id.menu_select_cancel) {
                                setDeselectAll();
                            } else {
                                return true;
                            }
                            return false;
                        }
                    });

                    Menu popupMenu = mPopup.getMenu();
                    MenuItem selectAllItem = popupMenu.findItem(R.id.menu_select_all);
                    MenuItem unSelectAllItem = popupMenu.findItem(R.id.menu_select_cancel);
                    if (mMsgListAdapter != null) {
                        Cursor cursor = mMsgListAdapter.getCursor();
                        if (cursor != null) {
                            if (mMsgListAdapter.getSelectedNumber() >= cursor.getCount()) {
                                if (selectAllItem != null) {
                                    selectAllItem.setVisible(false);
                                }
                                if (unSelectAllItem != null) {
                                    unSelectAllItem.setVisible(true);
                                }
                            } else {
                                if (selectAllItem != null) {
                                    selectAllItem.setVisible(true);
                                }
                                if (unSelectAllItem != null) {
                                    unSelectAllItem.setVisible(false);
                                }
                            }
                        } else {
                            if (selectAllItem != null) {
                                selectAllItem.setVisible(true);
                            }
                            if (unSelectAllItem != null) {
                                unSelectAllItem.setVisible(false);
                            }
                        }
                    } else if (selectAllItem != null) {
                        selectAllItem.setVisible(true);
                        if (unSelectAllItem != null) {
                            unSelectAllItem.setVisible(false);
                        }
                    }

                    mPopup.show();
                }
            });
            updateSelectCount();
            getMenuInflater().inflate(R.menu.compose_multi_select_menu, menu);

            // add for ipmessage
            mIpMultiDeleteActivity.onCreateIpActionMode(mode, menu);
            return true;
        }

        public PopupMenu getPopupMenu() {
            return mPopup;
        }
        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            MenuItem deleteItem = menu.findItem(R.id.delete);
            MenuItem lockItem = menu.findItem(R.id.lock);
            MenuItem forwardItem = menu.findItem(R.id.forward);
            MenuItem removeItem = menu.findItem(R.id.remove_lock);

            MmsLog.d(TAG, "onPrepareActionMode(): mLockCount = " + mLockCount);
            /// M: make disable if no item selected.
            int selectNum = getSelectedCount();

            MmsLog.d(TAG, "onPrepareActionMode(): selectNum = " + selectNum);
            if (mLockCount > 0) {
                menu.setGroupVisible(R.id.remove_lock_group, true);
            } else {
                menu.setGroupVisible(R.id.remove_lock_group, false);
            }
            if (mLockCount > 0 && mLockCount == selectNum) {
                menu.setGroupVisible(R.id.lock_group, false);
            } else {
                menu.setGroupVisible(R.id.lock_group, true);
            }

            if (selectNum > 0) {
                deleteItem.setEnabled(true);
                lockItem.setEnabled(true);
                forwardItem.setEnabled(true);
                removeItem.setEnabled(true);
            } else {
                deleteItem.setEnabled(false);
                lockItem.setEnabled(false);
                forwardItem.setEnabled(false);
                removeItem.setEnabled(false);
            }

            // add for ipmessage
            mIpMultiDeleteActivity.onPrepareIpActionMode(mode,
                    menu, selectNum, forwardItem.getItemId());
            mOpMultiDeleteActivityExt.onPrepareActionMode(forwardItem, selectNum);
            return true;
        }

        private void showMmsTipsDialog(final ActionMode mode, final MenuItem item) {
            AlertDialog.Builder builder = new AlertDialog.Builder(MultiDeleteActivity.this);
            builder.setTitle(R.string.forward_tips_title)
                   .setIconAttribute(android.R.attr.alertDialogIcon)
                   .setCancelable(true)
                   .setPositiveButton(R.string.dialog_continue,
                           new DialogInterface.OnClickListener() {
                       public final void onClick(DialogInterface dialog, int which) {
                           dialog.dismiss();
                           mMmsNotificationHasRun = true;
                           onActionItemClicked(mode, item);
                       }
                   })
                   .setNegativeButton(R.string.Cancel, null)
                   .setMessage(R.string.forward_tips_body)
                   .show();
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            if (mIpMultiDeleteActivity.onIpActionItemClicked(mode, item, getSelectedMsgIds(),
                mConversation.getRecipients().getNumbers(), mMsgListAdapter.getCursor())) {
                return true;
            }
            switch(item.getItemId()) {
            case R.id.delete:
                int mSelectedNumber = mMsgListAdapter.getSelectedNumber();
                /// M: fix bug ALPS00456634, delete msgs when the conversation has draft
                if (mSelectedNumber >= mMsgListAdapter.getCount() && !mConversation.hasDraft()) {
                    Long threadId = mConversation.getThreadId();
                    MultiDeleteMsgListener mMultiDeleteMsgListener = new MultiDeleteMsgListener();
                    confirmMultiDeleteMsgDialog(mMultiDeleteMsgListener,
                            selectedMsgHasLocked(), true, threadId,
                        MultiDeleteActivity.this);
                } else if (mMsgListAdapter.getSelectedNumber() > 0) {
                    MultiDeleteMsgListener mMultiDeleteMsgListener = new MultiDeleteMsgListener();
                    confirmMultiDeleteMsgDialog(mMultiDeleteMsgListener,
                            selectedMsgHasLocked(), false, null,
                        MultiDeleteActivity.this);
                }
                break;
            case R.id.forward:
                /// M: if forward has mms notification, we need show a tips dialog,
                ///this type can not forward.
                MmsLog.d(TAG, "forwardmessage is Click");
                if (isFastDoubleClick("forwardmessage", 500)) {
                    MmsLog.d(TAG, "forwardmessage is fast Click,ignore");
                    return true;
                }
                Iterator iter = mMsgListAdapter.getItemList().entrySet().iterator();
                int notificationNum = 0;
                while (iter.hasNext()) {
                    Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
                    if (entry.getValue()) {
                        if (entry.getKey() <= 0) {
                            MessageItem cacheitem = mMsgListAdapter.getCachedMessageItem("mms",
                                    -entry.getKey(), null);
                            if (cacheitem != null
                                    && cacheitem.mMessageType ==
                                        MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
                                notificationNum++;
                            }
                        }
                    }
                }
                mMmsNotificationCount = notificationNum;

                if (mMmsNotificationCount > 0 && !mMmsNotificationHasRun) {
                    showMmsTipsDialog(mode, item);
                    return true;
                }
                if (mOpMultiDeleteActivityExt.onActionItemClicked(MultiDeleteActivity.this)) {
                    return true;
                }
                break;
                case R.id.lock:

                    long[][] ids = getSelectedMsgIds();
                    if (ids != null) {
                        markAsLocked(ids, true);
                    }

                    break;
                case R.id.remove_lock:

                    long[][] msgIds = getSelectedMsgIds();
                    if (msgIds != null) {
                        markAsLocked(msgIds, false);
                    }
                    break;
            default:
                break;
            }
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            MultiDeleteActivity.this.finish();
//            setMarkState(false);
        }

        private long[][] getSelectedMsgIds() {

            long[][] ids = mIpMultiDeleteActivity.getSelectedMsgIds(mMsgListAdapter.getItemList(),
                                        mMsgListAdapter.getSelectedNumber());
            if (ids != null) {
                return ids;
            }
            Iterator importantIter = mMsgListAdapter.getItemList().entrySet().iterator();
            long[][] selectMessageIds = new long[2][mMsgListAdapter.getSelectedNumber()];

            int i = 0;
            int mmsIndex = 0;
            while (importantIter.hasNext()) {
                Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) importantIter.next();
                if (entry.getValue() && entry.getKey() > 0) {
                    selectMessageIds[0][i] = entry.getKey();
                    i++;
                } else if (entry.getValue() && entry.getKey() < 0) {
                    selectMessageIds[1][mmsIndex] = -entry.getKey();
                    mmsIndex++;
                }
            }
            return selectMessageIds;
        }
    }

    /**
     * M:
     *
     * @param smsIds
     * @param mmsIds
     * @param lock
     */
//    private void markAsLocked(final long smsIds[], final long mmsIds[], final boolean lock) {
    private void markAsLocked(final long[][] ids, final boolean lock) {
        final long smsIds[] = ids[0];
        final long mmsIds[] = ids[1];
        showProgressIndication();
        mIsLockOrUnlockFinish = false;
        final ContentValues values = new ContentValues(1);
        values.put("locked", lock ? 1 : 0);
        new Thread(new Runnable() {
            public void run() {
                mIpMultiDeleteActivity.markAsLocked(ids, lock);
                if (smsIds != null && smsIds.length > 0) {
                    if (!mOpMultiDeleteActivityExt.markAsLocked(getApplicationContext(), smsIds,
                            lock)) {
                        Uri uri = Sms.CONTENT_URI;
                        StringBuffer strBuf = new StringBuffer();
                        for (long id : smsIds) {
                            strBuf.append(id + ",");
                        }
                        String str = strBuf.toString();
                        String idSelect = str.substring(0, str.length() - 1);
                        getContentResolver().update(uri,
                                values, "_id in (" + idSelect + ")", null);
                    }
                }
                if (mmsIds != null && mmsIds.length > 0) {
                    Uri uri = Mms.CONTENT_URI;
                    StringBuffer strBuf = new StringBuffer();
                    for (long id : mmsIds) {
                        strBuf.append(id + ",");
                    }
                    String str = strBuf.toString();
                    String idSelect = str.substring(0, str.length() - 1);
                    getContentResolver().update(uri, values, "_id in (" + idSelect + ")", null);
                }
                if (mMessageListItemHandler != null) {
                    mMessageListItemHandler.post(new Runnable() {
                        public void run() {
                            dismissProgressIndication();
                            MultiDeleteActivity.this.finish();
                        }
                    });
                }
            }
        }).start();
        int smsIdsLen = smsIds != null ? smsIds.length : 0;
        int mmsIdsLen = mmsIds != null ? mmsIds.length : 0;
        if (lock) {
            mLockCount = (smsIdsLen + mmsIdsLen);
        } else {
            mLockCount = 0;
        }
    }

    /**
     * M:
     */
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case REQUEST_CODE_FORWARD:
                finish();
                break;
        }
    }

    public void prepareToForwardMessage() {
        Boolean mHasMms = false;
        Iterator iter = mMsgListAdapter.getItemList().entrySet().iterator();
        ArrayList<Long> selectSms = new ArrayList<Long>();
        ArrayList<Long> selectMms = new ArrayList<Long>();
        while (iter.hasNext()) {
            Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
            if (entry.getValue()) {
                if (entry.getKey() > 0) {
                    MmsLog.i(TAG, "sms");
                    selectSms.add(entry.getKey());
                } else {
                    MmsLog.i(TAG, "have  mms");
                    selectMms.add(entry.getKey());
                    mHasMms = true;
                }
            }
        }

        final ArrayList<Long> finalSelectSms = selectSms;
        if (mHasMms && !mMmsNotificationHasRun) {
            if (getSelectedCount() == 1) {
                /// M :add for one mms forward. @{
                long mMmsId = selectMms.get(0);
                MmsLog.i(TAG, "enter forward one mms and mMmsId is " + mMmsId);
                MessageItem cacheitem = mMsgListAdapter.getCachedMessageItem("mms", -mMmsId, null);
                /// M:for ALPS01013894 , if get getCachedMessageItem return null,
                /// mean this item don't in cache
                /// and if it is in itemlist, it mean this item is in listview@{
                if (cacheitem == null) {
                    iter = mMsgListAdapter.getItemList().entrySet().iterator();
                    while (iter.hasNext()) {
                        Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
                        MmsLog.i(TAG, "mMmsId " + mMmsId + " key " + entry.getKey());
                        if (entry.getKey() == mMmsId) {
                            Cursor cursor = mMsgListAdapter.getCursor();
                            if (cursor != null) {
                                MmsLog.i(TAG, "cursor is not null ");
                            }
                            if (cursor != null && cursor.moveToFirst()) {
                                do {
                                    MmsLog.i(TAG, "message_type "
                                            + cursor.getString(mMsgListAdapter.COLUMN_MSG_TYPE)
                                            + " mMmsId "
                                            + cursor.getLong(mMsgListAdapter.COLUMN_ID)
                                            + " subject " +
                                            cursor.getString(mMsgListAdapter.COLUMN_MMS_SUBJECT)
                                            + " mms type " + cursor.getInt(
                                            mMsgListAdapter.COLUMN_MMS_MESSAGE_TYPE));
                                    if (cursor.getInt(mMsgListAdapter.COLUMN_ID) == -mMmsId) {
                                        String highlightString = getIntent().getStringExtra(
                                                "highlight");
                                        Pattern highlight = highlightString == null ? null
                                                : Pattern.compile(
                                                        "\\b" + Pattern.quote(highlightString),
                                                        Pattern.CASE_INSENSITIVE);
                                        try {
                                            cacheitem = new MessageItem(
                                           this,
                                           0,
                                           cursor.getInt(mMsgListAdapter.COLUMN_MMS_MESSAGE_TYPE),
                                           cursor.getInt(mMsgListAdapter.COLUMN_SUBID),
                                           0,
                                           0,
                                           0,
                                           cursor.getLong(mMsgListAdapter.COLUMN_ID),
                                           cursor.getString(mMsgListAdapter.COLUMN_MSG_TYPE),
                                           cursor.getString(mMsgListAdapter.COLUMN_MMS_SUBJECT),
                                           null, null, highlight, false, 0, 0,
                                           cursor.getInt(mMsgListAdapter.COLUMN_MESSAGE_SIZE));
                                        } catch (MmsException e) {
                                            MmsLog.e(TAG, "MessageItem:", e);
                                        }
                                        break;
                                    }
                                } while (cursor.moveToNext());
                            }
                            break;
                        }
                    }
                }
                ///@}
                /// M: fix bug ALPS641407
                final MessageItem item = cacheitem;
                if (WorkingMessage.sCreationMode == 0
                        || !MessageUtils.isRestrictedType(MultiDeleteActivity.this, -mMmsId)) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            forwardOneMms(item);
                        }
                    });
                } else if (WorkingMessage.sCreationMode == WorkingMessage.WARNING_TYPE) {
                    new AlertDialog.Builder(MultiDeleteActivity.this)
                            .setTitle(R.string.restricted_forward_title)
                            .setIconAttribute(android.R.attr.alertDialogIcon)
                            .setMessage(R.string.restricted_forward_message)
                            .setPositiveButton(android.R.string.ok,
                                    new DialogInterface.OnClickListener() {
                                        public void onClick(DialogInterface dialog, int which) {
                                            int createMode = WorkingMessage.sCreationMode;
                                            WorkingMessage.sCreationMode = 0;
                                            runOnUiThread(new Runnable() {
                                                public void run() {
                                                    forwardOneMms(item);
                                                }
                                            });
                                            WorkingMessage.sCreationMode = createMode;
                                        }
                                    })
                            .setNegativeButton(android.R.string.cancel, null).show();
                }
                /// @}
            } else if (getSelectedCount() > 1) {
                MmsLog.i(TAG, "enter have  mms");
                new AlertDialog.Builder(MultiDeleteActivity.this)
                .setTitle(R.string.discard_mms_title)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(R.string.discard_mms_content)
                .setPositiveButton(R.string.dialog_continue,
                        new DialogInterface.OnClickListener() {
                    public final void onClick(DialogInterface dialog, int which) {
                        new Thread(new Runnable() {
                            public void run() {
                                forwardSmsMessage(finalSelectSms);
                            }
                        }, "ForwardMessage").start();
                    }
                })
                .setNegativeButton(android.R.string.cancel, null)
                .show();
            }
        } else {
            MmsLog.i(TAG, "enter have  sms");
            if (mMmsNotificationHasRun) {
                mMmsNotificationHasRun = false;
            }
            new Thread(new Runnable() {
                public void run() {
                    forwardSmsMessage(finalSelectSms);
                }
            }, "ForwardMessage").start();
        }
    }

    /// M: add for one mms forward. @{
    private ForwardMmsAsyncDialog mAsyncDialog;   // Used for background tasks.
    private class ForwardMmsAsyncDialog extends AsyncDialog {
        private Uri mTempMmsUri;            // Only used as a temporary to hold a slideshow uri
        private long mTempThreadId;         // Only used as a temporary to hold a threadId

        public ForwardMmsAsyncDialog(Activity activity) {
            super(activity);
        }

    }

    AsyncDialog getAsyncDialog() {
        if (mAsyncDialog == null) {
            mAsyncDialog = new ForwardMmsAsyncDialog(this);
        }
        return mAsyncDialog;
    }

    private void forwardOneMms(final MessageItem msgItem) {
        if (mIpMultiDeleteActivity.ipForwardOneMms(msgItem.mMessageUri)) {
            return;
        }
        getAsyncDialog().runAsync(new Runnable() {
            @Override
            public void run() {
                // This runnable gets run in a background thread.
                if (msgItem == null) {
                    return;
                }
                if (msgItem.mType.equals("mms")) {
                    MtkSendReq sendReq = new MtkSendReq();
                    String subject = getString(R.string.forward_prefix);
                    if (msgItem.mSubject != null) {
                        subject += msgItem.mSubject;
                    }
                    if (msgItem.mSlideshow != null) {
                        sendReq.setBody(msgItem.mSlideshow.makeCopy());
                    }
                    mAsyncDialog.mTempMmsUri = null;
                    try {
                        MtkPduPersister persister =
                                MtkPduPersister.getPduPersister(getApplicationContext()); //(MultiDeleteActivity.this);
                        mAsyncDialog.mTempMmsUri = persister.persist(sendReq,
                                Mms.Draft.CONTENT_URI, true,
                                MmsPreferenceActivity
                                    .getIsGroupMmsEnabled(MultiDeleteActivity.this), null);
                        mAsyncDialog.mTempThreadId = MessagingNotification.getThreadId(
                                MultiDeleteActivity.this, mAsyncDialog.mTempMmsUri);
                    } catch (MmsException e) {
                        Log.e(TAG, "Failed to copy message: " + msgItem.mMessageUri);
                        if (mMessageListItemHandler != null) {
                        mMessageListItemHandler.post(new Runnable() {
                        public void run() {
                            Toast.makeText(MultiDeleteActivity.this,
                                    R.string.cannot_save_message, Toast.LENGTH_SHORT).show();
                        }
                        });
                        }
                        return;
                    }
                }
            }
        }, new Runnable() {
            @Override
            public void run() {
                // Once the above background thread is complete, this runnable is run
                // on the UI thread.
                Intent intent = createIntent(MultiDeleteActivity.this);
                intent.putExtra(FORWARD_MESSAGE, true);
                if (mAsyncDialog.mTempThreadId > 0) {
                    intent.putExtra("thread_id", mAsyncDialog.mTempThreadId);
                }
                intent.putExtra("msg_uri", mAsyncDialog.mTempMmsUri);
                String subject = getString(R.string.forward_prefix);
                if (msgItem.mSubject != null) {
                    subject += msgItem.mSubject;
                }
                intent.putExtra("subject", subject);
                intent.setClassName(MultiDeleteActivity.this,
                        "com.android.mms.ui.ForwardMessageActivity");
                mOpMultiDeleteActivityExt.setExitCompose(intent, mMessageListItemHandler);
                startActivityForResult(intent, REQUEST_CODE_FORWARD);
            }
        }, R.string.sync_mms_to_db);
    }
    /// @}

    public void noSmsForward() {
        runOnUiThread(new Runnable() {
            public void run() {
                Toast.makeText(MultiDeleteActivity.this,
                        R.string.toast_sms_forward, Toast.LENGTH_SHORT).show();
                if (mMsgListAdapter.getSelectedNumber() > 0) {
                    mIsSelectedAll = false;
                    markCheckedState(mIsSelectedAll);
                    if (mSelectMode != null) {
                        mSelectMode.invalidate();
                    }
                }
            }
        });
    }

    public void beginForward(String body) {
        Intent intent = createIntent(this);
        intent.putExtra(FORWARD_MESSAGE, true);
        intent.putExtra(ComposeMessageActivity.SMS_BODY, body);

        // ForwardMessageActivity is simply an alias in the manifest for ComposeMessageActivity.
        // We have to make an alias because ComposeMessageActivity launch flags specify
        // singleTop. When we forward a message, we want to start a separate Compose Activity.
        // The only way to do that is to override the singleTop flag, which is impossible to do
        // in code. By creating an alias to the activity, without the singleTop flag, we can
        // launch a separate ComposeMessageActivity to edit the forward message.
        intent.setClassName(this, "com.android.mms.ui.ForwardMessageActivity");
        mOpMultiDeleteActivityExt.setExitCompose(intent, mMessageListItemHandler);
        startActivityForResult(intent, REQUEST_CODE_FORWARD);
    }

    private void forwardSmsMessage(ArrayList<Long> smsList) {
        int maxLength = MmsConfig.getMaxTextLimit();
            if (smsList.size() <= 0) {
                noSmsForward();
                return;
            }

            Collections.sort(smsList);
            StringBuffer strbuf = new StringBuffer();
            String tempbuf = null;
            boolean reachLimitFlag = false;
            Cursor cursor = mMsgListAdapter.getCursor();
            int findNum = 0;
            if (cursor != null && cursor.moveToFirst()) {
                do {
                    if (findNum == smsList.size()) {
                        break;
                    }
                    String type = cursor.getString(mMsgListAdapter.COLUMN_MSG_TYPE);
                    long msgId = cursor.getLong(mMsgListAdapter.COLUMN_ID);
                    if ("sms".equals(type) && smsList.contains(msgId)) {
                        findNum++;
                        String address = cursor.getString(mMsgListAdapter.COLUMN_SMS_ADDRESS);
                        String body    = cursor.getString(mMsgListAdapter.COLUMN_SMS_BODY);
                        int boxType = cursor.getInt(mMsgListAdapter.COLUMN_SMS_TYPE);
                        Contact contact = Contact.get(address, false);
                        String number = Contact.formatNameAndNumber(contact.getName(), contact.getNumber(), "");
                        strbuf.append(mOpMultiDeleteActivityExt.forwardMessage(
                              this, body, number, boxType));

                        if (strbuf.length() > maxLength) {
                            reachLimitFlag = true;
                            break;
                        }
                        if (findNum < smsList.size()) {
                            strbuf.append("\n");
                        }
                        MmsLog.d(TAG, "forwardMessage  strbuf.length()=" + strbuf.length());
                    }
                } while (cursor.moveToNext());
            }

            tempbuf = strbuf.toString();
            if (reachLimitFlag) {
                final String contentbuf = tempbuf;
                runOnUiThread(new Runnable() {
                    public void run() {
                        showReachLimitDialog(contentbuf);
                    }
                });
                return;
            }
            beginForward(tempbuf);
    }

    private Intent createIntent(Context context) {
        Intent intent = new Intent(context, MultiDeleteActivity.class);
        return intent;
    }

    public void showReachLimitDialog(final String mcontent) {
        new AlertDialog.Builder(MultiDeleteActivity.this)
        .setTitle(R.string.sms_size_limit)
        .setIconAttribute(android.R.attr.alertDialogIcon)
        .setMessage(R.string.dialog_sms_limit)
        .setPositiveButton(R.string.dialog_continue, new DialogInterface.OnClickListener() {
            public final void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
                Intent intent = createIntent(MultiDeleteActivity.this);
                intent.putExtra(FORWARD_MESSAGE, true);
                intent.putExtra(ComposeMessageActivity.SMS_BODY, mcontent);
                intent.setClassName(MultiDeleteActivity.this,
                        "com.android.mms.ui.ForwardMessageActivity");
                mOpMultiDeleteActivityExt.setExitCompose(intent, mMessageListItemHandler);
                startActivityForResult(intent, REQUEST_CODE_FORWARD);
            }
        })
        .setNegativeButton(android.R.string.cancel, null)
        .show();
    }

    @Override
    protected void onDestroy() {
        MmsLog.d(TAG, "onDestroy is called()");

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            super.onDestroy();
            return;
        }

        /// M: add for alps00829954 @{
        if (mMsgListAdapter != null) {
            mMsgListAdapter.destroyTaskStack();
            /// M: we need unregister cursor, so no more callback
            mMsgListAdapter.clearList();
            mMsgListAdapter.changeCursor(null);
        }
        MmsWidgetProvider.notifyDatasetChanged(this);
        /// @}
        unregisterReceiver(mSimReceiver);
        super.onDestroy();
    }

    /// M:For OP Callback:
    public void deleteMassTextInHost(String[] msgIds) {
        if (msgIds == null || msgIds.length < 1) {
            return;
        }
        String selection = "";
        for (String id : msgIds) {
            selection += (id + ",");
        }
        selection = selection.substring(0, selection.length() - 1);
        mBackgroundQueryHandler.startDelete(DELETE_MESSAGE_TOKEN, null, Sms.CONTENT_URI,
            "ipmsg_id in (select ipmsg_id from sms where ipmsg_id < 0 and _id in ("
                + selection + "))", null);
    }

    /// M: fix bug ALPS00450886. @{
    private void showProgressIndication() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(MultiDeleteActivity.this);
            mProgressDialog.setMessage(getString(R.string.please_wait));
            mProgressDialog.setIndeterminate(true);
            mProgressDialog.setCancelable(false);
        }
        mProgressDialog.show();
    }

    private void dismissProgressIndication() {
        if (mProgressDialog != null && !isFinishing() && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }
    }
    ///@}
    public boolean isFastDoubleClick(String viewName, long slotTime) {
        long time = System.currentTimeMillis();
        long lastClickTime = clickedViewAndTime.get(viewName) == null ? 0L : clickedViewAndTime
                .get(viewName);
        MmsLog.i(TAG, "isFastDoubleClick, clicked time = " + time + ", lastClickTime = "
                + lastClickTime);
        long slotT = time - lastClickTime;
        clickedViewAndTime.put(viewName, time);
        if (0 < slotT && slotT < slotTime) {
            return true;
        }
        return false;
    }

    public void onForwardActionItemClick() {
        mMmsNotificationHasRun = true;
        prepareToForwardMessage();
    }

    public void prepareToForwardMessageCallback() {
        prepareToForwardMessage();
    }

  /// M:For OP Callback:
    public void setDeleteRunningCount(int count) {
        mDeleteRunningCount = count;
    }
}
