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

package com.android.mms.ui;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.app.SearchManager;
import android.app.SearchableInfo;
import android.app.StatusBarManager;
import android.content.ActivityNotFoundException;
import android.content.AsyncQueryHandler;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SqliteWrapper;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Handler;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.os.UserManager;
import android.preference.PreferenceManager;
import android.provider.ContactsContract;
import android.provider.ContactsContract.Contacts;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Conversations;
import android.provider.Telephony.Threads;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.ActionMode;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.View.OnKeyListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.SearchView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.android.mms.LogTag;
import com.android.mms.MmsApp;
import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.Contact;
import com.android.mms.data.ContactList;
import com.android.mms.data.Conversation;
import com.android.mms.draft.DraftManager;
import com.android.mms.transaction.MessagingNotification;
import com.android.mms.transaction.SmsReceiver;
import com.android.mms.transaction.SmsRejectedReceiver;
import com.android.mms.ui.CustomMenu.DropDownMenu;
import com.android.mms.util.FeatureOption;
import com.android.mms.util.MmsLog;
import com.android.mms.util.Recycler;
import com.android.mms.widget.MmsWidgetProvider;
import com.mediatek.android.mms.pdu.MtkPduHeaders;
import com.mediatek.cb.cbmsg.CbMessageListActivity;
import com.mediatek.cb.cbmsg.CbMessagingNotification;
import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.callback.IConversationListCallback;
import com.mediatek.mms.callback.IConversationListHost;
import com.mediatek.mms.ext.IOpConversationListExt;
import com.mediatek.mms.ipmessage.IIpConversationListExt;
import com.mediatek.mms.util.PermissionCheckUtil;
import com.mediatek.mwi.MwiListActivity;
import com.mediatek.opmsg.util.OpMessageUtils;
import com.mediatek.setting.SettingListActivity;
import com.mediatek.setting.SmsPreferenceActivity;
import com.mediatek.setting.SubSelectActivity;
import com.mediatek.simmessage.ManageSimMessages;
import com.mediatek.wappush.WapPushMessagingNotification;
import com.mediatek.wappush.ui.WPMessageActivity;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;

import mediatek.telephony.MtkSmsManager;
import mediatek.telephony.MtkTelephony;
/* Mem opt*/
import com.android.mms.util.ThumbnailManager;
import com.android.mms.util.PduLoaderManager;

/**
 * This activity provides a list view of existing conversations.
 */
public class ConversationList extends ListActivity implements
        /// M:add interface
        IConversationListHost {
    private static final String TAG = "ConversationList";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = DEBUG;

    private static final int THREAD_LIST_QUERY_TOKEN       = 1701;
    private static final int UNREAD_THREADS_QUERY_TOKEN    = 1702;

    public static final int DELETE_CONVERSATION_TOKEN      = 1801;
    public static final int HAVE_LOCKED_MESSAGES_TOKEN     = 1802;
    public static final int DELETE_OBSOLETE_THREADS_TOKEN  = 1803;

    // IDs of the context menu items for the list of conversations.
    public static final int MENU_DELETE               = 0;
    public static final int MENU_VIEW                 = 1;
    public static final int MENU_VIEW_CONTACT         = 2;
    public static final int MENU_ADD_TO_CONTACTS      = 3;

    private ThreadListQueryHandler mQueryHandler;
    private ConversationListAdapter mListAdapter;
    private SharedPreferences mPrefs;
    private Handler mHandler;
    private boolean mNeedToMarkAsSeen;
    private TextView mUnreadConvCount;
    private MenuItem mSearchItem;
    private SearchView mSearchView;
    private View mSmsPromoBannerView;
    private int mSavedFirstVisiblePosition = AdapterView.INVALID_POSITION;
    private int mSavedFirstItemOffset;

    // keys for extras and icicles
    private final static String LAST_LIST_POS = "last_list_pos";
    private final static String LAST_LIST_OFFSET = "last_list_offset";

    private static final String CHECKED_MESSAGE_LIMITS = "checked_message_limits";

    // Whether or not we are currently enabled for SMS. This field is updated in onResume to make
    // sure we notice if the user has changed the default SMS app.
    private boolean mIsSmsEnabled;

    /// M: new members
    private static final String CONV_TAG = "Mms/convList";
    /// M: Code analyze 002, For new feature ALPS00041233, msim enhancment check in . @{
    private StatusBarManager mStatusBarManager;
    /// @}
    /// M: Code analyze 001, For new feature ALPS00131956, wappush: add new params . @{
    private int mType;
    private static final String WP_TAG = "Mms/WapPush";
    /// @}
    /// M: Code analyze 004, For bug ALPS00247476, ensure the scroll smooth . @{
    private static final int CHANGE_SCROLL_LISTENER_MIN_CURSOR_COUNT = 100;
    /// @}
    /// M: Code analyze 004, For bug ALPS00247476, ensure the scroll smooth . @{
    private MyScrollListener mScrollListener =
                    new MyScrollListener(CHANGE_SCROLL_LISTENER_MIN_CURSOR_COUNT,
                            "ConversationList_Scroll_Tread");
    /// @}

    /// M: Code analyze 008, For bug ALPS00250948, disable search in multi-select status . @{
    private boolean mDisableSearchFalg = false;
    /// M: Code analyze 005, For new feature ALPS00247476, add selectAll/unSelectAll . @{
    private ModeCallback mActionModeListener = new ModeCallback();
    private ActionMode mActionMode;
    /// @}
    /// M: Optimize select all performance, save actionmode status and reduce select time. @{
    private static String ACTIONMODE = "actionMode";
    private static String NEED_RESTORE_ADAPTER_STATE = "needRestore";
    private boolean mIsNeedRestoreAdapterState = false;
    private static String SELECT_THREAD_IDS = "selectThreadIds";
    private long[] mListSelectedThreads;
    /// @}

    /// M: Code analyze 009, For bug ALPS00270910, Default SIM card icon shown in status bar
    /// is incorrect, need to get current sim information . @{
    private static Activity sActivity = null;
    /// @}

    /// M: Code analyze 009, For new feature, plugin . @{
    private IOpConversationListExt mOpConversationList = null;
    /// @}

    /// M: add for ipmessage
    /// M: add for display unread thread count
    private static final int MAX_DISPLAY_UNREAD_COUNT = 99;
    private static final String DISPLAY_UNREAD_COUNT_CONTENT_FOR_ABOVE_99 = "99+";

    private ListView mListView; // we need this to update empty view.
    private View mEmptyViewDefault;
//    private ConversationEmptyView mEmptyView;
    private LinearLayout mIpEmptyView;

    private Context mContext = null;

    private LinearLayout mNetworkStatusBar;

    /// M: Remove cursor leak @{
    private boolean mNeedQuery = false;
    private boolean mIsInActivity = false; //If activity is not displayed, no need do query
    /// @}

    private boolean mIsFirstCreate;
    private AlertDialog mDeleteAlertDialog;

    private static boolean sIsDeleting = false;

    //add for ipmessage
    public IIpConversationListExt mIpConvList;
    public ConversationListCallback mIpConvListCallback = new ConversationListCallback();
    // add for mutli user
    private boolean isUserHasPerUsingMms = true;

    // when launch ConversationList from Launcher and app list, it will has different taskId,
    // so more than one ConversationList will put into the ActivityTask, AMS performance poor,
    // add a static intance to avoid this.
    private static Activity sInstance = null;

    private boolean mReceiverTag = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }
        if (sInstance != null) {
            MmsLog.d(TAG, "onCreate, sInstance!=this, sInstance:" + sInstance + ", this:" + this);
            this.finish();
            return;
        }
        sInstance = this;
        MmsLog.d(TAG, "onCreate, sInstance:" + sInstance);
        /// M: Code analyze 009, For bug ALPS00270910, Default SIM card icon shown in status
        /// bar is incorrect, need to get current sim information . @{
        sActivity = ConversationList.this;
        /// @}
        initPlugin(this);
        mOpConversationList.onCreate(this, savedInstanceState);

        setContentView(R.layout.conversation_list_screen);
        mSmsPromoBannerView = findViewById(R.id.banner_sms_promo);
        /// M: Code analyze 002, For new feature ALPS00041233, msim enhancment check in . @{
        mStatusBarManager = (StatusBarManager) getSystemService(Context.STATUS_BAR_SERVICE);
        /// @}
        mQueryHandler = new ThreadListQueryHandler(getContentResolver());

        mListView = getListView();
        mListView.setOnCreateContextMenuListener(mConvListOnCreateContextMenuListener);
        mListView.setOnKeyListener(mThreadListKeyListener);
        /// M: Code analyze 005, For new feature ALPS00247476, add selectAll/unSelectAll . @{
        mListView.setOnScrollListener(mScrollListener);
        mScrollListener.setThreadId(-1, ConversationList.this);
        /// @}

        // Tell the list view which view to display when the list is empty
        mEmptyViewDefault = findViewById(R.id.empty);
        mIpEmptyView = (LinearLayout) findViewById(R.id.ipmsg_empty_view);
