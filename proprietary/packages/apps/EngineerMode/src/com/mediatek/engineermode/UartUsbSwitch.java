package com.mediatek.engineermode;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;

import java.io.File;
import java.io.IOException;

public class UartUsbSwitch extends Activity {

    private static final String TAG = "UartUsbSwitch";
    private static final String DIR_PORT_MODE_1 = "/sys/devices/platform/mt_usb/";
    private static final String DIR_PORT_MODE_2
                                   = "/sys/devices/platform/musb-mtu3d/musb-hdrc/";
    private static final String DIR_PORT_MODE_3 = "/sys/bus/platform/devices/musb-hdrc/";
    private static final String DIR_PORT_MODE_4 = "/sys/class/udc/musb-hdrc/device/";
    private static final String USB_CONNECT_STATE = "/sys/class/android_usb/android0/state";
    private static final String FILE_PORT_MODE = "portmode";
    private static final String FILE_UART_STATE = "is_uart_plugged";
    private static final String USB_CONNECT = "CONNECT";
    private static final String USB_CONFIGURED = "CONFIGURED";
    private static final String UART_CONNECT = "1";
    private static final String UART_DISCONNECT = "0";
    private static final String SUCCESS = " success";
    private static final String FAIL = " fail";
    private static final String MODE_USB = "0";
    private static final String MODE_UART = "1";
    private static final String PROPERTY_USB_PORT = "vendor.usb.port.mode";
    private static final String VAL_USB = "usb";
    private static final String VAL_UART = "uart";
    private static final int MSG_CHECK_RESULT = 11;
    private static final int DIALOG_USB_WARNING = 0;
    private static final int DIALOG_USB_CONNECT_WARNING = 1;
    private static final int DIALOG_UART_CONNECT_WARNING = 2;
    private TextView mTvCurrent;
    private RadioGroup mRgMode;
    private WorkerHandler mWorkerHandler = null;
    private HandlerThread mWorkerThread = null;
    private String mModeVal;
    private RadioButton mRbUsb;
    private RadioButton mRbUart;
    private String mPortFile;
    private String mUartStateFile;
    private final RadioGroup.OnCheckedChangeListener mCheckListener = new RadioGroup.OnCheckedChangeListener() {

        public void onCheckedChanged(RadioGroup group, int checkedId) {
            Boolean bModeUsb = null;
            switch (checkedId) {
            case R.id.uart_usb_switch_mode_usb:
                bModeUsb = true;
                break;
            case R.id.uart_usb_switch_mode_uart:
                bModeUsb = false;
                break;
            case -1:
            default:
                break;
            }
            doSwitch(bModeUsb);
            Elog.d(TAG, "OnCheckedChangeListener.onCheckedChanged() checkId:" + checkedId
                    + " bModeUsb:" + bModeUsb);
        }

    };

