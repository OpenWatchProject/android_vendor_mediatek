package com.mediatek.calendar.selectevent;

import android.app.Activity;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.os.Bundle;

import com.android.calendar.CalendarController;
import com.android.calendar.R;
import com.android.calendar.agenda.AgendaFragment;
import com.mediatek.calendar.extension.IAgendaChoiceForExt;

///M:This class is for Choice calendar item
public class AgendaChoiceActivity extends Activity implements IAgendaChoiceForExt {
    private static final String KEY_OTHER_APP_RESTORE_TIME = "other_app_request_time";

    private CalendarController mController;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        // This needs to be created before setContentView
        mController = CalendarController.getInstance(this);
        setContentView(R.layout.agenda_choice);

        long timeMillis = -1;
        if (icicle != null) {
            timeMillis = icicle.getLong(KEY_OTHER_APP_RESTORE_TIME);
        } else {
            timeMillis = System.currentTimeMillis();
        }

        setFragments(timeMillis);
    }

    private void setFragments(long timeMillis) {
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        AgendaFragment frag = new EventSelectionFragment(timeMillis);
        ft.replace(R.id.agenda_choice_frame, frag);
        ft.commit();
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putLong(KEY_OTHER_APP_RESTORE_TIME, mController.getTime());
    }

    @Override
    public void retSelectedEvent(Intent ret) {
        setResult(Activity.RESULT_OK, ret);
        finish();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // To remove its CalendarController instance if exists
        CalendarController.removeInstance(this);
    }
}
///@}