//        mEmptyView = (ConversationEmptyView) findViewById(R.id.empty2);
        mNetworkStatusBar = (LinearLayout) findViewById(R.id.no_itnernet_view);
        TextView networkStatusTextView =
            ((TextView) mNetworkStatusBar.findViewById(R.id.no_internet_text));

        mListView.setOnItemLongClickListener(new ItemLongClickListener());
        /// M: Optimize select all performance, restore Actionmode status. @{
        if (savedInstanceState != null) {
            mIsNeedRestoreAdapterState =
                savedInstanceState.getBoolean(NEED_RESTORE_ADAPTER_STATE, false);
        } else {
            mIsNeedRestoreAdapterState = false;
        }
        /// @}
        initListAdapter();

        mContext = ConversationList.this;

        mHandler = new Handler();
        mPrefs = PreferenceManager.getDefaultSharedPreferences(this);
        boolean checkedMessageLimits = mPrefs.getBoolean(CHECKED_MESSAGE_LIMITS, false);
        if (DEBUG) {
            Log.v(TAG, "checkedMessageLimits: " + checkedMessageLimits);
        }
        if (!checkedMessageLimits) {
            runOneTimeStorageLimitCheckForLegacyMessages();
        }

        if (savedInstanceState != null) {
            mSavedFirstVisiblePosition = savedInstanceState.getInt(LAST_LIST_POS,
                    AdapterView.INVALID_POSITION);
            mSavedFirstItemOffset = savedInstanceState.getInt(LAST_LIST_OFFSET, 0);
        } else {
            mSavedFirstVisiblePosition = AdapterView.INVALID_POSITION;
            mSavedFirstItemOffset = 0;
        }

        mIsFirstCreate = true;
        mIpConvList = IpMessageUtils.getIpMessagePlugin(this).getIpConversationList();
        mIpConvList.onIpConversationListCreate(this, mIpConvListCallback,
                mListView, mIpEmptyView, mNetworkStatusBar, networkStatusTextView);
    }

    private void setupActionBar() {
        ActionBar actionBar = getActionBar();
        if (mUnreadConvCount == null) {
            ViewGroup v = (ViewGroup) LayoutInflater.from(this)
                .inflate(R.layout.conversation_list_actionbar, null);
            actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                    ActionBar.DISPLAY_SHOW_CUSTOM);
            actionBar.setCustomView(v,
                    new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                            ActionBar.LayoutParams.WRAP_CONTENT,
                            Gravity.CENTER_VERTICAL | Gravity.RIGHT));

            mUnreadConvCount = (TextView) v.findViewById(R.id.unread_conv_count);
        }
    }

    private final ConversationListAdapter.OnContentChangedListener mContentChangedListener =
        new ConversationListAdapter.OnContentChangedListener() {

        @Override
        public void onContentChanged(ConversationListAdapter adapter) {
            /// M: Remove cursor leak and reduce needless query @{
            /* Only need when activity is shown*/
            MmsLog.d(TAG, "onContentChanged begin");
            if (mIsInActivity) {
                mNeedQuery = true;
                startAsyncQuery();
            }
            /// @}
        }
    };

    private void initListAdapter() {
        mListAdapter = new ConversationListAdapter(this, null);
        /** M: now this code is useless and will lead to a JE, comment it.
         *  listener is set in onStart
         */
        //mListAdapter.setOnContentChangedListener(mContentChangedListener);
        setListAdapter(mListAdapter);
        getListView().setRecyclerListener(mListAdapter);
    }

    private void initSmsPromoBanner() {
        /// M: add for Mutli-user, show 'user is not allowed to use SMS' alert if user has no
        // permission to use SMS. @{
        ImageView defaultSmsAppIconImageView =
            (ImageView) mSmsPromoBannerView.findViewById(R.id.banner_sms_default_app_icon);
        TextView permissionAlertView = (TextView) mSmsPromoBannerView
                .findViewById(R.id.sms_permission_alert);
        LinearLayout disabledAlertView = (LinearLayout) mSmsPromoBannerView
                .findViewById(R.id.sms_disabled_alert);
        if (!isUserHasPerUsingMms) {
            mSmsPromoBannerView.setClickable(false);
            permissionAlertView.setVisibility(View.VISIBLE);
            disabledAlertView.setVisibility(View.GONE);
            defaultSmsAppIconImageView.setImageDrawable(getResources().getDrawable(
                    R.drawable.ic_launcher_smsmms));
            return;
        } else {
            mSmsPromoBannerView.setClickable(true);
            permissionAlertView.setVisibility(View.GONE);
            disabledAlertView.setVisibility(View.VISIBLE);
        }
        /// @}
        final PackageManager packageManager = getPackageManager();
        final String smsAppPackage = Telephony.Sms.getDefaultSmsPackage(this);

        // Get all the data we need about the default app to properly render the promo banner. We
        // try to show the icon and name of the user's selected SMS app and have the banner link
        // to that app. If we can't read that information for any reason we leave the fallback
        // text that links to Messaging settings where the user can change the default.
        Drawable smsAppIcon = null;
        ApplicationInfo smsAppInfo = null;
        try {
            smsAppIcon = packageManager.getApplicationIcon(smsAppPackage);
            smsAppInfo = packageManager.getApplicationInfo(smsAppPackage, 0);
        } catch (NameNotFoundException e) {
        }
        final Intent smsAppIntent = packageManager.getLaunchIntentForPackage(smsAppPackage);

        // If we got all the info we needed
        if (smsAppIcon != null && smsAppInfo != null && smsAppIntent != null) {
            defaultSmsAppIconImageView.setImageDrawable(smsAppIcon);
            TextView smsPromoBannerTitle =
                    (TextView) mSmsPromoBannerView.findViewById(R.id.banner_sms_promo_title);
            String message = getResources().getString(R.string.banner_sms_promo_title_application,
                    smsAppInfo.loadLabel(packageManager));
            smsPromoBannerTitle.setText(message);
            if (mOpConversationList != null) {
                mOpConversationList.initSmsPromoBanner(defaultSmsAppIconImageView, smsPromoBannerTitle,
                    (TextView) mSmsPromoBannerView.findViewById(R.id.banner_sms_promo_message),
                    smsAppInfo, packageManager);
            }
            mSmsPromoBannerView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    Intent intent  = mOpConversationList.onSmsPromoBannerViewClick(smsAppIntent);
                    startActivity(intent);
                }
            });
        } else {
            // Otherwise the banner will be left alone and will launch settings
            mSmsPromoBannerView.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    // Launch settings
                    if (!mOpConversationList.onClickSmsPromoBanner()) {
                        Intent settingIntent = new Intent(ConversationList.this,
                                SettingListActivity.class);
                        startActivityIfNeeded(settingIntent, -1);
                    }
                }
            });
        }
    }

    /**
     * Checks to see if the number of MMS and SMS messages are under the limits for the
     * recycler. If so, it will automatically turn on the recycler setting. If not, it
     * will prompt the user with a message and point them to the setting to manually
     * turn on the recycler.
     */
    public synchronized void runOneTimeStorageLimitCheckForLegacyMessages() {
        if (Recycler.isAutoDeleteEnabled(this)) {
            if (DEBUG) {
                Log.v(TAG, "recycler is already turned on");
            }
            // The recycler is already turned on. We don't need to check anything or warn
            // the user, just remember that we've made the check.
            markCheckedMessageLimit();
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                if (Recycler.checkForThreadsOverLimit(ConversationList.this)) {
                    if (DEBUG) {
                        Log.v(TAG, "checkForThreadsOverLimit TRUE");
                    }
                    // Dang, one or more of the threads are over the limit. Show an activity
                    // that'll encourage the user to manually turn on the setting. Delay showing
                    // this activity until a couple of seconds after the conversation list appears.
                    mHandler.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            Intent intent = new Intent(ConversationList.this,
                                    WarnOfStorageLimitsActivity.class);
                            startActivity(intent);
                        }
                    }, 2000);
                /** M: comment this else block
                } else {
                    if (DEBUG) Log.v(TAG, "checkForThreadsOverLimit silently turning on recycler");
                    // No threads were over the limit. Turn on the recycler by default.
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            SharedPreferences.Editor editor = mPrefs.edit();
                            editor.putBoolean(GeneralPreferenceActivity.AUTO_DELETE, true);
                            editor.apply();
                        }
                    });
                */
                }
                // Remember that we don't have to do the check anymore when starting MMS.
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        markCheckedMessageLimit();
                    }
                });
            }
        }, "ConversationList.runOneTimeStorageLimitCheckForLegacyMessages").start();
    }

    /**
     * Mark in preferences that we've checked the user's message limits. Once checked, we'll
     * never check them again, unless the user wipe-data or resets the device.
     */
    private void markCheckedMessageLimit() {
        if (DEBUG) {
            Log.v(TAG, "markCheckedMessageLimit");
        }
        SharedPreferences.Editor editor = mPrefs.edit();
        editor.putBoolean(CHECKED_MESSAGE_LIMITS, true);
        editor.apply();
    }

 /*   @Override
    protected void onNewIntent(Intent intent) {
        if (PermissionCheckUtil.requestRequiredPermissions(this)) {
            return;
        }

        // Handle intents that occur after the activity has already been created.
        MessagingNotification.cancelNotification(getApplicationContext(),
                SmsRejectedReceiver.SMS_REJECTED_NOTIFICATION_ID);
        startAsyncQuery();
    }
*/
    @Override
    protected void onStart() {
        super.onStart();
        if (sInstance != this) {
            MmsLog.d(TAG, "onStart, sInstance!=this, sInstance:" + sInstance + ", this:" + this);
            return;
        }
        // add for multi user
        isUserHasPerUsingMms = !UserManager.get(getApplicationContext()).hasUserRestriction(
                UserManager.DISALLOW_SMS);

        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return;
        }

        if ((mIpConvList != null) && !mIpConvList.onIpNeedLoadView(mEmptyViewDefault)) {
            if (mIsFirstCreate) {
              setupActionBar();
              setTitle(R.string.app_label);
              mIpEmptyView.setVisibility(View.GONE);
              mEmptyViewDefault.setVisibility(View.VISIBLE);
              mListView.setEmptyView(mEmptyViewDefault);
            }
        }

        startAsyncQuery();
        MessagingNotification.cancelNotification(getApplicationContext(),
                SmsRejectedReceiver.SMS_REJECTED_NOTIFICATION_ID);

        mNeedToMarkAsSeen = true;

        /// M: setOnContentChangedListener here, it will be removed in onStop @{
        mIsInActivity = true;
        if (mListAdapter != null) {
            MmsLog.d(TAG, "set onContentChanged listener");
            mListAdapter.setOnContentChangedListener(mContentChangedListener);
        }
        /// @}
        // We used to refresh the DraftCache here, but
        // refreshing the DraftCache each time we go to the ConversationList seems overly
        // aggressive. We already update the DraftCache when leaving CMA in onStop() and
        // onNewIntent(), and when we delete threads or delete all in CMA or this activity.
        // I hope we don't have to do such a heavy operation each time we enter here.
        /// M: Code analyze 0014, For new feature, third party may add/delete
        /// draft, and we must refresh to check this.
        /// M: to resolve ALPS00812509, do not refresh in onStart() to avoid frequently
        /// db operation while switch launcher and ConversationList.
        //DraftCache.getInstance().refresh();
        /// @}

        // we invalidate the contact cache here because we want to get updated presence
        // and any contact changes. We don't invalidate the cache by observing presence and contact
        // changes (since that's too untargeted), so as a tradeoff we do it here.
        // If we're in the middle of the app initialization where we're loading the conversation
        // threads, don't invalidate the cache because we're in the process of building it.
        // TODO: think of a better way to invalidate cache more surgically or based on actual
        // TODO: changes we care about
        if (!Conversation.loadingThreads()) {
            Contact.invalidateCache();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();

        /// M: @{
        mIsInActivity = false;
        /// @}

        if (mQueryHandler != null) {
            mQueryHandler.cancelOperation(THREAD_LIST_QUERY_TOKEN);
            mQueryHandler.cancelOperation(UNREAD_THREADS_QUERY_TOKEN);
            mNeedQuery = false;
        }
        /// @}
        if (mListAdapter != null) {
            mListAdapter.setOnContentChangedListener(null);
        }
        // Simply setting the choice mode causes the previous choice mode to finish and we exit
        // multi-select mode (if we're in it) and remove all the selections.
        /// M:comment a line
        //getListView().setChoiceMode(ListView.CHOICE_MODE_MULTIPLE_MODAL);

        /// M: comment this line
        //mListAdapter.changeCursor(null);
    }

    private void startAsyncQuery() {
        try {
            mNeedQuery = false;
            if ((mIpConvList != null) && !mIpConvList.onIpStartAsyncQuery()) {
                /// M: fix bug ALPS00941735, except Obsolete ThreadId when query
                ((TextView) (mEmptyViewDefault)).setText(R.string.loading_conversations);
                if (DraftManager.sEditingThread.isEmpty()) {
                    MmsLog.d(TAG, "DraftManager.sEditingThread = Empty");
                    Conversation.startQueryForAll(mQueryHandler, THREAD_LIST_QUERY_TOKEN);
                } else {
                    long exceptID = getExceptId();
                    MmsLog.d(TAG, "DraftManager except ThreadId = " + exceptID);
                    Conversation.startQuery(mQueryHandler,
                            THREAD_LIST_QUERY_TOKEN, "threads._id<>" + exceptID);
                }
                Conversation.startQuery(mQueryHandler, UNREAD_THREADS_QUERY_TOKEN, "allunread");
            }
        } catch (SQLiteException e) {
            SqliteWrapper.checkSQLiteException(this, e);
        }
    }

    SearchView.OnQueryTextListener mQueryTextListener = new SearchView.OnQueryTextListener() {
        @Override
        public boolean onQueryTextSubmit(String query) {
            Intent intent = new Intent();
            intent.setClass(ConversationList.this, SearchActivity.class);
            intent.putExtra(SearchManager.QUERY, query);
            startActivity(intent);
            mSearchItem.collapseActionView();
            return true;
        }

        @Override
        public boolean onQueryTextChange(String newText) {
            return false;
        }
    };

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            return false;
        }
        if (sInstance != this) {
            MmsLog.d(TAG, "onCreateOptionsMenu, sInstance:" + sInstance + ", this:" + this);
            return false;
        }
        /// M: for ALPS01861847, add for mutli user @{
        if (!isUserHasPerUsingMms) {
            MmsLog.d(TAG, "onCreateOptionsMenu user has no permission");
            return false;
        }
        /// @}
        getMenuInflater().inflate(R.menu.conversation_list_menu, menu);

        mSearchItem = menu.findItem(R.id.search);
        mSearchView = (SearchView) mSearchItem.getActionView();
        if (mOpConversationList != null) {
            mOpConversationList.onCreateOptionsMenu(menu);
        }
        /// @}
        mSearchView.setOnQueryTextListener(mQueryTextListener);
        mSearchView.setQueryHint(getString(R.string.search_hint));
        mSearchView.setIconifiedByDefault(true);
        SearchManager searchManager = (SearchManager) getSystemService(Context.SEARCH_SERVICE);

        if (searchManager != null) {
            SearchableInfo info = searchManager.getSearchableInfo(this.getComponentName());
            mSearchView.setSearchableInfo(info);
        }

        MenuItem cellBroadcastItem = menu.findItem(R.id.action_cell_broadcasts);
        if (cellBroadcastItem != null) {
            // Enable link to Cell broadcast activity depending on the value in config.xml.
            boolean isCellBroadcastAppLinkEnabled = this.getResources().getBoolean(
                    com.android.internal.R.bool.config_cellBroadcastAppLinks);
            try {
                if (isCellBroadcastAppLinkEnabled) {
                    PackageManager pm = getPackageManager();
                    if (pm.getApplicationEnabledSetting("com.android.cellbroadcastreceiver")
                            == PackageManager.COMPONENT_ENABLED_STATE_DISABLED) {
                        isCellBroadcastAppLinkEnabled = false;  // CMAS app disabled
                    }
                }
            } catch (IllegalArgumentException ignored) {
                isCellBroadcastAppLinkEnabled = false;  // CMAS app not installed
            }
            if (!isCellBroadcastAppLinkEnabled) {
                cellBroadcastItem.setVisible(false);
            }
        }
        if (mIpConvList != null) {
            mIpConvList.onIpCreateOptionsMenu(menu);
        }
        /// M: for mwi. @{
        if (FeatureOption.MTK_MWI_SUPPORT) {
            MenuItem mwiItem = menu.findItem(R.id.action_mwi);
            mwiItem.setVisible(true);
        }
        /// @}
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        /// M: for ALPS01861847, add for mutli user @{
        if (!isUserHasPerUsingMms) {
            MmsLog.d(TAG, "onPrepareOptionsMenu user has no permission");
            return false;
        }
        /// @}
        if (mOpConversationList != null) {
            mOpConversationList.onPrepareOptionsMenu(menu);
        }
        mOptionsMenu = menu ;
        setDeleteMenuVisible(menu);
        MenuItem item;
        item = menu.findItem(R.id.action_compose_new);
        if (item != null) {
            // Dim compose if SMS is disabled because it will not work (will show a toast)
            item.getIcon().setAlpha(MmsConfig.isSmsEnabled(this) ? 255 : 127);
        }
        if (!LogTag.DEBUG_DUMP) {
            item = menu.findItem(R.id.action_debug_dump);
            if (item != null) {
                item.setVisible(false);
            }
        }

        /// M: Code analyze 011, add code for omacp . @{
        item = menu.findItem(R.id.action_omacp);
        item.setVisible(false);
        Context otherAppContext = null;
        try {
            otherAppContext = this.createPackageContext("com.mediatek.omacp",
                    Context.CONTEXT_IGNORE_SECURITY);
        } catch (Exception e) {
            MmsLog.e(CONV_TAG, "ConversationList NotFoundContext");
        }
        if (null != otherAppContext) {
            item.setVisible(true);
        }
        /// @}
        if (mIpConvList != null) {
            mIpConvList.onIpPrepareOptionsMenu(menu);
        }
        item = menu.findItem(R.id.action_wappush);
        item.setVisible(true);

        //add for multi user feature
        if (UserHandle.myUserId() != UserHandle.USER_OWNER) {
            MenuItem itemSetting = menu.findItem(R.id.action_settings);
            MenuItem itemSimInfo = menu.findItem(R.id.action_siminfo);
            itemSetting.setVisible(false);
            itemSimInfo.setVisible(false);
        }

        return true;
    }

    @Override
    public boolean onSearchRequested() {
        if (mSearchItem != null) {
            mSearchItem.expandActionView();
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        /// M: Code analyze 009, For new feature, plugin . @{
        if ((mOpConversationList != null) && mOpConversationList.onOptionsItemSelected(item, R.id.action_settings)) {
            return true;
        }
        /// @}

        if ((mIpConvList != null) && mIpConvList.onIpOptionsItemSelected(item, mIsSmsEnabled)) {
            return true;
        }

        switch(item.getItemId()) {
            case R.id.action_compose_new:
                if (mIsSmsEnabled) {
                    createNewMessage();
                } else {
                    Toast.makeText(this, R.string.compose_disabled_toast, Toast.LENGTH_SHORT).show();
                }
                break;
            case R.id.action_delete_all:
                    // The invalid threadId of -1 means all threads here.
                    confirmDeleteThread(-1L, mQueryHandler);
                break;
            case R.id.action_settings:
                Intent settingIntent = new Intent(this, SettingListActivity.class);
                startActivityIfNeeded(settingIntent, -1);
                break;
            /// M: Code analyze 011, add omacp to option menu . @{
            case R.id.action_omacp:
                Intent omacpintent = new Intent();
                omacpintent.setClassName("com.mediatek.omacp",
                        "com.mediatek.omacp.message.OmacpMessageList");
                omacpintent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivityIfNeeded(omacpintent, -1);
                break;
            /// @}
            case R.id.action_wappush:
                Intent wpIntent = new Intent(this, WPMessageActivity.class);
                startActivity(wpIntent);
                break;
            case R.id.action_debug_dump:
                LogTag.dumpInternalTables(this);
                break;
            case R.id.action_cell_broadcasts:
                Intent cellBroadcastIntent = new Intent(Intent.ACTION_MAIN);
                cellBroadcastIntent.setComponent(new ComponentName(
                        "com.android.cellbroadcastreceiver",
                        "com.android.cellbroadcastreceiver.CellBroadcastListActivity"));
                cellBroadcastIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                try {
                    startActivity(cellBroadcastIntent);
                } catch (ActivityNotFoundException ignored) {
                    Log.e(TAG, "ActivityNotFoundException for CellBroadcastListActivity");
                }
                return true;
            case R.id.action_mwi:
                Intent mwiIntent = new Intent(this, MwiListActivity.class);
                startActivity(mwiIntent);
                break;
            default:
                return true;
        }
        return false;
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        // Note: don't read the thread id data from the ConversationListItem view passed in.
        // It's unreliable to read the cached data stored in the view because the ListItem
        // can be recycled, and the same view could be assigned to a different position
        // if you click the list item fast enough. Instead, get the cursor at the position
        // clicked and load the data from the cursor.
        // (ConversationListAdapter extends CursorAdapter, so getItemAtPosition() should
        // return the cursor object, which is moved to the position passed in)
        Cursor cursor  = (Cursor) getListView().getItemAtPosition(position);
        /// M: Code analyze 015, For bug,  add cursor == null check . @{
        if (cursor == null) {
            return;
        }
        /// @}
        Conversation conv = Conversation.from(this, cursor);
        long threadId = conv.getThreadId();
        /// M: Code analyze 005, For new feature ALPS00247476,
        /// handle click item with ActionMode . @{
        if (mActionMode != null) {
            boolean checked = mListAdapter.isContainThreadId(threadId);
            mActionModeListener.setItemChecked(position, !checked, null);
            mActionModeListener.updateActionMode();
            if (mListAdapter != null) {
                mListAdapter.notifyDataSetChanged();
            }
            return;
        }
        /// @}

        if (LogTag.VERBOSE) {
            Log.d(TAG, "onListItemClick: pos=" + position
                    + ", view=" + v + ", threadId=" + threadId);
        }

        /// M: Fix ipmessage bug
        viewThread(conv, conv.getType()) ;
    }

    /// M: Fix ipmessage bug @{
    private void viewThread(Conversation conv, int type) {

        long threadId = conv.getThreadId();

        /// M: Code analyze 001, For new feature ALPS00131956, wappush: modify
        /// the calling of openThread, add one parameter. @{
        if (LogTag.VERBOSE) {
            MmsLog.i(WP_TAG, "ConversationList: " + "conv.getType() is : " + conv.getType());
        }

        ContactList list = conv.getRecipients();
        if (list != null && list.size() == 1) {
            if ((mIpConvList != null) && mIpConvList.onIpOpenThread(list.get(0).getNumber(), threadId)) {
                conv.markAsSeen();
                return;
            }
        }
        openThread(threadId, type);
        /// @}
    }
    /// @}

    private void createNewMessage() {
        if ((mIpConvList != null) && mIpConvList.onIpCreateNewMessage()) {
            return;
        }
        startActivity(ComposeMessageActivity.createIntent(this, 0));
    }

    /// M: Code analyze 001, For new feature ALPS00131956, the method is extended. @{
    private void openThread(long threadId, int type) {
        switch (type) {
        case MtkTelephony.MtkThreads.WAPPUSH_THREAD:
                startActivity(WPMessageActivity.createIntent(this, threadId));
            break;
        case MtkTelephony.MtkThreads.CELL_BROADCAST_THREAD:
                startActivity(CbMessageListActivity.createIntent(this, threadId));
            break;
        default:
                startActivity(ComposeMessageActivity.createIntent(this, threadId));
            break;
        }
    }
    /// @}

    public static Intent createAddContactIntent(String address) {
        // address must be a single recipient
        Intent intent = new Intent(Intent.ACTION_INSERT_OR_EDIT);
        intent.setType(Contacts.CONTENT_ITEM_TYPE);
        if (Mms.isEmailAddress(address)) {
            intent.putExtra(ContactsContract.Intents.Insert.EMAIL, address);
        } else {
            intent.putExtra(ContactsContract.Intents.Insert.PHONE, address);
            intent.putExtra(ContactsContract.Intents.Insert.PHONE_TYPE,
                    ContactsContract.CommonDataKinds.Phone.TYPE_MOBILE);
        }
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);

        return intent;
    }

    private final OnCreateContextMenuListener mConvListOnCreateContextMenuListener =
        new OnCreateContextMenuListener() {
        @Override
        public void onCreateContextMenu(ContextMenu menu, View v,
                ContextMenuInfo menuInfo) {
            Cursor cursor = mListAdapter.getCursor();
            if (cursor == null || cursor.getPosition() < 0) {
                return;
            }
            Conversation conv = Conversation.from(ConversationList.this, cursor);
            /// M: Code analyze 001, For new feature ALPS00131956, wappush: get
            /// the added mType value. @{
            mType = conv.getType();
            MmsLog.i(WP_TAG, "ConversationList: " + "mType is : " + mType);
            /// @}

            ContactList recipients = conv.getRecipients();
            menu.setHeaderTitle(recipients.formatNames(","));

            AdapterView.AdapterContextMenuInfo info =
                (AdapterView.AdapterContextMenuInfo) menuInfo;
            menu.add(0, MENU_VIEW, 0, R.string.menu_view);

            // Only show if there's a single recipient
            if (recipients.size() == 1) {
                // do we have this recipient in contacts?
                if (recipients.get(0).existsInDatabase()) {
                    menu.add(0, MENU_VIEW_CONTACT, 0, R.string.menu_view_contact);
                } else {
                    menu.add(0, MENU_ADD_TO_CONTACTS, 0, R.string.menu_add_to_contacts);
                    String number = recipients.get(0).getNumber();
                    mIpConvList.onIpCreateContextMenu(menu, number);
                }
            }
            if (mIsSmsEnabled) {
                menu.add(0, MENU_DELETE, 0, R.string.menu_delete);
            }
        }
    };

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        Cursor cursor = mListAdapter.getCursor();
        if (cursor != null && cursor.getPosition() >= 0) {
            Conversation conv = Conversation.from(ConversationList.this, cursor);
            long threadId = conv.getThreadId();
            switch (item.getItemId()) {
            case MENU_DELETE: {
                confirmDeleteThread(threadId, mQueryHandler);
                break;
            }
            case MENU_VIEW: {
                /// M: Fix ipmessage bug @{
                /// M: Code analyze 001, For new feature ALPS00131956,
                /// wappush: method is changed. @{
                /// openThread(threadId, mType);
                /// @}
                viewThread(conv, mType) ;
                /// @}
                break;
            }
            case MENU_VIEW_CONTACT: {
                Contact contact = conv.getRecipients().get(0);
                Intent intent = new Intent(Intent.ACTION_VIEW, contact.getUri());
                intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
                startActivity(intent);
                break;
            }
            case MENU_ADD_TO_CONTACTS: {
                String address = conv.getRecipients().get(0).getNumber();
                startActivity(createAddContactIntent(address));
                break;
            }
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
        // AndroidManifest.xml).  Because the only translatable text
        // in this activity is "New Message", which has the full width
        // of phone to work with, localization shouldn't be a problem:
        // no abbreviated alternate words should be needed even in
        // 'wide' languages like German or Russian.

        super.onConfigurationChanged(newConfig);
        if (DEBUG) {
            Log.v(TAG, "onConfigurationChanged: " + newConfig);
        }
    }

    /**
     * Start the process of putting up a dialog to confirm deleting a thread,
     * but first start a background query to see if any of the threads or thread
     * contain locked messages so we'll know how detailed of a UI to display.
     * @param threadId id of the thread to delete or -1 for all threads
     * @param handler query handler to do the background locked query
     */
    public static void confirmDeleteThread(long threadId, AsyncQueryHandler handler) {
        ArrayList<Long> threadIds = null;
        if (threadId != -1) {
            threadIds = new ArrayList<Long>();
            threadIds.add(threadId);
        }
        confirmDeleteThreads(threadIds, handler);
    }

    /**
     * Start the process of putting up a dialog to confirm deleting threads,
     * but first start a background query to see if any of the threads
     * contain locked messages so we'll know how detailed of a UI to display.
     * @param threadIds list of threadIds to delete or null for all threads
     * @param handler query handler to do the background locked query
     */
    public static void confirmDeleteThreads(Collection<Long> threadIds,
            AsyncQueryHandler handler) {
        Conversation.startQueryHaveLockedMessages(handler, threadIds,
                HAVE_LOCKED_MESSAGES_TOKEN);
    }

    /**
     * Build and show the proper delete thread dialog. The UI is slightly different
     * depending on whether there are locked messages in the thread(s) and whether we're
     * deleting single/multiple threads or all threads.
     * @param listener gets called when the delete button is pressed
     * @param threadIds the thread IDs to be deleted (pass null for all threads)
     * @param hasLockedMessages whether the thread(s) contain locked messages
     * @param context used to load the various UI elements
     */
    private void confirmDeleteThreadDialog(final DeleteThreadListener listener,
            Collection<Long> threadIds,
            boolean hasLockedMessages,
            Context context) {
        View contents = View.inflate(context, R.layout.delete_thread_dialog_view, null);
        TextView msg = (TextView) contents.findViewById(R.id.message);

        if (threadIds == null) {
            msg.setText(R.string.confirm_delete_all_conversations);
        } else {
            // Show the number of threads getting deleted in the confirmation dialog.
            int cnt = threadIds.size();
            msg.setText(context.getResources().getQuantityString(
                R.plurals.confirm_delete_conversation, cnt, cnt));
        }

        final CheckBox checkbox = (CheckBox) contents.findViewById(R.id.delete_locked);
        if (!hasLockedMessages) {
            checkbox.setVisibility(View.GONE);
        } else {
            listener.setDeleteLockedMessage(checkbox.isChecked());
            checkbox.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    listener.setDeleteLockedMessage(checkbox.isChecked());
                }
            });
        }
        /// M: Code analyze 023, For bug ALPS00268161,
        /// when delete one MMS, one sms will not be deleted . @{
        Cursor cursor = null;
        int smsId = 0;
        int mmsId = 0;
        cursor = context.getContentResolver().query(Sms.CONTENT_URI,
                new String[] {"max(_id)"}, null, null, null);
        if (cursor != null) {
            try {
                if (cursor.moveToFirst()) {
                smsId = cursor.getInt(0);
                MmsLog.d(TAG, "confirmDeleteThreadDialog max SMS id = " + smsId);
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
                MmsLog.d(TAG, "confirmDeleteThreadDialog max MMS id = " + mmsId);
                }
            } finally {
                cursor.close();
                cursor = null;
            }
        }
        listener.setMaxMsgId(mmsId, smsId);
        /// @}

        if (mIsInActivity) {
            AlertDialog.Builder builder = new AlertDialog.Builder(context);
            builder.setTitle(R.string.confirm_dialog_title)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setCancelable(true)
                .setPositiveButton(R.string.delete, listener)
                .setNegativeButton(R.string.no, null)
                .setView(contents);
            mDeleteAlertDialog = builder.create();
            mDeleteAlertDialog.show();
        }
    }

    private final OnKeyListener mThreadListKeyListener = new OnKeyListener() {
        @Override
        public boolean onKey(View v, int keyCode, KeyEvent event) {
            if (event.getAction() == KeyEvent.ACTION_DOWN) {
                switch (keyCode) {
                    case KeyEvent.KEYCODE_DEL: {
                        long id = getListView().getSelectedItemId();
                        if (id > 0) {
                            confirmDeleteThread(id, mQueryHandler);
                        }
                        return true;
                    }
                }
            }
            return false;
        }
    };

    public static class DeleteThreadListener implements OnClickListener {
        private final Collection<Long> mThreadIds;
        private final AsyncQueryHandler mHandler;
        private final Context mContext;
        private boolean mDeleteLockedMessages;
        /// M: Code analyze 023, For bug ALPS00268161, when delete one MMS, one
        /// sms will not be deleted. . @{
        private int mMaxMmsId;
        private int mMaxSmsId;

        public void setMaxMsgId(int mmsId, int smsId) {
            mMaxMmsId = mmsId;
            mMaxSmsId = smsId;
        }
        /// @}

        public DeleteThreadListener(Collection<Long> threadIds, AsyncQueryHandler handler,
                Context context) {
            mThreadIds = threadIds;
            mHandler = handler;
            mContext = context;
        }


        public void setDeleteLockedMessage(boolean deleteLockedMessages) {
            mDeleteLockedMessages = deleteLockedMessages;
        }

        @Override
        public void onClick(DialogInterface dialog, final int whichButton) {
            MessageUtils.handleReadReport(mContext, mThreadIds,
                    MtkPduHeaders.READ_STATUS__DELETED_WITHOUT_BEING_READ, new Runnable() {
                @Override
                public void run() {
                    /// M: fix ALPS01524674, mThreadIds is a weak reference to mSelectedThreadIds.
                    /// if delete once, mThreadIds.size() will be updated to 0.
                    if (mThreadIds != null && mThreadIds.size() == 0) {
                        return;
                    }
                    /// M: Code analyze 013, For bug ALPS00046358 , The method about the
                    /// handler with progress dialog functio . @{
                    showProgressDialog();
                    /// @}
                    sIsDeleting = true;
                    /// M: delete ipmessage in ipmessage db
                    IpMessageUtils.onIpDeleteMessage(mContext,
                            mThreadIds, mMaxSmsId, mDeleteLockedMessages);

                    int token = DELETE_CONVERSATION_TOKEN;
                    /// M: wappush: do not need modify the code here,
                    /// but delete function in provider has been modified.
                    /// M: fix bug ALPS00415754, add some useful log
                    MmsLog.d(TAG, "before delete threads in conversationList, mThreadIds.size = "
                            + (mThreadIds == null ? "null" : mThreadIds.size()));
                    if (mThreadIds == null) {
                        /// M: Code analyze 023, For bug ALPS00268161, when delete one
                        /// MMS, one sms will not be deleted. . @{
                        Conversation.startDeleteAll(mHandler, token,
                                mDeleteLockedMessages, mMaxMmsId, mMaxSmsId);
                        /// @}
                        /// M: modify for fix ALPS01071334, move to onDeleteCompleted().
                        /// in some case, when refresh run, the messages have not
                        /// been deleted all, the draft state has not been changed,
                        /// so draftcache is wrong
                        // DraftCache.getInstance().refresh();
                    } else if (mThreadIds.size() <= 1) {
                        /// @}
                        for (long threadId : mThreadIds) {
                            /// M: Code analyze 023, For bug ALPS00268161, when delete one
                            /// MMS, one sms will not be deleted . @{
                            Conversation.startDelete(mHandler, token, mDeleteLockedMessages,
                                    threadId, mMaxMmsId, mMaxSmsId);
                        }
                    } else if (mThreadIds.size() > 1) {
                        /// M: Fix bug ALPS00780175, The 1300 threads deleting will cost more than
                        /// 10 minutes. Avoid to delete multi threads one by one,
                        /// let MmsSmsProvider
                        /// handle this action. @{
                        String[] threadIds = new String[mThreadIds.size()];
                        int i = 0;
                        for (long thread : mThreadIds) {
                            threadIds[i++] = String.valueOf(thread);
                        }
                        Conversation.startMultiDelete(mHandler, token, mDeleteLockedMessages,
                                threadIds, mMaxMmsId, mMaxSmsId);
                        /// @}
                    }
                    MmsLog.d(TAG, "after delete threads in conversationList");
                    /// @}
                }
                /// M: Code analyze 013, For bug ALPS00046358 , The method about the handler
                /// with progress dialog functio . @{
                private void showProgressDialog() {
                    if (mHandler instanceof BaseProgressQueryHandler) {
                        ((BaseProgressQueryHandler) mHandler).setProgressDialog(
                                DeleteProgressDialogUtil.getProgressDialog(mContext));
                        ((BaseProgressQueryHandler) mHandler).showProgressDialog();
                    }
                }
                /// @}
            });
            dialog.dismiss();
        }
    }

    private final Runnable mDeleteObsoleteThreadsRunnable = new Runnable() {
        @Override
        public void run() {
            /// M: Code analyze 024, For bug ALPS00234739 , draft can't be
            /// saved after share the edited picture to the same ricipient, so
            ///Remove old Mms draft in conversation list instead of compose view . @{
            MessageUtils.asyncDeleteOldMms();
            /// @}
            Conversation.asyncDeleteObsoleteThreads(mQueryHandler,
                    DELETE_OBSOLETE_THREADS_TOKEN);
        }
    };

    private final class ThreadListQueryHandler extends BaseProgressQueryHandler {
        public ThreadListQueryHandler(ContentResolver contentResolver) {
            super(contentResolver);
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
              /// M: Code analyze 015, For bug,  add cursor == null check . @{
              if (cursor == null) {
              /// M: Decrease query counter and do next query if any request  @{
                  setProgressBarIndeterminateVisibility(false);
                  if (mNeedQuery && mIsInActivity) {
                      MmsLog.d(TAG, "onQueryComplete cursor == null startAsyncQuery");
                      startAsyncQuery();
                  }
                  return;
              /// @}
              }
              if (mSearchView != null) {
                  String searchString = mSearchView.getQuery().toString();
                  if (searchString != null && searchString.length() > 0) {
                      MmsLog.d(TAG, "onQueryComplete mSearchView != null");
                      mSearchView.getSuggestionsAdapter().notifyDataSetChanged();
                  }
              }
            /// @}
            switch (token) {
            case THREAD_LIST_QUERY_TOKEN:
                /// M: If no listener for content change, means no need to refresh list @{
                if (mListAdapter.getOnContentChangedListener() == null) {
                    cursor.close();
                    return;
                }
                /// @}
                MmsLog.d(TAG, "onQueryComplete cursor count is " + cursor.getCount());
                /// M: add for ipmessage, update Empty View
                if (mIpConvList != null) {
                    mIpConvList.onIpUpdateEmptyView(cursor);
                }
//                updateEmptyView(cursor);
                mListAdapter.changeCursor(cursor);
                /// M: make a timer to update the list later, the time should update.
                mHandler.postDelayed(new Runnable() {
                    public void run() {
                        /// M: if only has header, not notify
                        if ((mIpConvList != null) && !mIpConvList.onIpQueryCompleteQueryList(mListView)) {
                            mListAdapter.notifyDataSetChanged();
                        }
                    }
                }, 60000);

//                if (!MmsConfig.isActivated(ConversationList.this)) {
                    if (mListAdapter.getCount() == 0
                            && getListView().getEmptyView() instanceof TextView) {
                        ((TextView) (getListView()
                                .getEmptyView())).setText(R.string.no_conversations);
                    }
//                }
                // M: add code @{ 
                if (!Conversation.isInitialized()) {
                    Conversation.init(getApplicationContext());
                } else {
                    Conversation.removeInvalidCache(cursor);
                }


                if (mNeedToMarkAsSeen) {
                    mNeedToMarkAsSeen = false;
                /// M: Code analyze 016, For new feature, wappush: method is changed . @{
                    Conversation.markAllConversationsAsSeen(getApplicationContext(),
                            Conversation.MARK_ALL_MESSAGE_AS_SEEN);

                    // Delete any obsolete threads. Obsolete threads are threads that aren't
                    // referenced by at least one message in the pdu or sms tables. We only call
                    // this on the first query (because of mNeedToMarkAsSeen).
                    mHandler.post(mDeleteObsoleteThreadsRunnable);
                }

                /// M: ALPS03646775. Get all threads id and sync to selected thread list
                HashSet<Long> theadIds = new HashSet<Long>();
                if (cursor.moveToFirst()) {
                    do {
                        theadIds.add(cursor.getLong(0));
                    } while (cursor.moveToNext());
                }
                List<Long> selected = new ArrayList<Long>(mListAdapter.getSelectedThreadsList());
                for (Long i : selected) {
                    if (!theadIds.contains(i)) {
                        mListAdapter.removeSelectedState(i);
                        MmsLog.d(TAG, "Thread deleted, remove from selected threads list: " + i);
                    }
                }
                /// @}

                /// M: Code analyze 005, For new feature ALPS00247476 . @{
                if (mActionMode != null) {
                    mActionModeListener.updateActionMode();
                }
                /// @}
                /// M: Fix bug ALPS00416081
                setDeleteMenuVisible(mOptionsMenu);

                /// Google JB MR1.1 patch. conversation list can restore scroll position
                if (mSavedFirstVisiblePosition != AdapterView.INVALID_POSITION) {
                    // Restore the list to its previous position.
                    getListView().setSelectionFromTop(mSavedFirstVisiblePosition,
                            mSavedFirstItemOffset);
                    mSavedFirstVisiblePosition = AdapterView.INVALID_POSITION;
                }
                break;

            case UNREAD_THREADS_QUERY_TOKEN:
                int count = 0;
                if (cursor != null) {
                    try {
                        if (cursor.getCount() > 0) {
                            cursor.moveToFirst();
                            count = cursor.getInt(0);
                        }
                        if (mIpConvList != null) {
                            count = mIpConvList.onIpGetUnreadCount(cursor, count);
                        }
                        MmsLog.d(TAG, "get threads unread message count = " + count);
                        /// @}
                    } finally {
                        cursor.close();
                    }
                }
                /// M: modified for unread count display
                if (count > MAX_DISPLAY_UNREAD_COUNT) {
                    mUnreadConvCount.setText(DISPLAY_UNREAD_COUNT_CONTENT_FOR_ABOVE_99);
                } else {
                    mUnreadConvCount.setText(count > 0 ? Integer.toString(count) : null);
                }
                break;

            case HAVE_LOCKED_MESSAGES_TOKEN:
                /// M: add a log
                MmsLog.d(TAG, "onQueryComplete HAVE_LOCKED_MESSAGES_TOKEN");
                @SuppressWarnings("unchecked")
                Collection<Long> threadIds = (Collection<Long>) cookie;
                ListView listView = getListView();
                ConversationListAdapter adapter;

                /// M: unwrap adapter if ListView has header
                if ((mIpConvList != null) && mIpConvList.onIpQueryComplete(listView) != null) {
                    adapter = (ConversationListAdapter) mIpConvList.onIpQueryComplete(listView);
                } else {
                    adapter = (ConversationListAdapter) listView.getAdapter();
                }

                if (adapter != null && threadIds != null) {
                    Cursor c = adapter.getCursor();
                    if (c != null && c.getCount() == threadIds.size()) {
                        threadIds = null;
                    }
                }
                confirmDeleteThreadDialog(new DeleteThreadListener(threadIds, mQueryHandler,
                        ConversationList.this), threadIds,
                        cursor != null && cursor.getCount() > 0,
                        ConversationList.this);
                if (cursor != null) {
                    cursor.close();
                }
                break;

            default:
                Log.e(TAG, "onQueryComplete called with unknown token " + token);
            }

            /// M: if only has header, show listview
            mIpConvList.onIpQueryCompleteEnd(mListView, mHandler, mListAdapter);

            /// M: Do next query if any requested @{
            if (mNeedQuery && mIsInActivity) {
                startAsyncQuery();
            }
            /// @}
        }

        @Override
        protected void onDeleteComplete(int token, Object cookie, int result) {
            /// M: comment it
            //super.onDeleteComplete(token, cookie, result);
            sIsDeleting = false;
            switch (token) {
            case DELETE_CONVERSATION_TOKEN:
                long threadId = cookie != null ? (Long) cookie : -1;     // default to all threads
                if (threadId == -1) {
                    // Rebuild the contacts cache now that all threads and their associated unique
                    // recipients have been deleted.
                    Contact.initCache(ConversationList.this);
                }
                /// M: threadId == -2 is multidelete. for fix bug ALPS01071334
                else if (threadId != -2) {
                    // Remove any recipients referenced by this single thread from the
                    // contacts cache. It's possible for two or more threads to reference
                    // the same contact. That's ok if we remove it. We'll recreate that contact
                    // when we init all Conversations below.
                    Conversation conv = Conversation.get(ConversationList.this, threadId, false);
                    if (conv != null) {
                        ContactList recipients = conv.getRecipients();
                        for (Contact contact : recipients) {
                            contact.removeFromCache();
                        }
                    }
                }
                // Make sure the conversation cache reflects the threads in the DB.
                Conversation.init(ConversationList.this);
                if (mActionMode != null) {
                        mHandler.postDelayed(new Runnable() {
                            public void run() {
                                if (mActionMode != null && !isFinishing()) {
                                    mActionMode.finish();
                                }
                            }
                        }, 300);
                }

                try {
                    IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                        ServiceManager.getService("phoneEx"));
                    if (iTelEx.isTestIccCard(0)) {
                        MmsLog.d(CONV_TAG, "All threads has been deleted, send notification..");
                        MtkSmsManager
                                .getSmsManagerForSubscriptionId(SmsReceiver.sLastIncomingSmsSubId)
                                .setSmsMemoryStatus(true);
                    }
                } catch (Exception ex) {
                    MmsLog.e(CONV_TAG, " " + ex.getMessage());
                }

                // Update the notification for new messages since they
                // may be deleted.
                MessagingNotification.nonBlockingUpdateNewMessageIndicator(ConversationList.this,
                        MessagingNotification.THREAD_NONE, false);
                // Update the notification for failed messages since they
                // may be deleted.
                MessagingNotification
                        .nonBlockingUpdateSendFailedNotification(ConversationList.this);
                /// M: update download failed messages since they may be deleted too.
                MessagingNotification.updateDownloadFailedNotification(ConversationList.this);

                /// M: Code analyze 001, For new feature ALPS00131956,
                /// wappush: Update the notification for new WAP Push/CB
                /// messages. @{
                if (FeatureOption.MTK_WAPPUSH_SUPPORT) {
                    WapPushMessagingNotification
                        .nonBlockingUpdateNewMessageIndicator(
                                ConversationList.this, WapPushMessagingNotification.THREAD_NONE);
                }
                /// @}
                    /// M: Code analyze 006, For bug ALPS00291435, solve no
                    /// response while deleting 50 messages . @{
                CbMessagingNotification.updateNewMessageIndicator(ConversationList.this);
                    /// @}
                // Make sure the list reflects the delete
                /// M: comment this line
                // startAsyncQuery();.-
                /** M: fix bug ALPS00357750 @{ */
                dismissProgressDialog();
                /** @} */
                /** M: show a toast
                if (DeleteThreadListener.sDeleteNumber > 0) {
                    int count = DeleteThreadListener.sDeleteNumber;
                    String toastString = ConversationList.this.getResources().getQuantityString(
                            R.plurals.deleted_conversations, count, count);
                    Toast.makeText(ConversationList.this, toastString, Toast.LENGTH_SHORT).show();
                    DeleteThreadListener.sDeleteNumber = 0;
                }
                */
                /// M: google android4.2 patch
                MmsWidgetProvider.notifyDatasetChanged(getApplicationContext());
                /// @}
                break;

            case DELETE_OBSOLETE_THREADS_TOKEN:
                // Nothing to do here.
                MmsLog.d(TAG, "DraftManager.sEditingThread.clear()");
                DraftManager.sEditingThread.clear();
                break;
            }
        }
    }
    
    /// M: Code analyze 005, For new feature ALPS00247476,
    /// replace multichoicemode by longClickListener . @{
    private class ModeCallback implements ActionMode.Callback {
        private View mMultiSelectActionBarView;
        /// M:
        private Button mSelectionTitle;
        //private TextView mSelectedConvCount;
        private HashSet<Long> mSelectedThreadIds;

        private MenuItem mDeleteItem;

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            MenuInflater inflater = getMenuInflater();
            /// M: Optimize select all performance, restore actionmode status. @{
            mListAdapter.clearstate();
            // M: add for ALPS01988446, dismiss search view.
            mSearchItem.collapseActionView();
            if (mIsNeedRestoreAdapterState) {
                for (int i = 0; i < mListSelectedThreads.length; i++) {
                    mListAdapter.setSelectedState(mListSelectedThreads[i]);
                }
                mIsNeedRestoreAdapterState = false;
            } else {
                Log.d(TAG, "onCreateActionMode: no need to restore adapter state");
            }
            /// @}
            mSelectedThreadIds = new HashSet<Long>();
            inflater.inflate(R.menu.conversation_multi_select_menu_with_selectall, menu);

            mDeleteItem = menu.findItem(R.id.delete);

            if (mMultiSelectActionBarView == null) {
                mMultiSelectActionBarView = LayoutInflater.from(ConversationList.this)
                    .inflate(R.layout.conversation_list_multi_select_actionbar2, null);
                /// M: change select tips style
                mSelectionTitle =
                    (Button) mMultiSelectActionBarView.findViewById(R.id.selection_menu);
                //mSelectedConvCount =
                //    (TextView)mMultiSelectActionBarView.findViewById(R.id.selected_conv_count);
            }
            mode.setCustomView(mMultiSelectActionBarView);
            ((Button) mMultiSelectActionBarView.findViewById(R.id.selection_menu))
                .setText(R.string.select_conversations);
            /// M: Code analyze 008, For bug ALPS00250948, disable search in
            // multi-select status . @{
            mDisableSearchFalg = true;
            /// @}
            /// M: Code analyze 005, For new feature ALPS00247476, set long clickable . @{
            getListView().setLongClickable(false);
            /// @}
            if (mIpConvList != null) {
                mIpConvList.onIpCreateActionMode(mode, menu);
            }
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            if (mMultiSelectActionBarView == null) {
                ViewGroup v = (ViewGroup) LayoutInflater.from(ConversationList.this)
                    .inflate(R.layout.conversation_list_multi_select_actionbar2, null);
                mode.setCustomView(v);
                /// M: change select tips style
                mSelectionTitle =
                    (Button) mMultiSelectActionBarView.findViewById(R.id.selection_menu);
                //mSelectedConvCount = (TextView)v.findViewById(R.id.selected_conv_count);

            }
            /// M: redesign selection action bar and add shortcut in common version. @{
            if (mCustomMenu == null) {
                mCustomMenu = new CustomMenu(ConversationList.this);
            }
            mSelectionMenu = mCustomMenu.addDropDownMenu(mSelectionTitle, R.menu.selection);
            mSelectionMenuItem = mSelectionMenu.findItem(R.id.action_select_all);
            mCustomMenu.setOnMenuItemClickListener(new OnMenuItemClickListener() {
                public boolean onMenuItemClick(MenuItem item) {
                    if (mSelectionTitle != null) {
                        mSelectionTitle.setEnabled(false);
                    }
                    if (mListAdapter.getCount() == mListAdapter.getSelectedThreadsList().size()) {
                        setAllItemChecked(mActionMode, false);
                    } else {
                        setAllItemChecked(mActionMode, true);
                    }
                    return false;
                }
            });
            /// @}
            if (mIpConvList != null) {
                mIpConvList.onIpPrepareActionMode(mode, menu);
            }
            return true;
        }

        @Override
        public boolean onActionItemClicked(final ActionMode mode, MenuItem item) {
            if (mIpConvList != null) {
                mIpConvList.onIpActionItemClicked(mode, item, mSelectedThreadIds);
            }
            switch (item.getItemId()) {
                case R.id.delete:
                    if (mSelectedThreadIds.size() > 0) {
                        Log.i(TAG, "ConversationList->ModeCallback: delete");
                        if (mDeleteAlertDialog != null && mDeleteAlertDialog.isShowing()) {
                            MmsLog.d(TAG, "no need to show delete dialog");
                        } else {
                            confirmDeleteThreads(mSelectedThreadIds, mQueryHandler);
                        }
                    } else {
                        item.setEnabled(false);
                    }
                    break;

                default:
                    break;
            }
            return true;
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            /// M: Code analyze 008, For bug ALPS00250948,
            /// disable search in multi-select status . @{
            mDisableSearchFalg = false;
            /// @}
            mListAdapter.clearstate();
            /// M: Code analyze 005, For new feature ALPS00247476, add selectAll/unSelectAll . @{
            getListView().setLongClickable(true);
            mSelectionMenu.dismiss();
            mActionMode = null;
            if (mListAdapter != null) {
                mListAdapter.notifyDataSetChanged();
            }
            /// @}
        }

        public void setItemChecked(int position, boolean checked, Cursor cursor) {
            ListView listView = getListView();
            if (cursor == null) {
                cursor = (Cursor) listView.getItemAtPosition(position);
            } else {
                cursor.moveToPosition(position);
            }
            long threadId = cursor.getLong(0);
            boolean isChecked = mListAdapter.isContainThreadId(threadId);
            if (checked == isChecked) {
                return;
            }
            if (checked) {
                mListAdapter.setSelectedState(threadId);
            } else {
                mListAdapter.removeSelectedState(threadId);
            }
        }
        /// @}

        private void updateActionMode() {
            mSelectedThreadIds = mListAdapter.getSelectedThreadsList();
            int checkedNum = mSelectedThreadIds.size();
            /// M: Code analyze 018, For bug, enable or diable mDeleteItem menu . @{
            if (mDeleteItem != null) {
                if (checkedNum > 0) {
                    mDeleteItem.setEnabled(true);
                } else {
                    mDeleteItem.setEnabled(false);
                }
                /// @}
            }
            if (mSelectionTitle != null && !mSelectionTitle.isEnabled()) {
                mSelectionTitle.setEnabled(true);
            }
            /// M: exit select mode if no item select
            if (checkedNum <= 0 && mActionMode != null) {
                mActionMode.finish();
                /// M: add for fix ALPS01448613, when checkedNum == 0,
                /// dismiss the deleteAlertDialog. @{
                if (mDeleteAlertDialog != null && mDeleteAlertDialog.isShowing()) {
                    mDeleteAlertDialog.dismiss();
                    mDeleteAlertDialog = null;
                }
                /// @}
            }
            mSelectionTitle.setText(ConversationList.this.getResources().getQuantityString(
                    R.plurals.message_view_selected_message_count, checkedNum, checkedNum));
            if (mActionMode != null) {
                mActionMode.invalidate();
            }
            updateSelectionTitle();
            if (mIpConvList != null) {
                mIpConvList.onIpUpdateActionMode(mSelectedThreadIds);
            }
        }

        /// M: Code analyze 005, For new feature ALPS00247476, select all messages . @{
        private void setAllItemChecked(ActionMode mode, final boolean checked) {
            mListAdapter.clearstate();
            if (checked) {
                mDeleteItem.setEnabled(false);
            }

            Cursor cursor = null;
//            String selection = null;
            // / M: ipmessage query.
            if (checked) {
                if (mIpConvList != null) {
                    cursor = mIpConvList.onIpGetAllThreads();
                }
                ConversationList conversationList = (ConversationList)getContext();
                if ((conversationList != null) && cursor == null) {
                    cursor = conversationList.getContentResolver()
                        .query(Conversation.sAllThreadsUri,
                            Conversation.ALL_THREADS_PROJECTION, null, null,
                            Conversations.DEFAULT_SORT_ORDER);
                }
                try {
                    if (cursor != null) {
                        MmsLog.d(TAG, "select all, cursor count is " + cursor.getCount());
                        for (int position = 0; position < cursor.getCount(); position++) {
                            setItemChecked(position, checked, cursor);
                        }
                    }
                } finally {
                    if (cursor != null) {
                        cursor.close();
                    }
                }
            }

            updateActionMode();
            // / M: Code analyze 018, For bug, enable or diable
            // mDeleteItem menu . @{
            if (checked) {
                mDeleteItem.setEnabled(true);
            } else {
                mDeleteItem.setEnabled(false);
            }
            // / @}

            if (mListAdapter != null) {
                mListAdapter.notifyDataSetChanged();
            }
        }
        /// @}

    }

    private void log(String format, Object... args) {
        String s = String.format(format, args);
        Log.d(TAG, "[" + Thread.currentThread().getId() + "] " + s);
    }

    /// M: Code analyze 009, For bug ALPS00270910, Default SIM card icon shown
    /// in status bar is incorrect, need to get current sim information . @{
    public static Activity getContext() {
        return sActivity;
    }
    /// @}

    /// M: Code analyze 005, For new feature ALPS00247476, long click Listenner . @{
    class ItemLongClickListener implements  ListView.OnItemLongClickListener {

        public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
            mActionMode = startActionMode(mActionModeListener);
            Log.d(TAG, "OnItemLongClickListener");
            mActionModeListener.setItemChecked(position, true, null);
            mActionModeListener.updateActionMode();
            if (mListAdapter != null) {
                mListAdapter.notifyDataSetChanged();
            }
            return true;
        }
    }
    /// @}

    @Override
    protected void onResume() {
        super.onResume();
        if (sInstance != this) {
            MmsLog.d(TAG, "onResume, sInstance!=this, sInstance:" + sInstance + ", this:" + this);
            return;
        }
        boolean isSmsEnabled = MmsConfig.isSmsEnabled(this);
        if (isSmsEnabled != mIsSmsEnabled) {
            mIsSmsEnabled = isSmsEnabled;
            if (!mIsFirstCreate) {
                invalidateOptionsMenu();
            }
        }
        // Multi-select is used to delete conversations. It is disabled if we are not the sms app.
        ListView listView = getListView();
        if (mIsSmsEnabled) {
            if (listView.getOnItemLongClickListener() == null) {
                listView.setOnItemLongClickListener(new ItemLongClickListener());
            }
        } else {
            listView.setOnItemLongClickListener(null);
            if (mActionMode != null) {
                mActionMode.finish();
                if (mDeleteAlertDialog != null && mDeleteAlertDialog.isShowing()) {
                    mDeleteAlertDialog.dismiss();
                    mDeleteAlertDialog = null;
                }
            }
        }

        // Show or hide the SMS promo banner
        if (mIsSmsEnabled || MmsConfig.isSmsPromoDismissed(this)) {
            mSmsPromoBannerView.setVisibility(View.GONE);
        } else {
            initSmsPromoBanner();
            mSmsPromoBannerView.setVisibility(View.VISIBLE);
        }

        // M: add for ALPS01846474, dismiss option menu if airplane mode changed.
        registerReceiver(mAirPlaneReceiver, new IntentFilter(Intent.ACTION_AIRPLANE_MODE_CHANGED));
        mReceiverTag = true;
        if (mOpConversationList != null) {
            mOpConversationList.onResume(mIsSmsEnabled);
        }
        mIsFirstCreate = false;
    }

    /// M: Code analyze 001, For new feature ALPS00242732, SIM indicator UI is not good . @{
    @Override
    protected void onPause() {
        super.onPause();

        /// Google JB MR1.1 patch. conversation list can restore scroll position
        // Remember where the list is scrolled to so we can restore the scroll position
        // when we come back to this activity and *after* we complete querying for the
        // conversations.
        ListView listView = getListView();
        mSavedFirstVisiblePosition = listView.getFirstVisiblePosition();
        View firstChild = listView.getChildAt(0);
        mSavedFirstItemOffset = (firstChild == null) ? 0 : firstChild.getTop();
        // M: add for ALPS01846474, dismiss option menu if airplane mode changed.
        if (mReceiverTag) {
            unregisterReceiver(mAirPlaneReceiver);
            mReceiverTag = false;
        }
    }
    /// @}

    @Override
    protected void onDestroy() {
        /// M: Remove not start queries, and close the last cursor hold be adapter@{
        MmsLog.d(TAG, "onDestroy, this = " + this);
        if (this == sInstance) {
            sInstance = null;
            MmsLog.d(TAG, "onDestroy, sInstance = null");
        }
        if (!PermissionCheckUtil.checkRequiredPermissions(this)) {
            super.onDestroy();
            return;
        }

        if (mQueryHandler != null) {
            mQueryHandler.removeCallbacksAndMessages(null);
            mQueryHandler.cancelOperation(THREAD_LIST_QUERY_TOKEN);
            mQueryHandler.cancelOperation(UNREAD_THREADS_QUERY_TOKEN);
        }
        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
        }
        if (mListAdapter != null) {
            mListAdapter.changeCursor(null);
        }
        /// @}

        /// M: Code analyze 004, For bug ALPS00247476, ensure the scroll smooth . @{
        mScrollListener.destroyThread();
        /// @}

        if (mIpConvList != null) {
            mIpConvList.onIpDestroy();
        }

        if (mActionMode != null) {
            mListAdapter.clearstate();
            mActionMode = null;
        }
        if (mDeleteAlertDialog != null) {
            mDeleteAlertDialog.dismiss();
            mDeleteAlertDialog = null;
        }
            /* Mem opt*/
        if ((sActivity != null) && sActivity.equals(ConversationList.this)) {
            MmsLog.d(TAG, "onDestroy1");
            sActivity = null;
            //MtkPduPersister.releaseOwnPduPersister(this);
            Conversation.clearCache();
            MmsApp.getApplication().getThumbnailManager().clear();
            MmsApp.getApplication().getPduLoaderManager().clear();
        }
        super.onDestroy();
    }

    /// M: fix ALPS01535674,ALPS01597191. don't dimiss action
    /// mode when back from ComposeMessageActivity. @{
    private boolean mBackKeyisDown = false;
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {

        if (event.getAction() == KeyEvent.ACTION_UP
                && event.getKeyCode() == KeyEvent.KEYCODE_BACK && !mBackKeyisDown) {
            mBackKeyisDown = false;
            return true;
        }
        if (event.getAction() == KeyEvent.ACTION_DOWN
                && event.getKeyCode() == KeyEvent.KEYCODE_BACK) {
            mBackKeyisDown = true;
        }
        return super.dispatchKeyEvent(event);
    }
    /// @}

    /// M: Code analyze 008, For bug ALPS00250948, disable search in
    /// multi-select status . @{
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (mDisableSearchFalg) {
            switch (keyCode) {
                case KeyEvent.KEYCODE_SEARCH:
                    // do nothing since we don't want search box which may cause UI crash
                    // TODO: mapping to other useful feature
                    return true;
                default:
                    break;
            }
        }
        return super.onKeyDown(keyCode, event);
    }
      /// @}

    /// M: Code analyze 020, For bug ALPS00050455, enhance the performance of
    /// launch time . @{
    @Override
    public void onBackPressed() {
        if (isTaskRoot()) {
            /** M: Instead of stopping, simply push this to the back of the stack.
             * This is only done when running at the top of the stack;
             * otherwise, we have been launched by someone else so need to
             * allow the user to go back to the caller.
             */
            if (mSearchItem != null && mSearchItem.isActionViewExpanded()) {
                mSearchItem.collapseActionView();
            } else {
                super.onBackPressed();
            }
        } else {
            super.onBackPressed();
        }
    }

    /// @}

    /// M: Code analyze 009, For new feature, plugin . @{
    private void initPlugin(Context context) {
        mOpConversationList = OpMessageUtils.getOpMessagePlugin().getOpConversationListExt();
        mOpConversationList.init(this);
    }

    /// @}

    /// M: Code analyze 013, For bug ALPS00046358 , The base class about the
    /// handler with progress dialog functio . @{
    public abstract static class BaseProgressQueryHandler extends AsyncQueryHandler {
        private NewProgressDialog mDialog;
        private int mProgress;

        public BaseProgressQueryHandler(ContentResolver resolver) {
            super(resolver);
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

        /** M:
         * Sets the max progress.
         * @param max the max progress.
         */
        public void setMax(int max) {
            if (mDialog != null) {
                mDialog.setMax(max);
            }
        }

        /** M:
         * Shows the progress dialog. Must be in UI thread.
         */
        public void showProgressDialog() {
            if (mDialog != null) {
                mDialog.show();
            }
        }

        /** M:
         * Rolls the progress as + 1.
         * @return if progress >= max.
         */
        protected boolean progress() {
            if (mDialog != null) {
                return ++mProgress >= mDialog.getMax();
            } else {
                return false;
            }
        }

        /** M: fix bug ALPS00351620
         * Dismisses the progress dialog.
         */
        protected void dismissProgressDialog() {
            // M: fix bug ALPS00357750
            if (mDialog == null) {
                MmsLog.e(TAG, "mDialog is null!");
                return;
            }

            mDialog.setDismiss(true);
            try {
                mDialog.dismiss();
            } catch (IllegalArgumentException e) {
                // if parent activity is destroyed,and code come here, will happen this.
                // just catch it.
                MmsLog.e(TAG, "ignore IllegalArgumentException:" + e);
            }
            mDialog = null;
        }
    }

    /// @}

    /// M: Code analyze 009, For new feature, plugin . @{
    public void showSimSms() {
        int subCount = SubscriptionManager.from(this).getActiveSubscriptionInfoCount();
        if (subCount > 1) {
            Intent simSmsIntent = new Intent();
            simSmsIntent.setClass(this, SubSelectActivity.class);
            simSmsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            simSmsIntent.putExtra(SmsPreferenceActivity.PREFERENCE_KEY,
                    SettingListActivity.SMS_MANAGE_SIM_MESSAGES);
            simSmsIntent.putExtra(SmsPreferenceActivity.PREFERENCE_TITLE_ID,
                    R.string.pref_title_manage_sim_messages);
            startActivity(simSmsIntent);
        } else if (subCount == 1) {
            Intent simSmsIntent = new Intent();
            simSmsIntent.setClass(this, ManageSimMessages.class);
            simSmsIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            simSmsIntent.putExtra(PhoneConstants.SUBSCRIPTION_KEY, SubscriptionManager
                    .from(MmsApp.getApplication()).getActiveSubscriptionInfoList().get(0)
                    .getSubscriptionId());
            startActivity(simSmsIntent);
        } else {
            Toast.makeText(ConversationList.this, R.string.no_sim_card, Toast.LENGTH_SHORT).show();
        }
    }

    /// M: add for ipmessage
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (mIpConvList != null) {
            mIpConvList.onIpActivityResult(requestCode, resultCode, data);
        }
        return;
    }

    /// M: Fix bug ALPS00416081 @{
    private Menu mOptionsMenu;

    private void setDeleteMenuVisible(Menu menu) {
        if (menu != null) {
            MenuItem item = menu.findItem(R.id.action_delete_all);
            if ((item != null) && (mListAdapter != null)) {
                item.setVisible(mListAdapter.getCount() > 0 && mIsSmsEnabled);
            }
        }
    }
    /// @}

    /// M: redesign selection action bar and add shortcut in common version. @{
    private CustomMenu mCustomMenu;
    private DropDownMenu mSelectionMenu;
    private MenuItem mSelectionMenuItem;

    private void updateSelectionTitle() {
        if (mSelectionMenuItem != null) {
            if (mListAdapter.getCount() == mListAdapter.getSelectedThreadsList().size()) {
                mSelectionMenuItem.setTitle(R.string.unselect_all);
            } else {
                mSelectionMenuItem.setTitle(R.string.select_all);
            }
        }
    }
    /// @}

    /// Google JB MR1.1 patch. conversation list can restore scroll position
    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        /// M: Optimize select all performance, save actionmode status. @{
        if (mActionMode != null && !sIsDeleting) {
            Log.d(TAG, "onSaveInstanceState: mActionMode not null");
            outState.putBoolean(ACTIONMODE, true);
            outState.putBoolean(NEED_RESTORE_ADAPTER_STATE, true);
            HashSet<Long> selectedThreadSet = mListAdapter.getSelectedThreadsList();
            Long[] selectList =
                (Long[]) selectedThreadSet.toArray(new Long[selectedThreadSet.size()]);
            long[] selectedThreadsList = new long[selectList.length];
            for (int i = 0; i < selectList.length; i++) {
                selectedThreadsList[i] = selectList[i].longValue();
            }
            outState.putLongArray(SELECT_THREAD_IDS, selectedThreadsList);
            Log.d(TAG, "onSaveInstanceState--selectThreadIds:" + selectedThreadsList.toString());
        }
        /// @}
        outState.putInt(LAST_LIST_POS, mSavedFirstVisiblePosition);
        outState.putInt(LAST_LIST_OFFSET, mSavedFirstItemOffset);
    }

    /// M: Optimize select all performance, restore actionmode status. @{
    @Override
    protected void  onRestoreInstanceState(Bundle state) {
        super.onRestoreInstanceState(state);
        if (state.getBoolean(ACTIONMODE, false)) {
            mListSelectedThreads = state.getLongArray(SELECT_THREAD_IDS);
            mActionMode = this.startActionMode(mActionModeListener);
        }
    }
    /// @}

    private long getExceptId() {
        long exceptID = 0;
        for (long id : DraftManager.sEditingThread) {
            MmsLog.d(TAG, "getExceptId() id = " + id);
            if (id > exceptID) {
                exceptID = id;
            }
        }
        return exceptID;
    }

    /// M: fix bug ALPS00998351, solute the issue "All of the threads still
    /// highlight after you back to all thread view". @{
    public boolean isActionMode() {
        return (mActionMode != null);
    }
    /// @}


    public class ConversationListCallback implements IConversationListCallback {

        @Override
        public void startIpQuery(String selection) {
          Conversation.startQueryExtend(mQueryHandler, THREAD_LIST_QUERY_TOKEN, selection);
          Conversation.startQuery(mQueryHandler, UNREAD_THREADS_QUERY_TOKEN, Threads.READ + "=0"
                  + " and " + selection);
        }

        @Override
        public void setEmptyViewVisible(int visible) {
            mEmptyViewDefault.setVisibility(visible);
        }

        @Override
        public void updateUnreadView(TextView ipUnreadView) {
            mUnreadConvCount = ipUnreadView;
        }

        @Override
        public void notifyDataSetChanged() {
            runOnUiThread(new Runnable() {
                public void run() {
                    if (mListAdapter != null) {
                        mListAdapter.notifyDataSetChanged();
                    }
                }
            });
        }

        @Override
        public void loadNormalLayout() {
            runOnUiThread(new Runnable() {
                public void run() {
                    setupActionBar();
                    setTitle(R.string.app_label);
                }
            });
        }

        @Override
        public void startQuery() {
            startAsyncQuery();
        }

//        @Override
//        public IIpContactExt updateGroupInfo(String number) {
//            Contact contact = Contact.get(number, false);
//            if (contact != null) {
//                contact.setName(null);
//                return contact.getIpContact(ConversationList.this);
//            }
//            return null;
//        }

        @Override
        public String getNumbersByThreadId(long threadId) {
            Conversation conversation = Conversation.get(
                    ConversationList.this, threadId, false);
            return TextUtils.join(",", conversation.getRecipients().getNumbers());
        }

        @Override
        public void invalidateGroupCache() {
            Contact.invalidateGroupCache();
        }

    }
    /**
     * M: add for ALPS01846474, dismiss option menu when AirPlane mode changed.
     */
    private final BroadcastReceiver mAirPlaneReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            closeOptionsMenu();
        }
    };
}
