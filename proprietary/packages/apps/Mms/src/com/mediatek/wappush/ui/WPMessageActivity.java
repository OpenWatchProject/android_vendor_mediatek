/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 * Copyright (C) 2007-2008 Esmertec AG.
 * Copyright (C) 2007-2008 The Android Open Source Project
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

package com.mediatek.wappush.ui;

import static com.mediatek.wappush.ui.WPMessageListAdapter.COLUMN_ID;
import static com.mediatek.wappush.ui.WPMessageListAdapter.COLUMN_WPMS_TYPE;
import static com.mediatek.wappush.ui.WPMessageListAdapter.WP_PROJECTION;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.AsyncQueryHandler;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.UserManager;
import android.provider.Telephony.Threads;
import android.text.ClipboardManager;
import android.util.Config;
import android.util.Log;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.PopupMenu.OnMenuItemClickListener;

import com.android.browser.provider.Browser;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.MmsApp;
import com.android.mms.data.Contact;
import com.android.mms.ui.ConversationList;
import com.android.mms.ui.ConversationList.BaseProgressQueryHandler;
import com.android.mms.ui.CustomMenu;
import com.android.mms.ui.MessageUtils;
import com.android.mms.ui.CustomMenu.DropDownMenu;
import com.android.mms.util.MmsLog;

import com.mediatek.mms.callback.ITextSizeAdjustHost;
import com.mediatek.mms.ext.IOpWPMessageActivityExt;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SettingListActivity;
import com.mediatek.wappush.SiExpiredCheck;
import com.mediatek.wappush.WapPushMessagingNotification;

import java.util.HashSet;
import java.util.regex.Pattern;

import mediatek.telephony.MtkTelephony.WapPush;

/**
 * M: This is the main UI for: 1. Viewing wappush message history of a
 * conversation.
 */
