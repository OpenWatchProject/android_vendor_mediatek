package com.mediatek.engineermode.usb;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.Chronometer;
import android.widget.CompoundButton;
import android.widget.Chronometer.OnChronometerTickListener;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Switch;
import android.widget.Toast;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.wifi.ChannelInfo;
import com.mediatek.engineermode.wifi.WiFi;
import com.mediatek.engineermode.wifi.WiFiStateManager;

import java.util.ArrayList;

public class UsbOtgSwitch extends Activity {

    private CheckBox mCbUSBOtg = null;
    private Switch mSWUSBOtg = null;
    private static final String TAG = "Usb/Otgswitch";
    private static final String PROPERTY_USB_OTG_SWITCH = "persist.vendor.usb.otg.switch";
    private static final String PROPERTY_USB_OTG_STATE = "vendor.usb.otg.switch.state";
    private final int QUERYTIMES = 1000;
    private int mCount = 0;
    private String value = null;
    private String state = null;
    private static final int HANDLER_ENABLE_SUCCEED = 0x01;
    private static final int HANDLER_DISENABLE_SUCCEED = 0x02;
    private static final int HANDLER_ENABLE_FAILED = 0x03;
    private static final int HANDLER_DISENABLE_FAILED = 0x04;
    OnCheckedChangeListener USBOtgCheckedChangeListener = null;

    private final Handler mHandler = new Handler() {

        public void handleMessage(Message msg) {

            if (HANDLER_ENABLE_SUCCEED == msg.what) {
                mSWUSBOtg.setText("Enabled");
            } else if (HANDLER_DISENABLE_SUCCEED == msg.what) {
                mSWUSBOtg.setText("Disabled");
            } else if (HANDLER_ENABLE_FAILED == msg.what) {
                mSWUSBOtg.setText("Enable failed");
                mSWUSBOtg.setOnCheckedChangeListener(null);

                mSWUSBOtg.setChecked(false);

                mSWUSBOtg.setOnCheckedChangeListener(USBOtgCheckedChangeListener);
            } else if (HANDLER_DISENABLE_FAILED == msg.what) {
                mSWUSBOtg.setText("Disable failed");
                mSWUSBOtg.setOnCheckedChangeListener(null);
                mSWUSBOtg.setChecked(true);
                mSWUSBOtg.setOnCheckedChangeListener(USBOtgCheckedChangeListener);
            }
            mSWUSBOtg.setEnabled(true);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usb_otg_switch);

        mSWUSBOtg = (Switch) this.findViewById(R.id.usb_otg_switchbutton);

        update_status();

        USBOtgCheckedChangeListener = new OnUSBOtgCheckedChangeListener();

        mSWUSBOtg.setOnCheckedChangeListener(USBOtgCheckedChangeListener);

    }

    public class OnUSBOtgCheckedChangeListener implements OnCheckedChangeListener {

        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            if (isChecked) {
                usbSwitchOTGmode(true);
            } else {
                usbSwitchOTGmode(false);
            }

        }
    }

    private void update_status() {
        if ("1".equals(EmUtils.systemPropertyGet(PROPERTY_USB_OTG_SWITCH,"unknown"))) {
            mSWUSBOtg.setChecked(true);
            mSWUSBOtg.setText("Enabled");
        } else {
            mSWUSBOtg.setChecked(false);
            mSWUSBOtg.setText("Disabled");
        }
    }

    private int usbSwitchOTGmode(boolean enable) {

        Elog.i(TAG, "usbSwitchOTGmode : enable is " + enable);
        mCount = 0;
        String text = enable ? "Enabling..." : "Disabling...";
        String val = enable ? "1" : "0";
        mSWUSBOtg.setText(text);
        mSWUSBOtg.setEnabled(false);
        try {
            EmUtils.getEmHidlService().setUsbOtgSwitch(val);
        } catch (Exception e) {
            Elog.e(TAG, "set property failed ...");
            e.printStackTrace();
        }
        waitForOtgState(val);
        return 0;
    }

    private void waitForOtgState(String usb_state) {
        // wait for the transition to complete.
        // give up after 10 second.
        state = usb_state;
        new Thread() {
            public void run() {

                while (true) {

                    // State transition is done when sys.usb.state is set to the
                    // new
                    // configuration
                    value = EmUtils.systemPropertyGet(PROPERTY_USB_OTG_STATE,"unknown");
                    if (state.equals(value)) {
                        if (state.equals("1")) {
                            mHandler.sendEmptyMessage(HANDLER_ENABLE_SUCCEED);
                        } else
                            mHandler.sendEmptyMessage(HANDLER_DISENABLE_SUCCEED);
                        return;
                    }
                    if (mCount == 5) {
                        if (state.equals("1")) {
                            mHandler.sendEmptyMessage(HANDLER_ENABLE_FAILED);
                        } else
                            mHandler.sendEmptyMessage(HANDLER_DISENABLE_FAILED);
                        return;
                    }
                    Elog.d(TAG, "mCount=" + mCount);
                    try {
                        mCount++;
                        Thread.sleep(QUERYTIMES);
                    } catch (InterruptedException e) {
                        Elog.e(TAG, "Catch InterruptedException");
                    }
                }
            }
        }.start();

    }
}
