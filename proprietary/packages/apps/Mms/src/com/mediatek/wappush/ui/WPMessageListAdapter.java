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

import android.content.Context;
import android.database.Cursor;
import android.os.Handler;
import android.provider.BaseColumns;
import android.util.Config;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnCreateContextMenuListener;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.CursorAdapter;
import android.widget.ListView;

import com.android.mms.R;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsLog;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.regex.Pattern;

import mediatek.telephony.MtkTelephony.WapPush;

/** M:
 * WPMessageListAdapter
 */
public class WPMessageListAdapter extends CursorAdapter {
    private static final String WP_TAG = "Mms/WapPush";
    private static final boolean DEBUG = false;
    private static final boolean LOCAL_LOGV = Config.LOGV && DEBUG;
    //add for multi-delete
    // public boolean mIsDeleteMode = false;
    private Map<Long, Boolean> mListItem;
    private List<WPMessageListItem> mMessageListItem = new ArrayList<WPMessageListItem>();

    static final String[] WP_PROJECTION = new String[] {
        // TODO: should move this symbol into com.android.mms.telephony.Telephony.
        BaseColumns._ID,
        WapPush.THREAD_ID,
        WapPush.ADDR,
        WapPush.SERVICE_ADDR,
        WapPush.READ,
        WapPush.DATE,
        WapPush.TYPE,
        WapPush.SIID,
        WapPush.URL,
        WapPush.CREATE,
        WapPush.EXPIRATION,
        WapPush.ACTION,
        WapPush.TEXT,
        WapPush.SUBSCRIPTION_ID,
        WapPush.LOCKED,
        WapPush.ERROR
    };

    // The indexes of the default columns which must be consistent
    // with above PROJECTION.
    static final int COLUMN_ID                  = 0;
    static final int COLUMN_THREAD_ID           = 1;
    static final int COLUMN_WPMS_ADDR           = 2;
    static final int COLUMN_WPMS_SERVICE_ADDR   = 3;
    static final int COLUMN_WPMS_READ           = 4;
    static final int COLUMN_WPMS_DATE           = 5;
    static final int COLUMN_WPMS_TYPE           = 6;
    static final int COLUMN_WPMS_SIID           = 7;
    static final int COLUMN_WPMS_URL            = 8;
    static final int COLUMN_WPMS_CREATE         = 9;
    static final int COLUMN_WPMS_EXPIRATION     = 10;
    static final int COLUMN_WPMS_ACTION         = 11;
    static final int COLUMN_WPMS_TEXT           = 12;
    static final int COLUMN_WPMS_SUBID          = 13;
    static final int COLUMN_WPMS_LOCKED         = 14;
    static final int COLUMN_WPMS_ERROR          = 15;

    private static final int CACHE_SIZE         = 50;

    protected LayoutInflater mInflater;
    private final LruCache<Long, WPMessageItem> mMessageItemCache;
    private final WPColumnsMap mColumnsMap;
    private OnDataSetChangedListener mOnDataSetChangedListener;
    private Handler mMsgListItemHandler;
    private Pattern mHighlight;
    private Context mContext;

    private float mTextSize = 0;

    public WPMessageListAdapter(
            Context context, Cursor c, ListView listView,
            boolean useDefaultColumnsMap, Pattern highlight) {
        super(context, c, FLAG_REGISTER_CONTENT_OBSERVER);
        mContext = context;
        mHighlight = highlight;

        mInflater = (LayoutInflater) context.getSystemService(
                Context.LAYOUT_INFLATER_SERVICE);
        mMessageItemCache = new LruCache<Long, WPMessageItem>(CACHE_SIZE);

        mListItem = new HashMap<Long, Boolean>();

        if (useDefaultColumnsMap) {
            mColumnsMap = new WPColumnsMap();
        } else {
            mColumnsMap = new WPColumnsMap(c);
        }

        listView.setRecyclerListener(new AbsListView.RecyclerListener() {
            @Override
            public void onMovedToScrapHeap(View view) {
                if (view instanceof WPMessageListItem) {
                    WPMessageListItem mli = (WPMessageListItem) view;
                    // Clear references to resources
                    mli.unbind();
                }
            }
        });
    }

    private OnCreateContextMenuListener mItemOnCreateContextMenuListener;

    public void setItemOnCreateContextMenuListener(OnCreateContextMenuListener l) {
        mItemOnCreateContextMenuListener = l;
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        if (view instanceof WPMessageListItem) {
            int type = cursor.getInt(mColumnsMap.mColumnWpmsType);
            long msgId = cursor.getLong(mColumnsMap.mColumnMsgId);

            WPMessageItem msgItem = getCachedMessageItem(type, msgId, cursor);
            if (msgItem != null) {
                WPMessageListItem mli = (WPMessageListItem) view;
                mMessageListItem.add(mli);
                mli.bind(msgItem, cursor.getPosition() == cursor.getCount() - 1);
                mli.setMsgListItemHandler(mMsgListItemHandler);

                /// M: add for text zoom
                if (mTextSize != 0) {
                    mli.setTextSize(mTextSize);
                }
            }
        }
    }

