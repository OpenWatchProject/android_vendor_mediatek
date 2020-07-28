package com.mediatek.engineermode.bluetooth;


import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Spinner;
import android.widget.TextView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

/**
 * Class for LE enhanced Test.
 *
 */
public class BleEnhancedTestMode extends Activity implements OnClickListener,
                OnCheckedChangeListener {

    private static final String TAG = "BLEEhTestMode";
    private static final int CHANNEL_NUM = 40;
    private static final int RETURN_SUCCESS = 0;

    // Dialog ID
    private static final int DLG_CHECK_STOP = 2;
    private static final int DLG_PL_LENGTH_INVALID = 3;
    private static final int DLG_CHECK_BT_DEVICE = 4;
    private static final int DLG_INIT_FAIL = 5;
    private static final int DLG_TEST_FAIL = 6;

    // Message ID
    private static final int INIT_BTTEST = 8;
    private static final int EXIT_BTTEST = 9;
    private static final int LIST_PHY = 10;
    private static final int TEST_START = 11;
    private static final int TEST_STOP = 12;

    private static final int PL_LENGTH_MIN = 0;
    private static final int PL_LENGTH_MAX = 255;

    // Widget
    private Button mBtnStart;
    private Button mBtnStop;
    private Spinner mSpChannel;
    private Spinner mSpPattern;
    private Spinner mSpPhy;
    private RadioGroup mRGTestMode;
    private TextView mTvResult;
    private EditText mEtPlLength;
    private View mViewPattern;
    private View mViewPlLength;

    // Value
    private String mResultStr;
    private ArrayAdapter<String> mSpPhyTxAdapter;
    private ArrayAdapter<String> mSpPhyRxAdapter;
    private int mChannelValue;
    private int mPatternValue;
    private int mPlLength;
    private int mPhyValue;

    private boolean m2MPhySupport = false;
    private boolean mCodedPhySupport = false;

    // jni layer object
    private BtTest mBtTest = new BtTest();

    private boolean mTestStarted = false;

    private boolean mIniting = false;
    private boolean mInited = false;

    // UI thread's handler
    private Handler mUiHandler = new Handler();

    // Working thread and handler
    private HandlerThread mWorkThread = null;
    private WorkHandler mWorkHandler = null;

    private BluetoothAdapter mBtAdapter = null;

    @Override
    protected void onCreate(Bundle onSavedInstanceState) {
        super.onCreate(onSavedInstanceState);
        setContentView(R.layout.ble_enhanced_test_mode);
        // Initialize UI component
        mViewPattern = findViewById(R.id.ble_eh_test_pattern_layout);
        mViewPlLength = findViewById(R.id.ble_eh_test_pl_length_layout);
        mBtnStart = (Button) findViewById(R.id.ble_eh_test_start_btn);
        mBtnStop = (Button) findViewById(R.id.ble_eh_test_stop_btn);
        mTvResult = (TextView) findViewById(R.id.ble_eh_test_result_tv);

        mSpChannel = (Spinner) findViewById(R.id.ble_eh_test_channel_sp);
        mSpPattern = (Spinner) findViewById(R.id.ble_eh_test_pattern_sp);
        mSpPhy = (Spinner) findViewById(R.id.ble_eh_test_phy_sp);

        mEtPlLength = (EditText) findViewById(R.id.ble_eh_test_pl_length_et);

        mBtnStart.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);

        ArrayAdapter<String> spChannelAdapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_spinner_item);
        spChannelAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        for (int i = 0; i < CHANNEL_NUM; i++) {
            spChannelAdapter
                    .add(getString(R.string.ble_eh_test_channnel) + i);
        }
        mSpChannel.setAdapter(spChannelAdapter);

        mSpPhyTxAdapter = new ArrayAdapter<String>(
                BleEnhancedTestMode.this, android.R.layout.simple_spinner_item);
        mSpPhyTxAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPhyRxAdapter = new ArrayAdapter<String>(
                BleEnhancedTestMode.this, android.R.layout.simple_spinner_item);
        mSpPhyRxAdapter
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mRGTestMode = (RadioGroup) findViewById(R.id.ble_eh_test_rg);
        mRGTestMode.setOnCheckedChangeListener(this);
        mRGTestMode.check(R.id.ble_eh_test_tx_rb);
        setViewState(true);

        mWorkThread = new HandlerThread(TAG);
        mWorkThread.start();

        mWorkHandler = new WorkHandler(mWorkThread.getLooper());

  }

    @Override
    protected void onResume() {
        super.onResume();
        if (mBtAdapter == null) {
            mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        }
        //Check BT state when activity is in foreground
        if (mBtAdapter == null) {
            showDialog(DLG_CHECK_BT_DEVICE);
        } else {
            if (!BtTest.checkInitState(mBtAdapter, this)) {
                finish();
            } else {
                if (mSpPhyTxAdapter.isEmpty()) {
                    Elog.v(TAG, "update phy list");
                    // Init test and get supported phy mode
                    mWorkHandler.sendEmptyMessage(INIT_BTTEST);
                    mWorkHandler.sendEmptyMessage(LIST_PHY);
                }
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DLG_CHECK_STOP:
            ProgressDialog dialog = new ProgressDialog(this);
            dialog.setMessage(getString(R.string.BT_init_dev));
            dialog.setCancelable(false);
            dialog.setIndeterminate(true);

            return dialog;

        case DLG_PL_LENGTH_INVALID:
            return new AlertDialog.Builder(this)
            .setTitle(R.string.Error)
            .setMessage(R.string.ble_eh_test_pl_length_invalid)
            .setPositiveButton(R.string.OK, null).create();

        case DLG_CHECK_BT_DEVICE:
            return new AlertDialog.Builder(this)
                .setCancelable(false)
                .setTitle(R.string.Error)
                .setMessage(R.string.BT_no_dev)
                .setPositiveButton(R.string.OK,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                int which) {
                            finish();
                        }
                    }).create();

        case DLG_INIT_FAIL:
            return new AlertDialog.Builder(this)
            .setCancelable(false)
            .setTitle(R.string.Error)
            .setMessage(R.string.ble_eh_test_init_fail)
            .setPositiveButton(R.string.OK,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog,
                            int which) {
                        finish();
                    }
                }).create();

        case DLG_TEST_FAIL:
            return new AlertDialog.Builder(this)
            .setTitle(R.string.Error)
            .setMessage(R.string.ble_eh_test_test_fail)
            .setPositiveButton(R.string.OK, null).create();

        default:
            return null;
        }
    }

    @Override
    protected void onDestroy() {
        //Uninit BT test
        mWorkHandler.sendEmptyMessage(EXIT_BTTEST);
        mWorkThread.quitSafely();
        super.onDestroy();

    }


    private void updatePhyList() {
        mSpPhyTxAdapter.add(getResources().getString(
                R.string.ble_eh_test_1m_phy));
        mSpPhyRxAdapter.add(getResources().getString(
                R.string.ble_eh_test_1m_phy));
        //TX and RX show different items for PHY mode
        if (m2MPhySupport) {
            mSpPhyTxAdapter.add(getResources().getString(
                    R.string.ble_eh_test_2m_phy));
            mSpPhyRxAdapter.add(getResources().getString(
                    R.string.ble_eh_test_2m_phy));
        }
        if (mCodedPhySupport) {
            mSpPhyTxAdapter.add(getResources().getString(
                    R.string.ble_eh_test_coded_phy_8));
            mSpPhyTxAdapter.add(getResources().getString(
                    R.string.ble_eh_test_coded_phy_2));
            mSpPhyRxAdapter.add(getResources().getString(
                    R.string.ble_eh_test_coded_phy));

        }
        if (mRGTestMode.getCheckedRadioButtonId() == R.id.ble_eh_test_tx_rb) {
            mSpPhy.setAdapter(mSpPhyTxAdapter);
        } else {
            mSpPhy.setAdapter(mSpPhyRxAdapter);
        }
    }


    private void handleTxTestStart() {
        //Send TX start cmd with params
        char[] cmd = new char[]{0x01, 0x34, 0x20, 0x04, (char) mChannelValue,
                (char) mPlLength, (char) mPatternValue, (char) mPhyValue};

        char[] response = mBtTest.hciCommandRun(cmd, cmd.length);
        mTestStarted = (response != null) ? true : false;
    }


    private void handleRxTestStart() {
        //Send RX start cmd with params
        char[] cmd = new char[]{0x01, 0x33, 0x20, 0x03,
              (char) mChannelValue, (char) mPhyValue, 0x00};

        char[] response = mBtTest.hciCommandRun(cmd, cmd.length);
        mTestStarted = (response != null) ? true : false;
    }

    private void handleTestStop(boolean showResult) {
        //Send test stop cmd
        char[] response = runStopTestCmd();
        if (response != null) {
            Elog.i(TAG, "response size " + response.length);
            if (showResult) {
                //show result for rx test
                if (response.length >= 9) {
                    mResultStr = String.format("***Packet Count: %d",
                            (long) response[8] * 256 + (long) response[7]);
                }

            } else {
                mResultStr = null;
            }
        }
        mTestStarted = false;
    }


    private char[] runStopTestCmd() {
        //Run test stop cmd
        char[] cmd = {0x01, 0x1F, 0x20, 0x00};
        return mBtTest.hciCommandRun(cmd, cmd.length);
    }


    private void handleStartBtnClick() {
        if (!initBtTestOjbect()) {
            return;
        }

        if (mRGTestMode.getCheckedRadioButtonId() == R.id.ble_eh_test_tx_rb) {
            handleTxTestStart();
        } else {
            handleRxTestStart();
        }
    }

    private void handleStopBtnClick() {
        if (mRGTestMode.getCheckedRadioButtonId() == R.id.ble_eh_test_tx_rb) {
            handleTestStop(false);
        } else {
            handleTestStop(true);
        }
        uninitBtTestOjbect();
    }

    private void setViewState(boolean state) {

        int count = mRGTestMode.getChildCount();
        for (int k = 0; k < count; k++) {
            mRGTestMode.getChildAt(k).setEnabled(state);
        }

        mSpChannel.setEnabled(state);
        mSpPattern.setEnabled(state);
        mSpPhy.setEnabled(state);
        mEtPlLength.setEnabled(state);
        mBtnStart.setEnabled(state);
        mBtnStop.setEnabled(!state);
    }

    //Init BT test
    private boolean initBtTestOjbect() {
        if (mIniting) {
            return false;
        }

        if (!mInited) {
            mIniting = true;
            mInited = (mBtTest.init() == RETURN_SUCCESS);
            Elog.i(TAG, "mBtTest init:" + mInited);
            mIniting = false;
        }
        return mInited;

    }


    private void uninitBtTestOjbect() {
        if (mInited) {
            mBtTest.unInit();
            mInited = false;
        }
    }

    /**
     * Handler for working thread.
     *
     */
    private final class WorkHandler extends Handler {

        private WorkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case INIT_BTTEST:
                if (!initBtTestOjbect()) {
                    mUiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            // TODO Auto-generated method stub
                            showDialog(DLG_INIT_FAIL);
                        }
                    });
                }
                break;
            case TEST_START:
                handleStartBtnClick();
                if (!mTestStarted) {
                    mUiHandler.post(new Runnable() {
                        @Override
                        public void run() {
                            // TODO Auto-generated method stub
                            setViewState(true);
                            showDialog(DLG_TEST_FAIL);
                        }
                    });
                }
                break;
            case TEST_STOP:
                handleStopBtnClick();
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        // TODO Auto-generated method stub
                        setViewState(true);
                        removeDialog(DLG_CHECK_STOP);
                        mTvResult.setText(mResultStr);
                    }
                });
                break;
            case EXIT_BTTEST:
                if (mTestStarted) {
                    handleTestStop(false);
                }
                uninitBtTestOjbect();
                break;
            case LIST_PHY:
                listPhy();
                mUiHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        // TODO Auto-generated method stub
                        updatePhyList();
                    }
                });
                break;
            default:
                break;
            }
        }
    }

    private void listPhy() {
        char[] cmd = new char[]{0x01, 0x03, 0x20, 0x00};
        char[] response = mBtTest.hciCommandRun(cmd, cmd.length);
        if ((response != null) && (response.length >= 3) && (response[6] == 0)) {
            m2MPhySupport = !((response[8] & 1) == 0);
            mCodedPhySupport = !((response[8] & (1 << 3)) == 0);
        }
    }

    @Override
    public void onClick(View v) {
        // TODO Auto-generated method stub

        if (v.equals(mBtnStart)) {
            Elog.i(TAG, "click start");
            mChannelValue = mSpChannel.getSelectedItemPosition();
            mPhyValue = mSpPhy.getSelectedItemPosition() + 1;
            if (mRGTestMode.getCheckedRadioButtonId() == R.id.ble_eh_test_tx_rb) {
                //Check pl length valid
                String strPlLength = mEtPlLength.getText().toString();
                try {
                    mPlLength = Integer.valueOf(strPlLength);
                } catch (NumberFormatException e) {
                    showDialog(DLG_PL_LENGTH_INVALID);
                    return;
                }

                if ((mPlLength < PL_LENGTH_MIN) || (mPlLength > PL_LENGTH_MAX)) {
                    showDialog(DLG_PL_LENGTH_INVALID);
                    return;
                }
                mPatternValue = mSpPattern.getSelectedItemPosition();
            }
            setViewState(false);
            mTvResult.setText("");
            mWorkHandler.sendEmptyMessage(TEST_START);

        } else if (v.equals(mBtnStop)) {
            Elog.i(TAG, "click stop");
            showDialog(DLG_CHECK_STOP);
            mWorkHandler.sendEmptyMessage(TEST_STOP);

        }
    }

    @Override
    public void onCheckedChanged(RadioGroup group, int checkedId) {
        // TODO Auto-generated method stub
        if (checkedId == R.id.ble_eh_test_tx_rb) {
            mViewPlLength.setVisibility(View.VISIBLE);
            mViewPattern.setVisibility(View.VISIBLE);
            mSpPhy.setAdapter(mSpPhyTxAdapter);
        } else {
            mViewPlLength.setVisibility(View.GONE);
            mViewPattern.setVisibility(View.GONE);
            mSpPhy.setAdapter(mSpPhyRxAdapter);
        }
    }
}