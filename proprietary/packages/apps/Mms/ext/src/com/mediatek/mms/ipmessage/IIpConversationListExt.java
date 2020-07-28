package com.mediatek.mms.ipmessage;

import android.app.Activity;
import android.content.Intent;
import android.database.Cursor;
import android.os.Handler;
import android.view.ActionMode;
import android.view.ContextMenu;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Adapter;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.mediatek.mms.callback.IConversationListCallback;

import java.util.HashSet;

public interface IIpConversationListExt {

    /**
     * M: called on onCreate
     * @param context: this activity
     * @param callback: this callback
     * @param listview: conversation listview
     * @param ipEmptyView: ipEmptyView
     * @param networkStatusBar: networkStatusBar
     * @param networkStatusTextView: networkStatusTextView
     * @return boolean
     * @internal
     */
    public boolean onIpConversationListCreate(Activity context,
            IConversationListCallback callback, ListView listview,
            LinearLayout ipEmptyView, LinearLayout networkStatusBar,
            TextView networkStatusTextView);

    /**
     * M: called on onStart
     * @param emptyViewDefault: this emptyViewDefault
     * @return boolean
     * @internal
     */
    public boolean onIpNeedLoadView(View emptyViewDefault);

    /**
     * M: called on startAsyncQuery
     * @return boolean
     * @internal
     */
    public boolean onIpStartAsyncQuery();

    /**
     * M: called onCreateOptionsMenu
     * @param menu: optionsmenu
     * @return boolean
     * @internal
     */
    public boolean onIpCreateOptionsMenu(Menu menu);

    /**
     * M: called onPrepareOptionsMenu
     * @param menu: optionsmenu
     * @return boolean
     * @internal
     */
    public boolean onIpPrepareOptionsMenu(Menu menu);

    /**
     * M: called onOptionsItemSelected
     * @param menu: optionsmenu
     * @param isSmsEnabled: isSmsEnabled
     * @return boolean
     * @internal
     */
    public boolean onIpOptionsItemSelected(MenuItem item, boolean isSmsEnabled);

    /**
     * M: called viewThread
     * @param number: number
     * @param threadId: threadId
     * @return boolean
     * @internal
     */
    public boolean onIpOpenThread(String number, long threadId);

    /**
     * M: called onCreateContextMenu
     * @param menu: ContextMenu
     * @param number: number
     * @return boolean
     * @internal
     */
    public boolean onIpCreateContextMenu(ContextMenu menu, String number);

    /**
     * M: called onQueryComplete
     * @param cursor: cursor
     * @return boolean
     * @internal
     */
    public boolean onIpUpdateEmptyView(Cursor cursor);

    /**
     * M: called onCreateActionMode
     * @param mode: ActionMode
     * @param menu: menu
     * @return boolean
     * @internal
     */
    public boolean onIpCreateActionMode(ActionMode mode, Menu menu);

    /**
     * M: called onPrepareActionMode
     * @param mode: ActionMode
     * @param menu: menu
     * @return boolean
     * @internal
     */
    public boolean onIpPrepareActionMode(ActionMode mode, Menu menu);

    /**
     * M: called onActionItemClicked
     * @param mode: ActionMode
     * @param menu: menu
     * @param selectedThreadIds: selectedThreadIds
     * @return boolean
     * @internal
     */
    public boolean onIpActionItemClicked(final ActionMode mode,
            MenuItem item, HashSet<Long> selectedThreadIds);


    /**
     * M: called UpdateActionMode
     * @param selectedThreadIds: selectedThreadIds
     * @return boolean
     * @internal
     */
    public boolean onIpUpdateActionMode(HashSet<Long> selectedThreadIds);

    /**
     * M: called setAllItemChecked
     * @return Cursor
     * @internal
     */
    public Cursor onIpGetAllThreads();

    /**
     * M: called onActivityResult
     * @param requestCode: requestCode
     * @param resultCode: resultCode
     * @param data: data
     * @internal
     */
    public void onIpActivityResult(int requestCode, int resultCode, Intent data);

    /**
     * M: called onQueryComplete
     * @param cursor: cursor
     * @param count: count
     * @return int: count
     * @internal
     */
    public int onIpGetUnreadCount(Cursor cursor, int count);

    /**
     * M: called onDestroy
     * @internal
     */
    public void onIpDestroy();

    /**
     * M: called onQueryComplete
     * @internal
     */
    public Adapter onIpQueryComplete(ListView listView);

    /**
     * M: called onIpQueryComplete (at the end)
     * @internal
     */
    public void onIpQueryCompleteEnd(final ListView listView, Handler handler,
            final BaseAdapter adapter);

    /**
     * M: called onIpQueryComplete (when token is THREAD_LIST_QUERY_TOKEN)
     * @internal
     */
    public boolean onIpQueryCompleteQueryList(final ListView listView);

    /**
     * M: called createNewMessage
     * @internal
     */
    public boolean onIpCreateNewMessage();
}