    public void clearAllContactListeners() {
        if (!mMessageListItem.isEmpty()) {
            for (WPMessageListItem item : mMessageListItem) {
                if (item != null) {
                    item.unbind();
                    item = null;
                }
            }
            mMessageListItem.clear();
        }
    }

    public interface OnDataSetChangedListener {
        void onDataSetChanged(WPMessageListAdapter adapter);
        void onContentChanged(WPMessageListAdapter adapter);
    }

    public void setOnDataSetChangedListener(OnDataSetChangedListener l) {
        mOnDataSetChangedListener = l;
    }

    public void setMsgListItemHandler(Handler handler) {
        mMsgListItemHandler = handler;
    }

    //add for multi-delete
    public void changeSelectedState(long listId) {
        mListItem.put(listId, !mListItem.get(listId));

    }
    public  Map<Long, Boolean> getItemList() {
        return mListItem;

    }

    public void initListMap(Cursor cursor) {
        if (cursor != null) {
            long itemId = 0;
            while (cursor.moveToNext()) {
                itemId = cursor.getLong(mColumnsMap.mColumnMsgId);
                if (mListItem.get(itemId) == null) {
                    mListItem.put(itemId, false);
                }
            }
        }
    }

    public void setItemsValue(boolean value, long[] keyArray) {
        Iterator iter = mListItem.entrySet().iterator();
        //keyArray = null means set the all item
        if (keyArray == null) {
            while (iter.hasNext()) {
                @SuppressWarnings("unchecked")
                Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
                entry.setValue(value);
            }
        } else {
            for (int i = 0; i < keyArray.length; i++) {
                mListItem.put(keyArray[i], value);
            }
        }
    }

    public void clearList() {
        setItemsValue(false, null);
    }

    public int getSelectedNumber() {
        Iterator iter = mListItem.entrySet().iterator();
        int number = 0;
        while (iter.hasNext()) {
            @SuppressWarnings("unchecked")
            Map.Entry<Long, Boolean> entry = (Entry<Long, Boolean>) iter.next();
            if (entry.getValue()) {
                number++;
            }
        }
        return number;
    }

    public void notifyImageLoaded(String address) {
    }

    @Override
    public void notifyDataSetChanged() {
        super.notifyDataSetChanged();
        if (LOCAL_LOGV) {
            MmsLog.v(WP_TAG, "WPMessageListAdapter.notifyDataSetChanged().");
        }

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
        return mInflater.inflate(R.layout.wp_message_list_item, parent, false);
    }

    public WPMessageItem getCachedMessageItem(int type, long msgId, Cursor c) {
        WPMessageItem item = mMessageItemCache.get(msgId);
        if (c != null && isCursorValid(c)) {
            if (item == null) {
                try {
                    item = new WPMessageItem(mContext, type, c, mColumnsMap, mHighlight);
                    mMessageItemCache.put(msgId, item);
                } catch (Exception e) {
                    MmsLog.e(WP_TAG, "WPMessageListAdapter: " + e.getMessage());
                }
            } else {
                item.mAction = c.getInt(COLUMN_WPMS_ACTION);
                item.mDate = c.getLong(COLUMN_WPMS_DATE);
                item.mTimestamp = MessageUtils.formatTimeStampString(mContext, item.mDate);
                item.mExpirationLong = c.getLong(COLUMN_WPMS_EXPIRATION) * 1000;
                item.mText = c.getString(COLUMN_WPMS_TEXT);
                item.mURL = c.getString(COLUMN_WPMS_URL);
                item.setIsUnread(c.getInt(COLUMN_WPMS_READ) == 0 ? true : false);
                item.mIsExpired = c.getInt(COLUMN_WPMS_ERROR);
            }
        }
        return item;
    }

    private boolean isCursorValid(Cursor cursor) {
        // Check whether the cursor is valid or not.
        if (cursor.isClosed() || cursor.isBeforeFirst() || cursor.isAfterLast()) {
            return false;
        }
        return true;
    }

    @Override
    public boolean areAllItemsEnabled() {
        return true;
    }

