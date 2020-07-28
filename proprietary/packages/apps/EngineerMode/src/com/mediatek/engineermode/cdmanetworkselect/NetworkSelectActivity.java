package com.mediatek.engineermode.cdmanetworkselect;

import android.app.Activity;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Spinner;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;

public class NetworkSelectActivity extends Activity implements OnCheckedChangeListener {
    private static final String TAG = "NetworkMode_cdma";
    private static final int EVENT_QUERY_NETWORKMODE_DONE = 101;
    private static final int EVENT_SET_NETWORKMODE_DONE = 102;
    private static final int EVENT_QUERY_EHRPD_ENABLE_DONE = 103;
    private static final int EVENT_SET_EHRPD_ENABLE_DONE = 104;

    private static final int HYBRID_INDEX = 0;
    private static final int CDMA_1X_ONLY_INDEX = 1;
    private static final int EVDO_ONLY_INDEX = 2;
    private static final int GLOBAL_MODE_INDEX = 3;
    private static final int LTE_CDMA_MODE_INDEX = 4;
    private static final int LTE_GSM_UMTS_MODE_INDEX = 5;

    private static final int HYBRID = 4;            //cdma_evdo
    private static final int CDMA_1X_ONLY = 5;     //cdma_only
    private static final int EVDO_ONLY = 6;         //EVDO_ONLY
    private static final int GLOBAL_MODE = 10;      //NETWORK_MODE_LTE_CDMA_EVDO_GSM_WCDMA
    private static final int LTE_CDMA_MODE = 8;     //NETWORK_MODE_LTE_CDMA_EVDO
    private static final int LTE_GSM_UMTS_MODE = 9; //NETWORK_MODE_LTE_GSM_WCDMA
    int selectNetworkMode = 0;
    private boolean mFirstEnter = true;
    private boolean mEHRPDFirstEnter = true;
    private boolean mSupportMode = true;
    private int mSimType;
    private int mSubId = 1;
    private Spinner mPreferredNetworkSpinner = null;
    private CheckBox mDisableeHRPDCheckBox = null;
    private ArrayAdapter<String> adapter = null;
    private String[] CdmaMode = {"Hybrid",               //0
            "CDMA 1X only",         //1
            "EVDO only"             //2
    };
    private String[] GsmMode = {
            "LTE/GSM/UMTS/CDMA",    //3
            "LTE/CDMA",             //4
            "LTE/GSM/UMTS"          //5
    };
    private TelephonyManager mTelephonyManager = null;
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            AsyncResult ar;
            switch (msg.what) {
                case EVENT_QUERY_NETWORKMODE_DONE:
                    Elog.d(TAG, "Get response EVENT_QUERY_NETWORKMODE_DONE");
                    int type = msg.arg1;
                    if (type > 0) {
                        Elog.d(TAG, "Get Preferred Type " + type);
                        EmUtils.showToast("Get NetworkMode Type " + type);
                        mSupportMode = true;
                        switch (type) {
                            case HYBRID:
                                mPreferredNetworkSpinner.setSelection(HYBRID_INDEX, true);
                                break;
                            case CDMA_1X_ONLY:
                                mPreferredNetworkSpinner.setSelection(CDMA_1X_ONLY_INDEX, true);
                                break;
                            case EVDO_ONLY:
                                mPreferredNetworkSpinner.setSelection(EVDO_ONLY_INDEX, true);
                                break;
                            case GLOBAL_MODE:
                                mPreferredNetworkSpinner.setSelection(GLOBAL_MODE_INDEX, true);
                                break;
                            case LTE_CDMA_MODE:
                                mPreferredNetworkSpinner.setSelection(LTE_CDMA_MODE_INDEX, true);
                                break;
                            case LTE_GSM_UMTS_MODE:
                                mPreferredNetworkSpinner.setSelection
                                        (LTE_GSM_UMTS_MODE_INDEX, true);
                                break;
                            default:
                                mSupportMode = false;
                                break;
                        }
                        if (mSupportMode == false) {
                            EmUtils.showToast("cdma netwok select not support the type: "
                                    + type, Toast.LENGTH_SHORT);
                            Elog.w(TAG, "cdma netwok select not support the type: ");
                        }
                    } else {
                        Elog.d(TAG, "query preferred failed");
                        EmUtils.showToast("query preferred failed");
                    }
                    break;
                case EVENT_SET_NETWORKMODE_DONE:
                    String info = "";
                    int status = msg.arg1;
                    if (status == 0) {
                        info = "set the network succeed";
                    } else {
                        info = "set the network failed";
                    }
                    EmUtils.showToast(info);
                    Elog.d(TAG, info);
                    break;
                case EVENT_QUERY_EHRPD_ENABLE_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        if (ar.result != null && ar.result instanceof String[]) {
                            String data[] = (String[]) ar.result;
                            if ((data.length > 0) && (data[0] != null)) {
                                Elog.d(TAG, "data[0]:" + data[0]);
                                if (mEHRPDFirstEnter) {
                                    mEHRPDFirstEnter = false;
                                }
                                mDisableeHRPDCheckBox.setChecked(data[0].equals("+EHRPD:0"));
                            }
                        }
                    } else {
                        EmUtils.showToast(R.string.query_eHRPD_state_fail, Toast.LENGTH_SHORT);
                        Elog.e(TAG, getString(R.string.query_eHRPD_state_fail));
                    }
                    break;
                case EVENT_SET_EHRPD_ENABLE_DONE:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        EmUtils.showToast(R.string.set_eHRPD_state_fail, Toast.LENGTH_SHORT);
                        Elog.e(TAG, getString(R.string.set_eHRPD_state_fail));
                        queryeHRPDStatus();
                    }
                    break;
                default:
                    break;
            }
        }
    };
    private OnItemSelectedListener mPreferredNetworkHandler = new OnItemSelectedListener() {
        public void onItemSelected(AdapterView parent, View v, int pos, long id) {
            switch (pos) {
                case HYBRID_INDEX: // 4
                    selectNetworkMode = HYBRID;
                    break;
                case CDMA_1X_ONLY_INDEX: // 5
                    selectNetworkMode = CDMA_1X_ONLY;
                    break;
                case EVDO_ONLY_INDEX: // 6
                    selectNetworkMode = EVDO_ONLY;
                    break;
                case GLOBAL_MODE_INDEX: //10
                    selectNetworkMode = GLOBAL_MODE;
                    break;
                case LTE_CDMA_MODE_INDEX: //8
                    selectNetworkMode = LTE_CDMA_MODE;
                    break;
                case LTE_GSM_UMTS_MODE_INDEX: //9
                    selectNetworkMode = LTE_GSM_UMTS_MODE;
                    break;
                default:
                    break;
            }
            if (mFirstEnter == true) {
                mFirstEnter = false;
            } else {
                Elog.d(TAG, "selectNetworkMode " + selectNetworkMode);
                EmUtils.showToast("selectNetworkMode to " + selectNetworkMode);

                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        boolean result = mTelephonyManager
                                .setPreferredNetworkType(mSubId, selectNetworkMode);
                        Message message = new Message();
                        message.what = EVENT_SET_NETWORKMODE_DONE;
                        message.arg1 = result ? 0 : -1;
                        mHandler.sendMessage(message);
                    }
                }).start();
                Settings.Global.putInt(getContentResolver(),
                        Settings.Global.PREFERRED_NETWORK_MODE + mSubId, selectNetworkMode);
                Elog.d(TAG, "write selectNetworkMode to settings ");
            }
        }

        public void onNothingSelected(AdapterView parent) {

        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.networkmode_switching);
        mPreferredNetworkSpinner = (Spinner) findViewById(R.id.networkModeSwitching);
        mDisableeHRPDCheckBox = (CheckBox) findViewById(R.id.disable_eHRPD);
        mTelephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
    }

    @Override
    protected void onResume() {
        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        Elog.d(TAG, "onCreate mSimType " + mSimType);


        int[] subId = SubscriptionManager.getSubId(mSimType);
        if (subId != null) {
            for (int i = 0; i < subId.length; i++) {
                Elog.i(TAG, "subId[" + i + "]: " + subId[i]);
            }
        }
        if (subId == null || subId.length == 0
                || !SubscriptionManager.isValidSubscriptionId(subId[0])) {
            EmUtils.showToast("Invalid sub id, please insert SIM Card!", Toast.LENGTH_SHORT);
            Elog.e(TAG, "Invalid sub id");
        } else {
            mSubId = subId[0];
        }


        adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item);

        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        for (int i = 0; i < CdmaMode.length; i++) {
            adapter.add(CdmaMode[i]);
        }

        for (int i = 0; i < GsmMode.length; i++) {
            adapter.add(GsmMode[i]);
        }

        mPreferredNetworkSpinner.setAdapter(adapter);

        mPreferredNetworkSpinner.setOnItemSelectedListener(mPreferredNetworkHandler);
        mDisableeHRPDCheckBox.setOnCheckedChangeListener(this);
        mFirstEnter = true;

        new Thread(new Runnable() {
            @Override
            public void run() {
                if (mTelephonyManager != null) {
                    int result = mTelephonyManager.getPreferredNetworkType(mSubId);
                    Message message = new Message();
                    message.what = EVENT_QUERY_NETWORKMODE_DONE;
                    message.arg1 = result;
                    mHandler.sendMessage(message);
                } else {
                    Elog.e(TAG, "mTelephonyManager = null");
                }
            }
        }).start();

        queryeHRPDStatus();
        super.onResume();
    }

    private void queryeHRPDStatus() {
        String atCommand = "AT+eHRPD?";
        sendAtCommand(ModemCategory.getCdmaCmdArr(
                new String[]{atCommand, "+EHRPD:", "DESTRILD:C2K"}),
                EVENT_QUERY_EHRPD_ENABLE_DONE);
    }

    private void seteHRPDStatus(int state) {
        String atCommand = null;
        switch (state) {
            case 0:
                atCommand = "AT+eHRPD=0";
                break;
            case 1:
                atCommand = "AT+eHRPD=1";
                break;
        }
        sendAtCommand(ModemCategory.getCdmaCmdArr(
                new String[]{atCommand, "", "DESTRILD:C2K"}), EVENT_SET_EHRPD_ENABLE_DONE);
    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.d(TAG, "sendAtCommand() " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(mSimType, command, mHandler.obtainMessage(msg));
    }

    @Override
    public void onCheckedChanged(CompoundButton view, boolean isChecked) {
        Elog.d(TAG, "m4GDataOnlyCheckBox check is" + isChecked);
        if (!mEHRPDFirstEnter) {
            mDisableeHRPDCheckBox.setChecked(isChecked);
            seteHRPDStatus(isChecked ? 0 : 1);
        }
    }
}