public class WPMessageActivity extends Activity implements Contact.UpdateListener,
        ITextSizeAdjustHost {
    public static final int REQUEST_CODE_ADD_CONTACT = 18;

    private static final String TAG = "Mms/WapPush";

    private static final boolean DEBUG = false;

    private static final boolean TRACE = false;

    private static final boolean LOCAL_LOGV = DEBUG ? Config.LOGD : Config.LOGV;

    // Menu ID
//    private static final int MENU_DELETE_THREAD = 1;
//
//    private static final int MENU_CONVERSATION_LIST = 6;

    private static final int MENU_DELETE_ALL_WAPPUSH = 5;

    private static final int MENU_MULTI_DELETE_MESSAGES = 9;

    private ModeCallback mActionModeListener = new ModeCallback();

    private ActionMode mActionMode;

    private static String ACTIONMODE = "actionMode";

    private static int mDeleteCounter = 0;
    private View mEmptyViewDefault;

    private static final int MESSAGE_LIST_QUERY_TOKEN = 9527;

    private static final int DELETE_MESSAGE_TOKEN = 9700;

    // / M: add for display unread thread count
    private static final int MAX_DISPLAY_UNREAD_COUNT = 99;

    private static final String DISPLAY_UNREAD_COUNT_CONTENT_FOR_ABOVE_99 = "99+";

    private static final int UNREAD_MESSAGES_QUERY_TOKEN = 9528;

    private ContentResolver mContentResolver;

    private BackgroundQueryHandler mBackgroundQueryHandler;

    private WPMessageListView mMsgListView; // ListView for messages in this
                                            // conversation

    public WPMessageListAdapter mMsgListAdapter; // and its corresponding
                                                 // ListAdapter

//    private boolean mIsLandscape; // Whether we're in landscape mode

    private static final String STR_RN = "\\r\\n"; // for "\r\n"

    private static final String STR_CN = "\n"; // the char value of '\n'

    protected static boolean sDestroy = false;

    private Long mThreadId = -1L;

    private boolean mPossiblePendingNotification; // If the message list has
                                                  // changed, we may have
    /// M: fix bug ALPS00447970, wap push notification @{
    private static final String UNSEEN_SELECTION = "seen=0";
    private static final String[] SEEN_PROJECTION = new String[] {
        "seen"
        };
    private boolean mNeedToMarkAsSeen;
    /// @}

    // SiExpired Check
    private SiExpiredCheck mSiExpiredCheck;

    public static final int SI_ACTION_NONE = 0;

    public static final int SI_ACTION_LOW = 1;

    public static final int SI_ACTION_MEDIUM = 2;

    public static final int SI_ACTION_HIGH = 3;

    public static final int SI_ACTION_DELETE = 4;

    public static final int SL_ACTION_LOW = 1;

    public static final int SL_ACTION_HIGH = 2;

    public static final int SL_ACTION_CACHE = 3;

    /// M: for ALPS01213243, dismiss the delete dialog if message has been deleted by other way.
    private static AlertDialog sDeleteDialog;

    // /M: add for Plugins
    private IOpWPMessageActivityExt mOpWPMessageActivityExt = null;

    // For KK to dismiss delete function when MMS is not default.
    private boolean mIsSmsEnabled;

    // add for mutli user
    private boolean isUserHasPerUsingMms = true;

    @SuppressWarnings("unused")
    private static void log(String logMsg) {
        Thread current = Thread.currentThread();
        long tid = current.getId();
        StackTraceElement[] stack = current.getStackTrace();
        String methodName = stack[3].getMethodName();
        // Prepend current thread ID and name of calling method to the message.
        logMsg = "[" + tid + "] [" + methodName + "] " + logMsg;
        MmsLog.dpi(TAG, "WPMessageActivity: " + logMsg);
    }

    // ==========================================================
    // Inner classes
    // ==========================================================
    private final Handler mMessageListItemHandler = new Handler();

    /**
     * Return the messageItem associated with the type ("si" or "sl") and
     * message id.
     *
     * @param type Type of the message: "si" or "sl"
     * @param msgId Message id of the message. This is the _id of the wappush
     *            msg or pdu row and is stored in the WPMessageItem
     * @param createFromCursorIfNotInCache true if the item is not found in the
     *            WPMessageListAdapter's cache and the code can create a new
     *            WPMessageItem based on the position of the current cursor. If
     *            false, the function returns null if the WPMessageItem isn't in
     *            the cache.
     * @return WPMessageItem or null if not found and
     *         createFromCursorIfNotInCache is false
     */
    private WPMessageItem getMessageItem(
            int type, long msgId, boolean createFromCursorIfNotInCache) {
        return mMsgListAdapter.getCachedMessageItem(type, msgId,
                createFromCursorIfNotInCache ? mMsgListAdapter.getCursor() : null);
    }

    private class DeleteMessageListener implements OnClickListener {
        //
        private final HashSet<Long> mMsgIds;
        private final AsyncQueryHandler mHandler;
        private final Context mContext;
        private ActionMode mMode;
        //
        public DeleteMessageListener(HashSet<Long> msgIds, AsyncQueryHandler handler,
                Context context, ActionMode mode) {
            mMsgIds = msgIds;
            mHandler = handler;
            mContext = context;
            mMode = mode;
        }
        private void showProgressDialog() {
            if (mHandler instanceof BackgroundQueryHandler) {
                ((BackgroundQueryHandler) mHandler).setProgressDialog(
                        DeleteProgressDialogUtil.getProgressDialog(mContext));
                ((BackgroundQueryHandler) mHandler).showProgressDialog();
            }
        }
        public void onClick(DialogInterface dialog, int whichButton) {
            if (mMode != null) {
                mMode.finish();
                mMode = null;
            }
            if (mMsgIds == null || mMsgIds.size() > 0) {
                showProgressDialog();
                if (mMsgIds == null) {
                    Uri deleteUri = WapPush.CONTENT_URI;
                    mBackgroundQueryHandler.startDelete(DELETE_MESSAGE_TOKEN, null, deleteUri,
                            null, null);
                } else {
                    mDeleteCounter = 0;
                    for (long msgId : mMsgIds) {
                        mDeleteCounter++;
                        Uri deleteUri = ContentUris.withAppendedId(WapPush.CONTENT_URI, msgId);
                        mBackgroundQueryHandler.startDelete(DELETE_MESSAGE_TOKEN, null, deleteUri,
                                null, null);
                    }
                }
            }
            dialog.dismiss();
        }
    }

    private void copyToClipboard(String str) {
        ClipboardManager clip = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
        clip.setText(str);
    }

    private static String getWPMessageDetails(Context context, WPMessageItem msgItem) {
        StringBuilder details = new StringBuilder();
        Resources res = context.getResources();

        //Priority: Low, Medium, High
        //SI: None, Low, Medium, High, Delete
        //Priority: SL: High, Low, Cache
        details.append(res.getString(R.string.wp_msg_priority_label));
        int priority = msgItem.mAction;
        int type = msgItem.mType;
        if (WapPush.TYPE_SI == type) {
            switch (priority) {
            case SI_ACTION_NONE:
                MmsLog.i(TAG, "WPMessageActivity: " + "action error, none");
                break;
            case SI_ACTION_LOW:
                details.append(res.getString(R.string.wp_msg_priority_low));
                break;
            case SI_ACTION_MEDIUM:
                details.append(res.getString(R.string.wp_msg_priority_medium));
                break;
            case SI_ACTION_HIGH:
                details.append(res.getString(R.string.wp_msg_priority_high));
                break;
            case SI_ACTION_DELETE:
                MmsLog.i(TAG, "WPMessageActivity: " + "action error, delete");
                break;
            default:
                MmsLog.i(TAG, "WPMessageActivity: " + "getWPMessageDetails si priority error.");
            }
        } else if (WapPush.TYPE_SL == type) {
            switch (priority) {
            case SL_ACTION_LOW:
                details.append(res.getString(R.string.wp_msg_priority_low));
                break;
            case SL_ACTION_HIGH:
                details.append(res.getString(R.string.wp_msg_priority_high));
                break;
            case SL_ACTION_CACHE:
                details.append(res.getString(R.string.wp_msg_priority_low));
                break;
            default:
                MmsLog.i(TAG, "WPMessageActivity: " + "getWPMessageDetails sl priority error.");
            }
        } else {
            MmsLog.i(TAG, "WPMessageActivity: " + "getWPMessageDetails type error.");
        }

        // Address: ***
        details.append('\n');
        details.append(res.getString(R.string.from_label));
        details.append(msgItem.mAddress);

        // Date: ***
        details.append('\n');
        details.append(res.getString(R.string.received_label));
        long date = msgItem.mDate;
        details.append(MessageUtils.formatTimeStampString(context, date, true));

        //Expired time
        long expiredDate = msgItem.mExpirationLong;
        if (expiredDate != 0) {
            details.append('\n');
            details.append(String.format(context.getString(R.string.wp_msg_expiration_label),
                    MessageUtils.formatTimeStampString(context, expiredDate, true)));
        }

        // WebSite: ***
        String url = msgItem.mURL;
        if ((url != null) && (!url.equals(""))) {
            details.append("\n\n");
            details.append(res.getString(R.string.website));
            details.append(url);
        }

        // Message Content
        String text = msgItem.mText;
        if ((text != null) && (!text.equals(""))) {
            details.append("\n\n");
            details.append(msgItem.mText);
        }

        return details.toString();
    }

    // ==========================================================
    // Activity methods
    // ==========================================================
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            MmsLog.i(TAG, "WPMessageActivity: " + "requestRequiredPermissions");
            return;
        }

        mOpWPMessageActivityExt = OpMessageUtils.getOpMessagePlugin().getOpWPMessageActivityExt();
        MmsLog.i(TAG, "WPMessageActivity: " + "Enter onCreate function.");

        setContentView(R.layout.wp_message_activity);
        // add for multi user
        isUserHasPerUsingMms = !UserManager.get(getApplicationContext()).hasUserRestriction(
                UserManager.DISALLOW_SMS);
        if (isUserHasPerUsingMms) {
            openApplication();
        }

        // Initialize members for UI elements.
        mMsgListView = (WPMessageListView) findViewById(R.id.history);
        mMsgListView.setDivider(null); // no divider so we look like IM
                                       // conversation.
        mEmptyViewDefault = findViewById(R.id.empty);
        mContentResolver = getContentResolver();
        mBackgroundQueryHandler = new BackgroundQueryHandler(mContentResolver);

        initialize(0);
        sDestroy = false;

        mSiExpiredCheck = new SiExpiredCheck(this);
        mSiExpiredCheck.startSiExpiredCheckThread();
        setupActionBar();
        setTitle(R.string.menu_wappush);
    }

    // if then intent is send from notification, we will open the latest push
    // message's url.
    public void openApplication() {
        Intent initIntent = getIntent();

        String url = initIntent.getStringExtra("URL");
        if (url != null) {
            Uri uri = Uri.parse(MessageUtils.checkAndModifyUrl(url));
            Intent intent = new Intent(Intent.ACTION_VIEW, uri);
            intent.putExtra(Browser.EXTRA_APPLICATION_ID, WPMessageActivity.this.getPackageName());
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
            try {
                startActivity(intent);
            } catch (ActivityNotFoundException ex) {
                Toast.makeText(WPMessageActivity.this, R.string.error_unsupported_scheme,
                        Toast.LENGTH_LONG).show();
                MmsLog.e(TAG, "Scheme " + uri.getScheme() + "is not supported!");
            }
        }
    }

    public void initialize(long originalThreadId) {
        Intent intent = getIntent();

        initActivityState(intent);

        log(" intent = " + intent + "originalThreadId = " + originalThreadId);

        // Set up the message history ListAdapter
        initMessageList();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        MmsLog.d(TAG, "onNewIntent");
        setIntent(intent);

        boolean fromHistory = (Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY == (intent.getFlags()
                & Intent.FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY));
        // check if we need to open the latest message's url
        if (!fromHistory) {
            openApplication();
        }
        initialize(0);
        //startMsgListQuery();
        //startUnreadQuery();
    }

    @Override
    protected void onRestart() {
        super.onRestart();
        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            MmsLog.d(TAG, "onRestart() requestAllPermissions return !!");
            return;
        }
        MmsLog.d(TAG, "onRestart function.");
    }

    @Override
    protected void onStart() {
        super.onStart();
        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        /// M: for ALPS01861847, add for mutli user @{
        if (!isUserHasPerUsingMms) {
            MmsLog.d(TAG, "onStart user has no permission");
            Toast.makeText(WPMessageActivity.this,
                R.string.user_has_no_premission, Toast.LENGTH_SHORT).show();
            markAllMessageAsSeen();
            return;
        }
        mNeedToMarkAsSeen = true;
        float textSize = MessageUtils.getPreferenceValueFloat(this,
                    SettingListActivity.TEXT_SIZE, 18);
        setTextSize(textSize);
        if (mMsgListAdapter != null) {
            mMsgListAdapter.setOnDataSetChangedListener(mDataSetChangedListener);
        }
        mOpWPMessageActivityExt.onStart(this, this);

        startMsgListQuery();
        startUnreadQuery();
    }

    private TextView mUnreadConvCount;

    private void setupActionBar() {
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        ViewGroup v = (ViewGroup) LayoutInflater.from(this).inflate(
                R.layout.conversation_list_actionbar_wp, null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setCustomView(v, new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL | Gravity.RIGHT));

        mUnreadConvCount = (TextView) v.findViewById(R.id.unread_conv_count);
    }

    @Override
    protected void onResume() {
        super.onResume();
        //addRecipientsListeners();
        // There seems to be a bug in the framework such that setting the title
        // here gets overwritten to the original title. Do this delayed as a
        // workaround.
        boolean isSmsEnable = MmsConfig.isSmsEnabled(this);
        MmsLog.d(TAG, "onResume  isSmsEnable: " + isSmsEnable);
        if (isSmsEnable != mIsSmsEnabled) {
            mIsSmsEnabled = isSmsEnable;
        }
        if (!isSmsEnable) {
            if (mActionMode != null) {
                mActionMode.finish();
            }
            if (sDeleteDialog != null && sDeleteDialog.isShowing()) {
                sDeleteDialog.dismiss();
                sDeleteDialog = null;
            }
        }
        mSiExpiredCheck.startExpiredCheck();
    }

    @Override
    protected void onPause() {
        super.onPause();
        //removeRecipientsListeners();
    }

    @Override
    protected void onStop() {
        super.onStop();
        // Allow any blocked calls to update the thread's read status.

        if (mMsgListAdapter != null) {
            mMsgListAdapter.setOnDataSetChangedListener(null);
            mMsgListAdapter.clearAllContactListeners();
        }
        if (mBackgroundQueryHandler != null) {
            mBackgroundQueryHandler.cancelOperation(UNREAD_MESSAGES_QUERY_TOKEN);
            mBackgroundQueryHandler.cancelOperation(MESSAGE_LIST_QUERY_TOKEN);
        }
        MmsLog.i(TAG, "WPMessageActivity: " + "onStop stopExpiredCheck.");
        mSiExpiredCheck.stopExpiredCheck();
    }

    @Override
    protected void onDestroy() {
        if (TRACE) {
            android.os.Debug.stopMethodTracing();
        }
        sDestroy = true;
        // stop the si expired check thread
        if (mSiExpiredCheck != null) {
            mSiExpiredCheck.stopSiExpiredCheckThread();
        }
        sDeleteDialog = null;
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        switch (keyCode) {
            case KeyEvent.KEYCODE_DEL:
                if ((mMsgListAdapter != null) && mMsgListView.isFocused()) {
                    Cursor cursor;
                    try {
                        cursor = (Cursor) mMsgListView.getSelectedItem();
                    } catch (ClassCastException e) {
                        MmsLog.e(TAG, "WPMessageActivity: " + "Unexpected ClassCastException.",
                                e);
                        return super.onKeyDown(keyCode, event);
                    }
                    HashSet<Long> msgIds = new HashSet<Long>();
                    if (cursor != null) {
                        final long msgId = cursor.getLong(COLUMN_ID);
                        msgIds.add(msgId);
                        DeleteMessageListener l = new DeleteMessageListener(msgIds,
                                mBackgroundQueryHandler, WPMessageActivity.this, null);
                        confirmDeleteMessageDialog(l, msgIds, WPMessageActivity.this);
                        return true;
                    }
                }
                break;
            default:
                break;
        }

        return super.onKeyDown(keyCode, event);
    }

    private void goToConversationList() {
        finish();
        startActivity(new Intent(this, ConversationList.class));
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        menu.clear();
        MmsLog.d(TAG, "onPrepareOptionsMenu");
        if (!isUserHasPerUsingMms) {
            MmsLog.d(TAG, "onPrepareOptionsMenu user has no permission");
            return false;
        }
        if (mMsgListAdapter.getCount() > 0) {
            Cursor cursor = mMsgListAdapter.getCursor();
            if (mIsSmsEnabled && (null != cursor) && (cursor.getCount() > 0)) {
                menu.add(0, MENU_DELETE_ALL_WAPPUSH, 0, R.string.menu_delete_allwappush).setIcon(
                        R.drawable.ic_menu_trash_holo_dark);
            }
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_DELETE_ALL_WAPPUSH:
                DeleteMessageListener l = new DeleteMessageListener(null,
                        mBackgroundQueryHandler, WPMessageActivity.this, null);
                confirmDeleteMessageDialog(l, null, WPMessageActivity.this);
                break;
            case android.R.id.home:
                finish();
                break;
            default:
                break;
        }

        return true;
    }

    public static void confirmDeleteMessageDialog(final DeleteMessageListener listener,
            HashSet<Long> msgIds,
            Context context) {
        View contents = View.inflate(context, R.layout.delete_thread_dialog_view, null);
        TextView msg = (TextView) contents.findViewById(R.id.message);

        if (msgIds == null) {
            msg.setText(R.string.confirm_delete_allwappush);
        } else {
            // Show the number of threads getting deleted in the confirmation dialog.
//            int cnt = msgIds.size();
            msg.setText(R.string.confirm_delete_selected_wappush);
//            msg.setText(context.getResources().getQuantityString(
//                R.plurals.confirm_delete_conversation, cnt, cnt));
        }

        final CheckBox checkbox = (CheckBox) contents.findViewById(R.id.delete_locked);
        checkbox.setVisibility(View.GONE);

        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        /// M: for ALPS01213243, dismiss the delete dialog if msg has been deleted by other way.
        sDeleteDialog = builder.setTitle(R.string.confirm_dialog_title)
            .setIconAttribute(android.R.attr.alertDialogIcon)
            .setCancelable(true)
            .setPositiveButton(R.string.delete, listener)
            .setNegativeButton(R.string.no, null)
            .setView(contents)
            .show();
    }

    private void startMsgListQuery() {
        Uri wpMessagesUri = WapPush.CONTENT_URI;
        if (wpMessagesUri == null) {
            MmsLog.w(TAG, "startMsgListQuery: wpMessagesUri is null, bail!");
            return;
        }
        MmsLog.d(TAG, "startMsgListQuery");

        // Cancel any pending queries
        mBackgroundQueryHandler.cancelOperation(MESSAGE_LIST_QUERY_TOKEN);
        try {
            // Kick off the new query
            mBackgroundQueryHandler.startQuery(MESSAGE_LIST_QUERY_TOKEN, null /* cookie */,
                    wpMessagesUri, WP_PROJECTION, null, null, "date DESC");
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    private void startUnreadQuery() {
        Uri wpMessagesUri = WapPush.CONTENT_URI;
        if (wpMessagesUri == null) {
            MmsLog.w(TAG, "startUnreadQuery: wpMessagesUri is null, bail!");
            return;
        }
        MmsLog.d(TAG, "startUnreadQuery");
        // Cancel any pending queries
        mBackgroundQueryHandler.cancelOperation(UNREAD_MESSAGES_QUERY_TOKEN);
        try {
            // Kick off the new query
            mBackgroundQueryHandler.startQuery(UNREAD_MESSAGES_QUERY_TOKEN, null /* cookie */,
                    wpMessagesUri, WP_PROJECTION, Threads.READ + "=0", null, null);
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    private void initMessageList() {
        if (mMsgListAdapter != null) {
            return;
        }

        String highlightString = getIntent().getStringExtra("highlight");
        Pattern highlight = highlightString == null ? null : Pattern.compile(
                "\\b" + Pattern.quote(highlightString), Pattern.CASE_INSENSITIVE);

        // Initialize the list adapter with a null cursor.
        mMsgListAdapter = new WPMessageListAdapter(this, null, mMsgListView, true, highlight);
        mMsgListView.setAdapter(mMsgListAdapter);
        mMsgListView.setItemsCanFocus(false);
        mMsgListView.setVisibility(View.VISIBLE);
        mMsgListView.setOnItemLongClickListener(new OnItemLongClickListener() {
            public boolean onItemLongClick(
                    AdapterView<?> parent, View view, int position, long id) {
                if (mIsSmsEnabled) {
                    mActionMode = WPMessageActivity.this.startActionMode(mActionModeListener);
                    Log.i(TAG, "OnItemLongClickListener");
                    mActionModeListener.setItemChecked(position, true);
                    if (mMsgListAdapter != null) {
                        mMsgListAdapter.notifyDataSetChanged();
                    }
                }
                return true;
            }
        });
        mMsgListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if (view == null) {
                    return;
                }
                final WPMessageItem msgItem = ((WPMessageListItem) view).getMessageItem();
                if (mActionMode != null) {
                    boolean checked = msgItem.isChecked();
                    mActionModeListener.setItemChecked(position, !checked);
                    if (mMsgListAdapter != null) {
                        mMsgListAdapter.notifyDataSetChanged();
                    }
                    return;
                }
                String messageDetails = getWPMessageDetails(WPMessageActivity.this, msgItem);
                AlertDialog.Builder b = new AlertDialog.Builder(WPMessageActivity.this);
                b.setTitle(R.string.menu_wappush);
                b.setCancelable(true);
                b.setMessage(messageDetails);

                b.setNegativeButton(android.R.string.cancel,
                        new DialogInterface.OnClickListener() {
                    @Override
                    public final void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                });
                b.setPositiveButton(R.string.visit_website, new DialogInterface.OnClickListener() {
                    @Override
                    public final void onClick(DialogInterface dialog, int which) {
                        // dialog.dismiss();
                        if (msgItem.mURL != null) {
                            Uri uri = Uri.parse(MessageUtils.checkAndModifyUrl(msgItem.mURL));
                            Intent intent = new Intent(Intent.ACTION_VIEW, uri);
                            intent.putExtra(Browser.EXTRA_APPLICATION_ID,
                                    WPMessageActivity.this.getPackageName());
                            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                            try {
                                startActivity(intent);
                            } catch (ActivityNotFoundException ex) {
                                Toast.makeText(WPMessageActivity.this,
                                        R.string.error_unsupported_scheme, Toast.LENGTH_LONG)
                                        .show();
                                MmsLog.e(TAG, "Scheme " + uri.getScheme() + "is not supported!");
                            }
                        }
                    }
                });
                b.show();
                msgItem.markAsRead();

            }
        });
    }

    private void initActivityState(Intent intent) {
        //addRecipientsListeners();
        // TODO: uncomment or not?
    }

    @Override
    public void onUserInteraction() {
        checkPendingNotification();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        if (hasFocus) {
            checkPendingNotification();
        }
    }

    private final WPMessageListAdapter.OnDataSetChangedListener mDataSetChangedListener
            = new WPMessageListAdapter.OnDataSetChangedListener() {
        public void onDataSetChanged(WPMessageListAdapter adapter) {
            mPossiblePendingNotification = true;
        }

        public void onContentChanged(WPMessageListAdapter adapter) {
            startMsgListQuery();
            startUnreadQuery();
        }
    };

    private void checkPendingNotification() {
        if (mPossiblePendingNotification && hasWindowFocus()) {
            mPossiblePendingNotification = false;
        }
    }

    private final class BackgroundQueryHandler extends BaseProgressQueryHandler {
        private NewProgressDialog dialog;

        public BackgroundQueryHandler(ContentResolver contentResolver) {
            super(contentResolver);
        }

        public void setProgressDialog(NewProgressDialog dialog) {
            this.dialog = dialog;
        }

        public void showProgressDialog() {
            if (dialog != null) {
                dialog.show();
            }
        }

        protected void dismissProgressDialog() {
            // M: fix bug ALPS00357750
            if (dialog == null) {
                MmsLog.e(TAG, "mDialog is null!");
                return;
            }

            dialog.setDismiss(true);
            try {
                dialog.dismiss();
            } catch (IllegalArgumentException e) {
                // if parent activity is destroyed,and code come here, will happen this.
                // just catch it.
                MmsLog.e(TAG, "ignore IllegalArgumentException", e);
            }
            dialog = null;
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            switch (token) {
                case MESSAGE_LIST_QUERY_TOKEN:
//                    updateEmptyView(cursor);
                    mMsgListAdapter.changeCursor(cursor);
                    // more people before the conversation begins.
                    if (mActionMode != null) {
                        mActionModeListener.confirmSyncCheckedPositons();
                    } else {
                        if (mMsgListAdapter != null) {
                            mMsgListAdapter.uncheckAll();
                        }
                    }
                    invalidateOptionsMenu(); // some menu items depend on the
                                             // adapter's count
                    if ((cursor != null) && (cursor.getCount() == 0)) {
                        mEmptyViewDefault.setVisibility(View.VISIBLE);
                    } else {
                        mEmptyViewDefault.setVisibility(View.GONE);
                    }
                    if (mNeedToMarkAsSeen) {
                        mNeedToMarkAsSeen = false;
                        markAllMessageAsSeen();
                    }
                    return;
                case UNREAD_MESSAGES_QUERY_TOKEN:
                    int count = 0;
                    if (cursor != null) {
                        // mMsgListAdapter.changeCursor(cursor);
                        count = cursor.getCount();
                        cursor.close();
                    }
                    // / M: modified for unread count display
                    if (count > MAX_DISPLAY_UNREAD_COUNT) {
                        mUnreadConvCount.setText(DISPLAY_UNREAD_COUNT_CONTENT_FOR_ABOVE_99);
                    } else {
                        mUnreadConvCount.setText(count > 0 ? Integer.toString(count) : null);
                    }
                    break;
                default:
                    break;
            }
        }

        @Override
        protected void onDeleteComplete(int token, Object cookie, int result) {
            /// M: delete the cache items after deleted the items, don't use it again.
            // for ALPS00676739 @{
            if (token == DELETE_MESSAGE_TOKEN && cookie instanceof HashSet<?>) {
                HashSet<Long> msgIds = (HashSet<Long>) cookie;
                mMsgListAdapter.deleteCachedItems(msgIds);
            }
            /// @}
            switch (token) {
//                case DELETE_ALL_MESSAGES_TOKEN:
//                    mDeleteCounter = 0;
                case DELETE_MESSAGE_TOKEN:
                    // Update the notification for new messages since they
                    // may be deleted.
                    if (mDeleteCounter > 1) {
                        mDeleteCounter--;
                        MmsLog.d(TAG,
                                "igonre a onDeleteComplete,mDeleteCounter:" + mDeleteCounter);
                        return;
                    }
                    mDeleteCounter = 0;
                    WapPushMessagingNotification.nonBlockingUpdateNewMessageIndicator(
                            WPMessageActivity.this, WapPushMessagingNotification.THREAD_NONE);
                    dismissProgressDialog();
                    break;
                default:
                    break;
            }
        }
    }

    public void onUpdate(final Contact updated) {
        // Using an existing handler for the post, rather than conjuring up a
        // new one.
        mMessageListItemHandler.post(new Runnable() {
            public void run() {
                invalidateOptionsMenu(); // some menu items depend on contact's
                                         // status.
                // bindToContactHeaderWidget(mConversation.getRecipients());
                // The contact information for one (or more) of the recipients
                // has changed.
                // Rebuild the message list so each MessageItem will get the
                // last contact info.
                WPMessageActivity.this.mMsgListAdapter.notifyDataSetChanged();
            }
        });
    }

    public static Intent createIntent(Context context, long threadId) {
        Intent intent = new Intent(context, WPMessageActivity.class);

        if (threadId > 0) {
            intent.setData(ContentUris.withAppendedId(WapPush.CONTENT_URI_THREAD, threadId));
        }

        return intent;
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        boolean ret = false;
        if (mOpWPMessageActivityExt != null) {
            ret = mOpWPMessageActivityExt.dispatchTouchEvent(ev);
        }
        if (!ret) {
            ret = super.dispatchTouchEvent(ev);
        }
        return ret;
    }

    public void setTextSize(float size) {
        if (mMsgListAdapter != null) {
            mMsgListAdapter.setTextSize(size);
        }
        if (mMsgListView != null) {
            int count = mMsgListView.getChildCount();
            for (int i = 0; i < count; i++) {
                View view = mMsgListView.getChildAt(i);
                if (view != null && view instanceof WPMessageListItem) {
                    WPMessageListItem item = (WPMessageListItem) view;
                    item.setTextSize(size);
                }
            }
        }
    }

    // / M: fix bug ALPS00380561, avoid ActivityNotFoundException @{
    @Override
    public void startActivity(Intent intent) {
        try {
            super.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Intent mChooserIntent = Intent.createChooser(intent, null);
            super.startActivity(mChooserIntent);
        }
    }

    // / @}

    private class ModeCallback implements ActionMode.Callback {
        private View mMultiSelectActionBarView;

        private Button mSelectionTitle;

        private HashSet<Long> mSelectedMsgIds;

        private HashSet<Integer> mCheckedPosition;

        private int mCheckedNum = 0;

        private MenuItem deleteitem;

        private DropDownMenu mSelectionMenu;

        private MenuItem mSelectionMenuItem;

        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            mSelectedMsgIds = new HashSet<Long>();
            mCheckedPosition = new HashSet<Integer>();
//            MenuInflater inflater = getMenuInflater();
//            inflater.inflate(R.menu.wappush_multi_select_menu, menu);
//            deleteitem = menu.findItem(R.id.delete);
            deleteitem = menu.add(0, MENU_MULTI_DELETE_MESSAGES, 0, R.string.delete_message)
                    .setIcon(R.drawable.ic_menu_trash_holo_dark)
                    .setTitle(R.string.delete_message);
            deleteitem.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            if (mMultiSelectActionBarView == null) {
                mMultiSelectActionBarView = LayoutInflater.from(WPMessageActivity.this).inflate(
                        R.layout.conversation_list_multi_select_actionbar2, null);
                mSelectionTitle = (Button) mMultiSelectActionBarView
                        .findViewById(R.id.selection_menu);
            }
            mode.setCustomView(mMultiSelectActionBarView);
            mSelectionTitle.setText(R.string.select_conversations);
            mMsgListView.setLongClickable(false);
            /// For ALPS02047260, L FW changed, onPrepareActionMode will not been called generally.
            // just call 'ActionMode.invalidate()' can make it to run onPrepareActionMode.
            // so, move the onPrepareActionMode codes into onCreateActionMode to
            // make sure views init success. @{
            CustomMenu customMenu = new CustomMenu(WPMessageActivity.this);
            mSelectionMenu = customMenu.addDropDownMenu(mSelectionTitle, R.menu.selection);
            mSelectionMenuItem = mSelectionMenu.findItem(R.id.action_select_all);
            customMenu.setOnMenuItemClickListener(new OnMenuItemClickListener() {
                public boolean onMenuItemClick(MenuItem item) {
                    if (mMsgListAdapter.isAllSelected()) {
                        setAllItemChecked(mActionMode, false);
                    } else {
                        setAllItemChecked(mActionMode, true);
                    }
                    return false;
                }
            });
            /// @}
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            return true;
        }

        public boolean onActionItemClicked(final ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
                case MENU_MULTI_DELETE_MESSAGES:
                    if (mSelectedMsgIds.size() > 0) {
                        // Log.v(TAG, "ConversationList->ModeCallback: delete");
                        //
                        DeleteMessageListener l = new DeleteMessageListener(mSelectedMsgIds,
                                mBackgroundQueryHandler, WPMessageActivity.this, mode);
                        confirmDeleteMessageDialog(l, mSelectedMsgIds, WPMessageActivity.this);
                        //
                    } else {
                        item.setEnabled(false);
                    }
                    break;

                default:
                    if (mCheckedPosition != null && mCheckedPosition.size() > 0) {
                        mCheckedPosition.clear();
                    }
                    break;
            }
            return true;
        }

        public void onDestroyActionMode(ActionMode mode) {
            mMsgListAdapter.uncheckSelect(mCheckedPosition);
            mCheckedPosition = null;
            mSelectedMsgIds = null;
            mMsgListView.setLongClickable(true);
            mCheckedNum = 0;
            mActionMode = null;
            if (mMsgListAdapter != null) {
                mMsgListAdapter.notifyDataSetChanged();
            }
        }

        public void setItemChecked(int position, boolean checked) {
            Cursor cursor = (Cursor) mMsgListView.getItemAtPosition(position);
            int type = cursor.getInt(COLUMN_WPMS_TYPE);
            long msgId = cursor.getLong(COLUMN_ID);
            WPMessageItem msgItem = mMsgListAdapter.getCachedMessageItem(type, msgId, cursor);
            if (checked == msgItem.isChecked()) {
                return;
            }
            msgItem.setIsChecked(checked);
            if (checked) {
                mSelectedMsgIds.add(msgId);
                mCheckedPosition.add(position);
                mCheckedNum++;
            } else {
                mSelectedMsgIds.remove(msgId);
                mCheckedPosition.remove(position);
                mCheckedNum--;
            }

            if (deleteitem != null) {
                if (mCheckedNum > 0) {
                    deleteitem.setEnabled(true);
                } else {
                    deleteitem.setEnabled(false);
                }
            }
            if (mCheckedNum == 0 && mActionMode != null) {
                mActionMode.finish();
            }
            mSelectionTitle.setText(WPMessageActivity.this.getResources().getQuantityString(
                    R.plurals.message_view_selected_message_count, mCheckedNum, mCheckedNum));
            updateSelectionTitle();
        }

        private void setAllItemChecked(ActionMode mode, boolean checked) {
            int num = mMsgListAdapter.getCount();
            for (int position = 0; position < num; position++) {
                setItemChecked(position, checked);
            }
            if (checked) {
                deleteitem.setEnabled(true);
            } else {
                deleteitem.setEnabled(false);
            }
            if (mMsgListAdapter != null) {
                mMsgListAdapter.notifyDataSetChanged();
            }
        }

        public void confirmSyncCheckedPositons() {
            mCheckedPosition.clear();
            mSelectedMsgIds.clear();
            int num = mMsgListView.getCount();

            for (int position = 0; position < num; position++) {
                Cursor cursor = (Cursor) mMsgListView.getItemAtPosition(position);
                int type = cursor.getInt(COLUMN_WPMS_TYPE);
                long msgId = cursor.getLong(COLUMN_ID);
                WPMessageItem msgItem = mMsgListAdapter.getCachedMessageItem(type, msgId, cursor);
                if (msgItem.isChecked()) {
                    mCheckedPosition.add(position);
                    mSelectedMsgIds.add(msgId);
                }
            }
            mCheckedNum = mCheckedPosition.size();
            /// M: for ALPS01213243, dismiss the delete dialog if message has been deleted by
            //other way. @{
            if (mCheckedNum == 0 && mActionMode != null) {
                if (sDeleteDialog != null && sDeleteDialog.isShowing()) {
                    sDeleteDialog.dismiss();
                    sDeleteDialog = null;
                }
                if (mMsgListAdapter.getCount() == 0) {
                    mActionMode.finish();
                }
            }
            /// @}
            mSelectionTitle.setText(WPMessageActivity.this.getResources().getQuantityString(
                    R.plurals.message_view_selected_message_count, mCheckedNum, mCheckedNum));
            updateSelectionTitle();
        }

        private void updateSelectionTitle() {
            if (mSelectionMenuItem != null) {
                if (mMsgListAdapter.isAllSelected()) {
                    mSelectionMenuItem.setTitle(R.string.unselect_all);
                } else {
                    mSelectionMenuItem.setTitle(R.string.select_all);
                }
            }
        }
    }

    /// M: Code analyze 001, For new feature ALPS00131956, wappush: mark all wappush
    /// message as seen . @{
    public void markAllMessageAsSeen() {
        Thread thread = new Thread(new Runnable() {
            public void run() {
                    blockingMarkAllWapPushMessagesAsSeen(WPMessageActivity.this);
                    WapPushMessagingNotification.blockingUpdateNewMessageIndicator(
                            WPMessageActivity.this, WapPushMessagingNotification.THREAD_NONE);
            }
        });
        thread.setPriority(Thread.MIN_PRIORITY);
        thread.start();
    }

    private static void blockingMarkAllWapPushMessagesAsSeen(final Context context) {
        ContentResolver resolver = context.getContentResolver();
        Cursor cursor = resolver.query(WapPush.CONTENT_URI,
                SEEN_PROJECTION, UNSEEN_SELECTION, null, null);
        int count = 0;
        if (cursor != null) {
            try {
                count = cursor.getCount();
            } finally {
                cursor.close();
            }
        }
        if (count == 0) {
            return;
        }
        ContentValues values = new ContentValues(1);
        values.put("seen", 1);
        resolver.update(WapPush.CONTENT_URI, values, UNSEEN_SELECTION, null);
    }
}
    /// @}
    class DeleteProgressDialogUtil {
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