    /* MessageListAdapter says that it contains two types of views. Really, it just contains
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
        return 1;   // Incoming and outgoing messages
    }

    @Override
    public int getItemViewType(int position) {
        Cursor cursor = (Cursor) getItem(position);
        return getItemViewType(cursor);
    }

    private int getItemViewType(Cursor cursor) {
        return 0;
    }

    public static class WPColumnsMap {
        public int mColumnMsgId;
        public int mColumnWpmsThreadId;
        public int mColumnWpmsAddr;
        public int mColumnWpmsServiceAddr;
        public int mColumnWpmsRead;
        public int mColumnWpmsDate;
        public int mColumnWpmsType;
        public int mColumnWpmsSiid;
        public int mColumnWpmsURL;
        public int mColumnWpmsCreate;
        public int mColumnWpmsExpiration;
        public int mColumnWpmsAction;
        public int mColumnWpmsText;
        public int mColumnWpmsSubId;
        public int mColumnWpmsLocked;
        public int mColumnWpmsError;

        public WPColumnsMap() {
            mColumnMsgId              = COLUMN_ID;
            mColumnWpmsThreadId       = COLUMN_THREAD_ID;
            mColumnWpmsAddr           = COLUMN_WPMS_ADDR;
            mColumnWpmsServiceAddr    = COLUMN_WPMS_SERVICE_ADDR;
            mColumnWpmsRead           = COLUMN_WPMS_READ;
            mColumnWpmsDate           = COLUMN_WPMS_DATE;
            mColumnWpmsType           = COLUMN_WPMS_TYPE;
            mColumnWpmsSiid           = COLUMN_WPMS_SIID;
            mColumnWpmsURL            = COLUMN_WPMS_URL;
            mColumnWpmsCreate         = COLUMN_WPMS_CREATE;
            mColumnWpmsExpiration     = COLUMN_WPMS_EXPIRATION;
            mColumnWpmsAction         = COLUMN_WPMS_ACTION;
            mColumnWpmsText           = COLUMN_WPMS_TEXT;
            mColumnWpmsSubId          = COLUMN_WPMS_SUBID;
            mColumnWpmsLocked         = COLUMN_WPMS_LOCKED;
            mColumnWpmsError          = COLUMN_WPMS_ERROR;
        }

        public WPColumnsMap(Cursor cursor) {
            // Ignore all 'not found' exceptions since the custom columns
            // may be just a subset of the default columns.
            try {
                mColumnMsgId = cursor.getColumnIndexOrThrow(BaseColumns._ID);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsThreadId = cursor.getColumnIndexOrThrow(WapPush.THREAD_ID);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsAddr = cursor.getColumnIndexOrThrow(WapPush.ADDR);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsServiceAddr = cursor.getColumnIndexOrThrow(WapPush.SERVICE_ADDR);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsRead = cursor.getColumnIndexOrThrow(WapPush.READ);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsDate = cursor.getColumnIndexOrThrow(WapPush.DATE);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsType = cursor.getColumnIndexOrThrow(WapPush.TYPE);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsSiid = cursor.getColumnIndexOrThrow(WapPush.SIID);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsURL = cursor.getColumnIndexOrThrow(WapPush.URL);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsCreate = cursor.getColumnIndexOrThrow(WapPush.CREATE);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsExpiration = cursor.getColumnIndexOrThrow(WapPush.EXPIRATION);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsAction = cursor.getColumnIndexOrThrow(WapPush.ACTION);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsText = cursor.getColumnIndexOrThrow(WapPush.TEXT);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsSubId = cursor.getColumnIndexOrThrow(WapPush.SUBSCRIPTION_ID);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsLocked = cursor.getColumnIndexOrThrow(WapPush.LOCKED);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }

            try {
                mColumnWpmsError = cursor.getColumnIndexOrThrow(WapPush.ERROR);
            } catch (IllegalArgumentException e) {
                MmsLog.e("colsMap", e.getMessage());
            }
        }
    }

    ///M: add for adjust text size
    public void setTextSize(float size) {
        mTextSize = size;
    }

    public boolean isAllSelected() {
        int count = getCount();
        boolean isAllChecked = true;
        for (int i = 0; i < count; i++) {
            Cursor cursor = (Cursor) getItem(i);
            int type = cursor.getInt(mColumnsMap.mColumnWpmsType);
            long msgId = cursor.getLong(mColumnsMap.mColumnMsgId);
            WPMessageItem msgItem = getCachedMessageItem(type, msgId, cursor);
            if (!msgItem.isChecked()) {
                isAllChecked = false;
                break;
            }
        }
        return isAllChecked;
    }

    public void uncheckSelect(HashSet<Integer> idSet) {
        if (idSet != null && idSet.size() > 0) {
            Iterator iterator = idSet.iterator();
            while (iterator.hasNext()) {
                int index = (Integer) iterator.next();
                Object obj = getItem(index);
                if (obj != null) {
                    Cursor cursor = (Cursor) obj;
                    int type = cursor.getInt(mColumnsMap.mColumnWpmsType);
                    long msgId = cursor.getLong(mColumnsMap.mColumnMsgId);
                    WPMessageItem msgItem = getCachedMessageItem(type, msgId, cursor);
                    msgItem.setIsChecked(false);
                } else {
                }
            }
        }
    }

    public void uncheckAll() {
        int count = getCount();
        for (int i = 0; i < count; i++) {
            Cursor cursor = (Cursor) getItem(i);
            int type = cursor.getInt(mColumnsMap.mColumnWpmsType);
            long msgId = cursor.getLong(mColumnsMap.mColumnMsgId);
            WPMessageItem msgItem = getCachedMessageItem(type, msgId, cursor);
            msgItem.setIsChecked(false);
        }
    }

    /**
     *  M: delete the cache item after deleted the item, don't use it again.
     *  for ALPS00676739.
     * @param msgIds
     */
    public void deleteCachedItems(HashSet<Long> msgIds) {
        if (msgIds == null) {
            mMessageItemCache.evictAll();
        } else {
            for (long id : msgIds) {
                mMessageItemCache.remove(id);
            }
        }
    }
}
