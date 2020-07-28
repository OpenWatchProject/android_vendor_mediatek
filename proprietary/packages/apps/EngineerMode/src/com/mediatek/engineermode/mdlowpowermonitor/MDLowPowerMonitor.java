package com.mediatek.engineermode.mdlowpowermonitor;

import android.app.Activity;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;


public class MDLowPowerMonitor extends Activity implements View.OnClickListener {
    private static final String TAG = "MDLowPowerMonitor";
    private static final int MSG_MONITOR_ENABLE = 101;
    private static final int MSG_MONITOR_DISABLE = 102;
    private static final int MSG_MONITOR_CONFIG_SET = 103;
    private static final String MD_MONITOR_ENABLE_CMD = "AT+EGCMD=9487";
    private static final String MD_MONITOR_DISABLE_CMD = "AT+EGCMD=9453";
    private static final String MD_MONITOR_CONFIG_CMD = "AT+EGCMD=9527,4,";
    public final int RET_SUCCESS = 1;
    public final int RET_FAILED = 0;
    private Button mRadioBtnEnabled = null;
    private Button mRadioBtnDisabled = null;
    private Spinner mSpinConfigValues = null;
    private Button buttonSet = null;
    private Toast mToast = null;
    private RadioGroup mRadioBtn = null;
    private boolean mFirstchecked = true;

    private Handler mCommandHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Elog.i(TAG, "Receive msg from modem");
            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (msg.what) {
                case MSG_MONITOR_ENABLE:
                    if (null == asyncResult.exception) {
                        showToast("MD Low Power Monitor Enable Succeed!");
                        Elog.d(TAG, "MD Low Power Monitor Enable Succeed!");
                    } else {
                        showToast("MD Low Power Monitor Enable Failed!");
                        Elog.d(TAG, "MD Low Power Monitor Enable Failed!");
                    }
                    break;
                case MSG_MONITOR_DISABLE:
                    if (null == asyncResult.exception) {
                        showToast("MD Low Power Monitor Disabled Succeed!");
                        Elog.d(TAG, "MD Low Power Monitor Disabled Succeed!");
                    } else {
                        showToast("MD Low Power Monitor Disabled Failed!");
                        Elog.d(TAG, "MD Low Power Monitor Disabled Failed!");
                    }
                    break;
                case MSG_MONITOR_CONFIG_SET:
                    if (null == asyncResult.exception) {
                        showToast("MD Low Power Monitor Config Succeed!");
                        Elog.d(TAG, "MD Low Power Monitor Config Succeed!");
                    } else {
                        showToast("MD Low Power Monitor Config Failed!");
                        Elog.d(TAG, "MD Low Power Monitor Config Failed!");
                    }
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.md_low_power_monitor);
        mFirstchecked = true;
        mRadioBtnEnabled = (Button) findViewById(R.id.md_low_power_monitor_enabled);
        mRadioBtnDisabled = (Button) findViewById(R.id.md_low_power_monitor_disabled);
        mSpinConfigValues = (Spinner) findViewById(R.id.sampling_rate_values_md_low_power);
        buttonSet = (Button) findViewById(R.id.md_low_power_monitor_set_button);
        mRadioBtnEnabled.setOnClickListener(this);
        mRadioBtnDisabled.setOnClickListener(this);
        buttonSet.setOnClickListener(this);
    }

    private int setMDLowPowerMonitorValue(int value) {

        return RET_FAILED;
    }

    private void sendCommand(String[] command, int msg) {
        Elog.d(TAG, "Send Command " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(command, mCommandHander.obtainMessage(msg));
    }

    private void showToast(String msg) {
        if (mToast != null) {
            mToast.cancel();
        }
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    public void onClick(View v) {
        if (v.getId() == R.id.md_low_power_monitor_enabled) {
            sendCommand(new String[]{MD_MONITOR_ENABLE_CMD, ""}, MSG_MONITOR_ENABLE);
        } else if (v.getId() == R.id.md_low_power_monitor_disabled) {
            sendCommand(new String[]{MD_MONITOR_DISABLE_CMD, ""}, MSG_MONITOR_DISABLE);
        } else if (v.getId() == R.id.md_low_power_monitor_set_button) {
            String value = mSpinConfigValues.getItemAtPosition(
                    mSpinConfigValues.getSelectedItemPosition()).toString();
            Elog.e(TAG, "value = " + value);

            String msg_value = String.format("\"%08x\"", Integer.parseInt(value) * 1000 * 1000);
            Elog.e(TAG, "msg_value = " + msg_value);

            sendCommand(new String[]{MD_MONITOR_CONFIG_CMD + msg_value, ""},
                    MSG_MONITOR_CONFIG_SET);
        }
    }
}
