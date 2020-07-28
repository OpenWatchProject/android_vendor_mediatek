package com.mediatek.calendar.selectevent;

import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.AttributeSet;
import android.view.View;
import android.widget.AdapterView;

import com.android.calendar.agenda.AgendaListView;
import com.mediatek.calendar.LogUtil;
import com.mediatek.calendar.extension.IAgendaChoiceForExt;

public class EventsListView extends AgendaListView {

    private static final String TAG = "EventsListView";

    private Context mContext;

    /**
     * M: constructor for a view
     * @param context will always be AgendaChoiceActivity
     * @param attrs
     */
    public EventsListView(Context context, AttributeSet attrs) {
        super(context, attrs);
        LogUtil.v(TAG, "EventsListView inited");
        mContext = context;
    }

    @Override
    public void onItemClick(AdapterView<?> a, View v, int position, long id) {
        /// M: index -1 is blank view, just return, do nothing. @{
        if (id == -1) {
            return;
        }
        /// @}
        long eventId = getEventIdByPosition(position);
        shareSingleEvent(eventId);
    }

    /**
     * M: share a single event
     * @param eventId the event id
     */
    private void shareSingleEvent(long eventId) {
        Uri uri = ContentUris.withAppendedId(Uri.parse(
        "content://com.mediatek.calendarimporter/events"), eventId);
        Intent intent = new Intent();
        intent.setData(uri);
        LogUtil.d(TAG, "onItemClick(), Email selected calendar, uri=" + uri);

        if (mContext instanceof IAgendaChoiceForExt) {
            ((IAgendaChoiceForExt) mContext).retSelectedEvent(intent);
        }
    }

}
