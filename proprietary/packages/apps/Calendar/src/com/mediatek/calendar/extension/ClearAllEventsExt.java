package com.mediatek.calendar.extension;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import android.view.Menu;

import com.android.calendar.R;
import com.mediatek.calendar.clearevents.SelectClearableCalendarsActivity;

/**
 * M: ClearAllEventsExt extends the AllInOneActivity's Options Menu
 * provides the ability to launch SelectClearableCalendarsActivity
 */
public class ClearAllEventsExt implements IOptionsMenuExt {

    private static final int MENU_ITEM_ID = R.id.action_delete_all_events;
    private static final String TAG = "ClearAllEventsExt";

    Context mContext;

    /**
     * Constructor
     * @param context context to start activity
     */
    public ClearAllEventsExt(Context context) {
        mContext = context;
    }

    @Override
    public void onCreateOptionsMenu(Menu menu) {
        menu.findItem(MENU_ITEM_ID).setVisible(true);
    }

    @Override
    public boolean onOptionsItemSelected(int itemId) {
        if (MENU_ITEM_ID == itemId) {
            Log.w(TAG, "delete all events.");
            //Selection must be specified for content://com.android.calendar/events
            launchSelectClearableCalendars();
            return true;
        }
        return false;
    }

    /**
     * launch the Activity to handle the request
     */
    private void launchSelectClearableCalendars() {
        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setClass(mContext, SelectClearableCalendarsActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT | Intent.FLAG_ACTIVITY_SINGLE_TOP);
        mContext.startActivity(intent);
    }

}
