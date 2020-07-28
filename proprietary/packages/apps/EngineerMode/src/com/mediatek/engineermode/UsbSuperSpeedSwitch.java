package com.mediatek.engineermode;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import android.os.Handler;
import android.os.Message;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ShellExe;
import com.mediatek.engineermode.R;

import java.io.IOException;

public class UsbSuperSpeedSwitch extends Activity implements CompoundButton.OnCheckedChangeListener{

    private static final String TAG = "Usb/SuperSpeedSwitch";

    private static final String CURRENT_SPEED_MODE_PATH = "/sys/class/udc/musb-hdrc/current_speed";
    private static final String PROPERTY_SUPER_SPEED_SWITCH = "vendor.usb.speed.mode";
    private static final String PROPERTY_SUPER_SPEED_SWITCH_REBOOT = "persist.vendor.usb.speed.mode";
    private static final String VALUE_SUPER_SPEED = "u3";
    private static final String VALUE_HIGH_SPEED = "u2";
    private static final int QUERY_CURRENT_STATUS = 10;
    private static final int QUERY_CURRENT_STATUS_DELAY = 2000;
    private TextView mStatus;

    private IntentFilter mIntentFilterUsbState;

    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {

            switch (msg.what) {
                case QUERY_CURRENT_STATUS:
                    updateCurrentStatus();
                    break;
                default:
                    break;
            }
        }

    };

    private BroadcastReceiver mIntentReceiverUsbState = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (UsbManager.ACTION_USB_STATE.equals(action)) {
                mHandler.removeMessages(QUERY_CURRENT_STATUS);
                mHandler.sendEmptyMessageDelayed(QUERY_CURRENT_STATUS, QUERY_CURRENT_STATUS_DELAY);
                mStatus.setText(getString(R.string.usb_loading_speed));
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usb_super_speed_switch);

        mStatus = (TextView) this.findViewById(R.id.usb_super_speed_status);

        Switch superSpeedSW = (Switch) this.findViewById(R.id.usb_super_speed);
        Switch superSpeedRebootSW = (Switch) this.findViewById(R.id.usb_super_speed_reboot);
        boolean isSS = VALUE_SUPER_SPEED.equals(
                EmUtils.systemPropertyGet(PROPERTY_SUPER_SPEED_SWITCH, VALUE_HIGH_SPEED));
        boolean isSSR = VALUE_SUPER_SPEED.equals(
                EmUtils.systemPropertyGet(PROPERTY_SUPER_SPEED_SWITCH_REBOOT, VALUE_HIGH_SPEED));
        superSpeedSW.setChecked(isSS);
        superSpeedRebootSW.setChecked(isSSR);
        superSpeedSW.setOnCheckedChangeListener(this);
        superSpeedRebootSW.setOnCheckedChangeListener(this);

        mIntentFilterUsbState = new IntentFilter();
        mIntentFilterUsbState.addAction(UsbManager.ACTION_USB_STATE);

        updateCurrentStatus();
    }

    @Override
    public void onStart() {
        super.onStart();
        registerReceiver(mIntentReceiverUsbState, mIntentFilterUsbState);
    }

    @Override
    public void onStop() {
        unregisterReceiver(mIntentReceiverUsbState);
        super.onStop();
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        switch (buttonView.getId()) {
            case R.id.usb_super_speed:
                switchSuperSpeed(PROPERTY_SUPER_SPEED_SWITCH, isChecked);
                break;
            case R.id.usb_super_speed_reboot:
                Switch superSpeedSW = (Switch) this.findViewById(R.id.usb_super_speed);
                switchSuperSpeed(PROPERTY_SUPER_SPEED_SWITCH_REBOOT, isChecked);
                superSpeedSW.setChecked(isChecked);
                break;
            default:
                break;
        }
    }

    private void updateCurrentStatus() {
        String speed = getSuperSpeedMode();
        mStatus.setText(getString(R.string.usb_super_speed_status, speed));
    }

    private void switchSuperSpeed(String key, boolean enable) {
        String val = enable ? VALUE_SUPER_SPEED : VALUE_HIGH_SPEED;
        Elog.i(TAG, "switch:" + key + " = " + val);
        try {
            EmUtils.getEmHidlService().setEmConfigure(key, val);
            mHandler.removeMessages(QUERY_CURRENT_STATUS);
            mHandler.sendEmptyMessageDelayed(QUERY_CURRENT_STATUS, QUERY_CURRENT_STATUS_DELAY);
            mStatus.setText(getString(R.string.usb_loading_speed));
        } catch (Exception e) {
            Elog.w(TAG, "set property failed ...");
            mStatus.setText(getString(R.string.usb_switch_speed_fail));
            e.printStackTrace();
        }
    }

    private String getSuperSpeedMode() {
        String result = getString(R.string.high_speed);
        String cmd = new StringBuilder("cat ").append(CURRENT_SPEED_MODE_PATH).toString();
        Elog.i(TAG, "getSuperSpeedMode cmd: " + cmd);
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(cmd, true)) {
                result = ShellExe.getOutput();
            }
        } catch (IOException e) {
            Elog.w(TAG, "get current dramc IOException: " + e.getMessage());
            Toast.makeText(this, "Get Super Speed Mode Fail",
                    Toast.LENGTH_LONG).show();
        }
        return result;
    }
}
