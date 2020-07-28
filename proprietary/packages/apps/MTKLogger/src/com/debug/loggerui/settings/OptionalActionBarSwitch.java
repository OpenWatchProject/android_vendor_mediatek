package com.debug.loggerui.settings;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.preference.PreferenceActivity;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ImageButton;
import android.widget.Switch;
import android.widget.TextView;

import com.debug.loggerui.R;

/**
 * Since ActionBar and Switch component were not supported in GB, to make sure
 * one copy source code can build pass in both JB and GB, we need to add mock
 * component for GB. This class will appear in JB and later load
 */
public class OptionalActionBarSwitch {
    private Switch mBarSwitch;

    private Activity mActivity;
    private TextView mActionBarTextView;

    /**
     * Constructor for Switch component, which appears in each log's detail
     * settings page.
     *
     * @param activity PreferenceActivity
     */
    public OptionalActionBarSwitch(PreferenceActivity activity) {
        mBarSwitch = new Switch(activity);
        if (activity.onIsHidingHeaders() || !activity.onIsMultiPane()) {
            mBarSwitch.setPadding(0, 0, 16, 0);
            activity.getActionBar().setDisplayOptions(ActionBar.DISPLAY_SHOW_CUSTOM,
                    ActionBar.DISPLAY_SHOW_CUSTOM);
            activity.getActionBar().setCustomView(
                    mBarSwitch,
                    new ActionBar.LayoutParams(ActionBar.LayoutParams.WRAP_CONTENT,
                            ActionBar.LayoutParams.WRAP_CONTENT, Gravity.CENTER_VERTICAL
                                    | Gravity.RIGHT));
        }
    }

    /**
     * Constructor for TextView in action bar, which appears in log file list
     * activity page.
     *
     * @param activity Activity
     * @param selectedNumber int
     */
    public OptionalActionBarSwitch(Activity activity, int selectedNumber) {
        mActivity = activity;
        // Inflate a custom action bar that contains the "done" button for
        // multi-choice
        LayoutInflater inflater =
                (LayoutInflater) activity.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View customActionBarView = inflater.inflate(R.layout.multichoice_custom_action_bar, null);
        ImageButton doneMenuItem =
                (ImageButton) customActionBarView.findViewById(R.id.done_menu_item);
        mActionBarTextView = (TextView) customActionBarView.findViewById(R.id.select_items);
        mActionBarTextView
                .setText(activity.getString(R.string.selected_item_count, selectedNumber));
        doneMenuItem.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mActivity.finish();
            }
        });

        // Show the custom action bar but hide the home icon and title
        activity.getActionBar().setDisplayOptions(
                ActionBar.DISPLAY_SHOW_CUSTOM,
                ActionBar.DISPLAY_SHOW_CUSTOM | ActionBar.DISPLAY_SHOW_HOME
                        | ActionBar.DISPLAY_SHOW_TITLE);
        activity.getActionBar().setCustomView(customActionBarView);
    }

    /**
     * @param num int
     */
    public void updateTitle(int num) {
        if (mActionBarTextView != null) {
            mActionBarTextView.setText(mActivity.getString(R.string.selected_item_count, num));
        }
    }

    /**
     * @param checked boolean
     */
    public void setChecked(boolean checked) {
        if (mBarSwitch != null) {
            mBarSwitch.setChecked(checked);
        }
    }

    /**
     * @param enabled boolean
     */
    public void setEnabled(boolean enabled) {
        if (mBarSwitch != null) {
            mBarSwitch.setEnabled(enabled);
        }
    }

    /**
     * @param listener LogSwitchListener
     */
    public void setOnCheckedChangeListener(LogSwitchListener listener) {
        if (mBarSwitch != null) {
            mBarSwitch.setOnCheckedChangeListener(listener);
        }
    }

}
