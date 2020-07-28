package com.mediatek.batterywarning;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View.OnClickListener;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

/**
  * ThermalWarningActivity: show warning dialog when thermal is over temperature.
  */
public class ThermalWarningActivity extends Activity {
    private static final String TAG = "ThermalWarningActivity";
    protected static final String KEY_TYPE = "type";

    private static final int[] sWarningMsg = new int[] { R.string.thermal_clear_temperature,
            R.string.thermal_over_temperature };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Intent intent = getIntent();
        int type = intent.getIntExtra(KEY_TYPE, -1);
        Log.d(TAG, "onCreate, type is " + type);

        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.thermal_warning);

        showWarningDialog(type);
    }

    private void showWarningDialog(int type) {
        TextView mMessageView = (TextView) findViewById(R.id.text);
        mMessageView.setText(getString(sWarningMsg[type]));

        Button button = (Button) findViewById(R.id.yes);
        button.setText(getString(android.R.string.yes));
        button.setOnClickListener(mYesContentListener);
    }

    private OnClickListener mYesContentListener = new OnClickListener() {
        public void onClick(View v) {
            finish();
        }
    };
}
