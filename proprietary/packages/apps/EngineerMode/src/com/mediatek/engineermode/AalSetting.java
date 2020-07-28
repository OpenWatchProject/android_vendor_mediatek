package com.mediatek.engineermode;

import android.app.Activity;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.widget.Toast;
import android.widget.ToggleButton;

/**
 * AAL Switch main activity.
 * @author mtk
 */
public class AalSetting extends Activity
        implements android.view.View.OnClickListener {
    private static final String TAG = "AalSettingTAG";
    private static final String AAL_PROPERTY_INIT_FUNCTION =
            "persist.vendor.sys.mtkaal.function";
    private static final int AAL_STATE_DISABLED = 0;
    private static final int AAL_STATE_ENABLED = 6;
    private static final int AAL_STATE_NOEXIST = -1;

    private ToggleButton mTbtnSwitch = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.aal_setting);
        mTbtnSwitch = (ToggleButton) findViewById(R.id.aal_state_tbtn);
        int state = getAalState();
        if (AAL_STATE_ENABLED == state
                || AAL_STATE_NOEXIST == state) {
            mTbtnSwitch.setText(getString(R.string.aal_on));
            mTbtnSwitch.setChecked(true);
        }
        mTbtnSwitch.setOnClickListener(this);
    }

    private int getAalState() {
        String value = EmUtils.systemPropertyGet(AAL_PROPERTY_INIT_FUNCTION, "-1");
        try {
            return Integer.parseInt(value);
        } catch (NumberFormatException e) {
            return AAL_STATE_NOEXIST;
        }
    }

    private int switchAal(boolean enabled) {
        Elog.d(TAG, "switchAal:" + enabled);
        int value = AAL_STATE_DISABLED;
        if (enabled) {
            value = AAL_STATE_ENABLED;
        }
        try {
            EmUtils.getEmHidlService().setEmConfigure(AAL_PROPERTY_INIT_FUNCTION,
                    String.valueOf(value));
        }
        catch (Exception e) {
            e.printStackTrace();
            Elog.e(TAG, "set property failed ...");
        }
        if (value == getAalState()) {
            return 0;
        }
        Elog.d(TAG, "fail to set aal system properties");
        return -1;
    }

    @Override
    public void onClick(View view) {
        if (view == mTbtnSwitch) {
            switchAal(mTbtnSwitch.isChecked());
            Toast.makeText(this, R.string.aal_switch_tip, Toast.LENGTH_SHORT).show();
        }
    }
}
