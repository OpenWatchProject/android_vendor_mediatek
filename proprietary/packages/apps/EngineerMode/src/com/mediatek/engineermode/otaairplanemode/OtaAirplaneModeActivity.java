package com.mediatek.engineermode.otaairplanemode;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.style.ForegroundColorSpan;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.mdml.Msg;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

public class OtaAirplaneModeActivity extends Activity implements View.OnClickListener,
        MDMCoreOperation.IMDMSeiviceInterface {
    private static final String TAG = "OtaAirplaneMode";

    private static final String SHREDPRE_NAME = "OtaAirplaneMode";
    private static final String KEY_FLAG_ENABLE = "enable_status";
    private static final String KEY_LAST_TIME = "last_time";
    private static final int MSG_ID_TIMEOUT = 1;
    private static final int MSG_ID_DL_TPUT_0_TIMEOUT = 2;
    private static final int DEFAULT_DL_TPUT_TIMEOUT = 10 * 1000;
    private static final float DEFAULT_AIRPLANE_TIMEOUT = 1.0f;
    private static final String SubscribeMsgIdName[] = {"MSG_ID_EM_EL1_STATUS_IND"};
    private static int mCount = 0;
    private static ServiceState mLastSS = null;
    private static int mLastCallState = TelephonyManager.CALL_STATE_IDLE;
    private static TextView mSimCardStatusTv;
    private static TextView mAirplaneCountTv;
    private static TextView airplane_network_status;
    private static EditText mAirplaneLastTimeEt;
    private static Button mLastTimeSetBt;
    private static CheckBox mEnableFeatureCb;
    private static List<MdmBaseComponent> componentsArray = new ArrayList<MdmBaseComponent>();
    private TelephonyManager mTelephonyManager;
    private PhoneStateListener mPhoneStateListener;
    private MsgHandler mHandler = new MsgHandler();
    private SimpleDateFormat mCurrectTime = null;
    private int mSimTypeToShow = 0;
    private int mDlTputTimeout = DEFAULT_DL_TPUT_TIMEOUT;
    private float mAirplanTimeout = DEFAULT_AIRPLANE_TIMEOUT;
    private int mLastDlTput = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ota_airplane_mode);
        log("Enter: onCreate");

        mSimCardStatusTv = (TextView) findViewById(R.id.airplan_simcard_status);
        mLastTimeSetBt = (Button) findViewById(R.id.airplane_time_last_set);
        mEnableFeatureCb = (CheckBox) findViewById(R.id.airplane_enable);
        mEnableFeatureCb.setOnCheckedChangeListener(new CheckBoxListener());
        mAirplaneCountTv = (TextView) findViewById(R.id.airplane_count);
        mAirplaneLastTimeEt = (EditText) findViewById(R.id.airplane_time_last);
        airplane_network_status = (TextView) findViewById(R.id.airplane_network_status);
        mLastTimeSetBt.setOnClickListener(this);
        mCurrectTime = new SimpleDateFormat("HH:mm:ss.S");

        if (!ModemCategory.isSimReady(-1)) {
            mSimCardStatusTv.setText("please insert only one card");
            log("please insert only one card");
            mLastTimeSetBt.setEnabled(false);
            mEnableFeatureCb.setEnabled(false);
            return;
        }

        log("The sim insert ");
        mSimCardStatusTv.setText("The sim insert ");
        mSimTypeToShow = PhoneConstants.SIM_ID_1;
        MdmBaseComponent components = new MdmBaseComponent();
        components.setEmComponentName(SubscribeMsgIdName);
        componentsArray.add(components);
        MDMCoreOperation.getInstance().mdmParametersSeting(componentsArray, mSimTypeToShow);
        MDMCoreOperation.getInstance().setOnMDMChangedListener(this);
        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);

        mPhoneStateListener = new PhoneStateListener() {
            @Override
            public void onServiceStateChanged(ServiceState serviceStatus) {

                if (is2GCSHasService(serviceStatus) != is2GCSHasService(mLastSS)) {
                    updateNetworkstatus("The 2G service status changed:", Color.BLUE);
                    updateNetworkstatus("\tis2GCSHasService(mLastSS): "
                            + is2GCSHasService(mLastSS), Color.BLUE);
                    updateNetworkstatus("\tis2GCSHasService(serviceState): "
                            + is2GCSHasService(serviceStatus), Color.BLUE);
                }
                if (is3G4GPSHasService(serviceStatus) != is3G4GPSHasService(mLastSS)) {
                    updateNetworkstatus("The 3G/4G service status changed:", Color.BLUE);
                    updateNetworkstatus("\tis3G4GPSHasService(mLastSS): "
                            + is3G4GPSHasService(mLastSS), Color.BLUE);
                    updateNetworkstatus("\tis3G4GPSHasService(serviceState):"
                            + is3G4GPSHasService(serviceStatus), Color.BLUE);
                }

                // 如果当前状态是2G CS无服务，且上一个状态是2G CS有服务，表示2G CS掉线，
                if (!is2GCSHasService(serviceStatus) && is2GCSHasService(mLastSS)) {
                    updateNetworkstatus("For 2G CS NO SERVICE:Call OTA Request", Color.RED);
                    handleOTARequest();
                } else if (!is3G4GPSHasService(serviceStatus) && is3G4GPSHasService(mLastSS)) {
                    updateNetworkstatus("For 3G/4G CS NO SERVICE:Call OTA Request", Color.RED);
                    handleOTARequest();
                }
                mLastSS = serviceStatus;
            }


            @Override
            public void onCallStateChanged(int state, String incomingNumber) {
                // 只要通话断开，不管是主动挂断，还是异常挂断，都开关飞行模式
                if (isInCall(state) != isInCall(mLastCallState)) {
                    updateNetworkstatus(getCurrectTime() + ":", Color.BLUE);
                    updateNetworkstatus("The call status changed:", Color.BLUE);
                    updateNetworkstatus("\tcall status(LastStatus): "
                            + isInCall(mLastCallState), Color.BLUE);
                    updateNetworkstatus("\tcall status(CurrentState): "
                            + isInCall(state), Color.BLUE);
                    if (!isInCall(state) && isInCall(mLastCallState)) {
                        updateNetworkstatus("For Call hangup: Call OTA Request", Color.RED);
                        handleOTARequest();
                    }
                }
                mLastCallState = state;
            }
        };

        mAirplanTimeout = Float.parseFloat(EmUtils.readSharedPreferences(SHREDPRE_NAME,
                KEY_LAST_TIME, String.valueOf(DEFAULT_AIRPLANE_TIMEOUT)));
        mAirplaneLastTimeEt.setText(String.valueOf(mAirplanTimeout));
        String buttonFlag = EmUtils.readSharedPreferences(SHREDPRE_NAME, KEY_FLAG_ENABLE, "0");
        if (buttonFlag.equals("1")) {
            mEnableFeatureCb.setChecked(true);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        log("onDestroy");
    }

    private String getCurrectTime() {
        String mTimes = mCurrectTime.format(new Date());
        return mTimes;
    }

    private void updateNetworkstatus(String msg, int color) {
        Spannable WordToSpan = new SpannableString(msg + "\r\n");
        WordToSpan.setSpan(new ForegroundColorSpan(color), 0, WordToSpan.length(),
                Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        airplane_network_status.append(WordToSpan);
    }

    void registerNetworkListen() {
        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_SERVICE_STATE |
                        PhoneStateListener.LISTEN_CALL_STATE);

        MDMCoreOperation.getInstance().mdmInitialize(this);
    }

    void unRegisterNetworkListen() {
        mTelephonyManager.listen(mPhoneStateListener,
                PhoneStateListener.LISTEN_NONE);
        MDMCoreOperation.getInstance().mdmlUnSubscribe();
        mHandler.removeCallbacksAndMessages(null);
    }

    void setupOtaRequest(boolean enable) {
        if (enable) {
            registerNetworkListen();
            airplane_network_status.setText("");
            mAirplaneCountTv.setText(String.valueOf(mCount));
            log("Enable the status listen succeed");
            EmUtils.showToast("Enable the status listen succeed");
            EmUtils.writeSharedPreferences(SHREDPRE_NAME, KEY_FLAG_ENABLE, "1");
        } else {
            mCount = 0;
            unRegisterNetworkListen();
            log("Disable the listen succeed");
            EmUtils.showToast("Disable the listen succeed");
            EmUtils.writeSharedPreferences(SHREDPRE_NAME, KEY_FLAG_ENABLE, "0");
        }
    }

    // 按照OTA需求，先打开飞行模式，过几秒后再关闭飞行模式
    private void handleOTARequest() {
        log("handleOTARequest");
        setAirplaneMode(true);
        mHandler.sendMessageDelayed(mHandler.obtainMessage(MSG_ID_TIMEOUT), (int)(mAirplanTimeout *
                1000));
    }

    private void setAirplaneMode(boolean onOff) {
        updateNetworkstatus(getCurrectTime() + ":", Color.GREEN);
        updateNetworkstatus("setAirplaneMode, onOff: " + onOff, Color.GREEN);
        EmUtils.setAirplaneModeEnabled(onOff);
    }

    // 如果注册上2G CS, 返回 true, 否则返回 false
    private boolean is2GCSHasService(ServiceState ss) {
        if (ss == null) {
            return false;
        }

        int voiceNetworkType = ss.getVoiceNetworkType();
        if (ss.getVoiceRegState() == ServiceState.STATE_IN_SERVICE) {
            if (is2G(voiceNetworkType)) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    private boolean is2G(int networkType) {
        return networkType == TelephonyManager.NETWORK_TYPE_GPRS
                || networkType == TelephonyManager.NETWORK_TYPE_EDGE
                || networkType == TelephonyManager.NETWORK_TYPE_GSM
                || networkType == TelephonyManager.NETWORK_TYPE_CDMA
                || networkType == TelephonyManager.NETWORK_TYPE_1xRTT;
    }

    // 如果注册上3G/4G PS, 返回 true, 否则返回 false
    private boolean is3G4GPSHasService(ServiceState ss) {
        if (ss == null) {
            return false;
        }

        int dataNetworkType = ss.getDataNetworkType();
        if (ss.getDataRegState() == ServiceState.STATE_IN_SERVICE) {
            if (is3G4G(dataNetworkType)) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    private boolean is3G4G(int networkType) {
        return networkType == TelephonyManager.NETWORK_TYPE_UMTS
                || networkType == TelephonyManager.NETWORK_TYPE_HSDPA
                || networkType == TelephonyManager.NETWORK_TYPE_HSUPA
                || networkType == TelephonyManager.NETWORK_TYPE_HSPA
                || networkType == TelephonyManager.NETWORK_TYPE_LTE
                || networkType == TelephonyManager.NETWORK_TYPE_HSPAP
                || networkType == TelephonyManager.NETWORK_TYPE_TD_SCDMA
                || networkType == TelephonyManager.NETWORK_TYPE_EVDO_0
                || networkType == TelephonyManager.NETWORK_TYPE_EVDO_A
                || networkType == TelephonyManager.NETWORK_TYPE_EVDO_B
                || networkType == TelephonyManager.NETWORK_TYPE_EHRPD;
        // || networkType == TelephonyManager.RIL_RADIO_TECHNOLOGY_LTE_CA;
    }

    private boolean isInCall(int state) {
        return !(state == TelephonyManager.CALL_STATE_IDLE);
    }

    public void setAirplaneSecond() {
        log("onClick, setAirplaneSecond");

        String input = mAirplaneLastTimeEt.getText().toString();
        if (input == null || input.equals("")) {
            EmUtils.showToast("Use the default value: 1s");
            return;
        }
        mAirplanTimeout = Float.parseFloat(input);
        EmUtils.writeSharedPreferences(SHREDPRE_NAME, KEY_LAST_TIME, String.valueOf
                (mAirplanTimeout));
        EmUtils.showToast("Set the Airplane Last time to(s):" + mAirplanTimeout);
        log("Set the Airplane Last time to(s): " + mAirplanTimeout);
    }

    private void log(String s) {
        Elog.i(TAG, s);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.airplane_time_last_set:
                setAirplaneSecond();
                break;
        }
    }

    @Override
    public void onUpdateMDMStatus(int msg_id) {
        switch (msg_id) {
            case MDMCoreOperation.MDM_SERVICE_INIT: {
                log("MDM Service init done");
                MDMCoreOperation.getInstance().mdmlSubscribe();
                break;
            }
            case MDMCoreOperation.SUBSCRIBE_DONE: {
                log("Subscribe message id done");
                MDMCoreOperation.getInstance().mdmlEnableSubscribe();
                break;
            }
            case MDMCoreOperation.UNSUBSCRIBE_DONE: {
                log("UnSubscribe message id done");
                MDMCoreOperation.getInstance().mdmlClosing();
                break;
            }
            default:
                break;
        }
    }

    @Override
    public void onUpdateMDMData(String name, Msg data) {

        if (name.equals("MSG_ID_EM_EL1_STATUS_IND")) {
            String coName = "dl_info" + "[0].";
            int dl_tput = MDMCoreOperation.getInstance().getFieldValue(data, coName + "DL_Tput",
                    true);
            log("PrimaryCellDlThroughput,dl_tput : " + dl_tput);

            if (isOtaPSDisconnect(dl_tput) && !isOtaPSDisconnect(mLastDlTput)) {
                log("The Ota PSD status changed:");
                log("PSD status(Last):" + mLastDlTput);
                log("PSD status(Serving):" + dl_tput);
                handleOtaPSDisconnect(true);
            } else if (!isOtaPSDisconnect(dl_tput) && isOtaPSDisconnect(mLastDlTput)) {
                log("The Ota PSD status changed:");
                log("PSD status(Last):" + mLastDlTput);
                log("PSD status(Serving):" + dl_tput);
                handleOtaPSDisconnect(false);
            }
            mLastDlTput = dl_tput;
        }
    }

    private boolean isOtaPSDisconnect(int dl_tput) {
        return dl_tput == 0;
    }

    private void handleOtaPSDisconnect(boolean disconnect) {
        log("PSD status:" + disconnect);
        if (disconnect) {
            mHandler.sendMessageDelayed(
                    mHandler.obtainMessage(MSG_ID_DL_TPUT_0_TIMEOUT), mDlTputTimeout);
        } else {
            mHandler.removeMessages(MSG_ID_DL_TPUT_0_TIMEOUT);
        }
    }

    private class MsgHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ID_TIMEOUT:
                    setAirplaneMode(false);
                    ++mCount;
                    mAirplaneCountTv.setText(String.valueOf(mCount));
                    log("Set AirplaneMode off, mCount: " + mCount);
                    break;
                case MSG_ID_DL_TPUT_0_TIMEOUT:
                    updateNetworkstatus("PSD disabled timeout:Call OTA Request", Color.RED);
                    handleOTARequest();
                    break;
            }
        }
    }

    class CheckBoxListener implements OnCheckedChangeListener {
        @Override
        public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
            log("Enable listen the network status changed: " + isChecked);
            setupOtaRequest(isChecked);
        }
    }

}
