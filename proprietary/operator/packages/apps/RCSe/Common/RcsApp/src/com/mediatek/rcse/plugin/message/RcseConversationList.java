package com.mediatek.rcse.plugin.message;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;

import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.DialogInterface.OnClickListener;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.ConnectivityManager;
import android.net.Uri;
import android.net.NetworkInfo.State;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Conversations;

import android.text.TextUtils;
import android.util.Log;
import android.view.ActionMode;
import android.view.ContextMenu;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListPopupWindow;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemClickListener;

import com.mediatek.mms.ipmessage.IIpContactExt;
import com.mediatek.mms.ipmessage.DefaultIpConversationListExt;
import com.mediatek.mms.callback.IConversationListCallback;
import com.mediatek.rcse.plugin.message.IpMessageConsts;
import com.mediatek.rcse.plugin.message.IpMessageConsts.RemoteActivities;
import com.mediatek.rcse.plugin.message.IpMessageConsts.SelectContactType;
import com.mediatek.rcse.service.MediatekFactory;
import com.mediatek.rcs.R;

public class RcseConversationList extends DefaultIpConversationListExt
implements INotificationsListener {
    private static String TAG = "RcseConversationList";
    private ConversationEmptyView mEmptyView;
    private ArrayAdapter<String> mDropdownAdapter;
    private AccountDropdownPopup mAccountDropdown;
    private Activity mContext = null;
    private ListView mListView; // we need this to update empty view.
    private LinearLayout mIpEmptyView;
    private View mConversationSpinner;
    private TextView mSpinnerTextView;
    private int mTypingCounter;
    private LinearLayout mNetworkStatusBar;
    private BroadcastReceiver mNetworkStateReceiver;
    private View mEmptyViewDefault;

    private boolean mIsJoynChanged;

    /// M: add for drop down list
    public static final int OPTION_CONVERSATION_LIST_ALL         = 0;
    public static final int OPTION_CONVERSATION_LIST_GROUP_CHATS = 1;
    public static final int OPTION_CONVERSATION_LIST_SPAM        = 2;
    public static final int OPTION_CONVERSATION_LIST_JOYN        = 3;
    public static final int OPTION_CONVERSATION_LIST_XMS         = 4;

    public static final int MENU_ADD_TO_CONTACTS      = 3;

    private static final int REQUEST_CODE_SELECT_CONTACT_FOR_GROUP = 100;
    private static final int REQUEST_CODE_INVITE = 101;
    private static final String KEY_SELECTION_SIMID = "SIMID";
    public static int sConversationListOption = OPTION_CONVERSATION_LIST_ALL;

    public IConversationListCallback mCallback;

    private static final int MENU_CREATE_GROUP_CHAT = 1000;
    private static final int MENU_MARK_AS_SPAM = 2000;
    private static final int MENU_MARK_AS_NONSPAM = 2001;
    private static final int MENU_DELETE = 0;

    @Override
    public boolean onIpConversationListCreate(Activity context,
            IConversationListCallback callback, ListView listview,
            LinearLayout ipEmptyView, LinearLayout networkStatusBar,
            TextView networkStatusTextView) {
        mContext = context;
        mListView = listview;
        mCallback = callback;
        Log.d(TAG, "onIpConversationListCreate():");
        if (IpMmsConfig.isServiceEnabled(context)) {
            IpMessageUtils.addIpMsgNotificationListeners(mContext, this);
        }

        mEmptyView = new ConversationEmptyView(mContext);
        mIpEmptyView = ipEmptyView;
        mIpEmptyView.addView(mEmptyView);
        
        mNetworkStatusBar = networkStatusBar;
        if (networkStatusTextView != null) {
            networkStatusTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_no_internet));
        }
        mIsJoynChanged = true;
        boolean needLoadIpView = onIpNeedLoadView(mIpEmptyView);
        return needLoadIpView;
    }

    public boolean onIpNeedLoadView(View emptyViewDefault) {
        Log.d(TAG, "onIpNeedLoadView is " + mIsJoynChanged);
        mEmptyViewDefault = emptyViewDefault;
        if (mIsJoynChanged) {
            mIsJoynChanged = false;
            if (IpMmsConfig.isActivated(mContext) && (IpMessageServiceMananger.getInstance(mContext).getDisableServiceStatus() != IpMessageConsts.DisableServiceStatus.DISABLE_PERMANENTLY)) {
            	Log.d(TAG, "onIpNeedLoadView activated ");
            	RcseConversation.setActivated(true);
                initSpinnerListAdapter();
                mContext.setTitle("");
                mEmptyViewDefault.setVisibility(View.GONE);
                mIpEmptyView.setVisibility(View.VISIBLE);
                mListView.setEmptyView(mIpEmptyView);
            } else {
                return false;
            }
        }
        if (IpMmsConfig.isServiceEnabled(mContext) && (IpMessageServiceMananger.getInstance(mContext).getDisableServiceStatus() != IpMessageConsts.DisableServiceStatus.DISABLE_PERMANENTLY)) {
            if (mNetworkStateReceiver == null) {
                IntentFilter filter = new IntentFilter();
                filter.addAction(ConnectivityManager.CONNECTIVITY_ACTION);
                mNetworkStateReceiver = new NetworkStateReceiver();
                mContext.registerReceiver(mNetworkStateReceiver, filter);
            }
        }
        else {
            return false;
        }
        return true;
    }

    @Override
    public boolean onIpStartAsyncQuery() {
        if (RcseConversation.getActivated()) {
            Log.d(TAG, "onIpStartAsyncQuery activated is " + true);
            String selection = null;
            if(mSpinnerTextView == null)
                return false;
            switch (sConversationListOption) {
            case OPTION_CONVERSATION_LIST_ALL:
                Log.d(TAG, "startAsyncQuery(): query for all messages except spam");
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_all));
                selection = "threads._id not in (SELECT DISTINCT "
                            + Sms.THREAD_ID
                            + " FROM thread_settings WHERE spam=1) ";
                break;
            case OPTION_CONVERSATION_LIST_GROUP_CHATS:
                Log.d(TAG, "startAsyncQuery(): query for group messages");
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_group_chats));
                selection = "threads._id IN (SELECT DISTINCT " + Sms.THREAD_ID
                        + " FROM thread_settings WHERE spam=0)"
                        + " AND threads.recipient_ids IN (SELECT _id FROM canonical_addresses" + " WHERE "
                        + "SUBSTR(address, 1, 4) = '" + IpMessageConsts.GROUP_START + "'" + ")";
                break;
            case OPTION_CONVERSATION_LIST_SPAM:
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_spam));
                //selection = Threads.SPAM + "=1 OR _ID in (SELECT DISTINCT " + Sms.THREAD_ID + " FROM sms WHERE "
                //        + Sms.SPAM + "=1) ";
                selection = "threads._id IN (SELECT DISTINCT " + Sms.THREAD_ID + " FROM thread_settings WHERE spam=1) ";
                Log.d(TAG, "startAsyncQuery(): query for spam messages, selection = " + selection);
                break;
            case OPTION_CONVERSATION_LIST_JOYN:
                Log.d(TAG, "startAsyncQuery(): query for joyn messages");
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_joyn));
                selection = "threads._id IN (SELECT DISTINCT " + Sms.THREAD_ID
                        + " FROM thread_settings WHERE spam=0)"
                        + " AND threads.recipient_ids IN (SELECT _id FROM canonical_addresses" + " WHERE "
                        + "SUBSTR(address, 1, 4) = '" + IpMessageConsts.JOYN_START + "'" + ")";
                break;
            case OPTION_CONVERSATION_LIST_XMS:
                Log.d(TAG, "startAsyncQuery(): query for xms messages");
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_xms));
                selection = "threads._id IN (SELECT DISTINCT " + Sms.THREAD_ID
                        + " FROM thread_settings WHERE spam=0)"
                        + " AND threads.recipient_ids NOT IN (SELECT _id FROM canonical_addresses" + " WHERE "
                        + "SUBSTR(address, 1, 4) = '" + IpMessageConsts.JOYN_START + "'" + ")"
                        + " AND threads.recipient_ids NOT IN (SELECT _id FROM canonical_addresses" + " WHERE "
                        + "SUBSTR(address, 1, 4) = '" + IpMessageConsts.GROUP_START + "'" + ")";
                break;
            default:
                break;
            }
            Log.d(TAG, "onIpStartAsyncQuery activated end is " + true);
            mCallback.startIpQuery(selection);
            /// M: update dropdown list
            mDropdownAdapter = getDropDownMenuData(mDropdownAdapter, sConversationListOption);
            return true;
        }
        Log.d(TAG, "onIpStartAsyncQuery activated is " + false);
        return false;
    }

    @Override
    public boolean onIpCreateOptionsMenu(Menu menu) {
        Log.d(TAG, "onIpCreateOptionsMenu():");
        MenuItem item = null;

        /// M: add for ipmessage menu
        if (IpMmsConfig.isActivated(mContext)) {
            Resources res = IpMessageResourceMananger.getInstance(mContext).getRcseResource();
            if(PluginUtils.sJOYN_SERVICE_STATUS != 2) {
                item = menu.add(0, MENU_CREATE_GROUP_CHAT, 0, res.getString(R.string.ipmsg_create_group_chat))
                    .setIcon(res.getDrawable(R.drawable.ipmsg_create_a_group_chat))
                    .setTitle(res.getString(R.string.ipmsg_create_group_chat));
                item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            }

            if (item != null &&
                IpMessageServiceMananger.getInstance(mContext).isFeatureSupported(IpMessageConsts.FeatureId.GROUP_MESSAGE)) {
                item.setVisible(true);
                if(PluginUtils.sJOYN_SERVICE_STATUS == 1)
                    item.setEnabled(false);
                else
                   item.setEnabled(true);
            }
        }
        return false;
    }

    @Override
    public boolean onIpPrepareOptionsMenu(Menu menu) {
    	Log.d(TAG, "onIpPrepareOptionsMenu():");
        Resources res = IpMessageResourceMananger.getInstance(mContext).getRcseResource();
       /* MenuItem createGroupItem = menu
                .add(0, MENU_CREATE_GROUP_CHAT, 0, res.getString(R.string.ipmsg_create_group_chat))
                .setIcon(res.getDrawable(R.drawable.ipmsg_create_a_group_chat))
                .setTitle(res.getString(R.string.ipmsg_create_group_chat));
        createGroupItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);*/
        /// M: add for ipmessage menu
       /* if (IpMmsConfig.isActivated(mContext)) {
            if (createGroupItem != null) {
                if (IpMessageServiceMananger.getInstance(mContext).getDisableServiceStatus() == IpMessageConsts.DisableServiceStatus.DISABLE_TEMPORARY) {
                    createGroupItem.setEnabled(false);
                    createGroupItem.getIcon().setAlpha(127);
                } else if (IpMessageServiceMananger.getInstance(mContext).getDisableServiceStatus() == IpMessageConsts.DisableServiceStatus.DISABLE_PERMANENTLY) {
                    createGroupItem.setVisible(false);
                } else {
                    createGroupItem.setVisible(true);
                    createGroupItem.getIcon().setAlpha(255);
                }
            }
//            if (item != null) {
//                // Dim compose if SMS is disabled because it will not work (will show a toast)
//                item.getIcon().setAlpha(mIsSmsEnabled ? 255 : 127);
//            }
        } else {
            if (createGroupItem != null) {
                createGroupItem.setVisible(false);
            }
        }*/
        return false;
    }

    @Override
    public boolean onIpOptionsItemSelected(MenuItem item, boolean isSmsEnabled) {
        /// M: add for ipmessage menu
        if (IpMmsConfig.isActivated(mContext)) {
            switch (item.getItemId()) {
            case MENU_CREATE_GROUP_CHAT:
                ///M: iSMS activation Statistics {@
                if (isSmsEnabled) {
                        if (IpMmsConfig.isActivated(mContext)) {
                        Intent createGroupIntent = new Intent(RemoteActivities.CONTACT);
                        createGroupIntent.putExtra(RemoteActivities.KEY_TYPE,
                                SelectContactType.IP_MESSAGE_USER);
                        createGroupIntent.putExtra(RemoteActivities.KEY_REQUEST_CODE,
                                REQUEST_CODE_SELECT_CONTACT_FOR_GROUP);
                        IpMessageActivitiesManager.getInstance(mContext).startRemoteActivity(mContext, createGroupIntent);
                    } else {
                        return true;
                    }
                } else {
                    Toast.makeText(
                            mContext,
                            IpMessageResourceMananger.getInstance(mContext)
                                    .getSingleString(
                                            IpMessageConsts.string.ipmsg_nms_mms_not_default),
                            Toast.LENGTH_LONG).show();
                }
              ///@}
                break;
            default:
                break;
            }
        }
        return false;
    }

    @Override
    public boolean onIpOpenThread(String number, long threadId) {
        if (IpMmsConfig.isServiceEnabled(mContext)) {
            Log.i(TAG, "open thread by number " + number);
            if (number.startsWith(IpMessageConsts.GROUP_START)) {
                Log.i(TAG, "open group thread by thread id " + threadId);
                openIpMsgThread(threadId);
                return true;
            } else if (number.startsWith(IpMessageConsts.MULTICHAT_START)) {
                Log.i(TAG, "open multi thread by thread id " + threadId);
                openIpMsgThread(threadId);
                return true;
            }
        }
        return false;
    }

    @Override
    public boolean onIpCreateContextMenu(ContextMenu menu, String number) {
        if (IpMmsConfig.isServiceEnabled(mContext) && number.startsWith(IpMessageConsts.GROUP_START)) {
            menu.removeItem(MENU_ADD_TO_CONTACTS);
        }
        return false;
    }

    @Override
    public boolean onIpUpdateEmptyView(Cursor cursor) {
        updateEmptyView(cursor);
        return false;
    }

    public boolean onIpDeleteThreads() {
        return false;
    }

    @Override
    public boolean onIpPrepareActionMode(ActionMode mode, Menu menu) {
        Resources res = IpMessageResourceMananger.getInstance(mContext).getRcseResource();
        if (sConversationListOption == OPTION_CONVERSATION_LIST_SPAM) {
            Log.d(TAG, "onIpPrepareActionMode!OPTION_CONVERSATION_LIST_SPAM");
            menu.removeItem(MENU_MARK_AS_SPAM);
            MenuItem item = menu.add(0, MENU_MARK_AS_SPAM, 0, res.getString(R.string.mark_as_spam))
                    .setIcon(res.getDrawable(R.drawable.ipmsg_mark_as_spam))
                    .setTitle(res.getString(R.string.mark_as_spam));
            item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            if (item != null) {
                item.setVisible(false);
            }
            menu.removeItem(MENU_MARK_AS_NONSPAM);
            item = menu.add(0, MENU_MARK_AS_NONSPAM, 0, res.getString(R.string.remove_frome_spam))
                    .setIcon(res.getDrawable(R.drawable.ipmsg_mark_as_non_spam))
                    .setTitle(res.getString(R.string.remove_frome_spam));
            item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            if (item != null) {
                item.setVisible(true);
            }
        } else {
            menu.removeItem(MENU_MARK_AS_SPAM);
            MenuItem item = menu.add(0, MENU_MARK_AS_SPAM, 0, res.getString(R.string.mark_as_spam))
                    .setIcon(res.getDrawable(R.drawable.ipmsg_mark_as_spam))
                    .setTitle(res.getString(R.string.mark_as_spam));
            item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            if (item != null) {
                if (!IpMmsConfig.isActivated(mContext)) {
                    item.setVisible(false);
                } else {
                    item.setVisible(true);
                }
            }
            menu.removeItem(MENU_MARK_AS_NONSPAM);
            item = menu.add(0, MENU_MARK_AS_NONSPAM, 0, res.getString(R.string.remove_frome_spam))
                    .setIcon(res.getDrawable(R.drawable.ipmsg_mark_as_non_spam))
                    .setTitle(res.getString(R.string.remove_frome_spam));
            item.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
            if (item != null) {
                item.setVisible(false);
            }
        }
        return false;
    }

    @Override
    public boolean onIpActionItemClicked(final ActionMode mode, MenuItem item, HashSet<Long> selectedThreadIds) {
        switch (item.getItemId()) {
        case MENU_MARK_AS_SPAM:
            Log.d(TAG, "click mark as spam!");
            final HashSet<Long> threadIds2 = (HashSet<Long>) selectedThreadIds.clone();
            OnClickListener listener = new OnClickListener() {
                public void onClick(DialogInterface dialog, final int whichButton) {
                    new Thread(new Runnable() {
                        @Override
                        public void run() {
                            int[] contactIds = new int[threadIds2.size()];
                            int i = 0;
                            for (Long threadId : threadIds2) {
                                String numbers = mCallback.getNumbersByThreadId(threadId.longValue());
                                int contactId = IpMessageContactManager.getInstance(mContext)
                                    .getContactIdByNumber(numbers);
                                contactIds[i] = contactId;
                                i++;
                                Log.d(TAG, "threadId:" + threadId + ", contactId:" + contactId);
                            }
                            IpMessageContactManager.getInstance(mContext).addContactToSpamList(contactIds);
                        }
                    }).start();
                    mode.finish();
                }
            };
            AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
            builder.setTitle(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.mark_as_spam))
                .setCancelable(true)
                .setPositiveButton(
                        IpMessageResourceMananger.getInstance(mContext)
                                        .getSingleString(IpMessageConsts.string.ipmsg_continue),
                        listener)
                .setNegativeButton(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_cancel), null)
                .setMessage(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_mark_as_spam_tips))
                .show();
            break;
        case MENU_DELETE:
        	Log.d(TAG, "click MENU_DELETE");
        	onIpDeleteMessage(mContext,selectedThreadIds,2000);
        	break;
        case MENU_MARK_AS_NONSPAM:
            Log.d(TAG, "click mark as nonspam!");
            final HashSet<Long> threadIds = (HashSet<Long>) selectedThreadIds.clone();
            new Thread(new Runnable() {
                @Override
                public void run() {
                    int[] contactIds = new int[threadIds.size()];
                    int i = 0;
                    for (Long threadId : threadIds) {
                        String numbers = mCallback.getNumbersByThreadId(threadId.longValue());
                        int contactId = IpMessageContactManager.getInstance(mContext)
                            .getContactIdByNumber(numbers);
                        contactIds[i] = contactId;
                        i++;
                        Log.d(TAG, "threadId:" + threadId + ", contactId:" + contactId);
                    }
                    IpMessageContactManager.getInstance(mContext).deleteContactFromSpamList(contactIds);
                }
            }).start();
            mode.finish();
            break;
            default:
                break;
        }
        return false;
    }

    public Cursor onIpGetAllThreads() {
        Cursor cursor = null;
        String selection = null;
        if (IpMmsConfig.isActivated(mContext)) {
            switch (sConversationListOption) {
                case OPTION_CONVERSATION_LIST_ALL:
                    Log.d(TAG, "setAllItemChecked(): query for all messages except spam");
                    selection = "threads._id not in (SELECT DISTINCT " + Sms.THREAD_ID
                            + " FROM thread_settings WHERE spam=1) ";
                    cursor = mContext.getContentResolver().query(
                            RcseConversation.sAllThreadsUriExtend,
                            RcseConversation.ALL_THREADS_PROJECTION_EXTEND, selection, null,
                            Conversations.DEFAULT_SORT_ORDER);
                    break;
                case OPTION_CONVERSATION_LIST_GROUP_CHATS:
                    Log.d(TAG, "setAllItemChecked(): query for group messages");
                    selection = "threads._id IN (SELECT DISTINCT "
                            + Sms.THREAD_ID
                            + " FROM thread_settings WHERE spam=0)"
                            + " AND threads.recipient_ids IN (SELECT _id FROM canonical_addresses"
                            + " WHERE " + "SUBSTR(address, 1, 4) = '"
                            + IpMessageConsts.GROUP_START + "'" + ")";
                    cursor = mContext.getContentResolver().query(
                            RcseConversation.sAllThreadsUriExtend,
                            RcseConversation.ALL_THREADS_PROJECTION_EXTEND, selection, null,
                            Conversations.DEFAULT_SORT_ORDER);
                    break;
                case OPTION_CONVERSATION_LIST_SPAM:
                    // selection = Threads.SPAM +
                    // "=1 OR _ID in (SELECT DISTINCT " + Sms.THREAD_ID +
                    // " FROM sms WHERE "
                    // + Sms.SPAM + "=1) ";
                    selection = "threads._id IN (SELECT DISTINCT " + Sms.THREAD_ID
                            + " FROM thread_settings WHERE spam=1) ";
                    Log.d(TAG, "setAllItemChecked(): query for spam messages, selection = "
                            + selection);
                    cursor = mContext.getContentResolver().query(
                            RcseConversation.sAllThreadsUriExtend,
                            RcseConversation.ALL_THREADS_PROJECTION_EXTEND, selection, null,
                            Conversations.DEFAULT_SORT_ORDER);
                    break;
                case OPTION_CONVERSATION_LIST_JOYN:
                    selection = "threads._id IN (SELECT DISTINCT " + Sms.THREAD_ID
                    + " FROM thread_settings WHERE spam=0)"
                    + " AND threads.recipient_ids IN (SELECT _id FROM canonical_addresses" + " WHERE "
                    + "SUBSTR(address, 1, 4) = '" + IpMessageConsts.JOYN_START + "'" + ")";
                    Log.d(TAG, "setAllItemChecked(): query for joyn messages, selection = "
                            + selection);
                    cursor = mContext.getContentResolver().query(
                            RcseConversation.sAllThreadsUriExtend,
                            RcseConversation.ALL_THREADS_PROJECTION_EXTEND, selection, null,
                            Conversations.DEFAULT_SORT_ORDER);
                    break;
                case OPTION_CONVERSATION_LIST_XMS:
                    selection = "threads._id IN (SELECT DISTINCT " + Sms.THREAD_ID
                    + " FROM thread_settings WHERE spam=0)"
                    + " AND threads.recipient_ids NOT IN (SELECT _id FROM canonical_addresses" + " WHERE "
                    + "SUBSTR(address, 1, 4) = '" + IpMessageConsts.JOYN_START + "'" + ")";
                    Log.d(TAG, "setAllItemChecked(): query for xms messages, selection = "
                            + selection);
                    cursor = mContext.getContentResolver().query(
                            RcseConversation.sAllThreadsUriExtend,
                            RcseConversation.ALL_THREADS_PROJECTION_EXTEND, selection, null,
                            Conversations.DEFAULT_SORT_ORDER);
                    break;
                default:
                    Log.d(TAG, "status error! not at any type.");
                    break;
            }
            if (cursor == null) {
                cursor = new MatrixCursor(new String[] {"_id"});
            }
        }
        return cursor;
    }

    public static void onIpDeleteMessage(Context context, Collection<Long> threadIds, int maxSmsId) {
    	Log.d(TAG, "onIpDeleteMessage(): threadIds.lenghth" + threadIds.size() + "threads:" + threadIds);
        IpMessageUtils.deleteIpMessage(context, threadIds, maxSmsId, false);
    }

    public void onIpActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "onActivityResult(): requestCode=" + requestCode + ", resultCode=" + resultCode);
        if (resultCode != mContext.RESULT_OK) {
            Log.d(TAG, "onActivityResult(): result is not OK.");
            return;
        }
        switch (requestCode) {
        case REQUEST_CODE_SELECT_CONTACT_FOR_GROUP:
            String[] mSelectContactsIds = data.getStringArrayExtra(IpMessageUtils.SELECTION_CONTACT_RESULT);
            if (mSelectContactsIds != null) {
                for (String contactId : mSelectContactsIds) {
                    Log.d(TAG, "onActivityResult(): SELECT_CONTACT get contact id = " + contactId);
                }
                Intent intent = new Intent(RemoteActivities.NEW_GROUP_CHAT);
                intent.putExtra(RemoteActivities.KEY_SIM_ID, data.getIntExtra(KEY_SELECTION_SIMID, 0));
                intent.putExtra(RemoteActivities.KEY_ARRAY, mSelectContactsIds);
                IpMessageActivitiesManager.getInstance(mContext).startRemoteActivity(mContext, intent);
                mSelectContactsIds = null;
            } else {
                Log.d(TAG, "onActivityResult(): SELECT_CONTACT get contact id is NULL!");
            }
            break;
        default:
            Log.d(TAG, "onActivityResult(): default return.");
            return;
        }
        return;
    }
    @Override
    public int onIpGetUnreadCount(Cursor cursor, int count) {
        if (RcseConversation.getActivated()) {
            count = cursor.getCount();
        }
        return count;
    }

    @Override
    public void onIpDestroy() {
        /// M: add for ipmessage
        if (mNetworkStateReceiver != null) {
            mContext.unregisterReceiver(mNetworkStateReceiver);
            mNetworkStateReceiver = null;
        }
        if (IpMmsConfig.isServiceEnabled(mContext)) {
            IpMessageUtils.removeIpMsgNotificationListeners(mContext, this);
        }
    }

    private void  updateEmptyView(Cursor cursor) {
        Log.d(TAG, "active:" + IpMmsConfig.isActivated(mContext));
        Log.d(TAG, "cursor count:" + cursor.getCount());
        if (IpMmsConfig.isActivated(mContext) && (cursor != null) && (cursor.getCount() == 0)) {
            // when there is no items, show a view
            Log.d(TAG, "sConversationListOption:" + sConversationListOption);
            switch (sConversationListOption) {
            case OPTION_CONVERSATION_LIST_ALL:
            case OPTION_CONVERSATION_LIST_XMS:
            case OPTION_CONVERSATION_LIST_JOYN:
                mEmptyView.setAllChatEmpty();
                ((TextView) (mEmptyViewDefault)).setText(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_allchat_empty));
                break;
            case OPTION_CONVERSATION_LIST_GROUP_CHATS:
                mEmptyView.setGroupChatEmpty(true);
                break;
            case OPTION_CONVERSATION_LIST_SPAM:
                mEmptyView.setSpamEmpty(true);
                break;
            default:
                Log.w(TAG, "unkown position!");
                break;
            }
            return;
        }
        mCallback.setEmptyViewVisible(View.GONE);
    }

    private void setupActionBar2() {
    	Log.d(TAG, "setupActionBar2 in context: " + mContext);
        ActionBar actionBar = mContext.getActionBar();       
        View v = null;/*(ViewGroup) IpMessageResourceMananger.getInstance(mContext).inflateView(
                R.layout.conversation_list_actionbar2, null, false);*/  
        LayoutInflater pluginInflater = LayoutInflater.from(MediatekFactory.getApplicationContext());
        v =  pluginInflater.inflate(R.layout.conversation_list_actionbar2, null);
        actionBar.setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM, ActionBar.DISPLAY_SHOW_CUSTOM);
        actionBar.setCustomView(v,
                new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                        ActionBar.LayoutParams.WRAP_CONTENT,
                        Gravity.CENTER_VERTICAL | Gravity.LEFT));

        mCallback.updateUnreadView((TextView) v.findViewById(R.id.unread_conv_count));
        mSpinnerTextView = (TextView) v.findViewById(R.id.conversation_list_name);
        mConversationSpinner = (View) v.findViewById(R.id.conversation_list_spinner);
        View unreadConvCountLayout = v.findViewById(R.id.unread_layout);
        if(unreadConvCountLayout != null){
        unreadConvCountLayout.setVisibility(View.VISIBLE);
        }
        
        if (IpMmsConfig.isActivated(mContext)) {
            mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_all));
            mConversationSpinner.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mDropdownAdapter.getCount() > 0) {
                    	Log.d(TAG, "setupActionBar2 show() ");
                        mAccountDropdown.show();
                    }
                }
            });
        } else {
            // hide views if no plugin exist
            mSpinnerTextView.setVisibility(View.GONE);
            mConversationSpinner.setVisibility(View.GONE);
        }
    }

    // / M: add for ipmessage: spam, group chats {@
    private void initSpinnerListAdapter() {
    	Log.d(TAG, "initSpinnerListAdapter is ");
        mDropdownAdapter = new ArrayAdapter<String>(MediatekFactory.getApplicationContext(), R.layout.conversation_list_title_drop_down_item,
                R.id.drop_down_menu_text, new ArrayList<String>());
        Log.d(TAG, "initSpinnerListAdapter mDropdownAdapter is " + mDropdownAdapter);
        mDropdownAdapter = getDropDownMenuData(mDropdownAdapter, OPTION_CONVERSATION_LIST_ALL);
        setupActionBar2();

        mAccountDropdown = new AccountDropdownPopup(MediatekFactory.getApplicationContext());
        mAccountDropdown.setAdapter(mDropdownAdapter);
   }

    private ArrayAdapter<String> getDropDownMenuData(ArrayAdapter<String> adapter, int dropdownStatus) {
    	Log.d(TAG, "getDropDownMenuData entry adapter" + adapter);
        if (null == adapter) {
            return null;
        }
        mDropdownAdapter.clear();

        if (dropdownStatus != OPTION_CONVERSATION_LIST_ALL) {
            adapter.add(IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_all));
        }

        if (dropdownStatus != OPTION_CONVERSATION_LIST_GROUP_CHATS /*&& dropdownStatus != OPTION_CONVERSATION_LIST_XMS*/) {
            adapter.add(IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_group_chats));
        }

        if (dropdownStatus != OPTION_CONVERSATION_LIST_SPAM) {
            adapter.add(IpMessageResourceMananger.getInstance(mContext)
                .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_spam));
        }

        if (IpMessageServiceMananger.getInstance(mContext).getIntegrationMode() == IpMessageConsts.IntegrationMode.CONVERGED_INBOX) {
            if (dropdownStatus != OPTION_CONVERSATION_LIST_JOYN) {
                adapter.add(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_joyn));
            }

            if (dropdownStatus != OPTION_CONVERSATION_LIST_XMS) {
                adapter.add(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_xms));
            }
        }
        return adapter;
    }

    // Based on Spinner.DropdownPopup
    private class AccountDropdownPopup extends ListPopupWindow {
        public AccountDropdownPopup(Context context) {
            super(context);
            setAnchorView(mConversationSpinner);
            setModal(true);
            // Add for fix pop window width not match every device issue
            try {
                int width = IpMessageResourceMananger.getInstance(MediatekFactory.getApplicationContext()).getRcseResource().getDimensionPixelSize(R.dimen.popup_min_width);
            setWidth(width);
            } catch (NotFoundException e) {
                Log.d(TAG, "AccountDropdownPopup NF Exception()" );
                e.printStackTrace();
            }

            setOnItemClickListener(new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View v, int position, long id) {
                    onAccountSpinnerItemClicked(position);
                    dismiss();
                }
            });
        }

        @Override
        public void show() {
        	Log.d(TAG, "AccountDropdownPopup show()" );
            setInputMethodMode(ListPopupWindow.INPUT_METHOD_NOT_NEEDED);
            super.show();
            // List view is instantiated in super.show(), so we need to do this after...
            getListView().setChoiceMode(ListView.CHOICE_MODE_SINGLE);
        }
    }

    private void onAccountSpinnerItemClicked(int position) {
    	Log.d(TAG, "onAccountSpinnerItemClicked position is " + position);
        switch (sConversationListOption) {
        case OPTION_CONVERSATION_LIST_ALL:
            position++;
            break;
        case OPTION_CONVERSATION_LIST_GROUP_CHATS:
            if (position > 0) {
                position++;
            }
            break;
        case OPTION_CONVERSATION_LIST_SPAM:
            if (position > 1) {
                position++;
            }
            break;
        case OPTION_CONVERSATION_LIST_JOYN:
            if (position > 2) {
                position++;
            }
            break;
        case OPTION_CONVERSATION_LIST_XMS:
            if (position > 3) {
                position++;
            }
            break;
        default:
            break;
        }
        switch (position) {
            case OPTION_CONVERSATION_LIST_ALL:
                sConversationListOption = OPTION_CONVERSATION_LIST_ALL;
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_all));
                mDropdownAdapter = getDropDownMenuData(mDropdownAdapter, sConversationListOption);
                mDropdownAdapter.notifyDataSetChanged();
                break;
            case OPTION_CONVERSATION_LIST_GROUP_CHATS:
                sConversationListOption = OPTION_CONVERSATION_LIST_GROUP_CHATS;
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_group_chats));
                mDropdownAdapter = getDropDownMenuData(mDropdownAdapter, sConversationListOption);
                mDropdownAdapter.notifyDataSetChanged();
                break;
            case OPTION_CONVERSATION_LIST_SPAM:
                sConversationListOption = OPTION_CONVERSATION_LIST_SPAM;
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                    .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_spam));
                mDropdownAdapter = getDropDownMenuData(mDropdownAdapter, sConversationListOption);
                mDropdownAdapter.notifyDataSetChanged();
                break;
            case OPTION_CONVERSATION_LIST_JOYN:
                sConversationListOption = OPTION_CONVERSATION_LIST_JOYN;
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_joyn));
                mDropdownAdapter = getDropDownMenuData(mDropdownAdapter, sConversationListOption);
                mDropdownAdapter.notifyDataSetChanged();
                break;
            case OPTION_CONVERSATION_LIST_XMS:
                sConversationListOption = OPTION_CONVERSATION_LIST_XMS;
                mSpinnerTextView.setText(IpMessageResourceMananger.getInstance(mContext)
                        .getSingleString(IpMessageConsts.string.ipmsg_conversation_list_xms));
                mDropdownAdapter = getDropDownMenuData(mDropdownAdapter, sConversationListOption);
                mDropdownAdapter.notifyDataSetChanged();
                break;
            default:
                break;
        }
        mCallback.startQuery();
        mContext.invalidateOptionsMenu();
    }
    /// @}

    private void openIpMsgThread(final long threadId) {
        Intent intent = new Intent(RemoteActivities.CHAT_DETAILS_BY_THREAD_ID);
        intent.putExtra(RemoteActivities.KEY_THREAD_ID, threadId);
        intent.putExtra(RemoteActivities.KEY_NEED_NEW_TASK, false);
        IpMessageActivitiesManager.getInstance(mContext).startRemoteActivity(mContext, intent);
    }

    @Override
    public void notificationsReceived(Intent intent) {
        Log.d(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "convList.notificationsReceived(): start, intent = " + intent);
        String action = intent.getAction();
        Log.d(TAG, "IpMessageUtils.getActionTypeByAction(action) is " + IpMessageUtils.getActionTypeByAction(action));
        if (TextUtils.isEmpty(action)) {
            return;
        }
        switch (IpMessageUtils.getActionTypeByAction(action)) {
        /// M: add for ipmessage register toast @{
        case IpMessageUtils.IPMSG_REG_STATUS_ACTION:
            int regStatus = intent.getIntExtra(IpMessageConsts.RegStatus.REGSTATUS, 0);
            switch (regStatus) {
            case IpMessageConsts.RegStatus.REG_OVER:
                mContext.runOnUiThread(new Runnable() {
                    public void run() {
                        Toast.makeText(
                                mContext,
                                IpMessageResourceMananger.getInstance(mContext)
                                        .getSingleString(
                                                IpMessageConsts.string.ipmsg_nms_enable_success),
                                Toast.LENGTH_SHORT).show();
                    }
                });
                break;

            default:
                break;

            }
            break;
        /// @}
        case IpMessageUtils.IPMSG_ERROR_ACTION:
            // do nothing
            return;
        case IpMessageUtils.IPMSG_NEW_MESSAGE_ACTION:
//            public static final String IP_MESSAGE_KEY = "IpMessageKey";
            break;
        case IpMessageUtils.IPMSG_REFRESH_CONTACT_LIST_ACTION:
            break;
        case IpMessageUtils.IPMSG_REFRESH_GROUP_LIST_ACTION:
            break;
        case IpMessageUtils.IPMSG_SERCIVE_STATUS_ACTION:
//            public static final int ON  = 1;
//            public static final int OFF = 0;
            break;
        case IpMessageUtils.IPMSG_IM_STATUS_ACTION:
            /** M: show typing feature is off for performance issue now.
            String number = intent.getStringExtra(IpMessageConsts.NUMBER);
            int status = IpMessageUtils.getContactManager(this).getStatusByNumber(number);
            Log.d(TAG, "notificationsReceived(): IM status. number = " + number
                + ", status = " + status);
            if (mTypingCounter > 10) {
                return;
            }
            ContactList contact = new ContactList();
            contact.add(Contact.get(number, false));
            Conversation conv = Conversation.getCached(this, contact);
            if (conv == null) {
                Log.w(TAG, "the number is not in conversation cache!");
                return;
            }
            //long threadId = conv.getThreadId();
            //Log.d(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "number query threadId:" + threadId);
            switch (status) {
            case ContactStatus.TYPING:
                conv.setTyping(true);
                Log.d(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "start typing");
                mTypingCounter++;
                runOnUiThread(new Runnable() {
                    public void run() {
                        mListAdapter.notifyDataSetChanged();
                    }
                });
                break;
            case ContactStatus.STOP_TYPING:
                conv.setTyping(false);
                Log.d(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "stop typing");
                mTypingCounter--;
                runOnUiThread(new Runnable() {
                    public void run() {
                        mListAdapter.notifyDataSetChanged();
                    }
                });
                break;
            default:
                Log.d(IpMessageUtils.IPMSG_NOTIFICATION_TAG, "ignore a status:" + status);
                break;
            }
            */
            break;

        case IpMessageUtils.IPMSG_ACTIVATION_STATUS_ACTION:
            break;
        case IpMessageUtils.IPMSG_IP_MESSAGE_STATUS_ACTION:
            // handle this notification in MessageListItem
            break;
        case IpMessageUtils.IPMSG_DOWNLOAD_ATTACH_STATUS_ACTION:
            // handle this notification in MessageListItem
            break;
        case IpMessageUtils.IPMSG_SET_PROFILE_RESULT_ACTION:
            break;
        case IpMessageUtils.IPMSG_BACKUP_MSG_STATUS_ACTION:
            break;
        case IpMessageUtils.IPMSG_RESTORE_MSG_STATUS_ACTION:
            break;
        case IpMessageUtils.IPMSG_UPDATE_GROUP_INFO:
            int groupId = intent.getIntExtra(IpMessageConsts.UpdateGroup.GROUP_ID, -1);
            Log.d(TAG, "update group info,group id:" + groupId);
            String number = IpMessageContactManager.getInstance(mContext).getNumberByEngineId((short) groupId);
            Log.d(TAG, "group number:" + number);
            //String contact = mCallback.updateGroupInfo(number);
           /* if (contact != null) {
                contact.clearAvatar();
            }*/
            mCallback.notifyDataSetChanged();
            break;
        case IpMessageUtils.IPMSG_IPMESSAGE_CONTACT_UPDATE:
            /** M: ipmessage plugin send this event when
             *  1. system contact info is changed, we may need update group avatar
             *  2. self head icon is changed, we may need update group avatar
             *  3. a ipmessage head icon is updated,  need update avatar
             *  if a system contact avatar is updated, and it is in a group.
             *  we will not receive a IPMSG_UPDATE_GROUP_INFO event,
             *  so we need invalid the group avatar cache and re-fetch it.
             */
            mCallback.invalidateGroupCache();
            mCallback.notifyDataSetChanged();
            break;
        case IpMessageUtils.IPMSG_SIM_INFO_ACTION:
            /// M: for a special case, boot up enter mms quickly may be not get right status.
            if (IpMmsConfig.isActivated(mContext)) {
                /// M: init ipmessage view
                mContext.runOnUiThread(new Runnable() {
                    public void run() {
                        RcseConversation.setActivated(true);
                        initSpinnerListAdapter();
                        mContext.setTitle("");
                        mEmptyViewDefault.setVisibility(View.GONE);
//                        mCallback.setEmptyViewVisible(View.GONE);
                        mIpEmptyView.setVisibility(View.VISIBLE);
                        mListView.setEmptyView(mIpEmptyView);
                        mContext.invalidateOptionsMenu();
                    }
                });
            } else {
                Log.d(TAG, "normal message layout");
                mCallback.loadNormalLayout();
            }
            break;
        // Add for joyn
        case IpMessageUtils.IPMSG_DISABLE_SERVICE_STATUS_ACTION:
            mIsJoynChanged = true;
            break;
        default:
            break;
        }
    }

    // a receiver to moniter the network status.
    private class NetworkStateReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            boolean success = false;
            ConnectivityManager connManager =
                    (ConnectivityManager) mContext.getSystemService(mContext.CONNECTIVITY_SERVICE);
            State state = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI).getState();
            if (State.CONNECTED == state) {
                success = true;
            }
            if (!success) {
                state = connManager.getNetworkInfo(ConnectivityManager.TYPE_MOBILE).getState();
                if (State.CONNECTED == state) {
                    success = true;
                }
            }
            showInternetStatusBar(!success);
        }
    }

    public void showInternetStatusBar(boolean show) {
        if (show) {
            mNetworkStatusBar.setVisibility(View.VISIBLE);
        } else {
            mNetworkStatusBar.setVisibility(View.GONE);
        }
    }

    
}