    private void showDialog(String title, String msg) {
        AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                true).setTitle(title).setMessage(msg).
                setPositiveButton(android.R.string.ok, null).create();
        dialog.show();
    }

    private class WorkerHandler extends Handler {
        WorkerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_CHECK_RESULT:
                final boolean result = waitForState(mModeVal, 2000);
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        updateStatus(VAL_USB.equals(mModeVal));
                        enableUsbUartSwitch(true);
                        showDialog(null, getString(R.string.uart_usb_switch_set)
                              + (result ? SUCCESS : FAIL));

                    }
                });

                break;
            default:
                Elog.w(TAG, "mWorkerHandler Unknown msg: " + msg.what);
                break;
            }
            super.handleMessage(msg);
        }
    }
    private void enableUsbUartSwitch(boolean enabled) {
        mRbUsb.setEnabled(enabled);
        mRbUart.setEnabled(enabled);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String dir;
        if (new File(DIR_PORT_MODE_1).exists()) {
            dir = DIR_PORT_MODE_1;
        }else if (new File(DIR_PORT_MODE_2).exists()){
            dir = DIR_PORT_MODE_2;
        }else if (new File(DIR_PORT_MODE_3).exists()){
            dir = DIR_PORT_MODE_3;
        }else if (new File(DIR_PORT_MODE_4).exists()){
            dir = DIR_PORT_MODE_4;
        }else {
            Toast.makeText(this, R.string.uart_usb_switch_notsupport,
                    Toast.LENGTH_SHORT).show();
            Elog.w(TAG, "Port mode file not exist");
            finish();
            return;
        }
        mPortFile = dir + FILE_PORT_MODE;
        mUartStateFile = dir + FILE_UART_STATE;
        Elog.v(TAG, "mPortFile: " + mPortFile);
        setContentView(R.layout.uart_usb_switch);
        mTvCurrent = (TextView) findViewById(R.id.uart_usb_switch_current_mode);
        mRgMode = (RadioGroup) findViewById(R.id.uart_usb_switch_mode);
        mRbUsb = (RadioButton) findViewById(R.id.uart_usb_switch_mode_usb);
        mRbUart = (RadioButton) findViewById(R.id.uart_usb_switch_mode_uart);
        mWorkerThread = new HandlerThread(TAG);
        mWorkerThread.start();
        mWorkerHandler = new WorkerHandler(mWorkerThread.getLooper());
        showDialog(DIALOG_USB_WARNING);
    }
    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        AlertDialog.Builder builder = null;
        if (id == DIALOG_USB_WARNING) {
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.uart_usb_switch_dialog_title);
            builder.setCancelable(false);
            builder.setMessage(getString(R.string.uart_usb_switch_warning));
            builder.setPositiveButton(R.string.ok, null);
            dialog = builder.create();
        } else if (id == DIALOG_USB_CONNECT_WARNING ||
                id == DIALOG_UART_CONNECT_WARNING) {
            builder = new AlertDialog.Builder(this);
            builder.setTitle(R.string.uart_usb_switch_dialog_title_error);
            builder.setCancelable(false);
            String msg = (id == DIALOG_USB_CONNECT_WARNING)
                    ? getString(R.string.uart_usb_switch_dialog_usb_error)
                    : getString(R.string.uart_usb_switch_dialog_uart_error);
            builder.setMessage(msg);
            builder.setPositiveButton(R.string.ok,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            finish();
                        }
                    });
            dialog = builder.create();
        }
        return dialog;
    }

    @Override
    protected void onResume() {
        super.onResume();
        String current = getUsbMode();
        Elog.v(TAG, "Current: " + current);
        if (null == current) {
            Toast.makeText(this, R.string.uart_usb_switch_geterror,
                    Toast.LENGTH_SHORT).show();
            finish();
            return;
        }
        Boolean mode = null;
        if (current.contains(MODE_USB)) {
            mode = true;
        } else if (current.contains(MODE_UART)) {
            mode = false;
        }
        updateStatus(mode);
        mRgMode.setOnCheckedChangeListener(mCheckListener);
    }

    @Override
    protected void onPause() {
        super.onPause();
    };

    @Override
    protected void onDestroy() {
        if (mWorkerHandler != null) {
            mWorkerHandler.removeMessages(MSG_CHECK_RESULT);
        }
        if (mWorkerThread != null) {
            mWorkerThread.quit();
        }
        super.onDestroy();
    };

    private void doSwitch(Boolean bModeUsb) {
        if (null != bModeUsb) {
            if (bModeUsb.booleanValue()) {
                if (isUartConnected()) {
                    showDialog(DIALOG_UART_CONNECT_WARNING);
                    return;
                }
                mModeVal = VAL_USB;
            } else {
                mModeVal = VAL_UART;
                if (isUsbConnected()) {
                    showDialog(DIALOG_USB_CONNECT_WARNING);
                    return;
                }
            }
            enableUsbUartSwitch(false);
            setUsbMode(mModeVal);
            mWorkerHandler.sendEmptyMessage(MSG_CHECK_RESULT);
        }
    }

    private void updateStatus(Boolean bModeUsb) {
        if (null == bModeUsb) {
            mTvCurrent.setText(R.string.uart_usb_switch_unknown);
            mRgMode.check(-1);
        } else if (bModeUsb.booleanValue()) {
            mTvCurrent.setText(R.string.uart_usb_switch_usb);
            mRgMode.check(R.id.uart_usb_switch_mode_usb);
        } else {
            mTvCurrent.setText(R.string.uart_usb_switch_uart);
            mRgMode.check(R.id.uart_usb_switch_mode_uart);
        }
    }

    private boolean isUartConnected() {
        boolean isConnected = false;
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("cat ");
        strBuilder.append(mUartStateFile);
        Elog.v(TAG, "isUartConnected cmd: " + strBuilder.toString());
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(strBuilder
                    .toString(), true)) {
                String result = ShellExe.getOutput();
                if (result.equals(UART_CONNECT)) {
                    isConnected = true;
                }
            }
        } catch (IOException e) {
            Elog.w(TAG, "get current dramc IOException: " + e.getMessage());
        }
        return isConnected;
    }

    private boolean isUsbConnected() {
        boolean isConnected = false;
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("cat ");
        strBuilder.append(USB_CONNECT_STATE);
        Elog.v(TAG, "isUsbConnected cmd: " + strBuilder.toString());
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(strBuilder
                    .toString(), true)) {
                String result = ShellExe.getOutput();
                if (result.equals(USB_CONFIGURED) || result.equals(USB_CONNECT)) {
                    isConnected = true;
                }
            }
        } catch (IOException e) {
            Elog.w(TAG, "get current dramc IOException: " + e.getMessage());
        }
        return isConnected;
    }

    private String getUsbMode() {
        String result = null;
        StringBuilder strBuilder = new StringBuilder();
        strBuilder.append("cat ");
        strBuilder.append(mPortFile);
        Elog.v(TAG, "get current dramc cmd: " + strBuilder.toString());
        try {
            if (ShellExe.RESULT_SUCCESS == ShellExe.execCommand(strBuilder
                    .toString(), true)) {
                result = ShellExe.getOutput();
            }
        } catch (IOException e) {
            Elog.w(TAG, "get current dramc IOException: " + e.getMessage());
        }
        return result;
    }

    private void setUsbMode(String value) {
        Elog.v(TAG, "setUsbMode(), value: " + value);

        try {
            EmUtils.getEmHidlService().setUsbPort(value);
        } catch (Exception e) {
            Elog.e(TAG, "set property failed ...");
            e.printStackTrace();
        }
    }

    private boolean waitForState(String modeVal, int milliSec) {
        int count = milliSec / 50;
        for (int i = 0; i < count; i++) {
            String relValue = EmUtils.systemPropertyGet(PROPERTY_USB_PORT,"unknown");
            Elog.d(TAG, "Check value of usb port mode:" + relValue);
            if (modeVal.equals(relValue)) {
                return true;
            }
            SystemClock.sleep(50);
        }
        return false;
    }
}
