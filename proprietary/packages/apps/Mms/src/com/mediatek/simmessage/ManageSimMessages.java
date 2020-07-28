/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
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

package com.mediatek.simmessage;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Telephony;
import android.telephony.SmsManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.SpannableString;
import android.text.style.URLSpan;
import android.text.util.Linkify;
import android.util.Log;
import android.view.ContextMenu;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.TextView;
import android.widget.Toast;

import com.android.browser.provider.Browser;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.data.ContactList;
import com.android.mms.ui.ComposeMessageActivity;
import com.android.mms.ui.ConversationList;
import com.android.mms.ui.CustomMenu;
import com.android.mms.ui.CustomMenu.DropDownMenu;
import com.android.mms.ui.MessageListAdapter;
import com.android.mms.ui.MessageListItem;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MessageResource;
import com.android.mms.util.MmsLog;
import com.android.mms.util.Recycler;
import com.android.mms.widget.MmsWidgetProvider;

import android.support.v4.content.LocalBroadcastManager;

import com.mediatek.internal.telephony.MtkIccSmsStorageStatus;
import com.mediatek.mms.callback.ITextSizeAdjustHost;
import com.mediatek.mms.ext.IOpManageSimMessagesExt;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SettingListActivity;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import mediatek.telephony.MtkSmsManager;
import mediatek.telephony.MtkTelephony;


/**
 * Displays a list of the SMS messages stored on the ICC.
 */
