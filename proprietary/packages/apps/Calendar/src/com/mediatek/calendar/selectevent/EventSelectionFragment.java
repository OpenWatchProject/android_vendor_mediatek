package com.mediatek.calendar.selectevent;

import android.view.LayoutInflater;
import android.view.View;

import com.android.calendar.R;
import com.android.calendar.agenda.AgendaFragment;
import com.android.calendar.agenda.AgendaListView;
import com.mediatek.calendar.LogUtil;

public class EventSelectionFragment extends AgendaFragment {

    private static final String TAG = "EventSelectionFragment";

    public EventSelectionFragment() {
        this(0);
    }

    /**
     * M: constructor
     * @param timeMillis time millis to launch the Fragment
     */
    public EventSelectionFragment(long timeMillis) {
        super(timeMillis, false);
        LogUtil.v(TAG, "EventSelectionFragment created");
    }

    @Override
    protected View extInflateFragmentView(LayoutInflater inflater) {
        LogUtil.v(TAG, "mtk_event_selection_fagment inflated");
        return inflater.inflate(R.layout.mtk_event_selection_fragment, null);
    }

    @Override
    protected AgendaListView extFindListView(View v) {
        LogUtil.v(TAG, "found EventsListView");
        return (AgendaListView) v.findViewById(R.id.mtk_events_list);
    }

}
