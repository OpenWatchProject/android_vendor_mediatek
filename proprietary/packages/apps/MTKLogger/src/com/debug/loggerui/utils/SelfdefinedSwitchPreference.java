package com.debug.loggerui.utils;

import android.content.Context;
import android.preference.Preference;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.Switch;

import com.debug.loggerui.R;

/**
 * Self define SwitchPreference, click on preference will not affect the
 * switch's checked status.
 *
 * This completed re-implementation is not necessary, it can be implemented by
 * extends system default SwitchPreference
 */
public class SelfdefinedSwitchPreference extends Preference {
    private Switch mSwitch = null;
    private boolean mIsChecked = false;
    private Context mContext;

    /**
     * @param context Context
     * @param attr AttributeSet
     */
    public SelfdefinedSwitchPreference(Context context, AttributeSet attr) {
        super(context, attr, 0);

        mContext = context;
        setLayoutResource(R.layout.log_pref_switch);
        if (isPersistent()) {
            mIsChecked =
                    PreferenceManager.getDefaultSharedPreferences(mContext).getBoolean(getKey(),
                            true);
        }
    }

    @Override
    protected void onBindView(View view) {
        super.onBindView(view);
        mSwitch = (Switch) view.findViewById(R.id.log_selected);
        mSwitch.setChecked(mIsChecked);
        if (isPersistent()) {
            persistBoolean(mIsChecked);
        }
        mSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                callOnStateChangeListener(isChecked);
            }
        });
    }

    @Override
    protected void onClick() {
        super.onClick();
    }

    public boolean isChecked() {
        return mIsChecked;
    }

    /**
     * @param checked boolean
     */
    public void setChecked(boolean checked) {
        mIsChecked = checked;
        notifyChanged();
    }

    private void callOnStateChangeListener(boolean newValue) {
        setChecked(newValue);
        this.callChangeListener(newValue);
    }

}
