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

public class DefaultIpConversationListExt implements IIpConversationListExt {

    @Override
    public boolean onIpConversationListCreate(Activity context,
            IConversationListCallback callback, ListView listview,
            LinearLayout ipEmptyView, LinearLayout networkStatusBar,
            TextView networkStatusTextView) {
        return false;
    }

    @Override
    public boolean onIpNeedLoadView(View emptyViewDefault) {
        return false;
    }

    @Override
    public boolean onIpStartAsyncQuery() {
        return false;
    }

    @Override
    public boolean onIpCreateOptionsMenu(Menu menu) {
        return false;
    }

    @Override
    public boolean onIpPrepareOptionsMenu(Menu menu) {
        return false;
    }

    @Override
    public boolean onIpOptionsItemSelected(MenuItem item, boolean isSmsEnabled) {
        return false;
    }

    @Override
    public boolean onIpOpenThread(String number, long threadId) {
        return false;
    }

    @Override
    public boolean onIpCreateContextMenu(ContextMenu menu, String number) {
        return false;
    }

    @Override
    public boolean onIpUpdateEmptyView(Cursor cursor) {
        return false;
    }

    @Override
    public boolean onIpCreateActionMode(ActionMode mode, Menu menu) {
        return false;
    }

    @Override
    public boolean onIpPrepareActionMode(ActionMode mode, Menu menu) {
        return false;
    }

    @Override
    public boolean onIpActionItemClicked(
            ActionMode mode, MenuItem item, HashSet<Long> selectedThreadIds) {
        return false;
    }

    @Override
    public boolean onIpUpdateActionMode(HashSet<Long> selectedThreadIds) {
        return false;
    }


    @Override
    public Cursor onIpGetAllThreads() {
        return null;
    }

    public void onIpActivityResult(int requestCode, int resultCode, Intent data) {
        return;
    }

    @Override
    public int onIpGetUnreadCount(Cursor cursor, int count) {
        return count;
    }

    @Override
    public void onIpDestroy() {

    }

    @Override
    public Adapter onIpQueryComplete(ListView listView) {
        return null;
    }

    @Override
    public void onIpQueryCompleteEnd(final ListView listView, Handler handler,
            final BaseAdapter adapter) {
    }

    @Override
    public boolean onIpQueryCompleteQueryList(final ListView listView) {
        return false;
    }

    @Override
    public boolean onIpCreateNewMessage() {
        return false;
    }
}
