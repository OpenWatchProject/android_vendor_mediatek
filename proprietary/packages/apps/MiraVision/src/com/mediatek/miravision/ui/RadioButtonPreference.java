package com.mediatek.miravision.ui;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RadioButton;
import android.widget.TextView;

public class RadioButtonPreference extends Preference implements View.OnClickListener {

    private static final String TAG = "Miravision/RadioButtonPreference";

    private TextView mPreferenceTitle = null;
    private RadioButton mPreferenceButton = null;
    private boolean mChecked = false;

    /**
     * RadioButtonPreference construct
     *
     * @param context
     *            the preference associated with
     */
    public RadioButtonPreference(Context context) {
        super(context);
        setLayoutResource(R.layout.preference_radiobutton);
    }

    /**
     * RadioButtonPreference construct
     *
     * @param context
     *            the preference associated with
     */
    public RadioButtonPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setLayoutResource(R.layout.preference_radiobutton);
    }

    @Override
    public View getView(View convertView, ViewGroup parent) {
        View view = super.getView(convertView, parent);
        Log.d(TAG, "getview");
        mPreferenceTitle = (TextView) view.findViewById(R.id.preference_title);
        mPreferenceTitle.setText(getTitle());
        mPreferenceButton = (RadioButton) view.findViewById(R.id.preference_radiobutton);
        mPreferenceButton.setOnClickListener(this);
        mPreferenceButton.setChecked(mChecked);
        return view;
    }

    /**
     * get the preference checked status
     *
     * @return the checked status
     */
    public boolean isChecked() {
        return mChecked;
    }

    @Override
    public void onClick(View v) {
        boolean newValue = !isChecked();
        if (!newValue) {
            Log.d(TAG, "button.onClick return");
            return;
        }

        if (setChecked(newValue)) {
            callChangeListener(newValue);
            Log.d(TAG, "button.onClick");
        }
    }

    @Override
    protected void onClick() {
        super.onClick();
        boolean newValue = !isChecked();
        if (!newValue) {
            Log.d(TAG, "preference.onClick return");
            return;
        }
        if (setChecked(newValue)) {
            callChangeListener(newValue);
            Log.d(TAG, "preference.onClick");
        }
    }

    /**
     * set the preferce checked or unchecked
     *
     * @param checked
     *            the checked status
     * @return set success or fail
     */
    public boolean setChecked(boolean checked) {
        if (null == mPreferenceButton) {
            Log.d(TAG, "setChecked return");
            mChecked = checked;
            return false;
        }

        if (mChecked != checked) {
            mPreferenceButton.setChecked(checked);
            mChecked = checked;
            return true;
        }
        return false;
    }
}
