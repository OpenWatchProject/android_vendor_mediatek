package com.mediatek.engineermode.wfdsettings;


import android.app.Activity;
import android.os.Bundle;
import android.os.SystemProperties;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Activity for IOT property settings.
 *
 */
public class WfdIotProperty extends Activity {
    private static final String TAG = "WfdIot";
    private CheckBox mScenaEnableChk;
    private CheckBox mDropEnableChk;
    private CheckBox mAvSyncEnableChk;
    private Button mSetBtn;

    private static final String SCENA_PROPERTY = "vendor.media.wfd.scenario.mode";
    private static final String DROP_PROPERTY = "vendor.media.wfd.drop.dummynal";
    private static final String AV_SYNC_PROPERTY = "vendor.media.wfd.av.sync";
    private static final String VALUE_ENABLE = "1";
    private static final String VALUE_DISABLE = "0";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.wfd_iot_property);
        mScenaEnableChk = (CheckBox) findViewById(R.id.checkBoxScena);
        mDropEnableChk = (CheckBox) findViewById(R.id.checkBoxDrop);
        mAvSyncEnableChk = (CheckBox) findViewById(R.id.checkBoxAcSync);
        mSetBtn = (Button) findViewById(R.id.buttonSet);
        initValue();
        mSetBtn.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                setFunctionEnable(SCENA_PROPERTY, mScenaEnableChk.isChecked());
                setFunctionEnable(DROP_PROPERTY, mDropEnableChk.isChecked());
                setFunctionEnable(AV_SYNC_PROPERTY, mAvSyncEnableChk.isChecked());
            }
        });

    }

    private void initValue() {
        mScenaEnableChk.setChecked(getFunctionEnable(SCENA_PROPERTY));
        mDropEnableChk.setChecked(getFunctionEnable(DROP_PROPERTY));
        mAvSyncEnableChk.setChecked(getFunctionEnable(AV_SYNC_PROPERTY));
    }

    private boolean getFunctionEnable(String systemproperty) {
        String value = SystemProperties.get(systemproperty);

        Elog.i(TAG, "get:" + systemproperty + " value: " + value);

        if (value == null) {
            return true;
        }

        value = value.trim();
        return (value.equals(VALUE_ENABLE));

    }

    private void setFunctionEnable(String systemproperty, boolean enable) {

        Elog.i(TAG, "set:" + systemproperty + " value: " + enable);

        SystemProperties.set(systemproperty, enable ? VALUE_ENABLE : VALUE_DISABLE);

    }


}
