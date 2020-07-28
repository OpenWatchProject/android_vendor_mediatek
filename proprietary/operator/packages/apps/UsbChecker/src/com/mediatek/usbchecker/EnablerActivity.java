package com.mediatek.usbchecker;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class EnablerActivity extends Activity {

    private static final String TAG = "UsbChecker.EnablerActivity";
    private Button mActivateDevice;
    private TextView mDeviceState;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_enabler);
        Log.i(TAG, "onCreate");

        mActivateDevice = (Button) findViewById(R.id.activate_device);
        mDeviceState = (TextView) findViewById(R.id.state_device);

        mActivateDevice.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View v) {
                activateDevice();
                updateUIAfterEnable();
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (UsbCheckerService.getActivateState()) {
            updateUIAfterEnable();
        }
    }

    private void activateDevice() {
        Log.i(TAG, "activateDevice");
        Intent intent = new Intent(UsbCheckerConstants.INTENT_ENGINEER_ACTIVATE);
        intent.setClass(this, UsbCheckerService.class);
        startService(intent);
    }

    private void updateUIAfterEnable() {
        Log.i(TAG, "updateUIAfterEnable");
        mDeviceState.setText(R.string.text_already_activated);
        mActivateDevice.setEnabled(false);
    }
}
