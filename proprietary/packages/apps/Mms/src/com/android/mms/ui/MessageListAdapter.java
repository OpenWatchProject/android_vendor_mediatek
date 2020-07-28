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

import android.content.ContentUris;
import android.content.Context;
import android.database.Cursor;
import android.database.StaleDataException;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.provider.BaseColumns;
import android.provider.Telephony;
import android.provider.Telephony.Mms;
import android.provider.Telephony.MmsSms;
import android.provider.Telephony.MmsSms.PendingMessages;
import android.provider.Telephony.Sms;
import android.provider.Telephony.Sms.Conversations;
import android.telephony.SmsManager;
import android.text.TextUtils;
import android.util.Log;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.ListView;

import com.android.mms.MmsApp;
import com.android.mms.R;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;
import com.mediatek.android.mms.pdu.MtkPduHeaders;

import com.mediatek.ipmsg.util.IpMessageUtils;
import com.mediatek.mms.callback.IColumnsMapCallback;
import com.mediatek.mms.ext.IOpMessageListAdapterExt;
import com.mediatek.mms.ipmessage.IIpColumnsMapExt;
import com.mediatek.mms.ipmessage.IIpMessageListAdapterExt;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.regex.Pattern;

import mediatek.telephony.MtkTelephony;

/**
 * The back-end data adapter of a message list.
 */
public class MessageListAdapter extends MessageCursorAdapter {
    private static final String TAG = "MessageListAdapter";

    public static final int MESSAGE_LIST_REFRASH                    = 0;
    public static final int MESSAGE_LIST_REFRASH_WITH_CLEAR_CACHE   = 1;
    private static final String IPMSG_TAG = "Mms/ipmsg/MessageListAdapter";
    private static final String TAG_DIVIDER = "Mms/divider";
    private static final boolean LOCAL_LOGV = false;
    private static final String SMS_IP_MESSAGE_ID = MtkTelephony.MtkSms.IPMSG_ID;
    private static final String SMS_SPAM = "spam";
    private Set<Long> mItemDividerSet;
    private HashMap<Long, Integer> mNotifyIndStatusMap;

    static final String[] PROJECTION = new String[] {
        // TODO: should move this symbol into com.android.mms.telephony.Telephony.
        // for both sms and mms.
        MmsSms.TYPE_DISCRIMINATOR_COLUMN,
        BaseColumns._ID,
        Conversations.THREAD_ID,
        Telephony.Sms.SUBSCRIPTION_ID,
        Sms.DATE,
        Mms.MESSAGE_SIZE,
        Sms.READ,
        Sms.LOCKED,
        // For SMS only
        Sms.ADDRESS,
        Sms.BODY,
        Sms.TYPE,
        Sms.STATUS,
        Sms.ERROR_CODE,
        Sms.SERVICE_CENTER,
        // For MMS only
        Mms.SUBJECT,
        Mms.SUBJECT_CHARSET,
        Mms.MESSAGE_TYPE,
        Mms.MESSAGE_BOX,
        Mms.DELIVERY_REPORT,
        Mms.READ_REPORT,
        PendingMessages.ERROR_TYPE,
        Mms.STATUS,
        /// M: ipmessage
        SMS_IP_MESSAGE_ID,
        SMS_SPAM,
    };

    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    // both sms and mms, 8 columns
    public static final int COLUMN_MSG_TYPE            = 0;
    public static final int COLUMN_ID                  = 1;
    public static final int COLUMN_THREAD_ID           = 2;
    public static final int COLUMN_SUBID               = 3;
    public static final int COLUMN_DATE                = 4;
    public static final int COLUMN_MESSAGE_SIZE        = 5;
    public static final int COLUMN_READ                = 6;
    public static final int COLUMN_LOCKED              = 7;
    // sms only, 6 columns
    public static final int COLUMN_SMS_ADDRESS         = 8;
    public static final int COLUMN_SMS_BODY            = 9;
    public static final int COLUMN_SMS_TYPE            = 10;
    public static final int COLUMN_SMS_STATUS          = 11;
    public static final int COLUMN_SMS_ERROR_CODE      = 12;
    public static final int COLUMN_SMS_SERVICE_CENTER  = 13;
    // mms only, 8 columns
    public static final int COLUMN_MMS_SUBJECT         = 14;
    public static final int COLUMN_MMS_SUBJECT_CHARSET = 15;
    public static final int COLUMN_MMS_MESSAGE_TYPE    = 16;
    public static final int COLUMN_MMS_MESSAGE_BOX     = 17;
    public static final int COLUMN_MMS_DELIVERY_REPORT = 18;
    public static final int COLUMN_MMS_READ_REPORT     = 19;
    public static final int COLUMN_MMS_ERROR_TYPE      = 20;
    public static final int COLUMN_MMS_STATUS          = 21;
    // ipmessage, 2 columns
    public static final int COLUMN_SMS_IP_MESSAGE_ID   = 22;
    public static final int COLUMN_SMS_SPAM            = 23;