public class ManageSimMessages extends Activity
        implements View.OnCreateContextMenuListener,
                   ITextSizeAdjustHost, Contact.UpdateListener {
    private static final String TAG = "ManageSimMessages";
    private static final int MENU_COPY_TO_PHONE_MEMORY = 0;
    private static final int MENU_DELETE_FROM_SIM = 1;

    private static final int OPTION_MENU_DELETE = 0;

    private static final int SHOW_LIST = 0;
    private static final int SHOW_EMPTY = 1;
    private static final int SHOW_BUSY = 2;
    private int mState;

    private ContentResolver mContentResolver;
    private Cursor mCursor = null;
    private ListView mSimList;
    private TextView mMessage;
    private MessageListAdapter mMsgListAdapter = null;
    private AsyncQueryHandler mQueryHandler = null;

    public static final int SIM_FULL_NOTIFICATION_ID = 234;

    /// M:
    private static final int DIALOG_REFRESH = 1;
    private static final int DIALOG_CAPACITY = 2;
    private static Uri sSimMessageUri;

    private static final Uri ICC_URI = Uri.parse("content://sms/icc");
    private static final int MENU_FORWARD = 2;
    private static final int MENU_REPLY = 3;
    private static final int MENU_ADD_TO_BOOKMARK      = 4;
    private static final int MENU_CALL_BACK            = 5;
    private static final int MENU_SEND_EMAIL           = 6;
    private static final int MENU_ADD_ADDRESS_TO_CONTACTS = 7;
    private static final int MENU_SEND_SMS              = 9;
    private static final int MENU_ADD_CONTACT           = 10;
    private static final int OPTION_MENU_SIM_CAPACITY = 1;

    ///M: Sim message column index. @{
    private static int COLUMN_SERVICE_CENTER_ADDRESS = 0;
    private static int COLUMN_ADDRESS = 1;
    private static int COLUMN_MESSAGE_CLASS = 2;
    private static int COLUMN_BODY = 3;
    private static int COLUMN_DATE = 4;
    private static int COLUMN_STATUS = 5;
    private static int COLUMN_INDEX_ON_ICC = 6;
    private static int COLUMN_IS_STATUS_REPORT = 7;
    private static int COLUMN_TRANSPORT_TYPE = 8;
    private static int COLUMN_TYPE = 9;
    private static int COLUMN_LOCKED = 10;
    private static int COLUMN_ERROR_CODE = 11;
    private static int COLUMN_ID = 12;
    private static int COLUMN_SIM_ID = 13;
    /// @}

    ProgressDialog mDialog;
    private static final String ALL_SMS = "999999";
    private static final String FOR_MULTIDELETE = "ForMultiDelete";
    private static final String CAPACITY_KEY = "capacity";
    // M: fix for bug ALPS01468873
    /*
    public static final String ACTION_NOTIFY_SIMMESSAGE_UPDATE =
            "com.android.mms.ACTION_NOTIFY_SIMMESSAGE_UPDATE";
    */
    private int mCurrentSubId = 0;
    public boolean isQuerying = false;
    public boolean isDeleting = false;
    /// M: extract telephony number ...
    private ArrayList<String> mURLs = new ArrayList<String>();
    private ContactList mContactList;
    ///M: add for plugins
    private IOpManageSimMessagesExt mOpManageSimMessages = null;
    ///M: fix bug ALPS00850867.
    private boolean mIsCurrentSimFull;

    /// M: fix bug ALPS00448222, posting UI update Runnables for Contact update
    private Handler mHandler = new Handler();

    /// M: add to fix costing time when delete message.
    private HashSet<Long> mDeletedMessageSet = new HashSet<Long>();

    ///M: add for avoid ANR when check sim state
    private QueryIccTask mQueryIccTask;
    // M: add for ALPS01857861, mark the CheckSimCapacityTask and cancel it when finish.
    private CheckSimCapacityTask mCheckMemoryTask;
    // M: fix for bug ALPS01468873
    //private boolean shouldRequery;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        try {
            dismissDialog(DIALOG_CAPACITY);
        } catch (IllegalArgumentException e) {
            // dialog is not showing
            MmsLog.e(TAG, "onCreate IllegalArgumentException " +  e);
        }
        /// M: fix bug ALPS00414035
        //IntentFilter intentFilter = new IntentFilter(
        //EncapsulatedTelephonyIntents.ACTION_SIM_INDICATOR_STATE_CHANGED);
        //registerReceiver(mSimReceiver, intentFilter);
        registerReceiver(mSimReceiver, new IntentFilter(
                TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED));
        ///M: for 'finish activity when airplane mode on' function.
        registerReceiver(airPlaneReceiver, new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED));
        registerReceiver(mSimReceiver, new IntentFilter(Telephony.Sms.Intents.SIM_FULL_ACTION));
        // M: fix for bug ALPS01468873
        //registerReceiver(mSimMessageUpdate,new IntentFilter(ACTION_NOTIFY_SIMMESSAGE_UPDATE));

        /// M: @{
        mCurrentSubId = getIntent().getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, 0);
        sSimMessageUri = ICC_URI.buildUpon().appendQueryParameter(PhoneConstants.SUBSCRIPTION_KEY,
        String.valueOf(mCurrentSubId)).build();
        MmsLog.ipi(TAG, "onCreate sSimMessageUri = " + sSimMessageUri
                + " Got sub id is : " + mCurrentSubId);
        /// @}

        // M: add for op
        mOpManageSimMessages = OpMessageUtils.getOpMessagePlugin().getOpManageSimMessagesExt();
        mOpManageSimMessages.onCreate(this, this, mCurrentSubId);

        setContentView(R.layout.sim_list);
        mContentResolver = getContentResolver();
        mQueryHandler = new QueryHandler(mContentResolver, this);
        mSimList = (ListView) findViewById(R.id.messages);
        mMessage = (TextView) findViewById(R.id.empty_message);

        mActionBar = getActionBar();
        mActionBar.setDisplayHomeAsUpEnabled(true);

        /// M: fix bug ALPS00448222, update ListView when contact update
        Contact.addListener(this);

        NotifyViewed();
        // start query
        mQueryIccTask = new QueryIccTask();
        mQueryIccTask.execute();
    }

    @Override
    protected void onNewIntent(Intent intent) {
        setIntent(intent);
        /// M: @{
        mCurrentSubId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, 0);
        MmsLog.d(TAG, "onNewIntent Got slot id is : " + mCurrentSubId);
        sSimMessageUri = ICC_URI.buildUpon().appendQueryParameter(PhoneConstants.SUBSCRIPTION_KEY,
        String.valueOf(mCurrentSubId)).build();

        /// @}
        //MessagingNotification.cancelNotification(getApplicationContext(),
        //SIM_FULL_NOTIFICATION_ID);
        NotifyViewed();
        try {
            dismissDialog(DIALOG_CAPACITY);
        } catch (IllegalArgumentException e) {
            // dialog is not showing
            MmsLog.e(TAG, "dismiss diaolog DIALOG_CAPACITY exception: " +  e);
        }
        mQueryIccTask.cancel(true);
        mQueryIccTask = new QueryIccTask();
        mQueryIccTask.execute();
    }

    private void NotifyViewed() {
        Intent intent = new Intent();
        // memslim
        //intent.setAction(SimFullReceiver.SIM_FULL_VIEWED_ACTION);
        intent.setAction(MmsApp.SIM_FULL_VIEWED_ACTION);
        intent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, mCurrentSubId);
        MmsLog.d(TAG, "NotifyViewed subid: " + mCurrentSubId);

        LocalBroadcastManager lbm = LocalBroadcastManager.getInstance(getApplicationContext());
        lbm.sendBroadcast(intent);
    }

    /**
     * M: Put checking SIM capacity action to background, because sometimes it cost long and
     * caused ANR.
     * And AP must execute query capacity and ICC message sync, can't asynchronous.
     */
    private class QueryIccTask extends AsyncTask<Void, Void, Boolean> {
        protected void onPreExecute() {
            super.onPreExecute();
            MmsLog.d(TAG, "QueryIccTask onPreExecute");
            isQuerying = true;
            updateState(SHOW_BUSY);
        }

        @Override
        protected Boolean doInBackground(Void... params) {
            MmsLog.d(TAG, "QueryIccTask doInBackground.");
            // check SIM whether is empty, need not query SIM messages if is empty.
            boolean isSimEmpty = false;
            MtkIccSmsStorageStatus simMemStatus = MtkSmsManager.getSmsManagerForSubscriptionId(
                    mCurrentSubId).getSmsSimMemoryStatus();
            if (null != simMemStatus) {
                MmsLog.d(TAG, "isCurrentSimEmpty isEmpty= " + isSimEmpty);
                isSimEmpty = simMemStatus.getUsedCount() == 0;
            }
            return isSimEmpty;
        }

        @Override
        protected void onPostExecute(Boolean isSimEmpty) {
            MmsLog.d(TAG, "QueryIccTask onPostExecute isSimEmpty: " + isSimEmpty);
            if (isSimEmpty) {
                // need not query SIM messages if SIM is empty.
                isQuerying = false;
                updateListWithCursor(null);
            } else {
                startQueryIcc();
            }
        }
    }

    private class QueryHandler extends AsyncQueryHandler {
        public QueryHandler(
                ContentResolver contentResolver, ManageSimMessages parent) {
            super(contentResolver);
        }

        @Override
        protected void onDeleteComplete(int token, Object expectDeleteNum, int actualDeletedNum) {
            super.onDeleteComplete(token, expectDeleteNum, actualDeletedNum);
            if (ManageSimMessages.this.isFinishing()) {
                return;
            }
            ///M: change for improve deleting and re-querying message flow performance.
            // 1. set isDeleting flag as false when delete completely.
            // 2. update the list manually if deleted successful.
            // 3. show failure toast if deleted fail. @{
            MmsLog.d(TAG, "onDeleteComplete expectDeleteNum " + expectDeleteNum +
                    " actualDeletedNum " + actualDeletedNum);
            isDeleting = false;
            if (actualDeletedNum <= 0) {
                checkDeleteMode();
                invalidateOptionsMenu();
                Toast.makeText(ManageSimMessages.this, getString(R.string.delete_unsuccessful),
                        Toast.LENGTH_SHORT).show();
                updateState(SHOW_LIST);
            } else if (actualDeletedNum == (Integer) expectDeleteNum) {
                updateListAfterDelete();
            } else {
                mQueryIccTask.cancel(true);
                mQueryIccTask = new QueryIccTask();
                mQueryIccTask.execute();
            }
            mDeletedMessageSet.clear();
            /// @}
            /// M: cancel the SIM FULL notification if deleted successful.
            if (actualDeletedNum > 0 && mIsCurrentSimFull) {
                /*MessagingNotification.cancelNotification(getApplicationContext(),
                        SIM_FULL_NOTIFICATION_ID);*/
                NotifyViewed();
            }
            /// @}
        }

        @Override
        protected void onQueryComplete(
                int token, Object cookie, Cursor cursor) {
            MmsLog.d(TAG, "onQueryComplete ");
            if (ManageSimMessages.this.isFinishing()) {
                return;
            }
            mQueryHandler.removeCallbacksAndMessages(null);
            isQuerying = false;
            updateListWithCursor(cursor);
        }
    }

    private void updateListWithCursor(Cursor cursor) {
        if (mCursor != null && !mCursor.isClosed()) {
            stopManagingCursor(mCursor);
        }
        MmsLog.d(TAG, "updateListWithCursor cursor: " + mCursor);
        mCursor = cursor;
        if (mCursor != null && mCursor.moveToFirst()) {
            mMsgListAdapter = new MessageListAdapter(this, mCursor, mSimList, false, null);
            MmsLog.d(TAG, "updateListWithCursor cursor size is " + mCursor.getCount());
            // Note that the MessageListAdapter doesn't support auto-requeries. If we
            // want to respond to changes we'd need to add a line like:
            //   mMsgListAdapter.setOnDataSetChangedListener(mDataSetChangedListener);
            // See ComposeMessageActivity for an example.
            mMsgListAdapter.setMsgListItemHandler(mMessageListItemHandler);
            mSimList.setAdapter(mMsgListAdapter);
            float textSize = MessageUtils.getPreferenceValueFloat(ManageSimMessages.this,
                    SettingListActivity.TEXT_SIZE, 18);
            setTextSize(textSize);
            mSimList.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    if (view != null) {
                        MessageListItem mli = (MessageListItem) view;
                        // / M: add for multi-delete
                        if (mli.mSelectedBox != null
                                && mli.mSelectedBox.getVisibility() == View.VISIBLE) {
                            if (!mli.mSelectedBox.isChecked()) {
                                mli.setSelectedBackGroud(true);
                            } else {
                                mli.setSelectedBackGroud(false);
                            }
                            Cursor cursor = (Cursor) mMsgListAdapter.getCursor();
                            String msgIndex = cursor.getString(cursor
                                    .getColumnIndexOrThrow("index_on_icc"));
                            MmsLog.d(MmsApp.TXN_TAG, "simMsg msgIndex = " + msgIndex);
                            String[] index = msgIndex.split(";");
                            mMsgListAdapter.changeSelectedState(index[0]);
                            updateActionBarText();
                            return;
                        }
                        mli.onMessageListItemClick();
                    }
                }
            });
            mSimList.setOnCreateContextMenuListener(this);
            updateState(SHOW_LIST);
            startManagingCursor(mCursor);
        } else {
            MmsLog.d(TAG, "updateListWithCursor cursor is null");
            // Let user know the SIM is empty
            // make a mock adapter avoid JE.
            mMsgListAdapter = new MessageListAdapter(this, mCursor, mSimList, true, null);
            updateState(SHOW_EMPTY);
        }
        /// M: invoke this, so onPrepareOptionsMenu will be invoked. refresh the menu.
        invalidateOptionsMenu();
        checkDeleteMode();

        // add for op
        mOpManageSimMessages.updateListWithCursor(cursor);
    }

    private void startQueryIcc() {
        try {
            /// M: add for OP
            Uri queryUri = mOpManageSimMessages.startQueryIcc(sSimMessageUri);
            /// @}
            mQueryHandler.startQuery(0, null, queryUri, null, null, null, null);
            MmsLog.dpi(TAG, "startQueryIcc " + queryUri);
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    @Override
    public void onCreateContextMenu(
            ContextMenu menu, View v,
            ContextMenu.ContextMenuInfo menuInfo) {
        if (mMsgListAdapter != null && mMsgListAdapter.mIsDeleteMode) {
            return;
        }

        /// M: Add for OP
        if (mOpManageSimMessages.onCreateContextMenu(mMsgListAdapter.getCursor())) {
           return;
        }
        /// @}
        menu.setHeaderTitle(R.string.message_options);
        AdapterView.AdapterContextMenuInfo info = null;
        try {
             info = (AdapterView.AdapterContextMenuInfo) menuInfo;
        } catch (ClassCastException exception) {
            Log.e(TAG, "Bad menuInfo.", exception);
        }
        if (info != null) {
            final Cursor cursor = (Cursor) mMsgListAdapter.getItem(info.position);
            addCallAndContactMenuItems(menu, cursor);
        }
        menu.add(0, MENU_FORWARD, 0, R.string.menu_forward);
        menu.add(0, MENU_REPLY, 0, R.string.menu_reply);
        menu.add(0, MENU_COPY_TO_PHONE_MEMORY, 0,
                R.string.sim_copy_to_phone_memory);
        if ((null != mCursor) && (mCursor.getCount()) > 0) {
            menu.add(0, MENU_DELETE_FROM_SIM, 0, R.string.sim_delete);
        }
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        MmsLog.d(TAG, "onContextItemSelected " + item.getItemId());
        AdapterView.AdapterContextMenuInfo info;
        try {
             info = (AdapterView.AdapterContextMenuInfo) item.getMenuInfo();
        } catch (ClassCastException exception) {
            Log.e(TAG, "Bad menuInfo.", exception);
            return false;
        }

        final Cursor cursor = (Cursor) mMsgListAdapter.getItem(info.position);
        if (cursor == null) {
            MmsLog.e(TAG, "Bad menuInfo, cursor is null");
            return false;
        }
        switch (item.getItemId()) {
        case MENU_COPY_TO_PHONE_MEMORY:
            copyToPhoneMemory(cursor);
            return true;
        case MENU_DELETE_FROM_SIM:
            final String msgIndex = getMsgIndexByCursor(cursor);
            // change for ALPS01909522, the cursor will move to the last message in screen by
            // re-bind view, so get out the msgId before re-bind view come.
            final long msgId = cursor.getLong(cursor.getColumnIndexOrThrow("_id"));
            confirmDeleteDialog(new OnClickListener() {
                public void onClick(DialogInterface dialog, int which) {
                    updateState(SHOW_BUSY);
                    new Thread(new Runnable() {
                        public void run() {
                            mDeletedMessageSet.add(msgId);
                            // deleteFromSim(msgIndex);
                            String[] index = msgIndex.split(";");
                            // Uri simUri =
                            // sDeleteAllContentUri.buildUpon().build();
                            Log.d(TAG, "onContextItemSelected startDelete length:" + index.length);
                            mQueryHandler.startDelete(/* DELETE_MESSAGE_TOKEN */0,
                                    index.length, sSimMessageUri, FOR_MULTIDELETE,
                                    index);
                            isDeleting = true;
                        }
                    }, "ManageSimMessages").start();
                    dialog.dismiss();
                }
            }, R.string.confirm_delete_SIM_message);
            return true;
        case MENU_FORWARD:
            forwardMessage(cursor);
            return true;
        case MENU_REPLY:
            replyMessage(cursor);
            return true;
        case MENU_ADD_TO_BOOKMARK:
            if (mURLs.size() == 1) {
                Browser.saveBookmark(ManageSimMessages.this, null, mURLs.get(0));
            } else if (mURLs.size() > 1) {
                CharSequence[] items = new CharSequence[mURLs.size()];
                for (int i = 0; i < mURLs.size(); i++) {
                    items[i] = mURLs.get(i);
                }
                new AlertDialog.Builder(ManageSimMessages.this)
                    .setTitle(R.string.menu_add_to_bookmark)
                    .setItems(items, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            Browser.saveBookmark(ManageSimMessages.this, null, mURLs.get(which));
                            }
                        })
                    .show();
            }
            return true;

        case MENU_ADD_CONTACT:
            String number = cursor.getString(cursor.getColumnIndexOrThrow("address"));
            startActivity(ConversationList.createAddContactIntent(number));
            return true;
        }
        return super.onContextItemSelected(item);
    }

    @Override
    public void onResume() {
        MmsLog.d(TAG, "onResume isQuerying: " + isQuerying);
        super.onResume();
        mOpManageSimMessages.onResume();

        ///M: do not show sim messages when sim is off. @{
        if (!MessageUtils.isSimMessageAccessable(this, mCurrentSubId)) {
            if (!MmsConfig.isSmsEnabled(this)) {
                Toast.makeText(getApplicationContext(),
                        getString(R.string.pref_title_sms_disabled), Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(getApplicationContext(),
                        getString(MessageResource.string.sim_close),
                        Toast.LENGTH_SHORT).show();
            }
            finish();
            return;
        }
        /// @}

        if (isQuerying) {
            // This means app is querying SIM SMS when left activity last time
            updateState(SHOW_BUSY);
        }
    }

    @Override
    public void onPause() {
        MmsLog.d(TAG, "onPause");
        super.onPause();
        //invalidate cache to refresh contact data
        Contact.invalidateCache();
    }

    private void copyToPhoneMemory(Cursor cursor) {
        final String address = cursor.getString(cursor.getColumnIndexOrThrow("address"));
        final String body = cursor.getString(cursor.getColumnIndexOrThrow("body"));
        final Long date = cursor.getLong(cursor.getColumnIndexOrThrow("date"));
        final String serviceCenter = cursor.getString(
                cursor.getColumnIndexOrThrow("service_center_address"));
        final boolean isIncomingMessage = isIncomingMessage(cursor);
        /// M: fix ALPS01659494,notify user can not save null address message
        if (address == null || address.equals("")) {
            MmsApp.getToastHandler().sendEmptyMessage(MmsApp.MSG_MMS_CAN_NOT_SAVE);
            MmsLog.d(MmsApp.TXN_TAG, "copyToPhoneMemory address is null ,return ");
            return;
          }
        /// @}
        new Thread(new Runnable() {
            public void run() {
                try {
                    if (isIncomingMessage) {
                        MmsLog.d(MmsApp.TXN_TAG, "Copy incoming sms to phone");
                        MtkTelephony.MtkSms.Inbox.addMessage(
                                mCurrentSubId, mContentResolver,
                                address, body, null, serviceCenter,
                                date, true);
                    } else {
                        // / M: outgoing sms has not date info
                        Long currentTime = System.currentTimeMillis();
                        MmsLog.d(MmsApp.TXN_TAG, "Copy outgoing sms to phone");
                        MtkTelephony.MtkSms.Sent.addMessage(mCurrentSubId,
                                mContentResolver, address, body, null,
                                serviceCenter, currentTime);
                    }
                    Recycler.getSmsRecycler().deleteOldMessages(
                            getApplicationContext());
                    MmsApp.getToastHandler().sendEmptyMessage(MmsApp.MSG_DONE);
                    MmsWidgetProvider.notifyDatasetChanged(getApplicationContext());
                } catch (SQLiteException e) {
                    SqliteWrapper.checkSQLiteException(getApplicationContext(), e);
                }
            }
        }, "copyToPhoneMemory").start();
    }

    private boolean isIncomingMessage(Cursor cursor) {
        int messageStatus = cursor.getInt(
                cursor.getColumnIndexOrThrow("status"));
        MmsLog.d(MmsApp.TXN_TAG, "message status:" + messageStatus);
        return (messageStatus == SmsManager.STATUS_ON_ICC_READ) ||
               (messageStatus == SmsManager.STATUS_ON_ICC_UNREAD);
    }

    private String getMsgIndexByCursor(Cursor cursor) {
        return cursor.getString(cursor.getColumnIndexOrThrow("index_on_icc"));
    }

    private Menu mOptionMenu;
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        mOptionMenu = menu;
        menu.add(0, OPTION_MENU_DELETE, 0, R.string.menu_delete_messages)
                .setIcon(android.R.drawable.ic_menu_delete);
        menu.add(0, OPTION_MENU_SIM_CAPACITY, 0,
                R.string.menu_show_icc_sms_capacity).setIcon(R.drawable.ic_menu_sim_capacity);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);

        /// M: fix bug ALPS00526071, remove "SimCapacity" menuitem in Sim Sms Selection Mode
        boolean isShowMenu = setOptionMenu();
        return true;
    }

    private boolean setOptionMenu() {
        if (mOptionMenu == null) {
            return false;
        }

        /// M: fix bug ALPS00526071, remove "SimCapacity" menuitem in Sim Sms Selection Mode @{
        boolean isShowDelectAll = (null != mCursor) && (mCursor.getCount() > 0)
                                && mState == SHOW_LIST && !mMsgListAdapter.mIsDeleteMode;

        boolean isShowCapacity = (mState == SHOW_LIST || mState == SHOW_EMPTY)
                                && (mMsgListAdapter == null || !mMsgListAdapter.mIsDeleteMode);
        /// @}

        MenuItem miDeleteAll = mOptionMenu.findItem(OPTION_MENU_DELETE);
        if (miDeleteAll != null) {
            miDeleteAll.setVisible(isShowDelectAll);
        }

        MenuItem miSimCapacity = mOptionMenu.findItem(OPTION_MENU_SIM_CAPACITY);
        if (miSimCapacity != null) {
            miSimCapacity.setVisible(isShowCapacity);
        }

        return isShowDelectAll || isShowCapacity;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case OPTION_MENU_DELETE:
                mMsgListAdapter.mIsDeleteMode = true;
                item.setVisible(false);
                checkDeleteMode();
                /// M: fix bug ALPS00526071, remove "SimCapacity" menuitem
                invalidateOptionsMenu();
                /// @}
                break;
            case OPTION_MENU_SIM_CAPACITY:
                ///M: Put checking SIM capacity action to background,
                // because sometimes it cost long and
                // caused ANR. mark the CheckSimCapacityTask and cancel it when finish. @{
                mCheckMemoryTask = new CheckSimCapacityTask();
                mCheckMemoryTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
                break;
                /// @}
            case android.R.id.home:
                if (mMsgListAdapter != null && mMsgListAdapter.mIsDeleteMode) {
                    // for ALPS01831024, show home key in delete mode, cancel delete if click it.
                    cancelDeleteMode();
                } else {
                    // The user clicked on the Message icon in the action bar. Take them back from
                    // wherever they came from
                    finish();
                }
                break;
        }

        return true;
    }

    private void confirmDeleteDialog(OnClickListener listener, int messageId) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.confirm_dialog_title);
        builder.setIconAttribute(android.R.attr.alertDialogIcon);
        builder.setCancelable(true);
        builder.setPositiveButton(R.string.yes, listener);
        builder.setNegativeButton(R.string.no, null);
        builder.setMessage(messageId);

        // M: add for OP
        mOpManageSimMessages.confirmDeleteDialog(builder);

        builder.show();
    }

    private void updateState(int state) {
        MmsLog.d(TAG, "updateState, state = " + state);
        if (mState == state) {
            return;
        }

        mState = state;
        switch (state) {
            case SHOW_LIST:
                try {
                    dismissDialog(DIALOG_REFRESH);
                } catch (IllegalArgumentException e) {
                    // dialog is not showing
                    MmsLog.e(TAG, "dismiss diaolog DIALOG_REFRESH exception: " +  e);
                }
                mSimList.setVisibility(View.VISIBLE);
                mSimList.requestFocus();
                mMessage.setVisibility(View.GONE);

                /// M: change for OP @{
                if (!mOpManageSimMessages.updateState(state, null)) {
                    setTitle(getString(R.string.sim_manage_messages_title));
                } /// @}
                try {
                    dismissDialog(DIALOG_REFRESH);
                } catch (IllegalArgumentException e) {
                    // dialog is not showing
                    MmsLog.e(TAG, "dismiss diaolog DIALOG_REFRESH exception: " +  e);
                }
                break;
            case SHOW_EMPTY:
                try {
                    dismissDialog(DIALOG_REFRESH);
                } catch (IllegalArgumentException e) {
                    // dialog is not showing
                    MmsLog.e(TAG, "dismiss diaolog DIALOG_REFRESH exception: " +  e);
                }
                mSimList.setVisibility(View.GONE);

                /// M: change for OP @{
                if (!mOpManageSimMessages.updateState(state, mMessage)) {
                    setTitle(getString(R.string.sim_manage_messages_title));
                } /// @}

                mMessage.setVisibility(View.VISIBLE);
                break;
            case SHOW_BUSY:
                mSimList.setVisibility(View.GONE);
                mMessage.setVisibility(View.GONE);
                setTitle(getString(R.string.refreshing));
                showDialog(DIALOG_REFRESH);
                break;
            default:
                Log.e(TAG, "Invalid State");
        }
    }

    private void viewMessage(Cursor cursor) {
        // TODO: Add this.
    }

    @Override
    protected void onDestroy() {
        MmsLog.d(TAG, "onDestroy");
        super.onDestroy();

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        /// M: fix bug ALPS00414035

        try {
            if (mSimReceiver != null) {
                unregisterReceiver(mSimReceiver);
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "onDestroy:" + e);
        }

        try {
        ///M: for 'finish activity when airplane mode on' function.
            if (airPlaneReceiver != null) {
                unregisterReceiver(airPlaneReceiver);
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "onDestroy:" + e);
        }
        // M: fix for bug ALPS01468873
        //unregisterReceiver(mSimMessageUpdate);
        if (mQueryHandler != null) {
            mQueryHandler.removeCallbacksAndMessages(null);
        }

        /// M: fix bug ALPS00448222, update ListView when contact update
        Contact.removeListener(this);
        /// M: add for ALPS01857861, cancel tasks when finish. @{
        mQueryIccTask.cancel(true);
        if (mCheckMemoryTask != null) {
            mCheckMemoryTask.cancel(true);
        }
        /// @}
        if (mSelectionMenu != null) {
            mSelectionMenu.dismiss();
        }
    }

    @Override
    public void startActivity(Intent intent) {
        try {
            super.startActivity(intent);
        } catch (ActivityNotFoundException e) {
            Intent mChooserIntent = Intent.createChooser(intent, null);
            super.startActivity(mChooserIntent);
        }
    }

    private void forwardMessage(Cursor cursor) {
        String body = cursor.getString(cursor.getColumnIndexOrThrow("body"));
        Intent intent = new Intent();
        intent.setClassName(this, "com.android.mms.ui.ForwardMessageActivity");
        intent.putExtra(ComposeMessageActivity.FORWARD_MESSAGE, true);
        if (body != null) {


            MmsLog.d(TAG, "call forwardMessage !!!");
            String addr = cursor.getString(cursor.getColumnIndexOrThrow("address"));
            Contact contact = Contact.get(addr, false);
            String nameAndNumber = Contact.formatNameAndNumber(contact.getName(),
                    contact.getNumber(), "");
            body = mOpManageSimMessages.forwardMessage(this, body, nameAndNumber, cursor);
            intent.putExtra(ComposeMessageActivity.SMS_BODY, body);
        }
        startActivity(intent);
    }

    private void replyMessage(Cursor cursor) {
        String address = cursor.getString(cursor.getColumnIndexOrThrow("address"));
        Intent intent = new Intent(Intent.ACTION_SENDTO,
                Uri.fromParts("sms", address, null));
        startActivity(intent);
    }

    private void addCallAndContactMenuItems(ContextMenu menu, Cursor cursor) {
        /// M: Add all possible links in the address & message
        StringBuilder textToSpannify = new StringBuilder();
        String reciBody = cursor.getString(cursor.getColumnIndexOrThrow("body"));
        String reciNumber = cursor.getString(cursor.getColumnIndexOrThrow("address"));
        textToSpannify.append(reciNumber + ": ");
        textToSpannify.append(reciBody);
        SpannableString msg = new SpannableString(textToSpannify.toString());
        Linkify.addLinks(msg, Linkify.EMAIL_ADDRESSES|Linkify.PHONE_NUMBERS|Linkify.WEB_URLS);
        ArrayList<String> uris =
            MessageUtils.extractUris(msg.getSpans(0, msg.length(), URLSpan.class));
        mURLs.clear();
        MmsLog.dpi(TAG, "addCallAndContactMenuItems uris.size() = " + uris.size());
        while (uris.size() > 0) {
            String uriString = uris.remove(0);
            // Remove any dupes so they don't get added to the menu multiple times
            while (uris.contains(uriString)) {
                uris.remove(uriString);
            }

            int sep = uriString.indexOf(":");
            String prefix = "";
            if (sep >= 0) {
                prefix = uriString.substring(0, sep);
                if ("mailto".equalsIgnoreCase(prefix) || "tel".equalsIgnoreCase(prefix)) {
                    uriString = uriString.substring(sep + 1);
                }
            }
            boolean addToContacts = false;
            if ("mailto".equalsIgnoreCase(prefix)) {
                String sendEmailString = getString(R.string.menu_send_email)
                        .replace("%s", uriString);
                Intent intent = new Intent(Intent.ACTION_VIEW,
                        Uri.parse("mailto:" + uriString));
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
//                menu.add(0, MENU_SEND_EMAIL, 0, sendEmailString).setIntent(intent);
                addToContacts = !MessageUtils.haveEmailContact(uriString, this);
            } else if ("tel".equalsIgnoreCase(prefix)) {
                addToContacts = !isNumberInContacts(uriString);
                MmsLog.d(TAG, "addCallAndContactMenuItems  addToContacts2 = " + addToContacts);
            } else {
                //add URL to book mark
                if (mURLs.size() <= 0) {
                    menu.add(0, MENU_ADD_TO_BOOKMARK, 0, R.string.menu_add_to_bookmark);
                }
                mURLs.add(uriString);
            }
            if (addToContacts) {
                 Intent intent = ConversationList.createAddContactIntent(uriString);
                //Intent intent = new Intent(Intent.ACTION_INSERT, Contacts.CONTENT_URI);
                //intent.putExtra(ContactsContract.Intents.Insert.PHONE, uriString);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                String addContactString = getString(
                        R.string.menu_add_address_to_contacts).replace("%s", uriString);
                menu.add(0, MENU_ADD_ADDRESS_TO_CONTACTS, 0, addContactString)
                    .setIntent(intent);
            }
        }
    }

    private boolean addRecipientToContact(ContextMenu menu, Cursor cursor) {
        boolean showAddContact = false;
        String reciNumber = cursor.getString(cursor.getColumnIndexOrThrow("address"));
        MmsLog.dpi(TAG, "addRecipientToContact reciNumber = " + reciNumber);
        // if there is at least one number not exist in contact db, should show add.
        mContactList = ContactList.getByNumbers(reciNumber, false, true);
        for (Contact contact : mContactList) {
            if (!contact.existsInDatabase()) {
                 showAddContact = true;
                 MmsLog.dpi(TAG, "not in contact[number:" + contact.getNumber()
                         + ",name:" + contact.getName());
                 break;
             }
         }
        boolean menuAddExist = (menu.findItem(MENU_ADD_CONTACT) != null);
        if (showAddContact) {
            if (!menuAddExist) {
                menu.add(0, MENU_ADD_CONTACT, 1, R.string.menu_add_to_contacts)
                        .setIcon(R.drawable.ic_menu_contact);
            }
        } else {
             menu.removeItem(MENU_ADD_CONTACT);
        }
        return true;
    }
    private boolean isNumberInContacts(String phoneNumber) {
        return Contact.get(phoneNumber, true).existsInDatabase();
    }

    private final Handler mMessageListItemHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MessageListItem.ITEM_CLICK:
                break;

            default:
                return;
            }
        }
    };


    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        MmsLog.d(TAG, "onCreateDialog id: " + id);
        if (id == DIALOG_REFRESH) {
            if (mDialog != null) {
                mDialog.dismiss();
                mDialog = null;
                MmsLog.d(TAG, "onCreateDialog mDialog is not null");
            }
            /// M: use QueryProcessDialog  for function: can finish ManageSimMessage activity
            // when querying if user click back key
            mDialog = new QueryProcessDialog(this);
            mDialog.setIndeterminate(true);
            mDialog.setCancelable(false);
            mDialog.setMessage(getString(R.string.refreshing));
            MmsLog.d(TAG, "onCreateDialog return mDialog: " + mDialog);
            return mDialog;
        } else if (id == DIALOG_CAPACITY && args != null) {
            return new AlertDialog.Builder(ManageSimMessages.this).setIconAttribute(
                    android.R.attr.alertDialogIcon).setTitle(R.string.show_icc_sms_capacity_title)
                    .setMessage(args.getString(CAPACITY_KEY, "")).setPositiveButton(
                            android.R.string.ok, null).setCancelable(true).create();
        }

        return null;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog, Bundle args) {
        super.onPrepareDialog(id, dialog);
        MmsLog.d(TAG, "onPrepareDialog id: " + id + " dialog " + dialog);
        if (id == DIALOG_CAPACITY && dialog instanceof AlertDialog && args != null) {
            AlertDialog alertDialog = (AlertDialog) dialog;
            alertDialog.setMessage(args.getString(CAPACITY_KEY, ""));
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            if (mMsgListAdapter != null && mMsgListAdapter.mIsDeleteMode) {
                mMsgListAdapter.mIsDeleteMode = false;
                checkDeleteMode();
                /// M: fix bug ALPS00526071, remove "SimCapacity" menuitem
                invalidateOptionsMenu();
                /// @}
               return true;
            }
        }

        return super.onKeyDown(keyCode, event);
    }

    private void confirmMultiDelete() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.confirm_dialog_title);
        builder.setIconAttribute(android.R.attr.alertDialogIcon);
        builder.setCancelable(true);
        builder.setMessage(R.string.confirm_delete_selected_messages);
        // add for OP
        mOpManageSimMessages.confirmMultiDelete(builder, mMsgListAdapter
                .getSimMsgItemList().entrySet().iterator());
        builder.setPositiveButton(R.string.delete, new DialogInterface.OnClickListener() {
             public void onClick(DialogInterface dialog, int which) {
                 mActionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_TITLE
                         | ActionBar.DISPLAY_HOME_AS_UP,
                         ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_HOME_AS_UP
                         | ActionBar.DISPLAY_SHOW_TITLE);
                 updateState(SHOW_BUSY);
                 new Thread(new Runnable() {
                     public void run() {
                         int count = mMsgListAdapter.getCount();
                         Map<String, Boolean> simMsgList = mMsgListAdapter.getSimMsgItemList();
                         ArrayList<String> selectedSimIds = new ArrayList<String>();
                         for (int position = 0; position < count; position++) {
                             Cursor cursor = (Cursor) mMsgListAdapter.getItem(position);
                             String msgIndex = getMsgIndexByCursor(cursor);
                             String[] index = msgIndex.split(";");
                             if ((simMsgList.get(index[0]) != null) && simMsgList.get(index[0])) {
                                 for (int n = 0; n < index.length; n++) {
                                     selectedSimIds.add(index[n]);
                                 }
                             mDeletedMessageSet.add(cursor.getLong(cursor
                                    .getColumnIndexOrThrow("_id")));
                            }
                         }
                         String[] argsSimMsg = selectedSimIds.toArray(
                                 new String[selectedSimIds.size()]);
                         /// M: Add for OP
                         argsSimMsg = mOpManageSimMessages.onMultiDelete(argsSimMsg);
                         /// @}
                         Log.d(TAG, "confirmMultiDelete startDelete length:" + argsSimMsg.length);
                         mQueryHandler.startDelete(/*DELETE_MESSAGE_TOKEN*/0,
                                 argsSimMsg.length, sSimMessageUri, FOR_MULTIDELETE, argsSimMsg);
                         isDeleting = true;
                     }
                 }).start();
                 mMsgListAdapter.mIsDeleteMode = false;
             }
         });
        builder.setNegativeButton(R.string.no, null);
        builder.show();
     }

    private void checkDeleteMode() {
        if (mMsgListAdapter == null) {
            mActionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_TITLE
                    | ActionBar.DISPLAY_HOME_AS_UP,
                    ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_HOME_AS_UP
                    | ActionBar.DISPLAY_SHOW_TITLE);
            return;
        }
        markCheckedState(false);
        if (mMsgListAdapter.mIsDeleteMode) {
            setUpActionBar();
        } else {
            mActionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_TITLE
                    | ActionBar.DISPLAY_HOME_AS_UP,
                    ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_HOME_AS_UP
                    | ActionBar.DISPLAY_SHOW_TITLE);
        }
        mSimList.invalidateViews();
    }

    private void markCheckedState(boolean checkedState) {
        mMsgListAdapter.setSimItemsValue(checkedState, null);
//        mDeleteButton.setEnabled(checkedState);
        int count = mSimList.getChildCount();
        MessageListItem item = null;
        for (int i = 0; i < count; i++) {
            item = (MessageListItem) mSimList.getChildAt(i);
            if (null != item) {
                item.setSelectedBackGroud(checkedState);
            }
        }
        updateActionBarText();
    }

    public void setTextSize(float size) {
        if (mMsgListAdapter != null) {
            mMsgListAdapter.setTextSize(size);
        }

        if (mSimList != null && mSimList.getVisibility() == View.VISIBLE) {
            int count = mSimList.getChildCount();
            for (int i = 0; i < count; i++) {
                MessageListItem item =  (MessageListItem) mSimList.getChildAt(i);
                if (item != null) {
                    item.setBodyTextSize(size);
                }
            }
        }
    }

    public boolean dispatchTouchEvent(MotionEvent ev) {
        if (!mOpManageSimMessages.dispatchTouchEvent(ev)) {
            return super.dispatchTouchEvent(ev);
        }
        return true;
    }

