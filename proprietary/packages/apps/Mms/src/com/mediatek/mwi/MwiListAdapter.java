package com.mediatek.mwi;

import android.content.Context;
import android.database.Cursor;
import android.util.Log;
import android.util.LruCache;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CursorAdapter;

import com.android.mms.R;

import com.mediatek.mwi.MwiMessage.Columns;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

public class MwiListAdapter extends CursorAdapter {
    private static final boolean DEBUG = true;
    private static final String TAG = "Mms/Mwi/MwiListAdapter";
    /*
     * If many MWI messages would be inserted or deleted at one time,
     * record the times.
     */
    private static int sChangeCount = 0;
    /*
     * If one action of database is done, the count would be add.
     */
    private int mNotifiedCount = 0;
    private DataSetChangeListener mDataSetChangeListener;
    protected LayoutInflater mInflater;
    private LruCache<Long, MwiMessage> mMessageItemCache;
    private List<MwiListItem> mMwiListItems = new ArrayList<MwiListItem>();
    private Context mContext;

    private static final int CACHE_SIZE         = 50;

    public MwiListAdapter(Context context, Cursor cursor) {
        super(context, cursor, FLAG_REGISTER_CONTENT_OBSERVER);
        mInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        mMessageItemCache = new LruCache<Long, MwiMessage>(CACHE_SIZE);
        mContext = context;
    }

    @Override
    protected void onContentChanged() {
        super.onContentChanged();
        mNotifiedCount++;
        Log.d(TAG, "onContentChanged notified count: "
                + mNotifiedCount + ", sChangeCount: " + sChangeCount);
        if (mNotifiedCount < sChangeCount) {
            return;
        }
        if (mDataSetChangeListener != null) {
            mDataSetChangeListener.onContentChanged();
            Log.d(TAG, "onContentChanged excuted, NotifiedCount: "
                    + mNotifiedCount + ", sChangeCount: " + sChangeCount);
        }
        sChangeCount -= mNotifiedCount;
        if (sChangeCount < 0) {
            sChangeCount = 0;
        }
        mNotifiedCount = 0;
    }

    public interface DataSetChangeListener {
        void onContentChanged();
    }

    public void setDataSetChangeListener(DataSetChangeListener dataSetChangeListener) {
        this.mDataSetChangeListener = dataSetChangeListener;
    }

    public static void updateChangeCount(int count) {
        sChangeCount += count;
    }

    @Override
    public View newView(Context context, Cursor cursor, ViewGroup parent) {
        return mInflater.inflate(R.layout.mwi_list_item, parent, false);
    }

    @Override
    public void bindView(View view, Context context, Cursor cursor) {
        if (view instanceof MwiListItem) {
            long msgId =
                cursor.getLong(cursor.getColumnIndex(MwiMessage.Columns.Id.getColumnName()));

            MwiMessage msgItem = getCachedMessageItem(msgId, cursor);
            if (msgItem != null) {
                MwiListItem mli = (MwiListItem) view;
                mMwiListItems.add(mli);
                mli.bind(msgItem);
            }
        }
    }

    public MwiMessage getCachedMessageItem(long msgId, Cursor c) {
        MwiMessage item = mMessageItemCache.get(msgId);
        if (c != null && isCursorValid(c)) {
            if (item == null) {
                try {
                    item = new MwiMessage(mContext, c);
                    mMessageItemCache.put(msgId, item);
                } catch (Exception e) {
                    Log.e(TAG, "getCachedMessageItem: " + e.getMessage());
                    e.printStackTrace();
                }
            } else {
                item.setTo(c.getString(c.getColumnIndex(Columns.To.getColumnName())));
                item.setFrom(c.getString(c.getColumnIndex(Columns.From.getColumnName())));
                item.setSubject(c.getString(c.getColumnIndex(Columns.Subject.getColumnName())));
                item.setDate(c.getLong(c.getColumnIndex(Columns.Date.getColumnName())));
                item.setPriorityId(c.getInt(c.getColumnIndex(Columns.Priority.getColumnName())));
                item.setMsgId(c.getString(c.getColumnIndex(Columns.MsgId.getColumnName())));
                item.setMsgContextId(
                        c.getInt(c.getColumnIndex(Columns.MsgContext.getColumnName())));
                item.setMsgAccount(
                        c.getString(c.getColumnIndex(Columns.MsgAccount.getColumnName())));
                item.setSeen(c.getInt(c.getColumnIndex(Columns.Seen.getColumnName())) == 1
                        ? true : false);
                item.setRead(c.getInt(c.getColumnIndex(Columns.Read.getColumnName())) == 1
                        ? true : false);
                item.setGotContent(
                        c.getInt(c.getColumnIndex(Columns.GotContent.getColumnName())) == 1
                        ? true : false);
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

    /**
     * delete the cache item after deleted the item, don't use it again.
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

    public boolean isAllSelected() {
        int count = getCount();
        boolean isAllChecked = true;
        for (int i = 0; i < count; i++) {
            Cursor cursor = (Cursor) getItem(i);
            long msgId =
                cursor.getLong(cursor.getColumnIndex(MwiMessage.Columns.Id.getColumnName()));
            MwiMessage msgItem = getCachedMessageItem(msgId, cursor);
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
                    long msgId =
                        cursor.getLong(
                                cursor.getColumnIndex(MwiMessage.Columns.Id.getColumnName()));
                    MwiMessage msgItem = getCachedMessageItem(msgId, cursor);
                    msgItem.setChecked(false);
                }
            }
        }
    }
}