    private static final int CACHE_SIZE         = 50;

    /// M:
    private static final int INCOMING_ITEM_TYPE = 0;
    private static final int OUTGOING_ITEM_TYPE = 1;

    protected LayoutInflater mInflater;
    private final MessageItemCache mMessageItemCache;
    private OnDataSetChangedListener mOnDataSetChangedListener;
    private Handler mMsgListItemHandler;
    private Pattern mHighlight;
    private Context mContext;
    /// M: google JB.MR1 patch, group mms
    private boolean mIsGroupConversation;

    /// M: @{
    public static final String CACHE_TAG = "Mms/MessageItemCache";

    private boolean mClearCacheFlag = true;
    static final int MSG_LIST_NEED_REFRASH   = 100;

    /// M: add for multi-delete
    public boolean mIsDeleteMode = false;
    private Map<Long, Boolean> mListItem;
    private Map<String, Boolean> mSimMsgListItem;

    ///M: add for adjust text size
    private float mTextSize = 0;
    /// @}

    private boolean mNeedHideView = false;

    private static int VIEW_TYPE_COUNT = 2;

    IIpMessageListAdapterExt mIpMessageListAdapter;

    public MessageListAdapter(
            Context context, Cursor c, ListView listView,
            boolean useDefaultColumnsMap, Pattern highlight) {
        super(context, c, FLAG_REGISTER_CONTENT_OBSERVER);
        mContext = context;
        mHighlight = highlight;

        mInflater = (LayoutInflater) context.getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
        mMessageItemCache = new MessageItemCache(CACHE_SIZE);

        /// M: @{
        mListItem = new LinkedHashMap<Long, Boolean>();
        mSimMsgListItem = new HashMap<String, Boolean>();
        mItemDividerSet = new HashSet<Long>();
        mNotifyIndStatusMap = new HashMap<Long, Integer>();
        /// @}

        listView.setRecyclerListener(new AbsListView.RecyclerListener() {
            @Override
            public void onMovedToScrapHeap(View view) {
                if (view instanceof MessageListItem) {
                    MessageListItem mli = (MessageListItem) view;
                    // Clear references to resources
                    mli.unbind();
                }
            }
        });

        // add for ipmessage
        mIpMessageListAdapter =
            IpMessageUtils.getIpMessagePlugin(mContext).getIpMessageListAdapter();
        mIpMessageListAdapter.onCreate(mContext, null);
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        MessageItem msgItem = null;
        if (view instanceof MessageListItem) {
            if(isSimMessge(cursor)) {
                try {
                    msgItem = new MessageItem(mContext, cursor, false);
                } catch (MmsException e) {
                    Log.e(TAG, "MmsException: ", e);
                } catch (StaleDataException stale) {
                    Log.e(TAG, "StaleDataException: ", stale);
                }
                MessageListItem mli = (MessageListItem) view;
                if (mIsDeleteMode) {
                    /// M: fix bug ALPS00432495, avoid IllegalArgumentException @{
                    if (cursor.getColumnIndex("index_on_icc") != -1) {
                        String msgIndex =
                                cursor.getString(cursor.getColumnIndexOrThrow("index_on_icc"));
                        String[] index = msgIndex.split(";");
                        if (mSimMsgListItem.get(index[0]) != null &&
                                mSimMsgListItem.get(index[0]) == true) {
                            msgItem.setSelectedState(true);
                        } else {
                            msgItem.setSelectedState(false);
                        }
                    }
                }
                int position = cursor.getPosition();
                mli.bind(msgItem, mIsGroupConversation, position, mIsDeleteMode);
                mli.setMsgListItemHandler(mMsgListItemHandler);
                mli.setMessageListItemAdapter(this);
                return;
            }

            mIpMessageListAdapter.onIpBindView(
                    ((MessageListItem) view).mIpMessageListItem, context, cursor);
            view.setVisibility(View.VISIBLE);
            String type = cursor.getString(COLUMN_MSG_TYPE);
            long msgId = cursor.getLong(COLUMN_ID);
            mNeedHideView = false;
            msgItem = getCachedMessageItem(type, msgId, cursor);
            if (!mIsScrolling || mIsDeleteMode) {
                if (LOCAL_LOGV) {
                    MmsLog.d(CACHE_TAG, "bindView():type=" + type + ", msgId=" + msgId);
                }
                if (msgItem != null) {
                    MessageListItem mli = (MessageListItem) view;
                    /// M: @{
                    /// M: for multi-delete
                    if (mIsDeleteMode) {
                        msgId = getKey(type, msgId);
                        if (mListItem.get(msgId) != null && mListItem.get(msgId) == true) {
                            msgItem.setSelectedState(true);
                        } else {
                            msgItem.setSelectedState(false);
                        }
                    }
                    /// @}
                    int position = cursor.getPosition();
                    /// M: google JB.MR1 patch, group mms
                    mli.bind(msgItem, mIsGroupConversation, position, mIsDeleteMode);
                    mli.setMsgListItemHandler(mMsgListItemHandler);
                    mli.setMessageListItemAdapter(this);
                } else {
                    MessageListItem mli = (MessageListItem) view;
                    if (mNeedHideView) {
                        mNeedHideView = false;
                        mli.hideAllView();
                        view.setVisibility(View.GONE);
                        return;
                    }
                    mli.bindDefault(null, cursor.getPosition() == cursor.getCount() - 1);
                }
            } else {
             // M:for ALPS01065027,just for compose sms messagelist in scrolling
                try {
                    MessageListItem mli = (MessageListItem) view;
                    if (msgItem == null) {
                        msgItem = new MessageItem(mContext, cursor);
                    }
                    mli.bindDefault(msgItem, cursor.getPosition() == cursor.getCount() - 1);
                } catch (MmsException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }
            /// M: add for text zoom
            if (mTextSize != 0) {
                MessageListItem mli = (MessageListItem) view;
                mli.setBodyTextSize(mTextSize);
            }
        }
    }

    public interface OnDataSetChangedListener {
        void onDataSetChanged(MessageListAdapter adapter);
        void onContentChanged(MessageListAdapter adapter);
    }

    public void setOnDataSetChangedListener(OnDataSetChangedListener l) {
        mOnDataSetChangedListener = l;
    }

    public void setMsgListItemHandler(Handler handler) {
        mMsgListItemHandler = handler;
    }

    /// M: google JB.MR1 patch, group mms
    public void setIsGroupConversation(boolean isGroup) {
        mIsGroupConversation = isGroup;
    }

    /// M: fix bug ALPS00488976, group mms
    public boolean isGroupConversation() {
        return mIsGroupConversation;
    }
    /// @}

 //   public void cancelBackgroundLoading() {
 //       mMessageItemCache.evictAll();   // causes entryRemoved to be called for each MessageItem
                                        // in the cache which causes us to cancel loading of
                                        // background pdu's and images.
 //   }

    @Override
    public void notifyDataSetChanged() {
        super.notifyDataSetChanged();
        if (LOCAL_LOGV) {
            Log.v(TAG, "MessageListAdapter.notifyDataSetChanged().");
        }

        if (mClearCacheFlag) {
       //     MmsLog.d(CACHE_TAG, "notifyDataSetChanged(), mMessageItemCache.evictAll");
       //     mMessageItemCache.evictAll();
        }
     //   mClearCacheFlag = true;

        if (mOnDataSetChangedListener != null) {
            mOnDataSetChangedListener.onDataSetChanged(this);
        }
    }

    @Override
    protected void onContentChanged() {
        if (getCursor() != null && !getCursor().isClosed()) {
            if (mOnDataSetChangedListener != null) {
                mOnDataSetChangedListener.onContentChanged(this);
            }
        }
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        // add for ipmessage
        View view = mIpMessageListAdapter.onIpNewView(mInflater, cursor, parent);
        if (view == null) {
            switch (getItemViewType(cursor)) {
                case INCOMING_ITEM_TYPE:
                    view = mInflater.inflate(R.layout.message_list_item_recv, parent, false);
                    break;
                case OUTGOING_ITEM_TYPE:
                default:
                    view = mInflater.inflate(R.layout.message_list_item_send, parent, false);
            }
        }
        return new MessageListItem(context, view);
    }


    public MessageItem getCachedMessageItem(String type, long msgId, final Cursor c) {
        if (!isCursorValid(c)) {
            MmsLog.e(CACHE_TAG, "getCachedMessageItem(): inValid cursor");
            return null;
        }

        final long key = getKey(type, msgId);
        MessageItem item = mMessageItemCache.get(key);

        if (item == null) {
            /// M: add for ipmessage, add isDrawTimeDivider status to MessageItem.
            final boolean isDrawTimeDivider = isMessageItemShowTimeDivider(c);
            if (type.equals("mms")) {
                final int boxId = c.getInt(COLUMN_MMS_MESSAGE_BOX);
                final int messageType = c.getInt(COLUMN_MMS_MESSAGE_TYPE);
                final int simId = c.getInt(COLUMN_SUBID);
                final int errorType = c.getInt(COLUMN_MMS_ERROR_TYPE);
                final int locked = c.getInt(COLUMN_LOCKED);
                final int charset = c.getInt(COLUMN_MMS_SUBJECT_CHARSET);
                final long mMsgId = msgId;
                final String mmsType = type;
                final String subject = c.getString(COLUMN_MMS_SUBJECT);
              //  final String serviceCenter = c.getString(ColumnsIndex.mColumnSmsServiceCenter);
                final String deliveryReport = c.getString(COLUMN_MMS_DELIVERY_REPORT);
                final String readReport = c.getString(COLUMN_MMS_READ_REPORT);
                final Pattern highlight = mHighlight;
                final long indDate = c.getLong(COLUMN_DATE);

                final int mmsSize = c.getInt(COLUMN_MESSAGE_SIZE);

                /// M: fix bug ALPS00406912
                final int mmsStatus;
                if (getNotifIndStatus(msgId) != -1) {
                    mmsStatus = getNotifIndStatus(msgId);
                } else {
                    mmsStatus = c.getInt(COLUMN_MMS_STATUS);
                }

                final Runnable r = new Runnable() {
                    @Override
                    public void run() {
                        final Message msg =
                            Message.obtain(mMsgListItemHandler, MSG_LIST_NEED_REFRASH,
                            MESSAGE_LIST_REFRASH, 0);
                        msg.sendToTarget();
                    }
                };
                final Object object = new Object();
                pushTask(key, new Runnable() {
                    @Override
                    public void run() {
                        try {
                            MessageItem backgroundItem = mMessageItemCache.get(key);
                            if (backgroundItem == null) {
                                backgroundItem = new MessageItem(mContext, boxId, messageType,
                                        simId, errorType, locked, charset, mMsgId, mmsType,
                                        subject,
                                        deliveryReport, readReport,
                                        highlight, isDrawTimeDivider, indDate, mmsStatus,
                                        mmsSize);
                                mMessageItemCache.put(key, backgroundItem);
                                mMsgListItemHandler.postDelayed(r, 200);
                            }
                            synchronized (object) {
                                object.notifyAll();
                            }
                        } catch (MmsException e) {
                            Log.e(TAG, "getCachedMessageItem: ", e);
                            if (messageType == MtkPduHeaders.MESSAGE_TYPE_NOTIFICATION_IND) {
                                mNeedHideView = true;
                            }
                            synchronized (object) {
                                object.notifyAll();
                            }
                        }
                    }
                });

                synchronized (object) {
                    try {
                        /// M: Fix ALPS00391886, avoid waiting too
                        /// long time when many uncached messages
                        int waitTime = 10;
                        /// @}
                        object.wait(waitTime);
                    } catch (InterruptedException ex) {
                        MmsLog.e(TAG, "wait has been intrrupted", ex);
                    }
                }
                item = mMessageItemCache.get(key);
                if (item != null) {
                    if (LOCAL_LOGV) {
                        MmsLog.d(CACHE_TAG, "getCachedMessageItem(): get item during wait.");
                    }
                    mMsgListItemHandler.removeCallbacks(r);
                }
            } else {
                try {
                    item = new MessageItem(mContext, type, c,
                            mHighlight, isDrawTimeDivider, 0L, mIsGroupConversation);
                    mMessageItemCache.put(key, item);
                } catch (MmsException e) {
                    Log.e(TAG, "getCachedMessageItem: ", e);
                } catch (StaleDataException stale) {
                    Log.e(TAG, "getCachedMessageItem: ", stale);
                }
            }
        } else if (type.equals("mms")) {
            if (item.updateMmsItem(c)) {
                // use the current messageItem
            } else {
                if (LOCAL_LOGV) {
                    MmsLog.d(CACHE_TAG, "getCachedMessageItem(): update the item: " +
                            item.toString());
                }
                mMessageItemCache.remove(key);
                item = getCachedMessageItem(type, msgId, c);
            }
        } else {
            // sms item
            if (item.updateSmsItem(c)) {
                // use the current messageItem
            } else {
                if (LOCAL_LOGV) {
                    MmsLog.d(CACHE_TAG, "getCachedMessageItem(): update the item: " +
                            item.toString());
                }
                mMessageItemCache.remove(key);
                item = getCachedMessageItem(type, msgId, c);
            }
        }

        if (null != item) {
            item.mIsDrawOnlineDivider = !TextUtils.isEmpty(mOnlineDividerString)
                    && isMessageItemShowOnlineDivider(c);
            item.mOnlineString = item.mIsDrawOnlineDivider ? mOnlineDividerString : "";
        }
        return item;
    }

    private boolean isCursorValid(Cursor cursor) {
        // Check whether the cursor is valid or not.
        if (cursor == null || cursor.isClosed()
                || cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        return true;
    }

    public static long getKey(String type, long id) {
        //add for ipMessage
        long key = IpMessageUtils.getKey(type, id);
        if (key != 0) {
            return key;
        }

        if (type.equals("mms")) {
            return -id;
        } else {
            return id;
        }
    }

    @Override
    public boolean areAllItemsEnabled() {
        return true;
    }

    @Override
    public void changeCursor(Cursor cursor) {
        clearNotifIndStatus();
        mIpMessageListAdapter.changeCursor(cursor);
        super.changeCursor(cursor);
    }

    /* MessageListAdapter says that it contains four types of views. Really, it just contains
     * a single type, a MessageListItem. Depending upon whether the message is an incoming or
     * outgoing message, the avatar and text and other items are laid out either left or right
     * justified. That works fine for everything but the message text. When views are recycled,
     * there's a greater than zero chance that the right-justified text on outgoing messages
     * will remain left-justified. The best solution at this point is to tell the adapter we've
     * got two different types of views. That way we won't recycle views between the two types.
     * @see android.widget.BaseAdapter#getViewTypeCount()
     */
    @Override
    public int getViewTypeCount() {
        // add for ipmessage
        int viewCount = mIpMessageListAdapter.getIpViewTypeCount();
        if (viewCount != -1) {
            return viewCount;
        }
        return VIEW_TYPE_COUNT; // 3; // Incoming, outgoing messages and dividers
    }

    @Override
    public int getItemViewType(int position) {
        Cursor cursor = (Cursor) getItem(position);
        // / add for ipmessage
        int itemType = mIpMessageListAdapter.getIpItemViewType(cursor);
        if (itemType != -1) {
            return itemType;
        }

        switch (getItemViewType(cursor)) {
        case INCOMING_ITEM_TYPE:
            return INCOMING_ITEM_TYPE;
        case OUTGOING_ITEM_TYPE:
            return OUTGOING_ITEM_TYPE;
        default:
            MmsLog.d(IPMSG_TAG, "getItemViewType(): item type = INCOMING_ITEM_TYPE (default)");
            return INCOMING_ITEM_TYPE;
        }
    }

    private int getItemViewType(Cursor cursor) {
        String type = getMsgType(cursor);

        int boxId;
        if ("sms".equals(type)) {
            /// M: check sim sms and set box id
            long status = getSmsStatus(cursor);
            boolean isSimMsg = false;
            if (status == SmsManager.STATUS_ON_ICC_SENT
                    || status == SmsManager.STATUS_ON_ICC_UNSENT) {
                isSimMsg = true;
                boxId = Sms.MESSAGE_TYPE_SENT;
            } else if (status == SmsManager.STATUS_ON_ICC_READ
                    || status == SmsManager.STATUS_ON_ICC_UNREAD) {
                isSimMsg = true;
                boxId = Sms.MESSAGE_TYPE_INBOX;
            } else {
                boxId = cursor.getInt(COLUMN_SMS_TYPE);
            }
        } else {
            boxId = cursor.getInt(COLUMN_MMS_MESSAGE_BOX);
        }
        return boxId == Mms.MESSAGE_BOX_INBOX ? INCOMING_ITEM_TYPE : OUTGOING_ITEM_TYPE;
    }

    public Cursor getCursorForItem(MessageItem item) {
        Cursor cursor = getCursor();
        if (isCursorValid(cursor)) {
            if (cursor.moveToFirst()) {
                do {
                    long id = cursor.getLong(mRowIDColumn);
                    if (id == item.mMsgId) {
                        return cursor;
                    }
                } while (cursor.moveToNext());
            }
        }
        return null;
    }

    private static class MessageItemCache extends LruCache<Long, MessageItem> {
        public MessageItemCache(int maxSize) {
            super(maxSize);
        }

        @Override
        protected void entryRemoved(boolean evicted, Long key,
                MessageItem oldValue, MessageItem newValue) {
            oldValue.cancelPduLoading();
        }
    }

    public void saveNotifIndStatus(long index, int Status) {
        mNotifyIndStatusMap.put(index, Status);
    }

    public int getNotifIndStatus(long index) {
        Integer result = mNotifyIndStatusMap.get(index);
        if (result == null) {
            return -1;
        } else {
            return mNotifyIndStatusMap.get(index);
        }
    }

    public void clearNotifIndStatus() {
        mNotifyIndStatusMap.clear();
    }

    /// M: @{
    /// M: add for multi-delete
    public void changeSelectedState(long listId) {

        MmsLog.d(TAG, "listId =" + listId);
        if (mListItem == null) {
            MmsLog.e(TAG, "mListItem is null");
            return;
        }
        if (mListItem.get(listId) == null || mListItem.get(listId) == false) {
            mListItem.put(listId, true);
        } else {
            mListItem.put(listId, false);
        }

    }

    public void changeSelectedState(String listId) {
        if (mSimMsgListItem == null) {
            return;
        }
        if (mSimMsgListItem.get(listId) == null || mSimMsgListItem.get(listId) == false) {
            mSimMsgListItem.put(listId, true);
        } else {
            mSimMsgListItem.put(listId, false);
        }

    }

    public  Map<Long, Boolean> getItemList() {
        return mListItem;

    }

    public  Map<String, Boolean> getSimMsgItemList() {
        return mSimMsgListItem;

    }

    public Uri getMessageUri(long messageId) {
        Uri messageUri = null;
        if (messageId > 0) {
            messageUri = ContentUris.withAppendedId(Sms.CONTENT_URI, messageId);
        } else {
            messageUri = ContentUris.withAppendedId(Mms.CONTENT_URI, -messageId);
        }
        return messageUri;
    }

    public void initListMap(Cursor cursor, boolean value) {
        MmsLog.e(TAG, "initListMap value:" + value);
        if (value == false) {
            clearList();
            return;
        }
        if (cursor != null) {
            long itemId = 0;
            String type;
            long msgId = 0L;
            long status = 0L;
            boolean isSimMsg = false;
            int position = cursor.getPosition();
            cursor.moveToPosition(-1);
            while (cursor.moveToNext()) {
                status = getSmsStatus(cursor);
                if (status == SmsManager.STATUS_ON_ICC_READ
                        || status == SmsManager.STATUS_ON_ICC_UNREAD
                        || status == SmsManager.STATUS_ON_ICC_SENT
                        || status == SmsManager.STATUS_ON_ICC_UNSENT) {
                    isSimMsg = true;
                }
                /// M: fix bug ALPS00432495, avoid IllegalArgumentException @{
                if (isSimMsg && cursor.getColumnIndex("index_on_icc") != -1) {
                    String msgIndex = cursor.getString(
                            cursor.getColumnIndexOrThrow("index_on_icc"));
                    String[] index = msgIndex.split(";");
//                    for (int n = 0; n < index.length; n++) {
                          mSimMsgListItem.put(index[0], value);
//                    }
                } else {
                    type = getMsgType(cursor);
                    msgId = cursor.getLong(COLUMN_ID);
                    itemId = getKey(type, msgId);

                    mListItem.put(itemId, value);
                }
            }
            cursor.moveToPosition(position);
        }
    }

    public void setItemsValue(boolean value, long[] keyArray) {
        Iterator iter = mListItem.entrySet().iterator();
        /// M: keyArray = null means set the all item
        if (keyArray == null) {
            initListMap(getCursor(), value);
        } else {
            for (int i = 0; i < keyArray.length; i++) {
                mListItem.put(keyArray[i], value);
            }
        }
    }

    public void setSimItemsValue(boolean value, long[] keyArray) {
        Iterator iter = mSimMsgListItem.entrySet().iterator();
        /// M: keyArray = null means set the all item
        if (keyArray == null) {
            initListMap(getCursor(), value);
        }
    }

    public void clearList() {
        if (mListItem != null) {
            mListItem.clear();
        }
        if (mSimMsgListItem != null) {
            mSimMsgListItem.clear();
        }
    }

    public int getSelectedNumber() {
        int number = 0;
        if (mListItem != null) {
            Iterator iter = mListItem.entrySet().iterator();
            while (iter.hasNext()) {
                @SuppressWarnings("unchecked")
                Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
                if (entry.getValue()) {
                    number++;
                }
            }
        }
        if (mSimMsgListItem != null) {
            Iterator simMsgIter = mSimMsgListItem.entrySet().iterator();
            while (simMsgIter.hasNext()) {
                @SuppressWarnings("unchecked")
                Map.Entry<String, Boolean> entry = (Entry<String, Boolean>) simMsgIter.next();
                if (entry.getValue()) {
                    number++;
                }
            }
        }
        return number;
    }

    ///M: add for adjust text size
    public void setTextSize(float size) {
        mTextSize = size;
    }

    private static class TaskStack {
        boolean mThreadOver = false;
        Thread mWorkerThread;
        private final ExecutorService mLoadingPool;
        private final HashMap<Long, Runnable> mThingsToLoad;

        public TaskStack() {
            mThingsToLoad = new HashMap<Long, Runnable>();
            mLoadingPool = Executors.newFixedThreadPool(4);
            mWorkerThread = new Thread(new Runnable() {
                public void run() {
                    while (!mThreadOver) {
                        Runnable r = null;
                        synchronized (mThingsToLoad) {
                            if (mThingsToLoad.size() == 0) {
                                try {
                                    mThingsToLoad.wait();
                                } catch (InterruptedException ex) {
                                    MmsLog.e(TAG, ex.getMessage());
                                }
                            }
                            if (mThingsToLoad.size() > 0) {
                                for (long key : mThingsToLoad.keySet()) {
                                    r = mThingsToLoad.remove(key);
                                    break;
                                }
                            }
                        }
                        if (r != null) {
                            mLoadingPool.execute(r);
                        }
                    }
                }
            });
            mWorkerThread.start();
        }

        public void push(long key, Runnable r) {
            synchronized (mThingsToLoad) {
                mThingsToLoad.put(key, r);
                mThingsToLoad.notify();
            }
        }

        public void destroy() {
            synchronized (mThingsToLoad) {
                mThreadOver = true;
                mThingsToLoad.clear();
                mThingsToLoad.notify();
                mLoadingPool.shutdown();
            }
        }
    }

    private final TaskStack mTaskQueue = new TaskStack();
    public void pushTask(long key, Runnable r) {
        mTaskQueue.push(key, r);
    }

    public void destroyTaskStack() {
        if (mTaskQueue != null) {
            mTaskQueue.destroy();
        }
    }

    public void setClearCacheFlag(boolean clearCacheFlag) {
        mClearCacheFlag = clearCacheFlag;
    }

    private boolean isMessageItemShowTimeDivider(Cursor cursor) {
        if (null == cursor) {
            return false;
        }
        if (cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        if (cursor.isFirst() || cursor.getCount() == 1) {
            return true;
        }
        long currentMessageTime = getMessageTimeByCursor(cursor);
        cursor.moveToPrevious();
        long previousMessageTime = getMessageTimeByCursor(cursor);
        cursor.moveToNext();
        return MessageUtils.shouldShowTimeDivider(previousMessageTime, currentMessageTime);
    }

    private long getMessageTimeByCursor(Cursor cursor) {
        String type = cursor.getString(COLUMN_MSG_TYPE);
        long time = mIpMessageListAdapter.getMessageTimeByCursor(type, cursor);
        if (time != 0) {
            return time;
        }
        if (type.equals("mms")) {
            return cursor.getLong(COLUMN_DATE) * 1000L;
        } else {
            return cursor.getLong(COLUMN_DATE) * 1L;
        }
    }

    private boolean isMessageItemShowOnlineDivider(Cursor cursor) {
        if (null == cursor) {
            return false;
        }
        if (cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        if (cursor.getCount() == 1) {
            return true;
        }
        long currentTime = getMessageTimeByCursor(cursor);
        if (cursor.isLast()) {
            return currentTime < mOnlineDividerTime;
        }
        cursor.moveToNext();
        long nextMessageTime = getMessageTimeByCursor(cursor);
        cursor.moveToPrevious();
        return (currentTime < mOnlineDividerTime) && (nextMessageTime > mOnlineDividerTime);
    }

    private long mOnlineDividerTime = System.currentTimeMillis();
    private String mOnlineDividerString = "";

    public void updateOnlineDividerTime() {
        mOnlineDividerTime = System.currentTimeMillis();
    }

    public void setOnlineDividerString(String onlineDividerString) {
        this.mOnlineDividerString = onlineDividerString;
    }
    /// @}

    /// M: For ComposeMessageActivity to check listener@{
    public OnDataSetChangedListener getOnDataSetChangedListener() {
        return mOnDataSetChangedListener;
    }
    /// @}

    @Override
    public boolean isEnabled(int position) {
        return mIpMessageListAdapter.isEnabled(getCursor(), position);
    }

    public static String getMsgType(Cursor c) {
        int index = c.getColumnIndex(MmsSms.TYPE_DISCRIMINATOR_COLUMN);
        if (index != -1) {
            return c.getString(index);
        }
        return null;
    }

    public static long getSmsStatus(Cursor c) {
        int index = c.getColumnIndex(Sms.STATUS);
        if (index != -1) {
            return c.getLong(index);
        }
        return -1;
    }

    public static boolean isSimMessge(Cursor c) {
        int index = c.getColumnIndex(Sms.STATUS);
        if (index != -1) {
            long status = c.getLong(index);
            if (status == SmsManager.STATUS_ON_ICC_SENT
                || status == SmsManager.STATUS_ON_ICC_UNSENT
                || status == SmsManager.STATUS_ON_ICC_READ
                || status == SmsManager.STATUS_ON_ICC_UNREAD) {
                return true;
            }
        }
        return false;
    }
}