/*    public void showAirPlaneToast(){
        String airPlaneString = getString(MessageResource.string.sim_close)
         + "," + getString(R.string.delete_unsuccessful);
        Toast.makeText(this, airPlaneString, Toast.LENGTH_SHORT).show();
    }*/

    /// M: update sim state dynamically. @{
    private BroadcastReceiver mSimReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            MmsLog.d(TAG, "onReceive mSimReceiver action: " + action);
            if (action.equals(TelephonyIntents.ACTION_SUBINFO_RECORD_UPDATED)) {
                List<SubscriptionInfo> nowSubList = SubscriptionManager.from(
                        MmsApp.getApplication()).getActiveSubscriptionInfoList();
                int i = 0, subCount = 0;

                if (nowSubList != null) {
                    subCount = nowSubList.size();
                    if (subCount >= 0) {
                        for (i = 0; i < subCount; i++) {
                            int subId = nowSubList.get(i).getSubscriptionId();
                            if (subId == mCurrentSubId)
                                return;
                        }
                    }
                }

                if ((i == subCount) || (nowSubList == null))
                    finish();

            } else if (action.equals(Telephony.Sms.Intents.SIM_FULL_ACTION)) {
                int subId = 0;
                subId = intent
                        .getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, PhoneConstants.SIM_ID_1);

                if (mCurrentSubId == subId) {
                    mIsCurrentSimFull = true;
                }
            }
        }
    };

    /// M: fix for bug ALPS01468873.@{
    /*
    private BroadcastReceiver mSimMessageUpdate = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            MmsLog.d(TAG, "onReceive mSimMessageUpdate action: " + action);
            if (action.equals(ACTION_NOTIFY_SIMMESSAGE_UPDATE)) {
                if(isQuerying) {
                    shouldRequery =true;
                }
                updateState(SHOW_BUSY);
                startQueryIcc();
            }
        }
    };
    */
    /// @}

    /// M: fix bug ALPS00448222, update ListView when contact update
    public void onUpdate(Contact updated) {
        mHandler.post(new Runnable() {
            public void run() {
                // TODO Auto-generated method stub
                if (mMsgListAdapter != null) {
                    mMsgListAdapter.notifyDataSetChanged();
                }
            }
        });
    }

    /// M: redesign select all action. @{

    private Button mActionBarText;
    private MenuItem mSelectionItem;
    private DropDownMenu mSelectionMenu;
    private ActionBar mActionBar;
    private void setUpActionBar() {
        // for ALPS01831024, add DISPLAY_HOME_AS_UP in delete mode.
        mActionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_HOME_AS_UP,
                ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_SHOW_TITLE
                        | ActionBar.DISPLAY_HOME_AS_UP);

        CustomMenu customMenu = new CustomMenu(this);
        View customView = LayoutInflater.from(this).inflate(
                R.layout.multi_select_simsms_actionbar, null);

        /// M: fix bug ALPS00441681, re-layout for landscape
        mActionBar.setCustomView(customView,
                new ActionBar.LayoutParams(ActionBar.LayoutParams.MATCH_PARENT,
                        ActionBar.LayoutParams.MATCH_PARENT,
                        Gravity.FILL));

        mActionBarText = (Button) customView.findViewById(R.id.selection_menu);
        mSelectionMenu = customMenu.addDropDownMenu(mActionBarText, R.menu.selection);
        mSelectionItem = mSelectionMenu.findItem(R.id.action_select_all);

        customMenu.setOnMenuItemClickListener(new OnMenuItemClickListener() {
            public boolean onMenuItemClick(MenuItem item) {
                if ((mMsgListAdapter.getSelectedNumber() > 0)
                        && (mMsgListAdapter.getSelectedNumber() >= mMsgListAdapter.getCount())) {
                    markCheckedState(false);
                } else {
                    markCheckedState(true);
                }
                return false;
            }
        });

        Button cancelSelection = (Button) customView.findViewById(R.id.selection_cancel);
        cancelSelection.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                cancelDeleteMode();
            }
        });

        Button deleteSelection = (Button) customView.findViewById(R.id.selection_done);
        deleteSelection.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                /// M: fix bug ALPS00526071, remove "SimCapacity" menuitem
                invalidateOptionsMenu();
                /// @}
                if (mMsgListAdapter.getSelectedNumber() > 0) {
                    confirmMultiDelete();
                }
            }
        });
        updateActionBarText();
    }

    private void updateActionBarText() {
        if (mMsgListAdapter != null && mActionBarText != null) {
            mActionBarText.setText(getResources().getQuantityString(
                    R.plurals.message_view_selected_message_count,
                    mMsgListAdapter.getSelectedNumber(),
                    mMsgListAdapter.getSelectedNumber()));
        }

        if (mSelectionItem != null && mMsgListAdapter != null) {
            if ((mMsgListAdapter.getSelectedNumber() > 0)
                    && (mMsgListAdapter.getSelectedNumber() >= mMsgListAdapter.getCount())) {
                mSelectionItem.setChecked(true);
                mSelectionItem.setTitle(R.string.unselect_all);
            } else {
                mSelectionItem.setChecked(false);
                mSelectionItem.setTitle(R.string.select_all);
            }
        }
    }

    /// @}

    /**
     *  M: listen this for finishing activity if airplane mode on.
     */
    private final BroadcastReceiver airPlaneReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_AIRPLANE_MODE_CHANGED)) {
                boolean airplaneOn = intent.getBooleanExtra("state", false);
                MmsLog.d(TAG, "onReceive airPlaneReceiver airplaneOn " + airplaneOn);
                if (airplaneOn) {
                    Toast.makeText(getApplicationContext(),
                            getString(MessageResource.string.sim_close),
                            Toast.LENGTH_SHORT).show();
                    ManageSimMessages.this.finish();
                }
            }
        }
    };

    public void updateListAfterDelete() {
        if (mCursor == null || mCursor.isClosed()) {
            return;
        }
        Cursor filertedMessges = buildFilteredCursor(mMsgListAdapter.getCursor());
        mMsgListAdapter = null;
        updateListWithCursor(filertedMessges);
    }

    private Cursor buildFilteredCursor(Cursor messages) {
        MatrixCursor cursor = new MatrixCursor(mMsgListAdapter.getCursor().getColumnNames(), 1);
        messages.moveToPosition(-1);
        while (messages.moveToNext()) {
            Long messageId = messages.getLong(cursor.getColumnIndexOrThrow("_id"));
            if (!mDeletedMessageSet.contains(messageId)) {
                addSMRow(cursor, messages);
            }
        }
        return cursor;
    }

    private void addSMRow(MatrixCursor targetCursor, Cursor sourceCursor) {
        Object[] row = new Object[14];
        row[0] = sourceCursor.getLong(COLUMN_SERVICE_CENTER_ADDRESS);
        row[1] = sourceCursor.getString(COLUMN_ADDRESS);
        row[2] = sourceCursor.getString(COLUMN_MESSAGE_CLASS);
        row[3] = sourceCursor.getString(COLUMN_BODY);
        row[4] = sourceCursor.getLong(COLUMN_DATE);
        row[5] = sourceCursor.getInt(COLUMN_STATUS);
        row[6] = sourceCursor.getString(COLUMN_INDEX_ON_ICC);
        row[7] = Boolean.getBoolean(sourceCursor.getString(COLUMN_IS_STATUS_REPORT));
        row[8] = sourceCursor.getString(COLUMN_TRANSPORT_TYPE);
        row[9] = sourceCursor.getInt(COLUMN_TYPE);
        row[10] = sourceCursor.getInt(COLUMN_LOCKED);
        row[11] = sourceCursor.getInt(COLUMN_ERROR_CODE);
        row[12] = sourceCursor.getInt(COLUMN_ID);
        row[13] = sourceCursor.getInt(COLUMN_SIM_ID);
        targetCursor.addRow(row);
    }

    /**
     * M: Put checking SIM capacity action to background, because sometimes it cost long and
     * caused ANR.
     */
    private class CheckSimCapacityTask extends AsyncTask<Void, Void, String> {
        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            MmsLog.d(TAG, "CheckSimCapacityTask onPreExecute");
            // / M: fix bug ALPS00526071, remove "SimCapacity" menuitem
            invalidateOptionsMenu();
            // / @}
            showDialog(DIALOG_REFRESH);
        }

        @Override
        protected String doInBackground(Void... params) {
            return checkSimCapacity();
        }

        @Override
        protected void onPostExecute(String message) {
            MmsLog.d(TAG, "CheckSimCapacityTask onPostExecute");
            try {
                dismissDialog(DIALOG_REFRESH);
            } catch (IllegalArgumentException e) {
                // dialog is not showing
                MmsLog.e(TAG, "dismiss diaolog DIALOG_REFRESH exception: " +  e);
            }
            Bundle capacityBundle = new Bundle();
            capacityBundle.putString(CAPACITY_KEY, message);
            showDialog(DIALOG_CAPACITY, capacityBundle);
        }
    }

    public String checkSimCapacity() {
        String message = null;
        MtkIccSmsStorageStatus simMemStatus = null;

        MmsLog.d(TAG, "checkSimCapacity mCurrentSubId = " + mCurrentSubId);
        simMemStatus = MtkSmsManager.getSmsManagerForSubscriptionId(mCurrentSubId)
                .getSmsSimMemoryStatus();
        MmsLog.dpi(TAG, "checkSimCapacity simMemStatus = " + simMemStatus);

        if (null != simMemStatus) {
            message = getString(R.string.icc_sms_used) + " "
                    + Integer.toString(simMemStatus.getUsedCount())
                    + "\n" + getString(R.string.icc_sms_total) + " "
                    + Integer.toString(simMemStatus.getTotalCount());
        } else {
            message = getString(R.string.get_icc_sms_capacity_failed);
        }
        message = mOpManageSimMessages.checkSimCapacity(simMemStatus, message);
        return message;
    }

    /**
     * M: For function: can finish ManageSimMessage activity when querying if user
     * click back key.
     */
    public class QueryProcessDialog extends ProgressDialog {
        public QueryProcessDialog(Context context) {
            super(context);
        }

        @Override
        public boolean dispatchKeyEvent(KeyEvent event) {
            switch (event.getKeyCode()) {
                case KeyEvent.KEYCODE_BACK:
                    if (event.getAction() == KeyEvent.ACTION_DOWN && isQuerying && isShowing()) {
                        Log.d(TAG, "QueryProcessDialog received KEYCODE_BACK so finish activity");
                        mQueryIccTask.cancel(true);
                        ManageSimMessages.this.finish();
                        return true;
                    }
            }
            return super.dispatchKeyEvent(event);
        }
    }

    private void cancelDeleteMode() {
        /// M: fix bug ALPS00526071, remove "SimCapacity" menuitem
        invalidateOptionsMenu();
        /// @}
        mMsgListAdapter.mIsDeleteMode = false;
        checkDeleteMode();
        mActionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_TITLE | ActionBar.DISPLAY_HOME_AS_UP,
                ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_HOME_AS_UP
                | ActionBar.DISPLAY_SHOW_TITLE);
    }
}
